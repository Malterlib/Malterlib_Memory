// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#if !defined(DMibConfig_OverrideSystemMalloc) && !defined(DMibMemoryOverrideDll)
void fg_MalterlibMallocOverrideInit()
{
}
void fg_MalterlibMallocOverrideInit_ReinstallHandler()
{
}
void fg_MalterlibMallocOverride_AtExitCalled()
{
}
void fg_MalterlibMallocOverride_CanStartThreads()
{
}
void fg_MalterlibMallocOverride_DestroyThreads()
{
}
void fg_MalterlibMallocOverride_PreDestroyNonTrackedMemoryManager()
{
}
bool fg_MalterlibMallocOverride_Enabled()
{
	return false;
}
void NMib::NSys::fg_Mem_DisableLazyReturnCheckout()
{
}
void NMib::NSys::fg_Mem_EnableLazyReturnCheckout()
{
}
void NMib::NSys::fg_Mem_PrepareFork()
{
}
void NMib::NSys::fg_Mem_ForkedChild()
{
}
void NMib::NSys::fg_Mem_ForkedParent()
{
}
#endif
