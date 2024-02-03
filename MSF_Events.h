#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Base.h"

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
		UInt16 type;  // 00
		UInt16 changeType;
		UInt32 owner;                      // 04 RE::ObjectRefHandle
		TESBoundObject* objAffected;                // 08
		UInt32 count;                               // 10
		UInt32 stackID;                             // 14
	};
	//STATIC_ASSERT(sizeof(Event) == 0x18);
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
DECLARE_EVENT_DISPATCHER(BGSOnPlayerUseWorkBenchEvent, 0x0441A10);

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
DECLARE_EVENT_DISPATCHER(BGSOnPlayerModArmorWeaponEvent, 0x0441790);

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
DECLARE_EVENT_DISPATCHER(TESCellFullyLoadedEvent, 0x00442050);

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


typedef UInt64(*HUDShowAmmoCounter)(HUDAmmoCounter* ammoCounter, UInt32 visibleTime);
extern RelocAddr <HUDShowAmmoCounter> HUDShowAmmoCounter_HookTarget;
extern RelocAddr <HUDShowAmmoCounter> HUDShowAmmoCounter_Original;
extern HUDShowAmmoCounter HUDShowAmmoCounter_Copied;
extern RelocPtr <UInt32> uAmmoCounterFadeTimeMS;

//typedef void* (*EquipHandler_UpdateAnimGraph)(Actor* actor, bool unk_rdx);
extern RelocAddr <_UpdateAnimGraph> EquipHandler_UpdateAnimGraph_HookTarget;
extern _UpdateAnimGraph EquipHandler_UpdateAnimGraph_Copied;

extern RelocAddr <_AttachModToStack> AttachModToStack_CallFromGameplay_HookTarget;
extern RelocAddr <_AttachModToStack> AttachModToStack_CallFromWorkbenchUI_HookTarget;
extern RelocAddr <_DeleteExtraData> DeleteExtraData_CallFromWorkbenchUI_HookTarget;
extern RelocAddr <uintptr_t> LoadBuffer_ExtraDataList_ExtraRank_JumpHookTarget;
extern _AttachModToStack AttachModToStack_CallFromGameplay_Copied;
extern _AttachModToStack AttachModToStack_CallFromWorkbenchUI_Copied;
extern _DeleteExtraData DeleteExtraData_CallFromWorkbenchUI_Copied;
extern uintptr_t LoadBuffer_ExtraDataList_ExtraRank_ReturnJumpAddr;
extern uintptr_t LoadBuffer_ExtraDataList_ExtraRank_BranchCode;

UInt8 PlayerAnimationEvent_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3);
UInt64 HUDShowAmmoCounter_Hook(HUDAmmoCounter* ammoCounter, UInt32 visibleTime);
void* AttackBlockHandler_Hook(void* handler);
void* EquipHandler_UpdateAnimGraph_Hook(Actor* actor, bool unk_rdx);
bool AttachModToStack_CallFromGameplay_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* modFunctor, UInt32 unk_r9d, UInt32* unk_rsp20);
bool AttachModToStack_CallFromWorkbenchUI_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* changesFunctor, UInt32 unk_r9d, UInt32* unk_rsp20);
bool DeleteExtraData_CallFromWorkbenchUI_Hook(BSExtraData** extraDataHead, ExtraDataType type);
ExtraRank* LoadBuffer_ExtraDataList_ExtraRank_Hook(ExtraRank* newExtraRank, UInt32 rank, ExtraDataList* futureParentList, BGSInventoryItem::Stack* futureParentStack);

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
