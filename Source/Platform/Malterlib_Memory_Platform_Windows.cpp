// Copyright © 2024 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Windows.h>
#include <psapi.h>

#include "../Malterlib_Memory_Platform.h"


using namespace NMib::NMemory;
using namespace NMib::NStr;
using namespace NMib;

namespace
{
}

CSystemMemoryStatistics NMemory::NPlatform::fg_Memory_GetStatistics(EMemoryStatisticsDetailLevel _DetailLevel)
{
	PERFORMANCE_INFORMATION PerfInfo;
	fg_MemClear(PerfInfo);

	PerfInfo.cb = sizeof(PERFORMANCE_INFORMATION);

	if (!GetPerformanceInfo(&PerfInfo, sizeof(PERFORMANCE_INFORMATION)))
		return {};

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
			return fAddByteStat(_Key, _nPages * PerfInfo.PageSize, _StatDetailLevel);
		}
	;

	fAddPageStat(gc_Str<"Total">, PerfInfo.PhysicalTotal, EMemoryStatisticsDetailLevel::mc_Basic);

	if (auto *pStat = fAddPageStat(gc_Str<"Available">, PerfInfo.PhysicalAvailable, EMemoryStatisticsDetailLevel::mc_Basic))
	{
		pStat->m_Characteristics.m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_LessThan;

		static constexpr auto c_WarningPercent = 5u;
		static constexpr auto c_ErrorPercent = 2u;

		static constexpr uint64 c_GibiByte = 1024u * 1024u * 1024u;

		static constexpr uint64 c_LargeMemoryWarning = 3u * c_GibiByte;
		static constexpr uint64 c_LargeMemoryError = 1u * c_GibiByte;

		auto OnePercentOfTotalApproximateBytes = ((PerfInfo.PhysicalTotal * PerfInfo.PageSize) / 128u);

		pStat->m_Characteristics.m_WarningValue = fg_Min(OnePercentOfTotalApproximateBytes * c_WarningPercent, c_LargeMemoryWarning);
		pStat->m_Characteristics.m_ErrorValue = fg_Min(OnePercentOfTotalApproximateBytes * c_ErrorPercent, c_LargeMemoryError);
	}

	fAddPageStat(gc_Str<"Committed">, PerfInfo.CommitTotal, EMemoryStatisticsDetailLevel::mc_Basic);

	fAddPageStat(gc_Str<"SystemCache">, PerfInfo.SystemCache, EMemoryStatisticsDetailLevel::mc_Detailed);
	fAddPageStat(gc_Str<"KernelPaged">, PerfInfo.KernelPaged, EMemoryStatisticsDetailLevel::mc_Detailed);
	fAddPageStat(gc_Str<"KernelNonpaged">, PerfInfo.KernelNonpaged, EMemoryStatisticsDetailLevel::mc_Detailed);

	fAddPageStat(gc_Str<"CommitLimit">, PerfInfo.CommitLimit);
	fAddPageStat(gc_Str<"CommitPeak">, PerfInfo.CommitPeak);

	return Return;
}
