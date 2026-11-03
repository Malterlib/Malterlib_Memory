// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>

#include <Mib/Intrusive/AVLTree>

// #define DMibDebugPoolAllocations

namespace NMib::NMemory
{
	struct CPoolType_Growing
	{
		template <typename t_CAllocator>
		struct TCBlock
		{
			DMibListLinkAllocatorSA_Link(TCBlock, m_FreeLink, t_CAllocator);
		};

		template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
		struct TCChunk
		{
			using CChunk = TCChunk;
			DMibListLinkAllocatorS_Link(CChunk, m_Link, t_CAllocator);
			umint m_Size;

			static constexpr aint mc_BlockSize = t_DataSize < sizeof(TCBlock<t_CAllocator>) ? sizeof(TCBlock<t_CAllocator>) : t_DataSize;
			static constexpr aint mc_DataSize = (mc_BlockSize + t_Alignment - 1) & (~(t_Alignment - 1));

			inline_large TCChunk(DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) &_Blocks, umint _Size);
			~TCChunk();
		};

		template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
		struct TCPool
		{
			constexpr TCPool(EAggregateInitialization _Init);
			inline_always TCPool();

			using CBlock = TCBlock<t_CAllocator>;

			using CChunk = TCChunk<t_CAllocator, fg_Max(t_DataSize, sizeof(CBlock)), fg_Max(t_Alignment, alignof(CBlock))>;

			DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(CChunk, m_Link, t_CAllocator) m_Chunks;
			DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) m_FreeBlocks;
#ifdef DMibDebug
			aint m_NumUsed;
#endif
			ENumaNode m_NumaNode;
			umint m_GrowSize;

			static constexpr aint mc_DataSize = CChunk::mc_DataSize;

			void f_Construct(ENumaNode _NumaNode, umint _GrowSize);
			void f_Destruct(ch8 const *_pTypeName);
			void *f_GetBlock();
			static void fs_FreeChunk(CChunk *_pChunk);
			fp32 f_Overhead() const;
			void f_ReturnBlock(void *_pBlock);

		private:
			void fp_AddChunk(void *_pMem, umint _Size);
		};
	};

	struct CPoolType_Freeable
	{
		struct CPool;

		template <typename t_CAllocator>
		struct TCBlock
		{
			union
			{
				void *m_pChunk;
#ifndef DDocumentation_Doxygen
				DMibListLinkAllocatorSA_Member(m_FreeLink, t_CAllocator);
#endif
			};
			DMibListLinkS_Trans(TCBlock, m_FreeLink);
		};

		template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
		struct TCChunk
		{
			using CChunk = TCChunk;

			DMibListLinkAllocatorD_Link(CChunk, m_Link, t_CAllocator);
			DMibListLinkAllocatorS_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) m_FreeBlocks;
			aint m_NumUsed;
			umint m_Size;

			static constexpr aint mc_BlockSize = (sizeof(TCBlock<t_CAllocator>) + t_Alignment - 1) & (~(t_Alignment - 1));
			static constexpr aint mc_DataSize = ((mc_BlockSize + t_DataSize) + t_Alignment - 1) & (~(t_Alignment - 1));

			inline_large TCChunk(umint _Size);
			~TCChunk();
		};

		template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
		struct TCPool
		{
			constexpr TCPool(EAggregateInitialization _Init);
			inline_always TCPool();

			using CBlock = TCBlock<t_CAllocator>;

			using CChunk = TCChunk<t_CAllocator, fg_Max(t_DataSize, sizeof(CBlock)), fg_Max(t_Alignment, alignof(CBlock))>;

			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_Chunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_FreeChunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_EmptyChunks;
			ENumaNode m_NumaNode;
			umint m_GrowSize;

			static constexpr aint mc_DataSize = CChunk::mc_DataSize;
			static constexpr aint mc_BlockSize = CChunk::mc_BlockSize;

			void f_Construct(ENumaNode _NumaNode, umint _GrowSize);
			void f_Destruct(ch8 const *_pTypeName);
			bool f_ContainsBlock(void *_pBlock);
			void *f_GetBlock();
			static void fs_FreeChunk(CChunk *_pChunk);
			fp32 f_Overhead() const;
			void f_ReturnBlock(void *_pBlock);
		};
	};

	struct CPoolType_FreeableSmall
	{
		struct CPool;

		template <typename t_CAllocator>
		struct TCBlock
		{
			DMibListLinkAllocatorSA_Member(m_FreeLink, t_CAllocator);
			DMibListLinkS_Trans(TCBlock, m_FreeLink);
		};

		template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
		struct TCChunk
		{
			using CChunk = TCChunk;

			struct CCompare
			{
				inline_small void const *operator () (TCChunk const &_Node) const;
			};

			NIntrusive::TCAVLLink<> m_TreeLink;
			DMibListLinkAllocatorD_Link(CChunk, m_Link, t_CAllocator); // 2
			DMibListLinkAllocatorS_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) m_FreeBlocks; // 1
			aint m_NumUsed; // 1
			umint m_Size;

			static constexpr aint mc_BlockSize = t_DataSize < sizeof(TCBlock<t_CAllocator>) ? sizeof(TCBlock<t_CAllocator>) : t_DataSize;
			static constexpr aint mc_DataSize = (mc_BlockSize + t_Alignment - 1) & (~(t_Alignment - 1));

			inline_large TCChunk(umint _Size);
			~TCChunk();
		};

		template <typename t_CAllocator, umint t_DataSize, umint t_Alignment>
		struct TCPool
		{
			constexpr TCPool(EAggregateInitialization _Init);
			inline_always TCPool();

			using CBlock = TCBlock<t_CAllocator>;
			using CChunk = TCChunk<t_CAllocator, fg_Max(t_DataSize, sizeof(CBlock)), fg_Max(t_Alignment, alignof(CBlock))>;

			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_Chunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_FreeChunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_EmptyChunks;

			NIntrusive::TCAVLTree<&CChunk::m_TreeLink, typename CChunk::CCompare> m_ChunkTree;

			ENumaNode m_NumaNode;
			umint m_GrowSize;

			static constexpr aint mc_DataSize = CChunk::mc_DataSize;

			void f_Construct(ENumaNode _NumaNode, umint _GrowSize);
			void f_Destruct(ch8 const *_pTypeName);
			bool f_ContainsBlock(void *_pBlock);
			fp32 f_Overhead() const;
			void *f_GetBlock();
			static void fs_FreeChunk(CChunk *_pChunk);
			void f_ReturnBlock(void *_pBlock);
		};
	};

	template <typename t_CLock>
	struct TCPoolAggregateData : public t_CLock
	{
		constexpr TCPoolAggregateData(EAggregateInitialization _Init);
		inline_always TCPoolAggregateData();

		umint m_bDoneInit;

		inline_small void f_Construct();
		inline_small void f_InitDone();
		inline_small bool f_ShouldInit();
	};

	template
	<
		typename t_CData
		, aint t_GrowSize = 128
		, typename t_CLock = NThread::CNoLock
		, typename t_CPoolType = CPoolType_Freeable
		, typename t_CAllocator = CAllocator_Virtual
		, typename t_CAggregateData = TCPoolAggregateData<t_CLock>
		, typename t_CDebugStrType = NStr::CFStr256
	>
	struct TCPoolAggregate : public t_CAggregateData
	{
		constexpr TCPoolAggregate(EAggregateInitialization _Init);
		inline_always TCPoolAggregate();

#if DMibConfig_Memory_Shims_Enable
		t_CDebugStrType m_DebugName;
#endif

	public:
		static constexpr aint mc_Reporting = t_CAllocator::mc_Reporting;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = false;

		constexpr bool operator == (TCPoolAggregate const &_Right) const noexcept;
		constexpr auto operator <=> (TCPoolAggregate const &_Right) const noexcept;

		typename t_CPoolType::template TCPool<t_CAllocator, sizeof(t_CData), alignof(t_CData)> m_Pool;
		using CData = t_CData;

		void f_Construct(ENumaNode _NumaNode);
		void f_Destruct();
		inline_small fp32 f_Overhead(void const *_pBlock);
		inline_small void *f_GetBlock();
		inline_small void f_ReturnBlock(void *_pToDelete);
		inline_small void f_Delete(t_CData *_pToDelete);
		template <typename ...tfp_CParams>
		t_CData *f_New(tfp_CParams && ...p_Params);

	private:
		inline_small void fp_ReturnBlock(void *_pBlock);
		inline_medium void *fp_GetBlock();
	};

#define DMibMemPoolStaticImpl(d_PoolType, d_PoolName) d_PoolType d_PoolName = {0};

#ifndef DMibPNoShortCuts
#	define DPoolStaticImpl(d_PoolType, d_PoolName) DMibMemPoolStaticImpl(d_PoolType, d_PoolName)
#endif

	template <typename t_CLock>
	struct TCNoAggregateData : public t_CLock
	{
		inline_small void f_InitDone();
		inline_small bool f_ShouldInit();
	};

	template <typename t_CData, umint t_GrowSize = 128, typename t_CLock = NMib::NThread::CNoLock, typename t_CPoolType = CPoolType_Freeable, typename t_CAllocator = CAllocator_Virtual>
	struct TCPool : public TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, TCNoAggregateData<t_CLock>>
	{
		TCPool(ENumaNode _NumaNode = ENumaNode_Default);
		~TCPool();
	};

	template <typename t_CData, umint t_GrowSize = 128, typename t_CLock = NMib::NThread::CNoLock, typename t_CAllocator = CAllocator_Virtual>
	struct TCPoolSmall : public TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_FreeableSmall, t_CAllocator, TCNoAggregateData<t_CLock>>
	{
		TCPoolSmall(ENumaNode _NumaNode = ENumaNode_Default);
		~TCPoolSmall();
	};

	template <typename t_CData, umint t_GrowSize = 128, typename t_CLock = NMib::NThread::CNoLock, typename t_CAllocator = CAllocator_Virtual>
	struct TCPoolGrowing : public TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_Growing, t_CAllocator, TCNoAggregateData<t_CLock>>
	{
		TCPoolGrowing(ENumaNode _NumaNode = ENumaNode_Default);
		~TCPoolGrowing();
	};

	/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*\
	|	TemplateClass:		Base class for enabling pooling functionality			|
	|																				|
	|	Parameters:																	|
	|		t_CData:		The class that is inheriting from this class			|
	|																				|
	|	Comments:			Make sure that you inherit from this class				|
	|						first, otherwise the delete wont work					|
	\*_____________________________________________________________________________*/
	template <typename t_CPool>
	struct TCBCPool
	{
		void *operator new (umint _NumBytes, t_CPool const &_Pool);
		void operator delete (void *_pToDelete, t_CPool const &_Pool) noexcept;
		void operator delete (void *_pToDelete);
	};

	template
	<
		typename t_CType
		, umint t_GrowSize = 128
		, typename t_CAllocator = NMib::NMemory::CAllocator_Virtual
		, typename t_CPoolType = NMib::NMemory::CPoolType_FreeableSmall
		, typename t_CLockType = typename NMib::NThread::CMutual
	>
	struct TCStaticPool;

	template
	<
		typename t_CType
		, umint t_GrowSize = 128
		, typename t_CAllocator = NMib::NMemory::CAllocator_Virtual
		, typename t_CPoolType = NMib::NMemory::CPoolType_FreeableSmall
		, typename t_CLockType = typename NMib::NThread::CMutual
	>
	struct TCStaticPoolAllocator : public CAllocator_Base
	{
		static constexpr aint mc_Reporting = t_CAllocator::mc_Reporting;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = true;

		constexpr bool operator == (TCStaticPoolAllocator const &_Right) const noexcept;
		constexpr auto operator <=> (TCStaticPoolAllocator const &_Right) const noexcept;

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<TCStaticPoolAllocator>;
		using CStaticPool = TCStaticPool<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>;

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static inline_small umint f_StaticAddresses();
		static inline_small umint f_GranularityAlloc(bool _bLargePages = false);
		static inline_small umint f_GranularityCommit(bool _bLargePages = false);
		static inline_small umint f_GranularityProtect(bool _bLargePages = false);
		static inline_small umint f_Size(void *_pBlock);
		static inline_small umint f_TrySize(void *_pBlock);
		static inline_small umint f_SizePadded(umint _Size);
		static constexpr inline_small bool f_CanCommit();
		static inline_small bool f_CanProtect();
		static inline_small bool f_DeterministicSize();
		static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect);
		static inline_small auto f_AllocWithSizeDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static inline_small void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static inline_small void *f_AllocAlignedWithSize(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static inline_small auto f_AllocDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static inline_small void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static inline_small void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		static inline_small void *f_Realloc(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static inline_small auto f_ReallocDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static inline_small void *f_Resize(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static inline_small auto f_ResizeDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		static inline_small void f_Commit(void *_pMem, umint _Size);
		static inline_small void f_Decommit(void *_pMem, umint _Size);
		static inline_small void f_Free(void *_pBlock, umint _Size);
		static inline_small void f_FreeNoSize(void *_pBlock);
		static inline_small fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		only_parameters_aliased static auto f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
			-> CAutoDestroy
		;
		only_parameters_aliased static CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		static CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);

		// NOLINTEND(malterlib-function-naming)
	};

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	struct TCStaticPool
	{
		using CPoolType = TCPool<t_CType, t_GrowSize, t_CLockType, t_CPoolType, t_CAllocator>;

		static NStorage::TCAggregate<CPoolType, 16> ms_Pool;

		using CAllocator = TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>;
	};

	template <typename t_CType, umint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	constinit NStorage::TCAggregate<typename TCStaticPool<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::CPoolType, 16>
		TCStaticPool<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::ms_Pool = {DAggregateInit}
	;

	template <typename tf_CType>
	void fg_StaticPoolDelete(tf_CType *_pToDelete);
	template <typename tf_CType, typename ...tfp_CParams>
	void fg_StaticPoolNew(tf_CType * &_pDestPtr, tfp_CParams && ...p_Params);

	template
	<
		typename t_CType
		, umint t_GrowSize = 128
		, typename t_CAllocator = NMib::NMemory::CAllocator_Virtual
		, typename t_CPoolType = NMib::NMemory::CPoolType_FreeableSmall
		, typename t_CLockType = NMib::NThread::CNoLock
	>
	struct TCPoolAllocator : public CAllocator_Base
	{
		static constexpr aint mc_Reporting = t_CAllocator::mc_Reporting;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = false;

		constexpr bool operator == (TCPoolAllocator const &_Right) const noexcept;
		constexpr auto operator <=> (TCPoolAllocator const &_Right) const noexcept;

		using CAutoDestroy = TCAllocator_AutoDestroy<TCPoolAllocator>;

		TCPool<t_CType, t_GrowSize, t_CLockType, t_CPoolType, t_CAllocator> m_Pool;

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static inline_small umint f_StaticAddresses();
		static inline_small umint f_GranularityAlloc(bool _bLargePages = false);
		static inline_small umint f_GranularityCommit(bool _bLargePages = false);
		static inline_small umint f_GranularityProtect(bool _bLargePages = false);
		static inline_small umint f_Size(void *_pBlock);
		static inline_small umint f_TrySize(void *_pBlock);
		static inline_small umint f_SizePadded(umint _Size);
		static constexpr inline_small bool f_CanCommit();
		static inline_small bool f_CanProtect();
		static inline_small bool f_DeterministicSize();
		static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect);

		// NOLINTEND(malterlib-function-naming)

		inline_small auto f_AllocWithSizeDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_small void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small void *f_AllocAlignedWithSize(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small auto f_AllocDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased inline_small void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		inline_small void *f_Realloc(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small auto f_ReallocDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_small void *f_Resize(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small auto f_ResizeDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static inline_small void f_Commit(void *_pMem, umint _Size);
		static inline_small void f_Decommit(void *_pMem, umint _Size);

		// NOLINTEND(malterlib-function-naming)

		inline_small void f_Free(void *_pBlock, umint _Size);
		inline_small void f_FreeNoSize(void *_pBlock);
		inline_small fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
	};

	template <typename t_CPoolType>
	struct TCPoolReferenceAllocator : public CAllocator_Base
	{
		static constexpr aint mc_Reporting = t_CPoolType::mc_Reporting;
		static constexpr aint mc_CanBeStatic = false;
		static constexpr bool mc_bMethodsStatic = false;

		constexpr bool operator == (TCPoolReferenceAllocator const &_Right) const noexcept;
		constexpr auto operator <=> (TCPoolReferenceAllocator const &_Right) const noexcept;

		using CData = typename t_CPoolType::CData;
		using CAutoDestroy = TCAllocator_AutoDestroy<TCPoolReferenceAllocator>;

		t_CPoolType &m_Pool;

		TCPoolReferenceAllocator(t_CPoolType &_Pool);

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static inline_small umint f_StaticAddresses();
		static inline_small umint f_GranularityAlloc(bool _bLargePages = false);
		static inline_small umint f_GranularityCommit(bool _bLargePages = false);
		static inline_small umint f_GranularityProtect(bool _bLargePages = false);
		static inline_small umint f_Size(void *_pBlock);
		static inline_small umint f_TrySize(void *_pBlock);
		static inline_small umint f_SizePadded(umint _Size);
		static constexpr inline_small bool f_CanCommit();
		static inline_small bool f_CanProtect();
		static inline_small bool f_DeterministicSize();
		static inline_small void f_Protect(void *_pMem, umint _Size, uaint _Protect);

		// NOLINTEND(malterlib-function-naming)

		inline_small auto f_AllocWithSizeDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_small void *f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small void *f_AllocAlignedWithSize(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small auto f_AllocDebug
			(
				umint _Size
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_small void *f_Alloc(umint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small void *f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased inline_small void f_AllocBatch
			(
				umint _Size
				, umint _Alignment
				, NFunction::TCFunctionNoAlloc<bool (void *_pAlloc, umint _Size)> const &_fFunctor
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
		;
		inline_small void *f_Realloc(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small auto f_ReallocDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;
		inline_small void *f_Resize(void *_pMem, umint _Size, umint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		inline_small auto f_ResizeDebug
			(
				void *_pMem
				, umint _Size
				, umint _OldSize
				, ch8 const *_pFile
				, aint _Line
				, EHeapDebugFlag _Flags = EHeapDebugFlag_None
				, EAllocationFlag _AllocFlags = EAllocationFlag_None
				, ENumaNode _NumaNode = ENumaNode_Default
			)
			-> void *
		;

		// NOLINTBEGIN(malterlib-function-naming): the allocator interface, static in stateless allocators and not in the others
		static inline_small void f_Commit(void *_pMem, umint _Size);
		static inline_small void f_Decommit(void *_pMem, umint _Size);

		// NOLINTEND(malterlib-function-naming)

		inline_small void f_Free(void *_pBlock, umint _Size);
		inline_small void f_FreeNoSize(void *_pBlock);
		inline_small fp32 f_Overhead(void const *_pBlock); // Number of bytes overhead for block
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		only_parameters_aliased CAutoDestroy f_AllocSafe(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default);
		CAutoDestroy f_MakeSafe(void *_pMemory, umint _Size);
	};
}

#include "Malterlib_Memory_Pool.hpp"
