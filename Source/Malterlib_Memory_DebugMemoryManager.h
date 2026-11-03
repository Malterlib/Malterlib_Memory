// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>
#include <Mib/Container/MapWithPool>

#define DMibConfig_Memory_CustomThreadLocal 2

#include "Malterlib_Memory_MemoryManager.hpp"

namespace NMib::NMemory
{
	using CDebugMemoryManagerUnderlying = int32;
	enum EDebugMemoryManager : int32
	{
		EDebugMemoryManager_None              = 0
		, EDebugMemoryManager_CheckUpper      = DMibBit(0) // Check for overwrites just above each allocation
		, EDebugMemoryManager_CheckRandom     = DMibBit(1) // Check for overwrites just below each allocation
		, EDebugMemoryManager_ProtectOnDemand = DMibBit(2) // Do not place any protection on guard pages until f_DemandProtection is called.
	};

	template <CDebugMemoryManagerUnderlying t_Options>
	struct TCDebugMemoryManager
	{
		TCDebugMemoryManager()
			: mp_Heap(CMemoryManagerConfig())
			, mp_Sequence(0)
			, mp_nFreeBlocks(0)
			, mp_bCheckUpper(0)
			, mp_nFreeMemBytes(0)
			, mp_bProtectOnDemand(false)
			, mp_bProtectionDemanded(false)
		{
			if constexpr (t_Options & EDebugMemoryManager_CheckUpper)
				mp_bCheckUpper = true;
			else if constexpr (t_Options & EDebugMemoryManager_CheckRandom)
			{
				NMib::NMisc::CAutoRandom Random;
				if (Random.f_GetValue<uint32>() & 1)
					mp_bCheckUpper = true;
				else
					mp_bCheckUpper = false;
			}

			auto CheckUpperStr = NSys::fg_Process_GetEnvironmentVariable_NonProtected(NStr::CFStr256("MalterlibMemoryOverwriteCheckUpper"));
			if (CheckUpperStr == "true")
				mp_bCheckUpper = true;
			else if (CheckUpperStr == "false")
				mp_bCheckUpper = false;

			// Use max 20 % of physical memory
			//mp_nMaxFreeMemory = NMib::NSys::fg_Process_GetPhysicalMemory() / 5;

#if DMibPPtrBits == 32
			mp_nMaxFreeMemory = umint(128) * uint64(1024) * uint64(1024);
#else
			mp_nMaxFreeMemory = uint64(4) * uint64(1024) * uint64(1024) * uint64(1024);
#endif
		}

		~TCDebugMemoryManager()
		{
			// Memory leaks
			//if (!mp_Blocks.f_IsEmpty())
			//	DMibPDebugBreak;

			DMibLock(mp_Lock);

			while (!mp_FreeBlocks.f_IsEmpty())
				fp_RemoveFreeBlock();

			mp_FreeBlocks.f_Clear();
		}

		void f_DestroyCleanupThreads()
		{
			mp_Heap.f_DestroyCleanupThreads();
		}

		CReportMemoryLightweight *f_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
		{
			return (CReportMemoryLightweight *)mp_Heap.f_SetCustomThreadLocal(0, _pMemoryReporter);
		}

#if DMibConfig_Memory_Shims_Lightweight
		EMemoryReportLightweightScopeFlag f_GetLightweightScopeFlags()
		{
			return (EMemoryReportLightweightScopeFlag)(umint)mp_Heap.f_GetCustomThreadLocal(1);
		}

		EMemoryReportLightweightScopeFlag f_SetLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
		{
			return (EMemoryReportLightweightScopeFlag)(umint)mp_Heap.f_SetCustomThreadLocal(1, (void *)(umint)_Flags);
		}

		EMemoryReportLightweightScopeFlag f_AddLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
		{
			return (EMemoryReportLightweightScopeFlag)(umint)mp_Heap.f_SetCustomThreadLocal(1, (void *)(umint)(f_GetLightweightScopeFlags() | _Flags));
		}
#endif

		NThread::CMutual &f_GetLock()
		{
			return mp_Lock;
		}

		void *f_AllocWithSize(umint &_Size, umint _Alignment)
		{
			_Alignment = fg_Max(umint(1 << mcp_MemoryManagerAlignment), _Alignment);
			umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();
			if (_Size == 0)
				_Size = 1;

			_Size = NPrivate::fg_AlignAllocationSizeOrThrow(_Size, _Alignment);
			umint UserNeededSize = NPrivate::fg_AlignAllocationSizeOrThrow(_Size, GranularityProtect);
			umint GuardSize = NPrivate::fg_AddAllocationSizeOrThrow(GranularityProtect, NPrivate::fg_AddAllocationSizeOrThrow(GranularityProtect, GranularityProtect));
			umint NeededSize = NPrivate::fg_AddAllocationSizeOrThrow(UserNeededSize, GuardSize);

			uint8 *pBlock = (uint8 *)mp_Heap.f_AllocAlignedWithSize(NeededSize, _Alignment);

			DMibLock(mp_Lock);

			CReportMemoryLightweight *pReporter = (CReportMemoryLightweight *)mp_Heap.f_GetCustomThreadLocal(0);
			if (pReporter)
				pReporter->f_Alloc(_Size);

			umint OriginalSize = _Size;

			uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
			uint8 *pRetBlock = pBlockStart + GranularityProtect;

			if (!mp_bProtectOnDemand || mp_bProtectionDemanded)
			{
				NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect, EProtect_Exec);
				//NSys::fg_Mem_VirtualProtect(pRetBlock, UserNeededSize, 0);
				NSys::fg_Mem_VirtualProtect(pRetBlock + UserNeededSize, GranularityProtect, EProtect_Exec);
			}

			if (mp_bCheckUpper)
				pRetBlock += UserNeededSize - _Size;

			CMemoryBlock &NewBlock = mp_Blocks[pRetBlock];
			NewBlock.m_pStart = pBlock;
			NewBlock.m_RealSize = NeededSize;
			NewBlock.m_AllocSize = _Size;
			NewBlock.m_OriginalSize = OriginalSize;

			return pRetBlock;
		}

		void f_Free(void *_pBlock, umint _Size)
		{
			if (!_pBlock)
				return;

			if (!_Size)
				DMibPDebugBreak;

			_Size = NPrivate::fg_AlignAllocationSizeOrThrow(_Size, umint(1 << mcp_MemoryManagerAlignment));
			return fp_Free(_pBlock, _Size);
		}

		void f_FreeNoSize(void *_pBlock)
		{
			fp_Free(_pBlock, 0);
		}

		umint f_Size(void const *_pBlock) const
		{
			if (!_pBlock)
				return 0;
			DMibLock(mp_Lock);
			uint8 *pBlock = (uint8 *)_pBlock;
			CMemoryBlock const *pMemBlock = mp_Blocks.f_FindEqual(pBlock);
			if (!pMemBlock)
				DMibPDebugBreak;

			return pMemBlock->m_AllocSize;
		}

		umint f_TrySize(void const *_pBlock) const
		{
			if (!_pBlock)
				return 0;
			DMibLock(mp_Lock);
			uint8 *pBlock = (uint8 *)_pBlock;
			CMemoryBlock const *pMemBlock = mp_Blocks.f_FindEqual(pBlock);
			if (pMemBlock)
				return pMemBlock->m_AllocSize;
			return 0;
		}

		umint f_Overhead(void const *_pBlock) const
		{
			if (!_pBlock)
				return 0;
			DMibLock(mp_Lock);
			umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();
			uint8 *pBlock = (uint8 *)_pBlock;
			CMemoryBlock const *pMemBlock = mp_Blocks.f_FindEqual(pBlock);
			if (!pMemBlock)
				DMibPDebugBreak;

			umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

			return UserNeededSize - pMemBlock->m_AllocSize + GranularityProtect * 2;
		}

		bool f_ContainsBlock(void *_pPtr)
		{
			DMibLock(mp_Lock);
			return mp_Heap.f_ContainsBlock(_pPtr);
			//mp_Blocks.f_FindEqual((uint8 const*)_pPtr) != nullptr;
		}

		void f_DemandProtection()
		{
			DMibLock(mp_Lock);

			if (mp_bProtectOnDemand && !mp_bProtectionDemanded)
			{
				umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

				for (auto BIter = mp_Blocks.f_GetIterator(); BIter; ++BIter)
				{
					CMemoryBlock *pMemBlock = BIter;

					uint8 *pBlock = pMemBlock->m_pStart;
					uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
					uint8 *pRetBlock = pBlockStart + GranularityProtect;

					umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

					NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect, EProtect_Exec);
					NSys::fg_Mem_VirtualProtect(pRetBlock + UserNeededSize, GranularityProtect, EProtect_Exec);
				}

				for (auto FIter = mp_FreeBlocks.f_GetIterator(); FIter; ++FIter)
				{
					CMemoryBlock *pMemBlock = FIter;

					umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

					uint8 *pBlock = pMemBlock->m_pStart;
					uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);

					NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect * 2 + UserNeededSize, EProtect_Exec);
				}

				mp_bProtectionDemanded = true;
			}
		}

		void f_Lock()
		{
			mp_Lock.f_Lock();
		}

		void f_Unlock()
		{
			mp_Lock.f_Unlock();
		}

		void f_PrepareFork()
		{
			mp_Lock.f_PrepareFork();
		}

		void f_ForkedChild()
		{
			mp_Lock.f_ForkedChild();
		}

		void f_ForkedParent()
		{
			mp_Lock.f_ForkedParent();
		}

	private:
		struct CMemoryBlock
		{
			uint8 *m_pStart;
			umint m_RealSize;
			umint m_AllocSize;
			umint m_OriginalSize;

			uint8 *f_GetUserAddress() const
			{
				return NContainer::TCMapWithPool<uint8 *, CMemoryBlock>::fs_GetKey(this);
			}
		};

		void fp_RemoveFreeBlock()
		{
			umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

			--mp_nFreeBlocks;
			CMemoryBlock *pMemBlock = mp_FreeBlocks.f_FindSmallest();
			umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);
			mp_nFreeMemBytes -= pMemBlock->m_RealSize;

			uint8 *pBlock = pMemBlock->m_pStart;
			uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
			if (!mp_bProtectOnDemand || mp_bProtectionDemanded)
				NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect * 2 + UserNeededSize, EProtect_ReadWrite);

			umint Size = pMemBlock->m_RealSize;

			mp_FreeBlocks.f_Remove(pMemBlock);

			{
				DMibUnlock(mp_Lock);
				mp_Heap.f_Free(pBlock, Size);
			}
		}

		void fp_Free(void *_pBlock, umint _Size)
		{
			if (!_pBlock)
				return;
			auto &Lock = mp_Lock;
			DMibLock(Lock);
			uint8 *pUserBlock = (uint8 *)_pBlock;
			CMemoryBlock *pMemBlock = mp_Blocks.f_FindEqual(pUserBlock);
			if (!pMemBlock)
				DMibPDebugBreak;

			if (_Size && _Size != pMemBlock->m_OriginalSize && _Size != pMemBlock->m_RealSize)
				DMibPDebugBreak; // Misreported size

			CReportMemoryLightweight *pReporter = (CReportMemoryLightweight *)mp_Heap.f_GetCustomThreadLocal(0);
			if (pReporter)
				pReporter->f_Free(_Size);

			umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

			umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

			mp_FreeBlocks[mp_Sequence++] = *pMemBlock;
			++mp_nFreeBlocks;
			mp_nFreeMemBytes += pMemBlock->m_RealSize;

			uint8 *pBlock = pMemBlock->m_pStart;
			uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
			uint8 *pRetBlock = pBlockStart + GranularityProtect;

			if (!mp_bProtectOnDemand || mp_bProtectionDemanded)
				NSys::fg_Mem_VirtualProtect(pRetBlock, UserNeededSize, EProtect_Exec);

			mp_Blocks.f_Remove(pMemBlock);

			while (mp_nFreeMemBytes > mp_nMaxFreeMemory)
				fp_RemoveFreeBlock();
		}

		umint mp_Sequence;
		NContainer::TCMapWithPool
			<
				uint8 *
				, CMemoryBlock
				, NMib::CSort_Default
				, NMib::NMemory::CAllocator_VirtualNoTrackingNoCommit
				, (2 * 1024 * 1024) / (sizeof(CMemoryBlock) + sizeof(void *))
				, NMib::NMemory::CPoolType_Growing
			>
			mp_Blocks
		;

		NContainer::TCMapWithPool
			<
				umint
				, CMemoryBlock
				, NMib::CSort_Default
				, NMib::NMemory::CAllocator_VirtualNoTrackingNoCommit
				, (2 * 1024 * 1024) / (sizeof(CMemoryBlock) + sizeof(umint))
				, NMib::NMemory::CPoolType_Growing
			>
			mp_FreeBlocks
		;
		umint mp_nFreeBlocks;
		umint mp_nFreeMemBytes;
		umint mp_nMaxFreeMemory;

		bool mp_bCheckUpper;

		bool mp_bProtectOnDemand;
		bool mp_bProtectionDemanded;

		mutable NThread::CMutual mp_Lock;

		// macOS needs 16 byte alignment to handle SSE
		static constexpr aint mcp_MemoryManagerAlignmentCalc = gc_HighestBitSet<sizeof(umint) * 2>;
		static constexpr aint mcp_MemoryManagerAlignment = mcp_MemoryManagerAlignmentCalc < 4 ? 4 : mcp_MemoryManagerAlignmentCalc;

		struct CMemoryManagerParamsOverrides : public CDefaultMemoryManagerParams
		{
			static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
			using CAllocator = NMemory::CAllocator_VirtualNoTracking;
		};

		struct CMemoryManagerParamsOverridesParams : public TCMemoryManagerParams<CMemoryManagerParamsOverrides>
		{
		};

		using CMemoryManager = NMemory::TCMemoryManager<CMemoryManagerParamsOverridesParams>;

		CMemoryManager mp_Heap;
	};
}
