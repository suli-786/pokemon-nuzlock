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

### 3.5 `dev_bag_menu` — landed

**Which branch.** `swsh_bag_menu` and `dev_bag_menu` are the **only** diverging pair in this remote
(every other `swsh_*` / `dev_*` pair points at the same commit). `dev_bag_menu` is strictly ahead:
`git log refs/ports/swsh_bag_menu ^refs/ports/dev_bag_menu` is **empty**, the other direction has
exactly one commit — `d6c5d66cbd` "parameterize icon sprite coordinates, fix party blend in TM/HM,
remove leftover vanilla scroll pocket arrows". `dev_bag_menu` was taken; there is nothing in
`swsh_bag_menu` that is not in it.

**Master toggle:** `SWSH_ITEM_MENU = TRUE` in `include/config/swsh_ui.h`, per §1. Note the **name**:
§1 had reserved the line as `SWSH_BAG_SCREEN`, but the ported code tests `SWSH_ITEM_MENU` in ~9000
lines and four shared files, so the config keeps the branch's spelling rather than adding a rename
layer — same call as `SWSH_STORAGE_SYSTEM` / `SWSH_PARTY_MENU`. Upstream's
`include/swsh_item_menu.h` held the master switch *and* eleven tuning knobs together (the §1
anti-pattern); the master moved out, the knobs stayed, and the header now `#include`s
`config/swsh_ui.h` itself. `src/swsh_item_menu.c` carries an explicit `#include "swsh_item_menu.h"`
as its second line — the §3.4 lesson, since the knobs do **not** ride the global config chain.

**Shape of the port.** Like the party menu (§3.4) and unlike storage (§3.3), this branch `#if`s the
vanilla files out — *two* of them:

| File | Wrapper | Post-link |
|---|---|---|
| `src/item_menu.c` | `#if !SWSH_ITEM_MENU` | `build/emerald/src/item_menu.o` = 0 text / 0 data / 0 bss |
| `src/battle_pyramid_bag.c` | `#if !SWSH_ITEM_MENU_PYRAMID` | `build/emerald/src/battle_pyramid_bag.o` = 0 / 0 / 0 |

`src/swsh_item_menu.c` (9020 lines) replaces **both**: it re-exports `gPyramidBagMenu`,
`gPyramidBagMenuState`, `CloseBattlePyramidBag`, `Task_CloseBattlePyramidBagMessage` and
`DisplayItemMessageInBattlePyramid` at its tail (L8874+), aliasing the pyramid bag onto the same
allocation as `gBagMenu`. Both dead files carry a header comment saying so. Post-link:
EWRAM **89.07 %**, IWRAM **86.57 %** (unchanged), ROM **79.17 %** — a ~0.01 pp move on two of three,
because two whole screens left the link at the same time as one bigger one arrived.

---

**THE CENTRAL DECISION: `SWSH_ITEM_MENU_IN_BAG_USE` is FALSE.**

Upstream ships it `(SWSH_ITEM_MENU && TRUE)`. It is now `(SWSH_ITEM_MENU && FALSE)` in
`include/swsh_item_menu.h`.

*What it does when TRUE.* The bag stops handing items to the party menu and performs Use / Give
itself, on a party panel drawn inside the bag screen. Concretely it intercepts at two points in
`src/item_use.c` — `SetUpItemUseCallback` (field / bag use) and `ItemUseInBattle_ShowPartyMenu`
(in-battle use) — and short-circuits to `BagMenu_OpenPartySelect` / `BagMenu_OpenPartySelectBattle`
before `gBagMenu->newScreenCallback` is ever set.

*Why we turned it off.* Every one of those short-circuits **bypasses `src/swsh_party_menu.c`**,
which is where the two `GetItemConsumability` guards live (§3.4 rows 12–13, L7512 / L10568) that
keep `ITEM_ENDLESS_CANDY` and `ITEM_CAP_CANDY` infinite-use. The in-bag path re-implements the whole
rare-candy chain from scratch and calls `RemoveBagItem` unconditionally on both of its candy exits —
`src/swsh_item_menu.c` **L7046** (evolution path) and **L7062** (apply path, `appliedCount`). Those
are the exact in-bag analogues of rows 12–13, and they are unguarded. Landing the branch as shipped
would have silently made both candies consumable again — the second time in this port suite that a
SwSh branch broke the custom candies, by a different mechanism than the party menu did. The
`grep -c GetItemConsumability src/swsh_party_menu.c` == 2 gate does **not** catch this: the guards
are still there, they just stop being on the path.

*What one flag collapses.* `IN_BAG_USE` is the parent of four other knobs, all of which go FALSE
with it, so the whole semantic-conflict surface disappears at once:

| Knob | What it gated |
|---|---|
| `SWSH_ITEM_MENU_IN_BAG_REUSE` | cursor stays in the party panel after a use/give, for repeat use |
| `SWSH_ITEM_MENU_IN_BATTLE_USE` | in-battle item use resolved inside the bag, incl. 12v12 multi-battle partner-party targeting |
| `SWSH_ITEM_MENU_PARTY_HP_BAR` | live HP bar + status icons in the in-bag party slots |
| `SWSH_ITEM_MENU_PYRAMID_ACTION` | the same inline Use/Give inside the Battle Pyramid bag |

It also drops four window IDs from `enum` in `include/item_menu.h` (`ITEMWIN_PP_MOVE_SELECT`,
`ITEMWIN_LEVEL_UP_STATS`, `ITEMWIN_ROTOM_CATALOG`, `ITEMWIN_ZYGARDE_CUBE`), ~19 fields from
`struct BagMenu`, and leaves three assets referenced only from dead code
(`party_slots.bin`, `status_icons.png`, `prompt_swap.png`). They still get built — the `INCGFX`
dependency scan is textual, not preprocessor-aware — but nothing incbins them, so they cost 0 ROM.

*What survives, i.e. what we still get.* Everything visual: the scrolling background
(`SWSH_ITEM_MENU_SCROLLING_BG`), the SwSh tileset / cursor / hover slots / scroll thumb, the four
sort orders, TM/HM move-type + category icons and contest info (`SWSH_ITEM_MENU_CONTEST_INFO`), the
`ITEMWIN_SELL_PRICE` window, the SwSh Battle Pyramid bag (`SWSH_ITEM_MENU_PYRAMID`) and the in-battle
**battle pockets** (`SWSH_ITEM_MENU_BATTLE_POCKETS` — Medicine / Poké Balls / Battle Items / Berries
instead of the field pockets). Item routing simply lands back where it did before this port: the SwSh
party menu.

*Hunks deliberately NOT taken because of the flag.* Both of the branch's battle-engine hunks are
100 % inside `#if SWSH_ITEM_MENU_IN_BATTLE_USE`, so with the flag off they emit nothing:

- `include/battle.h` — `#include "swsh_item_menu.h"` + `bool8 itemTargetPartner[MAX_BATTLERS_COUNT]`
  in `struct BattleStruct`.
- `src/battle_script_commands.c` — a new `ItemUseTargetsPartnerParty()` helper plus six
  redirect blocks in `BS_ItemRestoreHP`, `BS_ItemCureStatus`, `BS_ItemIncreaseStat`,
  `BS_ItemRestorePP`, so an item used on the *partner's* party in a 12v12 multi battle resolves
  against `gParties[B_TRAINER_PARTNER]`.

Taking them would have meant injecting `#include "swsh_item_menu.h"` into `include/battle.h` — a
header the whole battle engine pulls — to emit zero code, into two files that both carry local
overhaul deltas. They are recorded here instead. **Re-enabling `IN_BAG_USE` is a code change, not a
flag flip:** it needs (1) `GetItemConsumability` guards at `src/swsh_item_menu.c` L7046 + L7062,
(2) both hunks above re-applied from `refs/ports/dev_bag_menu` (base `ad0fd4d17f`), (3) a re-check
that nuzlocke rules 7/9 in `src/pokemon.c` `PokemonUseItemEffects` still sit on the new path.

---

**No hunk had to be redirected out of a dead file — checked, and here is the proof.** Our only local
delta in `src/item_menu.c` was upstream `42425ca6a2` ("Fix bag list truncation when tossing a whole
stack in berry pocket", RHH #10423), which rewrote `MergeSort`'s `usedCapacity` scan to not stop at
the first empty slot. That commit is **not** an ancestor of the branch base, so the fork could have
missed it — but Montblanc landed the identical fix independently as branch commit `301ec5799d`, and
`src/swsh_item_menu.c` L5148 already carries the fixed loop. Nothing to move.

The **message box / window-frame port (§ commit `0699f01eaf`) did not touch `src/item_menu.c`** — it
only patched `include/menu.h`, `src/menu.c`, `src/text.c`, `src/text_window.c`, `src/graphics.c`,
none of which this port kills. So unlike the party/storage case there was no window-frame work to
redirect. The SwSh frames still apply to the new bag: it draws its text through the shared
`src/text.c` / `src/menu.c` plumbing.

**Nuzlocke exposure: verified nil, by two paths.**

1. **Dupes clause.** Our `GetBallThrowableState` → `BALL_THROW_UNABLE_NUZLOCKE_DUPE` gate lives in
   `src/item_use.c`, which is **live**. The fork's `ItemMenu_UseInBattle` is byte-identical to the
   vanilla one and dispatches to `ItemUseInBattle_BagMenu` → `CannotUseItemsInBattle` →
   `GetBallThrowableState`. Gate intact, and now reachable through the battle-pockets bag too.
2. **Revive / dead-mon rules 7 and 9** live in `src/pokemon.c` `PokemonUseItemEffects`, which this
   port does not touch, and which is only reached from the party-menu path — the path
   `IN_BAG_USE = FALSE` keeps us on.

Registered key items (`ITEM_REPELLANT`, `ITEM_PORTA_HEAL`) also survive: the fork's
`UseRegisteredKeyItemOnField` (L4083) does `CreateTask(GetItemFieldFunc(gSaveBlock1Ptr->registeredItem), 8)`,
i.e. straight through the item's field func, exactly as vanilla did.

**§3.1's "don't land it twice" rule fired for the second time.** The branch ships `src/comfy_anim.c`
+ `include/comfy_anim.h` again — still the unhardened upstream copy (`diff` vs ours: 10 lines in the
header, 40 in the source). Both paths were kept out of the checkout; `git diff HEAD --
src/comfy_anim.c include/comfy_anim.h` is empty. The bag is now the **fourth** comfy-anim consumer
(cursor, scroll thumb, two pocket arrows = four live slots; a fifth, the party item icon, is inside
the disabled `IN_BAG_USE` block), so §3.1's `NUM_COMFY_ANIMS` 8 → 16 widening is doing real work
here.

**Deliberately not ported.** `graphics/bag/swsh/berry_flavor_mark.png` — referenced nowhere in the
branch (branch commit `3e1c653b20` switched berry flavours from sprites to text colours and left the
PNG behind). Dropped, same call as §3.2's `latin_frlg_nums` fonts. The other 18 assets under
`graphics/bag/swsh/` were taken. No `graphics_file_rules.mk` entry is needed — the branch uses
`INCGFX` / `INCBIN` throughout, like storage.

**Shared-file hunks hand-applied** (branch diff worked through selectively; never `git apply`):

- `include/item_menu.h` — `#include "swsh_item_menu.h"`; the `ITEMWIN_*` additions; the
  `enum BattlePocket` / `BAG_POCKET_IDS_COUNT` block; `POCKETS_COUNT` → `BAG_POCKET_IDS_COUNT` on the
  four cursor/scroll/count arrays; `isPyramid`; the three-buffer `tilemapBuffer` split; the
  `struct BagMenu` tail. Taken whole — our copy of this header was byte-identical to upstream.
- `src/item_menu.c`, `src/battle_pyramid_bag.c` — the `#if !…` wrappers plus a header comment each.
- `src/item_use.c` — both interception hunks, taken *with* their `#if SWSH_ITEM_MENU_IN_BAG_USE` /
  `#if SWSH_ITEM_MENU_IN_BATTLE_USE` guards, so they are inert today. Kept (rather than dropped like
  the battle-engine pair) because this is the file where the routing decision is made and the guard
  reads correctly here: `item_use.c` already includes `item_menu.h`, which now includes
  `swsh_item_menu.h`, so there is no include-order trap. With the flag off the emitted code is
  semantically identical to before (`inPyramid` is just the old condition, inverted and named).
- `include/battle.h`, `src/battle_script_commands.c` — **skipped**, see above.

**Behaviour deltas accepted consciously** (player-visible, no automated gate sees any of them):

1. **In battle the bag shows four battle pockets, not the field pockets** — Medicine / Poké Balls /
   Battle Items / Berries, assembled into `gBagMenu->battlePocketRefs`. A key item you could
   previously scroll to mid-battle is not in that list.
2. **The Battle Pyramid bag is now the SwSh bag** with a 10-slot scratch pocket
   (`gBagMenu->pyramidScratch`), sharing `gBagMenu`'s allocation.
3. **Use / Give still leaves the bag and opens the party menu**, i.e. exactly the pre-port flow. On
   an unmodified `dev_bag_menu` it would not — this is the deliberate deviation, not a bug.
4. Berry stat / berry tag pages stay off (`SWSH_ITEM_MENU_BERRY_STAT = FALSE`), upstream's own
   default.

---

### 3.6 `dev_battle_ui` — landed **unfinished, on purpose**

**Read this first.** Unlike §3.1–§3.5 this port was **not** finished before it landed. The owner's
call was "land it, then fix it", so the section closes with an explicit *known unfinished* list
instead of a clean bill of health. Everything here is reversible in one line:
`SWSH_BATTLE_UI = FALSE` in `include/config/swsh_ui.h`. That was verified, not assumed — the FALSE
build compiles and links clean at **26 566 568 B / 79.17% ROM, the pre-port figure**, while TRUE
costs 728 B (79.18%). EWRAM and IWRAM are unchanged either way (89.07% / 86.57%).

**Ref.** `refs/ports/dev_battle_ui` = `b2106afda6`. Base is expansion **1.14.1+8** (the branch's own
version header claims 1.14.2); our tree is **1.16.3-dev**, twelve tagged releases later. `patch
--dry-run` of the branch diff rejected **14 of 37 hunks (38%)** and fuzzed 5 more, so nothing was
applied mechanically — every hunk below was hand-placed or rewritten.

**Scope.** 49 files, +192/−78. No new `.c`/`.h`. The branch touches 41 assets: 38 replacements plus
3 new. We took **37 of the 38** — `mega_trigger.pal` was dropped because upstream deleted that file
(the palette now comes out of `mega_trigger.png`) — and all 3 new ones.

**Master toggle:** `SWSH_BATTLE_UI` in `include/config/swsh_ui.h`, per §1. There is no
`include/swsh_battle_ui.h` — the branch has no tuning block of its own, its knobs are ordinary
`#define`s inside `src/battle_interface.c` and `include/menu.h`, and they stayed there.

#### 3.6.1 Assets: copied, never overwritten

The branch **replaces** 38 files in `graphics/battle_interface/` in place. Doing that would have made
the toggle a one-way door, so every replacement was copied to
**`graphics/battle_interface/swsh/`** instead and selected at the `INCBIN`/`INCGFX` site. Same
pattern the message-box port used for `graphics/text_window/swsh/`.

| Selected in | Assets |
|---|---|
| `src/graphics.c` | `textbox.png` + `textbox_0.pal` + `textbox_map.bin`; the 12 changed entries of `gHealthboxElementsGfxTable` (`misc.4bpp` is the one the branch left alone and keeps the vanilla path); `ball_status_bar`, `ball_display`; the 5 `gHealthbox*Gfx` |
| `src/battle_interface.c` | `ability_pop_up.png` + `.pal`, the 4 `last_used_ball_*`, both `move_info_window_*`, new `swsh/category_icons.png` |
| `src/data/graphics/gimmicks.h` | the 5 `*_trigger.png`, `mega_indicator.png`, `dynamax_indicator.png` |

Every replacement PNG is **dimension-identical** to ours and differs only in colormap depth (8-bit vs
4-bit indexed), which `gbagfx` normalises. The healthbox palettes are byte-identical between old and
new art — the SwSh frames are a redraw inside the existing 16 colours, not a recolour.

One new build rule in `graphics_file_rules.mk`: `swsh/textbox.gbapal` is a `cat` of
`swsh/textbox_0.gbapal` + the **vanilla** `textbox_1.gbapal`, because the branch only restyled the
first of the two concatenated palettes.

#### 3.6.2 The four dead healthbox hunks — reimplemented, not ported

The branch was written against `AddTextPrinterAndCreateWindowOnHealthbox(str, x, y, bgColor, &winId)`,
which upstream **deleted**. Our tree blits into the sprite directly:
`FillSpriteRectColor(spriteId, …)` + `AddSpriteTextPrinterParameterized6(spriteId, FONT_SMALL, x, y,
…, sHealthBoxTextColor, …)`. These four are rewrites carrying the branch's *intent*:

| Branch hunk | Reimplemented as |
|---|---|
| `UpdateLvlInHealthbox`: y `3` → `IsDoubleBattle() ? 2 : 3` | `HEALTHBOX_TEXT_Y` macro (`src/battle_interface.c`), used at the `AddSpriteTextPrinterParameterized6` call |
| `PrintHpOnHealthbox` ×2: y `5` → `4` | `HEALTHBOX_HP_TEXT_Y` macro, applied to both the digit print and the `yOffset + 8` clear rect |
| `UpdateNickInHealthbox`: y `3` → `IsDoubleBattle() ? 2 : 3` | same `HEALTHBOX_TEXT_Y` |
| `color[1] = 1` → `6` | `sHealthBoxTextColor.foreground` |

**The clear rects had to move too, and the branch never did this.** Under the old API each print
allocated a scratch window, so raising the text raised its own background automatically. With sprite
blitting the clear rect is a separate hardcoded rectangle: leave it at `y = 5` while printing at
`y = 2` and the top row of the *previous* value survives. Both `FillSpriteRectColor` calls are now
expressed as `top = yPos + 2, height = 16 - (yPos + 2)` so they follow the text. At the vanilla
`yPos = 3` this is bit-identical to the stock `(…, 5, …, 11, …)`.

On `color[1] = 6`: taken, and it does make sense. Healthbox palette index 6 is `(82,106,98)`, a
slate that reads cleanly on the index-2 cream background; index 1 is near-black `(65,65,65)`. The
palette is identical between the two arts, so this is a pure restyle with no contrast risk. It also
explains the branch's `ability_pop_up.pal` note — it added a light tint at index 15 specifically so
the pop-up's shadow could "match name in healthbox".

#### 3.6.3 BG tile audit — the silent-VRAM-corruption fix

The branch hardcodes `#define SWSH_MOVE_DESC_WINDOW_BASE_TILE_NUM 0x21D`, i.e. vanilla
`STD_WINDOW_BASE_TILE_NUM 0x214` + 9. **Our `STD_WINDOW_BASE_TILE_NUM` is `0x21A`** (§ the
`SWSH_MESSAGE_BOX` port grew `MSG_BOX_TILE_COUNT` from 14 to 25). Taking `0x21D` literally would
have dropped the move-description frame **inside** the standard window frame — three tiles of
overlap, no warning, corrupt borders in whatever drew next. The branch author half-noticed: his own
trailing comment reads `// 0x223 for future reference`.

Fixed by **deriving** rather than hardcoding, in `include/menu.h`:

```
#define STD_WINDOW_TILE_COUNT               9
#define SWSH_MOVE_DESC_TILE_COUNT           10
#define SWSH_MOVE_DESC_WINDOW_BASE_TILE_NUM (STD_WINDOW_BASE_TILE_NUM + STD_WINDOW_TILE_COUNT)
```

which evaluates to `0x223` today and follows `STD_WINDOW_BASE_TILE_NUM` automatically if
`SWSH_MESSAGE_BOX` is ever flipped.

**The audit.** Battle BG0 is the only BG that sees this tile. From `sStandardBattleWindowTemplates`
(`src/battle_bg.c`), the window `baseBlock` runs are:

| Range | Owner |
|---|---|
| `0x020`–`0x0F7` | VS windows, `B_WIN_MSG` |
| `0x100`–`0x191` | level-up box, level-up banner |
| `0x190`–`0x1F7` | action menu, action prompt |
| **`0x1F8`–`0x28F`** | **free — this is the frame arena** |
| `0x290`–`0x2CF` | PP, PP remaining, move type, switch prompt |
| `0x300`–`0x34F` | the four move-name windows |
| `0x350`–`0x3BB` | `B_WIN_MOVE_DESCRIPTION` body (18×6) |

Inside the arena: message box `0x200`–`0x218` (25 tiles), standard window `0x21A`–`0x222` (9),
**move-desc frame `0x223`–`0x22C` (10)**. Next occupied tile is `0x290`. Headroom after the new
frame: **99 tiles**. No collision.

Note **10**, not 12. `LoadSwShMoveDescBoxGfx` upstream loads `0x180` bytes = 12 tiles, but
`move_desc_box.png` is 80×8 = **10 tiles** and its tilemap references indices 0–9 only. The upstream
call read 64 bytes of whatever `.rodata` followed the asset and wrote it into VRAM as two junk tiles.
Fixed to `TILE_OFFSET_4BPP(SWSH_MOVE_DESC_TILE_COUNT)`.

#### 3.6.4 Type icons vs the redrawn healthbox — analysed, one slot corrected

Our tree runs `B_SHOW_TYPES = SHOW_TYPES_ALWAYS`. The branch author shipped `SHOW_TYPES_NEVER` and
**never saw his layout with type icons on**, so this had to be derived rather than trusted.

Method — all static, no emulator:

- Type icons are 8×16 sprites at `sTypeIconPositions[position][isDoubles]` (`src/type_icons.c`),
  created with `CreateSpriteAtEnd(…, UCHAR_MAX)` → **subpriority 255**, versus the healthbox's
  subpriority 1 at the same OAM priority. **Type icons render behind the healthbox**, so overlap
  means *hidden*, not *garbled*.
- They also **slide** ±10px from the table value before resting (`GetTypeIconSlideMovement`), and the
  second type sits `+11px` below the first (`SetTypeIconXY`). Both were included.
- Healthbox sprite origins come from `sBattlerHealthboxCoords`; the player-singles box is forced to
  `ST_OAM_SQUARE` with size 3 = 64×64, the rest are 64×32, and the right half is pinned at `x + 64`
  (`SpriteCB_HealthBoxOther`). That maps the whole 128px-wide PNG onto a known screen rectangle, so
  the icon rectangles can be expressed in **image pixel coordinates** and compared against the actual
  non-transparent span of each row of the art.

Result (px of the 8px-wide icon covered by opaque frame):

| Slot | vanilla art | SwSh art | verdict |
|---|---|---|---|
| singles player left | 3px, rows 22–33 | 3px on the top row only, then 2/1/0 | **better** |
| singles opponent left | 1px | 0–2px | same |
| doubles player left / right | up to 8px on the lower icon | up to 3px | **better** |
| doubles opponent left / right | 2px on the upper icon | **6px** on the upper icon | **regression** |

The SwSh frame is a right-leaning parallelogram: its widest row is at the *top*, exactly where the
first type icon sits on the opponent's doubles box. Widest opaque column there is image column 96;
the icons start at column 91. So `sTypeIconPositions` gains a `+6` on the two doubles-opponent rows
only, behind `SWSH_OPPONENT_DOUBLES_TYPE_ICON_X_SHIFT`, which is `0` when `SWSH_BATTLE_UI` is off.
The other four slots are the same or better than vanilla and were left untouched — moving them would
be a regression against a layout the owner already reads fine.

**Confidence: high.** The geometry is fully determined by constants, and the one loose end was
closed by measuring the icon sheets: every glyph in `graphics/types/battle_icons1.png` and
`battle_icons2.png` fills its 8×16 cell edge to edge (x-extent 0–7 for all 20), so "6px covered"
means 6 of 6 visible pixels, not 6 of a padded 8. `ShouldFlipTypeIcon` only hflips, which does not
change the bounding box. The remaining unknown is `GetTypeIconBounceMovement`, which adds the
healthbox's `y2` during switch-in — a transient vertical wobble that does not change the ordering of
the table. **Playtest item #1 is still a doubles battle with a two-type opponent on the left.**

#### 3.6.5 Deliberately not taken

| Hunk | Why |
|---|---|
| `include/config/battle.h` (`B_LAST_USED_BALL_BUTTON` R→L, `B_FLAG_DYNAMAX_BATTLE`, `B_FLAG_TERA_ORB_NO_COST`) | the author's personal config. Ours is deliberate. |
| `include/constants/flags.h` (`FLAG_UNUSED_0x264/0x265` → `FLAG_DYNAMAX_ENABLED`/`FLAG_FREE_TERA`) | those two ids are **already claimed** by `FLAG_OVERHAUL_NO_WILD_ENCOUNTERS` / `FLAG_OVERHAUL_NO_TRAINER_SEE` in REGISTRY.md §1. Direct collision. |
| `src/battle_gimmick.c` trigger positions | branch moves `SINGLES_GIMMICK_TRIGGER_POS_Y_DIFF` −11 → −10 and X 30/31 → 35/36. Upstream re-tuned these after 1.14.2; ours are −5 / −2. The deltas are meaningless against our baseline, so nothing was applied. **This is a real open item** — the gimmick trigger has not been re-positioned for the new art at all. |
| `graphics/battle_interface/mega_trigger.pal` | file deleted upstream. The palette now comes from `mega_trigger.png`, and `swsh/mega_trigger.png` carries it. |
| the three `battle_controller_player.c` comment-outs | see §3.6.6. |
| `TAG_CATEGORY_ICONS 30004` | see §3.6.7. |

#### 3.6.6 The action prompt was kept

Branch commit `62d1ff284a` "DIsable action prompt texts due to updated battle text box frame"
comments out `BattleStringExpandPlaceholdersToDisplayedString(gText_WhatWillPkmnDo)` and **both**
`BattlePutTextOnWindow(…, B_WIN_ACTION_PROMPT)` calls. That deletes "What will X do?" *and* the
`B_SHOW_PARTNER_TARGET` doubles prompt, which is live gameplay information. **Not taken** — all three
calls are intact.

The author's stated reason is that the new textbox frame conflicts with the prompt, and he may be
right: `swsh/textbox_0.pal` restyles palette 0 (index 5 white → `38,35,35`, index 15
`106,164,164` → `54,53,52`) and `B_WIN_ACTION_PROMPT` draws in palette 0. **If the prompt turns out
to be unreadable or clipped against the new frame, the fix is the frame or the text colour, not
deleting the string.** Playtest item #2.

#### 3.6.7 Category icon tag

The branch defines `#define TAG_CATEGORY_ICONS 30004` in `src/battle_interface.c` and then creates
the sprite from `gSpriteTemplate_CategoryIcons`, which lives in `src/pokemon_summary_screen.c` and
hardcodes the same literal. It works today purely because the two literals match; renaming the
summary screen's tag would silently feed the battle sprite garbage tiles. It is also the same value
as `TAG_SHINY_ICON` in `src/swsh_summary_screen.c:841` (no runtime clash — different screens — but
still a coincidence waiting to bite).

Note the requested "just pick a distinct tag" **cannot** be done on its own: any other value needs a
matching template, because the tag is what binds sprite to sheet. So the port adds a self-contained
`gSpriteTemplate_SwShCategoryIcons` + sheet + palette + 3-frame anim table at
`TAG_SWSH_CATEGORY_ICONS 0xD7A0`, inside the battle tag block (`0xD6FF`–`0xD790`), and
`battle_controller_player.c` picks the template by toggle. ~20 lines, removes the cross-file
coupling, and the summary screen / Pokédex / move relearner keep the vanilla icons either way.

#### 3.6.8 Taken as-is (the genuinely good bits)

- **`PAL_STATUS_FRB`** — frostbite had been borrowing `PAL_STATUS_FRZ`'s colour, so FRZ and FRB were
  indistinguishable at a glance. Real information gain; the enum row is added **unconditionally** and
  both toggle paths give it a distinct colour.
- Gender symbols in their own colours on the healthbox.
- Exp bar halved and right-aligned (`B_EXPBAR_PIXELS 64 → 32`, `MoveBattleBarGraphically`
  right-aligns the shorter tile run inside the original 8-tile slot).
- Ability pop-up window widths, coordinates, text colours and the extra tile of left border.
- Last-used-ball / last-ball-window coordinates.
- Party summary balls +6px down (`bar_Y − 4` → `bar_Y + 2`, expressed here as
  `PARTY_SUMMARY_BALL_Y_OFFSET`). Upstream's own comment on that hunk says "down 3 pixels", which
  does not match its own code; the code is what was ported.
- Move-description panel: SwSh frame, two-tone body fill, `{COLOR_HIGHLIGHT_SHADOW 14 5 13}` on the
  CAT/PWR/ACC line, `B_WIN_COPYTOVRAM`, category icon at (39, 63).
- The branch's `acc_start` `0x6D` → `0x6C` hunk was a no-op: upstream already moved that literal to
  the equivalent `{CLEAR_TO 108}`.

#### 3.6.9 Gates

| Gate | Result |
|---|---|
| `make` with `SWSH_BATTLE_UI = TRUE` | clean. EWRAM 89.07%, IWRAM 86.57%, ROM 79.18% (26 567 296 B) |
| `make` with `SWSH_BATTLE_UI = FALSE` | clean. EWRAM 89.07%, IWRAM 86.57%, ROM 79.17% (26 566 568 B) — the pre-port figure |
| `make check TESTS="SaveBlock"` | 3/3 PASS |
| `make check TESTS="Cap Candy"` | 1/1 PASS |
| `make check TESTS="Endless Candy"` | 1/1 PASS |
| `make check TESTS="Nuzlocke blocks"` | 1/1 PASS |
| `make check TESTS="Full multi"` | 2 PASS + 1 KNOWN_FAILING, as expected |
| `make check TESTS="Battle"` | 23 PASS + 1 TO_DO, 0 FAIL |
| `make check` (full sweep) | **5507 tests: 4871 PASS, 15 FAIL, 14 KNOWN_FAILING, 7 EXPECT_FAILING, 3 ASSUMPTION_FAIL, 597 TO_DO.** All 15 FAILs pre-existing — see below |

**On the full sweep.** `test/battle/front_anim.c:5` ("Front anims work") **wedges the runner** — it
TIMEOUTs on an in-ROM `src/malloc.c:97` `block->magic == MALLOC_SYSTEM_ID` assertion and the hydra
retries it while climbing past 4 GB RSS, so nothing after it ever reports. This is pre-existing and
was flagged before the port started; the sweep was completed with that one file moved aside and
restored afterwards.

The only red `FAIL`s in the sweep are **15 pre-existing, data-driven trainer tests** — the
`Trainer Party Pool` group, the eight `Difficulty changes which party is used…` cases, and
`CreateNPCTrainerPartyForTrainer generates customized Pokémon`. They assert on fixed trainer ids:
`trainer_control.c:28` expects trainer **3** to be a Wobbuffet with a Master Ball, Telepathy and
friendship 42, but trainer 3 in this tree is `TRAINER_GRUNT_AQUA_HIDEOUT_2` — the overhaul's own
`f4d2428f9a` ("Overhaul Phases 0+1") rewrote `src/data/trainers.party`. Nothing in this port touches
trainer data, party generation or the difficulty system. Everything else is PASS / TO_DO /
EXPECT_FAILING (harness self-tests) / the 14 long-standing KNOWN_FAILINGs / three ASSUMPTION_FAILs
(`test/text.c:566` "Map names fit in popup", `test/battle/exp.c:106` and `:168`).

**No new failure was introduced.** Independently of the sweep: `grep -rl` over `test/` finds **zero**
references to any symbol this port changes (`B_EXPBAR_PIXELS`, `sTypeIconPositions`, `HEALTHBOX_*`,
`PAL_STATUS_*`, `SWSH_MOVE_DESC_*`, `CategoryIcons`, `SWSH_BATTLE_UI`), and the test runner is
headless, so the port's surface is invisible to the harness by construction. **That cuts both ways:
`make check` can never catch a regression in this port. Only playtesting can.**

#### 3.6.10 Known unfinished — iterate here

Ordered by how likely they are to actually bother you:

1. **Doubles opponent type icons** — the `+6` shift above is derived, not seen. First thing to look
   at in a real doubles battle.
2. **The action prompt vs the new textbox frame** (§3.6.6). If "What will X do?" looks wrong, fix the
   art or the palette; do not delete the call.
3. **Gimmick trigger position was never ported** (§3.6.5). Mega/Tera/Dynamax trigger sits at our
   1.16.3 coordinates on top of a 1.14-era redesign. Almost certainly needs a nudge.
4. **Safari healthbox** — `healthbox_safari.png` was replaced, but `CreateSafariPlayerHealthboxSprites`
   / `UpdateSafariBallsTextOnHealthbox` (`src/battle_interface.c`) still print at hardcoded `y = 3`
   and `y = 19` and were never re-tuned for it. Safari Zone only.
5. **Move-description frame vs `B_MOVE_DESCRIPTION_BUTTON`** — `WindowFunc_DrawSwShMoveDescFrame`
   assumes `B_WIN_MOVE_DESCRIPTION`'s exact 18×6 geometry (`tilemapLeft − 1` … `+ width + 3`). Any
   change to that window template needs the frame function revisited.
6. **`B_SHOW_EFFECTIVENESS = SHOW_EFFECTIVENESS_ALWAYS`** replaces the PP string with the
   effectiveness indicator in `B_WIN_PP`. That window is unchanged by this port and was not visually
   checked against the new frame.
7. The exp bar right-align was verified by reading the tile arithmetic (tiles 4–7 of the original
   8-tile run, i.e. image x 64–95), not by eye.

---

## 4. Pending branches — pre-merge drill

For **storage**: DONE, see §3.3.

For **party**: DONE, see §3.4. Rows 6–11 no longer exist (Cap Candy deleted); rows 12–13 live at
`src/swsh_party_menu.c` L7512 / L10568.

For **bag**: DONE, see §3.5. Rows 12–13 are protected by *routing*, not by a new guard —
`SWSH_ITEM_MENU_IN_BAG_USE = FALSE` keeps item use on the party-menu path where the guards already
are. The unguarded in-bag copies at `src/swsh_item_menu.c` L7046 / L7062 are dead code today and are
the first thing to fix if that flag is ever flipped back on.

For **battle UI**: LANDED BUT UNFINISHED, see §3.6 — and §3.6.10 in particular. This is the only
ported branch that is not "done"; treat `SWSH_BATTLE_UI` as work in progress, not as settled art.
Nuzlocke exposure is nil (the port touches no gate, no save field and no item path), but it does
touch two things the owner reads every turn: the always-on type icons (§3.6.4) and the
"What will X do?" prompt (§3.6.6).

**All six SwSh branches are now landed.** There is no pending SwSh UI port.

For **all**: re-read §2.4. Anything touching `struct BoxPokemon` or `enum MonData` is a
save-format change — run `make check TESTS="SaveBlock"` and expect 3/3.
