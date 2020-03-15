// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#define DMibConfig_Memory_CustomThreadLocal 2
#include "Malterlib_Memory_MemoryManager.hpp"

namespace NMib::NMemory
{
	using CDebugMemoryManagerUnderlaying = int32;
	enum EDebugMemoryManager : int32
	{
		EDebugMemoryManager_None = 0,
		EDebugMemoryManager_CheckUpper = DMibBit(0),			// Check for overwrites just above each allocation
		EDebugMemoryManager_CheckRandom = DMibBit(1),			// Check for overwrites just below each allocation
		EDebugMemoryManager_ProtectOnDemand = DMibBit(2),		// Do not place any protection on guard pages until f_DemandProtection is called.
	};
	template <CDebugMemoryManagerUnderlaying t_Options>
	class TCDebugMemoryManager
	{
		class CMemoryBlock
		{
		public:
			uint8 *m_pStart;
			mint m_RealSize;
			mint m_AllocSize;
			mint m_OriginalSize;

			uint8 *f_GetUserAddress() const
			{
				return NContainer::TCMapWithPool<uint8 *, CMemoryBlock>::fs_GetKey(this);
			}

		};

		mint m_Sequence;
		NContainer::TCMapWithPool
			<
				uint8 *
				, CMemoryBlock
				, NMib::CSort_Default
				, NMib::NMemory::CAllocator_VirtualNoTrackingNoCommit
				, (2*1024*1024) / (sizeof(CMemoryBlock) + sizeof(void *))
				, NMib::NMemory::CPoolType_Growing
			> m_Blocks
		;

		NContainer::TCMapWithPool
			<
				mint
				, CMemoryBlock
				, NMib::CSort_Default
				, NMib::NMemory::CAllocator_VirtualNoTrackingNoCommit
				, (2*1024*1024) / (sizeof(CMemoryBlock) + sizeof(mint))
				, NMib::NMemory::CPoolType_Growing
			> m_FreeBlocks
		;
		mint m_nFreeBlocks;
		mint m_nFreeMemBytes;
		mint m_nMaxFreeMemory;

		bool m_bCheckUpper;

		bool m_bProtectOnDemand;
		bool m_bProtectionDemanded;

		mutable NThread::CMutual m_Lock;

		enum
		{
//#ifdef DPlatformFamily_OSX
			// OSX needs 16 byte alignment to handle SSE
			EMemoryManagerAlignmentCalc = DMibGetHighestBitSet(sizeof(mint)*2) - 1
			, EMemoryManagerAlignment = EMemoryManagerAlignmentCalc < 4 ? 4 : EMemoryManagerAlignmentCalc
//#else
//			EMemoryManagerAlignment = DMibGetHighestBitSet(sizeof(mint)*2) - 1
//#endif
		};

		struct CMemoryManagerParams : public NMemory::CDefaultMemoryManagerParams
		{
			static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
			typedef NMemory::CAllocator_VirtualNoTracking CAllocator;
		};

		typedef NMemory::TCMemoryManager<CMemoryManagerParams> CMemoryManager;

		CMemoryManager m_Heap;

	public:

		TCDebugMemoryManager()
			: m_Heap(CMemoryManagerConfig())
			, m_Sequence(0)
			, m_nFreeBlocks(0)
			, m_bCheckUpper(0)
			, m_nFreeMemBytes(0)
			, m_bProtectOnDemand(false)
			, m_bProtectionDemanded(false)
		{
			if constexpr (t_Options & EDebugMemoryManager_CheckUpper)
				m_bCheckUpper = true;
			else if constexpr (t_Options & EDebugMemoryManager_CheckRandom)
			{
				NMib::NMisc::CAutoRandom Random;
				if (Random.f_GetValue<uint32>() & 1)
					m_bCheckUpper = true;
				else
					m_bCheckUpper = false;
			}

			auto CheckUpperStr = NSys::fg_Process_GetEnvironmentVariable_NonProtected(NStr::CFStr256("MalterlibMemoryOverwriteCheckUpper"));
			if (CheckUpperStr == "true")
				m_bCheckUpper = true;
			else if (CheckUpperStr == "false")
				m_bCheckUpper = false;

			// Use max 20 % of physical memory
			//m_nMaxFreeMemory = NMib::NSys::fg_Process_GetPhysicalMemory() / 5;

#if DMibPPtrBits == 32
			m_nMaxFreeMemory = mint(128)*uint64(1024)*uint64(1024);
#else
			m_nMaxFreeMemory = uint64(4)*uint64(1024)*uint64(1024)*uint64(1024);
#endif
		}

		void fp_RemoveFreeBlock()
		{
			mint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

			--m_nFreeBlocks;
			CMemoryBlock *pMemBlock = m_FreeBlocks.f_FindSmallest();
			mint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);
			m_nFreeMemBytes -= pMemBlock->m_RealSize;

			uint8 *pBlock = pMemBlock->m_pStart;
			uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
			if (!m_bProtectOnDemand || m_bProtectionDemanded)
				NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect*2 + UserNeededSize, EProtect_ReadWrite);

			mint Size = pMemBlock->m_RealSize;

			m_FreeBlocks.f_Remove(pMemBlock);

			{
				DMibUnlock(m_Lock);
				m_Heap.f_Free(pBlock, Size);
			}
		}

		~TCDebugMemoryManager()
		{
			// Memory leaks
			//if (!m_Blocks.f_IsEmpty())
			//	DMibPDebugBreak;

			DMibLock(m_Lock);

			while (!m_FreeBlocks.f_IsEmpty())
				fp_RemoveFreeBlock();

			m_FreeBlocks.f_Clear();
		}

		CReportMemoryLightweight *f_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
		{
			return (CReportMemoryLightweight *)m_Heap.f_SetCustomThreadLocal(0, _pMemoryReporter);
		}

#if DMibConfig_Memory_Shims_Lightweight
		EMemoryReportLightweightScopeFlag f_GetLightweightScopeFlags()
		{
			return (EMemoryReportLightweightScopeFlag)(mint)m_Heap.f_GetCustomThreadLocal(1);
		}

		EMemoryReportLightweightScopeFlag f_SetLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
		{
			return (EMemoryReportLightweightScopeFlag)(mint)m_Heap.f_SetCustomThreadLocal(1, (void *)(mint)_Flags);
		}

		EMemoryReportLightweightScopeFlag f_AddLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
		{
			return (EMemoryReportLightweightScopeFlag)(mint)m_Heap.f_SetCustomThreadLocal(1, (void *)(mint)(f_GetLightweightScopeFlags() | _Flags));
		}
#endif

		NThread::CMutual &f_GetLock()
		{
			return m_Lock;
		}

		void *f_AllocWithSize(mint &_Size, mint _Alignment)
		{
			_Alignment = fg_Max(mint(1 << EMemoryManagerAlignment), _Alignment);
			mint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();
			if (_Size == 0)
				_Size = 1;

			_Size = fg_AlignUp(_Size, _Alignment);
			mint UserNeededSize = fg_AlignUp(_Size, GranularityProtect);
			mint NeededSize = UserNeededSize + GranularityProtect*3;

			uint8 *pBlock = (uint8 *)m_Heap.f_AllocAlignedWithSize(NeededSize, _Alignment);

			DMibLock(m_Lock);

			CReportMemoryLightweight *pReporter = (CReportMemoryLightweight *)m_Heap.f_GetCustomThreadLocal(0);
			if (pReporter)
				pReporter->f_Alloc(_Size);

			mint OriginalSize = _Size;

			uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
			uint8 *pRetBlock = pBlockStart + GranularityProtect;

			if (!m_bProtectOnDemand || m_bProtectionDemanded)
			{
				NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect, EProtect_Exec);
				//NSys::fg_Mem_VirtualProtect(pRetBlock, UserNeededSize, 0);
				NSys::fg_Mem_VirtualProtect(pRetBlock + UserNeededSize, GranularityProtect, EProtect_Exec);
			}

			if (m_bCheckUpper)
				pRetBlock += UserNeededSize - _Size;

			CMemoryBlock &NewBlock = m_Blocks[pRetBlock];
			NewBlock.m_pStart = pBlock;
			NewBlock.m_RealSize = NeededSize;
			NewBlock.m_AllocSize = _Size;
			NewBlock.m_OriginalSize = OriginalSize;

			return pRetBlock;
		}

		void f_Free(void *_pBlock, mint _Size)
		{
			if (!_pBlock)
				return;

			if (!_Size)
				DMibPDebugBreak;

			_Size = fg_AlignUp(_Size, mint(1 << EMemoryManagerAlignment));
			return fp_Free(_pBlock, _Size);
		}
		void f_FreeNoSize(void *_pBlock)
		{
			fp_Free(_pBlock, 0);
		}

		void fp_Free(void *_pBlock, mint _Size)
		{
			if (!_pBlock)
				return;
			auto &Lock = m_Lock;
			DMibLock(Lock);
			uint8 *pUserBlock = (uint8 *)_pBlock;
			CMemoryBlock *pMemBlock = m_Blocks.f_FindEqual(pUserBlock);
			if (!pMemBlock)
				DMibPDebugBreak;

			if (_Size && _Size != pMemBlock->m_OriginalSize && _Size != pMemBlock->m_RealSize)
				DMibPDebugBreak; // Misreported size

			CReportMemoryLightweight *pReporter = (CReportMemoryLightweight *)m_Heap.f_GetCustomThreadLocal(0);
			if (pReporter)
				pReporter->f_Free(_Size);

			mint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

			mint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

			m_FreeBlocks[m_Sequence++] = *pMemBlock;
			++m_nFreeBlocks;
			m_nFreeMemBytes += pMemBlock->m_RealSize;

			uint8 *pBlock = pMemBlock->m_pStart;
			uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
			uint8 *pRetBlock = pBlockStart + GranularityProtect;

			if (!m_bProtectOnDemand || m_bProtectionDemanded)
				NSys::fg_Mem_VirtualProtect(pRetBlock, UserNeededSize, EProtect_Exec);

			m_Blocks.f_Remove(pMemBlock);

			while (m_nFreeMemBytes > m_nMaxFreeMemory)
				fp_RemoveFreeBlock();
		}

		mint f_Size(const void *_pBlock) const
		{
			if (!_pBlock)
				return 0;
			DMibLock(m_Lock);
			uint8 *pBlock = (uint8 *)_pBlock;
			CMemoryBlock const *pMemBlock = m_Blocks.f_FindEqual(pBlock);
			if (!pMemBlock)
				DMibPDebugBreak;

			return pMemBlock->m_AllocSize;
		}

		mint f_TrySize(const void *_pBlock) const
		{
			if (!_pBlock)
				return 0;
			DMibLock(m_Lock);
			uint8 *pBlock = (uint8 *)_pBlock;
			CMemoryBlock const *pMemBlock = m_Blocks.f_FindEqual(pBlock);
			if (pMemBlock)
				return pMemBlock->m_AllocSize;
			return 0;
		}

		mint f_Overhead(const void *_pBlock) const
		{
			if (!_pBlock)
				return 0;
			DMibLock(m_Lock);
			mint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();
			uint8 *pBlock = (uint8 *)_pBlock;
			CMemoryBlock const *pMemBlock = m_Blocks.f_FindEqual(pBlock);
			if (!pMemBlock)
				DMibPDebugBreak;

			mint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

			return UserNeededSize - pMemBlock->m_AllocSize + GranularityProtect * 2;
		}

		bool f_ContainsBlock(void* _pPtr)
		{
			DMibLock(m_Lock);
			return m_Heap.f_ContainsBlock(_pPtr);
			//m_Blocks.f_FindEqual((uint8 const*)_pPtr) != nullptr;
		}

		void f_DemandProtection()
		{
			DMibLock(m_Lock);

			if (m_bProtectOnDemand && !m_bProtectionDemanded)
			{
				mint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

				for (auto BIter = m_Blocks.f_GetIterator()
					;BIter
					;++BIter)
				{
					CMemoryBlock* pMemBlock = BIter;

					uint8 *pBlock = pMemBlock->m_pStart;
					uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
					uint8 *pRetBlock = pBlockStart + GranularityProtect;

					mint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

					NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect, EProtect_Exec);
					NSys::fg_Mem_VirtualProtect(pRetBlock + UserNeededSize, GranularityProtect, EProtect_Exec);
				}

				for (auto FIter = m_FreeBlocks.f_GetIterator()
					;FIter
					;++FIter)
				{
					CMemoryBlock* pMemBlock = FIter;

					mint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

					uint8 *pBlock = pMemBlock->m_pStart;
					uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);

					NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect*2 + UserNeededSize, EProtect_Exec);
				}

				m_bProtectionDemanded = true;
			}
		}

		void f_Lock()
		{
			m_Lock.f_Lock();
		}

		void f_Unlock()
		{
			m_Lock.f_Unlock();
		}

		void f_PrepareFork()
		{
			m_Lock.f_PrepareFork();
		}

		void f_ForkedChild()
		{
			m_Lock.f_ForkedChild();
		}

		void f_ForkedParent()
		{
			m_Lock.f_ForkedParent();
		}

	};
}
