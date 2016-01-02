// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{


		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Resize																							|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		inline_medium void *TCHeap<t_CHeapParams>::f_Realloc(void *_pMem, mint &_NewSize)
		{
			f_Free(_pMem);
			return f_Alloc(_NewSize);
		}

		template <class t_CHeapParams>
		inline_medium void *TCHeap<t_CHeapParams>::fp_Realloc(CChunk *_pChunk, void *_pMem, mint &_NewSize)
		{
			fp_Free(_pChunk, _pMem);
			return f_Alloc(_NewSize);
		}

		template <class t_CHeapParams>
		inline_medium void *TCHeap<t_CHeapParams>::f_Resize(void *_pMem, mint &_NewSize)
		{
			void *pNewMem = f_Alloc(_NewSize);
			if (!pNewMem)
			{
				f_Free(_pMem);
				return nullptr;
			}

			if (_pMem)
			{
				mint OldSize = f_Size(_pMem);
				fg_MemCopy(pNewMem, _pMem, OldSize < _NewSize ? OldSize : _NewSize);
				f_Free(_pMem);
			}
			return pNewMem;
		}

		template <class t_CHeapParams>
		inline_medium void *TCHeap<t_CHeapParams>::fp_Resize(CChunk *_pChunk, void *_pMem, mint &_NewSize)
		{
			void *pNewMem = f_Alloc(_NewSize);
			if (!pNewMem)
			{
				fp_Free(_pChunk, _pMem);
				return nullptr;
			}

			if (_pMem)
			{
				mint _OldSize = f_Size(_pMem);
				fg_MemCopy(pNewMem, _pMem, _OldSize < _NewSize ? _OldSize : _NewSize);
				fp_Free(_pChunk, _pMem);
			}
			return pNewMem;
		}

		template <class t_CHeapParams>
		inline_medium mint TCHeap<t_CHeapParams>::f_Size(const void *_pMem)
		{
			CBlock *pBlock = (CBlock *)(((uint8 *)_pMem) - EBlockPreSize);
			return pBlock->f_GetSizeNormal() - EBlockWholeSize;
		}

		template <class t_CHeapParams>
		inline_medium fp32 TCHeap<t_CHeapParams>::f_Overhead(const void *_pMem)
		{
			return fp32(EBlockWholeSize);
		}

		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::f_GetExtraData(void *_pMem)
		{
			return ((uint8 *)_pMem - EBlockExtraOffset);
		}


	}
}
