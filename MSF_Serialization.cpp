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