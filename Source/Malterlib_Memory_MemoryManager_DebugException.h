// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	DMibImpErrorClassDefine(CExceptionMemoryManagerDebug, NException::CDebugException);
#	define DMibErrorMemoryManagerDebug(_Description) DMibImpError(NMib::NMemory::CExceptionMemoryManagerDebug, _Description)

#	ifndef DMibPNoShortCuts
#		define DErrorMemoryManagerDebug(_Description) DMibErrorMemoryManagerDebug(_Description)
#	endif

}
