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