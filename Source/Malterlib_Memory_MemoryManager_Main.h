// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Memory_MemoryManager_Checkout.h"

namespace NMib
{
	namespace NMem
	{
		template <typename t_CParams>
		struct TCMemoryManagerThreadLocal;
		
		template <typename t_CParams>
		class TCMemoryManagerArenaHeapChunk;
		
		template <typename t_CParams>
		class TCMemoryManagerArenaHeap;
		
		template <typename t_CParams2, uint32 t_SlabType2>
		struct TCMemoryManagerSlab;

		template <typename t_CParams>
		struct TCMemoryManagerNumaArena;

		template <typename t_CParams>
		struct TCMemoryManagerNumaArenaBackgroundCleanup;
		
		template <typename t_CParams>
		class TCMemoryManagerCheckout
		{
			TCMemoryManager<t_CParams> * m_pMemoryManager;

			TCMemoryManagerCheckout(TCMemoryManagerCheckout const &_Other);
			TCMemoryManagerCheckout &operator = (TCMemoryManagerCheckout const& _Other);
		public:
			TCMemoryManagerCheckout(TCMemoryManagerCheckout && _Other);
			TCMemoryManagerCheckout(TCMemoryManager<t_CParams> * _pMemoryManager);
			TCMemoryManagerCheckout &operator = (TCMemoryManagerCheckout && _Other);
			~TCMemoryManagerCheckout();
		};

		template <typename t_CParams>
		class TCMemoryManagerCheckoutLight
		{
			TCMemoryManager<t_CParams> * m_pMemoryManager;

			TCMemoryManagerCheckoutLight(TCMemoryManagerCheckoutLight const& _Other);
			TCMemoryManagerCheckoutLight & operator = (TCMemoryManagerCheckoutLight const& _Other);
		public:
			TCMemoryManagerCheckoutLight(TCMemoryManagerCheckoutLight && _Other);
			TCMemoryManagerCheckoutLight(TCMemoryManager<t_CParams> * _pMemoryManager);
			TCMemoryManagerCheckoutLight & operator = (TCMemoryManagerCheckoutLight && _Other);
			~TCMemoryManagerCheckoutLight();
		};
		
		struct align_cacheline CLocalNumaNode
		{
			ENumaNode m_Node;

			CLocalNumaNode(ENumaNode _Node)
				: m_Node(_Node)
			{
			}
			CLocalNumaNode()
				: m_Node(ENumaNode_Default)
			{
			}
		};
		
		template <typename t_CParams>
		struct TCMemoryManager : public t_CParams::CNotifier::CGlobal, ICMemoryManagerReturnCheckout
		{
		public:
			
			template <typename... tfp_CAllocator>
			TCMemoryManager(tfp_CAllocator &&..._Params);
			
			~TCMemoryManager();

			TCMemoryManagerCheckout<t_CParams> f_Checkout();
			CMemoryManagerCheckout f_CheckoutVirtual();
			void f_CheckoutManual();
			void f_CheckinManual();
			void f_CheckinManualLight();

			void f_SetNumaNode(ENumaNode _NumaNode);
			void *f_Alloc(mint & _Size);
			void *f_AllocInline(mint & _Size);
			void *f_AllocAligned(mint & _Size, mint _Alignment);
			void *f_AllocAlignedInline(mint & _Size, mint _Alignment);
			void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			void *f_Realloc(void * _pMemory, mint &_Size);
			void *f_ReallocInline(void * _pMemory, mint &_Size);
			void *f_Resize(void * _pMemory, mint &_Size);
			void *f_ResizeInline(void * _pMemory, mint &_Size);
			mint f_Size(void const * _pMemory) const;
			mint f_TrySize(void const * _pMemory) const; // Warning, this function will cause can cause an access violatino if _pMemory is not part of this heap, it's up to the caller to handle this exception
			mint f_SizeInline(void const * _pMemory) const;
			fp32 f_Overhead(void const * _pMemory);
			bool f_ContainsBlock(void const * _pMemory);
			
			void f_Free(void * _pMemory);
			void f_FreeInline(void * _pMemory);
			mint f_SizePadded(mint _Size);
			
			void f_PrepareFork();
			void f_ForkedChild();
			void f_ForkedParent();
			void f_Lock();
			void f_Unlock();
			
			void f_EnumArenas(NFunction::TCFunctionNoAlloc<void (typename t_CParams::CNotifier::CArena *)> const &_Functor);
			void f_EnumHeaps(NFunction::TCFunctionNoAlloc<void (typename t_CParams::CNotifier::CHeap *)> const &_Functor);
			void f_EnumGlobal(NFunction::TCFunctionNoAlloc<void (typename t_CParams::CNotifier::CGlobal *)> const &_Functor);
			
			void f_GarbageCollect(bool _bDecommit);
			void f_WaitForBackgroundCleanup(); // Should only be used for unit tests

			void f_CanStartThreads();
			void f_DestroyCleanupThreads();
			
			mint f_GetNumUsedSlabs();
			mint f_GetNumFreeSlabs();
			
			bool f_CheckFree(bool _bBreak);

			void f_DestroyThreadLocals();

			void f_SetMaxArenas(mint _nArenas);
			
		private:

			void *fp_AllocSlowPath(mint & _Size);
			void *fp_AllocAlignedSlowPath(mint & _Size, mint _Alignment);
			void fp_AllocBatchSlowPath(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			
			TCMemoryManagerArena<t_CParams> *fp_CheckoutHelper(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);
			TCMemoryManagerArena<t_CParams> *fp_CheckoutHelperSlowPath(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);
			TCMemoryManagerArena<t_CParams> *fp_CheckoutHelperWaitForCleanup(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);
			TCMemoryManagerArena<t_CParams> *fp_CheckoutHelperWaitUnlock(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);

			TCMemoryManagerCheckoutLight<t_CParams> fp_Checkout(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);
			
			void *fp_AllocWithCheckout(mint &_Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena);

			void fp_AllocBatchWithCheckout(mint _Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);

			void fp_ProcessArenaMessages();
			
			void fp_EnumArenas(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArena<t_CParams> *)> const &_Functor, bool _bCleanup);
			void fp_EnumHeaps(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArenaHeap<t_CParams> *)> const &_Functor);

			void f_ReturnCheckoutVirtual() override;
			void f_TemporaryReturn() override;
			void f_TemporaryGetBack() override;
			void f_TakeOwnership() override;
			void f_RelinquishOwnership() override;

		private:
			
			template <typename t_CParams2>
			friend class TCMemoryManagerArenaHeapChunk;
			
			template <typename t_CParams2>
			friend struct TCMemoryManagerArena;
			
			template <typename t_CParams2>
			friend class TCMemoryManagerArenaHeap;
			
			template <typename t_CParams2, uint32 t_SlabType2>
			friend struct TCMemoryManagerSlab;
			
			template <typename t_CParams2>
			friend struct TCMemoryManagerNumaArena;

			template <typename t_CParams2>
			friend struct TCMemoryManagerNumaArenaBackgroundCleanup;

			template <typename t_CParams2>
			friend struct TCMemoryManagerThreadLocal;

			
			static_assert(TCIsPowerOfTwo<t_CParams::mc_SubSlabSize>::mc_Value, "Must be aligned to power of two");
			static_assert(TCIsPowerOfTwo<t_CParams::mc_SlabSize>::mc_Value, "Must be aligned to power of two");

			typename t_CParams::CAllocator m_Allocator;
			
			mutable align_cacheline NThread::CMutual m_NumaArenasLock;
			NIntrusive::TCAVLTree<typename TCMemoryManagerNumaArena<t_CParams>::CLinkTraits_m_Link, typename TCMemoryManagerNumaArena<t_CParams>::CCompare> m_NumaArenas;


			align_cacheline NMib::NThread::CMutual m_nArenasLock;
			mint m_nMaxArenas;
			mint m_nArenas;

			uint64 m_Magic;

			TCPool<CLocalNumaNode, 8, NThread::CMutual, NMem::CPoolType_Freeable, typename t_CParams::CAllocator> m_LocalNumaNodePool;

			NThread::TCThreadLocalDynamic
				<
					CLocalNumaNode
					, NThread::EThreadLocalFlag_Inherit
				> m_LocalNumaNode
			;

			NThread::TCThreadLocalDynamic
				<
					TCMemoryManagerThreadLocal<t_CParams>
					, NThread::EThreadLocalFlag(int(NThread::EThreadLocalFlag_AlwaysCreated) | int(NThread::EThreadLocalFlag_FastThreadLocal))
				> m_LocalArena
			;
			
			mutable NThread::CMutualManyRead m_HeapChunksLock;
			NContainer::TCMap<uint8 *, TCMemoryManagerArenaHeapChunk<t_CParams>, NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>> m_HeapChunks;

		};
		
	}
}

