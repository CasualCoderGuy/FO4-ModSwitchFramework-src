#include "MSF_Base.h"
#include "MSF_Events.h"
#include "MSF_WeaponState.h"

const char* modText = "This mod can only be attached at a workbench.";
const char* itemText = "You cannot equip this item.";
const char* unsupportedAmmoText = "You cannot equip non-playable ammo or fusion cores.";
const char* noAmmoTypeText = "You cannot equip this ammo type.";
const char* ammoEquipFailedText = "You cannot equip this ammo right now.";
const char* ammoUnequipText = "You cannot unequip ammo.";
const char* ammoEquippedText = "Ammo equipped.";

namespace MSF_Base
{
	//========================== Main Functions ===========================

	//FROM SCALEFORM:
	bool SwitchToSelectedAmmo(AmmoData::AmmoMod* selectedAmmo)//(void* obj)
	{
		_DEBUG("selectCalled");
		//AmmoData::AmmoMod* selectedAmmo = reinterpret_cast<AmmoData::AmmoMod*>(obj);

		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled) )// || (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}
		Actor* playerActor = *g_player;

		_DEBUG("checksPassed");
		SwitchData* switchData = new SwitchData();
		BGSMod::Attachment::Mod* mod = selectedAmmo->mod;
		switchData->targetAmmo = selectedAmmo->ammo;
		_DEBUG("mod: %p", mod);
		if (mod)
			switchData->ModToAttach = mod;
		else
		{
			BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
			mod = Utilities::GetModAtAttachPoint(modData, MSF_MainData::ammoAP);
			_DEBUG("modatAP: %p", mod);
			if (mod)
				switchData->ModToRemove = mod;
			else
				return false;
		}

		_DEBUG("modOK");

		if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))
		{
			switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
			MSF_Base::SwitchMod(switchData, true);
			//if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
			//	Utilities::DrawWeapon(*g_player);
			return true;
		}

		if (MSF_MainData::modSwitchManager.GetQueueCount() > 0 || MSF_MainData::modSwitchManager.GetState() != 0)
			return false;

		TESObjectWEAP::InstanceData* instance = Utilities::GetEquippedInstanceData(playerActor);
		bool isBCR = MSF_Data::InstanceHasBCRSupport(instance);
		bool isTR = MSF_Data::InstanceHasTRSupport(instance);
		if (isBCR)
		{
			switchData->SwitchFlags |= SwitchData::bReloadZeroCount;
			UInt32 targetAmmoCount = 0;
			UInt32 currentAmmoCount = 0;
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch))
				targetAmmoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, switchData->targetAmmo);
			if (targetAmmoCount)
			{
				ExtraWeaponState* ws = MSF_MainData::weaponStateStore.GetEquipped(*g_player);
				currentAmmoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, ws->GetCurrentAmmo());
			}
			if (!targetAmmoCount || !currentAmmoCount)
			{
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				MSF_Base::SwitchMod(switchData, true);
				if (targetAmmoCount && (*g_player)->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
					MSF_Base::ReloadWeapon(false, true);
				return true;
			}
		}
		else if (isTR)
			switchData->SwitchFlags |= SwitchData::bReloadFull;

		if ((*g_player)->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
		{
			if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadCompatibilityMode)
			{
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				MSF_Base::SwitchMod(switchData, true);
				MSF_Base::ReloadWeapon(isTR && !isBCR, isBCR);
				return true;
			}
			_DEBUG("toReload");
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); // | SwitchData::bReloadNotFinished
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			UInt64 invammo = Utilities::GetInventoryItemCount(playerActor->inventoryList, switchData->targetAmmo);
			Utilities::SetAnimationVariableInt(playerActor, "NextReloadAmmoCount", invammo > instance->ammoCapacity ? instance->ammoCapacity : invammo);
			if (!MSF_Base::ReloadWeapon(switchData->SwitchFlags & SwitchData::bReloadFull, switchData->SwitchFlags & SwitchData::bReloadZeroCount))
				MSF_MainData::modSwitchManager.ClearQueue();
				//MSF_Base::SwitchMod(switchData, true);
			return true;
		}
		else if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
		{
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			Utilities::DrawWeapon(*g_player);
			//delay check draw state
			return true;
		}
		delete switchData;
		return false;
	}

	//FROM HOTKEY:
	bool SwitchAmmoHotkey(UInt8 key)
	{
		Actor* playerActor = *g_player;
		_DEBUG("queueCount: %i; state: %02X; unk08: %08X; flags: %08X", MSF_MainData::modSwitchManager.GetQueueCount(), MSF_MainData::modSwitchManager.GetState(), playerActor->actorState.unk08, playerActor->actorState.flags);

		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))// || (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}
		_DEBUG("animOK");

		SwitchData* switchData = MSF_Data::GetNthAmmoMod(key);
		if (!switchData)
			return false;
		_DEBUG("ammoSwitchDataOK");

		if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))
		{
			switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
			MSF_Base::SwitchMod(switchData, true);
			//if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
			//	Utilities::DrawWeapon(*g_player);
			return true;
		}

		if (MSF_MainData::modSwitchManager.GetQueueCount() > 0 || MSF_MainData::modSwitchManager.GetState() != 0)
			return false;
		_DEBUG("queue/stateOK");

		TESObjectWEAP::InstanceData* instance = Utilities::GetEquippedInstanceData(playerActor);
		bool isBCR = MSF_Data::InstanceHasBCRSupport(instance);
		bool isTR = MSF_Data::InstanceHasTRSupport(instance);
		if (isBCR)
		{
			switchData->SwitchFlags |= SwitchData::bReloadZeroCount;
			UInt32 targetAmmoCount = 1;
			UInt32 currentAmmoCount = 0;
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch))
				targetAmmoCount = Utilities::GetInventoryItemCount(playerActor->inventoryList, switchData->targetAmmo);
			if (targetAmmoCount)
			{
				ExtraWeaponState* ws = MSF_MainData::weaponStateStore.GetEquipped(playerActor);
				currentAmmoCount = Utilities::GetInventoryItemCount((playerActor)->inventoryList, ws->GetCurrentAmmo());
			}
			_DEBUG("toReload BCR");
			if (!targetAmmoCount || !currentAmmoCount)
			{
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				MSF_Base::SwitchMod(switchData, true);
				if (targetAmmoCount && playerActor->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
					MSF_Base::ReloadWeapon(false, true);
				return true;
			}
		}
		else if (isTR)
			switchData->SwitchFlags |= SwitchData::bReloadFull;

		if (playerActor->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
		{
			if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadCompatibilityMode)
			{
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				MSF_Base::SwitchMod(switchData, true);
				MSF_Base::ReloadWeapon(isTR && !isBCR, isBCR);
				return true;
			}
			_DEBUG("toReload");
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); // | SwitchData::bReloadNotFinished
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			UInt64 invammo = Utilities::GetInventoryItemCount(playerActor->inventoryList, switchData->targetAmmo);
			Utilities::SetAnimationVariableInt(playerActor, "NextReloadAmmoCount", invammo > instance->ammoCapacity ? instance->ammoCapacity : invammo);
			if (!MSF_Base::ReloadWeapon(switchData->SwitchFlags & SwitchData::bReloadFull, switchData->SwitchFlags & SwitchData::bReloadZeroCount))
				MSF_MainData::modSwitchManager.ClearQueue();
			//MSF_Base::SwitchMod(switchData, true);
			return true;
		}
		else if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
		{
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			Utilities::DrawWeapon(playerActor);
			//delay check draw state
			return true;
		}
		delete switchData;
		return false;
	}

	const char* EquipAmmoPipboy(TESAmmo* ammo, bool bEquip)
	{
		if (!bEquip)
			return ammoUnequipText;
		if (MSF_Base::IsNotSupportedAmmo(ammo))
			return unsupportedAmmoText;
		SwitchData* switchData = MSF_Data::GetModForAmmo(ammo);
		if (!switchData)
			return noAmmoTypeText;
		switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
		if (!MSF_Base::SwitchMod(switchData, true))
			return ammoEquipFailedText;
		ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeReload, *g_player, ExtraWeaponState::bEventTypeReloadInventory, 0);
		MSF_Base::EquipAmmo((*g_player)->inventoryList, ammo);
		PlayEquipSound(*g_player, ammo, 1, 0);
		return nullptr;
	}

	//FROM SCALEFORM:
	bool SwitchToSelectedMod(ModData::Mod* modToAttach, ModData::Mod* modToRemove)//(void* modDataToAttach, void* modDataToRemove, bool bNeedInit)
	{
		if (!modToAttach && !modToRemove)
			return false;

		//ModData::Mod* modToAttach = reinterpret_cast<ModData::Mod*>(modDataToAttach);
		//ModData::Mod* modToRemove = reinterpret_cast<ModData::Mod*>(modDataToRemove);

		//if (!MSF_Data::CheckSwitchRequirements(stack, modToAttach, modToRemove))
		//	return false;

		return MSF_Data::QueueModsToSwitch(modToAttach, modToRemove);
	}


	//FROM HOTKEY:
	bool ToggleModHotkey(ModData* modData)
	{
		//BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(*g_player, 41);
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedWeaponStack(*g_player);
		return MSF_Data::GetNextMod(eqStack, modData);
	}

	bool SwitchModHotkey(UInt8 key, ModData* modData)
	{
		//BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(*g_player, 41);
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedWeaponStack(*g_player);
		return MSF_Data::GetNthMod(key, eqStack, modData);
	}

	const char* EquipModPipboy(TESObjectMISC* miscMod, bool bEquip)
	{
		return modText;
		//unordered_map MISC, ?
		//attachParent
		//modAssoc
		//playsound
		return nullptr;
	}

	bool HandlePendingAnimations()
	{
		Actor* playerActor = *g_player;
		SInt32 state = (*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState);
		UInt32 weaponActivity = playerActor->actorState.flags & ActorStateFlags0C::mWeaponActivityMask;
		//bool is3rdPersonModelShown : 1;    //player:: DFE:4
		MiddleHighProcessData* highProc = (MiddleHighProcessData*)playerActor->middleProcess->unk08;
		TESIdleForm* lastAnim = highProc->lastIdlePlayed;
		TESIdleForm* currAnim = highProc->currentIdle;
		_DEBUG("currAnim: %p, lastAnim: %p", currAnim, lastAnim);               
		_DEBUG("ActorState: %p %08X %08X %08X", playerActor, playerActor->actorState.unk08, playerActor->actorState.flags, state);
		if (state != 0 && state != 8)
			return false;
		if ((playerActor->actorState.unk08 & (ActorStateFlags08::kActorState_Bashing)) || // | ActorStateFlags08::kActorState_Sprint
			(playerActor->actorState.flags & (ActorStateFlags0C::kActorState_FurnitureState | ActorStateFlags0C::kWeaponState_Sheathing)) ||
			(weaponActivity == ActorStateFlags0C::kWeaponState_Reloading || weaponActivity == ActorStateFlags0C::kWeaponState_Firing) ||
			(!(playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Drawn) && (playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Draw)))
		{
			return false;
		}
		return true;
	}

	void EndSwitch(UInt16 flag)
	{
		MSF_MainData::modSwitchManager.SetState(MSF_MainData::modSwitchManager.GetState() & flag);
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData && !(switchData->SwitchFlags & SwitchData::bSwitchingInProgress))
		{
			if (switchData->SwitchFlags & SwitchData::bIgnoreAnimations)
				MSF_Base::SwitchMod(switchData, true);
			else if ((*g_player)->actorState.IsWeaponDrawn() || (switchData->SwitchFlags & ModData::Mod::bNotRequireWeaponToBeDrawn))
			{
				if (switchData->SwitchFlags & SwitchData::bReloadNeeded)
				{
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadCompatibilityMode)
					{
						switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
						MSF_Base::SwitchMod(switchData, true);
					}
					else
						switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress | ~SwitchData::bReloadNeeded); // | SwitchData::bReloadNotFinished
					if (!MSF_Base::ReloadWeapon(switchData->SwitchFlags & SwitchData::bReloadFull, switchData->SwitchFlags & SwitchData::bReloadZeroCount))
						MSF_MainData::modSwitchManager.ClearQueue();
				}
				else if (switchData->SwitchFlags & SwitchData::bAnimNeeded)
				{
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimCompatibilityMode)
					{
						switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
						MSF_Base::SwitchMod(switchData, true);
					}
					else
						switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress | ~SwitchData::bAnimNeeded);
					if (!MSF_Base::PlayAnim(switchData->animData))
						MSF_Base::SwitchMod(switchData, true);
				}
			}
			else if (switchData->SwitchFlags & SwitchData::bDrawEnabled)
			{
				switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress);
				Utilities::DrawWeapon(*g_player);
				//delay check draw state
			}
			//bFinishWithoutAnim
		}
	}

	void SwitchFlagsAND(UInt16 flag)
	{
		MSF_MainData::modSwitchManager.SetState(MSF_MainData::modSwitchManager.GetState() & flag);
	}

	void SwitchFlagsOR(UInt16 flag)
	{
		MSF_MainData::modSwitchManager.SetState(MSF_MainData::modSwitchManager.GetState() | flag);
	}

	bool SwitchMod(SwitchData* switchData, bool updateWidget)
	{
		if (!switchData)
			return false;
		_DEBUG("SwitchMod; flags: %02X ; Attach: %08X ; Remove: %08X", switchData->SwitchFlags, switchData->ModToAttach, switchData->ModToRemove);
		Actor* playerActor = *g_player;

		BGSMod::Attachment::Mod* modToAttach = switchData->ModToAttach;
		BGSMod::Attachment::Mod* modToRemove = switchData->ModToRemove;
		TESObjectMISC* looseModToAdd = switchData->LooseModToAdd;
		TESObjectMISC* looseModToRemove = switchData->LooseModToRemove;

		UInt16 flags = switchData->SwitchFlags;

		if (modToRemove)
		{
			//if (!Utilities::HasObjectMod(modData, mod))
			//	ClearSwitchFlags();
			bool bUpdateAnimGraph = false;
			if (!modToAttach && (flags & SwitchData::bUpdateAnimGraph))
				bUpdateAnimGraph = true;
			if (!AttachModToEquippedWeapon(playerActor, modToRemove, false, 2, bUpdateAnimGraph))
			{
				//_DEBUG("finishing1");
				MSF_MainData::modSwitchManager.FinishSwitch(switchData);
				return false;
			}
			//ReevalSwitchedWeapon(playerActor, modToRemove);
			if (looseModToAdd)
				Utilities::AddItem(playerActor, looseModToAdd, 1, true);
		}
		if (modToAttach)
		{
			//if (Utilities::HasObjectMod(modData, mod))
			//	ClearSwitchFlags();
			if (!AttachModToEquippedWeapon(playerActor, modToAttach, true, 2, (flags & SwitchData::bUpdateAnimGraph) != 0))
			{
				//_DEBUG("finishing2");
				MSF_MainData::modSwitchManager.FinishSwitch(switchData);
				return false;
			}
			if (looseModToRemove)
				Utilities::RemoveItem(playerActor, looseModToRemove, 1, true);
		}
		_DEBUG("finishing3");
		MSF_MainData::modSwitchManager.FinishSwitch(switchData);
		_DEBUG("finishOK");
		if (updateWidget)
			MSF_Scaleform::UpdateWidgetData();
		_DEBUG("updateOK");
		return true;
	}

	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modLoadedAmmoCount, bool updateAnimGraph)
	{
		_DEBUG("attach, updAnim: %02X", updateAnimGraph);
		//BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(actor, 41);
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedWeaponStack(actor);
		if (!stack)
			return false;
		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return false;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* modList = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;
		if (!modList)
			return false;
		BGSInventoryItem* item = nullptr;
		for (UInt32 i = 0; i < actor->inventoryList->items.count; i++)
		{
			if (actor->inventoryList->items.entries[i].form == (TESForm*)weapBase)
				item = &actor->inventoryList->items.entries[i];
		}
		if (!item)
			return false;

		_DEBUG("AcheckOK");

		bool ret = false;

		CheckStackIDFunctor IDfunctor(Utilities::GetStackID(item, stack));
		ModifyModDataFunctor modFunctor(mod, 0, bAttach , &ret);

		UInt32 unk = 0x00200000;
		AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);

		//ExtraDataList* newList = stack->extraData;
		BSExtraData* newExtraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* newModList = DYNAMIC_CAST(newExtraMods, BSExtraData, BGSObjectInstanceExtra);

		std::vector<BGSMod::Attachment::Mod*> invalidMods;
		MSF_Base::GetInvalidMods(&invalidMods, newModList, weapBase, mod);
		for (auto itMods = invalidMods.begin(); itMods != invalidMods.end(); itMods++)
		{
			BGSMod::Attachment::Mod* invalidMod = *itMods;
			_DEBUG("invalid mod: %08X", invalidMod->formID);
			unk = 0x00200000;
			bool success = false;
			ModifyModDataFunctor modifyModFunctor(invalidMod, 0, false, &success);
			//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
		}

		//BGSMod::Attachment::Mod* targetAmmoMod = nullptr;
		//bool toAttach = false;
		//TESAmmo* targetAmmo = nullptr;
		//ExtraWeaponState* weaponState = ExtraWeaponState::Init(dataList, nullptr);
		//if (weaponState)
		//{
		//	weaponState->UpdateWeaponStates(dataList, nullptr, instanceData->ammo);
		//	targetAmmo = weaponState->GetCurrentAmmo();
		//}
		//else if (instanceData)
		//	targetAmmo = instanceData->ammo;

		////BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(newModList, weapBase);
		//MSF_Base::GetAmmoModToModify(newModList, targetAmmo, weapBase, &targetAmmoMod, &toAttach);
		//if (targetAmmoMod)
		//{
		//	_DEBUG("target ammomod: %08X, rem: %02X", targetAmmoMod->formID, toAttach);
		//	unk = 0x00200000;
		//	bool success = false;
		//	ModifyModDataFunctor modifyModFunctor(targetAmmoMod, 0, toAttach, &success);
		//	//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
		//	AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
		//	extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		//	instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		//}

		std::vector<std::pair<BGSMod::Attachment::Mod*, bool>> stateModsToModify;
		ExtraWeaponState::HandleModChangeEvent(dataList, &stateModsToModify, ExtraWeaponState::kEventTypeModdedSwitch);
		for (auto nextPair : stateModsToModify)
		{
			_DEBUG("target state/ammomod: %08X, rem: %02X", nextPair.first->formID, nextPair.second);

			unk = 0x00200000;
			bool success = false;
			ModifyModDataFunctor modifyModFunctor(nextPair.first, 0, nextPair.second, &success);
			//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
		}

		extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);

		BGSObjectInstance idStruct;
		idStruct.object = weapBase;
		idStruct.instanceData = instanceData;

		//_DEBUG("mod ammo count1: %i", eqData->loadedAmmoCount);
		MSF_MainData::modSwitchManager.SetModChangeEvent(true);
		MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(true);
		EquipItemInternal(g_ActorEquipManager, actor, idStruct, 0, 1, nullptr, 0, 0, 0, 1, 0);
		//actor->middleProcess->unk08->unk290[1] &= 0xFFFFFFFF00000000;//0xFF00000000000000;
		//UpdateMiddleProcess(actor->middleProcess, actor, idStruct, newInstanceData->equipSlot);
		//actor->middleProcess->unk08->unk290[1] = actor->middleProcess->unk08->unk290[1] & 0xFFFFFFFF00000000 | 0x1;

		//UpdateEquippedWeaponData(actor->middleProcess->unk08->equipData[0].equippedData);

		//unkEquipSlotStruct equipSlotStruct;
		//equipSlotStruct.unk00 = 0;
		////equipSlotStruct.unk04 = 1;
		//equipSlotStruct.equipSlot = newInstanceData->equipSlot;
		//equipSlotStruct.unk10 = 0;
		//equipSlotStruct.unk18 = 0x10000;
		//newInstanceData->equipSlot->unk38 &= ~0b100;
		//MainEquipHandler(g_ActorEquipManager, actor, idStruct, equipSlotStruct);
		//UniversalEquipHandler(actor, idStruct, equipSlotStruct);

		//volatile long long* lockcnt = (volatile long long*)&actor->equipData->unk00;
		//InterlockedIncrement64(lockcnt);
		//UpdateEquipData(actor->equipData, idStruct, nullptr);
		//InterlockedDecrement64(lockcnt);
		//if (actor == *g_player)
		//{
		//	lockcnt = (volatile long long*)&(*g_player)->playerEquipData->unk00;
		//	InterlockedIncrement64(lockcnt);
		//	UpdateEquipData((*g_player)->playerEquipData, idStruct, nullptr);
		//	InterlockedDecrement64(lockcnt);
		//}
		
		//InterlockedIncrement(&newList->refCount);
		//NiStuff(*g_player, weapBase, &newList, 0, false, 0);

		//UpdateEnchantments(actor, idStruct, newList);
		//UInt8 unk1 = 1;
		//ActorStruct actorStruct;
		//actorStruct.actor = actor;
		//actorStruct.unk08 = &unk1;
		//UpdateAVModifiers(actorStruct, newInstanceData->modifiers);
		//
		//float reloadSpeed = *g_reloadSpeedMultiplier * newInstanceData->reloadSpeed;
		//UpdateAnimValueFloat(&actor->animGraphHolder, g_reloadSpeedAnimValueHolder, reloadSpeed);
		//if (newInstanceData->firingData)
		//	UpdateAnimValueFloat(&actor->animGraphHolder, g_sightedTransitionAnimValueHolder, newInstanceData->firingData->sightedTransition);

		//unkEquipSlotStruct equipSlotStruct;
		//equipSlotStruct.unk00 = 0;
		//equipSlotStruct.equipSlot = newInstanceData->equipSlot;
		//equipSlotStruct.unk10 = 0;
		//equipSlotStruct.unk18 = 0x10000;
		//UnkSub_EFF9D0(actor);
		//EquipHandler(unk_05AB38D0, actor, idStruct, equipSlotStruct);
		if (updateAnimGraph)
			UpdateAnimGraph(actor, false);
		//UnkSub_DFE930(actor, false);

		for (UInt32 i = 0; i < actor->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			actor->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != weapBase || !inventoryItem.stack)
				continue;
			bool bEquipped = false;
			for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
			{
				if (stack->flags & 1)
				{
					if (bEquipped)
					{
						volatile short* equipFlagPtr = (volatile short*)&stack->flags;
						InterlockedExchange16(equipFlagPtr, 0);
					}
					bEquipped = true;
				}
			}
		}

		EquipWeaponData* newEqData = (EquipWeaponData*)actor->middleProcess->unk08->equipData[0].equippedData;
		TESAmmo* newAmmoType = newEqData->ammo;
		_DEBUG("mod ammo count2: %i", newEqData->loadedAmmoCount);
		ExtraWeaponState* weaponState = ExtraWeaponState::Init(dataList, nullptr);
		if (weaponState)
			weaponState->HandleEquipEvent(dataList, newEqData);
		//if (modLoadedAmmoCount == 0 || (modLoadedAmmoCount == 2 && newAmmoType == ammoType)) //BCR!!
		//	newEqData->loadedAmmoCount = loadedAmmoCount;
		//else if (modLoadedAmmoCount == 1 || (modLoadedAmmoCount == 2 && newAmmoType != ammoType))
		//{
		//	UInt64 ammoCount = Utilities::GetInventoryItemCount(actor->inventoryList, newAmmoType);
		//	if (ammoCount < newInstanceData->ammoCapacity)
		//		newEqData->loadedAmmoCount = ammoCount;
		//	else
		//		newEqData->loadedAmmoCount = newInstanceData->ammoCapacity;
		//}

		//if (MSF_MainData::activeBurstManager)
		//	MSF_MainData::activeBurstManager->flags &= ~BurstModeData::bActive;
		//if (Utilities::WeaponInstanceHasKeyword(newInstanceData, MSF_MainData::FiringModBurstKW))
		//{
		//	BGSMod::Attachment::Mod* mod = Utilities::FindModByUniqueKeyword(newModList, MSF_MainData::FiringModBurstKW);
		//	if (mod)
		//	{
		//		auto it = MSF_MainData::burstModeData.find(mod);
		//		if (it != MSF_MainData::burstModeData.end());
		//		{
		//			BurstModeData* burstMode = it->second;

		//			if (MSF_MainData::activeBurstManager)
		//				delete MSF_MainData::activeBurstManager;
		//			MSF_MainData::activeBurstManager = new BurstModeManager(burstMode, true);
		//			//MSF_MainData::activeBurstManager->HandleEquipEvent(newInstanceData);
		//		}
		//	}
		//}

		return true;
	}

	bool AttachRemoveModToInventoryStack(TESObjectREFR* owner, BGSInventoryItem* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph)
	{
		if (!owner || !item || !mod || !item->form || !item->stack)
			return false;
		BGSInventoryItem::Stack* stack = Utilities::GetStack(item, stackID);
		if (!stack)
			return false;

		Actor* ownerActor = nullptr;
		bool isEquipped = false;
		if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
		{
			isEquipped = true;
			ownerActor = DYNAMIC_CAST(owner, TESObjectREFR, Actor);
		}

		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return false;
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData || !extraInstanceData->baseForm || !extraInstanceData->instanceData)
			return false;

		bool ret = false;

		CheckStackIDFunctor IDfunctor(stackID);
		ModifyModDataFunctor modFunctor(mod, 0, bAttach, &ret);
		modFunctor.shouldSplitStacks = shouldSplitStacks;
		modFunctor.transferEquippedToSplitStack = transferEquippedToSplitStack;

		UInt32 unk = 0x00200000;
		AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);

		extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData || !extraInstanceData->baseForm || !extraInstanceData->instanceData)
			return false;

		TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
		if (baseWeap)
		{
			BSExtraData* newExtraMods = dataList->GetByType(kExtraData_ObjectInstance);
			BGSObjectInstanceExtra* newModList = DYNAMIC_CAST(newExtraMods, BSExtraData, BGSObjectInstanceExtra);
			std::vector<BGSMod::Attachment::Mod*> invalidMods;
			MSF_Base::GetInvalidMods(&invalidMods, newModList, baseWeap, mod);
			for (auto itMods = invalidMods.begin(); itMods != invalidMods.end(); itMods++)
			{
				BGSMod::Attachment::Mod* invalidMod = *itMods;
				_DEBUG("invalid mod: %08X", invalidMod->formID);
				unk = 0x00200000;
				bool success = false;
				ModifyModDataFunctor modifyModFunctor(invalidMod, 0, false, &success);
				//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
				AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
			}
			std::vector<std::pair<BGSMod::Attachment::Mod*, bool>> stateModsToModify;
			ExtraWeaponState::HandleModChangeEvent(dataList, &stateModsToModify, ExtraWeaponState::kEventTypeModdedSwitch);
			for (auto nextPair : stateModsToModify)
			{
				unk = 0x00200000;
				bool success = false;
				ModifyModDataFunctor modifyModFunctor(nextPair.first, 0, nextPair.second, &success);
				//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
				AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
			}
		}

		if (isEquipped && ownerActor)
		{
			BGSObjectInstance idStruct;
			idStruct.object = extraInstanceData->baseForm;
			idStruct.instanceData = extraInstanceData->instanceData;

			UInt8 slot = Utilities::GetEquippedWeaponSlotIndex(ownerActor);
			if (baseWeap && slot != BIPOBJECT::kNone)
			{
				MSF_MainData::modSwitchManager.SetModChangeEvent(true);
				MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(true);
				EquipItemInternal(g_ActorEquipManager, ownerActor, idStruct, 0, 1, nullptr, 0, 0, 0, 1, 0);

				if (updateAnimGraph)
					UpdateAnimGraph(ownerActor, false);

				EquipWeaponData* newEqData = (EquipWeaponData*)ownerActor->middleProcess->unk08->equipData[0].equippedData;
				ExtraWeaponState* weaponState = ExtraWeaponState::Init(dataList, nullptr);
				if (weaponState)
					weaponState->HandleEquipEvent(dataList, newEqData);
			}
			else
			{
				UnequipItemInternal(g_ActorEquipManager, ownerActor, idStruct, 1, nullptr, -1, 0, 0, 0, 0, nullptr);
				EquipItemInternal(g_ActorEquipManager, ownerActor, idStruct, stackID, 1, nullptr, 1, 0, 0, 0, 0);
			}

			//?
			for (UInt32 i = 0; i < ownerActor->inventoryList->items.count; i++)
			{
				BGSInventoryItem inventoryItem;
				owner->inventoryList->items.GetNthItem(i, inventoryItem);
				if (inventoryItem.form != extraInstanceData->baseForm || !inventoryItem.stack)
					continue;
				bool bEquipped = false;
				for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
				{
					if (stack->flags & 1)
					{
						if (bEquipped)
						{
							volatile short* equipFlagPtr = (volatile short*)&stack->flags;
							InterlockedExchange16(equipFlagPtr, 0);
						}
						bEquipped = true;
					}
				}
			}
		}

		return true;
	}

	bool AttachRemoveModToEquippedItem(Actor* actor, UInt8 slotIndex, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph)
	{
		if (slotIndex >= BIPOBJECT::BIPED_OBJECT::kTotal)
			return false;
		if (!actor)
			return false;
		if (!actor->biped.get())
			return false;
		auto item = actor->biped.get()->object[slotIndex].parent.object;
		if (!item)
			return false;
		if (!actor->inventoryList)
			return false;
		for (UInt32 i = 0; i < actor->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			actor->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			UInt32 stackID = 0;
			for (BGSInventoryItem::Stack* currStack = inventoryItem.stack; currStack; currStack = currStack->next)
			{
				if (currStack->flags & BGSInventoryItem::Stack::kFlagEquipped)
					return AttachRemoveModToInventoryStack(actor, &inventoryItem, stackID, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph);
				stackID++;
			}
		}
		return false;
	}

	bool AttachRemoveModToInventoryStackPre(TESObjectREFR* owner, TESBoundObject* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph)
	{
		if (!item || !owner || !mod)
			return false;
		if (!owner->inventoryList)
			return false;
		BGSInventoryItem inventoryItem;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form == item && inventoryItem.stack)
				return AttachRemoveModToInventoryStack(owner, &inventoryItem, stackID, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph);
		}
		return false;
	}

	bool GetInvalidMods(std::vector<BGSMod::Attachment::Mod*>* invalidList, BGSObjectInstanceExtra* mods, TESObjectWEAP* baseWeap, BGSMod::Attachment::Mod* lastmod)
	{
		if (!invalidList || !mods || !baseWeap)
			return false;
		auto data = mods->data;
		if (!data || !data->forms)
			return false;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			//_DEBUG("checkInvalid: %p", objectMod);
			if (objectMod && MSF_MainData::instantiationRequirements.count(objectMod) > 0)
			{
				//_DEBUG("instOK");
				auto range = MSF_MainData::instantiationRequirements.equal_range(objectMod);
				bool bRemove = false;
				//std::vector<BGSMod::Attachment::Mod*> parentMods;
				//Utilities::GetParentMods(mods, objectMod, &parentMods);
				BGSMod::Attachment::Mod* parentMod = Utilities::GetParentMod(mods, objectMod);
				for (auto it = range.first; it != range.second; ++it)
				{
					/*_DEBUG("range");
					bRemove = true;
					for (std::vector<BGSMod::Attachment::Mod*>::iterator itMod = parentMods.begin(); itMod != parentMods.end(); itMod++)
					{
						//_DEBUG("parent");
						KeywordValueArray* instantiationData = reinterpret_cast<KeywordValueArray*>(&(*itMod)->unkB0);
						if (instantiationData->GetItemIndex(it->second) >= 0)
						{
							//_DEBUG("parentOK");
							bRemove = false;
							break;
						}
					}
					if (bRemove == false)
						break;
					*/
					KeywordValueArray* instantiationData = reinterpret_cast<KeywordValueArray*>(&parentMod->unkB0);
					if (instantiationData->GetItemIndex(it->second) >= 0)
					{
						//_DEBUG("parentOK");
						bRemove = false;
						break;
					}
				}
				if (bRemove)
					invalidList->push_back(objectMod);
			}
		}
		return true;
	}

	bool GetAmmoModToModify(BGSObjectInstanceExtra* mods, TESAmmo* targetAmmo, TESObjectWEAP* baseWeap, TESAmmo** finalAmmo, BGSMod::Attachment::Mod** modResult, bool* bAttach)
	{
		if (!targetAmmo || !modResult || !bAttach || !finalAmmo)
			return false;
		*modResult = nullptr;
		*bAttach = false;
		*finalAmmo = targetAmmo;
		BGSMod::Attachment::Mod* ammoMod = Utilities::GetModAtAttachPoint(mods, MSF_MainData::ammoAP);
		BGSMod::Attachment::Mod* targetMod = nullptr;
		auto itAmmo = MSF_MainData::ammoMap.find(targetAmmo);
		if (itAmmo != MSF_MainData::ammoMap.end())
			targetMod = itAmmo->second->mod;
		if (!ammoMod && !targetMod)
			return true;
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(mods, baseWeap);
		_DEBUG("targetAmmo: %08X", targetAmmo->formID);
		if (baseAmmo)
			_DEBUG("baseAmmo: %08X", baseAmmo->formID);
		if (ammoMod)
			_DEBUG("ammoMod: %08X", ammoMod->formID);
		if (targetMod)
			_DEBUG("targetMod: %08X", targetMod->formID);
		if (!baseAmmo)
		{
			_DEBUG("FindInvalid: no supported base ammo for instance");  //new: nullptr
			*modResult = ammoMod;
			*finalAmmo = nullptr;
			return true;
		}
		if (!targetMod && ammoMod)
		{
			_DEBUG("FindInvalid: no AmmoMod for target ammo"); //new: basecal
			*modResult = ammoMod;
			*finalAmmo = baseAmmo;
			return true;
		}
		auto itBaseAmmo = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itBaseAmmo != MSF_MainData::ammoDataMap.end())
		{
			auto ammoData = (*itBaseAmmo).second;
			//target mod == baseMod?
			auto itTargetMod = std::find_if(ammoData->ammoMods.begin(), ammoData->ammoMods.end(), [targetMod](AmmoData::AmmoMod& data) {
				return data.mod == targetMod;
				});
			if (itTargetMod != ammoData->ammoMods.end())
			{
				_DEBUG("FindInvalid: new base ammo, restore switched ammo");  //new: targetAmmo (state change)
				*modResult = targetMod;
				*finalAmmo = itTargetMod->ammo;
				*bAttach = true;
				return true;
			}
			auto itCurrentMod = std::find_if(ammoData->ammoMods.begin(), ammoData->ammoMods.end(), [ammoMod](AmmoData::AmmoMod& data) {
				return data.mod == ammoMod;
				});
			if (itCurrentMod != ammoData->ammoMods.end())
			{
				_DEBUG("FindInvalid: no invalid ammo found"); //nc: targetAmmo (ammo change, mod change)
				return true;
			}
		}
		_DEBUG("FindInvalid: invalid ammo found: no ammo types for base ammo OR currAmmoMod not in targetAmmoTypes"); //new: basecal (basecal mod change)
		*modResult = ammoMod;
		*finalAmmo = baseAmmo;
		return true;
	}

	BGSMod::Attachment::Mod* GetAmmoModIfInvalid(BGSObjectInstanceExtra* mods, TESObjectWEAP* baseWeap)
	{
		BGSMod::Attachment::Mod* ammoMod = Utilities::GetModAtAttachPoint(mods, MSF_MainData::ammoAP);
		if (!ammoMod)
			return nullptr;
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(mods, baseWeap);
		if (!baseAmmo)
			return nullptr;
		auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itAD != MSF_MainData::ammoDataMap.end())
		{
			auto ammoData = (*itAD).second;
			auto itMod = std::find_if(ammoData->ammoMods.begin(), ammoData->ammoMods.end(), [ammoMod](AmmoData::AmmoMod& data) {
				return data.mod == ammoMod;
				});
			if (itMod != ammoData->ammoMods.end())
				return nullptr;
		}
		return ammoMod;
	}

	void SpawnRandomMods(TESObjectCELL* cell)
	{
		if (MSF_MainData::MCMSettingFlags & (MSF_MainData::bSpawnRandomAmmo | MSF_MainData::bSpawnRandomMods))
		{
			//_DEBUG("mod spawn");
			for (UInt32 i = 0; i < cell->objectList.count; i++)
			{
				Actor* randomActor = DYNAMIC_CAST(cell->objectList[i], TESObjectREFR, Actor);
				if (!randomActor || !randomActor->biped.get() || randomActor == (*g_player))
					continue;
				//check if unique
				TESObjectWEAP* firearm = Utilities::GetEquippedWeapon(randomActor);
				if (!firearm)
					continue;
				if (randomActor->middleProcess && randomActor->middleProcess->unk08 && randomActor->middleProcess->unk08->equipData.entries && randomActor->middleProcess->unk08->equipData[0].equippedData)
				{
					_DEBUG("actor ok");
					TESAmmo* ammo = randomActor->middleProcess->unk08->equipData[0].equippedData->ammo;
					std::vector<BGSMod::Attachment::Mod*> mods;
					UInt32 count = 0;
					MSF_Data::PickRandomMods(&mods, &ammo, &count);
					_DEBUG("picked ammo: %p, %i", ammo, count);
					if (ammo)
					{
						Utilities::AddItem(randomActor, ammo, count, true);
					}
					for (auto itMods = mods.begin(); itMods != mods.end(); itMods++)
					{
						Utilities::AttachModToInventoryItem(randomActor, firearm, *itMods);
						_DEBUG("mod added");
					}
					//check mod association or inject to TESObjectWEAP at start
				}
			}
		}
	}

	bool EquipAmmo(BGSInventoryList* invList, TESAmmo* ammo)
	{
		if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowEquippedAmmo))
			return true;
		for (UInt32 i = 0; i < invList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			invList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form->formType != kFormType_AMMO)
				continue;
			if (!inventoryItem.stack)
				continue;
			UInt8 notSupported = MSF_Base::IsNotSupportedAmmo((TESAmmo*)inventoryItem.form);
			if (notSupported)
				continue;
			if (inventoryItem.form == ammo)
			{
				for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
					InterlockedExchange16((volatile short*)&stack->flags, 1);
			}
			else
			{
				for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
				{
					if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped) //&& not fusioncore
						InterlockedExchange16((volatile short*)&stack->flags, 0);
				}
			}
		}
		return true;
	}

	UInt8 IsNotSupportedAmmo(TESAmmo* ammo)
	{
		UInt8 ret = 0;
		if (ammo)
		{
			Ammo* ammoData = (Ammo*)ammo;
			if (!ammoData->isPlayable())
				ret |= ExtraWeaponState::WeaponState::bNotPlayable;
			if (Utilities::HasKeyword(&ammoData->keywords, MSF_MainData::FusionCoreKW))
				ret |= ExtraWeaponState::WeaponState::bIsFusionCore;
		}
		return ret;
	}



	//========================== Animation Functions ===========================

	bool ReloadWeapon(bool full, bool clearAmmoCount, bool forced, bool isSwitch)
	{
		Actor* playerActor = *g_player;
		if (isSwitch)
		{
			TESObjectWEAP::InstanceData* inst = Utilities::GetEquippedInstanceData(playerActor);
			if (inst && Utilities::WeaponInstanceHasKeyword(inst, MSF_MainData::AnimsEmptyBeforeReloadKW))
			{
				clearAmmoCount = false;
				full = true;
			}
		}
		if (clearAmmoCount)
		{
			EquipWeaponData* eqData = Utilities::GetEquippedWeaponData(playerActor);
			eqData->loadedAmmoCount = 0;
		}
		if (!full)
		{
			if (isSwitch)
				Utilities::SetAnimationVariableInt(playerActor, "SwitchAmmoTypeReload", 1);
			MSF_MainData::modSwitchManager.SetForcedReload(forced);
			bool success = Utilities::PlayIdleAction(playerActor, MSF_MainData::ActionReload);
			if (!success)
				Utilities::SetAnimationVariableInt(playerActor, "SwitchAmmoTypeReload", 0);
			return success;
		}
		TESIdleForm* relIdle = MSF_Data::GetReloadAnimation(playerActor);
		if (relIdle)
		{
			if (isSwitch)
				Utilities::SetAnimationVariableInt(playerActor, "SwitchAmmoTypeReload", 1);
			bool success = Utilities::PlayIdle(playerActor, relIdle);
			if (!success)
				Utilities::SetAnimationVariableInt(playerActor, "SwitchAmmoTypeReload", 0);
			return true;
		}
		return false;
	}

	bool DrawWeapon()
	{
		Actor* playerActor = *g_player;
		if (playerActor->actorState.IsWeaponDrawn())
			return false;
		Utilities::DrawWeapon(playerActor);
		return true;
	}

	bool PlayAnim(AnimationData* animData)
	{
		if (!animData)
			return false;
		TESIdleForm* anim = animData->GetAnimation();
		if (anim)
		{
			MSF_MainData::modSwitchManager.SetShouldBlendAnimation(animData->shouldBlendAnim);
			Utilities::PlayIdle(*g_player, anim);
			return true;
		}
		return false;
	}

}
