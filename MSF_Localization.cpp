#include "MSF_Localization.h"
#include "MSF_Data.h"
#include "f4se\GameSettings.h"
#include "f4se\GameStreams.h"
#include <comdef.h>

namespace MSF_Localization
{
	std::string Keys::modText = "$MOD_AT_WORKBENCH";
	std::string Keys::noAPText = "$NO_ATTACHPOINT";
	std::string Keys::noWeapText = "$NO_WEAPON";
	std::string Keys::couldntAttachText = "$COULD_NOT_ATTACH";
	std::string Keys::itemText = "$CANNOT_EQUIP";
	std::string Keys::unsupportedAmmoText = "$CANNOT_EQUIP_NP_AMMO";
	std::string Keys::noAmmoTypeText = "$CANNOT_EQUIP_AMMOTYPE";
	std::string Keys::ammoEquipFailedText = "$CANNOT_EQUIP_AMMO_RN";
	std::string Keys::ammoUnequipText = "$CANNOT_UNEQUIP_AMMO";
	std::string Keys::ammoEquippedText = "$AMMO_EQUIPPED";
	std::string Keys::missingLooseModText = "$MISSING_LOOSEMOD";
	std::string Keys::pipboyMagsizeLoadedText = "$PIP_MAGSIZE_LOADED";
	std::string Keys::menuAmmoTypeText = "$MENU_AMMOTYPE";
	std::string Keys::menuMuzzleText = "$MENU_MUZZLE";
	std::string Keys::menuNoneText = "$MENU_NONE";

	void FillDefaults()
	{
		MSF_MainData::loc = "en_d";
		MSF_MainData::translationMap[Keys::modText] = "This mod can only be attached at a workbench.";
		MSF_MainData::translationMap[Keys::noAPText] = "The equipped weapon does not have a compatible attach point for this mod.";
		MSF_MainData::translationMap[Keys::noWeapText] = "There is no compatible weapon equipped for this mod.";
		MSF_MainData::translationMap[Keys::couldntAttachText] = "Could not attach this mod to the equipped weapon.";
		MSF_MainData::translationMap[Keys::itemText] = "You cannot equip this item.";
		MSF_MainData::translationMap[Keys::unsupportedAmmoText] = "You cannot equip non-playable ammo or fusion cores.";
		MSF_MainData::translationMap[Keys::noAmmoTypeText] = "You cannot equip this ammo type.";
		MSF_MainData::translationMap[Keys::ammoEquipFailedText] = "You cannot equip this ammo right now.";
		MSF_MainData::translationMap[Keys::ammoUnequipText] = "You cannot unequip ammo.";
		MSF_MainData::translationMap[Keys::ammoEquippedText] = "Ammo equipped.";
		MSF_MainData::translationMap[Keys::missingLooseModText] = "Missing required loose mod.";
		MSF_MainData::translationMap[Keys::pipboyMagsizeLoadedText] = "Mag Size/Loaded";
		MSF_MainData::translationMap[Keys::menuAmmoTypeText] = "Ammo Type";
		MSF_MainData::translationMap[Keys::menuMuzzleText] = "Muzzle";
		MSF_MainData::translationMap[Keys::menuNoneText] = "None";
	}

	bool ParseTranslations()
	{
		Setting* setting = GetINISetting("sLanguage:General");
		std::string path = "Data\\MSF\\Translations\\";
		std::string loc = (setting && setting->GetType() == Setting::kType_String) ? setting->data.s : "en";
		_DEBUG("translate: %s", loc.c_str());

		if (MSF_MainData::loc == loc)
			return true;

		FillDefaults();

		// Construct translation filename
		path += "MSF_";
		path += loc;
		path += ".txt";
		_DEBUG("translate path: %s", path.c_str());

		if (GetFileAttributes(path.c_str()) == INVALID_FILE_ATTRIBUTES)
			_DEBUG("translate missing");
		BSResourceNiBinaryStream fileStream(path.c_str());
		if (!fileStream.IsValid())
			return false;
		else
			_MESSAGE("Reading translations from %s...", path.c_str());

		// Check if file is empty, if not check if the BOM is UTF-16
		UInt32	bom = 0;
		UInt32	ret = fileStream.Read(&bom, sizeof(UInt32));
		if (ret == 0) {
			_MESSAGE("Empty translation file.");
			return false;
		}
		if ((bom & 0xFFFFFF) != 0xBFBBEF) {
			_MESSAGE("BOM Error, translation file must be encoded in UTF-8 BOM. 0x%06X", bom & 0xFFFFFF);
			return false;
		}

		while (true)
		{
			char buf[512];
			UInt32	len = fileStream.ReadLine(buf, sizeof(buf) / sizeof(buf[0]), '\n');
			if (len == 0) // End of file
				break;

			// at least $ + char + \t + char
			if (len < 4 || buf[0] != '$')
				continue;

			char last = buf[len - 1];
			if (last == '\r')
				len--;

			// null terminate
			buf[len] = 0;

			UInt32 delimIdx = 0;
			for (UInt32 i = 0; i < len; i++)
				if (buf[i] == '\t')
					delimIdx = i;

			// at least $ + char
			if (delimIdx < 2)
				continue;

			// replace \t by \0
			buf[delimIdx] = 0;

			std::string key(buf);
			std::string translation(&buf[delimIdx + 1]);

			MSF_MainData::translationMap[key] = translation;
			
		}
		MSF_MainData::loc = loc;
		return true;
	}

	const char* GetTranslation(std::string key)
	{
		auto trIt = MSF_MainData::translationMap.find(key);
		if (trIt == MSF_MainData::translationMap.end())
			return "";
		return trIt->second.c_str();
	}
}