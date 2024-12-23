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
		//if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bEnableMetadataSaving))
		//{
		//	MSF_MainData::weaponStateStore.InvalidateAllIDs();
		//	MSF_MainData::weaponStateStore.ClearInvalidWeaponStates();
		//	_MESSAGE("MSF serialization data loading disabled.");
		//	return;
		//}
		_MESSAGE("Loading MSF serialization data.");
		UInt32 type, version, length;
		while (intfc->GetNextRecordInfo(&type, &version, &length))
		{
			switch (type)
			{
			case 'EXWS':
			{
				_DEBUG("Load info: %08X %08X %08X", type, version, length);
				MSF_Serialization::Load(intfc, version);
			}
			break;
			}
		}
		MSF_MainData::weaponStateStore.ClearInvalidWeaponStates();
	}

	bool Load(const F4SESerializationInterface * intfc, UInt32 version)
	{
		//UInt32 dataCount = 0;
		//Serialization::ReadData(intfc, &dataCount);
		//for (UInt32 idx = 0; idx < dataCount; idx++)
		if (version >= MIN_SUPPORTED_SERIALIZATION_VERSION)
		{
			StoredExtraWeaponState loadedExtraState(intfc, version);
			loadedExtraState.Recover(intfc, version);
		}
		else
		{
			WeaponStateID id = 0;
			Serialization::ReadData(intfc, &id);
			MSF_MainData::weaponStateStore.InvalidateID(id);
			_MESSAGE("WARNING: deserialization failure at extra weapon state with ID %08X: unsupported save file version", id);
			return false;
		}
		return true;
	}

	void SaveCallback(const F4SESerializationInterface * intfc)
	{
		//if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bEnableMetadataSaving))
		//{
		//	_MESSAGE("MSF serialization data saving disabled.");
		//	return;
		//}
		_MESSAGE("Saving MSF serialization data.");
		//intfc->OpenRecord(StoredExtraWeaponState::dataType, SERIALIZATION_VERSION);
		//UInt32 dataCount = MSF_MainData::weaponStateStore.GetCount();
		//intfc->WriteRecordData(&dataCount, sizeof(dataCount));
		MSF_MainData::weaponStateStore.SaveWeaponStates(MSF_Serialization::Save, intfc, MSF_VERSION);
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
	this->chamberedCount = weaponState->chamberedCount;
	this->shotCount = weaponState->shotCount;
	this->loadedAmmo = weaponState->loadedAmmo;
	if (weaponState->chamberedAmmo)
		this->chamberedAmmo = weaponState->chamberedAmmo->formID;
	else
		this->chamberedAmmo = 0;
	if (weaponState->equippedAmmo)
		this->equippedAmmo = weaponState->equippedAmmo->formID;
	else
		this->equippedAmmo = 0;
	for (auto itAmmo = weaponState->BCRammo.begin(); itAmmo != weaponState->BCRammo.end(); itAmmo++)
	{
		if (*itAmmo)
			this->BCRammo.push_back((*itAmmo)->formID);
	}
	for (auto itMod = weaponState->stateMods.begin(); itMod != weaponState->stateMods.end(); itMod++)
	{
		if (*itMod)
			this->stateMods.push_back((*itMod)->formID);
	}
}

StoredExtraWeaponState::StoredExtraWeaponState(ExtraWeaponState* extraWeaponState)
{
	_DEBUG("Saving WeaponState %08X", extraWeaponState->ID);
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
	if (MAKE_EXE_VERSION_EX(1, 0, 0, 0) <= version)
	{
		Serialization::ReadData(intfc, &this->equippedAmmo);
		Serialization::ReadData(intfc, &this->chamberedCount);
	}
	else
	{
		this->equippedAmmo = this->chamberedAmmo;
		this->chamberedCount = this->chamberSize;
	}
	UInt32 size = 0;
	Serialization::ReadData(intfc, &size);
	_DEBUG("Loaded data: v%08X %04X %04X %04X %04X %016X %08X %08X %08X %08X", version, this->flags, this->ammoCapacity, this->chamberSize, this->shotCount, this->loadedAmmo, this->chamberedAmmo, this->equippedAmmo, this->chamberedCount, size);
	for (UInt32 dataidx = 0; dataidx < size; dataidx++)
	{
		UInt32 ammoFormID = 0;
		Serialization::ReadData(intfc, &ammoFormID);
		this->BCRammo.push_back(ammoFormID);
	}
	UInt32 modsize = 0;
	Serialization::ReadData(intfc, &modsize);
	for (UInt32 dataidx = 0; dataidx < modsize; dataidx++)
	{
		UInt32 formIdKW = 0;
		UInt32 formIdMod = 0;
		Serialization::ReadData(intfc, &formIdKW);
		Serialization::ReadData(intfc, &formIdMod);
		this->stateMods[formIdKW] = formIdMod;
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
	intfc->WriteRecordData(&this->equippedAmmo, sizeof(this->equippedAmmo));
	intfc->WriteRecordData(&this->chamberedCount, sizeof(this->chamberedCount));
	UInt32 size = this->BCRammo.size();
	intfc->WriteRecordData(&size, sizeof(size));
	_DEBUG("Saved data: v%08X %04X %04X %04X %04X %016X %08X %08X %08X %08X", version, this->flags, this->ammoCapacity, this->chamberSize, this->shotCount, this->loadedAmmo, this->chamberedAmmo, this->equippedAmmo, this->chamberedCount, size);
	for (auto itAmmo = this->BCRammo.begin(); itAmmo != this->BCRammo.end(); itAmmo++)
	{
		UInt32 formId = *itAmmo;
		intfc->WriteRecordData(&formId, sizeof(formId));
	}
	UInt32 modsize = this->stateMods.size();
	intfc->WriteRecordData(&modsize, sizeof(modsize));
	for (auto itMod = this->stateMods.begin(); itMod != this->stateMods.end(); itMod++)
	{
		UInt32 formIdMod = *itMod;
		intfc->WriteRecordData(&formIdMod, sizeof(formIdMod));
	}
	return true;
}

ExtraWeaponState::WeaponState* StoredExtraWeaponState::StoredWeaponState::Recover(const F4SESerializationInterface* intfc, UInt32 version)
{
	UInt32 newFormID = 0;
	TESAmmo* recoveredChamberedAmmo = nullptr;
	if (intfc->ResolveFormId(this->chamberedAmmo, &newFormID))
		recoveredChamberedAmmo = (TESAmmo*)LookupFormByID(newFormID);
	TESAmmo* recoveredEquippedAmmo = nullptr;
	if (intfc->ResolveFormId(this->equippedAmmo, &newFormID))
		recoveredEquippedAmmo = (TESAmmo*)LookupFormByID(newFormID);
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
	std::vector<BGSMod::Attachment::Mod*> recoveredStateMods;
	for (auto itMod = this->stateMods.begin(); itMod != this->stateMods.end(); itMod++)
	{
		UInt32 newModFormID = 0;
		if (!intfc->ResolveFormId(*itMod, &newModFormID))
			continue;
		BGSMod::Attachment::Mod* recoveredStateMod = (BGSMod::Attachment::Mod*)LookupFormByID(newModFormID);
		if (!recoveredStateMod)
			continue;
		recoveredStateMods.push_back(recoveredStateMod);
	}
	return new ExtraWeaponState::WeaponState(this->flags, this->ammoCapacity, this->chamberSize, this->chamberedCount, this->shotCount, this->loadedAmmo, recoveredChamberedAmmo, recoveredEquippedAmmo, &recoveredBCRammoVector, &recoveredStateMods);
}

bool StoredExtraWeaponState::SaveExtra(const F4SESerializationInterface* intfc, UInt32 version)
{
	intfc->OpenRecord(this->dataType, version);

	intfc->WriteRecordData(&this->ID, sizeof(this->ID));
	intfc->WriteRecordData(&this->currentState, sizeof(this->currentState));
	UInt32 size = this->weaponStates.size();
	intfc->WriteRecordData(&size, sizeof(size));
	_DEBUG("Saved data: %08X, %08X, %08X", this->ID, this->currentState, size);
	for (auto itState = this->weaponStates.begin(); itState != this->weaponStates.end(); itState++)
	{
		_DEBUG("Saved data: %08X", itState->first);
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
	_DEBUG("Loaded data %08X ", this->ID, this->currentState, size);
	for (UInt32 dataidx = 0; dataidx < size; dataidx++)
	{
		UInt32 modFormID = 0;
		Serialization::ReadData(intfc, &modFormID);
		_DEBUG("Loaded data: %08X", modFormID);
		this->weaponStates.insert({ modFormID, StoredWeaponState(intfc, version) });
	}
}

bool StoredExtraWeaponState::Recover(const F4SESerializationInterface* intfc, UInt32 version)
{
	ExtraRank* holder = MSF_MainData::weaponStateStore.GetForLoad(this->ID);
	if (!holder)
	{
		_MESSAGE("WARNING: data recovery failure at extra weapon state with ID %08X: weapon state holder not found", this->ID);
		return false;
	}
	ExtraWeaponState* recoveredExtraWeaponState = new ExtraWeaponState(holder);
	_DEBUG("Loading ExtraRank %p, %08X", holder, holder->rank);
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