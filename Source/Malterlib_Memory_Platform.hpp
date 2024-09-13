// Copyright © 2024 Favro Holding AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename tf_CStr>
	void CSystemMemoryStatisticCharacteristics::f_Format(tf_CStr &o_Str) const
	{
		if (m_ValueComparisonOperator == EMemoryStatisticComparisonOperator::mc_None)
		{
			o_Str += typename tf_CStr::CFormat("{}")
				<< fs_UnitToStr(m_Unit)
			;
		}
		else
		{
			auto ComparisonOperatorStr = fs_ComparisonOperatorToStr(m_ValueComparisonOperator);
			o_Str += typename tf_CStr::CFormat("{} Warn: {}{ns } Error: {}{ns }")
				<< fs_UnitToStr(m_Unit)
				<< ComparisonOperatorStr
				<< m_WarningValue
				<< ComparisonOperatorStr
				<< m_ErrorValue
			;
		}
	}

	template <typename tf_CStr>
	void CSystemMemoryStatistic::f_Format(tf_CStr &o_Str) const
	{
		o_Str += typename tf_CStr::CFormat("{ns } {}")
			<< m_Value
			<< m_Characteristics
		;
	}

	template <typename tf_CStr>
	void CSystemMemoryStatistics::f_Format(tf_CStr &o_Str) const
	{
		o_Str += typename tf_CStr::CFormat("{}") << m_Statistics;
	}
}
