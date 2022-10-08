// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_macOS)
namespace NMib::NPlatform
{
	NThread::CMutual &fg_ForkLock();
}
#endif

namespace NMib::NMemory
{
	template <typename t_CParams>
	TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::TCMemoryManagerNumaArenaBackgroundCleanup(TCMemoryManagerNumaArena<t_CParams> * _pNumaArena)
		: mp_pNumaArena(_pNumaArena)
		, mp_bStarted(false)
		, mp_pMemoryManager(_pNumaArena->m_pMemoryManager)
	{
		if (g_bCanStartThreads.f_Load())
			f_CanStartThreads();
	}

	template <typename t_CParams>
	TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::~TCMemoryManagerNumaArenaBackgroundCleanup()
	{
	}

	template <typename t_CParams>
	int64 TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_GetTimestamp() const
	{
		auto Return = mp_Clock.f_GetCycles();

		DMibFastCheck(Return != 0);;

		return Return;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_OnNeedCleanup()
	{
		if (!mp_bStarted.f_Load(NAtomic::EMemoryOrder_Relaxed))
		{
			if (!g_bCanStartThreads.f_Load(NAtomic::EMemoryOrder_Relaxed)) // Cannot stat thread
			{
				mp_bStarted.f_FetchOr(3); // Signal
				return;
			}
			else
			{
				if (!mp_bStarted.f_FetchOr(1))
					fp_StartupThread();
			}
		}

		if (mp_bWaiting.f_Exchange(0))
		{
			if (mp_pThread)
				mp_pThread->m_EventWantQuit.f_Signal();
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_ForceStartThread()
	{
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_macOS)
		auto &ForkLock = NMib::NPlatform::fg_ForkLock();
		DMibLock(ForkLock);
#endif
		f_OnNeedCleanup();

		mp_FirstGarbageCollected.f_Wait();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::fp_StartupThread()
	{
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_macOS)
		auto &ForkLock = NMib::NPlatform::fg_ForkLock();
		if (!ForkLock.f_TryLock())
			return;
		DMibLock(ForkLock);
		ForkLock.f_Unlock();
		if (mp_pThread)
			return;
#endif
		mp_pThread = NThread::CThreadObjectNonTracked::fs_StartThread
			(
				[this](NThread::CThreadObjectNonTracked *_pThread) -> aint
				{
					if (mp_pNumaArena->m_NumaNode != ENumaNode_Default)
					{
						NSys::fg_Thread_SetNumaAffinity(NMib::NSys::fg_Thread_GetCurrent(), mp_pNumaArena->m_NumaNode);
						mp_pMemoryManager->f_SetNumaNode(mp_pNumaArena->m_NumaNode);
					}
					
					static_assert(t_CParams::mc_BackgroundCleanupLifetimeDecommit >= t_CParams::mc_BackgroundCleanupLifetime);

					bool bForceFullGarbageCollection = false;
					int64 LifeTime;
					fp32 WaitTime;
					{
						int64 MSLifetime = t_CParams::mc_BackgroundCleanupLifetime;
						WaitTime = fp32(MSLifetime) / fp32(1000.0f);
						LifeTime = MSLifetime * NTime::CSystem_Time::fs_CyclesFrequency() / 1000;
					}
					int64 LifeTimeDecommit;
					{
						int64 MSLifetime = t_CParams::mc_BackgroundCleanupLifetimeDecommit;
						LifeTimeDecommit = MSLifetime * NTime::CSystem_Time::fs_CyclesFrequency() / 1000;
					}
					int64 NextCleanup = mp_Clock.f_GetCycles() + LifeTime;

#ifdef DMibMemory_CleanupOnUSR1Signal
					auto pSubscription = NSys::fg_System_RegisterForSignal
						(
							30
							, [&]()
							{
								bForceFullGarbageCollection = true;
								NextCleanup = 0;
								_pThread->m_EventWantQuit.f_Signal();
							}
						)
					;
#endif
					bool bFirstWait = true;
					bool bNeedUpdate = true;
					while (_pThread->f_GetState() != NThread::EThreadState_EventWantQuit)
					{
						int64 Time = mp_Clock.f_GetCycles();

						if (Time >= NextCleanup)
						{
		//					DMibTraceSafe("{}\n", mp_Clock.f_GetTime());

							int64 Now = mp_Clock.f_GetCycles();
							int64 RemoveTime = Now - LifeTime;
							int64 RemoveTimeDecommit = Now - LifeTimeDecommit;
							[[maybe_unused]] auto &ThreadLocal = *mp_pNumaArena->m_pMemoryManager->m_LocalArena; // Precach thread local to prevent deadlock

							DMibLock(mp_GarbageCollectLock);

							mp_pNumaArena->m_pMemoryManager->f_LazyReturnCheckout();
							
							int64 NextUpdate = mp_pNumaArena->f_GarbageCollect({RemoveTime, RemoveTimeDecommit}, true, bForceFullGarbageCollection);
							bForceFullGarbageCollection = false;

							bNeedUpdate = NextUpdate != TCLimitsInt<int64>::mc_Max;
							NextCleanup = Time + LifeTime;
						}

						if (bNeedUpdate)
							_pThread->m_EventWantQuit.f_WaitTimeout(WaitTime);
						else
						{
							mp_bWaiting.f_Exchange(1);
							if (bFirstWait)
							{
								bFirstWait = false;
								mp_FirstGarbageCollected.f_SetSignaled();
							}
							_pThread->m_EventWantQuit.f_Wait();
						}

					}
#ifdef DMibMemory_CleanupOnUSR1Signal
					pSubscription.f_Clear();
#endif
					return 0;

				}
				, "Memory manager cleanup"
				, EExecutionPriority_BelowNormal
			)
		;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_CanStartThreads()
	{
		// Force initialization of time context on main thread as it uses environment which is not thread safe. In future try to remove dependency on time context here
		NTime::CSystem_Time::fs_CyclesFrequency();

		mp_Clock.f_Start(NTime::CSystem_Time::fs_CyclesFrequency() * 100);

		if (mp_bStarted.f_FetchAnd(~2) & 2)
		{
			// Start was scheduled
			fp_StartupThread();
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_StopThread()
	{
		if (mp_pThread)
		{
			mp_pThread->f_Stop();
			mp_pThread.f_Clear();
		}
	}

	template <typename t_CParams>
	bool TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_IsWaiting()
	{
		return mp_bWaiting.f_Load() != 0;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_Lock()
	{
		mp_GarbageCollectLock.f_Lock();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_Unlock()
	{
		mp_GarbageCollectLock.f_Unlock();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_PrepareFork()
	{
		if (mp_pThread)
			mp_pThread->f_PrepareFork();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_ForkedChild()
	{
		mp_bWaiting.f_Exchange(0);
		if (mp_pThread)
		{
			mp_pThread->f_ForkedChild();
			mp_pThread.f_Clear();
		}
		mp_GarbageCollectLock.f_ForkedChildLocked();
		mp_bStarted = false;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_ForkedParent()
	{
		if (mp_pThread)
			mp_pThread->f_ForkedParent();

#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_macOS)
		if (mp_bStarted && !mp_pThread)
			fp_StartupThread();
#endif
	}
}
