#include "global.h"
#include "test/test.h"
#include "pokemon.h"
#include "caps.h"
#include "item.h"
#include "nuzlocke.h"

// Regression tests for the overhaul's custom candies and the nuzlocke revive
// block. These exist because NuzlockeIsBlockedReviveItem originally detected
// revives via the ITEM4_REVIVE effect bit, which gItemEffect_RareCandy also
// sets - silently disabling every Rare/Exp/Cap/Endless Candy in the game.

static void CreateTestMon(struct Pokemon *mon, u32 level)
{
    CreateMon(mon, SPECIES_TREECKO, level, 32, OTID_STRUCT_PLAYER_ID);
}

TEST("Cap Candy raises a Pokemon straight to the current level cap")
{
    struct Pokemon mon;
    enum GrowthRate rate = gSpeciesInfo[SPECIES_TREECKO].growthRate;
    u32 cap = GetCurrentLevelCap();

    ASSUME(cap > 5);
    CreateTestMon(&mon, 5);
    ExecuteTableBasedItemEffect(&mon, ITEM_CAP_CANDY, 0, 0);

    EXPECT_EQ(GetMonData(&mon, MON_DATA_EXP), gExperienceTables[rate][cap]);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), cap);
}

TEST("Endless Candy raises a Pokemon by exactly one level")
{
    struct Pokemon mon;

    ASSUME(GetCurrentLevelCap() > 6);
    CreateTestMon(&mon, 5);
    ExecuteTableBasedItemEffect(&mon, ITEM_ENDLESS_CANDY, 0, 0);

    EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), 6);
}

TEST("Nuzlocke blocks revives but never candies")
{
    EXPECT(NuzlockeIsBlockedReviveItem(ITEM_REVIVE));
    EXPECT(NuzlockeIsBlockedReviveItem(ITEM_MAX_REVIVE));
    EXPECT(NuzlockeIsBlockedReviveItem(ITEM_REVIVAL_HERB));
    EXPECT(NuzlockeIsBlockedReviveItem(ITEM_SACRED_ASH));

    EXPECT(!NuzlockeIsBlockedReviveItem(ITEM_CAP_CANDY));
    EXPECT(!NuzlockeIsBlockedReviveItem(ITEM_ENDLESS_CANDY));
    EXPECT(!NuzlockeIsBlockedReviveItem(ITEM_RARE_CANDY));
    EXPECT(!NuzlockeIsBlockedReviveItem(ITEM_EXP_CANDY_XL));
    EXPECT(!NuzlockeIsBlockedReviveItem(ITEM_POTION));
}
