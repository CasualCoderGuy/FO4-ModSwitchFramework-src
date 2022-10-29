#pragma once
#include "Config.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"


namespace ModSwitchFramework
{

	void SpawnRandomMods(TESObjectCELL* cell);

	bool ReloadWeapon(void);
	bool DrawWeapon(void);
	bool FireBurst(Actor* actor);

	bool SwitchAmmoHotkey(UInt8 key);
	bool ToggleModHotkey(UInt8 switchID);
	bool SwitchToSelectedAmmo(void* obj);

	bool InitEquippedWeapon(void);
	bool SwitchMod(void);
	bool AddModDataToInstance(void);
	bool CompareStackModData(Actor* owner);
	bool SeparateInventoryStack(BGSInventoryItem::Stack* startStack, TESForm* baseWeap, VMArray<BGSMod::Attachment::Mod*> mods);
	bool ReplaceModExtraData(ExtraDataList * extraDataList, BGSMod::Attachment::Mod* oldMod, BGSMod::Attachment::Mod* newMod, bool AttachLast = false);
	bool ReevalStackMods(ExtraDataList * extraDataList);
	bool ReevalModdedWeapon(TESObjectWEAP* weapon, BGSMod::Attachment::Mod* mod);
	BGSMod::Attachment::Mod* FindModByUniqueKeyword(ExtraDataList* extraData, BGSKeyword* keyword);
	BGSMod::Attachment::Mod* GetNewModIfAmmoIsInvalid(ExtraDataList* dataList);
	bool UpdateInstanceDataLim(ExtraDataList* dataList);
	bool UpdateInstanceDataWithMod(BGSInventoryItem::Stack* stack, BGSMod::Attachment::Mod* newMod);
	//bool AttachModToStackedItem(BGSInventoryItem::Stack* stack, BGSMod::Attachment::Mod* mod);
}