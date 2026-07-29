#ifndef GUARD_HABITAT_H
#define GUARD_HABITAT_H

#include "constants/habitat.h"
#include "wild_encounter.h"

// Overhaul: habitat layer for the randomizer (ROADMAP 7.12, Phase 4 step 1).
//
// Both sides answer in HABITAT_* bit sets, and a candidate species is legal for
// a slot when GetSpeciesHabitats() & GetAreaHabitats() is non-zero. Nothing here
// changes encounters on its own -- step 2's table-level generator is what
// consumes it.

// The habitats a species can be found in. Never returns HABITAT_NONE for a
// species the randomizer can pick; test/habitat.c enforces that.
u32 GetSpeciesHabitats(u16 species);

// The habitats an encounter table accepts. HABITAT_NONE means the map is not in
// the table -- Kanto/Sevii maps, which this hack never reaches.
u32 GetAreaHabitats(u8 mapGroup, u8 mapNum, enum WildPokemonArea area);

#endif // GUARD_HABITAT_H
