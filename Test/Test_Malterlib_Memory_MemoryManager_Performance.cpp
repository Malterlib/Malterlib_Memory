// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>
#include <Mib/Test/Memory>
#include <Mib/Test/Performance>
#include <Mib/Test/ResultParser>
#include <Mib/Process/ProcessLaunch>
#include "Test_Malterlib_Memory_MemoryManager_Performance.h"

namespace
{
	using namespace NMib::NTest;
	using namespace NMib::NMemory;

	constexpr umint gc_TestSize = 512;
	constexpr umint gc_TestSizeEnd = 4096;
#if defined(DMibDebug) || defined(DMibSanitizerEnabled)
	constexpr umint gc_TestMaxMemory = 256*1024*1;
#else
	constexpr umint gc_TestMaxMemory = 256*1024*1024;
#endif
#if defined(DMibDebug) || defined(DMibSanitizerEnabled)
	constexpr umint gc_ArrayLimit = 128u;
#else
	constexpr umint gc_ArrayLimit = 16u * 1024u;
#endif


#if 0
	struct CDisplayStats
	{
		template <typename t_CParams, umint t_Index>
		struct TCGetWaste
		{
			static constexpr umint mc_Waste =
				t_CParams::mc_SlabSize
				-
				(
					TCMemoryManagerSlab<t_CParams, t_Index>::mc_nSubSlabs * t_CParams::mc_SlabTypeInfo[t_Index].m_SubSlabMutiplier * t_CParams::mc_SubSlabSize
					+ NMib::fg_AlignUpConstExpr(sizeof(TCMemoryManagerSlab<t_CParams, t_Index>), t_CParams::mc_SubSlabSize)
				)
			;
			static constexpr double mc_WastePercent = (double(mc_Waste) / double(t_CParams::mc_SlabSize)) * 100.0;
		};

		template <typename t_CParams, umint t_Index>
		struct TCGetOverhead
		{
			static constexpr umint mc_Overhead =
				t_CParams::mc_SlabSize - TCMemoryManagerSlab<t_CParams, t_Index>::mc_nSubSlabs * t_CParams::mc_SlabTypeInfo[t_Index].m_SubSlabMutiplier * t_CParams::mc_SubSlabSize
			;
			static constexpr double mc_OverheadPercent = (double(mc_Overhead) / double(t_CParams::mc_SlabSize)) * 100.0;
		};

		struct CPageSize4096 : public CDefaultMemoryManagerParams
		{
			static constexpr umint mc_SubSlabSize = 4096;
		};

		CDisplayStats()
		{
			auto fDisplayParams = []<typename tf_CParams>
				{
					using CParams = tf_CParams;
					NMib::NStr::CStr Type = NMib::fg_GetTypeName<tf_CParams>();

					// Stats
					DMibConOut("{}::mc_SlabSize = {}\r\n", Type, CParams::mc_SlabSize);
					DMibConOut("{}::mc_MaxSlabAllocSize = {}\r\n", Type, CParams::mc_MaxSlabAllocSize);
					DMibConOut("{}::mc_NumSizeLevels = {}\r\n", Type, CParams::mc_NumSizeLevels);
					DMibConOut("{}::mc_NumNormalSizeLevels = {}\r\n", Type, CParams::mc_NumNormalSizeLevels);
					DMibConOut("{}::CSubSlabIndex = {}\r\n", Type, NMib::fg_GetTypeName<typename CParams::CSubSlabIndex>());
					DMibConOut("{}::CNumAllocsPerSubSlabIndex = {}\r\n", Type, NMib::fg_GetTypeName<typename CParams::CNumAllocsPerSubSlabIndex>());
					DMibConOut("{}::fs_GetSlabTypeMetaSize(0) = {}\r\n", Type, CParams::fs_GetSlabTypeMetaSize(0));
					DMibConOut("{}::mc_MinNormalSlabBucket = {}\r\n", Type, CParams::mc_MinNormalSlabBucket);

					DMibConOut("{} Waste 0 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 0>::mc_Waste, TCGetWaste<CParams, 0>::mc_WastePercent);
					DMibConOut("{} Waste 1 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 1>::mc_Waste, TCGetWaste<CParams, 1>::mc_WastePercent);
					DMibConOut("{} Waste 2 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 2>::mc_Waste, TCGetWaste<CParams, 2>::mc_WastePercent);
					DMibConOut("{} Waste 3 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 3>::mc_Waste, TCGetWaste<CParams, 3>::mc_WastePercent);
					DMibConOut("{} Waste 4 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 4>::mc_Waste, TCGetWaste<CParams, 4>::mc_WastePercent);
					DMibConOut("{} Waste 5 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 5>::mc_Waste, TCGetWaste<CParams, 5>::mc_WastePercent);
					DMibConOut("{} Waste 6 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 6>::mc_Waste, TCGetWaste<CParams, 6>::mc_WastePercent);
					DMibConOut("{} Waste 7 = {} {fe2}%\r\n", Type, TCGetWaste<CParams, 7>::mc_Waste, TCGetWaste<CParams, 7>::mc_WastePercent);

					DMibConOut("{} Overhead 0 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 0>::mc_Overhead, TCGetOverhead<CParams, 0>::mc_OverheadPercent);
					DMibConOut("{} Overhead 1 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 1>::mc_Overhead, TCGetOverhead<CParams, 1>::mc_OverheadPercent);
					DMibConOut("{} Overhead 2 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 2>::mc_Overhead, TCGetOverhead<CParams, 2>::mc_OverheadPercent);
					DMibConOut("{} Overhead 3 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 3>::mc_Overhead, TCGetOverhead<CParams, 3>::mc_OverheadPercent);
					DMibConOut("{} Overhead 4 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 4>::mc_Overhead, TCGetOverhead<CParams, 4>::mc_OverheadPercent);
					DMibConOut("{} Overhead 5 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 5>::mc_Overhead, TCGetOverhead<CParams, 5>::mc_OverheadPercent);
					DMibConOut("{} Overhead 6 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 6>::mc_Overhead, TCGetOverhead<CParams, 6>::mc_OverheadPercent);
					DMibConOut("{} Overhead 7 = {} {fe2}%\r\n", Type, TCGetOverhead<CParams, 7>::mc_Overhead, TCGetOverhead<CParams, 7>::mc_OverheadPercent);

					DMibConOut("{}::mc_NumAllocsPerSubSlab[0] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[0]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[1] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[1]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[2] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[2]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[3] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[3]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[4] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[4]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[5] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[5]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[6] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[6]);
					DMibConOut("{}::mc_NumAllocsPerSubSlab[7] = {}\r\n", Type, CParams::mc_NumAllocsPerSubSlab[7]);
					DMibConOut("{}::mc_MaxAllocsPerSubSlab = {}\r\n", Type, CParams::mc_MaxAllocsPerSubSlab);
					DMibConOut("TCMemoryManagerSubSlabDataAlloc<{}>::mc_MaxAllocs = {}\r\n", Type, TCMemoryManagerSubSlabDataAlloc<CParams>::mc_MaxAllocs);
					DMibConOut("TCMemoryManagerSubSlabDataType<{}>::mc_MaxType = {}\r\n", Type, TCMemoryManagerSubSlabDataType<CParams>::mc_MaxType);
					DMibConOut("sizeof(TCMemoryManagerSubSlabDataAlloc<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlabDataAlloc<CParams>));
					DMibConOut("sizeof(TCMemoryManagerSubSlabDataType<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlabDataType<CParams>));

					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 0>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 0>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 0>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 1>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 1>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 1>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 2>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 2>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 2>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 3>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 3>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 3>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 4>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 4>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 4>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 5>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 5>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 5>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 6>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 6>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 6>), CParams::mc_SubSlabSize));
					DMibConOut("sizeof(TCMemoryManagerSlab<{}, 7>) = {} {}\r\n", Type, sizeof(TCMemoryManagerSlab<CParams, 7>), NMib::fg_AlignUp(sizeof(TCMemoryManagerSlab<CParams, 7>), CParams::mc_SubSlabSize));
					DMibConOut("TCMemoryManagerSlab<{}, 0>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 0>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 1>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 1>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 2>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 2>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 3>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 3>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 4>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 4>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 5>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 5>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 6>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 6>::mc_nSubSlabs));
					DMibConOut("TCMemoryManagerSlab<{}, 7>::mc_nSubSlabs = {}\r\n", Type, umint(TCMemoryManagerSlab<CParams, 7>::mc_nSubSlabs));

					DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<{},1>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlab_SmallSize<CParams,1>));
					DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<{},2>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlab_SmallSize<CParams,2>));
					DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<{},4>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlab_SmallSize<CParams,4>));
					DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<{},8>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlab_SmallSize<CParams,8>));
					DMibConOut("sizeof(TCMemoryManagerSubSlab_SmallSize<{},12>) = {}\r\n", Type, sizeof(TCMemoryManagerSubSlab_SmallSize<CParams,12>));
					DMibConOut("TCMemoryManagerSubSlab_SmallSize<{},1>::mc_NumAllocs = {}\r\n", Type, umint(TCMemoryManagerSubSlab_SmallSize<CParams,1>::mc_NumAllocs));
					DMibConOut("TCMemoryManagerSubSlab_SmallSize<{},2>::mc_NumAllocs = {}\r\n", Type, umint(TCMemoryManagerSubSlab_SmallSize<CParams,2>::mc_NumAllocs));
					DMibConOut("TCMemoryManagerSubSlab_SmallSize<{},4>::mc_NumAllocs = {}\r\n", Type, umint(TCMemoryManagerSubSlab_SmallSize<CParams,4>::mc_NumAllocs));
					DMibConOut("TCMemoryManagerSubSlab_SmallSize<{},8>::mc_NumAllocs = {}\r\n", Type, umint(TCMemoryManagerSubSlab_SmallSize<CParams,8>::mc_NumAllocs));
					DMibConOut("TCMemoryManagerSubSlab_SmallSize<{},12>::mc_NumAllocs = {}\r\n", Type, umint(TCMemoryManagerSubSlab_SmallSize<CParams,12>::mc_NumAllocs));
					DMibConOut("sizeof(TCMemoryManagerArena<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerArena<CParams>));
					DMibConOut("sizeof(TCMemoryManagerNumaArena<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerNumaArena<CParams>));
					DMibConOut("sizeof(TCMemoryManager<{}>) = {}\r\n", Type, sizeof(TCMemoryManager<CParams>));
					DMibConOut("sizeof(TCMemoryManagerArenaHeap<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerArenaHeap<CParams>));
					DMibConOut("sizeof(TCMemoryManagerArenaHeapChunk<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerArenaHeapChunk<CParams>));
					DMibConOut("sizeof(TCMemoryManagerArenaHeapChunk<{}>) = {}\r\n", Type, sizeof(TCMemoryManagerArenaHeapChunk<CParams>));
					DMibConOut("NSys::fg_Mem_PageSize() = {}\r\n\r\n", NMib::NSys::fg_Mem_PageSize());
				}
			;
			fDisplayParams.template operator()<TCMemoryManagerParams<>>();
			fDisplayParams.template operator()<TCMemoryManagerParams<CPageSize4096>>();
		}
	};

	CDisplayStats g_DisplayStats;

#endif

	class CPerformance_Tests : public CTest
	{
	public:

		umint m_nCores;
		CPerformance_Tests()
		{
			m_nCores = NMib::NSys::fg_Thread_GetVirtualCores();
		}

		struct CAllocPattern_Random
		{
			struct CAllocationInfo
			{
//				DMibListLinkD_Link(CAllocationInfo, m_Link);
				CAllocationInfo()
					: m_pAddress(nullptr)
				{
				}
				void *m_pAddress;
			};

			NMib::NMisc::CRandomShiftRNG m_Random;
			uint64 m_nIterations;
			CAllocationInfo *m_pAllocations;
			umint m_ArraySize;
			umint m_MaxAllocatedMemory;
			umint m_MaxAllocSize;
			bool m_bFailed = false;

//			using CAllocInfoList = DMibListLinkD_List(CAllocationInfo, m_Link);
	//		NMib::NMemory::TCPool<CAllocationInfo, 1024*1024*4, NMib::NThread::CNoLock, NMib::NMemory::CPoolType_Growing> m_AllocationInfoPool;


			CAllocPattern_Random()
				: m_pAllocations(nullptr)
			{
			}
			CAllocPattern_Random(umint _MaxAllocSize, umint _iThread, umint _iRepetition, NMib::ENumaNode _NumaNode)
				: m_Random(55556, _iThread, _iRepetition)
				, m_pAllocations(nullptr)
				, m_MaxAllocSize(_MaxAllocSize - 1)
				, m_nIterations(0)
				, m_MaxAllocatedMemory(gc_TestMaxMemory)
				, m_ArraySize(0)
			{
				m_ArraySize = NMib::fg_Min(((m_MaxAllocatedMemory * 2) / _MaxAllocSize), gc_ArrayLimit) - 1;

				umint Size = sizeof(CAllocationInfo) * (m_ArraySize + 1);
				m_pAllocations = (CAllocationInfo *)NMib::NSys::fg_Mem_VirtualAlloc(Size, NMib::EAllocationFlag_None, _NumaNode);
				for (umint i = 0; i <= m_ArraySize; ++i)
					new (m_pAllocations + i) CAllocationInfo();
			}

			static bool fs_Alignment()
			{
				return false;
			}

			umint f_GetIdealAllocations()
			{
				return m_ArraySize * 256;
			}

			~CAllocPattern_Random()
			{
				if (m_pAllocations)
					NMib::NSys::fg_Mem_VirtualFree(m_pAllocations, 0);
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_Random.f_GetValue<uint32>() & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						++m_nIterations;
					}

					{
						umint Size = (m_Random.f_GetValue<uint32>() & m_MaxAllocSize) + 1;
						//umint Size = m_MaxAllocSize + 1;
						Info.m_pAddress = _Heap.f_Alloc(Size);
						*((uint8 *)Info.m_pAddress) = iStart;
						//NMib::NMemory::fg_MemClear((uint8 *)Info.m_pAddress, Size);
						++m_nIterations;
					}
				}
			}

			template <typename tf_CHeap>
			uint64 f_Iterations(tf_CHeap &_Heap)
			{
				auto End = m_pAllocations + m_ArraySize + 1;
				for (auto Iter = m_pAllocations; Iter != End; ++Iter)
				{
					auto &Info = *Iter;
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						Info.m_pAddress = nullptr;
						++m_nIterations;
					}
				}
				uint64 Ret = m_nIterations;
				m_nIterations = 0;
				return Ret;
			}
		};

		struct CAllocPattern_RandomAlignment
		{
			struct CAllocationInfo
			{
//				DMibListLinkD_Link(CAllocationInfo, m_Link);
				CAllocationInfo()
					: m_pAddress(nullptr)
				{
				}
				void *m_pAddress;
			};

			NMib::NMisc::CRandomShiftRNG m_Random;
			uint64 m_nIterations;
			CAllocationInfo *m_pAllocations;
			umint m_ArraySize;
			umint m_MaxAllocatedMemory;
			umint m_MaxAllocSize;
			umint m_AlignBits;
			bool m_bFailed = false;

//			using CAllocInfoList = DMibListLinkD_List(CAllocationInfo, m_Link);
	//		NMib::NMemory::TCPool<CAllocationInfo, 1024*1024*4, NMib::NThread::CNoLock, NMib::NMemory::CPoolType_Growing> m_AllocationInfoPool;


			CAllocPattern_RandomAlignment()
				: m_pAllocations(nullptr)
			{
			}
			CAllocPattern_RandomAlignment(umint _MaxAllocSize, umint _iThread, umint _iRepetition, NMib::ENumaNode _NumaNode)
				: m_Random(55556, _iThread, _iRepetition)
				, m_pAllocations(nullptr)
				, m_MaxAllocSize(_MaxAllocSize - 1)
				, m_nIterations(0)
				, m_MaxAllocatedMemory(gc_TestMaxMemory)
				, m_ArraySize(0)
				, m_AlignBits(NMib::fg_GetHighestBitSet(_MaxAllocSize))
			{
				if (_MaxAllocSize == 0)
					m_AlignBits = 0;
				if (m_AlignBits == 0)
					m_AlignBits = 1;
				m_ArraySize = NMib::fg_Min(((m_MaxAllocatedMemory * 2) / _MaxAllocSize), gc_ArrayLimit) - 1;

				umint Size = sizeof(CAllocationInfo) * (m_ArraySize + 1);
				m_pAllocations = (CAllocationInfo *)NMib::NSys::fg_Mem_VirtualAlloc(Size, NMib::EAllocationFlag_None, _NumaNode);
				for (umint i = 0; i <= m_ArraySize; ++i)
					new (m_pAllocations + i) CAllocationInfo();
			}

			static bool fs_Alignment()
			{
				return true;
			}

			umint f_GetIdealAllocations()
			{
				return m_ArraySize * 256;
			}

			~CAllocPattern_RandomAlignment()
			{
				if (m_pAllocations)
					NMib::NSys::fg_Mem_VirtualFree(m_pAllocations, 0);
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_Random.f_GetValue<uint32>() & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						++m_nIterations;
					}

					{
						umint Size = (m_Random.f_GetValue<uint32>() & m_MaxAllocSize) + 1;
						umint AlignmentBits = m_Random.f_GetValue<uint32>() % m_AlignBits;
						//umint Size = m_MaxAllocSize + 1;
						umint Alignment = umint(1) << AlignmentBits;
						Info.m_pAddress = _Heap.f_AllocAligned(Size, Alignment);
						*((uint8 *)Info.m_pAddress) = iStart;
						if ((umint)Info.m_pAddress & (Alignment - 1))
						{
							m_bFailed = true;
						}

						++m_nIterations;
					}
				}
			}

			template <typename tf_CHeap>
			uint64 f_Iterations(tf_CHeap &_Heap)
			{
				auto End = m_pAllocations + m_ArraySize + 1;
				for (auto Iter = m_pAllocations; Iter != End; ++Iter)
				{
					auto &Info = *Iter;
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						Info.m_pAddress = nullptr;
						++m_nIterations;
					}
				}
				uint64 Ret = m_nIterations;
				m_nIterations = 0;
				return Ret;
			}

		};

		struct CAllocPattern_OneSize
		{
			struct CAllocationInfo
			{
				CAllocationInfo()
					: m_pAddress(nullptr)
				{
				}
				void *m_pAddress;
			};

			NMib::NMisc::CRandomShiftRNG m_Random;
			uint64 m_nIterations;
			CAllocationInfo *m_pAllocations;
			umint m_ArraySize;
			umint m_MaxAllocatedMemory;
			umint m_MaxAllocSize;
			bool m_bFailed = false;

			CAllocPattern_OneSize()
				: m_pAllocations(nullptr)
			{
			}
			CAllocPattern_OneSize(umint _MaxAllocSize, umint _iThread, umint _iRepetition, NMib::ENumaNode _NumaNode)
				: m_Random(55556, _iThread, _iRepetition)
				, m_pAllocations(nullptr)
				, m_MaxAllocSize(_MaxAllocSize)
				, m_nIterations(0)
				, m_MaxAllocatedMemory(gc_TestMaxMemory)
				, m_ArraySize(0)
			{
				m_ArraySize = NMib::fg_Min((m_MaxAllocatedMemory / _MaxAllocSize), gc_ArrayLimit) - 1;

				umint Size = sizeof(CAllocationInfo) * (m_ArraySize + 1);
				m_pAllocations = (CAllocationInfo *)NMib::NSys::fg_Mem_VirtualAlloc(Size, NMib::EAllocationFlag_None, _NumaNode);
				for (umint i = 0; i <= m_ArraySize; ++i)
					new (m_pAllocations + i) CAllocationInfo();
			}

			static bool fs_Alignment()
			{
				return false;
			}

			umint f_GetIdealAllocations()
			{
				return m_ArraySize * 256;
			}

			~CAllocPattern_OneSize()
			{
				if (m_pAllocations)
					NMib::NSys::fg_Mem_VirtualFree(m_pAllocations, 0);
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_Random.f_GetValue<uint32>() & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						++m_nIterations;
					}

					{
						umint Size = m_MaxAllocSize;
						Info.m_pAddress = _Heap.f_Alloc(Size);
						*((uint8 *)Info.m_pAddress) = iStart;
						++m_nIterations;
					}
				}
			}

			template <typename tf_CHeap>
			uint64 f_Iterations(tf_CHeap &_Heap)
			{
				auto End = m_pAllocations + m_ArraySize + 1;
				for (auto Iter = m_pAllocations; Iter != End; ++Iter)
				{
					auto &Info = *Iter;
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						Info.m_pAddress = nullptr;
						++m_nIterations;
					}
				}
				uint64 Ret = m_nIterations;
				m_nIterations = 0;
				return Ret;
			}
		};

		struct CAllocPattern_OneSizeLinear : public CAllocPattern_OneSize
		{
			CAllocPattern_OneSizeLinear()
			{
			}

			CAllocPattern_OneSizeLinear(umint _MaxAllocSize, umint _iThread, umint _iRepetition, NMib::ENumaNode _NumaNode)
				: CAllocPattern_OneSize(_MaxAllocSize, _iThread, _iRepetition, _NumaNode)
			{
			}

			template <typename tf_CHeap>
			inline_small void f_Next(tf_CHeap &_Heap)
			{
				{
					int32 iStart = m_nIterations & m_ArraySize;
					CAllocationInfo &Info = m_pAllocations[iStart];
					if (Info.m_pAddress)
					{
						_Heap.f_FreeNoSize(Info.m_pAddress);
						++m_nIterations;
					}

					{
						umint Size = m_MaxAllocSize;
						Info.m_pAddress = _Heap.f_Alloc(Size);
						*((uint8 *)Info.m_pAddress) = iStart;
						++m_nIterations;
					}
				}
			}
		};

		template <typename t_CHeap, typename t_CAllocPattern>
		struct TCThreadTest : public NMib::NThread::CThread
		{
			TCThreadTest()
				: m_Measure("Baseline")
				, m_pWakeEvent(nullptr)
				, m_iThread(0)
				, m_MaxAllocSize(0)
				, m_pHeap(nullptr)
				, m_iNumaNode(NMib::ENumaNode_Default)
			{
				m_StartedEvent.f_ResetSignaled();
			}
			t_CHeap *m_pHeap;
			NMib::NThread::CEvent *m_pWakeEvent;
			CTestPerformanceMeasure m_Measure;
			NMib::NThread::CEvent m_StartedEvent;
			umint m_iThread;
			umint m_iRepetition;
			umint m_MaxAllocSize;
			NMib::ENumaNode m_iNumaNode;
			virtual aint f_Main()
			{
				if (m_iNumaNode != NMib::ENumaNode_Default)
				{
					NMib::NSys::fg_Thread_SetNumaAffinity(NMib::NSys::fg_Thread_GetCurrent(), m_iNumaNode);
					m_pHeap->f_SetNumaNode(m_iNumaNode);
				}
				// Init heap
				m_pHeap->f_InitThread();
				m_pHeap->f_FreeNoSize(m_pHeap->f_Alloc(1));

				m_StartedEvent.f_SetSignaled();
				m_pWakeEvent->f_Wait();

				auto Checkout = m_pHeap->f_Checkout();
				(void)Checkout;

				auto fInnerLoop = [pHeapPointer = m_pHeap, this]() inline_never
					{
						auto pHeap = pHeapPointer;
						t_CAllocPattern Pattern(m_MaxAllocSize, m_iThread, m_iRepetition, m_iNumaNode);
						int nPattern = Pattern.f_GetIdealAllocations() / 32;
						uint64 nIterations = 0;
						{
							DMibTestScopeMeasure(m_Measure, nIterations);
							for (int i = 0; i < nPattern; ++i)
							{
								for (umint i = 0; i < 32; ++i)
								{
									Pattern.f_Next(*pHeap);
									//Pattern.f_Next(*m_pHeap);
									//Pattern.f_Next(*m_pHeap);
									//Pattern.f_Next(*m_pHeap);
								}
							}
							nIterations = Pattern.f_Iterations(*pHeap);
						}

						if (Pattern.m_bFailed)
							DMibTest(!DMibExpr(Pattern.m_bFailed))(ETestFlag_Aggregated);
					}
				;

				fInnerLoop();

				return 0;
			}

			virtual NMib::NStr::CStr f_GetThreadName()
			{
				return NMib::NStr::CStr::CFormat("Memory manager test {}") << m_iThread;
			}
		};

		template <typename tf_CHeap, typename tf_CAllocPattern>
		void f_DoTestPerform(CTestPerformance &_PerfTest, ETestMeasureType _MeasureType, ch8 const *_pName, umint _MaxAllocSize, umint _nThreads)
		{
			umint nTests = 5;

			tf_CHeap Heap;

			if (!Heap.f_Init(_nThreads, _MaxAllocSize))
				return;

			CTestPerformanceMeasure Measure(_pName);

			NMib::NThread::CEvent WakeEvent;

			umint nNodes = NMib::NSys::fg_Mem_GetNumNumaNodes();
			NMib::NContainer::TCVector<NMib::ENumaNode> Nodes;
			if (nNodes > 0)
			{
				Nodes.f_SetLen(nNodes);
				NMib::NSys::fg_Mem_GetNumaNodes(Nodes.f_GetArray(), nNodes);
			}


			for (umint iRepetition = 0; iRepetition < nTests; ++iRepetition)
			{
				// Reset started
				WakeEvent.f_ResetSignaled();
				NMib::NContainer::TCLinkedList<TCThreadTest<tf_CHeap, tf_CAllocPattern>> Tests;
				// Start threads
				int32 iNode = 0;
				for (umint i = 0; i < _nThreads; ++i)
				{
					auto &Test = Tests.f_Insert();
					Test.m_pWakeEvent = &WakeEvent;
					Test.m_iThread = i;
					Test.m_iRepetition = iRepetition;
					Test.m_MaxAllocSize = _MaxAllocSize;
					Test.m_pHeap = &Heap;
					if (nNodes)
					{
						Test.m_iNumaNode = Nodes[iNode];
						Heap.f_SetNumaNode(Test.m_iNumaNode);
						iNode = (iNode + 1) % nNodes;
					}
					Test.f_Start(NMib::EExecutionPriority_Normal, 0, 0, false);
				}

				// Wait for all threads to start
				for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
					Iter->m_StartedEvent.f_Wait();

				// Start all threads at the same time
				WakeEvent.f_SetSignaled();
				if (_nThreads > m_nCores)
					Measure.f_Start();

				// Wait for all threads to stop
				for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
					Iter->f_Stop();

				if (_nThreads > m_nCores)
				{
					uint64 nIterations = 0;
					for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
						nIterations += Iter->m_Measure.f_Iterations();

					Measure.f_Stop(nIterations, NMib::fg_Min(_nThreads, NMib::NSys::fg_Thread_GetPhysicalCores()));
				}
				else
				{
					// Add measures
					for (auto Iter = Tests.f_GetIterator(); Iter; ++Iter)
						Measure.f_AddThreadResult(Iter->m_Measure);

					Measure.f_NewRepetition();
				}
			}

			if (_MeasureType == ETestMeasureType_Reference)
				_PerfTest.f_AddReference(Measure);
			else if (_MeasureType == ETestMeasureType_Baseline)
				_PerfTest.f_AddBaseline(Measure);
			else if (_MeasureType == ETestMeasureType_Debug)
				_PerfTest.f_AddDebug(Measure);
			else
				_PerfTest.f_Add(Measure);
		}


		template <typename tf_CHeap, typename tf_CAllocPattern>
		void f_DoTest(CTestPerformance &_PerfTest, CTestMemory &_MemoryTest, ETestMeasureType _MeasureType, ch8 const *_pName, umint _MaxAllocSize, umint _nThreads)
		{
			//if (_MeasureType == ETestMeasureType_Debug)
			//	return; // Comment out during debugging?

			if (!(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
			{
				if (!tf_CHeap::fs_ShouldRun(_nThreads, tf_CAllocPattern::fs_Alignment()))
					return;

				CTestPerformanceResult Result;
				CTestMemoryResult MemoryResult;
				struct CLocalParser : public CTestResultParser
				{
					CTestPerformanceResult &m_Result;
					ETestMeasureType m_MeasureType;
					ch8 const *m_pName;
					bool m_bValidResult;
					bool m_bFailedTest = false;
					CLocalParser(CTestPerformanceResult &_Result, ETestMeasureType _MeasureType, ch8 const *_pName)
						: m_Result(_Result)
						, m_MeasureType(_MeasureType)
						, m_pName(_pName)
						, m_bValidResult(false)
					{
					}
					virtual void f_HandleHeader(NMib::NContainer::CRegistry const &_Reg) override
					{
					}
					virtual void f_HandleFooter(NMib::NContainer::CRegistry const &_Reg) override
					{
					}
					virtual void f_HandleCategory(NMib::NContainer::CRegistry const &_Reg) override
					{
					}
					virtual void f_HandleResult(NMib::NContainer::CRegistry const &_Reg) override
					{
						CTestResult Result;
						CTestResultParser::fs_DecodeResult(_Reg, Result);

						if (Result.m_Result == ETestResult_Fail && Result.m_TestPath.f_Find("m_bFailed") >= 0)
						{
							m_bFailedTest = true;
						}

					}
					virtual void f_HandlePerformanceResult(NMib::NContainer::CRegistry const &_Reg) override
					{
						CTestPerformanceResults Results;
						CTestResultParser::fs_DecodePerformanceResults(_Reg, Results);

						DMibCheck(Results.m_Results.f_GetLen() == 1);
						if (!Results.m_Results.f_IsEmpty())
						{
							m_Result = Results.m_Results.f_GetFirst();
							m_bValidResult = true;
						}
					}
					virtual void f_HandleMemoryResult(NMib::NContainer::CRegistry const &_Reg) override
					{
					}
				}
				ResultParser(Result, _MeasureType, _pName);

				NMib::NProcess::CProcessLaunchParams Params;
				Params.m_Target = NMib::NFile::CFile::fs_GetProgramPath();
				Params.m_Parameters = NMib::NProcess::CProcessLaunchParams::fs_GetParams
					(
						{
							"--test"
							, fg_TestGetCurrentPath() / _pName
							, "--logger"
							, "Registry"
							, "--filter-results"
							, "[\"All\"]"
							, "--process-recursive"
							, "--groups"
							, "[\"Performance\"]"
						}
					)
				;
				void *pProcess = nullptr;

				NMib::NThread::CMutual ProcessLaunchLock;
				NMib::NStorage::TCUniquePointer<NMib::NProcess::CProcessLaunch> pProcessLaunch;

				NMib::NThread::CEvent Exited;
				Exited.f_ResetSignaled();

				auto fl_AddStats
					= [&](NMib::NProcess::CProcessStatistics const &_Stats)
					{
						for (auto Iter = _Stats.m_Statistics.f_GetIterator(); Iter; ++Iter)
						{
							fp64 Value = Iter->m_Value;
							NMib::NStr::CStr Name = Iter.f_GetKey();
							switch (Iter->m_Unit)
							{
							case NMib::NProcess::EProcessStatUnit_GeneralNumber:
								{
									if (!Iter->m_CustomUnit.f_IsEmpty())
										Name += NMib::NStr::CStr::CFormat(" ({})") << Iter->m_CustomUnit;
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Bytes:
								{
									if (Iter->m_IdealScale == fp64(1024*1024*1024))
									{
										Value /= Iter->m_IdealScale;
										Name += " (GiB)";
									}
									else if (Iter->m_IdealScale == fp64(1024*1024))
									{
										Value /= Iter->m_IdealScale;
										Name += " (MiB)";
									}
									else if (Iter->m_IdealScale == fp64(1024))
									{
										Value /= Iter->m_IdealScale;
										Name += " (KiB)";
									}
									else
										Name += " (Bytes)";
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Cycles:
								{
									Name += " (Cycles)";
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Seconds:
								{
									if (Iter->m_IdealScale == fp64(0.000000001))
									{
										Value /= Iter->m_IdealScale;
										Name += " (ns)";
									}
									else if (Iter->m_IdealScale == fp64(0.000001))
									{
										Value /= Iter->m_IdealScale;
										Name += " (µs)";
									}
									else if (Iter->m_IdealScale == fp64(0.001))
									{
										Value /= Iter->m_IdealScale;
										Name += " (ms)";
									}
									else
										Name += " (s)";
								}
								break;
							case NMib::NProcess::EProcessStatUnit_Fraction:
								{
									Value *= 100.0;
									Name += " (%)";
								}
								break;
							default:
								DMibNeverGetHere(Iter->m_Unit);
								break;
							}
							Result.m_PerformanceCounters[Name] = CTestStats(Value);
						}
					}
				;

				Params.m_fOnStateChange =
					[&](NMib::NProcess::CProcessLaunchStateChangeVariant const &_State, fp64 _TimeSinceStart)
					{
						switch (_State.f_GetTypeID())
						{
						case NMib::NProcess::EProcessLaunchState_Exited:
							{
								//uint32 ExitCode = _State.f_Get<NMib::NProcess::EProcessLaunchState_Exited>();
								DMibCheck(pProcess);
								if (pProcess)
								{
									DMibLock(ProcessLaunchLock);
									NMib::NProcess::CProcessStatistics MemoryStats = pProcessLaunch->f_GetOverallMemoryStatistics();
									NMib::NProcess::CProcessStatistics ExecutionStats = pProcessLaunch->f_GetOverallExecutionStatistics();


									fl_AddStats(ExecutionStats);
									fl_AddStats(MemoryStats);
									/*
									for (auto Iter = MemoryStats.m_Statistics.f_GetIterator(); Iter; ++Iter)
									{
										if (Iter.f_GetKey() == "Peak working set size")
											MemoryResult.m_AllAllocations.m_BytesMaxAlloc = CTestStats(fp64(*Iter));
										MemoryResult.m_PerAllocationType[Iter.f_GetKey()].m_BytesMaxAlloc = CTestStats(fp64(*Iter));
									}*/


								}
								Exited.f_SetSignaled();
							}
							break;
						case NMib::NProcess::EProcessLaunchState_Launched:
							{
								pProcess = _State.f_Get<NMib::NProcess::EProcessLaunchState_Launched>();
							}
							break;
						case NMib::NProcess::EProcessLaunchState_LaunchFailed:
							{
								DMibConErrOut("Error: {}\r\n", _State.f_Get<NMib::NProcess::EProcessLaunchState_LaunchFailed>());
								Exited.f_SetSignaled();
							}
							break;
						}
					}
				;
				Params.m_fOnOutput =
					[&](NMib::NProcess::EProcessLaunchOutputType _OutputType, NMib::NStr::CStr const &_Output)
					{
						if (_OutputType == NMib::NProcess::EProcessLaunchOutputType_StdOut)
							ResultParser.f_FeedText(_Output);
						else
							DMibConErrOut("{}", _Output);
					}
				;
				{
					DMibLock(ProcessLaunchLock);
					pProcessLaunch = NMib::fg_Construct(Params, NMib::NProcess::EProcessLaunchCloseFlag_BlockOnExit);
				}

				NMib::NProcess::CProcessStatistics SampledMemoryStats;

				while (true)
				{
					try
					{
						auto ThisStat = pProcessLaunch->f_GetMemoryStatistics();
						for (auto iStat = ThisStat.m_Statistics.f_GetIterator(); iStat; ++iStat)
						{
							auto Mapped = SampledMemoryStats.m_Statistics("(Sampled Max) " + iStat.f_GetKey(), *iStat);
							if (!Mapped.f_WasCreated())
							{
								if (iStat->m_Value > (*Mapped).m_Value)
									(*Mapped).m_Value = iStat->m_Value;
							}

						}
					}
					catch (NMib::NException::CException const &)
					{
					}

					if (!Exited.f_WaitTimeout(0.01))
						break; // Exited
				}

				{
					DMibLock(ProcessLaunchLock);
					pProcessLaunch.f_Clear();
				}

				fl_AddStats(SampledMemoryStats);

				if (ResultParser.m_bFailedTest)
					DMibTest(!DMibExpr(ResultParser.m_bFailedTest))(ETestFlag_Aggregated);

				if (ResultParser.m_bValidResult)
				{
					_PerfTest.f_Add(Result);

					if (!MemoryResult.m_PerAllocationType.f_IsEmpty())
					{
						MemoryResult.m_MeasureType = Result.m_MeasureType;
						MemoryResult.m_Name = Result.m_Name;
						MemoryResult.m_nIterations = 1;
						MemoryResult.m_nRepetitions = 1;
						_MemoryTest.f_Add(MemoryResult);
					}
				}
				else
				{
					DMibTest(DMibExpr(false) && DMibExpr("Recursive test failed"))(Params.m_Parameters);
				}

				return;
			}

			struct CPerfromTest
			{
				CPerformance_Tests &m_This;
				CPerfromTest(CPerformance_Tests &_This)
					: m_This(_This)
				{
				}
				void f_Perform(CTestPerformance &_PerfTest, ETestMeasureType _MeasureType, ch8 const *_pName, umint _MaxAllocSize, umint _nThreads)
				{
					m_This.f_DoTestPerform<tf_CHeap, tf_CAllocPattern>(_PerfTest, _MeasureType, _pName, _MaxAllocSize, _nThreads);
				}
			};

			CPerfromTest Performer(*this);

			DMibTestSuite(_pName)
			{
				Performer.f_Perform(_PerfTest, _MeasureType, _pName, _MaxAllocSize, _nThreads);
				DMibTest(DMibExpr(_PerfTest));
			};
		}

		template <typename tf_CAllocPattern>
		void f_DoTestsManagers(umint _MaxAllocSize, tf_CAllocPattern const &_Pattern, umint _nThreads)
		{
			CTestPerformance PerfTest(0.5, false);
			CTestMemory MemoryTest(0.5, false);
			f_DoTest<CMalterlibMemoryDummy, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Baseline, "Baseline", _MaxAllocSize, _nThreads);
#ifdef DMemoryManagerTestEnable_Malterlib
			f_DoTest<CMalterlibMemoryMalterlib, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Normal, "Malterlib", _MaxAllocSize, _nThreads);
			f_DoTest<CMalterlibMemoryMalterlib_NoCheckout, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Normal, "Malterlib_Checkout", _MaxAllocSize, _nThreads);
#if DMibPPtrBits >= 64
			f_DoTest<CMalterlibMemoryMalterlib_LowBranch, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_LowBranch", _MaxAllocSize, _nThreads);
#endif

			//f_DoTest<CMalterlibMemoryMalterlib_Tracked, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_Tracked", _MaxAllocSize, _nThreads);
			//f_DoTest<CMalterlibMemoryMalterlib_Debug, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_Debug", _MaxAllocSize, _nThreads);
			/*
			f_DoTest<CMalterlibMemoryMalterlib_NoCommit, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_NoCommit", _MaxAllocSize, _nThreads);
			f_DoTest<CMalterlibMemoryMalterlib_NoDeferCleanup, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_DirectCleanup", _MaxAllocSize, _nThreads);
			f_DoTest<CMalterlibMemoryMalterlib_NoCleanup, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Debug, "Malterlib_NoCleanup", _MaxAllocSize, _nThreads);
			 */
#endif

#ifdef DMibConfig_MemoryManager_UseMalterlib
			ETestMeasureType ApplicationMeasureType = ETestMeasureType_Normal;
#else
			ETestMeasureType ApplicationMeasureType = ETestMeasureType_Reference;
#endif

#ifdef DMibConfig_OverrideSystemMalloc
			NMib::NStr::CStr SystemMallocOverrideSuffix = " (Overridden)";
			ETestMeasureType SystemMeasureType = ApplicationMeasureType;
#else
			NMib::NStr::CStr SystemMallocOverrideSuffix;
			ETestMeasureType SystemMeasureType = ETestMeasureType_Reference;
#endif

#ifdef DMemoryManagerTestEnable_MacOS
			f_DoTest<CMalterlibMemoryMacOS, tf_CAllocPattern>(PerfTest, MemoryTest, SystemMeasureType, "macOS" + SystemMallocOverrideSuffix, _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_StdLib
			f_DoTest<CMalterlibMemoryStdLib, tf_CAllocPattern>(PerfTest, MemoryTest, SystemMeasureType, "StdLib" + SystemMallocOverrideSuffix, _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_Application
			f_DoTest<CMalterlibMemoryApplication, tf_CAllocPattern>(PerfTest, MemoryTest, ApplicationMeasureType, "Application", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_TcMalloc
			f_DoTest<CMalterlibMemoryTcMalloc, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "TcMalloc", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_MiMalloc
			f_DoTest<CMalterlibMemoryMiMalloc, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "MiMalloc", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_WindowsLF
			f_DoTest<TCMalterlibMemoryWindowsLF<false>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "WindowsLF", _MaxAllocSize, _nThreads);
#endif
#ifdef DMemoryManagerTestEnable_WindowsDefault
			f_DoTest<TCMalterlibMemoryWindows<false, true>, tf_CAllocPattern>(PerfTest, MemoryTest, ETestMeasureType_Reference, "Windows", _MaxAllocSize, _nThreads);
#endif

			if (!(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
			{
				DMibTest(DMibExpr(PerfTest));
				if (!MemoryTest.f_IsEmpty())
					DMibTest(DMibExpr(MemoryTest));
			}
		}

		template <typename tf_CAllocPattern>
		void f_DoTests(umint _MaxAllocSize, tf_CAllocPattern const &_Pattern, umint _nThreads)
		{
			f_DoTestsManagers(_MaxAllocSize, _Pattern, _nThreads);
		}

		template <typename tf_CAllocPattern>
		void f_DoTests(ch8 const *_pPattern, tf_CAllocPattern const &_Pattern)
		{
			auto fRunTests = [&]
				{
					for
		#if 1
						(
							umint i = 1
							; i <= 1024*1024*8
							; i = i << 1
						)
		#else
						(
							umint i = 1
							//; i <= 1024*1024*8
		#if DMibPPtrBits == 64
							; i <= 12
		#else
							; i <= 4
		#endif
							; i = i < 4 ? i << 1 : i + 4
						)
		#endif
					{
						DMibTestCategory(NMib::NStr::CStr::CFormat("Max Alloc({})") << i)
						{
							f_DoPattern(_pPattern, _Pattern, i);
						};
					}
				}
			;
			if (NMib::NStr::fg_StrStartsWith(_pPattern, "Random"))
			{
				DMibTestCategory(_pPattern)
				{
					fRunTests();
				};
			}
			else
			{
				DMibTestCategory(CTestCategory(_pPattern) << CTestGroup("Manual"))
				{
					fRunTests();
				};
			}
		}

		template <typename tf_CAllocPattern>
		void f_DoPattern(ch8 const *_pPattern, tf_CAllocPattern const &_Pattern, umint _MaxAlloc)
		{
			umint nPhysicalCores = NMib::NSys::fg_Thread_GetPhysicalCores();
			umint nVirtualCores = NMib::NSys::fg_Thread_GetVirtualCores();
			umint nCores = m_nCores;
			//nCores = 1;
			umint i = 1;
			NMib::NContainer::TCSet<umint> AlreadyRan;

			auto fRunTest = [&](umint _nThreads)
				{
					if (!AlreadyRan(_nThreads).f_WasCreated())
						return;
					if (!(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
					{
						if
							(
								(_MaxAlloc >= gc_TestSize && _MaxAlloc <= gc_TestSizeEnd)
								&& (_nThreads == 1 || _nThreads == 2 || _nThreads == nPhysicalCores || _nThreads == nVirtualCores || _nThreads == nPhysicalCores * 2)
							)
						{
							DMibTestSuite(NMib::NStr::CStr::CFormat("Threads({})") << _nThreads)
							{
								f_DoTests(_MaxAlloc, _Pattern, _nThreads);
							};
						}
						else
						{
							DMibTestSuite(CTestCategory(NMib::NStr::CStr::CFormat("Threads({})") << _nThreads) << CTestGroup("Manual"))
							{
								f_DoTests(_MaxAlloc, _Pattern, _nThreads);
							};
						}
					}
					else
					{
						DMibTestCategory(NMib::NStr::CStr::CFormat("Threads({})") << _nThreads)
						{
							f_DoTests(_MaxAlloc, _Pattern, _nThreads);
						};
					}
				}
			;

			umint nEndCores = NMib::fg_Max(nCores * 2, 128u);

			for (; i <= nEndCores; i = i << 1)
				fRunTest(i);

			if (!NMib::fg_IsPowerOfTwo(nPhysicalCores) && nPhysicalCores != nCores)
				fRunTest(nPhysicalCores);

			i = nCores;
			fRunTest(i);
			i = i << 1;
#if 0
			umint nEndCores = NMib::fg_Min(nCores*1024, 64u); // Max 4096 threads as it taskes some time to start threads
			//umint nEndCores = NMib::fg_Min(nCores*1024, 4096u); // Max 4096 threads as it taskes some time to start threads
			if (NMib::NSys::fg_System_BeingDebugged() && !(fg_TestReportFlags() & ETestReportFlag_ProcessRecursive))
				nEndCores = NMib::fg_Min(nEndCores, 128u); // Running in debugger the debugger makes creating threads really slow
#endif

			for (; i <= nEndCores; i = i << 1)
			{
				fRunTest(i);
			}
		}

		void f_DoPatterns()
		{
			f_DoTests("Random", CAllocPattern_Random());
			f_DoTests("OneSize", CAllocPattern_OneSize());
			f_DoTests("OneSizeLinear", CAllocPattern_OneSizeLinear());
			f_DoTests("RandomAligned", CAllocPattern_RandomAlignment());
		}

		void f_DoTests()
		{
			DMibTestCategory(CTestCategory("Synthetic") << CTestGroup("Performance"))
			{
				f_DoPatterns();
			};
		}
	};

	DMibTestRegister(CPerformance_Tests, Malterlib::Memory::MemoryManager);

}

