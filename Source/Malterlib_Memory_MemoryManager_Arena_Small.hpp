// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{

		template <typename t_CParams>
		inline_small void *TCMemoryManagerArena<t_CParams>::fp_AllocSmallSize(mint &_Size)
		{
			
			mint iSlab = fps_GetSlabTypeFromSizeSmall(_Size);
			switch (iSlab)
			{
			case 0:
				_Size = 1;
				return fp_AllocSmall<1>();
			case 1:
				_Size = 2;
				return fp_AllocSmall<2>();
			case 2:
				_Size = 4;
				return fp_AllocSmall<4>();
			case 3:
				_Size = 8;
				return fp_AllocSmall<8>();
			case 4:
				if (mc_MinAlignment == 4)
				{
					_Size = 12;
					return fp_AllocSmall<12>();
				}
				else
				{
					_Size = 16;
					return fp_AllocSmall<16>();
				}
			case 5:
				if (mc_MinAlignment == 4)
				{
					_Size = 16;
					return fp_AllocSmall<16>();
				}
			default:
				DMibFastCheck(false);
				break;
			}
			return nullptr;
		}
		
		template <typename t_CParams>
		void TCMemoryManagerArena<t_CParams>::fp_AllocSmallSizeBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			mint iSlab = fps_GetSlabTypeFromSizeSmall(_Size);
			switch (iSlab)
			{
			case 0:
				while (true)
				{
					auto *pAlloc = fp_AllocSmall<1>();
					if (!_Functor(pAlloc, 1))
						break;
				}
				return;
			case 1:
				while (true)
				{
					auto *pAlloc = fp_AllocSmall<2>();
					if (!_Functor(pAlloc, 2))
						break;
				}
				return;
			case 2:
				while (true)
				{
					auto *pAlloc = fp_AllocSmall<4>();
					if (!_Functor(pAlloc, 4))
						break;
				}
				return;
			case 3:
				while (true)
				{
					auto *pAlloc = fp_AllocSmall<8>();
					if (!_Functor(pAlloc, 8))
						break;
				}
				return;
			case 4:
				if (mc_MinAlignment == 4)
				{
					while (true)
					{
						auto *pAlloc = fp_AllocSmall<12>();
						if (!_Functor(pAlloc, 12))
							break;
					}
					return;
				}
				else
				{
					while (true)
					{
						auto *pAlloc = fp_AllocSmall<16>();
						if (!_Functor(pAlloc, 16))
							break;
					}
					return;
				}
			case 5:
				if (mc_MinAlignment == 4)
				{
					while (true)
					{
						auto *pAlloc = fp_AllocSmall<16>();
						if (!_Functor(pAlloc, 16))
							break;
					}
					return;
				}
			default:
				DMibFastCheck(false);
				break;
			}
		}
		
		
		template <typename t_CParams>
		inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeSmall(void *_pMemory, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, mint _SlabType)
		{
			// Small slabs
			switch (_SlabType)
			{
			case 0:
				return fp_FreeSmall<1>(_pMemory, _pSlab);
			case 1:
				return fp_FreeSmall<2>(_pMemory, _pSlab);
			case 2:
				return fp_FreeSmall<4>(_pMemory, _pSlab);
			case 3:
				return fp_FreeSmall<8>(_pMemory, _pSlab);
			case 4:
				if (mc_MinAlignment == 4)
					return fp_FreeSmall<12>(_pMemory, _pSlab);
				else
					return fp_FreeSmall<16>(_pMemory, _pSlab);
			case 5:
				if (mc_MinAlignment == 4)
					return fp_FreeSmall<16>(_pMemory, _pSlab);
			default:
				DMibFastCheck(false);
				break;
			}
		}


		template <typename t_CParams>
		mint TCMemoryManagerArena<t_CParams>::fp_SizeSmall(void const * _pMemory, TCMemoryManagerSlab<t_CParams, 0> const * _pSlab, mint _SlabType) const
		{
			switch (_SlabType)
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

		template <typename t_CParams>
		fp32 TCMemoryManagerArena<t_CParams>::fp_OverheadSmall(void const * _pMemory, TCMemoryManagerSlab<t_CParams, 0> const * _pSlab, mint _SlabType) const
		{
			switch (_SlabType)
			{
			case 0:
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::mc_NumAllocs);
			case 1:
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 2>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 2>::mc_NumAllocs);
			case 2:
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 4>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 4>::mc_NumAllocs);
			case 3:
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 8>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 8>::mc_NumAllocs);
			case 4:
				if (mc_MinAlignment == 4)
					return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 12>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 12>::mc_NumAllocs);
				else
					return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>::mc_NumAllocs);
			case 5:
				if (mc_MinAlignment == 4)
					return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>::mc_NumAllocs);
			default:
				DMibFastCheck(false);
				break;
			}
			return 0.0;
		}
		
		template <typename t_CParams>
		inline_small mint TCMemoryManagerArena<t_CParams>::fps_GetSlabTypeFromSizeSmall(mint _Size)
		{
			mint iSlab;
			if (_Size < 2)
				iSlab = 0;
			else if (_Size <= TCMemoryManagerArena<t_CParams>::mc_MinAlignment)
				iSlab = NMib::fg_GetHighestBitSetNoZero(_Size - 1) + 1;
			else
				iSlab = TCMemoryManagerArena<t_CParams>::mc_nSmallSizeSlabsAligned + (_Size - (TCMemoryManagerArena<t_CParams>::mc_MinAlignment + 1)) / TCMemoryManagerArena<t_CParams>::mc_MinAlignment;

			return iSlab;
		}

		template <typename t_CParams>
		template <mint tf_Size>
		inline_never void *TCMemoryManagerArena<t_CParams>::fp_AllocSmall()
		{
			typedef TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> CSubSlab;

			auto &Slabs = m_SmallSizeSlabs[CSubSlab::mc_SmallSlabIndex];
			CSubSlab *pSlab = (CSubSlab *)Slabs.f_GetFirst();
			if (!pSlab)
				pSlab = fp_AllocSmallNoSlab<tf_Size>();

			bool bFull;
			void *pAlloc = pSlab->f_Alloc(bFull);
			if (this->mc_EnableCallbacks)
			{
				pSlab->f_OnCheckFree(*this, pAlloc, true);
				pSlab->f_OnAlloc(*this, pAlloc);
			}

			if (bFull)
			{
				pSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();
				m_SmallSizeSlabsFull.f_UnsafeInsertFirst(pSlab->m_Params.m_Link);
			}

			return pAlloc;
		}

		template <typename t_CParams>
		template <mint tf_Size>
		inline_never TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> *TCMemoryManagerArena<t_CParams>::fp_AllocSmallNoSlab() 
		{
			typedef TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> CSubSlab;

			auto pFreeSlab = (TCMemoryManagerSlab<t_CParams, 0> *)m_PartiallyFreeSlabs[0][0].f_GetFirst();

			if (!pFreeSlab)
				pFreeSlab = (TCMemoryManagerSlab<t_CParams, 0> *)fp_NewSlab(0, 0);

			CMemoryManagerSubSlab_Free *pExistingSlab = pFreeSlab->m_FreeSubSlabs.f_UnsafePop();
			uint8 *pSlabAddress;
			aint iAlloc;
			if (pExistingSlab)
			{
				fp_CheckSlabNoLongerGarbage(pFreeSlab);
				
				--pFreeSlab->m_nFreeSubSlabs;
				
				pSlabAddress = (uint8 *)pExistingSlab;
				iAlloc = mint((uint8 *)pSlabAddress - pFreeSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
			}
			else
			{
				iAlloc = pFreeSlab->f_FindFreeBitAndSet(0);

				pSlabAddress = pFreeSlab->f_GetSlabStart() + iAlloc * t_CParams::mc_SubSlabSize;
				pFreeSlab->f_CommitSubSlabs(iAlloc, 1);
			}
			++pFreeSlab->m_nAllocatedSubSlabs;

			mint SlabIndex = CSubSlab::mc_SmallSlabIndex;
			pFreeSlab->m_SubSlabData[iAlloc].m_Allocated.m_Type = SlabIndex;
			//DMibDTrace("Small: {}" DMibNewLine, pFreeSlab->m_SubSlabData[iAlloc].m_Allocated.m_Type);

			CSubSlab *pSlab = new(pSlabAddress) CSubSlab();
			
			if (this->mc_EnableCallbacks)
				pSlab->f_OnFillFree(*this);

			DMibFastCheck(SlabIndex < mc_nSmallSizeSlabs);
			auto &Slabs = m_SmallSizeSlabs[SlabIndex];

			Slabs.f_Insert(pSlab->m_Params.m_Link);
				
			return pSlab;
		}
		
		template <typename t_CParams>
		template <mint tf_Size>
		inline_small void TCMemoryManagerArena<t_CParams>::fp_FreeSmall(void *_pAlloc, TCMemoryManagerSlab<t_CParams, 0> *_pSlab)
		{
			typedef TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> CSubSlab;

			CSubSlab *pSlab = fg_AlignDown((CSubSlab *)_pAlloc, t_CParams::mc_SubSlabSize);

			if (this->mc_EnableCallbacks)
				pSlab->f_OnFree(*this, _pAlloc);

			bool bWasFull;
			bool bIsFullyFree;
			pSlab->f_Free(_pAlloc, bWasFull, bIsFullyFree);

			if (bIsFullyFree)
			{
				pSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();
				CMemoryManagerSubSlab_Free *pFreeSubSlab = (CMemoryManagerSubSlab_Free *)pSlab;
				pFreeSubSlab = new(pFreeSubSlab) CMemoryManagerSubSlab_Free();
				_pSlab->m_FreeSubSlabs.f_UnsafeInsert(pFreeSubSlab);
				if (t_CParams::mc_DeferCleanup & EDeferCleanup_OneSizeBlocks)
					fp_SlabHasGarbage(_pSlab);
				else
					fp_FreeSmallSubSlabs(_pSlab);
				++_pSlab->m_nFreeSubSlabs;
				--_pSlab->m_nAllocatedSubSlabs;
			}
			else if (bWasFull)
			{
				pSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();

				m_SmallSizeSlabs[CSubSlab::mc_SmallSlabIndex].f_UnsafeInsert(pSlab->m_Params.m_Link);
			}
		}
		
		
		template <typename t_CParams>
		template <mint tf_Size>
		bool TCMemoryManagerArena<t_CParams>::fp_CheckFreeSmall(bool _bBreak)
		{
			bool bError = false;

			typedef TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> CSubSlab;

			auto &Slabs = m_SmallSizeSlabs[CSubSlab::mc_SmallSlabIndex];
			for (auto iSlab = Slabs.f_GetIterator(); iSlab; ++iSlab)
			{
				CSubSlab * pSlab = (CSubSlab *)&*iSlab;
				if (pSlab->f_CheckFree(*this, _bBreak))
					bError = true;
			}

			return bError;
		}

	}
}