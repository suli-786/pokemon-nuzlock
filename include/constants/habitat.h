#ifndef GUARD_CONSTANTS_HABITAT_H
#define GUARD_CONSTANTS_HABITAT_H

// Overhaul: habitats for the randomizer's species distribution (ROADMAP 7.12
// rule 1). A species belongs to a SET of habitats, and an encounter table is
// tagged with the set it accepts, so a candidate is legal when the two sets
// intersect. Sets rather than single values on both sides: a Golbat is at home
// in a cave and on a mountain, and a mountain path is both mountain and field.
// The overlap is what stops the low-BST bands starving -- see docs/overhaul.
//
// Eleven habitats, per ROADMAP 7.12. There is deliberately no freshwater
// habitat: it was prototyped and dropped, being the thinnest bucket by far
// (37 species) and absent from the spec. Ponds and rivers are surface water.

#define HABITAT_NONE            0
#define HABITAT_FIELD           (1 << 0)   // routes, grassland, meadows
#define HABITAT_FOREST          (1 << 1)   // woods, dense tree cover
#define HABITAT_CAVE            (1 << 2)   // caves, tunnels, underground
#define HABITAT_MOUNTAIN        (1 << 3)   // slopes, volcanic ground, ash
#define HABITAT_DESERT          (1 << 4)   // sand, arid
#define HABITAT_RUINS           (1 << 5)   // tombs, towers, graveyards
#define HABITAT_INDUSTRIAL      (1 << 6)   // machinery, wrecks, generators
#define HABITAT_ICE             (1 << 7)   // ice and snow
#define HABITAT_SEA_SURFACE     (1 << 8)   // surfable water, ponds, rivers
#define HABITAT_SEA             (1 << 9)   // open sea, fished depths
#define HABITAT_DEEP_SEA        (1 << 10)  // underwater (Dive) maps

#define HABITAT_COUNT           11
#define HABITAT_ALL             ((1 << HABITAT_COUNT) - 1)

// Any water at all. Used for the "borrow from an adjacent habitat" fallback.
#define HABITAT_ANY_WATER       (HABITAT_SEA_SURFACE | HABITAT_SEA | HABITAT_DEEP_SEA)

#endif // GUARD_CONSTANTS_HABITAT_H
