#include "MSF_Base.h"
#include "MSF_Events.h"
#include "MSF_WeaponState.h"
#include "MSF_Localization.h"
#include "MSF_Test.h"

namespace MSF_Base
{
	//========================== Main Functions ===========================

	//FROM SCALEFORM:
	bool SwitchToSelectedAmmo(AmmoData::AmmoMod* selectedAmmo, BGSSoundDescriptorForm* sound)//(void* obj)
	{
		_DEBUG("selectCalled");
		//AmmoData::AmmoMod* selectedAmmo = reinterpret_cast<AmmoData::AmmoMod*>(obj);
		if (!selectedAmmo)
			return false;
		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled) )// || (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
			{
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
				return false;
			}
		}
		Actor* playerActor = *g_player;

		_DEBUG("checksPassed");
		SwitchData* switchData = new SwitchData();
		BGSMod::Attachment::Mod* mod = selectedAmmo->mod;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		BGSMod::Attachment::Mod* attachedMod = Utilities::GetModAtAttachPoint(modData, MSF_MainData::ammoAP);
		switchData->targetAmmo = selectedAmmo->ammo;
		switchData->SwitchFlags = selectedAmmo->flags & AmmoData::AmmoMod::mBitTransferMask;
		switchData->soundToPlay = sound;
		_DEBUG("mod: %p", mod);
		if (mod == attachedMod)
		{
			PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo, false, switchData->soundToPlay);
			delete switchData;
			return true;
		}
		if (mod)
			switchData->ModToAttach = mod;
		else
		{
			mod = attachedMod;
			_DEBUG("modatAP: %p", mod);
			if (mod)
				switchData->ModToRemove = mod;
			else
			{
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
				delete switchData;
				return false;
			}
		}

		_DEBUG("modOK");
		return SwitchAmmoCommon(switchData);
	}

	//FROM HOTKEY (or fire anim):
	bool SwitchAmmoHotkey(UInt8 key, BGSSoundDescriptorForm* sound, bool ignoreAnim, bool requireAmmo)
	{
		Actor* playerActor = *g_player;
		_DEBUG("queueCount: %i; state: %02X; unk08: %08X; flags: %08X", MSF_MainData::modSwitchManager.GetQueueCount(), MSF_MainData::modSwitchManager.GetState(), playerActor->actorState.unk08, playerActor->actorState.flags);

		if (!ignoreAnim && (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))// || (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled))
		{
			if (!MSF_Base::HandlePendingAnimations())
			{
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
				return false;
			}
		}
		_DEBUG("animOK");

		SwitchData* switchData = nullptr;
		if (key & KeybindData::bToggle)
			switchData = MSF_Data::GetNextAmmoMod(false, requireAmmo);
		else
			switchData = MSF_Data::GetNthAmmoMod(key);
		if (!switchData)
		{
			PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
			return false;
		}
		switchData->soundToPlay = sound;
		_DEBUG("ammoSwitchDataOK");
		return SwitchAmmoCommon(switchData);
	}

	bool SwitchAmmoCommon(SwitchData* switchData)
	{
		if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled))
		{
			BGSSoundDescriptorForm* sound = switchData->soundToPlay;
			switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
			if (MSF_Base::SwitchMod(switchData, true))
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo, false, sound);
			else
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
			//if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
			//	Utilities::DrawWeapon(*g_player);
			return true;
		}

		if (MSF_MainData::modSwitchManager.GetQueueCount() > 0 || MSF_MainData::modSwitchManager.GetState() != 0)
		{
			PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
			delete switchData;
			return false;
		}
		_DEBUG("queue/stateOK");

		Actor* playerActor = *g_player;
		TESObjectWEAP::InstanceData* instance = Utilities::GetEquippedInstanceData(playerActor);
		bool isBCR = MSF_Data::InstanceHasBCRSupport(instance);
		bool isTR = MSF_Data::InstanceHasTRSupport(instance) || MSF_WeaponState::EquippedWeaponHasTRSupport(playerActor);
		if (isBCR)
		{
			switchData->SwitchFlags |= SwitchData::bReloadZeroCount;
			UInt32 targetAmmoCount = 1; //0?
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
				BGSSoundDescriptorForm* sound = switchData->soundToPlay;
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				if (MSF_Base::SwitchMod(switchData, true))
					PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo, false, sound);
				else
					PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
				if (targetAmmoCount && playerActor->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
					MSF_Base::ReloadWeapon(false, true);
				return true;
			}
		}
		else if (isTR)
			switchData->SwitchFlags |= SwitchData::bReloadFull;

		if (playerActor->actorState.IsWeaponDrawn() && MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
		{
			if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadCompatibilityMode || switchData->SwitchFlags & SwitchData::bDoSwitchBeforeAnimations)
			{
				BGSSoundDescriptorForm* sound = switchData->soundToPlay;
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				if (MSF_Base::SwitchMod(switchData, true))
					PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo, false, sound);
				else
					PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
				MSF_Base::ReloadWeapon(isTR && !isBCR, isBCR);
				return true;
			}
			_DEBUG("toReload");
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress); // | SwitchData::bReloadNotFinished
			BGSSoundDescriptorForm* sound = switchData->soundToPlay;
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			UInt64 invammo = Utilities::GetInventoryItemCount(playerActor->inventoryList, switchData->targetAmmo);
			Utilities::SetAnimationVariableInt(playerActor, "NextReloadAmmoCount", invammo > instance->ammoCapacity ? instance->ammoCapacity : invammo);
			if (!MSF_Base::ReloadWeapon(switchData->SwitchFlags & SwitchData::bReloadFull, switchData->SwitchFlags & SwitchData::bReloadZeroCount))
			{
				MSF_MainData::modSwitchManager.ClearQueue();
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
			}
			else
				PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo, false, sound); //SOUND:onreload?
			//MSF_Base::SwitchMod(switchData, true);
			return true;
		}
		else if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
		{
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNeeded);
			BGSSoundDescriptorForm* sound = switchData->soundToPlay;
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			Utilities::DrawWeapon(playerActor);
			PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo, false, sound); //SOUND:ondraw?
			//delay check draw state
			return true;
		}
		PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail, 1, nullptr);
		delete switchData;
		return false;
	}



	const char* EquipAmmoPipboy(TESAmmo* ammo, bool bEquip)
	{
		if (!bEquip)
			return MSF_Localization::GetTranslation(MSF_Localization::Keys::ammoUnequipText);
		if (MSF_Base::IsNotSupportedAmmo(ammo))
			return MSF_Localization::GetTranslation(MSF_Localization::Keys::unsupportedAmmoText);
		SwitchData* switchData = MSF_Data::GetModForAmmo(ammo);
		if (!switchData)
			return MSF_Localization::GetTranslation(MSF_Localization::Keys::noAmmoTypeText);
		switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
		if (!MSF_Base::SwitchMod(switchData, true))
			return MSF_Localization::GetTranslation(MSF_Localization::Keys::ammoEquipFailedText);
		ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeReload, *g_player, ExtraWeaponState::bEventTypeReloadInventory, 0);
		MSF_Base::EquipAmmo((*g_player)->inventoryList, ammo);
		PlayEquipSound(*g_player, ammo, 1, 0);
		return nullptr;
	}

	//FROM SCALEFORM:
	bool SwitchToSelectedMod(ModData::Mod* modToAttach, ModData::Mod* modToRemove, BGSSoundDescriptorForm* sound)//(void* modDataToAttach, void* modDataToRemove, bool bNeedInit)
	{
		if (modToAttach == modToRemove)
			return false;

		//ModData::Mod* modToAttach = reinterpret_cast<ModData::Mod*>(modDataToAttach);
		//ModData::Mod* modToRemove = reinterpret_cast<ModData::Mod*>(modDataToRemove);

		//if (!MSF_Data::CheckSwitchRequirements(stack, modToAttach, modToRemove))
		//	return false;

		return MSF_Data::QueueModsToSwitch(modToAttach, modToRemove, sound);
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

	const char* EquipModPipboy(TESObjectMISC* miscMod, bool bEquip) //FindModsForLooseMod{ 2197524 };
	{
		return MSF_Localization::GetTranslation(MSF_Localization::Keys::modText);
		auto omod = MSF_MainData::miscModMap.find(miscMod);
		if (omod == MSF_MainData::miscModMap.end())
			return MSF_Localization::GetTranslation(MSF_Localization::Keys::modText);
		auto itMod = MSF_MainData::modDataMap.find(omod->second);
		Actor* playerActor = *g_player;
		if (itMod == MSF_MainData::modDataMap.end())
		{
			BGSObjectInstanceExtra* oie = Utilities::GetEquippedModData(playerActor);
			TESObjectWEAP* baseWeap = Utilities::GetEquippedWeapon(playerActor);
			if (!oie || !baseWeap)
				return MSF_Localization::GetTranslation(MSF_Localization::Keys::noWeapText);
			if (omod->second->unkC0 && !Utilities::ObjectInstanceHasAttachPoint(oie, omod->second->unkC0) && !Utilities::HasAttachPoint((AttachParentArray*)&baseWeap->attachParentArray, omod->second->unkC0))
				return MSF_Localization::GetTranslation(MSF_Localization::Keys::noAPText);
			BGSMod::Attachment::Mod* oldomod = Utilities::GetModAtAttachPoint(oie, omod->second->unkC0);
			TESObjectMISC* oldlmod = nullptr;
			if (oldomod)
				oldlmod = Utilities::GetLooseMod(oldomod);
			if (!AttachModToEquippedWeapon(playerActor, omod->second, true, 0, true, true))
				return MSF_Localization::GetTranslation(MSF_Localization::Keys::couldntAttachText);
			PlayEquipSound(*g_player, omod->first, 1, 0);
			Utilities::RemoveItem(playerActor, omod->first, 1, true);
			if (oldlmod)
				Utilities::AddItem(playerActor, oldlmod, 1, true);
		}
		else
		{
			auto itKB = MSF_MainData::keybindAPMap.find(omod->second->unkC0);
			if (itKB == MSF_MainData::keybindAPMap.end())
				return MSF_Localization::GetTranslation(MSF_Localization::Keys::noAPText);
			ModData* modData = itKB->second->modData;
			BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedWeaponStack(playerActor);
			ModData::Mod* oldMod = MSF_Data::GetOldModForEquip(eqStack, modData, itMod->second);
			if (!oldMod)
				return MSF_Localization::GetTranslation(MSF_Localization::Keys::noAPText);
			TESObjectMISC* oldlmod = Utilities::GetLooseMod(oldMod->mod);
			if (!AttachModToEquippedWeapon(playerActor, omod->second, true, 0, true, true))
				return MSF_Localization::GetTranslation(MSF_Localization::Keys::couldntAttachText);;
			PlayEquipSound(*g_player, omod->first, 1, 0);
			Utilities::RemoveItem(playerActor, omod->first, 1, true);
			if (oldlmod)
				Utilities::AddItem(playerActor, oldlmod, 1, true);
		}
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
			bool success = false;
			bool isAmmo = switchData->targetAmmo != nullptr;
			BGSSoundDescriptorForm* sound = switchData->soundToPlay;
			if (switchData->SwitchFlags & SwitchData::bIgnoreAnimations)
				success = MSF_Base::SwitchMod(switchData, true);
			else if ((*g_player)->actorState.IsWeaponDrawn() || (switchData->SwitchFlags & ModData::Mod::bNotRequireWeaponToBeDrawn))
			{
				if (switchData->SwitchFlags & SwitchData::bReloadNeeded)
				{
					UInt32 flags = switchData->SwitchFlags;
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadCompatibilityMode || switchData->SwitchFlags & SwitchData::bDoSwitchBeforeAnimations)
					{
						switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
						success = MSF_Base::SwitchMod(switchData, true);
					}
					else
					{
						switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bReloadInProgress | ~SwitchData::bReloadNeeded); // | SwitchData::bReloadNotFinished
						flags = switchData->SwitchFlags;
					}
					if (!MSF_Base::ReloadWeapon(flags & SwitchData::bReloadFull, flags & SwitchData::bReloadZeroCount))
						MSF_MainData::modSwitchManager.ClearQueue();
					else
						success = true;
				}
				else if (switchData->SwitchFlags & SwitchData::bAnimNeeded)
				{
					UInt32 flags = switchData->SwitchFlags;
					AnimationData* anim = switchData->animData;
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimCompatibilityMode || switchData->SwitchFlags & SwitchData::bDoSwitchBeforeAnimations)
					{
						switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
						success = MSF_Base::SwitchMod(switchData, true);
					}
					else
					{
						switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress | ~SwitchData::bAnimNeeded);
						flags = switchData->SwitchFlags;
					}
					if (!MSF_Base::PlayAnim(anim))
						MSF_MainData::modSwitchManager.ClearQueue(); //MSF_Base::SwitchMod(switchData, true);
					else
						success = true;
				}
			}
			else if (switchData->SwitchFlags & SwitchData::bDrawEnabled)
			{
				switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress);
				Utilities::DrawWeapon(*g_player);
				success = true;
				//delay check draw state
			}
			bool playSound = (!isAmmo &&(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundModFail)) || (isAmmo && (MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail));
			if (!success)
				MSF_Base::PlayFeedbackSound((!isAmmo && (MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundModFail)) || (isAmmo && (MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmoFail)), 1, nullptr);
			//else //SOUND:on anim success?
			//	MSF_Base::PlayFeedbackSound((!isAmmo && (MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundMod)) || (isAmmo && (MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundAmmo)), false, sound);

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
			bool bPlayFastEquipAnim = false;
			if (!modToAttach)
			{
				if (flags & SwitchData::bUpdateAnimGraph)
					bUpdateAnimGraph = true;
				if (flags & SwitchData::bPlayFastEquipAnim)
					bPlayFastEquipAnim = true;
			}
			if (!AttachModToEquippedWeapon(playerActor, modToRemove, false, 2, bUpdateAnimGraph, bPlayFastEquipAnim))
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
			if (!AttachModToEquippedWeapon(playerActor, modToAttach, true, 2, (flags & SwitchData::bUpdateAnimGraph) != 0, (flags & SwitchData::bPlayFastEquipAnim) != 0))
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

	void PrintStackData(Actor* actor, TESObjectWEAP* baseWeap)
	{
#ifndef DEBUG
		return;
#endif
		if (!baseWeap)
			return;
		if (!actor || !actor->inventoryList)
			return;
		BipedAnim* equipData = actor->biped.get();
		if (equipData)
		{
			auto modList = equipData->object[41].modExtra;
			UInt32 modCount = 0;
			if (modList && modList->data)
				modCount = modList->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
			_MESSAGE("invItem: %p, eqInstance: %p, modList: %p, modCount: %i", Utilities::GetEquippedInventoryWeapon(actor), equipData->object[41].parent.instanceData, modList, modCount);
		}
		for (UInt32 i = 0; i < actor->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			actor->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != baseWeap)
				continue;
			if (!inventoryItem.stack)
				_MESSAGE("no stack");
			_MESSAGE("invitem ID: %i", i);
			UInt32 stackID = 0;
			for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
			{
				if (!stack->extraData)
				{
					_MESSAGE("stackID: %i, count: %i, flags: %04X, stack: %p, no extra data", stackID, stack->count, stack->flags, stack);
					continue;
				}
				BSExtraData* extraMods = stack->extraData->GetByType(kExtraData_ObjectInstance);
				BGSObjectInstanceExtra* modList = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
				UInt32 modCount = 0;
				if (modList && modList->data)
					modCount = modList->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
				ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				TBO_InstanceData* iData = nullptr;
				if (extraInstanceData)
					iData = extraInstanceData->instanceData;
				UInt32 weapStateID = 0;
				ExtraRank* extraHolder = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_Rank), BSExtraData, ExtraRank);
				if (extraHolder)
					weapStateID = extraHolder->rank;
				_MESSAGE("stackID: %i, count: %i, flags: %04X, stack: %p, extraList: %p, modList: %p, instance: %p, holder: %p, stateID: %i, modCount: %i", stackID, stack->count, stack->flags, stack, stack->extraData, modList, iData, extraHolder, weapStateID, modCount);
				stackID++;
			}
		}

	}

	bool AttachModToEquippedWeapon(Actor* actor, BGSMod::Attachment::Mod* mod, bool bAttach, UInt8 modLoadedAmmoCount, bool updateAnimGraph, bool playFastEquipAnim)
	{
		_DEBUG("attach, updAnim: %02X", updateAnimGraph);
		//MSF_Test::DoLog(actor);
		//BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(actor, 41);
		//BGSInventoryItem::Stack* stack = Utilities::GetEquippedWeaponStack(actor);
		BGSInventoryItem* eqItem = Utilities::GetEquippedInventoryWeapon(actor);
		if (!eqItem || !eqItem->stack)
			return false;
		BGSInventoryItem::Stack* stack = eqItem->stack;
		//for (BGSInventoryItem::Stack* istack = stack; stack; stack->next)
		//{
		//	if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
		//		stack = istack;
		//}
		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return false;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* modList = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!modList || !extraInstanceData)
			return false;
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;
		BGSInventoryItem* item = nullptr;
		for (UInt32 i = 0; i < actor->inventoryList->items.count; i++)
		{
			if (actor->inventoryList->items.entries[i].form == (TESForm*)weapBase)
				item = &actor->inventoryList->items.entries[i];
		}
		if (!item)
			return false;

		_DEBUG("AcheckOK");

		PrintStackData(actor, weapBase);

		std::vector<TBO_InstanceData::ValueModifier> avifValues;
		GetActorValues((TESObjectWEAP::InstanceData*)extraInstanceData->instanceData, &avifValues);

		bool ret = false;

		CheckStackIDFunctor IDfunctor(0); //Utilities::GetStackID(item, stack)
		ModifyModDataFunctor modFunctor(mod, 0, bAttach , &ret);
		modFunctor.transferEquippedToSplitStack = true;

		UInt32 unk = 0x00200000;
		MSF_MainData::modSwitchManager.GetSetChangeAmmo((1 << (UInt8)modFunctor.transferEquippedToSplitStack) | (UInt16((MSF_MainData::MCMSettingFlags & MSF_MainData::bRandomizeLoadedAmmoOnSplitStack) / MSF_MainData::bRandomizeLoadedAmmoOnSplitStack) << 2));
		AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);
		MSF_MainData::modSwitchManager.GetSetChangeAmmo(0);
		PrintStackData(actor, weapBase);

		BGSInventoryItem::Stack* newStack = eqItem->stack;
		ExtraDataList* newList = newStack->extraData;
		BSExtraData* newExtraMods = newList->GetByType(kExtraData_ObjectInstance);
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
			modifyModFunctor.shouldSplitStacks = false;
			//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachRemoveModStack(item, &CheckStackIDFunctor(0), &modifyModFunctor, 0, &unk); //Utilities::GetStackID(item, stack)
		}
		PrintStackData(actor, weapBase);

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

		//get splitted stack; init a new rank/extrastate; do it with workbench & others
		std::vector<std::pair<BGSMod::Attachment::Mod*, bool>> stateModsToModify;
		ExtraWeaponState::HandleModChangeEvent(newList, &stateModsToModify, ExtraWeaponState::kEventTypeModdedSwitch, dataList);
		for (auto nextPair : stateModsToModify)
		{
			_DEBUG("target state/ammomod: %08X, rem: %02X", nextPair.first->formID, nextPair.second);

			unk = 0x00200000;
			bool success = false;
			ModifyModDataFunctor modifyModFunctor(nextPair.first, 0, nextPair.second, &success);
			modifyModFunctor.shouldSplitStacks = false;
			//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachRemoveModStack(item, &CheckStackIDFunctor(0), &modifyModFunctor, 0, &unk); //Utilities::GetStackID(item, stack)
		}

		PrintStackData(actor, weapBase);

		extraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData)
			return false;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);

		BGSObjectInstance idStruct;
		idStruct.object = weapBase;
		idStruct.instanceData = instanceData;

		//MSF_Test::DoLog(actor);
		//_DEBUG("mod ammo count1: %i", eqData->loadedAmmoCount);
		MSF_MainData::modSwitchManager.SetModChangeEvent(true);
		//if (updateAnimGraph)
			MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(true);
		//else
		//	MSF_MainData::modSwitchManager.SetIgnoreEquipAction(true);
		MSF_MainData::modSwitchManager.SetDontPutYourGunIn(true);
		EquipItemInternal(g_ActorEquipManager, actor, idStruct, 0, 1, nullptr, 0, 0, 0, 1, 0);
		MSF_MainData::modSwitchManager.SetDontPutYourGunIn(false);

		PatchActorValues(actor, instanceData, &avifValues);

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
		{
			_DEBUG("UpdateAnimationGraph");
			//PlayerCharacter* player = *g_player;
			//MSF_Test::FollowAnimationFor(player, 300, 10);
			if (!playFastEquipAnim)
				MSF_MainData::modSwitchManager.SetIgnoreEquipAction(true); //jiggleAfterMoveEffectMedium, playFastEquipSound
			UpdateAnimGraph(actor, false);
			//MSF_Test::DoLog(player);


			//InitializeActorInstant(actor, 0);
			//if (player == actor)
			//{
				//BGSAction* act = (BGSAction*)LookupFormByID(0x2ADF1);
				//_MESSAGE("updAnim %p", act);
			//	UpdateAnimation(player, 0.2f);
			//}
			//Utilities::PlayIdleAction(actor, act);
			//0x18BA8 IdleStop
			//0x2ADF1 ForceEquip
			//0x2FFA9 InitGraph
			//0x5704C InstantInitGraph
			//0x7F8E3 InstantIdleStop
			
		}
		//else
		//	MSF_MainData::modSwitchManager.SetIgnoreEquipAction(false);




		//UnkSub_DFE930(actor, false);
		/*
		PrintStackData(actor, weapBase);
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
		}*/
		PrintStackData(actor, weapBase);
		EquipWeaponData* newEqData = (EquipWeaponData*)actor->middleProcess->unk08->equipData[0].equippedData;
		TESAmmo* newAmmoType = newEqData->ammo;
		_DEBUG("mod ammo count2: %i", newEqData->loadedAmmoCount);
		ExtraWeaponState* weaponState = ExtraWeaponState::Init(newList, nullptr);
		if (weaponState)
			weaponState->HandleEquipEvent(newList, newEqData);
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

	bool AttachRemoveModToInventoryStack(TESObjectREFR* owner, BGSInventoryItem* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph, bool playFastEquipAnim)
	{
		if (!owner || !item || !mod || !item->form || !item->stack)
			return false;
		BGSInventoryItem::Stack* stack = Utilities::GetStack(item, stackID);
		if (!stack)
			return false;

		ExtraDataList* dataList = stack->extraData;
		//if (!dataList)
		//	return false;
		ExtraInstanceData* extraInstanceData = nullptr;
		if (dataList)
			extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		//if (!extraInstanceData || !extraInstanceData->baseForm || !extraInstanceData->instanceData)
		//	return false;

		Actor* ownerActor = nullptr;
		bool isEquipped = false;
		std::vector<TBO_InstanceData::ValueModifier> avifValues;
		if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
		{
			isEquipped = true;
			ownerActor = DYNAMIC_CAST(owner, TESObjectREFR, Actor);
			if (extraInstanceData && extraInstanceData->instanceData)
				GetActorValues((TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData), &avifValues);
		}

		bool ret = false;

		CheckStackIDFunctor IDfunctor(stackID);
		ModifyModDataFunctor modFunctor(mod, 0, bAttach, &ret);
		modFunctor.shouldSplitStacks = shouldSplitStacks;
		modFunctor.transferEquippedToSplitStack = transferEquippedToSplitStack;
		if (extraInstanceData)
			PrintStackData((Actor*)owner, (TESObjectWEAP*)extraInstanceData->baseForm);
		UInt32 unk = 0x00200000;
		MSF_MainData::modSwitchManager.GetSetChangeAmmo((1 << (UInt8)transferEquippedToSplitStack) | (UInt16((MSF_MainData::MCMSettingFlags & MSF_MainData::bRandomizeLoadedAmmoOnSplitStack) / MSF_MainData::bRandomizeLoadedAmmoOnSplitStack) << 2));
		AttachRemoveModStack(item, &IDfunctor, &modFunctor, 0, &unk);
		MSF_MainData::modSwitchManager.GetSetChangeAmmo(0);

		if (shouldSplitStacks)
		{
			if (isEquipped)
			{
				stack = item->stack;
				if (!transferEquippedToSplitStack && item->stack->next)
				{
					volatile short* equipFlagPtr = (volatile short*)&item->stack->flags;
					InterlockedExchange16(equipFlagPtr, 0);
					void* next1 = InterlockedExchangePointer((void* volatile*)&stack->next, item->stack);
					void* next0 = InterlockedExchangePointer((void* volatile*)&item->stack->next, next1);
					void* start = InterlockedExchangePointer((void* volatile*)&item->stack, next0);
				}
			}
			else
			{
				stackID++;
				UInt32 currID = 0;
				for (BGSInventoryItem::Stack* newStack = item->stack; stack; stack->next)
				{
					if (stackID == currID)
					{
						stack = newStack;
						break;
					}
					currID++;
				}
			}
		}

		ExtraDataList* newList = stack->extraData;
		if (!newList)
			return ret;
		extraInstanceData = DYNAMIC_CAST(newList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData || !extraInstanceData->baseForm || !extraInstanceData->instanceData)
			return ret;

		TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
		if (baseWeap)
		{
			PrintStackData((Actor*)owner, (TESObjectWEAP*)extraInstanceData->baseForm);
			BSExtraData* newExtraMods = newList->GetByType(kExtraData_ObjectInstance);
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
				modifyModFunctor.shouldSplitStacks = false;
				//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
				AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
			}
			std::vector<std::pair<BGSMod::Attachment::Mod*, bool>> stateModsToModify;
			ExtraWeaponState::HandleModChangeEvent(newList, &stateModsToModify, ExtraWeaponState::kEventTypeModdedSwitch, dataList);
			PrintStackData((Actor*)owner, (TESObjectWEAP*)extraInstanceData->baseForm);
			for (auto nextPair : stateModsToModify)
			{
				unk = 0x00200000;
				bool success = false;
				ModifyModDataFunctor modifyModFunctor(nextPair.first, 0, nextPair.second, &success);
				modifyModFunctor.shouldSplitStacks = false;
				//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
				AttachRemoveModStack(item, &CheckStackIDFunctor(Utilities::GetStackID(item, stack)), &modifyModFunctor, 0, &unk);
			}
		}
		PrintStackData((Actor*)owner, (TESObjectWEAP*)extraInstanceData->baseForm);
		if (isEquipped && ownerActor)
		{
			if (!transferEquippedToSplitStack)
			{
				extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				newList = dataList;
			}
			BGSObjectInstance idStruct;
			idStruct.object = extraInstanceData->baseForm;
			idStruct.instanceData = extraInstanceData->instanceData;

			UInt8 slot = Utilities::GetEquippedWeaponSlotIndex(ownerActor);
			if (baseWeap && slot != BIPOBJECT::kNone)
			{
				MSF_MainData::modSwitchManager.SetModChangeEvent(true);
				MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(true);
				MSF_MainData::modSwitchManager.SetDontPutYourGunIn(true);
				EquipItemInternal(g_ActorEquipManager, ownerActor, idStruct, 0, 1, nullptr, 0, 0, 0, 1, 0);
				MSF_MainData::modSwitchManager.SetDontPutYourGunIn(false);

				if (updateAnimGraph)
					UpdateAnimGraph(ownerActor, false);
				PrintStackData((Actor*)owner, (TESObjectWEAP*)extraInstanceData->baseForm);
				PatchActorValues(ownerActor, (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData), &avifValues);

				EquipWeaponData* newEqData = (EquipWeaponData*)ownerActor->middleProcess->unk08->equipData[0].equippedData;
				ExtraWeaponState* weaponState = ExtraWeaponState::Init(newList, nullptr);
				if (weaponState)
					weaponState->HandleEquipEvent(newList, newEqData);
			}
			else
			{
				UnequipItemInternal(g_ActorEquipManager, ownerActor, idStruct, 1, nullptr, -1, 0, 0, 0, 0, nullptr);
				EquipItemInternal(g_ActorEquipManager, ownerActor, idStruct, stackID, 1, nullptr, 1, 0, 0, 0, 0);
				PatchActorValues(ownerActor, (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData), &avifValues);
			}

			/*
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
			PrintStackData((Actor*)owner, (TESObjectWEAP*)extraInstanceData->baseForm);
			*/
		}

		return true;
	}

	bool AttachRemoveModToEquippedItem(Actor* actor, UInt8 slotIndex, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph, bool playFastEquipAnim)
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
		_DEBUG("raider checks ok");
		for (UInt32 i = 0; i < actor->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			actor->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			UInt32 stackID = 0;
			bool hasMultiple = inventoryItem.stack->next != nullptr;
			for (BGSInventoryItem::Stack* currStack = inventoryItem.stack; currStack; currStack = currStack->next)
			{
				if (!hasMultiple || (currStack->flags & BGSInventoryItem::Stack::kFlagEquipped))
					return AttachRemoveModToInventoryStack(actor, &inventoryItem, stackID, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph, playFastEquipAnim);
				else if (inventoryItem.stack->extraData)
				{
					ExtraInstanceData* instance = (ExtraInstanceData*)inventoryItem.stack->extraData->GetByType(ExtraDataType::kExtraData_InstanceData);
					if (instance && actor->biped.get()->object[slotIndex].parent.instanceData == instance->instanceData)
						return AttachRemoveModToInventoryStack(actor, &inventoryItem, stackID, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph, playFastEquipAnim);
				}
				stackID++;
				_DEBUG("raider stack not equipped");
			}
		}
		return false;
	}

	bool AttachRemoveModToInventoryStackPre(TESObjectREFR* owner, TESBoundObject* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph, bool playFastEquipAnim)
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
				return AttachRemoveModToInventoryStack(owner, &inventoryItem, stackID, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph, playFastEquipAnim);
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
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(mods, baseWeap);
		auto itBaseAmmo = MSF_MainData::ammoDataMap.end();
		if (baseAmmo)
			itBaseAmmo = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itBaseAmmo != MSF_MainData::ammoDataMap.end())
		{
			if (targetAmmo == itBaseAmmo->second->baseAmmoData.ammo)
				targetMod = itBaseAmmo->second->baseAmmoData.mod;
			else
			{
				auto itAmmo = std::find_if(itBaseAmmo->second->ammoMods.begin(), itBaseAmmo->second->ammoMods.end(), [targetAmmo](AmmoData::AmmoMod& data) { //TEST IT!!!
					return data.ammo == targetAmmo;
					});
				if (itAmmo != itBaseAmmo->second->ammoMods.end())
					targetMod = itAmmo->mod;
			}
		}
		//auto itAmmo = MSF_MainData::ammoMap.find(targetAmmo);
		//if (itAmmo != MSF_MainData::ammoMap.end())
		//	targetMod = itAmmo->second->mod;
		if (!ammoMod && !targetMod)
			return true;
		_DEBUG("targetAmmo: %08X", targetAmmo->formID);
		if (baseAmmo)
			_DEBUG("baseAmmo: %08X", baseAmmo->formID);
		if (ammoMod)
			_DEBUG("ammoMod: %08X", ammoMod->formID);
		if (targetMod)
			_DEBUG("targetMod: %08X", targetMod->formID);
		//TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(mods, baseWeap);
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
		//auto itBaseAmmo = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itBaseAmmo != MSF_MainData::ammoDataMap.end())
		{
			auto ammoData = (*itBaseAmmo).second;
			if (targetMod == ammoData->baseAmmoData.mod)
			{
				_DEBUG("FindInvalid: new base ammo, restore switched base ammo");  //new: targetAmmo (state change) //base-mod-OK?
				*modResult = targetMod;
				*finalAmmo = ammoData->baseAmmoData.ammo;
				*bAttach = true;
				return true;
			}
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
		if (MSF_MainData::MCMSettingFlags & (MSF_MainData::bSpawnRandomAmmo | MSF_MainData::bSpawnRandomMods | MSF_MainData::bReplaceAmmoWithSpawned))
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
					TESAmmo* baseAmmo = randomActor->middleProcess->unk08->equipData[0].equippedData->ammo;
					TESAmmo* ammo = baseAmmo;
					std::vector<BGSMod::Attachment::Mod*> mods;
					UInt32 count = 0;
					MSF_Data::PickRandomMods(&mods, &ammo, &count);
					_DEBUG("picked ammo: %p, %i", ammo, count);
					if (ammo)
					{
						if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReplaceAmmoWithSpawned)
						{
							count = Utilities::GetInventoryItemCount(randomActor->inventoryList, baseAmmo);
							if (!count)
								continue;
							Utilities::RemoveItem(randomActor, baseAmmo, count, true);
						}
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
		/*
		namespace PipboyDataManager
		{
			inline constexpr REL::ID Singleton{ 4796372 };
		}

		namespace PipboyDataGroup
		{
			inline constexpr REL::ID LockDataGroup{ 2225147 };
			inline constexpr REL::ID UnlockDataGroup{ 2225148 };
		}

		namespace PipboyInventoryData
		{
			inline constexpr REL::ID AddItemCardInfoEntry{ 2225267 };
			inline constexpr REL::ID BaseAddItemCardInfoEntry{ 2225270 };
			inline constexpr REL::ID InitializeItem{ 2225264 };
			inline constexpr REL::ID PopulateItemCardInfo{ 2225266 };
			inline constexpr REL::ID QueueItemCardRepopulate{ 2225311 };
			inline constexpr REL::ID RepopulateItemCardOnSection{ 2225279 };
			namespace PipboyInventoryMenu
			{
				PipboyInventoryMenu UpdateData(this){ 2224143 };


				PipboyInventoryData
				virtual BSEventNotifyControl ProcessEvent(const ActorEquipManagerEvent::Event& a_event, BSTEventSource<ActorEquipManagerEvent::Event>* a_source);                            // 03

				*/

		if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowEquippedAmmo))
			return true;
		_DEBUG("ammoEq");
		uintptr_t** pipboyInventoryData = (uintptr_t**)((uintptr_t)*g_pipboyDataMgr + 0x358);
		typedef EventResult(*_EquipEventProcess)(void* pipboyInventoryData, ActorEquipManagerEvent::Event* evn, BSTEventDispatcher<ActorEquipManagerEvent::Event>* a_source);
		_EquipEventProcess PipboyEquipProcessEvnFn = (_EquipEventProcess)*(*pipboyInventoryData + 3);
		_DEBUG("pipDataMgr: %p, pipInvMenu: %p, pipEqProcFn: %p, ", *g_pipboyDataMgr, pipboyInventoryData, *(*pipboyInventoryData + 3)); //BFB0
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
				UInt32 stackID = 0;
				for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
				{
					InterlockedExchange16((volatile short*)&stack->flags, 1);
					//ActorEquipManagerEvent::Event evn;
					//BGSObjectInstance obj;
					//obj.object = ammo;
					//evn.data = &obj;
					//evn.stackID = stackID;
					//evn.unequip = false;
					//_DEBUG("ammoEq2: %s", ammo->GetFullName());
					//PipboyEquipProcessEvnFn(pipboyInventoryData, &evn, &actorEquipManagerEventSourceMSF);
					//_DEBUG("ammoEqOK");
					//stackID++;
				}
			}
			else
			{
				UInt32 stackID = 0;
				for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
				{
					if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped) //&& not fusioncore
					{
						InterlockedExchange16((volatile short*)&stack->flags, 0);
						//ActorEquipManagerEvent::Event evn;
						//BGSObjectInstance obj;
						//obj.object = inventoryItem.form;
						//evn.data = &obj;
						//evn.stackID = stackID;
						//evn.unequip = false;
						//_DEBUG("ammoUEq2: %s", inventoryItem.form->GetFullName());
						//PipboyEquipProcessEvnFn(pipboyInventoryData, &evn, &actorEquipManagerEventSourceMSF);
						//_DEBUG("ammoUEqOK");
						//stackID++;
					}
				}
			}
		}
		//typedef void* (*_PipboyInventoryMenu_UpdateData)(void* pipboyInventoryMenu);
		//RelocAddr <_PipboyInventoryMenu_UpdateData> PipboyInventoryMenu_UpdateData(0xB8D5C0); //0xADEF10
		//uintptr_t pipboyInventoryMenu = (uintptr_t)(*g_ui)->GetMenu(BSFixedString("PipboyMenu"));
		//if (pipboyInventoryMenu)
		//{
		//	pipboyInventoryMenu += 0x160;
		//	_DEBUG("ammoUpd");
		//	PipboyInventoryMenu_UpdateData((void*)pipboyInventoryMenu);
		//	_DEBUG("ammoUpdOK");
		//}
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

	void PatchActorValues(TESObjectREFR* ref, TESObjectWEAP::InstanceData* newInstance, std::vector<TBO_InstanceData::ValueModifier>* avifValues)
	{
		if (!ref || !newInstance || !avifValues || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bPatchVanillaAVcalculation))
			return;
		if (!newInstance->modifiers || newInstance->modifiers->count == 0)
		{
			if (avifValues->size() == 0)
				return;
			for (auto mod : *avifValues)
				ref->actorValueOwner.Mod(0, mod.avInfo, -((float)(mod.unk08)));
		}
		else
		{
			for (UInt32 i = 0; i < newInstance->modifiers->count; i++)
			{
				TBO_InstanceData::ValueModifier* mod = &(newInstance->modifiers->entries[i]);
				ActorValueInfo* avif = mod->avInfo;
				auto itAVIF = std::find_if(avifValues->begin(), avifValues->end(), [avif](TBO_InstanceData::ValueModifier& data) {
					return data.avInfo == avif;
				});
				if (itAVIF == avifValues->end())
					ref->actorValueOwner.Mod(0, mod->avInfo, (float)(mod->unk08));
				else
					ref->actorValueOwner.Mod(0, mod->avInfo, ((float)(mod->unk08)) - ((float)(itAVIF->unk08)));
			}
		}
	}

	void GetActorValues(TESObjectWEAP::InstanceData* instance, std::vector<TBO_InstanceData::ValueModifier>* avifValues)
	{
		if (!instance || !avifValues || !instance->modifiers || instance->modifiers->count == 0)
			return;
		for (UInt32 i = 0; i < instance->modifiers->count; i++)
		{
			TBO_InstanceData::ValueModifier mod;
			instance->modifiers->GetNthItem(i, mod);
			avifValues->push_back(mod);
		}
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
		if (clearAmmoCount || MSF_MainData::MCMSettingFlags & MSF_MainData::bEmptyClipBeforeSwitch)
		{
			EquipWeaponData* eqData = Utilities::GetEquippedWeaponData(playerActor);
			eqData->loadedAmmoCount = 0;
		}
		if (!full || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bEnableTacticalReloadAnim))
		{
			if (isSwitch)
				Utilities::SetAnimationVariableInt(playerActor, "SwitchAmmoTypeReload", 1);
			MSF_MainData::modSwitchManager.SetForcedReload(forced);
			bool success = Utilities::PlayIdleAction(playerActor, MSF_MainData::ActionReload);
			_DEBUG("reloadsuccess: %i", success);
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
			_DEBUG("reloadsuccess: %i", success);
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

	bool PlayFeedbackSound(bool play, UInt8 type, BGSSoundDescriptorForm* success) // type: 0: success, 1: failGeneral, 2: failQuickkey, 3: failMenu
	{
		if (play)
		{
			_DEBUG("playSound: %i", type);
			if (type == 1)
				return Utilities::PlaySoundInternal(MSF_MainData::failSound, *g_player);
			if (type == 2)
				return Utilities::PlaySoundInternal(MSF_MainData::failSoundQuickkey, *g_player);
			if (type == 3)
				return Utilities::PlaySoundInternal(MSF_MainData::failSoundMenu, *g_player);
			else if (success)
				return Utilities::PlaySoundInternal(success, *g_player);
		}
		return false;
	}
}
