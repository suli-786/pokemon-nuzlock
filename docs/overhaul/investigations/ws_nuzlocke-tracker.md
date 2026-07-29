## Workstream: nuzlocke-tracker

**Overall effort:** M

### Already in the expansion (verified in repo)

- **SaveBlock3 with ~1620 bytes free (measured: sizeof=4 of 1624 capacity with default configs) — ideal tracker storage; stored in otherwise-unused per-sector chunk space so it costs zero extra flash sectors**
  - Where: `SAVE_BLOCK_3_CHUNK_SIZE 116 x NUM_SECTORS_PER_SLOT 14 in include/save.h; struct SaveBlock3 in include/global.h:256-274; size guard STATIC_ASSERT(sizeof(struct SaveBlock3) <= ...) in src/save.c:80`
  - How: Append tracker fields to struct SaveBlock3; the static asserts fail the build if you overflow. Verified headroom by compiling against this tree with repo flags (-mabi=apcs-gnu).
- **Reclaimable save space via FREE_* configs: 2516 B in SaveBlock1 + 1274 B in SaveBlock2 (single-player-irrelevant data: mystery gift/event, union room chat, e-reader, link records, record mixing, match call)**
  - Where: `include/config/save.h — FREE_MYSTERY_GIFT (876 B), FREE_MYSTERY_EVENT_BUFFERS (1104 B), FREE_UNION_ROOM_CHAT (212 B), FREE_MATCH_CALL (104 B), FREE_LINK_BATTLE_RECORDS (88 B), FREE_RECORD_MIXING_HALL_RECORDS (1032 B), etc.`
  - How: Set desired FREE_* defines to TRUE; fallback storage if SaveBlock3 gets crowded. Measured baseline: SaveBlock1 15568/15872 B (304 free), SaveBlock2 3884/3968 B (84 free).
- **Per-route keying: every map has a region-map-section id; ~90 Hoenn sections (MAPSEC_LITTLEROOT_TOWN..MAPSEC_DYNAMIC), ~218 total incl. Kanto**
  - Where: `include/constants/region_map_sections.h (auto-generated enum); gMapHeader.regionMapSectionId at include/global.fieldmap.h:234`
  - How: Index tracker records by regionMapSectionId of the current map — 4 bits/section x 90 = 45 bytes for full status tracking.
- **Run-stats storage already exists: u32 gameStats[64] in SaveBlock1 with 11 unused slots**
  - Where: `gameStats[NUM_GAME_STATS] at include/global.h:1134; NUM_USED_GAME_STATS 53 / NUM_GAME_STATS 64 in include/constants/game_stat.h:58-59`
  - How: Define new GAME_STAT_* ids 53-63 (deaths, encounters caught, whiteouts avoided...) and use IncrementGameStat — zero save-layout change.
- **Start menu is trivially extensible; DexNav already sets the precedent of a conditional custom entry**
  - Where: `sStartMenuItems[] at src/start_menu.c:192, MENU_ACTION_DEXNAV added via AddStartMenuAction() at src/start_menu.c:338`
  - How: Clone the DexNav pattern for a NUZLOCKE entry gated on a run-active flag.
- **Reusable list/menu UI frameworks for the tracker screen**
  - Where: `src/list_menu.c (generic scrolling list), src/pokenav_list.c (PokeNav scrolling list w/ sprites, drives match call + ribbons lists), src/debug.c (window+list overworld menu pattern, DEBUG_OVERWORLD_MENU in include/config/debug.h:5)`
  - How: Build the route-status list and graveyard as list_menu/pokenav_list consumers instead of a bespoke renderer.
- **PokeNav app framework with free slots (5 used of MAX_POKENAV_MENUITEMS 6) and a PokeNav region map for a map-overlay tracker view**
  - Where: `POKENAV_MENUITEM_* enum at include/pokenav.h:150-157, MAX_POKENAV_MENUITEMS at include/pokenav.h:167; src/pokenav_menu_handler.c; src/pokenav_region_map.c; src/region_map.c`
  - How: Add a POKENAV_MENUITEM_NUZLOCKE app, or recolor region-map sections by encounter status.
- **Optional HGSS-style Pokedex that could host a per-route status page**
  - Where: `POKEDEX_PLUS_HGSS FALSE in include/config/pokedex_plus_hgss.h:4; src/pokedex_plus_hgss.c`
  - How: Enable the config; extend one of its pages — only worthwhile if the owner adopts this dex for the UI-overhaul workstream.
- **Battle-outcome data needed to log encounter results already exposed**
  - Where: `B_OUTCOME_WON/RAN/CAUGHT in include/constants/battle.h:165-171; gBattleOutcome consumed post-battle in src/battle_main.c (~line 5578, where DexNav already hooks the same spot); BattleSetup_StartWildBattle at src/battle_setup.c:332`
  - How: Write tracker record on battle end: CAUGHT->caught, WON->killed, RAN/teleport->fled; first-encounter detection at wild battle start.
- **Spare event flags/vars for cheap toggles (run-active, tracker-unlocked)**
  - Where: `381 FLAG_UNUSED_* in include/constants/flags.h; 29 VAR_UNUSED_* in include/constants/vars.h`
  - How: Rename an unused flag/var; no layout change.

### Reuse candidates

- **TheXaman tx_randomizer_and_challenges (nuzlocke enforcement + per-route encounter flags + challenges menu UI)** — Proven per-mapsec encounter tracking (u8 NuzlockeEncounterFlags[9] in SaveBlock1; NuzlockeFlagGet/Set/Clear(u16 mapsec) — verified in branch's include/global.h:1058 and tx_randomizer_and_challenges.h), dupes/shiny clause, in-battle 'catchable' indicator icon, and a full challenges option-menu UI (TX_RAC_FEATURES.md in branch root)
  - Source: https://github.com/TheXaman/pokeemerald/tree/tx_randomizer_and_challenges
  - Port effort: M — storage scheme and flag API port in hours, but branch is vanilla-pret-based (7592 commits) so battle/menu hooks must be re-placed by hand onto expansion 1.16 files that diverged heavily. No formal license (pret-derived); credit TheXaman per community convention.
- **Modern Emerald (resetes12) — maintained expansion-era port of the tx_ system with an in-game CHALLENGES viewer** — src/tx_randomizer_and_challenges.c, src/tx_rac_menu.c, src/tx_rac_viewer.c (PC 'CHALLENGES' status-viewer screen — closest existing thing to an in-game tracker UI), graphics/battle_interface/nuzlocke_indicator.png, EASY/NORMAL/HARDCORE nuzlocke modes (verified in repo tree + README; pushed 2026-07, actively maintained)
  - Source: https://github.com/resetes12/pokeemerald
  - Port effort: M — same caveat: base is vanilla pokeemerald, not expansion, so this is cherry-pick material not a merge. No LICENSE file; ask resetes12 for asset reuse (indicator png) and credit both resetes12 and TheXaman.
- **pret wiki 'Add Nuzlocke Challenge' tutorial (devolov)** — Cheapest storage design: 5 game vars as a caught-per-route bitfield + a 'dead' bit in BoxPokemon's unused personality bits, giving an unlimited PC-box graveyard with zero SaveBlock growth; hook locations for encounter validation and faint handling
  - Source: https://github.com/pret/pokeemerald/wiki/Add-Nuzlocke-Challenge
  - Port effort: S — it is a tutorial (adapt, don't port); written for vanilla but the hooks (BattleSetup_StartWildBattle, faint scripts) exist by the same names in this tree. Credit devolov (+ Deokishisu for icon concept).
- **Emerald Rogue (Pokabbie), 'expansion' branch — custom full-screen menu/quest-log architecture on an expansion base** — Reference architecture for run-based games on pokeemerald-expansion: rogue_questmenu.c/h custom menu screens, run lifecycle state, hub stats (verified branches 'expansion'/'expansion-dev' and include/rogue_questmenu.h etc. in tree)
  - Source: https://github.com/Pokabbie/pokeemerald-rogue
  - Port effort: L — code is deeply entangled with Rogue's run loop; use as a pattern reference for the tracker screen rather than lifting files. Credit Pokabbie if any code is copied.

### Must build

- The tracker screen itself: route list with caught/killed/fled/missed status icons + graveyard page + run-stats page — no community project ships this UI; compose it from src/list_menu.c or src/pokenav_list.c (~the bulk of this workstream)
- struct SaveBlock3 additions: u8 routeStatus[45] (4 bits x ~90 Hoenn mapsecs) + struct GraveyardEntry {u16 species; u8 nickname[11]; u8 level; u8 mapsec; u8 cause;} x N (16 B each, 32 entries = 512 B)
- Event hooks writing records: post-battle outcome logger (gBattleOutcome switch), party-mon-fainted graveyard append, first-encounter marker at BattleSetup_StartWildBattle (small, shared with the enforcement workstream)
- Optional: region-map overlay recoloring MAPSECs by encounter status in src/region_map.c / src/pokenav_region_map.c
- Optional: PokeNav app icon graphics if the PokeNav surface is chosen (menu icon sheet is compressed gfx — real art/tooling work)

### Integration points

- include/global.h (struct SaveBlock3, line 256) + src/save.c size asserts (lines 80-83)
- include/config/save.h (FREE_* space reclamation)
- src/battle_setup.c BattleSetup_StartWildBattle (line 332) — first-encounter detection
- src/battle_main.c post-battle gBattleOutcome handling (~line 5578, alongside existing DexNav hook)
- src/battle_script_commands.c faint handling — graveyard append
- src/start_menu.c sStartMenuItems (line 192) / AddStartMenuAction (line 338)
- include/pokenav.h + src/pokenav_menu_handler.c + src/pokenav_list.c (if PokeNav app surface)
- src/region_map.c, src/pokenav_region_map.c (map overlay option)
- include/constants/region_map_sections.h + gMapHeader.regionMapSectionId (include/global.fieldmap.h:234)
- include/constants/game_stat.h + gameStats in SaveBlock1 (run stats)
- src/list_menu.c (tracker list rendering)

### Risks

- SaveBlock3 space contention: USE_DEXNAV_SEARCH_LEVELS (include/config/dexnav.h:5) costs 1 byte per species (~1500+) and alone overflows the 1624-byte SaveBlock3 — it is mutually exclusive with a SaveBlock3-resident tracker; FNPC_ENABLE_NPC_FOLLOWERS, OW_USE_FAKE_RTC, and OW_SHOW_ITEM_DESCRIPTIONS=FIRST_TIME also eat SaveBlock3. Coordinate with the DexNav/QoL workstream before choosing storage.
- Any SaveBlock field addition shifts offsets and bricks existing saves. Acceptable for fresh randomized runs, but pulling future upstream expansion updates mid-run can silently add SaveBlock3 fields (upstream did for apricorn trees, followers) — pin the base commit for the duration of a run.
- TheXaman/Modern Emerald code is vanilla-pret-based; expansion 1.16 battle and menu internals have diverged heavily, so reuse is manual cherry-picking — budget for hook re-placement, not a git merge.
- gBattleOutcome edge cases: wild mon fleeing (roamers, Safari Zone, Run Away ability) vs player fleeing both end in B_OUTCOME_RAN from different actors; killed-vs-fled classification needs per-case handling or statuses will be logged wrong.
- License/permission: pret-derived community code has no formal license — credit is community convention (TheXaman, devolov, resetes12, Pokabbie); explicitly ask before shipping Modern Emerald's nuzlocke_indicator.png artwork.
- PokeNav surface is capped at MAX_POKENAV_MENUITEMS 6 (5 used) and needs new compressed icon gfx — the start-menu surface has none of these constraints.

### Open decisions (owner's call)

- Where the tracker lives: (a) start menu entry — cheapest, always accessible, DexNav precedent; (b) PokeNav app — thematic, uses last-but-one free slot and needs icon art; (c) HGSS Pokedex page — only if the HGSS dex is adopted elsewhere. (a) now with (b) later is viable.
- Storage home: (a) SaveBlock3 (1620 B free, zero flash cost) — forfeits DexNav search levels; (b) SaveBlock1 after flipping FREE_MYSTERY_GIFT/FREE_MYSTERY_EVENT_BUFFERS etc. (up to 2516 B) — keeps SaveBlock3 for DexNav. Depends on whether DexNav search levels are wanted in the QoL workstream.
- Graveyard mechanism: (a) capped in-save list (32 x 16 B, dedicated memorial UI, mon data gone); (b) devolov 'dead' bit on BoxPokemon (unlimited, full mon record kept, but corpses occupy PC boxes and UI is just the PC); (c) both — bit for enforcement, list for the memorial screen.
- Route status granularity: full 4-state caught/killed/fled/missed at 4 bits/route (45 B) vs TheXaman-style 1-bit encounter-used (9-12 B) — 4-state is what makes the tracker informative; 1-bit is enforcement-only.
- Region-map status overlay (color routes by encounter state): polish worth ~2-4 days of gfx work — include or cut?
- Data ownership: tracker should read the SAME per-route flags the nuzlocke-enforcement workstream writes (single source of truth) — confirm that workstream adopts the mapsec-indexed scheme so the two don't diverge.
