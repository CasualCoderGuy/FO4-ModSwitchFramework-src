#include "Globals.h"
#include "f4se_common/Relocation.h"
#include "f4se_common/f4se_version.h"

#define GET_RVA(relocPtr) relocPtr.GetUIntPtr() - RelocationManager::s_baseAddr

/*
This file makes globals version-independent.

Initialization order is important for this file.

Since RelocPtrs are static globals with constructors they are initialized during the dynamic initialization phase.
Static initialization order is undefined for variables in different translation units, so we can't obtain the value of a RelocPtr during static init.

Initialization must thus be done explicitly:
Call G::Init() in the plugin load routine before calling RVAManager::UpdateAddresses().

Doing so ensures that all RelocPtrs have been initialized and can be used to initialize an RVA.
*/

#include "Config.h"
#include "f4se/GameData.h"

namespace Globals
{
    RVA<DataHandler*> dataHandler;

    void Init()
    {
        dataHandler = RVA<DataHandler*>(GET_RVA(g_dataHandler), "48 8B 05 ? ? ? ? 8B 13", 0, 3, 7);
    }
}

