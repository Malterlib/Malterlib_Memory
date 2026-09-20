// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Memory/Construct>
#include <Mib/Memory/Allocators/Default>

namespace
{
	using namespace NMib;
	using namespace NMib::NTest;

	// Records what the object's memory was freed with, which for a class with a
	// virtual destructor is what that destructor reported for the dynamic type.
	struct CRecordingAllocator : public NMemory::CAllocator_Heap
	{
		enum
		{
			mc_bIsDefault = false
		};

		static void f_Free(void *_pBlock, umint _Size)
		{
			ms_pFreed = _pBlock;
			ms_FreedSize = _Size;

			NMemory::CAllocator_Heap::f_Free(_pBlock, _Size);
		}

		static void *ms_pFreed;
		static umint ms_FreedSize;
	};

	void *CRecordingAllocator::ms_pFreed = nullptr;
	umint CRecordingAllocator::ms_FreedSize = 0;

	aint g_nDestroyed = 0;

	struct CBase
	{
		virtual ~CBase()
		{
			++g_nDestroyed;
		}

		long m_Base = 0;
	};

	struct CDerived : public CBase
	{
		~CDerived() override
		{
			++g_nDestroyed;
		}

		long m_Derived[3] = {};
	};

	struct CSecondBase
	{
		virtual ~CSecondBase()
		{
			++g_nDestroyed;
		}

		virtual void f_Second()
		{
		}

		long m_Second = 0;
	};

	struct CMultiple : public CBase, public CSecondBase
	{
		long m_Multiple[2] = {};
	};

	struct CVirtualDerived : public virtual CBase
	{
		long m_VirtualDerived[4] = {};
	};

	struct CFinal final : public CBase
	{
		long m_Final[5] = {};
	};

	struct CPlain
	{
		long m_Plain[2] = {};
	};

	class CSizedDestruction_Tests : public CTest
	{
	public:

		void f_DoTests()
		{
			DMibTestSuite("SizedDestruction")
			{
				DMibTestCategory("MostDerivedSize")
				{
					g_nDestroyed = 0;

					CBase *pObject = fg_ConstructObject<CDerived>(CRecordingAllocator());
					void *pMemory = pObject;

					fg_DeleteObject(CRecordingAllocator(), pObject);

					DMibExpect(g_nDestroyed, ==, 2);
					DMibExpect(CRecordingAllocator::ms_pFreed, ==, pMemory);
					DMibExpect(CRecordingAllocator::ms_FreedSize, ==, sizeof(CDerived));
				};

				DMibTestCategory("SecondBase")
				{
					g_nDestroyed = 0;

					CMultiple *pMultiple = fg_ConstructObject<CMultiple>(CRecordingAllocator());
					void *pMemory = pMultiple;
					CSecondBase *pObject = pMultiple;

					// The second base lives at an offset, so the destructor has to
					// hand back the address of the complete object.
					DMibExpectTrue((void *)pObject != pMemory);

					fg_DeleteObject(CRecordingAllocator(), pObject);

					DMibExpect(g_nDestroyed, ==, 2);
					DMibExpect(CRecordingAllocator::ms_pFreed, ==, pMemory);
					DMibExpect(CRecordingAllocator::ms_FreedSize, ==, sizeof(CMultiple));
				};

				DMibTestCategory("VirtualBase")
				{
					g_nDestroyed = 0;

					CVirtualDerived *pDerived = fg_ConstructObject<CVirtualDerived>(CRecordingAllocator());
					void *pMemory = pDerived;
					CBase *pObject = pDerived;

					fg_DeleteObject(CRecordingAllocator(), pObject);

					DMibExpect(g_nDestroyed, ==, 1);
					DMibExpect(CRecordingAllocator::ms_pFreed, ==, pMemory);
					DMibExpect(CRecordingAllocator::ms_FreedSize, ==, sizeof(CVirtualDerived));
				};

				DMibTestCategory("StaticSize")
				{
					// A final class and a class without a virtual destructor are
					// freed with the size of their static type.
					g_nDestroyed = 0;

					CFinal *pFinal = fg_ConstructObject<CFinal>(CRecordingAllocator());
					fg_DeleteObject(CRecordingAllocator(), pFinal);

					DMibExpect(g_nDestroyed, ==, 1);
					DMibExpect(CRecordingAllocator::ms_FreedSize, ==, sizeof(CFinal));

					CPlain *pPlain = fg_ConstructObject<CPlain>(CRecordingAllocator());
					fg_DeleteObject(CRecordingAllocator(), pPlain);

					DMibExpect(CRecordingAllocator::ms_FreedSize, ==, sizeof(CPlain));
				};

				DMibTestCategory("DeleteExpression")
				{
					// The other mode of the deleting destructor still frees the
					// object itself.
					g_nDestroyed = 0;

					CBase *pObject = new CDerived;
					delete pObject;

					DMibExpect(g_nDestroyed, ==, 2);
				};
			};
		}
	};

	DMibTestRegister(CSizedDestruction_Tests, Malterlib::Memory);
}
