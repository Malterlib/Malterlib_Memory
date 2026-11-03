// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <CDebugMemoryManagerUnderlying t_Options>
	TCDebugMemoryManager<t_Options>::TCDebugMemoryManager()
		: mp_Heap(CMemoryManagerConfig())
		, mp_Sequence(0)
		, mp_nFreeBlocks(0)
		, mp_bCheckUpper(0)
		, mp_nFreeMemBytes(0)
		, mp_bProtectOnDemand(false)
		, mp_bProtectionDemanded(false)
	{
		if constexpr (t_Options & EDebugMemoryManager_CheckUpper)
			mp_bCheckUpper = true;
		else if constexpr (t_Options & EDebugMemoryManager_CheckRandom)
		{
			NMib::NMisc::CAutoRandom Random;
			if (Random.f_GetValue<uint32>() & 1)
				mp_bCheckUpper = true;
			else
				mp_bCheckUpper = false;
		}

		auto CheckUpperStr = NSys::fg_Process_GetEnvironmentVariable_NonProtected(NStr::CFStr256("MalterlibMemoryOverwriteCheckUpper"));
		if (CheckUpperStr == "true")
			mp_bCheckUpper = true;
		else if (CheckUpperStr == "false")
			mp_bCheckUpper = false;

		// Use max 20 % of physical memory
		//mp_nMaxFreeMemory = NMib::NSys::fg_Process_GetPhysicalMemory() / 5;

#if DMibPPtrBits == 32
		mp_nMaxFreeMemory = umint(128) * uint64(1024) * uint64(1024);
#else
		mp_nMaxFreeMemory = uint64(4) * uint64(1024) * uint64(1024) * uint64(1024);
#endif
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	TCDebugMemoryManager<t_Options>::~TCDebugMemoryManager()
	{
		// Memory leaks
		//if (!mp_Blocks.f_IsEmpty())
		//	DMibPDebugBreak;

		DMibLock(mp_Lock);

		while (!mp_FreeBlocks.f_IsEmpty())
			fp_RemoveFreeBlock();

		mp_FreeBlocks.f_Clear();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_DestroyCleanupThreads()
	{
		mp_Heap.f_DestroyCleanupThreads();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	CReportMemoryLightweight *TCDebugMemoryManager<t_Options>::f_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
	{
		return (CReportMemoryLightweight *)mp_Heap.f_SetCustomThreadLocal(0, _pMemoryReporter);
	}

#if DMibConfig_Memory_Shims_Lightweight
	template <CDebugMemoryManagerUnderlying t_Options>
	EMemoryReportLightweightScopeFlag TCDebugMemoryManager<t_Options>::f_GetLightweightScopeFlags()
	{
		return (EMemoryReportLightweightScopeFlag)(umint)mp_Heap.f_GetCustomThreadLocal(1);
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	EMemoryReportLightweightScopeFlag TCDebugMemoryManager<t_Options>::f_SetLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		return (EMemoryReportLightweightScopeFlag)(umint)mp_Heap.f_SetCustomThreadLocal(1, (void *)(umint)_Flags);
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	EMemoryReportLightweightScopeFlag TCDebugMemoryManager<t_Options>::f_AddLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		return (EMemoryReportLightweightScopeFlag)(umint)mp_Heap.f_SetCustomThreadLocal(1, (void *)(umint)(f_GetLightweightScopeFlags() | _Flags));
	}
#endif

	template <CDebugMemoryManagerUnderlying t_Options>
	NThread::CMutual &TCDebugMemoryManager<t_Options>::f_GetLock()
	{
		return mp_Lock;
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void *TCDebugMemoryManager<t_Options>::f_AllocWithSize(umint &_Size, umint _Alignment)
	{
		_Alignment = fg_Max(umint(1 << mcp_MemoryManagerAlignment), _Alignment);
		umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();
		if (_Size == 0)
			_Size = 1;

		_Size = NPrivate::fg_AlignAllocationSizeOrThrow(_Size, _Alignment);
		umint UserNeededSize = NPrivate::fg_AlignAllocationSizeOrThrow(_Size, GranularityProtect);
		umint GuardSize = NPrivate::fg_AddAllocationSizeOrThrow(GranularityProtect, NPrivate::fg_AddAllocationSizeOrThrow(GranularityProtect, GranularityProtect));
		umint NeededSize = NPrivate::fg_AddAllocationSizeOrThrow(UserNeededSize, GuardSize);

		uint8 *pBlock = (uint8 *)mp_Heap.f_AllocAlignedWithSize(NeededSize, _Alignment);

		DMibLock(mp_Lock);

		CReportMemoryLightweight *pReporter = (CReportMemoryLightweight *)mp_Heap.f_GetCustomThreadLocal(0);
		if (pReporter)
			pReporter->f_Alloc(_Size);

		umint OriginalSize = _Size;

		uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
		uint8 *pRetBlock = pBlockStart + GranularityProtect;

		if (!mp_bProtectOnDemand || mp_bProtectionDemanded)
		{
			NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect, EProtect_Exec);
			//NSys::fg_Mem_VirtualProtect(pRetBlock, UserNeededSize, 0);
			NSys::fg_Mem_VirtualProtect(pRetBlock + UserNeededSize, GranularityProtect, EProtect_Exec);
		}

		if (mp_bCheckUpper)
			pRetBlock += UserNeededSize - _Size;

		CMemoryBlock &NewBlock = mp_Blocks[pRetBlock];
		NewBlock.m_pStart = pBlock;
		NewBlock.m_RealSize = NeededSize;
		NewBlock.m_AllocSize = _Size;
		NewBlock.m_OriginalSize = OriginalSize;

		return pRetBlock;
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_Free(void *_pBlock, umint _Size)
	{
		if (!_pBlock)
			return;

		if (!_Size)
			DMibPDebugBreak;

		_Size = NPrivate::fg_AlignAllocationSizeOrThrow(_Size, umint(1 << mcp_MemoryManagerAlignment));
		return fp_Free(_pBlock, _Size);
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_FreeNoSize(void *_pBlock)
	{
		fp_Free(_pBlock, 0);
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	umint TCDebugMemoryManager<t_Options>::f_Size(void const *_pBlock) const
	{
		if (!_pBlock)
			return 0;
		DMibLock(mp_Lock);
		uint8 *pBlock = (uint8 *)_pBlock;
		CMemoryBlock const *pMemBlock = mp_Blocks.f_FindEqual(pBlock);
		if (!pMemBlock)
			DMibPDebugBreak;

		return pMemBlock->m_AllocSize;
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	umint TCDebugMemoryManager<t_Options>::f_TrySize(void const *_pBlock) const
	{
		if (!_pBlock)
			return 0;
		DMibLock(mp_Lock);
		uint8 *pBlock = (uint8 *)_pBlock;
		CMemoryBlock const *pMemBlock = mp_Blocks.f_FindEqual(pBlock);
		if (pMemBlock)
			return pMemBlock->m_AllocSize;
		return 0;
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	umint TCDebugMemoryManager<t_Options>::f_Overhead(void const *_pBlock) const
	{
		if (!_pBlock)
			return 0;
		DMibLock(mp_Lock);
		umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();
		uint8 *pBlock = (uint8 *)_pBlock;
		CMemoryBlock const *pMemBlock = mp_Blocks.f_FindEqual(pBlock);
		if (!pMemBlock)
			DMibPDebugBreak;

		umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

		return UserNeededSize - pMemBlock->m_AllocSize + GranularityProtect * 2;
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	bool TCDebugMemoryManager<t_Options>::f_ContainsBlock(void *_pPtr)
	{
		DMibLock(mp_Lock);
		return mp_Heap.f_ContainsBlock(_pPtr);
		//mp_Blocks.f_FindEqual((uint8 const*)_pPtr) != nullptr;
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_DemandProtection()
	{
		DMibLock(mp_Lock);

		if (mp_bProtectOnDemand && !mp_bProtectionDemanded)
		{
			umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

			for (auto BIter = mp_Blocks.f_GetIterator(); BIter; ++BIter)
			{
				CMemoryBlock *pMemBlock = BIter;

				uint8 *pBlock = pMemBlock->m_pStart;
				uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
				uint8 *pRetBlock = pBlockStart + GranularityProtect;

				umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

				NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect, EProtect_Exec);
				NSys::fg_Mem_VirtualProtect(pRetBlock + UserNeededSize, GranularityProtect, EProtect_Exec);
			}

			for (auto FIter = mp_FreeBlocks.f_GetIterator(); FIter; ++FIter)
			{
				CMemoryBlock *pMemBlock = FIter;

				umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

				uint8 *pBlock = pMemBlock->m_pStart;
				uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);

				NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect * 2 + UserNeededSize, EProtect_Exec);
			}

			mp_bProtectionDemanded = true;
		}
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_Lock()
	{
		mp_Lock.f_Lock();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_Unlock()
	{
		mp_Lock.f_Unlock();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_PrepareFork()
	{
		mp_Lock.f_PrepareFork();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_ForkedChild()
	{
		mp_Lock.f_ForkedChild();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::f_ForkedParent()
	{
		mp_Lock.f_ForkedParent();
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	uint8 *TCDebugMemoryManager<t_Options>::CMemoryBlock::f_GetUserAddress() const
	{
		return NContainer::TCMapWithPool<uint8 *, CMemoryBlock>::fs_GetKey(this);
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::fp_RemoveFreeBlock()
	{
		umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

		--mp_nFreeBlocks;
		CMemoryBlock *pMemBlock = mp_FreeBlocks.f_FindSmallest();
		umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);
		mp_nFreeMemBytes -= pMemBlock->m_RealSize;

		uint8 *pBlock = pMemBlock->m_pStart;
		uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
		if (!mp_bProtectOnDemand || mp_bProtectionDemanded)
			NSys::fg_Mem_VirtualProtect(pBlockStart, GranularityProtect * 2 + UserNeededSize, EProtect_ReadWrite);

		umint Size = pMemBlock->m_RealSize;

		mp_FreeBlocks.f_Remove(pMemBlock);

		{
			DMibUnlock(mp_Lock);
			mp_Heap.f_Free(pBlock, Size);
		}
	}

	template <CDebugMemoryManagerUnderlying t_Options>
	void TCDebugMemoryManager<t_Options>::fp_Free(void *_pBlock, umint _Size)
	{
		if (!_pBlock)
			return;
		auto &Lock = mp_Lock;
		DMibLock(Lock);
		uint8 *pUserBlock = (uint8 *)_pBlock;
		CMemoryBlock *pMemBlock = mp_Blocks.f_FindEqual(pUserBlock);
		if (!pMemBlock)
			DMibPDebugBreak;

		if (_Size && _Size != pMemBlock->m_OriginalSize && _Size != pMemBlock->m_RealSize)
			DMibPDebugBreak; // Misreported size

		CReportMemoryLightweight *pReporter = (CReportMemoryLightweight *)mp_Heap.f_GetCustomThreadLocal(0);
		if (pReporter)
			pReporter->f_Free(_Size);

		umint GranularityProtect = CAllocator_VirtualNoTracking::f_GranularityProtect();

		umint UserNeededSize = fg_AlignUp(pMemBlock->m_AllocSize, GranularityProtect);

		mp_FreeBlocks[mp_Sequence++] = *pMemBlock;
		++mp_nFreeBlocks;
		mp_nFreeMemBytes += pMemBlock->m_RealSize;

		uint8 *pBlock = pMemBlock->m_pStart;
		uint8 *pBlockStart = fg_AlignUp(pBlock, GranularityProtect);
		uint8 *pRetBlock = pBlockStart + GranularityProtect;

		if (!mp_bProtectOnDemand || mp_bProtectionDemanded)
			NSys::fg_Mem_VirtualProtect(pRetBlock, UserNeededSize, EProtect_Exec);

		mp_Blocks.f_Remove(pMemBlock);

		while (mp_nFreeMemBytes > mp_nMaxFreeMemory)
			fp_RemoveFreeBlock();
	}
}
