#pragma once
#include "f4se_common/f4se_version.h"
#include "f4se/GameData.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameFormComponents.h"
#include "f4se/GameObjects.h"
#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusUtilities.h"
#include "f4se/GameCamera.h"
#include "rva/RVA.h"
#include  <thread>
#include  <chrono>

//-----------------------
// Plugin Information
//-----------------------
#define PLUGIN_VERSION				0x01
#define PLUGIN_VERSION_STRING		"0.1"
#define PLUGIN_NAME_SHORT			"MSF"
#define PLUGIN_NAME_LONG			"Mod Switch Framework"
#define SUPPORTED_RUNTIME_VERSION   CURRENT_RELEASE_RUNTIME
#define MINIMUM_RUNTIME_VERSION     CURRENT_RELEASE_RUNTIME
#define COMPATIBLE(runtimeVersion)  (runtimeVersion >= MINIMUM_RUNTIME_VERSION)
#define SCRIPTNAME "ModSwitchFramework"
#define MODNAME "anagy_ModSwitchFramework.esl"

#define DEBUG "DevMode"