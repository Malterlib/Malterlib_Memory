// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Block bucket																						|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		inline_small typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_GetBlockBucket(mint _Size)
		{
			if (EFreeSizeBucketTreeThresholdBits > 0 && _Size <= 1 << EFreeSizeBucketTreeThresholdBits)
			{
				// Do a bucket search
				if (t_CHeapParams::EAccurateBucketCache)
				{
					int iStartBucket = (_Size >> EAlignBits) - 1;
					for (int i = iStartBucket; i < int(EFreeSizeBucketTreeThresholdListSize); ++i)
					{
						if (m_pFreeBuckets[i])
							return m_pFreeBuckets[i];
					}
				}
				else
				{
					int iIndex =  (_Size >> EAlignBits) - 1;
					if (m_pFreeBuckets[iIndex])
						return m_pFreeBuckets[iIndex];
				}

				return m_FreeBlockBucketsTree.f_FindSmallestGreaterThanEqual(_Size);
			}
			else
				return m_FreeBlockBucketsTree.f_FindSmallestGreaterThanEqual(_Size);
		}

		template <class t_CHeapParams>
		inline_small typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_GetBlockBucket(mint _Size, mint &_PrevSize)
		{
			if (EFreeSizeBucketTreeThresholdBits > 0 && _Size <= 1 << EFreeSizeBucketTreeThresholdBits)
			{
				if (t_CHeapParams::EAccurateBucketCache)
				{
					int iStartBucket = (_Size >> EAlignBits) - 1;
					for (int i = iStartBucket; i < int(EFreeSizeBucketTreeThresholdListSize); ++i)
					{
						if (m_pFreeBuckets[i])
						{
							return m_pFreeBuckets[i];
						}
					}
				}
				else
				{
					int iIndex =  (_Size >> EAlignBits) - 1;
					if (m_pFreeBuckets[iIndex])
					{
						return m_pFreeBuckets[iIndex];
					}
				}
			}

			CFreeBlockBucket *pPrev = nullptr;
			CFreeBlockBucket *pRet = m_FreeBlockBucketsTree.f_FindSmallestGreaterThanEqualAndPrev(_Size, pPrev);
			if (!pRet)
				return nullptr;

/*				CFreeBlockBucket *pPrevCheck = m_FreeBlockBucketsTree.f_FindLargestLessThanEqual(_Size-1);

			if (pPrevCheck != pPrev)
				DMibPDebugBreak;*/
			if (pPrev)
				_PrevSize = pPrev->f_GetSize();
			else
				_PrevSize = 0;

			return pRet;
		}

		template <class t_CHeapParams>
		inline_small typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_GetBlockBucket(mint _Size, CFreeBlockBucket *&_pPrev)
		{
			if (EFreeSizeBucketTreeThresholdBits > 0 && _Size <= 1 << EFreeSizeBucketTreeThresholdBits)
			{
				if (t_CHeapParams::EAccurateBucketCache)
				{
					int iStartBucket = (_Size >> EAlignBits) - 1;
					for (int i = iStartBucket; i < int(EFreeSizeBucketTreeThresholdListSize); ++i)
					{
						if (m_pFreeBuckets[i])
						{
							return m_pFreeBuckets[i];
						}
					}
				}
				else
				{
					int iIndex =  (_Size >> EAlignBits) - 1;
					if (m_pFreeBuckets[iIndex])
					{
						return m_pFreeBuckets[iIndex];
					}
				}
			}

			CFreeBlockBucket *pRet = m_FreeBlockBucketsTree.f_FindSmallestGreaterThanEqualAndPrev(_Size, _pPrev);
			return pRet;
		}

		template <class t_CHeapParams>
		inline_small typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_GetBlockBucketExact(mint _Size)
		{
			if (EFreeSizeBucketTreeThresholdBits > 0 && _Size <= 1 << EFreeSizeBucketTreeThresholdBits)
			{
				return m_pFreeBuckets[(_Size >> EAlignBits) - 1];
			}
			else
				return m_FreeBlockBucketsTree.f_FindEqual(_Size);
		}


	}
}
