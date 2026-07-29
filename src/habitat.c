#include "global.h"
#include "habitat.h"
#include "pokemon.h"
#include "constants/habitat.h"
#include "constants/maps.h"
#include "constants/pokemon.h"
#include "data/habitats.h"
#include "data/habitats_vanilla.h"

// Overhaul: habitat layer for the randomizer's species distribution
// (ROADMAP 7.12 rule 1, Phase 4 step 1).
//
// Three layers, unioned, per ROADMAP 7.12 rule 1:
//
//   1. Where Emerald itself puts the species (src/data/habitats_vanilla.h).
//   2. Written type and egg-group rules (below).
//   3. A manual override list, for the handful layers 1 and 2 both miss.
//
// The spec named "official Gen1-3 habitat data" for layer 1, but the expansion
// carries no habitat field -- FRLG's Pokedex habitat covers Kanto's 151 only and
// is not in this repo. The game's own encounter tables are a strictly better
// source: in-repo, self-consistent with the very maps being tagged, and
// authoritative about the species the game was balanced around.
//
// Layer 1 is not a nicety. The type rules are geological -- Rock, Ground, Steel,
// Mineral -- so on their own they place Zubat on mountains, Whismur in fields
// and Makuhita in fields, when all three are Granite Cave and Rusturf Tunnel
// residents. Emerald knows where they live; we should ask it rather than guess.
//
// Layer 2 is DERIVED rather than tabulated: run over the 1028 species the
// randomizer can pick, ~20 rules classify 1025 of them, so a hand-authored table
// would be 1028 rows restating what the rules already say, and would rot the
// moment a species' typing changed.
//
// A species belongs to several habitats on purpose. Restricting each to one
// would starve the early game: measured over the real Hoenn tables, a cave slot
// whose vanilla occupant sits near 220 BST has only a handful of single-habitat
// candidates, because weak Pokemon are overwhelmingly field and forest animals
// while cave, ice and desert species skew strong.

// docs/overhaul/GYM_CLAIMS.md verified Hoenn's encounter maps at exactly 116.
// Pin it here: an untagged map answers HABITAT_NONE for every area, which is
// indistinguishable from a Kanto map the tests deliberately skip, so a forgotten
// tag would silently leave a whole route unable to generate anything. A compile
// error is the only way that mistake gets noticed.
STATIC_ASSERT(ARRAY_COUNT(sAreaHabitats) == 116, HoennEncounterMapCountChanged);

struct HabitatOverride
{
    u16 species;
    u16 habitats;
};

// Layer 3. Species that layers 1 and 2 between them still leave unplaced.
// Kept deliberately tiny: if this list starts growing, the rules are wrong and
// should be fixed instead of papered over one species at a time.
// test/habitat.c fails if any randomizable species ends up with no habitat.
static const struct HabitatOverride sHabitatOverrides[] =
{
    // Baby Pokemon are EGG_GROUP_NO_EGGS_DISCOVERED and so match no egg-group
    // rule; Togepi additionally has a config-dependent primary type.
    { SPECIES_CLEFFA,     HABITAT_FIELD | HABITAT_MOUNTAIN },
    { SPECIES_IGGLYBUFF,  HABITAT_FIELD },
    { SPECIES_TOGEPI,     HABITAT_FIELD | HABITAT_MOUNTAIN },
};

static bool32 HasType(const struct SpeciesInfo *info, u32 type)
{
    return info->types[0] == type || info->types[1] == type;
}

static bool32 HasEggGroup(const struct SpeciesInfo *info, u32 group)
{
    return info->eggGroups[0] == group || info->eggGroups[1] == group;
}

u32 GetSpeciesHabitats(u16 species)
{
    const struct SpeciesInfo *info;
    u32 h = HABITAT_NONE;
    bool32 aquatic;
    u32 i;

    if (species >= NUM_SPECIES)
        return HABITAT_NONE;

    // Layer 1: wherever Emerald itself places this species.
    for (i = 0; i < ARRAY_COUNT(sVanillaHabitats); i++)
    {
        if (sVanillaHabitats[i].species == species)
        {
            h |= sVanillaHabitats[i].habitats;
            break;
        }
    }

    info = &gSpeciesInfo[species];

    aquatic = HasType(info, TYPE_WATER)
           || HasEggGroup(info, EGG_GROUP_WATER_1)
           || HasEggGroup(info, EGG_GROUP_WATER_2)
           || HasEggGroup(info, EGG_GROUP_WATER_3);

    if (aquatic)
    {
        h |= HABITAT_SEA_SURFACE | HABITAT_SEA;
        // Water 3 is the shellfish/bottom-dweller group; armoured and venomous
        // swimmers likewise read as depth rather than surface.
        if (HasEggGroup(info, EGG_GROUP_WATER_3)
         || HasType(info, TYPE_ROCK) || HasType(info, TYPE_STEEL)
         || HasType(info, TYPE_DARK) || HasType(info, TYPE_POISON))
            h |= HABITAT_DEEP_SEA;
    }

    if (HasType(info, TYPE_BUG) || HasType(info, TYPE_GRASS)
     || HasEggGroup(info, EGG_GROUP_BUG) || HasEggGroup(info, EGG_GROUP_GRASS))
        h |= HABITAT_FOREST;

    if (HasType(info, TYPE_ROCK) || HasType(info, TYPE_GROUND)
     || HasType(info, TYPE_STEEL) || HasEggGroup(info, EGG_GROUP_MINERAL))
        h |= HABITAT_CAVE;

    if (HasType(info, TYPE_ROCK) || HasType(info, TYPE_FLYING)
     || HasType(info, TYPE_DRAGON) || HasType(info, TYPE_FIRE))
        h |= HABITAT_MOUNTAIN;

    if (HasType(info, TYPE_GROUND) && !aquatic)
        h |= HABITAT_DESERT;

    if (HasType(info, TYPE_GHOST) || HasEggGroup(info, EGG_GROUP_AMORPHOUS))
        h |= HABITAT_RUINS;

    if (HasType(info, TYPE_PSYCHIC) && !aquatic)
        h |= HABITAT_RUINS;

    if (HasType(info, TYPE_STEEL) || HasType(info, TYPE_ELECTRIC)
     || HasEggGroup(info, EGG_GROUP_MINERAL))
        h |= HABITAT_INDUSTRIAL;

    if (HasType(info, TYPE_ICE))
        h |= HABITAT_ICE;

    if (HasType(info, TYPE_NORMAL) || HasType(info, TYPE_FAIRY)
     || HasType(info, TYPE_FIGHTING) || HasType(info, TYPE_DARK)
     || HasType(info, TYPE_POISON)
     || HasEggGroup(info, EGG_GROUP_FIELD) || HasEggGroup(info, EGG_GROUP_HUMAN_LIKE)
     || HasEggGroup(info, EGG_GROUP_FAIRY) || HasEggGroup(info, EGG_GROUP_MONSTER)
     || HasEggGroup(info, EGG_GROUP_FLYING))
        h |= HABITAT_FIELD;

    // Layer 3: manual additions, applied last. These ADD habitats rather than
    // replace, so an override can never accidentally mask what layers 1 and 2
    // already worked out correctly.
    for (i = 0; i < ARRAY_COUNT(sHabitatOverrides); i++)
    {
        if (sHabitatOverrides[i].species == species)
        {
            h |= sHabitatOverrides[i].habitats;
            break;
        }
    }

    return h;
}

u32 GetAreaHabitats(u8 mapGroup, u8 mapNum, enum WildPokemonArea area)
{
    u16 map = (mapGroup << 8) | mapNum;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sAreaHabitats); i++)
    {
        if (sAreaHabitats[i].map != map)
            continue;

        switch (area)
        {
        case WILD_AREA_LAND:
        case WILD_AREA_HIDDEN:
            return sAreaHabitats[i].land;
        case WILD_AREA_WATER:
        case WILD_AREA_FISHING:
            return sAreaHabitats[i].water;
        case WILD_AREA_ROCKS:
            // Smashing rock exposes what lives inside it, wherever the map is.
            return sAreaHabitats[i].land | HABITAT_CAVE;
        }
        return HABITAT_NONE;
    }

    return HABITAT_NONE;
}
