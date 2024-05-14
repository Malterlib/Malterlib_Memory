// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

// #define DMibDebugPoolAllocations

namespace NMib::NMemory
{
	template <typename t_CData, aint t_GrowSize, typename t_CLock, typename t_CPoolType, typename t_CAllocator, typename t_CAggregateData, typename t_CDebugStrType>
	void TCPoolAggregate<t_CData, t_GrowSize, t_CLock, t_CPoolType, t_CAllocator, t_CAggregateData, t_CDebugStrType>::f_Construct(ENumaNode _NumaNode)
	{
		t_CAggregateData::f_Construct();
		m_Pool.f_Construct(_NumaNode, t_GrowSize);
#	if DMibConfig_Memory_Shims_Enable
		if constexpr (mc_Reporting)
		{
			m_DebugName = (NStr::CFStr256::CFormat("Pool {}") << fg_GetTypeName<t_CData>()).f_GetStr();
			DMibMemoryGoingToReportScope(this, true);
			DMibMemoryReportAllocatorName(this, m_DebugName.f_GetStr());
		}
#	endif
		t_CAggregateData::f_InitDone();

	}


	template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
	void CPoolType_Growing::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Destruct(ch8 const *_pTypeName)
	{
#if defined(DMibDebug) && !defined(DMibSanitizerEnabled_Thread)
		if (m_NumUsed != 0)
			DMibDTraceSafe("Memory leak in pool: {}" DMibNewLine, _pTypeName);
#endif
		// Make Freeblocks delete faster
		m_FreeBlocks.f_ClearFast();
		m_FreeBlocks.f_Destruct();

		CChunk *pChunkToDelete = m_Chunks.f_Pop();
		while (pChunkToDelete )
		{
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_Chunks.f_Pop();
		}
		m_Chunks.f_Destruct();
	}

	template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
	void CPoolType_Freeable::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Destruct(ch8 const *_pTypeName)
	{
#if defined(DMibDebug) && !defined(DMibSanitizerEnabled_Thread)
		if (!m_Chunks.f_IsEmpty() || !m_FreeChunks.f_IsEmpty())
			DMibDTraceSafe("Memory leak in pool: {}" DMibNewLine, _pTypeName);
#endif

		CChunk *pChunkToDelete = m_Chunks.f_Pop();
		while (pChunkToDelete)
		{
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_Chunks.f_Pop();
		}
		pChunkToDelete = m_FreeChunks.f_Pop();
		while (pChunkToDelete)
		{
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_FreeChunks.f_Pop();
		}
		pChunkToDelete = m_EmptyChunks.f_Pop();
		while (pChunkToDelete)
		{
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_EmptyChunks.f_Pop();
		}
		m_Chunks.f_Destruct();
		m_FreeChunks.f_Destruct();
		m_EmptyChunks.f_Destruct();
	}

	template <typename t_CAllocator, mint t_DataSize, mint t_Alignment>
	void CPoolType_FreeableSmall::TCPool<t_CAllocator, t_DataSize, t_Alignment>::f_Destruct(ch8 const *_pTypeName)
	{
#if defined(DMibDebug) && !defined(DMibSanitizerEnabled_Thread)
		if (!m_Chunks.f_IsEmpty() || !m_FreeChunks.f_IsEmpty())
			DMibDTraceSafe("Memory leak in pool: {}" DMibNewLine, _pTypeName);
#endif

		CChunk *pChunkToDelete = m_Chunks.f_Pop();
		while (pChunkToDelete)
		{
			m_ChunkTree.f_Remove(pChunkToDelete);
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_Chunks.f_Pop();
		}
		pChunkToDelete = m_FreeChunks.f_Pop();
		while (pChunkToDelete)
		{
			m_ChunkTree.f_Remove(pChunkToDelete);
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_FreeChunks.f_Pop();
		}
		pChunkToDelete = m_EmptyChunks.f_Pop();
		while (pChunkToDelete)
		{
			m_ChunkTree.f_Remove(pChunkToDelete);
			pChunkToDelete->~CChunk();
			fs_FreeChunk(pChunkToDelete);
			pChunkToDelete = m_EmptyChunks.f_Pop();
		}
		m_Chunks.f_Destruct();
		m_FreeChunks.f_Destruct();
		m_EmptyChunks.f_Destruct();
	}
}
