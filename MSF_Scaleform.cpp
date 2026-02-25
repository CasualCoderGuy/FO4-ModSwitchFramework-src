#include "MSF_Scaleform.h"
#include "MSF_Test.h"
#include "MSF_WeaponState.h"
#include "MSF_BurstMode.h"
#include "MSF_Localization.h"
#include "f4se\GameSettings.h"

_LoadCustomMenu LoadCustomMenu_F4SEHook;
SimpleLock MSFCustomMenu::menuLock;
std::unordered_map<std::string, MSFCustomMenuData*> MSFCustomMenu::customMenuData;
std::vector<MSFCustomMenu*> MSFCustomMenu::menuHandles;

void HandleInputEvent(ButtonEvent * inputEvent)
{
	if (!MSF_MainData::IsInitialized)
		return;
	UInt32	keyCode;
	UInt32	deviceType = inputEvent->deviceType;
	UInt32	keyMask = inputEvent->keyMask;

	//_DEBUG("mask: %08X", keyMask);
	// Mouse
	if (deviceType == InputEvent::kDeviceType_Mouse)
		keyCode = InputMap::kMacro_MouseButtonOffset + keyMask;
	// Gamepad
	else if (deviceType == InputEvent::kDeviceType_Gamepad)
	{
		keyCode = InputMap::GamepadMaskToKeycode(keyMask);
		//_DEBUG("gamepad: %08X", keyCode);
	}
	// Keyboard
	else
		keyCode = keyMask;

	// Valid scancode?
	if (keyCode >= InputMap::kMaxMacros)
		return;

	BSFixedString	control = *inputEvent->GetControlID();
	float			timer = inputEvent->timer;

	bool isDown = inputEvent->isDown == 1.0f && timer == 0.0f;
	bool isUp = inputEvent->isDown == 0.0f && timer != 0.0f;


	if (isDown)
	{
		switch (keyCode) 
		{
		case 160:
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
			break;	// Shift, Ctrl, Alt modifiers
		case 256:
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//MSF_Base::FireBurst(*g_player);
				//MSF_MainData::tmr.start();
				//_DEBUG("fire input");
			}
		}
			break;
#ifdef DEBUG
		case 0x21://PGUP
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//MSF_Test::ListModTable(*g_player);
				//MSF_Test::DumpAttachParent();
				//MSF_Test::GetWeaponState();
				//MSF_Test::DumpAttachParent();
				//MSF_Test::DumpForms();
				//MSF_Test::SetAmmoTest();
				//MSF_Scaleform::GetInterfaceVersion();
				//MSF_Scaleform::UpdateWidgetData(nullptr);
				//MSF_Scaleform::SetWidgetVisibility(true);
				//MSFMenu::CloseMenu();
				//MSFMenu::OpenMenu();
				//static BSFixedString menuName("MSFMenu");
				//CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
				//MSFAmmoMenu::OpenMenu();
				//MSFMenu::OpenMenu();
				//MSF_Test::ListEquippedItemTypes();
				//MSF_Test::ArmorAttachTest();
				//MSF_Test::RemapAnimTest(true, false); //sub_141387BE0
				//Utilities::PlayIdleAction(*g_player, (BGSAction*)LookupFormByID(0x3B248));
				//Utilities::PlayIdle(*g_player, MSF_MainData::fireIdle1stP);
				//MSF_Base::BurstTest(nullptr);
				//MSF_Test::DumpUnknownData((*g_player));
				//MSF_Test::DumpActorValue();
				//MSF_Test::DumpExtraDataInfo();
				//MSF_Test::CallAttachModToInvItem();
				//MSF_Test::NPCtest();
				
				//TESIdleForm* sw = reinterpret_cast<TESIdleForm*>(Utilities::GetFormFromIdentifier("SideAim.esp|1746"));//"VD_SIGMCXVirtus_MSFDemo.esp|2E03"
				//Utilities::PlayIdle(*g_player, sw);

				//MSF_Test::ExtraDataTest();
				//MSF_Test::TestIdle(true);
				//_DEBUG("pa: %02X", IsInPowerArmor(*g_player));

				//MSF_Test::ArmorAttachTest();
				//MSF_Test::PrintAmmoCount();
				//MSF_Test::AIM_ZM_MA_testDump();
				//MSF_Test::ProjectileTest();
				//Utilities::PlayIdleAction(*g_player, (BGSAction*)LookupFormByID(0x13454));
				//MSF_Test::ModTemplateTest();

				Utilities::PlaySoundInternal(MSF_MainData::failSound, *g_player);
				//MSF_Test::AttachStackTest(2, true);
				//MSF_Test::RaiderTest();
				//ShowNotification("φόσυϊιαϋν", nullptr, 0, 0);
				_DEBUG("str %s", "φόσυϊιαϋν");
				_DEBUG("test1");
			}
		}
		break;
		case 0x22://PGD
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//Utilities::AddRemActorValue((*g_player), MSF_MainData::BurstModeTime, true);
				//MSF_Test::AttachModInternalTest(true, 2);
				//MSF_Test::CreateWeaponState();
				//MSF_Test::CallAddItem();
				//MSF_Test::CallPlayAction();
				//MSF_Test::MenuFlagTest();
				//MSF_Test::CallAttachModToInvItem();
				//Utilities::ReloadWeapon(*g_player);
				//MSF_Test::TestIdle(false);
				//MSF_Test::DumpActorAnimData();
				//MSF_Test::TestEquipAmmo();
				//Utilities::PlayIdleAction(*g_player, (BGSAction*)LookupFormByID(0x13455));
				//MSF_Data::PatchBaseAmmoMods();
				//MSF_Test::RemapAnimTest(true, true);
				//MSF_Test::AttachStackTest(0, true);
				//ShowNotification("test", nullptr, 1, 0);
				PlaySoundVM((*g_gameVM)->m_virtualMachine, 0, MSF_MainData::failSound, *g_player);
				_DEBUG("test2");
			}
		}
		break;
		case 0x23://END
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//Utilities::AddRemActorValue((*g_player), MSF_MainData::BurstModeTime, false);
				//CALL_MEMBER_FN((*g_player), QueueUpdate)(true, 0, true, 0);
				//MSF_Test::AttachModInternalTest(true, 2);
				//MSF_Test::CallRemoveModFromInvItem();
				//MSF_Test::DamageEquippedWeapon(*g_player);
				//MSF_Test::ListExtraData();
				//MSF_Test::SplitStackTest(13, false);
				//Utilities::FireWeapon(*g_player, 1);
				//MSF_Test::DumpActorAnimData();
				//MSF_Test::DumpEquippedWeaponIdx();
				//Utilities::PlayIdleAction(*g_player, (BGSAction*)LookupFormByID(0x13456));
				//MSF_Test::ProjectileTest();
				//MSF_Test::RemapAnimTest(false, true);
				//MSF_Test::AttachStackTest(2, false);
				ShowNotification("test", nullptr, 1, 0);
				_DEBUG("test3");
			}
		}
		break;
#endif
		default: 
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				UInt8 modifiers = 0;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)	modifiers |= 0x1;
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000)	modifiers |= 0x2;
				if (GetAsyncKeyState(VK_MENU) & 0x8000)		modifiers |= 0x4;
				UInt64 key = ((UInt64)modifiers << 32) + keyCode;
				if (!key)
					break;
				if (key == MSF_MainData::lowerWeaponHotkey)
				{
					MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true, true); //sound:playFailIfRunning??
					Utilities::PlayIdleAction(*g_player, MSF_MainData::ActionGunDown);
					break;
				}
				//if (key == MSF_MainData::cancelQuickkey)
				//{
				//	MSF_MainData::modSwitchManager.ClearQuickSelection(true, true);
				//	break;
				//}
				if (key == MSF_MainData::cancelSwitchHotkey)
				{
					MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true);
					MSF_MainData::modSwitchManager.ClearQueue();
					break;
				}
				if (key == MSF_MainData::patchBaseAmmoHotkey)
				{
					MSF_Data::PatchBaseAmmoMods();
					break;
				}
				if (key == MSF_MainData::DEBUGprintStoredDataHotkey)
				{
					MSF_Data::PrintStoredData();
					break;
				}
				KeybindData* keyFn = MSF_Data::GetKeybindDataForKey(keyCode, modifiers);
				if (!keyFn)
					break;
				_DEBUG("key: %i, type: %02X", keyFn->keyCode, keyFn->type);
				if (keyFn->type & KeybindData::bGlobalMenu)
				{
					MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true, true);
					MSF_Scaleform::ToggleGlobalMenu(keyFn->selectMenu, &keyFn->menuAttachPoints);
				}
				else if (keyFn->type & KeybindData::bHUDselection)
				{
					MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true, true);
					MSF_Scaleform::HandleToggleMenu(keyFn->selectMenu, keyFn->modData);
				}
				else if (keyFn->type & KeybindData::bIsAmmo)
				{
					if (keyFn->type & KeybindData::bQuickKey)
						MSF_MainData::modSwitchManager.HandleQuickkey(keyFn);
					else
					{
						MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true, true);
						MSF_Base::SwitchAmmoHotkey(keyFn->type & KeybindData::mAmmoNumMask, keyFn->successSound);
					}
				}
				//if (keyFn->type & KeybindData::bGlobalMenu)
				//	MSF_Scaleform::ToggleGlobalMenu(keyFn->selectMenu);
				//else if(keyFn->type & KeybindData::bIsAmmo)
				//{
				//	if (keyFn->type & KeybindData::bHUDselection)
				//		MSF_Scaleform::ToggleAmmoMenu(keyFn->selectMenu);
				//	else
				//		MSF_Base::SwitchAmmoHotkey(keyFn->type & KeybindData::mNumMask);
				//}
				//else if (keyFn->type & KeybindData::bHUDselection)
				//	MSF_Scaleform::ToggleModMenu(keyFn->selectMenu, keyFn->modData);
				else if (keyFn->type & KeybindData::bToggle)
				{
					if (keyFn->type & KeybindData::bQuickKey)
						MSF_MainData::modSwitchManager.HandleQuickkey(keyFn);
					else
					{
						MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true, true);
						MSF_Base::ToggleModHotkey(keyFn->modData);
					}
				}
				else
				{
					MSF_MainData::modSwitchManager.ClearQuickSelection(true, true, true, true);
					MSF_Base::SwitchModHotkey(keyFn->type & KeybindData::mModNumMask, keyFn->modData);
				}
			}
		}
			break;
		}
	}
	else if (isUp)
	{
		if (keyCode == 256) 
		{
			if (MSF_MainData::activeBurstManager && (MSF_MainData::activeBurstManager->flags & BurstModeData::bActive))
				MSF_MainData::activeBurstManager->HandleReleaseEvent();
		}
		else
		{
			UInt8 modifiers = 0;
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)	modifiers |= 0x1;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)	modifiers |= 0x2;
			if (GetAsyncKeyState(VK_MENU) & 0x8000)		modifiers |= 0x4;
			KeybindData* keyFn = MSF_Data::GetKeybindDataForKey(keyCode, modifiers);
			MSF_MainData::modSwitchManager.SetQuickkeyUp(keyFn);
		}
	}
}

class F4SEInputHandlerSink : public PlayerInputHandler
{
public:
	F4SEInputHandlerSink() : PlayerInputHandler() { }

	virtual void OnButtonEvent(ButtonEvent * inputEvent) override
	{
		if ((*g_ui)->numPauseGame)
		{
			return;
		}

		HandleInputEvent(inputEvent);
	}


};

F4SEInputHandlerSink inputHandler;

void LoadMSFCustomMenu_Hook(IMenu* menu)
{
	MSFCustomMenu::menuLock.Lock();
	_DEBUG("looking for: %s", menu->menuName.c_str());
	auto menuData = MSFCustomMenu::customMenuData.find(menu->menuName.c_str());
	if (menuData != MSFCustomMenu::customMenuData.end())
	{
		BSFixedString menuPath = menuData->second->menuPath.c_str();
		BSFixedString rootPath = menuData->second->rootPath.c_str();
		UInt32 movieFlags = menuData->second->movieFlags;
		UInt32 extFlags = menuData->second->extFlags;
		menu->flags = menuData->second->menuFlags;
		menu->depth = menuData->second->depth;

		if ((menu->flags & IMenu::kFlag_UsesCursor) && (extFlags & MSFCustomMenuData::kExtFlag_CheckForGamepad))
		{
			if ((*g_inputDeviceMgr)->IsGamepadEnabled())
				menu->flags &= ~IMenu::kFlag_UsesCursor;
		}

		if (LoadMovieEx(*g_scaleformManager, menu, menu->movie, menuPath.c_str(), rootPath.c_str(), movieFlags, 0.0F))//(CALL_MEMBER_FN((*g_scaleformManager), LoadMovie)(menu, menu->movie, menuPath.c_str(), rootPath.c_str(), movieFlags))
		{
			menuData->second->isValid = true;
			menu->stage.SetMember("menuFlags", &GFxValue(menu->flags));
			menu->stage.SetMember("movieType", &GFxValue(movieFlags));
			menu->stage.SetMember("extendedFlags", &GFxValue(extFlags));

			GameMenuBase* gameMenu = static_cast<GameMenuBase*>(menu);

			CreateBaseShaderTarget(gameMenu->filterHolder, menu->stage);

			if (extFlags & MSFCustomMenuData::kExtFlag_InheritColors)
			{
				gameMenu->filterHolder->SetFilterColor(false);
				(*g_colorUpdateDispatcher)->eventDispatcher.AddEventSink(gameMenu->filterHolder);
			}

			if (menu->flags & IMenu::kFlag_CustomRendering)
			{
				gameMenu->shaderFXObjects.Push(gameMenu->filterHolder);
			}
		}
	}
	else
		LoadCustomMenu_F4SEHook(menu);
	MSFCustomMenu::menuLock.Release();
}

bool PipboyMenu::CreateItemDataW(PipboyMenu::ScaleformArgs* args, std::wstring text, std::string value)
{
	if (!args)
		return false;
	auto* movieRoot = args->movie->movieRoot;
	auto& pInfoObj = args->args[1];
	GFxValue extraData;
	movieRoot->CreateObject(&extraData);
	MSF_Scaleform::RegisterStringW(&extraData, movieRoot, "text", text.c_str());//
	MSF_Scaleform::RegisterString(&extraData, movieRoot, "value", value.c_str());
	MSF_Scaleform::RegisterInt(&extraData, movieRoot, "difference", 0);
	args->args[1].PushBack(&extraData);
	return true;
}

bool PipboyMenu::CreateItemData(PipboyMenu::ScaleformArgs* args, std::string text, std::string value)
{
	if (!args)
		return false;
	auto* movieRoot = args->movie->movieRoot;
	auto& pInfoObj = args->args[1];
	GFxValue extraData;
	movieRoot->CreateObject(&extraData);
	MSF_Scaleform::RegisterString(&extraData, movieRoot, "text", text.c_str());//
	MSF_Scaleform::RegisterString(&extraData, movieRoot, "value", value.c_str());
	MSF_Scaleform::RegisterInt(&extraData, movieRoot, "difference", 0);
	args->args[1].PushBack(&extraData);
	return true;
}

//void BarterInvoke_Hook(BarterMenu* menu, BarterMenu::ScaleformArgs* args) // B0A3A0 @ 2D1A7A0 barter, 2D1B140 contmenubase, 2D86420 contmenu
//{
//	BarterMenuInvoke_Copied(menu, args);
//	if (args->optionID == 0x3 && &&args->numArgs == 4 && args->args[0].GetType() == GFxValue::kType_Int && args->args[1].GetType() == GFxValue::kType_Array && args->args[2].GetType() == GFxValue::kType_Array)//B0C3C0 @ B0A54B
//}

void PipboyMenuInvoke_Hook(PipboyMenu* menu, PipboyMenu::ScaleformArgs* args)
{
	//if (args->optionID == 0xE) // view
	//	ignore
	//if (args->optionID == 0x10) // drop
	//	ignore
	//+workbechview +modsplit
	// changeammo
	PipboyMenuInvoke_Copied(menu, args);
	//(this->*Invoke_Original)(args);
	_DEBUG("pipboyei");
	if (args->optionID == 0xD && args->numArgs == 4 && args->args[0].GetType() == GFxValue::kType_Int && args->args[1].GetType() == GFxValue::kType_Array && args->args[2].GetType() == GFxValue::kType_Array)
	{
		_DEBUG("args ok");
		SInt32 selectedIndex = args->args[0].GetInt();
		PipboyObject* pHandlerData = nullptr;
		if (selectedIndex >= 0 && selectedIndex < (*g_pipboyDataMgr)->itemData.count)
			pHandlerData = (*g_pipboyDataMgr)->itemData[selectedIndex];
		_DEBUG("handler ok");
		if (pHandlerData != nullptr)
		{
			static BSFixedString handleName("handleID");
			static BSFixedString stackName("StackID");

			auto* pipboyValueHandle = static_cast<PipboyPrimitiveValue<UInt32>*>(pHandlerData->table.Find(&handleName)->value);
			auto* pipboyValueStack = static_cast<PipboyArray*>(pHandlerData->table.Find(&stackName)->value);
			_DEBUG("values ok");
			//auto* pipboyValue = static_cast<PipboyPrimitiveValue<UInt32>*>(pHandlerData->GetMemberValue(memberName));
			if (pipboyValueHandle != nullptr)
			{
				UInt32 handleID = pipboyValueHandle->value;
				_DEBUG("id: %08X", handleID);
				auto* pSelectedData = (*g_itemMenuDataMgr)->GetSelectedItem(handleID);
				auto* pSelectedForm = (*g_itemMenuDataMgr)->GetSelectedForm(handleID);
				UInt32 stackID = 0;
				_DEBUG("selected: %p, %p", pSelectedData, pSelectedForm);
				if (pipboyValueStack != nullptr && pipboyValueStack->values.entries)
				{
					auto* pipboyStackIDholder = static_cast<PipboyPrimitiveValue<UInt32>*>(*pipboyValueStack->values.entries);
					stackID = pipboyStackIDholder->value;
					_DEBUG("stack ok");
				}

				if (pSelectedData != nullptr && pSelectedData->form != nullptr)
				{
					_DEBUG("adding items");
					TESForm* pSelectedForm = pSelectedData->form;
					BGSInventoryItem::Stack* selectedStack = Utilities::GetStack(pSelectedData, stackID);
					if (!selectedStack)
						return;
					ExtraDataList* extraDataList = selectedStack->extraData;
					ExtraWeaponState::AmmoStateData ammoState;
					bool hasAmmoState = false;
					UInt8 notSupportedAmmo = 0;
					if (extraDataList)
					{
						ExtraRank* holder = (ExtraRank*)extraDataList->GetByType(kExtraData_Rank);
						if (holder)
						{
							ExtraWeaponState* extraState = MSF_MainData::weaponStateStore.Get(holder->rank);
							if (extraState)
							{
								hasAmmoState = extraState->GetAmmoStateData(&ammoState);
								notSupportedAmmo = extraState->HasNotSupportedAmmo();
							}
						}
						//switch (pSelectedForm->formType)
						//{
						//case kFormType_WEAP:
						if (pSelectedForm->formType == kFormType_WEAP)
						{
							//auto* pWeapon = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(neededInst, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
							//if (pWeapon)
							//{
							if (hasAmmoState && ammoState.ammoCapacity && (MSF_MainData::MCMSettingFlags & MSF_MainData::bDisplayMagInPipboy) && !notSupportedAmmo)
							{
								_DEBUG("creating items");
								std::string displayString = std::to_string(ammoState.ammoCapacity) + "/" + std::to_string(ammoState.loadedAmmo);
								menu->CreateItemData(args, MSF_Localization::GetTranslation(MSF_Localization::Keys::pipboyMagsizeLoadedText), displayString); //L"Mag size/Loaded"
							}
							//if (ammoState && ammoState->ammoCapacity && (MSF_MainData::MCMSettingFlags & MSF_MainData::bDisplayChamberInPipboy))
							//{
							//	std::string displayString = std::to_string(ammoState->chamberSize) + "/" + std::to_string(ammoState->chamberedCount);
							//	menu->CreateItemData(args, "Chamber Size/Loaded", displayString);
							//}
							//}
							//	break;
							//}
						}
					}
				}
			}
		}
	}
	//_DEBUG("return");
}

namespace MSF_Scaleform
{
	class OnMCMClosed : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			// OnMCMClosed
			_DEBUG("mcm closed");
		}
	};

	class SendF4SEVersion : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetInt(0);
			if (args->numArgs != 0) return;
			args->result->SetUInt(PACKED_F4SE_VERSION);
		}
	};

	class SendGameVersion : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetInt(0);
			if (args->numArgs != 0) return;
			args->result->SetUInt(CURRENT_RELEASE_RUNTIME);
		}
	};

	class SendMSFVersion : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetUInt(0);
			if (args->numArgs != 0) return;
			args->result->SetUInt(MSF_VERSION);
		}
	};

	class SendWidgetUpdate : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(1);
			UpdateWidgetData();
		}
	};

	class SendWidgetSettings : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(1);
			UpdateWidgetSettings();
		}
	};

	class SendLocalization : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetString("");
			if (args->numArgs != 0) return;
			Setting* setting = GetINISetting("sLanguage:General");
			std::string loc = (setting && setting->GetType() == Setting::kType_String) ? setting->data.s : "en";
			args->result->SetString(loc.c_str());
		}
	};

	class MenuClosedEventSink : public GFxFunctionHandler { //also:onmenuopencloseevent:hudmenu
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(1);
			//std::string menu = args->args[0].data.string;
			MSF_MainData::modSwitchManager.CloseOpenedMenu();
		}
	};

	class MenuOpenedEventSink : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(1);
			//std::string menu = args->args[0].data.string;
			//findmenu
			//MSF_MainData::switchData.openedMenu = nullptr;
		}
	};

	// SendBackSelectedAmmo(ammo:Pointer):Boolean;
	//class ReceiveSelectedAmmo : public GFxFunctionHandler {
	//public:
	//	virtual void Invoke(Args* args) {
	//		args->result->SetBool(false);
	//		_DEBUG("ammo scaleform callback");
	//		if (args->numArgs != 1) return;
	//		MSF_MainData::modSwitchManager.SetOpenedMenu(nullptr);
	//		_DEBUG("val: %i, %p", args->args[0].type, args->args[0].data.obj);
	//		//if (args->args[0].data.obj) //args->args[0].type == GFxValue::kType_Object && 
	//		//	MSF_Base::SwitchToSelectedAmmo(args->args[0].data.obj);

	//		args->result->SetBool(true);
	//	}
	//};

	class ReceiveSelectedAmmoIdx : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);
			_DEBUG("ammo scaleform callback idx");
			if (args->numArgs != 1) return;
			ModSwitchManager::OpenedMenuData menuData = MSF_MainData::modSwitchManager.GetOpenedMenuData();
			BGSSoundDescriptorForm* sound = nullptr;
			if (menuData.selectMenu)
				sound = menuData.selectMenu->successSound;
			MSF_MainData::modSwitchManager.CloseOpenedMenu();
			_DEBUG("val: %i, %i", args->args[0].type, args->args[0].data.u32);
			if (args->args[0].type == GFxValue::kType_UInt)
			{
				AmmoData::AmmoMod* ammo = MSF_MainData::modSwitchManager.GetDisplayedAmmoByIndex(args->args[0].data.u32);
				_DEBUG("ptr: %p", ammo);
				MSF_Base::SwitchToSelectedAmmo(ammo, sound);
			}
			MSF_MainData::modSwitchManager.ClearDisplayChioces();
			args->result->SetBool(true);
		}
	};

	//class ReceiveSelectedMod : public GFxFunctionHandler {
	//public:
	//	virtual void Invoke(Args* args) {
	//		args->result->SetBool(false);

	//		if (args->numArgs != 3) return;
	//		MSF_MainData::modSwitchManager.SetOpenedMenu(nullptr);
	//		//MSF_Base::SwitchToSelectedMod(args->args[0].data.obj, args->args[1].data.obj, args->args[2].data.boolean);

	//		args->result->SetBool(true);
	//	}
	//};

	class ReceiveSelectedModIdx : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);

			if (args->numArgs != 2) return;
			ModSwitchManager::OpenedMenuData menuData = MSF_MainData::modSwitchManager.GetOpenedMenuData();
			BGSSoundDescriptorForm* sound = nullptr;
			if (menuData.selectMenu)
				sound = menuData.selectMenu->successSound;
			MSF_MainData::modSwitchManager.CloseOpenedMenu();
			if (args->args[0].type == GFxValue::kType_UInt && args->args[1].type == GFxValue::kType_UInt) //, args->args[2].type == GFxValue::kType_Bool)
			{
				ModData::Mod* modAdd = MSF_MainData::modSwitchManager.GetDisplayedModByIndex(args->args[0].data.u32);
				ModData::Mod* modRem = MSF_MainData::modSwitchManager.GetDisplayedModByIndex(args->args[1].data.u32);
				MSF_Base::SwitchToSelectedMod(modAdd, modRem, sound);// args->args[2].data.boolean);
			}
			MSF_MainData::modSwitchManager.ClearDisplayChioces();
			args->result->SetBool(true);
		}
	};

	class ReceiveScaleformProperties : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);

			if (args->numArgs != 3) return;
			const char* cmname = *args->args[0].data.managedString;
			UInt8 type = args->args[1].data.u32 & 0x3;
			UInt32 version = args->args[2].data.u32;
			if (!cmname)
				cmname = "";
			_DEBUG("scaleform prop: %s, %02X, %08X", cmname, type, version);
			if (type == ModSelectionMenu::kType_Widget && version >= MIN_SUPPORTED_SWF_WIDGET_VERSION)
				MSF_MainData::widgetMenu = nullptr; // = find args->args[0].data.string in array of Keybind; MSF_MainData::widgetMenu->type = type; MSF_MainData::widgetMenu->version = version;
			else if ((type == ModSelectionMenu::kType_AmmoMenu && version >= MIN_SUPPORTED_SWF_AMMO_VERSION) || ((type & ModSelectionMenu::kType_ModMenu) && version >= MIN_SUPPORTED_SWF_MOD_VERSION))
				version = version; // = find args->args[0].data.string in array of Keybind; MSF_MainData::widgetMenu->type = type; MSF_MainData::widgetMenu->version = version;
			else
			{
				args->result->SetBool(false);
				return;
			}
			args->result->SetBool(true);
		}
	};

	//// GetModSettingString(modName:String, settingName:String):String;
	//class GetWidgetString : public GFxFunctionHandler {
	//public:
	//	virtual void Invoke(Args* args) {
	//		args->result->SetString("");

	//		if (args->numArgs != 2) return;
	//		if (args->args[0].GetType() != GFxValue::kType_String) return;
	//		if (args->args[1].GetType() != GFxValue::kType_String) return;

	//		args->result->SetString(SettingStore::GetInstance().GetModSettingString(args->args[0].GetString(), args->args[1].GetString()));
	//	}
	//};

	void ReceiveKeyEvents()
	{
		inputHandler.enabled = true;
		tArray<PlayerInputHandler*>* inputEvents = &((*g_playerControls)->inputEvents1);
		PlayerInputHandler* pInputHandler = &inputHandler;
		int idx = inputEvents->GetItemIndex(pInputHandler);
		if (idx == -1)
		{
			inputEvents->Insert(0, pInputHandler);
			_MESSAGE("Registered for input events.");
		}
	}

	void SetStringW(GFxValue* dst, const wchar_t* str)
	{
		dst->SetString(reinterpret_cast<const char*>(str));
		dst->type = GFxValue::kType_Unknown7;
	}

	void RegisterStringW(GFxValue* dst, GFxMovieRoot* root, const char* name, const wchar_t* str)
	{
		GFxValue	fxValue;
		root->CreateStringW(&fxValue, str);
		dst->SetMember(name, &fxValue);
	}

	void RegisterString(GFxValue * dst, GFxMovieRoot * root, const char * name, const char * str)
	{
		GFxValue	fxValue;
		root->CreateString(&fxValue, str);
		dst->SetMember(name, &fxValue);
	}

	void RegisterObject(GFxValue * dst, GFxMovieRoot * root, const char * name, void * obj)
	{
		GFxValue	fxValue;
		fxValue.data.obj = obj;
		dst->SetMember(name, &fxValue);
	}

	void RegisterBool(GFxValue * dst, GFxMovieRoot * root, const char * name, bool value)
	{
		GFxValue	fxValue;
		fxValue.SetBool(value);
		dst->SetMember(name, &fxValue);
	}

	void RegisterInt(GFxValue * dst, GFxMovieRoot * root, const char * name, SInt32 value)
	{
		GFxValue	fxValue;
		fxValue.SetInt(value);
		dst->SetMember(name, &fxValue);
	}

	bool StartWidgetHideCountdown(UInt32 delayTime)
	{
		static BSFixedString menuName("MSFwidget");
		IMenu* widgetMenu = (*g_ui)->GetMenu(menuName);
		if (!widgetMenu)
			return false;
		GFxMovieRoot* movieRoot = widgetMenu->movie->movieRoot;
		if (!movieRoot)
			return false;
		GFxValue arrArgs[4]; //obj?
		arrArgs[0].SetUInt(delayTime);
		movieRoot->Invoke("root.StartHideCountdown", nullptr, arrArgs, 1);
		return true;
	}

	bool UpdateWidgetSettings()
	{
		static BSFixedString menuName("MSFwidget");
		IMenu* widgetMenu = (*g_ui)->GetMenu(menuName);
		if (!widgetMenu)
			return false;
		GFxMovieRoot* movieRoot = widgetMenu->movie->movieRoot;
		if (!movieRoot)
			return false;

		Setting* setting = GetINISetting("sLanguage:General");
		std::string loc = (setting && setting->GetType() == Setting::kType_String) ? setting->data.s : "en";

		GFxValue arrArgs[10];
		arrArgs[0].SetUInt(MSF_MainData::MCMSettingFlags);
		arrArgs[1].SetUInt(MSF_MainData::widgetSettings.iFont);
		arrArgs[2].SetUInt(MSF_MainData::widgetSettings.GetRGBcolor());
		arrArgs[3].SetNumber(MSF_MainData::widgetSettings.fSliderMainX);
		arrArgs[4].SetNumber(MSF_MainData::widgetSettings.fSliderMainY);
		arrArgs[5].SetNumber(MSF_MainData::widgetSettings.fPowerArmorOffsetX);
		arrArgs[6].SetNumber(MSF_MainData::widgetSettings.fPowerArmorOffsetY);
		arrArgs[7].SetNumber(MSF_MainData::widgetSettings.fSliderAlpha);
		arrArgs[8].SetNumber(MSF_MainData::widgetSettings.fSliderScale);
		arrArgs[9].SetString(loc.c_str());
		movieRoot->Invoke("root.UpdateWidgetSettings", nullptr, arrArgs, 10);
		return true;
	}

	bool UpdateWidgetQuickkeyMod(KeywordValue ap, TESForm* nameForm, std::string nullModName, bool isAmmo)
	{
		static BSFixedString menuName("MSFwidget");
		IMenu* widgetMenu = (*g_ui)->GetMenu(menuName);
		if (!widgetMenu)
			return false;
		GFxMovieRoot* movieRoot = widgetMenu->movie->movieRoot;
		if (!movieRoot)
			return false;
		const char* apName = "";
		const char* modName = "";
		if (isAmmo)
			apName = MSF_Localization::GetTranslation(MSF_Localization::Keys::menuAmmoTypeText); //"Ammo Type"
		else if (ap == MSF_MainData::muzzleAP)
			apName = MSF_Localization::GetTranslation(MSF_Localization::Keys::menuMuzzleText); //"Muzzle"
		else
		{
			BGSKeyword* kw = GetKeywordFromValueArray(AttachParentArray::iDataType, ap);
			if (kw)
				apName = kw->GetFullName();
		}
		if (nameForm)
			modName = nameForm->GetFullName();
		else if (nullModName != "")
			modName = nullModName.c_str();
		else
			modName = MSF_Localization::GetTranslation(MSF_Localization::Keys::menuNoneText); //"None"
		GFxValue arrArgs[3]; //obj?
		arrArgs[0].SetString(apName);
		arrArgs[1].SetString(modName);
		arrArgs[2].SetBool(isAmmo);
		movieRoot->Invoke("root.UpdateWidgetQuickkeyMod", nullptr, arrArgs, 3);
		return true;
	}

	bool ClearWidgetQuickkeyMod()
	{
		static BSFixedString menuName("MSFwidget");
		IMenu* widgetMenu = (*g_ui)->GetMenu(menuName);
		if (!widgetMenu)
			return false;
		GFxMovieRoot* movieRoot = widgetMenu->movie->movieRoot;
		if (!movieRoot)
			return false;
		//GFxValue arrArgs[2]; //obj?
		//arrArgs[0].SetString(nameForm->GetFullName());
		//arrArgs[1].SetBool(isAmmo);
		movieRoot->Invoke("root.ClearWidgetQuickkeyMod", nullptr, nullptr, 0);
		return true;
	}

	bool UpdateWidgetData(TESObjectWEAP::InstanceData* instanceData)
	{
		Actor* playerActor = *g_player;
		if (!instanceData)
			instanceData = Utilities::GetEquippedWeaponInstanceData(playerActor); //Utilities::GetEquippedInstanceData(playerActor, 41);
		static BSFixedString menuName("MSFwidget");
		IMenu* widgetMenu = (*g_ui)->GetMenu(menuName);
		if (!widgetMenu)
			return false;
		GFxMovieRoot* movieRoot = widgetMenu->movie->movieRoot;
		if (!movieRoot)
			return false;
		const char* ammoName = "";
		std::string fmstr = MSF_Data::GetFMString(instanceData);
		std::string muzzlestr = MSF_Data::GetMuzzleString(instanceData);
		std::string scopestr = MSF_Data::GetScopeString(instanceData);
		const char* firingMode = fmstr.c_str();
		const char* muzzleName = muzzlestr.c_str();
		const char* scopeName = scopestr.c_str();
		UInt32 shapeID = 0; //baseAmmoID << 10 + ammoType + muzzleID << 20 + scopeID << 26
		if (instanceData && instanceData->ammo)
			ammoName = instanceData->ammo->GetFullName();
		bool isInPA = IsInPowerArmor(playerActor);
		_DEBUG("updWidget: %02X", isInPA);
		//BSTEventSink_ExitPowerArmor__Event_ /BSTEventSink_PreloadPowerArmor__Event_
		GFxValue arrArgs[6];
		arrArgs[0].SetString(ammoName);
		arrArgs[1].SetString(firingMode);
		arrArgs[2].SetString(muzzleName);
		arrArgs[3].SetString(scopeName);
		arrArgs[4].SetUInt(shapeID);
		arrArgs[5].SetBool(isInPA);
		//_DEBUG("ammo: %s, fm: %s, muzzle: %s, sc: %s", ammoName, firingMode, muzzleName, scopeName);
		movieRoot->Invoke("root.UpdateWidgetData", nullptr, arrArgs, 6);
		return true;
		//out of ammo, full (+1?)
	}

	inline bool PlayMenuFailed()
	{
		MSF_Base::PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundMenuFail, 3, nullptr);
		return false;
	}

	bool HandleToggleMenu(ModSelectionMenu* selectMenu, ModData* mods)
	{
		if (!selectMenu || !selectMenu->customMenuData)
			return PlayMenuFailed();
		//check conditions
		//cross check custom and MSFMenu
		_DEBUG("selectcustommenu");
		//static BSFixedString menuName("MSFMenu"); //!/rewrite
		ModSwitchManager::OpenedMenuData openedMenu = MSF_MainData::modSwitchManager.GetOpenedMenuData();
		if (openedMenu.isOpening)
			return PlayMenuFailed();
		if (openedMenu.selectMenu)
		{
			if (!(*g_ui)->IsMenuOpen(BSFixedString(openedMenu.selectMenu->customMenuData->menuName.c_str())))
				MSF_MainData::modSwitchManager.CloseOpenedMenu();
				//sound:menuSuccess
			else if (openedMenu.selectMenu == selectMenu)
			{
				MSF_MainData::modSwitchManager.CloseOpenedMenu();
				return true;
			}
			else if (!(selectMenu->customMenuData->extFlags & MSFCustomMenuData::kExtFlag_DontAllowMenuChange) && (MSF_MainData::MCMSettingFlags & MSF_MainData::bAllowChangingMSFMenus))
				MSF_MainData::modSwitchManager.CloseOpenedMenu();
			else
				return PlayMenuFailed();
		}
		_DEBUG("menuchecks0ok");
		//if (MSF_MainData::modSwitchManager.GetOpenedMenu() == selectMenu)
		//{
		//	std::string closePath = "root." + selectMenu->scaleformName + "_loader.content.Close"; //visible
		//	GFxValue result;
		//	menuRoot->Invoke(closePath.c_str(), &result, nullptr, 0);
		//	if (result.type == GFxValue::kType_Bool && result.data.boolean)
		//		MSF_MainData::modSwitchManager.SetOpenedMenu(nullptr);
		//	return nullptr;
		//}
		//else if (MSF_MainData::modSwitchManager.GetOpenedMenu() != nullptr)
		//	return nullptr;
		bool res = MSF_MainData::modSwitchManager.SetOpenedMenu(selectMenu, mods);
		if (res)
			return true;
		return PlayMenuFailed();
	}

	bool GetAmmoListForScaleform(GFxMovieRoot* menuRoot, GFxValue* dst)
	{
		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bAmmoRequireWeaponToBeDrawn) && !(*g_player)->actorState.IsWeaponDrawn())
			return false;
		BGSObjectInstanceExtra* moddata = Utilities::GetEquippedModData(*g_player, 41);
		TESObjectWEAP* baseWeapon = Utilities::GetEquippedGun(*g_player);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(moddata, baseWeapon);
		menuRoot->CreateArray(dst);
		if (baseAmmo)
		{
			TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(*g_player);
			auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
			if (itAD != MSF_MainData::ammoDataMap.end())
			{
				AmmoData* itAmmoData = itAD->second;
				UInt64 ammoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, baseAmmo);
				MSF_MainData::modSwitchManager.menuLock.Lock();
				if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch) || ammoCount != 0)
				{
					GFxValue BammoArg;
					menuRoot->CreateObject(&BammoArg);
					RegisterString(&BammoArg, menuRoot, "ammoName", baseAmmo->fullName.name.c_str());
					MSF_MainData::modSwitchManager.AddDisplayedAmmoNoLock(&itAmmoData->baseAmmoData);
					RegisterBool(&BammoArg, menuRoot, "isEquipped", instanceData->ammo == baseAmmo);
					RegisterInt(&BammoArg, menuRoot, "ammoCount", ammoCount);
					dst->PushBack(&BammoArg);
					_DEBUG("moddataptr: %p, %p", &itAmmoData->baseAmmoData, itAmmoData->baseAmmoData.mod);
				}
				for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
				{
					UInt64 ammoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, itAmmoMod->ammo);
					if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch) || ammoCount != 0 || instanceData->ammo == itAmmoMod->ammo) //update
					{
						GFxValue ammoArg;
						menuRoot->CreateObject(&ammoArg);
						RegisterString(&ammoArg, menuRoot, "ammoName", itAmmoMod->ammo->fullName.name.c_str());
						MSF_MainData::modSwitchManager.AddDisplayedAmmoNoLock(itAmmoMod._Ptr);
						RegisterBool(&ammoArg, menuRoot, "isEquipped", instanceData->ammo == itAmmoMod->ammo); //weapState!
						RegisterInt(&ammoArg, menuRoot, "ammoCount", ammoCount);
						dst->PushBack(&ammoArg);
						_DEBUG("moddataptr: %p, %p", itAmmoMod._Ptr, itAmmoMod->mod);
					}
				}
				MSF_MainData::modSwitchManager.menuLock.Release();
			}
		}
		if (!baseAmmo || dst->GetArraySize() < 2)
		{
			MSF_MainData::modSwitchManager.ClearAmmoDisplayChioces();
			return false;
		}
		return true;
	}

	bool GetModListForScaleform(GFxMovieRoot* menuRoot, GFxValue* dst, ModData* mods)
	{
		if (!mods)
			return false;
		//BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41); 
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedWeaponModData(*g_player);
		if (!modData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		std::vector<KeywordValue> instantiationValues;
		if (!Utilities::GetParentInstantiationValues(modData, mods->attachParentValue, &instantiationValues))
			return false;

		menuRoot->CreateArray(dst);
		ModData::ModCycle* modCycle = nullptr;
		for (std::vector<KeywordValue>::iterator itData = instantiationValues.begin(); itData != instantiationValues.end(); itData++)
		{
			KeywordValue value = *itData;
			auto itCycle = mods->modCycleMap.find(value);
			if (itCycle != mods->modCycleMap.end() && modCycle)
			{
				_MESSAGE("Ambiguity error"); //or combine
				return false;
			}
			modCycle = itCycle->second;
		}
		if (!modCycle)
			return false;

		if (modCycle->flags & ModData::ModCycle::bUIRequireWeaponToBeDrawn)
		{
			if (!(*g_player)->actorState.IsWeaponDrawn())
				return false;
			MSF_MainData::modSwitchManager.SetMenuNeedsDrawnWeapon();
		}
		BGSKeyword* apkw = GetKeywordFromValueArray(AttachParentArray::iDataType, mods->attachParentValue);
		const char* apName = "";
		if (apkw)
			apName = apkw->GetFullName();

		struct DisplayData
		{
			ModData::Mod* mod;
			const char* modName;
			const char* apName;
			bool isEquipped;
			bool reqsOK;
		};
		std::vector<DisplayData> displayData;
		BGSMod::Attachment::Mod* attachedMod = Utilities::GetModAtAttachPoint(modData, mods->attachParentValue);
		if (!(modCycle->flags & ModData::ModCycle::bCannotHaveNullMod))
		{
			DisplayData itemData;
			itemData.mod = nullptr;
			itemData.modName = MSF_Localization::GetTranslation(MSF_Localization::Keys::menuNoneText); //"None"
			itemData.apName = apName;
			itemData.isEquipped = attachedMod == nullptr;
			bool reqsOK = true;
			//check requirements
			itemData.reqsOK = reqsOK;
			displayData.push_back(itemData);
		}
		for (ModData::ModVector::iterator itMod = modCycle->mods.begin(); itMod != modCycle->mods.end(); itMod++) //idx, bShowAll, remove?
		{
			ModData::Mod* currMod = *itMod;
			DisplayData itemData;
			itemData.mod = currMod;
			itemData.modName = currMod->mod->fullName.name.c_str();
			itemData.apName = apName;
			itemData.isEquipped = currMod->mod == attachedMod;
			bool reqsOK = true;
			if (currMod->mod->flags & ModData::Mod::bRequireLooseMod) //update
			{
				TESObjectMISC* looseMod = Utilities::GetLooseMod(currMod->mod);
				if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) == 0)
					reqsOK = false;
			}
			//check requirements
			itemData.reqsOK = reqsOK;
			displayData.push_back(itemData);
		}
		if (displayData.size() < 2)
			return false;
		
		MSF_MainData::modSwitchManager.menuLock.Lock();
		for (auto dataitem : displayData)
		{
			MSF_MainData::modSwitchManager.AddDisplayedModNoLock(dataitem.mod);
			GFxValue modArg;
			menuRoot->CreateObject(&modArg);
			RegisterString(&modArg, menuRoot, "modName", dataitem.modName);
			RegisterString(&modArg, menuRoot, "apName", dataitem.apName);
			RegisterBool(&modArg, menuRoot, "isEquipped", dataitem.isEquipped);
			RegisterBool(&modArg, menuRoot, "reqsOK", dataitem.reqsOK);
			dst->PushBack(&modArg);
		}
		MSF_MainData::modSwitchManager.menuLock.Release();
		return true;
	}

	bool DisplaySelectionMenu(ModSelectionMenu* selectMenu, ModData* mods, GFxMovieRoot* menuRoot) //!/rewrite
	{
		//if anim, play anim, set menu to open
		_DEBUG("selectmenu");
		if (selectMenu && menuRoot)
		{
			_DEBUG("menurootok");
			Setting* setting = GetINISetting("sLanguage:General");
			std::string loc = (setting && setting->GetType() == Setting::kType_String) ? setting->data.s : "en";
			bool isInPA = IsInPowerArmor(*g_player);

			switch (selectMenu->type)
			{
			case ModSelectionMenu::kType_AmmoMenu:
			{
				GFxValue data[3];
				if (!GetAmmoListForScaleform(menuRoot, &data[0]))
					break;
				_DEBUG("ammolistok");
				data[1].SetBool(isInPA);
				data[2].SetString(loc.c_str());
				//std::string openPath = "root." + selectMenu->scaleformName + "_loader.content.ReceiveAmmoData";
				std::string openPath = "root.ReceiveAmmoData";
				if (menuRoot->Invoke(openPath.c_str(), nullptr, data, 3))
					return true;
				break;
			}
			case ModSelectionMenu::kType_ModMenu:
			{
				GFxValue data[3];
				if (!GetModListForScaleform(menuRoot, &data[0], mods))
					break;
				data[1].SetBool(isInPA);
				data[2].SetString(loc.c_str());
				std::string openPath = "root.ReceiveModData";
				if (menuRoot->Invoke(openPath.c_str(), nullptr, data, 3))
					return true;
				break;
			}
			case ModSelectionMenu::kType_All:
			{
				GFxValue data[6];
				bool bAmmo = GetAmmoListForScaleform(menuRoot, &data[0]);

				bool bMod = false;
				TESObjectWEAP* weap = Utilities::GetEquippedWeapon(*g_player);
				AttachParentArray* baseArray = nullptr;
				if (weap)
					baseArray = (AttachParentArray*)&weap->attachParentArray;
				BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player);
				auto apList = Utilities::GetAllAttachPoints(modData, baseArray);
				for (auto val : apList)
				{
					auto itKB = MSF_MainData::keybindAPMap.find(val);
					if (itKB == MSF_MainData::keybindAPMap.end())
						continue;
					bMod |= GetModListForScaleform(menuRoot, &data[1], itKB->second->modData);
				}
				if (!bAmmo && !bMod)
					break;
				data[2].SetBool(bAmmo);
				data[3].SetBool(bMod);
				data[4].SetBool(isInPA);
				data[5].SetString(loc.c_str());
				std::string openPath = "root.ReceiveAmmoAndModData";
				if (menuRoot->Invoke(openPath.c_str(), nullptr, data, 6))
					return true;
				break;
			}
			default: break;
			}
		}
		_DEBUG("invokefail");
		MSF_MainData::modSwitchManager.CloseOpenedMenu();
		MSF_MainData::modSwitchManager.ClearDisplayChioces();
		MSF_Base::PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundMenuFail, 3, nullptr);
		return false;
	}

	bool UpdateAmmoMenuCount(TESAmmo* ammo, UInt32 newCount)
	{
		ModSwitchManager::OpenedMenuData menuData = MSF_MainData::modSwitchManager.GetOpenedMenuData();
		if (!menuData.selectMenu || menuData.isOpening)
			return false;
		IMenu* ammoMenu = (*g_ui)->GetMenu(BSFixedString(menuData.selectMenu->scaleformName.c_str()));
		if (!ammoMenu || !ammoMenu->movie)
			return false;
		GFxMovieRoot* menuRoot = ammoMenu->movie->movieRoot;
		if (!menuRoot)
			return false;
		auto idxs = MSF_MainData::modSwitchManager.GetIdxsOfAmmo(ammo);
		if (idxs.size() == 0)
			return false;
		_DEBUG("updammoready");
		GFxValue data;
		menuRoot->CreateArray(&data);
		for (auto idx : idxs)
		{
			GFxValue ammoArg;
			menuRoot->CreateObject(&ammoArg);
			RegisterInt(&ammoArg, menuRoot, "ammoIdx", idx);
			RegisterInt(&ammoArg, menuRoot, "ammoCount", newCount);
			data.PushBack(&ammoArg);
		}
		std::string openPath = "root.UpdateAmmoCount";
		_DEBUG("updinvoke");
		if (menuRoot->Invoke(openPath.c_str(), nullptr, &data, 1))
		{
			_DEBUG("updinvokedone");
			return true;
		}
		return false;
	}
	bool UpdateAmmoMenuEq(BGSMod::Attachment::Mod* ammoMod)
	{
		ModSwitchManager::OpenedMenuData menuData = MSF_MainData::modSwitchManager.GetOpenedMenuData();
		if (!menuData.selectMenu || menuData.isOpening || !(menuData.selectMenu->type & 1))
			return false;
		IMenu* ammoMenu = (*g_ui)->GetMenu(BSFixedString(menuData.selectMenu->scaleformName.c_str()));
		if (!ammoMenu || !ammoMenu->movie)
			return false;
		GFxMovieRoot* menuRoot = ammoMenu->movie->movieRoot;
		if (!menuRoot)
			return false;
		if (!ammoMod)
			ammoMod = Utilities::GetModAtAttachPoint(Utilities::GetEquippedModData(*g_player), MSF_MainData::ammoAP);
		UInt32 idx = MSF_MainData::modSwitchManager.GetIdxOfAmmoMod(ammoMod);
		GFxValue ammoArg[1];
		ammoArg[0].SetInt(idx);
		std::string openPath = "root.UpdateEquippedAmmo";
		if (menuRoot->Invoke(openPath.c_str(), nullptr, ammoArg, 1))
			return true;
		return false;
	}
	bool UpdateModMenuReqs(TESObjectMISC* misc, UInt32 newCount)
	{
		ModSwitchManager::OpenedMenuData menuData = MSF_MainData::modSwitchManager.GetOpenedMenuData();
		if (!menuData.selectMenu || menuData.isOpening || !(menuData.selectMenu->type & 2))
			return false;
		IMenu* modMenu = (*g_ui)->GetMenu(BSFixedString(menuData.selectMenu->scaleformName.c_str()));
		if (!modMenu || !modMenu->movie)
			return false;
		GFxMovieRoot* menuRoot = modMenu->movie->movieRoot;
		if (!menuRoot)
			return false;
		auto idxs = MSF_MainData::modSwitchManager.GetIdxsOfMISCMod(misc);
		if (idxs.size() == 0)
			return false;
		GFxValue data;
		menuRoot->CreateArray(&data);
		for (auto idx : idxs)
		{
			GFxValue modArg;
			menuRoot->CreateObject(&modArg);
			RegisterInt(&modArg, menuRoot, "modIdx", idx);
			RegisterBool(&modArg, menuRoot, "reqsOK", newCount != 0);
			data.PushBack(&modArg);
		}
		std::string openPath = "root.UpdateModRequirements";
		if (menuRoot->Invoke(openPath.c_str(), nullptr, &data, 1))
			return true;
		return false;
	}


	bool ToggleGlobalMenu(ModSelectionMenu* menu, std::vector<KeywordValue>* attachPoints)
	{
		//if anim, play anim, set menu to open
		return false;
		GFxMovieRoot* menuRoot = nullptr;// HandleToggleMenu(menu);
		if (!menuRoot)
			return false;
		//getmodassociations
		//send data to menu
		
		//receive selected mod
	}

	void RegisterMSFScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot)
	{
		RegisterFunction<SendF4SEVersion>(codeObj, movieRoot, "GetF4SEVersion");
		RegisterFunction<SendGameVersion>(codeObj, movieRoot, "GetGameVersion");
		RegisterFunction<SendMSFVersion>(codeObj, movieRoot, "GetMSFVersion");
		RegisterFunction<SendLocalization>(codeObj, movieRoot, "GetLocalization");
		RegisterFunction<ReceiveScaleformProperties>(codeObj, movieRoot, "SendInterfaceProperties");
	}

	void RegisterMSFmenuFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot)
	{
		RegisterFunction<MenuClosedEventSink>(codeObj, movieRoot, "MenuClosedEventDispatcher");
		RegisterFunction<MenuOpenedEventSink>(codeObj, movieRoot, "MenuOpenedEventDispatcher");
		RegisterFunction<ReceiveSelectedAmmoIdx>(codeObj, movieRoot, "SendBackSelectedAmmoIdx");
		RegisterFunction<ReceiveSelectedModIdx>(codeObj, movieRoot, "SendBackSelectedModIdx");
	}

	void RegisterMSFwidgetFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot)
	{
		RegisterFunction<SendWidgetUpdate>(codeObj, movieRoot, "RequestWidgetUpdate");
		RegisterFunction<SendWidgetSettings>(codeObj, movieRoot, "RequestWidgetSettings");
	}

	void RegisterMSFCustomMenus()
	{
		for (auto it = MSF_MainData::keybindIDMap.begin(); it != MSF_MainData::keybindIDMap.end(); it++)
		{
			if (it->second->selectMenu)
				MSFCustomMenu::RegisterCustomMenu(it->second->selectMenu->customMenuData);
		}
	}

	bool RegisterScaleformCallback(GFxMovieView * view, GFxValue * f4se_root)
	{
		GFxMovieRoot* movieRoot = view->movieRoot;

		GFxValue currentSWFPath;
		const char* currentSWFPathString = nullptr;

		if (movieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url")) 
			currentSWFPathString = currentSWFPath.GetString();

		if (currentSWFPathString && strcmp(currentSWFPathString, "Interface/MSFwidget.swf") == 0)
		{
			GFxValue root; movieRoot->GetVariable(&root, "root");
			GFxValue msf; movieRoot->CreateObject(&msf);
			root.SetMember("msf", &msf);
			RegisterMSFScaleformFuncs(&msf, movieRoot);
			RegisterMSFwidgetFuncs(&msf, movieRoot);
		}
		// Look for the menu that we want to inject into.
		else if (false)//(currentSWFPathString && strcmp(currentSWFPathString, "Interface/MSFMenu.swf") == 0) 
		{
			GFxValue root; movieRoot->GetVariable(&root, "root");
			MSF_MainData::MSFMenuRoot = movieRoot;

			// Register native code object
			GFxValue msf; movieRoot->CreateObject(&msf);
			root.SetMember("msf", &msf);
			RegisterMSFScaleformFuncs(&msf, movieRoot);
			RegisterMSFmenuFuncs(&msf, movieRoot);
			_MESSAGE("MSF code object created in MSFMenu.");

			for (auto it = MSF_MainData::keybindIDMap.begin(); it != MSF_MainData::keybindIDMap.end(); it++)
			{
				if (it->second->selectMenu)
				{
					if (it->second->selectMenu->type == ModSelectionMenu::kType_Global)
						continue;

					// Inject swf
					GFxValue loader, urlRequest;
					movieRoot->CreateObject(&loader, "flash.display.Loader");
					std::string filename = it->second->selectMenu->scaleformName + ".swf";
					movieRoot->CreateObject(&urlRequest, "flash.net.URLRequest", &GFxValue(filename.c_str()), 1);

					std::string loaderstring = it->second->selectMenu->scaleformName + "_loader";
					root.SetMember(loaderstring.c_str(), &loader);
					std::string loadstring = "root." + loaderstring + ".load";
					bool injectionSuccess = movieRoot->Invoke(loadstring.c_str(), nullptr, &urlRequest, 1);

					bool addSuccess = movieRoot->Invoke("root.addChild", nullptr, &loader, 1);

					if (!injectionSuccess || !addSuccess) {
						_MESSAGE("WARNING: MSF Scaleform injection failed for %s.", it->second->selectMenu->scaleformName.c_str());
					}
				}
			}

			_MESSAGE("MSF: custom scaleform menus initialized");

		}
		else if (currentSWFPathString)
		{
			std::string path = currentSWFPathString;
			size_t first = path.find_last_of("/")+1;
			std::string basename = path.substr(first, path.find_last_of(".")-first);
			_DEBUG("MSF code object creation in %s menu.", basename.c_str());
			auto menuData = MSFCustomMenu::customMenuData.find(basename);
			if (menuData != MSFCustomMenu::customMenuData.end())
			{
				_DEBUG("menufound");
				ModSwitchManager::OpenedMenuData openedMenu = MSF_MainData::modSwitchManager.GetOpenedMenuData();
				if (!openedMenu.selectMenu || !openedMenu.selectMenu->customMenuData)
					MSFCustomMenu::CloseMenu(BSFixedString(basename.c_str()));
				else if (openedMenu.selectMenu->customMenuData->menuName != basename)
				{
					MSFCustomMenu::CloseMenu(BSFixedString(basename.c_str()));
					MSF_MainData::modSwitchManager.CloseOpenedMenu();
					_DEBUG("closedboth");
				}
				else if (!openedMenu.isOpening)
					_DEBUG("not opening"); //close OR skip OR do nothing
				else
				{
					GFxValue root, funcObj;
					if (!movieRoot->GetVariable(&root, "root") || !root.IsObject() || !root.GetMember("onMSFCodeObjCreated", &funcObj) || !funcObj.IsFunction()) //!root.HasMember("onMSFCodeObjCreated") || 
					{
						MSF_MainData::modSwitchManager.CloseOpenedMenu();
						MSF_Base::PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundMenuFail, 3, nullptr);
						return true;
					}
					GFxValue type, ver;
					if (!root.GetMember("InterfaceVersion", &ver) || ver.GetType() != GFxValue::Type::kType_UInt || !root.GetMember("InterfaceType", &type) || type.GetType() != GFxValue::Type::kType_UInt)
					{
						MSF_MainData::modSwitchManager.CloseOpenedMenu();
						MSF_Base::PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundMenuFail, 3, nullptr);
						return true;
					}
					UInt32 itype = type.GetUInt();
					UInt32 iver = ver.GetUInt();
					GFxValue funcObj2;
					_DEBUG("ver %i, %i", iver, MIN_SUPPORTED_SWF_AMMO_VERSION);
					if (itype != openedMenu.selectMenu->type || ((itype & 1) && iver < MIN_SUPPORTED_SWF_AMMO_VERSION) || ((itype & 2) && iver < MIN_SUPPORTED_SWF_MOD_VERSION) || ((itype == 1) && (!root.GetMember("ReceiveAmmoData", &funcObj2) || !funcObj2.IsFunction())) || ((itype == 2) && (!root.GetMember("ReceiveModData", &funcObj2) || !funcObj2.IsFunction())) || ((itype == 3) && (!root.GetMember("ReceiveAmmoAndModData", &funcObj2) || !funcObj2.IsFunction())))
					{
						MSF_MainData::modSwitchManager.CloseOpenedMenu();
						MSF_Base::PlayFeedbackSound(MSF_MainData::MCMSettingFlags & MSF_MainData::bPlayFeedbackSoundMenuFail, 3, nullptr);
						return true;
					}
					GFxValue msf; 
					movieRoot->CreateObject(&msf);
					root.SetMember("msf", &msf);
					RegisterMSFScaleformFuncs(&msf, movieRoot);
					RegisterMSFmenuFuncs(&msf, movieRoot);
					_DEBUG("MSF code object created.");
					if (!root.Invoke("onMSFCodeObjCreated", nullptr, &msf, 1))
					{
						MSF_MainData::modSwitchManager.CloseOpenedMenu();
						MSF_Base::PlayFeedbackSound(MSF_MainData::MCMSettingFlags& MSF_MainData::bPlayFeedbackSoundMenuFail, 3, nullptr);
						return true;
					}
					MSF_MainData::modSwitchManager.SetMenuIsOpened();
					DisplaySelectionMenu(openedMenu.selectMenu, openedMenu.mods, movieRoot);
				}
			}
		}

		return true;
	}
}