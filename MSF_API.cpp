#include "MSF_Events.h"
#include "MSF_Localization.h"
#include "f4se\GameSettings.h"

extern "C" MSF_API bool MSF_RegisterWidgetSettingsUpdateSink(MSF::EventSink<MSF::WidgetUpdate::SettingsData>*sink)
{
    return MSFwidgetSettingsDataUpdateSource.RegisterSink(sink);
}

extern "C" MSF_API bool MSF_RegisterWidgetDisplayUpdateSink(MSF::EventSink<MSF::WidgetUpdate::DisplayData>*sink)
{
    return MSFwidgetDisplayDataUpdateSource.RegisterSink(sink);
}

extern "C" MSF_API bool MSF_RegisterWidgetQuickkeyUpdateSink(MSF::EventSink<MSF::WidgetUpdate::QuickkeyData>*sink)
{
    return MSFwidgetQuickkeyDataUpdateSource.RegisterSink(sink);
}

extern "C" MSF_API bool MSF_RegisterWidgetQuickkeyClearSink(MSF::EventSink<MSF::WidgetUpdate::ClearQuickkeyMod>*sink)
{
    return MSFwidgetClearQuickkeySource.RegisterSink(sink);
}

extern "C" MSF_API bool MSF_GetWidgetSettingsData(MSF::WidgetUpdate::SettingsData& data)
{
	Setting* setting = GetINISetting("sLanguage:General");
	std::string loc = (setting && setting->GetType() == Setting::kType_String) ? setting->data.s : "en";

	data.MSFsettingsInMCM = MSF_MainData::MCMSettingFlags;
	data.fontType = MSF_MainData::widgetSettings.iFont;
	data.RGBcolor = MSF_MainData::widgetSettings.GetRGBcolor();
	data.fSliderMainX = MSF_MainData::widgetSettings.fSliderMainX;
	data.fSliderMainY = MSF_MainData::widgetSettings.fSliderMainY;
	data.fPowerArmorOffsetX = MSF_MainData::widgetSettings.fPowerArmorOffsetX;
	data.fPowerArmorOffsetY = MSF_MainData::widgetSettings.fPowerArmorOffsetY;
	data.fSliderAlpha = MSF_MainData::widgetSettings.fSliderAlpha;
	data.fSliderScale = MSF_MainData::widgetSettings.fSliderScale;
	data.localization = loc.c_str();
    return true;
}

extern "C" MSF_API bool MSF_GetWidgetDisplayData(MSF::WidgetUpdate::DisplayData& data)
{
	Actor* playerActor = *g_player;
	auto instanceData = Utilities::GetEquippedWeaponInstanceData(playerActor);
	data.ammoName = "";
	std::string fmstr = MSF_Data::GetFMString(instanceData);
	std::string muzzlestr = MSF_Data::GetMuzzleString(instanceData);
	std::string scopestr = MSF_Data::GetScopeString(instanceData);
	data.firingMode = fmstr.c_str();
	data.muzzleName = muzzlestr.c_str();
	data.scopeName = scopestr.c_str();
	data.shapeID = 0; //baseAmmoID << 10 + ammoType + muzzleID << 20 + scopeID << 26
	if (instanceData && instanceData->ammo)
		data.ammoName = instanceData->ammo->GetFullName();
	data.isInPA = IsInPowerArmor(playerActor);
    return true;
}
