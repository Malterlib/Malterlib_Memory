// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	// Keep in mind that these two classes are used in the cross module interface and has to be versioned properly if changed

	enum
	{
		ECMemoryManagerReturnCheckoutVersion = 0x103
	};

	struct ICMemoryManagerReturnCheckout
	{
		uint32 m_Version;

		// Available in version 0x101
		virtual void f_ReturnCheckoutVirtual() = 0;
		virtual void f_TemporaryReturn() = 0;
		virtual void f_TemporaryGetBack() = 0;
		virtual void f_TakeOwnership() = 0;
		virtual void f_RelinquishOwnership() = 0;
		virtual void f_GarbageCollectLocalArena(bool _bDecommit) = 0;

		// Available in version 0x103
		virtual bool f_GarbageCollectLocalArenaIfPending() = 0;
	};

	class CMemoryManagerCheckout
	{
		ICMemoryManagerReturnCheckout *m_pThreadLocal;

		CMemoryManagerCheckout(CMemoryManagerCheckout const &_Other);
		CMemoryManagerCheckout &operator = (CMemoryManagerCheckout const &_Other);
	public:
		CMemoryManagerCheckout(CMemoryManagerCheckout &&_Other);
		CMemoryManagerCheckout(ICMemoryManagerReturnCheckout *_pThreadLocal);
		CMemoryManagerCheckout &operator = (CMemoryManagerCheckout &&_Other);
		~CMemoryManagerCheckout();

		bool f_IsCheckedOut() const;

		void f_TemporaryReturn();
		void f_TemporaryGetBack();
		void f_TakeOwnership();
		void f_RelinquishOwnership();
		void f_CheckMessages();
		void f_GarbageCollectLocalArena(bool _bDecommit);
		bool f_GarbageCollectLocalArenaIfPending();
	};
}

#include "Malterlib_Memory_MemoryManager_Checkout.hpp"
