// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Memory/DebugMemoryManager>

namespace
{
	struct CDebugMemoryManager_Tests : public CTest
	{
		void f_DoTests()
		{
			DMibTestSuite("TestAlloc")
			{
				NMib::NMemory::TCDebugMemoryManager<NMib::NMemory::EDebugMemoryManager_None> TestManager;

				void *pMem[10000];
				for (umint iAlloc = 0; iAlloc < 10000; ++iAlloc)
				{
					umint Size = iAlloc;
					pMem[iAlloc] = TestManager.f_AllocWithSize(Size, 1);
				}
				for (umint iAlloc = 0; iAlloc < 10000; ++iAlloc)
				{
					umint Size = NMib::fg_Max(iAlloc, 1u);
					TestManager.f_Free(pMem[iAlloc], Size);
				}
				return "";
			};
		}
	};

	DMibTestRegister(CDebugMemoryManager_Tests, Malterlib::Memory);
}

