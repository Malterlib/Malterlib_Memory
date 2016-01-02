// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
		template <typename t_CSuper, typename t_CAllocationInfo = void>
		struct TCMemoryManagerTracked : public t_CSuper
		{
		public:
			template <typename... tfp_CAllocator>
			TCMemoryManagerTracked(ch8 const *_pName, tfp_CAllocator &&..._Params);
			
			~TCMemoryManagerTracked();

			void *f_Alloc(mint & _Size);
			void *f_AllocInline(mint & _Size);
			void *f_AllocAligned(mint & _Size, mint _Alignment);
			void *f_AllocAlignedInline(mint & _Size, mint _Alignment);
			void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			
			void *f_Realloc(void * _pMemory, mint &_Size);
			void *f_ReallocInline(void * _pMemory, mint &_Size);
			void *f_Resize(void * _pMemory, mint &_Size);
			void *f_ResizeInline(void * _pMemory, mint &_Size);

			void *f_AllocDebug(mint & _Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			void *f_AllocAlignedDebug(mint & _Size, mint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			void *f_ReallocDebug(void *_pMem, mint & _Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			void *f_ResizeDebug(void *_pMem, mint & _Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			
			mint f_Size(void const * _pMemory) const;
			mint f_TrySize(void const * _pMemory) const;
			mint f_SizeInline(void const * _pMemory) const;
			void f_Free(void * _pMemory);
			void f_FreeInline(void * _pMemory);
			mint f_SizePadded(mint _Size);

		private:

			void *fp_Alloc(mint &_Size, mint _Alignment);

			struct DMibPAlignType(CPreBlockData, 16)
			{
				mint m_HeaderSize;
				t_CAllocationInfo m_AllocationInfo;
			};
			
			ch8 const *mp_pName;			
		};

		template <typename t_CSuper>
		struct TCMemoryManagerTracked<t_CSuper, void> : public t_CSuper
		{
		public:
			template <typename... tfp_CAllocator>
			TCMemoryManagerTracked(ch8 const *_pName, tfp_CAllocator &&..._Params);
			
			~TCMemoryManagerTracked();

			void *f_Alloc(mint & _Size);
			void *f_AllocInline(mint & _Size);
			void *f_AllocAligned(mint & _Size, mint _Alignment);
			void *f_AllocAlignedInline(mint & _Size, mint _Alignment);
			void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			
			void *f_Realloc(void * _pMemory, mint &_Size);
			void *f_ReallocInline(void * _pMemory, mint &_Size);
			void *f_Resize(void * _pMemory, mint &_Size);
			void *f_ResizeInline(void * _pMemory, mint &_Size);

			void *f_AllocDebug(mint & _Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			void *f_AllocAlignedDebug(mint & _Size, mint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			void *f_ReallocDebug(void *_pMem, mint & _Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			void *f_ResizeDebug(void *_pMem, mint & _Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
			
			mint f_Size(void const * _pMemory) const;
			mint f_TrySize(void const * _pMemory) const;
			mint f_SizeInline(void const * _pMemory) const;
			void f_Free(void * _pMemory);
			void f_FreeInline(void * _pMemory);
			
		private:
			ch8 const *mp_pName;			
		};
	}
}

#include "Malterlib_Memory_MemoryManager_Tracked.hpp"
