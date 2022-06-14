// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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

	enum EArenaLockFlag
	{
		EArenaLockFlag_None
		, EArenaLockFlag_Normal = DMibBit(0)
		, EArenaLockFlag_Cleanup = DMibBit(1)
		, EArenaLockFlag_Waiting = DMibBit(2)
	};

	template <typename t_CParams>
	struct align_cacheline TCMemoryManagerArena : public t_CParams::CNotifier::CArena
	{
	public:
		TCMemoryManagerArena(TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic, ENumaNode _NumaNode, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
		~TCMemoryManagerArena();

		int64 f_GarbageCollect(ENumaArenaCleanup &_oCleanup, int64 _Timestamp);
		int64 f_DecommitDeferred(int64 _Timestamp);

		bool f_ProcessMessages();
		void f_AddMessage(CMessage *_pMessage, EMessageType _MessageType, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);

		void f_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
		void f_FreeThisThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		mint f_Size(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab);
		fp32 f_Overhead(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab);

		void *f_AllocWithSize(mint &_Size);
		void f_AllocBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);

		bool f_CheckFree(EMemoryManagerCheckFlag _Flags);

		mint f_GetNumUsedSlabs();
		mint f_GetNumFreeSlabs();

		bool f_ReturnCheckout();
		void f_ReturnCheckoutLight();

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

		using CMemoryManagerSubSlab_NormalLink = typename TCChooseType
			<
				t_CParams::mc_bUseFreeBlockCounting
				, CMemoryManagerSubSlab_NormalLinkWithBlocks
				, CMemoryManagerSubSlab_NormalLinkWithoutBlocks
			>
			::CType
		;

		using CMemoryManagerSubSlab_NormalFreeList = typename TCChooseType
			<
				t_CParams::mc_bUseFreeBlockCounting
				, CMemoryManagerSubSlab_NormalFreeListWithBlocks
				, CMemoryManagerSubSlab_NormalFreeListWithoutBlocks
			>
			::CType
		;

	public:
		// Links need to be public
		DMibMemoryManagerLink(TCMemoryManagerArena, m_NumaArenaLink);
		DMibMemoryManagerLink(TCMemoryManagerArena, m_Link);
		DMibMemoryManagerLink(TCMemoryManagerArena, m_CleanupLink);
	private:

		template <uint32 t_SlabType>
		TCMemoryManagerSlabShared<t_CParams> *fp_CreateSlab(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);

		TCMemoryManagerSlabShared<t_CParams> *fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);

		TCMemoryManagerSlabShared<t_CParams> *fp_NewSlab(uint32 _SlabType, uint32 _SizeType);


		template <mint tf_Size>
		TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> *fp_AllocSmallNoSlab();

		template <mint tf_Size>
		static void *fsp_AllocSmall(TCMemoryManagerArena *_pThis);

		static void *fsp_AllocSmallShared(TCMemoryManagerArena *_pThis, mint _Index);

		template <mint tf_Size>
		bool fp_CheckFreeSmall(EMemoryManagerCheckFlag _Flags);

		bool fp_FreeSmallSubSlabs(TCMemoryManagerSlabShared<t_CParams> *_pSlab);

		void fp_FreeSmallShared(TCMemoryManagerSubSlab_SmallSizeShared<t_CParams> *_pSubSlab, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, mint _Index, ESmallState _SmallState);

		static mint fsp_GetSlabTypeFromSizeSmall(mint &o_Size);

		void *fp_AllocSmallSize(mint &_Size);

		void fp_AllocSmallSizeBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);

		void *fp_AllocNormalUncached(CMemoryManagerSubSlab_NormalFreeList *_pList, mint _AlignedSize, mint _SubIndex, mint _SlabBucket);
		void *fp_AllocNormal(mint &_Size);

		void fp_AllocNormalBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);

		void fp_SlabHasGarbageInline(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_SlabHasGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_CheckSlabNoLongerGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_SubSlabNoLongerPending(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);

		int64 fp_GarbageCollectPerform(mint _SlabType, int64 _Timestamp);
		bool fp_GarbageCollectPerform(mint _SlabType);
		bool fp_GarbageCollect(mint _SlabType);
		void fp_GarbageCollectFull();

		static mint fs_GetAllocSize(mint _Size);

		[[nodiscard]] bool fp_FreeSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);

		void fp_FreeSmall(void *_pMemory, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, mint _SlabType);
		mint fp_SizeSmall(void const *_pMemory, TCMemoryManagerSlab<t_CParams, 0> const *_pSlab, mint _SlabType) const;
		fp32 fp_OverheadSmall(void const *_pMemory, TCMemoryManagerSlab<t_CParams, 0> const *_pSlab, mint _SlabType) const;

		void fp_Free(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
		void fp_FreeInline(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);

		bool fp_CheckFree(CMemoryManagerSubSlab_NormalLink *_pLink, EMemoryManagerCheckFlag _Flags);

		bool fp_ProcessMessages(CMemoryManagerSubSlab_NormalFreeList *_pFreeList);
		bool fp_ProcessMessageList(CMemoryManagerSubSlab_NormalFreeList *_pFreeList, mint &o_MessageList);


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
		static constexpr mint mc_MinAllocSize = mc_bUseSmallSizes ? 1 : t_CParams::mc_MinNormalAllocSize;
		static constexpr mint mc_MinAlignment = 4; // Can be 4 or 8
		static constexpr mint mc_nSmallSizeSlabsAligned = NMib::TCHighestBitSetCorrect<mint, mc_MinAlignment>::mc_Value + 1;
		static constexpr mint mc_nSmallSizeSlabs = mc_bUseSmallSizes ? (mc_nSmallSizeSlabsAligned + (20 - mc_MinAlignment*2) / mc_MinAlignment) : mc_MinArraySize;
		static constexpr mint mc_nLevel0Lists = mc_bUseSmallSizes ? (32 - sizeof(void *) * 2) / t_CParams::mc_MinNormalSizeAlignment : mc_MinArraySize;
		static constexpr mint mc_nNormalSizeLists = mc_bUseSmallSizes ? t_CParams::mc_NumNormalSizeLevels-1 : t_CParams::mc_NumNormalSizeLevels;
		static constexpr mint mc_Level0SmallestSize = 32 - t_CParams::mc_MinNormalSizeAlignment;
		static constexpr bool mc_EnableCallbacks = t_CParams::CNotifier::CArena::mc_EnableCallbacks;
		static constexpr mint mc_MessagesSpread = 16;

		static_assert(mc_bUseSmallSizes || sizeof(void *) > 4, "Not supported on 32 bit");

	private:
		static constexpr mint mc_SmallAllocCategoryJumpTableSize = mc_bUseSmallSizes ? (mc_bUseFreeBlockCounting ? 6 : 5) : 1;

		using FSmallAllocJump = void *(*)(TCMemoryManagerArena *);
		struct CSmallAllocJumbTable
		{
			FSmallAllocJump m_Table[mc_SmallAllocCategoryJumpTableSize];
		};
		static constexpr CSmallAllocJumbTable mc_SmallAllocCategoryJumpTable = []() constexpr -> CSmallAllocJumbTable
			{
				if constexpr (mc_bUseSmallSizes)
				{
					if constexpr (mc_bUseFreeBlockCounting)
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


		align_cacheline NAtomic::TCAtomic<mint> m_pNextArena = 0;
		align_cacheline NAtomic::TCAtomic<mint> m_Locked = EArenaLockFlag_None;
		align_cacheline NAtomic::TCAtomic<mint> m_MessagesAvailable = 0;
		struct CSpreadMessage
		{
			align_cacheline NAtomic::TCAtomic<mint> m_Messages = 0;
		};
		CSpreadMessage m_SpreadMessages[mc_MessagesSpread];
		mint m_DeferredMessages[mc_MessagesSpread] = {};

		uint64 m_Magic = 0;

		NAtomic::TCAtomic<mint> m_CheckoutCount = 0;

		DMibMemoryManagerList(CMemoryManagerSubSlab_SmallSizeLink, m_Link) m_SmallSizeSlabsFull;
		DMibMemoryManagerList(CMemoryManagerSubSlab_SmallSizeLink, m_Link) m_SmallSizeSlabs[mc_nSmallSizeSlabs];

		CMemoryManagerSubSlab_NormalFreeList m_NormalSizeSlabsLevel0[mc_nLevel0Lists];
		CMemoryManagerSubSlab_NormalFreeList m_NormalSizeSlabs[t_CParams::mc_NumSizesPerLevel][mc_nNormalSizeLists];

		TCMemoryManager<t_CParams> *m_pMemoryManager;
		TCMemoryManagerNumaArena<t_CParams> *m_pNumaArena;
		TCMemoryManagerThreadLocal<t_CParams> *m_pOwningThreadLocal = nullptr;
		ENumaNode m_NumaNode;

		bool m_bWantCleanup = false;
		bool m_bRequestedCleanup = false;
		bool m_bWantNumaFreeSlabsCleanup = false;
	};
}
