// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "Malterlib_Memory_MemoryManager.hpp"

namespace NMib
{
	// Shared by the plain and tracking system manager configurations so the flavor selection
	// applies to both
	struct CMemoryManagerParamsFlavorOverrides : NMemory::CDefaultMemoryManagerParams
	{
		// The MalterlibMemoryManagerFlavor build property emits this define for the non-default
		// flavors (BlockLists = 0, SubSlabLists = 1); Bitmap is the compiled-in default below
#ifdef DMibConfig_Memory_FreeStoreMode
		static constexpr NMemory::EMemoryManagerFreeStore mc_FreeStoreMode = (NMemory::EMemoryManagerFreeStore)DMibConfig_Memory_FreeStoreMode;
#else
		static constexpr NMemory::EMemoryManagerFreeStore mc_FreeStoreMode = NMemory::EMemoryManagerFreeStore_SubSlabBitmaps;
#endif

		static constexpr bool mc_bUseSmallSizes = mc_FreeStoreMode != NMemory::EMemoryManagerFreeStore_SubSlabBitmaps;

#ifdef DMibConfig_Memory_ReapInCleanup
		static constexpr bool mc_bReapInCleanup = DMibConfig_Memory_ReapInCleanup;
#else
		static constexpr bool mc_bReapInCleanup = mc_FreeStoreMode != NMemory::EMemoryManagerFreeStore_ArenaBlockLists;
#endif

#ifdef DMibConfig_Memory_ReapDenseBitmaps
		static constexpr bool mc_bReapDenseBitmaps = DMibConfig_Memory_ReapDenseBitmaps;
#else
		static constexpr bool mc_bReapDenseBitmaps = true;
#endif

#ifdef DMibConfig_Memory_GlobalAddressOrder
		static constexpr bool mc_bGlobalAddressOrder = DMibConfig_Memory_GlobalAddressOrder;
#else
		static constexpr bool mc_bGlobalAddressOrder = false;
#endif

#ifdef DMibConfig_Memory_OwnerReclaimRetain
		static constexpr umint mc_nOwnerReclaimRetainSubSlabs = DMibConfig_Memory_OwnerReclaimRetain;
#else
		static constexpr umint mc_nOwnerReclaimRetainSubSlabs = 0;
#endif
	};
}
