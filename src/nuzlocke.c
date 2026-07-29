// Nuzlocke rules engine — Overhaul Phase 3.
//
// The nine rules, and where each one lives:
//
//  1. Route key = gMapHeader.regionMapSectionId (the location banner).
//       -> NuzlockeGetCurrentMapSec / the 2-bit-per-MAPSEC table in SaveBlock3.
//  2. Dupes clause: species (or anything in its evolution family) already owned
//     => uncatchable, slot-neutral.
//       -> NuzlockeIsSpeciesOwned + NuzlockeClassifyWildEncounter, enforced in
//          GetBallThrowableState (src/item_use.c).
//  3. Shiny clause: always catchable, never consumes the slot.
//       -> NuzlockeClassifyWildEncounter.
//  4. The first legal encounter consumes the slot win or lose, and the outcome
//     is recorded.
//       -> NuzlockeOnBattleEnd, via gBattleOutcome.
//  5. A used route has no wild encounters at all, ever.
//       -> NuzlockeSuppressWildEncounters, called from the encounter pipeline
//          in src/wild_encounter.c. Deliberately NOT the WE_FLAG_NO_ENCOUNTER
//          flag: that flag is the player's manual Repellant toggle.
//  6. Statics/gifts/eggs are free: they never reach CreateWildMon, so they are
//     never classified and never consume anything.
//  7. Permadeath: fainted party Pokémon are swept into the graveyard box and
//     made unusable.
//       -> NuzlockeSweepFaintedParty + the NuzlockeIsMonDead gates.
//  8. Party wipe = game over.
//       -> NuzlockeOnBattleEnd sets NUZLOCKE_RUN_FAILED; CB2_WhiteOut hands off
//          to NuzlockeTryStartRunOverScreen.
//  9. Revives cannot resurrect the dead.
//       -> NuzlockeIsBlockedReviveItem, gated in PokemonUseItemEffects.
//
// Ordering note: the randomizer substitutes the wild species inside
// TryGenerateWildMon/GenerateFishingWildMon *before* they call CreateWildMon,
// so classifying at the end of CreateWildMon always sees the final species.

#include "global.h"
#include "nuzlocke.h"

#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "bg.h"
#include "event_data.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "safari_zone.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "title_screen.h"
#include "trainer_hill.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#if NUZLOCKE_ENABLED == TRUE

STATIC_ASSERT(NUZLOCKE_GRAVEYARD_BOX == TOTAL_BOXES_COUNT - 1, NuzlockeGraveyardMustBeLastBox);
STATIC_ASSERT(sizeof(NUZLOCKE_GRAVEYARD_BOX_NAME) <= BOX_NAME_LENGTH + 1, NuzlockeGraveyardNameTooLong);

// State for the encounter currently on screen. Rebuilt every CreateWildMon and
// consumed at battle end, so it never needs to survive a save.
static EWRAM_DATA u8 sEncounterClass = 0;   // enum NuzlockeEncounterClass
static EWRAM_DATA u8 sPendingMapSec = 0;    // route waiting to be consumed; 0 => none
static EWRAM_DATA bool8 sClassificationSuspended = 0; // set while a throwaway mon is generated

// sPendingMapSec is 0-initialised, and MAPSEC_LITTLEROOT_TOWN is 0, so the
// table is offset by one: 0 means "nothing pending".
#define NO_PENDING_ROUTE            0
#define PENDING_FROM_MAPSEC(m)      ((m) + 1)
#define MAPSEC_FROM_PENDING(p)      ((p) - 1)

// ---------------------------------------------------------------------------
// Route table (2 bits per MAPSEC, SaveBlock3)
// ---------------------------------------------------------------------------

bool32 NuzlockeIsActive(void)
{
    return TRUE;
}

u32 NuzlockeGetCurrentMapSec(void)
{
    return gMapHeader.regionMapSectionId;
}

// MAPSEC_NONE and MAPSEC_DYNAMIC have no meaningful location banner, so they
// get no route slot: they are treated as permanently untracked.
bool32 NuzlockeIsTrackedMapSec(u32 mapSec)
{
    return (mapSec < MAPSEC_COUNT && mapSec != MAPSEC_NONE && mapSec != MAPSEC_DYNAMIC);
}

u32 NuzlockeGetRouteState(u32 mapSec)
{
    if (!NuzlockeIsTrackedMapSec(mapSec))
        return NUZLOCKE_ROUTE_UNUSED;

    return (gSaveBlock3Ptr->nuzlockeRoutes[mapSec / NUZLOCKE_ROUTES_PER_BYTE]
            >> ((mapSec % NUZLOCKE_ROUTES_PER_BYTE) * 2)) & 3;
}

void NuzlockeSetRouteState(u32 mapSec, u32 state)
{
    u32 shift;
    u8 *byte;

    if (!NuzlockeIsTrackedMapSec(mapSec))
        return;

    byte = &gSaveBlock3Ptr->nuzlockeRoutes[mapSec / NUZLOCKE_ROUTES_PER_BYTE];
    shift = (mapSec % NUZLOCKE_ROUTES_PER_BYTE) * 2;
    *byte = (*byte & ~(3 << shift)) | ((state & 3) << shift);
}

bool32 NuzlockeIsRouteUsed(u32 mapSec)
{
    return NuzlockeGetRouteState(mapSec) != NUZLOCKE_ROUTE_UNUSED;
}

bool32 NuzlockeIsCurrentRouteUsed(void)
{
    return NuzlockeIsRouteUsed(NuzlockeGetCurrentMapSec());
}

u32 NuzlockeCountRoutesInState(u32 state)
{
    u32 mapSec, count = 0;

    for (mapSec = 0; mapSec < MAPSEC_COUNT; mapSec++)
    {
        if (NuzlockeIsTrackedMapSec(mapSec) && NuzlockeGetRouteState(mapSec) == state)
            count++;
    }
    return count;
}

u32 NuzlockeGetCatchCount(void)
{
    return gSaveBlock3Ptr->nuzlockeCatches;
}

u32 NuzlockeGetDeathCount(void)
{
    return gSaveBlock3Ptr->nuzlockeDeaths;
}

bool32 NuzlockeIsRunFailed(void)
{
    return (gSaveBlock3Ptr->nuzlockeRunState & NUZLOCKE_RUN_FAILED) != 0;
}

void NuzlockeSetRunFailed(void)
{
    gSaveBlock3Ptr->nuzlockeRunState |= NUZLOCKE_RUN_FAILED;
}

void NuzlockeClearRunFailed(void)
{
    gSaveBlock3Ptr->nuzlockeRunState &= ~NUZLOCKE_RUN_FAILED;
}

u32 NuzlockeGetGraveyardBoxId(void)
{
    return NUZLOCKE_GRAVEYARD_BOX;
}

bool32 NuzlockeIsGraveyardBox(u32 boxId)
{
#if NUZLOCKE_PERMADEATH == TRUE
    return boxId == NUZLOCKE_GRAVEYARD_BOX;
#else
    return FALSE;
#endif
}

void NuzlockeSetUpGraveyardBox(void)
{
    u8 *name = GetBoxNamePtr(NUZLOCKE_GRAVEYARD_BOX);

    if (name != NULL)
        StringCopy(name, COMPOUND_STRING(NUZLOCKE_GRAVEYARD_BOX_NAME));
}

void NuzlockeInitNewRun(void)
{
    memset(gSaveBlock3Ptr->nuzlockeRoutes, 0, sizeof(gSaveBlock3Ptr->nuzlockeRoutes));
    gSaveBlock3Ptr->nuzlockeRunState = NUZLOCKE_RUN_STARTED;
    gSaveBlock3Ptr->nuzlockeCatches = 0;
    gSaveBlock3Ptr->nuzlockeDeaths = 0;
    sEncounterClass = NUZLOCKE_ENCOUNTER_NONE;
    sPendingMapSec = NO_PENDING_ROUTE;
    NuzlockeSetUpGraveyardBox();
}

// ---------------------------------------------------------------------------
// Rule 2: dupes clause
// ---------------------------------------------------------------------------

#if NUZLOCKE_DUPES_CLAUSE == TRUE

static bool32 FamilyContains(const u16 *family, u32 count, u32 species)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (family[i] == species)
            return TRUE;
    }
    return FALSE;
}

// Walk back to the base form, then breadth-first forward over every evolution
// branch. The result is the whole family, so siblings (Vaporeon vs Jolteon)
// count as dupes of each other, which is the usual reading of the clause.
//
// GetSpeciesPreEvolution is a full species-table scan, so it is only ever run
// on the single encountered species (a handful of calls), never per owned mon.
static u32 BuildEvolutionFamily(u32 species, u16 *family, u32 max)
{
    u32 i, count, depth;
    enum Species base = SanitizeSpeciesId(species);

    for (depth = 0; depth < NUZLOCKE_MAX_PREEVO_DEPTH; depth++)
    {
        enum Species pre = GetSpeciesPreEvolution(base);
        if (pre == SPECIES_NONE)
            break;
        base = pre;
    }

    count = 0;
    family[count++] = base;

    for (i = 0; i < count; i++)
    {
        const struct Evolution *evolutions = GetSpeciesEvolutions(family[i]);
        u32 j;

        if (evolutions == NULL)
            continue;

        for (j = 0; evolutions[j].method != EVOLUTIONS_END; j++)
        {
            enum Species target = SanitizeSpeciesId(evolutions[j].targetSpecies);

            if (target == SPECIES_NONE || !IsSpeciesEnabled(target))
                continue;
            if (FamilyContains(family, count, target))
                continue;
            if (count >= max)
                return count;
            family[count++] = target;
        }
    }

    return count;
}

bool32 NuzlockeIsSpeciesOwned(u32 species)
{
    u16 family[NUZLOCKE_MAX_FAMILY_SIZE];
    u32 count, i, boxId, boxPos;

    if (species == SPECIES_NONE)
        return FALSE;

    count = BuildEvolutionFamily(species, family, ARRAY_COUNT(family));

    // MON_DATA_SPECIES_OR_EGG rather than SPECIES + IS_EGG: reading an
    // encrypted field decrypts and re-encrypts the whole mon, and this runs
    // over every box, so it is worth halving the number of reads.
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 owned;

        if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
            continue;
        owned = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        if (owned == SPECIES_NONE || owned == SPECIES_EGG)
            continue;
        if (FamilyContains(family, count, owned))
            return TRUE;
    }

    for (boxId = 0; boxId < TOTAL_BOXES_COUNT; boxId++)
    {
    #if NUZLOCKE_DUPES_COUNT_GRAVEYARD == FALSE
        if (NuzlockeIsGraveyardBox(boxId))
            continue;
    #endif
        for (boxPos = 0; boxPos < IN_BOX_COUNT; boxPos++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(boxId, boxPos);
            u32 owned;

            if (!GetBoxMonData(boxMon, MON_DATA_SANITY_HAS_SPECIES))
                continue;
            owned = GetBoxMonData(boxMon, MON_DATA_SPECIES_OR_EGG);
            if (owned == SPECIES_NONE || owned == SPECIES_EGG)
                continue;
            if (FamilyContains(family, count, owned))
                return TRUE;
        }
    }

    return FALSE;
}

#else // NUZLOCKE_DUPES_CLAUSE

bool32 NuzlockeIsSpeciesOwned(u32 species)
{
    return FALSE;
}

#endif // NUZLOCKE_DUPES_CLAUSE

// ---------------------------------------------------------------------------
// Rules 2-5: encounter classification and suppression
// ---------------------------------------------------------------------------

// Battles whose Pokémon are not "the route's wild encounter": link, frontier
// facilities, the Safari Zone, the catch tutorial and Birch's first battle.
// Only valid DURING a battle — gBattleTypeFlags still holds the previous
// battle's value while the player is walking around.
static bool32 IsUntrackedBattleType(void)
{
    return (gBattleTypeFlags & (BATTLE_TYPE_LINK
                              | BATTLE_TYPE_RECORDED_LINK
                              | BATTLE_TYPE_FRONTIER
                              | BATTLE_TYPE_TRAINER_HILL
                              | BATTLE_TYPE_SAFARI
                              | BATTLE_TYPE_EREADER_TRAINER
                              | BATTLE_TYPE_CATCH_TUTORIAL
                              | BATTLE_TYPE_FIRST_BATTLE)) != 0;
}

// The same idea, but from field state, so it is safe to call before
// gBattleTypeFlags has been set for the battle that is about to start.
static bool32 IsUntrackedLocation(void)
{
    return (GetSafariZoneFlag()
         || InBattlePike()
         || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE
         || InTrainerHillChallenge());
}

bool32 NuzlockeSuppressWildEncounters(void)
{
    if (IsUntrackedLocation())
        return FALSE;
    if (!NuzlockeIsTrackedMapSec(NuzlockeGetCurrentMapSec()))
        return FALSE;

    return NuzlockeIsCurrentRouteUsed();
}

void NuzlockeClassifyWildEncounter(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 mapSec = NuzlockeGetCurrentMapSec();
    u32 species;

    if (sClassificationSuspended)
        return;

    // A fresh generation fully replaces the previous verdict, so anything left
    // pending from an encounter that never became a battle is dropped here.
    sEncounterClass = NUZLOCKE_ENCOUNTER_NONE;
    sPendingMapSec = NO_PENDING_ROUTE;

    if (IsUntrackedLocation() || !NuzlockeIsTrackedMapSec(mapSec))
        return;
    // Rule 5 should already have suppressed this, but never burn a slot twice.
    if (NuzlockeIsRouteUsed(mapSec))
        return;

    species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return;

#if NUZLOCKE_SHINY_CLAUSE == TRUE
    // Rule 3 outranks rule 2: a shiny dupe is still catchable.
    if (IsMonShiny(mon))
    {
        sEncounterClass = NUZLOCKE_ENCOUNTER_SHINY;
        return;
    }
#endif

#if NUZLOCKE_DUPES_CLAUSE == TRUE
    if (NuzlockeIsSpeciesOwned(species))
    {
        sEncounterClass = NUZLOCKE_ENCOUNTER_DUPE;
        return;
    }
#endif

    sEncounterClass = NUZLOCKE_ENCOUNTER_LEGAL;
    sPendingMapSec = PENDING_FROM_MAPSEC(mapSec);
}

// DexNav builds a throwaway wild mon purely to read a moveset off it, long
// before any battle exists. That must not classify anything or leave a route
// pending, so the generator is wrapped in this suspend/resume pair.
void NuzlockeSuspendClassification(bool32 suspend)
{
    sClassificationSuspended = suspend;
}

u32 NuzlockeGetEncounterClass(void)
{
    return sEncounterClass;
}

bool32 NuzlockeIsEncounterUncatchable(void)
{
    return sEncounterClass == NUZLOCKE_ENCOUNTER_DUPE;
}

void NuzlockeOnMonCaught(void)
{
    u32 mapSec;

    if (sPendingMapSec == NO_PENDING_ROUTE)
        return;

    mapSec = MAPSEC_FROM_PENDING(sPendingMapSec);
    if (!NuzlockeIsRouteUsed(mapSec))
    {
        NuzlockeSetRouteState(mapSec, NUZLOCKE_ROUTE_CAUGHT);
        if (gSaveBlock3Ptr->nuzlockeCatches < 255)
            gSaveBlock3Ptr->nuzlockeCatches++;
    }
    sPendingMapSec = NO_PENDING_ROUTE;
}

// ---------------------------------------------------------------------------
// Rule 7: permadeath
// ---------------------------------------------------------------------------

bool32 NuzlockeIsMonDead(struct Pokemon *mon)
{
#if NUZLOCKE_PERMADEATH == TRUE
    if (mon == NULL || !GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
        return FALSE;
    return GetMonData(mon, MON_DATA_IS_DEAD) != 0;
#else
    return FALSE;
#endif
}

bool32 NuzlockeIsBoxMonDead(struct BoxPokemon *boxMon)
{
#if NUZLOCKE_PERMADEATH == TRUE
    if (boxMon == NULL || !GetBoxMonData(boxMon, MON_DATA_SANITY_HAS_SPECIES))
        return FALSE;
    return GetBoxMonData(boxMon, MON_DATA_IS_DEAD) != 0;
#else
    return FALSE;
#endif
}

bool32 NuzlockeIsBoxMonDeadAt(u32 boxId, u32 boxPosition)
{
#if NUZLOCKE_PERMADEATH == TRUE
    if (boxId >= TOTAL_BOXES_COUNT || boxPosition >= IN_BOX_COUNT)
        return FALSE;
    return NuzlockeIsBoxMonDead(GetBoxedMonPtr(boxId, boxPosition));
#else
    return FALSE;
#endif
}

#if NUZLOCKE_PERMADEATH == TRUE

// Mark dead and move to the graveyard. If the graveyard is full the mon still
// goes to the PC (and is still flagged dead, so it stays unusable) rather than
// being lost.
static void BuryMon(struct Pokemon *mon)
{
    u8 dead = TRUE;
    s16 boxPos = GetFirstFreeBoxSpot(NUZLOCKE_GRAVEYARD_BOX);

    SetMonData(mon, MON_DATA_IS_DEAD, &dead);

    if (boxPos >= 0)
        SetBoxMonAt(NUZLOCKE_GRAVEYARD_BOX, boxPos, &mon->box);
    else
        CopyMonToPC(mon);

    ZeroMonData(mon);
}

u32 NuzlockeSweepFaintedParty(void)
{
    u32 i, deaths = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
            continue;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (GetMonData(mon, MON_DATA_HP) != 0)
            continue;

        BuryMon(mon);
        deaths++;
    }

    if (deaths != 0)
    {
        CompactPartySlots();
        CalculatePlayerPartyCount();
        if (gSaveBlock3Ptr->nuzlockeDeaths + deaths < 255)
            gSaveBlock3Ptr->nuzlockeDeaths += deaths;
        else
            gSaveBlock3Ptr->nuzlockeDeaths = 255;
    }

    return deaths;
}

#else // NUZLOCKE_PERMADEATH

u32 NuzlockeSweepFaintedParty(void)
{
    return 0;
}

#endif // NUZLOCKE_PERMADEATH

// ---------------------------------------------------------------------------
// Rules 4 + 8: battle end
// ---------------------------------------------------------------------------

static u32 RouteStateFromOutcome(u32 outcome)
{
    switch (outcome)
    {
    case B_OUTCOME_CAUGHT:
        return NUZLOCKE_ROUTE_CAUGHT;
    case B_OUTCOME_WON:
        return NUZLOCKE_ROUTE_KILLED;
    default:
        // Ran, mon fled, teleported, forfeited, lost or drew: the route is
        // used up and nothing was gained.
        return NUZLOCKE_ROUTE_FLED;
    }
}

void NuzlockeOnBattleEnd(void)
{
    u32 partyCountBefore;

    if (IsUntrackedBattleType())
    {
        sEncounterClass = NUZLOCKE_ENCOUNTER_NONE;
        sPendingMapSec = NO_PENDING_ROUTE;
        return;
    }

    // Rule 4: the first legal encounter consumes the slot whatever happened.
    // Guarded so a pending route can only ever be spent by a wild battle on
    // that same route — never by a trainer fight, and never after the player
    // has walked somewhere else.
    if (sPendingMapSec != NO_PENDING_ROUTE)
    {
        u32 mapSec = MAPSEC_FROM_PENDING(sPendingMapSec);

        if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER)
         && mapSec == NuzlockeGetCurrentMapSec()
         && !NuzlockeIsRouteUsed(mapSec))
        {
            u32 state = RouteStateFromOutcome(gBattleOutcome);

            NuzlockeSetRouteState(mapSec, state);
            if (state == NUZLOCKE_ROUTE_CAUGHT && gSaveBlock3Ptr->nuzlockeCatches < 255)
                gSaveBlock3Ptr->nuzlockeCatches++;
        }
        sPendingMapSec = NO_PENDING_ROUTE;
    }
    sEncounterClass = NUZLOCKE_ENCOUNTER_NONE;

    // Rule 7 + rule 8.
    partyCountBefore = CalculatePlayerPartyCount();
    NuzlockeSweepFaintedParty();

#if NUZLOCKE_GAME_OVER_ON_WIPE == TRUE
    if (partyCountBefore != 0 && CalculatePlayerPartyCount() == 0)
        NuzlockeSetRunFailed();
#endif
}

// ---------------------------------------------------------------------------
// Rule 9: revives
// ---------------------------------------------------------------------------

bool32 NuzlockeIsBlockedReviveItem(u32 item)
{
#if NUZLOCKE_PERMADEATH == TRUE && NUZLOCKE_BLOCK_REVIVES == TRUE
    // Match on the item, NOT on its effect bits. ITEM4_REVIVE is also set by
    // gItemEffect_RareCandy (it is how the level-up HP restore is expressed),
    // so testing the bit blocked every Rare/Exp/Cap/Endless Candy in the game.
    switch (item)
    {
    case ITEM_REVIVE:
    case ITEM_MAX_REVIVE:
    case ITEM_REVIVAL_HERB:
    case ITEM_SACRED_ASH:
        return TRUE;
    default:
        return FALSE;
    }
#else
    return FALSE;
#endif
}

// ---------------------------------------------------------------------------
// Rule 8: the run-over screen
// ---------------------------------------------------------------------------

#if NUZLOCKE_GAME_OVER_ON_WIPE == TRUE

#define RUN_OVER_TEXT_WIN 0

// Printed in one shot with TEXT_SKIP_DRAW, so \n only: \l and \p both wait on
// a button press and need an interactive text printer running every frame.
static const u8 sText_RunOver[] = _(
    "Your whole party has fallen.\n"
    "THE RUN IS OVER.\n"
    "\n"
    "Your save has been kept as it is.");

static const struct BgTemplate sRunOverBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sRunOverWindowTemplates[] =
{
    [RUN_OVER_TEXT_WIN] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 6,
        .width = 28,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    DUMMY_WIN_TEMPLATE
};

static const u8 sRunOverTextColors[3] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };

static EWRAM_DATA u8 *sRunOverTilemapBuffer = NULL;

static void VBlankCB_RunOver(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_RunOverWaitForInput(void);
static void CB2_RunOverFadeOut(void);
static void CB2_RunOverToTitleScreen(void);

static void CB2_NuzlockeRunOver(void)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sRunOverBgTemplates, ARRAY_COUNT(sRunOverBgTemplates));
    if (sRunOverTilemapBuffer == NULL)
        sRunOverTilemapBuffer = Alloc(BG_SCREEN_SIZE);
    SetBgTilemapBuffer(0, sRunOverTilemapBuffer);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);

    InitWindows(sRunOverWindowTemplates);
    DeactivateAllTextPrinters();
    ResetSpriteData();
    ResetTasks();
    ResetPaletteFade();

    Menu_LoadStdPalAt(BG_PLTT_ID(15));
    // Palette entry 0 doubles as the backdrop; force it black so the message
    // reads as white-on-black rather than inheriting a menu colour.
    LoadPalette(&(const u16){RGB_BLACK}, BG_PLTT_ID(15), PLTT_SIZEOF(1));
    LoadPalette(&(const u16){RGB_BLACK}, 0, PLTT_SIZEOF(1));

    FillWindowPixelBuffer(RUN_OVER_TEXT_WIN, PIXEL_FILL(0));
    AddTextPrinterParameterized4(RUN_OVER_TEXT_WIN, FONT_NORMAL, 2, 2, 1, 0, sRunOverTextColors, TEXT_SKIP_DRAW, sText_RunOver);
    PutWindowTilemap(RUN_OVER_TEXT_WIN);
    CopyWindowToVram(RUN_OVER_TEXT_WIN, COPYWIN_FULL);
    CopyBgTilemapBufferToVram(0);
    ShowBg(0);

    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    EnableInterrupts(INTR_FLAG_VBLANK);
    SetVBlankCallback(VBlankCB_RunOver);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG0_ON | DISPCNT_OBJ_1D_MAP);
    SetMainCallback2(CB2_RunOverWaitForInput);
}

static void CB2_RunOverWaitForInput(void)
{
    UpdatePaletteFade();
    if (!gPaletteFade.active && JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        SetMainCallback2(CB2_RunOverFadeOut);
    }
}

static void CB2_RunOverFadeOut(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_RunOverToTitleScreen);
}

static void CB2_RunOverToTitleScreen(void)
{
    FreeAllWindowBuffers();
    UnsetBgTilemapBuffer(0);
    TRY_FREE_AND_SET_NULL(sRunOverTilemapBuffer);
    SetMainCallback2(CB2_InitTitleScreen);
}

bool32 NuzlockeTryStartRunOverScreen(void)
{
    if (!NuzlockeIsRunFailed())
        return FALSE;

    gMain.state = 0;
    SetMainCallback2(CB2_NuzlockeRunOver);
    return TRUE;
}

#else // NUZLOCKE_GAME_OVER_ON_WIPE

bool32 NuzlockeTryStartRunOverScreen(void)
{
    return FALSE;
}

#endif // NUZLOCKE_GAME_OVER_ON_WIPE

#else // NUZLOCKE_ENABLED

// Engine compiled out: every query answers "no nuzlocke", every hook is a no-op.

bool32 NuzlockeIsActive(void)                                  { return FALSE; }
u32 NuzlockeGetCurrentMapSec(void)                             { return MAPSEC_NONE; }
bool32 NuzlockeIsTrackedMapSec(u32 mapSec)                     { return FALSE; }
u32 NuzlockeGetRouteState(u32 mapSec)                          { return NUZLOCKE_ROUTE_UNUSED; }
bool32 NuzlockeIsRouteUsed(u32 mapSec)                         { return FALSE; }
bool32 NuzlockeIsCurrentRouteUsed(void)                        { return FALSE; }
u32 NuzlockeCountRoutesInState(u32 state)                      { return 0; }
u32 NuzlockeGetCatchCount(void)                                { return 0; }
u32 NuzlockeGetDeathCount(void)                                { return 0; }
bool32 NuzlockeIsRunFailed(void)                               { return FALSE; }
u32 NuzlockeGetGraveyardBoxId(void)                            { return TOTAL_BOXES_COUNT; }
bool32 NuzlockeIsGraveyardBox(u32 boxId)                       { return FALSE; }
bool32 NuzlockeIsSpeciesOwned(u32 species)                     { return FALSE; }
void NuzlockeSetRouteState(u32 mapSec, u32 state)              {}
void NuzlockeSuspendClassification(bool32 suspend)             {}
void NuzlockeSetRunFailed(void)                                {}
void NuzlockeClearRunFailed(void)                              {}
void NuzlockeInitNewRun(void)                                  {}
bool32 NuzlockeSuppressWildEncounters(void)                    { return FALSE; }
void NuzlockeClassifyWildEncounter(void)                       {}
u32 NuzlockeGetEncounterClass(void)                            { return NUZLOCKE_ENCOUNTER_NONE; }
bool32 NuzlockeIsEncounterUncatchable(void)                    { return FALSE; }
void NuzlockeOnMonCaught(void)                                 {}
void NuzlockeOnBattleEnd(void)                                 {}
u32 NuzlockeSweepFaintedParty(void)                            { return 0; }
bool32 NuzlockeIsMonDead(struct Pokemon *mon)                  { return FALSE; }
bool32 NuzlockeIsBoxMonDead(struct BoxPokemon *boxMon)         { return FALSE; }
bool32 NuzlockeIsBoxMonDeadAt(u32 boxId, u32 boxPosition)      { return FALSE; }
bool32 NuzlockeIsBlockedReviveItem(u32 item)                   { return FALSE; }
bool32 NuzlockeTryStartRunOverScreen(void)                     { return FALSE; }
void NuzlockeSetUpGraveyardBox(void)                           {}

#endif // NUZLOCKE_ENABLED
