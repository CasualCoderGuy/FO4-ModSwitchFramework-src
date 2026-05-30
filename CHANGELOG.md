---
v1.2.6
features:
- support for Fallout 4 1.11.221 with F4SE 0.7.8
- BCR support for NextGen
- added options to cancel pending mod switches with the attack key, ready key, mod switch hotkeys, and the repurposed debug cancel mod switch key
fixes:
- equipped ammo is now displayed in the pipboy correctly and reliably (this feature is now enabled by default)
- NPC ammo type spawn will only occur if the mod switch succeeded preventing missing equipped ammo in case of failure
- fixed a possible CTD during widget update
---
v1.2.5
features:
- fast equip animations are now disabled by default when using animation graph update (bUpdateAnimGraph); if the animation needs to be played use the bPlayFastEquipAnim OMOD flag or pass it as a Papyrus argument (Papyrus scripts have to be updated) 
---
v1.2.4
features:
- added melee and unarmed support for MSF widgets
- added feedback sounds (fail/success/next sounds for ammo & mod switching, menus, quickkeys)
- localization support with UTF-8 BOM encoded character sets from Data/MSF/Translations/MSF_[loc].txt (for sysntax and localized strings see MSF_en.txt)
- localization and priority support for widget display strings (see manual for more information)
- MSF widget and menus can now request game localization and receive localization and power armor status with the sent menu data
fixes:
- roll back on general animation graph updates introduced in v1.2.2 as it caused several issues
- PlayIdleAction dependent functions (reload, lower weapon, etc.) now properly work in the NG build
- fixed potential CTDs where OMODs in the objects mod list were not valid
- fixed potential CTD when casting objects from inventory events
- disabled BCR interface for NG that caused CTDs; until BCR RE is done it is not supported on NG
- replaced F4SE LoadMovie function that was missing an argument which may have caused visual bugs in the NG build
- some missing MSF widget functions are now properly registered
- custom projectile overrides now properly update when loading a save or modding a weapon at the workbench
---
v1.2.3
features:
- MSF now implements mod selection menus as proper individual custom menus, instead of injecting them to MSFMenu.swf; this requires new implementations in Action Script for the interface files (see MSF manual)
fixes:
- AttachRemoveModToInventoryStack now allows attaching mods to items without ExtraData
- AttachRemoveModToEquippedItem now allows attaching mods to equipped items that have missing equip flags
- circumvents the pipboy light event source override by BakaFramework.dll, which prevented MSF to function properly
- fixed possible CTD when invalid weapon state IDs were present
---
v1.2.2
features:
- animation graph updates now occur for every mod switch, from now on use the bUpdateAnimGraph flag only if the equip action should be played
- ammo count and mod requirements are now updated in the selection menus (new UI implementation needed, see MSF manual)
- when an ammo type runs out, now it is possible to automatically switch to the next/first available ammo type (can be set in the MCM)
- autolower now can be set to be ignored if the pipboy light is on
fixes:
- fixed potential memory leak with SwitchData
- fixed potential CTD with the emptyMag animation event
- fixed erroneous BCR/TR logic when switching ammo from UI menus
- fixed a bug that caused OMODs to be removed before the new OMOD was attached during a mod switch
- mixed ammo and mod selection menu now receives mods from all possible attach points (new UI implementation might be needed, see MSF manual)
---
v1.2.1
features:
- support for the Anniversary Update
- included a patch for vanilla Actor Value calculation, which caused incorrect substraction of AVs when changing mods on equipped items (including the AttachModToInventoryItem papyrus function)
- added GetModForAmmo and GetAmmoTypesForBaseAmmo Papyrus functions
- added MCM options to randomize the loaded ammo count for newly equipped weapons and split stacks
- added an MCM option to automatically set the clip counter to zero before ammo switching
- added an MCM option to turn off TR animation support
- added an MCM option to turn off TR chamber support
- added an MCM option to turn off extra weapon state storage
fixes:
- fixed possible CTD when duplicate weapon state IDs were present
- fixed bugs related to equipping, mod validation, and duplicate weapon states occuring when the equipped stack was split
- added proper weapon state copy and cleanup protocol corresponding to ExtraRank cloning and destructor
- the base ammo replacement option in the MCM will now correctly be independent of the spawn random ammo option
- compatibility patch for the Put Your Gun In mod
---
v1.2.0
features:
- added ammo cycling feature with hotkey
- added quick ammo cycling feature
- added base ammo mod functionality
- added automatic weapon lowering feature
- now it is possible to switch mods before the mod switch animation plays by setting the bDoSwitchBeforeAnimations flag (both ammo and omod switching)
- reworked ammo checking, now an ammo type can have multiple ammo type omods associated with it
- implemented loose mod equip from pipboy
- now support for tactical reload can be independently specified as a chamberData flag
- when spawning random ammo on NPCs, now it is possible to replace the base ammo with the same amount
- added new ammo spawn amout multiplier json variables for ammo types 
- added a base ammo spawn chance multiplier option to MCM settings
- implemented MSFwidget settings
fixes:
- fixed possible CTDs caused by missing InstanceData
- fixed keyboard input focus issue in action script
---
v1.1.1
features:
- added new animation variables: LoadedAmmoCount, MagAmmoCount, AmmoCapacity, NextReloadAmmoCount
fixes:
- fixed an issue with animation variables
- more robust hook checks and more informative error/debug messages
- easier version input in json files: each digit of the integer is a version number (current version: 1110)
- more informative pipboy ammo equip messages
file fixes:
- fixed an issue with keybinds that caused loss of functionality
- updated the behavior files with the latest hkxpack version, 3rd person behaviors added to main file as they no longer cause CTDs
- updated the pdf manual 
---
v1.1.0
features:
- integrated automatic leveled list injection and chances
- *Experimental* - implemented BGSProjectile properties for OMODs (with external json inputs)
- more detailed stored data printing
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