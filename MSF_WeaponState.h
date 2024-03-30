#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
//#include "MSF_Serialization.h"


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
	ExtraWeaponState(ExtraRank* holder);
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
		WeaponState(UInt16 flags, UInt16 ammoCapacity, UInt16 chamberSize, UInt16 shotCount, UInt64 loadedAmmo, TESAmmo* chamberedAmmo, std::vector<TESAmmo*>* BCRammo);
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
	friend class StoredExtraWeaponState;
private:
	ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	WeaponStateID ID;
	ExtraRank* holder; //ExtraDataList
	std::map<BGSMod::Attachment::Mod*, WeaponState*> weaponStates;
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
	void Free()
	{
		for (auto& state : vectorstorage)
			delete state;
		vectorstorage.clear();
		mapstorage.clear();
	};
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
	bool StoreForLoad(WeaponStateID id, ExtraRank* holder)
	{
		if (!holder || id == 0)
			return false;
		ExtraRank* occupied = mapstorage[id];
		if (!occupied)
			mapstorage[id] = holder;
		return true;
	};
	ExtraRank* GetForLoad(WeaponStateID id)
	{
		ExtraRank* holder = mapstorage[id];
		if (holder && holder->rank != id)
		{
			_DEBUG("IDerror");
			holder = nullptr;
		}
		return holder;
	};
	UInt32 GetCount()
	{
		return vectorstorage.size();
	};
	void SaveWeaponStates(std::function<bool(const F4SESerializationInterface*, UInt32, ExtraWeaponState*)> f_callback, const F4SESerializationInterface* intfc, UInt32 version)
	{
		for (const auto& state : vectorstorage)
		{
			if (state)
				f_callback(intfc, version, state);
		}
	};
private:
	std::unordered_map<WeaponStateID, ExtraRank*> mapstorage; //used only for the loading of f4se serialized data, WeaponStateID is invalid afterwards
	std::vector<ExtraWeaponState*> vectorstorage; // used for quick access of WeaponState with ExtraRank->rank (WeaponStateID) being the vector index +1
};