// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		class CAllocator_Placement
		{
		public:
			enum
			{
				mc_Reporting = false
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = false
			};

			typedef CDefaultPointerHolder CPtrHolder;

			struct CAutoDestroy : public CAllocator_AutoDestroy
			{
				CAutoDestroy(CAutoDestroy &&) = default;
				CAutoDestroy &operator =(CAutoDestroy &&) = default;
				CAutoDestroy() = default;
				~CAutoDestroy()
				{
					if (this->m_pMemory)
						f_Free(this->m_pMemory, this->m_Size);
				}
			};

			inline_always CAllocator_Placement();
			inline_always static bint f_IsStatic(void const *_pBlock);
			inline_always static bint f_OnlyOneAlloc();
			inline_always static mint f_StaticAddresses();
			inline_always static mint f_GranularityAlloc(bint _bLargePages = false);
			inline_always static mint f_GranularityCommit(bint _bLargePages = false);
			inline_always static mint f_GranularityProtect(bint _bLargePages = false);
			inline_always only_parameters_aliased static mint f_Size(void *_pBlock);
			inline_always only_parameters_aliased static mint f_TrySize(void *_pBlock);
			inline_always static mint f_SizePadded(mint _Size);
			inline_always static fp32 f_Overhead(void const *_pBlock);
			inline_always static bint f_CanCommit();
			inline_always static bint f_CanProtect();
			inline_always only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			inline_always  only_parameters_aliased return_not_aliased void *f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased void *f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased void *f_Alloc(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased void *f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased void *f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased void *f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased CAutoDestroy f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased return_not_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			inline_always only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			inline_always only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			inline_always only_parameters_aliased static void f_Free(void *_pBlock, mint _Size = 0);
		};		
	}
}

#include "Malterlib_Memory_Allocator_Placement.hpp"
