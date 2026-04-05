// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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

