// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#ifdef DMibConfig_OverrideSystemMalloc

#include <Mib/Core/Core>
#include <stdlib.h>
#include <malloc.h>

using namespace NMib::NMemory;

namespace NMib
{
	extern mint g_bCreatedSystem;
	namespace NSys
	{
		void fg_CreateSystem();
		namespace NPrivate
		{
			extern mint g_PageSize;
		}
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
		_Size = NMib::fg_AlignUp(_Size, 16);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocDebug(_Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_Alloc(_Size);
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
		mint Size = _nMembers * _Size;
		Size = NMib::fg_AlignUp(Size, 16);
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
		_Size = NMib::fg_AlignUp(_Size, 16);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_ResizeDebug(_pMemory, _Size, 0, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_Resize(_pMemory, _Size, 0);
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
		_Size = NMib::fg_AlignUp(_Size, _Alignment);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(_Size, _Alignment, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAligned(_Size, _Alignment);
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
		*_pOutMemory = memalign(_Alignment, _Size);
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

		mint MaxMembers = NMib::TCLimitsInt<mint>::mc_Max / _Size;
		if (_nMembers > MaxMembers)
		{
			errno = ENOMEM;
			return nullptr;
		}

		mint UnalignedSize = mint(_nMembers) * mint(_Size);

		if ((NMib::TCLimitsInt<mint>::mc_Max - UnalignedSize) <= 16)
		{
			errno = ENOMEM;
			return nullptr;
		}

		mint Size = NMib::fg_AlignUp(UnalignedSize, 16);
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
		_Size = NMib::fg_AlignUp(_Size, NMib::NSys::NPrivate::g_PageSize);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(_Size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAligned(_Size, NMib::NSys::NPrivate::g_PageSize);
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
		_Size = NMib::fg_AlignUp(_Size, NMib::NSys::NPrivate::g_PageSize);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(_Size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMemory::CAllocator_NonTrackedHeap::f_AllocAligned(_Size, NMib::NSys::NPrivate::g_PageSize);
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
