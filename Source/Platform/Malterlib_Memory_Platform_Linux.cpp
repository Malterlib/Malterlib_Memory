// Copyright © 2024 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include <Mib/Core/PlatformSpecific/PosixErrNo>
#include <Mib/Core/PlatformSpecific/LinuxProcFS>

#include "../Malterlib_Memory_Platform.h"

using namespace NMib::NMemory;
using namespace NMib::NStr;
using namespace NMib;

namespace
{
	struct CKnownStat
	{
		CStr m_Name;
		EMemoryStatisticsDetailLevel m_DetailLevel = EMemoryStatisticsDetailLevel::mc_All;
		EMemoryStatisticComparisonOperator m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_None;
		double m_WarningValue = 0.0;
		double m_ErrorValue = 0.0;
		CStr m_ComparePercentTo;
		bool m_bComparePercent = false;
	};

	constexpr CKnownStat gc_KnownStats[] =
		{
			{
				.m_Name = gc_Str<"Committed_AS">
				, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Basic
				, .m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_GreaterThan
				, .m_WarningValue = 0.95
				, .m_ErrorValue = 1.0
				, .m_ComparePercentTo = gc_Str<"MemTotal">
				, .m_bComparePercent = true
			}
			, {.m_Name = gc_Str<"MemAvailable">, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Basic}
			, {.m_Name = gc_Str<"MemFree">, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Basic}
			, {.m_Name = gc_Str<"MemTotal">, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Basic}
			, {.m_Name = gc_Str<"SwapCached">, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Detailed}
			, {.m_Name = gc_Str<"SwapFree">, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Detailed}
			, {.m_Name = gc_Str<"SwapTotal">, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Detailed}
		}
	;

#if DMibEnableSafeCheck > 0
	bool fg_KnownStatsAreSorted()
	{
		static bool s_bIsSorted = NMisc::fg_IsSorted
			(
				gc_KnownStats
				, sizeof(gc_KnownStats) / sizeof(gc_KnownStats[0])
				, [](CKnownStat const &_Left, CKnownStat const &_Right)
				{
					return _Left.m_Name <=> _Right.m_Name;
				}
			)
		;

		return s_bIsSorted;
	}
#endif

	CKnownStat const *fg_GetKnownStat(CStr const &_Key)
	{
		DMibFastCheck(fg_KnownStatsAreSorted());

		auto iKnownStat = NMisc::fg_BinarySearch
			(
				gc_KnownStats
				, sizeof(gc_KnownStats) / sizeof(gc_KnownStats[0])
				, _Key
				, [](CKnownStat const &_Left, CStr const &_Right)
				{
					return _Left.m_Name <=> _Right;
				}
			)
		;

		if (iKnownStat >= 0)
			return &gc_KnownStats[iKnownStat];
		else
			return nullptr;
	}
}

CSystemMemoryStatistics NMemory::NPlatform::fg_Memory_GetStatistics(EMemoryStatisticsDetailLevel _DetailLevel)
{
	auto FileData = NMib::NPlatform::fg_ReadProcFS("/proc/meminfo");
	CStr Data(FileData.f_GetArray(), FileData.f_GetLen());

	CSystemMemoryStatistics Return;

	NContainer::TCMap<CStr, CSystemMemoryStatistic> AllStats;
	for (auto &Line : Data.f_SplitLine())
	{
		auto Values = Line.f_Split(":");
		if (Values.f_GetLen() != 2)
			continue;

		auto &Key = Values[0];

		auto Value = Values[1].f_Trim();

		auto pParseValue = Value.f_GetStr();

		auto ValueInt = NStr::fg_StrToIntParse(pParseValue, uint64(0));

		auto Unit = EMemoryStatisticUnit::mc_Number;

		if (!*pParseValue)
			Unit = EMemoryStatisticUnit::mc_Number;
		else if (NStr::fg_StrEndsWith(pParseValue, " kB"))
		{
			ValueInt *= 1024;
			Unit = EMemoryStatisticUnit::mc_Bytes;
		}
		else
			continue;

		auto &Statistic = AllStats[Key];
		Statistic.m_Characteristics.m_Unit = Unit;
		Statistic.m_Value = ValueInt;
	}

	for (auto &StatisticEntry : AllStats.f_Entries())
	{
		auto &Key = StatisticEntry.f_Key();
		auto &Statistic = StatisticEntry.f_Value();

		auto *pKnownStat = fg_GetKnownStat(Key);

		if (pKnownStat)
		{
			if (pKnownStat->m_DetailLevel > _DetailLevel)
				continue;
		}
		else if (_DetailLevel != EMemoryStatisticsDetailLevel::mc_All)
			continue;

		if (pKnownStat)
		{
			auto &Characteristics = Statistic.m_Characteristics;
			Characteristics.m_ValueComparisonOperator = pKnownStat->m_ValueComparisonOperator;
			if (pKnownStat->m_ValueComparisonOperator != EMemoryStatisticComparisonOperator::mc_None)
			{
				if (pKnownStat->m_bComparePercent)
				{
					auto *pCompareToValue = AllStats.f_FindEqual(pKnownStat->m_ComparePercentTo);
					if (pCompareToValue)
					{
						Characteristics.m_WarningValue = (fp64(pKnownStat->m_WarningValue) * pCompareToValue->m_Value).f_ToInt();
						Characteristics.m_ErrorValue = (fp64(pKnownStat->m_ErrorValue) * pCompareToValue->m_Value).f_ToInt();
					}
				}
				else
				{
					Characteristics.m_WarningValue = fp64(pKnownStat->m_WarningValue).f_ToInt();
					Characteristics.m_ErrorValue = fp64(pKnownStat->m_ErrorValue).f_ToInt();
				}
			}
		}

		Return.m_Statistics[Key] = fg_Move(Statistic);
	}

	return Return;
}
