// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	enum EMessageType
	{
		EMessageType_FreeNormalBlock = 0
		, EMessageType_FreeSmallBlock
	};

	struct CMessage
	{
		umint m_Next;
	};

	struct CMessage_FreeNormalBlock : public CMessage
	{
	};

	struct CMessage_FreeSmallBlock : public CMessage
	{
		void *m_pBlock;
	};
}
