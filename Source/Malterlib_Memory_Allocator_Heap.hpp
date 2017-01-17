// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		inline_small bint CAllocator_Heap::f_IsStatic(void const *_pBlock)
		{
			return false;
		}

		inline_small bint CAllocator_Heap::f_OnlyOneAlloc()
		{
			return false;
		}

		inline_small mint CAllocator_Heap::f_StaticAddresses()
		{
			return 0;
		}

		inline_small mint CAllocator_Heap::f_GranularityAlloc(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		inline_small mint CAllocator_Heap::f_GranularityCommit(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		inline_small mint CAllocator_Heap::f_GranularityProtect(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		only_parameters_aliased inline_small mint CAllocator_Heap::f_Size(void *_pBlock)
		{
			return NMib::NMem::fg_Size(_pBlock);
		}

		only_parameters_aliased inline_small mint CAllocator_Heap::f_TrySize(void *_pBlock)
		{
			return NMib::NMem::fg_TrySize(_pBlock);
		}

		inline_small mint CAllocator_Heap::f_SizePadded(mint _Size)
		{
			return NMib::NMem::fg_SizePadded(_Size);
		}

		inline_small fp32 CAllocator_Heap::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			return NMib::NMem::fg_Overhead(_pBlock);
		}


		inline_small bint CAllocator_Heap::f_CanCommit()
		{
			return false;
		}

		inline_small bint CAllocator_Heap::f_CanProtect()
		{
			return false;
		}

		only_parameters_aliased inline_small void CAllocator_Heap::f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
#if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
#else
			return NMib::NMem::fg_Alloc(_Size);
#endif
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocDebug(Size, _pFile, _Line, _Flags, _AllocFlags);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAlignedDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
#if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::fg_AllocAlignedDebug(_Size, _Alignment, _pFile, _Line, _Flags);
#else
			return NMib::NMem::fg_AllocAligned(_Size, _Alignment);
#endif
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAlignedDebug(const mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocAlignedDebug(Size, _Alignment, _pFile, _Line, _Flags, _AllocFlags);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_Alloc(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_Alloc(Size, _AllocFlags);
		}
		
		only_parameters_aliased inline_small CAllocator_Heap::CAutoDestroy CAllocator_Heap::f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			CAllocator_Heap::CAutoDestroy AutoDestroy;
			AutoDestroy.m_pMemory = NMib::NMem::fg_AllocAligned(_Size, _Alignment);
			AutoDestroy.m_Size = _Size;
			
			return fg_Move(AutoDestroy);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_AllocAligned(_Size, _Alignment);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocAligned(Size, _Alignment, _AllocFlags);
		}
		
		only_parameters_aliased inline_small void CAllocator_Heap::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_AllocBatch(_Size, _Alignment, _Functor);
		}

		only_parameters_aliased inline_small void CAllocator_Heap::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
#if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::fg_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
#else
			return NMib::NMem::fg_AllocBatch(_Size, _Alignment, _Functor);
#endif
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_Realloc(_pMem, _Size);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_Heap::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_ReallocDebug(_pMem, _Size, _pFile, _Line, _Flags);
		}

		only_parameters_aliased inline_small void *CAllocator_Heap::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_Resize(_pMem, _Size);
		}

		only_parameters_aliased inline_small void *CAllocator_Heap::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return NMib::NMem::fg_ResizeDebug(_pMem, _Size, _pFile, _Line, _Flags);
		}


		only_parameters_aliased inline_small void CAllocator_Heap::f_Commit(void *_pMem, mint _Size)
		{
		}

		only_parameters_aliased inline_small void CAllocator_Heap::f_Decommit(void *_pMem, mint _Size)
		{
		}

		only_parameters_aliased inline_small void CAllocator_Heap::f_Free(void *_pBlock, mint _Size)
		{
			NMib::NMem::fg_Free(_pBlock);
		}

		///
		/// CAllocator_NonTrackedHeap
		///
		
		inline_small bint CAllocator_NonTrackedHeap::f_IsStatic(void const *_pBlock)
		{
			return false;
		}

		inline_small bint CAllocator_NonTrackedHeap::f_OnlyOneAlloc()
		{
			return false;
		}

		inline_small mint CAllocator_NonTrackedHeap::f_StaticAddresses()
		{
			return 0;
		}

		inline_small mint CAllocator_NonTrackedHeap::f_GranularityCommit(bint _bLargePages)
		{
			return f_GranularityAlloc();
		}
		inline_small mint CAllocator_NonTrackedHeap::f_GranularityProtect(bint _bLargePages)
		{
			return f_GranularityAlloc();
		}

		inline_small bint CAllocator_NonTrackedHeap::f_CanCommit()
		{
			return false;
		}

		inline_small bint CAllocator_NonTrackedHeap::f_CanProtect()
		{
			return false;
		}

		only_parameters_aliased inline_small void CAllocator_NonTrackedHeap::f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		inline_small only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocDebug(Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}

		inline_small only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAligendDebug(const mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocAlignedDebug(Size, _Alignment, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_NonTrackedHeap::f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_Alloc(Size, _AllocFlags);
		}

		only_parameters_aliased malloc_like inline_small void *CAllocator_NonTrackedHeap::f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocAligned(Size, _Alignment, _AllocFlags, _NumaNode);
		}
		
		only_parameters_aliased inline_small CAllocator_NonTrackedHeap::CAutoDestroy CAllocator_NonTrackedHeap::f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			CAllocator_NonTrackedHeap::CAutoDestroy AutoDestroy;
			AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment);
			AutoDestroy.m_Size = _Size;
			
			return fg_Move(AutoDestroy);
		}

		only_parameters_aliased inline_small void CAllocator_NonTrackedHeap::f_Commit(void *_pMem, mint _Size)
		{
		}

		only_parameters_aliased inline_small void CAllocator_NonTrackedHeap::f_Decommit(void *_pMem, mint _Size)
		{
		}
		

		///
		/// TCAllocator_Placement
		///

		template <mint t_AllocSize>
		inline_small void TCAllocator_Placement<t_AllocSize>::f_DoCheck(mint _Size)
		{
#ifdef DMibDebug
			DMibFastCheck(!m_bPointerTaken);
			m_bPointerTaken = true;
#endif
			DMibFastCheck(_Size == t_AllocSize);
		}

		template <mint t_AllocSize>
		TCAllocator_Placement<t_AllocSize>::TCAllocator_Placement(void *_pPointer)
			: m_pPointer(_pPointer)
#ifdef DMibDebug
			, m_bPointerTaken(false)
#endif
		{
		}

		template <mint t_AllocSize>
		bint TCAllocator_Placement<t_AllocSize>::f_IsStatic(void const *_pBlock)
		{
			return false;
		}

		template <mint t_AllocSize>
		bint TCAllocator_Placement<t_AllocSize>::f_OnlyOneAlloc()
		{
			return false;
		}

		template <mint t_AllocSize>
		inline_small mint TCAllocator_Placement<t_AllocSize>::f_StaticAddresses()
		{
			return 0;
		}

		template <mint t_AllocSize>
		mint TCAllocator_Placement<t_AllocSize>::f_GranularityAlloc(bint _bLargePages)
		{
			return 1;
		}
		template <mint t_AllocSize>
		mint TCAllocator_Placement<t_AllocSize>::f_GranularityCommit(bint _bLargePages)
		{
			return 1;
		}
		template <mint t_AllocSize>
		mint TCAllocator_Placement<t_AllocSize>::f_GranularityProtect(bint _bLargePages)
		{
			return 1;
		}
		template <mint t_AllocSize>
		only_parameters_aliased mint TCAllocator_Placement<t_AllocSize>::f_Size(void *_pBlock)
		{
			return t_AllocSize;
		}
		template <mint t_AllocSize>
		only_parameters_aliased mint TCAllocator_Placement<t_AllocSize>::f_TrySize(void *_pBlock)
		{
			DMibPDebugBreak; // Not supported
			return t_AllocSize;
		}
		template <mint t_AllocSize>
		mint TCAllocator_Placement<t_AllocSize>::f_SizePadded(mint _Size)
		{
			return t_AllocSize;
		}
		template <mint t_AllocSize>
		fp32 TCAllocator_Placement<t_AllocSize>::f_Overhead(void const *_pBlock)
		{
			return 0.0;
		}

		template <mint t_AllocSize>
		inline_small bint TCAllocator_Placement<t_AllocSize>::f_CanCommit()
		{
			return false;
		}

		template <mint t_AllocSize>
		inline_small bint TCAllocator_Placement<t_AllocSize>::f_CanProtect()
		{
			return false;
		}

		template <mint t_AllocSize>
		only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			return m_pPointer;
		}

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			return m_pPointer;
		}

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_Alloc(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			return m_pPointer;
		}

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			return m_pPointer;
		}

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			DMibFastCheck(((mint)m_pPointer & (_Alignment - 1)) == 0);
			return m_pPointer;
		}

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Placement<t_AllocSize>::f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			DMibFastCheck(((mint)m_pPointer & (_Alignment - 1)) == 0);
			return m_pPointer;
		}
		
		template <mint t_AllocSize>
		only_parameters_aliased inline_small auto TCAllocator_Placement<t_AllocSize>::f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
		{
			f_DoCheck(_Size);
			DMibFastCheck(((mint)m_pPointer & (_Alignment - 1)) == 0);
			
			CAutoDestroy AutoDestroy;
			AutoDestroy.m_pMemory = m_pPointer;
			AutoDestroy.m_Size = t_AllocSize;
			
			return fg_Move(AutoDestroy);
		}
		
		template <mint t_AllocSize>
		only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			f_DoCheck(_Size);
			DMibFastCheck(((mint)m_pPointer & (_Alignment - 1)) == 0);
			
			bool bRet = _Functor(m_pPointer, t_AllocSize);
			(void)bRet;
			DMibFastCheck(!bRet); // Has to abort after first alloc
		}			

		template <mint t_AllocSize>
		only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
		}			

		template <mint t_AllocSize>
		only_parameters_aliased malloc_like void *TCAllocator_Placement<t_AllocSize>::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibFastCheck(false);
		}
		template <mint t_AllocSize>
		only_parameters_aliased malloc_like void *TCAllocator_Placement<t_AllocSize>::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibFastCheck(false);
		}
		template <mint t_AllocSize>
		only_parameters_aliased void *TCAllocator_Placement<t_AllocSize>::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibFastCheck(false);
		}
		template <mint t_AllocSize>
		only_parameters_aliased void *TCAllocator_Placement<t_AllocSize>::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibFastCheck(false);
		}

		template <mint t_AllocSize>
		only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Commit(void *_pMem, mint _Size)
		{
		}

		template <mint t_AllocSize>
		only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Decommit(void *_pMem, mint _Size)
		{
		}

		template <mint t_AllocSize>
		only_parameters_aliased inline_small void TCAllocator_Placement<t_AllocSize>::f_Free(void *_pBlock, mint _Size)
		{
		}

		///
		/// TCAllocator_Static
		///
		
		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		bint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::fp_IsStatic(void const *_pBlock) const
		{
			return &m_Storage == _pBlock;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		malloc_like void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::fp_GetStatic()
		{
			return &m_Storage;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::TCAllocator_Static()
#if DMibEnableSafeCheck > 0
			: m_bAllocated(false)
#endif
		{
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::TCAllocator_Static(TCAllocator_Static&&_Other)
		{
#if DMibEnableSafeCheck > 0
			m_bAllocated = _Other.m_bAllocated;
			_Other.m_bAllocated = false;
#endif
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::TCAllocator_Static(TCAllocator_Static const &_Other)
		{
			// Do nothing
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>& TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::operator =(TCAllocator_Static&&_Other)
		{
#if DMibEnableSafeCheck > 0
			m_bAllocated = _Other.m_bAllocated;
			_Other.m_bAllocated = false;
#endif
			return *this;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>& TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::operator =(TCAllocator_Static const &_Other)
		{
			// Do nothing
			return *this;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		bint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_IsStatic(void const *_pBlock) const
		{
			return fp_IsStatic(_pBlock);
		}
		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		bint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_OnlyOneAlloc()
		{
			return true;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_StaticAddresses()
		{
			return 0;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_GranularityAlloc(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_GranularityCommit(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_GranularityProtect(bint _bLargePages)
		{
			return NMib::NMem::fg_Granularity();
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Size(void *_pBlock)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
#endif
			if (fp_IsStatic(_pBlock))
				return mcp_StorageSize;
			return t_CFallbackAllocator::f_Size(_pBlock);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_TrySize(void *_pBlock)
		{
#if DMibEnableSafeCheck > 0
			DMibFastCheck(m_bAllocated);
#endif
			if (fp_IsStatic(_pBlock))
				return mcp_StorageSize;
			return t_CFallbackAllocator::f_TrySize(_pBlock);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small mint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_SizePadded(mint _Size)
		{
			if (_Size <= mcp_StorageSize)
				return mcp_StorageSize;
			return t_CFallbackAllocator::f_SizePadded(_Size);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small fp32 TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			if (fp_IsStatic(_pBlock))
				return 0.0f;
			return t_CFallbackAllocator::f_Overhead(_pBlock);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small bint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_CanCommit()
		{
			return false;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		inline_small bint TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_CanProtect()
		{
			return false;
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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
			return t_CFallbackAllocator::f_AllocDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_AllocDebug(Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Alloc(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			mint Size = _Size;
			return f_Alloc(Size, _AllocFlags, _NumaNode);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			DMibFastCheck(_Alignment <= t_Alignment);
			mint Size = _Size;
			return f_AllocAligned(Size, _Alignment, _AllocFlags, _NumaNode);
		}
		
		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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
		
		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return f_AllocBatch(_Size, _Alignment, _Functor, _AllocFlags, _NumaNode);
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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
				return t_CFallbackAllocator::f_Alloc(_Size, _AllocFlags, _NumaNode);
			}
			else
			{
				if (_Size <= mcp_StorageSize)
				{
					t_CFallbackAllocator::f_Free(_pMem);
					_Size = mcp_StorageSize;
					return fp_GetStatic();
				}
				return t_CFallbackAllocator::f_Realloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
			}
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased malloc_like inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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
				return t_CFallbackAllocator::f_AllocDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
			}
			else
			{
				if (_Size <= mcp_StorageSize)
				{
					t_CFallbackAllocator::f_Free(_pMem);
					_Size = mcp_StorageSize;
					return fp_GetStatic();
				}
				return t_CFallbackAllocator::f_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
			}
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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
				mint OldSize = _Size;
				void *pMem = t_CFallbackAllocator::f_Alloc(_Size, _AllocFlags, _NumaNode);
				fg_MemCopy(pMem, fp_GetStatic(), fg_Min(_Size, OldSize));
			}
			else
			{
				if (_Size <= mcp_StorageSize)
				{
					_Size = mcp_StorageSize;
					mint OldSize = t_CFallbackAllocator::f_Size(_pMem);
					void *pMem = fp_GetStatic();
					fg_MemCopy(pMem, _pMem, fg_Min(_Size, OldSize));
					t_CFallbackAllocator::f_Free(_pMem);
					return pMem;
				}
				return t_CFallbackAllocator::f_Resize(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
			}
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void *TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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
				mint OldSize = _Size;
				void *pMem = t_CFallbackAllocator::f_AllocDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
				fg_MemCopy(pMem, fp_GetStatic(), fg_Min(_Size, OldSize));
			}
			else
			{
				if (_Size <= mcp_StorageSize)
				{
					_Size = mcp_StorageSize;
					mint OldSize = t_CFallbackAllocator::f_Size(_pMem);
					void *pMem = fp_GetStatic();
					fg_MemCopy(pMem, _pMem, fg_Min(_Size, OldSize));
					t_CFallbackAllocator::f_Free(_pMem);
					return pMem;
				}
				return t_CFallbackAllocator::f_ResizeDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
			}

			
		}


		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Commit(void *_pMem, mint _Size)
		{
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Decommit(void *_pMem, mint _Size)
		{
		}

		template <mint t_StaticStorage, mint t_Alignment, typename t_CFallbackAllocator>
		only_parameters_aliased inline_small void TCAllocator_Static<t_StaticStorage, t_Alignment, t_CFallbackAllocator>::f_Free(void *_pBlock, mint _Size)
		{
			if (fp_IsStatic(_pBlock))
			{
#if DMibEnableSafeCheck > 0
				DMibFastCheck(m_bAllocated);
				m_bAllocated = false;
#endif

				return;
			}
			NMib::NMem::fg_Free(_pBlock);
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
			: m_Deleter(_Other.m_Deleter)
		{
		}
		
		template <typename t_CDeleterType, typename t_CFallbackAllocator>
		TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter &&_Other)
			: m_Deleter(fg_Move(_Other.m_Deleter))
		{
		}
		
		template <typename t_CDeleterType, typename t_CFallbackAllocator>
		TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>& TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator>::operator= (TCAllocator_FunctorDeleter&& _Other)
		{
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
}

