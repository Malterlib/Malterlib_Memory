// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>
#include <Mib/Concurrency/ThreadSafeQueueAtomic>

namespace NMib::NMemory
{
	struct CCallstackMemoryReporter : public CGlobalReportMemory
	{
		struct CSize
		{
			CSize();

			zuint64 m_nAllocations;
			zuint64 m_nBytes;

			umint f_GetSize() const;

			CSize &operator += (CSize const &_Other);
			auto operator <=> (CSize const &_Other) const noexcept;
		};

		static CCallstackMemoryReporter *ms_pThis;

	public:
		CCallstackMemoryReporter();
		~CCallstackMemoryReporter();

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

		void f_Report(bool _bFullReport) override;

	private:
		void fp_Report(bool _bFullReport);

		struct CAllocator
		{
			NStr::CStrNonTracked m_Name;

			CAllocator();
			CAllocator(CAllocator const &_Other);
		};

	private:
		static constexpr umint mcp_MaxStackTraceDepth = 16;
		static constexpr umint mcp_ProcessQueueSize = (1 << 17);
		static constexpr umint mcp_ProcessQueueThreshold = (mcp_ProcessQueueSize / 2);

		struct COperation
		{
			umint m_MemoryAllocator;
			umint m_Address;
			umint m_Size;

			enum EOpType
			{
				EOpType_Alloc
				, EOpType_Resize
				, EOpType_Realloc
				, EOpType_Free
				, EOpType_Allocator
				, EOpType_Report
				, EOpType_FullReport
			} m_OpType;

			struct CCallstack
			{
				CMibCodeAddress m_Callstack[mcp_MaxStackTraceDepth];
				umint m_nCallstack;
			} m_Callstack;

			struct COldAlloc
			{
				umint m_Address;
				umint m_Size;
			} m_OldAlloc;

			NStr::CStrNonTracked m_AllocatorName;
		};

		void fp_PushToQueue(COperation const &_Op);
		void fp_PushMark();
		NContainer::TCThreadSafeQueueAtomic<COperation, CAllocator_NonTrackedHeap> mp_OperationQueue;

		struct CAllocationKey
		{
			CAllocationKey(umint _MemoryAllocator, umint _Address);

			umint m_Allocator;
			umint m_Address;

			auto operator <=> (CAllocationKey const &_Other) const noexcept = default;
		};

		NContainer::TCMap<umint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> mp_Allocators;

		NThread::CMutual mp_Lock;
		NAtomic::TCAtomic<umint> mp_LockRequests;

		NAtomic::TCAtomic<umint> mp_QueueSize;
		NThread::CEvent mp_QueueEvent;

		umint mp_nReports = 0;

	private:
		// Thread related

		NStorage::TCUniquePointer<NThread::CThreadObjectNonTracked, NMemory::CAllocator_NonTrackedHeap> mp_pThread;
		NThread::CLowLevelLock mp_ThreadLock;

		struct CCallstack
		{
			CCallstack();
			CCallstack(uint64 _Hash, CMibCodeAddress *_pStack, umint _nStack, umint _MemoryAllocator);

			uint64 m_Hash;

			CMibCodeAddress m_CallStack[mcp_MaxStackTraceDepth];
			umint m_nCallStack;

			umint m_Allocator;
			CAllocator *m_pAllocator;

			CSize m_Total;
		};

		NContainer::TCMap<uint64, CCallstack, CSort_Default, CAllocator_NonTrackedHeap> mp_Callstacks;
		NContainer::TCVector<CCallstack, NMemory::CAllocator_NonTrackedHeap> mp_Errors;

		CCallstack &fp_GetCallstack(umint _MemoryAllocator, umint _Hash, CMibCodeAddress *_pStack, umint _nStack);
		static NCryptography::CHashDigest_MD5 fsp_GetStackFingerprint(CMibCodeAddress *_pStack, umint _nStack);

		struct CAllocation
		{
			CAllocation(umint _Size, CCallstack *_pCallstack);

			umint m_Size;
			CCallstack *m_pCallstack;
			umint m_nIgnoreFree = 0;
		};

		NContainer::TCMap<CAllocationKey, CAllocation, CSort_Default, CAllocator_NonTrackedHeap> mp_Allocations;
		bool fp_RegisterAllocation(umint _MemoryAllocator, umint _Address, umint _Size, CCallstack *_pCallstack);
		CCallstack *fp_RemoveAllocation(umint _MemoryAllocator, umint _Address);

		void fp_StartupThread();
		void fp_ProcessQueue();
	};
}
