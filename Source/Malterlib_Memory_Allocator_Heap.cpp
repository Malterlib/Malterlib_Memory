// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_Memory_Allocator_Heap.h"

#ifndef DMibNoInlineNew
#	error "You must define this when compling this file"
#endif

namespace NMib
{
	namespace NMem
	{
#ifdef DMibPOverrideOperatorNew
		namespace NPrivate
		{
			struct CThreadLocal
			{
				CCaptureDefaultDelete *m_pCapture = nullptr;
			};

			struct CSubSystem_Memory : public CSubSystem
			{
				~CSubSystem_Memory()
				{
					m_bDestroyed = true;
				}

				void f_DestroyThreadLocal() override
				{
					m_bThreadLocalsDestroyed = true;
					m_ThreadLocal.f_Destroy();
				}

				bool m_bThreadLocalsDestroyed = false;
				bool m_bDestroyed = false;

				NThread::TCThreadLocal<CThreadLocal, NMem::CAllocator_NonTrackedHeap, NThread::EThreadLocalFlag_AlwaysCreated> m_ThreadLocal;
			};

			TCSubSystem<CSubSystem_Memory, ESubSystemDestruction_BeforeThreadLocals> g_SubSystem_Memory = {DAggregateInit};
		}

		CCaptureDefaultDelete::CCaptureDefaultDelete()
		{
			auto &ThreadLocal = *NPrivate::g_SubSystem_Memory->m_ThreadLocal;
			m_pPrevious = ThreadLocal.m_pCapture;
			ThreadLocal.m_pCapture = this;
		}

		CCaptureDefaultDelete::~CCaptureDefaultDelete()
		{
			auto &ThreadLocal = *NPrivate::g_SubSystem_Memory->m_ThreadLocal;
			ThreadLocal.m_pCapture = m_pPrevious;
		}

		bool CCaptureDefaultDelete::fs_ReportDelete(void *_pMemory, mint _Size)
		{
			DMibFastCheck(!fg_GetSys()->f_ThreadDestroyed());

			if (!NPrivate::g_SubSystem_Memory.f_WasCreated() || !_pMemory)
				return false;

			auto &SubSystem = *NPrivate::g_SubSystem_Memory;

			DMibFastCheck(!SubSystem.m_bDestroyed);
			DMibFastCheck(!SubSystem.m_bThreadLocalsDestroyed);

			auto *pThreadLocal = SubSystem.m_ThreadLocal.f_TryGet();
			if (!pThreadLocal)
				return false;
			if (!pThreadLocal->m_pCapture)
				return false;
			auto &Capture = *pThreadLocal->m_pCapture;
			if (Capture.m_pMemory) // Recursive delete
			{
				if (Capture.m_Size)
					fg_Free(Capture.m_pMemory, Capture.m_Size);
				else
					fg_FreeNoSize(Capture.m_pMemory);
			}
			Capture.m_pMemory = _pMemory;
			Capture.m_Size = _Size;
			return true;
		}
#endif

		/************************************************************************************************\
		||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
		|| Heap alloc functions
		||______________________________________________________________________________________________||
		\************************************************************************************************/
		
		bint g_MalterlibMemoryManager_Debug_EnableStackTrace = DMibConfig_MalterlibMemoryManager_Debug_EnableStackTrace;
		
		ch8 CAllocator_Virtual::ms_HeapName[] = "Virtual";

#if DMibConfig_Memory_Shims_Enable
		CAllocator_Virtual::CHeapInit CAllocator_Virtual::ms_HeapInit;
#endif
	}
}


/************************************************************************************************\
||ﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯﾯ||
|| new operators
||______________________________________________________________________________________________||
\************************************************************************************************/


#if defined(DMibPOverrideOperatorNew)

	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size)
	{
		return NMib::NMem::fg_Alloc(_Size);
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::nothrow_t const &) noexcept
	{
		try
		{
			return NMib::NMem::fg_Alloc(_Size);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory) noexcept
	{
		if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::nothrow_t const &) noexcept
	{
		if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size) noexcept
	{
		if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, _Size))
			return;

		NMib::NMem::fg_Free(_pMemory, _Size);
	}


	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size)
	{
		return NMib::NMem::fg_Alloc(_Size);
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::nothrow_t const &) noexcept
	{
		try
		{
			return NMib::NMem::fg_Alloc(_Size);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::nothrow_t const &) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size) noexcept
	{
		NMib::NMem::fg_Free(_pMemory, _Size);
	}


	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment)
	{
		return NMib::NMem::fg_AllocAligned(_Size, (mint)_Alignment);
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
	{
		try
		{
			return NMib::NMem::fg_AllocAligned(_Size, (mint)_Alignment);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment) noexcept
	{
		if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
	{
		if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
	{
		mint Size = NMib::fg_AlignUp(_Size, (mint)_Alignment);
		if (NMib::NMem::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, Size))
			return;

		NMib::NMem::fg_Free(_pMemory, Size);
	}


	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment)
	{
		return NMib::NMem::fg_AllocAligned(_Size, (mint)_Alignment);
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
	{
		try
		{
			return NMib::NMem::fg_AllocAligned(_Size, (mint)_Alignment);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
	{
		NMib::NMem::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
	{
		mint Size = NMib::fg_AlignUp(_Size, (mint)_Alignment);
		NMib::NMem::fg_Free(_pMemory, Size);
	}
#endif
