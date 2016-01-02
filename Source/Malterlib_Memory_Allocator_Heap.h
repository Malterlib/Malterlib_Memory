// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{

		/************************************************************************************************\
		||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
		|| Heap alloc functions
		||______________________________________________________________________________________________||
		\************************************************************************************************/
		
		only_parameters_aliased return_not_aliased void *fg_Alloc(mint &_Size);
		only_parameters_aliased return_not_aliased void *fg_AllocInitZero(mint &_Size);
		only_parameters_aliased return_not_aliased void *fg_AllocAligned(mint &_Size, mint _Align);
		only_parameters_aliased void fg_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
		only_parameters_aliased return_not_aliased void *fg_Realloc(void *_pMemory, mint &_Size);
		only_parameters_aliased return_not_aliased void *fg_Resize(void *_pMemory, mint &_Size);
		only_parameters_aliased void fg_Free(void *_pMemory);
		only_parameters_aliased mint fg_Size(const void *_pMemory);
		only_parameters_aliased mint fg_TrySize(const void *_pMemory);
		only_parameters_aliased mint fg_SizePadded(mint _Size);
		only_parameters_aliased fp32 fg_Overhead(void const *_pMemory);
		only_parameters_aliased mint fg_Granularity();
		
#			if DMibConfig_MalterlibMemoryManager_Debug
				only_parameters_aliased return_not_aliased void *fg_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				only_parameters_aliased return_not_aliased void *fg_AllocAlignedDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				only_parameters_aliased void fg_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				only_parameters_aliased return_not_aliased void *fg_ReallocDebug(void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
				only_parameters_aliased return_not_aliased void *fg_ResizeDebug(void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
#			else
				only_parameters_aliased return_not_aliased static inline_small void *fg_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_Alloc(_Size);
				}
				only_parameters_aliased return_not_aliased static inline_small void *fg_AllocAlignedDebug(mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_AllocAligned(_Size, _Align);
				}
				only_parameters_aliased return_not_aliased static inline_small void *fg_ReallocDebug(void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_Realloc(_pMemory, _Size);
				}
				only_parameters_aliased return_not_aliased static inline_small void *fg_ResizeDebug(void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
				{
					return fg_Resize(_pMemory, _Size);
				}
#			endif
		
		struct CAllocator_AutoDestroy
		{
			void *m_pMemory;
			mint m_Size;
			
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
			
			only_parameters_aliased return_not_aliased void *f_Get() const
			{
				return m_pMemory;
			}
			
			void f_Claim()
			{
				m_pMemory = nullptr;
			}
		};
		
		class CAllocator_Heap
		{
		public:

			enum
			{
				mc_Reporting = true
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = true
			};
			typedef CDefaultPointerHolder CPtrHolder;
			
			struct CAutoDestroy : public CAllocator_AutoDestroy
			{
				CAutoDestroy(CAutoDestroy &&) = default;
				CAutoDestroy &operator =(CAutoDestroy &&) = default;
				CAutoDestroy() = default;
				~CAutoDestroy()
				{
					if (this->m_pMemory)
						f_Free(this->m_pMemory, this->m_Size);
				}
			};

			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static mint f_StaticAddresses();
			
			static mint f_GranularityAlloc(bint _bLargePages = false);
			static mint f_GranularityCommit(bint _bLargePages = false);
			static mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased static mint f_Size(void *_pBlock);
			only_parameters_aliased static mint f_TrySize(void *_pBlock);
			static mint f_SizePadded(mint _Size);
			static fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
			static bint f_CanCommit();
			static bint f_CanProtect();
			only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased return_not_aliased static void *f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAlignedDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAlignedDebug(const mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Alloc(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static CAutoDestroy f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Free(void *_pBlock, mint _Size = 0);
		};		

		class CAllocator_NonTrackedHeap
		{
		public:
			enum
			{
				mc_Reporting = false
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = true
			};
			typedef CDefaultPointerHolder CPtrHolder;
			
			struct CAutoDestroy : public CAllocator_AutoDestroy
			{
				CAutoDestroy(CAutoDestroy &&) = default;
				CAutoDestroy &operator =(CAutoDestroy &&) = default;
				CAutoDestroy() = default;
				~CAutoDestroy()
				{
					if (this->m_pMemory)
						f_Free(this->m_pMemory, this->m_Size);
				}
			};

			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static mint f_StaticAddresses();
			static mint f_GranularityAlloc(bint _bLargePages = false);
			static mint f_GranularityCommit(bint _bLargePages = false);
			static mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased static mint f_Size(void *_pBlock);
			only_parameters_aliased static mint f_TrySize(void *_pBlock);
			static mint f_SizePadded(mint _Size);
			static fp32 f_Overhead(void const *_pBlock);
			static bint f_CanCommit();
			static bint f_CanProtect();
			only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased return_not_aliased static void *f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAlignedDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAligendDebug(const mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Alloc(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static CAutoDestroy f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Free(void *_pBlock, mint _Size = 0);
		};		

		class CAllocator_HeapNoDelete : public CAllocator_Heap
		{
		public:

		};

		template <mint t_AllocSize>
		class TCAllocator_Placement
		{
			void *m_pPointer;
#ifdef DMibDebug
			bool m_bPointerTaken;
#endif

			void f_DoCheck(mint _Size);
		public:
			enum
			{
				mc_Reporting = false
				, mc_CanBeStatic = false
				, mc_bMethodsStatic = false
			};

			typedef CDefaultPointerHolder CPtrHolder;

			struct CAutoDestroy : public CAllocator_AutoDestroy
			{
				CAutoDestroy(CAutoDestroy &&) = default;
				CAutoDestroy &operator =(CAutoDestroy &&) = default;
				CAutoDestroy() = default;
				~CAutoDestroy()
				{
					if (this->m_pMemory)
						f_Free(this->m_pMemory, this->m_Size);
				}
			};

			TCAllocator_Placement(void *_pPointer);
			static bint f_IsStatic(void const *_pBlock);
			static bint f_OnlyOneAlloc();
			static mint f_StaticAddresses();
			static mint f_GranularityAlloc(bint _bLargePages = false);
			static mint f_GranularityCommit(bint _bLargePages = false);
			static mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased static mint f_Size(void *_pBlock);
			only_parameters_aliased static mint f_TrySize(void *_pBlock);
			static mint f_SizePadded(mint _Size);
			static fp32 f_Overhead(void const *_pBlock);
			static bint f_CanCommit();
			static bint f_CanProtect();
			only_parameters_aliased static void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased return_not_aliased void *f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_Alloc(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased CAutoDestroy f_AllocSafe(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased static void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased static void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased static void f_Free(void *_pBlock, mint _Size = 0);
		};		

	}

	template <typename tf_CObjectType>
	static void fg_DeleteObject(NMem::CAllocator_HeapNoDelete &_Allocator, tf_CObjectType *_pObject)
	{
		DMibFastCheck(0); // Delete not supported
	}

	namespace NMem
	{

		// Only support allocating one object
		template <mint t_StaticStorage, mint t_Alignment = TCConstantMax<mint, sizeof(void *), NTraits::TCAlignmentOf<fp64>::mc_Value>::mc_Value, typename t_CFallbackAllocator = CAllocator_Heap>
		class TCAllocator_Static : public t_CFallbackAllocator
		{
		private:
			static const mint mcp_StorageSize = (t_StaticStorage + t_Alignment - 1) & ~mint(t_Alignment - 1);
			typedef uint8 CStorage[mcp_StorageSize];
			typedef typename NTraits::TCAlign<CStorage, t_Alignment>::CType CAlignedStorage;
			CAlignedStorage m_Storage;
#if DMibEnableSafeCheck > 0
			bint m_bAllocated;
#endif

			bint fp_IsStatic(void const *_pBlock) const;
			return_not_aliased void *fp_GetStatic();
		public:
			enum
			{
				mc_CanBeStatic = true
				, mc_bMethodsStatic = false
			};
			typedef CDefaultPointerHolder CPtrHolder;

			TCAllocator_Static();
			TCAllocator_Static(TCAllocator_Static &&_Other);
			TCAllocator_Static(TCAllocator_Static const &_Other);
			TCAllocator_Static & operator =(TCAllocator_Static &&_Other);
			TCAllocator_Static & operator =(TCAllocator_Static const &_Other);
			bint f_IsStatic(void const *_pBlock) const;
			static bint f_OnlyOneAlloc();
			mint f_StaticAddresses();
			mint f_GranularityAlloc(bint _bLargePages = false);
			mint f_GranularityCommit(bint _bLargePages = false);
			mint f_GranularityProtect(bint _bLargePages = false);
			only_parameters_aliased mint f_Size(void *_pBlock);
			only_parameters_aliased mint f_TrySize(void *_pBlock);
			mint f_SizePadded(mint _Size);
			fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
			bint f_CanCommit();
			bint f_CanProtect();
			only_parameters_aliased void f_Protect(void *_pMem, mint _Size, uaint _Protect);
			only_parameters_aliased return_not_aliased void *f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_AllocDebug(const mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_Alloc(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_Alloc(const mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_AllocAligned(const mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_Realloc(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_ReallocDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_Resize(void *_pMem, mint &_Size, mint _OldSize = 0, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased return_not_aliased void *f_ResizeDebug(void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
			only_parameters_aliased void f_Commit(void *_pMem, mint _Size);
			only_parameters_aliased void f_Decommit(void *_pMem, mint _Size);
			only_parameters_aliased void f_Free(void *_pBlock, mint _Size = 0);
		};

		template <typename t_CDeleterType, typename t_CFallbackAllocator = CAllocator_Heap>
		class TCAllocator_FunctorDeleter : public t_CFallbackAllocator
		{
			t_CDeleterType m_Deleter;

		public:
			
			TCAllocator_FunctorDeleter();
			TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter const &_Other);
			TCAllocator_FunctorDeleter(TCAllocator_FunctorDeleter &&_Other);
            TCAllocator_FunctorDeleter& operator= (TCAllocator_FunctorDeleter&& _Other);
			template <typename t_CParam0>
			TCAllocator_FunctorDeleter(t_CParam0 &&_Param0);
			t_CDeleterType &f_GetDeleter();
			t_CDeleterType const &f_GetDeleter() const;
		};
	}

	template <typename tf_CObjectType, typename t_CDeleterType, typename t_CFallbackAllocator>
	static void fg_DeleteObject(NMem::TCAllocator_FunctorDeleter<t_CDeleterType, t_CFallbackAllocator> &_Allocator, tf_CObjectType *_pObject)
	{
		_Allocator.f_GetDeleter()(_pObject);
	}

}



/************************************************************************************************\
||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
|| new operators
||______________________________________________________________________________________________||
\************************************************************************************************/

#ifndef DMibDefaultToolset

// Placement new
#ifndef __PLACEMENT_NEW_INLINE
#	define __PLACEMENT_NEW_INLINE

	only_parameters_aliased return_not_aliased inline_always void * operator new (mint _Size, void * variable_not_aliased _pPlacement) noexcept
	{
		return _pPlacement;
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept
	{	
	}

#endif

#ifndef __PLACEMENT_VEC_NEW_INLINE
#	define __PLACEMENT_VEC_NEW_INLINE

	only_parameters_aliased return_not_aliased inline_always void * operator new [] (mint _Size, void * variable_not_aliased _pPlacement) noexcept
	{
		return _pPlacement;
	}

	only_parameters_aliased inline_always void operator delete [] (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept
	{	
	}

#endif

#endif


template <mint t_ArraySize>
only_parameters_aliased return_not_aliased inline_always void * operator new (mint _Size, uint8 _Placement[t_ArraySize]) noexcept
{
	void * variable_not_aliased pValue = _Placement;
	return pValue;
}

template <mint t_ArraySize>
only_parameters_aliased inline_always void operator delete (void *_pToDelete, uint8 _Placement[t_ArraySize]) noexcept
{	
}


#ifdef DMibPOverrideOperatorNew
#include <new>
	// Default new
#	if DMibPInlineActive > 0 && !defined(DMibNoInlineNew)

		only_parameters_aliased return_not_aliased inline_always void * calling_convention_c operator new (mint _Size)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}
		only_parameters_aliased return_not_aliased inline_always void * calling_convention_c operator new (mint _Size, const std::nothrow_t &) noexcept
		{
			try
			{
				return NMib::NMem::fg_Alloc(_Size);
			}
			catch (...)
			{
				return nullptr;
			}
		}
		only_parameters_aliased inline_always void calling_convention_c operator delete (void *_pToDelete) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}
		only_parameters_aliased inline_always void calling_convention_c operator delete (void *_pToDelete, const std::nothrow_t &) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}
#ifndef DCompiler_MSVC
		only_parameters_aliased return_not_aliased inline_always void *operator new[] (mint _Size)
		{
			return NMib::NMem::fg_Alloc(_Size);
		}
		only_parameters_aliased return_not_aliased inline_always void *operator new[] (mint _Size, const std::nothrow_t &) noexcept
		{
			try
			{
				return NMib::NMem::fg_Alloc(_Size);
			}
			catch (...)
			{
				return nullptr;
			}
		}
		only_parameters_aliased inline_always void operator delete[] (void* _pToDelete) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}
		only_parameters_aliased inline_always void operator delete[] (void* _pToDelete, const std::nothrow_t &) noexcept
		{
			NMib::NMem::fg_Free(_pToDelete);
		}
#endif
#	else

		only_parameters_aliased return_not_aliased void * calling_convention_c operator new (mint _Size);
		only_parameters_aliased void calling_convention_c operator delete (void *_pToDelete) noexcept;
		only_parameters_aliased return_not_aliased void * calling_convention_c operator new (mint _Size, const std::nothrow_t &) noexcept;
		only_parameters_aliased void calling_convention_c operator delete (void *_pToDelete, const std::nothrow_t &) noexcept;

#ifndef DCompiler_MSVC
		only_parameters_aliased return_not_aliased void *operator new[] (mint _Size);
		only_parameters_aliased void operator delete[] (void* _pToDelete) noexcept;
		only_parameters_aliased return_not_aliased void *operator new[] (mint _Size, const std::nothrow_t &) noexcept;
		only_parameters_aliased void operator delete[] (void* _pToDelete, const std::nothrow_t &) noexcept;
#endif
#	endif
	// Default delete
#endif


// New with align

	struct CNewAligned
	{
		mint m_Alignment;
		CNewAligned(mint _Alignment)
			: m_Alignment(_Alignment)
		{
		}
	};

	only_parameters_aliased return_not_aliased inline_always void * operator new (mint _Size, CNewAligned const& _Alignment)
	{
		return NMib::NMem::fg_AllocAligned(_Size, _Alignment.m_Alignment);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, CNewAligned const& _Alignment) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete);
	}

#if DMibConfig_MalterlibMemoryManager_Debug

	only_parameters_aliased return_not_aliased inline_always void * operator new (mint _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMem::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased return_not_aliased inline_always void * operator new[] (mint _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMem::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete);
	}

	only_parameters_aliased return_not_aliased inline_always void * operator new (mint _Size, CNewAligned const& _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMem::fg_AllocAlignedDebug(_Size, _Alignment.m_Alignment, _pFile, _Line, _Flags);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, CNewAligned const& _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMem::fg_Free(_pToDelete);
	}


#endif

