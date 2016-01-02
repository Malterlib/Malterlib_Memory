// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{

		template <typename t_CParams>
		struct TCMemoryManagerNumaArenaBackgroundCleanup
		{
			TCMemoryManagerNumaArenaBackgroundCleanup(TCMemoryManagerNumaArena<t_CParams> * _pNumaArena);
			~TCMemoryManagerNumaArenaBackgroundCleanup();
			
			void f_OnNeedCleanup();

			int64 f_GetTimestamp() const;
			
			bool f_IsWaiting();

			void f_StopThread();
			void f_CanStartThreads();
			
			void f_PrepareFork();
			void f_ForkedChild();
			void f_ForkedParent();

		private:

			void fp_StartupThread();
			
		private:
			TCMemoryManager<t_CParams> * mp_pMemoryManager;
			TCMemoryManagerNumaArena<t_CParams> * mp_pNumaArena;
			NTime::CCyclesClock mp_Clock;
			align_cacheline NAtomic::TCAtomic<uint32> mp_bStarted;
			align_cacheline NAtomic::TCAtomic<uint32> mp_bWaiting;
			
			NPtr::TCUniquePointer<NThread::CThreadObjectNonTracked, NMem::CAllocator_NonTrackedHeap> mp_pThread;
		};
	}
}
