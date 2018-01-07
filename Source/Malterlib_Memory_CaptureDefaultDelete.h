// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib::NMem
{
#ifdef DMibPOverrideOperatorNew
	struct CCaptureDefaultDelete
	{
		CCaptureDefaultDelete();
		~CCaptureDefaultDelete();

		static bool fs_ReportDelete(void *_pMemory, mint _Size);

		CCaptureDefaultDelete *m_pPrevious;
		void *m_pMemory = nullptr;
		mint m_Size = 0;
	};
#endif
}
