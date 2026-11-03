// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory::NPrivate
{
	inline_always uint64 fg_CalcMagic(void *_pAddress, uint64 _Magic);
	inline_always bool fg_IsAllocationAlignmentValid(umint _Alignment);
	inline_always bool fg_TryAlignAllocationSize(umint &o_Size, umint _Alignment);
	inline_always umint fg_AlignAllocationSizeOrThrow(umint _Size, umint _Alignment);
	inline_always umint fg_AddAllocationSizeOrThrow(umint _Size, umint _ExtraSize);
	inline_always umint fg_AlignAllocationSizeWithExtraOrThrow(umint _Size, umint _ExtraSize, umint _Alignment);
}

#include "Malterlib_Memory_MemoryManager_Utils.hpp"
