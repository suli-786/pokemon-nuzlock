#ifndef GUARD_CONFIG_SWSH_UI_H
#define GUARD_CONFIG_SWSH_UI_H

// Master toggles for the ported Sword/Shield-style UI screens
// (branches by Montblanc / montmoguri, see docs/overhaul/REGISTRY.md).
//
// Convention (docs/overhaul/UI_PORT_CHECKLIST.md §1): only the master on/off
// switch for each screen lives here. Every look-and-feel knob stays in the
// screen's own header (include/swsh_*.h). This file is pulled in from
// include/constants/global.h, so these toggles are defined in every
// translation unit no matter what the include order is -- upstream relied on
// consumers including the feature header first, which silently compiled the
// feature out whenever they didn't.

#define SWSH_SUMMARY_SCREEN     TRUE    // Sword/Shield style Pokémon summary screen. Tuning: include/swsh_summary_screen.h
#define SWSH_STORAGE_SYSTEM     TRUE    // Sword/Shield style PC box screen.         Tuning: include/swsh_storage_system.h
#define SWSH_PARTY_MENU         TRUE    // Sword/Shield style party menu.            Tuning: include/swsh_party_menu.h
#define SWSH_MESSAGE_BOX        TRUE    // Sword/Shield style message box + name box. Tuning: include/menu.h, include/config/name_box.h
#define SWSH_ITEM_MENU          TRUE    // Sword/Shield style bag screen.            Tuning: include/swsh_item_menu.h
#define SWSH_BATTLE_UI          TRUE    // Sword/Shield style battle HUD.            Tuning: include/menu.h, src/battle_interface.c

// SWSH_BATTLE_UI is the least finished of the six: see docs/overhaul/UI_PORT_CHECKLIST.md
// §3.6.10 for the "known unfinished" list. Flipping it FALSE restores the stock battle
// screen -- art, healthbox text positions, exp bar, ability pop-up, type icon
// positions and the move-description frame all revert together, no other edit needed.

// The map name pop-up has no toggle here on purpose: it is selected with
// OW_POPUP_GENERATION == GEN_8 in include/config/overworld.h, which is already
// an upstream config knob in the right place.

// All five SwSh UI branches are ported; nothing is reserved here any more.
// (The bag toggle is spelled SWSH_ITEM_MENU, not the SWSH_BAG_SCREEN this
// comment used to reserve, so it matches the ~9000 lines of ported code that
// test it -- same reasoning as SWSH_STORAGE_SYSTEM / SWSH_PARTY_MENU.)

#endif // GUARD_CONFIG_SWSH_UI_H
