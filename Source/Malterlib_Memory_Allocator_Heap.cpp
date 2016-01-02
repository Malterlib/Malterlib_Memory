// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_Memory_Allocator_Heap.h"
#include "Malterlib_Memory_Heap.h"

#ifndef DMibNoInlineNew
#	error "You must define this when compling this file"
#endif

namespace NMib
{
	namespace NMem
	{

		/************************************************************************************************\
		||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
		|| Heap alloc functions
		||______________________________________________________________________________________________||
		\************************************************************************************************/
		
		bint g_MalterlibMemoryManager_Debug_EnableStackTrace = DMibConfig_MalterlibMemoryManager_Debug_EnableStackTrace;
		
		bool CHeap_FillDebug::ms_bDisableFillChecks 
			= DMibConfig_MalterlibMemoryManager_Debug_EnableFreedGuards 
			| DMibConfig_MalterlibMemoryManager_Debug_EnableAllocatedFill 
			| DMibConfig_MalterlibMemoryManager_Debug_EnableEdgeGuards
		;

		ch8 CAllocator_Virtual::ms_HeapName[] = "Virtual";

#if DMibConfig_Memory_Shims_Enable
		CAllocator_Virtual::CHeapInit CAllocator_Virtual::ms_HeapInit;
#endif
	}
}


/************************************************************************************************\
||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
|| new operators
||______________________________________________________________________________________________||
\************************************************************************************************/


#if defined(DMibPOverrideOperatorNew) && !defined(DCompiler_MSVC)

		only_parameters_aliased return_not_aliased void * calling_convention_c operator new (mint _Size)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}

		only_parameters_aliased void calling_convention_c operator delete (void *_pToDelete) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}

		only_parameters_aliased return_not_aliased void * calling_convention_c operator new[] (mint _Size)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}

		only_parameters_aliased void calling_convention_c operator delete[] (void *_pToDelete) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}

		only_parameters_aliased return_not_aliased void * calling_convention_c operator new (mint _Size, const std::nothrow_t &) noexcept
		{
			try
			{
				return NMib::NMem::fg_Alloc(_Size);
			}
			catch (...)
			{
				return nullptr;
			}
		}

		only_parameters_aliased void calling_convention_c operator delete (void *_pToDelete, const std::nothrow_t &) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}

		only_parameters_aliased return_not_aliased void * calling_convention_c operator new[] (mint _Size, const std::nothrow_t &) noexcept
		{
			try
			{
				return NMib::NMem::fg_Alloc(_Size);
			}
			catch (...)
			{
				return nullptr;
			}
		}

		only_parameters_aliased void calling_convention_c operator delete[] (void *_pToDelete, const std::nothrow_t &) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}
#endif




