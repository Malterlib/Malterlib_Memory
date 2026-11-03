// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#if defined(DPlatformFamily_Windows)
#	define DMibPWindowsTest
#endif

#ifndef DMibPWindowsTest
#	undef DMemoryManagerTestEnable_WindowsDefault
#	undef DMemoryManagerTestEnable_WindowsLF
#endif

#ifdef DMemoryManagerTestEnable_Application
#	include <Mib/Core/Core>
#endif

#ifdef DMemoryManagerTestEnable_Malterlib
#	include <Mib/Memory/MemoryManager>
#	include <Mib/Memory/MemoryManagerDebug>
#	include <Mib/Memory/MemoryManagerTracked>
#endif

#ifdef DMemoryManagerTestEnable_TcMalloc
#	include <gperftools/tcmalloc.h>
#endif

#ifdef DMemoryManagerTestEnable_MiMalloc
#	include <mimalloc.h>
#endif

#if defined(DMemoryManagerTestEnable_WindowsDefault) || defined(DMemoryManagerTestEnable_WindowsLF)
#	include <windows.h>
#endif

#ifdef DMemoryManagerTestEnable_StdLib
#	include <stdlib.h>
#endif

#ifdef DMemoryManagerTestEnable_MacOS
#	include <malloc/malloc.h>
#endif

#include <mutex>

namespace
{
	constexpr aint gc_UseFast = true;

	struct CMalterlibMemoryDummy
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			mp_MaxSize = _MaxSize;
			return true;
		}

		void f_InitThread()
		{
			(*mp_Memory).f_SetLen(mp_MaxSize * 2);
			(*mp_pMemory) = (*mp_Memory).f_GetArray();
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return NMib::fg_AlignUp((*mp_pMemory), _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return (*mp_pMemory);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}

	private:
		NMib::NThread::TCThreadLocal<NMib::NContainer::CByteVector> mp_Memory;
		NMib::NThread::TCThreadLocal
			<
				uint8 *
				, NMib::NMemory::CAllocator_Heap
				, (NMib::NThread::EThreadLocalFlag)(NMib::NThread::EThreadLocalFlag_AlwaysCreated | NMib::NThread::EThreadLocalFlag_FastThreadLocal)
			>
			mp_pMemory
		;
		umint mp_MaxSize;
	};

#ifdef DMemoryManagerTestEnable_Malterlib
	struct CMalterlibMemoryMalterlib
	{
	private:
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

#	if DMibPPtrBits >= 64
	// Without reap-in-cleanup, cross-thread frees still use delete messages, which needs the
	// small size slabs on 32 bit
	struct CMalterlibMemoryMalterlib_SubSlabBitmaps
	{
	private:
		struct CParamsOverrides : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EMemoryManagerFreeStore mc_FreeStoreMode = NMib::NMemory::EMemoryManagerFreeStore_SubSlabBitmaps;
			static constexpr bool mc_bUseSmallSizes = false;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CParamsOverrides>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_SubSlabBitmaps()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
#	endif

	struct CMalterlibMemoryMalterlib_SubSlabLists
	{
	private:
		struct CParamsOverrides : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EMemoryManagerFreeStore mc_FreeStoreMode = NMib::NMemory::EMemoryManagerFreeStore_SubSlabLists;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CParamsOverrides>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_SubSlabLists()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	struct TCMalterlibMemoryMalterlib_Reap
	{
	private:
		struct CParamsOverrides : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EMemoryManagerFreeStore mc_FreeStoreMode = t_FreeStoreMode;
			static constexpr bool mc_bUseSmallSizes = t_FreeStoreMode != NMib::NMemory::EMemoryManagerFreeStore_SubSlabBitmaps;
			static constexpr bool mc_bReapInCleanup = true;
			static constexpr bool mc_bGlobalAddressOrder = t_bGlobalAddressOrder;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CParamsOverrides>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		TCMalterlibMemoryMalterlib_Reap()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	using CMalterlibMemoryMalterlib_SubSlabListsReap = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabLists, false>;
	using CMalterlibMemoryMalterlib_SubSlabBitmapsReap = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabBitmaps, false>;
	using CMalterlibMemoryMalterlib_SubSlabListsReapOrder = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabLists, true>;
	using CMalterlibMemoryMalterlib_SubSlabBitmapsReapOrder = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabBitmaps, true>;

#	if DMibPPtrBits >= 64
	struct CMalterlibMemoryMalterlib_LowBranch
	{
	private:
		struct CParamsOverrides : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr bool mc_bUseSmallSizes = false;
			static constexpr bool mc_bSpecialCaseSlabType0 = false;
			static constexpr bool mc_bUseFreeBlockCounting = false;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CParamsOverrides>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_LowBranch()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
#	endif

	struct CMalterlibMemoryMalterlib_Debug
	{
	private:
#	ifdef DMibNeedDebugException
		static constexpr bool mcp_bHasExceptions = true;
#	else
		static constexpr bool mcp_bHasExceptions = false;
#	endif

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManagerDebug<CParams, mcp_bHasExceptions> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_Debug()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	struct CMalterlibMemoryMalterlib_Tracked
	{
	private:
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManagerTracked<NMib::NMemory::TCMemoryManager<CParams>> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_Tracked()
			: mp_MemoryManager("Test tracked manager", NMib::NMemory::CMemoryManagerConfig())
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	struct CMalterlibMemoryMalterlib_NoCleanup
	{
	private:
		struct CLocalParams : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EDeferCleanup mc_DeferCleanup = NMib::NMemory::EDeferCleanup
				(
					NMib::NMemory::EDeferCleanup_NoCleanup | NMib::NMemory::EDeferCleanup_OneSizeBlocks | NMib::NMemory::EDeferCleanup_Commit | NMib::NMemory::EDeferCleanup_Allocs
				)
			;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CLocalParams>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_NoCleanup()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	struct CMalterlibMemoryMalterlib_NoDeferCleanup
	{
	private:
		struct CLocalParams : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EDeferCleanup mc_DeferCleanup = NMib::NMemory::EDeferCleanup_None;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CLocalParams>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_NoDeferCleanup()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	struct CMalterlibMemoryMalterlib_NoCheckout
	{
	private:
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_NoCheckout()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};

	struct CMalterlibMemoryMalterlib_NoCommit
	{
	private:
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<NMib::NMemory::CMemoryManagerParams_NoCommit>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_NoCommit()
			: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			mp_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
		{
			return mp_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_TcMalloc
	struct CMalterlibMemoryTcMalloc
	{
		CMalterlibMemoryTcMalloc()
		{
		}

		~CMalterlibMemoryTcMalloc()
		{
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			tc_free(tc_malloc(1));
			return true;
		}

		void f_InitThread()
		{
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return tc_memalign(_Alignment, _Size);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return tc_malloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			tc_free(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_MiMalloc
	struct CMalterlibMemoryMiMalloc
	{
		CMalterlibMemoryMiMalloc()
		{
		}

		~CMalterlibMemoryMiMalloc()
		{
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return true;
			return true;
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			mi_process_load();
			mi_process_init();
			mi_free(mi_malloc(1));
			return true;
		}

		void f_InitThread()
		{
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return mi_malloc_aligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mi_malloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mi_free(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_MacOS
	struct CMalterlibMemoryMacOS
	{
	private:
		malloc_zone_t *mp_pMallocZone;

	public:
		CMalterlibMemoryMacOS()
			: mp_pMallocZone(nullptr)
		{
		}

		~CMalterlibMemoryMacOS()
		{
			if (mp_pMallocZone)
			{
				malloc_destroy_zone(mp_pMallocZone);
				mp_pMallocZone = nullptr;
			}
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
#	if DPlatformVersion <= 1050
			if (_bAlignment)
				return false;
#	endif
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			mp_pMallocZone = malloc_create_zone(1024 * 1024, 0);
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(NMib::fg_GetSys()->f_MemoryManager_Checkout())
		{
			return NMib::fg_GetSys()->f_MemoryManager_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
#	if DPlatformVersion <= 1050
			return nullptr;
#	else
			_Alignment = NMib::fg_Max(sizeof(void *), _Alignment);
			return mp_pMallocZone->memalign(mp_pMallocZone, _Alignment, _Size);
#	endif
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return mp_pMallocZone->malloc(mp_pMallocZone, _Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			mp_pMallocZone->free(mp_pMallocZone, _pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}

		NMib::NStr::CStr f_GetDesc()
		{
			NMib::NStr::CStr Str = "StdLib";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_StdLib
	struct CMalterlibMemoryStdLib
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
#	ifdef DPlatformFamily_Windows
			if (_bAlignment)
				return false;
#	endif
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(NMib::fg_GetSys()->f_MemoryManager_Checkout())
		{
			return NMib::fg_GetSys()->f_MemoryManager_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
#	ifdef DPlatformFamily_Windows
			return nullptr;
#	else
			void *pPtr = nullptr;
			posix_memalign(&pPtr, NMib::fg_Max(_Alignment, sizeof(void *)), _Size);

			return pPtr;
#	endif
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return malloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			free(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}

		NMib::NStr::CStr f_GetDesc()
		{
			NMib::NStr::CStr Str = "StdLib";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_Application
	struct CMalterlibMemoryApplication
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return true;
		}

		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(NMib::fg_GetSys()->f_MemoryManager_Checkout())
		{
			return NMib::fg_GetSys()->f_MemoryManager_Checkout();
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return NMib::NMemory::fg_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			return NMib::NMemory::fg_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}

		NMib::NStr::CStr f_GetDesc()
		{
			NMib::NStr::CStr Str = "Application";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsDefault
	template <bool t_bClear, bool t_bMultiThreaded>
	struct TCMalterlibMemoryWindows
	{
		TCMalterlibMemoryWindows()
			: m_Heap(nullptr)
		{
		}

		~TCMalterlibMemoryWindows()
		{
			if (m_Heap)
				HeapDestroy(m_Heap);
		}

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		HANDLE m_Heap;
		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			if (_nThreads > 1 && !t_bMultiThreaded)
				return false;
			m_Heap = HeapCreate(t_bMultiThreaded ? 0 : HEAP_NO_SERIALIZE, 0, 0);
			return true;
		}

		void f_InitThread()
		{
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return HeapAlloc(m_Heap, 0, _Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			HeapFree(m_Heap, 0, _pMem);
		}

		void f_Clear()
		{
			if (t_bClear)
			{
				if (m_Heap)
					HeapDestroy(m_Heap);
				m_Heap = HeapCreate(t_bMultiThreaded ? 0 : HEAP_NO_SERIALIZE, 0, 0);
			}
		}

		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsLF
	template <bool t_bClear>
	struct TCMalterlibMemoryWindowsLF
	{
		TCMalterlibMemoryWindowsLF()
			: m_Heap(nullptr)
		{
		}

		~TCMalterlibMemoryWindowsLF()
		{
			if (m_Heap)
				HeapDestroy(m_Heap);
		}

		HANDLE m_Heap;

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			f_Create();
			return true;
		}

		void f_InitThread()
		{
		}

		int f_Checkout()
		{
			return 0;
		}

		void f_Create()
		{
			m_Heap = HeapCreate(0, 0, 0);
			ULONG Enable = 3;
			HeapSetInformation(m_Heap, HeapCompatibilityInformation, &Enable, sizeof(Enable));
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return HeapAlloc(m_Heap, 0, _Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			HeapFree(m_Heap, 0, _pMem);
		}

		void f_Clear()
		{
			if (t_bClear)
			{
				if (m_Heap)
					HeapDestroy(m_Heap);
				f_Create();
			}
		}

		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_Virtual
	struct CMalterlibMemoryVirtual
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(umint _nThreads, umint _MaxSize)
		{
			return _nThreads <= 2;
		}

		void f_InitThread()
		{
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(umint _Size, umint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(umint _Size)
		{
			return NMib::NMemory::CAllocator_Virtual::f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			NMib::NMemory::CAllocator_Virtual::f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
#endif
}
