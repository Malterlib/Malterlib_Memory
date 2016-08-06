// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{

		class CCCompareAVL_CHeapChunk;

		/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*\
		|	Class:				A virtual class to enable a tree to be					|
		|																				|
		|	Comments:			Longer_description_not_mandatory						|
		\*_____________________________________________________________________________*/
		template <typename t_CAllocator>
		class TCHeapChunk
		{
		public:
			class CCompareAVL
			{
			public:
				inline_small void const *operator () (TCHeapChunk const &_Node) const
				{
					return _Node.m_pBase;
				}
			};

			DMibIntrusiveLinkT(TCHeapChunk, NIntrusive::TCAVLLink<NIntrusive::EAVLLinkType_Unaligned>, m_ChunkTree);

			typedef NIntrusive::TCAVLTree<CLinkTraits_m_ChunkTree, CCompareAVL> CTree;
			typedef typename CTree::CIterator CIterator;

			virtual ~TCHeapChunk(){}

			void *m_pBase;
			mint m_Size;
			// Aligned on 2 byte boundrary min size 4+4+8 = 16 bytes
			virtual void f_Destroy(CTree &_Tree)
			{
				_Tree.f_Remove(this);
			}

			virtual mint f_Size(const void *_pBlock) = 0;
			virtual mint f_SizeAndHeap(const void *_pBlock, void * &_pHeap) = 0;
			virtual fp32 f_Overhead(void const *_pBlock) = 0;
			virtual void f_Free(void *_pBlock) = 0;
			virtual void* f_Realloc(void *_pBlock, mint &_NewSize) = 0;
			virtual void* f_Resize(void *_pBlock, mint &_NewSize) = 0;
			virtual void* f_GetExtraData(void *_pBlock) = 0;
			virtual bint f_ContainsBlock(void *_pBlock) = 0;
			virtual void* f_GetHeapIdent() const = 0;
		};


		class CHeap_FillNoDebug
		{
		public:
			enum
			{
				EDoFills = false
			};
			static inline_small void fs_FillFree(void *_pData, mint _Size)
			{
			}
			static inline_small void fs_FillGuard(void *_pData, mint _Size)
			{
			}
			static inline_small void fs_FillAllocated(void *_pData, mint _Size)
			{
			}

			static inline_small void fs_CheckFree(void *_pData, mint _Size)
			{
			}

			static inline_small void fs_CheckPreGuard(void *_pData, mint _Size)
			{
			}
			static inline_small void fs_CheckPostGuard(void *_pData, mint _Size)
			{
			}
		};

		enum EMemoryFill
		{
			 EMemoryFill_Guard		= 0xFD
			,EMemoryFill_Free		= 0xDD
			,EMemoryFill_Allocated	= 0xCD
		};

		class CHeap_FillDebug
		{
		public:
			enum
			{
				EDoFills = true
			};
			static bool ms_bDisableFillChecks;
			
			static void fs_ReportDamage(const ch8 *_pMemoryType, void *_pMem, uint8 _Value, uint8 _MemoryFill);

			static inline_small void fs_FillFree(void *_pData, mint _Size)
			{
				if (ms_bDisableFillChecks)
					return;
				fg_ObjectSet((uint8 *)_pData, EMemoryFill_Free, _Size);
			}
			static inline_small void fs_FillGuard(void *_pData, mint _Size)
			{
				if (ms_bDisableFillChecks)
					return;
				fg_ObjectSet((uint8 *)_pData, EMemoryFill_Guard, _Size);
			}
			static inline_small void fs_FillAllocated(void *_pData, mint _Size)
			{
				if (ms_bDisableFillChecks)
					return;
				fg_ObjectSet((uint8 *)_pData, EMemoryFill_Allocated, _Size);
			}

			static inline_small void fs_CheckFree(void *_pData, mint _Size)
			{
				if (ms_bDisableFillChecks)
					return;
				if (fg_MemCmpOne((uint8 *)_pData, EMemoryFill_Free, _Size) != 0)
				{
					// We have a heap overwrite

					uint8 *pFirst = (uint8 *)_pData;
					uint8 *pFirstEnd = pFirst + _Size;
					
					while (pFirst < pFirstEnd)
					{
						if ((*pFirst) != EMemoryFill_Free)
						{
							fs_ReportDamage("In freed memory", pFirst, *pFirst, EMemoryFill_Free);
							(*pFirst) = EMemoryFill_Free;
						}
						++pFirst;
					}
				}
			}

			static inline_small void fs_CheckPreGuard(void *_pData, mint _Size)
			{
				if (ms_bDisableFillChecks)
					return;
				if (fg_MemCmpOne((uint8 *)_pData, EMemoryFill_Guard, _Size) != 0)
				{
					// We have a heap overwrite

					uint8 *pFirst = (uint8 *)_pData;
					uint8 *pFirstEnd = pFirst + _Size;
					
					while (pFirst < pFirstEnd)
					{
						if ((*pFirst) != EMemoryFill_Guard)
						{
							fs_ReportDamage("Before allocted block", pFirst, *pFirst, EMemoryFill_Guard);
							(*pFirst) = EMemoryFill_Guard;
						}
						++pFirst;
					}
				}
			}

			static inline_small void fs_CheckPostGuard(void *_pData, mint _Size)
			{
				if (ms_bDisableFillChecks)
					return;
				if (fg_MemCmpOne((uint8 *)_pData, EMemoryFill_Guard, _Size) != 0)
				{
					// We have a heap overwrite

					uint8 *pFirst = (uint8 *)_pData;
					uint8 *pFirstEnd = pFirst + _Size;
					
					while (pFirst < pFirstEnd)
					{
						if ((*pFirst) != EMemoryFill_Guard)
						{
							fs_ReportDamage("After allocted block", pFirst, *pFirst, EMemoryFill_Guard);
							(*pFirst) = EMemoryFill_Guard;
						}
						++pFirst;
					}
				}
			}
		};

	}
}
