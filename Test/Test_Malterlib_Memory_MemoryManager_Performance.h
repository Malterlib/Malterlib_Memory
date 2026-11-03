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
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();

	private:
		NMib::NThread::TCThreadLocal<NMib::NContainer::CByteVector> mp_Memory;
		NMib::NThread::TCThreadLocal
			<
				uint8 *
				, NMib::NMemory::CAllocator_Heap
				, (NMib::NThread::EThreadLocalFlag)(NMib::NThread::EThreadLocalFlag_AlwaysCreated | NMib::NThread::EThreadLocalFlag_FastThreadLocal)
			>
			mp_MemoryStart
		;
		umint mp_MaxSize;
	};

#ifdef DMemoryManagerTestEnable_Malterlib
	struct CMalterlibMemoryMalterlib
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

#	if DMibPPtrBits >= 64
	// Without reap-in-cleanup, cross-thread frees still use delete messages, which needs the
	// small size slabs on 32 bit
	struct CMalterlibMemoryMalterlib_SubSlabBitmaps
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
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
		CMalterlibMemoryMalterlib_SubSlabBitmaps();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#	endif

	struct CMalterlibMemoryMalterlib_SubSlabLists
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
		struct CParamsOverrides : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EMemoryManagerFreeStore mc_FreeStoreMode = NMib::NMemory::EMemoryManagerFreeStore_SubSlabLists;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CParamsOverrides>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_SubSlabLists();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	struct TCMalterlibMemoryMalterlib_Reap
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
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
		TCMalterlibMemoryMalterlib_Reap();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

	using CMalterlibMemoryMalterlib_SubSlabListsReap = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabLists, false>;
	using CMalterlibMemoryMalterlib_SubSlabBitmapsReap = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabBitmaps, false>;
	using CMalterlibMemoryMalterlib_SubSlabListsReapOrder = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabLists, true>;
	using CMalterlibMemoryMalterlib_SubSlabBitmapsReapOrder = TCMalterlibMemoryMalterlib_Reap<NMib::NMemory::EMemoryManagerFreeStore_SubSlabBitmaps, true>;

#	if DMibPPtrBits >= 64
	struct CMalterlibMemoryMalterlib_LowBranch
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
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
		CMalterlibMemoryMalterlib_LowBranch();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#	endif

	struct CMalterlibMemoryMalterlib_Debug
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
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
		CMalterlibMemoryMalterlib_Debug();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

	struct CMalterlibMemoryMalterlib_Tracked
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManagerTracked<NMib::NMemory::TCMemoryManager<CParams>> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_Tracked();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

	struct CMalterlibMemoryMalterlib_NoCleanup
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
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
		CMalterlibMemoryMalterlib_NoCleanup();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

	struct CMalterlibMemoryMalterlib_NoDeferCleanup
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
		struct CLocalParams : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EDeferCleanup mc_DeferCleanup = NMib::NMemory::EDeferCleanup_None;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CLocalParams>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_NoDeferCleanup();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};

	struct CMalterlibMemoryMalterlib_NoCheckout
	{
		CMalterlibMemoryMalterlib_NoCheckout();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();

	private:
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;
	};

	struct CMalterlibMemoryMalterlib_NoCommit
	{
	private:
		// Declared before the public section, whose f_Checkout return type uses mp_MemoryManager.
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<NMib::NMemory::CMemoryManagerParams_NoCommit>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> mp_MemoryManager;

	public:
		CMalterlibMemoryMalterlib_NoCommit();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(mp_MemoryManager.f_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#endif

#ifdef DMemoryManagerTestEnable_TcMalloc
	struct CMalterlibMemoryTcMalloc
	{
		CMalterlibMemoryTcMalloc();
		~CMalterlibMemoryTcMalloc();
		void f_SetNumaNode(NMib::ENumaNode _Node);
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#endif

#ifdef DMemoryManagerTestEnable_MiMalloc
	struct CMalterlibMemoryMiMalloc
	{
		CMalterlibMemoryMiMalloc();
		~CMalterlibMemoryMiMalloc();
		void f_SetNumaNode(NMib::ENumaNode _Node);
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#endif

#ifdef DMemoryManagerTestEnable_MacOS
	struct CMalterlibMemoryMacOS
	{
		CMalterlibMemoryMacOS();
		~CMalterlibMemoryMacOS();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(NMib::fg_GetSys()->f_MemoryManager_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
		NMib::NStr::CStr f_GetDesc();

	private:
		malloc_zone_t *mp_pMallocZone;
	};
#endif

#ifdef DMemoryManagerTestEnable_StdLib
	struct CMalterlibMemoryStdLib
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(NMib::fg_GetSys()->f_MemoryManager_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
		NMib::NStr::CStr f_GetDesc();
	};
#endif

#ifdef DMemoryManagerTestEnable_Application
	struct CMalterlibMemoryApplication
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		auto f_Checkout() -> decltype(NMib::fg_GetSys()->f_MemoryManager_Checkout());
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
		NMib::NStr::CStr f_GetDesc();
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsDefault
	template <bool t_bClear, bool t_bMultiThreaded>
	struct TCMalterlibMemoryWindows
	{
		TCMalterlibMemoryWindows();
		~TCMalterlibMemoryWindows();
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);

		HANDLE m_pHeap;
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsLF
	template <bool t_bClear>
	struct TCMalterlibMemoryWindowsLF
	{
		TCMalterlibMemoryWindowsLF();
		~TCMalterlibMemoryWindowsLF();

		HANDLE m_pHeap;

		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		void f_Create();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#endif

#ifdef DMemoryManagerTestEnable_Virtual
	struct CMalterlibMemoryVirtual
	{
		static bool fs_ShouldRun(umint _nThreads, bool _bAlignment);
		void f_SetNumaNode(NMib::ENumaNode _Node);
		bool f_Init(umint _nThreads, umint _MaxSize);
		void f_InitThread();
		int f_Checkout();
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment);
		inline_small void *f_Alloc(umint _Size);
		inline_small void f_FreeNoSize(void *_pMem);
		void f_Clear();
		void f_CheckHeap();
	};
#endif
}

#include "Test_Malterlib_Memory_MemoryManager_Performance.hpp"
