// Copyright © 2024 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib
#include "../Malterlib_Memory_Platform.h"

#include <Mib/Process/Platform>

#include <mach/mach.h>
#include <sys/sysctl.h>

using namespace NMib::NMemory;
using namespace NMib::NStr;
using namespace NMib;


namespace NMib::NSys::NPrivate
{
	extern mint g_PageSize;
}

namespace
{
	int fg_SysCtl_int(ch8 const *_pName)
	{
		int Value = 0;
		size_t DataSize = sizeof(Value);
		int Ret = sysctlbyname(_pName, &Value, &DataSize, nullptr, 0);

		if (Ret == 0)
			return Value;

		return 0;
	}

	mint fg_SysCtl_mint(ch8 const *_pName)
	{
		size_t Value = 0;
		size_t DataSize = sizeof(Value);
		int Ret = sysctlbyname(_pName, &Value, &DataSize, nullptr, 0);

		if (Ret == 0)
			return Value;

		return 0;
	}

	uint64 fg_GetStolenMemory()
	{
		// From: https://github.com/apple-oss-distributions/top/blob/a989bd5d18246e330e5feadd80958b913351f8ae/libtop.c#L851

		return fg_AlignDown(fg_SysCtl_mint("machdep.memmap.Reserved") + fg_SysCtl_mint("machdep.memmap.Unusable") + fg_SysCtl_mint("machdep.memmap.Other"), 128 * 1024 * 1024);
	}
}

CSystemMemoryStatistics NMemory::NPlatform::fg_Memory_GetStatistics(EMemoryStatisticsDetailLevel _DetailLevel)
{
	CSystemMemoryStatistics Return;

	auto fAddByteStat = [&](CStr const &_Key, auto _nBytes, EMemoryStatisticsDetailLevel _StatDetailLevel = EMemoryStatisticsDetailLevel::mc_All) -> CSystemMemoryStatistic *
		{
			if (_StatDetailLevel > _DetailLevel)
				return nullptr;

			auto &Statistic = Return.m_Statistics[_Key];
			Statistic.m_Value = _nBytes;
			Statistic.m_Characteristics.m_Unit = EMemoryStatisticUnit::mc_Bytes;

			return &Statistic;
		}
	;

	auto fAddPageStat = [&](CStr const &_Key, auto _nPages, EMemoryStatisticsDetailLevel _StatDetailLevel = EMemoryStatisticsDetailLevel::mc_All) -> CSystemMemoryStatistic *
		{
			return fAddByteStat(_Key, uint64(NSys::NPrivate::g_PageSize) * uint64(_nPages), _StatDetailLevel);
		}
	;

	auto fAddNumStat = [&](CStr const &_Key, auto _NumberOf, EMemoryStatisticsDetailLevel _StatDetailLevel = EMemoryStatisticsDetailLevel::mc_All) -> CSystemMemoryStatistic *
		{
			if (_StatDetailLevel > _DetailLevel)
				return nullptr;

			auto &Statistic = Return.m_Statistics[_Key];
			Statistic.m_Value = _NumberOf;
			Statistic.m_Characteristics.m_Unit = EMemoryStatisticUnit::mc_Number;

			return &Statistic;
		}
	;

	auto TotalMemory = NProcess::NPlatform::fg_Process_GetPhysicalMemory();

	fAddByteStat(gc_Str<"Total">, TotalMemory, EMemoryStatisticsDetailLevel::mc_Basic);

	if (auto *pStat = fAddNumStat(gc_Str<"PressureLevel">, fg_SysCtl_int("kern.memorystatus_vm_pressure_level"), EMemoryStatisticsDetailLevel::mc_Basic))
	{
		pStat->m_Characteristics.m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_GreaterThan;
		pStat->m_Characteristics.m_WarningValue = 1;
		pStat->m_Characteristics.m_ErrorValue = 3;
	}

	vm_statistics64_data_t VmStatistics;
	auto VmStatisticsCount = HOST_VM_INFO64_COUNT;
	kern_return_t Result = host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&VmStatistics, &VmStatisticsCount);
	if (Result == KERN_SUCCESS)
	{
		VmStatistics.wire_count += fg_GetStolenMemory();

		fAddPageStat(gc_Str<"Free">, VmStatistics.free_count, EMemoryStatisticsDetailLevel::mc_Basic);
		fAddPageStat(gc_Str<"Available">, VmStatistics.external_page_count + VmStatistics.free_count + VmStatistics.purgeable_count, EMemoryStatisticsDetailLevel::mc_Basic);

		fAddPageStat(gc_Str<"Active">, VmStatistics.active_count, EMemoryStatisticsDetailLevel::mc_Detailed);
		fAddPageStat(gc_Str<"Inactive">, VmStatistics.inactive_count, EMemoryStatisticsDetailLevel::mc_Detailed);
		fAddPageStat(gc_Str<"CurrentCompressed">, VmStatistics.compressor_page_count, EMemoryStatisticsDetailLevel::mc_Detailed);
		fAddPageStat(gc_Str<"CurrentUncompressedInCompressor">, VmStatistics.total_uncompressed_pages_in_compressor, EMemoryStatisticsDetailLevel::mc_Detailed);
		fAddPageStat(gc_Str<"Wired">, VmStatistics.wire_count, EMemoryStatisticsDetailLevel::mc_Detailed);

		fAddPageStat(gc_Str<"ZeroFill">, VmStatistics.zero_fill_count);
		fAddPageStat(gc_Str<"Reactivated">, VmStatistics.reactivations);
		fAddPageStat(gc_Str<"PagedIn">, VmStatistics.pageins);
		fAddPageStat(gc_Str<"PagedOut">, VmStatistics.pageouts);
		fAddPageStat(gc_Str<"Faulted">, VmStatistics.faults);
		fAddPageStat(gc_Str<"CopyOnWriteBytes">, VmStatistics.cow_faults);
		fAddNumStat(gc_Str<"ObjectCacheLookups">, VmStatistics.lookups);
		fAddNumStat(gc_Str<"ObjectCacheHits">, VmStatistics.hits);
		fAddPageStat(gc_Str<"Purged">, VmStatistics.purges);
		fAddPageStat(gc_Str<"Purgable">, VmStatistics.purgeable_count);
		fAddPageStat(gc_Str<"Speculative">, VmStatistics.speculative_count);
		fAddPageStat(gc_Str<"Decompressed">, VmStatistics.decompressions);
		fAddPageStat(gc_Str<"Compressed">, VmStatistics.compressions);
		fAddPageStat(gc_Str<"SwappedIn">, VmStatistics.swapins);
		fAddPageStat(gc_Str<"SwappedOut">, VmStatistics.swapouts);
		fAddPageStat(gc_Str<"Throttled">, VmStatistics.throttled_count);
		fAddPageStat(gc_Str<"FileBacked">, VmStatistics.external_page_count);
		fAddPageStat(gc_Str<"Anonymous">, VmStatistics.internal_page_count);
	}

	return Return;
}
