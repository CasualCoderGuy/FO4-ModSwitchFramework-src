#include "MSF_WeaponState.h"
#include "MSF_Base.h"
#include "MSF_Events.h"

ModData::Mod ExtraWeaponState::defaultStatePlaceholder;

ExtraWeaponState::ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	//BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	//parent.extraList = extraDataList;
	//parent.formID = extraInstanceData->baseForm->formID;
	this->ID = MSF_MainData::weaponStateStore.Add(this);
	if (extraDataList->HasType(kExtraData_Rank))
	{
		ExtraRank* extraHolder = (ExtraRank*)extraDataList->GetByType(kExtraData_Rank);
		extraHolder->rank = this->ID;
		this->holder = extraHolder;
	}
	else
	{
		ExtraRank* extraHolder = ExtraRank::Create(ID);
		this->holder = extraHolder;
		extraDataList->Add(ExtraDataType::kExtraData_Rank, extraHolder);
	}
	this->UpdateWeaponStates(extraDataList, equipData);
}

ExtraWeaponState::ExtraWeaponState(ExtraRank* extraHolder)
{
	this->ID = MSF_MainData::weaponStateStore.Add(this);
	extraHolder->rank = this->ID;
	this->holder = extraHolder;
	this->currentState = nullptr;
}

ExtraWeaponState::WeaponState::WeaponState(UInt16 newflags, UInt16 newammoCapacity, UInt16 newchamberSize, UInt16 newshotCount, UInt64 newloadedAmmo, TESAmmo* newchamberedAmmo, std::vector<TESAmmo*>* newBCRammo)
{
	this->flags = newflags;
	this->ammoCapacity = newammoCapacity;
	this->chamberSize = newchamberSize;
	this->shotCount = newshotCount;
	this->loadedAmmo = newloadedAmmo;
	this->chamberedAmmo = newchamberedAmmo;
	if (newBCRammo)
		this->BCRammo = *newBCRammo;
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
	//delete burstModeManager;
}

ExtraWeaponState* ExtraWeaponState::Init(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	//if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::mMakeExtraRankMask))
	//	return nullptr;
	//_DEBUG("WS_init");
	if (!extraDataList) // || !equipData || !equipData->ammo
		return nullptr;
	//_DEBUG("WS_inputok");
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	if (!extraInstanceData)
		return nullptr;
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm || !currInstanceData->ammo)
		return nullptr;
	//_DEBUG("WS_instanceok");
	if (extraDataList->HasType(ExtraDataType::kExtraData_Rank))
	{
		ExtraRank* holder = (ExtraRank*)extraDataList->GetByType(kExtraData_Rank);
		ExtraWeaponState* storedState = MSF_MainData::weaponStateStore.Get(holder->rank);
		_DEBUG("WS rank: %08X, state: %p", holder->rank, storedState);
		if (storedState)
			return storedState;
		else
			return new ExtraWeaponState(extraDataList, equipData);
	}
	return new ExtraWeaponState(extraDataList, equipData);
}

ExtraWeaponState::WeaponState::WeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData) //on load game: scan all extra rank, extra rank compare hook, equip hook, mod hook
{
	this->FillData(extraDataList,equipData);
}

bool ExtraWeaponState::WeaponState::FillData(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (equipData)
		this->loadedAmmo = equipData->loadedAmmoCount;
	else
		this->loadedAmmo = 0;
	this->chamberSize = 0;
	this->chamberedAmmo = nullptr;
	this->ammoCapacity = 0;
	this->flags = 0;
	this->shotCount = 0;
	//has BCR?
	//has tactical reload?
	//get chamber size
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!currInstanceData || !baseWeap || !attachedMods)
		return false;
	//BGSMod::Attachment::Mod* receiver = Utilities::GetModAtAttachPoint(attachedMods, MSF_MainData::receiverAP);
	MSF_Data::GetChamberData(attachedMods, &this->chamberSize, &this->flags);
	this->chamberedAmmo = currInstanceData->ammo;
	this->ammoCapacity = currInstanceData->ammoCapacity;
	if (!equipData)
		this->loadedAmmo = this->ammoCapacity;
	//BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	//if (!attachedMods)
	//	return;
	//BGSMod::Attachment::Mod* ammoOmod = Utilities::GetModAtAttachPoint(attachedMods, MSF_MainData::ammoAP);
	//if (!ammoOmod)
	//	return;
	//auto itammomod = MSF_MainData::ammoModMap.find(ammoOmod);
	//currentSwitchedAmmo = itammomod->second;
	return true;
}

bool ExtraWeaponState::HandleWeaponStateEvents(UInt8 eventType, Actor* actor)
{
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::mMakeExtraRankMask))
		return true;
	if (!actor)
		actor = *g_player;
	ExtraDataList* equippedWeapExtraData = nullptr;
	actor->GetEquippedExtraData(41, &equippedWeapExtraData);
	EquipWeaponData* equippedData = Utilities::GetEquippedWeaponData(actor);
	ExtraWeaponState* weaponState = ExtraWeaponState::Init(equippedWeapExtraData, equippedData);
	if (!weaponState)
		return false;
	bool ret = false;
	//_DEBUG("eqSwitch");
	switch (eventType)
	{
	case ExtraWeaponState::kEventTypeEquip: //ret = weaponState->HandleEquipEvent(equippedWeapExtraData, equippedData); break;
	{
		if (MSF_MainData::modSwitchManager.GetModChangeEvent())
		{
			ret = weaponState->HandleEquipEvent(equippedWeapExtraData, equippedData); //HandleModChangeEvent
			MSF_MainData::modSwitchManager.SetModChangeEvent(false);
		}
		else
			ret = weaponState->HandleEquipEvent(equippedWeapExtraData, equippedData);
	}
	break;
	case ExtraWeaponState::KEventTypeAmmoCount: ret = weaponState->HandleAmmoChangeEvent(equippedWeapExtraData, equippedData); break;
	case ExtraWeaponState::KEventTypeFireWeapon: ret = weaponState->HandleFireEvent(equippedWeapExtraData, equippedData); break;
	case ExtraWeaponState::KEventTypeReload: ret = weaponState->HandleReloadEvent(equippedWeapExtraData, equippedData, eventType); break;
	case ExtraWeaponState::KEventTypeModded: ret = weaponState->HandleModChangeEvent(equippedWeapExtraData, equippedData); break;
	default:
		return false; break;
	}
	return ret;
}

bool ExtraWeaponState::UpdateWeaponStates(ExtraDataList* extraDataList, EquipWeaponData* equipData, TESAmmo* newAmmo)
{
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	if (!attachedMods)
		return false;
	auto data = attachedMods->data;
	if (!data || !data->forms)
		return false;
	UInt64 priority = 0;
	BGSMod::Attachment::Mod* currModData = nullptr;
	for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
	{
		BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
		UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
		if (currPriority < priority)
			continue;
		auto itmoddata = MSF_MainData::modDataMap.find(objectMod);
		if (itmoddata == MSF_MainData::modDataMap.end())
			continue;
		ModData::Mod* moddata = itmoddata->second;
		if (!(moddata->flags & ModData::Mod::bHasSecondaryAmmo))
			continue;
		currModData = objectMod;
		priority = currPriority;
	}
	auto itstate = this->weaponStates.find(currModData); //might not work
	if (itstate == this->weaponStates.end())
	{
		WeaponState* newState = new ExtraWeaponState::WeaponState(extraDataList, equipData);
		this->weaponStates[currModData] = newState;
		this->currentState = newState;
		_DEBUG("NEWstate");
	}
	else if (this->currentState != itstate->second)
	{
		this->currentState = itstate->second;
		_DEBUG("UPDstate");
	}
	else if (newAmmo)
	{
		this->currentState->chamberedAmmo = newAmmo;
		_DEBUG("AMMOstate");
	}
	return true;
}


bool ExtraWeaponState::HandleEquipEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	_DEBUG("equipState");
	if (!extraDataList || !equipData || !equipData->ammo)
		return false;
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	if (!extraInstanceData)
		return false;
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm)
		return false;
	if (this->currentState)
	{
		_DEBUG("eq: %i, stored: %i", equipData->loadedAmmoCount, this->currentState->loadedAmmo);
		UInt32 inventoryAmmo = Utilities::GetInventoryItemCount((*g_player)->inventoryList, equipData->ammo);
		if (inventoryAmmo < this->currentState->loadedAmmo)
		{
			this->currentState->loadedAmmo = inventoryAmmo;
			equipData->loadedAmmoCount = inventoryAmmo;
		}
		else
			equipData->loadedAmmoCount = this->currentState->loadedAmmo;
		_DEBUG("eq: %i, stored: %i", equipData->loadedAmmoCount, this->currentState->loadedAmmo);
		//if (!Utilities::HasObjectMod(attachedMods, this->currentState->currentSwitchedAmmo->mod))//validate
		//{
		//	//attach
		//}
	}
	else
		this->UpdateWeaponStates(extraDataList, equipData); //should not reach this
	return true;
}

bool ExtraWeaponState::HandleFireEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	//if (!this->currentState)
	//	this->UpdateWeaponStates(extraDataList, equipData);
	//_DEBUG("fireWeaponState");
	//if (this->currentState->switchToAmmoAfterFire)
	//{
	//	this->currentState->shotCount++;
	//	if (this->currentState->shotCount >= this->currentState->chamberSize)
	//	{
	//		//evaluate
	//		if (MSF_Base::AttachModToEquippedWeapon(*g_player, this->currentState->switchToAmmoAfterFire->mod, true, 0, false))
	//		{
	//			this->currentState->currentSwitchedAmmo = this->currentState->switchToAmmoAfterFire;
	//			this->currentState->switchToAmmoAfterFire = nullptr;
	//			this->currentState->shotCount = 0;
	//			MSF_Scaleform::UpdateWidgetData();
	//		}
	//		//error handle
	//	}
	//}
	return true;
}

bool ExtraWeaponState::HandleAmmoChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (!this->currentState)
		this->UpdateWeaponStates(extraDataList, equipData);
	this->currentState->loadedAmmo = equipData->loadedAmmoCount;
	//_DEBUG("ammoChangeState");
	//if (equipData->loadedAmmoCount == 0 && this->currentState->switchToAmmoAfterFire)
	//{
	//	//evaluate
	//	if (MSF_Base::AttachModToEquippedWeapon(*g_player, this->currentState->switchToAmmoAfterFire->mod, true, 0, false))
	//	{
	//		this->currentState->currentSwitchedAmmo = this->currentState->switchToAmmoAfterFire;
	//		this->currentState->switchToAmmoAfterFire = nullptr;
	//		this->currentState->shotCount = 0;
	//		MSF_Scaleform::UpdateWidgetData();
	//	}
	//	//error handle
	//}
	return true;
}

bool ExtraWeaponState::HandleReloadEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType) //0:start,1:BCR,2:complete
{
	//_DEBUG("reloadState");
	//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	//TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
	//BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	//TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	//if (!currInstanceData || !baseWeap)
	//	return false;
	//if (!this->currentState)
	//	this->UpdateWeaponStates(extraDataList, equipData);
	//if (this->currentState->flags & ExtraWeaponState::WeaponState::bHasTacticalReload)
	//{
	//	equipData->loadedAmmoCount = currInstanceData->ammoCapacity + this->currentState->chamberSize; //if chamberedRounds < chamberSize ?
	//	this->currentState->loadedAmmo = equipData->loadedAmmoCount;
	//}

	return true;
}

bool ExtraWeaponState::HandleModChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	//if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::mMakeExtraRankMask))
	//	return true;
	//ExtraWeaponState::UpdateWeaponStates(extraDataList, equipData);

	return true;
}

TESAmmo* ExtraWeaponState::GetAmmoForWorkbenchUI(ExtraDataList* extraList)
{
	if (!extraList)
		return nullptr;
	if (extraList->HasType(ExtraDataType::kExtraData_Rank))
	{
		ExtraRank* holder = (ExtraRank*)extraList->GetByType(kExtraData_Rank);
		ExtraWeaponState* storedState = MSF_MainData::weaponStateStore.Get(holder->rank);
		_DEBUG("WS rank UI: %08X, state: %p", holder->rank, storedState);
		if (!storedState)
			return nullptr;

		BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		if (!attachedMods)
			return nullptr;
		auto data = attachedMods->data;
		if (!data || !data->forms)
			return nullptr;
		UInt64 priority = 0;
		BGSMod::Attachment::Mod* currModData = nullptr;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
			if (currPriority < priority)
				continue;
			auto itmoddata = MSF_MainData::modDataMap.find(objectMod);
			if (itmoddata == MSF_MainData::modDataMap.end())
				continue;
			ModData::Mod* moddata = itmoddata->second;
			if (!(moddata->flags & ModData::Mod::bHasSecondaryAmmo))
				continue;
			currModData = objectMod;
			priority = currPriority;
		}
		auto itstate = storedState->weaponStates.find(currModData); //might not work
		if (itstate != storedState->weaponStates.end() && storedState->currentState != itstate->second)
			return itstate->second->chamberedAmmo;
	}
	return nullptr;
}

bool ExtraWeaponState::SetCurrentAmmo(TESAmmo* ammo)
{
	if (this->currentState && ammo)
		return this->currentState->chamberedAmmo = ammo;
	else
		return false;
}

TESAmmo* ExtraWeaponState::GetCurrentAmmo()
{
	if (this->currentState)
		return this->currentState->chamberedAmmo;
	else
		return nullptr;
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