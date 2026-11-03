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
		TCDebugMemoryManager();
		~TCDebugMemoryManager();

		void f_DestroyCleanupThreads();
		CReportMemoryLightweight *f_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter);

#if DMibConfig_Memory_Shims_Lightweight
		EMemoryReportLightweightScopeFlag f_GetLightweightScopeFlags();
		EMemoryReportLightweightScopeFlag f_SetLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags);
		EMemoryReportLightweightScopeFlag f_AddLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags);
#endif

		NThread::CMutual &f_GetLock();
		void *f_AllocWithSize(umint &_Size, umint _Alignment);
		void f_Free(void *_pBlock, umint _Size);
		void f_FreeNoSize(void *_pBlock);
		umint f_Size(void const *_pBlock) const;
		umint f_TrySize(void const *_pBlock) const;
		umint f_Overhead(void const *_pBlock) const;
		bool f_ContainsBlock(void *_pPtr);
		void f_DemandProtection();
		void f_Lock();
		void f_Unlock();
		void f_PrepareFork();
		void f_ForkedChild();
		void f_ForkedParent();

	private:
		struct CMemoryBlock
		{
			uint8 *m_pStart;
			umint m_RealSize;
			umint m_AllocSize;
			umint m_OriginalSize;

			uint8 *f_GetUserAddress() const;
		};

		void fp_RemoveFreeBlock();
		void fp_Free(void *_pBlock, umint _Size);

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

#include "Malterlib_Memory_DebugMemoryManager.hpp"
