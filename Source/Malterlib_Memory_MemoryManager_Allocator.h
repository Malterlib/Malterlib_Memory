// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	struct TCAllocator_MemoryManager : public CAllocator_Base
	{
		using CAutoDestroy = TCAllocator_AutoDestroy<TCAllocator_MemoryManager>;

		TCAllocator_MemoryManager(TCMemoryManager<t_CParams> *_pMemoryManager);

		constexpr bool operator == (TCAllocator_MemoryManager const &_Right) const noexcept;
		constexpr auto operator <=> (TCAllocator_MemoryManager const &_Right) const noexcept;

		DMibNamedAsMemberFunction static bool f_IsStatic(void const *_pBlock);
		DMibNamedAsMemberFunction static bool f_OnlyOneAlloc();
		DMibNamedAsMemberFunction static inline_small umint f_StaticAddresses();
		DMibNamedAsMemberFunction static inline_small umint f_GranularityAlloc(bool _bLargePages = false);
		DMibNamedAsMemberFunction static inline_small umint f_GranularityCommit(bool _bLargePages = false);
		DMibNamedAsMemberFunction static inline_small umint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased inline_small umint f_Size(void *_pBlock);
		only_parameters_aliased inline_small umint f_TrySize(void *_pBlock);
		umint f_SizePadded(umint _Size);
		DMibNamedAsMemberFunction static inline_small fp32 f_Overhead(void const *_pBlock);
		DMibNamedAsMemberFunction static constexpr inline_small bool f_CanCommit();
		DMibNamedAsMemberFunction static inline_small bool f_CanProtect();
		DMibNamedAsMemberFunction static inline_small bool f_DeterministicSize();
		DMibNamedAsMemberFunction only_parameters_aliased static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		only_parameters_aliased return_not_aliased inline_small auto f_AllocWithSizeDebug
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
		only_parameters_aliased return_not_aliased inline_small auto f_AllocDebug
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
		only_parameters_aliased return_not_aliased inline_small auto f_AllocWithSize
			(
				umint &_Size
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		only_parameters_aliased return_not_aliased inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small auto f_AllocAlignedWithSize
			(
				umint &_Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		only_parameters_aliased return_not_aliased inline_small auto f_AllocAligned
			(
				umint _Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		only_parameters_aliased inline_small void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		only_parameters_aliased inline_small void f_AllocBatchDebug
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
		only_parameters_aliased return_not_aliased inline_small auto f_Realloc
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		only_parameters_aliased return_not_aliased inline_small auto f_ReallocDebug
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
		only_parameters_aliased inline_small auto f_Resize
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		only_parameters_aliased inline_small auto f_ResizeDebug
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
		DMibNamedAsMemberFunction only_parameters_aliased static inline_small void f_Commit(void *_pMem, umint _Size);
		DMibNamedAsMemberFunction only_parameters_aliased static inline_small void f_Decommit(void *_pMem, umint _Size);
		only_parameters_aliased inline_small void f_Free(void *_pBlock, umint _Size);
		only_parameters_aliased inline_small void f_FreeNoSize(void *_pBlock);
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);

		static constexpr aint mc_Reporting = false;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = false;

	private:
		TCMemoryManager<t_CParams> *mp_pMemoryManager;
	};
}

#include "Malterlib_Memory_MemoryManager_Allocator.hpp"
