// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
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
		void f_ForceStartThread();

	private:

		void fp_StartupThread();

	private:
		TCMemoryManager<t_CParams> * mp_pMemoryManager;
		TCMemoryManagerNumaArena<t_CParams> * mp_pNumaArena;
		NTime::CCyclesClock mp_Clock;
		align_cacheline NAtomic::TCAtomic<uint32> mp_bStarted;
		align_cacheline NAtomic::TCAtomic<uint32> mp_bWaiting;

		NStorage::TCUniquePointer<NThread::CThreadObjectNonTracked, NMemory::CAllocator_NonTrackedHeap> mp_pThread;
	};
}
