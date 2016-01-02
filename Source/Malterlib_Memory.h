// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		
		struct CAllocatorConstructTag
		{
		};

		class CGlobalReportMemory
		{
		public:

			virtual void f_AllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName) pure;
			virtual void f_AllocatorDelete(mint _MemoryAllocator) pure;

			virtual void f_ScopeEnter(mint _MemoryAllocator) pure;
			virtual void f_ScopeExit(mint _MemoryAllocator) pure;

			virtual void f_Alloc
				(
					mint _MemoryAllocator
					, mint _Address
					, mint _RequestedAlignment
					, mint _RequestedSize
					, mint _ReturnedSize
					, fp32 _nBytesOverhead
					, void *_pAllocationInfo
				) pure
			;
			virtual void f_Resize
				(
					mint _MemoryAllocator
					, mint _OldAddress
					, mint _OldSize
					, void const *_pOldAllocationInfo
					, mint _Address
					, mint _RequestedAlignment
					, mint _RequestedSize
					, mint _ReturnedSize
					, fp32 _nBytesOverhead
					, void *_pAllocationInfo
				) pure
			;
			virtual void f_Realloc
				(
					mint _MemoryAllocator
					, mint _OldAddress
					, mint _OldSize
					, void const *_pOldAllocationInfo
					, mint _Address
					, mint _RequestedAlignment
					, mint _RequestedSize
					, mint _ReturnedSize
					, fp32 _nBytesOverhead
					, void *_pAllocationInfo
				) pure
			;
			virtual void f_Free(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo) pure;
			virtual void f_GetSize(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo) pure;
			virtual void f_Protect(mint _MemoryAllocator, mint _Address, mint _Size, uaint _Protect) pure;
			virtual void f_Commit(mint _MemoryAllocator, mint _Address, mint _Size) pure;
			virtual void f_Decommit(mint _MemoryAllocator, mint _Address, mint _Size) pure;

			virtual void f_Report(bool _bFullReport) {};
		};

		class CReportMemory
		{
		public:
			virtual void f_Alloc
				(
					mint _MemoryAllocator
					, mint _AllocatorDepth
					, ch8 const *_pAllocatorName
					, mint _Address
					, mint _RequestedAlignment
					, mint _RequestedSize
					, mint _ReturnedSize
					, fp32 _nBytesOverhead
					, void *_pAllocationInfo
				) pure
			;
			virtual void f_Resize
				(
					mint _MemoryAllocator
					, mint _AllocatorDepth
					, ch8 const *_pAllocatorName
					, mint _OldAddress
					, mint _OldSize
					, void const *_pOldAllocationInfo
					, mint _Address
					, mint _RequestedAlignment
					, mint _RequestedSize
					, mint _ReturnedSize
					, fp32 _nBytesOverhead
					, void *_pAllocationInfo
				) pure
			;
			virtual void f_Realloc
				(
					mint _MemoryAllocator
					, mint _AllocatorDepth
					, ch8 const *_pAllocatorName
					, mint _OldAddress
					, mint _OldSize
					, void const *_pOldAllocationInfo
					, mint _Address
					, mint _RequestedAlignment
					, mint _RequestedSize
					, mint _ReturnedSize
					, fp32 _nBytesOverhead
					, void *_pAllocationInfo
				) pure
			;
			virtual void f_Free(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _AllocatorDepth, mint _Address, mint _Size, void const *_pAllocationInfo) pure;
			virtual void f_GetSize(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _AllocatorDepth, mint _Address, mint _Size, void const *_pAllocationInfo) pure;
			virtual void f_Protect(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _AllocatorDepth, mint _Address, mint _Size, uaint _Protect) pure;
			virtual void f_Commit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _AllocatorDepth, mint _Address, mint _Size) pure;
			virtual void f_Decommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _AllocatorDepth, mint _Address, mint _Size) pure;
			virtual void f_AllocatorDelete(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _AllocatorDepth) pure;
		};

#	if DMibConfig_Memory_Shims_Enable

		void fg_ReportMemoryAlloc
			(
				mint _MemoryAllocator
				, ch8 const *_pAllocatorName
				, mint _Address
				, mint _RequestedAlignment
				, mint _RequestedSize
				, mint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
		;
		void fg_ReportMemoryResize
			(
				mint _MemoryAllocator
				, ch8 const *_pAllocatorName
				, mint _OldAddress
				, mint _OldSize
				, void const *_pOldAllocationInfo
				, mint _Address
				, mint _RequestedAlignment
				, mint _RequestedSize
				, mint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
		;
		void fg_ReportMemoryRealloc
			(
				mint _MemoryAllocator
				, ch8 const *_pAllocatorName
				, mint _OldAddress
				, mint _OldSize
				, void const *_pOldAllocationInfo
				, mint _Address
				, mint _RequestedAlignment
				, mint _RequestedSize
				, mint _ReturnedSize
				, fp32 _nBytesOverhead
				, void *_pAllocationInfo
			)
		;
		void fg_ReportMemoryFree(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, void const *_pAllocationInfo);

		void fg_ReportMemoryGetSize(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, void const *_pAllocationInfo);
		void fg_ReportMemoryProtect(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, uaint _Protect);
		void fg_ReportMemoryCommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size);
		void fg_ReportMemoryDecommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size);
		void fg_ReportMemoryAllocatorDelete(mint _MemoryAllocator, ch8 const *_pAllocatorName);
		void fg_ReportMemoryGoingToReportEnter(mint _MemoryAllocator);
		void fg_ReportMemoryGoingToReportExit(mint _MemoryAllocator);
		void fg_ReportMemoryAllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName);
		void fg_ReportMemoryReportStatistics(bool _bFullReport);

		CReportMemory *fg_ReportMemoryTo(CReportMemory *_pMemoryReporter);
		void fg_ReportMemoryGloballyTo(CGlobalReportMemory *_pMemoryReporter);

		class CMemoryReportScope
		{
			CReportMemory *m_pOldReporter;
		public:
			CMemoryReportScope(CReportMemory &_Reporter)
			{
				m_pOldReporter = fg_ReportMemoryTo(&_Reporter);
			}
			~CMemoryReportScope()
			{
				fg_ReportMemoryTo(m_pOldReporter);
			}
		};

		class CDisableMemoryReporterScope
		{
			CReportMemory *m_pOldReporter;
		public:
			CDisableMemoryReporterScope()
			{
				m_pOldReporter = fg_ReportMemoryTo(nullptr);
			}
			~CDisableMemoryReporterScope()
			{
				fg_ReportMemoryTo(m_pOldReporter);
			}
		};

		class CMemoryReportGoingToReportScope
		{
			mint m_Allocator;
			bint m_bReport;
		public:
			CMemoryReportGoingToReportScope(mint _Allocator, bint _bReport)
				: m_Allocator(_Allocator)
				, m_bReport(_bReport)
			{
					if (_bReport)
				fg_ReportMemoryGoingToReportEnter(_Allocator);
			}
			~CMemoryReportGoingToReportScope()
			{
				if (m_bReport)
					fg_ReportMemoryGoingToReportExit(m_Allocator);
			}
		};

#		define DMibMemoryGoingToReportScope(d_MemoryAllocator, d_bReport) CMemoryReportGoingToReportScope MalterlibMemoryReportScope((mint)d_MemoryAllocator, d_bReport);
#		define DMibMemoryReportSaveVar(d_Name, d_Value) auto d_Name = d_Value
#		define DMibMemoryReportExpression(d_Expression) d_Expression
#		define DMibMemoryReportAlloc(d_MemoryAllocator, d_AllocatorName, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
			fg_ReportMemoryAlloc((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#		define DMibMemoryReportResize(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
			fg_ReportMemoryResize((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_OldAddress, d_OldSize, d_pOldAllocationInfo, (mint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#		define DMibMemoryReportRealloc(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
			fg_ReportMemoryRealloc((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_OldAddress, d_OldSize, d_pOldAllocationInfo, (mint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#		define DMibMemoryReportFree(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo) \
			fg_ReportMemoryFree((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size, d_pAllocationInfo)
#		define DMibMemoryReportGetSize(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo) \
			fg_ReportMemoryGetSize((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size, d_pAllocationInfo)
#		define DMibMemoryReportCommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size) \
			fg_ReportMemoryCommit((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size)
#		define DMibMemoryReportProtect(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_Protect) \
			fg_ReportMemoryProtect((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size, d_Protect)
#		define DMibMemoryReportDecommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size) \
			fg_ReportMemoryDecommit((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size)
#		define DMibMemoryReportAllocatorDelete(d_MemoryAllocator, d_AllocatorName) \
			fg_ReportMemoryAllocatorDelete((mint)d_MemoryAllocator, d_AllocatorName)
#		define DMibMemoryReportAllocatorName(d_MemoryAllocator, d_AllocatorName) \
			fg_ReportMemoryAllocatorName((mint)d_MemoryAllocator, d_AllocatorName)

#	else
#		define DMibMemoryGoingToReportScope(d_MemoryAllocator, d_bReport)
#		define DMibMemoryReportSaveVar(d_Name, d_Value)
#		define DMibMemoryReportExpression(d_Expression)
#		define DMibMemoryReportAlloc(d_MemoryAllocator, d_AllocatorName, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#		define DMibMemoryReportResize(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#		define DMibMemoryReportRealloc(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#		define DMibMemoryReportFree(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo)
#		define DMibMemoryReportGetSize(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo)
#		define DMibMemoryReportProtect(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_Protect)
#		define DMibMemoryReportCommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size)
#		define DMibMemoryReportDecommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size)
#		define DMibMemoryReportAllocatorDelete(d_MemoryAllocator, d_AllocatorName)
#		define DMibMemoryReportAllocatorName(d_MemoryAllocator, d_AllocatorName)
#	endif

		class CAllocator_Empty
		{
		public:
			
			typedef CDefaultPointerHolder CPtrHolder;

			static inline_small mint f_GranularityAlloc(bint _bLargePages = false)
			{
				return 1;
			}

			static inline_small mint f_GranularityCommit(bint _bLargePages = false)
			{
				return 1;
			}

			static inline_small mint f_GranularityProtect(bint _bLargePages = false)
			{
				return 1;
			}

			static inline_small mint f_Size(void *_pBlock)
			{
				return 0;
			}

			static inline_small mint f_TrySize(void *_pBlock)
			{
				return 0;
			}

			static inline_small mint f_SizePadded(mint _Size)
			{
				return _Size;
			}
			
			static inline_small fp32 f_Overhead(void const *_pBlock) // Number of bytes overhead for block
			{
				return 0.0f;
			}

			static inline_small bint f_CanCommit()
			{
				return false;
			}

			static inline_small bint f_CanProtect()
			{
				return false;
			}

			static inline_small void f_Protect(void *_pMem, mint _Size, uaint _Protect)
			{

			}

			static inline_small void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}

			static inline_small void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}
			static inline_small void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
			}
			static inline_small void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
			}

			static inline_small void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}

			static inline_small void *f_Realloc(void *_pMem, mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}

			static inline_small void *fs_ReallocLocation(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}

			static inline_small void *f_ReallocDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}

			static inline_small void *f_Resize(void *_pMem, mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			{
				return nullptr;
			}

			static inline_small void f_Commit(void *_pMem, mint _Size)
			{
			}

			static inline_small void f_Decommit(void *_pMem, mint _Size)
			{
			}

			static inline_small void f_Free(void *_pBlock)
			{
			}
		};		

		class CAllocator_Disable
		{
		public:
			
			typedef CDefaultPointerHolder CPtrHolder;
		};


		class CAllocator_Heap;

		/************************************************************************************************\
		||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
		|| Common functions
		||______________________________________________________________________________________________||
		\************************************************************************************************/
		
#		ifdef	DMibPIntrinsicMemCopy
			template <typename t_CData1, typename t_CData2>
			inline_small t_CData1 *fg_MemCopy(t_CData1 *_pDest, const t_CData2 *_pSource, mint _Size)
			{
				return (t_CData1 *)DMibPIntrinsicMemCopy(_pDest, _pSource, _Size);
			}
#		else
			template <typename t_CData1, typename t_CData2>
			inline_medium t_CData1 *fg_MemCopy(t_CData1 *_pDest, const t_CData2 *_pSource, mint _Size)
			{
		/*		for (mint i = 0; i < _Size; ++i)
					((uint8 *)_pDest)[i] = ((uint8 *)_pSource)[i];*/

				int DoLarge = _Size / sizeof(mint);
				mint *pDest = (mint *)_pDest;
				mint *pSrc = (mint *)_pSource;
				{
					mint *pDestEnd = pDest + DoLarge;
					
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
					uint8 *pDestEnd = pDest2 + (_Size - (DoLarge * sizeof(mint)));
					
					while ((pDestEnd - pDest2))
					{
						*pDest2 = *pSrc2;
						++pDest2;
						++pSrc2;
					}
				}

				return _pDest;
			}
#		endif

		template <typename t_CData1, typename t_CData2>
		inline_medium t_CData1 *fg_ObjectCopy(t_CData1 *_pDest, const t_CData2 *_pSource, mint _nObjects)
		{
			for (mint i = 0; i < _nObjects; ++i)
				_pDest[i] = _pSource[i];
			return _pDest;
		}

		template <typename t_CData1, mint _ListSize>
		inline_medium t_CData1 *fg_ObjectCopy(t_CData1 _Dest[_ListSize], t_CData1 const _Source[_ListSize])
		{
			for (mint i = 0; i < _ListSize; ++i)
				_Dest[i] = _Source[i];
			return _Dest;
		}

		template <typename t_CData1, typename t_CData2>
		inline_medium aint fg_ObjectCmp(t_CData1 *_pObject0, const t_CData2 *_pObject1, mint _nObjects0, mint _nObjects1)
		{
			mint nMin = fg_Min(_nObjects0, _nObjects1);
			for (mint i = 0; i < nMin; ++i)
			{
				if (_pObject0[i] != _pObject1[i])
				{
					if (_pObject0[i] > _pObject1[i])
						return 1;
					else
						return -1;
				}
			}
			if (_nObjects0 > _nObjects1)
				return 1;
			else if (_nObjects0 < _nObjects1)
				return -1;
			return 0;
		}

		
		template <typename t_CData1, typename t_CData2>
		inline_medium t_CData1 *fg_ObjectSet(t_CData1 *_pDest, const t_CData2 _SetValue, mint _NumElements)
		{
			t_CData1 *pDest = _pDest;
			t_CData1 *pEnd = pDest + _NumElements;
			while (pEnd - pDest)
			{
				(*pDest++) = _SetValue;
			}

			return _pDest;
		}
		
#		ifdef	DMibPIntrinsicMemSet
		template <typename t_CData2>
		inline_always uint8 *fg_ObjectSet(uint8 *_pDest, t_CData2 _SetValue, mint _NumElements)
		{
			return (uint8 *)DMibPIntrinsicMemSet(_pDest, uint8(_SetValue), _NumElements);
		}
#		endif

		template <typename t_CData1>
		inline_large t_CData1 *fg_MemClear(t_CData1 *_pFirst, mint _Size)
		{
			int DoSize = _Size / sizeof(mint);
			fg_ObjectSet((mint *)_pFirst, 0, DoSize);
			fg_ObjectSet((uint8 *)_pFirst + (DoSize * sizeof(mint)), 0, _Size - (DoSize * sizeof(mint)));
			return _pFirst;
		}

		template <typename t_CData1>
		inline_small t_CData1 &fg_MemClear(t_CData1 &_First)
		{
			fg_MemClear(&_First, sizeof(t_CData1));
			return _First;
		}

		template <typename t_CData1, mint _nElem>
		inline_small t_CData1 *fg_MemClear(t_CData1 _Data[_nElem])
		{
			fg_MemClear(&_Data, sizeof(t_CData1) * _nElem);
			return _Data;
		}

		template <typename t_CData1, typename t_CData2>
		inline_medium aint fg_MemCmp(const t_CData1 *_pFirst, const t_CData2 *_pSecond, mint _NumElements)
		{
			const t_CData1 *pFirst = _pFirst;
			const t_CData2 *pSecond = _pSecond;
			const t_CData1 *pFirstEnd = pFirst + _NumElements;
			
			while ((pFirstEnd - pFirst))
			{
				if ((*pFirst) != (*pSecond))
					return (*pFirst) - (*pSecond);
				++pFirst;
				++pSecond;
			}

			return 0;
		}

		template <typename t_CData1, typename t_CData2>
		inline_medium aint fg_MemCmpOne(const t_CData1 *_pFirst, const t_CData2 _Second, mint _NumElements)
		{
			const t_CData1 *pFirst = _pFirst;
			const t_CData1 *pFirstEnd = pFirst + _NumElements;
			
			while ((pFirstEnd - pFirst))
			{
				if ((*pFirst) != _Second)
					return (*pFirst) - _Second;
				++pFirst;
			}

			return 0;
		}

#		ifdef	DMibPIntrinsicMemMove
			template <typename t_CData1, typename t_CData2>
			inline_small t_CData1 *fg_MemMove(t_CData1 *_pDest, const t_CData2 *_pSource, mint _Size)
			{
				return (t_CData1 *)DMibPIntrinsicMemMove(_pDest, _pSource, _Size);
			}
#		else
			template <typename t_CData1, typename t_CData2>
			inline_extralarge t_CData1 *fg_MemMove(t_CData1 *_pDest, const t_CData2 *_pSource, mint _Size)
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
#		endif

	}

	
	template <typename t_CData, typename t_CSorter>
	class TCHelpQSort
	{
		static void fp_InsertSort(t_CData *_pArray, aint _Low, aint _High, const t_CSorter &_Sorter)
		{
			t_CData Temp;
			aint i, j;

			for (i = _Low + 1; i <= _High; i++) 
			{
				Temp = _pArray[i];

				for (j = i-1; j >= _Low && _Sorter(Temp, _pArray[j]); j--)
					_pArray[j+1] = _pArray[j];

				_pArray[j+1] = Temp;
			}
		}

		static aint fp_Partition(t_CData *_pArray, aint _Low, aint _High, const t_CSorter &_Sorter) 
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
				while (i < j && _Sorter(_pArray[i], Pivot)) 
					i++;
				while (j >= i && _Sorter(Pivot, _pArray[j])) 
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
		static void fpr_QuickSort(t_CData *_pArray, aint _Low, aint _High, const t_CSorter &_Sorter) 
		{
			aint m;

			while (_Low < _High) 
			{

				if (_High - _Low <= 12) 
				{
					fp_InsertSort(_pArray, _Low, _High, _Sorter);
					return;
				}

				m = fp_Partition(_pArray, _Low, _High, _Sorter);

				if (m - _Low <= _High - m) 
				{
					fpr_QuickSort(_pArray, _Low, m - 1, _Sorter);
					_Low = m + 1;
				}
				else
				{
					fpr_QuickSort(_pArray, m + 1, _High, _Sorter);
					_High = m - 1;
				}
			}
		}
	};

	template <typename t_CType, typename t_CSorter>
	void fg_QSort(t_CType * _pToSort, mint _nNumElements, const t_CSorter &_Sorter)
	{
		TCHelpQSort<t_CType, t_CSorter>::fpr_QuickSort(_pToSort, 0, _nNumElements-1, _Sorter);
	}
}

