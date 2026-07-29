# Route → Gym Claim Table (v2 — VERIFIED against game data)

Verified 2026-07-28 against `src/data/wild_encounters.json` (124 Emerald headers → 116 distinct maps), map connections, collision/layout data, and gate scripts in `data/maps/*/scripts.inc`. Assignment rule: an area belongs to the gym before which it first becomes reachable under **natural vanilla badge/HM order** (arc roadblocks pre-cleared). Gym N's generator pool = areas of gyms 1..N. Sequence breaks from pre-set arc flags never make an assignment illegal (they only let the *player* reach late areas early — see Notes 1).

| Gym | Unlocked by | Claimed encounter maps |
|---|---|---|
| **1 — Roxanne** | walkable from New Game | ROUTE101, ROUTE102, ROUTE103, ROUTE104, PETALBURG_CITY, PETALBURG_WOODS, ROUTE116, RUSTURF_TUNNEL, ROUTE115 *(beach fishing only — see Notes 2)* |
| **2 — Brawly** | Briney sailing (flags pre-set), Old Rod (Dewford) | DEWFORD_TOWN, ROUTE106, GRANITE_CAVE_1F/B1F/B2F/STEVENS_ROOM |
| **3 — Wattson** | Briney Dewford→Slateport leg | ROUTE109, SLATEPORT_CITY, ROUTE110, ROUTE117, ROUTE111 *(pond fishing only)*, ROUTE118 *(west-bank fishing only)* |
| **4 — Flannery** | Badge 3 → Rock Smash; north-of-Mauville walk | ROUTE112, FIERY_PATH, ROUTE113, ROUTE114, METEOR_FALLS_1F_1R, JAGGED_PASS + all rock-smash tables in earlier maps |
| **5 — Norman** | Badge 4 → Go-Goggles → desert | MIRAGE_TOWER_1F-4F + ROUTE111 desert land table |
| **6 — Winona** | Badge 5 → **Surf**; Basement Key; Good Rod (Rte 118) | ROUTE105, ROUTE107, ROUTE108, ABANDONED_SHIP_ROOMS_B1F, NEW_MAUVILLE_ENTRANCE/INSIDE, ROUTE119, ROUTE120, ALTERING_CAVE + **every water table of earlier maps** + ROUTE118 grass + ROUTE115 grass *(both verified Surf-only)* |
| **7 — Tate & Liza** | Badge 6 era; Super Rod (Mossdeep) | ROUTE121, ROUTE122, ROUTE123, LILYCOVE_CITY, MT_PYRE (1F-6F, exterior, summit), SAFARI_ZONE (S/SW/N/NW), MAGMA_HIDEOUT (all 8 maps), ROUTE124, MOSSDEEP_CITY, ROUTE125, SHOAL_CAVE (5 low-tide rooms), ROUTE126, ROUTE127, ROUTE128 |
| **8 — Juan** | Badge 7 → **Dive** | UNDERWATER_ROUTE124/126, SOOTOPOLIS_CITY, CAVE_OF_ORIGIN_ENTRANCE/1F *(see Notes 3 — may be sealed)*, SEAFLOOR_CAVERN (entrance + rooms 1-8), ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS, SKY_PILLAR_1F/3F/5F, ROUTE129-134, PACIFIDLOG_TOWN |
| **E4** | Badge 8 → Waterfall | EVER_GRANDE_CITY, VICTORY_ROAD_1F/B1F/B2F, METEOR_FALLS_1F_2R/B1F_1R/B1F_2R |
| **Post-game** | game clear | SAFARI_ZONE_SE/NE, METEOR_FALLS_STEVENS_CAVE, DESERT_UNDERPASS, ARTISAN_CAVE_1F/B1F |
| **Unreachable** | — | 3 unused R/S Cave of Origin maps |

**Reconciliation:** 9+6+6+6+4+9+35+25+7 = 107 gym/E4 + 6 post-game + 3 unreachable = **116 ✔ (0 unassigned)**.

## Notes (generator requirements)

1. **Sequence breaks from pre-set arc flags** — Briney sailing works pre-badge-1; the whole gym-4 block is walkable pre-badge-3; Magma Hideout opens in the gym-4 era if `VAR_JAGGED_PASS_STATE=2` is pre-set; Sky Pillar at badge 5 if `FLAG_WALLACE_GOES_TO_SKY_PILLAR` pre-set; Steven hands over Dive on first Mossdeep visit (badge 5-6 era) if `VAR_STEVENS_HOUSE_STATE=1` pre-set. All flagged assignments stay at their natural gym (owner rule: "those routes belong to that gym") — breaks only ever favor the player, never make a gym pool illegal.
2. **Table-level gates inside claimed maps** (generator must filter at the *table* level, not map level):
   - Water tables anywhere = badge 5+ (gym 6 era), regardless of when the map was claimed.
   - Fishing (OWNER-MODIFIED rod placement, v3): **Old Rod = gym 1 era** (NPC on Route 103/104 — recreates the owner's randomizer build), **Good Rod = gym 2 era** (relocated from vanilla Route 118), Super Rod = gym 7 era (vanilla Mossdeep). Gym 1 therefore has land + old-rod fishing; all 9 gym-1 locations usable from the start (Petalburg City + Route 115 are fishing-only there).
   - **Per-gym slot capacity with these rods (computed from wild_encounters.json):** gym 1: 94 · gym 2: 167 · gym 3: 221 · gym 4: 318 · gym 5: 378 · gym 6: 597 · gym 7: 1241 · gym 8: 1587 (cumulative encounter slots = ceiling of distinct species per seed, before evolution closure).
   - **E4 is EXCLUDED from the availability rule** (owner decision): the Elite 4 may use any species; their team system is designed separately.
   - ROUTE111: pond fishing gym 3, rock-smash gym 4, desert land gym 5.
   - ROUTE118: all 110 grass tiles verified east of the river → land table is gym 6, west-bank fishing gym 3.
   - ROUTE115: grass verified reachable ONLY by Surf landing → land table gym 6; gym-1 claim is beach fishing.
   - GRANITE_CAVE_B2F rock-smash = gym 4; darkness (Flash) doesn't block land tables (genuinely gym 2).
   - ROUTE130 land table = Mirage Island (daily RNG) — **exclude from generation**.
   - ALTERING_CAVE: only table 1 (Zubat) is live; tables 2-9 are unused event data — exclude.
3. **Cave of Origin is likely sealed in this hack** — the entrance NPC only steps aside during arc crisis states that never occur with arcs pre-completed. Either script him away or exclude both maps from pools/nuzlocke routes. (Owner call, low stakes.)
4. Shoal Cave inner rooms follow the real-time tide cycle, not badges (kept in gym 7).
5. Lower-confidence details (vanilla-knowledge, not script-verified): rod NPC locations, Safari-NW Mach Bike need — neither changes any assignment.
