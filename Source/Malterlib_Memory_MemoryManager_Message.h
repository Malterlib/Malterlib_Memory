// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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
		mint m_Next;
	};

	struct CMessage_FreeNormalBlock : public CMessage
	{
	};

	struct CMessage_FreeSmallBlock : public CMessage
	{
		void *m_pBlock;
	};
}
