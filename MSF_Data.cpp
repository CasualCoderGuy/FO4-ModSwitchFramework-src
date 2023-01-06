#include "MSF_Data.h"
#include "MSF_Scaleform.h"
//#include "MCM/SettingStore.h"
#include "json/json.h"
#include "RNG.h"
#include <fstream>
#include <algorithm>

std::vector<AmmoData> MSF_MainData::ammoData;
std::vector<SingleModPair> MSF_MainData::singleModPairs;
std::vector<ModPairArray> MSF_MainData::modPairArrays;
std::vector<MultipleMod> MSF_MainData::multiModAssocs;
std::vector<ReloadAnimData> MSF_MainData::reloadAnimData;
std::vector<FireAnimData> MSF_MainData::fireAnimData;

std::vector<HUDFiringModeData> MSF_MainData::fmDisplayData;
std::vector<HUDScopeData> MSF_MainData::scopeDisplayData;
std::vector<HUDMuzzleData> MSF_MainData::muzzleDisplayData;

BGSKeyword* MSF_MainData::hasSwitchedAmmoKW;
BGSKeyword* MSF_MainData::hasSwitchedSecAmmoKW;
BGSMod::Attachment::Mod* MSF_MainData::APbaseMod;
BGSMod::Attachment::Mod* MSF_MainData::NullMuzzleMod;
BGSKeyword* MSF_MainData::CanHaveNullMuzzleKW;
BGSKeyword* MSF_MainData::FiringModeUnderbarrelKW;
BGSMod::Attachment::Mod* MSF_MainData::PlaceholderMod;
BGSMod::Attachment::Mod* MSF_MainData::PlaceholderModAmmo;
ActorValueInfo*  MSF_MainData::BurstModeTime;
ActorValueInfo*  MSF_MainData::BurstModeFlags;
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
std::vector<KeybindData> MSF_MainData::keybinds;
std::vector<MCMfloatData> MSF_MainData::MCMfloats;
GFxMovieRoot* MSF_MainData::MSFMenuRoot = nullptr;
ModSelectionMenu* MSF_MainData::widgetMenu;
int MSF_MainData::numberOfOpenedMenus = 0;

SwitchData MSF_MainData::switchData;
std::vector<BurstMode> MSF_MainData::burstMode;
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
		MSF_MainData::NullMuzzleMod = (BGSMod::Attachment::Mod*)LookupFormByID((UInt32)0x004F21D);
		MSF_MainData::CanHaveNullMuzzleKW = (BGSKeyword*)LookupFormByID((UInt32)0x01C9E78);
		MSF_MainData::FiringModeUnderbarrelKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000021);
		MSF_MainData::BurstModeTime = (ActorValueInfo*)LookupFormByID(formIDbase | (UInt32)0x000006B); //& updateinstancedata, test if persists after unequipping weapon
		MSF_MainData::BurstModeFlags = (ActorValueInfo*)LookupFormByID(formIDbase | (UInt32)0x000006C);
		MSF_MainData::fireIdle1stP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0004AE1));
		MSF_MainData::fireIdle3rdP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0018E1F));
		MSF_MainData::reloadIdle1stP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0004D33));
		MSF_MainData::reloadIdle3rdP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x00BDDA6));
		MSF_MainData::ActionFireSingle = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A5A));
		MSF_MainData::ActionFireAuto = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A5C));
		MSF_MainData::ActionReload = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A56));
		MSF_MainData::ActionDraw = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x00132AF));
		MSF_MainData::ActionGunDown = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0022A35));
		//MSF_MainData::PlaceholderMod = (BGSMod::Attachment::Mod*)LookupFormByID(formIDbase | (UInt32)0x000005F);
		MSF_MainData::PlaceholderModAmmo = (BGSMod::Attachment::Mod*)LookupFormByID(formIDbase | (UInt32)0x000005F);
		//MSF_MainData::PlaceholderModFM = (BGSMod::Attachment::Mod*)LookupFormByID(formIDbase | (UInt32)0x0000060);
		//MSF_MainData::PlaceholderModUB = (BGSMod::Attachment::Mod*)LookupFormByID(formIDbase | (UInt32)0x0000061);
		//MSF_MainData::PlaceholderModZD = (BGSMod::Attachment::Mod*)LookupFormByID(formIDbase | (UInt32)0x0000062);
		
		return true;
	}

	bool InitMCMSettings()
	{
		if (!ReadMCMKeybindData())
			return false;

		MSF_MainData::MCMSettingFlags = 0x77F;
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
		MCMfloatData data;
		data.name = name;
		data.value = value;
		MSF_MainData::MCMfloats.push_back(data);
		return;
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
					std::vector<KeybindData>::iterator itKb = MSF_MainData::keybinds.begin();
					for (itKb; itKb != MSF_MainData::keybinds.end(); itKb++)
					{
						if (itKb->functionID == id)
						{
							itKb->keyCode = keybind["keycode"].asInt();
							itKb->modifiers = keybind["modifiers"].asInt();
							break;
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

						std::vector<KeybindData>::iterator itKb = MSF_MainData::keybinds.begin();
						for (itKb; itKb != MSF_MainData::keybinds.end(); itKb++)
						{
							if (itKb->functionID == id)
							{
								std::string menuName = keybind["menuName"].asString();
								std::string swfFilename = keybind["swfFilename"].asString();
								UInt8 flags = keybind["keyfunction"].asInt();
								//if ((menuName == "" || swfFilename == "") && (flags & KeybindData::bHUDselection))
								//	break;
								//if (flags & KeybindData::bHUDselection)
								//{
								//	if (itKb->selectMenu)
								//	{
								//		itKb->selectMenu->scaleformID = menuName;
								//		itKb->selectMenu->swfFilename = swfFilename;
								//	}
								//	else
								//		itKb->selectMenu = new ModSelectionMenu(menuName, swfFilename);
								//}
								//else
								itKb->selectMenu = nullptr;
								itKb->type = flags;
								break;
							}
							//BSReadAndWriteLocker locker(&g_customMenuLock);
							//g_customMenuData[menuName.c_str()].menuPath = menuPath;
							//g_customMenuData[menuName.c_str()].rootPath = rootPath;
							//g_customMenuData[menuName.c_str()].menuFlags = menuFlags;
							//g_customMenuData[menuName.c_str()].movieFlags = movieFlags;
							//g_customMenuData[menuName.c_str()].extFlags = extFlags;
							//g_customMenuData[menuName.c_str()].depth = depth;
						}
						if (itKb == MSF_MainData::keybinds.end())
						{
							std::string menuName = keybind["menuName"].asString();
							std::string swfFilename = keybind["swfFilename"].asString();
							UInt8 flags = keybind["keyfunction"].asInt();
							//if ((menuName == "" || swfFilename == "") && (flags & KeybindData::bHUDselection))
							//	continue;
							KeybindData kb = {};
							kb.functionID = id;
							kb.type = flags;
							kb.keyCode = 0;
							kb.modifiers = 0;
							//if (flags & KeybindData::bHUDselection)
							//	kb.selectMenu = new ModSelectionMenu(menuName, swfFilename);
							//else
							kb.selectMenu = nullptr;
							MSF_MainData::keybinds.push_back(kb);
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

		char* modSettingsDirectory = "Data\\MCM\\Settings\\anagy_ModSwitchFramework.ini";

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
		std::string iniLocation = "./Data/MCM/Settings/anagy_ModSwitchFramework.ini";
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

			if (flagValue)
				MSF_MainData::MCMSettingFlags |= flag;
			else
				MSF_MainData::MCMSettingFlags &= ~flag;
			return true;
		}
		else if (section == "Position")
		{
			for (std::vector<MCMfloatData>::iterator it = MSF_MainData::MCMfloats.begin(); it != MSF_MainData::MCMfloats.end(); it++)
			{
				if (it->name == settingName)
				{
					it->value = std::stof(settingValue);
					return true;
				}
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
				
				data1 = json["plugins"];

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

						std::vector<AmmoData>::iterator itAmmoData = MSF_MainData::ammoData.begin();
						for (itAmmoData; itAmmoData != MSF_MainData::ammoData.end(); itAmmoData++)
						{
							if (itAmmoData->baseAmmoData.ammo == baseAmmo)
							{
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
								break;
							}
						}
						if (itAmmoData == MSF_MainData::ammoData.end())
						{
							AmmoData ammoDataStruct;
							ammoDataStruct.baseAmmoData.ammo = baseAmmo;
							ammoDataStruct.baseAmmoData.mod = baseMod;
							ammoDataStruct.baseAmmoData.ammoID = ammoIDbase;
							ammoDataStruct.baseAmmoData.spawnChance = spawnChanceBase;
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

									std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = ammoDataStruct.ammoMods.begin();
									for (itAmmoMod; itAmmoMod != ammoDataStruct.ammoMods.end(); itAmmoMod++)
									{
										if (itAmmoMod->ammo == ammo)
										{
											itAmmoMod->mod = mod;
											itAmmoMod->ammoID = ammoID;
											itAmmoMod->spawnChance = spawnChance;
										}
									}
									if (itAmmoMod == ammoDataStruct.ammoMods.end())
									{
										AmmoData::AmmoMod ammoMod;
										ammoMod.ammo = ammo;
										ammoMod.mod = mod;
										ammoMod.ammoID = ammoID;
										ammoMod.spawnChance = spawnChance;
										ammoDataStruct.ammoMods.push_back(ammoMod);
									}
								}
							}
							if (ammoDataStruct.ammoMods.size() > 0)
								MSF_MainData::ammoData.push_back(ammoDataStruct);
						}
					}
				}

				keydata = json["hotkeys"];
				if (keydata.isArray())
				{
					for (int i = 0; i < keydata.size(); i++)
					{
						const Json::Value& key = keydata[i];
						std::string keyID = key["keyID"].asString();
						if (keyID == "")
							continue;
						std::vector<KeybindData>::iterator itKb = MSF_MainData::keybinds.begin();
						for (itKb; itKb != MSF_MainData::keybinds.end(); itKb++)
						{
							if (itKb->functionID == keyID)
							{
								if (itKb->type & KeybindData::bIsAmmo)
									break;
								data1 = key["modData"];
								if (data1.isArray())
								{
									for (int i = 0; i < data1.size(); i++)
									{
										const Json::Value& modData = data1[i];
										UInt16 type = modData["type"].asInt();
										if (type < 1 || type > 3)
											continue;
										std::string str = modData["fnKeyword"].asString();
										BGSKeyword* functionKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
										if (!functionKW)
											continue;
										UInt16 flags = modData["flags"].asInt();
										TESIdleForm* animIdle_1stP = nullptr;
										TESIdleForm* animIdle_3rdP = nullptr;
										str = modData["animIdle_1stP"].asString();
										if (str != "")
											animIdle_1stP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
										str = modData["animIdle_3rdP"].asString();
										if (str != "")
											animIdle_3rdP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
										bool bAdd = false;
										if (type == 1)
										{
											str = modData["baseMod"].asString();
											if (str == "")
												continue;
											BGSMod::Attachment::Mod* baseMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (!baseMod)
												continue;
											if (baseMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
												continue;
											str = modData["functionMod"].asString();
											if (str == "")
												continue;
											BGSMod::Attachment::Mod* functionMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (!functionMod)
												continue;
											if (functionMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
												continue;
											std::vector<ModAssociationData*>::iterator itData = itKb->modAssociations.begin();
											for (itData; itData != itKb->modAssociations.end(); itData++)
											{
												if ((*itData)->funcKeyword == functionKW)
													break;
												if ((*itData)->GetType() == 1)
												{
													SingleModPair* currModPair = static_cast<SingleModPair*>(*itData);
													if (currModPair->modPair.parentMod == baseMod)
														break;
												}
												else if ((*itData)->GetType() == 3)
												{
													MultipleMod* currMultipleMod = static_cast<MultipleMod*>(*itData);
													if (currMultipleMod->parentMod == baseMod)
														break;
												}
												else if ((*itData)->GetType() == 2)
												{
													ModPairArray* currModPairArray = static_cast<ModPairArray*>(*itData);
													std::vector<ModAssociationData::ModPair>::iterator itModPair = currModPairArray->modPairs.begin();
													for (itModPair; itModPair != currModPairArray->modPairs.end(); itModPair++)
													{
														if (itModPair->parentMod == baseMod)
															break;
													}
													if (itModPair != currModPairArray->modPairs.end())
														break;
												}
											}
											if (itData == itKb->modAssociations.end())
											{
												SingleModPair* modPair = new SingleModPair;
												modPair->modPair.parentMod = baseMod;
												modPair->modPair.functionMod = functionMod;
												modPair->funcKeyword = functionKW;
												modPair->flags = flags;
												modPair->animIdle_1stP = animIdle_1stP;
												modPair->animIdle_3rdP = animIdle_3rdP;
												itKb->modAssociations.push_back(modPair);
											}
										}
										else if (type == 3)
										{
											str = modData["baseMod"].asString();
											if (str == "")
												continue;
											BGSMod::Attachment::Mod* baseMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (!baseMod)
												continue;
											if (baseMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
												continue;
											std::vector<ModAssociationData*>::iterator itData = itKb->modAssociations.begin();
											for (itData; itData != itKb->modAssociations.end(); itData++)
											{
												if ((*itData)->funcKeyword == functionKW)
													break;
												if ((*itData)->GetType() == 1)
												{
													SingleModPair* currModPair = static_cast<SingleModPair*>(*itData);
													if (currModPair->modPair.parentMod == baseMod)
														break;
												}
												else if ((*itData)->GetType() == 3)
												{
													MultipleMod* currMultipleMod = static_cast<MultipleMod*>(*itData);
													if (currMultipleMod->parentMod == baseMod)
														break;
												}
												else if ((*itData)->GetType() == 2)
												{
													ModPairArray* currModPairArray = static_cast<ModPairArray*>(*itData);
													std::vector<ModAssociationData::ModPair>::iterator itModPair = currModPairArray->modPairs.begin();
													for (itModPair; itModPair != currModPairArray->modPairs.end(); itModPair++)
													{
														if (itModPair->parentMod == baseMod)
															break;
													}
													if (itModPair != currModPairArray->modPairs.end())
														break;
												}
											}
											if (itData == itKb->modAssociations.end())
											{
												MultipleMod* multipleMod = new MultipleMod;
												multipleMod->parentMod = baseMod;
												multipleMod->funcKeyword = functionKW;
												multipleMod->flags = flags;
												multipleMod->animIdle_1stP = animIdle_1stP;
												multipleMod->animIdle_3rdP = animIdle_3rdP;
												data2 = modData["functionMods"];
												if (data2.isArray())
												{
													for (int i2 = 0; i2 < data2.size(); i2++)
													{
														str = data2[i2].asString();
														if (str == "")
															continue;
														BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
														if (!mod)
															continue;
														if (mod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
															continue;
														std::vector<BGSMod::Attachment::Mod*>::iterator itMod = find(multipleMod->functionMods.begin(), multipleMod->functionMods.end(), mod);
														if (itMod == multipleMod->functionMods.end())
															multipleMod->functionMods.push_back(mod);
													}
												}
												if (multipleMod->functionMods.size() > 1)
													itKb->modAssociations.push_back(multipleMod);
												else
													delete multipleMod;
											}
										}
										else if (type == 2)
										{
											data2 = modData["modPairs"];
											if (data2.isArray())
											{
												ModPairArray* modPairArray = new ModPairArray;
												modPairArray->funcKeyword = functionKW;
												modPairArray->flags = flags;
												modPairArray->animIdle_1stP = animIdle_1stP;
												modPairArray->animIdle_3rdP = animIdle_3rdP;
												for (int i2 = 0; i2 < data2.size(); i2++)
												{
													str = data2[i2]["baseMod"].asString();
													if (str == "")
														continue;
													BGSMod::Attachment::Mod* baseMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
													if (!baseMod)
														continue;
													if (baseMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
														continue;
													str = data2[i2]["functionMod"].asString();
													if (str == "")
														continue;
													BGSMod::Attachment::Mod* functionMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
													if (!functionMod)
														continue;
													if (functionMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
														continue;
													std::vector<ModAssociationData*>::iterator itData = itKb->modAssociations.begin();
													for (itData; itData != itKb->modAssociations.end(); itData++)
													{
														if ((*itData)->funcKeyword == functionKW)
															break;
														if ((*itData)->GetType() == 1)
														{
															SingleModPair* currModPair = static_cast<SingleModPair*>(*itData);
															if (currModPair->modPair.parentMod == baseMod)
																break;
														}
														else if ((*itData)->GetType() == 3)
														{
															MultipleMod* currMultipleMod = static_cast<MultipleMod*>(*itData);
															if (currMultipleMod->parentMod == baseMod)
																break;
														}
														else if ((*itData)->GetType() == 2)
														{
															ModPairArray* currModPairArray = static_cast<ModPairArray*>(*itData);
															std::vector<ModAssociationData::ModPair>::iterator itModPair = currModPairArray->modPairs.begin();
															for (itModPair; itModPair != currModPairArray->modPairs.end(); itModPair++)
															{
																if (itModPair->parentMod == baseMod)
																	break;
															}
															if (itModPair != currModPairArray->modPairs.end())
																break;
														}
													}
													if (itData == itKb->modAssociations.end())
													{
														std::vector<ModAssociationData::ModPair>::iterator itModPair = modPairArray->modPairs.begin();
														for (itModPair; itModPair != modPairArray->modPairs.end(); itModPair++)
														{
															if (itModPair->parentMod == baseMod)
																break;
														}
														if (itModPair == modPairArray->modPairs.end())
														{
															ModAssociationData::ModPair modPair;
															modPair.parentMod = baseMod;
															modPair.functionMod = functionMod;
															modPairArray->modPairs.push_back(modPair);
														}
													}
												}
												if (modPairArray->modPairs.size() > 1)
													itKb->modAssociations.push_back(modPairArray);
												else
													delete modPairArray;
											}
										}
									}
								}
								break;
							}
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
						std::vector<ReloadAnimData>::iterator itAnim = MSF_MainData::reloadAnimData.begin();
						for (itAnim; itAnim != MSF_MainData::reloadAnimData.end(); itAnim++)
						{
							if (itAnim->uniqueWeap == weapon)
							{
								itAnim->animIdle_1stP = animIdle_1stP;
								itAnim->animIdle_3rdP = animIdle_3rdP;
								break;
							}
						}
						if (itAnim == MSF_MainData::reloadAnimData.end())
						{
							ReloadAnimData anim;
							anim.uniqueWeap = weapon;
							anim.animIdle_1stP = animIdle_1stP;
							anim.animIdle_3rdP = animIdle_3rdP;
							MSF_MainData::reloadAnimData.push_back(anim);
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
						std::vector<FireAnimData>::iterator itAnim = MSF_MainData::fireAnimData.begin();
						for (itAnim; itAnim != MSF_MainData::fireAnimData.end(); itAnim++)
						{
							if (itAnim->uniqueWeap == weapon)
							{
								itAnim->animIdle_1stP = animIdle_1stP;
								itAnim->animIdle_3rdP = animIdle_3rdP;
								break;
							}
						}
						if (itAnim == MSF_MainData::fireAnimData.end())
						{
							FireAnimData anim;
							anim.uniqueWeap = weapon;
							anim.animIdle_1stP = animIdle_1stP;
							anim.animIdle_3rdP = animIdle_3rdP;
							MSF_MainData::fireAnimData.push_back(anim);
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
							if (itData->modeKeyword == keyword)
							{
								itData->displayString = str;
								break;
							}
						}
						if (itData == MSF_MainData::fmDisplayData.end())
						{
							HUDFiringModeData displayData;
							displayData.modeKeyword = keyword;
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
							if (itData->scopeKeyword == keyword)
							{
								itData->displayString = str;
								break;
							}
						}
						if (itData == MSF_MainData::scopeDisplayData.end())
						{
							HUDScopeData displayData;
							displayData.scopeKeyword = keyword;
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
							if (itData->muzzleKeyword == keyword)
							{
								itData->displayString = str;
								break;
							}
						}
						if (itData == MSF_MainData::muzzleDisplayData.end())
						{
							HUDMuzzleData displayData;
							displayData.muzzleKeyword = keyword;
							displayData.displayString = str;
							MSF_MainData::muzzleDisplayData.push_back(displayData);
						}
					}
				}

				return true;
			}
		}
		catch (...)
		{
			_WARNING("Warning - Failed to load Gameplay Data from %s.json", fileName.c_str());
			return false;
		}
		return false;
	}

	//========================= Select Object Mod =======================
	bool GetNthAmmoMod(UInt32 num)
	{
		if (num < 0)
			return false;
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(*g_player, 41);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(stack);
		if (!baseAmmo)
			return false;
		for (std::vector<AmmoData>::iterator itAmmoData = MSF_MainData::ammoData.begin(); itAmmoData != MSF_MainData::ammoData.end(); itAmmoData++)
		{
			if (itAmmoData->baseAmmoData.ammo == baseAmmo)
			{
				if (num == 0)
				{
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
					{
						if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, baseAmmo) == 0)
							return false;
					}
					MSF_MainData::switchData.ModToRemove = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::hasSwitchedAmmoKW);
					MSF_MainData::switchData.ModToAttach = nullptr;
					MSF_MainData::switchData.LooseModToAdd = nullptr;
					MSF_MainData::switchData.LooseModToRemove = nullptr;
					return true;
				}
				num--;
				if ((num + 1) > itAmmoData->ammoMods.size())
					return false;
				AmmoData::AmmoMod* ammoMod = &itAmmoData->ammoMods[num];
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
				{
					if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, ammoMod->ammo) == 0)
						return false;
				}
				MSF_MainData::switchData.ModToAttach = ammoMod->mod;
				MSF_MainData::switchData.ModToRemove = nullptr;
				MSF_MainData::switchData.LooseModToAdd = nullptr;
				MSF_MainData::switchData.LooseModToRemove = nullptr;
				return true;

				//UInt32 index = 1;
				//for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
				//{
				//	if (index == num)
				//	{
				//		if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
				//		{
				//			if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, itAmmoMod->ammo) == 0)
				//				return false;
				//		}
				//		MSF_MainData::switchData.ModToAttach = itAmmoMod->mod;
				//		MSF_MainData::switchData.ModToRemove = nullptr;
				//		MSF_MainData::switchData.LooseModToAdd = nullptr;
				//		MSF_MainData::switchData.LooseModToRemove = nullptr;
				//		return true;
				//	}
				//	index++;
				//}
			}
		}
		return false;
	}

	bool GetNthMod(UInt32 num, std::vector<ModAssociationData*>* modAssociations)
	{
		if (num < 0)
			return false;
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(*g_player, 41);
		if (!stack || !modAssociations)
			return false;
		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return false;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* modData = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!modData || !instanceData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		for (std::vector<ModAssociationData*>::iterator itData = modAssociations->begin(); itData != modAssociations->end(); itData++)
		{
			UInt8 type = (*itData)->GetType();
			if (type == 0x1)
			{
				SingleModPair* modPair = static_cast<SingleModPair*>(*itData);
				if (Utilities::HasObjectMod(modData, modPair->modPair.parentMod))
				{
					if (num == 1)
					{
						MSF_MainData::switchData.ModToAttach = modPair->modPair.functionMod;
						MSF_MainData::switchData.ModToRemove = nullptr;
						MSF_MainData::switchData.LooseModToAdd = nullptr;
						if (modPair->flags & ModAssociationData::bRequireLooseMod)
						{
							TESObjectMISC* looseMod = Utilities::GetLooseMod(modPair->modPair.functionMod);
							if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) == 0)
								return false;
							MSF_MainData::switchData.LooseModToRemove = looseMod;
						}
						else
							MSF_MainData::switchData.LooseModToRemove = nullptr;
						return true;
					}
					else if (num == 0)
					{
						MSF_MainData::switchData.ModToRemove = modPair->modPair.functionMod;
						MSF_MainData::switchData.ModToAttach = nullptr;
						MSF_MainData::switchData.LooseModToRemove = nullptr;
						MSF_MainData::switchData.LooseModToAdd = (modPair->flags & ModAssociationData::bRequireLooseMod) ? Utilities::GetLooseMod(modPair->modPair.functionMod) : nullptr;
						return true;
					}
					return false;
				}
			}
			//else if (type == 0x3)
			//{
			//	MultipleMod* mods = static_cast<MultipleMod*>(*itData);
			//	if (Utilities::HasObjectMod(modData, mods->parentMod))
			//	{
			//		if (num == 0 && (mods->flags & ModAssociationData::bCanHaveNullMod))
			//		{
			//			MSF_MainData::switchData.ModToRemove = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), mods->funcKeyword);
			//			if (!MSF_MainData::switchData.ModToRemove)
			//				return false;
			//			MSF_MainData::switchData.ModToAttach = nullptr;
			//			MSF_MainData::switchData.LooseModToRemove = nullptr;
			//			MSF_MainData::switchData.LooseModToAdd = (mods->flags & ModAssociationData::bRequireLooseMod) ? Utilities::GetLooseMod(MSF_MainData::switchData.ModToRemove) : nullptr;
			//			return true;
			//		}
			//		if (mods->flags & ModAssociationData::bCanHaveNullMod)
			//			num--;
			//		if ((num + 1) > mods->functionMods.size())
			//			return false;
			//		MSF_MainData::switchData.ModToAttach = mods->functionMods[num];
			//		MSF_MainData::switchData.ModToRemove = nullptr;
			//		MSF_MainData::switchData.LooseModToAdd = nullptr;
			//		MSF_MainData::switchData.LooseModToRemove = (mods->flags & ModAssociationData::bRequireLooseMod) ? Utilities::GetLooseMod(MSF_MainData::switchData.ModToAttach) : nullptr;
			//		return true;
			//	}
			//}
			//else if (type == 0x2)
			//{
			//	ModPairArray* mods = static_cast<ModPairArray*>(*itData);
			//	if (num == 0 && (mods->flags & ModAssociationData::bCanHaveNullMod))
			//	{
			//		MSF_MainData::switchData.ModToRemove = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), mods->funcKeyword);
			//		if (!MSF_MainData::switchData.ModToRemove)
			//			return false;
			//		MSF_MainData::switchData.ModToAttach = nullptr;
			//		MSF_MainData::switchData.LooseModToRemove = nullptr;
			//		MSF_MainData::switchData.LooseModToAdd = (mods->flags & ModAssociationData::bRequireLooseMod) ? Utilities::GetLooseMod(MSF_MainData::switchData.ModToRemove) : nullptr;
			//		return true;
			//	}
			//	if (mods->flags & ModAssociationData::bCanHaveNullMod)
			//		num--;
			//	if ((num + 1) > mods->modPairs.size())
			//		return false;
			//	ModAssociationData::ModPair* modPair = &mods->modPairs[num];
			//	if (!Utilities::HasObjectMod(modData, modPair->parentMod))
			//		return false;
			//	MSF_MainData::switchData.ModToAttach = modPair->functionMod;
			//	MSF_MainData::switchData.ModToRemove = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), mods->funcKeyword);
			//	if (mods->flags & ModAssociationData::bRequireLooseMod)
			//	{
			//		MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(MSF_MainData::switchData.ModToRemove);
			//		MSF_MainData::switchData.LooseModToRemove = Utilities::GetLooseMod(MSF_MainData::switchData.ModToAttach);
			//	}
			//	else
			//	{
			//		MSF_MainData::switchData.LooseModToAdd = nullptr;
			//		MSF_MainData::switchData.LooseModToRemove = nullptr;
			//	}
			//	return true;
			//}
		}
		return false;
	}

	bool PickRandomMods(tArray<BGSMod::Attachment::Mod*>* mods, TESAmmo** ammo, UInt32* count)
	{
		TESAmmo* baseAmmo = *ammo;
		*ammo = nullptr;
		UInt32 chance = 0;
		if (baseAmmo)
		{
			for (std::vector<AmmoData>::iterator itAmmoData = MSF_MainData::ammoData.begin(); itAmmoData != MSF_MainData::ammoData.end(); itAmmoData++)
			{
				if (itAmmoData->baseAmmoData.ammo == baseAmmo)
				{
					chance += itAmmoData->baseAmmoData.spawnChance;
					UInt32 _chance = 0;
					for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
					{
						_chance += itAmmoMod->spawnChance;
					}
					if (_chance == 0)
						break;
					chance += _chance - 1;
					UInt32 picked = MSF_MainData::rng.RandomInt(0, chance - 1);
					_chance = itAmmoData->baseAmmoData.spawnChance - 1;
					if (picked <= _chance)
						break;
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
					break;
				}
			}
		}
		// mod association



		return false;
	}

	bool GetNextMod(BGSInventoryItem::Stack* eqStack, std::vector<ModAssociationData*>* modAssociations)
	{
		if (!eqStack || !modAssociations)
			return false;
		ExtraDataList* dataList = eqStack->extraData;
		if (!dataList)
			return false;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		BGSObjectInstanceExtra* modData = DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!modData || !instanceData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		_MESSAGE("checkOK");
		for (std::vector<ModAssociationData*>::iterator itData = modAssociations->begin(); itData != modAssociations->end(); itData++) //HasMod->next
		{
			UInt8 type = (*itData)->GetType();
			_MESSAGE("type: %i", type);
			if (type == 0x1)
			{
				SingleModPair* modPair = static_cast<SingleModPair*>(*itData);
				if (Utilities::HasObjectMod(modData, modPair->modPair.parentMod))
				{
					if (Utilities::HasObjectMod(modData, modPair->modPair.functionMod))
					{
						MSF_MainData::switchData.ModToRemove = modPair->modPair.functionMod;
						MSF_MainData::switchData.ModToAttach = nullptr;
						MSF_MainData::switchData.LooseModToRemove = nullptr;
						if (modPair->flags & ModAssociationData::bRequireLooseMod)
							MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(modPair->modPair.functionMod);
						else
							MSF_MainData::switchData.LooseModToAdd = nullptr;
					}
					else
					{
						if (modPair->flags & ModAssociationData::bRequireLooseMod)
						{
							TESObjectMISC* looseMod = Utilities::GetLooseMod(modPair->modPair.functionMod);
							if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
							{
								MSF_MainData::switchData.ModToAttach = modPair->modPair.functionMod;
								MSF_MainData::switchData.ModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToRemove = looseMod;
								MSF_MainData::switchData.LooseModToAdd = nullptr;
							}
							else
								return false; //Utilities::SendNotification("You lack the corresponding loose mod to apply this modification.");
						}
						else
						{
							MSF_MainData::switchData.ModToAttach = modPair->modPair.functionMod;
							MSF_MainData::switchData.ModToRemove = nullptr;
							MSF_MainData::switchData.LooseModToRemove = nullptr;
							MSF_MainData::switchData.LooseModToAdd = nullptr;
						}
					}
					MSF_MainData::switchData.SwitchFlags |= (modPair->flags & ModAssociationData::mBitTransferMask);
					MSF_MainData::switchData.AnimToPlay1stP = modPair->animIdle_1stP;
					MSF_MainData::switchData.AnimToPlay3rdP = modPair->animIdle_3rdP;
					return true;
				}
			}
			else if (type == 0x3)
			{
				MultipleMod* mods = static_cast<MultipleMod*>(*itData);
				if (Utilities::HasObjectMod(modData, mods->parentMod))
				{
					_MESSAGE("parentOK");
					if (Utilities::WeaponInstanceHasKeyword(instanceData, mods->funcKeyword))
					{
						_MESSAGE("hasKW");
						BGSMod::Attachment::Mod* oldMod = Utilities::FindModByUniqueKeyword(modData, mods->funcKeyword);
						std::vector<BGSMod::Attachment::Mod*>::iterator itMod = find(mods->functionMods.begin(), mods->functionMods.end(), oldMod);
						BGSMod::Attachment::Mod* currMod = *itMod;
						itMod++;
						if (itMod != mods->functionMods.end())
						{
							_MESSAGE("hasOldModNotEnd");
							if (mods->flags & ModAssociationData::bRequireLooseMod)
							{
								TESObjectMISC* looseMod;
								std::vector<BGSMod::Attachment::Mod*>::iterator itHelper = itMod;
								for (itMod; itMod != mods->functionMods.end(); itMod++)
								{
									looseMod = Utilities::GetLooseMod(*itMod);
									if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
										break;
								}
								if (itMod != mods->functionMods.end())
								{
									MSF_MainData::switchData.ModToAttach = *itMod;
									MSF_MainData::switchData.ModToRemove = nullptr;
									MSF_MainData::switchData.LooseModToRemove = looseMod;
									MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(currMod);
								}
								else if (mods->flags & ModAssociationData::bCanHaveNullMod)
								{
									MSF_MainData::switchData.ModToRemove = currMod;
									MSF_MainData::switchData.ModToAttach = nullptr;
									MSF_MainData::switchData.LooseModToRemove = nullptr;
									MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(currMod);
								}
								else
								{
									for (itMod = mods->functionMods.begin(); itMod != itHelper; itMod++)
									{
										looseMod = Utilities::GetLooseMod(*itMod);
										if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
											break;
									}
									if (itMod != itHelper)
									{
										MSF_MainData::switchData.ModToAttach = *itMod;
										MSF_MainData::switchData.ModToRemove = nullptr;
										MSF_MainData::switchData.LooseModToRemove = looseMod;
										MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(currMod);
									}
									else
										return false; //Utilities::SendNotification("You lack the corresponding loose mod to apply this modification.");
								}
							}
							else
							{
								MSF_MainData::switchData.ModToAttach = *itMod;
								MSF_MainData::switchData.ModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToAdd = nullptr;
							}
						}
						else
						{
							if (mods->flags & ModAssociationData::bCanHaveNullMod)
							{
								MSF_MainData::switchData.ModToRemove = currMod;
								MSF_MainData::switchData.ModToAttach = nullptr;
								MSF_MainData::switchData.LooseModToRemove = nullptr;
								if (mods->flags & ModAssociationData::bRequireLooseMod)
									MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(currMod);
								else
									MSF_MainData::switchData.LooseModToAdd = nullptr;
							}
							else if (mods->flags & ModAssociationData::bRequireLooseMod)
							{
								itMod--;
								TESObjectMISC* looseMod;
								std::vector<BGSMod::Attachment::Mod*>::iterator itMod2 = mods->functionMods.begin();
								for (itMod2; itMod2 != itMod; itMod2++)
								{
									looseMod = Utilities::GetLooseMod(*itMod2);
									if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
										break;
								}
								if (itMod2 != itMod)
								{
									MSF_MainData::switchData.ModToAttach = *itMod2;
									MSF_MainData::switchData.ModToRemove = nullptr;
									MSF_MainData::switchData.LooseModToRemove = looseMod;
									MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(currMod);
								}
								else
									return false; //Utilities::SendNotification("You lack the corresponding loose mod to apply this modification.");
							}
							else
							{
								MSF_MainData::switchData.ModToAttach = mods->functionMods[0];
								MSF_MainData::switchData.ModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToAdd = nullptr;
							}
						}
					}
					else
					{
						if (mods->flags & ModAssociationData::bRequireLooseMod)
						{
							TESObjectMISC* looseMod;
							std::vector<BGSMod::Attachment::Mod*>::iterator itMod = mods->functionMods.begin();
							for (itMod; itMod != mods->functionMods.end(); itMod++)
							{
								looseMod = Utilities::GetLooseMod(*itMod);
								if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
									break;
							}
							if (itMod != mods->functionMods.end())
							{
								MSF_MainData::switchData.ModToAttach = *itMod;
								MSF_MainData::switchData.ModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToRemove = looseMod;
								MSF_MainData::switchData.LooseModToAdd = nullptr;
							}
							else
								return false; //Utilities::SendNotification("You lack the corresponding loose mod to apply this modification.");
						}
						else
						{
							MSF_MainData::switchData.ModToAttach = mods->functionMods[0];
							MSF_MainData::switchData.ModToRemove = nullptr;
							MSF_MainData::switchData.LooseModToRemove = nullptr;
							MSF_MainData::switchData.LooseModToAdd = nullptr;
						}
					}
					MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
					MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
					MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
					return true;
				}
			}
			else if (type == 0x2)
			{
				ModPairArray* mods = static_cast<ModPairArray*>(*itData);
				if (Utilities::WeaponInstanceHasKeyword(instanceData, mods->funcKeyword))
				{
					_MESSAGE("hasOldMod");
					BGSMod::Attachment::Mod* oldMod = Utilities::FindModByUniqueKeyword(modData, mods->funcKeyword);
					std::vector<ModAssociationData::ModPair>::iterator itHelper = mods->modPairs.begin();
					bool selectNext = false;
					for (std::vector<ModAssociationData::ModPair>::iterator itMod = mods->modPairs.begin(); itMod != mods->modPairs.end(); itMod++)
					{
						if (selectNext)
						{
							if (Utilities::HasObjectMod(modData, itMod->parentMod))
							{
								_MESSAGE("hasParent: %08X", itMod->parentMod->formID);
								if (mods->flags & ModAssociationData::bRequireLooseMod)
								{
									TESObjectMISC* looseMod = Utilities::GetLooseMod(itMod->functionMod);
									if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
									{
										MSF_MainData::switchData.LooseModToRemove = looseMod;
										MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(oldMod);
									}
									else
										continue;
								}
								else
								{
									MSF_MainData::switchData.LooseModToRemove = nullptr;
									MSF_MainData::switchData.LooseModToAdd = nullptr;
								}
								MSF_MainData::switchData.ModToAttach = itMod->functionMod;
								MSF_MainData::switchData.ModToRemove = oldMod;
								MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
								MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
								MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
								return true;
							}
						}
						else if (itMod->functionMod == oldMod)
						{
							selectNext = true;
							itHelper = itMod;
						}
					}
					if (mods->flags & ModAssociationData::bCanHaveNullMod)
					{
						_MESSAGE("NullMod");
						MSF_MainData::switchData.ModToAttach = nullptr;
						MSF_MainData::switchData.ModToRemove = oldMod;
						MSF_MainData::switchData.LooseModToRemove = nullptr;
						MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
						MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
						MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
						if (mods->flags & ModAssociationData::bRequireLooseMod)
							MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(oldMod);
						else
							MSF_MainData::switchData.LooseModToAdd = nullptr;
						return true;
					}
					for (std::vector<ModAssociationData::ModPair>::iterator itMod = mods->modPairs.begin(); itMod != itHelper; itMod++)
					{
						if (Utilities::HasObjectMod(modData, itMod->parentMod))
						{
							if (mods->flags & ModAssociationData::bRequireLooseMod)
							{
								TESObjectMISC* looseMod = Utilities::GetLooseMod(itMod->functionMod);
								if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
								{
									MSF_MainData::switchData.LooseModToRemove = looseMod;
									MSF_MainData::switchData.LooseModToAdd = Utilities::GetLooseMod(oldMod);
								}
								else
									continue;
							}
							else
							{
								MSF_MainData::switchData.LooseModToRemove = nullptr;
								MSF_MainData::switchData.LooseModToAdd = nullptr;
							}
							MSF_MainData::switchData.ModToAttach = itMod->functionMod;
							MSF_MainData::switchData.ModToRemove = oldMod;
							MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
							MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
							MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
							return true;
						}
					}
				}
				else
				{
					for (std::vector<ModAssociationData::ModPair>::iterator itMod = mods->modPairs.begin(); itMod != mods->modPairs.end(); itMod++)
					{
						_MESSAGE("itP");
						if (Utilities::HasObjectMod(modData, itMod->parentMod))
						{
							_MESSAGE("parentOK");
							if (mods->flags & ModAssociationData::bRequireLooseMod)
							{
								TESObjectMISC* looseMod = Utilities::GetLooseMod(itMod->functionMod);
								if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, looseMod) != 0)
									MSF_MainData::switchData.LooseModToRemove = looseMod;
								else
									continue;
							}
							else
								MSF_MainData::switchData.LooseModToRemove = nullptr;
							MSF_MainData::switchData.ModToAttach = itMod->functionMod;
							MSF_MainData::switchData.ModToRemove = nullptr;
							MSF_MainData::switchData.LooseModToAdd = nullptr;
							MSF_MainData::switchData.SwitchFlags |= (mods->flags & ModAssociationData::mBitTransferMask);
							MSF_MainData::switchData.AnimToPlay1stP = mods->animIdle_1stP;
							MSF_MainData::switchData.AnimToPlay3rdP = mods->animIdle_3rdP;
							return true;
						}
					}
				}
			}
		}
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

	//========================= Animations =======================
	TESIdleForm* GetReloadAnimation(TESObjectWEAP* equippedWeap, bool get3rdP)
	{
		for (std::vector<ReloadAnimData>::iterator it = MSF_MainData::reloadAnimData.begin(); it != MSF_MainData::reloadAnimData.end(); it++)
		{
			if (it->uniqueWeap == equippedWeap)
			{
				if (get3rdP)
					return it->animIdle_3rdP;
				else
					return it->animIdle_1stP;
			}
		}
		if (get3rdP)
			return MSF_MainData::reloadIdle3rdP;
		else
			return MSF_MainData::reloadIdle1stP;
	}

	TESIdleForm* GetFireAnimation(TESObjectWEAP* equippedWeap, bool get3rdP)
	{
		for (std::vector<FireAnimData>::iterator it = MSF_MainData::fireAnimData.begin(); it != MSF_MainData::fireAnimData.end(); it++)
		{
			if (it->uniqueWeap == equippedWeap)
			{
				if (get3rdP)
					return it->animIdle_3rdP;
				else
					return it->animIdle_1stP;
			}
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
			if (!it->modeKeyword)
				continue;
			if (Utilities::WeaponInstanceHasKeyword(instanceData, it->modeKeyword))
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
			if (!it->scopeKeyword)
				continue;
			if (Utilities::WeaponInstanceHasKeyword(instanceData, it->scopeKeyword))
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
			if (!it->muzzleKeyword)
				continue;
			if (Utilities::WeaponInstanceHasKeyword(instanceData, it->muzzleKeyword))
			{
				return it->displayString;
			}
		}
		return "";
	}

	KeybindData* GetKeyFunctionID(UInt16 keyCode, UInt8 modifiers)
	{
		std::vector<KeybindData>::iterator it;
		it = std::find_if(MSF_MainData::keybinds.begin(), MSF_MainData::keybinds.end(), [keyCode, modifiers](KeybindData const& data){
		return data.keyCode == keyCode && data.modifiers == modifiers;
		} );
		if (it != MSF_MainData::keybinds.end())
			return it._Ptr;
		return nullptr;
		/*for (std::vector<KeybindData>::iterator it = MSF_MainData::keybinds.begin(); it != MSF_MainData::keybinds.end(); it++)
		{
			if (it->keyCode == keyCode)
			{
				if (it->modifiers == modifiers)
					return it._Ptr;
			}
		}
		return nullptr;*/
	}
}

