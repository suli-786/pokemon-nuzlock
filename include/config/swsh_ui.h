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

// The map name pop-up has no toggle here on purpose: it is selected with
// OW_POPUP_GENERATION == GEN_8 in include/config/overworld.h, which is already
// an upstream config knob in the right place.

// Reserved for the branches still to be ported:
// #define SWSH_STORAGE_SYSTEM  FALSE   // Sword/Shield style PC box screen
// #define SWSH_PARTY_MENU      FALSE   // Sword/Shield style party menu
// #define SWSH_BAG_SCREEN      FALSE   // Sword/Shield style bag

#endif // GUARD_CONFIG_SWSH_UI_H
