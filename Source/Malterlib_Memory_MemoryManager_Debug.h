// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Container/MapWithPool>

#include "Malterlib_Memory_MemoryManager_DebugException.h"

namespace NMib::NMemory
{
	struct CMemoryManagerDebugOptionsDefault
	{
		enum
		{
			mc_bCheckModifyAfterFree	= true
			, mc_bAsanPoisioning		= true
			, mc_bFillAllocated			= true
			, mc_StackTraceDepth		= 64
			, mc_nPreGuardBytes			= sizeof(mint) * 2
			, mc_nPostGuardBytes		= sizeof(mint) * 2
			, mc_Fill_Free				= 0xDD
			, mc_Fill_Allocated			= 0xCD
			, mc_Fill_Guard				= 0xFD
			, mc_bCanAllocateNonTracked	= true // Used in non-tracked heap to break recursive dependency on non-tracked heap
			, mc_bFreeValidation		= true // Enables validation of free blocks (double free)
			, mc_bEnumeration			= true // Enables enumeration. Needed for reporting leaks.
			, mc_bTraceLeaks			= true // Enables leak tracing
			, mc_bAssertOnMemoryLeak	= false
		};
	};
	template <typename t_CParams, bool t_bException, typename t_COptions = CMemoryManagerDebugOptionsDefault>
	struct TCMemoryManagerDebug;

	template <mint t_PaddingSize>
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
		static void fs_FillFree(uint8 *_pMemory, mint _nBytes);
		static void fs_FillAllocated(uint8 *_pMemory, mint _nBytes);
		static void fs_FillGuard(uint8 *_pMemory, mint _nBytes);

		static bool fs_CheckFree(uint8 *_pMemory, mint _Size, bool _bBreak);
		static bool fs_CheckGuard(ch8 const *_pMessage, uint8 *_pMemory, mint _Size, bool _bBreak);

		static bool fs_ReportDamage(ch8 const *_pMessage, uint8 *_pMemory, uint8 _Fill, bool _bBreak);

		struct CNotifier
		{
			struct CAllocInfo
			{
				mint m_Size;
				CMibCodeAddress m_CallStack[t_COptions::mc_StackTraceDepth ? t_COptions::mc_StackTraceDepth : 1];
				mint m_nCallStack;
				mint m_AllocID = 0;

				uint8 * f_GetAddress() const;
			};

			struct CGlobal;

			struct CArena
			{
				enum
				{
					mc_EnableCallbacks = true
				};

				CArena(CGlobal *_pGlobal);

				void f_OnFree(uint8 *_pMemory);
				void f_OnAlloc(uint8 *_pMemory, mint _nBytes);
				void f_OnFreeOtherThread(uint8 *_pMemory);

				void f_OnFillFree(uint8 *_pMemory, mint _nBytes, EMemoryManagerCheckFlag _Flags = EMemoryManagerCheckFlag_Protect);
				bool f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags);

				NContainer::TCMapWithPool<uint8 *, CAllocInfo, NMib::CSort_Default, typename t_CParams::CAllocator, (2*1024*1024) / sizeof(CAllocInfo), NMib::NMemory::CPoolType_Growing>
					m_Allocations
				;
				CGlobal *m_pGlobal;
			};

			struct CHeap
			{
				enum
				{
					mc_EnableCallbacks = true
				};

				CHeap(CGlobal *_pGlobal);

				void f_OnFree(uint8 *_pMemory);
				void f_OnAlloc(uint8 *_pMemory, mint _nBytes);

				void f_OnFillFree(uint8 *_pMemory, mint _nBytes, EMemoryManagerCheckFlag _Flags = EMemoryManagerCheckFlag_Protect);
				bool f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags);

				NContainer::TCMapWithPool<uint8 *, CAllocInfo, NMib::CSort_Default, typename t_CParams::CAllocator, (2*1024*1024) / sizeof(CAllocInfo), NMib::NMemory::CPoolType_Growing>
					m_Allocations
				;
				CGlobal *m_pGlobal;
			};

			struct CGlobal
			{
				enum
				{
					mc_EnableCallbacks = true
				};

				CGlobal(TCMemoryManager<TCMemoryManagerDebugParams> &_MemoryManager);

				void f_OnAlloc(uint8 *_pMemory, mint _nBytes);
				void f_OnFree(uint8 *_pMemory);

				void f_OnCommit(uint8 *_pMemory, mint _nBytes);
				void f_OnDecommit(uint8 *_pMemory, mint _nBytes);

				align_cacheline NAtomic::TCAtomic<mint> m_AllocIDCounter = 0;
				align_cacheline NThread::CMutual m_Lock;

				NContainer::TCMapWithPool<uint8 *, CAllocInfo, NMib::CSort_Default, typename t_CParams::CAllocator, (2*1024*1024) / sizeof(CAllocInfo), NMib::NMemory::CPoolType_Growing>
					m_Allocations
				;
			};
		};

	};

	template <typename t_CParams, bool t_bException, typename t_COptions>
	struct TCMemoryManagerDebug : public TCMemoryManager<TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>>
	{
	public:
		template <typename t_CParams2, bool t_bException2, typename t_COptions2>
		friend struct TCMemoryManagerDebugParams;

		using CParams = TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>;
		using CSuper = TCMemoryManager<CParams>;

	public:
		template <typename... tfp_CAllocator>
		TCMemoryManagerDebug(tfp_CAllocator &&..._Params);

		~TCMemoryManagerDebug();

		void *f_AllocWithSize(mint &_Size);
		void *f_Alloc(mint _Size);
		void *f_AllocWithSizeInline(mint &_Size);
		void *f_AllocAlignedWithSize(mint &_Size, mint _Alignment);
		void *f_AllocAligned(mint _Size, mint _Alignment);
		void *f_AllocAlignedWithSizeInline(mint &_Size, mint _Alignment);
		void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);

		void *f_Realloc(void * _pMemory, mint &_Size, mint _OldSize);
		void *f_ReallocInline(void * _pMemory, mint &_Size, mint _OldSize);
		void *f_Resize(void * _pMemory, mint &_Size, mint _OldSize);
		void *f_ResizeInline(void * _pMemory, mint &_Size, mint _OldSize);

		void f_Free(void * _pMemory, mint _Size);
		void f_FreeInline(void * _pMemory, mint _Size);
		void f_FreeNoSizeInline(void * _pMemory);
		void f_FreeNoSize(void * _pMemory);

		void *f_AllocWithSizeDebug(mint &_Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);

		mint f_Size(void const * _pMemory) const;
		mint f_TrySize(void const * _pMemory) const;
		mint f_SizeInline(void const * _pMemory) const;
		fp32 f_Overhead(void const * _pMemory);
		bool f_ContainsBlock(void const * _pMemory);

		void f_PrepareFork();
		void f_ForkedChild();
		void f_ForkedParent();

		TCMemoryManagerDebug *f_GetMemoryManager(void const *_pMemory); // Will only work between managers that share the same magic

		void f_DestroyThreadLocals();

		bool f_CheckAll(EMemoryManagerCheckFlag _Flags);

		mint f_SizePadded(mint _Size);

		void f_EnumAllocations
			(
				NFunction::TCFunctionNoAlloc
				<
					void (uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags, mint _ThreadID, mint _AllocID)
				> const &_Functor
			)
		;

		void f_ReportLeaks();
		bool f_ReportingLeaks();
	private:

		struct CPreBlockData
		{
			mint m_Reserved; // Overwritten when put on free list of another arena
			uint64 m_Magic;
			mint m_Offset;
			mint m_PreCheck;
			mint m_PostCheck;
			mint m_Size;
			mint m_RequestedSize;
			ch8 const * m_pFile;
			mint m_ThreadID;
			uint32 m_Line;
			EHeapDebugFlag m_Flags;
		};

		constexpr static mint mc_PreBlockDataSize = sizeof(CPreBlockData) + t_COptions::mc_nPreGuardBytes;
		constexpr static mint mc_PreBlockPadding = fg_AlignUpConstExpr(mc_PreBlockDataSize, sizeof(void *) * 2) - mc_PreBlockDataSize;

		struct CPreBlockNormal : public CPreBlockData
		{
			uint8 m_GuardBytes[t_COptions::mc_nPreGuardBytes];

			uint8 *f_GetPreGuard()
			{
				return (uint8 *)(this + 1) - t_COptions::mc_nPreGuardBytes;
			}
		};

		struct CPreBlockPadded : public CPreBlockData
		{
			TCMemoryManagerDebugPaddingHelper<mc_PreBlockPadding> m_Padding;
			uint8 m_GuardBytes[t_COptions::mc_nPreGuardBytes];

			uint8 *f_GetPreGuard()
			{
				return (uint8 *)(this + 1) - t_COptions::mc_nPreGuardBytes;
			}
		};

		using CPreBlock = TCConditional<(mc_PreBlockPadding > 0), CPreBlockPadded, CPreBlockNormal>;

		static_assert(sizeof(CPreBlock) == fg_AlignUpConstExpr(mc_PreBlockDataSize, sizeof(void *) * 2));

		constexpr static mint mc_PreBlockAlignment = fg_Max(alignof(CPreBlock), sizeof(void *) * 2);

		TCPool<zbool, 8, NThread::CMutual, NMemory::CPoolType_Freeable, typename t_CParams::CAllocator> m_ReportingLeaksPool;
		NThread::TCThreadLocalDynamic<zbool> m_bReportingLeaks;

	private:

		static bool fsp_CheckGuard(uint8 *_pMemory, EMemoryManagerCheckFlag _Flags);
		static uint8 *fsp_GetRealMemory(uint8 *_pMemory);

		void fp_EnumAllocations
			(
				NFunction::TCFunctionNoAlloc
				<
					void (uint8 *_pMemory, CPreBlock *_pPreAlloc, typename CParams::CNotifier::CAllocInfo &_AllocInfo)
				> const &_Functor
			)
		;
	};
}

#include "Malterlib_Memory_MemoryManager_Debug.hpp"
