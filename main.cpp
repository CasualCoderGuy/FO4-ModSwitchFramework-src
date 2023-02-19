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
		//return false;
	}

	REGISTER_EVENT(BGSOnPlayerModArmorWeaponEvent, modArmorWeaponEventSink);
	REGISTER_EVENT(BGSOnPlayerUseWorkBenchEvent, useWorkbenchEventSink);

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

			_MESSAGE("Registering MSF menus.");
			MSFWidgetMenu::RegisterMenu();
			MSFAmmoMenu::RegisterMenu();
			MSFModMenu::RegisterMenu();
			MSFWidgetMenu::OpenMenu();
			//MSFMenu::OpenMenu();
			//if (!Utilities::HasObjectMod(Utilities::GetEquippedModData(*g_player, 41), MSF_MainData::APbaseMod))
			//{
			//	MSF_MainData::switchData.SwitchFlags = SwitchData::bNeedInit;
			//	//MSF_Base::InitWeapon();
			//}
			delayTask delayUpd(500, true, &MSF_Scaleform::UpdateWidgetData);
			//MSF_Scaleform::UpdateWidgetData();
			// Inject translations
			//BSScaleformTranslator * translator = (BSScaleformTranslator*)(*g_scaleformManager)->stateBag->GetStateAddRef(GFxState::kInterface_Translator);
			//if (translator) {
			//	SSWTranslator::LoadTranslations(translator);
			//}
			//MSFMenu::RegisterMenu();
			//static BSFixedString menuName("MSFMenu");
			//BSReadAndWriteLocker locker(&g_customMenuLock);
			//g_customMenuData[menuName.c_str()].menuPath = menuName;
			//g_customMenuData[menuName.c_str()].rootPath = "root1";
			//g_customMenuData[menuName.c_str()].menuFlags = 2050;
			//g_customMenuData[menuName.c_str()].movieFlags = 0;
			//g_customMenuData[menuName.c_str()].extFlags = 0;
			//g_customMenuData[menuName.c_str()].depth = 6;
			//(*g_ui)->Register(menuName.c_str(), CreateCustomMenu);
			//_MESSAGE("RegisterMenu %s", (*g_ui)->IsMenuRegistered(menuName) ? "registered" : "not registered");
		}
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

				REGISTER_EVENT(TESCellFullyLoadedEvent, cellFullyLoadedEventSink);

				MSF_Data::InitCompatibility();
			}
		}
	}
}

bool InitPlugin(UInt32 runtimeVersion = 0) {
	_MESSAGE("%s v%s dll loaded...\n", PLUGIN_NAME_SHORT, MSF_VERSION_STRING);
	_MESSAGE("runtime version: %08X", runtimeVersion);
#ifdef DEBUG
	_MESSAGE("DevMode enabled!");
#endif

	if (GetFileAttributes("Data\\F4SE\\Plugins\\mcm.dll") == INVALID_FILE_ATTRIBUTES)
	{
		_FATALERROR("Fatal Error - Missing mcm.dll - Install MCM for this plugin to properly function");
		return false;
	}

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
	g_branchTrampoline.Write5Call(HUDShowAmmoCounter_HookTarget.GetUIntPtr(), (uintptr_t)HUDShowAmmoCounter_Hook);

	if (!MSF_Data::ReadKeybindData())
	{
		_FATALERROR("Fatal Error - MSF was unable to initialize keybinds");
		return false;
	}

	//if (!g_scaleform->Register(PLUGIN_NAME_SHORT, MSF_Scaleform::RegisterScaleformCallback))
	//	_MESSAGE("MSF widget scaleform registration failed");

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
