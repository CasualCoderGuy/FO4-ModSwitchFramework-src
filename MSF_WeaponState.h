#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"


typedef UInt32 WeaponStateID;

class DataHolderParentInstance
{
public:
	ExtraDataList* extraList;
	UInt32 formID;
	UInt32 stackID;
	ObjectRefHandle refHandle;
};

class ExtraWeaponState
{
public:
	~ExtraWeaponState();
	static ExtraWeaponState* Init(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	static bool HandleWeaponStateEvents(UInt8 eventType);
	static ModData::Mod defaultStatePlaceholder;
	//bool SetWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, bool temporary);
	//bool RecoverTemporaryState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	//bool SetCurrentStateTemporary();
	bool HandleEquipEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool HandleFireEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool HandleAmmoChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool HandleReloadEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType);
	bool HandleModChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData); //update burst manager
	bool UpdateWeaponStates(ExtraDataList* extraDataList, EquipWeaponData* equipData);

	enum
	{
		kEventTypeUndefined,
		kEventTypeEquip,
		KEventTypeAmmoCount,
		KEventTypeFireWeapon,
		KEventTypeReload,
		KEventTypeModded //WB vs switch?
	};

	class WeaponState
	{
	public:
		WeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
		enum
		{
			bHasLevel = 0x01,
			bActive = 0x02,
			bHasTacticalReload = 0x10,
			bHasBCR = 0x20
		};
		UInt16 flags; //state flags
		UInt16 ammoCapacity;
		UInt16 chamberSize;
		volatile short shotCount; 
		volatile long long loadedAmmo;
		TESAmmo* chamberedAmmo;
		std::vector<TESAmmo*> BCRammo;
		//AmmoData::AmmoMod* currentSwitchedAmmo;
		//AmmoData::AmmoMod* switchToAmmoAfterFire;
		//std::vector<ModData::Mod*> attachedMods; //maybe later
	};
private:
	ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	WeaponStateID ID;
	ExtraRank* holder; //ExtraDataList
	std::map<ModData::Mod*, WeaponState*> weaponStates;
	WeaponState* currentState;
	//BurstModeManager* burstModeManager;
};

class WeaponStateStore
{
public:
	WeaponStateStore()
	{
		mapstorage.reserve(100);
		vectorstorage.reserve(100);
	};
	void Free() {};
	WeaponStateID Add(ExtraWeaponState* state)
	{
		//nocheck
		vectorstorage.push_back(state);
		return vectorstorage.size();
	};
	ExtraWeaponState* Get(WeaponStateID id)
	{
		ExtraWeaponState* state = nullptr;
		if (id && id <= vectorstorage.size())
			state = vectorstorage[id - 1];
		return state;
	};
	ExtraWeaponState* GetValid(WeaponStateID id)
	{
		ExtraWeaponState* state = nullptr;
		if (id && id <= vectorstorage.size())
			state = vectorstorage[id - 1];
		if (state)// && state->ValidateParent())
			return state;
		return nullptr;
	};
private:
	std::unordered_map<WeaponStateID, ExtraWeaponState*> mapstorage;
	std::vector<ExtraWeaponState*> vectorstorage;
};