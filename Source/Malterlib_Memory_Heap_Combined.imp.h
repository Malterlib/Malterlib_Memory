// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::TCHeap_Combined(ch8 const *_pDebugName)
#		if DMibConfig_Memory_Shims_Enable
			: m_pDebugName(_pDebugName)
#		endif
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAllocatorName(this, m_pDebugName);
#endif

			m_Heap_8Bit.f_Construct(&m_ChunksTree);
			m_Heap_16Bit.f_Construct(&m_ChunksTree);				
			m_Heap_ArchSize.f_Construct(&m_ChunksTree);
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::~TCHeap_Combined()
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			f_Clear();
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAllocatorDelete(this, m_pDebugName);
#endif
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		void TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Clear()
		{
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);
			m_Heap_8Bit.f_Clear();
			m_Heap_16Bit.f_Clear();
			m_Heap_ArchSize.f_Clear();

			while (m_ChunksTree.f_GetRoot())
			{
				m_ChunksTree.f_GetRoot()->f_Destroy(m_ChunksTree);
			}             		
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		mint TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_MaxGranularity()
		{
			mint MaxGran = 0;
			if ((t_EnableHeapsFlags & 1))
				MaxGran = fg_Max(m_Heap_8Bit.f_MaxGranularity(), MaxGran);
			if ((t_EnableHeapsFlags & 2))
				MaxGran = fg_Max(m_Heap_16Bit.f_MaxGranularity(), MaxGran);
			if (t_VirtualLimit > 0)
				MaxGran = fg_Max(CAllocator::f_GranularityAlloc(), MaxGran);
			if ((t_EnableHeapsFlags & 4))
				MaxGran = fg_Max(m_Heap_ArchSize.f_MaxGranularity(), MaxGran);

			return MaxGran;
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		mint TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Get8BitLargest()
		{
			return m_Heap_8Bit.f_LargestBlock();
		}


		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Alloc(const mint &_Size)
		{
			mint Size = _Size;
			return f_Alloc(Size);
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased return_not_aliased void *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Alloc(mint &_Size)
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

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased return_not_aliased void *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::fp_Alloc(mint &_Size)
		{
			void *pRet;
			if ((t_EnableHeapsFlags & 1) && _Size <= m_Heap_8Bit.f_LargestBlock())
				pRet = m_Heap_8Bit.f_Alloc(_Size);
			else if ((t_EnableHeapsFlags & 2) && _Size <= m_Heap_16Bit.f_LargestBlock())
				pRet = m_Heap_16Bit.f_Alloc(_Size);
			else if (t_VirtualLimit > 0 && _Size >= (mint)t_VirtualLimit)
			{
				_Size = NMib::fg_AlignUp(_Size, CAllocator::f_GranularityAlloc());
				pRet = CAllocator::f_Alloc(_Size);
			}
			else if ((t_EnableHeapsFlags & 4) && _Size <= m_Heap_ArchSize.f_LargestBlock())
				pRet = m_Heap_ArchSize.f_Alloc(_Size);
			else
			{
				DMibErrorMemory("Memory of size requested is bigger than heap supports");
				pRet = nullptr;
			}
			return pRet;
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased return_not_aliased void *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_AllocAligned(mint &_Size, mint _Align)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pRet;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				_Size = fg_AlignUp(_Size, _Align);
				if ((t_EnableHeapsFlags & 1) && (_Size + _Align) <= m_Heap_8Bit.f_LargestBlock())
					pRet = m_Heap_8Bit.f_AllocAligned(_Size, _Align);
				else if ((t_EnableHeapsFlags & 2) && (_Size + _Align) <= m_Heap_16Bit.f_LargestBlock())
					pRet = m_Heap_16Bit.f_AllocAligned(_Size, _Align);
				else if (t_VirtualLimit > 0 && (_Size + _Align) >= (mint)t_VirtualLimit)
				{
					_Size = NMib::fg_AlignUp(_Size, CAllocator::f_GranularityAlloc());
					pRet = CAllocator::f_Alloc(_Size);
				}
				else if ((t_EnableHeapsFlags & 4) && (_Size + _Align) <= m_Heap_ArchSize.f_LargestBlock())
					pRet = m_Heap_ArchSize.f_AllocAligned(_Size, _Align);
				else
				{
					DMibErrorMemory("Memory of size requested is bigger than heap supports");
					pRet = nullptr;
				}
				DMibMemoryReportExpression(Overhead = fp_Overhead(pRet));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pRet, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pRet;
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased return_not_aliased inline_medium TCHeapChunk<typename TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::CAllocator> *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::fp_FindChunk(const void *_pMemory)
		{
			const void *pToFind = _pMemory;
			TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
			if (t_VirtualLimit > 0)
			{
				if ((uint8 *)pToFind > (uint8 *)pChunk->m_pBase + pChunk->m_Size)
					return nullptr;
				else
					return pChunk;
			}
			else
				return pChunk;
		}
		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased void TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Free(void *_pMemory)
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
		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased void TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::fp_Free(void *_pMemory)
		{
			TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(_pMemory);
				
			if (t_VirtualLimit < 0 || pChunk)
				pChunk->f_Free(_pMemory);
			else
				CAllocator::f_Free(_pMemory);
		}
			
		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased inline_small bint TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_ContainsBlock(void *_pMemory)
		{
			if (this->f_OwnsLock())
			{
				const void *pToFind = _pMemory;
				TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(pToFind);
				if (!pChunk)
					return false;
				
				return pChunk->f_ContainsBlock(_pMemory);
			}
			else
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				const void *pToFind = _pMemory;
				TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(pToFind);
				if (!pChunk)
					return false;
					
				return pChunk->f_ContainsBlock(_pMemory);
			}
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased mint TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::fp_Size(const void *_pMemory)
		{
			TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(_pMemory);
			if (t_VirtualLimit < 0 || pChunk)
				return pChunk->f_Size(_pMemory);
			else
				return CAllocator::f_Size(_pMemory);
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased mint TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Size(const void *_pMemory)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			mint Ret;
			if (!_pMemory)
				Ret = 0;
			else
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

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased fp32 TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Overhead(void const *_pMemory)
		{
			if (!_pMemory)
				return 0.0f;
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);

			TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(_pMemory);
			if (t_VirtualLimit < 0 || pChunk)
				return pChunk->f_Overhead(_pMemory);
			else
				return CAllocator::f_Overhead(_pMemory);
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased fp32 TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::fp_Overhead(void const *_pMemory)
		{
			if (!_pMemory)
				return 0.0f;
			TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(_pMemory);
			if (t_VirtualLimit < 0 || pChunk)
				return pChunk->f_Overhead(_pMemory);
			else
				return CAllocator::f_Overhead(_pMemory);
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased return_not_aliased void *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Realloc(void *_pMemory, mint &_NewSize)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _NewSize);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pNewMem;
			mint OldSize = 0;
			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				if (_pMemory)
				{
					TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(_pMemory);

					void *pHeap;
					if (t_VirtualLimit < 0 || pChunk)
						OldSize	= pChunk->f_SizeAndHeap(_pMemory, pHeap);
					else
					{
						pHeap = nullptr;
						OldSize = CAllocator::f_Size(_pMemory);
					}

					if ((t_EnableHeapsFlags & 1) && pHeap == &m_Heap_8Bit)
					{
						if (_NewSize <= m_Heap_8Bit.f_LargestBlock())
						{
							if (t_VirtualLimit < 0 || pChunk)
								pNewMem = pChunk->f_Realloc(_pMemory, _NewSize);
							else
								pNewMem = CAllocator::f_Realloc(_pMemory, _NewSize);
						}
						else
						{
							if (t_VirtualLimit < 0 || pChunk)
								pChunk->f_Free(_pMemory);
							else
								CAllocator::f_Free(_pMemory);
							pNewMem = fp_Alloc(_NewSize);
						}
					}
					else if ((t_EnableHeapsFlags & 2) && pHeap == &m_Heap_16Bit)
					{
						if (_NewSize <= m_Heap_16Bit.f_LargestBlock())
						{
							if (t_VirtualLimit < 0 || pChunk)
								pNewMem = pChunk->f_Realloc(_pMemory, _NewSize);
							else
								pNewMem = CAllocator::f_Realloc(_pMemory, _NewSize);
						}
						else
						{
							if (t_VirtualLimit < 0 || pChunk)
								pChunk->f_Free(_pMemory);
							else
								CAllocator::f_Free(_pMemory);
							pNewMem = fp_Alloc(_NewSize);
						}
					}
					else if ((t_EnableHeapsFlags & 4))
					{
						if (t_VirtualLimit < 0 || pChunk)
							pNewMem = pChunk->f_Realloc(_pMemory, _NewSize);
						else
							pNewMem = CAllocator::f_Realloc(_pMemory, _NewSize);
					}
					else
					{
						// Should not get here
						return nullptr;
					}
				}
				else
				{
					pNewMem = fp_Alloc(_NewSize);
				}
				DMibMemoryReportExpression(Overhead = fp_Overhead(pNewMem));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportRealloc(this, m_pDebugName, _pMemory, OldSize, nullptr, pNewMem, 0, RequestedSize, _NewSize, Overhead, nullptr);
#endif
			return pNewMem;
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		only_parameters_aliased void *TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_Resize(void *_pMemory, mint &_NewSize)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _NewSize);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pNewMem;
			mint OldSize = 0;

			{
				DMibLockTyped(typename t_CParams16Bit::CLock, *this);
				if (_pMemory)
				{
					TCHeapChunk<CAllocator> *pChunk = fp_FindChunk(_pMemory);

					void *pHeap;
					if (t_VirtualLimit < 0 || pChunk)
						OldSize	= pChunk->f_SizeAndHeap(_pMemory, pHeap);
					else
					{
						pHeap = nullptr;
						OldSize = CAllocator::f_Size(_pMemory);
					}

					if ((t_EnableHeapsFlags & 1) && pHeap == &m_Heap_8Bit)
					{
						if (_NewSize <= m_Heap_8Bit.f_LargestBlock())
						{
							if (t_VirtualLimit < 0 || pChunk)
								pNewMem = pChunk->f_Resize(_pMemory, _NewSize);
							else
								pNewMem = CAllocator::f_Resize(_pMemory, _NewSize);
						}
						else
						{
							pNewMem = fp_Alloc(_NewSize);
							NMem::fg_MemCopy(pNewMem, _pMemory, fg_Min(OldSize, _NewSize));

							if (t_VirtualLimit < 0 || pChunk)
								pChunk->f_Free(_pMemory);
							else
								CAllocator::f_Free(_pMemory);
						}
					}
					else if ((t_EnableHeapsFlags & 2) && pHeap == &m_Heap_16Bit)
					{
						if (_NewSize <= m_Heap_16Bit.f_LargestBlock())
						{
							if (t_VirtualLimit < 0 || pChunk)
								pNewMem = pChunk->f_Resize(_pMemory, _NewSize);
							else
								pNewMem = CAllocator::f_Resize(_pMemory, _NewSize);
						}
						else
						{
							pNewMem = fp_Alloc(_NewSize);
							NMem::fg_MemCopy(pNewMem, _pMemory, fg_Min(OldSize, _NewSize));

							if (t_VirtualLimit < 0 || pChunk)
								pChunk->f_Free(_pMemory);
							else
								CAllocator::f_Free(_pMemory);
						}
					}
					else if ((t_EnableHeapsFlags & 4))
					{
						if (t_VirtualLimit < 0 || pChunk)
							pNewMem = pChunk->f_Resize(_pMemory, _NewSize);
						else
							pNewMem = CAllocator::f_Resize(_pMemory, _NewSize);
					}
					else
					{
						// Should not get here
						return nullptr;
					}
				}
				else
					pNewMem = fp_Alloc(_NewSize);
				DMibMemoryReportExpression(Overhead = fp_Overhead(pNewMem));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportResize(this, m_pDebugName, _pMemory, OldSize, nullptr, pNewMem, 0, RequestedSize, _NewSize, Overhead, nullptr);
#endif
			return pNewMem;
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		bint TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_CheckHeap(bint _bBreak)
		{
			DMibLockTyped(typename t_CParams16Bit::CLock, *this);
			if (!m_Heap_8Bit.f_CheckHeap(_bBreak, false))
				return false;
			if (!m_Heap_16Bit.f_CheckHeap(_bBreak, false))
				return false;
			if (!m_Heap_ArchSize.f_CheckHeap(_bBreak, false))
				return false;

			return true;
		}

		template <aint t_VirtualLimit, typename t_CParams16Bit, typename t_CParamArchSize, aint t_EnableHeapsFlags>
		void TCHeap_Combined<t_VirtualLimit, t_CParams16Bit, t_CParamArchSize, t_EnableHeapsFlags>::f_TraceLeaks(bool _bFreeBlocks)
		{
			// Start by eliminating cached blocks
			// No debug Heap
		}
	}
}

