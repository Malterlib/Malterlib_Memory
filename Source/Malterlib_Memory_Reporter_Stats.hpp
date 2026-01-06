// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_Reporter_Stats.h"

namespace NMib::NMemory
{
	CStatsMemoryReporter *CStatsMemoryReporter::ms_pThis = nullptr;

	CStatsMemoryReporter::CStatsMemoryReporter()
	{
		if (ms_pThis != nullptr)
			DMibPDebugBreak;
		ms_pThis = this;

		fg_ReportMemoryGloballyTo(this);
	}

	void fg_ReportSize(mint _Tabs, CStatsMemoryReporter::CSize const &_Size)
	{
		fp64 OverheadPercent = (_Size.m_nBytesOverhead / fp64(_Size.m_nBytes)) * 100.0;
		fp64 OverheadPerAlloc = (_Size.m_nBytesOverhead / fp64(_Size.m_nAllocations));
		fp64 OverallocPercent = fp64(_Size.m_nBytes - _Size.m_nBytesRequested) / fp64(_Size.m_nBytes) * 100.0;
		fp64 OverallocPerAlloc = fp64(_Size.m_nBytes - _Size.m_nBytesRequested) / fp64(_Size.m_nAllocations);
		fp64 TimeSpentAlloc = fp64(_Size.m_Cycles) * NTime::CSystem_Time::fs_CyclesFrequencyReciprocal();
		fp64 TimeSpentFree = fp64(_Size.m_CyclesFree) * NTime::CSystem_Time::fs_CyclesFrequencyReciprocal();
		fp64 CyclesPerAllocation = fp64(_Size.m_Cycles) / fp64(_Size.m_nAllocations);
		fp64 CyclesPerFree = fp64(_Size.m_CyclesFree) / fp64(_Size.m_nAllocations);

		DMibTraceSafe("{sf ,sj*}Time alloc	  {sl12,fe2} ms  ({fe2} cycles per alloc){\n}", "" << _Tabs << (TimeSpentAlloc * 1000.0) << CyclesPerAllocation);
		DMibTraceSafe("{sf ,sj*}Time free	  {sl12,fe2} ms  ({fe2} cycles per free){\n}", "" << _Tabs << (TimeSpentFree * 1000.0) << CyclesPerFree);
		DMibTraceSafe("{sf ,sj*}Allocations  {sl12}{\n}", "" << _Tabs << _Size.m_nAllocations);
		DMibTraceSafe("{sf ,sj*}Bytes        {sl12}{\n}", "" << _Tabs << _Size.m_nBytes);
		DMibTraceSafe("{sf ,sj*}Overhead     {sl12,fe2}     ({fe2} %)  ({fe2} per alloc){\n}", "" << _Tabs << _Size.m_nBytesOverhead << OverheadPercent << OverheadPerAlloc);
		DMibTraceSafe("{sf ,sj*}Overalloc    {sl12}     ({fe2} %)  ({fe2} per alloc){\n}", "" << _Tabs << (_Size.m_nBytes - _Size.m_nBytesRequested) << OverallocPercent << OverallocPerAlloc);
	}

	CStatsMemoryReporter::~CStatsMemoryReporter()
	{
		fg_ReportMemoryGloballyTo(nullptr);

		m_ThreadLocal.f_Destroy(); // Collect all thread locals into global count

		ms_pThis = nullptr;

		DMibTraceSafe("Memory statistics:{\n}", 0);

		for (auto iAllocator = m_GlobalAllocators.f_GetIterator(); iAllocator; ++iAllocator)
		{
			DMibTraceSafe("Allocator: {}{\n}", iAllocator->m_Name);
			DMibListLinkDS_List(CSize, m_Link) SizesList;
			for (auto iSize = iAllocator->m_Sizes.f_GetIterator(); iSize; ++iSize)
				SizesList.f_Insert(*iSize);

			SizesList.f_Sort
				(
					[&](CSize const* _pLeft, CSize const* _pRight)
					{
						return _pLeft->m_Cycles < _pRight->m_Cycles;

/*							if (_pLeft->m_nAllocations < _pRight->m_nAllocations)
							return true;
						else if (_pLeft->m_nAllocations > _pRight->m_nAllocations)
							return false;

						return _pLeft->f_GetSize() < _pRight->f_GetSize();*/
					}
				)
			;

			for (auto iSize = SizesList.f_GetIterator(); iSize; ++iSize)
			{
				DMibTraceSafe("    Size {}:{\n}", iSize->f_GetSize());
				fg_ReportSize(8, *iSize);
			}
			DMibTraceSafe("Total{\n}", 0);
			fg_ReportSize(4, iAllocator->m_Total);
		}
	}

	CStatsMemoryReporter::CThreadLocal::~CThreadLocal()
	{
		DMibLock(ms_pThis->m_Lock);
		for (auto iAllocator = m_Allocators.f_GetIterator(); iAllocator; ++iAllocator)
		{
			auto &Allocator = ms_pThis->m_GlobalAllocators[iAllocator.f_GetKey()];
			if (!iAllocator->m_Name.f_IsEmpty())
				Allocator.m_Name = iAllocator->m_Name;
			Allocator.m_Total += iAllocator->m_Total;
			for (auto iSize = iAllocator->m_Sizes.f_GetIterator(); iSize; ++iSize)
			{
				Allocator.m_Sizes[iSize.f_GetKey()] += *iSize;
			}
		}
	}

	void CStatsMemoryReporter::f_AllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName)
	{
		auto &ThreadLocal = *m_ThreadLocal;

		auto & Allocator = ThreadLocal.m_Allocators[_MemoryAllocator];
		Allocator.m_Name = _pAllocatorName;
	}

	void CStatsMemoryReporter::f_AllocatorDelete(mint _MemoryAllocator)
	{
	}

	void CStatsMemoryReporter::f_ScopeEnter(mint _MemoryAllocator)
	{
		auto &ThreadLocal = *m_ThreadLocal;
		if (ThreadLocal.m_CyclesDepth >= 16)
			DMibPDebugBreak;
		ThreadLocal.m_CyclesStart[ThreadLocal.m_CyclesDepth] = NTime::NPlatform::fg_Timer_CyclesFast();
		++ThreadLocal.m_CyclesDepth;
	}

	void CStatsMemoryReporter::f_ScopeExit(mint _MemoryAllocator)
	{
		auto &ThreadLocal = *m_ThreadLocal;
		--ThreadLocal.m_CyclesDepth;
	}

	void CStatsMemoryReporter::f_Alloc
		(
			mint _MemoryAllocator
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
		uint64 CyclesEnd = NTime::NPlatform::fg_Timer_CyclesFast();
		auto &ThreadLocal = *m_ThreadLocal;
		uint64 nCycles = CyclesEnd - ThreadLocal.m_CyclesStart[ThreadLocal.m_CyclesDepth - 1];

		auto & Allocator = ThreadLocal.m_Allocators[_MemoryAllocator];

		++Allocator.m_Total.m_nAllocations;
		Allocator.m_Total.m_nBytes += _ReturnedSize;
		Allocator.m_Total.m_nBytesRequested += _RequestedSize;
		Allocator.m_Total.m_nBytesOverhead += _nBytesOverhead;
		Allocator.m_Total.m_Cycles += nCycles;

		auto & Size = Allocator.m_Sizes[_ReturnedSize];

		++Size.m_nAllocations;
		Size.m_nBytes += _ReturnedSize;
		Size.m_nBytesRequested += _RequestedSize;
		Size.m_nBytesOverhead += _nBytesOverhead;
		Size.m_Cycles += nCycles;
	}

	void CStatsMemoryReporter::f_Resize
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
		)
	{
		uint64 CyclesEnd = NTime::NPlatform::fg_Timer_CyclesFast();
		auto &ThreadLocal = *m_ThreadLocal;
		uint64 nCycles = CyclesEnd - ThreadLocal.m_CyclesStart[ThreadLocal.m_CyclesDepth - 1];
		auto & Allocator = ThreadLocal.m_Allocators[_MemoryAllocator];
		auto & Size = Allocator.m_Sizes[_ReturnedSize];
		Allocator.m_Total.m_Cycles += nCycles;
		Size.m_Cycles += nCycles;

		if (_OldAddress != _Address)
		{
			++Allocator.m_Total.m_nAllocations;
			Allocator.m_Total.m_nBytes += _ReturnedSize;
			Allocator.m_Total.m_nBytesRequested += _RequestedSize;
			Allocator.m_Total.m_nBytesOverhead += _nBytesOverhead;

			++Size.m_nAllocations;
			Size.m_nBytes += _ReturnedSize;
			Size.m_nBytesRequested += _RequestedSize;
			Size.m_nBytesOverhead += _nBytesOverhead;
		}
	}

	void CStatsMemoryReporter::f_Realloc
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
		)
	{
		uint64 CyclesEnd = NTime::NPlatform::fg_Timer_CyclesFast();
		auto &ThreadLocal = *m_ThreadLocal;
		uint64 nCycles = CyclesEnd - ThreadLocal.m_CyclesStart[ThreadLocal.m_CyclesDepth - 1];
		auto & Allocator = ThreadLocal.m_Allocators[_MemoryAllocator];
		auto & Size = Allocator.m_Sizes[_ReturnedSize];
		Allocator.m_Total.m_Cycles += nCycles;
		Size.m_Cycles += nCycles;

		if (_OldAddress != _Address)
		{
			++Allocator.m_Total.m_nAllocations;
			Allocator.m_Total.m_nBytes += _ReturnedSize;
			Allocator.m_Total.m_nBytesRequested += _RequestedSize;
			Allocator.m_Total.m_nBytesOverhead += _nBytesOverhead;

			++Size.m_nAllocations;
			Size.m_nBytes += _ReturnedSize;
			Size.m_nBytesRequested += _RequestedSize;
			Size.m_nBytesOverhead += _nBytesOverhead;
		}
	}

	void CStatsMemoryReporter::f_Free(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo)
	{
		uint64 CyclesEnd = NTime::NPlatform::fg_Timer_CyclesFast();
		auto &ThreadLocal = *m_ThreadLocal;
		uint64 nCycles = CyclesEnd - ThreadLocal.m_CyclesStart[ThreadLocal.m_CyclesDepth - 1];
		auto & Allocator = ThreadLocal.m_Allocators[_MemoryAllocator];
		auto & Size = Allocator.m_Sizes[_Size];
		Allocator.m_Total.m_CyclesFree += nCycles;
		Size.m_CyclesFree += nCycles;
	}

	void CStatsMemoryReporter::f_GetSize(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo)
	{
	}

	void CStatsMemoryReporter::f_Protect(mint _MemoryAllocator, mint _Address, mint _Size, uaint _Protect)
	{
	}

	void CStatsMemoryReporter::f_Commit(mint _MemoryAllocator, mint _Address, mint _Size)
	{
	}

	void CStatsMemoryReporter::f_Decommit(mint _MemoryAllocator, mint _Address, mint _Size)
	{
	}
}
