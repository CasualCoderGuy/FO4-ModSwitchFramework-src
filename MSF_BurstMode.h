#pragma once
#include "MSF_Shared.h"
#include "f4se\GameThreads.h"

class BurstModeData
{
public:
	BurstModeData(UInt32 delay, UInt8 settings, UInt8 totalShots)
	{
		delayTime = delay;
		flags = settings;
		numOfTotalShots = totalShots;
	};
	enum
	{
		bOnePullBurst = 0x01, //If FALSE, upon releasing the trigger the firing is stopped; if TRUE, all the shots will be fired in a burst
		bResetShotCountOnRelease = 0x02, //If TRUE, upon releasing the trigger the shot count will reset; if FALSE, the shot count will not reset(only applies if bOnePullBurst is FALSE)
		bResetShotCountOnReload = 0x04,
		bTypeAuto = 0x08,
		bActive = 0x10
	};
	UInt32 delayTime; //Interval between two shots in a single burst(in milliseconds)
	UInt8 flags;
	UInt8 numOfTotalShots; //Number of shots fired during a single burst
protected:
	BurstModeData() {}
};

class BurstModeManager : public BurstModeData
{
public:
	//BurstModeManager(BurstModeData* templateData, UInt8 bActive) : BurstModeData(templateData->delayTime, templateData->flags, templateData->numOfTotalShots) { numOfShotsFired = 0; SetState(bActive); }
	BurstModeManager(BurstModeData* templateData, bool bActive) { delayTime = templateData->delayTime; flags = templateData->flags; numOfTotalShots = templateData->numOfTotalShots;  numOfShotsFired = 0; SetState(bActive << 4); };
	bool HandleFireEvent();
	bool HandleReleaseEvent();
	bool ResetShotsOnReload();
	bool FireWeapon();
	bool HandleEquipEvent(TESObjectWEAP::InstanceData* weaponInstance); //ExtraDataList* extraDataList
	bool HandleModChangeEvent(ExtraDataList* extraDataList);
	bool SetState(UInt8 bActive);
private:
	volatile short numOfShotsFired;
};

class FireBurstTask : public ITaskDelegate
{
public:
	FireBurstTask(BurstModeManager* thisManager)
	{
		manager = thisManager;
	};
	virtual void Run() final
	{
		_DEBUG("fire burst thread");
		manager->FireWeapon();
	}
private:
	BurstModeManager* manager;
};