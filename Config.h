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
#define DEBUG "DevMode"

#define MSF_VERSION_INTEGER_MAJOR	0
#define MSF_VERSION_INTEGER_MINOR	0
#define MSF_VERSION_INTEGER_PATCH	0
#define MSF_VERSION_INTEGER_BETA	4
#define MSF_VERSION_RELEASEID		0

#ifdef DEBUG
#define MSF_VERSION_STRING		"0.0.0.4" 
#define MSF_VERSION				MAKE_EXE_VERSION_EX(MSF_VERSION_INTEGER_MAJOR, MSF_VERSION_INTEGER_MINOR, MSF_VERSION_INTEGER_PATCH, MSF_VERSION_INTEGER_BETA)
#else
#define MSF_VERSION_STRING		"0.0.1"
#define MSF_VERSION				MAKE_EXE_VERSION_EX(MSF_VERSION_INTEGER_MAJOR, MSF_VERSION_INTEGER_MINOR, MSF_VERSION_INTEGER_PATCH, 0)
#endif

#define MIN_SUPPORTED_KB_VERSION				MAKE_EXE_VERSION_EX(0, 0, 0, 3)
#define MIN_SUPPORTED_DATA_VERSION				MAKE_EXE_VERSION_EX(0, 0, 0, 3)
#define MIN_SUPPORTED_SWF_WIDGET_VERSION		MAKE_EXE_VERSION_EX(0, 0, 0, 1)
#define MIN_SUPPORTED_SWF_AMMO_VERSION			MAKE_EXE_VERSION_EX(0, 0, 0, 1)
#define MIN_SUPPORTED_SWF_MOD_VERSION			MAKE_EXE_VERSION_EX(0, 0, 0, 1)
#define MIN_SUPPORTED_SERIALIZATION_VERSION		MAKE_EXE_VERSION_EX(0, 0, 0, 3)

#define SUPPORTED_RUNTIME_VERSION			CURRENT_RELEASE_RUNTIME
#define COMPATIBLE(runtimeVersion)			(runtimeVersion == SUPPORTED_RUNTIME_VERSION)

#define PLUGIN_NAME_SHORT		"MSF"
#define PLUGIN_NAME_LONG		"Mod Switch Framework"
#define SCRIPTNAME				"ModSwitchFramework"
#define MODNAME					"ModSwitchFramework.esl"
#define AUTHOR_NAME				"CasualCoderGuy"

class VersionData
{
public:
	UInt32 gameVersion;
	UInt32 f4seVersion;
	UInt32 msfVersion;
};

#ifdef DEBUG
inline void _DEBUG(const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	gLog.Log(IDebugLog::kLevel_Message, fmt, args);
	va_end(args);
}
#else
inline void _DEBUG(const char* fmt, ...)
{

}
#endif