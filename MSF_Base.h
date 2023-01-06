#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"

namespace MSF_Base
{
	bool SwitchToSelectedAmmo(void* obj, bool bAttach);
	bool SwitchAmmoHotkey(UInt8 key);
	bool SwitchToSelectedMod(void* modToAttach, void* modToRemove);
	bool ToggleModHotkey(std::vector<ModAssociationData*>* modAssociations);
	bool SwitchModHotkey(UInt8 key, std::vector<ModAssociationData*>* modAssociations);
	bool HandlePendingAnimations(bool bDoQueueSwitch = false);
	bool InitWeapon();
	bool SwitchMod();
	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modAmmoCount, bool updateAnimGraph);
	bool ReevalModdedWeapon(TESObjectWEAP* weapon);
	bool ReevalSwitchedWeapon(Actor* owner, BGSMod::Attachment::Mod* changedMod);
	void SpawnRandomMods(TESObjectCELL* cell);

	bool ReloadWeapon();
	bool DrawWeapon();
	bool PlayAnim();
	bool FireBurst(Actor* actor);
	void BurstTest(BurstMode* data);
}

struct DataToAdd
{
	UInt8 op;
	UInt8 target;
	float f;
	UInt32 i;
};