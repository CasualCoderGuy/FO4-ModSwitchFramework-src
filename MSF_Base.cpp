#include "MSF_Base.h"

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
			MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
			if (!MSF_Base::DrawWeapon())
			{
				MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
				{
					MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); // | SwitchData::bReloadNotFinished
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
		if (SwitchData::bNeedInit)
		{
			if (!MSF_Base::InitWeapon())
			{
				MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bNeedInit;
				return false;
			}
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
				MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled)
				{
					MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress); // | SwitchData::bAnimNotFinished
					if (MSF_Base::PlayAnim())
						return true;
					else
						MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
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
	bool ToggleModHotkey(std::vector<ModAssociationData*>* modAssociations)
	{
		_MESSAGE("swflags: %04X", MSF_MainData::switchData.SwitchFlags);
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		Actor* playerActor = *g_player;
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		if (MSF_Data::GetNextMod(eqStack, modAssociations))
		{
			if (SwitchData::bNeedInit)
			{
				if (!MSF_Base::InitWeapon())
				{
					MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bNeedInit;
					return false;
				}
			}
			if (MSF_MainData::switchData.ModToAttach)
				_MESSAGE("mod: %08X", MSF_MainData::switchData.ModToAttach->formID);
			if (MSF_MainData::switchData.ModToRemove)
				_MESSAGE("mod: %08X", MSF_MainData::switchData.ModToRemove->formID);
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
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled)
					{
						MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress); // | SwitchData::bAnimNotFinished
						if (MSF_Base::PlayAnim())
							return true;
						else
							MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
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

	bool SwitchModHotkey(UInt8 key, std::vector<ModAssociationData*>* modAssociations)
	{
		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (MSF_Data::GetNthMod(key, modAssociations))
		{
			if (SwitchData::bNeedInit)
			{
				if (!MSF_Base::InitWeapon())
				{
					MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bNeedInit;
					return false;
				}
			}
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
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled)
					{
						MSF_MainData::switchData.SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress); // | SwitchData::bAnimNotFinished
						if (MSF_Base::PlayAnim())
							return true;
						else
							MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
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

	bool HandlePendingAnimations(bool bDoQueueSwitch) //doqueueswitch; delay(not here, at anim)!
	{
		Actor* playerActor = *g_player;
		if ((playerActor->actorState.unk08 & (ActorStateFlags08::kActorState_Bashing)) || // | ActorStateFlags08::kActorState_Sprint
			(playerActor->actorState.flags & (ActorStateFlags0C::kActorState_FurnitureState | ActorStateFlags0C::kWeaponState_Reloading | ActorStateFlags0C::kWeaponState_Fire | ActorStateFlags0C::kWeaponState_Sheathing)) || // | ActorStateFlags0C::kWeaponState_Lowered | ActorStateFlags0C::kWeaponState_Aim
			(!(playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Drawn) && (playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Draw)))
		{
			if (!bDoQueueSwitch)
				MSF_MainData::switchData.ClearData();
			else
				MSF_MainData::switchData.SwitchFlags |= MSF_MainData::switchData.bQueuedSwitch;
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

	void SwitchFlagsAND(UInt16 flag)
	{
		MSF_MainData::switchData.SwitchFlags &= flag;
	}

	void SwitchFlagsOR(UInt16 flag)
	{
		MSF_MainData::switchData.SwitchFlags |= flag;
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
		MSF_MainData::switchData.SwitchFlags = flags & (SwitchData::bReloadInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);

		if (modToRemove)
		{
			//if (!Utilities::HasObjectMod(modData, mod))
			//	ClearSwitchFlags();
			bool bUpdateAnimGraph = false;
			if (!modToAttach && (flags & SwitchData::bUpdateAnimGraph))
				bUpdateAnimGraph = true;
			if (!AttachModToEquippedWeapon(playerActor, modToRemove, false, 2, bUpdateAnimGraph))
				return false;
			ReevalSwitchedWeapon(playerActor, modToRemove);
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
		MSF_Scaleform::UpdateWidgetData();
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

	bool ReevalAttachedMods(TESObjectWEAP* baseWeap, BGSObjectInstanceExtra* mods)
	{
		if (!baseWeap || !mods)
			return false;
		auto data = mods->data;
		if (!data || !data->forms)
			return false;
		std::vector<BGSMod::Attachment::Mod*> objectMods;
		std::vector<BGSMod::Attachment::Mod*> remaining;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (objectMod)
			{
				objectMods.push_back(objectMod);
				remaining.push_back(objectMod);
			}
		}
		AttachParentArray* baseAP = (AttachParentArray*)&baseWeap->attachParentArray;
		for (std::vector<BGSMod::Attachment::Mod*>::iterator modIt = objectMods.begin(); modIt != objectMods.end(); modIt++)
		{
			if (baseAP->kewordValueArray.GetItemIndex((*modIt)->unkC0) >= 0)
			{
				//remaining.erase()
				//	objectMods.
			}
		}
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

	void SetCurrentWeaponState(bool hasSecondaryAmmo, bool clearSecondaryAmmo)
	{
		ExtraDataList* extraData = nullptr;
		(*g_player)->GetEquippedExtraData(41, &extraData);
		if (extraData)
		{
			ExtraWeaponState* weaponState = (ExtraWeaponState*)extraData->GetByType(ExtraWeaponState::kType_ExtraWeaponState);
			EquipWeaponData* eqData = (EquipWeaponData*)(*g_player)->middleProcess->unk08->equipData->equippedData;
			if (eqData && eqData->ammo)
			{
				if (!weaponState)
				{
					weaponState = new ExtraWeaponState;
					extraData->Add(ExtraWeaponState::kType_ExtraWeaponState, weaponState);
				}
				if (hasSecondaryAmmo)
				{
					weaponState->magazineStateMP.second.ammoType = eqData->ammo;
					weaponState->magazineStateMP.second.magazineCount = eqData->loadedAmmoCount;
				}
				else
				{
					weaponState->magazineStateMP.first.ammoType = eqData->ammo;
					weaponState->magazineStateMP.first.magazineCount = eqData->loadedAmmoCount;
					if (clearSecondaryAmmo)
					{
						weaponState->magazineStateMP.second.ammoType = nullptr;
						weaponState->magazineStateMP.second.magazineCount = 0;
					}
				}
			}
		}
	}

	void RecoverWeaponState(bool recoverSecondary)
	{
		ExtraDataList* extraData = nullptr;
		(*g_player)->GetEquippedExtraData(41, &extraData);
		if (extraData)
		{
			ExtraWeaponState* weaponState = (ExtraWeaponState*)extraData->GetByType(ExtraWeaponState::kType_ExtraWeaponState);
			if (weaponState)
			{
				EquipWeaponData* eqData = (EquipWeaponData*)(*g_player)->middleProcess->unk08->equipData->equippedData;
				if (eqData && eqData->ammo)
				{
					if (recoverSecondary && weaponState->magazineStateMP.second.ammoType)
					{
						if (eqData->ammo == weaponState->magazineStateMP.second.ammoType)
							eqData->loadedAmmoCount = weaponState->magazineStateMP.second.magazineCount;
						else
							eqData->loadedAmmoCount; //change back to switched ammo
					}
					else if (weaponState->magazineStateMP.first.ammoType)
					{
						if (eqData->ammo == weaponState->magazineStateMP.first.ammoType)
							eqData->loadedAmmoCount = weaponState->magazineStateMP.first.magazineCount;
						else
							eqData->loadedAmmoCount; //change back to switched ammo
					}
				}
			}
		}
	}

	void SetChamberedRound(bool hasSecondaryAmmo, bool clearSecondaryAmmo) //only if hasKW chamberedEnable/secChamberedEnable
	{
		ExtraDataList* extraData = nullptr;
		(*g_player)->GetEquippedExtraData(41, &extraData);
		if (extraData)
		{
			ExtraWeaponState* weaponState = (ExtraWeaponState*)extraData->GetByType(ExtraWeaponState::kType_ExtraWeaponState);
			EquipWeaponData* eqData = (EquipWeaponData*)(*g_player)->middleProcess->unk08->equipData->equippedData;
			if (eqData && eqData->ammo)
			{
				if (!weaponState)
				{
					weaponState = new ExtraWeaponState;
					extraData->Add(ExtraWeaponState::kType_ExtraWeaponState, weaponState);
				}
				if (hasSecondaryAmmo)
				{
					weaponState->chamberedRoundMP.second.ammoType = eqData->ammo;
					weaponState->chamberedRoundMP.second.magazineCount = eqData->loadedAmmoCount;
					//+++ BGSImpactDataSet, damageTypes, minRange, maxRange, outOfRangeMultiplier, secondary, critChargeBonus, minPowerShot, critDamageMult, projectileOverride, numProjectiles, stagger, baseDamage
					//remove chambered ammo
				}
				else
				{
					weaponState->chamberedRoundMP.first.ammoType = eqData->ammo;
					weaponState->chamberedRoundMP.first.magazineCount = eqData->loadedAmmoCount;
					//+++
					//remove chambered ammo
					if (clearSecondaryAmmo)
					{
						weaponState->chamberedRoundMP.second.ammoType = nullptr;
						weaponState->chamberedRoundMP.second.magazineCount = 0;
					}
				}
			}
		}
	}

	void RecoverChamberedRound(bool recoverSecondary) //only if hasKW chamberedEnable/secChamberedEnable
	{
		ExtraDataList* extraData = nullptr;
		(*g_player)->GetEquippedExtraData(41, &extraData);
		if (extraData)
		{
			ExtraWeaponState* weaponState = (ExtraWeaponState*)extraData->GetByType(ExtraWeaponState::kType_ExtraWeaponState);
			EquipWeaponData* eqData = (EquipWeaponData*)(*g_player)->middleProcess->unk08->equipData->equippedData;
			TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast((*g_player)->middleProcess->unk08->equipData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
			if (eqData && eqData->ammo && instanceData && instanceData->ammo && weaponState)
			{
				if (recoverSecondary && weaponState->chamberedRoundMP.second.ammoType && weaponState->chamberedRoundMP.second.magazineCount > 0)
				{
					TESAmmo* currentAmmo = instanceData->ammo;
					instanceData->ammo = weaponState->chamberedRoundMP.second.ammoType;
					eqData->ammo = instanceData->ammo;
					weaponState->chamberedRoundMP.second.ammoType = currentAmmo;
					//+++
					UInt64 ammoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, currentAmmo);
					if (ammoCount < instanceData->ammoCapacity)
						eqData->loadedAmmoCount = ammoCount + 1;
					else
						eqData->loadedAmmoCount = instanceData->ammoCapacity + 1;

				}
				else if (weaponState->chamberedRoundMP.first.ammoType && weaponState->chamberedRoundMP.first.magazineCount > 0)
				{
					TESAmmo* currentAmmo = instanceData->ammo;
					instanceData->ammo = weaponState->chamberedRoundMP.first.ammoType;
					eqData->ammo = instanceData->ammo;
					weaponState->chamberedRoundMP.first.ammoType = currentAmmo;
					//+++
					UInt64 ammoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, currentAmmo);
					if (ammoCount < instanceData->ammoCapacity)
						eqData->loadedAmmoCount = ammoCount + 1;
					else
						eqData->loadedAmmoCount = instanceData->ammoCapacity + 1;
				}
			}
		}
	}

	void ClearChamberedRound(bool clearSecondary) //only if hasKW chamberedEnable/secChamberedEnable
	{
		ExtraDataList* extraData = nullptr;
		(*g_player)->GetEquippedExtraData(41, &extraData);
		if (extraData)
		{
			ExtraWeaponState* weaponState = (ExtraWeaponState*)extraData->GetByType(ExtraWeaponState::kType_ExtraWeaponState);
			EquipWeaponData* eqData = (EquipWeaponData*)(*g_player)->middleProcess->unk08->equipData->equippedData;
			TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast((*g_player)->middleProcess->unk08->equipData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
			if (eqData && instanceData && weaponState)
			{
				if (clearSecondary)
				{
					instanceData->ammo = weaponState->chamberedRoundMP.second.ammoType;
					eqData->ammo = instanceData->ammo;
					//+++
					weaponState->chamberedRoundMP.second.ammoType = nullptr;
					weaponState->chamberedRoundMP.second.magazineCount = 0;
				}
				else
				{
					instanceData->ammo = weaponState->chamberedRoundMP.first.ammoType;
					eqData->ammo = instanceData->ammo;
					//+++
					weaponState->chamberedRoundMP.first.ammoType = nullptr;
					weaponState->chamberedRoundMP.first.magazineCount = 0;
				}
			}
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
