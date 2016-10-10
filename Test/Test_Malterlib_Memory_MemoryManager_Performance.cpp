// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include <Mib/Test/Memory>
#include <Mib/Test/Performance>
#include <Mib/Process/ProcessLaunch>
#include "Test_Malterlib_Memory_MemoryManager_Performance.h"

namespace
{
	using namespace NMib::NTest;
	using namespace NMib::NMem;
	
#if 0
	struct CDisplayStats
	{
		CDisplayStats()
		{
			// Stats
			mint SlabSize = CDefaultMemoryManagerParams::mc_SlabSize;
			DMibConOut("CDefaultMemoryManagerParams::mc_SlabSize = {}\r\n", SlabSize);
			
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 0>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 0>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 0>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 1>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 1>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 1>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 2>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 2>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 2>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 3>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 3>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 3>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 4>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 4>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 4>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 5>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 5>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 5>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 6>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 6>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 6>), 4096));
			DMibConOut("sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 7>) = {} {}\r\n", sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 7>) << NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 7>), 4096));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 0>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 0>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 1>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 1>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 2>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 2>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 3>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 3>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 4>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 4>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 5>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 5>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 6>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 6>::mc_SubSlabs));
			DMibConOut("TCMemoryManagerSlab<CDefaultMemoryManagerParams, 7>::mc_SubSlabs = {}\r\n", mint(TCMemoryManagerSlab<CDefaultMemoryManagerParams, 7>::mc_SubSlabs));
			DMibConOut("sizeof(CMemoryManagerSubSlabData) = {}\r\n", sizeof(CMemoryManagerSubSlabData));			
			DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,1>) = {}\r\n", sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,1>));
			DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,2>) = {}\r\n", sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,2>));
			DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,4>) = {}\r\n", sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,4>));
			DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,8>) = {}\r\n", sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,8>));
			DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,12>) = {}\r\n", sizeof(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,12>));
			DMibConOut("TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,1>::mc_NumAllocs = {}\r\n", mint(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,1>::mc_NumAllocs));
			DMibConOut("TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,2>::mc_NumAllocs = {}\r\n", mint(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,2>::mc_NumAllocs));
			DMibConOut("TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,4>::mc_NumAllocs = {}\r\n", mint(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,4>::mc_NumAllocs));
			DMibConOut("TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,8>::mc_NumAllocs = {}\r\n", mint(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,8>::mc_NumAllocs));
			DMibConOut("TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,12>::mc_NumAllocs = {}\r\n",mint(TCMemoryManagerSubSlab_SmallSize<CDefaultMemoryManagerParams,12>::mc_NumAllocs));
			DMibConOut("sizeof(TCMemoryManagerArena<CDefaultMemoryManagerParams>) = {}\r\n", sizeof(TCMemoryManagerArena<CDefaultMemoryManagerParams>));
			DMibConOut("sizeof(TCMemoryManagerNumaArena<CDefaultMemoryManagerParams>) = {}\r\n", sizeof(TCMemoryManagerNumaArena<CDefaultMemoryManagerParams>));
			DMibConOut("sizeof(TCMemoryManager<CDefaultMemoryManagerParams>) = {}\r\n", sizeof(TCMemoryManager<CDefaultMemoryManagerParams>));
			DMibConOut("sizeof(TCMemoryManagerArenaHeap<CDefaultMemoryManagerParams>) = {}\r\n", sizeof(TCMemoryManagerArenaHeap<CDefaultMemoryManagerParams>));
			DMibConOut("sizeof(TCMemoryManagerArenaHeapChunk<CDefaultMemoryManagerParams>) = {}\r\n", sizeof(TCMemoryManagerArenaHeapChunk<CDefaultMemoryManagerParams>));
			
		}
	};
	
	CDisplayStats g_DisplayStats;

#endif
	
	class CPerformance_Tests : public CTest
	{
	public:

		mint m_nCores;
		CPerformance_Tests()
		{
			m_nCores = NMib::NSys::fg_Thread_GetVirtualCores();
			
			
		}

		struct CAllocPattern_Random
		{
			struct CAllocationInfo
			{
//				DMibListLinkD_Link(CAllocationInfo, m_Link);
				CAllocationInfo()
					: m_pAddress(nullptr)
				{
				}
				void *m_pAddress;
			};
			
			NMib::NMisc::CRandomShiftRNG m_Random;
			uint64 m_nIterations;
			CAllocationInfo *m_pAllocations;
			mint m_ArraySize;
			mint m_MaxAllocatedMemory;
			mint m_MaxAllocSize;
			zbint m_bFailed;

//			typedef DMibListLinkD_List(CAllocationInfo, m_Link) CAllocInfoList;
	//		NMib::NMem::TCPool<CAllocationInfo, 1024*1024*4, NMib::NThread::CNoLock, NMib::NMem::CPoolType_Growing> m_AllocationInfoPool;


			CAllocPattern_Random()
				: m_pAllocations(nullptr)
			{
			}
			CAllocPattern_Random(mint _MaxAllocSize, mint _iThread, NMib::ENumaNode _NumaNode)
				: m_Random(55556 + _iThread)
				, m_pAllocations(nullptr)
				, m_MaxAllocSize(_MaxAllocSize - 1)
				, m_nIterations(0)
				, m_MaxAllocatedMemory(256*1024*1024)
				, m_ArraySize(0)
			{
				m_ArraySize = NMib::fg_Min(((m_MaxAllocatedMemory * 2) / _MaxAllocSize), (16u * 1024u)) - 1;

				mint Size = sizeof(CAllocationInfo) * (m_ArraySize + 1);
				m_pAllocations = (CAllocationInfo *)NMib::NSys::fg_Mem_VirtualAlloc(Size, NMib::EAllocationFlag_None, _NumaNode);
				for (mint i = 0; i <= m_ArraySize; ++i)
					new (m_pAllocations + i) CAllocationInfo();
			}

			static bool fs_Alignment()
			{
				return false;
			}
			
			mint f_GetIdealAllocations()
			{
				return m_ArraySize * 256;
			}

			~CAllocPattern_Random()
			{
				if (m_pAllocations)
					NMib::NSys::fg_Mem_VirtualFree(m_pAllocations, 0);
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_Random.f_GetValue<uint32>() & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						++m_nIterations;
					}

					{
						mint Size = (m_Random.f_GetValue<uint32>() & m_MaxAllocSize) + 1;
						//mint Size = m_MaxAllocSize + 1;
						Info.m_pAddress = _Heap.f_Alloc(Size);
						//NMib::NMem::fg_MemClear((uint8 *)Info.m_pAddress, Size);
						++m_nIterations;
					}
				}
			}

			template <typename tf_CHeap>
			uint64 f_Iterations(tf_CHeap &_Heap)
			{
				auto End = m_pAllocations + m_ArraySize + 1;
				for (auto Iter = m_pAllocations; Iter != End; ++Iter)
				{
					auto &Info = *Iter;
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						Info.m_pAddress = nullptr;
						++m_nIterations;
					}
				}
				uint64 Ret = m_nIterations;
				m_nIterations = 0;
				return Ret;
			}
		};

		struct CAllocPattern_RandomAlignment
		{
			struct CAllocationInfo
			{
//				DMibListLinkD_Link(CAllocationInfo, m_Link);
				CAllocationInfo()
					: m_pAddress(nullptr)
				{
				}
				void *m_pAddress;
			};

			NMib::NMisc::CRandomShiftRNG m_Random;
			uint64 m_nIterations;
			CAllocationInfo *m_pAllocations;
			mint m_ArraySize;
			mint m_MaxAllocatedMemory;
			mint m_MaxAllocSize;
			mint m_AlignBits;
			zbint m_bFailed;

//			typedef DMibListLinkD_List(CAllocationInfo, m_Link) CAllocInfoList;
	//		NMib::NMem::TCPool<CAllocationInfo, 1024*1024*4, NMib::NThread::CNoLock, NMib::NMem::CPoolType_Growing> m_AllocationInfoPool;


			CAllocPattern_RandomAlignment()
				: m_pAllocations(nullptr)
			{
			}
			CAllocPattern_RandomAlignment(mint _MaxAllocSize, mint _iThread, NMib::ENumaNode _NumaNode)
				: m_Random(55556 + _iThread)
				, m_pAllocations(nullptr)
				, m_MaxAllocSize(_MaxAllocSize - 1)
				, m_nIterations(0)
				, m_MaxAllocatedMemory(256*1024*1024)
				, m_ArraySize(0)
				, m_AlignBits(NMib::fg_GetHighestBitSet(_MaxAllocSize))
			{
				if (_MaxAllocSize == 0)
					m_AlignBits = 0;
				if (m_AlignBits == 0)
					m_AlignBits = 1;
				m_ArraySize = NMib::fg_Min(((m_MaxAllocatedMemory * 2) / _MaxAllocSize), (16u * 1024u)) - 1;

				mint Size = sizeof(CAllocationInfo) * (m_ArraySize + 1);
				m_pAllocations = (CAllocationInfo *)NMib::NSys::fg_Mem_VirtualAlloc(Size, NMib::EAllocationFlag_None, _NumaNode);
				for (mint i = 0; i <= m_ArraySize; ++i)
					new (m_pAllocations + i) CAllocationInfo();
			}

			static bool fs_Alignment()
			{
				return true;
			}			
			
			mint f_GetIdealAllocations()
			{
				return m_ArraySize * 256;
			}

			~CAllocPattern_RandomAlignment()
			{
				if (m_pAllocations)
					NMib::NSys::fg_Mem_VirtualFree(m_pAllocations, 0);
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_Random.f_GetValue<uint32>() & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						++m_nIterations;
					}

					{
						mint Size = (m_Random.f_GetValue<uint32>() & m_MaxAllocSize) + 1;
						mint AlignmentBits = m_Random.f_GetValue<uint32>() % m_AlignBits;
						//mint Size = m_MaxAllocSize + 1;
						mint Alignment = mint(1) << AlignmentBits;
						Info.m_pAddress = _Heap.f_AllocAligned(Size, Alignment);
						if ((mint)Info.m_pAddress & (Alignment - 1))
						{
							m_bFailed = true;
						}

						++m_nIterations;
					}
				}
			}

			template <typename tf_CHeap>
			uint64 f_Iterations(tf_CHeap &_Heap)
			{
				auto End = m_pAllocations + m_ArraySize + 1;
				for (auto Iter = m_pAllocations; Iter != End; ++Iter)
				{
					auto &Info = *Iter;
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						Info.m_pAddress = nullptr;
						++m_nIterations;
					}
				}
				uint64 Ret = m_nIterations;
				m_nIterations = 0;
				return Ret;
			}
			
		};

		struct CAllocPattern_OneSize
		{
			struct CAllocationInfo
			{
				CAllocationInfo()
					: m_pAddress(nullptr)
				{
				}
				void *m_pAddress;
			};
			
			NMib::NMisc::CRandomShiftRNG m_Random;
			uint64 m_nIterations;
			CAllocationInfo *m_pAllocations;
			mint m_ArraySize;
			mint m_MaxAllocatedMemory;
			mint m_MaxAllocSize;
			zbint m_bFailed;

			CAllocPattern_OneSize()
				: m_pAllocations(nullptr)
			{
			}
			CAllocPattern_OneSize(mint _MaxAllocSize, mint _iThread, NMib::ENumaNode _NumaNode)
				: m_Random(55556 + _iThread)
				, m_pAllocations(nullptr)
				, m_MaxAllocSize(_MaxAllocSize)
				, m_nIterations(0)
				, m_MaxAllocatedMemory(256*1024*1024)
				, m_ArraySize(0)
			{
				m_ArraySize = NMib::fg_Min((m_MaxAllocatedMemory / _MaxAllocSize), (16u * 1024u)) - 1;

				mint Size = sizeof(CAllocationInfo) * (m_ArraySize + 1);
				m_pAllocations = (CAllocationInfo *)NMib::NSys::fg_Mem_VirtualAlloc(Size, NMib::EAllocationFlag_None, _NumaNode);
				for (mint i = 0; i <= m_ArraySize; ++i)
					new (m_pAllocations + i) CAllocationInfo();
			}

			static bool fs_Alignment()
			{
				return false;
			}

			mint f_GetIdealAllocations()
			{
				return m_ArraySize * 256;
			}
			
			~CAllocPattern_OneSize()
			{
				if (m_pAllocations)
					NMib::NSys::fg_Mem_VirtualFree(m_pAllocations, 0);
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_Random.f_GetValue<uint32>() & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						++m_nIterations;
					}

					{
						mint Size = m_MaxAllocSize;
						Info.m_pAddress = _Heap.f_Alloc(Size);
						++m_nIterations;
					}
				}
			}

			template <typename tf_CHeap>
			uint64 f_Iterations(tf_CHeap &_Heap)
			{
				auto End = m_pAllocations + m_ArraySize + 1;
				for (auto Iter = m_pAllocations; Iter != End; ++Iter)
				{
					auto &Info = *Iter;
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						Info.m_pAddress = nullptr;
						++m_nIterations;
					}
				}
				uint64 Ret = m_nIterations;
				m_nIterations = 0;
				return Ret;
			}
		};

		struct CAllocPattern_OneSizeLinear : public CAllocPattern_OneSize
		{
			CAllocPattern_OneSizeLinear()
			{
			}

			CAllocPattern_OneSizeLinear(mint _MaxAllocSize, mint _iThread, NMib::ENumaNode _NumaNode)
				: CAllocPattern_OneSize(_MaxAllocSize, _iThread, _NumaNode)
			{
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_nIterations & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_Free(Info.m_pAddress);
						++m_nIterations;
					}

					{
						mint Size = m_MaxAllocSize;
						Info.m_pAddress = _Heap.f_Alloc(Size);
						++m_nIterations;
					}
				}
			}
		};

		template <typename t_CHeap, typename t_CAllocPattern>
		struct TCThreadTest : public NMib::NThread::CThread
		{
			TCThreadTest()
				: m_Measure("Baseline")
				, m_pWakeEvent(nullptr)
				, m_iThread(0)
				, m_pStop(nullptr)
				, m_MaxAllocSize(0)
				, m_pHeap(nullptr)
				, m_iNumaNode(NMib::ENumaNode_Default)
			{
				m_StartedEvent.f_ResetSignaled();
			}
			t_CHeap *m_pHeap;
			NMib::NThread::CEvent *m_pWakeEvent;
			CTestPerformanceMeasure m_Measure;
			NMib::NThread::CEvent m_StartedEvent;
			bint volatile *m_pStop;
			mint m_iThread;
			mint m_MaxAllocSize;
			NMib::ENumaNode m_iNumaNode;
			virtual aint f_Main()
			{
				if (m_iNumaNode != NMib::ENumaNode_Default)
				{
					NMib::NSys::fg_Thread_SetNumaAffinity(NMib::NSys::fg_Thread_GetCurrent(), m_iNumaNode);
					m_pHeap->f_SetNumaNode(m_iNumaNode);
				}
				// Init heap
				m_pHeap->f_InitThread();
				m_pHeap->f_Free(m_pHeap->f_Alloc(1));

				m_StartedEvent.f_SetSignaled();
				m_pWakeEvent->f_Wait();

				auto Checkout = m_pHeap->f_Checkout();
				(void)Checkout;
				
				auto fInnerLoop = [pHeapPointer = m_pHeap, this]() inline_never 
					{
						auto pHeap = pHeapPointer; 
						t_CAllocPattern Pattern(m_MaxAllocSize, m_iThread, m_iNumaNode);
						int nPattern = Pattern.f_GetIdealAllocations() / 32;
						uint64 nIterations = 0;
						{
							DMibTestScopeMeasure(m_Measure, nIterations);
							for (int i = 0; i < nPattern; ++i)
							{
								for (mint i = 0; i < 32; ++i)
								{
									Pattern.f_Next(*pHeap);
									//Pattern.f_Next(*m_pHeap);
									//Pattern.f_Next(*m_pHeap);
									//Pattern.f_Next(*m_pHeap);
								}
								//if (*m_pStop)
								//	break;
							}
							nIterations = Pattern.f_Iterations(*pHeap);
						}

						if (Pattern.m_bFailed)
							DMibTest(!DMibExpr(Pattern.m_bFailed))(ETestFlag_Aggregated);
					}
				;
				
				fInnerLoop();

				return 0;
			}

			virtual NMib::NStr::CStr f_GetThreadName()
			{
				return NMib::NStr::CStr::CFormat("Memory manager test {}") << m_iThread;
			}
		};
		
		template <typename tf_CHeap, typename tf_CAllocPattern>
		void f_DoTestPerform(CTestPerformance &_PerfTest, ETestMeasureType _MeasureType, ch8 const *_pName, mint _MaxAllocSize, mint _nThreads)
		{
			mint nTests = 5;

			tf_CHeap Heap;

			if (!Heap.f_Init(_nThreads, _MaxAllocSize))
				return;

			CTestPerformanceMeasure Measure(_pName);

			NMib::NThread::CEvent WakeEvent;
			bint volatile bStop = false;

			mint nNodes = NMib::NSys::fg_Mem_GetNumNumaNodes();
			NMib::NContainer::TCVector<NMib::ENumaNode> Nodes;
			if (nNodes > 0)
			{
				Nodes.f_SetLen(nNodes);
				NMib::NSys::fg_Mem_GetNumaNodes(Nodes.f_GetArray(), nNodes);
			}


			for (mint i = 0; i < nTests; ++i)
			{
				// Reset started
				WakeEvent.f_ResetSignaled();
				bStop = false;
				NMib::NContainer::TCLinkedList<TCThreadTest<tf_CHeap, tf_CAllocPattern>> Tests;
				// Start threads
				int32 iNode = 0;
				for (mint i = 0; i < _nThreads; ++i)
				{
					auto &Test = Tests.f_Insert();
					Test.m_pWakeEvent = &WakeEvent;
					Test.m_iThread = i;
					Test.m_pStop = &bStop;
					Test.m_MaxAllocSize = _MaxAllocSize;
					Test.m_pHeap = &Heap;
					if (nNodes)
					{
						Test.m_iNumaNode = Nodes[iNode];
						Heap.f_SetNumaNode(Test.m_iNumaNode);
						iNode = (iNode + 1) % nNodes;
					}
					Test.f_Start(NMib::EThreadPriority_Normal, 0, 0, false, true);
				}

				// Wait for all threads to start
				for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
					Iter->m_StartedEvent.f_Wait();

				// Start all threads at the same time
				WakeEvent.f_SetSignaled();
				if (_nThreads > m_nCores)
					Measure.f_Start();
				// Let run for allotted time
				{
					NMib::NTime::CClock Clock;
					Clock.f_Start();
					{
						fp64 Runtime;
						if (_nThreads < m_nCores)
							Runtime = 0.4;
						else if (_nThreads == m_nCores)
							Runtime = 0.5;
						else
							Runtime = 1.0;
						/*else
							Runtime = 2.0 * (_nThreads / m_nCores);*/
						/*
						Runtime = 5.0;
						fp64 TimeLeft = Runtime - Clock.f_GetTime();
						while (TimeLeft > 0.0)
						{
							NMib::NSys::fg_Thread_Sleep(TimeLeft);
							TimeLeft = Runtime - Clock.f_GetTime();
						}*/
					}
					// Stop all threads
					bStop = true;
				}

				// Wait for all threads to stop
				for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
					Iter->f_Stop();

				if (_nThreads > m_nCores)
				{
					uint64 nIterations = 0;
					for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
						nIterations += Iter->m_Measure.f_Iterations();

					Measure.f_Stop(nIterations, _nThreads);
				}
				else
				{
					// Add measures
					for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
						Measure.f_AddThreadResult(Iter->m_Measure);

					Measure.f_NewRepetition();
				}
			}

			if (_MeasureType == ETestMeasureType_Reference)
				_PerfTest.f_AddReference(Measure);
			else if (_MeasureType == ETestMeasureType_Baseline)
				_PerfTest.f_AddBaseline(Measure); 
			else if (_MeasureType == ETestMeasureType_Debug)
				_PerfTest.f_AddDebug(Measure);
			else
				_PerfTest.f_Add(Measure);
		}


		template <typename tf_CHeap, typename tf_CAllocPattern>
		void f_DoTest(CTestPerformance &_PerfTest, CTestMemory &_MemoryTest, ETestMeasureType _MeasureType, ch8 const *_pName, mint _MaxAllocSize, mint _nThreads)
		{
			//if (_MeasureType == ETestMeasureType_Debug)
			//	return; // Comment out during debugging?

			if (!(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
			{
				if (!tf_CHeap::fs_ShouldRun(_nThreads, tf_CAllocPattern::fs_Alignment()))
					return;

				CTestPerformanceResult Result;
				CTestMemoryResult MemoryResult;
				struct CLocalParser : public CTestResultParser
				{
					CTestPerformanceResult &m_Result;
					ETestMeasureType m_MeasureType;
					ch8 const *m_pName;
					bint m_bValidResult;
					zbint m_bFailedTest;
					CLocalParser(CTestPerformanceResult &_Result, ETestMeasureType _MeasureType, ch8 const *_pName)
						: m_Result(_Result)
						, m_MeasureType(_MeasureType)
						, m_pName(_pName)
						, m_bValidResult(false)
					{
					}
					virtual void f_HandleHeader(NMib::NRegistry::CRegistry_CStr const &_Reg) override
					{
					}
					virtual void f_HandleFooter(NMib::NRegistry::CRegistry_CStr const &_Reg) override
					{
					}
					virtual void f_HandleCategory(NMib::NRegistry::CRegistry_CStr const &_Reg) override
					{
					}
					virtual void f_HandleResult(NMib::NRegistry::CRegistry_CStr const &_Reg) override
					{
						CTestResult Result;
						CTestResultParser::fs_DecodeResult(_Reg, Result);

						if (Result.m_Result == ETestResult_Fail && Result.m_TestPath.f_Find("m_bFailed") >= 0)
						{
							m_bFailedTest = true;
						}

					}
					virtual void f_HandlePerformanceResult(NMib::NRegistry::CRegistry_CStr const &_Reg) override
					{
						CTestPerformanceResults Results;
						CTestResultParser::fs_DecodePerformanceResults(_Reg, Results);

						DMibCheck(Results.m_Results.f_GetLen() == 1);
						if (!Results.m_Results.f_IsEmpty())
						{
							m_Result = Results.m_Results.f_GetFirst();
							m_bValidResult = true;
						}
					}
					virtual void f_HandleMemoryResult(NMib::NRegistry::CRegistry_CStr const &_Reg) override
					{
					}
				}
				ResultParser(Result, _MeasureType, _pName);

				NMib::NProcess::CProcessLaunchParams Params;
				Params.m_Target = NMib::NFile::CFile::fs_GetProgramPath();
				Params.m_Parameters = NMib::NStr::CStr::CFormat("--Tests \"{}/{}\" --TestLogger Registry --TestResults (All ProcessRecursive) --TestGroups Performance") << fg_TestGetCurrentPath() << _pName;
				DMibTrace("{}\r\n", Params.m_Parameters);
				void *pProcess = nullptr;
				
				NMib::NPtr::TCUniquePointer<NMib::NProcess::CProcessLaunch> pProcessLaunch;
				
				NMib::NThread::CEvent Exited;
				Exited.f_ResetSignaled();

				auto fl_AddStats
					= [&](NMib::NProcess::CProcessStatistics const &_Stats)
					{
						for (auto Iter = _Stats.m_Statistics.f_GetIterator(); Iter; ++Iter)
						{
							fp64 Value = Iter->m_Value;
							NMib::NStr::CStr Name = Iter.f_GetKey();
							switch (Iter->m_Unit)
							{
							case NMib::NProcess::EProcessStatUnit_GeneralNumber:
								{
									if (!Iter->m_CustomUnit.f_IsEmpty())
										Name += NMib::NStr::CStr::CFormat(" ({})") << Iter->m_CustomUnit;
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Bytes:
								{
									if (Iter->m_IdealScale == fp64(1024*1024*1024))
									{
										Value /= Iter->m_IdealScale;
										Name += " (GiB)";
									}
									else if (Iter->m_IdealScale == fp64(1024*1024))
									{
										Value /= Iter->m_IdealScale;
										Name += " (MiB)";
									}
									else if (Iter->m_IdealScale == fp64(1024))
									{
										Value /= Iter->m_IdealScale;
										Name += " (KiB)";
									}
									else
										Name += " (Bytes)";
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Cycles:
								{
									Name += " (Cycles)";
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Seconds:
								{
									if (Iter->m_IdealScale == fp64(0.000000001))
									{
										Value /= Iter->m_IdealScale;
										Name += " (ns)";
									}
									else if (Iter->m_IdealScale == fp64(0.000001))
									{
										Value /= Iter->m_IdealScale;
										Name += " (µs)";
									}
									else if (Iter->m_IdealScale == fp64(0.001))
									{
										Value /= Iter->m_IdealScale;
										Name += " (ms)";
									}
									else
										Name += " (s)";
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Fraction:
								{
									Value *= 100.0;
									Name += " (%)";
								}
								break;
							default:
								DMibNeverGetHere(Iter->m_Unit);
								break;
							}
							Result.m_PerformanceCounters[Name] = CTestStats(Value);
						}
					}
				;
				
				Params.m_fOnStateChange = 
					[&](NMib::NProcess::CProcessLaunchStateChangeVariant const &_State, fp64 _TimeSinceStart)
					{
						switch (_State.f_GetTypeID())
						{
						case NMib::NProcess::EProcessLaunchState_Exited:
							{
								//uint32 ExitCode = _State.f_Get<NMib::NProcess::EProcessLaunchState_Exited>();
								DMibCheck(pProcess);
								if (pProcess)
								{
									NMib::NProcess::CProcessStatistics MemoryStats = pProcessLaunch->f_GetOverallMemoryStatistics();
									NMib::NProcess::CProcessStatistics ExecutionStats = pProcessLaunch->f_GetOverallExecutionStatistics();


									fl_AddStats(ExecutionStats);
									fl_AddStats(MemoryStats);
									/*
									for (auto Iter = MemoryStats.m_Statistics.f_GetIterator(); Iter; ++Iter)
									{
										if (Iter.f_GetKey() == "Peak working set size")
											MemoryResult.m_AllAllocations.m_BytesMaxAlloc = CTestStats(fp64(*Iter));
										MemoryResult.m_PerAllocationType[Iter.f_GetKey()].m_BytesMaxAlloc = CTestStats(fp64(*Iter));
									}*/

							
								}
								Exited.f_SetSignaled();
							}
							break;
						case NMib::NProcess::EProcessLaunchState_Launched:
							{
								pProcess = _State.f_Get<NMib::NProcess::EProcessLaunchState_Launched>();
							}
							break;
						case NMib::NProcess::EProcessLaunchState_LaunchFailed:
							{
								DMibDTrace("Error: {}\r\n", _State.f_Get<NMib::NProcess::EProcessLaunchState_LaunchFailed>());
								Exited.f_SetSignaled();
							}
							break;
						}
					}
				;
				Params.m_fOnOutput = 
					[&](NMib::NProcess::EProcessLaunchOutputType _OutputType, NMib::NStr::CStr const &_Output)
					{
						if (_OutputType == NMib::NProcess::EProcessLaunchOutputType_StdOut)
							ResultParser.f_FeedText(_Output);
					}
				;
				{
					pProcessLaunch = NMib::fg_Construct(Params, NMib::NProcess::EProcessLaunchCloseFlag_BlockOnExit);
					
				}
				
				NMib::NProcess::CProcessStatistics SampledMemoryStats;
				
				while (true)
				{
					try
					{
						auto ThisStat = pProcessLaunch->f_GetMemoryStatistics();
						for (auto iStat = ThisStat.m_Statistics.f_GetIterator(); iStat; ++iStat)
						{
							auto Mapped = SampledMemoryStats.m_Statistics("(Sampled Max) " + iStat.f_GetKey(), *iStat);
							if (!Mapped.f_WasCreated())
							{
								if (iStat->m_Value > (*Mapped).m_Value)
									(*Mapped).m_Value = iStat->m_Value;
							}
								
						}
					}
					catch (NMib::NException::CException const &)
					{
					}
					
					if (!Exited.f_WaitTimeout(0.01))
						break; // Exited
				}

				pProcessLaunch.f_Clear();
				
				fl_AddStats(SampledMemoryStats);

				if (ResultParser.m_bFailedTest)
					DMibTest(!DMibExpr(ResultParser.m_bFailedTest))(ETestFlag_Aggregated);

				if (ResultParser.m_bValidResult)
				{
					_PerfTest.f_Add(Result);

					if (!MemoryResult.m_PerAllocationType.f_IsEmpty())
					{
						MemoryResult.m_MeasureType = Result.m_MeasureType;
						MemoryResult.m_Name = Result.m_Name;
						MemoryResult.m_nIterations = 1;
						MemoryResult.m_nRepetitions = 1;
						_MemoryTest.f_Add(MemoryResult);
					}
				}
				else
				{
					DMibTest(DMibExpr(false) && DMibExpr("Recursive test failed"))(Params.m_Parameters);
				}

				return;
			}

			struct CPerfromTest
			{
				CPerformance_Tests &m_This;
				CPerfromTest(CPerformance_Tests &_This)
					: m_This(_This)
				{
				}
				void f_Perform(CTestPerformance &_PerfTest, ETestMeasureType _MeasureType, ch8 const *_pName, mint _MaxAllocSize, mint _nThreads)
				{
					m_This.f_DoTestPerform<tf_CHeap, tf_CAllocPattern>(_PerfTest, _MeasureType, _pName, _MaxAllocSize, _nThreads);
				}
			};

			CPerfromTest Performer(*this);

			DMibTestSuite(_pName)
			{
				Performer.f_Perform(_PerfTest, _MeasureType, _pName, _MaxAllocSize, _nThreads);
				DMibTest(DMibExpr(_PerfTest));
			};
		}

		template <typename tf_CAllocPattern>
		void f_DoTestsManagers(mint _MaxAllocSize, tf_CAllocPattern const &_Pattern, mint _nThreads)
		{
			CTestPerformance PerfTest(0.5, false);
			CTestMemory MemoryTest(0.5, false);
			f_DoTest<CMalterlibMemoryDummy, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Baseline, "Baseline", _MaxAllocSize, _nThreads);
#ifdef DMemoryManagerTestEnable_MalterlibNew
			f_DoTest<CMalterlibMemoryMalterlibNew, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Normal, "Malterlib", _MaxAllocSize, _nThreads);
			f_DoTest<CMalterlibMemoryMalterlibNew_NoCheckout, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Normal, "Malterlib_Checkout", _MaxAllocSize, _nThreads);

			//f_DoTest<CMalterlibMemoryMalterlibNew_Tracked, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_Tracked", _MaxAllocSize, _nThreads);
			//f_DoTest<CMalterlibMemoryMalterlibNew_Debug, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_Debug", _MaxAllocSize, _nThreads);
			/*
			f_DoTest<CMalterlibMemoryMalterlibNew_NoCommit, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_NoCommit", _MaxAllocSize, _nThreads);
			f_DoTest<CMalterlibMemoryMalterlibNew_NoDeferCleanup, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_DirectCleanup", _MaxAllocSize, _nThreads);
			f_DoTest<CMalterlibMemoryMalterlibNew_NoCleanup, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_NoCleanup", _MaxAllocSize, _nThreads);
			 */
#endif
#ifdef DMemoryManagerTestEnable_OSX
			f_DoTest<CMalterlibMemoryOSX, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "OSX", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_StdLib
			f_DoTest<CMalterlibMemoryStdLib, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "StdLib", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_Application
			f_DoTest<CMalterlibMemoryApplication, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "Application", _MaxAllocSize, _nThreads);
#endif
			
#ifdef DMemoryManagerTestEnable_LLAlloc
			f_DoTest<CMalterlibMemoryLLAlloc, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "LLAlloc", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_DlMalloc
			f_DoTest<CMalterlibMemoryDlMallocClear, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "DlMalloc_ST", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_DlMallocMultiThreaded
			f_DoTest<CMalterlibMemoryDlMallocMultiThreadedClear, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "DlMalloc", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_TcMalloc
			f_DoTest<CMalterlibMemoryTcMalloc, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "TcMalloc", _MaxAllocSize, _nThreads);
#endif
      
#ifdef DMemoryManagerTestEnable_PtMalloc
			f_DoTest<CMalterlibMemoryPtMalloc, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "PtMalloc", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_WindowsLF
			f_DoTest<TCMalterlibMemoryWindowsLF<false>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "WindowsLF", _MaxAllocSize, _nThreads);
#endif

#ifdef DMemoryManagerTestEnable_MalterlibOld
			f_DoTest<TCMalterlibMemoryImp<CHeap_StandAlone_Commit1_Clear1_Lock0>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "MalterlibOld_ST", _MaxAllocSize, _nThreads);
			f_DoTest<TCMalterlibMemoryImp<CHeap_Combined_Commit1_Clear1_Lock1>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "MalterlibOld_Combined", _MaxAllocSize, _nThreads);

//			f_DoTest<TCMalterlibMemoryImp<CHeap_StandAlone_Commit1_Clear1_Lock1>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "MalterlibMT", _MaxAllocSize, _nThreads);
//			f_DoTest<TCMalterlibMemoryImp<CHeap_Combined_Commit1_Clear1_Lock0>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "MalterlibCombined", _MaxAllocSize, _nThreads);
/*			f_DoTest<TCMalterlibMemoryImp<CHeapSmall_StandAlone_Commit1_Clear1_Lock0>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "MalterlibSmall", _MaxAllocSize, _nThreads);
			f_DoTest<TCMalterlibMemoryImp<CHeapSmall_Combined_Commit1_Clear1_Lock0>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "MalterlibSmallCombined", _MaxAllocSize, _nThreads);
			f_DoTest<TCMalterlibMemoryImp<CHeapSmall_StandAlone_Commit1_Clear1_Lock1>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "MalterlibSmallMT", _MaxAllocSize, _nThreads);
			f_DoTest<TCMalterlibMemoryImp<CHeapSmall_Combined_Commit1_Clear1_Lock1>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "MalterlibSmallCombinedMT", _MaxAllocSize, _nThreads);*/
#endif

			if (!(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
			{
				DMibTest(DMibExpr(PerfTest));
				if (!MemoryTest.f_IsEmpty())
					DMibTest(DMibExpr(MemoryTest));
			}
		}

		template <typename tf_CAllocPattern>
		void f_DoTests(mint _MaxAllocSize, tf_CAllocPattern const &_Pattern, mint _nThreads)
		{
			f_DoTestsManagers(_MaxAllocSize, _Pattern, _nThreads);
		}

		template <typename tf_CAllocPattern>
		void f_DoTests(tf_CAllocPattern const &_Pattern, mint _nThreads)
		{
			for 
#if 1
				(
					mint i = 1
					; i <= 1024*1024*8
					; i = i << 1
				)
#else
				(
					mint i = 1
					//; i <= 1024*1024*8
#if DMibPPtrBits == 64
					; i <= 12
#else
					; i <= 4
#endif
					; i = i < 4 ? i << 1 : i + 4
				)
#endif
			{
				if (!(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
				{
					DMibTestSuite(NMib::NStr::CStr::CFormat("Max Alloc({})") << i)
					{
						f_DoTests(i, _Pattern, _nThreads);
					};
				}
				else
				{
					DMibTestCategory(NMib::NStr::CStr::CFormat("Max Alloc({})") << i)
					{
						f_DoTests(i, _Pattern, _nThreads);
					};
				}
			}
		}

		template <typename tf_CAllocPattern>
		void f_DoPattern(ch8 const *_pPattern, tf_CAllocPattern const &_Pattern)
		{
			DMibTestCategory(_pPattern)
			{
				mint nCores = m_nCores;
				//nCores = 1;
				mint i = 1;
				for (; i < nCores; i = i << 1)
				{
					DMibTestCategory(NMib::NStr::CStr::CFormat("Threads({})") << i)
					{
						f_DoTests(_Pattern, i);
					};
				}
				i = nCores;
				DMibTestCategory(NMib::NStr::CStr::CFormat("Threads({})") << i)
				{
					f_DoTests(_Pattern, i);
				};
				i = i << 1;
				mint nEndCores = nCores * 2;
#if 0
				mint nEndCores = NMib::fg_Min(nCores*1024, 64u); // Max 4096 threads as it taskes some time to start threads
				//mint nEndCores = NMib::fg_Min(nCores*1024, 4096u); // Max 4096 threads as it taskes some time to start threads
				if (NMib::NSys::fg_System_BeingDebugged() && !(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
					nEndCores = NMib::fg_Min(nEndCores, 128u); // Running in debugger the debugger makes creating threads really slow
#endif
				
				for (; i <= nEndCores; i = i << 1)
				{
					DMibTestCategory(NMib::NStr::CStr::CFormat("Threads({})") << i)
					{
						f_DoTests(_Pattern, i);
					};
				}
			};
		}

		void f_DoPatterns()
		{
			f_DoPattern("Random", CAllocPattern_Random());
			f_DoPattern("OneSize", CAllocPattern_OneSize());
			f_DoPattern("OneSizeLinear", CAllocPattern_OneSizeLinear());
			f_DoPattern("RandomAligned", CAllocPattern_RandomAlignment());
		}

		void f_DoTests()
		{
#if 0
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(32)/Max Alloc(1)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(1)/Max Alloc(1)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests Malterlib/Mem/MemoryManager* --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(16)/*" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(32)/Max Alloc(1)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(16)/Max Alloc(4)/*" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(16)/Max Alloc(4)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(1)/Max Alloc(131072)/MalterlibNewNoCommit" --TestLogger Registry --TestResults (All ProcessRecursive) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(32)/Max Alloc(512)" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(4)/Max Alloc(512)" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(1)/*" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/OneSize/Threads(1)/*" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)

			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/RandomAligned/Threads(1)/*" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/RandomAligned/Threads(1)/Max Alloc(32768)" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)

			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(4)/*" --TestGroups Performance --TestResults (All DetailedPerformance CompareToBaseline)
			
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(4)/Max Alloc(1)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/OneSize/Threads(1)/Max Alloc(8)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/OneSize/Threads(1)/Max Alloc(1)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(1)/Max Alloc(1024)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/OneSize/Threads(1)/Max Alloc(16)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance

			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(4)/Max Alloc(16)/MalterlibNewNoCommit" --TestLogger Registry --TestResults (All ProcessRecursive DetailedPerformance) --TestGroups Performance
			
			--Tests "Malterlib/Mem/MemoryManager/Performance/Synthetic/Random/Threads(1)/Max Alloc(262144)/MalterlibNew" --TestLogger Registry --TestResults (All ProcessRecursive) --TestGroups Performance
#endif

			
			DMibTestCategory(CTestCategory("Synthetic") << CTestGroup("Performance"))
			{
				f_DoPatterns();
			};
		}
	};

	DMibTestRegister(CPerformance_Tests, Malterlib::Memory::MemoryManager);

}

