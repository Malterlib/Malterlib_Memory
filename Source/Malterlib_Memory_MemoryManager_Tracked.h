// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CSuper, typename t_CAllocationInfo = void>
	struct TCMemoryManagerTracked : public t_CSuper
	{
	public:
		template <typename... tfp_CAllocator>
		TCMemoryManagerTracked(ch8 const *_pName, tfp_CAllocator &&..._Params);

		~TCMemoryManagerTracked();

		void *f_AllocWithSize(umint &_Size);
		void *f_Alloc(umint _Size);
		void *f_AllocWithSizeInline(umint &_Size);
		void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment);
		void *f_AllocAligned(umint _Size, umint _Alignment);
		void *f_AllocAlignedWithSizeInline(umint &_Size, umint _Alignment);
		void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		void *f_Realloc(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_ReallocInline(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_Resize(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_ResizeInline(void * _pMemory, umint &_Size, umint _OldSize);

		void f_Free(void * _pMemory, umint _Size);
		void f_FreeInline(void * _pMemory, umint _Size);
		void f_FreeNoSize(void * _pMemory);
		void f_FreeNoSizeInline(void * _pMemory);

		void *f_AllocWithSizeDebug(umint &_Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);

		umint f_Size(void const * _pMemory) const;
		umint f_TrySize(void const * _pMemory) const;
		umint f_SizeInline(void const * _pMemory) const;
		umint f_SizePadded(umint _Size);

		TCMemoryManagerTracked *f_GetMemoryManager(void const *_pMemory); // Will only work between managers that share the same magic

	private:

		void *fp_Alloc(umint &_Size, umint _Alignment);

		struct DMibPAlignType(CPreBlockData, 16)
		{
			umint m_HeaderSize;
			umint m_TotalSize;
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

		void *f_AllocWithSize(umint &_Size);
		void *f_Alloc(umint _Size);
		void *f_AllocWithSizeInline(umint &_Size);
		void *f_AllocAlignedWithSize(umint &_Size, umint _Alignment);
		void *f_AllocAligned(umint _Size, umint _Alignment);
		void *f_AllocAlignedWithSizeInline(umint &_Size, umint _Alignment);
		void f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor);

		void *f_Realloc(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_ReallocInline(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_Resize(void * _pMemory, umint &_Size, umint _OldSize);
		void *f_ResizeInline(void * _pMemory, umint &_Size, umint _OldSize);

		void f_Free(void * _pMemory, umint _Size);
		void f_FreeInline(void * _pMemory, umint _Size);
		void f_FreeNoSizeInline(void * _pMemory);
		void f_FreeNoSize(void * _pMemory);

		void *f_AllocWithSizeDebug(umint &_Size, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ReallocDebug(void *_pMem, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);
		void *f_ResizeDebug(void *_pMem, umint &_Size, umint _OldSize, ch8 const * _pFile, uint32 _Line, EHeapDebugFlag _Flags);

		umint f_Size(void const * _pMemory) const;
		umint f_TrySize(void const * _pMemory) const;
		umint f_SizeInline(void const * _pMemory) const;

		TCMemoryManagerTracked *f_GetMemoryManager(void const *_pMemory); // Will only work between managers that share the same magic

	private:
		ch8 const *mp_pName;
	};
}

#include "Malterlib_Memory_MemoryManager_Tracked.hpp"
