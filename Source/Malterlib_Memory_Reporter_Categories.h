
#pragma once

#include <Mib/Core/Core>

#include "Malterlib_Memory_Reporter_CategoriesInterface.h"

namespace NMib::NMemory
{
	class CCategoriesMemoryReporter : public CGlobalReportMemory
	{
	public:
		static CCategoriesMemoryReporter *ms_pThis;
	public:

		CCategoriesMemoryReporter();
		~CCategoriesMemoryReporter();

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

		void f_Report(bool _bFullReport) override;

	public:

		void fp_ReportAlloc(mint _Size, void *_pAllocationInfo);
		void fp_ReportFree(mint _Size, void const *_pAllocationInfo);

		struct CThreadLocal
		{
			CMemoryCategory *m_pCurrentCategory;

			CThreadLocal();
		};

		NThread::TCThreadLocal<CThreadLocal, CAllocator_NonTrackedHeap> m_ThreadLocal;
	};
}
