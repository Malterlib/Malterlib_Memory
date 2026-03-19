// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include <Mib/Atomic/Atomic>

namespace NMib::NMemory
{
	enum EMemoryManagerFeatureFlag : uint32
	{
		EMemoryManagerFeatureFlag_None = 0
		, EMemoryManagerFeatureFlag_TraceLeaks = DMibBit(0)
		, EMemoryManagerFeatureFlag_StackTrace = DMibBit(1)
		, EMemoryManagerFeatureFlag_CheckModifyAfterFree = DMibBit(2)
		, EMemoryManagerFeatureFlag_PreGuard = DMibBit(3)
		, EMemoryManagerFeatureFlag_PostGuard = DMibBit(4)
		, EMemoryManagerFeatureFlag_FreeValidation = DMibBit(5)
		, EMemoryManagerFeatureFlag_Enumeration = DMibBit(6)
		, EMemoryManagerFeatureFlag_AssertOnMemoryLeak = DMibBit(7)
	};

	class CGlobalReportMemory
	{
	public:

		virtual void f_AllocatorName(umint _MemoryAllocator, ch8 const* _pAllocatorName) = 0;
		virtual void f_AllocatorDelete(umint _MemoryAllocator) = 0;

		virtual void f_ScopeEnter(umint _MemoryAllocator) = 0;
		virtual void f_ScopeExit(umint _MemoryAllocator) = 0;

		virtual void f_Alloc
			(
				umint _MemoryAllocator
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) = 0
		;
		virtual void f_Resize
			(
				umint _MemoryAllocator
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) = 0
		;
		virtual void f_Realloc
			(
				umint _MemoryAllocator
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) = 0
		;
		virtual void f_Free(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_GetSize(umint _MemoryAllocator, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_Protect(umint _MemoryAllocator, umint _Address, umint _Size, uaint _Protect) = 0;
		virtual void f_Commit(umint _MemoryAllocator, umint _Address, umint _Size) = 0;
		virtual void f_Decommit(umint _MemoryAllocator, umint _Address, umint _Size) = 0;

		virtual void f_Report(bool _bFullReport) {};
	};

	class CReportMemory
	{
	public:
		virtual void f_Alloc
			(
				umint _MemoryAllocator
				, umint _AllocatorDepth
				, ch8 const *_pAllocatorName
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) = 0
		;
		virtual void f_Resize
			(
				umint _MemoryAllocator
				, umint _AllocatorDepth
				, ch8 const *_pAllocatorName
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) = 0
		;
		virtual void f_Realloc
			(
				umint _MemoryAllocator
				, umint _AllocatorDepth
				, ch8 const *_pAllocatorName
				, umint _OldAddress
				, umint _OldSize
				, void const *_pOldAllocationInfo
				, umint _Address
				, umint _RequestedAlignment
				, umint _RequestedSize
				, umint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			) = 0
		;
		virtual void f_Free(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_GetSize(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size, void const *_pAllocationInfo) = 0;
		virtual void f_Protect(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size, uaint _Protect) = 0;
		virtual void f_Commit(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size) = 0;
		virtual void f_Decommit(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth, umint _Address, umint _Size) = 0;
		virtual void f_AllocatorDelete(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _AllocatorDepth) = 0;
	};

	class CReportMemoryLightweight
	{
	public:
		virtual void f_Alloc(umint _ReturnedSize) = 0;
		virtual void f_Free(umint _Size) = 0;
	};

	class CAllocator_Base
	{
	public:
		enum
		{
			mc_bIsDefault = false
		};

		constexpr bool operator == (CAllocator_Base const &_Right) const noexcept = default;
	};


	struct CAllocator_AutoDestroy
	{
		void *m_pMemory;
		umint m_Size;

		CAllocator_AutoDestroy(void *_pMemory, umint _Size)
			: m_pMemory(_pMemory)
			, m_Size(_Size)
		{
		}

		CAllocator_AutoDestroy(CAllocator_AutoDestroy &&_Other)
			: m_pMemory(_Other.m_pMemory)
			, m_Size(_Other.m_Size)
		{
			_Other.m_pMemory = nullptr;
		}

		CAllocator_AutoDestroy &operator =(CAllocator_AutoDestroy &&_Other)
		{
			m_pMemory = _Other.m_pMemory;
			m_Size = _Other.m_Size;
			_Other.m_pMemory = nullptr;
			return *this;
		}

		CAllocator_AutoDestroy()
			: m_pMemory(nullptr)
		{
		}

		only_parameters_aliased malloc_like void *f_Get() const
		{
			return m_pMemory;
		}

		void f_Claim()
		{
			m_pMemory = nullptr;
		}
	};

	template <typename t_CAllocator>
	struct TCAllocator_AutoDestroyStatic : public CAllocator_AutoDestroy
	{
		TCAllocator_AutoDestroyStatic(void *_pMemory, umint _Size)
			: CAllocator_AutoDestroy(_pMemory, _Size)
		{
		}
		TCAllocator_AutoDestroyStatic(TCAllocator_AutoDestroyStatic &&) = default;
		TCAllocator_AutoDestroyStatic &operator = (TCAllocator_AutoDestroyStatic &&) = default;
		TCAllocator_AutoDestroyStatic() = default;
		~TCAllocator_AutoDestroyStatic()
		{
			if (this->m_pMemory)
				t_CAllocator::f_Free(this->m_pMemory, this->m_Size);
		}
	};

	template <typename t_CAllocator>
	struct TCAllocator_AutoDestroy : public CAllocator_AutoDestroy
	{
		TCAllocator_AutoDestroy(void *_pMemory, umint _Size, t_CAllocator &_Allocator)
			: CAllocator_AutoDestroy(_pMemory, _Size)
			, m_Allocator(_Allocator)
		{
		}
		TCAllocator_AutoDestroy(TCAllocator_AutoDestroy &&) = default;
		TCAllocator_AutoDestroy &operator = (TCAllocator_AutoDestroy &&) = default;
		TCAllocator_AutoDestroy(t_CAllocator &_Allocator)
			: m_Allocator(_Allocator)
		{
		};
		~TCAllocator_AutoDestroy()
		{
			if (this->m_pMemory)
				m_Allocator.f_Free(this->m_pMemory, this->m_Size);
		}
		t_CAllocator &m_Allocator;
	};

	class CAllocator_Empty : public CAllocator_Base
	{
	public:

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<CAllocator_Empty>;

		static inline_small umint f_GranularityAlloc(bool _bLargePages = false)
		{
			return 1;
		}

		static inline_small umint f_GranularityCommit(bool _bLargePages = false)
		{
			return 1;
		}

		static inline_small umint f_GranularityProtect(bool _bLargePages = false)
		{
			return 1;
		}

		static inline_small umint f_Size(void *_pBlock)
		{
			return 0;
		}

		static inline_small umint f_TrySize(void *_pBlock)
		{
			return 0;
		}

		static inline_small umint f_SizePadded(umint _Size)
		{
			return _Size;
		}

		static inline_small fp32 f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			return 0.0f;
		}

		constexpr static inline_small bool f_CanCommit()
		{
			return false;
		}

		static inline_small bool f_CanProtect()
		{
			return false;
		}

		static inline_small bool f_DeterministicSize()
		{
			return true;
		}

		static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect)
		{

		}

		static inline_small void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_AllocAlignedDebug(umint _Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
		}

		static inline_small void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
		}

		static inline_small void *f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}
		static inline_small void *f_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_Realloc(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *fs_ReallocLocation(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_ReallocDebug(void *_pMem, umint _Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void *f_Resize(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return nullptr;
		}

		static inline_small void f_Commit(void *_pMem, umint _Size)
		{
		}

		static inline_small void f_Decommit(void *_pMem, umint _Size)
		{
		}

		static inline_small void f_Free(void *_pBlock, umint _Size)
		{
		}

		static inline_small void f_FreeNoSize(void *_pBlock)
		{
		}

		only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return {};
		}
		only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			return {};
		}

		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size)
		{
			return CAutoDestroy{};
		}
	};

	class CAllocator_Disable : public CAllocator_Base
	{
	public:
	};

	class CAllocator_Heap;

	/************************************************************************************************\
	||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
	|| Common functions
	||______________________________________________________________________________________________||
	\************************************************************************************************/

#	ifdef	DMibPIntrinsicMemCopy
		template <typename t_CData1, typename t_CData2>
		inline_small t_CData1 *fg_MemCopy(t_CData1 *_pDest, const t_CData2 *_pSource, umint _Size) noexcept
		{
			return (t_CData1 *)DMibPIntrinsicMemCopy(_pDest, _pSource, _Size);
		}
#	else
		template <typename t_CData1, typename t_CData2>
		inline_medium t_CData1 *fg_MemCopy(t_CData1 *_pDest, const t_CData2 *_pSource, umint _Size) noexcept
		{
	/*		for (umint i = 0; i < _Size; ++i)
				((uint8 *)_pDest)[i] = ((uint8 *)_pSource)[i];*/

			int DoLarge = _Size / sizeof(umint);
			umint *pDest = (umint *)_pDest;
			umint *pSrc = (umint *)_pSource;
			{
				umint *pDestEnd = pDest + DoLarge;

				while ((pDestEnd - pDest))
				{
					*pDest = *pSrc;
					++pDest;
					++pSrc;
				}
			}

			{
				uint8 *pDest2 = (uint8 *)pDest;
				uint8 *pSrc2 = (uint8 *)pSrc;
				uint8 *pDestEnd = pDest2 + (_Size - (DoLarge * sizeof(umint)));

				while ((pDestEnd - pDest2))
				{
					*pDest2 = *pSrc2;
					++pDest2;
					++pSrc2;
				}
			}

			return _pDest;
		}
#	endif

	template <typename t_CData1, typename t_CData2>
	inline_medium t_CData1 *fg_ObjectCopy(t_CData1 *_pDest, t_CData2 const *_pSource, umint _nObjects) noexcept(noexcept(fg_GetType<t_CData1 &>() = fg_GetType<t_CData2 const &>()))
	{
		for (umint i = 0; i < _nObjects; ++i)
			_pDest[i] = _pSource[i];
		return _pDest;
	}

	template <typename t_CData1, umint _ListSize>
	inline_medium t_CData1 *fg_ObjectCopy(t_CData1 _Dest[_ListSize], t_CData1 const _Source[_ListSize]) noexcept(noexcept(fg_GetType<t_CData1 &>() = fg_GetType<t_CData1 const &>()))
	{
		for (umint i = 0; i < _ListSize; ++i)
			_Dest[i] = _Source[i];
		return _Dest;
	}

	template <typename t_CData1, typename t_CData2>
	inline_medium t_CData1 *fg_ObjectSet(t_CData1 *_pDest, t_CData2 const _SetValue, umint _NumElements) noexcept(noexcept(fg_GetType<t_CData1 &>() = fg_GetType<t_CData2 const &>()))
	{
		t_CData1 *pDest = _pDest;
		t_CData1 *pEnd = pDest + _NumElements;
		while (pEnd - pDest)
			(*pDest++) = _SetValue;

		return _pDest;
	}

#	ifdef	DMibPIntrinsicMemSet
	template <typename t_CData2>
	inline_always uint8 *fg_ObjectSet(uint8 *_pDest, t_CData2 _SetValue, umint _NumElements) noexcept
	{
		return (uint8 *)DMibPIntrinsicMemSet(_pDest, uint8(_SetValue), _NumElements);
	}
#	endif

#ifdef	DMibPIntrinsicMemSet
	template <typename t_CData1>
	inline_always t_CData1 *fg_MemClear(t_CData1 *_pFirst, umint _Size) noexcept
	{
		DMibPIntrinsicMemSet((uint8 *)_pFirst, uint8(0), _Size);
		return _pFirst;
	}
#else
	template <typename t_CData1>
	inline_large t_CData1 *fg_MemClear(t_CData1 *_pFirst, umint _Size) noexcept
	{
		umint DoSize = _Size / sizeof(umint);
		fg_ObjectSet((umint *)_pFirst, 0, DoSize);
		fg_ObjectSet((uint8 *)_pFirst + (DoSize * sizeof(umint)), 0, _Size - (DoSize * sizeof(umint)));
		return _pFirst;
	}
#endif

	template <typename t_CData1>
	inline_always t_CData1 &fg_MemClear(t_CData1 &_First) noexcept
	{
		fg_MemClear(&_First, sizeof(t_CData1));
		return _First;
	}

	template <typename t_CData1, umint _nElem>
	inline_always t_CData1 *fg_MemClear(t_CData1 _Data[_nElem]) noexcept
	{
		fg_MemClear(&_Data, sizeof(t_CData1) * _nElem);
		return _Data;
	}

#	ifdef	DMibPIntrinsicMemSet
#		ifdef DCompiler_MSVC
			template <typename t_CData1>
			inline_always t_CData1 *fg_SecureMemClear(t_CData1 * volatile _pFirst, umint _Size) noexcept
			{
				DMibPIntrinsicMemSet((uint8 *)_pFirst, uint8(0), _Size);
				NAtomic::fg_CompilerFence();
				return _pFirst;
			}
#		else
			template <typename t_CData1>
			inline_always t_CData1 *fg_SecureMemClear(t_CData1 *_pFirst, umint _Size) noexcept
			{
				DMibPIntrinsicMemSet((uint8 *)_pFirst, uint8(0), _Size);
				__asm__ __volatile__("" ::"r"(_pFirst): "memory");
				return _pFirst;
			}
#		endif
#	else
		template <typename t_CData1>
		inline_large t_CData1 *fg_SecureMemClear(t_CData1 * volatile _pFirst, umint _Size) noexcept
		{
			umint DoSize = _Size / sizeof(umint);
			fg_ObjectSet((umint *)_pFirst, 0, DoSize);
			fg_ObjectSet((uint8 *)_pFirst + (DoSize * sizeof(umint)), 0, _Size - (DoSize * sizeof(umint)));
			NAtomic::fg_CompilerFence();
			return _pFirst;
		}
#	endif

	template <typename t_CData1>
	inline_always t_CData1 &fg_SecureMemClear(t_CData1 &_First) noexcept
	{
		fg_SecureMemClear(&_First, sizeof(t_CData1));
		return _First;
	}

	template <typename t_CData1, umint _nElem>
	inline_always t_CData1 *fg_SecureMemClear(t_CData1 _Data[_nElem]) noexcept
	{
		fg_SecureMemClear(&_Data, sizeof(t_CData1) * _nElem);
		return _Data;
	}


#ifdef DMibPIntrinsicMemCmp
	static inline_always aint fg_MemCmp(uint8 const *_pFirst, uint8 const *_pSecond, umint _Size) noexcept
	{
		return DMibPIntrinsicMemCmp(_pFirst, _pSecond, _Size);
	}
#else
	static inline_medium aint fg_MemCmp(uint8 const *_pFirst, uint8 const *_pSecond, umint _Size) noexcept
	{
		uint8 const *pFirst = _pFirst;
		uint8 const *pSecond = _pSecond;
		uint8 const *pFirstEnd = pFirst + _Size;

		while ((pFirstEnd - pFirst))
		{
			if ((*pFirst) != (*pSecond))
				return (*pFirst) - (*pSecond);
			++pFirst;
			++pSecond;
		}

		return 0;
	}
#endif

	static inline_medium aint fg_MemCmpOne(uint8 const *_pFirst, const uint8 _Second, umint _Size) noexcept
	{
		uint8 const *pFirst = _pFirst;
		uint8 const *pFirstEnd = pFirst + _Size;

		while ((pFirstEnd - pFirst))
		{
			if ((*pFirst) != _Second)
				return (*pFirst) - _Second;
			++pFirst;
		}

		return 0;
	}

#	ifdef	DMibPIntrinsicMemMove
		template <typename t_CData1, typename t_CData2>
		inline_small t_CData1 *fg_MemMove(t_CData1 *_pDest, const t_CData2 *_pSource, umint _Size) noexcept
		{
			return (t_CData1 *)DMibPIntrinsicMemMove(_pDest, _pSource, _Size);
		}
#	else
		template <typename t_CData1, typename t_CData2>
		inline_extralarge t_CData1 *fg_MemMove(t_CData1 *_pDest, const t_CData2 *_pSource, umint _Size) noexcept
		{
			uint8 *pSrc = (uint8 *)_pSource;
			uint8 *pDest = (uint8 *)_pDest;

			if (pDest > (pSrc + _Size) || pDest < pSrc)
			{
				uint8 *pSrcEnd = pSrc + _Size;
				while (pSrcEnd - pSrc)
				{
					*pDest = *pSrc;
					++pDest;
					++pSrc;
				}
			}
			else
			{
				pSrc = (uint8 *)_pSource + _Size - 1;
				pDest = (uint8 *)_pDest + _Size - 1;
				uint8 *pSrcEnd = (uint8 *)_pSource - 1;
				while (pSrcEnd - pSrc)
				{
					*pDest = *pSrc;
					--pDest;
					--pSrc;
				}
			}

			return _pDest;
		}
#	endif

	namespace NPrivate
	{
		template <typename t_CData, typename t_CSorter>
		class TCHelpQSort
		{
			static void fp_InsertSort(t_CData *_pArray, aint _Low, aint _High, const t_CSorter &_fCompare)
			{
				t_CData Temp;
				aint i, j;

				for (i = _Low + 1; i <= _High; i++)
				{
					Temp = _pArray[i];

					for (j = i-1; j >= _Low && fg_CheckOrdering(_fCompare(Temp, _pArray[j])) < 0; j--)
						_pArray[j+1] = _pArray[j];

					_pArray[j+1] = Temp;
				}
			}

			static aint fp_Partition(t_CData *_pArray, aint _Low, aint _High, const t_CSorter &_fCompare)
			{
				t_CData Temp, Pivot;
				aint i, j, p;

				p = _Low + ((_High - _Low)>>1);
				Pivot = _pArray[p];
				_pArray[p] = _pArray[_Low];

				i = _Low+1;
				j = _High;
				while (1)
				{
					while (i < j && fg_CheckOrdering(_fCompare(_pArray[i], Pivot)) < 0)
						i++;
					while (j >= i && fg_CheckOrdering(_fCompare(Pivot, _pArray[j])) < 0)
						j--;
					if (i >= j)
						break;
					Temp = _pArray[i];
					_pArray[i] = _pArray[j];
					_pArray[j] = Temp;
					j--; i++;
				}

				_pArray[_Low] = _pArray[j];
				_pArray[j] = Pivot;

				return j;
			}

		public:
			static void fpr_QuickSort(t_CData *_pArray, aint _Low, aint _High, const t_CSorter &_fCompare)
			{
				aint m;

				while (_Low < _High)
				{

					if (_High - _Low <= 12)
					{
						fp_InsertSort(_pArray, _Low, _High, _fCompare);
						return;
					}

					m = fp_Partition(_pArray, _Low, _High, _fCompare);

					if (m - _Low <= _High - m)
					{
						fpr_QuickSort(_pArray, _Low, m - 1, _fCompare);
						_Low = m + 1;
					}
					else
					{
						fpr_QuickSort(_pArray, m + 1, _High, _fCompare);
						_High = m - 1;
					}
				}
			}
		};
	}

	template <typename tf_CType, typename tf_CCompare>
	void fg_QSort(tf_CType *_pToSort, umint _nNumElements, const tf_CCompare &_fCompare)
	{
		NPrivate::TCHelpQSort<tf_CType, tf_CCompare>::fpr_QuickSort(_pToSort, 0, _nNumElements-1, _fCompare);
	}
}

#ifndef DMibPNoShortCuts
	using namespace NMib::NMemory;
#endif
