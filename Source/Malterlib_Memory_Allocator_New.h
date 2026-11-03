// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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
		EHeapDebugFlag_None                 = 0
		, EHeapDebugFlag_Ignore             = DMibBit(0)
		, EHeapDebugFlag_Internal           = DMibBit(1)
		, EHeapDebugFlag_Freed              = DMibBit(2)
		, EHeapDebugFlag_FreedOnOtherThread = DMibBit(3)
	};
}

namespace NMib::NMemory
{
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_Alloc(umint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAligned(umint _Size, umint _Align);
	DMibMemory_MemoryManagerExport only_parameters_aliased void fg_Free(void *_pMemory, umint _Size);
	DMibMemory_MemoryManagerExport only_parameters_aliased void fg_FreeNoSize(void *_pMemory);

#if DMibConfig_MalterlibMemoryManager_Debug
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocDebug(umint _Size, ch8 const *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
	DMibMemory_MemoryManagerExport only_parameters_aliased malloc_like void *fg_AllocAlignedDebug
		(
			umint _Size
			, umint _Align
			, ch8 const *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags = EHeapDebugFlag_None
		)
	;
#else
	only_parameters_aliased malloc_like static inline_small void *fg_AllocDebug(umint _Size, ch8 const *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
	only_parameters_aliased malloc_like static inline_small void *fg_AllocAlignedDebug(umint _Size, umint _Align, ch8 const *_pFile, aint _Line, EHeapDebugFlag _Flags = EHeapDebugFlag_None);
#endif
}

#if !defined(DMibDefaultToolset) && defined(DMalterlibUseStaticLibCxx) && defined(_LIBCPP_DISABLE_NEW_DELETE)

// Placement new
#	ifndef __PLACEMENT_NEW_INLINE
#		define __PLACEMENT_NEW_INLINE // NOLINT(malterlib-macro-naming)
#		define DMibPMemoryPlacementNew

only_parameters_aliased malloc_like inline_always void *operator new (umint _Size, void * variable_not_aliased _pPlacement) noexcept;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept;

#	endif

#	ifndef __PLACEMENT_VEC_NEW_INLINE
#		define __PLACEMENT_VEC_NEW_INLINE // NOLINT(malterlib-macro-naming)
#		define DMibPMemoryPlacementVecNew

only_parameters_aliased malloc_like inline_always void *operator new [] (umint _Size, void * variable_not_aliased _pPlacement) noexcept;
only_parameters_aliased inline_always void operator delete [] (void *_pToDelete, void * variable_not_aliased _pPlacement) noexcept;

#	endif

#endif

template <umint tf_ArraySize>
only_parameters_aliased malloc_like inline_always void *operator new (umint _Size, uint8 _Placement[tf_ArraySize]) noexcept;

template <umint tf_ArraySize>
only_parameters_aliased inline_always void operator delete (void *_pToDelete, uint8 _Placement[tf_ArraySize]) noexcept;

#ifdef DMibPOverrideOperatorNew
	// Default new
#	if DMibPInlineActive > 0 && !defined(DMibNoInlineNew) && !defined(DCompiler_MSVC) && defined(DMalterlibUseStaticLibCxx)
inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size);
only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete (void *_pMemory) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::size_t _Size) noexcept;
inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size);
only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::size_t _Size) noexcept;
inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size, std::align_val_t _Alignment);
only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::align_val_t _Alignment) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept;
inline_always only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size, std::align_val_t _Alignment);
only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::align_val_t _Alignment) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
inline_always only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept;
#	else
#		ifdef DMalterlibUseStaticLibCxx
#			ifdef _LIBCPP_DISABLE_NEW_DELETE
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size);
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete (void *_pMemory) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::size_t _Size) noexcept;
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size);
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::size_t _Size) noexcept;
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size, std::align_val_t _Alignment);
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new (std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::align_val_t _Alignment) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete (void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept;
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size, std::align_val_t _Alignment);
DMibNewVisibility only_parameters_aliased malloc_like void * calling_convention_c operator new[] (std::size_t _Size, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::align_val_t _Alignment) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::align_val_t _Alignment, std::nothrow_t const &_NoThrow) noexcept;
DMibNewVisibility only_parameters_aliased void calling_convention_c operator delete[] (void *_pMemory, std::size_t _Size, std::align_val_t _Alignment) noexcept;
#			endif
#		endif
#	endif
#endif

#if DMibConfig_MalterlibMemoryManager_Debug
#	if defined(DMibPOverrideOperatorNew) && (defined(DMalterlibUseStaticLibCxx) || defined(DCompiler_MSVC) || defined(DCompiler_clang_cl))
only_parameters_aliased malloc_like inline_always void *operator new
	(
		std::size_t _Size
		, std::align_val_t _Alignment
		, ch8 const *_pFile
		, NMib::CLineNumber const &_Line
		, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None
	)
;
only_parameters_aliased malloc_like inline_always void *operator new[]
	(std::size_t _Size, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
;
only_parameters_aliased malloc_like inline_always void *operator new
	(
		std::size_t _Size
		, ch8 const *_pFile
		, NMib::CLineNumber const &_Line
		, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None
	)
;
only_parameters_aliased malloc_like inline_always void *operator new[]
	(std::size_t _Size, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete
	(
		void *_pToDelete
		, size_t _Size
		, std::align_val_t _Alignment
		, ch8 const *_pFile
		, NMib::CLineNumber const &_Line
		, NMib::EHeapDebugFlag _Flags
	)
	noexcept
;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags)
	noexcept
;
only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete[]
	(void *_pToDelete, size_t _Size, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept
;
only_parameters_aliased inline_always void operator delete[]
	(void *_pToDelete, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept
;
#	else
only_parameters_aliased malloc_like inline_always void *operator new
	(
		std::size_t _Size
		, std::align_val_t _Alignment
		, ch8 const *_pFile
		, NMib::CLineNumber const &_Line
		, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None
	)
;
only_parameters_aliased malloc_like inline_always void *operator new[]
	(std::size_t _Size, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
;
only_parameters_aliased malloc_like inline_always void *operator new
	(
		std::size_t _Size
		, ch8 const *_pFile
		, NMib::CLineNumber const &_Line
		, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None
	)
;
only_parameters_aliased malloc_like inline_always void *operator new[]
	(std::size_t _Size, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags = NMib::EHeapDebugFlag_None)
;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, size_t _Size, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete
	(
		void *_pToDelete
		, size_t _Size
		, std::align_val_t _Alignment
		, ch8 const *_pFile
		, NMib::CLineNumber const &_Line
		, NMib::EHeapDebugFlag _Flags
	)
	noexcept
;
only_parameters_aliased inline_always void operator delete (void *_pToDelete, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags)
	noexcept
;
only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete[] (void *_pToDelete, size_t _Size, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept;
only_parameters_aliased inline_always void operator delete[]
	(void *_pToDelete, size_t _Size, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept
;
only_parameters_aliased inline_always void operator delete[]
	(void *_pToDelete, std::align_val_t _Alignment, ch8 const *_pFile, NMib::CLineNumber const &_Line, NMib::EHeapDebugFlag _Flags) noexcept
;
#	endif
#endif

#include "Malterlib_Memory_Allocator_New.hpp"
