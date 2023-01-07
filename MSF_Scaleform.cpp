#include "MSF_Scaleform.h"
#include "MSF_Test.h"

RelocPtr <BSScaleformManager *> g_scaleformManager(0x058DE410);

void HandleInputEvent(ButtonEvent * inputEvent)
{
	if (!MSF_MainData::IsInitialized)
		return;
	UInt32	keyCode;
	UInt32	deviceType = inputEvent->deviceType;
	UInt32	keyMask = inputEvent->keyMask;

	// Mouse
	if (deviceType == InputEvent::kDeviceType_Mouse)
		keyCode = InputMap::kMacro_MouseButtonOffset + keyMask;
	// Gamepad
	else if (deviceType == InputEvent::kDeviceType_Gamepad)
		keyCode = InputMap::GamepadMaskToKeycode(keyMask);
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
				//_MESSAGE("fire input");
			}
		}
			break;
#ifdef DEBUG
		case 103:
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//MSF_Test::ListModTable(*g_player);
				//MSF_Test::PrintStoredData();
				//MSF_Scaleform::GetInterfaceVersion();
				//MSF_Scaleform::UpdateWidgetData(nullptr);
				//MSF_Scaleform::SetWidgetVisibility(true);
				//MSFMenu::OpenMenu();
				//static BSFixedString menuName("MSFMenu");
				//CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
				MSFAmmoMenu::OpenMenu();
				MSFModMenu::OpenMenu();
				//MSF_Test::ListEquippedItemTypes();
				//MSF_Test::ArmorAttachTest();
				//MSF_Test::RemapAnimTest(); //sub_141387BE0
				//Utilities::PlayIdleAction(*g_player, (BGSAction*)LookupFormByID(0x3B248));
				//Utilities::PlayIdle(*g_player, MSF_MainData::fireIdle1stP);
				//MSF_Base::BurstTest(nullptr);
				//MSF_Test::DumpUnknownData((*g_player));
				//MSF_Test::DumpActorValue();
				//MSF_Test::DumpExtraDataInfo();
				//MSF_Test::CallAttachModToInvItem();
				//CALL_MEMBER_FN((*g_player), UpdateEquipment)();
				_MESSAGE("test1");
			}
		}
		break;
		case 104:
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//Utilities::AddRemActorValue((*g_player), MSF_MainData::BurstModeTime, true);
				MSF_Test::AttachModInternalTest(true, 2);
				//MSF_Test::CallAddItem();
				//MSF_Test::CallPlayAction();
				//MSF_Test::CallAttachModToInvItem();
				_MESSAGE("test2");
			}
		}
		break;
		case 105:
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				//Utilities::AddRemActorValue((*g_player), MSF_MainData::BurstModeTime, false);
				//CALL_MEMBER_FN((*g_player), QueueUpdate)(true, 0, true, 0);
				MSF_Test::AttachModInternalTest(false, 2);
				//MSF_Test::CallRemoveModFromInvItem();
				_MESSAGE("test3");
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
				KeybindData* keyFn = MSF_Data::GetKeyFunctionID(keyCode, modifiers);
				if (!keyFn)
					break;
				_MESSAGE("key: %i, type: %02X", keyFn->keyCode, keyFn->type);
				if (keyFn->type & KeybindData::bCancel)
					MSF_MainData::switchData.ClearData();
				else if(keyFn->type & KeybindData::bIsAmmo)
				{
					if (keyFn->type & KeybindData::bHUDselection)
						MSF_Scaleform::ToggleAmmoMenu(keyFn->selectMenu);
					else
						MSF_Base::SwitchAmmoHotkey(keyFn->type & KeybindData::mNumMask);
				}
				else if (keyFn->type & KeybindData::bHUDselection)
					MSF_Scaleform::ToggleModMenu(keyFn->selectMenu, &keyFn->modAssociations);
				else if (keyFn->type & KeybindData::bToggle)
					MSF_Base::ToggleModHotkey(&keyFn->modAssociations);
				else
					MSF_Base::SwitchModHotkey(keyFn->type & KeybindData::mNumMask, &keyFn->modAssociations);
			}
		}
			break;
		}
	}
	else if (isUp)
	{
		if (keyCode == 256) 
		{
			//burst up
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

namespace MSF_Scaleform
{
	class OnMCMClosed : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			// OnMCMClosed
			_MESSAGE("mcm closed");
		}
	};

	class SendF4SEVersion : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetInt(0);
			if (args->numArgs != 0) return;
			args->result->SetInt(110163);
		}
	};

	class SendGameVersion : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetInt(0);
			if (args->numArgs != 0) return;
			args->result->SetInt(110163);
		}
	};

	class SendMSFVersion : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetInt(0);
			if (args->numArgs != 0) return;
			args->result->SetInt(110163);
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

	class MenuClosedEventSink : public GFxFunctionHandler { //also:onmenuopencloseevent:hudmenu
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(1);
			//std::string menu = args->args[0].data.string;
			MSF_MainData::switchData.openedMenu = nullptr;
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
	class ReceiveSelectedAmmo : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);

			if (args->numArgs != 1) return;
			MSF_MainData::switchData.openedMenu = nullptr;
			if (args->args[0].data.obj)
				MSF_Base::SwitchToSelectedAmmo(args->args[0].data.obj, args->args[1].data.boolean);

			args->result->SetBool(true);
		}
	};

	class ReceiveSelectedMod : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);

			if (args->numArgs != 2) return;
			MSF_MainData::switchData.openedMenu = nullptr;
			MSF_Base::SwitchToSelectedMod(args->args[0].data.obj, args->args[1].data.obj);

			args->result->SetBool(true);
		}
	};

	class ReceiveWidgetProperties : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);

			if (args->numArgs != 3) return;
			UInt8 type = args->args[1].data.u32 & 0x3;
			UInt32 version = args->args[2].data.u32;
			if (type == ModSelectionMenu::kType_Widget && version >= MIN_SUPPORTED_SWF_WIDGET_VERSION)
				MSF_MainData::widgetMenu = nullptr; // = find args->args[0].data.string in array of Keybind; MSF_MainData::widgetMenu->type = type; MSF_MainData::widgetMenu->version = version;
			else if ((type == ModSelectionMenu::kType_AmmoMenu && version >= MIN_SUPPORTED_SWF_AMMO_VERSION) || (type == ModSelectionMenu::kType_ModMenu && version >= MIN_SUPPORTED_SWF_MOD_VERSION))
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

	UInt32 GetInterfaceVersion() //ModSelectionMenu* customMenu
	{
		IMenu * pHUD = nullptr;
		static BSFixedString menuName("HUDMenu");
		if ((*g_ui) != nullptr && (pHUD = (*g_ui)->GetMenu(menuName), pHUD))
		{
			GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
			if (movieRoot != nullptr)
			{
				//GFxValue arrArgs[2];
				//arrArgs[0].SetUInt(hp);
				//arrArgs[1].SetUInt(dt);
				//UInt32 a = arrArgs[0].GetUInt();
				//UInt32 b = arrArgs[1].GetUInt();
				_MESSAGE("calling");

				GFxValue vis, ver, verfn;
				movieRoot->GetVariable(&vis, "root.msf_loader.content.visible");
				movieRoot->GetVariable(&ver, "root.msf_loader.content.InterfaceVersion");
				movieRoot->Invoke("root.msf_loader.content.SendInterfaceVersion", &verfn, nullptr, 0);
				_MESSAGE("vis type: %08X, ver type: %08X, verfn type: %08X", vis.GetType(), ver.GetType(), verfn.GetType());

				//_MESSAGE("res type: %08X", GFxGlobaltest.GetType());
				//_MESSAGE("res: %08X", GFxGlobaltest.GetInt());
				//return GFxGlobaltest.GetInt();
			}
		}
		return 0;
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
		GFxValue arrArgs[4]; //obj?
		arrArgs[0].SetUInt(MSF_MainData::MCMSettingFlags);
		arrArgs[1].SetUInt(0);
		arrArgs[2].SetUInt(0);
		arrArgs[3].SetUInt(0);
		movieRoot->Invoke("root.UpdateWidgetSettings", nullptr, arrArgs, 4);
		return true;
	}

	bool UpdateWidgetData()
	{
		Actor* playerActor = *g_player;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
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
		GFxValue arrArgs[5];
		arrArgs[0].SetString(ammoName);
		arrArgs[1].SetString(firingMode);
		arrArgs[2].SetString(muzzleName);
		arrArgs[3].SetString(scopeName);
		arrArgs[4].SetUInt(shapeID);
		//_MESSAGE("ammo: %s, fm: %s, muzzle: %s, sc: %s", ammoName, firingMode, muzzleName, scopeName);
		movieRoot->Invoke("root.UpdateWidgetData", nullptr, arrArgs, 5);
		return true;
		//out of ammo, full (+1?)
	}

	bool ToggleAmmoMenu(ModSelectionMenu* selectMenu)
	{
		//if (!MSF_MainData::MSFMenuRoot || !selectMenu)
		//	return false;
		//check conditions
		//GFxMovieRoot* menuRoot = MSF_MainData::MSFMenuRoot;
		
		static BSFixedString menuName("MSFAmmoMenu");
		IMenu* ammoMenu = (*g_ui)->GetMenu(menuName);
		if (!ammoMenu)
			return false;
		GFxMovieRoot* menuRoot = ammoMenu->movie->movieRoot;
		if (!menuRoot)
			return false;

		//if (MSF_MainData::switchData.openedMenu == selectMenu)
		//{
		//	std::string closePath = "root." + selectMenu->scaleformID + "_loader.content.close";
		//	GFxValue result;
		//	menuRoot->Invoke(closePath.c_str(), &result, nullptr, 0); //"root.msf_loader.ammoMenu.close"
		//	if (result.GetBool())
		//	{
		//		MSF_MainData::switchData.openedMenu = nullptr;
		//		return true;
		//	}
		//	return false;
		//}
		//else if (MSF_MainData::switchData.openedMenu != nullptr)
		//	return false;

		if (!(*g_ui)->IsMenuOpen(menuName))
		{
			MSFAmmoMenu::CloseMenu();
			return true;
		}
		else if (!(*g_ui)->IsMenuOpen(BSFixedString("MSFModMenu")))
		{
			MSFModMenu::CloseMenu();
		}

		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (!MSF_Base::InitWeapon())
			return false;

		GFxValue args[1];
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(*g_player, 41);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
		GFxValue ammoData;
		menuRoot->CreateArray(&ammoData);
		if (baseAmmo)
		{
			TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(*g_player);
			AmmoData* itAmmoData = MSF_MainData::ammoDataMap[baseAmmo];
			if (itAmmoData)
			{
				UInt64 ammoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, baseAmmo);
				if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch) || ammoCount != 0)
				{
					GFxValue BammoArg;
					menuRoot->CreateObject(&BammoArg);
					RegisterString(&BammoArg, menuRoot, "ammoName", baseAmmo->fullName.name.c_str());
					RegisterObject(&BammoArg, menuRoot, "ammoObj", &itAmmoData->baseAmmoData);
					RegisterBool(&BammoArg, menuRoot, "isEquipped", instanceData->ammo == baseAmmo);
					RegisterInt(&BammoArg, menuRoot, "ammoCount", ammoCount);
					ammoData.PushBack(&BammoArg);
				}
				for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
				{
					UInt64 ammoCount = Utilities::GetInventoryItemCount((*g_player)->inventoryList, baseAmmo);
					if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch) || ammoCount != 0)
					{
						GFxValue ammoArg;
						menuRoot->CreateObject(&ammoArg);
						RegisterString(&ammoArg, menuRoot, "ammoName", itAmmoMod->ammo->fullName.name.c_str());
						RegisterObject(&ammoArg, menuRoot, "ammoObj", itAmmoMod._Ptr);
						RegisterBool(&ammoArg, menuRoot, "isEquipped", instanceData->ammo == itAmmoMod->ammo);
						RegisterInt(&ammoArg, menuRoot, "ammoCount", ammoCount);
						ammoData.PushBack(&ammoArg);
					}
				}
			}
		}
		if (!baseAmmo || ammoData.GetArraySize() < 2)
			return false;

		args[0].SetMember("ammoData", &ammoData);
		//std::string openPath = "root." + selectMenu->scaleformID + "_loader.content.open";
		//MSF_MainData::switchData.openedMenu = selectMenu;
		MSFAmmoMenu::OpenMenu();
		std::string openPath = "root.ReceiveAmmoData";
		if (menuRoot->Invoke(openPath.c_str(), nullptr, args, 1))
			return true;
		MSF_MainData::switchData.openedMenu = nullptr;
		return false;
	}

	bool ToggleModMenu(ModSelectionMenu* selectMenu, std::vector<ModAssociationData*>* modAssociations)
	{
		//if (!MSF_MainData::MSFMenuRoot || !selectMenu)
		//	return false;
		if (!modAssociations)
			return false;
		//check conditions
		//GFxMovieRoot* menuRoot = MSF_MainData::MSFMenuRoot

		static BSFixedString menuName("MSFModMenu");
		IMenu* ammoMenu = (*g_ui)->GetMenu(menuName);
		if (!ammoMenu)
			return false;
		GFxMovieRoot* menuRoot = ammoMenu->movie->movieRoot;
		if (!menuRoot)
			return false;

		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41);
		if (!modData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;

		//if (MSF_MainData::switchData.openedMenu == selectMenu)
		//{
		//	std::string closePath = "root." + selectMenu->scaleformID + "_loader.content.close";
		//	GFxValue result;
		//	menuRoot->Invoke(closePath.c_str(), &result, nullptr, 0);
		//	if (result.GetBool())
		//	{
		//		MSF_MainData::switchData.openedMenu = nullptr;
		//		return true;
		//	}
		//	return false;
		//}
		//else if (MSF_MainData::switchData.openedMenu != nullptr)
		//	return false;

		if (!(*g_ui)->IsMenuOpen(menuName))
		{
			MSFModMenu::CloseMenu();
			return true;
		}
		else if (!(*g_ui)->IsMenuOpen(BSFixedString("MSFAmmoMenu")))
		{
			MSFAmmoMenu::CloseMenu();
		}

		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bSetLooseMods;

		GFxValue args[2];
		GFxValue modPtrs;
		menuRoot->CreateArray(&modPtrs);
		GFxValue bNull;
		menuRoot->CreateObject(&bNull);
		MSF_MainData::switchData.SwitchFlags &= ~(ModAssociationData::mBitTransferMask);
		MSF_MainData::switchData.AnimToPlay1stP = nullptr;
		MSF_MainData::switchData.AnimToPlay3rdP = nullptr;

		for (std::vector<ModAssociationData*>::iterator itData = modAssociations->begin(); itData != modAssociations->end(); itData++)
		{
			UInt8 type = (*itData)->GetType();
			if (type == 0x1)
			{
				SingleModPair* modPair = static_cast<SingleModPair*>(*itData);
				if (Utilities::HasObjectMod(modData, modPair->modPair.parentMod))
				{
					if (modPair->flags & ModAssociationData::bRequireLooseMod)
					{
						TESObjectMISC* looseMod = Utilities::GetLooseMod(modPair->modPair.functionMod);
						if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
							break;
					}
					GFxValue modArg;
					menuRoot->CreateObject(&modArg);
					RegisterString(&modArg, menuRoot, "modName", modPair->modPair.functionMod->fullName.name.c_str());
					RegisterObject(&modArg, menuRoot, "modObj", modPair->modPair.functionMod);
					RegisterBool(&modArg, menuRoot, "isEquipped", Utilities::HasObjectMod(modData, modPair->modPair.functionMod));
					modPtrs.PushBack(&modArg);
					RegisterBool(&bNull, menuRoot, "bNullMod", true);
					MSF_MainData::switchData.SwitchFlags |= (modPair->flags & ModAssociationData::mBitTransferMask);
					MSF_MainData::switchData.AnimToPlay1stP = modPair->animIdle_1stP;
					MSF_MainData::switchData.AnimToPlay3rdP = modPair->animIdle_3rdP;
					break;
				}
			}
			else if (type == 0x3)
			{
				MultipleMod* mods = static_cast<MultipleMod*>(*itData);
				if (Utilities::HasObjectMod(modData, mods->parentMod))
				{
					for (std::vector<BGSMod::Attachment::Mod*>::iterator itMod = mods->functionMods.begin(); itMod != mods->functionMods.end(); itMod++)
					{
						if (mods->flags & ModAssociationData::bRequireLooseMod)
						{
							TESObjectMISC* looseMod = Utilities::GetLooseMod(*itMod);
							if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
								continue;
						}
						GFxValue modArg;
						menuRoot->CreateObject(&modArg);
						RegisterString(&modArg, menuRoot, "modName", (*itMod)->fullName.name.c_str());
						RegisterObject(&modArg, menuRoot, "modObj", *itMod);
						RegisterBool(&modArg, menuRoot, "isEquipped", Utilities::HasObjectMod(modData, (*itMod)));
						modPtrs.PushBack(&modArg);
					}
					RegisterBool(&bNull, menuRoot, "bNullMod", mods->flags & ModAssociationData::bCanHaveNullMod);
					MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
					MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
					MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
					break;
				}
			}
			else if (type == 0x2)
			{
				ModPairArray* mods = static_cast<ModPairArray*>(*itData);
				bool found = false;
				for (std::vector<ModAssociationData::ModPair>::iterator itMod = mods->modPairs.begin(); itMod != mods->modPairs.end(); itMod++)
				{
					if (Utilities::HasObjectMod(modData, itMod->parentMod))
					{
						if (mods->flags & ModAssociationData::bRequireLooseMod)
						{
							TESObjectMISC* looseMod = Utilities::GetLooseMod(itMod->functionMod);
							if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
								continue;
						}
						found = true;
						GFxValue modArg;
						menuRoot->CreateObject(&modArg);
						RegisterString(&modArg, menuRoot, "modName", itMod->functionMod->fullName.name.c_str());
						RegisterObject(&modArg, menuRoot, "modObj", itMod->functionMod);
						RegisterBool(&modArg, menuRoot, "isEquipped", Utilities::HasObjectMod(modData, itMod->functionMod));
						modPtrs.PushBack(&modArg);
					}
				}
				if (found)
				{
					RegisterBool(&bNull, menuRoot, "bNullMod", mods->flags & ModAssociationData::bCanHaveNullMod);
					MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
					MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
					MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
					break;
				}
			}
		}
		if (bNull.type != GFxValue::kType_Bool)
		{
			MSF_MainData::switchData.ClearData();
			return false;
		}
		if (modPtrs.GetArraySize() == 0 || (!bNull.data.boolean && modPtrs.GetArraySize() < 2))
		{
			MSF_MainData::switchData.ClearData();
			return false;
		}
		
		args[0].SetMember("modPtrs", &modPtrs);
		args[1].SetMember("canHaveNullMod", &modPtrs);
		//std::string openPath = "root." + selectMenu->scaleformID + "_loader.content.open";
		//MSF_MainData::switchData.openedMenu = selectMenu;
		MSFModMenu::OpenMenu();
		std::string openPath = "root.ReceiveModData";
		if (menuRoot->Invoke(openPath.c_str(), nullptr, args, 2))
			return true;
		MSF_MainData::switchData.openedMenu = nullptr;
		MSF_MainData::switchData.ClearData();
		return false;
	}

	//IMenu * GetMenuByMovie(GFxMovieView * movie)
	//{
	//	if (!movie) {
	//		return nullptr;
	//	}

	//	BSReadLocker locker(g_menuTableLock);

	//	IMenu * menu = nullptr;
	//	menuTable.ForEach([movie, &menu](MenuTableItem * item)
	//	{
	//		IMenu * itemMenu = item->menuInstance;
	//		if (itemMenu) {
	//			GFxMovieView * view = itemMenu->movie;
	//			if (view) {
	//				if (movie == view) {
	//					menu = itemMenu;
	//					return false;
	//				}
	//			}
	//		}
	//		return true;
	//	});

	//	return menu;
	//}

	void RegisterMCMScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot)
	{
		//RegisterFunction<OnMCMOpen>(codeObj, movieRoot, "OnMCMOpen");
		RegisterFunction<OnMCMClosed>(codeObj, movieRoot, "OnMCMClose");

		//RegisterFunction<SetModSettingInt>(codeObj, movieRoot, "SetModSettingInt");
		//RegisterFunction<SetModSettingBool>(codeObj, movieRoot, "SetModSettingBool");
		//RegisterFunction<SetModSettingFloat>(codeObj, movieRoot, "SetModSettingFloat");
		//RegisterFunction<SetModSettingString>(codeObj, movieRoot, "SetModSettingString");

		//RegisterFunction<SetKeybind>(codeObj, movieRoot, "SetKeybind");
		//RegisterFunction<ClearKeybind>(codeObj, movieRoot, "ClearKeybind"); //clear keybinds
		//RegisterFunction<RemapKeybind>(codeObj, movieRoot, "RemapKeybind");

	}

	void RegisterMSFScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot)
	{
		RegisterFunction<SendF4SEVersion>(codeObj, movieRoot, "GetF4SEVersion");
		RegisterFunction<SendGameVersion>(codeObj, movieRoot, "GetGameVersion");
		RegisterFunction<SendMSFVersion>(codeObj, movieRoot, "GetMSFVersion");
		RegisterFunction<SendWidgetUpdate>(codeObj, movieRoot, "RequestWidgetUpdate");
		RegisterFunction<SendWidgetSettings>(codeObj, movieRoot, "RequestWidgetSettings");
		RegisterFunction<ReceiveWidgetProperties>(codeObj, movieRoot, "SendInterfaceProperties");
		RegisterFunction<MenuClosedEventSink>(codeObj, movieRoot, "MenuClosedEventDispatcher");
		RegisterFunction<MenuOpenedEventSink>(codeObj, movieRoot, "MenuOpenedEventDispatcher");
		RegisterFunction<ReceiveSelectedAmmo>(codeObj, movieRoot, "SendBackSelectedAmmo");
		RegisterFunction<ReceiveSelectedMod>(codeObj, movieRoot, "SendBackSelectedMod");
	}

	bool RegisterScaleformCallback(GFxMovieView * view, GFxValue * f4se_root)
	{
		GFxMovieRoot* movieRoot = view->movieRoot;

		GFxValue currentSWFPath;
		const char* currentSWFPathString = nullptr;

		if (movieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url")) {
			currentSWFPathString = currentSWFPath.GetString();
		}

		// Look for the menu that we want to inject into.
		if (strcmp(currentSWFPathString, "Interface/HUDMenu.swf") == 0) {
			GFxValue root; movieRoot->GetVariable(&root, "root");
			MSF_MainData::MSFMenuRoot = movieRoot;

			// Register native code object
			GFxValue msf; movieRoot->CreateObject(&msf);
			root.SetMember("msf", &msf);
			RegisterMSFScaleformFuncs(&msf, movieRoot);
			_MESSAGE("MSF code object created in HUD menu.");

			//for (std::vector<KeybindData>::iterator it = MSF_MainData::keybinds.begin(); it != MSF_MainData::keybinds.end(); it++)
			//{
			//	if (it->selectMenu)
			//	{
			//		// Inject swf
			//		GFxValue loader, urlRequest;
			//		movieRoot->CreateObject(&loader, "flash.display.Loader");
			//		movieRoot->CreateObject(&urlRequest, "flash.net.URLRequest", &GFxValue(it->selectMenu->swfFilename.c_str()), 1);

			//		std::string loadstring = it->selectMenu->scaleformID + "_loader";
			//		root.SetMember(loadstring.c_str(), &loader);
			//		bool injectionSuccess = movieRoot->Invoke("root.msf_loader.load", nullptr, &urlRequest, 1);

			//		bool addSuccess = movieRoot->Invoke("root.addChild", nullptr, &loader, 1);

			//		if (!injectionSuccess || !addSuccess) {
			//			_MESSAGE("WARNING: MSF Scaleform injection failed for %s.", it->selectMenu->scaleformID.c_str());
			//		}
			//	}
			//}

			// Inject swf
			GFxValue loader, urlRequest;
			movieRoot->CreateObject(&loader, "flash.display.Loader");
			movieRoot->CreateObject(&urlRequest, "flash.net.URLRequest", &GFxValue("MSFwidget.swf"), 1);

			root.SetMember("msf_loader", &loader);
			bool injectionSuccess = movieRoot->Invoke("root.msf_loader.load", nullptr, &urlRequest, 1);

			bool addSuccess = movieRoot->Invoke("root.addChild", nullptr, &loader, 1);

			if (!injectionSuccess || !addSuccess) {
				_MESSAGE("WARNING: MSF Scaleform injection failed.");
			}


			/*GFxValue msf_loader; root.GetMember("msf_loader", &msf_loader);
			GFxValue msfmsf; root.GetMember("msf", &msfmsf);
			_MESSAGE("msf: %02X, msf_loader: %02X, content: %02X", root.HasMember("msf"), root.HasMember("msf_loader"), msf_loader.HasMember("content"));
			GFxValue vis, ver, verfn;
			movieRoot->GetVariable(&vis, "root.msf_loader.content.visible");
			movieRoot->GetVariable(&ver, "root.msf_loader.content.InterfaceVersion");
			movieRoot->Invoke("root.msf_loader.content.SendInterfaceVersion", &verfn, nullptr, 0);
			_MESSAGE("vis type: %08X, ver type: %08X, verfn type: %08X", vis.GetType(), ver.GetType(), verfn.GetType());
			//GFxValue msf_loader_content; msf_loader.GetMember("content", &msf_loader_content);
			//_MESSAGE("UpdateWidgetData: %02X, InterfaceVersion: %02X, MSFwidget: %02X", msf_loader_content.HasMember("UpdateWidgetData"), msf_loader_content.HasMember("InterfaceVersion"), msf_loader_content.HasMember("MSFwidget"));
			*/
		}

		return true;
	}

	bool RegisterMCMCallback()
	{
		BSFixedString mainMenuStr("PauseMenu");
		IMenu* menu = (*g_ui)->GetMenu(mainMenuStr);
		GFxMovieRoot* movieRoot = menu->movie->movieRoot;
		GFxValue currentSWFPath;
		const char* currentSWFPathString = nullptr;

		if (movieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url")) {
			currentSWFPathString = currentSWFPath.GetString();
		}
		else {
			_MESSAGE("WARNING: MCM callback registration failed.");
		}

		// Look for the menu that we want to inject into.
		if (strcmp(currentSWFPathString, "Interface/MainMenu.swf") == 0) {
			GFxValue root; movieRoot->GetVariable(&root, "root");
			GFxValue mcm;
			root.GetMember("mcm", &mcm);
			if (!mcm.data.obj)
			{
				_MESSAGE("WARNING: MCM callback registration failed.");
				return false;
			}
			RegisterMCMScaleformFuncs(&mcm, movieRoot);
		}

		return true;
	}
}