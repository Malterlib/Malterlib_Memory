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

#if 1
#	define DMibMemoryManagerLink DMibListLinkDS_Link
#	define DMibMemoryManagerList DMibListLinkDS_List
#	define DMibMemoryManagerList_FromTemplate DMibListLinkDS_List_FromTemplate
#else
#	define DMibMemoryManagerLink DMibListLinkD_Link
#	define DMibMemoryManagerList DMibListLinkD_List
#	define DMibMemoryManagerList_FromTemplate DMibListLinkD_List_FromTemplate
#endif


#include "Malterlib_Memory_MemoryManager_Config.h"
#include "Malterlib_Memory_MemoryManager_Allocator.h"
#include "Malterlib_Memory_MemoryManager_Heap.h"
#include "Malterlib_Memory_MemoryManager_Utils.h"
#include "Malterlib_Memory_MemoryManager_SubSlab.h"
#include "Malterlib_Memory_MemoryManager_SmallSubSlab.h"
#include "Malterlib_Memory_MemoryManager_Slab.h"
#include "Malterlib_Memory_MemoryManager_Message.h"
#include "Malterlib_Memory_MemoryManager_Arena.h"
#include "Malterlib_Memory_MemoryManager_NumaArena.h"
#include "Malterlib_Memory_MemoryManager_Main.h"

#include "Malterlib_Memory_MemoryManager_Config.hpp"
#include "Malterlib_Memory_MemoryManager_Main.hpp"
#include "Malterlib_Memory_MemoryManager_Arena.hpp"
#include "Malterlib_Memory_MemoryManager_BackgroundCleanup.hpp"
#include "Malterlib_Memory_MemoryManager_NumaArena.hpp"
#include "Malterlib_Memory_MemoryManager_Slab.hpp"
#include "Malterlib_Memory_MemoryManager_SmallSubSlab.hpp"
#include "Malterlib_Memory_MemoryManager_Heap.hpp"


#undef DMibMemoryManagerLink
#undef DMibMemoryManagerList
#undef DMibMemoryManagerList_FromTemplate
