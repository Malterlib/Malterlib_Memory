// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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
			mint f_GetSize() const
			{
				return NContainer::TCMap<mint, CSize, CSort_Default, CAllocator_NonTrackedHeap>::fs_GetKey(*this);
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

		void f_AllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName) override;
		void f_AllocatorDelete(mint _MemoryAllocator) override;
		void f_ScopeEnter(mint _MemoryAllocator) override;
		void f_ScopeExit(mint _MemoryAllocator) override;
		void f_Alloc
			(
				mint _MemoryAllocator
				, mint _Address
				, mint _RequestedAlignment
				, mint _RequestedSize
				, mint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) override
		;
		void f_Resize
			(
				mint _MemoryAllocator
				, mint _OldAddress
				, mint _OldSize
				, void const *_pOldAllocationInfo
				, mint _Address
				, mint _RequestedAlignment
				, mint _RequestedSize
				, mint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) override
		;
		void f_Realloc
			(
				mint _MemoryAllocator
				, mint _OldAddress
				, mint _OldSize
				, void const *_pOldAllocationInfo
				, mint _Address
				, mint _RequestedAlignment
				, mint _RequestedSize
				, mint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) override
		;

		void f_Free(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo) override;

		void f_GetSize(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo) override;
		void f_Protect(mint _MemoryAllocator, mint _Address, mint _Size, uaint _Protect) override;
		void f_Commit(mint _MemoryAllocator, mint _Address, mint _Size) override;
		void f_Decommit(mint _MemoryAllocator, mint _Address, mint _Size) override;

	private:
		struct CAllocator
		{
			NStr::CStrNonTracked m_Name;

			CSize m_Total;

			NContainer::TCMap<mint, CSize, CSort_Default, CAllocator_NonTrackedHeap> m_Sizes;
		};

		struct CThreadLocal
		{
			mint m_CyclesDepth;
			uint64 m_CyclesStart[16]; // 16 should be more than enough
			NContainer::TCMap<mint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> m_Allocators;
			CThreadLocal()
				: m_CyclesDepth(0)
			{
			}
			~CThreadLocal();
		};

	private:
		NThread::TCThreadLocal<CThreadLocal, NMemory::CAllocator_NonTrackedHeap, NThread::EThreadLocalFlag_FastThreadLocal> m_ThreadLocal;

		NThread::CMutual m_Lock;
		NContainer::TCMap<mint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> m_GlobalAllocators;
	};
}
