// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	CAllocator_Placement::CAllocator_Placement()
	{
	}

	bool CAllocator_Placement::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	bool CAllocator_Placement::f_OnlyOneAlloc()
	{
		return false;
	}

	inline_small umint CAllocator_Placement::f_StaticAddresses()
	{
		return 0;
	}

	umint CAllocator_Placement::f_GranularityAlloc(bool _bLargePages)
	{
		return 1;
	}
	umint CAllocator_Placement::f_GranularityCommit(bool _bLargePages)
	{
		return 1;
	}
	umint CAllocator_Placement::f_GranularityProtect(bool _bLargePages)
	{
		return 1;
	}
	only_parameters_aliased umint CAllocator_Placement::f_Size(void *_pBlock)
	{
		DMibPDebugBreak; // Not supported
		return 0;
	}
	only_parameters_aliased umint CAllocator_Placement::f_TrySize(void *_pBlock)
	{
		DMibPDebugBreak; // Not supported
		return 0;
	}
	umint CAllocator_Placement::f_SizePadded(umint _Size)
	{
		DMibPDebugBreak; // Not supported
		return 0;
	}
	fp32 CAllocator_Placement::f_Overhead(void const *_pBlock)
	{
		return 0.0;
	}

	constexpr inline_small bool CAllocator_Placement::f_CanCommit()
	{
		return false;
	}

	inline_small bool CAllocator_Placement::f_CanProtect()
	{
		return false;
	}

	inline_small bool CAllocator_Placement::f_DeterministicSize()
	{
		return true;
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{

	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Placement::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased inline_small auto CAllocator_Placement::f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		DMibPDebugBreak; // Not supported

		CAutoDestroy AutoDestroy;

		return fg_Move(AutoDestroy);
	}

	only_parameters_aliased inline_small auto CAllocator_Placement::f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		DMibPDebugBreak; // Not supported

		CAutoDestroy AutoDestroy;

		return fg_Move(AutoDestroy);
	}

	inline_small auto CAllocator_Placement::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
	}

	only_parameters_aliased malloc_like void *CAllocator_Placement::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}
	only_parameters_aliased malloc_like void *CAllocator_Placement::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}
	only_parameters_aliased void *CAllocator_Placement::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}
	only_parameters_aliased void *CAllocator_Placement::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibPDebugBreak; // Not supported
		return nullptr;
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_Commit(void *_pMem, umint _Size)
	{
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_Decommit(void *_pMem, umint _Size)
	{
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_Free(void *_pBlock, umint _Size)
	{
	}

	only_parameters_aliased inline_small void CAllocator_Placement::f_FreeNoSize(void *_pBlock)
	{
	}
}
