#include "ModSwitchFramework.h"

//PlayIdleInternal2(actor, idlePA, 0, (*g_gameVM)->m_virtualMachine, 0);

namespace ModSwitchFramework
{

	//========================== Main Functions ===========================

	//  FROM MOD EVENT:
	//bool ReevalModdedWeapon(TESBoundObject* weapon, BGSMod::Attachment::Mod* mod)
	//  FROM SCALEFORM:
	//bool ReevalModdedWeapon(BGSInventoryItem::Stack* stack)
	//{
	//	bool IsModified = ReevalStackMods(stack->extraData);
	//	if ((stack->flags & 7) && IsModified)
	//	{
	//		owner->middleProcess->unk08->equipData->instanceData = instanceData;
	//		owner->middleProcess->unk08->equipData->equippedData->ammo = instanceData->ammo;
	//		owner->middleProcess->unk08->equipData->equippedData->unk18 = instanceData->ammoCapacity;
	//	}
	//}

	//FROM SCALEFORM:
	bool SwitchToSelectedAmmo(void* obj)
	{
		TESAmmo* selectedAmmo = reinterpret_cast<TESAmmo*>(obj);
		if (!selectedAmmo)
			return false;
		Actor* playerActor = *g_player;
		//IsReloading, isanimplaying?
		if (MSF_MainData::SwitchFlags != 127)
			return false;
		MSF_MainData::SwitchFlags = 125;
		BGSMod::Attachment::Mod* mod = MSF_Data::GetModForAmmo(selectedAmmo);
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		//if (Utilities::HasObjectMod(modData, MSF_MainData::APbaseMod) && !Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW))
		if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW))
		{
			if (mod)
			{
				//if (!Utilities::HasObjectMod(modData, mod))
				{
					MSF_MainData::ModToSwitch = mod;
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
					{
						if (!DrawWeapon())
						{
							if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
							{
								ReloadWeapon();
								return true;
							}
						}
						else
							return true;
					}
					//SwitchMod();
					AddModDataToInstance();
					return true;
				}
			}
		}
		MSF_MainData::SwitchFlags = 127;
		MSF_MainData::ModToSwitch = nullptr;
		MSF_MainData::AnimToPlay = nullptr;
		return false;
	}

	//FROM HOTKEY:
	bool SwitchAmmoHotkey(UInt8 key)
	{
		if (key <= 0)
			return false;
		if (MSF_MainData::SwitchFlags != 127)
			return false;
		Actor* playerActor = *g_player;
		//IsReloading, isanimplaying?
		MSF_MainData::SwitchFlags = 125;
		BGSMod::Attachment::Mod* mod = MSF_Data::GetNthMod(key);
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		//if (Utilities::HasObjectMod(modData, MSF_MainData::APbaseMod) && !Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW))
		if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW))
		{
			if (mod)
			{
				//if (!Utilities::HasObjectMod(modData, mod))
				{
					MSF_MainData::ModToSwitch = mod;
					if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
					{
						if (!DrawWeapon())
						{
							if (MSF_MainData::MCMSettingFlags & MSF_MainData::bReloadEnabled)
							{
								ReloadWeapon();
								return true;
							}
						}
						else
							return true;
					}
					//SwitchMod();
					AddModDataToInstance();
					return true;
				}
			}
			_MESSAGE("nomod");
		}
		MSF_MainData::SwitchFlags = 127;
		MSF_MainData::ModToSwitch = nullptr;
		MSF_MainData::AnimToPlay = nullptr;
		return false;
	}

	//FROM HOTKEY:
	bool ToggleModHotkey(UInt8 switchID)
	{
		Actor* playerActor = *g_player;
		//IsReloading, isanimplaying?
		if (MSF_MainData::SwitchFlags != 127)
			return false;
		MSF_MainData::SwitchFlags = switchID;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		//if (priority == 123 && ((*g_ui)->IsMenuOpen("ScopeMenu") || (*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState) == 4))
		//	return false;
		bool DoSwitch = false;
		//if (Utilities::HasObjectMod(modData, MSF_MainData::APbaseMod))
		{
			if (switchID == 123)
			{
				if (MSF_Data::GetNextScopeMod(modData))
					DoSwitch = true;
			}
			else if (switchID == 124)
			{
				if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW))
				{
					if (MSF_Data::GetNextFiringModeModStack(eqStack))//MSF_Data::GetNextFiringModeMod(modData)
						DoSwitch = true;
				}
			}
			else if (switchID == 126)
			{
				if (MSF_Data::GetUnderbarrelMod(modData))
					DoSwitch = true;
			}
		}
		if (MSF_MainData::ModToSwitch && DoSwitch)
		{
			//if (!Utilities::HasObjectMod(modData, MSF_MainData::ModToSwitch))
			{
				if (MSF_MainData::MCMSettingFlags & MSF_MainData::bDrawEnabled)
				{
					if (!DrawWeapon())
					{
						if ((MSF_MainData::MCMSettingFlags & MSF_MainData::bCustomAnimEnabled) && MSF_MainData::AnimToPlay)
						{
							return Utilities::PlayIdle(playerActor, MSF_MainData::AnimToPlay);
						}
					}
					else
						return true;
				}
				//SwitchMod();
				AddModDataToInstance();
				return true;
			}
		}
		MSF_MainData::SwitchFlags = 127;
		MSF_MainData::ModToSwitch = nullptr;
		MSF_MainData::AnimToPlay = nullptr;
		return false;
	}


	bool SwitchMod(void)
	{
		BGSMod::Attachment::Mod* mod = MSF_MainData::ModToSwitch;
		MSF_MainData::ModToSwitch = nullptr;
		if (!mod)
		{
			if (MSF_MainData::SwitchFlags == 0)
				return false;
			else
			{
				MSF_MainData::SwitchFlags = 127;
				return false;
			}
		}
		UInt8 priority = mod->priority;
		if (MSF_MainData::SwitchFlags != mod->priority)
		{
			if (MSF_MainData::SwitchFlags == 0)
				return false;
			else
			{
				MSF_MainData::SwitchFlags = 127;
				return false;
			}
		}
		Actor* playerActor = *g_player;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		if (!modData)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		if (!instanceData)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		if (!Utilities::HasObjectMod(modData, MSF_MainData::APbaseMod))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW) && (priority == 124 || priority == 125))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		//if (priority == 123 && ((*g_ui)->IsMenuOpen("ScopeMenu") || (*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState) == 4))
		//	return false;
		if (Utilities::HasObjectMod(modData, mod))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		BGSMod::Attachment::Mod* oldMod = Utilities::GetFirstModWithPriority(modData, priority);
		if (!oldMod)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		if (!eqStack)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		ExtraDataList * stackDataList = eqStack->extraData;
		if (!stackDataList)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		VMArray<BGSMod::Attachment::Mod*> allAttachedMod;//= papyrusActor::GetWornItemMods(playerActor, 41);
		if (!ReplaceModExtraData(stackDataList, oldMod, mod, false))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		if (!UpdateInstanceDataLim(stackDataList))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		playerActor->middleProcess->unk08->equipData->equippedData->ammo = instanceData->ammo;
		playerActor->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)instanceData->ammoCapacity;
		MSF_MainData::SwitchFlags = 127;
		if (MSF_MainData::MCMSettingFlags & MSF_MainData::bNamingEnabled)
		{
			BSExtraData * extraName = stackDataList->GetByType(ExtraDataType::kExtraData_TextDisplayData);
			ExtraTextDisplayData * textDisplay = DYNAMIC_CAST(extraName, BSExtraData, ExtraTextDisplayData);
			Utilities::RenameAtTextStart(textDisplay, oldMod->fullName.name, mod->fullName.name);
		}
		SeparateInventoryStack(eqStack, playerActor->equipData->slots[41].item, allAttachedMod);
		CALL_MEMBER_FN(playerActor, UpdateEquipment)();
		CALL_MEMBER_FN(playerActor, QueueUpdate)(0, 0, 1, 0);
		MSF_Scaleform::UpdateWidget(priority);
		return true;
	}

	bool AddModDataToInstance(void)
	{
		BGSMod::Attachment::Mod* mod = MSF_MainData::ModToSwitch;
		MSF_MainData::ModToSwitch = nullptr;
		if (!mod)
		{
			if (MSF_MainData::SwitchFlags == 0)
				return false;
			else
			{
				MSF_MainData::SwitchFlags = 127;
				return false;
			}
		}
		UInt8 priority = mod->priority;
		if (MSF_MainData::SwitchFlags != mod->priority)
		{
			if (MSF_MainData::SwitchFlags == 0)
				return false;
			else
			{
				MSF_MainData::SwitchFlags = 127;
				return false;
			}
		}
		Actor* playerActor = *g_player;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		if (!modData)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		if (!instanceData)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW) && (priority == 124 || priority == 125))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		//if (priority == 123 && ((*g_ui)->IsMenuOpen("ScopeMenu") || (*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState) == 4))
		//	return false;
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		if (!eqStack)
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}
		//ExtraDataList * stackDataList = eqStack->extraData;
		//if (!stackDataList)
		//	return false;
		//VMArray<BGSMod::Attachment::Mod*> allAttachedMod;//= papyrusActor::GetWornItemMods(playerActor, 41);
		if (!UpdateInstanceDataWithMod(eqStack, mod))
		{
			MSF_MainData::SwitchFlags = 127;
			return false;
		}

		//UInt64 newLoadedAmmoNum = 
		playerActor->middleProcess->unk08->equipData->equippedData->ammo = instanceData->ammo;
		playerActor->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)instanceData->ammoCapacity;
		MSF_MainData::SwitchFlags = 127;

		if (MSF_MainData::MCMSettingFlags & MSF_MainData::bNamingEnabled)
		{
			//BSExtraData * extraName = stackDataList->GetByType(ExtraDataType::kExtraData_TextDisplayData);
			//ExtraTextDisplayData * textDisplay = DYNAMIC_CAST(extraName, BSExtraData, ExtraTextDisplayData);
			//Utilities::RenameAtTextStart(textDisplay, oldMod->fullName.name, mod->fullName.name);
		}
		//SeparateInventoryStack(eqStack, playerActor->equipData->slots[41].item, allAttachedMod);
		MSF_Scaleform::UpdateWidget(priority);
		return true;
	}

	bool CompareStackModData(Actor* owner) //crashes on game load: extraData->GetByType ; clear MSF_MainData::stackMods on new game?
	{
		std::vector<BGSInventoryItem::Stack*> stacks;
		for (UInt32 i = 0; i < owner->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i, inventoryItem);
			TESObjectWEAP* asWeap = DYNAMIC_CAST(inventoryItem.form, TESForm, TESObjectWEAP);
			if (asWeap && inventoryItem.stack)
			{
				for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
					stacks.push_back(stack);
			}
		}
		std::vector<StackModAssociation>::iterator itStack = MSF_MainData::stackMods.begin();
		while (itStack != MSF_MainData::stackMods.end())
		{
			std::vector<BGSInventoryItem::Stack*>::iterator itMod = find(stacks.begin(), stacks.end(), itStack->stack);
			if (itMod == stacks.end())
			{
				itStack = MSF_MainData::stackMods.erase(itStack);
				ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(itStack->stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::modifiedInstanceKW))
				{
					_MESSAGE("unlikely");
				}
			}
			else
			{
				ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(itStack->stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::modifiedInstanceKW))
				{
					UpdateInstanceDataWithMod(itStack->stack, nullptr);
					_MESSAGE("upd");
				}
				++itStack;
			}
		}
		return true;
	}

	BGSObjectInstanceExtra* modDataEx = nullptr;
	BGSObjectInstanceExtra::Data* omodData = nullptr;
	BGSObjectInstanceExtra::Data::Form* omodForms = nullptr;


	bool InitEquippedWeapon(void)
	{
		//return false;
		if (MSF_MainData::SwitchFlags != 0)
			return false;
		Actor* playerActor = *g_player;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);

		if (!modData)
		{
			MSF_MainData::SwitchFlags = 127;//?
			return false;
		}
		_MESSAGE("ptr: %08X, unk10: %i, unk13: %i, unk14: %i, unk20: %i, unk22: %i, unk24: %i, size: %i", omodForms, modData->unk10, modData->unk13, modData->unk14, modData->unk20, modData->unk22, modData->unk24, modData->data->blockSize);
		if (modDataEx == modData)
			_MESSAGE("eq modData");
		if (omodData == modData->data)
			_MESSAGE("eq omodData");
		if (omodForms == modData->data->forms)
			_MESSAGE("eq omodForms");
		modDataEx = modData;
		omodData = modData->data;
		omodForms = modData->data->forms;
		for (UInt32 i = 0; i < modData->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++)
		{
			_MESSAGE("id: %08X, unk04: %08X", modData->data->forms[i].formId, modData->data->forms[i].unk04);
		}
		//		if (Utilities::HasObjectMod(modData, MSF_MainData::APbaseMod))
		if (Utilities::GetFirstModWithPriority(modData, 123) && Utilities::GetFirstModWithPriority(modData, 124) && Utilities::GetFirstModWithPriority(modData, 125) && Utilities::GetFirstModWithPriority(modData, 126))
		{
			MSF_MainData::SwitchFlags = 127;
			return true;
		}

		//return Utilities::AddRemModTest(modData, MSF_MainData::APbaseMod->formID, true);
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		ExtraDataList * stackDataList = eqStack->extraData;
		BSReadAndWriteLocker locker(&stackDataList->m_lock);
		_MESSAGE("locked");
		BGSObjectInstanceExtra* newOIE = Utilities::AddRemModTest(modData, MSF_MainData::APbaseMod->formID, true);
		if (!newOIE)
			return false;
		//if (stackDataList) {
		//	BSExtraData* objectModData = stackDataList->GetByType(kExtraData_ObjectInstance);
		//	if (objectModData)
		//	{
		//		//InterlockedIncrement(&instanceData->m_refCount);
		//		_MESSAGE("switching in stack");
		//		stackDataList->Remove(kExtraData_ObjectInstance, modData);
		//		stackDataList->Add(kExtraData_ObjectInstance, newOIE);
		//	}
		//}
		//ActorEquipData * equipData = playerActor->equipData;
		//if (!equipData)
		//	return false;
		//_MESSAGE("switching in eqdata");
		//equipData->slots[41].extraData = newOIE;

		UpdateInstanceDataLim(stackDataList);

		return true;


		/*
		TESForm* eqWeap = playerActor->equipData->slots[41].item;
		if (!eqWeap)
		return false;
		UInt32 weapCount = 0;
		UInt32 count = 0;
		BGSInventoryItem::Stack* stackEq = nullptr;
		for (UInt32 i1 = 0; i1 < playerActor->inventoryList->items.count; i1++)
		{
		BGSInventoryItem inventoryItem;
		playerActor->inventoryList->items.GetNthItem(i1, inventoryItem);
		TESForm* baseWeapon = inventoryItem.form;
		if (baseWeapon && baseWeapon == eqWeap)
		{
		for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
		{
		count++;
		weapCount += stack->count;
		//_MESSAGE("round: %i, count: %i", count, stack->count);
		if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
		{
		//	_MESSAGE("equipped");
		//	eqNum = stack->count;
		stackEq = stack;
		//	instanceDataEx = stack->extraData->GetByType(kExtraData_InstanceData);
		}
		//BSExtraData* modList = stack->extraData->GetByType(kExtraData_ObjectInstance);
		//if (extraData == modList)
		//{
		//	_MESSAGE("same");
		//}
		//modList = stack->extraData->GetByType(kExtraData_InstanceData);
		//if (instanceDataEx == modList)
		//{
		//	_MESSAGE("same2");
		//}
		}
		}
		}

		_MESSAGE("weap count: %i", weapCount);
		MSF_MainData::SwitchFlags = 127;
		if (weapCount > 1)
		{
		if (!MSF_MainData::DummyNPC || !MSF_MainData::ContTemp || !MSF_MainData::ContFinal)
		return false;
		UInt32 index = 0;
		UInt32 num = 1;
		bool Silent = true;
		bool Force = false;
		bool endloop = false;
		VMArray<VMVariable> args;
		VMVariable var1; VMVariable var2;  VMVariable var3; VMVariable var4; VMVariable var5; VMVariable var6; VMVariable var7; VMVariable var8; VMVariable var9;
		var1.Set(&eqWeap); var2.Set(&num); var3.Set(&Silent);
		args.Clear(); args.Push(&var1); args.Push(&var2); args.Push(&var3); var4.Set(&MSF_MainData::ContTemp); args.Push(&var4);
		CallFunctionNoWait(playerActor, "RemoveItem", args);
		UInt32 rem = weapCount - 1;
		//args.Clear(); var2.Set(&rem); var4.Set(&MSF_MainData::ContFinal); args.Push(&var1); args.Push(&var2); args.Push(&var3); args.Push(&var4);//removes only the same stack if any remains from that stack
		//CallFunctionNoWait(playerActor, "RemoveItem", args);
		var5.Set(&MSF_MainData::ContFinal);
		for (index = 0; index < rem; index++)
		{
		VMArray<VMVariable> args2;
		args2.Push(&var1); args2.Push(&var2); args2.Push(&var3); args2.Push(&var5);
		CallFunctionNoWait(playerActor, "RemoveItem", args2);
		_MESSAGE("rem: %i, %i", index, rem);
		}
		TESObjectREFR* actorRef = DYNAMIC_CAST(playerActor, Actor, TESObjectREFR);
		var6.Set(&actorRef);
		VMArray<VMVariable> args3;
		args3.Push(&var1); args3.Push(&var2); args3.Push(&var3); args3.Push(&var6);
		CallFunctionNoWait(MSF_MainData::ContTemp, "RemoveItem", args3);
		bool Attached = false;
		for (index = 0; index < 200; index++) //100?-250
		{
		//AttachModToInventoryItem(playerActor, eqWeap, MSF_MainData::APbaseMod);
		var7.Set(&MSF_MainData::APbaseMod);
		VMArray<VMVariable> args4;
		args4.Push(&var1); args4.Push(&var7);
		CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args4);
		//for (UInt32 i1 = 0; i1 < playerActor->inventoryList->items.count; i1++)
		//{
		//	BGSInventoryItem inventoryItem;
		//	playerActor->inventoryList->items.GetNthItem(i1, inventoryItem);
		//	TESForm* baseWeapon = inventoryItem.form;
		//	if (baseWeapon && baseWeapon == eqWeap)
		//	{
		//		for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
		//		{
		//			BGSObjectInstanceExtra* modList = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		//			if (Utilities::HasObjectMod(modList, MSF_MainData::APbaseMod))
		//				Attached = true;
		//			//if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
		//			//{
		//			//	_MESSAGE("equipped");
		//			//}
		//		}
		//	}
		//}
		//if (Attached)
		//{
		//	_MESSAGE("attached: %i", index);
		//	break;
		//}
		}
		//var7.Set(&MSF_MainData::APbaseMod);
		//VMArray<VMVariable> args4;
		//args4.Push(&var1); args4.Push(&var7);
		//for (index = 0; index < 30 * weapCount; index++)
		//{
		//	//AttachModToInventoryItem(playerActor, eqWeap, MSF_MainData::APbaseMod);
		//	CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args4);
		//}
		//var7.Set(&MSF_MainData::APbaseMod);
		//VMArray<VMVariable> args4;
		//args4.Push(&var1); args4.Push(&var7);
		//CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args4);

		//var4.Set(&MSF_MainData::APbaseMod); args.Push(&var4); args.Push(&var2);
		//TESObjectREFR* ref1 = DYNAMIC_CAST(modData, BGSObjectInstanceExtra, TESObjectREFR);
		//TESObjectREFR* ref2 = (TESObjectREFR*)Runtime_DynamicCast(stackEq, RTTI_BGSInventoryItem__Stack, RTTI_TESObjectREFR);
		//TESObjectREFR* ref3 = (TESObjectREFR*)Runtime_DynamicCast(stackEq, RTTI_BSExtraData, RTTI_TESObjectREFR);
		//TESObjectREFR* ref4 = MSF_MainData::TestRifle;
		//CallFunctionNoWait(ref1, "AttachMod", args);
		//CallFunctionNoWait(ref2, "AttachMod", args);
		//CallFunctionNoWait(ref3, "AttachMod", args);
		//CallFunctionNoWait(MSF_MainData::TestRifle, "AttachMod", args);
		//_MESSAGE("test: %08X, %08X, %08X, %08X", ref1, ref2, ref3, MSF_MainData::TestRifle);




		//attach
		//args.Clear(); var2.Set(&num); var4.Set(&playerActor); args.Push(&var1); args.Push(&var2); args.Push(&var3); args.Push(&var4);
		//CallFunctionNoWait(MSF_MainData::DummyNPC, "RemoveItem", args);
		//args.Clear(); args.Push(&var1); var5.Set(&MSF_MainData::APbaseMod); args.Push(&var5);
		//CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//args.Clear(); args.Push(&var1); var5.Set(&MSF_MainData::PlaceholderModZD); args.Push(&var5);
		//CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//args.Clear(); args.Push(&var1); var5.Set(&MSF_MainData::PlaceholderModFM); args.Push(&var5);
		//CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//args.Clear(); args.Push(&var1); var5.Set(&MSF_MainData::PlaceholderModAmmo); args.Push(&var5);
		//CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//args.Clear(); args.Push(&var1); var5.Set(&MSF_MainData::PlaceholderModUB); args.Push(&var5);
		//CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//args.Clear(); args.Push(&var1); var5.Set(&Force); args.Push(&var5); args.Push(&var3);
		//CallFunctionNoWait(playerActor, "EquipItem", args);
		////rem = weapCount - 1;
		//args.Clear(); var2.Set(&rem); var4.Set(&playerActor); args.Push(&var1); args.Push(&var2); args.Push(&var3); args.Push(&var4);
		//CallFunctionNoWait(MSF_MainData::ContFinal, "RemoveItem", args);
		//_MESSAGE("rem3");
		//MSF_MainData::SwitchFlags = 127;
		return true;
		}
		if (weapCount == 1)
		{
		bool Silent = true;
		bool Force = false;
		VMArray<VMVariable> args;
		VMVariable var1; VMVariable var2;  VMVariable var3;
		var1.Set(&eqWeap);
		//args.Clear(); var2.Set(&Force); var3.Set(&Silent); args.Push(&var1); args.Push(&var2); args.Push(&var3);
		//CallFunctionNoWait(playerActor, "UnequipItem", args);
		args.Clear(); args.Push(&var1); var2.Set(&MSF_MainData::APbaseMod); args.Push(&var2);
		CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//AttachModToInventoryItem(playerActor, eqWeap, MSF_MainData::APbaseMod);
		args.Clear(); args.Push(&var1); var2.Set(&MSF_MainData::PlaceholderModZD); args.Push(&var2);
		CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		args.Clear(); args.Push(&var1); var2.Set(&MSF_MainData::PlaceholderModFM); args.Push(&var2);
		CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		args.Clear(); args.Push(&var1); var2.Set(&MSF_MainData::PlaceholderModAmmo); args.Push(&var2);
		CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		args.Clear(); args.Push(&var1); var2.Set(&MSF_MainData::PlaceholderModUB); args.Push(&var2);
		CallFunctionNoWait(playerActor, "AttachModToInventoryItem", args);
		//args.Clear(); var2.Set(&Force); var3.Set(&Silent); args.Push(&var1); args.Push(&var2); args.Push(&var3);
		//CallFunctionNoWait(playerActor, "EquipItem", args);
		//MSF_MainData::SwitchFlags = 127;
		return true;
		}
		//MSF_MainData::SwitchFlags = 127;
		return false;*/
	}

	bool SeparateInventoryStack(BGSInventoryItem::Stack* startStack, TESForm* baseWeap, VMArray<BGSMod::Attachment::Mod*> mods)
	{
		if (!startStack || !baseWeap)
			return false;
		UInt32 count = (UInt32)startStack->count - 1;
		if (count < 1)
			return true;
		VMArray<VMVariable> args;
		VMVariable var1; VMVariable var2; VMVariable var3; VMVariable var4;
		args.Clear(); var1.Set(&baseWeap);  args.Push(&var1);
		CallFunctionNoWait(MSF_MainData::DummyNPC, "AddItem", args);
		BGSInventoryItem::Stack* newStack = nullptr;
		for (UInt32 i = 0; i < MSF_MainData::DummyNPC->inventoryList->items.count; i++)
		{
			BGSInventoryItem inventoryItem;
			MSF_MainData::DummyNPC->inventoryList->items.GetNthItem(i, inventoryItem);
			if (inventoryItem.form == baseWeap)
			{
				newStack = inventoryItem.stack;
				break;
			}
		}
		if (!newStack)
			return false;
		BGSMod::Attachment::Mod* mod;
		for (UInt32 i = 0; i < mods.Length(); i++)
		{
			mods.Get(&mod, i);
			if (!mod)
				continue;
			args.Clear(); args.Push(&var1); var2.Set(&mod); args.Push(&var2);
			CallFunctionNoWait(MSF_MainData::DummyNPC, "AttachModToInventoryItem", args);
		}
		startStack->count = 1;
		newStack->count = (SInt32)count;
		bool Silent = true;
		Actor* playerActor = *g_player;
		args.Clear(); args.Push(&var1); var2.Set(&count); args.Push(&var2); var3.Set(&Silent); args.Push(&var3); var4.Set(&playerActor); args.Push(&var4);
		CallFunctionNoWait(MSF_MainData::DummyNPC, "RemoveItem", args);
		return true;
	}

	void SpawnRandomMods(TESObjectCELL* cell)
	{
		for (UInt32 i = 0; i < cell->objectList.count; i++)
		{
			Actor* randomActor = (Actor*)cell->objectList[i];
			if (!randomActor)
				continue;
			TESObjectWEAP* firearm = (TESObjectWEAP*)randomActor->equipData->slots[41].item;
			if (!firearm)
				continue;
			TESAmmo* ammo = randomActor->middleProcess->unk08->equipData->equippedData->ammo;
			tArray<BGSMod::Attachment::Mod*> mods;
			UInt32 count = 0;
			MSF_Data::PickRandomMods(&mods, &ammo, &count);
			if (ammo)
			{
				VMArray<VMVariable> args;
				VMVariable var1; VMVariable var2;
				var1.Set(&ammo); args.Push(&var1); var2.Set(&count); args.Push(&var2);
				CallFunctionNoWait(randomActor, "AddItem", args);
			}
			if (mods.count > 0)
			{
				VMArray<VMVariable> args;
				VMVariable var1; VMVariable var2;
				for (UInt32 n = 0; n < mods.count; n++)
				{
					args.Clear(); var1.Set(&firearm); args.Push(&var1); var2.Set(&mods[n]); args.Push(&var2);
					CallFunctionNoWait(randomActor, "AttachModToInventoryItem", args);
				}
			}
			//check mod association or inject to TESObjectWEAP at start
		}
	}

	//========================== Animation Functions ===========================

	bool ReloadWeapon(void)
	{
		Actor* playerActor = *g_player;
		PlayerCamera* playerCamera = *g_playerCamera;
		SInt32 state = playerCamera->GetCameraStateId(playerCamera->cameraState);
		TESObjectWEAP* eqWeap = DYNAMIC_CAST(playerActor->equipData->slots[41].item, TESForm, TESObjectWEAP);
		if (!eqWeap)
			return false;
		if (state == 0)
		{
			TESIdleForm* relIdle = MSF_Data::GetReloadAnimation(eqWeap, false);
			Utilities::PlayIdle(playerActor, relIdle);
			return true;
		}
		else if (state == 7 || state == 8)
		{
			TESIdleForm* relIdle = MSF_Data::GetReloadAnimation(eqWeap, true);
			Utilities::PlayIdle(playerActor, relIdle);
			return true;
		}
		return false;
	}

	bool DrawWeapon(void)
	{
		Actor* playerActor = *g_player;
		if (playerActor->actorState.IsWeaponDrawn())
			return false;
		Utilities::DrawWeapon(playerActor);
		return true;
	}

	void BurstWaitAndShoot(BurstMode* data)
	{
		if (!data)
			return;
		UInt8 totalShots = data->flags &= ~0xF0;
		for (data->NumOfRoundsFired; data->NumOfRoundsFired < totalShots; data->NumOfRoundsFired++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(data->wait));
			if (data->animReady)
			{
				data->animReady = false;
				Utilities::PlayIdle(data->actor, data->fireIdle);
			}
			//Onkeyup: if !enableonepullburst end thread, if resetshotcounts (AV) NumOfRoundsFired = 0;
		}
		

	}
	
	bool FireBurst(Actor* actor)
	{
		if (!actor)
			return false;
		float av = Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeTime->formID);
		_MESSAGE("av: %f", av);
		return true;

		TESObjectWEAP* eqWeap = DYNAMIC_CAST(actor->equipData->slots[41].item, TESForm, TESObjectWEAP);
		if (!eqWeap)
			return false;
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(actor->equipData->slots[41].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		if (!instanceData)
			return false;
		if (!actor->middleProcess || !actor->middleProcess->unk08->equipData || !actor->middleProcess->unk08->equipData->equippedData)
			return false;
		if (actor->middleProcess->unk08->equipData->equippedData->unk18 <= 0)
			return false; //if resetshotcountsonreload
		//isAnimPlaying
		UInt16 wait = (UInt16)roundp(Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeTime->formID));
		if (wait < 50)
			return false;
		// check if actor has burst data w/ iterator, if not, make new one
		BurstMode* data = nullptr;
		data->actor = actor;
		data->wait = wait;
		//
		//if (MSF_MainData::tmr.IsRunning())
		//	return false;
		//MSF_MainData::tmr.start();
		data->animReady = false;
		SInt32 state = 8;
		if (actor == *g_player)
		{
			PlayerCamera* playerCamera = *g_playerCamera;
			state = playerCamera->GetCameraStateId(playerCamera->cameraState);
			data->flags = (UInt8)roundp(Utilities::GetActorValue(&actor->actorValueData, MSF_MainData::BurstModeFlags->formID));
		}
		if (state == 0)
		{
			data->fireIdle = MSF_Data::GetFireAnimation(eqWeap, false);
			std::thread(BurstWaitAndShoot, data);
			return true;
		}
		else if (state == 7 || state == 8)
		{
			data->fireIdle = MSF_Data::GetFireAnimation(eqWeap, true);
			std::thread(BurstWaitAndShoot, data);
			return true;
		}
	}

	//========================== Workbench Modding Functions ===========================

	bool ReevalModdedWeapon(TESObjectWEAP* weapon, BGSMod::Attachment::Mod* mod)
	{
		_MESSAGE("wb function called");
		if (!weapon || !mod)
			return false;
		Actor* owner = *g_player;
		//VMArray<BGSInventoryItem::Stack*> stackList;
		for (UInt32 i1 = 0; i1 < owner->inventoryList->items.count; i1++)
		{
			BGSInventoryItem inventoryItem;
			owner->inventoryList->items.GetNthItem(i1, inventoryItem);
			TESObjectWEAP* baseWeapon = DYNAMIC_CAST(inventoryItem.form, TESForm, TESObjectWEAP);
			if (baseWeapon != weapon)
				continue;
			_MESSAGE("weap match %i", i1);
			for (BGSInventoryItem::Stack* stack = inventoryItem.stack; stack; stack = stack->next)
			{
				BGSObjectInstanceExtra * objectModData = DYNAMIC_CAST(stack->extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
				auto data = objectModData->data;
				if (!data || !data->forms)
					continue;
				UInt32 modCount = data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
				BGSMod::Attachment::Mod* objectMod = nullptr;
				for (UInt32 i2 = 0; i2 < modCount; i2++)
				{
					objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i2].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
					if (objectMod == mod)
					{
						_MESSAGE("mod match %i", i2); break;
					}
				}


				//objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[modCount - 1].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
				if (objectMod == mod)
				{
					ExtraInstanceData* extraInstance = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
					TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstance->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
					//for (UInt32 i3 = 0; i3 < modKWstruct.Length(); i3++)
					//{
					//	SwitchData currentLine;
					//	modKWstruct.Get(&currentLine, i3);
					//	BGSKeyword* switchedKW;
					//	currentLine.Get("SwitchedKW", &switchedKW);
					//	if (switchedKW)
					//	{
					//		if ((*instanceData->keywords).HasKeyword(switchedKW))
					//		{
					//			_MESSAGE("has switch KW %i", i3);
					//			BGSKeyword* enableSwitchKW;
					//			currentLine.Get("EnableSwitchKW", &enableSwitchKW);
					//			BGSMod::Attachment::Mod* newMod = nullptr;
					//			if (enableSwitchKW)
					//			{
					//				if (!(*instanceData->keywords).HasKeyword(enableSwitchKW))
					//				{
					//					_MESSAGE("missing enable KW %i", i3);
					//					currentLine.Get("PlaceholderMod", &newMod);
					//				}
					//			}

					bool IsModified = ReevalStackMods(stack->extraData);
					if ((stack->flags & 7) && IsModified)
					{
						owner->middleProcess->unk08->equipData->instanceData = instanceData;
						owner->middleProcess->unk08->equipData->equippedData->ammo = instanceData->ammo;
						owner->middleProcess->unk08->equipData->equippedData->unk18 = instanceData->ammoCapacity;
					}

				}
			}
		}
		return true;
	}




	//========================== Mod Replacing ===========================

	bool ReplaceModExtraData(ExtraDataList * extraDataList, BGSMod::Attachment::Mod* oldMod, BGSMod::Attachment::Mod* newMod, bool AttachLast)
	{
		if (!extraDataList || !oldMod || !newMod)
			return false;
		bool reorder = false;
		BSExtraData * extraData = extraDataList->GetByType(ExtraDataType::kExtraData_ObjectInstance);
		if (extraData)
		{
			BGSObjectInstanceExtra * objectModData = DYNAMIC_CAST(extraData, BSExtraData, BGSObjectInstanceExtra);
			if (objectModData)
			{
				auto data = objectModData->data;
				if (!data || !data->forms)
					return false;
				UInt32 entryNum = data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
				UInt32 unk = 256;
				for (UInt32 i = 0; i < entryNum; i++)
				{
					if (data->forms[i].formId == oldMod->formID)
					{
						if (!AttachLast || (i + 1 == entryNum))
						{
							data->forms[i].formId = newMod->formID;
							return true;
						}
						else
						{
							unk = data->forms[i].unk04;
							data->forms[i].formId = data->forms[i + 1].formId;
							data->forms[i].unk04 = data->forms[i + 1].unk04;
							reorder = true;
						}
					}
					else if (reorder && (i + 1 < entryNum))
					{
						data->forms[i].formId = data->forms[i + 1].formId;
						data->forms[i].unk04 = data->forms[i + 1].unk04;
					}
					else if (reorder)
					{
						data->forms[i].formId = newMod->formID;
						data->forms[i].unk04 = unk;
						return true;
					}
				}
			}
		}
		return false;
	}

	bool ReevalStackMods(ExtraDataList * extraDataList)
	{
		BGSObjectInstanceExtra * objectModData = DYNAMIC_CAST(extraDataList->GetByType(ExtraDataType::kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		auto data = objectModData->data;
		if (!data || !data->forms)
			return false;
		ExtraInstanceData* extraInstance = DYNAMIC_CAST(extraDataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstance->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		bool IsModified = false;
		for (UInt32 i3 = 123; i3 < 127; i3++)
		{
			BGSMod::Attachment::Mod* newMod = nullptr;
			if (i3 == 125)
				newMod = GetNewModIfAmmoIsInvalid(extraDataList);
			else if (i3 == 123)
				newMod = MSF_Data::ValidateScopeMod(objectModData);
			else if (i3 == 124)
				newMod = MSF_Data::ValidateFiringMode(objectModData);
			else if (i3 == 126)
				newMod = MSF_Data::ValidateUnderbarrelMod(objectModData);
			if (!newMod)
				continue;
			IsModified = true;
			BGSMod::Attachment::Mod* oldMod = Utilities::GetFirstModWithPriority(objectModData, i3);
			ReplaceModExtraData(extraDataList, oldMod, newMod, false);
			UpdateInstanceDataLim(extraDataList);
			BSExtraData * extraName = extraDataList->GetByType(ExtraDataType::kExtraData_TextDisplayData);
			ExtraTextDisplayData * textDisplay = DYNAMIC_CAST(extraName, BSExtraData, ExtraTextDisplayData);
			Utilities::RenameAtTextStart(textDisplay, oldMod->fullName.name, newMod->fullName.name);
		}
		return IsModified;
		
	}

	BGSMod::Attachment::Mod* GetNewModIfAmmoIsInvalid(ExtraDataList* dataList)
	{
		_MESSAGE("ammo fn called");
		BGSObjectInstanceExtra* objectModData = DYNAMIC_CAST(dataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;

		if (!instanceData || !objectModData || !weapBase)
			return nullptr;

		auto data = objectModData->data;
		if (!data || !data->forms)
			return nullptr;
		TESAmmo* baseAmmo = nullptr;
		TESAmmo* ammoConverted = nullptr;
		//TESAmmo* switchedAmmo = nullptr;
		UInt8 priority = 0x81;
		//TESAmmo* UBAmmo = nullptr;
		for (UInt32 i3 = 0; i3 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i3++)
		{
			BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i3].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			for (UInt32 i4 = 0; i4 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i4++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i4];
				if (data->target == 61 && data->value.form)
				{
					if (data->op == BGSMod::Container::Data::kOpFlag_Rem_Form)
					{
						baseAmmo = (TESAmmo*)data->value.form; 
						_MESSAGE("specified base ammo: %s", baseAmmo->GetFullName());
					}
					else if ((objectMod->priority < 123 || objectMod->priority > 127) && objectMod->priority >= priority)
					{
						ammoConverted = (TESAmmo*)data->value.form;
						priority = objectMod->priority;
						_MESSAGE("actual base ammo (conv): %s", ammoConverted->GetFullName());
					}
					//else if (objectMod->priority == 125)
					//	switchedAmmo = (TESAmmo*)data->value.form;
					//else if (objectMod->priority == 126)
					//	UBAmmo = (TESAmmo*)data->value.form;
				}
			}
		}
		if (!ammoConverted){
			ammoConverted = weapBase->weapData.ammo; _MESSAGE("actual base ammo: %s", ammoConverted->GetFullName());
		}
		if (baseAmmo)
		{
			if (baseAmmo != ammoConverted)
			{
				for (std::vector<AmmoData>::iterator itAmmoData = MSF_MainData::ammoData.begin(); itAmmoData != MSF_MainData::ammoData.end(); itAmmoData++)
				{
					if (itAmmoData->baseAmmo == ammoConverted)
					{
						return itAmmoData->baseMod;
					}
				}
				return  MSF_MainData::PlaceholderModAmmo;
			}
		}
		return nullptr;
	}

	bool UpdateInstanceDataLim(ExtraDataList* dataList)
	{
		if (!dataList)
			return false;
		
		BGSObjectInstanceExtra* modList = DYNAMIC_CAST(dataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		ExtraTextDisplayData* displayName = DYNAMIC_CAST(dataList->GetByType(kExtraData_TextDisplayData), BSExtraData, ExtraTextDisplayData);
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;


		if (!instanceData || !modList || !displayName || !weapBase)
			return false;

		auto data = modList->data;
		if (!data || !data->forms)
			return false;

		//set base values
		//keywords
		tArray<BGSKeyword*> keywords;
		if (weapBase->keyword.numKeywords > 0)
		{
			for (UInt32 i7 = 0; i7 < weapBase->keyword.numKeywords; i7++)
				keywords.Push(weapBase->keyword.keywords[i7]);
		}
		//damageTypes
		tArray<TBO_InstanceData::DamageTypes> newDamageTypes;
		tArray<TBO_InstanceData::DamageTypes> * baseDamageTypes = nullptr;
		baseDamageTypes = weapBase->weapData.damageTypes;
		if (baseDamageTypes)
		{
			for (UInt32 i13 = 0; i13 < baseDamageTypes->count; i13++)
			{
				TESObjectWEAP::InstanceData::DamageTypes dt;
				baseDamageTypes->GetNthItem(i13, dt);
				newDamageTypes.Push(dt);
			}
		}
		//enchantments
		tArray<EnchantmentItem*> enchs;
		tArray<EnchantmentItem*> * newEnchs = nullptr;
		newEnchs = weapBase->weapData.enchantments;
		if (newEnchs)
		{
			for (UInt32 i12 = 0; i12 < newEnchs->count; i12++)
			{
				EnchantmentItem* ench;
				newEnchs->GetNthItem(i12, ench);
				enchs.Push(ench);
			}
		}


		//AimModel
		MSFAimModel* aimModel = (MSFAimModel*)instanceData->aimModel;
		Utilities::UpdateAimModel(instanceData->aimModel, weapBase->weapData.aimModel);

		//ints
		instanceData->ammoCapacity = weapBase->weapData.ammoCapacity;
		instanceData->baseDamage = weapBase->weapData.baseDamage;
		//instanceData->value = weapBase->weapData.value; unnecessary
		//instanceData->unk134 = weapBase->weapData.unk134; //irank
		instanceData->firingData->numProjectiles = weapBase->weapData.firingData->numProjectiles;

		//floats
		instanceData->actionCost = weapBase->weapData.actionCost;
		instanceData->attackDelay = weapBase->weapData.attackDelay;
		instanceData->critChargeBonus = weapBase->weapData.critChargeBonus;
		instanceData->critDamageMult = weapBase->weapData.critDamageMult;
		instanceData->fullPowerSeconds = weapBase->weapData.fullPowerSeconds;
		instanceData->maxRange = weapBase->weapData.maxRange;
		instanceData->minPowerShot = weapBase->weapData.minPowerShot;
		instanceData->minRange = weapBase->weapData.minRange;
		instanceData->outOfRangeMultiplier = weapBase->weapData.outOfRangeMultiplier;
		instanceData->reach = weapBase->weapData.reach;
		instanceData->reloadSpeed = weapBase->weapData.reloadSpeed;
		instanceData->secondary = weapBase->weapData.secondary;
		instanceData->firingData->sightedTransition = weapBase->weapData.firingData->sightedTransition;
		//instanceData->unkEC = weapBase->weapData.unkEC; //fSoundLevelMult
		instanceData->speed = weapBase->weapData.speed;
		//instanceData->weight = weapBase->weapData.weight; unnecessary
		//zoom data camera offset + imagespace

		//anim fix floats
		//instanceData->unkC0; //attack seconds
		//instanceData->firingData->unk1C; //fire seconds?
		//instanceData->firingData->unk18; //reload seconds



		//forms
		instanceData->ammo = weapBase->weapData.ammo;
		instanceData->unk58 = weapBase->weapData.unk58; //impact data
		instanceData->firingData->projectileOverride = weapBase->weapData.firingData->projectileOverride;
		instanceData->zoomData = weapBase->weapData.zoomData;
		instanceData->addAmmoList = weapBase->weapData.addAmmoList; //npc ammo list
		instanceData->unk78 = weapBase->weapData.unk78; //crit effect
		//instanceData->equipSlot = weapBase->weapData.equipSlot; unnecessary
		//bash impact data, block material

		////sounds
		//instanceData->unk28 = weapBase->weapData.unk28; //psAttackFailSound = 55
		//instanceData->unk10 = weapBase->weapData.unk10; //psAttackSound = 52 
		//instanceData->unk38 = weapBase->weapData.unk38; //psEquipSound = 57 
		//instanceData->unk48 = weapBase->weapData.unk48; //psFastEquipSound = 91 
		//instanceData->unk30 = weapBase->weapData.unk30; //psIdleSound = 56 
		//instanceData->unk40 = weapBase->weapData.unk40; //psUnEquipSound = 58 
		//instanceData->unk20 = weapBase->weapData.unk20; //psAttackLoop = 54 && use reinterpret_cast
		//instanceData->unk18 = weapBase->weapData.unk18; //psAttackSound2D = 53 && use reinterpret_cast

		//misc
		instanceData->flags = weapBase->weapData.flags;
		instanceData->stagger = weapBase->weapData.stagger;
		instanceData->unk118 = weapBase->weapData.unk118;
		instanceData->unk114 = weapBase->weapData.unk114;
		
		float fv1 = 0.0;
		//float fv2 = 0;
		UInt32 iv1 = 0;
		//UInt32 iv2 = 0;

		tArray<BGSMod::Attachment::Mod*> objMods;
		BGSMod::Attachment::Mod* newmod = nullptr;
		UInt8 priority = 0;
		UInt8 pr = 0;
		for (UInt32 i1 = 0; i1 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i1++)
		{
			for (UInt32 i2 = 0; i2 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i2++)
			{
				BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i2].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
				if (objMods.GetItemIndex(mod) >= 0)
					continue;
				if (mod->priority >= 128)
					pr = mod->priority - 128;
				else
					pr = mod->priority + 128;
				if (priority > pr || !newmod)
				{
					newmod = mod;
					priority = pr;
					//_MESSAGE(">");
				}
			}
			objMods.Push(newmod);
			//_MESSAGE("adding mod: %08X pr %i", newmod->formID, newmod->priority);
			newmod = nullptr;
		}

		for (UInt32 i1 = 0; i1 < objMods.count; i1++)
		{
			BGSMod::Attachment::Mod* objectMod = objMods[i1];
			for (UInt32 i2 = 0; i2 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i2++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i2];
				UInt32 targetType = BGSMod::Container::kWeaponTarget_Offset + data->target;
				//_MESSAGE("rd %i, op: %i", i2, data->op);
				switch (data->op)
				{
				case BGSMod::Container::Data::kOpFlag_Set_Bool:
				{
					iv1 = data->value.i.v1;
					//iv2 = data->value.i.v2;
					switch (targetType)
					{
					case 51: instanceData->firingData->numProjectiles = iv1; break;
					case 28: instanceData->baseDamage = iv1; break;
					case 37: aimModel->CoF_DecrDelayMS = iv1; break;
					case 44: aimModel->Rec_RunawayShots = iv1; break;
					}

				}
					break;
				case BGSMod::Container::Data::kOpFlag_Add_Int:
				{
					iv1 = data->value.i.v1;
					//iv2 = data->value.i.v2;
					switch (targetType)
					{
					case 51: instanceData->firingData->numProjectiles += iv1; break;
					case 28: instanceData->baseDamage += iv1; break;
					case 37: aimModel->CoF_DecrDelayMS += iv1; break;
					case 44: aimModel->Rec_RunawayShots += iv1; break;
					}

				}
					break;

				case BGSMod::Container::Data::kOpFlag_Set_Float:
				{
					fv1 = data->value.f.v1;
					//fv2 = data->value.f.v2;
					switch (targetType)
					{
					case 79: instanceData->actionCost = fv1; break;
					case 4: instanceData->attackDelay = fv1; break;
					case 8: instanceData->critChargeBonus = fv1; break;
					case 90: instanceData->critDamageMult = fv1; break;
					case 84: instanceData->fullPowerSeconds = fv1; break;
					case 3: instanceData->maxRange = fv1; break;
					case 87: instanceData->minPowerShot = fv1; break;
					case 2: instanceData->minRange = fv1; break;
					case 6: instanceData->outOfRangeMultiplier = fv1; break;
					case 1: instanceData->reach = fv1; break;
					case 76: instanceData->reloadSpeed = fv1; break;
					case 7: instanceData->secondary = fv1; break;
					case 83: instanceData->firingData->sightedTransition = fv1; break;
					case 0: instanceData->speed = fv1; break;
					case 41: aimModel->Rec_MaxPerShot = fv1; aimModel->formID = 0; break;
					case 42: aimModel->Rec_MinPerShot = fv1; aimModel->formID = 0; break;
					case 43: aimModel->Rec_HipMult = fv1; aimModel->formID = 0; break;
					case 45: aimModel->Rec_ArcMaxDegrees = fv1; aimModel->formID = 0; break;
					case 46: aimModel->Rec_ArcRotate = fv1; aimModel->formID = 0; break;
					case 40: aimModel->Rec_DimSightsMult = fv1; aimModel->formID = 0; break;
					case 39: aimModel->Rec_DimSpringForce = fv1; aimModel->formID = 0; break;
					case 33: aimModel->CoF_MinAngle = fv1; aimModel->formID = 0; break;
					case 34: aimModel->CoF_MaxAngle = fv1; aimModel->formID = 0; break;
					case 35: aimModel->CoF_IncrPerShot = fv1; aimModel->formID = 0; break;
					case 36: aimModel->CoF_DecrPerSec = fv1; aimModel->formID = 0; break;
					case 38: aimModel->CoF_SneakMult = fv1; aimModel->formID = 0; break;
					case 47: aimModel->CoF_IronSightsMult = fv1; aimModel->formID = 0; break;
					case 66: aimModel->BaseStability = fv1; aimModel->formID = 0; break;
					}
				}
					break;

				case BGSMod::Container::Data::kOpFlag_Mul_Add_Float:
				{
					fv1 = data->value.f.v1;
					//fv2 = data->value.f.v2;
					switch (targetType)
					{
					case 51: instanceData->firingData->numProjectiles += roundp(fv1*instanceData->firingData->numProjectiles); break;
					case 28: instanceData->baseDamage += roundp(fv1*instanceData->baseDamage); break;
					case 37: aimModel->CoF_DecrDelayMS += roundp(fv1*aimModel->CoF_DecrDelayMS); break;
					case 44: aimModel->Rec_RunawayShots += roundp(fv1*aimModel->Rec_RunawayShots); break;

					case 79: instanceData->actionCost += (fv1*instanceData->actionCost); break;
					case 4: instanceData->attackDelay += (fv1*instanceData->attackDelay); break;
					case 8: instanceData->critChargeBonus += (fv1*instanceData->critChargeBonus); break;
					case 90: instanceData->critDamageMult += (fv1*instanceData->critDamageMult); break;
					case 84: instanceData->fullPowerSeconds += (fv1*instanceData->fullPowerSeconds); break;
					case 3: instanceData->maxRange += (fv1*instanceData->maxRange); break;
					case 87: instanceData->minPowerShot += (fv1*instanceData->minPowerShot); break;
					case 2: instanceData->minRange += (fv1*instanceData->minRange); break;
					case 6: instanceData->outOfRangeMultiplier += (fv1*instanceData->outOfRangeMultiplier); break;
					case 1: instanceData->reach += (fv1*instanceData->reach); break;
					case 76: instanceData->reloadSpeed += (fv1*instanceData->reloadSpeed); break;
					case 7: instanceData->secondary += (fv1*instanceData->secondary); break;
					case 83: instanceData->firingData->sightedTransition += (fv1*instanceData->firingData->sightedTransition); break;
					case 0: instanceData->speed += (fv1*instanceData->speed); break;
					case 41: aimModel->Rec_MaxPerShot += (fv1*aimModel->Rec_MaxPerShot); aimModel->formID = 0; break;
					case 42: aimModel->Rec_MinPerShot += (fv1*aimModel->Rec_MinPerShot); aimModel->formID = 0; break;
					case 43: aimModel->Rec_HipMult += (fv1*aimModel->Rec_HipMult); aimModel->formID = 0; break;
					case 45: aimModel->Rec_ArcMaxDegrees += (fv1*aimModel->Rec_ArcMaxDegrees); aimModel->formID = 0; break;
					case 46: aimModel->Rec_ArcRotate += (fv1*aimModel->Rec_ArcRotate); aimModel->formID = 0; break;
					case 40: aimModel->Rec_DimSightsMult += (fv1*aimModel->Rec_DimSightsMult); aimModel->formID = 0; break;
					case 39: aimModel->Rec_DimSpringForce += (fv1*aimModel->Rec_DimSpringForce); aimModel->formID = 0; break;
					case 33: aimModel->CoF_MinAngle += (fv1*aimModel->CoF_MinAngle); aimModel->formID = 0; break;
					case 34: aimModel->CoF_MaxAngle += (fv1*aimModel->CoF_MaxAngle); aimModel->formID = 0; break;
					case 35: aimModel->CoF_IncrPerShot += (fv1*aimModel->CoF_IncrPerShot); aimModel->formID = 0; break;
					case 36: aimModel->CoF_DecrPerSec += (fv1*aimModel->CoF_DecrPerSec); aimModel->formID = 0; break;
					case 38: aimModel->CoF_SneakMult += (fv1*aimModel->CoF_SneakMult); aimModel->formID = 0; break;
					case 47: aimModel->CoF_IronSightsMult += (fv1*aimModel->CoF_IronSightsMult); aimModel->formID = 0; break;
					case 66: aimModel->BaseStability += (fv1*aimModel->BaseStability); aimModel->formID = 0; break;
					}
				}
					break;
				case BGSMod::Container::Data::kOpFlag_Add_Float:
				{
					fv1 = data->value.f.v1;
					//fv2 = data->value.f.v2;
					switch (targetType)
					{
					case 79: instanceData->actionCost += fv1; break;
					case 4: instanceData->attackDelay += fv1; break;
					case 8: instanceData->critChargeBonus += fv1; break;
					case 90: instanceData->critDamageMult += fv1; break;
					case 84: instanceData->fullPowerSeconds += fv1; break;
					case 3: instanceData->maxRange += fv1; break;
					case 87: instanceData->minPowerShot += fv1; break;
					case 2: instanceData->minRange += fv1; break;
					case 6: instanceData->outOfRangeMultiplier += fv1; break;
					case 1: instanceData->reach += fv1; break;
					case 76: instanceData->reloadSpeed += fv1; break;
					case 7: instanceData->secondary += fv1; break;
					case 83: instanceData->firingData->sightedTransition += fv1; break;
					case 0: instanceData->speed += fv1; break;
					case 41: aimModel->Rec_MaxPerShot += fv1; aimModel->formID = 0; break;
					case 42: aimModel->Rec_MinPerShot += fv1; aimModel->formID = 0; break;
					case 43: aimModel->Rec_HipMult += fv1; aimModel->formID = 0; break;
					case 45: aimModel->Rec_ArcMaxDegrees += fv1; aimModel->formID = 0; break;
					case 46: aimModel->Rec_ArcRotate += fv1; aimModel->formID = 0; break;
					case 40: aimModel->Rec_DimSightsMult += fv1; aimModel->formID = 0; break;
					case 39: aimModel->Rec_DimSpringForce += fv1; aimModel->formID = 0; break;
					case 33: aimModel->CoF_MinAngle += fv1; aimModel->formID = 0; break;
					case 34: aimModel->CoF_MaxAngle += fv1; aimModel->formID = 0; break;
					case 35: aimModel->CoF_IncrPerShot += fv1; aimModel->formID = 0; break;
					case 36: aimModel->CoF_DecrPerSec += fv1; aimModel->formID = 0; break;
					case 38: aimModel->CoF_SneakMult += fv1; aimModel->formID = 0; break;
					case 47: aimModel->CoF_IronSightsMult += fv1; aimModel->formID = 0; break;
					case 66: aimModel->BaseStability += fv1; aimModel->formID = 0; break;
					}
				}
					break;
				case BGSMod::Container::Data::kOpFlag_Set_Enum: 
				{
					iv1 = data->value.i.v1;
					switch (targetType)
					{
						case 82: instanceData->stagger = iv1; break;
						case 9: instanceData->unk118 = iv1; break; //ehHitBehavior
						case 59: instanceData->unk114 = iv1; break; //elSoundLevel
						//case 68: instanceData->stagger = iv1; break; eoZoomDataOverlay
						//case 15: instanceData->stagger = iv1; break; ewType
					}
				}
				case BGSMod::Container::Data::kOpFlag_Set_Form:
				case BGSMod::Container::Data::kOpFlag_Add_Form: //?, need if?
				{
					switch (targetType)
					{
					case 61: //always SET
					{
						TESAmmo* ammo = (TESAmmo*)data->value.form;
						if (ammo)
							instanceData->ammo = ammo;
					}
						break;
					case 60: //always SET
					{
						BGSImpactDataSet* impactData = (BGSImpactDataSet*)data->value.form;
						if (impactData)
							instanceData->unk58 = impactData;
					}
						break;
					case 80: //always SET
					{
						BGSProjectile* proj = (BGSProjectile*)data->value.form;
						instanceData->firingData->projectileOverride = proj;
					}
						break;
					case 67: //always SET
					{
						BGSZoomData* zoomData = (BGSZoomData*)data->value.form;
						if (zoomData)
							instanceData->zoomData = zoomData;
					}
						break;
					case 75: //always SET
					{
						TESLevItem* NPCammolist = (TESLevItem*)data->value.form;
						if (NPCammolist)
							instanceData->addAmmoList = NPCammolist;
					}
						break;
					case 62: //always SET
					{
						SpellItem* criteffect = (SpellItem*)data->value.form;
						if (criteffect)
							instanceData->unk78 = criteffect;
					}
						break;
					case 32: //always SET
					{
						BGSAimModel* newModel = (BGSAimModel*)data->value.form; //dincast
						if (newModel)
							Utilities::UpdateAimModel(instanceData->aimModel, newModel); //aimModel->formID = data->value.form->formID;
					}
						break;
					case 65:
					{
						EnchantmentItem* ench = (EnchantmentItem*)data->value.form;
						if (!ench)
							break;
						SInt64 index = enchs.GetItemIndex(ench);
						if (index < 0)
							enchs.Push(ench);
					}
						break;
					case 31:
					{
						BGSKeyword* keyword = (BGSKeyword*)data->value.form;
						if (!keyword)
							break;
						SInt64 index = keywords.GetItemIndex(keyword);
						if (index < 0)
							keywords.Push(keyword);
					}
						break;
						//psSounds
					}
				}
					break;
				case BGSMod::Container::Data::kOpFlag_Rem_Form: //??? if rem form==current -> set base
				{
					switch (targetType)
					{
					case 65:
					{
						EnchantmentItem* ench = (EnchantmentItem*)data->value.form;
						if (!ench)
							break;
						SInt64 index = enchs.GetItemIndex(ench);
						if (index >= 0)
							enchs.Remove(index);
					}
						break;
					case 31:
					{
						BGSKeyword* keyword = (BGSKeyword*)data->value.form;
						if (!keyword)
							break;
						SInt64 index = keywords.GetItemIndex(keyword);
						if (index >= 0)
							keywords.Remove(index);
					}
						break;
					}
				}
					break;
				case BGSMod::Container::Data::kOpFlag_Set_Int:
				case BGSMod::Container::Data::kOpFlag_Or_Bool:
				case BGSMod::Container::Data::kOpFlag_And_Bool:
				{
					UInt8 op = data->op;
					switch (targetType)
					{
					case 25: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0008000, data->value.i.v1, op); break; //-auto
					case 81: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0400000, data->value.i.v1, op); break; //-BoltAction
					case 26: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0020000, data->value.i.v1, op); break; //-CantDrop
					case 93: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000200, data->value.i.v1, op); break; //-ChargingAttack
					case 18: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000008, data->value.i.v1, op); break; //-ChargingReload
					case 92: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x1000000, data->value.i.v1, op); break; //-DisableShells
					case 21: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000100, data->value.i.v1, op); break; //effectondeath
					case 20: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000020, data->value.i.v1, op); break; //fixedrange
					case 48: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0200000, data->value.i.v1, op); break; //-kFlag_HasScope
					case 85: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000800, data->value.i.v1, op); break; //-HoldInputToPower
					case 24: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0004000, data->value.i.v1, op); break; //-IgnoresNormalResist
					case 19: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000010, data->value.i.v1, op); break; //-MinorCrime
					case 23: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0001000, data->value.i.v1, op); break; //-NonHostile
					case 27: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0100000, data->value.i.v1, op); break; //-NotUsedInNormalCombat
					case 17: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000002, data->value.i.v1, op); break; //-NPCsUseAmmo
					case 16: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000001, data->value.i.v1, op); break; //playeronly
					case 86: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0010000, data->value.i.v1, op); break; //-RepeatableSingleFire
					case 22: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000400, data->value.i.v1, op); break; //alternaterumble
					}
				}
					break;
				case BGSMod::Container::Data::kOpFlag_Set_Form_Float:
				case BGSMod::Container::Data::kOpFlag_Add_Form_Float:
				case BGSMod::Container::Data::kOpFlag_Mul_Add_Form_Float:
				{
					switch (targetType)
					{
					case 77:
					{
						BGSDamageType* dmgtype = (BGSDamageType*)LookupFormByID(data->value.ff.formId);
						if (!dmgtype)
							break;
						bool HasDmgType = false;
						TESObjectWEAP::InstanceData::DamageTypes dt;
						for (UInt32 i9 = 0; i9 < newDamageTypes.count; i9++)
						{
							newDamageTypes.GetNthItem(i9, dt);
							if (dt.damageType == dmgtype)
							{
								HasDmgType = true;
								break;
							}
						}
						if (!HasDmgType)
						{
							dt.damageType = dmgtype;
							dt.value = 0;
						}
						switch (data->op)
						{
						case BGSMod::Container::Data::kOpFlag_Set_Form_Float: dt.value = roundp(data->value.ff.v2); break;
						case BGSMod::Container::Data::kOpFlag_Add_Form_Float: dt.value += roundp(data->value.ff.v2); break;
						case BGSMod::Container::Data::kOpFlag_Mul_Add_Form_Float: dt.value += roundp(data->value.ff.v2*dt.value); break;
						}
						if (dt.value > 0)
							newDamageTypes.Push(dt);
					}
						break;
						//vaActorValue
					}
				}
					break;
				}
			//
			}
		}
		//keywords
		if (instanceData->keywords)
		{
			Heap_Free(instanceData->keywords->keywords);
			instanceData->keywords->keywords = nullptr;
			instanceData->keywords->numKeywords = 0;
			if (keywords.count > 0)
			{
				instanceData->keywords->keywords = (BGSKeyword**)Heap_Allocate(sizeof(BGSKeyword*) * keywords.count);
				instanceData->keywords->numKeywords = keywords.count;
				for (UInt32 i3 = 0; i3 < keywords.count; i3++)
					instanceData->keywords->keywords[i3] = keywords[i3];
			}
			
		}
		//enchantments
		tArray<EnchantmentItem*> ** enchantments = nullptr;
		enchantments = &instanceData->enchantments;
		if (!(*enchantments)) //only if array>0
			(*enchantments) = new tArray<EnchantmentItem*>();
		(*enchantments)->Clear();
		for (UInt32 i9 = 0; i9 < enchs.count; i9++)
		{
			EnchantmentItem* ench;
			enchs.GetNthItem(i9, ench);
			(*enchantments)->Push(ench);
		}
		//damageTypes
		tArray<TBO_InstanceData::DamageTypes> ** damageTypes = nullptr;
		_MESSAGE("end dt");
		damageTypes = &instanceData->damageTypes;
		if (!(*damageTypes)) //only if array>0
			(*damageTypes) = new tArray<TBO_InstanceData::DamageTypes>();
		(*damageTypes)->Clear();
		for (UInt32 i8 = 0; i8 < newDamageTypes.count; i8++)
		{
			TESObjectWEAP::InstanceData::DamageTypes dt;
			newDamageTypes.GetNthItem(i8, dt);
			(*damageTypes)->Push(dt);
		}
		return true;
	}

	bool UpdateInstanceDataWithMod(BGSInventoryItem::Stack* stack, BGSMod::Attachment::Mod* newMod)
	{
		if (!stack)
			return false;
		ExtraDataList* dataList = stack->extraData;
		if (!dataList)
			return false;
		
		BGSObjectInstanceExtra* modList = DYNAMIC_CAST(dataList->GetByType(kExtraData_ObjectInstance), BSExtraData, BGSObjectInstanceExtra);
		ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(dataList->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		ExtraTextDisplayData* displayName = DYNAMIC_CAST(dataList->GetByType(kExtraData_TextDisplayData), BSExtraData, ExtraTextDisplayData);
		TESObjectWEAP* weapBase = (TESObjectWEAP*)extraInstanceData->baseForm;

		if (!instanceData || !modList || !displayName || !weapBase)
			return false;

		auto data = modList->data;
		if (!data || !data->forms)
			return false;

		std::vector<BGSMod::Attachment::Mod*> objMods;
		for (UInt32 i1 = 0; i1 < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i1++)
		{
			BGSMod::Attachment::Mod* mod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[i1].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
			objMods.push_back(mod);
		}

		std::vector<StackModAssociation>::iterator itStack = MSF_MainData::stackMods.begin();
		for (itStack; itStack != MSF_MainData::stackMods.end(); itStack++)
		{
			if (itStack->stack == stack) //or instance data?
			{
				if (newMod)
				{
					switch (newMod->priority)
					{
					case 123: itStack->ModZD = newMod; break;
					case 124: itStack->ModFM = newMod; break;
					case 125: itStack->ModAmmo = newMod; break;
					case 126: itStack->ModUB = newMod; break;
					default: return false;
					}
				}
				if (itStack->ModZD)
					objMods.push_back(itStack->ModZD);
				if (itStack->ModFM)
					objMods.push_back(itStack->ModFM);
				if (itStack->ModAmmo)
					objMods.push_back(itStack->ModAmmo);
				if (itStack->ModUB)
					objMods.push_back(itStack->ModUB);
				break;
			}
		}

		if (itStack == MSF_MainData::stackMods.end() && newMod)
		{
			StackModAssociation assoc;
			assoc.stack = stack;
			assoc.ModAmmo = nullptr;
			assoc.ModFM = nullptr;
			assoc.ModUB = nullptr;
			assoc.ModZD = nullptr;
			switch (newMod->priority)
			{
			case 123: assoc.ModZD = newMod; break;
			case 124: assoc.ModFM = newMod; break;
			case 125: assoc.ModAmmo = newMod; break;
			case 126: assoc.ModUB = newMod; break;
			default: return false;
			}
			MSF_MainData::stackMods.push_back(assoc);
			objMods.push_back(newMod);
		}

		//set base values
		//keywords
		tArray<BGSKeyword*> keywords;
		if (weapBase->keyword.numKeywords > 0)
		{
			for (UInt32 i7 = 0; i7 < weapBase->keyword.numKeywords; i7++)
				keywords.Push(weapBase->keyword.keywords[i7]);
		}
		//damageTypes
		tArray<TBO_InstanceData::DamageTypes> newDamageTypes;
		tArray<TBO_InstanceData::DamageTypes> * baseDamageTypes = nullptr;
		baseDamageTypes = weapBase->weapData.damageTypes;
		if (baseDamageTypes)
		{
			for (UInt32 i13 = 0; i13 < baseDamageTypes->count; i13++)
			{
				TESObjectWEAP::InstanceData::DamageTypes dt;
				baseDamageTypes->GetNthItem(i13, dt);
				newDamageTypes.Push(dt);
			}
		}
		//enchantments
		tArray<EnchantmentItem*> enchs;
		tArray<EnchantmentItem*> * newEnchs = nullptr;
		newEnchs = weapBase->weapData.enchantments;
		if (newEnchs)
		{
			for (UInt32 i12 = 0; i12 < newEnchs->count; i12++)
			{
				EnchantmentItem* ench;
				newEnchs->GetNthItem(i12, ench);
				enchs.Push(ench);
			}
		}
		//AimModel
		MSFAimModel* aimModel = (MSFAimModel*)instanceData->aimModel;
		Utilities::UpdateAimModel(instanceData->aimModel, weapBase->weapData.aimModel);

		//ints
		instanceData->ammoCapacity = weapBase->weapData.ammoCapacity;
		instanceData->baseDamage = weapBase->weapData.baseDamage;
		//instanceData->value = weapBase->weapData.value; unnecessary
		//instanceData->unk134 = weapBase->weapData.unk134; //irank
		instanceData->firingData->numProjectiles = weapBase->weapData.firingData->numProjectiles;

		//floats
		instanceData->actionCost = weapBase->weapData.actionCost;
		instanceData->attackDelay = weapBase->weapData.attackDelay;
		instanceData->critChargeBonus = weapBase->weapData.critChargeBonus;
		instanceData->critDamageMult = weapBase->weapData.critDamageMult;
		instanceData->fullPowerSeconds = weapBase->weapData.fullPowerSeconds;
		instanceData->maxRange = weapBase->weapData.maxRange;
		instanceData->minPowerShot = weapBase->weapData.minPowerShot;
		instanceData->minRange = weapBase->weapData.minRange;
		instanceData->outOfRangeMultiplier = weapBase->weapData.outOfRangeMultiplier;
		instanceData->reach = weapBase->weapData.reach;
		instanceData->reloadSpeed = weapBase->weapData.reloadSpeed;
		instanceData->secondary = weapBase->weapData.secondary;
		instanceData->firingData->sightedTransition = weapBase->weapData.firingData->sightedTransition;
		//instanceData->unkEC = weapBase->weapData.unkEC; //fSoundLevelMult
		instanceData->speed = weapBase->weapData.speed;
		//instanceData->weight = weapBase->weapData.weight; unnecessary
		//zoom data camera offset + imagespace

		//anim fix floats
		//instanceData->unkC0; //attack seconds
		//instanceData->firingData->unk1C; //fire seconds?
		//instanceData->firingData->unk18; //reload seconds

		//forms
		instanceData->ammo = weapBase->weapData.ammo;
		instanceData->unk58 = weapBase->weapData.unk58; //impact data
		instanceData->firingData->projectileOverride = weapBase->weapData.firingData->projectileOverride;
		instanceData->zoomData = weapBase->weapData.zoomData;
		instanceData->addAmmoList = weapBase->weapData.addAmmoList; //npc ammo list
		instanceData->unk78 = weapBase->weapData.unk78; //crit effect
		//instanceData->equipSlot = weapBase->weapData.equipSlot; unnecessary
		//bash impact data, block material

		////sounds
		//instanceData->unk28 = weapBase->weapData.unk28; //psAttackFailSound = 55
		//instanceData->unk10 = weapBase->weapData.unk10; //psAttackSound = 52 
		//instanceData->unk38 = weapBase->weapData.unk38; //psEquipSound = 57 
		//instanceData->unk48 = weapBase->weapData.unk48; //psFastEquipSound = 91 
		//instanceData->unk30 = weapBase->weapData.unk30; //psIdleSound = 56 
		//instanceData->unk40 = weapBase->weapData.unk40; //psUnEquipSound = 58 
		//instanceData->unk20 = weapBase->weapData.unk20; //psAttackLoop = 54 && use reinterpret_cast
		//instanceData->unk18 = weapBase->weapData.unk18; //psAttackSound2D = 53 && use reinterpret_cast

		//misc
		instanceData->flags = weapBase->weapData.flags;
		instanceData->stagger = weapBase->weapData.stagger;
		instanceData->unk118 = weapBase->weapData.unk118;
		instanceData->unk114 = weapBase->weapData.unk114;

		float fv1 = 0.0;
		UInt32 iv1 = 0;

		for (UInt32 i1 = 0; i1 < objMods.size(); i1++)
		{
			BGSMod::Attachment::Mod* objectMod = objMods[i1];
			//_MESSAGE("mod %i", i1);
			//_MESSAGE("formid %08X", objectMod->formID); //objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data));
			for (UInt32 i2 = 0; i2 < objectMod->modContainer.dataSize / sizeof(BGSMod::Container::Data); i2++)
			{
				BGSMod::Container::Data * data = &objectMod->modContainer.data[i2];
				UInt32 targetType = BGSMod::Container::kWeaponTarget_Offset + data->target;

				//_MESSAGE("rd %i, op: %i", i2, data->op);
				switch (data->op)
				{
				case BGSMod::Container::Data::kOpFlag_Set_Bool:
				{
					iv1 = data->value.i.v1;
					//iv2 = data->value.i.v2;
					switch (targetType)
					{
					case 51: instanceData->firingData->numProjectiles = iv1; break;
					case 28: instanceData->baseDamage = iv1; break;
					case 37: aimModel->CoF_DecrDelayMS = iv1; break;
					case 44: aimModel->Rec_RunawayShots = iv1; break;
					}

				}
				break;
				case BGSMod::Container::Data::kOpFlag_Add_Int:
				{
					iv1 = data->value.i.v1;
					//iv2 = data->value.i.v2;
					switch (targetType)
					{
					case 51: instanceData->firingData->numProjectiles += iv1; break;
					case 28: instanceData->baseDamage += iv1; break;
					case 37: aimModel->CoF_DecrDelayMS += iv1; break;
					case 44: aimModel->Rec_RunawayShots += iv1; break;

					}
				}
				break;

				case BGSMod::Container::Data::kOpFlag_Set_Float:
				{
					fv1 = data->value.f.v1;
					//fv2 = data->value.f.v2;
					switch (targetType)
					{
					case 79: instanceData->actionCost = fv1; break;
					case 4: instanceData->attackDelay = fv1; break;
					case 8: instanceData->critChargeBonus = fv1; break;
					case 90: instanceData->critDamageMult = fv1; break;
					case 84: instanceData->fullPowerSeconds = fv1; break;
					case 3: instanceData->maxRange = fv1; break;
					case 87: instanceData->minPowerShot = fv1; break;
					case 2: instanceData->minRange = fv1; break;
					case 6: instanceData->outOfRangeMultiplier = fv1; break;
					case 1: instanceData->reach = fv1; break;
					case 76: instanceData->reloadSpeed = fv1; break;
					case 7: instanceData->secondary = fv1; break;
					case 83: instanceData->firingData->sightedTransition = fv1; break;
					case 0: instanceData->speed = fv1; break;
					case 41: aimModel->Rec_MaxPerShot = fv1; aimModel->formID = 0; break;
					case 42: aimModel->Rec_MinPerShot = fv1; aimModel->formID = 0; break;
					case 43: aimModel->Rec_HipMult = fv1; aimModel->formID = 0; break;
					case 45: aimModel->Rec_ArcMaxDegrees = fv1; aimModel->formID = 0; break;
					case 46: aimModel->Rec_ArcRotate = fv1; aimModel->formID = 0; break;
					case 40: aimModel->Rec_DimSightsMult = fv1; aimModel->formID = 0; break;
					case 39: aimModel->Rec_DimSpringForce = fv1; aimModel->formID = 0; break;
					case 33: aimModel->CoF_MinAngle = fv1; aimModel->formID = 0; break;
					case 34: aimModel->CoF_MaxAngle = fv1; aimModel->formID = 0; break;
					case 35: aimModel->CoF_IncrPerShot = fv1; aimModel->formID = 0; break;
					case 36: aimModel->CoF_DecrPerSec = fv1; aimModel->formID = 0; break;
					case 38: aimModel->CoF_SneakMult = fv1; aimModel->formID = 0; break;
					case 47: aimModel->CoF_IronSightsMult = fv1; aimModel->formID = 0; break;
					case 66: aimModel->BaseStability = fv1; aimModel->formID = 0; break;
					}
				}
				break;

				case BGSMod::Container::Data::kOpFlag_Mul_Add_Float:
				{
					fv1 = data->value.f.v1;
					//fv2 = data->value.f.v2;
					switch (targetType)
					{
					case 51: instanceData->firingData->numProjectiles += roundp(fv1*instanceData->firingData->numProjectiles); break;
					case 28: instanceData->baseDamage += roundp(fv1*instanceData->baseDamage); break;
					case 37: aimModel->CoF_DecrDelayMS += roundp(fv1*aimModel->CoF_DecrDelayMS); break;
					case 44: aimModel->Rec_RunawayShots += roundp(fv1*aimModel->Rec_RunawayShots); break;

					case 79: instanceData->actionCost += (fv1*instanceData->actionCost); break;
					case 4: instanceData->attackDelay += (fv1*instanceData->attackDelay); break;
					case 8: instanceData->critChargeBonus += (fv1*instanceData->critChargeBonus); break;
					case 90: instanceData->critDamageMult += (fv1*instanceData->critDamageMult); break;
					case 84: instanceData->fullPowerSeconds += (fv1*instanceData->fullPowerSeconds); break;
					case 3: instanceData->maxRange += (fv1*instanceData->maxRange); break;
					case 87: instanceData->minPowerShot += (fv1*instanceData->minPowerShot); break;
					case 2: instanceData->minRange += (fv1*instanceData->minRange); break;
					case 6: instanceData->outOfRangeMultiplier += (fv1*instanceData->outOfRangeMultiplier); break;
					case 1: instanceData->reach += (fv1*instanceData->reach); break;
					case 76: instanceData->reloadSpeed += (fv1*instanceData->reloadSpeed); break;
					case 7: instanceData->secondary += (fv1*instanceData->secondary); break;
					case 83: instanceData->firingData->sightedTransition += (fv1*instanceData->firingData->sightedTransition); break;
					case 0: instanceData->speed += (fv1*instanceData->speed); break;
					case 41: aimModel->Rec_MaxPerShot += (fv1*aimModel->Rec_MaxPerShot); aimModel->formID = 0; break;
					case 42: aimModel->Rec_MinPerShot += (fv1*aimModel->Rec_MinPerShot); aimModel->formID = 0; break;
					case 43: aimModel->Rec_HipMult += (fv1*aimModel->Rec_HipMult); aimModel->formID = 0; break;
					case 45: aimModel->Rec_ArcMaxDegrees += (fv1*aimModel->Rec_ArcMaxDegrees); aimModel->formID = 0; break;
					case 46: aimModel->Rec_ArcRotate += (fv1*aimModel->Rec_ArcRotate); aimModel->formID = 0; break;
					case 40: aimModel->Rec_DimSightsMult += (fv1*aimModel->Rec_DimSightsMult); aimModel->formID = 0; break;
					case 39: aimModel->Rec_DimSpringForce += (fv1*aimModel->Rec_DimSpringForce); aimModel->formID = 0; break;
					case 33: aimModel->CoF_MinAngle += (fv1*aimModel->CoF_MinAngle); aimModel->formID = 0; break;
					case 34: aimModel->CoF_MaxAngle += (fv1*aimModel->CoF_MaxAngle); aimModel->formID = 0; break;
					case 35: aimModel->CoF_IncrPerShot += (fv1*aimModel->CoF_IncrPerShot); aimModel->formID = 0; break;
					case 36: aimModel->CoF_DecrPerSec += (fv1*aimModel->CoF_DecrPerSec); aimModel->formID = 0; break;
					case 38: aimModel->CoF_SneakMult += (fv1*aimModel->CoF_SneakMult); aimModel->formID = 0; break;
					case 47: aimModel->CoF_IronSightsMult += (fv1*aimModel->CoF_IronSightsMult); aimModel->formID = 0; break;
					case 66: aimModel->BaseStability += (fv1*aimModel->BaseStability); aimModel->formID = 0; break;
					}
				}
				break;
				case BGSMod::Container::Data::kOpFlag_Add_Float:
				{
					fv1 = data->value.f.v1;
					//fv2 = data->value.f.v2;
					switch (targetType)
					{
					case 79: instanceData->actionCost += fv1; break;
					case 4: instanceData->attackDelay += fv1; break;
					case 8: instanceData->critChargeBonus += fv1; break;
					case 90: instanceData->critDamageMult += fv1; break;
					case 84: instanceData->fullPowerSeconds += fv1; break;
					case 3: instanceData->maxRange += fv1; break;
					case 87: instanceData->minPowerShot += fv1; break;
					case 2: instanceData->minRange += fv1; break;
					case 6: instanceData->outOfRangeMultiplier += fv1; break;
					case 1: instanceData->reach += fv1; break;
					case 76: instanceData->reloadSpeed += fv1; break;
					case 7: instanceData->secondary += fv1; break;
					case 83: instanceData->firingData->sightedTransition += fv1; break;
					case 0: instanceData->speed += fv1; break;
					case 41: aimModel->Rec_MaxPerShot += fv1; aimModel->formID = 0; break;
					case 42: aimModel->Rec_MinPerShot += fv1; aimModel->formID = 0; break;
					case 43: aimModel->Rec_HipMult += fv1; aimModel->formID = 0; break;
					case 45: aimModel->Rec_ArcMaxDegrees += fv1; aimModel->formID = 0; break;
					case 46: aimModel->Rec_ArcRotate += fv1; aimModel->formID = 0; break;
					case 40: aimModel->Rec_DimSightsMult += fv1; aimModel->formID = 0; break;
					case 39: aimModel->Rec_DimSpringForce += fv1; aimModel->formID = 0; break;
					case 33: aimModel->CoF_MinAngle += fv1; aimModel->formID = 0; break;
					case 34: aimModel->CoF_MaxAngle += fv1; aimModel->formID = 0; break;
					case 35: aimModel->CoF_IncrPerShot += fv1; aimModel->formID = 0; break;
					case 36: aimModel->CoF_DecrPerSec += fv1; aimModel->formID = 0; break;
					case 38: aimModel->CoF_SneakMult += fv1; aimModel->formID = 0; break;
					case 47: aimModel->CoF_IronSightsMult += fv1; aimModel->formID = 0; break;
					case 66: aimModel->BaseStability += fv1; aimModel->formID = 0; break;
					}
				}
				break;
				case BGSMod::Container::Data::kOpFlag_Set_Enum:
				{
					iv1 = data->value.i.v1;
					switch (targetType)
					{
					case 82: instanceData->stagger = iv1; break;
					case 9: instanceData->unk118 = iv1; break; //ehHitBehavior
					case 59: instanceData->unk114 = iv1; break; //elSoundLevel
						//case 68: instanceData->stagger = iv1; break; eoZoomDataOverlay
						//case 15: instanceData->stagger = iv1; break; ewType
					}
				}
				case BGSMod::Container::Data::kOpFlag_Set_Form:
				case BGSMod::Container::Data::kOpFlag_Add_Form: //?, need if?
				{
					switch (targetType)
					{
					case 61: //always SET
					{
						TESAmmo* ammo = (TESAmmo*)data->value.form;
						if (ammo)
							instanceData->ammo = ammo;
					}
					break;
					case 60: //always SET
					{
						BGSImpactDataSet* impactData = (BGSImpactDataSet*)data->value.form;
						if (impactData)
							instanceData->unk58 = impactData;
					}
					break;
					case 80: //always SET
					{
						BGSProjectile* proj = (BGSProjectile*)data->value.form;
						instanceData->firingData->projectileOverride = proj;
					}
					break;
					case 67: //always SET
					{
						BGSZoomData* zoomData = (BGSZoomData*)data->value.form;
						if (zoomData)
							instanceData->zoomData = zoomData;
					}
					break;
					case 75: //always SET
					{
						TESLevItem* NPCammolist = (TESLevItem*)data->value.form;
						if (NPCammolist)
							instanceData->addAmmoList = NPCammolist;
					}
					break;
					case 62: //always SET
					{
						SpellItem* criteffect = (SpellItem*)data->value.form;
						if (criteffect)
							instanceData->unk78 = criteffect;
					}
					break;
					case 32: //always SET
					{
						BGSAimModel* newModel = (BGSAimModel*)data->value.form; //dincast
						if (newModel)
							Utilities::UpdateAimModel(instanceData->aimModel, newModel); //aimModel->formID = data->value.form->formID;
					}
					break;
					case 65:
					{
						EnchantmentItem* ench = (EnchantmentItem*)data->value.form;
						if (!ench)
							break;
						SInt64 index = enchs.GetItemIndex(ench);
						if (index < 0)
							enchs.Push(ench);
					}
					break;
					case 31:
					{
						BGSKeyword* keyword = (BGSKeyword*)data->value.form;
						if (!keyword)
							break;
						SInt64 index = keywords.GetItemIndex(keyword);
						if (index < 0)
							keywords.Push(keyword);
					}
					break;
					//psSounds : no effect?
					}
				}
				break;
				case BGSMod::Container::Data::kOpFlag_Rem_Form: //??? if rem form==current -> set base
				{
					switch (targetType)
					{
					case 65:
					{
						EnchantmentItem* ench = (EnchantmentItem*)data->value.form;
						if (!ench)
							break;
						SInt64 index = enchs.GetItemIndex(ench);
						if (index >= 0)
							enchs.Remove(index);
					}
					break;
					case 31:
					{
						BGSKeyword* keyword = (BGSKeyword*)data->value.form;
						if (!keyword)
							break;
						SInt64 index = keywords.GetItemIndex(keyword);
						if (index >= 0)
							keywords.Remove(index);
					}
					break;
					}
				}
				break;
				case BGSMod::Container::Data::kOpFlag_Set_Int:
				case BGSMod::Container::Data::kOpFlag_Or_Bool:
				case BGSMod::Container::Data::kOpFlag_And_Bool:
				{
					UInt8 op = data->op;
					switch (targetType)
					{
					case 25: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0008000, data->value.i.v1, op); break; //-auto
					case 81: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0400000, data->value.i.v1, op); break; //-BoltAction
					case 26: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0020000, data->value.i.v1, op); break; //-CantDrop
					case 93: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000200, data->value.i.v1, op); break; //-ChargingAttack
					case 18: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000008, data->value.i.v1, op); break; //-ChargingReload
					case 92: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x1000000, data->value.i.v1, op); break; //-DisableShells
					case 21: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000100, data->value.i.v1, op); break; //effectondeath
					case 20: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000020, data->value.i.v1, op); break; //fixedrange
					case 48: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0200000, data->value.i.v1, op); break; //-kFlag_HasScope
					case 85: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000800, data->value.i.v1, op); break; //-HoldInputToPower
					case 24: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0004000, data->value.i.v1, op); break; //-IgnoresNormalResist
					case 19: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000010, data->value.i.v1, op); break; //-MinorCrime
					case 23: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0001000, data->value.i.v1, op); break; //-NonHostile
					case 27: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0100000, data->value.i.v1, op); break; //-NotUsedInNormalCombat
					case 17: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000002, data->value.i.v1, op); break; //-NPCsUseAmmo
					case 16: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000001, data->value.i.v1, op); break; //playeronly
					case 86: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0010000, data->value.i.v1, op); break; //-RepeatableSingleFire
					case 22: instanceData->flags = Utilities::AddRemFlag(instanceData->flags, 0x0000400, data->value.i.v1, op); break; //alternaterumble
					}
				}
				break;
				case BGSMod::Container::Data::kOpFlag_Set_Form_Float:
				case BGSMod::Container::Data::kOpFlag_Add_Form_Float:
				case BGSMod::Container::Data::kOpFlag_Mul_Add_Form_Float:
				{
					switch (targetType)
					{
					case 77:
					{
						BGSDamageType* dmgtype = (BGSDamageType*)LookupFormByID(data->value.ff.formId);
						if (!dmgtype)
							break;
						bool HasDmgType = false;
						TESObjectWEAP::InstanceData::DamageTypes dt;
						for (UInt32 i9 = 0; i9 < newDamageTypes.count; i9++)
						{
							newDamageTypes.GetNthItem(i9, dt);
							if (dt.damageType == dmgtype)
							{
								HasDmgType = true;
								break;
							}
						}
						if (!HasDmgType)
						{
							dt.damageType = dmgtype;
							dt.value = 0;
						}
						switch (data->op)
						{
						case BGSMod::Container::Data::kOpFlag_Set_Form_Float: dt.value = roundp(data->value.ff.v2); break;
						case BGSMod::Container::Data::kOpFlag_Add_Form_Float: dt.value += roundp(data->value.ff.v2); break;
						case BGSMod::Container::Data::kOpFlag_Mul_Add_Form_Float: dt.value += roundp(data->value.ff.v2*dt.value); break;
						}
						if (dt.value > 0)
							newDamageTypes.Push(dt);
					}
					break;
					//vaActorValue
					}
				}
				break;
				}
			}
		}

		//keywords
		keywords.Push(MSF_MainData::modifiedInstanceKW);
		if (instanceData->keywords)
		{
			Heap_Free(instanceData->keywords->keywords);
			instanceData->keywords->keywords = nullptr;
			instanceData->keywords->numKeywords = 0;
			if (keywords.count > 0)
			{
				instanceData->keywords->keywords = (BGSKeyword**)Heap_Allocate(sizeof(BGSKeyword*) * keywords.count);
				instanceData->keywords->numKeywords = keywords.count;
				for (UInt32 i3 = 0; i3 < keywords.count; i3++)
					instanceData->keywords->keywords[i3] = keywords[i3];
			}

		}

		//enchantments
		tArray<EnchantmentItem*> ** enchantments = nullptr;
		enchantments = &instanceData->enchantments;
		if (!(*enchantments)) //only if array>0
			(*enchantments) = new tArray<EnchantmentItem*>();
		(*enchantments)->Clear();
		for (UInt32 i9 = 0; i9 < enchs.count; i9++)
		{
			EnchantmentItem* ench;
			enchs.GetNthItem(i9, ench);
			(*enchantments)->Push(ench);
		}

		//damageTypes
		tArray<TBO_InstanceData::DamageTypes> ** damageTypes = nullptr;

		damageTypes = &instanceData->damageTypes;
		if (!(*damageTypes)) //only if array>0
			(*damageTypes) = new tArray<TBO_InstanceData::DamageTypes>();
		(*damageTypes)->Clear();
		for (UInt32 i8 = 0; i8 < newDamageTypes.count; i8++)
		{
			TESObjectWEAP::InstanceData::DamageTypes dt;
			newDamageTypes.GetNthItem(i8, dt);
			(*damageTypes)->Push(dt);
		}
		return true;
	}

	bool AttachModToStackedItem(BGSInventoryItem::Stack* stack, BGSMod::Attachment::Mod* mod)
	{
		if (stack->count == 1)
		{
		}
		else if (stack->count > 1)
		{
			SInt32 newCount = stack->count - 1;
			//create new stack
			stack->count = 1;
			//attach mod like AttachMod or AttachModToInventoryItem, but do NOT unequip&reequip
		}
		return false;
	}
}
