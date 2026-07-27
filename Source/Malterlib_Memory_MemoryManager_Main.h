// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "Malterlib_Memory_MemoryManager_Checkout.h"

namespace NMib::NMemory
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
		TCMemoryManager<t_CParams> *m_pMemoryManager = nullptr;

		TCMemoryManagerCheckout(TCMemoryManagerCheckout const &) = delete;
		TCMemoryManagerCheckout &operator = (TCMemoryManagerCheckout const &) = delete;

	public:
		TCMemoryManagerCheckout() = default;
		TCMemoryManagerCheckout(TCMemoryManagerCheckout &&_Other);
		TCMemoryManagerCheckout(TCMemoryManager<t_CParams> *_pMemoryManager);
		TCMemoryManagerCheckout &operator = (TCMemoryManagerCheckout<t_CParams> &&_Other);
		~TCMemoryManagerCheckout();

		bool f_IsValid() const;
	};

	template <typename t_CParams>
	class TCMemoryManagerCheckoutLight
	{
		TCMemoryManager<t_CParams> *m_pMemoryManager;

		TCMemoryManagerCheckoutLight(TCMemoryManagerCheckoutLight const &) = delete;
		TCMemoryManagerCheckoutLight &operator = (TCMemoryManagerCheckoutLight const &) = delete;

	public:
		TCMemoryManagerCheckoutLight(TCMemoryManagerCheckoutLight &&_Other);
		TCMemoryManagerCheckoutLight(TCMemoryManager<t_CParams> *_pMemoryManager);
		TCMemoryManagerCheckoutLight &operator = (TCMemoryManagerCheckoutLight<t_CParams> &&_Other);
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

	struct CMemoryManagerConfig
	{
#if DMibPPtrBits >= 64
		uint32 m_nMaxArenas = 0;
#elif DMibPPtrBits == 32
		uint32 m_nMaxArenas = 8;
#else
#	error "Decide max arenas"
#endif
		uint64 m_Magic = NMisc::fg_GetHighEntropyRandomInteger<uint64>();
	};

	template <typename t_CParams>
	struct TCMemoryManager : public t_CParams::CNotifier::CGlobal, ICMemoryManagerReturnCheckout
	{
		using CParams = t_CParams;

		template <typename... tfp_CAllocator>
		TCMemoryManager(CMemoryManagerConfig const &_Config, tfp_CAllocator &&..._Params);

		~TCMemoryManager();

		TCMemoryManagerCheckout<t_CParams> f_Checkout();
		TCMemoryManagerCheckout<t_CParams> f_CheckoutForce();
		CMemoryManagerCheckout f_CheckoutVirtual();
		void f_CheckoutManual();
		void f_CheckinManual();
		void f_CheckinManualLight();
		bool f_IsCheckedOut();

		void f_LazyReturnCheckout();
		void f_LazyReturnCheckoutPrevent();
		void f_LazyReturnCheckoutAllow();
		void f_CanDoLazyCheckout();

		void f_SetNumaNode(ENumaNode _NumaNode);
		void *f_AllocWithSize(umint &_Size);
		void *f_AllocWithSizeInline(umint &_Size);
		void *f_Alloc(umint _Size);
		void *f_AllocInline(umint _Size);
		void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment);
		void *f_AllocAlignedWithSizeInline(umint &_Size, umint _Alignment);
		inline_never void *f_AllocAligned(umint _Size, umint _Alignment);
		void *f_AllocAlignedInline(umint _Size, umint _Alignment);
		void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);
		void *f_Realloc(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_ReallocInline(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_Resize(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_ResizeInline(void * _pMemory, umint &_Size, umint _OldSize);
		umint f_Size(void const * _pMemory) const;
		umint f_TrySize(void const * _pMemory) const; // Warning, this function will cause can cause an access violatino if _pMemory is not part of this heap, it's up to the caller to handle this exception
		umint f_SizeInline(void const * _pMemory) const;
		fp32 f_Overhead(void const * _pMemory);
		bool f_ContainsBlock(void const * _pMemory);

		uint64 f_GetMagic() const;
		TCMemoryManager *f_GetMemoryManager(void const *_pMemory); // Will only work between managers that share the same magic

		void f_Free(void * _pMemory, umint _Size);
		void f_FreeNoSize(void * _pMemory);
		void f_FreeInline(void * _pMemory, umint _Size);
		void f_FreeNoSizeInline(void * _pMemory);
		umint f_SizePadded(umint _Size);

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
		void f_ForceStartCleanupThreads();

		umint f_GetNumUsedSlabs();
		umint f_GetNumFreeSlabs();

		bool f_CheckFree(EMemoryManagerCheckFlag _Flags);

		void f_DestroyThreadLocals();

#if DMibConfig_Memory_CustomThreadLocal
		void *f_GetCustomThreadLocal(umint _Index);
		void *f_SetCustomThreadLocal(umint _Index, void *_pCustom);
#endif

#if DMibConfig_Memory_Shims_Lightweight
		CReportMemoryLightweight *f_ReportMemoryTo(CReportMemoryLightweight *_pMemoryReporter);
		EMemoryReportLightweightScopeFlag f_GetLightweightScopeFlags();
		EMemoryReportLightweightScopeFlag f_SetLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags);
		EMemoryReportLightweightScopeFlag f_AddLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags);
#endif

	private:
		void *fp_AllocAlignedSlowPath(umint &_Size, umint _Alignment);
		void fp_AllocBatchSlowPath(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		void fp_FreeSlowPath(void * _pMemory, umint _Size);

		TCMemoryManagerNumaArena<t_CParams> *fp_GetAnyNumaArena();

		TCMemoryManagerArena<t_CParams> *fp_CheckoutHelper(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);
		TCMemoryManagerArena<t_CParams> *fp_CheckoutHelperSlowPath(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);

		TCMemoryManagerCheckoutLight<t_CParams> fp_Checkout(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal);

		void *fp_AllocWithCheckout(umint &_Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena);
		void *fp_AllocWithTempCheckout(umint &_Size);

		void fp_AllocBatchWithCheckout(umint _Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);
		void fp_AllocBatchWithTempCheckout(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		void fp_ProcessArenaMessages();

		void fp_EnumArenas(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArena<t_CParams> *)> const &_Functor, bool _bCleanup);
		void fp_EnumHeaps(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArenaHeap<t_CParams> *)> const &_Functor);

#if DMibConfig_Memory_Shims_Lightweight
		inline_always void fp_TrackAlloc(umint _Size);
		inline_always static bool fsp_ShouldTrackAlloc(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena);
#endif

		void f_ReturnCheckoutVirtual() override;
		void f_TemporaryReturn() override;
		void f_TemporaryGetBack() override;
		void f_TakeOwnership() override;
		void f_RelinquishOwnership() override;

	protected:
		void f_GarbageCollectLocalArena(bool _bDecommit) override;
		bool f_GarbageCollectLocalArenaIfPending() override;

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

		template <typename t_CParams2>
		friend struct TCMemoryManagerLimitedTemporaryReturn;

		static_assert(cIsPowerOfTwo<t_CParams::mc_SubSlabSize>, "Must be aligned to power of two");
		static_assert(cIsPowerOfTwo<t_CParams::mc_SlabSize>, "Must be aligned to power of two");

		static constexpr bool mc_EnableCallbacks = t_CParams::CNotifier::CGlobal::mc_EnableCallbacks;

		typename t_CParams::CAllocator m_Allocator;

		mutable align_cacheline NThread::CLowLevelLock m_NumaArenasLock;
		NIntrusive::TCAVLTree<&TCMemoryManagerNumaArena<t_CParams>::m_Link, typename TCMemoryManagerNumaArena<t_CParams>::CCompare> m_NumaArenas;

		uint32 m_nMaxArenas;
		bool m_bCanDoLazyCheckout = false;
		bool m_bThreadLocalsDestroyed = false;
		uint64 m_Magic;

		TCPool<CLocalNumaNode, 8, NThread::CLowLevelLock, NMemory::CPoolType_Freeable, typename t_CParams::CAllocator> m_LocalNumaNodePool;

		NThread::TCThreadLocalDynamic
			<
				CLocalNumaNode
				, NThread::EThreadLocalFlag_Inherit
			> m_LocalNumaNode
		;

		NThread::TCThreadLocalDynamic
			<
				TCMemoryManagerThreadLocal<t_CParams>
				, NThread::EThreadLocalFlag_AlwaysCreated | NThread::EThreadLocalFlag_FastThreadLocal | NThread::EThreadLocalFlag_Inherit
			> m_LocalArena
		;

		mutable NThread::CMutualManyRead m_HeapChunksLock;
		NContainer::TCMap<uint8 *, TCMemoryManagerArenaHeapChunk<t_CParams>, NMib::CSort_Default, TCAllocator_MemoryManager<t_CParams>> m_HeapChunks;
	};
}
