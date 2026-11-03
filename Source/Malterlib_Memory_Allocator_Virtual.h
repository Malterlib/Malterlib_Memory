// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	/************************************************************************************************\
	||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
	|| Allocators
	||______________________________________________________________________________________________||
	\************************************************************************************************/

	struct CAllocator_Virtual : public CAllocator_Base
	{
		static constexpr aint mc_Reporting = true;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = true;

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Virtual>;

		constexpr bool operator == (CAllocator_Virtual const &_Right) const noexcept;
		constexpr auto operator <=> (CAllocator_Virtual const &_Right) const noexcept;

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static bool f_OnlyOneAlloc();
		static bool f_IsStatic(void const *_pBlock);
		static umint f_StaticAddresses();
		static umint f_GranularityAlloc(bool _bLargePages = false);
		static umint f_GranularityCommit(bool _bLargePages = false);
		static umint f_GranularityProtect(bool _bLargePages = false);
		static constexpr bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		// NOLINTEND(malterlib-function-naming)
		static ch8 ms_HeapName[];

#if DMibConfig_Memory_Shims_Enable
		struct CHeapInit
		{
			CHeapInit();
		};

		static CHeapInit ms_HeapInit;
#endif

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		static void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		static void f_AllocBatchDebug
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		static auto f_AllocWithSizeDebug
			(
				umint &_Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static auto f_AllocAlignedWithSizeDebug
			(
				umint &_Size
				, umint _Alignment
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static auto f_AllocDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static auto f_AllocAlignedDebug
			(
				umint _Size
				, umint _Alignment
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static auto f_ReallocDebug
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static auto f_ResizeDebug
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static void f_Commit(void *_pMem, umint _Size);
		static void f_Decommit(void *_pMem, umint _Size);
		static void f_Free(void *_pBlock, umint _Size);
		static void f_FreeNoSize(void *_pBlock);
		static umint f_Size(void const *_pBlock);
		static umint f_TrySize(void const *_pBlock);
		static umint f_SizePadded(umint _Size);
		static fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		only_parameters_aliased static auto f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			-> CAutoDestroy
		;
		only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		// NOLINTEND(malterlib-function-naming)
	};

	struct CAllocator_VirtualNoCommit : public CAllocator_Virtual
	{
		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static constexpr bool f_CanCommit();
		static bool f_CanProtect();
		static void f_Commit(void *_pMem, umint _Size);
		static void f_Decommit(void *_pMem, umint _Size);
		// NOLINTEND(malterlib-function-naming)
	};

	struct CAllocator_VirtualNoTracking : public CAllocator_Base
	{
		static constexpr aint mc_Reporting = false;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = true;

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_VirtualNoTracking>;

		constexpr bool operator == (CAllocator_VirtualNoTracking const &_Right) const noexcept = default;
		constexpr auto operator <=> (CAllocator_VirtualNoTracking const &_Right) const noexcept;

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static bool f_OnlyOneAlloc();
		static bool f_IsStatic(void const *_pBlock);
		static umint f_StaticAddresses();
		static umint f_GranularityAlloc(bool _bLargePages = false);
		static umint f_GranularityCommit(bool _bLargePages = false);
		static umint f_GranularityProtect(bool _bLargePages = false);
		static constexpr bool f_CanCommit();
		static bool f_CanProtect();
		static bool f_DeterministicSize();
		static void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		static void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		static void f_AllocBatchDebug
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		static auto f_AllocWithSizeDebug
			(
				umint &_Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static auto f_AllocAlignedWithSizeDebug
			(
				umint &_Size
				, umint _Alignment
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static auto f_AllocDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static auto f_AllocAlignedDebug
			(
				umint _Size
				, umint _Alignment
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static auto f_ReallocDebug
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static auto f_ResizeDebug
			(
				void *_pMem
				, umint &_Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static void f_Commit(void *_pMem, umint _Size);
		static void f_Decommit(void *_pMem, umint _Size);
		static void f_Free(void *_pBlock, umint _Size);
		static void f_FreeNoSize(void *_pBlock);
		static umint f_Size(void const *_pBlock);
		static umint f_TrySize(void const *_pBlock);
		static umint f_SizePadded(umint _Size);
		static fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		only_parameters_aliased static auto f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			-> CAutoDestroy
		;
		only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
		// NOLINTEND(malterlib-function-naming)
	};

	struct CAllocator_VirtualNoTrackingNoCommit : public CAllocator_VirtualNoTracking
	{
		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static constexpr bool f_CanCommit();
		static bool f_CanProtect();
		static void f_Commit(void *_pMem, umint _Size);
		static void f_Decommit(void *_pMem, umint _Size);
		// NOLINTEND(malterlib-function-naming)
	};
}
