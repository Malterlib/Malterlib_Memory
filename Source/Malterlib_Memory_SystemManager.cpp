// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#include "Malterlib_Memory_SystemManager_CrossModule.h"

#if defined(DMibConfig_MemoryManager_UseNew)
#	include "Malterlib_Memory_SystemManager_New.hpp"
#elif defined(DMibConfig_MemoryManager_UseSystem)
#	include "Malterlib_Memory_SystemManager_System.hpp"
#elif defined(DMibConfig_MemoryManager_UseOverwriteCheck)
#	include "Malterlib_Memory_SystemManager_OverwriteCheck.hpp"
#elif defined(DMibConfig_MemoryManager_UseOld)
#	include "Malterlib_Memory_SystemManager_Old.hpp"
#elif defined(DMibConfig_MemoryManager_UseDLMalloc)
#	include "Malterlib_Memory_SystemManager_DLMalloc.hpp"
#elif defined(DMibConfig_MemoryManager_UseLLAlloc)
#	include "Malterlib_Memory_SystemManager_LLAlloc.hpp"
#else
#	error "Invalid memory manager specified in build system"
#endif

#include "Malterlib_Memory_SystemManager_CrossModule.hpp"
