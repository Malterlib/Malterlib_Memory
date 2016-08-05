// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "../SDK/LLAlloc/ll_alloc.h"

namespace NMib
{
	ch8 const* g_pMemoryManagerName = "LLAlloc memory manager";

	namespace NMem
	{
		void CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
		}
	
		void CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
		}
	
		struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
		{
			static constexpr bool mc_SupportsNonTracked = true;
			static constexpr bool mc_SupportsDebug = false;
			inline_always static void fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
				DMibMemoryReportAllocatorName(g_pMemoryManagerName, g_pMemoryManagerName);
			}
	
			inline_always static void fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
				DMibMemoryReportAllocatorDelete(g_pMemoryManagerName, g_pMemoryManagerName);
			}

			inline_always static void *fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
			{
				auto *pRet = llalloc_malloc(_Size);
				_Size = llalloc__msize(pRet);
				return pRet;
			}
			inline_always static void *fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignmentment)
			{
				auto *pRet = llalloc_memalign(_Alignmentment, _Size);
				_Size = llalloc__msize(pRet);
				//_Size = HeapSize(g_pMainHeap, 0, pRet);
				return pRet;
			}
			inline_always static void *fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
			{
				auto *pRet = llalloc_realloc(_pMemory, _Size);
				_Size = llalloc__msize(pRet);
				return pRet;
			}

			inline_always static void *fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
			{
				auto *pRet = llalloc_realloc(_pMemory, _Size);
				_Size = llalloc__msize(pRet);
				return pRet;
			}			

			inline_always static void fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				llalloc_free(_pMemory);
			}

			inline_always static mint fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				return llalloc__msize((void *)_pMemory);
			}

			inline_always static mint fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				DMibPDebugBreak; // Not supported
				return 0;
			}
		};

		void * CCrossModuleImplementation::fs_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			auto *pRet = llalloc_malloc(_Size);
			_Size = llalloc__msize(pRet);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		void * CCrossModuleImplementation::fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(fs_Alloc(_pModule, _Size), _Size);
		}

		void * CCrossModuleImplementation::fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			auto *pRet = llalloc_memalign(_Alignment, _Size);
			_Size = llalloc__msize(pRet);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		void * CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(mint Size = fs_Size(_pModule, _pMemory));
			auto *pRet = llalloc_realloc(_pMemory, _Size);
			_Size = llalloc__msize(pRet);
			DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		void * CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(mint Size = fs_Size(_pModule, _pMemory));
			auto *pRet = llalloc_realloc(_pMemory, _Size);
			_Size = llalloc__msize(pRet);
			DMibMemoryReportResize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		void CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			if (!_pMemory)
				return;
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(Size, llalloc__msize(_pMemory));
			llalloc_free(_pMemory);
			DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
		}

		mint CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			mint Ret = llalloc__msize((void *)_pMemory);
			DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
			return Ret;
		}

		inline_always mint CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		
		mint CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_AlignUp(_Size, 16);
		}
		
		fp32 CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return 0.0;
		}
		
		mint CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return 16;
		}
	
	} // Namespace NMem
} // Namespace NMib

