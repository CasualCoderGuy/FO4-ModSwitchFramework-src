#include "MSF_Papyrus.h"
#include "MSF_Data.h"
#include "MSF_Base.h"
#include "MSF_Events.h"

namespace MSF_Papyrus
{

	//============InstanceData===========
	TESAmmo * GetEquippedAmmo(StaticFunctionTag*, Actor* owner)
	{
		if (!owner)
			return nullptr;
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(owner, 41);
		if (instanceData) 
		{
			return instanceData->ammo;
		}
		return nullptr;
	}

	bool SetEquippedAmmo(StaticFunctionTag*, Actor* owner, TESAmmo* newAmmo)
	{
		if (!owner || !newAmmo)
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
		if (!owner || !newProjectile)
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

	UInt32 GetEquippedStackSize(StaticFunctionTag*, Actor* owner, UInt32 slotIndex)
	{
		if (!owner)
			return false;
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

	UInt32 GetStackSize(StaticFunctionTag*, TESObjectREFR* owner, TESForm* item, UInt32 stackID)
	{
		if (!owner || !item || !owner->inventoryList)
			return false;
		UInt32 count = 0;

		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			BGSInventoryItem::Stack* foundStack = Utilities::GetStack(&inventoryItem, stackID);
			if (foundStack)
				count = (UInt32)foundStack->count;
			break;
		}
		return count;
	}

	UInt32 GetStackCount(StaticFunctionTag*, TESObjectREFR* owner, TESForm* item)
	{
		if (!owner || !item || !owner->inventoryList)
			return false;
		UInt32 count = 0;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			for (BGSInventoryItem::Stack* currStack = inventoryItem.stack; currStack; currStack = currStack->next)
				count++;
			break;
		}
		return count;
	}


	bool AttachRemoveModToEquippedWeapon(StaticFunctionTag*, Actor* owner, BGSMod::Attachment::Mod* mod, bool bAttach, bool bUpdateAnimGraph)
	{
		if (!owner || !mod)
			return false;
		
		_DEBUG("attach, updAnim: %02X", bUpdateAnimGraph);
		return MSF_Base::AttachModToEquippedWeapon(owner, mod, bAttach, 2, bUpdateAnimGraph);
	}

	bool AttachRemoveModToEquippedItemPapyrus(StaticFunctionTag*, Actor* actor, UInt32 slotIndex, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph)
	{
		return MSF_Base::AttachRemoveModToEquippedItem(actor, slotIndex, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph);
	}

	bool AttachRemoveModToInventoryStackPapyrus(StaticFunctionTag*, TESObjectREFR* owner, TESForm* item, UInt32 stackID, BGSMod::Attachment::Mod* mod, bool bAttach, bool shouldSplitStacks, bool transferEquippedToSplitStack, bool updateAnimGraph)
	{
		TESBoundObject* tbo = DYNAMIC_CAST(item, TESForm, TESBoundObject);
		return MSF_Base::AttachRemoveModToInventoryStackPre(owner, tbo, stackID, mod, bAttach, shouldSplitStacks, transferEquippedToSplitStack, updateAnimGraph);
	}

	VMArray<TESForm*> GetAllInventoryItem(StaticFunctionTag*, TESObjectREFR* owner)
	{
		VMArray<TESForm*> result;
		if (!owner || !owner->inventoryList)
			return result;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (!inventoryItem.form || !inventoryItem.stack)
				continue;
			result.Push(&inventoryItem.form);
		}
		return result;
	}

	VMArray<TESObjectWEAP*> GetAllInventoryWeapon(StaticFunctionTag*, TESObjectREFR* owner)
	{
		VMArray<TESObjectWEAP*> result;
		if (!owner || !owner->inventoryList)
			return result;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (!inventoryItem.form || !inventoryItem.stack)
				continue;
			TESObjectWEAP* weap = DYNAMIC_CAST(inventoryItem.form, TESForm, TESObjectWEAP);
			if (!weap)
				continue;
			result.Push(&weap);
		}
		return result;
	}

	VMArray<TESObjectARMO*> GetAllInventoryArmor(StaticFunctionTag*, TESObjectREFR* owner)
	{
		VMArray<TESObjectARMO*> result;
		if (!owner || !owner->inventoryList)
			return result;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (!inventoryItem.form || !inventoryItem.stack)
				continue;
			TESObjectARMO* armor = DYNAMIC_CAST(inventoryItem.form, TESForm, TESObjectARMO);
			if (!armor)
				continue;
			result.Push(&armor);
		}
		return result;
	}

	VMArray<TESObjectREFR*> GetAllRefInCell(StaticFunctionTag*, TESObjectCELL* cell)
	{
		VMArray<TESObjectREFR*> result;
		if (!cell)
			return result;
		for (UInt32 i = 0; i < cell->objectList.count; i++)
		{
			TESObjectREFR* nextRef = cell->objectList[i];
			if (!nextRef)
				continue;
			result.Push(&nextRef);
		}
		return result;
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
		TESObjectWEAP* weap = Utilities::GetEquippedGun(owner);
		BGSObjectInstanceExtra* moddata = Utilities::GetEquippedModData(owner);
		return MSF_Data::GetBaseCaliber(moddata, weap);
	}

	BGSMod::Attachment::Mod* GetModAtAttachPoint(StaticFunctionTag*, TESObjectREFR* owner, TESForm* item, UInt32 stackID, BGSKeyword* ap_keyword)
	{
		BGSInventoryItem::Stack* stack = Utilities::GetStackFromItem(owner, item, stackID);
		if (!stack || !stack->extraData)
			return nullptr;
		BGSObjectInstanceExtra* modData = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		KeywordValue value = Utilities::GetAttachValueForTypedKeyword(ap_keyword);
		return Utilities::GetModAtAttachPoint(modData, value);
	}

	BGSKeyword* GetModAttachParent(StaticFunctionTag*, BGSMod::Attachment::Mod* mod)
	{
		return Utilities::GetAttachParent(mod);
	}

	bool ObjectHasAttachPoint(StaticFunctionTag*, TESForm* object, BGSKeyword* attachPointKW)
	{
		if (!object || !attachPointKW)
			return false;
		AttachParentArray* attachPoints = nullptr;
		TESObjectWEAP* weap = DYNAMIC_CAST(object, TESForm, TESObjectWEAP);
		if (weap)
			attachPoints = (AttachParentArray*)&weap->attachParentArray;
		else
		{
			BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(object, RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (mod)
				attachPoints = (AttachParentArray*)&mod->unk98;
			else
			{
				TESObjectARMO* armor = DYNAMIC_CAST(object, TESForm, TESObjectARMO);
				if (armor)
					attachPoints = (AttachParentArray*)&armor->parentArray;
				else
				{
					TESNPC* npc = DYNAMIC_CAST(object, TESForm, TESNPC);
					if (npc)
						attachPoints = (AttachParentArray*)&npc->attachParentArray;
					else
					{
						TESFurniture* furn = DYNAMIC_CAST(object, TESForm, TESFurniture);
						if (furn)
							attachPoints = (AttachParentArray*)&furn->unk160;
						else
						{
							TESRace* race = DYNAMIC_CAST(object, TESForm, TESRace);
							if (race)
								attachPoints = (AttachParentArray*)&race->unk388;

						}
					}
				}
			}
		}
		return Utilities::HasAttachPoint(attachPoints, attachPointKW);
	}

	VMArray<BGSMod::Attachment::Mod*> StackGetAllMods(StaticFunctionTag*, TESObjectREFR* owner, TESForm* item, UInt32 stackID)
	{
		VMArray<BGSMod::Attachment::Mod*> result;
		BGSInventoryItem::Stack* stack = Utilities::GetStackFromItem(owner, item, stackID);
		if (!stack || !stack->extraData)
			return result;
		BGSObjectInstanceExtra* modData = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		if (!modData)
			return result;
		auto data = modData->data;
		if (!data || !data->forms)
			return result;

		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (objectMod)
				result.Push(&objectMod);
		}
		return result;
	}

	TESObjectWEAP* GetEquippedWeapon(StaticFunctionTag*, Actor* ownerActor)
	{
		return Utilities::GetEquippedWeapon(ownerActor);
	}



	//GetModTarget
	//getAttachpoints
	//equip

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
		new NativeFunction1 <StaticFunctionTag, TESAmmo*, Actor*>("GetEquippedAmmo", SCRIPTNAME, GetEquippedAmmo, vm));
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
		new NativeFunction2 <StaticFunctionTag, UInt32, Actor*, UInt32>("GetEquippedStackSize", SCRIPTNAME, GetEquippedStackSize, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, TESObjectREFR*, TESForm*, UInt32>("GetStackSize", SCRIPTNAME, GetStackSize, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, TESObjectREFR*, TESForm*>("GetStackCount", SCRIPTNAME, GetStackCount, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, BGSMod::Attachment::Mod*, UInt32>("EquippedItemHasMod", SCRIPTNAME, EquippedItemHasMod, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESAmmo*, Actor*>("GetEquippedBaseCaliber", SCRIPTNAME, GetEquippedBaseCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, Actor*, BGSKeyword*, UInt32>("EquippedItemHasKeyword", SCRIPTNAME, EquippedItemHasKeyword, vm));
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, bool, Actor*, BGSMod::Attachment::Mod*, bool, bool>("AttachRemoveModToEquippedWeapon", SCRIPTNAME, AttachRemoveModToEquippedWeapon, vm));
	vm->RegisterFunction(
		new NativeFunction7 <StaticFunctionTag, bool, Actor*, UInt32, BGSMod::Attachment::Mod*, bool, bool, bool, bool>("AttachRemoveModToEquippedItem", SCRIPTNAME, AttachRemoveModToEquippedItemPapyrus, vm));
	vm->RegisterFunction(
		new NativeFunction8 <StaticFunctionTag, bool, TESObjectREFR*, TESForm*, UInt32, BGSMod::Attachment::Mod*, bool, bool, bool, bool>("AttachRemoveModToInventoryStack", SCRIPTNAME, AttachRemoveModToInventoryStackPapyrus, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<TESForm*>, TESObjectREFR*>("GetAllInventoryItem", SCRIPTNAME, GetAllInventoryItem, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<TESObjectWEAP*>, TESObjectREFR*>("GetAllInventoryWeapon", SCRIPTNAME, GetAllInventoryWeapon, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<TESObjectARMO*>, TESObjectREFR*>("GetAllInventoryArmor", SCRIPTNAME, GetAllInventoryArmor, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<TESObjectREFR*>, TESObjectCELL*>("GetAllRefInCell", SCRIPTNAME, GetAllRefInCell, vm));
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, BGSMod::Attachment::Mod*, TESObjectREFR*, TESForm*, UInt32, BGSKeyword*>("GetModAtAttachPoint", SCRIPTNAME, GetModAtAttachPoint, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, VMArray<BGSMod::Attachment::Mod*>, TESObjectREFR*, TESForm*, UInt32>("StackGetAllMods", SCRIPTNAME, StackGetAllMods, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, TESForm*, BGSKeyword*>("ObjectHasAttachPoint", SCRIPTNAME, ObjectHasAttachPoint, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, BGSKeyword*, BGSMod::Attachment::Mod*>("GetModAttachParent", SCRIPTNAME, GetModAttachParent, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESObjectWEAP*, Actor*>("GetEquippedWeapon", SCRIPTNAME, GetEquippedWeapon, vm));

	vm->RegisterFunction(
		new NativeFunction0 <StaticFunctionTag, UInt32>("GetVersion", SCRIPTNAME, GetVersion, vm));


	_MESSAGE("Native functions registered from MSF_Papyrus.cpp");
}