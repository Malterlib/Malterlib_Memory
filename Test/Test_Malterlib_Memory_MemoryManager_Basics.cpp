// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Test/Memory>
#include <Mib/Test/Exception>

#include "../../Memory/Source/Malterlib_Memory_MemoryManager.h"
#include "../../Memory/Source/Malterlib_Memory_MemoryManager.hpp"
#include "../../Memory/Source/Malterlib_Memory_MemoryManager_Tracked.h"

namespace
{
	using namespace NMib::NTest;
	using namespace NMib::NMemory;

	constexpr umint gc_TestPageSize = gc_OsMaxPageSize;

	class CBasics_Tests : public CTest
	{
	public:

		CBasics_Tests()
		{
		}

		struct CAlloc
		{
			void *m_pAlloc;
			umint m_Size;
		};

		void f_NonFinished()
		{
#if 0
			TCMemoryManagerSlab<CParamsNoCleanup, 0> *pSlab = nullptr;


			{
				TCMemoryManager<CParamsNoCleanup> MemoryManager;
				umint LastSlab = 0xFFFFFFFF;
				NMib::NContainer::TCLinkedList<void *> Allocated;
				for (int i = 0; i <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++i)
				{
					umint Size = (*MemoryManager.m_LocalArena).f_SizePadded(i);
					if (i < 1024)
						Size = i;
					if (Size != LastSlab)
					{
						umint SizeAlloc = Size;
						auto pMem0 = MemoryManager.f_AllocWithSize(SizeAlloc);
						Allocated.f_Insert(pMem0);
						LastSlab = Size;
					}
				}
				for (int i = CParamsNoCleanup::mc_MaxSlabAllocSize + 1; i <= CParamsNoCleanup::mc_MaxSlabAllocSize * 16; i += CParamsNoCleanup::mc_MaxSlabAllocSize)
				{
					umint Size = i;
					auto pMem0 = MemoryManager.f_AllocWithSize(Size);
					Allocated.f_Insert(pMem0);
				}

				NMib::NThread::CEvent AllAllocated;
				AllAllocated.f_ResetSignaled();
				NMib::NThread::CEvent ThreadStopEvent;
				ThreadStopEvent.f_ResetSignaled();

				auto fl_ThreadTest
					= [&](NMib::NThread::CThreadObject *_pThread) -> aint
					{
						for (int i = 0; i <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++i)
						{
							umint Size = (*MemoryManager.m_LocalArena).f_SizePadded(i);
							if (i < 1024)
								Size = i;
							if (Size != LastSlab)
							{
								umint SizeAlloc = Size;
								auto pMem0 = MemoryManager.f_AllocWithSize(SizeAlloc);
								Allocated.f_Insert(pMem0);
								LastSlab = Size;
							}
						}
						return 0;
					}
				;

				NMib::NStorage::TCUniquePointer<NMib::NThread::CThreadObject> pThread
					= NMib::NThread::CThreadObject::fs_StartThread
					(
						[&](NMib::NThread::CThreadObject *_pThread) -> aint
						{
							fl_ThreadTest(_pThread);
							AllAllocated.f_SetSignaled();
							ThreadStopEvent.f_Wait();
							return 0;
						}
						, "Test thread alloc"
					)
				;
				AllAllocated.f_Wait();
				for (auto iAllocated = Allocated.f_GetIterator(); iAllocated; ++iAllocated)
					MemoryManager.f_Free(*iAllocated);
				Allocated.f_Clear();

				for (int i = 0; i < 1024; ++i)
				{
					NMib::NStorage::TCUniquePointer<NMib::NThread::CThreadObject> pThread
						= NMib::NThread::CThreadObject::fs_StartThread
						(
							fl_ThreadTest
							, "Test thread alloc"
						)
					;
					pThread.f_Clear();
					for (auto iAllocated = Allocated.f_GetIterator(); iAllocated; ++iAllocated)
						MemoryManager.f_Free(*iAllocated);
					Allocated.f_Clear();
				}

				umint Size = 17;
				auto pMem0 = MemoryManager.f_AllocWithSize(Size);
				Size = 11;
				auto pMem1 = MemoryManager.f_AllocWithSize(Size);
				Size = 2;
				auto pMem2 = MemoryManager.f_AllocWithSize(Size);
				Size = 32;
				auto pMem3 = MemoryManager.f_AllocWithSize(Size);
				Size = 28;
				auto pMem4 = MemoryManager.f_AllocWithSize(Size);
				MemoryManager.f_Free(pMem4);
				pMem4 = MemoryManager.f_AllocWithSize(Size);
				MemoryManager.f_Free(pMem4);
				pMem4 = MemoryManager.f_AllocWithSize(Size);
				MemoryManager.f_Free(pMem4);
				pMem4 = MemoryManager.f_AllocWithSize(Size);
				MemoryManager.f_Free(pMem4);
				MemoryManager.f_Free(pMem0);
				MemoryManager.f_Free(pMem1);
				MemoryManager.f_Free(pMem2);
				MemoryManager.f_Free(pMem3);
				ThreadStopEvent.f_SetSignaled();
				pThread.f_Clear();
			}
#endif
		}


		void f_Dummy()
		{
		}

		template <typename tf_CParams>
		struct TCParamsNoCleanup : public tf_CParams
		{
			static constexpr bool mc_bBackgroundCleanup = false; // Background cleanups will hurt predictability
		};

		template <typename tf_CParams>
		struct TCParamsBackgroundTest : public tf_CParams
		{
			static constexpr uint32 mc_BackgroundCleanupLifetime = 1;
			static constexpr uint32 mc_BackgroundCleanupLifetimeDecommit = 1;
		};

		template <umint t_PageSize, umint t_nSizesPerLevel>
		struct TCParams : public CDefaultMemoryManagerParams
		{
			static constexpr umint mc_NumSizesPerLevel = t_nSizesPerLevel;
			static constexpr umint mc_SubSlabSize = t_PageSize;
			static constexpr bool mc_bUseFreeBlockCounting = true;
			static constexpr umint mc_PreventCacheConflictSize = 0;
			static constexpr bool mc_bUseSlabFromEnd = true;
		};

		template <typename tf_CParams, umint t_PageSize>
		void f_TestMemory()
		{
			using CParams = TCMemoryManagerParams<tf_CParams>;
			using CParamsNoCleanup = TCMemoryManagerParams<TCParamsNoCleanup<tf_CParams>>;

			DMibTestSuite("Internals")
			{
				// Just run for checking the asserts
				auto fCheckParams = [=]<umint tf_nSizesPerLevel>(umint _nSizesPerLevel)
					{
						using CParams = TCMemoryManagerParams<TCParams<t_PageSize, tf_nSizesPerLevel>>;
						// Just run for checking the asserts
						for (int i = 0; i < CParams::mc_SlabSize / CParams::mc_SubSlabSize; ++i)
						{
							for (umint iSize = 0; iSize < _nSizesPerLevel; ++iSize)
								CParams::fs_DivideBySlabMultiplier(i, iSize);
						}

					}
				;
				fCheckParams.template operator ()<16>(16);
				fCheckParams.template operator ()<8>(8);
				fCheckParams.template operator ()<4>(4);
				fCheckParams.template operator ()<2>(2);
				fCheckParams.template operator ()<1>(1);

				{
					using CParams = TCMemoryManagerParams<TCParams<t_PageSize, 16>>;
					static_assert(CParams::mc_NumAllocsPerSubSlab[0] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize);
					static_assert(CParams::mc_NumAllocsPerSubSlab[1] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[2] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[3] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[4] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 4);
					static_assert(CParams::mc_NumAllocsPerSubSlab[5] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[6] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[7] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[8] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 2);
					static_assert(CParams::mc_NumAllocsPerSubSlab[9] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[10] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[11] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[12] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 4);
					static_assert(CParams::mc_NumAllocsPerSubSlab[13] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
					static_assert(CParams::mc_NumAllocsPerSubSlab[14] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[15] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 16);
				}
				{
					using CParams = TCMemoryManagerParams<TCParams<t_PageSize, 8>>;
					static_assert(CParams::mc_NumAllocsPerSubSlab[0] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize);
					static_assert(CParams::mc_NumAllocsPerSubSlab[1] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[2] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 4);
					static_assert(CParams::mc_NumAllocsPerSubSlab[3] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[4] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 2);
					static_assert(CParams::mc_NumAllocsPerSubSlab[5] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
					static_assert(CParams::mc_NumAllocsPerSubSlab[6] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 4);
					static_assert(CParams::mc_NumAllocsPerSubSlab[7] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 8);
				}
				{
					using CParams = TCMemoryManagerParams<TCParams<t_PageSize, 4>>;
					static_assert(CParams::mc_NumAllocsPerSubSlab[0] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize);
					static_assert(CParams::mc_NumAllocsPerSubSlab[1] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 4);
					static_assert(CParams::mc_NumAllocsPerSubSlab[2] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 2);
					static_assert(CParams::mc_NumAllocsPerSubSlab[3] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 4);
				}
				{
					using CParams = TCMemoryManagerParams<TCParams<t_PageSize, 2>>;
					static_assert(CParams::mc_NumAllocsPerSubSlab[0] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize);
					static_assert(CParams::mc_NumAllocsPerSubSlab[1] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize * 2);
				}
				{
					using CParams = TCMemoryManagerParams<TCParams<t_PageSize, 1>>;
					static_assert(CParams::mc_NumAllocsPerSubSlab[0] << CParams::mc_MinNormalSlabBucket == CParams::mc_SubSlabSize);
				}

				auto fAligned = [](umint _Size)
					{
						if constexpr (!tf_CParams::mc_bAllowUnalignedFreeList)
							return NMib::fg_AlignUp(_Size, sizeof(void *));
						return _Size;
					}
				;

				{
					DMibTestPath("Aligned allocation overflow");
					TCMemoryManager<CParams> Manager{CMemoryManagerConfig()};

					umint OverflowSize = NMib::TCLimitsInt<umint>::mc_Max;
					DMibExpectExceptionType(Manager.f_AllocAlignedWithSize(OverflowSize, 16), NMib::NException::CExceptionMemory);
				}

				{
					DMibTestPath("Sizes 8");
					TCMemoryManager<CParams> Manager{CMemoryManagerConfig()};

					if constexpr (tf_CParams::mc_bUseSmallSizes)
					{
						DMibTest(DMibExpr(Manager.f_SizePadded(0)) == DMibExpr(1));
						DMibTest(DMibExpr(Manager.f_SizePadded(1)) == DMibExpr(1));
						DMibTest(DMibExpr(Manager.f_SizePadded(2)) == DMibExpr(2));
						DMibTest(DMibExpr(Manager.f_SizePadded(3)) == DMibExpr(4));
						DMibTest(DMibExpr(Manager.f_SizePadded(4)) == DMibExpr(4));
						DMibTest(DMibExpr(Manager.f_SizePadded(5)) == DMibExpr(8));
						DMibTest(DMibExpr(Manager.f_SizePadded(6)) == DMibExpr(8));
						DMibTest(DMibExpr(Manager.f_SizePadded(7)) == DMibExpr(8));
						DMibTest(DMibExpr(Manager.f_SizePadded(8)) == DMibExpr(8));
						DMibTest(DMibExpr(Manager.f_SizePadded(9)) == DMibExpr(12));
						DMibTest(DMibExpr(Manager.f_SizePadded(10)) == DMibExpr(12));
						DMibTest(DMibExpr(Manager.f_SizePadded(11)) == DMibExpr(12));
						DMibTest(DMibExpr(Manager.f_SizePadded(12)) == DMibExpr(12));
						DMibTest(DMibExpr(Manager.f_SizePadded(13)) == DMibExpr(16));
						DMibTest(DMibExpr(Manager.f_SizePadded(14)) == DMibExpr(16));
						DMibTest(DMibExpr(Manager.f_SizePadded(15)) == DMibExpr(16));
						DMibTest(DMibExpr(Manager.f_SizePadded(16)) == DMibExpr(16));
					}
					else
					{
						DMibTest(DMibExpr(Manager.f_SizePadded(0)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(1)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(2)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(3)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(4)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(5)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(6)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(7)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(8)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(9)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(10)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(11)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(12)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(13)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(14)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(15)) == DMibExpr(fAligned(20)));
						DMibTest(DMibExpr(Manager.f_SizePadded(16)) == DMibExpr(fAligned(20)));
					}

					DMibTest(DMibExpr(Manager.f_SizePadded(20)) == DMibExpr(fAligned(20)));
					DMibTest(DMibExpr(Manager.f_SizePadded(24)) == DMibExpr(fAligned(24)));
					DMibTest(DMibExpr(Manager.f_SizePadded(28)) == DMibExpr(fAligned(28)));
					DMibTest(DMibExpr(Manager.f_SizePadded(29)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(32)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(33)) == DMibExpr(fAligned(36)));
					DMibTest(DMibExpr(Manager.f_SizePadded(36)) == DMibExpr(fAligned(36)));
					DMibTest(DMibExpr(Manager.f_SizePadded(37)) == DMibExpr(fAligned(40)));
					DMibTest(DMibExpr(Manager.f_SizePadded(40)) == DMibExpr(fAligned(40)));
					DMibTest(DMibExpr(Manager.f_SizePadded(41)) == DMibExpr(fAligned(44)));
					DMibTest(DMibExpr(Manager.f_SizePadded(44)) == DMibExpr(fAligned(44)));
					DMibTest(DMibExpr(Manager.f_SizePadded(45)) == DMibExpr(fAligned(48)));
					DMibTest(DMibExpr(Manager.f_SizePadded(48)) == DMibExpr(fAligned(48)));
					DMibTest(DMibExpr(Manager.f_SizePadded(49)) == DMibExpr(fAligned(52)));
					DMibTest(DMibExpr(Manager.f_SizePadded(52)) == DMibExpr(fAligned(52)));
					DMibTest(DMibExpr(Manager.f_SizePadded(53)) == DMibExpr(fAligned(56)));
					DMibTest(DMibExpr(Manager.f_SizePadded(56)) == DMibExpr(fAligned(56)));
					DMibTest(DMibExpr(Manager.f_SizePadded(57)) == DMibExpr(fAligned(60)));
					DMibTest(DMibExpr(Manager.f_SizePadded(60)) == DMibExpr(fAligned(60)));
					DMibTest(DMibExpr(Manager.f_SizePadded(61)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(64)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(65)) == DMibExpr(fAligned(72)));
					DMibTest(DMibExpr(Manager.f_SizePadded(72)) == DMibExpr(fAligned(72)));
					DMibTest(DMibExpr(Manager.f_SizePadded(73)) == DMibExpr(fAligned(80)));
					DMibTest(DMibExpr(Manager.f_SizePadded(80)) == DMibExpr(fAligned(80)));
					DMibTest(DMibExpr(Manager.f_SizePadded(81)) == DMibExpr(fAligned(88)));
					DMibTest(DMibExpr(Manager.f_SizePadded(88)) == DMibExpr(fAligned(88)));
					DMibTest(DMibExpr(Manager.f_SizePadded(89)) == DMibExpr(fAligned(96)));
					DMibTest(DMibExpr(Manager.f_SizePadded(96)) == DMibExpr(fAligned(96)));
					DMibTest(DMibExpr(Manager.f_SizePadded(97)) == DMibExpr(fAligned(104)));
					DMibTest(DMibExpr(Manager.f_SizePadded(104)) == DMibExpr(fAligned(104)));
					DMibTest(DMibExpr(Manager.f_SizePadded(105)) == DMibExpr(fAligned(112)));
					DMibTest(DMibExpr(Manager.f_SizePadded(112)) == DMibExpr(fAligned(112)));
					DMibTest(DMibExpr(Manager.f_SizePadded(113)) == DMibExpr(fAligned(120)));
					DMibTest(DMibExpr(Manager.f_SizePadded(120)) == DMibExpr(fAligned(120)));
					DMibTest(DMibExpr(Manager.f_SizePadded(121)) == DMibExpr(fAligned(128)));
					DMibTest(DMibExpr(Manager.f_SizePadded(128)) == DMibExpr(fAligned(128)));
				}
				{
					DMibTestPath("Sizes 4");
					TCMemoryManager<TCMemoryManagerParams<TCParams<t_PageSize, 4>>> Manager{CMemoryManagerConfig()};

					DMibTest(DMibExpr(Manager.f_SizePadded(0)) == DMibExpr(1));
					DMibTest(DMibExpr(Manager.f_SizePadded(1)) == DMibExpr(1));
					DMibTest(DMibExpr(Manager.f_SizePadded(2)) == DMibExpr(2));
					DMibTest(DMibExpr(Manager.f_SizePadded(3)) == DMibExpr(4));
					DMibTest(DMibExpr(Manager.f_SizePadded(4)) == DMibExpr(4));
					DMibTest(DMibExpr(Manager.f_SizePadded(5)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(6)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(7)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(8)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(9)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(10)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(11)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(12)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(13)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(14)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(15)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(16)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(20)) == DMibExpr(fAligned(20)));
					DMibTest(DMibExpr(Manager.f_SizePadded(24)) == DMibExpr(fAligned(24)));
					DMibTest(DMibExpr(Manager.f_SizePadded(32)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(33)) == DMibExpr(fAligned(40)));
					DMibTest(DMibExpr(Manager.f_SizePadded(40)) == DMibExpr(fAligned(40)));
					DMibTest(DMibExpr(Manager.f_SizePadded(41)) == DMibExpr(fAligned(48)));
					DMibTest(DMibExpr(Manager.f_SizePadded(48)) == DMibExpr(fAligned(48)));
					DMibTest(DMibExpr(Manager.f_SizePadded(49)) == DMibExpr(fAligned(56)));
					DMibTest(DMibExpr(Manager.f_SizePadded(56)) == DMibExpr(fAligned(56)));
					DMibTest(DMibExpr(Manager.f_SizePadded(57)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(64)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(65)) == DMibExpr(fAligned(80)));
					DMibTest(DMibExpr(Manager.f_SizePadded(80)) == DMibExpr(fAligned(80)));
					DMibTest(DMibExpr(Manager.f_SizePadded(81)) == DMibExpr(fAligned(96)));
					DMibTest(DMibExpr(Manager.f_SizePadded(96)) == DMibExpr(fAligned(96)));
					DMibTest(DMibExpr(Manager.f_SizePadded(97)) == DMibExpr(fAligned(112)));
					DMibTest(DMibExpr(Manager.f_SizePadded(112)) == DMibExpr(fAligned(112)));
					DMibTest(DMibExpr(Manager.f_SizePadded(113)) == DMibExpr(fAligned(128)));
					DMibTest(DMibExpr(Manager.f_SizePadded(128)) == DMibExpr(fAligned(128)));
				}
				{
					DMibTestPath("Sizes 2");
					TCMemoryManager<TCMemoryManagerParams<TCParams<t_PageSize, 2>>> Manager{CMemoryManagerConfig()};

					DMibTest(DMibExpr(Manager.f_SizePadded(0)) == DMibExpr(1));
					DMibTest(DMibExpr(Manager.f_SizePadded(1)) == DMibExpr(1));
					DMibTest(DMibExpr(Manager.f_SizePadded(2)) == DMibExpr(2));
					DMibTest(DMibExpr(Manager.f_SizePadded(3)) == DMibExpr(4));
					DMibTest(DMibExpr(Manager.f_SizePadded(4)) == DMibExpr(4));
					DMibTest(DMibExpr(Manager.f_SizePadded(5)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(6)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(7)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(8)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(9)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(10)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(11)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(12)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(13)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(14)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(15)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(16)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(20)) == DMibExpr(fAligned(24)));
					DMibTest(DMibExpr(Manager.f_SizePadded(24)) == DMibExpr(fAligned(24)));
					DMibTest(DMibExpr(Manager.f_SizePadded(32)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(33)) == DMibExpr(fAligned(48)));
					DMibTest(DMibExpr(Manager.f_SizePadded(48)) == DMibExpr(fAligned(48)));
					DMibTest(DMibExpr(Manager.f_SizePadded(49)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(64)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(65)) == DMibExpr(fAligned(96)));
					DMibTest(DMibExpr(Manager.f_SizePadded(96)) == DMibExpr(fAligned(96)));
					DMibTest(DMibExpr(Manager.f_SizePadded(97)) == DMibExpr(fAligned(128)));
					DMibTest(DMibExpr(Manager.f_SizePadded(128)) == DMibExpr(fAligned(128)));
				}
				{
					DMibTestPath("Sizes 1");
					TCMemoryManager<TCMemoryManagerParams<TCParams<t_PageSize, 1>>> Manager{CMemoryManagerConfig()};

					DMibTest(DMibExpr(Manager.f_SizePadded(0)) == DMibExpr(1));
					DMibTest(DMibExpr(Manager.f_SizePadded(1)) == DMibExpr(1));
					DMibTest(DMibExpr(Manager.f_SizePadded(2)) == DMibExpr(2));
					DMibTest(DMibExpr(Manager.f_SizePadded(3)) == DMibExpr(4));
					DMibTest(DMibExpr(Manager.f_SizePadded(4)) == DMibExpr(4));
					DMibTest(DMibExpr(Manager.f_SizePadded(5)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(6)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(7)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(8)) == DMibExpr(8));
					DMibTest(DMibExpr(Manager.f_SizePadded(9)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(10)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(11)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(12)) == DMibExpr(12));
					DMibTest(DMibExpr(Manager.f_SizePadded(13)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(14)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(15)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(16)) == DMibExpr(16));
					DMibTest(DMibExpr(Manager.f_SizePadded(20)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(24)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(32)) == DMibExpr(fAligned(32)));
					DMibTest(DMibExpr(Manager.f_SizePadded(33)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(64)) == DMibExpr(fAligned(64)));
					DMibTest(DMibExpr(Manager.f_SizePadded(65)) == DMibExpr(fAligned(128)));
					DMibTest(DMibExpr(Manager.f_SizePadded(128)) == DMibExpr(fAligned(128)));
				}
			};


			DMibTestSuite("All sizes")
			{
				CTestMemoryMeasure MeasureMemory("Alloc");

				NMib::NContainer::TCVector<CAlloc> Allocs;

				MeasureMemory.f_Start();

				{
					TCMemoryManagerTracked<TCMemoryManager<CParamsNoCleanup>> MemoryManager("Test", CMemoryManagerConfig());
					MemoryManager.f_ForceStartCleanupThreads();
					umint LastAlloc = 0;
					auto Checkout = MemoryManager.f_CheckoutForce();
					for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
					{
						umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
						if (AllocSize != LastAlloc || MemorySize < 1024)
						{
							LastAlloc = AllocSize;
							umint Size = MemorySize;
							auto pMemory = MemoryManager.f_AllocWithSize(Size);
							umint ReturnedSize = MemoryManager.f_Size(pMemory);
							umint AllocSize = MemoryManager.f_SizePadded(MemorySize);

							{
#if DMibConfig_Memory_Shims_Enable

								NMib::NMemory::CDisableMemoryReporterScope DisableReport;
#endif
								Allocs.f_Insert({pMemory, Size});
								DMibTest(DMibExpr(AllocSize) == DMibExpr(Size))(ETestFlag_Aggregated);
								DMibTest(DMibExpr(Size) >= DMibExpr(MemorySize))(ETestFlag_Aggregated);
								DMibTest(DMibExpr(ReturnedSize) == DMibExpr(Size))(ETestFlag_Aggregated);
							}
						}
					}
					for (auto &Alloc : Allocs)
						MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);

					MemoryManager.f_GarbageCollect(true);
				}

				MeasureMemory.f_Stop(1);

				NMib::NTest::CTestMemoryResult Results;
				MeasureMemory.f_GetResults(Results);

#if DMibConfig_Memory_Shims_Enable
				DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesAlloc.m_Average) == DMibExpr(Results.m_AllAllocations.m_BytesFree.m_Average));
#endif
			};

			DMibTestSuite("All sizes batch")
			{
				CTestMemoryMeasure MeasureMemory("Alloc");

				NMib::NContainer::TCVector<CAlloc> Allocs;

				MeasureMemory.f_Start();

				{
					TCMemoryManagerTracked<TCMemoryManager<CParamsNoCleanup>> MemoryManager("Test", CMemoryManagerConfig());
					MemoryManager.f_ForceStartCleanupThreads();
					umint LastAlloc = 0;
					auto Checkout = MemoryManager.f_CheckoutForce();
					for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
					{
						umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
						if (AllocSize != LastAlloc || MemorySize < 1024)
						{
							LastAlloc = AllocSize;
							umint Size = MemorySize;
							umint nAllocs = 16;
							MemoryManager.f_AllocBatch
								(
									Size
									, 1
									, [&](void * _pAlloc, umint _Size)
									{

										{
#if DMibConfig_Memory_Shims_Enable

											NMib::NMemory::CDisableMemoryReporterScope DisableReport;
#endif
											DMibTest(DMibExpr(_Size) >= DMibExpr(MemorySize))(ETestFlag_Aggregated);
											Allocs.f_Insert({_pAlloc, _Size});
										}

										if ((nAllocs--) == 0)
											return false;
										return true;
									}
								)
							;
							{
							}
						}
					}
					for (auto &Alloc : Allocs)
						MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);

					MemoryManager.f_GarbageCollect(true);
				}

				MeasureMemory.f_Stop(1);

				NMib::NTest::CTestMemoryResult Results;
				MeasureMemory.f_GetResults(Results);

#if DMibConfig_Memory_Shims_Enable
				DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesAlloc.m_Average) == DMibExpr(Results.m_AllAllocations.m_BytesFree.m_Average));
#endif
			};

			DMibTestSuite("Big allocs")
			{
				TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
				auto Checkout = MemoryManager.f_CheckoutForce();
				umint LastAlloc = 0;
				for (umint MemorySize = CParamsNoCleanup::mc_MaxSlabAllocSize * 2; MemorySize <= CParamsNoCleanup::mc_SlabSize * 4; MemorySize *= 2)
				{
					umint AllocSize = MemorySize;
					if (AllocSize != LastAlloc)
					{
						LastAlloc = AllocSize;
						DMibTestPath(NMib::NStr::CStr::CFormat("{}") << AllocSize);

						umint Size = AllocSize;
						auto pAlloc = MemoryManager.f_AllocWithSize(Size);
						DMibTest(DMibExpr(true));
						MemoryManager.f_Free(pAlloc, Size);
					}
				}
			};

			DMibTestSuite("Multiple sub slabs")
			{
				TCMemoryManager<CParamsNoCleanup> MemoryManagerTest{CMemoryManagerConfig()};
				umint LastAlloc = 0;

				for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
				{
					umint AllocSize = MemoryManagerTest.f_SizePadded(MemorySize);
					if (AllocSize != LastAlloc)
					{
						LastAlloc = AllocSize;
						DMibTestPath(NMib::NStr::CStr::CFormat("{}") << AllocSize);

						umint nAlloc = (CParamsNoCleanup::mc_MaxSubSlabMultipliedSize * 3) / AllocSize;

						TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
						auto Checkout = MemoryManager.f_CheckoutForce();
						NMib::NContainer::TCVector<void *> Allocs;
						Allocs.f_SetLen(nAlloc);
						for (umint i = 0; i < nAlloc; ++i)
						{
							umint Size = AllocSize;
							Allocs[i] = MemoryManager.f_AllocWithSize(Size);
						}
						for (umint i = 0; i < nAlloc; ++i)
						{
							MemoryManager.f_Free(Allocs[i], AllocSize);
						}
						DMibTest(DMibExpr(true));
					}
				}
			};

			DMibTestSuite("Multiple slabs")
			{
				TCMemoryManager<CParamsNoCleanup> MemoryManagerTest{CMemoryManagerConfig()};

				umint LastAlloc = 0;
				for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
				{
					umint AllocSize = MemoryManagerTest.f_SizePadded(MemorySize);
					if (AllocSize != LastAlloc)
					{
						LastAlloc = AllocSize;
						if (AllocSize < 512)
							continue;
						DMibTestPath(NMib::NStr::CStr::CFormat("{}") << AllocSize);

						TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
						auto Checkout = MemoryManager.f_CheckoutForce();
						umint nAlloc = (CParamsNoCleanup::mc_SlabSize * 4) / MemorySize;
						NMib::NContainer::TCVector<void *> Allocs;
						Allocs.f_SetLen(nAlloc);
						for (umint i = 0; i < nAlloc; ++i)
						{
							umint Size = AllocSize;
							Allocs[i] = MemoryManager.f_AllocWithSize(Size);
						}
						for (umint i = 0; i < nAlloc; ++i)
						{
							MemoryManager.f_Free(Allocs[i], AllocSize);
						}
						DMibTest(DMibExpr(true));
					}
				}
			};

			DMibTestSuite("Aligned")
			{
				umint LastAlloc = 0;
				umint LastAlignment = 1;
				TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
				auto Checkout = MemoryManager.f_CheckoutForce();
				uint8 * pLast = nullptr;
				for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxHeapAllocSize * 2; ++MemorySize)
				{
					umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
					if (AllocSize != LastAlloc)
					{
						for (umint Alignment = 1; Alignment <= CParamsNoCleanup::mc_MaxHeapAllocSize * 2; Alignment <<= 1)
						{
							if (pLast)
								MemoryManager.f_Free(pLast, NMib::fg_AlignUp(LastAlloc, LastAlignment));

							umint Size = AllocSize;
							pLast = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, Alignment);
							LastAlloc = AllocSize;
							LastAlignment = Alignment;
							DMibTest(DMibExpr(pLast) == DMibExpr(NMib::fg_AlignUp(pLast, Alignment)))(ETestFlag_Aggregated);
						}
					}
				}
				if (pLast)
					MemoryManager.f_Free(pLast, NMib::fg_AlignUp(LastAlloc, LastAlignment));
			};

			DMibTestSuite("Commit")
			{
				[[maybe_unused]] umint LastAlloc = 0;
				TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
				umint SlabSize = CParamsNoCleanup::mc_SlabSize;

				{
					DMibTestPath("Alloc1");
					auto Checkout = MemoryManager.f_CheckoutForce();
					umint nAllocs = (SlabSize / 2) / 256;
					NMib::NContainer::TCVector<CAlloc> Allocs;

					Allocs.f_SetLen(nAllocs);

					CTestMemoryMeasure MeasureMemory("Alloc");

					MeasureMemory.f_Start();

					for (umint i = 0; i < nAllocs; ++i)
					{
						umint Size = 256;
						Allocs[i] = {MemoryManager.f_AllocWithSize(Size), Size};
					}

					for (umint i = 0; i < nAllocs; ++i)
						MemoryManager.f_Free(Allocs[i].m_pAlloc, Allocs[i].m_Size);

					MeasureMemory.f_Stop(1);

					NMib::NTest::CTestMemoryResult Results;
					MeasureMemory.f_GetResults(Results);

#	if DMibConfig_Memory_Shims_Enable
					umint MetaCommit = CParamsNoCleanup::fs_GetSlabTypeMetaSize(0);
					DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesCommit.m_Average) == DMibExpr(SlabSize / 2 + MetaCommit));
#	endif
				}
				[[maybe_unused]] umint MaxCommittedSubSlabs = 0;
				MemoryManager.f_GarbageCollect(false);
				[[maybe_unused]] umint PreviousWaste;
				{
					DMibTestPath("Alloc2");
					auto Checkout = MemoryManager.f_CheckoutForce();
					umint nAllocs = (SlabSize / 2) / 384;
					NMib::NContainer::TCVector<CAlloc> Allocs;

					Allocs.f_SetLen(nAllocs);

					CTestMemoryMeasure MeasureMemory("Alloc");

					MeasureMemory.f_Start();

					for (umint i = 0; i < nAllocs; ++i)
					{
						umint Size = 384;
						Allocs[i] = {MemoryManager.f_AllocWithSize(Size), Size};
					}

					for (umint i = 0; i < nAllocs; ++i)
						MemoryManager.f_Free(Allocs[i].m_pAlloc, Allocs[i].m_Size);

					MeasureMemory.f_Stop(1);

					NMib::NTest::CTestMemoryResult Results;
					MeasureMemory.f_GetResults(Results);

#	if DMibConfig_Memory_Shims_Enable
					umint PreviousMetaSubSlabs = CParamsNoCleanup::fs_GetSlabTypeMetaSize(0) / CParamsNoCleanup::mc_SubSlabSize;
					umint MetaSubSlabs = CParamsNoCleanup::fs_GetSlabTypeMetaSize(4) / CParamsNoCleanup::mc_SubSlabSize;
					umint Multiplier = CParamsNoCleanup::mc_SlabTypeInfo[4].m_SubSlabMutiplier;
					umint SubSlabSize = Multiplier * CParamsNoCleanup::mc_SubSlabSize;
					umint PreviousCommittedSubSlabs = (SlabSize / 2) / CParamsNoCleanup::mc_SubSlabSize;
					umint CommittedSubSlabs = ((PreviousCommittedSubSlabs + Multiplier - 1) / Multiplier) * Multiplier;
					MaxCommittedSubSlabs = NMib::fg_Max(CommittedSubSlabs, MaxCommittedSubSlabs, PreviousCommittedSubSlabs);
					umint Waste = CParamsNoCleanup::mc_SlabSize - MetaSubSlabs * CParamsNoCleanup::mc_SubSlabSize - CParamsNoCleanup::mc_NumSubSlabs[4] * SubSlabSize;
					PreviousWaste = Waste;
					DMibExpect(Results.m_AllAllocations.m_BytesDecommit.m_Average, ==, Waste);
					// Re-typing the slab expands each partially committed range outward to whole
					// multiplier-sized sub-slab groups, costing up to Multiplier - 1 sub-slabs per
					// range. Arena block lists leave a single contiguous committed range, so the
					// edge cost is exact; the sub-slab stores can leave a scattered committed set
					// whose range count depends on the mode and page size, so only bound the total
					// edge overhead: it has to stay far below re-committing the transferred set
					if constexpr (CParamsNoCleanup::mc_FreeStoreMode == EMemoryManagerFreeStore_ArenaBlockLists)
						DMibExpect(Results.m_AllAllocations.m_BytesCommit.m_Average, ==, (CommittedSubSlabs - PreviousCommittedSubSlabs) * CParamsNoCleanup::mc_SubSlabSize);
					else
						DMibExpect(Results.m_AllAllocations.m_BytesCommit.m_Average, <=, PreviousCommittedSubSlabs * CParamsNoCleanup::mc_SubSlabSize / 32);
#	endif
				}
				MemoryManager.f_GarbageCollect(false);
				{
					DMibTestPath("Alloc3");
					auto Checkout = MemoryManager.f_CheckoutForce();
					umint nAllocs = (SlabSize / 2) / 256;
					NMib::NContainer::TCVector<CAlloc> Allocs;

					Allocs.f_SetLen(nAllocs);

					CTestMemoryMeasure MeasureMemory("Alloc");

					MeasureMemory.f_Start();

					for (umint i = 0; i < nAllocs; ++i)
					{
						umint Size = 256;
						Allocs[i] = {MemoryManager.f_AllocWithSize(Size), Size};
					}

					for (umint i = 0; i < nAllocs; ++i)
						MemoryManager.f_Free(Allocs[i].m_pAlloc, Allocs[i].m_Size);

					MeasureMemory.f_Stop(1);

					NMib::NTest::CTestMemoryResult Results;
					MeasureMemory.f_GetResults(Results);

					MaxCommittedSubSlabs = (SlabSize / 2) / CParamsNoCleanup::mc_SubSlabSize;

#	if DMibConfig_Memory_Shims_Enable
					DMibExpect(Results.m_AllAllocations.m_BytesDecommit.m_Average, ==, 0);
					DMibExpect(Results.m_AllAllocations.m_BytesCommit.m_Average, ==, PreviousWaste);
#	endif
				}
				MemoryManager.f_GarbageCollect(false);
				{
					DMibTestPath("Alloc4");
					auto Checkout = MemoryManager.f_CheckoutForce();
					umint AllocSize = 256 + (258/8)*3;
					umint nAllocs = (SlabSize / 2) / AllocSize;
					umint nAllocBytes = nAllocs * AllocSize;
					NMib::NContainer::TCVector<CAlloc> Allocs;

					Allocs.f_SetLen(nAllocs);

					CTestMemoryMeasure MeasureMemory("Alloc");

					MeasureMemory.f_Start();

					for (umint i = 0; i < nAllocs; ++i)
					{
						umint Size = AllocSize;
						Allocs[i] = {MemoryManager.f_AllocWithSize(Size), Size};
					}

					for (umint i = 0; i < nAllocs; ++i)
						MemoryManager.f_Free(Allocs[i].m_pAlloc, Allocs[i].m_Size);

					MeasureMemory.f_Stop(1);

					NMib::NTest::CTestMemoryResult Results;
					MeasureMemory.f_GetResults(Results);

#	if DMibConfig_Memory_Shims_Enable
					// TODO: Calculate actual start and end to get correct sizes
					umint MetaSubSlabs = CParamsNoCleanup::fs_GetSlabTypeMetaSize(3) / CParamsNoCleanup::mc_SubSlabSize;
					umint Multiplier = CParamsNoCleanup::mc_SlabTypeInfo[3].m_SubSlabMutiplier;
					umint SubSlabSize = Multiplier * CParamsNoCleanup::mc_SubSlabSize;
					umint Waste = CParamsNoCleanup::mc_SlabSize - MetaSubSlabs * CParamsNoCleanup::mc_SubSlabSize - CParamsNoCleanup::mc_NumSubSlabs[3] * SubSlabSize;
					DMibExpect(Results.m_AllAllocations.m_BytesDecommit.m_Average, ==, Waste);
					DMibExpect(Results.m_AllAllocations.m_BytesCommit.m_Average, >=, CParamsNoCleanup::mc_SubSlabSize);
#	endif
				}
			};

			DMibTestCategory("Garbage collection")
			{
				DMibTestSuite("Full")
				{
					umint LastAlloc = 0;
					TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
					MemoryManager.f_ForceStartCleanupThreads();

					for (umint i = 0; i < 2; ++i)
					{
						ch8 const *pPath = "First Pass";
						if (i == 1)
							pPath = "Second Pass";
						DMibTestPath(pPath);
						NMib::NContainer::TCVector<CAlloc> Allocs;

						for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
						{
							umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
							if (AllocSize != LastAlloc)
							{
								LastAlloc = AllocSize;
								umint Size = MemorySize;
								auto pMemory = MemoryManager.f_AllocWithSize(Size);
								umint ReturnedSize = MemoryManager.f_Size(pMemory);
								Allocs.f_Insert({pMemory, MemorySize});
								umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
								DMibTest(DMibExpr(AllocSize) == DMibExpr(Size))(ETestFlag_Aggregated);
								DMibTest(DMibExpr(Size) >= DMibExpr(MemorySize))(ETestFlag_Aggregated);
								DMibTest(DMibExpr(ReturnedSize) == DMibExpr(Size))(ETestFlag_Aggregated);
							}
						}
						for (umint MemorySize = CParamsNoCleanup::mc_MaxSlabAllocSize * 2; MemorySize <= CParamsNoCleanup::mc_SlabSize * 4; MemorySize *= 2)
						{
							umint AllocSize = MemorySize;
							if (AllocSize != LastAlloc)
							{
								LastAlloc = AllocSize;
								umint Size = AllocSize;
								auto pAlloc = MemoryManager.f_AllocWithSize(Size);
								Allocs.f_Insert({pAlloc, AllocSize});
							}
						}

						DMibTest(DMibExpr(MemoryManager.f_GetNumUsedSlabs()) > DMibExpr(2u));
						DMibTest(DMibExpr(MemoryManager.f_GetNumFreeSlabs()) == DMibExpr(0));

						for (auto &Alloc : Allocs)
							MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);

						CTestMemoryMeasure MeasureMemory("Alloc");
						MeasureMemory.f_Start();
						MemoryManager.f_GarbageCollect(true);
						MeasureMemory.f_Stop(1);

						NMib::NTest::CTestMemoryResult Results;
						MeasureMemory.f_GetResults(Results);

#	if DMibConfig_Memory_Shims_Enable
						DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesDecommit.m_Average) > DMibExpr(0));
#	endif

						DMibTest(DMibExpr(MemoryManager.f_GetNumUsedSlabs()) == DMibExpr(0));
						DMibTest(DMibExpr(MemoryManager.f_GetNumFreeSlabs()) == DMibExpr(1u));
					}
				};

				DMibTestSuite("Partial")
				{
					umint LastAlloc = 0;
					TCMemoryManager<CParamsNoCleanup> MemoryManager{CMemoryManagerConfig()};
					MemoryManager.f_ForceStartCleanupThreads();

					for (umint i = 0; i < 2; ++i)
					{
						ch8 const *pPath = "First Pass";
						if (i == 1)
							pPath = "Second Pass";
						DMibTestPath(pPath);
						NMib::NContainer::TCVector<CAlloc> Allocs;
						NMib::NContainer::TCVector<CAlloc> BigAllocs;

						for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
						{
							umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
							if (AllocSize != LastAlloc)
							{
								LastAlloc = AllocSize;
								umint Size = MemorySize;
								auto pMemory = MemoryManager.f_AllocWithSize(Size);
								umint ReturnedSize = MemoryManager.f_Size(pMemory);
								Allocs.f_Insert({pMemory, MemorySize});
								umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
								DMibTest(DMibExpr(AllocSize) == DMibExpr(Size))(ETestFlag_Aggregated);
								DMibTest(DMibExpr(Size) >= DMibExpr(MemorySize))(ETestFlag_Aggregated);
								DMibTest(DMibExpr(ReturnedSize) == DMibExpr(Size))(ETestFlag_Aggregated);
							}
						}
						for (umint MemorySize = CParamsNoCleanup::mc_MaxSlabAllocSize * 2; MemorySize <= CParamsNoCleanup::mc_SlabSize * 4; MemorySize *= 2)
						{
							umint AllocSize = MemorySize;
							if (AllocSize != LastAlloc)
							{
								LastAlloc = AllocSize;
								umint Size = AllocSize;
								auto pAlloc = MemoryManager.f_AllocWithSize(Size);
								BigAllocs.f_Insert({pAlloc, AllocSize});
							}
						}

						DMibTest(DMibExpr(MemoryManager.f_GetNumUsedSlabs()) > DMibExpr(2u));
						DMibTest(DMibExpr(MemoryManager.f_GetNumFreeSlabs()) == DMibExpr(0));

						for (umint i = 1; i < Allocs.f_GetLen(); ++i)
							MemoryManager.f_Free(Allocs[i].m_pAlloc, Allocs[i].m_Size);

						{
							CTestMemoryMeasure MeasureMemory("Alloc");
							MeasureMemory.f_Start();
							MemoryManager.f_GarbageCollect(true);
							MeasureMemory.f_Stop(1);
							NMib::NTest::CTestMemoryResult Results;
							MeasureMemory.f_GetResults(Results);
#	if DMibConfig_Memory_Shims_Enable
							DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesDecommit.m_Average) > DMibExpr(0));
#	endif
						}

						for (umint i = 0; i < BigAllocs.f_GetLen(); ++i)
							MemoryManager.f_Free(BigAllocs[i].m_pAlloc, BigAllocs[i].m_Size);

						CTestMemoryMeasure MeasureMemory("Alloc");
						MeasureMemory.f_Start();
						MemoryManager.f_GarbageCollect(true);
						MeasureMemory.f_Stop(1);

						NMib::NTest::CTestMemoryResult Results;
						MeasureMemory.f_GetResults(Results);

#	if DMibConfig_Memory_Shims_Enable
						DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesDecommit.m_Average) > DMibExpr(0) && DMibExpr("Second"));
#	endif
						umint nSlabs = MemoryManager.f_GetNumUsedSlabs();

						DMibTest(DMibExpr(MemoryManager.f_GetNumUsedSlabs()) == DMibExpr(1u));
						DMibTest(DMibExpr(MemoryManager.f_GetNumFreeSlabs()) == DMibExpr(1u));

						MemoryManager.f_Free(Allocs[0].m_pAlloc, Allocs[0].m_Size);
						MemoryManager.f_GarbageCollect(true);

						DMibTest(DMibExpr(MemoryManager.f_GetNumUsedSlabs()) == DMibExpr(0));
						DMibTest(DMibExpr(MemoryManager.f_GetNumFreeSlabs()) == DMibExpr(1u) && DMibExpr("Second"));
					}
				};
			};

#if !(defined(DMibSanitizerEnabled_Address) && DMibPPtrBits <= 32)
			using CParamsBackgroundTest = TCMemoryManagerParams<TCParamsBackgroundTest<tf_CParams>>;
			DMibTestSuite("BackgroundCleanup")
			{
				TCMemoryManager<CParamsBackgroundTest> MemoryManager{CMemoryManagerConfig()};

				MemoryManager.f_ForceStartCleanupThreads();
				MemoryManager.f_GarbageCollect(true); // Make sure that the thread local is created for this thread

				NMib::NContainer::TCVector<NMib::NStorage::TCUniquePointer<NMib::NThread::CThreadObject>> StartedThreads;
				{
					CTestMemoryMeasure MeasureMemory("Alloc");
					MeasureMemory.f_Start();
					MeasureMemory.f_Stop(1);
				}

				umint nThreads = 16;
#if DMibPPtrBits <= 32
				nThreads = 4;
#endif
				for (umint i = 0; i < nThreads; ++i)
				{
					StartedThreads.f_Insert
						(
							NMib::NThread::CThreadObject::fs_StartThread
							(
								[&](NMib::NThread::CThreadObject * _pThread) -> aint
								{
									NMib::NSys::fg_Thread_Sleep(fp64(0.1) + NMib::NMisc::fg_GetRandomFloat()*0.1);

									for (int i = 0; i < 64; ++i)
									{
										NMib::NContainer::TCVector<CAlloc> Allocs;
										NMib::NContainer::TCVector<CAlloc> BigAllocs;
										NMib::NContainer::TCVector<CAlloc> HugeAllocs;
										auto Checkout = MemoryManager.f_CheckoutForce();
										umint LastAlloc = 0;
										for (umint MemorySize = 1; MemorySize <= CParamsNoCleanup::mc_MaxSlabAllocSize; ++MemorySize)
										{
											umint AllocSize = MemoryManager.f_SizePadded(MemorySize);
											if (AllocSize != LastAlloc || MemorySize < 1024)
											{
												LastAlloc = AllocSize;
												umint Size = MemorySize;
												auto pMemory = MemoryManager.f_AllocWithSize(Size);
												Allocs.f_Insert({pMemory, MemorySize});
											}
										}

										for (umint MemorySize = CParamsNoCleanup::mc_MaxSlabAllocSize * 2; MemorySize <= CParamsNoCleanup::mc_MaxHeapAllocSize; MemorySize *= 2)
										{
											umint AllocSize = MemorySize;
											if (AllocSize != LastAlloc)
											{
												LastAlloc = AllocSize;
												umint Size = AllocSize;
												auto pAlloc = MemoryManager.f_AllocWithSize(Size);
												BigAllocs.f_Insert({pAlloc, AllocSize});
											}
										}

										for (umint MemorySize = CParamsNoCleanup::mc_MaxHeapAllocSize * 2; MemorySize <= CParamsNoCleanup::mc_SlabSize * 4; MemorySize *= 2)
										{
											umint AllocSize = MemorySize;
											if (AllocSize != LastAlloc)
											{
												LastAlloc = AllocSize;
												umint Size = AllocSize;
												auto pAlloc = MemoryManager.f_AllocWithSize(Size);
												HugeAllocs.f_Insert({pAlloc, AllocSize});
											}
										}

										for (auto &Alloc : Allocs)
											MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);
										for (auto &Alloc : BigAllocs)
											MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);
										for (auto &Alloc : HugeAllocs)
											MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);
										NMib::NSys::fg_Thread_Sleep(fp64(0.005) + NMib::NMisc::fg_GetRandomFloat()*0.005);
									}

									return 0;
								}
								, "Test cleanup memory manager"
							)
						)
					;
				}

				StartedThreads.f_Clear();
				NMib::NSys::fg_Thread_Sleep(fp64(0.002));
				MemoryManager.f_WaitForBackgroundCleanup();
				{
					{
						// Do a checkout to precache the thread local, so this does not get measured.
						MemoryManager.f_CheckoutForce();
					}
					CTestMemoryMeasure MeasureMemory("Alloc");
					MeasureMemory.f_Start();
					MemoryManager.f_GarbageCollect(true);
					MeasureMemory.f_Stop(1);

					NMib::NTest::CTestMemoryResult Results;
					MeasureMemory.f_GetResults(Results);

					DMibTest(DMibExpr(Results.m_AllAllocations.m_nAllocations.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nFree.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nResize.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nRealloc.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nGetSize.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nProtect.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nCommit.m_Average) == DMibExpr(0));
					DMibTest(DMibExpr(Results.m_AllAllocations.m_nDecommit.m_Average) == DMibExpr(0));
				}
			};
#endif
		}

		template <umint t_PageSize>
		struct TCTestParamsNoSmallSize : public TCParams<t_PageSize, 8>
		{
			static constexpr bool mc_bUseSmallSizes = false;
		};

		void f_DoTests()
		{
			DMibTestCategory("Default")
			{
				if constexpr (gc_OsMaxPageSize != 4096)
				{
					DMibTestCategory("OsMaxPageSize")
					{
						f_TestMemory<TCParams<gc_TestPageSize, 8>, gc_TestPageSize>();
					};
				}

				DMibTestCategory("4096")
				{
					f_TestMemory<TCParams<4096, 8>, 4096>();
				};
			};
#if DMibPPtrBits >= 64
			DMibTestCategory("NoSmallSize")
			{
				if constexpr (gc_OsMaxPageSize != 4096)
				{
					DMibTestCategory("OsMaxPageSize")
					{
						f_TestMemory<TCTestParamsNoSmallSize<gc_OsMaxPageSize>, gc_OsMaxPageSize>();
					};
				}

				DMibTestCategory("4096")
				{
					f_TestMemory<TCTestParamsNoSmallSize<4096>, 4096>();
				};
			};
#endif
#if !defined(DMibSanitizerEnabled_Thread) && !defined(DCompiler_MSVC_Workaround_DllsBroken)
			// tsan does not currently support unloading dlls
			DMibTestSuite("Dll")
			{
				NMib::NStr::CStr DllPath = NMib::NStr::CStr("Test_Malterlib_Helper_Memory") + NMib::NFile::CFile::fs_GetDllExtension();
#ifdef DPlatformFamily_Linux
				DllPath = NMib::NFile::CFile::fs_AppendPath(NMib::NFile::CFile::fs_GetProgramDirectory(), DllPath);
#endif
				auto pDll = NMib::NSys::fg_LoadLibrary(DllPath);

				DMibTest(DMibExpr(pDll))(ETest_FailAndStop);

				void (calling_convention_c *pTestFunc)() = nullptr;

				(void * &)pTestFunc = NMib::NSys::fg_GetLibrarySymbol(pDll, "fg_TestMemory");
				DMibTest(DMibExpr(pTestFunc))(ETest_FailAndStop);

				pTestFunc();

				NMib::NSys::fg_FreeLibrary(pDll);
			};
#endif
		}
	};

	DMibTestRegister(CBasics_Tests, Malterlib::Memory::MemoryManager);

}
