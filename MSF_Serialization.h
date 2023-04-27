#pragma once
#include "f4se/PluginAPI.h"
#include "f4se/PapyrusEvents.h"
#include <unordered_map>

namespace Serialization
{
	void RevertCallback(const F4SESerializationInterface * intfc);
	void LoadCallback(const F4SESerializationInterface * intfc);
	void SaveCallback(const F4SESerializationInterface * intfc);

	bool Load(const F4SESerializationInterface * intfc, UInt32 version);
	bool Save(const F4SESerializationInterface * intfc, UInt32 type, UInt32 version);
}