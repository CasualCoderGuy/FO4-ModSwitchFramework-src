#include "MSF_Base.h"
#include "MSF_Events.h"

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

		if (!MSF_Base::InitWeapon())
			return false;

		_DEBUG("checksPassed");
		SwitchData* switchData = new SwitchData();
		BGSMod::Attachment::Mod* mod = selectedAmmo->mod;
		_DEBUG("mod: %p", mod);
		if (mod)
			switchData->ModToAttach = mod;
		else
		{
			BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41);
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

		if ((*g_player)->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
		{
			_DEBUG("toReload");
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); // | SwitchData::bReloadNotFinished
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			if (!MSF_Base::ReloadWeapon())
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
		_DEBUG("queueCount: %i; unk08: %08X; flags: %08X", MSF_MainData::modSwitchManager.GetQueueCount(), playerActor->actorState.unk08, playerActor->actorState.flags);

		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))// || (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}
		if (!MSF_Base::InitWeapon())
			return false;

		SwitchData* switchData = MSF_Data::GetNthAmmoMod(key);
		if (!switchData)
			return false;

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

		if ((*g_player)->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
		{
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); // | SwitchData::bReloadNotFinished
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			if (!MSF_Base::ReloadWeapon())
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
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(*g_player, 41);
		return MSF_Data::GetNextMod(eqStack, modData);
	}

	bool SwitchModHotkey(UInt8 key, ModData* modData)
	{
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(*g_player, 41);
		return MSF_Data::GetNthMod(key, eqStack, modData);
	}

	bool HandlePendingAnimations()
	{
		Actor* playerActor = *g_player;
		if ((playerActor->actorState.unk08 & (ActorStateFlags08::kActorState_Bashing)) || // | ActorStateFlags08::kActorState_Sprint
			(playerActor->actorState.flags & (ActorStateFlags0C::kActorState_FurnitureState | ActorStateFlags0C::kWeaponState_Reloading | ActorStateFlags0C::kWeaponState_Fire | ActorStateFlags0C::kWeaponState_Sheathing)) || // | ActorStateFlags0C::kWeaponState_Lowered | ActorStateFlags0C::kWeaponState_Aim
			(!(playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Drawn) && (playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Draw)))
		{
			return false;
		}
		return true;
	}

	bool InitWeapon()
	{
		//Actor* playerActor = *g_player;
		//if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(playerActor, 41), MSF_MainData::APbaseMod))
		//{
		//	_DEBUG("init");
		//	if (!AttachModToEquippedWeapon(playerActor, MSF_MainData::APbaseMod, true, 0, false))
		//		return false;
		//	if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(playerActor, 41), MSF_MainData::APbaseMod))
		//		return false;
		//}
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
			else if ((*g_player)->actorState.IsWeaponDrawn())
			{
				if (switchData->SwitchFlags & SwitchData::bReloadNeeded)
				{
					switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress | ~SwitchData::bReloadNeeded); // | SwitchData::bReloadNotFinished
					if (!MSF_Base::ReloadWeapon())
						MSF_MainData::modSwitchManager.ClearQueue();
				}
				else if (switchData->SwitchFlags & SwitchData::bAnimNeeded)
				{
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
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(actor, 41);
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
		EquipWeaponData* eqData = (EquipWeaponData*)actor->middleProcess->unk08->equipData->equippedData;
		UInt64 loadedAmmoCount = eqData->loadedAmmoCount;
		TESAmmo* ammoType = eqData->ammo;

		bool ret = false;

		CheckStackIDFunctor IDfunctor(Utilities::GetStackID(item, stack));
		ModifyModDataFunctor modFunctor(mod, 0, bAttach , &ret);

		UInt32 unk = 0x00200000;
		AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);

		ExtraDataList* newList = stack->extraData;
		BSExtraData* newExtraMods = newList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* newModList = DYNAMIC_CAST(newExtraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* newExtraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* newInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(newExtraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);

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

		BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(newModList, weapBase);
		if (invalidAmmoMod)
		{
			_DEBUG("invalid ammomod: %08X", invalidAmmoMod->formID);
			unk = 0x00200000;
			bool success = false;
			ModifyModDataFunctor modifyModFunctor(invalidAmmoMod, 0, false, &success);
			//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);;
		}

		BGSObjectInstance idStruct;
		idStruct.baseForm = weapBase;
		idStruct.instanceData = newInstanceData;

		MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(true);
		EquipItemInternal(g_ActorEquipManager, actor, idStruct, 0, 1, nullptr, 0, 0, 0, 1, 0);
		//actor->middleProcess->unk08->unk290[1] &= 0xFFFFFFFF00000000;//0xFF00000000000000;
		//UpdateMiddleProcess(actor->middleProcess, actor, idStruct, newInstanceData->equipSlot);
		//actor->middleProcess->unk08->unk290[1] = actor->middleProcess->unk08->unk290[1] & 0xFFFFFFFF00000000 | 0x1;

		//UpdateEquippedWeaponData(actor->middleProcess->unk08->equipData->equippedData);

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
		
		//InterlockedIncrement(&newList->m_refCount);
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

		EquipWeaponData* newEqData = (EquipWeaponData*)actor->middleProcess->unk08->equipData->equippedData;
		TESAmmo* newAmmoType = newEqData->ammo;

		if (modLoadedAmmoCount == 0 || (modLoadedAmmoCount == 2 && newAmmoType == ammoType)) //BCR!!
			newEqData->loadedAmmoCount = loadedAmmoCount;
		else if (modLoadedAmmoCount == 1 || (modLoadedAmmoCount == 2 && newAmmoType != ammoType))
		{
			UInt64 ammoCount = Utilities::GetInventoryItemCount(actor->inventoryList, newAmmoType);
			if (ammoCount < newInstanceData->ammoCapacity)
				newEqData->loadedAmmoCount = ammoCount;
			else
				newEqData->loadedAmmoCount = newInstanceData->ammoCapacity;
		}
		//ExtraWeaponState* weaponState = ExtraWeaponState::Init(newList, newEqData);
		//weaponState->HandleModChangeEvent(newList, equipData);

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
			if (objectMod && MSF_MainData::instantiationRequirements.count(objectMod) > 0)
			{
				auto range = MSF_MainData::instantiationRequirements.equal_range(objectMod);
				bool bRemove = false;
				std::vector<BGSMod::Attachment::Mod*> parentMods;
				Utilities::GetParentMods(mods, objectMod, &parentMods);
				for (auto it = range.first; it != range.second; ++it)
				{
					bRemove = true;
					for (std::vector<BGSMod::Attachment::Mod*>::iterator itMod = parentMods.begin(); itMod != parentMods.end(); itMod++)
					{
						KeywordValueArray* instantiationData = reinterpret_cast<KeywordValueArray*>(&(*itMod)->unkB0);
						if (instantiationData->GetItemIndex(it->second) >= 0)
						{
							bRemove = false;
							break;
						}
					}
					if (bRemove == false)
						break;
				}
				if (bRemove)
					invalidList->push_back(objectMod);
			}
		}
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
			_DEBUG("mod spawn");
			for (UInt32 i = 0; i < cell->objectList.count; i++)
			{
				Actor* randomActor = DYNAMIC_CAST(cell->objectList[i], TESObjectREFR, Actor);
				if (!randomActor || !randomActor->equipData || randomActor == (*g_player))
					continue;
				//check if unique
				TESObjectWEAP* firearm = DYNAMIC_CAST(randomActor->equipData->slots[41].item, TESForm, TESObjectWEAP);
				if (!firearm)
					continue;
				if (randomActor->middleProcess && randomActor->middleProcess->unk08 && randomActor->middleProcess->unk08->equipData && randomActor->middleProcess->unk08->equipData->equippedData)
				{
					_DEBUG("actor ok");
					TESAmmo* ammo = randomActor->middleProcess->unk08->equipData->equippedData->ammo;
					std::vector<BGSMod::Attachment::Mod*> mods;
					UInt32 count = 0;
					MSF_Data::PickRandomMods(&mods, &ammo, &count);
					_DEBUG("picked ammo: %p, %i", ammo, count);
					if (ammo)
					{
						Utilities::AddItem(randomActor, ammo, count, true);
						//Utilities::AttachModToInventoryItem(randomActor, firearm, MSF_MainData::APbaseMod);
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



	//========================== Animation Functions ===========================

	bool ReloadWeapon()
	{
		Actor* playerActor = *g_player;
		TESIdleForm* relIdle = MSF_Data::GetReloadAnimation(playerActor);
		if (relIdle)
		{
			Utilities::PlayIdle(playerActor, relIdle);
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
			Utilities::PlayIdle(*g_player, anim);
			return true;
		}
		return false;
	}

}
