// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
#if DMibConfig_MemoryManager_Stats_EnableCategories
	constexpr CMemoryCategory::CMemoryCategory(ch8 const *_pName)
		: m_pName(_pName)
		, m_Link()
	{
	}

	inline ch8 const *CMemoryCategory::CCompare::operator () (CMemoryCategory const &_Node) const
	{
		return _Node.m_pName;
	}

	inline COrdering_Partial CMemoryCategory::CCompare::operator () (ch8 const *_pLeft, ch8 const *_pRight) const
	{
		return NStr::fg_StrCmp(_pLeft, _pRight) <=> 0;
	}

	inline CMemoryCategoryScope::CMemoryCategoryScope(CMemoryCategory *_pScope)
		: mp_pOldCategory(fg_Mem_SetCategory(_pScope))
		, mp_pNewCategory(_pScope)
	{
	}

	inline CMemoryCategoryScope::~CMemoryCategoryScope()
	{
		fg_Mem_SetCategory(mp_pOldCategory);
	}

	inline void CMemoryCategoryScope::f_Suspend() noexcept
	{
		fg_Mem_SetCategory(mp_pOldCategory);
	}

	inline void CMemoryCategoryScope::f_ResumeNoExcept() noexcept
	{
		mp_pOldCategory = fg_Mem_SetCategory(mp_pNewCategory);
	}
#endif
}
