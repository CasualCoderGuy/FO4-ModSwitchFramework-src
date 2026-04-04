#pragma once
#include "f4se/ScaleformLoader.h"
#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/CustomMenu.h"
#include "f4se/PapyrusScaleformAdapter.h"
#include "f4se/PapyrusEvents.h"
#include "f4se/GameInput.h"
#include "f4se/InputMap.h"
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Base.h"
#include "MSF_Addresses.h"

class ItemMenuDataManager;

namespace MSF_Scaleform
{
	void ReceiveKeyEvents();
	void RegisterMSFCustomMenus();
	bool RegisterScaleformCallback(GFxMovieView* view, GFxValue* f4se_root);
	bool RegisterScaleformTest(GFxMovieView * view, GFxValue * f4se_root);
	void RegisterMSFScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot);
	bool StartWidgetHideCountdown(UInt32 delayTime);
	bool UpdateWidgetData(TESObjectWEAP::InstanceData* instanceData = nullptr);
	bool UpdateWidgetSettings();
	bool UpdateWidgetQuickkeyMod(KeywordValue ap, TESForm* nameForm, std::string nullModName, bool isAmmo);
	bool ClearWidgetQuickkeyMod();
	UInt32 GetInterfaceVersion(); 
	bool HandleToggleMenu(ModSelectionMenu* selectMenu, ModData* mods);
	bool DisplaySelectionMenu(ModSelectionMenu* selectMenu, ModData* mods, GFxMovieRoot* menuRoot);
	bool ToggleGlobalMenu(ModSelectionMenu* selectMenu, std::vector<KeywordValue>* attachPoints);
	bool UpdateAmmoMenuCount(TESAmmo* ammo, UInt32 newCount);
	bool UpdateAmmoMenuEq(BGSMod::Attachment::Mod* ammoMod);
	bool UpdateModMenuReqs(TESObjectMISC* misc, UInt32 newCount);
	void RegisterStringW(GFxValue* dst, GFxMovieRoot* root, const char* name, const wchar_t* str);
	void RegisterString(GFxValue* dst, GFxMovieRoot* root, const char* name, const char* str);
	void RegisterObject(GFxValue* dst, GFxMovieRoot* root, const char* name, void* obj);
	void RegisterBool(GFxValue* dst, GFxMovieRoot* root, const char* name, bool value);
	void RegisterInt(GFxValue* dst, GFxMovieRoot* root, const char* name, SInt32 value);
}

class BSBinarySerializer;

class PipboyValueEx
{
public:

	enum SERIALIZATION_DATA_TYPE : std::uint8_t
	{
		kBool = 0x0,
		kInt8 = 0x1,
		kUint8 = 0x2,
		kInt32 = 0x3,
		kUint32 = 0x4,
		kFloat = 0x5,
		kString = 0x6,
		kArray = 0x7,
		kObject = 0x8,
	};

	virtual ~PipboyValueEx() = default;  // 00

	// add
	virtual void                    CleanDirtyToGame();                                                        // 01
	virtual void                    Serialize(void* a_json) = 0;                                        // 02
	virtual void                    SerializeChanges(BSBinarySerializer& a_serializer, bool a_fullSerialize);  // 03
	virtual SERIALIZATION_DATA_TYPE GetType() = 0;                                                             // 04

	// members
	UInt32 id;                  // 08
	bool          isDirtyGame;         // 0C
	bool          isDirtyToCompanion;  // 0D
	PipboyValueEx* parentValue;         // 10
};
STATIC_ASSERT(sizeof(PipboyValueEx) == 0x18);

class PipboyObjectEx : public PipboyValueEx
{
public:

	virtual ~PipboyObjectEx();  // 00

	// override
	virtual void                    CleanDirtyToGame() override;                                                        // 01
	virtual void                    Serialize(void* a_json) override;                                            // 02
	virtual void                    SerializeChanges(BSBinarySerializer& a_serializer, bool a_fullSerialize) override;  // 03
	virtual SERIALIZATION_DATA_TYPE GetType() override;                                                                 // 04

	template <typename T>
	T GetMember(const BSFixedString& a_name)
	{
		const auto it = memberMap.find(a_name);
		return (it != memberMap.end()) ? static_cast<T>(it->second) : nullptr;
	}

	void AddMember(const BSFixedString* a_name, PipboyValueEx* a_member)
	{
		//using func_t = decltype(&PipboyObject::AddMember);
		//static REL::Relocation<func_t> func{ ID::PipboyObject::AddMember };
		//return func(this, a_name, a_member);
	}

	// members
	BSTHashMap<BSFixedString, PipboyValueEx*> memberMap;         // 18
	BSTSet<std::uint32_t>                   addedMemberIDs;    // 48
	tArray<std::uint32_t>                 removedMemberIDs;  // 78
	bool                                    newlyCreated;      // 90
};
STATIC_ASSERT(sizeof(PipboyObjectEx) == 0x98);

template <class T>
class PipboyPrimitiveValueEx : public PipboyValueEx  // Forward declaration for specialization **only**
{
public:
	virtual ~PipboyPrimitiveValueEx() {}  // 00

	// override
	virtual void                    CleanDirtyToGame() override {}                                                                                              // 00
	virtual void                    Serialize(void* a_json) override {}                                                                 // 01
	virtual void                    SerializeChanges(BSBinarySerializer& a_serializer, bool a_fullSerialization) override {}  // 03
	virtual SERIALIZATION_DATA_TYPE GetType() override { return SERIALIZATION_DATA_TYPE::kUint32; }

	operator T() const
	{
		return value;
	}

	// members
	T value;  // 18
};

template <class T>
class PipboyPrimitiveThrottledValue : public PipboyPrimitiveValue<T>
{
public:

	void* unk20;
	void* unk28;
	T value1;
	void* unk38;
	void* unk40;
	void* unk48;
	void* unk50;
	void* unk58;
	void* unk60;
	void* unk68;
};
STATIC_ASSERT(sizeof(PipboyPrimitiveThrottledValue<bool>) == 0x70);

class PipboyArray : public PipboyValueEx  // 00
{
public:

	// members
	tArray<PipboyValueEx*>  values;           // 18
	BSTSet<std::uint32_t>   addedElementIDs;    // 30
	tArray<std::uint32_t> removedElementIDs;  // 60
	bool                    newlyCreated;       // 78
};
STATIC_ASSERT(sizeof(PipboyArray) == 0x80);

class PipboySubMenuEx : public BSTEventSink<PipboyValueChangedEvent>  // 00
{
public:

	// override (BSTEventSink<PipboyValueChangedEvent>)
	virtual EventResult	ReceiveEvent(PipboyValueChangedEvent& a_event, void* a_source);


	// add
	virtual void UpdateData() = 0;  // 02

	// members
	GFxValue* dataObj;  // 08
	GFxValue* menuObj;  // 10
};
STATIC_ASSERT(sizeof(PipboySubMenuEx) == 0x18);

class PipboyInventoryMenu : public PipboySubMenuEx  // 00
{
public:

	// override (PipboySubMenu)
	virtual void UpdateData() override
	{
	}

	void SetQuickkey(int a_selectedIndex, int a_quickkeyIndex)
	{
	}
};
STATIC_ASSERT(sizeof(PipboyInventoryMenu) == 0x18);

class IdleInputEvent
{
public:
	// members
	bool starting;  // 00
};
STATIC_ASSERT(sizeof(IdleInputEvent) == 0x1);

class PipboyMenu : public GameMenuBase
{
public:
	class ScaleformArgs
	{
	public:

		GFxValue* result;	// 00
		GFxMovieView* movie;	// 08
		GFxValue* thisObj;	// 10
		GFxValue* unk18;	// 18
		GFxValue* args;		// 20
		UInt32			numArgs;	// 28
		UInt32			pad2C;		// 2C
		UInt32			optionID;	// 30 pUserData
	};

	//using FnInvoke = void(__thiscall PipboyMenu::*)(ScaleformArgs*);
	//static FnInvoke Invoke_Original;

	bool CreateItemDataW(PipboyMenu::ScaleformArgs* args, std::wstring text, std::string value);
	bool CreateItemData(PipboyMenu::ScaleformArgs* args, std::string text, std::string value);

	BSTEventSink<MenuOpenCloseEvent> openCloseEvn;		// 0E0
	BSTEventSink<IdleInputEvent> inputEvn;				// 0E8
	GFxValue				dataObj;                    // 0F0
	UInt64					unk110[0x50 >> 3];			// 110
	PipboyInventoryMenu		inventoryMenuObj;           // 160
	UInt64					unk178[0xE0 >> 3];			// 178
	//PipboyStatsMenu       statsMenuObj;               // 110
	//PipboySpecialMenu     specialMenuObj;             // 130
	//PipboyPerksMenu       perksMenuObj;               // 148
	//PipboyInventoryMenu   inventoryMenuObj;           // 160
	//PipboyQuestMenu       questMenuObj;               // 178
	//PipboyWorkshopMenu    workshopMenuObj;            // 190
	//PipboyLogMenu         logMenuObj;                 // 1A8
	//PipboyMapMenu         mapMenuObj;                 // 1C0
	//PipboyRadioMenu       radioMenuObj;               // 220
	//PipboyPlayerInfoMenu  playerInfoMenuObj;          // 240
	UInt8				  disableInputCounter;        // 258
	bool                  pipboyCursorEnabled;        // 259
	bool                  showingModalMessage;        // 25A
	bool                  pipboyHiddenByAnotherMenu;  // 25B
	bool                  performFastTravelCheck;     // 25C

};
STATIC_ASSERT(offsetof(PipboyMenu, disableInputCounter) == 0x258);
STATIC_ASSERT(sizeof(PipboyMenu) == 0x260);

typedef void(*_PipboyMenuInvoke)(PipboyMenu* menu, PipboyMenu::ScaleformArgs* args);
extern RelocAddr<_PipboyMenuInvoke> PipboyMenuInvoke_HookAddress;
extern RelocAddr<_PipboyMenuInvoke> PipboyMenuInvoke_Original;
extern _PipboyMenuInvoke PipboyMenuInvoke_Copied;
void PipboyMenuInvoke_Hook(PipboyMenu* menu, PipboyMenu::ScaleformArgs* args);


typedef TESForm*(*_GetSelectedForm)(ItemMenuDataManager* mgr, UInt32& handleID);
extern RelocAddr <_GetSelectedForm> GetSelectedForm_Original;
typedef BGSInventoryItem* (*_GetSelectedItem)(ItemMenuDataManager* mgr, UInt32& handleID);
extern RelocAddr <_GetSelectedItem> GetSelectedItem_Original;

extern RelocAddr<uintptr_t> IMenuCreateHook_Start;
typedef void(*_LoadCustomMenu)(IMenu* menu);
extern _LoadCustomMenu LoadCustomMenu_F4SEHook;
void LoadMSFCustomMenu_Hook(IMenu* menu);

class ItemMenuDataManager
{
public:
	void* unk00;
	UInt32				unk08;
	UInt32				unk0C;
	tArray<void*>		unkArray10;	// EventSinks
	tArray<void*>		unkArray28;
	tArray<void*>		unkArray40;
	UInt8				unk58;
	UInt8				pad59[7];
	UInt32				unk60;
	UInt32				unk64;
	tArray<void*>		unkArray68;
	tArray<void*>		unkArray80;
	tArray<void*>		unkArray98;
	UInt8				unkB0;
	UInt8				padB1[7];
	struct Entry
	{
	public:
		UInt32			handleID; // 00
		UInt32			ownerHandle; // 04
		UInt16			itemPosition; // 08 position in inventoryList
		UInt16			unk0A;
	};
	STATIC_ASSERT(sizeof(Entry) == 0xC);
	tArray<Entry>		inventoryItems; // B8

	//DEFINE_MEMBER_FN_1(GetSelectedForm, TESForm*, ID(1453201, 2194011).offset(), UInt32& handleID);
	//DEFINE_MEMBER_FN_1(GetSelectedItem, BGSInventoryItem*, ID(1200959, 2194009).offset(), UInt32& handleID);

	inline TESForm* GetSelectedForm(UInt32& handleID)
	{
		return GetSelectedForm_Original(this, handleID);
	}
	inline BGSInventoryItem* GetSelectedItem(UInt32& handleID)
	{
		return GetSelectedItem_Original(this, handleID);
	}
	//BGSInventoryItem
};
STATIC_ASSERT(sizeof(ItemMenuDataManager) == 0xD0);
extern RelocPtr<ItemMenuDataManager*> g_itemMenuDataMgr;

namespace ActorEquipManagerEvent
{
	struct Event;
}

namespace ActorValueEvents
{
	class ActorValueChangedEvent;
}

namespace BGSInventoryItemEvent
{
	class Event;
}

namespace HolotapeStateChanged
{
	class Event;
}

namespace FavoriteMgr_Events
{
	class ComponentFavoriteEvent;
}

namespace PerkValueEvents
{
	class PerkValueChangedEvent;
	class PerkEntryUpdatedEvent;
}

namespace PlayerDifficultySettingChanged
{
	class Event;
}

struct PipboyValueChangedEvent;

class PipboyDataGroup :
	public BSTEventDispatcher<PipboyValueChangedEvent>  // 00
{
public:

	virtual ~PipboyDataGroup() = default;  // 00

	// override
	virtual EventResult	ReceiveEvent(const PipboyValueChangedEvent& a_event, void* a_source);  // 01

	// add
	virtual void Populate(bool a_arg1) = 0;  // 02
	virtual void DoClearData() = 0;          // 03
	virtual void DoClearSink() = 0;          // 04

	//void LockDataGroup()
	//{
	//	using func_t = decltype(&PipboyDataGroup::LockDataGroup);
	//	static REL::Relocation<func_t> func{ ID::PipboyDataGroup::LockDataGroup };
	//	return func(this);
	//}

	//void UnlockDataGroup()
	//{
	//	using func_t = decltype(&PipboyDataGroup::UnlockDataGroup);
	//	static REL::Relocation<func_t> func{ ID::PipboyDataGroup::UnlockDataGroup };
	//	return func(this);
	//}

	// members
	CRITICAL_SECTION pipboyDataMutex;  // 60
	void* throttleManager;  // 88
	PipboyValueEx* rootValue;        // 90
};
STATIC_ASSERT(sizeof(PipboyDataGroup) == 0x98);

class PipboyInventoryData :
	public PipboyDataGroup,                                           // 00
	public BSTEventSink<BGSInventoryListEvent::Event>,                // 98
	public BSTEventSink<ActorEquipManagerEvent::Event>,               // A0
	public BSTEventSink<ActorValueEvents::ActorValueChangedEvent>,    // A8
	public BSTEventSink<PerkValueEvents::PerkEntryUpdatedEvent>,      // B0
	public BSTEventSink<PerkValueEvents::PerkValueChangedEvent>,      // B8
	public BSTEventSink<InventoryInterface::FavoriteChangedEvent>,    // C0
	public BSTEventSink<HolotapeStateChanged::Event>,                 // C8
	public BSTEventSink<BGSInventoryItemEvent::Event>,                // D0
	public BSTEventSink<FavoriteMgr_Events::ComponentFavoriteEvent>,  // D8
	public BSTEventSink<PlayerDifficultySettingChanged::Event>        // E0
{
public:

	virtual ~PipboyInventoryData();

	enum class ENTRY_TYPE
	{
		kInt = 0x0,
		kFixedString = 0x1,
		kFloat = 0x1
	};

	enum class ITEM_ICON_ID
	{
		kNone = 0x1,
		kBasicPistol = 0x2,
		kLaserPistol = 0x3,
		kPlasmaPistol = 0x4,
		kShotgun = 0x5,
		kBasicRifle = 0x6,
		kAssaultRifle = 0x7,
		kGaussRifle = 0x8,
		kLaserMusket = 0x9,
		kLaserRifle = 0xA,
		kPlasmaRifle = 0xB,
		k1hMelee = 0xC,
		k2hMelee = 0xD,
		kH2hWeap = 0xE,
		kMissileLauncher = 0xF,
		kFatman = 0x10,
		kMinigun = 0x11,
		kGatlingLaser = 0x12,
		kCryolator = 0x13,
		kFlamer = 0x14,
		kFlareGun = 0x15,
		kGammaGun = 0x16,
		kJunkJet = 0x17,
		kRailwayRifle = 0x18,
		kSyringer = 0x19,
		kBroadsider = 0x1A,
		kAlienBlaster = 0x1B,
		kRipper = 0x1C,
		kShishkebab = 0x1D,
		kThrownWeap = 0x1E,
		kExplosiveMine = 0x1F,
		kBottlecapMine = 0x20,
		kCryoMine = 0x21,
		kCryoGrenade = 0x22,
		kPlasmaMine = 0x23,
		kPlasmaGrenade = 0x24,
		kPulseMine = 0x25,
		kPulseGrenade = 0x26,
		kMolotov = 0x27,
		kNukaGrenade = 0x28,
		kRepairKit = 0x29,
		kSurgeryKit = 0x2A,
		kArmor = 0x2B,
		kGloves = 0x2C,
		kHelmet = 0x2D,
		kBook = 0x2E,
		kClothes = 0x2F,
		kAmmo = 0x30,
		kAdrenaline = 0x31,
		kDisease = 0x32,
		kSleep = 0x33,
		kHunger = 0x34,
		kThirst = 0x35,
		kChems = 0x36,
		kStimpak = 0x37,
		kFood = 0x38,
		kAlcohol = 0x39,
		kStealthBoy = 0x3A,
		kUnknown = 0x3B,
		kMax = 0x3B
	};

	enum class SORT_ON_FIELD
	{
		kAlphabetically = 0x0,
		kDamage = 0x1,
		kRof = 0x2,
		kRange = 0x3,
		kAccuracy = 0x4,
		kValue = 0x5,
		kWeight = 0x6
	};

	class StackEntry
	{
	public:
		// members
		PipboyObject* linkedObject;     // 00
		UInt32 inventoryHandle;  // 08
	};
	STATIC_ASSERT(sizeof(StackEntry) == 0x10);

	class ItemEntry
	{
	public:
		// members
		BSTList<PipboyInventoryData::StackEntry*> stackEntries;  // 00 - BSTList<PipboyInventoryData::StackEntry*>
	};
	STATIC_ASSERT(sizeof(ItemEntry) == 0x8);

	// override
	virtual EventResult	ReceiveEvent(const struct PipboyValueChangedEvent& a_event, void* a_source);  // 01

	// add
	virtual EventResult	ReceiveEvent(const BGSInventoryListEvent::Event& a_event, void* a_source);                              // 02
	virtual EventResult	ReceiveEvent(const ActorEquipManagerEvent::Event& a_event, void* a_source);                            // 03
	virtual EventResult	ReceiveEvent(const ActorValueEvents::ActorValueChangedEvent& a_event, void* a_source);      // 04
	virtual EventResult	ReceiveEvent(const PerkValueEvents::PerkEntryUpdatedEvent& a_event, void* a_source);          // 05
	virtual EventResult	ReceiveEvent(const PerkValueEvents::PerkValueChangedEvent& a_event, void* a_source);          // 06
	virtual EventResult	ReceiveEvent(const InventoryInterface::FavoriteChangedEvent& a_event, void* a_source);      // 07
	virtual EventResult	ReceiveEvent(const HolotapeStateChanged::Event& a_event, void* a_source);                                // 08
	virtual EventResult	ReceiveEvent(const BGSInventoryItemEvent::Event& a_event, void* a_source);                              // 09
	virtual EventResult	ReceiveEvent(const FavoriteMgr_Events::ComponentFavoriteEvent& a_event, void* a_source);  // 0A
	virtual EventResult	ReceiveEvent(const PlayerDifficultySettingChanged::Event& a_event, void* a_source);            // 0B

	virtual void Populate(bool a_arg1);  // 0C
	virtual void DoClearData();          // 0D
	virtual void DoClearSink();          // 0E

	//void AddItemCardInfoEntry(const BSFixedStringCS* a_string, float a_value, PipboyArray* a_itemCardSection)
	//{
	//	using func_t = decltype(&PipboyInventoryData::AddItemCardInfoEntry);
	//	static REL::Relocation<func_t> func{ ID::PipboyInventoryData::AddItemCardInfoEntry };
	//	return func(this, a_string, a_value, a_itemCardSection);
	//}

	//PipboyObject* BaseAddItemCardInfoEntry(const BSFixedStringCS* a_string, PipboyArray* a_itemCardSection)
	//{
	//	using func_t = decltype(&PipboyInventoryData::BaseAddItemCardInfoEntry);
	//	static REL::Relocation<func_t> func{ ID::PipboyInventoryData::BaseAddItemCardInfoEntry };
	//	return func(this, a_string, a_itemCardSection);
	//}

	//void PopulateItemCardInfo(const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data)
	//{
	//	using func_t = decltype(&PipboyInventoryData::PopulateItemCardInfo);
	//	static REL::Relocation<func_t> func{ ID::PipboyInventoryData::PopulateItemCardInfo };
	//	return func(this, a_inventoryItem, a_stack, a_data);
	//}

	//void QueueItemCardRepopulate(ENUM_FORM_ID a_formType)
	//{
	//	using func_t = decltype(&PipboyInventoryData::QueueItemCardRepopulate);
	//	static REL::Relocation<func_t> func{ ID::PipboyInventoryData::QueueItemCardRepopulate };
	//	return func(this, a_formType);
	//}

	//void RepopulateItemCardOnSection(ENUM_FORM_ID a_itemTypeID)
	//{
	//	using func_t = decltype(&PipboyInventoryData::RepopulateItemCardOnSection);
	//	static REL::Relocation<func_t> func{ ID::PipboyInventoryData::RepopulateItemCardOnSection };
	//	return func(this, a_itemTypeID);
	//}

	// members
	PipboyObjectEx* inventoryObject;             // E8
	BSTHashMap<std::uint32_t, PipboyInventoryData::ItemEntry> itemEntries;                 // F0
	BSTHashMap<const BGSComponent*, PipboyObjectEx*>            invComponents;               // 120
	tArray<PipboyObjectEx*>                                   sortedItems;                 // 150
	BSTSet<UInt32>                                      queuedRepopulateCategories;  // 168
};
STATIC_ASSERT(sizeof(PipboyInventoryData) == 0x198);

//class PipboyDataManager
//{
//public:
//	//4B8
//	UInt64							unk00[0x4A8 >> 3];
//	tArray<PipboyObject*>			itemData;
//};
//STATIC_ASSERT(sizeof(PipboyDataManager) == 0x4C0);
//class PipboyDataManager
//{
//public:
//
//	// members
//	PipboyObject          rootObject;       // 008
//	PipboyStatsData       statsData;        // 0A0
//	PipboySpecialData     specialData;      // 1D8
//	PipboyPerksData       perksData;        // 280
//	PipboyInventoryData   inventoryData;    // 358
//	PipboyQuestData       questData;        // 4F0
//	PipboyWorkshopData    workshopData;     // 5D8
//	PipboyLogData         logData;          // 718
//	PipboyMapData         mapData;          // 7F0
//	PipboyRadioData       radioData;        // A28
//	PipboyPlayerInfoData  playerInfoData;   // AD8
//	PipboyStatusData      statusData;       // BE0
//	PipboyDataGroup* dataGroups[11];   // CC0
//	PipboyThrottleManager throttleManager;  // D18
//};
//STATIC_ASSERT(sizeof(PipboyDataManager) == 0xD48);
class PipboyDataManager
{
public:
	UInt64 vtbl;
	PipboyObjectEx          rootObject;       // 008
	UInt64							unkA0[0x2B8 >> 3]; //A0
	PipboyInventoryData   itemData;    // 358
};
STATIC_ASSERT(offsetof(PipboyDataManager, rootObject) == 0x8);
STATIC_ASSERT(offsetof(PipboyDataManager, itemData) == 0x358);
extern RelocPtr<PipboyDataManager*> g_pipboyDataMgr;



class HUDMenuAmmoDisplay
{
public:
	static HUDMenuAmmoDisplay* Init(GFxMovieRoot* root = nullptr)
	{
		if (!root)
		{
			IMenu* menu = (*g_ui)->GetMenu(BSFixedString("HUDMenu"));
			if (!menu) 
				return nullptr;
			GFxMovieView* movie = menu->movie;
			if (!movie) 
				return nullptr; 
			root = movie->movieRoot;
			if (!root) 
				return nullptr;
		}
		HUDMenuAmmoDisplay* tf = new HUDMenuAmmoDisplay(root);
		if (tf->IsOK())
			return tf;
		delete tf;
		return nullptr;
	}
	bool IsOK() { return textfieldOK; }
	bool SetDisplayedAmmo(UInt32 newmag, UInt32 newchamber, UInt32 newreserve, bool hasChamber)
	{
		if (!movieRoot || !textfieldOK)
			return false;
		mag = newmag;
		reserve = newreserve;
		if (chamber != newchamber)
		{
			chamber = newchamber;
			if (!hasChamber)
				chamber = 0;
		}
		return true;
	}
	UInt32 GetDisplayedChamberAmmo()
	{
		return chamber;
	}
private:
	HUDMenuAmmoDisplay(GFxMovieRoot* root = nullptr)
	{
		textfieldOK = false;
		movieRoot = root;
		formatting = "+%u";
		mag = 0; chamber = 0; reserve = 0;
		textfieldOK = true;
	}

	bool textfieldOK;
	GFxMovieRoot* movieRoot;
	GFxValue ChamberCount_tf;
	GFxValue ReserveCount_tf;
	GFxValue ClipCount_tf;
	GFxValue AmmoType_tf;
	GFxValue FiringMode_tf;
	GFxValue Muzzle_tf;
	GFxValue Scope_tf;
	GFxValue TextFormatting;
	std::string formatting;
	char chamberText[16];
	UInt32 mag;
	UInt32 chamber;
	UInt32 reserve;
	bool chamberVisible;
};

class MSFWidgetMenu : public GameMenuBase
{
public:
	MSFWidgetMenu() : GameMenuBase()
	{
		enum
		{
			//Confirmed
			kFlag_PauseGame = 0x01,
			kFlag_DoNotDeleteOnClose = 0x02,
			kFlag_ShowCursor = 0x04,
			kFlag_EnableMenuControl = 0x08, // 1, 2
			kFlag_ShaderdWorld = 0x20,
			kFlag_Open = 0x40,//set it after open.
			kFlag_DoNotPreventGameSave = 0x800,
			kFlag_ApplyDropDownFilter = 0x8000, //
			kFlag_BlurBackground = 0x400000,

			//Unconfirmed
			kFlag_Modal = 0x10,
			kFlag_PreventGameLoad = 0x80,
			kFlag_Unk0100 = 0x100,
			kFlag_HideOther = 0x200,
			kFlag_DisableInteractive = 0x4000,
			kFlag_UpdateCursorOnPlatformChange = 0x400,
			kFlag_Unk1000 = 0x1000,
			kFlag_ItemMenu = 0x2000,
			kFlag_Unk10000 = 0x10000,	// mouse cursor
			kFlag_Unk800000 = 0x800000
		};
		flags = kFlag_AllowSaving | kFlag_AlwaysOpen | kFlag_DontHideCursorWhenTopmost;// | kFlag_ApplyDropDownFilter | kFlag_Unk800000;// | 0x40000 | 0x80000 | 0x200000;
		depth = 0x6;
		if (LoadMovieEx(*g_scaleformManager, this, this->movie, "MSFwidget", "root1", 0, 0.0F))//(CALL_MEMBER_FN((*g_scaleformManager), LoadMovie)(this, this->movie, "MSFwidget", "root1", 0))
		{

			_MESSAGE("MSF widget loaded.");

			CreateBaseShaderTarget(this->filterHolder, this->stage);

			//inherit_colors
			this->filterHolder->SetFilterColor(false);
			(*g_colorUpdateDispatcher)->eventDispatcher.AddEventSink(this->filterHolder);

			if (flags & kFlag_ApplyDropDownFilter)
				this->shaderFXObjects.Push(this->filterHolder);
		}
	}
	virtual void	Invoke(Args * args) final
	{
		switch (args->optionID)
		{
		case 0:
			break;
		default:
			break;
		}
	}

	virtual void	RegisterFunctions() final
	{
		_MESSAGE("RegisterFunctions");
	}

	virtual UInt32	ProcessMessage(UIMessage * msg) final
	{
		GFxMovieRoot * root = movie->movieRoot;
		GFxValue ValueToSet;
		//_DEBUG("message proc!!");
		return this->GameMenuBase::ProcessMessage(msg);
	};

	virtual void	DrawNextFrame(float unk0, void * unk1) final
	{
		return this->GameMenuBase::AdvanceMovie(unk0, unk1);
	};

	static IMenu * CreateMSFMenu()
	{
		return new MSFWidgetMenu();
	}

	static void OpenMenu()
	{
		static BSFixedString menuName("MSFwidget");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
	}

	static void CloseMenu()
	{
		static BSFixedString menuName("MSFwidget");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Close);
	}

	static void RegisterMenu()
	{
		static BSFixedString menuName("MSFwidget");
		if ((*g_ui) != nullptr && !(*g_ui)->IsMenuRegistered(menuName))
		{
			(*g_ui)->Register("MSFwidget", CreateMSFMenu);
		}
		_MESSAGE("MSFwidget %s", (*g_ui)->IsMenuRegistered(menuName) ? "registered" : "not registered");
	}
};

class MSFMenu : public GameMenuBase
{
public:

	MSFMenu() : GameMenuBase()
	{
		flags = kFlag_AllowSaving | kFlag_AlwaysOpen;// | kFlag_UsesCursor | kFlag_UpdateUsesCursor | kFlag_PausesGame | kFlag_FreezeFrameBackground //| kFlag_ApplyDropDownFilter | kFlag_Unk800000 | kFlag_DisableInteractive;// | 0x40000 | 0x80000 | 0x200000;
		//kFlag_UsesMenuContext kFlag_FreezeFramePause kFlag_DontHideCursorWhenTopmost kFlag_UsesBlurredBackground
		depth = 0x6;
		if (LoadMovieEx(*g_scaleformManager, this, this->movie, "MSFMenu", "root1", 0, 0.0F))//(CALL_MEMBER_FN((*g_scaleformManager), LoadMovie)(this, this->movie, "MSFMenu", "root1", 0))
		{

			_MESSAGE("MSF Menu loaded.");

			CreateBaseShaderTarget(this->filterHolder, this->stage);

			//inherit_colors
			this->filterHolder->SetFilterColor(false);
			(*g_colorUpdateDispatcher)->eventDispatcher.AddEventSink(this->filterHolder);

			if (flags & kFlag_CustomRendering)
				this->shaderFXObjects.Push(this->filterHolder);
		}
	}

	virtual void	Invoke(Args * args) final
	{
		switch (args->optionID)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		default:
			break;
		}
	}

	virtual void	RegisterFunctions() final
	{
		_MESSAGE("RegisterFunctions");
	}

	virtual UInt32	ProcessMessage(UIMessage * msg) final
	{
		return this->GameMenuBase::ProcessMessage(msg);
	};

	virtual void	DrawNextFrame(float unk0, void * unk1) final
	{
		//_DMESSAGE("DrawNextFrame");
		return this->GameMenuBase::AdvanceMovie(unk0, unk1);
	};

	static IMenu * CreateMSFMenu()
	{
		return new MSFMenu();
	}

	static void OpenMenu()
	{
		static BSFixedString menuName("MSFMenu");
		//_DEBUG("opening menu");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
	}

	static void CloseMenu()
	{
		static BSFixedString menuName("MSFMenu");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Close);
	}

	static void RegisterMenu()
	{
		static BSFixedString menuName("MSFMenu");
		if ((*g_ui) != nullptr && !(*g_ui)->IsMenuRegistered(menuName))
		{
			(*g_ui)->Register("MSFMenu", CreateMSFMenu);
			_MESSAGE("MSFMenu %s", (*g_ui)->IsMenuRegistered(menuName) ? "registered" : "not registered");
		}
	}
};

class MSFCustomMenuData
{
public:
	std::string	menuName;
	std::string	menuPath;
	std::string	rootPath;
	UInt32			menuFlags;
	UInt32			movieFlags;
	UInt32			extFlags;
	UInt32			depth;
	bool			isValid;

	enum ExtendedFlags
	{
		kExtFlag_InheritColors = 0x01,
		kExtFlag_CheckForGamepad = 0x02,
		kExtFlag_DontAllowMenuChange = 0x04
	};

	enum MovieType
	{
		kNoScale,
		kShowAll,
		kExactFit,
		kNoBorder
	};
};

class MSFCustomMenu : CustomMenu
{
public:
	static bool RegisterCustomMenu(MSFCustomMenuData* menuData)
	{
		if (menuData && !(*g_ui)->IsMenuRegistered(BSFixedString(menuData->menuName.c_str())))
		{
			menuLock.Lock();
			customMenuData[menuData->menuName.c_str()] = menuData;
			(*g_ui)->Register(menuData->menuName.c_str(), CreateCustomMSFMenu);
			menuLock.Release();
			return true;
		}
		return false;
	}

	static void OpenMenu(BSFixedString menuName)
	{
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
	}

	static void CloseMenu(BSFixedString menuName)
	{
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Close);
	}

	static IMenu* CreateCustomMSFMenu()
	{
		return new MSFCustomMenu();
	}

	static SimpleLock menuLock;
	static std::unordered_map<std::string, MSFCustomMenuData*> customMenuData;
	static std::vector<MSFCustomMenu*> menuHandles;
};
