#pragma once
#include "f4se/ScaleformLoader.h"
#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformMovie.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/PapyrusScaleformAdapter.h"
#include "f4se/PapyrusEvents.h"
#include "f4se/GameMenus.h"
#include "f4se/GameInput.h"
#include "f4se/InputMap.h"
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Base.h"

class GFxMovieRoot;
class GFxMovieView;
class GFxValue;

struct KeybindInfo;

namespace MSF_Scaleform
{
	void ReceiveKeyEvents();
	bool RegisterMSFScaleform(GFxMovieView* view, GFxValue* f4se_root);
	bool RegisterMCMCallback();
	void RegisterMCMScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot);
	void RegisterMSFScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot);
	bool UpdateWidget(UInt8 flags);
	bool ToggleAmmoMenu(std::string path);
	bool ToggleModMenu(std::string path, std::vector<ModAssociationData*>* modAssociations);
}
