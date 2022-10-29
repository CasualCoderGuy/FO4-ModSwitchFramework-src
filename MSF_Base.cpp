#include "MSF_Base.h"

RelocAddr <_AttachModToStack> AttachRemoveModStack(0x01A84B0);
RelocAddr <_UpdMidProc> UpdateMiddleProcess(0x0E2C3E0);
RelocAddr <_UpdateEquipData> UpdateEquipData(0x01C0040);
RelocAddr <_UpdateAnimGraph> UpdateAnimGraph(0x0D7EB20);
RelocAddr <_EquipHandler> EquipHandler(0x0E1D6D0);
RelocAddr <_UniversalEquipHandler> UniversalEquipHandler(0x0DBEA80);
RelocAddr <_UnkSub_EFF9D0> UnkSub_EFF9D0(0xEFF9D0);
RelocAddr <_UnkSub_DFE930> UnkSub_DFE930(0xDFE930);

namespace MSF_Base
{
	//========================== Main Functions ===========================

	//FROM SCALEFORM:
	bool SwitchToSelectedAmmo(void* obj, bool bAttach)
	{
		AmmoData::AmmoMod* selectedAmmo = reinterpret_cast<AmmoData::AmmoMod*>(obj);
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (!MSF_Base::InitWeapon())
			return false;
		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled) || (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}
		BGSMod::Attachment::Mod* mod = selectedAmmo->mod;
		if (bAttach)
		{
			MSF_MainData::switchData.ModToAttach = mod;
			MSF_MainData::switchData.ModToRemove = nullptr;
		}
		else
		{
			MSF_MainData::switchData.ModToAttach = nullptr;
			MSF_MainData::switchData.ModToRemove = mod;
		}

		if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled || (*g_player)->actorState.IsWeaponDrawn())
		{
			MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadNeeded); //| SwitchData::bDrawInProgress
			if (!MSF_Base::DrawWeapon())
			{
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
				{
					MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress); // | SwitchData::bReloadInProgress | SwitchData::bReloadNotFinished
					MSF_Base::ReloadWeapon();
					return true;
				}
			}
			else
				return true;
		}
		MSF_MainData::switchData.SwitchFlags |= SwitchData::bSwitchingInProgress;
		MSF_Base::SwitchMod();
		return true;
	}

	//FROM HOTKEY:
	bool SwitchAmmoHotkey(UInt8 key)
	{
		Actor* playerActor = *g_player;
		_MESSAGE("swflags: %04X; unk08: %08X; flags: %08X", MSF_MainData::switchData.SwitchFlags, playerActor->actorState.unk08, playerActor->actorState.flags);
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (!MSF_Base::InitWeapon())
			return false;
		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled) || (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}
		if (MSF_Data::GetNthAmmoMod(key))
		{
			_MESSAGE("modOK");
			if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled || (*g_player)->actorState.IsWeaponDrawn())
			{
				MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
				if (!MSF_Base::DrawWeapon())
				{
					_MESSAGE("drawOK");
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
					{
						MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); //  | SwitchData::bReloadNotFinished
						if (MSF_Base::ReloadWeapon()) _MESSAGE("reloadOK");;
						return true;
					}
				}
				else
					return true;
			}
			MSF_MainData::switchData.SwitchFlags |= SwitchData::bSwitchingInProgress;
			MSF_Base::SwitchMod();
			return true;
		}
		MSF_MainData::switchData.ClearData();
		return false;
	}

	//FROM SCALEFORM:
	bool SwitchToSelectedMod(void* modToAttach, void* modToRemove)
	{
		if (!modToAttach && !modToRemove)
			return false;
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (!(MSF_MainData::switchData.SwitchFlags & SwitchData::bIgnoreAnimations))
		{
			if (!MSF_Base::HandlePendingAnimations())
				return false;
		}
		MSF_MainData::switchData.ModToAttach = reinterpret_cast<BGSMod::Attachment::Mod*>(modToAttach);
		MSF_MainData::switchData.ModToRemove = reinterpret_cast<BGSMod::Attachment::Mod*>(modToRemove);
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bSetLooseMods)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bSetLooseMods;
			if (MSF_MainData::switchData.ModToAttach)
				MSF_MainData::switchData.LooseModToRemove = Utilities::GetLooseMod(MSF_MainData::switchData.ModToAttach);
			else
				MSF_MainData::switchData.LooseModToRemove = nullptr;
			if (MSF_MainData::switchData.ModToRemove)
				MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(MSF_MainData::switchData.ModToRemove);
			else
				MSF_MainData::switchData.LooseModToAdd = nullptr;
		}
		else
		{
			MSF_MainData::switchData.LooseModToRemove = nullptr;
			MSF_MainData::switchData.LooseModToAdd = nullptr;
		}
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bDrawEnabled || (*g_player)->actorState.IsWeaponDrawn())
		{
			MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
			if (!MSF_Base::DrawWeapon())
			{
				MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
				if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled) && MSF_Base::PlayAnim())
					return true;
			}
			else
				return true;
		}
		MSF_MainData::switchData.SwitchFlags |= SwitchData::bSwitchingInProgress;
		MSF_Base::SwitchMod();
		return true;
	}


	//FROM HOTKEY:
	bool ToggleModHotkey(std::vector<ModAssociationData*>* modAssociations)
	{
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		Actor* playerActor = *g_player;
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		if (MSF_Data::GetNextMod(eqStack, modAssociations))
		{
			if (!(MSF_MainData::switchData.SwitchFlags & SwitchData::bIgnoreAnimations))
			{
				if (!MSF_Base::HandlePendingAnimations())
					return false;
			}
			if (MSF_MainData::switchData.SwitchFlags & SwitchData::bDrawEnabled || playerActor->actorState.IsWeaponDrawn())
			{
				MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
				if (!MSF_Base::DrawWeapon())
				{
					MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
					if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled) && MSF_Base::PlayAnim())
						return true;
				}
				else
					return true;
			}
			MSF_MainData::switchData.SwitchFlags |= SwitchData::bSwitchingInProgress;
			MSF_Base::SwitchMod();
			return true;
		}
		MSF_MainData::switchData.ClearData();
		return false;
	}

	bool SwitchModHotkey(UInt8 key, std::vector<ModAssociationData*>* modAssociations)
	{
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (MSF_Data::GetNthMod(key, modAssociations))
		{
			if (!(MSF_MainData::switchData.SwitchFlags & SwitchData::bIgnoreAnimations))
			{
				if (!MSF_Base::HandlePendingAnimations())
					return false;
			}
			if (MSF_MainData::switchData.SwitchFlags & SwitchData::bDrawEnabled || (*g_player)->actorState.IsWeaponDrawn())
			{
				MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
				if (!MSF_Base::DrawWeapon())
				{
					MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
					if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled) && MSF_Base::PlayAnim())
						return true;
				}
				else
					return true;
			}
			MSF_MainData::switchData.SwitchFlags |= SwitchData::bSwitchingInProgress;
			MSF_Base::SwitchMod();
			return true;
		}
		MSF_MainData::switchData.ClearData();
		return false;
	}

	bool HandlePendingAnimations() //delay!
	{
		Actor* playerActor = *g_player;
		if (playerActor->actorState.unk08 & (ActorStateFlags08::kActorState_Bashing)) // | ActorStateFlags08::kActorState_Sprint
		{
			MSF_MainData::switchData.ClearData();
			return false;
		}
		if (playerActor->actorState.flags & (ActorStateFlags0C::kActorState_FurnitureState | ActorStateFlags0C::kWeaponState_Reloading | ActorStateFlags0C::kWeaponState_Fire | ActorStateFlags0C::kWeaponState_Sheathing)) // | ActorStateFlags0C::kWeaponState_Lowered | ActorStateFlags0C::kWeaponState_Aim
		{
			MSF_MainData::switchData.ClearData();
			return false;
		}
		if (!(playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Drawn) && (playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Draw))
		{
			MSF_MainData::switchData.ClearData();
			return false;
		}
		return true;
	}

	bool InitWeapon()
	{
		Actor* playerActor = *g_player;
		if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(playerActor, 41), MSF_MainData::APbaseMod))
		{
			if (!AttachModToEquippedWeapon(playerActor, MSF_MainData::APbaseMod, true, 0, false))
				return false;
			if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(playerActor, 41), MSF_MainData::APbaseMod))
				return false;
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bNeedInit;
		}
		return true;
	}

	bool SwitchMod()
	{
		_MESSAGE("SwitchMod; flags: %02X ; Attach: %08X ; Remove: %08X", MSF_MainData::switchData.SwitchFlags, MSF_MainData::switchData.ModToAttach, MSF_MainData::switchData.ModToRemove);
		Actor* playerActor = *g_player;

		BGSMod::Attachment::Mod* modToAttach = MSF_MainData::switchData.ModToAttach;
		BGSMod::Attachment::Mod* modToRemove = MSF_MainData::switchData.ModToRemove;
		TESObjectMISC* looseModToAdd = MSF_MainData::switchData.LooseModToAdd;
		TESObjectMISC* looseModToRemove = MSF_MainData::switchData.LooseModToRemove;

		UInt16 flags = MSF_MainData::switchData.SwitchFlags;
		MSF_MainData::switchData.ClearData();
		MSF_MainData::switchData.SwitchFlags = flags & ~SwitchData::bSwitchingInProgress;


		

		if (modToRemove)
		{
			//if (!Utilities::HasObjectMod(modData, mod))
			//	ClearSwitchFlags();
			bool bUpdateAnimGraph = false;
			if (!modToAttach && flags & SwitchData::bUpdateAnimGraph)
				bUpdateAnimGraph = true;
			if (!AttachModToEquippedWeapon(playerActor, modToRemove, false, 2, bUpdateAnimGraph))
				return false;
			if (looseModToAdd)
				Utilities::AddItem(playerActor, looseModToAdd, 1, true);
		}
		if (modToAttach)
		{
			//if (Utilities::HasObjectMod(modData, mod))
			//	ClearSwitchFlags();
			if (!AttachModToEquippedWeapon(playerActor, modToAttach, true, 2, (flags & SwitchData::bUpdateAnimGraph) != 0))
				return false;
			if (looseModToRemove)
				Utilities::RemoveItem(playerActor, looseModToRemove, 1, true);
		}
		//MSF_Scaleform::UpdateWidget(priority);
		return true;
	}

	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modLoadedAmmoCount, bool updateAnimGraph)
	{
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
				auto data = objectModData->data;
				if (!data || !data->forms)
					continue;
				ExtraInstanceData* extraInstance = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstance->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				if (!instanceData)
					continue;
				if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::hasSwitchedAmmoKW))
				{
					BGSMod::Attachment::Mod* mod = Utilities::FindModByUniqueKeyword(objectModData, MSF_MainData::hasSwitchedAmmoKW);
					TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
					bool found = false;
					for (std::vector<AmmoData>::iterator itAmmoData = MSF_MainData::ammoData.begin(); itAmmoData != MSF_MainData::ammoData.end(); itAmmoData++)
					{
						if (itAmmoData->baseAmmoData.ammo == baseAmmo)
						{
							for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
							{
								if (itAmmoMod->mod == mod)
								{
									found = true;
									break;
								}
							}
							break;
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

	void CreateRandomWeaponHealth(TESObjectCELL* cell)
	{
		for (UInt32 i = 0; i < cell->objectList.count; i++)
		{
			Actor* nextActor = DYNAMIC_CAST(cell->objectList[i], TESObjectREFR, Actor);
			if (!nextActor)
				continue;
			UInt8 isPlayer = (nextActor == (*g_player));
			if (!nextActor->inventoryList)
				continue;
			for (UInt32 i = 0; i < nextActor->inventoryList->items.count; i++)
			{
				TESObjectWEAP* nextWeap = DYNAMIC_CAST(nextActor->inventoryList->items[i].form, TESForm, TESObjectWEAP);
				if (!nextWeap)
					continue;
				for (BGSInventoryItem::Stack* stack = nextActor->inventoryList->items[i].stack; stack; stack = stack->next)
				{
					if (!stack->extraData)
						continue;
					if (!stack->extraData->HasType(kExtraData_Health))
						stack->extraData->Add(kExtraData_Health, ExtraHealth::Create(isPlayer*1.0 + (isPlayer ^ 1)*MSF_MainData::rng.RandomFloat(0.05, 1.0)));
				}
			}
		}
	}

	void DamageEquippedWeapon(Actor* actor)
	{
		UInt8 slots[1] = { 41 };
		TESObjectWEAP* weap = nullptr;
		ExtraDataList* extraData = nullptr;
		TESObjectWEAP::InstanceData* instanceData = nullptr;
		for (UInt8 i = 0; i < sizeof(slots) / sizeof(UInt8); i++)
		{
			weap = (TESObjectWEAP*)actor->equipData->slots[slots[i]].item;
			if (!weap) continue;
			else 
			{ 
				actor->GetEquippedExtraData(slots[i], &extraData);
				instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(actor->equipData->slots[slots[i]].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				break; 
			}
		}
		if (!weap || !extraData || !instanceData)
			return;
		ExtraHealth* extraHealth = DYNAMIC_CAST(extraData->GetByType(kExtraData_Health), BSExtraData, ExtraHealth);
		if (!extraHealth) //create
		{
			extraData->Add(kExtraData_Health, ExtraHealth::Create(1.0)); _MESSAGE("create");
		}
		else
		//for (UInt32 i = 0; i < 1; i++) //keyword-multiplier struct iterator
		{
			BGSKeyword* kw = nullptr; float baseMultiplier = 0.001; float kwMultiplier = 1.0;
			//if (Utilities::WeaponInstanceHasKeyword(instanceData, kw))
			//{
				extraHealth->health -= baseMultiplier * kwMultiplier;
				_MESSAGE("health: %f", extraHealth->health);
				if (extraHealth->health <= 0.0)
				{
					extraHealth->health = 0.0;
					//set broken
				}
				//update widget
			//}
		}
	}

	void JamWeapon(Actor* actor)
	{
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(actor->equipData->slots[41].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData)
			return;
		//chance: more likely to jam when condition is lower
		//jam weapon
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

	bool PlayAnim()
	{
		PlayerCamera* playerCamera = *g_playerCamera;
		SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		if (state == 0 && MSF_MainData::switchData.AnimToPlay1stP)
		{
			Utilities::PlayIdle(*g_player, MSF_MainData::switchData.AnimToPlay1stP);
			return true;
		}
		else if ((state == 7 || state == 8) && MSF_MainData::switchData.AnimToPlay3rdP)
		{
			Utilities::PlayIdle(*g_player, MSF_MainData::switchData.AnimToPlay3rdP);
			return true;
		}
		return false;
	}

	void BurstWaitAndShoot(BurstMode* data)
	{
		if (!data)
			return;
		UInt8 totalShots = data->flags &= ~0xF0;
		for (data->NumOfRoundsFired; data->NumOfRoundsFired < totalShots; data->NumOfRoundsFired++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(data->wait));
			if (data->animReady)
			{
				data->animReady = false;
				Utilities::PlayIdle(data->actor, data->fireIdle);
			}
			//Onkeyup: if !enableonepullburst end thread, if resetshotcounts (AV) NumOfRoundsFired = 0;
		}


	}

	void BurstTest(BurstMode* data)
	{
		if (!data)
		{
			data = new BurstMode;
			data->actor = *g_player;
			data->fireSingleAction = MSF_MainData::ActionFireSingle;
			data->fireIdle = MSF_MainData::fireIdle1stP;
			data->wait = 1000;
			data->totalShots = 3;
			data->NumOfRoundsFired = 1;
			delayTask delay(data->wait, true, &BurstTest, data);
			return;
		}
		Utilities::PlayIdle(data->actor, data->fireIdle);
		data->NumOfRoundsFired++;
		if (data->NumOfRoundsFired < data->totalShots)
			delayTask delay(data->wait, false, &BurstTest, data);
		else
			delete data;
	}



	bool FireBurst(Actor* actor)
	{
		if (!actor)
			return false;
		float av = Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeTime->formID);
		_MESSAGE("av: %f", av);
		return true;

		TESObjectWEAP* eqWeap = DYNAMIC_CAST(actor->equipData->slots[41].item, TESForm, TESObjectWEAP);
		if (!eqWeap)
			return false;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(actor->equipData->slots[41].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData)
			return false;
		if (!actor->middleProcess || !actor->middleProcess->unk08->equipData || !actor->middleProcess->unk08->equipData->equippedData)
			return false;
		if (actor->middleProcess->unk08->equipData->equippedData->unk18 <= 0)
			return false; //if resetshotcountsonreload
		//isAnimPlaying
		UInt16 wait = (UInt16)roundp(Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeTime->formID));
		if (wait < 50)
			return false;
		// check if actor has burst data w/ iterator, if not, make new one
		BurstMode* data = nullptr;
		data->actor = actor;
		data->wait = wait;
		//
		//if (MSF_MainData::tmr.IsRunning())
		//	return false;
		//MSF_MainData::tmr.start();
		data->animReady = false;
		SInt32 state = 8;
		if (actor == *g_player)
		{
			PlayerCamera* playerCamera = *g_playerCamera;
			state = playerCamera->GetCameraStateId(playerCamera->cameraState);
			data->flags = (UInt8)roundp(Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeFlags->formID));
		}
		if (state == 0)
		{
			data->fireIdle = MSF_Data::GetFireAnimation(eqWeap, false);
			std::thread(BurstWaitAndShoot, data);
			return true;
		}
		else if (state == 7 || state == 8)
		{
			data->fireIdle = MSF_Data::GetFireAnimation(eqWeap, true);
			std::thread(BurstWaitAndShoot, data);
			return true;
		}
	}
}
