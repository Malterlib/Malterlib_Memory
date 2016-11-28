// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		template <class t_CHeapParams = TCHeapParams<> >
		class TCHeap_StandAlone : public t_CHeapParams::CLock
		{
#		if DMibConfig_Memory_Shims_Enable
			ch8 const *m_pDebugName;
#		endif
		public:
			TCHeap<t_CHeapParams> m_Heap;
			typedef t_CHeapParams CHeapParams;
			typedef typename t_CHeapParams::CAllocator CAllocator;
			typedef typename TCHeap<t_CHeapParams>::CFreeBlockBucket CFreeBlockBucketTyped;
		private:
			typename TCHeapChunk<CAllocator>::CTree m_ChunksTree;
			only_parameters_aliased return_not_aliased inline_small void *fp_Alloc(mint &_Size);
			only_parameters_aliased inline_small void fp_Free(void *_pMemory);
			only_parameters_aliased inline_small fp32 fp_Overhead(void const *_pMemory);
		public:

			TCHeap_StandAlone(ch8 const *_pDebugName = "Undefined TCHeap_StandAlone");
			~TCHeap_StandAlone();
			void f_Clear();
			mint f_MaxGranularity();
			only_parameters_aliased return_not_aliased inline_small void *f_Alloc(const mint &_Size);
			only_parameters_aliased return_not_aliased inline_small void *f_Alloc(mint &_Size);
			only_parameters_aliased return_not_aliased inline_small void *f_AllocAligned(mint &_Size, mint _Align);
			only_parameters_aliased inline_small void f_Free(void *_pMemory);
			only_parameters_aliased inline_small bint f_ContainsBlock(void *_pMemory);
			only_parameters_aliased inline_small mint f_Size(const void *_pMemory);
			only_parameters_aliased inline_small fp32 f_Overhead(void const *_pMemory);
			only_parameters_aliased return_not_aliased inline_small void *f_Realloc(void *_pMemory, mint &_NewSize);
			only_parameters_aliased inline_small void *f_Resize(void *_pMemory, mint &_NewSize);
			inline_small bint f_CheckHeap(bint _bBreak);
			
			using t_CHeapParams::CLock::f_Lock;
			using t_CHeapParams::CLock::f_Unlock;

		};
	}
}
