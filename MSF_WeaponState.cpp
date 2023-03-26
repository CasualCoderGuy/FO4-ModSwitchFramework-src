#include "MSF_WeaponState.h"

ExtraWeaponState::ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	type = kType_ExtraWeaponState;
	unk10 = 0;
	unk13 = 0;
	unk14 = 0;
	next = NULL;
	extraDataList->Add(kType_ExtraWeaponState, this);
}

ExtraWeaponState::~ExtraWeaponState()
{
	for (auto it = weaponStates.begin(); it != weaponStates.end(); it++)
	{
		delete std::get<1>(*it);
		std::get<1>(*it) = nullptr;
		delete std::get<2>(*it);
		std::get<2>(*it) = nullptr;
	}
	weaponStates.clear();
}

ExtraWeaponState* ExtraWeaponState::Init(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (!extraDataList || !equipData || !equipData->ammo)
		return nullptr;
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData)
		return nullptr;
	return new ExtraWeaponState(extraDataList, equipData);
}

ExtraWeaponState::WeaponState::WeaponState(TESObjectWEAP::InstanceData* instanceData, UInt64 loadedAmmoCount)
{
	magazineCount = loadedAmmoCount;
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
	auto it = std::find_if(weaponStates.begin(), weaponStates.end(), [activeStateMod](std::tuple<BGSMod::Attachment::Mod*, WeaponState*, WeaponState*> state){
		return std::get<0>(state) == activeStateMod;
	});
	WeaponState* weaponState = new WeaponState(currInstanceData, equipData->loadedAmmoCount);
	if (it != weaponStates.end())
	{
		if (temporary)
		{
			WeaponState* oldState = std::get<1>(*it);
			std::get<1>(*it) = weaponState;
			delete oldState;
		}
		else
		{
			WeaponState* oldState = std::get<2>(*it);
			std::get<2>(*it) = weaponState;
			delete oldState;
		}
	}
	else
	{
		if (temporary)
			weaponStates.push_back(std::make_tuple(activeStateMod, weaponState, nullptr));
		else
			weaponStates.push_back(std::make_tuple(activeStateMod, nullptr, weaponState));
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