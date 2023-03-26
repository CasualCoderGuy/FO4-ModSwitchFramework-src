#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"

class ExtraWeaponState : public BSExtraData
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
	//std::pair<WeaponState, WeaponState> primaryState; //temporaryState, baseState
	//std::pair<WeaponState, WeaponState> secondaryState;
	//bool hasSecondaryAmmo;
	//BGSMod::Attachment::Mod* baseMod;
	//BGSMod::Attachment::Mod* functionMod;
};