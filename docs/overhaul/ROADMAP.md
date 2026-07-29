# Overhaul Roadmap — Randomized Nuzlocke QoL System

**Base:** pokeemerald-expansion 1.16.3-dev (`include/constants/expansion.h`).
**Investigated:** 2026-07-28 by 13 research agents (full reports in [investigations/](investigations/)).
**Status:** awaiting owner decisions (section 3), then build begins per section 5.

---

## 1. TL;DR

Roughly **half the wishlist already exists in the expansion as config switches** — verified flag/config names below, most are one-line flips. The real engineering is five modules: **nuzlocke engine, tracker UI, fair randomizer, knowledge panels, quiz NPCs** — and for every one of them a community donor branch or in-repo primitive exists, so nothing is built from scratch. The architecture is settled: one `include/config/<feature>.h` per module (repo idiom), persistent state in SaveBlock3 (1620 of 1624 bytes free) guarded by existing `STATIC_ASSERT`s, scripted state in the 381 unused event flags, OOP-ish const-vtable interfaces modeled on `struct TestRunner` (`include/test/test.h`).

---

## 2. Quick wins — config flips, zero or near-zero code

| Goal | Switch | Where | Note |
|---|---|---|---|
| Instant text | `TEXT_SPEED_INSTANT` → TRUE (or assign `FLAG_TEXT_SPEED_INSTANT` for a toggle) | `include/config/text.h` | 4th "Instant" options value exists but isn't drawn in the menu |
| Auto-advance dialog | `AUTO_SCROLL_TEXT` → TRUE | `include/config/text.h` | Global; A/B still advances early |
| Faster battles | lower `B_WAIT_TIME_MULTIPLIER` (16 → ~8), `B_FAST_INTRO_NO_SLIDE` → TRUE | `include/config/battle.h` | HP/EXP fast drain already on |
| Wild encounters OFF toggle | assign `WE_FLAG_NO_ENCOUNTER` a `FLAG_UNUSED_*` id | `include/config/wild_encounter.h:6` | Runtime-toggleable; fishing/Sweet Scent deliberately bypass (good for nuzlocke) |
| Trainer sight-battles OFF | assign `OW_FLAG_NO_TRAINER_SEE` | `include/config/overworld.h:111` | Trainers still battle when talked to |
| Type effectiveness on move select | `B_SHOW_EFFECTIVENESS` — already ON (SEEN) | `include/config/battle.h:402` | Flip to ALWAYS for randomizer (see conflict C9) |
| Enemy type icons in battle | `B_SHOW_TYPES` → SHOW_TYPES_ALWAYS | `include/config/battle.h:396` | |
| HGSS Pokédex (stats/moves/evos pages) | `POKEDEX_PLUS_HGSS` → TRUE | `include/config/pokedex_plus_hgss.h` | The in-game-knowledge anchor; has dark mode |
| IV/EV pages on summary | `P_SUMMARY_SCREEN_IV_EV_INFO` → TRUE | `include/config/summary_screen.h` | Relearner-from-summary already on |
| Level/EXP caps (nuzlocke) | `B_EXP_CAP_TYPE`, `B_LEVEL_CAP_TYPE`, `B_RARE_CANDY_CAP` | `include/config/caps.h` | Badge-keyed via `sLevelCapFlagMap` (`src/caps.c`) |
| Gym doubles | `Battle Type: Doubles` per trainer | `src/data/trainers.party` | No script changes needed; wilds stay singles by default |
| Smarter gym AI | `AI: Smart Trainer / Prediction` per trainer | `src/data/trainers.party` | Gym leaders currently only have `Basic Trainer`! |
| Difficulty variants | `B_VAR_DIFFICULTY` + `Difficulty: Hard` team blocks | `include/config/battle.h:272` | Per-trainer Hard teams, Normal fallback |
| DS-style mon sprites, all-gen move anims, modern item icons | already default | `include/config/pokemon.h` | 947/1390 sprite sets animated; Gen 8/9 partly static (upstream #5883) |
| Day/night tint | `OW_ENABLE_DNS` — already ON | `include/config/overworld.h:102` | Per-map lamp lighting is separate manual work |
| Follower Pokémon | `OW_FOLLOWERS_ENABLED` → TRUE | `include/config/overworld.h:61` | Optional; cutscene edge cases |
| BW2 map popups | `OW_POPUP_GENERATION` → GEN_5 | `include/config/overworld.h` | Alpha-blend conflict with DNS noted in dns.md |
| Skip RHH splash | `EXPANSION_INTRO` → FALSE | `include/config/general.h:76` | |
| Reusable TMs, indoor running, last-ball R-hotkey, catch-swap | mostly already on | `include/config/item.h`, `overworld.h`, `battle.h` | See ws_dialog-speed.md for the full QoL sweep |
| New-game skip (dev) | `ENABLE_QUICKSTART` — already ON; debug "Cheat start" | `include/config/quickstart.h`, `data/scripts/debug.inc:11` | Both compiled out on RELEASE builds — see decision D8 |

**Caveat:** Quickstart and all debug menus are `DISABLED_ON_RELEASE`. Either ship dev builds or strip the guards.

---

## 3. Owner decisions (blocking — everything else is scheduled around these)

> **Status update 2026-07-28 (v5): ALL decisions D1-D8 and the full nuzlocke rules sheet are DECIDED.** (v5 corrections: first legal encounter consumes the slot win-or-lose; dupes uncatchable and slot-neutral; whiteout is just mass permadeath, continue from storage; wager battles allow fleeing at the cost of the stake.) D2/D8: dev builds, merge upstream patch tags between runs only, personal-use project. D5: cut ALL arc content including the Steven Space Center battle. D6 proceeds on the recommendation (FREE_* flips day one, tracker in SaveBlock3, no DexNav search levels). D7: fully specified in §7.3. Only pending review: the hand-authored route→gym claim table (§7.9) — reviewed when authored in Phase 4. **Roadmap is build-ready.**

### D1. Randomizer path — FOUNDATIONAL
External UPR is **eliminated on evidence**: it reads vanilla Gen-3 ROM layouts, the expansion rewrites them all while keeping the same game code (BPEE), so UPR would corrupt the ROM. Its fairness algorithms (similar-BST ±10% widening bands, type themes, legendary limits) survive as the spec. Remaining options:

| | Fairness control | Effort | New-run convenience | Nuzlocke compat |
|---|---|---|---|---|
| **B: in-engine seeded** (hook `CreateWildMon`, seed in save) | 5 | L | 5 — new seed = New Game | 5 |
| **C: build-time Python** over `wild_encounters.json` + `trainers.party` | 5 | M | 3 — rebuild per run | 5 |
| Hybrid: C for trainers, B for wilds | 5 | M-L | 4 | 5 |

**Recommendation (med confidence): start with C, design its logic so it can migrate into B later.** You already build from source; a rebuild per run is one command. Strongest counterargument: if you run frequently, B's "new seed = new game, no PC needed" wins long-term — hence keep the mapping logic path-agnostic.
**Scope (high confidence): randomize wilds + route trainers only; gyms/E4 exempt** — gyms use Trainer Party Pools (curated 10-15 mon pools, roles tagged, per-run picks) so they stay coherent AND vary per run. Full detail: [investigations/ws_randomizer-fairness.md](investigations/ws_randomizer-fairness.md).

> **DECIDED (v2):** build-time script path. Randomize: wild species, items, and quiz-NPC question assignment. Do **NOT** randomize abilities or movesets (`B_TRAINER_MON_RANDOM_ABILITY` stays FALSE). Gyms/E4 exempt — replaced by run-aware generation, see §7.9.

### D2. Upstream merge policy
(a) merge every patch tag (~monthly), (b) merge 90-day minor tags only, (c) freeze at 1.16.x.
**Recommendation (med): (a) for patch tags — they're mostly battle/AI bugfixes you want — but NEVER mid-run** (upstream sometimes adds SaveBlock3 fields, which bricks saves). Pin the commit for each run's duration.

### D3. VGC depth for gyms
Everything up to bring-6-pick-4 is config/authoring on existing systems (`Battle Type: Doubles`, pools with species/item clauses, `AI: Double Battle / Know Opponent Party`). The genuinely new pieces: player pick-4 UI (fisham branch, near-drop-in on 1.16.2), open team preview screen (only donor needs permission or a rewrite), flat-level normalization (no prior art).
**Recommendation (med): phase it** — doubles + pools + clauses + AI flags now; fisham pick-4 next; team preview cheap version (Mugshot VS-card + `AI_FLAG_KNOW_OPPONENT_PARTY`) and skip flat-level (level caps already normalize in practice). Counterargument: pick-4 ritual adds ~30s per attempt, against your time-saving goal — you can drop it later if it grates.

> **DECIDED (v2):** gyms only: doubles + bring-6-pick-4 + FULL mutual team preview — player sees the opponent's 6 before picking, AI sees the player's 6 (`AI_FLAG_KNOW_OPPONENT_PARTY`). So the real preview screen (verdant port or template rebuild) is IN scope, not the cheap version.

### D4. UI overhaul depth
(a) config-QoL only, (b) coherent SwSh suite by Montblanc (summary/party/bag/PC/messagebox — 1.16.2-based, merges nearly clean), (c) mix-and-match best-of-breed (needs a consistency pass).
**Recommendation (med): HGSS dex + config flips in phase 1; adopt the Montblanc SwSh suite as its own later phase** (one style, one author, one credit, fewest conflicts). Battle UI: keep expansion default first; pollythadon's BW UI (1.16.2) is the safest reskin if wanted later.

> **DECIDED (v2):** full overhaul wanted — config flips now, Montblanc SwSh suite as its own phase, plus better trainer/NPC sprites, battle backgrounds, and overworld assets (Platinum pack + Modern Emerald backgrounds).

### D5. Story-cut granularity
(a) full arc-complete at New Game, (b) neutralize the 6 roadblocks only (hideouts stay as optional dungeons), (c) hybrid: keep ONE beat — the Mossdeep Space Center **Steven multi-battle** (it's a doubles fight; fits the VGC theme).
**Recommendation (low-med): (c).** Also decide climax handling: pre-resolve Kyogre/Groudon/Rayquaza vs keep Sootopolis as the one surviving arc before gym 8.

### D6. Save layout — decide BEFORE the first real run
- Flip `FREE_*` flags in `include/config/save.h` day one (+3790 B reclaimed; kills Mystery Gift/e-reader/union-room/link-records — all dead in single-player; `FREE_MATCH_CALL` also kills rematch data — keep match call if you want rematches).
- Tracker + nuzlocke state → SaveBlock3. **Mutually exclusive with DexNav search levels** (`USE_DEXNAV_SEARCH_LEVELS` alone costs ~1500 B). Recommendation: skip search levels; DexNav itself doesn't need them.
- Any SaveBlock change mid-run bricks the save. Layout freezes at run start.

### D7. Nuzlocke rules sheet (settings menu defaults)
Dead mon: party-flagged / **auto-graveyard-box** / released · First-encounter miss: slot burned vs retry-until-catch · Dupes: species vs **evolution line** vs dex-caught · Statics/gifts/roamers/Safari policy · Shiny clause on/off · Level cap: **hard** vs soft · Whiteout: run over vs auto-disable+autosave · Wild doubles: keep off. Every one is cheap — the donor design tracks encounter and catch bits separately, so these become menu options rather than forks.

### D8. Build mode
Dev build (keeps Quickstart, debug menus, Cheat start, AI score viewer for free) vs release build with guards stripped. **Recommendation (high): dev build** — this is a personal project; the debug tooling is your QA harness.

---

## 4. Conflict register (cross-workstream — the gap-check)

- **C1 Randomizer × gym AI teams:** randomized gym parties destroy curated sets. Resolved by D1 scope: gyms exempt, use pools.
- **C2 Encounters-off × nuzlocke first encounter:** you still want one encounter per route. Policy options: manual toggle, auto-enable on maps with unused slot (small cross-module hook), or encounters permanently off with deliberate capture via fishing/Sweet Scent (which bypass the flag by design).
- **C3 Skip-all dialog × quizzes/choices:** structurally safe — menus poll `JOY_NEW` only and item-gets block on `waitfanfare`, so held buttons can't auto-confirm. Add a per-textbox debounce to hold-skip so quiz questions can't blast past unread.
- **C4 SaveBlock3 contention:** tracker (~560 B) + nuzlocke state vs DexNav search levels (~1500 B, doesn't fit) vs NPC followers vs `OW_SHOW_ITEM_DESCRIPTIONS=FIRST_TIME`. Architecture keeps a byte ledger; D6 resolves.
- **C5 Flag/var land-grab:** ≥5 modules want `FLAG_UNUSED_*`/`VAR_UNUSED_*` ids. A central registry file (`include/constants/overhaul_flags.h`, reserved ranges per module) is Phase-0 work.
- **C6 Summary reskin × weakness panel:** both rewrite `src/pokemon_summary_screen.c`. Sequence: choose the screen (D4) before building the panel — or put the panel in the HGSS dex (merge-safest).
- **C7 Doubles gyms × permadeath:** 1 usable mon → auto 1v2 (near-hopeless). Gate gym doors with `HasEnoughMonsForDoubleBattle` or accept. Also: pick-4 party backup must restore BEFORE whiteout logic or mons vanish — fisham patched exactly this; test with nuzlocke death rules.
- **C8 Randomizer × knowledge accuracy:** Pokédex area/evo data and SEEN-gated indicators lie under randomization unless they read runtime data. Cheap fixes: `B_SHOW_EFFECTIVENESS/B_SHOW_TYPES` → ALWAYS; dex-area page reads the remap or gets hidden.
- **C9 Story cut × XP curve:** ~15 removed Aqua/Magma fights = a mid-game XP hole. Level caps absorb most of it; gym team levels must be authored against the post-cut curve. Owner of this: the AI/teams phase.
- **C10 DNS lamp lighting × map edits:** per-map lighting requires reverting upstream commit `a5b079d833` — do it before any other map edits, or skip lighting.
- **C11 Quiz rewards × nuzlocke economy:** repeatable rewards = free-candy faucet. Use one-time flags or `DAILY_FLAGS`.

**Sequencing constraints:** save layout final before run 1 (D6) · randomizer species-remap must run BEFORE nuzlocke marking/dupes hooks in `CreateWildMon` · reskin decision before weakness panel (C6) · lighting before map edits (C10) · upstream merges never mid-run (D2).

---

## 5. Build order

| Phase | What | Effort | Depends on |
|---|---|---|---|
| 0 | **Scaffolding:** flag/var registry, SaveBlock3 ledger, `FREE_*` flips, config header stubs, CI trim to Emerald-only, CREDITS.md skeleton, module-interface example | S | D6, D8 |
| 1 | **Config-flip wave:** everything in section 2 + hold-skip patch (~25 lines in `src/text.c` `SetResultWithButtonPress`) | S | — |
| 2 | **Story cuts:** New Game preset script (gate map fully verified in ws_story-cuts.md), start-at-starter warp, compensation items | M | D5 |
| 3 | **Nuzlocke engine:** port NecroDingo branch design (1.13.3 donor, hooks re-applied by hand), + auto-graveyard sweep, settings menu, gift/static policy, revive gating, DexNav/OWE hook coverage | M | D7, phase 0 |
| 4 | **Randomizer:** per D1 — seeded mapping + leak-path sweep (DexNav, Feebas, outbreaks, scripted statics) | M-L | D1, phase 3 hook order |
| 5 | **Tracker UI:** SaveBlock3 records, route list + graveyard + run stats via `list_menu`, start-menu entry (DexNav precedent) | M | phase 3 |
| 6 | **Gyms:** Hard-difficulty doubles pools per leader (Smogon-set converter script), AI flags, doubles wiring incl. rematches, pick-4 if D3 says so | M | D3, phases 2-4 |
| 7 | **Knowledge builds:** weakness/resistance panel (from `gTypeEffectivenessTable` + `GetOverworldTypeEffectiveness`), recommendation page (pure-data scorer — NOT battle-AI reuse; verified hard-bound to live battle state) | M | C6/D4 |
| 8 | **Quiz NPCs:** `dynmultichoice` driver + generators from type/move/species tables (auto-derived content tracks your balance edits), Poryscript optional | M | phase 1 |
| 9 | **UI/visuals:** per D4 — SwSh suite or config-only; Platinum trainer/OW sprite pack; battle backgrounds | L | D4 |

Phases 5-8 are largely parallelizable after 3-4 land.

---

## 6. Key reuse sources (credit everything in CREDITS.md; ask-permission flags noted)

| Source | Feeds | Status |
|---|---|---|
| NecroDingo `nuzlocke-challenge` branch (+ devolov's pret-wiki tutorial it's based on) | nuzlocke engine | patch-donor; credit both, courtesy ask |
| TheXaman `tx_randomizer_and_challenges` / resetes12 Modern Emerald | tracker storage scheme, challenges menu, catchable-indicator | vanilla-based, cherry-pick; ask re: indicator art |
| fisham-org feature branches (select-mons, battle-mode-toggle, level-scaling) | pick-4 UI, doubles toggle | 1.16.2, near-drop-in; level-scaling needs a test pass (author-disclosed AI-assisted) |
| Montblanc SwSh suite / pollythadon BW battle UI / miriamlefae start menu | UI overhaul | 1.16.1-1.16.2, cleanest UI donors |
| grunt-lucas `sample-ui` | scaffold for every new screen | template, copy freely w/ credit |
| @pkmn/smogon JSON (data.pkmn.cc) | gym team sets (trainers.party IS Showdown syntax) | MIT code / Smogon data, credit |
| Run & Bun AI doc | AI tuning target | reference only |
| Emerald Rogue (Pokabbie) | design reference: run lifecycle, query engine, hub start | XL to port — read, don't lift, without permission |
| pret wiki Tutorials + Team Aqua's Asset Repo wiki | dozens of S-effort diffs + branch index | free w/ credit |
| Platinum OW/Trainer pack + DS-style 64x64 sprite repo (PokeCommunity) | human sprites, missing anim frames | read thread rules manually (403 to bots) |

Known upstream AI issues that hit VGC gyms directly: #10135 (doubles AI over-targets KOs), #10259, #10276 — argument for D2(a).

---

## 7. Owner refinements v2 (2026-07-28)

### 7.1 Register bag (S-M)
The register button opens a mini-bag with **6 registerable slots**: Porta-Heal (party heal key item — nuzlocke balance note: consider a cooldown/charge system so attrition still matters), Repellent (= the `WE_FLAG_NO_ENCOUNTER` toggle item), Cap Candy (§7.4), Mach Bike, Acro Bike, + 2 free slots. **Donor exists:** TheXaman registered-items menu, expansion-updated at iriv24's branch (see §6).

### 7.2 Dialog rules (S)
Auto-advance ON globally, but **quiz NPC questions are exempt** — quiz scripts clear `gTextFlags.autoScroll` per message (the per-script opt-out already exists; the global config just sets the default).

### 7.3 Nuzlocke encounter/tracker model (confirmed design)
- Route key = the on-screen location banner = `regionMapSectionId` (owner's instinct matches the code exactly — that IS the keying the tracker uses).
- **Once a route's encounter is used → ALL wild encounters auto-off on that route** (auto-set `WE_FLAG_NO_ENCOUNTER` per-map on entry). This resolves conflict C2: no manual toggling per route; the Repellent item is the manual override for unused routes.
- Tracker UI: per-route tick boxes (caught/used), graveyard, run stats.
- Dupes clause (v5): if the player **has the species or anything in its evo line** (party/PC), the encounter is a dupe — it does **not** consume the route slot AND **cannot be caught** (ball throws blocked); KO it or run freely, it simply doesn't count.
- Shiny clause: shinies never consume the slot but are always catchable/usable.
- **First-encounter rule (v5, hardcore):** the first LEGAL wild encounter (non-dupe, non-shiny) on a route IS the route's encounter, **win or lose** — catch it and you keep it; KO it or let it flee and the route counts as completed with nothing gained. The slot is consumed the moment the legal encounter resolves, whatever the outcome.
- **Whiteout (v7 — FINAL): party wipe = game over.** The classic nuzlocke rule, enforced: losing all party Pokémon ends the run (game-over screen, run marked failed). Storage Pokémon do not save you. Owner chose this partly because it's also the CHEAPER build — no empty-party forced-box-selection flow needed, just a game-over path on battle loss. (Supersedes v5 continue-from-storage.)
- **Statics/gifts (DECIDED): allowed, and each is its own unique encounter** — fossils, gift mons, static legendaries are free catches that do NOT consume any route's slot.

### 7.4 Cap Candy (S-M)
Rare Candy variant, improved over Randolocke's: **one use = instantly to the current level cap, in a single action — no per-level move-learn prompts interrupting** (Randolocke's version walked up level by level, stopping at every learnable move; owner explicitly wants that gone). Newly learnable moves are picked up afterwards via the summary-screen move relearner (already enabled). Must be usable on **PC-boxed mons** too. Builds on `B_RARE_CANDY_CAP` + `caps.c` `GetCurrentLevelCap()`.
**Repellant (v14 DECIDED):** keep OUR total on/off encounter switch (`FLAG_OVERHAUL_NO_WILD_ENCOUNTERS`, already built) — NOT Randolocke's weak-only Repel semantics. Where we've already built something better, ours wins.
**(v8) Cap Candy IS the leveling mechanism — the no-grind philosophy.** No Exp Share (owner declined it: pointless when Cap Candy exists). Set mode NOT forced (owner: nuzlocke is hard enough already; default battle style stays).

### 7.4b No-grind stat rules (v8)
- **Max friendship (255) at catch** for every mon — friendship evolutions (Crobat, Lucario, …) work immediately. Side effect: Return always max power (irrelevant — enemies never get TMs).
- **EVs auto-managed, badge-scaled, even spread (v9 DECIDED):** battle EV gain OFF (`EV_CAP_NO_GAIN`); every owned mon's EVs are auto-set to the current badge tier of the in-tree EV cap table (`src/caps.c` `sEvCapFlagMap`: 30→90→150→…→510 per badge, ÷6 per stat = 5→15→25→35→45→55→65→75→85), refreshed at catch and on badge earn (party + boxes). Enemies generated at the same tier — perfectly symmetric, zero grinding, zero choices.
- **HMs usable from the bag without teaching the move** (Phase 2; pret-wiki prior art) — owner: "fly without flying being a skill".

### 7.4c Economy (v8; delivery model corrected v10)
- **DECIDED (owner's words):** a woman NPC near the start gives 999 Master Balls — an NPC gift, not a silent bag grant.
- **PROPOSED, NOT CONFIRMED:** delivering the new key items (Porta-Heal, Repellent, Box Link) via NPCs; a renewable in-world source for Cap Candy; the general "content-over-code" rule. Owner must confirm each before these are built.
- **Gate-item manifest (v10):** every progression-relevant item (rods, bikes, Go-Goggles, HMs, keys, Devon Scope) is pinned at a known NPC — NEVER randomized — and listed in one manifest file that the randomizer and gym-team generator read for "obtainable before gym N" logic. Verified vanilla anchors: Old Rod = Dewford, Good Rod = Route 118, Super Rod = Mossdeep. Moving any gate NPC means editing the manifest, and pools/gates recompute.
- Money is deliberately irrelevant (max money via the same start NPC).
- **Rewards are never money**: quiz/wager reward tables pay in Pokémon, items, and TMs only.
- Starters: **randomized** — selection system TBD, owner wants a design discussion soon (before Phase 2's start-at-starter flow is built).

### 7.5 Key items (S each)
- **Box Link** — open the PC storage anywhere (swap party freely).
- Move Relearner: **already covered** — relearner from the summary screen moves page is ON by default (`P_SUMMARY_SCREEN_MOVE_RELEARNER`); a key-item wrapper is optional.
- Wall clock changeable **anytime** (small script edit on the clock interaction) so day/night is player-controlled.

### 7.6 In-battle enemy summary (M — new build)
From battle, open a summary for the enemy mon: **base stats + weaknesses/resistances**. No community prior art found in the investigation; build on the weakness-panel primitives (`GetOverworldTypeEffectiveness`, `gTypeEffectivenessTable`) + a trimmed summary layout. Pairs with `B_SHOW_TYPES` ALWAYS.

### 7.7 Summary screen rework (M)
- Replace the useless last page (contest data) — it becomes the **weakness/resistance panel**; more pages can follow (recommendation page lands here later).
- Stats display should show **final-evolution base stats** (for run planning); current-form stats stay on the IV/EV page. Multiple evolution options get their own page.
- **Learnset page (v6):** every mon gets a summary page listing its full level-up learnset — all moves it learns and at what level (data source: `GetLevelUpMovesBySpecies`; HGSS dex already renders this per species, reuse its list widget).

### 7.8 Wager battles replace hard optional trainers (M — new system)
Non-gym trainers that are "difficult" (forced doubles OR 4+ mons) become **opt-in wager NPCs**: they announce the reward up front, and the player must **stake 1 Pokémon** to accept. **DECIDED loss rule: handed over on loss** — lose the battle and the NPC keeps the staked mon (removed like an outgoing trade, regardless of whether it fainted; fainted teammates still die per normal permadeath). **Fleeing is allowed (v5):** running away forfeits the wager — staked mon handed over — but spares the rest of the team; cutting losses is a legitimate play. All other filler trainers/NPCs become quiz NPCs (type chart, weather effects, mechanics — teaching content). Overworld content model: **quiz NPCs (knowledge → reward) + wager trainers (risk → reward) + gyms (progression)**.

### 7.9 Run-aware gym team generation (M-L — replaces fixed gym teams)
Owner constraints (DECIDED): no repeated teams across runs, unknowable in advance, fair/no gimmicks, well-rounded, and **built only from what the player could have obtained before that gym in this run** — with these rules:

- **Route ownership:** the region's areas are partitioned among the 8 gyms — each gym "claims" the areas that open up on the way to it. Gym N's species pool = encounter tables of areas claimed by gyms 1..N (own claimed routes included). A gym may NEVER use species from a future gym's areas, even if sequence-breaking could reach them.
- *Proposed definition of "before the gym" (needs owner sign-off):* a hand-authored one-time table mapping every route/area to the gym whose badge progression it precedes — Hoenn's gate structure is fixed (even with arcs cut), so this is a static table the build script reads, not computed reachability.
- **Items too:** held items limited to items obtainable in areas claimed by gyms 1..N (marts + ground + gifts). **Gym 1: no held items at all.**
- **Team quality over role dogma:** individual mons MAY be 4x-weak — no per-mon bans; the constraint is the team as a whole is well-built (coverage, not 6 glass cannons, species+item clause).
- **Per-mon roll knobs (v17 DECIDED):** hidden abilities allowed ONLY where an Ability Patch is obtainable under the same claimed-area item rule (player can use found patches identically); enemy IVs RANDOM (like player catches — no flat 31 bosses); moves = the mon's genuine level-up learnset replayed to its level, keep the newest 4, sanity-fixed (≥2 damaging, ≥1 damaging STAB, swap earlier learnset moves in if needed), never TMs; evolved forms use their own learnset. **Player starters get forced near-perfect IVs (31s)** — Randolocke behavior the owner remembers and wants kept.
- **Team-building intelligence (v12 DECIDED): Option C — random with sanity rules only.** Gym teams are RANDOM draws from the gym's own pool (see own-pool rule below), with reroll-on-degenerate rules (not 6 of one type, not 6 glass cannons, no fully-evolved sweeper at gym 1, level/item rules) and NO offensive optimization. Difficulty comes from smart AI + doubles format, mirroring the both-sides-random fairness of UPR-style randomizers. Gyms 1-2 variance guard: **optional reward battles that grant a Pokémon** (via the wager/reward NPC system) give the player extra early team options.
- **Own-pool rule (v12): each gym's teams draw ONLY from its own claimed areas** — no cumulative inheritance (the player keeps cumulative access; gyms don't). Rebalanced partition proposed (gym 1 ~90 / 2 ~80 / 3 ~61 / 4 ~82 / 5 ~60 / 6-8 large but sampled) — awaiting owner confirmation in GYM_CLAIMS.
- **No TM moves for enemies (v6):** generated gym teams and randomized trainers draw movesets from **level-up learnsets only** — no TM/tutor/egg moves, ever.
- **E4/Champion (v11 — supersedes v7):** EXCLUDED from the availability rule — the E4 may use ANY species; their team system is a separate design (TBD with owner). Still tuned to the final cap (64) as the run's climax: five consecutive fights, deaths persist, player arrives attrited.
- Mechanism: the build-time randomizer script generates encounter tables first, then derives each leader's 10-15 mon pool from the claim table + those encounters, seeded per run; leader brings 6, picks 4 via TPP (`B_POOL_SETTING_CONSISTENT_RNG` prevents save-scum rerolls).

### 7.10 Movement speed setting (S)
Player-facing setting that increases overworld character speed (walk/run multiplier). Community prior art exists (run-speed tutorials on the pret wiki); lands with the Phase 2 QoL batch or the options menu.

*(Dropped by owner: speed/turn-order indicator; Move Relearner key item — summary-screen relearner covers it.)*

### 7.11 Bag sub-filters (M)
Second-level filter inside pockets (battle items / evo items / held items...). Check whether the Montblanc SwSh bag port already restructures pockets before building anything custom.

### 7.12 Species distribution (v15 — FINAL)
Rides on the ported tertu randomizer module (runtime, seed per save). Rules, in pick order:

1. **Habitat first**: every encounter table carries a hand-authored habitat tag (Field, Forest, Cave, Volcanic/Mountain, Desert, Ghost/Ruins, Industrial, Ice, Sea surface, Sea, Deep sea). Species classified by 3 layers: official Gen1-3 habitat data → written type/egg-group rules → manual override file.
2. **Then strength**: candidate must be within ±10% BST of the vanilla occupant (Randolocke's rule). Starved → widen BST band → then borrow from adjacent habitat (forest↔field, cave↔mountain, sea↔sea-surface). No slot may end empty.
3. **Guarantees (owner conditions):**
   - *Completeness*: every species has ≥1 habitat — enforced by a build-time check that fails loudly on orphans.
   - *Area diversity*: within one area's pool, all species distinct AND no type dominance — max ~1/3 of an area's slots may share a primary type (picker rerolls past the cap; exact ratio tunable).
4. Legendaries eligible when BST-close (Randolocke behavior); statics/gifts remain the guaranteed-legendary source. Vanilla slot odds kept.
5. **Starters**: three random options — each must evolve, final evolution BST ≥ **480**, current form in the starter strength band, and the trio shares no type. **All three offered starter lines are EXCLUDED from every gym's pool** (v16). E4 exclusion TBD with the E4 system.
6. **Above-540 policy (v16 DECIDED — option a):** the tier-8 ceiling stands; >540 species never spawn wild. They reach the player via evolution of banded catches, and legendaries via their dedicated channels (statics, map seller, battle rewards).

### 7.13 Run-aware evolution items — SHOP MODEL (v19 DECIDED, supersedes v18 loot model)
Owner's design: shops become the evolution-item source — they were useless (Porta Heal etc.) and now have a job.
- **Evolution items NEVER spawn as loot.** Removed from ground/hidden item randomization entirely.
- **Each area's nearby town shop stocks the evolution items** needed by species obtainable in that area's tier, from that tier onward. Species appears in the run → its item is purchasable nearby. Money is infinite by design, so evolving is deliberately easy — that's the point.
- Both guarantees carry over: no orphan stock (item only if its species is in the run), no locked evolutions (needed item always stocked by the right tier).
- Gym evolution-closure symmetric via the same rule: a gym may field an item-evolution only if a shop within its tier stocks that item.
- **AI never reads shops (v22 — supersedes v21, owner-verified logic):** shops are a player-facing VIEW derived from run data; the AI derives everything from the SOURCE data directly — its species pool (evolution closure: evolve freely if the evolver is in its claimed areas, item existence is implied) and the tier-legal item set (held items). Symmetry with the player is guaranteed by construction, since shop stock derives from the same facts. Rationale: reading shop inventory would create an init-order dependency (teams generated before stock = silently broken gyms) and let future shop hand-tweaks invisibly change gym legality. Sanity kept: species-specific items only on their matching species; gym 1 holds nothing.
- **Species-specific items follow the same rule (v20):** items that only serve one species/line (Thick Club, Light Ball, Leek, Metal/Quick Powder, Soul Dew, Griseous Orb, Ogerpon masks, Silvally memories, Booster Energy, etc.) never spawn as loot; they are shop-stocked near where their species is obtainable, only in runs where it exists. For legendaries/statics, the item stocks at the tier their event unlocks. Gyms may only hold such items under the same availability check.

### 7.14 Bag in battle (v19 DECIDED)
**Gym and E4 battles: bag fully disabled for BOTH sides** — player uses no items mid-battle, and gym trainers' AI-use items (vanilla "Items: Potion / Potion" lines) are stripped. Held items unaffected (clauses still apply). Porta Heal works only outside battle by nature. Other battle types (wilds, wagers) keep normal bag rules unless owner says otherwise.

### 7.15 The three-tier system (v23 DECIDED — powers wagers & rewards)
**Pokémon tiers** (judged by area-band BST + line potential): line reaches **480+** = POWERFUL · reaches **400+** = NORMAL · below = WEAK.
**Item tiers**: POWERFUL = VGC-grade (Choice items, Life Orb class) · NORMAL · WEAK. **Berries and TMs are categorized into the same three tiers** (one-time hand-authored table).
**Wager battle difficulty** (v24: levels relative to the AREA's gym-tier cap — a Route 110 wager scales to gym 3's cap forever, regardless of when you fight it; backtracked early areas stay easy by design):
- WEAK: 2 Pokémon at cap−4
- NORMAL: 4 Pokémon — 3 at cap−3, 1 at cap−2
- POWERFUL: 6 Pokémon — 1 at cap, 1 at −1, 2 at −2, 2 at −3
**UX**: talking to a wager NPC shows the reward and an accept/decline prompt BEFORE anything starts; **the stake must equal the reward's tier** (powerful reward = stake a powerful-tier mon). Reward mons drawn from the area's band/habitat, tiered by the line-potential rule.

### 7.16 Elite 4 — the Champion's Draft (v23 DECIDED)
- Pool: **any species** (no availability restriction), every mon's line-max BST ≥ 480 (the POWERFUL tier — "make sure they have powerful pokemon"), base power shaped by Randolocke's rule (±10% BST of their vanilla slots, whose occupants are already strong).
- **Draft order: Wallace picks first, then Drake → Glacia → Phoebe → Sidney; a species picked by one is unavailable to the rest.** The Champion drafts from the top of the pool downward.
- Levels at the final cap (64). Attrited-party climax as specced. **Battle format (v26 DECIDED): identical to gyms — doubles, bring-6-pick-4, mutual team preview — five consecutive times.** THE DESIGN BOOK IS CLOSED: every design decision in this roadmap is now resolved; remaining work is build phases 1-9.

### 7.17 Misc owner decisions (v23)
- **Norman pool fix — DECIDED (v25):** Petalburg Woods + Petalburg City transfer to Norman (gym 5). Own-pool sizes ≈ gym 1: 78 · gym 2: 75 · gym 3: 61 · gym 4: 82 · gym 5: 77 · gym 6: 219 · gym 7: 644 · gym 8: 346. The own-pool partition is now FINAL.
- **Quiz NPCs:** ALL filler NPCs become quizzes; questions may be static per NPC. Details deferred.
- **Legendaries:** copy Randolocke's channels (statics pre-gym-8, map seller, E4-victory gift) but **all legendary statics at level 64** (the cap), not vanilla levels.
- **Cave of Origin: EXCLUDED** (from pools, routes, and the map — blocker NPC stays). Standing principle: when story content is in doubt, exclude it.

### Phase mapping for v2 items
Phase 1-2: 7.2, 7.5 (clock, Box Link), Repellent item · Phase 3: 7.3 (engine+auto-suppress), 7.4 · Phase 4: randomizer incl. items + quiz assignment + 7.9 pool generation · Phase 5: tracker UI (7.3) · Phase 6: gyms incl. preview screen + pick-4 · Phase 7: 7.6, 7.7, 7.10 · Phase 8: quizzes + 7.8 wagers · Phase 9: UI suite incl. 7.1 register bag + 7.11 + sprites/backgrounds.

---

*Per-workstream detail — verified file:line integration points, full risk lists, every open decision: [investigations/](investigations/) (13 reports).*
