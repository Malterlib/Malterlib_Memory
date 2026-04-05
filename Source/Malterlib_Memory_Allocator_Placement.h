// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	class CAllocator_Placement : public CAllocator_Base
	{
	public:
		enum
		{
			mc_Reporting = false
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = false
		};

		constexpr bool operator == (CAllocator_Placement const &_Right) const noexcept
		{
			return true;
		}

		constexpr auto operator <=> (CAllocator_Placement const &_Right) const noexcept
		{
			return COrdering_Strong::equal;
		}

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Placement>;

		inline_always CAllocator_Placement();
		inline_always static bool f_IsStatic(void const *_pBlock);
		inline_always static bool f_OnlyOneAlloc();
		inline_always static umint f_StaticAddresses();
		inline_always static umint f_GranularityAlloc(bool _bLargePages = false);
		inline_always static umint f_GranularityCommit(bool _bLargePages = false);
		inline_always static umint f_GranularityProtect(bool _bLargePages = false);
		inline_always only_parameters_aliased static umint f_Size(void *_pBlock);
		inline_always only_parameters_aliased static umint f_TrySize(void *_pBlock);
		inline_always static umint f_SizePadded(umint _Size);
		inline_always static fp32 f_Overhead(void const *_pBlock);
		constexpr inline_always static bool f_CanCommit();
		inline_always static bool f_CanProtect();
		inline_always static bool f_DeterministicSize();
		inline_always only_parameters_aliased static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		inline_always only_parameters_aliased static malloc_like void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		inline_always only_parameters_aliased static void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void f_Commit(void *_pMem, umint _Size);
		inline_always only_parameters_aliased static void f_Decommit(void *_pMem, umint _Size);
		inline_always only_parameters_aliased static void f_Free(void *_pBlock, umint _Size);
		inline_always only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
	};
}

#include "Malterlib_Memory_Allocator_Placement.hpp"
