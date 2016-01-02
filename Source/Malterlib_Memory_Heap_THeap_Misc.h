// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{

		class CTCHeap_SizeHolderSmall
		{
		public:
			static mint m_Size;
			enum
			{
				EStoresSize = false
			};
		};

		class CTCHeap_SizeHolderFast
		{
		public:
            mint m_Size;
			enum
			{
				EStoresSize = true
			};
		};

		class CHeapBlock8Bit
		{
		public:
			enum 
			{
				 EUseableBits = 7
				,ENeedExtendedBlocks = 1
			};
			uint16 m_Bits;
			inline_small aint f_Next() const
			{
				return m_Bits&0x7f;
			}
			inline_small aint f_Prev() const
			{
				return (m_Bits&0x3f80) >> 7;
			}
			inline_small aint f_Type() const
			{
				return (m_Bits&0xc000) >> 14;
			}
			inline_small void f_SetType(aint _Type)
			{
				m_Bits = (m_Bits & (~0xc000)) | _Type << 14;
			}
			inline_small void f_SetNext(aint _Next)
			{
				m_Bits = (m_Bits & (~0x7f)) | _Next;
			}
			inline_small void f_SetPrev(aint _Prev)
			{
				m_Bits = (m_Bits & (~0x3f80)) | _Prev << 7;
			}
			inline_small void f_SetAll(aint _Next, aint _Prev, aint _Type)
			{
				m_Bits = _Next | _Prev << 7 | _Type << 14;
			}

		};

		class CHeapBlock16Bit
		{
		public:
			enum 
			{
				 EUseableBits = 15
				,ENeedExtendedBlocks = 1
			};
			uint32 m_Bits;
			inline_small aint f_Next() const
			{
				return m_Bits&0x7fff;
			}
			inline_small aint f_Prev() const
			{
				return (m_Bits&0x3fff8000) >> 15;
			}
			inline_small aint f_Type() const
			{
				return (m_Bits&0xc0000000) >> 30;
			}
			inline_small void f_SetType(aint _Type)
			{
				m_Bits = (m_Bits & (~0xc0000000)) | _Type << 30;
			}
			inline_small void f_SetNext(aint _Next)
			{
				m_Bits = (m_Bits & (~0x7fff)) | _Next;
			}
			inline_small void f_SetPrev(aint _Prev)
			{
				m_Bits = (m_Bits & (~0x3fff8000)) | _Prev << 15;
			}
			inline_small void f_SetAll(aint _Next, aint _Prev, aint _Type)
			{
				m_Bits = _Next | _Prev << 15 | _Type << 30;
			}

		};

		class CHeapBlockArchSize
		{
		public:
			enum 
			{
				 EUseableBits = (sizeof(void *) * 8) - 1
				,ENeedExtendedBlocks = 0
			};
			mint m_BitsNext;
			mint m_BitsPrev;
			inline_small aint f_Next() const
			{
				return m_BitsNext >> 1;
			}
			inline_small aint f_Prev() const
			{
				return m_BitsPrev >> 1;
			}
			inline_small aint f_Type() const
			{
				return (m_BitsNext & 1) | ((m_BitsPrev & 1) << 1);
			}
			inline_small void f_SetType(aint _Type)
			{
				m_BitsNext = (m_BitsNext & (~1)) | (_Type & 1);
				m_BitsPrev = (m_BitsPrev & (~1)) | (_Type >> 1);
			}
			inline_small void f_SetNext(aint _Next)
			{
				m_BitsNext = (m_BitsNext & 1) | (_Next << 1);
			}
			inline_small void f_SetPrev(aint _Prev)
			{
				m_BitsPrev = (m_BitsPrev & 1) | (_Prev << 1);
			}
			inline_small void f_SetAll(aint _Next, aint _Prev, aint _Type)
			{
				m_BitsNext = (_Next << 1) | (_Type & 1);
				m_BitsPrev = (_Prev << 1) | (_Type >> 1);
			}
		};

		class CHeapDefaultParams
		{
		public:
			const static EAllocationFlag mc_AllocationFlags = EAllocationFlag_None;
			enum 
			{
				 EGrowSize = 4096*1024
				,ENumChunkFreeThreshold = 2
				,EAlignBits = DMibGetHighestBitSet(sizeof(mint)*2)
				,EExtraSpace = 0
				,EGuardPreSize = 0
				,EGuardPostSize = 0
				,EbOptimizeForSize = 0
				,EBlockCacheSize = 64 + sizeof(mint)
				,ECacheFreeThreshold = 256
				,EFreeSizeBucketTreeThresholdBits = 9
				,ETreeOpt0 = 1
				,ETreeOpt1 = 1
				,ETreeOpt2 = 1
				,ETreeOpt3 = 1
				,EAccurateBucketCache = 0
				,ELargePages = 0
			};
			typedef CAllocator_Virtual CAllocator;
			typedef CHeap_FillNoDebug CFillDebug;
			typedef CTCHeap_SizeHolderFast CSizeHolder;
			typedef CHeapBlockArchSize CBlock;
			typedef NMib::NThread::CNoLock CLock;
		};

		template <mint t_GrowSize = 4096*1024, typename t_CAllocator = CAllocator_VirtualNoCommit, mint t_NumChunkFreeThreshold = 2, mint t_AlignBits = DMibGetHighestBitSet(sizeof(mint)*2), typename t_CLock = NMib::NThread::CNoLock>
		class TCHeapParams : public CHeapDefaultParams
		{
		public:
			enum 
			{
				 EGrowSize = t_GrowSize
				,ENumChunkFreeThreshold = t_NumChunkFreeThreshold
				,EAlignBits = t_AlignBits
			};
			typedef t_CAllocator CAllocator;
			typedef CHeap_FillNoDebug CFillDebug;
			typedef CTCHeap_SizeHolderFast CSizeHolder;
			typedef t_CLock CLock;
		};

		template <mint t_GrowSize = 4096*1024, typename t_CAllocator = CAllocator_VirtualNoCommit, mint t_NumChunkFreeThreshold = 2, mint t_AlignBits = DMibGetHighestBitSet(sizeof(mint)*2), typename t_CLock = NMib::NThread::CNoLock, mint t_ExtraSpace = 0, mint t_GuardPreSize = 0, mint t_GuardPostSize = 0>
		class TCHeapParamsDebug : public CHeapDefaultParams
		{
		public:
			enum 
			{
				 EGrowSize = t_GrowSize
				,ENumChunkFreeThreshold = t_NumChunkFreeThreshold
				,EAlignBits = t_AlignBits
				,EExtraSpace = t_ExtraSpace
				,EGuardPreSize = t_GuardPreSize
				,EGuardPostSize = t_GuardPostSize
			};
			typedef t_CAllocator CAllocator;
			typedef CHeap_FillDebug CFillDebug;
			typedef CTCHeap_SizeHolderFast CSizeHolder;
			typedef t_CLock CLock;
		};
	}
}
