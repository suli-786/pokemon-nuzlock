# UI Port Checklist — what a SwSh branch can silently delete

**Why this file exists.** The Montblanc SwSh UI branches (map popups, summary screen, storage
screen, and the party / bag screens still to come) do not patch our screens — they *replace* them. A
replacement screen compiles cleanly and boots fine while quietly dropping every hook we added to
the screen it replaced. The failure mode is **a green build and a missing feature**, which no
compiler warning and no `make check` will catch.

Before merging any SwSh branch, walk the table for that branch and confirm each row still works
(code-read at minimum, playtest where noted).

---

## 1. Convention: master toggles live in `include/config/`

Upstream SwSh branches put their master switch inside the feature header
(`include/swsh_summary_screen.h` had `#define SWSH_SUMMARY_SCREEN TRUE` next to 20 internal
tuning defines) and then relied on **include order** — `#include "swsh_summary_screen.h"` was
inserted *above* `#include "pokemon_summary_screen.h"` in each consumer — so that `#if
SWSH_SUMMARY_SCREEN` saw a definition. Any file that includes `pokemon_summary_screen.h` without
the SwSh header first gets `#if <undefined>` → `0` → the SwSh path silently vanishes. That is the
same class of bug this whole document is about.

**Our rule:**

| Lives in | What |
|---|---|
| `include/config/swsh_ui.h` | the **master on/off toggle** for each SwSh UI branch (`SWSH_SUMMARY_SCREEN`, `SWSH_STORAGE_SYSTEM`, and future `SWSH_PARTY_MENU` / `SWSH_BAG_SCREEN`) |
| `include/swsh_*.h` | everything else — the branch's own look-and-feel defines, constants, prototypes |

`include/config/swsh_ui.h` is pulled in from `include/constants/global.h` alongside the other
config headers, so a master toggle is **always** defined everywhere, in every translation unit,
regardless of include order. The feature header `#include`s the config header too, so it is
self-sufficient.

Exception, by design: the **map popup** master toggle is `OW_POPUP_GENERATION` in
`include/config/overworld.h`. That is already an upstream config knob in the right place, so it
stays put; `GEN_8` selects the SwSh popup.

---

## 2. At-risk features

Legend for **Branch**: which future SwSh port can clobber the row.
`storage` = SwSh PC/box screen · `party` = SwSh party menu · `bag` = SwSh bag ·
`summary` = SwSh summary screen · `popup` = SwSh map popup.

### 2.1 Nuzlocke PC gates — `src/pokemon_storage_system.c` (branch: **storage**) — RESOLVED, see §3.3

The SwSh storage branch rewrites the box UI wholesale. All five gates live in functions the
branch is expected to rewrite or replace.

`SWSH_STORAGE_SYSTEM` does **not** `#if` the vanilla file out — `src/pokemon_storage_system.c`
still compiles whole and five entry points early-return into `src/swsh_storage_system.c`. So the
vanilla copies of rows 2–5 below are still *present*, just **dead**. That is the exact green-build
/ missing-feature trap this document is about: grepping for `Nuzlocke` in
`pokemon_storage_system.c` still finds all five hits even when none of them run.

| # | Vanilla site (now dead under `SWSH_STORAGE_SYSTEM`) | Function | What it does | Live site | Verify by |
|---|---|---|---|---|---|
| 1 | `src/pokemon_storage_system.c` L1725 | `ResetPokemonStorageSystem` | calls `NuzlockeSetUpGraveyardBox()` — names the last box and marks it as the graveyard on new game | **still live** — SwSh does not fork this function; `src/new_game.c` L205 calls the global | new game → PC → last box is the graveyard |
| 2 | L2400 | `Task_PokeStorageMain`, `case INPUT_MULTIMOVE_START` | multi-move bypasses the per-mon gate, so the graveyard box refuses group grabs outright (`SE_FAILURE`, no state change) | `src/swsh_storage_system.c` L7318, in `HandleInput_InBox` — **moved on purpose**, see §3.3 | stand in graveyard box, SELECT to the green cursor, press A → buzz, nothing picked up |
| 3 | L4299 | `UpdateBoxToSendMons` | never aims `VAR_PC_BOX_TO_SEND_MON` at the graveyard box | `src/swsh_storage_system.c` L4416, verbatim | scroll to graveyard, leave PC, catch a mon → goes elsewhere |
| 4 | L6518 | `TryStorePartyMonInBox` | graveyard box refuses living deposits (returns `FALSE`) | **rewritten** as `CanDepositMonInBox`, `src/swsh_storage_system.c` L2580 (used at L2623 + L2747) — no `TryStorePartyMonInBox` exists in the SwSh screen | deposit, pick GRAVE in the choose-box grid → buzz, box not accepted |
| 5 | L7804 | `SetMenuTexts_Mon` | dead box mon: refuses the A-press for **every** box option at once (withdraw / move / shift / place), in the menu, auto-action **and** multi-move flows | `src/swsh_storage_system.c` L7814, verbatim | A on a graveyard mon → buzz, no menu |

**Companion gate outside the storage file** (branch: **storage**, but lives elsewhere so it is
easy to miss): `src/pokemon.c` `CopyMonToPC` L3025 skips the graveyard box when auto-boxing a
caught mon. A storage rewrite that reimplements auto-box routing must keep this. The SwSh branch
does **not** touch `src/pokemon.c` and does not reimplement auto-box routing, so this gate is
untouched and still live.

Backing API: `NuzlockeIsGraveyardBox`, `NuzlockeIsBoxMonDead`, `NuzlockeSetUpGraveyardBox` in
`include/nuzlocke.h` / `src/nuzlocke.c`.

### 2.2 Cap Candy + Endless Candy — `src/party_menu.c` (branch: **party**, secondary **bag**) — RESOLVED, see §3.4

**Rows 6–11 no longer exist.** Cap Candy's hand-rolled callback was deleted before the party port
landed (commit `1b520f9a66`) and the item is now pure data — an Exp Candy on a tier that always
overshoots the level cap. Rows 12–13 survived the port and now live in `src/swsh_party_menu.c`.
The table below is kept as the historical record of what the port had to account for.

Cap Candy is a Phase 2a item that jumps a mon straight to `GetCurrentLevelCap()` in one action.
It **was** implemented entirely inside `party_menu.c`, in the region a SwSh party-menu rewrite owns.

| # | Site | Symbol | Note |
|---|---|---|---|
| 6 | ~L6079 | `ItemUseCB_CapCandy` | the entry point; writes EXP directly, `CalculateMonStats` once, no per-level move prompts |
| 7 | ~L6121 | `Task_CapCandyDisplayLevelUpStatsPg1` | stat page 1 |
| 8 | ~L6131 | `Task_CapCandyDisplayLevelUpStatsPg2` | stat page 2 |
| 9 | ~L6141 | `Task_CapCandyTryEvolution` | single evolution check at the end |
| 10 | ~L6152 | `CB2_ReturnToPartyMenuUsingCapCandy` | re-entry callback after the evolution scene (fwd-declared at ~L6033) |
| 11 | ~L6053 | `PartyMenuTryEvolution` — the `else if (GetItemFieldFunc(...) == ItemUseOutOfBattle_CapCandy && ...)` branch | **the easiest row to lose**: it is one `else if` inside an upstream function. Without it the Cap Candy chain drops out of the party menu after evolving. |
| 12 | ~L5857 | `ItemUseCB_RareCandy`, evolution path | `if (GetItemConsumability(gSpecialVar_ItemId)) RemoveBagItem(...)` — Endless Candy is never consumed |
| 13 | ~L5877 | `ItemUseCB_RareCandy`, level-up path | same guard, second call site |

Rows 12–13 are *modifications of upstream lines*, not additions. A SwSh party menu that re-lands
upstream's `RemoveBagItem(gSpecialVar_ItemId, 1)` unconditionally will silently make Endless
Candy consumable again — a bug you only notice by counting items.

Also depends on `ItemUseOutOfBattle_CapCandy` in `src/item_use.c` / `include/item_use.h` and the
item definitions in `src/data/items.h` + `include/constants/items.h`. Those are outside the party
menu and are lower risk, but a **bag** rewrite touches item field-func dispatch.

### 2.3 Summary screen IV/EV display (branch: **summary**) — RESOLVED, see §3.2

| # | Site | Symbol | Note |
|---|---|---|---|
| 14 | `include/config/summary_screen.h` | `P_SUMMARY_SCREEN_IV_EV_INFO = TRUE` | A-button cycling Stats → IVs → EVs on the skills page |
| 15 | same | `P_SUMMARY_SCREEN_IV_EV_VALUES = TRUE` | raw numbers, not letter grades |
| 16 | same | `P_SUMMARY_SCREEN_IV_HYPERTRAIN = TRUE` | hyper-trained stats read as 31 |

The SwSh summary screen **ignores** most of `include/config/summary_screen.h`. It honours only
`P_SUMMARY_SCREEN_RENAME`, `P_SUMMARY_SCREEN_MOVE_RELEARNER` and `P_SHOW_DYNAMIC_TYPES`;
everything else is re-expressed as its own `SWSH_SUMMARY_*` define or hardcoded. See §3 for how
rows 14–16 were reconciled when the branch landed.

### 2.4 Other overhaul hooks worth re-checking per branch

| Site | Branch at risk | Note |
|---|---|---|
| `src/pokemon.c` `PokemonUseItemEffects` ~L3549 | bag / party | nuzlocke rule 9 (revive items inert) + rule 7 (nothing works on a dead mon). Guarded by `!usedByAI`. |
| `src/pokemon.c` `GetAbilityBySpecies` ~L3159 | — | randomizer ability hook; no SwSh branch touches it, listed for completeness |
| `include/pokemon.h` L39-42 / L266-269 | — | `MON_DATA_IS_DEAD` + `BoxPokemon.isDead:1` (was `unused_13`). **Any** branch that touches `struct BoxPokemon` or the `enum MonData` order breaks saves. |
| `include/config/overworld.h` L11 / L61 / L111 / L141 | popup | our four flips: `OW_HIDE_REPEAT_MAP_POPUP`, `OW_FOLLOWERS_ENABLED`, `OW_FLAG_NO_TRAINER_SEE`, `OW_UNION_DISABLE_CHECK`. The popup port only touches `OW_POPUP_GENERATION` at L118. |

---

## 3. Reconciliation log

### 3.1 `swsh_map_popups` — landed

- Master toggle: `OW_POPUP_GENERATION = GEN_8` in `include/config/overworld.h` (L118). Our four
  other flips in that file untouched.
- Nothing in §2 is at risk from this branch: `src/map_name_popup.c` and `src/menu.c` were
  byte-identical between expansion 1.16.1 and our HEAD, so the branch versions were taken whole.
- `src/comfy_anim.c` / `include/comfy_anim.h` landed here **once** as the shared module. The
  `swsh_summary_screen` branch ships a byte-identical copy — do not land it twice. Any future
  SwSh branch that ships `comfy_anim` should be diffed against ours and its copy dropped.

Hardening applied to `comfy_anim` on landing (deviations from upstream, keep them on any refresh):

1. `NUM_COMFY_ANIMS` 8 → 16. Eight slots is one screen's worth; the summary screen alone
   allocates several and the map popup holds one across map transitions.
2. `CreateComfyAnim_Easing` / `CreateComfyAnim_Spring` returned `INVALID_COMFY_ANIM`
   (`== NUM_COMFY_ANIMS`) when the pool was exhausted, and **every** caller then indexes
   `gComfyAnims[id]` unguarded — a straight out-of-bounds read/write into EWRAM. Fixed by sizing
   `gComfyAnims` to `NUM_COMFY_ANIMS + 1` and initialising the overflow slot like any other, so
   the pointer is always in bounds and the animation still completes (shared/degraded rather than
   corrupting memory). `AdvanceComfyAnimations` and `ReleaseComfyAnims` cover the slot too.
3. Dropped the unused `s32 c1` local in `ComfyAnimEasing_EaseInOutBack`.

### 3.2 `swsh_summary_screen` — landed

**IV/EV decision.** The SwSh screen supports IV/EV natively via `SWSH_SUMMARY_SHOW_IV_EV`
(`include/swsh_summary_screen.h`): A on the skills page cycles Stats → IVs → EVs, with a
right-aligned "View IV" / "View EV" / "View Stats" button prompt, and it prints **raw numeric
values**. That covers rows 14 and 15 with no new code — the config is left `TRUE`.

Upstream hardcoded `SWSH_SUMMARY_SHOW_IV_EV TRUE`, i.e. a second independent switch for a feature
we already have a config for. Changed to `#define SWSH_SUMMARY_SHOW_IV_EV
P_SUMMARY_SCREEN_IV_EV_INFO` so there is exactly one switch with one meaning — otherwise flipping
`P_SUMMARY_SCREEN_IV_EV_INFO` would silently do nothing. Behaviour today is unchanged (both are
`TRUE`). `include/swsh_summary_screen.h` now includes `config/summary_screen.h` itself rather than
depending on include order.

Row 16 (`P_SUMMARY_SCREEN_IV_HYPERTRAIN`) was **not** covered: the SwSh screen read
`MON_DATA_*_IV` raw, so a hyper-trained stat showed its true IV instead of 31. Fixed by routing
the six IV reads through the existing `GetAdjustedIvData()` from `src/pokemon_summary_screen.c`
(now declared in `include/pokemon_summary_screen.h`), which is the exact function the vanilla
screen uses and which itself checks `P_SUMMARY_SCREEN_IV_HYPERTRAIN`. Smallest faithful version;
no new page, no new asset.

**Deliberately not ported.** The branch also ships `graphics/fonts/latin_frlg_nums{,_narrow,_narrower}.png`.
Nothing in the branch references them — no font table, no `graphics_file_rules.mk` entry, no
`INCGFX` — so they are dead experiment leftovers and were skipped. If a later SwSh branch wants
FRLG-style digits it will need the font plumbing too, not just the PNGs.

**Known upstream quirk, not fixed** (`src/map_name_popup.c`): under `GEN_8`,
`HideMapNamePopUpWindow` calls `ReleaseComfyAnim(gTasks[gPopupTaskId].tComfyAnimId)`
unconditionally. `tComfyAnimId` is `data[5]`, zero-initialised by `CreateTask`, and is only
assigned once `STATE_SLIDE_IN` runs — so a pop-up hidden before it starts sliding releases comfy
anim slot 0, which it does not own. Harmless today (the summary screen is the only other consumer
and the two are never live at once) but worth remembering if a third consumer appears.

Still config-ignored by the SwSh screen, accepted as-is (cosmetic, no feature loss):
`P_SUMMARY_SCREEN_NATURE_COLORS` (SwSh has its own `SWSH_SUMMARY_NATURE_COLORS`, also `TRUE`),
`P_SUMMARY_SCREEN_IV_EV_BOX_ONLY` / `IV_ONLY` / `EV_ONLY` / `IV_EV_TILESET` (all `FALSE` for us,
so nothing to preserve), `P_FLAG_SUMMARY_SCREEN_IV_EV_INFO` (`0` for us).

**Shared-file hunks hand-applied** (our edits in these files are in disjoint regions and were
verified present before and after):

- `include/pokemon.h` — `PokemonSummaryDoMonAnimation` gains `bool32 isShadow`.
- `include/pokemon_summary_screen.h` — `PSS_PAGE_CONDITIONS` / `PSS_PAGE_MEMO` under the master
  switch, `StopShadowAnimDelayTask` decl, `GetAdjustedIvData` decl, `#include "swsh_summary_screen.h"`.
- `src/pokemon.c` — `tIsShadow` task data, the new param, `StopPokemonAnimationDelayTask`
  `if` → `while`, new `StopShadowAnimDelayTask`.
- `src/pokemon_summary_screen.c` — early-return redirects in `ShowPokemonSummaryScreen`,
  `ShowSelectMovePokemonSummaryScreen`, `GetMoveSlotToReplace`, `SummaryScreen_SetAnimDelayTaskId`,
  plus the one `PokemonSummaryDoMonAnimation` call site.

---

### 3.3 `swsh_storage_system` — landed

**Master toggle:** `SWSH_STORAGE_SYSTEM = TRUE` in `include/config/swsh_ui.h`, per §1. Upstream
had it inside `include/swsh_storage_system.h`; that header now includes `config/swsh_ui.h` and
keeps only its own tuning (`SWSH_STORAGE_CHOOSE_BOX_GRID`, `SWSH_STORAGE_SCROLLING_BG`).

**Shape of the port.** Unlike the summary screen, this branch does not `#if` the vanilla file out.
`src/pokemon_storage_system.c` gains five `if (SWSH_STORAGE_SYSTEM) { ..._SwSh(); return; }`
early-returns (L1626 `ShowPokemonStorageSystemPC`, L7000 `SetMonFormPSS`, L7012
`SetMonFormPSS_ItemHold`, L10139 `UpdateSpeciesSpritePSS`, L10180 `ChooseMonFromStorage`) plus one
`#include`. Everything else in that file still compiles; the box-UI half of it is now unreachable.

Consequences worth remembering:

- `src/swsh_storage_system.c` is a **fork** of `src/pokemon_storage_system.c` at expansion 1.16.2,
  not a patch. Any future edit to box-UI behaviour has to be made in the SwSh file (or both).
- Functions the fork did **not** take stayed shared and global, and are still the only copy:
  `ResetPokemonStorageSystem`, `GetFirstFreeBoxSpot`, `StorageGetCurrentBox`, `SetPCBoxToSendMon`,
  `RemoveSelectedPcMon`, `IsDestinationBoxFull`, the box-name/wallpaper accessors. Gate row 1 rides
  on that and needed no work.
- `static u32 sItemIconGfxBuffer[98]` now exists in both files — ~392 B of IWRAM is spent twice.
  IWRAM sits at 86.6% after this port; worth reclaiming if it ever gets tight.
- Ships **no** `comfy_anim` copy, so §3.1's "don't land it twice" rule needed no action here.
- No `graphics_file_rules.mk` entry needed: the branch migrated to `INCGFX`, so all 58 assets are
  pulled in from `src/data/swsh_storage_system.h`.

**The one gate that had to be rewritten (row 4).** `TryStorePartyMonInBox` has no counterpart in
the SwSh screen. Vanilla's deposit hands the mon to that helper, which finds a slot and stores it,
so a `return FALSE` there both refuses the box and aborts the deposit. The SwSh deposit instead
opens the choose-box grid, validates the picked box inline (`GetFirstFreeBoxSpot(boxId) == -1` →
`SE_FAILURE`), then scrolls and animates the placement itself across ten task states. The gate
therefore moved to the validation step — the only remaining point where a box can still be
refused — folded into a named predicate `CanDepositMonInBox()` so both of `Task_DepositMon`'s
choose-box states (mon already in hand / mon still in the party) share one rule.

**The one gate that had to move (row 2).** Vanilla starts multi-move from `SELECT_BUTTON`, which
returns `INPUT_MULTIMOVE_START` without touching state, so the gate can sit in
`Task_PokeStorageMain`. In SwSh, multi-move is a third **cursor mode** (SELECT cycles
normal → auto-action → multi-move) and `HandleInput_InBox` writes
`sStorage->inBoxMovingMode = MOVE_MODE_MULTIPLE_SELECTING` *before* returning the input. Gating in
the task would leave that flag armed with no `MultiMove_Start` behind it, so the gate sits
immediately above the write instead and returns `INPUT_NONE`.

Rows 3 and 5 pasted verbatim.

**Row 5 got wider for free.** All three SwSh cursor modes funnel their A-press through
`SetSelectionMenuTexts()` → `SetMenuTexts_Mon()`, and the box-title menu only ever offers
Jump/Wallpaper/Name. So one gate still covers every action that can reach a dead box mon.

**Known gaps, unchanged from the vanilla screen** (pre-existing, *not* introduced by this port —
listed so they are not mistaken for port damage):

1. **A living mon can still be hand-placed into the graveyard box** via Move Pokémon: pick a mon
   up, walk to an empty GRAVE slot, Place. Row 5 only fires on a *dead* mon under the cursor and
   an empty slot is neither. Row 4 covers the Deposit flow only. Closing it means adding a
   `sIsMonBeingMoved && NuzlockeIsGraveyardBox(StorageGetCurrentBox())` refusal to the
   `OPTION_MOVE_MONS` branch of `SetMenuTexts_Mon` — in **both** files, or they diverge. Left as a
   rules decision rather than folded into a port.
2. **Move Items mode ignores the dead gate**: `OPTION_MOVE_ITEMS` routes to `SetMenuTexts_Item`,
   which never consults row 5, so a held item can be taken from a dead mon. Arguably correct
   (you want the item back) and identical to the vanilla screen.
3. **Multi-move outside the graveyard is still per-box, not per-mon**: a stray dead mon sitting in
   a normal box would be grabbed by a multi-move rectangle. Row 2 is a whole-box refusal keyed on
   the graveyard, same as vanilla. Not reachable in normal play — the sweep only ever writes dead
   mons to box 13.

**Cosmetic notes.** `ResetPokemonStorageSystem` seeds wallpapers with vanilla's
`MAX_DEFAULT_WALLPAPER` (`WALLPAPER_SAVANNA`), but the SwSh screen has its own 20-entry
type-themed wallpaper set; the ids do not mean the same thing. Harmless — SwSh's `GetBoxWallpaper`
clamps anything `>= WALLPAPER_COUNT` to `WALLPAPER_BASE` — but new-game boxes get arbitrary
wallpapers. `TOTAL_BOXES_COUNT` is 14, under the 15-box cap for `SWSH_STORAGE_CHOOSE_BOX_GRID`, so
the 5×3 grid is active and GRAVE is the last cell; the partial-row math handles 5/5/4 correctly.

`ShowPokemonPCFromParty_SwSh()` is exported from `include/swsh_storage_system.h` and implemented
(`EnterPokeStorage(OPTION_MOVE_MONS)`), but **nothing calls it yet** — it is the hook the SwSh
party-menu branch will need. *(Superseded: §3.4 is what finally calls it.)*

---

### 3.4 `swsh_party_menu` — landed

**Master toggle:** `SWSH_PARTY_MENU = TRUE` in `include/config/swsh_ui.h`, per §1. Tuning lives in
the new `include/swsh_party_menu.h` (`SWSH_PARTY_MENU_PC_ACCESS`, `SWSH_PARTY_MON_IDLE_ANIMS`,
`SWSH_PARTY_MON_IDLE_ANIMS_FRAMES`).

The branch's `include/config/swsh_party_menu.h` was **not** taken — it holds the master switch and
three tuning knobs together, which is exactly the §1 anti-pattern. It also reached consumers by an
include chain (`constants/party_menu.h` → `config/swsh_party_menu.h`). Because we cut that chain,
`src/swsh_party_menu.c` carries an explicit `#include "swsh_party_menu.h"` of its own. **Without
it the four `#if SWSH_PARTY_MENU_PC_ACCESS` blocks evaluate to `0` and the entire PC-from-party
feature vanishes with a green build** — the §1 failure class, live. `include/party_menu.h` includes
the same header so `CB2_ReopenPartyMenuFromPC`'s guarded declaration matches its definition.

**Shape of the port.** Unlike storage (§3.3), this one *does* `#if` the vanilla file out.
`src/party_menu.c` is wrapped in `#if !SWSH_PARTY_MENU` — opened immediately after the include
block, closed at EOF next to the `#endif // TESTING` — so `build/emerald/src/party_menu.o` is
**0 text / 0 data / 0 bss**. Better than storage's dead-but-linked situation, but grep still finds
~8600 lines that never run, so the file carries a header comment saying so. Post-link:
IWRAM **86.57 %** (unchanged — the swap is RAM-neutral), EWRAM 89.06 % (+104 B), ROM 79.16 %.

**Rows 6–11: deleted, not ported.** Cap Candy's callback chain was demolished in commit
`1b520f9a66`, before this port, because it was broken independently of any UI work: it wrote EXP
directly and never entered the engine's move-learn state machine. Root cause of the fix is that
the expansion already does the job — with `B_EXP_CAP_TYPE == EXP_CAP_HARD` and `B_RARE_CANDY_CAP`,
the Exp Candy branch of `PokemonUseItemEffects` clamps experience to `GetCurrentLevelCap()`. So
`ITEM_CAP_CANDY` is now data: `.holdEffectParam = EXP_TO_CAP`, `.fieldUseFunc =
ItemUseOutOfBattle_RareCandy`, `.effect = gItemEffect_RareCandy`. The new tier lives in three
places that must stay in sync:

| Site | What |
|---|---|
| `include/constants/items.h` | `#define EXP_TO_CAP 6` |
| `src/pokemon.c` `sExpCandyExperienceTable` | `[EXP_TO_CAP - 1] = 2000000` |
| `src/data/items.h` `[ITEM_CAP_CANDY]` | `.holdEffectParam = EXP_TO_CAP` |

Miss the table entry and `param - 1 < ARRAY_COUNT(sExpCandyExperienceTable)` goes false,
`dataUnsigned` stays 0, the `if (dataUnsigned != 0) // Failsafe` skips everything, and **Cap Candy
does nothing at all, with no error**. 2,000,000 is chosen because it exceeds 1,640,000 (max L100
experience on the slowest growth rate), so one application always overshoots and the hard-cap
clamp pins it to exactly the cap on any curve.

One defensive edit inside the ported file: `ItemUseCB_RareCandy`'s multi-use branch computed
`u16 candyExp = sExpCandyExperienceTable[tHoldEffectParam - 1]`, which truncates 2,000,000 to
33,920. Widened to `u32` (both `candyExp` and `candyCount`). Unreachable today — Cap Candy is
`.importance = 1` so its quantity is always 1 — but a landmine for anyone who makes a big-tier
candy stackable.

**Rows 12–13: re-established.** The branch re-landed upstream's unconditional
`RemoveBagItem(gSpecialVar_ItemId, ...)` on both candy paths. Guards restored:

| Row | Live site | Function |
|---|---|---|
| 12 | `src/swsh_party_menu.c` L7512 | `ItemUseCB_RareCandy`, cannot-use/evolve path |
| 13 | `src/swsh_party_menu.c` L10568 | `ItemUse_ApplyExpCandy` |

Note the shape change: the SwSh menu splits the apply step into its own `ItemUse_ApplyExpCandy` and
quantifies it (`tItemCount`, reached directly or via a new "how many" prompt), so row 13 is not the
same function it used to be. All 29 `RemoveBagItem` sites in the file were walked; those two are
the only ones on a candy path. **`ItemUse_ApplyEvReduceBerry` / `ItemUse_ApplyEvIncreaseItem` are
deliberately left unguarded** — the vanilla file does not guard them either, and adding it would be
a behaviour change smuggled in under a port.

Free win from the branch: `PartyMenuTryEvolution` keys its re-entry chain on
`GetItemFieldFunc(...) == ItemUseOutOfBattle_RareCandy && CheckBagHasItem(...)`. Endless Candy and
Cap Candy both use that field func and are never consumed, so `CheckBagHasItem` stays TRUE and the
"keep using it" loop survives evolution with no extra code — this is what old row 11 was for.

**§3.1's "don't land it twice" rule fired for the first time.** The branch ships `src/comfy_anim.c`
+ `include/comfy_anim.h` — the **unhardened** upstream copy (pool of 8, `gComfyAnims[NUM_COMFY_ANIMS]`
with no overflow slot). Both paths were dropped from the checkout. The API is identical
(`CreateComfyAnim_Easing/_Spring`, `ReleaseComfyAnim`, `AdvanceComfyAnimations`), so
`swsh_party_menu.c` compiles against ours unchanged. Taking the branch's copy would have silently
reintroduced the out-of-bounds EWRAM write — and it would now hit *more* often, since the party
menu is a **third** comfy-anim consumer alongside the summary screen and the map popup. §3.2's
"harmless today, the two are never live at once" caveat about `HideMapNamePopUpWindow` releasing
slot 0 is worth re-reading with that third consumer in mind.

**PC-from-party plumbing.** The branch patches `src/pokemon_storage_system.c`, whose box UI is
unreachable under `SWSH_STORAGE_SYSTEM` — a verbatim apply is a green build and a dead feature.
Split instead:

- **State owner:** `src/pokemon_storage_system.c`, next to the other `EWRAM_DATA`.
  `EWRAM_DATA static MainCallback sReturnToPartyCallback` plus three accessors —
  `PokemonPC_SetReturnToPartyCallback`, `PokemonPC_HasReturnToPartyCallback`, and the
  consume-once `PokemonPC_TakeReturnToPartyCallback`. Same "shared globals stay in the vanilla
  file" pattern §3.3 describes for `StorageGetCurrentBox` et al. Duplicating the static into both
  forks would give two sources of truth that desync the moment one path writes and the other reads.
  Cost: 4 B EWRAM.
- **Live exit task:** `FieldTask_ReturnToPartyMenu` in `src/swsh_storage_system.c`, placed after the
  `#undef tState/...` block so the task-data macros are out of scope, and `CB2_ExitPokeStorage`
  there now picks between it and `FieldTask_ReturnToPcMenu`. All three exits from the SwSh PC
  (normal exit + the two `EnterPokeStorage` alloc-failure paths) funnel through that one function,
  so one branch covers every route out.
- A dead-but-honest twin of both lives in `src/pokemon_storage_system.c` for the
  `SWSH_STORAGE_SYSTEM = FALSE` case, same discipline §3.3 applied to the five gates.
- `ShowPokemonPCFromParty` is the **sixth** `if (SWSH_STORAGE_SYSTEM) { ..._SwSh(); return; }`
  early-return in `src/pokemon_storage_system.c` — §3.3's list of five is now a list of six. It is
  what finally calls the `ShowPokemonPCFromParty_SwSh` hook that had been sitting there uncalled
  since the storage port.

**Hardening beyond the branch (stale-callback trap).** `swsh_party_menu.c` arms the callback
*before* opening the PC. A session that exits by a route bypassing `CB2_ExitPokeStorage` — notably
`OPTION_SELECT_MON`'s `CB2_ReturnToFieldContinueScript` — leaves the pointer live, and the next PC
opened normally from the field would dump the player into a party menu on exit. Two mitigations,
both applied: the accessor is consume-once (reading clears), and `ShowPokemonStorageSystemPC`
NULLs it on entry so the field PC always starts clean.

**`DisplayPartyMenuStdMessage` is now `UNUSED` and suppresses 13 prompt IDs**
(`src/swsh_party_menu.c` L3495): nine are cleared outright (`CHOOSE_MON`, `CHOOSE_MON_2`,
`MOVE_TO_WHERE`, `TEACH_WHICH_MON`, `USE_ON_WHICH_MON`, `GIVE_TO_WHICH_MON`, `RESTORE_WHICH_MOVE`,
`BOOST_PP_WHICH_MOVE`, `CHOOSE_MON_FOR_BOX`) and four return silently (`DO_WHAT_WITH_MON`,
`DO_WHAT_WITH_ITEM`, `DO_WHAT_WITH_MAIL`, `MOVE_ITEM_WHERE`). `src/fldeff_softboiled.c` calls it 4×
and still links, but Softboiled's "Use on which Pokémon?" prompt no longer appears. Cosmetic, and
invisible to every automated gate — playtest it.

**Shared-file hunks hand-applied** (branch diff worked through selectively; never `git apply`):

- `include/constants/party_menu.h` — took `PARTY_ACTION_MOVE_ITEM 16`, `PARTY_ACTION_FUSION 17`,
  `PARTY_MSG_SEND_MON_TO_BOX 32`; dropped the branch's `#include "config/swsh_party_menu.h"`.
  Safe because `struct PartyMenu.action` is a full `u8`, not a bitfield.
- `include/party_menu.h` — `#include "swsh_party_menu.h"` (not the branch's `constants/party_menu.h`)
  + the guarded `CB2_ReopenPartyMenuFromPC` decl.
- `include/pokemon.h` — took the `GetFormChangeTargetSpecies` / `GetFormChangeTargetSpeciesBoxMon`
  declarations (**required**: both are non-static in `src/pokemon.c` but were declared nowhere, and
  `swsh_party_menu.c` calls the first), and the cosmetic `u8 usedByAI` → `bool8 usedByAI` which
  removes a header/impl mismatch. Skipped the `PokemonSummaryDoMonAnimation` hunk — already ours
  from §3.2.
- `include/pokemon_storage_system.h` — `#include "main.h"` + the PC-from-party decls above.
- `src/menu_specialized.c` — took the whole `DrawLevelUpWindowPg1` 3-digit-delta hunk. This is a
  **bug fix we are inheriting** and it is load-bearing: Cap Candy jumps ~90 levels in one action
  and produces stat deltas well over 99, which the old `ConvertIntToDecimalStringN(..., 2)`
  truncated silently.
- `src/pokemon.c` — took three hunks. (1) `CreateMonSpritesGfxManager` early-returns when the
  manager is already `GFX_MANAGER_ACTIVE`, needed for party ↔ summary round trips now that both
  animate a mon sprite. (2) `DestroyMonSpritesGfxManager` NULLs `sMonSpritesGfxManagers[managerId]`
  *before* the free, so the SwSh summary screen cannot read a dangling global. (3) `evChange`
  widened `s8` → `s16` so the `MAX_TOTAL_EVS - evCount` clamp (up to 510) is representable.
  **The widening was rewritten, not taken.** The branch's version shadows — it declares a fresh
  `s16 evChange` inside the first block while the second assigns the outer one — and more
  importantly `evChange = temp2` with `temp2` a `u32` turns EV-lowering berries (`0xF6` = −10) into
  **+246** once the variable is wide enough to hold it. Both sites are now
  `evChange = (s8)itemEffect[itemEffectParam];`, cast mandatory. Dropped four pure-churn hunks
  (backslash realignment, a `GetMonData(..., NULL)` re-add, three `ITEM5_FRIENDSHIP_*`
  de-indentations that actually break the switch-case style) and skipped four `isShadow` hunks
  already ours from §3.2.
- `test/party_menu.c` — took the `KNOWN_FAILING` hunk on the first of the three tests (SwSh has no
  Cancel or Confirm button, so that navigation assertion cannot hold). **This is the cheapest
  automated detector for the whole include-order failure class**: if that test reports `PASS`
  instead of `EXPECTED_FAIL`, `SWSH_PARTY_MENU` is not reaching `party_menu.c` and the vanilla menu
  is still live. All three tests still link because `swsh_party_menu.c` carries its own `#if TESTING`
  definition of `Test_UpdatePartySelectionSingleLayout`.
- Skipped entirely, already ours: `include/pokemon_summary_screen.h`, `src/pokemon_summary_screen.c`.

**Behaviour deltas accepted consciously** (not regressions, but player-visible):

1. **Cap Candy now runs the full move-learn chain.** The deleted callback wrote EXP directly and
   skipped per-level prompts; the stock path runs `Task_DisplayLevelUpStatsPg1` →
   `Task_TryLearnNewMoves` → `PartyMenuTryEvolution`, so ~90 levels of move prompts arrive in one
   action. That is the documented intent of the decision, and the most visible change in the port.
2. Egg refusal preserved (`IsSelectedMonNotEgg`, the direct counterpart of vanilla's check).
3. Message text changes to the exp-candy string ("gained N Exp. Points and elevated to Lv. X").
   `ConvertIntToDecimalStringN(..., 7)` fits 2,000,000 exactly — watch for clipping.

**Nuzlocke exposure: none.** Our `party_menu.c` delta was five hunks, all candy. The party-adjacent
nuzlocke gates live in `src/pokemon.c` (`PokemonUseItemEffects` rules 7/9, `CopyMonToPC`), which
this port does not touch. But the port opens a **new route into the PC**, so all five §3.3 gates
are now reachable from the party menu as well as the field PC — re-verify them by that route.

---

## 4. Pending branches — pre-merge drill

For **storage**: DONE, see §3.3.

For **party**: DONE, see §3.4. Rows 6–11 no longer exist (Cap Candy deleted); rows 12–13 live at
`src/swsh_party_menu.c` L7512 / L10568.

For **bag**: rows 12–13 indirectly (item consumability). `ItemUseOutOfBattle_CapCandy` is gone —
Cap Candy now dispatches through `ItemUseOutOfBattle_RareCandy` like every other candy, so a bag
rewrite only has to keep `GetItemConsumability` honest in the field-use path.

For **all**: re-read §2.4. Anything touching `struct BoxPokemon` or `enum MonData` is a
save-format change — run `make check TESTS="SaveBlock"` and expect 3/3.
