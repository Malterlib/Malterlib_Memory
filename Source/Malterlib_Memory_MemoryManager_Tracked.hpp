// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	///
	/// Tracked
	/// =======

	template <typename t_CSuper>
	template <typename... tfp_CAllocator>
	TCMemoryManagerTracked<t_CSuper, void>::TCMemoryManagerTracked(ch8 const *_pName, tfp_CAllocator &&..._Params)
		: t_CSuper(fg_Forward<tfp_CAllocator>(_Params)...)
		, mp_pName(_pName)
	{
		DMibMemoryReportAllocatorName(this, _pName);
	}

	template <typename t_CSuper>
	TCMemoryManagerTracked<t_CSuper, void>::~TCMemoryManagerTracked()
	{

	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocWithSizeInline(umint &_Size)
	{
		return f_AllocWithSize(_Size);
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocWithSize(umint &_Size)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = t_CSuper::f_AllocWithSize(_Size);
		DMibMemoryReportAlloc(this, mp_pName, pRet, 0, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocWithSizeDebug(umint &_Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = t_CSuper::f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
		DMibMemoryReportAlloc(this, mp_pName, pRet, 0, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocAlignedWithSizeInline(umint &_Size, umint _Alignment)
	{
		return f_AllocAlignedWithSize(_Size, _Alignment);
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocAlignedWithSize(umint &_Size, umint _Alignment)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = t_CSuper::f_AllocAlignedWithSize(_Size, _Alignment);
		DMibMemoryReportAlloc(this, mp_pName, pRet, _Alignment, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return f_AllocAlignedWithSize(_Size, _Alignment);
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_Alloc(umint _Size)
	{
		return f_AllocAligned(_Size, 1);
	}

	template <typename t_CSuper>
	void TCMemoryManagerTracked<t_CSuper, void>::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor)
	{
		struct CFunctorOptions
		{
			umint m_RequestedSize;
			umint m_Alignment;
			NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const *m_pFunctor;
		};

		CFunctorOptions Options;

		Options.m_pFunctor = &_Functor;
		Options.m_RequestedSize = _Size;
		Options.m_Alignment = _Alignment;

		t_CSuper::f_AllocBatch
			(
				_Size
				, _Alignment
				, [this, &Options](void * _pAlloc, umint _Size)
				{
					(void)this;
					DMibMemoryGoingToReportScope(this, true);
					DMibMemoryReportAlloc(this, mp_pName, _pAlloc, Options.m_Alignment, Options.m_RequestedSize, _Size, this->f_Overhead(_pAlloc), nullptr);
					return (*Options.m_pFunctor)(_pAlloc, _Size);
				}
			)
		;
	}

	template <typename t_CSuper>
	void TCMemoryManagerTracked<t_CSuper, void>::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		struct CFunctorOptions
		{
			ch8 const * m_pFile;
			uint32 m_Line;
			EHeapDebugFlag m_Flags;
			umint m_RequestedSize;
			umint m_Alignment;
			NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const *m_pFunctor;
		};

		CFunctorOptions Options;

		Options.m_pFile = _pFile;
		Options.m_Line = _Line;
		Options.m_Flags = _Flags;
		Options.m_pFunctor = &_Functor;
		Options.m_RequestedSize = _Size;
		Options.m_Alignment = _Alignment;

		t_CSuper::f_AllocBatchDebug
			(
				_Size
				, _Alignment
				, [this, &Options](void * _pAlloc, umint _Size)
				{
					(void)this;
					DMibMemoryGoingToReportScope(this, true);
					DMibMemoryReportAlloc(this, mp_pName, _pAlloc, Options.m_Alignment, Options.m_RequestedSize, _Size, this->f_Overhead(_pAlloc), nullptr);
					return (*Options.m_pFunctor)(_pAlloc, _Size);
				}
				, _pFile
				, _Line
				, _Flags
			)
		;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = t_CSuper::f_AllocAlignedWithSizeDebug(_Size, _Alignment, _pFile, _Line, _Flags);
		DMibMemoryReportAlloc(this, mp_pName, pRet, _Alignment, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_ReallocInline(void * _pMemory, umint &_Size, umint _OldSize)
	{
		return f_Realloc(_pMemory, _Size, _OldSize);
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_Realloc(void * _pMemory, umint &_Size, umint _OldSize)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(umint Size = _OldSize ? t_CSuper::f_SizePadded(_OldSize) : t_CSuper::f_Size(_pMemory));
		void *pRet = t_CSuper::f_Realloc(_pMemory, _Size, _OldSize);
		DMibMemoryReportRealloc(this, mp_pName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_ReallocDebug(void * _pMemory, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(umint Size = _OldSize ? t_CSuper::f_SizePadded(_OldSize) : t_CSuper::f_Size(_pMemory));
		void *pRet = t_CSuper::f_ReallocDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
		DMibMemoryReportRealloc(this, mp_pName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_ResizeInline(void * _pMemory, umint &_Size, umint _OldSize)
	{
		return f_Resize(_pMemory, _Size, _OldSize);
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_Resize(void * _pMemory, umint &_Size, umint _OldSize)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(umint Size = _OldSize ? t_CSuper::f_SizePadded(_OldSize) : _pMemory ? t_CSuper::f_Size(_pMemory) : 0);
		void *pRet = t_CSuper::f_Resize(_pMemory, _Size, _OldSize);
		DMibMemoryReportResize(this, mp_pName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	void *TCMemoryManagerTracked<t_CSuper, void>::f_ResizeDebug(void * _pMemory, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(umint Size = _OldSize ? t_CSuper::f_SizePadded(_OldSize) : _pMemory ? t_CSuper::f_Size(_pMemory) : 0);
		void *pRet = t_CSuper::f_ResizeDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
		DMibMemoryReportResize(this, mp_pName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, this->f_Overhead(pRet), nullptr);
		return pRet;
	}

	template <typename t_CSuper>
	umint TCMemoryManagerTracked<t_CSuper, void>::f_SizeInline(void const * _pMemory) const
	{
		return f_Size(_pMemory);
	}

	template <typename t_CSuper>
	umint TCMemoryManagerTracked<t_CSuper, void>::f_Size(void const * _pMemory) const
	{
		DMibMemoryGoingToReportScope(this, true);
		umint Ret = t_CSuper::f_Size(_pMemory);
		DMibMemoryReportGetSize(this, mp_pName, _pMemory, Ret, nullptr);
		return Ret;
	}

	template <typename t_CSuper>
	umint TCMemoryManagerTracked<t_CSuper, void>::f_TrySize(void const * _pMemory) const
	{
		DMibMemoryGoingToReportScope(this, true);
		umint Ret = t_CSuper::f_TrySize(_pMemory);
		DMibMemoryReportGetSize(this, mp_pName, _pMemory, Ret, nullptr);
		return Ret;
	}

	template <typename t_CSuper>
	void TCMemoryManagerTracked<t_CSuper, void>::f_FreeInline(void * _pMemory, umint _Size)
	{
		return f_Free(_pMemory, _Size);
	}

	template <typename t_CSuper>
	void TCMemoryManagerTracked<t_CSuper, void>::f_Free(void * _pMemory, umint _Size)
	{
		if (!_pMemory)
			return;
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportExpression(umint Size = t_CSuper::f_SizePadded(_Size));
		t_CSuper::f_Free(_pMemory, _Size);
		DMibMemoryReportFree(this, mp_pName, _pMemory, Size, nullptr);
	}

	template <typename t_CSuper>
	void TCMemoryManagerTracked<t_CSuper, void>::f_FreeNoSizeInline(void * _pMemory)
	{
		return f_FreeNoSize(_pMemory);
	}

	template <typename t_CSuper>
	void TCMemoryManagerTracked<t_CSuper, void>::f_FreeNoSize(void * _pMemory)
	{
		if (!_pMemory)
			return;
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportExpression(umint Size = t_CSuper::f_Size(_pMemory));
		t_CSuper::f_FreeNoSize(_pMemory);
		DMibMemoryReportFree(this, mp_pName, _pMemory, Size, nullptr);
	}

	template <typename t_CSuper>
	auto TCMemoryManagerTracked<t_CSuper, void>::f_GetMemoryManager(void const *_pMemory) -> TCMemoryManagerTracked *
	{
		auto *pMemoryManager = t_CSuper::f_GetMemoryManager(_pMemory);
		return fg_AutoStaticCast(pMemoryManager);
	}

	//
	// With allocation info
	// ====================


	template <typename t_CSuper, typename t_CAllocationInfo>
	template <typename... tfp_CAllocator>
	TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::TCMemoryManagerTracked(ch8 const *_pName, tfp_CAllocator &&..._Params)
		: t_CSuper(fg_Forward<tfp_CAllocator>(_Params)...)
		, mp_pName(_pName)
	{
		DMibMemoryReportAllocatorName(this, _pName);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::~TCMemoryManagerTracked()
	{

	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocWithSizeInline(umint &_Size)
	{
		return f_AllocWithSize(_Size);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_Alloc(umint _Size)
	{
		return f_AllocAligned(_Size, 1);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocWithSize(umint &_Size)
	{
		return f_AllocAlignedWithSize(_Size, 1);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocWithSizeDebug(umint &_Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		return f_AllocWithSize(_Size);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocAlignedWithSizeInline(umint &_Size, umint _Alignment)
	{
		return f_AllocAlignedWithSize(_Size, _Alignment);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocAligned(umint _Size, umint _Alignment)
	{
		return f_AllocAlignedWithSize(_Size, _Alignment);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocAlignedWithSize(umint &_Size, umint _Alignment)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);

		_Alignment = fg_Max(_Alignment, sizeof(void *) * 2, alignof(CPreBlockData));

		umint NeededSize = fg_AlignUp(_Size, _Alignment) + fg_AlignUp(sizeof(CPreBlockData), _Alignment);
		uint8 *pAlloc = (uint8 *)t_CSuper::f_AllocAlignedWithSize(NeededSize, _Alignment);
		uint8 *pRet = pAlloc;
		pRet += sizeof(CPreBlockData);
		pRet = fg_AlignUp(pRet, _Alignment);

		CPreBlockData *pPreBlock = (CPreBlockData *)pRet - 1;

		pPreBlock->m_HeaderSize = pRet - pAlloc;
		pPreBlock->m_TotalSize = NeededSize;

		_Size = NeededSize - pPreBlock->m_HeaderSize;

		DMibMemoryReportAlloc(this, mp_pName, pRet, _Alignment, RequestedSize, _Size, this->f_Overhead(pAlloc) + pPreBlock->m_HeaderSize, &pPreBlock->m_AllocationInfo);
		return pRet;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor)
	{
		struct CFunctorOptions
		{
			umint m_RequestedSize;
			umint m_Alignment;
			NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const *m_pFunctor;
		};

		CFunctorOptions Options;

		_Alignment = fg_Max(_Alignment, sizeof(void *) * 2, alignof(CPreBlockData));

		Options.m_pFunctor = &_Functor;
		Options.m_RequestedSize = fg_AlignUp(_Size, _Alignment) + fg_AlignUp(sizeof(CPreBlockData), _Alignment);
		Options.m_Alignment = _Alignment;

		t_CSuper::f_AllocBatch
			(
				Options.m_RequestedSize
				, _Alignment
				, [this, &Options](void * _pAlloc, umint _Size)
				{
					DMibMemoryGoingToReportScope(this, true);

					uint8 *pAlloc = (uint8 *)_pAlloc;
					uint8 *pRet = pAlloc;
					pRet += sizeof(CPreBlockData);
					pRet = fg_AlignUp(pRet, Options.m_Alignment);

					CPreBlockData *pPreBlock = (CPreBlockData *)pRet - 1;

					pPreBlock->m_HeaderSize = pRet - pAlloc;
					pPreBlock->m_TotalSize = _Size;

					umint Size = _Size - pPreBlock->m_HeaderSize;

					DMibMemoryReportAlloc(this, mp_pName, pRet, Options.m_Alignment, Options.m_RequestedSize, Size, this->f_Overhead(_pAlloc) + pPreBlock->m_HeaderSize, &pPreBlock->m_AllocationInfo);
					return (*Options.m_pFunctor)(pRet, Size);
				}
			)
		;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		return f_AllocBatch(_Size, _Alignment, _Functor);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		return f_AllocAlignedWithSize(_Size, _Alignment);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_ReallocInline(void * _pMemory, umint &_Size, umint _OldSize)
	{
		return f_Realloc(_pMemory, _Size, _OldSize);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_Realloc(void * _pMemory, umint &_Size, umint _OldSize)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);

		t_CAllocationInfo OldInfo;
		t_CAllocationInfo *pOldInfo = nullptr;
		void *pOldMemory = nullptr;
		[[maybe_unused]] umint Size = 0;
		umint OldSize = 0;
		if (_pMemory)
		{
			CPreBlockData *pOldPreBlock = (CPreBlockData *)_pMemory - 1;
			pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_HeaderSize;
			Size = _OldSize ? t_CSuper::f_SizePadded(_OldSize) : (t_CSuper::f_Size(pOldMemory) - pOldPreBlock->m_HeaderSize);
			if (_OldSize)
				OldSize = _OldSize + pOldPreBlock->m_HeaderSize;
			OldInfo = pOldPreBlock->m_AllocationInfo;
			pOldInfo = &OldInfo;
		}

		uint8 *pRet;
		CPreBlockData *pPreBlock;
		uint8 *pAlloc;
		{
			umint Alignment = fg_Max(sizeof(void *) * 2, alignof(CPreBlockData));

			umint NeededSize = fg_AlignUp(_Size, Alignment) + fg_AlignUp(sizeof(CPreBlockData), Alignment);
			pAlloc = (uint8 *)t_CSuper::f_Realloc(pOldMemory, NeededSize, OldSize);
			pRet = pAlloc;
			pRet += sizeof(CPreBlockData);
			pRet = fg_AlignUp(pRet, Alignment);

			pPreBlock = (CPreBlockData *)pRet - 1;

			pPreBlock->m_HeaderSize = pRet - pAlloc;
			pPreBlock->m_TotalSize = NeededSize;

			_Size = NeededSize - pPreBlock->m_HeaderSize;
		}

		DMibMemoryReportRealloc(this, mp_pName, _pMemory, Size, &OldInfo, pRet, 0, RequestedSize, _Size, this->f_Overhead(pAlloc) + pPreBlock->m_HeaderSize, &pPreBlock->m_AllocationInfo);
		return pRet;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_ReallocDebug(void * _pMemory, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		return f_Realloc(_pMemory, _Size, _OldSize);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_ResizeInline(void * _pMemory, umint &_Size, umint _OldSize)
	{
		return f_Resize(_pMemory, _Size, _OldSize);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_Resize(void * _pMemory, umint &_Size, umint _OldSize)
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);

		t_CAllocationInfo OldInfo;
		t_CAllocationInfo *pOldInfo = nullptr;
		void *pOldMemory = nullptr;
		[[maybe_unused]] umint Size = 0;
		umint OldSize = 0;
		if (_pMemory)
		{
			CPreBlockData *pOldPreBlock = (CPreBlockData *)_pMemory - 1;
			pOldMemory = (uint8 *)_pMemory - pOldPreBlock->m_HeaderSize;
			Size = _OldSize ? t_CSuper::f_SizePadded(_OldSize) : (t_CSuper::f_Size(pOldMemory) - pOldPreBlock->m_HeaderSize);
			if (_OldSize)
				OldSize = _OldSize + pOldPreBlock->m_HeaderSize;
			OldInfo = pOldPreBlock->m_AllocationInfo;
			pOldInfo = &OldInfo;
		}

		uint8 *pRet;
		CPreBlockData *pPreBlock;
		uint8 *pAlloc;
		{
			umint Alignment = fg_Max(sizeof(void *) * 2, alignof(CPreBlockData));

			umint NeededSize = fg_AlignUp(_Size, Alignment) + fg_AlignUp(sizeof(CPreBlockData), Alignment);
			pAlloc = (uint8 *)t_CSuper::f_Resize(pOldMemory, NeededSize, OldSize);
			pRet = pAlloc;
			pRet += sizeof(CPreBlockData);
			pRet = fg_AlignUp(pRet, Alignment);

			pPreBlock = (CPreBlockData *)pRet - 1;

			pPreBlock->m_HeaderSize = pRet - pAlloc;
			pPreBlock->m_TotalSize = NeededSize;

			_Size = NeededSize - pPreBlock->m_HeaderSize;
		}

		DMibMemoryReportResize(this, mp_pName, _pMemory, Size, pOldInfo, pRet, 0, RequestedSize, _Size, this->f_Overhead(pAlloc) + pPreBlock->m_HeaderSize, &pPreBlock->m_AllocationInfo);
		return pRet;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void *TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_ResizeDebug(void * _pMemory, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags)
	{
		return f_Resize(_pMemory, _Size, _OldSize);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	umint TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_SizeInline(void const * _pMemory) const
	{
		return f_Size(_pMemory);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	umint TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_Size(void const * _pMemory) const
	{
		DMibMemoryGoingToReportScope(this, true);
		CPreBlockData *pPreBlock = (CPreBlockData *)_pMemory - 1;
		void *pMemory = (uint8 *)_pMemory - pPreBlock->m_HeaderSize;
		umint Ret = t_CSuper::f_Size(pMemory) - pPreBlock->m_HeaderSize;
		DMibMemoryReportGetSize(this, mp_pName, _pMemory, Ret, &pPreBlock->m_AllocationInfo);
		return Ret;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	umint TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_TrySize(void const * _pMemory) const
	{
		DMibMemoryGoingToReportScope(this, true);
		CPreBlockData *pPreBlock = (CPreBlockData *)_pMemory - 1;
		void *pMemory = (uint8 *)_pMemory - pPreBlock->m_HeaderSize;
		umint Ret = t_CSuper::f_TrySize(pMemory) - pPreBlock->m_HeaderSize;
		DMibMemoryReportGetSize(this, mp_pName, _pMemory, Ret, &pPreBlock->m_AllocationInfo);
		return Ret;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_FreeInline(void * _pMemory, umint _Size)
	{
		return f_Free(_pMemory, _Size);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_Free(void * _pMemory, umint _Size)
	{
		if (!_pMemory)
			return;
		CPreBlockData *pPreBlock = (CPreBlockData *)_pMemory - 1;
		void *pMemory = (uint8 *)_pMemory - pPreBlock->m_HeaderSize;
		t_CAllocationInfo AllocationInfo = pPreBlock->m_AllocationInfo;

		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportExpression(umint Size = f_SizePadded(_Size));
		t_CSuper::f_Free(pMemory, pPreBlock->m_TotalSize);
		DMibMemoryReportFree(this, mp_pName, _pMemory, Size, &AllocationInfo);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_FreeNoSizeInline(void * _pMemory)
	{
		return f_FreeNoSize(_pMemory);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	void TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_FreeNoSize(void * _pMemory)
	{
		if (!_pMemory)
			return;
		CPreBlockData *pPreBlock = (CPreBlockData *)_pMemory - 1;
		void *pMemory = (uint8 *)_pMemory - pPreBlock->m_HeaderSize;
		t_CAllocationInfo AllocationInfo = pPreBlock->m_AllocationInfo;

		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportExpression(umint Size = t_CSuper::f_Size(pMemory) - pPreBlock->m_HeaderSize);
		t_CSuper::f_FreeNoSize(pMemory);
		DMibMemoryReportFree(this, mp_pName, _pMemory, Size, &AllocationInfo);
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	umint TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_SizePadded(umint _Size)
	{
		umint Alignment = sizeof(void *) * 2;

		umint NeededSize = fg_AlignUp(_Size, Alignment) + fg_AlignUp(sizeof(CPreBlockData), Alignment);
		umint SizePadded = t_CSuper::f_SizePadded(NeededSize);
		umint HeaderSize = fg_AlignUp(sizeof(CPreBlockData), Alignment);

		return SizePadded - HeaderSize;
	}

	template <typename t_CSuper, typename t_CAllocationInfo>
	auto TCMemoryManagerTracked<t_CSuper, t_CAllocationInfo>::f_GetMemoryManager(void const *_pMemory) -> TCMemoryManagerTracked *
	{
		auto *pMemoryManager = t_CSuper::f_GetMemoryManager(_pMemory);
		return fg_AutoStaticCast(pMemoryManager);
	}
}
