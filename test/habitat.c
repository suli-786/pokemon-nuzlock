#include "global.h"
#include "test/test.h"
#include "habitat.h"
#include "wild_encounter.h"
#include "constants/habitat.h"
#include "constants/maps.h"

// Overhaul Phase 4 step 1: guards for the habitat layer (ROADMAP 7.12 rule 1).
//
// The "completeness" guarantee the owner asked for is the first test: a species
// with no habitat can never be picked for any slot, so it would silently vanish
// from the randomizer. Failing loudly here is the whole point.

TEST("Habitat: every species has at least one habitat")
{
    u32 species;
    u32 orphans = 0;

    for (species = 1; species < NUM_SPECIES; species++)
    {
        // Mirrors the randomizer's own IsSpeciesPermitted: a zeroed stat block
        // is how a disabled species is spelled.
        if (gSpeciesInfo[species].baseHP == 0)
            continue;

        if (GetSpeciesHabitats(species) == HABITAT_NONE)
            orphans++;
    }

    EXPECT_EQ(orphans, 0);
}

TEST("Habitat: aquatic species are never land-only, and land species are never sea-only")
{
    // Two species whose classification the encounter generator leans on hardest.
    EXPECT(GetSpeciesHabitats(SPECIES_TENTACOOL) & HABITAT_ANY_WATER);
    EXPECT(!(GetSpeciesHabitats(SPECIES_TENTACOOL) & HABITAT_DESERT));
    EXPECT(GetSpeciesHabitats(SPECIES_ZIGZAGOON) & HABITAT_FIELD);
    EXPECT(!(GetSpeciesHabitats(SPECIES_ZIGZAGOON) & HABITAT_ANY_WATER));
    EXPECT(GetSpeciesHabitats(SPECIES_ZUBAT) & HABITAT_CAVE);
    EXPECT(GetSpeciesHabitats(SPECIES_SNORUNT) & HABITAT_ICE);
}

TEST("Habitat: tagged maps answer for the areas they actually have")
{
    u32 i;
    u32 untagged = 0;

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        u8 group = gWildMonHeaders[i].mapGroup;
        u8 num = gWildMonHeaders[i].mapNum;
        const struct WildEncounterTypes *t = &gWildMonHeaders[i].encounterTypes[TIME_MORNING];

        u32 land = GetAreaHabitats(group, num, WILD_AREA_LAND);
        u32 water = GetAreaHabitats(group, num, WILD_AREA_WATER);
        u32 fishing = GetAreaHabitats(group, num, WILD_AREA_FISHING);

        // A map we do not cover at all (Kanto/Sevii) answers zero everywhere;
        // that is expected and not a failure. A map we DO cover must answer for
        // every area it actually ships a table for -- otherwise a real Hoenn
        // table would fall through to no candidates.
        if ((land | water | fishing) == HABITAT_NONE)
            continue;

        if (t->landMonsInfo != NULL && land == HABITAT_NONE)
            untagged++;
        if (t->waterMonsInfo != NULL && water == HABITAT_NONE)
            untagged++;
        if (t->fishingMonsInfo != NULL && fishing == HABITAT_NONE)
            untagged++;
    }

    EXPECT_EQ(untagged, 0);
}

TEST("Habitat: no species is missing the habitat Emerald actually puts it in")
{
    // Re-derives layer 1 straight from the live encounter tables and checks
    // GetSpeciesHabitats() already covers it. This is the drift guard for the
    // generated src/data/habitats_vanilla.h: edit wild_encounters.json or the
    // map tags without regenerating, and this fails instead of quietly leaving
    // a species unable to appear in its own home.
    u32 i, slot, missing = 0;

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        u8 group = gWildMonHeaders[i].mapGroup;
        u8 num = gWildMonHeaders[i].mapNum;
        u32 t;

        for (t = 0; t < TIMES_OF_DAY_COUNT; t++)
        {
            const struct WildEncounterTypes *types = &gWildMonHeaders[i].encounterTypes[t];
            const struct WildPokemonInfo *tables[4] = {
                types->landMonsInfo, types->waterMonsInfo,
                types->rockSmashMonsInfo, types->fishingMonsInfo,
            };
            const enum WildPokemonArea areas[4] = {
                WILD_AREA_LAND, WILD_AREA_WATER, WILD_AREA_ROCKS, WILD_AREA_FISHING,
            };
            u32 a;

            for (a = 0; a < 4; a++)
            {
                u32 want = GetAreaHabitats(group, num, areas[a]);
                u32 count;

                if (tables[a] == NULL || want == HABITAT_NONE)
                    continue;   // untagged map (Kanto/Sevii), or no such table

                count = (areas[a] == WILD_AREA_LAND) ? LAND_WILD_COUNT
                      : (areas[a] == WILD_AREA_WATER) ? WATER_WILD_COUNT
                      : (areas[a] == WILD_AREA_ROCKS) ? ROCK_WILD_COUNT
                      : FISH_WILD_COUNT;

                for (slot = 0; slot < count; slot++)
                {
                    u16 species = tables[a]->wildPokemon[slot].species;
                    if (species == SPECIES_NONE)
                        continue;
                    if ((GetSpeciesHabitats(species) & want) == 0)
                        missing++;
                }
            }
        }
    }

    EXPECT_EQ(missing, 0);
}

TEST("Habitat: map tags read back as authored")
{
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101),
                           WILD_AREA_LAND) == HABITAT_FIELD);
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_GRANITE_CAVE_1F), MAP_NUM(MAP_GRANITE_CAVE_1F),
                           WILD_AREA_LAND) == HABITAT_CAVE);
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_UNDERWATER_ROUTE124), MAP_NUM(MAP_UNDERWATER_ROUTE124),
                           WILD_AREA_WATER) & HABITAT_DEEP_SEA);
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM),
                           MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM),
                           WILD_AREA_LAND) & HABITAT_ICE);
    // Rock smash adds cave wherever it is used, including out in the open.
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_ROUTE111), MAP_NUM(MAP_ROUTE111),
                           WILD_AREA_ROCKS) & HABITAT_CAVE);
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_ROUTE111), MAP_NUM(MAP_ROUTE111),
                           WILD_AREA_LAND) == HABITAT_DESERT);
    // A map this hack never reaches is deliberately absent.
    EXPECT(GetAreaHabitats(MAP_GROUP(MAP_VIRIDIAN_FOREST), MAP_NUM(MAP_VIRIDIAN_FOREST),
                           WILD_AREA_LAND) == HABITAT_NONE);
}
