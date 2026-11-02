// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
#ifdef DMibNeedDebugException

	DMibImpErrorClassDefine(CExceptionMemoryManagerDebug, NException::CDebugException);
#	define DMibErrorMemoryManagerDebug(d_Description) DMibImpError(NMib::NMemory::CExceptionMemoryManagerDebug, d_Description)

#	ifndef DMibPNoShortCuts
#		define DErrorMemoryManagerDebug(d_Description) DMibErrorMemoryManagerDebug(d_Description)
#	endif
#endif
}
