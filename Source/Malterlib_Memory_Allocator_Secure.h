// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{

		/************************************************************************************************\
		||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
		|| Secure Allocator
		||______________________________________________________________________________________________||
		\************************************************************************************************/
				
		template
		<
			typename t_CBaseAllocator
			, bool t_bStatic = t_CBaseAllocator::mc_bMethodsStatic
		>
		class TCAllocator_Secure : public t_CBaseAllocator
		{
		public:
			typedef t_CBaseAllocator CBaseAllocator;

			void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			void f_Free(void *_pBlock, mint _Size = 0);
		};	


		template<typename t_CBaseAllocator>
		class TCAllocator_Secure<t_CBaseAllocator, true> : public t_CBaseAllocator
		{
		public:
			typedef t_CBaseAllocator CBaseAllocator;

			static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			static void f_Free(void *_pBlock, mint _Size = 0);
		};
		
		using CAllocator_HeapSecure = TCAllocator_Secure<CAllocator_Heap>;

	} // Namespace NMem

} // Namespace NMib
