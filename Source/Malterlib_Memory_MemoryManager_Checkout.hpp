// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	inline CMemoryManagerCheckout::CMemoryManagerCheckout(CMemoryManagerCheckout &&_Other)
		: m_pThreadLocal(_Other.m_pThreadLocal)
	{
		_Other.m_pThreadLocal = nullptr;
	}

	inline CMemoryManagerCheckout & CMemoryManagerCheckout::operator = (CMemoryManagerCheckout && _Other)
	{
		if (m_pThreadLocal)
			m_pThreadLocal->f_ReturnCheckoutVirtual();

		m_pThreadLocal = _Other.m_pThreadLocal;
		_Other.m_pThreadLocal = nullptr;
		return *this;
	}

	inline CMemoryManagerCheckout::CMemoryManagerCheckout(ICMemoryManagerReturnCheckout *_pThreadLocal)
		: m_pThreadLocal(_pThreadLocal)
	{

	}

	inline CMemoryManagerCheckout::~CMemoryManagerCheckout()
	{
		if (m_pThreadLocal)
			m_pThreadLocal->f_ReturnCheckoutVirtual();
	}

	inline bool CMemoryManagerCheckout::f_IsCheckedOut() const
	{
		return m_pThreadLocal != nullptr;
	}

	inline void CMemoryManagerCheckout::f_TemporaryReturn()
	{
		if (m_pThreadLocal)
			m_pThreadLocal->f_TemporaryReturn();
	}

	inline void CMemoryManagerCheckout::f_TemporaryGetBack()
	{
		if (m_pThreadLocal)
			m_pThreadLocal->f_TemporaryGetBack();
	}

	inline void CMemoryManagerCheckout::f_TakeOwnership()
	{
		if (m_pThreadLocal)
			m_pThreadLocal->f_TakeOwnership();
	}

	inline void CMemoryManagerCheckout::f_RelinquishOwnership()
	{
		if (m_pThreadLocal)
			m_pThreadLocal->f_RelinquishOwnership();
	}

	inline void CMemoryManagerCheckout::f_CheckMessages()
	{
		if (m_pThreadLocal)
		{
			m_pThreadLocal->f_TemporaryReturn();
			m_pThreadLocal->f_TemporaryGetBack();
		}
	}

	inline void CMemoryManagerCheckout::f_GarbageCollectLocalArena(bool _bDecommit)
	{
		if (m_pThreadLocal)
		{
			if (m_pThreadLocal->m_Version < 0x102)
			{
				m_pThreadLocal->f_TemporaryReturn();
				m_pThreadLocal->f_TemporaryGetBack();
				return;
			}
			m_pThreadLocal->f_GarbageCollectLocalArena(_bDecommit);
		}
	}

	inline bool CMemoryManagerCheckout::f_GarbageCollectLocalArenaIfPending()
	{
		if (m_pThreadLocal)
		{
			if (m_pThreadLocal->m_Version < 0x103)
			{
				f_GarbageCollectLocalArena(false);
				return true;
			}
			return m_pThreadLocal->f_GarbageCollectLocalArenaIfPending();
		}
		return false;
	}
}
