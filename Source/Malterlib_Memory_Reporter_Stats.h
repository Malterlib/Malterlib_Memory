// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	class CStatsMemoryReporter : public CGlobalReportMemory
	{
	public:
		struct CSize
		{
			CSize()
				: m_nBytesOverhead(0.0)
			{
			}
			zuint64 m_nAllocations;
			zuint64 m_nBytes;
			zuint64 m_nBytesRequested;
			fp64 m_nBytesOverhead;
			zuint64 m_Cycles;
			zuint64 m_CyclesFree;
			DMibListLinkDS_Link(CSize, m_Link);
			umint f_GetSize() const
			{
				return NContainer::TCMap<umint, CSize, CSort_Default, CAllocator_NonTrackedHeap>::fs_GetKey(*this);
			}

			CSize & operator += (CSize const& _Other)
			{
				m_nAllocations += _Other.m_nAllocations;
				m_nBytes += _Other.m_nBytes;
				m_nBytesRequested += _Other.m_nBytesRequested;
				m_nBytesOverhead += _Other.m_nBytesOverhead;
				m_Cycles += _Other.m_Cycles;
				m_CyclesFree += _Other.m_CyclesFree;
				return *this;
			}
		};

		static CStatsMemoryReporter *ms_pThis;

	public:
		CStatsMemoryReporter();
		~CStatsMemoryReporter();

		void f_AllocatorName(umint _MemoryAllocator, ch8 const* _pAllocatorName) override;
		void f_AllocatorDelete(umint _MemoryAllocator) override;
		void f_ScopeEnter(umint _MemoryAllocator) override;
		void f_ScopeExit(umint _MemoryAllocator) override;
		void f_Alloc
			(
				umint _MemoryAllocator
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) override
		;
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
			) override
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
			) override
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
			CThreadLocal()
				: m_CyclesDepth(0)
			{
			}
			~CThreadLocal();
		};

	private:
		NThread::TCThreadLocal<CThreadLocal, NMemory::CAllocator_NonTrackedHeap, NThread::EThreadLocalFlag_FastThreadLocal> m_ThreadLocal;

		NThread::CMutual m_Lock;
		NContainer::TCMap<umint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> m_GlobalAllocators;
	};
}
