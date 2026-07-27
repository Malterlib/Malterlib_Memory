// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	enum EMemoryManagerFreeStore
	{
		EMemoryManagerFreeStore_ArenaBlockLists // Arena-wide per-size-class lists of free blocks (doubly linked through the blocks)
		, EMemoryManagerFreeStore_SubSlabLists // Per-size-class lists of sub-slabs; free blocks chained per sub-slab with bump carving
		, EMemoryManagerFreeStore_SubSlabBitmaps // Per-size-class lists of sub-slabs; free blocks tracked per sub-slab with bitmaps returning lowest address first
	};

	struct CMemoryManagerSubSlab_Free
	{
		DMibMemoryManagerLink(CMemoryManagerSubSlab_Free, m_Link);
	};

	struct CMemoryManagerSubSlab_SmallSizeLink
	{
		DMibMemoryManagerLink(CMemoryManagerSubSlab_SmallSizeLink, m_Link);
	};

	DMibPStartPackedStruct;
	struct DMibPPackedStruct CMemoryManagerSubSlab_NormalLinkWithBlocks
	{
		CMemoryManagerSubSlab_NormalLinkWithBlocks() = delete;
		~CMemoryManagerSubSlab_NormalLinkWithBlocks() = delete;

		DMibPPackedStruct DMibListLinkDS_Link(CMemoryManagerSubSlab_NormalLinkWithBlocks, m_Link);
		DMibPPackedStruct uint32 m_nBlocks;
	};
	DMibPEndPackedStruct;

	struct CMemoryManagerSubSlab_NormalLinkWithoutBlocks
	{
		CMemoryManagerSubSlab_NormalLinkWithoutBlocks() = delete;
		~CMemoryManagerSubSlab_NormalLinkWithoutBlocks() = delete;

		DMibListLinkDS_Link(CMemoryManagerSubSlab_NormalLinkWithoutBlocks, m_Link);
	};

	using CMemoryManagerSubSlab_NormalFreeListWithBlocks = DMibListLinkDS_List(CMemoryManagerSubSlab_NormalLinkWithBlocks, m_Link);
	using CMemoryManagerSubSlab_NormalFreeListWithoutBlocks = DMibListLinkDS_List(CMemoryManagerSubSlab_NormalLinkWithoutBlocks, m_Link);

	// EMemoryManagerFreeStore_SubSlabLists: per-size-class lists link sub-slabs (one node per
	// sub-slab in the slab header) instead of individual free blocks
	struct CMemoryManagerSubSlab_ListNode
	{
		DMibListLinkDS_Link(CMemoryManagerSubSlab_ListNode, m_Link);
	};

	using CMemoryManagerSubSlab_SubSlabList = DMibListLinkDS_List(CMemoryManagerSubSlab_ListNode, m_Link);

	// Free/carve state for one sub-slab. The chain threads next pointers through the first
	// pointer-size bytes of each free block, so the allocation fast path is a single-branch
	// pop like mimalloc's page free list. The region [m_CarveOffset, m_CarveEnd) (quarter-byte
	// offsets relative to the sub-slab start) has never been handed out; carved blocks are
	// threaded into the chain in ascending address order one batch at a time by the allocation
	// slow path, so a fully collected sub-slab restarts in ascending address order.
	struct CMemoryManagerSubSlabFreeState
	{
		// Sentinel for the uint16 quarter-offset links of the cross-thread remote free chain
		static constexpr uint16 mc_ChainEnd = 0xFFFF;

		void *m_pFreeHead;
		uint16 m_CarveOffset;
		uint16 m_CarveEnd;
		uint16 m_CarveStart;
	};

	// EMemoryManagerFreeStore_SubSlabBitmaps: bit set = block free, bit index = block index in
	// the sub-slab, so find-lowest-set returns blocks in ascending address order. For sub-slabs
	// with at most 64 blocks m_Bits is the bitmap itself; for larger sub-slabs the bitmap words
	// live in-band at the sub-slab start and m_Bits is their nonzero-summary (bit w set = word w
	// has free blocks). Either way m_Bits == 0 means the sub-slab is exhausted.
	struct CMemoryManagerSubSlabFreeBits
	{
		uint64 m_Bits;
	};

	// mc_bReapInCleanup with SubSlabBitmaps: cross-thread frees fetch_or their block bit here
	// instead of pushing a message; the reap exchanges the word to 0 and ORs it into the free
	// bitmap while holding the arena. Only used for sub-slabs whose bitmap fits the header slot
	// (at most 64 blocks); dense in-band sub-slabs keep the message path.
	struct CMemoryManagerSubSlabRemoteFreeBits
	{
		NAtomic::TCAtomic<uint64> m_Bits;
	};

	// mc_bReapInCleanup with SubSlabLists: cross-thread frees CAS-push their block onto this
	// chain (uint16 quarter-offset next links threaded through the blocks, as the free chain).
	// m_Head is 0 when empty, otherwise the quarter-offset of the chain head plus one.
	struct CMemoryManagerSubSlabRemoteFreeChain
	{
		NAtomic::TCAtomic<uint32> m_Head;
	};

	// Per-slab cross-thread notification state for mc_bReapInCleanup; lives on its own cache
	// line in the slab header so fetch_or traffic from freeing threads does not false-share the
	// owner-hot header fields
	struct CMemoryManagerSlabRemoteFreeNotify
	{
		NAtomic::TCAtomic<uint64> m_SummaryTop;
		NAtomic::TCAtomic<umint> m_StackNext;
	};

	struct CMemoryManagerSubSlab_GarbageCollect
	{
		DMibMemoryManagerLink(CMemoryManagerSubSlab_GarbageCollect, m_LinkArena);
		DMibMemoryManagerLink(CMemoryManagerSubSlab_GarbageCollect, m_LinkSlab);
	};
}
