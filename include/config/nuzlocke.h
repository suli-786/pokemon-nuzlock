#ifndef GUARD_CONFIG_NUZLOCKE_H
#define GUARD_CONFIG_NUZLOCKE_H

#include "constants/region_map_sections.h"

// Nuzlocke rules engine (Overhaul Phase 3 — see docs/overhaul/ROADMAP.md 7.3).
//
// Everything here defaults ON: this is the owner's game, not an optional mode.
// The individual clause toggles exist so a future settings menu (Phase 5) can
// flip them per run without touching the engine.

// Master switch. FALSE compiles the whole engine out, including its SaveBlock3
// storage, and the game behaves exactly like stock pokeemerald-expansion.
#define NUZLOCKE_ENABLED                    TRUE

// Rule 2: an encounter whose species (or any member of its evolution family) is
// already owned is a DUPE — uncatchable, and it does NOT consume the route slot.
#define NUZLOCKE_DUPES_CLAUSE               TRUE

// Whether Pokémon sitting in the graveyard box count as "owned" for the dupes
// clause. TRUE is the literal reading of the rule ("check party AND all PC
// boxes"): a species that already died cannot be re-caught elsewhere.
#define NUZLOCKE_DUPES_COUNT_GRAVEYARD      TRUE

// Rule 3: shiny encounters are always catchable and never consume a route slot.
#define NUZLOCKE_SHINY_CLAUSE               TRUE

// Rule 7: fainted party Pokémon are dead — swept into the graveyard box at the
// end of the battle and made permanently unusable.
#define NUZLOCKE_PERMADEATH                 TRUE

// Rule 8: losing every party Pokémon ends the run. Storage does not rescue it.
#define NUZLOCKE_GAME_OVER_ON_WIPE          TRUE

// Rule 9: Revive / Max Revive / Revival Herb / Sacred Ash are inert for the
// player. (Opposing trainers' AI item use is unaffected.)
#define NUZLOCKE_BLOCK_REVIVES              TRUE

// PC box the dead are moved to. Must be the last box: the engine also uses it
// as the "never auto-send catches here" box. Checked with a STATIC_ASSERT in
// src/nuzlocke.c against TOTAL_BOXES_COUNT.
#define NUZLOCKE_GRAVEYARD_BOX              13

// Box names are capped at BOX_NAME_LENGTH (8) characters, so the full word
// "GRAVEYARD" (9) does not fit. Change freely, keep it <= 8 characters.
#define NUZLOCKE_GRAVEYARD_BOX_NAME         "GRAVE"

// Largest evolution family the dupes clause can hold at once (Eevee is 9).
#define NUZLOCKE_MAX_FAMILY_SIZE            16

// How deep to walk backwards looking for the base form of a species.
#define NUZLOCKE_MAX_PREEVO_DEPTH           5

// --- derived, do not edit ---------------------------------------------------

// Two bits of route state per MAPSEC: unused / caught / killed / fled.
#define NUZLOCKE_ROUTES_PER_BYTE            4
#define NUZLOCKE_ROUTE_BYTES                ((MAPSEC_COUNT + NUZLOCKE_ROUTES_PER_BYTE - 1) / NUZLOCKE_ROUTES_PER_BYTE)

#endif // GUARD_CONFIG_NUZLOCKE_H
