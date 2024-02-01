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
#include "MSF_Test.h"


IDebugLog				gLog;
PluginHandle			g_pluginHandle =	kPluginHandle_Invalid;
F4SEPapyrusInterface	*g_papyrus   =		NULL;
F4SEMessagingInterface	*g_messaging =		NULL;
F4SEScaleformInterface	*g_scaleform =		NULL;

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
		return false;
	}

	REGISTER_EVENT(BGSOnPlayerModArmorWeaponEvent, modArmorWeaponEventSink);
	REGISTER_EVENT(BGSOnPlayerUseWorkBenchEvent, useWorkbenchEventSink);

	if (!RegisterInventoryEvent((*g_player)->inventoryList, &MSF_MainData::playerInventoryEventSink))
	{
		_MESSAGE("MSF was unable to register for PlayerInventoryEvent");
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
			MSF_MainData::modSwitchManager.Reset();
			MSF_MainData::modSwitchManager.SetCurrentWeapon(Utilities::GetEquippedInstanceData(*g_player, 41));

			_MESSAGE("Registering MSF menus.");
			MSFWidgetMenu::RegisterMenu();
			//MSFAmmoMenu::RegisterMenu();
			//MSFModMenu::RegisterMenu();
			MSFWidgetMenu::OpenMenu();
			delayTask delayUpd(500, true, &MSF_Scaleform::UpdateWidgetData);

			//BSScaleformTranslator * translator = (BSScaleformTranslator*)(*g_scaleformManager)->stateBag->GetStateAddRef(GFxState::kInterface_Translator);
			//if (translator) {
			//	SSWTranslator::LoadTranslations(translator);
			//}
			MSFMenu::RegisterMenu();
			MSFMenu::OpenMenu();
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

				REGISTER_EVENT(TESCellFullyLoadedEvent, cellFullyLoadedEventSink);

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
	PlayerAnimationEvent_Original = HookUtil::SafeWrite64(PlayerAnimationEvent_HookTarget.GetUIntPtr(), &PlayerAnimationEvent_Hook);
	//g_branchTrampoline.Write5Call(AttackBlockHandler_HookTarget.GetUIntPtr(), (uintptr_t)AttackBlockHandler_Hook);

	HUDShowAmmoCounter_Copied = HookUtil::GetFnPtrFromCall5(HUDShowAmmoCounter_HookTarget.GetUIntPtr(), &HUDShowAmmoCounter_Hook);
	EquipHandler_UpdateAnimGraph_Copied = HookUtil::GetFnPtrFromCall5(EquipHandler_UpdateAnimGraph_HookTarget.GetUIntPtr(), &EquipHandler_UpdateAnimGraph_Hook);
	AttachModToStack_CallFromGameplay_Copied = HookUtil::GetFnPtrFromCall5(AttachModToStack_CallFromGameplay_HookTarget.GetUIntPtr(), &AttachModToStack_CallFromGameplay_Hook);
	AttachModToStack_CallFromWorkbenchUI_Copied = HookUtil::GetFnPtrFromCall5(AttachModToStack_CallFromWorkbenchUI_HookTarget.GetUIntPtr(), &AttachModToStack_CallFromWorkbenchUI_Hook);

	//if (check copied address validity)
	//	return false;

	g_branchTrampoline.Write5Call(HUDShowAmmoCounter_HookTarget.GetUIntPtr(), (uintptr_t)HUDShowAmmoCounter_Hook);
	g_branchTrampoline.Write5Call(EquipHandler_UpdateAnimGraph_HookTarget.GetUIntPtr(), (uintptr_t)EquipHandler_UpdateAnimGraph_Hook);
	g_branchTrampoline.Write5Call(AttachModToStack_CallFromGameplay_HookTarget.GetUIntPtr(), (uintptr_t)AttachModToStack_CallFromGameplay_Hook);
	g_branchTrampoline.Write5Call(AttachModToStack_CallFromWorkbenchUI_HookTarget.GetUIntPtr(), (uintptr_t)AttachModToStack_CallFromWorkbenchUI_Hook);

	//if (check write success)
	//	return false;
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

bool InitPlugin(UInt32 runtimeVersion = 0) {
	_MESSAGE("%s v%s dll loaded...\n", PLUGIN_NAME_SHORT, MSF_VERSION_STRING);
	_MESSAGE("runtime version: %08X", runtimeVersion);
#ifdef DEBUG
	_MESSAGE("DevMode enabled!");
#endif

	if (GetFileAttributes("Data\\F4SE\\Plugins\\mcm.dll") == INVALID_FILE_ATTRIBUTES)
	{
		_WARNING("Warning - Missing mcm.dll - Keybinds and settings are read from MCM files. While it is possible to maintain these manually, installing and using MCM is highly recommended.");
		//_FATALERROR("Fatal Error - Missing mcm.dll - Install MCM for this plugin to properly function");
		//return false;
	}

	Globals::Init();
	RVAManager::UpdateAddresses(runtimeVersion);

	if (!g_localTrampoline.Create(1024 * 64, nullptr)) {
		_ERROR("Fatal Error - Couldn't create codegen buffer. Skipping remainder of init process.");
		return false;
	}

	if (!g_branchTrampoline.Create(1024 * 64)) {
		_ERROR("Fatal Error - Couldn't create branch trampoline. Skipping remainder of init process.");
		return false;
	}

	_MESSAGE("Base address: %p", reinterpret_cast<uintptr_t>(GetModuleHandle(NULL)));

	//InitializeOffsets();

	if (!WriteHooks())
		return false;

	if (!MSF_Data::ReadKeybindData())
	{
		_FATALERROR("Fatal Error - MSF was unable to initialize keybinds");
		return false;
	}

	if (!g_scaleform->Register(PLUGIN_NAME_SHORT, MSF_Scaleform::RegisterScaleformCallback))
		_MESSAGE("MSF scaleform registration failed");
	//if (!g_scaleform->Register(PLUGIN_NAME_SHORT, MSF_Scaleform::RegisterScaleformTest))
	//	_MESSAGE("MSF widget scaleform registration failed");

	srand(time(NULL));

	return true;
}

extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\ModSwitchFramework.log");

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name =		PLUGIN_NAME_SHORT;
	info->version =		MSF_VERSION;
	
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

	return true;
}

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
	if (g_messaging) 
	{
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);
		//g_messaging->RegisterListener(g_pluginHandle, NULL, MSFMessageHandler);
	}

	if (g_papyrus) 
	{
		g_papyrus->Register(MSF_Papyrus::RegisterPapyrus);
#ifdef DEBUG
		g_papyrus->Register(MSF_Test::RegisterPapyrus);
#endif
	}
	
	return InitPlugin(f4se->runtimeVersion);
}

}
