#pragma once
#include "MSF_Shared.h"

class PlayerInventoryListEventSink;
class ActorEquipManagerEventSink;
class WeaponStateStore;

class AmmoData
{
public:
	struct AmmoMod
	{
		TESAmmo* ammo;
		BGSMod::Attachment::Mod* mod;
		UInt16 ammoID;
		float spawnChance;
	};
	AmmoMod baseAmmoData;
	std::vector<AmmoMod> ammoMods;
};

class AnimationData
{
public:
	AnimationData(TESIdleForm* animIdle_1stP_in, TESIdleForm* animIdle_3rdP_in, TESIdleForm* animIdle_1stP_PA_in, TESIdleForm* animIdle_3rdP_PA_in)
	{
		animIdle_1stP = animIdle_1stP_in; animIdle_3rdP = animIdle_3rdP_in; animIdle_1stP_PA = animIdle_1stP_PA_in; animIdle_3rdP_PA = animIdle_3rdP_PA_in;
	};
	TESIdleForm* GetAnimation()
	{
		PlayerCamera* playerCamera = *g_playerCamera;
		SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		bool isInPA = IsInPowerArmor(*g_player);
		if (state == 0)
		{
			if (isInPA)
				return animIdle_1stP_PA;
			else
				return animIdle_1stP;
		}
		else if (state == 7 || state == 8)
		{
			if (isInPA)
				return animIdle_3rdP_PA;
			else
				return animIdle_3rdP;
		}
		return nullptr;
	};
	TESIdleForm* animIdle_1stP;
	TESIdleForm* animIdle_3rdP;
	TESIdleForm* animIdle_1stP_PA;
	TESIdleForm* animIdle_3rdP_PA;
	//BGSAction* animAction;
};

class ModData
{
public:
	class Mod
	{
	public:
		enum
		{
			bStandaloneAttach = 0x0001,
			bStandaloneRemove = 0x0002,
			bHasSecondaryAmmo = 0x0004,
			bRequireWeaponToBeDrawn = 0x1000,
			bRequireLooseMod = 0x2000,
			bUpdateAnimGraph = 0x4000,
			bIgnoreAnimations = 0x8000,
			mBitTransferMask = 0xF000
		};
		class AttachRequirements
		{
		public:
			bool CheckRequirements(ExtraDataList* extraData);
			struct Requirements
			{
				std::vector<TESObjectWEAP*> weapons;
				std::vector<BGSMod::Attachment::Mod*> mods;
				std::vector<BGSKeyword*> keywords;
			};
			Requirements requirements;
			Requirements lackrequirements;
		};
		BGSMod::Attachment::Mod* mod;
		UInt16 flags;
		KeywordValue animFlavor;
		float spawnChance;
		AttachRequirements* attachRequirements;
		AnimationData* animData;
	};
	typedef std::vector<Mod*> ModVector;
	struct ModCycle
	{
		enum
		{
			bCannotHaveNullMod = 0x0001,
			bUIRequireWeaponToBeDrawn = 0x0002
		};
		ModVector mods;
		UInt32 flags;
		float spawnChanceBase;
	};
	enum
	{
		bRequireAPmod = 0x0001
	};
	UInt16 attachParentValue;
	UInt16 flags;
	Mod::AttachRequirements* APattachRequirements;
	std::unordered_map<KeywordValue, ModCycle*> modCycleMap; //hash: attachparent mod's instantiation keyword value, bucket: modcycle; if multiple cycles are found ambiuguity error is thrown
};

class ModCompatibilityEdits
{
public:
	KeywordValue attachParent;
	std::vector<KeywordValue> removedAPslots;
	std::vector<KeywordValue> addedAPslots;
	std::vector<KeywordValue> removedFilters;
	std::vector<KeywordValue> addedFilters;
};

class BurstModeData
{
public:
	BurstModeData(UInt32 delay, UInt8 settings, UInt8 totalShots)
	{
		delayTime = delay;
		flags = settings;
		numOfTotalShots = totalShots;
	};
	enum
	{
		bOnePullBurst = 0x01, //If FALSE, upon releasing the trigger the firing is stopped; if TRUE, all the shots will be fired in a burst
		bResetShotCountOnRelease = 0x02, //If TRUE, upon releasing the trigger the shot count will reset; if FALSE, the shot count will not reset(only applies if bOnePullBurst is FALSE)
		bResetShotCountOnReload = 0x04,
		bTypeAuto = 0x08,
		bActive = 0x10
	};
	UInt32 delayTime; //Interval between two shots in a single burst(in milliseconds)
	UInt8 flags;
	UInt8 numOfTotalShots; //Number of shots fired during a single burst
protected:
	BurstModeData() {}
};

class BurstModeManager : public BurstModeData
{
public:
	//BurstModeManager(BurstModeData* templateData, UInt8 bActive) : BurstModeData(templateData->delayTime, templateData->flags, templateData->numOfTotalShots) { numOfShotsFired = 0; SetState(bActive); }
	BurstModeManager(BurstModeData* templateData, bool bActive) { delayTime = templateData->delayTime; flags = templateData->flags; numOfTotalShots = templateData->numOfTotalShots;  numOfShotsFired = 0; SetState(bActive << 4); };
	bool HandleFireEvent();
	bool HandleReleaseEvent();
	bool ResetShotsOnReload();
	bool FireWeapon();
	bool HandleEquipEvent(TESObjectWEAP::InstanceData* weaponInstance); //ExtraDataList* extraDataList
	bool HandleModChangeEvent(ExtraDataList* extraDataList);
	bool SetState(UInt8 bActive);
private:
	volatile short numOfShotsFired;
};

class HUDDisplayData
{
public:
	BGSKeyword* keyword;
	std::string displayString;
};
class HUDFiringModeData : public HUDDisplayData {};
class HUDScopeData : public HUDDisplayData {};
class HUDMuzzleData : public HUDDisplayData {};

class ModSelectionMenu
{
public:
	std::string scaleformName;
	UInt8 type;
	UInt16 flags;
	UInt32 version;
	AnimationData* menuAnimation;
	enum
	{
		kType_Widget = 0,
		kType_AmmoMenu = 1,
		kType_ModMenu = 2,
		kType_All = 3
	};
	ModSelectionMenu(std::string name, UInt8 menuType){
		scaleformName = name; type = menuType; version = 0; flags = 0; menuAnimation = nullptr;
	};
};

class KeybindData
{
public:

	enum
	{
		//lower 4 bits: Nth
		bGlobalMenu = 0x10,
		bToggle = 0x20,
		bHUDselection = 0x40,
		bIsAmmo = 0x80,
		mNumMask = 0x0F
	};
	std::string functionID;
	UInt8 type;
	UInt16 keyCode;
	UInt8 modifiers;
	ModSelectionMenu* selectMenu;
	ModData* modData;
};

class MCMfloatData
{
public:
	std::string name;
	float value;
};

class SwitchData
{
public:
	SwitchData()
	{
		SwitchFlags = 0;
		ModToAttach = nullptr;
		ModToRemove = nullptr;
		LooseModToRemove = nullptr;
		LooseModToAdd = nullptr;
		animFlavor = 0;
		animData = nullptr;
	};
	enum
	{
		bNeedInit = 0x0001,
		bSwitchingInProgress = 0x0002,
		bQueuedHUDSelection = 0x0004,
		bSetChamberedAmmo = 0x0008,
		bDrawNeeded = 0x0080,
		bDrawInProgress = 0x0800,
		bReloadNeeded = 0x0010,
		bReloadInProgress = 0x0020,
		bReloadNotFinished = 0x0040,
		bAnimNeeded = 0x0100,
		bAnimInProgress = 0x0200,
		bAnimNotFinished = 0x0400,
		bDrawEnabled = 0x1000,
		bSetLooseMods = 0x2000,
		bUpdateAnimGraph = 0x4000,
		bIgnoreAnimations = 0x8000
	};
	UInt16 SwitchFlags;
	KeywordValue animFlavor;
	BGSMod::Attachment::Mod* ModToAttach;
	BGSMod::Attachment::Mod* ModToRemove;
	TESObjectMISC* LooseModToRemove;
	TESObjectMISC* LooseModToAdd;
	AnimationData* animData;
};

class ModSwitchManager
{
private:
	volatile UInt16 ignoreAnimGraphUpdate;
	volatile UInt16 ignoreDeleteExtraData;
	volatile UInt16 equipEvent;
	volatile UInt16 switchState;
	SimpleLock queueLock;
	std::vector<SwitchData*> switchDataQueue;

	ModSelectionMenu* volatile openedMenu;
	volatile UInt16 numberOfOpenedMenus;
	std::vector<AmmoData::AmmoMod*> displayedAmmoChoices;
	std::vector<ModData::Mod*> displayedModChoices;

	TESObjectWEAP::InstanceData* volatile equippedInstanceData;
public:
	SimpleLock menuLock;
	ModSwitchManager()
	{
		displayedAmmoChoices.reserve(20);
		displayedModChoices.reserve(20);
		InterlockedExchange16((volatile short*)&switchState, 0);
		InterlockedExchange16((volatile short*)&ignoreAnimGraphUpdate, 0);
		InterlockedExchange16((volatile short*)&ignoreDeleteExtraData, 0);
		InterlockedExchange16((volatile short*)&equipEvent, 0);
		InterlockedExchangePointer((void* volatile*)&openedMenu, nullptr);
		InterlockedExchange16((volatile short*)&numberOfOpenedMenus, 0);
		InterlockedExchangePointer((void* volatile*)&equippedInstanceData, nullptr);
	};
	enum
	{
		bState_ReloadNotFinished = 0x0040,
		bState_AnimNotFinished = 0x0400
	};
	UInt16 GetState() { return switchState; };
	void SetState(UInt16 state) { InterlockedExchange16((volatile short*)&switchState, state); };
	void SetIgnoreAnimGraph(bool bIgnore) { InterlockedExchange16((volatile short*)&ignoreAnimGraphUpdate, bIgnore); };
	bool GetIgnoreAnimGraph() { return ignoreAnimGraphUpdate; };
	void SetIgnoreDeleteExtraData(bool bIgnore) { InterlockedExchange16((volatile short*)&ignoreDeleteExtraData, bIgnore); };
	bool GetIgnoreDeleteExtraData() { return ignoreDeleteExtraData; };
	void SetEquipEvent(bool bEquip) { InterlockedExchange16((volatile short*)&equipEvent, bEquip); };
	bool GetEquipEvent() { return equipEvent; };
	TESObjectWEAP::InstanceData* GetCurrentWeapon() { return equippedInstanceData; };
	void SetCurrentWeapon(TESObjectWEAP::InstanceData* weaponInstance) { InterlockedExchangePointer((void* volatile*)&equippedInstanceData, weaponInstance); };
	void IncOpenedMenus() { InterlockedIncrement16((volatile short*)&numberOfOpenedMenus); };
	void DecOpenedMenus() { InterlockedDecrement16((volatile short*)&numberOfOpenedMenus); };
	int GetOpenedMenus() { return numberOfOpenedMenus; };
	ModSelectionMenu* GetOpenedMenu() { return openedMenu; };
	void SetOpenedMenu(ModSelectionMenu* menu) { InterlockedExchangePointer((void* volatile*)&openedMenu, menu); };
	bool CloseOpenedMenu() 
	{ 
		if (!openedMenu)
			return true;
		static BSFixedString menuName("MSFMenu");
		IMenu* MSFmenu = (*g_ui)->GetMenu(menuName);
		if (!MSFmenu)
			return false;
		if (!MSFmenu->movie)
			return false;
		GFxMovieRoot* menuRoot = MSFmenu->movie->movieRoot;
		if (!menuRoot)
			return false;
		std::string closePath = "root." + openedMenu->scaleformName + "_loader.content.Close";
		GFxValue result;
		menuRoot->Invoke(closePath.c_str(), &result, nullptr, 0);
		InterlockedExchangePointer((void* volatile*)&openedMenu, nullptr); 
		return result.data.boolean;
	};
	bool QueueSwitch(SwitchData* data)
	{
		if (!data)
			return false;
		queueLock.Lock();
		switchDataQueue.push_back(data);
		queueLock.Release();
		return true;
	};
	bool FinishSwitch(SwitchData* data)
	{
		if (!data)
			return false;
		queueLock.Lock();
		auto it = std::find(switchDataQueue.begin(), switchDataQueue.end(), data);
		if (it != switchDataQueue.end())
			switchDataQueue.erase(it);
		queueLock.Release();
		_DEBUG("data: %p", data);
		delete data;
		return true;
	};
	SwitchData* GetNextSwitch()
	{
		SwitchData* result = nullptr;
		queueLock.Lock();
		if (switchDataQueue.begin() != switchDataQueue.end())
			result = switchDataQueue[0];
		queueLock.Release();
		return result;
	};
	bool ClearQueue()
	{
		queueLock.Lock();
		for (auto it = switchDataQueue.begin(); it != switchDataQueue.end(); it++)
		{
			SwitchData* data = *it;
			*it = nullptr;
			delete data;
		}
		switchDataQueue.clear();
		InterlockedExchange16((volatile short*)&switchState, 0);
		//_DEBUG("unlock");
		queueLock.Release();
		return true;
	};
	UInt32 GetQueueCount() { return switchDataQueue.size(); };

	void AddDisplayedAmmoNoLock(AmmoData::AmmoMod* ammo) { displayedAmmoChoices.push_back(ammo); };
	void AddDisplayedModNoLock(ModData::Mod* mod) { displayedModChoices.push_back(mod); };
	AmmoData::AmmoMod* GetDisplayedAmmoByIndex(UInt32 idx) 
	{ 
		AmmoData::AmmoMod* ammo = nullptr;
		menuLock.Lock();
		if (idx < displayedAmmoChoices.size())
			ammo = displayedAmmoChoices[idx];
		menuLock.Release();
		return ammo;
	};
	ModData::Mod* GetDisplayedModByIndex(UInt32 idx)
	{
		ModData::Mod* mod = nullptr;
		menuLock.Lock();
		if (idx < displayedModChoices.size())
			mod = displayedModChoices[idx];
		menuLock.Release();
		return mod;
	};
	void ClearDisplayChioces()
	{
		menuLock.Lock();
		displayedAmmoChoices.clear();
		displayedModChoices.clear();
		displayedAmmoChoices.reserve(20);
		displayedModChoices.reserve(20);
		menuLock.Release();
	};
	void ClearAmmoDisplayChioces() { menuLock.Lock(); displayedAmmoChoices.clear(); displayedAmmoChoices.reserve(20); menuLock.Release(); }
	void ClearModDisplayChioces() { menuLock.Lock(); displayedModChoices.clear(); displayedModChoices.reserve(20); menuLock.Release(); }

	void Reset()
	{
		ClearQueue(); 
		InterlockedExchange16((volatile short*)&ignoreAnimGraphUpdate, 0);
		InterlockedExchange16((volatile short*)&ignoreDeleteExtraData, 0);
		InterlockedExchange16((volatile short*)&equipEvent, 0);
		InterlockedExchangePointer((void* volatile*)&openedMenu, nullptr);
		InterlockedExchange16((volatile short*)&numberOfOpenedMenus, 0);
		ClearDisplayChioces();
		InterlockedExchangePointer((void* volatile*)&equippedInstanceData, nullptr);
	};
};

class MSF_MainData
{
public:
	static bool IsInitialized;
	static bool GameIsLoading;

	static RandomNumber rng;
	static int iCheckDelayMS;

	static GFxMovieRoot* MSFMenuRoot;
	static ModSelectionMenu* widgetMenu;

	static ModSwitchManager modSwitchManager;
	static WeaponStateStore weaponStateStore;
	static PlayerInventoryListEventSink playerInventoryEventSink;
	static ActorEquipManagerEventSink actorEquipManagerEventSink;
	static UInt64 cancelSwitchHotkey;
	static UInt64 lowerWeaponHotkey;
	static UInt64 DEBUGprintStoredDataHotkey;
	static Utilities::Timer tmr;

	static BurstModeManager* activeBurstManager;

	//Data added by plugins
	static std::unordered_map<UInt64, KeybindData*> keybindMap;
	static std::unordered_map<std::string, KeybindData*> keybindIDMap;
	static std::unordered_map<TESAmmo*, AmmoData*> ammoDataMap;
	static std::unordered_map<TESObjectWEAP*, AnimationData*> reloadAnimDataMap;
	static std::unordered_map<TESObjectWEAP*, AnimationData*> fireAnimDataMap;
	static std::vector<HUDFiringModeData> fmDisplayData;
	static std::vector<HUDScopeData> scopeDisplayData;
	static std::vector<HUDMuzzleData> muzzleDisplayData;
	static std::unordered_map<BGSMod::Attachment::Mod*, ModCompatibilityEdits*> compatibilityEdits;
	static std::unordered_multimap<BGSMod::Attachment::Mod*, KeywordValue> instantiationRequirements;
	static std::unordered_map<BGSMod::Attachment::Mod*, BurstModeData*> burstModeData;
	static std::unordered_map<BGSMod::Attachment::Mod*, ModData::Mod*> modDataMap;
	static std::unordered_map<BGSMod::Attachment::Mod*, AmmoData::AmmoMod*> ammoModMap;

	//Mandatory Data, filled during mod initialization
	static KeywordValue ammoAP;
	static KeywordValue magAP;
	static KeywordValue receiverAP;
	static KeywordValue muzzleAP;
	static BGSKeyword* baseModCompatibilityKW;
	static BGSKeyword* hasSwitchedAmmoKW;
	static BGSKeyword* hasUniqueStateKW;
	static BGSKeyword* tacticalReloadKW;
	static BGSMod::Attachment::Mod* APbaseMod;
	static BGSMod::Attachment::Mod* NullMuzzleMod;
	static BGSKeyword* CanHaveNullMuzzleKW;
	static BGSKeyword* FiringModBurstKW;
	static BGSKeyword* FiringModeUnderbarrelKW;
	static TESIdleForm* reloadIdle1stP;
	static TESIdleForm* reloadIdle3rdP;
	static TESIdleForm* fireIdle1stP; //single
	static TESIdleForm* fireIdle3rdP; //single
	static BGSAction* ActionFireSingle;
	static BGSAction* ActionFireAuto;
	static BGSAction* ActionReload;
	static BGSAction* ActionRightRelease;
	static BGSAction* ActionDraw;
	static BGSAction* ActionGunDown;

	//MCM data (read on init and on update)
	enum
	{
		bReloadEnabled = 0x00000100,
		bDrawEnabled = 0x00000200,
		bCustomAnimEnabled = 0x00000400,
		bAmmoRequireWeaponToBeDrawn = 0x00000800,
		bRequireAmmoToSwitch = 0x00001000,
		bSpawnRandomAmmo = 0x00002000,
		bSpawnRandomMods = 0x00004000,
		bWidgetAlwaysVisible = 0x00000001,
		bShowAmmoIcon = 0x00000002,
		bShowMuzzleIcon = 0x00000004,
		bShowAmmoName = 0x00000008,
		bShowMuzzleName = 0x00000010,
		bShowFiringMode = 0x00000020,
		bShowScopeData = 0x00000040,
		bShowUnavailableMods = 0x00000080,
		bEnableMetadataSaving = 0x00010000,
		bEnableAmmoSaving = 0x00020000,
		bEnableTacticalReloadAll = 0x00040000,
		bEnableTacticalReloadAnim = 0x00080000,
		bEnableBCRSupport = 0x00100000,
		mMakeExtraRankMask = bEnableAmmoSaving | bEnableTacticalReloadAll | bEnableTacticalReloadAnim | bEnableBCRSupport
	};
	static UInt32 MCMSettingFlags;
	static UInt16 iMinRandomAmmo;
	static UInt16 iMaxRandomAmmo;
	static UInt16 iAutolowerTimeSec;
	static std::unordered_map<std::string, float> MCMfloatSettingMap;
};

namespace MSF_Data
{
	bool InitData();
	bool FillQuickAccessData();
	bool InitCompatibility();
	bool InitMCMSettings();
	void ClearInternalKeybinds();
	void ClearNativeMSFKeybindsInMCM();
	void ClearAllMSFKeybindsInMCM();
	bool ReadMCMKeybindData();
	bool ReadKeybindData();
	bool ReadUserSettings();
	bool SetUserModifiedValue(std::string section, std::string settingName, std::string settingValue);
	void AddFloatSetting(std::string name, float value);
	bool LoadPluginData();
	bool ReadDataFromJSON(std::string fileName, std::string location);
	void PrintStoredData();
	SwitchData* GetNthAmmoMod(UInt32 num);
	bool GetNthMod(UInt32 num, BGSInventoryItem::Stack* eqStack, ModData* modData);
	bool GetNextMod(BGSInventoryItem::Stack* eqStack, ModData* modData);
	bool CheckSwitchRequirements(BGSInventoryItem::Stack* stack, ModData::Mod* modToAttach, ModData::Mod* modToRemove);
	bool QueueModsToSwitch(ModData::Mod* modToAttach, ModData::Mod* modToRemove);
	TESAmmo* GetBaseCaliber(BGSObjectInstanceExtra* objectModData, TESObjectWEAP* weapBase);
	UInt16 GetChamberSize(TESObjectWEAP* baseWeapon, BGSMod::Attachment::Mod* receiverMod);
	bool PickRandomMods(std::vector<BGSMod::Attachment::Mod*>* mods, TESAmmo** ammo, UInt32* count);
	TESIdleForm* GetReloadAnimation(Actor* actor);
	TESIdleForm* GetFireAnimation(Actor* actor);
	std::string GetFMString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetScopeString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetMuzzleString(TESObjectWEAP::InstanceData* instanceData);
	KeybindData* GetKeybindDataForKey(UInt16 keyCode, UInt8 modifiers);
}