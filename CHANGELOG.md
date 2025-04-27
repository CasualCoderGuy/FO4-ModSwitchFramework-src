---
v1.0.1
features:
- BCR and TR support added (loaded bullets remain in the chamber; when switching ammo, the full reload animations should be triggered; added forced reload capability to PlayIdleAction)
- added the option to display mag size and loaded ammo count for weapons in the Pipboy menu
- ammo can now be switched through the Pipboy menu
- option to disable automatic reload added
- chambered and magazine ammo type and count should now be accurately tracked in ExtraWeaponState
- option to display currently equipped ammo in the Pipboy menu has been added
- a new reload type is added, when the ammo needs to be emptied from the weapon before reloading (use emptyMag animation event and MSF_AnimsEmptyBeforeReload keyword)
- added new animation variables (SwitchAmmoTypeReload, ChamberedAmmoCount, ChamberSize, and reserved variables)
- added custom mod switch animation compatibility setting (for behavior file incompatibilities); if this is turned on in the MCM, the mod change will happen at the end of the animation instead of after the "switchMod" event (Note: this can cause the mod to not be switched even if the custom animation sent the "switchMod" event before it was interrupted)
fixes:
- the animations called with PlayIdle will not play WPNEquipFast (if you do need it to be played for a mod switch, specify the bShouldNotStopIdle(0x10000) flag)
- fixed a CTD that occured during serialization
- fixed issues occurring with special ammo; fusion core type ammo and not playable ammo are not supported
- the widget now should not display weapon info after a weapon has been unequipped
- fixed an issue with weapon state storage
- fixed an issue with weapon state mods serialization
- safer task delay
- fixed a bug where bNotRequireWeaponToBeDrawn was not taken into account
---
v0.1.1
features:
- added OMOD storage for Weapon States (bHasUniqueState)
- MSF now supports mod switching for unarmed and melee weapons
- added the ability to specify chamber sizes and properties for OMODs (default chamber size: ballistic weapons: 1; other: 0)
- added reload animation compatibility setting (e.g. BCR, TR); with this setting turned on the ammo change will take place before the reload animation (note: this can cause the ammo to be changed even if the reload animation is interrupted)
- added adjustable reload and custom animation end delays (cooldowns), before a next switch can be initiated (warning: too short delay can result in a stuck switch state, because the next animation cannot be started when the previous animation is still playing, if you experience this use the cancel switch hotkey or initiate the animation manually)
- added several new Papyrus functions
fixes:
- changed hasSwithedAmmo keyword use to attach parent check; hasSwithedAmmo keyword is obsolete
- scaleform requests for binary versions now return the correct version numbers
- fixed a CTD caused by calling the AttachMod/AttachModToInventoryItem papyrus functions on some items
- fixed an issue when reading spawn chance for a mod in a mod cycle
- when evaluating an attach parent, now the mod with the highest priority will be regarded as the parent mod
- fixed an error that occured when loading invalid weapon states 