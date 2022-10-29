#include "MSF_SoftSwitch.h"

inline bool ClearSwitchFlags() { MSF_MainData::SwitchFlags = 127; return false; }

namespace MSF_SoftSwitch
{
	bool AddModDataToInstance(void)
	{
		BGSMod::Attachment::Mod* mod = MSF_MainData::ModToSwitch;
		MSF_MainData::ModToSwitch = nullptr;
		if (!mod)
		{
			if (MSF_MainData::SwitchFlags == 0)
				return false;
			else
				return ClearSwitchFlags();
		}
		UInt8 priority = mod->priority;
		if (MSF_MainData::SwitchFlags != mod->priority)
		{
			if (MSF_MainData::SwitchFlags == 0)
				return false;
			else
				return ClearSwitchFlags();
		}
		Actor* playerActor = *g_player;
		BGSObjectInstanceExtra* modData = Utilities::GetEquippedModData(playerActor, 41);
		if (!modData)
			return ClearSwitchFlags();
		TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(playerActor, 41);
		if (!instanceData)
			return ClearSwitchFlags();
		if (Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::FiringModeUnderbarrelKW) && (priority == 124 || priority == 125))
			return ClearSwitchFlags();
		//if (priority == 123 && ((*g_ui)->IsMenuOpen("ScopeMenu") || (*g_playerCamera)->GetCameraStateId((*g_playerCamera)->cameraState) == 4))
		//	return false;
		BGSInventoryItem::Stack* eqStack = Utilities::GetEquippedStack(playerActor, 41);
		if (!eqStack)
			return ClearSwitchFlags();
		//ExtraDataList * stackDataList = eqStack->extraData;
		//if (!stackDataList)
		//	return false;
		//VMArray<BGSMod::Attachment::Mod*> allAttachedMod;//= papyrusActor::GetWornItemMods(playerActor, 41);
		if (!MSF_SoftSwitch::UpdateInstanceDataWithMod(eqStack, mod))
			return ClearSwitchFlags();
		if (playerActor->middleProcess->unk08->equipData->equippedData->ammo != instanceData->ammo)
		{
			//UInt64 newLoadedAmmoNum = remains in chamber
			UInt64 ammoCount = Utilities::GetInventoryItemCount(playerActor->inventoryList, instanceData->ammo);
			playerActor->middleProcess->unk08->equipData->equippedData->ammo = instanceData->ammo;
			if (ammoCount > instanceData->ammoCapacity)
				playerActor->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)instanceData->ammoCapacity;
			else
				playerActor->middleProcess->unk08->equipData->equippedData->unk18 = ammoCount;
		}
		MSF_MainData::SwitchFlags = 127;

		//if (MSF_MainData::MCMSettingFlags & MSF_MainData::bNamingEnabled)
		//{
		//	BSExtraData * extraName = stackDataList->GetByType(ExtraDataType::kExtraData_TextDisplayData);
		//	ExtraTextDisplayData * textDisplay = DYNAMIC_CAST(extraName, BSExtraData, ExtraTextDisplayData);
		//	Utilities::RenameAtTextStart(textDisplay, oldMod->fullName.name, mod->fullName.name);
		//}
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
				//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(itStack->stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				//TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				//if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::modifiedInstanceKW))
				//{
				//	_MESSAGE("unlikely");
				//}
			}
			else
			{
				//ExtraInstanceData* extraInstanceData = DYNAMIC_CAST(itStack->stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
				//TESObjectWEAP::InstanceData* instanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(extraInstanceData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
				//if (!Utilities::WeaponInstanceHasKeyword(instanceData, MSF_MainData::modifiedInstanceKW))
				//{
				//	MSF_SoftSwitch::UpdateInstanceDataWithMod(itStack->stack, nullptr);
				//	_MESSAGE("upd");
				//}
				++itStack;
			}
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
		Utilities::UpdateAimModel((MSFAimModel*)instanceData->aimModel, (MSFAimModel*)weapBase->weapData.aimModel);

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
							Utilities::UpdateAimModel((MSFAimModel*)instanceData->aimModel, (MSFAimModel*)newModel); //aimModel->formID = data->value.form->formID;
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
}
