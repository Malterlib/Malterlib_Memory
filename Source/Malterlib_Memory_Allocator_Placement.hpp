// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{

		CAllocator_Placement::CAllocator_Placement()
		{
		}

		bint CAllocator_Placement::f_IsStatic(void const *_pBlock)
		{
			return false;
		}

		bint CAllocator_Placement::f_OnlyOneAlloc()
		{
			return false;
		}

		inline_small mint CAllocator_Placement::f_StaticAddresses()
		{
			return 0;
		}

		mint CAllocator_Placement::f_GranularityAlloc(bint _bLargePages)
		{
			return 1;
		}
		mint CAllocator_Placement::f_GranularityCommit(bint _bLargePages)
		{
			return 1;
		}
		mint CAllocator_Placement::f_GranularityProtect(bint _bLargePages)
		{
			return 1;
		}
		only_parameters_aliased mint CAllocator_Placement::f_Size(void *_pBlock)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		only_parameters_aliased mint CAllocator_Placement::f_TrySize(void *_pBlock)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		mint CAllocator_Placement::f_SizePadded(mint _Size)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		fp32 CAllocator_Placement::f_Overhead(void const *_pBlock)
		{
			return 0.0;
		}

		inline_small bint CAllocator_Placement::f_CanCommit()
		{
			return false;
		}

		inline_small bint CAllocator_Placement::f_CanProtect()
		{
			return false;
		}

		only_parameters_aliased inline_small void CAllocator_Placement::f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_Alloc(mint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}
		
		only_parameters_aliased inline_small auto CAllocator_Placement::f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
		{
			DMibPDebugBreak; // Not supported

			CAutoDestroy AutoDestroy;

			return fg_Move(AutoDestroy);
		}

		only_parameters_aliased inline_small auto CAllocator_Placement::f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
		{
			DMibPDebugBreak; // Not supported

			CAutoDestroy AutoDestroy;

			return fg_Move(AutoDestroy);
		}

		inline_small auto CAllocator_Placement::f_MakeSafe(void *_pMemory, mint _Size) -> CAutoDestroy
		{
			return CAutoDestroy{_pMemory, _Size};
		}

		only_parameters_aliased inline_small void CAllocator_Placement::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
		}			

		only_parameters_aliased inline_small void CAllocator_Placement::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
		}			

		only_parameters_aliased malloc_like void *CAllocator_Placement::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}
		only_parameters_aliased malloc_like void *CAllocator_Placement::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}
		only_parameters_aliased void *CAllocator_Placement::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}
		only_parameters_aliased void *CAllocator_Placement::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibPDebugBreak; // Not supported
			return nullptr;
		}

		only_parameters_aliased inline_small void CAllocator_Placement::f_Commit(void *_pMem, mint _Size)
		{
		}

		only_parameters_aliased inline_small void CAllocator_Placement::f_Decommit(void *_pMem, mint _Size)
		{
		}

		only_parameters_aliased inline_small void CAllocator_Placement::f_Free(void *_pBlock, mint _Size)
		{
		}

		only_parameters_aliased inline_small void CAllocator_Placement::f_FreeNoSize(void *_pBlock)
		{
		}
	}
}

