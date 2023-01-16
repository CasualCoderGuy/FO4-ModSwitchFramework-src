#include "MSF_Shared.h"


RelocAddr <uintptr_t> s_BGSObjectInstanceExtraVtbl(0x2C4BE10); // ??_7BGSObjectInstanceExtra@@6B@
RelocAddr <_DrawWeapon> DrawWeaponInternal(0x138C700);
RelocAddr <_AddItem_Native> AddItemNative(0x1402B40); 
RelocAddr <_RemoveItem_Native> RemoveItemNative(0x140D1F0);
RelocAddr <_IsInIronSights> IsInIronSights(0x13911D0);
RelocAddr <_SetAnimationVariableBool> SetAnimationVariableBoolInternal(0x140EB30); //0x140EA10
RelocAddr <_PlayIdle> PlayIdleInternal(0x13864C0); //0x13863A0
RelocAddr <_PlayIdle2> PlayIdleInternal2(0x13864C0);
RelocAddr <_PlayIdleAction> PlayIdleActionInternal(0x13865C0); //0x13864A0 
RelocAddr <_ShowNotification> ShowNotification(0x0AE1E90);
RelocAddr <_PlaySubgraphAnimation> PlaySubgraphAnimationInternal(0x138A250); //0x138A130
RelocAddr <_GetKeywordFromValueArray> GetKeywordFromValueArray(0x0569070);
RelocAddr <_AttachModToStack> AttachRemoveModStack(0x01A84B0);
RelocAddr <_UpdMidProc> UpdateMiddleProcess(0x0E2C3E0);
RelocAddr <_UpdateEquipData> UpdateEquipData(0x01C0040);
RelocAddr <_UpdateAnimGraph> UpdateAnimGraph(0x0D7EB20);
RelocAddr <_EquipHandler> EquipHandler(0x0E1D6D0);
RelocAddr <_UniversalEquipHandler> UniversalEquipHandler(0x0DBEA80);
RelocAddr <_UnkSub_EFF9D0> UnkSub_EFF9D0(0xEFF9D0);
RelocAddr <_UnkSub_DFE930> UnkSub_DFE930(0xDFE930);

RelocPtr  <void*> g_pipboyInventoryData(0x5ABCAB8); // 130
RelocPtr  <void*> g_CheckStackIDFunctor(0x2C5C928);
RelocPtr  <void*> g_ModifyModDataFunctor(0x2D11060);
RelocPtr  <void*> unk_05AB38D0(0x5AB38D0);
RelocPtr  <tArray<BGSKeyword*>> g_AttachPointKeywordArray(0x59DA3F0);

BSExtraData::~BSExtraData() {};
void BSExtraData::Unk_01() {};
void BSExtraData::Unk_02() {};

CheckStackIDFunctor::CheckStackIDFunctor(UInt64 ID)
{
	vtbl = g_CheckStackIDFunctor;
	stackID = ID;
	pad0C = 0;
}

ModifyModDataFunctor::ModifyModDataFunctor(BGSMod::Attachment::Mod* modToAttach, UInt8* ret, bool Attach)
{
	vtbl = g_ModifyModDataFunctor;
	unk08 = 1;
	mod = modToAttach;
	byteptr = ret;
	*byteptr = 1;
	unk28 = Attach * 0x100 + !Attach * 0xFF;//0xFF when remove 
	//unk29 = 1;
	//unk2A = 0;
	//pad2B = 0;
	//pad2C = 0;
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

BGSObjectInstanceExtra* CreateObjectInstanceExtra(BGSObjectInstanceExtra::Data* data)
{
	BGSObjectInstanceExtra* pInstanceData = (BGSObjectInstanceExtra*)BSExtraData::Create(sizeof(BGSObjectInstanceExtra), s_BGSObjectInstanceExtraVtbl.GetUIntPtr());
	pInstanceData->type = kExtraData_ObjectInstance;
	pInstanceData->data = data;
	return pInstanceData;
}

namespace Utilities
{

	void SendNotification(std::string asNotificationText)
	{
		ShowNotification(asNotificationText, 0, 1);
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
			UInt8 modIndex = (UInt8)(formID >> 12 & 0xFF);
			str << (*g_dataHandler)->modList.lightMods[modIndex]->name << '|';
			formID &= 0xFFFF;
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

	TESObjectWEAP::InstanceData* GetEquippedInstanceData(Actor * ownerActor, UInt32 iEquipSlot)
	{
		if (ownerActor) 
		{
			if (iEquipSlot >= ActorEquipData::kMaxSlots)
				return nullptr;
			//ActorEquipData * equipData = ownerActor->equipData;
			//if (!equipData)
			//	return nullptr;
			//auto item = equipData->slots[iEquipSlot].item;
			//if (!item)
			//	return nullptr;
			//return (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(equipData->slots[iEquipSlot].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
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
			if (iEquipSlot >= ActorEquipData::kMaxSlots)
				return nullptr;
			//ActorEquipData * equipData = ownerActor->equipData;
			//if (!equipData)
			//	return nullptr;
			//auto item = equipData->slots[iEquipSlot].item;
			//if (!item)
			//	return nullptr;
			//return equipData->slots[iEquipSlot].extraData;
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

	UInt32 GetEquippedItemFormID(Actor * owner, UInt32 slotIndex)
	{
		if (owner) 
		{
			if (slotIndex >= ActorEquipData::kMaxSlots)
				return 0x0;
			ActorEquipData * equipData = owner->equipData;
			if (!equipData)
				return 0x0;
			TESForm *item = equipData->slots[slotIndex].item;
			if (!item)
				return 0x0;
			return item->formID;
		}
		return 0x0;
	}

	BGSInventoryItem::Stack* GetEquippedStack(Actor* owner, UInt32 slotIndex)
	{
		BGSInventoryItem::Stack* eqStack = nullptr;
		if (slotIndex >= ActorEquipData::kMaxSlots)
			return nullptr;
		if (!owner)
			return nullptr;
		if (!owner->equipData)
			return nullptr;
		auto item = owner->equipData->slots[slotIndex].item;
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

	UInt64 GetStackID(BGSInventoryItem* item, BGSInventoryItem::Stack* stack) //equipped stack ID should always be 0 but check nevertheless
	{
		UInt64 IDcount = 0;
		for (BGSInventoryItem::Stack* currStack = item->stack; currStack; currStack = currStack->next)
		{
			if (currStack == stack)
				return IDcount;
			IDcount++;
		}
		return 0;
	}

	UInt64 GetInventoryItemCount(BGSInventoryList* inventory, TESForm* item)
	{
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

	SInt16 GetValueForTypedKeyword(BGSKeyword* keyword)
	{
		if (!keyword)
			return -1;
		return g_AttachPointKeywordArray->GetItemIndex(keyword);
	}

	bool HasAttachPoint(AttachParentArray* attachPoints, BGSKeyword* attachPointKW)
	{
		if (!attachPoints || !attachPointKW)
			return false;
		for (UInt32 i = 0; i < attachPoints->kewordValueArray.count; i++)
		{
			UInt16 value = attachPoints->kewordValueArray[i];
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
			AttachParentArray* attachPoints = reinterpret_cast<AttachParentArray*>(&objectMod->unk98);
			for (UInt32 i = 0; i < attachPoints->kewordValueArray.count; i++)
			{
				UInt16 value = attachPoints->kewordValueArray[i];
				BGSKeyword* keyword = GetKeywordFromValueArray(AttachParentArray::iDataType, value);
				if (keyword == attachPointKW)
					return true;
			}
		}
		return false;
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

	bool WeaponInstanceHasKeyword(TESObjectWEAP::InstanceData* instanceData, BGSKeyword* checkKW)
	{
		if (!checkKW || !instanceData || !instanceData->keywords)
			return false;
		auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(&instanceData->keywords->keywordBase, 1);
		if (HasKeyword_Internal(&instanceData->keywords->keywordBase, checkKW, 0))
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

	BGSInventoryItem::Stack* AddInventoryStack(BGSInventoryItem::Stack* startStack) // not working
	{
		if (!startStack)
			return nullptr;
		SInt32 startCount = startStack->count;
		BGSInventoryItem::Stack* stack = startStack;
		do
		{
			stack = stack->next;
		} while (stack);
		*stack = *startStack;
		stack->count = startCount - 1;
		startStack->count = 1;
		return stack;
	}

}