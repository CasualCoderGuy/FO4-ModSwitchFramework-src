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

class ModAssociationData
{
protected:
	UInt8 type;
public:
	UInt8 GetType() { return type; }
	struct ModPair
	{
		BGSMod::Attachment::Mod* parentMod;
		BGSMod::Attachment::Mod* functionMod;
	};
	enum
	{
		bRequireAPmod = 0x0001,
		bCanHaveNullMod = 0x0002,
		bCanToggleSingle = 0x0004,
		bCanToggleSingle_SkipFirst = 0x0008,
		bThreadSafe = 0x0100,
		bDrawEnabled = 0x1000,
		bRequireLooseMod = 0x2000,
		bUpdateAnimGraph = 0x4000,
		bIgnoreAnimations = 0x8000,
		mBitTransferMask = 0xF001
	};
	UInt16 flags;
	BGSKeyword* funcKeyword; //!!!!
	TESIdleForm* animIdle_1stP;
	TESIdleForm* animIdle_3rdP;
};

class SingleModPair : public ModAssociationData
{
public:
	SingleModPair() { type = 0x1; }
	ModPair modPair;
};

class ModPairArray : public ModAssociationData
{
public:
	ModPairArray() { type = 0x2; }
	std::vector<ModPair> modPairs;
};

class MultipleMod : public ModAssociationData
{
public:
	MultipleMod() { type = 0x3; }
	BGSMod::Attachment::Mod* parentMod;
	std::vector<BGSMod::Attachment::Mod*> functionMods;
};

class AnimationData
{
public:
	TESIdleForm* animIdle_1stP;
	TESIdleForm* animIdle_3rdP;
	//BGSAction* animAction;
};

class BurstMode
{
public:
	Actor* actor;
	UInt8 NumOfRoundsFired;
	Utilities::Timer tmr;
	TESIdleForm* fireIdle;
	BGSAction* fireSingleAction;
	UInt16 wait;
	UInt8 flags;
	UInt8 totalShots;
	bool animReady;
};

class HUDFiringModeData
{
public:
	BGSKeyword* modeKeyword;
	std::string displayString;
};

class HUDScopeData
{
public:
	BGSKeyword* scopeKeyword;
	std::string displayString;
};

class HUDMuzzleData
{
public:
	BGSKeyword* muzzleKeyword;
	std::string displayString;
};

class ModSelectionMenu
{
public:
	std::string scaleformID;
	std::string swfFilename;
	UInt32 type;
	UInt32 version;
	enum
	{
		kType_Unspecified = 0,
		kType_AmmoMenu = 1,
		kType_ModMenu = 2,
		kType_Widget = 3
	};
	ModSelectionMenu(std::string id, std::string swf){
		scaleformID = id; swfFilename = swf; type = 0; version = 0;
	};
};

class KeybindData
{
public:
	enum
	{
		//lower 4 bits: Nth
		bCancel = 0x10,
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
	//std::string swfPath;
	//std::string menuScriptPath;
	std::vector<ModAssociationData*> modAssociations;
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
	//SwitchData() { InitializeCriticalSection(switchCriticalSection); }
	enum
	{
		bNeedInit = 0x0001,
		bSwitchingInProgress = 0x0002,
		bQueuedSwitch = 0x0004,
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
	LPCRITICAL_SECTION switchCriticalSection;
	UInt16 SwitchFlags;
	BGSMod::Attachment::Mod* ModToAttach;
	BGSMod::Attachment::Mod* ModToRemove;
	TESObjectMISC* LooseModToRemove;
	TESObjectMISC* LooseModToAdd;
	TESIdleForm* AnimToPlay1stP;
	TESIdleForm* AnimToPlay3rdP;
	TESObjectWEAP::InstanceData* equippedInstanceData;
	ModSelectionMenu* openedMenu;
	void ClearData()
	{
		//EnterCriticalSection(switchCriticalSection);
		SwitchFlags = 0;
		ModToAttach = nullptr;
		ModToRemove = nullptr;
		LooseModToRemove = nullptr;
		LooseModToAdd = nullptr;
		AnimToPlay1stP = nullptr;
		AnimToPlay3rdP = nullptr;
		//LeaveCriticalSection(switchCriticalSection);
	}
};

class ModSwitchManager
{
	UInt16 SwitchState;
	SwitchData* threadUnsafeData;
	std::string openedMenu;
	TESObjectWEAP::InstanceData* equippedInstanceData;
};

class MSF_MainData
{
public:
	static bool IsInitialized;
	static RandomNumber rng;
	static int iCheckDelayMS;
	static GFxMovieRoot* MSFMenuRoot;
	static ModSelectionMenu* widgetMenu;
	static int numberOfOpenedMenus;

	static SwitchData switchData;
	static std::vector<BurstMode> burstMode;
	static Utilities::Timer tmr;

	//Data added by plugins
	static std::vector<AmmoData> ammoData;
	static std::vector<SingleModPair> singleModPairs;
	static std::vector<ModPairArray> modPairArrays;
	static std::vector<MultipleMod> multiModAssocs;

	static std::vector<HUDFiringModeData> fmDisplayData;
	static std::vector<HUDScopeData> scopeDisplayData;
	static std::vector<HUDMuzzleData> muzzleDisplayData;

	/////
	static std::unordered_map<TESAmmo*, AmmoData*> ammoDataMap;
	static std::unordered_map<BGSMod::Attachment::Mod*, TESAmmo*> ammoModMap;
	static std::unordered_map<TESObjectWEAP*, AnimationData*> reloadAnimDataMap;
	static std::unordered_map<TESObjectWEAP*, AnimationData*> fireAnimDataMap;

	//Mandatory Data, filled during mod initialization
	static BGSKeyword* hasSwitchedAmmoKW;
	static BGSKeyword* hasSwitchedSecAmmoKW;
	static BGSMod::Attachment::Mod* APbaseMod;
	static BGSMod::Attachment::Mod* NullMuzzleMod;
	static BGSKeyword* CanHaveNullMuzzleKW;
	static BGSKeyword* FiringModeUnderbarrelKW;
	static BGSMod::Attachment::Mod* PlaceholderMod;
	static BGSMod::Attachment::Mod* PlaceholderModAmmo;
	static ActorValueInfo*  BurstModeTime;
	static ActorValueInfo*  BurstModeFlags;
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
		bShowScopeData = 0x0040
	};
	static UInt16 MCMSettingFlags;
	static std::unordered_map<std::string, float> MCMfloatSettingMap;
	static std::unordered_map<UInt64, KeybindData*> keybindMap;
	static std::unordered_map<std::string, KeybindData*> keybindIDMap;
};

namespace MSF_Data
{
	bool InitData();
	bool InitMCMSettings();
	bool ReadMCMKeybindData();
	bool ReadKeybindData();
	bool ReadUserSettings();
	bool SetUserModifiedValue(std::string section, std::string settingName, std::string settingValue);
	void AddFloatSetting(std::string name, float value);
	bool LoadPluginData();
	bool ReadDataFromJSON(std::string fileName, std::string location);
	bool GetNthAmmoMod(UInt32 num);
	bool GetNthMod(UInt32 num, std::vector<ModAssociationData*>* modAssociations);
	bool GetNextMod(BGSInventoryItem::Stack* eqStack, std::vector<ModAssociationData*>* modAssociations);
	TESAmmo* GetBaseCaliber(BGSInventoryItem::Stack* stack);
	bool PickRandomMods(tArray<BGSMod::Attachment::Mod*>* mods, TESAmmo** ammo, UInt32* count);
	TESIdleForm* GetReloadAnimation(TESObjectWEAP* equippedWeap, bool get3rdP);
	TESIdleForm* GetFireAnimation(TESObjectWEAP* equippedWeap, bool get3rdP);
	std::string GetFMString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetScopeString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetMuzzleString(TESObjectWEAP::InstanceData* instanceData);
	KeybindData* GetKeyFunctionID(UInt16 keyCode, UInt8 modifiers);
}