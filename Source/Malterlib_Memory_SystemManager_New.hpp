// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if DMibConfig_Memory_Shims_Enable
	#include "Malterlib_Memory_SystemManager_NewWithTracking.hpp"
#else

#include "Malterlib_Memory_MemoryManager.h"
#include "Malterlib_Memory_SystemManager_New.h"

namespace NMib
{
	NMib::NAggregate::TCAggregateSimple<CMemoryManager> g_MainHeap = {DAggregateInit};
	NMib::NThread::CMutualAggregate g_MemoryManagerForkLock = {DAggregateInit};
	mint g_MemoryManagerForkedCount = 0;
	bool g_MemoryManagerUnforked = false;
	
	namespace NMem
	{
		inline_always void CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			g_MainHeap.f_Construct(CMemoryManagerConfig());
			{
				// Make sure the code for checking out manager is included
				auto MemoryManagerCheckout = NMib::fg_GetSys()->f_MemoryManager_Checkout();
				NSys::fg_Compiler_MakeActive(1, &MemoryManagerCheckout);
			}
		}
		
		inline_always void CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
#			if DEnableDebugMemoryManager
				if (!CSystem::ms_bDisableMemoryManagerLeakReport)
					g_MainHeap->f_ReportLeaks();
#			endif
#if !defined(DMibMemoryOverrideDll)
			if (fg_GetSys()->f_IsDll())
			{
				g_MainHeap.f_Destruct(); // We only need to destroy the heap if we are a DLL. Not doing this in exes can significantly speed up exit times
				g_MemoryManagerForkLock.f_Destruct();
			}
			else
#endif
			{
				if (!g_bMemoryManagerNeededAfterDestroy)
				{
#					ifdef DMibConfig_HeapNeverDestroyed
						g_MainHeap->f_DestroyThreadLocals(); // For debug checks later
#					else
						g_MainHeap.f_Destruct();
						g_MemoryManagerForkLock.f_Destruct();			
#					endif
				}
			}
			
		}
		
		struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
		{
			inline_always static void fs_MemoryManager_GarbageCollect(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap->f_GarbageCollect(true);
			}

			inline_always static NMem::CMemoryManagerCheckout fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule)
			{
				return g_MainHeap->f_CheckoutVirtual();
			}	
			
			inline_always static void fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
			}
			
			inline_always static bool fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
			{
	#			if DEnableDebugMemoryManager
					return g_MainHeap->f_CheckAll(_bBreak);
	#			else
					return true;
	#			endif
			}
			
			inline_always static bool fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule)
			{
	#			if DEnableDebugMemoryManager
					return g_MainHeap->f_ReportingLeaks();
	#			else
					return false;
	#			endif
			}
			

			static constexpr bool mc_SupportsNonTracked = true;

			inline_always static void fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
			{
				g_MemoryManagerForkLock.f_Lock();
				if (++g_MemoryManagerForkedCount > 1)
					return;
				g_MemoryManagerForkLock.f_PrepareFork();
		
				g_MemoryManagerUnforked = false;
				
				g_MainHeap->f_Lock();
				g_MainHeap->f_CheckoutManual();
				g_MainHeap->f_PrepareFork();
			}
			
			inline_always static void fs_MemoryManager_ForkedParent(CMemoryManagerCrossModule *_pModule)
			{
				--g_MemoryManagerForkedCount;
				if (g_MemoryManagerUnforked)
				{
					g_MemoryManagerForkLock.f_Unlock();
					return;
				}
				g_MemoryManagerForkLock.f_ForkedParent();
				g_MemoryManagerUnforked = true;
				
				g_MainHeap->f_ForkedParent();
				g_MainHeap->f_CheckinManual();
				g_MainHeap->f_Unlock();

				g_MemoryManagerForkLock.f_Unlock();
			}
			
			inline_always static void fs_MemoryManager_ForkedChild(CMemoryManagerCrossModule *_pModule)
			{
				--g_MemoryManagerForkedCount;
				if (g_MemoryManagerUnforked)
				{
					g_MemoryManagerForkLock.f_Unlock();
					return;
				}
				g_MemoryManagerForkLock.f_ForkedChild();
				g_MemoryManagerUnforked = true;

				g_MainHeap->f_ForkedChild();
				g_MainHeap->f_CheckinManual();
				g_MainHeap->f_Unlock();

				g_MemoryManagerForkLock.f_Unlock();
			}

			inline_always static void fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap->f_DestroyCleanupThreads();
			}

			inline_always static void fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap->f_CanStartThreads();
			}
			
			inline_always static void fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode)
			{
				g_MainHeap->f_SetNumaNode(_NumaNode);
			}
		
			inline_always static void fs_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
			{
				g_MainHeap->f_AllocBatch(_Size, _Alignment, _Functor);
			}
			inline_always static void fs_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
			{
				g_MainHeap->f_AllocBatch
					(
						_Size
						, _Alignment
						, [&](void * _pAlloc, mint _Size)
						{
							return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
						}
					)
				;
			}
			inline_always static void fs_NonTracked_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
			{
				g_MainHeap->f_AllocBatch(_Size, _Alignment, _Functor);
			}
			inline_always static void fs_NonTracked_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
			{
				g_MainHeap->f_AllocBatch
					(
						_Size
						, _Alignment
						, [&](void * _pAlloc, mint _Size)
						{
							return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
						}
					)
				;
			}
			
#		if DEnableDebugMemoryManager
			static constexpr bool mc_SupportsDebug = true;
			inline_always static void *fs_AllocDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_MainHeap->f_AllocDebug(_Size, _pFile, _Line, _Flags);
			}
			inline_always static void *fs_AllocAlignedDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_MainHeap->f_AllocAlignedDebug(_Size, _Align, _pFile, _Line, _Flags);
			}
			inline_always static void *fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_MainHeap->f_ReallocDebug(_pMemory, _Size, _pFile, _Line, _Flags);
			}
			inline_always static void *fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_MainHeap->f_ResizeDebug(_pMemory, _Size, _pFile, _Line, _Flags);
			}
			inline_always static void fs_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				g_MainHeap->f_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
			}
			inline_always static void fs_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				g_MainHeap->f_AllocBatchDebug
					(
						_Size
						, _Alignment
						, [&](void * _pAlloc, mint _Size)
						{
							return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
						}
						, _pFile
						, _Line
						, _Flags
					)
				;
			}
#		else
			static constexpr bool mc_SupportsDebug = false;
#		endif
		};
		

		inline_always void *CCrossModuleImplementation::fs_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return g_MainHeap->f_AllocAligned(_Size, 1);
		}

		inline_always void *CCrossModuleImplementation::fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(g_MainHeap->f_AllocAligned(_Size, 1), _Size);
		}

		inline_always void *CCrossModuleImplementation::fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align)
		{
			return g_MainHeap->f_AllocAligned(_Size, _Align);
		}

		inline_always void *CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return g_MainHeap->f_Realloc(_pMemory, _Size);
		}
		
		inline_always void *CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return g_MainHeap->f_Resize(_pMemory, _Size);
		}

		inline_always void CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			g_MainHeap->f_Free(_pMemory);
		}

		inline_always mint CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			return g_MainHeap->f_Size(_pMemory);
		}
		
		inline_always mint CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			return g_MainHeap->f_TrySize(_pMemory);
		}
		
		inline_always mint CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return g_MainHeap->f_SizePadded(_Size);
		}
		
		inline_always fp32 CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return g_MainHeap->f_Overhead(_pMemory);
		}
		
		inline_always mint CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return 1;
		}

	} // Namespace NMem
} // Namespace NMib

#endif // else DMibConfig_Memory_Shims_Enable
