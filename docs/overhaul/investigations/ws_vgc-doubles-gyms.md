## Workstream: vgc-doubles-gyms

**Overall effort:** M — core (all-gym doubles + AI pools with clauses + pick-4 UI via fisham branch) is mostly configuration/authoring on features already in the tree; grows to L only if the owner wants the full team-preview screen, flat-level normalization, and per-gym twin-leader formats all built.

### Already in the expansion (verified in repo)

- **Per-trainer forced double battles (the core mechanism for all-doubles gyms)**
  - Where: `src/data/trainers.party property 'Battle Type: Doubles' or 'Double Battle: Yes' (parsed by tools/trainerproc/main.c:1249-1266) -> 'u16 battleType:2' + enum TRAINER_BATTLE_TYPE_DOUBLES in include/data.h:91-95,137 -> BATTLE_TYPE_DOUBLE set in src/battle_setup.c:1364-1367 (normal battles) and :1526-1527 (rematches)`
  - How: Edit each gym trainer/leader entry in src/data/trainers.party, add 'Battle Type: Doubles'. Works with plain trainerbattle_single map scripts - no script changes needed. Defaults to Singles, so routes/wilds untouched.
- **1v2 doubles when the player has only 1 usable mon (nuzlocke-relevant)**
  - Where: `FEATURES.md:33 ('1v2/2v1 battles' listed as supported battle type); the legacy 2-mon gate only applies to the old trainerbattle_double script path via 'special HasEnoughMonsForDoubleBattle' in data/scripts/trainer_battle.inc:31`
  - How: Nothing to enable - trainers.party-doubles battles start as 1v2 automatically if the player has one usable mon. If the owner prefers to block instead, reuse HasEnoughMonsForDoubleBattle (src/script_pokemon_util.c:79) in gym scripts.
- **Bring-6-pick-N team selection UI with species clause, item clause, and level cap already enforced**
  - Where: `PARTY_MENU_TYPE_CHOOSE_HALF (include/party_menu.h:27), ChoosePartyForBattleFrontier (src/script_pokemon_util.c:205, count from gSpecialVar_0x8004+1), clause validation in CheckBattleEntriesAndGetMessage (src/party_menu.c:7331-7367: same-species PARTY_MSG_MONS_CANT_BE_SAME, same-item PARTY_MSG_NO_SAME_HOLD_ITEMS), ReducePlayerPartyToSelectedMons (src/script_pokemon_util.c:226), SavePlayerParty/LoadPlayerParty (src/load_save.c:170/179)`
  - How: Script sequence at gym door: SavePlayerParty -> setvar 0x8004/0x8005 -> special ChoosePartyForBattleFrontier -> special ReducePlayerPartyToSelectedMons -> battle -> special LoadPlayerParty. Party backup lives in existing gSaveBlock1Ptr->playerParty (include/global.h:1111) - zero new save space. Caveat: eligibility also applies gSpeciesInfo[].isFrontierBanned (src/party_menu.c:7325) and reads VAR_FRONTIER_FACILITY.
- **AI-side bring-6-pick-4 with VGC clauses (Trainer Party Pools)**
  - Where: `docs/tutorials/how_to_trainer_party_pool.md; enable per-trainer with 'Party Size: 4' + more defined mons; 'Pool Rules: Doubles' = POOL_RULESET_DOUBLES in src/data/battle_pool_rules.h; clause configs B_POOL_RULE_SPECIES_CLAUSE / B_POOL_RULE_ITEM_CLAUSE / B_POOL_RULE_MEGA_STONE_CLAUSE etc. at include/config/battle.h:417-422; deterministic options B_POOL_SETTING_CONSISTENT_RNG / B_POOL_SETTING_USE_FIXED_SEED at battle.h:414-415`
  - How: Give each gym leader a 6-mon pool with Party Size: 4, Pool Rules: Doubles, Lead/Ace tags - the leader 'brings 6, picks 4' with species+item clause, randomized per encounter (great for randomized nuzlocke replayability).
- **Doubles-aware AI + team-preview-equivalent AI knowledge**
  - Where: `'AI: Double Battle' flag in trainers.party (AI_FLAG_DOUBLE_BATTLE, tuning at include/config/ai.h:136-145 e.g. FRIENDLY_FIRE_NORMAL_THRESHOLD, DOUBLE_TRICK_ROOM_ON_LAST_TURN_CHANCE); AI_FLAG_KNOW_OPPONENT_PARTY at include/constants/battle_ai.h:41 - docs/tutorials/ai_flags.md:151 explicitly calls it 'functions similarly to a team preview'`
  - How: Add 'AI: Double Battle / Know Opponent Party / ...' to gym trainers in trainers.party. (Deep AI tuning belongs to the gym-AI workstream.)
- **Two-opponent and partner doubles (VGC-flavored 2-leader gyms)**
  - Where: `trainerbattle_two_trainers macro (asm/macros/event.inc:825), BATTLE_TYPE_TWO_OPPONENTS (include/constants/battle.h:125); partner battles via src/data/battle_partners.party; per-trainer 'Multi Party: Half' + B_MULTI_HALF_TEAMS (include/config/battle.h:366) caps each side at 3 to keep multis short`
  - How: Use trainerbattle_two_trainers in a gym script for a twin-leaders battle; Multi Party: Half keeps it 3+3 vs 4.
- **Wild/route encounters stay singles by default**
  - Where: `WE_DOUBLE_WILD_CHANCE = 0 (include/config/wild_encounter.h:9) and WE_FLAG_FORCE_DOUBLE_WILD = 0 (:14), cleared in src/overworld.c:450`
  - How: Do nothing - wild doubles are opt-in and currently off.
- **VGC-adjacent extras already available**
  - Where: `B_FLAG_SLEEP_CLAUSE (include/config/battle.h:262); level/exp caps in include/config/caps.h (B_EXP_CAP_TYPE, B_LEVEL_CAP_TYPE, B_RARE_CANDY_CAP); pre-battle 'Mugshot: <color>' VS transition per trainer (tools/trainerproc/main.c:1267, mugshotColor in include/data.h:138)`
  - How: Set config defines / add Mugshot property to gym leaders for a modern VS-card transition (partial substitute for a preview screen).

### Reuse candidates

- **fisham33 'Select Pokemon for Battle' feature branch** — Turnkey bring-6-pick-N gym flow: trainerbattle_selectmons script macro (trainer, texts, post-battle script, num_pokemon 1-6), party save/restore around battle, cancellation handling, loss-continues-script variant (trainerbattle_continuescript, useful with nuzlocke no-whiteout), reuses frontier structures so zero new save data
  - Source: https://github.com/fisham-org/pokeemerald-expansion-features (branch feature/select-mons-for-battle; guide: wiki page Select-Pokemon-for-Battle)
  - Port effort: S - branch is only 5 commits on expansion 1.16.2, owner is on 1.16.3-dev master, small merge; author publishes explicitly for reuse with pull instructions; no license file (standard for pokeemerald derivatives) so credit 'fisham33' in CREDITS
- **fisham33 'Battle Mode Toggle' feature branch** — Options-menu setting forcing ALL trainer battles to singles/doubles/mixed with safety checks (skips doubles if trainer has 1 mon or player lacks 2 usable) - alternative to hand-editing every trainer if the owner wants game-wide doubles, persisted in save data
  - Source: https://github.com/fisham-org/pokeemerald-expansion-features (branch feature/battle-mode-toggle; listed in Team Aqua's Asset Repo wiki Feature-Branches page)
  - Port effort: S - small options-menu + battle_setup patch; note it consumes save options bits (minor SaveBlock impact); credit fisham33
- **fisham33 'Level Scaling' feature branch** — Opponent-side level normalization: 6 scaling modes (level cap, party average/highest...), auto-devolution, EV/moveset/item tier scaling, per-trainer opt-out, zero save data - covers 'gym is always the right level' without touching the player's mons
  - Source: https://github.com/fisham-org/pokeemerald-expansion-features (branch feature/level-scaling)
  - Port effort: M - large system (new include/config/level_scaling.h); author discloses parts were AI-assisted and edge cases undertested, so needs a test pass; credit fisham33
- **Obsidian Emerald source (VGC/doubles challenge hack by Skolgrahd & Speaker)** — The best-known all-doubles VGC Emerald hack with public source: per-trainer .doubleBattle + CB2_InitBattleInternal hook (mechanism upstream has since absorbed as battleType, so code port unneeded) - value is its VGC-style gym teams, field-effect leader design, and doubles level curve as authoring reference
  - Source: https://github.com/skolgrahd/pokeemerald-expansion (fork of rh-hideout, branches master/bugfixing; PokeCommunity thread id 528759 - site blocks automated fetch with 403)
  - Port effort: S as design/content reference (read trainers.h, copy team-building patterns); reusing their actual teams verbatim needs the authors' permission + credit; repo base is expansion 1.8.1 so code itself is stale
- **Pokemon Emerald Double Battle Edition (laser_dolphin)** — All-doubles conversion on decomp with a curated QoL mod list (README credits each pret-wiki/PokeCommunity mod used) - same per-trainer doubleBattle + battle_main hook approach; useful as second reference for which trainers/scripts need touching (they kept 455 trainerbattle_single scripts and forced doubles via trainer data, confirming scripts don't need rewriting)
  - Source: https://github.com/poke-dodge/pokeemerald-dbe (redirect from laserXdolphin/pokeemerald-dbe; expansion 1.9.4/1.10.1 base)
  - Port effort: S as reference only - mechanism already upstream in 1.16.x; no license, credit laser_dolphin if any content copied
- **verdant 'best_of_three' pre-battle team preview screen (fakuzatsu)** — Only expansion-based open-team-preview implementation found: full-screen overworld menu showing player party icons vs opponent TrainerMon party icons + both trainer sprites, script-invocable before battle - directly the 'open team preview' UI pattern
  - Source: https://github.com/fakuzatsu/verdant (src/best_of_three.c, ~15KB)
  - Port effort: M - built on expansion 1.10-dev, needs API updates to 1.16 (GetTrainerStructFromId/gParties era) and pick-order additions; repo has NO license and is a personal hack: requires fakuzatsu's permission + credit before reuse (screen skeleton follows the common free-to-use ghoulslash UI-menu template, so a permission-free rewrite from the template is a fallback)
- **Complete Fire Red Upgrade (CFRU) in-battle team preview (Skeli789)** — Most mature GBA team-preview UX (toggleable in-battle preview overlay) - design reference for interaction/layout
  - Source: https://github.com/Skeli789/Complete-Fire-Red-Upgrade (src/battle_indicators.c: DisplayInBattleTeamPreview, TryLoadTeamPreviewTrigger; src/move_menu.c: HandleInputTeamPreview)
  - Port effort: XL - FireRed binary-injection hackbase, incompatible codebase; treat as design reference only, do not port; CFRU requires credit per its README if anything is derived
- **Trainer Party Pools upstream docs (already in tree, online mirror verified)** — Authoritative how-to for the AI-side pick-4-with-clauses system described above (POOL_RULESET_DOUBLES, tags, clause configs)
  - Source: https://rh-hideout.github.io/pokeemerald-expansion/tutorials/how_to_trainer_party_pool.html
  - Port effort: S - zero port, feature is in the working tree; RHH credit already required by using expansion

### Must build

- Open team preview screen (pre-battle, shows opponent's actual team) - nothing upstream or in any expansion branch found; adapt verdant's best_of_three.c or rebuild from the ghoulslash UI template with mon icons from GetTrainerStructFromId()->party (M)
- True VGC flat-level normalization (temporarily set both sides to e.g. 50 and restore after) - no implementation exists anywhere found; small build piggybacking SavePlayerParty -> SetMonData(EXP)+CalculateMonStats -> LoadPlayerParty restore (S-M), only if owner rejects the two existing alternatives (exp/level caps, opponent-side scaling)
- Species/item clause validation for full-party gym battles if the pick-4 UI is NOT used - a small script special mirroring CheckBattleEntriesAndGetMessage outside the CHOOSE_HALF flow (S)
- Gym script wiring: insert the select-mons/preview/heal-restore sequence + doubles-safe rematch handling into all 8 gym maps (+ optionally E4) in data/maps/*Gym*/scripts.inc (S-M, mechanical)
- Decouple gym pick-4 eligibility from frontier rules if desired: strip isFrontierBanned check and VAR_FRONTIER_FACILITY dependency in the party_menu eligibility path (S)

### Integration points

- src/data/trainers.party (Battle Type / Party Size / Pool Rules / Tags / AI per gym trainer)
- src/data/battle_pool_rules.h + include/config/battle.h:414-422 (pool clause configs)
- src/battle_setup.c (BattleSetup_StartTrainerBattle / BattleSetup_StartRematchBattle doubles hooks)
- src/party_menu.c CHOOSE_HALF flow (GetMaxBattleEntries/CheckBattleEntriesAndGetMessage/GetBattleEntryEligibility)
- src/script_pokemon_util.c (ChoosePartyForBattleFrontier / ReducePlayerPartyToSelectedMons)
- src/load_save.c + include/global.h:1111 gSaveBlock1Ptr->playerParty (party backup - existing save space, none new)
- data/specials.inc + asm/macros/event.inc (trainerbattle_* macros, new selectmons macro)
- data/maps/*Gym*/scripts.inc (8 gyms) + data/scripts/trainer_battle.inc
- include/config/ai.h + include/constants/battle_ai.h (doubles AI flags - shared surface with gym-AI workstream)
- include/config/caps.h (level caps - shared surface with nuzlocke workstream)
- include/config/wild_encounter.h (confirm wild doubles stay off)

### Risks

- fisham select-mons branch is on expansion 1.16.2; owner tree is 1.16.3-dev master - small but real merge conflicts in party_menu.c/script_pokemon_util.c (both recently refactored upstream to gParties[])
- Nuzlocke interplay: loss handling with a reduced 4-mon party must restore the full party BEFORE whiteout/no-whiteout (B_FLAG_NO_WHITEOUT) logic runs, or mons can be lost from the backup - fisham's branch patched exactly this (RestoreSelectMonsPartyAfterBattle in CB2_EndTrainerBattle); needs explicit testing with the nuzlocke workstream's death/whiteout rules
- 1v2 auto-doubles: with one usable mon a doubles gym becomes a near-hopeless 1v2 - by design in expansion; owner must pick block-vs-allow or nuzlocke runs can be silently unwinnable
- Pick-4 eligibility reuses frontier rules: gSpeciesInfo[].isFrontierBanned would silently bar randomized legendaries from gym selection, and the flow reads VAR_FRONTIER_FACILITY / gSpecialVar_0x8004-5 (fragile shared globals if any other feature repurposes them)
- verdant team-preview code has no license: shipping it without fakuzatsu's permission violates community norms; fallback is a from-template rewrite (adds ~1 week)
- fisham level-scaling is explicitly part-AI-generated and undertested per the author - do not adopt without a test pass; interacts with the nuzlocke level-cap system (double-scaling if both enabled)
- Gym rematch path is separate (BattleSetup_StartRematchBattle) - doubles + pick-4 wiring must be duplicated there or rematches regress to full-party battles
- Trainer slides / mugshots / two-opponent battles each have doubles-specific edge cases (e.g. AI_FLAG_DOUBLE_ACE_POKEMON duplicate-mon bug was only fixed in 1.13.4) - stay on current master and rerun the battle test suite after merging branches
- SaveBlock: core plan needs zero new save space (party backup + selected order already exist); only the optional battle-mode options toggle consumes save options bits

### Open decisions (owner's call)

- Doubles scope: (a) gyms + leaders only via per-trainer 'Battle Type: Doubles' (surgical, recommended-shaped), (b) every trainer game-wide via fisham battle-mode-toggle or mass trainers.party edit (Obsidian/DBE style), (c) player-facing options toggle so runs can vary
- One-usable-mon gym entry (nuzlocke deaths): allow engine 1v2 as-is, or gate the gym door with a 2-usable-mons check (HasEnoughMonsForDoubleBattle) forcing the player to catch/revive first
- Level handling: (1) none - rely on nuzlocke exp/level caps in caps.h, (2) opponent-side auto-scaling (fisham level-scaling, needs test pass), (3) true VGC flat-level build for gym battles only; options 1 and 2 exist today, 3 is a build
- Team preview: (a) full pre-battle preview screen (adapt verdant - requires contacting fakuzatsu for permission, or rebuild from template), (b) cheap version: Mugshot VS transition + AI_FLAG_KNOW_OPPONENT_PARTY only, (c) skip - blind teams preserve nuzlocke tension with randomized leaders (pools already randomize them)
- Whether the player-side pick-4 UI is mandatory at gym doors (full VGC ritual, adds ~30s per attempt) or gyms just battle the current 6-mon party 4-at-most via doubles (faster, less VGC-authentic) - conflicts with the max-QoL/time-saving goal
- Which VGC clauses to actually enforce on the player: species clause is near-meaningless in a randomized nuzlocke (dupes clause likely already applies), item clause matters; decide item-clause exclusions (e.g. allow duplicate berries via poolItemClauseExclusions symmetry)
- Gym format per gym: single leader 4v4 doubles, twin leaders via trainerbattle_two_trainers (3+3 with Multi Party: Half), or leader + player-partner multi - can vary per gym for variety
- Do E4/Champion/rival fights follow the same VGC doubles ruleset or stay full 6v6 doubles/singles
- Contacting authors: whether owner wants to ask fakuzatsu (verdant preview) and Skolgrahd/Speaker (Obsidian team designs) for reuse permission, vs building/authoring in-house with credits only for the freely-published fisham branches
