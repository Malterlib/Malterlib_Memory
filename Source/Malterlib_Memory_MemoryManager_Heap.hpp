// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	inline CMemoryManagerArenaHeapBlock::CMemoryManagerArenaHeapBlock()
		: m_Flags(EMemoryManagerArenaHeapBlockFlag_None)
	{
	}

	template <typename t_CParams>
	uint8 *TCMemoryManagerArenaHeapChunk<t_CParams>::f_GetBlockAddress(CMemoryManagerArenaHeapBlock const *_pBlock) const
	{
		return NContainer::TCMap<uint8 *, CMemoryManagerArenaHeapBlock, NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>>::fs_GetKey(*_pBlock);
	}

	template <typename t_CParams>
	umint TCMemoryManagerArenaHeapChunk<t_CParams>::f_GetBlockSize(CMemoryManagerArenaHeapBlock const *_pBlock) const
	{
		uint8 *pAddress = f_GetBlockAddress(_pBlock);
		auto pNextBlock = mp_Blocks.f_FindSmallestGreaterThanEqual(pAddress + 1);
		if (pNextBlock)
			return f_GetBlockAddress(pNextBlock) - pAddress;

		return mp_Size - (pAddress - f_GetAddress());
	}

	template <typename t_CParams>
	bool TCMemoryManagerArenaHeapChunk<t_CParams>::f_IsEmpty()
	{
		auto iBlock = mp_Blocks.f_GetIterator();
		if (!iBlock)
			return true;

		if (iBlock->m_Flags & EMemoryManagerArenaHeapBlockFlag_Allocated)
			return false;

		++iBlock;

		if (iBlock)
			return false;

		return true;
	}

	template <typename t_CParams>
	TCMemoryManagerArenaHeapChunk<t_CParams>::TCMemoryManagerArenaHeapChunk(umint _Size, TCMemoryManagerArenaHeap<t_CParams> *_pHeap)
		: mp_Size(_Size)
		, mp_pHeap(_pHeap)
		, mp_Blocks(CAllocatorConstructTag(), _pHeap->mp_pMemoryManager)
		, mp_RequestedCleanup(ENumaArenaCleanup_None)
	{
	}

	template <typename t_CParams>
	TCMemoryManagerArenaHeapChunk<t_CParams>::~TCMemoryManagerArenaHeapChunk()
	{
		DMibMemLightweightTrackDisableScope;

		for (auto iBlock = mp_Blocks.f_GetIterator(); iBlock; ++iBlock)
		{
			auto *pBlock = &*iBlock;
			if (!(pBlock->m_Flags & EMemoryManagerArenaHeapBlockFlag_Allocated))
			{
				DMibFastCheck(pBlock->m_Link.f_IsInList());
				umint BlockSize = f_GetBlockSize(pBlock);
				auto pFreeBucket = mp_pHeap->mp_FreeBuckets.f_FindEqual(BlockSize);
				DMibFastCheck(pFreeBucket);
				pBlock->m_Link.f_UnlinkLinked();
				if (pFreeBucket && pFreeBucket->f_IsEmpty())
					mp_pHeap->mp_FreeBuckets.f_Remove(pFreeBucket);
			}
			else
				DMibFastCheck(!pBlock->m_Link.f_IsInList());
		}

		mp_Blocks.f_Clear();

		umint Size = f_GetSize();
		uint8 *pAddress = f_GetAddress();

		if constexpr (TCMemoryManagerArena<t_CParams>::mc_bEnableCallbacks)
			mp_pHeap->mp_pMemoryManager->f_OnCommit(pAddress, Size);

		mp_pHeap->mp_pMemoryManager->mp_Allocator.f_Free(pAddress, Size);
	}

	template <typename t_CParams>
	uint8 *TCMemoryManagerArenaHeapChunk<t_CParams>::f_GetAddress() const
	{
		return NContainer::TCMap<uint8 *, TCMemoryManagerArenaHeapChunk<t_CParams>, NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>>::fs_GetKey(*this);
	}

	template <typename t_CParams>
	uint8 *TCMemoryManagerArenaHeapChunk<t_CParams>::f_GetEndAddress() const
	{
		return f_GetAddress() + f_GetSize();
	}

	template <typename t_CParams>
	umint TCMemoryManagerArenaHeapChunk<t_CParams>::f_GetSize() const
	{
		return mp_Size;
	}

	template <typename t_CParams>
	TCMemoryManagerArenaHeap<t_CParams> *TCMemoryManagerArenaHeapChunk<t_CParams>::f_GetHeap() const
	{
		return mp_pHeap;
	}

	////////////////////////////////////////////////////

	template <typename t_CParams>
	TCMemoryManagerArenaHeap<t_CParams>::TCMemoryManagerArenaHeap(TCMemoryManager<t_CParams> *_pMemoryManager, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena)
		: t_CParams::CNotifier::CHeap(_pMemoryManager)
		, mp_FreeBuckets(CAllocatorConstructTag(), _pMemoryManager)
		, mp_pNumaArena(_pNumaArena)
		, mp_pMemoryManager(_pMemoryManager)
	{
	}

	template <typename t_CParams>
	TCMemoryManagerArenaHeap<t_CParams>::~TCMemoryManagerArenaHeap()
	{
		DMibMemLightweightTrackDisableScope;
		for (auto iChunk = mp_Chunks.f_GetIterator(); iChunk;)
		{
			auto pChunk = &*iChunk;
			++iChunk;
			mp_pMemoryManager->mp_HeapChunks.f_Remove(pChunk);
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_RequestCleanup(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, ENumaArenaCleanup _Cleanup)
	{
		if ((_pChunk->mp_RequestedCleanup & _Cleanup) != _Cleanup)
		{
			_pChunk->mp_RequestedCleanup |= _Cleanup;
			mp_ChunksNeedingCleanup.f_Insert(_pChunk);
			mp_pNumaArena->f_RequestCleanup(_Cleanup);
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::f_Destroy()
	{
		DMibMemLightweightTrackDisableScope;

		mp_FreeBuckets.f_Clear();
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_InitBlockCommit(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size)
	{
		// Make sure that the end of every slab is commited. This allows the free logic to work at O(1) complexity for small allocations because it can assume that the end of each
		// slab is a valid memory address

		auto Granularity = mp_pMemoryManager->mp_Allocator.f_GranularityCommit();
		uint8 *pEndAddress = _pAddress + _Size;
		for (uint8 *pAddress = fg_AlignUp(_pAddress + 1, t_CParams::mc_SlabSize) - Granularity; pAddress < pEndAddress; pAddress += t_CParams::mc_SlabSize)
		{
			mp_pMemoryManager->mp_Allocator.f_Commit(pAddress, Granularity);
			if constexpr (mcp_bEnableCallbacks)
				mp_pMemoryManager->f_OnCommit(pAddress, Granularity);
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_CommitBlock(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size)
	{
		auto pChunkAddress = _pChunk->f_GetAddress();

		umint StartBit = (_pAddress - pChunkAddress) / t_CParams::mc_HeapBlockSize;
		umint nBits = _Size / t_CParams::mc_HeapBlockSize;

		if constexpr (mcp_bEnableCallbacks)
		{
			mp_pMemoryManager->f_OnCommit(pChunkAddress + StartBit * t_CParams::mc_HeapBlockSize, nBits * t_CParams::mc_HeapBlockSize);

			_pChunk->mp_Committed.f_EnumSetBitRanges
				(
					[&](umint _Bit, umint _nBits) -> bool
					{
						this->f_OnCheckFree(pChunkAddress + _Bit * t_CParams::mc_HeapBlockSize, _nBits * t_CParams::mc_HeapBlockSize, EMemoryManagerCheckFlag_Default);
						return true;
					}
					, StartBit
					, StartBit + nBits
				)
			;
		}

		_pChunk->mp_Committed.f_EnumFreeBitRanges
			(
				[&](umint _Bit, umint _nBits) -> bool
				{
					mp_pMemoryManager->mp_Allocator.f_Commit(pChunkAddress + _Bit * t_CParams::mc_HeapBlockSize, _nBits * t_CParams::mc_HeapBlockSize);
					return true;
				}
				, StartBit
				, StartBit + nBits
			)
		;
		_pChunk->mp_Committed.template f_SetBitRange<true>(StartBit, nBits);
		_pChunk->mp_DeferredDecommit.template f_SetBitRange<false>(StartBit, nBits);
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_DecommitBlockForReal(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size)
	{
		auto pChunkAddress = _pChunk->f_GetAddress();
		umint StartBit = (_pAddress - pChunkAddress) / t_CParams::mc_HeapBlockSize;
		umint nBits = _Size / t_CParams::mc_HeapBlockSize;

		_pChunk->mp_Committed.f_EnumSetBitRanges
			(
				[&](umint _Bit, umint _nBits) -> bool
				{
					uint8 *pStartAddress = pChunkAddress + _Bit * t_CParams::mc_HeapBlockSize;
					umint nBytesToDecommit = _nBits * t_CParams::mc_HeapBlockSize;
					uint8 *pEndAddress = pStartAddress + nBytesToDecommit;
					for (uint8 *pAddress = pStartAddress; pAddress < pEndAddress;)
					{
						uint8 *pMaxEnd = fg_AlignUp(pAddress + 1, t_CParams::mc_SlabSize) - t_CParams::mc_SubSlabSize;
						if (pMaxEnd == pAddress)
						{
							pAddress += t_CParams::mc_SubSlabSize;
							continue;
						}

						uint8 *pThisEnd = fg_Min(pEndAddress, pMaxEnd);

						mp_pMemoryManager->mp_Allocator.f_Decommit(pAddress, pThisEnd - pAddress);

						pAddress = pThisEnd;
					}
					return true;
				}
				, StartBit
				, StartBit + nBits
			)
		;
		_pChunk->mp_Committed.template f_SetBitRange<false>(StartBit, nBits);
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_DecommitBlock(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size)
	{
		auto pChunkAddress = _pChunk->f_GetAddress();
		umint StartBit = (_pAddress - pChunkAddress) / t_CParams::mc_HeapBlockSize;
		umint nBits = _Size / t_CParams::mc_HeapBlockSize;

		if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Commit) != 0)
		{
			if constexpr (mcp_bEnableCallbacks)
			{
				_pChunk->mp_Committed.f_EnumSetBitRanges
					(
						[&](umint _Bit, umint _nBits) -> bool
						{
							uint8 *pStartAddress = pChunkAddress + _Bit * t_CParams::mc_HeapBlockSize;
							umint nBytesToDecommit = _nBits * t_CParams::mc_HeapBlockSize;
							uint8 *pEndAddress = pStartAddress + nBytesToDecommit;
							for (uint8 *pAddress = pStartAddress; pAddress < pEndAddress;)
							{
								uint8 *pMaxEnd = fg_AlignUp(pAddress + 1, t_CParams::mc_SlabSize) - t_CParams::mc_SubSlabSize;
								if (pMaxEnd == pAddress)
								{
									this->f_OnFillFree(pAddress, t_CParams::mc_SubSlabSize, EMemoryManagerCheckFlag_None);
									pAddress += t_CParams::mc_SubSlabSize;
									continue;
								}

								uint8 *pThisEnd = fg_Min(pEndAddress, pMaxEnd);

								this->f_OnFillFree(pAddress, pThisEnd - pAddress);

								pAddress = pThisEnd;
							}

							return true;
						}
						, StartBit
						, StartBit + nBits
					)
				;
			}

			_pChunk->mp_DeferredDecommit.template f_SetBitRange<true>(StartBit, nBits);

			_pChunk->mp_DecommitTimestamp = TCMemoryManagerNumaArena<t_CParams>::fs_GetTimestamp();
			fp_RequestCleanup(_pChunk, ENumaArenaCleanup_HeapCommit);
			return;
		}
		else if constexpr (mcp_bEnableCallbacks)
			mp_pMemoryManager->f_OnDecommit(pChunkAddress + StartBit * t_CParams::mc_HeapBlockSize, nBits * t_CParams::mc_HeapBlockSize);

		fp_DecommitBlockForReal(_pChunk, _pAddress, _Size);
	}

	template <typename t_CParams>
	int64 TCMemoryManagerArenaHeap<t_CParams>::f_GarbageCollect(int64 _Timestamp)
	{
		int64 NextTimestamp = TCLimitsInt<int64>::mc_Max;
		DMibMemLightweightTrackDisableScope;

		for (auto iChunk = mp_ChunksNeedingCleanup.f_GetIterator(); iChunk;)
		{
			auto pChunk = &*iChunk;
			++iChunk;

			if (pChunk->mp_FreeTimestamp > _Timestamp)
			{
				NextTimestamp = fg_Min(NextTimestamp, pChunk->mp_FreeTimestamp);
				continue;
			}

			if (pChunk->f_IsEmpty())
			{
				DMibLock(mp_pMemoryManager->mp_HeapChunksLock);
				mp_pMemoryManager->mp_HeapChunks.f_Remove(pChunk);
			}
			else if ((pChunk->mp_RequestedCleanup &= ~ENumaArenaCleanup_HeapGarbage) == 0)
				pChunk->mp_CleanupLink.f_Unlink();
		}

		return NextTimestamp;
	}

	template <typename t_CParams>
	int64 TCMemoryManagerArenaHeap<t_CParams>::f_DecommitDeferred(int64 _Timestamp)
	{
		int64 NextTimestamp = TCLimitsInt<int64>::mc_Max;

		if constexpr (!t_CParams::CAllocator::f_CanCommit())
			return NextTimestamp;

		for (auto iChunk = mp_ChunksNeedingCleanup.f_GetIterator(); iChunk;)
		{
			auto pChunk = &*iChunk;
			++iChunk;

			if (pChunk->mp_DecommitTimestamp > _Timestamp)
			{
				NextTimestamp = fg_Min(NextTimestamp, pChunk->mp_DecommitTimestamp);
				continue;
			}

			auto pChunkAddress = pChunk->f_GetAddress();
			umint nBits = pChunk->mp_Size / t_CParams::mc_HeapBlockSize;
			pChunk->mp_DeferredDecommit.f_EnumSetBitRanges
				(
					[&](umint _Bit, umint _nBits) -> bool
					{
						uint8 *pStartAddress = pChunkAddress + _Bit * t_CParams::mc_HeapBlockSize;
						umint nBytesToDecommit = _nBits * t_CParams::mc_HeapBlockSize;
						fp_DecommitBlockForReal(pChunk, pStartAddress, nBytesToDecommit);
						return true;
					}
					, 0
					, nBits
				)
			;
			pChunk->mp_DeferredDecommit.template f_SetBitRange<false>(0, nBits);
			if ((pChunk->mp_RequestedCleanup &= ~ENumaArenaCleanup_HeapCommit) == 0)
				pChunk->mp_CleanupLink.f_Unlink();
		}

		return NextTimestamp;
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_AddNewChunk()
	{
		DMibMemLightweightTrackDisableScope;

		umint Size = t_CParams::mc_HeapChunkSize;
		EAllocationFlag Flags = t_CParams::mc_AllocationFlags;
		if constexpr (t_CParams::CAllocator::f_CanCommit())
			Flags |= EAllocationFlag_NoCommit;

		uint8 *pMemory = (uint8 *)mp_pMemoryManager->mp_Allocator.f_AllocAlignedWithSize(Size, t_CParams::mc_SlabSize, Flags, mp_pNumaArena->mp_NumaNode);

		TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk;
		{
			DMibLock(mp_pMemoryManager->mp_HeapChunksLock);
			pChunk = &*mp_pMemoryManager->mp_HeapChunks(pMemory, Size, this);
		}

		mp_Chunks.f_Insert(pChunk);

		if constexpr (t_CParams::CAllocator::f_CanCommit())
		{
			if constexpr (mcp_bEnableCallbacks)
				mp_pMemoryManager->f_OnDecommit(pMemory, t_CParams::mc_SlabSize);

			fp_InitBlockCommit(pChunk, pMemory, Size);
		}
		else if constexpr (mcp_bEnableCallbacks)
			this->f_OnFillFree(pMemory, Size);

		auto &Block = pChunk->mp_Blocks[pMemory];

		Block.m_pChunk = pChunk;

		mp_FreeBuckets[Size].f_Insert(Block);
	}

	template <typename t_CParams>
	umint TCMemoryManagerArenaHeap<t_CParams>::fs_GetAllocSize(umint _Size)
	{
		return fg_AlignUp(_Size, t_CParams::mc_HeapBlockSize);
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManagerArenaHeap<t_CParams>::f_AllocWithSize(umint &_Size)
	{
		DMibLock(*this);
		umint Size = (_Size + t_CParams::mc_HeapBlockSize - 1) & ~umint(t_CParams::mc_HeapBlockSize - 1);

		DMibMemLightweightTrack(mp_pMemoryManager->fp_TrackAlloc(Size));
		DMibMemLightweightTrackDisableScope;

		auto pBucket = mp_FreeBuckets.f_FindSmallestGreaterThanEqual(Size);
		if (!pBucket)
		{
			fp_AddNewChunk();
			pBucket = mp_FreeBuckets.f_FindSmallestGreaterThanEqual(Size);
			DMibFastCheck(pBucket);
		}

		umint FoundSize = mp_FreeBuckets.fs_GetKey(pBucket);

		auto pBlock = pBucket->f_Pop();

		if (pBucket->f_IsEmpty())
			mp_FreeBuckets.f_Remove(pBucket);

		umint LeftOverSize = FoundSize - Size;

		pBlock->m_Flags |= EMemoryManagerArenaHeapBlockFlag_Allocated;

		TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk = fg_AutoStaticCast(pBlock->m_pChunk);

		uint8 *pRetAddress = pChunk->f_GetBlockAddress(pBlock);

		if (LeftOverSize > 0)
		{
			uint8 *pLeftOverAddress = pRetAddress + Size;
			auto &LeftOverBlock = pChunk->mp_Blocks[pLeftOverAddress];
			LeftOverBlock.m_pChunk = pChunk;
			mp_FreeBuckets[LeftOverSize].f_Insert(LeftOverBlock);
		}

		if constexpr (t_CParams::CAllocator::f_CanCommit())
			fp_CommitBlock(pChunk, pRetAddress, Size);
		else if constexpr (mcp_bEnableCallbacks)
			this->f_OnCheckFree(pRetAddress, Size, EMemoryManagerCheckFlag_Default);

		if constexpr (mcp_bEnableCallbacks)
			this->f_OnAlloc(pRetAddress, Size);

		_Size = Size;

		return pRetAddress;
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManagerArenaHeap<t_CParams>::f_AllocAlignedWithSize(umint &_Size, umint _Alignment)
	{
		DMibLock(*this);
		umint Size = fg_AlignUp(_Size, _Alignment);
		Size = (Size + t_CParams::mc_HeapBlockSize - 1) & ~umint(t_CParams::mc_HeapBlockSize - 1);

		DMibMemLightweightTrack(mp_pMemoryManager->fp_TrackAlloc(Size));
		DMibMemLightweightTrackDisableScope;

		auto pBucket = mp_FreeBuckets.f_FindEqual(Size);

		uint8 *pRetAddress = nullptr;
		TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk = nullptr;
		if (pBucket)
		{
			// First try to find an exact match in the 16 most recent free blocks for our size
			umint nLoops = 0;
			CMemoryManagerArenaHeapBlock *pBlock = nullptr;
			for (auto iFreeBlocks = pBucket->f_GetIterator(); iFreeBlocks && nLoops < 16; ++iFreeBlocks, ++nLoops)
			{
				auto pFreeBlock = iFreeBlocks.f_GetCurrent();
				TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk = fg_AutoStaticCast(pFreeBlock->m_pChunk);

				uint8 *pRetAddress = pChunk->f_GetBlockAddress(pFreeBlock);
				if (!((umint)pRetAddress & (_Alignment - 1)))
				{
					// This block is already aligned so lets use it
					pFreeBlock->m_Link.f_Unlink();

					pBlock = pFreeBlock;

					if (pBucket->f_IsEmpty())
						mp_FreeBuckets.f_Remove(pBucket);
					break;
				}
			}

			if (pBlock)
			{
				pBlock->m_Flags |= EMemoryManagerArenaHeapBlockFlag_Allocated;

				pChunk = fg_AutoStaticCast(pBlock->m_pChunk);

				pRetAddress = pChunk->f_GetBlockAddress(pBlock);
			}
		}

		if (!pRetAddress)
		{
			umint NeededSize = Size + _Alignment - t_CParams::mc_HeapBlockSize; // Find enough for aligning pointer

			pBucket = mp_FreeBuckets.f_FindSmallestGreaterThanEqual(NeededSize);
			if (!pBucket)
			{
				fp_AddNewChunk();
				pBucket = mp_FreeBuckets.f_FindSmallestGreaterThanEqual(NeededSize);
				DMibFastCheck(pBucket);
			}

			umint FoundSize = mp_FreeBuckets.fs_GetKey(pBucket);

			auto pBlock = pBucket->f_Pop();

			if (pBucket->f_IsEmpty())
				mp_FreeBuckets.f_Remove(pBucket);

			pChunk = fg_AutoStaticCast(pBlock->m_pChunk);
			uint8 *pStartOfBlock = pChunk->f_GetBlockAddress(pBlock);
			pRetAddress = fg_AlignUp(pStartOfBlock, _Alignment);

			umint LeftOverSizeStart = pRetAddress - pStartOfBlock;

			if (LeftOverSizeStart)
			{
				FoundSize -= LeftOverSizeStart;
				auto &NewBlock = pChunk->mp_Blocks[pRetAddress];
				NewBlock.m_pChunk = pChunk;
				mp_FreeBuckets[LeftOverSizeStart].f_Insert(*pBlock);

				pBlock = &NewBlock;
			}

			umint LeftOverSize = FoundSize - Size;

			pBlock->m_Flags |= EMemoryManagerArenaHeapBlockFlag_Allocated;

			if (LeftOverSize > 0)
			{
				uint8 *pLeftOverAddress = pRetAddress + Size;
				auto &LeftOverBlock = pChunk->mp_Blocks[pLeftOverAddress];
				LeftOverBlock.m_pChunk = pChunk;
				mp_FreeBuckets[LeftOverSize].f_Insert(LeftOverBlock);
			}
		}

		if constexpr (t_CParams::CAllocator::f_CanCommit())
			fp_CommitBlock(pChunk, pRetAddress, Size);
		else if constexpr (mcp_bEnableCallbacks)
			this->f_OnCheckFree(pRetAddress, Size, EMemoryManagerCheckFlag_Default);

		if constexpr (mcp_bEnableCallbacks)
			this->f_OnAlloc(pRetAddress, Size);

		_Size = Size;

		return pRetAddress;
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_RemoveFreeBlock(CMemoryManagerArenaHeapBlock *_pBlock, TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk)
	{
		umint BlockSize = _pChunk->f_GetBlockSize(_pBlock);

		auto pBucket = mp_FreeBuckets.f_FindEqual(BlockSize);

		DMibFastCheck(pBucket);

		pBucket->f_Remove(_pBlock);

		if (pBucket->f_IsEmpty())
			mp_FreeBuckets.f_Remove(pBucket);
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::fp_TraceBlocks(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk)
	{
		DMibTrace("Blocks start\n", 0);
		for (auto &Block : _pChunk->mp_Blocks)
			DMibTrace("0x{}: {}\n", (void *)(_pChunk->f_GetBlockAddress(&Block)), Block.m_Flags);
		DMibTrace("Blocks end\n", 0);
	}

	template <typename t_CParams>
	umint TCMemoryManagerArenaHeap<t_CParams>::f_Size(void const *_pMemory, TCMemoryManagerArenaHeapChunk<t_CParams> const *_pChunk) const
	{
		DMibLock(fg_RemoveQualifiers(*this));

		uint8 *pMem = (uint8 *)_pMemory;
		auto *pBlock = _pChunk->mp_Blocks.f_FindEqual(pMem);

		DMibFastCheck(pBlock);

		return _pChunk->f_GetBlockSize(pBlock);
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::f_Lock()
	{
		DMibFastCheck(!mp_pMemoryManager->mp_LocalArena->m_pNumaArena->mp_bLimitedArenas || !mp_pMemoryManager->mp_LocalArena->m_pArena);
		mp_Lock.f_Lock();
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::f_Unlock()
	{
		mp_Lock.f_Unlock();
	}

	template <typename t_CParams>
	void TCMemoryManagerArenaHeap<t_CParams>::f_ForkedChild()
	{
		mp_Lock.f_ForkedChildLocked();
	}

	template <typename t_CParams>
	fp32 TCMemoryManagerArenaHeap<t_CParams>::f_Overhead(void const *_pMemory, TCMemoryManagerArenaHeapChunk<t_CParams> const *_pChunk) const
	{
		DMibLock(fg_RemoveQualifiers(*this));
		uint8 *pMem = (uint8 *)_pMemory;
		auto *pBlock = _pChunk->mp_Blocks.f_FindEqual(pMem);

		DMibFastCheck(pBlock);

		fp32 OverheadPerByte = fp32(sizeof(TCMemoryManagerArenaHeapChunk<t_CParams>)) / fp32(_pChunk->mp_Size);
		umint Size = _pChunk->f_GetBlockSize(pBlock);

		return fp32(Size) * OverheadPerByte
			+ sizeof(CMemoryManagerArenaHeapBlock)
			+ sizeof(void *) * 3 // Address and map tree pointers
		;
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArenaHeap<t_CParams>::f_Free(void *_pMem, TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk)
	{
		DMibLock(*this);
		DMibMemLightweightTrackDisableScope;

		uint8 *pMem = (uint8 *)_pMem;

		if constexpr (mcp_bEnableCallbacks)
			this->f_OnFree(pMem);

		auto *pBlock = _pChunk->mp_Blocks.f_FindEqual(pMem);

		DMibFastCheck(pBlock);

		umint BlockSize = _pChunk->f_GetBlockSize(pBlock);

		if constexpr (t_CParams::CAllocator::f_CanCommit())
			fp_DecommitBlock(_pChunk, pMem, BlockSize);
		else if constexpr (mcp_bEnableCallbacks)
			this->f_OnFillFree(pMem, BlockSize);

		pBlock->m_Flags &= ~EMemoryManagerArenaHeapBlockFlag_Allocated;
		auto pNewFreeBlock = pBlock;

		auto pPrevBlock = _pChunk->mp_Blocks.f_FindLargestLessThanEqual(pMem - 1);
		auto pNextBlock = _pChunk->mp_Blocks.f_FindSmallestGreaterThanEqual(pMem + 1);

		if (pPrevBlock && !(pPrevBlock->m_Flags & EMemoryManagerArenaHeapBlockFlag_Allocated))
		{
			fp_RemoveFreeBlock(pPrevBlock, _pChunk);
			pNewFreeBlock = pPrevBlock;
			_pChunk->mp_Blocks.f_Remove(pBlock);
		}

		if (pNextBlock && !(pNextBlock->m_Flags & EMemoryManagerArenaHeapBlockFlag_Allocated))
		{
			fp_RemoveFreeBlock(pNextBlock, _pChunk);
			_pChunk->mp_Blocks.f_Remove(pNextBlock);
		}

		umint NewSize = _pChunk->f_GetBlockSize(pNewFreeBlock);

		mp_FreeBuckets[NewSize].f_Insert(*pNewFreeBlock);

		if (NewSize == t_CParams::mc_HeapChunkSize)
		{
			// Totally free chunk, add for cleanup
			if constexpr (t_CParams::mc_bBackgroundCleanup)
				_pChunk->mp_FreeTimestamp = TCMemoryManagerNumaArena<t_CParams>::fs_GetTimestamp();
			fp_RequestCleanup(_pChunk, ENumaArenaCleanup_HeapGarbage);
		}
	}

	template <typename t_CParams>
	bool TCMemoryManagerArenaHeap<t_CParams>::f_CheckFree(EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;

		for (auto iBucket = mp_FreeBuckets.f_GetIterator(); iBucket; ++iBucket)
		{
			umint Size = iBucket.f_GetKey();

			for (auto iBlock = iBucket->f_GetIterator(); iBlock; ++iBlock)
			{
				TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk = fg_AutoStaticCast(iBlock->m_pChunk);

				uint8 *pAddress = pChunk->f_GetBlockAddress(iBlock);
				DMibFastCheck(Size == pChunk->f_GetBlockSize(iBlock));

				if constexpr (mcp_bEnableCallbacks)
				{
					if constexpr (t_CParams::CAllocator::f_CanCommit())
					{
						auto pChunkAddress = pChunk->f_GetAddress();

						umint StartBit = (pAddress - pChunkAddress) / t_CParams::mc_HeapBlockSize;
						umint nBits = Size / t_CParams::mc_HeapBlockSize;

						if constexpr (mcp_bEnableCallbacks)
						{
							pChunk->mp_Committed.f_EnumSetBitRanges
								(
									[&](umint _Bit, umint _nBits) -> bool
									{
										if (this->f_OnCheckFree(pChunkAddress + _Bit * t_CParams::mc_HeapBlockSize, _nBits * t_CParams::mc_HeapBlockSize, _Flags))
											bError = true;
										return true;
									}
									, StartBit
									, StartBit + nBits
								)
							;
						}
					}
					else
					{
						if (this->f_OnCheckFree(pAddress, Size, _Flags))
							bError = true;
					}
				}
			}
		}

		return bError;
	}
}
