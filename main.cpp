#include <shlobj.h>

#include "AddressLibDecoder\versionlibdb.h"
#include "rva/RVA.h"
#include "rva/Hook/HookUtil.h"
#include "Globals.h"

#include "xbyak/xbyak.h"
#include "f4se_common\SafeWrite.h"
#include "f4se_common\BranchTrampoline.h"
#include "f4se/PluginAPI.h"
#include "f4se_common\Utilities.h"

#include "MSF_Base.h"
#include "MSF_Data.h"
#include "MSF_Scaleform.h"
#include "MSF_Events.h"
#include "MSF_Papyrus.h"
#include "MSF_WeaponState.h"
#include "MSF_Serialization.h"
#include "MSF_Localization.h"
#include "MSF_Addresses.h"
#ifdef DEBUG
#include "MSF_Test.h"
#endif


IDebugLog					gLog;
PluginHandle				g_pluginHandle	=	kPluginHandle_Invalid;
F4SEPapyrusInterface*		g_papyrus		=	NULL;
F4SEMessagingInterface*		g_messaging		=	NULL;
F4SEScaleformInterface*		g_scaleform		=	NULL;
F4SESerializationInterface* g_serialization =	NULL;
F4SETaskInterface*			g_threading		=	NULL;

void WritePutYourGunInCompatibility()
{
#ifndef NEXTGEN
	if (MSF_MainData::PutYourGunInCompatibility)
	{
		SInt32 displ = 0;
		uintptr_t fn = 0;

		HookUtil::SafeReadBuf(MainEquipHandler.GetUIntPtr()+0x1, &displ, sizeof(displ));

		if (displ != 0x0 && displ != 0x20244C89)
		{
			uintptr_t nextInstr = MainEquipHandler.GetUIntPtr() + 0x5;
			fn = static_cast<ptrdiff_t>(displ) + nextInstr;
		}
		if (fn != 0 && fn != PutYourGunInBranchCode)
		{
			struct PutYourGunIn_BranchCode : Xbyak::CodeGenerator {
				PutYourGunIn_BranchCode(void* buf) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retn, retnAddr, pygiCode, doAddr, cmpAddr;

					mov(rax, ptr[rip + cmpAddr]);
					cmp(byte[rax], 0);
					je(retn);
					mov(rax, ptr[rip + doAddr]);
					movzx(eax, word[rax]);
					test(ax, ax);
					jnz(retn);
					jmp(ptr[rip + pygiCode]);
					L(retn);
					//mov(eax, 0);
					//xchg(word[rip + doAddr], ax);
					mov(ptr[rsp + 0x20], r9);
					jmp(ptr[rip + retnAddr]);

					L(retnAddr);
					dq(MainEquipHandler.GetUIntPtr() + 0x5);
					L(pygiCode);
					dq((uintptr_t)MainEquipHandler_Copied);
					L(doAddr);
					dq((uintptr_t)&MSF_MainData::modSwitchManager.dontPutYourGunIn);
					L(cmpAddr);
					dq((uintptr_t)&MSF_MainData::PutYourGunInCompatibility);

				}
			};

			MainEquipHandler_Copied = (_MainEquipHandler)fn;
			void* PutYourGunInBranch_codeBuf = g_localTrampoline.StartAlloc();
			PutYourGunIn_BranchCode PutYourGunInBranch_code(PutYourGunInBranch_codeBuf);
			g_localTrampoline.EndAlloc(PutYourGunInBranch_code.getCurr());
			PutYourGunInBranchCode = (uintptr_t)PutYourGunInBranch_codeBuf;
			g_branchTrampoline.Write5Branch(MainEquipHandler.GetUIntPtr(), PutYourGunInBranchCode);
		}
	}
#endif
}

bool WriteF4SEScaleformHook()
{
	//_DEBUG("menu create hook: %p, mapsize: %08X", IMenuCreateHook_Start.GetUIntPtr(), sizeof(g_customMenuData));
	//_DEBUG("testaddr: %p, testaddr2: %0p", &RegisterCustomMenu, &g_customMenuDataX);
	uintptr_t target = HookUtil::GetFnPtrFromCall5(IMenuCreateHook_Start.GetUIntPtr());
	//_DEBUG("target: %p", target);
	if (target)
	{
		target += 6;
		uintptr_t target2 = 0;
		HookUtil::SafeReadBuf(target, &target2, sizeof(target2));
		//_DEBUG("target2: %p", target2);
		if (target2)
		{
			target2 += 0x1F;
			uintptr_t LoadFnHook = 0;
			//HookUtil::SafeReadBuf(target2, &LoadFnHook, sizeof(LoadFnHook));
			LoadCustomMenu_F4SEHook = HookUtil::SafeWrite64(target2, &LoadMSFCustomMenu_Hook);
			//_DEBUG("LoadFnHook: %p", LoadFnHook);
			//LoadFnHook += 0x164 + 0x3;
			//SInt32 displ = 0;
			//uintptr_t mapaddr = 0;
			//HookUtil::SafeReadBuf(LoadFnHook, &displ, sizeof(displ));
			//uintptr_t nextInstr = LoadFnHook + 0x4;
			//mapaddr = static_cast<ptrdiff_t>(displ) + nextInstr; //F8D8
			//mapaddr -= 0x8;
			//_DEBUG("mapaddr: %p", mapaddr);+
			if (LoadCustomMenu_F4SEHook)
				return true;
		}
	}
	return false;
}

bool RegisterAfterLoadEvents()
{
	auto eventDispatcher1 = GET_EVENT_DISPATCHER(PlayerAmmoCountEvent);
	if (eventDispatcher1)
		eventDispatcher1->AddEventSink(&playerAmmoCountEventSink);
	else
	{
		_MESSAGE("MSF was unable to register for PlayerAmmoCountEvent");
		return false;
	}

	BSTEventDispatcher<MenuOpenCloseEvent>* eventDispatcher2 = &(*g_ui)->menuOpenCloseEventSource;
	if (eventDispatcher2)
		eventDispatcher2->eventSinks.Push(&menuOpenCloseSink);
	else
	{
		_MESSAGE("MSF was unable to register for MenuOpenCloseEvent");
		return false;
	}

	auto eventDispatcher3 = GET_EVENT_DISPATCHER(PipboyLightEvent);
	if (eventDispatcher3)
		eventDispatcher3->eventSinks.Push(&pipboyLightEvent);
	else 
	{
		if (MSF_MainData::BAKACompatibility && (*g_globalEvents)->eventSources.count >= 0x1B)
		{
			eventDispatcher3 = (BSTEventDispatcher<PipboyLightEvent>*) & (*g_globalEvents)->eventSources[0x1A]->eventDispatcher;
			eventDispatcher3->eventSinks.Push(&pipboyLightEvent);
		}
		else
		{
			_MESSAGE("MSF was unable to register for PipboyLightsEvent");
			return false;
		}
	}

	//REGISTER_EVENT(BGSOnPlayerModArmorWeaponEvent, modArmorWeaponEventSink);
	//REGISTER_EVENT(BGSOnPlayerUseWorkBenchEvent, useWorkbenchEventSink);

	if (!RegisterInventoryEvent((*g_player)->inventoryList, &MSF_MainData::playerInventoryEventSink))
	{
		_MESSAGE("MSF was unable to register for PlayerInventoryEvent");
		return false;
	}

	if (!g_ActorEquipManager->equipUnequipEventSource.AddEventSink(&MSF_MainData::actorEquipManagerEventSink))
	{
		_MESSAGE("MSF was unable to register for ActorEquipManagerEvent");
		return false;
	}

	return true;
}

class TESLoadGameHandler : public BSTEventSink<TESLoadGameEvent>
{
public:
	virtual ~TESLoadGameHandler() { };
	virtual    EventResult    ReceiveEvent(TESLoadGameEvent * evn, void * dispatcher) override
	{
					//instance midprocess ammo count!
		if (!MSF_MainData::IsInitialized)
		{
			if (!RegisterAfterLoadEvents())
				_ERROR("Event registration failed");
			else 
				MSF_MainData::IsInitialized = true;
		}
		if (MSF_MainData::IsInitialized)
		{
			uint64_t res = CheckHooks();
			if (res == 0)
				_MESSAGE("Hooks OK.");

			MSF_MainData::modSwitchManager.Reset();

			_MESSAGE("Registering MSF menus.");
			MSFWidgetMenu::RegisterMenu();
			//MSFAmmoMenu::RegisterMenu();
			//MSFModMenu::RegisterMenu();
			MSFWidgetMenu::OpenMenu();

			//delayTask delayUpd(500, true, &MSF_Scaleform::UpdateWidgetData, nullptr);
			WidgetUpdateTask* updTask = new WidgetUpdateTask(true, true);
			delayTask delayUpd(500, true, g_threading->AddUITask, updTask);

			//BSScaleformTranslator * translator = (BSScaleformTranslator*)(*g_scaleformManager)->stateBag->GetStateAddRef(GFxState::kInterface_Translator);
			//if (translator) {
			//	SSWTranslator::LoadTranslations(translator);
			//}
			MSF_Scaleform::RegisterMSFCustomMenus(); //MSFMenu::RegisterMenu(); //!/registermenus
			//MSFMenu::OpenMenu(); //!/comment
			delete MSF_MainData::ammoDisplay;
			MSF_MainData::ammoDisplay = HUDMenuAmmoDisplay::Init();

			TESAmmo* ammo = nullptr;
			ExtraWeaponState::HandleWeaponStateEvents(ExtraWeaponState::kEventTypeEquip);
			EquipWeaponData* eqWeapData = Utilities::GetEquippedWeaponData(*g_player);
			if (eqWeapData)
				ammo = eqWeapData->ammo;
			MSF_Base::EquipAmmo((*g_player)->inventoryList, ammo);

			TESObjectWEAP* weapBase = Utilities::GetEquippedGun(*g_player);
			TESObjectWEAP::InstanceData* instanceData = Utilities::GetEquippedInstanceData(*g_player);
			if (weapBase && instanceData)
			{
				BGSObjectInstance idStruct;
				idStruct.object = weapBase;
				idStruct.instanceData = instanceData;
				//MSF_MainData::modSwitchManager.SetIgnoreEquipAction(true);
				MSF_MainData::modSwitchManager.SetIgnoreAnimGraph(true);
				MSF_MainData::modSwitchManager.SetDontPutYourGunIn(true);
				EquipItemInternal(g_ActorEquipManager, *g_player, idStruct, 0, 1, nullptr, 0, 0, 0, 1, 0);
				MSF_MainData::modSwitchManager.SetDontPutYourGunIn(false);
			}

			if (MSF_MainData::iAutolowerTimeMS)
			{
				LowerWeaponTask* lowerTask = new LowerWeaponTask();
				MSF_MainData::modSwitchManager.lowerGunTimer.start(MSF_MainData::iAutolowerTimeMS, g_threading->AddTask, lowerTask);
				//delayTask(MSF_MainData::iAutolowerTimeMS, true, g_threading->AddTask, lowerTask);
			}
		}
		return kEvent_Continue;
	}
};

void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case F4SEMessagingInterface::kMessage_GameDataReady:
	{
		if (msg->data == (void*)true)
		{
			//MSF_Scaleform::RegisterMCMCallback();
			if (!MSF_Data::InitData())
				_FATALERROR("Fatal Error - MSF was unable to initialize plugin data");
			else if (!MSF_Data::InitMCMSettings())
				_FATALERROR("Fatal Error - MSF was unable to initialize MCM settings");
			else
			{
				WritePutYourGunInCompatibility();
				if (!WriteF4SEScaleformHook())
					_MESSAGE("ERROR - Could not write F4SE Custom Menu Hook.");
				static auto pLoadGameHandler = new TESLoadGameHandler();
				GetEventDispatcher<TESLoadGameEvent>()->AddEventSink(pLoadGameHandler);
				MSF_Scaleform::ReceiveKeyEvents();

				RVAUtils::Timer tmr;
				tmr.start();
				MSF_Data::LoadPluginData();
				_MESSAGE("Plugin Data Loading Time: %llu ms.", tmr.stop());

				MSF_Localization::ParseTranslations();

				MSF_Data::InjectLeveledLists();

				REGISTER_EVENT(TESCellFullyLoadedEvent, cellFullyLoadedEventSink);
				REGISTER_EVENT(TESCombatEvent, combatEvnSink);

				MSF_Data::InitCompatibility();
			}
		}
	}
	break;
	case F4SEMessagingInterface::kMessage_PreLoadGame: MSF_MainData::GameIsLoading = true; break;
	case F4SEMessagingInterface::kMessage_PostLoadGame: MSF_MainData::GameIsLoading = false; break;
	//case 363636:
	//{
	//	AttachModMessage* data = (AttachModMessage*)msg->data;
	//	if (data)
	//		MSF_Base::AttachModToEquippedWeapon(data->actor, data->mod, data->bAttach, data->modAmmoCount, data->updateAnimGraph);
	//}
	//break;
	}
}

bool WriteHooks()
{
	AttackInputHandlerSelf_Copied = HookUtil::GetFnPtrFromCall5(AttackInputHandler_SelfHookTarget.GetUIntPtr(), &AttackInputHandlerReload_Hook);
	HUDShowAmmoCounter_Copied = HookUtil::GetFnPtrFromCall5(HUDShowAmmoCounter_HookTarget.GetUIntPtr(), &HUDShowAmmoCounter_Hook);
	EquipHandler_UpdateAnimGraph_Copied = HookUtil::GetFnPtrFromCall5(EquipHandler_UpdateAnimGraph_HookTarget.GetUIntPtr(), &EquipHandler_UpdateAnimGraph_Hook);
	EquipHandler_PlayEquipAction_Copied = HookUtil::GetFnPtrFromCall5(EquipHandler_PlayEquipAction_HookTarget.GetUIntPtr(), &EquipHandler_PlayEquipAction_Hook);
	AttachModToStack_CallFromGameplay_Copied = HookUtil::GetFnPtrFromCall5(AttachModToStack_CallFromGameplay_HookTarget.GetUIntPtr(), &AttachModToStack_CallFromGameplay_Hook);
	AttachModToStack_CallFromWorkbenchUI_Copied = HookUtil::GetFnPtrFromCall5(AttachModToStack_CallFromWorkbenchUI_HookTarget.GetUIntPtr(), &AttachModToStack_CallFromWorkbenchUI_Hook);
	DeleteExtraData_CallFromWorkbenchUI_Copied = HookUtil::GetFnPtrFromCall5(DeleteExtraData_CallFromWorkbenchUI_HookTarget.GetUIntPtr(), &DeleteExtraData_CallFromWorkbenchUI_Hook);
	UpdateEquipData_Copied = HookUtil::GetFnPtrFromCall5(UpdateEquipData_HookTarget.GetUIntPtr(), &UpdateEquipData_Hook);
	LoadBuffer_ExtraDataList_ExtraRank_ReturnJumpAddr = HookUtil::GetFnPtrFromCall5(LoadBuffer_ExtraDataList_ExtraRank_JumpHookTarget.GetUIntPtr());
	AttackInputHandler_Copied = HookUtil::GetFnPtrFromCall5(AttackInputHandler_HookTarget.GetUIntPtr(), &AttackInputHandler_Hook);
	ObjectInstanceCtor_Copied = HookUtil::GetFnPtrFromCall5(ObjectInstanceCtor_HookTarget.GetUIntPtr(), &ObjectInstanceCtor_Hook);
	ActorEquipManagerPre_Copied = HookUtil::GetFnPtrFromCall6(ActorEquipManagerPre_JumpHookTarget.GetUIntPtr());
	AttachRemoveModInternal_Copied = HookUtil::GetFnPtrFromCall5(AModToInvItem_Attach_AV_HookTarget.GetUIntPtr(), &AttachRemoveModInternal_Hook);
	EquipItemPapyrus_Copied = HookUtil::GetFnPtrFromCall5(AModToInvItem_Equip_AV_HookTarget.GetUIntPtr(), &EquipItemPapyrus_Hook);
	ExtraRankConstructor_Copied = HookUtil::GetFnPtrFromCall5(ExtraRankConstructor_HookTarget.GetUIntPtr(), &ExtraRankConstructor_Hook);

	uint32_t res = CheckHookCopies();
	if (res)
	{
		_ERROR("Error - Hook targets not found. Error code: 0x%08X", res);
		return false;
	}


	struct LoadExtraRank_InjectCode : Xbyak::CodeGenerator {
		LoadExtraRank_InjectCode(void* buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label hook, retnLabel;
#ifndef NEXTGEN
			mov(edx, dword[r13 + 0x24]);
			add(rdx, qword[r13 + 0x8]);
			mov(r8, r12);
			mov(edx, dword[rdx]);
			mov(r9, qword[rsp + 0x318]);
#else
			mov(edx, dword[r15 + 0x24]);
			add(rdx, qword[r15 + 0x8]);
			mov(r8, r13);
			mov(edx, dword[rdx]);
			xor(r9, r9);
#endif
			mov(rcx, rax);
			call(ptr[rip + hook]);
			jmp(ptr[rip + retnLabel]);

			L(hook);
			dq((uintptr_t)LoadBuffer_ExtraDataList_ExtraRank_Hook);
			L(retnLabel);
			dq(LoadBuffer_ExtraDataList_ExtraRank_ReturnJumpAddr);
		}
	};
	void* codeBuf = g_localTrampoline.StartAlloc();
	LoadExtraRank_InjectCode code(codeBuf);
	g_localTrampoline.EndAlloc(code.getCurr());

	LoadBuffer_ExtraDataList_ExtraRank_BranchCode = (uintptr_t)codeBuf;
	g_branchTrampoline.Write5Branch(LoadBuffer_ExtraDataList_ExtraRank_JumpHookTarget.GetUIntPtr(), LoadBuffer_ExtraDataList_ExtraRank_BranchCode);
	

	struct CheckAmmoCountForReload_InjectCode : Xbyak::CodeGenerator {
		CheckAmmoCountForReload_InjectCode(void* buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label hook, retnLabel;

#ifndef NEXTGEN
			mov(rdx, qword[rsi + 8]);
			mov(rcx, qword[rdx]);
			mov(edx, r14d);
			mov(r8d, eax);
			call(ptr[rip + hook]);
			mov(rcx, qword[rsi]);
#else
			mov(rdx, qword[r14 + 8]);
			mov(rcx, qword[rdx]);
			mov(edx, ebx);
			mov(r8d, eax);
			call(ptr[rip + hook]);
			mov(rcx, qword[r14]);
#endif
			mov(byte[rcx], al);
			jmp(ptr[rip + retnLabel]);

			L(hook);
			dq((uintptr_t)CheckAmmoCountForReload_Hook);
			L(retnLabel);
			dq(CheckAmmoCountForReload_ReturnJumpAddr);
		}
	};
	void* CheckAmmoCount_codeBuf = g_localTrampoline.StartAlloc();
	CheckAmmoCountForReload_InjectCode CheckAmmoCount_code(CheckAmmoCount_codeBuf);
	g_localTrampoline.EndAlloc(CheckAmmoCount_code.getCurr());
	CheckAmmoCountForReload_BranchCode = (uintptr_t)CheckAmmoCount_codeBuf;
	g_branchTrampoline.Write5Branch(CheckAmmoCountForReload_JumpHookTarget.GetUIntPtr(), CheckAmmoCountForReload_BranchCode);

	struct CannotEquipItem_InjectCode : Xbyak::CodeGenerator {
		CannotEquipItem_InjectCode(void* buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label hook, cantTextToLoad, modTextToLoad, toReturn, toMod, retnGenLabel, retnModLabel, skipLabel;
				// r13, r15, rbx, rdi, rsi
				// r12, r14, rbx, rdi, rsi

			mov(edi, eax);
			mov(sil, r9b);
#ifndef NEXTGEN
			mov(r13, rdx);
			mov(r15d, r8d);
			mov(rdx, qword[rbp - 0x41]);
			mov(rcx, qword[rbp + 0x57]);
#else
			mov(rcx, r14);
			mov(r12, rdx);
			mov(rdx, qword[rbp - 0x19]);
			mov(r14d, r8d);
#endif
			mov(r9d, edi);
			mov(r8d, ebx);
			call(ptr[rip + hook]);
			test(rax, rax);
			jnz(toReturn);
			jmp(ptr[rip + skipLabel]);
			L(toReturn);
#ifndef NEXTGEN
			mov(rdx, r13);
			mov(r8d, r15d);
#else
			mov(rdx, r12);
			mov(r8d, r14d);
#endif
			mov(r9b, sil);
			mov(rcx, rax);
			cmp(edi, 2);
			je(toMod);
			//mov(rcx, qword[rip + cantTextToLoad]);
			jmp(ptr[rip + retnGenLabel]);
			L(toMod);
			//mov(rcx, ptr[rip + modTextToLoad]);
			jmp(ptr[rip + retnModLabel]);

			L(hook);
			dq((uintptr_t)CannotEquipItem_Hook);
			L(cantTextToLoad);
			dq(CannotEquipItem_TextAddr);
			L(modTextToLoad);
			dq(0); //(uintptr_t)modText
			L(retnGenLabel);
			dq(CannotEquipItemGen_ReturnJumpAddr);
			L(retnModLabel);
			dq(CannotEquipItemMod_ReturnJumpAddr);
			L(skipLabel);
			dq(CannotEquipItem_SkipJumpAddr);
		}
	};
	void* CannotEquipItem_codeBuf = g_localTrampoline.StartAlloc();
	CannotEquipItem_InjectCode CannotEquipItem_code(CannotEquipItem_codeBuf);
	g_localTrampoline.EndAlloc(CannotEquipItem_code.getCurr());
	CannotEquipItem_BranchCode = (uintptr_t)CannotEquipItem_codeBuf;
	g_branchTrampoline.Write5Branch(CannotEquipItemGen_JumpHookTarget.GetUIntPtr(), CannotEquipItem_BranchCode);
	g_branchTrampoline.Write5Branch(CannotEquipItemMod_JumpHookTarget.GetUIntPtr(), CannotEquipItem_BranchCode);

	struct ActorEquipManagerPre_InjectCode : Xbyak::CodeGenerator {
		ActorEquipManagerPre_InjectCode(void* buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label hook, retnLabel, toEnd;
			jz(toEnd);

			push(r10);
			push(r11);
			push(rdx);
			push(rax);
			//push(r8);
			sub(rsp, 0x100);
			mov(r8d, r9d);
#ifndef NEXTGEN
			mov(rdx, rsi);
#else
			mov(rdx, rdi);
#endif
			mov(rcx, rax);
			call(ptr[rip + hook]);

			add(rsp, 0x100);
			//pop(r8);
			pop(rax);
			pop(rdx);
			pop(r11);
			pop(r10);
			jmp(ptr[rip + retnLabel]);

			L(hook);
			dq((uintptr_t)ActorEquipManagerPre_Hook);
			L(retnLabel);
			dq(ActorEquipManagerPre_ReturnJumpAddr);
			L(toEnd);
			dq(ActorEquipManagerEnd_ReturnJumpAddr);
		}
	};
	void* ActorEquipManagerPre_codeBuf = g_localTrampoline.StartAlloc();
	ActorEquipManagerPre_InjectCode ActorEquipManagerPre_code(ActorEquipManagerPre_codeBuf);
	g_localTrampoline.EndAlloc(ActorEquipManagerPre_code.getCurr());
	ActorEquipManagerPre_BranchCode = (uintptr_t)ActorEquipManagerPre_codeBuf;
	g_branchTrampoline.Write6Branch(ActorEquipManagerPre_JumpHookTarget.GetUIntPtr(), ActorEquipManagerPre_BranchCode);

	PlayerAnimationEvent_Original = HookUtil::SafeWrite64(PlayerAnimationEvent_HookTarget.GetUIntPtr(), &PlayerAnimationEvent_Hook);
	ExtraRankCompare_Copied = (uintptr_t)HookUtil::SafeWrite64(s_ExtraRankVtbl.GetUIntPtr()+8, &ExtraRankCompare_Hook);
	ExtraRankDestructor_Copied = HookUtil::SafeWrite64(s_ExtraRankVtbl.GetUIntPtr(), &ExtraRankDestructor_Hook);
	PipboyMenuInvoke_Copied = HookUtil::SafeWrite64(PipboyMenuInvoke_HookAddress.GetUIntPtr(), &PipboyMenuInvoke_Hook);
	if (!PlayerAnimationEvent_Original || !ExtraRankCompare_Copied || !PipboyMenuInvoke_Copied)
	{
		_MESSAGE("Hook SafeWrite64 failed.");
		return false;
	}
	//g_branchTrampoline.Write5Call(AttackBlockHandler_HookTarget.GetUIntPtr(), (uintptr_t)AttackBlockHandler_Hook);
	g_branchTrampoline.Write5Call(AttackInputHandler_HookTarget.GetUIntPtr(), (uintptr_t)AttackInputHandler_Hook);
	g_branchTrampoline.Write5Call(AttackInputHandler_SelfHookTarget.GetUIntPtr(), (uintptr_t)AttackInputHandlerReload_Hook);
	g_branchTrampoline.Write5Call(HUDShowAmmoCounter_HookTarget.GetUIntPtr(), (uintptr_t)HUDShowAmmoCounter_Hook);
	g_branchTrampoline.Write5Call(EquipHandler_UpdateAnimGraph_HookTarget.GetUIntPtr(), (uintptr_t)EquipHandler_UpdateAnimGraph_Hook);
	g_branchTrampoline.Write5Call(EquipHandler_PlayEquipAction_HookTarget.GetUIntPtr(), (uintptr_t)EquipHandler_PlayEquipAction_Hook);
	g_branchTrampoline.Write5Call(AttachModToStack_CallFromGameplay_HookTarget.GetUIntPtr(), (uintptr_t)AttachModToStack_CallFromGameplay_Hook);
	g_branchTrampoline.Write5Call(AttachModToStack_CallFromWorkbenchUI_HookTarget.GetUIntPtr(), (uintptr_t)AttachModToStack_CallFromWorkbenchUI_Hook);
	g_branchTrampoline.Write5Call(DeleteExtraData_CallFromWorkbenchUI_HookTarget.GetUIntPtr(), (uintptr_t)DeleteExtraData_CallFromWorkbenchUI_Hook);
	g_branchTrampoline.Write5Call(UpdateEquipData_HookTarget.GetUIntPtr(), (uintptr_t)UpdateEquipData_Hook);
	g_branchTrampoline.Write5Call(ObjectInstanceCtor_HookTarget.GetUIntPtr(), (uintptr_t)ObjectInstanceCtor_Hook);
	g_branchTrampoline.Write5Call(AModToInvItem_Attach_AV_HookTarget.GetUIntPtr(), (uintptr_t)AttachRemoveModInternal_Hook);
	g_branchTrampoline.Write5Call(AModToInvItem_Equip_AV_HookTarget.GetUIntPtr(), (uintptr_t)EquipItemPapyrus_Hook);
	g_branchTrampoline.Write5Call(ExtraRankConstructor_HookTarget.GetUIntPtr(), (uintptr_t)ExtraRankConstructor_Hook);



	res = CheckHookTargets();
	if (res)
	{
		_MESSAGE("Hook target mismatch found during initialization. This message may be ignored. Error code: 0x%08X", res);
	}

	return true;
}

bool InitializeOffsets()
{
	VersionDb db;
	if (!db.Load())
	{
		_FATALERROR("Failed to load version database for current executable!");
		return false;
	}
	else
	{
		// "SkyrimSE.exe", "1.5.97.0"
		_MESSAGE("Loaded database for %s version %s.", db.GetModuleName().c_str(), db.GetLoadedVersionString().c_str());
	}

	// This address already includes the base address of module so we can use the address directly.
	void* MyAddress = db.FindAddressById(123);
	if (MyAddress == NULL)
	{
		_FATALERROR("Failed to find address!");
		return false;
	}

	// This offset does not include base address. Actual address would be ModuleBase + MyOffset.
	unsigned long long MyOffset = 0;
	if (!db.FindOffsetById(123, MyOffset))
	{
		_FATALERROR("Failed to find offset for my thing!");
		return false;
	}

	// Everything was successful.
	return true;
}

bool InitPlugin(const F4SEInterface* f4se) 
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\ModSwitchFramework.log");
	_MESSAGE("%s v%s dll loaded...\n", PLUGIN_NAME_SHORT, MSF_VERSION_STRING);
#if CURRENT_RELEASE_RUNTIME >= MAKE_EXE_VERSION(1, 11, 137) 
	_MESSAGE("Anniversary Edition (NextGen) version\n");
#elif defined(NEXTGEN)
	_MESSAGE("NextGen version\n");
#endif
	_MESSAGE("Runtime version: %08X", f4se->runtimeVersion);
	_MESSAGE("F4SE version: %08X", f4se->f4seVersion);
#ifdef DEBUG
	_MESSAGE("DebugMode enabled!");
#endif
	_MESSAGE("Base address: %p", IDDatabase::get().base());//reinterpret_cast<uintptr_t>(GetModuleHandle(NULL)));

#ifndef NEXTGEN
	// Check game version
	if (!COMPATIBLE(f4se->runtimeVersion))
	{
		char str[512];
		sprintf_s(str, sizeof(str), "Error! Your game version: v%d.%d.%d.%d\nExpected version: v%d.%d.%d.%d\n%s will be disabled.",
			GET_EXE_VERSION_MAJOR(f4se->runtimeVersion),
			GET_EXE_VERSION_MINOR(f4se->runtimeVersion),
			GET_EXE_VERSION_BUILD(f4se->runtimeVersion),
			GET_EXE_VERSION_SUB(f4se->runtimeVersion),
			GET_EXE_VERSION_MAJOR(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_MINOR(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_BUILD(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_SUB(SUPPORTED_RUNTIME_VERSION),
			PLUGIN_NAME_LONG
		);

		MessageBox(NULL, str, PLUGIN_NAME_LONG, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
#else
	if (f4se->runtimeVersion <= RUNTIME_VERSION_1_10_163)
	{
		char str[512];
		sprintf_s(str, sizeof(str), "Error! NextGen plugin loaded, but game version is OldGen (v%d.%d.%d.%d).\n%s will be disabled.",
			GET_EXE_VERSION_MAJOR(f4se->runtimeVersion),
			GET_EXE_VERSION_MINOR(f4se->runtimeVersion),
			GET_EXE_VERSION_BUILD(f4se->runtimeVersion),
			GET_EXE_VERSION_SUB(f4se->runtimeVersion),
			PLUGIN_NAME_LONG
		);
		MessageBox(NULL, str, PLUGIN_NAME_LONG, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
#endif

	g_pluginHandle = f4se->GetPluginHandle();
	if (!g_localTrampoline.Create(1024 * 64, nullptr)) {
		_ERROR("Fatal Error - Couldn't create codegen buffer. Skipping remainder of init process.");
		return false;
	}
	if (!g_branchTrampoline.Create(1024 * 64)) {
		_ERROR("Fatal Error - Couldn't create branch trampoline. Skipping remainder of init process.");
		return false;
	}

	g_messaging = (F4SEMessagingInterface*)f4se->QueryInterface(kInterface_Messaging);
	if (!g_messaging)
	{
		_FATALERROR("Fatal Error - Messaging query failed");
		return false;
	}
	g_papyrus = (F4SEPapyrusInterface*)f4se->QueryInterface(kInterface_Papyrus);
	if (!g_papyrus)
	{
		_FATALERROR("Fatal Error - Papyrus query failed");
		return false;
	}
	g_scaleform = (F4SEScaleformInterface*)f4se->QueryInterface(kInterface_Scaleform);
	if (!g_scaleform)
	{
		_FATALERROR("Fatal Error - Scaleform query failed");
		return false;
	}
	g_serialization = (F4SESerializationInterface*)f4se->QueryInterface(kInterface_Serialization);
	if (!g_serialization)
	{
		_FATALERROR("Fatal Error - Serialization query failed");
		return false;
	}
	g_threading = (F4SETaskInterface*)f4se->QueryInterface(kInterface_Task);
	if (!g_serialization)
	{
		_FATALERROR("Fatal Error - Task query failed");
		return false;
	}
	
	g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);
	//g_messaging->RegisterListener(g_pluginHandle, NULL, MSFMessageHandler);
	if (!g_scaleform->Register(PLUGIN_NAME_SHORT, MSF_Scaleform::RegisterScaleformCallback))
		_MESSAGE("MSF scaleform registration failed");
	//if (!g_scaleform->Register(PLUGIN_NAME_SHORT, MSF_Scaleform::RegisterScaleformTest))
	//	_MESSAGE("MSF widget scaleform registration failed");
	g_serialization->SetUniqueID(g_pluginHandle, 'MSF');
	g_serialization->SetRevertCallback(g_pluginHandle, MSF_Serialization::RevertCallback);
	g_serialization->SetLoadCallback(g_pluginHandle, MSF_Serialization::LoadCallback);
	g_serialization->SetSaveCallback(g_pluginHandle, MSF_Serialization::SaveCallback);
	g_papyrus->Register(MSF_Papyrus::RegisterPapyrus);
#ifdef DEBUG
	g_papyrus->Register(MSF_Test::RegisterPapyrus);
#endif

	if (GetFileAttributes("Data\\F4SE\\Plugins\\mcm.dll") == INVALID_FILE_ATTRIBUTES)
	{
		_WARNING("Warning - Missing mcm.dll - Keybinds and settings are read from MCM files. While it is possible to maintain these manually, installing and using MCM is highly recommended.");
		//_FATALERROR("Fatal Error - Missing mcm.dll - Install MCM for this plugin to properly function");
		//return false;
	}

	if (!MSF_Data::ReadKeybindData())
	{
		_FATALERROR("Fatal Error - MSF was unable to initialize keybinds");
		return false;
	}

	Globals::Init();
	RVAManager::UpdateAddresses(f4se->runtimeVersion);
	//InitializeOffsets();
	if (!WriteHooks())
	{
		_FATALERROR("Fatal Error - MSF was unable to write hooks");
		return false;
	}

	if (GetFileAttributes("Data\\F4SE\\Plugins\\PutUrGunIn.dll") != INVALID_FILE_ATTRIBUTES)
	{
		MSF_MainData::PutYourGunInCompatibility = true;
		_MESSAGE("PutUrGunIn.dll detected - Compatibility patch enabled");
	}

	if (GetFileAttributes("Data\\F4SE\\Plugins\\BakaFramework.dll") != INVALID_FILE_ATTRIBUTES)
	{
		MSF_MainData::BAKACompatibility = true;
		_MESSAGE("BakaFramework.dll detected - Compatibility patch enabled");
	}

	srand(time(NULL));

	return true;
}

extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name =		PLUGIN_NAME_SHORT;
	info->version =		MSF_VERSION;

	return true;
}

#if CURRENT_RELEASE_RUNTIME > RUNTIME_VERSION_1_10_163 
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version =
	{
		F4SEPluginVersionData::kVersion,

		MSF_VERSION,
		PLUGIN_NAME_LONG,
		AUTHOR_NAME,

		0,	// not version independent
		0,	// not version independent
		{ CURRENT_RELEASE_RUNTIME, 0 },

		0,	// works with any version of the script extender
	};
#endif

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
	InitPlugin(f4se);
	return true;
}

}
