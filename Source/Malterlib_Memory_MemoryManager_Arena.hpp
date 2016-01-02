// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Memory_MemoryManager_Arena_Small.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Normal.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Slab.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Message.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Garbage.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Commit.hpp"

namespace NMib
{
	namespace NMem
	{
		template <typename t_CParams>
		TCMemoryManagerArena<t_CParams>::TCMemoryManagerArena(TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic, ENumaNode _NumaNode, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena)
			: t_CParams::CNotifier::CArena(_pMemoryManager)
			, m_pMemoryManager(_pMemoryManager)
			, m_Magic(_Magic)
			, m_NumaNode(_NumaNode)
			, m_pNumaArena(_pNumaArena)
			, m_CheckoutCount(0)
			, m_bWantCleanup(false)
			, m_bRequestedCleanup(false)
			, m_bWantNumaFreeSlabsCleanup(false)
			, m_Locked(EArenaLockFlag_None)
		{
			DMibFastCheck((uint8 *)&m_Locked == fg_AlignUp((uint8 *)&m_Locked, DMibPMemoryCacheLineSize));
			DMibFastCheck((uint8 *)&m_Messages == fg_AlignUp((uint8 *)&m_Messages, DMibPMemoryCacheLineSize));
		}
		
		template <typename t_CParams>
		void TCMemoryManagerArena<t_CParams>::fp_RequestCleanup()
		{
			m_bWantCleanup = true;
		}
		
		template <typename t_CParams>
		bool TCMemoryManagerArena<t_CParams>::f_ReturnCheckout()
		{
			DMibFastCheck(m_CheckoutCount > 0);
			if (--m_CheckoutCount == 0)
			{
				fp_CheckMessages();
				bool bNeedCleanup = fp_CheckCleanup();
				auto LockResult = m_Locked.f_Exchange(EArenaLockFlag_None, NAtomic::EMemoryOrder_Release);
				if (LockResult & EArenaLockFlag_Waiting)
					m_pNumaArena->f_ArenaAvailable(this);
				if ((LockResult & EArenaLockFlag_Cleanup) || bNeedCleanup)
					m_pNumaArena->f_OnNeedCleanup();
				return true;
			}
			return false;
		}		

		template <typename t_CParams>
		void TCMemoryManagerArena<t_CParams>::f_ReturnCheckoutLight()
		{
			DMibFastCheck(m_CheckoutCount == 0); // Should not have been increased
			bool bNeedCleanup = false;
			fp_CheckMessages();
			if (m_bWantCleanup)
				bNeedCleanup = fp_CheckCleanup();
			auto LockResult = m_Locked.f_Exchange(EArenaLockFlag_None, NAtomic::EMemoryOrder_Release);
			if (LockResult & EArenaLockFlag_Waiting)
				m_pNumaArena->f_ArenaAvailable(this);
			if ((LockResult & EArenaLockFlag_Cleanup) || bNeedCleanup)
				m_pNumaArena->f_OnNeedCleanup();
		}		
		
		template <typename t_CParams>
		TCMemoryManagerArena<t_CParams>::~TCMemoryManagerArena()
		{
			{
				for (mint iSlab = 0; iSlab < mc_nSmallSizeSlabs; ++iSlab)
					m_SmallSizeSlabs[iSlab].f_Clear();
			}
			{
				for (mint iSlab = 0; iSlab < mc_nLevel0Lists; ++iSlab)
					m_NormalSizeSlabsLevel0[iSlab].f_Clear();
			}
			{
				for (mint iSlab2 = 0; iSlab2 < t_CParams::mc_NumSizesPerLevel; ++iSlab2)
				{
					for (mint iSlab = 0; iSlab < mc_nNormalSizeLists; ++iSlab)
						m_NormalSizeSlabs[iSlab2][iSlab].f_Clear();
				}
			}
			m_SmallSizeSlabsFull.f_Clear();

			while (auto pSlab = m_FreeSlabs.f_Pop())
			{
				pSlab->~TCMemoryManagerSlabShared<t_CParams>();
				m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
			}
	
			{
				mint nSlabs = sizeof(m_PartiallyFreeSlabs) / sizeof(m_PartiallyFreeSlabs[0]);
				for (mint iSlab = 0; iSlab < nSlabs; ++iSlab)
				{
					for (mint iLevel = 0; iLevel < mc_NumSubSlabSizeLevels; ++iLevel)
					{
						while (auto pSlab = m_PartiallyFreeSlabs[iSlab][iLevel].f_Pop())
						{
							pSlab->~TCMemoryManagerSlabShared<t_CParams>();
							m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
						}
						m_PartiallyFreeSlabsAvailable[iSlab].template f_SetBit<false>(iLevel);
					}
				}
			}
	
			while (auto pSlab = m_FullSlabs.f_Pop())
			{
				pSlab->~TCMemoryManagerSlabShared<t_CParams>();
				m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
			}
		}

		template <typename t_CParams>
		void TCMemoryManagerArena<t_CParams>::fp_FreeSmallSubSlabs(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
		{
			auto SlabType = _pSlab->m_SlabType;
			for (auto iSubSlab = _pSlab->m_FreeSubSlabs.f_GetIterator(); iSubSlab; )
			{
				auto pSubSlab = &(*iSubSlab);
				++iSubSlab;
				--_pSlab->m_nFreeSubSlabs;
				pSubSlab->~CMemoryManagerSubSlab_Free();
				auto pSlabAddress = (uint8 *)pSubSlab;
				auto iAlloc = mint(pSlabAddress - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
				iAlloc = t_CParams::fs_DivideBySlabMultiplier(iAlloc, SlabType);
				_pSlab->f_DecommitSubSlabs(iAlloc, 1);
				_pSlab->f_SetBitFree(0, iAlloc);
			}
			DMibFastCheck(_pSlab->m_nFreeSubSlabs == 0);
			fp_CheckSlabNoLongerGarbage(_pSlab);
		}


		template <typename t_CParams>
		inline_small void *TCMemoryManagerArena<t_CParams>::f_Alloc(mint &_Size)
		{
			//fp_CheckMessages();
			void * pAlloc;
			if (_Size <= mc_SmallSizeSlabsLargestSize)
				pAlloc = fp_AllocSmallSize(_Size);
			else
				pAlloc = fp_AllocNormal(_Size);

			if (unlikely(m_bWantNumaFreeSlabsCleanup))
				fp_CheckCleanupNumaFree();
			return pAlloc;
		}

		template <typename t_CParams>
		void TCMemoryManagerArena<t_CParams>::f_AllocBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			if (_Size <= mc_SmallSizeSlabsLargestSize)
				fp_AllocSmallSizeBatch(_Size, _Functor);
			else
				fp_AllocNormalBatch(_Size, _Functor);

			if (unlikely(m_bWantNumaFreeSlabsCleanup))
				fp_CheckCleanupNumaFree();
		}

		template <typename t_CParams>
		inline_small void TCMemoryManagerArena<t_CParams>::f_FreeThisThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
		{
			//fp_CheckMessages();
			fp_FreeInline(_pMemory, _pSlab);
			if (unlikely(m_bWantNumaFreeSlabsCleanup))
				fp_CheckCleanupNumaFree();
		}

		template <typename t_CParams>
		inline_never void TCMemoryManagerArena<t_CParams>::fp_Free(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
		{
			return fp_FreeInline(_pMemory, _pSlab);
		}

		template <typename t_CParams>
		inline_small void TCMemoryManagerArena<t_CParams>::f_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena)
		{
			fp_FreeOtherThread(_pMemory, _pSlab, _LocalArena);
		}


		template <typename t_CParams>
		inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena)
		{
			auto SubSlab = mint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

			mint SlabType = _pSlab->m_SlabType;
			auto pData = _pSlab->f_GetSubSlabData();

			mint iSubSlab;
			mint SlabBucket;
			CMessage *pFreeLink = (CMessage *)_pMemory;
			EMessageType FreeLinkType = EMessageType_FreeNormalBlock;

			if (SlabType == 0)
			{
				iSubSlab = SubSlab;
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
				
#if DMibPPtrBits == 32
				if (SlabBucket <= 1)
#elif DMibPPtrBits == 64
				if (SlabBucket <= 2)
#else
#error "Implement this"
#endif
				{
					TCMemoryManagerCheckoutLight<t_CParams> Checkout(nullptr);
					if (_LocalArena.m_pArena == nullptr)
					{
						Checkout = m_pMemoryManager->fp_Checkout(_LocalArena);
						if (_LocalArena.m_pArena == _pSlab->m_pArena)
						{
							f_FreeThisThread(_pMemory, _pSlab);
							return;
						}
					}
					
					FreeLinkType = EMessageType_FreeSmallBlock;
					mint Size = sizeof(CMessage_FreeSmallBlock);
					CMessage_FreeSmallBlock *pFreeLinkSmallBlock = (CMessage_FreeSmallBlock *)m_pMemoryManager->f_Alloc(Size);
					pFreeLinkSmallBlock->m_pBlock = _pMemory;
					pFreeLink = pFreeLinkSmallBlock;
				}
			}

			f_AddMessage(pFreeLink, FreeLinkType);
			m_pNumaArena->f_RequestCleanupWeak(ENumaArenaCleanup_ProcessMessages);
		}
		
		template <typename t_CParams>
		inline_small mint TCMemoryManagerArena<t_CParams>::fs_GetAllocSize(mint _Size)
		{
			if (_Size <= mc_SmallSizeSlabsLargestSize)
			{
				mint iSlab = fps_GetSlabTypeFromSizeSmall(_Size);
				switch (iSlab)
				{
				case 0:
					return 1;
				case 1:
					return 2;
				case 2:
					return 4;
				case 3:
					return 8;
				case 4:
					if (mc_MinAlignment == 4)
						return 12;
					else
						return 16;
				case 5:
					if (mc_MinAlignment == 4)
						return 16;
				default:
					DMibFastCheck(false);
					break;
				}
				return 0;
			}
			else
			{
				mint Size = fg_AlignUp(_Size, mc_MinNormalSizeAlignment);
				DMibFastCheck(Size >= sizeof(void *) * 2);
				mint SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
				mint SlabBucketGranularity = (mint(1) << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
				return fg_AlignUp(Size, SlabBucketGranularity);
			}
			return 0;
		}

		template <typename t_CParams>
		inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab)
		{
			auto pData = _pSlab->f_GetSubSlabData();
			auto pSlabData = _pSlab->f_GetSlabStart();
			mint SlabType = _pSlab->m_SlabType;
			
			DMibFastCheck(_iSubSlab < _pSlab->f_GetNumSubSlabs());
			
			mint SlabBucket = pData[_iSubSlab].m_Allocated.m_Type - 2;
			
			DMibFastCheck(SlabBucket < 21);
			
			DMibFastCheck(pData[_iSubSlab].m_Allocated.m_nAllocs == 0);
			
			mint AlignedSize;

			mint SubSlabMultiplier = t_CParams::ms_SlabTypeInfo[SlabType].m_SubSlabMutiplier;
			mint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;
					
			mint SlabBucketStart = mint(1) << SlabBucket;
			AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

			uint32 nSubSlabs = 1;
			if (AlignedSize > MinSize)
				nSubSlabs = t_CParams::fs_DivideBySlabMultiplier(AlignedSize/t_CParams::mc_SubSlabSize, SlabType);
			
			auto pStart = pSlabData + _iSubSlab * t_CParams::mc_SubSlabSize * SubSlabMultiplier;
			auto pEnd = pStart + t_CParams::mc_SubSlabSize * SubSlabMultiplier * nSubSlabs;
			for (auto pToRemove = pStart; pToRemove < pEnd; )
			{
				CMemoryManagerSubSlab_NormalLink *pFreeBlock = (CMemoryManagerSubSlab_NormalLink *)pToRemove;
				pToRemove += AlignedSize * pFreeBlock->m_nBlocks;
				pFreeBlock->m_Link.f_UnsafeUnlink();
			}

			if (!(t_CParams::mc_DeferCleanup & EDeferCleanup_OneSizeBlocks) || nSubSlabs > 1)
			{
				_pSlab->f_DecommitSubSlabs(_iSubSlab, nSubSlabs);
				
				mint Level = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);

				DMibFastCheck(((_iSubSlab >> Level) << Level) == _iSubSlab);
				_pSlab->f_SetBitFree(Level, _iSubSlab >> Level);
			}
			else
			{
				CMemoryManagerSubSlab_Free *pFreeSubSlab = (CMemoryManagerSubSlab_Free *)pStart;
				pFreeSubSlab = new(pFreeSubSlab) CMemoryManagerSubSlab_Free();
				_pSlab->m_FreeSubSlabs.f_UnsafeInsert(pFreeSubSlab);
				fp_SlabHasGarbage(_pSlab);
				++_pSlab->m_nFreeSubSlabs;
			}
 
			_pSlab->m_nAllocatedSubSlabs -= nSubSlabs;
		}

		template <typename t_CParams>
		mint TCMemoryManagerArena<t_CParams>::f_GetNumFreeSlabs()
		{
			mint nSlabs = 0;

			nSlabs += m_FreeSlabs.f_GetLen();
			
			return nSlabs;			
		}
		
		template <typename t_CParams>
		mint TCMemoryManagerArena<t_CParams>::f_GetNumUsedSlabs()
		{
			mint nSlabs = 0;
			for (mint iSlabType = 0; iSlabType < t_CParams::mc_NumSizesPerLevel; ++iSlabType)
			{
				for (mint i = 0; i < mc_NumSubSlabSizeLevels; ++i)
					nSlabs += m_PartiallyFreeSlabs[iSlabType][i].f_GetLen();
			}

			nSlabs += m_FullSlabs.f_GetLen();
			
			return nSlabs;			
		}

		template <typename t_CParams>
		bool TCMemoryManagerArena<t_CParams>::f_CheckFree(bool _bBreak)
		{
			bool bError = false;
			for (mint i = 0; i < mc_nLevel0Lists; ++i)
			{
				for (auto iAlloc = m_NormalSizeSlabsLevel0[i].f_GetIterator(); iAlloc; ++iAlloc)
				{
					if (fp_CheckFree(&*iAlloc, _bBreak))
						bError = true;
					
				}
			}
			for (mint j = 0; j < t_CParams::mc_NumSizesPerLevel; ++j)
			{
				for (mint i = 0; i < mc_nNormalSizeLists; ++i)
				{
					for (auto iAlloc = m_NormalSizeSlabs[j][i].f_GetIterator(); iAlloc; ++iAlloc)
					{
						if (fp_CheckFree(&*iAlloc, _bBreak))
							bError = true;
					}
				}
			}
			
			if (fp_CheckFreeSmall<1>(_bBreak))
				bError = true;
			if (fp_CheckFreeSmall<2>(_bBreak))
				bError = true;
			if (fp_CheckFreeSmall<4>(_bBreak))
				bError = true;
			if (fp_CheckFreeSmall<8>(_bBreak))
				bError = true;
			if (mc_MinAlignment == 4)
				if (fp_CheckFreeSmall<12>(_bBreak))
					bError = true;
			if (fp_CheckFreeSmall<16>(_bBreak))
				bError = true;
 			
			return bError;
		}
		
	}
}