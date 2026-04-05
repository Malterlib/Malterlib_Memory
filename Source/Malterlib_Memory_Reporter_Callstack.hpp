// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_Reporter_Callstack.h"

namespace NMib::NMemory
{
	CCallstackMemoryReporter *CCallstackMemoryReporter::ms_pThis = nullptr;

	CCallstackMemoryReporter::CCallstackMemoryReporter()
	{
		if (ms_pThis != nullptr)
			DMibPDebugBreak;
		ms_pThis = this;

		fg_ReportMemoryGloballyTo(this);

		fp_StartupThread();
	}

	void CCallstackMemoryReporter::f_Report(bool _bFullReport)
	{
		COperation Alloc;
		Alloc.m_OpType = _bFullReport ? COperation::EOpType_FullReport : COperation::EOpType_Report;
		fp_PushToQueue(Alloc);
	}

	void CCallstackMemoryReporter::fp_Report(bool _bFullReport)
	{
		m_LockRequests.f_FetchAdd(1);
		DMibLock(ms_pThis->m_Lock);
		auto CleanUpLock = fg_OnScopeExit([&]
			{
				m_LockRequests.f_FetchSub(1);
			}
		);

		NStr::CStrNonTracked ProgramDir = NFile::CFile::fs_GetProgramDirectoryNonTracked();

		{
			NFile::CFile::fs_CreateDirectory(ProgramDir + "/CrashDumps");
			// Export TSV
			NFile::CFile ExportFile;
			ExportFile.f_Open(ProgramDir + "/CrashDumps/MemoryReport.tsv", NFile::EFileOpen_Write | NFile::EFileOpen_DontTruncate);
			ExportFile.f_SetPositionFromEnd(0);

			for (auto const& Callstack : m_Callstacks)
			{
				if (!Callstack.m_Total.m_nBytes)
					continue;

				NStr::CStrNonTracked Data = NStr::fg_Format<NStr::CStrNonTracked>
					(
						"{}\t{}\t{}\t{}\t{}\n"
						, m_nReports
						, Callstack.m_Hash
						, Callstack.m_Allocator
						, Callstack.m_Total.m_nBytes
						, Callstack.m_Total.m_nAllocations
					)
				;
				ExportFile.f_Write(Data.f_GetStr(), Data.f_GetLen());
			}

			++m_nReports;
			ExportFile.f_Close();
		}

		{
			// Export TSV
			NFile::CFile ExportFile;
			ExportFile.f_Open(ProgramDir + "/CrashDumps/Allocators.tsv", NFile::EFileOpen_Write);


			for (auto iAllocator = m_Allocators.f_GetIterator(); iAllocator; ++iAllocator)
			{
				NStr::CStrNonTracked Data = NStr::fg_Format<NStr::CStrNonTracked>("{}\t{}\n",  iAllocator.f_GetKey(), iAllocator->m_Name);
				ExportFile.f_Write(Data.f_GetStr(), Data.f_GetLen());
			}

			++m_nReports;
			ExportFile.f_Close();
		}

		if (!_bFullReport)
			return;

		// These variables will be included in dump explicitly
		NContainer::TCVector<CCallstack, CAllocator_NonTrackedHeap>	Callstacks;
		NContainer::TCVector<CAllocator, CAllocator_NonTrackedHeap>	Allocators;
		auto & Errors = m_Errors;

		for (auto const& Callstack : m_Callstacks)
			Callstacks.f_Insert(Callstack);

		Callstacks.f_Sort
			(
				[&](CCallstack const& _Left, CCallstack const& _Right)
				{
					return _Left.m_Total < _Right.m_Total;
				}
			)
		;

		Allocators.f_SetLen(m_Allocators.f_GetLen());
		NContainer::TCMap<umint, CAllocator *, CSort_Default, CAllocator_NonTrackedHeap> AllocatorMap;

		umint iRow = 0;
		for (auto iAllocator = m_Allocators.f_GetIterator(); iAllocator; ++iAllocator, ++iRow)
		{
			auto & Dest = Allocators[iRow];
			Dest = *iAllocator;
			AllocatorMap[iAllocator.f_GetKey()] = &Dest;
		}

		for (auto & Callstack : Callstacks)
			Callstack.m_pAllocator = AllocatorMap[Callstack.m_Allocator];

		NMib::NContainer::TCVector<void*, NMemory::CAllocator_NonTrackedHeap> Locations;
		NMib::NContainer::TCVector<umint, NMib::NMemory::CAllocator_NonTrackedHeap> Sizes;

		Locations.f_Insert(Callstacks.f_GetArray());
		Sizes.f_Insert(Callstacks.f_GetLen() * sizeof(CCallstack));

		if (!Errors.f_IsEmpty())
		{
			Locations.f_Insert(Errors.f_GetArray());
			Sizes.f_Insert(Errors.f_GetLen() * sizeof(CCallstack));
		}

		Locations.f_Insert(Allocators.f_GetArray());
		Sizes.f_Insert(Allocators.f_GetLen() * sizeof(CAllocator));

		for (auto & Allocator : Allocators)
		{
			if (!Allocator.m_Name.f_IsEmpty())
			{
				Locations.f_Insert((void*)Allocator.m_Name.f_GetStr());
				Sizes.f_Insert(Allocator.m_Name.f_GetLen() + 1);
			}
		}

		NSys::fg_Debug_GenerateMemoryDump(Locations, Sizes);
	}


	CCallstackMemoryReporter::~CCallstackMemoryReporter()
	{
		fg_ReportMemoryGloballyTo(nullptr);

		if (mp_pThread)
		{
			mp_pThread->f_Stop();
			mp_pThread.f_Clear();
		}

		ms_pThis = nullptr;
	}

	void CCallstackMemoryReporter::f_AllocatorName(umint _MemoryAllocator, ch8 const* _pAllocatorName)
	{
		COperation Allocator;
		Allocator.m_OpType = COperation::EOpType_Allocator;

		Allocator.m_MemoryAllocator = _MemoryAllocator;
		Allocator.m_AllocatorName = _pAllocatorName;

		fp_PushToQueue(Allocator);
	}

	void CCallstackMemoryReporter::f_AllocatorDelete(umint _MemoryAllocator)
	{
	}

	void CCallstackMemoryReporter::f_ScopeEnter(umint _MemoryAllocator)
	{
	}

	void CCallstackMemoryReporter::f_ScopeExit(umint _MemoryAllocator)
	{
	}

	void CCallstackMemoryReporter::f_Alloc
		(
			umint _MemoryAllocator
			, umint _Address
			, umint _RequestedAlignment
			, umint _RequestedSize
			, umint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
		COperation Alloc;
		Alloc.m_MemoryAllocator = _MemoryAllocator;
		Alloc.m_Address = _Address;
		Alloc.m_Size = _ReturnedSize;

		Alloc.m_OpType = COperation::EOpType_Alloc;
		Alloc.m_Callstack.m_nCallstack = NSys::fg_System_GetStackTrace(Alloc.m_Callstack.m_Callstack, EMaxStackTraceDepth);
		fp_PushToQueue(Alloc);
	}

	void CCallstackMemoryReporter::f_Resize
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
	{
		COperation Resize;
		Resize.m_MemoryAllocator = _MemoryAllocator;
		Resize.m_Address = _Address;
		Resize.m_Size = _ReturnedSize;

		Resize.m_OpType = COperation::EOpType_Resize;
		Resize.m_OldAlloc.m_Address = _OldAddress;
		Resize.m_OldAlloc.m_Size = _OldSize;
		Resize.m_Callstack.m_nCallstack = NSys::fg_System_GetStackTrace(Resize.m_Callstack.m_Callstack, EMaxStackTraceDepth);
		fp_PushToQueue(Resize);
	}

	void CCallstackMemoryReporter::f_Realloc
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
	{
		COperation Realloc;
		Realloc.m_MemoryAllocator = _MemoryAllocator;
		Realloc.m_Address = _Address;
		Realloc.m_Size = _ReturnedSize;

		Realloc.m_OpType = COperation::EOpType_Realloc;
		Realloc.m_OldAlloc.m_Address = _OldAddress;
		Realloc.m_OldAlloc.m_Size = _OldSize;
		Realloc.m_Callstack.m_nCallstack = NSys::fg_System_GetStackTrace(Realloc.m_Callstack.m_Callstack, EMaxStackTraceDepth);
		fp_PushToQueue(Realloc);
	}

	void CCallstackMemoryReporter::f_Free(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo)
	{
		COperation Free;
		Free.m_MemoryAllocator = _MemoryAllocator;
		Free.m_Address = _Address;
		Free.m_Size = _Size;

		Free.m_OpType = COperation::EOpType_Free;
		Free.m_OldAlloc.m_Address = _Address;
		Free.m_OldAlloc.m_Size = _Size;
		Free.m_Callstack.m_nCallstack = NSys::fg_System_GetStackTrace(Free.m_Callstack.m_Callstack, EMaxStackTraceDepth);
		fp_PushToQueue(Free);
	}

	static umint const gc_QueueBitReset = umint(1) << (sizeof(umint*) * 8 - 1);
	static umint const gc_QueueBitResetFinished = umint(1) << (sizeof(umint*) * 8 - 2);
	static umint const gc_QueueBitInv = ~(gc_QueueBitReset | gc_QueueBitResetFinished);
	void CCallstackMemoryReporter::fp_PushToQueue(COperation const& _Op)
	{
		umint QueueSize = ++m_QueueSize;
		umint SizeAnd = QueueSize & gc_QueueBitInv;

		if (SizeAnd == EProcessQueueSize)
		{
			QueueSize = m_QueueSize.f_FetchOr(gc_QueueBitReset);
			if ((QueueSize & gc_QueueBitReset) == 0)
			{
				while ((QueueSize & gc_QueueBitResetFinished))
					QueueSize = m_QueueSize.f_Load();

				m_QueueEvent.f_ResetSignaled();
				QueueSize = m_QueueSize.f_FetchOr(gc_QueueBitResetFinished);
				SizeAnd = QueueSize & gc_QueueBitInv;
			}
		}

		if (SizeAnd == EProcessQueueThreshold)
		{
			while (!mp_pThread)
				fp_StartupThread();

			mp_pThread->m_EventWantQuit.f_Signal();
		}

		if (SizeAnd >= EProcessQueueSize)
		{
			while (SizeAnd > EProcessQueueThreshold)
			{
				m_QueueEvent.f_Wait();
				QueueSize = m_QueueSize.f_Load();
				SizeAnd = QueueSize & gc_QueueBitInv;
			}
		}

		m_OperationQueue.f_Push(_Op);

	}

	void CCallstackMemoryReporter::f_GetSize(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo)
	{
	}

	void CCallstackMemoryReporter::f_Protect(umint _MemoryAllocator, umint _Address, umint _Size, uaint _Protect)
	{
	}

	void CCallstackMemoryReporter::f_Commit(umint _MemoryAllocator, umint _Address, umint _Size)
	{
	}

	void CCallstackMemoryReporter::f_Decommit(umint _MemoryAllocator, umint _Address, umint _Size)
	{
	}

	void CCallstackMemoryReporter::fp_StartupThread()
	{
		if (mp_pThread || !g_bCanStartThreads.f_Load(NAtomic::gc_MemoryOrder_Relaxed) || !NTime::CSystem_Time::fs_TimeInitDone())
			return;

		{
			DMibLock(mp_ThreadLock);
			if (mp_pThread)
				return;

			mp_pThread = NThread::CThreadObjectNonTracked::fs_StartThread
				(
					[this](NThread::CThreadObjectNonTracked *_pThread) -> aint
					{
						while (_pThread->f_GetState() != NThread::EThreadState_EventWantQuit)
						{
							fp_ProcessQueue();
							_pThread->m_EventWantQuit.f_WaitTimeout(0.5);
						}
						return 0;
					}
					, "Memory reporter queue processor"
				)
			;
		}
	}

	void CCallstackMemoryReporter::fp_ProcessQueue()
	{
		DMibLock(ms_pThis->m_Lock);
		umint nProcessed = 0;
		while (auto Entry = m_OperationQueue.f_Pop())
		{
			umint QueueSize = --m_QueueSize;
			umint SizeAnd = QueueSize & gc_QueueBitInv;

			if (SizeAnd <= EProcessQueueThreshold && (QueueSize & gc_QueueBitReset))
			{
				while ((QueueSize & gc_QueueBitResetFinished) == 0)
				{
					QueueSize = m_QueueSize.f_Load();
					SizeAnd = QueueSize & gc_QueueBitInv;
				}
				QueueSize = m_QueueSize.f_FetchAnd(~gc_QueueBitReset);
				m_QueueEvent.f_SetSignaled();
				QueueSize = m_QueueSize.f_FetchAnd(~gc_QueueBitResetFinished);
			}

			auto & Op = Entry.f_Get();

			switch (Op.m_OpType)
			{
			case COperation::EOpType_Alloc:
				{
					uint64 Hash = fsp_GetStackFingerprint(Op.m_Callstack.m_Callstack, Op.m_Callstack.m_nCallstack).f_FoldToInt<uint64>();
					auto & Callstack = fp_GetCallstack(Op.m_MemoryAllocator, Hash, Op.m_Callstack.m_Callstack, Op.m_Callstack.m_nCallstack);
					fp_RegisterAllocation(Op.m_MemoryAllocator, Op.m_Address, Op.m_Size, &Callstack);
				}
				break;

			case COperation::EOpType_Resize:
			case COperation::EOpType_Realloc:
				{
					auto pCallstack = fp_RemoveAllocation(Op.m_MemoryAllocator, Op.m_OldAlloc.m_Address);
					if (pCallstack)
						fp_RegisterAllocation(Op.m_MemoryAllocator, Op.m_Address, Op.m_Size, pCallstack);
					else
					{
						DMibTraceSafe("Unknown allocation freed during resize/realloc {} {}, possible loss of {} bytes {\n}", Op.m_MemoryAllocator, Op.m_OldAlloc.m_Address, Op.m_OldAlloc.m_Size);
						uint64 Hash = fsp_GetStackFingerprint(Op.m_Callstack.m_Callstack, Op.m_Callstack.m_nCallstack).f_FoldToInt<uint64>();
						auto & Callstack = fp_GetCallstack(Op.m_MemoryAllocator, Hash, Op.m_Callstack.m_Callstack, Op.m_Callstack.m_nCallstack);
						fp_RegisterAllocation(Op.m_MemoryAllocator, Op.m_Address, Op.m_Size, &Callstack);
					}
				}
				break;

			case COperation::EOpType_Free:
				{
					auto pCallback = fp_RemoveAllocation(Op.m_MemoryAllocator, Op.m_Address);
					if (!pCallback)
					{
						DMibTraceSafe("Unknown allocation freed {} {}, possible loss of {} bytes {\n}", Op.m_MemoryAllocator, Op.m_Address, Op.m_Size);

/*
						uint64 Hash = fsp_GetStackFingerprint(Op.m_Callstack.m_Callstack, Op.m_Callstack.m_nCallstack).f_FoldToInt<uint64>();
						CCallstack & ErrorCallstack = m_Errors.f_Insert(fg_Construct(Hash, Op.m_Callstack.m_Callstack, Op.m_Callstack.m_nCallstack, Op.m_MemoryAllocator));
						++ErrorCallstack.m_Total.m_nAllocations;
						ErrorCallstack.m_Total.m_nBytesFreed += Op.m_Size;
*/
					}
				}
				break;
			case COperation::EOpType_Allocator:
				{
					auto & Allocator = m_Allocators[Op.m_MemoryAllocator];
					Allocator.m_Name = Op.m_AllocatorName;
				}
				break;
			case COperation::EOpType_Report:
				{
					fp_Report(false);
				}
				break;
			case COperation::EOpType_FullReport:
				{
					fp_Report(true);
				}
				break;

			default:
				DMibNeverGetHere;
			}
			++nProcessed;

			if (m_LockRequests.f_Load())
				break;
		}

		if (nProcessed)
		{
			NSys::fg_DebugOutput(NStr::fg_Format<NStr::CStrNonTracked>("Processed {} operations{\n}", nProcessed));
		}
	}

	CCallstackMemoryReporter::CCallstack& CCallstackMemoryReporter::fp_GetCallstack(umint _MemoryAllocator, umint _Hash, CMibCodeAddress *_pStack, umint _nStack)
	{
		auto &CallStack = m_Callstacks[_Hash, _Hash, _pStack, _nStack, _MemoryAllocator];
		return CallStack;
	}

	NCryptography::CHashDigest_MD5 CCallstackMemoryReporter::fsp_GetStackFingerprint(CMibCodeAddress *_pStack, umint _nStack)
	{
		NCryptography::CHash_MD5 Digest;
		NCryptography::TCBinaryStreamHashRef<NCryptography::CHash_MD5> Stream;
		Stream.f_Open(&Digest);
		Stream.f_FeedBytes(_pStack, _nStack * sizeof(CMibCodeAddress));
		return Stream.f_GetDigest();
	}

	bool CCallstackMemoryReporter::fp_RegisterAllocation(umint _MemoryAllocator, umint _Address, umint _Size, CCallstack* _pCallstack)
	{
		auto MapResult = m_Allocations(CAllocationKey(_MemoryAllocator, _Address), _Size, _pCallstack);
		CAllocation &Allocation = *MapResult;

		if (!MapResult.f_WasCreated())
		{
			DMibTraceSafe("Allocation on already allocated address {} {}{\n}", _MemoryAllocator, _Address);
			--Allocation.m_pCallstack->m_Total.m_nAllocations;
			Allocation.m_pCallstack->m_Total.m_nBytes -= Allocation.m_Size;

			Allocation.m_Size = _Size;
			Allocation.m_pCallstack = _pCallstack;
			++Allocation.m_nIgnoreFree;
		}

		++_pCallstack->m_Total.m_nAllocations;
		_pCallstack->m_Total.m_nBytes += _Size;

		return bCreated;
	}

	CCallstackMemoryReporter::CCallstack* CCallstackMemoryReporter::fp_RemoveAllocation(umint _MemoryAllocator, umint _Address)
	{
		CAllocationKey Key(_MemoryAllocator, _Address);
		auto pAllocation = m_Allocations.f_FindEqual(Key);

		if (!pAllocation)
			return nullptr;

		CCallstack* pCallstack = pAllocation->m_pCallstack;

		if (pAllocation->m_nIgnoreFree)
		{
			--pAllocation->m_nIgnoreFree;
			return pCallstack;
		}

		--pCallstack->m_Total.m_nAllocations;
		pCallstack->m_Total.m_nBytes -= pAllocation->m_Size;
		m_Allocations.f_Remove(pAllocation);

		return pCallstack;
	}
}
