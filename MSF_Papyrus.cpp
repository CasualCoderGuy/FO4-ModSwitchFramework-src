#include "MSF_Papyrus.h"

namespace MSF_Papyrus
{

	bool UpdateEquipment(StaticFunctionTag*, Actor * actor)
	{
		if (!actor)
			return false;
		//actor->middleProcess->unk08->lock.Release();
		CALL_MEMBER_FN(actor, UpdateEquipment)();
		//CALL_MEMBER_FN(actor->middleProcess, UpdateEquipment)(actor, actor->flags); //flags?
		//actor->middleProcess->unk08->lock.Lock(3);
		return true;
	}

	bool UpdateEquippedInstanceData(StaticFunctionTag*, Actor* actor, UInt32 newLoadedAmmoNum = -1)
	{
		if (!actor)
			return false;
		if (newLoadedAmmoNum == -1)
			newLoadedAmmoNum = (UInt32)actor->middleProcess->unk08->equipData->equippedData->unk18;
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(actor, 41);
		if (!eqStack)
			return false;
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(eqStack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData)
			return false;
		actor->middleProcess->unk08->equipData->instanceData = extraInstanceData->instanceData;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData)
			return false;
		actor->middleProcess->unk08->equipData->equippedData->ammo = instanceData->ammo;
		if (newLoadedAmmoNum == -2)
		{
			TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(actor, 41);
			if (instanceData)
			{
				newLoadedAmmoNum = (UInt32)instanceData->ammoCapacity;
			}
		}
		actor->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)newLoadedAmmoNum;
		return true;
	}

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

	bool SetEquippedAmmo(StaticFunctionTag*, Actor* owner, TESAmmo* newAmmo, UInt32 slotID = 41) //instant
	{
		if (!owner)
			return false;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
		if (instanceData) 
		{
			if (instanceData->ammo != newAmmo && owner->middleProcess->unk08->equipData->equippedData) 
			{
				instanceData->ammo = newAmmo;
				owner->middleProcess->unk08->equipData->equippedData->ammo = newAmmo;
				//owner->middleProcess->unk08->equipData->equippedData->unk18 = 0;
				return true;
			}
		}
		return false;
	}

	UInt32 GetEquippedAmmoCapacity(StaticFunctionTag*, Actor* owner, UInt32 slotID = 41)
	{
		if (!owner)
			return 0;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
		if (instanceData) 
		{
			return instanceData->ammoCapacity;
		}
		return 0;
	}

	bool SetEquippedAmmoCapacity(StaticFunctionTag*, Actor* owner, UInt32 newCap, UInt32 slotID = 41)
	{
		if (!owner || newCap < 0)
			return 0;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
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

	BGSProjectile * GetEquippedProjectile(StaticFunctionTag*, Actor* owner, UInt32 slotID = 41)
	{
		if (!owner)
			return nullptr;
		TESObjectWEAP::InstanceData * instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
		if (instanceData) 
		{
			if (instanceData->firingData) 
			{
				return instanceData->firingData->projectileOverride;
			}
		}
		return nullptr;
	}

	bool SetEquippedProjectile(StaticFunctionTag*, Actor * owner, BGSProjectile* newProjectile, UInt32 slotID = 41)
	{
		if (!owner)
			return false;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
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
		if (owner->middleProcess && owner->middleProcess->unk08->equipData && owner->middleProcess->unk08->equipData->equippedData) 
		{
			return (UInt32)owner->middleProcess->unk08->equipData->equippedData->unk18;
		}
		return 0;
	}

	bool SetLoadedAmmoCount(StaticFunctionTag*, Actor* owner, UInt32 amount = 0)
	{
		if (!owner || amount < 0)
			return false;
		if (owner->middleProcess && owner->middleProcess->unk08->equipData && owner->middleProcess->unk08->equipData->equippedData)
		{
			owner->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)amount;
			return true;
		}
		return false;
	}

	TESForm* GetEquippedZoomData(StaticFunctionTag*, Actor* owner, UInt32 slotID = 41)
	{
		if (!owner)
			return nullptr;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
		if (instanceData)
		{
			if (instanceData->zoomData)
				return (TESForm*)instanceData->zoomData;
		}
		return nullptr;
	}

	bool SetEquippedZoomData(StaticFunctionTag*, Actor* owner, TESForm* zoomData, UInt32 slotID = 41)
	{
		if (!owner || !zoomData)
			return nullptr;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, slotID);
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

	bool ToggleAutomaticWeapon(StaticFunctionTag*, Actor* owner) //soundmapping!
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
			//if (instanceData->flags & 0x0008000)
			//	instanceData->flags &= ~0x0008000;
			//else
			//	instanceData->flags |= 0x0008000;
			return true;
		}
		return false;
	}

	bool SetAutomaticWeapon(StaticFunctionTag*, Actor* owner, bool bAuto) //soundmapping!
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
		if (slotIndex >= ActorEquipData::kMaxSlots)
			return 0;
		if (!owner->equipData)
			return 0;
		auto item = owner->equipData->slots[slotIndex].item;
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
		if (slotIndex >= ActorEquipData::kMaxSlots || count < 0)
			return false;
		if (!owner->equipData)
			return false;
		auto item = owner->equipData->slots[slotIndex].item;
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

	TESAmmo* GetEquippedBaseCaliber(StaticFunctionTag*, Actor * owner, UInt32 slotID = 41)
	{
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(owner, slotID);
		if (!stack)
			return nullptr;
		BGSObjectInstanceExtra* objectModData = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;

		if (!instanceData || !objectModData || !weapBase)
			return nullptr;

		auto data = objectModData->data;
		if (!data || !data->forms)
			return nullptr;
		//TESAmmo* ammo = nullptr;
		TESAmmo* ammoConverted = nullptr;
		//TESAmmo* switchedAmmo = nullptr;
		//TESAmmo* UBAmmo = nullptr;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			for (UInt32 i4 = 0; i4 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i4];
				if (data->target == 61 && data->value.form)
				{
					if (objectMod->priority < 123 || objectMod->priority > 127)
						ammoConverted = (TESAmmo*)data->value.form;
					//else if (objectMod->priority == 125)
					//	switchedAmmo = (TESAmmo*)data->value.form;
					//else if (objectMod->priority == 126)
					//	UBAmmo = (TESAmmo*)data->value.form;
				}
			}
		}
		if (!ammoConverted)
			ammoConverted = weapBase->weapData.ammo;
		return ammoConverted;
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


void MSF_Papyrus::RegisterFuncs(VirtualMachine* vm)
{
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("UpdateEquipment", SCRIPTNAME, UpdateEquipment, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("UpdateEquippedInstanceData", SCRIPTNAME, UpdateEquippedInstanceData, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESAmmo*, Actor*, UInt32>("GetEquippedAmmo", SCRIPTNAME, GetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, TESAmmo*, UInt32>("SetEquippedAmmo", SCRIPTNAME, SetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, Actor*, UInt32>("GetEquippedAmmoCapacity", SCRIPTNAME, GetEquippedAmmoCapacity, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, UInt32, UInt32>("SetEquippedAmmoCapacity", SCRIPTNAME, SetEquippedAmmoCapacity, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, BGSProjectile*, UInt32>("SetEquippedProjectile", SCRIPTNAME, SetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSProjectile*, Actor*, UInt32>("GetEquippedProjectile", SCRIPTNAME, GetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("SetLoadedAmmoCount", SCRIPTNAME, SetLoadedAmmoCount, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Actor*>("GetLoadedAmmoCount", SCRIPTNAME, GetLoadedAmmoCount, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, TESForm*, UInt32>("SetEquippedZoomData", SCRIPTNAME, SetEquippedZoomData, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESForm*, Actor*, UInt32>("GetEquippedZoomData", SCRIPTNAME, GetEquippedZoomData, vm));
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
		new NativeFunction2 <StaticFunctionTag, TESAmmo*, Actor*, UInt32>("GetEquippedBaseCaliber", SCRIPTNAME, GetEquippedBaseCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, BGSKeyword*, UInt32>("EquippedItemHasKeyword", SCRIPTNAME, EquippedItemHasKeyword, vm));

	vm->RegisterFunction(
		new NativeFunction0 <StaticFunctionTag, UInt32>("GetVersion", SCRIPTNAME, GetVersion, vm));


	_MESSAGE("Native functions registered from MSF_Papyrus.cpp");
}