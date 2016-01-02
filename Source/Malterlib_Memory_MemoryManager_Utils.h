// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
		namespace NPrivate
		{
			inline_always uint64 fg_CalcMagic(void *_pAddress, uint64 _Magic)
			{
				uint64 Magic = _Magic ^ ((uint64)(mint)_pAddress);
				return Magic;
			}
		}
	}
}

