// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#define module_export __attribute__ ((__visibility__("default")))
#define assure_used __attribute__((used))

#include <stdlib.h>

extern "C"
{
	module_export void fg_MalterlibSystem_InitEarly(int argc, char const* argv[], char const* envp[], char const* apple[], const struct ProgramVars * vars);
	extern void fg_MalterlibSystem_InitDyldDummyHelper() __attribute__((weak_import));
	assure_used module_export void fg_MalterlibSystem_InitHelper()
	{
		
	}
}

namespace
{
	void __attribute__ ((constructor(-1111111111))) fg_InitMalterlibEarly(int argc, char const* argv[], char const* envp[], char const* apple[], const struct ProgramVars * vars)
	{
		if (fg_MalterlibSystem_InitDyldDummyHelper)
			fg_MalterlibSystem_InitDyldDummyHelper();
		fg_MalterlibSystem_InitEarly(argc, argv, envp, apple, vars);
	}
}