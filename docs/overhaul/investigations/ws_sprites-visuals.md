## Workstream: sprites-visuals

**Overall effort:** M — the headline items (DS-style mon sprites all gens, follower sprites, full move-animation coverage, modern item icons, battle shadows, DNS) are already merged and are config flips; remaining real work is asset-pack insertion (trainers/NPCs/backgrounds) and optional per-map lighting

### Already in the expansion (verified in repo)

- **DS-style (Gen 4/5) 64x64 front/back sprites, icons, footprints for ALL Gens 1-9 — default since expansion 1.10.0 (commit 358c0d0699)**
  - Where: `P_GBA_STYLE_SPECIES_GFX / P_GBA_STYLE_SPECIES_ICONS / P_GBA_STYLE_SPECIES_FOOTPRINTS, all FALSE(=DS style) in include/config/pokemon.h lines 48-50; assets in graphics/pokemon/<species>/ referenced by src/data/graphics/pokemon.h`
  - How: Nothing to do — modern sprites are already the default. Flip the three configs to TRUE only if GBA style is wanted (note: doing so disables enemy battle shadows per the config comment)
- **Two-frame Emerald-style front animations (947 of ~1390 sprite sets animated; 443 Gen 8/9-era mons+forms are single-frame static)**
  - Where: `P_TWO_FRAME_FRONT_SPRITES TRUE in include/config/pokemon.h line 47; animated = graphics/pokemon/*/anim_front.png, static = front.png (verified counts via find)`
  - How: Already on. Static Gen 8/9 mons silently use one frame; second frames are spriting work upstream tracks in rh-hideout issue #5883
- **Follower Pokemon (HGSS style) with OW sprites shipped for 1299 mons/forms incl. megas**
  - Where: `OW_FOLLOWERS_ENABLED FALSE in include/config/overworld.h line 61; sprites graphics/pokemon/*/overworld.png + overworld_normal/shiny.pal; support: OW_POKEMON_OBJECT_EVENTS TRUE, OW_FOLLOWERS_BOBBING TRUE, OW_FOLLOWERS_POKEBALLS TRUE, OW_BATTLE_ONLY_FORMS TRUE, OW_LARGE_OW_SUPPORT TRUE, OW_GFX_COMPRESS TRUE (same file)`
  - How: Set OW_FOLLOWERS_ENABLED TRUE and rebuild. Config comment warns 'additional scripting may be required for them to be fully supported' (e.g. hiding followers in cutscenes)
- **Gen 4+ enemy mon battle shadows**
  - Where: `B_ENEMY_MON_SHADOW_STYLE GEN_LATEST in include/config/battle.h line 409`
  - How: Already on by default
- **Battle animations for every move through Gen 9 (990 gBattleAnimMove_ scripts, incl. Tera Blast, Rage Fist, Ivy Cudgel, Malignant Chain; only Shell Side Arm + Gulp Missile are marked placeholders)**
  - Where: `data/battle_anim_scripts.s (990 labels, verified by grep); wired per-move via .battleAnimScript in src/data/moves_info.h / include/move.h`
  - How: Nothing to do — this community effort (ghoulslash's 'add animations for all moves' + CFRU ports) is fully merged
- **Modern item icons for all expansion items (619 icon PNGs incl. Gen 4-9 items)**
  - Where: `graphics/items/icons/ (619 files) + graphics/items/icon_palettes/, referenced from src/data/graphics/items.h`
  - How: Nothing to do — shipped by default
- **Day/night overworld palette tint (DNS) with per-color light-blending support (.pla files) and lamp-glow object events**
  - Where: `OW_ENABLE_DNS TRUE in include/config/overworld.h line 102; OW_SHADOW_INTENSITY, OW_OBJECT_VANILLA_SHADOWS FALSE (line 105, warns each object costs 2 sprites); full tutorial docs/tutorials/dns.md`
  - How: Tint already on. Per-map lighting (glowing windows/lamps) is NOT set up on vanilla maps — dns.md says revert upstream commit a5b079d833 (PR #6562) and place OBJ_EVENT_GFX_LIGHT_SPRITE objects
- **B2W2-style map name popups (with optional time display and alpha blend)**
  - Where: `OW_POPUP_GENERATION GEN_3 default, options GEN_5/B2W2 configs OW_POPUP_BW_* in include/config/overworld.h lines 118-135`
  - How: Set OW_POPUP_GENERATION to GEN_5 for the modern popup look
- **DPP-style NPC followers (partner trainers walking behind player)**
  - Where: `FNPC_ENABLE_NPC_FOLLOWERS FALSE in include/config/follower_npc.h; tutorial docs/tutorials/how_to_follower_npc.md`
  - How: Flip to TRUE + use script macros. Config comment: 'Slightly increases the size of the saveblock (SaveBlock3)'
- **FRLG-style map preview screens on map entry**
  - Where: `MPS_ENABLE_MAP_PREVIEWS IS_FRLG in include/config/map_preview_screen.h; tutorial docs/tutorials/how_to_map_preview_screen.md`
  - How: Set MPS_ENABLE_MAP_PREVIEWS TRUE (default only active for FRLG builds); previews must be authored per-map
- **Speaker name box over dialogue (modern visual convention)**
  - Where: `include/config/name_box.h (OW_NAME_BOX_* configs, OW_NAME_BOX_NPC_TRAINER FALSE); tutorial docs/tutorials/how_to_namebox.md`
  - How: Set OW_NAME_BOX_NPC_TRAINER TRUE for automatic trainer name boxes; use macros for scripted dialogue
- **Pokemon Sprite Visualizer debug screen (test any sprite/animation in-game)**
  - Where: `DEBUG_POKEMON_SPRITE_VISUALIZER DISABLED_ON_RELEASE in include/config/debug.h line 15; src/pokemon_sprite_visualizer.c`
  - How: Press Select on the summary screen in debug builds
- **Battle anim polish toggles: hide healthboxes during move anims, victory anim after KO, anim after failed ball**
  - Where: `B_HIDE_HEALTHBOX_IN_ANIMS TRUE, B_ANIMATE_MON_AFTER_KO TRUE, B_ANIMATE_MON_AFTER_FAILED_POKEBALL TRUE in include/config/battle.h lines 327-334`
  - How: Already on by default
- **Improved sprite/palette compression ('smol') keeping the huge sprite set within ROM budget**
  - Where: `INCGFX .4bpp.smol pipeline in src/data/graphics/pokemon.h; OW_GFX_COMPRESS TRUE in include/config/overworld.h line 54`
  - How: Automatic at build time

### Reuse candidates

- **DS-style Gen VII and Beyond Pokemon Sprite Repository in 64x64 (PokeCommunity)** — The upstream source of the Gen 7-9 sprites already in expansion; still active (Dropbox updated 2026) — source for missing second animation frames, new forms, and sprite fixes
  - Source: https://www.pokecommunity.com/threads/ds-style-gen-vii-and-beyond-pok%C3%A9mon-sprite-repository-in-64x64.368703/
  - Port effort: S — drop-in PNG replacement per species folder + `make`. Free to use WITH CREDIT to the thread's listed spriters (this exact thread is already cited in this repo's CREDITS.md 'Resources'); PokeCommunity blocks automated fetching (403), so read thread-OP usage rules manually before shipping
- **Platinum OW/Trainer Sprite Pack for pokeemerald/pokefirered** — DPP-style trainer battle sprites + NPC overworld sprites pre-formatted for pokeemerald-expansion — the main candidate for 'modern humans to match the DS-style mons'
  - Source: https://www.pokecommunity.com/threads/platinum-ow-trainer-sprite-pack-for-pokeemerald-pokefirered.537257/
  - Port effort: M — assets are expansion-ready but each trainer/OW sprite must be wired into graphics/trainers/ and graphics/object_events/pics/people/ (repo tutorials: docs/tutorials/how_to_trainer_front_pic.md, pret wiki INCGFX OW tutorial). Credit required per thread rules (verify OP manually — 403 to bots)
- **Modern Emerald (resetes12)** — Optional modernized battle backgrounds (menu-selectable), surf animations, follower variants — closest public-source example of the exact visual upgrades wanted
  - Source: https://github.com/resetes12/pokeemerald
  - Port effort: M — it is pokeemerald-based, not expansion-master, so code needs adaptation; raw background tilesets port as S (128x128 16-color PNG swap into graphics/battle_environment/). No formal license (standard pret situation): credit and ideally ask the author
- **pret pokeemerald wiki graphics tutorials** — Verified step-by-step guides: 'Editing Battle Backgrounds', 'Adding new event object or overworld sprites (INCGFX)', '80x80 Sprites', 'New Battle Transitions', 'Custom Battle Mugshots', 'Adding Walking Animations to All NPCs'
  - Source: https://github.com/pret/pokeemerald/wiki/Tutorials
  - Port effort: S — tutorials, not assets; apply directly to this tree
- **rh-hideout upstream sprite-issue trackers** — Issue #5883 'Missing mon animations' (open; itemized list of species missing 2nd frames) and #5135 'Current sprite issues' (open) — pull upstream fixes as they land instead of doing local spriting
  - Source: https://github.com/rh-hideout/pokeemerald-expansion/issues/5883
  - Port effort: S — periodic `git merge` from upstream master; zero local work
- **Emerald Rogue (Pokabbie)** — Public-source hack with custom visual/UI work; low value for THIS workstream specifically (its visuals are bespoke UI, mon sprites same lineage as expansion)
  - Source: https://github.com/Pokabbie/pokeemerald-rogue
  - Port effort: L — heavily diverged base; only worth mining for isolated ideas, not asset ports

### Must build

- Second animation frames for the ~443 static Gen 8/9 sprites (pure spriting labor, no code) — only if 100% two-frame coverage matters; otherwise wait on upstream issue #5883
- Per-map night lighting (.pla light-blend palettes + OBJ_EVENT_GFX_LIGHT_SPRITE lamp objects) following docs/tutorials/dns.md — manual per-map work, no code framework missing
- New battle-environment slots if adding (rather than replacing) backgrounds — small extension of gBattleEnvironmentInfo/src/battle_bg.c plus graphics

### Integration points

- /home/suleiman/pokeemerald-expansion/include/config/pokemon.h (sprite style, two-frame, icon configs)
- /home/suleiman/pokeemerald-expansion/include/config/overworld.h (followers, DNS, shadows, popups)
- /home/suleiman/pokeemerald-expansion/include/config/battle.h (enemy shadows, anim polish toggles)
- /home/suleiman/pokeemerald-expansion/include/config/follower_npc.h (NPC followers, SaveBlock3 impact)
- /home/suleiman/pokeemerald-expansion/graphics/pokemon/<species>/ + /home/suleiman/pokeemerald-expansion/src/data/graphics/pokemon.h (mon sprite assets + INCGFX refs)
- /home/suleiman/pokeemerald-expansion/graphics/battle_environment/ + /home/suleiman/pokeemerald-expansion/src/battle_bg.c (battle backgrounds, currently vanilla 12)
- /home/suleiman/pokeemerald-expansion/data/battle_anim_scripts.s + /home/suleiman/pokeemerald-expansion/src/data/moves_info.h (move animations)
- /home/suleiman/pokeemerald-expansion/graphics/trainers/front_pics (180 pics) + graphics/object_events/pics/people/ (trainer + NPC art swaps)
- /home/suleiman/pokeemerald-expansion/docs/tutorials/dns.md, how_to_trainer_front_pic.md, how_to_follower_npc.md, how_to_map_preview_screen.md, how_to_namebox.md (in-repo how-tos)

### Risks

- ROM budget: the in-tree build measures 25.29 MB used of the hard 32 MB GBA cap (pokeemerald.gba, 0xFF-padded) — ~6.7 MB headroom. OW/trainer packs and backgrounds fit easily; blanket multi-frame sprite additions or big tileset packs need budget tracking
- PokeCommunity returns 403 to automated fetching — per-thread usage/credit rules for the sprite packs were NOT machine-verified; owner must read thread OPs and add credits (repo precedent: CREDITS.md 'Resources'/'Other Credits' sections already credit these threads and DeviantArt spriters princess-phoenix/larryturbo/kidkatt)
- Enabling followers: config comment warns extra scripting may be needed for full support (cutscenes/scripted movement); OW_LARGE_OW_SUPPORT overhead already on
- OW_OBJECT_VANILLA_SHADOWS FALSE (default) makes EVERY overworld object cost two hardware sprites (verified comment, overworld.h line 105) — sprite-limit pressure on busy maps
- Reverting P_GBA_STYLE_SPECIES_GFX to GBA sprites disables B_ENEMY_MON_SHADOW_STYLE (verified config comment) — style choices are coupled
- DNS map lighting for vanilla Hoenn maps requires reverting upstream commit a5b079d833 (PR #6562), which conflicts with any other map edits made first — sequence this before map work
- ~443 Gen 8/9-era mons/forms have static front sprites; visible inconsistency next to animated Gen 1-7 mons; upstream fix is open-ended (issue #5883)
- Modern Emerald code (not assets) is on plain pokeemerald — porting its background-selection menu code to expansion master needs adaptation; assets themselves are trivial
- Sprites/visuals have near-zero SaveBlock impact EXCEPT FNPC_ENABLE_NPC_FOLLOWERS (grows SaveBlock3, verified comment) — coordinate with whichever workstream owns save-space budget

### Open decisions (owner's call)

- Follower Pokemon: enable OW_FOLLOWERS_ENABLED (HGSS charm, sprite-limit + scripting edge cases) vs leave off (cleaner, one config)
- Human art direction: keep vanilla RSE trainer/NPC sprites (zero work, style mismatch with DS-style mons) vs port Platinum OW/Trainer pack (M effort, consistent DS look, credit obligations)
- Battle backgrounds: keep vanilla 12 (free) vs replace in-place with a modern pack (S, per pret tutorial) vs port Modern Emerald's optional menu-selectable backgrounds (M, best UX)
- Night lighting: DNS tint only, already on (free) vs full per-map light-blending + lamp objects (significant manual map work; must precede other map edits)
- Gen 8/9 static sprites: accept single-frame (free) vs periodically pull second frames from upstream/DS-style repository thread (S recurring) vs sprite them locally (XL, not recommended)
- Map popups: keep Gen 3 style vs switch OW_POPUP_GENERATION to GEN_5 (B2W2 look; known alpha-blend conflict with DNS per dns.md)
- NPC followers (FNPC_ENABLE_NPC_FOLLOWERS): enable for story/partner moments at the cost of SaveBlock3 growth vs skip
