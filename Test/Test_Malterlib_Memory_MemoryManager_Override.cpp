// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Test/Test>
#include <Mib/Test/Performance>
#include <Mib/Process/ProcessLaunch>
#include <Mib/Core/SubSystem>

#if defined(DPlatformFamily_macOS) || defined(DPlatformFamily_Linux)
	#include <memory>
	#include <string>
	#include <stdlib.h>
	#include <pthread.h>
	#include <errno.h>
	#include <signal.h>
	#include <string.h>
	#include <unistd.h>
#endif

#ifdef DPlatformFamily_macOS
	#include <malloc/malloc.h>
	#include <dispatch/dispatch.h>
#endif

#if defined(DPlatformFamily_macOS) || defined(DPlatformFamily_Linux)
extern NMib::NAtomic::TCAtomic<bool> g_bSysDeleted;
#endif

namespace
{
	using namespace NMib;
	using namespace NMib::NMemory;
	using namespace NMib::NStr;
	using namespace NMib::NContainer;

#if defined(DPlatformFamily_macOS) || defined(DPlatformFamily_Linux)
	constinit NAtomic::TCAtomic<bool> g_bForeignThreadAllocating{false};
	constinit NAtomic::TCAtomic<umint> g_nForeignThreadsStarted{0};
	constinit NAtomic::TCAtomic<bool> g_bLateThreadAllocated{false};

	constexpr ch8 gc_LateThreadAllocated[] = "LateThreadAllocated\n";
	constexpr ch8 gc_LateThreadNotCreated[] = "LateThreadNotCreated\n";

	void fg_AllocateOnce();
	void fg_AllocateUntilProcessExits(void *);

	// Reports to the supervising process without anything from the destroyed system
	void fg_ReportLate(ch8 const *_pMessage, umint _Length)
	{
		while (_Length)
		{
			auto nWritten = write(STDERR_FILENO, _pMessage, _Length);
			if (nWritten <= 0)
				return;

			_pMessage += nWritten;
			_Length -= umint(nWritten);
		}
	}

	void *fg_AllocateOnNewThread(void *)
	{
		fg_AllocateUntilProcessExits(nullptr);

		return nullptr;
	}

	void *fg_AllocateOnLateThread(void *)
	{
		fg_AllocateOnce();
		fg_ReportLate(gc_LateThreadAllocated, sizeof(gc_LateThreadAllocated) - 1);
		g_bLateThreadAllocated.f_Store(true);

		fg_AllocateUntilProcessExits(nullptr);

		return nullptr;
	}

	// Starts a thread that allocates when the system has been destroyed for a time that is given in microseconds, as
	// the system does for work that arrives then.
	void *fg_AllocateLate(void *_pDelay)
	{
		g_nForeignThreadsStarted.f_FetchAdd(1);

		while (!::g_bSysDeleted.f_Load())
			usleep(10);

		usleep(useconds_t(umint(_pDelay)));

		pthread_t Thread;
		if (pthread_create(&Thread, nullptr, &fg_AllocateOnLateThread, nullptr) == 0)
			pthread_detach(Thread);
		else
			fg_ReportLate(gc_LateThreadNotCreated, sizeof(gc_LateThreadNotCreated) - 1);

		fg_AllocateUntilProcessExits(nullptr);

		return nullptr;
	}

	// The teardown destroys this after g_bSysDeleted is set, so waiting here keeps the process from exiting before a
	// thread created after that has allocated.
	struct CSubSystem_WaitForLateThread : public CSubSystem
	{
		~CSubSystem_WaitForLateThread()
		{
			for (umint i = 0; i < 10000 && !g_bLateThreadAllocated.f_Load(); ++i)
				usleep(1000);
		}
	};

	constinit TCSubSystem<CSubSystem_WaitForLateThread, ESubSystemDestruction_BeforeMemoryManager> g_SubSystem_WaitForLateThread = {DAggregateInit};

	void fg_AllocateOnce()
	{
		void *pMemory = malloc(24);
		pMemory = realloc(pMemory, 4096);
		free(pMemory);

		auto *pObject = new std::string(128, 'a');
		pObject->append(1024, 'b');
		delete pObject;
	}

	// Runs on a thread of the system, as the work that frameworks of the system leave running when the process exits.
	void fg_AllocateUntilProcessExits(void *)
	{
		while (true)
		{
			fg_AllocateOnce();
			g_bForeignThreadAllocating.f_Exchange(true);
		}
	}
#endif

	class COverride_Tests : public CTest
	{
	public:

		void f_DoTests()
		{
#if defined(DPlatformFamily_macOS) || defined(DPlatformFamily_Linux)
			DMibTestSuite("ForeignThreadAfterDestroy")
			{
				if (fg_GetSys()->f_GetEnvironmentVariable("MalterlibForeignThreadAfterDestroyChild", "") == "true")
				{
					constexpr fp64 c_Timeout = 60.0;

					[[maybe_unused]] auto &WaitForLateThread = *g_SubSystem_WaitForLateThread;

					// The threads are still allocating when the process exits, which destroys the memory manager
					for (umint i = 0; i < 4; ++i)
					{
	#ifdef DPlatformFamily_macOS
						dispatch_async_f(dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), nullptr, &fg_AllocateUntilProcessExits);
	#else
						pthread_t Thread;
						if (pthread_create(&Thread, nullptr, &fg_AllocateOnNewThread, nullptr) == 0)
							pthread_detach(Thread);
	#endif
					}

					NTime::CStopwatch Stopwatch(true);
					while (!g_bForeignThreadAllocating.f_Load() && Stopwatch.f_GetTime() < c_Timeout)
						NSys::fg_Thread_Sleep(0.001);

					constexpr umint c_nLateThreads = 64;
					constexpr umint c_DelayStep = 50;

					umint nLateThreads = 0;
					for (umint i = 0; i < c_nLateThreads; ++i)
					{
						pthread_t Thread;
						if (pthread_create(&Thread, nullptr, &fg_AllocateLate, (void *)(i * c_DelayStep)) == 0)
						{
							pthread_detach(Thread);
							++nLateThreads;
						}
					}

					Stopwatch.f_Start();
					while (g_nForeignThreadsStarted.f_Load() < nLateThreads && Stopwatch.f_GetTime() < c_Timeout)
						NSys::fg_Thread_Sleep(0.0001);

					return;
				}

				constexpr fp64 c_Timeout = 120.0;
				constexpr umint c_nChildren = 20;

				struct CChild
				{
					NThread::CMutual m_Lock;
					NThread::CEventAutoReset m_Exited;
					CStr m_Output;
					uint32 m_ExitCode = 0;
					bool m_bLaunched = false;
				};

				umint nFailed = 0;
				umint nTimedOut = 0;
				umint nWithLateThread = 0;
				umint nLateThreadsNotCreated = 0;
				CStr FirstOutput;

				for (umint i = 0; i < c_nChildren; ++i)
				{
					NStorage::TCSharedPointer<CChild> pChild = fg_Construct();

					NProcess::CProcessLaunchParams Params;
					Params.m_Target = NFile::CFile::fs_GetProgramPath();
					Params.m_Parameters = NProcess::CProcessLaunchParams::fs_GetParams({"--test", fg_TestGetCurrentPath(), "--process-recursive"});
					Params.m_Environment["MalterlibForeignThreadAfterDestroyChild"] = "true";
					Params.m_bMergeEnvironment = true;
					Params.m_bSeparateStdErr = true;

					Params.m_fOnStateChange = [pChild](NProcess::CProcessLaunchStateChangeVariant const &_State, fp64)
						{
							switch (_State.f_GetTypeID())
							{
							case NProcess::EProcessLaunchState_Launched:
								{
									DMibLock(pChild->m_Lock);
									pChild->m_bLaunched = true;
								}

								break;
							case NProcess::EProcessLaunchState_LaunchFailed:
								{
									DMibLock(pChild->m_Lock);
									pChild->m_Output += _State.f_Get<NProcess::EProcessLaunchState_LaunchFailed>();
								}

								pChild->m_Exited.f_Signal();

								break;
							case NProcess::EProcessLaunchState_Exited:
								{
									DMibLock(pChild->m_Lock);
									pChild->m_ExitCode = _State.f_Get<NProcess::EProcessLaunchState_Exited>();
								}

								pChild->m_Exited.f_Signal();

								break;
							}
						}
					;

					Params.m_fOnOutput = [pChild](NProcess::EProcessLaunchOutputType, CStr const &_Output)
						{
							DMibLock(pChild->m_Lock);
							pChild->m_Output += _Output;
						}
					;

					bool bTimedOut = false;
					{
						NProcess::CProcessLaunch Launch(Params, NProcess::EProcessLaunchCloseFlag_BlockOnExit);

						// A process that stops while it exits is ended, so that it is reported and not waited for
						bTimedOut = pChild->m_Exited.f_WaitTimeout(c_Timeout);
						if (bTimedOut)
						{
							int KillResult = kill(pid_t(Launch.f_GetProcessID()), SIGKILL);
							if (KillResult != 0)
								DMibConOut("Failed to end the process that timed out: {}\n", CStr(strerror(errno)));
						}
					}

					DMibLock(pChild->m_Lock);

					nTimedOut += bTimedOut;
					nWithLateThread += pChild->m_Output.f_Find(gc_LateThreadAllocated) >= 0;
					nLateThreadsNotCreated += pChild->m_Output.f_Find(gc_LateThreadNotCreated) >= 0;

					if (pChild->m_bLaunched && pChild->m_ExitCode == 0 && !bTimedOut)
						continue;

					if (!nFailed)
						FirstOutput = pChild->m_Output;

					++nFailed;
				}

				if (nFailed)
					DMibConOut("{}\n", FirstOutput);

				DMibExpect(nFailed, ==, 0);
				DMibExpect(nTimedOut, ==, 0);
				DMibExpect(nLateThreadsNotCreated, ==, 0);

				DMibExpect(nWithLateThread, ==, c_nChildren);
			};
#endif

#ifdef DPlatformFamily_macOS
			DMibTestSuite("TrySize")
			{
				void *pMemory = malloc(5);
				auto Cleanup = g_OnScopeExit / [&]
					{
						free(pMemory);
					}
				;
				auto Size = malloc_size(pMemory);
				DMibExpect(Size, >, 0);

				auto Size2 = malloc_size(&pMemory);
				DMibExpect(Size2, ==, 0);

				auto *pInvalidMemory = (void* *)(umint)4096;

				auto SizeInvalidMemory = malloc_size(pInvalidMemory);
				DMibExpect(SizeInvalidMemory, ==, 0);
			};
			DMibTestSuite(CTestCategory("TrySizePerf") << CTestGroup("Performance"))
			{
				auto fDoTest = [](umint _nZones)
					{
						DMibTestPath("{} Zones"_f << _nZones);

						TCVector<malloc_zone_t *> Zones;

						for (umint i = 0; i < _nZones; ++i)
							Zones.f_Insert(malloc_create_zone(0, 0));

						void *pMemory;
						if (_nZones)
							pMemory = malloc_zone_malloc(Zones.f_GetLast(), 5);
						else
							pMemory = malloc(5);

						auto Cleanup = g_OnScopeExit / [&]
							{
								free(pMemory);

								for (auto &pZone : Zones)
									malloc_destroy_zone(pZone);
							}
						;


						{
							constexpr umint c_nTests = 5;
							constexpr umint c_nSizeTests = 10000;

							CTestPerformanceMeasure MallocSizeTime("malloc_size");
							for (umint i = 0; i < c_nTests; ++i)
							{
								DMibTestScopeMeasure(MallocSizeTime, c_nSizeTests);
								for (umint i = 0; i < c_nSizeTests; ++i)
								{
									auto *pInvalidMemory = (void* *)(umint)4096;
									[[maybe_unused]] auto SizeInvalidMemory = malloc_size(pInvalidMemory);
								}
							}

							CTestPerformance SadCase(1.0);
							SadCase.f_Add(MallocSizeTime);

							DMibTest(DMibExpr(SadCase));
						}
						{
							constexpr umint c_nTests = 5;
							constexpr umint c_nSizeTests = 10000;

							CTestPerformanceMeasure MallocSizeTime("malloc_size");

							for (umint i = 0; i < c_nTests; ++i)
							{
								DMibTestScopeMeasure(MallocSizeTime, c_nSizeTests);
								for (umint i = 0; i < c_nSizeTests; ++i)
								{
									[[maybe_unused]] auto SizeInvalidMemory = malloc_size(pMemory);
								}
							}

							CTestPerformance HappyCase(1.0);
							HappyCase.f_Add(MallocSizeTime);

							DMibTest(DMibExpr(HappyCase));
						}
					}
				;
				fDoTest(0);
				fDoTest(10);
				fDoTest(100);
#ifndef DMibDebug
				fDoTest(200);
#endif
			};
#endif
		}
	};

	DMibTestRegister(COverride_Tests, Malterlib::Memory::MemoryManager);
}
