// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#define module_export __attribute__ ((__visibility__("default")))
#define assure_used __attribute__((used))

#if defined(DMibMemoryOverrideDll)
#define DMibMalterlibOverrideMallocExport
#else
#define DMibMalterlibOverrideMallocExport module_export
#endif

#include "Malterlib_Memory_SystemOverride_MacOSInterpose.h"

#pragma clang diagnostic ignored "-Wunguarded-availability-new"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc/malloc.h>

extern "C"
{
	extern void fg_MalterlibSystem_InitEarly(COriginalFunctions const &_Functions, int argc, char const* argv[], char const* envp[], char const* apple[], const struct ProgramVars * vars);
	extern void fg_MalterlibSystem_InitBeforeMalloc(COriginalFunctions const &_Functions);
	extern void fg_MalterlibSystem_InitAfterMalloc();
#if !defined(DMibMemoryOverrideDll)
	extern void fg_MalterlibSystem_InitDyldDummyHelper() __attribute__((weak_import));
#endif
	assure_used DMibMalterlibOverrideMallocExport void fg_MalterlibSystem_InitHelper()
	{

	}

	int malloc_jumpstart(int);

}
struct CTesting
{
	void *m_pTest1;
	void *m_pTest2;
};

extern "C"
{

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) &d_Function,

#ifndef DMibMemoryOverrideDll
	static
#endif

	constinit COriginalFunctions g_OriginalFunctions =
		{
#			include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
		}
	;

	struct CInterpose
	{
		void (*m_Replacement)();
		void (*m_Replacee)();
	};

#define DMibMacOSInterpose(d_Replacement, d_Replacee) \
	assure_used static CInterpose g_Malterlib_Interpose_##d_Replacee \
	__attribute__ ((section ("__DATA,__interpose"))) = { (void (*)())&d_Replacement, (void (*)())&d_Replacee}

	extern void __malloc_init(const char *apple[]) __attribute__((weak_import)); // from libsystem_malloc.dylib
	void fg_Malterlib___malloc_init(const char *apple[])
	{
		fg_MalterlibSystem_InitBeforeMalloc(g_OriginalFunctions);
		if (__malloc_init)
			__malloc_init(apple);
		fg_MalterlibSystem_InitAfterMalloc();
	}

	extern void __libc_init(const struct ProgramVars *vars, void (*atfork_prepare)(void),void (*atfork_parent)(void),void (*atfork_child)(void),const char *apple[]) __attribute__((weak_import));
	void fg_Malterlib___libc_init(const struct ProgramVars *vars, void (*atfork_prepare)(void),void (*atfork_parent)(void),void (*atfork_child)(void),const char *apple[])
	{
		fg_MalterlibSystem_InitBeforeMalloc(g_OriginalFunctions);
		if (__libc_init)
			__libc_init(vars, atfork_prepare, atfork_parent, atfork_child, apple);
		fg_MalterlibSystem_InitAfterMalloc();
	}

	DMibMacOSInterpose(fg_Malterlib___malloc_init, __malloc_init);
	DMibMacOSInterpose(fg_Malterlib___libc_init, __libc_init);

#if defined(DMibMemoryOverrideDll)

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) \
	extern d_Return fg_Malterlib_##d_Function(__VA_ARGS__); \
	DMibMacOSInterpose(fg_Malterlib_##d_Function, d_Function);

#else

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) \
	extern d_Return fg_Malterlib_##d_Function(__VA_ARGS__); \
	module_export d_Return fg_Malterlib_Interpose_##d_Function(__VA_ARGS__) \
	{ \
		return fg_Malterlib_##d_Function d_Args; \
	} \
	DMibMacOSInterpose(fg_Malterlib_Interpose_##d_Function, d_Function);

#endif

#include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
}

namespace
{
	void __attribute__ ((constructor(-1111111111))) fg_InitMalterlibEarly(int argc, char const* argv[], char const* envp[], char const* apple[], const struct ProgramVars * vars)
	{
#if !defined(DMibMemoryOverrideDll)
		if (fg_MalterlibSystem_InitDyldDummyHelper)
			fg_MalterlibSystem_InitDyldDummyHelper();
#endif
		fg_MalterlibSystem_InitEarly(g_OriginalFunctions, argc, argv, envp, apple, vars);
	}
}
