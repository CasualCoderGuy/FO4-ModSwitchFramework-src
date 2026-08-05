#pragma once

#ifdef MSF_DLL
#define MSF_API __declspec(dllexport)
#else
#define MSF_API __declspec(dllimport)
#endif

namespace MSF
{
	enum class EventResult : std::int32_t
	{
		kProcessed,
		kNotProcessed
	};

	template <class EventData>
	class EventSink
	{
	public:
		virtual ~EventSink() = default;
		virtual	EventResult ReceiveEvent(const EventData& eventData) { return EventResult::kProcessed; };
	};

	namespace WidgetUpdate
	{
		class SettingsData
		{
		public:
			uint64_t MSFsettingsInMCM;	//see MSF_Data.h
			uint32_t fontType;
			uint32_t RGBcolor;
			float fSliderMainX;
			float fSliderMainY;
			float fPowerArmorOffsetX;
			float fPowerArmorOffsetY;
			float fSliderAlpha;
			float fSliderScale;
			const char* localization;	//game localization string
		};

		class DisplayData
		{
		public:
			const char* ammoName;
			const char* firingMode;
			const char* muzzleName;
			const char* scopeName;
			uint32_t shapeID;
			bool isInPA;
		};

		class QuickkeyData
		{
		public:
			const char* APname;
			const char* modName;
			bool isAmmo;
		};

		class ClearQuickkeyMod
		{
		public:
		};
	}
}

extern "C" MSF_API bool MSF_RegisterWidgetSettingsUpdateSink(MSF::EventSink<MSF::WidgetUpdate::SettingsData>* sink);
extern "C" MSF_API bool MSF_RegisterWidgetDisplayUpdateSink(MSF::EventSink<MSF::WidgetUpdate::DisplayData>* sink);
extern "C" MSF_API bool MSF_RegisterWidgetQuickkeyUpdateSink(MSF::EventSink<MSF::WidgetUpdate::QuickkeyData>* sink);
extern "C" MSF_API bool MSF_RegisterWidgetQuickkeyClearSink(MSF::EventSink<MSF::WidgetUpdate::ClearQuickkeyMod>* sink);
extern "C" MSF_API bool MSF_GetWidgetSettingsData(MSF::WidgetUpdate::SettingsData & data);
extern "C" MSF_API bool MSF_GetWidgetDisplayData(MSF::WidgetUpdate::DisplayData & data);