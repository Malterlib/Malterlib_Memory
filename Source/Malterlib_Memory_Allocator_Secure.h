// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
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
		using CBaseAllocator = t_CBaseAllocator;
		using t_CBaseAllocator::operator <=>;
		using t_CBaseAllocator::operator ==;

		void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		void f_Free(void *_pBlock, umint _Size);
		void f_FreeNoSize(void *_pBlock);
	};


	template<typename t_CBaseAllocator>
	class TCAllocator_Secure<t_CBaseAllocator, true> : public t_CBaseAllocator
	{
	public:
		using CBaseAllocator = t_CBaseAllocator;
		using t_CBaseAllocator::operator <=>;
		using t_CBaseAllocator::operator ==;

		static void *f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static void f_Free(void *_pBlock, umint _Size);
		static void f_FreeNoSize(void *_pBlock);
	};

	struct CAllocator_HeapSecure : public TCAllocator_Secure<CAllocator_Heap, true>
	{
		using TCAllocator_Secure<CAllocator_Heap, true>::operator <=>;
		using TCAllocator_Secure<CAllocator_Heap, true>::operator ==;
	};
}
