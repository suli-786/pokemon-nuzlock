## Workstream: dialog-speed

**Overall effort:** S

### Already in the expansion (verified in repo)

- **Instant text (global)**
  - Where: ``TEXT_SPEED_INSTANT` in /home/suleiman/pokeemerald-expansion/include/config/text.h:14; consumed by `GetPlayerTextSpeed()` at src/text.c:332 and `RenderText` at src/text.c:1336`
  - How: Set to TRUE; renders all text effectively instantly, overrides the in-game options menu and the flag
- **Instant text (runtime toggle)**
  - Where: ``FLAG_TEXT_SPEED_INSTANT` in /home/suleiman/pokeemerald-expansion/include/config/text.h:17 (default 0 = unassigned); free flags like `FLAG_UNUSED_0x264` at include/constants/flags.h:667`
  - How: Assign an unused flag ID, then setflag/clearflag from any script or the debug menu to toggle instant text per-save
- **Per-option text speed multipliers**
  - Where: ``TEXT_SPEED_SLOW_MODIFIER`/`TEXT_SPEED_MEDIUM_MODIFIER`/`TEXT_SPEED_FAST_MODIFIER` in include/config/text.h:10-12 (capped at 31 by STATIC_ASSERT in include/text.h:8-11)`
  - How: Raise FAST modifier; comment states 18-20 is essentially instant — lets 'Fast' in options = near-instant without touching flags
- **Auto-advance dialogue (auto-scroll)**
  - Where: ``AUTO_SCROLL_TEXT` + `NUM_FRAMES_AUTO_SCROLL_DELAY` (default 49) in include/config/text.h:5-6; logic in TextPrinterWaitAutoMode/TextPrinterWaitWithDownArrow, src/text.c:1237-1293; added in RHH PR #5054 per docs/changelogs/1.10.x/1.10.0.md:39`
  - How: Set AUTO_SCROLL_TEXT TRUE; text advances itself after the delay and A/B still advances early (src/text.c:1267-1273)
- **Faster battle text pauses**
  - Where: ``B_WAIT_TIME_MULTIPLIER` (default 16 = vanilla) in include/config/battle.h:328`
  - How: Lower the value; directly shortens every scripted pause in battle — the single biggest battle time-saver
- **Fast battle intro**
  - Where: ``B_FAST_INTRO_PKMN_TEXT` (TRUE by default) and `B_FAST_INTRO_NO_SLIDE` (FALSE) in include/config/battle.h:321-322`
  - How: Already on for text-during-animation; flip B_FAST_INTRO_NO_SLIDE TRUE to also cut the intro slide
- **Fast HP/EXP bar drain**
  - Where: ``B_FAST_HP_DRAIN` and `B_FAST_EXP_GROW`, both TRUE, include/config/battle.h:323-324`
  - How: Already enabled by default
- **B moves cursor to Run vs wild mons**
  - Where: ``B_QUICK_MOVE_CURSOR_TO_RUN` (FALSE) include/config/battle.h:329`
  - How: Set TRUE
- **Last-used Poke Ball hotkey + cycling**
  - Where: ``B_LAST_USED_BALL` TRUE, `B_LAST_USED_BALL_BUTTON` R_BUTTON, `B_LAST_USED_BALL_CYCLE` TRUE, include/config/battle.h:348-350`
  - How: Already on; R throws last ball, hold R + D-pad cycles — big nuzlocke catch-flow saver
- **Swap caught mon into party**
  - Where: ``B_CATCH_SWAP_INTO_PARTY` GEN_LATEST, include/config/battle.h:351 (+ HM guard :352)`
  - How: Already on via Gen7+ behavior
- **Repel/Lure re-use prompt with menu**
  - Where: ``I_REPEL_LURE_MENU` TRUE and `VAR_LAST_REPEL_LURE_USED` (0 = unassigned) in include/config/item.h:36-37`
  - How: Menu already on; assign an unused var to remember last-used repel/lure cursor position
- **Party-wide Exp Share toggle**
  - Where: ``I_EXP_SHARE_FLAG` (0) and `I_EXP_SHARE_ITEM` in include/config/item.h:31-32`
  - How: Assign a flag to enable Gen6-style whole-party exp, toggleable in-game
- **Reusable TMs**
  - Where: ``I_REUSABLE_TMS` FALSE, include/config/item.h:26`
  - How: Set TRUE
- **Run indoors**
  - Where: ``OW_RUNNING_INDOORS` GEN_LATEST, include/config/overworld.h:5`
  - How: Already on
- **Faster Pokemon Center heals**
  - Where: ``OW_UNION_DISABLE_CHECK` (FALSE) and `OW_FLAG_MOVE_UNION_ROOM_CHECK` (0) include/config/overworld.h:141-142; `OW_IGNORE_EGGS_ON_HEAL` :140`
  - How: Set OW_UNION_DISABLE_CHECK TRUE to skip the Union Room check dialogue/load (single-player = zero cost)
- **Suppress repeat map-name popups**
  - Where: ``OW_HIDE_REPEAT_MAP_POPUP` FALSE, include/config/overworld.h:11`
  - How: Set TRUE
- **Fly from map with R (Poke Rider)**
  - Where: ``OW_FLAG_POKE_RIDER` (0), include/config/overworld.h:113`
  - How: Assign a flag; press R on a fly-able location in PokeNav/Town Map to fly instantly
- **Tutors/traders pick from PC and party**
  - Where: ``OW_CHOOSE_FROM_PC_AND_PARTY` TRUE, include/config/overworld.h:15`
  - How: Already on
- **Berry Blender all-berries-at-once animation**
  - Where: ``BERRY_BLENDER_THROW_ALL_BERRIES_AT_ONCE` TRUE, include/config/overworld.h:145`
  - How: Already on
- **Item-obtain description box**
  - Where: ``OW_SHOW_ITEM_DESCRIPTIONS` (OFF) include/config/overworld.h:18-21`
  - How: ALWAYS mode is safe; FIRST_TIME mode is flagged save-breaking (SaveBlock3) in the comment
- **Title-screen Quickstart (testing)**
  - Where: ``ENABLE_QUICKSTART` TRUE, include/config/quickstart.h:9`
  - How: SELECT on title starts a new game instantly (disabled on release builds)
- **Debug menus (dev-time QoL)**
  - Where: ``DEBUG_OVERWORLD_MENU` (R+START) and `DEBUG_BATTLE_MENU` (SELECT in battle), include/config/debug.h:5,11, DISABLED_ON_RELEASE`
  - How: Already on in dev builds — use to set FLAG_TEXT_SPEED_INSTANT etc. without scripting
- **Internal 4th text speed 'Instant' options value**
  - Where: ``OPTIONS_TEXT_SPEED_INSTANT` = 3 in include/constants/global.h:196; NOT exposed in menu — src/option_menu.c:447-457 draws only Slow/Mid/Fast`
  - How: Value exists and is fully wired in src/text.c; exposing it in the options menu is a small build item

### Reuse candidates

- **pret wiki Simple Modifications: text speed-up / autoscroll / Auto-Run snippets** — Reference implementations for auto-mash text and auto-run; autoscroll is already upstreamed into expansion as AUTO_SCROLL_TEXT so only Auto-Run is novel
  - Source: https://github.com/pret/pokeemerald/wiki/Tutorials (verified; index links PokeCommunity posts p=10266385 'Make text speed-up like holding A/B automatically', p=10266389 'Make text autoscroll', p=10161076 'Auto-Run' — PokeCommunity itself returns 403 to automated fetches, owner must open in browser)
  - Port effort: S — snippets are a few lines each; expansion's field_player_avatar.c has diverged slightly so expect trivial conflict fixes. Community norm: credit the post authors in your credits list; Simple Modifications posts are shared for free use
- **Push B to Toggle Running Shoes (pret wiki)** — Toggle-run: one flag + edits to src/field_control_avatar.c and src/field_player_avatar.c; alternative to always-on auto-run
  - Source: https://github.com/pret/pokeemerald/wiki/Push-B-to-Toggle-Running-Shoes (verified to exist)
  - Port effort: S — small documented diff against vanilla pokeemerald; minor adaptation to expansion. Wiki tutorials are freely reusable, credit author customary
- **pokeemerald-ex-speedchoice (speedrun practice fork)** — Prior art for a start-of-game 'speed options' menu (instant text, plot skipping, spinner behavior); mostly superseded by expansion 1.16 configs
  - Source: https://github.com/ProjectRevoTPP/pokeemerald-ex-speedchoice (verified; speedchoice.diff contains INSTANT TEXT, PLOTLESS, SPINNERS options)
  - Port effort: XL — single giant diff against an ancient expansion base; do NOT port wholesale, only mine ideas (its instant-text is marked DEPRECATED in the diff). No explicit license beyond pret's; credit if copying code
- **Emerald Rogue source** — General aggressive-QoL reference from a shipped hack; I could NOT verify a specific turbo/text-skip implementation via code search, so treat as inspiration only
  - Source: https://github.com/Pokabbie/pokeemerald-rogue (verified to exist, public decomp-based source)
  - Port effort: L — different codebase generation, cherry-picking anything is nontrivial; verify feature exists before investing. Pokabbie asks for credit for reused work (standard romhack norm)

### Must build

- Hold-button skip-all: no verified drop-in patch for expansion 1.16.x exists; build is ~20-30 lines — extend SetResultWithButtonPress (src/text.c:1251) to also accept JOY_HELD of a chosen button (A, or dedicated R 'turbo'), ideally gated behind a config/flag, combined with TEXT_SPEED_INSTANT for full skip
- Expose 'Instant' as a 4th Text Speed choice in the options menu (src/option_menu.c currently draws only Slow/Mid/Fast at :447-457, though OPTIONS_TEXT_SPEED_INSTANT=3 is already wired in src/text.c) — only needed if owner prefers a player-facing setting over a flag
- Optional anti-overshoot debounce for hold-skip (e.g. minimum 5-10 frames per textbox while held) so one hold cannot blast through several boxes unread

### Integration points

- /home/suleiman/pokeemerald-expansion/include/config/text.h (all dialog-speed knobs)
- /home/suleiman/pokeemerald-expansion/src/text.c — GetPlayerTextSpeed:324, IsPlayerTextSpeedInstant:353, SetResultWithButtonPress:1251 (single choke point for ALL field/battle text-wait prompts — hold-skip patch goes here), TextPrinterWaitAutoMode:1237, TextPrinterWaitWithDownArrow:1260, TextPrinterWait:1278, RenderText:1327-1353
- /home/suleiman/pokeemerald-expansion/src/field_message_box.c:21-24 (resets gTextFlags per field message; canABSpeedUpPrint is FALSE in field — held A/B currently does nothing outside battle)
- /home/suleiman/pokeemerald-expansion/src/battle_message.c:3860-3890 (battle text sets canABSpeedUpPrint/autoScroll)
- /home/suleiman/pokeemerald-expansion/src/option_menu.c (Slow/Mid/Fast UI)
- /home/suleiman/pokeemerald-expansion/src/menu.c:743+ (yes/no and multichoice input is JOY_NEW-only — the structural guarantee that a held skip button can never auto-confirm a prompt)
- /home/suleiman/pokeemerald-expansion/data/scripts/obtain_item.inc + src/scrcmd.c:1203 ScrCmd_waitfanfare (item-receive blocks on fanfare independent of text-wait — hold-skip cannot bypass it)
- /home/suleiman/pokeemerald-expansion/include/config/battle.h:321-352 (battle QoL block)
- /home/suleiman/pokeemerald-expansion/include/config/item.h and include/config/overworld.h (overworld QoL toggles)
- /home/suleiman/pokeemerald-expansion/include/constants/flags.h (assign FLAG_TEXT_SPEED_INSTANT / repel var / exp-share flag from unused slots)

### Risks

- Hold-skip overshoot: skipping one-time NPC dialogue loses information permanently in a no-external-sites run — mitigate with debounce, or prefer instant-text + AUTO_SCROLL_TEXT over a true hold-skip
- Yes/No + item safety is solid by construction (menus poll JOY_NEW only, src/menu.c; item-get blocks on waitfanfare) but SetResultWithButtonPress feeds EVERY textbox in the game (PokeNav, braille, credits) — needs a playtest pass after patching
- AUTO_SCROLL_TEXT is compile-time global — it also auto-advances important story text; there is no per-message opt-out unless scripts set gTextFlags.autoScroll individually
- FLAG_TEXT_SPEED_INSTANT, VAR_LAST_REPEL_LURE_USED, I_EXP_SHARE_FLAG, OW_FLAG_POKE_RIDER all default to 0 (disabled) and each consumes one unused flag/var — no SaveBlock growth (flags/vars already exist in SaveBlock1), but coordinate flag allocation with the nuzlocke workstream to avoid collisions
- OW_SHOW_ITEM_DESCRIPTIONS in FIRST_TIME mode is explicitly marked SAVE-BREAKING (SaveBlock3) in include/config/overworld.h:19 — decide before first real save
- PokeCommunity snippet posts 403 automated fetches — the owner must retrieve them manually via browser; credit post authors per Simple Modifications community norms
- TEXT_SPEED modifiers cap at 31 (STATIC_ASSERT include/text.h:8-11) to keep printing synced with A/B input — do not bypass the cap

### Open decisions (owner's call)

- Instant-text delivery: (a) TEXT_SPEED_INSTANT=TRUE always-on (zero effort, no toggle), (b) assign FLAG_TEXT_SPEED_INSTANT for an in-game toggle via script/debug menu (near-zero effort), or (c) build the 4th 'Instant' options-menu entry (small build, cleanest UX)
- Skip-all mechanism: (a) no new code — instant text + AUTO_SCROLL_TEXT=TRUE approximates skip-all with prompts still blocking, (b) patch SetResultWithButtonPress for hold-A/B skip, or (c) dedicated R-button 'turbo' held modifier — (b)/(c) are the only true hold-skips
- Auto-advance default: keep NUM_FRAMES_AUTO_SCROLL_DELAY at 49 (~0.8s) or tune down; whether auto-advance should apply globally at compile time given the lost-information risk
- Battle text pace: how far to lower B_WAIT_TIME_MULTIPLIER from 16 (8 = roughly 2x faster pauses; too low makes multi-effect turns hard to follow in nuzlocke decision-making)
- Movement QoL: auto-run always-on ('Running speed by default' snippet) vs B-toggle running shoes (pret wiki tutorial) vs leave as-is with OW_RUNNING_INDOORS already enabled
- Whether to flip the currently-off freebies: B_FAST_INTRO_NO_SLIDE, B_QUICK_MOVE_CURSOR_TO_RUN, OW_HIDE_REPEAT_MAP_POPUP, OW_UNION_DISABLE_CHECK, I_REUSABLE_TMS, and assign OW_FLAG_POKE_RIDER / I_EXP_SHARE_FLAG / VAR_LAST_REPEL_LURE_USED
