#include "MSF_Data.h"
#include "MSF_Scaleform.h"
#include "MSF_Events.h"
#include "MSF_WeaponState.h"
#include "json/json.h"
#include "RNG.h"
#include <fstream>

std::unordered_map<TESAmmo*, AmmoData*> MSF_MainData::ammoDataMap;
std::unordered_map<TESObjectWEAP*, AnimationData*> MSF_MainData::reloadAnimDataMap;
std::unordered_map<TESObjectWEAP*, AnimationData*> MSF_MainData::fireAnimDataMap;

std::unordered_map<UInt64, KeybindData*> MSF_MainData::keybindMap;
std::unordered_map<std::string, KeybindData*> MSF_MainData::keybindIDMap;
std::unordered_map<std::string, float> MSF_MainData::MCMfloatSettingMap;

std::vector<HUDFiringModeData> MSF_MainData::fmDisplayData;
std::vector<HUDScopeData> MSF_MainData::scopeDisplayData;
std::vector<HUDMuzzleData> MSF_MainData::muzzleDisplayData;

BGSKeyword* MSF_MainData::baseModCompatibilityKW;
BGSKeyword* MSF_MainData::hasSwitchedAmmoKW;
BGSKeyword* MSF_MainData::hasUniqueStateKW;
BGSMod::Attachment::Mod* MSF_MainData::APbaseMod;
BGSMod::Attachment::Mod* MSF_MainData::NullMuzzleMod;
BGSKeyword* MSF_MainData::CanHaveNullMuzzleKW;
BGSKeyword* MSF_MainData::FiringModeUnderbarrelKW;
TESIdleForm* MSF_MainData::reloadIdle1stP;
TESIdleForm* MSF_MainData::reloadIdle3rdP;
TESIdleForm* MSF_MainData::fireIdle1stP;
TESIdleForm* MSF_MainData::fireIdle3rdP;
BGSAction* MSF_MainData::ActionFireSingle;
BGSAction* MSF_MainData::ActionFireAuto;
BGSAction* MSF_MainData::ActionReload;
BGSAction* MSF_MainData::ActionDraw;
BGSAction* MSF_MainData::ActionGunDown;

bool MSF_MainData::IsInitialized = false;
int MSF_MainData::iCheckDelayMS = 10;
UInt16 MSF_MainData::MCMSettingFlags = 0x000;
GFxMovieRoot* MSF_MainData::MSFMenuRoot = nullptr;
ModSelectionMenu* MSF_MainData::widgetMenu;

std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*> MSF_MainData::compatibilityEdits;
std::unordered_multimap<BGSMod::Attachment::Mod*, KeywordValue> MSF_MainData::instantiationRequirements;
ModSwitchManager MSF_MainData::modSwitchManager;
PlayerInventoryListEventSink MSF_MainData::playerInventoryEventSink;
WeaponStateStore MSF_MainData::weaponStateStore;
KeywordValue MSF_MainData::ammoAP = 0;
UInt64 MSF_MainData::lowerWeaponHotkey = 0;
UInt64 MSF_MainData::cancelSwitchHotkey = 0;
std::unordered_map<BGSMod::Attachment::Mod*, BurstModeData*>  MSF_MainData::burstModeData;
Utilities::Timer MSF_MainData::tmr;

RandomNumber MSF_MainData::rng;

namespace MSF_Data
{
	bool InitData()
	{
		UInt32 formIDbase = 0;
		UInt8 espModIndex = (*g_dataHandler)->GetLoadedModIndex(MODNAME);
		//if (espModIndex == (UInt8)-1)
		//	return false;
		if (espModIndex != 0xFF)
		{
			formIDbase = ((UInt32)espModIndex) << 24;
		}
		else
		{
			UInt16 eslModIndex = (*g_dataHandler)->GetLoadedLightModIndex(MODNAME);
			if (eslModIndex != 0xFFFF)
			{
				formIDbase = 0xFE000000 | (UInt32(eslModIndex) << 12);
			}
			else
				return false;
		}
		MSF_MainData::hasSwitchedAmmoKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x000002F);
		MSF_MainData::APbaseMod = (BGSMod::Attachment::Mod*)LookupFormByID(formIDbase | (UInt32)0x0000065);
		if (MSF_MainData::baseModCompatibilityKW)
		{
			UInt16 attachParent = Utilities::GetAttachValueForTypedKeyword(MSF_MainData::baseModCompatibilityKW);
			if (attachParent >= 0)
				MSF_MainData::APbaseMod->unkC0 = attachParent;
		}
		BGSKeyword* ammoApkeyword = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x000000B);
		MSF_MainData::ammoAP = Utilities::GetAttachValueForTypedKeyword(ammoApkeyword);
		MSF_MainData::NullMuzzleMod = (BGSMod::Attachment::Mod*)LookupFormByID((UInt32)0x004F21D);
		MSF_MainData::CanHaveNullMuzzleKW = (BGSKeyword*)LookupFormByID((UInt32)0x01C9E78);
		MSF_MainData::FiringModeUnderbarrelKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000021);
		MSF_MainData::hasUniqueStateKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000001);
		MSF_MainData::fireIdle1stP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0004AE1));
		MSF_MainData::fireIdle3rdP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0018E1F));
		MSF_MainData::reloadIdle1stP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0004D33));
		MSF_MainData::reloadIdle3rdP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x00BDDA6));
		MSF_MainData::ActionFireSingle = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A5A));
		MSF_MainData::ActionFireAuto = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A5C));
		MSF_MainData::ActionReload = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A56));
		MSF_MainData::ActionDraw = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x00132AF));
		MSF_MainData::ActionGunDown = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0022A35));
		
		return true;
	}

	bool InitCompatibility()
	{
		for (std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*>::iterator itComp = MSF_MainData::compatibilityEdits.begin(); itComp != MSF_MainData::compatibilityEdits.end(); itComp++)
		{
			BGSMod::Attachment::Mod* mod = itComp->first;
			if (itComp->second->attachParent > 0)
				mod->unkC0 = itComp->second->attachParent;
			KeywordValueArray* instantiationData = reinterpret_cast<KeywordValueArray*>(&mod->unkB0);
			for (std::vector<KeywordValue>::iterator itValue = itComp->second->removedFilters.begin(); itValue != itComp->second->removedFilters.end(); itValue++)
			{
				SInt64 idx = instantiationData->GetItemIndex(*itValue);
				if (idx >= 0)
					instantiationData->Remove(idx);
			}
			for (std::vector<KeywordValue>::iterator itValue = itComp->second->addedFilters.begin(); itValue != itComp->second->addedFilters.end(); itValue++)
				instantiationData->Push(*itValue);
			AttachParentArray* attachParentArray = reinterpret_cast<AttachParentArray*>(&mod->unk98);
			for (std::vector<KeywordValue>::iterator itValue = itComp->second->removedAPslots.begin(); itValue != itComp->second->removedAPslots.end(); itValue++)
			{
				SInt64 idx = attachParentArray->kewordValueArray.GetItemIndex(*itValue);
				if (idx >= 0)
					attachParentArray->kewordValueArray.Remove(idx);
			}
			for (std::vector<KeywordValue>::iterator itValue = itComp->second->addedAPslots.begin(); itValue != itComp->second->addedAPslots.end(); itValue++)
				attachParentArray->kewordValueArray.Push(*itValue);
		}
		return true;
	}

	bool InitMCMSettings()
	{
		if (!ReadMCMKeybindData())
			return false;

		MSF_MainData::MCMSettingFlags = 0x1DFF;
		AddFloatSetting("fSliderMainX", 950.0);
		AddFloatSetting("fSliderMainY", 565.0);
		AddFloatSetting("fPowerArmorOffsetX", 0.0);
		AddFloatSetting("fPowerArmorOffsetY", 0.0);
		AddFloatSetting("fSliderAmmoIconX", 0.0);
		AddFloatSetting("fSliderAmmoIconY", 0.0);
		AddFloatSetting("fSliderMuzzleIconX", 0.0);
		AddFloatSetting("fSliderMuzzleIconY", 0.0);
		AddFloatSetting("fSliderAmmoNameX", 0.0);
		AddFloatSetting("fSliderAmmoNameY", 0.0);
		AddFloatSetting("fSliderMuzzleNameX", 0.0);
		AddFloatSetting("fSliderMuzzleNameY", 0.0);
		AddFloatSetting("fSliderFiringModeX", 0.0);
		AddFloatSetting("fSliderFiringModeY", 0.0);
		AddFloatSetting("fSliderScopeDataX", 0.0);
		AddFloatSetting("fSliderScopeDataY", 0.0);

		if (ReadUserSettings())
			_MESSAGE("User settings loaded for MSF");
		return true;
	}

	void AddFloatSetting(std::string name, float value)
	{
		MSF_MainData::MCMfloatSettingMap[name] = value;
	}

	bool ReadMCMKeybindData()
	{
		try
		{
			std::string filePath = "Data\\MCM\\Settings\\Keybinds.json";
			std::ifstream file(filePath);
			if (file.is_open())
			{
				Json::Value json, keybinds;
				Json::Reader reader;
				reader.parse(file, json);

				if (json["version"].asInt() < 1)
					return false;

				keybinds = json["keybinds"];
				if (!keybinds.isArray())
					return false;

				for (int i = 0; i < keybinds.size(); i++)
				{
					const Json::Value& keybind = keybinds[i];

					std::string id = keybind["id"].asString();
					if (id == "")
						continue;
					else if (id == "MSF_CANCEL")
					{
						UInt64 keyCode = keybind["keycode"].asInt();
						UInt64 modifiers = keybind["modifiers"].asInt();
						UInt64 key = (modifiers << 32) + keyCode;
						if (key)
							MSF_MainData::cancelSwitchHotkey = key;
					}
					else if (id == "MSF_LOWER")
					{
						UInt64 keyCode = keybind["keycode"].asInt();
						UInt64 modifiers = keybind["modifiers"].asInt();
						UInt64 key = (modifiers << 32) + keyCode;
						if (key)
							MSF_MainData::lowerWeaponHotkey = key;
					}
					else
					{
						auto kbIt = MSF_MainData::keybindIDMap.find(id);
						if (kbIt != MSF_MainData::keybindIDMap.end())
						{
							KeybindData* kb = kbIt->second;
							UInt64 key = ((UInt64)kb->modifiers << 32) + kb->keyCode;
							if (key)
								MSF_MainData::keybindMap.erase(key);
							kb->keyCode = keybind["keycode"].asInt();
							kb->modifiers = keybind["modifiers"].asInt();
							key = ((UInt64)kb->modifiers << 32) + kb->keyCode;
							MSF_MainData::keybindMap[key] = kb;
						}
					}
				}

				return true;
			}
		}
		catch (...)
		{
			_FATALERROR("Fatal Error - MCM Keybind storage deserialization failure");
			return false;
		}
		return false;
	}

	bool ReadKeybindData()
	{
		char* modSettingsDirectory = "Data\\MSF\\Keybinds\\*.json";

		HANDLE hFind;
		WIN32_FIND_DATA data;
		std::vector<WIN32_FIND_DATA> keybindFiles;

		_MESSAGE("Loading MSF keybind data");

		hFind = FindFirstFile(modSettingsDirectory, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				keybindFiles.push_back(data);
			} while (FindNextFile(hFind, &data));
			FindClose(hFind);
			
		}
		else
			return false;

		for (int i = 0; i < keybindFiles.size(); i++) {
			std::string jsonLocation = "./Data/MSF/Keybinds/";
			jsonLocation += keybindFiles[i].cFileName;

			std::string modName(keybindFiles[i].cFileName);
			modName = modName.substr(0, modName.find_last_of('.'));

			try
			{
				std::ifstream file(jsonLocation);
				if (file.is_open())
				{
					Json::Value json, keybinds, data2;
					Json::Reader reader;
					reader.parse(file, json);

					if (json["version"].asInt() < MIN_SUPPORTED_KB_VERSION)
					{
						_ERROR("Unsupported keybind version: %s", keybindFiles[i].cFileName);
						continue;
					}

					keybinds = json["keybinds"];
					if (!keybinds.isArray())
						return false;

					for (int i = 0; i < keybinds.size(); i++)
					{
						const Json::Value& keybind = keybinds[i];

						std::string id = keybind["id"].asString();
						if (id == "")
							continue;

						auto kbIt = MSF_MainData::keybindIDMap.find(id);
						if (kbIt != MSF_MainData::keybindIDMap.end())
						{
							KeybindData* kb = kbIt->second;
							std::string menuName = keybind["menuName"].asString();
							UInt8 flags = keybind["keyfunction"].asInt();
							if ((menuName == "") && (flags & KeybindData::bHUDselection))
								break;
							if (flags & KeybindData::bHUDselection)
							{
								UInt8 type = 0;
								if (flags & KeybindData::bGlobalMenu)
									type = ModSelectionMenu::kType_All;
								else if (flags & KeybindData::bIsAmmo)
									type = ModSelectionMenu::kType_AmmoMenu;
								else
									type = ModSelectionMenu::kType_ModMenu;
								if (kb->selectMenu)
								{
									kb->selectMenu->scaleformName = menuName;
									kb->selectMenu->type = type;
								}
								else
									kb->selectMenu = new ModSelectionMenu(menuName, type);
							}
							else
							{
								kb->selectMenu = nullptr;
								kb->type = flags;
							}
						}
						else
						{
							std::string menuName = keybind["menuName"].asString();
							UInt8 flags = keybind["keyfunction"].asInt();
							if (menuName == "" && (flags & KeybindData::bHUDselection))
								continue;
							KeybindData* kb = new KeybindData();
							kb->functionID = id;
							kb->type = flags;
							kb->keyCode = 0;
							kb->modifiers = 0;
							if (flags & KeybindData::bHUDselection)
							{
								UInt8 type = 0;
								if (flags & KeybindData::bGlobalMenu)
									type = ModSelectionMenu::kType_All;
								else if (flags & KeybindData::bIsAmmo)
									type = ModSelectionMenu::kType_AmmoMenu;
								else
									type = ModSelectionMenu::kType_ModMenu;
								kb->selectMenu = new ModSelectionMenu(menuName, type);
							}
							else
								kb->selectMenu = nullptr;
							kb->modData = nullptr;
							MSF_MainData::keybindIDMap[id] = kb;
						}
					}
				}
			}
			catch (...)
			{
				_FATALERROR("Fatal Error - MSF Keybind data deserialization failure");
				return false;
			}
		}
		return true;
	}

	bool ReadUserSettings() 
	{

		char* modSettingsDirectory = "Data\\MCM\\Settings\\ModSwitchFramework.ini";

		HANDLE hFind;
		WIN32_FIND_DATA data;

		hFind = FindFirstFile(modSettingsDirectory, &data);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			_MESSAGE("No user settings found for MSF");
			return false;
		}
		FindClose(hFind);

		// Extract all sections
		std::vector<std::string> sections;
		LPTSTR lpszReturnBuffer = new TCHAR[1024];
		std::string iniLocation = "./Data/MCM/Settings/ModSwitchFramework.ini";
		DWORD sizeWritten = GetPrivateProfileSectionNames(lpszReturnBuffer, 1024, iniLocation.c_str());
		if (sizeWritten == (1024 - 2)) {
			_WARNING("Warning: Too many sections. Settings will not be read.");
			delete lpszReturnBuffer;
			return false;
		}

		for (LPTSTR p = lpszReturnBuffer; *p; p++) {
			std::string sectionName(p);
			sections.push_back(sectionName);
			p += strlen(p);
			ASSERT(*p == 0);
		}

		delete lpszReturnBuffer;

		for (int j = 0; j < sections.size(); j++) {
			// Extract all keys within section
			int len = 1024;
			LPTSTR lpReturnedString = new TCHAR[len];
			DWORD sizeWritten = GetPrivateProfileSection(sections[j].c_str(), lpReturnedString, len, iniLocation.c_str());
			while (sizeWritten == (len - 2)) {
				// Buffer too small to contain all entries; expand buffer and try again.
				delete lpReturnedString;
				len <<= 1;
				lpReturnedString = new TCHAR[len];
				sizeWritten = GetPrivateProfileSection(sections[j].c_str(), lpReturnedString, len, iniLocation.c_str());
				//_MESSAGE("Expanded buffer to %d bytes.", len);
			}
			
			for (LPTSTR p = lpReturnedString; *p; p++) {
				std::string valuePair(p);
				auto delimiter = valuePair.find_first_of('=');
				std::string settingName = valuePair.substr(0, delimiter);
				std::string settingValue = valuePair.substr(delimiter + 1);
				SetUserModifiedValue(sections[j], settingName, settingValue);
				p += strlen(p);
				ASSERT(*p == 0);
			}

			delete lpReturnedString;
		}
		return true;
	}

	bool SetUserModifiedValue(std::string section, std::string settingName, std::string settingValue)
	{
		if (section == "Gameplay" || section == "Display")
		{
			//auto delimiter = settingName.find_first_of('_');
			//if (delimiter == -1)
			//	return false;
			//std::string flagStr = settingName.substr(0, delimiter);
			//if (flagStr == "")
			//	return false;
			//UInt16 flagType = std::stoi(flagStr);
			//if (flagType == 0)
			//	return false;
			//bool flagValue = settingValue != "0";
			//_MESSAGE("Setting read: %04X, %02X", flagType, flagValue);
			//if (flagValue)
			//	MSF_MainData::MCMSettingFlags |= (1 << flagType);
			//else
			//	MSF_MainData::MCMSettingFlags &= ~(1 << flagType);
			//return true;
			bool flagValue = settingValue != "0";
			UInt16 flag = 0;
			if (settingName == "bWidgetAlwaysVisible")
				flag = MSF_MainData::bWidgetAlwaysVisible;
			else if (settingName == "bShowAmmoIcon")
				flag = MSF_MainData::bShowAmmoIcon;
			else if (settingName == "bShowMuzzleIcon")
				flag = MSF_MainData::bShowMuzzleIcon;
			else if (settingName == "bShowAmmoName")
				flag = MSF_MainData::bShowAmmoName;
			else if (settingName == "bShowMuzzleName")
				flag = MSF_MainData::bShowMuzzleName;
			else if (settingName == "bShowFiringMode")
				flag = MSF_MainData::bShowFiringMode;
			else if (settingName == "bShowZoomData")
				flag = MSF_MainData::bShowScopeData;
			else if (settingName == "bReloadEnabled")
				flag = MSF_MainData::bReloadEnabled;
			else if (settingName == "bDrawEnabled")
				flag = MSF_MainData::bDrawEnabled;
			else if (settingName == "bCustomAnimEnabled")
				flag = MSF_MainData::bCustomAnimEnabled;
			else if (settingName == "bRequireAmmoToSwitch")
				flag = MSF_MainData::bRequireAmmoToSwitch;
			else if (settingName == "bAmmoRequireWeaponToBeDrawn")
				flag = MSF_MainData::bAmmoRequireWeaponToBeDrawn;
			else if (settingName == "bShowUnavailableMods")
				flag = MSF_MainData::bShowUnavailableMods;

			if (flagValue)
				MSF_MainData::MCMSettingFlags |= flag;
			else
				MSF_MainData::MCMSettingFlags &= ~flag;
			return true;
		}
		else if (section == "Position")
		{
			std::unordered_map<std::string, float>::iterator setting = MSF_MainData::MCMfloatSettingMap.find(settingName);
			if (setting != MSF_MainData::MCMfloatSettingMap.end())
			{
				setting->second = std::stof(settingValue);
				return true;
			}
		}
		return false;
	}

	bool LoadPluginData() 
	{
		char* modSettingsDirectory = "Data\\MSF\\*.json";

		HANDLE hFind;
		WIN32_FIND_DATA data;
		std::vector<WIN32_FIND_DATA> modSettingFiles;

		hFind = FindFirstFile(modSettingsDirectory, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				modSettingFiles.push_back(data);
			} while (FindNextFile(hFind, &data));
			FindClose(hFind);
		}
		else
			return false;

		_MESSAGE("Number of mod setting files: %d", modSettingFiles.size());

		for (int i = 0; i < modSettingFiles.size(); i++) {
			std::string jsonLocation = "./Data/MSF/";
			jsonLocation += modSettingFiles[i].cFileName;

			std::string modName(modSettingFiles[i].cFileName);
			modName = modName.substr(0, modName.find_last_of('.'));

			if (!ReadDataFromJSON(modName, jsonLocation))
				_ERROR("Cannot read data from %s", modSettingFiles[i].cFileName);
		}
		return true;
	}

	bool ReadDataFromJSON(std::string fileName, std::string location)
	{
		try
		{
			std::ifstream file(location);
			if (file.is_open())
			{
				Json::Value json, keydata, data1, data2;
				Json::Reader reader;
				reader.parse(file, json);

				if (json["version"].asInt() < MIN_SUPPORTED_DATA_VERSION)
				{
					_ERROR("Unsupported data version: %s", fileName.c_str());
					return true;
				}

				data1 = json["compatibility"];
				if (data1.isObject())
				{
					std::string apKWstr = data1["baseModAttachPoint"].asString();
					if (apKWstr != "")
					{
						BGSKeyword* apKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(apKWstr), TESForm, BGSKeyword);
						if (!apKW || Utilities::GetAttachValueForTypedKeyword(apKW) < 0)
						{
							if (MSF_MainData::baseModCompatibilityKW)
								_WARNING("Conflict: attach point base mod compatibility keyword already loaded with formID: %s. Ignoring new keyword: %s in file %s", Utilities::GetIdentifierFromForm(MSF_MainData::baseModCompatibilityKW), Utilities::GetIdentifierFromForm(apKW), fileName.c_str());
							else
								MSF_MainData::baseModCompatibilityKW = apKW;
						}
					}
					data2 = data1["attachParent"];
					if (data2.isArray())
					{
						for (int i = 0; i < data2.size(); i++)
						{
							const Json::Value& apData = data2[i];
							std::string str = apData["mod"].asString();
							if (str == "")
								continue;
							BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
							if (!mod)
								continue;
							str = apData["apKeyword"].asString();
							if (str == "")
								continue;
							BGSKeyword* apKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
							KeywordValue value = Utilities::GetAttachValueForTypedKeyword(apKW);
							if (!apKW || value < 0)
								continue;
							std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*>::iterator compIt = MSF_MainData::compatibilityEdits.find(mod);
							if (compIt != MSF_MainData::compatibilityEdits.end() && compIt->second)
							{
								if (compIt->second->attachParent != 0)
								{
									BGSKeyword* kw = nullptr;
									g_AttachPointKeywordArray.GetPtr()->GetNthItem(compIt->second->attachParent, kw);
									_WARNING("Conflict: attach parent already changed for mod with formID: %s to keyword: %s. Ignoring new keyword: %s in file %s",
										Utilities::GetIdentifierFromForm(compIt->first), Utilities::GetIdentifierFromForm(kw), str.c_str(), fileName.c_str());
								}
								else
									compIt->second->attachParent = value;
							}
							else
							{
								ModCompatibilityEdits* compatibility = new ModCompatibilityEdits();
								compatibility->attachParent = value;
								MSF_MainData::compatibilityEdits[mod] = compatibility;
							}
						}
					}
				}

				data2 = data1["attachChildren"];
				if (data2.isArray())
				{
					for (int i = 0; i < data2.size(); i++)
					{
						const Json::Value& apData = data2[i];
						std::string str = apData["mod"].asString();
						if (str == "")
							continue;
						BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
						if (!mod)
							continue;
						str = apData["apKeyword"].asString();
						if (str == "")
							continue;
						bool bRemove = apData["bRemove"].asBool();
						BGSKeyword* apKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
						KeywordValue value = Utilities::GetAttachValueForTypedKeyword(apKW);
						if (!apKW || value < 0)
							continue;
						std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*>::iterator compIt = MSF_MainData::compatibilityEdits.find(mod);
						if (compIt != MSF_MainData::compatibilityEdits.end() && compIt->second)
						{
							std::vector<KeywordValue>::iterator foundEntry = std::find(compIt->second->addedAPslots.begin(), compIt->second->addedAPslots.end(), value);
							const char* removedstr = "added";
							if (foundEntry == compIt->second->addedAPslots.end())
							{
								foundEntry = std::find(compIt->second->removedAPslots.begin(), compIt->second->removedAPslots.end(), value);
								removedstr = "removed";
							}
							if (foundEntry != compIt->second->addedAPslots.end())
							{
								_WARNING("Conflict: attach children for mod with formID: %s already has %s keyword: %s. Ignoring keyword in file %s",
									Utilities::GetIdentifierFromForm(compIt->first), removedstr, str.c_str(), fileName.c_str());
							}
							else if (bRemove)
								compIt->second->removedAPslots.push_back(value);
							else
								compIt->second->addedAPslots.push_back(value);
						}
						else
						{
							ModCompatibilityEdits* compatibility = new ModCompatibilityEdits();
							compatibility->attachParent = 0;
							if (bRemove)
								compatibility->removedAPslots.push_back(value);
							else
								compatibility->addedAPslots.push_back(value);
							MSF_MainData::compatibilityEdits[mod] = compatibility;
						}
					}
				}

				data2 = data1["instantiationKWs"];
				if (data2.isArray())
				{
					for (int i = 0; i < data2.size(); i++)
					{
						const Json::Value& ifData = data2[i];
						std::string str = ifData["mod"].asString();
						if (str == "")
							continue;
						BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
						if (!mod)
							continue;
						str = ifData["ifKeyword"].asString();
						if (str == "")
							continue;
						bool bRemove = ifData["bRemove"].asBool();
						BGSKeyword* ifKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
						KeywordValue value = Utilities::GetInstantiationValueForTypedKeyword(ifKW);
						if (!ifKW || value < 0)
							continue;
						std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*>::iterator compIt = MSF_MainData::compatibilityEdits.find(mod);
						if (compIt != MSF_MainData::compatibilityEdits.end() && compIt->second)
						{
							std::vector<KeywordValue>::iterator foundEntry = std::find(compIt->second->addedFilters.begin(), compIt->second->addedFilters.end(), value);
							const char* removedstr = "added";
							if (foundEntry == compIt->second->addedFilters.end())
							{
								foundEntry = std::find(compIt->second->removedFilters.begin(), compIt->second->removedFilters.end(), value);
								removedstr = "removed";
							}
							if (foundEntry != compIt->second->addedFilters.end())
							{
								_WARNING("Conflict: instantiation filter for mod with formID: %s already has %s keyword: %s. Ignoring keyword in file %s",
									Utilities::GetIdentifierFromForm(compIt->first), removedstr, str.c_str(), fileName.c_str());
							}
							else if (bRemove)
								compIt->second->removedFilters.push_back(value);
							else
								compIt->second->addedFilters.push_back(value);
						}
						else
						{
							ModCompatibilityEdits* compatibility = new ModCompatibilityEdits();
							compatibility->attachParent = 0;
							if (bRemove)
								compatibility->removedFilters.push_back(value);
							else
								compatibility->addedFilters.push_back(value);
							MSF_MainData::compatibilityEdits[mod] = compatibility;
						}
					}
				}

				
				data1 = json["plugins"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						//const Json::Value& dataN = data1[i];
						//std::string pluginName = dataN["pluginName"].asString();
						std::string pluginName = data1[i].asString();
						if (pluginName == "")
							continue;
						UInt8 espModIndex = (*g_dataHandler)->GetLoadedModIndex(pluginName.c_str());
						if (espModIndex == 0xFF)
						{
							UInt16 eslModIndex = (*g_dataHandler)->GetLoadedLightModIndex(pluginName.c_str());
							if (eslModIndex == 0xFFFF)
								return false;
						}
					}
				}

				data1 = json["ammoData"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& ammoData = data1[i];
						std::string str = ammoData["baseAmmo"].asString();
						if (str == "")
							continue;
						TESAmmo* baseAmmo = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESAmmo);
						if (!baseAmmo)
							continue;
						BGSMod::Attachment::Mod* baseMod = nullptr;
						//str = ammoData["baseMod"].asString();
						//if (str != "")
						//	baseMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
						//else
						//	baseMod = MSF_MainData::PlaceholderModAmmo;
						//if (baseMod->priority != 125 || baseMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
						//	continue;
						UInt16 ammoIDbase = ammoData["baseAmmoID"].asInt();
						UInt8 spawnChanceBase = ammoData["spawnChanceBase"].asFloat();

						auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
						if (itAD != MSF_MainData::ammoDataMap.end())
						{
							AmmoData* itAmmoData = itAD->second;
							itAmmoData->baseAmmoData.mod = baseMod;
							itAmmoData->baseAmmoData.ammoID = ammoIDbase;
							itAmmoData->baseAmmoData.spawnChance = spawnChanceBase;
							data2 = ammoData["ammoTypes"];
							if (data2.isArray())
							{
								for (int i2 = 0; i2 < data2.size(); i2++)
								{
									const Json::Value& ammoType = data2[i2];
									std::string type = ammoType["ammo"].asString();
									if (type == "")
										continue;
									TESAmmo* ammo = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(type), TESForm, TESAmmo);
									if (!ammo)
										continue;
									type = ammoType["mod"].asString();
									if (type == "")
										continue;
									BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(type), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
									if (!mod)
										continue;
									if (mod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
										continue;
									UInt16 ammoID = ammoType["ammoID"].asInt();
									UInt8 spawnChance = ammoType["spawnChance"].asInt();

									std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin();
									for (itAmmoMod; itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
									{
										if (itAmmoMod->ammo == ammo)
										{
											itAmmoMod->mod = mod;
											itAmmoMod->ammoID = ammoID;
											itAmmoMod->spawnChance = spawnChance;
										}
									}
									if (itAmmoMod == itAmmoData->ammoMods.end())
									{
										AmmoData::AmmoMod ammoMod;
										ammoMod.ammo = ammo;
										ammoMod.mod = mod;
										ammoMod.ammoID = ammoID;
										ammoMod.spawnChance = spawnChance;
										itAmmoData->ammoMods.push_back(ammoMod);
									}
								}
							}
						}
						else
						{
							AmmoData* ammoDataStruct = new AmmoData();
							ammoDataStruct->baseAmmoData.ammo = baseAmmo;
							ammoDataStruct->baseAmmoData.mod = baseMod;
							ammoDataStruct->baseAmmoData.ammoID = ammoIDbase;
							ammoDataStruct->baseAmmoData.spawnChance = spawnChanceBase;
							data2 = ammoData["ammoTypes"];
							if (data2.isArray())
							{
								for (int i2 = 0; i2 < data2.size(); i2++)
								{
									const Json::Value& ammoType = data2[i2];
									std::string type = ammoType["ammo"].asString();
									if (type == "")
										continue;
									TESAmmo* ammo = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(type), TESForm, TESAmmo);
									if (!ammo)
										continue;
									type = ammoType["mod"].asString();
									if (type == "")
										continue;
									BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(type), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
									if (!mod)
										continue;
									if (mod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
										continue;
									UInt16 ammoID = ammoType["ammoID"].asInt();
									UInt8 spawnChance = ammoType["spawnChance"].asInt();

									std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = ammoDataStruct->ammoMods.begin();
									for (itAmmoMod; itAmmoMod != ammoDataStruct->ammoMods.end(); itAmmoMod++)
									{
										if (itAmmoMod->ammo == ammo)
										{
											itAmmoMod->mod = mod;
											itAmmoMod->ammoID = ammoID;
											itAmmoMod->spawnChance = spawnChance;
										}
									}
									if (itAmmoMod == ammoDataStruct->ammoMods.end())
									{
										AmmoData::AmmoMod ammoMod;
										ammoMod.ammo = ammo;
										ammoMod.mod = mod;
										ammoMod.ammoID = ammoID;
										ammoMod.spawnChance = spawnChance;
										ammoDataStruct->ammoMods.push_back(ammoMod);
									}
								}
							}
							if (ammoDataStruct->ammoMods.size() > 0)
								MSF_MainData::ammoDataMap[baseAmmo] = ammoDataStruct;
						}
					}
				}

				keydata = json["hotkeys"];
				if (keydata.isArray())
				{
					//_MESSAGE("hotkeys");
					for (int i = 0; i < keydata.size(); i++)
					{
						//_MESSAGE("i: %i", i);
						const Json::Value& key = keydata[i];
						std::string keyID = key["keyID"].asString();
						if (keyID == "")
							continue;
						auto kb = MSF_MainData::keybindIDMap.find(keyID);
						if (kb == MSF_MainData::keybindIDMap.end())
							continue;
						KeybindData* itKb = kb->second;
						if (itKb->type & KeybindData::bIsAmmo)
							continue;
						const Json::Value& modStruct = key["modData"];
						//_MESSAGE("beforeModStruct");
						if (!modStruct.isArray()) /////!!!!!!
						{
							//_MESSAGE("modStruct");
							std::string str = modStruct["apKeyword"].asString();
							BGSKeyword* apKeyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
							if (!apKeyword)
								continue;
							KeywordValue apValue = Utilities::GetAttachValueForTypedKeyword(apKeyword);
							if (apValue < 0)
								continue;
							//_MESSAGE("kw ok, modData: %p", itKb->modData);
							UInt16 apflags = modStruct["apFlags"].asInt();
							ModData* modData = itKb->modData;
							//APattachReqs
							if (!modData)
							{
								//_MESSAGE("new ModData");
								modData = new ModData();
								modData->attachParentValue = apValue;
								modData->flags = apflags;
							}
							const Json::Value& modCycles = modStruct["modArrays"];
							if (modCycles.isArray())
							{
								for (int i = 0; i < modCycles.size(); i++)
								{
									const Json::Value& modCycle = modCycles[i];
									str = modCycle["ifKeyword"].asString();
									BGSKeyword* ifKeyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
									if (!ifKeyword)
										continue;
									KeywordValue ifValue = Utilities::GetInstantiationValueForTypedKeyword(ifKeyword);
									if (ifValue < 0)
										continue;
									UInt16 ifflags = modCycle["ifFlags"].asInt();
									auto itCycle = modData->modCycleMap.find(ifValue);
									ModData::ModCycle* cycle = nullptr;
									if (itCycle == modData->modCycleMap.end())
									{
										cycle = new ModData::ModCycle();
										cycle->flags = ifflags;
									}
									else
										cycle = itCycle->second;
									const Json::Value& mods = modCycle["mods"];
									if (mods.isArray())
									{
										for (int i = 0; i < mods.size(); i++)
										{
											const Json::Value& switchmod = mods[i];
											str = switchmod["mod"].asString();
											if (str == "")
												continue;
											BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (!mod || mod->unkC0 != apValue)
												continue;
											ModData::ModVector::iterator itMod = std::find_if(cycle->mods.begin(), cycle->mods.end(), [mod](ModData::Mod* data){
												return data->mod == mod;
											});
											if (itMod != cycle->mods.end()) //overwrite?
												continue;
											UInt16 modflags = switchmod["modFlags"].asInt();
											KeywordValue animFlavor = 0;
											str = switchmod["animFlavor"].asString();
											if (str != "")
											{
												BGSKeyword* flavor = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
												KeywordValue value = Utilities::GetAnimFlavorValueForTypedKeyword(flavor);
												if (value != -1)
													animFlavor = value;
											}
											//requirements
											TESIdleForm* animIdle_1stP = nullptr;
											TESIdleForm* animIdle_3rdP = nullptr;
											str = switchmod["animIdle_1stP"].asString();
											if (str != "")
												animIdle_1stP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
											str = switchmod["animIdle_3rdP"].asString();
											if (str != "")
												animIdle_3rdP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);

											ModData::Mod* modDataMod = new ModData::Mod();
											modDataMod->mod = mod;
											modDataMod->flags = modflags;
											if (animIdle_1stP || animIdle_3rdP)
											{
												AnimationData* animData = new AnimationData();
												animData->animIdle_1stP = animIdle_1stP;
												animData->animIdle_3rdP = animIdle_3rdP;
												modDataMod->animData = animData;
											}
											cycle->mods.push_back(modDataMod);
										}
									}
									if (cycle->mods.size() < 1 || ((cycle->flags & ModData::ModCycle::bCannotHaveNullMod) && cycle->mods.size() < 2))
									{
										for (ModData::ModVector::iterator itMod = cycle->mods.begin(); itMod != cycle->mods.end(); itMod++)
										{
											ModData::Mod* modDataMod = *itMod;
											delete modDataMod;
										}
										delete cycle;
										continue;
									}
									modData->modCycleMap[ifValue] = cycle;
								}
							}
							if (modData->modCycleMap.size() < 1)
							{
								//deletereqs
								delete modData;
								continue;
							}
							itKb->modData = modData;
							if (modData->flags & ModData::bRequireAPmod)
								Utilities::AddAttachValue((AttachParentArray*)&MSF_MainData::APbaseMod->unk98, apValue);

						}
					}
				}

				data1 = json["reloadAnim"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& animData = data1[i];
						std::string str = animData["weapon"].asString();
						if (str == "")
							continue;
						TESObjectWEAP* weapon = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESObjectWEAP);
						if (!weapon)
							continue;
						TESIdleForm* animIdle_1stP = nullptr;
						TESIdleForm* animIdle_3rdP = nullptr;
						str = animData["animIdle_1stP"].asString();
						if (str != "")
							animIdle_1stP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_1stP)
							continue;
						str = animData["animIdle_3rdP"].asString();
						if (str != "")
							animIdle_3rdP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_3rdP)
							continue;

						auto itAnim = MSF_MainData::reloadAnimDataMap.find(weapon);
						if (itAnim != MSF_MainData::reloadAnimDataMap.end())
						{
							itAnim->second->animIdle_1stP = animIdle_1stP;
							itAnim->second->animIdle_3rdP = animIdle_3rdP;
						}
						else
						{
							AnimationData* animationData = new AnimationData();
							animationData->animIdle_1stP = animIdle_1stP;
							animationData->animIdle_3rdP = animIdle_3rdP;
							MSF_MainData::reloadAnimDataMap[weapon] = animationData;
						}
					}
				}

				data1 = json["firingAnim"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& animData = data1[i];
						std::string str = animData["weapon"].asString();
						if (str == "")
							continue;
						TESObjectWEAP* weapon = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESObjectWEAP);
						if (!weapon)
							continue;
						TESIdleForm* animIdle_1stP = nullptr;
						TESIdleForm* animIdle_3rdP = nullptr;
						str = animData["animIdle_1stP"].asString();
						if (str != "")
							animIdle_1stP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_1stP)
							continue;
						str = animData["animIdle_3rdP"].asString();
						if (str != "")
							animIdle_3rdP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_3rdP)
							continue;

						auto itAnim = MSF_MainData::fireAnimDataMap.find(weapon);
						if (itAnim != MSF_MainData::fireAnimDataMap.end())
						{
							itAnim->second->animIdle_1stP = animIdle_1stP;
							itAnim->second->animIdle_3rdP = animIdle_3rdP;
						}
						else
						{
							AnimationData* animationData = new AnimationData();
							animationData = new AnimationData();
							animationData->animIdle_1stP = animIdle_1stP;
							animationData->animIdle_3rdP = animIdle_3rdP;
							MSF_MainData::fireAnimDataMap[weapon] = animationData;
						}
					}
				}

				data1 = json["fmScaleform"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& scaleformData = data1[i];
						std::string str = scaleformData["keyword"].asString();
						if (str == "")
							continue;
						BGSKeyword* keyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
						if (!keyword)
							continue;
						str = scaleformData["displayString"].asString();
						std::vector<HUDFiringModeData>::iterator itData = MSF_MainData::fmDisplayData.begin();
						for (itData; itData != MSF_MainData::fmDisplayData.end(); itData++)
						{
							if (itData->keyword == keyword)
							{
								itData->displayString = str;
								break;
							}
						}
						if (itData == MSF_MainData::fmDisplayData.end())
						{
							HUDFiringModeData displayData;
							displayData.keyword = keyword;
							displayData.displayString = str;
							MSF_MainData::fmDisplayData.push_back(displayData);
						}
					}
				}

				data1 = json["scopeScaleform"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& scaleformData = data1[i];
						std::string str = scaleformData["keyword"].asString();
						if (str == "")
							continue;
						BGSKeyword* keyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
						if (!keyword)
							continue;
						str = scaleformData["displayString"].asString();
						std::vector<HUDScopeData>::iterator itData = MSF_MainData::scopeDisplayData.begin();
						for (itData; itData != MSF_MainData::scopeDisplayData.end(); itData++)
						{
							if (itData->keyword == keyword)
							{
								itData->displayString = str;
								break;
							}
						}
						if (itData == MSF_MainData::scopeDisplayData.end())
						{
							HUDScopeData displayData;
							displayData.keyword = keyword;
							displayData.displayString = str;
							MSF_MainData::scopeDisplayData.push_back(displayData);
						}
					}
				}

				data1 = json["muzzleScaleform"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& scaleformData = data1[i];
						std::string str = scaleformData["keyword"].asString();
						if (str == "")
							continue;
						BGSKeyword* keyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
						if (!keyword)
							continue;
						str = scaleformData["displayString"].asString();
						std::vector<HUDMuzzleData>::iterator itData = MSF_MainData::muzzleDisplayData.begin();
						for (itData; itData != MSF_MainData::muzzleDisplayData.end(); itData++)
						{
							if (itData->keyword == keyword)
							{
								itData->displayString = str;
								break;
							}
						}
						if (itData == MSF_MainData::muzzleDisplayData.end())
						{
							HUDMuzzleData displayData;
							displayData.keyword = keyword;
							displayData.displayString = str;
							MSF_MainData::muzzleDisplayData.push_back(displayData);
						}
					}
				}
				_MESSAGE("end True");
				return true;
			}
		}
		catch (...)
		{
			_WARNING("Warning - Failed to load Gameplay Data from %s.json", fileName.c_str());
			return false;
		}
		_MESSAGE("end False");
		return false;
	}

	//========================= Select Object Mod =======================
	SwitchData* GetNthAmmoMod(UInt32 num)
	{
		if (num < 0)
			return nullptr;
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(*g_player, 41);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
		if (!baseAmmo)
			return nullptr;
		auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itAD != MSF_MainData::ammoDataMap.end())
		{
			AmmoData* itAmmoData = itAD->second;
			if (num == 0)
			{
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
				{
					if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, baseAmmo) == 0)
						return nullptr;
				}
				SwitchData* switchData = new SwitchData();
				switchData->ModToRemove = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::hasSwitchedAmmoKW);
				return switchData;
			}
			num--;
			if ((num + 1) > itAmmoData->ammoMods.size())
				return nullptr;
			AmmoData::AmmoMod* ammoMod = &itAmmoData->ammoMods[num];
			if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
			{
				if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, ammoMod->ammo) == 0)
					return nullptr;
			}
			SwitchData* switchData = new SwitchData();
			switchData->ModToAttach = ammoMod->mod;
			return switchData;
		}
		return nullptr;
	}

	bool GetNthMod(UInt32 num, BGSInventoryItem::Stack* eqStack, ModData* modData)
	{
		if (num < 0)
			return false;
		if (!eqStack || !modData)
			return false;
		ExtraDataList* dataList = eqStack->extraData;
		if (!dataList)
			return false;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!attachedMods || !instanceData)
			return false;

		std::vector<KeywordValue> instantiationValues;
		if (!Utilities::GetParentInstantiationValues(attachedMods, modData->attachParentValue, &instantiationValues))
			return false;

		ModData::ModCycle* modCycle = nullptr;
		for (std::vector<KeywordValue>::iterator itData = instantiationValues.begin(); itData != instantiationValues.end(); itData++)
		{
			KeywordValue value = *itData;
			auto itCycle = modData->modCycleMap.find(value);
			if (itCycle != modData->modCycleMap.end() && modCycle)
			{
				_MESSAGE("Ambiguity error");
				return false;
			}
			modCycle = itCycle->second;
		}
		if (!modCycle)
			return false;

		if (num > modCycle->mods.size() || (modCycle->flags & ModData::ModCycle::bCannotHaveNullMod && num >= modCycle->mods.size()))
			return false; //out of range
		ModData::Mod* modToAttach = nullptr;
		ModData::Mod* modToRemove = nullptr;
		if (modCycle->flags & ModData::ModCycle::bCannotHaveNullMod)
			modToAttach = modCycle->mods[num];
		else if (num != 0)
			modToAttach = modCycle->mods[num-1];

		BGSMod::Attachment::Mod* attachedMod = Utilities::GetModAtAttachPoint(attachedMods, modData->attachParentValue);
		ModData::ModVector::iterator itMod = std::find_if(modCycle->mods.begin(), modCycle->mods.end(), [attachedMod](ModData::Mod* data){
			return data->mod == attachedMod;
		});
		if (itMod != modCycle->mods.end())
			modToRemove = *itMod;

		if (!CheckSwitchRequirements(eqStack, modToAttach, modToRemove))
			return false;

		return QueueModsToSwitch(modToAttach, modToRemove, modData->flags & ModData::bRequireAPmod);
	}

	bool GetNextMod(BGSInventoryItem::Stack* eqStack, ModData* modData)
	{
		if (!eqStack || !modData)
			return false;
		ExtraDataList* dataList = eqStack->extraData;
		if (!dataList)
			return false;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!attachedMods || !instanceData)
			return false;

		_MESSAGE("check OK");
		std::vector<KeywordValue> instantiationValues;
		if (!Utilities::GetParentInstantiationValues(attachedMods, modData->attachParentValue, &instantiationValues))
			return false;
		_MESSAGE("if counts: %i", instantiationValues.size());

		ModData::ModCycle* modCycle = nullptr;
		for (std::vector<KeywordValue>::iterator itData = instantiationValues.begin(); itData != instantiationValues.end(); itData++)
		{
			KeywordValue value = *itData;
			_MESSAGE("it: %i", value);
			auto itCycle = modData->modCycleMap.find(value);
			if (itCycle != modData->modCycleMap.end() && modCycle)
			{
				_MESSAGE("Ambiguity error");
				return false;
			}
			modCycle = itCycle->second;
		}
		if (!modCycle)
			return false;

		ModData::Mod* modToAttach = nullptr;
		ModData::Mod* modToRemove = nullptr;
		BGSMod::Attachment::Mod* attachedMod = Utilities::GetModAtAttachPoint(attachedMods, modData->attachParentValue);
		ModData::ModVector::iterator itMod = std::find_if(modCycle->mods.begin(), modCycle->mods.end(), [attachedMod](ModData::Mod* data){
			return data->mod == attachedMod;
		});
		if (itMod == modCycle->mods.end())
			modToAttach = modCycle->mods[0];
		else
		{
			modToRemove = *itMod;
			itMod++;
			if (itMod != modCycle->mods.end())
				modToAttach = *itMod;
			else if (modCycle->flags & ModData::ModCycle::bCannotHaveNullMod)
				modToAttach = modCycle->mods[0];
		}

		if (!CheckSwitchRequirements(eqStack, modToAttach, modToRemove))
			return false;

		return QueueModsToSwitch(modToAttach, modToRemove, modData->flags & ModData::bRequireAPmod);
	}

	bool CheckSwitchRequirements(BGSInventoryItem::Stack* stack, ModData::Mod* modToAttach, ModData::Mod* modToRemove)
	{
		return true;
	}

	bool QueueModsToSwitch(ModData::Mod* modToAttach, ModData::Mod* modToRemove, bool bNeedInit)
	{
		if (modToAttach == modToRemove)
			return false; //no change
		SwitchData* switchRemove = nullptr;
		SwitchData* switchAttach = nullptr;
		if (modToRemove && (!modToAttach || (modToRemove->flags & ModData::Mod::bStandaloneRemove) || (modToAttach && (modToRemove->flags & ModData::Mod::bStandaloneAttach))))
		{
			if (!(modToRemove->flags & SwitchData::bIgnoreAnimations))
			{
				if (!MSF_Base::HandlePendingAnimations())
					return false;
			}
			switchRemove = new SwitchData();
			switchRemove->ModToRemove = modToRemove->mod;
			switchRemove->LooseModToAdd = (modToRemove->flags & ModData::Mod::bRequireLooseMod) ? Utilities::GetLooseMod(modToRemove->mod) : nullptr;
			switchRemove->SwitchFlags = modToRemove->flags & ModData::Mod::mBitTransferMask;
			if (modToRemove->animData)
				switchRemove->animData = modToRemove->animData;
		}
		if (modToAttach)
		{
			TESObjectMISC* looseMod = nullptr;
			if (modToAttach->flags & ModData::Mod::bRequireLooseMod)
			{
				looseMod = Utilities::GetLooseMod(modToAttach->mod);
				if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) == 0)
				{
					delete switchRemove;
					return false; //missing loosemod
				}
			}
			if (!(modToAttach->flags & SwitchData::bIgnoreAnimations) || ((!switchRemove && modToRemove) && !(modToRemove->flags & SwitchData::bIgnoreAnimations)))
			{
				if (!MSF_Base::HandlePendingAnimations())
				{
					delete switchRemove;
					return false;
				}
			}
			switchAttach = new SwitchData();
			switchAttach->LooseModToRemove = looseMod;
			switchAttach->ModToAttach = modToAttach->mod;
			switchAttach->SwitchFlags = modToAttach->flags & ModData::Mod::mBitTransferMask;
			if (!switchRemove && modToRemove)
			{
				switchAttach->ModToRemove = modToRemove->mod;
				switchAttach->LooseModToAdd = (modToRemove->flags & ModData::Mod::bRequireLooseMod) ? Utilities::GetLooseMod(modToRemove->mod) : nullptr;
				switchAttach->SwitchFlags &= (modToRemove->flags & ModData::Mod::bRequireWeaponToBeDrawn) | ModData::Mod::bRequireLooseMod | ModData::Mod::bUpdateAnimGraph | ModData::Mod::bIgnoreAnimations;
				switchAttach->SwitchFlags |= modToRemove->flags & ModData::Mod::bUpdateAnimGraph;
			}
			if (modToAttach->animData)
				switchAttach->animData = modToAttach->animData;
		}

		if (switchRemove && (!switchRemove->animData || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled)))
		{
			if ((MSF_MainData::modSwitchManager.GetState() != 0 || MSF_MainData::modSwitchManager.GetQueueCount() > 0) && switchAttach && switchAttach->animData && (MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled))
			{
				delete switchRemove;
				delete switchAttach;
				return false;
			}
			if (!MSF_Base::SwitchMod(switchRemove, true))
			{
				delete switchAttach;
				return false;
			}
			if (!switchAttach)
				return true;
			if (!switchAttach->animData || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled))
				return MSF_Base::SwitchMod(switchAttach, true);
		}
		else if (switchAttach && ((!switchRemove && !switchAttach->animData) || !(MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled)))
			return MSF_Base::SwitchMod(switchAttach, true);

		if (MSF_MainData::modSwitchManager.GetState() != 0 || MSF_MainData::modSwitchManager.GetQueueCount() > 0)
		{
			delete switchRemove;
			delete switchAttach;
			return false;
		}

		SwitchData* switchData = nullptr;
		if (switchRemove)
			switchData = switchRemove;
		else
			switchData = switchAttach;
		bool QueueAttach = false;
		if (switchRemove && switchAttach)
		{
			QueueAttach = true;
			switchAttach->SwitchFlags |= SwitchData::bAnimNeeded;
		}

		if ((*g_player)->actorState.IsWeaponDrawn())
		{
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bAnimInProgress); // | SwitchData::bReloadNotFinished
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			if (QueueAttach)
				MSF_MainData::modSwitchManager.QueueSwitch(switchAttach);
			if (!MSF_Base::PlayAnim(switchData->animData))
			{
				MSF_Base::SwitchMod(switchData, true);
				switchAttach->SwitchFlags &= ~SwitchData::bAnimNeeded;
				if (QueueAttach && !MSF_Base::PlayAnim(switchAttach->animData))
					MSF_Base::SwitchMod(switchAttach, true);
			}
			return true;
		}
		else if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
		{
			switchData->SwitchFlags |= (SwitchData::bSwitchingInProgress | SwitchData::bDrawInProgress | SwitchData::bAnimNeeded);
			MSF_MainData::modSwitchManager.QueueSwitch(switchData);
			if (QueueAttach)
				MSF_MainData::modSwitchManager.QueueSwitch(switchAttach);
			Utilities::DrawWeapon(*g_player);
			//delay check draw state
			return true;
		}
		delete switchRemove;
		delete switchAttach;
		return false;
	}

	TESAmmo* GetBaseCaliber(BGSInventoryItem::Stack* stack)
	{
		if (!stack)
			return nullptr;
		BGSObjectInstanceExtra* objectModData = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;

		if (!objectModData || !weapBase)
			return nullptr;

		auto data = objectModData->data;
		if (!data || !data->forms)
			return nullptr;

		UInt64 priority = 0;
		TESAmmo* ammoConverted = nullptr;
		//TESAmmo* switchedAmmo = nullptr;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (!objectMod)
				continue;
			//UInt64 currPriority = ((objectMod->priority & 0x80) >> 7) * (objectMod->priority & ~0x80) + (((objectMod->priority & 0x80) >> 7) ^ 1)*(objectMod->priority & ~0x80) + (((objectMod->priority & 0x80) >> 7) ^ 1) * 127;
			UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
			TESAmmo* currModAmmo = nullptr;
			bool isSwitchedAmmo = false;
			for (UInt32 i4 = 0; i4 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i4];
				if (data->target == 61 && data->value.form && data->op == 128)
				{
					if (currPriority >= priority)
						currModAmmo = (TESAmmo*)data->value.form;
				}
				else if (data->target == 31 && data->value.form && data->op == 144)
				{
					if ((BGSKeyword*)data->value.form == MSF_MainData::hasSwitchedAmmoKW)
						isSwitchedAmmo = true;
				}
			}
			if (!isSwitchedAmmo && currModAmmo)
			{
				ammoConverted = currModAmmo;
				priority = currPriority;
			}
		}

		if (!ammoConverted)
			return weapBase->weapData.ammo;
		return ammoConverted;
	}

	bool PickRandomMods(tArray<BGSMod::Attachment::Mod*>* mods, TESAmmo** ammo, UInt32* count)
	{
		TESAmmo* baseAmmo = *ammo;
		*ammo = nullptr;
		UInt32 chance = 0;
		if (baseAmmo)
		{
			auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
			if (itAD != MSF_MainData::ammoDataMap.end())
			{
				AmmoData* itAmmoData = itAD->second;
				chance += itAmmoData->baseAmmoData.spawnChance;
				UInt32 _chance = 0;
				for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
				{
					_chance += itAmmoMod->spawnChance;
				}
				if (_chance == 0)
					return false;
				chance += _chance - 1;
				UInt32 picked = MSF_MainData::rng.RandomInt(0, chance - 1);
				_chance = itAmmoData->baseAmmoData.spawnChance - 1;
				if (picked <= _chance)
					return false;
				for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
				{
					_chance += itAmmoMod->spawnChance;
					if (picked <= _chance)
					{
						mods->Push(itAmmoMod->mod);
						*ammo = itAmmoMod->ammo;
						*count = MSF_MainData::rng.RandomInt(6, 48);
						break;
					}
				}
				return true;
			}
		}
		// mod association



		return false;
	}

	//========================= Animations =======================
	TESIdleForm* GetReloadAnimation(TESObjectWEAP* equippedWeap, bool get3rdP)
	{
		auto itAnim = MSF_MainData::reloadAnimDataMap.find(equippedWeap);
		if (itAnim != MSF_MainData::reloadAnimDataMap.end())
		{
			if (get3rdP)
				return itAnim->second->animIdle_3rdP;
			else
				return itAnim->second->animIdle_1stP;
		}
		if (get3rdP)
			return MSF_MainData::reloadIdle3rdP;
		else
			return MSF_MainData::reloadIdle1stP;
	}

	TESIdleForm* GetFireAnimation(TESObjectWEAP* equippedWeap, bool get3rdP)
	{
		auto itAnim = MSF_MainData::fireAnimDataMap.find(equippedWeap);
		if (itAnim != MSF_MainData::fireAnimDataMap.end())
		{
			if (get3rdP)
				return itAnim->second->animIdle_3rdP;
			else
				return itAnim->second->animIdle_1stP;
		}
		if (get3rdP)
			return MSF_MainData::fireIdle3rdP;
		else
			return MSF_MainData::fireIdle1stP;
	}

	//================= Interface Data ==================

	std::string GetFMString(TESObjectWEAP::InstanceData* instanceData)
	{
		if (!instanceData)
			return "";
		for (std::vector<HUDFiringModeData>::iterator it = MSF_MainData::fmDisplayData.begin(); it != MSF_MainData::fmDisplayData.end(); it++)
		{
			if (!it->keyword)
				continue;
			if (Utilities::WeaponInstanceHasKeyword(instanceData, it->keyword))
			{
				return it->displayString;
			}
		}
		return "";
	}

	std::string GetScopeString(TESObjectWEAP::InstanceData* instanceData)
	{
		if (!instanceData)
			return "";
		for (std::vector<HUDScopeData>::iterator it = MSF_MainData::scopeDisplayData.begin(); it != MSF_MainData::scopeDisplayData.end(); it++)
		{
			if (!it->keyword)
				continue;
			if (Utilities::WeaponInstanceHasKeyword(instanceData, it->keyword))
			{
				return it->displayString;
			}
		}
		return "";
	}

	std::string GetMuzzleString(TESObjectWEAP::InstanceData* instanceData) //mod data -> muzzle mod -> name
	{
		if (!instanceData)
			return "";
		for (std::vector<HUDMuzzleData>::iterator it = MSF_MainData::muzzleDisplayData.begin(); it != MSF_MainData::muzzleDisplayData.end(); it++)
		{
			if (!it->keyword)
				continue;
			if (Utilities::WeaponInstanceHasKeyword(instanceData, it->keyword))
			{
				return it->displayString;
			}
		}
		return "";
	}

	KeybindData* GetKeybindDataForKey(UInt16 keyCode, UInt8 modifiers)
	{
		UInt64 key = ((UInt64)modifiers << 32) + keyCode;
		auto itKey = MSF_MainData::keybindMap.find(key);
		if (itKey != MSF_MainData::keybindMap.end())
			return itKey->second;
		else
			return nullptr;
	}
}

