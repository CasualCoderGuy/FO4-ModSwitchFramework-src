#pragma once
#include "MSF_Shared.h"

namespace MSF_Localization
{
	bool ParseTranslations();
	const char* GetTranslation(std::string key);

	class Keys
	{
	public:
		static std::string modText;
		static std::string noAPText;
		static std::string noWeapText;
		static std::string couldntAttachText;
		static std::string itemText;
		static std::string unsupportedAmmoText;
		static std::string noAmmoTypeText;
		static std::string ammoEquipFailedText;
		static std::string ammoUnequipText;
		static std::string ammoEquippedText;
		static std::string missingLooseModText;
		static std::string pipboyMagsizeLoadedText;
		static std::string menuAmmoTypeText;
		static std::string menuMuzzleText;
		static std::string menuNoneText;
	};
}