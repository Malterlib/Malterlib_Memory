// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_Memory_Heap.h"

namespace NMib
{
	namespace NMem
	{
		
		void CHeap_FillDebug::fs_ReportDamage(const ch8 *_pMemoryType, void *_pMem, uint8 _Value, uint8 _MemoryFill)
		{
			NSys::fg_DebugOutput((NStr::CFStr256::CFormat("DAMAGE: {} at 0x{nfh,sf0,sj*}: 0x{nfh,sf0,sj2} resetting to 0x{nfh,sf0,sj2}" DMibNewLine) << 
				_pMemoryType << (mint
				)_pMem << (sizeof(mint)*2) << _Value << _MemoryFill).f_GetStr().f_GetStr());
			static bint bBreak = true;
			if (bBreak)
			{
				DMibPDebugBreak; // Memory damaged
			}
		}

		mint CTCHeap_SizeHolderSmall::m_Size = 0;

	}
}

