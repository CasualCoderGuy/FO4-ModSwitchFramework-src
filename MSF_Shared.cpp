#include "MSF_Shared.h"


BSExtraData::~BSExtraData() {};
void BSExtraData::Unk_01() {};
void BSExtraData::Unk_02() {};

CheckStackIDFunctor::CheckStackIDFunctor(UInt32 ID)
{
	vtbl = g_CheckStackIDFunctor;
	stackID = ID;
}

ModifyModDataFunctor::ModifyModDataFunctor(BGSMod::Attachment::Mod* mod, UInt8 slotIndex, bool bAttach, bool* success) :
	mod(mod),
	success(success),
	slotIndex(slotIndex),
	attach(bAttach)
{
	vtbl = g_ModifyModDataFunctor;
	if (success) {
		*success = true;
	}
}
ApplyChangesFunctor::ApplyChangesFunctor(TESBoundObject* foundObject, BGSObjectInstanceExtra* moddata, BGSMod::Attachment::Mod* mod, bool ignoreWeapon, bool remove, bool equipLocked, UInt8 setExtraData) :
	moddata(moddata),
	foundObject(foundObject),
	mod(mod),
	ignoreWeapon(ignoreWeapon),
	remove(remove),
	equipLocked(equipLocked),
	setExtraData(setExtraData)
{
	vtbl = g_ApplyChangesFunctor;
}


SplitStackFunctor::SplitStackFunctor(bool transferEquipped, UInt32 newCount, BGSInventoryItem::Stack* oldStack)
{
	transferEquippedToSplitStack = transferEquipped;
	stackCount = newCount;
	stack = oldStack;
}

bool SplitStackFunctor::Apply(TESBoundObject* item, BGSInventoryItem::Stack* newStack)
{
	if (!item || !newStack || this->stackCount == 0)
		return false;
	if (this->stack)
		this->stack->count -= this->stackCount + newStack->count;
	newStack->count = this->stackCount;
	if (this->preventUnequip)
	{
		if (this->transferEquippedToSplitStack && this->stack)
			this->stack->flags |= 0x0;
		else
			newStack->flags |= 0x0;
	}
}


UInt32 roundp(float a)
{
	int result = (int)a;
	if (abs(a - result) >= 0.5)
	{
		if (result > 0)
			return (UInt32)result + 1;
		else if (result < 0)
			return (UInt32)result - 1;
	}
	else
		return (UInt32)result;
	return 0;
};

UInt32 inttover(UInt32 input)
{
	return (input % 10) + ((input / 10 % 10) << 4) + ((input / 100 % 10) << 16) + ((input / 1000 % 10) << 24);
}

BGSObjectInstanceExtra* CreateObjectInstanceExtra(BGSObjectInstanceExtra::Data* data)
{
	BGSObjectInstanceExtra* pInstanceData = (BGSObjectInstanceExtra*)BSExtraData::Create(sizeof(BGSObjectInstanceExtra), s_BGSObjectInstanceExtraVtbl.GetUIntPtr());
	pInstanceData->type = kExtraData_ObjectInstance;
	pInstanceData->data = data;
	return pInstanceData;
}

ExtraUniqueID* CreateExtraUniqueID(UInt16 id, UInt32 form)
{
	ExtraUniqueID* extraID = (ExtraUniqueID*)BSExtraData::Create(sizeof(ExtraUniqueID), s_BGSObjectInstanceExtraVtbl.GetUIntPtr());
	extraID->type = ExtraDataType::kExtraData_UniqueID;
	extraID->uniqueId = id;
	extraID->unk1A = form >> 16;
	extraID->formOwner = form & 0xFFFF;
	return extraID;
}

ExtraModRank* ExtraModRank::Create(UInt32 modrank)
{
	ExtraModRank* extraModRank = (ExtraModRank*)BSExtraData::Create(sizeof(ExtraModRank), s_ExtraModRankVtbl.GetUIntPtr());
	extraModRank->type = ExtraDataType::kExtraData_ModRank;
	extraModRank->rank = modrank;
	return extraModRank;
}

ExtraRank* ExtraRank::Create(UInt32 rank)
{
	ExtraRank* extraRank = (ExtraRank*)BSExtraData::Create(sizeof(ExtraRank), s_ExtraRankVtbl.GetUIntPtr());
	extraRank->type = ExtraDataType::kExtraData_Rank;
	extraRank->rank = rank;
	return extraRank;
}

ExtraAmmo* ExtraAmmo::Create(UInt32 ammo)
{
	ExtraAmmo* extraAmmo = (ExtraAmmo*)BSExtraData::Create(sizeof(ExtraAmmo), s_ExtraAmmoVtbl.GetUIntPtr());
	extraAmmo->type = ExtraDataType::kExtraData_Ammo;
	extraAmmo->ammo = ammo;
	return extraAmmo;
}

void FreeLeveledList(LEVELED_OBJECT* levo, uint32_t arg = 0x3)
{
	if (!levo)
		return;
	//using func_t = decltype(&FreeLeveledList);
	//const REL::Relocation<func_t> func{ REL::ID(296092) };  //140042CA0
	//return func(levo, arg);
	Heap_Free(levo);
}

struct_ll* reserveNewLevL(uint32_t count)
{
	//RE::MemoryManager mm = RE::MemoryManager::GetSingleton();
	//return (struct_ll*)mm.Allocate(sizeof(LEVELED_OBJECT) * count + sizeof(size_t), 0, 0);
	void* memory = Heap_Allocate(sizeof(LEVELED_OBJECT) * count + sizeof(size_t));
	memset(memory, 0, sizeof(LEVELED_OBJECT) * count + sizeof(size_t));
	return (struct_ll*)memory;
}

std::vector<LEVELED_OBJECT> LeveledList::getListAsVector()
{
	std::vector<LEVELED_OBJECT> loVec;
	if (!this->leveledLists || this->baseListCount == 0)
		return loVec;
	for (uint32_t i = 0; i < static_cast<uint32_t>(this->baseListCount); i++)
		loVec.push_back(this->leveledLists[i]);
	return loVec;
}

bool LeveledList::CreateList(const std::vector<LEVELED_OBJECT>& levo)
{
	if (this->leveledLists) 
	{
		memset(this->leveledLists, 0, this->baseListCount * sizeof(LEVELED_OBJECT));
		FreeLeveledList(this->leveledLists);

		this->leveledLists = nullptr;
		this->baseListCount = 0;
	}

	if (levo.empty())
		return false;

	uint32_t size = static_cast<uint32_t>(levo.size());
	struct_ll* newLL = reserveNewLevL(size);
	if (!newLL)
		return false;

	newLL->size = size;
	for (uint32_t i = 0; i < size; i++)
		newLL->ll[i] = levo[i];

	this->leveledLists = newLL->ll;
	this->baseListCount = static_cast<int8_t>(size);
	return true;
}

bool LeveledList::CopyData(LeveledList* originalll, bool doMiscData, bool copyList)
{
	if (doMiscData)
	{
		if (!this->leveledLists && !copyList)
		{
			this->baseListCount = originalll->baseListCount;
			this->leveledLists = originalll->leveledLists;
		}
		this->chanceGlobal = originalll->chanceGlobal;
		this->chanceNone = originalll->chanceNone;
		this->keywordChances = originalll->keywordChances;
		this->llFlags = originalll->llFlags;
		this->maxUseAllCount = originalll->maxUseAllCount;
		if (!this->scriptAddedLists && !copyList)
		{
			this->scriptAddedLists = originalll->scriptAddedLists;
			this->scriptListCount = originalll->scriptListCount;
		}
	}
	if (copyList)
	{
		if (this->leveledLists)
		{
			memset(this->leveledLists, 0, this->baseListCount * sizeof(LEVELED_OBJECT));
			FreeLeveledList(this->leveledLists);
			this->leveledLists = nullptr;
			this->baseListCount = 0;
		}
		if (originalll->baseListCount > 0)
		{
			struct_ll* newLL = reserveNewLevL(originalll->baseListCount);
			if (newLL)
			{
				newLL->size = originalll->baseListCount;
				for (uint32_t i = 0; i < originalll->baseListCount; i++)
					newLL->ll[i] = originalll->leveledLists[i];
				this->leveledLists = newLL->ll;
				this->baseListCount = originalll->baseListCount;
			}
		}
	}

	return true;
}

void LeveledList::ClearList()
{
	if (this->leveledLists) 
	{
		memset(this->leveledLists, 0, this->baseListCount * sizeof(LEVELED_OBJECT));
		FreeLeveledList(this->leveledLists);

		this->leveledLists = nullptr;
		this->baseListCount = 0;
	}
}

namespace Utilities
{

	void SendNotification(std::string asNotificationText)
	{
		ShowNotification(asNotificationText.c_str(), 0, 1);
	}

	//void ShowMessagebox(std::string asText) {
	//	CallGlobalFunctionNoWait1<BSFixedString>("Debug", "Messagebox", BSFixedString(asText.c_str()));
	//}

	void AddItem(TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent)
	{
		if (!target || !form || count < 1)
			return;
		unkItemStruct item;
		item.item = form;
		item.unk08 = 0;
		AddItemNative((*g_gameVM)->m_virtualMachine, 0, target, item, count, bSilent);
	}

	void RemoveItem(TESObjectREFR* target, TESForm* form, SInt32 count, bool bSilent, TESObjectREFR* toContainer)
	{
		if (!target || !form || count < 1)
			return;
		unkItemStruct item;
		item.item = form;
		item.unk08 = 0;
		RemoveItemNative((*g_gameVM)->m_virtualMachine, 0, target, item, count, bSilent, toContainer);
	}

	void AttachModToInventoryItem(TESObjectREFR* objRef, TESForm* invItem, BGSMod::Attachment::Mod* mod)
	{
		if (!objRef || !invItem || !mod)
			return;
		AttachModToInventoryItem_Internal((*g_gameVM)->m_virtualMachine, 0, objRef, invItem, mod, true);
	}

	TESForm* GetFormFromIdentifier(const std::string& identifier)
	{
		auto delimiter = identifier.find('|');
		if (delimiter != std::string::npos) 
		{
			std::string modName = identifier.substr(0, delimiter);
			std::string modForm = identifier.substr(delimiter + 1);

			UInt32 formID = 0;
			UInt8 espModIndex = (*g_dataHandler)->GetLoadedModIndex(modName.c_str());
			//if (espModIndex == (UInt8)-1)
			//	return false;
			if (espModIndex != 0xFF)
			{
				formID = ((UInt32)espModIndex) << 24;
			}
			else
			{
				UInt16 eslModIndex = (*g_dataHandler)->GetLoadedLightModIndex(modName.c_str());
				if (eslModIndex != 0xFFFF)
				{
					formID = 0xFE000000 | (UInt32(eslModIndex) << 12);
				}
				else
					return nullptr;
			}
			formID |= std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
			return LookupFormByID(formID);
		}
		return nullptr;
	}

	const char* GetIdentifierFromForm(TESForm* form)
	{
		if (!form)
			return "";
		UInt32 formID = form->formID;
		if ((formID & 0xFF000000) == 0xFE000000)
		{
			std::ostringstream str;
			UInt8 modIndex = (UInt8)(formID >> 12 & 0xFFF);
			str << (*g_dataHandler)->modList.lightMods[modIndex]->name << '|';
			formID &= 0xFFF;
			str << std::hex << formID;
			return str.str().c_str();
		}
		else if ((formID & 0xFF000000) != 0xFF000000)
		{
			std::ostringstream str;
			UInt8 modIndex = (UInt8)(formID >> 24);
			str << (*g_dataHandler)->modList.loadedMods[modIndex]->name << '|';
			formID &= 0xFFFFFF;
			str << std::hex << formID;
			return str.str().c_str();
		}
		return "";
	}

	bool AddToFormList(BGSListForm* flst, TESForm* form, SInt64 idx)
	{
		if (!flst || !form)
			return false;
		if (idx >= 0 && idx <= flst->forms.count)
			flst->forms.Insert(idx & 0xFFFFFFFF, form);
		else
			flst->forms.Push(form);
		return true;
	}

	UInt32 GetLoadedAmmoCount(Actor* owner)
	{
		if (!owner)
			return 0;
		if (owner->middleProcess && owner->middleProcess->unk08->equipData.entries && owner->middleProcess->unk08->equipData[0].equippedData)
			return (UInt32)owner->middleProcess->unk08->equipData[0].equippedData->unk18;
		return 0;
	}

	TESObjectWEAP::InstanceData* GetEquippedInstanceData(Actor * ownerActor, UInt32 iEquipSlot)
	{
		if (ownerActor) 
		{
			if (iEquipSlot >= BIPOBJECT::BIPED_OBJECT::kTotal)
				return nullptr;
			//BipedAnim * equipData = ownerActor->equipData;
			//if (!equipData)
			//	return nullptr;
			//auto item = equipData->object[iEquipSlot].parent.object;
			//if (!item)
			//	return nullptr;
			//return (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(equipData->object[iEquipSlot].parent.instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
			BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(ownerActor, iEquipSlot);
			if (!eqStack)
				return nullptr;
			ExtraDataList* dataList = eqStack->extraData;
			if (!dataList)
				return nullptr;
			ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
			return (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		}
		return nullptr;
	}

	BGSObjectInstanceExtra* GetEquippedModData(Actor * ownerActor, UInt32 iEquipSlot)
	{
		if (ownerActor)
		{
			if (iEquipSlot >= BIPOBJECT::BIPED_OBJECT::kTotal)
				return nullptr;
			//ActorEquipData * equipData = ownerActor->equipData;
			//if (!equipData)
			//	return nullptr;
			//auto item = equipData->object[iEquipSlot].parent.object;
			//if (!item)
			//	return nullptr;
			//return equipData->object[iEquipSlot].extraData;
			BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(ownerActor, iEquipSlot);
			if (!eqStack)
				return nullptr;
			ExtraDataList* dataList = eqStack->extraData;
			if (!dataList)
				return nullptr;
			BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
			return DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
		}
		return nullptr;
	}

	BGSObjectInstanceExtra* GetEquippedWeaponModData(Actor* ownerActor)
	{
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedWeaponStack(ownerActor);
		if (!eqStack)
			return nullptr;
		ExtraDataList* dataList = eqStack->extraData;
		if (!dataList)
			return nullptr;
		BSExtraData* extraMods = dataList->GetByType(kExtraData_ObjectInstance);
		if (!extraMods)
			return nullptr;
		return DYNAMIC_CAST(extraMods, BSExtraData, BGSObjectInstanceExtra);
	}

	TESObjectWEAP* GetEquippedGun(Actor* ownerActor)
	{
		if (ownerActor)
		{
			BipedAnim * equipData = ownerActor->biped.get();
			if (!equipData)
				return nullptr;
			auto item = equipData->object[41].parent.object;
			if (!item)
				return nullptr;
			return DYNAMIC_CAST(item, TESForm, TESObjectWEAP);
		}
		return nullptr;
	}

	TESObjectWEAP* GetEquippedWeapon(Actor* ownerActor)
	{
		//if (!ownerActor || !ownerActor->middleProcess || !ownerActor->middleProcess->unk08 || !ownerActor->middleProcess->unk08->equipData.entries)
		//	return nullptr;
		BipedAnim* equipData = ownerActor->biped.get();
		if (!equipData)
			return nullptr;
		auto item = equipData->object[41].parent.object;
		if (!item)
		{
			item = equipData->object[33].parent.object;
			if (!item)
			{
				item = equipData->object[37].parent.object;
				if (!item)
					item = equipData->object[32].parent.object;
			}
		}
		if (!item)
			return nullptr;
		return DYNAMIC_CAST(item, TESForm, TESObjectWEAP);
	}

	UInt8 GetEquippedWeaponSlotIndex(Actor* ownerActor)
	{
		UInt8 idx = -1;
		BipedAnim* equipData = ownerActor->biped.get();
		if (!equipData)
			return idx;
		if (equipData->object[41].parent.object)
			return 41;
		else if (equipData->object[33].parent.object)
			return 33;
		else if (equipData->object[37].parent.object)
			return 37;
		else if (equipData->object[32].parent.object)
			return 32;
		else
			return idx;
	}

	UInt32 GetEquippedItemFormID(Actor * owner, UInt32 slotIndex)
	{
		if (owner) 
		{
			if (slotIndex >= BIPOBJECT::BIPED_OBJECT::kTotal)
				return 0x0;
			BipedAnim* equipData = owner->biped.get();
			if (!equipData)
				return 0x0;
			TESForm *item = equipData->object[slotIndex].parent.object;
			if (!item)
				return 0x0;
			return item->formID;
		}
		return 0x0;
	}

	BGSInventoryItem::Stack* GetEquippedStack(Actor* owner, UInt32 slotIndex)
	{
		BGSInventoryItem::Stack* eqStack = nullptr;
		if (slotIndex >= BIPOBJECT::BIPED_OBJECT::kTotal)
			return nullptr;
		if (!owner)
			return nullptr;
		if (!owner->biped.get())
			return nullptr;
		auto item = owner->biped.get()->object[slotIndex].parent.object;
		if (!item)
			return nullptr;
		if (!owner->inventoryList)
			return nullptr;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form != item || !inventoryItem.stack)
				continue;
			bool ret = inventoryItem.stack->Visit([&](BGSInventoryItem::Stack * stack)
			{
				if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
				{
					eqStack = stack;
					return false;
				}

				return true;
			});
			if (!ret)
				break;
		}

		return eqStack;
	}

	BGSInventoryItem::Stack* GetEquippedWeaponStack(Actor* owner)
	{
		BGSInventoryItem::Stack* eqStack = nullptr;
		auto item = Utilities::GetEquippedWeapon(owner);
		if (!item)
			return nullptr;
		if (!owner->inventoryList)
			return nullptr;
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
						eqStack = stack;
						return false;
					}
					return true;
				});
			if (!ret)
				break;
		}
		return eqStack;
	}

	UInt32 GetStackID(BGSInventoryItem* item, BGSInventoryItem::Stack* stack) //equipped stack ID should always be 0 but check nevertheless
	{
		UInt32 IDcount = 0;
		for (BGSInventoryItem::Stack* currStack = item->stack; currStack; currStack = currStack->next)
		{
			if (currStack == stack)
				return IDcount;
			IDcount++;
		}
		return -1;
	}

	BGSInventoryItem::Stack* GetStack(BGSInventoryItem* item, UInt32 stackID)
	{
		if (!item)
			return nullptr;
		UInt32 IDcount = 0;
		for (BGSInventoryItem::Stack* currStack = item->stack; currStack; currStack = currStack->next)
		{
			if (IDcount == stackID)
				return currStack;
			IDcount++;
		}
		return nullptr;
	}

	BGSInventoryItem::Stack* GetStackFromItem(TESObjectREFR* owner, TESForm* item, UInt32 stackID)
	{
		if (!item || !owner)
			return nullptr;
		if (!owner->inventoryList)
			return nullptr;
		BGSInventoryItem inventoryItem;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form == item && inventoryItem.stack)
				return GetStack(&inventoryItem, stackID);
		}
		return nullptr;
	}

	UInt64 GetInventoryItemCount(BGSInventoryList* inventory, TESForm* item)
	{
		if (!inventory || !item)
			return 0;
		UInt64 count = 0;
		for (UInt32 i = 0; i < inventory->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			inventory->items.GetNthItem(i, inventoryItem);
			if (item != inventoryItem.form || !inventoryItem.stack)
				continue;
			for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
				count += stack->count;
		}
		return count;
	}

	EquipWeaponData* GetEquippedWeaponData(Actor* owner)
	{
		if (!owner || !owner->middleProcess || !owner->middleProcess->unk08 || !owner->middleProcess->unk08->equipData.entries)
			return nullptr;
		return (EquipWeaponData*)owner->middleProcess->unk08->equipData[0].equippedData;
	}

	TESObjectMISC* GetLooseMod(BGSMod::Attachment::Mod* thisMod)
	{
		auto pair = g_modAttachmentMap->Find(&thisMod);
		if (pair) {
			return pair->miscObject;
		}
		return nullptr;
	}

	BGSMod::Attachment::Mod* FindModByUniqueKeyword(BGSObjectInstanceExtra* modData, BGSKeyword* keyword)
	{
		if (!modData)
			return nullptr;
		auto data = modData->data;
		if (!data || !data->forms)
			return nullptr;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			for (UInt32 i4 = 0; i4 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i4];
				if (data->target == 31 && data->value.form)
				{
					BGSKeyword* modKeyword = (BGSKeyword*)data->value.form;
					if (modKeyword == keyword)
						return objectMod;
				}
			}
		}
		return nullptr;
	}

	std::vector<BGSMod::Attachment::Mod*> FindModsByUniqueKeyword(BGSObjectInstanceExtra* modData, BGSKeyword* keyword)
	{
		std::vector<BGSMod::Attachment::Mod*> foundMods;
		if (!modData)
			return foundMods;
		auto data = modData->data;
		if (!data || !data->forms)
			return foundMods;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			for (UInt32 i4 = 0; i4 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i4];
				if (data->target == 31 && data->value.form)
				{
					BGSKeyword* modKeyword = (BGSKeyword*)data->value.form;
					if (modKeyword == keyword)
						foundMods.push_back(objectMod);
				}
			}
		}
		return foundMods;
	}

	BGSMod::Attachment::Mod* GetFirstModWithPriority(BGSObjectInstanceExtra* modData, UInt8 priority)
	{
		if (!modData)
			return nullptr;
		auto data = modData->data;
		if (!data || !data->forms)
			return nullptr;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (objectMod->priority == priority)
				return objectMod;
		}
		return nullptr;
	}

	bool HasObjectMod(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod)
	{
		if (!modData || !mod)
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

	BGSKeyword* GetAttachParent(BGSMod::Attachment::Mod* mod)
	{
		if (!mod)
			return nullptr;
		return GetKeywordFromValueArray(AttachParentArray::iDataType, mod->unkC0);
	}

	bool GetParentMods(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod, std::vector<BGSMod::Attachment::Mod*>* parents)
	{
		if (!modData || !mod || !parents)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		parents->clear();
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (!objectMod || objectMod == mod)
				continue;
			AttachParentArray* attachPoints = reinterpret_cast<AttachParentArray*>(&objectMod->unk98);
			if (attachPoints->kewordValueArray.GetItemIndex(mod->unkC0) >= 0)
				parents->push_back(objectMod);
		}
		return true;
	}

	BGSMod::Attachment::Mod* GetParentMod(BGSObjectInstanceExtra* modData, BGSMod::Attachment::Mod* mod)
	{
		BGSMod::Attachment::Mod* parentMod = nullptr;
		if (!modData || !mod)
			return parentMod;
		auto data = modData->data;
		if (!data || !data->forms)
			return parentMod;
		UInt64 priority = 0;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (!objectMod || objectMod == mod)
				continue;
			UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
			if (currPriority < priority)
				continue;
			AttachParentArray* attachPoints = reinterpret_cast<AttachParentArray*>(&objectMod->unk98);
			if (attachPoints->kewordValueArray.GetItemIndex(mod->unkC0) >= 0)
			{
				parentMod = objectMod;
				priority = currPriority;
			}
		}
		return parentMod;
	}

	KeywordValue GetAttachValueForTypedKeyword(BGSKeyword* keyword)
	{
		if (!keyword)
			return -1;
		return g_AttachPointKeywordArray->GetItemIndex(keyword);
	}

	KeywordValue GetInstantiationValueForTypedKeyword(BGSKeyword* keyword)
	{
		if (!keyword)
			return -1;
		return g_InstantiationKeywordArray->GetItemIndex(keyword);
	}

	KeywordValue GetAnimFlavorValueForTypedKeyword(BGSKeyword* keyword)
	{
		if (!keyword)
			return -1;
		return g_AnimFlavorKeywordArray->GetItemIndex(keyword);
	}

	bool HasAttachPoint(AttachParentArray* attachPoints, BGSKeyword* attachPointKW)
	{
		if (!attachPoints || !attachPointKW)
			return false;
		for (UInt32 i = 0; i < attachPoints->kewordValueArray.count; i++)
		{
			KeywordValue value = attachPoints->kewordValueArray[i];
			BGSKeyword* keyword = GetKeywordFromValueArray(AttachParentArray::iDataType, value);
			if (keyword == attachPointKW)
				return true;
		}
		return false;
	}

	bool ObjectInstanceHasAttachPoint(BGSObjectInstanceExtra* modData, BGSKeyword* attachPointKW)
	{
		if (!modData || !attachPointKW)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (!objectMod)
				continue;
			AttachParentArray* attachPoints = reinterpret_cast<AttachParentArray*>(&objectMod->unk98);
			for (UInt32 i = 0; i < attachPoints->kewordValueArray.count; i++)
			{
				KeywordValue value = attachPoints->kewordValueArray[i];
				BGSKeyword* keyword = GetKeywordFromValueArray(AttachParentArray::iDataType, value);
				if (keyword == attachPointKW)
					return true;
			}
		}
		return false;
	}

	BGSMod::Attachment::Mod* GetModAtAttachPoint(BGSObjectInstanceExtra* modData, KeywordValue value)
	{
		if (!modData)
			return nullptr;
		auto data = modData->data;
		if (!data || !data->forms)
			return nullptr;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (!objectMod)
				continue;
			if (objectMod->unkC0 == value)
				return objectMod;
		}
		return nullptr;
	}

	bool GetParentInstantiationValues(BGSObjectInstanceExtra* modData, KeywordValue parentValue, std::vector<KeywordValue>* instantiationValues)
	{
		if (!modData)
			return false;
		auto data = modData->data;
		if (!data || !data->forms)
			return false;
		instantiationValues->clear();
		UInt64 priority = 0;
		KeywordValueArray* instantiationData = nullptr;
		for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			if (!objectMod)
				continue;
			UInt64 currPriority = convertToUnsignedAbs<UInt8>(objectMod->priority);
			if (currPriority < priority)
				continue;
			AttachParentArray* attachPoints = reinterpret_cast<AttachParentArray*>(&objectMod->unk98);
			for (UInt32 i = 0; i < attachPoints->kewordValueArray.count; i++)
			{
				//_DEBUG("api: %i", i);
				KeywordValue value = attachPoints->kewordValueArray[i];
				if (value == parentValue)
				{
					instantiationData = reinterpret_cast<KeywordValueArray*>(&objectMod->unkB0);
					//_DEBUG("if count: %i", instantiationData->count);
					priority = currPriority;
				}
			}
		}
		if (instantiationData)
		{
			for (UInt32 i = 0; i < instantiationData->count; i++)
				instantiationValues->push_back((*instantiationData)[i]);
		}
		//_DEBUG("if counts: %i", instantiationValues->size());
		return true;
	}

	bool AddAttachPoint(AttachParentArray* attachPoints, BGSKeyword* attachPointKW)
	{
		if (!attachPoints || !attachPointKW)
			return false;
		SInt16 idx = g_AttachPointKeywordArray->GetItemIndex(attachPointKW);
		if (idx >= 0)
		{
			UInt16 uidx = (UInt16)idx;
			if (attachPoints->kewordValueArray.GetItemIndex(uidx) == -1)
				attachPoints->kewordValueArray.Push(idx);
			return true;
		}
		return false;
	}

	bool AddAttachValue(AttachParentArray* attachPoints, KeywordValue attachValue)
	{
		if (!attachPoints || attachValue < 0)
			return false;
		if (attachPoints->kewordValueArray.GetItemIndex(attachValue) == -1)
		{
			attachPoints->kewordValueArray.Push(attachValue);
			return true;
		}
		return false;
	}

	bool WeaponInstanceHasKeyword(TESObjectWEAP::InstanceData* instanceData, BGSKeyword* checkKW)
	{
		if (!checkKW || !instanceData || !instanceData->keywords)
			return false;
		auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(&instanceData->keywords->keywordBase, 1);
		if (HasKeyword_Internal(&instanceData->keywords->keywordBase, checkKW, 0))
			return true;
		return false;
	}

	bool HasKeyword(BGSKeywordForm* keywordForm, BGSKeyword* checkKW)
	{
		if (!keywordForm || !checkKW)
			return false;
		auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(&keywordForm->keywordBase, 1);
		if (HasKeyword_Internal(&keywordForm->keywordBase, checkKW, 0))
			return true;
		return false;
	}

	bool UpdateAimModel(MSFAimModel* oldModel, MSFAimModel* newModel)
	{
		if (!oldModel || !newModel)
			return false;
		oldModel->BaseStability += newModel->BaseStability;
		oldModel->CoF_MinAngle += newModel->CoF_MinAngle;
		oldModel->CoF_MaxAngle += newModel->CoF_MaxAngle;
		oldModel->CoF_IncrPerShot += newModel->CoF_IncrPerShot;
		oldModel->CoF_DecrPerSec += newModel->CoF_DecrPerSec;
		oldModel->CoF_DecrDelayMS += newModel->CoF_DecrDelayMS;
		oldModel->CoF_SneakMult += newModel->CoF_SneakMult;
		oldModel->Rec_DimSpringForce += newModel->Rec_DimSpringForce;
		oldModel->Rec_DimSightsMult += newModel->Rec_DimSightsMult;
		oldModel->Rec_MaxPerShot += newModel->Rec_MaxPerShot;
		oldModel->Rec_MinPerShot += newModel->Rec_MinPerShot;
		oldModel->Rec_HipMult += newModel->Rec_HipMult;
		oldModel->Rec_RunawayShots += newModel->Rec_RunawayShots;
		oldModel->Rec_ArcMaxDegrees += newModel->Rec_ArcMaxDegrees;
		oldModel->Rec_ArcRotate += newModel->Rec_ArcRotate;
		oldModel->CoF_IronSightsMult += newModel->CoF_IronSightsMult;
		return true;
	}

	bool UpdateZoomData(MSFZoomData* zoomData, MSFZoomData* newZoomData)
	{
		if (!zoomData || !newZoomData)
			return false;
		zoomData->FOVmult += newZoomData->FOVmult;
		if (newZoomData->imageSpaceFormID != 0)
			zoomData->imageSpaceFormID = newZoomData->imageSpaceFormID;
		if (newZoomData->imageSpace)
			zoomData->imageSpace = newZoomData->imageSpace;
		zoomData->offsetX += newZoomData->offsetX;
		zoomData->offsetY += newZoomData->offsetY;
		zoomData->offsetZ += newZoomData->offsetZ;
		return true;
	}

	void PlaySubgraphAnimation(Actor* target, BSFixedString asEventName) 
	{
		PlaySubgraphAnimationInternal((*g_gameVM)->m_virtualMachine, 0, target, asEventName);
	}

	void SetAnimationVariableBool(TESObjectREFR* ref, BSFixedString asVariableName, bool newVal) 
	{
		SetAnimationVariableBoolInternal((*g_gameVM)->m_virtualMachine, 1, ref, asVariableName, newVal);
	}

	void SetAnimationVariableInt(TESObjectREFR* ref, BSFixedString asVariableName, SInt32 newVal)
	{
		SetAnimationVariableIntInternal((*g_gameVM)->m_virtualMachine, 1, ref, asVariableName, newVal);
	}

	void SetAnimationVariableFloat(TESObjectREFR* ref, BSFixedString asVariableName, float newVal)
	{
		SetAnimationVariableFloatInternal((*g_gameVM)->m_virtualMachine, 1, ref, asVariableName, newVal);
	}

	bool GetAnimationVariableBool(TESObjectREFR* ref, BSFixedString asVariableName)
	{
		return GetAnimationVariableBoolInternal((*g_gameVM)->m_virtualMachine, 1, ref, asVariableName);
	}

	SInt32 GetAnimationVariableInt(TESObjectREFR* ref, BSFixedString asVariableName)
	{
		return GetAnimationVariableIntInternal((*g_gameVM)->m_virtualMachine, 1, ref, asVariableName);
	}

	float GetAnimationVariableFloat(TESObjectREFR* ref, BSFixedString asVariableName)
	{
		return GetAnimationVariableFloatInternal((*g_gameVM)->m_virtualMachine, 1, ref, asVariableName);
	}

	bool PlayIdle(Actor* actor, TESIdleForm* idle)
	{
		return PlayIdleInternal2(actor, idle, 0, (*g_gameVM)->m_virtualMachine, 0);
	}

	bool PlayIdleAction(Actor* actor, BGSAction* action)
	{
		return PlayIdleActionInternal(actor, action, nullptr, (*g_gameVM)->m_virtualMachine, 0);
	}

	void DrawWeapon(Actor* actor)
	{
		return DrawWeaponInternal((*g_gameVM)->m_virtualMachine, 0, actor);
	}

	void FireWeapon(Actor* actor, UInt32 shots)
	{
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(actor, 41);
		if (!stack)
			return;
		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return;
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData || !extraInstanceData->baseForm || !extraInstanceData->instanceData)
			return;
		BGSObjectInstance idStruct;
		idStruct.object = extraInstanceData->baseForm;
		idStruct.instanceData = extraInstanceData->instanceData;
		FireWeaponInternal(actor, idStruct, 41, shots);
	}

	void ReloadWeapon(Actor* actor)
	{
		BGSInventoryItem::Stack* stack = Utilities::GetEquippedStack(actor, 41);
		if (!stack)
			return;
		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return;
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (!extraInstanceData || !extraInstanceData->baseForm || !extraInstanceData->instanceData)
			return;
		BGSObjectInstance idStruct;
		idStruct.object = extraInstanceData->baseForm;
		idStruct.instanceData = extraInstanceData->instanceData;
		ReloadWeaponInternal(actor, idStruct, 41);
	}

	bool ModActorValue(Actor* actor, ActorValueInfo* av, float val)
	{
		for (UInt32 i = 0; i < actor->actorValueData.count; i++)
		{
			Actor::ActorValueData data;
			actor->actorValueData.GetNthItem(i, data);
			if (data.avFormID == av->formID)
			{
				data.value = val;
			}
		}
		return false;
	}

	bool AddRemActorValue(Actor* actor, ActorValueInfo* av, bool bAdd)
	{
		for (UInt32 i = 0; i < actor->actorValueData.count; i++)
		{
			Actor::ActorValueData data;
			actor->actorValueData.GetNthItem(i, data);
			if (data.avFormID == av->formID)
			{
				if (bAdd)
					return true;
				else
				{
					actor->actorValueData.Remove(i);
					return true;
				}
			}
		}
		if (bAdd)
		{
			Actor::ActorValueData data;
			data.avFormID = av->formID;
			data.value = av->defaultBase;
			actor->actorValueData.Push(data);
		}
		return true;
	}


	bool AddRemKeyword(BGSKeywordForm* keywordForm, BGSKeyword* keyword, bool bAdd)
	{
		if (keywordForm && keyword)
		{
			bool hadKW = false;
			VMArray<BGSKeyword*> kwds;
			for (UInt32 i = 0; i < keywordForm->numKeywords; i++)
			{
				if (keywordForm->keywords[i] != keyword)
					kwds.Push(&keywordForm->keywords[i]);
				else
					hadKW = true;
			}
			if (!(hadKW ^ bAdd))
				return true;
			if (bAdd)
				kwds.Push(&keyword);
			Heap_Free(keywordForm->keywords);
			keywordForm->keywords = nullptr;
			keywordForm->numKeywords = 0;
			keywordForm->keywords = (BGSKeyword**)Heap_Allocate(sizeof(BGSKeyword*) * kwds.Length());
			if (kwds.Length() > 0)
			{
				keywordForm->numKeywords = kwds.Length();

				for (UInt32 i = 0; i < kwds.Length(); i++)
				{
					BGSKeyword * kwd = nullptr;
					kwds.Get(&kwd, i);
					keywordForm->keywords[i] = kwd;
				}
			}
			return true;
		}
		return false;
	}

	UInt32 AddRemFlag(UInt32 flagHolder, UInt32 flag, UInt8 bAdd, UInt8 op)
	{
		switch (op)
		{
		case BGSMod::Container::Data::kOpFlag_Set_Int: { if (bAdd) flagHolder |= flag; else flagHolder &= ~flag; } break;
		case BGSMod::Container::Data::kOpFlag_And_Bool: { if (!bAdd) flagHolder &= ~flag; } break;
		case BGSMod::Container::Data::kOpFlag_Or_Bool: { if (bAdd) flagHolder |= flag; } break;
		case BGSMod::Container::Data::kOpFlag_Mod: { flagHolder ^= flag; } break;
		}
		return flagHolder;
	}

	float GetActorValue(tArray<Actor::ActorValueData>* avdata, UInt32 formId)
	{
		for (UInt32 i = 0; i < avdata->count; i++)
		{
			Actor::ActorValueData data;
			avdata->GetNthItem(i, data);
			if (data.avFormID == formId)
				return data.value;
		}
		return -1;
	}

}

TlsShare::TlsShare()
{
	DWORD tlsidx = *(hkLifoAllocator_TLS.GetPtr());
	void* hkLifoAllocator = TlsGetValue(tlsidx);
	std::pair<DWORD, void*> TlsPair;
	TlsPair.first = tlsidx;
	TlsPair.second = hkLifoAllocator;
	TlsValues.push_back(TlsPair);

	tlsidx = *(unk1_TLS.GetPtr());
	void* unkptr = TlsGetValue(tlsidx);
	TlsPair.first = tlsidx;
	TlsPair.second = unkptr;
	TlsValues.push_back(TlsPair);
};

void TlsShare::CopyTls()
{
	for (auto it = TlsValues.begin(); it != TlsValues.end(); it++)
		TlsSetValue(it->first, it->second);
};