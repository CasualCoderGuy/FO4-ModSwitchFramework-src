#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"

namespace MSF_Base
{
	bool SwitchToSelectedAmmo(AmmoData::AmmoMod* selectedAmmo);//(void* obj)(void* obj);
	bool SwitchAmmoHotkey(UInt8 key);
	bool SwitchToSelectedMod(ModData::Mod* modToAttach, ModData::Mod* modToRemove, bool bNeedInit);//(void* modToAttach, void* modToRemove, bool bNeedInit);
	bool ToggleModHotkey(ModData* modData);
	bool SwitchModHotkey(UInt8 key, ModData* modData);
	bool HandlePendingAnimations();
	bool InitWeapon();
	void EndSwitch(UInt16 flag);
	void SwitchFlagsAND(UInt16 flag);
	void SwitchFlagsOR(UInt16 flag);
	bool SwitchMod(SwitchData* switchData, bool updateWidget);
	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modAmmoCount, bool updateAnimGraph);
	bool ReevalModdedWeapon(TESObjectWEAP* weapon);
	bool ReevalSwitchedWeapon(Actor* owner, BGSMod::Attachment::Mod* changedMod);
	bool ReevalAttachedMods(Actor* owner, BGSInventoryItem* item, BGSInventoryItem::Stack* stack);
	void SpawnRandomMods(TESObjectCELL* cell);

	bool ReloadWeapon();
	bool DrawWeapon();
	bool PlayAnim(AnimationData* animData);
}

struct DataToAdd
{
	UInt8 op;
	UInt8 target;
	float f;
	UInt32 i;
};