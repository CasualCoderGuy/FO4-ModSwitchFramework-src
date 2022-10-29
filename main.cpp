#include <shlobj.h>

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
#include "MSF_Papyrus.h"
//#include "PapyrusStack.h"
//#include "PapyrusInstanceData.h"
#include "MSF_Test.h"


IDebugLog				gLog;
PluginHandle			g_pluginHandle =	kPluginHandle_Invalid;
F4SEPapyrusInterface	*g_papyrus   =		NULL;
F4SEMessagingInterface	*g_messaging =		NULL;
F4SEScaleformInterface	*g_scaleform =		NULL;

//==================== Events =========================
BSTEventDispatcher<void*>* GetGlobalEventDispatcher(BSTGlobalEvent* globalEvents, const char * dispatcherName)
{
	for (int i = 0; i < globalEvents->eventSources.count; i++) {
		const char* name = GetObjectClassName(globalEvents->eventSources[i]) + 15;    // ?$EventSource@V
		if (strstr(name, dispatcherName) == name) {
			return &globalEvents->eventSources[i]->eventDispatcher;
		}
	}
	return nullptr;
}
#define GET_EVENT_DISPATCHER(EventName) (BSTEventDispatcher<EventName>*) GetGlobalEventDispatcher(*g_globalEvents, #EventName);

//Anim Event
class BSAnimationGraphEvent
{
public:
	TESForm* unk00;
	BSFixedString eventName;
	//etc
};
typedef UInt8(*_tf1)(void * thissink, BSAnimationGraphEvent* evnstruct, void** dispatcher);
RelocAddr <_tf1> tf1_HookTarget(0x2D442E0);
_tf1 tf1_Original;

struct BGSOnPlayerUseWorkBenchEvent
{
};
class BGSOnPlayerUseWorkBenchEventSink : public BSTEventSink<BGSOnPlayerUseWorkBenchEvent>
{
public:
	virtual	EventResult	ReceiveEvent(BGSOnPlayerUseWorkBenchEvent* evn, void * dispatcher) override
	{
		_MESSAGE("Workbench evn");
		return kEvent_Continue;
	}
};
BGSOnPlayerUseWorkBenchEventSink useWorkbenchEventSink;
DECLARE_EVENT_DISPATCHER(BGSOnPlayerUseWorkBenchEvent, 0x0441A10);

//Player Mod Event
struct BGSOnPlayerModArmorWeaponEvent
{
	TESBoundObject*						object;			// 00 TESObjectWEAP or TESObjectARMO
	BGSMod::Attachment::Mod*			mod;			// 08
};
class BGSOnPlayerModArmorWeaponEventSink : public BSTEventSink<BGSOnPlayerModArmorWeaponEvent>
{
public:
	virtual	EventResult	ReceiveEvent(BGSOnPlayerModArmorWeaponEvent * evn, void * dispatcher) override
	{
		TESObjectWEAP* moddedWeap = DYNAMIC_CAST(evn->object, TESBoundObject, TESObjectWEAP);
		if (moddedWeap)
			delayTask delay(100, true, &MSF_Base::ReevalModdedWeapon, moddedWeap);
		return kEvent_Continue;
	}
};
BGSOnPlayerModArmorWeaponEventSink modArmorWeaponEventSink;
DECLARE_EVENT_DISPATCHER(BGSOnPlayerModArmorWeaponEvent, 0x0441790);

//Cell Load Event
struct TESCellFullyLoadedEvent
{
	TESObjectCELL*				cell;			// 00
};
class TESCellFullyLoadedEventSink : public BSTEventSink<TESCellFullyLoadedEvent>
{
public:
	virtual	EventResult	ReceiveEvent(TESCellFullyLoadedEvent * evn, void * dispatcher) override
	{
		//MSF_Base::SpawnRandomMods(evn->cell);
		//MSF_Base::CreateRandomWeaponHealth(evn->cell);
		_MESSAGE("Cell evn");
		return kEvent_Continue;
	}
};
TESCellFullyLoadedEventSink cellFullyLoadedEventSink;
DECLARE_EVENT_DISPATCHER(TESCellFullyLoadedEvent, 0x00442050);

//Equip event
struct PlayerAmmoCountEvent
{
	UInt32							ammoCount;        // 00
	UInt32							totalAmmoCount;    // 04
	UInt64							unk08;            // 08
	TESObjectWEAP*					weapon;            // 10
	TESObjectWEAP::InstanceData*    weaponInstance;
	//...
};
STATIC_ASSERT(offsetof(PlayerAmmoCountEvent, weapon) == 0x10);
class PlayerAmmoCountEventSink : public BSTEventSink<PlayerAmmoCountEvent>
{
public:
	virtual ~PlayerAmmoCountEventSink() { };

	virtual    EventResult    ReceiveEvent(PlayerAmmoCountEvent * evn, void * dispatcher) override
	{
		_MESSAGE("ammoC");
		if ((*g_player)->equipData->slots[41].instanceData)
		{
			if (evn->weaponInstance && evn->weaponInstance != MSF_MainData::switchData.equippedInstanceData)
			{
				MSF_MainData::switchData.equippedInstanceData = evn->weaponInstance;
				//if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::APbaseMod))
				//{
				//	MSF_MainData::switchData.ClearData();
				//	MSF_MainData::switchData.SwitchFlags = SwitchData::bNeedInit;
				//	if (!(*g_ui)->IsMenuOpen(BSFixedString("pipboyMenu"))) //(!(*g_ui)->IsMenuOpen(BSFixedString("CursorMenu"))
				//		MSF_Base::InitWeapon();
				//}
				//MSF_Scaleform::UpdateWidget(127);
			}
		}
		else
		{
			MSF_MainData::switchData.equippedInstanceData = nullptr;
			MSF_MainData::switchData.ClearData();
			//MSF_Scaleform::UpdateWidget(127);
		}
		
		return kEvent_Continue;
	}
};
PlayerAmmoCountEventSink playerAmmoCountEventSink;

//Menu OpenClose Event
class MenuOpenCloseSink : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, void * dispatcher) override
	{
		const char* name = evn->menuName.c_str();
		//if (!_strcmpi("pipboyMenu", name))
		//{
		//	if (!evn->isOpen && (MSF_MainData::switchData.SwitchFlags & SwitchData::bNeedInit))
		//		MSF_Base::InitWeapon();
		//}
		if (!_strcmpi("pauseMenu", name) && !evn->isOpen)
		{
			MSF_Data::ReadMCMKeybindData();
			MSF_Data::ReadUserSettings();
		}
		
		return kEvent_Continue;
	}
};
MenuOpenCloseSink menuOpenCloseSink;

//volatile short count = 3;
//Animation event hook
UInt8 tf1_Hook(void* arg1, BSAnimationGraphEvent* arg2, void** arg3) //on abort?
{
	const char* name = arg2->eventName.c_str();
	if (!_strcmpi("reloadComplete", name))
	{
		_MESSAGE("reloadCOMP %02X", MSF_MainData::switchData.SwitchFlags);
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bReloadInProgress)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bReloadInProgress;
			MSF_Base::SwitchMod();
		}
	}
	if (!_strcmpi("reloadEnd", name))
	{
		_MESSAGE("reloadEnd");
		MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bReloadNotFinished;
	}
	else if (!_strcmpi("weaponDraw", name))
	{
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bDrawInProgress)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bDrawInProgress;
			if ((MSF_MainData::switchData.SwitchFlags & SwitchData::bReloadNeeded))
			{
				MSF_MainData::switchData.SwitchFlags = (MSF_MainData::switchData.SwitchFlags & ~SwitchData::bReloadNeeded) | SwitchData::bReloadInProgress; // | SwitchData::bReloadNotFinished
				if (!MSF_Base::ReloadWeapon())
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bReloadInProgress | SwitchData::bReloadNotFinished);
			}
			else if (SwitchData::bAnimNeeded)
			{
				MSF_MainData::switchData.SwitchFlags = (MSF_MainData::switchData.SwitchFlags & ~SwitchData::bAnimNeeded) | SwitchData::bAnimInProgress; //| SwitchData::bAnimNotFinished
				if (!MSF_Base::PlayAnim())
					MSF_MainData::switchData.SwitchFlags &= ~(SwitchData::bAnimInProgress | SwitchData::bAnimNotFinished);
			}
		}
	}
	else if (!_strcmpi("weaponFire", name))
	{
		MSF_Base::DamageEquippedWeapon(*g_player);
		if (MSF_MainData::tmr.IsRunning())
		{
			if (MSF_MainData::tmr.stop() < 1000)
				MSF_Base::FireBurst(*g_player);
		}
		//InterlockedDecrement16(&count);
		//if (count)
		//	Utilities::PlayIdle(*g_player, MSF_MainData::fireIdle1stP);
		//else
		//	InterlockedExchange16(&count, 3);
		//_MESSAGE("Anim: fire %i", MSF_MainData::tmr.stop());
	}
	else if (!_strcmpi("switchMod", name))
	{
		if (MSF_MainData::switchData.SwitchFlags & SwitchData::bAnimInProgress)
		{
			MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bAnimInProgress;
			MSF_Base::SwitchMod();
		}
		//_MESSAGE("Anim: switch");
	}
	else if (!_strcmpi("customAnimEnd", name))
	{
		MSF_MainData::switchData.SwitchFlags &= ~SwitchData::bAnimNotFinished;
	}
	return tf1_Original(arg1, arg2, arg3);
}

bool RegisterAfterLoadEvents()
{
	auto eventDispatcher4 = GET_EVENT_DISPATCHER(PlayerAmmoCountEvent);
	if (eventDispatcher4)
		eventDispatcher4->AddEventSink(&playerAmmoCountEventSink);
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
		//return false;
	}

	//BGSOnPlayerModArmorWeaponEvent_Dispatcher_address = RVA <BSTEventDispatcher<BGSOnPlayerModArmorWeaponEvent>*>(
	//	"BGSOnPlayerModArmorWeaponEvent_Dispatcher_address", {
	//		{ RUNTIME_VERSION_1_10_50, 0x00441610 },//{ RUNTIME_VERSION_1_10_163, 0x00441610 },
	//	}, "E8 ? ? ? ? 49 8B 0F 49 8D 56 10", BASIC_EVENT_HANDLER_INDIRECTIONS(0x2C3)); //.text:0000000141371568                 call    sub_141376150
	//if (BGSOnPlayerModArmorWeaponEvent_Dispatcher_address.GetUIntPtr() != 0)
	//{
	REGISTER_EVENT(BGSOnPlayerModArmorWeaponEvent, modArmorWeaponEventSink);
	REGISTER_EVENT(BGSOnPlayerUseWorkBenchEvent, useWorkbenchEventSink);
	//	_MESSAGE("ModEvent dispatcher address: %08X", BGSOnPlayerModArmorWeaponEvent_Dispatcher_address.GetUIntPtr());
	//}
	//else
	//{
	//	_MESSAGE("cant register BGSOnPlayerModArmorWeaponEvent:");
	//	//return false;
	//}

	//auto eventDispatcher6 = GET_EVENT_DISPATCHER(BGSOnPlayerModArmorWeaponEvent);
	//if (eventDispatcher6)
	//	eventDispatcher6->AddEventSink(&modArmorWeaponEventSink);
	//else
	//{
	//	_MESSAGE("MSF was unable to register for BGSOnPlayerModArmorWeaponEvent");
	//	//return false;
	//}
	//REGISTER_EVENT(TESCellFullyLoadedEvent, cellFullyLoadedEventSink);

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
			MSF_MainData::switchData.ClearData();
			MSF_MainData::switchData.equippedInstanceData = Utilities::GetEquippedInstanceData(*g_player, 41);
			//if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::APbaseMod))
			//{
			//	MSF_MainData::switchData.SwitchFlags = SwitchData::bNeedInit;
			//	//MSF_Base::InitWeapon();
			//}
			//MSF_Scaleform::UpdateWidget(127);
		}
		return kEvent_Continue;
	}
};

class CombatEvnHandler : public BSTEventSink<TESCombatEvent>
{
public:
	virtual ~CombatEvnHandler() { };
	virtual    EventResult    ReceiveEvent(TESCombatEvent * evn, void * dispatcher) override
	{
		//instance midprocess ammo count!
		_MESSAGE("combat started");
		return kEvent_Continue;
	}
};

void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{
	if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady)
	{
		if (msg->data == (void*)true)
		{
			//MSF_Scaleform::RegisterMCMCallback();
			if (!MSF_Data::InitData())
				_FATALERROR("MSF was unable to initialize plugin data"); 
			else if (!MSF_Data::InitMCMSettings())
				_FATALERROR("MSF was unable to initialize MCM settings");
			else
			{
				static auto pLoadGameHandler = new TESLoadGameHandler();
				GetEventDispatcher<TESLoadGameEvent>()->AddEventSink(pLoadGameHandler);
				MSF_Scaleform::ReceiveKeyEvents();

				RVAUtils::Timer tmr; 
				tmr.start();
				MSF_Data::LoadPluginData();
				_MESSAGE("Plugin Data Loading Time: %llu ms.", tmr.stop());

				//TESCellFullyLoadedEvent_Dispatcher_address = RVA <BSTEventDispatcher<TESCellFullyLoadedEvent>*>(
				//	"TESCellFullyLoadedEvent_Dispatcher_address", {
				//		{ RUNTIME_VERSION_1_10_50, 0x00441E30 },
				//	}, "E8 ? ? ? ? 49 8B 0F 49 8D 56 10", BASIC_EVENT_HANDLER_INDIRECTIONS1(0xA0));
				//if (TESCellFullyLoadedEvent_Dispatcher_address.GetUIntPtr() != 0)
				//{
				//	//REGISTER_EVENT(TESCellFullyLoadedEvent, cellFullyLoadedEventSink);
				//	_MESSAGE("CellEvent dispatcher address: %08X", TESCellFullyLoadedEvent_Dispatcher_address.GetUIntPtr());
				//}
				//else
				//{
				//	_ERROR("cant register TESCellFullyLoadedEvent:");
				//	//return false;
				//}
				//static auto pCombatHandler = new CombatEvnHandler();
				//GetEventDispatcher<TESCombatEvent>()->AddEventSink(pCombatHandler);
				REGISTER_EVENT(TESCellFullyLoadedEvent, cellFullyLoadedEventSink);
				//REGISTER_EVENT(TESCellAttachDetachEvent, cellAttachDetachEventSink);
			}
		}
	}
}

bool InitPlugin(UInt32 runtimeVersion = 0) {
	_MESSAGE("%s v%s dll loaded...\n", PLUGIN_NAME_SHORT, PLUGIN_VERSION_STRING);
	_MESSAGE("runtime version: %08X", runtimeVersion);
#ifdef DEBUG
	_MESSAGE("DevMode enabled!");
#endif
	Globals::Init();
	RVAManager::UpdateAddresses(runtimeVersion);

	if (!g_localTrampoline.Create(1024 * 64, nullptr)) {
		_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
		return false;
	}

	if (!g_branchTrampoline.Create(1024 * 64)) {
		_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
		return false;
	}
	if (!g_localTrampoline.Create(1024 * 64, nullptr))
	{
		_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
		return false;
	}

	_MESSAGE("Base address: %p", reinterpret_cast<uintptr_t>(GetModuleHandle(NULL)));

	tf1_Original = HookUtil::SafeWrite64(tf1_HookTarget.GetUIntPtr(), &tf1_Hook);

	InitializeCriticalSection(switchCriticalSection);

	return true;
}

extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\ModSwitchFramework.log");

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name =		PLUGIN_NAME_SHORT;
	info->version =		PLUGIN_VERSION;
	
	g_pluginHandle =	f4se->GetPluginHandle();
	
	// Check game version
	if (!COMPATIBLE(f4se->runtimeVersion)) 
	{
		char str[512];
		sprintf_s(str, sizeof(str), "Your game version: v%d.%d.%d.%d\nExpected version: v%d.%d.%d.%d\n%s will be disabled.",
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

	if (f4se->runtimeVersion > SUPPORTED_RUNTIME_VERSION) 
	{
		_FATALERROR("INFO: Newer game version (%08X) than target (%08X).", f4se->runtimeVersion, SUPPORTED_RUNTIME_VERSION);
	}

	g_messaging = (F4SEMessagingInterface *)f4se->QueryInterface(kInterface_Messaging);
	if (!g_messaging) 
	{
		_FATALERROR("Fatal Error - Messaging query failed");
		return false;
	}

	g_papyrus = (F4SEPapyrusInterface *)f4se->QueryInterface(kInterface_Papyrus);
	if (!g_papyrus) 
	{
		_FATALERROR("Fatal Error - Papyrus query failed");
		return false;
	}

	g_scaleform = (F4SEScaleformInterface *)f4se->QueryInterface(kInterface_Scaleform);
	if (!g_scaleform) 
	{
		_FATALERROR("Fatal Error - Scaleform query failed");
		return false;
	}

	if (GetFileAttributes("Data\\F4SE\\Plugins\\mcm.dll") == INVALID_FILE_ATTRIBUTES)
	{
		_FATALERROR("Fatal Error - Missing mcm.dll - Install MCM for this plugin to properly function");
		return false;
	}

	return true;
}

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
	if (g_messaging) 
	{
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);
//		g_messaging->RegisterListener(g_pluginHandle, "F4SE", OnF4SEMessage);
	}

	if (g_papyrus) 
	{
		g_papyrus->Register(MSF_Papyrus::RegisterPapyrus);
//		g_papyrus->Register(PapyrusStack::RegisterPapyrus);
//		g_papyrus->Register(PapyrusInstanceData::RegisterPapyrus);

#ifdef DEBUG
		g_papyrus->Register(MSF_Test::RegisterPapyrus);
#endif
	}

	if (g_scaleform)
	{
		if (!MSF_Data::ReadKeybindData())
		{
			_FATALERROR("MSF was unable to initialize keybinds");
			return false;
		}
		g_scaleform->Register("msf", MSF_Scaleform::RegisterMSFScaleform);
	}
	
	return InitPlugin(f4se->runtimeVersion);
}

}
