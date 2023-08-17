#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"


typedef unsigned long WeaponStateID;

class DataHolderParentInstance
{
public:
	ExtraDataList* extraList;
	UInt32 formID;
	UInt32 stackID;
	ObjectRefHandle refHandle;
};

class ExtraWeaponState : public BSExtraData
{
public:
	virtual ~ExtraWeaponState() override;
	virtual void Unk_01() override {};
	virtual void Unk_02() override {};
	static ExtraWeaponState* Init(ExtraDataList* extraDataList, EquipWeaponData* equipData, DataHolderParentInstance &instance);
	bool SetWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, bool temporary);
	bool RecoverTemporaryState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool SetCurrentStateTemporary();
	bool HandleFireEvent();
	bool HandleReloadEvent();
	bool HandleModChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData); //update burst manager
	bool SetParentRef(ObjectRefHandle refHandle);
	bool SetParentInvItem(ExtraDataList* extraList);
	bool ValidateParent();
	enum
	{
		kType_ExtraWeaponState = 0xF1
	};
	class WeaponState
	{
	public:
		WeaponState(TESObjectWEAP::InstanceData* instanceData);
		~WeaponState();
		enum
		{
			bHasLevel = 0x01,
			bActive = 0x02
		};
		UInt32 flags; //state flags
		UInt16 ammoCapacity;
		TESAmmo* switchedAmmoType;
		BGSMod::Attachment::Mod* switchedAmmoMod;
	private:
		TESAmmo* ammoType;
		BGSImpactDataSet* impactDataSet; //unk58
		BGSProjectile* projectileOverride;
		SpellItem* critEffect; //unk78
		tArray<TBO_InstanceData::DamageTypes> damageTypes;
		UInt16 baseDamage;
		UInt32 numProjectiles;
		float secondary;
		float critDamageMult;
		float minRange;
		float maxRange;
		float outOfRangeMultiplier;
		UInt32 stagger;
	};
private:
	ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, DataHolderParentInstance &instance);
	DataHolderParentInstance parent;
	WeaponStateID ID;
	std::vector<std::tuple<BGSMod::Attachment::Mod*, UInt64, WeaponState*, WeaponState*>> weaponStates;
	BurstModeManager* burstModeManager;
	//std::pair<WeaponState, WeaponState> primaryState; //temporaryState, baseState
	//std::pair<WeaponState, WeaponState> secondaryState;
	//bool hasSecondaryAmmo;
	//BGSMod::Attachment::Mod* baseMod;
	//BGSMod::Attachment::Mod* functionMod;
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
		if (state && state->ValidateParent())
			return state;
		return nullptr;
	};
private:
	std::unordered_map<WeaponStateID, ExtraWeaponState*> mapstorage;
	std::vector<ExtraWeaponState*> vectorstorage;
};