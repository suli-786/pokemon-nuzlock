#ifndef GUARD_NUZLOCKE_H
#define GUARD_NUZLOCKE_H

#include "config/nuzlocke.h"

struct Pokemon;
struct BoxPokemon;

// Per-route slot state, 2 bits per MAPSEC in SaveBlock3.
// The slot is keyed by gMapHeader.regionMapSectionId, i.e. the on-screen
// location banner: every floor of Granite Cave is one route, and Underwater 124
// is a different route from surface 124.
enum NuzlockeRouteState
{
    NUZLOCKE_ROUTE_UNUSED = 0,  // no legal encounter has resolved here yet
    NUZLOCKE_ROUTE_CAUGHT = 1,  // the route's encounter was caught
    NUZLOCKE_ROUTE_KILLED = 2,  // the route's encounter was knocked out
    NUZLOCKE_ROUTE_FLED   = 3,  // got away / player ran / player lost: used, nothing gained
};

// Classification of the wild Pokémon currently on screen.
enum NuzlockeEncounterClass
{
    NUZLOCKE_ENCOUNTER_NONE,   // not a tracked wild encounter (static, gift, roamer, facility, egg)
    NUZLOCKE_ENCOUNTER_LEGAL,  // the route's encounter: catchable, consumes the slot win or lose
    NUZLOCKE_ENCOUNTER_DUPE,   // species/evo-family already owned: uncatchable, slot-neutral
    NUZLOCKE_ENCOUNTER_SHINY,  // shiny: catchable, slot-neutral
};

// Bits of gSaveBlock3Ptr->nuzlockeRunState.
#define NUZLOCKE_RUN_FAILED      (1 << 0)  // party was wiped; the run is over
#define NUZLOCKE_RUN_STARTED     (1 << 1)  // set once the save has been initialised by the engine

// ---------------------------------------------------------------------------
// Queries. Safe to call with the engine compiled out (they degrade to
// "nuzlocke inactive"). The Phase 5 tracker UI should only need these.
// ---------------------------------------------------------------------------

bool32 NuzlockeIsActive(void);
u32 NuzlockeGetCurrentMapSec(void);
bool32 NuzlockeIsTrackedMapSec(u32 mapSec);
u32 NuzlockeGetRouteState(u32 mapSec);
bool32 NuzlockeIsRouteUsed(u32 mapSec);
bool32 NuzlockeIsCurrentRouteUsed(void);
u32 NuzlockeCountRoutesInState(u32 state);
u32 NuzlockeGetCatchCount(void);
u32 NuzlockeGetDeathCount(void);
bool32 NuzlockeIsRunFailed(void);
u32 NuzlockeGetGraveyardBoxId(void);
bool32 NuzlockeIsGraveyardBox(u32 boxId);
bool32 NuzlockeIsSpeciesOwned(u32 species);

// Mutators. Exposed for the tracker/debug menu; the engine drives them itself.
void NuzlockeSetRouteState(u32 mapSec, u32 state);
void NuzlockeSetRunFailed(void);
void NuzlockeClearRunFailed(void);
void NuzlockeInitNewRun(void);

// ---------------------------------------------------------------------------
// Engine hooks. Called from the encounter/battle/storage pipelines.
// ---------------------------------------------------------------------------

// Rule 5: TRUE when the current map's route slot is spent and every wild
// encounter here must be suppressed.
bool32 NuzlockeSuppressWildEncounters(void);

// Rule 2/3/4: classify the wild Pokémon that was just built into the enemy
// party slot 0. Called at the end of CreateWildMon, i.e. AFTER the randomizer
// has already substituted the species.
void NuzlockeClassifyWildEncounter(void);

// Wrap any CreateWildMon call that builds a Pokémon which will NOT become a
// battle (DexNav's moveset probe) so it neither classifies nor reserves a slot.
void NuzlockeSuspendClassification(bool32 suspend);

u32 NuzlockeGetEncounterClass(void);
bool32 NuzlockeIsEncounterUncatchable(void);
void NuzlockeOnMonCaught(void);

// Rules 4/7/8: consume the route slot, sweep the dead into the graveyard and
// decide whether the run just ended. Called once per battle, at battle end.
void NuzlockeOnBattleEnd(void);

// Rule 7: sweep HP==0 party members into the graveyard box. Returns the number
// of Pokémon that died. Also used by the field-poison path.
u32 NuzlockeSweepFaintedParty(void);

// Rule 7: permanently-dead predicate + usability gates.
bool32 NuzlockeIsMonDead(struct Pokemon *mon);
bool32 NuzlockeIsBoxMonDead(struct BoxPokemon *boxMon);
bool32 NuzlockeIsBoxMonDeadAt(u32 boxId, u32 boxPosition);

// Rule 9: TRUE if this item is a revive the player must not be able to use.
bool32 NuzlockeIsBlockedReviveItem(u32 item);

// Rule 8: called from CB2_WhiteOut. Returns TRUE when it has taken over the
// main callback with the run-over sequence.
bool32 NuzlockeTryStartRunOverScreen(void);

// Naming + housekeeping for the graveyard box.
void NuzlockeSetUpGraveyardBox(void);

#endif // GUARD_NUZLOCKE_H
