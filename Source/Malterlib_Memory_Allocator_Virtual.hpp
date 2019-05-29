// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	///
	/// CAllocator_Virtual
	///

	inline_small bool CAllocator_Virtual::f_OnlyOneAlloc()
	{
		return false;
	}

	inline_small bool CAllocator_Virtual::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	inline_small mint CAllocator_Virtual::f_StaticAddresses()
	{
		return 1;
	}

	inline_small mint CAllocator_Virtual::f_GranularityAlloc(bool _bLargePages)
	{
		return NSys::fg_Mem_VirtualGranularityAlloc(_bLargePages);
	}
	inline_small mint CAllocator_Virtual::f_GranularityCommit(bool _bLargePages)
	{
		return NSys::fg_Mem_VirtualGranularityCommit(_bLargePages);
	}
	inline_small mint CAllocator_Virtual::f_GranularityProtect(bool _bLargePages)
	{
		return NSys::fg_Mem_VirtualGranularityProtect(_bLargePages);
	}

	inline_small bool CAllocator_Virtual::f_CanCommit()
	{
		return NSys::fg_Mem_VirtualCanCommit();
	}

	inline_small bool CAllocator_Virtual::f_CanProtect()
	{
		return NSys::fg_Mem_VirtualCanProtect();
	}

	inline_small void CAllocator_Virtual::f_Protect(void *_pMem, mint _Size, uaint _Protect)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		NSys::fg_Mem_VirtualProtect(_pMem, _Size, _Protect);
		DMibMemoryReportProtect(ms_HeapName, ms_HeapName, _pMem, _Size, _Protect);
	}

	inline_small void *CAllocator_Virtual::f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode);
		DMibMemoryReportAlloc(ms_HeapName, ms_HeapName, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode, _Alignment);
		DMibMemoryReportAlloc(ms_HeapName, ms_HeapName, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_Alloc(mint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocWithSize(_Size, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_Virtual::f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
	}

	inline_small void CAllocator_Virtual::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		while (true)
		{
			mint Size = _Size;
			void * pMem = f_AllocAlignedWithSize(Size, _Alignment);
			if (!_Functor(pMem, Size))
				break;
		}
	}

	inline_small void CAllocator_Virtual::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_Virtual::f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode);
		DMibMemoryReportAlloc(ms_HeapName, ms_HeapName, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode, _Alignment);
		DMibMemoryReportAlloc(ms_HeapName, ms_HeapName, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_Virtual::f_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocAlignedWithSizeDebug(_Size, _Alignment, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_Virtual::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualRealloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		DMibMemoryReportRealloc(ms_HeapName, ms_HeapName, _pMem, _OldSize, nullptr, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualRealloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		DMibMemoryReportRealloc(ms_HeapName, ms_HeapName, _pMem, _OldSize, nullptr, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualResize(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		DMibMemoryReportResize(ms_HeapName, ms_HeapName, _pMem, _OldSize, nullptr, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void *CAllocator_Virtual::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = NSys::fg_Mem_VirtualResize(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		DMibMemoryReportResize(ms_HeapName, ms_HeapName, _pMem, _OldSize, nullptr, pRet, 0, RequestedSize, _Size, f_Overhead(pRet), nullptr);
		return pRet;
	}

	inline_small void CAllocator_Virtual::f_Commit(void *_pMem, mint _Size)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		NSys::fg_Mem_VirtualCommit(_pMem, _Size);
		DMibMemoryReportCommit(ms_HeapName, ms_HeapName, _pMem, _Size);
	}

	inline_small void CAllocator_Virtual::f_Decommit(void *_pMem, mint _Size)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		NSys::fg_Mem_VirtualDecommit(_pMem, _Size);
		DMibMemoryReportDecommit(ms_HeapName, ms_HeapName, _pMem, _Size);
	}

	inline_small void CAllocator_Virtual::f_Free(void *_pBlock, mint _Size)
	{
		DMibFastCheck(_Size != 0);
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		NSys::fg_Mem_VirtualFree(_pBlock, _Size);
		DMibMemoryReportFree(ms_HeapName, ms_HeapName, _pBlock, _Size, nullptr);
	}

	inline_small void CAllocator_Virtual::f_FreeNoSize(void *_pBlock)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		DMibMemoryReportSaveVar(Size, f_Size(_pBlock));
		NSys::fg_Mem_VirtualFree(_pBlock, 0);
		DMibMemoryReportFree(ms_HeapName, ms_HeapName, _pBlock, Size, nullptr);
	}

	inline_small mint CAllocator_Virtual::f_Size(const void *_pBlock)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		mint Ret = NSys::fg_Mem_VirtualSize(_pBlock);
		DMibMemoryReportGetSize(ms_HeapName, ms_HeapName, _pBlock, Ret, nullptr);
		return Ret;
	}

	inline_small mint CAllocator_Virtual::f_TrySize(const void *_pBlock)
	{
		DMibMemoryGoingToReportScope(ms_HeapName, true);
		mint Ret = NSys::fg_Mem_VirtualTrySize(_pBlock);
		DMibMemoryReportGetSize(ms_HeapName, ms_HeapName, _pBlock, Ret, nullptr);
		return Ret;
	}

	inline_small mint CAllocator_Virtual::f_SizePadded(mint _Size)
	{
		return fg_AlignUp(_Size, f_GranularityAlloc());
	}

	inline_small fp32 CAllocator_Virtual::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
	{
		return NSys::fg_Mem_VirtualOverhead(_pBlock);
	}

	inline_small only_parameters_aliased auto CAllocator_Virtual::f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	inline_small only_parameters_aliased auto CAllocator_Virtual::f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	inline_small auto CAllocator_Virtual::f_MakeSafe(void *_pMemory, mint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	///
	/// CAllocator_VirtualNoCommit
	///

	inline_small bool CAllocator_VirtualNoCommit::f_CanCommit()
	{
		return 0;
	}

	inline_small bool CAllocator_VirtualNoCommit::f_CanProtect()
	{
		return 0;
	}
	inline_small void CAllocator_VirtualNoCommit::f_Commit(void *_pMem, mint _Size)
	{
		DMibFastCheck(false);
	}

	inline_small void CAllocator_VirtualNoCommit::f_Decommit(void *_pMem, mint _Size)
	{
		DMibFastCheck(false);
	}

	///
	/// CAllocator_VirtualNoTracking
	///
	inline_small bool CAllocator_VirtualNoTracking::f_OnlyOneAlloc()
	{
		return false;
	}

	inline_small bool CAllocator_VirtualNoTracking::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	inline_small mint CAllocator_VirtualNoTracking::f_StaticAddresses()
	{
		return 1;
	}

	inline_small mint CAllocator_VirtualNoTracking::f_GranularityAlloc(bool _bLargePages)
	{
		return NSys::fg_Mem_VirtualGranularityAlloc(_bLargePages);
	}
	inline_small mint CAllocator_VirtualNoTracking::f_GranularityCommit(bool _bLargePages)
	{
		return NSys::fg_Mem_VirtualGranularityCommit(_bLargePages);
	}
	inline_small mint CAllocator_VirtualNoTracking::f_GranularityProtect(bool _bLargePages)
	{
		return NSys::fg_Mem_VirtualGranularityProtect(_bLargePages);
	}

	inline_small bool CAllocator_VirtualNoTracking::f_CanCommit()
	{
		return NSys::fg_Mem_VirtualCanCommit();
	}

	inline_small bool CAllocator_VirtualNoTracking::f_CanProtect()
	{
		return NSys::fg_Mem_VirtualCanProtect();
	}

	inline_small void CAllocator_VirtualNoTracking::f_Protect(void *_pMem, mint _Size, uaint _Protect)
	{
		return NSys::fg_Mem_VirtualProtect(_pMem, _Size, _Protect);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode, _Alignment);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_Alloc(mint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode, _Alignment);
	}

	inline_small void CAllocator_VirtualNoTracking::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		while (true)
		{
			mint Size = _Size;
			void * pMem = f_AllocAlignedWithSize(Size, _Alignment);
			if (!_Functor(pMem, Size))
				break;
		}
	}

	inline_small void CAllocator_VirtualNoTracking::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode);
	}


	inline_small void *CAllocator_VirtualNoTracking::f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode, _Alignment);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualAlloc(_Size, _AllocFlags, _NumaNode, _Alignment);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualRealloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualRealloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualResize(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
	}

	inline_small void *CAllocator_VirtualNoTracking::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NSys::fg_Mem_VirtualResize(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
	}

	inline_small void CAllocator_VirtualNoTracking::f_Commit(void *_pMem, mint _Size)
	{
		NSys::fg_Mem_VirtualCommit(_pMem, _Size);
	}

	inline_small void CAllocator_VirtualNoTracking::f_Decommit(void *_pMem, mint _Size)
	{
		NSys::fg_Mem_VirtualDecommit(_pMem, _Size);
	}

	inline_small void CAllocator_VirtualNoTracking::f_Free(void *_pBlock, mint _Size)
	{
		DMibFastCheck(_Size != 0);
		NSys::fg_Mem_VirtualFree(_pBlock, _Size);
	}

	inline_small void CAllocator_VirtualNoTracking::f_FreeNoSize(void *_pBlock)
	{
		NSys::fg_Mem_VirtualFree(_pBlock, 0);
	}

	inline_small mint CAllocator_VirtualNoTracking::f_Size(const void *_pBlock)
	{
		return NSys::fg_Mem_VirtualSize(_pBlock);
	}

	inline_small mint CAllocator_VirtualNoTracking::f_TrySize(const void *_pBlock)
	{
		return NSys::fg_Mem_VirtualTrySize(_pBlock);
	}

	inline_small mint CAllocator_VirtualNoTracking::f_SizePadded(mint _Size)
	{
		return fg_AlignUp(_Size, f_GranularityAlloc());
	}

	inline_small fp32 CAllocator_VirtualNoTracking::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
	{
		return NSys::fg_Mem_VirtualOverhead(_pBlock);
	}

	inline_small only_parameters_aliased auto CAllocator_VirtualNoTracking::f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	inline_small only_parameters_aliased auto CAllocator_VirtualNoTracking::f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	inline_small auto CAllocator_VirtualNoTracking::f_MakeSafe(void *_pMemory, mint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	///
	/// CAllocator_VirtualNoTrackingNoCommit
	///

	inline_small bool CAllocator_VirtualNoTrackingNoCommit::f_CanCommit()
	{
		return 0;
	}

	inline_small bool CAllocator_VirtualNoTrackingNoCommit::f_CanProtect()
	{
		return 0;
	}

	inline_small void CAllocator_VirtualNoTrackingNoCommit::f_Commit(void *_pMem, mint _Size)
	{
		DMibFastCheck(false);
	}

	inline_small void CAllocator_VirtualNoTrackingNoCommit::f_Decommit(void *_pMem, mint _Size)
	{
		DMibFastCheck(false);
	}
}
