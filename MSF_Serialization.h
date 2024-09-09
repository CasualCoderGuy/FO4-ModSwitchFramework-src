#pragma once
#include "f4se/PluginAPI.h"
#include "f4se/PapyrusEvents.h"
#include "MSF_WeaponState.h"
#include "MSF_Data.h"
#include <unordered_map>

	
class StoredExtraWeaponState
{
public:
	static const UInt32 dataType = 'EXWS';
	StoredExtraWeaponState(ExtraWeaponState* extraWeaponState);
	StoredExtraWeaponState(const F4SESerializationInterface* intfc, UInt32 version);
	bool SaveExtra(const F4SESerializationInterface* intfc, UInt32 version);
	bool Recover(const F4SESerializationInterface* intfc, UInt32 version);

	/*
	static StoredExtraWeaponState* ConvertForStorage(ExtraWeaponState* extraWeaponState);
	bool Recover();

	class StoredAmmoMod
	{
	public:
		StoredAmmoMod(AmmoData::AmmoMod* ammoMod);
		AmmoData::AmmoMod* Recover();

		UInt32 baseAmmo;
		UInt32 ammoType;
		UInt32 ammoMod;
		UInt8 ammo_number;
	};

	class StoredModData
	{
	public:
		StoredModData(ModData::Mod* mod);
		ModData::Mod* Recover();

		UInt32 attachParent;
		UInt32 instantiationFilter;
		UInt32 mod;
		UInt8 mod_number;
	};
	*/
	class StoredWeaponState
	{
	public:
		StoredWeaponState(ExtraWeaponState::WeaponState* weaponState);
		StoredWeaponState(const F4SESerializationInterface* intfc, UInt32 version);
		bool SaveState(const F4SESerializationInterface* intfc, UInt32 version);
		ExtraWeaponState::WeaponState* Recover(const F4SESerializationInterface* intfc, UInt32 version);

		UInt16 flags; //state flags
		UInt16 ammoCapacity;
		UInt16 chamberSize;
		UInt16 shotCount;
		UInt32 loadedAmmo;
		UInt32 chamberedAmmo;
		std::vector<UInt32> BCRammo;
		std::vector<UInt32> stateMods;
		//StoredAmmoMod switchToAmmoAfterFire;
		//StoredAmmoMod currentSwitchedAmmo;
		//std::vector<ModData::Mod*> attachedMods; //maybe later
	};
private:
	WeaponStateID ID;
	UInt32 currentState;
	//BurstModeManager* burstModeManager;
	std::map<UInt32, StoredWeaponState> weaponStates;
};
	
namespace MSF_Serialization
{

	void RevertCallback(const F4SESerializationInterface* intfc);
	void LoadCallback(const F4SESerializationInterface* intfc);
	void SaveCallback(const F4SESerializationInterface* intfc);

	bool Load(const F4SESerializationInterface* intfc, UInt32 version);
	bool Save(const F4SESerializationInterface* intfc, UInt32 version, ExtraWeaponState* extraState);
}