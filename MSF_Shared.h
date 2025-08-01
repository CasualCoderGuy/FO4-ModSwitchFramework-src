#pragma once
#include "Config.h"
#include "f4se/GameData.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameFormComponents.h"
#include "f4se/GameObjects.h"
#include "f4se/GameReferences.h"
#include "f4se/ScaleformMovie.h"
#include "f4se/GameMenus.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusUtilities.h"
#include "f4se/PapyrusEvents.h"
#include "f4se/GameCamera.h"
#include "rva/RVA.h"
//#include "decomp\BSPointerHandle.h"
#include "RNG.h"
#include  <thread>
#include  <chrono>
#include  <sstream>
#include <algorithm>


extern IDebugLog gLog;
extern PluginHandle g_pluginHandle;
extern F4SEPapyrusInterface* g_papyrus;
extern F4SEMessagingInterface* g_messaging;
extern F4SEScaleformInterface* g_scaleform;
extern F4SESerializationInterface* g_serialization;
extern F4SETaskInterface* g_threading;

UInt32 roundp(float a);
UInt32 inttover(UInt32 input);
BGSObjectInstanceExtra* CreateObjectInstanceExtra(BGSObjectInstanceExtra::Data* data);
ExtraUniqueID* CreateExtraUniqueID(UInt16 id, UInt32 form);

typedef unsigned short KeywordValue;
typedef UInt32 ObjectRefHandle;

namespace InventoryInterface
{
	struct CountChangedEvent
	{
	public:
		// members
		std::uint32_t inventoryOwnerID;  // 00
		std::uint32_t itemID;            // 04
		std::int32_t newCount;           // 08
		std::int32_t oldCount;           // 0C
	};
	STATIC_ASSERT(sizeof(CountChangedEvent) == 0x10);

	struct FavoriteChangedEvent
	{
	public:
		~FavoriteChangedEvent() noexcept {}  // intentional

		// members
		BGSInventoryItem* itemAffected;  // 0
	};
	STATIC_ASSERT(sizeof(FavoriteChangedEvent) == 0x8);
}

class BGSInventoryInterface
{
public:
	struct Agent
	{
	public:
		// members
		std::uint32_t handleID;     // 0
		ObjectRefHandle itemOwner;  // 4
		std::uint16_t listIndex;    // 8
		std::uint16_t refCount;     // A
	};
	STATIC_ASSERT(sizeof(Agent) == 0xC);

	UInt64 pad;
	BSTEventDispatcher<InventoryInterface::CountChangedEvent> countChangedEventSource;    // 08
	BSTEventDispatcher<InventoryInterface::FavoriteChangedEvent> favChangedEventSource;   // 60
	tArray<Agent> agentArray;  // B8
};
STATIC_ASSERT(sizeof(BGSInventoryInterface) == 0xD0);

class NiFormArray;
class TESConditionItem;

class TESCondition
{
public:
	TESConditionItem* head;
};

struct IDLE_DATA
{
public:
	// members
	std::int8_t loopMin;        // 0
	std::int8_t loopMax;        // 1
	std::int8_t flags;          // 2
	std::uint16_t replayDelay;  // 4
};
STATIC_ASSERT(sizeof(IDLE_DATA) == 0x6);

class TESIdleForm : public TESForm
{
public:
	enum { kTypeID = kFormType_IDLE };


	// members
	TESCondition conditions;          // 20
	IDLE_DATA data;                   // 28
	NiFormArray* childIdles;          // 30
	TESIdleForm* parentIdle;          // 38
	TESIdleForm* prevIdle;            // 40
	BSFixedString behaviorGraphName;  // 48
	BSFixedString animEventName;      // 50
	BSFixedString animFileName;       // 58
	BSString formEditorID;     // 60

};
STATIC_ASSERT(sizeof(TESIdleForm) == 0x70);

class TESImageSpaceModifiableForm :
	public BaseFormComponent  // 00
{
public:
	// members
	TESImageSpaceModifier* formImageSpaceModifying;  // 08
};
STATIC_ASSERT(sizeof(TESImageSpaceModifiableForm) == 0x10);

struct BGSExplosionData
{
public:
	// members
	TESObjectLIGH* light;                                                // 00
	BGSSoundDescriptorForm* sound1;                                      // 08
	BGSSoundDescriptorForm* sound2;                                      // 10
	BGSImpactDataSet* impactDataSet;                                     // 18
	TESBoundObject* impactPlacedObject;                                  // 20
	BGSProjectile* spawnProjectile;                                      // 28
	NiPoint3 projectileVector;                                           // 30
	float projectileSpread;                                              // 3C
	std::uint32_t projectileCount;                                       // 40
	float force;                                                         // 44
	float damage;                                                        // 48
	float innerRadius;                                                   // 4C
	float outerRadius;                                                   // 50
	float imageSpaceRadius;                                              // 54
	float verticalOffsetMult;                                            // 58
	UInt32 flags;                                                 // 5C
	UInt32 soundLevel;              // 60
	float placedObjectFadeDelay;                                         // 64
	UInt32 staggerMagnitude;  // 68
};
STATIC_ASSERT(sizeof(BGSExplosionData) == 0x70);

class BGSExplosion : 
	public TESBoundObject,              // 000
	public TESFullName,                 // 068
	public TESModel,                    // 078
	public TESEnchantableForm,          // 0A8
	public BGSPreloadable,              // 0C0
	public TESImageSpaceModifiableForm  // 0C8
{
public:
	// members
	BGSExplosionData data;  // 0D8
};
STATIC_ASSERT(sizeof(BGSExplosion) == 0x148);

class MSFAimModel : public TESForm //https://github.com/isathar/F4SE_AmmoTweaksExtension cast AimModel as MSFAimModel, edit result
{
public:
	enum { kTypeID = kFormType_AMDL };

	// CoF_ = spread/cone of fire, Rec_ = recoil:
	float			CoF_MinAngle;			//20 - min. spread angle (crosshair size)
	float			CoF_MaxAngle;			//24 - max. spread angle
	float			CoF_IncrPerShot;		//28 - spread increase per shot
	float			CoF_DecrPerSec;			//2C - spread decrease per second (after delay)
	UInt32			CoF_DecrDelayMS;		//30 - delay in ms before spread starts to decrease after firing
	float			CoF_SneakMult;			//34 - multiplier applied to spread while sneaking/crouched
	float			Rec_DimSpringForce;		//38 - amount of automatic aim correction after recoil
	float			Rec_DimSightsMult;		//3C - amount of automatic aim correction after recoil while aiming
	float			Rec_MaxPerShot;			//40 - max. amount of recoil per shot
	float			Rec_MinPerShot;			//44 - min. amount of recoil per shot
	float			Rec_HipMult;			//48 - multiplier applied to recoil while firing from the hip
	UInt32			Rec_RunawayShots;		//4C - the number of shots before recoil becomes unbearable?
	float			Rec_ArcMaxDegrees;		//50 - max. difference from the base recoil angle per shot in degrees
	float			Rec_ArcRotate;			//54 - angle for the recoil direction (clock-wise from 12:00)
	float			CoF_IronSightsMult;		//58 - multiplier applied to spread while aiming without a scope
	float			BaseStability;			//5C - multiplier applied to the amount of camera movement while using a scope
};

class MSFZoomData : public TESForm
{
public:
	enum { kTypeID = kFormType_ZOOM };

	float			FOVmult;			//20
	UInt32			overlay;			//24
	UInt32			imageSpaceFormID;	//28
	float			offsetX;			//2C
	float			offsetY;			//30
	float			offsetZ;			//34
	TESImageSpaceModifier*	imageSpace;	//38
};
STATIC_ASSERT(sizeof(MSFZoomData) == 0x40);

class BGSSoundKeywordMapping : public TESForm//, BSISoundDescriptor
{
public:
	enum { kTypeID = kFormType_KSSM };

	UInt64		unk20;		//20
	UInt32		unk28;		//28
	UInt32		unk2C;		//2C
	UInt32		unk30;		//30
	UInt32		unk34;		//34
	void*		unk38;		//38 to vtbl?
	UInt64		unk40;		//40
	void*		unk48;		//48 to keywords?
	UInt64		unk50[3];	//50
	void*		unk68;		//68 to vtbl?
	UInt64		unk70;		//70
	UInt64		unk78;		//78
	BGSSoundDescriptorForm*	primarySound;	//80
	BGSSoundDescriptorForm*	exteriorTail;	//88
	BGSSoundDescriptorForm*	VATS_Sound;		//90
	float		VATS_threshold;		//98
};
STATIC_ASSERT(offsetof(BGSSoundKeywordMapping, primarySound) == 0x80);

class EquipWeaponData : public EquippedItemData
{
public:
	virtual ~EquipWeaponData();

	TESAmmo* ammo;                                                                               // 10
	volatile int loadedAmmoCount;                                                                     // 18
	MSFAimModel* aimModel;                                                                          // 20
	void* muzzleFlash;                                                                    // 28
	NiAVObject* fireNode;                                                                        // 30
	UInt64 attackState;                                                               // 38
	void* fireLocations[3];  // 40
	void* weaponPreload;                                                         // 58
	void* projectilePreload;                                                     // 60
	void* reserveProjectileClones;                                          // 68
	void* idleSound;                                                                     // 70
	void* attackSound;                                                                   // 78
	void* reverbSound;                                                                   // 80
	void* prevAttack;                                                                    // 88
	void* prevReverb;                                                                    // 90
	BGSSoundKeywordMapping* attackSoundData;                                               // 98
	bool reverbSoundIsTail;
	//TESAmmo*	ammo;					// 10
	//UInt64		loadedAmmoCount;		// 18
	//void*		unk20;					// 20
	//UInt64		unk28;					// 28
	//NiAVObject*	object;					// 30
	//UInt64		unk38[4];				// 38
	//void*		unk58;					// 58 QueuedFile
	//void*		unk60;					// 60 QueuedFile
	//void*		unk68;					// 68 BSCloneReserver
	//UInt64		unk70[5];				// 70
	//BGSSoundKeywordMapping* firingSound;// 98
	//BGSKeyword*	unkKeyword;				// A0
};
STATIC_ASSERT(offsetof(EquipWeaponData, attackSoundData) == 0x98);

class CheckStackIDFunctor
{
private:
	void* vtbl;		//00
public:
	CheckStackIDFunctor(UInt32 ID);
	UInt32 stackID;	//08
};

class StackDataWriteFunctor
{
protected:
	void* vtbl;								// 00
public:
	bool shouldSplitStacks{ true };              // 08
	bool transferEquippedToSplitStack{ false };  // 09
};
STATIC_ASSERT(sizeof(StackDataWriteFunctor) == 0x10);

class ModifyModDataFunctor : public StackDataWriteFunctor
{
public:
	ModifyModDataFunctor(BGSMod::Attachment::Mod* mod, UInt8 slotIndex, bool bAttach, bool* success);

	BGSMod::Attachment::Mod* mod;			// 10
	TESBoundObject* foundObject{ nullptr };	// 18 not needed to set in advance
	bool* success;							// 20 set to 1, return value
	const UInt8 slotIndex;					// 28
	const bool attach;						// 29
	bool equipLocked{ false };				// 2A
};
STATIC_ASSERT(sizeof(ModifyModDataFunctor) == 0x30);
STATIC_ASSERT(offsetof(ModifyModDataFunctor, mod) == 0x10);

class SplitStackFunctor
{
public:
	SplitStackFunctor(bool transferEquipped, UInt32 newCount, BGSInventoryItem::Stack* oldStack);
	virtual bool Apply(TESBoundObject* item, BGSInventoryItem::Stack* newStack);

	bool shouldSplitStacks{ true };              // 08
	bool transferEquippedToSplitStack{ false };  // 09
	bool preventUnequip{ false };
	UInt32 stackCount;
	BGSInventoryItem::Stack* stack;
};

class ApplyChangesFunctor : public StackDataWriteFunctor
{
public:
	ApplyChangesFunctor(TESBoundObject* foundObject, BGSObjectInstanceExtra* moddata, BGSMod::Attachment::Mod* mod, bool ignoreWeapon, bool remove, bool equipLocked, UInt8 setExtraData);

	BGSObjectInstanceExtra* moddata;		// 10
	TESBoundObject* foundObject;			// 18
	BGSMod::Attachment::Mod* mod;			// 20
	bool ignoreWeapon;						// 28 true
	bool remove;							// 29 
	bool equipLocked;						// 2A false
	UInt8 setExtraData;						// 2B FE (ignored when FE)

};
STATIC_ASSERT(sizeof(ApplyChangesFunctor) == 0x30);
STATIC_ASSERT(offsetof(ApplyChangesFunctor, remove) == 0x29);

class BGSEquipIndex
{
public:
	std::uint32_t index;  // 0
};
STATIC_ASSERT(sizeof(BGSEquipIndex) == 0x4);

class ExtraEnchantment : public BSExtraData
{
public:
	ExtraEnchantment();
	virtual ~ExtraEnchantment();

	EnchantmentItem* enchant;		// 08
	UInt16				maxCharge;		// 0C
	UInt8				unk0E;			// 0E - usually 0
	UInt8				pad0F;			// 0F

	static ExtraEnchantment* Create();
};

class ExtraModRank : public BSExtraData
{
public:
	UInt32				rank;		// 18

	static ExtraModRank* Create(UInt32 modrank);
};

class ExtraRank : public BSExtraData
{
public:
	UInt32				rank;		// 18

	static ExtraRank* Create(UInt32 rank);
};

class ExtraAmmo : public BSExtraData
{
public:
	UInt32				ammo;		// 18

	static ExtraAmmo* Create(UInt32 ammo);
};

struct unkTBOStruct
{
	TESBoundObject* baseForm;
	UInt64 unk08;
};

struct ActorStruct
{
	Actor* actor;
	UInt8* unk08;
};

struct unkItemStruct
{
	TESForm* item;
	UInt16 unk08;
};

struct unkEquipSlotStruct
{
	UInt64 unk00; //=1 or 0
	BGSEquipSlot* equipSlot;
	UInt64 unk10; //=0
	UInt64 unk18; //=0x10001 or 0x10000
};

class RemoveItemData
{
public:

};

class RemoveItemData2
{
public:

};

template <typename T> class TypedKeywordValueArray
{
public:
	T* entries;
	UInt32 count;
	//T& operator[](int idx) { return *(ptr + idx); }

	TypedKeywordValueArray() : entries(NULL), count(0) { }

	T& operator[](UInt64 index)
	{
		return entries[index];
	}

	bool GetNthItem(UInt64 index, T& pT) const
	{
		if (index < count) {
			pT = entries[index];
			return true;
		}
		return false;
	}

	SInt64 GetItemIndex(T & pFind) const
	{
		for (UInt64 n = 0; n < count; n++) {
			T& pT = entries[n];
			if (pT == pFind)
				return n;
		}
		return -1;
	}

	void Clear()
	{
		Heap_Free(entries);
		entries = NULL;
		count = 0;
	}

	bool Push(const T & entry)
	{
		UInt32 idx = count;
		if (!Grow())
			return false;
		entries[idx] = entry;
		return true;
	};

	bool Insert(UInt32 index, const T & entry)
	{
		if (!entries)
			return false;

		UInt32 lastSize = count;

		if (!Grow())
			return false;

		if (index != lastSize)  // Not inserting onto the end, need to move everything down
		{
			UInt32 remaining = count - index;
			memmove_s(&entries[index + 1], sizeof(T) * remaining, &entries[index], sizeof(T) * remaining); // Move the rest up
		}

		entries[index] = entry;
		return true;
	};

	bool Remove(UInt32 index)
	{
		if (!entries || index >= count)
			return false;

		// This might not be right for pointer types...
		(&entries[index])->~T();

		if (index + 1 < count) {
			UInt32 remaining = count - index;
			memmove_s(&entries[index], sizeof(T) * remaining, &entries[index + 1], sizeof(T) * remaining); // Move the rest up
		}
		count--;
		if (count == 0)
			Clear();
		else
			Shrink();

		return true;
	}

private:
	bool Shrink()
	{
		if (!entries) return false;

		try {
			UInt32 newSize = count;
			T * oldArray = entries;
			T * newArray = (T *)Heap_Allocate(sizeof(T) * newSize); // Allocate new block
			memmove_s(newArray, sizeof(T) * newSize, entries, sizeof(T) * newSize); // Move the old block
			entries = newArray;
			Heap_Free(oldArray); // Free the old block
			return true;
		}
		catch (...) {
			return false;
		}

		return false;
	}

	bool Grow()
	{
		if (!entries) {
			entries = (T *)Heap_Allocate(sizeof(T));
			count = 1;
			return true;
		}

		try {
			UInt32 oldSize = count;
			UInt32 newSize = oldSize + 1;
			T * oldArray = entries;
			T * newArray = (T *)Heap_Allocate(sizeof(T) * newSize); // Allocate new block
			if (oldArray)
				memmove_s(newArray, sizeof(T) * newSize, entries, sizeof(T) * oldSize); // Move the old block
			entries = newArray;
			count = newSize;

			if (oldArray)
				Heap_Free(oldArray); // Free the old block

			for (UInt32 i = oldSize; i < newSize; i++) // Allocate the rest of the free blocks
				new (&entries[i]) T;

			return true;
		}
		catch (...) {
			return false;
		}

		return false;
	}

	DEFINE_STATIC_HEAP(Heap_Allocate, Heap_Free)
};

typedef TypedKeywordValueArray<KeywordValue> KeywordValueArray;

class AttachParentArray : public BaseFormComponent
{
public:
	KeywordValueArray	kewordValueArray;
	enum
	{
		iDataType = 2
	};
};

namespace Utilities
{
	TESForm* GetFormFromIdentifier(const std::string& identifier);
	const char* GetIdentifierFromForm(TESForm* form);
	bool AddToFormList(BGSListForm* flst, TESForm* form, SInt64 idx);
	UInt32 GetLoadedAmmoCount(Actor* owner);
	UInt32 GetEquippedItemFormID(Actor * ownerActor, UInt32 iEquipSlot = 41);
	TESObjectWEAP::InstanceData * GetEquippedInstanceData(Actor * ownerActor, UInt32 iEquipSlot = 41);
	BGSObjectInstanceExtra* GetEquippedModData(Actor * ownerActor, UInt32 iEquipSlot = 41);
	BGSObjectInstanceExtra* GetEquippedWeaponModData(Actor* ownerActor);
	BGSInventoryItem::Stack* GetEquippedStack(Actor* owner, UInt32 slotIndex);
	BGSInventoryItem::Stack* GetEquippedWeaponStack(Actor* owner);
	TESObjectWEAP* GetEquippedGun(Actor* ownerActor);
	TESObjectWEAP* GetEquippedWeapon(Actor* ownerActor);
	UInt8 GetEquippedWeaponSlotIndex(Actor* ownerActor);
	UInt32 GetStackID(BGSInventoryItem* item, BGSInventoryItem::Stack* stack);
	BGSInventoryItem::Stack* GetStack(BGSInventoryItem* item, UInt32 stackID);
	BGSInventoryItem::Stack* GetStackFromItem(TESObjectREFR* owner, TESForm* item, UInt32 stackID);
	UInt64 GetInventoryItemCount(BGSInventoryList* inventory, TESForm* item);
	EquipWeaponData* GetEquippedWeaponData(Actor* owner);
	TESObjectMISC* GetLooseMod(BGSMod::Attachment::Mod* thisMod);
	BGSMod::Attachment::Mod* FindModByUniqueKeyword(BGSObjectInstanceExtra* modData, BGSKeyword* keyword);
	std::vector<BGSMod::Attachment::Mod*> FindModsByUniqueKeyword(BGSObjectInstanceExtra* modData, BGSKeyword* keyword);
	BGSMod::Attachment::Mod* GetFirstModWithPriority(BGSObjectInstanceExtra* modData, UInt8 priority);
	bool HasObjectMod(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod);
	BGSKeyword* GetAttachParent(BGSMod::Attachment::Mod* mod);
	bool GetParentMods(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod, std::vector<BGSMod::Attachment::Mod*>* parents);
	BGSMod::Attachment::Mod* GetParentMod(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod);
	KeywordValue GetAttachValueForTypedKeyword(BGSKeyword* keyword);
	KeywordValue GetInstantiationValueForTypedKeyword(BGSKeyword* keyword);
	KeywordValue GetAnimFlavorValueForTypedKeyword(BGSKeyword* keyword);
	bool HasAttachPoint(AttachParentArray* attachPoints, BGSKeyword* attachPointKW);
	bool ObjectInstanceHasAttachPoint(BGSObjectInstanceExtra* modData, BGSKeyword* attachPointKW);
	BGSMod::Attachment::Mod* GetModAtAttachPoint(BGSObjectInstanceExtra* modData, KeywordValue keywordValue);
	bool GetParentInstantiationValues(BGSObjectInstanceExtra* modData, KeywordValue parentValue, std::vector<KeywordValue>* instantiationValues);
	bool AddAttachPoint(AttachParentArray* attachPoints, BGSKeyword* attachPointKW);
	bool AddAttachValue(AttachParentArray* attachPoints, KeywordValue attachValue);
	bool WeaponInstanceHasKeyword(TESObjectWEAP::InstanceData* instanceData, BGSKeyword* checkKW);
	bool HasKeyword(BGSKeywordForm* keywordForm, BGSKeyword* checkKW);
	bool UpdateAimModel(MSFAimModel* oldModel, MSFAimModel* newModel);
	bool UpdateZoomData(MSFZoomData* oldData, MSFZoomData* newData);
	bool PlayIdle(Actor* actor, TESIdleForm* idle);
	bool PlayIdleAction(Actor* actor, BGSAction* action);
	void DrawWeapon(Actor* actor);
	void FireWeapon(Actor* actor, UInt32 shots);
	void ReloadWeapon(Actor* actor);
	void SetAnimationVariableBool(TESObjectREFR* ref, BSFixedString asVariableName, bool newVal);
	void SetAnimationVariableInt(TESObjectREFR* ref, BSFixedString asVariableName, SInt32 newVal);
	void SetAnimationVariableFloat(TESObjectREFR* ref, BSFixedString asVariableName, float newVal);
	bool GetAnimationVariableBool(TESObjectREFR* ref, BSFixedString asVariableName);
	SInt32 GetAnimationVariableInt(TESObjectREFR* ref, BSFixedString asVariableName);
	float GetAnimationVariableFloat(TESObjectREFR* ref, BSFixedString asVariableName);
	void SendNotification(std::string asNotificationText);
	//void ShowMessagebox(std::string asText);
	void AddItem(TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent);
	void RemoveItem(TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent, TESObjectREFR* toContainer = nullptr);
	void AttachModToInventoryItem(TESObjectREFR* objRef, TESForm* invItem, BGSMod::Attachment::Mod* mod);
	bool ModActorValue(Actor* actor, ActorValueInfo* av, float val);
	bool AddRemActorValue(Actor* actor, ActorValueInfo* av, bool bAdd);
	bool AddRemKeyword(BGSKeywordForm* keywordForm, BGSKeyword* keyword, bool bAdd);
	UInt32 AddRemFlag(UInt32 flagHolder, UInt32 flag, UInt8 bAdd, UInt8 op = 0);
	float GetActorValue(tArray<Actor::ActorValueData>* avdata, UInt32 formId);


	class Timer
	{
	public:
		void start()
		{
			countStart = std::chrono::steady_clock::now();
			_IsRunning = true;
		}

		long long int getElapsed()
		{
			if (!_IsRunning)
				return 0;
			countEnd = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(countEnd - countStart).count();
		}

		long long int stop()
		{
			if (!_IsRunning)
				return 0;
			_IsRunning = false;
			countEnd = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(countEnd - countStart).count();
		}

		long long int getLast()
		{
			if (_IsRunning)
				return 0;
			return std::chrono::duration_cast<std::chrono::milliseconds>(countEnd - countStart).count();
		}

		bool IsRunning()
		{
			return _IsRunning;
		}

	private:
		std::chrono::steady_clock::time_point countStart, countEnd;
		bool _IsRunning;
	};
}

class TlsShare
{
private:
	std::vector<std::pair<DWORD, void*>> TlsValues;
public:
	TlsShare();
	void CopyTls();
};

class delayTask
{
public:
	template <class callable, class... arguments>
	delayTask(int delay, bool async, callable&& f, arguments&&... args)
	{
		std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

		if (async)
		{
			TlsShare* tlsShare = new TlsShare;
			std::thread([delay, task, tlsShare]() {
				tlsShare->CopyTls();
				std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				task();
				delete tlsShare;
			}).detach();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			task();
		}
	}

};

typedef void(*_AttachModToInventoryItem)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* objRef, TESForm* invItem, BGSMod::Attachment::Mod* mod, bool unkbool);
typedef void(*_AttachMod)(Actor* actor, TESBoundObject* baseItem, CheckStackIDFunctor* CheckStackIDFunctor, StackDataWriteFunctor* ModifyModDataFunctor, UInt8 arg_unk28, void** weapbaseMf0, UInt8 unk_FFor0, BGSMod::Attachment::Mod* mod);
typedef bool(*_AttachModToStack)(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, StackDataWriteFunctor* modFuntor, UInt32 unk_r9d, UInt32* unk_rsp20); //, UInt32 unk_rsp50
typedef bool(*_ModifyStackData)(BGSInventoryItem* invItem, BGSInventoryItem::Stack** stack, StackDataWriteFunctor* modFuntor);
typedef bool(*_UpdMidProc)(Actor::AIProcess* midProc, Actor* actor, BGSObjectInstance weaponBaseStruct, BGSEquipSlot* equipSlot);
typedef void(*_UpdateEquipData)(BipedAnim* equipData, BGSObjectInstance instance, UInt32* r8d);
typedef void*(*_UpdateAnimGraph)(Actor* actor, bool rdx);
typedef void(*_UpdateEnchantments)(Actor* actor, BGSObjectInstance BGSObjectInstance, ExtraDataList* extraDataList);
typedef void(*_UpdateAVModifiers)(ActorStruct actorStruct, tArray<TBO_InstanceData::ValueModifier>* valueModifiers);
typedef void(*_UpdateAnimValueFloat)(IAnimationGraphManagerHolder* animManager, void* dataHolder, float newValue);
typedef bool(*_DeleteExtraData)(BSExtraData** extraDataHead, ExtraDataType type);

typedef void(*_UpdateEquippedWeaponData)(EquippedWeaponData* data, UInt32 edx);
typedef bool(*_MainEquipHandler)(void* unkmanager, Actor* actor, BGSObjectInstance weaponBaseStruct, unkEquipSlotStruct equipSlotStruct);
typedef bool(*_EquipHandler)(void* unkmanager, Actor* actor, BGSObjectInstance weaponBaseStruct, unkEquipSlotStruct equipSlotStruct);
typedef void(*_UniversalEquipHandler)(Actor* actor, BGSObjectInstance weaponBaseStruct, unkEquipSlotStruct equipSlotStruct);
typedef void(*_NiStuff)(PlayerCharacter* player, TESObjectWEAP* weapBase, ExtraDataList** extraDataList, UInt32 r9d, bool rbp20, UInt32 rbp28);
typedef void(*_UnkSub_EFF9D0)(Actor* actor);
typedef void(*_UnkSub_DFE930)(Actor* actor, bool rdx);
// //virtual void AttachWeapon(const BGSObjectInstanceT<TESObjectWEAP>& a_weapon, BGSEquipIndex a_equipIndex);																							// A5
//virtual void DoReparentWeapon(const TESObjectWEAP* a_weapon, BGSEquipIndex a_equipIndex, bool a_weaponDrawn);                                                                                    // 118

typedef void(*_SetDelete)(TESForm* form, bool a_deleted);
typedef TESForm* (*_CreateDuplicateForm)(TESForm* original, bool a_createEditorID, tHashSet<TESForm*, TESForm*>* a_copyMap);
typedef BGSKeyword*(*_GetKeywordFromValueArray)(UInt32 valueArrayBase, KeywordValue value);
typedef bool(*_HasPerkInternal)(Actor* actor, BGSPerk* perk);
typedef bool(*_IKeywordFormBase_HasKeyword)(IKeywordFormBase* keywordFormBase, BGSKeyword* keyword, UInt32 unk3); //https://github.com/shavkacagarikia/ExtraItemInfo
typedef void(*_AddItem_Native)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* target, unkItemStruct itemStruct, SInt32 count, bool bSilent);
typedef void(*_RemoveItem_Native)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* target, unkItemStruct itemStruct, SInt32 count, bool bSilent, TESObjectREFR* toContainer);
typedef ObjectRefHandle(*_RemoveItem_Virtual)(TESObjectREFR* ref, RemoveItemData& a_data, RemoveItemData2& a_data2);    // 6D, ObjREF
typedef void(*_SetAnimationVariableBool)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName, bool newVal);
typedef void(*_SetAnimationVariableInt)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName, SInt32 newVal);
typedef void(*_SetAnimationVariableFloat)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName, float newVal);
typedef bool(*_GetAnimationVariableBool)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName);
typedef SInt32(*_GetAnimationVariableInt)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName);
typedef float(*_GetAnimationVariableFloat)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName);
typedef bool(*_PlayIdle)(VirtualMachine* vm, UInt32 stackId, Actor* actor, TESIdleForm* idle);
typedef bool(*_PlayIdle2)(Actor* actor, TESIdleForm* idle, UInt64 unk, VirtualMachine* vm, UInt32 stackId);
typedef bool(*_PlayIdleAction)(Actor* actor, BGSAction* action, TESObjectREFR* target, VirtualMachine* vm, UInt32 stackId);
typedef void(*_PlaySubgraphAnimation)(VirtualMachine* vm, UInt32 stackId, Actor* target, BSFixedString asEventName);
typedef bool(*_InitializeActorInstant)(Actor* actor, UInt32 edx);
typedef void(*_UpdateAnimation)(PlayerCharacter* player, float delta);
typedef void (*_PlayEquipSound)(Actor* target, TESBoundObject* a_boundObj, bool a_pickUp, bool a_use);
typedef bool(*_FireWeaponInternal)(Actor* actor);
typedef void(*_ChangeAnimArchetype)(Actor* target, BGSKeyword* archetypeKW);
typedef void(*_ChangeAnimFlavor)(Actor* target, BGSKeyword* flavorKW);
typedef void(*_CheckKeywordType)(BGSKeyword* keyword, UInt32 type); //7: AnimArchetype; 13: AnimFlavor
typedef bool(*_IsInIronSights)(VirtualMachine* vm, Actor* actor);
typedef bool(*_IsInPowerArmor)(Actor* actor);
typedef void(*_DrawWeapon)(VirtualMachine* vm, UInt32 stackId, Actor* actor);
typedef bool(*_FireWeaponInternal)(Actor* actor);
typedef bool(*_ReloadWeapon)(Actor* actor, const BGSObjectInstance& a_weapon, UInt32 a_equipIndex);                                                                                        // 0EF E9BE00
typedef UInt32(*_UseAmmo)(Actor* actor, const BGSObjectInstance& a_weapon, UInt32 a_equipIndex, UInt32 a_shotCount);                                                         // 0F0 EFCE90
typedef void(*_EjectShellCasing)(TESObjectREFR* ref, TESObjectWEAP::InstanceData* instanceData, BGSEquipIndex eqIdx);
typedef void(*_ShowNotification)(const char* text, UInt32 edx, UInt32 r8d);
typedef bool(*_EquipItem)(void* actorEquipManager, Actor* actor, const BGSObjectInstance& a_object, UInt32 stackID, UInt32 number, const BGSEquipSlot* slot, bool queue, bool forceEquip, bool playSound, bool applyNow, bool preventUnequip);
typedef bool(*_UnEquipItem)(void* actorEquipManager, Actor* actor, const BGSObjectInstance& a_object, SInt32 number, const BGSEquipSlot* slot, UInt32 stackID, bool queue, bool forceEquip, bool playSound, bool applyNow, const BGSEquipSlot* a_slotBeingReplaced);

typedef ObjectRefHandle*(*_GetHandle)(ObjectRefHandle* handleOut, TESObjectREFR* ref);
extern RelocAddr <_GetHandle> GetHandle;
typedef bool(*_GetNiSmartPointer)(ObjectRefHandle* a_handle, TESObjectREFR** a_smartPointerOut);
extern RelocAddr <_GetNiSmartPointer> GetNiSmartPointer;
typedef bool(*_GetSmartPointer)(ObjectRefHandle* a_handle, TESObjectREFR** a_smartPointerOut);
extern RelocAddr <_GetSmartPointer> GetSmartPointer;
typedef BGSInventoryItem*(*_RequestInventoryItem)(BGSInventoryInterface* itfc, UInt32* a_handleID);
extern RelocAddr <_RequestInventoryItem> RequestInventoryItem;
extern RelocAddr <_CreateDuplicateForm> CreateDuplicateForm;
extern RelocAddr <_SetDelete> SetDeleteForm;

extern RelocAddr <uintptr_t> s_BGSObjectInstanceExtraVtbl;
extern RelocAddr <uintptr_t> s_ExtraUniqueIDVtbl;
extern RelocAddr <uintptr_t> s_ExtraModRankVtbl;
extern RelocAddr <uintptr_t> s_ExtraRankVtbl; 
extern RelocAddr <uintptr_t> s_ExtraAmmoVtbl;
extern RelocAddr <_EquipItem> EquipItemInternal;
extern RelocAddr <_UnEquipItem> UnequipItemInternal;
extern RelocAddr <_HasPerkInternal> HasPerkInternal;
extern RelocAddr <_AddItem_Native> AddItemNative;
extern RelocAddr <_RemoveItem_Native> RemoveItemNative;
extern RelocAddr <_SetAnimationVariableBool> SetAnimationVariableBoolInternal;
extern RelocAddr <_SetAnimationVariableInt> SetAnimationVariableIntInternal;
extern RelocAddr <_SetAnimationVariableFloat> SetAnimationVariableFloatInternal;
extern RelocAddr <_GetAnimationVariableBool> GetAnimationVariableBoolInternal;
extern RelocAddr <_GetAnimationVariableInt> GetAnimationVariableIntInternal;
extern RelocAddr <_GetAnimationVariableFloat> GetAnimationVariableFloatInternal;
extern RelocAddr <_PlayIdle> PlayIdleInternal; //0x13863A0
extern RelocAddr <_PlayIdle2> PlayIdleInternal2;
extern RelocAddr <_PlayIdleAction> PlayIdleActionInternal; //0x13864A0 
extern RelocAddr <_PlaySubgraphAnimation> PlaySubgraphAnimationInternal; //0x138A130
extern RelocAddr <_InitializeActorInstant> InitializeActorInstant;
extern RelocAddr <_UpdateAnimation> UpdateAnimation;
extern RelocAddr <_PlayEquipSound> PlayEquipSound;
extern RelocAddr <_ChangeAnimArchetype> ChangeAnimArchetype; //1387C10(vm*,0,actor*,kw*)
extern RelocAddr <_ChangeAnimFlavor> ChangeAnimFlavor; //1387CA0(vm*,0,actor*,kw*)
extern RelocAddr <_CheckKeywordType> CheckKeywordType;
extern RelocAddr <_IsInIronSights> IsInIronSights;
extern RelocAddr <_IsInPowerArmor> IsInPowerArmor;
extern RelocAddr <_DrawWeapon> DrawWeaponInternal;
extern RelocAddr <_UseAmmo> FireWeaponInternal;
extern RelocAddr <_EjectShellCasing> EjectShellCasing;
extern RelocAddr <_ReloadWeapon> ReloadWeaponInternal;
extern RelocAddr <_ShowNotification> ShowNotification;
extern RelocAddr <_GetKeywordFromValueArray> GetKeywordFromValueArray;
extern RelocAddr <_AttachModToInventoryItem> AttachModToInventoryItem_Internal;
extern RelocAddr <_AttachModToStack> AttachRemoveModStack;
extern RelocAddr <_ModifyStackData> ModifyStackData;
extern RelocAddr <_UpdMidProc> UpdateMiddleProcess;
extern RelocAddr <_UpdateEquipData> UpdateEquipData;
extern RelocAddr <_UpdateAnimGraph> UpdateAnimGraph;
extern RelocAddr <_UpdateEnchantments> UpdateEnchantments;
extern RelocAddr <_UpdateAVModifiers> UpdateAVModifiers;
extern RelocAddr <_UpdateAnimValueFloat> UpdateAnimValueFloat;
extern RelocAddr <_EquipHandler> EquipHandler;
extern RelocAddr <_UniversalEquipHandler> UniversalEquipHandler;
extern RelocAddr <_UnkSub_EFF9D0> UnkSub_EFF9D0;
extern RelocAddr <_UnkSub_DFE930> UnkSub_DFE930;
extern RelocAddr <_MainEquipHandler> MainEquipHandler;
extern RelocAddr <_NiStuff> NiStuff;
extern RelocAddr <_UpdateEquippedWeaponData> UpdateEquippedWeaponData;

extern RelocPtr <void*> g_pipboyInventoryData;
extern RelocPtr <void*> g_CheckStackIDFunctor;
extern RelocPtr <void*> g_ModifyModDataFunctor;
extern RelocPtr <void*> g_ApplyChangesFunctor;
extern RelocPtr <tArray<BGSKeyword*>> g_AttachPointKeywordArray;
extern RelocPtr <tArray<BGSKeyword*>> g_InstantiationKeywordArray;
extern RelocPtr <tArray<BGSKeyword*>> g_ModAssociationKeywordArray;
extern RelocPtr <tArray<BGSKeyword*>> g_AnimArchetypeKeywordArray;
extern RelocPtr <tArray<BGSKeyword*>> g_RecipeFilterKeywordArray;
extern RelocPtr <tArray<BGSKeyword*>> g_AnimFlavorKeywordArray;
extern RelocPtr <void*> g_sightedTransitionAnimValueHolder;
extern RelocPtr <void*> g_reloadSpeedAnimValueHolder;
extern RelocPtr <float> g_reloadSpeedMultiplier;
extern RelocPtr <std::unordered_map<UInt32, TESForm*>> g_FormMap;
extern RelocPtr <BSReadWriteLock*> g_FormMapLock;
extern RelocPtr <BGSInventoryInterface*> g_InventoryInterface;

extern RelocPtr  <DWORD> hkLifoAllocator_TLS;
extern RelocPtr  <DWORD> unk1_TLS;
extern RelocPtr  <void*> hkLifoAllocator_vtbl;

class ModColData
{
public:
	enum
	{
		kFlag_Optional = 1,
		kFlag_Unk = 2
	};
	BGSMod::Attachment::Mod* mod;
	UInt8 minlvl;
	UInt8 flags;
	//UInt16 padA;
	//UInt32 padC;
};

enum FormFlags
{
	kType_ModCol = 0x80
};

enum ActorStateFlags08
{
	kActorState_Sprint = 0x100,
	kActorState_Walk = 0x60,
	kActorState_Run = 0xA0,
	kActorState_Bashing = 0x6000000, //2, 4 or 6
	kActorState_Movement = 0xF, //different for directions
	kActorState_Freeroam = 0x80000000 //0 when in furniture
};

enum ActorStateFlags0C
{ 
	kWeaponState_Holstered = 0, 
	kWeaponState_Drawn = 0x2, 
	kWeaponState_Draw = 0x4, 
	kWeaponState_Sheathing = 0x8,

	kWeaponState_Aim = 0x18000, 
	kWeaponState_Lowered1stP = 0x08000, 
	kWeaponState_Lowered3rdP = 0x04000,
	kWeaponState_Ready1stP = 0,
	kWeaponState_Ready3rdP = 0x14000,
	kWeaponState_Reloading = 0x10000, 
	kWeaponState_Firing = 0x1C000,
	mWeaponActivityMask = 0x000FF000,

	kActorState_Sneak = 0x900,
	kActorState_FurnitureState = 0xE0000 //4: enter, 8: in, C:exit
};

/*
		std::uint32_t moveMode: 14;             // 08:00
		std::uint32_t flyState: 3;              // 08:14
		std::uint32_t lifeState: 4;             // 08:17
		std::uint32_t knockState: 4;            // 08:21
		std::uint32_t meleeAttackState: 3;      // 08:25
		std::uint32_t talkingToPlayer: 1;       // 08:28
		std::uint32_t forceRun: 1;              // 08:29
		std::uint32_t forceSneak: 1;            // 08:30
		std::uint32_t headTracking: 1;          // 08:31
		std::uint32_t reanimating: 1;           // 0C:00
		WEAPON_STATE weaponState: 3;            // 0C:01
		std::uint32_t wantBlocking: 1;          // 0C:04
		std::uint32_t flightBlocked: 1;         // 0C:05
		std::uint32_t recoil: 2;                // 0C:06
		std::uint32_t allowFlying: 1;           // 0C:08
		std::uint32_t staggered: 1;             // 0C:09
		std::uint32_t inWrongProcessLevel: 1;   // 0C:10
		std::uint32_t stance: 3;                // 0C:11
		std::uint32_t gunState: 4;              // 0C:14 3rdP 14,16
		INTERACTING_STATE interactingState: 2;  // 0C:18
		std::uint32_t headTrackRotation: 1;     // 0C:20
		std::uint32_t inSyncAnim: 1;            // 0C:21
		
		000FF000
		1stP, 3rdP fire:	0001 1100 no
		1stP, 3rdP aim:		0001 1000 yes
		3rdP up:			0001 0100 yes
		3rdP low:			0000 0100 yes
		1stP low:			0000 1000 yes
		1stP, 3rdP reload:	0001 0000 no
		
*/

enum AttachFlags
{
	bAttach = 0x1000,
	bFullEquip = 0x0100,//use this for armor
	bUpdateAnimGraph = 0x0200,

	iAmmoCountEmpty = 0x0000,
	iAmmoCountFull = 0x0001,
	iAmmoCountPersistent = 0x0002,
	iAmmoCountAdaptive = 0x0003,
	mAmmoCountMask = iAmmoCountEmpty | iAmmoCountFull | iAmmoCountPersistent | iAmmoCountAdaptive
};

enum EquipSlotType
{
	kType_HairTop,
	kType_HairLong,
	kType_FaceGenHead,
	kType_Body,
	kType_LeftHand,
	kType_RightHand,
	kType_UTorso,
	kType_ULeftArm,
	kType_URightArm,
	kType_ULeftLeg,
	kType_URightLeg,
	kType_ATorso,
	kType_ALeftArm,
	kType_ARightArm,
	kType_ALeftLeg,
	kType_ARightLeg,
	kType_Headband,
	kType_Eyes,
	kType_Beard,
	kType_Mouth,
	kType_Neck,
	kType_Ring,
	kType_Scalp,
	kType_Decapitation,
	kType_Unk1,
	kType_Unk2,
	kType_Unk3,
	kType_Unk4,
	kType_Unk5,
	kType_Shield,
	kType_Pipboy,
	kType_FX,
	kType_UnarmedWeapon,
	kType_MeleeWeapon,
	kType_WeaponDagger,
	kType_WeaponAxe,
	kType_WeaponMace,
	kType_WeaponTwoHandMelee,
	kType_WeaponBow,
	kType_WeaponStaff,
	kType_Quiver,
	kType_WeaponGun,
	kType_WeaponGrenade,
	kType_WeaponMine,
	kTotal
};

template <typename T> 
T convertToUnsignedAbs(T signedInt)
{
	short maxBitAddr = sizeof(signedInt) * 8 - 1;
	return ((signedInt & (1 << maxBitAddr)) >> maxBitAddr)*(signedInt & ~(1 << maxBitAddr)) + (((signedInt & (1 << maxBitAddr)) >> maxBitAddr) ^ 1)*(signedInt & ~(1 << maxBitAddr)) + (((signedInt & (1 << maxBitAddr)) >> maxBitAddr) ^ 1) * ~(1 << maxBitAddr);
}

template <typename T>
T GetVirtualFunction(void* baseObject, int vtblIndex) 
{
	uintptr_t* vtbl = reinterpret_cast<uintptr_t**>(baseObject)[0];
	return reinterpret_cast<T>(vtbl[vtblIndex]);
}

template <typename T>
T GetOffset(const void* baseObject, int offset) {
	return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}

template<typename T>
T* GetOffsetPtr(const void * baseObject, int offset)
{
	return reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}

class Ammo : public TESBoundObject
{
public:
	enum { kTypeID = kFormType_AMMO };

	// parents
	TESFullName					fullName;		
	BGSModelMaterialSwap		mat;		
	TESIcon						icon;			
	BGSMessageIcon				messageIcon;	
	TESValueForm				value;			
	BGSDestructibleObjectForm	destructible;	
	BGSPickupPutdownSounds		sounds;			
	TESDescription				description;	
	BGSKeywordForm				keywords;
	TESWeightForm				weight;

	struct AMMO_DATA
	{
	public:
		// members
		BGSProjectile* projectile;  // 00
		std::uint32_t health;       // 08
		std::int8_t flags;          // 0C
		float damage;               // 10
	};

	enum 
	{
		kIgnoreNormalResist = (1 << 0),
		kNotPlayable = (1 << 1),
		kNotBolt = (1 << 2)
	};

	bool isBolt() { return (data.flags & kNotBolt) != kNotBolt; }
	bool isPlayable() { return (data.flags & kNotPlayable) != kNotPlayable; }

	AMMO_DATA data;                // 160
	StringCache::Ref shortDesc;  // 178
	TESModel shellCasing;          // 180
};
STATIC_ASSERT(sizeof(Ammo) == 0x1B0);

class MagicalItem : public TESBoundObject
{
public:

	// parents
	TESFullName		fullName;	// 20
	BGSKeywordForm	keyword;	// 28

	// members
	struct EffectItem
	{
		float	magnitude;		// 00
		UInt32	area;			// 04
		UInt32	duration;		// 08
		EffectSetting* mgef;	// 0C
		float	cost;			// 10 - ?
		UInt32	unk14;			// 14 - ?
	};

	tArray<EffectItem*> effectItemList;	// 34
	UInt32				hostile;	// 40
	EffectSetting*		unk44;	// 44
	UInt32				unk48;	// 48
	UInt32				unk4C;	// 4C
};

class ActiveEffect
{
public:
	enum { kTypeID = kFormType_ActiveMagicEffect };

	enum
	{
		kFlag_Inactive = 0x8000
	};

	virtual ~ActiveEffect();

	class ActiveEffectReferenceEffectController
	{
	public:
		virtual ~ActiveEffectReferenceEffectController();

		//	void			** _vtbl;	// 00
		ActiveEffect	* effect;	// 04
		// possibly more
	};

	//	void					** _vtbl;		// 00
	ActiveEffectReferenceEffectController	controller;	// 04
	UInt32					unk0C[8];		// 0C
	void					* niNode;		// 2C
	MagicalItem				* item;			// 30
	MagicalItem::EffectItem	* effect;		// 34
	TESObjectREFR			* reference;	// 38
	TESForm					* sourceItem;	// 3C
	UInt32					unk40;			// 40
	UInt32					unk44;			// 44
	float					elapsed;		// 48
	float					duration;		// 4C
	float					magnitude;		// 50
	UInt32					flags;			// 54
	UInt32					unk58;			// 58
	UInt32					effectNum;		// 5C - Somekind of counter used to determine whether the ActiveMagicEffect handle is valid
	UInt32					unk60;			// 60
	UInt32					actorValue;		// 64 - Only seems to appear on value modifiers
	UInt32					unk68;			// 68
	UInt32					unk6C;			// 6C
};

class MagicalTarget
{
public:
	virtual ~MagicalTarget();

	virtual UInt8					Unk_01(int);
	virtual void				* Unk_02(void);
	virtual UInt8					Unk_03(void);
	virtual bool					Unk_04(void);
	virtual int						Unk_05(int);
	virtual bool					Unk_06(void); // pure   018
	virtual tArray<ActiveEffect>		* GetActiveEffects(void); // pure     01C
	virtual int						Unk_08(int);
	virtual void					Unk_09(int);
	virtual double					Unk_0A(int, int, int);
	virtual UInt8					Unk_0B(int, int, int);

	//	void	** _vtbl;	// 00
	UInt32 unk04;		// 04
	UInt32 unk08;		// 08
};

struct BGSProjectileData
{
public:
	enum
	{
		kType_Missile = 0x0001,
		kType_Lobber = 0x0002,
		kType_Beam = 0x0004,
		kType_Flame = 0x0008,
		kType_Cone = 0x0010,
		kType_Barrier = 0x0020,
		kType_Arrow = 0x0040,

		bSupersonic = 0x0080,
		bMuzzleFlash = 0x0008,
		bExplosion = 0x0002,
		bAltTrigger = 0x0004,
		bHitscan = 0x0001,
		bCanBeDisabled = 0x0020,
		bCanBePickedUp = 0x0040,
		bPinsLimbs = 0x0100,
		bPassThroughObjects = 0x0200,
		bDisableAimCorr = 0x0400,
		bPenetratesGeom = 0x0800,
		bContinuousUpdate = 0x1000,
		bSeeksTarget = 0x2000
	};
	// members
	UInt16 flags;							  // 00
	UInt16 type;							  // 02
	float gravity;                            // 04
	float speed;                              // 08
	float range;                              // 0C
	TESObjectLIGH* light;                     // 10
	TESObjectLIGH* muzzleFlashLight;          // 18
	float explosionProximity;                 // 20
	float explosionTimer;                     // 24
	BGSExplosion* explosionType;              // 28
	BGSSoundDescriptorForm* activeSoundLoop;  // 30
	float muzzleFlashDuration;                // 38
	float fadeOutTime;                        // 3C
	float force;                              // 40
	BGSSoundDescriptorForm* countdownSound;   // 48
	BGSSoundDescriptorForm* deactivateSound;  // 50
	TESObjectWEAP* defaultWeaponSource;       // 58
	float coneSpread;                         // 60
	float collisionRadius;                    // 64
	float lifetime;                           // 68
	float relaunchInterval;                   // 6C
	BGSTextureSet* decalData;                 // 70
	void* collisionLayer;        // 78 BGSCollisionLayer
	BGSProjectile* vatsProjectile;            // 80
	UInt64 tracerFrequency;              // 88
};
STATIC_ASSERT(sizeof(BGSProjectileData) == 0x90);

class Projectile : public TESBoundObject            // 000
{
public:
	//static constexpr auto RTTI{ RTTI::BGSProjectile };
	//static constexpr auto VTABLE{ VTABLE::BGSProjectile };
	//static constexpr auto FORM_ID{ ENUM_FORM_ID::kPROJ };

	// members
	TESFullName name;             // 068
	TESModel model;             // 078
	BGSPreloadable preload;            // 0A8
	BGSDestructibleObjectForm dest;  // 0B0
	BGSProjectileData data;                                  // 0C0
	TESModel muzzleFlashModel;                               // 150
	UInt64 soundLevel;  // 180
private:
	Projectile();
};
STATIC_ASSERT(sizeof(Projectile) == 0x188);

struct LEVELED_OBJECT
{
public:
	LEVELED_OBJECT(TESForm* a_form)
	{
		form = a_form;
		containerItemExtra = nullptr;
		count = 1;
		level = 1;
		chanceNone = 0;
	};
	// members
	TESForm* form;                  // 00
	BSExtraData* containerItemExtra;  // 08
	UInt16 count;            // 10
	UInt16 level;            // 12
	UInt8 chanceNone;         // 14
};
STATIC_ASSERT(sizeof(LEVELED_OBJECT) == 0x18);

struct struct_ll
{
	uint32_t size;
	LEVELED_OBJECT ll[];
};

class LeveledList : public BaseFormComponent  // 00
{
public:
	virtual ~LeveledList();

	// add
	virtual UInt8 GetChanceNone();											// 07
	virtual bool GetMultCalc();                                             // 08
	virtual UInt32 GetMaxLevelDifference() { return 0; }					// 09
	virtual const char* GetOverrideName() { return nullptr; }               // 0A
	virtual bool GetCanContainFormsOfType(UInt8 a_type) const = 0;			// 0B

	std::vector<LEVELED_OBJECT> getListAsVector();
	bool CreateList(const std::vector<LEVELED_OBJECT>& levo);
	bool CopyData(LeveledList* llist, bool doMiscData, bool copyList);
	void ClearList();

	// members
	TESGlobal* chanceGlobal;                                                        // 08
	void* keywordChances;															// 10
	LEVELED_OBJECT* leveledLists;                                                   // 18
	LEVELED_OBJECT** scriptAddedLists;                                              // 20
	UInt8 scriptListCount;															// 28
	UInt8 baseListCount;															// 29
	UInt8 chanceNone;																// 2A
	UInt8 llFlags;	//calcForLevel: 0x1, calcEachItem: 0x2, calcUseAll: 0x4			// 2B
	UInt8 maxUseAllCount;															// 2C
};
STATIC_ASSERT(sizeof(LeveledList) == 0x30);

class LevItem :
	public TESBoundObject,  // 00
	public LeveledList   // 68
{
public:
};
STATIC_ASSERT(sizeof(TESLevItem) == 0x98);

struct SubgraphIdentifier
{
public:
	// members
	std::size_t identifier;  // 0
};
STATIC_ASSERT(sizeof(SubgraphIdentifier) == 0x8);

struct SubgraphHandle
{
public:
	// members
	std::uint64_t handle;  // 0
};
STATIC_ASSERT(sizeof(SubgraphHandle) == 0x8);

struct SubGraphIdleRootData
{
public:
	// members
	SubgraphIdentifier subGraphID;  // 00
	BSFixedString idleRootName;     // 08
	std::int8_t count;              // 10
	std::int8_t activeCount;        // 11
	bool forFirstPerson;            // 12
};
STATIC_ASSERT(sizeof(SubGraphIdleRootData) == 0x18);

struct BGSBehaviorRootData
{
public:
	// members
	BSFixedString idleManagerRootToUse;  // 00
	BSFixedString behaviorRootFilename;  // 08
	SubgraphIdentifier identifier;       // 10
	bool firstPerson;                    // 18
};
STATIC_ASSERT(sizeof(BGSBehaviorRootData) == 0x20);

class BGSInventoryListEventData__Event
{

};

struct MiddleHighProcessData
{
public:
	// members
	BSTEventDispatcher<BGSInventoryListEventData__Event> inventoryEventSource;                                                 // 000
	NiPointer<bhkNPCollisionObject> poseBound;                                                                // 058
	UInt64 runOncePackage[6];                                                                              // 060
	tArray<UInt32> deadDetectList;                                                                     // 090
	tList<TESObjectREFR*> refListChairBed;                                                             // 0A8
	NiPoint3 rotation;                                                                                        // 0B8
	NiPoint3 rotationSpeed;                                                                                   // 0C4
	NiPoint3 actorMountPosition;                                                                              // 0D0
	NiPoint3 furniturePathPoint;                                                                              // 0DC
	NiPoint3 lastSeenPosition;                                                                                // 0E8
	UInt32 bleedoutAttacker;                                                                             // 0F4
	UInt32 wardState;                                         // 0F8
	void* animResponse;  // 100
	tArray<void*> commandedActors;                                                             // 108
	NiNode* damageRootNode[26];                                                                               // 120
	NiNode* weaponBone;                                                                                       // 1F0
	NiAVObject* headNode;                                                                                     // 1F8
	NiAVObject* torsoNode;                                                                                    // 200
	NiAVObject* faceTargetSourceNode;                                                                         // 208
	void* faceNodeSkinned;                                                                         // 210
	void* lightingProperty;                                                     // 218
	void* listItemstoEquipUnequip;                                                                      // 220
	void* lastHitData;                                                                                     // 228
	void* headDeferredHideLimb;                                                                   // 230
	tArray<void*> activeEffects;                                                                           // 238
	UInt64 it;
	void* animationGraphManager;                                           // 258
	void* animationVariableCache;                                                    // 260
	tArray<SubGraphIdleRootData> subGraphIdleManagerRoots;                                                  // 268
	SimpleLock equippedItemsLock;                                                                             // 280
	tArray<EquippedItemData> equippedItems;                                                                     // 288
	tArray<void*> clothExtraDataCache;                                                          // 2A0
	tArray<SubgraphHandle[2]> subGraphHandles;                                               // 2B8
	SubgraphIdentifier currentDefaultSubGraphID[4];                                            // 2D0
	SubgraphIdentifier requestedDefaultSubGraphID[4];                                          // 2F0
	SubgraphIdentifier currentWeaponSubGraphID[4];                                             // 310
	SubgraphIdentifier requestedWeaponSubGraphID[4];                                           // 330
	void* stanceData;                                                         // 350
	tArray<void*> nodeLocationArray;           // 358
	float stanceHeightArray[6][2];                                                                            // 370
	float headHeightOffset;                                                                                   // 3A0
	ObjectRefHandle currentFurniture;                                                                         // 3A4
	ObjectRefHandle occupiedFurniture;                                                                        // 3A8
	TESIdleForm* currentIdle;                                                                                 // 3B0
	UInt32 commandingActor;                                                                              // 3B8
	const TESIdleForm* furnitureIdle;                                                                         // 3C0
	void* faceAnimationData;                                                                // 3C8
	MagicItem* currentPackageSpell;                                                                           // 3D0
	TESObjectWEAP* lastBoundWeapon;                                                                           // 3D8
	void* charController;                                                         // 3E0
	void* penetrationDetectUtil;                                         // 3E8
	void* bodyPartPreload;                                                                    // 3F0
	TESIdleForm* lastIdlePlayed;                                                                              // 3F8
	void* perkData;                                                                                     // 400
	NiPoint3 lookAtLocation;                                                                                  // 408
	float pursueTimer;                                                                                        // 414
	float furnitureAngle;                                                                                     // 418
	float furnitureEntryFootstepDeltaTarget;                                                                  // 41C
	float packageIdleTimer;                                                                                   // 420
	float equippedWeight;                                                                                     // 424
	float desiredSpeed;                                                                                       // 428
	float animationSpeed;                                                                                     // 42C
	float bleedoutTimer;                                                                                      // 430
	float bleedoutRate;                                                                                       // 434
	float bleedoutMaxHealth;                                                                                  // 438
	float maxWardPower;                                                                                       // 43C
	float animGraphEventTimeout;                                                                              // 440
	float torchEvalTimer;                                                                                     // 444
	float alphaMult;                                                                                          // 448
	float scriptRefractPower;                                                                                 // 44C
	float sleepingTimer;                                                                                      // 450
	float deferredKillTimer;                                                                                  // 454
	float killMoveTimer;                                                                                      // 458
	float staggerTimer;                                                                                       // 45C
	float mountDismountSafetyTimer;                                                                           // 460
	float rangeWaypointIdleTimer;                                                                       // 464
	std::int32_t packageIdleNumber;                                                                           // 468
	std::int32_t reservationSlot;                                                                             // 46C
	SubgraphIdentifier currentFurnitureSubgraphID;                                                            // 470
	std::uint32_t currentFurnitureMarkerID;                                                                   // 478
	std::uint32_t occupiedFurnitureMarkerID;                                                                  // 47C
	std::uint64_t nextExtraArrow3DUpdate;                                                                     // 480
	std::uint32_t deferredKill;                                                                               // 488
	std::uint32_t flareFlags;                                                                                 // 48C
	std::int32_t useItem;                                              // 490
	std::int16_t animActionSuccess;                                                                           // 494
	std::uint16_t update3DModel;                                                                              // 496
	std::int8_t weaponCullCounter;                                                                            // 498
	std::int8_t archetypeChangeType;                                                                          // 499
	bool animWeaponCull;                                                                                      // 49A
	bool aimingTarget;                                                                                        // 49B
	bool doneClothesChange;                                                                                   // 49C
	bool pickPackIdle;                                                                                        // 49D
	bool doneOnce;                                                                                            // 49E
	bool refreshFlareFlags;                                                                                   // 49F
	bool pickPocketed;                                                                                        // 4A0
	bool summonedCreature;                                                                                    // 4A1
	bool forceNextUpdate;                                                                                     // 4A2
	bool playedBeginIdles;                                                                                    // 4A3
	bool playedEndIdles;                                                                                      // 4A4
	bool quickPlacement;                                                                                      // 4A5
	bool beenAttacked;                                                                                        // 4A6
	bool alwaysHit;                                                                                           // 4A7
	bool doNoDamage;                                                                                          // 4A8
	bool soulTrapped;                                                                                         // 4A9
	bool lookAt;                                                                                              // 4AA
	bool eating;                                                                                              // 4AB
	bool calcLight;                                                                                           // 4AC
	bool preventCombat;                                                                                       // 4AD
	bool dyingFromBleedout;                                                                                   // 4AE
	bool fleeing;                                                                                             // 4AF
	bool instantGetOut;                                                                                       // 4B0
	bool hostileGuard;                                                                                        // 4B1
	bool stopIdleFailed;                                                                                      // 4B2
	bool killQueued;                                                                                          // 4B3
	bool ragdollInstant;                                                                                      // 4B4
	bool scriptDeferredKill;                                                                                  // 4B5
	bool furnitureEntryLeftFootFirst;                                                                         // 4B6
	bool furnitureAnimationPlayed;                                                                            // 4B7
	bool queuedInstantInteractionAnimation;                                                                   // 4B8
	bool queuedModifyInitialAnimationPose;                                                                    // 4B9
	bool avoidPlayer;                                                                                         // 4BA
	bool usingPathingFaceTargetWhileTrackingOutOfRange;                                                       // 4BB
};
STATIC_ASSERT(offsetof(MiddleHighProcessData, lastIdlePlayed) == 0x3F8);
STATIC_ASSERT(sizeof(MiddleHighProcessData) == 0x4C0);

	