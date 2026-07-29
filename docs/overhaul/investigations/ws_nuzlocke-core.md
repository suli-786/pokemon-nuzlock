## Workstream: nuzlocke-core

**Overall effort:** M

### Already in the expansion (verified in repo)

- **Level caps (hard/soft EXP cap, badge-flag or var driven)**
  - Where: `B_EXP_CAP_TYPE, B_LEVEL_CAP_TYPE, B_LEVEL_CAP_VARIABLE, B_RARE_CANDY_CAP, B_LEVEL_CAP_EXP_UP in /home/suleiman/pokeemerald-expansion/include/config/caps.h; cap table sLevelCapFlagMap + GetCurrentLevelCap() in /home/suleiman/pokeemerald-expansion/src/caps.c:8-30`
  - How: Set B_EXP_CAP_TYPE to EXP_CAP_HARD (or EXP_CAP_SOFT) and B_LEVEL_CAP_TYPE to LEVEL_CAP_FLAG_LIST, then populate sLevelCapFlagMap with badge flags -> cap levels. B_RARE_CANDY_CAP TRUE blocks candy over-cap. No new code needed.
- **EV caps (optional hardcore extra)**
  - Where: `B_EV_CAP_TYPE, EV_CAP_NO_GAIN in /home/suleiman/pokeemerald-expansion/include/config/caps.h`
  - How: Set B_EV_CAP_TYPE (e.g. EV_CAP_NO_GAIN) — same flag-list mechanism as level caps.
- **Runtime catch-blocking flag (reusable as first-encounter enforcement gate)**
  - Where: `WE_FLAG_NO_CATCHING in /home/suleiman/pokeemerald-expansion/include/config/wild_encounter.h:16; enforced in /home/suleiman/pokeemerald-expansion/src/item_use.c:1155 (also P_ONLY_OBTAINABLE_SHINIES interaction src/pokemon.c:874, debug toggle src/debug.c:2602)`
  - How: Assign an unused flag ID; ball throws are then refused while set. A nuzlocke engine can set/clear it dynamically per route, though a custom check gives better messaging.
- **Wild-battle control flags (no encounters, no running, smart wild AI)**
  - Where: `WE_FLAG_NO_ENCOUNTER, WE_FLAG_NO_RUNNING, WE_SMART_WILD_AI_FLAG in /home/suleiman/pokeemerald-expansion/include/config/wild_encounter.h`
  - How: Assign flag IDs; useful for hardcore variants (no-run wilds) and scripted control.
- **Double wild battles (edge case the rules engine must handle)**
  - Where: `WE_DOUBLE_WILD_CHANCE, WE_FLAG_FORCE_DOUBLE_WILD, WE_DOUBLE_WILD_REQUIRE_2_MONS in /home/suleiman/pokeemerald-expansion/include/config/wild_encounter.h:9-14`
  - How: Config only; if enabled alongside nuzlocke, the engine must decide which of the two mons is 'the' encounter (see open decisions).
- **Catch-to-party swap prompt (base for auto-box-on-death UX)**
  - Where: `B_CATCH_SWAP_INTO_PARTY in /home/suleiman/pokeemerald-expansion/include/config/battle.h:351; flow in Cmd_givecaughtmon at /home/suleiman/pokeemerald-expansion/src/battle_script_commands.c:10131`
  - How: Already on (GEN_LATEST); Cmd_givecaughtmon is the single catch-success hook for marking a route's catch.
- **Free persistent storage for rule state (no SaveBlock surgery needed)**
  - Where: `381 FLAG_UNUSED slots in /home/suleiman/pokeemerald-expansion/include/constants/flags.h (e.g. FLAG_UNUSED_0x020 line 54); 29 VAR_UNUSED in include/constants/vars.h (0x40F7+ line 269); free bit unused_13:1 in BoxPokemon at include/pokemon.h:268; SaveBlock3 struct with headroom (max 1624 bytes) at include/global.h:256-274`
  - How: Rename unused flags/vars for nuzlocke state (zero save-size impact), or add a per-MAPSEC bitfield to SaveBlock3 (~28 bytes for all MAPSEC_COUNT sections) for a future-proof design.
- **Met-location already recorded per mon (dupes/first-encounter audit data)**
  - Where: `GetCurrentRegionMapSectionId() written to metLocation in /home/suleiman/pokeemerald-expansion/src/pokemon.c:1005; readable via MON_DATA_MET_LOCATION`
  - How: Free evidence trail of where each mon was caught; usable for retroactive rule checks and a graveyard/memorial UI.
- **Dupes-clause helper APIs**
  - Where: `GetSpeciesPreEvolution() at /home/suleiman/pokeemerald-expansion/src/pokemon.c:6684; GetSetPokedexFlag(dexNo, FLAG_GET_CAUGHT) declared in include/pokedex.h:14`
  - How: Walk pre-evolutions + check dex caught-flags to implement species-line dupes clause with no new data structures.
- **Debug menu flag/var toggles for testing rules**
  - Where: `/home/suleiman/pokeemerald-expansion/src/debug.c (e.g. WE_FLAG_NO_CATCHING toggle at line 2602); config include/config/debug.h`
  - How: Existing debug menu can flip nuzlocke flags/vars in-game for fast rule testing.

### Reuse candidates

- **NecroDingo nuzlocke-challenge branch (expansion-adapted, modular)** — Complete working engine on pokeemerald-expansion 1.13.3-dev: new src/nuzlocke.c (471 lines) + include/nuzlocke.h, hooks across 33 files. First-encounter per consolidated location (GetNuzlockeLocationId merges underwater/surface, cavern floors; 4 vars encounters + 4 vars catches = 64 locations each), dupes clause, shiny clause, permadeath via isDead:1 bit repurposing unused_13 in BoxPokemon + MON_DATA_IS_DEAD, in-battle catchability indicator sprite, ball-throw block, new-game mode selection (birch speech + main_menu.c), whiteout handling (disable-mode-with-autosave or continue), trainer card label. Uses FLAG_UNUSED_0x020 and VAR_UNUSED_0x40F7-0x40FE — all still free in our 1.16.3 tree (verified).
  - Source: https://github.com/NecroDingo/pokeemerald-expansion-dingo (branch: nuzlocke-challenge, 3 commits; docs: https://github.com/NecroDingo/pokeemerald-expansion-dingo/wiki/Nuzlocke-Challenge-Implementation)
  - Port effort: M — the branch is a patch-donor, not mergeable: 1.13.3->1.16.3 refactors (e.g. CreateWildMon now takes enum Species; battle_script_commands.c/battle_main.c drift) force re-applying every hook by hand; must also add hooks for post-1.13 encounter entry points (DexNav, OWEs). No license file (standard for decomp forks); credit NecroDingo + devolov in CREDITS, courtesy permission ask on GitHub recommended.
- **devolov's 'Add Nuzlocke Challenge' pret wiki tutorial** — The original design NecroDingo adapted, for vanilla pokeemerald, save-compatible: 5x16-bit route-seen vars, dead bit in BoxPokemon, HasWildPokmnOnThisRouteBeenSeen() 3-state check (catchable/used/dupe), location consolidation, plus extra optional clauses the branch lacks: no held items, no battle items, forced Set mode, Nurse Joy announces level cap, catchability indicator. Documents the known Sacred Ash edge case.
  - Source: https://github.com/pret/pokeemerald/wiki/Add-Nuzlocke-Challenge
  - Port effort: S as design reference / cherry-pick source for the extra clauses on top of the NecroDingo port (M if used standalone from vanilla). Public wiki content; credit devolov.
- **Native expansion level-cap system (upstream, already merged)** — Level caps keyed to badge flags with hard/soft EXP modes, rare-candy cap, catch-up EXP boost — replaces the tutorial-era level-cap hacks entirely.
  - Source: https://github.com/rh-hideout/pokeemerald-expansion (in-tree: include/config/caps.h, src/caps.c)
  - Port effort: S — configuration only, zero porting; upstream code already in the working tree.

### Must build

- Auto-box/release on death: NecroDingo/devolov keep dead mons in party (isDead flag); owner wants them removed — add battle-end party sweep (hook FreeResetData_ReturnToOvOrDoEvolutions, src/battle_main.c:5572) moving HP==0 mons to a designated graveyard PC box via CopyMonToPC (src/pokemon.c:3006), plus the field-poison faint path (src/field_poison.c:44)
- Nuzlocke settings menu: per-run toggles (dupes clause, shiny clause, gift/static policy, failed-encounter policy, level-cap mode) — branch only has a single on/off at new game; store choices in one bitmask var
- Gift/static/roamer/Safari policy enforcement: no prior art handles these; needs a policy check in ScriptGiveMon (src/script_pokemon_util.c:479) for gifts and encounter-classification for statics/roamers/Safari
- Hook coverage for post-1.13 encounter entry points: DexNav encounters (CreateWildMon calls at src/dexnav.c:1184 and 1260) and Overworld Wild Encounters (WE_OW_ENCOUNTERS) must mark/check first-encounter state
- Revive blocking under permadeath: gate Revive/Max Revive/Revival Herb/Sacred Ash in item_use + remove from mart stock; fix the documented Sacred Ash crash edge when dead mon in party
- Level-cap table for the gym progression: populate sLevelCapFlagMap in src/caps.c with this hack's badge flags and chosen cap levels (config work + balancing, trivial code)

### Integration points

- /home/suleiman/pokeemerald-expansion/src/wild_encounter.c — CreateWildMon (line 466, single choke point all wild generation passes through), TryGenerateWildMon (480), FishingWildEncounter (925), RockSmashWildEncounter (790), TryStartRoamerEncounter call sites (703/754/876/899), Safari checks (447, 1176)
- /home/suleiman/pokeemerald-expansion/src/battle_script_commands.c:10131 — Cmd_givecaughtmon (catch-success: mark route used, dupes bookkeeping)
- /home/suleiman/pokeemerald-expansion/src/item_use.c:1155 — ball-throw gate (first-encounter enforcement point)
- /home/suleiman/pokeemerald-expansion/src/battle_main.c:5572 — FreeResetData_ReturnToOvOrDoEvolutions (battle-end permadeath sweep)
- /home/suleiman/pokeemerald-expansion/src/field_poison.c — FaintFromFieldPoison/MonFaintedFromPoison (out-of-battle death path)
- /home/suleiman/pokeemerald-expansion/src/script_pokemon_util.c:479 — ScriptGiveMon (gift-mon policy)
- /home/suleiman/pokeemerald-expansion/src/dexnav.c:1184,1260 — DexNav encounter generation (extra entry point to hook)
- /home/suleiman/pokeemerald-expansion/include/config/caps.h + src/caps.c — level caps
- /home/suleiman/pokeemerald-expansion/include/pokemon.h:268 (unused_13 bit), include/constants/flags.h, include/constants/vars.h, include/global.h:256 SaveBlock3 — persistent state
- /home/suleiman/pokeemerald-expansion/src/main_menu.c + src/new_game.c — mode selection at new game
- /home/suleiman/pokeemerald-expansion/src/battle_interface.c — catchable/used indicator in battle UI (feeds the in-game-knowledge workstream)
- Randomizer workstream — its species-replacement hook must run before nuzlocke marking/dupes checks in CreateWildMon

### Risks

- Version drift is the main cost: donor branch is expansion 1.13.3, ours is 1.16.3 — battle_script_commands.c/battle_main.c/main_menu.c refactored (enum Species signatures etc.); every hook must be re-applied manually; a naive git merge will conflict everywhere
- Ordering with the randomizer: first-encounter marking and dupes checks read the species inside CreateWildMon — if the randomizer swaps species after marking, dupes/shiny clauses evaluate the wrong mon
- Double wild battles vs first-encounter rule: two mons generated at once; NecroDingo sidesteps by forcing singles on first encounters — conflicts with the owner's doubles-heavy design if wild doubles are enabled
- Event-var storage caps tracking at 64 consolidated locations per set; a Hoenn+extras hack near/over that needs the SaveBlock3 per-MAPSEC bitfield instead (~28 bytes for MAPSEC_COUNT) — decide before save format ossifies
- isDead consumes the last free BoxPokemon bit (unused_13) — collides with any other workstream wanting that bit; auto-release variant avoids it but loses graveyard/memorial data
- Roamers and fleeing encounters: if a roamer or fled mon burns the route slot, player loses catches through no fault; needs explicit classification
- Safari Zone uses a different ball flow than the bag gate at item_use.c:1155 — needs its own hook or Safari exempted
- Permadeath is only as strong as revive access: revives from marts, pickup, hidden items and Sacred Ash all need gating; Sacred Ash has a known crash edge with dead party mons (documented in devolov's tutorial)
- Save-scumming cannot be prevented on GBA hardware (soft-reset before autosave); only mitigable with autosave-on-death, which itself needs the save workstream's cooperation
- No license on either reuse source (decomp community norm) — ship with CREDITS entries for devolov and NecroDingo; permission ask is courtesy, not legal requirement, but owner said to note it

### Open decisions (owner's call)

- Dead mon disposal: (a) stay in party flagged dead/unusable (donor behavior, keeps memorial visible), (b) auto-move to a graveyard PC box at battle end, (c) forced release (data gone). Owner asked for auto-box/release — pick b or c, and whether a memorial UI is wanted
- First-encounter miss policy: if the first wild mon is KO'd or flees, is the route slot burned (hardcore, donor behavior marks on encounter) or retryable until a catch (casual, mark only on catch)? Donor tracks both bits so either is cheap — choose default and whether it's a setting
- Dupes clause scope: same species only, whole evolution line (via GetSpeciesPreEvolution), or dex-caught check; and does a dupe first encounter burn the slot or defer to next non-dupe
- Statics/gifts/roamers: each counts as the location's encounter — yes/no per category; gifts banned entirely, always allowed, or allowed-only-if-first; roamer flee = slot burned or exempt
- Safari Zone: inside nuzlocke rules (one catch attempt total) or exempt zone
- Level cap: EXP_CAP_HARD vs EXP_CAP_SOFT vs none; B_RARE_CANDY_CAP on/off; caps keyed to badges (LEVEL_CAP_FLAG_LIST) vs script-driven var (LEVEL_CAP_VARIABLE)
- Shiny clause default: on (shinies always catchable, never burn slot — donor behavior) or off
- Whiteout consequence: run over with locked/deleted save, run over but free-play continues, or nuzlocke mode auto-disables with autosave (donor offers the last two)
- Double wild battles when enabled: first mon generated counts, player picks by catching either, or wild doubles suppressed on unvisited routes (donor behavior)
- State storage: rename unused vars/flags (zero risk, 64-location ceiling, donor-compatible) vs new SaveBlock3 bitfield (~28-56 bytes, unlimited locations, cleaner for randomized multi-region) — decide before first real run since it defines save layout
- Credits/permission: CREDITS.md entries for devolov + NecroDingo are mandatory either way; whether to ask NecroDingo before shipping a derived branch is owner's call
