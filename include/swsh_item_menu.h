#ifndef GUARD_SWSH_ITEM_MENU_H
#define GUARD_SWSH_ITEM_MENU_H

#include "config/swsh_ui.h"

// Look-and-feel / behaviour knobs for the SwSh bag screen. The master on/off
// switch is SWSH_ITEM_MENU in include/config/swsh_ui.h -- see
// docs/overhaul/UI_PORT_CHECKLIST.md §1 for why it lives there and not here.

#define SWSH_ITEM_MENU_CONTEST_INFO     (SWSH_ITEM_MENU && TRUE)                // Show contest info for TMs/HMs in the item menu
#define SWSH_ITEM_MENU_BERRY_STAT       (SWSH_ITEM_MENU && FALSE)               // Show berry stat (flavors, size, etc.) in the item menu
#define SWSH_ITEM_MENU_BERRY_TAG        (SWSH_ITEM_MENU_BERRY_STAT && FALSE)    // Show berry tag info
#define SWSH_ITEM_MENU_SCROLLING_BG     (SWSH_ITEM_MENU && TRUE)                // Enable scrolling background (BG3)

// Overhaul: DELIBERATELY FALSE, see docs/overhaul/UI_PORT_CHECKLIST.md §3.5.
// Upstream ships this TRUE. When TRUE the bag performs Use/Give itself, on a
// party panel drawn inside the bag, instead of handing the item to the party
// menu. That bypasses src/swsh_party_menu.c, where the two GetItemConsumability
// guards that keep ITEM_ENDLESS_CANDY / ITEM_CAP_CANDY infinite-use live. Until
// equivalent guards exist on the in-bag path, item actions stay on the party
// menu route. Turning this back on is a code change, not just a flag flip --
// the checklist section lists the hunks that have to come back with it.
#define SWSH_ITEM_MENU_IN_BAG_USE       (SWSH_ITEM_MENU && FALSE)               // Perform item actions (Use/Give) in bag (skip party menu)

#define SWSH_ITEM_MENU_IN_BAG_REUSE     (SWSH_ITEM_MENU_IN_BAG_USE && TRUE)     // Keep item cursor in party after use/give
#define SWSH_ITEM_MENU_IN_BATTLE_USE    (SWSH_ITEM_MENU_IN_BAG_USE && TRUE)     // Use items in bag during battle (skip party menu)
#define SWSH_ITEM_MENU_PARTY_HP_BAR     (SWSH_ITEM_MENU_IN_BAG_USE && TRUE)     // Show HP bar in party slot for certain items usage

#define SWSH_ITEM_MENU_PYRAMID          (SWSH_ITEM_MENU && TRUE)                // Use SwSh bag menu for the Battle Pyramid
#define SWSH_ITEM_MENU_PYRAMID_ACTION   (SWSH_ITEM_MENU_PYRAMID && SWSH_ITEM_MENU_IN_BAG_USE)   // Perform inline Use/Give in the pyramid bag

#define SWSH_ITEM_MENU_BATTLE_POCKETS   (SWSH_ITEM_MENU && TRUE)                // In battle, show battle pockets (Medicine/Poké Balls/Battle Items/Berries) instead of the field pockets

#if SWSH_ITEM_MENU_IN_BAG_USE
void BagMenu_OpenPartySelect(u8 taskId);
#if SWSH_ITEM_MENU_IN_BATTLE_USE
void BagMenu_OpenPartySelectBattle(u8 taskId);
#endif // SWSH_ITEM_MENU_IN_BATTLE_USE
#endif // SWSH_ITEM_MENU_IN_BAG_USE

#endif // GUARD_SWSH_ITEM_MENU_H
