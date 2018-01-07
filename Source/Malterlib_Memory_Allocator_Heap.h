// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

#include "Malterlib_Memory_CaptureDefaultDelete.h"

namespace NMib
{
	namespace NMem
	{
		/************************************************************************************************\
		||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
		|| Heap alloc functions
		||______________________________________________________________________________________________||
		\************************************************************************************************/

#if defined(DMibMemoryOverrideDll)
#	define DMibMemory_MemoryManagerExport module_export
#else
#	define DMibMemory_MemoryManagerExport
#endif

		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocWithSize(mint &_Size);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_Alloc(mint _Size);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocInitZeroWithSize(mint &_Size);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocInitZero(mint _Size);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedWithSize(mint &_Size, mint _Align);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAligned(mint _Size, mint _Align);
		DMibMemory_MemoryManagerExport only_parameters_aliased void fg_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None);
		DMibMemory_MemoryManagerExport only_parameters_aliased void *fg_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None);
		DMibMemory_MemoryManagerExport only_parameters_aliased void fg_Free(void *_pMemory, mint _Size);
		DMibMemory_MemoryManagerExport only_parameters_aliased void fg_FreeNoSize(void *_pMemory);
		DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_Size(const void *_pMemory);
		DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_TrySize(const void *_pMemory);
		DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_SizePadded(mint _Size);
		DMibMemory_MemoryManagerExport only_parameters_aliased fp32 fg_Overhead(void const *_pMemory);
		DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_Granularity();
		
#			if DMibConfig_MalterlibMemoryManager_Debug
				DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedWithSizeDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedDebug(mint _Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				DMibMemory_MemoryManagerExport only_parameters_aliased void fg_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None);
				DMibMemory_MemoryManagerExport only_parameters_aliased void *fg_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None);
#			else
				only_parameters_aliased malloc_like static inline_small void *fg_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_AllocWithSize(_Size);
				}
				only_parameters_aliased malloc_like static inline_small void *fg_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_Alloc(_Size);
				}
				only_parameters_aliased malloc_like static inline_small void *fg_AllocAlignedWithSizeDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_AllocAlignedWithSize(_Size, _Align);
				}
				only_parameters_aliased malloc_like static inline_small void *fg_AllocAlignedDebug(mint _Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_AllocAligned(_Size, _Align);
				}
				only_parameters_aliased malloc_like static inline_small void *fg_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None)
				{
					return fg_Realloc(_pMemory, _Size, _OldSize, _AllocFlags);
				}
				only_parameters_aliased static inline_small void *fg_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None)
				{
					return fg_Resize(_pMemory, _Size, _OldSize, _AllocFlags);
				}
#			endif

		class CAllocator_Heap : public CAllocator_Base
		{
		public:

			enum
			{
				mc_bIsDefault = true
				, mc_Reporting = true
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = true
			};
			typedef CDefaultPointerHolder CPtrHolder;

			using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Heap>;

			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static mint f_StaticAddresses();
			
			static mint f_GranularityAlloc(bint _bLargePages = false);
			static mint f_GranularityCommit(bint _bLargePages = false);
			static mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased static mint f_Size(void *_pBlock);
			only_parameters_aliased static mint f_TrySize(void *_pBlock);
			static mint f_SizePadded(mint _Size);
			static fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
			static bint f_CanCommit();
			static bint f_CanProtect();
			only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased malloc_like static void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Free(void *_pBlock, mint _Size);
			only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
		};

		class CAllocator_NonTrackedHeap : public CAllocator_Base
		{
		public:
			enum
			{
				mc_Reporting = false
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = true
			};
			typedef CDefaultPointerHolder CPtrHolder;
			
			using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_NonTrackedHeap>;

			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static mint f_StaticAddresses();
			static mint f_GranularityAlloc(bint _bLargePages = false);
			static mint f_GranularityCommit(bint _bLargePages = false);
			static mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased static mint f_Size(void *_pBlock);
			only_parameters_aliased static mint f_TrySize(void *_pBlock);
			static mint f_SizePadded(mint _Size);
			static fp32 f_Overhead(void const *_pBlock);
			static bint f_CanCommit();
			static bint f_CanProtect();
			only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased malloc_like static void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Free(void *_pBlock, mint _Size);
			only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
		};

		class CAllocator_HeapNoDelete : public CAllocator_Heap
		{
		public:
			enum
			{
				mc_bIsDefault = false
			};
		};

		template <mint t_AllocSize>
		class TCAllocator_Placement : public CAllocator_Base
		{
			void *m_pPointer;
#ifdef DMibDebug
			bool m_bPointerTaken;
#endif

			void f_DoCheck(mint _Size);
		public:
			enum
			{
				mc_Reporting = false
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = false
			};

			typedef CDefaultPointerHolder CPtrHolder;

			using CAutoDestroy = TCAllocator_AutoDestroyStatic<TCAllocator_Placement>;

			TCAllocator_Placement(void *_pPointer);
			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static mint f_StaticAddresses();
			static mint f_GranularityAlloc(bint _bLargePages = false);
			static mint f_GranularityCommit(bint _bLargePages = false);
			static mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased static mint f_Size(void *_pBlock);
			only_parameters_aliased static mint f_TrySize(void *_pBlock);
			static mint f_SizePadded(mint _Size);
			static fp32 f_Overhead(void const *_pBlock);
			static bint f_CanCommit();
			static bint f_CanProtect();
			only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased malloc_like void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Free(void *_pBlock, mint _Size);
			only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
		};

	}

	template <typename tf_CObjectType>
	static void fg_DeleteObject(NMem::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		DMibFastCheck(0); // Delete not supported
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	static void fg_DeleteObjectDefiniteType(NMem::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		DMibFastCheck(0); // Delete not supported
	}

	namespace NMem
	{

		// Only support allocating one object
		template <mint t_StaticStorage, mint t_Alignment = TCConstantMax<mint, sizeof(void *), NTraits::TCAlignmentOf<fp64>::mc_Value>::mc_Value, typename t_CFallbackAllocator = CAllocator_Heap>
		class TCAllocator_Static : public t_CFallbackAllocator
		{
		private:
			static const mint mcp_StorageSize = (t_StaticStorage + t_Alignment - 1) & ~mint(t_Alignment - 1);
			typedef uint8 CStorage[mcp_StorageSize];
			typedef typename NTraits::TCAlign<CStorage, t_Alignment>::CType CAlignedStorage;
			CAlignedStorage m_Storage;
#if DMibEnableSafeCheck > 0
			bint m_bAllocated;
#endif

			bint fp_IsStatic(void const *_pBlock) const;
			malloc_like void *fp_GetStatic();
		public:
			enum
			{
				mc_CanBeStatic = true
				, mc_bMethodsStatic = false
			};
			typedef CDefaultPointerHolder CPtrHolder;

			using CAutoDestroy = TCAllocator_AutoDestroy<TCAllocator_Static>;

			TCAllocator_Static();
			TCAllocator_Static(TCAllocator_Static &&_Other);
			TCAllocator_Static(TCAllocator_Static const &_Other);
			TCAllocator_Static & operator =(TCAllocator_Static &&_Other);
			TCAllocator_Static & operator =(TCAllocator_Static const &_Other);
			bint f_IsStatic(void const *_pBlock) const;
			static bint f_OnlyOneAlloc();
			mint f_StaticAddresses();
			mint f_GranularityAlloc(bint _bLargePages = false);
			mint f_GranularityCommit(bint _bLargePages = false);
			mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased mint f_Size(void *_pBlock);
			only_parameters_aliased mint f_TrySize(void *_pBlock);
			mint f_SizePadded(mint _Size);
			fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
			bint f_CanCommit();
			bint f_CanProtect();
			only_parameters_aliased void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased malloc_like void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased malloc_like void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void *f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased void f_Free(void *_pBlock, mint _Size);
			only_parameters_aliased void f_FreeNoSize(void *_pBlock);
			only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		};

		template <typename t_CDeleterType, typename t_CFallbackAllocator = CAllocator_Heap>
		class TCAllocator_FunctorDeleter : public t_CFallbackAllocator
		{
			t_CDeleterType m_Deleter;

		public:
			
			TCAllocator_FunctorDeleter();
			TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter const &_Other);
			TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter &&_Other);
            TCAllocator_FunctorDeleter& operator= (TCAllocator_FunctorDeleter&& _Other);
			template <typename t_CParam0>
			TCAllocator_FunctorDeleter(t_CParam0 &&_Param0);
			t_CDeleterType &f_GetDeleter();
			t_CDeleterType const &f_GetDeleter() const;
		};
	}

	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObject(NMem::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}

	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObjectDefiniteType(NMem::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}
}



/************************************************************************************************\
||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
|| new operators
||______________________________________________________________________________________________||
\************************************************************************************************/

#ifndef DMibDefaultToolset

// Placement new
#ifndef __PLACEMENT_NEW_INLINE
#	define __PLACEMENT_NEW_INLINE

	only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, void * variable_not_aliased _pPlacement) noexcept
	{
		return _pPlacement;
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept
	{	
	}

#endif

#ifndef __PLACEMENT_VEC_NEW_INLINE
#	define __PLACEMENT_VEC_NEW_INLINE

	only_parameters_aliased malloc_like inline_always void * operator new [] (mint _Size, void * variable_not_aliased _pPlacement) noexcept
	{
		return _pPlacement;
	}

	only_parameters_aliased inline_always void operator delete [] (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept
	{	
	}

#endif

#endif


template <mint t_ArraySize>
only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, uint8 _Placement[t_ArraySize]) noexcept
{
	void * variable_not_aliased pValue = _Placement;
	return pValue;
}

template <mint t_ArraySize>
only_parameters_aliased inline_always void operator delete (void *_pToDelete, uint8 _Placement[t_ArraySize]) noexcept
{	
}


#ifdef DMibPOverrideOperatorNew
#	include <new>
	// Default new
#	if DMibPInlineActive > 0 && !defined(DMibNoInlineNew) && !defined(DCompiler_MSVC)
		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory) noexcept
		{
			if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::nothrow_t const &) noexcept
		{
			if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size) noexcept
		{
			if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, _Size))
				return;

			NMib::NMem::fg_Free(_pMemory, _Size);
		}


		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory) noexcept
		{
			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::nothrow_t const &) noexcept
		{
			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size) noexcept
		{
			NMib::NMem::fg_Free(_pMemory, _Size);
		}

		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment)
		{
			return NMib::NMem::fg_AllocAligned(_Size, (mint)_Alignment);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment) noexcept
		{
			if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
		{
			if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
		{
			mint Size = NMib::fg_AlignUp(_Size, (mint)_Alignment);
			if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, Size))
				return;

			NMib::NMem::fg_Free(_pMemory, Size);
		}


		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment)
		{
			return NMib::NMem::fg_AllocAligned(_Size, (mint)_Alignment);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment) noexcept
		{
			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
		{
			NMib::NMem::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
		{
			mint Size = NMib::fg_AlignUp(_Size, (mint)_Alignment);
			NMib::NMem::fg_Free(_pMemory, Size);
		}

#	else
		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size);
		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete(void *_pMemory) noexcept;
		only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size) noexcept;
		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size);
		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory) noexcept;
		only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size) noexcept;
		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment);
		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment) noexcept;
		only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept;
		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment);
		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment) noexcept;
		only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size, std::align_val_t) noexcept;
#	endif
#endif


// New with align

	struct CNewAligned
	{
		mint m_Alignment;
		CNewAligned(mint _Alignment)
			: m_Alignment(_Alignment)
		{
		}
	};

	only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, CNewAligned const& _Alignment)
	{
		return NMib::NMem::fg_AllocAligned(_Size, _Alignment.m_Alignment);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, CNewAligned const& _Alignment) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, CNewAligned const& _Alignment) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete, _Size);
	}

#if DMibConfig_MalterlibMemoryManager_Debug

	only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMem::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased malloc_like inline_always void * operator new[] (mint _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMem::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete, _Size);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete, _Size);
	}

	only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, CNewAligned const& _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMem::fg_AllocAlignedDebug(_Size, _Alignment.m_Alignment, _pFile, _Line, _Flags);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, CNewAligned const& _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, CNewAligned const& _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete, _Size);
	}

#endif

