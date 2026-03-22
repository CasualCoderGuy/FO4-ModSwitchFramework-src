#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"

namespace MSF_Base
{
	bool SwitchToSelectedAmmo(AmmoData::AmmoMod* selectedAmmo, BGSSoundDescriptorForm* sound);//(void* obj)(void* obj);
	bool SwitchAmmoHotkey(UInt8 key, BGSSoundDescriptorForm* sound, bool ignoreAnim = false, bool requireAmmo = false);
	bool SwitchAmmoCommon(SwitchData* switchData);
	const char* EquipAmmoPipboy(TESAmmo* ammo, bool bEquip);
	bool SwitchToSelectedMod(ModData::Mod* modToAttach, ModData::Mod* modToRemove, BGSSoundDescriptorForm* sound);//(void* modToAttach, void* modToRemove, bool bNeedInit);
	bool ToggleModHotkey(ModData* modData);
	bool SwitchModHotkey(UInt8 key, ModData* modData);
	const char* EquipModPipboy(TESObjectMISC* miscMod, bool bEquip);
	bool HandlePendingAnimations();
	void EndSwitch(UInt16 flag);
	void SwitchFlagsAND(UInt16 flag);
	void SwitchFlagsOR(UInt16 flag);
	bool SwitchMod(SwitchData* switchData, bool updateWidget);
	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modAmmoCount, bool updateAnimGraph, bool playFastEquipAnim);
	bool AttachRemoveModToInventoryStack(TESObjectREFR* owner, BGSInventoryItem* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph, bool playFastEquipAnim);
	bool AttachRemoveModToEquippedItem(Actor* actor, UInt8 slotIndex, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph, bool playFastEquipAnim);
	bool AttachRemoveModToInventoryStackPre(TESObjectREFR* owner, TESBoundObject* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph, bool playFastEquipAnim);
	bool GetInvalidMods(std::vector<BGSMod::Attachment::Mod*>* invalidList, BGSObjectInstanceExtra* mods, TESObjectWEAP* baseWeap, BGSMod::Attachment::Mod* lastmod);
	bool GetAmmoModToModify(BGSObjectInstanceExtra* mods, TESAmmo* targetAmmo, TESObjectWEAP* baseWeap, TESAmmo** finalAmmo, BGSMod::Attachment::Mod** modResult, bool* bAttach);
	BGSMod::Attachment::Mod* GetAmmoModIfInvalid(BGSObjectInstanceExtra* mods, TESObjectWEAP* baseWeap);
	void SpawnRandomMods(TESObjectCELL* cell);
	bool EquipAmmo(BGSInventoryList* invList, TESAmmo* ammo);
	UInt8 IsNotSupportedAmmo(TESAmmo* ammo);
	void PatchActorValues(TESObjectREFR* ref, TESObjectWEAP::InstanceData* newInstance, std::vector<TBO_InstanceData::ValueModifier>* avifValues);
	void GetActorValues(TESObjectWEAP::InstanceData* instance, std::vector<TBO_InstanceData::ValueModifier>* avifValues);

	bool ReloadWeapon(bool full, bool clearAmmoCount = false, bool forced = true, bool isSwitch = true);
	bool DrawWeapon();
	bool PlayAnim(AnimationData* animData);
	bool PlayFeedbackSound(bool play, UInt8 type, BGSSoundDescriptorForm* success);
}

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