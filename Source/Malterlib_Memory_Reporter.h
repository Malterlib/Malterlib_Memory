// Copyright © 2018 Nonna Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#if DMibConfig_Memory_Shims_Lightweight
#	include <Mib/Core/CoroutineHandler>
#endif

namespace NMib::NMemory
{
#if DMibConfig_Memory_Shims_Lightweight
	#define DMibMemLightweightTrack(d_Expression) d_Expression

	CReportMemoryLightweight *fg_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter);

	class CMemoryReportLightweightScope final : public CCoroutineThreadLocalHandler
	{
		CReportMemoryLightweight *m_pOldReporter;
		CReportMemoryLightweight *m_pNewReporter;
	public:
		CMemoryReportLightweightScope(CReportMemoryLightweight *_pReporter)
			: m_pNewReporter(_pReporter)
		{
			m_pOldReporter = fg_ReportMemoryLightweightTo(_pReporter);
		}
		CMemoryReportLightweightScope(CReportMemoryLightweight &_Reporter)
		{
			m_pOldReporter = fg_ReportMemoryLightweightTo(&_Reporter);
		}
		~CMemoryReportLightweightScope()
		{
			fg_ReportMemoryLightweightTo(m_pOldReporter);
		}
		void f_Suspend() override
		{
			fg_ReportMemoryLightweightTo(m_pOldReporter);
		}
		void f_Resume() override
		{
			m_pOldReporter = fg_ReportMemoryLightweightTo(m_pNewReporter);
		}
	};

	enum EMemoryReportLightweightScopeFlag
	{
		EMemoryReportLightweightScopeFlag_None = 0
		, EMemoryReportLightweightScopeFlag_InCScope = DMibBit(0) // We are inside a C allocation scope, for example malloc, or free
		, EMemoryReportLightweightScopeFlag_User0 = DMibBit(1)
	};

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeGetFlags();
	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeSetFlags(EMemoryReportLightweightScopeFlag _Flags);
	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeAddFlags(EMemoryReportLightweightScopeFlag _Flags);

	class CMemoryReportLightweightScopeFlagScope
	{
	public:
		CMemoryReportLightweightScopeFlagScope(EMemoryReportLightweightScopeFlag _AddFlags)
		{
			m_OldFlags = fg_MemoryLightweightScopeAddFlags(_AddFlags);
		}
		~CMemoryReportLightweightScopeFlagScope()
		{
			fg_MemoryLightweightScopeSetFlags(m_OldFlags);
		}

	private:
		DMibThreadLocalScopeDebugMember;
		EMemoryReportLightweightScopeFlag m_OldFlags;
	};

	class CMemoryReportLightweightScopeFlagLowLevelScope
	{
	public:
		CMemoryReportLightweightScopeFlagLowLevelScope(EMemoryReportLightweightScopeFlag _AddFlags)
		{
			m_OldFlags = fg_MemoryLightweightScopeAddFlags(_AddFlags);
		}
		~CMemoryReportLightweightScopeFlagLowLevelScope()
		{
			fg_MemoryLightweightScopeSetFlags(m_OldFlags);
		}

	private:
		EMemoryReportLightweightScopeFlag m_OldFlags;
	};

	#define DMibMemLightweightTrackDisableScope NMib::NMemory::CMemoryReportLightweightScope DisableLightweightTrack(nullptr)
	#define DMibMemLightweightTrackAddFlagsScope(d_Flags) NMib::NMemory::CMemoryReportLightweightScopeFlagScope AddLightweightTrackScopFlags(d_Flags)
	#define DMibMemLightweightTrackAddFlagsLowLevelScope(d_Flags) NMib::NMemory::CMemoryReportLightweightScopeFlagLowLevelScope AddLightweightTrackScopFlags(d_Flags)
#else
	#define DMibMemLightweightTrack(d_Expression)
	#define DMibMemLightweightTrackDisableScope
	#define DMibMemLightweightTrackAddFlagsScope(d_Flags)
	#define DMibMemLightweightTrackAddFlagsLowLevelScope(d_Flags)
#endif

#if DMibConfig_Memory_Shims_Enable

	void fg_ReportMemoryAlloc
		(
			mint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	;
	void fg_ReportMemoryResize
		(
			mint _MemoryAllocator
			, ch8 const *_pAllocatorName
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
	;
	void fg_ReportMemoryRealloc
		(
			mint _MemoryAllocator
			, ch8 const *_pAllocatorName
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
	;
	void fg_ReportMemoryFree(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, void const *_pAllocationInfo);

	void fg_ReportMemoryGetSize(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, void const *_pAllocationInfo);
	void fg_ReportMemoryProtect(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, uaint _Protect);
	void fg_ReportMemoryCommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size);
	void fg_ReportMemoryDecommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size);
	void fg_ReportMemoryAllocatorDelete(mint _MemoryAllocator, ch8 const *_pAllocatorName);
	void fg_ReportMemoryGoingToReportEnter(mint _MemoryAllocator);
	void fg_ReportMemoryGoingToReportExit(mint _MemoryAllocator);
	void fg_ReportMemoryAllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName);
	void fg_ReportMemoryReportStatistics(bool _bFullReport);

	CReportMemory *fg_ReportMemoryTo(CReportMemory *_pMemoryReporter);
	void fg_ReportMemoryGloballyTo(CGlobalReportMemory *_pMemoryReporter);

	class CMemoryReportScope final : public CCoroutineThreadLocalHandler
	{
		CReportMemory *m_pOldReporter;
		CReportMemory *m_pNewReporter;
	public:
		CMemoryReportScope(CReportMemory &_Reporter)
			: m_pNewReporter(&_Reporter)
		{
			m_pOldReporter = fg_ReportMemoryTo(&_Reporter);
		}
		~CMemoryReportScope()
		{
			fg_ReportMemoryTo(m_pOldReporter);
		}
		void f_Suspend() override
		{
			fg_ReportMemoryTo(m_pOldReporter);
		}
		void f_Resume() override
		{
			m_pOldReporter = fg_ReportMemoryTo(m_pNewReporter);
		}
	};

	class CDisableMemoryReporterScope final : public CCoroutineThreadLocalHandler
	{
		CReportMemory *m_pOldReporter;
	public:
		CDisableMemoryReporterScope()
		{
			m_pOldReporter = fg_ReportMemoryTo(nullptr);
		}
		~CDisableMemoryReporterScope()
		{
			fg_ReportMemoryTo(m_pOldReporter);
		}
		void f_Suspend() override
		{
			fg_ReportMemoryTo(m_pOldReporter);
		}
		void f_Resume() override
		{
			m_pOldReporter = fg_ReportMemoryTo(nullptr);
		}
	};

	class CMemoryReportGoingToReportScope
	{
		mint m_Allocator;
		bool m_bReport;
	public:
		CMemoryReportGoingToReportScope(mint _Allocator, bool _bReport)
			: m_Allocator(_Allocator)
			, m_bReport(_bReport)
		{
			if (_bReport)
				fg_ReportMemoryGoingToReportEnter(_Allocator);
		}
		~CMemoryReportGoingToReportScope()
		{
			if (m_bReport)
				fg_ReportMemoryGoingToReportExit(m_Allocator);
		}
	};

#	define DMibMemoryGoingToReportScope(d_MemoryAllocator, d_bReport) CMemoryReportGoingToReportScope MalterlibMemoryReportScope((mint)d_MemoryAllocator, d_bReport);
#	define DMibMemoryReportSaveVar(d_Name, d_Value) auto d_Name = d_Value
#	define DMibMemoryReportExpression(d_Expression) d_Expression
#	define DMibMemoryReportAlloc(d_MemoryAllocator, d_AllocatorName, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
		fg_ReportMemoryAlloc((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportResize(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
		fg_ReportMemoryResize((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_OldAddress, d_OldSize, d_pOldAllocationInfo, (mint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportRealloc(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
		fg_ReportMemoryRealloc((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_OldAddress, d_OldSize, d_pOldAllocationInfo, (mint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportFree(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo) \
		fg_ReportMemoryFree((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportGetSize(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo) \
		fg_ReportMemoryGetSize((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportCommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size) \
		fg_ReportMemoryCommit((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size)
#	define DMibMemoryReportProtect(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_Protect) \
		fg_ReportMemoryProtect((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size, d_Protect)
#	define DMibMemoryReportDecommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size) \
		fg_ReportMemoryDecommit((mint)d_MemoryAllocator, d_AllocatorName, (mint)d_Address, d_Size)
#	define DMibMemoryReportAllocatorDelete(d_MemoryAllocator, d_AllocatorName) \
		fg_ReportMemoryAllocatorDelete((mint)d_MemoryAllocator, d_AllocatorName)
#	define DMibMemoryReportAllocatorName(d_MemoryAllocator, d_AllocatorName) \
		fg_ReportMemoryAllocatorName((mint)d_MemoryAllocator, d_AllocatorName)

#else
#	define DMibMemoryGoingToReportScope(d_MemoryAllocator, d_bReport)
#	define DMibMemoryReportSaveVar(d_Name, d_Value)
#	define DMibMemoryReportExpression(d_Expression)
#	define DMibMemoryReportAlloc(d_MemoryAllocator, d_AllocatorName, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportResize(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportRealloc(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportFree(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportGetSize(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportProtect(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_Protect)
#	define DMibMemoryReportCommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size)
#	define DMibMemoryReportDecommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size)
#	define DMibMemoryReportAllocatorDelete(d_MemoryAllocator, d_AllocatorName)
#	define DMibMemoryReportAllocatorName(d_MemoryAllocator, d_AllocatorName)
#endif
}
