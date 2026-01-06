// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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
