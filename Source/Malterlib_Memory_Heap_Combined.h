// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		template <aint t_VirtualLimit = -1, typename t_CParams16Bit = TCHeapParams<>, typename t_CParamArchSize = t_CParams16Bit, aint t_EnableHeapsFlags = 0x7>
		class TCHeap_Combined : public t_CParams16Bit::CLock
		{
		public:
			typedef t_CParams16Bit CHeapParams;
			typedef typename t_CParams16Bit::CAllocator CAllocator;
		private:
#		if DMibConfig_Memory_Shims_Enable
			ch8 const *m_pDebugName;
#		endif
			
			typedef typename TCHeapChunk<CAllocator>::CCompareAVL CCompareAVL;
			typedef TCHeapChunk<CAllocator> CHeapChunk;
			typename TCHeapChunk<CAllocator>::CTree m_ChunksTree;

			class CParams8Bit : public t_CParams16Bit
			{
			public:
				
				const static EAllocationFlag mc_AllocationFlags = EAllocationFlag_None;
				typedef CHeapBlock8Bit CBlock;
			};

			class CParams16Bit : public t_CParams16Bit
			{
			public:
				const static EAllocationFlag mc_AllocationFlags = EAllocationFlag_LocationDown;
				typedef CHeapBlock16Bit CBlock;
			};
			class CParamsArchSize : public t_CParamArchSize
			{
			public:
				const static EAllocationFlag mc_AllocationFlags = EAllocationFlag_LocationUp;
				typedef CHeapBlockArchSize CBlock;
			};

			TCHeap<CParams8Bit> m_Heap_8Bit;
			TCHeap<CParams16Bit> m_Heap_16Bit;
			TCHeap<CParamsArchSize> m_Heap_ArchSize;

			only_parameters_aliased return_not_aliased inline_medium TCHeapChunk<CAllocator> *fp_FindChunk(const void *_pMemory);
			only_parameters_aliased return_not_aliased void *fp_Alloc(mint &_Size);
			only_parameters_aliased void fp_Free(void *_pMemory);
			only_parameters_aliased fp32 fp_Overhead(void const *_pMemory);
			only_parameters_aliased mint fp_Size(const void *_pMemory);

		public:

			TCHeap_Combined(ch8 const *_pDebugName = "Undefined TCHeap_Combined");
			~TCHeap_Combined();

			void f_Clear();
			mint f_MaxGranularity();
			only_parameters_aliased return_not_aliased inline_small void *f_Alloc(const mint &_Size);
			only_parameters_aliased return_not_aliased void *f_Alloc(mint &_Size);
			only_parameters_aliased return_not_aliased void *f_AllocAligned(mint &_Size, mint _Align);
			only_parameters_aliased void f_Free(void *_pMemory);
			only_parameters_aliased inline_small bint f_ContainsBlock(void *_pMemory);
			only_parameters_aliased mint f_Size(const void *_pMemory);
			only_parameters_aliased fp32 f_Overhead(void const *_pMemory);
			only_parameters_aliased return_not_aliased void *f_Realloc(void *_pMemory, mint &_NewSize);
			only_parameters_aliased return_not_aliased void *f_Resize(void *_pMemory, mint &_NewSize);
			mint f_Get8BitLargest();
			bint f_CheckHeap(bint _bBreak);
			void f_TraceLeaks(bool _bFreeBlocks);
			
			using t_CParams16Bit::CLock::f_Lock;
			using t_CParams16Bit::CLock::f_Unlock;
			
		};
	}
}

