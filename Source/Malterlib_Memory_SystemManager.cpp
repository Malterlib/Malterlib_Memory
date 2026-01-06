// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#include "Malterlib_Memory_SystemManager_CrossModule.h"

#if defined(DMibConfig_MemoryManager_UseMalterlib)
#	include "Malterlib_Memory_SystemManager_Malterlib.hpp"
#elif defined(DMibConfig_MemoryManager_UseSystem)
#	include "Malterlib_Memory_SystemManager_System.hpp"
#elif defined(DMibConfig_MemoryManager_UseOverwriteCheck)
#	include "Malterlib_Memory_SystemManager_OverwriteCheck.hpp"
#elif defined(DMibConfig_MemoryManager_UseMiMalloc)
#	include "Malterlib_Memory_SystemManager_MiMalloc.hpp"
#elif defined(DMibConfig_MemoryManager_UseTcMalloc)
#	include "Malterlib_Memory_SystemManager_TcMalloc.hpp"
#else
#	error "Invalid memory manager specified in build system"
#endif

#include "Malterlib_Memory_SystemManager_CrossModule.hpp"
