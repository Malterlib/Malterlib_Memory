// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{

		extern char const* gc_IgnoreFunctions[];

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_TraceDebugDataInfo(t_CDebugData *_pDebugData, void *_pMem)
		{
			t_CDebugData *pDebugData = _pDebugData;

			// Do this now, rather than when reporting the stacktrace, so we can see if we should ignore this report.
			int LongestSource = 0; 
			int LongestModule = 0; 
			CStackTraceInfo *plStackTraceInfos[EMaxStackTraceDepth];
			{
				for (int i = 0; i < EMaxStackTraceDepth; ++i)
				{
					CStackTraceInfo *pTraceInfo = pDebugData->f_GetStackTraceInfo(i);
					if (!pTraceInfo)
					{
						CMibCodeAddress Address = pDebugData->f_GetStackTraceAddress(i);
						if (Address)
						{
							pTraceInfo = plStackTraceInfos[i] = NSys::fg_Debug_AquireStackTraceInfo(Address);
						}
						else
							plStackTraceInfos[i] = nullptr;
					}
					else
						plStackTraceInfos[i] = nullptr;

					if (pTraceInfo)
					{
						if (pTraceInfo->m_pFunctionName)
						{
							for (char const** pCurIgnoreFunction = gc_IgnoreFunctions; *pCurIgnoreFunction; ++pCurIgnoreFunction)
							{
								if (NStr::fg_StrStartsWith(pTraceInfo->m_pFunctionName, *pCurIgnoreFunction))
								{
									// Free Any allocated infos here
									for (int j = 0; j <= i; ++j)
									{
										if (plStackTraceInfos[j])
											NSys::fg_Debug_ReleaseStackTraceInfo(plStackTraceInfos[j]);
									}
									return; // Ignore this report.
								}
							}
						}

						if (pTraceInfo->m_pSourceFileName)
						{
							int Len = NStr::fg_StrLen(pTraceInfo->m_pSourceFileName) + 8;
							if (Len > LongestSource)
								LongestSource = Len;
						}
						if (pTraceInfo->m_pModuleName)
						{
							int Len = NStr::fg_StrLen(pTraceInfo->m_pModuleName) + 5;
							if (Len > LongestModule)
								LongestModule = Len;
						}

					}						

				}
			}

			const ch8 *pFile = pDebugData->f_GetFile();
			if (!pFile)
				pFile = "";

			void *pCurrentBlock = _pMem;
			typedef NStr::TCFStr<ch8, 1024, NStr::EStrType_UTF>::CType CFStr;
			NStr::CFStr256 TempStr;
			TempStr = NStr::CFStr256::CFormat(DMibPFileLineFormat " ") << pFile << pDebugData->f_GetLine();

//				int Len = 96 - TempStr.f_GetLen();
//				while (Len > 0) 
//				{
//					TempStr.f_AddChar(' ');
//					--Len;
//				}

			mint BlockSize = fp_Size(pCurrentBlock);
			TempStr += (NStr::CFStr256::CFormat("Memory Leak, Flags = 0x{sj*4,sf0,nh}, Address = 0x{sj*4,sf0,nh}, Size = 0x{sj*4,sf0,nh}, Thread = {}" DMibNewLine)
				<< pDebugData->f_GetFlags()
				<< (mint)pCurrentBlock
				<< BlockSize
				<< pDebugData->f_GetThreadID()
				<< sizeof(void *)*2
				).f_GetStr();

			NSys::fg_DebugOutput(TempStr.f_GetStr());
			int nBytes = fg_Min(BlockSize, (mint)128);
			NSys::fg_DebugOutput((NStr::CFStr256::CFormat("Displaying first {} bytes of block" DMibNewLine) << nBytes).f_GetStr().f_GetStr());

			NStr::CFStr256 TraceString;
			NStr::CFStr256 TraceString2;
			for (int i = 0; i < nBytes; ++i)
			{
				TraceString += (NStr::CFStr256::CFormat("{sj2,sf0,nh}") << ((uint8 *)pCurrentBlock)[i]).f_GetStr();
				if (((uint8 *)pCurrentBlock)[i] > 31)
					TraceString2.f_AddChar(((uint8 *)pCurrentBlock)[i]);
				else
					TraceString2.f_AddChar('$');
			}

			TraceString += DMibNewLine;
			TraceString2 += DMibNewLine;
			NSys::fg_DebugOutput(TraceString.f_GetStr());
			NSys::fg_DebugOutput(TraceString2.f_GetStr());

			NSys::fg_DebugOutput("StackTrace:" DMibNewLine);

			for (int i = 0; i < EMaxStackTraceDepth; ++i)
			{
				CStackTraceInfo *pTraceInfo = pDebugData->f_GetStackTraceInfo(i);
				if (!pTraceInfo)
					pTraceInfo = plStackTraceInfos[i];

				void *pAddress = (void *)pDebugData->f_GetStackTraceAddress(i);
				if (pTraceInfo)
				{
					CFStr TempStr;
					TempStr
						= CFStr::CFormat("{}({})")
						<< pTraceInfo->m_pSourceFileName
						<< pTraceInfo->m_SourceLine
					;

					TempStr.f_AddChars(' ', LongestSource - TempStr.f_GetLen());

					TempStr
						+= CFStr::CFormat(": {}")
						<< pTraceInfo->m_pModuleName
					;
					TempStr.f_AddChars(' ', (LongestSource + LongestModule) - TempStr.f_GetLen());

					TempStr
						+= CFStr::CFormat(" \"{}\" 0x{}" DMibNewLine)
						<< pTraceInfo->m_pFunctionName
						<< pAddress
					;

					NSys::fg_DebugOutput(TempStr.f_GetStr());
				}
				else if (pAddress)
				{
					CFStr TempStr;

					TempStr.f_AddChars(' ', LongestSource - TempStr.f_GetLen());

					TempStr.f_AddChars(' ', (LongestSource + LongestModule) - TempStr.f_GetLen());

					TempStr
						+= CFStr::CFormat(" 0x{}" DMibNewLine)
						<< pAddress
					;

					NSys::fg_DebugOutput(TempStr.f_GetStr());
				}
				if (plStackTraceInfos[i])
					NSys::fg_Debug_ReleaseStackTraceInfo(plStackTraceInfos[i]);
			}
			NSys::fg_DebugOutput(DMibNewLine);
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::TCHeap_CombinedDebug(ch8 const *_pName)
#		if DMibConfig_Memory_Shims_Enable
			: m_pDebugName(_pName)
			, m_DbgCounter(0)
#		else
			: m_DbgCounter(0)
#		endif
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAllocatorName(this, m_pDebugName);
#endif
			m_bCheckHeap = false;
			m_Heap_16Bit.f_Construct(&m_ChunksTree);				
			m_Heap_ArchSize.f_Construct(&m_ChunksTree);				
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::~TCHeap_CombinedDebug()
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			f_Clear();
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAllocatorDelete(this, m_pDebugName);
#endif
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_AllocateBlock(mint _Block)
		{
			if (m_AllocatedBlocks.f_FindEqual(_Block))
			{
				DMibPDebugBreak; // Block is already allocated, heap is corrupt
			}
			CAllocatedBlock *pBlock = m_AllocatedBlocksPool.f_New();
			pBlock->m_Address = _Block;
			m_AllocatedBlocks.f_Insert(pBlock);
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_FreeBlock(mint _Block)
		{
			CAllocatedBlock *pBlock = m_AllocatedBlocks.f_FindEqual(_Block);
			if (!pBlock)
			{
				DMibPDebugBreak; // Block is already free or invalid. Don't free the same block twice
			}

			m_AllocatedBlocks.f_Remove(pBlock);
			m_AllocatedBlocksPool.f_Delete(pBlock);
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased bint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_IsAllocated(const void *_pBlock)
		{
			mint ToFind = (mint)_pBlock;
			return m_AllocatedBlocks.f_FindEqual(ToFind) != nullptr;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Clear()
		{
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);
			CAllocatedBlock *pAllocatedBlock = m_AllocatedBlocks.f_GetRoot();
				
			while (pAllocatedBlock)
			{
				m_AllocatedBlocks.f_Remove(pAllocatedBlock);
				m_AllocatedBlocksPool.f_Delete(pAllocatedBlock);
				pAllocatedBlock = m_AllocatedBlocks.f_GetRoot();
			}
			m_Heap_16Bit.f_Clear();
			m_Heap_ArchSize.f_Clear();
//				DMibTreeAVLAllocator_Iterator_FromTemplate(TCHeapChunk<CAllocator>, m_ChunkTree, typename TCHeapChunk<CAllocator>::CCompareAVL, CAllocator) Iter = m_ChunksTree;
//
//				while (Iter)
//				{
//					Iter->f_Destroy(m_ChunksTree);
//					Iter = m_ChunksTree;
//				}
			while (m_ChunksTree.f_GetRoot())
			{
				m_ChunksTree.f_GetRoot()->f_Destroy(m_ChunksTree);
			}
		}
			
		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Alloc(const mint &_Size)
		{
			mint Size = _Size;
			void *pMem = f_Alloc(Size);
			return pMem;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_Alloc(mint &_Size)
		{
			void *pRet = fp_AllocInternal(_Size, nullptr);
			return pRet;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Alloc(mint &_Size)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pRet;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				pRet = fp_Alloc(_Size);
				DMibMemoryReportExpression(Overhead = fp_Overhead(pRet));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pRet, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pRet;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_AllocInternal(mint &_Size, t_CDebugData *_pData)
		{
			if (m_bCheckHeap)
				fp_CheckHeap(true);
			t_CDebugData *pDebugData;
			void *pMemory;
			if (_Size <= m_Heap_16Bit.f_LargestBlock())
			{
				pMemory = m_Heap_16Bit.f_Alloc(_Size);
				pDebugData = (t_CDebugData *)m_Heap_16Bit.f_GetExtraData(pMemory);
			}
			else if (_Size <= m_Heap_ArchSize.f_LargestBlock())
			{
				pMemory = m_Heap_ArchSize.f_Alloc(_Size);
				pDebugData = (t_CDebugData *)m_Heap_ArchSize.f_GetExtraData(pMemory);
			}
			else
			{
				DMibErrorMemory("Memory of size requested is bigger than heap supports");
			}

			if (!pMemory)
				return nullptr;

			if (_pData)
			{
				*pDebugData = *_pData;
			}
			else
			{
				new(pDebugData) t_CDebugData();
				fp_StackTraceAquire(pDebugData);
			}

			fp_AllocateBlock((mint)pMemory);
			if (m_bCheckHeap)
				fp_CheckHeap(true);
			return pMemory;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_StackTraceAquire(t_CDebugData *_pDebugData)
		{
			if (g_MalterlibMemoryManager_Debug_EnableStackTrace)
			{
				CMibCodeAddress lStack[EMaxStackTraceDepth];
				mint nStack = NSys::fg_System_GetStackTrace(lStack, EMaxStackTraceDepth);

				_pDebugData->f_AquireStackTrace(lStack, nStack);
			}
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_StackTraceRelease(t_CDebugData *_pDebugData)
		{
			_pDebugData->f_ReleaseStackTrace();
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		mint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_MaxGranularity()
		{
			mint MaxGran = 0;
			MaxGran = fg_Max(m_Heap_16Bit.f_MaxGranularity(), MaxGran);
			MaxGran = fg_Max(m_Heap_ArchSize.f_MaxGranularity(), MaxGran);
			return MaxGran;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			t_CDebugData DebugData;
			DebugData.f_SetFile(_pFile);
			DebugData.f_SetLine(_Line);
			DebugData.f_SetThreadID(NSys::fg_Thread_GetCurrentUID());
			DebugData.f_SetFlags(_Flags);
			fp_StackTraceAquire(&DebugData);
			void *pRet;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				pRet = fp_AllocInternal(_Size, &DebugData);
				DMibMemoryReportExpression(Overhead = fp_Overhead(pRet));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pRet, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pRet;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_AllocAligned(mint &_Size, mint _Align)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pMemory;			
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				if (m_bCheckHeap)
					fp_CheckHeap(true);
				t_CDebugData *pDebugData;
				_Size = fg_AlignUp(_Size, _Align);
				if ((_Size + _Align) <= m_Heap_16Bit.f_LargestBlock())
				{
					pMemory = m_Heap_16Bit.f_AllocAligned(_Size, _Align);
					pDebugData = (t_CDebugData *)m_Heap_16Bit.f_GetExtraData(pMemory);
				}
				else if ((_Size + _Align) <= m_Heap_ArchSize.f_LargestBlock())
				{
					pMemory = m_Heap_ArchSize.f_AllocAligned(_Size, _Align);
					pDebugData = (t_CDebugData *)m_Heap_ArchSize.f_GetExtraData(pMemory);
				}
				else
				{
					DMibErrorMemory("Memory of size requested is bigger than heap supports");
				}

				pDebugData->f_SetFile(nullptr);
				pDebugData->f_SetLine(0);
				pDebugData->f_SetThreadID(NSys::fg_Thread_GetCurrentUID());
				pDebugData->f_SetFlags(EHeapDebugFlag_None);
				fp_StackTraceAquire(pDebugData);

				fp_AllocateBlock((mint)pMemory);
				
				DMibMemoryReportExpression(Overhead = fp_Overhead(pMemory));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pMemory, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pMemory;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_ResetDebugData16(void *_pMem, t_CDebugData &_Data)
		{
			*((t_CDebugData *)m_Heap_16Bit.f_GetExtraData(_pMem)) = _Data;
			return _pMem;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_ResetDebugDataArch(void *_pMem, t_CDebugData &_Data)
		{
			*((t_CDebugData *)m_Heap_ArchSize.f_GetExtraData(_pMem)) = _Data;
			return _pMem;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_AllocAlignedDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pMemory;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				if (m_bCheckHeap)
					fp_CheckHeap(true);
				t_CDebugData *pDebugData;
				if (_Size <= m_Heap_16Bit.f_LargestBlock())
				{
					pMemory = m_Heap_16Bit.f_AllocAligned(_Size, _Align);
					pDebugData = (t_CDebugData *)m_Heap_16Bit.f_GetExtraData(pMemory);
				}
				else if (_Size <= m_Heap_ArchSize.f_LargestBlock())
				{
					pMemory = m_Heap_ArchSize.f_AllocAligned(_Size, _Align);
					pDebugData = (t_CDebugData *)m_Heap_ArchSize.f_GetExtraData(pMemory);
				}
				else
					DMibErrorMemory("Memory of size requested is bigger than heap supports");


				pDebugData->f_SetFile(_pFile);
				pDebugData->f_SetLine(_Line);
				pDebugData->f_SetThreadID(NSys::fg_Thread_GetCurrentUID());
				pDebugData->f_SetFlags(_Flags);
				fp_StackTraceAquire(pDebugData);

				fp_AllocateBlock((mint)pMemory);
				DMibMemoryReportExpression(Overhead = fp_Overhead(pMemory));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pMemory, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pMemory;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_Free(void *_pMemory)
		{
			if (m_bCheckHeap)
				fp_CheckHeap(true);

			fp_FreeBlock((mint)_pMemory);

			const void *pToFind = _pMemory;
			TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
			t_CDebugData *pExtraData = (t_CDebugData *)pChunk->f_GetExtraData(_pMemory);
			fp_StackTraceRelease(pExtraData);
			pChunk->f_Free(_pMemory);

			if (m_bCheckHeap)
				fp_CheckHeap(true);
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Free(void *_pMemory)
		{
			if (!_pMemory)
				return;
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportExpression(mint Size = 0);
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				DMibMemoryReportExpression(Size = fp_Size(_pMemory));
				fp_Free(_pMemory);
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportFree(this, m_pDebugName, _pMemory, Size, nullptr);
#endif
		}
			
		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased inline_small bint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_ContainsBlock(void *_pMemory)
		{
			if (this->f_OwnsLock())
			{
				const void *pToFind = _pMemory;
				TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
				if (!pChunk)
					return false;
				
				return pChunk->f_ContainsBlock(_pMemory);
			}
			else
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				const void *pToFind = _pMemory;
				TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
				if (!pChunk)
					return false;
					
				return pChunk->f_ContainsBlock(_pMemory);
			}
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased inline_small mint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_Size(const void *_pMemory)
		{
			if (!_pMemory)
				return 0;

			const void *pToFind = _pMemory;
			TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
			mint Ret = pChunk->f_Size(_pMemory);
			return Ret;
		}
		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased inline_small mint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Size(const void *_pMemory)
		{
			if (!_pMemory)
				return 0;
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			mint Ret;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				Ret = fp_Size(_pMemory);
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportGetSize(this, m_pDebugName, _pMemory, Ret, nullptr);
#endif
			
			return Ret;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased inline_small fp32 TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Overhead(void const *_pMemory)
		{
			if (!_pMemory)
				return 0;
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);

			const void *pToFind = _pMemory;
			TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
			return pChunk->f_Overhead(_pMemory);
		}
		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased inline_small fp32 TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_Overhead(void const *_pMemory)
		{
			if (!_pMemory)
				return 0;

			const void *pToFind = _pMemory;
			TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
			return pChunk->f_Overhead(_pMemory);
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Realloc(void *_pMemory, mint &_NewSize)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _NewSize);
			void *pNewBlock;
			mint OldSize = 0;

			DMibMemoryReportExpression(fp32 Overhead = 0);
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				if (_pMemory)
				{
					t_CDebugData DebugData;
					const void *pToFind = _pMemory;
					TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);

					void *pHeap;
					OldSize = pChunk->f_SizeAndHeap(_pMemory, pHeap);

					if (pHeap == &m_Heap_16Bit)
					{
						DebugData = *((t_CDebugData*)m_Heap_16Bit.f_GetExtraData(_pMemory));
						if (_NewSize <= m_Heap_16Bit.f_LargestBlock())
						{
							if (m_bCheckHeap)
								fp_CheckHeap(true);

							pNewBlock = pChunk->f_Realloc(_pMemory, _NewSize);
							fp_ResetDebugData16(pNewBlock, DebugData);
							if (pNewBlock != pToFind)
							{
								fp_FreeBlock((mint)pToFind);
								fp_AllocateBlock((mint)pNewBlock);
							}
							if (m_bCheckHeap)
								fp_CheckHeap(true);

						}
						else
						{
							fp_Free(_pMemory);
							pNewBlock = fp_AllocInternal(_NewSize, &DebugData);
						}
					}
					else
					{
						if (m_bCheckHeap)
							fp_CheckHeap(true);
						pNewBlock = pChunk->f_Realloc(_pMemory, _NewSize);
						fp_ResetDebugDataArch(pNewBlock, DebugData);
						if (pNewBlock != pToFind)
						{
							fp_FreeBlock((mint)pToFind);
							fp_AllocateBlock((mint)pNewBlock);
						}
						if (m_bCheckHeap)
							fp_CheckHeap(true);
					}
				}
				else
					pNewBlock = fp_Alloc(_NewSize);
				DMibMemoryReportExpression(Overhead = fp_Overhead(pNewBlock));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportRealloc(this, m_pDebugName, _pMemory, OldSize, nullptr, pNewBlock, 0, RequestedSize, _NewSize, Overhead, nullptr);
#endif
			return pNewBlock;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		only_parameters_aliased return_not_aliased void *TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_Resize(void *_pMemory, mint &_NewSize)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _NewSize);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pNewBlock;
			mint OldSize = 0;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				if (_pMemory)
				{
					t_CDebugData DebugData;
					const void *pToFind = _pMemory;
					TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
					void *pHeap;
					OldSize = pChunk->f_SizeAndHeap(_pMemory, pHeap);

					if (pHeap == &m_Heap_16Bit)
					{
						DebugData = *((t_CDebugData*)m_Heap_16Bit.f_GetExtraData(_pMemory));
						if (_NewSize <= m_Heap_16Bit.f_LargestBlock())
						{
							if (m_bCheckHeap)
								fp_CheckHeap(true);

							pNewBlock = pChunk->f_Resize(_pMemory, _NewSize);
							fp_ResetDebugData16(pNewBlock, DebugData);
							if (pNewBlock != pToFind)
							{
								fp_FreeBlock((mint)pToFind);
								fp_AllocateBlock((mint)pNewBlock);
							}
							if (m_bCheckHeap)
								fp_CheckHeap(true);

							return pNewBlock;
						}
						else
						{
							pNewBlock = fp_AllocInternal(_NewSize, &DebugData);
							if (!pNewBlock)
							{
								fp_Free(_pMemory);
								return nullptr;
							}

							fg_MemCopy(pNewBlock, _pMemory, OldSize < _NewSize ? OldSize : _NewSize);
							fp_Free(_pMemory);
						}
					}
					else
					{
						if (m_bCheckHeap)
							fp_CheckHeap(true);

						pNewBlock = pChunk->f_Resize(_pMemory, _NewSize);
						fp_ResetDebugDataArch(pNewBlock, DebugData);
						if (pNewBlock != pToFind)
						{
							fp_FreeBlock((mint)pToFind);
							fp_AllocateBlock((mint)pNewBlock);
						}

						if (m_bCheckHeap)
							fp_CheckHeap(true);
					}
				}
				else
				{
					pNewBlock = fp_Alloc(_NewSize);
				}
				DMibMemoryReportExpression(Overhead = fp_Overhead(pNewBlock));
			}
			
	#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportResize(this, m_pDebugName, _pMemory, OldSize, nullptr, pNewBlock, 0, RequestedSize, _NewSize, Overhead, nullptr);
	#endif
			return pNewBlock;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		inline_small bint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_CheckHeap(bint _bBreak)
		{
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);
			return fp_CheckHeap(_bBreak);
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		inline_small bint TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_CheckHeap(bint _bBreak)
		{
			++m_DbgCounter;
			if (!m_Heap_16Bit.f_CheckHeap(_bBreak, false))
			{
				if (_bBreak)
					DMibPDebugBreak;
				return false;
			}

			if (!m_Heap_ArchSize.f_CheckHeap(_bBreak, false))
			{
				if (_bBreak)
					DMibPDebugBreak;
				return false;
			}

			return true;
		}

//		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
//		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::fp_TraceDebugDataInfo(t_CDebugData *_pDebugData, void *_pMem);

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_TraceLeaks(bool _bFreeBlocks)
		{
			TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_TraceAllocated
				(
					[this](t_CDebugData* _pDebugData, void* _pMem)
					{
						fp_TraceDebugDataInfo(_pDebugData, _pMem);
					}
					, true
					, _bFreeBlocks
				)
			;
		}

		template <class t_CParams16Bit, class t_CParamArchSize, mint t_MaxHeapTraceDepth, class t_CDebugData>
		template <typename t_CReportFunc> // ReportFunc = void (t_CDebugData* _pDebugData, void* _pMem)
		void TCHeap_CombinedDebug<t_CParams16Bit, t_CParamArchSize, t_MaxHeapTraceDepth, t_CDebugData>::f_TraceAllocated
			(
				t_CReportFunc const &_ReportFunc
				, bint _bAllowIgnore
				, bool _bFreeBlocks
			)
		{
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);

			// Start by eliminating cached blocks
			m_Heap_16Bit.f_Clear();
			m_Heap_ArchSize.f_Clear();

			{
				void *pEnumContext = m_Heap_16Bit.f_EnumAllocatedBlocksStart();

				void *pCurrentBlock = m_Heap_16Bit.f_EnumAllocatedBlocksNext(pEnumContext);

				while (pCurrentBlock)
				{
					t_CDebugData *pDebugData = (t_CDebugData *)m_Heap_16Bit.f_GetExtraData(pCurrentBlock);

					if (!_bAllowIgnore || !(pDebugData->f_GetFlags() & EHeapDebugFlag_Ignore))
					{
						_ReportFunc(pDebugData, pCurrentBlock);
					}

					void *pLastBlock = pCurrentBlock;
					pCurrentBlock = m_Heap_16Bit.f_EnumAllocatedBlocksNext(pEnumContext);

					// Free the block so the debug data trace is released
					if (_bFreeBlocks)
						fp_Free(pLastBlock);
				}

				m_Heap_16Bit.f_EnumAllocatedBlocksFinish(pEnumContext);
			}

			{
				void *pEnumContext = m_Heap_ArchSize.f_EnumAllocatedBlocksStart();

				void *pCurrentBlock = m_Heap_ArchSize.f_EnumAllocatedBlocksNext(pEnumContext);

				while (pCurrentBlock)
				{
					t_CDebugData *pDebugData = (t_CDebugData *)m_Heap_ArchSize.f_GetExtraData(pCurrentBlock);
					if (!_bAllowIgnore || !(pDebugData->f_GetFlags() & EHeapDebugFlag_Ignore))
					{
						_ReportFunc(pDebugData, pCurrentBlock);
					}

					void *pLastBlock = pCurrentBlock;
					pCurrentBlock = m_Heap_ArchSize.f_EnumAllocatedBlocksNext(pEnumContext);

					if (_bFreeBlocks)
						fp_Free(pLastBlock);
				}

				m_Heap_ArchSize.f_EnumAllocatedBlocksFinish(pEnumContext);
			}
                
		}

	}
}

