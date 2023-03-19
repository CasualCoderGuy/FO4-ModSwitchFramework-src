#include "MSF_Events.h"

//RelocAddr <AttackBlockHandler> AttackBlockHandler_HookTarget(0x0F4B24C);
//RelocAddr <AttackBlockHandler> AttackBlockHandler_Original(0x0F48D00);
RelocAddr <AttackBlockHandler> AttackBlockHandler_HookTarget(0x0F494DA);
RelocAddr <AttackBlockHandler> AttackBlockHandler_Original(0x0F4B080);
RelocAddr <HUDShowAmmoCounter> HUDShowAmmoCounter_HookTarget(0x0A0E9D2);
RelocAddr <HUDShowAmmoCounter> HUDShowAmmoCounter_Original(0x0A22D00);
RelocPtr <UInt32> uAmmoCounterFadeTimeMS(0x375CF30); //A0E9C9
RelocAddr <_tf1> tf1_HookTarget(0x2D442E0);

BGSOnPlayerUseWorkBenchEventSink useWorkbenchEventSink;
BGSOnPlayerModArmorWeaponEventSink modArmorWeaponEventSink;
TESCellFullyLoadedEventSink cellFullyLoadedEventSink;
PlayerAmmoCountEventSink playerAmmoCountEventSink;
MenuOpenCloseSink menuOpenCloseSink;
_tf1 tf1_Original;

EventResult	BGSOnPlayerUseWorkBenchEventSink::ReceiveEvent(BGSOnPlayerUseWorkBenchEvent* evn, void * dispatcher)
{
	return kEvent_Continue;
}

EventResult	BGSOnPlayerModArmorWeaponEventSink::ReceiveEvent(BGSOnPlayerModArmorWeaponEvent * evn, void * dispatcher)
{
	TESObjectWEAP* moddedWeap = DYNAMIC_CAST(evn->object, TESBoundObject, TESObjectWEAP);
	if (moddedWeap)
		delayTask delay(100, true, &MSF_Base::ReevalModdedWeapon, moddedWeap);
	return kEvent_Continue;
}

EventResult	TESCellFullyLoadedEventSink::ReceiveEvent(TESCellFullyLoadedEvent * evn, void * dispatcher)
{
	//MSF_Base::SpawnRandomMods(evn->cell);
	//_MESSAGE("Cell evn");
	return kEvent_Continue;
}

EventResult CombatEvnHandler::ReceiveEvent(TESCombatEvent * evn, void * dispatcher)
{
	//instance midprocess ammo count!
	_MESSAGE("combat started");
	return kEvent_Continue;
}

EventResult PlayerAmmoCountEventSink::ReceiveEvent(PlayerAmmoCountEvent * evn, void * dispatcher)
{
	if (evn->weaponInstance != MSF_MainData::modSwitchManager.GetCurrentWeapon())
	{
		MSF_MainData::modSwitchManager.SetCurrentWeapon(evn->weaponInstance);
		MSF_MainData::modSwitchManager.ClearQueue();
		//close menu
		MSF_Scaleform::UpdateWidgetData();
	}

	return kEvent_Continue;
}

EventResult	MenuOpenCloseSink::ReceiveEvent(MenuOpenCloseEvent * evn, void * dispatcher)
{
	const char* name = evn->menuName.c_str();
	//if (!_strcmpi("pipboyMenu", name))
	//{
	//	if (!evn->isOpen && (MSF_MainData::switchData.SwitchFlags & SwitchData::bNeedInit))
	//		MSF_Base::InitWeapon();
	//}
	if (!_strcmpi("pauseMenu", name) && !evn->isOpen)
	{
		MSF_Data::ReadMCMKeybindData();
		MSF_Data::ReadUserSettings();
	}
	else if (!_strcmpi("ContainerMenu", evn->menuName.c_str()) || !_strcmpi("BarterMenu", evn->menuName.c_str()) || \
		!_strcmpi("ExamineMenu", evn->menuName.c_str()) || !_strcmpi("WorkshopMenu", evn->menuName.c_str()) || \
		!_strcmpi("Workshop_CaravanMenu", evn->menuName.c_str()) || !_strcmpi("LevelUpMenu", evn->menuName.c_str()) || \
		!_strcmpi("BookMenu", evn->menuName.c_str()) || !_strcmpi("CookingMenu", evn->menuName.c_str()) || \
		!_strcmpi("LoadingMenu", evn->menuName.c_str()) || !_strcmpi("PipboyMenu", evn->menuName.c_str()))
	{
		if (evn->isOpen)
		{
			MSF_MainData::modSwitchManager.IncOpenedMenus();
		}
		else if (MSF_MainData::modSwitchManager.GetOpenedMenus() > 0)
		{
			MSF_MainData::modSwitchManager.DecOpenedMenus();
		}
		if (MSF_MainData::modSwitchManager.GetOpenedMenus() > 0)
		{
			//_MESSAGE("CloseMSFMenu");
			//MSFMenu::CloseMenu();
			MSFWidgetMenu::CloseMenu();
			MSFAmmoMenu::CloseMenu();
			MSFModMenu::CloseMenu();

		}
		else
		{
			//_MESSAGE("OpenMSFMenu from MenuOpenCloseHandler");
			//MSFMenu::OpenMenu();
			MSFWidgetMenu::OpenMenu();
		}
	}

	return kEvent_Continue;
}

BSTEventDispatcher<void*>* GetGlobalEventDispatcher(BSTGlobalEvent* globalEvents, const char * dispatcherName)
{
	for (int i = 0; i < globalEvents->eventSources.count; i++) {
		const char* name = GetObjectClassName(globalEvents->eventSources[i]) + 15;    // ?$EventSource@V
		if (strstr(name, dispatcherName) == name) {
			return &globalEvents->eventSources[i]->eventDispatcher;
		}
	}
	return nullptr;
}

void* AttackBlockHandler_Hook(void* handler)
{
	_MESSAGE("Attack");
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bWidgetAlwaysVisible))
		return nullptr;
	return AttackBlockHandler_Original(handler);
}

UInt64 HUDShowAmmoCounter_Hook(HUDAmmoCounter* ammoCounter, UInt32 visibleTime)
{
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bWidgetAlwaysVisible))
		MSF_Scaleform::StartWidgetHideCountdown(visibleTime);
	return HUDShowAmmoCounter_Original(ammoCounter, visibleTime);
}

volatile short count = 3;

UInt8 tf1_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3)
{
	const char* name = arg2->eventName.c_str();
	if (!_strcmpi("reloadComplete", name))
	{
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			_MESSAGE("reloadCOMP %02X", switchData->SwitchFlags);
			if (switchData->SwitchFlags & SwitchData::bReloadInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bReloadInProgress;
				MSF_MainData::modSwitchManager.SetState(ModSwitchManager::bState_ReloadNotFinished);
				MSF_Base::SwitchMod(switchData, true);
				_MESSAGE("switchOK");
			}
		}
	}
	if (!_strcmpi("reloadEnd", name))
	{
		_MESSAGE("reloadEnd");
		if (MSF_MainData::modSwitchManager.GetState() & ModSwitchManager::bState_ReloadNotFinished)
		{
			UInt16 endFlag = ~ModSwitchManager::bState_ReloadNotFinished;
			delayTask delayEnd(10, true, &MSF_Base::EndSwitch, endFlag);
		}
	}
	else if (!_strcmpi("weaponDraw", name))
	{
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			_MESSAGE("drawFlags: %08X", switchData->SwitchFlags);
			if (switchData->SwitchFlags & SwitchData::bDrawInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bDrawInProgress;
				if ((switchData->SwitchFlags & SwitchData::bReloadNeeded))
				{
					_MESSAGE("reloading");
					switchData->SwitchFlags = (switchData->SwitchFlags & ~SwitchData::bReloadNeeded) | SwitchData::bReloadInProgress; // | SwitchData::bReloadNotFinished
					delayTask delayReload(400, true, &MSF_Base::ReloadWeapon);
					//if (!MSF_Base::ReloadWeapon())
					//	MSF_MainData::modSwitchManager.ClearQueue();
				}
				else if (SwitchData::bAnimNeeded)
				{
					switchData->SwitchFlags = (switchData->SwitchFlags & ~SwitchData::bAnimNeeded) | SwitchData::bAnimInProgress; //| SwitchData::bAnimNotFinished
					if (!MSF_Base::PlayAnim(switchData->animData))
						MSF_MainData::modSwitchManager.ClearQueue();
				}
			}
		}
	}
	else if (!_strcmpi("weaponFire", name))
	{
		//if (MSF_MainData::tmr.IsRunning())
		//{
		//	if (MSF_MainData::tmr.stop() < 1000)
		//		MSF_Base::FireBurst(*g_player);
		//}
		//InterlockedDecrement16(&count);
		//if (count)
		//	delayTask delayReload(80, true, &Utilities::PlayIdle, *g_player, MSF_MainData::fireIdle1stP);
		//	//Utilities::PlayIdle(*g_player, MSF_MainData::fireIdle1stP);
		//else
		//	InterlockedExchange16(&count, 3);
		//_MESSAGE("Anim: fire %i", MSF_MainData::tmr.stop());
	}
	else if (!_strcmpi("switchMod", name))
	{
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			if (switchData->SwitchFlags & SwitchData::bAnimInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bAnimInProgress;
				MSF_MainData::modSwitchManager.SetState(ModSwitchManager::bState_ReloadNotFinished);
				MSF_Base::SwitchMod(switchData, true);
			}
		}
		//_MESSAGE("Anim: switch");
	}
	else if (!_strcmpi("customAnimEnd", name))
	{
		if (MSF_MainData::modSwitchManager.GetState() & ModSwitchManager::bState_AnimNotFinished)
		{
			UInt16 endFlag = ~ModSwitchManager::bState_AnimNotFinished;
			delayTask delayEnd(10, true, &MSF_Base::EndSwitch, endFlag);
		}
	}
	return tf1_Original(arg1, arg2, arg3);
}