// Overhaul: habitat tags for every Hoenn encounter table (ROADMAP 7.12 rule 1).
//
// 116 maps, matching the verified count in docs/overhaul/GYM_CLAIMS.md exactly.
// Kanto/Sevii maps are deliberately absent: this hack never reaches them, and
// GetAreaHabitats() answers HABITAT_NONE for anything not listed.
//
// Two masks per map. `land` also covers rock smash (with HABITAT_CAVE added --
// you are breaking open rock either way), and `water` also covers fishing.
// Masks rather than single habitats because most places genuinely are more than
// one thing: a mountain path is mountain and field, Meteor Falls is cave and
// mountain, and that overlap is what keeps the low-BST candidate pools from
// starving.

struct AreaHabitat
{
    u16 map;        // MAP_* constant: (mapGroup << 8) | mapNum
    u16 land;       // land table, and the base for rock smash
    u16 water;      // water table, and fishing
};

#define SEA_ (HABITAT_SEA_SURFACE | HABITAT_SEA)

static const struct AreaHabitat sAreaHabitats[] =
{
    // --- Gym 1: the starting routes -----------------------------------------
    { MAP_ROUTE101,                     HABITAT_FIELD,                        HABITAT_NONE },
    { MAP_ROUTE102,                     HABITAT_FIELD | HABITAT_FOREST,       HABITAT_SEA_SURFACE },
    { MAP_ROUTE103,                     HABITAT_FIELD,                        SEA_ },
    { MAP_ROUTE104,                     HABITAT_FIELD | HABITAT_FOREST,       SEA_ },
    { MAP_PETALBURG_CITY,               HABITAT_NONE,                         HABITAT_SEA_SURFACE },
    { MAP_PETALBURG_WOODS,              HABITAT_FOREST,                       HABITAT_NONE },
    { MAP_ROUTE116,                     HABITAT_FIELD,                        HABITAT_NONE },
    { MAP_RUSTURF_TUNNEL,               HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_ROUTE115,                     HABITAT_FIELD | HABITAT_MOUNTAIN,     SEA_ },

    // --- Gym 2: Dewford and Granite Cave ------------------------------------
    { MAP_DEWFORD_TOWN,                 HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE106,                     HABITAT_NONE,                         SEA_ },
    { MAP_GRANITE_CAVE_1F,              HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_GRANITE_CAVE_B1F,             HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_GRANITE_CAVE_B2F,             HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_GRANITE_CAVE_STEVENS_ROOM,    HABITAT_CAVE,                         HABITAT_NONE },

    // --- Gym 3: Slateport, Mauville approaches ------------------------------
    { MAP_ROUTE109,                     HABITAT_NONE,                         SEA_ },
    { MAP_SLATEPORT_CITY,               HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE110,                     HABITAT_FIELD,                        SEA_ },
    { MAP_ROUTE117,                     HABITAT_FIELD,                        HABITAT_SEA_SURFACE },
    // Route 111 is three places at once: desert land, a pond, and rock smash.
    { MAP_ROUTE111,                     HABITAT_DESERT,                       HABITAT_SEA_SURFACE },
    { MAP_ROUTE118,                     HABITAT_FIELD,                        SEA_ },

    // --- Gym 4: the volcanic belt -------------------------------------------
    { MAP_ROUTE112,                     HABITAT_MOUNTAIN | HABITAT_FIELD,     HABITAT_NONE },
    { MAP_FIERY_PATH,                   HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_ROUTE113,                     HABITAT_MOUNTAIN | HABITAT_FIELD,     HABITAT_NONE },
    { MAP_ROUTE114,                     HABITAT_MOUNTAIN | HABITAT_FIELD,     HABITAT_SEA_SURFACE },
    { MAP_METEOR_FALLS_1F_1R,           HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_SEA_SURFACE },
    { MAP_JAGGED_PASS,                  HABITAT_MOUNTAIN,                     HABITAT_NONE },

    // --- Gym 5: the desert ---------------------------------------------------
    { MAP_MIRAGE_TOWER_1F,              HABITAT_DESERT | HABITAT_RUINS,       HABITAT_NONE },
    { MAP_MIRAGE_TOWER_2F,              HABITAT_DESERT | HABITAT_RUINS,       HABITAT_NONE },
    { MAP_MIRAGE_TOWER_3F,              HABITAT_DESERT | HABITAT_RUINS,       HABITAT_NONE },
    { MAP_MIRAGE_TOWER_4F,              HABITAT_DESERT | HABITAT_RUINS,       HABITAT_NONE },

    // --- Gym 6: Surf opens the map ------------------------------------------
    { MAP_ROUTE105,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE107,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE108,                     HABITAT_NONE,                         SEA_ },
    { MAP_ABANDONED_SHIP_ROOMS_B1F,     HABITAT_NONE,                         SEA_ | HABITAT_INDUSTRIAL },
    { MAP_NEW_MAUVILLE_ENTRANCE,        HABITAT_INDUSTRIAL | HABITAT_CAVE,    HABITAT_NONE },
    { MAP_NEW_MAUVILLE_INSIDE,          HABITAT_INDUSTRIAL | HABITAT_CAVE,    HABITAT_NONE },
    { MAP_ROUTE119,                     HABITAT_FOREST | HABITAT_FIELD,       HABITAT_SEA_SURFACE },
    { MAP_ROUTE120,                     HABITAT_FOREST | HABITAT_FIELD,       HABITAT_SEA_SURFACE },
    { MAP_ALTERING_CAVE,                HABITAT_CAVE,                         HABITAT_NONE },

    // --- Gym 7: the east and the sea ----------------------------------------
    { MAP_ROUTE121,                     HABITAT_FIELD,                        SEA_ },
    { MAP_ROUTE122,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE123,                     HABITAT_FIELD,                        SEA_ },
    { MAP_LILYCOVE_CITY,                HABITAT_NONE,                         SEA_ },
    { MAP_MT_PYRE_1F,                   HABITAT_RUINS,                        HABITAT_NONE },
    { MAP_MT_PYRE_2F,                   HABITAT_RUINS,                        HABITAT_NONE },
    { MAP_MT_PYRE_3F,                   HABITAT_RUINS,                        HABITAT_NONE },
    { MAP_MT_PYRE_4F,                   HABITAT_RUINS,                        HABITAT_NONE },
    { MAP_MT_PYRE_5F,                   HABITAT_RUINS,                        HABITAT_NONE },
    { MAP_MT_PYRE_6F,                   HABITAT_RUINS,                        HABITAT_NONE },
    { MAP_MT_PYRE_EXTERIOR,             HABITAT_RUINS | HABITAT_MOUNTAIN | HABITAT_FIELD, HABITAT_NONE },
    { MAP_MT_PYRE_SUMMIT,               HABITAT_RUINS | HABITAT_MOUNTAIN,     HABITAT_NONE },
    { MAP_SAFARI_ZONE_SOUTH,            HABITAT_FIELD,                        HABITAT_NONE },
    { MAP_SAFARI_ZONE_NORTH,            HABITAT_FIELD,                        HABITAT_NONE },
    { MAP_SAFARI_ZONE_SOUTHWEST,        HABITAT_FIELD | HABITAT_FOREST,       HABITAT_SEA_SURFACE },
    { MAP_SAFARI_ZONE_NORTHWEST,        HABITAT_FIELD | HABITAT_FOREST,       HABITAT_SEA_SURFACE },
    { MAP_MAGMA_HIDEOUT_1F,             HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_2F_1R,          HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_2F_2R,          HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_2F_3R,          HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_3F_1R,          HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_3F_2R,          HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_3F_3R,          HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_MAGMA_HIDEOUT_4F,             HABITAT_MOUNTAIN | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_ROUTE124,                     HABITAT_NONE,                         SEA_ },
    { MAP_MOSSDEEP_CITY,                HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE125,                     HABITAT_NONE,                         SEA_ },
    { MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM, HABITAT_CAVE,                    HABITAT_SEA_SURFACE },
    { MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM,    HABITAT_CAVE,                    HABITAT_SEA_SURFACE },
    { MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM,   HABITAT_CAVE,                    HABITAT_NONE },
    { MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM,    HABITAT_CAVE,                    HABITAT_NONE },
    { MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM,      HABITAT_ICE | HABITAT_CAVE,      HABITAT_NONE },
    { MAP_ROUTE126,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE127,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE128,                     HABITAT_NONE,                         SEA_ },

    // --- Gym 8: Dive and the far south --------------------------------------
    { MAP_UNDERWATER_ROUTE124,          HABITAT_NONE,                         HABITAT_DEEP_SEA | HABITAT_SEA },
    { MAP_UNDERWATER_ROUTE126,          HABITAT_NONE,                         HABITAT_DEEP_SEA | HABITAT_SEA },
    { MAP_SOOTOPOLIS_CITY,              HABITAT_NONE,                         SEA_ },
    { MAP_CAVE_OF_ORIGIN_ENTRANCE,      HABITAT_CAVE | HABITAT_RUINS,         HABITAT_NONE },
    { MAP_CAVE_OF_ORIGIN_1F,            HABITAT_CAVE | HABITAT_RUINS,         HABITAT_NONE },
    { MAP_SEAFLOOR_CAVERN_ENTRANCE,     HABITAT_NONE,                         SEA_ },
    { MAP_SEAFLOOR_CAVERN_ROOM1,        HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_SEAFLOOR_CAVERN_ROOM2,        HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_SEAFLOOR_CAVERN_ROOM3,        HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_SEAFLOOR_CAVERN_ROOM4,        HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_SEAFLOOR_CAVERN_ROOM5,        HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_SEAFLOOR_CAVERN_ROOM6,        HABITAT_CAVE,                         SEA_ },
    { MAP_SEAFLOOR_CAVERN_ROOM7,        HABITAT_CAVE,                         SEA_ },
    { MAP_SEAFLOOR_CAVERN_ROOM8,        HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS, HABITAT_NONE,                SEA_ | HABITAT_INDUSTRIAL },
    { MAP_SKY_PILLAR_1F,                HABITAT_RUINS | HABITAT_MOUNTAIN,     HABITAT_NONE },
    { MAP_SKY_PILLAR_3F,                HABITAT_RUINS | HABITAT_MOUNTAIN,     HABITAT_NONE },
    { MAP_SKY_PILLAR_5F,                HABITAT_RUINS | HABITAT_MOUNTAIN,     HABITAT_NONE },
    { MAP_ROUTE129,                     HABITAT_NONE,                         SEA_ },
    // Route 130's land table is Mirage Island, excluded from generation
    // (GYM_CLAIMS.md Notes 2) -- tagged anyway so the map is never an orphan.
    { MAP_ROUTE130,                     HABITAT_FIELD,                        SEA_ },
    { MAP_ROUTE131,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE132,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE133,                     HABITAT_NONE,                         SEA_ },
    { MAP_ROUTE134,                     HABITAT_NONE,                         SEA_ },
    { MAP_PACIFIDLOG_TOWN,              HABITAT_NONE,                         SEA_ },

    // --- Elite Four route ----------------------------------------------------
    { MAP_EVER_GRANDE_CITY,             HABITAT_NONE,                         SEA_ },
    { MAP_VICTORY_ROAD_1F,              HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_NONE },
    { MAP_VICTORY_ROAD_B1F,             HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_NONE },
    { MAP_VICTORY_ROAD_B2F,             HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_SEA_SURFACE },
    { MAP_METEOR_FALLS_1F_2R,           HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_SEA_SURFACE },
    { MAP_METEOR_FALLS_B1F_1R,          HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_SEA_SURFACE },
    { MAP_METEOR_FALLS_B1F_2R,          HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_SEA_SURFACE },

    // --- Post-game -----------------------------------------------------------
    { MAP_SAFARI_ZONE_SOUTHEAST,        HABITAT_FIELD | HABITAT_FOREST,       HABITAT_SEA_SURFACE },
    { MAP_SAFARI_ZONE_NORTHEAST,        HABITAT_FIELD,                        HABITAT_NONE },
    { MAP_METEOR_FALLS_STEVENS_CAVE,    HABITAT_CAVE | HABITAT_MOUNTAIN,      HABITAT_NONE },
    { MAP_DESERT_UNDERPASS,             HABITAT_CAVE | HABITAT_DESERT,        HABITAT_NONE },
    { MAP_ARTISAN_CAVE_1F,              HABITAT_CAVE,                         HABITAT_NONE },
    { MAP_ARTISAN_CAVE_B1F,             HABITAT_CAVE,                         HABITAT_NONE },

    // --- Unreachable, kept so the map count reconciles with GYM_CLAIMS.md ----
    { MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1, HABITAT_CAVE,             HABITAT_NONE },
    { MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2, HABITAT_CAVE,             HABITAT_NONE },
    { MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3, HABITAT_CAVE,             HABITAT_NONE },
};

#undef SEA_
