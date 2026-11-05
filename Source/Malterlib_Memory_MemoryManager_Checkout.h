// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	// Keep in mind that these two classes are used in the cross module interface and has to be versioned properly if changed

	constexpr uint32 gc_CMemoryManagerReturnCheckoutVersion = 0x103;

	struct ICMemoryManagerReturnCheckout
	{
		// Available in version 0x101
		virtual void f_ReturnCheckoutVirtual() = 0;
		virtual void f_TemporaryReturn() = 0;
		virtual void f_TemporaryGetBack() = 0;
		virtual void f_TakeOwnership() = 0;
		virtual void f_RelinquishOwnership() = 0;
		virtual void f_GarbageCollectLocalArena(bool _bDecommit) = 0;

		// Available in version 0x103
		virtual bool f_GarbageCollectLocalArenaIfPending() = 0;

		uint32 m_Version;
	};

	struct CMemoryManagerCheckout
	{
		CMemoryManagerCheckout(CMemoryManagerCheckout &&_Other);
		CMemoryManagerCheckout(ICMemoryManagerReturnCheckout *_pThreadLocal);
		~CMemoryManagerCheckout();
		CMemoryManagerCheckout &operator = (CMemoryManagerCheckout &&_Other);

		bool f_IsCheckedOut() const;

		void f_TemporaryReturn();
		void f_TemporaryGetBack();
		void f_TakeOwnership();
		void f_RelinquishOwnership();
		void f_CheckMessages();
		void f_GarbageCollectLocalArena(bool _bDecommit);
		bool f_GarbageCollectLocalArenaIfPending();

	private:
		CMemoryManagerCheckout(CMemoryManagerCheckout const &_Other);
		CMemoryManagerCheckout &operator = (CMemoryManagerCheckout const &_Other);

		ICMemoryManagerReturnCheckout *mp_pThreadLocal;
	};
}

#include "Malterlib_Memory_MemoryManager_Checkout.hpp"
