// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifdef DMibConfig_OverrideSystemMalloc

#include <Mib/Core/Core>
#include "Malterlib_Memory_MemoryManager_Utils.h"
#include <errno.h>
#include <stdlib.h>
#include <malloc.h>

using namespace NMib::NMemory;

namespace NMib
{
	extern umint g_bCreatedSystem;
	namespace NSys
	{
		void fg_CreateSystem();
		namespace NPrivate
		{
			extern umint g_PageSize;
		}
	}
}

namespace
{
	inline_always bool fg_TryAlignSystemAllocationSize(umint &o_Size, umint _Alignment)
	{
		if (!NMib::NMemory::NPrivate::fg_IsAllocationAlignmentValid(_Alignment))
		{
			errno = EINVAL;
			return false;
		}

		if (!NMib::NMemory::NPrivate::fg_TryAlignAllocationSize(o_Size, _Alignment))
		{
			errno = ENOMEM;
			return false;
		}

		return true;
	}
}

extern "C"
{
	/* Allocate SIZE bytes of memory.  */
	module_export void *malloc(size_t _Size) __THROW __wur
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		umint Size = umint(_Size);
		if (!fg_TryAlignSystemAllocationSize(Size, 16))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocDebug(Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_Alloc(Size);
#		endif
	}

	module_export void *calloc (size_t _nMembers, size_t _Size) __THROW __wur
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		umint Size;
		if (NMib::fg_MultiplyOverflow(umint(_nMembers), umint(_Size), Size))
		{
			errno = ENOMEM;
			return nullptr;
		}
		if (!fg_TryAlignSystemAllocationSize(Size, 16))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			auto pRet = NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocDebug(Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			auto pRet = NMib::NMemory::CAllocator_NonTrackedHeap::f_Alloc(Size);
#		endif
		NMib::NMemory::fg_MemClear(pRet, Size);
		return pRet;
	}

	module_export void *realloc (void *_pMemory, size_t _Size) __THROW
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		umint Size = umint(_Size);
		if (!fg_TryAlignSystemAllocationSize(Size, 16))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_ResizeDebug(_pMemory, Size, 0, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_Resize(_pMemory, Size, 0);
#		endif
	}

	module_export void free (void *_pMemory) __THROW
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		NMib::NMemory::CAllocator_NonTrackedHeap::f_FreeNoSize(_pMemory);
	}

	module_export void cfree (void *_pMemory) __THROW
	{
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		NMib::NMemory::CAllocator_NonTrackedHeap::f_FreeNoSize(_pMemory);
	}
	module_export void *memalign (size_t _Alignment, size_t _Size) __THROW __wur
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		umint Size = umint(_Size);
		if (!fg_TryAlignSystemAllocationSize(Size, umint(_Alignment)))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(Size, _Alignment, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAligned(Size, _Alignment);
#		endif
	}

	module_export void *__libc_memalign(size_t _Alignment, size_t _Size) __THROW __wur
	{
		return memalign(_Alignment, _Size);
	}

	module_export void *aligned_alloc(size_t _Alignment, size_t _Size) __THROW __wur
	{
		return memalign(_Alignment, _Size);
	}

	module_export int posix_memalign(void **_pOutMemory, size_t _Alignment, size_t _Size)
	{
		if (_Alignment < sizeof(void *) || !NMib::NMemory::NPrivate::fg_IsAllocationAlignmentValid(umint(_Alignment)))
			return EINVAL;

		errno = 0;
		void *pMemory = memalign(_Alignment, _Size);
		if (!pMemory)
			return errno ? errno : ENOMEM;

		*_pOutMemory = pMemory;
		return 0;
	}

	module_export void *reallocarray(void *_pMemory, size_t _nMembers, size_t _Size)
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		if (_Size == 0)
			_Size = 1;

		umint UnalignedSize;
		if (NMib::fg_MultiplyOverflow(umint(_nMembers), umint(_Size), UnalignedSize))
		{
			errno = ENOMEM;
			return nullptr;
		}

		umint Size = UnalignedSize;
		if (!fg_TryAlignSystemAllocationSize(Size, 16))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_ResizeDebug(_pMemory, Size, 0, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_Resize(_pMemory, Size, 0);
#		endif
	}

	module_export struct mallinfo mallinfo(void)
	{
		struct mallinfo res;
		fg_MemClear(res);
		return res;
	}

	int mallopt(int _Command, int _Value)
	{
		return 0;
	}

	void malloc_stats(void)
	{
	}

	module_export void *valloc(size_t _Size) __THROW __wur
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		umint Size = umint(_Size);
		if (!fg_TryAlignSystemAllocationSize(Size, NMib::NSys::NPrivate::g_PageSize))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(Size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAligned(Size, NMib::NSys::NPrivate::g_PageSize);
#		endif
	}
	module_export void * pvalloc(size_t _Size) __THROW __wur
	{
#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		umint Size = umint(_Size);
		if (!fg_TryAlignSystemAllocationSize(Size, NMib::NSys::NPrivate::g_PageSize))
			return nullptr;
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(Size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAligned(Size, NMib::NSys::NPrivate::g_PageSize);
#		endif
	}
	module_export size_t malloc_usable_size(void *_pMemory) __THROW
	{
		if (!_pMemory)
			return 0;

#ifndef DMibInitInPreInitArray
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		return NMib::NMemory::CAllocator_NonTrackedHeap::f_Size(_pMemory);
	}
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
