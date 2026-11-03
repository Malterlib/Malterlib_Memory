// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifdef DMibConfig_OverrideSystemMalloc

#include <Mib/Core/Core>
#include "Malterlib_Memory_MemoryManager_Utils.h"
#include <windows.h>
#include <errno.h>

#ifdef DArchitecture_x86
#	define DDefaultCallingConv __cdecl
#else
#	define DDefaultCallingConv
#endif

#define DMibAllowCodeStandardViolations 1

#undef new
#undef malloc
#undef free
#undef realloc
#undef calloc
#undef memalign
#undef _msize
#undef _expand

#undef _malloc_dbg
#undef _free_dbg
#undef _realloc_dbg
#undef _calloc_dbg
#undef _calloc_dbg_impl
#undef memalign_dbg
#undef _msize_dbg
#undef _expand_dbg
#undef _nh_malloc
#undef _heap_alloc
#undef _recalloc_dbg
#undef _malloc_base
#undef _malloc_dbg
#undef _nh_malloc_dbg
#undef _heap_alloc_dbg

#undef _free_base
#undef _calloc_base
#undef _realloc_base
#undef _msize_base
#undef _expand_base
#undef _nh_malloc_base
#undef _malloc_crt
#undef _calloc_crt
#undef _realloc_crt
#undef _recalloc_crt
#undef _recalloc_base

#undef _aligned_free
#undef _aligned_malloc
#undef _aligned_offset_malloc
#undef _aligned_realloc
#undef _aligned_recalloc
#undef _aligned_offset_realloc
#undef _aligned_offset_recalloc
#undef _aligned_msize

#undef _aligned_free_dbg
#undef _aligned_malloc_dbg
#undef _aligned_offset_malloc_dbg
#undef _aligned_realloc_dbg
#undef _aligned_recalloc_dbg
#undef _aligned_offset_realloc_dbg
#undef _aligned_offset_recalloc_dbg
#undef _aligned_msize_dbg

#undef _CrtSetBreakAlloc
#undef _CrtSetDbgBlockType
#undef _CrtSetAllocHook
#undef _CrtCheckMemory
#undef _CrtSetDbgFlag
#undef _CrtDoForAllClientObjects
#undef _CrtIsValidPointer
#undef _CrtIsValidHeapPointer
#undef _CrtIsMemoryBlock
#undef _CrtSetDumpClient
#undef _CrtMemCheckpoint
#undef _CrtMemDifference
#undef _CrtMemDumpAllObjectsSince
#undef _CrtDumpMemoryLeaks
#undef _CrtMemDumpStatistics
#undef _CrtReportBlockType
#undef _CrtSetCheckCount
#undef _CrtGetCheckCount

#define DMibSystemAlignment sizeof(void *)*2

using namespace NMib;
using namespace NMib::NMemory;

namespace
{
	inline_always bool fg_TryAlignWindowsAllocationSize(umint &o_Size, umint _Alignment = DMibSystemAlignment)
	{
		if (!NMib::NMemory::NPrivate::fg_TryAlignAllocationSize(o_Size, _Alignment))
		{
			errno = ENOMEM;
			return false;
		}

		return true;
	}

	inline_always bool fg_TryMultiplyWindowsAllocationSize(size_t _Count, size_t _Size, umint &o_Size, int *o_pErrno = nullptr)
	{
		if (NMib::fg_MultiplyOverflow(umint(_Count), umint(_Size), o_Size))
		{
			errno = ENOMEM;
			if (o_pErrno)
				*o_pErrno = ENOMEM;
			return false;
		}

		return true;
	}

	[[maybe_unused]] inline_always bool fg_TryAddWindowsAllocationSize(size_t _Size, size_t _Extra, size_t &o_Size)
	{
		if (_Size > NMib::TCLimitsInt<size_t>::mc_Max - _Extra)
		{
			errno = ENOMEM;
			return false;
		}

		o_Size = _Size + _Extra;
		return true;
	}
}

extern "C"
{
#if _MSC_VER >= 1400
#	define DMemDeclNaR __declspec(noalias) __declspec(restrict)
#	define DMemDeclNa __declspec(noalias)
#else
#	define DMemDeclNaR
#	define DMemDeclNa
#endif
	size_t __crtDebugFillThreshold = SIZE_MAX;

	DMemDeclNaR void * DDefaultCallingConv malloc(size_t);
	DMemDeclNa void DDefaultCallingConv free(void *);
	DMemDeclNaR void * DDefaultCallingConv realloc(void *, size_t);
	DMemDeclNaR void * DDefaultCallingConv calloc(size_t, size_t);
	DMemDeclNaR void * DDefaultCallingConv memalign(size_t, size_t);

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| malloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv malloc(size_t sz)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(sz))
			return nullptr;
		return NMib::NMemory::fg_Alloc(sz);
	}

	void * DDefaultCallingConv _malloc_base(size_t sz)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(sz))
			return nullptr;
		return NMib::NMemory::fg_Alloc(sz);
	}

	void * DDefaultCallingConv _malloc_dbg(size_t sz, int BlockType, char const *Filename, int Line)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(sz))
			return nullptr;
		return NMib::NMemory::fg_AllocDebug(sz, Filename, Line, (BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| calloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv calloc(size_t nelem, size_t elsize)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(nelem, elsize, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * DDefaultCallingConv _calloc_dbg(size_t nelem, size_t elsize, int BlockType, char const *Filename, int Line)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(nelem, elsize, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_AllocDebug(Size, Filename, Line, (BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * __cdecl _calloc_dbg_impl(size_t nNum, size_t nSize, int nBlockUse, char const *szFileName, int nLine, int *errno_tmp)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(nNum, nSize, UnalignedSize, errno_tmp))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
		{
			if (errno_tmp)
				*errno_tmp = ENOMEM;
			return nullptr;
		}
		void *pMemory = NMib::NMemory::fg_AllocDebug(Size, szFileName, nLine, (nBlockUse == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * DDefaultCallingConv _calloc_impl(size_t nelem, size_t elsize, int *errno_tmp)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(nelem, elsize, UnalignedSize, errno_tmp))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
		{
			if (errno_tmp)
				*errno_tmp = ENOMEM;
			return nullptr;
		}
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * DDefaultCallingConv _calloc_base(size_t nelem, size_t elsize)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(nelem, elsize, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	_CRTIMP __checkReturn __bcount_opt(_Size) void * __cdecl _malloc_crt(__in size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Alloc(_Size);
		return pMemory;
	}

	_CRTIMP void * __cdecl _calloc_crt(size_t count, size_t size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(count, size, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	_CRTIMP __checkReturn __bcount_opt(_Size) void * __cdecl _realloc_crt(__inout_opt void *_Ptr, __in size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Resize(_Ptr, _Size, 0, EAllocationFlag_SizeNotNeeded);
		return pMemory;
	}

	_CRTIMP __checkReturn __bcount_opt(_Size *_Count) void * __cdecl _recalloc_crt(__inout_opt void *_Ptr, __in size_t _Count, __in size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint Size;
		if (!fg_TryMultiplyWindowsAllocationSize(_Count, _Size, Size))
			return nullptr;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Resize(_Ptr, Size, 0, EAllocationFlag_SizeNotNeeded);
		return pMemory;
	}

	_Success_(return != 0) _Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(_Count * _Size)
	_ACRTIMP _CRTALLOCATOR _CRTRESTRICT
	void * __cdecl _recalloc_base(_Pre_maybenull_ _Post_invalid_ void *_Ptr, _In_ size_t _Count, _In_ size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint Size;
		if (!fg_TryMultiplyWindowsAllocationSize(_Count, _Size, Size))
			return nullptr;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Resize(_Ptr, Size, 0, EAllocationFlag_SizeNotNeeded);
		return pMemory;
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| free
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNa void DDefaultCallingConv free(void *ptr)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		NMib::NMemory::fg_FreeNoSize(ptr);
	}

	void DDefaultCallingConv _free_base(void *ptr)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		NMib::NMemory::fg_FreeNoSize(ptr);
	}

	void DDefaultCallingConv _free_dbg(void *ptr, int)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		NMib::NMemory::fg_FreeNoSize(ptr);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| memalign
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv memalign(size_t alignment, size_t size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!NMib::NMemory::NPrivate::fg_IsAllocationAlignmentValid(alignment))
		{
			errno = EINVAL;
			return nullptr;
		}
		if (!fg_TryAlignWindowsAllocationSize(size, alignment))
			return nullptr;
		return NMib::NMemory::fg_AllocAligned(size, alignment);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| realloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv realloc(void *ptr, size_t sz)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(sz))
			return nullptr;
		return NMib::NMemory::fg_Resize(ptr, sz, 0, EAllocationFlag_SizeNotNeeded);
	}

	void * DDefaultCallingConv _realloc_base(void *ptr, size_t sz)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(sz))
			return nullptr;
		return NMib::NMemory::fg_Resize(ptr, sz, 0, EAllocationFlag_SizeNotNeeded);
	}

	void * DDefaultCallingConv _realloc_dbg(void *ptr, size_t sz, int BlockType, char const *Filename, int Line)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(sz))
			return nullptr;
		return NMib::NMemory::fg_ResizeDebug(ptr, sz, 0, Filename, Line, (BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
	}

	DMemDeclNaR void * DDefaultCallingConv _recalloc(void *memblock, size_t count, size_t size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint Size;
		if (!fg_TryMultiplyWindowsAllocationSize(count, size, Size))
			return nullptr;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		return NMib::NMemory::fg_Resize(memblock, Size, 0, EAllocationFlag_SizeNotNeeded);
	}

//#ifndef DArchX86_64
// DMemDeclNaR
//#endif
	void * DDefaultCallingConv _recalloc_dbg(void *memblock, size_t count, size_t size, int nBlockUse, char const *szFileName, int nLine)
	{
		size_t TotalSize = 0;

		if (!fg_TryMultiplyWindowsAllocationSize(count, size, TotalSize))
			return nullptr;

		return _realloc_dbg(memblock, TotalSize, nBlockUse, szFileName, nLine);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _msize
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	umint DDefaultCallingConv _msize(void *mem)
	{
		return NMib::NMemory::fg_Size(mem);
	}

	umint DDefaultCallingConv _msize_base(void *mem) _CRT_NOEXCEPT
	{
		return NMib::NMemory::fg_Size(mem);
	}

	umint DDefaultCallingConv _msize_dbg(void *mem, int blockType)
	{
		return NMib::NMemory::fg_Size(mem);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _expand
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	void * DDefaultCallingConv _expand(void *mem, size_t size)
	{
		// Just fail :P
		DMibPDebugBreak;
		return mem;
	}

	void * DDefaultCallingConv _expand_base(void *mem, size_t size)
	{
		// Just fail :P
		DMibPDebugBreak;
		return mem;
	}

	void * DDefaultCallingConv _expand_dbg(void *userData, umint newSize, int blockType, char const *filename, int linenumber)
	{
		// Just fail :P
		DMibPDebugBreak;
		return userData;
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _nh_malloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	_Check_return_ _Ret_opt_bytecap_(_Size) void * __cdecl _nh_malloc(_In_ size_t _Size, _In_ int _NhFlag)
	{
		return malloc(_Size);
	}

	void * DDefaultCallingConv _nh_malloc_base(umint nSize, int nhFlag)
	{
		return malloc(nSize);
	}

	void * DDefaultCallingConv _nh_malloc_dbg(umint nSize, int nhFlag, int nBlockUse, char const *szFileName, int nLine)
	{
		return _malloc_dbg(nSize, nBlockUse, szFileName, nLine);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _heap_alloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	_Check_return_ _Ret_opt_bytecap_(_Size) void * __cdecl _heap_alloc(_In_ size_t _Size)
	{
		return malloc(_Size);
	}

	void * DDefaultCallingConv _heap_alloc_base(umint nSize)
	{
		return malloc(nSize);
	}

	void * DDefaultCallingConv _heap_alloc_dbg(umint nSize, int nBlockUse, char const *szFileName, int nLine)
	{
		return _malloc_dbg(nSize, nBlockUse, szFileName, nLine);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _free_lk
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	void DDefaultCallingConv _free_lk(void *pUserData)
	{
		free(pUserData);
	}

	void DDefaultCallingConv _free_lk_base(void *pUserData)
	{
		free(pUserData);
	}

	void DDefaultCallingConv _free_dbg_lk(void *pUserData, int nBlockUse)
	{
		free(pUserData);
	}

	__declspec(noalias) void DDefaultCallingConv _freea_s(void *ptr)
	{
		// DMibPDebugBreak;
		if (ptr != nullptr)
		{
			ptr = (char *)ptr - _ALLOCA_S_MARKER_SIZE;
			if (*((size_t *)ptr) == _ALLOCA_S_HEAP_MARKER)
				free(ptr);
		}
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| Debug routines
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	long DDefaultCallingConv _CrtSetBreakAlloc(long lNewBreakAlloc)
	{
		return 0;
	}

	void DDefaultCallingConv _CrtSetDbgBlockType(void *pUserData, int nBlockUse)
	{
	}

	typedef int (DDefaultCallingConv *_CRT_ALLOC_HOOK)(int _AllocationType, void *_pData, umint _Size, int _BlockUse, long _Request, unsigned char const *_pFileName, int _LineNumber);

	_CRT_ALLOC_HOOK DDefaultCallingConv _CrtSetAllocHook(_CRT_ALLOC_HOOK pfnNewHook)
	{
		return nullptr;
	}

	int DDefaultCallingConv CheckBytes(unsigned char *pb, unsigned char bCheck, umint nSize)
	{
		// We actually need to do this (not just return 1) since both positive and negative returns are used :-)
		unsigned char *pEnd = pb + nSize;
		while (pb < pEnd)
		{
			if (*pb++ != bCheck)
				return 0;
		}
		return 1;
	}

	int DDefaultCallingConv _CrtCheckMemory(void)
	{
		return NMib::fg_GetSys()->f_MemoryManager_Check(false);
	}

	int DDefaultCallingConv _CrtSetDbgFlag(int fNewBits)
	{
		return _CRTDBG_LEAK_CHECK_DF;
	}

	void DDefaultCallingConv _CrtDoForAllClientObjects(void (DDefaultCallingConv *pfn)(void *_pData, void *_pContext), void *pContext)
	{
	}

	int DDefaultCallingConv _CrtIsValidPointer(void const *pv, unsigned int nBytes, int bReadWrite)
	{
		return
			(
				pv != nullptr
#ifdef _WIN32
				&& !IsBadReadPtr(pv, nBytes)
				&& (!bReadWrite || !IsBadWritePtr((LPVOID)pv, nBytes))
#endif  /* _WIN32 */
			)
		;
	}

	int DDefaultCallingConv _CrtIsValidHeapPointer(void const *pUserData)
	{
		return true;
	}

	int DDefaultCallingConv _CrtIsMemoryBlock(void const *pUserData, unsigned int nBytes, long *plRequestNumber, char **pszFileName, int *pnLine)
	{
		return false;
	}

	typedef void (DDefaultCallingConv *_CRT_DUMP_CLIENT)(void *_pUserPortion, umint _BlockSize);

	_CRT_DUMP_CLIENT DDefaultCallingConv _CrtSetDumpClient(_CRT_DUMP_CLIENT pfnNewDump)
	{
		return nullptr;
	}

	void DDefaultCallingConv _CrtMemCheckpoint(_CrtMemState *state)
	{
	}

	int DDefaultCallingConv _CrtMemDifference(_CrtMemState *state, _CrtMemState const *oldState, _CrtMemState const *newState)
	{
		return false;
	}

	void DDefaultCallingConv _CrtMemDumpAllObjectsSince(_CrtMemState const *state)
	{
	}

	int DDefaultCallingConv _CrtDumpMemoryLeaks(void)
	{
		return false;
	}

	void DDefaultCallingConv _CrtMemDumpStatistics(_CrtMemState const *state)
	{
	}

	int DDefaultCallingConv _CrtReportBlockType(void const *pUserData)
	{
		return 0;
	}

	int DDefaultCallingConv _CrtSetCheckCount(int fCheckCount)
	{
		return 0;
	}

	_CRTIMP int __cdecl _CrtGetCheckCount(void)
	{
		return 0;
	}

	int DDefaultCallingConv _heap_init()
	{
		//fg_CTRHackHeapInit();
		return true;
	}

	extern "C" HANDLE __acrt_heap = nullptr;

	void DDefaultCallingConv _heap_term(void)
	{
	}

	int DDefaultCallingConv __heap_select(void)
	{
		DMibPDebugBreak;
		return 0;
	}

	extern "C" bool __cdecl __acrt_initialize_heap()
	{
		return true;
	}

	extern "C" bool __cdecl __acrt_uninitialize_heap(bool const)
	{
		return true;
	}

	extern "C" intptr_t __cdecl _get_heap_handle()
	{
		DMibPDebugBreak;
		return 0;
	}

	extern "C" HANDLE __acrt_getheap()
	{
		DMibPDebugBreak;
		return nullptr;
	}

	void DDefaultCallingConv _heap_abort(void)
	{
	}
#ifdef DMibDebug
#	define DAlignGapSize sizeof(void *)

	typedef struct CAlignMemBlockHdr
	{
		void *m_pHead;
		unsigned char m_Gap[DAlignGapSize];
	} CAlignMemBlockHdr;

#	define DIs2PowN(d_Value)   (((d_Value)&(d_Value-1)) == 0)

#	define DNoMansLandSize 4

	static unsigned char gs_NoMansLandFill = 0xFD; // fill no-man's land with this
	static unsigned char gs_AlignLandFill = 0xED; // fill no-man's land for aligned routines

	void * __cdecl _aligned_malloc(size_t size, size_t align)
	{
		return _aligned_offset_malloc_dbg(size, align, 0, NULL, 0);
	}

	void * __cdecl _aligned_malloc_dbg(size_t size, size_t align, char const *f_name, int line_n)
	{
		return _aligned_offset_malloc_dbg(size, align, 0, f_name, line_n);
	}

	void * __cdecl _aligned_realloc(void *memblock, size_t size, size_t align)
	{
		return _aligned_offset_realloc_dbg(memblock, size, align, 0, NULL, 0);
	}

	void * __cdecl _aligned_realloc_dbg(void *memblock, size_t size, size_t align, char const *f_name, int line_n)
	{
		return _aligned_offset_realloc_dbg(memblock, size, align, 0, f_name, line_n);
	}

	void * __cdecl _aligned_recalloc_dbg(void *memblock, size_t count, size_t size, size_t align, char const *f_name, int line_n)
	{
		return _aligned_offset_recalloc_dbg(memblock, count, size, align, 0, f_name, line_n);
	}

	void * __cdecl _aligned_offset_malloc(size_t size, size_t align, size_t offset)
	{
		return _aligned_offset_malloc_dbg(size, align, offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_malloc_dbg(size_t size, size_t align, size_t offset, char const *f_name, int line_n)
	{
		uintptr_t BlockAddress, AlignedAddress, HeaderGap;
		CAlignMemBlockHdr *pHdr;
		size_t NonUserSize, BlockSize;

		/* validation section */
		align = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) - 1;

		HeaderGap = (0 - offset) & (sizeof(uintptr_t) - 1);

		NonUserSize = HeaderGap + align + sizeof(CAlignMemBlockHdr); // cannot overflow
		if (!fg_TryAddWindowsAllocationSize(size, NonUserSize, BlockSize))
			return NULL;

		if ((BlockAddress = (uintptr_t)_malloc_dbg(BlockSize, _NORMAL_BLOCK, f_name, line_n)) == (uintptr_t)NULL)
			return NULL;

		AlignedAddress = ((BlockAddress + NonUserSize + offset) & ~align) - offset;
		pHdr = (CAlignMemBlockHdr *)(AlignedAddress - HeaderGap) - 1;
		memset((void *)pHdr->m_Gap, gs_AlignLandFill, DAlignGapSize);
		pHdr->m_pHead = (void *)BlockAddress;
		return (void *)AlignedAddress;
	}

	void * __cdecl _aligned_offset_realloc(void *memblock, size_t size, size_t align, size_t offset)
	{
		return _aligned_offset_realloc_dbg(memblock, size, align, offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_recalloc(void *memblock, size_t count, size_t size, size_t align, size_t offset)
	{
		return _aligned_offset_recalloc_dbg(memblock, count, size, align, offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_realloc_dbg(void *memblock, size_t size, size_t align, size_t offset, char const *f_name, int line_n)
	{
		uintptr_t BlockAddress, AlignedAddress, HeaderGap, MoveSize;
		CAlignMemBlockHdr *pHdr, *pOldHeader;
		size_t NonUserSize, BlockSize;

		if (memblock == NULL)
			return _aligned_offset_malloc_dbg(size, align, offset, f_name, line_n);
		if (size == 0)
		{
			_aligned_free_dbg(memblock);
			return NULL;
		}

		pOldHeader = (CAlignMemBlockHdr *)((uintptr_t)memblock & ~(sizeof(uintptr_t) - 1)) - 1;

		if (CheckBytes((unsigned char *)memblock - DNoMansLandSize, gs_NoMansLandFill, DNoMansLandSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "The block at 0x%p was not allocated by _aligned routines, use realloc()", memblock);
			errno = 22;
			return NULL;
		}

		if (!CheckBytes(pOldHeader->m_Gap, gs_AlignLandFill, DAlignGapSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", memblock);
		}

		/* validation section */

		MoveSize = _msize(pOldHeader->m_pHead) - ((uintptr_t)memblock - (uintptr_t)pOldHeader->m_pHead);

		align = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) - 1;

		HeaderGap = (0 - offset) & (sizeof(uintptr_t) - 1);

		NonUserSize = HeaderGap + align + sizeof(CAlignMemBlockHdr); // cannot overflow
		if (!fg_TryAddWindowsAllocationSize(size, NonUserSize, BlockSize))
			return NULL;

		if ((BlockAddress = (uintptr_t)_malloc_dbg(BlockSize, _NORMAL_BLOCK, f_name, line_n)) == (uintptr_t)NULL)
			return NULL;

		AlignedAddress = ((BlockAddress + NonUserSize + offset) & ~align) - offset;
		pHdr = (CAlignMemBlockHdr *)(AlignedAddress - HeaderGap) - 1;
		memset((void *)pHdr->m_Gap, gs_AlignLandFill, DAlignGapSize);
		pHdr->m_pHead = (void *)BlockAddress;

		memcpy((void *)AlignedAddress, memblock, MoveSize > size ? size : MoveSize);
		_free_dbg(pOldHeader->m_pHead, _NORMAL_BLOCK);

		return (void *)AlignedAddress;
	}

	void * __cdecl _aligned_offset_recalloc_dbg(void *memblock, size_t count, size_t size, size_t align, size_t offset, char const *f_name, int line_n)
	{
		size_t UserSize = 0; // wanted size, passed to aligned realoc
		size_t FillStart = 0; // location where aligned recalloc starts to fill with 0
							  /* filling must start from the end of the previous user block */
							  /* and must stop at the end of the new user block */
		void *pResult = NULL; // result of aligned recalloc

		/* ensure that (size * count) does not overflow */
		if (!fg_TryMultiplyWindowsAllocationSize(count, size, UserSize))
			return NULL;

		if (memblock != NULL)
			FillStart = _aligned_msize(memblock, align, offset);

		pResult = _aligned_offset_realloc_dbg(memblock, UserSize, align, offset, f_name, line_n);

		if (pResult != NULL)
		{
			if (FillStart < UserSize)
				memset((char *)pResult + FillStart, 0, UserSize - FillStart);
		}
		return pResult;
	}

	void __cdecl _aligned_free(void *memblock)
	{
		_aligned_free_dbg(memblock);
	}

	void __cdecl _aligned_free_dbg(void *memblock)
	{
		CAlignMemBlockHdr *pHdr;

		if (memblock == NULL)
			return;

		pHdr = (CAlignMemBlockHdr *)((uintptr_t)memblock & ~(sizeof(uintptr_t) - 1)) - 1;

		if (CheckBytes((unsigned char *)memblock - DNoMansLandSize, gs_NoMansLandFill, DNoMansLandSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "The block at 0x%p was not allocated by _aligned routines, use free()", memblock);
			return;
		}

		if (!CheckBytes(pHdr->m_Gap, gs_AlignLandFill, DAlignGapSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", memblock);
		}
		_free_dbg((void *)pHdr->m_pHead, _NORMAL_BLOCK);
	}

	size_t __cdecl _aligned_msize(void *memblock, size_t align, size_t offset)
	{
		return _aligned_msize_dbg(memblock, align, offset);
	}

	size_t __cdecl _aligned_msize_dbg(void *memblock, size_t align, size_t offset)
	{
		size_t HeaderSize = 0; // Size of the header block
		size_t FooterSize = 0; // Size of the footer block
		size_t TotalSize = 0; // total size of the allocated block
		size_t UserSize = 0; // size of the user block
		uintptr_t Gap = 0; // keep the alignment of the data block
						   /* after the sizeof(void*) aligned pointer */
						   /* to the beginning of the allocated block */

		/* HEADER SIZE + FOOTER SIZE = GAP + ALIGN + SIZE OF A POINTER*/
		/* HEADER SIZE + USER SIZE + FOOTER SIZE = TOTAL SIZE */

		CAlignMemBlockHdr *pHdr = NULL; // points to the beginning of the allocated block
		pHdr = (CAlignMemBlockHdr *)((uintptr_t)memblock & ~(sizeof(uintptr_t) - 1)) - 1;
		TotalSize = _msize(pHdr->m_pHead);
		HeaderSize = (uintptr_t)memblock - (uintptr_t)pHdr->m_pHead;
		Gap = (0 - offset) & (sizeof(uintptr_t) - 1);
		/* The align cannot be smaller than the sizeof(uintptr_t) */
		align = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) - 1;
		FooterSize = Gap + align + sizeof(CAlignMemBlockHdr) - HeaderSize;
		UserSize = TotalSize - HeaderSize - FooterSize;
		return UserSize;
	}
#endif
}

void fg_MalterlibMallocOverride_CanStartThreads()
{
}

void fg_MalterlibMallocOverride_DestroyThreads()
{
}

void fg_MalterlibMallocOverride_PreDestroyNonTrackedMemoryManager()
{
}

void NMib::NSys::fg_Mem_DisableLazyReturnCheckout()
{
}

void NMib::NSys::fg_Mem_EnableLazyReturnCheckout()
{
}

void NMib::NSys::fg_Mem_PrepareFork()
{
}

void NMib::NSys::fg_Mem_ForkedChild()
{
}

void NMib::NSys::fg_Mem_ForkedParent()
{
}
#endif
