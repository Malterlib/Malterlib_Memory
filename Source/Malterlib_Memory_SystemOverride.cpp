// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#ifdef DMibConfig_OverrideSystemMalloc
#	if defined(DPlatformFamily_Linux)
#		include "Malterlib_Memory_SystemOverride_Linux.hpp"
#	elif defined(DPlatformFamily_Windows)
#		include "Malterlib_Memory_SystemOverride_Windows.hpp"
#	elif defined(DPlatformFamily_OSX)
#		include "Malterlib_Memory_SystemOverride_OSX.hpp"
#	else
#		error "Implement this"
#	endif
#else
void fg_MalterlibMallocOverrideInit()
{
}
void fg_MalterlibMallocOverrideInit_ReinstallHandler()
{
}
void fg_MalterlibMallocOverride_AtExitCalled()
{
}
#endif
