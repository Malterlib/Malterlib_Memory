// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <stdlib.h>
#include <malloc.h>

#ifdef DArchitechture_x86
	#define DMibMemoryAssumeAlignment 8
#elif defined(DArchitechture_x64)
	#define DMibMemoryAssumeAlignment 16
#else
	#define DMibMemoryAssumeAlignment 8
#endif

//#define DMibMemoryCorrectAlignment
#define DMibMemoryCorectSize

namespace NMib
{

	ch8 const* g_pMemoryManagerName = "Linux system memory manager";
	
	namespace NMem
	{
		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
		}
		
		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
		}
		
		struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
		{
			static constexpr bool mc_SupportsNonTracked = false;
			static constexpr bool mc_SupportsDebug = false;
			inline_always static void DMibCrossmoduleAPI fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
				DMibMemoryReportAllocatorName(g_pMemoryManagerName, g_pMemoryManagerName);
			}
			inline_always static void DMibCrossmoduleAPI fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
				DMibMemoryReportAllocatorDelete(g_pMemoryManagerName, g_pMemoryManagerName);
			}

			inline_always static void * DMibCrossmoduleAPI fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
				DMibMemoryReportSaveVar(RequestedSize, _Size);
				void *pRet = malloc(_Size);
				DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
				return pRet;
			}

			inline_always static void * DMibCrossmoduleAPI fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				return fg_MemClear(fs_Alloc(_pModule, _Size), _Size);
			}

			inline_always static void * DMibCrossmoduleAPI fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
			{
	#ifdef DMibMemoryCorrectAlignment
	#ifdef DMibMemoryAssumeAlignment
				if (_Alignment <= DMibMemoryAssumeAlignment)
					return CCrossModuleImplementation::fs_Alloc(_pModule, _Size);
	#endif
				DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
				DMibMemoryReportSaveVar(RequestedSize, _Size);
				void *pRet = memalign(_Alignment, _Size);
				DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
				DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
				return pRet;
	#else
				return CCrossModuleImplementation::fs_Alloc(_pModule, _Size);
	#endif
			}
		};
		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = malloc(_Size);
#ifdef DMibMemoryCorectSize
			_Size = malloc_usable_size(pRet);
#endif
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(fs_AllocWithSize(_pModule, _Size), _Size);
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
#ifdef DMibMemoryCorrectAlignment
#ifdef DMibMemoryAssumeAlignment
			if (_Alignment <= DMibMemoryAssumeAlignment)
				return CCrossModuleImplementation::fs_AllocWithSize(_pModule, _Size);
#endif
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = memalign(_Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
#ifdef DMibMemoryCorectSize
			_Size = malloc_usable_size(pRet);
#endif
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
#else
			return CCrossModuleImplementation::fs_AllocWithSize(_pModule, _Size);
#endif
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
			void *pRet = realloc(_pMemory, _Size);
#ifdef DMibMemoryCorectSize
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = malloc_usable_size(pRet);
#endif
			DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
			void *pRet = realloc(_pMemory, _Size);
#ifdef DMibMemoryCorectSize
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = malloc_usable_size(pRet);
#endif
			DMibMemoryReportResize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			if (!_pMemory)
				return;

			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(Size, fs_SizePadded(_pModule, _Size));
			free(_pMemory);
			DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
		}

		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			if (!_pMemory)
				return;

			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(Size, malloc_usable_size(_pMemory));
			free(_pMemory);
			DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
		}

		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			mint Ret = malloc_usable_size((void *)_pMemory);
			DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
			return Ret;
		}

		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		
		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_AlignUp(_Size, sizeof(void *) * 2);
		}
		
		inline_always fp32 DMibCrossmoduleAPI CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return 0.0;
		}
		
		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return 16;
		}
	}
	
} // Namespace NMib

