// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#ifdef DPlatfromFamily_OSX
#include <malloc/malloc.h>
#endif

#ifdef DPlatformFamily_Windows
#include <windows.h>
#endif

HANDLE g_pMainHeap = nullptr;

namespace NMib
{
	ch8 const* g_pMemoryManagerName = "Windows system memory manager";

	namespace NMem
	{
		inline_always void CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			//g_pMainHeap = HeapCreate(0, 0);
			g_pMainHeap = GetProcessHeap();
			ULONG Enable = 3;
			HeapSetInformation(g_pMainHeap, HeapCompatibilityInformation, &Enable, sizeof(Enable));
		}
	
		inline_always void CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			//if (fg_GetSys()->f_IsDll())
			//	HeapDestroy(g_pMainHeap);
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

			inline_always static void *fs_NonTracked_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
			{
				void * pRet = HeapAlloc(g_pMainHeap, 0, _Size);
				_Size = HeapSize(g_pMainHeap, 0, pRet);
				return pRet;
			}
			inline_always static void *fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
			{
				void *pRet = HeapAlloc(g_pMainHeap, 0, _Size);
				_Size = HeapSize(g_pMainHeap, 0, pRet);
				return pRet;
			}
			inline_always static void *fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				void * pRet = HeapAlloc(g_pMainHeap, 0, _Size);
				return pRet;
			}
			inline_always static void *fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
			{
				void *pRet = HeapAlloc(g_pMainHeap, 0, _Size);
				return pRet;
			}
			inline_always static void *fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
			{
				void *pRet = HeapReAlloc(g_pMainHeap, 0, _pMemory, _Size);
				if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
					_Size = HeapSize(g_pMainHeap, 0, pRet);
				return pRet;
			}

			inline_always static void *fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
			{
				void *pRet = HeapReAlloc(g_pMainHeap, 0, _pMemory, _Size);
				if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
					_Size = HeapSize(g_pMainHeap, 0, pRet);
				return pRet;
			}			

			inline_always static void fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
			{
				HeapFree(g_pMainHeap, 0, _pMemory);
			}

			inline_always static void fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				HeapFree(g_pMainHeap, 0, _pMemory);
			}

			inline_always static mint fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				return HeapSize(g_pMainHeap, 0, _pMemory);
			}

			inline_always static mint fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				DMibPDebugBreak; // Not supported
				return 0;
			}

			inline_always static void *fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
				DMibMemoryReportSaveVar(RequestedSize, _Size);
				void * pRet = HeapAlloc(g_pMainHeap, 0, _Size);
				DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
				return pRet;
			}

			inline_always static void *fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				return fg_MemClear(fs_Alloc(_pModule, _Size), _Size);
			}

			inline_always static void *fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
			{
				DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
				DMibMemoryReportSaveVar(RequestedSize, _Size);
				void *pRet = HeapAlloc(g_pMainHeap, 0, _Size);
				DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
				return pRet;
			}

		};
	
		inline_always void * CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void * pRet = HeapAlloc(g_pMainHeap, 0, _Size);
			_Size = HeapSize(g_pMainHeap, 0, pRet);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void * CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(fs_AllocWithSize(_pModule, _Size), _Size);
		}

		inline_always void * CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = HeapAlloc(g_pMainHeap, 0, _Size);
			_Size = HeapSize(g_pMainHeap, 0, pRet);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void * CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
			void *pRet = HeapReAlloc(g_pMainHeap, 0, _pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = HeapSize(g_pMainHeap, 0, pRet);
			DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void * CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
			void *pRet = HeapReAlloc(g_pMainHeap, 0, _pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = HeapSize(g_pMainHeap, 0, pRet);
			DMibMemoryReportResize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			if (!_pMemory)
				return;
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(Size, fs_SizePadded(_pModule, _Size));
			HeapFree(g_pMainHeap, 0, _pMemory);
			DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
		}

		inline_always void CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			if (!_pMemory)
				return;
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(Size, HeapSize(g_pMainHeap, 0, _pMemory));
			HeapFree(g_pMainHeap, 0, _pMemory);
			DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
		}

		inline_always mint CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			mint Ret = HeapSize(g_pMainHeap, 0, _pMemory);
			DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
			return Ret;
		}

		inline_always mint CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		
		inline_always mint CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_AlignUp(_Size, 16);
		}
		
		inline_always fp32 CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return 0.0;
		}
		
		inline_always mint CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return 16;
		}

	} // Namespace NMem
} // Namespace NMib
