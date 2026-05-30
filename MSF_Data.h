#pragma once
#include "MSF_Shared.h"

class PlayerInventoryListEventSink;
class ActorEquipManagerEventSink;
class WeaponStateStore;
class HUDMenuAmmoDisplay;
class BCRinterface;
class BurstModeManager;
class BurstModeData;
class MSFCustomMenu;
class MSFCustomMenuData;

class AmmoData
{
public:
	class AmmoMod
	{
	public:
		enum
		{
			bDoSwitchBeforeAnimations = 0x0800,
			mBitTransferMask = 0x0800
		};
		TESAmmo* ammo;
		BGSMod::Attachment::Mod* mod;
		UInt16 ammoID;
		UInt16 flags;
		float spawnChance;
		float spawnMinAmountMultiplier;
		float spawnMaxAmountMultiplier;
	};
	AmmoMod baseAmmoData;
	std::vector<AmmoMod> ammoMods;
	LevItem* baseAmmoLL;
};

class AnimationData
{
public:
	AnimationData(TESIdleForm* animIdle_1stP_in, TESIdleForm* animIdle_3rdP_in, TESIdleForm* animIdle_1stP_PA_in, TESIdleForm* animIdle_3rdP_PA_in, bool shouldBlend = true)
	{
		animIdle_1stP = animIdle_1stP_in; animIdle_3rdP = animIdle_3rdP_in; animIdle_1stP_PA = animIdle_1stP_PA_in; animIdle_3rdP_PA = animIdle_3rdP_PA_in; shouldBlendAnim = shouldBlend;
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
		else if (state == 8)
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
	bool shouldBlendAnim;
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
			bHasUniqueState = 0x0008,
			bCanBeEquippedByMiscMod = 0x0010,
			bCannotCancelAnim = 0x0200,
			bPlayFastEquipAnim = 0x0400,
			bDoSwitchBeforeAnimations = 0x0800,
			bNotRequireWeaponToBeDrawn = 0x1000,
			bRequireLooseMod = 0x2000,
			bUpdateAnimGraph = 0x4000,
			bIgnoreAnimations = 0x8000,
			mBitTransferMask = 0xFF00UL,
			bShouldNotStopIdle = 0x10000
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
	class ModCycle
	{
	public:
		enum
		{
			bCannotHaveNullMod = 0x0001,
			bUIRequireWeaponToBeDrawn = 0x0002
		};
		ModVector mods;
		UInt32 flags;
		float spawnChanceBase;
		std::string nullModName;
	};
	/*enum
	{
		bRequireAPmod = 0x0001
		bHasUniqueState = 0x0002
	};*/
	UInt16 attachParentValue;
	UInt16 flags;
	Mod::AttachRequirements* APattachRequirements;
	BGSSoundDescriptorForm* successSound;
	std::unordered_map<KeywordValue, ModCycle*> modCycleMap; //hash: attachparent mod's instantiation keyword value, bucket: modcycle; if multiple cycles are found ambiuguity error is thrown
};

class ChamberData
{
public:
	ChamberData(UInt16 Size, UInt16 Flags, UInt32 Mod = 0)
	{
		this->chamberSize = Size;
		this->flags = Flags;
		this->mod = Mod;
	}
	UInt16 chamberSize;
	UInt16 flags;
	UInt32 mod;
};

//class UniqueState
//{
//public:
//	std::vector<KeywordValue> uniqueStateAPs;
//};

class ModCompatibilityEdits
{
public:
	KeywordValue attachParent;
	std::vector<KeywordValue> removedAPslots;
	std::vector<KeywordValue> addedAPslots;
	std::vector<KeywordValue> removedFilters;
	std::vector<KeywordValue> addedFilters;
};

class HUDDisplayData
{
public:
	BGSKeyword* keyword;
	std::string displayString;
	float priority;
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
	MSFCustomMenuData* customMenuData;
	AnimationData* menuAnimation;
	BGSSoundDescriptorForm* successSound;
	enum
	{
		kType_Widget = 0,
		kType_AmmoMenu = 1,
		kType_ModMenu = 2,
		kType_All = 3,
		kType_Global = 4
	};
	ModSelectionMenu(std::string name, UInt8 menuType, BGSSoundDescriptorForm* sound){
		scaleformName = name; type = menuType; version = 0; menuAnimation = nullptr; customMenuData = nullptr; successSound = sound;
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
		mAmmoNumMask = 0x2F,
		mModNumMask = 0x0F,
		bMenuBoth = 0x100,
		bQuickKey = 0x200
	};
	std::string functionID;
	UInt16 type;
	UInt16 keyCode;
	UInt8 modifiers;
	ModSelectionMenu* selectMenu;
	BGSSoundDescriptorForm* successSound;
	ModData* modData;
	std::vector<KeywordValue> menuAttachPoints;
};

class MCMfloatData
{
public:
	std::string name;
	float value;
};

class CustomProjectileFormManager
{
public:
	BGSProjectile* Clone(BGSProjectile* proj);
	bool ApplyMods(ExtraDataList* extraDataList);
	bool AddModData(BGSMod::Attachment::Mod* mod, BGSMod::Container::Data data, bool overwrite = true);
	void Cleanup(BGSProjectile* proj);
	bool ClearData();
	enum WeaponFormProperty
	{
		//flags
		kNull = 150,
		kWeaponTarget_Supersonic,
		kWeaponTarget_MuzzleFlash,
		kWeaponTarget_Explosion,
		kWeaponTarget_AltTrigger,
		kWeaponTarget_Hitscan,
		kWeaponTarget_CanBeDisabled,
		kWeaponTarget_CanBePickedUp,
		kWeaponTarget_PinsLimbs,
		kWeaponTarget_PassThroughObjects,
		kWeaponTarget_DisableAimCorr,
		kWeaponTarget_PenetratesGeom,
		kWeaponTarget_ContinuousUpdate,
		kWeaponTarget_SeeksTarget,
		//int
		kWeaponTarget_TracerFreq,
		kWeaponTarget_SoundLevel,
		kWeaponTarget_Type,
		//float
		kWeaponTarget_Gravity,
		kWeaponTarget_Speed,
		kWeaponTarget_Range,
		kWeaponTarget_ExpProximity,
		kWeaponTarget_ExpTimer,
		kWeaponTarget_MuzzleFlashDur,
		kWeaponTarget_FadeOutTime,
		kWeaponTarget_Force,
		kWeaponTarget_ConeSpread,
		kWeaponTarget_CollRadius,
		kWeaponTarget_Lifetime,
		kWeaponTarget_RelaunchInt,
		//Form
		kWeaponTarget_Light,
		kWeaponTarget_MuzzleFlashLight,
		kWeaponTarget_ExpType,
		kWeaponTarget_ActSoundLoop,
		kWeaponTarget_CountdownSound,
		kWeaponTarget_DeactivateSound,
		kWeaponTarget_DecalData,
		kWeaponTarget_CollisionLayer,
		kWeaponTarget_VATSprojectile,
		kWeaponTarget_Model,
		kWeaponTarget_MuzzleFlashModel,
		kWeaponTarget_Destruction,
		kMax

	};
	std::unordered_map<BGSMod::Attachment::Mod*, std::vector<BGSMod::Container::Data>> projectileModMap;
	std::unordered_map<ExtraDataList*, std::pair<BGSProjectile*, BGSProjectile*>> projectileParentMap;
private:
	bool ReturnCleanup(std::unordered_map<ExtraDataList*, std::pair<BGSProjectile*, BGSProjectile*>>::iterator foundData, TESObjectWEAP::InstanceData* instance);
};

class SwitchData
{
public:
	SwitchData()
	{
		SwitchFlags = 0;
		targetAmmo = nullptr;
		ModToAttach = nullptr;
		ModToRemove = nullptr;
		LooseModToRemove = nullptr;
		LooseModToAdd = nullptr;
		animFlavor = 0;
		animData = nullptr;
		soundToPlay = nullptr;
	};
	~SwitchData()
	{
		SwitchFlags = 0;
		targetAmmo = nullptr;
		ModToAttach = nullptr;
		ModToRemove = nullptr;
		LooseModToRemove = nullptr;
		LooseModToAdd = nullptr;
		animFlavor = 0;
		animData = nullptr;
		soundToPlay = nullptr;
	}
	enum
	{
		//bNeedInit = 0x0001,
		//bQueuedHUDSelection = 0x0004,
		//bSetChamberedAmmo = 0x0008,
		bSwitchingInProgress = 0x00010000,
		bDrawNeeded = 0x00020000,
		bDrawInProgress = 0x00040000,
		bReloadNeeded = 0x00100000,
		bReloadInProgress = 0x00200000,
		bReloadNotFinished = 0x00400000,
		bReloadFull = 0x00800000,
		bReloadZeroCount = 0x08000000,
		bAnimNeeded = 0x01000000,
		bAnimInProgress = 0x02000000,
		bAnimNotFinished = 0x04000000,
		bDoSwitchBeforeAnimations = 0x00000800,
		bDrawEnabled = 0x00001000,
		bSetLooseMods = 0x00002000,
		bUpdateAnimGraph = 0x00004000,
		bIgnoreAnimations = 0x00008000,
		bPlayFastEquipAnim = 0x00000400,
		bCannotCancelAnim = 0x00000200

		//bDoSwitchBeforeAnimations
	};
	UInt32 SwitchFlags;
	KeywordValue animFlavor;
	TESAmmo* targetAmmo;
	BGSMod::Attachment::Mod* ModToAttach;
	BGSMod::Attachment::Mod* ModToRemove;
	TESObjectMISC* LooseModToRemove;
	TESObjectMISC* LooseModToAdd;
	AnimationData* animData;
	BGSSoundDescriptorForm* soundToPlay;
};

class ModSwitchManager
{
public:
	struct OpenedMenuData
	{
		ModSelectionMenu* selectMenu;
		ModData* mods;
		bool isOpening;
		bool UINeedsToBeDrawn;
	};

	volatile UInt16 dontPutYourGunIn;
private:
	volatile UInt16 ignoreAnimGraphUpdate;
	volatile UInt16 ignorePlayEquipAction;
	volatile UInt16 ignoreDeleteExtraData;
	volatile UInt16 shouldBlendAnimation;
	volatile UInt16 modChangeEvent;
	volatile UInt16 isInPA;
	volatile UInt16 isBCRreload;
	volatile UInt16 switchState;
	volatile UInt16 forcedReload;
	volatile UInt16 changeAmmo;
	volatile UInt16 animCantBeCancelled;
	SimpleLock queueLock;
	std::vector<SwitchData*> switchDataQueue;

	struct OpenedMenuDataHolder : OpenedMenuData
	{
		SimpleLock menuLock;
	} menuData;

	//ModSelectionMenu* volatile openedMenu;
	volatile UInt16 numberOfOpenedMenus;
	std::vector<AmmoData::AmmoMod*> displayedAmmoChoices;
	std::vector<ModData::Mod*> displayedModChoices;

	union
	{
		struct
		{
			AmmoData* ammoData;
			AmmoData::AmmoMod* ammoMod;
		} ammo;
		struct
		{
			ModData::ModCycle* cycle;
			ModData::Mod* modToAttach;
			ModData::Mod* modToRemove;
		} mod;
	} quickSelection;
	UInt32 quickSelectIdx;
	bool quickSelectIsAmmo;
	bool keyIsDown;
	BGSSoundDescriptorForm* quickKeySound;
	SimpleLock quickSelectLock;
	DelayedExecutor quickKeyTimer;
	KeybindData* lastQuickKey;

	TESObjectWEAP::InstanceData* volatile equippedInstanceData;
public:
	SimpleLock menuLock;
	DelayedExecutor lowerGunTimer;
	ModSwitchManager()
	{
		displayedAmmoChoices.reserve(20);
		displayedModChoices.reserve(20);
		InterlockedExchange16((volatile short*)&switchState, 0);
		InterlockedExchange16((volatile short*)&ignoreAnimGraphUpdate, 0);
		InterlockedExchange16((volatile short*)&ignorePlayEquipAction, 0);
		InterlockedExchange16((volatile short*)&ignoreDeleteExtraData, 0);
		InterlockedExchange16((volatile short*)&modChangeEvent, 0);
		//InterlockedExchangePointer((void* volatile*)&openedMenu, nullptr);
		InterlockedExchange16((volatile short*)&numberOfOpenedMenus, 0);
		InterlockedExchangePointer((void* volatile*)&equippedInstanceData, nullptr);
		InterlockedExchange16((volatile short*)&isBCRreload, 0);
		InterlockedExchange16((volatile short*)&isInPA, 0);
		InterlockedExchange16((volatile short*)&shouldBlendAnimation, 0);
		InterlockedExchange16((volatile short*)&forcedReload, 0);
		InterlockedExchange16((volatile short*)&dontPutYourGunIn, 0);
		InterlockedExchange16((volatile short*)&changeAmmo, 0);
		InterlockedExchange16((volatile short*)&animCantBeCancelled, 0);
		ClearQuickSelection();
		ClearSelectMenu();
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
	void SetIgnoreEquipAction(bool bIgnore) { InterlockedExchange16((volatile short*)&ignorePlayEquipAction, bIgnore); };
	bool GetIgnoreEquipAction() { return ignorePlayEquipAction; };
	void SetIgnoreDeleteExtraData(bool bIgnore) { InterlockedExchange16((volatile short*)&ignoreDeleteExtraData, bIgnore); };
	bool GetIgnoreDeleteExtraData() { return ignoreDeleteExtraData; };
	void SetShouldBlendAnimation(bool bBlend) { InterlockedExchange16((volatile short*)&shouldBlendAnimation, bBlend); };
	bool GetShouldBlendAnimation() { return shouldBlendAnimation; };
	void SetModChangeEvent(bool bEquip) { InterlockedExchange16((volatile short*)&modChangeEvent, bEquip); };
	bool GetModChangeEvent() { return modChangeEvent; };
	void SetIsBCRreload(UInt16 bBCRreload) { InterlockedExchange16((volatile short*)&isBCRreload, bBCRreload); };
	UInt16 GetIsBCRreload() { return isBCRreload; };
	bool GetSetForcedReload() { return InterlockedCompareExchange16((volatile short*)&forcedReload, 0, 1); };
	bool SetForcedReload(bool bForce) { return InterlockedExchange16((volatile short*)&forcedReload, bForce); };
	void SetDontPutYourGunIn(bool bEquip) { InterlockedExchange16((volatile short*)&dontPutYourGunIn, bEquip); };
	bool GetDontPutYourGunIn() { return dontPutYourGunIn; };
	UInt8 GetSetChangeAmmo(UInt16 doChangeAmmo) { return InterlockedExchange16((volatile short*)&changeAmmo, doChangeAmmo); };
	void SetAnimCanBeCancelled(bool bCanCancel) { InterlockedExchange16((volatile short*)&animCantBeCancelled, !bCanCancel); };
	bool IsAnimPastCancelPoint() { return animCantBeCancelled == 1; };
	TESObjectWEAP::InstanceData* GetCurrentWeapon() { return equippedInstanceData; };
	void SetCurrentWeapon(TESObjectWEAP::InstanceData* weaponInstance) { InterlockedExchangePointer((void* volatile*)&equippedInstanceData, weaponInstance); };
	void IncOpenedMenus() { InterlockedIncrement16((volatile short*)&numberOfOpenedMenus); };
	void DecOpenedMenus() { InterlockedDecrement16((volatile short*)&numberOfOpenedMenus); };
	int GetOpenedMenus() { return numberOfOpenedMenus; };
	OpenedMenuData GetOpenedMenuData()
	{ 
		menuData.menuLock.Lock(); 
		OpenedMenuData openedMenuData = OpenedMenuData{ menuData.selectMenu, menuData.mods, menuData.isOpening, menuData.UINeedsToBeDrawn };
		menuData.menuLock.Release();
		return openedMenuData;
	};
	void SetMenuIsOpened(bool opening = false)
	{
		menuData.menuLock.Lock();
		menuData.isOpening = opening;
		menuData.menuLock.Release();
	};
	void SetMenuNeedsDrawnWeapon(bool needs = true)
	{
		menuData.menuLock.Lock();
		menuData.UINeedsToBeDrawn = needs;
		menuData.menuLock.Release();
	};
	bool SetOpenedMenu(ModSelectionMenu* menu, ModData* mods); //!/rewrite
	bool CloseOpenedMenu(); //!/rewrite
	bool QueueSwitch(SwitchData* data)
	{
		if (!data)
			return false;
		queueLock.Lock();
		switchDataQueue.push_back(data);
		queueLock.Release();
		_DEBUG("QUEUED");
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
		_DEBUG("FINISH data: %p", data);
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
		_DEBUG("GETNEXT");
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
		_DEBUG("CLEARED");
		InterlockedExchange16((volatile short*)&switchState, 0);
		//_DEBUG("unlock");
		queueLock.Release();
		return true;
	};
	UInt32 GetQueueCount() { return switchDataQueue.size(); };

	void AddDisplayedAmmoNoLock(AmmoData::AmmoMod* ammo) { displayedAmmoChoices.push_back(ammo); };
	void AddDisplayedModNoLock(ModData::Mod* mod) { displayedModChoices.push_back(mod); };
	std::vector<UInt32> GetIdxsOfAmmo(TESAmmo* ammo) 
	{ 
		std::vector<UInt32> idxs;
		if (!ammo)
			return idxs;
		menuLock.Lock();
		for (UInt32 ammoIdx = 0; ammoIdx < displayedAmmoChoices.size(); ammoIdx++)
		{
			auto ammoCh = displayedAmmoChoices[ammoIdx];
			if (ammoCh && ammoCh->ammo == ammo)
				idxs.push_back(ammoIdx);
		}
		menuLock.Release();
		return idxs;
	};
	std::vector<UInt32> GetIdxsOfMISCMod(TESObjectMISC* misc)
	{
		std::vector<UInt32> idxs;
		if (!misc)
			return idxs;
		menuLock.Lock();
		for (UInt32 modIdx = 0; modIdx < displayedModChoices.size(); modIdx++)
		{
			auto modCh = displayedAmmoChoices[modIdx];
			if (modCh && (modCh->flags & ModData::Mod::bRequireLooseMod) && Utilities::GetLooseMod(modCh->mod) == misc)
				idxs.push_back(modIdx);
		}
		menuLock.Release();
		return idxs;
	};
	UInt32 GetIdxOfAmmoMod(BGSMod::Attachment::Mod* ammoMod)
	{
		menuLock.Lock();
		for (UInt32 ammoIdx = 0; ammoIdx < displayedAmmoChoices.size(); ammoIdx++)
		{
			auto ammoCh = displayedAmmoChoices[ammoIdx];
			if (ammoCh && ammoCh->mod == ammoMod)
			{
				menuLock.Release();
				return ammoIdx;
			}
		}
		menuLock.Release();
		return -1;
	};
	ModData::Mod* GetDisplayedModByIndexNoLock(ModData::Mod* mod) { displayedModChoices.push_back(mod); };
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
	void ClearSelectMenu()
	{
		menuData.menuLock.Lock();
		menuData.selectMenu = nullptr;
		menuData.mods = nullptr;
		menuData.isOpening = false;
		menuData.UINeedsToBeDrawn = false;
		menuData.menuLock.Release();
	};
	void HandlePAEvent();
	
	bool SetModSelection(ModData::ModCycle* cycle, ModData::Mod* modToAttach, ModData::Mod* modToRemove, UInt32 selectIdx);
	bool SetAmmoSelection(AmmoData* data, AmmoData::AmmoMod* ammoMod, UInt32 selectIdx);
	bool HandleQuickkey(KeybindData* input);
	bool HandleQuickkeyTimeout();
	void ClearQuickSelection(bool doLock = true, bool updateWidget = false, bool playFailSound = false, bool onlyRunning = false);
	void SetQuickkeyUp(KeybindData* input); //should not matter which;

	void Reset()
	{
		ClearQueue(); 
		InterlockedExchange16((volatile short*)&ignoreAnimGraphUpdate, 0);
		InterlockedExchange16((volatile short*)&ignorePlayEquipAction, 0);
		InterlockedExchange16((volatile short*)&ignoreDeleteExtraData, 0);
		InterlockedExchange16((volatile short*)&modChangeEvent, 0);
		//InterlockedExchangePointer((void* volatile*)&openedMenu, nullptr);
		InterlockedExchange16((volatile short*)&numberOfOpenedMenus, 0);
		ClearDisplayChioces();
		InterlockedExchangePointer((void* volatile*)&equippedInstanceData, nullptr);
		UInt16 inPA = IsInPowerArmor(*g_player);
		InterlockedExchange16((volatile short*)&isInPA, inPA);
		InterlockedExchange16((volatile short*)&isBCRreload, 0);
		InterlockedExchange16((volatile short*)&shouldBlendAnimation, 0);
		InterlockedExchange16((volatile short*)&forcedReload, 0);
		InterlockedExchange16((volatile short*)&dontPutYourGunIn, 0);
		InterlockedExchange16((volatile short*)&changeAmmo, 0);
		InterlockedExchange16((volatile short*)&animCantBeCancelled, 0);
		ClearQuickSelection();
		ClearSelectMenu();
		quickKeyTimer.cancel();
		lowerGunTimer.cancel();
	};
};

class MSFWidgetSettings
{ 
public:
	UInt32 GetRGBcolor() { return (iColorR << 0x10) + (iColorG << 0x08) + (iColorB << 0x00); };
	float fSliderMainX = 1250.0;
	float fSliderMainY = 540.0;
	float fSliderScale = 1.0;
	float fSliderAlpha = 1.0;
	float fPowerArmorOffsetX = 0.0;
	float fPowerArmorOffsetY = -100.0;
	UInt8 iColorR = 255;
	UInt8 iColorG = 255;
	UInt8 iColorB = 255;
	UInt32 iFont = 1;

	float fSliderAmmoIconX = 0.0;
	float fSliderAmmoIconY = 0.0;
	float fSliderMuzzleIconX = 0.0;
	float fSliderMuzzleIconY = 0.0;
	float fSliderAmmoNameX = 0.0;
	float fSliderAmmoNameY = 0.0;
	float fSliderMuzzleNameX = 0.0;
	float fSliderMuzzleNameY = 0.0;
	float fSliderFiringModeX = 0.0;
	float fSliderFiringModeY = 0.0;
	float fSliderScopeDataX = 0.0;
	float fSliderScopeDataY = 0.0;
};

class MSF_MainData
{
public:
	static bool IsInitialized;
	static bool GameIsLoading;
	static bool PutYourGunInCompatibility;
	static bool BAKACompatibility;
	static bool HadEquippedAmmo;

	static RandomNumber rng;
	static int iCheckDelayMS;
	static int quickKeyTimeoutMS;
	static int iCancelDelayMS;
	static float fBaseChanceMultiplier;

	static GFxMovieRoot* MSFMenuRoot;
	static ModSelectionMenu* widgetMenu;
	static HUDMenuAmmoDisplay* ammoDisplay;

	static BCRinterface BCRinterfaceHolder;
	static ModSwitchManager modSwitchManager;
	static WeaponStateStore weaponStateStore;
	static CustomProjectileFormManager projectileManager;
	static PlayerInventoryListEventSink playerInventoryEventSink;
	static ActorEquipManagerEventSink actorEquipManagerEventSink;
	static UInt64 cancelSwitchHotkey;
	static UInt64 patchBaseAmmoHotkey;
	static UInt64 lowerWeaponHotkey;
	static UInt64 DEBUGprintStoredDataHotkey;
	static Utilities::Timer lowerTmr;
	static long long lowerDelay;
	static BGSSoundDescriptorForm* failSound;
	static BGSSoundDescriptorForm* failSoundQuickkey;
	static BGSSoundDescriptorForm* failSoundMenu;
	static BGSSoundDescriptorForm* nextSoundQuickkey;
	static std::string loc;

	static BurstModeManager* activeBurstManager;

	//Data added by plugins
	static std::unordered_map<UInt64, KeybindData*> keybindMap;
	static std::unordered_map<std::string, KeybindData*> keybindIDMap;
	static std::unordered_map<KeywordValue, KeybindData*> keybindAPMap;
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
	//static std::unordered_map<TESAmmo*, AmmoData::AmmoMod*> ammoMap;
	static std::unordered_map<BGSMod::Attachment::Mod*, ChamberData> modChamberMap;
	static std::unordered_map<TESObjectMISC*, BGSMod::Attachment::Mod*> miscModMap;
	//static std::unordered_map<BGSMod::Attachment::Mod*, UniqueState> modUniqueStateMap;
	//static std::unordered_map<TESObjectWEAP*, UniqueState> weapUniqueStateMap;
	static std::vector<KeywordValue> uniqueStateAPValues;
	static std::vector<TESAmmo*> dontRemoveAmmoOnReload;
	static std::vector<TESObjectWEAP*> BCRweapons;
	static std::vector<BGSMod::Attachment::Mod*> equippableMods;
	static std::unordered_map<std::string, std::string> translationMap;

	//Mandatory Data, filled during mod initialization
	static KeywordValue ammoAP;
	static KeywordValue magAP;
	static KeywordValue receiverAP;
	static KeywordValue muzzleAP;
	static BGSKeyword* baseModCompatibilityKW;
	static BGSKeyword* hasSwitchedAmmoKW;
	static BGSKeyword* hasUniqueStateKW;
	static BGSKeyword* tacticalReloadKW;
	static ActorValueInfo* BCR_AVIF;
	static ActorValueInfo* BCR_AVIF2;
	static BGSMod::Attachment::Mod* APbaseMod;
	static BGSMod::Attachment::Mod* NullMuzzleMod;
	static BGSProjectile* ProjectileDummy;
	static BGSKeyword* CanHaveNullMuzzleKW;
	static BGSKeyword* FiringModBurstKW;
	static BGSKeyword* FiringModeUnderbarrelKW;
	static BGSKeyword* BallisticWeaponKW;
	static BGSKeyword* MineKW;
	static BGSKeyword* GrenadeKW;
	static BGSKeyword* UnarmedKW;
	static BGSKeyword* Melee1HKW;
	static BGSKeyword* Melee2HKW;
	static BGSKeyword* IsMagKW;
	static BGSKeyword* AnimsEmptyBeforeReloadKW;
	static BGSKeyword* FusionCoreKW;
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
	enum : UInt64
	{
		bReloadEnabled = 0x00000100,
		bDrawEnabled = 0x00000200,
		bCustomAnimEnabled = 0x00000400,
		bAmmoRequireWeaponToBeDrawn = 0x00000800,
		bRequireAmmoToSwitch = 0x00001000,
		bSpawnRandomAmmo = 0x00002000,
		bSpawnRandomMods = 0x00004000,
		bInjectLeveledLists = 0x00008000,
		bWidgetAlwaysVisible = 0x00000001,
		bShowAmmoIcon = 0x00000002,
		bShowMuzzleIcon = 0x00000004,
		bShowAmmoName = 0x00000008,
		bShowMuzzleName = 0x00000010,
		bShowFiringMode = 0x00000020,
		bShowScopeData = 0x00000040,
		bShowQuickkeySelection = 0x00000080,
		bShowUnavailableMods = 0x40000000,
		bEnableMetadataSaving = 0x00010000,
		bEnableExtraWeaponState = 0x00020000,
		bEnableTacticalReloadChamber = 0x00040000,
		bEnableTacticalReloadAnim = 0x00080000,
		bEnableBCRSupport = 0x00100000,
		bReloadCompatibilityMode = 0x00200000,
		bCustomAnimCompatibilityMode = 0x00400000,
		bShowEquippedAmmo = 0x00800000,
		bDisplayChamberedAmmoOnHUD = 0x01000000,
		bDisplayConditionInPipboy = 0x02000000,
		bDisplayMagInPipboy = 0x04000000,
		bDisplayChamberInPipboy = 0x08000000,
		bDisableAutomaticReload = 0x10000000,
		bLowerWeaponAfterSprint = 0x20000000,
		bReplaceAmmoWithSpawned = 0x100000000,
		bEmptyClipBeforeSwitch = 0x80000000,
		bPlayFeedbackSoundAmmo = 0x200000000,
		bPlayFeedbackSoundAmmoFail = 0x400000000,
		bPlayFeedbackSoundMod = 0x800000000,
		bPlayFeedbackSoundModWithoutAnim = 0x1000000000,
		bPlayFeedbackSoundModFail = 0x2000000000,
		bPlayFeedbackSoundMenuOpen = 0x4000000000,
		bPlayFeedbackSoundMenuFail = 0x8000000000,
		bPlayFeedbackSoundQuickkeyFail = 0x1000000000000,
		bPlayFeedbackSoundQuickkeyNext = 0x2000000000000,
		bPatchVanillaAVcalculation = 0x10000000000,
		bRandomizeLoadedAmmoOnSplitStack = 0x20000000000,
		bRandomizeLoadedAmmoOnNewWeapon = 0x40000000000,
		bDontAutolowerWeaponWithFlashlightOn = 0x80000000000,
		bSwitchToNewAmmoTypeWhenDepleted = 0x100000000000,
		bStartDepletedSwitchFromBaseAmmo = 0x200000000000,
		bAllowChangingMSFMenus = 0x400000000000,
		bAllowCancelReload = 0x800000000000,
		bAllowCancelModSwitch = 0x1000000000000,
		bAllowAttackKeyToCancel = 0x2000000000000,
		bAllowReadyKeyToCancel = 0x4000000000000,
		bAllowNonSwitchReloadCancel = 0x8000000000000,
		mMakeExtraRankMask = bEnableExtraWeaponState //| bEnableTacticalReloadAll | bEnableTacticalReloadAnim | bEnableBCRSupport
	};
	static UInt64 MCMSettingFlags;
	static UInt16 iMinRandomAmmo;
	static UInt16 iMaxRandomAmmo;
	static UInt16 iAutolowerTimeMS;
	static UInt16 iReloadAnimEndEventDelayMS;
	static UInt16 iCustomAnimEndEventDelayMS;
	static UInt16 iDrawAnimEndEventDelayMS;
	static MSFWidgetSettings widgetSettings;
	//static std::unordered_map<std::string, float> MCMfloatSettingMap;
};

namespace MSF_Data
{
	bool InitData();
	bool InjectLeveledLists();
	bool PatchBaseAmmoMods();
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
	SwitchData* GetNextAmmoMod(bool isQuickkey = false, bool requireAmmo = false);
	SwitchData* GetNthAmmoMod(UInt32 num);
	SwitchData* GetModForAmmo(TESAmmo* targetAmmo);
	bool GetNthMod(UInt32 num, BGSInventoryItem::Stack* eqStack, ModData* modData);
	bool GetNextMod(BGSInventoryItem::Stack* eqStack, ModData* modData, bool isQuickkey = false);
	ModData::Mod* GetOldModForEquip(BGSInventoryItem::Stack* eqStack, ModData* modData, ModData::Mod* modDataMod);
	bool CheckSwitchRequirements(BGSInventoryItem::Stack* stack, ModData::Mod* modToAttach, ModData::Mod* modToRemove);
	bool QueueModsToSwitch(ModData::Mod* modToAttach, ModData::Mod* modToRemove, BGSSoundDescriptorForm* sound);
	TESAmmo* GetBaseCaliber(BGSObjectInstanceExtra* objectModData, TESObjectWEAP* weapBase);
	bool GetChamberData(BGSObjectInstanceExtra* mods, TESObjectWEAP::InstanceData* weapInstance, UInt16* chamberSize, UInt16* flags);
	bool GetAttachedChildren(BGSObjectInstanceExtra* mods, BGSMod::Attachment::Mod* parent, std::vector<BGSMod::Attachment::Mod*>* children, bool checkIF);
	bool PickRandomMods(std::vector<BGSMod::Attachment::Mod*>* mods, BGSMod::Attachment::Mod** ammoMod, TESAmmo** ammo, UInt32* count);
	TESIdleForm* GetReloadAnimation(Actor* actor);
	TESIdleForm* GetFireAnimation(Actor* actor);
	bool InstanceHasBCRSupport(TESObjectWEAP::InstanceData* instance);
	bool WeaponHasBCRSupport(TESObjectWEAP* weapon);
	bool InstanceHasTRSupport(TESObjectWEAP::InstanceData* instance);
	std::string GetFMString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetScopeString(TESObjectWEAP::InstanceData* instanceData);
	std::string GetMuzzleString(TESObjectWEAP::InstanceData* instanceData);
	KeybindData* GetKeybindDataForKey(UInt16 keyCode, UInt8 modifiers);
}