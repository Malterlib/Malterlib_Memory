// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "../../Core/Source/Malterlib_Core_PlatformInterface.h"

namespace NMib
{
	namespace NMem
	{
		template <mint t_MaxStackTraceDepth>
		class TCHeapDebugData_Fast
		{
		private:
			const ch8 *m_pFile;
			aint m_Line;
			EHeapDebugFlag m_Flags;
			mint m_ThreadID;
//				CStackTraceInfo *m_plStackTrace[EMaxStackTraceDepth];
			CMibCodeAddress m_lStackTrace[t_MaxStackTraceDepth];
		public:

			void f_SetFile(const ch8 *_pFile)
			{
				m_pFile = _pFile;
			}
			const ch8 *f_GetFile()
			{
				if (m_pFile)
					return m_pFile;
				return "";
			}

			void f_SetLine(aint _Line)
			{
				m_Line = _Line;
			}
			aint f_GetLine()
			{
				return m_Line;
			}

			void f_SetThreadID(mint _ThreadID)
			{
				m_ThreadID = _ThreadID;
			}
			mint f_GetThreadID()
			{
				return m_ThreadID;
			}

			void f_SetFlags(EHeapDebugFlag _Flags)
			{
				m_Flags = _Flags;
			}
			aint f_GetFlags()
			{
				return m_Flags;
			}

			void f_AquireStackTrace(CMibCodeAddress *_lStack, mint _nStack)
			{
				NMem::fg_MemCopy(m_lStackTrace, _lStack, _nStack * sizeof(CMibCodeAddress));
			}

			void f_ReleaseStackTrace()
			{
			}

			CStackTraceInfo *f_GetStackTraceInfo(mint _iDepth)
			{
				return nullptr;
			}

			CMibCodeAddress f_GetStackTraceAddress(mint _iDepth)
			{
				return m_lStackTrace[_iDepth];
			}

			TCHeapDebugData_Fast()
			{
				m_pFile = nullptr;
				m_Line = 0;
				m_Flags = EHeapDebugFlag_None;
				NMem::fg_ObjectSet(m_lStackTrace, nullptr, t_MaxStackTraceDepth);
			}
		};

		template <mint t_MaxStackTraceDepth>
		class TCHeapDebugData_Safe
		{
		private:
			const ch8 * m_pFile;
			aint m_Line;
			EHeapDebugFlag m_Flags;
			mint m_ThreadID;
			aint m_StackTraceDepth;
			CMibCodeAddress m_lStackTrace[t_MaxStackTraceDepth];
			CStackTraceInfo *m_plStackTrace[t_MaxStackTraceDepth];
		public:

			void f_SetFile(const ch8 *_pFile)
			{
				m_pFile = _pFile;
			}
			const ch8 *f_GetFile()
			{
				if (m_pFile)
					return m_pFile;
				return "";
			}

			void f_SetLine(aint _Line)
			{
				m_Line = _Line;
			}
			aint f_GetLine()
			{
				return m_Line;
			}

			void f_SetThreadID(mint _ThreadID)
			{
				m_ThreadID = _ThreadID;
			}
			mint f_GetThreadID()
			{
				return m_ThreadID;
			}



			void f_SetFlags(EHeapDebugFlag _Flags)
			{
				m_Flags = _Flags;
			}
			aint f_GetFlags()
			{
				return m_Flags;
			}

			void f_AquireStackTrace(CMibCodeAddress *_lStack, mint _nStack)
			{
				m_StackTraceDepth = _nStack;
				for (mint i = 0; i < _nStack; ++i)
				{
					m_plStackTrace[i] = NSys::fg_Debug_AquireStackTraceInfo(_lStack[i]);
					m_lStackTrace[i] = _lStack[i];
				}
			}

			void f_ReleaseStackTrace()
			{
				for (int i = 0; i < m_StackTraceDepth; ++i)
				{
					if (m_plStackTrace[i])
						NSys::fg_Debug_ReleaseStackTraceInfo(m_plStackTrace[i]);
				}
			}

			CStackTraceInfo *f_GetStackTraceInfo(mint _iDepth)
			{
				return m_plStackTrace[_iDepth];
			}

			CMibCodeAddress f_GetStackTraceAddress(mint _iDepth)
			{
				return m_lStackTrace[_iDepth];
			}


			TCHeapDebugData_Safe()
			{
				m_StackTraceDepth = 0;
				m_Line = 0;
				m_Flags = 0;
				fg_ObjectSet(m_lStackTrace, 0, t_MaxStackTraceDepth);
				fg_ObjectSet(m_plStackTrace, (CStackTraceInfo *)0, t_MaxStackTraceDepth);
			}
		};

		extern bint g_MalterlibMemoryManager_Debug_EnableStackTrace;

		template <class t_CParams16Bit = TCHeapParamsDebug<>, class t_CParamArchSize = t_CParams16Bit, mint t_MaxHeapTraceDepth = 64, class t_CDebugData = TCHeapDebugData_Fast<t_MaxHeapTraceDepth> >
		class TCHeap_CombinedDebug : public t_CParams16Bit::CLock
		{
#		if DMibConfig_Memory_Shims_Enable
			ch8 const *m_pDebugName;
#		endif
		public:
			typedef typename t_CParams16Bit::CAllocator CAllocator;

			typename TCHeapChunk<CAllocator>::CTree m_ChunksTree;
			enum
			{
				EMaxStackTraceDepth = t_MaxHeapTraceDepth,

			};

			bint m_bCheckHeap;
			mint m_DbgCounter;

			class CParam16Bit : public t_CParams16Bit
			{
			public:
				const static EAllocationFlag mc_AllocationFlags = EAllocationFlag_LocationDown;
				enum 
				{
					EExtraSpace = sizeof(t_CDebugData)
					,EGuardPreSize = sizeof(mint) * 2
					,EGuardPostSize = sizeof(mint) * 2
				};
				typedef CHeapBlock16Bit CBlock;
			};
			class CParamArchSize : public t_CParamArchSize
			{
			public:
				const static EAllocationFlag mc_AllocationFlags = EAllocationFlag_LocationUp;
				enum 
				{
					EExtraSpace = sizeof(t_CDebugData)
					,EGuardPreSize = sizeof(mint) * 2
					,EGuardPostSize = sizeof(mint) * 2
//					,EBlockCacheSize = 0
				};
				typedef CHeapBlockArchSize CBlock;
			};

			TCHeap<CParam16Bit> m_Heap_16Bit;
			TCHeap<CParamArchSize> m_Heap_ArchSize;

			class CAllocatedBlock
			{
			public:
				mint m_Address;
				class CCompare
				{
				public:
					inline_small mint operator () (CAllocatedBlock const &_Node) const
					{
						return _Node.m_Address;
					}
				};

				DMibIntrusiveLinkT(CAllocatedBlock, NIntrusive::TCAVLLink<NIntrusive::EAVLLinkType_Unaligned>, m_TreeLink);
			};

			NIntrusive::TCAVLTree<typename CAllocatedBlock::CLinkTraits_m_TreeLink, typename CAllocatedBlock::CCompare> m_AllocatedBlocks;
			NMem::TCPool<CAllocatedBlock, 128, NThread::CNoLock, CPoolType_Freeable, CAllocator> m_AllocatedBlocksPool;

			only_parameters_aliased return_not_aliased inline_small void *fp_Alloc(mint &_Size);
			only_parameters_aliased void fp_Free(void *_pMemory);
			void fp_TraceDebugDataInfo(t_CDebugData *_pDebugData, void *_pMem);
			only_parameters_aliased return_not_aliased inline_small void *fp_AllocInternal(mint &_Size, t_CDebugData *_pData);
			void fp_StackTraceAquire(t_CDebugData *_pDebugData);
			void fp_StackTraceRelease(t_CDebugData *_pDebugData);
			void fp_AllocateBlock(mint _Block);
			void fp_FreeBlock(mint _Block);
			bint fp_IsAllocated(const void *_pBlock);
			void *fp_ResetDebugData16(void *_pMem, t_CDebugData &_Data);
			void *fp_ResetDebugDataArch(void *_pMem, t_CDebugData &_Data);
			only_parameters_aliased inline_small fp32 fp_Overhead(void const *_pMemory);
			inline_small bint fp_CheckHeap(bint _bBreak);
			only_parameters_aliased inline_small mint fp_Size(const void *_pMemory);
		public:
			TCHeap_CombinedDebug(ch8 const *_pName = "Undefined TCHeap_CombinedDebug");
			~TCHeap_CombinedDebug();

			void f_Clear();
			mint f_MaxGranularity();

			only_parameters_aliased return_not_aliased inline_small void *f_Alloc(const mint &_Size);
			only_parameters_aliased return_not_aliased inline_small void *f_Alloc(mint &_Size);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocAligned(mint &_Size, mint _Align);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocAlignedDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
			only_parameters_aliased void f_Free(void *_pMemory);
			only_parameters_aliased inline_small bint f_ContainsBlock(void *_pMemory);
			only_parameters_aliased inline_small mint f_Size(const void *_pMemory);
			only_parameters_aliased inline_small fp32 f_Overhead(void const *_pMemory);
			only_parameters_aliased return_not_aliased void *f_Realloc(void *_pMemory, mint &_NewSize);
			only_parameters_aliased void *f_Resize(void *_pMemory, mint &_NewSize);
			inline_small bint f_CheckHeap(bint _bBreak);
			template <typename t_CReportFunc> // ReportFunc = void (t_CDebugData* _pDebugData, void* _pMem)
			void f_TraceAllocated(t_CReportFunc const &_ReportFunc, bint _bAllowIgnore = true, bool _bFreeBlocks = true);
			void f_TraceLeaks(bool _bFreeBlocks);

			using t_CParams16Bit::CLock::f_Lock;
			using t_CParams16Bit::CLock::f_Unlock;
			
		};
	}
}

