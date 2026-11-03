// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace
{
	inline void *CMalterlibMemoryDummy::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return NMib::fg_AlignUp((*mp_MemoryStart), _Alignment);
	}

	inline void *CMalterlibMemoryDummy::f_Alloc(umint _Size)
	{
		return (*mp_MemoryStart);
	}

	inline void CMalterlibMemoryDummy::f_FreeNoSize(void *_pMem)
	{
	}

#ifdef DMemoryManagerTestEnable_Malterlib
	inline void *CMalterlibMemoryMalterlib::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

#	if DMibPPtrBits >= 64
	inline void *CMalterlibMemoryMalterlib_SubSlabBitmaps::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_SubSlabBitmaps::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_SubSlabBitmaps::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}
#	endif

	inline void *CMalterlibMemoryMalterlib_SubSlabLists::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_SubSlabLists::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_SubSlabLists::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::TCMalterlibMemoryMalterlib_Reap()
		: mp_MemoryManager{NMib::NMemory::CMemoryManagerConfig()}
	{
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline bool TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::fs_ShouldRun(umint _nThreads, bool _bAlignment)
	{
		return true;
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_SetNumaNode(NMib::ENumaNode _Node)
	{
		mp_MemoryManager.f_SetNumaNode(_Node);
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline bool TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_Init(umint _nThreads, umint _MaxSize)
	{
		return true;
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_InitThread()
	{
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline auto TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_Checkout() -> decltype(mp_MemoryManager.f_Checkout())
	{
		return mp_MemoryManager.f_Checkout();
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void *TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void *TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_Clear()
	{
	}

	template <NMib::NMemory::EMemoryManagerFreeStore t_FreeStoreMode, bool t_bGlobalAddressOrder>
	inline void TCMalterlibMemoryMalterlib_Reap<t_FreeStoreMode, t_bGlobalAddressOrder>::f_CheckHeap()
	{
	}

#	if DMibPPtrBits >= 64
	inline void *CMalterlibMemoryMalterlib_LowBranch::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_LowBranch::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_LowBranch::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}
#	endif

	inline void *CMalterlibMemoryMalterlib_Debug::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_Debug::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_Debug::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	inline void *CMalterlibMemoryMalterlib_Tracked::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_Tracked::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_Tracked::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	inline void *CMalterlibMemoryMalterlib_NoCleanup::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_NoCleanup::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_NoCleanup::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	inline void *CMalterlibMemoryMalterlib_NoDeferCleanup::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_NoDeferCleanup::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_NoDeferCleanup::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	inline void *CMalterlibMemoryMalterlib_NoCheckout::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_NoCheckout::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_NoCheckout::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}

	inline void *CMalterlibMemoryMalterlib_NoCommit::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mp_MemoryManager.f_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMalterlib_NoCommit::f_Alloc(umint _Size)
	{
		return mp_MemoryManager.f_Alloc(_Size);
	}

	inline void CMalterlibMemoryMalterlib_NoCommit::f_FreeNoSize(void *_pMem)
	{
		mp_MemoryManager.f_FreeNoSize(_pMem);
	}
#endif

#ifdef DMemoryManagerTestEnable_TcMalloc
	inline void *CMalterlibMemoryTcMalloc::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return tc_memalign(_Alignment, _Size);
	}

	inline void *CMalterlibMemoryTcMalloc::f_Alloc(umint _Size)
	{
		return tc_malloc(_Size);
	}

	inline void CMalterlibMemoryTcMalloc::f_FreeNoSize(void *_pMem)
	{
		tc_free(_pMem);
	}
#endif

#ifdef DMemoryManagerTestEnable_MiMalloc
	inline void *CMalterlibMemoryMiMalloc::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return mi_malloc_aligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryMiMalloc::f_Alloc(umint _Size)
	{
		return mi_malloc(_Size);
	}

	inline void CMalterlibMemoryMiMalloc::f_FreeNoSize(void *_pMem)
	{
		mi_free(_pMem);
	}
#endif

#ifdef DMemoryManagerTestEnable_MacOS
	inline void *CMalterlibMemoryMacOS::f_AllocAligned(umint _Size, umint _Alignment)
	{
#	if DPlatformVersion <= 1050
		return nullptr;
#	else
		_Alignment = NMib::fg_Max(sizeof(void *), _Alignment);
		return mp_pMallocZone->memalign(mp_pMallocZone, _Alignment, _Size);
#	endif
	}

	inline void *CMalterlibMemoryMacOS::f_Alloc(umint _Size)
	{
		return mp_pMallocZone->malloc(mp_pMallocZone, _Size);
	}

	inline void CMalterlibMemoryMacOS::f_FreeNoSize(void *_pMem)
	{
		mp_pMallocZone->free(mp_pMallocZone, _pMem);
	}
#endif

#ifdef DMemoryManagerTestEnable_StdLib
	inline void *CMalterlibMemoryStdLib::f_AllocAligned(umint _Size, umint _Alignment)
	{
#	ifdef DPlatformFamily_Windows
		return nullptr;
#	else
		void *pPtr = nullptr;
		posix_memalign(&pPtr, NMib::fg_Max(_Alignment, sizeof(void *)), _Size);

		return pPtr;
#	endif
	}

	inline void *CMalterlibMemoryStdLib::f_Alloc(umint _Size)
	{
		return malloc(_Size);
	}

	inline void CMalterlibMemoryStdLib::f_FreeNoSize(void *_pMem)
	{
		free(_pMem);
	}
#endif

#ifdef DMemoryManagerTestEnable_Application
	inline void *CMalterlibMemoryApplication::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return NMib::NMemory::fg_AllocAligned(_Size, _Alignment);
	}

	inline void *CMalterlibMemoryApplication::f_Alloc(umint _Size)
	{
		return NMib::NMemory::fg_Alloc(_Size);
	}

	inline void CMalterlibMemoryApplication::f_FreeNoSize(void *_pMem)
	{
		return NMib::NMemory::fg_FreeNoSize(_pMem);
	}
#endif

#ifdef DMemoryManagerTestEnable_WindowsDefault
	template <bool t_bClear, bool t_bMultiThreaded>
	inline TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::TCMalterlibMemoryWindows()
		: m_pHeap(nullptr)
	{
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::~TCMalterlibMemoryWindows()
	{
		if (m_pHeap)
			HeapDestroy(m_pHeap);
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline bool TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::fs_ShouldRun(umint _nThreads, bool _bAlignment)
	{
		if (_bAlignment)
			return false;
		return true;
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_SetNumaNode(NMib::ENumaNode _Node)
	{
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline bool TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_Init(umint _nThreads, umint _MaxSize)
	{
		if (_nThreads > 1 && !t_bMultiThreaded)
			return false;
		m_pHeap = HeapCreate(t_bMultiThreaded ? 0 : HEAP_NO_SERIALIZE, 0, 0);
		return true;
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_InitThread()
	{
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline int TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_Checkout()
	{
		return 0;
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void *TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return nullptr;
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void *TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_Alloc(umint _Size)
	{
		return HeapAlloc(m_pHeap, 0, _Size);
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_FreeNoSize(void *_pMem)
	{
		HeapFree(m_pHeap, 0, _pMem);
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_Clear()
	{
		if (t_bClear)
		{
			if (m_pHeap)
				HeapDestroy(m_pHeap);
			m_pHeap = HeapCreate(t_bMultiThreaded ? 0 : HEAP_NO_SERIALIZE, 0, 0);
		}
	}

	template <bool t_bClear, bool t_bMultiThreaded>
	inline void TCMalterlibMemoryWindows<t_bClear, t_bMultiThreaded>::f_CheckHeap()
	{
	}
#endif

#ifdef DMemoryManagerTestEnable_WindowsLF
	template <bool t_bClear>
	inline TCMalterlibMemoryWindowsLF<t_bClear>::TCMalterlibMemoryWindowsLF()
		: m_pHeap(nullptr)
	{
	}

	template <bool t_bClear>
	inline TCMalterlibMemoryWindowsLF<t_bClear>::~TCMalterlibMemoryWindowsLF()
	{
		if (m_pHeap)
			HeapDestroy(m_pHeap);
	}

	template <bool t_bClear>
	inline bool TCMalterlibMemoryWindowsLF<t_bClear>::fs_ShouldRun(umint _nThreads, bool _bAlignment)
	{
		if (_bAlignment)
			return false;
		return true;
	}

	template <bool t_bClear>
	inline void TCMalterlibMemoryWindowsLF<t_bClear>::f_SetNumaNode(NMib::ENumaNode _Node)
	{
	}

	template <bool t_bClear>
	inline bool TCMalterlibMemoryWindowsLF<t_bClear>::f_Init(umint _nThreads, umint _MaxSize)
	{
		f_Create();
		return true;
	}

	template <bool t_bClear>
	inline void TCMalterlibMemoryWindowsLF<t_bClear>::f_InitThread()
	{
	}

	template <bool t_bClear>
	inline int TCMalterlibMemoryWindowsLF<t_bClear>::f_Checkout()
	{
		return 0;
	}

	template <bool t_bClear>
	inline void TCMalterlibMemoryWindowsLF<t_bClear>::f_Create()
	{
		m_pHeap = HeapCreate(0, 0, 0);
		ULONG Enable = 3;
		HeapSetInformation(m_pHeap, HeapCompatibilityInformation, &Enable, sizeof(Enable));
	}

	template <bool t_bClear>
	inline void *TCMalterlibMemoryWindowsLF<t_bClear>::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return nullptr;
	}

	template <bool t_bClear>
	inline void *TCMalterlibMemoryWindowsLF<t_bClear>::f_Alloc(umint _Size)
	{
		return HeapAlloc(m_pHeap, 0, _Size);
	}

	template <bool t_bClear>
	inline void TCMalterlibMemoryWindowsLF<t_bClear>::f_FreeNoSize(void *_pMem)
	{
		HeapFree(m_pHeap, 0, _pMem);
	}

	template <bool t_bClear>
	inline void TCMalterlibMemoryWindowsLF<t_bClear>::f_Clear()
	{
		if (t_bClear)
		{
			if (m_pHeap)
				HeapDestroy(m_pHeap);
			f_Create();
		}
	}

	template <bool t_bClear>
	inline void TCMalterlibMemoryWindowsLF<t_bClear>::f_CheckHeap()
	{
	}
#endif

#ifdef DMemoryManagerTestEnable_Virtual
	inline void *CMalterlibMemoryVirtual::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return nullptr;
	}

	inline void *CMalterlibMemoryVirtual::f_Alloc(umint _Size)
	{
		return NMib::NMemory::CAllocator_Virtual::f_Alloc(_Size);
	}

	inline void CMalterlibMemoryVirtual::f_FreeNoSize(void *_pMem)
	{
		NMib::NMemory::CAllocator_Virtual::f_FreeNoSize(_pMem);
	}
#endif
}
