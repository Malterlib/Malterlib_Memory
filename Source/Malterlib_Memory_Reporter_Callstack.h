
#pragma once

#include <Mib/Core/Core>
#include <Mib/Concurrency/ThreadSafeQueueAtomic>

namespace NMib
{
	namespace NMem
	{
		class CCallstackMemoryReporter : public CGlobalReportMemory
		{
			enum
			{
				EMaxStackTraceDepth = 16,
				EProcessQueueSize = (1 << 17),
				EProcessQueueThreshold = (EProcessQueueSize/2),
			};

		public:
			struct CSize
			{
				CSize()
				{
				}
				zuint64 m_nAllocations;
				zuint64 m_nBytes;

				mint f_GetSize() const
				{
					return NContainer::TCMap<mint, CSize, CSort_Default, CAllocator_NonTrackedHeap>::fs_GetKey(*this);
				}

				CSize & operator += (CSize const& _Other)
				{
					m_nAllocations += _Other.m_nAllocations;
					m_nBytes += _Other.m_nBytes;
					return *this;
				}

				bint operator< (CSize const& _Other) const
				{
					return m_nBytes > _Other.m_nBytes;
				}
			};
			static CCallstackMemoryReporter *ms_pThis;
		public:

			CCallstackMemoryReporter();
			~CCallstackMemoryReporter();

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

		private:

			void fp_Report(bool _bFullReport);

			struct CAllocator
			{
				NStr::CStrNonTracked m_Name;

				CAllocator() {};

				CAllocator(CAllocator const& _Other)
					: m_Name(_Other.m_Name)
				{
					m_Name.f_MakeUnique();
				}
			};

		private:

			struct COperation
			{
				mint m_MemoryAllocator;
				mint m_Address;
				mint m_Size;
				
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

				struct Callstack
				{
					CMibCodeAddress m_Callstack[EMaxStackTraceDepth];
					mint m_nCallstack;
				} m_Callstack;

				struct OldAlloc
				{
					mint m_Address;
					mint m_Size;
				} m_OldAlloc;

				NStr::CStrNonTracked m_AllocatorName;
			};

			void fp_PushToQueue(COperation const& _Op);
			void fp_PushMark();
			NContainer::TCThreadSafeQueueAtomic<COperation, CAllocator_NonTrackedHeap> m_OperationQueue;
			
			struct CAllocationKey		
			{
				CAllocationKey(mint _MemoryAllocator, mint _Address)
					: m_Allocator(_MemoryAllocator)
					, m_Address(_Address)
				{
				}

				mint m_Allocator;
				mint m_Address;

				bool operator <(CAllocationKey const& _Other) const
				{
					if (m_Allocator < _Other.m_Allocator) 
						return true;
					else if (m_Allocator > _Other.m_Allocator)
						return false;

					return (m_Address < _Other.m_Address);
				}
			};
			NContainer::TCMap<mint, CAllocator, CSort_Default, CAllocator_NonTrackedHeap> m_Allocators;

			NThread::CMutual m_Lock;
			NAtomic::TCAtomic<mint> m_LockRequests;

			NAtomic::TCAtomic<mint> m_QueueSize;
			NThread::CEvent m_QueueEvent;

			mint m_nReports = 0;

		private:
			// Thread related

			NPtr::TCUniquePointer<NThread::CThreadObjectNonTracked, NMem::CAllocator_NonTrackedHeap> mp_pThread;
			NThread::CSpinLock mp_ThreadLock;

			struct CCallstack
			{
				CCallstack() {};

				CCallstack(uint64 _Hash, CMibCodeAddress *_lStack, mint _nStack, mint _MemoryAllocator)
					: m_Hash(_Hash)
					, m_nCallStack(_nStack)
					, m_Allocator(_MemoryAllocator)
				{
					NMem::fg_MemCopy(m_CallStack, _lStack, m_nCallStack * sizeof(CMibCodeAddress));
				}

				uint64 m_Hash;
			
				CMibCodeAddress m_CallStack[EMaxStackTraceDepth];
				mint m_nCallStack;
				
				mint m_Allocator;
				CAllocator* m_pAllocator;

				CSize m_Total;
			};
			NContainer::TCMap<uint64, CCallstack, CSort_Default, CAllocator_NonTrackedHeap> m_Callstacks;
			NContainer::TCVector<CCallstack, NMem::CAllocator_NonTrackedHeap> m_Errors;

			CCallstack& fp_GetCallstack(mint _MemoryAllocator, mint _Hash, CMibCodeAddress *_pStack, mint _nStack);
			static NDataProcessing::CHashDigest_MD5 fsp_GetStackFingerprint(CMibCodeAddress *_pStack, mint _nStack);


			struct CAllocation
			{
				CAllocation(mint _Size, CCallstack* _pCallstack)
					: m_Size(_Size)
					, m_pCallstack(_pCallstack)
				{
				}

				mint m_Size;
				CCallstack* m_pCallstack;
				mint m_nIgnoreFree = 0;
			};
			NContainer::TCMap<CAllocationKey, CAllocation, CSort_Default, CAllocator_NonTrackedHeap> m_Allocations;
			bool fp_RegisterAllocation(mint _MemoryAllocator, mint _Address, mint _Size, CCallstack* _pCallstack);
			CCallstack* fp_RemoveAllocation(mint _MemoryAllocator, mint _Address);


			void fp_StartupThread();
			void fp_ProcessQueue();
		};

	} // Namespace NMem
} // Namespace NMib
