#include "MSF_Serialization.h"
#include "f4se/PluginAPI.h"


namespace MSF_Serialization
{

	void RevertCallback(const F4SESerializationInterface * intfc)
	{
		_MESSAGE("Clearing MSF serialization data.");
		MSF_MainData::weaponStateStore.Free();
	}

	void LoadCallback(const F4SESerializationInterface * intfc)
	{
		_MESSAGE("Loading MSF serialization data.");
		UInt32 type, version, length;
		while (intfc->GetNextRecordInfo(&type, &version, &length))
		{
			switch (type)
			{
			case 'EXWS':
				MSF_Serialization::Load(intfc, SERIALIZATION_VERSION);
				break;
			}
		}

	}

	bool Load(const F4SESerializationInterface * intfc, UInt32 version)
	{
		UInt32 dataCount = 0;
		Serialization::ReadData(intfc, &dataCount);
		for (UInt32 idx = 0; idx < dataCount; idx++)
		{
			StoredExtraWeaponState loadedExtraState(intfc, version);
			loadedExtraState.Recover(intfc, version);
		}
		return true;
	}

	void SaveCallback(const F4SESerializationInterface * intfc)
	{
		_MESSAGE("Saving MSF serialization data.");
		intfc->OpenRecord(StoredExtraWeaponState::dataType, SERIALIZATION_VERSION);
		UInt32 dataCount = MSF_MainData::weaponStateStore.GetCount();
		intfc->WriteRecordData(&dataCount, sizeof(dataCount));
		MSF_MainData::weaponStateStore.SaveWeaponStates(MSF_Serialization::Save, intfc, SERIALIZATION_VERSION);
	}

	bool Save(const F4SESerializationInterface * intfc, UInt32 version, ExtraWeaponState* extraState)
	{
		StoredExtraWeaponState storedExtraState(extraState);
		storedExtraState.SaveExtra(intfc, version);
		return true;
	}

}

StoredExtraWeaponState::StoredWeaponState::StoredWeaponState(ExtraWeaponState::WeaponState* weaponState)
{
	this->flags = weaponState->flags;
	this->ammoCapacity = weaponState->ammoCapacity;
	this->chamberSize = weaponState->chamberSize;
	this->shotCount = weaponState->shotCount;
	this->loadedAmmo = weaponState->loadedAmmo;
	this->chamberedAmmo = weaponState->chamberedAmmo->formID;
	for (auto itAmmo = weaponState->BCRammo.begin(); itAmmo != weaponState->BCRammo.end(); itAmmo++)
		this->BCRammo.push_back((*itAmmo)->formID);
}

StoredExtraWeaponState::StoredExtraWeaponState(ExtraWeaponState* extraWeaponState)
{
	this->ID = extraWeaponState->ID;
	this->currentState = 0;
	UInt32 idx = 1;
	for (auto itState = extraWeaponState->weaponStates.begin(); itState != extraWeaponState->weaponStates.end(); itState++)
	{
		ExtraWeaponState::WeaponState* weaponState = itState->second;
		if (weaponState == extraWeaponState->currentState)
			this->currentState = idx;
		if (itState->first)
		{
			this->weaponStates.insert({ itState->first->formID, StoredWeaponState(weaponState) });
		}
		else
		{
			this->weaponStates.insert({ 0, StoredWeaponState(weaponState) });
		}
		idx++;
	}
}

StoredExtraWeaponState::StoredWeaponState::StoredWeaponState(const F4SESerializationInterface* intfc, UInt32 version)
{
	Serialization::ReadData(intfc, &this->flags);
	Serialization::ReadData(intfc, &this->ammoCapacity);
	Serialization::ReadData(intfc, &this->chamberSize);
	Serialization::ReadData(intfc, &this->shotCount);
	Serialization::ReadData(intfc, &this->loadedAmmo);
	Serialization::ReadData(intfc, &this->chamberedAmmo);
	UInt32 size = 0;
	Serialization::ReadData(intfc, &size);
	for (UInt32 dataidx = 0; dataidx < size; dataidx++)
	{
		UInt32 ammoFormID = 0;
		Serialization::ReadData(intfc, &ammoFormID);
		this->BCRammo.push_back(ammoFormID);
	}
}

bool StoredExtraWeaponState::StoredWeaponState::SaveState(const F4SESerializationInterface* intfc, UInt32 version)
{
	intfc->WriteRecordData(&this->flags, sizeof(this->flags));
	intfc->WriteRecordData(&this->ammoCapacity, sizeof(this->ammoCapacity));
	intfc->WriteRecordData(&this->chamberSize, sizeof(this->chamberSize));
	intfc->WriteRecordData(&this->shotCount, sizeof(this->shotCount));
	intfc->WriteRecordData(&this->loadedAmmo, sizeof(this->loadedAmmo));
	intfc->WriteRecordData(&this->chamberedAmmo, sizeof(this->chamberedAmmo));
	UInt32 size = this->BCRammo.size();
	intfc->WriteRecordData(&size, sizeof(size));
	for (auto itAmmo = this->BCRammo.begin(); itAmmo != this->BCRammo.end(); itAmmo++)
	{
		UInt32 formId = *itAmmo;
		intfc->WriteRecordData(&formId, sizeof(formId));
	}
	return true;
}

ExtraWeaponState::WeaponState* StoredExtraWeaponState::StoredWeaponState::Recover(const F4SESerializationInterface* intfc, UInt32 version)
{
	UInt32 newFormID = 0;
	TESAmmo* recoveredAmmo = nullptr;
	if (intfc->ResolveFormId(this->chamberedAmmo, &newFormID))
		recoveredAmmo = (TESAmmo*)LookupFormByID(newFormID);
	std::vector<TESAmmo*> recoveredBCRammoVector;
	for (auto itAmmo = this->BCRammo.begin(); itAmmo != this->BCRammo.end(); itAmmo++)
	{
		UInt32 newBCRFormID = 0;
		if (!intfc->ResolveFormId(*itAmmo, &newBCRFormID))
			continue;
		TESAmmo* recoveredBCRAmmo = (TESAmmo*)LookupFormByID(newBCRFormID);
		if (!recoveredBCRAmmo)
			continue;
		recoveredBCRammoVector.push_back(recoveredBCRAmmo);
	}
	return new ExtraWeaponState::WeaponState(this->flags, this->ammoCapacity, this->chamberSize, this->shotCount, this->loadedAmmo, recoveredAmmo, &recoveredBCRammoVector);
}

bool StoredExtraWeaponState::SaveExtra(const F4SESerializationInterface* intfc, UInt32 version)
{
	intfc->OpenRecord(this->dataType, version);

	intfc->WriteRecordData(&this->ID, sizeof(this->ID));
	intfc->WriteRecordData(&this->currentState, sizeof(this->currentState));
	UInt32 size = this->weaponStates.size();
	intfc->WriteRecordData(&size, sizeof(size));
	for (auto itState = this->weaponStates.begin(); itState != this->weaponStates.end(); itState++)
	{
		intfc->WriteRecordData(&itState->first, sizeof(itState->first));
		itState->second.SaveState(intfc, version);
	}
	return true;
}

StoredExtraWeaponState::StoredExtraWeaponState(const F4SESerializationInterface* intfc, UInt32 version)
{
	Serialization::ReadData(intfc, &this->ID);
	Serialization::ReadData(intfc, &this->currentState);
	UInt32 size = 0;
	Serialization::ReadData(intfc, &size);
	for (UInt32 dataidx = 0; dataidx < size; dataidx++)
	{
		UInt32 modFormID = 0;
		Serialization::ReadData(intfc, &modFormID);
		this->weaponStates.insert({ modFormID, StoredWeaponState(intfc, version) });
	}
}

bool StoredExtraWeaponState::Recover(const F4SESerializationInterface* intfc, UInt32 version)
{
	ExtraRank* holder = MSF_MainData::weaponStateStore.GetForLoad(this->ID);
	if (!holder)
		return false;
	ExtraWeaponState* recoveredExtraWeaponState = new ExtraWeaponState(holder);
	UInt32 idx = 0;
	for (auto itStates = this->weaponStates.begin(); itStates != this->weaponStates.end(); itStates++)
	{
		idx++;
		ExtraWeaponState::WeaponState* recoveredWeaponState = nullptr;
		if (itStates->first != 0)
		{
			UInt32 newFormID = 0;
			if (!intfc->ResolveFormId(itStates->first, &newFormID))
				continue;
			BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)LookupFormByID(newFormID);
			if (!mod)
				continue;
			recoveredWeaponState = itStates->second.Recover(intfc, version);
			if (!recoveredWeaponState)
				continue;
			recoveredExtraWeaponState->weaponStates[mod] = recoveredWeaponState;
		}
		else
		{
			recoveredWeaponState = itStates->second.Recover(intfc, version);
			if (!recoveredWeaponState)
				continue;
			recoveredExtraWeaponState->weaponStates[nullptr] = recoveredWeaponState;
		}
		if (this->currentState == idx)
			recoveredExtraWeaponState->currentState = recoveredWeaponState;
	}
	return true;
}