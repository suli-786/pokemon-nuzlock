## Workstream: encounter-toggles

**Overall effort:** S (flag assignment + key-item wrap) to M if the multipage options-menu port is chosen as the surface

### Already in the expansion (verified in repo)

- **Wild-encounter disable flag (runtime toggleable)**
  - Where: `WE_FLAG_NO_ENCOUNTER in include/config/wild_encounter.h:6 (default 0 = feature off). Consumed in ShouldDisableRandomEncounters() at src/field_control_avatar.c:895-898 and in the overworld-encounter spawner at src/wild_encounter_ow.c:242. Verified in this tree (expansion v1.16.3 per include/constants/expansion.h).`
  - How: Assign an unused flag ID, e.g. #define WE_FLAG_NO_ENCOUNTER FLAG_UNUSED_0x264 (381 FLAG_UNUSED_* IDs exist in include/constants/flags.h, e.g. lines 667-669). Then FlagSet/FlagClear/FlagToggle at runtime from any script or C code suppresses all step-triggered random encounters (grass/cave/surf) and OWE spawns.
- **Trainer sight-battle disable flag (runtime toggleable)**
  - Where: `OW_FLAG_NO_TRAINER_SEE in include/config/overworld.h:111 (default 0 = feature off). Consumed as the first check of CheckForTrainersWantingBattle() at src/trainer_see.c:444.`
  - How: Assign an unused flag ID in the config; when the flag is set, trainers never initiate sight battles but still battle when talked to (per the config comment and code path) — so gym trainers and any required trainer remain fightable via interaction.
- **Ready-made debug-menu toggles for both flags**
  - Where: `src/debug.c: DEBUG_FLAGVAR_MENU_ITEM_TOGGLE_ENCOUNTER (line 105, handler ~2566-2573), DEBUG_FLAGVAR_MENU_ITEM_TOGGLE_TRAINER_SEE (line 106, handler ~2579-2586); also TOGGLE_COLLISION and TOGGLE_CATCHING. Menu enabled via DEBUG_OVERWORLD_MENU in include/config/debug.h:5 (DISABLED_ON_RELEASE; R+START in overworld).`
  - How: Once the config flag IDs are nonzero, the entries become toggleable automatically (they show DEBUG_OPTION_CANT_BE_TOGGLED while the ID is 0, src/debug.c:1253-1259). Zero work for a dev-time toggle.
- **Deliberate encounter paths bypass the encounter-off flag (nuzlocke-friendly)**
  - Where: `FishingWildEncounter (src/wild_encounter.c:925) and SweetScentWildEncounter (src/wild_encounter.c:832) never check WE_FLAG_NO_ENCOUNTER; the flag only gates the step-trigger path in src/field_control_avatar.c and OWE spawning. Scripted/static battles (BattleSetup_StartScriptedWildBattle src/battle_setup.c:486, BattleSetup_StartLegendaryBattle :541, script trainerbattle commands) contain no references to either flag (verified by grep of src/battle_setup.c).`
  - How: Leave encounters off by default and take the route's nuzlocke first encounter deliberately via fishing/Sweet Scent, or briefly toggle off the flag. Static/legendary/scripted encounters and story battles always fire regardless of both toggles.
- **In-repo template for a registerable toggle key item**
  - Where: `ItemUseOutOfBattle_ExpShare at src/item_use.c:250-273 — FlagToggle(I_EXP_SHARE_FLAG) with on/off SE + message, works from bag or SELECT-registered.`
  - How: Clone this function pattern for two new key items (Encounter Charm / Stealth Charm style) that FlagToggle WE_FLAG_NO_ENCOUNTER and OW_FLAG_NO_TRAINER_SEE; register one to SELECT for instant toggling.
- **Adjacent nuzlocke-relevant wild flags (same file, same mechanism)**
  - Where: `include/config/wild_encounter.h: WE_FLAG_NO_CATCHING (line 16), WE_FLAG_NO_RUNNING (line 17), WE_FLAG_FORCE_DOUBLE_WILD (line 14), WE_SMART_WILD_AI_FLAG (line 15).`
  - How: Assign flag IDs and set/clear at runtime — NO_CATCHING can enforce 'only first encounter is catchable' by setting it after the route's first catch (hand-off to nuzlocke workstream).
- **Zero save-space cost for all of the above**
  - Where: `Flags live in the existing SaveBlock1 array: u8 flags[NUM_FLAG_BYTES] at include/global.h:1132; the toggles reuse already-allocated unused flag IDs.`
  - How: No SaveBlock layout change needed as long as toggles are flag-backed rather than new option-struct fields.

### Reuse candidates

- **Options Plus (tx_optionsPlus) — multipage scrolling options menu** — A scrolling multi-page options menu (also adds faster text, HP/EXP bar speed, metric/imperial) into which flag-backed 'Wild Encounters: On/Off' and 'Trainer Sight: On/Off' rows can be added cheaply; also serves the fleet's broader QoL/options goals.
  - Source: https://github.com/TheXaman/pokeemerald/tree/tx_optionsPlus (branch verified to exist); tutorial: https://github.com/pret/pokeemerald/wiki/New-Options-Plus-%E2%80%90-Multipage-Options-Menu-with-Faster-Text,-HP%E2%80%90EXP-Bar-Speeds-and-Metric
  - Port effort: M — based on vanilla pret/pokeemerald, not expansion 1.16.x, so expect conflicts in src/option_menu.c and minor SaveBlock2 changes; credits REQUIRED: TheXaman plus DizzyEgg, Lunos, AsparagusEduardo, ella_trifle (dark-theme variant additionally Archie, Mudskip).
- **Multipage Options Menu tutorial (devolov)** — Step-by-step diff for adding a second options page; its worked example is literally a toggle option (followers) — directly adaptable to encounter/trainer-sight toggles backed by FlagGet/FlagToggle instead of new SaveBlock fields.
  - Source: https://github.com/pret/pokeemerald/wiki/Multipage-Options-Menu
  - Port effort: S — hand-applied edits to expansion's src/option_menu.c; credit devolov/voloved (no formal requirement stated on the page).
- **pret wiki Tutorials directory (prior-art index)** — Verified index listing 'Toggling Trainers Seeing You' and 'Deactivate Wild Encounters With A Flag' — both now superseded by the expansion's native OW_FLAG_NO_TRAINER_SEE / WE_FLAG_NO_ENCOUNTER, confirming no port is needed for the core toggles; also lists 'Add Nuzlocke Challenge' (hand off to nuzlocke workstream).
  - Source: https://github.com/pret/pokeemerald/wiki/Tutorials
  - Port effort: S — reference only; nothing to port for this workstream's core.

### Must build

- Assign real flag IDs: two one-line edits (WE_FLAG_NO_ENCOUNTER in include/config/wild_encounter.h, OW_FLAG_NO_TRAINER_SEE in include/config/overworld.h) to FLAG_UNUSED_* values
- Player-facing wrap, cheapest path: two key items cloned from ItemUseOutOfBattle_ExpShare (new entries in src/data/items.h + two ~20-line item_use funcs + strings); options-menu path instead/additionally requires the menu port above
- Optional nuzlocke auto-mode: map-entry hook that clears WE_FLAG_NO_ENCOUNTER when the current route's first encounter is unused and re-sets it after the encounter resolves — depends on the nuzlocke workstream's per-route first-encounter tracker (small, but cross-stream)
- Optional: per-trainer exemption so specific required trainers keep sight-aggro while the global flag is set (no native support; would extend the check at src/trainer_see.c:444)

### Integration points

- include/config/wild_encounter.h (WE_FLAG_NO_ENCOUNTER, WE_FLAG_NO_CATCHING, WE_FLAG_NO_RUNNING assignment)
- include/config/overworld.h (OW_FLAG_NO_TRAINER_SEE assignment)
- src/field_control_avatar.c — ShouldDisableRandomEncounters()/CheckStandardWildEncounter()
- src/trainer_see.c — CheckForTrainersWantingBattle()
- src/wild_encounter_ow.c — UpdateOverworldWildEncounter() (if OWE mode enabled)
- src/debug.c — existing flag/var toggle menu (dev-time control)
- src/item_use.c + src/data/items.h (key-item wrap) or src/option_menu.c (options-menu wrap)
- Nuzlocke workstream: first-encounter tracker + WE_FLAG_NO_CATCHING enforcement hook
- include/constants/flags.h — consume 2-4 FLAG_UNUSED_* IDs (coordinate ID allocation across the 12 workstreams to avoid collisions)

### Risks

- OW_FLAG_NO_TRAINER_SEE is global: gym/required trainers also stop sight-aggroing (still fightable by talking); any event that assumes a sight battle fires could soft-skip content — no per-map/per-trainer exemption exists natively
- Encounters-off blocks the nuzlocke first encounter too; a player who forgets to toggle per route loses time or takes the encounter via fishing/Sweet Scent instead — decide the auto-toggle policy up front
- Encounter-off is deliberately not airtight (fishing, Sweet Scent, scripted/static encounters bypass) — correct for nuzlocke but must be documented so it doesn't read as a bug
- Options Plus branch is vanilla-pokeemerald-based; merging into expansion 1.16.3's option_menu.c will conflict and its SaveBlock2 edits need review against expansion's layout; credit obligations apply
- Storing new options as SaveBlock2 fields breaks old saves if mid-run; flag-backed toggles avoid all save-layout risk — prefer them
- DEBUG_OVERWORLD_MENU is DISABLED_ON_RELEASE (include/config/debug.h:5); relying on it as the player-facing toggle means shipping with the full debug/cheat menu enabled
- Flag ID collisions if multiple workstreams grab FLAG_UNUSED_* values independently

### Open decisions (owner's call)

- Toggle surface: (a) two SELECT-registerable key items cloned from the Exp Share pattern (fastest, S), (b) options-menu rows via devolov tutorial or Options Plus port (cleaner UX, S-M), (c) keep debug menu enabled in release builds (free but exposes cheats), or (d) key items now + options menu later
- Nuzlocke interaction policy: (a) manual — player toggles encounters per new route, (b) auto — encounters force-enabled on maps whose first encounter is unused, auto-disabled after (needs cross-stream hook), or (c) encounters permanently off and first encounters taken only via fishing/Sweet Scent-style deliberate methods
- Trainer-sight scope: accept global suppression with talk-to-battle fallback, or invest in per-trainer exemptions so gym-route/required trainers keep sight-aggro
- Whether to also wire WE_FLAG_NO_CATCHING (auto-set after a route's first catch) as part of nuzlocke enforcement or leave catching unrestricted
