// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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

		constexpr bool operator == (CAllocator_Placement const &_Right) const
		{
			return true;
		}

		constexpr auto operator <=> (CAllocator_Placement const &_Right) const
		{
			return COrdering_Strong::equal;
		}

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Placement>;

		inline_always CAllocator_Placement();
		inline_always static bool f_IsStatic(void const *_pBlock);
		inline_always static bool f_OnlyOneAlloc();
		inline_always static mint f_StaticAddresses();
		inline_always static mint f_GranularityAlloc(bool _bLargePages = false);
		inline_always static mint f_GranularityCommit(bool _bLargePages = false);
		inline_always static mint f_GranularityProtect(bool _bLargePages = false);
		inline_always only_parameters_aliased static mint f_Size(void *_pBlock);
		inline_always only_parameters_aliased static mint f_TrySize(void *_pBlock);
		inline_always static mint f_SizePadded(mint _Size);
		inline_always static fp32 f_Overhead(void const *_pBlock);
		constexpr inline_always static bool f_CanCommit();
		inline_always static bool f_CanProtect();
		inline_always static bool f_DeterministicSize();
		inline_always only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
		inline_always only_parameters_aliased static malloc_like void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size);
		inline_always only_parameters_aliased static void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
		inline_always only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
		inline_always only_parameters_aliased static void f_Free(void *_pBlock, mint _Size);
		inline_always only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
	};
}

#include "Malterlib_Memory_Allocator_Placement.hpp"
