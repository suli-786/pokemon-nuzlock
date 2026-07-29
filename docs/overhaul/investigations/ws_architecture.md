## Workstream: architecture

**Overall effort:** M

### Already in the expansion (verified in repo)

- **Per-feature config header convention (the pattern our modules must follow)**
  - Where: `include/config/*.h — 24 headers today (general.h, battle.h, ai.h, caps.h, save.h, debug.h, quickstart.h, dexnav.h, follower_npc.h, name_box.h, map_preview_screen.h, ...); include/config/general.h defines RHH_EXPANSION and GEN_1..GEN_9/GEN_LATEST semantics; verified in this tree (expansion 1.16.3-dev per include/constants/expansion.h)`
  - How: Each feature = one config header of #define toggles included where needed; add include/config/nuzlocke.h etc. New fields in structs are guarded by #if FEATURE_TOGGLE, exactly like struct SaveBlock3 does in include/global.h:256-274
- **Save-space reclamation flags (3790 bytes reclaimable)**
  - Where: `include/config/save.h — FREE_MYSTERY_EVENT_BUFFERS (1104B), FREE_MYSTERY_GIFT (876B), FREE_UNION_ROOM_CHAT (212B), FREE_MATCH_CALL (104B), FREE_LINK_BATTLE_RECORDS (88B), etc.; file comments total 2516B SaveBlock1 + 1274B SaveBlock2`
  - How: Flip FALSE→TRUE; all are link/e-reader features, safe for single-player. Note FREE_MATCH_CALL also kills rematch/VS-Seeker data — check with QoL workstream before enabling
- **SaveBlock3: expansion's own growable save region, 1620 of 1624 bytes free today**
  - Where: `include/global.h:256 (struct SaveBlock3), include/save.h:8 (SAVE_BLOCK_3_CHUNK_SIZE 116 × NUM_SECTORS_PER_SLOT 14 = 1624B, carved from per-sector footer space), STATIC_ASSERT at src/save.c:80 enforces the budget; measured sizeof = 4 bytes with default configs (only dexNavChain active)`
  - How: Add our persistent fields (nuzlocke state, knowledge-seen flags) to struct SaveBlock3 behind #if config guards; the STATIC_ASSERT fails the build if we overflow — free safety net
- **Measured free save bytes at current defaults (compiled probe against this tree, arm-none-eabi, MODERN=1)**
  - Where: `src/save.c:80-83 STATIC_ASSERTs define budgets: SaveBlock1 = 15568/15872 used → 304B free; SaveBlock2 = 3884/3968 → 84B free; PokemonStorage = 34144/35712 → 1568B free; SaveBlock3 = 4/1624 → 1620B free; SECTOR_DATA_SIZE 3968 in include/save.h:7`
  - How: Budget order for new persistent data: SaveBlock3 first (1620B), then FREE_* flags to open SB1/SB2, then PokemonStorage slack (needs layout change) or repurposing special sectors 30-31 as last resort
- **Unused event flags and vars for scripted features (nuzlocke per-route encounter flags, gym state)**
  - Where: `include/constants/flags.h — 375 FLAG_UNUSED_0x* defines (plus daily-flag block); include/constants/vars.h — 22 VAR_UNUSED_0x40* vars; both verified by grep count in this tree`
  - How: Rename FLAG_UNUSED_* / VAR_UNUSED_* to feature names; zero save-size cost since flag/var arrays already exist in SaveBlock1
- **Battle test harness incl. AI tests — directly covers gym-AI and doubles workstreams**
  - Where: `include/test/battle.h:971-974 (AI_SINGLE_BATTLE_TEST / AI_DOUBLE_BATTLE_TEST with EXPECT_MOVE/EXPECT_SWITCH/SCORE_* macros), test/battle/ai/, docs/tutorials/how_to_testing_system.md`
  - How: `make check -j` (all), `make check TESTS="prefix"` (filtered), `make pokeemerald-test.elf TESTS=...` for interactive mgba debugging; write AI regression tests for every gym-AI tweak
- **Non-battle unit-test pattern for pure logic (nuzlocke rules engine, randomizer determinism)**
  - Where: `test/bag.c, test/party_menu.c, test/save.c, test/random.c, test/random_mon_generation.c, test/species.c — overworld/logic tests run by the same runner (test/test_runner.c)`
  - How: Follow existing TEST(...) files; the runner vtable is struct TestRunner (include/test/test.h:11), an in-repo example of the function-pointer-interface pattern to copy for our modules
- **mgba-rom-test + hydra parallel runner shipped in-tree (Linux/mac/Windows binaries)**
  - Where: `tools/mgba/mgba-rom-test{,-mac,.exe}, tools/mgba-rom-test-hydra/ (source, built by make); wired at Makefile:228-235`
  - How: Nothing to install; `make check` uses them. GBA-legal: tests run in a headless mGBA against the real ROM
- **CI already covers build+test; fork inherits it**
  - Where: `.github/workflows/build.yml — jobs: build-emerald/firered/leafgreen, release (make release), test (TEST=1 make check), docs_validate; runs on push to master/upcoming and PRs`
  - How: Enable Actions on the fork; optionally delete firered/leafgreen jobs to cut CI time (single-player Emerald project)
- **Debug menus for manual QA of our features**
  - Where: `include/config/debug.h — DEBUG_OVERWORLD_MENU (R+Start, flags/vars/givemon editor), DEBUG_BATTLE_MENU (Select in battle), DEBUG_POKEMON_SPRITE_VISUALIZER; all DISABLED_ON_RELEASE`
  - How: Already on in dev builds; add our feature toggles as new debug-menu entries (src/debug.c) instead of building bespoke test harnesses
- **OOP-ish C interface patterns to model module APIs on**
  - Where: `struct TestRunner vtable include/test/test.h:11-22; per-battler function-pointer dispatch gBattlerControllerFuncs include/battle_controllers.h:298; struct ListMenuTemplate callbacks include/list_menu.h:63-64; struct MenuAction func union include/menu.h:44; task/MainCallback2 + window-template ownership used by every UI screen (large self-contained example: src/pokedex_plus_hgss.c)`
  - How: Define e.g. struct NuzlockeRuleset { bool32 (*canCatch)(...); void (*onFaint)(...); } const vtables selected by config — same shape as TestRunner
- **Makefile auto-globs source subdirectories — zero build-system work for new modules**
  - Where: `Makefile:306 — C_SRCS_IN := $(wildcard $(C_SUBDIR)/*.c $(C_SUBDIR)/*/*.c $(C_SUBDIR)/*/*/*.c); test/ globbed identically at Makefile:310`
  - How: Both flat src/nuzlocke.c and nested src/nuzlocke/*.c compile automatically; src/ is currently flat (only subdir is src/data), so flat-file-per-feature is the idiomatic choice
- **Feature-module exemplars to copy structure from**
  - Where: `src/dexnav.c + include/config/dexnav.h + docs/tutorials/how_to_random_mon_generator.md pattern; also follower_npc, name_box, map_preview_screen (each: one src file + one config header + one docs/tutorials/how_to_*.md)`
  - How: Replicate the triple (src file, config header, tutorial doc) per feature; docs/ is an mdbook (docs/book.toml) published via .github/workflows/docs.yml
- **Level/EV cap system — nuzlocke-adjacent infrastructure already present**
  - Where: `include/config/caps.h — B_EXP_CAP_TYPE (EXP_CAP_HARD/SOFT), B_LEVEL_CAP_TYPE (LEVEL_CAP_FLAG_LIST keyed to badge flags via sLevelCapFlagMap, or LEVEL_CAP_VARIABLE), B_RARE_CANDY_CAP`
  - How: Nuzlocke workstream should build on these toggles rather than reimplementing caps
- **Upstream release cadence documented in-repo**
  - Where: `docs/team_procedures/schedule.md — minor release every 90 days, patch releases at least monthly, big-feature freeze T-30d, merge freeze T-14d; two branches: master (stable) and upcoming (next minor); current tree = 1.16.3 untagged dev on top of 1.16.2 (include/constants/expansion.h)`
  - How: Plan merges around tagged releases (expansion/1.x.y tags); avoid tracking upcoming (churn)

### Reuse candidates

- **Sample UI scaffold (grunt-lucas sample-ui branch)** — A documented, copyable full-screen menu template (BG/window templates, task lifecycle, sprite loading, input handling) — the standard community starting point for new UI screens; base scaffold for the knowledge-base UI and nuzlocke status screens
  - Source: https://github.com/grunt-lucas/pokeemerald-expansion/tree/sample-ui (branch verified via GitHub API; companion tutorial/sample-ui branch also exists)
  - Port effort: S — built as a copy-me template on expansion base; needs API-rename fixups against 1.16. No formal license (pokeemerald derivatives carry none); credit grunt-lucas per community norm
- **Team Aqua's Asset Repo (wiki + feature branches + assets)** — Curated index of community feature branches by author, plus free-to-use UI/graphics assets and practical guides on keeping a fork updated — feeds the UI/visuals and QoL workstreams and our merge playbook
  - Source: https://github.com/Pawkkie/Team-Aquas-Asset-Repo (verified; wiki has 46+ pages: feature-branch index, git setup/updating guides, UI assets, tilesets)
  - Port effort: S per asset/branch — assets are drop-in by design. License: explicitly free to use/edit by default, but credit to original creators is required; respect per-asset restrictions
- **Emerald Rogue (Pokabbie) — full run-based overhaul with public source** — Working reference architecture for exactly our problem class: run rules enforcement, run-config menus, randomization pipeline, custom save handling for run state — mine for design patterns and data layouts
  - Source: https://github.com/Pokabbie/pokeemerald-rogue (verified public, ~9.7k commits)
  - Port effort: L — built on an older/heavily diverged base, so lift patterns not diffs; wholesale code ports would be XL and version-fragile. No formal license; credit Pokabbie and ask permission before large verbatim lifts
- **pret pokeemerald wiki tutorials** — Step-by-step diffs for dozens of QoL/UI/save modifications that apply near-directly to expansion; the Code section covers save-functionality changes relevant to our save strategy
  - Source: https://github.com/pret/pokeemerald/wiki/Tutorials (verified; categorized UI/Item/Scripting/Overworld/Battling/Code tutorials with difficulty ratings)
  - Port effort: S each — written as apply-by-hand diffs; some need expansion-API renames. Credit tutorial authors per community norm
- **RHH pokeemerald-expansion upstream (master tags) + docs site** — The update stream itself: tagged patch releases (~monthly) and minors (90-day), changelogs per version in docs/changelogs/, canonical documentation
  - Source: https://github.com/rh-hideout/pokeemerald-expansion and https://rh-hideout.github.io/pokeemerald-expansion/ (both verified; docs cover 1.16.2 incl. install/update guides)
  - Port effort: S per patch-tag merge / M per minor merge if our code stays in separate files + config headers; grows to L per merge if we deep-edit battle/AI/UI internals

### Must build

- Nuzlocke rules-engine module (src/nuzlocke.c + include/config/nuzlocke.h + SaveBlock3 fields + flags from FLAG_UNUSED pool) — no maintained drop-in exists for expansion 1.16; Emerald Rogue serves as pattern reference only
- Knowledge/in-game-dex UI module (src/knowledge.c or src/knowledge/) — scaffold from sample-ui branch; data source is already-in-ROM gSpeciesInfo/move/learnset tables, so build is UI + query layer only
- Module interface convention doc + one worked example (const function-pointer vtable per feature, modeled on struct TestRunner in include/test/test.h) so all 11 workstreams produce uniform, merge-isolated code
- Save-budget ledger: a single comment block + STATIC_ASSERT margin check in include/global.h reserving SaveBlock3 bytes per workstream before anyone commits fields (1620B is enough for everything planned only if allocated deliberately)
- Merge-upkeep playbook: script `git remote add rhh; git merge expansion/1.x.y` per tag + conflict-hotspot checklist (battle AI, battle_util, UI screens); document freeze criteria for an eventual 2.0 major
- Runtime-toggle bridge only if owner wants in-game feature switches: expansion configs are compile-time #defines; runtime toggles need a var/flag per feature plus an options-menu extension
- Implementation-order plan across the fleet (dependency-driven): 1) architecture scaffolding (config headers, save ledger, CI trim, merge playbook) → 2) QoL/time-savers + debug/quickstart polish (no save impact, immediate payoff) → 3) nuzlocke engine (claims save fields + flags first) → 4) randomizer/encounter tracking (depends on nuzlocke's per-route encounter model) → 5) gym-focused AI then VGC double gyms (AI config + trainer data; gated on test harness familiarity, doubles AI depends on smarter-AI work) → 6) knowledge UI and modern UI/visuals last (depend on sample-ui scaffold and on the data models above being stable)

### Integration points

- include/config/*.h (new per-feature config headers slot beside the existing 24)
- include/global.h struct SaveBlock3 (all new persistent state) + include/save.h sector layout
- src/save.c STATIC_ASSERT budget guards (build-time overflow protection)
- include/constants/flags.h and include/constants/vars.h (rename FLAG_UNUSED_*/VAR_UNUSED_* for scripted state)
- Makefile source globs (Makefile:306) — new src files/dirs auto-build
- test/ + include/test/battle.h (AI_*_BATTLE_TEST for gym AI; plain TEST() for rules logic) and tools/mgba-rom-test-hydra
- .github/workflows/build.yml (CI: build + TEST=1 make check)
- include/config/ai.h + src/battle_ai_*.c (gym AI workstream's hook surface)
- include/config/caps.h (nuzlocke level caps already exist)
- src/data/trainers.party + trainerproc (VGC double-gym teams; docs/tutorials/how_to_trainer_party_pool.md)
- src/debug.c + include/config/debug.h (manual QA entry points)
- docs/ mdbook (docs/book.toml) if we document our own modules

### Risks

- SaveBlock1/2 are nearly full at defaults (304B / 84B free): any workstream adding fields there un-coordinated breaks the build (best case) or corrupts saves (worst case, if asserts are bypassed); all persistent state must go through the SaveBlock3 ledger
- Upstream ships minors every 90 days that churn battle/AI internals (this repo's own recent commits are all battle fixes); the gym-AI and battle-UI workstreams edit exactly those files, so every merge will conflict there regardless of module isolation
- A future expansion 2.0 major (2.x rows appear in docs/team_procedures/schedule.md sample table) may force a freeze-vs-rewrite decision mid-project
- Community branches (sample-ui, Team Aqua feature branches, Emerald Rogue) target older expansion versions; APIs rename frequently (e.g. enum BattlerId), so naive cherry-picks compile-fail or silently misbehave — always re-test after porting
- Licensing: pokeemerald-derived code has no formal license; credit is a hard community norm (Team Aqua repo requires it explicitly), and wholesale lifts from named hacks like Emerald Rogue need author permission
- If save needs exceed SaveBlock3 (1620B), repurposing special sectors 30-31 (SECTOR_ID_TRAINER_HILL / SECTOR_ID_RECORDED_BATTLE, include/save.h:28-29) yields ~4KB each but those are single-buffered — a mid-write power loss loses that data, unlike the double-buffered main slots
- changing struct SaveBlock layouts mid-playthrough invalidates existing saves — field layout must be final before the owner starts a real run, or a save-migration shim must be built

### Open decisions (owner's call)

- Upstream policy — (a) merge every patch tag (~monthly, small regular pain, always current), (b) merge only 90-day minor tags (batched pain), or (c) freeze at 1.16.x now and cherry-pick only bugfixes (zero merge pain, growing drift, lose future gen updates)
- Module layout — (a) repo-idiomatic flat src/<feature>.c per feature (matches upstream style, simpler merges) vs (b) src/<feature>/ subdirectories (Makefile already supports; cleaner for the multi-file knowledge UI, slightly non-idiomatic)
- Save-overflow fallback order if >1620B persistent data is needed — (a) flip FREE_* flags in include/config/save.h (+3790B, deletes Mystery Gift/match-call/link-records: all dead in single-player, but FREE_MATCH_CALL also removes rematch data), (b) repurpose Trainer Hill/Recorded Battle sectors (+~8KB, single-buffered corruption risk), (c) reduce PC box count (PokemonStorage has 1568B slack before that)
- Feature toggles: compile-time only (free, rebuild to change) vs in-game settings menu (runtime vars cost save bytes + options-UI work) — affects every workstream's design
- CI scope on the fork: keep all three GAME_VERSION build jobs (upstream parity, slower) vs strip to EMERALD-only + test job (faster, single-player project doesn't ship FRLG)
- Guard our SaveBlock3 fields with #if config toggles (upstream-style, saves bytes when disabled, but toggling invalidates saves) vs unconditional fields (stable save layout across config changes)
- Whether to enable FREE_* flags now, before the first real playthrough, so the save layout is final from day one (recommended by the save-layout-invalidation risk) — owner must confirm none of the freed vanilla features are wanted
