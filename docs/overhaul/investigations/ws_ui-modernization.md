## Workstream: ui-modernization

**Overall effort:** L

### Already in the expansion (verified in repo)

- **HGSS-style Pokedex Plus (TheXaman) incl. dark mode, decapped gfx, evo/egg-move/TM info pages (big 'in-game knowledge' win)**
  - Where: `POKEDEX_PLUS_HGSS, HGSS_DARK_MODE, HGSS_DECAPPED, HGSS_SHOW_EGG_MOVES_FOR_EVOS in include/config/pokedex_plus_hgss.h; code src/pokedex_plus_hgss.c`
  - How: Set POKEDEX_PLUS_HGSS TRUE (plus desired HGSS_* toggles) and rebuild
- **Summary screen extras: IV/EV pages, nature-colored stats, rename from summary, move relearner on moves page**
  - Where: `P_SUMMARY_SCREEN_IV_EV_INFO, P_SUMMARY_SCREEN_NATURE_COLORS, P_SUMMARY_SCREEN_RENAME, P_SUMMARY_SCREEN_MOVE_RELEARNER, P_ENABLE_MOVE_RELEARNERS in include/config/summary_screen.h`
  - How: Flip the P_SUMMARY_SCREEN_* defines to TRUE; optional flag-gated variant via P_FLAG_SUMMARY_SCREEN_IV_EV_INFO
- **Battle UI QoL: move description window, category icons, type icons next to healthbars, type-effectiveness indicator on moves (seen-based), show targets, last-used-ball with cycling, faster battle text**
  - Where: `B_SHOW_MOVE_DESCRIPTION, B_SHOW_CATEGORY_ICON, B_SHOW_TYPES, B_SHOW_EFFECTIVENESS (SHOW_EFFECTIVENESS_SEEN default), B_SHOW_TARGETS, B_LAST_USED_BALL/B_LAST_USED_BALL_CYCLE, B_WAIT_TIME_MULTIPLIER in include/config/battle.h; rendering in src/battle_interface.c`
  - How: Config toggles only; e.g. set B_SHOW_TYPES to SHOW_TYPES_ALWAYS and lower B_WAIT_TIME_MULTIPLIER for faster battles
- **Instant/auto-scrolling text**
  - Where: `TEXT_SPEED_INSTANT, TEXT_SPEED_*_MODIFIER, AUTO_SCROLL_TEXT, FLAG_TEXT_SPEED_INSTANT in include/config/text.h`
  - How: Set TEXT_SPEED_INSTANT TRUE or raise modifiers (18-20 is near-instant per file comment)
- **BW2-style overworld map name popups (black/white theme, optional clock)**
  - Where: `OW_POPUP_GENERATION, OW_POPUP_BW_COLOR, OW_POPUP_BW_TIME_MODE, OW_POPUP_BW_ALPHA_BLEND, OW_HIDE_REPEAT_MAP_POPUP in include/config/overworld.h`
  - How: Set OW_POPUP_GENERATION to GEN_5 and pick color/time options
- **FRLG-style map preview screens on map entry**
  - Where: `MPS_ENABLE_MAP_PREVIEWS and duration configs in include/config/map_preview_screen.h; tutorial docs/tutorials/how_to_map_preview_screen.md`
  - How: Set MPS_ENABLE_MAP_PREVIEWS TRUE and register previews per tutorial
- **Speaker name box over dialogue (dynamic width, auto NPC-trainer names)**
  - Where: `OW_NAME_BOX_* in include/config/name_box.h; src/field_name_box.c; docs/tutorials/how_to_namebox.md`
  - How: Enable OW_NAME_BOX_NPC_TRAINER and use namebox macros in scripts
- **Skippable RHH splash intro (declutter boot)**
  - Where: `EXPANSION_INTRO in include/config/general.h line 76; src/expansion_intro.c`
  - How: Set EXPANSION_INTRO FALSE to remove the extra splash
- **Quickstart: SELECT on title starts a new game instantly (debug builds only)**
  - Where: `ENABLE_QUICKSTART, QUICKSTART_HUD in include/config/quickstart.h`
  - How: On by default in debug builds; disabled on release builds
- **Follower Pokemon (HGSS style) and NPC followers (visual modernization)**
  - Where: `OW_FOLLOWERS_ENABLED etc. in include/config/overworld.h; include/config/follower_npc.h; docs/tutorials/how_to_follower_npc.md`
  - How: Set OW_FOLLOWERS_ENABLED TRUE (needs OW_POKEMON_OBJECT_EVENTS)
- **Ability pop-ups, move info window, last-ball window sprites already implemented in battle interface**
  - Where: `src/battle_interface.c (SpriteCb_AbilityPopUp, SpriteCB_LastUsedBall, SpriteCB_MoveInfoWin)`
  - How: Already active; controlled by the battle.h configs above
- **FRLG build target (whole alternate UI/tileset baseline) merged into this master**
  - Where: `docs/tutorials/how_to_frlg.md; MPS_ENABLE_MAP_PREVIEWS defaults to IS_FRLG`
  - How: make firered / make leafgreen (not recommended for this project, but shows FRLG assets are in-tree)

### Reuse candidates

- **RHH official decapitalization branch (upcoming-decap)** — Full mixed-case text across the game - the single biggest 'modern feel' change; replaces the decap that was reverted in 1.8.4 (docs/changelogs/1.8.x/1.8.4.md)
  - Source: https://github.com/rh-hideout/pokeemerald-expansion (branch upcoming-decap, verified via git ls-remote; expansion.h reads 1.17.0-dev)
  - Port effort: M - same repo but tracks 'upcoming' (1.17-dev), so merging into 1.16.3 master pulls extra churn; alternative is waiting for the 1.17 release. Credit: RHH (already required by base).
- **SwSh UI suite by Montblanc (summary, party, bag, PC storage, message box, map popups, window frames, party-PC access)** — The only coherent modern-UI suite covering nearly every screen the owner listed, in one consistent Gen-8 style
  - Source: https://github.com/montmoguri/pokeemerald-expansion (branches swsh_summary_screen, swsh_party_menu, swsh_bag_menu, swsh_storage_system, swsh_message_box, swsh_map_popups, dev_window_frames, party_menu_pc_access - all verified via ls-remote; screenshots on https://github.com/Pawkkie/Team-Aquas-Asset-Repo/wiki/Feature-Branches)
  - Port effort: S per module / M for the whole suite - swsh_summary_screen and swsh_party_menu verified based on expansion 1.16.2 (matches this tree); message box requires >=1.14. No formal license; community norm: credit Montblanc.
- **Gen 5 Black/White battle UI by pollythadon** — Full BW-inspired battle interface replacement (action/move selection, healthboxes) current with this tree
  - Source: https://github.com/pollythadon/pokeemerald-expansion/tree/gen5bwUI (branch verified; expansion.h on branch reads 1.16.2)
  - Port effort: S - base 1.16.2 matches master; explicit credit requirement: pollythadon plus original creators EternalCode, PlatinumMaster, NicoSwag, mudskipper13 (stated on Team Aqua wiki entry).
- **BW summary screen (expansion version) by ravepossum** — Gen-5 summary screen demake with IV/EV, relearner, tera icons; additive (new file) so low collision with vanilla summary code
  - Source: https://github.com/ravepossum/pokeemerald-expansion/tree/bw_summary_screen_expansion (verified; GitHub compare vs expansion/1.14.0: 44 commits, adds src/bw_summary_screen.c +5538 lines and graphics/summary_screen/bw/, with small hooks in party_menu.c, pokemon_storage_system.c, trade.c, evolution_scene.c, battle_factory_screen.c)
  - Port effort: M - base is expansion 1.14.0, two minors behind; needs rebase over 1.15/1.16 changes incl. the INCGFX graphics-include migration (1.16.0 PR #10036). Long credit list (Againsts, Sphericalice, Buffelsaft, DizzyEgg, Zeturic, Skeli, Zatsu, Lhea, Black Fragrant, Montblanc et al.) documented in the branch wiki: https://github.com/ravepossum/pokeemerald-expansion/wiki
- **HGSS battle UI (expansion-only) by ravepossum** — HGSS-style healthboxes/battle UI; singles, doubles, safari and all expansion gimmicks supported (per author)
  - Source: https://github.com/ravepossum/pokeemerald-expansion/tree/hgss_battle_ui_expansion (verified; compare vs 1.14.0: 8 commits, ~36 graphics files plus small diffs to src/battle_interface.c and src/battle_gimmick.c)
  - Port effort: S-M - mostly a graphics swap so rebasing to 1.16.3 is mechanical, but base is 1.14.0 (INCGFX conversion needed). Credit ravepossum.
- **Unbound-style start menu by miriamlefae** — Sprite-based horizontal start menu a la Pokemon Unbound; DNS/dark-area aware
  - Source: https://github.com/miriamlefae/pokeemerald-expansion (branches feat/usm/1.16.1 and feat/usm/upcoming, verified via ls-remote)
  - Port effort: S - maintained against 1.16.1 and upcoming, near-current with this tree. Credit miriamlefae.
- **Full Screen Start Menu by Archie and Mudskip** — Full-screen start menu with player info, time, party HP; self-contained src/ui_startmenu_full.c + graphics/ui_startmenu_full/
  - Source: https://github.com/pret/pokeemerald/wiki/Full-Screen-Start-Menu-by-Archie-and-Mudskip (verified; pull branch full_start_menu from https://github.com/TeamAquasHideout/pokeemerald, verified via ls-remote)
  - Port effort: M - written on vanilla pokeemerald, marked 'Expansion Compatible' with a one-line fix when HGSS dex is used; manual port to 1.16.3 required. Credits: Archie (code), Mudskip (gfx), ghoulslash (UI shell), Rioluwott (HP bar).
- **Options Plus with dark theme (TheXaman)** — Multi-page modern options menu incl. UI theme/dark mode - pairs with HGSS_DARK_MODE dex
  - Source: https://github.com/TheXaman/pokeemerald/tree/tx_optionsPlus (verified via ls-remote; expansion-adjacent copy: branch thexaman_options_plus_dark_theme on https://github.com/TeamAquasHideout/pokeemerald)
  - Port effort: M - vanilla pokeemerald base, needs manual port to expansion 1.16.3 (src/option_menu.c is still vanilla in this tree, so collision is low). Credit TheXaman.
- **Multipage Options Menu tutorial (devolov)** — Cheap L/R-paged options menu pattern - the natural home for nuzlocke/QoL toggles from other workstreams
  - Source: https://github.com/pret/pokeemerald/wiki/Multipage-Options-Menu (verified)
  - Port effort: S - hand-apply; touches only src/option_menu.c + strings; new options need SaveBlock2 bitfields (tiny save impact).
- **Registered items menu, expansion-updated (TheXaman via iriv24)** — Multiple SELECT-registered key items via a popup menu - big time-saver QoL
  - Source: https://github.com/iriv24/pokeemerald-expansion (branches original_tx_registered_items_menu and iriv24/tx_registered_items_menu, verified via ls-remote)
  - Port effort: S-M - expansion-based but branch base version unverified; small feature surface. Credit TheXaman + iriv24.
- **Sample UI scaffold (grunt-lucas)** — Clean template + tutorial for building any brand-new screen (e.g. nuzlocke rules/graveyard screen from the other workstreams)
  - Source: https://github.com/grunt-lucas/pokeemerald-expansion (branches sample-ui / tutorial/sample-ui, verified via ls-remote)
  - Port effort: S - reference/template, copy patterns rather than merge. Credit grunt-lucas.
- **Nico's Cool UI (BW-inspired battle UI)** — Alternative BW-flavored battle UI with type icons, singles+doubles
  - Source: https://github.com/NicoSwag/pokeemerald-expansion/tree/nicos_cool_ui (verified; expansion.h on branch reads 1.11.2)
  - Port effort: L - base 1.11.2 (five minors behind) and battle gimmicks explicitly unsupported; pollythadon's gen5bwUI supersedes it for this project. Credit NicoSwag/Archie.
- **Emerald Rogue (Pokabbie) as reference implementation** — Battle-tested single-player QoL/UI ideas (run summaries, custom menus) built on expansion - reference more than direct merge
  - Source: https://github.com/Pokabbie/pokeemerald-rogue (verified via ls-remote; branch 'expansion' exists)
  - Port effort: L - full-game fork, cherry-picking UI code out is invasive; use as design reference. Credit Pokabbie if code is lifted.

### Must build

- Font restyle: no verified drop-in community port for expansion - edit graphics/fonts/latin_normal.png (+ short/narrow variants) plus width tables in src/fonts.c and gFonts in src/text.c (small, asset-driven)
- Decluttering pass: removing/hiding vanilla screens irrelevant to a randomized nuzlocke (contest pages in summary/pokenav, ribbons, mystery gift menu entries) - hack-specific edits, no community branch
- Visual consistency pass if mixing sources (e.g. SwSh bag + BW summary + HGSS battle UI): unify window frames (graphics/text_window/, src/text_window.c), palettes and fonts
- INCGFX conversion for any pre-1.16 branch being ported (graphics include system changed in 1.16.0, PR #10036)
- Nuzlocke-specific options page entries (wired into whichever options-menu redesign is chosen)

### Integration points

- src/pokemon_summary_screen.c (4886 lines) + include/config/summary_screen.h + graphics/summary_screen/ - summary screen; callers that need hooks for any replacement: src/party_menu.c, src/pokemon_storage_system.c, src/trade.c, src/evolution_scene.c, src/battle_factory_screen.c, src/move_relearner.c
- src/battle_interface.c (3177 lines) + graphics/battle_interface/ - healthboxes, ability pop-up, last-ball, move-info window, gimmick indicators; any battle reskin lands here
- src/battle_bg.c (window templates/backgrounds), src/battle_controllers.c + battle controller player code (action/move selection), src/battle_message.c (battle text) - rest of battle UI blast radius
- src/start_menu.c (1520 lines) - start menu; src/main_menu.c - title/new-game menu; src/option_menu.c (684 lines, still vanilla 6-item menu = low-collision replacement target)
- src/item_menu.c (3030 lines) + src/item_menu_icons.c + graphics/bag/ - bag; src/party_menu.c (8620 lines) - party
- src/text.c, src/fonts.c, graphics/fonts/ - font rendering; src/text_window.c + graphics/text_window/ - message box frames; include/config/text.h - speed
- src/intro.c, src/expansion_intro.c (EXPANSION_INTRO in include/config/general.h) - boot sequence declutter
- src/pokedex_plus_hgss.c + include/config/pokedex_plus_hgss.h - the in-game-knowledge dex the rest of the UI should match
- graphics_file_rules.mk + INCGFX macros - graphics build system all ports must conform to; include/config/*.h - toggle surface

### Risks

- Version skew is the main cost driver: ravepossum branches are 1.14.0-based, Nico 1.11.2, TheXaman/Archie vanilla-based; this tree is 1.16.3-dev with the INCGFX graphics migration (1.16.0) - expect mechanical but broad conflicts. 1.16.2-based candidates (Montblanc suite, pollythadon) merge nearly clean
- Replacement summary screens duplicate features this tree already has via config (relearner, rename, IV/EV) - must verify parity or lose in-tree features when swapping screens
- Battle-UI reskins must support expansion gimmick triggers (tera/dynamax/z-move indicators in battle_interface.c); Nico's UI explicitly does not - verify pollythadon/ravepossum against the gimmicks actually used
- Stacking multiple community branches multiplies conflicts because several touch the same callers (party_menu.c, pokemon_summary_screen.c hooks); order the merges and budget re-test time
- upcoming-decap tracks the unreleased 1.17 line - merging now drags unreleased engine changes into a 1.16.3 base; waiting for the 1.17 tag is safer
- No formal licenses anywhere in this ecosystem (pret-derived code has no explicit license); everything runs on credit norms - keep a CREDITS entry per pulled branch, and pollythadon/BSBob/RHH state explicit credit requests. Nothing here appears permission-gated, but confirm nothing was scraped from closed hacks (pollythadon ports EternalCode/PlatinumMaster work)
- 'Expansion compatible' labels on vanilla-based branches (Archie start menu, tx_optionsPlus) come from wiki claims, not CI - budget for real porting
- SaveBlock impact: pure reskins are zero; anything adding options (multipage options, theme switcher) needs new SaveBlock2 bits - trivial individually but coordinate with the nuzlocke workstream which also wants save space
- Screenshots for most candidates live on the Team Aqua Feature-Branches wiki page (github.com/Pawkkie/Team-Aquas-Asset-Repo/wiki/Feature-Branches) as user-attachments links - fine today, but archive copies locally since wiki pages get edited

### Open decisions (owner's call)

- Battle UI direction: (a) keep expansion default and just enable config QoL (zero risk), (b) pollythadon BW UI (1.16.2, near-drop-in), (c) ravepossum HGSS UI (1.14, gimmick-complete, rebase needed), (d) Nico's UI (oldest, no gimmicks - not recommended)
- Summary screen: (a) keep in-tree screen + enable IV/EV/relearner configs, (b) Montblanc SwSh demake (1.16.2), (c) ravepossum BW demake (1.14, additive file)
- Suite coherence: adopt Montblanc's SwSh set wholesale for one consistent style across summary/party/bag/PC/messagebox, vs mix-and-match best-of-breed per screen (needs the consistency pass)
- Decapitalization: merge rh-hideout upcoming-decap now, wait for expansion 1.17, backport prof-harpe's 1.12.2 branch (https://github.com/prof-harpe/pokeemerald-expansion/tree/Decapitalized), or stay all-caps
- Start menu: miriamlefae Unbound-style (1.16.1, easiest), Archie/Mudskip full-screen (vanilla port, flashier), or vanilla
- Options menu: hand-applied multipage tutorial (cheapest, owns the nuzlocke toggles) vs full tx_optionsPlus port with dark theme
- HGSS Pokedex Plus: enable now with dark mode on/off - this anchors the game's 'in-game knowledge' aesthetic, so decide it first and match other screens to it
- Font: keep vanilla Emerald font vs commission/draw a custom glyph sheet (no verified ready-made port found)
