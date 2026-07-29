# Overhaul Registry — flags, vars, save space, credits

**Rule: nothing claims a flag, var, or save byte without a row here first.** This file exists because ≥5 modules want `FLAG_UNUSED_*`/`VAR_UNUSED_*` ids and SaveBlock3 bytes; two branches silently grabbing the same one is a corrupting bug (conflict C5 in [ROADMAP.md](ROADMAP.md)).

## 1. Event flag allocations

Pool: 375+ `FLAG_UNUSED_0x*` ids in `include/constants/flags.h` (+ `DAILY_FLAGS` region for daily-reset uses). Claim = rename the `FLAG_UNUSED_*` define in place (zero save cost) and record it here.

| Flag (new name) | Replaces | Module | Purpose |
|---|---|---|---|
| `FLAG_OVERHAUL_NO_WILD_ENCOUNTERS` | `FLAG_UNUSED_0x264` | encounters | assigned to `WE_FLAG_NO_ENCOUNTER` (Phase 1) |
| `FLAG_OVERHAUL_NO_TRAINER_SEE` | `FLAG_UNUSED_0x265` | encounters | assigned to `OW_FLAG_NO_TRAINER_SEE` (Phase 1) |
| — reserved block: QoL toggles (0x266..0x26B) | TBD at assignment | dialog/encounters | `I_EXP_SHARE_FLAG`, `OW_FLAG_POKE_RIDER`, spares (`FLAG_TEXT_SPEED_INSTANT` unneeded — instant text is global) |
| — reserved block: nuzlocke (≤8 flags) | TBD | nuzlocke engine | run-active, mode bits, `WE_FLAG_NO_CATCHING` dupe gate |
| — reserved block: quizzes (contiguous run, size TBD by NPC count) | TBD | quiz NPCs | one-time reward flags; daily quizzes use `DAILY_FLAGS` |
| — reserved block: wagers (≤16 flags) | TBD | wager battles | per-NPC wager-completed flags |
| — reserved: AI/dev (≤2) | TBD | ai-quality | `B_FLAG_AI_VS_AI_BATTLE` |

## 2. Event var allocations

Pool: 29 `VAR_UNUSED_0x40*` in `include/constants/vars.h`.

| Var | Module | Purpose |
|---|---|---|
| 1 × var | ai-quality | `B_VAR_DIFFICULTY` |
| 1 × var | dialog-speed | `VAR_LAST_REPEL_LURE_USED` |
| ≤2 × vars | nuzlocke | settings bitmask, run state |
| ≤2 × vars | quiz/wager | tier/session scratch |

## 3. Save-space ledger

Measured at Phase 0 (see `src/save.c:80-83` STATIC_ASSERTs — the build fails if any block overflows, so this ledger is advisory but the asserts are the enforcement):

| Block | Capacity/free before Phase 0 | After Phase 0 `FREE_*` flips | Planned consumers |
|---|---|---|---|
| SaveBlock1 | 304 B free | +2384 B freed (mystery event/gift, union chat, enigma berry, link records, extra seen flags) | overflow space only; flags/vars live here already at zero cost |
| SaveBlock2 | 84 B free | +1344 B freed (BT e-reader, Pokémon Jump, hall records, extra seen flags) | options bits for new settings menus |
| SaveBlock3 | 1620 of 1624 B free | unchanged | **primary home for new persistent state** (below) |

**Kept on purpose:** `FREE_TRAINER_HILL`/`FREE_TRAINER_TOWER` FALSE (playable facilities). `FREE_MATCH_CALL` flipped TRUE in Phase 1 — owner decided no rematches (+104 B).

SaveBlock3 reservations (target ≤1300 B, keep ≥300 B margin for upstream merges — upstream also adds SB3 fields):

| Reservation | Bytes | Module |
|---|---|---|
| Route tracker: 4-bit status × ~90 MAPSECs | 45 | nuzlocke tracker |
| Graveyard: 32 × 16 B entries | 512 | nuzlocke tracker |
| Nuzlocke run state (seed echo, settings, counters) | ~32 | nuzlocke engine |
| Run seed (u32) | 4 | randomizer |
| Wager ledger (staked-mon records, small) | ~32 | wager battles |
| **Total planned** | **~625** | leaves ~995 B |

**Banned by ledger:** `USE_DEXNAV_SEARCH_LEVELS` (~1500 B — does not fit alongside the tracker; DexNav itself is fine). `OW_SHOW_ITEM_DESCRIPTIONS=FIRST_TIME` (SB3 cost, use ALWAYS mode). `FNPC_ENABLE_NPC_FOLLOWERS` grows SB3 — claim a row first if ever wanted.

**Hard rule:** save layout freezes when a run starts. No upstream merges, no config flips that touch SaveBlocks, mid-run.

## 4. Credits tracker

Every ported branch/asset/tutorial gets a row when its code lands (CREDITS.md gets the final formatted entry; this is the working list). Personal-use project, but credit is the community norm regardless.

| Source / author | What we take | Status |
|---|---|---|
| RHH pokeemerald-expansion | base + everything | in CREDITS.md already |
| devolov (pret wiki nuzlocke tutorial) | nuzlocke design | pending port |
| NecroDingo (nuzlocke-challenge branch) | nuzlocke engine patterns | pending port; courtesy ask |
| TheXaman | tracker storage scheme, registered-items menu, options-plus | pending |
| iriv24 | registered-items expansion update | pending |
| fisham33 | select-mons (pick-4), battle-mode toggle | pending |
| Montblanc (montmoguri) | SwSh UI suite | pending |
| pollythadon (+ EternalCode, PlatinumMaster, NicoSwag, mudskipper13) | BW battle UI (if adopted) | pending |
| miriamlefae | Unbound-style start menu (if adopted) | pending |
| ravepossum | BW summary / HGSS battle UI (if adopted) | pending |
| grunt-lucas | sample-ui scaffold | pending |
| fakuzatsu (verdant) | team preview screen | **permission required before use** |
| Pokabbie (Emerald Rogue) | design reference only | no code lifted so far |
| resetes12 (Modern Emerald) | challenges viewer reference, nuzlocke indicator art | **ask before using art** |
| huderlem | Poryscript (MIT) | pending |
| Smogon / @pkmn (data.pkmn.cc) | gym set data via converter | pending |
| PokeCommunity sprite threads (DS-style 64x64, Platinum OW/trainer pack) | sprite assets | **read thread rules manually first** |

## 5. Module conventions (Phase 0 decision, from ws_architecture.md)

- One feature = `src/<feature>.c` + `include/config/<feature>.h` (+ optional `docs/overhaul/how_to_<feature>.md`) — flat files, matching repo idiom; Makefile auto-globs.
- Persistent state → SaveBlock3 behind the ledger; scripted state → registry flags/vars.
- Behavior variants → const struct of function pointers selected by config (model: `struct TestRunner`, `include/test/test.h:11`).
- Every battle-visible feature ships with a `TEST()`/`AI_*_BATTLE_TEST` where the harness can express it; `make check` stays green.
- Upstream merges: tagged patch releases only, never mid-run, always followed by `make check`.
