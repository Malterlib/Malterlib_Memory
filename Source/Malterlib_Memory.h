// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>
#include <Mib/Atomic/Atomic>

namespace NMib::NMemory
{
	enum EMemoryManagerFeatureFlag : uint32
	{
		EMemoryManagerFeatureFlag_None                   = 0
		, EMemoryManagerFeatureFlag_TraceLeaks           = DMibBit(0)
		, EMemoryManagerFeatureFlag_StackTrace           = DMibBit(1)
		, EMemoryManagerFeatureFlag_CheckModifyAfterFree = DMibBit(2)
		, EMemoryManagerFeatureFlag_PreGuard             = DMibBit(3)
		, EMemoryManagerFeatureFlag_PostGuard            = DMibBit(4)
		, EMemoryManagerFeatureFlag_FreeValidation       = DMibBit(5)
		, EMemoryManagerFeatureFlag_Enumeration          = DMibBit(6)
		, EMemoryManagerFeatureFlag_AssertOnMemoryLeak   = DMibBit(7)
	};

	struct CGlobalReportMemory
	{
		virtual void f_AllocatorName(umint _MemoryAllocator, ch8 const *_pAllocatorName) = 0;
		virtual void f_AllocatorDelete(umint _MemoryAllocator) = 0;

		virtual void f_ScopeEnter(umint _MemoryAllocator) = 0;
		virtual void f_ScopeExit(umint _MemoryAllocator) = 0;

		virtual void f_Alloc(umint _MemoryAllocator, umint _Address, umint _RequestedAlignment, umint _RequestedSize, umint _ReturnedSize, fp32 _nBytesOverhead, void *_pAllocationInfo) = 0;
		virtual void f_Resize
			(
				umint _MemoryAllocator
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			= 0
		;
		virtual void f_Realloc
			(
				umint _MemoryAllocator
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			= 0
		;
		virtual void f_Free(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_GetSize(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_Protect(umint _MemoryAllocator, umint _Address, umint _Size, uaint _Protect) = 0;
		virtual void f_Commit(umint _MemoryAllocator, umint _Address, umint _Size) = 0;
		virtual void f_Decommit(umint _MemoryAllocator, umint _Address, umint _Size) = 0;

		virtual void f_Report(bool _bFullReport);
	};

	struct CReportMemory
	{
		virtual void f_Alloc
			(
				umint _MemoryAllocator
				, umint _AllocatorDepth
				, ch8 const *_pAllocatorName
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			= 0
		;
		virtual void f_Resize
			(
				umint _MemoryAllocator
				, umint _AllocatorDepth
				, ch8 const *_pAllocatorName
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			= 0
		;
		virtual void f_Realloc
			(
				umint _MemoryAllocator
				, umint _AllocatorDepth
				, ch8 const *_pAllocatorName
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
			= 0
		;
		virtual void f_Free(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_GetSize(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_Protect(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size, uaint _Protect) = 0;
		virtual void f_Commit(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size) = 0;
		virtual void f_Decommit(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size) = 0;
		virtual void f_AllocatorDelete(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth) = 0;
	};

	struct CReportMemoryLightweight
	{
		virtual void f_Alloc(umint _ReturnedSize) = 0;
		virtual void f_Free(umint _Size) = 0;
	};

	struct CAllocator_Base
	{
		static constexpr bool mc_bIsDefault = false;

		constexpr bool operator == (CAllocator_Base const &_Right) const noexcept = default;
	};

	struct CAllocator_AutoDestroy
	{
		void *m_pMemory;
		umint m_Size;

		CAllocator_AutoDestroy(void *_pMemory, umint _Size);
		CAllocator_AutoDestroy(CAllocator_AutoDestroy &&_Other);
		CAllocator_AutoDestroy &operator = (CAllocator_AutoDestroy &&_Other);
		CAllocator_AutoDestroy();

		only_parameters_aliased malloc_like void *f_Get() const;
		void f_Claim();
	};

	template <typename t_CAllocator>
	struct TCAllocator_AutoDestroyStatic : public CAllocator_AutoDestroy
	{
		TCAllocator_AutoDestroyStatic(void *_pMemory, umint _Size);
		TCAllocator_AutoDestroyStatic(TCAllocator_AutoDestroyStatic &&_Other) = default;
		TCAllocator_AutoDestroyStatic &operator = (TCAllocator_AutoDestroyStatic &&_Other) = default;
		TCAllocator_AutoDestroyStatic() = default;
		~TCAllocator_AutoDestroyStatic();
	};

	template <typename t_CAllocator>
	struct TCAllocator_AutoDestroy : public CAllocator_AutoDestroy
	{
		TCAllocator_AutoDestroy(void *_pMemory, umint _Size, t_CAllocator &_Allocator);
		TCAllocator_AutoDestroy(TCAllocator_AutoDestroy &&_Other) = default;
		TCAllocator_AutoDestroy &operator = (TCAllocator_AutoDestroy &&_Other) = default;
		TCAllocator_AutoDestroy(t_CAllocator &_Allocator);
		~TCAllocator_AutoDestroy();

		t_CAllocator &m_Allocator;
	};

	struct CAllocator_Empty : public CAllocator_Base
	{
		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Empty>;

		DMibNamedAsMemberFunction static inline_small umint f_GranularityAlloc(bool _bLargePages = false);
		DMibNamedAsMemberFunction static inline_small umint f_GranularityCommit(bool _bLargePages = false);
		DMibNamedAsMemberFunction static inline_small umint f_GranularityProtect(bool _bLargePages = false);
		DMibNamedAsMemberFunction static inline_small umint f_Size(void *_pBlock);
		DMibNamedAsMemberFunction static inline_small umint f_TrySize(void *_pBlock);
		DMibNamedAsMemberFunction static inline_small umint f_SizePadded(umint _Size);
		DMibNamedAsMemberFunction static inline_small fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		DMibNamedAsMemberFunction static constexpr inline_small bool f_CanCommit();
		DMibNamedAsMemberFunction static inline_small bool f_CanProtect();
		DMibNamedAsMemberFunction static inline_small bool f_DeterministicSize();
		DMibNamedAsMemberFunction static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		DMibNamedAsMemberFunction static inline_small void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		DMibNamedAsMemberFunction static inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		DMibNamedAsMemberFunction static inline_small void *f_AllocAlignedWithSize
			(
				umint &_Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		DMibNamedAsMemberFunction static inline_small void *f_AllocAligned
			(
				umint _Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		DMibNamedAsMemberFunction static inline_small auto f_AllocAlignedWithSizeDebug
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
		DMibNamedAsMemberFunction static inline_small auto f_AllocAlignedDebug
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
		DMibNamedAsMemberFunction static inline_small void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		DMibNamedAsMemberFunction static inline_small void f_AllocBatchDebug
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
		DMibNamedAsMemberFunction static inline_small auto f_AllocWithSizeDebug
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
		DMibNamedAsMemberFunction static inline_small auto f_AllocDebug
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
		DMibNamedAsMemberFunction static inline_small void *f_Realloc
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		static inline_small void *fs_ReallocLocation(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		DMibNamedAsMemberFunction static inline_small auto f_ReallocDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		DMibNamedAsMemberFunction static inline_small void *f_Resize
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		DMibNamedAsMemberFunction static inline_small void f_Commit(void *_pMem, umint _Size);
		DMibNamedAsMemberFunction static inline_small void f_Decommit(void *_pMem, umint _Size);
		DMibNamedAsMemberFunction static inline_small void f_Free(void *_pBlock, umint _Size);
		DMibNamedAsMemberFunction static inline_small void f_FreeNoSize(void *_pBlock);
		DMibNamedAsMemberFunction only_parameters_aliased static auto f_AllocSafeWithSize
			(
				umint &_Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> CAutoDestroy
		;
		DMibNamedAsMemberFunction only_parameters_aliased static CAutoDestroy f_AllocSafe
			(
				umint _Size
				, umint _Alignment
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		DMibNamedAsMemberFunction static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
	};

	struct CAllocator_Disable : public CAllocator_Base
	{
	};

	struct CAllocator_Heap;

	/************************************************************************************************\
	||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
	|| Common functions
	||______________________________________________________________________________________________||
	\************************************************************************************************/

#ifdef	DMibPIntrinsicMemCopy
	template <typename tf_CData1, typename tf_CData2>
	inline_small tf_CData1 *fg_MemCopy(tf_CData1 *_pDest, tf_CData2 const *_pSource, umint _Size) noexcept;
#else
	template <typename tf_CData1, typename tf_CData2>
	inline_medium tf_CData1 *fg_MemCopy(tf_CData1 *_pDest, tf_CData2 const *_pSource, umint _Size) noexcept;
#endif

	template <typename tf_CData1, typename tf_CData2>
	inline_medium tf_CData1 *fg_ObjectCopy(tf_CData1 *_pDest, tf_CData2 const *_pSource, umint _nObjects) noexcept(noexcept(fg_GetType<tf_CData1 &>() = fg_GetType<tf_CData2 const &>()));
	template <typename tf_CData1, umint tf_ListSize>
	inline_medium auto fg_ObjectCopy(tf_CData1 _Dest[tf_ListSize], tf_CData1 const _Source[tf_ListSize]) noexcept(noexcept(fg_GetType<tf_CData1 &>() = fg_GetType<tf_CData1 const &>()))
		-> tf_CData1 *
	;

	template <typename tf_CData1, typename tf_CData2>
	inline_medium tf_CData1 *fg_ObjectSet(tf_CData1 *_pDest, tf_CData2 const _SetValue, umint _NumElements) noexcept(noexcept(fg_GetType<tf_CData1 &>() = fg_GetType<tf_CData2 const &>()));

#ifdef	DMibPIntrinsicMemSet
	template <typename tf_CData2>
	inline_always uint8 *fg_ObjectSet(uint8 *_pDest, tf_CData2 _SetValue, umint _NumElements) noexcept;
#endif

#ifdef	DMibPIntrinsicMemSet
	template <typename tf_CData1>
	inline_always tf_CData1 *fg_MemClear(tf_CData1 *_pFirst, umint _Size) noexcept;
#else
	template <typename tf_CData1>
	inline_large tf_CData1 *fg_MemClear(tf_CData1 *_pFirst, umint _Size) noexcept;
#endif

	template <typename tf_CData1>
	inline_always tf_CData1 &fg_MemClear(tf_CData1 &_First) noexcept;
	template <typename tf_CData1, umint tf_nElements>
	inline_always tf_CData1 *fg_MemClear(tf_CData1 _Data[tf_nElements]) noexcept;

#ifdef	DMibPIntrinsicMemSet
#	ifdef DCompiler_MSVC
	template <typename tf_CData1>
	inline_always tf_CData1 *fg_SecureMemClear(tf_CData1 * volatile _pFirst, umint _Size) noexcept;
#	else
	template <typename tf_CData1>
	inline_always tf_CData1 *fg_SecureMemClear(tf_CData1 *_pFirst, umint _Size) noexcept;
#	endif
#else
	template <typename tf_CData1>
	inline_large tf_CData1 *fg_SecureMemClear(tf_CData1 * volatile _pFirst, umint _Size) noexcept;
#endif

	template <typename tf_CData1>
	inline_always tf_CData1 &fg_SecureMemClear(tf_CData1 &_First) noexcept;
	template <typename tf_CData1, umint tf_nElements>
	inline_always tf_CData1 *fg_SecureMemClear(tf_CData1 _Data[tf_nElements]) noexcept;

#ifdef DMibPIntrinsicMemCmp
	static inline_always aint fg_MemCmp(uint8 const *_pFirst, uint8 const *_pSecond, umint _Size) noexcept;
#else
	static inline_medium aint fg_MemCmp(uint8 const *_pFirst, uint8 const *_pSecond, umint _Size) noexcept;
#endif

	static inline_medium aint fg_MemCmpOne(uint8 const *_pFirst, uint8 const _Second, umint _Size) noexcept;

#ifdef	DMibPIntrinsicMemMove
	template <typename tf_CData1, typename tf_CData2>
	inline_small tf_CData1 *fg_MemMove(tf_CData1 *_pDest, tf_CData2 const *_pSource, umint _Size) noexcept;
#else
	template <typename tf_CData1, typename tf_CData2>
	inline_extralarge tf_CData1 *fg_MemMove(tf_CData1 *_pDest, tf_CData2 const *_pSource, umint _Size) noexcept;
#endif

	namespace NPrivate
	{
		template <typename t_CData, typename t_FSorter>
		struct TCHelpQSort
		{
			static void fs_QuickSort(t_CData *_pArray, aint _Low, aint _High, t_FSorter const &_fCompare);

		private:
			static void fsp_InsertSort(t_CData *_pArray, aint _Low, aint _High, t_FSorter const &_fCompare);
			static aint fsp_Partition(t_CData *_pArray, aint _Low, aint _High, t_FSorter const &_fCompare);
		};
	}

	template <typename tf_CType, typename tf_FCompare>
	void fg_QSort(tf_CType *_pToSort, umint _nNumElements, tf_FCompare const &_fCompare);
}

#ifndef DMibPNoShortCuts
using namespace NMib::NMemory;
#endif

#include "Malterlib_Memory.hpp"
