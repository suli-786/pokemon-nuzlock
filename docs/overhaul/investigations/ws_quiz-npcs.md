## Workstream: quiz-npcs

**Overall effort:** M

### Already in the expansion (verified in repo)

- **Dynamic multichoice menus (dynmultichoice / dynmultipush / dynmultistack) — scrollable runtime-built choice menus, the ideal quiz answer picker**
  - Where: `asm/macros/event.inc:1947-1981 (macros), include/constants/script_menu.h:229-236 (DYN_MULTICHOICE_CB_DEBUG/SHOW_ITEM/NONE), src/script_menu.c:100 ScriptMenu_MultichoiceDynamic + stack API at lines 247-305, in-repo tutorial docs/tutorials/how_to_dynmultichoice.md (published at https://rh-hideout.github.io/pokeemerald-expansion/tutorials/how_to_dynmultichoice.html, verified)`
  - How: Script: `dynmultichoice 0, 0, FALSE, N, 0, DYN_MULTICHOICE_CB_NONE, TextA, TextB, ...` then `switch VAR_RESULT`. Choice texts may embed {STR_VAR_1..3}, so a C special can inject answer strings at runtime; a callback set (like DYN_MULTICHOICE_CB_SHOW_ITEM) can preview info per highlighted row. Selection returns index in VAR_RESULT, B-press = 127.
- **Static multichoice + an existing 'teaching NPC' precedent: the Trainer's School blackboard status-condition lessons**
  - Where: `asm/macros/event.inc:946 (multichoice), :971 (multichoicegrid); MULTI_* ids in include/constants/script_menu.h (MULTI_NONE=255 cap); lesson usage `multichoicegrid 8, 1, MULTI_STATUS_INFO, 3, FALSE` in data/maps/RustboroCity_PokemonSchool/scripts.inc:13 and data/maps/ViridianCity_School_Frlg/scripts.inc:56`
  - How: Copy the blackboard pattern for menu-driven lesson NPCs; but prefer dynmultichoice for quizzes since static lists need a new MULTI_ id + entry in src/data/script_menu.h per menu.
- **Lilycove Quiz Lady — complete quiz question-pool/answer/prize framework (Easy-Chat-word format)**
  - Where: `src/lilycove_lady.c, src/data/lilycove_lady.h:285+ (`sQuizLadyQuestions[]` = {.question = 9 EC words, .answer = EC word, .prize = item}), data/scripts/lilycove_lady.inc:129+ (script flow with specials QuizLadyShowQuizQuestion, QuizLadyGetPlayerAnswer, QuizLadyPickNewQuestion, SetQuizLadyState_GivePrize), struct LilycoveLadyQuiz in include/global.h:884 (46 bytes already in SaveBlock1)`
  - How: Works out of the box in Lilycove PC. Its {question, answer, prize} pool struct + script/specials split is the authoring template to copy — but its Easy Chat word-entry answer UI is a bad fit for teaching type matchups; use dynmultichoice for answers instead.
- **FRLG content is merged into this tree (commit 97e83ebe6a 'Add FRLG (#7423)') including Cinnabar Gym quiz-door machines — working quiz-gated progression template**
  - Where: `data/maps/CinnabarIsland_Gym_Frlg/scripts.inc (CinnabarIsland_Gym_EventScript_Quiz1..6: MSGBOX_YESNO question, correct → SE_UNLOCK + door opens + setflag FLAG_CINNABAR_GYM_QUIZ_1..6, wrong → trainer battle)`
  - How: Direct copy-paste template for yes/no quiz gates and for the 'wrong answer = battle' consequence pattern; flags already exist in include/constants/flags.h.
- **String-buffer macros for composing auto-generated question text from game data**
  - Where: `asm/macros/event.inc:1225 bufferspeciesname, :1247 bufferitemname, :1261 buffermovename, :1268 buffernumberstring, :1282 bufferstring, :1940 bufferitemnameplural; gStringVar usage throughout src/`
  - How: A C special writes type/move/species names into gStringVar1-3; the script's msgbox question text and dynmultichoice answers reference {STR_VAR_1..3} — one generic script serves unlimited generated questions.
- **Trivially extensible specials + native script commands for quiz logic in C**
  - Where: `data/specials.inc:1 (`def_special` registration, e.g. line 20 HealPlayerParty), `callnative` macro asm/macros/event.inc:273, `special`/`specialvar` usage everywhere in data/scripts/`
  - How: Add `def_special Quiz_PickQuestion` etc.; scripts call `specialvar VAR_RESULT, Quiz_CheckAnswer`. This is exactly how the Quiz Lady is wired.
- **Random reward / random species script commands (expansion-only) for reward tables and generated questions**
  - Where: `asm/macros/event.inc:1044 `getrandomspecies dest, option` and :1053 `getrandomitem dest, option`, backed by RandomSpeciesGeneratorOptions/RandomItemGeneratorOptions; docs/tutorials/how_to_random_mon_generator.md; plus vanilla `random limit` macro at event.inc:1354`
  - How: Define a RandomItemGeneratorOptions entry per difficulty tier → `getrandomitem VAR_0x8004, QUIZ_TIER_X` gives randomized rewards with zero new code; getrandomspecies feeds 'what type is {species}?' questions.
- **All data tables needed to auto-derive quiz content already exist as C tables**
  - Where: `gTypeEffectivenessTable[NUMBER_OF_MON_TYPES][NUMBER_OF_MON_TYPES] and gTypesInfo[] in src/data/types_info.h:14,49 (externs include/battle_main.h:125-126; struct TypeInfo with .name/.generic at include/data.h:157); gMovesInfo[MOVES_COUNT_ALL] include/move.h:226 (power/type/category/description); gSpeciesInfo[] include/pokemon.h:725 (types/abilities/stats); gAbilitiesInfo[] include/pokemon.h:734 (name + description); GetTypeModifier() src/battle_util.c:8322`
  - How: A generator special indexes these tables directly, so questions automatically track any balance edits the owner makes (custom type chart, changed movesets) — no content ever goes stale.
- **Free persistence budget: quiz-answered/reward-claimed tracking without SaveBlock growth**
  - Where: `include/constants/flags.h — 381 `FLAG_UNUSED_*` entries plus DAILY_FLAGS region (line 1573, auto-reset daily); include/constants/vars.h — 29 `VAR_UNUSED_*`; flags live in existing SaveBlock1 u8 flags[NUM_FLAG_BYTES] (include/global.h:1132)`
  - How: Assign one unused flag per one-time quiz reward (`goto_if_set`/`setflag`, event.inc:1997); use DAILY_FLAGS for repeatable daily quizzes. Zero save-format change up to ~380 quiz NPCs.
- **TV/interview data structures — verified present but NOT a useful quiz base**
  - Where: `include/global.tv.h (TVShow union, line 493), gSaveBlock1Ptr->tvShows[TV_SHOWS_COUNT] include/global.h:1147, data/scripts/interview.inc`
  - How: Record-mixing/Easy-Chat oriented show records; no question/answer machinery worth reusing — noted so no one re-investigates it.

### Reuse candidates

- **Poryscript (high-level script compiler)** — Cheap authoring at scale: switch/case, if/else, inline text with automatic line-wrapping via format(), user constants — a hand-written quiz becomes ~10 readable lines instead of label spaghetti; hundreds of quizzes stay maintainable
  - Source: https://github.com/huderlem/poryscript
  - Port effort: S — documented Makefile integration (SCRIPT rule compiling data/%.pory → data/%.inc) and git-submodule install; MIT license, credit huderlem in CREDITS.md; adds a Go binary to the toolchain
- **FRLG Teachy TV app (adjacent: menu-driven battle-mechanics tutorials, not quizzes)** — A full in-game 'lessons' UI teaching battle mechanics — complements quizzes as the 'study material'; note this tree already has ITEM_TEACHY_TV, its icon and gfx (src/data/items.h:14148) but NOT the app itself (no src/teachy_tv.c here)
  - Source: https://github.com/pret/pokefirered/blob/master/src/teachy_tv.c
  - Port effort: L — pokefirered UI code with its own graphics/tilemaps and scripted demo battles; pret decomp code is customarily reusable with credit, same lineage as this repo. Only worth it if the owner wants lessons, not just quizzes

### Must build

- struct QuizQuestion data format + pools: {question text, 3-4 answer strings, correct index, difficulty tier, explanation text, reward} in a new src/data/quiz_questions.h, tiered arrays
- One generic driver script + ~4 new specials (Quiz_PickQuestion excluding already-answered flags, Quiz_BufferQuestion, Quiz_CheckAnswer, Quiz_GiveReward) so every quiz NPC's map script is one line: setvar VAR_0x8004 TIER; call EventScript_QuizNpc
- Authoring macro (asm .macro or poryscript template) wrapping question+answers+reward into one declaration so a new hand-written quiz is ~6 lines of data, zero script logic
- Auto-derivation generators in C: type-matchup questions from gTypeEffectivenessTable (pick atk/def type, buffer names via gTypesInfo, ask effectiveness — 4 fixed answers: super/not very/no/normal), move-fact questions from gMovesInfo (category/type/power), species/ability questions from gSpeciesInfo/gAbilitiesInfo — infinite content, zero authoring, auto-tracks owner's balance edits
- Teach-on-wrong-answer flow: show explanation text (hand-written) or computed correct answer (generated) before releasing the player
- Flag allocation block: reserve a contiguous run of FLAG_UNUSED_* (or extend FLAGS_COUNT) with QUIZ_FLAGS_START defines to avoid collisions with other workstreams

### Integration points

- asm/macros/event.inc (new quiz authoring macro; existing dynmultichoice/buffer macros)
- data/specials.inc (register new Quiz_* specials)
- src/script_menu.c (dynmultichoice callback set if answer-preview behavior wanted) or new src/quiz.c
- src/data/quiz_questions.h (new) alongside src/data/lilycove_lady.h as the pattern
- data/maps/*/scripts.inc — replace filler NPC scripts (both Hoenn and merged-FRLG Kanto maps exist in this tree)
- include/constants/flags.h (quiz progress flags from the 381 FLAG_UNUSED pool / DAILY_FLAGS)
- src/data/types_info.h, include/move.h, include/pokemon.h data tables (read-only inputs to generators)
- Makefile + tools/ if Poryscript is adopted
- data/scripts/lilycove_lady.inc + data/maps/CinnabarIsland_Gym_Frlg/scripts.inc (existing quiz content to keep/repurpose)

### Risks

- Flag-pool collision: other workstreams (nuzlocke enforcement, QoL) will also grab FLAG_UNUSED_*; without a central registry two branches will silently reuse the same flag — reserve ranges up front
- Auto-generated questions can leak owner customizations as 'wrong' answers if generators bypass config (e.g. B_FLAG_INVERSE_BATTLE mid-run) — generators must call GetTypeModifier()-level logic, not raw table reads, when a battle-config flag alters effectiveness
- dynmultichoice rows are single-line ListMenu items: answers must be short phrases; long questions belong in the msgbox, not the menu — content format should enforce length limits at compile time
- Randomized-nuzlocke context: species-specific generated questions may quiz mons the player never encounters; mechanics/type questions generalize, species trivia doesn't
- Authoring volume is the real cost: engine is days, but 'hundreds of quizzes' hand-written is weeks — the generator share of content determines schedule
- FRLG maps double the filler-NPC surface if the Kanto side is playable; scoping quiz NPC placement to Hoenn only halves the work
- Repeatable-quiz rewards are an economy faucet in a nuzlocke (free Rare Candies etc.); daily flags or one-time flags needed to keep runs honest

### Open decisions (owner's call)

- Answer input format: (a) 4-option dynmultichoice [recommended, teaches matchups directly], (b) yes/no Cinnabar-style [cheapest, weakest teaching], (c) Easy-Chat word entry Quiz-Lady-style [most 'authentic', worst UX]
- Content mix: what share auto-generated (type chart/moves/species — infinite, dry) vs hand-written (mechanics nuance: hazards, weather, ability interactions — costly, better teaching)?
- Repeatability/economy: one-time reward flag per NPC, daily-reset via DAILY_FLAGS, or infinitely repeatable with trivial rewards (money only)?
- Reward model: fixed item per question, tier-based getrandomitem pools, or escalating streak rewards?
- Adopt Poryscript for authoring (adds Go tool to build, big authoring win) or stay raw-asm macros (zero new deps)?
- Placement policy: convert ALL filler NPCs to quizzes, or one designated quiz NPC per town/route; and Hoenn-only vs also the merged FRLG Kanto maps?
- Fate of existing quiz content: keep Lilycove Quiz Lady and Cinnabar quiz doors as-is, or fold them into the new system?
- Wrong-answer consequence: nothing, explanation-then-retry (best for teaching), locked until tomorrow, or Cinnabar-style forced trainer battle?
