// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	struct CStatsMemoryReporter : public CGlobalReportMemory
	{
		struct CSize
		{
			CSize();

			zuint64 m_nAllocations;
			zuint64 m_nBytes;
			zuint64 m_nBytesRequested;
			fp64 m_nBytesOverhead;
			zuint64 m_Cycles;
			zuint64 m_CyclesFree;
			DMibListLinkDS_Link(CSize, m_Link);
			umint f_GetSize() const;

			CSize &operator += (CSize const &_Other);
		};

		static CStatsMemoryReporter *ms_pThis;

	public:
		CStatsMemoryReporter();
		~CStatsMemoryReporter();

		void f_AllocatorName(umint _MemoryAllocator, ch8 const *_pAllocatorName) override;
		void f_AllocatorDelete(umint _MemoryAllocator) override;
		void f_ScopeEnter(umint _MemoryAllocator) override;
		void f_ScopeExit(umint _MemoryAllocator) override;
		void f_Alloc(umint _MemoryAllocator, umint _Address, umint _RequestedAlignment, umint _RequestedSize, umint _ReturnedSize, fp32 _nBytesOverhead, void *_pAllocationInfo) override;
		void f_Resize
			(
				umint _MemoryAllocator
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			override
		;
		void f_Realloc
			(
				umint _MemoryAllocator
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			override
		;

		void f_Free(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo) override;

		void f_GetSize(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo) override;
		void f_Protect(umint _MemoryAllocator, umint _Address, umint _Size, uaint _Protect) override;
		void f_Commit(umint _MemoryAllocator, umint _Address, umint _Size) override;
		void f_Decommit(umint _MemoryAllocator, umint _Address, umint _Size) override;

	private:
		struct CAllocator
		{
			NStr::CStrNonTracked m_Name;

			CSize m_Total;

			NContainer::TCMap<umint, CSize, CSort_Default, CAllocator_NonTrackedHeap> m_Sizes;
		};

		struct CThreadLocal
		{
			umint m_CyclesDepth;
			uint64 m_CyclesStart[16]; // 16 should be more than enough
			NContainer::TCMap<umint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> m_Allocators;
			CThreadLocal();
			~CThreadLocal();
		};

	private:
		NThread::TCThreadLocal<CThreadLocal, NMemory::CAllocator_NonTrackedHeap, NThread::EThreadLocalFlag_FastThreadLocal> mp_ThreadLocal;

		NThread::CMutual mp_Lock;
		NContainer::TCMap<umint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> mp_GlobalAllocators;
	};
}
