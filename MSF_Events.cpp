#include "MSF_Events.h"
#include "MSF_WeaponState.h"

HUDShowAmmoCounter HUDShowAmmoCounter_Copied;
_UpdateAnimGraph EquipHandler_UpdateAnimGraph_Copied;
_AttachModToStack AttachModToStack_CallFromGameplay_Copied;
_AttachModToStack AttachModToStack_CallFromWorkbenchUI_Copied;
_DeleteExtraData DeleteExtraData_CallFromWorkbenchUI_Copied;
BGSOnPlayerUseWorkBenchEventSink useWorkbenchEventSink;
BGSOnPlayerModArmorWeaponEventSink modArmorWeaponEventSink;
TESCellFullyLoadedEventSink cellFullyLoadedEventSink;
PlayerAmmoCountEventSink playerAmmoCountEventSink;
MenuOpenCloseSink menuOpenCloseSink;
_PlayerAnimationEvent PlayerAnimationEvent_Original;

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
	MSF_Base::SpawnRandomMods(evn->cell);
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
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(*g_player, 41);
		if (instanceData != MSF_MainData::modSwitchManager.GetCurrentWeapon())
		{
			MSF_MainData::modSwitchManager.SetCurrentWeapon(instanceData);
			MSF_MainData::modSwitchManager.ClearQueue();
			MSF_MainData::modSwitchManager.CloseOpenedMenu();
			MSF_Scaleform::UpdateWidgetData();
			//if (MSF_MainData::activeBurstManager)
			//	MSF_MainData::activeBurstManager->flags &= ~BurstModeData::bActive;
			//if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModBurstKW))
			//{
			//	BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41);
			//	BGSMod::Attachment::Mod* mod = Utilities::FindModByUniqueKeyword(modData, MSF_MainData::FiringModBurstKW);
			//	if (mod)
			//	{
			//		auto it = MSF_MainData::burstModeData.find(mod);
			//		if (it != MSF_MainData::burstModeData.end());
			//		{
			//			BurstModeData* burstMode = it->second;

			//			if (MSF_MainData::activeBurstManager)
			//				delete MSF_MainData::activeBurstManager;
			//			MSF_MainData::activeBurstManager = new BurstModeManager(burstMode, true);
			//			//MSF_MainData::activeBurstManager->HandleEquipEvent(instanceData);
			//		}
			//	}
			//}
		}
	}

	return kEvent_Continue;
}

EventResult	PlayerInventoryListEventSink::ReceiveEvent(BGSInventoryListEventData::Event* evn, void * dispatcher)
{
	if (!evn)
		return kEvent_Continue;
	switch (evn->type)
	{
	case BGSInventoryListEventData::kAddStack: {}; //_MESSAGE("kAddStack"); }; //
	case BGSInventoryListEventData::kChangedStack: {}; //_MESSAGE("kChangedStack"); };
	case BGSInventoryListEventData::kAddNewItem: {}; //_MESSAGE("kAddNewItem"); };
	case BGSInventoryListEventData::kRemoveItem: {
		//_MESSAGE("kRemoveItem"); 
		//_MESSAGE("obj: %p", evn->objAffected);
		//_MESSAGE("own: %p", evn->owner);
		//Actor* player = *g_player;
		//_MESSAGE("count: %08X", (*g_player)->handleRefObject.GetRefCount());
		//_MESSAGE("ownRef2Handle: %p", GetHandle((TESObjectREFR**)&player));
		//_MESSAGE("count: %08X", (*g_player)->handleRefObject.GetRefCount());
		//TESObjectREFR* ref = nullptr;
		//bool ret = GetNiSmartPointer(evn->owner, ref);
		//_MESSAGE("ownHandle2RefNi: %02X, %p", ret, ref);
		//_MESSAGE("count: %08X", (*g_player)->handleRefObject.GetRefCount());
		//ref = nullptr;
		//ret = GetSmartPointer(evn->owner, ref);
		//_MESSAGE("ownHandle2Ref: %02X, %p", ret, ref);
	};
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
			MSF_MainData::modSwitchManager.CloseOpenedMenu();
			MSFMenu::CloseMenu();
			MSFWidgetMenu::CloseMenu();

		}
		else
		{
			//_MESSAGE("OpenMSFMenu from MenuOpenCloseHandler");
			MSFMenu::OpenMenu();
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

bool RegisterInventoryEvent(BGSInventoryList* list, BSTEventSink<BGSInventoryListEventData::Event>* sink)
{
	if (!list || !sink)
		return false;
	InventoryList* inv = (InventoryList*)list;
	return inv->eventSource.AddEventSink(sink);
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
	return HUDShowAmmoCounter_Copied(ammoCounter, visibleTime);
	//return HUDShowAmmoCounter_Original(ammoCounter, visibleTime);
}

void* EquipHandler_UpdateAnimGraph_Hook(Actor* actor, bool unk_rdx)
{
	if (MSF_MainData::modSwitchManager.GetIgnoreAnimGraph())
		MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(false);
	else
		return EquipHandler_UpdateAnimGraph_Copied(actor, unk_rdx);
		//return UpdateAnimGraph(actor, unk_rdx);
	return 0;
}

bool AttachModToStack_CallFromGameplay_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* modFunctor, UInt32 unk_r9d, UInt32* unk_rsp20)
{
	if (MSF_MainData::GameIsLoading || (*g_ui)->IsMenuOpen("LoadingMenu"))
		return AttachModToStack_CallFromGameplay_Copied(invItem, IDfunctor, modFunctor, unk_r9d, unk_rsp20);

	ModifyModDataFunctor* modWriteFunctor = (ModifyModDataFunctor*)modFunctor;

	_MESSAGE("CALL from Gameplay");
	return AttachModToStack_CallFromGameplay_Copied(invItem, IDfunctor, modFunctor, unk_r9d, unk_rsp20);
}

bool AttachModToStack_CallFromWorkbenchUI_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* changesFunctor, UInt32 unk_r9d, UInt32* unk_rsp20)
{
	if (MSF_MainData::GameIsLoading || (*g_ui)->IsMenuOpen("LoadingMenu"))
		return AttachModToStack_CallFromWorkbenchUI_Copied(invItem, IDfunctor, changesFunctor, unk_r9d, unk_rsp20);

	CheckStackIDFunctor newIDfunctor = CheckStackIDFunctor(IDfunctor->stackID);
	CheckStackIDFunctor newIDfunctor2 = CheckStackIDFunctor(IDfunctor->stackID);
	bool result = AttachModToStack_CallFromWorkbenchUI_Copied(invItem, IDfunctor, changesFunctor, unk_r9d, unk_rsp20);

	ApplyChangesFunctor* applyChangesFunctor = (ApplyChangesFunctor*)changesFunctor;
	if (applyChangesFunctor && applyChangesFunctor->mod && applyChangesFunctor->foundObject && applyChangesFunctor->moddata)
	{
		BGSMod::Attachment::Mod* attachedMod = applyChangesFunctor->mod;
		if (attachedMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
			return result;
		TESObjectWEAP* weapon = DYNAMIC_CAST(applyChangesFunctor->foundObject, TESBoundObject, TESObjectWEAP);
		BGSObjectInstanceExtra* moddata = applyChangesFunctor->moddata;
		_MESSAGE("moddata: %p", moddata);
		_MESSAGE("has: %02X", Utilities::HasObjectMod(moddata, MSF_MainData::APbaseMod));
		
		UInt32 unk = 0;
		ApplyChangesFunctor newChangesFunctor = ApplyChangesFunctor(applyChangesFunctor->foundObject, applyChangesFunctor->moddata, MSF_MainData::APbaseMod, applyChangesFunctor->ignoreWeapon, applyChangesFunctor->remove, applyChangesFunctor->equipLocked, applyChangesFunctor->setExtraData);
		MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
		AttachModToStack_CallFromWorkbenchUI_Copied(invItem, &newIDfunctor, &newChangesFunctor, unk_r9d, &unk);

		_MESSAGE("has: %02X", Utilities::HasObjectMod(moddata, MSF_MainData::APbaseMod));


		_MESSAGE("size: %08X, params: %02X, %02X, %02X, %02X, %04X, %02X", moddata->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form), applyChangesFunctor->shouldSplitStacks, applyChangesFunctor->transferEquippedToSplitStack, applyChangesFunctor->ignoreWeapon, applyChangesFunctor->remove, applyChangesFunctor->equipLocked, applyChangesFunctor->setExtraData);
		std::vector<BGSMod::Attachment::Mod*> invalidMods;
		MSF_Base::GetInvalidMods(&invalidMods, moddata, weapon, attachedMod);
		BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(moddata, weapon); //remove!
		_MESSAGE("invalid: %08X", invalidMods.size());
		if (invalidAmmoMod)
		{
			_MESSAGE("ammomod: %s", invalidAmmoMod->GetFullName());
			MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			UInt32 unk = 0;
			ApplyChangesFunctor removeChangesFunctor = ApplyChangesFunctor(applyChangesFunctor->foundObject, applyChangesFunctor->moddata, invalidAmmoMod, applyChangesFunctor->ignoreWeapon, true, applyChangesFunctor->equipLocked, applyChangesFunctor->setExtraData);
			MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachModToStack_CallFromWorkbenchUI_Copied(invItem, &newIDfunctor2, &removeChangesFunctor, unk_r9d, &unk);
		}

		for (UInt32 i = 0; i < moddata->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			_MESSAGE("form: %08X", moddata->data->forms[i].formId);
		}
	}
	//reevaluate
//ApplyChangesFunctor newChangesFunctor = ApplyChangesFunctor(applyChangesFunctor->foundObject, applyChangesFunctor->moddata, applyChangesFunctor->mod, applyChangesFunctor->unk28, applyChangesFunctor->unk29, applyChangesFunctor->unk2A, applyChangesFunctor->unk2B);

	return result;
}

bool DeleteExtraData_CallFromWorkbenchUI_Hook(BSExtraData** extraDataHead, ExtraDataType type)
{
	if (MSF_MainData::modSwitchManager.GetIgnoreDeleteExtraData())
		MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(false);
	else
		return DeleteExtraData_CallFromWorkbenchUI_Copied(extraDataHead, type);
	return false;
}

bool ExtraRankCompare_Hook(ExtraRank* extra1, ExtraRank* extra2) //ctor: B8670 v. A9F60 v. 9DC03
{
	ExtraWeaponState* state1 = MSF_MainData::weaponStateStore.Get(extra1->rank);
	ExtraWeaponState* state2 = MSF_MainData::weaponStateStore.Get(extra2->rank);
	if (state1 || state2)
		return false;
	else
		return extra1->rank != extra2->rank;
}

UInt8 PlayerAnimationEvent_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3)
{
	const char* name = arg2->eventName.c_str();
	if (!_strcmpi("reloadComplete", name))
	{
		if (MSF_MainData::activeBurstManager && (MSF_MainData::activeBurstManager->flags & BurstModeData::bActive))
			MSF_MainData::activeBurstManager->ResetShotsOnReload();
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			//_MESSAGE("reloadCOMP %02X", switchData->SwitchFlags);
			if (switchData->SwitchFlags & SwitchData::bReloadInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bReloadInProgress;
				MSF_MainData::modSwitchManager.SetState(ModSwitchManager::bState_ReloadNotFinished);
				MSF_Base::SwitchMod(switchData, true);
				//_MESSAGE("switchOK");
			}
		}
	}
	if (!_strcmpi("reloadEnd", name))
	{
		//_MESSAGE("reloadEnd");
		if (MSF_MainData::modSwitchManager.GetState() & ModSwitchManager::bState_ReloadNotFinished)
		{
			UInt16 endFlag = ~ModSwitchManager::bState_ReloadNotFinished;
			delayTask delayEnd(100, true, &MSF_Base::EndSwitch, endFlag);
		}
	}
	else if (!_strcmpi("weaponDraw", name))
	{
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			//_MESSAGE("drawFlags: %08X", switchData->SwitchFlags);
			if (switchData->SwitchFlags & SwitchData::bDrawInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bDrawInProgress;
				if ((switchData->SwitchFlags & SwitchData::bReloadNeeded))
				{
					//_MESSAGE("reloading");
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
	//on sheath: MSF_MainData::modSwitchManager.CloseOpenedMenu();
	else if (!_strcmpi("weaponFire", name))
	{
		if (MSF_MainData::activeBurstManager && (MSF_MainData::activeBurstManager->flags & BurstModeData::bActive))
			MSF_MainData::activeBurstManager->HandleFireEvent();
		
		//if (MSF_MainData::tmr.IsRunning())
		//{
		//	if (MSF_MainData::tmr.stop() < 1000)
		//		MSF_Base::FireBurst(*g_player);
		//}
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
	}
	else if (!_strcmpi("customAnimEnd", name))
	{
		if (MSF_MainData::modSwitchManager.GetState() & ModSwitchManager::bState_AnimNotFinished)
		{
			UInt16 endFlag = ~ModSwitchManager::bState_AnimNotFinished;
			delayTask delayEnd(100, true, &MSF_Base::EndSwitch, endFlag);
		}
	}
	else if (!_strcmpi("toggleMenu", name))
	{
	}
	return PlayerAnimationEvent_Original(arg1, arg2, arg3);
}