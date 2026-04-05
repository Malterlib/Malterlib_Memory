// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
#ifdef DMibNeedDebugException

	DMibImpErrorClassDefine(CExceptionMemoryManagerDebug, NException::CDebugException);
#	define DMibErrorMemoryManagerDebug(_Description) DMibImpError(NMib::NMemory::CExceptionMemoryManagerDebug, _Description)

#	ifndef DMibPNoShortCuts
#		define DErrorMemoryManagerDebug(_Description) DMibErrorMemoryManagerDebug(_Description)
#	endif
#endif
}
