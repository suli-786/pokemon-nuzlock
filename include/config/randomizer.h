#ifndef GUARD_CONFIG_RANDOMIZER_H
#define GUARD_CONFIG_RANDOMIZER_H

#include "constants/items.h"
#include "constants/flags.h"
#include "constants/vars.h"

// In-game randomizer (ported from tertu-marten's randomizer via Zetraphes' branch).
//
// Global control. If FALSE, no randomizer functionality will be compiled in and
// the game is 100% vanilla. If TRUE, the randomizer code is compiled and each
// feature is controlled at runtime by the RANDOMIZER_FLAG_* flags below
// (all of which are cleared on a fresh save, so behavior stays vanilla until
// they are set, e.g. from the debug menu or an event script).
#define RANDOMIZER_AVAILABLE                   TRUE

// If TRUE, the trainer ID (including secret ID) is the randomizer seed and no
// extra storage is used. If FALSE, the seed is stored in the two vars below.
#define RANDOMIZER_SEED_IS_TRAINER_ID          TRUE

#if RANDOMIZER_AVAILABLE == TRUE

// If TRUE, dynamically generated randomization tables stored in EWRAM are used.
// This consumes 6 bytes of EWRAM for each species present.
// NOTE: the ported module requires this to be TRUE; the ROM-table variant was
// not brought over.
#define RANDOMIZER_DYNAMIC_SPECIES    TRUE

#if RANDOMIZER_DYNAMIC_SPECIES == TRUE

// If the longest evolutionary chain (excluding babies) is longer than this,
// the dynamic evolutionary stage randomization table will be generated
// incorrectly.
#define RANDOMIZER_MAX_EVO_STAGES   5

#endif // RANDOMIZER_DYNAMIC_SPECIES

// Highest TM that randomized TMs can turn into.
#define RANDOMIZER_MAX_TM           ITEM_TM50

// Vars and features

// These allow you to force enable or disable individual randomization features.
// If undefined, the feature is enabled when the matching flag below is set.
// If defined and TRUE, the feature is always enabled.
// If defined and FALSE, the feature is always disabled.
// Overhaul defaults (docs/overhaul/ROADMAP.md): everything randomized EXCEPT
// abilities (owner: never) — always on, no in-game setup needed. Field items
// are interim-on until the shop model (7.13) replaces loot evolution items.
#define FORCE_RANDOMIZE_WILD_MON                  TRUE
#define FORCE_RANDOMIZE_FIELD_ITEMS               TRUE
#define FORCE_RANDOMIZE_TRAINER_MON               TRUE
#define FORCE_RANDOMIZE_FIXED_MON                 TRUE
#define FORCE_RANDOMIZE_STARTER_AND_GIFT_MON      TRUE
#define FORCE_RANDOMIZE_EGG_MON                   TRUE
#define FORCE_RANDOMIZE_ABILITIES                 FALSE

// These flags control whether a particular randomization feature is active.
// They are ignored if the FORCE_* defines above are set.
// Flag/var ids registered in docs/overhaul/REGISTRY.md.
#ifndef FORCE_RANDOMIZE_WILD_MON
#define RANDOMIZER_FLAG_WILD_MON                      FLAG_UNUSED_0x020
#endif

#ifndef FORCE_RANDOMIZE_FIELD_ITEMS
#define RANDOMIZER_FLAG_FIELD_ITEMS                   FLAG_UNUSED_0x021
#endif

#ifndef FORCE_RANDOMIZE_TRAINER_MON
#define RANDOMIZER_FLAG_TRAINER_MON                   FLAG_UNUSED_0x022
#endif

#ifndef FORCE_RANDOMIZE_FIXED_MON
#define RANDOMIZER_FLAG_FIXED_MON                     FLAG_UNUSED_0x023
#endif

#ifndef FORCE_RANDOMIZE_STARTER_AND_GIFT_MON
#define RANDOMIZER_FLAG_STARTER_AND_GIFT_MON          FLAG_UNUSED_0x024
#endif

#ifndef FORCE_RANDOMIZE_EGG_MON
#define RANDOMIZER_FLAG_EGG_MON                       FLAG_UNUSED_0x025
#endif

#ifndef FORCE_RANDOMIZE_ABILITIES
#define RANDOMIZER_FLAG_ABILITIES                     FLAG_UNUSED_0x026
#endif

// Var holding the enum RandomizerSpeciesMode used for species randomization.
#define RANDOMIZER_VAR_SPECIES_MODE                   VAR_UNUSED_0x404E

#if RANDOMIZER_SEED_IS_TRAINER_ID == FALSE
#define RANDOMIZER_VAR_SEED_L                         VAR_UNUSED_0x40FA
#define RANDOMIZER_VAR_SEED_H                         VAR_UNUSED_0x40FB
#endif

#endif // RANDOMIZER_AVAILABLE

#endif // GUARD_CONFIG_RANDOMIZER_H
