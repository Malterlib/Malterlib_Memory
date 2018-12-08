// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_OSX)
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
		mp_Clock.f_Start();
	}

	template <typename t_CParams>
	TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::~TCMemoryManagerNumaArenaBackgroundCleanup()
	{
	}

	template <typename t_CParams>
	int64 TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_GetTimestamp() const
	{
		return mp_Clock.f_GetCycles();
	}


	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_OnNeedCleanup()
	{
		if (!mp_bStarted.f_Load(NAtomic::EMemoryOrder_Relaxed))
		{
			if (!g_bCanStartThreads) // Cannot stat thread
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
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_OSX)
		auto &ForkLock = NMib::NPlatform::fg_ForkLock();
		DMibLock(ForkLock);
#endif
		f_OnNeedCleanup();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::fp_StartupThread()
	{
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_OSX)
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
					bool bForceFullGarbageCollection = false;
					int64 MSLifetime = t_CParams::mc_BackgroundCleanupLifetime;
					fp32 WaitTime = fp32(MSLifetime) / fp32(1000.0f);
					int64 LifeTime = MSLifetime * NTime::CSystem_Time::fs_CyclesFrequency() / 1000;
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
					bool bNeedUpdate = true;
					while (_pThread->f_GetState() != NThread::EThreadState_EventWantQuit)
					{
						int64 Time = mp_Clock.f_GetCycles();

						if (Time >= NextCleanup)
						{
		//					DMibTraceSafe("{}\n", mp_Clock.f_GetTime());

							int64 RemoveTime = mp_Clock.f_GetCycles() - LifeTime;
							int64 NextUpdate = mp_pNumaArena->f_GarbageCollect(RemoveTime, true, false, bForceFullGarbageCollection);
							bForceFullGarbageCollection = false;

							bNeedUpdate = NextUpdate != TCLimitsInt<int64>::mc_Max;
							NextCleanup = Time + LifeTime;
						}

						if (bNeedUpdate)
							_pThread->m_EventWantQuit.f_WaitTimeout(WaitTime);
						else
						{
							mp_bWaiting.f_Exchange(1);
							_pThread->m_EventWantQuit.f_Wait();
						}

					}
#ifdef DMibMemory_CleanupOnUSR1Signal
					pSubscription.f_Clear();
#endif
					return 0;

				}
				, "Memory manager cleanup"
			)
		;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_CanStartThreads()
	{
		// Force initialization of time context on main thread as it uses environment which is not thread safe. In future try to remove dependency on time context here
		NTime::CSystem_Time::fs_CyclesFrequency();

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
		mp_bStarted = false;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::f_ForkedParent()
	{
		if (mp_pThread)
			mp_pThread->f_ForkedParent();

#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_OSX)
		if (mp_bStarted && !mp_pThread)
			fp_StartupThread();
#endif
	}
}
