// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#if DMibConfig_Memory_Shims_Lightweight
#	include <Mib/Core/CoroutineHandler>
#endif

namespace NMib::NMemory
{
#if DMibConfig_Memory_Shims_Lightweight
#	define DMibMemLightweightTrack(d_Expression) d_Expression

	CReportMemoryLightweight *fg_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter);

	struct CMemoryReportLightweightScope final : public CCoroutineThreadLocalHandler
	{
		CMemoryReportLightweightScope(CReportMemoryLightweight *_pReporter);
		CMemoryReportLightweightScope(CReportMemoryLightweight &_Reporter);
		~CMemoryReportLightweightScope();

		void f_Suspend() noexcept override;
		void f_ResumeNoExcept() noexcept override;

	private:
		CReportMemoryLightweight *mp_pOldReporter;
		CReportMemoryLightweight *mp_pNewReporter;
	};

	enum EMemoryReportLightweightScopeFlag
	{
		EMemoryReportLightweightScopeFlag_None       = 0
		, EMemoryReportLightweightScopeFlag_InCScope = DMibBit(0) // We are inside a C allocation scope, for example malloc, or free
		, EMemoryReportLightweightScopeFlag_User0    = DMibBit(1)
	};

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeGetFlags();
	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeSetFlags(EMemoryReportLightweightScopeFlag _Flags);
	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeAddFlags(EMemoryReportLightweightScopeFlag _Flags);

	struct CMemoryReportLightweightScopeFlagScope
	{
		CMemoryReportLightweightScopeFlagScope(EMemoryReportLightweightScopeFlag _AddFlags);
		~CMemoryReportLightweightScopeFlagScope();

	private:
		DMibThreadLocalScopeDebugMember;
		EMemoryReportLightweightScopeFlag m_OldFlags;
	};

	struct CMemoryReportLightweightScopeFlagLowLevelScope
	{
		CMemoryReportLightweightScopeFlagLowLevelScope(EMemoryReportLightweightScopeFlag _AddFlags);
		~CMemoryReportLightweightScopeFlagLowLevelScope();

	private:
		EMemoryReportLightweightScopeFlag m_OldFlags;
	};

#	define DMibMemLightweightTrackDisableScope NMib::NMemory::CMemoryReportLightweightScope DisableLightweightTrack(nullptr)
#	define DMibMemLightweightTrackAddFlagsScope(d_Flags) NMib::NMemory::CMemoryReportLightweightScopeFlagScope AddLightweightTrackScopFlags(d_Flags)
#	define DMibMemLightweightTrackAddFlagsLowLevelScope(d_Flags) NMib::NMemory::CMemoryReportLightweightScopeFlagLowLevelScope AddLightweightTrackScopFlags(d_Flags)
#else
#	define DMibMemLightweightTrack(d_Expression)
#	define DMibMemLightweightTrackDisableScope
#	define DMibMemLightweightTrackAddFlagsScope(d_Flags)
#	define DMibMemLightweightTrackAddFlagsLowLevelScope(d_Flags)
#endif

#if DMibConfig_Memory_Shims_Enable

	void fg_ReportMemoryAlloc
		(
			umint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, umint _Address
			, umint _RequestedAlignment
			, umint _RequestedSize
			, umint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	;
	void fg_ReportMemoryResize
		(
			umint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, umint _OldAddress
			, umint _OldSize
			, void const *_pOldAllocationInfo
			, umint _Address
			, umint _RequestedAlignment
			, umint _RequestedSize
			, umint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	;
	void fg_ReportMemoryRealloc
		(
			umint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, umint _OldAddress
			, umint _OldSize
			, void const *_pOldAllocationInfo
			, umint _Address
			, umint _RequestedAlignment
			, umint _RequestedSize
			, umint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	;
	void fg_ReportMemoryFree(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _Address, umint _Size, void const *_pAllocationInfo);

	void fg_ReportMemoryGetSize(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _Address, umint _Size, void const *_pAllocationInfo);
	void fg_ReportMemoryProtect(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _Address, umint _Size, uaint _Protect);
	void fg_ReportMemoryCommit(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _Address, umint _Size);
	void fg_ReportMemoryDecommit(umint _MemoryAllocator, ch8 const *_pAllocatorName, umint _Address, umint _Size);
	void fg_ReportMemoryAllocatorDelete(umint _MemoryAllocator, ch8 const *_pAllocatorName);
	void fg_ReportMemoryGoingToReportEnter(umint _MemoryAllocator);
	void fg_ReportMemoryGoingToReportExit(umint _MemoryAllocator);
	void fg_ReportMemoryAllocatorName(umint _MemoryAllocator, ch8 const *_pAllocatorName);
	void fg_ReportMemoryReportStatistics(bool _bFullReport);

	CReportMemory *fg_ReportMemoryTo(CReportMemory *_pMemoryReporter);
	void fg_ReportMemoryGloballyTo(CGlobalReportMemory *_pMemoryReporter);

	struct CMemoryReportScope final : public CCoroutineThreadLocalHandler
	{
		CMemoryReportScope(CReportMemory &_Reporter);
		~CMemoryReportScope();

		void f_Suspend() noexcept override;
		void f_ResumeNoExcept() noexcept override;

	private:
		CReportMemory *mp_pOldReporter;
		CReportMemory *mp_pNewReporter;
	};

	struct CDisableMemoryReporterScope final : public CCoroutineThreadLocalHandler
	{
		CDisableMemoryReporterScope();
		~CDisableMemoryReporterScope();

		void f_Suspend() noexcept override;
		void f_ResumeNoExcept() noexcept override;

	private:
		CReportMemory *mp_pOldReporter;
	};

	struct CMemoryReportGoingToReportScope
	{
		CMemoryReportGoingToReportScope(umint _Allocator, bool _bReport);
		~CMemoryReportGoingToReportScope();

	private:
		umint mp_Allocator;
		bool mp_bReport;
	};

#	define DMibMemoryGoingToReportScope(d_MemoryAllocator, d_bReport) CMemoryReportGoingToReportScope MalterlibMemoryReportScope((umint)d_MemoryAllocator, d_bReport);
#	define DMibMemoryReportSaveVar(d_Name, d_Value) auto d_Name = d_Value
#	define DMibMemoryReportExpression(d_Expression) d_Expression
#	define DMibMemoryReportAlloc(d_MemoryAllocator, d_AllocatorName, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
		fg_ReportMemoryAlloc((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportResize(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, \
		d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
		fg_ReportMemoryResize((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_OldAddress, d_OldSize, d_pOldAllocationInfo, (umint)d_Address, \
			d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportRealloc(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, \
		d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo) \
		fg_ReportMemoryRealloc((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_OldAddress, d_OldSize, d_pOldAllocationInfo, (umint)d_Address, \
			d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportFree(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo) \
		fg_ReportMemoryFree((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportGetSize(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo) \
		fg_ReportMemoryGetSize((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportCommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size) \
		fg_ReportMemoryCommit((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_Address, d_Size)
#	define DMibMemoryReportProtect(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_Protect) \
		fg_ReportMemoryProtect((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_Address, d_Size, d_Protect)
#	define DMibMemoryReportDecommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size) \
		fg_ReportMemoryDecommit((umint)d_MemoryAllocator, d_AllocatorName, (umint)d_Address, d_Size)
#	define DMibMemoryReportAllocatorDelete(d_MemoryAllocator, d_AllocatorName) \
		fg_ReportMemoryAllocatorDelete((umint)d_MemoryAllocator, d_AllocatorName)
#	define DMibMemoryReportAllocatorName(d_MemoryAllocator, d_AllocatorName) \
		fg_ReportMemoryAllocatorName((umint)d_MemoryAllocator, d_AllocatorName)

#else
#	define DMibMemoryGoingToReportScope(d_MemoryAllocator, d_bReport)
#	define DMibMemoryReportSaveVar(d_Name, d_Value)
#	define DMibMemoryReportExpression(d_Expression)
#	define DMibMemoryReportAlloc(d_MemoryAllocator, d_AllocatorName, d_Address, d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportResize(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, \
		d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportRealloc(d_MemoryAllocator, d_AllocatorName, d_OldAddress, d_OldSize, d_pOldAllocationInfo, d_Address, \
		d_RequestedAlignment, d_RequestedSize, d_ReturnedSize, d_nBytesOverhead, d_pAllocationInfo)
#	define DMibMemoryReportFree(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportGetSize(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_pAllocationInfo)
#	define DMibMemoryReportProtect(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size, d_Protect)
#	define DMibMemoryReportCommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size)
#	define DMibMemoryReportDecommit(d_MemoryAllocator, d_AllocatorName, d_Address, d_Size)
#	define DMibMemoryReportAllocatorDelete(d_MemoryAllocator, d_AllocatorName)
#	define DMibMemoryReportAllocatorName(d_MemoryAllocator, d_AllocatorName)
#endif
}

#include "Malterlib_Memory_Reporter.hpp"
