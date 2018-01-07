// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

DMibAppNoClass;
DMibPMain;

NMib::NThread::TCThreadLocal<NMib::NStr::CStr, NMib::NMem::CAllocator_Heap, NMib::NThread::EThreadLocalFlag_AlwaysCreated> g_ThreadLocal;

extern "C"
{
	struct CAlloc
	{
		void *m_pAlloc;
		mint m_Size;
	};
	module_export void calling_convention_c fg_TestMemory()
	{
		*g_ThreadLocal = NMib::NStr::CStr::fs_ToStr(NMib::NSys::fg_Thread_GetCurrentUID());
		
		NMib::NContainer::TCVector<NMib::NPtr::TCUniquePointer<NMib::NThread::CThreadObject>> StartedThreads;
		
		for (int i = 0; i < 16; ++i)
		{
			StartedThreads.f_Insert
				(
					NMib::NThread::CThreadObject::fs_StartThread
					(
						[&](NMib::NThread::CThreadObject * _pThread) -> aint
						{
							NMib::NSys::fg_Thread_Sleep(fp64(0.1) + NMib::NMisc::fg_GetRandomFloat()*0.1);
							
							for (int i = 0; i < 2; ++i)
							{
								NMib::NContainer::TCVector<CAlloc> Allocs;
								NMib::NContainer::TCVector<CAlloc> BigAllocs;
								NMib::NContainer::TCVector<CAlloc> HugeAllocs;
								auto Checkout = NMib::fg_GetSys()->f_MemoryManager_Checkout();
								mint LastAlloc = 0;
								for (mint MemorySize = 1; MemorySize <= 512*1024; ++MemorySize)
								{
									mint AllocSize = NMib::NMem::fg_SizePadded(MemorySize);
									if (AllocSize != LastAlloc || MemorySize < 1024)
									{
										LastAlloc = AllocSize;
										mint Size = MemorySize;
										auto pMemory = NMib::NMem::fg_AllocWithSize(Size);
										Allocs.f_Insert({pMemory, Size});
									}
								}
								
								for (mint MemorySize = 512*1024 * 2; MemorySize <= 16*1024*1024; MemorySize *= 2)
								{
									mint AllocSize = MemorySize;
									if (AllocSize != LastAlloc)
									{
										LastAlloc = AllocSize;
										mint Size = AllocSize;
										auto pAlloc = NMib::NMem::fg_AllocWithSize(Size);
										BigAllocs.f_Insert({pAlloc, Size});
									}
								}
								
								for (mint MemorySize = 16*1024*1024 * 2; MemorySize <= 16*1024*1024 * 4; MemorySize *= 2)
								{
									mint AllocSize = MemorySize;
									if (AllocSize != LastAlloc)
									{
										LastAlloc = AllocSize;
										mint Size = AllocSize;
										auto pAlloc = NMib::NMem::fg_AllocWithSize(Size);
										HugeAllocs.f_Insert({pAlloc, Size});
									}
								}
								
								for (auto &Alloc : Allocs)
									NMib::NMem::fg_Free(Alloc.m_pAlloc, Alloc.m_Size);
								for (auto &Alloc : BigAllocs)
									NMib::NMem::fg_Free(Alloc.m_pAlloc, Alloc.m_Size);
								for (auto &Alloc : HugeAllocs)
									NMib::NMem::fg_Free(Alloc.m_pAlloc, Alloc.m_Size);
								NMib::NSys::fg_Thread_Sleep(fp64(0.005) + NMib::NMisc::fg_GetRandomFloat()*0.005);
							}
							
							return 0;
						}
						, "Test memory manager in DLL"
					)
				)
			;
		}
		// DMibNew int;
	}

}
