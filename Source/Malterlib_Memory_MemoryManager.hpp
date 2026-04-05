// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>


//#	define DMibPNoUnalignedAccess

namespace NMib::NMemory
{
	template <typename t_CParams>
	struct TCMemoryManager;

	template <typename t_CParams>
	struct TCMemoryManagerArena;

	template <typename t_CParams>
	struct TCMemoryManagerNumaArena;
}

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
