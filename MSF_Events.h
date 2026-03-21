#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Base.h"
#include "MSF_Addresses.h"
#include "f4se\GameThreads.h"

//Anim Event
class BSAnimationGraphEvent
{
public:
	std::uint64_t holderID;  // 00
	BSFixedString eventName;       // 08
	BSFixedString payload;   // 10
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

DECLARE_EVENT_DISPATCHER(TESCellFullyLoadedEvent, ID(1175067, 2201825).offset());

//CombatEvent
class CombatEvnHandler : public BSTEventSink<TESCombatEvent>
{
public:
	virtual ~CombatEvnHandler() { };
	virtual EventResult ReceiveEvent(TESCombatEvent * evn, void * dispatcher) override;
};
extern CombatEvnHandler combatEvnSink;

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

struct PipboyLightEvent
{
	bool isOn1;
	bool isOn2;
};

class PipboyLightEventSink : public BSTEventSink<PipboyLightEvent>
{
public:
	virtual	EventResult	ReceiveEvent(PipboyLightEvent* evn, void* dispatcher) override;
};
extern PipboyLightEventSink pipboyLightEvent;
//DECLARE_EVENT_DISPATCHER(PipboyLightEvent, ID(317686, ).offset()); //2FD7500

typedef UInt8(*_PlayerAnimationEvent)(void * thissink, BSAnimationGraphEvent* evnstruct, void** dispatcher);
extern RelocAddr <_PlayerAnimationEvent> PlayerAnimationEvent_HookTarget;
extern _PlayerAnimationEvent PlayerAnimationEvent_Original;

//BSTEventSource<InventoryInterface::CountChangedEvent>
//BSTEventSource<ActorEquipManagerEvent::Event>
//GetEventDispatcher<TESEquipEvent>()->AddEventSink(&_TESEquipEventSink);

namespace ActorEquipManagerEvent
{
	struct Event
	{
		bool unequip;
		BGSObjectInstance* data;
		Actor* targetActor;
		UInt32 stackID;
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

class ActorEquipManagerEventSourceMSF : public BSTEventDispatcher<ActorEquipManagerEvent::Event>
{

};
extern ActorEquipManagerEventSourceMSF actorEquipManagerEventSourceMSF;

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
extern RelocAddr <_PlayEquipAction> EquipHandler_PlayEquipAction_HookTarget;
extern _PlayEquipAction EquipHandler_PlayEquipAction_Copied;
extern RelocAddr <_BGSObjectInstanceExtra_ctor> ObjectInstanceCtor_HookTarget;
extern _BGSObjectInstanceExtra_ctor ObjectInstanceCtor_Copied;
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
extern AttackInputHandler AttackInputHandlerSelf_Copied;
extern uintptr_t ActorEquipManagerPre_Copied;
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

extern RelocAddr <uintptr_t> ActorEquipManagerPre_JumpHookTarget; 
extern RelocAddr <uintptr_t> ActorEquipManagerPre_ReturnJumpAddr;
extern RelocAddr <uintptr_t> ActorEquipManagerEnd_ReturnJumpAddr;
extern uintptr_t ActorEquipManagerPre_BranchCode;

extern RelocAddr <uintptr_t> CannotEquipItemGen_JumpHookTarget;
extern RelocAddr <uintptr_t> CannotEquipItemMod_JumpHookTarget;
extern RelocAddr <uintptr_t> CannotEquipItemGen_ReturnJumpAddr;
extern RelocAddr <uintptr_t> CannotEquipItemMod_ReturnJumpAddr;
extern RelocAddr <uintptr_t> CannotEquipItem_SkipJumpAddr;
extern RelocAddr <uintptr_t> CannotEquipItem_TextAddr;
extern uintptr_t CannotEquipItem_BranchCode;

extern RelocAddr <uintptr_t> AModToInvItem_Attach_AV_HookTarget;
extern RelocAddr <uintptr_t> AModToInvItem_Equip_AV_HookTarget;
extern _AttachRemoveModInternal AttachRemoveModInternal_Copied;
extern _EquipItemPapyrus EquipItemPapyrus_Copied;

extern RelocAddr <uintptr_t> ExtraRankConstructor_HookTarget;
extern RelocAddr <_ExtraRankCtor> ExtraRankConstructor_Original;
extern RelocAddr <_ExtraRankDtor> ExtraRankDestructor_Original;
extern _ExtraRankCtor ExtraRankConstructor_Copied;
extern _ExtraRankDtor ExtraRankDestructor_Copied;

extern _MainEquipHandler MainEquipHandler_Copied;
extern uintptr_t PutYourGunInBranchCode;

struct ReloadJumpReplace {
	uint8_t original[2] = { 0x0F, 0x84 };
	uint8_t replacement[2] = { 0x90, 0xE9 };
};
extern ReloadJumpReplace ReloadJumpOverwriteCode;

class WidgetUpdateTask : public ITaskDelegate
{
public:
	WidgetUpdateTask(bool settings = false, bool quickkey = false)
	{
		doSettings = settings;
		clearQuickkey = quickkey;
	};
	virtual void Run() final
	{
		if (doSettings)
			MSF_Scaleform::UpdateWidgetSettings();
		if (clearQuickkey)
			MSF_Scaleform::ClearWidgetQuickkeyMod();
		_DEBUG("widget update thread");
		MSF_Scaleform::UpdateWidgetData();
	}
	bool doSettings;
	bool clearQuickkey;
};

class MSFMenuUpdateTask : public ITaskDelegate
{
public:
	MSFMenuUpdateTask(TESAmmo* changedAmmo, TESObjectMISC* changedmisc)
	{
		ammo = changedAmmo;
		misc = changedmisc;
	};
	virtual void Run() final
	{
		if (ammo)
			MSF_Scaleform::UpdateAmmoMenuCount(ammo, Utilities::GetInventoryItemCount((*g_player)->inventoryList, ammo));
		else
			MSF_Scaleform::UpdateModMenuReqs(misc, Utilities::GetInventoryItemCount((*g_player)->inventoryList, misc));
	}
	TESAmmo* ammo;
	TESObjectMISC* misc;

	static void StartUpdate(TESBoundObject* changeditem)
	{
		if (!changeditem)
			return;
		if (changeditem->formType == FormType::kFormType_AMMO)
		{
			TESAmmo* ammo = DYNAMIC_CAST(changeditem, TESBoundObject, TESAmmo);
			if (ammo)
			{
				MSF_Scaleform::UpdateAmmoMenuCount(ammo, Utilities::GetInventoryItemCount((*g_player)->inventoryList, ammo));
				//MSFMenuUpdateTask* updTask = new MSFMenuUpdateTask(ammo, nullptr);
				//g_threading->AddUITask(updTask);
				return;
			}
		}
		else if (changeditem->formType == FormType::kFormType_MISC)
		{
			TESObjectMISC* misc = DYNAMIC_CAST(changeditem, TESBoundObject, TESObjectMISC);
			if (misc)
			{
				MSF_Scaleform::UpdateModMenuReqs(misc, Utilities::GetInventoryItemCount((*g_player)->inventoryList, misc));
				//MSFMenuUpdateTask* updTask = new MSFMenuUpdateTask(nullptr, misc);
				//g_threading->AddUITask(updTask);
			}
		}
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

class AnimTask : public ITaskDelegate
{
public:
	AnimTask(AnimationData* animation)
	{
		animData = animation;
	};
	virtual void Run() final
	{
		MSF_Base::PlayAnim(animData);
	}
private:
	AnimationData* animData;
};

class FireGunTask : public ITaskDelegate
{
public:
	virtual void Run() final
	{
		Actor* playerActor = *g_player;
		auto instance = Utilities::GetEquippedInstanceData(playerActor);
		if (!instance)
			return;
		if (instance->flags & TESObjectWEAP::InstanceData::kFlag_Automatic)
			Utilities::PlayIdleAction(playerActor, MSF_MainData::ActionFireAuto);
		else
			Utilities::PlayIdleAction(playerActor, MSF_MainData::ActionFireSingle);
	}
};

class LowerWeaponTask : public ITaskDelegate
{
public:
	LowerWeaponTask() : doDown(true), force(false)
	{};
	virtual void Run() final
	{
		_DEBUG("Lower Task");
		PlayerCharacter* playerActor = *g_player;
		if (playerActor->IsInCombat() && !force)
		{
			LowerWeaponTask* lowerTask = new LowerWeaponTask();
			MSF_MainData::modSwitchManager.lowerGunTimer.start(MSF_MainData::iAutolowerTimeMS, g_threading->AddTask, lowerTask);
			return;
		}
		if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bDontAutolowerWeaponWithFlashlightOn) && playerActor->unkB68[17] != 0)
			return;
		UInt32 weaponActivity = playerActor->actorState.flags & ActorStateFlags0C::mWeaponActivityMask;
		//if (!(playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Drawn) || (playerActor->actorState.flags & (ActorStateFlags0C::kWeaponState_Draw | ActorStateFlags0C::kWeaponState_Sheathing | \
		//	ActorStateFlags0C::kActorState_FurnitureState)) || (weaponActivity == ActorStateFlags0C::kWeaponState_Reloading || weaponActivity == ActorStateFlags0C::kWeaponState_Firing) || \
		//	(playerActor->actorState.unk08 & (ActorStateFlags08::kActorState_Sprint | ActorStateFlags08::kActorState_Bashing)))
		if ((playerActor->actorState.unk08 & (ActorStateFlags08::kActorState_Bashing)) || // | ActorStateFlags08::kActorState_Sprint
				(playerActor->actorState.flags & (ActorStateFlags0C::kActorState_FurnitureState | ActorStateFlags0C::kWeaponState_Sheathing)) ||
				(weaponActivity == ActorStateFlags0C::kWeaponState_Reloading || weaponActivity == ActorStateFlags0C::kWeaponState_Firing) ||
				(!(playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Drawn) && (playerActor->actorState.flags & ActorStateFlags0C::kWeaponState_Draw)))
			return;
		bool isDown = (playerActor->actorState.flags & (ActorStateFlags0C::kWeaponState_Lowered1stP | ActorStateFlags0C::kWeaponState_Lowered3rdP));
		_DEBUG("Lower Check OK, isDown: %i, doDown: %i", isDown, doDown);
		if (isDown ^ doDown)
			Utilities::PlayIdleAction(playerActor, MSF_MainData::ActionGunDown);
	}
private:
	bool doDown;
	bool force;
};

class QuickkeySelectTask : public ITaskDelegate
{
public:
	virtual void Run() final
	{
		MSF_MainData::modSwitchManager.HandleQuickkeyTimeout();
	}
};

UInt8 PlayerAnimationEvent_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3);
UInt64 HUDShowAmmoCounter_Hook(HUDAmmoCounter* ammoCounter, UInt32 visibleTime);
void* AttackBlockHandler_Hook(void* handler);
bool AttackInputHandler_Hook(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d);
bool AttackInputHandlerReload_Hook(void* PlayerInputHandler, UInt32 inputCode, UInt32 r8d); 
void* EquipHandler_UpdateAnimGraph_Hook(Actor* actor, bool unk_rdx);
void EquipHandler_PlayEquipAction_Hook(Actor* actor, bool unk_rdx);
void AttachRemoveModInternal_Hook(Actor* actor, TESBoundObject* baseItem, CheckStackIDFunctor* CheckStackIDFunctor, StackDataWriteFunctor* ModifyModDataFunctor, void* arg_1, void* arg_2, void* arg_3, void* arg_4, void* arg_5, void* arg_6, void* arg_7);
void EquipItemPapyrus_Hook(Actor* actor, TESBoundObject* baseItem, UInt32 r8d);
bool AttachModToStack_CallFromGameplay_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* modFunctor, UInt32 unk_r9d, UInt32* unk_rsp20);
bool AttachModToStack_CallFromWorkbenchUI_Hook(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* changesFunctor, UInt32 unk_r9d, UInt32* unk_rsp20);
bool DeleteExtraData_CallFromWorkbenchUI_Hook(BSExtraData** extraDataHead, ExtraDataType type);
bool UpdMidProc_Hook(Actor::AIProcess* midProc, Actor* actor, BGSObjectInstance weaponBaseStruct, BGSEquipSlot* equipSlot);
void UpdateEquipData_Hook(BipedAnim* equipData, BGSObjectInstance instance, UInt32* r8d);
void UpdateEquippedWeaponData_Hook(EquippedWeaponData* data);
BGSObjectInstanceExtra* ObjectInstanceCtor_Hook(BGSObjectInstanceExtra* allocatedHeap, BGSMod::Template::Item* templateItem, TESBoundObject* parentForm, void* instanceFilter);
ExtraRank* LoadBuffer_ExtraDataList_ExtraRank_Hook(ExtraRank* newExtraRank, UInt32 rank, ExtraDataList* futureParentList, BGSInventoryItem::Stack* futureParentStack);
void ExtraRankConstructor_Hook(ExtraDataList* parentList, UInt32 rank);
ExtraRank* ExtraRankDestructor_Hook(ExtraRank* extra, bool cast);
bool ExtraRankCompare_Hook(ExtraRank* extra1, ExtraRank* extra2);
bool CheckAmmoCountForReload_Hook(Actor* target, UInt32 loadedAmmo, UInt32 ammoCap, UInt32 ammoReserve);
const char* CannotEquipItem_Hook(TESObjectREFR* target, TESForm* item, UInt32 unequip, UInt32 type);
void ActorEquipManagerPre_Hook(Actor* owner, BGSObjectInstance* object, UInt32 eqStackID);

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
#define REGISTER_EVENT(_event, _sink) GetEventDispatcher<_event>()->AddEventSink(&_sink);
