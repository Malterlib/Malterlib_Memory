// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_MemoryManager.hpp"
#include "Malterlib_Memory_MemoryManager_Tracked.h"

#if DMibConfig_MalterlibMemoryManager_Debug && DMibConfig_MalterlibMemoryManager_Debug_Features || DMibConfig_MalterlibMemoryManager_Debug_Features == 1
#	define DEnableDebugMemoryManager 1
#else
#	define DEnableDebugMemoryManager 0
#endif

#if DEnableDebugMemoryManager
#	include "Malterlib_Memory_MemoryManager_Debug.h"
#endif

#include "Malterlib_Memory_Reporter_CategoriesInterface.h"

namespace NMib
{

	struct CMemoryManagerParams : public NMem::CDefaultMemoryManagerParams
	{
		typedef CMainHeapVirtualAllocator CAllocator;
		static const EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
	};

#	if DEnableDebugMemoryManager
		struct CMemoryManagerDebugOptions : public NMem::CMemoryManagerDebugOptionsDefault
		{
			enum
			{
				EDummy
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableFreedGuards
				, mc_bCheckModifyAfterFree	= false
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableAllocatedFill
				, mc_bFillAllocated			= false
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableStackTrace
				, mc_StackTraceDepth		= 0
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableEdgeGuards
				, mc_nPreGuardBytes			= 0
				, mc_nPostGuardBytes		= 0
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableFreeValidation
				, mc_bFreeValidation		= false
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableFreeValidation && !DMibConfig_MalterlibMemoryManager_Debug_EnableMemoryLeaks
				, mc_bEnumeration			= false
	#endif
	#if  !DMibConfig_MalterlibMemoryManager_Debug_EnableMemoryLeaks
				, mc_bTraceLeaks			= false
	#endif
			};
		};
		typedef NMem::TCMemoryManagerDebug<CMemoryManagerParams, false, CMemoryManagerDebugOptions> CMemoryManagerWithDebug;
#	else
		typedef NMem::TCMemoryManager<CMemoryManagerParams> CMemoryManagerWithDebug;
#	endif

#if !DMibConfig_MemoryManager_Stats_EnableCategories
	namespace NMem
	{
		typedef void CTrackedAllocationInfo;
	}
#endif
	
	typedef NMem::TCMemoryManagerTracked<CMemoryManagerWithDebug, NMem::CTrackedAllocationInfo> CMemoryManager;
	
	NMib::NAggregate::TCAggregateSimple<CMemoryManager> g_MainHeap = {DAggregateInit};
	
	struct CMemoryManagerNonTrackedParams : public CMemoryManagerParams
	{
		static const EAllocationFlag mc_AllocationFlags = EAllocationFlag_NonTrackedMainHeap;
		typedef NMem::CAllocator_VirtualNoTracking CAllocator;
		static const bool mc_bBackgroundCleanup = false; // Threading potentially recursive allocations
	};

#if DEnableDebugMemoryManager
	struct CMemoryManagerNonTrackedDebugOptions : public CMemoryManagerDebugOptions
	{
		enum
		{
			mc_bCanAllocateNonTracked = false // Threading potentially recursive allocations
		};
	};
	typedef NMem::TCMemoryManagerDebug<CMemoryManagerNonTrackedParams, false, CMemoryManagerNonTrackedDebugOptions> CMemoryManagerNonTracked;
#else
	typedef NMem::TCMemoryManager<CMemoryManagerNonTrackedParams> CMemoryManagerNonTracked;
#endif
	
	NMib::NAggregate::TCAggregateSimple<CMemoryManagerNonTracked> g_NonTrackedHeap = {DAggregateInit};
	NMib::NThread::CMutualAggregate g_MemoryManagerForkLock = {DAggregateInit};
	mint g_MemoryManagerForkedCount = 0;
	bool g_MemoryManagerUnforked = false;

	namespace NMem
	{
		inline_always void CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			g_NonTrackedHeap.f_Construct();
		}
		
		inline_always void CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
#			if DEnableDebugMemoryManager
				if (!CSystem::ms_bDisableMemoryManagerLeakReport)
					g_NonTrackedHeap->f_ReportLeaks();
#			endif
			if (!g_bMemoryManagerNeededAfterDestroy)
			{
#				ifdef DMibConfig_HeapNeverDestroyed
					g_NonTrackedHeap->f_DestroyThreadLocals(); // For debug checks later
#				else
					g_NonTrackedHeap.f_Destruct();
					g_MemoryManagerForkLock.f_Destruct();
#				endif
			}
		}
		
		struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
		{
			inline_always static void fs_MemoryManager_GarbageCollect(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap->f_GarbageCollect(true);
				g_NonTrackedHeap->f_GarbageCollect(true);
			}
			
			inline_always static void fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap.f_Construct("Main memory manager");
				{
					// Make sure the code for checking out manager is included
					auto MemoryManagerCheckout = NMib::fg_GetSys()->f_MemoryManager_Checkout();
					NSys::fg_Compiler_MakeActive(1, &MemoryManagerCheckout);
				}
			}
			inline_always static CMemoryManagerCheckout fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule)
			{
				return g_MainHeap->f_CheckoutVirtual();
			}	
			
			inline_always static void fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
		#		if DEnableDebugMemoryManager
				{
					if (!CSystem::ms_bDisableMemoryManagerLeakReport)
						g_MainHeap->f_ReportLeaks();
				}
		#		endif
				{
					if (!g_bMemoryManagerNeededAfterDestroy)
					{
#					ifdef DMibConfig_HeapNeverDestroyed
						g_MainHeap->f_DestroyThreadLocals(); // For debug checks later
#					else
						g_MainHeap.f_Destruct();
#					endif
					}
				}
			}
			
			inline_always static bool fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
			{
				bool bRet = true;
#				if DEnableDebugMemoryManager
				{
					if (!g_MainHeap->f_CheckAll(_bBreak))
						bRet = false;
					if (!g_NonTrackedHeap->f_CheckAll(_bBreak))
						bRet = false;
				}
#				endif
				return bRet;
			}

			inline_always static bool fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule)
			{
	#			if DEnableDebugMemoryManager
					return g_MainHeap->f_ReportingLeaks();
	#			else
					return false;
	#			endif
			}
			inline_always static void fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
			{
				g_MemoryManagerForkLock.f_Lock();
				if (++g_MemoryManagerForkedCount > 1)
					return;
				g_MemoryManagerForkLock.f_PrepareFork();
		
				g_MemoryManagerUnforked = false;
				
				g_MainHeap->f_Lock();
				g_NonTrackedHeap->f_Lock();
				
				g_MainHeap->f_CheckoutManual();
				g_NonTrackedHeap->f_CheckoutManual();

				g_MainHeap->f_PrepareFork();
				g_NonTrackedHeap->f_PrepareFork();
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
				g_NonTrackedHeap->f_ForkedParent();
				g_MainHeap->f_ForkedParent();
				
				g_NonTrackedHeap->f_CheckinManual();
				g_MainHeap->f_CheckinManual();

				g_NonTrackedHeap->f_Unlock();
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
				g_NonTrackedHeap->f_ForkedChild();
				g_MainHeap->f_ForkedChild();

				g_NonTrackedHeap->f_CheckinManual();
				g_MainHeap->f_CheckinManual();

				g_NonTrackedHeap->f_Unlock();
				g_MainHeap->f_Unlock();
				
				g_MemoryManagerForkLock.f_Unlock();
			}
			
			inline_always static void fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap->f_DestroyCleanupThreads();
				g_NonTrackedHeap->f_DestroyCleanupThreads();
			}
			
			inline_always static void fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap->f_CanStartThreads();
				g_NonTrackedHeap->f_CanStartThreads();
			}

			inline_always static void fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode)
			{
				g_MainHeap->f_SetNumaNode(_NumaNode);
				// Don't bother with non-tracked heap as that should not be used a lot
				//g_NonTrackedHeap->f_SetNumaNode(_NumaNode);
			}
			static const bool mc_SupportsNonTracked = true;
#			if DEnableDebugMemoryManager
				static const bool mc_SupportsDebug = true;
				inline_always static void * fs_AllocDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_MainHeap->f_AllocDebug(_Size, _pFile, _Line, _Flags);
				}
				inline_always static void * fs_AllocAlignedDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_MainHeap->f_AllocAlignedDebug(_Size, _Align, _pFile, _Line, _Flags);
				}
				inline_always static void * fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_MainHeap->f_ReallocDebug(_pMemory, _Size, _pFile, _Line, _Flags);
				}
				inline_always static void * fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
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
#			else
				static const bool mc_SupportsDebug = false;
#			endif

			inline_always static mint fs_NonTracked_Granularity(CMemoryManagerCrossModule *_pModule)
			{
				return 1;
			}

			inline_always static mint fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pBlock)
			{
				return g_NonTrackedHeap->f_Size(_pBlock);
			}

			inline_always static mint fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pBlock)
			{
				return g_NonTrackedHeap->f_TrySize(_pBlock);
			}

			inline_always static mint fs_NonTracked_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				return g_NonTrackedHeap->f_SizePadded(_Size);
			}

			inline_always static fp32 fs_NonTracked_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pBlock) // Number of bytes overhead for block
			{
				return g_NonTrackedHeap->f_Overhead(_pBlock);
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
				inline_always static void *fs_NonTracked_AllocDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_NonTrackedHeap->f_AllocDebug(_Size, _pFile, _Line, _Flags);
				}
				inline_always static void *fs_NonTracked_AllocAlignedDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_NonTrackedHeap->f_AllocAlignedDebug(_Size, _Align, _pFile, _Line, _Flags);
				}
				inline_always static void *fs_NonTracked_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_NonTrackedHeap->f_ReallocDebug(_pMemory, _Size, _pFile, _Line, _Flags);
				}
				inline_always static void *fs_NonTracked_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					return g_NonTrackedHeap->f_ResizeDebug(_pMemory, _Size, _pFile, _Line, _Flags);
				}
			
				inline_always static void fs_NonTracked_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					g_NonTrackedHeap->f_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
				}
				inline_always static void fs_NonTracked_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
				{
					g_NonTrackedHeap->f_AllocBatchDebug
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
	#		endif
			
			inline_always static void *fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
			{
				return g_NonTrackedHeap->f_Alloc(_Size);
			}
			inline_always static void *fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
			{
				return g_NonTrackedHeap->f_AllocAligned(_Size, _Alignment);
			}
			inline_always static void *fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size)
			{
				return g_NonTrackedHeap->f_Realloc(_pMem, _Size);
			}

			inline_always static void *fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size)
			{
				return g_NonTrackedHeap->f_Resize(_pMem, _Size);
			}

			inline_always static void fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pBlock)
			{
				return g_NonTrackedHeap->f_Free(_pBlock);
			}
		};

		inline_always void * CCrossModuleImplementation::fs_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return g_MainHeap->f_Alloc(_Size);
		}

		inline_always void * CCrossModuleImplementation::fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(g_MainHeap->f_Alloc(_Size), _Size);
		}

		inline_always void * CCrossModuleImplementation::fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align)
		{
			return g_MainHeap->f_AllocAligned(_Size, _Align);
		}


		inline_always void * CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return g_MainHeap->f_Realloc(_pMemory, _Size);
		}

		
		inline_always void * CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
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
