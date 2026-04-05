// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Memory/Platform>

namespace NMib::NMemory
{
	NStr::CStr CSystemMemoryStatisticCharacteristics::fs_UnitToStr(EMemoryStatisticUnit _Unit)
	{
		switch (_Unit)
		{
		case EMemoryStatisticUnit::mc_Number: return {};
		case EMemoryStatisticUnit::mc_Bytes: return NStr::gc_Str<"bytes">;
		}

		return {};
	}

	NStr::CStr CSystemMemoryStatisticCharacteristics::fs_ComparisonOperatorToStr(EMemoryStatisticComparisonOperator _Operator)
	{
		switch (_Operator)
		{
		case EMemoryStatisticComparisonOperator::mc_None: return {};
		case EMemoryStatisticComparisonOperator::mc_LessThan: return NStr::gc_Str<"<">;
		case EMemoryStatisticComparisonOperator::mc_GreaterThan: return NStr::gc_Str<">">;
		case EMemoryStatisticComparisonOperator::mc_Equal: return NStr::gc_Str<"=">;
		}

		return {};
	}
}
