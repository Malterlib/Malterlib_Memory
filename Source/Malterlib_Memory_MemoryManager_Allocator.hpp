// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
	
		template <typename t_CParams>
		TCAllocator_MemoryManager<t_CParams>::TCAllocator_MemoryManager(TCMemoryManager<t_CParams> *_pMemoryManager)
			: m_pMemoryManager(_pMemoryManager)
		{
		}

		template <typename t_CParams>
		inline bint TCAllocator_MemoryManager<t_CParams>::f_IsStatic(void const *_pBlock)
		{
			return false;
		}

		template <typename t_CParams>
		inline bint TCAllocator_MemoryManager<t_CParams>::f_OnlyOneAlloc()
		{
			return false;
		}

		template <typename t_CParams>
		inline_small mint TCAllocator_MemoryManager<t_CParams>::f_StaticAddresses()
		{
			return 0;
		}

		template <typename t_CParams>
		inline_small mint TCAllocator_MemoryManager<t_CParams>::f_GranularityAlloc(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		template <typename t_CParams>
		inline_small mint TCAllocator_MemoryManager<t_CParams>::f_GranularityCommit(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		template <typename t_CParams>
		inline_small mint TCAllocator_MemoryManager<t_CParams>::f_GranularityProtect(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		template <typename t_CParams>
		only_parameters_aliased inline_small mint TCAllocator_MemoryManager<t_CParams>::f_Size(void *_pBlock)
		{
			return m_pMemoryManager->f_Size(_pBlock);
		}

		template <typename t_CParams>
		only_parameters_aliased inline_small mint TCAllocator_MemoryManager<t_CParams>::f_TrySize(void *_pBlock)
		{
			return m_pMemoryManager->f_TrySize(_pBlock);
		}

		template <typename t_CParams>
		inline_small mint TCAllocator_MemoryManager<t_CParams>::f_SizePadded(mint _Size)
		{
			return m_pMemoryManager->f_SizePadded(_Size);
		}

		template <typename t_CParams>
		inline_small fp32 TCAllocator_MemoryManager<t_CParams>::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			return 0;
		}

		template <typename t_CParams>
		inline_small bint TCAllocator_MemoryManager<t_CParams>::f_CanCommit()
		{
			return false;
		}

		template <typename t_CParams>
		inline_small bint f_CanProtect()
		{
			return false;
		}

		template <typename t_CParams>
		only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_Alloc(_Size);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return m_pMemoryManager->f_Alloc(Size);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Alloc(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_Alloc(_Size);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_Alloc(Size, _AllocFlags);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_AllocAligned(_Size, _Alignment);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return m_pMemoryManager->f_AllocAligned(Size, _Alignment);
		}

		template <typename t_CParams>
		only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_AllocBatch(_Size, _Alignment, _Functor);
		}
		
		template <typename t_CParams>
		only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_AllocBatch(_Size, _Alignment, _Functor);
		}
		
		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_Realloc(_pMem, _Size);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_Realloc(_pMem, _Size);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_Resize(_pMem, _Size);
		}

		template <typename t_CParams>
		only_parameters_aliased return_not_aliased inline_small void *TCAllocator_MemoryManager<t_CParams>::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return m_pMemoryManager->f_Resize(_Size);
		}
		
		template <typename t_CParams>
		only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Commit(void *_pMem, mint _Size)
		{
		}

		template <typename t_CParams>
		only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Decommit(void *_pMem, mint _Size)
		{
		}

		template <typename t_CParams>
		only_parameters_aliased inline_small void TCAllocator_MemoryManager<t_CParams>::f_Free(void *_pBlock, mint _Size)
		{
			m_pMemoryManager->f_Free(_pBlock);
		}
	}
}

