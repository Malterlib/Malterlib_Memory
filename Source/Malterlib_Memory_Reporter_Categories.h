// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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

		void f_Report(bool _bFullReport) override;

	public:

		void fp_ReportAlloc(umint _Size, void *_pAllocationInfo);
		void fp_ReportFree(umint _Size, void const *_pAllocationInfo);

		struct CThreadLocal
		{
			CMemoryCategory *m_pCurrentCategory;

			CThreadLocal();
		};

		NThread::TCThreadLocal<CThreadLocal, CAllocator_NonTrackedHeap> m_ThreadLocal;
	};
}
