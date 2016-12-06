// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#ifdef DMibConfig_OverrideSystemMalloc

#include <Mib/Core/Core>
#include <stdlib.h>
#include <malloc.h>

using namespace NMib::NMem;

namespace NMib
{
	namespace NSys
	{
		namespace NPrivate
		{
			extern mint g_PageSize;
		}
	}
}

extern "C"
{
	/* Allocate SIZE bytes of memory.  */
	module_export void *malloc(size_t __size) __THROW __wur
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		__size = NMib::fg_AlignUp(__size, 16);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocDebug(__size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMem::CAllocator_NonTrackedHeap::f_Alloc(__size);
#		endif
	}

	module_export void *calloc (size_t __nmemb, size_t __size) __THROW __wur
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		mint Size = __nmemb * __size;
		Size = NMib::fg_AlignUp(Size, 16);
#		if DMibConfig_MalterlibMemoryManager_Debug
			auto pRet = NMib::NMem::CAllocator_NonTrackedHeap::f_AllocDebug(Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			auto pRet = NMib::NMem::CAllocator_NonTrackedHeap::f_Alloc(Size);
#		endif
		NMib::NMem::fg_MemClear(pRet, Size);
		return pRet;
	}	
	
	module_export void *realloc (void *__ptr, size_t __size) __THROW
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		__size = NMib::fg_AlignUp(__size, 16);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::CAllocator_NonTrackedHeap::f_ResizeDebug(__ptr, __size, 0, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMem::CAllocator_NonTrackedHeap::f_Resize(__ptr, __size);
#		endif
	}
	
	module_export void free (void *__ptr) __THROW
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		NMib::NMem::CAllocator_NonTrackedHeap::f_Free(__ptr);
	}

	module_export void cfree (void *__ptr) __THROW
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		NMib::NMem::CAllocator_NonTrackedHeap::f_Free(__ptr);
	}
	module_export void *memalign (size_t __alignment, size_t __size) __THROW __wur
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		__size = NMib::fg_AlignUp(__size, __alignment);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(__size, __alignment, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocAligned(__size, __alignment);
#		endif
	}
	module_export void *valloc (size_t __size) __THROW __wur
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		__size = NMib::fg_AlignUp(__size, NMib::NSys::NPrivate::g_PageSize);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(__size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocAligned(__size, NMib::NSys::NPrivate::g_PageSize);
#		endif
	}
	module_export void * pvalloc (size_t __size) __THROW __wur
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		__size = NMib::fg_AlignUp(__size, NMib::NSys::NPrivate::g_PageSize);
#		if DMibConfig_MalterlibMemoryManager_Debug
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocAlignedDebug(__size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore);
#		else
			return NMib::NMem::CAllocator_NonTrackedHeap::f_AllocAligned(__size, NMib::NSys::NPrivate::g_PageSize);
#		endif
	}
	module_export size_t malloc_usable_size (void *__ptr) __THROW
	{
		DMibFastCheck(NMib::g_bCanUseSystemMalloc);
		return NMib::NMem::CAllocator_NonTrackedHeap::f_Size(__ptr);
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

#endif
