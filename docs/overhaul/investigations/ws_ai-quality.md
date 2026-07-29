## Workstream: ai-quality

**Overall effort:** M — the engine already ships everything needed (flags, difficulty variants, pools, doubles, tests, debug tooling); the real cost is authoring ~8 gym + E4 + rival teams/pools and resolving the randomizer policy. Rises to L only if Rogue-style runtime team generation or custom doubles scoring is chosen.

### Already in the expansion (verified in repo)

- **Modern AI flag system (34 behavior flags + composites)**
  - Where: `include/constants/battle_ai.h — AI_FLAG_CHECK_BAD_MOVE, AI_FLAG_TRY_TO_FAINT, AI_FLAG_CHECK_VIABILITY, AI_FLAG_SMART_SWITCHING, AI_FLAG_OMNISCIENT, AI_FLAG_ACE_POKEMON, AI_FLAG_DOUBLE_ACE_POKEMON, AI_FLAG_PREDICT_SWITCH/INCOMING_MON/MOVE, AI_FLAG_PP_STALL_PREVENTION, AI_FLAG_SMART_TERA, AI_FLAG_ASSUME_STAB, AI_FLAG_ASSUME_STATUS_MOVES, AI_FLAG_WEIGH_ABILITY_PREDICTION, etc.; composites AI_FLAG_BASIC_TRAINER, AI_FLAG_SMART_TRAINER, AI_FLAG_PREDICTION, AI_FLAG_ASSUMPTIONS (lines 46-49). Implementation: src/battle_ai_main.c, battle_ai_util.c, battle_ai_switch.c, battle_ai_items.c, battle_ai_field_statuses.c, battle_ai_record.c. Docs: docs/tutorials/ai_flags.md, ai_logic.md`
  - How: Per-trainer 'AI: Smart Trainer / Prediction' line in src/data/trainers.party. Repo docs recommend AI_FLAG_SMART_TRAINER (+ AI_FLAG_PREDICTION for bosses) as the maintained 'smartest' composite — gym leaders currently only have 'AI: Basic Trainer' (e.g. TRAINER_ROXANNE_1, trainers.party line 4801), so upgrading gyms is a one-line edit per trainer
- **AI tuning knobs (switch %, score thresholds, damage-roll policy, prediction/tera chances)**
  - Where: `include/config/ai.h — e.g. SHOULD_SWITCH_* percentages, AI_BAD_SCORE_THRESHOLD (90), AI_ROLL_ATTACKING/DEFENDING (AI_ROLL_MAX/MEDIAN), PREDICT_SWITCH_CHANCE (50), PREDICT_MOVE_CHANCE (100), AI_CONSERVE_TERA_CHANCE_PER_MON, PP_STALL_* settings`
  - How: Edit defines and rebuild; e.g. set PREDICT_SWITCH_CHANCE to 100 for deterministic prediction, raise switch percentages for more Run&Bun-like play
- **Difficulty system with per-trainer Easy/Normal/Hard team variants**
  - Where: `include/config/battle.h:272 B_VAR_DIFFICULTY (default 0=off); include/constants/difficulty.h (DIFFICULTY_EASY/NORMAL/HARD/TEST); src/difficulty.c (Script_SetDifficulty/Increase/Decrease/GetDifficulty); trainers.party 'Difficulty:' field parsed by tools/trainerproc/main.c:1282 into gTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT] (src/data.c:230); falls back to Normal if no variant exists (GetTrainerDifficultyLevel). Added in 1.11.0 (docs/changelogs/1.11.x/1.11.0.md, PR #5337)`
  - How: Assign B_VAR_DIFFICULTY to a free VAR_ constant, call Script_SetDifficulty after NewGameInitData, then author '=== TRAINER_ROXANNE_1 === / Difficulty: Hard' blocks with new teams — no engine work. Zero SaveBlock growth (reuses an existing var slot)
- **AI-vs-AI battle mode for evaluating AI changes in-game**
  - Where: `include/config/battle.h:258 B_FLAG_AI_VS_AI_BATTLE; consumed in src/battle_controllers.c:61 IsAiVsAiBattle()`
  - How: Assign a free FLAG_ constant; while set, the player's mons are AI-controlled in subsequent battles — lets you watch tuned gym AI fight itself
- **Headless AI test harness**
  - Where: `include/test/battle.h:971-974 AI_SINGLE_BATTLE_TEST / AI_DOUBLE_BATTLE_TEST (+ AI_MULTI/AI_TWO_VS_ONE per test_runner_battle.c:2219); ~19 existing suites in test/battle/ai/ (ai_doubles.c, ai_switching.c, ai_flag_predict_move.c, ...); docs/tutorials/how_to_testing_system.md`
  - How: make check-style test runner; write EXPECT_MOVE-style assertions for gym-specific scenarios before/after tuning
- **In-battle AI debugging (score viewer, decision-time profiler)**
  - Where: `src/battle_debug.c — 'AI Info' (LIST_ITEM_AI_INFO, per-move AI scores incl. recent score-highlight fix commit b88200a4bf) and 'AI Party' (LIST_ITEM_AI_PARTY) pages in the battle debug menu; include/config/debug.h:12 DEBUG_AI_DELAY_TIMER (frames AI takes to decide, replaces 'What will PKMN do' text)`
  - How: Enable debug menu config, open battle debug during a fight to see live move scores; turn on DEBUG_AI_DELAY_TIMER when stacking heavy flags to watch GBA CPU cost
- **Trainer teams in Pokemon Showdown export syntax (competitive sets are copy-paste)**
  - Where: `src/data/trainers.party (855 trainer blocks; header documents full syntax: EVs/IVs/Nature/Ability/Item/Tera Type/'Battle Type: Doubles'/Ace via party order), compiled by tools/trainerproc (Makefile:225); also battle_partners.party for multi-battle allies`
  - How: Paste a Showdown set under a trainer block, add 'Level:' (trainerproc defaults to 100!) and 'AI:' lines; 'Battle Type: Doubles' turns any gym into a doubles fight
- **Trainer Party Pools (TPP) — randomized-but-coherent teams from curated pools**
  - Where: `docs/tutorials/how_to_trainer_party_pool.md; trainers.party fields 'Party Size', 'Pool Rules', 'Pool Pick Functions', 'Pool Prune', 'Copy Pool', per-mon 'Tags:' (Lead/Ace/Weather Setter/Weather Abuser/Support + 3 free tags) parsed in tools/trainerproc/main.c:1297-1320; rulesets incl. POOL_RULESET_DOUBLES, POOL_RULESET_WEATHER_DOUBLES, POOL_RULESET_SUPPORT_DOUBLES in src/data/battle_pool_rules.h; species/item clauses via B_POOL_RULE_* in include/config/battle.h; AI_FLAG_RANDOMIZE_SWITCHIN / AI_FLAG_RANDOMIZE_PARTY_INDICES support it`
  - How: Give a gym leader a 10-15 mon curated pool with role tags and 'Party Size: 4' + doubles ruleset — every run gets a different but role-coherent VGC team. This is the native answer to 'teams must stay sensible when randomized'
- **Per-battle custom AI (boss fights) via dynamic AI functions**
  - Where: `include/constants/battle_ai.h:52 AI_FLAG_DYNAMIC_FUNC; docs/tutorials/ai_dynamic_functions.md; script cmds setdynamicaifunc / setdynamicswitchaifunc (examples AI_TagBattlePreferFoe, ShouldSwitchDynFuncExample)`
  - How: Set flag on the trainer, call setdynamicaifunc <YourFunc> in the gym script before trainerbattle; auto-clears after the battle
- **Rule-filtered random mon generator (scripting-level)**
  - Where: `docs/tutorials/how_to_random_mon_generator.md; include/constants/random_mon_generation.h; src/data/random_mon_generator.h (RandomSpeciesGeneratorOptions: species pools, legendary/paradox bans, filter funcs); script cmd getrandomspecies`
  - How: Reusable building block if the randomizer workstream wants type-constrained random gym mons instead of fixed pools
- **Difficulty-aware trainer slide-in messages (gym flavor)**
  - Where: `docs/tutorials/how_to_new_trainer_slide.md — SetTrainerSlideMessage(enum DifficultyLevel, ...); difficulty fallback added in 1.11.0 (PR #6088)`
  - How: Author slide messages per difficulty for gym leaders (last-mon, low-HP triggers)

### Reuse candidates

- **Smogon analysis sets as JSON (@pkmn/smogon, data.pkmn.cc)** — Machine-readable curated competitive sets per generation/format (refreshed daily) — the raw material for coherent gym teams; since trainers.party IS Showdown export syntax, sets convert with a trivial script (add Level/AI lines, downscale movepools)
  - Source: https://github.com/pkmn/smogon
  - Port effort: S — JSON-to-paste transform is a ~100-line Python script. License: code MIT; set/analysis data copyrighted by Smogon and contributors — fine for personal use, credit Smogon if the hack is distributed; no official API, formats may change
- **Run & Bun AI documentation (community transcription)** — The reference for what a beloved hard-nuzlocke AI does (full team omniscience, deterministic scoring, documented tie-breaks) — use as a tuning target for include/config/ai.h knobs and flag choices. Design reference only: Run & Bun itself is not open source, so no code to port
  - Source: https://github.com/pranavmenonx/PokemonRunAndBunHelper/blob/main/AI%20Document%20for%20RnB%20(1.07).txt
  - Port effort: S — reading + config tuning, no code. Credit dekzeh/community doc authors if you replicate documented behavior in shipped docs
- **Emerald Rogue (procedural type-themed gym teams + difficulty scaling)** — Working, battle-tested runtime generator for coherent gym-leader teams of an assigned type that scale with progress, plus configurable trainer-intelligence options — the proven pattern for 'coherent teams under randomization'. Note: repo default branch is 'vanilla'; Rogue code lives on its release branches (e.g. the expansion-based 2.x branches)
  - Source: https://github.com/Pokabbie/pokeemerald-rogue
  - Port effort: XL to port the generator into upstream 1.16.x (Rogue forked an older base, deep divergence); S-M to copy its design (tag/role-based generation) onto the in-repo TPP + random-mon-generator instead — recommended. Decomp-scene code has no formal license; community norm is to ask Pokabbie and credit in CREDITS.md
- **rh-hideout expansion docs (AI flags guide)** — Maintained upstream documentation of every AI flag with recommendations (matches this tree's docs/tutorials/ai_flags.md) — authoritative reference while tuning
  - Source: https://rh-hideout.github.io/pokeemerald-expansion/tutorials/ai_flags.html
  - Port effort: S — reference only, nothing to port
- **Upstream expansion master (continuous AI fixes)** — Active 'category: battle-ai' issue tracker (12 open, incl. #10135 doubles AI over-targets KOs via greedy per-battler scoring — directly hits VGC gyms; #10259 charging-move turn-cost underestimation; #10276 party-count bugs; #9922 mid-turn switch bug) and a steady stream of merged AI fixes (this tree already has recent ones, e.g. commits 12de80e9c0, b88200a4bf, d85fca45c8)
  - Source: https://github.com/rh-hideout/pokeemerald-expansion/issues?q=is%3Aissue%20state%3Aopen%20label%3A%22category%3A%20battle-ai%22
  - Port effort: S per sync — periodic 'git merge upstream/master' picks up AI fixes for free; effort is regression-testing the rest of the overhaul, not the merge itself
- **trainer_editor (GUI trainer/party editor, Go, Apache-2.0)** — Searchable GUI for editing trainer parties — but pinned to an old expansion commit (pre-dates current trainers.party features like TPP/Difficulty)
  - Source: https://github.com/geefuoco/trainer_editor
  - Port effort: M to update to 1.16.x format and low value — trainers.party is already human-friendly Showdown text; skip unless bulk-editing hundreds of generic trainers. Apache-2.0, attribution required if modified+redistributed

### Must build

- Author the actual gym content: per-leader Hard-difficulty doubles teams ('Difficulty: Hard' + 'Battle Type: Doubles' + 'AI: Smart Trainer / Prediction' + curated pools with role Tags) — pure trainers.party authoring, no engine code; hardest part is adapting Smogon sets to level caps and early-game movepools by hand
- Small Smogon-JSON -> trainers.party batch converter (none exists per web search; ~100-line script using data.pkmn.cc sets)
- Randomizer interaction policy layer: either exempt gym leaders/bosses from trainer randomization, or regenerate their pools type-coherently (glue between the randomizer workstream and TPP/random-mon-generator); includes optional seeding so a TPP team doesn't reroll on save-scum (fairness in nuzlocke)
- Optional: batch AI-vs-AI evaluation loop (script driving AI_DOUBLE_BATTLE_TEST scenarios or B_FLAG_AI_VS_AI_BATTLE) to measure win rates of tuned gym teams vs representative player teams
- Optional: per-gym dynamic AI functions (setdynamicaifunc) for signature boss behaviors, e.g. Trick Room gym commits to setter-first lines

### Integration points

- src/data/trainers.party + src/data/battle_partners.party (all team content)
- tools/trainerproc/main.c (party format compiler: Difficulty, Party Size, Pool Rules, Tags, AI fields)
- include/constants/battle_ai.h + include/config/ai.h (flag set and tuning knobs)
- include/config/battle.h (B_VAR_DIFFICULTY:272, B_FLAG_AI_VS_AI_BATTLE:258, B_POOL_RULE_*)
- src/difficulty.c + include/constants/difficulty.h (difficulty scripting API)
- src/battle_ai_main.c / battle_ai_util.c / battle_ai_switch.c (only if custom scoring is added)
- src/data/battle_pool_rules.h (TPP rulesets for doubles gyms)
- data/maps/*Gym*/scripts.inc (doubles conversion, Script_SetDifficulty calls, setdynamicaifunc)
- test/battle/ai/* + include/test/battle.h (regression tests for tuned behavior)
- src/battle_debug.c + include/config/debug.h (AI score viewer, delay timer)
- include/config/caps.h (level-cap workstream determines gym levels the sets must be built around)
- Randomizer workstream (must agree on gym-leader exemption or pool regeneration)

### Risks

- Randomizer vs curated teams is a direct conflict: if trainer parties are randomized, hand-built sets (items/EVs/moves tuned to species) become incoherent — must resolve via exemption or TPP-based regeneration before authoring teams
- Doubles AI is the engine's weakest area right now (upstream #10135: greedy per-battler scoring over-commits to KOs; partner coordination limited to specific cases) — VGC-style gyms lean exactly on this; expect some dumb turns without upstream syncs or custom dynamic functions
- Prediction/omniscient flags in a permadeath format can feel unfair AND are stochastic by default (PREDICT_SWITCH_CHANCE 50 in include/config/ai.h) — deaths to coin-flip predictions may frustrate; deterministic settings change game feel
- AI decision time on GBA hardware grows with SMART_* + PREDICT_* + doubles (4 battlers x 4 moves x rolls); must profile with DEBUG_AI_DELAY_TIMER — noticeable input lag between turns is the failure mode
- Base is untagged 1.16.3-dev (EXPANSION_TAGGED_RELEASE FALSE, include/constants/expansion.h) — syncing upstream master for AI fixes can break other workstreams' patches; freezing forfeits fixes
- trainerproc defaults: omitted Level = 100, omitted IVs = 31 — a missing Level line in an authored set silently produces a level-100 gym mon
- Smogon set data is copyrighted (credit Smogon; only an issue if the hack is publicly distributed); Emerald Rogue reuse needs Pokabbie's blessing per scene norms
- TPP teams re-roll each battle start (no persistence) — save-scumming a gym reroll is possible; seeding fix touches battle-start code (no SaveBlock growth needed if seeded from trainer ID + a run seed)
- SaveBlock impact of this workstream is effectively zero (difficulty = existing var slot, AI-vs-AI = existing flag slot) — but only if free VAR_/FLAG_ slots remain after other workstreams claim theirs

### Open decisions (owner's call)

- Difficulty architecture: (a) rebalance Normal in place (no B_VAR_DIFFICULTY, least work), (b) Hard-only variants via 'Difficulty: Hard' with Normal fallback, (c) full Easy/Normal/Hard authoring (3x content cost). (b) fits a personal nuzlocke build best but it's the owner's call
- Gym AI ceiling: AI_FLAG_SMART_TRAINER only vs SMART_TRAINER + AI_FLAG_PREDICTION vs adding ASSUMPTIONS instead of full OMNISCIENT (fairer: AI infers rather than knows) — and whether to make prediction deterministic (PREDICT_SWITCH_CHANCE 100) Run&Bun-style or keep stochastic
- Randomization scope for gyms: exempt gym leaders entirely (fixed curated teams) vs per-run TPP pool picks from curated type pools vs fully generated teams (Rogue-style) — determines whether team authoring is 6 mons or 15-mon pools per leader
- Doubles rollout: all 8 gyms + E4 as VGC doubles, or mixed singles/doubles (doubles only where the AI handles the archetype well, e.g. weather/Trick Room)
- Upstream policy: track rh-hideout master for continuous AI fixes (merge risk across workstreams) vs freeze at current 1.16.3-dev and cherry-pick specific AI PRs
- Set source of truth: hand-adapt Smogon gen9 sets per gym (highest quality, most work) vs script-generate from data.pkmn.cc then hand-fix (faster, needs review pass)
