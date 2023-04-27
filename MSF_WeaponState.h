#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"

class DataHolderParentInstance
{
	UInt32 formID;
	UInt32 stackID;
	ObjectRefHandle refHandle;
};

class BurstModeManager : public BurstModeData
{
public:
	BurstModeManager(BurstModeData* templateData, UInt8 bActive) : BurstModeData(templateData->delayTime, templateData->flags, templateData->numOfTotalShots) { numOfShotsFired = 0; SetState(bActive); }
	bool HandleFireEvent();
	bool ResetShotsOnReload();
	bool SetState(UInt8 bActive);
private:
	volatile UInt8 numOfShotsFired;
};

class ExtraWeaponState : public BSExtraData, DataHolderParentInstance
{
public:
	virtual ~ExtraWeaponState() override;
	virtual void Unk_01() override {};
	virtual void Unk_02() override {};
	static ExtraWeaponState* Init(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool SetWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, bool temporary);
	bool RecoverTemporaryState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool SetCurrentStateTemporary();
	bool HandleFireEvent();
	bool HandleReloadEvent();
	bool HandleModChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData); //update burst manager
	enum
	{
		kType_ExtraWeaponState = 0xF1
	};
	class WeaponState
	{
	public:
		WeaponState(TESObjectWEAP::InstanceData* instanceData, UInt64 loadedAmmoCount);
		~WeaponState();
		enum
		{
			bHasLevel = 0x01,
			bActive = 0x02
		};
		UInt32 flags; //state flags
		UInt16 ammoCapacity;
		UInt64 magazineCount;
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
	ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	std::vector<std::tuple<BGSMod::Attachment::Mod*, WeaponState*, WeaponState*>> weaponStates;
	BurstModeManager* burstModeManager;
	//std::pair<WeaponState, WeaponState> primaryState; //temporaryState, baseState
	//std::pair<WeaponState, WeaponState> secondaryState;
	//bool hasSecondaryAmmo;
	//BGSMod::Attachment::Mod* baseMod;
	//BGSMod::Attachment::Mod* functionMod;
};

typedef unsigned long WeaponStateID;
class WeaponStateStore
{
public:
	WeaponStateStore()
	{
		storage.reserve(100);
	};
	void Free() {};

private:
	std::unordered_map<WeaponStateID, ExtraWeaponState*> storage;
};