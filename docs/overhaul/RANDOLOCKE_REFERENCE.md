# Randolocke v1.1 — Source-Build Reference

The owner's previous randomizer nuzlocke build ("Randolocke", played as a .bps patch on Emerald). Facts below come from its official v1.1 release notes (owner-provided). This is the reference model our randomizer copies or consciously departs from.

## Confirmed facts from the release notes

**Randomization**
- Species randomization is **per slot** (stated verbatim).
- Opponents' moves ARE randomized by default; **menu toggles exist to NOT randomize abilities and NOT randomize learnsets** (the owner played with these off — matches our no-random-abilities/moves decision).
- TMs are randomized.
- With randomized learnsets, default mode re-sorts so **powerful moves are learned later**.
- Randomization appears to be **runtime, per save** (a bug fix mentions TM randomization not persisting across power cycles).

**QoL / economy NPCs (Oldale Town)**
- NPC gifts **999 Ultra Balls** — repeatable, every talk. *(Owner remembered Master Balls — actually Ultra.)*
- NPC gifts **$999,999** — repeatable.
- Catch rates increased globally.

**Nuzlocke options (in-game menu, set at New Game)**
- Enforce first-encounter-per-route catching.
- Disable bag during battles.
- Enforce nicknaming.
- **Porta Heal**: healing item; default mode does NOT revive fainted Pokémon.

**Legendaries**
- An NPC sells maps to all legendary locations.
- Regis, Kyogre, Groudon, Latias, Latios unlock right before gym 8 (post-Rayquaza event).

## Identified (web research + ROM analysis, 2026-07-29)
- **Pokémon Randolocke v1.1 by Istorian** (PokéCommunity, Aug 2025) — public recreation of the PChal/PointCrow Nuzlocke Invitational ROM.
- Built on **pokeemerald-expansion 1.12.1** + Zetraphes' `tertu-randomizer` fork — one custom module, `src/randomizer.c`. Confirmed by our ROM string analysis (same codebase family as this project).
- **Randomization is runtime, per save: seed = Trainer ID.**

## The algorithm (from its source/thread)
- **Wilds:** `RandomizeWildEncounter` — deterministic per-slot mapping seeded on (map, area-type, slot). Replacement drawn uniformly from species within **±10% BST** of the vanilla occupant. Gen 1–9 pool.
- **No evolution-stage rule** — BST proximity is the only constraint (final forms can appear early if BST-close).
- **Legendaries ARE eligible** in wild/trainer slots when BST-close (wild Zygarde and Norman rolling Dialga are documented); only *static* legendaries are event-gated (Regis/Kyogre/Groudon pre-gym-8, map-seller for Lati@s/Mew/Deoxys/Ho-oh/Lugia post-Rayquaza).
- **Slot odds untouched** (vanilla land 20/20/10/10/10/10/5/5/4/4/1/1 etc.).
- **Trainers:** `RandomizeTrainerMon` seeded on (trainerId, slot, party size), same ±10% BST rule; levels rescaled to hard caps **14/21/24/29/36/43/47/50/63/100**; abilities/movesets/TMs randomized by default (owner played with ability/learnset randomization OFF via menu toggles).
- **Old Rod sailor moved to Route 103** (v1.0) — owner's early-rod memory confirmed.
- **Ultra Balls, not Master** — the author deliberately excluded Master Balls ("guarantee capture goes against the thrill of randomness") and raised catch rates instead.

## The custom item kit (decoded from the ROM — exact in-game text)
| Item | Effect (verbatim description) |
|---|---|
| **Endless Candy** | "Endlessly raises the level of a Pokémon by one." — infinite-use +1 level |
| **Cap Candy** | "Endlessly raises the level of a Pokémon until the cap." — infinite-use, straight to level cap |
| **Repellant** | Key-item toggle: "The Repellant is activated. Wild Pokémon will be repelled." Bag text: "Endlessly repels **weak** wild Pokémon" — i.e. classic Repel semantics (blocks encounters below your lead's level), NOT a total encounter switch |
| **Porta Heal** | "Party Pokémon healed successfully." — full party heal anywhere; v1.1 default: does NOT revive |

## Custom NPCs found in ROM text
- Oldale: 999 Ultra Balls giver + $999,999 giver (repeatable).
- Route 103: Old Rod sailor (moved from Dewford).
- **Istorian (the designer) appears as an in-game NPC**: gifts **Eternatus** for beating the Elite Four, plus a Pokédex-completion reward.
- Legendary map seller (Slateport; post-Rayquaza maps to Lati@s/Mew/Deoxys/Ho-oh/Lugia).

**Owner directive (2026-07-29): Randolocke is the BASELINE for this project** — its behaviors are the default; the overhaul = Randolocke's game + the owner's additions (nuzlocke tracker/enforcement, VGC doubles gyms, quiz/wager NPCs, knowledge system, UI overhaul...). When any of my earlier approximations conflict with verified Randolocke behavior, Randolocke's version wins unless the owner overrides.

## Source availability (verified 2026-07-29)
- Randolocke's own additions (Oldale NPCs, learnset generator, RANDOM_RULES, menu toggles): **no public source** (Istorian; PokéCommunity thread #537596).
- **The randomizer engine IS public**: Randolocke credits Zetraphes' `tertu-randomizer` branch (github.com/Zetraphes/pokeemerald-expansion), itself from tertu-m's randomizer — which exists as **open upstream PR rh-hideout#3998 "Standalone randomizer"** against OUR codebase. Exact fingerprint match (`GetSpeciesGroup` string). Local copies saved to scratchpad (`randomizer_tertu.c`).
- `src/randomizer.c` (993 lines) mechanics: group modes `MON_RANDOM` / `MON_RANDOM_BST` (±100/1024 ≈ ±9.8% BST band, heap-sorted + binary-searched) / `MON_RANDOM_LEGEND_AWARE` (legendary-flag groups) / `MON_EVOLUTION` (evo-stage groups); SFC32 stream seeded per encounter slot (global seed ⊕ map/area/slot); trainers seeded per (trainerId, partySize, slot); starters/gifts via `GetUniqueMonList` (guaranteed no duplicates).
- **PROPOSED (pending owner confirm):** port this module to 1.16.3 as our randomizer base (runtime, seed-per-save — new run without rebuilding) and build the claim-table gym generator + Option-C sanity rules on top, replacing the build-time Python plan. Credits: tertu-m, Zetraphes; Istorian for the reference design.
