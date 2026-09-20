// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	struct CCapturedDelete
	{
		void *m_pMemory = nullptr;
		umint m_Size = 0;
	};

#if defined(DMibPOverrideOperatorNew) && !defined(DMibPSizedDestructors)
	// Captures the memory a delete expression would free, so that the caller can
	// free it with its own allocator. A compiler with sized deleting destructors
	// has no need for it: the destructor returns the memory and its size.
	struct CCaptureDefaultDelete
	{
		CCaptureDefaultDelete();
		~CCaptureDefaultDelete();

		static bool fs_ReportDelete(void *_pMemory, umint _Size) noexcept;

		CCaptureDefaultDelete *m_pPrevious;
		CCapturedDelete m_Captured;
	};
#endif
}

// Reports a delete to the capture scope that is active on this thread, and
// returns from the surrounding deallocation function when it took over the
// memory. Expands to nothing where a deleting destructor returns the size.
#if defined(DMibPOverrideOperatorNew) && !defined(DMibPSizedDestructors)
#	define DMibReturnIfDeleteCaptured(d_pMemory, d_Size) \
		if (NMib::NMemory::CCaptureDefaultDelete::fs_ReportDelete(d_pMemory, d_Size)) \
			return
#else
#	define DMibReturnIfDeleteCaptured(d_pMemory, d_Size) ((void)0)
#endif
