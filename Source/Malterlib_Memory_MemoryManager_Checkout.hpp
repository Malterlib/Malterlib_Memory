// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	inline CMemoryManagerCheckout::CMemoryManagerCheckout(CMemoryManagerCheckout &&_Other)
		: mp_pThreadLocal(_Other.mp_pThreadLocal)
	{
		_Other.mp_pThreadLocal = nullptr;
	}

	inline CMemoryManagerCheckout &CMemoryManagerCheckout::operator = (CMemoryManagerCheckout &&_Other)
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_ReturnCheckoutVirtual();

		mp_pThreadLocal = _Other.mp_pThreadLocal;
		_Other.mp_pThreadLocal = nullptr;
		return *this;
	}

	inline CMemoryManagerCheckout::CMemoryManagerCheckout(ICMemoryManagerReturnCheckout *_pThreadLocal)
		: mp_pThreadLocal(_pThreadLocal)
	{
	}

	inline CMemoryManagerCheckout::~CMemoryManagerCheckout()
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_ReturnCheckoutVirtual();
	}

	inline bool CMemoryManagerCheckout::f_IsCheckedOut() const
	{
		return mp_pThreadLocal != nullptr;
	}

	inline void CMemoryManagerCheckout::f_TemporaryReturn()
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_TemporaryReturn();
	}

	inline void CMemoryManagerCheckout::f_TemporaryGetBack()
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_TemporaryGetBack();
	}

	inline void CMemoryManagerCheckout::f_TakeOwnership()
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_TakeOwnership();
	}

	inline void CMemoryManagerCheckout::f_RelinquishOwnership()
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_RelinquishOwnership();
	}

	inline void CMemoryManagerCheckout::f_CheckMessages()
	{
		if (mp_pThreadLocal)
		{
			mp_pThreadLocal->f_TemporaryReturn();
			mp_pThreadLocal->f_TemporaryGetBack();
		}
	}

	inline void CMemoryManagerCheckout::f_GarbageCollectLocalArena(bool _bDecommit)
	{
		if (mp_pThreadLocal)
		{
			if (mp_pThreadLocal->m_Version < 0x102)
			{
				mp_pThreadLocal->f_TemporaryReturn();
				mp_pThreadLocal->f_TemporaryGetBack();
				return;
			}
			mp_pThreadLocal->f_GarbageCollectLocalArena(_bDecommit);
		}
	}

	inline bool CMemoryManagerCheckout::f_GarbageCollectLocalArenaIfPending()
	{
		if (mp_pThreadLocal)
		{
			if (mp_pThreadLocal->m_Version < 0x103)
			{
				f_GarbageCollectLocalArena(false);
				return true;
			}
			return mp_pThreadLocal->f_GarbageCollectLocalArenaIfPending();
		}
		return false;
	}
}
