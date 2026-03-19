// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_Memory_Allocator_Heap.h"

#ifndef DMibNoInlineNew
#	error "You must define this when compling this file"
#endif

namespace NMib::NMemory
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

			NThread::TCThreadLocal<CThreadLocal, NMemory::CAllocator_NonTrackedHeap, NThread::EThreadLocalFlag_AlwaysCreated> m_ThreadLocal;
		};

		constinit TCSubSystem<CSubSystem_Memory, ESubSystemDestruction_BeforeThreadLocals> g_SubSystem_Memory = {DAggregateInit};
	}

	inline_always_lto CCaptureDefaultDelete::CCaptureDefaultDelete()
	{
		auto &ThreadLocal = *NPrivate::g_SubSystem_Memory->m_ThreadLocal;
		m_pPrevious = ThreadLocal.m_pCapture;
		ThreadLocal.m_pCapture = this;
	}

	inline_always_lto CCaptureDefaultDelete::~CCaptureDefaultDelete()
	{
		auto &ThreadLocal = *NPrivate::g_SubSystem_Memory->m_ThreadLocal;
		ThreadLocal.m_pCapture = m_pPrevious;
	}

	namespace
	{
		inline_never void fg_ReportDeleteSlowPath(void *_pMemory, umint _Size) noexcept
		{
			if (_Size)
				fg_Free(_pMemory, _Size);
			else
				fg_FreeNoSize(_pMemory);
		}
	}

	inline_always_lto bool CCaptureDefaultDelete::fs_ReportDelete(void *_pMemory, umint _Size) noexcept
	{
		if (!NPrivate::g_SubSystem_Memory.f_WasCreated() || !_pMemory) [[unlikely]]
			return false;

		auto &SubSystem = *NPrivate::g_SubSystem_Memory;

		DMibFastCheck(!SubSystem.m_bDestroyed);
		DMibFastCheck(!SubSystem.m_bThreadLocalsDestroyed);

		auto *pThreadLocal = SubSystem.m_ThreadLocal.f_TryGet();
		if (!pThreadLocal) [[unlikely]]
			return false;

		if (!pThreadLocal->m_pCapture) [[unlikely]]
			return false;

		auto &Capture = *pThreadLocal->m_pCapture;
		if (Capture.m_Captured.m_pMemory) [[unlikely]] // Recursive delete
			fg_ReportDeleteSlowPath(Capture.m_Captured.m_pMemory, Capture.m_Captured.m_Size);

		Capture.m_Captured.m_pMemory = _pMemory;
		Capture.m_Captured.m_Size = _Size;

		return true;
	}
#endif

	void fg_Mem_InitSubsystem()
	{
#ifdef DMibPOverrideOperatorNew
		*NPrivate::g_SubSystem_Memory;
#endif
	}

	bool g_MalterlibMemoryManager_Debug_EnableStackTrace = DMibConfig_MalterlibMemoryManager_Debug_EnableStackTrace;

	ch8 CAllocator_Virtual::ms_HeapName[] = "Virtual";

#if DMibConfig_Memory_Shims_Enable
	CAllocator_Virtual::CHeapInit::CHeapInit()
	{
		DMibMemoryGoingToReportScope(this, true);
		DMibMemoryReportAllocatorName(ms_HeapName, ms_HeapName);
	}

	CAllocator_Virtual::CHeapInit CAllocator_Virtual::ms_HeapInit;
#endif

}

#if defined(DMibPOverrideOperatorNew)
#if defined(DMibSanitizerEnabled) && !defined(DPlatformFamily_macOS) && !defined(DPlatformFamily_Windows)
	// operator delete(void*, std::align_val_t)
	extern "C" void __wrap__ZdlPvSt11align_val_tRKSt9nothrow_t(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow)
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

#if DMibPPtrBits >= 64
	// operator delete(void*, unsigned long, std::align_val_t)
	//extern "C" void __real__ZdlPvmSt11align_val_t(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment);
	extern "C" void __wrap__ZdlPvmSt11align_val_t(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment)
#else
	extern "C" void __wrap__ZdlPvjSt11align_val_t(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment)
#endif
	{
		umint Size = NMib::fg_AlignUp(_Size, (umint)_Alignment);
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, Size))
			return;

		NMib::NMemory::fg_Free(_pMemory, Size);
	}

	// operator delete(void*, std::align_val_t)
	//extern "C" void __real__ZdlPvSt11align_val_t(void *_pMemory, std::align_val_t _Alignment);
	extern "C" void __wrap__ZdlPvSt11align_val_t(void *_pMemory, std::align_val_t _Alignment)
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

#if DMibPPtrBits >= 64
	// operator new(unsigned long)
	extern "C" void *__wrap__Znwm(std::size_t _Size)
#else
	extern "C" void *__wrap__Znwj(std::size_t _Size)
#endif
	{
		return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));
	}

#if DMibPPtrBits >= 64
	// operator new(unsigned long)
	extern "C" void *__wrap__ZnwmRKSt9nothrow_t(std::size_t _Size, std::nothrow_t const &) noexcept
#else
	extern "C" void *__wrap__ZnwjRKSt9nothrow_t(std::size_t _Size, std::nothrow_t const &) noexcept
#endif
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	// operator delete(void *)
	extern "C" void __wrap__ZdlPv(void *_pMemory) noexcept
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	// operator delete(void *)
	extern "C" void __wrap__ZdlPvRKSt9nothrow_t(void *_pMemory, std::nothrow_t const &) noexcept
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

#if DMibPPtrBits >= 64
	// operator delete(void*, unsigned long)
	extern "C" void __wrap__ZdlPvm(void *_pMemory, std::size_t _Size) noexcept
#else
	extern "C" void __wrap__ZdlPvj(void *_pMemory, std::size_t _Size) noexcept
#endif
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, _Size))
			return;

		NMib::NMemory::fg_Free(_pMemory, _Size);
	}

#if DMibPPtrBits >= 64
	// operator new[](unsigned long)
	extern "C" void * __wrap__Znam(std::size_t _Size)
#else
	extern "C" void * __wrap__Znaj(std::size_t _Size)
#endif
	{
		return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));;
	}

#if DMibPPtrBits >= 64
	// operator new[](unsigned long)
	extern "C" void * __wrap__ZnamRKSt9nothrow_t(std::size_t _Size, std::nothrow_t const &) noexcept
#else
	extern "C" void * __wrap__ZnajRKSt9nothrow_t(std::size_t _Size, std::nothrow_t const &) noexcept
#endif
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	// operator delete[](void *)
	extern "C" void __wrap__ZdaPv(void *_pMemory) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	// operator delete[](void *)
	extern "C" void __wrap__ZdaPvRKSt9nothrow_t(void *_pMemory, std::nothrow_t const &) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

#if DMibPPtrBits >= 64
	// operator delete[](void*, unsigned long)
	extern "C" void __wrap__ZdaPvm(void *_pMemory, std::size_t _Size) noexcept
#else
	extern "C" void __wrap__ZdaPvj(void *_pMemory, std::size_t _Size) noexcept
#endif
	{
		NMib::NMemory::fg_Free(_pMemory, _Size);
	}

#if DMibPPtrBits >= 64
	// operator new(unsigned long, std::align_val_t)
	extern "C" void * __wrap__ZnwmSt11align_val_t(std::size_t _Size, std::align_val_t _Alignment)
#else
	// operator new(unsigned int, std::align_val_t)
	extern "C" void * __wrap__ZnwjSt11align_val_t(std::size_t _Size, std::align_val_t _Alignment)
#endif
	{
		return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
	}

#if DMibPPtrBits >= 64
	// operator new(unsigned long, std::align_val_t)
	extern "C" void * __wrap__ZnwmSt11align_val_tRKSt9nothrow_t(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
#else
	// operator new(unsigned int, std::align_val_t)
	extern "C" void * __wrap__ZnwjSt11align_val_tRKSt9nothrow_t(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
#endif
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

#if DMibPPtrBits >= 64
	// operator new[](unsigned long, std::align_val_t)
	extern "C" void * __wrap__ZnamSt11align_val_t(std::size_t _Size, std::align_val_t _Alignment)
#else
	extern "C" void * __wrap__ZnajSt11align_val_t(std::size_t _Size, std::align_val_t _Alignment)
#endif
	{
		return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
	}

#if DMibPPtrBits >= 64
	// operator new[](unsigned long, std::align_val_t)
	extern "C" void * __wrap__ZnamSt11align_val_tRKSt9nothrow_t(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
#else
	extern "C" void * __wrap__ZnajSt11align_val_tRKSt9nothrow_t(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
#endif
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	// operator delete[](void*, std::align_val_t)
	extern "C" void __wrap__ZdaPvSt11align_val_t(void *_pMemory, std::align_val_t _Alignment) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	// operator delete[](void*, std::align_val_t)
	extern "C" void __wrap__ZdaPvSt11align_val_tRKSt9nothrow_t(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

#if DMibPPtrBits >= 64
	// operator delete[](void*, unsigned long, std::align_val_t)
	extern "C" void __wrap__ZdaPvmSt11align_val_t(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
#else
	extern "C" void __wrap__ZdaPvjSt11align_val_t(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
#endif
	{
		umint Size = NMib::fg_AlignUp(_Size, (umint)_Alignment);
		NMib::NMemory::fg_Free(_pMemory, Size);
	}

#else

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment) noexcept
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
	{
		umint Size = NMib::fg_AlignUp(_Size, (umint)_Alignment);
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, Size))
			return;

		NMib::NMemory::fg_Free(_pMemory, Size);
	}

		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size) // _Znwm
	{
		return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::nothrow_t const &) noexcept // _ZnwmRKSt9nothrow_t
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory) noexcept // _ZdlPv
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::nothrow_t const &) noexcept // _ZdlPvRKSt9nothrow_t
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
			return;

		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size) noexcept // _ZdlPvm
	{
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, _Size))
			return;

		NMib::NMemory::fg_Free(_pMemory, _Size);
	}


	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size) // _Znam
	{
		return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));;
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::nothrow_t const &) noexcept // _ZnamRKSt9nothrow_t
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, umint(1) << NMib::fg_GetLowestBitSetNoZero(_Size));
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory) noexcept // _ZdaPv
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::nothrow_t const &) noexcept // _ZdaPvRKSt9nothrow_t
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size) noexcept // _ZdaPvm
	{
		NMib::NMemory::fg_Free(_pMemory, _Size);
	}


	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment) // _ZnwmSt11align_val_t
	{
		return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept // _ZnwmSt11align_val_tRKSt9nothrow_t
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment) // _ZnamSt11align_val_t
	{
		return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
	}

	only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept // _ZnamSt11align_val_tRKSt9nothrow_t
	{
		try
		{
			return NMib::NMemory::fg_AllocAligned(_Size, (umint)_Alignment);
		}
		catch (NMib::NException::CException const &)
		{
			return nullptr;
		}
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment) noexcept // _ZdaPvSt11align_val_t
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept // _ZdaPvSt11align_val_tRKSt9nothrow_t
	{
		NMib::NMemory::fg_FreeNoSize(_pMemory);
	}

	only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept // _ZdaPvmSt11align_val_t
	{
		umint Size = NMib::fg_AlignUp(_Size, (umint)_Alignment);
		NMib::NMemory::fg_Free(_pMemory, Size);
	}
#endif
#endif
