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
				NMib::NMem::TCDebugMemoryManager<NMib::NMem::EDebugMemoryManager_None> TestManager;

				void *pMem[10000];
				for (mint i = 0; i < 10000; ++i)
				{
					mint Size = i;
					pMem[i] = TestManager.f_AllocWithSize(Size, 1);
				}
				for (mint i = 0; i < 10000; ++i)
				{
					mint Size = NMib::fg_Max(i, 1u);
					TestManager.f_Free(pMem[i], Size);
				}
				return "";		
			};
		}
	};

	DMibTestRegister(CDebugMemoryManager_Tests, Malterlib::Memory);
}

