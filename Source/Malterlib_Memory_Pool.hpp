// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_Growing::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::TCChunk
		(
			DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) &_Blocks
			, umint _Size
		)
	{
		m_Size = _Size;

		uint8 *pStart = fg_AlignUp((uint8 *)(this + 1), t_Alignment);
		uint8 *pEnd = (uint8 *)this + _Size;
		umint nBlocks = (pEnd - pStart) / mc_DataSize;

		uint8 *pEndBlock = pStart;
		uint8 *pCurrentBlock = pStart + (nBlocks - 1) * mc_DataSize;

		while (pCurrentBlock >= pEndBlock)
		{
			TCBlock<t_CAllocator> *pToInsert = (TCBlock<t_CAllocator> *)pCurrentBlock;
			_Blocks.f_UnsafeInsertFirst(pToInsert);
			pCurrentBlock -= mc_DataSize;
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_Growing::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::~TCChunk()
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	constexpr CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::TCPool(EAggregateInitialization _Init)
		: m_Chunks(_Init)
		, m_FreeBlocks(_Init)
#ifdef DMibDebug
		, m_NumUsed(0)
#endif
		, m_NumaNode(ENumaNode_Default)
		, m_GrowSize(0)
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::TCPool()
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Construct(ENumaNode _NumaNode, umint _GrowSize)
	{
		m_NumaNode = _NumaNode;
		m_GrowSize = _GrowSize;
		m_Chunks.f_Construct();
		m_FreeBlocks.f_Construct();
#ifdef DMibDebug
		m_NumUsed = 0;
#endif
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::fp_AddChunk(void *_pMem, umint _Size)
	{
		CChunk *pNewChunk = new(_pMem) CChunk(m_FreeBlocks, _Size);
		m_Chunks.f_Insert(pNewChunk);
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void *CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_GetBlock()
	{
		TCBlock<t_CAllocator> *pBlock = m_FreeBlocks.f_Pop();

		if (pBlock)
		{
#ifdef DMibDebug
			++m_NumUsed;
#endif
			return pBlock;
		}
		else
		{
			umint Size = ((((sizeof(CChunk) + m_GrowSize * mc_DataSize) - 1) / t_CAllocator::f_GranularityAlloc()) + 1) * t_CAllocator::f_GranularityAlloc();
			void *pMem = t_CAllocator::f_AllocWithSize(Size, EAllocationFlag_WillFreeWithSize, m_NumaNode);

			if (pMem)
			{
				fp_AddChunk(pMem, Size);
				pBlock = m_FreeBlocks.f_Pop();
				DMibFastCheck(pBlock); // Must succed
#ifdef DMibDebug
				++m_NumUsed;
#endif
				return pBlock;
			}
			else
				return nullptr;
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::fs_FreeChunk(CChunk *_pChunk)
	{
		t_CAllocator::f_Free(_pChunk, _pChunk->m_Size);
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline fp32 CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Overhead() const
	{
		if constexpr (mc_DataSize > t_DataSize)
			return fp32(mc_DataSize - t_DataSize);
		else
			return 0.0f;
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_ReturnBlock(void *_pBlock)
	{
		TCBlock<t_CAllocator> *pBlock = (TCBlock<t_CAllocator> *)_pBlock;
		// We are adding to a full block
		m_FreeBlocks.f_UnsafePush(pBlock);
#ifdef DMibDebug
		--m_NumUsed;
#endif
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_Freeable::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::TCChunk(umint _Size)
	{
		m_Size = _Size;
		m_NumUsed = 0;

		uint8 *pStart = fg_AlignUp((uint8 *)(this + 1), t_Alignment);
		uint8 *pEnd = (uint8 *)this + _Size;
		umint nBlocks = (pEnd - pStart) / mc_DataSize;

		uint8 *pEndBlock = pStart;
		uint8 *pCurrentBlock = pStart + (nBlocks - 1) * mc_DataSize;

		while (pCurrentBlock >= pEndBlock)
		{
			TCBlock<t_CAllocator> *pToInsert = (TCBlock<t_CAllocator> *)pCurrentBlock;
			m_FreeBlocks.f_UnsafeInsertFirst(pToInsert);
			pCurrentBlock -= mc_DataSize;
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_Freeable::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::~TCChunk()
	{
		//DMibFastCheck(m_NumUsed == 0); // Memory leak in pool!
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	constexpr CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::TCPool(EAggregateInitialization _Init)
		: m_Chunks(_Init)
		, m_FreeChunks(_Init)
		, m_EmptyChunks(_Init)
		, m_NumaNode(ENumaNode_Default)
		, m_GrowSize(0)
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::TCPool()
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Construct(ENumaNode _NumaNode, umint _GrowSize)
	{
		m_NumaNode = _NumaNode;
		m_GrowSize = _GrowSize;
		m_Chunks.f_Construct();
		m_FreeChunks.f_Construct();
		m_EmptyChunks.f_Construct();
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline bool CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_ContainsBlock(void *_pBlock)
	{
		DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *Lists[] = {&m_Chunks, &m_FreeChunks, &m_EmptyChunks};

		for (umint iList = 0; iList < sizeof(Lists) / sizeof(DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *); ++iList)
		{
			DMibListLinkAllocatorD_Iter_FromTemplate(CChunk, m_Link, t_CAllocator) Iter = *Lists[iList];
			while (Iter)
			{
				if ((umint)_pBlock >= (umint)(CChunk *)Iter && (umint)_pBlock < (umint)(CChunk *)Iter + Iter->m_Size)
					return true;
				++Iter;
			}
		}

		return false;
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void *CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_GetBlock()
	{
		// DMibListLinkD_Iter(TCChunk<>, m_Link) Iter = m_FreeChunks;

		CChunk *pChunk = m_FreeChunks.f_GetFirst();

		TCBlock<t_CAllocator> *pBlock = nullptr;

		if (pChunk)
			pBlock = pChunk->m_FreeBlocks.f_Pop();
		else
		{
			pChunk = m_EmptyChunks.f_GetFirst();
			if (pChunk)
				pBlock = pChunk->m_FreeBlocks.f_Pop();
		}

		if (pBlock)
		{
			if (pChunk->m_FreeBlocks.f_IsEmpty())
			{
				// Full block
				m_Chunks.f_Insert(pChunk);
			}
			else if (pChunk->m_NumUsed == 0)
				m_FreeChunks.f_Insert(pChunk);
			++pChunk->m_NumUsed;
			pBlock->m_pChunk = pChunk;
			return ((uint8 *)pBlock) + mc_BlockSize;
		}
		else
		{
			umint Size = ((((sizeof(CChunk) + m_GrowSize * mc_DataSize) - 1) / t_CAllocator::f_GranularityAlloc()) + 1) * t_CAllocator::f_GranularityAlloc();
			void *pMem = t_CAllocator::f_AllocWithSize(Size, EAllocationFlag_WillFreeWithSize, m_NumaNode);

			if (pMem)
			{
				CChunk *pNewChunk = new(pMem) CChunk(Size);

				pBlock = pNewChunk->m_FreeBlocks.f_Pop();
				DMibFastCheck(pBlock); // Must succed
				m_FreeChunks.f_Insert(pNewChunk);

				++pNewChunk->m_NumUsed;
				pBlock->m_pChunk = pNewChunk;
				return ((uint8 *)pBlock) + mc_BlockSize;
			}
			else
				return nullptr;
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::fs_FreeChunk(CChunk *_pChunk)
	{
		t_CAllocator::f_Free(_pChunk, _pChunk->m_Size);
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline fp32 CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Overhead() const
	{
		return fp32(mc_BlockSize);
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_ReturnBlock(void *_pBlock)
	{
		// Too slow to check this
		// 	DMibFastCheck(f_ContainsBlock(_pBlock), "Must be part of pool");
		TCBlock<t_CAllocator> *pBlock = (TCBlock<t_CAllocator> *)((uint8 *)_pBlock - mc_BlockSize);
		CChunk *pChunk = ((CChunk *)(pBlock->m_pChunk));
		if (pChunk->m_FreeBlocks.f_IsEmpty())
		{
			// We are adding to a full block
			pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
			--pChunk->m_NumUsed;
			m_FreeChunks.f_InsertFirst(pChunk);
		}
		else
		{
			pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
			--pChunk->m_NumUsed;

			if (pChunk->m_NumUsed == 0)
			{
				if (!m_EmptyChunks.f_IsEmpty())
				{
					// Remove the excess chunk
					CChunk *pChunkToDelete = m_EmptyChunks.f_Pop();

					pChunkToDelete->~CChunk();
					fs_FreeChunk(pChunkToDelete);
				}
				m_EmptyChunks.f_Insert(pChunk);
			}
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void const *CPoolType_FreeableSmall::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::CCompare::operator () (TCChunk const &_Node) const
	{
		return (void const *)(&_Node);
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_FreeableSmall::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::TCChunk(umint _Size)
	{
		m_Size = _Size;
		m_NumUsed = 0;

		uint8 *pStart = fg_AlignUp((uint8 *)(this + 1), t_Alignment);
		uint8 *pEnd = (uint8 *)this + _Size;
		umint nBlocks = (pEnd - pStart) / mc_DataSize;

		uint8 *pEndBlock = pStart;
		uint8 *pCurrentBlock = pStart + (nBlocks - 1) * mc_DataSize;

		while (pCurrentBlock >= pEndBlock)
		{
			TCBlock<t_CAllocator> *pToInsert = (TCBlock<t_CAllocator> *)pCurrentBlock;
			m_FreeBlocks.f_UnsafeInsertFirst(pToInsert);
			pCurrentBlock -= mc_DataSize;
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_FreeableSmall::TCChunk<t_CAllocator, t_DataSize, t_Alignment>::~TCChunk()
	{
		//DMibFastCheck(m_NumUsed == 0); // Memory leak in pool!
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	constexpr CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::TCPool(EAggregateInitialization _Init)
		: m_Chunks(_Init)
		, m_FreeChunks(_Init)
		, m_EmptyChunks(_Init)
		, m_ChunkTree(_Init)
		, m_NumaNode(ENumaNode_Default)
		, m_GrowSize(0)
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::TCPool()
	{
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Construct(ENumaNode _NumaNode, umint _GrowSize)
	{
		m_NumaNode = _NumaNode;
		m_GrowSize = _GrowSize;
		m_Chunks.f_Construct();
		m_FreeChunks.f_Construct();
		m_EmptyChunks.f_Construct();
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline bool CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_ContainsBlock(void *_pBlock)
	{
		DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *Lists[] = {&m_Chunks, &m_FreeChunks, &m_EmptyChunks};

		for (umint iList = 0; iList < sizeof(Lists) / sizeof(DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *); ++iList)
		{
			DMibListLinkAllocatorD_Iter_FromTemplate(CChunk, m_Link, t_CAllocator) Iter = *Lists[iList];
			while (Iter)
			{
				if ((umint)_pBlock >= (umint)(CChunk *)Iter && (umint)_pBlock < (umint)(CChunk *)Iter + Iter->m_Size)
					return true;
				++Iter;
			}
		}

		return false;
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline fp32 CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Overhead() const
	{
		if constexpr (mc_DataSize > t_DataSize)
			return fp32(mc_DataSize - t_DataSize);
		else
			return 0.0f;
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void *CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_GetBlock()
	{
		// DMibListLinkD_Iter(TCChunk<>, m_Link) Iter = m_FreeChunks;

		CChunk *pChunk = m_FreeChunks.f_GetFirst();

		TCBlock<t_CAllocator> *pBlock = nullptr;

		if (pChunk)
			pBlock = pChunk->m_FreeBlocks.f_Pop();

		if (!pBlock)
		{
			pChunk = m_EmptyChunks.f_GetFirst();
			if (pChunk)
				pBlock = pChunk->m_FreeBlocks.f_Pop();
		}

		if (pBlock)
		{
			if (pChunk->m_FreeBlocks.f_IsEmpty())
			{
				// Full block
				m_Chunks.f_Insert(pChunk);
			}
			else if (pChunk->m_NumUsed == 0)
				m_FreeChunks.f_Insert(pChunk);
			++pChunk->m_NumUsed;
			return pBlock;
		}
		else
		{
			umint Size = ((((sizeof(CChunk) + m_GrowSize * mc_DataSize) - 1) / t_CAllocator::f_GranularityAlloc()) + 1) * t_CAllocator::f_GranularityAlloc();
			void *pMem = t_CAllocator::f_AllocWithSize(Size, EAllocationFlag_WillFreeWithSize, m_NumaNode);

			if (pMem)
			{
				CChunk *pNewChunk = new(pMem) CChunk(Size);

				m_ChunkTree.f_Insert(pNewChunk);

				pBlock = pNewChunk->m_FreeBlocks.f_Pop();
				DMibFastCheck(pBlock); // Must succed
				m_FreeChunks.f_Insert(pNewChunk);

				++pNewChunk->m_NumUsed;

				return pBlock;
			}
			else
				return nullptr;
		}
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::fs_FreeChunk(CChunk *_pChunk)
	{
		t_CAllocator::f_Free(_pChunk, _pChunk->m_Size);
	}

	template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
	inline void CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_ReturnBlock(void *_pBlock)
	{
		TCBlock<t_CAllocator> *pBlock = (TCBlock<t_CAllocator> *)_pBlock;
		CChunk *pChunk = m_ChunkTree.f_FindLargestLessThanEqual(_pBlock);
		DMibFastCheck(pChunk && (umint)_pBlock < (umint)pChunk + pChunk->m_Size); // Must be part of pool
		if (pChunk->m_FreeBlocks.f_IsEmpty())
		{
			// We are adding to a full block
			pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
			--pChunk->m_NumUsed;
			m_FreeChunks.f_InsertFirst(pChunk);
		}
		else
		{
			pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
			--pChunk->m_NumUsed;

			if (pChunk->m_NumUsed == 0)
			{
				if (!m_EmptyChunks.f_IsEmpty())
				{
					// Remove the excess chunk
					CChunk *pChunkToDelete = m_EmptyChunks.f_Pop();
					m_ChunkTree.f_Remove(pChunkToDelete);
					pChunkToDelete->~CChunk();
					fs_FreeChunk(pChunkToDelete);
				}
				m_EmptyChunks.f_Insert(pChunk);
			}
		}
	}

	template <typename t_CLock>
	constexpr TCPoolAggregateData<t_CLock>::TCPoolAggregateData(EAggregateInitialization _Init)
		: t_CLock(_Init)
		, m_bDoneInit(false)
	{
	}

	template <typename t_CLock>
	inline TCPoolAggregateData<t_CLock>::TCPoolAggregateData()
	{
	}

	template <typename t_CLock>
	inline void TCPoolAggregateData<t_CLock>::f_Construct()
	{
		// Hide lock construct
	}

	template <typename t_CLock>
	inline void TCPoolAggregateData<t_CLock>::f_InitDone()
	{
		m_bDoneInit = true;
	}

	template <typename t_CLock>
	inline bool TCPoolAggregateData<t_CLock>::f_ShouldInit()
	{
		if (m_bDoneInit)
			return false;
		else
			return true;
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	constexpr TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::TCPoolAggregate(EAggregateInitialization _Init)
		: t_CAggregateData(_Init)
#if DMibConfig_Memory_Shims_Enable
		, m_DebugName()
#endif
		, m_Pool(_Init)
	{
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::TCPoolAggregate()
	{
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	constexpr bool TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::operator == (TCPoolAggregate const &_Right) const noexcept
	{
		return this == &_Right;
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	constexpr auto TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::operator <=> (TCPoolAggregate const &_Right) const noexcept
	{
		return this <=> &_Right;
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline void TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_Destruct()
	{
		DMibMemoryGoingToReportScope(this, mc_Reporting);
		if (!t_CAggregateData::f_ShouldInit())
		{
#ifdef DMibDebug
			m_Pool.f_Destruct(fg_GetTypeName<t_CData>());
#else
			m_Pool.f_Destruct("Unknown");
#endif
		}
		t_CAggregateData::f_Destruct();
#if DMibConfig_Memory_Shims_Enable
		if constexpr (mc_Reporting)
			DMibMemoryReportAllocatorDelete(this, m_DebugName.f_GetStr());
#endif
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline fp32 TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_Overhead(void const *_pBlock)
	{
		DMibLockTyped(t_CLock, *this);
		return m_Pool.f_Overhead();
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline void *TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_GetBlock()
	{
		return fp_GetBlock();
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline void TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_ReturnBlock(void *_pToDelete)
	{
		fp_ReturnBlock(_pToDelete);
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline void TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_Delete(t_CData *_pToDelete)
	{
		_pToDelete->~t_CData();
		fp_ReturnBlock(_pToDelete);
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	template <typename ...tfp_CParams>
	inline t_CData *TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_New(tfp_CParams && ...p_Params)
	{
		void *pMem;
		pMem = fp_GetBlock();
		t_CData *pObject = new(pMem) t_CData(fg_Forward<tfp_CParams>(p_Params)...);
		return pObject;
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline void TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::fp_ReturnBlock(void *_pBlock)
	{
		DMibMemoryGoingToReportScope(this, mc_Reporting);
		{
			DMibLockTyped(t_CLock, *this);
			m_Pool.f_ReturnBlock(_pBlock);
		}
#if DMibConfig_Memory_Shims_Enable
		if constexpr (mc_Reporting)
			DMibMemoryReportFree(this, m_DebugName.f_GetStr(), _pBlock, sizeof(t_CData), nullptr);
#endif
	}

	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	inline void *TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::fp_GetBlock()
	{
		DMibMemoryGoingToReportScope(this, mc_Reporting);

		DMibMemoryReportExpression(fp32 Overhead);
		void *pBlock;
		{
			DMibLockTyped(t_CLock, *this);
			if (t_CAggregateData::f_ShouldInit())
				f_Construct(ENumaNode_Default);

			pBlock = m_Pool.f_GetBlock();
			DMibFastCheck(pBlock); // Memory error ??
			DMibMemoryReportExpression(Overhead = m_Pool.f_Overhead());
		}

#if DMibConfig_Memory_Shims_Enable
		if constexpr (mc_Reporting)
			DMibMemoryReportAlloc(this, m_DebugName.f_GetStr(), pBlock, 0, sizeof(t_CData), sizeof(t_CData), Overhead, nullptr);
#endif
		return pBlock;
	}

	template <typename t_CLock>
	inline void TCNoAggregateData<t_CLock>::f_InitDone()
	{
	}

	template <typename t_CLock>
	inline bool TCNoAggregateData<t_CLock>::f_ShouldInit()
	{
		return false;
	}

	template <typename t_CData, umint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator>
	inline TCPool<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator>::TCPool(ENumaNode _NumaNode)
	{
		TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, TCNoAggregateData<t_CLock>>::f_Construct(_NumaNode);
	}

	template <typename t_CData, umint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator>
	inline TCPool<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator>::~TCPool()
	{
		TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, TCNoAggregateData<t_CLock>>::f_Destruct();
	}

	template <typename t_CData, umint t_GrowSize, typename t_CLock, typename t_CAllocator>
	inline TCPoolSmall<t_CData, t_GrowSize, t_CLock, t_CAllocator>::TCPoolSmall(ENumaNode _NumaNode)
	{
		TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_FreeableSmall, t_CAllocator, TCNoAggregateData<t_CLock>>::f_Construct(_NumaNode);
	}

	template <typename t_CData, umint t_GrowSize, typename t_CLock, typename t_CAllocator>
	inline TCPoolSmall<t_CData, t_GrowSize, t_CLock, t_CAllocator>::~TCPoolSmall()
	{
		TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_FreeableSmall, t_CAllocator, TCNoAggregateData<t_CLock>>::f_Destruct();
	}

	template <typename t_CData, umint t_GrowSize, typename t_CLock, typename t_CAllocator>
	inline TCPoolGrowing<t_CData, t_GrowSize, t_CLock, t_CAllocator>::TCPoolGrowing(ENumaNode _NumaNode)
	{
		TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_Growing, t_CAllocator, TCNoAggregateData<t_CLock>>::f_Construct(_NumaNode);
	}

	template <typename t_CData, umint t_GrowSize, typename t_CLock, typename t_CAllocator>
	inline TCPoolGrowing<t_CData, t_GrowSize, t_CLock, t_CAllocator>::~TCPoolGrowing()
	{
		TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_Growing, t_CAllocator, TCNoAggregateData<t_CLock>>::f_Destruct();
	}

	template <typename t_CPool>
	inline void *TCBCPool<t_CPool>::operator new (umint _NumBytes, t_CPool const &_Pool)
	{
		return _Pool.f_GetBlock(_NumBytes);
	}

	template <typename t_CPool>
	inline void TCBCPool<t_CPool>::operator delete (void *_pToDelete, t_CPool const &_Pool) noexcept
	{
		_Pool.f_ReturnBlock(_pToDelete);
	}

	template <typename t_CPool>
	inline void TCBCPool<t_CPool>::operator delete (void *_pToDelete)
	{
		// We do nothing here, as we dont know the pool
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constexpr bool TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::operator == (TCStaticPoolAllocator const &_Right) const noexcept
	{
		return true;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constexpr auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::operator <=> (TCStaticPoolAllocator const &_Right) const noexcept
	{
		return COrdering_Strong::equal;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_StaticAddresses()
	{
		return 0;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_GranularityAlloc(bool _bLargePages)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_GranularityCommit(bool _bLargePages)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_GranularityProtect(bool _bLargePages)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Size(void *_pBlock)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_TrySize(void *_pBlock)
	{
		DMibPDebugBreak; // Not supported
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_SizePadded(umint _Size)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constexpr bool TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_CanCommit()
	{
		return false;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline bool TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_CanProtect()
	{
		return false;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline bool TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_DeterministicSize()
	{
		return true;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocWithSizeDebug
		(
			umint _Size
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void *TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocAlignedWithSize
		(
			umint _Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocDebug
		(
			umint _Size
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void *TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocAligned
		(
			umint _Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocBatch
		(
			umint _Size
			, umint _Alignment
			, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		while (true)
		{
			umint Size = _Size;
			void *pMem = CStaticPool::ms_Pool->f_GetBlock();
			if (!_fFunctor(pMem, Size))
				break;
		}
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Realloc
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_ReallocDebug
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Resize
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_ResizeDebug
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return CStaticPool::ms_Pool->f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Commit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Decommit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Free(void *_pBlock, umint _Size)
	{
		return CStaticPool::ms_Pool->f_ReturnBlock(_pBlock);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_FreeNoSize(void *_pBlock)
	{
		return CStaticPool::ms_Pool->f_ReturnBlock(_pBlock);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline fp32 TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Overhead(void const *_pBlock)
	{
		return CStaticPool::ms_Pool->f_Overhead(_pBlock);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	only_parameters_aliased inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocSafeWithSize
		(
			umint &_Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> CAutoDestroy
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	only_parameters_aliased inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocSafe
		(
			umint _Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> CAutoDestroy
	{
		CAutoDestroy AutoDestroy;
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size};
	}

	template <typename tf_CType>
	void fg_StaticPoolDelete(tf_CType *_pToDelete)
	{
		TCStaticPool<tf_CType>::ms_Pool->f_Delete(_pToDelete);
	}

	template <typename tf_CType, typename ...tfp_CParams>
	void fg_StaticPoolNew(tf_CType * &_pDestPtr, tfp_CParams && ...p_Params)
	{
		_pDestPtr = TCStaticPool<tf_CType>::ms_Pool->f_New(fg_Forward<tfp_CParams>(p_Params)...);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constexpr bool TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::operator == (TCPoolAllocator const &_Right) const noexcept
	{
		return this == &_Right;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constexpr auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::operator <=> (TCPoolAllocator const &_Right) const noexcept
	{
		return this <=> &_Right;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_StaticAddresses()
	{
		return 0;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_GranularityAlloc(bool _bLargePages)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_GranularityCommit(bool _bLargePages)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_GranularityProtect(bool _bLargePages)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Size(void *_pBlock)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_TrySize(void *_pBlock)
	{
		DMibPDebugBreak; // Not supported
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline umint TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_SizePadded(umint _Size)
	{
		return sizeof(t_CType);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constexpr bool TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_CanCommit()
	{
		return false;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline bool TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_CanProtect()
	{
		return false;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline bool TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_DeterministicSize()
	{
		return true;
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocWithSizeDebug
		(
			umint _Size
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void *TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocAlignedWithSize
		(
			umint _Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocDebug
		(
			umint _Size
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void *TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void *TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	only_parameters_aliased inline void TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocBatch
		(
			umint _Size
			, umint _Alignment
			, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		while (true)
		{
			umint Size = _Size;
			void *pMem = m_Pool.f_GetBlock();
			if (!_fFunctor(pMem, Size))
				break;
		}
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Realloc
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_ReallocDebug
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Resize
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_ResizeDebug
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(t_CType));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Commit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Decommit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Free(void *_pBlock, umint _Size)
	{
		return m_Pool.f_ReturnBlock(_pBlock);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline void TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_FreeNoSize(void *_pBlock)
	{
		return m_Pool.f_ReturnBlock(_pBlock);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline fp32 TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_Overhead(void const *_pBlock)
	{
		return m_Pool.f_Overhead(_pBlock);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	only_parameters_aliased inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocSafeWithSize
		(
			umint &_Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	only_parameters_aliased inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_AllocSafe
		(
			umint _Size
			, umint _Alignment
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	inline auto TCPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size, *this};
	}

	template <typename t_CPoolType>
	constexpr bool TCPoolReferenceAllocator<t_CPoolType>::operator == (TCPoolReferenceAllocator const &_Right) const noexcept
	{
		return &m_Pool == &_Right.m_Pool;
	}

	template <typename t_CPoolType>
	constexpr auto TCPoolReferenceAllocator<t_CPoolType>::operator <=> (TCPoolReferenceAllocator const &_Right) const noexcept
	{
		return &m_Pool <=> &_Right.m_Pool;
	}

	template <typename t_CPoolType>
	inline TCPoolReferenceAllocator<t_CPoolType>::TCPoolReferenceAllocator(t_CPoolType &_Pool)
		: m_Pool(_Pool)
	{
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_StaticAddresses()
	{
		return 0;
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_GranularityAlloc(bool _bLargePages)
	{
		return sizeof(CData);
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_GranularityCommit(bool _bLargePages)
	{
		return sizeof(CData);
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_GranularityProtect(bool _bLargePages)
	{
		return sizeof(CData);
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_Size(void *_pBlock)
	{
		return sizeof(CData);
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_TrySize(void *_pBlock)
	{
		DMibPDebugBreak; // Not supported
		return sizeof(CData);
	}

	template <typename t_CPoolType>
	inline umint TCPoolReferenceAllocator<t_CPoolType>::f_SizePadded(umint _Size)
	{
		return sizeof(CData);
	}

	template <typename t_CPoolType>
	constexpr bool TCPoolReferenceAllocator<t_CPoolType>::f_CanCommit()
	{
		return false;
	}

	template <typename t_CPoolType>
	inline bool TCPoolReferenceAllocator<t_CPoolType>::f_CanProtect()
	{
		return false;
	}

	template <typename t_CPoolType>
	inline bool TCPoolReferenceAllocator<t_CPoolType>::f_DeterministicSize()
	{
		return true;
	}

	template <typename t_CPoolType>
	inline void TCPoolReferenceAllocator<t_CPoolType>::f_Protect(void *_pMem, umint _Size, uaint _Protect)
	{
	}

	template <typename t_CPoolType>
	inline auto TCPoolReferenceAllocator<t_CPoolType>::f_AllocWithSizeDebug
		(
			umint _Size
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(CData));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline void *TCPoolReferenceAllocator<t_CPoolType>::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(CData));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline void *TCPoolReferenceAllocator<t_CPoolType>::f_AllocAlignedWithSize(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(CData));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline auto TCPoolReferenceAllocator<t_CPoolType>::f_AllocDebug(umint _Size, ch8 const *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(CData));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline void *TCPoolReferenceAllocator<t_CPoolType>::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(CData));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline void *TCPoolReferenceAllocator<t_CPoolType>::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(CData));
		return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	only_parameters_aliased inline void TCPoolReferenceAllocator<t_CPoolType>::f_AllocBatch
		(
			umint _Size
			, umint _Alignment
			, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
	{
		DMibFastCheck(_Size == sizeof(CData));
		while (true)
		{
			umint Size = _Size;
			void *pMem = m_Pool.f_GetBlock();
			if (!_fFunctor(pMem, Size))
				break;
		}
	}

	template <typename t_CPoolType>
	inline void *TCPoolReferenceAllocator<t_CPoolType>::f_Realloc(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(CData));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline auto TCPoolReferenceAllocator<t_CPoolType>::f_ReallocDebug
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(CData));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline void *TCPoolReferenceAllocator<t_CPoolType>::f_Resize(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		DMibFastCheck(_Size == sizeof(CData));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline auto TCPoolReferenceAllocator<t_CPoolType>::f_ResizeDebug
		(
			void *_pMem
			, umint _Size
			, umint _OldSize
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
			, ENumaNode _NumaNode
		)
		-> void *
	{
		DMibFastCheck(_Size == sizeof(CData));
		if (_pMem)
			return _pMem;
		else
			return m_Pool.f_GetBlock();
	}

	template <typename t_CPoolType>
	inline void TCPoolReferenceAllocator<t_CPoolType>::f_Commit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CPoolType>
	inline void TCPoolReferenceAllocator<t_CPoolType>::f_Decommit(void *_pMem, umint _Size)
	{
	}

	template <typename t_CPoolType>
	inline void TCPoolReferenceAllocator<t_CPoolType>::f_Free(void *_pBlock, umint _Size)
	{
		return m_Pool.f_ReturnBlock(_pBlock);
	}

	template <typename t_CPoolType>
	inline void TCPoolReferenceAllocator<t_CPoolType>::f_FreeNoSize(void *_pBlock)
	{
		return m_Pool.f_ReturnBlock(_pBlock);
	}

	template <typename t_CPoolType>
	inline fp32 TCPoolReferenceAllocator<t_CPoolType>::f_Overhead(void const *_pBlock)
	{
		return m_Pool.f_Overhead(_pBlock);
	}

	template <typename t_CPoolType>
	only_parameters_aliased inline auto TCPoolReferenceAllocator<t_CPoolType>::f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		-> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CPoolType>
	only_parameters_aliased inline auto TCPoolReferenceAllocator<t_CPoolType>::f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode) -> CAutoDestroy
	{
		CAutoDestroy AutoDestroy{*this};
		AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
		AutoDestroy.m_Size = _Size;

		return fg_Move(AutoDestroy);
	}

	template <typename t_CPoolType>
	inline auto TCPoolReferenceAllocator<t_CPoolType>::f_MakeSafe(void *_pMemory, umint _Size) -> CAutoDestroy
	{
		return CAutoDestroy{_pMemory, _Size, *this};
	}
}
