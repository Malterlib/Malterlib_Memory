// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
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
	struct DMibPPackedStruct CMemoryManagerSubSlab_NormalLinkWithBlocks
	{
		CMemoryManagerSubSlab_NormalLinkWithBlocks() = delete;
		~CMemoryManagerSubSlab_NormalLinkWithBlocks() = delete;

		DMibPPackedStruct DMibListLinkDS_Link(CMemoryManagerSubSlab_NormalLinkWithBlocks, m_Link);
		DMibPPackedStruct uint32 m_nBlocks;
	};
	DMibPEndPackedStruct;

	struct CMemoryManagerSubSlab_NormalLinkWithoutBlocks
	{
		CMemoryManagerSubSlab_NormalLinkWithoutBlocks() = delete;
		~CMemoryManagerSubSlab_NormalLinkWithoutBlocks() = delete;

		DMibListLinkDS_Link(CMemoryManagerSubSlab_NormalLinkWithoutBlocks, m_Link);
	};

	using CMemoryManagerSubSlab_NormalFreeListWithBlocks = DMibListLinkDS_List(CMemoryManagerSubSlab_NormalLinkWithBlocks, m_Link);
	using CMemoryManagerSubSlab_NormalFreeListWithoutBlocks = DMibListLinkDS_List(CMemoryManagerSubSlab_NormalLinkWithoutBlocks, m_Link);

	struct CMemoryManagerSubSlab_GarbageCollect
	{
		DMibMemoryManagerLink(CMemoryManagerSubSlab_GarbageCollect, m_LinkArena);
		DMibMemoryManagerLink(CMemoryManagerSubSlab_GarbageCollect, m_LinkSlab);
	};
}
