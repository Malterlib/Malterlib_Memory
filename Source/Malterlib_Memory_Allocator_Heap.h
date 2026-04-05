// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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

	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocWithSize(umint &_Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocInitZeroWithSize(umint &_Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocInitZero(umint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedWithSize(umint &_Size, umint _Align);
	DMibMemory_MemoryManagerExport only_parameters_aliased void fg_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_Realloc(void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None);
	DMibMemory_MemoryManagerExport only_parameters_aliased void *fg_Resize(void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None);
	DMibMemory_MemoryManagerExport only_parameters_aliased umint fg_Size(const void *_pMemory);
	DMibMemory_MemoryManagerExport only_parameters_aliased umint fg_TrySize(const void *_pMemory);
	DMibMemory_MemoryManagerExport only_parameters_aliased umint fg_SizePadded(umint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased fp32 fg_Overhead(void const *_pMemory);
	DMibMemory_MemoryManagerExport only_parameters_aliased umint fg_Granularity();
	DMibMemory_MemoryManagerExport bool fg_AllocHasDeterministicSize();
	DMibMemory_MemoryManagerExport EMemoryManagerFeatureFlag fg_MemoryManagerFeatures();

#		if DMibConfig_MalterlibMemoryManager_Debug
			DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedWithSizeDebug(umint &_Size, umint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased void fg_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_ReallocDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None);
			DMibMemory_MemoryManagerExport only_parameters_aliased void *fg_ResizeDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None);
#		else
			only_parameters_aliased malloc_like static inline_small void *fg_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
			{
				return fg_AllocWithSize(_Size);
			}
			only_parameters_aliased malloc_like static inline_small void *fg_AllocAlignedWithSizeDebug(umint &_Size, umint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
			{
				return fg_AllocAlignedWithSize(_Size, _Align);
			}
			only_parameters_aliased malloc_like static inline_small void *fg_ReallocDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None)
			{
				return fg_Realloc(_pMemory, _Size, _OldSize, _AllocFlags);
			}
			only_parameters_aliased static inline_small void *fg_ResizeDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None)
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

		constexpr bool operator == (CAllocator_Heap const &_Right) const noexcept
		{
			return true;
		}

		constexpr auto operator <=> (CAllocator_Heap const &_Right) const noexcept
		{
			return COrdering_Strong::equal;
		}

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Heap>;

		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static umint f_StaticAddresses();

		static umint f_GranularityAlloc(bool _bLargePages = false);
		static umint f_GranularityCommit(bool _bLargePages = false);
		static umint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased static umint f_Size(void *_pBlock);
		only_parameters_aliased static umint f_TrySize(void *_pBlock);
		static umint f_SizePadded(umint _Size);
		static fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		constexpr static bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		only_parameters_aliased static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		only_parameters_aliased malloc_like static void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedDebug(umint _Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		only_parameters_aliased malloc_like static void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void f_Commit(void *_pMem, umint _Size);
		only_parameters_aliased static void f_Decommit(void *_pMem, umint _Size);
		only_parameters_aliased static void f_Free(void *_pBlock, umint _Size);
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

		constexpr bool operator == (CAllocator_NonTrackedHeap const &_Right) const noexcept
		{
			return true;
		}

		constexpr auto operator <=> (CAllocator_NonTrackedHeap const &_Right) const noexcept
		{
			return COrdering_Strong::equal;
		}

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_NonTrackedHeap>;

		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static umint f_StaticAddresses();
		static umint f_GranularityAlloc(bool _bLargePages = false);
		static umint f_GranularityCommit(bool _bLargePages = false);
		static umint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased static umint f_Size(void *_pBlock);
		only_parameters_aliased static umint f_TrySize(void *_pBlock);
		static umint f_SizePadded(umint _Size);
		static fp32 f_Overhead(void const *_pBlock);
		constexpr static bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		only_parameters_aliased static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		only_parameters_aliased malloc_like static void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedDebug(umint _Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		only_parameters_aliased static void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void f_Commit(void *_pMem, umint _Size);
		only_parameters_aliased static void f_Decommit(void *_pMem, umint _Size);
		only_parameters_aliased static void f_Free(void *_pBlock, umint _Size);
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

	template <umint t_AllocSize>
	class TCAllocator_Placement : public CAllocator_Base
	{
		void *m_pPointer;
#ifdef DMibDebug
		bool m_bPointerTaken;
#endif

		void f_DoCheck(umint _Size);
	public:
		enum
		{
			mc_Reporting = false
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = false
		};

		constexpr bool operator == (TCAllocator_Placement const &_Right) const noexcept
		{
			return m_pPointer == _Right.m_pPointer;
		}

		constexpr auto operator <=> (TCAllocator_Placement const &_Right) const noexcept
		{
			return m_pPointer <=> _Right.m_pPointer;
		}

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<TCAllocator_Placement>;

		TCAllocator_Placement(void *_pPointer);
		TCAllocator_Placement(TCAllocator_Placement const &_Other) = delete;
		TCAllocator_Placement &operator = (TCAllocator_Placement const &_Other) = delete;
		TCAllocator_Placement(TCAllocator_Placement &&_Other) noexcept;
		TCAllocator_Placement &operator = (TCAllocator_Placement &&_Other) noexcept;
		static bool f_IsStatic(void const *_pBlock);
		static bool f_OnlyOneAlloc();
		static umint f_StaticAddresses();
		static umint f_GranularityAlloc(bool _bLargePages = false);
		static umint f_GranularityCommit(bool _bLargePages = false);
		static umint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased static umint f_Size(void *_pBlock);
		only_parameters_aliased static umint f_TrySize(void *_pBlock);
		static umint f_SizePadded(umint _Size);
		static fp32 f_Overhead(void const *_pBlock);
		constexpr static bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		only_parameters_aliased static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		only_parameters_aliased malloc_like void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		only_parameters_aliased void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like static void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased static void f_Commit(void *_pMem, umint _Size);
		only_parameters_aliased static void f_Decommit(void *_pMem, umint _Size);
		only_parameters_aliased static void f_Free(void *_pBlock, umint _Size);
		only_parameters_aliased static void f_FreeNoSize(void *_pBlock);
	};
}

namespace NMib
{
	template <typename tf_CObjectType>
	static void fg_DeleteObject(NMemory::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject)
	{
		DMibFastCheck(0); // Delete not supported
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	static void fg_DeleteObjectDefiniteType(NMemory::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject)
	{
		DMibFastCheck(0); // Delete not supported
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	static void fg_DeleteObjectDefiniteType(NMemory::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject, umint _Alignment)
	{
		DMibFastCheck(0); // Delete not supported
	}
}

namespace NMib::NMemory
{
	// Only support allocating one object
	template <umint t_StaticStorage, umint t_Alignment = gc_ConstantMax<umint, sizeof(void *), alignof(fp64)>, typename t_CFallbackAllocator = CAllocator_Heap>
	class TCAllocator_Static : public t_CFallbackAllocator
	{
	private:
		static const umint mcp_StorageSize = (t_StaticStorage + t_Alignment - 1) & ~umint(t_Alignment - 1);

		alignas(t_Alignment) uint8 m_Storage[mcp_StorageSize];
#if DMibEnableSafeCheck > 0
		bool m_bAllocated = false;
#endif

		bool fp_IsStatic(void const *_pBlock) const;
		malloc_like void *fp_GetStatic();
	public:
		enum
		{
			mc_CanBeStatic = true
			, mc_bMethodsStatic = false
		};

		constexpr bool operator == (TCAllocator_Static const &_Right) const noexcept
		{
			return static_cast<t_CFallbackAllocator const &>(*this) == static_cast<t_CFallbackAllocator const &>(_Right);
		}

		constexpr auto operator <=> (TCAllocator_Static const &_Right) const noexcept
		{
			return static_cast<t_CFallbackAllocator const &>(*this) <=> static_cast<t_CFallbackAllocator const &>(_Right);
		}

		using CAutoDestroy = TCAllocator_AutoDestroy<TCAllocator_Static>;

		TCAllocator_Static();
		TCAllocator_Static(TCAllocator_Static &&_Other);
		TCAllocator_Static(TCAllocator_Static const &_Other);
		TCAllocator_Static & operator =(TCAllocator_Static &&_Other);
		TCAllocator_Static & operator =(TCAllocator_Static const &_Other);
		bool f_IsStatic(void const *_pBlock) const;
		static bool f_OnlyOneAlloc();
		umint f_StaticAddresses();
		umint f_GranularityAlloc(bool _bLargePages = false);
		umint f_GranularityCommit(bool _bLargePages = false);
		umint f_GranularityProtect(bool _bLargePages = false);
		only_parameters_aliased umint f_Size(void *_pBlock);
		only_parameters_aliased umint f_TrySize(void *_pBlock);
		umint f_SizePadded(umint _Size);
		fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		constexpr bool f_CanCommit();
		bool f_CanProtect();
		bool f_DeterministicSize();
		only_parameters_aliased void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		only_parameters_aliased malloc_like void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased malloc_like void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased void f_Commit(void *_pMem, umint _Size);
		only_parameters_aliased void f_Decommit(void *_pMem, umint _Size);
		only_parameters_aliased void f_Free(void *_pBlock, umint _Size);
		only_parameters_aliased void f_FreeNoSize(void *_pBlock);
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
	};

	template <typename t_CDeleterType, typename t_CFallbackAllocator = CAllocator_Heap>
	class TCAllocator_FunctorDeleter : public t_CFallbackAllocator
	{
		t_CDeleterType m_Deleter;

	public:

		TCAllocator_FunctorDeleter();
		TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter const &_Other);
		TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter &&_Other);
		TCAllocator_FunctorDeleter &operator= (TCAllocator_FunctorDeleter const &_Other);
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
	static void fg_DeleteObject(NMemory::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}

	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObjectDefiniteType(NMemory::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}

	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObjectDefiniteType(NMemory::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject, umint _Alignment)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}
}
