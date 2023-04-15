#include "MSF_Base.h"

namespace MSF_Base
{
	//========================== Main Functions ===========================

	//FROM SCALEFORM:
	bool SwitchToSelectedAmmo(AmmoData::AmmoMod* selectedAmmo)//(void* obj)
	{
		_MESSAGE("selectCalled");
		//AmmoData::AmmoMod* selectedAmmo = reinterpret_cast<AmmoData::AmmoMod*>(obj);

		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled) )// || (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}

		if (!MSF_Base::InitWeapon())
			return false;

		_MESSAGE("checksPassed");
		SwitchData* switchData = new SwitchData();
		BGSMod::Attachment::Mod* mod = selectedAmmo->mod;
		_MESSAGE("mod: %p", mod);
		if (mod)
			switchData->ModToAttach = mod;
		else
		{
			BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41);
			mod = Utilities::GetModAtAttachPoint(modData, MSF_MainData::ammoAP);
			_MESSAGE("modatAP: %p", mod);
			if (mod)
				switchData->ModToRemove = mod;
			else
				return false;
		}

		_MESSAGE("modOK");

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
			_MESSAGE("toReload");
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
		_MESSAGE("queueCount: %i; unk08: %08X; flags: %08X", MSF_MainData::modSwitchManager.GetQueueCount(), playerActor->actorState.unk08, playerActor->actorState.flags);

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
	bool SwitchToSelectedMod(ModData::Mod* modToAttach, ModData::Mod* modToRemove, bool bNeedInit)//(void* modDataToAttach, void* modDataToRemove, bool bNeedInit)
	{
		if (!modToAttach && !modToRemove)
			return false;

		//ModData::Mod* modToAttach = reinterpret_cast<ModData::Mod*>(modDataToAttach);
		//ModData::Mod* modToRemove = reinterpret_cast<ModData::Mod*>(modDataToRemove);

		//if (!MSF_Data::CheckSwitchRequirements(stack, modToAttach, modToRemove))
		//	return false;

		return MSF_Data::QueueModsToSwitch(modToAttach, modToRemove, bNeedInit);
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
		Actor* playerActor = *g_player;
		if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(playerActor, 41), MSF_MainData::APbaseMod))
		{
			_MESSAGE("init");
			if (!AttachModToEquippedWeapon(playerActor, MSF_MainData::APbaseMod, true, 0, false))
				return false;
			if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(playerActor, 41), MSF_MainData::APbaseMod))
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
		_MESSAGE("SwitchMod; flags: %02X ; Attach: %08X ; Remove: %08X", switchData->SwitchFlags, switchData->ModToAttach, switchData->ModToRemove);
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
				_MESSAGE("finishing1");
				MSF_MainData::modSwitchManager.FinishSwitch(switchData);
				return false;
			}
			ReevalSwitchedWeapon(playerActor, modToRemove);
			if (looseModToAdd)
				Utilities::AddItem(playerActor, looseModToAdd, 1, true);
		}
		if (modToAttach)
		{
			//if (Utilities::HasObjectMod(modData, mod))
			//	ClearSwitchFlags();
			if (!AttachModToEquippedWeapon(playerActor, modToAttach, true, 2, (flags & SwitchData::bUpdateAnimGraph) != 0))
			{
				_MESSAGE("finishing2");
				MSF_MainData::modSwitchManager.FinishSwitch(switchData);
				return false;
			}
			if (looseModToRemove)
				Utilities::RemoveItem(playerActor, looseModToRemove, 1, true);
		}
		_MESSAGE("finishing3");
		MSF_MainData::modSwitchManager.FinishSwitch(switchData);
		_MESSAGE("finishOK");
		if (updateWidget)
			MSF_Scaleform::UpdateWidgetData();
		_MESSAGE("updateOK");
		return true;
	}

	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modLoadedAmmoCount, bool updateAnimGraph)
	{
		_MESSAGE("attach, updAnim: %02X", updateAnimGraph);
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

		_MESSAGE("AcheckOK");
		EquipWeaponData* eqData = (EquipWeaponData*)actor->middleProcess->unk08->equipData->equippedData;
		UInt64 loadedAmmoCount = eqData->loadedAmmoCount;
		TESAmmo* ammoType = eqData->ammo;

		UInt8 ret = 1;
		
		CheckStackIDFunctor IDfunctor(Utilities::GetStackID(item, stack));
		ModifyModDataFunctor modFunctor(mod, &ret, bAttach);

		UInt32 unk = 0x00200000;
		AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);

		ExtraDataList* newList = stack->extraData;
		BSExtraData* newExtraMods = newList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* newModList = DYNAMIC_CAST(newExtraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* newExtraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* newInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(newExtraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);

		InstanceDataStruct idStruct;
		idStruct.baseForm = weapBase;
		idStruct.instanceData = newInstanceData;

		actor->middleProcess->unk08->unk290[1] &= 0xFFFFFFFF00000000;//0xFF00000000000000;
		UpdateMiddleProcess(actor->middleProcess, actor, idStruct, newInstanceData->equipSlot);
		actor->middleProcess->unk08->unk290[1] = actor->middleProcess->unk08->unk290[1] & 0xFFFFFFFF00000000 | 0x1;

		volatile long long* lockcnt = (volatile long long*)&actor->equipData->unk00;
		InterlockedIncrement64(lockcnt);
		UpdateEquipData(actor->equipData, idStruct, nullptr);
		InterlockedDecrement64(lockcnt);

		UpdateEnchantments(actor, idStruct, newList);
		UInt8 unk1 = 1;
		ActorStruct actorStruct;
		actorStruct.actor = actor;
		actorStruct.unk08 = &unk1;
		UpdateAVModifiers(actorStruct, newInstanceData->modifiers);

		float reloadSpeed = *g_reloadSpeedMultiplier * newInstanceData->reloadSpeed;
		UpdateAnimValueFloat(&actor->animGraphHolder, g_reloadSpeedAnimValueHolder, reloadSpeed);
		if (newInstanceData->firingData)
			UpdateAnimValueFloat(&actor->animGraphHolder, g_sightedTransitionAnimValueHolder, newInstanceData->firingData->sightedTransition);

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

		if (modLoadedAmmoCount == 0 || (modLoadedAmmoCount == 2 && newAmmoType == ammoType))
			newEqData->loadedAmmoCount = loadedAmmoCount;
		else if (modLoadedAmmoCount == 1 || (modLoadedAmmoCount == 2 && newAmmoType != ammoType))
		{
			UInt64 ammoCount = Utilities::GetInventoryItemCount(actor->inventoryList, newAmmoType);
			if (ammoCount < newInstanceData->ammoCapacity)
				newEqData->loadedAmmoCount = ammoCount;
			else
				newEqData->loadedAmmoCount = newInstanceData->ammoCapacity;
		}

		return true;
	}

	bool ReevalModdedWeapon(TESObjectWEAP* weapon)
	{
		if (!weapon)
			return false;
		Actor* owner = *g_player;
		BGSInventoryItem* item = nullptr;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			item = &owner->inventoryList->items.entries[i];
			TESObjectWEAP* baseWeapon = DYNAMIC_CAST(item->form, TESForm, TESObjectWEAP);
			if (baseWeapon != weapon)
				continue;
			for (BGSInventoryItem::Stack* stack = item->stack; stack; stack = stack->next)
			{
				BGSObjectInstanceExtra * objectModData = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
				ExtraInstanceData* extraInstance = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstance->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				if (!instanceData)
					continue;
				ReevalAttachedMods(owner, item, stack);
				if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::hasSwitchedAmmoKW))
				{
					BGSMod::Attachment::Mod* mod = Utilities::FindModByUniqueKeyword(objectModData, MSF_MainData::hasSwitchedAmmoKW);
					if (!mod)
						continue;
					TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
					bool found = false;
					if (!baseAmmo)
						continue;
					AmmoData* itAmmoData = MSF_MainData::ammoDataMap[baseAmmo];
					if (itAmmoData)
					{
						for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
						{
							if (itAmmoMod->mod == mod)
							{
								found = true;
								break;
							}
						}
					}
					if (!found)
					{
						UInt8 ret = 1;
						CheckStackIDFunctor IDfunctor(Utilities::GetStackID(item, stack));
						ModifyModDataFunctor modFunctor(mod, &ret, false);
						UInt32 unk = 0x00200000;
						AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);
						if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
						{
							ExtraDataList* newList = stack->extraData;
							ExtraInstanceData* newExtraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
							TESObjectWEAP::InstanceData* newInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(newExtraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
							InstanceDataStruct idStruct;
							idStruct.baseForm = weapon;
							idStruct.instanceData = newInstanceData;
							owner->middleProcess->unk08->unk290[1] &= 0xFFFFFFFF00000000;//0xFF00000000000000;
							UpdateMiddleProcess(owner->middleProcess, owner, idStruct, newInstanceData->equipSlot);
							owner->middleProcess->unk08->unk290[1] = owner->middleProcess->unk08->unk290[1] & 0xFFFFFFFF00000000 | 0x1;
							volatile long long* lockcnt = (volatile long long*)&owner->equipData->unk00;
							InterlockedIncrement64(lockcnt);
							UpdateEquipData(owner->equipData, idStruct, nullptr);
							InterlockedDecrement64(lockcnt);
							UpdateEnchantments(owner, idStruct, newList);
							UInt8 unk1 = 1;
							ActorStruct actorStruct;
							actorStruct.actor = owner;
							actorStruct.unk08 = &unk1;
							UpdateAVModifiers(actorStruct, newInstanceData->modifiers);
							float reloadSpeed = *g_reloadSpeedMultiplier * newInstanceData->reloadSpeed;
							UpdateAnimValueFloat(&owner->animGraphHolder, g_reloadSpeedAnimValueHolder, reloadSpeed);
							if (newInstanceData->firingData)
								UpdateAnimValueFloat(&owner->animGraphHolder, g_sightedTransitionAnimValueHolder, newInstanceData->firingData->sightedTransition);
							//UpdateAnimGraph(actor, false);
							bool bEquipped = false;
							for (BGSInventoryItem::Stack* stacks = item->stack; stacks; stacks = stacks->next)
							{
								if (stacks->flags & 1)
								{
									if (bEquipped)
									{
										volatile short* equipFlagPtr = (volatile short*)&stacks->flags;
										InterlockedExchange16(equipFlagPtr, 0);
									}
									bEquipped = true;
								}
							}
						}
					}
				}
			}
		}
		return true;
	}

	bool ReevalSwitchedWeapon(Actor* owner, BGSMod::Attachment::Mod* changedMod)
	{
		if (!changedMod)
			return false;
		bool doCheck = false;
		for (UInt32 i4 = 0; i4 < changedMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
		{
			BGSMod::Container::Data * data = &changedMod->modContainer.data[i4];
			if (data->target == 61 && data->value.form && data->op == 128)
			{
				doCheck = true;
			}
		}
		if (!doCheck)
			return true;

		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(owner, 41);
		if (!stack)
			return false;
		BGSObjectInstanceExtra * objectModData = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstance = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstance)
			return false;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstance->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData)
			return false;
		if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::hasSwitchedAmmoKW))
		{
			BGSMod::Attachment::Mod* mod = Utilities::FindModByUniqueKeyword(objectModData, MSF_MainData::hasSwitchedAmmoKW);
			if (!mod)
				return false;
			TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
			bool found = false;
			if (!baseAmmo)
				return false;
			AmmoData* itAmmoData = MSF_MainData::ammoDataMap[baseAmmo];
			if (itAmmoData)
			{
				for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
				{
					if (itAmmoMod->mod == mod)
					{
						found = true;
						break;
					}
				}
			}
			if (!found)
				AttachModToEquippedWeapon(owner, mod, false, 2, false);
		}
		return true;
	}

	bool ReevalAttachedMods(Actor* owner, BGSInventoryItem* item, BGSInventoryItem::Stack* stack)
	{
		if (!owner || !item || !stack)
			return false;
		BGSObjectInstanceExtra* mods = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		TESObjectWEAP* baseWeap = DYNAMIC_CAST(item->form, TESForm, TESObjectWEAP);
		if (!mods || !baseWeap)
			return false;
		auto data = mods->data;
		if (!data || !data->forms)
			return false;
		std::vector<BGSMod::Attachment::Mod*> objectMods;
		std::vector<BGSMod::Attachment::Mod*> toRemove;
		std::vector<BGSMod::Attachment::Mod*> qualified;
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
					toRemove.push_back(objectMod);
			}
		}
		_MESSAGE("toremove: %i", toRemove.size());
		for (std::vector<BGSMod::Attachment::Mod*>::iterator itMod = toRemove.begin(); itMod != toRemove.end(); itMod++)
		{
			UInt8 ret = 1;
			CheckStackIDFunctor IDfunctor(Utilities::GetStackID(item, stack));
			ModifyModDataFunctor modFunctor(*itMod, &ret, false);
			UInt32 unk = 0x00200000;
			AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);
			if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
			{
				ExtraDataList* newList = stack->extraData;
				ExtraInstanceData* newExtraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				TESObjectWEAP::InstanceData* newInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(newExtraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				InstanceDataStruct idStruct;
				idStruct.baseForm = baseWeap;
				idStruct.instanceData = newInstanceData;
				owner->middleProcess->unk08->unk290[1] &= 0xFFFFFFFF00000000;//0xFF00000000000000;
				UpdateMiddleProcess(owner->middleProcess, owner, idStruct, newInstanceData->equipSlot);
				owner->middleProcess->unk08->unk290[1] = owner->middleProcess->unk08->unk290[1] & 0xFFFFFFFF00000000 | 0x1;
				volatile long long* lockcnt = (volatile long long*)&owner->equipData->unk00;
				InterlockedIncrement64(lockcnt);
				UpdateEquipData(owner->equipData, idStruct, nullptr);
				InterlockedDecrement64(lockcnt);
				UpdateEnchantments(owner, idStruct, newList);
				UInt8 unk1 = 1;
				ActorStruct actorStruct;
				actorStruct.actor = owner;
				actorStruct.unk08 = &unk1;
				UpdateAVModifiers(actorStruct, newInstanceData->modifiers);
				float reloadSpeed = *g_reloadSpeedMultiplier * newInstanceData->reloadSpeed;
				UpdateAnimValueFloat(&owner->animGraphHolder, g_reloadSpeedAnimValueHolder, reloadSpeed);
				if (newInstanceData->firingData)
					UpdateAnimValueFloat(&owner->animGraphHolder, g_sightedTransitionAnimValueHolder, newInstanceData->firingData->sightedTransition);
				//UpdateAnimGraph(actor, false);
				bool bEquipped = false;
				for (BGSInventoryItem::Stack* stacks = item->stack; stacks; stacks = stacks->next)
				{
					if (stacks->flags & 1)
					{
						if (bEquipped)
						{
							volatile short* equipFlagPtr = (volatile short*)&stacks->flags;
							InterlockedExchange16(equipFlagPtr, 0);
						}
						bEquipped = true;
					}
				}
			}
		}
		BGSObjectInstanceExtra* newmods = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		if (!newmods)
			return false;
		auto newdata = mods->data;
		if (!newdata || !newdata->forms)
			return false;
		for (UInt32 i3 = 0; i3 < newdata->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(newdata->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			objectMods.push_back(objectMod);
		}
		AttachParentArray* baseAP = (AttachParentArray*)&baseWeap->attachParentArray;
		if (baseAP->kewordValueArray.count > 0)
		{
			std::vector<KeywordValue> attachPoints;
			for (UInt32 i = 0; i < baseAP->kewordValueArray.count; i++)
			{
				attachPoints.push_back(baseAP->kewordValueArray[i]);
				//BGSKeyword* kw = GetKeywordFromValueArray(AttachParentArray::iDataType, baseAP->kewordValueArray[i]);
				//if (kw)
				//	_MESSAGE("apkw: %s", kw->GetEditorID());
			}
			//_MESSAGE("baseap: %i", attachPoints.size());
			//for (std::vector<KeywordValue>::iterator itValue = attachPoints.begin(); itValue != attachPoints.end(); itValue++)
			for (UInt32 i = 0; i < attachPoints.size(); i++)
			{
				for (std::vector<BGSMod::Attachment::Mod*>::iterator itMod = objectMods.begin(); itMod != objectMods.end(); itMod++)
				{
					if (*itMod)
					{
						if ((*itMod)->unkC0 == attachPoints[i])
						{
							qualified.push_back(*itMod);
							AttachParentArray* apArray = (AttachParentArray*)&(*itMod)->unk98;
							for (UInt32 i2 = 0; i2 < apArray->kewordValueArray.count; i2++)
							{
								attachPoints.push_back(apArray->kewordValueArray[i2]);
								//BGSKeyword* kw = GetKeywordFromValueArray(AttachParentArray::iDataType, baseAP->kewordValueArray[i]);
								//if (kw)
								//	_MESSAGE("apkw: %s", kw->GetEditorID());
							}
							*itMod = nullptr;
						}
					}
				}
				if (qualified.size() == objectMods.size())
					break;
			}
			_MESSAGE("ap: %i", attachPoints.size());
			_MESSAGE("qualified: %i", qualified.size());
			_MESSAGE("all: %i", objectMods.size());
			for (std::vector<BGSMod::Attachment::Mod*>::iterator itMod = objectMods.begin(); itMod != objectMods.end(); itMod++)
			{
				if (*itMod)
				{
					UInt8 ret = 1;
					CheckStackIDFunctor IDfunctor(Utilities::GetStackID(item, stack));
					ModifyModDataFunctor modFunctor(*itMod, &ret, false);
					UInt32 unk = 0x00200000;
					AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);
					if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
					{
						ExtraDataList* newList = stack->extraData;
						ExtraInstanceData* newExtraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
						TESObjectWEAP::InstanceData* newInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(newExtraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
						InstanceDataStruct idStruct;
						idStruct.baseForm = baseWeap;
						idStruct.instanceData = newInstanceData;
						owner->middleProcess->unk08->unk290[1] &= 0xFFFFFFFF00000000;//0xFF00000000000000;
						UpdateMiddleProcess(owner->middleProcess, owner, idStruct, newInstanceData->equipSlot);
						owner->middleProcess->unk08->unk290[1] = owner->middleProcess->unk08->unk290[1] & 0xFFFFFFFF00000000 | 0x1;
						volatile long long* lockcnt = (volatile long long*)&owner->equipData->unk00;
						InterlockedIncrement64(lockcnt);
						UpdateEquipData(owner->equipData, idStruct, nullptr);
						InterlockedDecrement64(lockcnt);
						UpdateEnchantments(owner, idStruct, newList);
						UInt8 unk1 = 1;
						ActorStruct actorStruct;
						actorStruct.actor = owner;
						actorStruct.unk08 = &unk1;
						UpdateAVModifiers(actorStruct, newInstanceData->modifiers);
						float reloadSpeed = *g_reloadSpeedMultiplier * newInstanceData->reloadSpeed;
						UpdateAnimValueFloat(&owner->animGraphHolder, g_reloadSpeedAnimValueHolder, reloadSpeed);
						if (newInstanceData->firingData)
							UpdateAnimValueFloat(&owner->animGraphHolder, g_sightedTransitionAnimValueHolder, newInstanceData->firingData->sightedTransition);
						//UpdateAnimGraph(actor, false);
						bool bEquipped = false;
						for (BGSInventoryItem::Stack* stacks = item->stack; stacks; stacks = stacks->next)
						{
							if (stacks->flags & 1)
							{
								if (bEquipped)
								{
									volatile short* equipFlagPtr = (volatile short*)&stacks->flags;
									InterlockedExchange16(equipFlagPtr, 0);
								}
								bEquipped = true;
							}
						}
					}
				}
			}
		}
		return true;
	}

	void SpawnRandomMods(TESObjectCELL* cell)
	{
		for (UInt32 i = 0; i < cell->objectList.count; i++)
		{
			Actor* randomActor = DYNAMIC_CAST(cell->objectList[i], TESObjectREFR, Actor);
			if (!randomActor || randomActor == (*g_player))
				continue;
			TESObjectWEAP* firearm = (TESObjectWEAP*)randomActor->equipData->slots[41].item;
			if (!firearm)
				continue;
			TESAmmo* ammo = randomActor->middleProcess->unk08->equipData->equippedData->ammo;
			tArray<BGSMod::Attachment::Mod*> mods;
			UInt32 count = 0;
			MSF_Data::PickRandomMods(&mods, &ammo, &count);
			if (ammo)
			{
				VMArray<VMVariable> args;
				VMVariable var1; VMVariable var2;
				var1.Set(&ammo); args.Push(&var1); var2.Set(&count); args.Push(&var2);
				CallFunctionNoWait(randomActor, "AddItem", args);
			}
			if (mods.count > 0)
			{
				VMArray<VMVariable> args;
				VMVariable var1; VMVariable var2;
				for (UInt32 n = 0; n < mods.count; n++)
				{
					args.Clear(); var1.Set(&firearm); args.Push(&var1); var2.Set(&mods[n]); args.Push(&var2);
					CallFunctionNoWait(randomActor, "AttachModToInventoryItem", args);
				}
			}
			//check mod association or inject to TESObjectWEAP at start
		}
	}



	//========================== Animation Functions ===========================

	bool ReloadWeapon()
	{
		Actor* playerActor = *g_player;
		PlayerCamera* playerCamera = *g_playerCamera;
		SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		TESObjectWEAP* eqWeap = DYNAMIC_CAST(playerActor->equipData->slots[41].item, TESForm, TESObjectWEAP);
		if (!eqWeap)
			return false;
		if (state == 0)
		{
			TESIdleForm* relIdle = MSF_Data::GetReloadAnimation(eqWeap, false);
			Utilities::PlayIdle(playerActor, relIdle);
			return true;
		}
		else if (state == 7 || state == 8)
		{
			TESIdleForm* relIdle = MSF_Data::GetReloadAnimation(eqWeap, true);
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
		PlayerCamera* playerCamera = *g_playerCamera;
		SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		if (state == 0 && animData->animIdle_1stP)
		{
			Utilities::PlayIdle(*g_player, animData->animIdle_1stP);
			return true;
		}
		else if ((state == 7 || state == 8) && animData->animIdle_3rdP)
		{
			Utilities::PlayIdle(*g_player, animData->animIdle_3rdP);
			return true;
		}
		return false;
	}

}
