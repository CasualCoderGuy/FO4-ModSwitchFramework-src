#include "MSF_Scaleform.h"
#include "MSF_Test.h"

GFxMovieRoot* msfRoot;

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
				MSF_MainData::tmr.start();
				//_MESSAGE("fire input");
			}
		}
			break;
#ifdef DEBUG
		case 103:
		{
			if ((*g_ui)->numPauseGame == 0)
			{
				MSF_Test::ListModTable(*g_player);
				//MSF_Test::PrintStoredData();
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
				if (keyFn->type & KeybindData::bIsAmmo)
				{
					if (keyFn->type & KeybindData::bHUDselection)
						MSF_Scaleform::ToggleAmmoMenu(keyFn->menuScriptPath);
					else
						MSF_Base::SwitchAmmoHotkey(keyFn->type & 0x0F);
				}
				else if (keyFn->type & KeybindData::bHUDselection)
					MSF_Scaleform::ToggleModMenu(keyFn->menuScriptPath, &keyFn->modAssociations);
				else if (keyFn->type & KeybindData::bToggle)
					MSF_Base::ToggleModHotkey(&keyFn->modAssociations);
				else
					MSF_Base::SwitchModHotkey(keyFn->type & 0x0F, &keyFn->modAssociations);
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

	// SendBackSelectedAmmo(ammo:Pointer):Boolean;
	class ReceiveSelectedAmmo : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
			args->result->SetBool(false);

			if (args->numArgs != 1) return;
			MSF_MainData::switchData.openedMenu = "";
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
			MSF_MainData::switchData.openedMenu = "";
			MSF_Base::SwitchToSelectedMod(args->args[0].data.obj, args->args[1].data.obj);

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

	bool UpdateWidget(UInt8 flags)
	{
		Actor* playerActor = *g_player;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		if (!msfRoot)
			return false;
		GFxValue arrArgs[6];
		if (!instanceData)
		{
			arrArgs[0].SetString("");
			arrArgs[1].SetString("");
			arrArgs[2].SetString("");
			arrArgs[3].SetString("");
			arrArgs[4].SetInt(0); //muzzle
			arrArgs[5].SetInt(0); //ammo id
			
		}
		else if (flags == 125)
		{
			arrArgs[0].SetString("N/A");
			arrArgs[1].SetString("N/A");
			arrArgs[2].SetString("N/A");
			if (!instanceData->ammo || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowAmmoName))
				msfRoot->CreateString(&arrArgs[3], "");
			else
				msfRoot->CreateString(&arrArgs[3], instanceData->ammo->fullName.name.c_str());
			arrArgs[4].SetInt(-1); //muzzle
			arrArgs[5].SetInt(0); //ammo id
		}
		else if (flags == 124)
		{
			arrArgs[0].SetString("N/A");
			arrArgs[1].SetString("N/A");
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowFiringMode))
				msfRoot->CreateString(&arrArgs[2], "");
			else
				msfRoot->CreateString(&arrArgs[2], MSF_Data::GetFMString(instanceData).c_str());
			arrArgs[3].SetString("N/A");
			arrArgs[4].SetInt(-1); //muzzle
			arrArgs[5].SetInt(-1); //ammo id
		}
		else if (flags == 123)
		{
			arrArgs[0].SetString("N/A");
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowScopeData))
				msfRoot->CreateString(&arrArgs[1], "");
			else
				msfRoot->CreateString(&arrArgs[1], MSF_Data::GetScopeString(instanceData).c_str());
			arrArgs[2].SetString("N/A");
			arrArgs[3].SetString("N/A");
			arrArgs[4].SetInt(-1); //muzzle
			arrArgs[5].SetInt(-1); //ammo id
		}
		else if (flags == 122)
		{
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowMuzzleName))
				msfRoot->CreateString(&arrArgs[0], "");
			else
				msfRoot->CreateString(&arrArgs[0], MSF_Data::GetMuzzleString(instanceData).c_str());
			arrArgs[1].SetString("N/A");
			arrArgs[2].SetString("N/A");
			arrArgs[3].SetString("N/A");
			arrArgs[4].SetInt(0); //muzzle
			arrArgs[5].SetInt(-1); //ammo id
		}
		else if (flags == 126 || flags == 0)
		{
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowMuzzleName))
				msfRoot->CreateString(&arrArgs[0], "");
			else
				msfRoot->CreateString(&arrArgs[0], MSF_Data::GetMuzzleString(instanceData).c_str());
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowScopeData))
				msfRoot->CreateString(&arrArgs[1], "");
			else
				msfRoot->CreateString(&arrArgs[1], MSF_Data::GetScopeString(instanceData).c_str());
			if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowFiringMode))
				msfRoot->CreateString(&arrArgs[2], "");
			else
				msfRoot->CreateString(&arrArgs[2], MSF_Data::GetFMString(instanceData).c_str());
			if (!instanceData->ammo || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bShowAmmoName))
				msfRoot->CreateString(&arrArgs[3], "");
			else
				msfRoot->CreateString(&arrArgs[3], instanceData->ammo->fullName.name.c_str());
			arrArgs[4].SetInt(0); //muzzle
			arrArgs[5].SetInt(0); //ammo id
		}
		msfRoot->Invoke("root.msf_loader.widget.updateWidget", nullptr, arrArgs, 6);
		return true;
	}

	bool ToggleAmmoMenu(std::string path)
	{
		if (!msfRoot || path == "")
			return false;
		//check conditions

		if (MSF_MainData::switchData.openedMenu == path)
		{
			std::string closePath = path + ".close";
			GFxValue result;
			msfRoot->Invoke(closePath.c_str(), &result, nullptr, 0); //"root.msf_loader.ammoMenu.close"
			if (result.GetBool())
			{
				MSF_MainData::switchData.openedMenu = "";
				return true;
			}
			return false;
		}
		else if (MSF_MainData::switchData.openedMenu != "")
			return false;

		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		if (!MSF_Base::InitWeapon())
			return false;

		GFxValue args[1];
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(*g_player, 41);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
		GFxValue ammoData;
		msfRoot->CreateArray(&ammoData);
		if (baseAmmo)
		{
			TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(*g_player);
			for (std::vector<AmmoData>::iterator itAmmoData = MSF_MainData::ammoData.begin(); itAmmoData != MSF_MainData::ammoData.end(); itAmmoData++)
			{
				if (itAmmoData->baseAmmoData.ammo == baseAmmo)
				{
					GFxValue BammoArg;
					msfRoot->CreateObject(&BammoArg);
					RegisterString(&BammoArg, msfRoot, "ammoName", baseAmmo->fullName.name.c_str());
					RegisterObject(&BammoArg, msfRoot, "ammoObj", &itAmmoData->baseAmmoData);
					RegisterBool(&BammoArg, msfRoot, "isEquipped", instanceData->ammo == baseAmmo);
					ammoData.PushBack(&BammoArg);
					for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
					{
						for (UInt32 i1 = 0; i1 < (*g_player)->inventoryList->items.count; i1++)
						{
							BGSInventoryItem inventoryItem;
							(*g_player)->inventoryList->items.GetNthItem(i1, inventoryItem);
							TESAmmo* ammo = (TESAmmo*)inventoryItem.form;
							if (ammo && itAmmoMod->ammo == ammo)
							{
								GFxValue ammoArg;
								msfRoot->CreateObject(&ammoArg);
								RegisterString(&ammoArg, msfRoot, "ammoName", ammo->fullName.name.c_str());
								RegisterObject(&ammoArg, msfRoot, "ammoObj", itAmmoMod._Ptr);
								RegisterBool(&ammoArg, msfRoot, "isEquipped", instanceData->ammo == ammo);
								RegisterInt(&ammoArg, msfRoot, "ammoCount", inventoryItem.stack->count);
								ammoData.PushBack(&ammoArg);
								break;
							}
						}
					}
					break;
				}
			}
		}
		if (!baseAmmo || ammoData.GetArraySize() < 2)
			return false;

		args[0].SetMember("ammoData", &ammoData);
		std::string openPath = path + ".open";
		MSF_MainData::switchData.openedMenu = path;
		if (msfRoot->Invoke(openPath.c_str(), nullptr, args, 1))
			return true;
		MSF_MainData::switchData.openedMenu = "";
		return false;
	}

	bool ToggleModMenu(std::string path, std::vector<ModAssociationData*>* modAssociations)
	{
		if (!modAssociations || path == "")
			return false;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(*g_player, 41);
		if (!modData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;

		if (!msfRoot)
			return false;
		//check conditions
		
		if (MSF_MainData::switchData.openedMenu == path)
		{
			std::string closePath = path + ".close";
			GFxValue result;
			msfRoot->Invoke(closePath.c_str(), &result, nullptr, 0);
			if (result.GetBool())
			{
				MSF_MainData::switchData.openedMenu = "";
				return true;
			}
			return false;
		}
		else if (MSF_MainData::switchData.openedMenu != "")
			return false;

		if (MSF_MainData::switchData.SwitchFlags & (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bReloadNotFinished | SwitchData::bAnimNotFinished))
			return false;
		MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bSetLooseMods;

		GFxValue args[2];
		GFxValue modPtrs;
		msfRoot->CreateArray(&modPtrs);
		GFxValue bNull;
		msfRoot->CreateObject(&bNull);
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
					msfRoot->CreateObject(&modArg);
					RegisterString(&modArg, msfRoot, "modName", modPair->modPair.functionMod->fullName.name.c_str());
					RegisterObject(&modArg, msfRoot, "modObj", modPair->modPair.functionMod);
					RegisterBool(&modArg, msfRoot, "isEquipped", Utilities::HasObjectMod(modData, modPair->modPair.functionMod));
					modPtrs.PushBack(&modArg);
					RegisterBool(&bNull, msfRoot, "bNullMod", true);
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
						msfRoot->CreateObject(&modArg);
						RegisterString(&modArg, msfRoot, "modName", (*itMod)->fullName.name.c_str());
						RegisterObject(&modArg, msfRoot, "modObj", *itMod);
						RegisterBool(&modArg, msfRoot, "isEquipped", Utilities::HasObjectMod(modData, (*itMod)));
						modPtrs.PushBack(&modArg);
					}
					RegisterBool(&bNull, msfRoot, "bNullMod", mods->flags & ModAssociationData::bCanHaveNullMod);
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
						msfRoot->CreateObject(&modArg);
						RegisterString(&modArg, msfRoot, "modName", itMod->functionMod->fullName.name.c_str());
						RegisterObject(&modArg, msfRoot, "modObj", itMod->functionMod);
						RegisterBool(&modArg, msfRoot, "isEquipped", Utilities::HasObjectMod(modData, itMod->functionMod));
						modPtrs.PushBack(&modArg);
					}
				}
				if (found)
				{
					RegisterBool(&bNull, msfRoot, "bNullMod", mods->flags & ModAssociationData::bCanHaveNullMod);
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
		std::string openPath = path + ".open";
		MSF_MainData::switchData.openedMenu = path;
		if (msfRoot->Invoke(openPath.c_str(), nullptr, args, 2))
			return true;
		MSF_MainData::switchData.openedMenu = "";
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
		//getversion
		RegisterFunction<ReceiveSelectedAmmo>(codeObj, movieRoot, "SendBackSelectedAmmo");
		RegisterFunction<ReceiveSelectedMod>(codeObj, movieRoot, "SendBackSelectedMod");
	}

	bool RegisterMSFScaleform(GFxMovieView * view, GFxValue * f4se_root)
	{
		msfRoot = view->movieRoot;

		GFxValue currentSWFPath;
		const char* currentSWFPathString = nullptr;

		if (msfRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url")) {
			currentSWFPathString = currentSWFPath.GetString();
		}
		else {
			_MESSAGE("WARNING: MSF Scaleform registration failed.");
		}

		// Look for the menu that we want to inject into.
		if (strcmp(currentSWFPathString, "Interface/HUDMenu.swf") == 0) {
			GFxValue root; msfRoot->GetVariable(&root, "root");

			//for
			// Register native code object
			GFxValue msf; msfRoot->CreateObject(&msf);
			root.SetMember("msf", &msf);
			RegisterMSFScaleformFuncs(&msf, msfRoot);

			// Inject MCM menu
			GFxValue loader, urlRequest;
			msfRoot->CreateObject(&loader, "flash.display.Loader");
			msfRoot->CreateObject(&urlRequest, "flash.net.URLRequest", &GFxValue("anagy_ModSwitchFramework.swf"), 1);

			root.SetMember("msf_loader", &loader);
			bool injectionSuccess = msfRoot->Invoke("root.msf_loader.load", nullptr, &urlRequest, 1);

			msfRoot->Invoke("root.Menu_mc.addChild", nullptr, &loader, 1);

			if (!injectionSuccess) {
				_MESSAGE("WARNING: MSF Scaleform injection failed.");
			}
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