// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	struct CAllocator_Placement : public CAllocator_Base
	{
		static constexpr aint mc_Reporting = false;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = false;

		constexpr bool operator == (CAllocator_Placement const &_Right) const noexcept;
		constexpr auto operator <=> (CAllocator_Placement const &_Right) const noexcept;

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Placement>;

		inline_always CAllocator_Placement();
		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
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
		static constexpr inline_always bool f_CanCommit();
		inline_always static bool f_CanProtect();
		inline_always static bool f_DeterministicSize();
		inline_always only_parameters_aliased static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		inline_always only_parameters_aliased static malloc_like auto f_AllocWithSizeDebug
			(
				umint &_Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static malloc_like auto f_AllocDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static malloc_like auto f_AllocWithSize
			(
				umint &_Size
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static malloc_like void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_always only_parameters_aliased static malloc_like auto f_AllocAlignedWithSize
			(
				umint &_Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static malloc_like auto f_AllocAligned
			(
				umint _Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static auto f_AllocSafeWithSize
			(
				umint &_Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> CAutoDestroy
		;
		inline_always only_parameters_aliased static auto f_AllocSafe
			(
				umint _Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> CAutoDestroy
		;
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		inline_always only_parameters_aliased static void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		inline_always only_parameters_aliased static void f_AllocBatchDebug
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		inline_always only_parameters_aliased static malloc_like auto f_Realloc
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static malloc_like auto f_ReallocDebug
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static auto f_Resize
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static auto f_ResizeDebug
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_always only_parameters_aliased static void f_Commit(void *_pMem, umint _Size);
		inline_always only_parameters_aliased static void f_Decommit(void *_pMem, umint _Size);
		inline_always only_parameters_aliased static void f_Free(void *_pBlock, umint _Size);
		inline_always only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
		// NOLINTEND(malterlib-function-naming)
	};
}

#include "Malterlib_Memory_Allocator_Placement.hpp"
