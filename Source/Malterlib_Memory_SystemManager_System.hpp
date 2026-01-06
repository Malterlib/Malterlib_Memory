// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if defined(DPlatformFamily_macOS)
#	if DMibConfig_MemoryManager_System_UseLibC
#		include "Malterlib_Memory_SystemManager_System_Linux.hpp"
#	else
#		include "Malterlib_Memory_SystemManager_System_MacOS.hpp"
#	endif
#elif defined(DPlatformFamily_Linux)
#	include "Malterlib_Memory_SystemManager_System_Linux.hpp"
#elif defined(DPlatformFamily_Windows)
#	include "Malterlib_Memory_SystemManager_System_Windows.hpp"
#else
#	error "Implement this"
#endif

