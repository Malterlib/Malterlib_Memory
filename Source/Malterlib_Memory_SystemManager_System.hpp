// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if defined(DPlatformFamily_OSX)
#	include "Malterlib_Memory_SystemManager_System_OSX.hpp"
#elif defined(DPlatformFamily_Linux)
#	include "Malterlib_Memory_SystemManager_System_Linux.hpp"
#elif defined(DPlatformFamily_Windows)
#	include "Malterlib_Memory_SystemManager_System_Windows.hpp"
#else
#	error "Implement this"
#endif

