// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{

		/************************************************************************************************\
		||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
		|| Secure Allocator
		||______________________________________________________________________________________________||
		\************************************************************************************************/

		
		template <typename t_CBaseAllocator, bool t_bStatic>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			return CBaseAllocator::f_Realloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		}

		template <typename t_CBaseAllocator, bool t_bStatic>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			return CBaseAllocator::f_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}

		template <typename t_CBaseAllocator, bool t_bStatic>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			void* pNewMem = CBaseAllocator::f_Alloc(_Size, _AllocFlags, _NumaNode);

			fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			CBaseAllocator::f_Free(_pMem);

			return pNewMem;
		}

		template <typename t_CBaseAllocator, bool t_bStatic>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			void* pNewMem = CBaseAllocator::f_AllocDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);

			fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			CBaseAllocator::f_Free(_pMem);

			return pNewMem;
		}

		template <typename t_CBaseAllocator, bool t_bStatic>
		inline_small void TAllocator_Secure<t_CBaseAllocator, t_bStatic>::f_Free(void *_pBlock, mint _Size)
		{
			if (_Size == 0)
				_Size = CBaseAllocator::f_Size(_pBlock);

			NMem::fg_ObjectSet((uint8*)_pBlock, 0, _Size);

			return CBaseAllocator::f_Free(_pBlock, _Size);
		}


		template<typename t_CBaseAllocator>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, true>::f_Realloc(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			return CBaseAllocator::f_Realloc(_pMem, _Size, _OldSize, _AllocFlags, _NumaNode);
		}

		template<typename t_CBaseAllocator>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, true>::f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			return CBaseAllocator::f_ReallocDebug(_pMem, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);
		}

		template<typename t_CBaseAllocator>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, true>::f_Resize(void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			void* pNewMem = CBaseAllocator::f_Alloc(_Size, _AllocFlags, _NumaNode);

			fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			CBaseAllocator::f_Free(_pMem);

			return pNewMem;
		}

		template<typename t_CBaseAllocator>
		inline_small void *TAllocator_Secure<t_CBaseAllocator, true>::f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			if (_OldSize == 0)
				_OldSize = CBaseAllocator::f_Size(_pMem);

			void* pNewMem = CBaseAllocator::f_AllocDebug(_Size, _pFile, _Line, _Flags, _AllocFlags, _NumaNode);

			fg_MemCopy(pNewMem, _pMem, fg_Min(_Size, _OldSize));

			NMem::fg_ObjectSet((uint8*)_pMem, 0, _OldSize);

			CBaseAllocator::f_Free(_pMem);

			return pNewMem;
		}

		template<typename t_CBaseAllocator>
		inline_small void TAllocator_Secure<t_CBaseAllocator, true>::f_Free(void *_pBlock, mint _Size)
		{
			if (_Size == 0)
				_Size = CBaseAllocator::f_Size(_pBlock);

			NMem::fg_ObjectSet((uint8*)_pBlock, 0, _Size);

			return CBaseAllocator::f_Free(_pBlock, _Size);
		}

	} // Namespace NMem

} // Namespace NMib
