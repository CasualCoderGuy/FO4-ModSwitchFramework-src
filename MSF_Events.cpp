#include "MSF_Events.h"
#include "MSF_WeaponState.h"

AttackInputHandler AttackInputHandler_Copied;
AttackInputHandler AttackInputHandlerSelf_Copied;
uintptr_t ActorEquipManagerPre_Copied;
HUDShowAmmoCounter HUDShowAmmoCounter_Copied;
_UpdateAnimGraph EquipHandler_UpdateAnimGraph_Copied;
_AttachModToStack AttachModToStack_CallFromGameplay_Copied;
_AttachModToStack AttachModToStack_CallFromWorkbenchUI_Copied;
_DeleteExtraData DeleteExtraData_CallFromWorkbenchUI_Copied;
_UpdateEquipData UpdateEquipData_Copied;
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
	//TESObjectWEAP* moddedWeap = DYNAMIC_CAST(evn->object, TESBoundObject, TESObjectWEAP);
	//if (moddedWeap)
	//	delayTask delay(100, true, &MSF_Base::ReevalModdedWeapon, moddedWeap);
	return kEvent_Continue;
}

EventResult	TESCellFullyLoadedEventSink::ReceiveEvent(TESCellFullyLoadedEvent * evn, void * dispatcher)
{
	MSF_Base::SpawnRandomMods(evn->cell);
	//_DEBUG("Cell evn");
	return kEvent_Continue;
}

EventResult CombatEvnHandler::ReceiveEvent(TESCombatEvent * evn, void * dispatcher)
{
	//instance midprocess ammo count!
	//_DEBUG("combat started");
	return kEvent_Continue;
}

void HelperFn(ActorEquipManagerEvent::Event* evn)
{
	_DEBUG("equipEvent");
}

EventResult	ActorEquipManagerEventSink::ReceiveEvent(ActorEquipManagerEvent::Event* evn, void* dispatcher)
{
	//return kEvent_Continue;
	if (!evn->data || (evn->targetActor != *g_player))
		return kEvent_Continue;
	_DEBUG("equipEvent item: %p, instance: %p, equipslot: %p, data: %p, bEquip: %02X, bPA: %02X", evn->data->equippedItem, evn->data->instancedata, evn->data->equipSlot, evn->data->equippedWeaponData, evn->equip, IsInPowerArmor(*g_player));
	MSF_MainData::modSwitchManager.HandlePAEvent();
	TESObjectWEAP* eventWeapon = DYNAMIC_CAST(evn->data->equippedItem, TESBoundObject, TESObjectWEAP);
	if (!eventWeapon)
		return kEvent_Continue;
	if (!MSF_MainData::modSwitchManager.GetModChangeEvent())
	{
		MSF_MainData::modSwitchManager.ClearQueue();
		MSF_MainData::modSwitchManager.CloseOpenedMenu();
	}
	TESObjectWEAP::InstanceData* eventInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(evn->data->instancedata, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	//TESObjectWEAP::InstanceData* equippedInstanceData = Utilities::GetEquippedInstanceData(*g_player);
	//MSF_Scaleform::UpdateWidgetData(eventInstanceData);
	//delayTask delayUpd(250, true, &MSF_Scaleform::UpdateWidgetData, nullptr);
	WidgetUpdateTask* updTask = new WidgetUpdateTask();
	delayTask delayUpd(250, true, g_threading->AddUITask, updTask);
	if (!eventInstanceData)// || !equippedInstanceData || (eventInstanceData != equippedInstanceData))
		return kEvent_Continue;
	//EquipWeaponData* equipData = evn->data->equippedWeaponData;
	//if (!equipData)
	//	return kEvent_Continue;
	//ExtraDataList* equippedExtraData = nullptr;
	//(*g_player)->GetEquippedExtraData(41, &equippedExtraData);
	//if (!equippedExtraData)
	//	return kEvent_Continue;
	_DEBUG("eqStart");
	//ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeEquip);
	//MSF_MainData::modSwitchManager.SetEquipEvent(true);
	//HelperFn(evn);

	return kEvent_Continue;
}

void ActorEquipManagerPre_Hook(Actor* owner, BGSObjectInstance* object)
{

	ExtraDataList* dataList = nullptr;
	if (!owner->inventoryList || !object || !object->object || !object->instanceData)
		return;
	if (object->object->formType != kFormType_WEAP)
		return;
	for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
	{
		if (owner->inventoryList->items.entries[i].form == object->object)
		{
			for (auto stack = owner->inventoryList->items.entries[i].stack; stack; stack = stack->next)
			{
				if (!stack->extraData)
					continue;
				ExtraInstanceData* eidata = (ExtraInstanceData*)stack->extraData->GetByType(ExtraDataType::kExtraData_InstanceData);
				if (eidata && eidata->instanceData == object->instanceData)
				{
					dataList = stack->extraData;
					break;
				}
			}
		}
	}
	if (!dataList)
		return;
	MSF_MainData::projectileManager.ApplyMods(dataList);
}

EventResult PlayerAmmoCountEventSink::ReceiveEvent(PlayerAmmoCountEvent * evn, void * dispatcher)
{
	//_DEBUG("ammoCount: %i, totAmmo: %i, instance: %p", evn->ammoCount, evn->totalAmmoCount, evn->weaponInstance);
	ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeAmmoCount);
	//ExtraDataList* extralist = nullptr;
	//(*g_player)->GetEquippedExtraData(41, &extralist);
	//if (!extralist)
	//	return kEvent_Continue;
	//EquipWeaponData* equippedData = Utilities::GetEquippedWeaponData(*g_player);
	//if (!equippedData)
	//	return kEvent_Continue;
	//ExtraWeaponState* weaponState = ExtraWeaponState::Init(extralist, equippedData);
	//weaponState->HandleModChangeEvent(extralist, equippedData);


	//if (evn->weaponInstance != MSF_MainData::modSwitchManager.GetCurrentWeapon())
	//{
	//	TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(*g_player, 41);
	//	if (instanceData != MSF_MainData::modSwitchManager.GetCurrentWeapon())
	//	{
	//		MSF_MainData::modSwitchManager.SetCurrentWeapon(instanceData);
	//		MSF_MainData::modSwitchManager.ClearQueue();
	//		MSF_MainData::modSwitchManager.CloseOpenedMenu();
	//		MSF_Scaleform::UpdateWidgetData();
	//		//if (MSF_MainData::activeBurstManager)
	//		//	MSF_MainData::activeBurstManager->flags &= ~BurstModeData::bActive;
	//		//if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModBurstKW))
	//		//{
	//		//	BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41);
	//		//	BGSMod::Attachment::Mod* mod = Utilities::FindModByUniqueKeyword(modData, MSF_MainData::FiringModBurstKW);
	//		//	if (mod)
	//		//	{
	//		//		auto it = MSF_MainData::burstModeData.find(mod);
	//		//		if (it != MSF_MainData::burstModeData.end());
	//		//		{
	//		//			BurstModeData* burstMode = it->second;

	//		//			if (MSF_MainData::activeBurstManager)
	//		//				delete MSF_MainData::activeBurstManager;
	//		//			MSF_MainData::activeBurstManager = new BurstModeManager(burstMode, true);
	//		//			//MSF_MainData::activeBurstManager->HandleEquipEvent(instanceData);
	//		//		}
	//		//	}
	//		//}
	//	}
	//}

	return kEvent_Continue;
}

EventResult	PlayerInventoryListEventSink::ReceiveEvent(BGSInventoryListEventData::Event* evn, void * dispatcher)
{
	if (!evn)
		return kEvent_Continue;
	switch (evn->changeType)
	{
	case BGSInventoryListEventData::kAddStack: {} break; //_DEBUG("kAddStack"); }; //
	case BGSInventoryListEventData::kChangedStack: {} break; //_DEBUG("kChangedStack"); };
	case BGSInventoryListEventData::kAddNewItem: {} break; //_DEBUG("kAddNewItem"); };
	case BGSInventoryListEventData::kRemoveItem: {
		//_DEBUG("kRemoveItem"); 
		//_DEBUG("obj: %p", evn->objAffected);
		//_DEBUG("own: %p", evn->owner);
		//Actor* player = *g_player;
		//_DEBUG("count: %08X", (*g_player)->handleRefObject.GetRefCount());
		//ObjectRefHandle handle = 0;
		//_DEBUG("ownRef2Handle: %p", GetHandle(&handle, (TESObjectREFR*)player));
		//_DEBUG("count: %08X", (*g_player)->handleRefObject.GetRefCount());
		//TESObjectREFR* ref = nullptr;
		//bool ret = GetNiSmartPointer(&evn->owner, &ref);
		//if (ref)
		//	ref->handleRefObject.DecRefHandle();
		//_DEBUG("ownHandle2RefNi: %02X, %p", ret, ref);
		//_DEBUG("count: %08X", (*g_player)->handleRefObject.GetRefCount());

		//ref = nullptr;
		//ret = GetSmartPointer(evn->owner, ref);
		//_DEBUG("ownHandle2Ref: %02X, %p", ret, ref);
	}
	break;
	}
	return kEvent_Continue;
}

EventResult	MenuOpenCloseSink::ReceiveEvent(MenuOpenCloseEvent * evn, void * dispatcher)
{
	const char* name = evn->menuName.c_str();
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
			//_DEBUG("CloseMSFMenu");
			MSF_MainData::modSwitchManager.CloseOpenedMenu();
			MSFMenu::CloseMenu();
			MSFWidgetMenu::CloseMenu();

		}
		else
		{
			//_DEBUG("OpenMSFMenu from MenuOpenCloseHandler");
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
	return AttackBlockHandler_Original(handler);
}

bool AttackInputHandler_Hook(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d)
{
	//_DEBUG("Attack");
	//if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bWidgetAlwaysVisible))
	//	return false;
	return AttackInputHandler_Original(PlayerInputHandler, inputCode, r8d);
}

bool AttackInputHandlerReload_Hook(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d)
{
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bDisableAutomaticReload))
		return false;
	return AttackInputHandler_Original(PlayerInputHandler, inputCode, r8d);
}

//bool UpdMidProc_Hook(Actor::AIProcess* midProc, Actor* actor, BGSObjectInstance weaponBaseStruct, BGSEquipSlot* equipSlot)
//{
//	_DEBUG("UpdMidProc_Hook");
//	UpdMidProc_Copied(midProc, actor, weaponBaseStruct, equipSlot);
//}

void UpdateEquipData_Hook(BipedAnim* equipData, BGSObjectInstance instance, UInt32* r8d)
{
	//_DEBUG("UpdateEquipData_Hook1");
	UpdateEquipData_Copied(equipData, instance, r8d);
	if (MSF_MainData::GameIsLoading)
		return;
	//EquipWeaponData* eqdata = Utilities::GetEquippedWeaponData(*g_player);
	//if (eqdata)
	//{
	//	_DEBUG("eq: %i", eqdata->loadedAmmoCount);
	//}
	//if (r8d)
	//	_DEBUG("eq r8d: %i", *r8d);
	if (equipData == (*g_player)->biped.get())
	{
		_DEBUG("UpdateEquipData_Hook");
		TESObjectWEAP* eqWeap = DYNAMIC_CAST(instance.object, TESBoundObject, TESObjectWEAP);
		if (eqWeap)
		{
			if (MSF_MainData::modSwitchManager.GetIsBCRreload() != ExtraWeaponState::bEventTypeReloadSwitchBCR)
				MSF_MainData::modSwitchManager.SetIsBCRreload(0);
			else
				MSF_MainData::modSwitchManager.SetIsBCRreload(ExtraWeaponState::bEventTypeReloadFullBCR);
		}
		TESAmmo* ammo = nullptr;
		ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeEquip);
		EquipWeaponData* eqWeapData = Utilities::GetEquippedWeaponData(*g_player);
		if (eqWeapData)
			ammo = eqWeapData->ammo;
		MSF_Base::EquipAmmo((*g_player)->inventoryList, ammo);
	}
	else
	{
		//MSF_MainData::modSwitchManager.SetModChangeEvent(false);
#ifndef NEXTGEN
		TESObjectREFR* objref = nullptr;
		GetNiSmartPointer(&equipData->actorRef, &objref);
#else
		TESObjectREFR* objref = equipData->actorRef.get().get();
#endif
		if (objref)
		{
			objref->handleRefObject.DecRefHandle();
			Actor* owner = DYNAMIC_CAST(objref, TESObjectREFR, Actor);
			if (owner)
				ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeEquip, owner);
		}
	}
}

//void UpdateEquippedWeaponData_Hook(EquippedWeaponData* data)
//{
//	_DEBUG("UpdateEquippedWeaponData_Hook");
//	UpdateEquippedWeaponData_Copied(data);
//}

UInt64 HUDShowAmmoCounter_Hook(HUDAmmoCounter* ammoCounter, UInt32 visibleTime)
{
	if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bWidgetAlwaysVisible))
		MSF_Scaleform::StartWidgetHideCountdown(visibleTime);
	return HUDShowAmmoCounter_Copied(ammoCounter, visibleTime);
	//return HUDShowAmmoCounter_Original(ammoCounter, visibleTime);
}

bool CheckAmmoCountForReload_Hook(Actor* target, UInt32 loadedAmmo, UInt32 ammoCap, UInt32 ammoReserve)
{
	_DEBUG("reloadCheck L: %i, cap: %i, R: %i", loadedAmmo, ammoCap, ammoReserve);
	if (target == *g_player && MSF_MainData::modSwitchManager.GetSetForcedReload())
		return true;
	//ExtraWeaponState* extraState = MSF_MainData::weaponStateStore.GetEquipped(target);
	//if (extraState)
	//{
	//	ExtraWeaponState::AmmoStateData* ammoState = extraState->GetAmmoStateData();
	//	_DEBUG("reloadAS: %i, %02X", ammoState->chamberedCount, (loadedAmmo - ammoState->chamberedCount < ammoCap) && ammoReserve);
	//	if (ammoState)
	//		return ((loadedAmmo - ammoState->chamberedCount < ammoCap) && ammoReserve); //HasTRsupport
	//}
	TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(target);
	if (MSF_Data::InstanceHasTRSupport(instanceData))
	{
		UInt16 chamberSize = 0;
		UInt16 flags = 0;
		MSF_Data::GetChamberData(Utilities::GetEquippedModData(target), instanceData, &chamberSize, &flags);
		return (loadedAmmo < (ammoCap+chamberSize) && loadedAmmo < ammoReserve);
	}
	return (loadedAmmo < ammoCap && loadedAmmo < ammoReserve);
}

const char* CannotEquipItem_Hook(TESObjectREFR* target, TESForm* item, UInt32 unequip, UInt32 type)
{
	if (!item || target != *g_player)
		return type == 2 ? modText : itemText;
	if (item->formType == kFormType_AMMO)
		return MSF_Base::EquipAmmoPipboy((TESAmmo*)item, !unequip);
	if (item->formType == kFormType_MISC)
		return MSF_Base::EquipModPipboy((TESObjectMISC*)item, !unequip);
	return type == 2 ? modText : itemText;
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
	
	if (MSF_MainData::GameIsLoading || (*g_ui)->IsMenuOpen("LoadingMenu") || !invItem)
		return AttachModToStack_CallFromGameplay_Copied(invItem, IDfunctor, modFunctor, unk_r9d, unk_rsp20);

	ModifyModDataFunctor* modWriteFunctor = (ModifyModDataFunctor*)modFunctor;
	UInt32 stackID = IDfunctor->stackID;
	UInt32 unk = *unk_rsp20;
	UInt8 slotIndex = modWriteFunctor->slotIndex;
	_DEBUG("unk: %08X, slot index: %02X", unk, slotIndex);

	bool result = AttachModToStack_CallFromGameplay_Copied(invItem, IDfunctor, modFunctor, unk_r9d, unk_rsp20);
	BGSInventoryItem::Stack* stack = Utilities::GetStack(invItem, stackID);
	if (!stack)
		return result;
	if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
		MSF_MainData::modSwitchManager.SetModChangeEvent(true);
	_DEBUG("stackOK");

	if (modWriteFunctor && modWriteFunctor->mod)
	{
		_DEBUG("functorOK");
		TESObjectWEAP* weapon = DYNAMIC_CAST(invItem->form, TESForm, TESObjectWEAP);
		if (!weapon)
			return result;
		_DEBUG("weaponOK");
		BGSMod::Attachment::Mod* attachedMod = modWriteFunctor->mod;
		//if (attachedMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
		//	return result;
		_DEBUG("typeOK %p", attachedMod);
		ExtraDataList* extraList = stack->extraData;
		if (!extraList)
			return result;
		_DEBUG("extraOK");
		BGSObjectInstanceExtra* moddata = DYNAMIC_CAST(extraList->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		if (!moddata)
			return result;
		_DEBUG("dataOK");

		std::vector<BGSMod::Attachment::Mod*> invalidMods;
		MSF_Base::GetInvalidMods(&invalidMods, moddata, weapon, attachedMod);
		for (auto itMods = invalidMods.begin(); itMods != invalidMods.end(); itMods++)
		{
			BGSMod::Attachment::Mod* invalidMod = *itMods;
			_DEBUG("invalid mod: %08X", invalidMod->formID);
			UInt32 newunk = unk;
			bool success = false;
			ModifyModDataFunctor modifyModFunctor = ModifyModDataFunctor(invalidMod, slotIndex, false, &success);
			AttachModToStack_CallFromGameplay_Copied(invItem, &CheckStackIDFunctor(stackID), &modifyModFunctor, unk_r9d, &newunk);
		}

		//ExtraInstanceData* newExtraInstanceData = DYNAMIC_CAST(extraList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		//TESObjectWEAP::InstanceData* newInstanceData = nullptr;
		//if (newExtraInstanceData)
		//	newInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(newExtraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);

		//BGSMod::Attachment::Mod* targetAmmoMod = nullptr;
		//bool toAttach = false;
		//TESAmmo* targetAmmo = nullptr;
		//ExtraWeaponState* weaponState = ExtraWeaponState::Init(extraList, nullptr);
		//if (weaponState && newInstanceData)
		//{
		//	weaponState->UpdateWeaponStates(extraList, nullptr, newInstanceData->ammo);
		//	targetAmmo = weaponState->GetCurrentAmmo();
		//	_DEBUG("GOTammo: %p", targetAmmo);
		//}
		//else if (newInstanceData)
		//	targetAmmo = newInstanceData->ammo;

		////BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(moddata, weapon); 
		//MSF_Base::GetAmmoModToModify(moddata, targetAmmo, weapon, &targetAmmoMod, &toAttach);
		//if (targetAmmoMod)
		//{
		//	_DEBUG("target ammomod WB: %08X, rem: %02X", targetAmmoMod->formID, toAttach);
		//	UInt32 newunk = unk;
		//	bool success = false;
		//	ModifyModDataFunctor modifyModFunctor = ModifyModDataFunctor(targetAmmoMod, slotIndex, toAttach, &success);
		//	//MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
		//	AttachModToStack_CallFromGameplay_Copied(invItem, &CheckStackIDFunctor(stackID), &modifyModFunctor, unk_r9d, &newunk);
		//}

		std::vector<std::pair<BGSMod::Attachment::Mod*, bool>> stateModsToModify;
		ExtraWeaponState::HandleModChangeEvent(extraList, &stateModsToModify, ExtraWeaponState::kEventTypeModdedGameplay);
		for (auto nextPair : stateModsToModify)
		{
			_DEBUG("target state/ammomod GP: %08X, rem: %02X", nextPair.first->formID, nextPair.second);
			UInt32 newunk = unk;
			bool success = false;
			ModifyModDataFunctor modifyModFunctor = ModifyModDataFunctor(nextPair.first, slotIndex, nextPair.second, &success);
			MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachModToStack_CallFromGameplay_Copied(invItem, &CheckStackIDFunctor(stackID), &modifyModFunctor, unk_r9d, &newunk);
		}

		//BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(moddata, weapon); 
		//if (invalidAmmoMod)
		//{
		//	_DEBUG("invalid ammomod: %08X", invalidAmmoMod->formID);
		//	UInt32 newunk = unk;
		//	bool success = false;
		//	ModifyModDataFunctor modifyModFunctor = ModifyModDataFunctor(invalidAmmoMod, slotIndex, false, &success);
		//	AttachModToStack_CallFromGameplay_Copied(invItem, &CheckStackIDFunctor(stackID), &modifyModFunctor, unk_r9d, &newunk);
		//}
		//ExtraWeaponState* weaponState = ExtraWeaponState::Init(extraList, nullptr);
		//if (!weaponState)
		//	return result;
		//weaponState->UpdateWeaponStatesModded(invItem, stackID);
	}
	return result;
}

bool AttachModToStack_CallFromWorkbenchUI_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* changesFunctor, UInt32 unk_r9d, UInt32* unk_rsp20)
{
	if (MSF_MainData::GameIsLoading || (*g_ui)->IsMenuOpen("LoadingMenu"))
		return AttachModToStack_CallFromWorkbenchUI_Copied(invItem, IDfunctor, changesFunctor, unk_r9d, unk_rsp20);

	UInt32 stackID = IDfunctor->stackID;
	UInt32 unk = *unk_rsp20;

	bool result = AttachModToStack_CallFromWorkbenchUI_Copied(invItem, IDfunctor, changesFunctor, unk_r9d, unk_rsp20);

	ApplyChangesFunctor* applyChangesFunctor = (ApplyChangesFunctor*)changesFunctor;
	if (applyChangesFunctor && applyChangesFunctor->mod && applyChangesFunctor->foundObject && applyChangesFunctor->moddata)
	{
		TESObjectWEAP* weapon = DYNAMIC_CAST(applyChangesFunctor->foundObject, TESBoundObject, TESObjectWEAP);
		if (!weapon)
			return result;
		BGSMod::Attachment::Mod* attachedMod = applyChangesFunctor->mod;
		if (attachedMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
			return result;
		BGSObjectInstanceExtra* moddata = applyChangesFunctor->moddata;

		std::vector<BGSMod::Attachment::Mod*> invalidMods;
		MSF_Base::GetInvalidMods(&invalidMods, moddata, weapon, attachedMod);
		for (auto itMods = invalidMods.begin(); itMods != invalidMods.end(); itMods++)
		{
			BGSMod::Attachment::Mod* invalidMod = *itMods;
			_DEBUG("invalid mod UI: %08X", invalidMod->formID);
			UInt32 newunk = unk;
			ApplyChangesFunctor removeInvalidModFunctor = ApplyChangesFunctor(applyChangesFunctor->foundObject, applyChangesFunctor->moddata, invalidMod, applyChangesFunctor->ignoreWeapon, true, applyChangesFunctor->equipLocked, applyChangesFunctor->setExtraData);
			MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachModToStack_CallFromWorkbenchUI_Copied(invItem, &CheckStackIDFunctor(stackID), &removeInvalidModFunctor, unk_r9d, &newunk);
		}

		BGSInventoryItem::Stack* stack = Utilities::GetStack(invItem, stackID);
		if (!stack)
			return result;
		ExtraDataList* extraList = stack->extraData;
		if (!extraList)
			return result;
		//TESObjectWEAP::InstanceData* instanceData = nullptr;
		//TESObjectWEAP* weapBase = nullptr;
		//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		//if (extraInstanceData)
		//{
		//	instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		//	weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;
		//}

		//BGSMod::Attachment::Mod* targetAmmoMod = nullptr;
		//bool toAttach = false;
		//TESAmmo* targetAmmo = ExtraWeaponState::GetAmmoForWorkbenchUI(extraList);
		//if (instanceData && !targetAmmo)
		//	targetAmmo = instanceData->ammo;

		//_DEBUG("targetAmmo %p, ammo: %p", targetAmmo, instanceData->ammo);

		////BGSMod::Attachment::Mod* invalidAmmoMod = MSF_Base::GetAmmoModIfInvalid(moddata, weapon);
		//MSF_Base::GetAmmoModToModify(moddata, targetAmmo, weapBase, &targetAmmoMod, &toAttach);
		//if (targetAmmoMod)
		//{
		//	_DEBUG("target ammomod UI: %08X, rem: %02X", targetAmmoMod->formID, toAttach);
		//	UInt32 newunk = unk;
		//	ApplyChangesFunctor removeInvalidModFunctor = ApplyChangesFunctor(applyChangesFunctor->foundObject, applyChangesFunctor->moddata, targetAmmoMod, applyChangesFunctor->ignoreWeapon, !toAttach, applyChangesFunctor->equipLocked, applyChangesFunctor->setExtraData);
		//	MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
		//	AttachModToStack_CallFromWorkbenchUI_Copied(invItem, &CheckStackIDFunctor(stackID), &removeInvalidModFunctor, unk_r9d, &newunk);
		//}

		std::vector<std::pair<BGSMod::Attachment::Mod*, bool>> stateModsToModify;
		ExtraWeaponState::HandleModChangeEvent(extraList, &stateModsToModify, ExtraWeaponState::kEventTypeModdedWorkbench);
		for (auto nextPair : stateModsToModify)
		{
			_DEBUG("target state/ammomod WB_UI: %08X, rem: %02X", nextPair.first->formID, nextPair.second);
			UInt32 newunk = unk;
			ApplyChangesFunctor changeStateModFunctor = ApplyChangesFunctor(applyChangesFunctor->foundObject, applyChangesFunctor->moddata, nextPair.first, applyChangesFunctor->ignoreWeapon, !nextPair.second, applyChangesFunctor->equipLocked, applyChangesFunctor->setExtraData);
			MSF_MainData::modSwitchManager.SetIgnoreDeleteExtraData(true);
			AttachModToStack_CallFromWorkbenchUI_Copied(invItem, &CheckStackIDFunctor(stackID), &changeStateModFunctor, unk_r9d, &newunk);
		}
	}
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

ExtraRank* LoadBuffer_ExtraDataList_ExtraRank_Hook(ExtraRank* newExtraRank, UInt32 rank, ExtraDataList* futureParentList, BGSInventoryItem::Stack* futureParentStack) //futureParentStack only valid if owner is inventory item (check vtbl!); must return newExtraRank
{
	_DEBUG("loaded ExtraRank: %08X, %p", rank, newExtraRank);
	MSF_MainData::weaponStateStore.StoreForLoad(rank, newExtraRank);
	return newExtraRank;
}

bool ExtraRankCompare_Hook(ExtraRank* extra1, ExtraRank* extra2) //ctor: B8670 v. A9F60 v. 9DC03
{
	//_DEBUG("COMPARE");
	if (!extra1 || !extra2)
		return false;
	//_DEBUG("%08X vs %08X", extra1->rank, extra2->rank);
	//ExtraWeaponState* state1 = MSF_MainData::weaponStateStore.Get(extra1->rank);
	//ExtraWeaponState* state2 = MSF_MainData::weaponStateStore.Get(extra2->rank);
	//if (state1 || state2)
	//	return false;
	//else
		return extra1->rank != extra2->rank;
}

UInt8 PlayerAnimationEvent_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3)
{
	const char* name = arg2->eventName.c_str();
	bool reload = false;
	UInt8 didSwitch = 0;
	UInt32 oldLoadedAmmoCount = 0;
	if (!_strcmpi("reloadComplete", name))
	{
		EquipWeaponData* eqData = Utilities::GetEquippedWeaponData(*g_player);
		if (eqData)
			oldLoadedAmmoCount = eqData->loadedAmmoCount;
		_DEBUG("reloadCOMP: %i", oldLoadedAmmoCount);
		reload = true;
		//if (MSF_MainData::activeBurstManager && (MSF_MainData::activeBurstManager->flags & BurstModeData::bActive))
		//	MSF_MainData::activeBurstManager->ResetShotsOnReload();
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			_DEBUG("swDatOK");
			if (switchData->SwitchFlags & SwitchData::bReloadInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bReloadInProgress;
				if ((*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState) == 0)
					MSF_MainData::modSwitchManager.SetState(ModSwitchManager::bState_ReloadNotFinished);
				if (MSF_MainData::modSwitchManager.GetIsBCRreload())
				{
					MSF_MainData::BCRinterfaceHolder.StoreBCRvariables();
					MSF_MainData::modSwitchManager.SetIsBCRreload(ExtraWeaponState::bEventTypeReloadSwitchBCR);
				}
				MSF_Base::SwitchMod(switchData, true);
				didSwitch = ExtraWeaponState::bEventTypeReloadAfterSwitch;
				//_DEBUG("switchOK");
			}
		}
		//ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeReload); //BCR!!

	}
	else if (!_strcmpi("reloadEnd", name))
	{
		_DEBUG("reloadEnd");
		MSF_MainData::modSwitchManager.SetIsBCRreload(0);
		if (MSF_MainData::modSwitchManager.GetState() & ModSwitchManager::bState_ReloadNotFinished)
		{
			UInt16 endFlag = ~ModSwitchManager::bState_ReloadNotFinished;
			EndSwitchTask* endTask = new EndSwitchTask(endFlag);
			delayTask delayEnd(MSF_MainData::iReloadAnimEndEventDelayMS, true, g_threading->AddTask, endTask);
			//delayTask delayEnd(MSF_MainData::iReloadAnimEndEventDelayMS, true, &MSF_Base::EndSwitch, endFlag);
		}
	}
	else if (!_strcmpi("weaponDraw", name))
	{
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData)
		{
			//_DEBUG("drawFlags: %08X", switchData->SwitchFlags);
			if (switchData->SwitchFlags & SwitchData::bDrawInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bDrawInProgress;
				if ((switchData->SwitchFlags & SwitchData::bReloadNeeded))
				{
					//_DEBUG("reloading");
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadCompatibilityMode)
					{
						switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
						MSF_Base::SwitchMod(switchData, true);
					}
					else
						switchData->SwitchFlags = (switchData->SwitchFlags & ~SwitchData::bReloadNeeded) | SwitchData::bReloadInProgress; // | SwitchData::bReloadNotFinished
					ReloadTask* reloadTask = new ReloadTask(switchData->SwitchFlags & SwitchData::bReloadFull, switchData->SwitchFlags & SwitchData::bReloadZeroCount, true, true);
					delayTask delayReload(MSF_MainData::iDrawAnimEndEventDelayMS, true, g_threading->AddTask, reloadTask);
					//delayTask delayReload(MSF_MainData::iDrawAnimEndEventDelayMS, true, &MSF_Base::ReloadWeapon, switchData->SwitchFlags & SwitchData::bReloadFull, switchData->SwitchFlags & SwitchData::bReloadZeroCount, true, true);
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
		//if (MSF_MainData::activeBurstManager && (MSF_MainData::activeBurstManager->flags & BurstModeData::bActive))
		//	MSF_MainData::activeBurstManager->HandleFireEvent();
		//if (MSF_MainData::tmr.IsRunning())
		//{
		//	if (MSF_MainData::tmr.stop() < 1000)
		//		MSF_Base::FireBurst(*g_player);
		//}
		//_DEBUG("Anim: fire %i", MSF_MainData::tmr.stop());
		ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeFireWeapon);
		//_DEBUG("Anim: fire");
	}
	else if (!_strcmpi("IdleStop", name))
	{
		if (MSF_MainData::modSwitchManager.GetShouldBlendAnimation())
		{
			MSF_MainData::modSwitchManager.SetShouldBlendAnimation(false);
			PlayerCharacter* player = *g_player;
			InitializeActorInstant(player, 0);
			UpdateAnimation(player, 0.2f);
		}
		if (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimCompatibilityMode)
		{
			SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
			if (switchData) //!(switchData->SwitchFlags & SwitchData::bSwitchingInProgress))
			{
				switchData->SwitchFlags |= SwitchData::bSwitchingInProgress;
				MSF_Base::SwitchMod(switchData, true);
			}
		}
	}
	else if (!_strcmpi("switchMod", name))
	{
		SwitchData* switchData = MSF_MainData::modSwitchManager.GetNextSwitch();
		if (switchData && !(MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimCompatibilityMode))
		{
			if (switchData->SwitchFlags & SwitchData::bAnimInProgress)
			{
				switchData->SwitchFlags &= ~SwitchData::bAnimInProgress;
				if ((*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState) == 0)
					MSF_MainData::modSwitchManager.SetState(ModSwitchManager::bState_AnimNotFinished);
				MSF_Base::SwitchMod(switchData, true);
			}
		}
	}
	else if (!_strcmpi("customAnimEnd", name))
	{
		if (MSF_MainData::modSwitchManager.GetState() & ModSwitchManager::bState_AnimNotFinished && !(MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimCompatibilityMode))
		{
			UInt16 endFlag = ~ModSwitchManager::bState_AnimNotFinished;
			EndSwitchTask* endTask = new EndSwitchTask(endFlag);
			delayTask delayEnd(MSF_MainData::iCustomAnimEndEventDelayMS, true, g_threading->AddTask, endTask);
			//delayTask delayEnd(MSF_MainData::iCustomAnimEndEventDelayMS, true, &MSF_Base::EndSwitch, endFlag);
		}
	}
	else if (!_strcmpi("Event00", name))
	{
		_DEBUG("BCR Event00");
		EquipWeaponData* eqData = Utilities::GetEquippedWeaponData(*g_player);
		if (eqData)
			oldLoadedAmmoCount = eqData->loadedAmmoCount;
		UInt16 BCRtype = oldLoadedAmmoCount ? ExtraWeaponState::bEventTypeReloadBCR : ExtraWeaponState::bEventTypeReloadFullBCR;
		MSF_MainData::modSwitchManager.SetIsBCRreload(BCRtype);
	}
	else if (!_strcmpi("emptyMag", name))
	{
		//ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeEmptyMag);
		EquipWeaponData* eqData = Utilities::GetEquippedWeaponData(*g_player);
		eqData->loadedAmmoCount = 0;
	}
	else if (!_strcmpi("switchMag", name))
	{
		//ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeSwitchMag);
	}
	else if (!_strcmpi("toggleMenu", name))
	{
		//get menu to open
		//open menu
	}
	//else if (!_strcmpi("uncullBone", name))
	//{
	//	_DEBUG("uncullBone");
	//	MSF_MainData::BCRinterfaceHolder.StoreBCRvariables();
	//	MSF_MainData::BCRinterfaceHolder.LogStored();
	//}
	//else if (!_strcmpi("cullBone", name))
	//{
	//	_DEBUG("cullBone");
	//	MSF_MainData::BCRinterfaceHolder.StoreBCRvariables();
	//	MSF_MainData::BCRinterfaceHolder.LogStored();
	//}

	UInt8 ret = PlayerAnimationEvent_Original(arg1, arg2, arg3);

	if (reload == true)
	{
		if (Utilities::GetAnimationVariableInt(*g_player, "SwitchAmmoTypeReload"))
		{
			_DEBUG("animVar: 1");
			Utilities::SetAnimationVariableInt(*g_player, "SwitchAmmoTypeReload", 0);
		}
		_DEBUG("reloadCOMP2: %i", Utilities::GetEquippedWeaponData(*g_player)->loadedAmmoCount);
		ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeReload, *g_player, didSwitch, oldLoadedAmmoCount); //BCR!!
	}

	return ret;
}