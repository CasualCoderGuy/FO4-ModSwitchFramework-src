#include "MSF_WeaponState.h"

ExtraWeaponState::ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, DataHolderParentInstance &instance)
{
	type = kType_ExtraWeaponState;
	unk10 = 0;
	unk13 = 0;
	unk14 = 0;
	next = NULL;
	//extraDataList->Add(kType_ExtraWeaponState, this);
	//this can cause unexpected ctd, we will use BGSObjectInstanceExtra::unk14 padding as unique ID store instead
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	//parent.extraList = extraDataList;
	//parent.formID = extraInstanceData->baseForm->formID;
	parent = instance;
	attachedMods->unk14 = MSF_MainData::weaponStateStore.Add(this);
	//setweaponstate: init(noSwitch?),loadSave,beforeSwitchAmmo,afterSwitchAmmo,beforeSwitchMod,afterSwitchMod,fireWeaponAfterAmmoSwitch,fireWeaponAfterReload;;validate mod-weapstate pairs
}

ExtraWeaponState::~ExtraWeaponState()
{
	for (auto it = weaponStates.begin(); it != weaponStates.end(); it++)
	{
		delete std::get<2>(*it);
		std::get<2>(*it) = nullptr;
		delete std::get<3>(*it);
		std::get<3>(*it) = nullptr;
	}
	weaponStates.clear();
	delete burstModeManager;
}

ExtraWeaponState* ExtraWeaponState::Init(ExtraDataList* extraDataList, EquipWeaponData* equipData, DataHolderParentInstance &instance)
{
	if (!extraDataList || !equipData || !equipData->ammo)
		return nullptr;
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm)
		return nullptr;
	//if (extraDataList->HasType(ExtraWeaponState::kType_ExtraWeaponState))
	//	return (ExtraWeaponState*)extraDataList->GetByType(ExtraWeaponState::kType_ExtraWeaponState);
	ExtraWeaponState* state = MSF_MainData::weaponStateStore.GetValid(attachedMods->unk14);
	if (state)
		return state;
	return new ExtraWeaponState(extraDataList, equipData, instance);
}

ExtraWeaponState::WeaponState::WeaponState(TESObjectWEAP::InstanceData* instanceData)
{
	if (!instanceData)
		return;
	ammoCapacity = instanceData->ammoCapacity;
	ammoType = instanceData->ammo;
	impactDataSet = instanceData->unk58; //unk58
	projectileOverride = instanceData->firingData->projectileOverride;
	critEffect = instanceData->unk78; //unk78
	baseDamage = instanceData->baseDamage;
	numProjectiles = instanceData->firingData->numProjectiles;
	secondary = instanceData->secondary;
	critDamageMult = instanceData->critDamageMult;
	minRange = instanceData->minRange;
	maxRange = instanceData->maxRange;
	outOfRangeMultiplier = instanceData->outOfRangeMultiplier;
	stagger = instanceData->stagger;
	if (instanceData->damageTypes)
	{
		UInt32 dtCount = instanceData->damageTypes->count;
		if (dtCount && instanceData->damageTypes->entries)
		{
			damageTypes.Allocate(dtCount);
			if (damageTypes.entries)
				memcpy(&damageTypes.entries, &instanceData->damageTypes->entries, dtCount*sizeof(TBO_InstanceData::DamageTypes));
		}
	}
}

ExtraWeaponState::WeaponState::~WeaponState()
{
	damageTypes.Clear();
}

bool ExtraWeaponState::SetWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, bool temporary)
{
	if (!extraDataList || !equipData || !equipData->ammo)
		return false;//clear
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData)
		return false;//clear

	UInt64 priority = 0;
	BGSMod::Attachment::Mod* activeStateMod = nullptr;

	auto data = attachedMods->data;
	if (!data || !data->forms)
		return false;
	for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
	{
		BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
		UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
		for (UInt32 j = 0; j < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); j++)
		{
			BGSMod::Container::Data * data = &objectMod->modContainer.data[j];
			if (data->target == 31 && data->value.form)
			{
				BGSKeyword* modKeyword = (BGSKeyword*)data->value.form;
				if (modKeyword == MSF_MainData::hasUniqueStateKW)
				{
					if (currPriority >= priority)
					{
						activeStateMod = objectMod;
						break;
					}
				}
			}
		}
	}
	auto it = std::find_if(weaponStates.begin(), weaponStates.end(), [activeStateMod](std::tuple<BGSMod::Attachment::Mod*, UInt64, WeaponState*, WeaponState*> state){
		return std::get<0>(state) == activeStateMod;
	});
	WeaponState* weaponState = new WeaponState(currInstanceData);
	if (it != weaponStates.end())
	{
		if (temporary)
		{
			WeaponState* oldState = std::get<2>(*it);
			std::get<2>(*it) = weaponState;
			delete oldState;
		}
		else
		{
			WeaponState* oldState = std::get<3>(*it);
			std::get<3>(*it) = weaponState;
			delete oldState;
		}
	}
	else
	{
		if (temporary)
			weaponStates.push_back(std::make_tuple(activeStateMod, equipData->loadedAmmoCount, weaponState, nullptr));
		else
			weaponStates.push_back(std::make_tuple(activeStateMod, equipData->loadedAmmoCount, nullptr, weaponState));
	}
	return true;
}

bool ExtraWeaponState::RecoverTemporaryState(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{

}

bool ExtraWeaponState::SetCurrentStateTemporary()
{

}

bool ExtraWeaponState::HandleFireEvent()
{

}

bool ExtraWeaponState::HandleReloadEvent()
{

}

bool ExtraWeaponState::HandleModChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{

}

bool BurstModeManager::SetState(UInt8 bActive)
{
	flags ^= (-(bActive & BurstModeManager::bActive) ^ flags) & BurstModeManager::bActive;
}

bool ExtraWeaponState::SetParentRef(ObjectRefHandle refHandle)
{
	//validate refHandle
	parent.refHandle = refHandle;
	parent.extraList = nullptr;
	return true;
}

bool ExtraWeaponState::SetParentInvItem(ExtraDataList* extraList)
{
	if (!extraList)
		return false;
	parent.refHandle = 0;
	parent.extraList = extraList;
	return true;
}

bool ExtraWeaponState::ValidateParent()
{
	if (parent.extraList)
	{
		ExtraDataList* extraList = dynamic_cast<ExtraDataList*>(parent.extraList);
		if (!extraList)
			return false;
		BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		if (!attachedMods)
			return false;
		if (this == MSF_MainData::weaponStateStore.Get(attachedMods->unk14))
			return true;
		return false;
	}
	else 
	{
		return false;
	}
}

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