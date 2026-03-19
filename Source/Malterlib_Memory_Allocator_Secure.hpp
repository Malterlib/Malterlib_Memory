// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	/************************************************************************************************\
	||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
	|| Secure Allocator
	||______________________________________________________________________________________________||
	\************************************************************************************************/


	template <typename t_CBaseAllocator, bool t_bStatic>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		return CBaseAllocator::f_Realloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
	}

	template <typename t_CBaseAllocator, bool t_bStatic>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		return CBaseAllocator::f_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	template <typename t_CBaseAllocator, bool t_bStatic>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		void* pNewMem = CBaseAllocator::f_AllocWithSize(_Size, _AllocFlags, _NumaNode);

		fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		CBaseAllocator::f_Free(_pMem, _OldSize);

		return pNewMem;
	}

	template <typename t_CBaseAllocator, bool t_bStatic>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		void* pNewMem = CBaseAllocator::f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);

		fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		CBaseAllocator::f_Free(_pMem, _OldSize);

		return pNewMem;
	}

	template <typename t_CBaseAllocator, bool t_bStatic>
	inline_small void TCAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_Free(void *_pBlock, umint _Size)
	{
		DMibFastCheck(_Size != 0);
		fg_SecureMemClear((uint8*)_pBlock, _Size);
		return CBaseAllocator::f_Free(_pBlock, _Size);
	}

	template <typename t_CBaseAllocator, bool t_bStatic>
	inline_small void TCAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_FreeNoSize(void *_pBlock)
	{
		umint Size = CBaseAllocator::f_Size(_pBlock);
		fg_SecureMemClear((uint8*)_pBlock, Size);
		return CBaseAllocator::f_Free(_pBlock, Size);
	}


	template<typename t_CBaseAllocator>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, true>::f_Realloc(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		return CBaseAllocator::f_Realloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
	}

	template<typename t_CBaseAllocator>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, true>::f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		return CBaseAllocator::f_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
	}

	template<typename t_CBaseAllocator>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, true>::f_Resize(void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		void* pNewMem = CBaseAllocator::f_AllocWithSize(_Size, _AllocFlags, _NumaNode);

		fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		CBaseAllocator::f_Free(_pMem, _OldSize);

		return pNewMem;
	}

	template<typename t_CBaseAllocator>
	inline_small void *TCAllocator_Secure<t_CBaseAllocator, true>::f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		if (_OldSize == 0)
			_OldSize = CBaseAllocator::f_Size(_pMem);

		void* pNewMem = CBaseAllocator::f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);

		fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

		fg_SecureMemClear((uint8*)_pMem, _OldSize);

		CBaseAllocator::f_Free(_pMem, _OldSize);

		return pNewMem;
	}

	template<typename t_CBaseAllocator>
	inline_small void TCAllocator_Secure<t_CBaseAllocator, true>::f_Free(void *_pBlock, umint _Size)
	{
		DMibFastCheck(_Size != 0);

		fg_SecureMemClear((uint8*)_pBlock, _Size);

		return CBaseAllocator::f_Free(_pBlock, _Size);
	}

	template<typename t_CBaseAllocator>
	inline_small void TCAllocator_Secure<t_CBaseAllocator, true>::f_FreeNoSize(void *_pBlock)
	{
		umint Size = CBaseAllocator::f_Size(_pBlock);

		fg_SecureMemClear((uint8*)_pBlock, Size);

		return CBaseAllocator::f_Free(_pBlock, Size);
	}
}
