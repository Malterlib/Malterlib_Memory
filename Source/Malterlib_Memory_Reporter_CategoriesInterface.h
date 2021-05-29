
#pragma once
#include <Mib/Atomic/Atomic>

namespace NMib::NMemory
{
#if DMibConfig_MemoryManager_Stats_EnableCategories
	struct CMemoryCategory
	{
		constexpr CMemoryCategory(ch8 const *_pName)
			: m_pName(_pName)
			, m_Link()
		{
		}

		ch8 const *m_pName;
		NIntrusive::TCAVLLinkAggregate<> m_Link;
		align_cacheline NAtomic::TCAtomicAggregate<smint> m_nBytes{0};
		align_cacheline NAtomic::TCAtomicAggregate<smint> m_nAllocations{0};
		align_cacheline NAtomic::TCAtomicAggregate<bool> m_AddedToList{false};

		class CCompare
		{
		public:
			inline_small ch8 const *operator() (CMemoryCategory const &_Node) const
			{
				return _Node.m_pName;
			}
			bool operator() (ch8 const *_pLeft, ch8 const *_pRight) const
			{
				return NStr::fg_StrCmp(_pLeft, _pRight) < 0;
			}
		};
	};

	struct CTrackedAllocationInfo
	{
		CMemoryCategory *m_pCategory;
	};

#	define DMibMemoryDefineCategory(d_Name) extern CMemoryCategory g_MemoryCategory_##d_Name

	CMemoryCategory *fg_Mem_SetCategory(CMemoryCategory *_pCategory);

	CMemoryCategory *fg_Mem_DefineDynamicCategory(ch8 const *_pName);

	struct CMemoryCategoryScope final : public CCoroutineThreadLocalHandler
	{
		CMemoryCategoryScope(CMemoryCategory *_pScope)
			: mp_pOldCategory(fg_Mem_SetCategory(_pScope))
			, mp_pNewCategory(_pScope)
		{
		}
		~CMemoryCategoryScope()
		{
			fg_Mem_SetCategory(mp_pOldCategory);
		}
		void f_Suspend() override
		{
			fg_Mem_SetCategory(mp_pOldCategory);
		}
		void f_Resume() override
		{
			mp_pOldCategory = fg_Mem_SetCategory(mp_pNewCategory);
		}
	private:
		CMemoryCategory *mp_pNewCategory;
		CMemoryCategory *mp_pOldCategory;
	};

#	define DMibMemoryImplementCategory(d_Name) NMib::NMemory::CMemoryCategory g_MemoryCategory_##d_Name = {DMibStringize(d_Name)}

#	define DMibMemoryDefineDynamicCategory(d_Name) NMib::NMemory::fg_Mem_DefineDynamicCategory(d_Name);

#	define DMibMemoryCategory(d_Name) NMib::NMemory::CMemoryCategoryScope MemoryCategoryScope(&g_MemoryCategory_##d_Name)

#	define DMibMemoryDynamicCategory(d_pCategory) NMib::NMemory::CMemoryCategoryScope MemoryCategoryScope(d_pCategory)

#else
#	define DMibMemoryDefineCategory(d_Name)
#	define DMibMemoryImplementCategory(d_Name)
#	define DMibMemoryDefineDynamicCategory(d_Name)
#	define DMibMemoryCategory(d_Category)
#	define DMibMemoryDynamicCategory(d_pCategory)
#endif

#ifndef DMibPNoShortCuts
#	define DMemoryDefineCategory DMibMemoryDefineCategory
#	define DMemoryImplementCategory DMibMemoryImplementCategory
#	define DMemoryDefineDynamicCategory DMibMemoryDefineDynamicCategory
#	define DMemoryCategory DMibMemoryCategory
#endif

}
