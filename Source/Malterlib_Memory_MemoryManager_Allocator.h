// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	class TCAllocator_MemoryManager : public CAllocator_Base
	{
		TCMemoryManager<t_CParams> *m_pMemoryManager;
	public:

		using CAutoDestroy = TCAllocator_AutoDestroy<TCAllocator_MemoryManager>;

		enum
		{
			mc_Reporting = false
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = false
		};

		TCAllocator_MemoryManager(TCMemoryManager<t_CParams> *_pMemoryManager);

		constexpr bool operator == (TCAllocator_MemoryManager const &_Right) const noexcept
		{
			return m_pMemoryManager == _Right.m_pMemoryManager;
		}

		constexpr auto operator <=> (TCAllocator_MemoryManager const &_Right) const noexcept
		{
			return m_pMemoryManager <=> _Right.m_pMemoryManager;
		}

		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static inline_small umint f_StaticAddresses();
		static inline_small umint f_GranularityAlloc(bool _bLargePages = false);
		static inline_small umint f_GranularityCommit(bool _bLargePages = false);
		static inline_small umint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased inline_small umint f_Size(void *_pBlock);
		only_parameters_aliased inline_small umint f_TrySize(void *_pBlock);
		umint f_SizePadded(umint _Size);
		static inline_small fp32 f_Overhead(void const *_pBlock);
		constexpr static inline_small bool f_CanCommit();
		static inline_small bool f_CanProtect();
		static inline_small bool f_DeterministicSize();
		only_parameters_aliased static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		only_parameters_aliased return_not_aliased inline_small void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased inline_small void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased inline_small void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased return_not_aliased inline_small void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased inline_small void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased inline_small void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static inline_small void f_Commit(void *_pMem, umint _Size);
		only_parameters_aliased static inline_small void f_Decommit(void *_pMem, umint _Size);
		only_parameters_aliased inline_small void f_Free(void *_pBlock, umint _Size);
		only_parameters_aliased inline_small void f_FreeNoSize(void *_pBlock);
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
	};
}

#include "Malterlib_Memory_MemoryManager_Allocator.hpp"
