// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
		struct CMemoryManagerSubSlab_Free
		{
			DMibMemoryManagerLink(CMemoryManagerSubSlab_Free, m_Link);
		};

		struct CMemoryManagerSubSlab_SmallSizeLink
		{
			DMibMemoryManagerLink(CMemoryManagerSubSlab_SmallSizeLink, m_Link);
		};

		DMibPStartPackedStruct;
		struct CMemoryManagerSubSlab_NormalLink
		{
			DMibListLinkDS_Link(CMemoryManagerSubSlab_NormalLink, m_Link);
			uint32 m_nBlocks;

		private:
			CMemoryManagerSubSlab_NormalLink();
			~CMemoryManagerSubSlab_NormalLink();
			
		} DMibPPackedStruct;
		DMibPEndPackedStruct;

		struct CMemoryManagerSubSlab_GarbageCollect
		{
			DMibMemoryManagerLink(CMemoryManagerSubSlab_GarbageCollect, m_LinkArena);
			DMibMemoryManagerLink(CMemoryManagerSubSlab_GarbageCollect, m_LinkSlab);
		};

	}
}
