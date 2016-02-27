// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
		/// Keep in mind that these two classes are used in the cross module interface and has to be versioned properly if changed
		
		enum 
		{
			ECMemoryManagerReturnCheckoutVersion = 0x101
		};
		struct ICMemoryManagerReturnCheckout
		{
			uint32 m_Version;

			// Available in version 0x101
			virtual void f_ReturnCheckoutVirtual() pure;
			virtual void f_TemporaryReturn() pure;
			virtual void f_TemporaryGetBack() pure;
			virtual void f_TakeOwnership() pure;
			virtual void f_RelinquishOwnership() pure;
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

			void f_TemporaryReturn();
			void f_TemporaryGetBack();
			void f_TakeOwnership();
			void f_RelinquishOwnership();
			void f_CheckMessages();
		};
	}
}

#include "Malterlib_Memory_MemoryManager_Checkout.hpp"
