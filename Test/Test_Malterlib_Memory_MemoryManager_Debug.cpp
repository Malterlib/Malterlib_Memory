// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Test/Memory>
#include <Mib/Test/Exception>

#include "../../Memory/Source/Malterlib_Memory_MemoryManager.h"
#include "../../Memory/Source/Malterlib_Memory_MemoryManager.hpp"

#include "../../Memory/Source/Malterlib_Memory_MemoryManager_Debug.h"
#include "../../Memory/Source/Malterlib_Memory_MemoryManager_Debug.hpp"

#include "../../Memory/Source/Malterlib_Memory_MemoryManager_Tracked.h"

namespace
{
	using namespace NMib::NTest;
	using namespace NMib::NMemory;
	class CDebug_Tests : public CTest
	{
	public:
		struct CDebugOptions : public CMemoryManagerDebugOptionsDefault
		{
			enum
			{
				mc_bAsanPoisioning = false
				, mc_StackTraceDepth 	= 0
			};
		};

		struct CDebugOptionsWithoutGuard : public CDebugOptions
		{
			enum
			{
				mc_bAsanPoisioning = false
				, mc_nPreGuardBytes		= 0
				, mc_nPostGuardBytes	= 0
				, mc_StackTraceDepth 	= 0
			};
		};

		CDebug_Tests()
		{
		}

		struct CAlloc
		{
			void *m_pAlloc;
			mint m_Size;
		};

		void f_Dummy()
		{
		}

		void f_DoTests()
		{
			
			DMibTestSuite("All sizes batch")
			{
				CTestMemoryMeasure MeasureMemory("Alloc");
				
				NMib::NContainer::TCVector<CAlloc> Allocs;
				
				MeasureMemory.f_Start();
				
				{
					TCMemoryManagerTracked<TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions>> MemoryManager{"Test", CMemoryManagerConfig()};
					mint LastAlloc = 0;
					auto Checkout = MemoryManager.f_CheckoutForce();
					for (mint MemorySize = 1; MemorySize <= TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>::mc_MaxSlabAllocSize; ++MemorySize)
					{
						mint AllocSize = MemoryManager.f_SizePadded(MemorySize);
						if (AllocSize != LastAlloc || MemorySize < 1024)
						{
							LastAlloc = AllocSize;
							mint Size = MemorySize;
							mint nAllocs = 16;
							MemoryManager.f_AllocBatchDebug
								(
									Size
									, 1
									, [&](void * _pAlloc, mint _Size)
									{
										
										{
#if DMibConfig_Memory_Shims_Enable

											NMib::NMemory::CDisableMemoryReporterScope DisableReport;
#endif
											DMibTest(DMibExpr(_Size) >= DMibExpr(MemorySize))(ETestFlag_Aggregated);
											Allocs.f_Insert({_pAlloc, _Size});
										}
										
										if ((nAllocs--) == 0) 
											return false;
										return true;
									}
									, DMibPFile
									, DMibPLine								 
									, NMib::EHeapDebugFlag_None
								)
							;
							{
							}
						}
					}
					for (auto &Alloc : Allocs)
						MemoryManager.f_Free(Alloc.m_pAlloc, Alloc.m_Size);
					
					MemoryManager.f_GarbageCollect(true);
				}
				
				MeasureMemory.f_Stop(1);
					
				NMib::NTest::CTestMemoryResult Results;
				MeasureMemory.f_GetResults(Results);
				
#if DMibConfig_Memory_Shims_Enable
				DMibTest(DMibExpr(Results.m_AllAllocations.m_BytesAlloc.m_Average) == DMibExpr(Results.m_AllAllocations.m_BytesFree.m_Average));
#endif
			};
			DMibTestSuite("Leaks")
			{
				TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
				
				uint8 *pAlloc = nullptr;
				bool bFoundAlloc = false;

				auto fl_AllocFunctor
					=[&](uint8 *_pAlloc, mint, CMibCodeAddress*, mint, const ch8 *, uint32, uint32, mint)
					{
						if (pAlloc == _pAlloc)
							bFoundAlloc = true;
					}
				;

				{
					DMibTestPath("Small");
					mint Size = 16;
					pAlloc = (uint8 *)MemoryManager.f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_None);
					MemoryManager.f_EnumAllocations(fl_AllocFunctor);
					DMibTest(DMibExpr(bFoundAlloc));
					MemoryManager.f_Free(pAlloc, Size);
					bFoundAlloc = false;
				}
				{
					DMibTestPath("Big");
					mint Size = 512*1024;
					pAlloc = (uint8 *)MemoryManager.f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_None);
					MemoryManager.f_EnumAllocations(fl_AllocFunctor);
					DMibTest(DMibExpr(bFoundAlloc));
					MemoryManager.f_Free(pAlloc, Size);
					bFoundAlloc = false;
				}
				{
					DMibTestPath("Huge");
					mint Size = 32*1024*1024;
					pAlloc = (uint8 *)MemoryManager.f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_None);
					MemoryManager.f_EnumAllocations(fl_AllocFunctor);
					DMibTest(DMibExpr(bFoundAlloc));
					MemoryManager.f_Free(pAlloc, Size);
					bFoundAlloc = false;
				}
			};
			
			DMibTestSuite("Double free")
			{
				TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptionsWithoutGuard> MemoryManager{CMemoryManagerConfig()};
				auto Checkout = MemoryManager.f_CheckoutForce();
				
				auto DoubleFreeException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Double free");
				{
					
					DMibTestPath("Small");
					mint Size = 1;
					void *pMemory = MemoryManager.f_AllocWithSize(Size);
					MemoryManager.f_Free(pMemory, Size);

					DMibExpectException(MemoryManager.f_Free(pMemory, Size), DoubleFreeException);
				}
				{
					DMibTestPath("Normal");
					mint Size = 64;
					void *pMemory = MemoryManager.f_AllocWithSize(Size);
					MemoryManager.f_Free(pMemory, Size);
					DMibExpectException(MemoryManager.f_Free(pMemory, Size),DoubleFreeException );
				}
				{
					DMibTestPath("Big");
					mint Size = 512*1024;
					void *pMemory = MemoryManager.f_AllocWithSize(Size);
					MemoryManager.f_Free(pMemory, Size);
					DMibExpectException(MemoryManager.f_Free(pMemory, Size), DoubleFreeException);
				}
				
				// No possibility of double free as the second free will just crash
/*
				{
					DMibTestPath("Huge");
					mint Size = 32*1024*1024;
					void *pMemory = MemoryManager.f_AllocWithSize(Size);
					MemoryManager.f_Free(pMemory);
					DMibExpectException(MemoryManager.f_Free(pMemory), DoubleFreeException);
				}
 */
			};
			
			DMibTestCategory("Memory overwrite small")
			{
				DMibTestSuite("Freed")
				{
					TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptionsWithoutGuard> MemoryManager{CMemoryManagerConfig()};
					auto Checkout = MemoryManager.f_CheckoutForce();
					
					auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten after being freed");
					mint Size = 4;
					uint8 *pMemory0 = (uint8 *)MemoryManager.f_AllocWithSize(Size);
					uint8 *pMemory = (uint8 *)MemoryManager.f_AllocWithSize(Size);
					MemoryManager.f_Free(pMemory, Size);

					uint8 OldValue = pMemory[3];
					pMemory[3] = 0;
					
					Size = 4;

					DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
					DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
					DMibExpectException(MemoryManager.f_AllocWithSize(Size), OverwriteException);

					pMemory[3] = OldValue;
					
					MemoryManager.f_Free(pMemory0, Size);
				};
			};
			DMibTestCategory("Memory overwrite")
			{
				auto fl_TestOverwite
					= [&](mint _Size, ch8 const * _pDesc)
					{
						DMibTestPath(_pDesc);

						if (_Size < 16*1024*1024)
						{
							DMibTestSuite("Freed")
							{
								f_Dummy();
								TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
								auto Checkout = MemoryManager.f_CheckoutForce();
								
								auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten after being freed");
								mint Size = _Size;
								uint8 *pMemory = (uint8 *)MemoryManager.f_AllocWithSize(Size);
								MemoryManager.f_Free(pMemory, Size);

								uint8 OldValue = pMemory[0];
								pMemory[0] = 0;
								
								Size = _Size;

								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_AllocWithSize(Size), OverwriteException);

								pMemory[0] = OldValue;
							};
						}
						
						DMibTestSuite("Pre block")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten before allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocWithSize(Size);

							uint8 OldValue = pMemory[-1];
							pMemory[-1] = 0;
						
							DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
							DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
							DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

							pMemory[-1] = OldValue;
							MemoryManager.f_Free(pMemory, Size);
						};
						
						DMibTestSuite("Post block")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten after allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocWithSize(Size);

							uint8 OldValue = pMemory[Size];
							pMemory[Size] = 0;
						
							DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
							DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
							DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

							pMemory[Size] = OldValue;
							MemoryManager.f_Free(pMemory, Size);
						};

						DMibTestSuite("Pre block aligned")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten before allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, _Size * 4);

							DMibTest(DMibExpr(NMib::fg_AlignUp(pMemory, _Size * 4)) == DMibExpr(pMemory));
							
							uint8 OldValue = pMemory[-1];
							pMemory[-1] = 0;
							
							DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
							DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
							DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

							pMemory[-1] = OldValue;
							MemoryManager.f_Free(pMemory, Size);
						};
						
						DMibTestSuite("Post block alligned")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten after allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, _Size * 4);

							DMibTest(DMibExpr(NMib::fg_AlignUp(pMemory, _Size * 4)) == DMibExpr(pMemory));

							uint8 OldValue = pMemory[Size];
							pMemory[Size] = 0;

							DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
							DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
							DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

							pMemory[Size] = OldValue;
							MemoryManager.f_Free(pMemory, Size);
						};

						DMibTestSuite("Pre block realloc")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten before allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, _Size * 4);

							{
								DMibTestPath("Bigger");

								uint8 OldValue = pMemory[-1];
								pMemory[-1] = 0;
								
								mint NewSizeBigger = _Size * 8;
						
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Realloc(pMemory, NewSizeBigger, Size), OverwriteException);

								pMemory[-1] = OldValue;
								
								pMemory = (uint8 *)MemoryManager.f_Realloc(pMemory, NewSizeBigger, Size);
								Size = NewSizeBigger;
							}

							{
								DMibTestPath("Smaller");

								uint8 OldValue = pMemory[-1];
								pMemory[-1] = 0;
								
								mint NewSizeSmaller = _Size;
						
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Realloc(pMemory, NewSizeSmaller, Size), OverwriteException);

								pMemory[-1] = OldValue;
								
								pMemory = (uint8 *)MemoryManager.f_Realloc(pMemory, NewSizeSmaller, Size);
								Size = NewSizeSmaller ;
							}

							{
								uint8 OldValue = pMemory[-1];
								pMemory[-1] = 0;
								
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

								pMemory[-1] = OldValue;
								MemoryManager.f_Free(pMemory, Size);
							}
						};
						
						DMibTestSuite("Post block realloc")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten after allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, _Size * 4);

							{
								DMibTestPath("Bigger");

								uint8 OldValue = pMemory[Size];
								pMemory[Size] = 0;
								
								mint NewSizeBigger = _Size * 8;
						
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Realloc(pMemory, NewSizeBigger, Size), OverwriteException);

								pMemory[Size] = OldValue;

								pMemory = (uint8 *)MemoryManager.f_Realloc(pMemory, NewSizeBigger, Size);
								
								Size = NewSizeBigger;
							}

							{
								DMibTestPath("Smaller");

								uint8 OldValue = pMemory[Size];
								pMemory[Size] = 0;
								
								mint NewSizeSmaller = _Size;
							
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Realloc(pMemory, NewSizeSmaller, Size), OverwriteException);

								pMemory[Size] = OldValue;

								pMemory = (uint8 *)MemoryManager.f_Realloc(pMemory, NewSizeSmaller, Size);
								
								Size = NewSizeSmaller;
							}
							
							{
								uint8 OldValue = pMemory[Size];
								pMemory[Size] = 0;
								
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

								pMemory[Size] = OldValue;
								MemoryManager.f_Free(pMemory, Size);
							}
						};
						
						DMibTestSuite("Pre block resize")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten before allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, _Size * 4);

							{
								DMibTestPath("Bigger");

								uint8 OldValue = pMemory[-1];
								pMemory[-1] = 0;
								
								mint NewSizeBigger = _Size * 8;
						
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Resize(pMemory, NewSizeBigger, Size), OverwriteException);

								pMemory[-1] = OldValue;
								
								pMemory = (uint8 *)MemoryManager.f_Resize(pMemory, NewSizeBigger, Size);
								Size = NewSizeBigger;
							}

							{
								DMibTestPath("Smaller");

								uint8 OldValue = pMemory[-1];
								pMemory[-1] = 0;
								
								mint NewSizeSmaller = _Size;
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Resize(pMemory, NewSizeSmaller, Size), OverwriteException);

								pMemory[-1] = OldValue;
								
								pMemory = (uint8 *)MemoryManager.f_Resize(pMemory, NewSizeSmaller, Size);
								Size = NewSizeSmaller;
							}

							{
								uint8 OldValue = pMemory[-1];
								pMemory[-1] = 0;
								
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

								pMemory[-1] = OldValue;
								MemoryManager.f_Free(pMemory, Size);
							}
						};
						
						DMibTestSuite("Post block resize")
						{
							TCMemoryManagerDebug<TCMemoryManagerParams<CDefaultMemoryManagerParams_Tests>, true, CDebugOptions> MemoryManager{CMemoryManagerConfig()};
							auto Checkout = MemoryManager.f_CheckoutForce();
							auto OverwriteException = DMibImpExceptionInstance(CExceptionMemoryManagerDebug, "Memory overwritten after allocated block");
							mint Size = _Size;
							uint8 *pMemory = (uint8 *)MemoryManager.f_AllocAlignedWithSize(Size, _Size * 4);

							{
								DMibTestPath("Bigger");

								uint8 OldValue = pMemory[Size];
								pMemory[Size] = 0;
								
								mint NewSizeBigger = _Size * 8;
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Resize(pMemory, NewSizeBigger, Size), OverwriteException);

								pMemory[Size] = OldValue;

								pMemory = (uint8 *)MemoryManager.f_Resize(pMemory, NewSizeBigger, Size);
								
								Size = NewSizeBigger;
							}

							{
								DMibTestPath("Smaller");

								uint8 OldValue = pMemory[Size];
								pMemory[Size] = 0;
								
								mint NewSizeSmaller = _Size;
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Resize(pMemory, NewSizeSmaller, Size), OverwriteException);

								pMemory[Size] = OldValue;

								pMemory = (uint8 *)MemoryManager.f_Resize(pMemory, NewSizeSmaller, Size);
								
								Size = NewSizeSmaller;
							}
							
							{
								uint8 OldValue = pMemory[Size];
								pMemory[Size] = 0;
								
								DMibTest(!DMibExpr(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Unprotect)));
								DMibExpectException(MemoryManager.f_CheckAll(EMemoryManagerCheckFlag_Break), OverwriteException);
								DMibExpectException(MemoryManager.f_Free(pMemory, Size), OverwriteException);

								pMemory[Size] = OldValue;
								MemoryManager.f_Free(pMemory, Size);
							}
						};
					}
				;
				
				fl_TestOverwite(64, "Normal");
				fl_TestOverwite(512*1024, "Big");
#if DMibPPtrBits > 32
				fl_TestOverwite(32*1024*1024, "Huge");
#endif
			};
		}
	};

	DMibTestRegister(CDebug_Tests, Malterlib::Memory::MemoryManager);

}

