// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Memory_MemoryManager_BackgroundCleanup.h"

namespace NMib
{
	namespace NMem
	{
		
		template <typename t_CParams>
		struct TCMemoryManager;

		template <typename t_CParams>
		struct TCMemoryManagerArena;
		
		template <typename t_CParams, uint32 t_SlabType>
		struct TCMemoryManagerSlab;
		
		template <typename t_CParams>
		class TCMemoryManagerArenaHeap;

		template <typename t_CParams>
		struct align_cacheline TCMemoryManagerThreadLocal
		{
			TCMemoryManagerNumaArena<t_CParams> *m_pNumaArena;
			TCMemoryManagerArena<t_CParams> *m_pArena;
			TCMemoryManagerArena<t_CParams> *m_pPreferredArena;
			mint m_TemporaryReturnCheckoutCount;
			TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal const& _Other);
		public:
			TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal && _Other, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
			TCMemoryManagerThreadLocal(TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
			~TCMemoryManagerThreadLocal();

			void f_ReturnCheckout();
			void f_ReturnCheckoutLight();
			void f_TemporaryReturn();
			void f_TemporaryGetBack();
			void f_TakeOwnership();
			void f_RelinquishOwnership();
		};
		
		template <typename t_CParams>
		struct align_cacheline TCMemoryManagerNumaArena
		{
		public:

			class CCompare
			{
			public:
				inline_small ENumaNode operator () (TCMemoryManagerNumaArena const &_Node) const
				{
					return _Node.m_NumaNode;
				}
			};
			DMibIntrusiveLinkT(TCMemoryManagerNumaArena, NIntrusive::TCAVLLink<>, m_Link);
			
		public:

			TCMemoryManagerNumaArena(ENumaNode _NumaNode, TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic);
			~TCMemoryManagerNumaArena();

			TCMemoryManagerArena<t_CParams> *f_NewArena();
			
			void f_OnNeedCleanup();
			
			int64 f_GetTimestamp() const;
			
			void f_RequestCleanup(ENumaArenaCleanup _Cleanup);
			void f_RequestCleanupWeak(ENumaArenaCleanup _Cleanup);
			
			int64 f_GarbageCollect(int64 _Timestamp, bool _bDecommit, bool _bHasNumaArenasLock);
			
			bool f_ProcessArenaMessages(bool _bIncremental, bool & _oDeferred, bool _bHasNumaArenasLock);
			
			void f_ArenaAvailable(TCMemoryManagerArena<t_CParams> * _pArena);
			
			void f_CanStartThreads();


		private:
			
			template <typename t_CParams2>
			friend struct TCMemoryManager;
			
			template <typename t_CParams2>
			friend struct TCMemoryManagerArena;
			
			template <typename t_CParams2, uint32 t_SlabType2>
			friend struct TCMemoryManagerSlab;

			template <typename t_CParams2>
			friend class TCMemoryManagerArenaHeap;
			
			template <typename t_CParams2>
			friend struct TCMemoryManagerNumaArenaBackgroundCleanup;

			template <typename t_CParams2>
			friend struct TCMemoryManagerThreadLocal;
			
		private:
				

			TCPool<TCMemoryManagerArena<t_CParams>, 128, NThread::CMutual, NMem::CPoolType_Freeable, typename t_CParams::CAllocator> m_Pool;
			TCPool<TCMemoryManagerThreadLocal<t_CParams>, 128, NThread::CMutual, NMem::CPoolType_Freeable, typename t_CParams::CAllocator> m_PoolThreadLocal;
			TCMemoryManager<t_CParams> *m_pMemoryManager;

			TCMemoryManagerArenaHeap<t_CParams> m_Heap;
			
			align_cacheline NMib::NThread::CMutual m_ArenasLock;
			DMibListLinkDS_List_FromTemplate(TCMemoryManagerArena<t_CParams>, m_NumaArenaLink) m_Arenas;
			mint m_nArenas;

			DMibListLinkDS_List_FromTemplate(TCMemoryManagerArena<t_CParams>, m_CleanupLink) m_ArenasNeedCleanup;
			
			align_cacheline NAtomic::TCAtomic<TCMemoryManagerArena<t_CParams> *> m_pFirstArena;
			
			NThread::CEventAutoReset m_ArenaAvailableEvent;

			align_cacheline NMib::NThread::CMutual m_FreeSlabsLock;
			DMibListLinkDS_List_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link0) m_FreeSlabs;
			DMibListLinkDS_List_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link2) m_FreeSlabsNeedingDecommit;
			
			TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams> m_BackgroundCleanup;
			

			uint64 m_Magic;
			ENumaNode m_NumaNode;
			align_cacheline NAtomic::TCAtomic<uint32> m_RequestedCleanup;

		};

	}
}
