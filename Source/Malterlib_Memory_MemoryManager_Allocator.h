// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
	
		template <typename t_CParams>
		class TCAllocator_MemoryManager
		{
			TCMemoryManager<t_CParams> *m_pMemoryManager;
		public:

			typedef CDefaultPointerHolder CPtrHolder;
			using CAutoDestroy = TCAllocator_AutoDestroy<TCAllocator_MemoryManager>;

			enum
			{
				mc_Reporting = false
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = false
			};
			
			TCAllocator_MemoryManager(TCMemoryManager<t_CParams> *_pMemoryManager);
			
			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static inline_small mint f_StaticAddresses();
			static inline_small mint f_GranularityAlloc(bint _bLargePages = false);
			static inline_small mint f_GranularityCommit(bint _bLargePages = false);
			static inline_small mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased inline_small mint f_Size(void *_pBlock);
			only_parameters_aliased inline_small mint f_TrySize(void *_pBlock);
			mint f_SizePadded(mint _Size);
			static inline_small fp32 f_Overhead(void const *_pBlock);
			static inline_small bint f_CanCommit();
			static inline_small bint f_CanProtect();
			only_parameters_aliased static inline_small void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased inline_small void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased inline_small void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased inline_small void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased inline_small void *f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased inline_small void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static inline_small void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static inline_small void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased inline_small void f_Free(void *_pBlock, mint _Size);
			only_parameters_aliased inline_small void f_FreeNoSize(void *_pBlock);
			only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		};
	}
}

#include "Malterlib_Memory_MemoryManager_Allocator.hpp"
