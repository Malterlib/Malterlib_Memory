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
	using FCalculateValue = fp64 (*)(fp64 _CompareToValue);
	struct CKnownStat
	{
		CStr m_Name;
		EMemoryStatisticsDetailLevel m_DetailLevel = EMemoryStatisticsDetailLevel::mc_All;
		EMemoryStatisticComparisonOperator m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_None;
		FCalculateValue m_fWarningValue = nullptr;
		FCalculateValue m_fErrorValue = nullptr;
		CStr m_CompareTo;
	};

	constexpr CKnownStat gc_KnownStats[] =
		{
			{
				.m_Name = gc_Str<"Committed_AS">
				, .m_DetailLevel = EMemoryStatisticsDetailLevel::mc_Basic
				, .m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_GreaterThan
				, .m_fWarningValue = [](fp64 _TotalMemory) -> fp64
				{
					fp64 GibiByte = 1024 * 1024 * 1024;

					// Committed_AS can be larger than total memory. Committed memory might not have been touched and in that case does not take up any physical memory.
					if (_TotalMemory <= GibiByte * 1.0)
						return _TotalMemory * 1.6;
					else if (_TotalMemory <= GibiByte * 2.0)
						return _TotalMemory * 1.4;
					else if (_TotalMemory <= GibiByte * 4.0)
						return _TotalMemory * 1.2;
					else if (_TotalMemory <= GibiByte * 8.0)
						return _TotalMemory * 1.1;
					else
						return _TotalMemory + GibiByte * 1.0;
				}
				, .m_fErrorValue = [](fp64 _TotalMemory) -> fp64
				{
					fp64 GibiByte = 1024 * 1024 * 1024;

					// Committed_AS can be larger than total memory. Committed memory might not have been touched and in that case does not take up any physical memory.
					if (_TotalMemory <= GibiByte * 1.0)
						return _TotalMemory * 1.7;
					else if (_TotalMemory <= GibiByte * 2.0)
						return _TotalMemory * 1.5;
					else if (_TotalMemory <= GibiByte * 4.0)
						return _TotalMemory * 1.3;
					else if (_TotalMemory <= GibiByte * 8.0)
						return _TotalMemory * 1.2;
					else
						return _TotalMemory + GibiByte * 2.0;
				}
				, .m_CompareTo = gc_Str<"MemTotal">
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
				, fg_ArraySize(gc_KnownStats)
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
				, fg_ArraySize(gc_KnownStats)
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
			if (pKnownStat->m_ValueComparisonOperator != EMemoryStatisticComparisonOperator::mc_None && (pKnownStat->m_fWarningValue || pKnownStat->m_fErrorValue))
			{
				auto *pCompareToValue = AllStats.f_FindEqual(pKnownStat->m_CompareTo);

				if (pKnownStat->m_fWarningValue)
				{
					if (pCompareToValue)
						Characteristics.m_WarningValue = pKnownStat->m_fWarningValue(pCompareToValue->m_Value).f_ToInt();
					else
						Characteristics.m_WarningValue = pKnownStat->m_fWarningValue(0.0).f_ToInt();
				}

				if (pKnownStat->m_fErrorValue)
				{
					if (pCompareToValue)
						Characteristics.m_ErrorValue = pKnownStat->m_fErrorValue(pCompareToValue->m_Value).f_ToInt();
					else
						Characteristics.m_ErrorValue = pKnownStat->m_fErrorValue(0.0).f_ToInt();
				}
			}
		}

		Return.m_Statistics[Key] = fg_Move(Statistic);
	}

	return Return;
}
