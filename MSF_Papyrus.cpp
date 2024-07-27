#include "MSF_Papyrus.h"
#include "MSF_Data.h"

namespace MSF_Papyrus
{

	//============InstanceData===========
	TESAmmo * GetEquippedAmmo(StaticFunctionTag*, Actor* owner, UInt32 slotID = 41)
	{
		if (!owner)
			return nullptr;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
		if (instanceData) 
		{
			return instanceData->ammo;
		}
		return nullptr;
	}

	bool SetEquippedAmmo(StaticFunctionTag*, Actor* owner, TESAmmo* newAmmo)
	{
		if (!owner)
			return false;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData) 
		{
			if (instanceData->ammo != newAmmo && owner->middleProcess->unk08->equipData[0].equippedData)
			{
				instanceData->ammo = newAmmo;
				owner->middleProcess->unk08->equipData[0].equippedData->ammo = newAmmo;
				//owner->middleProcess->unk08->equipData[0].equippedData->unk18 = 0;
				return true;
			}
		}
		return false;
	}

	UInt32 GetEquippedAmmoCapacity(StaticFunctionTag*, Actor* owner)
	{
		if (!owner)
			return 0;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData) 
		{
			return instanceData->ammoCapacity;
		}
		return 0;
	}

	bool SetEquippedAmmoCapacity(StaticFunctionTag*, Actor* owner, UInt32 newCap)
	{
		if (!owner || newCap < 0)
			return 0;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData) 
		{
			if (instanceData->ammoCapacity != newCap) 
			{
				instanceData->ammoCapacity = newCap;
				return true;
			}
		}
		return false;
	}

	BGSProjectile * GetEquippedProjectile(StaticFunctionTag*, Actor* owner)
	{
		if (!owner)
			return nullptr;
		TESObjectWEAP::InstanceData * instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData) 
		{
			if (instanceData->firingData) 
			{
				return instanceData->firingData->projectileOverride;
			}
		}
		return nullptr;
	}

	bool SetEquippedProjectile(StaticFunctionTag*, Actor * owner, BGSProjectile* newProjectile)
	{
		if (!owner)
			return false;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData) 
		{
			if (instanceData->firingData) 
			{
				instanceData->firingData->projectileOverride = newProjectile;
				return true;
			}
		}
		return false;
	}

	UInt32 GetLoadedAmmoCount(StaticFunctionTag*, Actor* owner)
	{
		if (!owner)
			return 0;
		if (owner->middleProcess && owner->middleProcess->unk08->equipData.entries && owner->middleProcess->unk08->equipData[0].equippedData)
		{
			return (UInt32)owner->middleProcess->unk08->equipData[0].equippedData->unk18;
		}
		return 0;
	}

	bool SetLoadedAmmoCount(StaticFunctionTag*, Actor* owner, UInt32 amount = 0)
	{
		if (!owner || amount < 0)
			return false;
		if (owner->middleProcess && owner->middleProcess->unk08->equipData.entries && owner->middleProcess->unk08->equipData[0].equippedData)
		{
			owner->middleProcess->unk08->equipData[0].equippedData->unk18 = (UInt64)amount;
			return true;
		}
		return false;
	}

	TESForm* GetEquippedZoomData(StaticFunctionTag*, Actor* owner)
	{
		if (!owner)
			return nullptr;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData)
		{
			if (instanceData->zoomData)
				return (TESForm*)instanceData->zoomData;
		}
		return nullptr;
	}

	bool SetEquippedZoomData(StaticFunctionTag*, Actor* owner, TESForm* zoomData)
	{
		if (!owner || !zoomData)
			return nullptr;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		BGSZoomData* zoom = (BGSZoomData*)zoomData;
		if (instanceData && zoom)
		{
			if (instanceData->zoomData)
			{
				instanceData->zoomData = zoom;
				return true;
			}
		}
		return false;
	}

	bool ToggleAutomaticWeapon(StaticFunctionTag*, Actor* owner)
	{

		if (!owner)
			return false;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		BGSKeyword* soundKeyword = (BGSKeyword*)LookupFormByID(0x00A191C);
		if (instanceData && soundKeyword)
		{
			instanceData->flags ^= 0x0008000;
			if (instanceData->flags & 0x0008000)
				Utilities::AddRemKeyword(instanceData->keywords, soundKeyword, false);
			else
				Utilities::AddRemKeyword(instanceData->keywords, soundKeyword, true);
			return true;
		}
		return false;
	}

	bool SetAutomaticWeapon(StaticFunctionTag*, Actor* owner, bool bAuto)
	{
		if (!owner)
			return false;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		BGSKeyword* soundKeyword = (BGSKeyword*)LookupFormByID(0x00A191C);
		if (instanceData && soundKeyword)
		{
			if (!bAuto && (instanceData->flags & 0x0008000))
			{
				instanceData->flags &= ~0x0008000;
				Utilities::AddRemKeyword(instanceData->keywords, soundKeyword, false);
			}
			else if (bAuto && (instanceData->flags | 0x0008000))
			{
				instanceData->flags |= 0x0008000;
				Utilities::AddRemKeyword(instanceData->keywords, soundKeyword, true);
			}
			return true;
		}
		return false;
	}

	UInt32 GetEquippedStackCount(StaticFunctionTag*, Actor* owner, UInt32 slotIndex)
	{
		UInt32 count = 0;
		if (slotIndex >= BIPOBJECT::BIPED_OBJECT::kTotal)
			return 0;
		if (!owner->biped.get())
			return 0;
		auto item = owner->biped.get()->object[slotIndex].parent.object;
		if (!item)
			return 0;
		if (!owner->inventoryList)
			return 0;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			bool ret = inventoryItem.stack->Visit([&](BGSInventoryItem::Stack* stack)
			{
				if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
				{
					count = (UInt32)stack->count;
					return false;
				}

				return true;
			});
			if (!ret)
				break;
		}

		return count;
	}

	bool SetEquippedStackCount(StaticFunctionTag*, Actor* owner, UInt32 count, UInt32 slotIndex)
	{
		if (slotIndex >= BIPOBJECT::BIPED_OBJECT::kTotal || count < 0)
			return false;
		if (!owner->biped.get())
			return false;
		auto item = owner->biped.get()->object[slotIndex].parent.object;
		if (!item)
			return false;
		if (!owner->inventoryList)
			return false;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			bool ret = inventoryItem.stack->Visit([&](BGSInventoryItem::Stack* stack)
			{
				if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
				{
					stack->count = (SInt32)count;
					return false;
				}

				return true;
			});
			if (!ret)
				break;
		}

		return true;
	}

	bool EquippedItemHasMod(StaticFunctionTag*, Actor* owner, BGSMod::Attachment::Mod* mod, UInt32 slotIndex)
	{
		if (!owner || !mod)
			return false;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(owner, slotIndex);
		if (!modData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			if (data->forms[i].formId == mod->formID)
				return true;
		}
		return false;
	}

	bool EquippedItemHasKeyword(StaticFunctionTag*, Actor* owner, BGSKeyword* keyword, UInt32 slotIndex)
	{
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotIndex);
		if (Utilities::WeaponInstanceHasKeyword(instanceData, keyword))
			return true;
		return false;
	}

	TESAmmo* GetEquippedBaseCaliber(StaticFunctionTag*, Actor * owner)
	{
		TESObjectWEAP* weap = Utilities::GetEquippedWeapon(owner);
		BGSObjectInstanceExtra* moddata = Utilities::GetEquippedModData(owner);
		return MSF_Data::GetBaseCaliber(moddata, weap);
	}


	//=====Version=====
	UInt32 GetVersion(StaticFunctionTag* base)
	{
		return MSF_VERSION;
	}
}

bool MSF_Papyrus::RegisterPapyrus(VirtualMachine* vm)
{
	MSF_Papyrus::RegisterFuncs(vm);
	return true;
}

//GetAttachParentKeyword +WEAP
//GetAttachPointKeywords +WEAP
//GetModAtAttachPoint


void MSF_Papyrus::RegisterFuncs(VirtualMachine* vm)
{
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESAmmo*, Actor*, UInt32>("GetEquippedAmmo", SCRIPTNAME, GetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, TESAmmo*>("SetEquippedAmmo", SCRIPTNAME, SetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Actor*>("GetEquippedAmmoCapacity", SCRIPTNAME, GetEquippedAmmoCapacity, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("SetEquippedAmmoCapacity", SCRIPTNAME, SetEquippedAmmoCapacity, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, BGSProjectile*>("SetEquippedProjectile", SCRIPTNAME, SetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, BGSProjectile*, Actor*>("GetEquippedProjectile", SCRIPTNAME, GetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("SetLoadedAmmoCount", SCRIPTNAME, SetLoadedAmmoCount, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Actor*>("GetLoadedAmmoCount", SCRIPTNAME, GetLoadedAmmoCount, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, TESForm*>("SetEquippedZoomData", SCRIPTNAME, SetEquippedZoomData, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Actor*>("GetEquippedZoomData", SCRIPTNAME, GetEquippedZoomData, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("ToggleAutomaticWeapon", SCRIPTNAME, ToggleAutomaticWeapon, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, bool>("SetAutomaticWeapon", SCRIPTNAME, SetAutomaticWeapon, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, Actor*, UInt32>("GetEquippedStackCount", SCRIPTNAME, GetEquippedStackCount, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, UInt32, UInt32>("SetEquippedStackCount", SCRIPTNAME, SetEquippedStackCount, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, BGSMod::Attachment::Mod*, UInt32>("EquippedItemHasMod", SCRIPTNAME, EquippedItemHasMod, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESAmmo*, Actor*>("GetEquippedBaseCaliber", SCRIPTNAME, GetEquippedBaseCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, BGSKeyword*, UInt32>("EquippedItemHasKeyword", SCRIPTNAME, EquippedItemHasKeyword, vm));

	vm->RegisterFunction(
		new NativeFunction0 <StaticFunctionTag, UInt32>("GetVersion", SCRIPTNAME, GetVersion, vm));


	_MESSAGE("Native functions registered from MSF_Papyrus.cpp");
}