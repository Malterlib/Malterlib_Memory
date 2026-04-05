// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Test/Test>
#include <Mib/Test/Performance>

#ifdef DPlatformFamily_macOS
	#include <memory>
	#include <malloc/malloc.h>
#endif

namespace
{
	using namespace NMib;
	using namespace NMib::NMemory;
	using namespace NMib::NStr;
	using namespace NMib::NContainer;

	class COverride_Tests : public CTest
	{
	public:

		void f_DoTests()
		{
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
