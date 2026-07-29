#ifndef GUARD_SWSH_PARTY_MENU_H
#define GUARD_SWSH_PARTY_MENU_H

#include "config/swsh_ui.h"

// The master toggle SWSH_PARTY_MENU lives in include/config/swsh_ui.h to match
// this repo's config convention -- see docs/overhaul/UI_PORT_CHECKLIST.md.
// Everything below is this screen's own look-and-feel tuning and stays here.
//
// NOTE: src/swsh_party_menu.c includes THIS header directly. Upstream reached
// these toggles through constants/party_menu.h -> config/swsh_party_menu.h,
// which we deliberately do not carry; without the direct include the #if
// SWSH_PARTY_MENU_PC_ACCESS blocks would silently evaluate to 0.

#define SWSH_PARTY_MENU_PC_ACCESS         TRUE
#define SWSH_PARTY_MON_IDLE_ANIMS         TRUE
#define SWSH_PARTY_MON_IDLE_ANIMS_FRAMES  300 // Number of frames before mon animation loops

#endif // GUARD_SWSH_PARTY_MENU_H
