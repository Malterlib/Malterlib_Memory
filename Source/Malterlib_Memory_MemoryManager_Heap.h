// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Container/BitArrayHierarchical>

namespace NMib::NMemory
{
	enum ENumaArenaCleanup
	{
		ENumaArenaCleanup_None = 0
		, ENumaArenaCleanup_HeapGarbage = DMibBit(0)
		, ENumaArenaCleanup_HeapCommit = DMibBit(1)
		, ENumaArenaCleanup_ProcessMessages = DMibBit(2)
		, ENumaArenaCleanup_FreeSlabs = DMibBit(3)
	};

	struct CMemoryManagerGarbageOptions
	{
		int64 m_Timestamp;
		int64 m_TimestampDecommit;
	};

	template <typename t_CParams>
	class TCMemoryManagerArenaHeap;

	enum EMemoryManagerArenaHeapBlockFlag
	{
		EMemoryManagerArenaHeapBlockFlag_None = 0
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
	class TCMemoryManagerArenaHeapChunk
	{
		template <typename t_CParams2>
		friend class TCMemoryManagerArenaHeap;

		mint m_Size;
		TCMemoryManagerArenaHeap<t_CParams> *m_pHeap;
		NContainer::TCBitArrayHierarchical<t_CParams::mc_HeapChunkSize / t_CParams::mc_HeapBlockSize> m_Committed;
		NContainer::TCBitArrayHierarchical<t_CParams::mc_HeapChunkSize / t_CParams::mc_HeapBlockSize> m_DeferredDecommit;

		ENumaArenaCleanup m_RequestedCleanup;

		DMibListLinkDS_Link(TCMemoryManagerArenaHeapChunk, m_HeapLink);
		DMibListLinkDS_Link(TCMemoryManagerArenaHeapChunk, m_CleanupLink);


		NContainer::TCMap<uint8 *, CMemoryManagerArenaHeapBlock, NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>> m_Blocks;

		int64 m_DecommitTimestamp = 0;
		int64 m_FreeTimestamp = 0;

	public:

		TCMemoryManagerArenaHeapChunk(mint _Size, TCMemoryManagerArenaHeap<t_CParams> *_pHeap);
		~TCMemoryManagerArenaHeapChunk();

		uint8 *f_GetBlockAddress(CMemoryManagerArenaHeapBlock const *_pBlock) const;
		mint f_GetBlockSize(CMemoryManagerArenaHeapBlock const *_pBlock) const;

		uint8 *f_GetAddress() const;
		uint8 *f_GetEndAddress() const;
		mint f_GetSize() const;
		TCMemoryManagerArenaHeap<t_CParams> *f_GetHeap() const;

		bool f_IsEmpty();
	};

	template <typename t_CParams>
	class TCMemoryManagerArenaHeap : public t_CParams::CNotifier::CHeap
	{
		static constexpr bool mc_EnableCallbacks = t_CParams::CNotifier::CHeap::mc_EnableCallbacks;

		template <typename t_CParams2>
		friend class TCMemoryManagerArenaHeapChunk;

		align_cacheline mutable NThread::CLowLevelLock m_Lock;

		TCMemoryManager<t_CParams> *m_pMemoryManager;
		TCMemoryManagerNumaArena<t_CParams> *m_pNumaArena;

		NContainer::TCMap<mint, DMibListLinkDS_List_FromTemplate(CMemoryManagerArenaHeapBlock, m_Link), NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>> m_FreeBuckets;

		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArenaHeapChunk<t_CParams>, m_HeapLink) m_Chunks;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArenaHeapChunk<t_CParams>, m_CleanupLink) m_ChunksNeedingCleanup;

		void fp_RemoveFreeBlock(CMemoryManagerArenaHeapBlock *_pBlock, TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk);

		void fp_AddNewChunk();

		void fp_InitBlockCommit(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, mint _Size);
		void fp_CommitBlock(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, mint _Size);
		void fp_DecommitBlockForReal(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, mint _Size);
		void fp_DecommitBlock(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, uint8 *_pAddress, mint _Size);

		void fp_TraceBlocks(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk);

		void fp_RequestCleanup(TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk, ENumaArenaCleanup _Cleanup);

	public:
		TCMemoryManagerArenaHeap(TCMemoryManager<t_CParams> *_pMemoryManager, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
		~TCMemoryManagerArenaHeap();

		void f_Destroy();

		void *f_AllocWithSize(mint &_Size);
		void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment);
		void f_Free(void *_pMem, TCMemoryManagerArenaHeapChunk<t_CParams> *_pChunk);
		mint f_Size(void const * _pMemory, TCMemoryManagerArenaHeapChunk<t_CParams> const *_pChunk) const;
		fp32 f_Overhead(void const * _pMemory, TCMemoryManagerArenaHeapChunk<t_CParams> const *_pChunk) const;
		static mint fs_GetAllocSize(mint _Size);

		void f_Lock();
		void f_Unlock();

		int64 f_GarbageCollect(int64 _Timestamp);
		int64 f_DecommitDeferred(int64 _Timestamp);

		bool f_CheckFree(EMemoryManagerCheckFlag _Flags);

	};
}
