// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Container/BitArrayHierarchical>

namespace NMib::NMemory
{
	enum ENumaArenaCleanup
	{
		ENumaArenaCleanup_None              = 0
		, ENumaArenaCleanup_HeapGarbage     = DMibBit(0)
		, ENumaArenaCleanup_HeapCommit      = DMibBit(1)
		, ENumaArenaCleanup_ProcessMessages = DMibBit(2)
		, ENumaArenaCleanup_FreeSlabs       = DMibBit(3)
	};

	struct CMemoryManagerGarbageOptions
	{
		int64 m_Timestamp;
		int64 m_TimestampDecommit;
	};

	template <typename t_CParams>
	struct TCMemoryManagerArenaHeap;

	enum EMemoryManagerArenaHeapBlockFlag
	{
		EMemoryManagerArenaHeapBlockFlag_None        = 0
		, EMemoryManagerArenaHeapBlockFlag_Allocated = DMibBit(0)
	};

	struct CMemoryManagerArenaHeapBlock
	{
		DMibListLinkDS_Link(CMemoryManagerArenaHeapBlock, m_Link);
		void *m_pChunk;
		EMemoryManagerArenaHeapBlockFlag m_Flags;

		CMemoryManagerArenaHeapBlock();
	};

	template <typename t_CParams>
	struct TCMemoryManagerArenaHeapChunk
	{
		TCMemoryManagerArenaHeapChunk(umint _Size, TCMemoryManagerArenaHeap<t_CParams> *_pHeap);
		~TCMemoryManagerArenaHeapChunk();

		uint8 *f_GetBlockAddress(CMemoryManagerArenaHeapBlock const *_pBlock) const;
		umint f_GetBlockSize(CMemoryManagerArenaHeapBlock const *_pBlock) const;

		uint8 *f_GetAddress() const;
		uint8 *f_GetEndAddress() const;
		umint f_GetSize() const;
		TCMemoryManagerArenaHeap<t_CParams> *f_GetHeap() const;

		bool f_IsEmpty();

	private:
		template <typename t_CParams2>
		friend struct TCMemoryManagerArenaHeap;

		umint mp_Size;
		TCMemoryManagerArenaHeap<t_CParams> *mp_pHeap;
		NContainer::TCBitArrayHierarchical<t_CParams::mc_HeapChunkSize / t_CParams::mc_HeapBlockSize> mp_Committed;
		NContainer::TCBitArrayHierarchical<t_CParams::mc_HeapChunkSize / t_CParams::mc_HeapBlockSize> mp_DeferredDecommit;

		ENumaArenaCleanup mp_RequestedCleanup;

		DMibListLinkDS_Link(TCMemoryManagerArenaHeapChunk, mp_HeapLink);
		DMibListLinkDS_Link(TCMemoryManagerArenaHeapChunk, mp_CleanupLink);

		NContainer::TCMap<uint8 *, CMemoryManagerArenaHeapBlock, NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>> mp_Blocks;

		int64 mp_DecommitTimestamp = 0;
		int64 mp_FreeTimestamp = 0;
	};

	template <typename t_CParams>
	struct TCMemoryManagerArenaHeap : public t_CParams::CNotifier::CHeap
	{
		TCMemoryManagerArenaHeap(TCMemoryManager<t_CParams> *_pMemoryManager, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
		~TCMemoryManagerArenaHeap();

		void f_Destroy();

		void *f_AllocWithSize(umint &_Size);
		void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment);
		void f_Free(void *_pMem, TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk);
		umint f_Size(void const *_pMemory, TCMemoryManagerArenaHeapChunk<t_CParams> const *_pChunk) const;
		fp32 f_Overhead(void const *_pMemory, TCMemoryManagerArenaHeapChunk<t_CParams> const *_pChunk) const;
		static umint fs_GetAllocSize(umint _Size);

		void f_Lock();
		void f_Unlock();
		void f_ForkedChild();

		int64 f_GarbageCollect(int64 _Timestamp);
		int64 f_DecommitDeferred(int64 _Timestamp);

		bool f_CheckFree(EMemoryManagerCheckFlag _Flags);

	private:
		static constexpr bool mcp_bEnableCallbacks = t_CParams::CNotifier::CHeap::mc_bEnableCallbacks;

		template <typename t_CParams2>
		friend struct TCMemoryManagerArenaHeapChunk;

		align_cacheline mutable NThread::CLowLevelLock mp_Lock;

		TCMemoryManager<t_CParams> *mp_pMemoryManager;
		TCMemoryManagerNumaArena<t_CParams> *mp_pNumaArena;

		NContainer::TCMap<umint, DMibListLinkDS_List_FromTemplate(CMemoryManagerArenaHeapBlock, m_Link), NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>> mp_FreeBuckets;

		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArenaHeapChunk<t_CParams>, mp_HeapLink) mp_Chunks;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArenaHeapChunk<t_CParams>, mp_CleanupLink) mp_ChunksNeedingCleanup;

		void fp_RemoveFreeBlock(CMemoryManagerArenaHeapBlock *_pBlock, TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk);

		void fp_AddNewChunk();

		void fp_InitBlockCommit(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size);
		void fp_CommitBlock(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size);
		void fp_DecommitBlockForReal(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size);
		void fp_DecommitBlock(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, umint _Size);

		void fp_TraceBlocks(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk);

		void fp_RequestCleanup(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, ENumaArenaCleanup _Cleanup);
	};
}
