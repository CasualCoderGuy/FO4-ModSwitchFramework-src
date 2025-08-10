#pragma once
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "rva\ModuleRelocation.h"
//#include "MSF_Serialization.h"


typedef UInt32 WeaponStateID;

class DataHolderParentInstance
{
public:
	ExtraDataList* extraList;
	UInt32 formID;
	UInt32 stackID;
	ObjectRefHandle refHandle;
};

class ExtraWeaponState
{
public:
	~ExtraWeaponState();
	ExtraWeaponState(ExtraRank* holder);
	static ExtraWeaponState* Init(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	static bool HandleWeaponStateEvents(UInt8 eventType, Actor* actor = nullptr, UInt8 eventSubtype = 0, UInt32 oldLoadedAmmoCount = 0);
	static TESAmmo* GetAmmoForWorkbenchUI(ExtraDataList* extraList);
	//static auto GetCurrentUniqueState(BGSObjectInstanceExtra* attachedMods);
	static ModData::Mod* ExtraWeaponState::GetCurrentUniqueStateMod(BGSObjectInstanceExtra* attachedMods);
	static bool ExtraWeaponState::HasTRSupport(ExtraDataList* extraDataList);
	static ModData::Mod defaultStatePlaceholder;
	//bool SetWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, bool temporary);
	//bool RecoverTemporaryState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	//bool SetCurrentStateTemporary();
	bool HandleEquipEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool HandleFireEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool HandleAmmoChangeEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	bool HandleReloadEvent(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType, UInt32 oldLoadedAmmoCount);
	static bool HandleModChangeEvent(ExtraDataList* extraDataList, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify, UInt8 eventType); //update burst manager
	bool UpdateWeaponStates(ExtraDataList* extraDataList, EquipWeaponData* equipData, UInt8 eventType, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify = nullptr);
	TESAmmo* GetCurrentAmmo();
	bool SetCurrentAmmo(TESAmmo* ammo);
	bool SetEquippedAmmo(TESAmmo* ammo);
	TESAmmo* GetEquippedAmmo();
	UInt8 HasNotSupportedAmmo();
	void PrintStoredData();

	enum
	{
		kEventTypeUndefined,
		kEventTypeEquip,
		kEventTypeAmmoCount,
		kEventTypeFireWeapon,
		kEventTypeReload,
		kEventTypeEmptyMag,
		kEventTypeSwitchMag,
		kEventTypeModded,
		kEventTypeModdedWorkbench,
		kEventTypeModdedGameplay,
		kEventTypeModdedSwitch,
		kEventTypeModdedAmmo,
		kEventTypeModdedNewChamber,

		bEventTypeReloadBCR,
		bEventTypeReloadFullBCR,
		bEventTypeReloadSwitchBCR,
		bEventTypeReloadEndBCR,
		bEventTypeReloadTactical,
		bEventTypeReloadAfterSwitch,
		bEventTypeReloadInventory
	};
	struct AmmoStateData
	{
		UInt16 ammoCapacity;
		UInt16 chamberSize; //if -1: equals to ammoCapacity
		UInt32 loadedAmmo;
		UInt32 chamberedCount;
	};
	AmmoStateData* GetAmmoStateData();

	class WeaponState
	{
	public:
		WeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData, ModData::Mod* currUniqueStateMod);
		WeaponState(UInt16 newflags, UInt16 newammoCapacity, UInt16 newchamberSize, UInt32 newchamberedCount, UInt16 newshotCount, UInt64 newloadedAmmo, TESAmmo* newchamberedAmmo, TESAmmo* newequippedAmmo, std::vector<TESAmmo*>* newBCRammo, std::vector<BGSMod::Attachment::Mod*>* newStateMods);		bool FillData(ExtraDataList* extraDataList, EquipWeaponData* equipData, ModData::Mod* currUniqueStateMod);
		bool UpdateAmmoState(ExtraDataList* extraDataList, BGSObjectInstanceExtra* attachedMods, UInt8 eventType, bool stateChange, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify);
		WeaponState* Clone();
		enum
		{
			bHasLevel = 0x0100,
			bActive = 0x0200,
			bHasTacticalReload = 0x0010,
			bHasBCR = 0x0020,
			bChamberLIFO = 0x0040, //otherwise FIFO
			mChamberMask = 0x00F0,
			bNotPlayable = 0x0001,
			bIsFusionCore = 0x0002,
			mAmmoMask = 0x000F
		};
		UInt16 flags; //state flags
		UInt16 ammoCapacity;
		UInt16 chamberSize; //if -1: equals to ammoCapacity
		volatile short shotCount; 
		volatile int loadedAmmo;
		volatile int chamberedCount;
		TESAmmo* chamberedAmmo;
		TESAmmo* equippedAmmo;
		std::vector<TESAmmo*> BCRammo; //if BCR && TR: size=ammoCap; if !BCR && TR: size=chamber; if BCR && !TR: size=ammoCap
		std::vector<BGSMod::Attachment::Mod*> stateMods;
	private:
		WeaponState() {};
	};
	friend class StoredExtraWeaponState;
	bool GetUniqueStateModsToModify(BGSObjectInstanceExtra* attachedMods, std::vector<std::pair<BGSMod::Attachment::Mod*, bool>>* modsToModify, std::pair<WeaponState*, WeaponState*>* stateChange);
private:
	ExtraWeaponState(ExtraDataList* extraDataList, EquipWeaponData* equipData);
	WeaponStateID ID;
	ExtraRank* holder; //ExtraDataList
	std::map<BGSMod::Attachment::Mod*, WeaponState*> weaponStates;
	WeaponState* currentState;
	//BurstModeManager* burstModeManager;
};

class WeaponStateStore
{
public:
	WeaponStateStore()
	{
		mapstorage.reserve(100);
		vectorstorage.reserve(100);
		ranksToClear.reserve(100);
	};
	void Free()
	{
		for (auto& state : vectorstorage)
			delete state;
		vectorstorage.clear();
		mapstorage.clear();
		ranksToClear.clear();
	};
	WeaponStateID Add(ExtraWeaponState* state)
	{
		//nocheck
		vectorstorage.push_back(state);
		return vectorstorage.size();
	};
	ExtraWeaponState* Get(WeaponStateID id)
	{
		ExtraWeaponState* state = nullptr;
		if (id && id <= vectorstorage.size())
			state = vectorstorage[id - 1];
		return state;
	};
	ExtraWeaponState* GetValid(WeaponStateID id)
	{
		ExtraWeaponState* state = nullptr;
		if (id && id <= vectorstorage.size())
			state = vectorstorage[id - 1];
		if (state)// && state->ValidateParent())
			return state;
		return nullptr;
	};
	bool StoreForLoad(WeaponStateID id, ExtraRank* holder)
	{
		if (!holder || id == 0)
			return false;
		auto itMap = mapstorage.find(id);
		auto itClear = ranksToClear.find(id);
		if (itMap == mapstorage.end() && itClear == ranksToClear.end())
			mapstorage[id] = holder;
		else
		{
			_MESSAGE("WARNING: duplicate WeaponStateID found");
			ranksToClear.insert(ranksToClear.begin(), std::pair<WeaponStateID, ExtraRank*>(id, holder));
			ranksToClear.insert(ranksToClear.begin(), std::pair<WeaponStateID, ExtraRank*>(id, itMap->second));
			mapstorage.erase(itMap);
			return false;
		}
		//ExtraRank* occupied = mapstorage[id];
		//if (!occupied)
		//	mapstorage[id] = holder;
		return true;
	};
	ExtraWeaponState* GetEquipped(Actor* owner)
	{
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedWeaponStack(owner);
		if (!eqStack)
			return nullptr;
		ExtraDataList* dataList = eqStack->extraData;
		if (!dataList)
			return nullptr;
		ExtraRank* holder = (ExtraRank*)dataList->GetByType(kExtraData_Rank);
		if (!holder)
			return nullptr;
		ExtraWeaponState* state = nullptr;
		if (holder->rank && holder->rank <= vectorstorage.size())
			state = vectorstorage[holder->rank - 1];
		return state;
	};
	ExtraRank* GetForLoad(WeaponStateID id)
	{
		if (id == 0)
			return nullptr;
		auto itHolder = mapstorage.find(id);
		if (itHolder == mapstorage.end() || !itHolder->second)
			return nullptr;
		ExtraRank* holder = itHolder->second;
		if (holder->rank != id)
		{
			_DEBUG("IDerror");
			holder = nullptr;
		}
		return holder;
	};
	UInt32 GetCount()
	{
		return vectorstorage.size();
	};
	void SaveWeaponStates(std::function<bool(const F4SESerializationInterface*, UInt32, ExtraWeaponState*)> f_callback, const F4SESerializationInterface* intfc, UInt32 version)
	{
		for (const auto& state : vectorstorage)
		{
			if (state)
				f_callback(intfc, version, state);
		}
	};
	void InvalidateID(WeaponStateID id)
	{
		if (id == 0)
			return;
		auto itMap = mapstorage.find(id);
		if (itMap == mapstorage.end())
			return;
		ranksToClear.insert(ranksToClear.begin(), std::pair<WeaponStateID, ExtraRank*>(itMap->first, itMap->second));
		mapstorage.erase(itMap);
	};
	void InvalidateAllIDs()
	{
		ranksToClear.insert(mapstorage.begin(), mapstorage.end());
		mapstorage.clear();
	};
	bool ClearInvalidWeaponStates()
	{
		for (auto holder : ranksToClear)
		{
			_DEBUG("invalid: %08X %p", holder.first, holder.second);
			holder.second->rank = 0;
		}
		ranksToClear.clear();
		return true;
	};
	void PrintStoredWeaponStates()
	{
		_MESSAGE("");
		_MESSAGE("===Printing stored WeaponStates===");
		for (const auto& state : vectorstorage)
		{
			if (state)
				state->PrintStoredData();
		}
		_MESSAGE("===Printing WeaponState IDs in Player inventory===");
		for (UInt32 i = 0; i < (*g_player)->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			(*g_player)->inventoryList->items.GetNthItem(i, inventoryItem);
			TESObjectWEAP* weap = DYNAMIC_CAST(inventoryItem.form, TESForm, TESObjectWEAP);
			if (!weap || !inventoryItem.stack)
				continue;
			UInt32 stackID = -1;
			for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
			{
				stackID++;
				if (stack->extraData)
				{
					ExtraRank* extraRank = (ExtraRank*)stack->extraData->GetByType(kExtraData_Rank);
					if (!extraRank)
						continue;
					UInt32 modNo = 0;
					BGSObjectInstanceExtra* oie = (BGSObjectInstanceExtra*)stack->extraData->GetByType(kExtraData_ObjectInstance);
					if (oie)
						modNo = oie->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
					uint8_t isEquipped = 0;
					if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
						isEquipped = 1;
					_MESSAGE("ID: %08X, item: %08X, stackID: %08X, modNo: %i, isEquipped: %02X", extraRank->rank, weap->formID, stackID, modNo, isEquipped);

				}
			}
		}
		_MESSAGE("");
	};
private:
	std::unordered_map<WeaponStateID, ExtraRank*> mapstorage; //used only for the loading of f4se serialized data, WeaponStateID is invalid afterwards
	std::unordered_multimap<WeaponStateID, ExtraRank*> ranksToClear;
	std::vector<ExtraWeaponState*> vectorstorage; // used for quick access of WeaponState with ExtraRank->rank (WeaponStateID) being the vector index +1
};

class BCRinterface
{
public:
	BCRinterface()
	{
		_base = reinterpret_cast<uintptr_t>(GetModuleHandle("BulletCountedReload.dll"));
		if (_base)
			_MESSAGE("BulletCountedReload.dll found at %p", _base);
		else
			_MESSAGE("BulletCountedReload.dll not found");
		//BulletCountedReload.dll+8785
		//73004
		//730B8
		BCR_ammoCount = RelocModuleAddr<UInt32>(_base, 0x79B24);
		BCR_ammoCapacity = RelocModuleAddr<UInt32>(_base, 0x79B20);
		BCR_instanceData = RelocModuleAddr<TESObjectWEAP::InstanceData*>(_base, 0x79B10);
		BCR_totalAmmo = RelocModuleAddr<UInt32>(_base, 0x79B04);
		reloadEnded = RelocModuleAddr<bool>(_base, 0x73CA8);
		reloadStarted = RelocModuleAddr<bool>(_base, 0x79AD2);
		incrementer = RelocModuleAddr<UInt32>(_base, 0x79AD4);
		toAdd = RelocModuleAddr<UInt32>(_base, 0x79B00);
		stopPressed = RelocModuleAddr<bool>(_base, 0x79AD1);
		animWillPlay = RelocModuleAddr<bool>(_base, 0x73CA9);
		uncull = RelocModuleAddr<bool>(_base, 0x79AD3);
		readyToStop = RelocModuleAddr<bool>(_base, 0x79AD0);
		animDone = RelocModuleAddr<UInt32>(_base, 0x79AE0);
	};
	bool SetBCRammoCap(UInt32 ammoCap)
	{
		if (!_base)
			return false;
		return (*(UInt32*)BCR_ammoCapacity.GetUIntPtr()) = ammoCap;
	}
	bool SetBCRloadedAmmo(UInt32 loadedAmmo)
	{
		if (!_base)
			return false;
		return (*(UInt32*)BCR_ammoCount.GetUIntPtr()) = loadedAmmo;
	}
	bool StoreBCRvariables()
	{
		if (!_base)
			return false;
		stored_BCR_ammoCount = *(UInt32*)BCR_ammoCount.GetUIntPtr();
		stored_BCR_ammoCapacity = *(UInt32*)BCR_ammoCapacity.GetUIntPtr();
		stored_BCR_totalAmmo = *(UInt32*)BCR_totalAmmo.GetUIntPtr();
		stored_reloadEnded = *(bool*)reloadEnded.GetUIntPtr();
		stored_reloadStarted = *(bool*)reloadStarted.GetUIntPtr();
		stored_incrementer = *(UInt32*)incrementer.GetUIntPtr();
		stored_toAdd = *(UInt32*)toAdd.GetUIntPtr();
		stored_stopPressed = *(bool*)stopPressed.GetUIntPtr();
		stored_animWillPlay = *(bool*)animWillPlay.GetUIntPtr();
		stored_uncull = *(bool*)uncull.GetUIntPtr();
		stored_readyToStop = *(bool*)readyToStop.GetUIntPtr();
		stored_animDone = *(UInt32*)animDone.GetUIntPtr();
		return true;
	}
	bool RestoreBCRvariables()
	{
		if (!_base)
			return false;
		 (*(UInt32*)BCR_ammoCount.GetUIntPtr()) = stored_BCR_ammoCount;
		 (*(UInt32*)BCR_ammoCapacity.GetUIntPtr()) = stored_BCR_ammoCapacity;
		 (*(UInt32*)BCR_totalAmmo.GetUIntPtr()) = stored_BCR_totalAmmo;
		 (*(bool*)reloadEnded.GetUIntPtr()) = stored_reloadEnded; //
		 (*(bool*)reloadStarted.GetUIntPtr()) = stored_reloadStarted;
		 (*(UInt32*)incrementer.GetUIntPtr()) = stored_incrementer; //
		 (*(UInt32*)toAdd.GetUIntPtr()) = stored_toAdd;
		 (*(bool*)stopPressed.GetUIntPtr()) = stored_stopPressed; //
		 (*(bool*)animWillPlay.GetUIntPtr()) = stored_animWillPlay; //
		 (*(bool*)uncull.GetUIntPtr()) = stored_uncull;
		 (*(bool*)readyToStop.GetUIntPtr()) = stored_readyToStop;
		 (*(UInt32*)animDone.GetUIntPtr()) = stored_animDone;
		return true;
	}
	bool UpdateBCRvariables(UInt32 ammoCount, UInt32 ammoCapacity, UInt32 totalAmmoCount)
	{
		if (!_base)
			return false;
		UInt32 ammoCap = *(UInt32*)BCR_ammoCapacity.GetUIntPtr();
		UInt32 add = ammoCap;
		if (ammoCap > totalAmmoCount)
			add = totalAmmoCount;
		if (add != ammoCap && ammoCount)
			(*(UInt32*)incrementer.GetUIntPtr()) = ammoCount-1;
		_DEBUG("ammoCount: %08X, add: %08X, totalAmmoCount: %08X, ammoCap: %08X", ammoCount, add, totalAmmoCount, ammoCap);
		(*(UInt32*)BCR_ammoCount.GetUIntPtr()) = ammoCount;
		//*(UInt32*)BCR_ammoCapacity.GetUIntPtr() = stored_BCR_ammoCapacity;
		(*(UInt32*)BCR_totalAmmo.GetUIntPtr()) = totalAmmoCount-ammoCount;
		(*(UInt32*)toAdd.GetUIntPtr()) = add;
		(*(bool*)readyToStop.GetUIntPtr()) = true;
		(*(UInt32*)animDone.GetUIntPtr()) = 1;
		return true;
	}
	void LogStored()
	{
		_MESSAGE("%08X, %08X, %08X, %02X, %02X, %08X, %08X, %02X, %02X, %02X, %02X", stored_BCR_ammoCount, stored_BCR_ammoCapacity, stored_BCR_totalAmmo, stored_reloadEnded, stored_reloadStarted, stored_incrementer, stored_toAdd, stored_stopPressed, stored_animWillPlay, stored_uncull, stored_readyToStop);
	}

	bool IsLoaded() { return _base ? true : false; }
private:
	uintptr_t _base;
	RelocModuleAddr<TESObjectWEAP::InstanceData*> BCR_instanceData;
	RelocModuleAddr<UInt32> BCR_ammoCount;
	RelocModuleAddr<UInt32> BCR_ammoCapacity;
	RelocModuleAddr<UInt32> BCR_totalAmmo;
	RelocModuleAddr<bool> reloadEnded;
	RelocModuleAddr<bool> reloadStarted;
	RelocModuleAddr<UInt32> incrementer;
	RelocModuleAddr<UInt32> toAdd;
	RelocModuleAddr<bool> stopPressed;
	RelocModuleAddr<bool> animWillPlay;
	RelocModuleAddr<bool> uncull;
	RelocModuleAddr<bool> readyToStop;
	RelocModuleAddr<UInt32> animDone;
	UInt32 stored_BCR_ammoCount;
	UInt32 stored_BCR_ammoCapacity;
	UInt32 stored_BCR_totalAmmo;
	bool stored_reloadEnded;
	bool stored_reloadStarted;
	UInt32 stored_incrementer;
	UInt32 stored_toAdd;
	bool stored_stopPressed;
	bool stored_animWillPlay;
	bool stored_uncull;
	bool stored_readyToStop;
	UInt32 stored_animDone;
};

namespace MSF_WeaponState
{
	bool EquippedWeaponHasTRSupport(Actor* owner);
};