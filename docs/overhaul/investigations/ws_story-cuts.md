## Workstream: story-cuts

**Overall effort:** M

### Already in the expansion (verified in repo)

- **Quickstart: press SELECT on title screen to start a new game, skipping main menu + entire Birch speech/naming (auto-named BRENDAN/MAY, gender configurable). Expansion version in this tree: 1.16.3-dev (include/constants/expansion.h). Player still lands in the truck intro.**
  - Where: `include/config/quickstart.h (ENABLE_QUICKSTART, QUICKSTART_GENDER, QUICKSTART_HUD); src/quickstart.c (CB2_SkipToNewGame -> CB2_NewGame); docs/tutorials/quickstart.md`
  - How: ENABLE_QUICKSTART is already TRUE by default. CAVEAT: force-disabled on release builds — include/quickstart.h '#if RELEASE #define QUICKSTART FALSE' (RELEASE set by Makefile line 160-161 via -DRELEASE). Ship a non-release build or delete the guard.
- **Debug menu 'Cheat start' — the exact in-repo template for skipping the whole Littleroot intro via flags/vars: sets FLAG_RESCUED_BIRCH, FLAG_ADVENTURE_STARTED, FLAG_SYS_POKEMON_GET, VAR_LITTLEROOT_INTRO_STATE=7, VAR_LITTLEROOT_TOWN_STATE=4, VAR_ROUTE101_STATE=3, VAR_BIRCH_LAB_STATE=2, VAR_LITTLEROOT_RIVAL_STATE=4, Pokedex+National Dex, running shoes, bike, all 8 badges, and the Mr. Briney sequence-break preset (VAR_BRINEY_HOUSE_STATE=1, VAR_ROUTE116_STATE=2, FLAG_HIDE_ROUTE_116_MR_BRINEY, clearflag FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY/PEEKO).**
  - Where: `data/scripts/debug.inc lines 11-70 (Debug_CheatStart); wired at src/debug.c:588 (DebugAction_Util_CheatStart); menu enabled by DEBUG_OVERWORLD_MENU in include/config/debug.h (DISABLED_ON_RELEASE, hold R + press START)`
  - How: Usable today in dev builds: R+START -> Utilities -> Cheat start. For the overhaul, copy/trim this block into the New Game path instead of the debug menu.
- **Single New Game injection point where every new save already runs a script — the natural place to pre-set 'Aqua/Magma arcs complete' flags with zero new C code.**
  - Where: `data/scripts/new_game.inc line 115 (EventScript_ResetAllMapFlags), executed by src/new_game.c:218 NewGameInitData() via RunScriptImmediately; truck warp is src/new_game.c:136-143 WarpToTruck()`
  - How: Append setflag/setvar block to EventScript_ResetAllMapFlags (or a called sub-script). To start at starter selection, also repoint WarpToTruck() to Route 101/Birch's Lab and reuse the 'special ChooseStarter' pattern from data/maps/Route101/scripts.inc:218-237 (Route101_EventScript_BirchsBag: special ChooseStarter -> waitstate -> first battle -> setvar VAR_ROUTE101_STATE, 3).
- **Debug Flags/Vars editor + full overworld debug menu for live-testing every sequence-break in the gate map without rebuilding.**
  - Where: `include/config/debug.h (DEBUG_OVERWORLD_MENU, DEBUG_OVERWORLD_HELD_KEYS); src/debug.c`
  - How: Already on in dev builds (R+START). Use to hand-verify each pre-set flag's overworld effect before baking into new_game.inc.
- **Wall-clock decoupling: skipping the intro also skips the mandatory bedroom clock-set step (VAR_LITTLEROOT_INTRO_STATE=4 blocks stairs, data/maps/LittlerootTown_BrendansHouse_2F/scripts.inc:9). Expansion offers a fake RTC so real-clock setup can be irrelevant.**
  - Where: `include/config/overworld.h:93 (OW_USE_FAKE_RTC) and OW_FLAG_PAUSE_TIME:110; FLAG_SET_WALL_CLOCK = 0x51 in include/constants/flags.h:105`
  - How: Either 'setflag FLAG_SET_WALL_CLOCK' in the New Game preset, or set OW_USE_FAKE_RTC TRUE. One of the two is required or RTC-driven content (berries, Shoal Cave, lottery, day/night) misbehaves.

### Reuse candidates

- **Debug_CheatStart flag recipe (upstream RHH expansion, already in tree)** — Maintained, tested flag/var list for 'intro complete' state incl. the non-obvious Briney preset; extend it with the arc-complete block rather than deriving from scratch
  - Source: https://github.com/rh-hideout/pokeemerald-expansion
  - Port effort: S — copy/trim an .inc script within the same repo; no license issue (already the project base, credit RHH per repo norms)
- **Archipelago Emerald base fork (monhacks/emerald-archipelago)** — Battle-tested randomizer-oriented event streamlining on vanilla pokeemerald scripts: Wally catch-tutorial skip, roadblock/story event handling, instant text, run-anywhere — a proven checklist of which vanilla events randomizer players want cut (see https://archipelago.gg/games/Pokemon%20Emerald/info/en for the change list)
  - Source: https://github.com/monhacks/emerald-archipelago
  - Port effort: M — built on pret pokeemerald, not expansion; script-level diffs port conceptually but not verbatim. pret-derived repos carry no formal license: credit monhacks/Zunawe and ask permission before lifting code wholesale
- **Emerald Rogue (Pokabbie)** — Reference architecture for a hub-start game with the entire vanilla story removed (roguelike Emerald, public source); useful as a design map of what can be deleted vs must be neutralized
  - Source: https://github.com/pokabbie/pokeemerald-rogue
  - Port effort: XL — total conversion on an older base; use as reference reading only, or S if only consulting. Credit/permission from Pokabbie required if any code is copied
- **pret wiki: Option to Skip Copyright and Intro** — Skips the pre-title copyright/intro movie (src/intro.c side) — the one segment Quickstart does not cover; listed on https://github.com/pret/pokeemerald/wiki/Tutorials
  - Source: https://github.com/pret/pokeemerald/wiki/Option-to-Skip-Copyright-and-Intro
  - Port effort: S — one-star wiki tutorial, small C edit; wiki tutorials are freely reusable with author credit

### Must build

- Arc-complete New Game preset script (~35 setflag/setvar lines appended to EventScript_ResetAllMapFlags) covering the verified gate map: FLAG_HIDE_ROUTE_110_TEAM_AQUA (normally SlateportCity_OceanicMuseum_2F/scripts.inc:79 - blocks badge 3), FLAG_HIDE_ROUTE_112_TEAM_MAGMA (MeteorFalls_1F_1R/scripts.inc:86 - blocks badge 4), FLAG_HIDE_ROUTE_119_TEAM_AQUA (Route119_WeatherInstitute_2F/scripts.inc:66 - blocks badge 6), FLAG_HIDE_LILYCOVE_CITY_AQUA_GRUNTS (AquaHideout_B2F/scripts.inc:50 - blocks badge 7), FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE, FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT + VAR_SLATEPORT_CITY_STATE=1/VAR_SLATEPORT_HARBOR_STATE=2 (MagmaHideout_4F/scripts.inc:61-63, SlateportCity_Harbor/scripts.inc:64), VAR_MOSSDEEP_SPACE_CENTER_STATE=3 + FLAG_DEFEATED_MAGMA_SPACE_CENTER + VAR_STEVENS_HOUSE_STATE=1 (Dive gate: MossdeepCity_StevensHouse/scripts.inc:25 OnFrame gives ITEM_HM_DIVE), Devon-goods chain FLAG_DEVON_GOODS_STOLEN/RECOVERED/RETURNED/DELIVERED + FLAG_DELIVERED_STEVEN_LETTER + FLAG_RECEIVED_POKENAV (Briney sailing branch: Route104_MrBrineysHouse/scripts.inc:26-27), Sootopolis end-state VAR_SOOTOPOLIS_CITY_STATE=6 + FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN + FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE (Waterfall gate: SootopolisCity/scripts.inc:1272-1287) + clearflag FLAG_HIDE_SOOTOPOLIS_CITY_RESIDENTS/WALLACE (normally SeafloorCavern_Room9:120, SkyPillar_Outside:84), VAR_ROUTE128_STATE=2, VAR_SEAFLOOR_CAVERN_STATE=1, VAR_SKY_PILLAR_STATE=3 (postgame Rayquaza catchable: SkyPillar_Top/scripts.inc:18-19 needs >=2), VAR_MT_PYRE_STATE=3, VAR_PETALBURG_WOODS_STATE=1, VAR_RUSTURF_TUNNEL_STATE=3, VAR_WEATHER_INSTITUTE_STATE=2 + clearflag FLAG_HIDE_WEATHER_INSTITUTE_1F_WORKERS (postgame Terra/Marine Cave trigger lives on these NPCs: Route119_WeatherInstitute_2F/scripts.inc:128)
- Start-at-starter flow: repoint WarpToTruck() (src/new_game.c:136) to Birch's Lab or Route 101, add small ON_FRAME script calling 'special ChooseStarter' (clone of Route101_EventScript_BirchsBag, data/maps/Route101/scripts.inc:218-237), plus setflag FLAG_SET_WALL_CLOCK; pair with intro-complete vars from Debug_CheatStart lines 12-35 (minus badges/bike)
- Remove RELEASE guards if shipping a release build: include/quickstart.h #if RELEASE and DISABLED_ON_RELEASE in include/config/debug.h
- Compensation drops for cut arc rewards: Exp. Share (FLAG_RECEIVED_EXP_SHARE), Castform (FLAG_RECEIVED_CASTFORM path in Weather Institute), Meteorite/TM from Cozmo, and level-curve rebalance for ~15 removed Aqua/Magma scripted battles (hand off to difficulty/randomizer workstreams)
- Cosmetic cleanup pass (optional): Match Call/TV/NPC dialogue that references skipped beats

### Integration points

- data/scripts/new_game.inc (EventScript_ResetAllMapFlags — primary injection point)
- src/new_game.c (NewGameInitData:161-237, WarpToTruck:136)
- src/quickstart.c, include/quickstart.h, include/config/quickstart.h
- src/main_menu.c (Task_NewGameBirchSpeech_* chain — bypassed by Quickstart)
- data/scripts/debug.inc (Debug_CheatStart), src/debug.c, include/config/debug.h
- data/maps/Route101/scripts.inc + src/starter_choose.c (special ChooseStarter; VAR_STARTER_MON consumed by rival battles in data/maps/Route103, Route110, Route119, LilycoveCity scripts)
- include/constants/flags.h, include/constants/vars.h (all gating constants)
- Arc beat scripts (verified gate sites): data/maps/{PetalburgWoods, RustboroCity, RusturfTunnel, Route104_MrBrineysHouse, SlateportCity, SlateportCity_OceanicMuseum_1F/2F, SlateportCity_Harbor, MeteorFalls_1F_1R, MtPyre_Summit, MagmaHideout_4F, AquaHideout_B2F, MossdeepCity_Gym, MossdeepCity_SpaceCenter_2F, MossdeepCity_StevensHouse, SeafloorCavern_Room9, Route128, SootopolisCity, CaveOfOrigin_B1F, SkyPillar_Outside, SkyPillar_Top, SootopolisCity_Gym_1F, Route119, Route119_WeatherInstitute_2F}/scripts.inc
- data/scripts/abnormal_weather.inc + src/field_specials.c:3517-3600 (postgame Groudon/Kyogre) and data/scripts/hall_of_fame.inc:14-15 (SS Tidal unlock — game-clear gated, unaffected)
- include/config/overworld.h (OW_USE_FAKE_RTC / FLAG_SET_WALL_CLOCK interaction)
- include/constants/expansion.h (1.16.3-dev baseline for any upstream cherry-picks)

### Risks

- Hard-lock chain: 6 arc beats are physical progression gates — Briney boat (badge 2, Devon flags), Route 110 grunt (badge 3), Route 112 Magma (badge 4/Lavaridge), Route 119 grunt (badge 6/Fortree), Lilycove grunts (badge 7/Mossdeep), HM Dive via VAR_STEVENS_HOUSE_STATE and HM Waterfall via FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE (Seafloor/Sootopolis/Victory Road). Missing any one preset = unwinnable save; nuzlocke rules make restarts extra costly
- Sootopolis is a 6-state machine (VAR_SOOTOPOLIS_CITY_STATE 0-6) with companion flags and layout/weather branches (SootopolisCity/scripts.inc:36-89); a partial preset leaves ghost NPCs, downpour weather, or a locked gym
- Postgame legendaries depend on skipped-arc residue: Terra/Marine Cave abnormal weather requires Weather Institute workers visible + FLAG_SYS_GAME_CLEAR; Sky Pillar Rayquaza needs VAR_SKY_PILLAR_STATE>=2 and FLAG_DEFEATED_RAYQUAZA unset; Red/Blue Orb held items (expansion primal reversion) have NO obtain script even in vanilla — separate itemization gap
- givemon/additem inside EventScript_ResetAllMapFlags executes headless via RunScriptImmediately before the overworld exists — Debug_CheatStart is only ever run in-field, so the New Game context needs an emulator test pass (party/bag writes should work; UI-dependent commands will not)
- Quickstart and the debug menu are compiled out when RELEASE=1 (include/quickstart.h, DISABLED_ON_RELEASE) — decide build mode early or the shipped ROM silently loses the feature
- XP/economy skew: cutting ~15 Aqua/Magma scripted fights (incl. Archie, Maxie x2, Matt, Tabitha, Courtney, Steven multi) removes a large exp source mid-game; level caps/curve work in the difficulty workstream must account for it
- Story flags double as content keys: any future custom content keyed on 'FLAG_GROUDON_AWAKENED...' etc. cannot distinguish skipped from played; document the preset as canonical state
- This tree dual-targets FRLG (IS_FRLG branches in src/new_game.c, Debug_CheatStartFrlg) — edits to shared new-game code must guard with IS_FRLG or break the other target
- No SaveBlock impact (all existing flags/vars; zero new persistent fields) — but that also means no room to 'remember' pre-skip state if the owner later wants arcs re-enableable per-save

### Open decisions (owner's call)

- Skip granularity: (a) full arc-complete at New Game — fastest runs, loses all Aqua/Magma battles+rewards; (b) neutralize roadblocks only (pre-set the 6 blocker flags, leave hideouts/Mt. Chimney as optional dungeons); (c) hybrid keeping ONE beat — the Mossdeep Space Center Steven multi battle (fits the VGC-doubles theme; it self-arms via MossdeepCity_Gym:84 after badge 7, so keeping it just means not pre-setting its 3 flags)
- New game entry point: (a) Quickstart as-is (truck+Littleroot intro still ~3 min); (b) skip to bedroom with clock pre-set; (c) skip straight to starter selection via ChooseStarter special; (d) skip past starter too — randomizer grants the starter and player spawns at Littleroot exit
- Where cut rewards land: grant Exp Share/PokeNav/Bike/HMs Dive+Waterfall at New Game (max time-save, trivializes early routes) vs relocate onto existing NPCs/marts (preserves progression feel) — must be co-decided with the randomizer/itemization workstream
- Build mode: ship dev build (keeps Quickstart + debug menu + Cheat start for free) vs release build with guards manually removed (cleaner, but each debug convenience must be explicitly ported)
- Climax handling: pre-resolve Kyogre/Groudon/Rayquaza (Rayquaza immediately catchable at Sky Pillar, Sootopolis open from the start) vs keep the Sootopolis crisis as the single surviving story arc gating gym 8 (Juan) — affects whether Wallace/Waterfall and gym 8 need alternate unlock scripting
