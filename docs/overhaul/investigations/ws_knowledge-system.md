## Workstream: knowledge-system

**Overall effort:** M — the majority of the wishlist is config flips on expansion 1.16.3 (B_SHOW_EFFECTIVENESS already on, B_SHOW_TYPES/POKEDEX_PLUS_HGSS/IV-EV one-liners); genuinely new work is the S-M weakness panel and the M recommendation page, both buildable from existing pure-data primitives with no SaveBlock cost

### Already in the expansion (verified in repo)

- **In-battle type-effectiveness indicators on move-select UI (Gen7+ style; replaces PP string)**
  - Where: `B_SHOW_EFFECTIVENESS in include/config/battle.h:402-406 (SHOW_EFFECTIVENESS_NEVER/ALWAYS/CAUGHT/SEEN); impl in src/battle_controller_player.c (enum EFFECTIVENESS_* ~line 2380); added in v1.12.0 via PR #6559`
  - How: Already ON by default at SHOW_EFFECTIVENESS_SEEN; change to SHOW_EFFECTIVENESS_ALWAYS if seen-gating is unwanted in a randomizer
- **Type icons next to enemy healthbars while choosing a move (shows the target's types)**
  - Where: `B_SHOW_TYPES in include/config/battle.h:396-400, default SHOW_TYPES_NEVER; impl in src/type_icons.c; added v1.10.0 PR #5131 by pkmnsnfrn`
  - How: Set B_SHOW_TYPES to SHOW_TYPES_SEEN or SHOW_TYPES_ALWAYS — one-line config flip
- **HGSS-style Pokedex Plus: base stats, EV yields, abilities incl. hidden, catch rate, exp yield, egg cycles/groups, growth rate, level-up/egg/TM/tutor moves, evolutions screen, forms screen, area, cry, size**
  - Where: `POKEDEX_PLUS_HGSS in include/config/pokedex_plus_hgss.h:4 (default FALSE); impl src/pokedex_plus_hgss.c (8814 lines; Task_LoadEvolutionScreen, Task_LoadFormsScreen, stats toggle)`
  - How: Set POKEDEX_PLUS_HGSS TRUE; sub-options: HGSS_DARK_MODE, HGSS_DECAPPED, HGSS_HIDE_UNSEEN_EVOLUTION_NAMES, HGSS_HIDE_UNOWNED_EVOLUTION_METHODS, HGSS_SORT_TMS_BY_NUM, HGSS_SHOW_EGG_MOVES_FOR_EVOS. NOTE: it does NOT show weaknesses/type matchups
- **IV/EV display on summary screen (cycle Stats/IVs/EVs; letter grades or raw values; box-only variant; flag-gated variant)**
  - Where: `P_SUMMARY_SCREEN_IV_EV_INFO, P_SUMMARY_SCREEN_IV_EV_VALUES, P_SUMMARY_SCREEN_IV_EV_BOX_ONLY, P_FLAG_SUMMARY_SCREEN_IV_EV_INFO in include/config/summary_screen.h:9-30`
  - How: Set P_SUMMARY_SCREEN_IV_EV_INFO TRUE (plus P_SUMMARY_SCREEN_IV_EV_VALUES TRUE for raw numbers); may need make clean if using P_SUMMARY_SCREEN_IV_EV_TILESET
- **Move relearner accessible from summary screen moves page (plus egg/TM/tutor relearner variants)**
  - Where: `P_SUMMARY_SCREEN_MOVE_RELEARNER (TRUE by default) and P_ENABLE_MOVE_RELEARNERS, P_TM_MOVES_RELEARNER, P_FLAG_EGG_MOVES, P_FLAG_TUTOR_MOVES in include/config/summary_screen.h:33-52`
  - How: Summary-screen relearner already on; set P_ENABLE_MOVE_RELEARNERS TRUE for egg/TM/tutor relearning
- **In-battle move description window (power/accuracy/effect while selecting a move)**
  - Where: `B_SHOW_MOVE_DESCRIPTION (TRUE) at include/config/battle.h:294; B_MOVE_DESCRIPTION_BUTTON = L_BUTTON at battle.h:331`
  - How: Already on; press L on a move in battle
- **Physical/Special/Status category icons in summary screen and move relearner**
  - Where: `B_SHOW_CATEGORY_ICON (TRUE) at include/config/battle.h:326`
  - How: Already on by default
- **Nature-based stat coloring on summary screen (red/blue for boosted/reduced stats)**
  - Where: `P_SUMMARY_SCREEN_NATURE_COLORS (TRUE) at include/config/summary_screen.h:5`
  - How: Already on by default
- **DexNav wild-mon knowledge (preview species, ability, moves, level of encounters before fighting)**
  - Where: `DEXNAV_ENABLED at include/config/dexnav.h:4 (default FALSE) plus DN_FLAG_DEXNAV_GET / DN_VAR_SPECIES which must be assigned to real flag/var IDs`
  - How: Set DEXNAV_ENABLED TRUE and assign the DN_ flags/vars to unused flag/var slots; no new SaveBlock fields

### Reuse candidates

- **DP/Pt-style summary screen port (Jaizu)** — Complete modern (Gen4-style) summary screen UI replacement — the strongest existing 'modern summary screen' with public source for pokeemerald
  - Source: https://github.com/citrusbolt/pokeemerald (branch: summary_screen, verified to exist, last updated Oct 2023); PokeCommunity thread: https://www.pokecommunity.com/threads/pok%C3%A9mon-dp-pt-summary-screen.469950/
  - Port effort: L — built on vanilla pret/pokeemerald (2023), while expansion 1.16.3 has heavily rewritten src/pokemon_summary_screen.c (IV/EV pages, relearner, Tera types, dynamic move types); requires manual re-implementation on top, not a clean merge. Credit Jaizu/citrusbolt required; PC/pret community norm is credit-in-README, but no explicit license — ask permission
- **pret wiki: Show Type Effectiveness In Battle (options-menu toggle pattern)** — Superseded by B_SHOW_EFFECTIVENESS for the indicator itself, but shows the pattern for exposing it as an in-game Options menu toggle
  - Source: https://github.com/pret/pokeemerald/wiki/Show-Type-Effectiveness-In-Battle-Using-Pre-Existing--Function-and-Disable-in-Option-Menu (listed on https://github.com/pret/pokeemerald/wiki/Tutorials)
  - Port effort: S — only the options-menu wiring is needed; wiki tutorials are free to use with credit
- **pret wiki: Show IVs/EVs in Summary Screen** — Reference implementation for summary-screen stat-page cycling — NOT needed (expansion already ships P_SUMMARY_SCREEN_IV_EV_INFO); listed only to prevent redundant porting
  - Source: https://github.com/pret/pokeemerald/wiki/Show-IVs-EVs-in-Summary-Screen
  - Port effort: S — skip; use the built-in config instead
- **Modern Emerald (resetes12)** — Design prior art for 'game documentation in-game': Pokedex Stats page with all species data, IV/EV via L/R in stats view, START-during-move-select info submenu; public source on expansion base
  - Source: https://github.com/resetes12/pokeemerald
  - Port effort: M — expansion 1.16 equivalents already cover most of it (HGSS dex, IV/EV configs, move description window); cherry-pick only the move-select submenu idea if wanted. Credits-based project on pret base — credit resetes12 and upstream sources it lists
- **Emerald Rogue (Pokabbie)** — Reference for randomizer-friendly in-game knowledge UX (quick species info in a run-based randomized context); public source
  - Source: https://github.com/Pokabbie/pokeemerald-rogue
  - Port effort: L — Rogue's codebase diverges massively from stock expansion; treat as design reference only. Credit Pokabbie if any code is lifted

### Must build

- (a) Defensive weaknesses/resistances panel (per-mon, dual-type-combined, ability-aware) on the summary screen and/or HGSS dex stats page — no public expansion-1.16 port found; build from gTypeEffectivenessTable (include/battle_main.h:126), GetTypeModifier (src/battle_util.c:8322, pure table lookup), GetOverworldTypeEffectiveness (src/battle_util.c:8293, takes struct Pokemon* out of battle and handles abilities like Levitate), and existing type-icon graphics from src/type_icons.c / HGSS dex — S-M effort, zero SaveBlock impact
- (c) Recommendation page ('best move types vs this mon', 'role this mon suits') — no prior art anywhere; recommend building a lightweight pure-data scorer (gSpeciesInfo base-stat spread for role heuristics: physical/special attacker, wall, speedster; type-chart scan for best attacking types) rather than reusing battle AI scoring, because src/battle_ai_main.c / battle_ai_util.c APIs (e.g. AI_CalcDamage, include/battle_ai_util.h:173) are hard-bound to live battle state (enum BattlerId, gBattleMons, AiLogicData) and would need a faked battle context outside battle — M effort
- In-game Options-menu toggles for B_SHOW_EFFECTIVENESS / B_SHOW_TYPES gating (currently compile-time only) — S effort, optional

### Integration points

- /home/suleiman/pokeemerald-expansion/include/config/battle.h (B_SHOW_EFFECTIVENESS:406, B_SHOW_TYPES:400, B_SHOW_MOVE_DESCRIPTION:294, B_SHOW_CATEGORY_ICON:326)
- /home/suleiman/pokeemerald-expansion/include/config/pokedex_plus_hgss.h
- /home/suleiman/pokeemerald-expansion/include/config/summary_screen.h
- /home/suleiman/pokeemerald-expansion/include/config/dexnav.h
- /home/suleiman/pokeemerald-expansion/src/pokemon_summary_screen.c (weakness panel + any modern-UI reskin land here)
- /home/suleiman/pokeemerald-expansion/src/pokedex_plus_hgss.c (dex-side weakness/recommendation page)
- /home/suleiman/pokeemerald-expansion/src/type_icons.c (reusable type icon sprites)
- /home/suleiman/pokeemerald-expansion/src/battle_controller_player.c (move-select effectiveness UI)
- /home/suleiman/pokeemerald-expansion/src/battle_util.c (GetOverworldTypeEffectiveness:8293, GetTypeModifier:8322) + include/battle_main.h (gTypeEffectivenessTable:126)
- /home/suleiman/pokeemerald-expansion/include/battle_ai_util.h + src/battle_ai_util.c (only if in-battle AI-backed move suggestions are chosen)
- /home/suleiman/pokeemerald-expansion/include/pokemon.h (GetLevelUpMovesBySpecies:880, CanLearnTeachableMove:879 for recommendation data)

### Risks

- Workstream collision: a DP/Pt- or BW-style summary reskin and the weakness panel both rewrite src/pokemon_summary_screen.c — sequence them (reskin first) or the panel gets built twice
- src/pokedex_plus_hgss.c is a single 8814-line file actively touched upstream; deep customization creates merge pain on future expansion updates
- SEEN/CAUGHT gating (B_SHOW_EFFECTIVENESS default = SEEN) interacts badly with randomized nuzlockes early-game — indicators hidden for unseen species exactly when the player most needs them; ALWAYS leaks nothing in a randomizer since types may be species-standard anyway
- B_SHOW_EFFECTIVENESS replaces the PP string on move select (upstream PR #6559 design tradeoff) — PP becomes invisible while the indicator shows
- If the randomizer workstream randomizes types/evolutions, static dex data (evolutions screen, type icons) shows vanilla-wrong info unless the dex reads randomized runtime data — cross-workstream dependency
- Community-asset licensing: pret-derived code has no formal license; convention is credit + ask permission (Jaizu/citrusbolt, resetes12, Pokabbie); PokeCommunity blocks automated scraping (HTTP 403), so permission requests must be manual
- Shiny-Miner/New-BW-summary-screen is a FireRed binary-hack asset, not decomp code — not portable despite appearing in searches
- No SaveBlock impact for anything in this workstream (effectiveness gating reuses existing dex seen/caught flags; DexNav uses existing unused flag/var slots) — low risk, but DexNav flag/var IDs must not collide with other workstreams claiming unused flags

### Open decisions (owner's call)

- Where the weakness/resistance display lives: (1) new summary-screen page, (2) HGSS dex stats sub-page, (3) both — (2) is cheapest and merge-safest, (1) is most convenient mid-run
- Effectiveness indicator gating for a randomized nuzlocke: SHOW_EFFECTIVENESS_ALWAYS (max QoL, no dex grinding) vs keep SEEN (default, preserves discovery) — same choice separately for B_SHOW_TYPES
- Recommendation engine depth: (1) static data-driven page from base stats + type chart (M, out-of-battle, works in dex/summary), (2) in-battle AI-backed 'suggested move' highlight reusing AI_CalcDamage (M-L, battle-only, exact but risks trivializing fights), (3) both
- Modern summary UI: port Jaizu's DP/Pt summary (L effort, needs permission/credit) vs keep expansion summary + add weakness panel (S-M, no external deps) — interacts with the UI/visuals workstream's choice
- HGSS dex spoiler policy for a randomizer: enable HGSS_HIDE_UNSEEN_EVOLUTION_NAMES / HGSS_HIDE_UNOWNED_EVOLUTION_METHODS or show everything (max knowledge)
- Whether battle-info toggles belong in the in-game Options menu (extra S build) or stay compile-time
