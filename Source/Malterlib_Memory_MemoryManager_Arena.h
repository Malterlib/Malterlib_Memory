// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Container/BitArray>

namespace NMib
{
	namespace NMem
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
			void f_AddMessage(CMessage *_pMessage, EMessageType _MessageType);

			void f_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena);
			void f_FreeThisThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
			mint f_Size(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab);
			fp32 f_Overhead(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab);
			
			void *f_Alloc(mint &_Size);
			void f_AllocBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			
			bool f_CheckFree(bool _bBreak);
			
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
			
			typedef DMibListLinkDS_List(CMemoryManagerSubSlab_NormalLink, m_Link) CNormalFreeList;

		public:
			// Links need to be public
			DMibMemoryManagerLink(TCMemoryManagerArena, m_NumaArenaLink);
			DMibMemoryManagerLink(TCMemoryManagerArena, m_Link);
			DMibMemoryManagerLink(TCMemoryManagerArena, m_CleanupLink);
		private:			
			

			template <uint32 t_SlabType>
			TCMemoryManagerSlabShared<t_CParams> *fp_CreateSlab(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);

			template <mint tf_nSlabSizes>
			typename TCEnableIf<tf_nSlabSizes == 8, TCMemoryManagerSlabShared<t_CParams> *>::CType fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);
			template <mint tf_nSlabSizes>
			typename TCEnableIf<tf_nSlabSizes == 4, TCMemoryManagerSlabShared<t_CParams> *>::CType fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);
			template <mint tf_nSlabSizes>
			typename TCEnableIf<tf_nSlabSizes == 2, TCMemoryManagerSlabShared<t_CParams> *>::CType fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);
			template <mint tf_nSlabSizes>
			typename TCEnableIf<tf_nSlabSizes == 1, TCMemoryManagerSlabShared<t_CParams> *>::CType fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab);

			TCMemoryManagerSlabShared<t_CParams> *fp_NewSlab(uint32 _SlabType, uint32 _SizeType);

			
			template <mint tf_Size>
			TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> *fp_AllocSmallNoSlab();

			template <mint tf_Size>
			void *fp_AllocSmall();

			template <mint tf_Size>
			bool fp_CheckFreeSmall(bool _bBreak);
			
			void fp_FreeSmallSubSlabs(TCMemoryManagerSlabShared<t_CParams> *_pSlab);

			template <mint tf_Size>
			void fp_FreeSmall(void *_pAlloc, TCMemoryManagerSlab<t_CParams, 0> *_pSlab);

			static mint fps_GetSlabTypeFromSizeSmall(mint _Size);

			void *fp_AllocSmallSize(mint &_Size);

			void fp_AllocSmallSizeBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);

			void *fp_AllocNormalUncached(CNormalFreeList *_pList, mint _AlignedSize, mint _SubIndex, mint _SlabBucket);
			void *fp_AllocNormal(mint &_Size);

			void fp_AllocNormalBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			
			void fp_SlabHasGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
			void fp_CheckSlabNoLongerGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab);
			void fp_SubSlabNoLongerPending(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);

			int64 fp_GarbageCollectPerform(mint _SlabType, int64 _Timestamp);
			bool fp_GarbageCollectPerform(mint _SlabType);
			bool fp_GarbageCollect(mint _SlabType);
			void fp_GarbageCollectFull();
			
			static mint fs_GetAllocSize(mint _Size);

			void fp_FreeSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab);
			
			void fp_FreeSmall(void *_pMemory, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, mint _SlabType);
			mint fp_SizeSmall(void const *_pMemory, TCMemoryManagerSlab<t_CParams, 0> const *_pSlab, mint _SlabType) const;
			fp32 fp_OverheadSmall(void const *_pMemory, TCMemoryManagerSlab<t_CParams, 0> const *_pSlab, mint _SlabType) const;

			void fp_Free(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);
			void fp_FreeInline(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab);

			bool fp_CheckFree(CMemoryManagerSubSlab_NormalLink *_pLink, bool _bBreak);

			bool fp_ProcessMessages();

			void fp_CheckMessages();
			bool fp_CheckCleanup();
			bool fp_CheckCleanupNumaFree();

			void fp_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena);
			
			void fp_RequestCleanup();
		public:

			static const mint mc_MinAlignment = 4; // Can be 4 or 8
			static const mint mc_nSmallSizeSlabsAligned = NMib::TCHighestBitSetCorrect<mint, mc_MinAlignment>::mc_Value + 1;
			static const mint mc_nSmallSizeSlabs = mc_nSmallSizeSlabsAligned + (20 - mc_MinAlignment*2) / mc_MinAlignment;
			static const mint mc_MinAlignmentCalc = 16 / t_CParams::mc_NumSizesPerLevel;
			static const mint mc_MinNormalSizeAlignment = mc_MinAlignmentCalc < 4 ? 4 : mc_MinAlignmentCalc;
			static const mint mc_nLevel0Lists = (32 - sizeof(void *) * 2) / mc_MinNormalSizeAlignment;
			static const mint mc_nNormalSizeLists = t_CParams::mc_NumNormalSizeLevels-1;
			static const mint mc_Level0SmallestSize = 32 - mc_MinNormalSizeAlignment;
			static const mint mc_SmallSizeSlabsLargestSize = 16;
			static const mint mc_NumSubSlabSizeLevels = t_CParams::mc_NumSizeLevels - TCHighestBitSetCorrect<mint, t_CParams::mc_SubSlabSize>::mc_Value;
			
		private:
			
			DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link0) m_FreeSlabs;
			DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link0) m_PartiallyFreeSlabs[t_CParams::mc_NumSizesPerLevel][mc_NumSubSlabSizeLevels];
			NContainer::TCBitArray<mc_NumSubSlabSizeLevels> m_PartiallyFreeSlabsAvailable[t_CParams::mc_NumSizesPerLevel];

			DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link0) m_FullSlabs;

			DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link1) m_SlabsToGarbageCollect[t_CParams::mc_NumSizesPerLevel];
			DMibMemoryManagerList_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link2) m_SlabsNeedingDecommit;

			
			align_cacheline NAtomic::TCAtomic<mint> m_pNextArena;
			align_cacheline NAtomic::TCAtomic<uint32> m_Locked;
			align_cacheline NAtomic::TCAtomic<mint> m_Messages;
			
			uint64 m_Magic;
			
			mint m_CheckoutCount;
			
			DMibMemoryManagerList(CMemoryManagerSubSlab_SmallSizeLink, m_Link) m_SmallSizeSlabsFull;
			DMibMemoryManagerList(CMemoryManagerSubSlab_SmallSizeLink, m_Link) m_SmallSizeSlabs[mc_nSmallSizeSlabs];
			
			CNormalFreeList m_NormalSizeSlabsLevel0[mc_nLevel0Lists];
			CNormalFreeList m_NormalSizeSlabs[t_CParams::mc_NumSizesPerLevel][mc_nNormalSizeLists];
			
			TCMemoryManager<t_CParams> *m_pMemoryManager;
			TCMemoryManagerNumaArena<t_CParams> *m_pNumaArena;
			ENumaNode m_NumaNode;
			
			bool m_bWantCleanup;
			bool m_bRequestedCleanup;
			bool m_bWantNumaFreeSlabsCleanup;
		};
	}
}
