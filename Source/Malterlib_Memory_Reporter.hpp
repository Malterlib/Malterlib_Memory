// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
#if DMibConfig_Memory_Shims_Lightweight
	inline CMemoryReportLightweightScope::CMemoryReportLightweightScope(CReportMemoryLightweight *_pReporter)
		: mp_pNewReporter(_pReporter)
	{
		mp_pOldReporter = fg_ReportMemoryLightweightTo(_pReporter);
	}

	inline CMemoryReportLightweightScope::CMemoryReportLightweightScope(CReportMemoryLightweight &_Reporter)
	{
		mp_pOldReporter = fg_ReportMemoryLightweightTo(&_Reporter);
	}

	inline CMemoryReportLightweightScope::~CMemoryReportLightweightScope()
	{
		fg_ReportMemoryLightweightTo(mp_pOldReporter);
	}

	inline void CMemoryReportLightweightScope::f_Suspend() noexcept
	{
		fg_ReportMemoryLightweightTo(mp_pOldReporter);
	}

	inline void CMemoryReportLightweightScope::f_ResumeNoExcept() noexcept
	{
		mp_pOldReporter = fg_ReportMemoryLightweightTo(mp_pNewReporter);
	}

	inline CMemoryReportLightweightScopeFlagScope::CMemoryReportLightweightScopeFlagScope(EMemoryReportLightweightScopeFlag _AddFlags)
	{
		m_OldFlags = fg_MemoryLightweightScopeAddFlags(_AddFlags);
	}

	inline CMemoryReportLightweightScopeFlagScope::~CMemoryReportLightweightScopeFlagScope()
	{
		fg_MemoryLightweightScopeSetFlags(m_OldFlags);
	}

	inline CMemoryReportLightweightScopeFlagLowLevelScope::CMemoryReportLightweightScopeFlagLowLevelScope(EMemoryReportLightweightScopeFlag _AddFlags)
	{
		m_OldFlags = fg_MemoryLightweightScopeAddFlags(_AddFlags);
	}

	inline CMemoryReportLightweightScopeFlagLowLevelScope::~CMemoryReportLightweightScopeFlagLowLevelScope()
	{
		fg_MemoryLightweightScopeSetFlags(m_OldFlags);
	}
#endif

#if DMibConfig_Memory_Shims_Enable
	inline CMemoryReportScope::CMemoryReportScope(CReportMemory &_Reporter)
		: mp_pNewReporter(&_Reporter)
	{
		mp_pOldReporter = fg_ReportMemoryTo(&_Reporter);
	}

	inline CMemoryReportScope::~CMemoryReportScope()
	{
		fg_ReportMemoryTo(mp_pOldReporter);
	}

	inline void CMemoryReportScope::f_Suspend() noexcept
	{
		fg_ReportMemoryTo(mp_pOldReporter);
	}

	inline void CMemoryReportScope::f_ResumeNoExcept() noexcept
	{
		mp_pOldReporter = fg_ReportMemoryTo(mp_pNewReporter);
	}

	inline CDisableMemoryReporterScope::CDisableMemoryReporterScope()
	{
		mp_pOldReporter = fg_ReportMemoryTo(nullptr);
	}

	inline CDisableMemoryReporterScope::~CDisableMemoryReporterScope()
	{
		fg_ReportMemoryTo(mp_pOldReporter);
	}

	inline void CDisableMemoryReporterScope::f_Suspend() noexcept
	{
		fg_ReportMemoryTo(mp_pOldReporter);
	}

	inline void CDisableMemoryReporterScope::f_ResumeNoExcept() noexcept
	{
		mp_pOldReporter = fg_ReportMemoryTo(nullptr);
	}

	inline CMemoryReportGoingToReportScope::CMemoryReportGoingToReportScope(umint _Allocator, bool _bReport)
		: mp_Allocator(_Allocator)
		, mp_bReport(_bReport)
	{
		if (_bReport)
			fg_ReportMemoryGoingToReportEnter(_Allocator);
	}

	inline CMemoryReportGoingToReportScope::~CMemoryReportGoingToReportScope()
	{
		if (mp_bReport)
			fg_ReportMemoryGoingToReportExit(mp_Allocator);
	}
#endif
}
