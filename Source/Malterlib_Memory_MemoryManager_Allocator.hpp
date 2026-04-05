// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	TCAllocator_MemoryManager<t_CParams>::TCAllocator_MemoryManager(TCMemoryManager<t_CParams> *_pMemoryManager)
		: m_pMemoryManager(_pMemoryManager)
	{
	}

	template <typename t_CParams>
	inline bool TCAllocator_MemoryManager<t_CParams>::f_IsStatic(void const *_pBlock)
	{
		return false;
	}

	template <typename t_CParams>
	inline bool TCAllocator_MemoryManager<t_CParams>::f_OnlyOneAlloc()
	{
		return false;
	}

	template <typename t_CParams>
	inline_small umint TCAllocator_MemoryManager<t_CParams>::f_StaticAddresses()
	{
		return 0;
	}

	template <typename t_CParams>
	inline_small umint TCAllocator_MemoryManager<t_CParams>::f_GranularityAlloc(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	template <typename t_CParams>
	inline_small umint TCAllocator_MemoryManager<t_CParams>::f_GranularityCommit(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	template <typename t_CParams>
	inline_small umint TCAllocator_MemoryManager<t_CParams>::f_GranularityProtect(bool _bLargePages)
	{
		return NMib::NMemory::fg_Granularity();
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small umint TCAllocator_MemoryManager<t_CParams>::f_Size(void *_pBlock)
	{
		return m_pMemoryManager->f_Size(_pBlock);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small umint TCAllocator_MemoryManager<t_CParams>::f_TrySize(void *_pBlock)
	{
		return m_pMemoryManager->f_TrySize(_pBlock);
	}

	template <typename t_CParams>
	inline_small umint TCAllocator_MemoryManager<t_CParams>::f_SizePadded(umint _Size)
	{
		return m_pMemoryManager->f_SizePadded(_Size);
	}

	template <typename t_CParams>
	inline_small fp32 TCAllocator_MemoryManager<t_CParams>::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
	{
		return 0;
	}

	template <typename t_CParams>
	constexpr inline_small bool TCAllocator_MemoryManager<t_CParams>::f_CanCommit()
	{
		return false;
	}

	template <typename t_CParams>
	inline_small bool TCAllocator_MemoryManager<t_CParams>::f_CanProtect()
	{
		return false;
	}

	template <typename t_CParams>
	inline_small bool TCAllocator_MemoryManager<t_CParams>::f_DeterministicSize()
	{
		return true;
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{

	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_AllocWithSize(_Size);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_Alloc(_Size);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_AllocWithSize(_Size);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_Alloc(_Size);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_AllocAlignedWithSize(_Size, _Alignment);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_AllocAligned(_Size, _Alignment);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_AllocBatch(_Size, _Alignment, _Functor);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_AllocBatch(_Size, _Alignment, _Functor);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_Realloc(_pMem, _Size, _OldSize);
	}

	template <typename t_CParams>
	only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_Realloc(_pMem, _Size, _OldSize);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_Resize(_pMem, _Size, _OldSize);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return m_pMemoryManager->f_Resize(_pMem, _Size, _OldSize);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Commit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Decommit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Free(void *_pBlock, umint _Size)
	{
		m_pMemoryManager->f_Free(_pBlock, _Size);
	}

	template <typename t_CParams>
	only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_FreeNoSize(void *_pBlock)
	{
		m_pMemoryManager->f_FreeNoSize(_pBlock);
	}

	template <typename t_CParams>
	only_parameters_aliased auto TCAllocator_MemoryManager<t_CParams>::f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CParams>
	only_parameters_aliased auto TCAllocator_MemoryManager<t_CParams>::f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CParams>
	auto TCAllocator_MemoryManager<t_CParams>::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size, *this};
	}
}
