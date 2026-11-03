// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Container/MapWithPool>

#include "Malterlib_Memory_MemoryManager_DebugException.h"

namespace NMib::NMemory
{
	struct CMemoryManagerDebugOptionsDefault
	{
		static constexpr bool mc_bCheckModifyAfterFree = true;
		static constexpr bool mc_bAsanPoisioning = true;
		static constexpr bool mc_bFillAllocated = true;
		static constexpr aint mc_StackTraceDepth = 64;
		static constexpr aint mc_nPreGuardBytes = sizeof(umint) * 2;
		static constexpr aint mc_nPostGuardBytes = sizeof(umint) * 2;
		static constexpr aint mc_Fill_Free = 0xDD;
		static constexpr aint mc_Fill_Allocated = 0xCD;
		static constexpr aint mc_Fill_Guard = 0xFD;
		static constexpr bool mc_bCanAllocateNonTracked = true;
		static constexpr bool mc_bFreeValidation = true;
		static constexpr bool mc_bEnumeration = true;
		static constexpr bool mc_bTraceLeaks = true;
		static constexpr bool mc_bAssertOnMemoryLeak = false;
	};

	template <typename t_CParams, bool t_bException, typename t_COptions = CMemoryManagerDebugOptionsDefault>
	struct TCMemoryManagerDebug;

	template <umint t_PaddingSize>
	struct TCMemoryManagerDebugPaddingHelper
	{
		uint8 m_Padding[t_PaddingSize];
	};

	template <>
	struct TCMemoryManagerDebugPaddingHelper<0>
	{
	};

	template <typename t_CParams, bool t_bException, typename t_COptions>
	struct TCMemoryManagerDebugParams : public t_CParams
	{
		static void fs_FillFree(uint8 *_pMemory, umint _nBytes);
		static void fs_FillAllocated(uint8 *_pMemory, umint _nBytes);
		static void fs_FillGuard(uint8 *_pMemory, umint _nBytes);

		static bool fs_CheckFree(uint8 *_pMemory, umint _Size, bool _bBreak);
		static bool fs_CheckGuard(ch8 const *_pMessage, uint8 *_pMemory, umint _Size, bool _bBreak);

		static bool fs_ReportDamage(ch8 const *_pMessage, uint8 *_pMemory, uint8 _Fill, bool _bBreak);

		struct CNotifier
		{
			struct CAllocInfo
			{
				umint m_Size;
				CMibCodeAddress m_CallStack[t_COptions::mc_StackTraceDepth ? t_COptions::mc_StackTraceDepth : 1];
				umint m_nCallStack;
				umint m_AllocID = 0;

				uint8 *f_GetAddress() const;
			};

			struct CGlobal;

			struct CArena
			{
				static constexpr aint mc_bEnableCallbacks = true;

				CArena(CGlobal *_pGlobal);

				void f_OnFree(uint8 *_pMemory);
				void f_OnAlloc(uint8 *_pMemory, umint _nBytes);
				void f_OnFreeOtherThread(uint8 *_pMemory);

				void f_OnFillFree(uint8 *_pMemory, umint _nBytes, EMemoryManagerCheckFlag _Flags = EMemoryManagerCheckFlag_Protect);
				bool f_OnCheckFree(uint8 *_pUntouchedMemory, umint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags);

				NContainer::TCMapWithPool<uint8 *, CAllocInfo, NMib::CSort_Default, typename t_CParams::CAllocator, (2 * 1024 * 1024) / sizeof(CAllocInfo), NMib::NMemory::CPoolType_Growing>
					m_Allocations
				;
				CGlobal *m_pGlobal;
			};

			struct CHeap
			{
				static constexpr aint mc_bEnableCallbacks = true;

				CHeap(CGlobal *_pGlobal);

				void f_OnFree(uint8 *_pMemory);
				void f_OnAlloc(uint8 *_pMemory, umint _nBytes);

				void f_OnFillFree(uint8 *_pMemory, umint _nBytes, EMemoryManagerCheckFlag _Flags = EMemoryManagerCheckFlag_Protect);
				bool f_OnCheckFree(uint8 *_pUntouchedMemory, umint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags);

				NContainer::TCMapWithPool<uint8 *, CAllocInfo, NMib::CSort_Default, typename t_CParams::CAllocator, (2 * 1024 * 1024) / sizeof(CAllocInfo), NMib::NMemory::CPoolType_Growing>
					m_Allocations
				;
				CGlobal *m_pGlobal;
			};

			struct CGlobal
			{
				static constexpr aint mc_bEnableCallbacks = true;

				CGlobal(TCMemoryManager<TCMemoryManagerDebugParams> &_MemoryManager);

				void f_OnAlloc(uint8 *_pMemory, umint _nBytes);
				void f_OnFree(uint8 *_pMemory);

				void f_OnCommit(uint8 *_pMemory, umint _nBytes);
				void f_OnDecommit(uint8 *_pMemory, umint _nBytes);

				align_cacheline NAtomic::TCAtomic<umint> m_AllocIDCounter = 0;
				align_cacheline NThread::CMutual m_Lock;

				NContainer::TCMapWithPool<uint8 *, CAllocInfo, NMib::CSort_Default, typename t_CParams::CAllocator, (2 * 1024 * 1024) / sizeof(CAllocInfo), NMib::NMemory::CPoolType_Growing>
					m_Allocations
				;
			};
		};
	};

	template <typename t_CParams, bool t_bException, typename t_COptions>
	struct TCMemoryManagerDebug : public TCMemoryManager<TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>>
	{
		template <typename t_CParams2, bool t_bException2, typename t_COptions2>
		friend struct TCMemoryManagerDebugParams;

		using CParams = TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>;
		using CSuper = TCMemoryManager<CParams>;

	public:
		template <typename ...tfp_CAllocator>
		TCMemoryManagerDebug(tfp_CAllocator && ...p_Params);

		~TCMemoryManagerDebug();

		void *f_AllocWithSize(umint &_Size);
		void *f_Alloc(umint _Size);
		void *f_AllocWithSizeInline(umint &_Size);
		void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment);
		void *f_AllocAligned(umint _Size, umint _Alignment);
		void *f_AllocAlignedWithSizeInline(umint &_Size, umint _Alignment);
		void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor);

		void *f_Realloc(void *_pMemory, umint &_Size, umint _OldSize);
		void *f_ReallocInline(void *_pMemory, umint &_Size, umint _OldSize);
		void *f_Resize(void *_pMemory, umint &_Size, umint _OldSize);
		void *f_ResizeInline(void *_pMemory, umint &_Size, umint _OldSize);

		void f_Free(void *_pMemory, umint _Size);
		void f_FreeInline(void *_pMemory, umint _Size);
		void f_FreeNoSizeInline(void *_pMemory);
		void f_FreeNoSize(void *_pMemory);

		void *f_AllocWithSizeDebug(umint &_Size, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void f_AllocBatchDebug
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, ch8 const *_pFile
				, uint32 _Line
				, EHeapDebugFlag _Flags
			)
		;
		void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags);

		umint f_Size(void const *_pMemory) const;
		umint f_TrySize(void const *_pMemory) const;
		umint f_SizeInline(void const *_pMemory) const;
		fp32 f_Overhead(void const *_pMemory);
		bool f_ContainsBlock(void const *_pMemory);

		void f_PrepareFork();
		void f_ForkedChild();
		void f_ForkedParent();

		TCMemoryManagerDebug *f_GetMemoryManager(void const *_pMemory); // Will only work between managers that share the same magic

		void f_DestroyThreadLocals();

		bool f_CheckAll(EMemoryManagerCheckFlag _Flags);

		umint f_SizePadded(umint _Size);

		void f_EnumAllocations
			(
				NFunction::TCFunctionNoAlloc
				<
					void
					(
						uint8 *_pMemory
						, umint _Size
						, CMibCodeAddress *_pStackTrace
						, umint _nStackTrace
						, ch8 const *_pFile
						, uint32 _Line
						, EHeapDebugFlag _Flags
						, umint _ThreadID
						, umint _AllocID
					)
				>
				const &_fFunctor
			)
		;

		void f_ReportLeaks();
		bool f_ReportingLeaks();

	private:
		struct CPreBlockData
		{
			umint m_Reserved; // Overwritten when put on free list of another arena
			uint64 m_Magic;
			umint m_Offset;
			umint m_PreCheck;
			umint m_PostCheck;
			umint m_Size;
			umint m_RequestedSize;
			ch8 const *m_pFile;
			umint m_ThreadID;
			uint32 m_Line;
			EHeapDebugFlag m_Flags;
		};

		static constexpr umint mcp_PreBlockDataSize = sizeof(CPreBlockData) + t_COptions::mc_nPreGuardBytes;
		static constexpr umint mcp_PreBlockPadding = fg_AlignUpConstExpr(mcp_PreBlockDataSize, sizeof(void *) * 2) - mcp_PreBlockDataSize;

		struct CPreBlockNormal : public CPreBlockData
		{
			uint8 m_GuardBytes[t_COptions::mc_nPreGuardBytes];

			uint8 *f_GetPreGuard();
		};

		struct CPreBlockPadded : public CPreBlockData
		{
			TCMemoryManagerDebugPaddingHelper<mcp_PreBlockPadding> m_Padding;
			uint8 m_GuardBytes[t_COptions::mc_nPreGuardBytes];

			uint8 *f_GetPreGuard();
		};

		using CPreBlock = TCConditional<(mcp_PreBlockPadding > 0), CPreBlockPadded, CPreBlockNormal>;

		static_assert(sizeof(CPreBlock) == fg_AlignUpConstExpr(mcp_PreBlockDataSize, sizeof(void *) * 2));

		static constexpr umint mcp_PreBlockAlignment = fg_Max(alignof(CPreBlock), sizeof(void *) * 2);

		TCPool<zbool, 8, NThread::CMutual, NMemory::CPoolType_Freeable, typename t_CParams::CAllocator> mp_ReportingLeaksPool;
		NThread::TCThreadLocalDynamic<zbool, NThread::EThreadLocalFlag_FastThreadLocal> mp_ReportingLeaks;

	private:
		static bool fsp_CheckGuard(uint8 *_pMemory, EMemoryManagerCheckFlag _Flags);
		static uint8 *fsp_GetRealMemory(uint8 *_pMemory);

		void fp_EnumAllocations(NFunction::TCFunctionNoAlloc<void (uint8 *_pMemory, CPreBlock *_pPreAlloc, typename CParams::CNotifier::CAllocInfo &_AllocInfo)> const &_fFunctor);
	};
}

#include "Malterlib_Memory_MemoryManager_Debug.hpp"
