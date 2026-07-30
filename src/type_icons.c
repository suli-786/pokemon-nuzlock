#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "decompress.h"
#include "graphics.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon_summary_screen.h"
#include "sprite.h"
#include "type_icons.h"

static void LoadTypeSpritesAndPalettes(void);
static void LoadTypeIconsPerBattler(enum BattlerId, u32);

static bool32 UseDoubleBattleCoords(u32);

static enum Type GetMonPublicType(enum BattlerId, u32);
static bool32 ShouldHideUncaughtType(enum Species species);
static bool32 ShouldHideUnseenType(enum Species species);
static enum Type GetMonDefensiveTeraType(struct Pokemon *, struct Pokemon *, enum BattlerId, u32, enum Species, enum Species);
static bool32 IsIllusionActiveAndTypeUnchanged(struct Pokemon *, enum Species, enum BattlerId);

static void CreateSpriteFromType(u32, bool32, enum Type[], u32, enum BattlerId);
static bool32 ShouldSkipSecondType(enum Type[], u32);
static void SetTypeIconXY(s32*, s32*, u32, bool32, u32);

static void CreateSpriteAndSetTypeSpriteAttributes(enum Type, u32 x, u32 y, u32, enum BattlerId, bool32);
static bool32 ShouldFlipTypeIcon(bool32, u32, enum Type);

static void SpriteCB_TypeIcon(struct Sprite*);
static void DestroyTypeIcon(struct Sprite*);
static void FreeAllTypeIconResources(void);
static bool32 ShouldHideTypeIcon(enum BattlerId);
static s32 GetTypeIconHideMovement(bool32, u32);
static s32 GetTypeIconSlideMovement(bool32, u32, s32);
static s32 GetTypeIconBounceMovement(s32, u32);

// SWSH_BATTLE_UI redraws the healthbox as a right-leaning parallelogram, which pushes the
// *top* of the opponent's doubles frame 4px further right than the vanilla art. Type icons
// are drawn behind the healthbox (subpriority 255 vs 1), so anything that lands under the
// frame is simply hidden. Measured against the two arts:
//
//   position               vanilla covered   SwSh covered (before this shift)
//   singles player L       3px of 8          3px, and only on the icon's top row
//   singles opponent L     1px of 8          0-2px
//   doubles player L/R     up to 8px         up to 3px
//   doubles opponent L/R   2px of 8          6px of 8 on the whole upper icon  <-- regression
//
// Only the doubles opponent needed correcting: its frame's widest row reaches image
// column 96, so the icons have to start at column 97, i.e. 6px further right. Every other
// slot is the same or better than vanilla and is left alone.
// See docs/overhaul/UI_PORT_CHECKLIST.md §3.6 for the full derivation.
#if SWSH_BATTLE_UI
#define SWSH_OPPONENT_DOUBLES_TYPE_ICON_X_SHIFT 6
#else
#define SWSH_OPPONENT_DOUBLES_TYPE_ICON_X_SHIFT 0
#endif

const struct Coords16 sTypeIconPositions[][2] =
{
    // Overhaul: the singles icons were straddling the edge of their own healthbox
    // and getting clipped -- the player's sat at x 221 (spanning 205-237) while the
    // card ends near 235, and the opponent's at x 20 (spanning 4-36) while the card
    // only starts at 12. Both are pulled inboard so they sit on the card, which has
    // room for them now the exp bar is gone.
    [B_POSITION_PLAYER_LEFT] =
    {
        [FALSE] = {212, 84},
        [TRUE] = {144, 71},
    },
    [B_POSITION_OPPONENT_LEFT] =
    {
        [FALSE] = {34, 50},
        [TRUE] = {97 + SWSH_OPPONENT_DOUBLES_TYPE_ICON_X_SHIFT, 14},
    },
    [B_POSITION_PLAYER_RIGHT] =
    {
        [TRUE] = {156, 96},
    },
    [B_POSITION_OPPONENT_RIGHT] =
    {
        [TRUE] = {85 + SWSH_OPPONENT_DOUBLES_TYPE_ICON_X_SHIFT, 39},
    },
};

const union AnimCmd sSpriteAnim_TypeIcon_Normal[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_NORMAL), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Fighting[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_FIGHTING), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Flying[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_FLYING), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Poison[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_POISON), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Ground[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_GROUND), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Rock[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_ROCK), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Bug[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_BUG), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Ghost[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_GHOST), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Steel[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_STEEL), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Mystery[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_MYSTERY), 0),
    ANIMCMD_END
};

const union AnimCmd sSpriteAnim_TypeIcon_Fire[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_FIRE), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Water[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_WATER), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Grass[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_GRASS), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Electric[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_ELECTRIC), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Psychic[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_PSYCHIC), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Ice[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_ICE), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Dragon[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_DRAGON), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Dark[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_DARK), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Fairy[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_FAIRY), 0),
    ANIMCMD_END
};

const union AnimCmd *const sSpriteAnimTable_TypeIcons[] =
{
    [TYPE_NONE] =       sSpriteAnim_TypeIcon_Mystery,
    [TYPE_NORMAL] =     sSpriteAnim_TypeIcon_Normal,
    [TYPE_FIGHTING] =   sSpriteAnim_TypeIcon_Fighting,
    [TYPE_FLYING] =     sSpriteAnim_TypeIcon_Flying,
    [TYPE_POISON] =     sSpriteAnim_TypeIcon_Poison,
    [TYPE_GROUND] =     sSpriteAnim_TypeIcon_Ground,
    [TYPE_ROCK] =       sSpriteAnim_TypeIcon_Rock,
    [TYPE_BUG] =        sSpriteAnim_TypeIcon_Bug,
    [TYPE_GHOST] =      sSpriteAnim_TypeIcon_Ghost,
    [TYPE_STEEL] =      sSpriteAnim_TypeIcon_Steel,
    [TYPE_MYSTERY] =    sSpriteAnim_TypeIcon_Mystery,
    [TYPE_FIRE] =       sSpriteAnim_TypeIcon_Fire,
    [TYPE_WATER] =      sSpriteAnim_TypeIcon_Water,
    [TYPE_GRASS] =      sSpriteAnim_TypeIcon_Grass,
    [TYPE_ELECTRIC] =   sSpriteAnim_TypeIcon_Electric,
    [TYPE_PSYCHIC] =    sSpriteAnim_TypeIcon_Psychic,
    [TYPE_ICE] =        sSpriteAnim_TypeIcon_Ice,
    [TYPE_DRAGON] =     sSpriteAnim_TypeIcon_Dragon,
    [TYPE_DARK] =       sSpriteAnim_TypeIcon_Dark,
    [TYPE_FAIRY] =      sSpriteAnim_TypeIcon_Fairy,
    [TYPE_STELLAR] =    sSpriteAnim_TypeIcon_Mystery,
};

const struct SpritePalette sTypeIconPal1 =
{
    .data = gBattleIcons_Pal1,
    .tag = TYPE_ICON_TAG
};

const struct SpritePalette sTypeIconPal2 =
{
    .data = gBattleIcons_Pal2,
    .tag = TYPE_ICON_TAG_2
};

const struct OamData sOamData_TypeIcons =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x16),
    .size = SPRITE_SIZE(8x16),
    .priority = 1,
};

const struct CompressedSpriteSheet sSpriteSheet_TypeIcons2 =
{
    .data = gBattleIcons_Gfx2,
    .size = (8*16) * 9,
    .tag = TYPE_ICON_TAG_2,
};

const struct CompressedSpriteSheet sSpriteSheet_TypeIcons1 =
{
    .data = gBattleIcons_Gfx1,
    .size = (8*16) * 10,
    .tag = TYPE_ICON_TAG,
};

const struct SpriteTemplate sSpriteTemplate_TypeIcons1 =
{
    .tileTag = TYPE_ICON_TAG,
    .paletteTag = TYPE_ICON_TAG,
    .oam = &sOamData_TypeIcons,
    .anims = sSpriteAnimTable_TypeIcons,
    .callback = SpriteCB_TypeIcon
};

const struct SpriteTemplate sSpriteTemplate_TypeIcons2 =
{
    .tileTag = TYPE_ICON_TAG_2,
    .paletteTag = TYPE_ICON_TAG_2,
    .oam = &sOamData_TypeIcons,
    .anims = sSpriteAnimTable_TypeIcons,
    .callback = SpriteCB_TypeIcon
};

void LoadTypeIcons(enum BattlerId battler)
{
    u32 position;

    struct Pokemon* mon = GetBattlerMon(battler);
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);

    if (B_SHOW_TYPES == SHOW_TYPES_NEVER
        || (B_SHOW_TYPES == SHOW_TYPES_SEEN && !GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN)))
        return;

    LoadTypeSpritesAndPalettes();

    for (position = 0; position < gBattlersCount; ++position)
        LoadTypeIconsPerBattler(battler, position);
}

static void LoadTypeSpritesAndPalettes(void)
{
#if SWSH_BATTLE_UI
    // Overhaul: the opponent's types are shown with the 32x16 name badges the
    // summary screen already uses (graphics/types/*.png via gSpriteSheet_MoveTypes)
    // instead of the stock 8x16 chips. At 8 pixels the chip's symbol is unreadable --
    // poison, ghost and psychic are indistinguishable -- while the badge spells the
    // type out. Nothing is drawn; this is the same art the rest of the game uses.
    //
    // The badges want 3 OBJ palettes at slots 13-15 (matching how the summary screen
    // loads them, since gTypesInfo[].palette indexes that block) against the chips'
    // 2, and about 6.6KB of object VRAM against 2.4KB. That is affordable only
    // because the player's icons are no longer created at all -- see
    // LoadTypeIconsPerBattler.
    if (GetSpriteTileStartByTag(gSpriteSheet_MoveTypes.tag) != 0xFFFF)
        return;

    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
    LoadPalette(gMoveTypes_Pal, OBJ_PLTT_ID(13), 3 * PLTT_SIZE_4BPP);
#else
    if (IndexOfSpritePaletteTag(TYPE_ICON_TAG) != UCHAR_MAX)
        return;

    LoadCompressedSpriteSheet(&sSpriteSheet_TypeIcons1);
    LoadCompressedSpriteSheet(&sSpriteSheet_TypeIcons2);
    LoadSpritePalette(&sTypeIconPal1);
    LoadSpritePalette(&sTypeIconPal2);
#endif
}

static void LoadTypeIconsPerBattler(enum BattlerId battler, u32 position)
{
    u32 typeNum;
    enum Type types[2];
    enum BattlerId battlerId = GetBattlerAtPosition(position);
    bool32 useDoubleBattleCoords = UseDoubleBattleCoords(battlerId);

    if (!IsBattlerAlive(battlerId))
        return;

#if SWSH_BATTLE_UI
    // Opponent only. You already know your own Pokemon's typing, and the badges are
    // four times the width of the chips they replace -- showing all four on a 240px
    // screen would bury the battlefield. Skipping the player's side is also what pays
    // for the badges' extra palette and VRAM.
    if (IsOnPlayerSide(battlerId))
        return;
#endif

    for (typeNum = 0; typeNum < 2; ++typeNum)
        types[typeNum] = GetMonPublicType(battlerId, typeNum);

    for (typeNum = 0; typeNum < 2; ++typeNum)
        CreateSpriteFromType(position, useDoubleBattleCoords, types, typeNum, battler);
}

static bool32 UseDoubleBattleCoords(u32 position)
{
    if (!IsDoubleBattle())
        return FALSE;

    if ((position == B_POSITION_PLAYER_LEFT) && (gBattleMons[B_POSITION_PLAYER_RIGHT].species == SPECIES_NONE))
        return FALSE;

    if ((position == B_POSITION_OPPONENT_LEFT) && (gBattleMons[B_POSITION_OPPONENT_RIGHT].species == SPECIES_NONE))
        return FALSE;

    return TRUE;
}

static enum Type GetMonPublicType(enum BattlerId battlerId, u32 typeNum)
{
    struct Pokemon *mon = GetBattlerMon(battlerId);
    enum Species monSpecies = GetMonData(mon,MON_DATA_SPECIES,NULL);
    struct Pokemon *monIllusion;
    enum Species illusionSpecies;

    if (ShouldHideUncaughtType(monSpecies) || ShouldHideUnseenType(monSpecies))
        return TYPE_MYSTERY;

    monIllusion = GetIllusionMonPtr(battlerId);
    illusionSpecies = GetMonData(monIllusion,MON_DATA_SPECIES,NULL);

    if (GetActiveGimmick(battlerId) == GIMMICK_TERA)
        return GetMonDefensiveTeraType(mon,monIllusion,battlerId,typeNum,illusionSpecies,monSpecies);

    if (IsIllusionActiveAndTypeUnchanged(monIllusion,monSpecies, battlerId))
        return GetSpeciesType(illusionSpecies, typeNum);

    return gBattleMons[battlerId].types[typeNum];
}

static bool32 ShouldHideUncaughtType(enum Species species)
{
    if (B_SHOW_TYPES != SHOW_TYPES_CAUGHT)
        return FALSE;

    if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
        return FALSE;

    return TRUE;
}

static bool32 ShouldHideUnseenType(enum Species species)
{
    if (B_SHOW_TYPES != SHOW_TYPES_SEEN)
        return FALSE;

    if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        return FALSE;

    return TRUE;
}

static enum Type GetMonDefensiveTeraType(struct Pokemon *mon, struct Pokemon *monIllusion, enum BattlerId battlerId, u32 typeNum, enum Species illusionSpecies, enum Species monSpecies)
{
    enum Type teraType = GetBattlerTeraType(battlerId);
    enum Species targetSpecies;

    if (teraType != TYPE_STELLAR)
        return teraType;

    targetSpecies = (monIllusion != NULL) ? illusionSpecies : monSpecies;

    return GetSpeciesType(targetSpecies, typeNum);
}

static bool32 IsIllusionActiveAndTypeUnchanged(struct Pokemon *monIllusion, enum Species monSpecies, enum BattlerId battlerId)
{
    u32 typeNum;

    if (monIllusion == NULL)
        return FALSE;

    for (typeNum = 0; typeNum < 2; typeNum++)
        if (GetSpeciesType(monSpecies, typeNum) != gBattleMons[battlerId].types[typeNum])
        return FALSE;

    return TRUE;
}

static void CreateSpriteFromType(u32 position, bool32 useDoubleBattleCoords, enum Type types[], u32 typeNum, enum BattlerId battler)
{
    s32 x = 0, y = 0;

    if (ShouldSkipSecondType(types, typeNum))
        return;

    SetTypeIconXY(&x, &y, position, useDoubleBattleCoords, typeNum);

    CreateSpriteAndSetTypeSpriteAttributes(types[typeNum], x, y, position, battler, useDoubleBattleCoords);
}

static bool32 ShouldSkipSecondType(enum Type types[], u32 typeNum)
{
    if (!typeNum)
        return FALSE;

    if (types[0] != types[1])
        return FALSE;

    return TRUE;
}

static void SetTypeIconXY(s32* x, s32* y, u32 position, bool32 useDoubleBattleCoords, u32 typeNum)
{
    *x = sTypeIconPositions[position][useDoubleBattleCoords].x;
    // 11px apart suits the 8x16 chips; the 32x16 badges need 17 or the second type
    // overlaps the first.
    *y = sTypeIconPositions[position][useDoubleBattleCoords].y
       + ((SWSH_BATTLE_UI ? 17 : 11) * typeNum);
}

static void CreateSpriteAndSetTypeSpriteAttributes(enum Type type, u32 x, u32 y, u32 position, enum BattlerId battler, bool32 useDoubleBattleCoords)
{
    struct Sprite* sprite;
#if SWSH_BATTLE_UI
    const struct SpriteTemplate* spriteTemplate = &gSpriteTemplate_MoveTypes;
#else
    const struct SpriteTemplate* spriteTemplate = gTypesInfo[type].useSecondTypeIconPalette ? &sSpriteTemplate_TypeIcons2 : &sSpriteTemplate_TypeIcons1;
#endif
    u32 spriteId = CreateSpriteAtEnd(spriteTemplate, x, y, UCHAR_MAX);

    if (spriteId == MAX_SPRITES)
        return;

    sprite = &gSprites[spriteId];
    sprite->tMonPosition = position;
    sprite->tBattlerId = battler;
    sprite->tVerticalPosition = y;

#if SWSH_BATTLE_UI
    // gSpriteTemplate_MoveTypes carries no callback of its own, so the slide-in and
    // hide behaviour has to be reattached. Palette follows gTypesInfo, offset to the
    // 13-15 block the sheet was loaded into. No flipping -- the badges carry text.
    sprite->callback = SpriteCB_TypeIcon;
    // gTypesInfo[].palette is already the absolute OBJ palette slot (13, 14 or 15),
    // matching where LoadTypeSpritesAndPalettes puts the three palettes. Adding 13
    // on top pushed it to 26-28 and the badge rendered solid black.
    sprite->oam.paletteNum = gTypesInfo[type].palette;
    sprite->hFlip = FALSE;
#else
    sprite->hFlip = ShouldFlipTypeIcon(useDoubleBattleCoords, position, type);
#endif

    StartSpriteAnim(sprite, type);
}

static bool32 ShouldFlipTypeIcon(bool32 useDoubleBattleCoords, u32 position, enum Type typeId)
{
    enum BattleSide side = (useDoubleBattleCoords) ? B_SIDE_OPPONENT : B_SIDE_PLAYER;

    if (GetBattlerSide(GetBattlerAtPosition(position)) != side)
        return FALSE;

    return !gTypesInfo[typeId].isSpecialCaseType;
}

static void SpriteCB_TypeIcon(struct Sprite *sprite)
{
    u32 position = sprite->tMonPosition;
    enum BattlerId battlerId = sprite->tBattlerId;
    bool32 useDoubleBattleCoords = UseDoubleBattleCoords(GetBattlerAtPosition(position));

    if (sprite->tHideIconTimer == NUM_FRAMES_HIDE_TYPE_ICON)
    {
        DestroyTypeIcon(sprite);
        return;
    }

    if (ShouldHideTypeIcon(battlerId))
    {
        sprite->x += GetTypeIconHideMovement(useDoubleBattleCoords, position);
        ++sprite->tHideIconTimer;
        return;
    }

    sprite->x += GetTypeIconSlideMovement(useDoubleBattleCoords,position, sprite->x);
    sprite->y = GetTypeIconBounceMovement(sprite->tVerticalPosition,position);
}

static const u32 typeIconTags[] =
{
    TYPE_ICON_TAG,
    TYPE_ICON_TAG_2
};

static void DestroyTypeIcon(struct Sprite* sprite)
{
    u32 spriteId, tag;

    DestroySpriteAndFreeResources(sprite);

    for (spriteId = 0; spriteId < MAX_SPRITES; ++spriteId)
    {
        if (!gSprites[spriteId].inUse)
            continue;

        for (tag = 0; tag < 2; tag++)
        {
            if (gSprites[spriteId].template->paletteTag == typeIconTags[tag])
                return;

            if (gSprites[spriteId].template->tileTag == typeIconTags[tag])
                return;
        }
    }

    FreeAllTypeIconResources();
}

static void FreeAllTypeIconResources(void)
{
    u32 tag;

    for (tag = 0; tag < 2; tag++)
    {
        FreeSpriteTilesByTag(typeIconTags[tag]);
        FreeSpritePaletteByTag(typeIconTags[tag]);
    }
}

static void (*const sShowTypesControllerFuncs[])(enum BattlerId battler) =
{
    PlayerHandleChooseMove,
    HandleChooseMoveAfterDma3,
    HandleInputChooseTarget,
    HandleInputShowTargets,
    HandleInputShowEntireFieldTargets,
    HandleMoveSwitching,
    HandleInputChooseMove,
};


static bool32 ShouldHideTypeIcon(enum BattlerId battlerId)
{
    u32 funcIndex;

    for (funcIndex = 0; funcIndex < ARRAY_COUNT(sShowTypesControllerFuncs); funcIndex++)
        if (gBattlerControllerFuncs[battlerId] == sShowTypesControllerFuncs[funcIndex])
            return FALSE;

    return TRUE;
}

static s32 GetTypeIconHideMovement(bool32 useDoubleBattleCoords, u32 position)
{
    if (useDoubleBattleCoords)
    {
        if (position == B_POSITION_PLAYER_LEFT || position == B_POSITION_PLAYER_RIGHT)
            return 1;
        else
            return -1;
    }

    if (position == B_POSITION_PLAYER_LEFT)
        return -1;
    else
        return 1;
}

static s32 GetTypeIconSlideMovement(bool32 useDoubleBattleCoords, u32 position, s32 xPos)
{
    if (useDoubleBattleCoords)
    {
        switch (position)
        {
        case B_POSITION_PLAYER_LEFT:
        case B_POSITION_PLAYER_RIGHT:
            if (xPos > sTypeIconPositions[position][useDoubleBattleCoords].x - 10)
                return -1;
            break;
        default:
        case B_POSITION_OPPONENT_LEFT:
        case B_POSITION_OPPONENT_RIGHT:
            if (xPos < sTypeIconPositions[position][useDoubleBattleCoords].x + 10)
                return 1;
            break;
        }
        return 0;
    }

    if (position == B_POSITION_PLAYER_LEFT)
    {
        if (xPos < sTypeIconPositions[position][useDoubleBattleCoords].x + 10)
            return 1;
    }
    else
    {
        if (xPos > sTypeIconPositions[position][useDoubleBattleCoords].x - 10)
            return -1;
    }
    return 0;
}

static s32 GetTypeIconBounceMovement(s32 originalY, u32 position)
{
    struct Sprite *healthbox = &gSprites[gHealthboxSpriteIds[GetBattlerAtPosition(position)]];
    return originalY + healthbox->y2;
}
