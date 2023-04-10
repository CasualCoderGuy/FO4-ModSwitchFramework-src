#pragma once
#include "MSF_Shared.h"


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
	// TESIdleForm* GetAnimation();
	TESIdleForm* animIdle_1stP;
	TESIdleForm* animIdle_3rdP;
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
		AttachRequirements* attachRequirements;
		AnimationData* animData;
	};
	typedef std::vector<Mod*> ModVector;
	struct ModCycle
	{
		enum
		{
			bCannotHaveNullMod = 0x0001
		};
		ModVector mods;
		UInt32 flags;
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
	}
	enum
	{
		bOnePullBurst = 0x01, //If FALSE, upon releasing the trigger the firing is stopped; if TRUE, all the shots will be fired in a burst
		bResetShotCountOnRelease = 0x02, //If TRUE, upon releasing the trigger the shot count will reset; if FALSE, the shot count will not reset(only applies if bOnePullBurst is FALSE)
		bResetShotCountOnReload = 0x04,
		bActive = 0x08
	};
	UInt32 delayTime; //Interval between two shots in a single burst(in milliseconds)
	UInt8 flags;
	UInt8 numOfTotalShots; //Number of shots fired during a single burst
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
	UInt32 version;
	enum
	{
		kType_Widget = 0,
		kType_AmmoMenu = 1,
		kType_ModMenu = 2,
		kType_All = 3
	};
	ModSelectionMenu(std::string name, UInt8 menuType){
		scaleformName = name; type = menuType; version = 0;
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
	BGSMod::Attachment::Mod* ModToAttach;
	BGSMod::Attachment::Mod* ModToRemove;
	TESObjectMISC* LooseModToRemove;
	TESObjectMISC* LooseModToAdd;
	AnimationData* animData;
};

class ModSwitchManager
{
private:
	volatile UInt16 switchState;
	SimpleLock queueLock;
	std::vector<SwitchData*> switchDataQueue;

	ModSelectionMenu* volatile openedMenu;
	volatile UInt16 numberOfOpenedMenus;

	TESObjectWEAP::InstanceData* volatile equippedInstanceData;
public:
	ModSwitchManager()
	{
		InterlockedExchange16((volatile short*)&switchState, 0);
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
	TESObjectWEAP::InstanceData* GetCurrentWeapon() { return equippedInstanceData; };
	void SetCurrentWeapon(TESObjectWEAP::InstanceData* weaponInstance) { InterlockedExchangePointer((void* volatile*)&equippedInstanceData, weaponInstance); };
	void IncOpenedMenus() { InterlockedIncrement16((volatile short*)&numberOfOpenedMenus); };
	void DecOpenedMenus() { InterlockedDecrement16((volatile short*)&numberOfOpenedMenus); };
	int GetOpenedMenus() { return numberOfOpenedMenus; };
	ModSelectionMenu* GetOpenedMenu() { return openedMenu; };
	void SetOpenedMenu(ModSelectionMenu* menu) { InterlockedExchangePointer((void* volatile*)&openedMenu, menu); };
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
		_MESSAGE("data: %p", data);
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
		_MESSAGE("unlock");
		queueLock.Release();
		return true;
	};
	UInt32 GetQueueCount() { return switchDataQueue.size(); };

	void Reset()
	{
		ClearQueue();
		InterlockedExchangePointer((void* volatile*)&openedMenu, nullptr);
		InterlockedExchange16((volatile short*)&numberOfOpenedMenus, 0);
		InterlockedExchangePointer((void* volatile*)&equippedInstanceData, nullptr);
	};
};

class MSF_MainData
{
public:
	static bool IsInitialized;

	static RandomNumber rng;
	static int iCheckDelayMS;

	static GFxMovieRoot* MSFMenuRoot;
	static ModSelectionMenu* widgetMenu;

	static ModSwitchManager modSwitchManager;
	static UInt64 cancelSwitchHotkey;
	static UInt64 lowerWeaponHotkey;
	static Utilities::Timer tmr;

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

	//Mandatory Data, filled during mod initialization
	static KeywordValue ammoAP;
	static BGSKeyword* baseModCompatibilityKW;
	static BGSKeyword* hasSwitchedAmmoKW;
	static BGSKeyword* hasUniqueStateKW;
	static BGSMod::Attachment::Mod* APbaseMod;
	static BGSMod::Attachment::Mod* NullMuzzleMod;
	static BGSKeyword* CanHaveNullMuzzleKW;
	static BGSKeyword* FiringModeUnderbarrelKW;
	static TESIdleForm* reloadIdle1stP;
	static TESIdleForm* reloadIdle3rdP;
	static TESIdleForm* fireIdle1stP; //single
	static TESIdleForm* fireIdle3rdP; //single
	static BGSAction* ActionFireSingle;
	static BGSAction* ActionFireAuto;
	static BGSAction* ActionReload;
	static BGSAction* ActionDraw;
	static BGSAction* ActionGunDown;

	//MCM data (read on init and on update)
	enum
	{
		bReloadEnabled = 0x0100,
		bDrawEnabled = 0x0200,
		bCustomAnimEnabled = 0x0400,
		bRequireAmmoToSwitch = 0x0800,
		bWidgetAlwaysVisible = 0x0001,
		bShowAmmoIcon = 0x0002,
		bShowMuzzleIcon = 0x0004,
		bShowAmmoName = 0x0008,
		bShowMuzzleName = 0x0010,
		bShowFiringMode = 0x0020,
		bShowScopeData = 0x0040,
		bShowUnavailableMods = 0x0080
	};
	static UInt16 MCMSettingFlags;
	static std::unordered_map<std::string, float> MCMfloatSettingMap;
};

namespace MSF_Data
{
	bool InitData();
	bool InitCompatibility();
	bool InitMCMSettings();
	bool ReadMCMKeybindData();
	bool ReadKeybindData();
	bool ReadUserSettings();
	bool SetUserModifiedValue(std::string section, std::string settingName, std::string settingValue);
	void AddFloatSetting(std::string name, float value);
	bool LoadPluginData();
	bool ReadDataFromJSON(std::string fileName, std::string location);
	SwitchData* GetNthAmmoMod(UInt32 num);
	bool GetNthMod(UInt32 num, BGSInventoryItem::Stack* eqStack, ModData* modData);
	bool GetNextMod(BGSInventoryItem::Stack* eqStack, ModData* modData);
	bool CheckSwitchRequirements(BGSInventoryItem::Stack* stack, ModData::Mod* modToAttach, ModData::Mod* modToRemove);
	bool QueueModsToSwitch(ModData::Mod* modToAttach, ModData::Mod* modToRemove, bool bNeedInit);
	TESAmmo* GetBaseCaliber(BGSInventoryItem::Stack* stack);
	bool PickRandomMods(tArray<BGSMod::Attachment::Mod*>* mods, TESAmmo** ammo, UInt32* count);
	TESIdleForm* GetReloadAnimation(TESObjectWEAP* equippedWeap, bool get3rdP);
	TESIdleForm* GetFireAnimation(TESObjectWEAP* equippedWeap, bool get3rdP);
	std::string GetFMString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetScopeString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetMuzzleString(TESObjectWEAP::InstanceData* instanceData);
	KeybindData* GetKeybindDataForKey(UInt16 keyCode, UInt8 modifiers);
}