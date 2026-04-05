// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>

void fg_MalterlibMallocOverrideInit();

struct CTest
{
	CTest()
	{
		fg_MalterlibMallocOverrideInit();
	}

};

CTest g_Test;
