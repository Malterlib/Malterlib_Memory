// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#if 0

class CTestPool : public CMalterlibTest
{
public:

	class CTestClass
	{
	public:
//		CTestClass()
//		{
//			DMibTrace("CTestClass()\n", 0);
//		}
//		~CTestClass()
//		{
////			DMibTrace("~CTestClass()\n", 0);
	//	}
		int32 m_Linkage1;
//		int32 m_Linkage2;
//		int32 m_Linkage3;
//		DMibListLinkD_Link(CTestClass, m_Link);
	};


	bool f_AutomaticTest()
	{
		return true;
	}

	NMib::NStr::CStr Certify(CTestInterface &_Interface)
	{
//		DMibListLinkD_List(CTestClass, m_Link) m_Test;

//		DMibTrace("\n\nPool\n\n", 0);

		NMib::NMemory::TCPool<CTestClass, 128, NMib::NThread::CNoLock, NMib::NMemory::CPoolType_Freeable, NMib::NMemory::CAllocator_Virtual> TestPool;

//		TCPoolAggregate<CTestClass> TestPool = {0};

		DMibTrace("sizeof(CTestClass) = {}\n", sizeof(CTestClass));

#ifdef DMibDebug
		static const aint NumTests = 1024;
#else
		static const aint NumTests = 128000;
#endif
		static const aint NumIter = 32;

		CTestClass **TestList = DMibNew CTestClass*[NumTests];

		NMib::NTime::CPerfTimeMeasureMin Timer;
		NMib::NTime::CPerfTimeMeasureMin TimerDelete;
		for (aint i = 0; i < NumIter; ++i)
		{
			{
				DMibScopePerfTimeMeasureMin(Timer);
				for (aint i = 0; i < NumTests; ++i)
				{
					TestList[i] = TestPool.f_New();
				}
			}
			{
				DMibScopePerfTimeMeasureMin(TimerDelete);
				for (aint i = 0; i < NumTests; ++i)
				{
					TestPool.f_Delete(TestList[i]);
				}
			}
		}
		DMibTrace("Performance for Pool new CPoolTye_Freeable = {0} news per second\n", fp64(NumTests) / Timer.f_GetTime());
		DMibTrace("Performance for Pool delete CPoolTye_Freeable = {0} deletes per second\n", fp64(NumTests) / TimerDelete.f_GetTime());

		Timer.f_Reset();
		TimerDelete.f_Reset();
		NMib::NMemory::TCPool<CTestClass, 128, NMib::NThread::CNoLock, NMib::NMemory::CPoolType_Growing, NMib::NMemory::CAllocator_Virtual> TestPoolGrowing;
		for (aint i = 0; i < NumIter; ++i)
		{
			{
				DMibScopePerfTimeMeasureMin(Timer);
				for (aint i = 0; i < NumTests; ++i)
				{
					TestList[i] = TestPoolGrowing.f_New();
				}
			}
			{
				DMibScopePerfTimeMeasureMin(TimerDelete);
				for (aint i = 0; i < NumTests; ++i)
				{
					TestPoolGrowing.f_Delete(TestList[i]);
				}
			}
		}

		DMibTrace("Performance for Pool new CPoolTye_Growing = {0} news per second\n", fp64(NumTests) / Timer.f_GetTime());
		DMibTrace("Performance for Pool delete CPoolTye_Growing = {0} deletes per second\n", fp64(NumTests) / TimerDelete.f_GetTime());

		delete [] TestList;

		return ("");
	}


};

DMibRuntimeClass(CMalterlibTest, CTestPool);

#endif
