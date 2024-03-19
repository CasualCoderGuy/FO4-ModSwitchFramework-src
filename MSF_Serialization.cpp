#include "MSF_Serialization.h"
#include "f4se/PluginAPI.h"


namespace MSF_Serialization
{
	std::set<UInt32> readedNotes;


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
		UInt32 TSCount = 0;
		if (!Serialization::ReadData(intfc, &TSCount))
		{
			_MESSAGE("Error loading readed notes count");
			return false;
		}
		_MESSAGE("readed notes count loaded: %i", TSCount);
		for (UInt32 i = 0; i < TSCount; i++)
		{
			UInt32 oldformId = 0;
			UInt32 newformId = 0;

			if (!Serialization::ReadData(intfc, &oldformId))
			{
				_MESSAGE("Error loading formId parameter");
				return false;
			}

			// Skip if handle is no longer valid.
			if (!intfc->ResolveFormId(oldformId, &newformId))
				continue;

			readedNotes.insert(newformId);
		}
		return true;
	}

	void SaveCallback(const F4SESerializationInterface * intfc)
	{
		_MESSAGE("Saving MSF serialization data.");
		MSF_Serialization::Save(intfc, 'EXWS', SERIALIZATION_VERSION);
	}

	bool Save(const F4SESerializationInterface * intfc, UInt32 type, UInt32 version)
	{
		intfc->OpenRecord(type, version);

		UInt32 size = readedNotes.size();
		_MESSAGE("readed notes count to save: %i", size);
		if (!intfc->WriteRecordData(&size, sizeof(size)))
			return false;

		for (auto & form : readedNotes)
		{
			UInt32 formId = form;
			if (!intfc->WriteRecordData(&formId, sizeof(formId)))
				return false;
		}
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

bool StoredExtraWeaponState::SaveExtra(const F4SESerializationInterface* intfc, UInt32 version)
{
	intfc->OpenRecord(this->dataType, version);

	intfc->WriteRecordData(&this->ID, sizeof(this->ID));
	intfc->WriteRecordData(&this->currentState, sizeof(this->currentState));
	UInt32 size = this->weaponStates.size();
	intfc->WriteRecordData(&this->currentState, sizeof(this->currentState));
	for (auto itState = this->weaponStates.begin(); itState != this->weaponStates.end(); itState++)
	{
		intfc->WriteRecordData(&itState->first, sizeof(itState->first));
		itState->second.SaveState(intfc, version);
	}
	return true;
}
