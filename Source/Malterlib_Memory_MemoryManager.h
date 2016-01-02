// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

//#	define DMibPNoUnalignedAccess

namespace NMib
{
	namespace NMem
	{
		template <typename t_CParams>
		struct TCMemoryManager;

		template <typename t_CParams>
		struct TCMemoryManagerArena;

		template <typename t_CParams>
		struct TCMemoryManagerNumaArena;

	}
}

#include "Malterlib_Memory_MemoryManager_Config.h"
#include "Malterlib_Memory_MemoryManager_Allocator.h"
#include "Malterlib_Memory_MemoryManager_Main.h"

#undef DMibMemoryManagerLink
#undef DMibMemoryManagerList
#undef DMibMemoryManagerList_FromTemplate
