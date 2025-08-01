#include "MSF_Data.h"
#include "MSF_Addresses.h"
#include "MSF_Scaleform.h"
#include "MSF_Events.h"
#include "MSF_WeaponState.h"
#include "rva/Hook/HookUtil.h"
#include "json/json.h"
#include "RNG.h"
#include <fstream>
#include <random>
#include <algorithm>

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
BGSKeyword* MSF_MainData::tacticalReloadKW = nullptr;
ActorValueInfo* MSF_MainData::BCR_AVIF;
ActorValueInfo* MSF_MainData::BCR_AVIF2;
BGSMod::Attachment::Mod* MSF_MainData::APbaseMod;
BGSMod::Attachment::Mod* MSF_MainData::NullMuzzleMod;
BGSProjectile* MSF_MainData::ProjectileDummy;
BGSKeyword* MSF_MainData::CanHaveNullMuzzleKW;
BGSKeyword* MSF_MainData::FiringModBurstKW;
BGSKeyword* MSF_MainData::FiringModeUnderbarrelKW;
BGSKeyword* MSF_MainData::BallisticWeaponKW;
BGSKeyword* MSF_MainData::MineKW;
BGSKeyword* MSF_MainData::GrenadeKW;
BGSKeyword* MSF_MainData::UnarmedKW;
BGSKeyword* MSF_MainData::Melee1HKW;
BGSKeyword* MSF_MainData::Melee2HKW;
BGSKeyword* MSF_MainData::IsMagKW;
BGSKeyword* MSF_MainData::AnimsEmptyBeforeReloadKW;
BGSKeyword* MSF_MainData::FusionCoreKW;
TESIdleForm* MSF_MainData::reloadIdle1stP;
TESIdleForm* MSF_MainData::reloadIdle3rdP;
TESIdleForm* MSF_MainData::fireIdle1stP;
TESIdleForm* MSF_MainData::fireIdle3rdP;
BGSAction* MSF_MainData::ActionFireSingle;
BGSAction* MSF_MainData::ActionFireAuto;
BGSAction* MSF_MainData::ActionReload;
BGSAction* MSF_MainData::ActionDraw;
BGSAction* MSF_MainData::ActionGunDown;
BGSAction* MSF_MainData::ActionRightRelease;
bool MSF_MainData::GameIsLoading = true;
bool MSF_MainData::IsInitialized = false;
int MSF_MainData::iCheckDelayMS = 10;
UInt32 MSF_MainData::MCMSettingFlags = 0;
UInt16 MSF_MainData::iMinRandomAmmo = 5;
UInt16 MSF_MainData::iMaxRandomAmmo = 50;
UInt16 MSF_MainData::iAutolowerTimeSec = 0;
UInt16 MSF_MainData::iReloadAnimEndEventDelayMS = 500;
UInt16 MSF_MainData::iCustomAnimEndEventDelayMS = 100;
UInt16 MSF_MainData::iDrawAnimEndEventDelayMS = 500;
GFxMovieRoot* MSF_MainData::MSFMenuRoot = nullptr;
ModSelectionMenu* MSF_MainData::widgetMenu = nullptr;
HUDMenuAmmoDisplay* MSF_MainData::ammoDisplay = nullptr;

BurstModeManager* MSF_MainData::activeBurstManager = nullptr;

BCRinterface MSF_MainData::BCRinterfaceHolder;
std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*> MSF_MainData::compatibilityEdits;
std::unordered_multimap<BGSMod::Attachment::Mod*, KeywordValue> MSF_MainData::instantiationRequirements;
ModSwitchManager MSF_MainData::modSwitchManager;
CustomProjectileFormManager MSF_MainData::projectileManager;
PlayerInventoryListEventSink MSF_MainData::playerInventoryEventSink;
ActorEquipManagerEventSink MSF_MainData::actorEquipManagerEventSink;
WeaponStateStore MSF_MainData::weaponStateStore;
KeywordValue MSF_MainData::ammoAP = -1;
KeywordValue MSF_MainData::magAP = -1;
KeywordValue MSF_MainData::receiverAP = -1;
KeywordValue MSF_MainData::muzzleAP = -1;
UInt64 MSF_MainData::lowerWeaponHotkey = 0;
UInt64 MSF_MainData::cancelSwitchHotkey = 0;
UInt64 MSF_MainData::DEBUGprintStoredDataHotkey = 0;
std::unordered_map<BGSMod::Attachment::Mod*, BurstModeData*>  MSF_MainData::burstModeData;
std::unordered_map<BGSMod::Attachment::Mod*, ModData::Mod*> MSF_MainData::modDataMap;
std::unordered_map<BGSMod::Attachment::Mod*, AmmoData::AmmoMod*> MSF_MainData::ammoModMap;
std::unordered_map<TESAmmo*, AmmoData::AmmoMod*> MSF_MainData::ammoMap;
std::unordered_map<BGSMod::Attachment::Mod*, ChamberData> MSF_MainData::modChamberMap;
std::vector<KeywordValue> MSF_MainData::uniqueStateAPValues;
std::vector<TESAmmo*> MSF_MainData::dontRemoveAmmoOnReload;
std::vector<TESObjectWEAP*>  MSF_MainData::BCRweapons;
Utilities::Timer MSF_MainData::lowerTmr;

RandomNumber MSF_MainData::rng;

void ModSwitchManager::HandlePAEvent()
{
	UInt16 inPA = IsInPowerArmor(*g_player);
	UInt16 wasInPA = InterlockedExchange16((volatile short*)&isInPA, inPA);
	if (inPA != wasInPA)
		MSF_Scaleform::UpdateWidgetData();
}

namespace MSF_Data
{
	bool InjectAttachPoints()
	{
		if (MSF_MainData::ammoAP == -1)
			return false;
		tArray<TESObjectWEAP*>* weaps = &(*g_dataHandler)->arrWEAP;
		for (UInt64 idx = 0; idx < weaps->count; idx++)
		{
			TESObjectWEAP* weapForm = nullptr;
			weaps->GetNthItem(idx, weapForm);
			AttachParentArray* aps = (AttachParentArray*)&weapForm->attachParentArray;
			aps->kewordValueArray.Push(MSF_MainData::ammoAP);
		}
		return true;
	}

	bool InjectLeveledLists()
	{
		if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bInjectLeveledLists))
			return false;
		for (std::unordered_map<TESAmmo*, AmmoData*>::iterator it = MSF_MainData::ammoDataMap.begin(); it != MSF_MainData::ammoDataMap.end(); it++)
		{
			AmmoData* itAmmoData = it->second;
			if (!itAmmoData || !itAmmoData->baseAmmoLL)
				continue;
			LevItem* llForm = itAmmoData->baseAmmoLL;
			_DEBUG("llForm: %p, formID: %08X; flags: %08X; llfalgs: %02X; formType: %02X; baseListCount: %i", llForm, llForm->formID, llForm->flags, llForm->llFlags, llForm->formType, llForm->baseListCount);
			std::vector<double> chances;
			double totalChance = itAmmoData->baseAmmoData.spawnChance;
			chances.push_back(itAmmoData->baseAmmoData.spawnChance);
			for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin(); itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
			{
				chances.push_back(itAmmoMod->spawnChance);
				totalChance += itAmmoMod->spawnChance;
			}
			if (totalChance == itAmmoData->baseAmmoData.spawnChance)
				continue;
			if (itAmmoData->baseAmmoData.spawnChance == 0)
			{
				llForm->baseListCount = 0;
				llForm->leveledLists = nullptr;
				llForm->scriptListCount = 0;
				llForm->scriptAddedLists = nullptr;
				llForm->chanceNone = 0;
				llForm->chanceGlobal = nullptr;
				llForm->llFlags = 3;
				std::vector<LEVELED_OBJECT> typeLLObjs;
				//std::string typedNumStr = "";
				for (auto itAmmoMod : itAmmoData->ammoMods)
				{
					if (itAmmoMod.spawnChance == 0)
					{
						//typedNumStr += ", 0";
						continue;
					}
					LevItem* copiedForType = (LevItem*)CreateDuplicateForm(llForm, true, nullptr);
					copiedForType->CopyData(llForm, true, true);
					for (uint32_t i = 0; i < static_cast<uint32_t>(copiedForType->baseListCount); i++)
						copiedForType->leveledLists[i].form = itAmmoMod.ammo;
					float typeChance = itAmmoMod.spawnChance / (totalChance - itAmmoData->baseAmmoData.spawnChance) * 40;
					UInt8 chanceNum = round(typeChance);
					typeLLObjs.insert(typeLLObjs.end(), chanceNum, LEVELED_OBJECT(copiedForType));
					//typedNumStr += ", "+ std::to_string(chanceNum);
				}
				llForm->CreateList(typeLLObjs);
				continue;
			}
			for (UInt8 typepcs = 1; typepcs < 42; typepcs++)
			{
				float totPcs = typepcs / (1 - (itAmmoData->baseAmmoData.spawnChance / totalChance));
				UInt32 totPcsR = round(totPcs);
				float distErr = abs(1 - typepcs / totPcsR - itAmmoData->baseAmmoData.spawnChance / totalChance);
				if (distErr <= 0.01 || totPcsR > 40)
				{
					if (totPcsR > 40)
						totPcsR = 40;
					LevItem* copiedBase = (LevItem*)CreateDuplicateForm(llForm, true, nullptr);
					copiedBase->CopyData(llForm, true, false);
					llForm->baseListCount = 0;
					llForm->leveledLists = nullptr;
					llForm->scriptListCount = 0;
					llForm->scriptAddedLists = nullptr;
					llForm->chanceNone = 0;
					llForm->chanceGlobal = nullptr;
					llForm->llFlags = 3;
					std::vector<LEVELED_OBJECT> typeLLObjs;
					//std::string typedNumStr = "";
					for (auto itAmmoMod : itAmmoData->ammoMods)
					{
						if (itAmmoMod.spawnChance == 0)
						{
							//typedNumStr += ", 0";
							continue;
						}
						LevItem* copiedForType = (LevItem*)CreateDuplicateForm(copiedBase, true, nullptr);
						copiedForType->CopyData(copiedBase, true, true);
						for (uint32_t i = 0; i < static_cast<uint32_t>(copiedForType->baseListCount); i++)
							copiedForType->leveledLists[i].form = itAmmoMod.ammo;
						float typeChance = itAmmoMod.spawnChance / (totalChance - itAmmoData->baseAmmoData.spawnChance) * 40;
						UInt8 chanceNum = round(typeChance);
						typeLLObjs.insert(typeLLObjs.end(), chanceNum, LEVELED_OBJECT(copiedForType));
						//typedNumStr += ", "+ std::to_string(chanceNum);
					}
					LevItem* copiedForCollective = (LevItem*)CreateDuplicateForm(llForm, true, nullptr);
					copiedForCollective->CreateList(typeLLObjs);
					std::vector<LEVELED_OBJECT> finalLLObjs;
					finalLLObjs.insert(finalLLObjs.end(), totPcsR-typepcs, LEVELED_OBJECT(copiedBase));
					finalLLObjs.insert(finalLLObjs.end(), typepcs, LEVELED_OBJECT(copiedForCollective));
					llForm->CreateList(finalLLObjs);
					//_DEBUG("replaced w/ %i original and %i typed; type dist: %s", totPcsR - typepcs, typepcs, typedNumStr.c_str());
					break;
				}
			}
		}
		return true;
	}

	bool InitData()
	{
		UInt32 formIDbase = 0;
		UInt16 eslModIndex = (*g_dataHandler)->GetLoadedLightModIndex(MODNAME);
		if (eslModIndex != 0xFFFF)
		{
			formIDbase = 0xFE000000 | (UInt32(eslModIndex) << 12);
		}
		else
			return false;

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
		BGSKeyword* magAPkeyword = (BGSKeyword*)LookupFormByID((UInt32)0x005D4D7);
		MSF_MainData::magAP = Utilities::GetAttachValueForTypedKeyword(magAPkeyword);
		BGSKeyword* receiverAPkeyword = (BGSKeyword*)LookupFormByID((UInt32)0x0024004);
		MSF_MainData::receiverAP = Utilities::GetAttachValueForTypedKeyword(receiverAPkeyword);
		BGSKeyword* muzzleAPkeyword = (BGSKeyword*)LookupFormByID((UInt32)0x002249C);
		MSF_MainData::muzzleAP = Utilities::GetAttachValueForTypedKeyword(muzzleAPkeyword);

		MSF_MainData::NullMuzzleMod = (BGSMod::Attachment::Mod*)LookupFormByID((UInt32)0x004F21D);
		MSF_MainData::CanHaveNullMuzzleKW = (BGSKeyword*)LookupFormByID((UInt32)0x01C9E78);
		MSF_MainData::BallisticWeaponKW = (BGSKeyword*)LookupFormByID((UInt32)0x0092A86);
		MSF_MainData::MineKW = (BGSKeyword*)LookupFormByID((UInt32)0x010C414);
		MSF_MainData::GrenadeKW = (BGSKeyword*)LookupFormByID((UInt32)0x010C415);
		MSF_MainData::UnarmedKW = (BGSKeyword*)LookupFormByID((UInt32)0x0226453);
		MSF_MainData::Melee1HKW = (BGSKeyword*)LookupFormByID((UInt32)0x004A0A4);
		MSF_MainData::Melee2HKW = (BGSKeyword*)LookupFormByID((UInt32)0x004A0A5);
		MSF_MainData::FiringModBurstKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000024);
		MSF_MainData::FiringModeUnderbarrelKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000021);
		MSF_MainData::hasUniqueStateKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000001);
		MSF_MainData::IsMagKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000002);
		MSF_MainData::AnimsEmptyBeforeReloadKW = (BGSKeyword*)LookupFormByID(formIDbase | (UInt32)0x0000003);
		MSF_MainData::ProjectileDummy = (BGSProjectile*)LookupFormByID(formIDbase | (UInt32)0x0000004);
		MSF_MainData::fireIdle1stP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0004AE1));
		MSF_MainData::fireIdle3rdP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0018E1F));
		MSF_MainData::reloadIdle1stP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x0004D33));
		MSF_MainData::reloadIdle3rdP = reinterpret_cast<TESIdleForm*>(LookupFormByID((UInt32)0x00BDDA6));
		MSF_MainData::ActionFireSingle = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A5A));
		MSF_MainData::ActionFireAuto = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A5C));
		MSF_MainData::ActionReload = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0004A56));
		MSF_MainData::ActionDraw = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x00132AF));
		MSF_MainData::ActionGunDown = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0022A35));
		MSF_MainData::ActionRightRelease = reinterpret_cast<BGSAction*>(LookupFormByID((UInt32)0x0013454));
		MSF_MainData::BCR_AVIF = reinterpret_cast<ActorValueInfo*>(LookupFormByID((UInt32)0x0000300)); 
		MSF_MainData::BCR_AVIF2 = reinterpret_cast<ActorValueInfo*>(LookupFormByID((UInt32)0x00002FC));
		MSF_MainData::FusionCoreKW = reinterpret_cast<BGSKeyword*>(LookupFormByID((UInt32)0x005BDAA)); //1025AE

		UInt8 tacticalReloadModIndex = (*g_dataHandler)->GetLoadedModIndex("TacticalReload.esm");
		if (tacticalReloadModIndex != 0xFF)
		{
			UInt32 TRformIDbase = ((UInt32)tacticalReloadModIndex) << 24;
			MSF_MainData::tacticalReloadKW = (BGSKeyword*)LookupFormByID(TRformIDbase | (UInt32)0x0001734);
		}

		if (!ammoApkeyword || !MSF_MainData::hasUniqueStateKW || MSF_MainData::ammoAP == -1)
			return false;

		return InjectAttachPoints();
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

	bool FillQuickAccessData()
	{
		for (std::unordered_map<TESAmmo*, AmmoData*>::iterator it = MSF_MainData::ammoDataMap.begin(); it != MSF_MainData::ammoDataMap.end(); it++)
		{
			AmmoData* itAmmoData = it->second;
			if (!itAmmoData)
				continue;
			MSF_MainData::ammoMap[itAmmoData->baseAmmoData.ammo] = &itAmmoData->baseAmmoData;
			BGSMod::Attachment::Mod* baseMod = itAmmoData->baseAmmoData.mod;
			if (baseMod)
				MSF_MainData::ammoModMap[baseMod] = &itAmmoData->baseAmmoData;
			for (std::vector<AmmoData::AmmoMod>::iterator itAmmo = itAmmoData->ammoMods.begin(); itAmmo != itAmmoData->ammoMods.end(); itAmmo++)
			{
				MSF_MainData::ammoModMap[itAmmo->mod] = itAmmo._Ptr;
				MSF_MainData::ammoMap[itAmmo->ammo] = itAmmo._Ptr;
			}
		}
		for (std::unordered_map<std::string, KeybindData*>::iterator it = MSF_MainData::keybindIDMap.begin(); it != MSF_MainData::keybindIDMap.end(); it++)
		{
			KeybindData* itKb = it->second;
			if (!itKb)
				continue;
			if (!itKb->modData)
				continue;
			BGSKeyword* kw = nullptr;
			for (std::unordered_map<KeywordValue, ModData::ModCycle*>::iterator itCycles = itKb->modData->modCycleMap.begin(); itCycles != itKb->modData->modCycleMap.end(); itCycles++)
			{
				ModData::ModCycle* cycle = itCycles->second;
				for (std::vector<ModData::Mod*>::iterator itData = cycle->mods.begin(); itData != cycle->mods.end(); itData++)
				{
					ModData::Mod* mod = *itData;
					if (mod->mod)
						MSF_MainData::modDataMap[mod->mod] = mod;
				}
			}
		}
		return true;
	}

	bool InitMCMSettings()
	{
		if (!ReadMCMKeybindData())
			return false;

		MSF_MainData::MCMSettingFlags = (MSF_MainData::bReloadEnabled | MSF_MainData::bCustomAnimEnabled | MSF_MainData::bAmmoRequireWeaponToBeDrawn | MSF_MainData::bRequireAmmoToSwitch\
			| MSF_MainData::bSpawnRandomAmmo | MSF_MainData::bSpawnRandomMods | MSF_MainData::bInjectLeveledLists | MSF_MainData::bWidgetAlwaysVisible | MSF_MainData::bShowAmmoIcon | MSF_MainData::bShowMuzzleIcon | MSF_MainData::bShowAmmoName \
			| MSF_MainData::bShowMuzzleName | MSF_MainData::bShowFiringMode | MSF_MainData::bShowScopeData | MSF_MainData::bShowUnavailableMods | MSF_MainData::bEnableMetadataSaving \
			| MSF_MainData::bEnableAmmoSaving | MSF_MainData::bEnableTacticalReloadAnim | MSF_MainData::bEnableBCRSupport | MSF_MainData::bDisplayChamberedAmmoOnHUD | MSF_MainData::bDisplayMagInPipboy \
			| MSF_MainData::bDisplayChamberInPipboy);


		AddFloatSetting("fBaseChanceMultiplier", 1.0);

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

	void ClearInternalKeybinds()
	{
		MSF_MainData::cancelSwitchHotkey = 0;
		MSF_MainData::lowerWeaponHotkey = 0;
		MSF_MainData::DEBUGprintStoredDataHotkey = 0;
		MSF_MainData::keybindMap.clear();
	}

	void ClearNativeMSFKeybindsInMCM()
	{
	}

	void ClearAllMSFKeybindsInMCM()
	{
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

				if (!reader.good())
				{
					std::string err = reader.getFormattedErrorMessages();
					_ERROR("JSON data parser: Failed to read MCM keybind data. Detailed error message:");
					_MESSAGE("%s", err.c_str());
					return false;
				}

				if (json["version"].asInt() < 1)
					return false;

				keybinds = json["keybinds"];
				if (!keybinds.isArray())
					return false;

				ClearInternalKeybinds();

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
					else if (id == "MSF_DATA")
					{
						UInt64 keyCode = keybind["keycode"].asInt();
						UInt64 modifiers = keybind["modifiers"].asInt();
						UInt64 key = (modifiers << 32) + keyCode;
						if (key)
							MSF_MainData::DEBUGprintStoredDataHotkey = key;
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

					if (!reader.good())
					{
						std::string err = reader.getFormattedErrorMessages();
						_ERROR("JSON keybind parser: Syntax error in %s, skipping datafile. Detailed error message:", modName.c_str());
						_MESSAGE("%s", err.c_str());
						continue;
					}
					if (inttover(json["version"].asInt()) < MIN_SUPPORTED_KB_VERSION)
					{
						_ERROR("Unsupported keybind version: %s", keybindFiles[i].cFileName);
						continue;
					}

					keybinds = json["keybinds"];
					if (!keybinds.isArray())
					{
						_MESSAGE("Keybind read error in %s: 'keybinds' must be a json array", modName.c_str());
						continue;
					}

					for (int i = 0; i < keybinds.size(); i++)
					{
						const Json::Value& keybind = keybinds[i];

						std::string id = keybind["id"].asString();
						if (id == "")
						{
							_MESSAGE("Keybind read error in %s: 'id' can not be an empty string", modName.c_str());
							continue;
						}

						auto kbIt = MSF_MainData::keybindIDMap.find(id);
						if (kbIt != MSF_MainData::keybindIDMap.end())
						{
							_MESSAGE("Keybind read warning in %s: keybind with id '%s' has been overwritten", modName.c_str(), id.c_str());
							KeybindData* kb = kbIt->second;
							std::string menuName = keybind["menuName"].asString();
							UInt16 flags = keybind["keyfunction"].asInt();
							if ((menuName == "") && (flags & KeybindData::bHUDselection))
							{
								_MESSAGE("Keybind read error in %s: if 'bHUDselection' flag is set 'menuName' can not be an empty string", modName.c_str());
								continue;
							}
							if (flags & KeybindData::bHUDselection)
							{
								UInt8 type = 0;
								if (flags & KeybindData::bGlobalMenu)
									type = ModSelectionMenu::kType_Global;
								else if (flags & KeybindData::bMenuBoth)
									type = ModSelectionMenu::kType_All;
								else if (flags & KeybindData::bIsAmmo)
									type = ModSelectionMenu::kType_AmmoMenu;
								else
									type = ModSelectionMenu::kType_ModMenu;
								if (kb->selectMenu)
								{
									_MESSAGE("Keybind read warning in %s: mod selection menu data in keybind with id '%s' has been overwritten", modName.c_str(), id.c_str());
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
							{
								_MESSAGE("Keybind read error in %s: if 'bHUDselection' flag is set 'menuName' can not be an empty string", modName.c_str());
								continue;
							}
							KeybindData* kb = new KeybindData();
							kb->functionID = id;
							kb->type = flags;
							kb->keyCode = 0;
							kb->modifiers = 0;
							if (flags & KeybindData::bHUDselection)
							{
								UInt8 type = 0;
								if (flags & KeybindData::bGlobalMenu)
									type = ModSelectionMenu::kType_Global;
								else if (flags & KeybindData::bMenuBoth)
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
				//_DEBUG("Expanded buffer to %d bytes.", len);
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
		if (MSF_MainData::iMinRandomAmmo > MSF_MainData::iMaxRandomAmmo)
			MSF_MainData::iMinRandomAmmo = MSF_MainData::iMaxRandomAmmo;
		return true;
	}

	bool SetUserModifiedValue(std::string section, std::string settingName, std::string settingValue)
	{
		if (section == "Bool")
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
			//_DEBUG("Setting read: %04X, %02X", flagType, flagValue);
			//if (flagValue)
			//	MSF_MainData::MCMSettingFlags |= (1 << flagType);
			//else
			//	MSF_MainData::MCMSettingFlags &= ~(1 << flagType);
			//return true;
			bool flagValue = settingValue != "0";
			UInt32 flag = 0;
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
			else if (settingName == "bDisableAutomaticReload")
			{
				flag = MSF_MainData::bDisableAutomaticReload;
				if (!(MSF_MainData::MCMSettingFlags & MSF_MainData::bDisableAutomaticReload) && flagValue)
					HookUtil::SafeWriteBuf(SkipReloadJumpAddr.GetUIntPtr(), &ReloadJumpOverwriteCode.replacement, sizeof(ReloadJumpOverwriteCode.replacement));
				else if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bDisableAutomaticReload) && !flagValue)
					HookUtil::SafeWriteBuf(SkipReloadJumpAddr.GetUIntPtr(), &ReloadJumpOverwriteCode.original, sizeof(ReloadJumpOverwriteCode.original));
			}
			else if (settingName == "bShowUnavailableMods")
				flag = MSF_MainData::bShowUnavailableMods;
			else if (settingName == "bSpawnRandomAmmo")
				flag = MSF_MainData::bSpawnRandomAmmo;
			else if (settingName == "bSpawnRandomMods")
				flag = MSF_MainData::bSpawnRandomMods;
			else if (settingName == "bInjectLeveledLists")
				flag = MSF_MainData::bInjectLeveledLists;
			else if (settingName == "bEnableMetadataSaving")
				flag = MSF_MainData::bEnableMetadataSaving;
			else if (settingName == "bEnableAmmoSaving")
				flag = MSF_MainData::bEnableAmmoSaving;
			else if (settingName == "bEnableTacticalReloadAll")
				flag = MSF_MainData::bEnableTacticalReloadAll;
			else if (settingName == "bEnableTacticalReloadAnim")
				flag = MSF_MainData::bEnableTacticalReloadAnim;
			else if (settingName == "bEnableBCRSupport")
				flag = MSF_MainData::bEnableBCRSupport;
			else if (settingName == "bReloadCompatibilityMode")
				flag = MSF_MainData::bReloadCompatibilityMode;
			else if (settingName == "bDisplayChamberedAmmoOnHUD")
				flag = MSF_MainData::bDisplayChamberedAmmoOnHUD;
			else if (settingName == "bDisplayConditionInPipboy")
				flag = MSF_MainData::bDisplayConditionInPipboy;
			else if (settingName == "bDisplayMagInPipboy")
				flag = MSF_MainData::bDisplayMagInPipboy;
			else if (settingName == "bDisplayChamberInPipboy")
				flag = MSF_MainData::bDisplayChamberInPipboy;
			else if (settingName == "bShowEquippedAmmo")
				flag = MSF_MainData::bShowEquippedAmmo;
			else
				return false;

			if (flagValue)
				MSF_MainData::MCMSettingFlags |= flag;
			else
				MSF_MainData::MCMSettingFlags &= ~flag;
			return true;
		}
		else if (section == "Int")
		{
			UInt16 value = 0;
			if (settingName == "iAutolowerTimeSec")
				MSF_MainData::iAutolowerTimeSec = std::stoi(settingValue);
			else if (settingName == "iMinRandomAmmo")
				MSF_MainData::iMinRandomAmmo = std::stoi(settingValue);
			else if (settingName == "iMaxRandomAmmo")
				MSF_MainData::iMaxRandomAmmo = std::stoi(settingValue);
			else if (settingName == "iReloadAnimEndEventDelayMS")
				MSF_MainData::iReloadAnimEndEventDelayMS = std::stoi(settingValue);
			else if (settingName == "iCustomAnimEndEventDelayMS")
				MSF_MainData::iCustomAnimEndEventDelayMS = std::stoi(settingValue);
			else if (settingName == "iDrawAnimEndEventDelayMS")
				MSF_MainData::iDrawAnimEndEventDelayMS = std::stoi(settingValue);
			return true;
		}
		else if (section == "Float")
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

		FillQuickAccessData();

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

				if (!reader.good())
				{
					std::string err = reader.getFormattedErrorMessages();
					_ERROR("JSON data parser: Syntax error in %s, skipping datafile. Detailed error message:", fileName.c_str());
					_MESSAGE("%s", err.c_str());
					return false;
				}

				if (inttover(json["version"].asInt()) < MIN_SUPPORTED_DATA_VERSION)
				{
					_ERROR("JSON data: Unsupported data version in %s", fileName.c_str());
					return false;
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
							{
								_ERROR("JSON data: Missing required plugin file (%s) in %s", pluginName.c_str(), fileName.c_str());
								return false;
							}
						}
					}
				}

				data1 = json["projectileMod"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& proj = data1[i];
						std::string str = proj["mod"].asString();
						if (str == "")
						{
							_MESSAGE("Data error in %s: 'projectileMod->mod' can not be an empty string", fileName.c_str());
							continue;
						}
						BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
						if (!mod)
						{
							_MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data", fileName.c_str(), str.c_str());
							continue;
						}
						if (mod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
						{
							_MESSAGE("Data error in %s: target type of mod '%s' is not weapon", fileName.c_str(), str.c_str());
							continue;
						}
						UInt16 flags = proj["flags"].asInt();
						
						data2 = proj["BGSModContainerData"];
						if (data2.isArray())
						{
							for (int ii = 0; ii < data2.size(); ii++)
							{
								const Json::Value& cont = data2[ii];
								BGSMod::Container::Data data;
								data.target = cont["target"].asInt();
								if (data.target <= CustomProjectileFormManager::WeaponFormProperty::kNull || data.target >= CustomProjectileFormManager::WeaponFormProperty::kMax)
								{
									_MESSAGE("Data error in %s: 'projectileMod->BGSModContainerData->target' target type out of range", fileName.c_str());
									continue;
								}
								bool success = false;
								data.op = cont["op"].asInt();
								switch (data.op)
								{
								case BGSMod::Container::Data::kOpFlag_Set_Bool:
								case BGSMod::Container::Data::kOpFlag_Add_Int:
								case BGSMod::Container::Data::kOpFlag_Set_Int:
								case BGSMod::Container::Data::kOpFlag_Or_Bool:
								case BGSMod::Container::Data::kOpFlag_And_Bool:
								{
									if (data.target > CustomProjectileFormManager::kWeaponTarget_TracerFreq)
										break;
									data.value.i.v1 = cont["value"].asInt();
									if (data.value.i.v1 > 255)
										break;
									success = true;
								}
								break;
								case BGSMod::Container::Data::kOpFlag_Set_Enum:
								{
									data.value.i.v1 = cont["value"].asInt();
									if (data.target == CustomProjectileFormManager::kWeaponTarget_SoundLevel && data.value.i.v1 >= 0 && data.value.i.v1 <= 4)
										success = true;
									else if (data.target == CustomProjectileFormManager::kWeaponTarget_Type && data.value.i.v1 >= 0 && data.value.i.v1 <= 6)
									{
										data.value.i.v1 = 1 << data.value.i.v1;
										success = true;
									}
								}
								case BGSMod::Container::Data::kOpFlag_Set_Float:
								case BGSMod::Container::Data::kOpFlag_Mul_Add_Float:
								case BGSMod::Container::Data::kOpFlag_Add_Float:
								{
									if (data.target > CustomProjectileFormManager::kWeaponTarget_RelaunchInt || (data.target < CustomProjectileFormManager::kWeaponTarget_Gravity && data.target != CustomProjectileFormManager::kWeaponTarget_TracerFreq))
										break;
									data.value.f.v1 = cont["value"].asFloat();
									if (data.op == BGSMod::Container::Data::kOpFlag_Set_Float)
									{
										if (data.value.f.v1 < 0 && (data.target == CustomProjectileFormManager::kWeaponTarget_Speed || data.target == CustomProjectileFormManager::kWeaponTarget_Range || data.target == CustomProjectileFormManager::kWeaponTarget_ExpTimer || data.target == CustomProjectileFormManager::kWeaponTarget_MuzzleFlashDur))
											break;
										if (data.target == CustomProjectileFormManager::kWeaponTarget_RelaunchInt && data.value.f.v1 < 0.0010)
											break;
										if ((data.target == CustomProjectileFormManager::kWeaponTarget_RelaunchInt || data.target == CustomProjectileFormManager::kWeaponTarget_MuzzleFlashDur) && data.value.f.v1 > 5.0)
											break;
										if (data.target == CustomProjectileFormManager::kWeaponTarget_Speed && data.value.f.v1 > 1000000000.00)
											break;
									}
									success = true;
								}
								break;
								case BGSMod::Container::Data::kOpFlag_Set_Form:
								case BGSMod::Container::Data::kOpFlag_Add_Form:
								{
									str = cont["value"].asString();
									if (str == "")
									{
										_MESSAGE("Data error in %s: 'projectileMod->BGSModContainerData->value(TESForm)' can not be an empty string", fileName.c_str());
										break;
									}
									data.value.form = Utilities::GetFormFromIdentifier(str);
									if (!data.value.form)
									{
										_MESSAGE("Data error in %s 'projectileMod->BGSModContainerData->value(TESForm)': '%s' could not be found in loaded game data", fileName.c_str(), str.c_str());
										break;
									}
									switch (data.target)
									{
									case CustomProjectileFormManager::kWeaponTarget_Light:
									case CustomProjectileFormManager::kWeaponTarget_MuzzleFlashLight:
									{
										TESObjectLIGH* form = DYNAMIC_CAST(data.value.form, TESForm, TESObjectLIGH);
										if (form)
											success = true;
									}
									break;
									case CustomProjectileFormManager::kWeaponTarget_ExpType:
									{
										BGSExplosion* form = DYNAMIC_CAST(data.value.form, TESForm, BGSExplosion);
										if (form)
											success = true;
									}
									break;
									case CustomProjectileFormManager::kWeaponTarget_ActSoundLoop:
									case CustomProjectileFormManager::kWeaponTarget_CountdownSound:
									case CustomProjectileFormManager::kWeaponTarget_DeactivateSound:
									{
										BGSSoundDescriptorForm* form = DYNAMIC_CAST(data.value.form, TESForm, BGSSoundDescriptorForm);
										if (form)
											success = true;
									}
									break;
									case CustomProjectileFormManager::kWeaponTarget_DecalData:
									{
										BGSTextureSet* form = DYNAMIC_CAST(data.value.form, TESForm, BGSTextureSet);
										if (form)
											success = true;
									}
									break;
									case CustomProjectileFormManager::kWeaponTarget_CollisionLayer:
									{
										TESForm* form = (TESForm*)Runtime_DynamicCast(data.value.form, RTTI_TESForm, RTTI_BGSCollisionLayer);
										if (form)
											success = true;
									}
									break;
									case CustomProjectileFormManager::kWeaponTarget_VATSprojectile:
									{
										BGSProjectile* form = DYNAMIC_CAST(data.value.form, TESForm, BGSProjectile);
										if (form)
											success = true;
									}
									break;
									case CustomProjectileFormManager::kWeaponTarget_Model:
									case CustomProjectileFormManager::kWeaponTarget_MuzzleFlashModel:
									{
										TESModel* form = DYNAMIC_CAST(data.value.form, TESForm, TESModel);
										if (form)
											success = true;
									}
									break;
									default:
									{
										_MESSAGE("Data error in %s 'projectileMod->BGSModContainerData->value(TESForm)': invalid target specified for form '%s'", fileName.c_str(), str.c_str());
									}
									break;
									}
								}
								break;
								case BGSMod::Container::Data::kOpFlag_Rem_Form:
								{
									if (data.target > CustomProjectileFormManager::kWeaponTarget_RelaunchInt)
									{
										data.value.form = nullptr;
										success = true;
									}
								}
								break;
								default:
								{
									_MESSAGE("Data error in %s 'projectileMod->BGSModContainerData->op': no valid operator found", fileName.c_str());
								}
								break;
								}
								if (success)
								{
									MSF_MainData::projectileManager.AddModData(mod, data, flags & 1);
									_DEBUG("added");
								}
							}
						}
					}
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
						else
							_MESSAGE("Data error in %s: keyword with identifier '%s' or its value could not be found in loaded game data", fileName.c_str(), apKWstr.c_str());
					}
					else 
						_MESSAGE("Data error in %s: 'apKWstr' must have a non-empty string value", fileName.c_str());
					data2 = data1["attachParent"];
					if (data2.isArray())
					{
						for (int i = 0; i < data2.size(); i++)
						{
							const Json::Value& apData = data2[i];
							std::string str = apData["mod"].asString();
							if (str == "")
							{
								_MESSAGE("Data error in %s: 'compatibility->attachParent->mod' can not be an empty string", fileName.c_str());
								continue;
							}
							BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
							if (!mod)
							{
								_MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data", fileName.c_str(), str.c_str());
								continue;
							}
							str = apData["apKeyword"].asString();
							if (str == "")
							{
								_MESSAGE("Data error in %s: 'compatibility->attachParent->apKeyword' can not be an empty string", fileName.c_str());
								continue;
							}
							BGSKeyword* apKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
							KeywordValue value = Utilities::GetAttachValueForTypedKeyword(apKW);
							if (!apKW || value < 0)
							{
								_MESSAGE("Data error in %s: attach parent keyword '%s' or its value could not be found in loaded game data", fileName.c_str(), str.c_str());
								continue;
							}
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



					data2 = data1["attachChildren"];
					if (data2.isArray())
					{
						for (int i = 0; i < data2.size(); i++)
						{
							const Json::Value& apData = data2[i];
							std::string str = apData["mod"].asString();
							if (str == "")
							{
								_MESSAGE("Data error in %s: 'compatibility->attachChildren->mod' can not be an empty string", fileName.c_str());
								continue;
							}
							BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
							if (!mod)
							{
								_MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data", fileName.c_str(), str.c_str());
								continue;
							}
							str = apData["apKeyword"].asString();
							if (str == "")
							{
								_MESSAGE("Data error in %s: 'compatibility->attachChildren->apKeyword' can not be an empty string", fileName.c_str());
								continue;
							}
							bool bRemove = apData["bRemove"].asBool();
							BGSKeyword* apKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
							KeywordValue value = Utilities::GetAttachValueForTypedKeyword(apKW);
							if (!apKW || value < 0)
							{
								_MESSAGE("Data error in %s: attach parent keyword '%s' or its value could not be found in loaded game data", fileName.c_str(), str.c_str());
								continue;
							}
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
							{
								_MESSAGE("Data error in %s: 'compatibility->instantiationKWs->mod' can not be an empty string", fileName.c_str());
								continue;
							}
							BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
							if (!mod)
							{
								_MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data", fileName.c_str(), str.c_str());
								continue;
							}
							str = ifData["ifKeyword"].asString();
							if (str == "")
							{
								_MESSAGE("Data error in %s: 'compatibility->instantiationKWs->apKeyword' can not be an empty string", fileName.c_str());
								continue;
							}
							bool bRemove = ifData["bRemove"].asBool();
							BGSKeyword* ifKW = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, BGSKeyword);
							KeywordValue value = Utilities::GetInstantiationValueForTypedKeyword(ifKW);
							if (!ifKW || value < 0)
							{
								_MESSAGE("Data error in %s: instantiation keyword '%s' or its value could not be found in loaded game data", fileName.c_str(), str.c_str());
								continue;
							}
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
				}

				data1 = json["ammoData"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& ammoData = data1[i];
						std::string str = ammoData["baseAmmo"].asString();
						if (str == "")
						{ _MESSAGE("Data error in %s: 'ammoData->baseAmmo' can not be an empty string", fileName.c_str()); continue; }
						TESAmmo* baseAmmo = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESAmmo);
						if (!baseAmmo)
						{ _MESSAGE("Data error in %s: ammo '%s' could not be found in loaded game data", fileName.c_str(), str.c_str()); continue; }
						if (MSF_Base::IsNotSupportedAmmo(baseAmmo))
						{ _MESSAGE("Data error in %s: ammo '%s' is not supported", fileName.c_str(), str.c_str()); continue; }
						BGSMod::Attachment::Mod* baseMod = nullptr;
						//str = ammoData["baseMod"].asString();
						//if (str != "")
						//	baseMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
						//else
						//	baseMod = MSF_MainData::PlaceholderModAmmo;
						//if (baseMod->priority != 125 || baseMod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
						//	continue;
						UInt16 ammoIDbase = ammoData["baseAmmoID"].asInt();
						float spawnChanceBase = ammoData["spawnChanceBase"].asFloat();
						LevItem* baseAmmoLL = nullptr;
						str = ammoData["baseLeveledList"].asString();
						if (str != "")
							baseAmmoLL = (LevItem*)DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESLevItem);

						auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
						if (itAD != MSF_MainData::ammoDataMap.end())
						{
							_MESSAGE("Data warning in %s: ammo data with base ammo '%s' has been overwritten", fileName.c_str(), str.c_str());
							AmmoData* itAmmoData = itAD->second;
							itAmmoData->baseAmmoData.mod = baseMod;
							itAmmoData->baseAmmoData.ammoID = ammoIDbase;
							itAmmoData->baseAmmoData.spawnChance = spawnChanceBase;
							itAmmoData->baseAmmoLL = baseAmmoLL;
							data2 = ammoData["ammoTypes"];
							if (data2.isArray())
							{
								for (int i2 = 0; i2 < data2.size(); i2++)
								{
									const Json::Value& ammoType = data2[i2];
									std::string type = ammoType["ammo"].asString();
									if (type == "")
									{ _MESSAGE("Data error in %s: 'ammoData->ammoTypes->ammo' can not be an empty string", fileName.c_str()); continue; }
									TESAmmo* ammo = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(type), TESForm, TESAmmo);
									if (!ammo)
									{ _MESSAGE("Data error in %s: ammo '%s' could not be found in loaded game data", fileName.c_str(), type.c_str()); continue; }
									if (MSF_Base::IsNotSupportedAmmo(ammo))
									{ _MESSAGE("Data error in %s: ammo '%s' is not supported", fileName.c_str(), str.c_str()); continue; }
									type = ammoType["mod"].asString();
									if (type == "")
									{ _MESSAGE("Data error in %s: 'ammoData->ammoTypes->mod' can not be an empty string", fileName.c_str()); continue; }
									BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(type), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
									if (!mod)
									{ _MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data", fileName.c_str(), type.c_str()); continue; }
									if (mod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
									{ _MESSAGE("Data error in %s: target type of mod '%s' is not weapon", fileName.c_str(), type.c_str()); continue; }
									mod->unkC0 = MSF_MainData::ammoAP;
									//if (mod->unkC0 != MSF_MainData::ammoAP)
									//{ _MESSAGE("Data error in %s: attach point of ammo mod '%s' is not MSF_ap_AmmoType", fileName.c_str(), type.c_str()); continue; }
									UInt16 ammoID = ammoType["ammoID"].asInt();
									float spawnChance = ammoType["spawnChance"].asFloat();

									std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = itAmmoData->ammoMods.begin();
									for (itAmmoMod; itAmmoMod != itAmmoData->ammoMods.end(); itAmmoMod++)
									{
										_MESSAGE("Data warning in %s: ammo type '%s' with base ammo '%s' has been overwritten", fileName.c_str(), type.c_str(), str.c_str());
										if (itAmmoMod->ammo == ammo)
										{
											itAmmoMod->mod = mod;
											itAmmoMod->ammoID = ammoID;
											itAmmoMod->spawnChance = spawnChance;
											break;
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
							ammoDataStruct->baseAmmoLL = baseAmmoLL;
							data2 = ammoData["ammoTypes"];
							if (data2.isArray())
							{
								for (int i2 = 0; i2 < data2.size(); i2++)
								{
									const Json::Value& ammoType = data2[i2];
									std::string type = ammoType["ammo"].asString();
									if (type == "")
									{ _MESSAGE("Data error in %s: 'ammoData->ammoTypes->ammo' can not be an empty string", fileName.c_str()); continue; }
									TESAmmo* ammo = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(type), TESForm, TESAmmo);
									if (!ammo)
									{ _MESSAGE("Data error in %s: ammo '%s' could not be found in loaded game data", fileName.c_str(), type.c_str()); continue; }
									if (MSF_Base::IsNotSupportedAmmo(ammo))
									{ _MESSAGE("Data error in %s: ammo '%s' is not supported", fileName.c_str(), str.c_str()); continue; }
									type = ammoType["mod"].asString();
									if (type == "")
									{ _MESSAGE("Data error in %s: 'ammoData->ammoTypes->mod' can not be an empty string", fileName.c_str()); continue; }
									BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(type), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
									if (!mod)
									{ _MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data", fileName.c_str(), type.c_str()); continue; }
									if (mod->targetType != BGSMod::Attachment::Mod::kTargetType_Weapon)
									{ _MESSAGE("Data error in %s: target type of mod '%s' is not weapon", fileName.c_str(), type.c_str()); continue; }
									mod->unkC0 = MSF_MainData::ammoAP;
									//if (mod->unkC0 != MSF_MainData::ammoAP) 
									//{ _MESSAGE("Data error in %s: attach point of ammo mod '%s' is not MSF_ap_AmmoType", fileName.c_str(), type.c_str()); continue; }
									UInt16 ammoID = ammoType["ammoID"].asInt();
									float spawnChance = ammoType["spawnChance"].asFloat();

									std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = ammoDataStruct->ammoMods.begin();
									for (itAmmoMod; itAmmoMod != ammoDataStruct->ammoMods.end(); itAmmoMod++)
									{
										if (itAmmoMod->ammo == ammo)
										{
											_MESSAGE("Data warning in %s: ammo type '%s' with base ammo '%s' has been overwritten", fileName.c_str(), type.c_str(), str.c_str());
											itAmmoMod->mod = mod;
											itAmmoMod->ammoID = ammoID;
											itAmmoMod->spawnChance = spawnChance;
											break;
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
							else
							{
								_MESSAGE("Data error in %s: ignoring new base ammo '%s' because it has no associated ammo types", fileName.c_str(), str.c_str());
								delete ammoDataStruct;
							}
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
						{
							_MESSAGE("Data error in %s: 'hotkeys->keyID' can not be an empty string", fileName.c_str()); continue;
						}
						auto kb = MSF_MainData::keybindIDMap.find(keyID);
						if (kb == MSF_MainData::keybindIDMap.end())
						{
							_MESSAGE("Data error in %s: keybind with keyID '%s' could not be found in the loaded keybind data files, skipping all of its data entirely", fileName.c_str(), keyID.c_str()); continue;
						}
						KeybindData* itKb = kb->second;
						if (itKb->type & KeybindData::bIsAmmo)
						{
							_MESSAGE("Data error in %s: keybind with keyID '%s' is flagged as ammo, skipping all of its data in 'hotkeys' section", fileName.c_str(), keyID.c_str()); continue;
						}
						if (itKb->type & KeybindData::bGlobalMenu)
						{
							//get attach points
							continue;
						}
						//getmenuanim
						const Json::Value& modStruct = key["modData"];
						if (!modStruct.isArray()) /////!!!!!!
						{
							std::string apstr = modStruct["apKeyword"].asString();
							if (apstr == "")
							{
								_MESSAGE("Data error in %s: 'hotkeys->modData->apKeyword' can not be an empty string", fileName.c_str()); continue;
							}
							BGSKeyword* apKeyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(apstr), TESForm, BGSKeyword);
							if (!apKeyword)
							{
								_MESSAGE("Data error in %s: attach parent keyword '%s' could not be found in loaded game data", fileName.c_str(), apstr.c_str());
								continue;
							}
							KeywordValue apValue = Utilities::GetAttachValueForTypedKeyword(apKeyword);
							if (apValue < 0)
							{
								_MESSAGE("Data error in %s: attach parent keyword '%s' value could not be found in loaded game data", fileName.c_str(), apstr.c_str());
								continue;
							}
							UInt16 apflags = modStruct["apFlags"].asInt();
							ModData* modData = itKb->modData;
							//APattachReqs
							if (!modData)
							{
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
									std::string ifstr = modCycle["ifKeyword"].asString();
									if (ifstr == "")
									{
										_MESSAGE("Data error in %s: 'hotkeys->modData->modArrays->ifKeyword' can not be an empty string", fileName.c_str()); continue;
									}
									BGSKeyword* ifKeyword = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(ifstr), TESForm, BGSKeyword);
									if (!ifKeyword)
									{
										_MESSAGE("Data error in %s: instantiation keyword '%s' could not be found in loaded game data", fileName.c_str(), ifstr.c_str());
										continue;
									}
									KeywordValue ifValue = Utilities::GetInstantiationValueForTypedKeyword(ifKeyword);
									if (ifValue < 0)
									{
										_MESSAGE("Data error in %s: instantiation keyword value '%s' could not be found in loaded game data", fileName.c_str(), ifstr.c_str());
										continue;
									}
									UInt16 ifflags = modCycle["ifFlags"].asInt();
									float spawnChanceBase = modCycle["spawnChanceBase"].asFloat();
									auto itCycle = modData->modCycleMap.find(ifValue);
									ModData::ModCycle* cycle = nullptr;
									if (itCycle == modData->modCycleMap.end())
									{
										cycle = new ModData::ModCycle();
										cycle->flags = ifflags;
										cycle->spawnChanceBase = spawnChanceBase;
									}
									else
										cycle = itCycle->second;
									const Json::Value& mods = modCycle["mods"];
									if (mods.isArray())
									{
										for (int i = 0; i < mods.size(); i++)
										{
											const Json::Value& switchmod = mods[i];
											std::string str = switchmod["mod"].asString();
											if (str == "")
											{
												_MESSAGE("Data error in %s: 'hotkeys->modData->modArrays->mods->mod' can not be an empty string", fileName.c_str()); continue;
											}
											BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (!mod || mod->unkC0 != apValue)
											{
												_MESSAGE("Data error in %s: mod '%s' could not be found in loaded game data or its attach parent is not %s", fileName.c_str(), str.c_str(), apstr.c_str());
												continue;
											}
											ModData::ModVector::iterator itMod = std::find_if(cycle->mods.begin(), cycle->mods.end(), [mod](ModData::Mod* data){
												return data->mod == mod;
											});
											if (itMod != cycle->mods.end()) //overwrite?
											{
												_WARNING("Conflict in %s: mod %s is already in mod cycle with instantiation filter %s", fileName.c_str(), str.c_str(), ifstr.c_str());
												continue;
											}
										
											UInt32 modflags = switchmod["modFlags"].asInt();
											float spawnChance = switchmod["spawnChance"].asFloat();
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
											TESIdleForm* animIdle_1stP_PA = nullptr;
											TESIdleForm* animIdle_3rdP_PA = nullptr;
											str = switchmod["animIdle_1stP"].asString();
											if (str != "")
												animIdle_1stP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
											str = switchmod["animIdle_3rdP"].asString();
											if (str != "")
												animIdle_3rdP = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
											str = switchmod["animIdle_1stP_PA"].asString();
											if (str != "")
												animIdle_1stP_PA = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
											str = switchmod["animIdle_3rdP_PA"].asString();
											if (str != "")
												animIdle_3rdP_PA = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
											ModData::Mod* modDataMod = new ModData::Mod();
											modDataMod->mod = mod; 
											modDataMod->flags = modflags;
											modDataMod->spawnChance = spawnChance;
											if (animIdle_1stP || animIdle_3rdP || animIdle_1stP_PA || animIdle_3rdP_PA)
											{
												AnimationData* animData = new AnimationData(animIdle_1stP, animIdle_3rdP, animIdle_1stP_PA, animIdle_3rdP_PA, ((modflags >> 16) & 1) == 0);
												modDataMod->animData = animData;
											}
											cycle->mods.push_back(modDataMod);
										}
									}
									if (cycle->mods.size() < 1 || ((cycle->flags & ModData::ModCycle::bCannotHaveNullMod) && cycle->mods.size() < 2))
									{
										_MESSAGE("Data error in %s: ignoring new mod cycle with instantiation keyword '%s' because its size is below the minimum size requirement", fileName.c_str(), ifstr.c_str());
										for (ModData::ModVector::iterator itMod = cycle->mods.begin(); itMod != cycle->mods.end(); itMod++)
										{
											ModData::Mod* modDataMod = *itMod;
											delete modDataMod;
										}
										delete cycle;
										continue;
									}
									modData->modCycleMap[ifValue] = cycle;
									for (ModData::ModVector::iterator itMod = cycle->mods.begin(); itMod != cycle->mods.end(); itMod++)
										MSF_MainData::instantiationRequirements.insert({ (*itMod)->mod, ifValue });
								}
							}
							if (modData->modCycleMap.size() < 1)
							{
								_MESSAGE("Data error in %s: ignoring new mod data map with attach parent keyword '%s' because its size is below the minimum size requirement", fileName.c_str(), apstr.c_str());
								//deletereqs
								delete modData;
								continue;
							}
							itKb->modData = modData;
							//if (modData->flags & ModData::bRequireAPmod)
							//	Utilities::AddAttachValue((AttachParentArray*)&MSF_MainData::APbaseMod->unk98, apValue);

						}
					}
				}

				data1 = json["chamberData"];
				if (data1.isArray())
				{
					for (int i = 0; i < data1.size(); i++)
					{
						const Json::Value& chamberSize = data1[i];
						std::string str = chamberSize["mod"].asString();
						if (str == "")
							continue;
						BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(Utilities::GetFormFromIdentifier(str), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
						if (!mod)
							continue;
						UInt16 flags = chamberSize["flags"].asInt();
						flags = (flags << 4) & ExtraWeaponState::WeaponState::mChamberMask;
						UInt16 size = chamberSize["chamberSize"].asInt();

						auto itChamberMod = MSF_MainData::modChamberMap.find(mod);
						if (itChamberMod != MSF_MainData::modChamberMap.end())
						{
							itChamberMod->second.chamberSize = size;
							itChamberMod->second.flags = flags;
						}
						else
							MSF_MainData::modChamberMap.insert({ mod, ChamberData(size, flags) });
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
						TESIdleForm* animIdle_1stP_PA = nullptr;
						TESIdleForm* animIdle_3rdP_PA = nullptr;
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
						str = animData["animIdle_1stP_PA"].asString();
						if (str != "")
							animIdle_1stP_PA = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_1stP_PA)
							continue;
						str = animData["animIdle_3rdP_PA"].asString();
						if (str != "")
							animIdle_3rdP_PA = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_3rdP_PA)
							continue;

						auto itAnim = MSF_MainData::reloadAnimDataMap.find(weapon);
						if (itAnim != MSF_MainData::reloadAnimDataMap.end())
						{
							itAnim->second->animIdle_1stP = animIdle_1stP;
							itAnim->second->animIdle_3rdP = animIdle_3rdP;
						}
						else
						{
							AnimationData* animationData = new AnimationData(animIdle_1stP, animIdle_3rdP, animIdle_1stP_PA, animIdle_3rdP_PA);
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
						TESIdleForm* animIdle_1stP_PA = nullptr;
						TESIdleForm* animIdle_3rdP_PA = nullptr;
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
						str = animData["animIdle_1stP_PA"].asString();
						if (str != "")
							animIdle_1stP_PA = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_1stP_PA)
							continue;
						str = animData["animIdle_3rdP_PA"].asString();
						if (str != "")
							animIdle_3rdP_PA = DYNAMIC_CAST(Utilities::GetFormFromIdentifier(str), TESForm, TESIdleForm);
						if (!animIdle_3rdP_PA)
							continue;

						auto itAnim = MSF_MainData::fireAnimDataMap.find(weapon);
						if (itAnim != MSF_MainData::fireAnimDataMap.end())
						{
							itAnim->second->animIdle_1stP = animIdle_1stP;
							itAnim->second->animIdle_3rdP = animIdle_3rdP;
						}
						else
						{
							AnimationData* animationData = new AnimationData(animIdle_1stP, animIdle_3rdP, animIdle_1stP_PA, animIdle_3rdP_PA);
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

	void PrintStoredData()
	{
		_MESSAGE("");
		_MESSAGE("storedDATA:");
		_MESSAGE("ammoAP: %04X", MSF_MainData::ammoAP);
		_MESSAGE("MCM settings: %04X", MSF_MainData::MCMSettingFlags);
		for (std::unordered_map<TESAmmo*, AmmoData*>::iterator it = MSF_MainData::ammoDataMap.begin(); it != MSF_MainData::ammoDataMap.end(); it++)
		{
			AmmoData* itAmmoData = it->second;
			if (!itAmmoData)
				continue;
			_MESSAGE("baseAmmo: %08X; size: %i, baseID: %i, baseChance: %f, leveledList: %08X", itAmmoData->baseAmmoData.ammo->formID, itAmmoData->ammoMods.size(), itAmmoData->baseAmmoData.ammoID, itAmmoData->baseAmmoData.spawnChance, itAmmoData->baseAmmoLL ? itAmmoData->baseAmmoLL->formID : 0);
			for (auto itAmmoMod : itAmmoData->ammoMods)
				_MESSAGE("----Ammo: %08X; AmmoMod: %08X, ammoID: %i, spawnChance: %f, apOK: %02X", itAmmoMod.ammo->formID, itAmmoMod.mod->formID, itAmmoMod.ammoID, itAmmoMod.spawnChance, itAmmoMod.mod->unkC0 == MSF_MainData::ammoAP);
		}
		for (std::unordered_map<std::string, KeybindData*>::iterator it = MSF_MainData::keybindIDMap.begin(); it != MSF_MainData::keybindIDMap.end(); it++)
		{
			KeybindData* itKb = it->second;
			if (!itKb)
				continue;
			if (itKb->selectMenu)
				_MESSAGE("kbID: %s; flags: %i; key: %i; swfName: %s; swfType: %i; swfVer: %08X", itKb->functionID.c_str(), itKb->type, itKb->keyCode, itKb->selectMenu->scaleformName.c_str(), itKb->selectMenu->type, itKb->selectMenu->version);
			else
				_MESSAGE("kbID: %s; flags: %i; key: %i", itKb->functionID.c_str(), itKb->type, itKb->keyCode);
			if (!itKb->modData)
				continue;
			BGSKeyword* kw = nullptr;
			g_AttachPointKeywordArray->GetNthItem(itKb->modData->attachParentValue, kw);
			if (kw)
				_MESSAGE("modData: apkeyword: %08X, flags: %08X, cycleCount: %i", kw->formID, itKb->modData->flags, itKb->modData->modCycleMap.size());
			for (std::unordered_map<KeywordValue, ModData::ModCycle*>::iterator itCycles = itKb->modData->modCycleMap.begin(); itCycles != itKb->modData->modCycleMap.end(); itCycles++)
			{
				BGSKeyword* ifkw = nullptr;
				g_InstantiationKeywordArray->GetNthItem(itCycles->first, ifkw);
				ModData::ModCycle* cycle = itCycles->second;
				if (ifkw && cycle)
					_MESSAGE("-modCycle: ifkeyword: %08X, flags: %08X, modCount: %i", ifkw->formID, cycle->flags, cycle->mods.size());
				for (std::vector<ModData::Mod*>::iterator itData = cycle->mods.begin(); itData != cycle->mods.end(); itData++)
				{
					ModData::Mod* mod = *itData;
					if (mod->mod)
						_MESSAGE("--MOD: %08X, flags: %08X", mod->mod->formID, mod->flags);
				}
			}
		}

		for (auto itMod : MSF_MainData::projectileManager.projectileModMap)
		{
			_MESSAGE("projMod: %08X; size: %i", itMod.first->formID, itMod.second.size());
			for (auto itData : itMod.second)
			{
				if (itData.target <= CustomProjectileFormManager::WeaponFormProperty::kWeaponTarget_SoundLevel)
					_MESSAGE("----target: %i; op: %i, value: %i", itData.target, itData.op, itData.value.i.v1);
				else if (itData.target >= CustomProjectileFormManager::WeaponFormProperty::kWeaponTarget_Light)
					_MESSAGE("----target: %i; op: %i, value: %08X", itData.target, itData.op, itData.value.form->formID);
				else
					_MESSAGE("----target: %i; op: %i, value: f", itData.target, itData.op, itData.value.f.v1);
			}
		}

		_MESSAGE("MCM float Settings:");
		for (std::unordered_map<std::string, float>::iterator itSettings = MSF_MainData::MCMfloatSettingMap.begin(); itSettings != MSF_MainData::MCMfloatSettingMap.end(); itSettings++)
			_MESSAGE("- %s : %f", itSettings->first.c_str(), itSettings->second);
		_MESSAGE("weap reload:");
		for (std::unordered_map<TESObjectWEAP*, AnimationData*>::iterator itAnim = MSF_MainData::reloadAnimDataMap.begin(); itAnim != MSF_MainData::reloadAnimDataMap.end(); itAnim++)
			_MESSAGE("- %08X", itAnim->first->formID);
		_MESSAGE("weap fire:");
		for (std::unordered_map<TESObjectWEAP*, AnimationData*>::iterator itAnim = MSF_MainData::fireAnimDataMap.begin(); itAnim != MSF_MainData::fireAnimDataMap.end(); itAnim++)
			_MESSAGE("- %08X", itAnim->first->formID);
		for (std::vector<HUDFiringModeData>::iterator itData = MSF_MainData::fmDisplayData.begin(); itData != MSF_MainData::fmDisplayData.end(); itData++)
			_MESSAGE("fmKeyword: %08X, display: %s", itData->keyword->formID, itData->displayString.c_str());
		for (std::vector<HUDScopeData>::iterator itData = MSF_MainData::scopeDisplayData.begin(); itData != MSF_MainData::scopeDisplayData.end(); itData++)
			_MESSAGE("scKeyword: %08X, display: %s", itData->keyword->formID, itData->displayString.c_str());
		for (std::vector<HUDMuzzleData>::iterator itData = MSF_MainData::muzzleDisplayData.begin(); itData != MSF_MainData::muzzleDisplayData.end(); itData++)
			_MESSAGE("mzKeyword: %08X, display: %s", itData->keyword->formID, itData->displayString.c_str());

		MSF_MainData::weaponStateStore.PrintStoredWeaponStates();

		if (MSF_MainData::ammoAP != -1)
		{
			UInt32 errnum = 0;
			tArray<TESObjectWEAP*>* weaps = &(*g_dataHandler)->arrWEAP;
			for (UInt64 idx = 0; idx < weaps->count; idx++)
			{
				TESObjectWEAP* weapForm = nullptr;
				weaps->GetNthItem(idx, weapForm);
				AttachParentArray* aps = (AttachParentArray*)&weapForm->attachParentArray;
				if (aps->kewordValueArray.GetItemIndex(MSF_MainData::ammoAP) == -1)
					errnum += 1;
			}
			_MESSAGE("Missing ammo AP: %i", errnum);
		}
		else
			_MESSAGE("Ammo attach point error.");

		uint64_t res = CheckHooks();
		if (res == 0)
			_MESSAGE("Hooks OK.");

		_MESSAGE("");
	}

	//========================= Select Object Mod =======================
	SwitchData* GetNthAmmoMod(UInt32 num)
	{
		if (num < 0)
			return nullptr;
		BGSObjectInstanceExtra* moddata = Utilities::GetEquippedModData(*g_player, 41);
		TESObjectWEAP* baseWeapon = Utilities::GetEquippedGun(*g_player);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(moddata, baseWeapon);
		if (!baseAmmo)
			return nullptr;
		_DEBUG("baseOK");
		auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itAD != MSF_MainData::ammoDataMap.end())
		{
			_DEBUG("ammofound");
			AmmoData* itAmmoData = itAD->second;
			if (num == 0)
			{
				_DEBUG("num0");
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
				{
					_DEBUG("invcount base");
					if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, baseAmmo) == 0)
						return nullptr;
				}
				///BGSMod::Attachment::Mod* currSwitchedMod = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::hasSwitchedAmmoKW);
				BGSMod::Attachment::Mod* currSwitchedMod = Utilities::GetModAtAttachPoint(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::ammoAP);
				if (!currSwitchedMod)
					return nullptr;
				SwitchData* switchData = new SwitchData();
				switchData->ModToRemove = currSwitchedMod;
				switchData->targetAmmo = baseAmmo;
				return switchData;
			}
			num--;
			_DEBUG("check: %02X, %02X", (num + 1), itAmmoData->ammoMods.size());
			if ((num + 1) > itAmmoData->ammoMods.size())
				return nullptr;
			AmmoData::AmmoMod* ammoMod = &itAmmoData->ammoMods[num];
			if (MSF_MainData::MCMSettingFlags & MSF_MainData::bRequireAmmoToSwitch)
			{
				_DEBUG("invcount");
				if (Utilities::GetInventoryItemCount((*g_player)->inventoryList, ammoMod->ammo) == 0)
					return nullptr;
			}
			//BGSMod::Attachment::Mod* currSwitchedMod = Utilities::FindModByUniqueKeyword(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::hasSwitchedAmmoKW);
			BGSMod::Attachment::Mod* currSwitchedMod = Utilities::GetModAtAttachPoint(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::ammoAP);
			if (currSwitchedMod == ammoMod->mod)
				return nullptr;
			SwitchData* switchData = new SwitchData();
			switchData->ModToAttach = ammoMod->mod;
			switchData->targetAmmo = ammoMod->ammo;
			_DEBUG("retOK");
			return switchData;
		}
		return nullptr;
	}

	SwitchData* GetModForAmmo(TESAmmo* targetAmmo)
	{
		if (!targetAmmo)
			return nullptr;
		BGSObjectInstanceExtra* moddata = Utilities::GetEquippedModData(*g_player, 41);
		TESObjectWEAP* baseWeapon = Utilities::GetEquippedGun(*g_player);
		TESAmmo* baseAmmo = MSF_Data::GetBaseCaliber(moddata, baseWeapon);
		if (!baseAmmo)
			return nullptr;
		_DEBUG("baseOK");
		auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
		if (itAD != MSF_MainData::ammoDataMap.end())
		{
			_DEBUG("ammofound");
			AmmoData* itAmmoData = itAD->second;
			if (targetAmmo == baseAmmo)
			{
				BGSMod::Attachment::Mod* currSwitchedMod = Utilities::GetModAtAttachPoint(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::ammoAP);
				if (!currSwitchedMod)
					return nullptr;
				SwitchData* switchData = new SwitchData();
				switchData->ModToRemove = currSwitchedMod;
				switchData->targetAmmo = baseAmmo;
				return switchData;
			}
			for (auto itAmmoMod : itAmmoData->ammoMods)
			{
				if (itAmmoMod.ammo == targetAmmo)
				{
					BGSMod::Attachment::Mod* currSwitchedMod = Utilities::GetModAtAttachPoint(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::ammoAP);
					if (currSwitchedMod == itAmmoMod.mod)
						return nullptr;
					SwitchData* switchData = new SwitchData();
					switchData->ModToAttach = itAmmoMod.mod;
					switchData->targetAmmo = itAmmoMod.ammo;
					_DEBUG("retOK");
					return switchData;
				}
			}
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
		if (!attachedMods)
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

		return QueueModsToSwitch(modToAttach, modToRemove);
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
		if (!attachedMods)
			return false;

		_DEBUG("check OK");
		std::vector<KeywordValue> instantiationValues;
		if (!Utilities::GetParentInstantiationValues(attachedMods, modData->attachParentValue, &instantiationValues))
			return false;
		_DEBUG("if counts: %i / %i", instantiationValues.size(), instantiationValues.capacity());

		ModData::ModCycle* modCycle = nullptr;
		for (std::vector<KeywordValue>::iterator itData = instantiationValues.begin(); itData != instantiationValues.end(); itData++)
		{
			KeywordValue value = *itData;
			_DEBUG("it: %i", value);
			auto itCycle = modData->modCycleMap.find(value);
			if (itCycle != modData->modCycleMap.end())
			{
				if (modCycle)
				{
					_MESSAGE("Ambiguity error");
					return false;
				}
				modCycle = itCycle->second;
			}
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
		{
			if (modCycle->flags & ModData::ModCycle::bCannotHaveNullMod)
				modToAttach = modCycle->mods[1];
			else
				modToAttach = modCycle->mods[0];
		}
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

		return QueueModsToSwitch(modToAttach, modToRemove);
	}

	bool CheckSwitchRequirements(BGSInventoryItem::Stack* stack, ModData::Mod* modToAttach, ModData::Mod* modToRemove)
	{
		return true;
	}

	bool QueueModsToSwitch(ModData::Mod* modToAttach, ModData::Mod* modToRemove)
	{
		_DEBUG("Queueing");
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
					Utilities::SendNotification("Missing required loose mod.");
					return false;
				}
			}
			if (!(modToAttach->flags & SwitchData::bIgnoreAnimations) || ((!switchRemove && modToRemove) && !(modToRemove->flags & SwitchData::bIgnoreAnimations)))
			{
				_DEBUG("Pending");
				if (!MSF_Base::HandlePendingAnimations())
				{
					delete switchRemove;
					return false;
				}
				_DEBUG("PendingOK");
			}
			switchAttach = new SwitchData();
			switchAttach->LooseModToRemove = looseMod;
			switchAttach->ModToAttach = modToAttach->mod;
			switchAttach->SwitchFlags = modToAttach->flags & ModData::Mod::mBitTransferMask;
			if (!switchRemove && modToRemove)
			{
				switchAttach->ModToRemove = modToRemove->mod;
				switchAttach->LooseModToAdd = (modToRemove->flags & ModData::Mod::bRequireLooseMod) ? Utilities::GetLooseMod(modToRemove->mod) : nullptr;
				switchAttach->SwitchFlags &= ~(modToRemove->flags & ModData::Mod::bNotRequireWeaponToBeDrawn);
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

		if ((*g_player)->actorState.IsWeaponDrawn() || (switchData->SwitchFlags & ModData::Mod::bNotRequireWeaponToBeDrawn))
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

	TESAmmo* GetBaseCaliber(BGSObjectInstanceExtra* objectModData, TESObjectWEAP* weapBase)
	{
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
			//bool isSwitchedAmmo = false;
			for (UInt32 i4 = 0; i4 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i4];
				if (data->target == 61 && data->value.form && data->op == 128)
				{
					if (currPriority >= priority)
						currModAmmo = (TESAmmo*)data->value.form;
				}
				//else if (data->target == 31 && data->value.form && data->op == 144)
				//{
				//	if ((BGSKeyword*)data->value.form == MSF_MainData::hasSwitchedAmmoKW)
				//		isSwitchedAmmo = true;
				//}
			}
			//if (!isSwitchedAmmo && currModAmmo)
			if (objectMod->unkC0 != MSF_MainData::ammoAP && currModAmmo)
			{
				ammoConverted = currModAmmo;
				priority = currPriority;
			}
		}

		if (!ammoConverted)
			return weapBase->weapData.ammo;
		return ammoConverted;
	}

	bool GetChamberData(BGSObjectInstanceExtra* mods, TESObjectWEAP::InstanceData* weapInstance, UInt16* chamberSize, UInt16* flags)
	{
		if (!mods || !weapInstance || !weapInstance->ammo || !chamberSize || !flags)
			return false;
		auto data = mods->data;
		if (!data || !data->forms)
			return false;
		UInt64 priority = 0;
		ChamberData* currChamberData = nullptr;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
			if (currPriority < priority)
				continue;
			auto itmoddata = MSF_MainData::modChamberMap.find(objectMod);
			if (itmoddata == MSF_MainData::modChamberMap.end())
				continue;
			currChamberData = &itmoddata->second;
			priority = currPriority;
		}
		if (!currChamberData)
		{
			if (Utilities::WeaponInstanceHasKeyword(weapInstance, MSF_MainData::BallisticWeaponKW))
				*chamberSize = 1;
			else
				*chamberSize = 0;
			return true;
		}
		if (currChamberData->chamberSize == -1)
			*chamberSize = weapInstance->ammoCapacity;
		else
			*chamberSize = currChamberData->chamberSize;
		*flags = (*flags & ~ExtraWeaponState::WeaponState::mChamberMask) | currChamberData->flags;
		return true;
	}

	bool GetAttachedChildren(BGSObjectInstanceExtra* mods, BGSMod::Attachment::Mod* parent, std::vector<BGSMod::Attachment::Mod*>* children, bool checkIF)
	{
		if (!children || !mods || !parent)
			return false;
		AttachParentArray* attachParentArray = reinterpret_cast<AttachParentArray*>(&parent->unk98);
		KeywordValueArray* instantiationData = reinterpret_cast<KeywordValueArray*>(&parent->unkB0);
		for (UInt32 idx = 0; idx < attachParentArray->kewordValueArray.count; idx++)
		{
			KeywordValue ap = attachParentArray->kewordValueArray[idx];
			BGSMod::Attachment::Mod* child = Utilities::GetModAtAttachPoint(mods, ap);
			if (checkIF)
			{
				auto range = MSF_MainData::instantiationRequirements.equal_range(child);
				auto it = range.first;
				if (it != range.second)
				{
					for (it; it != range.second; ++it)
					{
						if (instantiationData->GetItemIndex(it->second) >= 0)
						{
							//_DEBUG("parentOK");
							children->push_back(child);
							GetAttachedChildren(mods, child, children, checkIF);
							break;
						}
					}
				}
				else
				{
					children->push_back(child);
					GetAttachedChildren(mods, child, children, checkIF);
				}
			}
			else
			{
				children->push_back(child);
				GetAttachedChildren(mods, child, children, checkIF);
			}
		}
		return true;
	}

	bool PickRandomMods(std::vector<BGSMod::Attachment::Mod*>* mods, TESAmmo** ammo, UInt32* count)
	{
		TESAmmo* baseAmmo = *ammo;
		*ammo = nullptr;
		UInt32 chance = 0;
		if (baseAmmo && (MSF_MainData::MCMSettingFlags & MSF_MainData::bSpawnRandomAmmo))
		{
			auto itAD = MSF_MainData::ammoDataMap.find(baseAmmo);
			if (itAD != MSF_MainData::ammoDataMap.end())
			{

				AmmoData* ammoData = itAD->second;

				std::mt19937 generator(std::random_device{}());
				std::vector<double> chances;

				float chanceMultiplier = MSF_MainData::MCMfloatSettingMap["fBaseChanceMultiplier"];
				if (chanceMultiplier < 0.0)
					chanceMultiplier = 1.0;
				chances.push_back(ammoData->baseAmmoData.spawnChance * chanceMultiplier);
				for (std::vector<AmmoData::AmmoMod>::iterator itAmmoMod = ammoData->ammoMods.begin(); itAmmoMod != ammoData->ammoMods.end(); itAmmoMod++)
					chances.push_back(itAmmoMod->spawnChance);

				std::discrete_distribution<> distribution(chances.begin(), chances.end());
				int idx = distribution(generator);

				if (idx > 0)
				{
					idx--;
					AmmoData::AmmoMod* chosenAmmoMod = &ammoData->ammoMods[idx];

					mods->push_back(chosenAmmoMod->mod);
					*ammo = chosenAmmoMod->ammo;
					*count = rand() % (MSF_MainData::iMaxRandomAmmo - MSF_MainData::iMinRandomAmmo + 1) + MSF_MainData::iMinRandomAmmo;

				}

				return true;
			}
		}
		// mod association

		return false;
	}

	//========================= Animations =======================
	TESIdleForm* GetReloadAnimation(Actor* actor)
	{
		if (!actor || !actor->biped.get())
			return nullptr;
		SInt32 state = 8;
		if (actor == *g_player)
		{
			PlayerCamera* playerCamera = *g_playerCamera;
			SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		}
		TESObjectWEAP* equippedWeap = DYNAMIC_CAST(actor->biped.get()->object[41].parent.object, TESForm, TESObjectWEAP);
		if (!equippedWeap)
			return nullptr;
		bool isInPA = IsInPowerArmor(actor);
		auto itAnim = MSF_MainData::reloadAnimDataMap.find(equippedWeap);
		if (itAnim != MSF_MainData::reloadAnimDataMap.end())
		{
			if (state == 0)
			{
				if (isInPA)
					return itAnim->second->animIdle_1stP_PA;
				else
					return itAnim->second->animIdle_1stP;
			}
			else if (state == 8)
			{
				if (isInPA)
					return itAnim->second->animIdle_3rdP_PA;
				else
					return itAnim->second->animIdle_3rdP;
			}
			return nullptr;
		}
		if (state == 0)
			return MSF_MainData::reloadIdle1stP;
		else if (state == 8)
			return MSF_MainData::reloadIdle3rdP;
		return nullptr;
	}

	TESIdleForm* GetFireAnimation(Actor* actor)
	{
		if (!actor || !actor->biped.get())
			return nullptr;
		SInt32 state = 8;
		if (actor == *g_player)
		{
			PlayerCamera* playerCamera = *g_playerCamera;
			SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		}
		TESObjectWEAP* equippedWeap = DYNAMIC_CAST(actor->biped.get()->object[41].parent.object, TESForm, TESObjectWEAP);
		if (!equippedWeap)
			return nullptr;
		bool isInPA = IsInPowerArmor(actor);
		auto itAnim = MSF_MainData::fireAnimDataMap.find(equippedWeap);
		if (itAnim != MSF_MainData::fireAnimDataMap.end())
		{
			if (state == 0)
			{
				if (isInPA)
					return itAnim->second->animIdle_1stP_PA;
				else
					return itAnim->second->animIdle_1stP;
			}
			else if (state == 8)
			{
				if (isInPA)
					return itAnim->second->animIdle_3rdP_PA;
				else
					return itAnim->second->animIdle_3rdP;
			}
			return nullptr;
		}
		if (state == 0)
			return MSF_MainData::fireIdle1stP;
		else if (state == 8)
			return MSF_MainData::fireIdle3rdP;
		return nullptr;
	}

	bool InstanceHasBCRSupport(TESObjectWEAP::InstanceData* instance)
	{
		if (!instance || !MSF_MainData::BCR_AVIF || !MSF_MainData::BCR_AVIF2)
			return false;
		return instance->skill == MSF_MainData::BCR_AVIF || instance->skill == MSF_MainData::BCR_AVIF2;
	}

	bool WeaponHasBCRSupport(TESObjectWEAP* weapon)
	{
		if (!weapon || !MSF_MainData::BCR_AVIF || !MSF_MainData::BCR_AVIF2)
			return false;
		return weapon->weapData.skill == MSF_MainData::BCR_AVIF || weapon->weapData.skill == MSF_MainData::BCR_AVIF2;
	}

	bool InstanceHasTRSupport(TESObjectWEAP::InstanceData* instance)
	{
		if (!instance || !MSF_MainData::tacticalReloadKW)
			return false;
		return Utilities::WeaponInstanceHasKeyword(instance, MSF_MainData::tacticalReloadKW);
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

BGSProjectile* CustomProjectileFormManager::Clone(BGSProjectile* proj)
{
	BGSProjectile* result = nullptr;
	/*if (!proj)
		return result;
	void* memory = Heap_Allocate(sizeof(Projectile));
	errno_t error = memcpy_s(memory, sizeof(Projectile), proj, sizeof(Projectile));
	if (error)
		Heap_Free(memory);
	else
	{
		result = (BGSProjectile*)memory;
		result->formID = 0;
		//result->formType = 0x92;
		result->unk08 = nullptr;
		result->flags = 0x00004008;
		result->unk18 = 0;
	}*/
	result = (BGSProjectile*)CreateDuplicateForm(proj, true, nullptr);
	errno_t error = memcpy_s(&result->bounds1, sizeof(Projectile)-offsetof(Projectile, bounds1), &proj->bounds1, sizeof(Projectile)-offsetof(Projectile, bounds1));
	if (error)
	{
		Cleanup(result);
		return nullptr;
	}
	return result;
}

void CustomProjectileFormManager::Cleanup(BGSProjectile* proj)
{
	//Heap_Free(proj);
	SetDeleteForm(proj, true);
	//2C727C8                 dq offset sub_140153D20
}

bool CustomProjectileFormManager::ClearData()
{
	for (auto entry : projectileParentMap)
		Cleanup(entry.second.first);
	projectileParentMap.clear();
}

bool CustomProjectileFormManager::ApplyMods(ExtraDataList* extraDataList)
{
	if (!extraDataList)
		return false;
	auto itOldProj = projectileParentMap.find(extraDataList);
	TESObjectWEAP::InstanceData* instanceData = nullptr;
	ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
	if (!extraInstanceData)
		return ReturnCleanup(itOldProj, instanceData);
	instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	if (!instanceData || !instanceData->firingData)
		return ReturnCleanup(itOldProj, instanceData);
	TESObjectWEAP* baseWeap = DYNAMIC_CAST(extraInstanceData->baseForm, TESForm, TESObjectWEAP);
	if (!baseWeap)
		return ReturnCleanup(itOldProj, instanceData);
	BGSObjectInstanceExtra* attachedMods = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
	if (!attachedMods)
		return ReturnCleanup(itOldProj, instanceData);
	auto data = attachedMods->data;
	if (!data || !data->forms)
		return ReturnCleanup(itOldProj, instanceData);
	std::vector<BGSMod::Attachment::Mod*> objMods;
	for (UInt32 idx = 0; idx < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); idx++)
	{
		BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[idx].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
		objMods.push_back(mod);
	}
	std::sort(objMods.begin(), objMods.end(),
		[](BGSMod::Attachment::Mod* a, BGSMod::Attachment::Mod* b){
			return a->priority < b->priority;
		});
	std::vector<std::vector<BGSMod::Container::Data>*> toModify;
	for (auto mod : objMods)
	{
		auto itData = projectileModMap.find(mod);
		if (itData != projectileModMap.end())
		{
			toModify.push_back(&itData->second);
		}
	}
	if (toModify.size() == 0)
	{
		return ReturnCleanup(itOldProj, instanceData);
	}
	BGSProjectile* baseProj = instanceData->firingData->projectileOverride; //priority order?
	if (!baseProj && baseWeap->weapData.ammo)
	{
		Ammo* ammo = (Ammo*)baseWeap->weapData.ammo;
		baseProj = ammo->data.projectile;
	}
	//else
	//	return ReturnCleanup(itOldProj, instanceData);
	if (!baseProj)
	{
		return ReturnCleanup(itOldProj, instanceData);
	}
	Projectile* newProj = (Projectile*)baseProj;
	if (newProj->formID >> 0x18 != 0xFF)
		newProj = (Projectile*)Clone(baseProj);
	if (!newProj)
	{
		return ReturnCleanup(itOldProj, instanceData);
	}
	float fv1 = 0.0;
	UInt32 iv1 = 0;
	for (auto vec : toModify)
	{
		for (auto data : *vec)
		{
			switch (data.op)
			{
			case BGSMod::Container::Data::kOpFlag_Set_Bool:
			{
				iv1 = data.value.i.v1;
				//iv2 = data->value.i.v2;
				if (data.target == kWeaponTarget_TracerFreq)
					newProj->data.tracerFrequency = iv1;

			}
			break;
			case BGSMod::Container::Data::kOpFlag_Add_Int:
			{
				iv1 = data.value.i.v1;
				//iv2 = data->value.i.v2;
				if (data.target == kWeaponTarget_TracerFreq)
				{
					if (iv1 < 0 && newProj->data.tracerFrequency < iv1)
						newProj->data.tracerFrequency = 0;
					else
					{
						newProj->data.tracerFrequency += iv1;
						if (newProj->data.tracerFrequency > 255)
							newProj->data.tracerFrequency = 255;
					}
				}
			}
			break;
			case BGSMod::Container::Data::kOpFlag_Set_Float:
			{
				fv1 = data.value.f.v1;
				//fv2 = data->value.f.v2;
				switch (data.target)
				{
				case kWeaponTarget_Gravity: newProj->data.gravity = fv1; break;
				case kWeaponTarget_Speed: newProj->data.speed = fv1; break;
				case kWeaponTarget_Range: newProj->data.range = fv1; break;
				case kWeaponTarget_ExpProximity: newProj->data.explosionProximity = fv1; break;
				case kWeaponTarget_ExpTimer: newProj->data.explosionTimer = fv1; break;
				case kWeaponTarget_MuzzleFlashDur: newProj->data.muzzleFlashDuration = fv1; break;
				case kWeaponTarget_FadeOutTime: newProj->data.fadeOutTime = fv1; break;
				case kWeaponTarget_Force: newProj->data.force = fv1; break;
				case kWeaponTarget_ConeSpread: newProj->data.coneSpread = fv1; break;
				case kWeaponTarget_CollRadius: newProj->data.collisionRadius = fv1; break;
				case kWeaponTarget_Lifetime: newProj->data.lifetime = fv1; break;
				case kWeaponTarget_RelaunchInt: newProj->data.relaunchInterval = fv1; break;
				}
			}
			break;
			case BGSMod::Container::Data::kOpFlag_Mul_Add_Float:
			{
				fv1 = data.value.f.v1;
				//fv2 = data->value.f.v2;
				switch (data.target)
				{
				case kWeaponTarget_TracerFreq: 
				{
					float res = newProj->data.tracerFrequency * fv1;
					if (res < 0)
						newProj->data.tracerFrequency = 0;
					else if (res > 255)
						newProj->data.tracerFrequency = 255;
					else
						newProj->data.tracerFrequency = roundp(res);
				}
				break;
				case kWeaponTarget_Gravity: newProj->data.gravity = newProj->data.gravity * fv1; break;
				case kWeaponTarget_Speed: 
				{
					newProj->data.speed = newProj->data.speed * fv1;
					if (newProj->data.speed < 0)
						newProj->data.speed = 0.0;
					else if (newProj->data.speed > 1000000000.00)
						newProj->data.speed = 1000000000.00;
				}
				break;
				case kWeaponTarget_Range: 
				{
					newProj->data.range = newProj->data.range * fv1;
					if (newProj->data.range < 0)
						newProj->data.range = 0.0;
				}
				break;
				case kWeaponTarget_ExpProximity: newProj->data.explosionProximity = newProj->data.explosionProximity * fv1; break;
				case kWeaponTarget_ExpTimer: 
				{
					newProj->data.explosionTimer = newProj->data.explosionTimer * fv1;
					if (newProj->data.explosionTimer < 0)
						newProj->data.explosionTimer = 0.0;
				}
				break;
				case kWeaponTarget_MuzzleFlashDur: 
				{
					newProj->data.muzzleFlashDuration = newProj->data.muzzleFlashDuration * fv1;
					if (newProj->data.muzzleFlashDuration < 0.0)
						newProj->data.muzzleFlashDuration = 0.0;
					else if (newProj->data.muzzleFlashDuration > 5.0)
						newProj->data.muzzleFlashDuration = 5.0;
				}
				break;
				case kWeaponTarget_FadeOutTime: newProj->data.fadeOutTime = newProj->data.fadeOutTime * fv1; break;
				case kWeaponTarget_Force: newProj->data.force = newProj->data.force * fv1; break;
				case kWeaponTarget_ConeSpread: newProj->data.coneSpread = newProj->data.coneSpread * fv1; break;
				case kWeaponTarget_CollRadius: newProj->data.collisionRadius = newProj->data.collisionRadius * fv1; break;
				case kWeaponTarget_Lifetime: newProj->data.lifetime = newProj->data.lifetime * fv1; break;
				case kWeaponTarget_RelaunchInt: 
				{
					newProj->data.relaunchInterval = newProj->data.relaunchInterval * fv1;
					if (newProj->data.relaunchInterval < 0.0010)
						newProj->data.relaunchInterval = 0.0010;
					else if (newProj->data.relaunchInterval > 5.0)
						newProj->data.relaunchInterval = 5.0;
				}
				break;
				}
			}
			break;
			case BGSMod::Container::Data::kOpFlag_Add_Float:
			{
				fv1 = data.value.f.v1;
				//fv2 = data->value.f.v2;
				switch (data.target)
				{
				case kWeaponTarget_Gravity: newProj->data.gravity += fv1; break;
				case kWeaponTarget_Speed: 
				{
					newProj->data.speed += fv1;
					if (newProj->data.speed < 0)
						newProj->data.speed = 0.0;
					else if (newProj->data.speed > 1000000000.00)
						newProj->data.speed = 1000000000.00;
				}
				break;
				case kWeaponTarget_Range: 
				{
					newProj->data.range += fv1;
					if (newProj->data.range < 0)
						newProj->data.range = 0.0;
				}
				break;
				case kWeaponTarget_ExpProximity: newProj->data.explosionProximity += fv1; break;
				case kWeaponTarget_ExpTimer: 
				{
					newProj->data.explosionTimer += fv1;
					if (newProj->data.explosionTimer < 0)
						newProj->data.explosionTimer = 0.0;
				}
				break;
				case kWeaponTarget_MuzzleFlashDur: 
				{
					newProj->data.muzzleFlashDuration += fv1;
					if (newProj->data.muzzleFlashDuration < 0.0)
						newProj->data.muzzleFlashDuration = 0.0;
					else if (newProj->data.muzzleFlashDuration > 5.0)
						newProj->data.muzzleFlashDuration = 5.0;
				}
				break;
				case kWeaponTarget_FadeOutTime: newProj->data.fadeOutTime += fv1; break;
				case kWeaponTarget_Force: newProj->data.force += fv1; break;
				case kWeaponTarget_ConeSpread: newProj->data.coneSpread += fv1; break;
				case kWeaponTarget_CollRadius: newProj->data.collisionRadius += fv1; break;
				case kWeaponTarget_Lifetime: newProj->data.lifetime += fv1; break;
				case kWeaponTarget_RelaunchInt: 
				{
					newProj->data.relaunchInterval += fv1;
					if (newProj->data.relaunchInterval < 0.0010)
						newProj->data.relaunchInterval = 0.0010;
					else if (newProj->data.relaunchInterval > 5.0)
						newProj->data.relaunchInterval = 5.0;
				}
				break;
				}
			}
			break;
			case BGSMod::Container::Data::kOpFlag_Set_Enum:
			{
				iv1 = data.value.i.v1;
				//iv2 = data->value.i.v2;
				if (data.target == kWeaponTarget_SoundLevel)
					newProj->soundLevel = iv1;
				else if (data.target == kWeaponTarget_Type)
					newProj->data.type = iv1;
			}
			break;
			case BGSMod::Container::Data::kOpFlag_Set_Form:
			case BGSMod::Container::Data::kOpFlag_Add_Form:
			case BGSMod::Container::Data::kOpFlag_Rem_Form:
			{
				switch (data.target)
				{
				case kWeaponTarget_Light: newProj->data.light = (TESObjectLIGH*)data.value.form; break;
				case kWeaponTarget_MuzzleFlashLight: newProj->data.muzzleFlashLight = (TESObjectLIGH*)data.value.form; break;
				case kWeaponTarget_ExpType: newProj->data.explosionType = (BGSExplosion*)data.value.form; break;
				case kWeaponTarget_ActSoundLoop: newProj->data.activeSoundLoop = (BGSSoundDescriptorForm*)data.value.form; break;
				case kWeaponTarget_CountdownSound: newProj->data.countdownSound = (BGSSoundDescriptorForm*)data.value.form; break;
				case kWeaponTarget_DeactivateSound: newProj->data.deactivateSound = (BGSSoundDescriptorForm*)data.value.form; break;
				case kWeaponTarget_DecalData: newProj->data.decalData = (BGSTextureSet*)data.value.form; break;
				case kWeaponTarget_CollisionLayer: newProj->data.collisionLayer = data.value.form; break;
				case kWeaponTarget_VATSprojectile: newProj->data.vatsProjectile = (BGSProjectile*)data.value.form; break;
				case kWeaponTarget_Model: newProj->model = *(TESModel*)data.value.form; break;
				case kWeaponTarget_MuzzleFlashModel: newProj->muzzleFlashModel = *(TESModel*)data.value.form; break;
					//dest
				}
			}
			break;
			case BGSMod::Container::Data::kOpFlag_Set_Int:
			case BGSMod::Container::Data::kOpFlag_Or_Bool:
			case BGSMod::Container::Data::kOpFlag_And_Bool:
			{
				UInt8 op = data.op;
				switch (data.target)
				{
				case kWeaponTarget_Supersonic: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bSupersonic, data.value.i.v1, op); break;
				case kWeaponTarget_MuzzleFlash: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bMuzzleFlash, data.value.i.v1, op); break;
				case kWeaponTarget_Explosion: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bExplosion, data.value.i.v1, op); break;
				case kWeaponTarget_AltTrigger: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bAltTrigger, data.value.i.v1, op); break;
				case kWeaponTarget_Hitscan: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bHitscan, data.value.i.v1, op); break;
				case kWeaponTarget_CanBeDisabled: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bCanBeDisabled, data.value.i.v1, op); break;
				case kWeaponTarget_CanBePickedUp: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bCanBePickedUp, data.value.i.v1, op); break;
				case kWeaponTarget_PinsLimbs: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bPinsLimbs, data.value.i.v1, op); break;
				case kWeaponTarget_PassThroughObjects: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bPassThroughObjects, data.value.i.v1, op); break;
				case kWeaponTarget_DisableAimCorr: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bDisableAimCorr, data.value.i.v1, op); break;
				case kWeaponTarget_PenetratesGeom: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bPenetratesGeom, data.value.i.v1, op); break;
				case kWeaponTarget_ContinuousUpdate: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bContinuousUpdate, data.value.i.v1, op); break;
				case kWeaponTarget_SeeksTarget: newProj->data.flags = Utilities::AddRemFlag(newProj->data.flags, BGSProjectileData::bSeeksTarget, data.value.i.v1, op); break;
				}
			}
			break;

			}
		}
	}

	if (itOldProj != projectileParentMap.end())
	{
		Cleanup(itOldProj->second.first); //delay heapfree, oncellunload / onmenuclosed(loadingscreen)
		itOldProj->second.second = instanceData->firingData->projectileOverride;
		itOldProj->second.first = (BGSProjectile*)newProj;
		instanceData->firingData->projectileOverride = (BGSProjectile*)newProj;
		return true;
	}
	projectileParentMap[extraDataList] = { (BGSProjectile*)newProj, instanceData->firingData->projectileOverride };
	instanceData->firingData->projectileOverride = (BGSProjectile*)newProj;
	return true;
}

bool CustomProjectileFormManager::ReturnCleanup(std::unordered_map<ExtraDataList*, std::pair<BGSProjectile*, BGSProjectile*>>::iterator foundData, TESObjectWEAP::InstanceData* instance) //delay heapfree
{
	if (foundData == projectileParentMap.end())
	{
		if (!instance || !instance->firingData || !instance->firingData->projectileOverride || (instance->firingData->projectileOverride->formID >> 0x18) != 0xFF)
			return true;
		Cleanup(instance->firingData->projectileOverride);
		instance->firingData->projectileOverride = nullptr;
		return true;
	}
	if (instance && instance->firingData && instance->firingData->projectileOverride && (instance->firingData->projectileOverride->formID >> 0x18) == 0xFF)
	{
		if (instance->firingData->projectileOverride != foundData->second.first)
		{
			Cleanup(foundData->second.first);
			projectileParentMap.erase(foundData);
		}
		Cleanup(instance->firingData->projectileOverride);
		instance->firingData->projectileOverride = foundData->second.second;
		projectileParentMap.erase(foundData);
		return true;
	}
	Cleanup(foundData->second.first);
	projectileParentMap.erase(foundData);
	return true;
}

bool CustomProjectileFormManager::AddModData(BGSMod::Attachment::Mod* mod, BGSMod::Container::Data data, bool overwrite)
{
	auto entries = &projectileModMap[mod];
	for (auto entry : *entries)
	{
		if (entry.target == data.target)
		{
			if (!overwrite)
				return false;
			entry.op = data.op;
			entry.value = data.value;
			return true;
		}
	}
	entries->push_back(data);
	return true;
}
