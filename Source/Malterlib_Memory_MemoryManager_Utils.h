// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory::NPrivate
{
	inline_always uint64 fg_CalcMagic(void *_pAddress, uint64 _Magic)
	{
		uint64 Magic = _Magic ^ ((uint64)(umint)_pAddress);
		return Magic;
	}
}
