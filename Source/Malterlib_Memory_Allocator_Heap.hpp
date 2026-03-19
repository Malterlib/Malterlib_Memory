// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	inline_small bool CAllocator_Heap::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	inline_small bool CAllocator_Heap::f_OnlyOneAlloc()
	{
		return false;
	}

	inline_small umint CAllocator_Heap::f_StaticAddresses()
	{
		return 0;
	}

	inline_small umint CAllocator_Heap::f_GranularityAlloc(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	inline_small umint CAllocator_Heap::f_GranularityCommit(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	inline_small umint CAllocator_Heap::f_GranularityProtect(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	only_parameters_aliased inline_small umint CAllocator_Heap::f_Size(void *_pBlock)
	{
		return NMib::NMemory::fg_Size(_pBlock);
	}

	only_parameters_aliased inline_small umint CAllocator_Heap::f_TrySize(void *_pBlock)
	{
		return NMib::NMemory::fg_TrySize(_pBlock);
	}

	inline_small umint CAllocator_Heap::f_SizePadded(umint _Size)
	{
		return NMib::NMemory::fg_SizePadded(_Size);
	}

	inline_small fp32 CAllocator_Heap::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
	{
		return NMib::NMemory::fg_Overhead(_pBlock);
	}


	constexpr inline_small bool CAllocator_Heap::f_CanCommit()
	{
		return false;
	}

	inline_small bool CAllocator_Heap::f_CanProtect()
	{
		return false;
	}

	inline_small bool CAllocator_Heap::f_DeterministicSize()
	{
		return NMib::NMemory::fg_AllocHasDeterministicSize();
	}

	only_parameters_aliased inline_small void CAllocator_Heap::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{

	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
#if DMibConfig_MalterlibMemoryManager_Debug
		return NMib::NMemory::fg_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
#else
		return NMib::NMemory::fg_AllocWithSize(_Size);
#endif
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
#if DMibConfig_MalterlibMemoryManager_Debug
		return NMib::NMemory::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
#else
		return NMib::NMemory::fg_Alloc(_Size);
#endif
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
#if DMibConfig_MalterlibMemoryManager_Debug
		return NMib::NMemory::fg_AllocAlignedWithSizeDebug(_Size, _Alignment, _pFile, _Line, _Flags);
#else
		return NMib::NMemory::fg_AllocAlignedWithSize(_Size, _Alignment);
#endif
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAlignedDebug(umint _Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
#if DMibConfig_MalterlibMemoryManager_Debug
		return NMib::NMemory::fg_AllocAlignedDebug(_Size, _Alignment, _pFile, _Line, _Flags);
#else
		return NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
#endif
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_AllocWithSize(_Size);
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_Alloc(_Size);
	}

	only_parameters_aliased inline_small CAllocator_Heap::CAutoDestroy CAllocator_Heap::f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = NMib::NMemory::fg_AllocAlignedWithSize(_Size, _Alignment);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	only_parameters_aliased inline_small CAllocator_Heap::CAutoDestroy CAllocator_Heap::f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	inline_small CAllocator_Heap::CAutoDestroy CAllocator_Heap::f_MakeSafe(void *_pMemory, umint _Size)
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_AllocAlignedWithSize(_Size, _Alignment);
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
	}

	only_parameters_aliased inline_small void CAllocator_Heap::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_AllocBatch(_Size, _Alignment, _Functor);
	}

	only_parameters_aliased inline_small void CAllocator_Heap::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
#if DMibConfig_MalterlibMemoryManager_Debug
		return NMib::NMemory::fg_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
#else
		return NMib::NMemory::fg_AllocBatch(_Size, _Alignment, _Functor);
#endif
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_Realloc(_pMem, _Size, _OldSize, _AllocFlags);
	}

	only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}

	only_parameters_aliased inline_small void *CAllocator_Heap::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_Resize(_pMem, _Size, _OldSize, _AllocFlags);
	}

	only_parameters_aliased inline_small void *CAllocator_Heap::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return NMib::NMemory::fg_ResizeDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}


	only_parameters_aliased inline_small void CAllocator_Heap::f_Commit(void *_pMem, umint _Size)
	{
	}

	only_parameters_aliased inline_small void CAllocator_Heap::f_Decommit(void *_pMem, umint _Size)
	{
	}

	only_parameters_aliased inline_small void CAllocator_Heap::f_Free(void *_pBlock, umint _Size)
	{
		NMib::NMemory::fg_Free(_pBlock, _Size);
	}

	only_parameters_aliased inline_small void CAllocator_Heap::f_FreeNoSize(void *_pBlock)
	{
		NMib::NMemory::fg_FreeNoSize(_pBlock);
	}

	///
	/// CAllocator_NonTrackedHeap
	///

	inline_small bool CAllocator_NonTrackedHeap::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	inline_small bool CAllocator_NonTrackedHeap::f_OnlyOneAlloc()
	{
		return false;
	}

	inline_small umint CAllocator_NonTrackedHeap::f_StaticAddresses()
	{
		return 0;
	}

	inline_small umint CAllocator_NonTrackedHeap::f_GranularityCommit(bool _bLargePages)
	{
		return f_GranularityAlloc();
	}
	inline_small umint CAllocator_NonTrackedHeap::f_GranularityProtect(bool _bLargePages)
	{
		return f_GranularityAlloc();
	}

	constexpr inline_small bool CAllocator_NonTrackedHeap::f_CanCommit()
	{
		return false;
	}

	inline_small bool CAllocator_NonTrackedHeap::f_CanProtect()
	{
		return false;
	}

	inline_small bool CAllocator_NonTrackedHeap::f_DeterministicSize()
	{
		return NMib::NMemory::fg_AllocHasDeterministicSize();
	}

	only_parameters_aliased inline_small void CAllocator_NonTrackedHeap::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{

	}

	inline_small only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		umint Size = _Size;
		return f_AllocWithSizeDebug(Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	inline_small only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedDebug(umint _Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		umint Size = _Size;
		return f_AllocAlignedWithSizeDebug(Size, _Alignment, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	only_parameters_aliased inline_small CAllocator_NonTrackedHeap::CAutoDestroy CAllocator_NonTrackedHeap::f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		CAllocator_NonTrackedHeap::CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	only_parameters_aliased inline_small CAllocator_NonTrackedHeap::CAutoDestroy CAllocator_NonTrackedHeap::f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		CAllocator_NonTrackedHeap::CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	inline_small CAllocator_NonTrackedHeap::CAutoDestroy CAllocator_NonTrackedHeap::f_MakeSafe(void *_pMemory, umint _Size)
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	only_parameters_aliased inline_small void CAllocator_NonTrackedHeap::f_Commit(void *_pMem, umint _Size)
	{
	}

	only_parameters_aliased inline_small void CAllocator_NonTrackedHeap::f_Decommit(void *_pMem, umint _Size)
	{
	}


	///
	/// TCAllocator_Placement
	///

	template <umint t_AllocSize>
	inline_small void TCAllocator_Placement<t_AllocSize>::f_DoCheck(umint _Size)
	{
		DMibFastCheck(m_pPointer);
#ifdef DMibDebug
		DMibFastCheck(!m_bPointerTaken);
		m_bPointerTaken = true;
#endif
		DMibFastCheck(_Size == t_AllocSize);
	}

	template <umint t_AllocSize>
	TCAllocator_Placement<t_AllocSize>::TCAllocator_Placement(void *_pPointer)
		: m_pPointer(_pPointer)
#ifdef DMibDebug
		, m_bPointerTaken(false)
#endif
	{
	}

	template <umint t_AllocSize>
	TCAllocator_Placement<t_AllocSize>::TCAllocator_Placement(TCAllocator_Placement &&_Other) noexcept
		: m_pPointer(fg_Exchange(_Other.m_pPointer, nullptr))
#ifdef DMibDebug
		, m_bPointerTaken(fg_Exchange(_Other.m_bPointerTaken, true))
#endif
	{
	}

	template <umint t_AllocSize>
	auto TCAllocator_Placement<t_AllocSize>::operator = (TCAllocator_Placement &&_Other) noexcept -> TCAllocator_Placement &
	{
		if (this == &_Other)
			return *this;

		m_pPointer = fg_Exchange(_Other.m_pPointer, nullptr);
#ifdef DMibDebug
		m_bPointerTaken = fg_Exchange(_Other.m_bPointerTaken, true);
#endif
		return *this;
	}

	template <umint t_AllocSize>
	bool TCAllocator_Placement<t_AllocSize>::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	template <umint t_AllocSize>
	bool TCAllocator_Placement<t_AllocSize>::f_OnlyOneAlloc()
	{
		return false;
	}

	template <umint t_AllocSize>
	inline_small umint TCAllocator_Placement<t_AllocSize>::f_StaticAddresses()
	{
		return 0;
	}

	template <umint t_AllocSize>
	umint TCAllocator_Placement<t_AllocSize>::f_GranularityAlloc(bool _bLargePages)
	{
		return 1;
	}
	template <umint t_AllocSize>
	umint TCAllocator_Placement<t_AllocSize>::f_GranularityCommit(bool _bLargePages)
	{
		return 1;
	}
	template <umint t_AllocSize>
	umint TCAllocator_Placement<t_AllocSize>::f_GranularityProtect(bool _bLargePages)
	{
		return 1;
	}
	template <umint t_AllocSize>
	only_parameters_aliased umint TCAllocator_Placement<t_AllocSize>::f_Size(void *_pBlock)
	{
		return t_AllocSize;
	}
	template <umint t_AllocSize>
	only_parameters_aliased umint TCAllocator_Placement<t_AllocSize>::f_TrySize(void *_pBlock)
	{
		DMibPDebugBreak; // Not supported
		return t_AllocSize;
	}
	template <umint t_AllocSize>
	umint TCAllocator_Placement<t_AllocSize>::f_SizePadded(umint _Size)
	{
		return t_AllocSize;
	}
	template <umint t_AllocSize>
	fp32 TCAllocator_Placement<t_AllocSize>::f_Overhead(void const *_pBlock)
	{
		return 0.0;
	}

	template <umint t_AllocSize>
	constexpr inline_small bool TCAllocator_Placement<t_AllocSize>::f_CanCommit()
	{
		return false;
	}

	template <umint t_AllocSize>
	inline_small bool TCAllocator_Placement<t_AllocSize>::f_CanProtect()
	{
		return false;
	}

	template <umint t_AllocSize>
	inline_small bool TCAllocator_Placement<t_AllocSize>::f_DeterministicSize()
	{
		return true;
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{

	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		return m_pPointer;
	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		return m_pPointer;
	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		return m_pPointer;
	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		return m_pPointer;
	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		DMibFastCheck(((umint)m_pPointer & (_Alignment - 1)) == 0);
		return m_pPointer;
	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		DMibFastCheck(((umint)m_pPointer & (_Alignment - 1)) == 0);
		return m_pPointer;
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small auto TCAllocator_Placement<t_AllocSize>::f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		f_DoCheck(_Size);
		DMibFastCheck(((umint)m_pPointer & (_Alignment - 1)) == 0);

		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = m_pPointer;
		AutoDestroy.m_Size = t_AllocSize;

		return fg_Move(AutoDestroy);
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small auto TCAllocator_Placement<t_AllocSize>::f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		f_DoCheck(_Size);
		DMibFastCheck(((umint)m_pPointer & (_Alignment - 1)) == 0);

		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = m_pPointer;
		AutoDestroy.m_Size = t_AllocSize;

		return fg_Move(AutoDestroy);
	}

	template <umint t_AllocSize>
	inline_small auto TCAllocator_Placement<t_AllocSize>::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		f_DoCheck(_Size);
		DMibFastCheck(((umint)m_pPointer & (_Alignment - 1)) == 0);

		bool bRet = _Functor(m_pPointer, t_AllocSize);
		(void)bRet;
		DMibFastCheck(!bRet); // Has to abort after first alloc
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
	}

	template <umint t_AllocSize>
	only_parameters_aliased malloc_like void *TCAllocator_Placement<t_AllocSize>::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(false);
	}
	template <umint t_AllocSize>
	only_parameters_aliased malloc_like void *TCAllocator_Placement<t_AllocSize>::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(false);
	}
	template <umint t_AllocSize>
	only_parameters_aliased void *TCAllocator_Placement<t_AllocSize>::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(false);
	}
	template <umint t_AllocSize>
	only_parameters_aliased void *TCAllocator_Placement<t_AllocSize>::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(false);
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Commit(void *_pMem, umint _Size)
	{
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Decommit(void *_pMem, umint _Size)
	{
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Free(void *_pBlock, umint _Size)
	{
	}

	template <umint t_AllocSize>
	only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_FreeNoSize(void *_pBlock)
	{
	}

	///
	/// TCAllocator_Static
	///

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	bool TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::fp_IsStatic(void const *_pBlock) const
	{
		return &m_Storage == _pBlock;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	malloc_like void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::fp_GetStatic()
	{
		return &m_Storage;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::TCAllocator_Static()
	{
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::TCAllocator_Static(TCAllocator_Static&&_Other)
		: t_CFallbackAllocator(fg_Move(static_cast<t_CFallbackAllocator &>(_Other)))
	{
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::TCAllocator_Static(TCAllocator_Static const &_Other)
		: t_CFallbackAllocator(static_cast<t_CFallbackAllocator const &>(_Other))
	{
		// Do nothing
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator> &TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::operator = (TCAllocator_Static &&_Other)
	{
		DMibFastCheck(!m_bAllocated && !_Other.m_bAllocated);
		static_cast<t_CFallbackAllocator &>(*this) = fg_Move(static_cast<t_CFallbackAllocator &>(_Other));
		return *this;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	auto TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::operator = (TCAllocator_Static const &_Other)
		-> TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator> &
	{
		DMibFastCheck(!m_bAllocated && !_Other.m_bAllocated);
		static_cast<t_CFallbackAllocator &>(*this) = static_cast<t_CFallbackAllocator const &>(_Other);
		return *this;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	bool TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_IsStatic(void const *_pBlock) const
	{
		return fp_IsStatic(_pBlock);
	}
	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	bool TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_OnlyOneAlloc()
	{
		return true;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_StaticAddresses()
	{
		return 0;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_GranularityAlloc(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_GranularityCommit(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_GranularityProtect(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Size(void *_pBlock)
	{
		DMibFastCheck(m_bAllocated);
		if (fp_IsStatic(_pBlock))
			return mcp_StorageSize;
		return t_CFallbackAllocator::f_Size(_pBlock);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_TrySize(void *_pBlock)
	{
		DMibFastCheck(m_bAllocated);
		if (fp_IsStatic(_pBlock))
			return mcp_StorageSize;
		return t_CFallbackAllocator::f_TrySize(_pBlock);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small umint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_SizePadded(umint _Size)
	{
		if (_Size <= mcp_StorageSize)
			return mcp_StorageSize;
		return t_CFallbackAllocator::f_SizePadded(_Size);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small fp32 TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
	{
		if (fp_IsStatic(_pBlock))
			return 0.0f;
		return t_CFallbackAllocator::f_Overhead(_pBlock);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	constexpr inline_small bool TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_CanCommit()
	{
		return false;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small bool TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_CanProtect()
	{
		return false;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small bool TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_DeterministicSize()
	{
		return true;
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{

	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{

		if (_Size <= mcp_StorageSize)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(!m_bAllocated);
			m_bAllocated = true;
#endif
			_Size = mcp_StorageSize;
			return fp_GetStatic();
		}
		return t_CFallbackAllocator::f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		umint Size = _Size;
		return f_AllocWithSizeDebug(Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_Size <= mcp_StorageSize)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(!m_bAllocated);
			m_bAllocated = true;
#endif
			_Size = mcp_StorageSize;
			return fp_GetStatic();
		}
		return t_CFallbackAllocator::f_AllocWithSize(_Size, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_Size <= mcp_StorageSize)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(!m_bAllocated);
			m_bAllocated = true;
#endif
			_Size = mcp_StorageSize;
			return fp_GetStatic();
		}
		return t_CFallbackAllocator::f_Alloc(_Size, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Alignment <= t_Alignment);
		if (_Size <= mcp_StorageSize)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(!m_bAllocated);
			m_bAllocated = true;
#endif
			_Size = mcp_StorageSize;
			return fp_GetStatic();
		}
		return t_CFallbackAllocator::f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Alignment <= t_Alignment);
		if (_Size <= mcp_StorageSize)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(!m_bAllocated);
			m_bAllocated = true;
#endif
			_Size = mcp_StorageSize;
			return fp_GetStatic();
		}
		return t_CFallbackAllocator::f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Alignment <= t_Alignment);
		if (_Size <= mcp_StorageSize)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(!m_bAllocated);
			m_bAllocated = true;
#endif
			bool bRet = _Functor(fp_GetStatic(), mcp_StorageSize);
			(void)bRet;
			DMibFastCheck(!bRet);
			return;
		}
		return t_CFallbackAllocator::f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (fp_IsStatic(_pMem))
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
#endif
			if (_Size <= mcp_StorageSize)
			{
				_Size = mcp_StorageSize;
				return fp_GetStatic();
			}
#if DMibEnableSafeCheck > 0
			m_bAllocated = false;
#endif
			return t_CFallbackAllocator::f_AllocWithSize(_Size, _AllocFlags, _NumaNode);
		}
		else
		{
			if (_Size <= mcp_StorageSize)
			{
				if (_OldSize)
					t_CFallbackAllocator::f_Free(_pMem, _OldSize);
				else
					t_CFallbackAllocator::f_FreeNoSize(_pMem);
				_Size = mcp_StorageSize;
				return fp_GetStatic();
			}
			return t_CFallbackAllocator::f_Realloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		}
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (fp_IsStatic(_pMem))
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
#endif
			if (_Size <= mcp_StorageSize)
			{
				_Size = mcp_StorageSize;
				return fp_GetStatic();
			}
#if DMibEnableSafeCheck > 0
			m_bAllocated = false;
#endif
			return t_CFallbackAllocator::f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}
		else
		{
			if (_Size <= mcp_StorageSize)
			{
				if (_OldSize)
					t_CFallbackAllocator::f_Free(_pMem, _OldSize);
				else
					t_CFallbackAllocator::f_FreeNoSize(_pMem);
				_Size = mcp_StorageSize;
				return fp_GetStatic();
			}
			return t_CFallbackAllocator::f_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (fp_IsStatic(_pMem))
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
#endif
			if (_Size <= mcp_StorageSize)
			{
				_Size = mcp_StorageSize;
				return fp_GetStatic();
			}
#if DMibEnableSafeCheck > 0
			m_bAllocated = false;
#endif
			void *pMem = t_CFallbackAllocator::f_AllocWithSize(_Size, _AllocFlags, _NumaNode);
			fg_MemCopy(pMem, fp_GetStatic(), fg_Min(_Size, mcp_StorageSize));
		}
		else
		{
			if (_Size <= mcp_StorageSize)
			{
				_Size = mcp_StorageSize;
				umint OldSize = _OldSize ? _OldSize : t_CFallbackAllocator::f_Size(_pMem);
				void *pMem = fp_GetStatic();
				fg_MemCopy(pMem, _pMem, fg_Min(_Size, OldSize));
				t_CFallbackAllocator::f_Free(_pMem, OldSize);
				return pMem;
			}
			return t_CFallbackAllocator::f_Resize(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		}
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (fp_IsStatic(_pMem))
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
#endif
			if (_Size <= mcp_StorageSize)
			{
				_Size = mcp_StorageSize;
				return fp_GetStatic();
			}
#if DMibEnableSafeCheck > 0
			m_bAllocated = false;
#endif
			void *pMem = t_CFallbackAllocator::f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
			fg_MemCopy(pMem, fp_GetStatic(), fg_Min(_Size, mcp_StorageSize));
		}
		else
		{
			if (_Size <= mcp_StorageSize)
			{
				_Size = mcp_StorageSize;
				umint OldSize = _OldSize ? _OldSize : t_CFallbackAllocator::f_Size(_pMem);
				void *pMem = fp_GetStatic();
				fg_MemCopy(pMem, _pMem, fg_Min(_Size, OldSize));
				t_CFallbackAllocator::f_Free(_pMem, OldSize);
				return pMem;
			}
			return t_CFallbackAllocator::f_ResizeDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}


	}


	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Commit(void *_pMem, umint _Size)
	{
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Decommit(void *_pMem, umint _Size)
	{
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Free(void *_pBlock, umint _Size)
	{
		if (fp_IsStatic(_pBlock))
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
			m_bAllocated = false;
#endif

			return;
		}
		NMib::NMemory::fg_Free(_pBlock, _Size);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_FreeNoSize(void *_pBlock)
	{
		if (fp_IsStatic(_pBlock))
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
			m_bAllocated = false;
#endif

			return;
		}
		NMib::NMemory::fg_FreeNoSize(_pBlock);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased auto TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocSafeWithSize
		(
			umint &_Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	only_parameters_aliased auto TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocSafe
		(
			umint _Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <umint t_StaticStorage, umint t_Alignment, typename t_CFallbackAllocator>
	inline_small auto TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size, *this};
	}

	///
	/// TCAllocator_FunctorDeleter
	///

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::TCAllocator_FunctorDeleter()
	{
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter const &_Other)
		: t_CFallbackAllocator(static_cast<t_CFallbackAllocator const &>(_Other))
		, m_Deleter(_Other.m_Deleter)
	{
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter &&_Other)
		: t_CFallbackAllocator(fg_Move(static_cast<t_CFallbackAllocator &>(_Other)))
		, m_Deleter(fg_Move(_Other.m_Deleter))
	{
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::operator= (TCAllocator_FunctorDeleter const &_Other)
	{
		static_cast<t_CFallbackAllocator &>(*this) = static_cast<t_CFallbackAllocator const &>(_Other);
		m_Deleter = _Other.m_Deleter;
		return *this;
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>& TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::operator= (TCAllocator_FunctorDeleter&& _Other)
	{
		static_cast<t_CFallbackAllocator &>(*this) = fg_Move(static_cast<t_CFallbackAllocator &>(_Other));
		m_Deleter = fg_Move(_Other.m_Deleter);
		return *this;
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	template <typename t_CParam0>
	TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::TCAllocator_FunctorDeleter(t_CParam0 &&_Param0)
		: m_Deleter(fg_Forward<t_CParam0>(_Param0))
	{
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	t_CDeleterType &TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::f_GetDeleter()
	{
		return m_Deleter;
	}

	template <typename t_CDeleterType, typename t_CFallbackAllocator>
	t_CDeleterType const &TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::f_GetDeleter() const
	{
		return m_Deleter;
	}
}

namespace NMib
{
	template <typename t_CType>
	inline_small void fg_Delete(t_CType * &_pToDelete)
	{
		static_assert(!NTraits::cIsAbstract<t_CType> || NTraits::cHasVirtualDestructor<t_CType>);
		if (_pToDelete)
		{
			fg_DeleteObject(NMemory::CDefaultAllocator(), _pToDelete);
			_pToDelete = nullptr;
		}
	}
}
