// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		template <class t_CHeapParams>
		TCHeap_StandAlone<t_CHeapParams>::TCHeap_StandAlone(ch8 const *_pDebugName)
#		if DMibConfig_Memory_Shims_Enable
			: m_pDebugName(_pDebugName)
#		endif
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAllocatorName(this, m_pDebugName);
#endif

			m_Heap.f_Construct(&m_ChunksTree);
		}
		
		template <class t_CHeapParams>
		TCHeap_StandAlone<t_CHeapParams>::~TCHeap_StandAlone()				
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			f_Clear();
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAllocatorDelete(this, m_pDebugName);
#endif
		}

		template <class t_CHeapParams>
		void TCHeap_StandAlone<t_CHeapParams>::f_Clear()
		{
			DMibLockTyped(typename t_CHeapParams::CLock, *this);
			m_Heap.f_Clear();
//				DMibTreeAVLAllocator_Iterator_FromTemplate(TCHeapChunk<CAllocator>, m_ChunkTree, typename TCHeapChunk<CAllocator>::CCompareAVL, CAllocator) Iter = m_ChunksTree;
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

		template <class t_CHeapParams>
		mint TCHeap_StandAlone<t_CHeapParams>::f_MaxGranularity()
		{
			return m_Heap.f_MaxGranularity();
		}


		template <class t_CHeapParams>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_StandAlone<t_CHeapParams>::f_Alloc(const mint &_Size)
		{
			mint Size = _Size;
			return f_Alloc(Size);
		}

		template <class t_CHeapParams>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_StandAlone<t_CHeapParams>::f_Alloc(mint &_Size)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pRet;
			{
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				pRet = fp_Alloc(_Size);
				DMibMemoryReportExpression(Overhead = fp_Overhead(pRet));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pRet, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pRet;
		}

		template <class t_CHeapParams>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_StandAlone<t_CHeapParams>::fp_Alloc(mint &_Size)
		{
			if (_Size <= m_Heap.f_LargestBlock())
				return m_Heap.f_Alloc(_Size);
			else
				DMibErrorMemory("Memory of size requested is bigger than heap supports");
		}

		template <class t_CHeapParams>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_StandAlone<t_CHeapParams>::f_AllocAligned(mint &_Size, mint _Align)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pRet;
			{
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				if (_Size <= m_Heap.f_LargestBlock())
					pRet = m_Heap.f_AllocAligned(_Size, _Align);
				else
					DMibErrorMemory("Memory of size requested is bigger than heap supports");
				DMibMemoryReportExpression(Overhead = fp_Overhead(pRet));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportAlloc(this, m_pDebugName, pRet, 0, RequestedSize, _Size, Overhead, nullptr);
#endif
			return pRet;
		}

		template <class t_CHeapParams>
		only_parameters_aliased inline_small void TCHeap_StandAlone<t_CHeapParams>::f_Free(void *_pMemory)
		{
			if (!_pMemory)
				return;
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportExpression(mint Size = 0);
			{
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				DMibMemoryReportExpression(Size = m_Heap.f_Size(_pMemory));
				m_Heap.f_Free(_pMemory);
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportFree(this, m_pDebugName, _pMemory, Size, nullptr);
#endif
		}
			
		template <class t_CHeapParams>
		only_parameters_aliased inline_small bint TCHeap_StandAlone<t_CHeapParams>::f_ContainsBlock(void *_pMemory)
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
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				const void *pToFind = _pMemory;
				TCHeapChunk<CAllocator> *pChunk = m_ChunksTree.f_FindLargestLessThanEqual(pToFind);
				if (!pChunk)
					return false;
					
				return pChunk->f_ContainsBlock(_pMemory);
			}
		}
			

		template <class t_CHeapParams>
		only_parameters_aliased inline_small mint TCHeap_StandAlone<t_CHeapParams>::f_Size(const void *_pMemory)
		{
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			mint Ret;
			{
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				Ret = m_Heap.f_Size(_pMemory);
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportGetSize(this, m_pDebugName, _pMemory, Ret, nullptr);
#endif
			return Ret;
		}

		template <class t_CHeapParams>
		only_parameters_aliased inline_small fp32 TCHeap_StandAlone<t_CHeapParams>::f_Overhead(const void *_pMemory)
		{
			DMibLockTyped(typename t_CHeapParams::CLock, *this);
			return m_Heap.f_Overhead(_pMemory);
		}

		template <class t_CHeapParams>
		only_parameters_aliased inline_small fp32 TCHeap_StandAlone<t_CHeapParams>::fp_Overhead(const void *_pMemory)
		{
			return m_Heap.f_Overhead(_pMemory);
		}

		template <class t_CHeapParams>
		only_parameters_aliased return_not_aliased inline_small void *TCHeap_StandAlone<t_CHeapParams>::f_Realloc(void *_pMemory, mint &_NewSize)
		{
			DMibMemoryReportSaveVar(RequestedSize, _NewSize);
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pNewMem;
			mint OldSize = 0;
			{
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				OldSize = m_Heap.f_Size(_pMemory);
				if (_NewSize <= m_Heap.f_LargestBlock())
					pNewMem = m_Heap.f_Realloc(_pMemory, _NewSize);
				else
					DMibErrorMemory("Memory of size requested is bigger than heap supports");
				DMibMemoryReportExpression(Overhead = fp_Overhead(pNewMem));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportRealloc(this, m_pDebugName, _pMemory, OldSize, nullptr, pNewMem, 0, RequestedSize, _NewSize, Overhead, nullptr);
#endif
			return pNewMem;
		}

		template <class t_CHeapParams>
		only_parameters_aliased inline_small void *TCHeap_StandAlone<t_CHeapParams>::f_Resize(void *_pMemory, mint &_NewSize)
		{
			DMibMemoryReportSaveVar(RequestedSize, _NewSize);
			DMibMemoryGoingToReportScope(this, m_pDebugName != nullptr);
			DMibMemoryReportExpression(fp32 Overhead = 0);
			void *pNewMem;
			mint OldSize = 0;
			{
				DMibLockTyped(typename t_CHeapParams::CLock, *this);
				OldSize = m_Heap.f_Size(_pMemory);
				if (_NewSize <= m_Heap.f_LargestBlock())
					pNewMem = m_Heap.f_Resize(_pMemory, _NewSize);
				else
					DMibErrorMemory("Memory of size requested is bigger than heap supports");
				DMibMemoryReportExpression(Overhead = fp_Overhead(pNewMem));
			}
#if DMibConfig_Memory_Shims_Enable
			if (m_pDebugName != nullptr)
				DMibMemoryReportResize(this, m_pDebugName, _pMemory, OldSize, nullptr, pNewMem, 0, RequestedSize, _NewSize, Overhead, nullptr);
#endif
			return pNewMem;
		}

		template <class t_CHeapParams>
		inline_small bint TCHeap_StandAlone<t_CHeapParams>::f_CheckHeap(bint _bBreak)
		{
			DMibLockTyped(typename t_CHeapParams::CLock, *this);
			return m_Heap.f_CheckHeap(_bBreak, false);
		}
	}
}
