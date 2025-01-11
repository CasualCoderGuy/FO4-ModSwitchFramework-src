#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Base.h"
#include "f4se\GameThreads.h"

//Anim Event
class BSAnimationGraphEvent
{
public:
	TESForm* unk00;
	BSFixedString eventName;
	//etc
};

class BGSInventoryListEventData
{
public:
	enum
	{
		kAddStack,
		kChangedStack,
		kAddNewItem,
		kRemoveItem,
		kClear,
		UpdateWeight
	};

	struct Event
	{
	public:
		// members
		UInt16 changeType;  // 00
		ObjectRefHandle owner;                      // 04 RE::ObjectRefHandle
		TESBoundObject* objAffected;                // 08
		UInt32 count;                               // 10
		UInt32 stackID;                             // 14
	};
	STATIC_ASSERT(sizeof(Event) == 0x18);
};

class InventoryList
{
public:
	BSTEventDispatcher<BGSInventoryListEventData::Event> eventSource;	// 00
	tArray<BGSInventoryItem> items;		// 58
	float			inventoryWeight;	// 70 - is (-1) when not calculated
	ObjectRefHandle owner;				// 74
	BSReadWriteLock	inventoryLock;		// 78
};

class PlayerInventoryListEventSink : public BSTEventSink<BGSInventoryListEventData::Event>
{
public:
	virtual	EventResult	ReceiveEvent(BGSInventoryListEventData::Event* evn, void * dispatcher) override;
};

struct BGSOnPlayerUseWorkBenchEvent {};
class BGSOnPlayerUseWorkBenchEventSink : public BSTEventSink<BGSOnPlayerUseWorkBenchEvent>
{
public:
	virtual	EventResult	ReceiveEvent(BGSOnPlayerUseWorkBenchEvent* evn, void * dispatcher) override;
};
extern BGSOnPlayerUseWorkBenchEventSink useWorkbenchEventSink;
//DECLARE_EVENT_DISPATCHER(BGSOnPlayerUseWorkBenchEvent, 0x0441A10); lastgen

//Player Mod Event
struct BGSOnPlayerModArmorWeaponEvent
{
	TESBoundObject*						object;			// 00 TESObjectWEAP or TESObjectARMO
	BGSMod::Attachment::Mod*			mod;			// 08
};
class BGSOnPlayerModArmorWeaponEventSink : public BSTEventSink<BGSOnPlayerModArmorWeaponEvent>
{
public:
	virtual	EventResult	ReceiveEvent(BGSOnPlayerModArmorWeaponEvent * evn, void * dispatcher) override;
};
extern BGSOnPlayerModArmorWeaponEventSink modArmorWeaponEventSink;
//DECLARE_EVENT_DISPATCHER(BGSOnPlayerModArmorWeaponEvent, 0x0441790); lastgen

//Cell Load Event
struct TESCellFullyLoadedEvent
{
	TESObjectCELL*				cell;			// 00
};
class TESCellFullyLoadedEventSink : public BSTEventSink<TESCellFullyLoadedEvent>
{
public:
	virtual	EventResult	ReceiveEvent(TESCellFullyLoadedEvent * evn, void * dispatcher) override;
};
extern TESCellFullyLoadedEventSink cellFullyLoadedEventSink;

#if CURRENT_RELEASE_RUNTIME == RUNTIME_VERSION_1_10_163
DECLARE_EVENT_DISPATCHER(TESCellFullyLoadedEvent, 0x00442050); //diff: A0 
#elif CURRENT_RELEASE_RUNTIME == MAKE_EXE_VERSION(1, 10, 980) 
DECLARE_EVENT_DISPATCHER(TESCellFullyLoadedEvent, 0x004DCB30); //diff: D0 //2201825
#elif CURRENT_RELEASE_RUNTIME == MAKE_EXE_VERSION(1, 10, 984) 
DECLARE_EVENT_DISPATCHER(TESCellFullyLoadedEvent, 0x004DCB60);
#endif

//CombatEvent
class CombatEvnHandler : public BSTEventSink<TESCombatEvent>
{
public:
	virtual ~CombatEvnHandler() { };
	virtual EventResult ReceiveEvent(TESCombatEvent * evn, void * dispatcher) override;
};

//Equip event
struct PlayerAmmoCountEvent
{
	UInt32							ammoCount;        // 00
	UInt32							totalAmmoCount;    // 04
	UInt64							unk08;            // 08
	TESObjectWEAP*					weapon;            // 10
	TESObjectWEAP::InstanceData*    weaponInstance;
	//...
};
STATIC_ASSERT(offsetof(PlayerAmmoCountEvent, weapon) == 0x10);

class PlayerAmmoCountEventSink : public BSTEventSink<PlayerAmmoCountEvent>
{
public:
	virtual ~PlayerAmmoCountEventSink() { };
	virtual EventResult ReceiveEvent(PlayerAmmoCountEvent * evn, void * dispatcher) override;
};
extern PlayerAmmoCountEventSink playerAmmoCountEventSink;

//Menu OpenClose Event
class MenuOpenCloseSink : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, void * dispatcher) override;
};
extern MenuOpenCloseSink menuOpenCloseSink;

typedef UInt8(*_PlayerAnimationEvent)(void * thissink, BSAnimationGraphEvent* evnstruct, void** dispatcher);
extern RelocAddr <_PlayerAnimationEvent> PlayerAnimationEvent_HookTarget;
extern _PlayerAnimationEvent PlayerAnimationEvent_Original;

//BSTEventSource<InventoryInterface::CountChangedEvent>
//BSTEventSource<ActorEquipManagerEvent::Event>
//GetEventDispatcher<TESEquipEvent>()->AddEventSink(&_TESEquipEventSink);

namespace ActorEquipManagerEvent
{
	struct EventData
	{
		TESBoundObject* equippedItem;
		TBO_InstanceData* instancedata;
		BGSEquipSlot* equipSlot; //weaponequipslot //maybe not
		UInt64 unk18;								//maybe not
		EquipWeaponData* equippedWeaponData;		//maybe not
	};

	struct Event
	{
		bool equip;
		EventData* data;
		Actor* targetActor;
	};
}

class ActorEquipManager
{
public:
	UInt64 singleton;
	BSTEventDispatcher<ActorEquipManagerEvent::Event> equipUnequipEventSource;
};
extern RelocPtr <ActorEquipManager> g_ActorEquipManager;

class ActorEquipManagerEventSink : public BSTEventSink<ActorEquipManagerEvent::Event>
{
public:
	virtual	EventResult	ReceiveEvent(ActorEquipManagerEvent::Event* evn, void* dispatcher) override;
};
void HelperFn(ActorEquipManagerEvent::Event* evn);

struct PlayerWeaponReloadEvent
{

};

struct PlayerSetWeaponStateEvent
{

};

struct PlayerCrosshairModeEvent
{
	UInt32				mode;			// 00 
};

template <typename T>
class BSTValueEventSink : public BSTEventSink<T>
{
public:
	UInt32		unk00;
	UInt32		unk08;
	UInt32		unk10;
	UInt32		unk18;
};

class HUDAmmoCounter : public HUDComponentBase
{
public:
	BSGFxDisplayObject								ClipCount_tf;		// F8
	BSGFxDisplayObject								ReserveCount_tf;	// 148
	UInt32											ClipCount;			// 198
	UInt32											unk19C;				// 19C init as byte = 0
	UInt32											ReserveCount;		// 1A0
	UInt32											unk1A4;				// 1A4 init as byte = 0
	UInt32											unk1A8;				// 1A8
	UInt32											unk1AC;				// 1AC init as byte = 0
	UInt32											unk1B0;				// 1B0
	UInt32											unk1B4;				// 1B4 init as byte = 0
	BSTValueEventSink<PlayerAmmoCountEvent>			ves1;				// 1B8
	UInt64											unk1D0;
	BSTValueEventSink<PlayerWeaponReloadEvent>		ves2;				// 1D8
	BSTValueEventSink<PlayerCrosshairModeEvent>		ves3;				// 1F0
	UInt64											unk208;
	BSTValueEventSink<PlayerSetWeaponStateEvent>	ves4;				// 210
	UInt64											unk228;
};
STATIC_ASSERT(sizeof(HUDAmmoCounter) == 0x230);

typedef void*(*AttackBlockHandler)(void* handler);
extern RelocAddr <AttackBlockHandler> AttackBlockHandler_HookTarget;
extern RelocAddr <AttackBlockHandler> AttackBlockHandler_Original;

typedef bool (*AttackInputHandler)(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d);
extern RelocAddr <AttackInputHandler> AttackInputHandler_HookTarget;
extern RelocAddr <AttackInputHandler> AttackInputHandler_Original;

struct AmmoCountData
{
	UInt32 ammoCount;
	UInt32 pad04;
	bool unk08;
	UInt16 pad0A;
	bool unk0C;
	UInt16 pad0E;
	bool unk10;
	HUDAmmoCounter* ammoCounter;
};
STATIC_ASSERT(sizeof(AmmoCountData) == 0x20);

typedef AmmoCountData* (*CalcClipAmmoCounter)(AmmoCountData* ammoCountData);
extern RelocAddr <CalcClipAmmoCounter> CalcClipAmmoCounter_HookTarget;
extern RelocAddr <CalcClipAmmoCounter> CalcClipAmmoCounter_Original;
extern CalcClipAmmoCounter CalcClipAmmoCounter_Copied;
typedef UInt64(*HUDShowAmmoCounter)(HUDAmmoCounter* ammoCounter, UInt32 visibleTime);
extern RelocAddr <HUDShowAmmoCounter> HUDShowAmmoCounter_HookTarget;
extern RelocAddr <HUDShowAmmoCounter> HUDShowAmmoCounter_Original;
extern HUDShowAmmoCounter HUDShowAmmoCounter_Copied;
extern RelocPtr <UInt32> uAmmoCounterFadeTimeMS;

//typedef void* (*EquipHandler_UpdateAnimGraph)(Actor* actor, bool unk_rdx);
extern RelocAddr <_UpdateAnimGraph> EquipHandler_UpdateAnimGraph_HookTarget;
extern _UpdateAnimGraph EquipHandler_UpdateAnimGraph_Copied;

extern RelocAddr <_EjectShellCasing> EjectShellCasing_HookTarget1;
extern RelocAddr <_EjectShellCasing> EjectShellCasing_HookTarget2;
extern RelocAddr <_AttachModToStack> AttachModToStack_CallFromGameplay_HookTarget;
extern RelocAddr <_AttachModToStack> AttachModToStack_CallFromWorkbenchUI_HookTarget;
extern RelocAddr <_DeleteExtraData> DeleteExtraData_CallFromWorkbenchUI_HookTarget;
extern RelocAddr <_UpdMidProc> UpdMidProc_HookTarget;
extern RelocAddr <_UpdateEquipData> UpdateEquipData_HookTarget;
extern RelocAddr <_UpdateEquippedWeaponData> UpdateEquippedWeaponData_HookTarget;
extern RelocAddr <uintptr_t> LoadBuffer_ExtraDataList_ExtraRank_JumpHookTarget;
extern RelocAddr <uintptr_t> SkipReloadJumpAddr;
extern RelocAddr <AttackInputHandler> AttackInputHandler_SelfHookTarget;
extern AttackInputHandler AttackInputHandler_Copied;
extern _AttachModToStack AttachModToStack_CallFromGameplay_Copied;
extern _AttachModToStack AttachModToStack_CallFromWorkbenchUI_Copied;
extern _DeleteExtraData DeleteExtraData_CallFromWorkbenchUI_Copied;
extern _UpdMidProc UpdMidProc_Copied;
extern _UpdateEquipData UpdateEquipData_Copied;
extern _UpdateEquippedWeaponData UpdateEquippedWeaponData_Copied;
extern uintptr_t LoadBuffer_ExtraDataList_ExtraRank_ReturnJumpAddr;
extern uintptr_t LoadBuffer_ExtraDataList_ExtraRank_BranchCode;
extern uintptr_t ExtraRankCompare_Copied;
extern RelocAddr <uintptr_t> AmmoReserveCalcAddr;
extern RelocAddr <_RemoveItem_Virtual> RemoveItem_ConsumeAmmo_HookTarget;
extern _EjectShellCasing EjectShellCasing_Copied1;
extern _EjectShellCasing EjectShellCasing_Copied2;

typedef UInt32(*_GetLoadedAmmoCount)(Actor* owner, UInt32 edx);
extern RelocAddr <_GetLoadedAmmoCount> GetLoadedAmmoCount_Original;
extern RelocAddr <_GetLoadedAmmoCount> GetLoadedAmmoCount_HookTarget;
extern _GetLoadedAmmoCount GetLoadedAmmoCount_Copied;
typedef UInt32(*_CheckAmmoForReload)(Actor* target, TESBoundObject* toCount);
extern RelocAddr <_CheckAmmoForReload> CheckAmmoForReload_Original;
extern RelocAddr <_CheckAmmoForReload> CheckAmmoForReload_HookTarget;
extern _CheckAmmoForReload CheckAmmoForReload_Copied;
extern RelocAddr <uintptr_t> CheckAmmoCountForReload_JumpHookTarget;
extern RelocAddr <uintptr_t> CheckAmmoCountForReload_ReturnJumpAddr;
extern uintptr_t CheckAmmoCountForReload_BranchCode;

extern RelocAddr <uintptr_t> CannotEquipItemGen_JumpHookTarget;
extern RelocAddr <uintptr_t> CannotEquipItemMod_JumpHookTarget;
extern RelocAddr <uintptr_t> CannotEquipItemGen_ReturnJumpAddr;
extern RelocAddr <uintptr_t> CannotEquipItemMod_ReturnJumpAddr;
extern RelocAddr <uintptr_t> CannotEquipItem_SkipJumpAddr;
extern RelocAddr <uintptr_t> CannotEquipItem_TextAddr;
extern uintptr_t CannotEquipItem_BranchCode;

struct ReloadJumpReplace {
	uint8_t original[2] = { 0x0F, 0x84 };
	uint8_t replacement[2] = { 0x90, 0xE9 };
};
extern ReloadJumpReplace ReloadJumpOverwriteCode;

class WidgetUpdateTask : public ITaskDelegate
{
public:
	virtual void Run() final
	{
		_DEBUG("widget update thread");
		MSF_Scaleform::UpdateWidgetData();
	}
};

class EndSwitchTask : public ITaskDelegate
{
public:
	EndSwitchTask(UInt16 endFlags)
	{
		flags = endFlags;
	};
	virtual void Run() final
	{
		_DEBUG("end switch thread");
		MSF_Base::EndSwitch(flags);
	}
private:
	UInt16 flags;
};

class ReloadTask : public ITaskDelegate
{
public:
	ReloadTask(bool full, bool clearAmmoCount = false, bool forced = true, bool isSwitch = true)
	{
		bFull = full;
		bClearAmmoCount = clearAmmoCount;
		bForced = forced;
		bIsSwitch = isSwitch;
	};
	virtual void Run() final
	{
		_DEBUG("end switch thread");
		MSF_Base::ReloadWeapon(bFull, bClearAmmoCount, bForced, bIsSwitch);
	}
private:
	bool bFull;
	bool bClearAmmoCount;
	bool bForced;
	bool bIsSwitch;
};

UInt8 PlayerAnimationEvent_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3);
UInt64 HUDShowAmmoCounter_Hook(HUDAmmoCounter* ammoCounter, UInt32 visibleTime);
void* AttackBlockHandler_Hook(void* handler);
bool AttackInputHandler_Hook(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d);
bool AttackInputHandlerReload_Hook(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d);
void* EquipHandler_UpdateAnimGraph_Hook(Actor* actor, bool unk_rdx);
bool AttachModToStack_CallFromGameplay_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* modFunctor, UInt32 unk_r9d, UInt32* unk_rsp20);
bool AttachModToStack_CallFromWorkbenchUI_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* changesFunctor, UInt32 unk_r9d, UInt32* unk_rsp20);
bool DeleteExtraData_CallFromWorkbenchUI_Hook(BSExtraData** extraDataHead, ExtraDataType type);
bool UpdMidProc_Hook(Actor::AIProcess* midProc, Actor* actor, BGSObjectInstance weaponBaseStruct, BGSEquipSlot* equipSlot);
void UpdateEquipData_Hook(BipedAnim* equipData, BGSObjectInstance instance, UInt32* r8d);
void UpdateEquippedWeaponData_Hook(EquippedWeaponData* data);
ExtraRank* LoadBuffer_ExtraDataList_ExtraRank_Hook(ExtraRank* newExtraRank, UInt32 rank, ExtraDataList* futureParentList, BGSInventoryItem::Stack* futureParentStack);
bool ExtraRankCompare_Hook(ExtraRank* extra1, ExtraRank* extra2);
bool CheckAmmoCountForReload_Hook(Actor* target, UInt32 loadedAmmo, UInt32 ammoCap, UInt32 ammoReserve);
const char* CannotEquipItem_Hook(TESObjectREFR* target, TESForm* item, UInt32 unequip, UInt32 type);

bool RegisterInventoryEvent(BGSInventoryList* list, BSTEventSink<BGSInventoryListEventData::Event>* sink);
BSTEventDispatcher<void*>* GetGlobalEventDispatcher(BSTGlobalEvent* globalEvents, const char * dispatcherName);
#define GET_EVENT_DISPATCHER(EventName) (BSTEventDispatcher<EventName>*) GetGlobalEventDispatcher(*g_globalEvents, #EventName);

#define DECLARE_EVENT_DISPATCHER_EX(Event, address) \
template<> inline BSTEventDispatcher<Event> * GetEventDispatcher() \
{ \
	typedef BSTEventDispatcher<Event> * (*_GetEventDispatcher)(); \
	RelocAddr<_GetEventDispatcher> GetDispatcher(address.GetUIntPtr()-RelocationManager::s_baseAddr); \
	return GetDispatcher(); \
}
#define GAME_VM_EVENTS_REG_SIG "E8 ? ? ? ? 49 8B 0F 49 8D 56 10"
#define InventoryEventHandler_EVENTS_REG_SIG "48 89 5C 24 08 57 48 83 EC 20 33 FF 48 8B D9 48 85 C9 74 ? 48 83 C1 10 EB ?"
#define InventoryEventHandler_INDIRECTIONS(_offset) { { _offset,1,5 },{ 0x9,1,5 } }
#define BASIC_EVENT_HANDLER_INDIRECTIONS(_offset) { {0,1,5},{0x14,1,5},{ _offset,1,5 },{ 0x9,1,5 } }
#define BASIC_EVENT_HANDLER_INDIRECTIONS1(_offset) { {0,1,5},{0x14,1,5},{ _offset,1,5 },{ 0x35,1,5 } }
#define FRAGMENT_EVENT_HANDLER_INDIRECTIONS(_offset) { {0,1,5},{0x2C,1,5},{ _offset,1,5 },{ 0x9,1,5 } }
#define REGISTER_EVENT(_event, _sink) GetEventDispatcher<_event>()->AddEventSink(&_sink);
