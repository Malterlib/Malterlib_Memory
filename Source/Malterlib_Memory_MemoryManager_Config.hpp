// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{

		///
		/// Global
		/// ======
		
		template <typename tf_CMemoryManager>
		inline CDefaultMemoryManagerNotifier::CGlobal::CGlobal(tf_CMemoryManager & _MemMan)
		{
		}
		
		inline void CDefaultMemoryManagerNotifier::CGlobal::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
		{
		}
		
		inline void CDefaultMemoryManagerNotifier::CGlobal::f_OnFree(uint8 *_pMemory)
		{
		}
		
		///
		/// Arena
		/// =====
		
		inline CDefaultMemoryManagerNotifier::CArena::CArena(CGlobal *_pGlobal)
		{
		}
		
		inline void CDefaultMemoryManagerNotifier::CArena::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
		{
		}

		inline void CDefaultMemoryManagerNotifier::CArena::f_OnFree(uint8 *_pMemory)
		{
		}
		
		inline void CDefaultMemoryManagerNotifier::CArena::f_OnFillFree(uint8 *_pMemory, mint _nBytes)
		{
		}
		
		inline bool CDefaultMemoryManagerNotifier::CArena::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak)
		{
			return false;
		}
		
		///
		/// Heap
		/// ====
		
		inline CDefaultMemoryManagerNotifier::CHeap::CHeap(CGlobal *_pGlobal)
		{
		}
		
		inline void CDefaultMemoryManagerNotifier::CHeap::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
		{
		}

		inline void CDefaultMemoryManagerNotifier::CHeap::f_OnFree(uint8 *_pMemory)
		{
		}
		
		inline void CDefaultMemoryManagerNotifier::CHeap::f_OnFillFree(uint8 *_pMemory, mint _nBytes)
		{
		}
		
		inline bool CDefaultMemoryManagerNotifier::CHeap::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak)
		{
			return false;
		}
		
		
	}
}
