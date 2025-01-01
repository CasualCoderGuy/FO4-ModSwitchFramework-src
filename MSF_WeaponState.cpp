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
	this->UpdateWeaponStates(extraDataList, equipData, kEventTypeUndefined);
}

ExtraWeaponState::ExtraWeaponState(ExtraRank* extraHolder)
{
	this->ID = MSF_MainData::weaponStateStore.Add(this);
	extraHolder->rank = this->ID;
	this->holder = extraHolder;
	this->currentState = nullptr;
}

ExtraWeaponState::WeaponState::WeaponState(UInt16 newflags, UInt16 newammoCapacity, UInt16 newchamberSize, UInt32 newchamberedCount, UInt16 newshotCount, UInt64 newloadedAmmo, TESAmmo* newchamberedAmmo, TESAmmo* newequippedAmmo, std::vector<TESAmmo*>* newBCRammo, std::vector<BGSMod::Attachment::Mod*>* newStateMods) {
	this->flags = newflags;
	this->ammoCapacity = newammoCapacity;
	this->chamberSize = newchamberSize;
	this->chamberedCount = newchamberedCount;
	this->shotCount = newshotCount;
	this->loadedAmmo = newloadedAmmo;
	this->chamberedAmmo = newchamberedAmmo;
	this->equippedAmmo = newequippedAmmo;
	if (newBCRammo)
		this->BCRammo = *newBCRammo;
	if (newStateMods)
		this->stateMods = *newStateMods;
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
#ifdef ENABLEWEAPONSTATESFORMELEE
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm || Utilities::WeaponInstanceHasKeyword(currInstanceData, MSF_MainData::GrenadeKW) || Utilities::WeaponInstanceHasKeyword(currInstanceData, MSF_MainData::MineKW)) // || !currInstanceData->ammo)
		return nullptr;
#else
	if (!attachedMods || !currInstanceData || !extraInstanceData->baseForm || !currInstanceData->ammo)
		return nullptr;
#endif

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

ExtraWeaponState::WeaponState* ExtraWeaponState::WeaponState::Clone()
{
	WeaponState* newState = new WeaponState();
	newState->flags = this->flags;
	newState->ammoCapacity = this->ammoCapacity;
	newState->chamberSize = this->chamberSize;
	newState->chamberedCount = this->chamberedCount;
	newState->shotCount = this->shotCount;
	newState->loadedAmmo = this->loadedAmmo;
	newState->chamberedAmmo = this->chamberedAmmo;
	newState->equippedAmmo = this->equippedAmmo;
	newState->BCRammo = this->BCRammo;
	newState->stateMods = this->stateMods;
	return newState;
}

ExtraWeaponState::WeaponState::WeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, ModData::Mod* currUniqueStateMod) //on load game: scan all extra rank, extra rank compare hook, equip hook, mod hook
{
	this->FillData(extraDataList, equipData, currUniqueStateMod);
}

bool ExtraWeaponState::WeaponState::FillData(ExtraDataList* extraDataList, EquipWeaponData* equipData, ModData::Mod* currUniqueStateMod)
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
	this->chamberedCount = 0;
	this->equippedAmmo = nullptr;
	this->BCRammo.clear();
	this->stateMods.clear();
	//has BCR?
	//has tactical reload?
	//get chamber size
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	if (!extraInstanceData)
		return false;
	TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!currInstanceData || !baseWeap || !attachedMods)
		return false;

	if (currUniqueStateMod && (currUniqueStateMod->flags & ModData::Mod::bHasUniqueState))
		MSF_Data::GetAttachedChildren(attachedMods, currUniqueStateMod->mod, &this->stateMods, true); //these are only the valid mods

	if (currInstanceData->ammo)
	{
		//BGSMod::Attachment::Mod* receiver = Utilities::GetModAtAttachPoint(attachedMods, MSF_MainData::receiverAP);
		MSF_Data::GetChamberData(attachedMods, currInstanceData, &this->chamberSize, &this->flags);
		this->chamberedAmmo = currInstanceData->ammo;
		this->equippedAmmo = currInstanceData->ammo;
		this->ammoCapacity = currInstanceData->ammoCapacity;
		if (!equipData)
			this->loadedAmmo = this->ammoCapacity;
		this->chamberedCount = this->loadedAmmo < this->chamberSize ? this->loadedAmmo : this->chamberSize;
		if (MSF_Data::InstanceHasTRSupport(currInstanceData))
			this->flags |= bHasTacticalReload;
		if (MSF_Data::InstanceHasBCRSupport(currInstanceData))
		{
			this->flags |= bHasBCR;
			for (UInt32 ammoIdx = 0; ammoIdx < this->loadedAmmo; ammoIdx++)
				this->BCRammo.push_back(this->equippedAmmo);
		}
		this->flags |= (MSF_Base::IsNotSupportedAmmo(currInstanceData->ammo) & mAmmoMask);
	}

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

bool ExtraWeaponState::HandleWeaponStateEvents(UInt8 eventType, Actor* actor, UInt8 eventSubtype, UInt32 oldLoadedAmmoCount)
{
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::mMakeExtraRankMask))
		return true;
	if (!actor)
		actor = *g_player;
	UInt8 slot = Utilities::GetEquippedWeaponSlotIndex(actor);
	ExtraDataList* equippedWeapExtraData = nullptr;
	actor->GetEquippedExtraData(slot, &equippedWeapExtraData);
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
	case ExtraWeaponState::kEventTypeAmmoCount: ret = weaponState->HandleAmmoChangeEvent(equippedWeapExtraData, equippedData); break;
	case ExtraWeaponState::kEventTypeFireWeapon: ret = weaponState->HandleFireEvent(equippedWeapExtraData, equippedData); break;
	case ExtraWeaponState::kEventTypeReload: ret = weaponState->HandleReloadEvent(equippedWeapExtraData, equippedData, eventSubtype, oldLoadedAmmoCount); break;
	//case ExtraWeaponState::KEventTypeModded: ret = weaponState->HandleModChangeEvent(equippedWeapExtraData, equippedData); break;
	default:
		return false; break;
	}
	return ret;
}

//std::unordered_map<BGSMod::Attachment::Mod*, UniqueState*>::iterator
//auto ExtraWeaponState::GetCurrentUniqueState(BGSObjectInstanceExtra* attachedMods)
//{
//	if (!attachedMods)
//		return MSF_MainData::modUniqueStateMap.end();
//	auto data = attachedMods->data;
//	if (!data || !data->forms)
//		return MSF_MainData::modUniqueStateMap.end();
//	UInt64 priority = 0;
//	auto state = MSF_MainData::modUniqueStateMap.end();
//	for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
//	{
//		BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
//		UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
//		if (currPriority < priority)
//			continue;
//		//auto itmoddata = MSF_MainData::modDataMap.find(objectMod);
//		//if (itmoddata == MSF_MainData::modDataMap.end())
//		//	continue;
//		//ModData::Mod* moddata = itmoddata->second;
//		//if (!(moddata->flags & ModData::Mod::bHasUniqueState))
//		//	continue;
//		auto itmodstate = MSF_MainData::modUniqueStateMap.find(objectMod);
//		if (itmodstate == MSF_MainData::modUniqueStateMap.end())
//			continue;
//		state = itmodstate;
//		priority = currPriority;
//	}
//	return state;
//}

ModData::Mod* ExtraWeaponState::GetCurrentUniqueStateMod(BGSObjectInstanceExtra* attachedMods)
{
	if (!attachedMods)
		return nullptr;
	auto data = attachedMods->data;
	if (!data || !data->forms)
		return nullptr;
	UInt64 priority = 0;
	ModData::Mod* stateMod = nullptr;
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
		if (!(moddata->flags & (ModData::Mod::bHasUniqueState | ModData::Mod::bHasSecondaryAmmo)))
			continue;
		stateMod = moddata;
		priority = currPriority;
	}
	return stateMod;
}

bool ExtraWeaponState::GetUniqueStateModsToModify(BGSObjectInstanceExtra* attachedMods, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify, std::pair<WeaponState*, WeaponState*>* stateChange)
{
	if (!modsToModify || !stateChange)
		return false;
	if (!stateChange->first || stateChange->second)
		return false;
	for (auto oldMod : stateChange->first->stateMods)
	{
		auto itNewMods = std::find_if(stateChange->second->stateMods.begin(), stateChange->second->stateMods.end(), [oldMod](BGSMod::Attachment::Mod* newMod) {
			return newMod->unkC0 == oldMod->unkC0;
			});
		if (itNewMods == stateChange->second->stateMods.end())
			modsToModify->push_back(std::pair<BGSMod::Attachment::Mod*, bool>(oldMod, false));
	}
	for (auto newMod : stateChange->first->stateMods)
	{
		modsToModify->push_back(std::pair<BGSMod::Attachment::Mod*, bool>(newMod, true));
	}
	return true;
}
bool ExtraWeaponState::WeaponState::UpdateAmmoState(ExtraDataList* extraDataList, BGSObjectInstanceExtra* attachedMods, UInt8 eventType, bool stateChange, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify)
{
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	if (!extraInstanceData)
		return false;
	TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	TESObjectWEAP* weapon = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
	if (!weapon || !instanceData || !instanceData->ammo)
		return false;
	UInt8 notSupportedAmmo = (MSF_Base::IsNotSupportedAmmo(instanceData->ammo) & WeaponState::mAmmoMask);
	this->flags |= notSupportedAmmo;
	if (notSupportedAmmo)
		return false;

	if (eventType != kEventTypeModdedWorkbench)
	{
		MSF_Data::GetChamberData(attachedMods, instanceData, &this->chamberSize, &this->flags);
		this->ammoCapacity = instanceData->ammoCapacity;
		//if (!equipData)
		//	statechange.second->loadedAmmo = statechange.second->ammoCapacity;
	}
	TESAmmo* targetAmmo = nullptr;
	bool hasBCR = MSF_Data::InstanceHasBCRSupport(instanceData);
	if (!hasBCR)
	{
		if (this->BCRammo.size() > 0 && (eventType != kEventTypeModdedWorkbench))
			this->BCRammo.clear();
		if (stateChange)
			targetAmmo = this->chamberedAmmo;
		else
			targetAmmo = instanceData->ammo;
	}
	else if (this->BCRammo.size() > 0)
	{
		//UInt32 idx = this->flags & ExtraWeaponState::WeaponState::bChamberLIFO ? this->BCRammo.size() - 1 : 0;
		//targetAmmo = this->BCRammo[idx];
		if (stateChange)
			targetAmmo = this->chamberedAmmo;
		else
			targetAmmo = instanceData->ammo;
	}
	else if (stateChange)
		targetAmmo = this->chamberedAmmo;
	else
		targetAmmo = instanceData->ammo;
	BGSMod::Attachment::Mod* targetAmmoMod = nullptr;
	TESAmmo* finalAmmo = nullptr;
	bool toAttach = false;
	MSF_Base::GetAmmoModToModify(attachedMods, targetAmmo, weapon, &finalAmmo, &targetAmmoMod, &toAttach);
	if (targetAmmoMod)
	{
		if (modsToModify)
			modsToModify->push_back(std::pair<BGSMod::Attachment::Mod*, bool>(targetAmmoMod, toAttach));
	}
	if (eventType == kEventTypeModdedSwitch)
	{
		this->chamberedAmmo = finalAmmo; //this->currentState->chamberedAmmo = instanceData->ammo;
		this->equippedAmmo = finalAmmo;
	}
	else if (eventType == kEventTypeModdedGameplay && this->chamberedAmmo != finalAmmo)
	{
		this->loadedAmmo = this->ammoCapacity; //!!!
		if (hasBCR)
		{
			this->BCRammo.clear();
			for (UInt32 ammoIdx = 0; ammoIdx < this->loadedAmmo; ammoIdx++)
				this->BCRammo.push_back(finalAmmo);
		}
		this->chamberedAmmo = finalAmmo;
		this->equippedAmmo = finalAmmo;
	}
		
	return true;
}

bool ExtraWeaponState::UpdateWeaponStates(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify)
{
	auto statechange = std::pair<WeaponState*, WeaponState*>(this->currentState, nullptr);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	ModData::Mod* currStateModData = GetCurrentUniqueStateMod(attachedMods);
	BGSMod::Attachment::Mod* currStateMod = nullptr;
	if (currStateModData)
		currStateMod = currStateModData->mod;
	auto itstate = this->weaponStates.find(currStateMod);
	if (itstate == this->weaponStates.end()) // this->currentState should be nullptr here: no update needed
	{
		if (eventType != kEventTypeModdedWorkbench)
		{
			statechange.second = new ExtraWeaponState::WeaponState(extraDataList, equipData, currStateModData);
			this->weaponStates[currStateMod] = statechange.second;
			this->currentState = statechange.second;
		}
		_DEBUG("NEWstate");
	}
	else if (this->currentState != itstate->second)
	{
		statechange.second = itstate->second;
		if (eventType != kEventTypeModdedWorkbench)
			this->currentState = itstate->second;

		if (currStateModData && (currStateModData->flags & ModData::Mod::bHasUniqueState))
			MSF_Data::GetAttachedChildren(attachedMods, currStateModData->mod, &statechange.second->stateMods, true); //these are only the valid mods
		this->GetUniqueStateModsToModify(attachedMods, modsToModify, &statechange);

		statechange.second->UpdateAmmoState(extraDataList, attachedMods, eventType, true, modsToModify);

		_DEBUG("UPDstate");
	}
	else if (eventType == kEventTypeModdedGameplay || eventType == kEventTypeModdedSwitch || eventType == kEventTypeModdedWorkbench)
	{
		this->currentState->UpdateAmmoState(extraDataList, attachedMods, eventType, false, modsToModify);

		/*ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData)
			return false;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData || !instanceData->ammo)
			return false;
		this->currentState->chamberedAmmo = instanceData->ammo;*/
		_DEBUG("AMMOstate");
	}
	return false;
}


bool ExtraWeaponState::HandleEquipEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	_DEBUG("equipState");
	if (!extraDataList || !equipData) // || !equipData->ammo)
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
		if (!currInstanceData->ammo)
			return true;
		_DEBUG("eq: %i, stored: %i", equipData->loadedAmmoCount, this->currentState->loadedAmmo);
		UInt8 notSupportedAmmo = (MSF_Base::IsNotSupportedAmmo(currInstanceData->ammo) & WeaponState::mAmmoMask);
		this->currentState->flags |= notSupportedAmmo;
		if (notSupportedAmmo)
		{
			this->currentState->loadedAmmo = equipData->loadedAmmoCount;
			this->currentState->chamberedCount = equipData->loadedAmmoCount < this->currentState->chamberSize ? equipData->loadedAmmoCount : this->currentState->chamberSize;
			return true;
		}

		UInt32 inventoryAmmo = Utilities::GetInventoryItemCount((*g_player)->inventoryList, equipData->ammo);
		if (inventoryAmmo < this->currentState->loadedAmmo)
		{
			this->currentState->loadedAmmo = inventoryAmmo;
			equipData->loadedAmmoCount = inventoryAmmo;
		}
		else
			equipData->loadedAmmoCount = this->currentState->loadedAmmo;
		this->currentState->flags |= MSF_Data::InstanceHasBCRSupport(currInstanceData) << 5;
		this->currentState->flags |= MSF_Data::InstanceHasTRSupport(currInstanceData) << 4;
		if ((this->currentState->flags & WeaponState::bHasBCR) && (this->currentState->flags & WeaponState::bHasTacticalReload))
			MSF_MainData::BCRinterfaceHolder.SetBCRammoCap(this->currentState->ammoCapacity + 1);
		//set BCR ammoCount?
		_DEBUG("eq: %i, stored: %i", equipData->loadedAmmoCount, this->currentState->loadedAmmo);
		//if (!Utilities::HasObjectMod(attachedMods, this->currentState->currentSwitchedAmmo->mod))//validate
		//{
		//	//attach
		//}
	}
	else
		this->UpdateWeaponStates(extraDataList, equipData, kEventTypeEquip); //should not reach this
	return true;
}

bool ExtraWeaponState::HandleFireEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	_DEBUG("fireWeaponState");
	if (!this->currentState)
		this->UpdateWeaponStates(extraDataList, equipData, kEventTypeReload);
	else
	{
		if (this->currentState->BCRammo.size() > 0)
		{
			UInt32 idxDel = 0;
			UInt32 idxNext = 0;
			if (this->currentState->flags & ExtraWeaponState::WeaponState::bChamberLIFO)
			{
				idxDel = this->currentState->BCRammo.size() - 1;
				idxNext = this->currentState->BCRammo.size() - 2;
			}
			TESAmmo* firedAmmo = this->currentState->BCRammo[idxDel];
			TESAmmo* nextAmmo = this->currentState->BCRammo[idxNext];
			this->currentState->BCRammo.erase(this->currentState->BCRammo.begin() + idxDel);
		}
	}
	return true;
}

bool ExtraWeaponState::HandleAmmoChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData)
{
	if (!this->currentState)
		this->UpdateWeaponStates(extraDataList, equipData, kEventTypeAmmoCount);
	this->currentState->loadedAmmo = equipData->loadedAmmoCount;
	if (this->currentState->loadedAmmo < this->currentState->chamberSize)
		this->currentState->chamberedCount = this->currentState->loadedAmmo;
	if ((this->currentState->flags & WeaponState::bHasBCR) && this->currentState->loadedAmmo != this->currentState->BCRammo.size())
	{
		this->currentState->BCRammo.clear();
		for (UInt32 ammoIdx = 0; ammoIdx < this->currentState->loadedAmmo; ammoIdx++)
			this->currentState->BCRammo.push_back(this->currentState->chamberedAmmo);
	}
	_DEBUG("ammoChangeState");
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

bool ExtraWeaponState::HandleReloadEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType, UInt32 oldLoadedAmmoCount) //0:start,1:BCR,2:complete
{
	_DEBUG("reloadState");
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
	TESObjectWEAP::InstanceData* currInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!currInstanceData || !baseWeap)
		return false;
	if (this->currentState)
	{
		if (!currInstanceData->ammo)
			return true;
		UInt8 notSupportedAmmo = (MSF_Base::IsNotSupportedAmmo(currInstanceData->ammo) & WeaponState::mAmmoMask);
		this->currentState->flags |= notSupportedAmmo;
		if (notSupportedAmmo)
		{
			this->currentState->loadedAmmo = equipData->loadedAmmoCount;
			this->currentState->chamberedCount = equipData->loadedAmmoCount < this->currentState->chamberSize ? equipData->loadedAmmoCount : this->currentState->chamberSize;
			return true;
		}
		if (eventType == ExtraWeaponState::bEventTypeReloadInventory)
		{
			UInt32 ammoToUse = Utilities::GetInventoryItemCount((*g_player)->inventoryList, this->currentState->equippedAmmo);
			if (ammoToUse > currInstanceData->ammoCapacity)
				ammoToUse = currInstanceData->ammoCapacity;
			equipData->loadedAmmoCount = ammoToUse;
			this->currentState->loadedAmmo = ammoToUse;
			this->currentState->chamberedCount = equipData->loadedAmmoCount < this->currentState->chamberSize ? equipData->loadedAmmoCount : this->currentState->chamberSize;
			if (MSF_Data::InstanceHasBCRSupport(currInstanceData))
			{
				this->currentState->BCRammo.clear();
				for (UInt32 ammoIdx = 0; ammoIdx < this->currentState->loadedAmmo; ammoIdx++)
					this->currentState->BCRammo.push_back(this->currentState->chamberedAmmo);
			}
			return true;
		}

		UInt16 BCRtype = MSF_MainData::modSwitchManager.GetIsBCRreload();
		if (BCRtype)
		{
			//if (MSF_Data::InstanceHasTRSupport(currInstanceData))
			//	MSF_MainData::BCRinterfaceHolder.SetBCRammoCap(this->currentState->ammoCapacity + 1);
			this->currentState->BCRammo.push_back(this->currentState->equippedAmmo);
			this->currentState->chamberedCount = equipData->loadedAmmoCount < this->currentState->chamberSize ? equipData->loadedAmmoCount : this->currentState->chamberSize;
			if (eventType == ExtraWeaponState::bEventTypeReloadAfterSwitch)
			{
				equipData->loadedAmmoCount = 1;
				UInt32 totalAmmoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, this->currentState->equippedAmmo);
				MSF_MainData::BCRinterfaceHolder.RestoreBCRvariables();
				MSF_MainData::BCRinterfaceHolder.UpdateBCRvariables(1, currInstanceData->ammoCapacity, totalAmmoCount);
			}
		}
		else
		{
			UInt32 ammoToUse = Utilities::GetInventoryItemCount((*g_player)->inventoryList, this->currentState->equippedAmmo);
			if (ammoToUse > currInstanceData->ammoCapacity)
				ammoToUse = currInstanceData->ammoCapacity;
			if (MSF_Data::InstanceHasTRSupport(currInstanceData) && (eventType != ExtraWeaponState::bEventTypeReloadAfterSwitch))
				equipData->loadedAmmoCount = ammoToUse + (oldLoadedAmmoCount < this->currentState->chamberedCount ? oldLoadedAmmoCount : this->currentState->chamberedCount); //currInstanceData->ammoCapacity +
			this->currentState->chamberedCount = equipData->loadedAmmoCount < this->currentState->chamberSize ? equipData->loadedAmmoCount : this->currentState->chamberSize;
		}
	}

	return true;
}

bool ExtraWeaponState::HandleModChangeEvent(ExtraDataList* extraDataList, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify, UInt8 eventType)
{
	if (!extraDataList || !modsToModify)
		return false;

	if (eventType != kEventTypeModdedWorkbench)
	{
		ExtraWeaponState* weaponState = ExtraWeaponState::Init(extraDataList, nullptr);
		if (weaponState)
		{
			weaponState->UpdateWeaponStates(extraDataList, nullptr, eventType, modsToModify);
		}
		//else 
		//{
		//	_DEBUG("ERROR: impossible mod change reached");
			/*
			ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
			if (!extraInstanceData)
				return false;
			TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
			TESObjectWEAP* weapon = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
			BGSObjectInstanceExtra* moddata = DYNAMIC_CAST(extraDataList->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);

			BGSMod::Attachment::Mod* targetAmmoMod = nullptr;
			bool toAttach = false;
			if (!instanceData)
				return false;
			MSF_Base::GetAmmoModToModify(moddata, instanceData->ammo, weapon, &targetAmmoMod, &toAttach);
			if (!targetAmmoMod)
				return false;
			modsToModify->push_back(std::pair<BGSMod::Attachment::Mod*, bool >(targetAmmoMod, toAttach));
			*/
		//}
	}
	else
	{
		if (extraDataList->HasType(ExtraDataType::kExtraData_Rank))
		{
			ExtraRank* holder = (ExtraRank*)extraDataList->GetByType(kExtraData_Rank);
			ExtraWeaponState* weaponState = MSF_MainData::weaponStateStore.Get(holder->rank);
			_DEBUG("WS rank UI: %08X, state: %p", holder->rank, weaponState);
			if (weaponState)
			{
				weaponState->UpdateWeaponStates(extraDataList, nullptr, eventType, modsToModify);

				//targetAmmo = ExtraWeaponState::GetAmmoForWorkbenchUI(extraDataList);

			}
			else
				_DEBUG("ERROR: impossible mod change reached");
		}
		//else
		//	_DEBUG("ERROR: impossible mod change reached");
	}
	//BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(moddata, weapon); 
	//MSF_Base::GetAmmoModToModify(moddata, targetAmmo, weapon, &targetAmmoMod, &toAttach);




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

bool ExtraWeaponState::SetEquippedAmmo(TESAmmo* ammo)
{
	if (this->currentState && ammo)
		return this->currentState->equippedAmmo = ammo;
	else
		return false;
}

TESAmmo* ExtraWeaponState::GetEquippedAmmo()
{
	if (this->currentState)
		return this->currentState->equippedAmmo;
	else
		return nullptr;
}

ExtraWeaponState::AmmoStateData* ExtraWeaponState::GetAmmoStateData()
{
	if (!this->currentState)
		return nullptr;
	AmmoStateData data;
	data.ammoCapacity = this->currentState->ammoCapacity;
	data.chamberedCount = this->currentState->chamberedCount;
	data.chamberSize = this->currentState->chamberSize;
	data.loadedAmmo = this->currentState->loadedAmmo;
	return &data;
}

UInt8 ExtraWeaponState::HasNotSupportedAmmo()
{
	if (!this->currentState)
		return 0;
	return this->currentState->flags & WeaponState::mAmmoMask;
}

void ExtraWeaponState::PrintStoredData()
{
	_MESSAGE("ID: %08X; total states: %08X; active state marked with *", this->ID, this->weaponStates.size());
	UInt32 idx = 0;
	for (const auto& state : this->weaponStates)
	{
		UInt32 modFormID = 0;
		if (state.first)
			modFormID = state.first->formID;
		if (state.second)
		{
			UInt32 ammoFormID = 0;
			UInt32 chAmmoFormID = 0;
			if (state.second->chamberedAmmo)
				chAmmoFormID = state.second->chamberedAmmo->formID;
			if (state.second->equippedAmmo)
				ammoFormID = state.second->equippedAmmo->formID;
			if (state.second == this->currentState)
				_MESSAGE("-State*%02X: mod: %08X; chamberedAmmo: %08X; equippedAmmo: %08X; loadedAmmoCount %i; chamberSize: %i; chamberedCount: %i; ammoCapacity: %i; shotCount: %i; flags: %04X", idx, modFormID, chAmmoFormID, ammoFormID, state.second->loadedAmmo, state.second->chamberSize, state.second->chamberedCount, state.second->ammoCapacity, state.second->shotCount, state.second->flags);
			else
				_MESSAGE("-State %02X: mod: %08X; chamberedAmmo: %08X; equippedAmmo: %08X; loadedAmmoCount %i; chamberSize: %i; chamberedCount: %i; ammoCapacity: %i; shotCount: %i; flags: %04X", idx, modFormID, chAmmoFormID, ammoFormID, state.second->loadedAmmo, state.second->chamberSize, state.second->chamberedCount, state.second->ammoCapacity, state.second->shotCount, state.second->flags);
		}
		else if (state.second == this->currentState)
			_MESSAGE("-State*%02X: mod: %08X", idx, modFormID);
		else
			_MESSAGE("-State %02X: mod: %08X", idx, modFormID);
		idx++;
	}
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