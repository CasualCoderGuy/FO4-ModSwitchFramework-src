#include "MSF_WeaponState.h"
#include "MSF_Base.h"


ModData::Mod ExtraWeaponState::defaultStatePlaceholder;

bool BurstModeManager::SetState(UInt8 bActive)
{
	flags ^= (-(bActive & BurstModeManager::bActive) ^ flags) & BurstModeManager::bActive;
	return true;
}

bool BurstModeManager::HandleFireEvent()
{
	InterlockedIncrement16(&numOfShotsFired);
	if (numOfShotsFired < numOfTotalShots)
	{
		if (!(flags & BurstModeData::bTypeAuto))
			delayTask delayNextShot(delayTime, true, &BurstModeManager::FireWeapon, this);
	}
	else
	{
		if (flags & BurstModeData::bTypeAuto)
			Utilities::PlayIdleAction(*g_player, MSF_MainData::ActionRightRelease);
		InterlockedExchange16(&numOfShotsFired, 0);
	}
	return true;
}

bool BurstModeManager::ResetShotsOnReload()
{
	if (flags & BurstModeData::bResetShotCountOnReload)
		InterlockedExchange16(&numOfShotsFired, 0);
	return true;
}

bool BurstModeManager::HandleReleaseEvent()
{
	if ((flags & BurstModeData::bResetShotCountOnRelease) && (flags & BurstModeData::bTypeAuto))
		InterlockedExchange16(&numOfShotsFired, 0);
	return true;
}

bool BurstModeManager::FireWeapon()
{
	if (!(flags & BurstModeData::bOnePullBurst))
	{
		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
		{
			if (flags & BurstModeData::bResetShotCountOnRelease)
				InterlockedExchange16(&numOfShotsFired, 0);
			return true;
		}
	}
	Actor* player = *g_player;
	TESIdleForm* fireIdle = MSF_Data::GetFireAnimation(player);
	if (fireIdle)
		return Utilities::PlayIdle(player, fireIdle);
	return false;
}

bool BurstModeManager::HandleEquipEvent(TESObjectWEAP::InstanceData* weaponInstance) //ExtraDataList* extraDataList
{
	InterlockedExchange16(&numOfShotsFired, 0);
	if (!weaponInstance)
	{
		flags &= ~BurstModeData::bActive;
		return false;
	}
	if (Utilities::WeaponInstanceHasKeyword(weaponInstance, MSF_MainData::FiringModBurstKW))
		flags |= BurstModeData::bActive;
	else
		flags &= ~BurstModeData::bActive;
	return true;
}

bool BurstModeManager::HandleModChangeEvent(ExtraDataList* extraDataList)
{
	InterlockedExchange16(&numOfShotsFired, 0);
	if (!extraDataList)
	{
		flags &= ~BurstModeData::bActive;
		return false;
	}
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModBurstKW))
		flags |= BurstModeData::bActive;
	else
		flags &= ~BurstModeData::bActive;
	return true;
}

ExtraWeaponState::ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	//parent.extraList = extraDataList;
	//parent.formID = extraInstanceData->baseForm->formID;
	this->ID = MSF_MainData::weaponStateStore.Add(this);
	ExtraRank* holder = ExtraRank::Create(ID);
	this->holder = holder;
	extraDataList->Add(ExtraDataType::kExtraData_Rank, holder);
	//setweaponstate: init(noSwitch?),loadSave,beforeSwitchAmmo,afterSwitchAmmo,beforeSwitchMod,afterSwitchMod,fireWeaponAfterAmmoSwitch,fireWeaponAfterReload;;validate mod-weapstate pairs
}

ExtraWeaponState::~ExtraWeaponState()
{
	for (auto it = weaponStates.begin(); it != weaponStates.end(); it++)
	{
		WeaponState* state = it->second;
		delete state;
		it->second = nullptr;
	}
	weaponStates.clear();
	delete burstModeManager;
}

ExtraWeaponState* ExtraWeaponState::Init(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (!extraDataList || !equipData || !equipData->ammo)
		return nullptr;
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm)
		return nullptr;
	if (extraDataList->HasType(ExtraDataType::kExtraData_Rank))
	{
		ExtraRank* holder = (ExtraRank*)extraDataList->GetByType(kExtraData_Rank);
		return MSF_MainData::weaponStateStore.Get(holder->rank);
	}
	//ExtraWeaponState* state = MSF_MainData::weaponStateStore.GetValid(attachedMods->unk14);
	//if (state)
	//	return state;
	return new ExtraWeaponState(extraDataList, equipData);
}

ExtraWeaponState::WeaponState::WeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData) //on load game: scan all extra rank, extra rank compare hook, equip hook, mod hook
{
	//create new ID and class instance
	//add ID and class instance pair to map
	//create new extra rankx with ID
	//add extra rank to ExtraDataList
}


//bool ExtraWeaponState::SetWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, bool temporary)
//{
//	if (!extraDataList || !equipData || !equipData->ammo)
//		return false;//clear
//	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
//	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
//	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
//	if (!attachedMods || !currInstanceData)
//		return false;//clear
//
//	UInt64 priority = 0;
//	BGSMod::Attachment::Mod* activeStateMod = nullptr;
//
//	auto data = attachedMods->data;
//	if (!data || !data->forms)
//		return false;
//	for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
//	{
//		BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
//		UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
//		for (UInt32 j = 0; j < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); j++)
//		{
//			BGSMod::Container::Data * data = &objectMod->modContainer.data[j];
//			if (data->target == 31 && data->value.form)
//			{
//				BGSKeyword* modKeyword = (BGSKeyword*)data->value.form;
//				if (modKeyword == MSF_MainData::hasUniqueStateKW)
//				{
//					if (currPriority >= priority)
//					{
//						activeStateMod = objectMod;
//						break;
//					}
//				}
//			}
//		}
//	}
//	auto it = std::find_if(weaponStates.begin(), weaponStates.end(), [activeStateMod](std::tuple<BGSMod::Attachment::Mod*, UInt64, WeaponState*, WeaponState*> state){
//		return std::get<0>(state) == activeStateMod;
//	});
//	WeaponState* weaponState = new WeaponState(currInstanceData);
//	if (it != weaponStates.end())
//	{
//		if (temporary)
//		{
//			WeaponState* oldState = std::get<2>(*it);
//			std::get<2>(*it) = weaponState;
//			delete oldState;
//		}
//		else
//		{
//			WeaponState* oldState = std::get<3>(*it);
//			std::get<3>(*it) = weaponState;
//			delete oldState;
//		}
//	}
//	else
//	{
//		if (temporary)
//			weaponStates.push_back(std::make_tuple(activeStateMod, equipData->loadedAmmoCount, weaponState, nullptr));
//		else
//			weaponStates.push_back(std::make_tuple(activeStateMod, equipData->loadedAmmoCount, nullptr, weaponState));
//	}
//	return true;
//}
//
//bool ExtraWeaponState::RecoverTemporaryState(ExtraDataList* extraDataList, EquipWeaponData* equipData)
//{
//
//}
//
//bool ExtraWeaponState::SetCurrentStateTemporary()
//{
//
//}

bool HandleWeaponStateEvents(UInt8 eventType)
{
	Actor* player = *g_player;
	ExtraDataList* equippedWeapExtraData = nullptr;
	player->GetEquippedExtraData(41, &equippedWeapExtraData);
	EquipWeaponData* equippedData = (EquipWeaponData*)Utilities::GetEquippedWeaponData(player);
	ExtraWeaponState* weaponState = ExtraWeaponState::Init(equippedWeapExtraData, equippedData);
}


bool ExtraWeaponState::HandleEquipEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (!extraDataList || !equipData || !equipData->ammo)
		return false;
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm)
		return false;
	if (this->currentState)
	{
		equipData->loadedAmmoCount = this->currentState->loadedAmmo;
		if (!Utilities::HasObjectMod(attachedMods, this->currentState->currentSwitchedAmmo->mod))//validate
		{
			//attach
		}
	}
	//add state?
}

bool ExtraWeaponState::HandleFireEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (this->currentState)
	{
		this->currentState->loadedAmmo = equipData->loadedAmmoCount;
		this->currentState->shotCount++;
		if (this->currentState->switchToAmmoAfterFire && (this->currentState->shotCount >= this->currentState->chamberSize))
		{
			//evaluate
			if (MSF_Base::AttachModToEquippedWeapon(*g_player, this->currentState->switchToAmmoAfterFire->mod, true, 0, false))
			{
				this->currentState->currentSwitchedAmmo = this->currentState->switchToAmmoAfterFire;
				this->currentState->switchToAmmoAfterFire = nullptr;
				MSF_Scaleform::UpdateWidgetData();
			}
			//error handle
		}
	}
	//add state?
}

bool ExtraWeaponState::HandleReloadEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType) //0:start,1:BCR,2:complete
{
	if (this->currentState)
	{
		equipData->loadedAmmoCount = this->currentState->ammoCapacity + this->currentState->chamberSize; //BCR!
		this->currentState->loadedAmmo = equipData->loadedAmmoCount;
	}
	//add state?
}

bool ExtraWeaponState::HandleModChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{

}

//bool ExtraWeaponState::SetParentRef(ObjectRefHandle refHandle)
//{
//	//validate refHandle
//	parent.refHandle = refHandle;
//	parent.extraList = nullptr;
//	return true;
//}
//
//bool ExtraWeaponState::SetParentInvItem(ExtraDataList* extraList)
//{
//	if (!extraList)
//		return false;
//	parent.refHandle = 0;
//	parent.extraList = extraList;
//	return true;
//}
//
//bool ExtraWeaponState::ValidateParent()
//{
//	if (parent.extraList)
//	{
//		ExtraDataList* extraList = dynamic_cast<ExtraDataList*>(parent.extraList);
//		if (!extraList)
//			return false;
//		BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
//		if (!attachedMods)
//			return false;
//		if (this == MSF_MainData::weaponStateStore.Get(attachedMods->unk14))
//			return true;
//		return false;
//	}
//	else 
//	{
//		return false;
//	}
//}

/*
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
				weaponState = new ExtraWeaponState();
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
				weaponState = new ExtraWeaponState();
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
*/

/*
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
			data = new BurstMode();
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
*/