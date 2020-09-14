// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if defined(DPlatformFamily_Windows)
	#define DMibPWindowsTest
#endif

#ifndef DMibPWindowsTest
#	undef DMemoryManagerTestEnable_WindowsDefault
#	undef DMemoryManagerTestEnable_WindowsLF
#endif

#ifdef DMemoryManagerTestEnable_Application
#include <Mib/Core/Core>
#endif

#ifdef DMemoryManagerTestEnable_Malterlib
#include <Mib/Memory/MemoryManager>
#include <Mib/Memory/MemoryManagerDebug>
#include <Mib/Memory/MemoryManagerTracked>
#endif

#ifdef DMemoryManagerTestEnable_TcMalloc
#include <gperftools/tcmalloc.h>
#endif

#ifdef DMemoryManagerTestEnable_MiMalloc
#include <mimalloc.h>
#endif

#if defined(DMemoryManagerTestEnable_WindowsDefault) || defined(DMemoryManagerTestEnable_WindowsLF)
#include <windows.h>
#endif

#ifdef DMemoryManagerTestEnable_StdLib
#include <stdlib.h>
#endif

#ifdef DMemoryManagerTestEnable_OSX
#include <malloc/malloc.h>
#endif

#include <mutex>

namespace
{

	enum
	{
		EUseFast = true
	};

	class CMalterlibMemoryDummy
	{
		NMib::NThread::TCThreadLocal<NMib::NContainer::CByteVector> m_Memory;
		NMib::NThread::TCThreadLocal
			<
				uint8 *
				, NMib::NMemory::CAllocator_Heap
				, (NMib::NThread::EThreadLocalFlag)(NMib::NThread::EThreadLocalFlag_AlwaysCreated | NMib::NThread::EThreadLocalFlag_FastThreadLocal)
			>
			m_pMemory
		;
		mint m_MaxSize;
	public:
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			m_MaxSize = _MaxSize;
			return true;
		}

		void f_InitThread()
		{
			(*m_Memory).f_SetLen(m_MaxSize * 2);
			(*m_pMemory) = (*m_Memory).f_GetArray();
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{

			return NMib::fg_AlignUp((*m_pMemory), _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return (*m_pMemory);
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
	};

#ifdef DMemoryManagerTestEnable_Malterlib
	class CMalterlibMemoryMalterlib
	{
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}
		void f_CheckHeap()
		{
		}
	};

#if DMibPPtrBits >= 64
	class CMalterlibMemoryMalterlib_LowBranch
	{
		struct CParamsOverrides : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr bool mc_bUseSmallSizes = false;
			static constexpr bool mc_bSpecialCaseSlabType0 = false;
			static constexpr bool mc_bUseFreeBlockCounting = false;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CParamsOverrides>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib_LowBranch()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}
		void f_CheckHeap()
		{
		}
	};
#endif

	class CMalterlibMemoryMalterlib_Debug
	{
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};
		NMib::NMemory::TCMemoryManagerDebug<CParams, false> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib_Debug()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};


	class CMalterlibMemoryMalterlib_Tracked
	{
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};
		NMib::NMemory::TCMemoryManagerTracked<NMib::NMemory::TCMemoryManager<CParams>> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib_Tracked()
			: m_MemoryManager("Test tracked manager", NMib::NMemory::CMemoryManagerConfig())
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};

	class CMalterlibMemoryMalterlib_NoCleanup
	{
		struct CLocalParams : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EDeferCleanup mc_DeferCleanup
				= NMib::NMemory::EDeferCleanup
				(
					NMib::NMemory::EDeferCleanup_NoCleanup
					| NMib::NMemory::EDeferCleanup_OneSizeBlocks
					| NMib::NMemory::EDeferCleanup_Commit
					| NMib::NMemory::EDeferCleanup_Allocs
				)
			;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CLocalParams>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib_NoCleanup()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryMalterlib_NoDeferCleanup
	{
		struct CLocalParams : public NMib::NMemory::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMemory::EDeferCleanup mc_DeferCleanup = NMib::NMemory::EDeferCleanup_None;
		};

		struct CParams : public NMib::NMemory::TCMemoryManagerParams<CLocalParams>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> m_MemoryManager;
	public:

		CMalterlibMemoryMalterlib_NoDeferCleanup()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryMalterlib_NoCheckout
	{
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<>
		{
		};

		NMib::NMemory::TCMemoryManager<CParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib_NoCheckout()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
		}

		void f_Clear()
		{
		}

		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryMalterlib_NoCommit
	{
		struct CParams : public NMib::NMemory::TCMemoryManagerParams<NMib::NMemory::CMemoryManagerParams_NoCommit>
		{
		};
		
		NMib::NMemory::TCMemoryManager<CParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlib_NoCommit()
			: m_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
		{
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}

		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_MemoryManager.f_FreeNoSize(_pMem);
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
	class CMalterlibMemoryTcMalloc
	{
	public:
		CMalterlibMemoryTcMalloc()
		{
		}
		~CMalterlibMemoryTcMalloc()
		{
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return tc_memalign(_Alignment, _Size);;
		}

		inline_small void *f_Alloc(mint _Size)
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
	class CMalterlibMemoryMiMalloc
	{
	public:
		CMalterlibMemoryMiMalloc()
		{
		}
		~CMalterlibMemoryMiMalloc()
		{
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return true;
			return true;
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return mi_malloc_aligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
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

#ifdef DMemoryManagerTestEnable_OSX
	class CMalterlibMemoryOSX
	{
		malloc_zone_t *m_pMallocZone;
	public:
		CMalterlibMemoryOSX()
			: m_pMallocZone(nullptr)
		{
		}

		~CMalterlibMemoryOSX()
		{
			if (m_pMallocZone)
			{
				malloc_destroy_zone(m_pMallocZone);
				m_pMallocZone = nullptr;
			}
		}

		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
#ifdef DPlatform_OSX10_5
			if (_bAlignment)
				return false;
#endif
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
		{
			m_pMallocZone = malloc_create_zone(1024*1024, 0);
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
#ifndef DPlatform_OSX10_5
			_Alignment = NMib::fg_Max(sizeof(void *), _Alignment);
			return m_pMallocZone->memalign(m_pMallocZone, _Alignment, _Size);
#else
			return nullptr;
#endif
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_pMallocZone->malloc(m_pMallocZone, _Size);
		}

		inline_small void f_FreeNoSize(void *_pMem)
		{
			m_pMallocZone->free(m_pMallocZone, _pMem);
		}

		void f_Clear()
		{
		}
		void f_CheckHeap()
		{
		}
		NMib::NStr::CStr GetDesc()
		{
			NMib::NStr::CStr Str = "StdLib";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_StdLib
	class CMalterlibMemoryStdLib
	{
	public:
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
#ifdef DPlatformFamily_Windows
			if (_bAlignment)
				return false;
#endif
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
#ifdef DPlatformFamily_Windows
			return nullptr;
#else
			void *pPtr;
			posix_memalign(&pPtr, _Alignment, _Size);
			return pPtr;
#endif
		}

		inline_small void *f_Alloc(mint _Size)
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
		NMib::NStr::CStr GetDesc()
		{
			NMib::NStr::CStr Str = "StdLib";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_Application
	class CMalterlibMemoryApplication
	{
	public:
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
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
		NMib::NStr::CStr GetDesc()
		{
			NMib::NStr::CStr Str = "Application";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsDefault
	template <bool t_bClear, bool t_bMultiThreaded>
	class TCMalterlibMemoryWindows
	{
	public:
		TCMalterlibMemoryWindows()
			: m_Heap(nullptr)
		{
		}
		~TCMalterlibMemoryWindows()
		{
			if (m_Heap)
				HeapDestroy(m_Heap);
		}
		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return false;
			return _nThreads <= 3;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		HANDLE m_Heap;
		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
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
	class TCMalterlibMemoryWindowsLF
	{
	public:
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

		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
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
	class CMalterlibMemoryVirtual
	{
	public:

		static bool fs_ShouldRun(mint _nThreads, bool _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bool f_Init(mint _nThreads, mint _MaxSize)
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

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
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
