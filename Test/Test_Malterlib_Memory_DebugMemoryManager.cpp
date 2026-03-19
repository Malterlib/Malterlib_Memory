// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Memory/DebugMemoryManager>

namespace
{
	class CDebugMemoryManager_Tests : public CTest
	{
	public:

		void f_DoTests()
		{
			DMibTestSuite("TestAlloc")
			{
				NMib::NMemory::TCDebugMemoryManager<NMib::NMemory::EDebugMemoryManager_None> TestManager;

				void *pMem[10000];
				for (umint i = 0; i < 10000; ++i)
				{
					umint Size = i;
					pMem[i] = TestManager.f_AllocWithSize(Size, 1);
				}
				for (umint i = 0; i < 10000; ++i)
				{
					umint Size = NMib::fg_Max(i, 1u);
					TestManager.f_Free(pMem[i], Size);
				}
				return "";
			};
		}
	};

	DMibTestRegister(CDebugMemoryManager_Tests, Malterlib::Memory);
}

