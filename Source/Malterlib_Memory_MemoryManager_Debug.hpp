// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#if defined(DCompiler_clang) && !defined(DPlatformFamily_Emscripten)
#	define DEnableVector
#endif

#ifdef DEnableVector
typedef uint8 vec16uint8 __attribute__((ext_vector_type(16)));
typedef char vec16bool __attribute__((ext_vector_type(16)));
typedef uint64 vec2uint64 __attribute__((ext_vector_type(2)));
#endif

namespace NMib::NMemory
{
#ifdef DEnableVector
	inline_never static bool fg_CheckMemory(uint8 *_pMemory, uint8 _Value, mint _nBytes)
	{
		uint8 *pMemory = _pMemory;
		uint8 *pEnd = fg_AlignUp(pMemory, 16);
		while (pMemory < pEnd)
		{
			if (*pMemory != _Value)
				return true;
			++pMemory;
		}
		pMemory = pEnd;
		pEnd = fg_AlignDown(_pMemory + _nBytes, 16);

		vec16uint8 VecFill = {_Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value, _Value};
		vec16uint8 * pVecStart = (vec16uint8 *)pMemory;
		vec16uint8 * pVecEnd = (vec16uint8 *)pEnd;

		vec16bool Differs = {0};

#if 0
		vec16uint8 * pVecEnd4 = (vec16uint8 *)pEnd - 3;
		while (pVecStart < pVecEnd4)
		{
			Differs = Differs || (pVecStart[0] != VecFill);
			Differs = Differs || (pVecStart[1] != VecFill);
			Differs = Differs || (pVecStart[2] != VecFill);
			Differs = Differs || (pVecStart[3] != VecFill);
			pVecStart += 4;
		}
#endif
		while (pVecStart < pVecEnd)
		{
			Differs = Differs || (*pVecStart != VecFill);
			++pVecStart;
		}

		if ((*((vec2uint64 *)&Differs))[0] || (*((vec2uint64 *)&Differs))[1])
			return true;

		pMemory = pEnd;
		pEnd = _pMemory + _nBytes;

		while (pMemory < pEnd)
		{
			if (*pMemory != _Value)
				return true;
			++pMemory;
		}
		return false;
	}
#else
	static bool fg_CheckMemory(uint8 *_pMemory, uint8 _Value, mint _nBytes)
	{
		return fg_MemCmpOne(_pMemory, _Value, _nBytes) != 0;
	}
#endif

	///
	/// Debug
	/// =====

	template <typename t_CParams, bool t_bException, typename t_COptions>
	template <typename... tfp_CAllocator>
	TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::TCMemoryManagerDebug(tfp_CAllocator &&..._Params)
		: CSuper(fg_Forward<tfp_CAllocator>(_Params)...)
		, m_bReportingLeaks
		(
			[this] (zbool *_pParent, bool _bMove) -> zbool *
			{
				if (_pParent)
					return m_ReportingLeaksPool.f_New(*_pParent);
				else
					return m_ReportingLeaksPool.f_New();
			}
			, [this] (zbool *_pParent)
			{
				m_ReportingLeaksPool.f_Delete(_pParent);
			}
		)
	{

	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::~TCMemoryManagerDebug()
	{

	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_DestroyThreadLocals()
	{
		m_bReportingLeaks.f_Destroy();
		CSuper::f_DestroyThreadLocals();
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::fp_EnumAllocations
		(
			NFunction::TCFunctionNoAlloc
			<
				void (uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, CPreBlock *_pPreAlloc)
			> const &_Functor
		)
	{
		if (!t_COptions::mc_bEnumeration)
			return;

		auto fl_ReportAlloc
			= [&](typename TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CAllocInfo & _AllocInfo)
			{

				uint8 * pAddress = _AllocInfo.f_GetAddress();
				if
					(
						(t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes) != 0
						&& _AllocInfo.m_Size > sizeof(CPreBlock) && ((CPreBlock *)pAddress)->m_Magic == 0x12b30ce0658a1ceaULL
					)
				{
					CPreBlock *pPreBlock = (CPreBlock *)(pAddress + ((CPreBlock *)pAddress)->m_Offset);
					_Functor(pAddress, _AllocInfo.m_Size, _AllocInfo.m_CallStack, _AllocInfo.m_nCallStack, pPreBlock);
				}
			}
		;
		CSuper::f_EnumArenas
			(
				[&](typename CParams::CNotifier::CArena *_pArena)
				{
					for (auto iAlloc = _pArena->m_Allocations.f_GetIterator(); iAlloc; ++iAlloc)
						fl_ReportAlloc(*iAlloc);
				}
			)
		;
		CSuper::f_EnumHeaps
			(
				[&](typename CParams::CNotifier::CHeap *_pHeap)
				{
					for (auto iAlloc = _pHeap->m_Allocations.f_GetIterator(); iAlloc; ++iAlloc)
						fl_ReportAlloc(*iAlloc);
				}
			)
		;
		CSuper::f_EnumGlobal
			(
				[&](typename CParams::CNotifier::CGlobal *_pGlobal)
				{
					for (auto iAlloc = _pGlobal->m_Allocations.f_GetIterator(); iAlloc; ++iAlloc)
						fl_ReportAlloc(*iAlloc);
				}
			)
		;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	bool TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_CheckAll(bool _bBreak)
	{
		bool bError = false;
		fp_EnumAllocations
			(
				[&](uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, CPreBlock *_pPreBlock)
				{
					auto pAddress = (uint8 *)(_pPreBlock + 1);
					if (fsp_CheckGuard((uint8 *)pAddress, _bBreak))
						bError = true;
				}
			)
		;

		if (this->f_CheckFree(_bBreak))
			bError = true;

		return !bError;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_EnumAllocations
		(
			NFunction::TCFunctionNoAlloc
			<
				void (uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags, mint _ThreadID)
			> const &_Functor
		)
	{

		fp_EnumAllocations
			(
				[&](uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, CPreBlock *_pPreBlock)
				{
					auto pAddress = (uint8 *)(_pPreBlock + 1);
					_Functor(pAddress, _pPreBlock->m_Size, _pStackTrace, _nStackTrace, _pPreBlock->m_pFile, _pPreBlock->m_Line, _pPreBlock->m_Flags, _pPreBlock->m_ThreadID);
				}
			)
		;
	}

	namespace NPrivate
	{
		void fg_ReportLeak(uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags, mint _ThreadID, bool _bCanAllocateNonTracked);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocWithSizeInline(mint &_Size)
	{
		return f_AllocWithSize(_Size);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocWithSize(mint &_Size)
	{
		return f_AllocWithSizeDebug(_Size, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_Alloc(mint _Size)
	{
		return f_AllocWithSizeDebug(_Size, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	bool TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ReportingLeaks()
	{
		auto *pReportingLeaks = m_bReportingLeaks.f_TryGet();
		if (!pReportingLeaks)
		{
			if (fg_GetSys()->f_ThreadDestroyed())
				return false;
			return *m_bReportingLeaks;
		}
		return *pReportingLeaks;
	};


	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ReportLeaks()
	{
		if (!t_COptions::mc_bTraceLeaks)
			return;
		*m_bReportingLeaks = true;
		auto Cleanup = fg_OnScopeExit
			(
				[&]()
				{
					*m_bReportingLeaks = false;
				}
			)
		;
		f_EnumAllocations
			(
				[&](uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags, mint _ThreadID)
				{
					NPrivate::fg_ReportLeak(_pMemory, _Size, _pStackTrace, _nStackTrace, _pFile, _Line, _Flags, _ThreadID, t_COptions::mc_bCanAllocateNonTracked);
				}
			)
		;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocWithSizeDebug(mint &_Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibFastCheck(!f_ReportingLeaks());
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			mint PreBytes = sizeof(CPreBlock);
			mint PostBytes = t_COptions::mc_nPostGuardBytes;
			mint Size = _Size + PreBytes + PostBytes;
			uint8 * pMemory = (uint8 *)CSuper::f_AllocWithSize(Size);

			mint RequestedSize = _Size;
			_Size = Size - (PreBytes + PostBytes);

			CPreBlock *pPreBlock = (CPreBlock *)(pMemory + PreBytes - sizeof(CPreBlock));

			pPreBlock->m_PreCheck = PreBytes;
			pPreBlock->m_PostCheck = PostBytes;
			pPreBlock->m_RequestedSize = RequestedSize;
			pPreBlock->m_Size = _Size;
			pPreBlock->m_Offset = PreBytes - sizeof(CPreBlock);
			pPreBlock->m_pFile = _pFile;
			pPreBlock->m_Line = _Line;
			pPreBlock->m_Flags = _Flags;
			pPreBlock->m_ThreadID = NSys::fg_Thread_GetCurrentUID();
			pPreBlock->m_Magic = 0x12b30ce0658a1ceaULL;
			DMibFastCheck(pPreBlock->m_Offset == 0 || pPreBlock->m_Offset >= (sizeof(pPreBlock->m_Magic) + sizeof(pPreBlock->m_Offset)));
			((CPreBlock *)pMemory)->m_Offset = pPreBlock->m_Offset;
			((CPreBlock *)pMemory)->m_Magic = pPreBlock->m_Magic;

			if (t_COptions::mc_nPreGuardBytes)
				CParams::fs_FillGuard(pPreBlock->f_GetPreGuard(), t_COptions::mc_nPreGuardBytes);
			if (t_COptions::mc_bFillAllocated)
				CParams::fs_FillAllocated(pMemory + PreBytes, _Size);
			if (PostBytes)
				CParams::fs_FillGuard(pMemory + PreBytes + _Size, t_COptions::mc_nPostGuardBytes);

			return pMemory + PreBytes;
		}
		uint8* pMemory = (uint8*)CSuper::f_AllocWithSize(_Size);
		if (t_COptions::mc_bFillAllocated)
			CParams::fs_FillAllocated(pMemory, _Size);
		return pMemory;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocAlignedWithSize(mint &_Size, mint _Alignment)
	{
		return f_AllocAlignedWithSizeDebug(_Size, _Alignment, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocAligned(mint _Size, mint _Alignment)
	{
		return f_AllocAlignedWithSizeDebug(_Size, _Alignment, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocAlignedWithSizeInline(mint &_Size, mint _Alignment)
	{
		return f_AllocAlignedWithSizeDebug(_Size, _Alignment, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		return f_AllocBatchDebug(_Size, _Alignment, _Functor, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibFastCheck(!f_ReportingLeaks());
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			struct CFunctorOptions
			{
				ch8 const * m_pFile;
				uint32 m_Line;
				EHeapDebugFlag m_Flags;
				mint m_Alignment;
				mint m_RequestedSize;
				NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const *m_pFunctor;
			};

			CFunctorOptions Options;

			Options.m_pFile = _pFile;
			Options.m_Line = _Line;
			Options.m_Flags = _Flags;
			Options.m_pFunctor = &_Functor;
			Options.m_Alignment = _Alignment;
			Options.m_RequestedSize = fg_AlignUp(_Size, _Alignment);

			mint PreBytes = fg_AlignUp(sizeof(CPreBlock) + (sizeof(((CPreBlock *)nullptr)->m_Magic) + sizeof(((CPreBlock *)nullptr)->m_Offset)), _Alignment);
			mint PostBytes = fg_AlignUp(t_COptions::mc_nPostGuardBytes, _Alignment);
			mint Size = _Size + PreBytes + PostBytes;

			CSuper::f_AllocBatch
				(
					Size
					, _Alignment
					, [&Options](void * _pAlloc, mint _Size)
					{
						mint PreBytes = fg_AlignUp(sizeof(CPreBlock) + (sizeof(((CPreBlock *)nullptr)->m_Magic) + sizeof(((CPreBlock *)nullptr)->m_Offset)), Options.m_Alignment);
						mint PostBytes = fg_AlignUp(t_COptions::mc_nPostGuardBytes, Options.m_Alignment);
						uint8 * pMemory = (uint8 *)_pAlloc;

						mint RetSize = _Size - (PreBytes + PostBytes);

						CPreBlock *pPreBlock = (CPreBlock *)(pMemory + PreBytes - sizeof(CPreBlock));

						pPreBlock->m_PreCheck = PreBytes;
						pPreBlock->m_PostCheck = PostBytes;
						pPreBlock->m_RequestedSize = Options.m_RequestedSize;
						pPreBlock->m_Size = RetSize;
						pPreBlock->m_Offset = PreBytes - sizeof(CPreBlock);
						pPreBlock->m_pFile = Options.m_pFile;
						pPreBlock->m_Line = Options.m_Line;
						pPreBlock->m_Flags = Options.m_Flags;
						pPreBlock->m_ThreadID = NSys::fg_Thread_GetCurrentUID();
						pPreBlock->m_Magic = 0x12b30ce0658a1ceaULL;
						DMibFastCheck(pPreBlock->m_Offset == 0 || pPreBlock->m_Offset >= (sizeof(pPreBlock->m_Magic) + sizeof(pPreBlock->m_Offset)));
						((CPreBlock *)pMemory)->m_Offset = pPreBlock->m_Offset;
						((CPreBlock *)pMemory)->m_Magic = pPreBlock->m_Magic;

						if (t_COptions::mc_nPreGuardBytes != 0)
							CParams::fs_FillGuard(pPreBlock->f_GetPreGuard(), t_COptions::mc_nPreGuardBytes);
						if (t_COptions::mc_bFillAllocated != 0)
							CParams::fs_FillAllocated(pMemory + PreBytes, RetSize);
						if (PostBytes)
							CParams::fs_FillGuard(pMemory + PreBytes + RetSize, t_COptions::mc_nPostGuardBytes);

						return (*Options.m_pFunctor)(pMemory + PreBytes, RetSize);
					}
				)
			;

			return;
		}

		if (t_COptions::mc_bFillAllocated)
		{
			struct CFunctorOptions
			{
				NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const *m_pFunctor;
			};

			CFunctorOptions Options;

			Options.m_pFunctor = &_Functor;

			CSuper::f_AllocBatch
				(
					_Size
					, _Alignment
					, [&Options](void * _pAlloc, mint _Size)
					{
						uint8 * pMemory = (uint8 *)_pAlloc;
						CParams::fs_FillAllocated(pMemory, _Size);
						return (*Options.m_pFunctor)(pMemory, _Size);
					}
				)
			;

			return;
		}
		return CSuper::f_AllocBatch(_Size, _Alignment, _Functor);
	}


	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibFastCheck(!f_ReportingLeaks());
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			mint PreBytes = fg_AlignUp(sizeof(CPreBlock) + (sizeof(((CPreBlock *)nullptr)->m_Magic) + sizeof(((CPreBlock *)nullptr)->m_Offset)), _Alignment);
			mint PostBytes = fg_AlignUp(t_COptions::mc_nPostGuardBytes, _Alignment);
			mint Size = _Size + PreBytes + PostBytes;
			uint8 * pMemory = (uint8 *)CSuper::f_AllocAlignedWithSize(Size, _Alignment);

			mint RequestedSize = fg_AlignUp(_Size, _Alignment);
			_Size = Size - (PreBytes + PostBytes);

			CPreBlock *pPreBlock = (CPreBlock *)(pMemory + PreBytes - sizeof(CPreBlock));

			pPreBlock->m_PreCheck = PreBytes;
			pPreBlock->m_PostCheck = PostBytes;
			pPreBlock->m_RequestedSize = RequestedSize;
			pPreBlock->m_Size = _Size;
			pPreBlock->m_Offset = PreBytes - sizeof(CPreBlock);
			pPreBlock->m_pFile = _pFile;
			pPreBlock->m_Line = _Line;
			pPreBlock->m_Flags = _Flags;
			pPreBlock->m_ThreadID = NSys::fg_Thread_GetCurrentUID();
			pPreBlock->m_Magic = 0x12b30ce0658a1ceaULL;
			DMibFastCheck(pPreBlock->m_Offset == 0 || pPreBlock->m_Offset >= (sizeof(pPreBlock->m_Magic) + sizeof(pPreBlock->m_Offset)));
			((CPreBlock *)pMemory)->m_Offset = pPreBlock->m_Offset;
			((CPreBlock *)pMemory)->m_Magic = pPreBlock->m_Magic;

			if (t_COptions::mc_nPreGuardBytes)
				CParams::fs_FillGuard(pPreBlock->f_GetPreGuard(), t_COptions::mc_nPreGuardBytes);
			if (t_COptions::mc_bFillAllocated)
				CParams::fs_FillAllocated(pMemory + PreBytes, _Size);
			if (PostBytes)
				CParams::fs_FillGuard(pMemory + PreBytes + _Size, t_COptions::mc_nPostGuardBytes);

			return pMemory + PreBytes;
		}
		uint8* pMemory = (uint8*)CSuper::f_AllocAlignedWithSize(_Size, _Alignment);
		if (t_COptions::mc_bFillAllocated)
			CParams::fs_FillAllocated(pMemory, _Size);
		return pMemory;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	bool TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::fsp_CheckGuard(uint8 *_pMemory, bool _bBreak)
	{
		CPreBlock *pPreBlock = ((CPreBlock *)_pMemory) - 1;

		bool bError = false;

		if (t_COptions::mc_nPreGuardBytes)
		{
			if (CParams::fs_CheckGuard("Memory overwritten before allocated block", pPreBlock->f_GetPreGuard(), t_COptions::mc_nPreGuardBytes, _bBreak))
				bError = true;

		}
		if (t_COptions::mc_nPostGuardBytes)
		{
			if (CParams::fs_CheckGuard("Memory overwritten after allocated block", _pMemory + pPreBlock->m_Size, t_COptions::mc_nPostGuardBytes, _bBreak))
				bError = true;
		}
		return bError;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	uint8 *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::fsp_GetRealMemory(uint8 *_pMemory)
	{
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			CPreBlock *pOldPreBlock = ((CPreBlock *)_pMemory) - 1;
			uint8 * pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_PreCheck;
			return pOldMemory;
		}
		return _pMemory;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_Realloc(void * _pMemory, mint &_Size, mint _OldSize)
	{
		DMibFastCheck(!f_ReportingLeaks());
		return f_ReallocDebug(_pMemory, _Size, _OldSize, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ReallocInline(void * _pMemory, mint &_Size, mint _OldSize)
	{
		DMibFastCheck(!f_ReportingLeaks());
		return f_ReallocDebug(_pMemory, _Size, _OldSize, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ReallocDebug(void * _pMemory, mint &_Size, mint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		if (!_pMemory)
			return f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
		DMibFastCheck(!f_ReportingLeaks());
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			fsp_CheckGuard((uint8 *)_pMemory, true);

			CPreBlock *pOldPreBlock = ((CPreBlock *)_pMemory) - 1;
			uint8 * pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_PreCheck;

			mint PreBytes = sizeof(CPreBlock);
			mint PostBytes = t_COptions::mc_nPostGuardBytes;
			mint Size = _Size + PreBytes + PostBytes;

			mint OldPaddedSize = 0;
			if (_OldSize)
			{
				DMibFastCheck(_OldSize == pOldPreBlock->m_Size || _OldSize == pOldPreBlock->m_RequestedSize);
				OldPaddedSize = _OldSize + PreBytes + PostBytes;
			}

			uint8 * pMemory = (uint8 *)CSuper::f_Realloc(pOldMemory, Size, OldPaddedSize);

			if (pMemory == pOldMemory)
				return _pMemory;

			mint RequestedSize = _Size;
			_Size = Size - (PreBytes + PostBytes);

			CPreBlock *pPreBlock = (CPreBlock *)(pMemory + PreBytes - sizeof(CPreBlock));

			pPreBlock->m_PreCheck = PreBytes;
			pPreBlock->m_PostCheck = PostBytes;
			pPreBlock->m_RequestedSize = RequestedSize;
			pPreBlock->m_Size = _Size;
			pPreBlock->m_Offset = PreBytes - sizeof(CPreBlock);
			pPreBlock->m_pFile = _pFile;
			pPreBlock->m_Line = _Line;
			pPreBlock->m_Flags = _Flags;
			pPreBlock->m_ThreadID = NSys::fg_Thread_GetCurrentUID();
			pPreBlock->m_Magic = 0x12b30ce0658a1ceaULL;
			DMibFastCheck(pPreBlock->m_Offset == 0 || pPreBlock->m_Offset >= (sizeof(pPreBlock->m_Magic) + sizeof(pPreBlock->m_Offset)));
			((CPreBlock *)pMemory)->m_Offset = pPreBlock->m_Offset;
			((CPreBlock *)pMemory)->m_Magic = pPreBlock->m_Magic;

			if (t_COptions::mc_nPreGuardBytes)
				CParams::fs_FillGuard(pPreBlock->f_GetPreGuard(), t_COptions::mc_nPreGuardBytes);
			if (t_COptions::mc_bFillAllocated)
				CParams::fs_FillAllocated(pMemory + PreBytes, _Size);
			if (PostBytes)
				CParams::fs_FillGuard(pMemory + PreBytes + _Size, t_COptions::mc_nPostGuardBytes);

			return pMemory + PreBytes;
		}

		uint8* pMemory = (uint8*)CSuper::f_Realloc(_pMemory, _Size, _OldSize);
		if (t_COptions::mc_bFillAllocated)
			CParams::fs_FillAllocated(pMemory, _Size);
		return pMemory;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_Resize(void * _pMemory, mint &_Size, mint _OldSize)
	{
		return f_ResizeDebug(_pMemory, _Size, _OldSize, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ResizeInline(void * _pMemory, mint &_Size, mint _OldSize)
	{
		return f_ResizeDebug(_pMemory, _Size, _OldSize, nullptr, 0, EHeapDebugFlag_None);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void *TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ResizeDebug(void * _pMemory, mint &_Size, mint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		if (!_pMemory)
			return f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
		DMibFastCheck(!f_ReportingLeaks());
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			fsp_CheckGuard((uint8 *)_pMemory, true);

			CPreBlock *pOldPreBlock = ((CPreBlock *)_pMemory) - 1;
			mint OldSize = pOldPreBlock->m_Size;
			uint8 * pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_PreCheck;

			mint PreBytes = pOldPreBlock->m_PreCheck;
			mint PostBytes = t_COptions::mc_nPostGuardBytes;
			mint Size = _Size + PreBytes + PostBytes;

			mint OldPaddedSize = 0;
			if (_OldSize)
			{
				DMibFastCheck(_OldSize == pOldPreBlock->m_Size || _OldSize == pOldPreBlock->m_RequestedSize);
				OldPaddedSize = _OldSize + PreBytes + PostBytes;
			}

			uint8 * pMemory = (uint8 *)CSuper::f_Resize(pOldMemory, Size, OldPaddedSize);

			if (pMemory == pOldMemory)
				return _pMemory;

			mint RequestedSize = _Size;
			_Size = Size - (PreBytes + PostBytes);

			CPreBlock *pPreBlock = (CPreBlock *)(pMemory + PreBytes - sizeof(CPreBlock));

			pPreBlock->m_PreCheck = PreBytes;
			pPreBlock->m_PostCheck = PostBytes;
			pPreBlock->m_RequestedSize = RequestedSize;
			pPreBlock->m_Size = _Size;
			pPreBlock->m_Offset = PreBytes - sizeof(CPreBlock);
			pPreBlock->m_pFile = _pFile;
			pPreBlock->m_Line = _Line;
			pPreBlock->m_Flags = _Flags;
			pPreBlock->m_ThreadID = NSys::fg_Thread_GetCurrentUID();
			pPreBlock->m_Magic = 0x12b30ce0658a1ceaULL;
			DMibFastCheck(pPreBlock->m_Offset == 0 || pPreBlock->m_Offset >= (sizeof(pPreBlock->m_Magic) + sizeof(pPreBlock->m_Offset)));
			((CPreBlock *)pMemory)->m_Offset = pPreBlock->m_Offset;
			((CPreBlock *)pMemory)->m_Magic = pPreBlock->m_Magic;

			if (t_COptions::mc_nPreGuardBytes)
				CParams::fs_FillGuard(pPreBlock->f_GetPreGuard(), t_COptions::mc_nPreGuardBytes);
			if (OldSize < _Size)
			{
				if (t_COptions::mc_bFillAllocated)
					CParams::fs_FillAllocated(pMemory + PreBytes + OldSize, _Size - OldSize);
			}
			if (PostBytes)
				CParams::fs_FillGuard(pMemory + PreBytes + _Size, t_COptions::mc_nPostGuardBytes);

			return pMemory + PreBytes;
		}
		return CSuper::f_Resize(_pMemory, _Size, _OldSize);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	mint TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_SizeInline(void const * _pMemory) const
	{
		return f_Size(_pMemory);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	mint TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_Size(void const * _pMemory) const
	{
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			CPreBlock *pOldPreBlock = ((CPreBlock *)_pMemory) - 1;
			uint8 * pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_PreCheck;

			mint AllocatorSize = CSuper::f_Size(pOldMemory);

			if (AllocatorSize - (pOldPreBlock->m_PostCheck + pOldPreBlock->m_PreCheck) < pOldPreBlock->m_Size)
			{
				if (t_bException)
					DMibErrorMemoryManagerDebug("Corruption in pre block");
				else
					DMibPDebugBreak;
			}

			return pOldPreBlock->m_Size;
		}
		return CSuper::f_Size(_pMemory);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	mint TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_TrySize(void const * _pMemory) const
	{
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			CPreBlock *pOldPreBlock = ((CPreBlock *)_pMemory) - 1;
			uint8 * pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_PreCheck;

			mint AllocatorSize = CSuper::f_TrySize(pOldMemory);
			if (!AllocatorSize)
				return 0;

			if (AllocatorSize - (pOldPreBlock->m_PostCheck + pOldPreBlock->m_PreCheck) < pOldPreBlock->m_Size)
			{
				if (t_bException)
					DMibErrorMemoryManagerDebug("Corruption in pre block");
				else
					DMibPDebugBreak;
			}

			return pOldPreBlock->m_Size;
		}
		return CSuper::f_TrySize(_pMemory);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	fp32 TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_Overhead(void const * _pMemory)
	{
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			CPreBlock *pOldPreBlock = ((CPreBlock *)_pMemory) - 1;
			uint8 * pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_PreCheck;
			return CSuper::f_Overhead(pOldMemory) + pOldPreBlock->m_PostCheck + pOldPreBlock->m_PreCheck;
		}

		uint8 * pOldMemory = fsp_GetRealMemory((uint8 *)_pMemory);
		return CSuper::f_Overhead(pOldMemory);
	}


	template <typename t_CParams, bool t_bException, typename t_COptions>
	bool TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_ContainsBlock(void const * _pMemory)
	{
		uint8 * pOldMemory = fsp_GetRealMemory((uint8 *)_pMemory);

		return CSuper::f_ContainsBlock(pOldMemory);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	auto TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_GetMemoryManager(void const *_pMemory) -> TCMemoryManagerDebug *
	{
		auto *pMemoryManager = CSuper::f_GetMemoryManager(_pMemory);
		return fg_AutoStaticCast(pMemoryManager);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_FreeInline(void * _pMemory, mint _Size)
	{
		return f_Free(_pMemory, _Size);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_Free(void *_pMemory, mint _Size)
	{
		if (!_pMemory)
			return;
		DMibFastCheck(_Size != 0);
		DMibFastCheck(!f_ReportingLeaks());
		fsp_CheckGuard((uint8 *)_pMemory, true);

		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			CPreBlock *pPreBlock = ((CPreBlock *)_pMemory) - 1;
			uint8 * pOldMemory = (uint8 *)_pMemory - pPreBlock->m_PreCheck;

			mint PreBytes = sizeof(CPreBlock);
			mint PostBytes = t_COptions::mc_nPostGuardBytes;
			mint Size = _Size + PreBytes + PostBytes;

			DMibFastCheck(_Size == pPreBlock->m_Size || _Size == pPreBlock->m_RequestedSize);
			return CSuper::f_Free(pOldMemory, Size);
		}
		else
		{
			uint8 * pOldMemory = fsp_GetRealMemory((uint8 *)_pMemory);
			return CSuper::f_Free(pOldMemory, _Size);
		}
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_FreeNoSize(void *_pMemory)
	{
		if (!_pMemory)
			return;
		DMibFastCheck(!f_ReportingLeaks());
		fsp_CheckGuard((uint8 *)_pMemory, true);

		uint8 * pOldMemory = fsp_GetRealMemory((uint8 *)_pMemory);
		return CSuper::f_FreeNoSize(pOldMemory);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_FreeNoSizeInline(void * _pMemory)
	{
		return f_FreeNoSize(_pMemory);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	mint TCMemoryManagerDebug<t_CParams, t_bException, t_COptions>::f_SizePadded(mint _Size)
	{
		if (t_COptions::mc_nPreGuardBytes + t_COptions::mc_nPostGuardBytes)
		{
			mint PreBytes = sizeof(CPreBlock);
			mint PostBytes = t_COptions::mc_nPostGuardBytes;

			mint Size = CSuper::f_SizePadded(_Size + PreBytes + PostBytes);

			return Size - (PreBytes + PostBytes);
		}
		return CSuper::f_SizePadded(_Size);
	}

	///
	/// Params
	/// ======

	template <typename t_CParams, bool t_bException, typename t_COptions>
	uint8 * TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CAllocInfo::f_GetAddress() const
	{
		return NContainer::TCMapWithPool<uint8 *, CAllocInfo>::fs_GetKey(*this);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::fs_FillFree(uint8 *_pMemory, mint _nBytes)
	{
		fg_ObjectSet(_pMemory, t_COptions::mc_Fill_Free, _nBytes);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::fs_FillAllocated(uint8 *_pMemory, mint _nBytes)
	{
		fg_ObjectSet(_pMemory, t_COptions::mc_Fill_Allocated, _nBytes);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::fs_FillGuard(uint8 *_pMemory, mint _nBytes)
	{
		fg_ObjectSet(_pMemory, t_COptions::mc_Fill_Guard, _nBytes);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	bool TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::fs_CheckFree(uint8 *_pMemory, mint _Size, bool _bBreak)
	{
		bool bError = false;

		if (fg_CheckMemory(_pMemory, t_COptions::mc_Fill_Free, _Size))
		{
			// We have a heap overwrite

			uint8 * pFirst = _pMemory;
			uint8 * pFirstEnd = pFirst + _Size;

			while (pFirst < pFirstEnd)
			{
				if ((*pFirst) != t_COptions::mc_Fill_Free)
				{
					if (fs_ReportDamage("Memory overwritten after being freed", pFirst, t_COptions::mc_Fill_Free, _bBreak))
						bError = true;
					if (!t_bException)
						(*pFirst) = t_COptions::mc_Fill_Free;
				}
				++pFirst;
			}
		}
		return bError;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	bool TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::fs_CheckGuard(ch8 const *_pMessage, uint8 *_pMemory, mint _Size, bool _bBreak)
	{
		bool bError = false;
		// "Memory overwritten before allocated block"
		// "Memory overwritten after allocated block"
		if (fg_CheckMemory(_pMemory, t_COptions::mc_Fill_Guard, _Size))
		{
			// We have a heap overwrite

			uint8 * pFirst = _pMemory;
			uint8 * pFirstEnd = pFirst + _Size;

			while (pFirst < pFirstEnd)
			{
				if ((*pFirst) != t_COptions::mc_Fill_Guard)
				{
					if (fs_ReportDamage(_pMessage, pFirst, t_COptions::mc_Fill_Guard, _bBreak))
						bError = true;
					if (!t_bException)
						(*pFirst) = t_COptions::mc_Fill_Guard;
				}
				++pFirst;
			}
		}

		return bError;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never bool TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::fs_ReportDamage(ch8 const *_pMessage, uint8 *_pMemory, uint8 _Fill, bool _bBreak)
	{
		if (!t_bException)
		{
			DMibTraceSafe
				(
					"DAMAGE: {} at 0x{nfh,sf0,sj*}: 0x{nfh,sf0,sj2} resetting to 0x{nfh,sf0,sj2}{\n}"
					, _pMessage
					<< (mint)_pMemory
					<< (sizeof(mint)*2)
					<< *_pMemory
					<< _Fill
				)
			;
		}

		if (_bBreak)
		{
			if (t_bException)
				DMibErrorMemoryManagerDebug(_pMessage);
			else
			{
				static bool bBreak = true;
				if (bBreak)
				{
					DMibPDebugBreak; // Memory damaged
				}
			}
		}
		return true;
	}

	///
	/// Global
	///	======

	template <typename t_CParams, bool t_bException, typename t_COptions>
	TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CGlobal::CGlobal(TCMemoryManager<TCMemoryManagerDebugParams> & _MemoryManager)
		: m_MemoryManager(*((TCMemoryManagerDebug<t_CParams, t_bException, t_COptions> *)&_MemoryManager))
	{
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CGlobal::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
	{
		if (!t_COptions::mc_bFreeValidation && !t_COptions::mc_bEnumeration && t_COptions::mc_StackTraceDepth == 0)
			return;

		DMibLock(m_Lock);
		auto Mapped = m_Allocations(_pMemory);

		if (!Mapped.f_WasCreated())
		{
			// Double alloc (internal error)
			if (t_bException)
				DMibErrorMemoryManagerDebug("Double alloc (internal error)");
			else
				DMibPDebugBreak;
		}

		auto &AllocInfo = *Mapped;

		AllocInfo.m_Size = _nBytes;

		if (t_COptions::mc_StackTraceDepth)
			AllocInfo.m_nCallStack = NMib::NSys::fg_System_GetStackTrace(AllocInfo.m_CallStack, t_COptions::mc_StackTraceDepth);
		else
			AllocInfo.m_nCallStack = 0;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CGlobal::f_OnFree(uint8 *_pMemory)
	{
		if (!t_COptions::mc_bFreeValidation && !t_COptions::mc_bEnumeration && t_COptions::mc_StackTraceDepth == 0)
			return;

		{
			DMibLock(m_Lock);
			auto pOldAlloc = m_Allocations.f_FindEqual(_pMemory);

			if (!pOldAlloc)
			{
				// Double free
				if (t_bException)
					DMibErrorMemoryManagerDebug("Double free");
				else
					DMibPDebugBreak;
			}

			m_Allocations.f_Remove(pOldAlloc);
		}
	}

	///
	/// Arena
	/// =====

	template <typename t_CParams, bool t_bException, typename t_COptions>
	TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CArena::CArena(CGlobal *_pGlobal)
	{
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CArena::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
	{
		if (!t_COptions::mc_bFreeValidation && !t_COptions::mc_bEnumeration && t_COptions::mc_StackTraceDepth == 0)
			return;

		auto Mapped = m_Allocations(_pMemory);

		if (!Mapped.f_WasCreated())
		{
			// Double alloc (internal error)
			if (t_bException)
				DMibErrorMemoryManagerDebug("Double alloc (internal error)");
			else
				DMibPDebugBreak;
		}

		auto &AllocInfo = *Mapped;

		AllocInfo.m_Size = _nBytes;

		if (t_COptions::mc_StackTraceDepth)
			AllocInfo.m_nCallStack = NMib::NSys::fg_System_GetStackTrace(AllocInfo.m_CallStack, t_COptions::mc_StackTraceDepth);
		else
			AllocInfo.m_nCallStack = 0;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CArena::f_OnFree(uint8 *_pMemory)
	{
		if (!t_COptions::mc_bFreeValidation && !t_COptions::mc_bEnumeration && t_COptions::mc_StackTraceDepth == 0)
			return;

		auto pOldAlloc = m_Allocations.f_FindEqual(_pMemory);

		if (!pOldAlloc)
		{
			// Double free
			if (t_bException)
				DMibErrorMemoryManagerDebug("Double free");
			else
				DMibPDebugBreak;
		}

		m_Allocations.f_Remove(pOldAlloc);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CArena::f_OnFillFree(uint8 *_pMemory, mint _nBytes)
	{
		if (t_COptions::mc_bCheckModifyAfterFree)
			fs_FillFree(_pMemory, _nBytes);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never bool TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CArena::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak)
	{
		bool bError = false;
		if (t_COptions::mc_bCheckModifyAfterFree)
		{
			if (fs_CheckFree(_pUntouchedMemory, _nUntouchedBytes, _bBreak))
				bError = true;
		}

		return bError;
	}

	///
	/// Heap
	/// ====

	template <typename t_CParams, bool t_bException, typename t_COptions>
	TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CHeap::CHeap(CGlobal *_pGlobal)
	{
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CHeap::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
	{
		if (!t_COptions::mc_bFreeValidation && !t_COptions::mc_bEnumeration && t_COptions::mc_StackTraceDepth == 0)
			return;

		auto Mapped = m_Allocations(_pMemory);

		if (!Mapped.f_WasCreated())
		{
			// Double alloc (internal error)
			if (t_bException)
				DMibErrorMemoryManagerDebug("Double alloc (internal error)");
			else
				DMibPDebugBreak;
		}

		auto &AllocInfo = *Mapped;

		AllocInfo.m_Size = _nBytes;

		if (t_COptions::mc_StackTraceDepth)
			AllocInfo.m_nCallStack = NMib::NSys::fg_System_GetStackTrace(AllocInfo.m_CallStack, t_COptions::mc_StackTraceDepth);
		else
			AllocInfo.m_nCallStack = 0;
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CHeap::f_OnFree(uint8 *_pMemory)
	{
		if (!t_COptions::mc_bFreeValidation && !t_COptions::mc_bEnumeration && t_COptions::mc_StackTraceDepth == 0)
			return;

		auto pOldAlloc = m_Allocations.f_FindEqual(_pMemory);

		if (!pOldAlloc)
		{
			// Double free
			if (t_bException)
				DMibErrorMemoryManagerDebug("Double free");
			else
				DMibPDebugBreak;
		}

		m_Allocations.f_Remove(pOldAlloc);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never void TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CHeap::f_OnFillFree(uint8 *_pMemory, mint _nBytes)
	{
		if (t_COptions::mc_bCheckModifyAfterFree)
			fs_FillFree(_pMemory, _nBytes);
	}

	template <typename t_CParams, bool t_bException, typename t_COptions>
	inline_never bool TCMemoryManagerDebugParams<t_CParams, t_bException, t_COptions>::CNotifier::CHeap::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak)
	{
		bool bError = false;
		if (t_COptions::mc_bCheckModifyAfterFree)
		{
			if (fs_CheckFree(_pUntouchedMemory, _nUntouchedBytes, _bBreak))
				bError = true;
		}
		return bError;
	}
}
