// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Container/BitArray>

namespace NMib::NMemory
{
	template <typename t_CParams>
	struct TCMemoryManagerThreadLocal;

	template <typename t_CParams>
	struct TCMemoryManager;

	template <typename t_CParams, uint32 t_SlabType>
	struct TCMemoryManagerSlab;

	template <typename t_CParams>
	struct TCMemoryManagerSlabShared;

	template <typename t_CParams>
	struct TCMemoryManagerNumaArena;

	template <typename t_CParams>
	struct align_cacheline TCMemoryManagerArena : public t_CParams::CNotifier::CArena
	{
	public:
		TCMemoryManagerArena(TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic, ENumaNode _NumaNode, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena, bool _bLimitedArenas);
		~TCMemoryManagerArena();

		int64 f_GarbageCollect(ENumaArenaCleanup &_oCleanup, int64 _Timestamp, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
		int64 f_DecommitDeferred(int64 _Timestamp);

		bool f_ProcessMessages();
		bool f_ProcessMessagesAbortable(bool &o_bAborted, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
		void f_AddMessage(CMessage *_pMessage, EMessageType _MessageType, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);

		void f_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
		void f_FreeThisThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		umint f_Size(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab);
		fp32 f_Overhead(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab);

		void *f_AllocWithSize(umint &_Size);
		void f_AllocBatch(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		bool f_CheckFree(EMemoryManagerCheckFlag _Flags);

		umint f_GetNumUsedSlabs();
		umint f_GetNumFreeSlabs();

		bool f_ReturnCheckout();
		void f_ReturnCheckoutLight();

		void f_ForkedChild();

		bool f_IsContended(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena) const;

	private:
		template <typename t_CParams2>
		friend struct TCMemoryManager;

		template <typename t_CParams2, uint32 t_SlabType2>
		friend struct TCMemoryManagerSlab;

		template <typename t_CParams2>
		friend struct TCMemoryManagerSlabShared;

		template <typename t_CParams2>
		friend struct TCMemoryManagerNumaArena;

		template <typename t_CParams2>
		friend struct TCMemoryManagerThreadLocal;

		using CMemoryManagerSubSlab_NormalLink = TCConditional
			<
				t_CParams::mc_bUseFreeBlockCounting
				, CMemoryManagerSubSlab_NormalLinkWithBlocks
				, CMemoryManagerSubSlab_NormalLinkWithoutBlocks
			>
		;

		using CMemoryManagerSubSlab_NormalFreeList = TCConditional
			<
				t_CParams::mc_bUseFreeBlockCounting
				, CMemoryManagerSubSlab_NormalFreeListWithBlocks
				, CMemoryManagerSubSlab_NormalFreeListWithoutBlocks
			>
		;

	public:
		static constexpr bool mc_bSubSlabStore = t_CParams::mc_FreeStoreMode != EMemoryManagerFreeStore_ArenaBlockLists;
		static constexpr bool mc_bSubSlabLists = t_CParams::mc_FreeStoreMode == EMemoryManagerFreeStore_SubSlabLists;
		static constexpr bool mc_bSubSlabBitmaps = t_CParams::mc_FreeStoreMode == EMemoryManagerFreeStore_SubSlabBitmaps;

		// With no block metadata every cross-thread free lands in the reap pending bits, so the
		// free-message machinery (spread stacks, availability counter, deferred lists) compiles
		// out entirely
		static constexpr bool mc_bUseFreeMessages = !t_CParams::mc_bNoBlockMetadata;

		// The reap pending-slab stack only exists when cross-thread frees land in pending state
		static constexpr bool mc_bUseRemoteFreeReap = mc_bSubSlabStore && t_CParams::mc_bReapInCleanup;
	private:

		// The per-size-class lists hold free blocks with ArenaBlockLists and sub-slabs with the
		// sub-slab store modes
		using CNormalFreeStoreList = TCConditional
			<
				mc_bSubSlabStore
				, CMemoryManagerSubSlab_SubSlabList
				, CMemoryManagerSubSlab_NormalFreeList
			>
		;

		// Sub-slab store modes: the sub-slab currently allocated from, per size-class list. It
		// is removed from its list while current; exhaustion is discovered lazily by the next
		// allocation, so a drained sub-slab can briefly stay cached as current and the relink
		// paths must not list it (chains guard on the slot, bitmaps keep the drained word's
		// summary bit stale-set until the slow path advances past it).
		//
		// The slot is padded to its power-of-two size (one 64-byte cache line with 64-bit
		// pointers) so slots never straddle a line, and the four fields the allocation fast
		// path reads live in its first half.
		struct alignas(fg_RoundPowerOfTwoUp(sizeof(void *) * 6 + sizeof(uint32) * 2)) CCurrentSubSlabBitmaps
		{
			// Cursor for the bitmap word being drained, so the fast path pops bits with a
			// single load/store and no summary hop (for sub-slabs of at most 64 blocks it
			// points at the header word itself)
			uint64 *m_pWord = nullptr;
			// m_pBase advanced to the cursor word's first block, so the fast path turns a bit
			// index straight into an address without reloading m_pBase and m_WordIndexBase
			uint8 *m_pWordBase = nullptr;
			TCMemoryManagerSubSlabDataAlloc<t_CParams> *m_pDataAlloc = nullptr;
			TCSubSlabFreeStore<t_CParams> *m_pFreeState = nullptr; // null = no current sub-slab

			uint8 *m_pBase = nullptr;
			TCMemoryManagerSlabShared<t_CParams> *m_pSlab = nullptr;
			uint32 m_iSubSlab = 0;
			uint32 m_WordIndexBase = 0;
		};

		struct alignas(fg_RoundPowerOfTwoUp(sizeof(void *) * 4 + sizeof(uint32))) CCurrentSubSlabLists
		{
			TCMemoryManagerSubSlabDataAlloc<t_CParams> *m_pDataAlloc = nullptr;
			TCSubSlabFreeStore<t_CParams> *m_pFreeState = nullptr; // null = no current sub-slab

			uint8 *m_pBase = nullptr;
			TCMemoryManagerSlabShared<t_CParams> *m_pSlab = nullptr;
			uint32 m_iSubSlab = 0;
		};

		static_assert(sizeof(CCurrentSubSlabBitmaps) == alignof(CCurrentSubSlabBitmaps), "The alignment expression should match the slot's fields");
		static_assert(sizeof(CCurrentSubSlabLists) == alignof(CCurrentSubSlabLists), "The alignment expression should match the slot's fields");

		struct CCurrentSubSlabEmpty
		{
		};

		using CCurrentSubSlab = TCConditional<mc_bSubSlabBitmaps, CCurrentSubSlabBitmaps, CCurrentSubSlabLists>;

		using CNormalFreeStoreCurrent = TCConditional<mc_bSubSlabStore, CCurrentSubSlab, CCurrentSubSlabEmpty>;

	public:
		// Links need to be public
		DMibMemoryManagerLink(TCMemoryManagerArena, m_NumaArenaLink);
		DMibMemoryManagerLink(TCMemoryManagerArena, m_Link);
		DMibMemoryManagerLink(TCMemoryManagerArena, m_FreeArenasLink);
		DMibMemoryManagerLink(TCMemoryManagerArena, m_CleanupLink);
	private:

		template <uint32 t_SlabType>
		TCMemoryManagerSlabShared<t_CParams> *fp_CreateSlab(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);

		TCMemoryManagerSlabShared<t_CParams> *fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);

		TCMemoryManagerSlabShared<t_CParams> *fp_NewSlab(uint32 _SlabType, uint32 _SizeType);


		template <umint tf_Size>
		TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> *fp_AllocSmallNoSlab();

		template <umint tf_Size>
		static void *fsp_AllocSmall(TCMemoryManagerArena *_pThis);

		static void *fsp_AllocSmallShared(TCMemoryManagerArena *_pThis, umint _Index);

		template <umint tf_Size>
		bool fp_CheckFreeSmall(EMemoryManagerCheckFlag _Flags);

		bool fp_FreeSmallSubSlabs(TCMemoryManagerSlabShared<t_CParams> *_pSlab);

		void fp_FreeSmallShared(TCMemoryManagerSubSlab_SmallSizeShared<t_CParams> *_pSubSlab, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, umint _Index, ESmallState _SmallState);

		static umint fsp_GetSlabTypeFromSizeSmall(umint &o_Size);

		void *fp_AllocSmallSize(umint &_Size);

		void fp_AllocSmallSizeBatch(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		void *fp_AllocNormalUncached(CNormalFreeStoreList *_pList, CNormalFreeStoreCurrent *_pCurrent, umint _AlignedSize, umint _SubIndex, umint _SlabBucket);
		void *fp_AllocNormal(umint &_Size);

		// Sub-slab store helpers
		static void fsp_GetCarveRange(uint8 *_pSubSlabStart, umint _AlignedSize, umint _SlabType, umint _SlabBucket, umint &o_StartOffset, umint &o_EndOffset);
		static TCMemoryManagerSlabShared<t_CParams> *fsp_SlabFromSubSlabNode(CMemoryManagerSubSlab_ListNode *_pNode, umint &o_iSubSlab);
		void fp_SetCurrentSubSlab(CNormalFreeStoreCurrent *_pCurrent, TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab);
		CNormalFreeStoreCurrent *fp_GetCurrentSubSlabSlot(umint _SlabType, umint _SlabBucket, umint _AlignedSize);
		CNormalFreeStoreList *fp_GetNormalFreeList(umint _SlabType, umint _SlabBucket, umint _AlignedSize);
		// Flat size-class number for the per-size-class arrays. Callers that already hold the
		// unnormalized sub index (before the align-up carry is folded into the size level) can
		// pass it directly: class (Level, mc_NumSizesPerLevel) is class (Level + 1, 0)
		static constexpr umint fs_NormalSizeClass(umint _SlabBucket, umint _SubIndex);
		bool fp_CheckFreeSubSlabChain(TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab, EMemoryManagerCheckFlag _Flags);

		// mc_bReapInCleanup: cross-thread frees land in per-sub-slab pending state, discovered
		// through an atomic per-slab summary and a per-arena stack of pending slabs
		inline_never void fp_PushRemoteFreeSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		[[nodiscard]] bool fp_ReapRemoteSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab);
		inline_never bool fp_ReapRemoteSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		template <bool tf_bAbortable>
		bool fp_ReapRemoteFrees(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena, CNormalFreeStoreList *_pFreeList);
		bool fp_HasRemoteFrees() const;

		// mc_bGlobalAddressOrder: cleanup-side re-sort of the per-size-class sub-slab lists
		void fp_SortSubSlabListsIfDirty();
		static void fsp_SortSubSlabList(CNormalFreeStoreList &_List);
		static void fsp_MergeSubSlabLists(CNormalFreeStoreList &_A, CNormalFreeStoreList &_B, CNormalFreeStoreList &o_Out);

		// SubSlabLists helper: thread the next batch of carved blocks into the free chain
		bool fp_ExtendSubSlabChain(CMemoryManagerSubSlabFreeState &_FreeState, uint8 *_pBase, umint _AlignedSize);

		// SubSlabBitmaps helpers
		static umint fsp_GetSubSlabBlocks(umint _SlabType, umint _SlabBucket);
		static umint fsp_BitmapBlockIndex(umint _ByteOffset, umint _SlabType, umint _SlabBucket);
		static umint fsp_BitmapRemoteWordsOffset(umint _nWords);
		static bool fsp_BitmapAdvanceCurrentWord(CNormalFreeStoreCurrent &_Current, umint _AlignedSize);
		static bool fsp_BitmapFreeBlock(CMemoryManagerSubSlabFreeBits &_Bits, uint8 *_pBase, void *_pMemory, umint _AlignedSize, umint _SlabType, umint _SlabBucket, umint _nBlocks);

		void fp_AllocNormalBatch(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		void fp_SlabHasGarbageInline(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_SlabHasGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_CheckSlabNoLongerGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_SubSlabNoLongerPending(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);

		int64 fp_GarbageCollectPerform(umint _SlabType, int64 _Timestamp, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
		bool fp_GarbageCollectPerform(umint _SlabType);
		bool fp_GarbageCollect(umint _SlabType);
		void fp_GarbageCollectFull(bool _bRetain = false);
		void fp_GarbageCollectConsolidate();
		inline_never void fp_ConsolidateSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);

		static umint fs_GetAllocSize(umint _Size);

		[[nodiscard]] bool fp_FreeSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);

		void fp_FreeSmall(void *_pMemory, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, umint _SlabType);
		umint fp_SizeSmall(void const *_pMemory, TCMemoryManagerSlab<t_CParams, 0> const *_pSlab, umint _SlabType) const;
		fp32 fp_OverheadSmall(void const *_pMemory, TCMemoryManagerSlab<t_CParams, 0> const *_pSlab, umint _SlabType) const;

		void fp_Free(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_FreeFromMessage(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_FreeInline(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		inline_never void fp_FreeSubSlabWentEmpty(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);
		inline_never void fp_FreeRelistSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab, umint _SlabType, umint _SlabBucket, umint _AlignedSize);

		bool fp_CheckFree(CMemoryManagerSubSlab_NormalLink *_pLink, EMemoryManagerCheckFlag _Flags);

		bool fp_ProcessMessages(CNormalFreeStoreList *_pFreeList);
		bool fp_ProcessMessagesAbortable(bool &o_bAborted, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
		bool f_HasPendingMessages() const;
		template <bool tf_bAbortable, bool tf_bFreeList>
		inline_never bool fp_ProcessMessageList(umint &o_MessageList, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena, CNormalFreeStoreList *_pFreeList, smint &_nToProcess);

		void fp_CheckMessages();
		bool fp_CheckCleanup();
		bool fp_CheckCleanupNumaFree();

		void fp_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);

		void fp_RequestCleanup();
	public:

#ifdef DCompiler_MSVC_Workaround
		static constexpr bool mc_MinArraySize = 1;
#else
		static constexpr bool mc_MinArraySize = 0;
#endif

		static constexpr bool mc_bUseFreeBlockCounting = t_CParams::mc_bUseFreeBlockCounting;
		static constexpr bool mc_bSpecialCaseSlabType0 = t_CParams::mc_bSpecialCaseSlabType0;
		static constexpr bool mc_bUseSmallSizes = t_CParams::mc_bUseSmallSizes;
		static constexpr umint mc_MinAllocSize = mc_bUseSmallSizes ? 1 : t_CParams::mc_MinNormalAllocSize;
		static constexpr umint mc_MinAlignment = 4; // Can be 4 or 8
		static constexpr umint mc_nSmallSizeSlabsAligned = NMib::gc_HighestBitSet<mc_MinAlignment> + 1;
		static constexpr umint mc_nSmallSizeSlabs = mc_bUseSmallSizes ? (mc_nSmallSizeSlabsAligned + (20 - mc_MinAlignment*2) / mc_MinAlignment) : mc_MinArraySize;
		static constexpr umint mc_nLevel0Lists = mc_bUseSmallSizes ? (32 - sizeof(void *) * 2) / t_CParams::mc_MinNormalSizeAlignment : mc_MinArraySize;
		static constexpr umint mc_nNormalSizeLists = mc_bUseSmallSizes ? t_CParams::mc_NumNormalSizeLevels-1 : t_CParams::mc_NumNormalSizeLevels;

		// The per-size-class arrays are indexed by one flat class number, size level major, so
		// neighbouring size classes are neighbours in memory
		static constexpr umint mc_nNormalSizeClasses = mc_nNormalSizeLists * t_CParams::mc_NumSizesPerLevel;

		static constexpr umint mc_Level0SmallestSize = 32 - t_CParams::mc_MinNormalSizeAlignment;
		static constexpr bool mc_EnableCallbacks = t_CParams::CNotifier::CArena::mc_EnableCallbacks;
#if defined(DArchitecture_arm64) || defined(DArchitecture_arm64e)
		static constexpr umint mc_MessagesSpread = 16;
#else
		static constexpr umint mc_MessagesSpread = 1;
#endif

		// Cross-thread frees store their delete message in the freed block itself, which the small
		// size slab rescue path in fp_FreeOtherThread only makes safe on 32 bit; stores that keep
		// no metadata in free blocks do not use delete messages at all and are exempt
		static_assert
			(
				mc_bUseSmallSizes || t_CParams::mc_bNoBlockMetadata || sizeof(void *) > 4
				, "Disabling the small size slabs on 32 bit is only supported for free stores that keep no metadata in free blocks"
			)
		;

	private:
		static constexpr umint mc_SmallAllocCategoryJumpTableSize = mc_bUseSmallSizes ? (t_CParams::mc_SmallSizeSlabsLargestSize == 16 ? 6 : 5) : 1;

		using FSmallAllocJump = void *(*)(TCMemoryManagerArena *);
		struct CSmallAllocJumbTable
		{
			FSmallAllocJump m_Table[mc_SmallAllocCategoryJumpTableSize];
		};
		static constexpr CSmallAllocJumbTable mc_SmallAllocCategoryJumpTable = []() constexpr -> CSmallAllocJumbTable
			{
				if constexpr (mc_bUseSmallSizes)
				{
					if constexpr (t_CParams::mc_SmallSizeSlabsLargestSize == 16)
					{
						return
							{
								&fsp_AllocSmall<1>
								, &fsp_AllocSmall<2>
								, &fsp_AllocSmall<4>
								, &fsp_AllocSmall<8>
								, &fsp_AllocSmall<12>
								, &fsp_AllocSmall<16>
							}
						;
					}
					else
					{
						return
							{
								&fsp_AllocSmall<1>
								, &fsp_AllocSmall<2>
								, &fsp_AllocSmall<4>
								, &fsp_AllocSmall<8>
								, &fsp_AllocSmall<12>
							}
						;
					}
				}
				else
					return {nullptr};
			}
			()
		;

		DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link) m_FreeSlabs;
#ifndef DDocumentation_Doxygen
		DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link) m_PartiallyFreeSlabs[t_CParams::mc_NumSizesPerLevel][t_CParams::mc_NumSubSlabSizeLevels];
#endif
		NContainer::TCBitArray<t_CParams::mc_NumSubSlabSizeLevels> m_PartiallyFreeSlabsAvailable[t_CParams::mc_NumSizesPerLevel];

		DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link) m_FullSlabs;

#ifndef DDocumentation_Doxygen
		DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_LinkToGarbageCollect) m_SlabsToGarbageCollect[t_CParams::mc_NumSizesPerLevel];
#endif
		DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_LinkNeedDecommit) m_SlabsNeedingDecommit;

		// The arena lock, its contention counter and the checkout count are all written by
		// other threads, so each gets a private cache line; the struct size rounds up to
		// whole lines, which keeps neighbouring members off all three lines regardless of
		// which conditional states below are empty
		struct CLockState
		{
			align_cacheline NThread::CLowLevelLock m_Lock;
			align_cacheline NAtomic::TCAtomic<umint> m_LockContended;
			align_cacheline NAtomic::TCAtomic<umint> m_CheckoutCount = 0;
		};

		CLockState m_LockState;

		struct CMessageState
		{
			align_cacheline NAtomic::TCAtomic<umint> m_MessagesAvailable = 0;
			struct CSpreadMessage
			{
				align_cacheline NAtomic::TCAtomic<umint> m_Messages = 0;
			};
			CSpreadMessage m_SpreadMessages[mc_MessagesSpread];
			umint m_DeferredMessages[mc_MessagesSpread] = {};
		};

		struct CMessageStateEmpty
		{
		};

		DMibNoUniqueAddress TCConditional<mc_bUseFreeMessages, CMessageState, CMessageStateEmpty> m_MessageState;

		// mc_bReapInCleanup: Treiber stack of slabs with pending cross-thread frees (slab
		// pointers linked through m_pRemoteFreeNotify->m_StackNext, 0 terminated). Pushed by the
		// freeing thread that observes the slab's empty-to-pending transition, popped only while
		// holding the arena.
		struct CRemoteFreeState
		{
			align_cacheline NAtomic::TCAtomic<umint> m_RemoteFreeSlabs = 0;
		};

		struct CRemoteFreeStateEmpty
		{
		};

		DMibNoUniqueAddress TCConditional<mc_bUseRemoteFreeReap, CRemoteFreeState, CRemoteFreeStateEmpty> m_RemoteFreeState;

		DMibMemoryManagerList(CMemoryManagerSubSlab_SmallSizeLink, m_Link) m_SmallSizeSlabsFull;
		DMibMemoryManagerList(CMemoryManagerSubSlab_SmallSizeLink, m_Link) m_SmallSizeSlabs[mc_nSmallSizeSlabs];

		CNormalFreeStoreList m_NormalSizeSlabsLevel0[mc_nLevel0Lists];
		CNormalFreeStoreList m_NormalSizeSlabs[mc_nNormalSizeClasses];

		struct CCurrentSubSlabs
		{
			CCurrentSubSlab m_Level0[mc_nLevel0Lists];
			CCurrentSubSlab m_Normal[mc_nNormalSizeClasses];
		};

		struct CCurrentSubSlabsEmpty
		{
		};

		DMibNoUniqueAddress TCConditional<mc_bSubSlabStore, CCurrentSubSlabs, CCurrentSubSlabsEmpty> m_CurrentSubSlabs;

		uint64 m_Magic = 0; // Read-only after construction, kept on the read-mostly line with the pointers below

		TCMemoryManager<t_CParams> *m_pMemoryManager;
		TCMemoryManagerNumaArena<t_CParams> *m_pNumaArena;
		ENumaNode m_NumaNode;
		uint8 m_iLimitedArena = 0;

		bool m_bWantCleanup = false;
		bool m_bRequestedCleanup = false;
		bool m_bWantNumaFreeSlabsCleanup = false;
		bool m_bLimitedArenas = false;
		bool m_bSubSlabListsDirty = false; // mc_bGlobalAddressOrder: a fast-path head insert broke the address order
	};
}
