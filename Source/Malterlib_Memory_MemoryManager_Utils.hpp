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

	inline_always bool fg_IsAllocationAlignmentValid(umint _Alignment)
	{
		return _Alignment != 0 && (_Alignment & (_Alignment - 1)) == 0;
	}

	inline_always bool fg_TryAlignAllocationSize(umint &o_Size, umint _Alignment)
	{
		if (!fg_IsAllocationAlignmentValid(_Alignment))
			return false;

		umint AlignmentMask = _Alignment - 1;
		if (o_Size > TCLimitsInt<umint>::mc_Max - AlignmentMask)
			return false;

		o_Size = fg_AlignUp(o_Size, _Alignment);
		return true;
	}

	inline_always umint fg_AlignAllocationSizeOrThrow(umint _Size, umint _Alignment)
	{
		DMibFastCheck(fg_IsAllocationAlignmentValid(_Alignment));

		umint AlignmentMask = _Alignment - 1;
		if (_Size > TCLimitsInt<umint>::mc_Max - AlignmentMask)
			DMibErrorMemory("Allocation size overflow");

		return fg_AlignUp(_Size, _Alignment);
	}

	inline_always umint fg_AddAllocationSizeOrThrow(umint _Size, umint _ExtraSize)
	{
		if (_Size > TCLimitsInt<umint>::mc_Max - _ExtraSize)
			DMibErrorMemory("Allocation size overflow");

		return _Size + _ExtraSize;
	}

	inline_always umint fg_AlignAllocationSizeWithExtraOrThrow(umint _Size, umint _ExtraSize, umint _Alignment)
	{
		return fg_AlignAllocationSizeOrThrow(fg_AddAllocationSizeOrThrow(_Size, _ExtraSize), _Alignment);
	}
}
