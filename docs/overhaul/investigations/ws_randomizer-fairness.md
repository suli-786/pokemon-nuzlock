## Workstream: randomizer-fairness

**Overall effort:** M if Path C (build-time script) is chosen; L for full in-engine Path B with all leak paths covered. Path A is a non-starter, not an effort tier.

### Already in the expansion (verified in repo)

- **Random Mon Generation engine (script-level randomizer with fairness filters: BST-band filter, legendary/mythical/sub-legendary/ultra-beast/paradox bans, species pools, banned-species lists, form randomization, random balls, random teachable moves, random held items with hold-effect bans)**
  - Where: `src/random_mon_generation.c (struct RandomSpeciesGeneratorOptions, IsInBstRangeFilterFunc), src/data/random_mon_generator.h (option arrays, intentionally empty by default), include/constants/random_mon_generation.h (SPECIES_GENERATOR_* / ITEM_GENERATOR_* enums), tutorial docs/tutorials/how_to_random_mon_generator.md — added in expansion 1.16.0 (PR #9896, docs/changelogs/1.16.x/1.16.0.md); this tree is 1.16.3 (include/constants/expansion.h)`
  - How: Define option sets in src/data/random_mon_generator.h, then in scripts: getrandomspecies VAR_0x8000, SPECIES_GENERATOR_BST_RESTRICTED, arg1=<BST target>, arg2=<leniency>; getrandomitem VAR_0x8001, ITEM_GENERATOR_STANDARD; givemon/createmon with ball=BALL_RANDOM and movex=MOVE_RANDOM_TEACHABLE. NOT hooked into wild encounters or trainer parties — script gifts/statics only; but its filter primitives are directly reusable for an in-engine randomizer.
- **Trainer Party Pools — deterministic per-save trainer team variance with competitive-structure rules (lead/ace/weather tags, species clause, item clause), directly useful for gym variety without full randomization**
  - Where: `src/trainer_pools.c, src/data/battle_pool_rules.h, docs/tutorials/how_to_trainer_party_pool.md; configs B_POOL_SETTING_CONSISTENT_RNG, B_POOL_SETTING_USE_FIXED_SEED, B_POOL_SETTING_FIXED_SEED, B_POOL_RULE_SPECIES_CLAUSE, B_POOL_RULE_ITEM_CLAUSE etc. in include/config/battle.h (lines ~415-423)`
  - How: Give a trainer more defined mons than partySize (trainers.party: Party Size < defined mons); set B_POOL_SETTING_CONSISTENT_RNG TRUE for reproducible pools per save; tag mons LEAD/ACE/WEATHER_SETTER etc.
- **Per-purpose seedable RNG streams (sfc32) — clean building block for a deterministic seeded species mapping that does not disturb gameplay RNG**
  - Where: `include/random.h (LocalRandomSeed(u32) at line 66, RNG_* stream tags), src/random.c (SeedRng/SeedRng2)`
  - How: LocalRandomSeed(runSeed ^ speciesId) gives a pure, save-stable species->species map with zero global RNG impact.
- **SaveBlock precedent for storing a seed + save free-space guards**
  - Where: `gSaveBlock1Ptr->dailySeed (u32) at include/global.h:1124, written in src/clock.c:43; STATIC_ASSERT free-space checks at src/save.c:80-83`
  - How: Add u32 runSeed to SaveBlock1 next to dailySeed (4 bytes; asserts fail the build if space runs out).
- **Random legal trainer abilities**
  - Where: `B_TRAINER_MON_RANDOM_ABILITY in include/config/battle.h:359`
  - How: Set TRUE to give trainer mons a random legal ability.
- **All encounter/trainer data is machine-editable text — enables a cheap build-time randomizer (Path C)**
  - Where: `src/data/wild_encounters.json (~1 MB, all encounter tables), src/data/trainers.party (Showdown-export syntax trainer parties)`
  - How: A seeded Python script rewrites these two files pre-build; make then produces a fully GBA-legal randomized ROM.

### Reuse candidates

- **Universal Pokemon Randomizer ZX (Path A baseline)** — External GUI randomizer; fairness features worth copying as SPEC only: Similar Strength (±10% BST band, widening +5pp until ≥3 candidates — wiki Pokemon-Base-Statistics#similar-strength), type-themed areas/trainers, area/global 1-to-1 mapping, Limit Main-Game Legendaries (statics 20% band, expands downward only)
  - Source: https://github.com/Ajarmar/universal-pokemon-randomizer-zx
  - Port effort: XL / nonviable on expansion ROMs — UPR reads vanilla Gen-3 data layouts; upstream readme states 'Randomizing ROM hacks of the above games is not supported for the most part' (Dabomstew readme.txt lines 144-147); expansion rewrites every relevant structure (NUM_SPECIES ~1560 at include/constants/species.h:1697, new TrainerMon/SpeciesInfo formats) while keeping GAME_CODE BPEE (Makefile:3), so UPR would misdetect it as vanilla Emerald and corrupt data. GPLv3. Use its wiki algorithm as the fairness spec, not the tool.
- **Universal Pokemon Randomizer FVX (active UPR fork)** — Maintained UPR lineage (pushed 2026-07-27), same fairness settings evolved; would require authoring a brand-new expansion RomHandler in Java against offsets that shift every rebuild
  - Source: https://github.com/upr-fvx/universal-pokemon-randomizer-fvx
  - Port effort: XL — same structural incompatibility as ZX; per-build offset drift makes a ROM-level handler a treadmill (confirmed pain point that drove raffitz to abandon UPR internals). GPLv3.
- **Emerald Rogue species query engine (Path B prior art)** — Proven in-engine bitset query/filter system for building fair species pools on GBA (type/BST/legendary filters feeding seeded runs); repo active (pushed 2026-06-27)
  - Source: https://github.com/Pokabbie/pokeemerald-rogue (branches: expansion, expansion-dev; src/rogue_query.c, include/rogue_query.h, src/rogue_query_script.c)
  - Port effort: L — expansion-dev is pinned to expansion 1.7.4 vs our 1.16.3, and rogue_query is coupled to rogue_controller/rogue_settings subsystems; treat as reference architecture rather than drop-in. No license file (decomp-derived): ask Pokabbie's permission and credit if porting substantial code.
- **pokeemerald-expansion-rand (Path A/C hybrid prior art)** — Python post-build ROM randomizer that solves the offset problem by reading the ELF symbol table produced by the build; seeded, multiple ROMs per reference build; author explicitly built it because UPR internals were impractical for expansion
  - Source: https://github.com/raffitz/pokeemerald-expansion-rand
  - Port effort: M — concept (symbol-table-driven patching) is sound and reusable; code is unlicensed and dormant since 2021 on a pre-1.0 expansion, so expect rewrites; simpler to apply the same idea at source level (Path C) than to revive it.
- **Decomp Pokemon Randomizer (Kevinxde/Tervaxx)** — Source-level randomizer for pokeemerald/expansion: wild + static encounters, trainers, overworld items, evolutions, types/abilities — the closest prior art to Path C
  - Source: https://www.pokecommunity.com/threads/tool-decomp-pok%C3%A9mon-randomizer.451319/ (thread exists; PokeCommunity serves 403 to non-browser fetchers — verify in a browser)
  - Port effort: M — author-declared deprecated for recent expansion versions; the current trainers.party Showdown format and 1.16 wild_encounters.json would need adapter work; check thread for license/credit terms before reuse.
- **Cloudef/pokeemerald-randomizer** — GPLv3 seeded warp/wild/trainer randomizer for vanilla pokeemerald (ROM-patching, softlock-recovery ideas)
  - Source: https://github.com/Cloudef/pokeemerald-randomizer
  - Port effort: L — vanilla-targeted, dormant since 2022; value is reference-only (seeding UX, softlock escape hatch).

### Must build

- Path B core: seeded stable species->species mapping function (LocalRandomSeed hash + BST-band candidate search reusing IsInBstRangeFilterFunc logic) hooked into CreateWildMon (src/wild_encounter.c:466) — one choke point covers land/water/rock/fishing/outbreak
- Path B trainers: species remap + legal moveset/ability regeneration inside CreateNPCTrainerPartyFromTrainer (src/battle_main.c:1864), with capped retry loops (mirror FastPickRandomSpecies pattern) to stay in GBA CPU budget at battle start
- Run-seed plumbing: u32 runSeed in SaveBlock1, rolled (or player-entered) at New Game, shown in-game for shareable runs (4 bytes save impact)
- Coverage sweep for leak paths: dexnav.c:1184/1260, wild_encounter_ow.c:882 (Feebas/OW encounters), CreateScriptedWildMon (src/script_pokemon_util.c:117, scrcmd.c:2528) for statics/gifts — all must route through the same mapping
- Path C alternative: standalone seeded Python script implementing UPR-style similar-strength + type-theme logic over src/data/wild_encounters.json and src/data/trainers.party (no engine changes; rebuild per run)
- Populate src/data/random_mon_generator.h option arrays (ships empty) regardless of path, for randomized gift/static events

### Integration points

- src/wild_encounter.c (TryGenerateWildMon:480, CreateWildMon:466, outbreak path :556)
- src/battle_main.c (CreateNPCTrainerPartyFromTrainer:1864, CustomTrainerPartyAssignMoves:1839)
- src/random_mon_generation.c + src/data/random_mon_generator.h + include/constants/random_mon_generation.h
- src/dexnav.c:1184,1260 and src/wild_encounter_ow.c:882 (encounter paths that bypass TryGenerateWildMon)
- src/script_pokemon_util.c:117 / src/scrcmd.c:2528 (scripted statics and gifts)
- include/random.h (LocalRandomSeed) and include/global.h SaveBlock1 (runSeed next to dailySeed:1124; free-space asserts src/save.c:80-83)
- src/data/wild_encounters.json and src/data/trainers.party (Path C inputs)
- src/trainer_pools.c + include/config/battle.h B_POOL_* (gym variety without randomization)
- Cross-workstream: nuzlocke first-encounter/dupes logic (mapping stability determines dupes-clause behavior); gym-AI workstream (randomized teams degrade curated AI); in-game-knowledge workstream (Pokedex/DexNav area data must reflect the remap or it lies to the player)

### Risks

- Path A dead end confirmed: UPR mis-detects an expansion ROM as vanilla Emerald (GAME_CODE stays BPEE) and would corrupt rewritten data tables — any time spent trying settings there is wasted
- In-engine (B) leak paths: DexNav, Feebas spots, outbreaks, scripted statics and gift mons all create Pokemon outside the main wild pipeline; missing any one breaks nuzlocke first-encounter fairness and reveals the unrandomized game
- Trainer randomization without regenerating movesets/abilities from the new species' learnset yields illegal or trivially weak teams; regeneration adds per-battle CPU cost on GBA — must be bounded (capped retries, precomputed candidate pools)
- Conflict with sibling workstreams: fully randomized gym teams undermine the smarter-gym-AI and VGC-double-gym designs (AI synergy assumes curated teams); safest split is randomize wilds+route trainers, keep gyms on Trainer Party Pools
- License/permission: Emerald Rogue and raffitz's tool ship no license (decomp-derived) — need explicit permission + credit to port code; UPR/FVX/Cloudef are GPLv3 (algorithm ideas are fine; verbatim code would GPL-encumber the hack's source)
- Path C couples a run to its ROM build: rebuilding with a new seed mid-run invalidates run integrity — must stamp the seed into the ROM and the save and refuse mismatches (cheap check, but easy to forget)
- Save impact is trivial (u32) but SaveBlock1 headroom is shared with other workstreams' persistent features — coordinate via the STATIC_ASSERTs in src/save.c
- Small-pool BST bands at low levels (e.g., BST<300 replacements) can starve candidate searches; need UPR-style band widening or the search hangs/falls back to SPECIES_NONE

### Open decisions (owner's call)

- FOUNDATIONAL — pick randomization path (scored 1-5: fairness-control / effort / rerun-convenience / nuzlocke-compat): Path A external UPR = nonviable (0) on expansion ROMs, eliminated on evidence; Path B in-engine seeded randomizer = 5 fairness / effort L / 5 rerun (new seed = New Game, no PC needed, seed shareable) / 5 nuzlocke; Path C build-time Python over wild_encounters.json + trainers.party = 5 fairness / effort M (recommended if minimizing build cost; owner already builds from source) / 3 rerun (rebuild per run) / 5 nuzlocke. Hybrid also valid: C for trainers, B for wilds
- Mapping stability: global 1-to-1 species map per seed (dupes clause stays meaningful, player can learn the mapping — UPR 'Global 1-to-1') vs per-area mapping vs fully independent rolls (max variety, noisy dupes clause)
- Fairness knob defaults: BST band width (UPR default ±10% widening +5pp), keep type themes for areas/gyms or not, ban legendaries/mythicals/UBs/paradox (engine flags already exist), require evolution-capable replacements early game
- Randomization scope: wilds only / wilds + route trainers / also gyms+E4 (conflicts with hand-designed VGC double gyms — owner must arbitrate with the gym workstream)
- Build vs port for Path B: fresh implementation on 1.16.3's random_mon_generation.c primitives (clean, no permission needed) vs porting Emerald Rogue's rogue_query engine (proven but 1.7.4-era, permission required)
- Seed source UX: player-entered seed at New Game (re-runnable, shareable with viewers) vs auto Random32 with seed display in options/Trainer Card
