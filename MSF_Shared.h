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
#include "RNG.h"
#include  <thread>
#include  <chrono>


UInt32 roundp(float a);
BGSObjectInstanceExtra* CreateObjectInstanceExtra(BGSObjectInstanceExtra::Data* data);

class TESIdleForm;

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

	TESAmmo*	ammo;					// 10
	UInt64		loadedAmmoCount;		// 18
	void*		unk20;					// 20
	UInt64		unk28;					// 28
	NiAVObject*	object;					// 30
	UInt64		unk38[4];				// 38
	void*		unk58;					// 58 QueuedFile
	void*		unk60;					// 60 QueuedFile
	void*		unk68;					// 68 BSCloneReserver
	UInt64		unk70[5];				// 70
	BGSSoundKeywordMapping* firingSound;// 98
	BGSKeyword*	unkKeyword;				// A0
};
STATIC_ASSERT(offsetof(EquipWeaponData, firingSound) == 0x98);

class CheckStackIDFunctor
{
private:
	void* vtbl;		//00
	UInt64 stackID;	//08
	UInt64 pad0C;	//0C
public:
	CheckStackIDFunctor(UInt64 stackID);
};

class ModifyModDataFunctor
{
private:
	void* vtbl;						//00
public:
	UInt64 unk08; //1
	//UInt16 unk08;					//08 set to 1
	//UInt32 pad0A;					//0A not set when calling sub_1401A84B0
	//UInt16 pad0E;					//0E not set when calling sub_1401A84B0
	BGSMod::Attachment::Mod* mod;	//10
	TESObjectWEAP* baseWeap;		//18 set after calling sub_1401A84B0, no need to set in advance
	UInt8* byteptr;					//20 set to 1, wth is this
	UInt64 unk28;					//0x100 when attach, 0xFF when remove 
	//UInt8 unk28;					//28 set to 0
	//UInt8 unk29;					//29 set to 0x1, might be unk04 from BGSObjectInstanceExtra::Data::Form
	//UInt8 unk2A;					//2A from BGSInventoryItem::Stack flag lsh3, filled after calling sub_1401A84B0, no need to set in advance
	//UInt8 pad2B;					//2B set to 0
	//UInt32 pad2C;					//2C set to 0
	ModifyModDataFunctor(BGSMod::Attachment::Mod* modToAttach, UInt8* ret, bool Attach);
};
STATIC_ASSERT(sizeof(ModifyModDataFunctor) == 0x30);

struct unkTBOStruct
{
	TESBoundObject* baseForm;
	UInt64 unk08;
};

struct InstanceDataStruct
{
	TESBoundObject* baseForm;
	TBO_InstanceData* instanceData;
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

namespace Utilities
{
	TESForm* GetFormFromIdentifier(const std::string& identifier);
	UInt32 GetEquippedItemFormID(Actor * ownerActor, UInt32 iEquipSlot = 41);
	TESObjectWEAP::InstanceData * GetEquippedInstanceData(Actor * ownerActor, UInt32 iEquipSlot = 41);
	BGSObjectInstanceExtra* GetEquippedModData(Actor * ownerActor, UInt32 iEquipSlot = 41);
	BGSInventoryItem::Stack* GetEquippedStack(Actor* owner, UInt32 slotIndex);
	UInt64 GetStackID(BGSInventoryItem* item, BGSInventoryItem::Stack* stack);
	UInt64 GetInventoryItemCount(BGSInventoryList* inventory, TESForm* item);
	TESObjectMISC* GetLooseMod(BGSMod::Attachment::Mod* thisMod);
	BGSMod::Attachment::Mod* FindModByUniqueKeyword(BGSObjectInstanceExtra* modData, BGSKeyword* keyword);
	std::vector<BGSMod::Attachment::Mod*> FindModsByUniqueKeyword(BGSObjectInstanceExtra* modData, BGSKeyword* keyword);
	BGSMod::Attachment::Mod* GetFirstModWithPriority(BGSObjectInstanceExtra* modData, UInt8 priority);
	bool HasObjectMod(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod);
	bool WeaponInstanceHasKeyword(TESObjectWEAP::InstanceData* instanceData, BGSKeyword* checkKW);
	bool UpdateAimModel(MSFAimModel* oldModel, MSFAimModel* newModel);
	bool UpdateZoomData(MSFZoomData* oldData, MSFZoomData* newData);
	bool PlayIdle(Actor* actor, TESIdleForm* idle);
	bool PlayIdleAction(Actor* actor, BGSAction* action);
	void DrawWeapon(Actor* actor);
	void SetAnimationVariableBool(TESObjectREFR* ref, BSFixedString asVariableName, bool newVal);
	void SendNotification(std::string asNotificationText);
	//void ShowMessagebox(std::string asText);
	void AddItem(TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent);
	void RemoveItem(TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent, TESObjectREFR* toContainer = nullptr);
	bool ModActorValue(Actor* actor, ActorValueInfo* av, float val);
	bool AddRemActorValue(Actor* actor, ActorValueInfo* av, bool bAdd);
	bool AddRemKeyword(BGSKeywordForm* keywordForm, BGSKeyword* keyword, bool bAdd);
	UInt32 AddRemFlag(UInt32 flagHolder, UInt32 flag, UInt8 bAdd, UInt8 op = 0);
	float GetActorValue(tArray<Actor::ActorValueData>* avdata, UInt32 formId);
	BGSInventoryItem::Stack* AddInventoryStack(BGSInventoryItem::Stack* startStack);


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
class delayTask
{
public:
	template <class callable, class... arguments>
	delayTask(int delay, bool async, callable&& f, arguments&&... args)
	{
		std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

		if (async)
		{
			std::thread([delay, task]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				task();
			}).detach();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			task();
		}
	}

};

typedef void(*_AttachMod)(Actor* actor, TESObjectWEAP* baseWeap, void** CheckStackIDFunctor, void** ModifyModDataFunctor, UInt8 arg_unk28, void** weapbaseMf0, UInt8 unk_FFor0, BGSMod::Attachment::Mod* mod);
typedef bool(*_AttachModToStack)(BGSInventoryItem* invItem, CheckStackIDFunctor* IDfunctor, ModifyModDataFunctor* modFuntor, UInt32 unk_r9d, UInt32* unk_rsp20); //, UInt32 unk_rsp50
typedef bool(*_UpdMidProc)(Actor::MiddleProcess* midProc, Actor* actor, InstanceDataStruct weaponBaseStruct, BGSEquipSlot* equipSlot);
typedef void(*_UpdateEquipData)(ActorEquipData* equipData, InstanceDataStruct instance, UInt32* r8d);
typedef void(*_UpdateAnimGraph)(Actor* actor, bool rdx);
typedef bool(*_EquipHandler)(void* unkmanager, Actor* actor, InstanceDataStruct weaponBaseStruct, unkEquipSlotStruct equipSlotStruct);
typedef void(*_UniversalEquipHandler)(Actor* actor, InstanceDataStruct weaponBaseStruct, unkEquipSlotStruct equipSlotStruct);
typedef void(*_UnkSub_EFF9D0)(Actor* actor);
typedef void(*_UnkSub_DFE930)(Actor* actor, bool rdx);

typedef bool(*_IKeywordFormBase_HasKeyword)(IKeywordFormBase* keywordFormBase, BGSKeyword* keyword, UInt32 unk3); //https://github.com/shavkacagarikia/ExtraItemInfo
typedef void(*_AddItem_Native)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* target, unkItemStruct itemStruct, SInt32 count, bool bSilent);
typedef void(*_RemoveItem_Native)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* target, unkItemStruct itemStruct, SInt32 count, bool bSilent, TESObjectREFR* toContainer);
typedef bool(*_IsInIronSights)(VirtualMachine* vm, Actor* actor);
typedef void(*_SetAnimationVariableBool)(VirtualMachine* vm, UInt32 stackId, TESObjectREFR* ref, BSFixedString asVariableName, bool newVal);
typedef bool(*_PlayIdle)(VirtualMachine* vm, UInt32 stackId, Actor* actor, TESIdleForm* idle);
typedef bool(*_PlayIdle2)(Actor* actor, TESIdleForm* idle, UInt64 unk, VirtualMachine* vm, UInt32 stackId);
typedef bool(*_PlayIdleAction)(Actor* actor, BGSAction* action, TESObjectREFR* target, VirtualMachine* vm, UInt32 stackId);
typedef void(*_PlaySubgraphAnimation)(VirtualMachine* vm, UInt32 stackId, Actor* target, BSFixedString asEventName);
typedef void(*_DrawWeapon)(VirtualMachine* vm, UInt32 stackId, Actor* actor);
typedef void(*_ShowNotification)(std::string text, UInt32 edx, UInt32 r8d);
typedef UInt32(*_EquipItem)(void* unkmanager, Actor* actor, unkTBOStruct TBOStruct, SInt32 unk_r9d, SInt8 unk_rsp20, void* unk_rsp28, SInt8 unk_rsp30, bool forceequip, SInt8 unk_rsp40, SInt8 unk_rsp48, bool preventunequip);
typedef UInt8(*_UnEquipItem)(void* unkmanager, Actor* actor, unkTBOStruct TBOStruct, SInt32 unk_r9d, void* equipslot, SInt8 unk_rsp28, SInt8 unk_rsp30, bool abPreventEquip, SInt8 unk_rsp40, SInt8 unk_rsp48, void* unk_rsp50);
//typedef bool(*_UnEquipItem)(void* unkPtr, Actor* target, unkWeapBaseStruct baseWeap, UInt32 unk_r9d, UInt64 unk_rsp20, SInt32 unk_rsp28, UInt8 unk_rsp30, UInt8 unk_rsp38, UInt8 unk_rsp40, UInt8 unk_rsp48, UInt64 unk_rsp50);
// sub_140E1BEF0(qword_145A10618, v7, &v25, 1, v16, -1, 1, a5, 1, 0, 0i64);

extern RelocPtr <void*> g_pipboyInventoryData;
extern RelocPtr <void*> g_CheckStackIDFunctor;
extern RelocPtr <void*> g_ModifyModDataFunctor;
extern RelocPtr <void*> unk_05AB38D0; //unkmanager


//AddItem?shorter?: 0x143A0C0
//RemItem?shorter?: 0x143AA60

//sub_140E23A90 fire weapon?

//allfunctions: sub_1413E6BC0
//objRef_functions: sub_141410E40
//debug_functions: sub_1413BA570
//actor_functions: sub_14138C960
//weapon_functions: sub_141446AA0 //fire?
//scriptObj_functions: sub_1413DB070
//Game_functions: sub_1413C22C0
//Utility_functions: sub_141452C30
//Message_functions: sub_1413AF660

//modifyModDataFunctor:
//sub_1409CE4E0
//sub_1414038B0 //attachmodtoinvitem
//sub_141403A70 //uniqueID


/*
0010010680000000: 4->6: drawn 0: holstered, A: sheathing

0011810680000000: 18: aim, 08: weap behúzva, 1C: fire, 10: reloading
enum {  };
0010090680000000: 9: sneak, LAST 3 BYTE: 1:sprint; 6:slow walk, A: run
0010090680000000: after 68: 2/4/6: bashing
0014010080000000: 4: furniture enter, then 8, C when exit; 8: when in freeroam, 0 ehrn in furniture

UInt32 unkA94; //stg to do w/ held object, increasing
UInt64 v. UInt8 unkB18; //if holding object: 4, otherwise: F
tArray<mouseSpringAction> unkB38;
BSTValueEventSource<PlayerCommandTypeEvent> unkB20; //0:interact, 3: freeaim, 6: pick up
UInt32 unkB5C; picked up objectREF formID
UInt38 unkB60; float? when holding object, constant
UInt64 v. UInt8 unkCE8; 1 when holding object

*/

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
	kWeaponState_Lowered = 0x08000, 
	kWeaponState_Reloading = 0x10000, 
	kWeaponState_Fire = 0x1C000,
	kActorState_Sneak = 0x900,
	kActorState_FurnitureState = 0xE0000 //4: enter, 8: in, C:exit
};

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
	kType_Weapon = 41
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
	TESFullName			fullName;		// 20
	BGSModelMaterialSwap	mat;		// 28
	TESIcon				icon;			// 44
	BGSMessageIcon		messageIcon;	// 4C
	TESValueForm		value;			// 58
	BGSDestructibleObjectForm	destructible;	// 50
	BGSPickupPutdownSounds		sounds;			// 68
	TESDescription		description;	// 74
	BGSKeywordForm		keyword;		// 80

	// members

	// 0C
	struct AmmoSettings
	{
		BGSProjectile	* projectile;
		UInt32			flags;
		float			damage;
	};

	enum {
		kIgnoreNormalResist = (1 << 0),
		kNotPlayable = (1 << 1),
		kNotBolt = (1 << 2)
	};

	bool isBolt() { return (settings.flags & kNotBolt) != kNotBolt; }
	bool isPlayable() { return (settings.flags & kNotPlayable) != kNotPlayable; }

	AmmoSettings		settings;	// 8C
	StringCache::Ref	unk98;	// 98
};

class ExtraEnchantment : public BSExtraData
{
public:
	ExtraEnchantment();
	virtual ~ExtraEnchantment();

	EnchantmentItem*	enchant;		// 08
	UInt16				maxCharge;		// 0C
	UInt8				unk0E;			// 0E - usually 0
	UInt8				pad0F;			// 0F

	static ExtraEnchantment* Create();
};

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

class TESIdleForm : public TESForm
{
public:
	enum { kTypeID = kFormType_IDLE };
};
	