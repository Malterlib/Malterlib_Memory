// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#ifndef DMibConfig_OverrideSystemMalloc
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
bool fg_MalterlibMallocOverride_Enabled()
{
	return false;
}
#endif
