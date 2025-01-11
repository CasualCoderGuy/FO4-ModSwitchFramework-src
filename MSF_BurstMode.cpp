#include "MSF_BurstMode.h"
#include "MSF_Data.h"

bool BurstModeManager::SetState(UInt8 bActive)
{
	flags ^= (-(bActive & BurstModeManager::bActive) ^ flags) & BurstModeManager::bActive;
	return true;
}

bool BurstModeManager::HandleFireEvent()
{
	InterlockedIncrement16(&numOfShotsFired);
	if (numOfShotsFired < numOfTotalShots)
	{
		if (!(flags & BurstModeData::bTypeAuto))
		{
			FireBurstTask* burstTask = new FireBurstTask(this);
			delayTask delayBurst(delayTime, true, g_threading->AddTask, burstTask);
			//delayTask delayNextShot(delayTime, true, &BurstModeManager::FireWeapon, this);
		}
	}
	else
	{
		if (flags & BurstModeData::bTypeAuto)
			Utilities::PlayIdleAction(*g_player, MSF_MainData::ActionRightRelease);
		InterlockedExchange16(&numOfShotsFired, 0);
	}
	return true;
}

bool BurstModeManager::ResetShotsOnReload()
{
	if (flags & BurstModeData::bResetShotCountOnReload)
		InterlockedExchange16(&numOfShotsFired, 0);
	return true;
}

bool BurstModeManager::HandleReleaseEvent()
{
	if ((flags & BurstModeData::bResetShotCountOnRelease) && (flags & BurstModeData::bTypeAuto))
		InterlockedExchange16(&numOfShotsFired, 0);
	return true;
}

bool BurstModeManager::FireWeapon()
{
	if (!(flags & BurstModeData::bOnePullBurst))
	{
		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
		{
			if (flags & BurstModeData::bResetShotCountOnRelease)
				InterlockedExchange16(&numOfShotsFired, 0);
			return true;
		}
	}
	Actor* player = *g_player;
	TESIdleForm* fireIdle = MSF_Data::GetFireAnimation(player);
	if (fireIdle)
		return Utilities::PlayIdle(player, fireIdle);
	return false;
}

bool BurstModeManager::HandleEquipEvent(TESObjectWEAP::InstanceData* weaponInstance) //ExtraDataList* extraDataList
{
	InterlockedExchange16(&numOfShotsFired, 0);
	if (!weaponInstance)
	{
		flags &= ~BurstModeData::bActive;
		return false;
	}
	if (Utilities::WeaponInstanceHasKeyword(weaponInstance, MSF_MainData::FiringModBurstKW))
		flags |= BurstModeData::bActive;
	else
		flags &= ~BurstModeData::bActive;
	return true;
}

bool BurstModeManager::HandleModChangeEvent(ExtraDataList* extraDataList)
{
	InterlockedExchange16(&numOfShotsFired, 0);
	if (!extraDataList)
	{
		flags &= ~BurstModeData::bActive;
		return false;
	}
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModBurstKW))
		flags |= BurstModeData::bActive;
	else
		flags &= ~BurstModeData::bActive;
	return true;
}

/*
	void BurstWaitAndShoot(BurstMode* data)
	{
		if (!data)
			return;
		UInt8 totalShots = data->flags &= ~0xF0;
		for (data->NumOfRoundsFired; data->NumOfRoundsFired < totalShots; data->NumOfRoundsFired++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(data->wait));
			if (data->animReady)
			{
				data->animReady = false;
				Utilities::PlayIdle(data->actor, data->fireIdle);
			}
			//Onkeyup: if !enableonepullburst end thread, if resetshotcounts (AV) NumOfRoundsFired = 0;
		}


	}

	void BurstTest(BurstMode* data)
	{
		if (!data)
		{
			data = new BurstMode();
			data->actor = *g_player;
			data->fireSingleAction = MSF_MainData::ActionFireSingle;
			data->fireIdle = MSF_MainData::fireIdle1stP;
			data->wait = 1000;
			data->totalShots = 3;
			data->NumOfRoundsFired = 1;
			delayTask delay(data->wait, true, &BurstTest, data);
			return;
		}
		Utilities::PlayIdle(data->actor, data->fireIdle);
		data->NumOfRoundsFired++;
		if (data->NumOfRoundsFired < data->totalShots)
			delayTask delay(data->wait, false, &BurstTest, data);
		else
			delete data;
	}



	bool FireBurst(Actor* actor)
	{
		if (!actor)
			return false;
		float av = Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeTime->formID);
		_DEBUG("av: %f", av);
		return true;

		TESObjectWEAP* eqWeap = DYNAMIC_CAST(actor->equipData->object[41].parent.object, TESForm, TESObjectWEAP);
		if (!eqWeap)
			return false;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(actor->equipData->object[41].parent.instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData)
			return false;
		if (!actor->middleProcess || !actor->middleProcess->unk08->equipData.entries || !actor->middleProcess->unk08->equipData[0].equippedData)
			return false;
		if (actor->middleProcess->unk08->equipData[0].equippedData->unk18 <= 0)
			return false; //if resetshotcountsonreload
		//isAnimPlaying
		UInt16 wait = (UInt16)roundp(Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeTime->formID));
		if (wait < 50)
			return false;
		// check if actor has burst data w/ iterator, if not, make new one
		BurstMode* data = nullptr;
		data->actor = actor;
		data->wait = wait;
		//
		//if (MSF_MainData::tmr.IsRunning())
		//	return false;
		//MSF_MainData::tmr.start();
		data->animReady = false;
		SInt32 state = 8;
		if (actor == *g_player)
		{
			PlayerCamera* playerCamera = *g_playerCamera;
			state = playerCamera->GetCameraStateId(playerCamera->cameraState);
			data->flags = (UInt8)roundp(Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeFlags->formID));
		}
		if (state == 0)
		{
			data->fireIdle = MSF_Data::GetFireAnimation(eqWeap, false);
			std::thread(BurstWaitAndShoot, data);
			return true;
		}
		else if (state == 7 || state == 8)
		{
			data->fireIdle = MSF_Data::GetFireAnimation(eqWeap, true);
			std::thread(BurstWaitAndShoot, data);
			return true;
		}
	}
*/