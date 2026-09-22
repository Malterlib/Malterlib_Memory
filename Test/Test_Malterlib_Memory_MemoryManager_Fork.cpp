// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Test/Test>
#include <Mib/Memory/Memory>
#include <Mib/Thread/ThreadObject>
#include <Mib/Atomic/Atomic>
#include <Mib/Container/Vector>

#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_macOS)
	#include <signal.h>
	#include <sys/wait.h>
	#include <unistd.h>
#endif

namespace
{
	using namespace NMib;
	using namespace NMib::NTest;
	using namespace NMib::NThread;

	struct CFork_Tests : CTest
	{
		void f_DoTests()
		{
#if defined(DPlatformFamily_Linux) || defined(DPlatformFamily_macOS)
			// Blocks are freed after the last allocation, and pacing keeps heap chunks being created for most of the cycle
			auto fAllocate = [](umint _Size, umint _nBlocks, fp32 _Pace) -> bool
				{
					NContainer::TCVector<uint8 *> Blocks;
					Blocks.f_SetLen(_nBlocks);
					bool bSuccess = true;
					for (auto &pBlock : Blocks)
					{
						pBlock = (uint8 *)NMemory::fg_Alloc(_Size);
						if (!pBlock)
						{
							bSuccess = false;
							continue;
						}

						pBlock[0] = 1;
						pBlock[_Size - 1] = 1;
						if (_Pace > 0.0f)
							NSys::fg_Thread_Sleep(_Pace);
					}

					for (auto *pBlock : Blocks)
					{
						if (pBlock)
							NMemory::fg_Free(pBlock, _Size);
					}

					return bSuccess;
				}
			;

			auto fWaitForChild = [](pid_t _ProcessID) -> NStr::CStr
				{
					int Status = 0;
					pid_t Waited = 0;
					for (umint iPoll = 0; iPoll < 1000 && Waited == 0; ++iPoll)
					{
						Waited = waitpid(_ProcessID, &Status, WNOHANG);
						if (Waited == 0)
							NSys::fg_Thread_Sleep(0.01f);
					}

					if (Waited < 0)
						return NStr::CStr::CFormat("waitpid: {}\n") << strerror(errno);

					if (Waited == 0)
					{
						kill(_ProcessID, SIGKILL);
						waitpid(_ProcessID, &Status, 0);
						return NStr::CStr::CFormat("child {} hangs\n") << _ProcessID;
					}

					if (WIFSIGNALED(Status))
						return NStr::CStr::CFormat("child {} killed by signal {}\n") << _ProcessID << WTERMSIG(Status);

					if (WEXITSTATUS(Status) != 0)
						return NStr::CStr::CFormat("child {} exited with {}\n") << _ProcessID << WEXITSTATUS(Status);

					return {};
				}
			;

			// Another thread is inside the memory manager while the parent forks, and the child allocates on its main thread and on a new thread
			auto fForkDuringAllocations = [&](umint _Size, umint _nBlocks, fp32 _Pace)
				{
					NAtomic::TCAtomic<bool> bStop{false};
					auto pAllocatorThread = CThreadObject::fs_StartThread
						(
							[&](CThreadObject *) -> aint
							{
								while (!bStop.f_Load())
									fAllocate(_Size, _nBlocks, _Pace);

								return 0;
							}
							, "Allocator"
						)
					;

					NStr::CStr Failures;
					for (umint iFork = 0; iFork < 50; ++iFork)
					{
						pid_t ProcessID = fork();
						if (ProcessID < 0)
						{
							Failures += NStr::CStr::CFormat("fork: {}\n") << strerror(errno);
							continue;
						}

						if (!ProcessID)
						{
							alarm(60);

							// The new thread inherits the preferred arena, and only a busy preferred arena makes it pick another
							auto pThread = CThreadObject::fs_StartThread
								(
									[&](CThreadObject *) -> aint
									{
										return fAllocate(_Size, _nBlocks, 0.0f) ? 0 : 1;
									}
									, "Child allocator"
								)
							;
							bool bSuccess = fAllocate(_Size, _nBlocks, 0.0f);
							if (pThread->f_Stop() != 0)
								bSuccess = false;

							_exit(bSuccess ? 0 : 1);
						}

						Failures += fWaitForChild(ProcessID);
					}

					bStop = true;
					pAllocatorThread->f_Stop();

					DMibExpect(Failures, ==, "");
				}
			;

			DMibTestSuite("DuringAllocations")
			{
				DMibTestCategory("Heap")
				{
					fForkDuringAllocations(16 * 1024 * 1024, 24, 0.0025f); // Every block needs a heap chunk of its own
				};

				DMibTestCategory("Slabs")
				{
					fForkDuringAllocations(64, 4096, 0.0f);
				};
			};
#endif
		}
	};

	DMibTestRegister(CFork_Tests, Malterlib::Memory::MemoryManager);
}
