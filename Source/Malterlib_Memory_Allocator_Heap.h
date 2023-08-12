// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_Allocator_New.h"
#include "Malterlib_Memory_CaptureDefaultDelete.h"

namespace NMib::NMemory
{
	/************************************************************************************************\
	||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
	|| Heap alloc functions
	||______________________________________________________________________________________________||
	\************************************************************************************************/

	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocWithSize(mint &_Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocInitZeroWithSize(mint &_Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocInitZero(mint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedWithSize(mint &_Size, mint _Align);
	DMibMemory_MemoryManagerExport only_parameters_aliased void fg_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None);
	DMibMemory_MemoryManagerExport only_parameters_aliased void *fg_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None);
	DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_Size(const void *_pMemory);
	DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_TrySize(const void *_pMemory);
	DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_SizePadded(mint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased fp32 fg_Overhead(void const *_pMemory);
	DMibMemory_MemoryManagerExport only_parameters_aliased mint fg_Granularity();
	DMibMemory_MemoryManagerExport bool fg_AllocHasDeterministicSize();
	DMibMemory_MemoryManagerExport EMemoryManagerFeatureFlag fg_MemoryManagerFeatures();

#		if DMibConfig_MalterlibMemoryManager_Debug
			DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedWithSizeDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased void fg_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased void *fg_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None);
#		else
			only_parameters_aliased malloc_like static inline_small void *fg_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
			{
				return fg_AllocWithSize(_Size);
			}
			only_parameters_aliased malloc_like static inline_small void *fg_AllocAlignedWithSizeDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
			{
				return fg_AllocAlignedWithSize(_Size, _Align);
			}
			only_parameters_aliased malloc_like static inline_small void *fg_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None)
			{
				return fg_Realloc(_pMemory, _Size, _OldSize, _AllocFlags);
			}
			only_parameters_aliased static inline_small void *fg_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None)
			{
				return fg_Resize(_pMemory, _Size, _OldSize, _AllocFlags);
			}
#		endif

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

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Heap>;

		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static mint f_StaticAddresses();

		static mint f_GranularityAlloc(bool _bLargePages = false);
		static mint f_GranularityCommit(bool _bLargePages = false);
		static mint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased static mint f_Size(void *_pBlock);
		only_parameters_aliased static mint f_TrySize(void *_pBlock);
		static mint f_SizePadded(mint _Size);
		static fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		constexpr static bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
		only_parameters_aliased malloc_like static void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size);
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

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_NonTrackedHeap>;

		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static mint f_StaticAddresses();
		static mint f_GranularityAlloc(bool _bLargePages = false);
		static mint f_GranularityCommit(bool _bLargePages = false);
		static mint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased static mint f_Size(void *_pBlock);
		only_parameters_aliased static mint f_TrySize(void *_pBlock);
		static mint f_SizePadded(mint _Size);
		static fp32 f_Overhead(void const *_pBlock);
		constexpr static bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
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
		static CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size);
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

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<TCAllocator_Placement>;

		TCAllocator_Placement(void *_pPointer);
		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static mint f_StaticAddresses();
		static mint f_GranularityAlloc(bool _bLargePages = false);
		static mint f_GranularityCommit(bool _bLargePages = false);
		static mint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased static mint f_Size(void *_pBlock);
		only_parameters_aliased static mint f_TrySize(void *_pBlock);
		static mint f_SizePadded(mint _Size);
		static fp32 f_Overhead(void const *_pBlock);
		constexpr static bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
		only_parameters_aliased malloc_like void *f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size);
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

namespace NMib
{
	template <typename tf_CObjectType>
	static void fg_DeleteObject(NMemory::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		DMibFastCheck(0); // Delete not supported
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	static void fg_DeleteObjectDefiniteType(NMemory::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		DMibFastCheck(0); // Delete not supported
	}
}

namespace NMib::NMemory
{
	// Only support allocating one object
	template <mint t_StaticStorage, mint t_Alignment = TCConstantMax<mint, sizeof(void *), alignof(fp64)>::mc_Value, typename t_CFallbackAllocator = CAllocator_Heap>
	class TCAllocator_Static : public t_CFallbackAllocator
	{
	private:
		static const mint mcp_StorageSize = (t_StaticStorage + t_Alignment - 1) & ~mint(t_Alignment - 1);
		typedef uint8 CStorage[mcp_StorageSize];
		typedef typename NTraits::TCAlign<CStorage, t_Alignment>::CType CAlignedStorage;
		CAlignedStorage m_Storage;
#if DMibEnableSafeCheck > 0
		bool m_bAllocated;
#endif

		bool fp_IsStatic(void const *_pBlock) const;
		malloc_like void *fp_GetStatic();
	public:
		enum
		{
			mc_CanBeStatic = true
			, mc_bMethodsStatic = false
		};
		using CAutoDestroy = TCAllocator_AutoDestroy<TCAllocator_Static>;

		TCAllocator_Static();
		TCAllocator_Static(TCAllocator_Static &&_Other);
		TCAllocator_Static(TCAllocator_Static const &_Other);
		TCAllocator_Static & operator =(TCAllocator_Static &&_Other);
		TCAllocator_Static & operator =(TCAllocator_Static const &_Other);
		bool f_IsStatic(void const *_pBlock) const;
		static bool f_OnlyOneAlloc();
		mint f_StaticAddresses();
		mint f_GranularityAlloc(bool _bLargePages = false);
		mint f_GranularityCommit(bool _bLargePages = false);
		mint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased mint f_Size(void *_pBlock);
		only_parameters_aliased mint f_TrySize(void *_pBlock);
		mint f_SizePadded(mint _Size);
		fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		constexpr bool f_CanCommit();
		bool f_CanProtect();
		bool f_DeterministicSize();
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
		CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size);
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

namespace NMib
{
	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObject(NMemory::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}

	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObjectDefiniteType(NMemory::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject, mint _Alignment = 1)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}
}
