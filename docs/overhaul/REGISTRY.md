# Overhaul Registry — flags, vars, save space, credits

**Rule: nothing claims a flag, var, or save byte without a row here first.** This file exists because ≥5 modules want `FLAG_UNUSED_*`/`VAR_UNUSED_*` ids and SaveBlock3 bytes; two branches silently grabbing the same one is a corrupting bug (conflict C5 in [ROADMAP.md](ROADMAP.md)).

## 1. Event flag allocations

Pool: 375+ `FLAG_UNUSED_0x*` ids in `include/constants/flags.h` (+ `DAILY_FLAGS` region for daily-reset uses). Claim = rename the `FLAG_UNUSED_*` define in place (zero save cost) and record it here.

| Flag (new name) | Replaces | Module | Purpose |
|---|---|---|---|
| `FLAG_OVERHAUL_NO_WILD_ENCOUNTERS` | `FLAG_UNUSED_0x264` | encounters | assigned to `WE_FLAG_NO_ENCOUNTER` (Phase 1); toggled by the Repellant key item (Phase 2a) |
| `FLAG_OVERHAUL_NO_TRAINER_SEE` | `FLAG_UNUSED_0x265` | encounters | assigned to `OW_FLAG_NO_TRAINER_SEE` (Phase 1) |
| `FLAG_OVERHAUL_QUARTERMASTER_KIT` | `FLAG_UNUSED_0x266` | economy NPCs | Oldale Quartermaster one-time gift of the 4-item kit (Cap Candy, Endless Candy, Porta Heal, Repellant) — Phase 2a |
| — Route 103 Old Rod NPC (no new flag) | reuses `FLAG_RECEIVED_OLD_ROD` | economy NPCs | shares Dewford's vanilla flag so double rods are impossible (Phase 2a) |
| — randomizer feature flags (0x020..0x026, referenced not renamed) | `FLAG_UNUSED_0x020`–`0x026` | randomizer | `RANDOMIZER_FLAG_{WILD_MON,FIELD_ITEMS,TRAINER_MON,FIXED_MON,STARTER_AND_GIFT_MON,EGG_MON,ABILITIES}` via `include/config/randomizer.h`; inert while `RANDOMIZER_AVAILABLE` is FALSE |
| — reserved block: QoL toggles (0x267..0x26B) | TBD at assignment | dialog/encounters | `I_EXP_SHARE_FLAG`, `OW_FLAG_POKE_RIDER`, spares (`FLAG_TEXT_SPEED_INSTANT` unneeded — instant text is global); 0x266 was consumed by the Quartermaster (row above) |
| — nuzlocke engine: **0 flags claimed** (reservation released) | — | nuzlocke engine | Phase 3 shipped with all state in SaveBlock3 (see §3). Run-active/run-failed are `NuzlockeIsRunFailed()`; the dupe ball-block is a custom `BALL_THROW_UNABLE_NUZLOCKE_DUPE` state in `GetBallThrowableState`, **not** `WE_FLAG_NO_CATCHING` — that define stays 0 and free |
| — reserved block: quizzes (contiguous run, size TBD by NPC count) | TBD | quiz NPCs | one-time reward flags; daily quizzes use `DAILY_FLAGS` |
| — reserved block: wagers (≤16 flags) | TBD | wager battles | per-NPC wager-completed flags |
| — reserved: AI/dev (≤2) | TBD | ai-quality | `B_FLAG_AI_VS_AI_BATTLE` |

## 2. Event var allocations

Pool: 29 `VAR_UNUSED_0x40*` in `include/constants/vars.h`.

| Var | Module | Purpose |
|---|---|---|
| 1 × var | ai-quality | `B_VAR_DIFFICULTY` |
| 1 × var | dialog-speed | `VAR_LAST_REPEL_LURE_USED` |
| — nuzlocke: **0 vars claimed** (reservation released) | nuzlocke | Phase 3 put settings in `include/config/nuzlocke.h` (compile-time) and run state in SaveBlock3 |
| ≤2 × vars | quiz/wager | tier/session scratch |
| `VAR_UNUSED_0x404E` (referenced not renamed) | randomizer | `RANDOMIZER_VAR_SPECIES_MODE` via `include/config/randomizer.h`; inert while `RANDOMIZER_AVAILABLE` is FALSE |
| `VAR_UNUSED_0x40FA`/`0x40FB` (conditional) | randomizer | seed storage only if `RANDOMIZER_SEED_IS_TRAINER_ID` is set to FALSE (default: seed = trainer ID, no vars used) |

## 3. Save-space ledger

Measured at Phase 0 (see `src/save.c:80-83` STATIC_ASSERTs — the build fails if any block overflows, so this ledger is advisory but the asserts are the enforcement):

| Block | Capacity/free before Phase 0 | After Phase 0 `FREE_*` flips | Planned consumers |
|---|---|---|---|
| SaveBlock1 | 304 B free | +2384 B freed (mystery event/gift, union chat, enigma berry, link records, extra seen flags) | overflow space only; flags/vars live here already at zero cost |
| SaveBlock2 | 84 B free | +1344 B freed (BT e-reader, Pokémon Jump, hall records, extra seen flags) | options bits for new settings menus |
| SaveBlock3 | 1620 of 1624 B free | unchanged | **primary home for new persistent state** (below) |

**Kept on purpose:** `FREE_TRAINER_HILL`/`FREE_TRAINER_TOWER` FALSE (playable facilities). `FREE_MATCH_CALL` flipped TRUE in Phase 1 — owner decided no rematches (+104 B).

SaveBlock3 reservations (target ≤1300 B, keep ≥300 B margin for upstream merges — upstream also adds SB3 fields):

| Reservation | Bytes | Module | Status |
|---|---|---|---|
| `nuzlockeRoutes[]` — 2-bit route state × `MAPSEC_COUNT` (210) | **53** | nuzlocke engine | **CLAIMED (Phase 3)** — `NUZLOCKE_ROUTE_BYTES` in `include/config/nuzlocke.h`; 4 states: unused/caught/killed/fled |
| `nuzlockeRunState` + `nuzlockeCatches` + `nuzlockeDeaths` (u8 each) | **3** | nuzlocke engine | **CLAIMED (Phase 3)** |
| Graveyard | 0 | nuzlocke engine | **not needed** — Phase 3 uses PC box 13 (`TOTAL_BOXES_COUNT - 1`, renamed "GRAVE") plus a repurposed `BoxPokemon` bit, so zero SB3 cost |
| Run seed (u32) | 4 | randomizer | reserved (seed is currently the trainer ID) |
| Wager ledger (staked-mon records, small) | ~32 | wager battles | reserved |
| Tracker UI scratch (Phase 5) | ~16 | nuzlocke tracker | reserved |
| **Total claimed so far** | **56** | | SaveBlock3 = **60 B** of 1624 (`test/save.c` `T_SAVEBLOCK3_SIZE`) |

**Bitfield claims outside the SaveBlocks:**

| Bit | Was | Module | Purpose |
|---|---|---|---|
| `struct BoxPokemon.isDead:1` (`include/pokemon.h`) | `unused_13:1` | nuzlocke engine | permadeath marker, readable via `MON_DATA_IS_DEAD`. This was the **last free bit in `BoxPokemon`** — any other workstream that wants a per-mon bit now needs a different home. |
| PC box `TOTAL_BOXES_COUNT - 1` (box 13) | box "BOX14" | nuzlocke engine | the graveyard. `CopyMonToPC`, `IsDestinationBoxFull` and `UpdateBoxToSendMons` all skip it, so living Pokémon never land there. Under `SWSH_STORAGE_SYSTEM` the deposit-side gate is `CanDepositMonInBox` in `src/swsh_storage_system.c` (the SwSh screen has no `TryStorePartyMonInBox`). |

**Banned by ledger:** `USE_DEXNAV_SEARCH_LEVELS` (~1500 B — does not fit alongside the tracker; DexNav itself is fine). `OW_SHOW_ITEM_DESCRIPTIONS=FIRST_TIME` (SB3 cost, use ALWAYS mode). `FNPC_ENABLE_NPC_FOLLOWERS` grows SB3 — claim a row first if ever wanted.

**Hard rule:** save layout freezes when a run starts. No upstream merges, no config flips that touch SaveBlocks, mid-run.

## 4. Credits tracker

Every ported branch/asset/tutorial gets a row when its code lands (CREDITS.md gets the final formatted entry; this is the working list). Personal-use project, but credit is the community norm regardless.

| Source / author | What we take | Status |
|---|---|---|
| RHH pokeemerald-expansion | base + everything | in CREDITS.md already |
| devolov (pret wiki nuzlocke tutorial) | nuzlocke design: dead-bit-in-BoxPokemon, location consolidation, 3-state encounter check | Phase 3 reimplemented from scratch against 1.16.3 — no code lifted, design credit owed |
| NecroDingo (nuzlocke-challenge branch) | nuzlocke engine patterns: hook-site map, dupes/shiny clause shape | Phase 3 reimplemented from scratch — no code lifted, design credit owed; courtesy ask still optional |
| TheXaman | tracker storage scheme, registered-items menu, options-plus | pending |
| iriv24 | registered-items expansion update | pending |
| fisham33 | select-mons (pick-4), battle-mode toggle | pending |
| Montblanc (montmoguri) | SwSh UI suite | **fully landed** — all seven branches (map pop-up, summary, storage, party, message box, bag, battle UI); see the rows below. Six are finished; `dev_battle_ui` landed deliberately unfinished, see its row |
| Montblanc (montmoguri), branch `swsh_map_popups` | SwSh map name pop-up: `graphics/map_popup/swsh.png`, the `GEN_8` paths in `src/map_name_popup.c` + `src/menu.c` | **landed** (Phase 4 UI port). `OW_POPUP_GENERATION = GEN_8`. |
| Montblanc (montmoguri), branch `swsh_summary_screen` | SwSh summary screen: `src/swsh_summary_screen.c`, `include/swsh_summary_screen.h`, 30 assets under `graphics/summary_screen/swsh/`, plus the shim hunks in `pokemon.c`/`pokemon.h`/`pokemon_summary_screen.c`/`.h` | **landed** (Phase 4 UI port). Master toggle relocated to `include/config/swsh_ui.h`; IV/EV reads routed through `GetAdjustedIvData` for hyper-training. |
| Montblanc (montmoguri), branch `swsh_storage_system` | SwSh PC box screen: `src/swsh_storage_system.c`, `src/data/swsh_storage_system.h`, `include/swsh_storage_system.h`, 58 assets under `graphics/pokemon_storage/swsh/`, plus the 5 redirect hunks in `src/pokemon_storage_system.c` | **landed** (Phase 4 UI port). Master toggle relocated to `include/config/swsh_ui.h`; all five nuzlocke PC gates re-implemented inside the new screen — see docs/overhaul/UI_PORT_CHECKLIST.md §3.3. Ships no `comfy_anim` copy. |
| Montblanc (montmoguri), branch `swsh_party_menu` | SwSh party menu: `src/swsh_party_menu.c`, `src/data/swsh_party_menu.h`, 20 assets under `graphics/party_menu/swsh/`, plus hand-applied hunks in `include/constants/party_menu.h`, `include/party_menu.h`, `include/pokemon.h`, `include/pokemon_storage_system.h`, `src/menu_specialized.c`, `src/pokemon.c`, `test/party_menu.c` | **landed** (Phase 4 UI port), ref `9c2b81bd46`. Master toggle relocated to `include/config/swsh_ui.h`; new `include/swsh_party_menu.h` holds tuning and is included directly by the ported file. `src/party_menu.c` compiled out under `#if !SWSH_PARTY_MENU`. Endless/Cap Candy consumability guards re-established; PC-from-party plumbing hand-written against `src/swsh_storage_system.c`. Branch's `comfy_anim` copy **dropped** (ours is hardened). See docs/overhaul/UI_PORT_CHECKLIST.md §3.4. |
| Montblanc (montmoguri), branch `swsh_message_box` | SwSh message box + name box: 5 assets under `graphics/text_window/swsh/` + `graphics/interface/swsh/std_menu.pal`, plus the hunks in `include/menu.h`, `src/menu.c`, `src/text.c`, `src/text_window.c`, `src/graphics.c` | **landed** (Phase 4 UI port), ref `0699f01eaf`. Master toggle `SWSH_MESSAGE_BOX` in `include/config/swsh_ui.h`. Touches shared text/window plumbing, so every text box and menu border in the game picks it up. |
| Montblanc (montmoguri), branch `dev_bag_menu` | SwSh bag screen: `src/swsh_item_menu.c` (9020 lines), `include/swsh_item_menu.h`, 18 assets under `graphics/bag/swsh/`, plus hand-applied hunks in `include/item_menu.h`, `src/item_menu.c`, `src/battle_pyramid_bag.c`, `src/item_use.c` | **landed** (Phase 4 UI port). `dev_bag_menu` chosen over `swsh_bag_menu` — they are the one diverging pair and `dev` is strictly one commit ahead. Master toggle `SWSH_ITEM_MENU` relocated to `include/config/swsh_ui.h`; tuning stays in `include/swsh_item_menu.h`, which the ported file includes directly. `src/item_menu.c` and `src/battle_pyramid_bag.c` compiled out. **`SWSH_ITEM_MENU_IN_BAG_USE` forced FALSE** so item Use/Give still routes through `src/swsh_party_menu.c` and its Cap/Endless Candy guards; the branch's `include/battle.h` + `src/battle_script_commands.c` hunks are therefore not taken. Branch's `comfy_anim` copy **dropped** (ours is hardened). See docs/overhaul/UI_PORT_CHECKLIST.md §3.5. |
| Montblanc (montmoguri), branch `dev_battle_ui` | SwSh battle HUD: 38 replaced assets copied to `graphics/battle_interface/swsh/` + `graphics/text_window/swsh/move_desc_box.{png,bin}` + `graphics/battle_interface/swsh/category_icons.png`, plus hand-applied/rewritten hunks in `src/battle_interface.c`, `src/battle_controller_player.c`, `src/menu.c`, `src/text_window.c`, `src/graphics.c`, `src/data/graphics/gimmicks.h`, `src/type_icons.c`, `include/menu.h`, `include/text_window.h`, `include/battle_interface.h`, `graphics_file_rules.mk` | **landed unfinished, on purpose** (Phase 4 UI port), ref `b2106afda6`. Master toggle `SWSH_BATTLE_UI` in `include/config/swsh_ui.h`; FALSE restores the stock battle screen and relinks to the exact pre-port ROM size. Base is 1.14.1+8 vs our 1.16.3-dev — 14 of 37 hunks rejected outright, so the four healthbox text hunks were **reimplemented** against the current sprite-blitting API. Branch's hardcoded `SWSH_MOVE_DESC_WINDOW_BASE_TILE_NUM 0x21D` corrected to a derived `0x223` (it would have corrupted the standard window frame under `SWSH_MESSAGE_BOX`). Branch's `include/config/battle.h`, `include/constants/flags.h` (would collide with the §1 `FLAG_OVERHAUL_*` claims), `src/battle_gimmick.c` trigger positions and action-prompt deletions **not taken**. Type icon positions re-derived against the new art; doubles-opponent slots shifted +6px. See docs/overhaul/UI_PORT_CHECKLIST.md §3.6, and §3.6.9 for the open items. |
| pret / pokefirered (via Montblanc) | `src/comfy_anim.c` + `include/comfy_anim.h` — the easing/spring animation module. Originates in **pret/pokefirered**; the summary-screen, storage and party branches all ship a byte-identical (unhardened) copy, landed once here. | **landed** (Phase 4 UI port), with our hardening — see docs/overhaul/UI_PORT_CHECKLIST.md §3.1 |
| pollythadon (+ EternalCode, PlatinumMaster, NicoSwag, mudskipper13) | BW battle UI (if adopted) | pending |
| miriamlefae | Unbound-style start menu (if adopted) | pending |
| ravepossum | BW summary / HGSS battle UI (if adopted) | pending |
| grunt-lucas | sample-ui scaffold | pending |
| fakuzatsu (verdant) | team preview screen | **permission required before use** |
| Pokabbie (Emerald Rogue) | design reference only | no code lifted so far |
| Istorian (Randolocke v1.1) | design reference: item kit (Cap/Endless Candy, Porta Heal, Repellant), Oldale gift NPCs, Route 103 rod NPC | Phase 2a reimplemented from scratch (no public source) |
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
