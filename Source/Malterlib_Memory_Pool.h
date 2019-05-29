// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#include <Mib/Intrusive/AVLTree>

// #define DMibDebugPoolAllocations

namespace NMib::NMemory
{
	class CPoolType_Growing
	{
	public:

		template <typename t_CAllocator>
		class TCBlock
		{
		public:
			DMibListLinkAllocatorSA_Link(TCBlock, m_FreeLink, t_CAllocator);
		};

		template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
		class TCChunk
		{
		public:
			typedef TCChunk CChunk;
			DMibListLinkAllocatorS_Link(CChunk, m_Link, t_CAllocator);
			mint m_Size;

			enum
			{
				EBlockSize = t_DataSize < sizeof(TCBlock<t_CAllocator>) ? sizeof(TCBlock<t_CAllocator>) : t_DataSize
				, EDataSize = (EBlockSize + t_Alignment - 1) & (~(t_Alignment - 1))
			};

			inline_large TCChunk(DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) &_Blocks, mint _Size)
			{
				m_Size = _Size;

				uint8 *pStart = fg_AlignUp((uint8 *)(this + 1), t_Alignment);
				uint8 *pEnd = (uint8 *)this + _Size;
				mint nBlocks = (pEnd - pStart) / EDataSize;

				uint8 *pEndBlock = pStart;
				uint8 *pCurrentBlock = pStart + (nBlocks - 1) * EDataSize;

				while (pCurrentBlock >= pEndBlock)
				{
					TCBlock<t_CAllocator> *pToInsert = (TCBlock<t_CAllocator> *)pCurrentBlock;
					_Blocks.f_UnsafeInsertFirst(pToInsert);
					pCurrentBlock -= EDataSize;
				}
			}

			~TCChunk()
			{
			}
		};


		template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
		class TCPool
		{
		public:
#ifndef DMibNoAggregateConstexpr
			constexpr TCPool(EAggregateInitialization _Init)
				: m_Chunks(_Init)
				, m_FreeBlocks(_Init)
#ifdef DMibDebug
				, m_NumUsed(0)
#endif
				, m_NumaNode(ENumaNode_Default)
				, m_GrowSize(0)
			{
			}
			inline_always TCPool()
			{
			}
#endif
			typedef TCChunk<t_CAllocator, t_DataSize, t_Alignment> CChunk;

			DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(CChunk, m_Link, t_CAllocator) m_Chunks;
			DMibListLinkAllocatorSA_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) m_FreeBlocks;
#				ifdef DMibDebug
				aint m_NumUsed;
#				endif
			ENumaNode m_NumaNode;
			mint m_GrowSize;

			enum
			{
				EDataSize = CChunk::EDataSize
			};

			void f_Construct(ENumaNode _NumaNode, mint _GrowSize)
			{
				m_NumaNode = _NumaNode;
				m_GrowSize = _GrowSize;
				m_Chunks.f_Construct();
				m_FreeBlocks.f_Construct();
#					ifdef DMibDebug
					m_NumUsed = 0;
#					endif
			}

			void f_Destruct(ch8 const *_pTypeName);

			void fp_AddChunk(void *_pMem, mint _Size)
			{
				CChunk *pNewChunk = new(_pMem) CChunk(m_FreeBlocks, _Size);
				m_Chunks.f_Insert(pNewChunk);
			}

			void *f_GetBlock()
			{
				TCBlock<t_CAllocator> * pBlock = m_FreeBlocks.f_Pop();

				if (pBlock)
				{
#						ifdef DMibDebug
						++m_NumUsed;
#						endif
					return pBlock;
				}
				else
				{

					mint Size = ((((sizeof(CChunk) + m_GrowSize * EDataSize) - 1) / t_CAllocator::f_GranularityAlloc()) + 1) * t_CAllocator::f_GranularityAlloc();
					void *pMem = t_CAllocator::f_AllocWithSize(Size, EAllocationFlag_WillFreeWithSize, m_NumaNode);

					if (pMem)
					{
						fp_AddChunk(pMem, Size);
						pBlock = m_FreeBlocks.f_Pop();
						DMibFastCheck(pBlock); // Must succed
#							ifdef DMibDebug
							++m_NumUsed;
#							endif
						return pBlock;
					}
					else
					{
						return nullptr;
					}
				}
			}

			static void fs_FreeChunk(CChunk *_pChunk)
			{
				t_CAllocator::f_Free(_pChunk, _pChunk->m_Size);
			}

			fp32 f_Overhead() const
			{
				if (EDataSize > t_DataSize)
					return fp32(EDataSize - t_DataSize);
				else
					return 0.0f;
			}

			void f_ReturnBlock(void *_pBlock)
			{
				TCBlock<t_CAllocator> *pBlock = (TCBlock<t_CAllocator> *)_pBlock;
				// We are adding to a full block
				m_FreeBlocks.f_UnsafePush(pBlock);
#					ifdef DMibDebug
					--m_NumUsed;
#					endif
			}

		};

	};

	class CPoolType_Freeable
	{
	public:

		class CPool;

		template <typename t_CAllocator>
		class TCBlock
		{
		public:
			union
			{
				void *m_pChunk;
#ifndef DDocumentation_Doxygen
				DMibListLinkAllocatorSA_Member(m_FreeLink, t_CAllocator);
#endif
			};
			DMibListLinkS_Trans(TCBlock, m_FreeLink);
		};

		template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
		class TCChunk
		{
		public:
			typedef TCChunk CChunk;
			DMibListLinkAllocatorD_Link(CChunk, m_Link, t_CAllocator);
			DMibListLinkAllocatorS_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) m_FreeBlocks;
			aint m_NumUsed;
			mint m_Size;

			enum
			{
				EBlockSize = (sizeof(TCBlock<t_CAllocator>) + t_Alignment - 1) & (~(t_Alignment - 1))
				, EDataSize = ((EBlockSize + t_DataSize) + t_Alignment - 1) & (~(t_Alignment - 1))
			};

			inline_large TCChunk(mint _Size)
			{

				m_Size = _Size;
				m_NumUsed = 0;

				uint8 *pStart = fg_AlignUp((uint8 *)(this + 1), t_Alignment);
				uint8 *pEnd = (uint8 *)this + _Size;
				mint nBlocks = (pEnd - pStart) / EDataSize;

				uint8 *pEndBlock = pStart;
				uint8 *pCurrentBlock = pStart + (nBlocks - 1) * EDataSize;

				while (pCurrentBlock >= pEndBlock)
				{
					TCBlock<t_CAllocator> *pToInsert = (TCBlock<t_CAllocator> *)pCurrentBlock;
					m_FreeBlocks.f_UnsafeInsertFirst(pToInsert);
					pCurrentBlock -= EDataSize;
				}

			}

			~TCChunk()
			{
				//DMibFastCheck(m_NumUsed == 0); // Memory leak in pool!
			}
		};


		template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
		class TCPool
		{
		public:
#ifndef DMibNoAggregateConstexpr
			constexpr TCPool(EAggregateInitialization _Init)
				: m_Chunks(_Init)
				, m_FreeChunks(_Init)
				, m_EmptyChunks(_Init)
				, m_NumaNode(ENumaNode_Default)
				, m_GrowSize(0)
			{
			}
			inline_always TCPool()
			{
			}
#endif
			typedef TCChunk<t_CAllocator, t_DataSize, t_Alignment> CChunk;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_Chunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_FreeChunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_EmptyChunks;
			ENumaNode m_NumaNode;
			mint m_GrowSize;

			enum
			{
				EDataSize = CChunk::EDataSize
				, EBlockSize = CChunk::EBlockSize
			};

			void f_Construct(ENumaNode _NumaNode, mint _GrowSize)
			{
				m_NumaNode = _NumaNode;
				m_GrowSize = _GrowSize;
				m_Chunks.f_Construct();
				m_FreeChunks.f_Construct();
				m_EmptyChunks.f_Construct();
			}

			void f_Destruct(ch8 const *_pTypeName);

			bool f_ContainsBlock(void *_pBlock)
			{
				DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *lLists[] = {&m_Chunks, &m_FreeChunks, &m_EmptyChunks};

				for (mint i = 0; i < sizeof(lLists) / sizeof(DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *); ++i)
				{
					DMibListLinkAllocatorD_Iter_FromTemplate(CChunk, m_Link, t_CAllocator) Iter = *lLists[i];
					while (Iter)
					{
						if ((mint)_pBlock >= (mint)(CChunk *)Iter && (mint)_pBlock < (mint)(CChunk *)Iter + Iter->m_Size)
							return true;
						++Iter;
					}
				}

				return false;
			}

			void *f_GetBlock()
			{
//					DMibListLinkD_Iter(TCChunk<>, m_Link) Iter = m_FreeChunks;

				CChunk *pChunk = m_FreeChunks.f_GetFirst();

				TCBlock<t_CAllocator> * pBlock = nullptr;

				if (pChunk)
					pBlock = pChunk->m_FreeBlocks.f_Pop();
				else
				{
					pChunk = m_EmptyChunks.f_GetFirst();
					if (pChunk)
						pBlock = pChunk->m_FreeBlocks.f_Pop();
				}

				if (pBlock)
				{
					if (pChunk->m_FreeBlocks.f_IsEmpty())
					{
						// Full block
						m_Chunks.f_Insert(pChunk);
					}
					else if (pChunk->m_NumUsed == 0)
					{
						m_FreeChunks.f_Insert(pChunk);
					}
					++pChunk->m_NumUsed;
					pBlock->m_pChunk = pChunk;
					return ((uint8*)pBlock) + EBlockSize;
				}
				else
				{
					mint Size = ((((sizeof(CChunk) + m_GrowSize * EDataSize) - 1) / t_CAllocator::f_GranularityAlloc()) + 1) * t_CAllocator::f_GranularityAlloc();
					void *pMem = t_CAllocator::f_AllocWithSize(Size, EAllocationFlag_WillFreeWithSize, m_NumaNode);

					if (pMem)
					{
						CChunk *pNewChunk = new(pMem) CChunk(Size);

						pBlock = pNewChunk->m_FreeBlocks.f_Pop();
						DMibFastCheck(pBlock); // Must succed
						m_FreeChunks.f_Insert(pNewChunk);

						++pNewChunk->m_NumUsed;
						pBlock->m_pChunk = pNewChunk;
						return ((uint8*)pBlock) + EBlockSize;
					}
					else
					{
						return nullptr;
					}
				}
			}

			static void fs_FreeChunk(CChunk *_pChunk)
			{
				t_CAllocator::f_Free(_pChunk, _pChunk->m_Size);
			}


			fp32 f_Overhead() const
			{
				return fp32(EBlockSize);
			}

			void f_ReturnBlock(void *_pBlock)
			{
				// Too slow to check this
//					DMibFastCheck(f_ContainsBlock(_pBlock), "Must be part of pool");
				TCBlock<t_CAllocator> *pBlock = (TCBlock<t_CAllocator> *)((uint8 *)_pBlock - EBlockSize);
				CChunk *pChunk = ((CChunk *)(pBlock->m_pChunk));
				if (pChunk->m_FreeBlocks.f_IsEmpty())
				{
					// We are adding to a full block
					pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
					--pChunk->m_NumUsed;
					m_FreeChunks.f_InsertFirst(pChunk);
				}
				else
				{
					pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
					--pChunk->m_NumUsed;

					if (pChunk->m_NumUsed == 0)
					{
						if (!m_EmptyChunks.f_IsEmpty())
						{
							// Remove the excess chunk
							CChunk *pChunkToDelete = m_EmptyChunks.f_Pop();


							pChunkToDelete->~CChunk();
							fs_FreeChunk(pChunkToDelete);
						}
						m_EmptyChunks.f_Insert(pChunk);
					}
				}
			}

		};

	};

	class CPoolType_FreeableSmall
	{
	public:

		class CPool;

		template <typename t_CAllocator>
		class TCBlock
		{
		public:
			DMibListLinkAllocatorSA_Member(m_FreeLink, t_CAllocator);
			DMibListLinkS_Trans(TCBlock, m_FreeLink);
		};

		template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
		class TCChunk
		{
		public:
			typedef TCChunk CChunk;

			class CCompare
			{
			public:
				inline_small void const *operator () (TCChunk const &_Node) const
				{
					return (void const *)(&_Node);
				}
			};

			NIntrusive::TCAVLLink<> m_TreeLink;
			DMibListLinkAllocatorD_Link(CChunk, m_Link, t_CAllocator); // 2
			DMibListLinkAllocatorS_ListNoLastPtr_FromTemplate(TCBlock<t_CAllocator>, m_FreeLink, t_CAllocator) m_FreeBlocks; // 1
			aint m_NumUsed; // 1
			mint m_Size;

			enum
			{
				EBlockSize = t_DataSize < sizeof(TCBlock<t_CAllocator>) ? sizeof(TCBlock<t_CAllocator>) : t_DataSize
				, EDataSize = (EBlockSize + t_Alignment - 1) & (~(t_Alignment - 1))
			};

			inline_large TCChunk(mint _Size)
			{
				m_Size = _Size;
				m_NumUsed = 0;

				uint8 *pStart = fg_AlignUp((uint8 *)(this + 1), t_Alignment);
				uint8 *pEnd = (uint8 *)this + _Size;
				mint nBlocks = (pEnd - pStart) / EDataSize;

				uint8 *pEndBlock = pStart;
				uint8 *pCurrentBlock = pStart + (nBlocks - 1) * EDataSize;

				while (pCurrentBlock >= pEndBlock)
				{
					TCBlock<t_CAllocator> *pToInsert = (TCBlock<t_CAllocator> *)pCurrentBlock;
					m_FreeBlocks.f_UnsafeInsertFirst(pToInsert);
					pCurrentBlock -= EDataSize;
				}

			}

			~TCChunk()
			{
				//DMibFastCheck(m_NumUsed == 0); // Memory leak in pool!
			}
		};


		template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
		class TCPool
		{
		public:
#ifndef DMibNoAggregateConstexpr
			constexpr TCPool(EAggregateInitialization _Init)
				: m_Chunks(_Init)
				, m_FreeChunks(_Init)
				, m_EmptyChunks(_Init)
				, m_ChunkTree(_Init)
				, m_NumaNode(ENumaNode_Default)
				, m_GrowSize(0)
			{
			}
			inline_always TCPool()
			{
			}
#endif
			typedef TCChunk<t_CAllocator, t_DataSize, t_Alignment> CChunk;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_Chunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_FreeChunks;
			DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) m_EmptyChunks;

			NIntrusive::TCAVLTree<&CChunk::m_TreeLink, typename CChunk::CCompare> m_ChunkTree;

			ENumaNode m_NumaNode;
			mint m_GrowSize;

			enum
			{
				EDataSize = CChunk::EDataSize
			};

			void f_Construct(ENumaNode _NumaNode, mint _GrowSize)
			{
				m_NumaNode = _NumaNode;
				m_GrowSize = _GrowSize;
				m_Chunks.f_Construct();
				m_FreeChunks.f_Construct();
				m_EmptyChunks.f_Construct();
			}

			void f_Destruct(ch8 const *_pTypeName);

			bool f_ContainsBlock(void *_pBlock)
			{
				DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *lLists[] = {&m_Chunks, &m_FreeChunks, &m_EmptyChunks};

				for (mint i = 0; i < sizeof(lLists) / sizeof(DMibListLinkAllocatorDA_List_FromTemplate(CChunk, m_Link, t_CAllocator) *); ++i)
				{
					DMibListLinkAllocatorD_Iter_FromTemplate(CChunk, m_Link, t_CAllocator) Iter = *lLists[i];
					while (Iter)
					{
						if ((mint)_pBlock >= (mint)(CChunk *)Iter && (mint)_pBlock < (mint)(CChunk *)Iter + Iter->m_Size)
							return true;
						++Iter;
					}
				}

				return false;
			}

			fp32 f_Overhead() const
			{
				if (EDataSize > t_DataSize)
					return fp32(EDataSize - t_DataSize);
				else
					return 0.0f;
			}

			void *f_GetBlock()
			{
	//					DMibListLinkD_Iter(TCChunk<>, m_Link) Iter = m_FreeChunks;

				CChunk *pChunk = m_FreeChunks.f_GetFirst();

				TCBlock<t_CAllocator> * pBlock = nullptr;

				if (pChunk)
				{
					pBlock = pChunk->m_FreeBlocks.f_Pop();
				}

				if (!pBlock)
				{
					pChunk = m_EmptyChunks.f_GetFirst();
					if (pChunk)
					{
						pBlock = pChunk->m_FreeBlocks.f_Pop();
					}
				}

				if (pBlock)
				{
					if (pChunk->m_FreeBlocks.f_IsEmpty())
					{
						// Full block
						m_Chunks.f_Insert(pChunk);
					}
					else if (pChunk->m_NumUsed == 0)
					{
						m_FreeChunks.f_Insert(pChunk);
					}
					++pChunk->m_NumUsed;
					return pBlock;
				}
				else
				{
					mint Size = ((((sizeof(CChunk) + m_GrowSize * EDataSize) - 1) / t_CAllocator::f_GranularityAlloc()) + 1) * t_CAllocator::f_GranularityAlloc();
					void *pMem = t_CAllocator::f_AllocWithSize(Size, EAllocationFlag_WillFreeWithSize, m_NumaNode);

					if (pMem)
					{
						CChunk *pNewChunk = new(pMem) CChunk(Size);

						m_ChunkTree.f_Insert(pNewChunk);

						pBlock = pNewChunk->m_FreeBlocks.f_Pop();
						DMibFastCheck(pBlock); // Must succed
						m_FreeChunks.f_Insert(pNewChunk);

						++pNewChunk->m_NumUsed;

						return pBlock;
					}
					else
					{
						return nullptr;
					}
				}
			}

			static void fs_FreeChunk(CChunk *_pChunk)
			{
				t_CAllocator::f_Free(_pChunk, _pChunk->m_Size);
			}

			void f_ReturnBlock(void *_pBlock)
			{
				TCBlock<t_CAllocator> *pBlock = (TCBlock<t_CAllocator> *)_pBlock;
				CChunk *pChunk = m_ChunkTree.f_FindLargestLessThanEqual(_pBlock);
				DMibFastCheck(pChunk && (mint)_pBlock < (mint)pChunk + pChunk->m_Size); // Must be part of pool
				if (pChunk->m_FreeBlocks.f_IsEmpty())
				{
					// We are adding to a full block
					pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
					--pChunk->m_NumUsed;
					m_FreeChunks.f_InsertFirst(pChunk);
				}
				else
				{
					pChunk->m_FreeBlocks.f_UnsafePush(pBlock);
					--pChunk->m_NumUsed;

					if (pChunk->m_NumUsed == 0)
					{
						if (!m_EmptyChunks.f_IsEmpty())
						{
							// Remove the excess chunk
							CChunk *pChunkToDelete = m_EmptyChunks.f_Pop();
							m_ChunkTree.f_Remove(pChunkToDelete);
							pChunkToDelete->~CChunk();
							fs_FreeChunk(pChunkToDelete);
						}
						m_EmptyChunks.f_Insert(pChunk);
					}
				}
			}

		};

	};


	template <typename t_CLock>
	class TCPoolAggregateData : public t_CLock
	{
	public:
#ifndef DMibNoAggregateConstexpr
		constexpr TCPoolAggregateData(EAggregateInitialization _Init)
			: t_CLock(_Init)
		{
		}
		inline_always TCPoolAggregateData()
		{
		}
#endif

		mint m_bDoneInit;

		inline_small void f_InitDone()
		{
			m_bDoneInit = true;
		}

		inline_small bool f_ShouldInit()
		{
			if (m_bDoneInit)
				return false;
			else
				return true;
		}
	};

	template <typename t_CData, aint t_GrowSize = 128, typename t_CLock = NThread::CNoLock, typename t_CPoolType = CPoolType_Freeable, typename t_CAllocator = CAllocator_Virtual, typename t_CAggregateData = TCPoolAggregateData<t_CLock>, typename t_CDebugStrType = NStr::CFStrAggregate256>
	class TCPoolAggregate : public t_CAggregateData
	{
	public:
#ifndef DMibNoAggregateConstexpr
		constexpr TCPoolAggregate(EAggregateInitialization _Init)
			: t_CAggregateData(_Init)
#		if DMibConfig_Memory_Shims_Enable
			, m_DebugName(_Init)
#endif
			, m_Pool(_Init)
		{
		}
		inline_always TCPoolAggregate()
		{
		}
#endif
		inline_small void fp_ReturnBlock(void * _pBlock)
		{
			DMibMemoryGoingToReportScope(this, mc_Reporting);
			{
				DMibLockTyped(t_CLock, *this);
				m_Pool.f_ReturnBlock(_pBlock);
			}
#if DMibConfig_Memory_Shims_Enable
			if (mc_Reporting)
				DMibMemoryReportFree(this, m_DebugName.f_GetStr(), _pBlock, sizeof(t_CData), nullptr);
#endif
		}

		inline_medium void *fp_GetBlock()
		{
			DMibMemoryGoingToReportScope(this, mc_Reporting);

			DMibMemoryReportExpression(fp32 Overhead);
			void *pBlock;
			{
				DMibLockTyped(t_CLock, *this);
				if (t_CAggregateData::f_ShouldInit())
					f_Construct(ENumaNode_Default);

				pBlock = m_Pool.f_GetBlock();
				DMibFastCheck(pBlock); // Memory error ??
				DMibMemoryReportExpression(Overhead = m_Pool.f_Overhead());
			}

#if DMibConfig_Memory_Shims_Enable
			if (mc_Reporting)
				DMibMemoryReportAlloc(this, m_DebugName.f_GetStr(), pBlock, 0, sizeof(t_CData), sizeof(t_CData), Overhead, nullptr);
#endif
			return pBlock;
		}

#		if DMibConfig_Memory_Shims_Enable
		t_CDebugStrType m_DebugName;
#		endif

	public:
		enum
		{
			mc_Reporting = t_CAllocator::mc_Reporting
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = false
		};

		typename t_CPoolType::template TCPool<t_CAllocator, sizeof(t_CData), NTraits::TCAlignmentOf<t_CData>::mc_Value> m_Pool;
		typedef t_CData CData;

		void f_Construct(ENumaNode _NumaNode);

		void f_Destruct()
		{
			DMibMemoryGoingToReportScope(this, mc_Reporting);
			if (!t_CAggregateData::f_ShouldInit())
			{
#ifdef DMibDebug
				m_Pool.f_Destruct(fg_GetTypeName<t_CData>());
#else
				m_Pool.f_Destruct("Unknown");
#endif
			}
			t_CAggregateData::f_Destruct();
#if DMibConfig_Memory_Shims_Enable
			if (mc_Reporting)
				DMibMemoryReportAllocatorDelete(this, m_DebugName.f_GetStr());
#endif
		}

		inline_small fp32 f_Overhead(void const *_pBlock)
		{
			DMibLockTyped(t_CLock, *this);
			return m_Pool.f_Overhead();
		}

		inline_small void *f_GetBlock()
		{
			return fp_GetBlock();
		}


		inline_small void f_ReturnBlock(void *_pToDelete)
		{
			fp_ReturnBlock(_pToDelete);
		}

		inline_small void f_Delete(t_CData *_pToDelete)
		{
			_pToDelete->~t_CData();
			fp_ReturnBlock(_pToDelete);
		}

		template <typename... tfp_CParams>
		t_CData *f_New(tfp_CParams &&... p_Params)
		{
			void *pMem;
			pMem = fp_GetBlock();
			t_CData *pObject = new(pMem) t_CData(fg_Forward<tfp_CParams>(p_Params)...);
			return pObject;
		}
	};

#		define DMibMemPoolStaticImpl(_PoolType, _PoolName) _PoolType _PoolName = {0};

#		ifndef DMibPNoShortCuts
#			define DPoolStaticImpl(_PoolType, _PoolName) DMibMemPoolStaticImpl(_PoolType, _PoolName)
#		endif

	template <typename t_CLock>
	class TCNoAggregateData : public t_CLock
	{
	public:
		static inline_small void f_InitDone()
		{
		}

		static inline_small bool f_ShouldInit()
		{
			return false;
		}
	};

	template <typename t_CData, mint t_GrowSize = 128, typename t_CLock = NMib::NThread::CNoLock, typename t_CPoolType = CPoolType_Freeable, typename t_CAllocator = CAllocator_Virtual>
	class TCPool : public TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, TCNoAggregateData<t_CLock> >
	{
	public:
		TCPool(ENumaNode _NumaNode = ENumaNode_Default)
		{
			TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, TCNoAggregateData<t_CLock> >::f_Construct(_NumaNode);
		}
		~TCPool()
		{
			TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, TCNoAggregateData<t_CLock> >::f_Destruct();
		}
	};

	template <typename t_CData, mint t_GrowSize = 128, typename t_CLock = NMib::NThread::CNoLock, typename t_CAllocator = CAllocator_Virtual>
	class TCPoolSmall : public TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_FreeableSmall, t_CAllocator, TCNoAggregateData<t_CLock> >
	{
	public:
		TCPoolSmall(ENumaNode _NumaNode = ENumaNode_Default)
		{
			TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_FreeableSmall, t_CAllocator, TCNoAggregateData<t_CLock> >::f_Construct(_NumaNode);
		}
		~TCPoolSmall()
		{
			TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_FreeableSmall, t_CAllocator, TCNoAggregateData<t_CLock> >::f_Destruct();
		}
	};

	template <typename t_CData, mint t_GrowSize = 128, typename t_CLock = NMib::NThread::CNoLock, typename t_CAllocator = CAllocator_Virtual>
	class TCPoolGrowing : public TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_Growing, t_CAllocator, TCNoAggregateData<t_CLock> >
	{
	public:
		TCPoolGrowing(ENumaNode _NumaNode = ENumaNode_Default)
		{
			TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_Growing, t_CAllocator, TCNoAggregateData<t_CLock> >::f_Construct(_NumaNode);
		}
		~TCPoolGrowing()
		{
			TCPoolAggregate<t_CData, t_GrowSize, t_CLock, CPoolType_Growing, t_CAllocator, TCNoAggregateData<t_CLock> >::f_Destruct();
		}
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
	class TBCPool
	{
	public:
		void * operator new(mint _NumBytes, const t_CPool &_Pool)
		{
			return _Pool.f_GetBlock(_NumBytes);
		}

		void operator delete(void *_pToDelete, const t_CPool &_Pool) noexcept
		{
			_Pool.f_ReturnBlock(_pToDelete);
		}

		void operator delete(void *_pToDelete)
		{
			// We do nothing here, as we dont know the pool
		}

	};


	template
	<
		typename t_CType
		, mint t_GrowSize = 128
		, typename t_CAllocator = NMib::NMemory::CAllocator_Virtual
		, typename t_CPoolType = NMib::NMemory::CPoolType_FreeableSmall
		, typename t_CLockType = typename NMib::NThread::CMutual
	>
	class TCStaticPool;

	template
	<
		typename t_CType
		, mint t_GrowSize = 128
		, typename t_CAllocator = NMib::NMemory::CAllocator_Virtual
		, typename t_CPoolType = NMib::NMemory::CPoolType_FreeableSmall
		, typename t_CLockType = typename NMib::NThread::CMutual
	>
	class TCStaticPoolAllocator
	{
	public:

		enum
		{
			mc_Reporting = t_CAllocator::mc_Reporting
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = true
		};

		using CAutoDestroy = TCAllocator_AutoDestroyStatic<TCStaticPoolAllocator>;

		typedef TCStaticPool<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType> CStaticPool;
		static inline_small mint f_StaticAddresses()
		{
			return 0;
		}

		static inline_small mint f_GranularityAlloc(bool _bLargePages = false)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_GranularityCommit(bool _bLargePages = false)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_GranularityProtect(bool _bLargePages = false)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_Size(void *_pBlock)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_TrySize(void *_pBlock)
		{
			DMibPDebugBreak; // Not supported
			return sizeof(t_CType);
		}

		static inline_small mint f_SizePadded(mint _Size)
		{
			return sizeof(t_CType);
		}

		static inline_small bool f_CanCommit()
		{
			return false;
		}

		static inline_small bool f_CanProtect()
		{
			return false;
		}

		static inline_small void f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		static inline_small void *f_AllocWithSizeDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_AllocAlignedWithSize(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			while (true)
			{
				mint Size = _Size;
				void * pMem = CStaticPool::ms_Pool->f_GetBlock();
				if (!_Functor(pMem, Size))
					break;
			}
		}


		static inline_small void *f_Realloc(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_ReallocDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_Resize(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void *f_ResizeDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return CStaticPool::ms_Pool->f_GetBlock();
		}

		static inline_small void f_Commit(void *_pMem, mint _Size)
		{
		}

		static inline_small void f_Decommit(void *_pMem, mint _Size)
		{
		}

		static inline_small void f_Free(void *_pBlock, mint _Size)
		{
			return CStaticPool::ms_Pool->f_ReturnBlock(_pBlock);
		}

		static inline_small void f_FreeNoSize(void *_pBlock)
		{
			return CStaticPool::ms_Pool->f_ReturnBlock(_pBlock);
		}

		static inline_small fp32 f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			return CStaticPool::ms_Pool->f_Overhead(_pBlock);
		}
		only_parameters_aliased static CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			CAutoDestroy AutoDestroy;
			AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
			AutoDestroy.m_Size = _Size;

			return fg_Move(AutoDestroy);
		}
		only_parameters_aliased static CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			CAutoDestroy AutoDestroy;
			AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
			AutoDestroy.m_Size = _Size;

			return fg_Move(AutoDestroy);
		}

		static CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size)
		{
			return CAutoDestroy{_pMemory, _Size};
		}
	};

	template <typename t_CType, mint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	class TCStaticPool
	{
	public:
		typedef TCPool<t_CType, t_GrowSize, t_CLockType, t_CPoolType, t_CAllocator> CPoolType;

		static NStorage::TCAggregate<CPoolType, 16> ms_Pool;

		typedef TCStaticPoolAllocator<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType> CAllocator;
	};

	template <typename t_CType, mint t_GrowSize, typename t_CAllocator, typename t_CPoolType, typename t_CLockType>
	NStorage::TCAggregate<typename TCStaticPool<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::CPoolType, 16> TCStaticPool<t_CType, t_GrowSize, t_CAllocator, t_CPoolType, t_CLockType>::ms_Pool = {DAggregateInit};

	template <typename t_CType>
	void fg_StaticPoolDelete(t_CType *_pToDelete)
	{
		TCStaticPool<t_CType>::ms_Pool->f_Delete(_pToDelete);
	}

	template <typename t_CType, typename... tfp_CParams>
	void fg_StaticPoolNew(t_CType * &_pDestPtr, tfp_CParams&&... p_Params)
	{
		_pDestPtr = TCStaticPool<t_CType>::ms_Pool->f_New(fg_Forward<tfp_CParams>(p_Params)...);
	}

	template
	<
		typename t_CType
		, mint t_GrowSize = 128
		, typename t_CAllocator = NMib::NMemory::CAllocator_Virtual
		, typename t_CPoolType = NMib::NMemory::CPoolType_FreeableSmall
		, typename t_CLockType = NMib::NThread::CNoLock
	>
	class TCPoolAllocator : public CAllocator_Base
	{
	public:
		enum
		{
			mc_Reporting = t_CAllocator::mc_Reporting
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = false
		};

		using CAutoDestroy = TCAllocator_AutoDestroy<TCPoolAllocator>;

		TCPool<t_CType, t_GrowSize, t_CLockType, t_CPoolType, t_CAllocator> m_Pool;

		static inline_small mint f_StaticAddresses()
		{
			return 0;
		}

		static inline_small mint f_GranularityAlloc(bool _bLargePages = false)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_GranularityCommit(bool _bLargePages = false)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_GranularityProtect(bool _bLargePages = false)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_Size(void *_pBlock)
		{
			return sizeof(t_CType);
		}

		static inline_small mint f_TrySize(void *_pBlock)
		{
			DMibPDebugBreak; // Not supported
			return sizeof(t_CType);
		}

		static inline_small mint f_SizePadded(mint _Size)
		{
			return sizeof(t_CType);
		}

		static inline_small bool f_CanCommit()
		{
			return false;
		}

		static inline_small bool f_CanProtect()
		{
			return false;
		}

		static inline_small void f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		inline_small void *f_AllocWithSizeDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocAlignedWithSize(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			return m_Pool.f_GetBlock();
		}

		only_parameters_aliased inline_small void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			while (true)
			{
				mint Size = _Size;
				void * pMem = m_Pool.f_GetBlock();
				if (!_Functor(pMem, Size))
					break;
			}
		}

		inline_small void *f_Realloc(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		inline_small void *f_ReallocDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		inline_small void *f_Resize(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		inline_small void *f_ResizeDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(t_CType));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		static inline_small void f_Commit(void *_pMem, mint _Size)
		{
		}

		static inline_small void f_Decommit(void *_pMem, mint _Size)
		{
		}

		inline_small void f_Free(void *_pBlock, mint _Size)
		{
			return m_Pool.f_ReturnBlock(_pBlock);
		}

		inline_small void f_FreeNoSize(void *_pBlock)
		{
			return m_Pool.f_ReturnBlock(_pBlock);
		}

		inline_small fp32 f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			return m_Pool.f_Overhead(_pBlock);
		}
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			CAutoDestroy AutoDestroy{*this};
			AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
			AutoDestroy.m_Size = _Size;

			return fg_Move(AutoDestroy);
		}
		only_parameters_aliased CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			CAutoDestroy AutoDestroy{*this};
			AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
			AutoDestroy.m_Size = _Size;

			return fg_Move(AutoDestroy);
		}

		CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size)
		{
			return CAutoDestroy{_pMemory, _Size, *this};
		}
	};

	template <typename t_CPoolType>
	class TCPoolReferenceAllocator
	{
	public:
		enum
		{
			mc_Reporting = t_CPoolType::mc_Reporting
			, mc_CanBeStatic = false
			, mc_bMethodsStatic = false
		};

		typedef typename t_CPoolType::CData CData;
		using CAutoDestroy = TCAllocator_AutoDestroy<TCPoolReferenceAllocator>;

		t_CPoolType &m_Pool;

		TCPoolReferenceAllocator(t_CPoolType &_Pool)
			: m_Pool(_Pool)
		{
		}

		static inline_small mint f_StaticAddresses()
		{
			return 0;
		}

		static inline_small mint f_GranularityAlloc(bool _bLargePages = false)
		{
			return sizeof(CData);
		}

		static inline_small mint f_GranularityCommit(bool _bLargePages = false)
		{
			return sizeof(CData);
		}

		static inline_small mint f_GranularityProtect(bool _bLargePages = false)
		{
			return sizeof(CData);
		}

		static inline_small mint f_Size(void *_pBlock)
		{
			return sizeof(CData);
		}

		static inline_small mint f_TrySize(void *_pBlock)
		{
			DMibPDebugBreak; // Not supported
			return sizeof(CData);
		}

		static inline_small mint f_SizePadded(mint _Size)
		{
			return sizeof(CData);
		}

		static inline_small bool f_CanCommit()
		{
			return false;
		}

		static inline_small bool f_CanProtect()
		{
			return false;
		}

		static inline_small void f_Protect(void *_pMem, mint _Size, uaint _Protect)
		{

		}

		inline_small void *f_AllocWithSizeDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocAlignedWithSize(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_Alloc(mint _Size, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			return m_Pool.f_GetBlock();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			return m_Pool.f_GetBlock();
		}

		only_parameters_aliased inline_small void f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			while (true)
			{
				mint Size = _Size;
				void * pMem = m_Pool.f_GetBlock();
				if (!_Functor(pMem, Size))
					break;
			}
		}

		inline_small void *f_Realloc(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		inline_small void *f_ReallocDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		inline_small void *f_Resize(void *_pMem, mint _Size, mint _OldSize, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		inline_small void *f_ResizeDebug(void *_pMem, mint _Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			DMibFastCheck(_Size == sizeof(CData));
			if (_pMem)
				return _pMem;
			else
				return m_Pool.f_GetBlock();
		}

		static inline_small void f_Commit(void *_pMem, mint _Size)
		{
		}

		static inline_small void f_Decommit(void *_pMem, mint _Size)
		{
		}

		inline_small void f_Free(void *_pBlock, mint _Size)
		{
			return m_Pool.f_ReturnBlock(_pBlock);
		}

		inline_small void f_FreeNoSize(void *_pBlock)
		{
			return m_Pool.f_ReturnBlock(_pBlock);
		}

		inline_small fp32 f_Overhead(void const *_pBlock) // Number of bytes overhead for block
		{
			return m_Pool.f_Overhead(_pBlock);
		}
		only_parameters_aliased CAutoDestroy f_AllocSafeWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			CAutoDestroy AutoDestroy{*this};
			AutoDestroy.m_pMemory = f_AllocAlignedWithSize(_Size, _Alignment, _AllocFlags, _NumaNode);
			AutoDestroy.m_Size = _Size;

			return fg_Move(AutoDestroy);
		}
		only_parameters_aliased CAutoDestroy f_AllocSafe(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags = EAllocationFlag_None, ENumaNode _NumaNode = ENumaNode_Default)
		{
			CAutoDestroy AutoDestroy{*this};
			AutoDestroy.m_pMemory = f_AllocAligned(_Size, _Alignment, _AllocFlags, _NumaNode);
			AutoDestroy.m_Size = _Size;

			return fg_Move(AutoDestroy);
		}

		CAutoDestroy f_MakeSafe(void *_pMemory, mint _Size)
		{
			return CAutoDestroy{_pMemory, _Size, *this};
		}
	};
}
