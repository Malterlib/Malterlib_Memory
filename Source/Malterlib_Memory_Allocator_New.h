// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_CaptureDefaultDelete.h"


#if defined(DMalterlibUseStaticLibCxx)
#	define DMibNewVisibility _LIBCPP_OVERRIDABLE_FUNC_VIS
#	include <stddef.h>

	namespace std
	{
		using ::nullptr_t;
		using ::ptrdiff_t _LIBCPP_USING_IF_EXISTS;
		using ::size_t _LIBCPP_USING_IF_EXISTS;
	}
#	include <__new/align_val_t.h>
#	include <__new/nothrow_t.h>
#else
#	define DMibNewVisibility
#	include <new>
#endif

#if defined(DMibMemoryOverrideDll)
#	define DMibMemory_MemoryManagerExport module_export
#else
#	define DMibMemory_MemoryManagerExport
#endif

namespace NMib
{
	enum EHeapDebugFlag : uint32
	{
		EHeapDebugFlag_None = 0
		, EHeapDebugFlag_Ignore		= DMibBit(0)
		, EHeapDebugFlag_Internal	= DMibBit(1)
		, EHeapDebugFlag_Freed		= DMibBit(2)
		, EHeapDebugFlag_FreedOnOtherThread	= DMibBit(3)
	};
}

namespace NMib::NMemory
{
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_Alloc(mint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAligned(mint _Size, mint _Align);
	DMibMemory_MemoryManagerExport only_parameters_aliased void fg_Free(void *_pMemory, mint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased void fg_FreeNoSize(void *_pMemory);

#	if DMibConfig_MalterlibMemoryManager_Debug
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
		DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedDebug(mint _Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
#	else
		only_parameters_aliased malloc_like static inline_small void *fg_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
		{
			return fg_Alloc(_Size);
		}
		only_parameters_aliased malloc_like static inline_small void *fg_AllocAlignedDebug(mint _Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None)
		{
			return fg_AllocAligned(_Size, _Align);
		}
#	endif

}

#if !defined(DMibDefaultToolset) && defined(DMalterlibUseStaticLibCxx) && defined(_LIBCPP_DISABLE_NEW_DELETE)

// Placement new
#ifndef __PLACEMENT_NEW_INLINE
#	define __PLACEMENT_NEW_INLINE

	only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, void * variable_not_aliased _pPlacement) noexcept
	{
		return _pPlacement;
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept
	{
	}

#endif

#ifndef __PLACEMENT_VEC_NEW_INLINE
#	define __PLACEMENT_VEC_NEW_INLINE

	only_parameters_aliased malloc_like inline_always void * operator new [] (mint _Size, void * variable_not_aliased _pPlacement) noexcept
	{
		return _pPlacement;
	}

	only_parameters_aliased inline_always void operator delete [] (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept
	{
	}

#endif

#endif


template <mint t_ArraySize>
only_parameters_aliased malloc_like inline_always void * operator new (mint _Size, uint8 _Placement[t_ArraySize]) noexcept
{
	void * variable_not_aliased pValue = _Placement;
	return pValue;
}

template <mint t_ArraySize>
only_parameters_aliased inline_always void operator delete (void *_pToDelete, uint8 _Placement[t_ArraySize]) noexcept
{
}

#ifdef DMibPOverrideOperatorNew
	// Default new
#	if DMibPInlineActive > 0 && !defined(DMibNoInlineNew) && !defined(DCompiler_MSVC) && defined(DMalterlibUseStaticLibCxx)
		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size)
		{
			return NMib::NMemory::fg_Alloc(_Size);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory) noexcept
		{
			if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::nothrow_t const &) noexcept
		{
			if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size) noexcept
		{
			if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, _Size))
				return;

			NMib::NMemory::fg_Free(_pMemory, _Size);
		}


		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size)
		{
			return NMib::NMemory::fg_Alloc(_Size);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory) noexcept
		{
			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::nothrow_t const &) noexcept
		{
			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size) noexcept
		{
			NMib::NMemory::fg_Free(_pMemory, _Size);
		}

		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment)
		{
			return NMib::NMemory::fg_AllocAligned(_Size, (mint)_Alignment);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment) noexcept
		{
			if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
		{
			if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, 0))
				return;

			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
		{
			mint Size = NMib::fg_AlignUp(_Size, (mint)_Alignment);
			if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(_pMemory, Size))
				return;

			NMib::NMemory::fg_Free(_pMemory, Size);
		}


		inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment)
		{
			return NMib::NMemory::fg_AllocAligned(_Size, (mint)_Alignment);
		}

		only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment) noexcept
		{
			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept
		{
			NMib::NMemory::fg_FreeNoSize(_pMemory);
		}

		inline_always only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept
		{
			mint Size = NMib::fg_AlignUp(_Size, (mint)_Alignment);
			NMib::NMemory::fg_Free(_pMemory, Size);
		}

#	else
#ifdef DMalterlibUseStaticLibCxx
#ifdef _LIBCPP_DISABLE_NEW_DELETE
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size);
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete(void *_pMemory) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size) noexcept;
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size);
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size) noexcept;
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment);
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new(std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete(void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept;
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment);
		DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[](std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &) noexcept;
		DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[](void *_pMemory, std::size_t _Size, std::align_val_t) noexcept;
#endif
#endif
#	endif
#endif

#if DMibConfig_MalterlibMemoryManager_Debug
#if defined(DMibPOverrideOperatorNew) && (defined(DMalterlibUseStaticLibCxx) || defined(DCompiler_MSVC) || defined(DCompiler_clang_cl))
	only_parameters_aliased malloc_like inline_always void * operator new (std::size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMemory::fg_AllocAlignedDebug(_Size, (mint)_Alignment, _pFile, _Line, _Flags);
	}

	only_parameters_aliased malloc_like inline_always void * operator new[] (std::size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMemory::fg_AllocAlignedDebug(_Size, (mint)_Alignment, _pFile, _Line, _Flags);
	}

	only_parameters_aliased malloc_like inline_always void * operator new (std::size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMemory::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased malloc_like inline_always void * operator new[] (std::size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return NMib::NMemory::fg_AllocDebug(_Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_Free(_pToDelete, _Size);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_Free(_pToDelete, _Size);
	}


	only_parameters_aliased inline_always void operator delete (void *_pToDelete, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pToDelete);
	}


	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_Free(_pToDelete, _Size);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_Free(_pToDelete, _Size);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		NMib::NMemory::fg_FreeNoSize(_pToDelete);
	}
#else
	only_parameters_aliased malloc_like inline_always void * operator new (std::size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return operator new (_Size, _Alignment);
	}

	only_parameters_aliased malloc_like inline_always void * operator new[] (std::size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return operator new[] (_Size, _Alignment);
	}

	only_parameters_aliased malloc_like inline_always void * operator new (std::size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return operator new (_Size);
	}

	only_parameters_aliased malloc_like inline_always void * operator new[] (std::size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
	{
		return operator new[] (_Size);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete (_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete (_pToDelete, _Size);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete (_pToDelete, _Size, _Alignment);
	}

	only_parameters_aliased inline_always void operator delete (void *_pToDelete, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete (_pToDelete, _Alignment);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete[] (_pToDelete);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete[] (_pToDelete, _Size);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete[] (_pToDelete, _Size, _Alignment);
	}

	only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, std::align_val_t _Alignment, const ch8 *_pFile, const NMib::CLineNumber &_Line, NMib::EHeapDebugFlag _Flags) noexcept
	{
		return operator delete[] (_pToDelete, _Alignment);
	}
#endif
#endif
