
#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_Reporter_Categories.h"
#include "Malterlib_Memory_Reporter_CategoriesInterface.h"
#ifdef DPlatformFamily_Windows
#include <Windows.h>
#include <Perflib.h>
#include <Mib/Cryptography/UUID>
#include <Mib/Cryptography/Hashes/MD5>
#include <Mib/Process/ProcessLaunch>
#include <Mib/Core/PlatformSpecific/WindowsError>

#endif

namespace NMib::NMemory
{
	CMemoryCategory g_DefaultMemoryCategory = {"Default category"};

	struct CDynamicMemoryCategory : public CMemoryCategory
	{
#ifdef DCompiler_clang
		constexpr CDynamicMemoryCategory()
			: CMemoryCategory(nullptr)
			, m_Name{0}
		{
		}
#endif
		ch8 m_Name[64];
	};

	constinit NThread::CMutualAggregate g_AllCategoriesLock = {DAggregateInit};
	constinit bool g_bAllCategoriesInit = true;
	constinit NIntrusive::TCAVLTreeAggregate<&CMemoryCategory::m_Link, CMemoryCategory::CCompare> g_AllCategories = {DAggregateInit};
	constinit NStorage::TCAggregateSimple<TCPoolGrowing<CDynamicMemoryCategory, 128, NMib::NThread::CNoLock, CAllocator_VirtualNoTracking>> g_CategoriesPool = {DAggregateInit};

	constinit NAtomic::TCAtomicAggregate<int64> g_CyclesTimer = {DAggregateInit};
	int64 g_CyclesTimerFrequency = 0;

#ifdef DPlatformFamily_Windows
	struct CPerformanceCounters
	{

		GUID m_ProdiverGUID;
		GUID m_CounterSetGUID;

		HANDLE m_pMalterlibPerfProvider = nullptr;

		bool m_bValid = false;

		struct
		{
			PERF_COUNTERSET_INFO m_CounterSet;
			PERF_COUNTER_INFO m_Counter0;
			PERF_COUNTER_INFO m_Counter1;
		} m_MemoryUsedInfo;

		NContainer::TCMap<NStr::CStrNonTracked, PPERF_COUNTERSET_INSTANCE, NMib::CSort_Default, NMib::NMemory::CAllocator_NonTrackedHeap> m_PerfCounters;

		~CPerformanceCounters()
		{
			if (m_pMalterlibPerfProvider != NULL)
			{
				PerfStopProvider(m_pMalterlibPerfProvider);
				m_pMalterlibPerfProvider = NULL;
			}
		}

		CPerformanceCounters()
		{
			NStr::CStrNonTracked Executable = NFile::CFile::fs_GetProgramPath();
			NCryptography::CUniversallyUniqueIdentifier HashRootGUID("354e81d6-242b-4b1e-badd-37ac3aceadbb", NCryptography::EUniversallyUniqueIdentifierFormat_Bare);

			NCryptography::CUniversallyUniqueIdentifier ProdiverGUID(NCryptography::EUniversallyUniqueIdentifierGenerate_StringHash, HashRootGUID, Executable + "/Provider");
			NCryptography::CUniversallyUniqueIdentifier CounterSetGUID(NCryptography::EUniversallyUniqueIdentifierGenerate_StringHash, HashRootGUID, Executable + "/CounterSet");

			m_ProdiverGUID = reinterpret_cast<GUID &>(ProdiverGUID);
			m_CounterSetGUID = reinterpret_cast<GUID &>(CounterSetGUID);

			NStr::CStrNonTracked ExecutableHash = NStr::fg_Format<NStr::CStrNonTracked>("{nfh,sj8,sf0}", NCryptography::CHash_MD5::fs_DigestFromData(Executable.f_GetStr(), Executable.f_GetLen()).f_FoldToInt<uint32>());

			NStr::CStrNonTracked Manifest = R"***(
<?xml version='1.0' encoding='utf-8' standalone='yes'?>
<instrumentationManifest
    xmlns="http://schemas.microsoft.com/win/2004/08/events"
    xmlns:trace="http://schemas.microsoft.com/win/2004/08/events/trace"
    xmlns:win="http://manifests.microsoft.com/win/2004/08/windows/events"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:schemaLocation="http://schemas.microsoft.com/win/2004/08/events eventman.xsd"
    >
  <instrumentation>
    <counters
        xmlns="http://schemas.microsoft.com/win/2005/12/counters"
        xmlns:auto-ns1="http://schemas.microsoft.com/win/2004/08/events"
        schemaVersion="1.1"
        >
      <provider applicationIdentity = "{Executable}"
                providerName        = "{ProviderName}"
                providerType        = "userMode"
                symbol              = "{ExecutableName}"
                providerGuid        = "{ProviderGUID}">
        <counterSet    guid        = "{CounterSetGUID}"
                       uri         = "com.malterlib.Counters.memory.categories.{ExecutableHash}"
                       name        = "Malterlib Memory categories ({ExecutableName})"
                       description = "This counter provides memory statistics by category"
                       symbol      = "MemoryBytes"
                       instances   = "multiple">
          <counter id           = "1"
                   uri          = "com.malterlib.Counters.memory.categories.{ExecutableHash}.bytes"
                   name         = "Bytes allocated"
                   description  = "Number of bytes allocated for category"
                   type         = "perf_counter_large_rawcount"
                   defaultScale = "-8"
                   detailLevel  = "standard">
          </counter>
          <counter id           = "2"
                   uri          = "com.malterlib.Counters.memory.categories.{ExecutableHash}.allocations"
                   name         = "Number of allocations"
                   description  = "Number of allocations in category"
                   type         = "perf_counter_large_rawcount"
                   defaultScale = "-5"
                   detailLevel  = "standard">
          </counter>
        </counterSet>
      </provider>
    </counters>
  </instrumentation>
</instrumentationManifest>
)***";

			Manifest = Manifest.f_Replace("{Executable}", NFile::CFile::fs_GetFile(Executable));
			Manifest = Manifest.f_Replace("{ProviderName}", "Malterlib memory (" + NFile::CFile::fs_GetFileNoExt(Executable) + ")");
			Manifest = Manifest.f_Replace("{ExecutableName}", NFile::CFile::fs_GetFileNoExt(Executable));
			Manifest = Manifest.f_Replace("{ProviderGUID}", ProdiverGUID.f_GetAsString(NCryptography::EUniversallyUniqueIdentifierFormat_Registry));
			Manifest = Manifest.f_Replace("{CounterSetGUID}", CounterSetGUID.f_GetAsString(NCryptography::EUniversallyUniqueIdentifierFormat_Registry));
			Manifest = Manifest.f_Replace("{ExecutableHash}", ExecutableHash);

			NStr::CStrNonTracked ManifestFileName = NFile::CFile::fs_GetPath(Executable) + "/" + NFile::CFile::fs_GetFileNoExt(Executable) + "_MalterlibMemoryCategoryCounters.man";
			NFile::CFile::fs_WriteStringToFile(ManifestFileName, Manifest);

			{
				NProcess::CProcessLaunchParams Params;
				Params.m_bAllowExecutableLocate = true;
				NStr::CStr StdOut;
				NStr::CStr StdErr;
				uint32 ExitCode;
				if (!NProcess::CProcessLaunch::fs_LaunchBlock("unlodctr.exe", NContainer::fg_CreateVector<NStr::CStr>("/M:" + ManifestFileName), StdOut, StdErr, ExitCode, Params))
				{
					DMibTraceSafe("unlodctr.exe failed with: {}\n", StdErr);
				}
				else if (ExitCode != 0)
				{
					DMibTraceSafe("unlodctr.exe failed with: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(ExitCode));
				}
			}

				DMibTraceSafe("0xc0000bb8: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(0xc0000bb8));

			{
				NProcess::CProcessLaunchParams Params;
				Params.m_bAllowExecutableLocate = true;
				NStr::CStr StdOut;
				NStr::CStr StdErr;
				uint32 ExitCode;
				if (!NProcess::CProcessLaunch::fs_LaunchBlock("lodctr.exe", NContainer::fg_CreateVector<NStr::CStr>("/M:" + ManifestFileName), StdOut, StdErr, ExitCode, Params))
				{
					DMibTraceSafe("lodctr.exe failed with: {}\n", StdErr);
					return;
				}
				if (ExitCode != 0)
				{
					DMibTraceSafe("lodctr.exe failed with: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(ExitCode));
					return;
				}
			}

			m_MemoryUsedInfo =
				{
					{ m_CounterSetGUID, m_ProdiverGUID, 2, PERF_COUNTERSET_MULTI_INSTANCES },
					{ 1, PERF_COUNTER_LARGE_RAWCOUNT, 0, sizeof(ULONGLONG), PERF_DETAIL_NOVICE, -8, 0 },
					{ 2, PERF_COUNTER_LARGE_RAWCOUNT, 0, sizeof(ULONGLONG), PERF_DETAIL_NOVICE, -5, 0 },
				}
			;

			ULONG Status;
			PERF_PROVIDER_CONTEXT ProviderContext;

			ZeroMemory(&ProviderContext, sizeof(PERF_PROVIDER_CONTEXT));
			ProviderContext.ContextSize = sizeof(PERF_PROVIDER_CONTEXT);

			Status = PerfStartProviderEx(&m_ProdiverGUID,
											&ProviderContext,
											&m_pMalterlibPerfProvider);
			if (Status != ERROR_SUCCESS)
			{
				DMibTraceSafe("PerfStartProviderEx failed: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(Status));
				m_pMalterlibPerfProvider = nullptr;
				return;
			}

			Status = PerfSetCounterSetInfo(m_pMalterlibPerfProvider, &m_MemoryUsedInfo.m_CounterSet, sizeof(m_MemoryUsedInfo));
			if (Status != ERROR_SUCCESS)
			{
				DMibTraceSafe("PerfSetCounterSetInfo failed: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(Status));
				return;
			}


			m_bValid = true;
		}


		void f_SetCounter(NStr::CStrNonTracked const &_Name, int64 _Bytes, int64 _Allocations)
		{
			if (!m_bValid)
				return;
			auto pCounter = m_PerfCounters.f_FindEqual(_Name);
			if (!pCounter)
			{
				// Create the instances for multiple instance counter set.
				NStr::CWStrNonTracked Name = _Name;
				auto pNewCounter = PerfCreateInstance(m_pMalterlibPerfProvider, &m_CounterSetGUID, Name.f_GetStr(), 0);
				if (!pNewCounter)
				{
					DMibTraceSafe("PerfCreateInstance failed: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr());
					return;
				}
				m_PerfCounters[_Name] = pNewCounter;
				pCounter = m_PerfCounters.f_FindEqual(_Name);
			}

			ULONG Status = PerfSetULongLongCounterValue(m_pMalterlibPerfProvider, *pCounter, 1, _Bytes);
			if (Status != ERROR_SUCCESS)
			{
				DMibTraceSafe("PerfSetULongLongCounterValue failed: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(Status));
				return;
			}
			Status = PerfSetULongLongCounterValue(m_pMalterlibPerfProvider, *pCounter, 2, _Allocations);
			if (Status != ERROR_SUCCESS)
			{
				DMibTraceSafe("PerfSetULongLongCounterValue failed: {}\n", NMib::NPlatform::fg_Win32_GetLastErrorStr(Status));
				return;
			}
		}

	};

	constinit NThread::CMutualAggregate g_PerfcountersLock = {DAggregateInit};
	constinit bool g_PerfcountersInit = false;
	constinit NStorage::TCAggregateSimple<CPerformanceCounters> g_Perfcounters = {DAggregateInit};
#endif

	void fg_InitCategories()
	{
		g_bAllCategoriesInit = false;
		g_AllCategoriesLock.f_Construct();
		g_AllCategories.f_Construct();
		g_CategoriesPool.f_Construct();
	}

	CCategoriesMemoryReporter::CThreadLocal::CThreadLocal()
		: m_pCurrentCategory(&g_DefaultMemoryCategory)
	{
	}

	CMemoryCategory *fg_Mem_SetCategory(CMemoryCategory *_pCategory)
	{
		if (!CCategoriesMemoryReporter::ms_pThis)
			return nullptr;

		auto &ThreadLocal = *CCategoriesMemoryReporter::ms_pThis->m_ThreadLocal;

		auto pRet = ThreadLocal.m_pCurrentCategory;
		ThreadLocal.m_pCurrentCategory = _pCategory;

		return pRet;
	}

	CMemoryCategory *fg_Mem_DefineDynamicCategory(ch8 const *_pName)
	{
		if (g_bAllCategoriesInit)
			fg_InitCategories();
		DMibLock(g_AllCategoriesLock);

		auto pCategory = g_AllCategories.f_FindEqual(_pName);
		if (pCategory)
			return pCategory;

		auto &Category = *g_CategoriesPool->f_New();
		Category.m_Link.f_Construct();

		NStr::fg_StrCopy(Category.m_Name, _pName, 64);
		Category.m_pName = Category.m_Name;

		g_AllCategories.f_Insert(Category);

		Category.m_AddedToList.f_Exchange(true);

		return &Category;
	}

	CCategoriesMemoryReporter *CCategoriesMemoryReporter::ms_pThis = nullptr;

	CCategoriesMemoryReporter::CCategoriesMemoryReporter()
	{
		if (g_bAllCategoriesInit)
			fg_InitCategories();

		fg_ReportMemoryGloballyTo(this);

		if (ms_pThis != nullptr)
			DMibPDebugBreak;
		ms_pThis = this;
	}

	void CCategoriesMemoryReporter::f_Report(bool _bFullReport)
	{
		struct CStatsEntry
		{
			NStr::CStrNonTracked m_Name;
			uint64 m_nBytes;
			uint64 m_nAllocations;
			bool operator < (CStatsEntry const &_Right) const
			{
				if (_Right.m_nBytes < m_nBytes)
					return true;
				else if (_Right.m_nBytes > m_nBytes)
					return false;
				return m_Name < _Right.m_Name;
			}
		};

		CStatsEntry Total;
		Total.m_Name = "_Total";
		Total.m_nBytes = 0;
		Total.m_nAllocations = 0;
		NContainer::TCVector<CStatsEntry, CAllocator_NonTrackedHeap> StatEntries;

		{
			DMibLock(g_AllCategoriesLock);

			for (auto iCategory = g_AllCategories.f_GetIterator(); iCategory; ++iCategory)
			{
				auto &Entry = StatEntries.f_Insert();
				Entry.m_Name = iCategory->m_pName;
				Entry.m_nBytes = iCategory->m_nBytes.f_Load();
				Entry.m_nAllocations = iCategory->m_nAllocations.f_Load();
				Total.m_nBytes += Entry.m_nBytes;
				Total.m_nAllocations += Entry.m_nAllocations;
			}
		}

		StatEntries.f_Insert(Total);
		StatEntries.f_Sort();
		NTime::CTime Time = NTime::CTime::fs_NowLocal();

		NTime::CTimeConvert::CDateTime DateTime;
		NTime::CTimeConvert(Time).f_ExtractDateTime(DateTime);

		NStr::CStrNonTracked ToLog;
		{
#ifdef DPlatformFamily_Windows
			DMibLock(g_PerfcountersLock);
			if (!g_PerfcountersInit)
			{
				g_PerfcountersInit = true;
				g_Perfcounters.f_Construct();
			}
#endif

			for (auto &StatEntry : StatEntries)
			{
				if (_bFullReport)
				{
					ToLog += NStr::CStrNonTracked::CFormat("{}-{sj2,sf0}-{sj2,sf0}\t{sj2,sf0}:{sj2,sf0}:{sj2,sf0}\t{}\t{}\t{}\t{}{\n}")
						<< DateTime.m_Year
						<< DateTime.m_Month
						<< DateTime.m_DayOfMonth
						<< DateTime.m_Hour
						<< DateTime.m_Minute
						<< DateTime.m_Second
						<< StatEntry.m_Name
						<< StatEntry.m_nBytes
						<< StatEntry.m_nAllocations
						<< (StatEntry.m_nAllocations ? StatEntry.m_nBytes / StatEntry.m_nAllocations : 0)
					;
				}
#ifdef DPlatformFamily_Windows
				g_Perfcounters->f_SetCounter(StatEntry.m_Name, StatEntry.m_nBytes, StatEntry.m_nAllocations);
#endif
			}
		}

		if (_bFullReport)
		{
			try
			{
				NStr::CStrNonTracked OutputDirectory = NFile::CFile::fs_GetProgramDirectoryNonTracked() + "/CrashDumps";
				NFile::CFile::fs_CreateDirectory(OutputDirectory);

				NStr::CStrNonTracked OutputFile = OutputDirectory + "/MemoryCategoryLog.tsv";

				NFile::CFile File;
				File.f_Open(OutputFile, NFile::EFileOpen_Write | NFile::EFileOpen_DontTruncate);
				File.f_SetPositionFromEnd(0);
				File.f_Write(ToLog.f_GetStr(), ToLog.f_GetLen());
			}
			catch (NFile::CExceptionFile const &)
			{
			}
		}
	}


	CCategoriesMemoryReporter::~CCategoriesMemoryReporter()
	{
		fg_ReportMemoryGloballyTo(nullptr);

		ms_pThis = nullptr;
	}

	void CCategoriesMemoryReporter::f_AllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName)
	{
	}

	void CCategoriesMemoryReporter::f_AllocatorDelete(mint _MemoryAllocator)
	{
	}

	void CCategoriesMemoryReporter::f_ScopeEnter(mint _MemoryAllocator)
	{
	}

	void CCategoriesMemoryReporter::f_ScopeExit(mint _MemoryAllocator)
	{
	}

	void CCategoriesMemoryReporter::fp_ReportAlloc(mint _Size, void *_pAllocationInfo)
	{
		if (!_pAllocationInfo)
		{
			// Not supported
			return;
		}

		CTrackedAllocationInfo &AllocationInfo = *((CTrackedAllocationInfo *)_pAllocationInfo);

		auto &ThreadLocal = *CCategoriesMemoryReporter::ms_pThis->m_ThreadLocal;

		auto pCurrentCategory = ThreadLocal.m_pCurrentCategory;
		if (!pCurrentCategory)
		{
			AllocationInfo.m_pCategory = nullptr;
			return;
		}
		AllocationInfo.m_pCategory = pCurrentCategory;

		if (!pCurrentCategory->m_AddedToList.f_Load(NAtomic::EMemoryOrder_Relaxed))
		{
			if (!pCurrentCategory->m_AddedToList.f_Exchange(true))
			{
				pCurrentCategory->m_Link.f_Construct();
				if (g_bAllCategoriesInit)
					fg_InitCategories();
				DMibLock(g_AllCategoriesLock);

				g_AllCategories.f_Insert(pCurrentCategory);
			}
		}

		pCurrentCategory->m_nBytes += _Size;
		++pCurrentCategory->m_nAllocations;
#if 0
		if (g_bCanStartThreads.f_Load(NAtomic::EMemoryOrder_Relaxed) && NTime::CSystem_Time::fs_TimeInitDone())
		{
			int64 Cycles = NTime::NPlatform::fg_Timer_CyclesFast();
			int64 CurrentValue = g_CyclesTimer.f_Load();

			if (Cycles > CurrentValue)
			{
				if (!g_CyclesTimerFrequency)
					g_CyclesTimerFrequency = fg_GetSys()->f_CyclesFrequency();

				int64 NewValue = Cycles + g_CyclesTimerFrequency;
				if (g_CyclesTimer.f_CompareExchangeStrong(CurrentValue, NewValue))
				{
					f_Report(true);
				}
			}
		}
#endif
	}

	void CCategoriesMemoryReporter::fp_ReportFree(mint _Size, void const *_pAllocationInfo)
	{
		if (!_pAllocationInfo)
		{
			// Not supported
			return;
		}

		CTrackedAllocationInfo const &AllocationInfo = *((CTrackedAllocationInfo const *)_pAllocationInfo);

		if (!AllocationInfo.m_pCategory)
			return;

		AllocationInfo.m_pCategory->m_nBytes -= _Size;
		--AllocationInfo.m_pCategory->m_nAllocations;
	}

	void CCategoriesMemoryReporter::f_Alloc
		(
			mint _MemoryAllocator
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
		fp_ReportAlloc(_ReturnedSize, _pAllocationInfo);
	}

	void CCategoriesMemoryReporter::f_Resize
		(
			mint _MemoryAllocator
			, mint _OldAddress
			, mint _OldSize
			, void const *_pOldAllocationInfo
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
		fp_ReportFree(_OldSize, _pOldAllocationInfo);
		fp_ReportAlloc(_ReturnedSize, _pAllocationInfo);
	}

	void CCategoriesMemoryReporter::f_Realloc
		(
			mint _MemoryAllocator
			, mint _OldAddress
			, mint _OldSize
			, void const *_pOldAllocationInfo
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
		fp_ReportFree(_OldSize, _pOldAllocationInfo);
		fp_ReportAlloc(_ReturnedSize, _pAllocationInfo);
	}

	void CCategoriesMemoryReporter::f_Free(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo)
	{
		fp_ReportFree(_Size, _pAllocationInfo);
	}

	void CCategoriesMemoryReporter::f_GetSize(mint _MemoryAllocator, mint _Address, mint _Size, void const *_pAllocationInfo)
	{
	}

	void CCategoriesMemoryReporter::f_Protect(mint _MemoryAllocator, mint _Address, mint _Size, uaint _Protect)
	{
	}

	void CCategoriesMemoryReporter::f_Commit(mint _MemoryAllocator, mint _Address, mint _Size)
	{
	}

	void CCategoriesMemoryReporter::f_Decommit(mint _MemoryAllocator, mint _Address, mint _Size)
	{
	}
}
