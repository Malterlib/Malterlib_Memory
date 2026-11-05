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

	inline_always bool fg_TryMultiplyWindowsAllocationSize(size_t _nMembers, size_t _Size, umint &o_Size, int *o_pErrno = nullptr)
	{
		if (NMib::fg_MultiplyOverflow(umint(_nMembers), umint(_Size), o_Size))
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

	DMemDeclNaR void * DDefaultCallingConv malloc(size_t _Size);
	DMemDeclNa void DDefaultCallingConv free(void *_pMemory);
	DMemDeclNaR void * DDefaultCallingConv realloc(void *_pMemory, size_t _Size);
	DMemDeclNaR void * DDefaultCallingConv calloc(size_t _nMembers, size_t _Size);
	DMemDeclNaR void * DDefaultCallingConv memalign(size_t _Alignment, size_t _Size);

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| malloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv malloc(size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		return NMib::NMemory::fg_Alloc(_Size);
	}

	void * DDefaultCallingConv _malloc_base(size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		return NMib::NMemory::fg_Alloc(_Size);
	}

	void * DDefaultCallingConv _malloc_dbg(size_t _Size, int _BlockType, char const *_pFile, int _Line)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		return NMib::NMemory::fg_AllocDebug(_Size, _pFile, _Line, (_BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| calloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv calloc(size_t _nMembers, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * DDefaultCallingConv _calloc_dbg(size_t _nMembers, size_t _Size, int _BlockType, char const *_pFile, int _Line)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_AllocDebug(Size, _pFile, _Line, (_BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * __cdecl _calloc_dbg_impl(size_t _nMembers, size_t _Size, int _BlockType, char const *_pFile, int _Line, int *o_pErrno)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UnalignedSize, o_pErrno))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
		{
			if (o_pErrno)
				*o_pErrno = ENOMEM;
			return nullptr;
		}
		void *pMemory = NMib::NMemory::fg_AllocDebug(Size, _pFile, _Line, (_BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * DDefaultCallingConv _calloc_impl(size_t _nMembers, size_t _Size, int *o_pErrno)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UnalignedSize, o_pErrno))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
		{
			if (o_pErrno)
				*o_pErrno = ENOMEM;
			return nullptr;
		}
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	void * DDefaultCallingConv _calloc_base(size_t _nMembers, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UnalignedSize))
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

	_CRTIMP void * __cdecl _calloc_crt(size_t _nMembers, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint UnalignedSize;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UnalignedSize))
			return nullptr;
		umint Size = UnalignedSize;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Alloc(Size);
		memset(pMemory, 0, UnalignedSize);
		return pMemory;
	}

	_CRTIMP __checkReturn __bcount_opt(_Size) void * __cdecl _realloc_crt(__inout_opt void *_pMemory, __in size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Resize(_pMemory, _Size, 0, EAllocationFlag_SizeNotNeeded);
		return pMemory;
	}

	_CRTIMP __checkReturn __bcount_opt(_Size *_nMembers) void * __cdecl _recalloc_crt(__inout_opt void *_pMemory, __in size_t _nMembers, __in size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint Size;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, Size))
			return nullptr;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Resize(_pMemory, Size, 0, EAllocationFlag_SizeNotNeeded);
		return pMemory;
	}

	_Success_(return != 0) _Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(_nMembers * _Size)
	_ACRTIMP _CRTALLOCATOR _CRTRESTRICT
	void * __cdecl _recalloc_base(_Pre_maybenull_ _Post_invalid_ void *_pMemory, _In_ size_t _nMembers, _In_ size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint Size;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, Size))
			return nullptr;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		void *pMemory = NMib::NMemory::fg_Resize(_pMemory, Size, 0, EAllocationFlag_SizeNotNeeded);
		return pMemory;
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| free
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNa void DDefaultCallingConv free(void *_pMemory)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	void DDefaultCallingConv _free_base(void *_pMemory)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	void DDefaultCallingConv _free_dbg(void *_pMemory, int _BlockType)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| memalign
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv memalign(size_t _Alignment, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!NMib::NMemory::NPrivate::fg_IsAllocationAlignmentValid(_Alignment))
		{
			errno = EINVAL;
			return nullptr;
		}
		if (!fg_TryAlignWindowsAllocationSize(_Size, _Alignment))
			return nullptr;
		return NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| realloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	DMemDeclNaR void * DDefaultCallingConv realloc(void *_pMemory, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		return NMib::NMemory::fg_Resize(_pMemory, _Size, 0, EAllocationFlag_SizeNotNeeded);
	}

	void * DDefaultCallingConv _realloc_base(void *_pMemory, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		return NMib::NMemory::fg_Resize(_pMemory, _Size, 0, EAllocationFlag_SizeNotNeeded);
	}

	void * DDefaultCallingConv _realloc_dbg(void *_pMemory, size_t _Size, int _BlockType, char const *_pFile, int _Line)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		if (!fg_TryAlignWindowsAllocationSize(_Size))
			return nullptr;
		return NMib::NMemory::fg_ResizeDebug(_pMemory, _Size, 0, _pFile, _Line, (_BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
	}

	DMemDeclNaR void * DDefaultCallingConv _recalloc(void *_pMemory, size_t _nMembers, size_t _Size)
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		umint Size;
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, Size))
			return nullptr;
		if (!fg_TryAlignWindowsAllocationSize(Size))
			return nullptr;
		return NMib::NMemory::fg_Resize(_pMemory, Size, 0, EAllocationFlag_SizeNotNeeded);
	}

//#ifndef DArchX86_64
// DMemDeclNaR
//#endif
	void * DDefaultCallingConv _recalloc_dbg(void *_pMemory, size_t _nMembers, size_t _Size, int _BlockType, char const *_pFile, int _Line)
	{
		size_t TotalSize = 0;

		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, TotalSize))
			return nullptr;

		return _realloc_dbg(_pMemory, TotalSize, _BlockType, _pFile, _Line);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _msize
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	umint DDefaultCallingConv _msize(void *_pMemory)
	{
		return NMib::NMemory::fg_Size(_pMemory);
	}

	umint DDefaultCallingConv _msize_base(void *_pMemory) _CRT_NOEXCEPT
	{
		return NMib::NMemory::fg_Size(_pMemory);
	}

	umint DDefaultCallingConv _msize_dbg(void *_pMemory, int _BlockType)
	{
		return NMib::NMemory::fg_Size(_pMemory);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _expand
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	void * DDefaultCallingConv _expand(void *_pMemory, size_t _Size)
	{
		// Just fail :P
		DMibPDebugBreak;
		return _pMemory;
	}

	void * DDefaultCallingConv _expand_base(void *_pMemory, size_t _Size)
	{
		// Just fail :P
		DMibPDebugBreak;
		return _pMemory;
	}

	void * DDefaultCallingConv _expand_dbg(void *_pMemory, umint _Size, int _BlockType, char const *_pFile, int _Line)
	{
		// Just fail :P
		DMibPDebugBreak;
		return _pMemory;
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _nh_malloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	_Check_return_ _Ret_opt_bytecap_(_Size) void * __cdecl _nh_malloc(_In_ size_t _Size, _In_ int _NewHandlerFlag)
	{
		return malloc(_Size);
	}

	void * DDefaultCallingConv _nh_malloc_base(umint _Size, int _NewHandlerFlag)
	{
		return malloc(_Size);
	}

	void * DDefaultCallingConv _nh_malloc_dbg(umint _Size, int _NewHandlerFlag, int _BlockType, char const *_pFile, int _Line)
	{
		return _malloc_dbg(_Size, _BlockType, _pFile, _Line);
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

	void * DDefaultCallingConv _heap_alloc_base(umint _Size)
	{
		return malloc(_Size);
	}

	void * DDefaultCallingConv _heap_alloc_dbg(umint _Size, int _BlockType, char const *_pFile, int _Line)
	{
		return _malloc_dbg(_Size, _BlockType, _pFile, _Line);
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| _free_lk
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	void DDefaultCallingConv _free_lk(void *_pMemory)
	{
		free(_pMemory);
	}

	void DDefaultCallingConv _free_lk_base(void *_pMemory)
	{
		free(_pMemory);
	}

	void DDefaultCallingConv _free_dbg_lk(void *_pMemory, int _BlockType)
	{
		free(_pMemory);
	}

	__declspec(noalias) void DDefaultCallingConv _freea_s(void *_pMemory)
	{
		// DMibPDebugBreak;
		if (_pMemory != nullptr)
		{
			_pMemory = (char *)_pMemory - _ALLOCA_S_MARKER_SIZE;
			if (*((size_t *)_pMemory) == _ALLOCA_S_HEAP_MARKER)
				free(_pMemory);
		}
	}

	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| Debug routines
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	long DDefaultCallingConv _CrtSetBreakAlloc(long _BreakRequestNumber)
	{
		return 0;
	}

	void DDefaultCallingConv _CrtSetDbgBlockType(void *_pMemory, int _BlockType)
	{
	}

	typedef int (DDefaultCallingConv *_CRT_ALLOC_HOOK)(int _AllocationType, void *_pMemory, umint _Size, int _BlockType, long _RequestNumber, unsigned char const *_pFile, int _Line);

	_CRT_ALLOC_HOOK DDefaultCallingConv _CrtSetAllocHook(_CRT_ALLOC_HOOK _fNewHook)
	{
		return nullptr;
	}

	int DDefaultCallingConv CheckBytes(unsigned char *_pBytes, unsigned char _Expected, umint _Size)
	{
		// We actually need to do this (not just return 1) since both positive and negative returns are used :-)
		unsigned char *pEnd = _pBytes + _Size;
		while (_pBytes < pEnd)
		{
			if (*_pBytes++ != _Expected)
				return 0;
		}
		return 1;
	}

	int DDefaultCallingConv _CrtCheckMemory(void)
	{
		return NMib::fg_GetSys()->f_MemoryManager_Check(false);
	}

	int DDefaultCallingConv _CrtSetDbgFlag(int _NewFlags)
	{
		return _CRTDBG_LEAK_CHECK_DF;
	}

	void DDefaultCallingConv _CrtDoForAllClientObjects(void (DDefaultCallingConv *_fCallback)(void *_pMemory, void *_pContext), void *_pContext)
	{
	}

	int DDefaultCallingConv _CrtIsValidPointer(void const *_pMemory, unsigned int _Size, int _bReadWrite)
	{
		return
			(
				_pMemory != nullptr
#ifdef _WIN32
				&& !IsBadReadPtr(_pMemory, _Size)
				&& (!_bReadWrite || !IsBadWritePtr((LPVOID)_pMemory, _Size))
#endif  /* _WIN32 */
			)
		;
	}

	int DDefaultCallingConv _CrtIsValidHeapPointer(void const *_pMemory)
	{
		return true;
	}

	int DDefaultCallingConv _CrtIsMemoryBlock(void const *_pMemory, unsigned int _Size, long *o_pRequestNumber, char **o_pFile, int *o_pLine)
	{
		return false;
	}

	typedef void (DDefaultCallingConv *_CRT_DUMP_CLIENT)(void *_pMemory, umint _Size);

	_CRT_DUMP_CLIENT DDefaultCallingConv _CrtSetDumpClient(_CRT_DUMP_CLIENT _fNewDump)
	{
		return nullptr;
	}

	void DDefaultCallingConv _CrtMemCheckpoint(_CrtMemState *o_pState)
	{
	}

	int DDefaultCallingConv _CrtMemDifference(_CrtMemState *o_pDifference, _CrtMemState const *_pOldState, _CrtMemState const *_pNewState)
	{
		return false;
	}

	void DDefaultCallingConv _CrtMemDumpAllObjectsSince(_CrtMemState const *_pState)
	{
	}

	int DDefaultCallingConv _CrtDumpMemoryLeaks(void)
	{
		return false;
	}

	void DDefaultCallingConv _CrtMemDumpStatistics(_CrtMemState const *_pState)
	{
	}

	int DDefaultCallingConv _CrtReportBlockType(void const *_pMemory)
	{
		return 0;
	}

	int DDefaultCallingConv _CrtSetCheckCount(int _CheckCount)
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

	extern "C" bool __cdecl __acrt_uninitialize_heap(bool const _bTerminating)
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

	void * __cdecl _aligned_malloc(size_t _Size, size_t _Alignment)
	{
		return _aligned_offset_malloc_dbg(_Size, _Alignment, 0, NULL, 0);
	}

	void * __cdecl _aligned_malloc_dbg(size_t _Size, size_t _Alignment, char const *_pFile, int _Line)
	{
		return _aligned_offset_malloc_dbg(_Size, _Alignment, 0, _pFile, _Line);
	}

	void * __cdecl _aligned_realloc(void *_pMemory, size_t _Size, size_t _Alignment)
	{
		return _aligned_offset_realloc_dbg(_pMemory, _Size, _Alignment, 0, NULL, 0);
	}

	void * __cdecl _aligned_realloc_dbg(void *_pMemory, size_t _Size, size_t _Alignment, char const *_pFile, int _Line)
	{
		return _aligned_offset_realloc_dbg(_pMemory, _Size, _Alignment, 0, _pFile, _Line);
	}

	void * __cdecl _aligned_recalloc_dbg(void *_pMemory, size_t _nMembers, size_t _Size, size_t _Alignment, char const *_pFile, int _Line)
	{
		return _aligned_offset_recalloc_dbg(_pMemory, _nMembers, _Size, _Alignment, 0, _pFile, _Line);
	}

	void * __cdecl _aligned_offset_malloc(size_t _Size, size_t _Alignment, size_t _Offset)
	{
		return _aligned_offset_malloc_dbg(_Size, _Alignment, _Offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_malloc_dbg(size_t _Size, size_t _Alignment, size_t _Offset, char const *_pFile, int _Line)
	{
		uintptr_t BlockAddress, AlignedAddress, HeaderGap;
		CAlignMemBlockHdr *pHdr;
		size_t NonUserSize, BlockSize;

		/* validation section */
		_Alignment = (_Alignment > sizeof(uintptr_t) ? _Alignment : sizeof(uintptr_t)) - 1;

		HeaderGap = (0 - _Offset) & (sizeof(uintptr_t) - 1);

		NonUserSize = HeaderGap + _Alignment + sizeof(CAlignMemBlockHdr); // cannot overflow
		if (!fg_TryAddWindowsAllocationSize(_Size, NonUserSize, BlockSize))
			return NULL;

		if ((BlockAddress = (uintptr_t)_malloc_dbg(BlockSize, _NORMAL_BLOCK, _pFile, _Line)) == (uintptr_t)NULL)
			return NULL;

		AlignedAddress = ((BlockAddress + NonUserSize + _Offset) & ~_Alignment) - _Offset;
		pHdr = (CAlignMemBlockHdr *)(AlignedAddress - HeaderGap) - 1;
		memset((void *)pHdr->m_Gap, gs_AlignLandFill, DAlignGapSize);
		pHdr->m_pHead = (void *)BlockAddress;
		return (void *)AlignedAddress;
	}

	void * __cdecl _aligned_offset_realloc(void *_pMemory, size_t _Size, size_t _Alignment, size_t _Offset)
	{
		return _aligned_offset_realloc_dbg(_pMemory, _Size, _Alignment, _Offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_recalloc(void *_pMemory, size_t _nMembers, size_t _Size, size_t _Alignment, size_t _Offset)
	{
		return _aligned_offset_recalloc_dbg(_pMemory, _nMembers, _Size, _Alignment, _Offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_realloc_dbg(void *_pMemory, size_t _Size, size_t _Alignment, size_t _Offset, char const *_pFile, int _Line)
	{
		uintptr_t BlockAddress, AlignedAddress, HeaderGap, MoveSize;
		CAlignMemBlockHdr *pHdr, *pOldHeader;
		size_t NonUserSize, BlockSize;

		if (_pMemory == NULL)
			return _aligned_offset_malloc_dbg(_Size, _Alignment, _Offset, _pFile, _Line);
		if (_Size == 0)
		{
			_aligned_free_dbg(_pMemory);
			return NULL;
		}

		pOldHeader = (CAlignMemBlockHdr *)((uintptr_t)_pMemory & ~(sizeof(uintptr_t) - 1)) - 1;

		if (CheckBytes((unsigned char *)_pMemory - DNoMansLandSize, gs_NoMansLandFill, DNoMansLandSize))
		{
			// We don't know where (file, linenum) _pMemory was allocated
			_RPT1(_CRT_ERROR, "The block at 0x%p was not allocated by _aligned routines, use realloc()", _pMemory);
			errno = 22;
			return NULL;
		}

		if (!CheckBytes(pOldHeader->m_Gap, gs_AlignLandFill, DAlignGapSize))
		{
			// We don't know where (file, linenum) _pMemory was allocated
			_RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", _pMemory);
		}

		/* validation section */

		MoveSize = _msize(pOldHeader->m_pHead) - ((uintptr_t)_pMemory - (uintptr_t)pOldHeader->m_pHead);

		_Alignment = (_Alignment > sizeof(uintptr_t) ? _Alignment : sizeof(uintptr_t)) - 1;

		HeaderGap = (0 - _Offset) & (sizeof(uintptr_t) - 1);

		NonUserSize = HeaderGap + _Alignment + sizeof(CAlignMemBlockHdr); // cannot overflow
		if (!fg_TryAddWindowsAllocationSize(_Size, NonUserSize, BlockSize))
			return NULL;

		if ((BlockAddress = (uintptr_t)_malloc_dbg(BlockSize, _NORMAL_BLOCK, _pFile, _Line)) == (uintptr_t)NULL)
			return NULL;

		AlignedAddress = ((BlockAddress + NonUserSize + _Offset) & ~_Alignment) - _Offset;
		pHdr = (CAlignMemBlockHdr *)(AlignedAddress - HeaderGap) - 1;
		memset((void *)pHdr->m_Gap, gs_AlignLandFill, DAlignGapSize);
		pHdr->m_pHead = (void *)BlockAddress;

		memcpy((void *)AlignedAddress, _pMemory, MoveSize > _Size ? _Size : MoveSize);
		_free_dbg(pOldHeader->m_pHead, _NORMAL_BLOCK);

		return (void *)AlignedAddress;
	}

	void * __cdecl _aligned_offset_recalloc_dbg(void *_pMemory, size_t _nMembers, size_t _Size, size_t _Alignment, size_t _Offset, char const *_pFile, int _Line)
	{
		size_t UserSize = 0; // wanted size, passed to aligned realoc
		size_t FillStart = 0; // location where aligned recalloc starts to fill with 0
							  /* filling must start from the end of the previous user block */
							  /* and must stop at the end of the new user block */
		void *pResult = NULL; // result of aligned recalloc

		/* ensure that (_Size * _nMembers) does not overflow */
		if (!fg_TryMultiplyWindowsAllocationSize(_nMembers, _Size, UserSize))
			return NULL;

		if (_pMemory != NULL)
			FillStart = _aligned_msize(_pMemory, _Alignment, _Offset);

		pResult = _aligned_offset_realloc_dbg(_pMemory, UserSize, _Alignment, _Offset, _pFile, _Line);

		if (pResult != NULL)
		{
			if (FillStart < UserSize)
				memset((char *)pResult + FillStart, 0, UserSize - FillStart);
		}
		return pResult;
	}

	void __cdecl _aligned_free(void *_pMemory)
	{
		_aligned_free_dbg(_pMemory);
	}

	void __cdecl _aligned_free_dbg(void *_pMemory)
	{
		CAlignMemBlockHdr *pHdr;

		if (_pMemory == NULL)
			return;

		pHdr = (CAlignMemBlockHdr *)((uintptr_t)_pMemory & ~(sizeof(uintptr_t) - 1)) - 1;

		if (CheckBytes((unsigned char *)_pMemory - DNoMansLandSize, gs_NoMansLandFill, DNoMansLandSize))
		{
			// We don't know where (file, linenum) _pMemory was allocated
			_RPT1(_CRT_ERROR, "The block at 0x%p was not allocated by _aligned routines, use free()", _pMemory);
			return;
		}

		if (!CheckBytes(pHdr->m_Gap, gs_AlignLandFill, DAlignGapSize))
		{
			// We don't know where (file, linenum) _pMemory was allocated
			_RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", _pMemory);
		}
		_free_dbg((void *)pHdr->m_pHead, _NORMAL_BLOCK);
	}

	size_t __cdecl _aligned_msize(void *_pMemory, size_t _Alignment, size_t _Offset)
	{
		return _aligned_msize_dbg(_pMemory, _Alignment, _Offset);
	}

	size_t __cdecl _aligned_msize_dbg(void *_pMemory, size_t _Alignment, size_t _Offset)
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
		pHdr = (CAlignMemBlockHdr *)((uintptr_t)_pMemory & ~(sizeof(uintptr_t) - 1)) - 1;
		TotalSize = _msize(pHdr->m_pHead);
		HeaderSize = (uintptr_t)_pMemory - (uintptr_t)pHdr->m_pHead;
		Gap = (0 - _Offset) & (sizeof(uintptr_t) - 1);
		/* The align cannot be smaller than the sizeof(uintptr_t) */
		_Alignment = (_Alignment > sizeof(uintptr_t) ? _Alignment : sizeof(uintptr_t)) - 1;
		FooterSize = Gap + _Alignment + sizeof(CAlignMemBlockHdr) - HeaderSize;
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
