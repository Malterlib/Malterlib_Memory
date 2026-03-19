// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	struct CCapturedDelete
	{
		void *m_pMemory = nullptr;
		umint m_Size = 0;
	};

#ifdef DMibPOverrideOperatorNew
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
