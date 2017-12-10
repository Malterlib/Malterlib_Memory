// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include <Mib/Test/Memory>
#include "../../Memory/Source/Malterlib_Memory_Heap.h"

namespace
{

	enum 
	{
		EUseFast = true
	};

	//#define VirtualLimit 1024*1024
	#define VirtualLimit -1
	
	template <class t_CAllocator, aint t_bClearHeaps, bint t_bLock = false, aint t_AlignBits = NMib::TCHighestBitSetCorrect<aint, sizeof(void *)>::mc_Value, bint t_bUseFast = EUseFast>
	class TCLocalHeapParams : public NMib::NMem::CHeapDefaultParams
	{
	public:
		static_assert((1 << t_AlignBits) >= int(sizeof(void *)));
		enum
		{
			EGrowSize = 4*1024*1024
			,ENumChunkFreeThreshold = t_bClearHeaps
			,EAlignBits = t_AlignBits
			,EbOptimizeForSize = 0
			,EBlockCacheSize = (64 + sizeof(mint)) * t_bUseFast
			,ECacheFreeThreshold = 256
			,EFreeSizeBucketTreeThresholdBits = 9
			,ETreeOpt0 = 1
			,ETreeOpt1 = 1
			,ETreeOpt2 = 1
			,ETreeOpt3 = 1
			,EAccurateBucketCache = 0
			,ELargePages = 0
		};
		typedef t_CAllocator CAllocator;
		typedef NMib::NMem::CTCHeap_SizeHolderFast CSizeHolder;
		typedef typename NMib::TCChooseType<t_bLock, NMib::NThread::CMutual, NMib::NThread::CNoLock>::CType CLock;
	};

}

namespace
{
	using namespace NMib::NTest;
	using namespace NMib::NMem;
	class COldHeap_Tests : public CTest
	{
	public:

		COldHeap_Tests()
		{
		}

		void f_DoTests()
		{
			DMibTestCategory("Bugs")
			{
				DMibTestSuite("Combined heap wrong size")
				{
					TCHeap_Combined<-1, TCLocalHeapParams<CAllocator_VirtualNoCommit, false, false, 4, false> > Heap;
					mint Align = 1 << 4;

					mint Largest = Heap.f_Get8BitLargest();
					
					void *pBlock = nullptr;
					void *pBlock1 = nullptr;
					void *pBlock2 = nullptr;

					auto Cleanup
						= NMib::fg_OnScopeExit
						(
							[&]()
							{
								Heap.f_Free(pBlock);
								pBlock = nullptr;
								Heap.f_Free(pBlock1);
								pBlock1 = nullptr;
								Heap.f_Free(pBlock2);
								pBlock2 = nullptr;
							}
						)
					;
					
					pBlock = Heap.f_Alloc(Largest - Align); // 1982 - almost largest block
					pBlock1 = Heap.f_Alloc(Align); // Alloc another (32 bytes)
					pBlock2 = Heap.f_Alloc(Align); // Alloc another so free block cannot be used
					Heap.f_Free(pBlock);	// Free both so the free block becomes largest block + align
					pBlock = nullptr;
					Heap.f_Free(pBlock1);	//
					pBlock1 = nullptr;
					pBlock = Heap.f_Alloc(Largest); // This allocation will be 16 bytes larger that largest block alloc which caused wrong heap to be chosen in f_Resize
					mint Size = Largest * 2;
					pBlock = Heap.f_Resize(pBlock, Size);
					int CurrentSize = 64*1024;
					for (; CurrentSize > 0; CurrentSize -= 1)
					{
						mint Size = CurrentSize;
						pBlock = Heap.f_Resize(pBlock, Size);
					}
					for (; CurrentSize < 64*1024; CurrentSize += 1)
					{
						mint Size = CurrentSize;
						pBlock = Heap.f_Resize(pBlock, Size);
					}
					for (; CurrentSize > 0; CurrentSize -= 1)
					{
						mint Size = CurrentSize;
						pBlock = Heap.f_Resize(pBlock, Size);
					}

					Heap.f_Free(pBlock);
					pBlock = nullptr;
					Heap.f_Free(pBlock2);
					pBlock2 = nullptr;
				};
				DMibTestSuite("Allocation fails for certain sizes above 4 MiB")
				{
					auto pMemory = CAllocator_Heap::f_Alloc(20971488);
					DMibTest(DMibExpr(pMemory))(ETest_FailAndStop);
					CAllocator_Heap::f_Free(pMemory);
				};
				DMibTestSuite("AlignmentStandalone")
				{
					TCHeap_StandAlone<TCHeapParams<4096*1024, CAllocator_Virtual>> Heap;
					
					NMib::NContainer::TCVector<void *> Allocs;
					auto Cleanup
						= NMib::fg_OnScopeExit
						(
							[&]()
							{
								for (auto iAlloc = Allocs.f_GetIterator(); iAlloc; ++iAlloc)
									Heap.f_Free(*iAlloc);
							}
						)
					;
					
					for (int i = 0; i < 128; ++i)
					{
						for (mint Align = 1; Align <= 8*1024*1024; Align <<= 1)
						{
							mint Size = 1;
							void *pAlloc = Heap.f_AllocAligned(Size, Align);
							auto Cleanup
								= NMib::fg_OnScopeExit
								(
									[&]()
									{
										Heap.f_Free(pAlloc);
									}
								)
							;
							
							DMibTest(DMibExpr(pAlloc))(ETestFlag_Aggregated);
							DMibTest(DMibExpr(((mint)pAlloc & (Align - 1))) == DMibExpr(0))(ETestFlag_Aggregated);
							Heap.f_CheckHeap(true);
							Heap.f_Free(pAlloc);
							pAlloc = nullptr;
							Heap.f_CheckHeap(true);
							//Allocs.f_Insert(pAlloc);
						
						}
					}
					for (auto iAlloc = Allocs.f_GetIterator(); iAlloc; ++iAlloc)
					{
						Heap.f_Free(*iAlloc);
						*iAlloc = nullptr;
					}
				};
#if DMibPPtrBits > 32 // Runs out of virtual address space on 32 bit processes
				DMibTestSuite("AlignmentNormal")
				{
					TCHeap_Combined<-1, TCHeapParams<4096*1024, CAllocator_Virtual>> Heap;
					
					NMib::NContainer::TCVector<void *> Allocs;
					auto Cleanup
						= NMib::fg_OnScopeExit
						(
							[&]()
							{
								for (auto iAlloc = Allocs.f_GetIterator(); iAlloc; ++iAlloc)
									Heap.f_Free(*iAlloc);
							}
						)
					;
					
					for (int i = 0; i < 128; ++i)
					{
						for (mint Align = 1; Align <= 8*1024*1024; Align <<= 1)
						{
							mint Size = 1;
							void *pAlloc = Heap.f_AllocAligned(Size, Align);
							Heap.f_CheckHeap(true);
							Allocs.f_Insert(pAlloc);
							DMibTest(DMibExpr(pAlloc))(ETestFlag_Aggregated);
							DMibTest(DMibExpr(((mint)pAlloc & (Align - 1))) == DMibExpr(0))(ETestFlag_Aggregated);
						
						}
					}
					for (auto iAlloc = Allocs.f_GetIterator(); iAlloc; ++iAlloc)
					{
						Heap.f_Free(*iAlloc);
						*iAlloc = nullptr;
						Heap.f_CheckHeap(true);
					}
				};
				DMibTestSuite("AlignmentDebug")
				{
					TCHeap_CombinedDebug<TCHeapParamsDebug<4096*1024, CAllocator_Virtual>> Heap;
					
					NMib::NContainer::TCVector<void *> Allocs;
					auto Cleanup
						= NMib::fg_OnScopeExit
						(
							[&]()
							{
								for (auto iAlloc = Allocs.f_GetIterator(); iAlloc; ++iAlloc)
									Heap.f_Free(*iAlloc);
							}
						)
					;
					
					for (int i = 0; i < 128; ++i)
					{
						for (mint Align = 1; Align <= 8*1024*1024; Align <<= 1)
						{
							mint Size = 1;
							void *pAlloc = Heap.f_AllocAligned(Size, Align);
							Heap.f_CheckHeap(true);
							Allocs.f_Insert(pAlloc);
							DMibTest(DMibExpr(pAlloc))(ETestFlag_Aggregated);
							DMibTest(DMibExpr(((mint)pAlloc & (Align - 1))) == DMibExpr(0))(ETestFlag_Aggregated);
						
						}
					}
					for (auto iAlloc = Allocs.f_GetIterator(); iAlloc; ++iAlloc)
					{
						Heap.f_Free(*iAlloc);
						*iAlloc = nullptr;
						Heap.f_CheckHeap(true);
					}
				};
#endif
			};
		}
	};

	DMibTestRegister(COldHeap_Tests, Malterlib::Memory);

}

