#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"

namespace MSF_Base
{
	bool SwitchToSelectedAmmo(AmmoData::AmmoMod* selectedAmmo);//(void* obj)(void* obj);
	bool SwitchAmmoHotkey(UInt8 key);
	const char* EquipAmmoPipboy(TESAmmo* ammo, bool bEquip);
	bool SwitchToSelectedMod(ModData::Mod* modToAttach, ModData::Mod* modToRemove);//(void* modToAttach, void* modToRemove, bool bNeedInit);
	bool ToggleModHotkey(ModData* modData);
	bool SwitchModHotkey(UInt8 key, ModData* modData);
	const char* EquipModPipboy(TESObjectMISC* miscMod, bool bEquip);
	bool HandlePendingAnimations();
	void EndSwitch(UInt16 flag);
	void SwitchFlagsAND(UInt16 flag);
	void SwitchFlagsOR(UInt16 flag);
	bool SwitchMod(SwitchData* switchData, bool updateWidget);
	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modAmmoCount, bool updateAnimGraph);
	bool AttachRemoveModToEquippedItem(Actor* actor, UInt8 slotIndex, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph);
	bool AttachRemoveModToInventoryStackPre(TESObjectREFR* owner, TESBoundObject* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph);
	bool GetInvalidMods(std::vector<BGSMod::Attachment::Mod*>* invalidList, BGSObjectInstanceExtra* mods, TESObjectWEAP* baseWeap, BGSMod::Attachment::Mod* lastmod);
	bool GetAmmoModToModify(BGSObjectInstanceExtra* mods, TESAmmo* targetAmmo, TESObjectWEAP* baseWeap, TESAmmo** finalAmmo, BGSMod::Attachment::Mod** modResult, bool* bAttach);
	BGSMod::Attachment::Mod* GetAmmoModIfInvalid(BGSObjectInstanceExtra* mods, TESObjectWEAP* baseWeap);
	void SpawnRandomMods(TESObjectCELL* cell);
	bool EquipAmmo(BGSInventoryList* invList, TESAmmo* ammo);
	UInt8 IsNotSupportedAmmo(TESAmmo* ammo);

	bool ReloadWeapon(bool full, bool clearAmmoCount = false, bool forced = true, bool isSwitch = true);
	bool DrawWeapon();
	bool PlayAnim(AnimationData* animData);
}

extern const char* modText;
extern const char* itemText;
extern const char* ammoUnequipText;

struct AttachModMessage
{
	Actor* actor;
	BGSMod::Attachment::Mod* mod;
	bool bAttach;
	UInt8 modAmmoCount;
	bool updateAnimGraph;
};

struct DataToAdd
{
	UInt8 op;
	UInt8 target;
	float f;
	UInt32 i;
};