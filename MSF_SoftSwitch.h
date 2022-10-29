#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"

namespace MSF_SoftSwitch
{
	bool AddModDataToInstance(void);
	bool CompareStackModData(Actor* owner);
	bool UpdateInstanceDataWithMod(BGSInventoryItem::Stack* stack, BGSMod::Attachment::Mod* newMod);
}