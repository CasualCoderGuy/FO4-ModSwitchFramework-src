#include "MSF_Events.h"

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
	if ((*g_player)->equipData->slots[41].instanceData)
	{
		if (evn->weaponInstance && evn->weaponInstance != MSF_MainData::switchData.equippedInstanceData)
		{
			MSF_MainData::switchData.equippedInstanceData = evn->weaponInstance;
			//if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::APbaseMod))
			//{
			//	MSF_MainData::switchData.ClearData();
			//	MSF_MainData::switchData.SwitchFlags = SwitchData::bNeedInit;
			//	if (!(*g_ui)->IsMenuOpen(BSFixedString("pipboyMenu"))) //(!(*g_ui)->IsMenuOpen(BSFixedString("CursorMenu"))
			//		MSF_Base::InitWeapon();
			//}
			MSF_Scaleform::UpdateWidgetData();
			//MSFWidgetMenu::OpenMenu();
		}
	}
	else
	{
		MSF_MainData::switchData.equippedInstanceData = nullptr;
		MSF_MainData::switchData.ClearData();
		MSF_Scaleform::UpdateWidgetData();
		//MSFWidgetMenu::OpenMenu();
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
			MSF_MainData::numberOfOpenedMenus++;
		}
		else if (MSF_MainData::numberOfOpenedMenus > 0)
		{
			MSF_MainData::numberOfOpenedMenus--;
		}
		if (MSF_MainData::numberOfOpenedMenus > 0)
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

UInt64 HUDShowAmmoCounter_Hook(HUDAmmoCounter* ammoCounter, UInt32 visibleTime)
{
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bWidgetAlwaysVisible))
		MSF_Scaleform::StartWidgetHideCountdown(visibleTime);
	return HUDShowAmmoCounter_Original(ammoCounter, visibleTime);;
}

UInt8 tf1_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3)
{
	const char* name = arg2->eventName.c_str();
	if (!_strcmpi("reloadComplete", name))
	{
		_MESSAGE("reloadCOMP %02X", MSF_MainData::switchData.SwitchFlags);
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bReloadInProgress)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bReloadInProgress;
			MSF_Base::SwitchMod();
		}
	}
	if (!_strcmpi("reloadEnd", name))
	{
		_MESSAGE("reloadEnd");
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bReloadInProgress)
		{
			UInt16 endFlag = ~SwitchData::bReloadNotFinished;
			delayTask delayEnd(10, true, &MSF_Base::SwitchFlagsAND, endFlag);
		}
	}
	else if (!_strcmpi("weaponDraw", name))
	{
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bDrawInProgress)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bDrawInProgress;
			if ((MSF_MainData::switchData.SwitchFlags & SwitchData::bReloadNeeded))
			{
				MSF_MainData::switchData.SwitchFlags = (MSF_MainData::switchData.SwitchFlags & ~SwitchData::bReloadNeeded) | SwitchData::bReloadInProgress; // | SwitchData::bReloadNotFinished
				if (!MSF_Base::ReloadWeapon())
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bReloadInProgress | SwitchData::bReloadNotFinished);
			}
			else if (SwitchData::bAnimNeeded)
			{
				MSF_MainData::switchData.SwitchFlags = (MSF_MainData::switchData.SwitchFlags & ~SwitchData::bAnimNeeded) | SwitchData::bAnimInProgress; //| SwitchData::bAnimNotFinished
				if (!MSF_Base::PlayAnim())
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
			}
		}
	}
	else if (!_strcmpi("weaponFire", name))
	{
		if (MSF_MainData::tmr.IsRunning())
		{
			if (MSF_MainData::tmr.stop() < 1000)
				MSF_Base::FireBurst(*g_player);
		}
		//InterlockedDecrement16(&count);
		//if (count)
		//	Utilities::PlayIdle(*g_player, MSF_MainData::fireIdle1stP);
		//else
		//	InterlockedExchange16(&count, 3);
		//_MESSAGE("Anim: fire %i", MSF_MainData::tmr.stop());
	}
	else if (!_strcmpi("switchMod", name))
	{
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bAnimInProgress)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bAnimInProgress;
			MSF_Base::SwitchMod();
		}
		//_MESSAGE("Anim: switch");
	}
	else if (!_strcmpi("customAnimEnd", name))
	{
		UInt16 endFlag = ~SwitchData::bAnimNotFinished;
		delayTask delayEnd(10, true, &MSF_Base::SwitchFlagsAND, endFlag);
	}
	return tf1_Original(arg1, arg2, arg3);
}