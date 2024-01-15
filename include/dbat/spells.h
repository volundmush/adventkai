/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#include "structs.h"

#define DEFAULT_STAFF_LVL    12
#define DEFAULT_WAND_LVL    12

#define CAST_UNDEFINED    (-1)
#define CAST_SPELL    0
#define CAST_POTION    1
#define CAST_WAND    2
#define CAST_STAFF    3
#define CAST_SCROLL    4
#define CAST_STRIKE    5

#define MAG_DAMAGE        (1 << 0)
#define MAG_AFFECTS        (1 << 1)
#define MAG_UNAFFECTS        (1 << 2)
#define MAG_POINTS        (1 << 3)
#define MAG_ALTER_OBJS        (1 << 4)
#define MAG_GROUPS        (1 << 5)
#define MAG_MASSES        (1 << 6)
#define MAG_AREAS        (1 << 7)
#define MAG_SUMMONS        (1 << 8)
#define MAG_CREATIONS        (1 << 9)
#define MAG_MANUAL        (1 << 10)
#define MAG_AFFECTSV        (1 << 11)
#define MAG_ACTION_FREE        (1 << 12)
#define MAG_ACTION_PARTIAL    (1 << 13)
#define MAG_ACTION_FULL        (1 << 14)
#define MAG_NEXTSTRIKE        (1 << 15)
#define MAG_TOUCH_MELEE        (1 << 16)
#define MAG_TOUCH_RANGED    (1 << 17)

#define MAGSAVE_FORT        (1 << 0)
#define MAGSAVE_REFLEX        (1 << 1)
#define MAGSAVE_WILL        (1 << 2)
#define MAGSAVE_HALF        (1 << 3)
#define MAGSAVE_NONE        (1 << 4)
#define MAGSAVE_PARTIAL        (1 << 5)

#define MAGCOMP_DIVINE_FOCUS    (1 << 0)
#define MAGCOMP_EXP_COST    (1 << 1)
#define MAGCOMP_FOCUS        (1 << 2)
#define MAGCOMP_MATERIAL    (1 << 3)
#define MAGCOMP_SOMATIC        (1 << 4)
#define MAGCOMP_VERBAL        (1 << 5)


#define TYPE_UNDEFINED            (-1)
#define SPELL_RESERVED_DBC        0  /* SKILL NUMBER ZERO -- RESERVED */

/* Taken from the SRD under OGL, see ../doc/srd.txt for information */
#define SPELL_MAGE_ARMOR        1
#define SPELL_TELEPORT            2
#define SPELL_BLESS            3
#define SPELL_BLINDNESS            4
#define SPELL_BURNING_HANDS        5
#define SPELL_CALL_LIGHTNING        6
#define SPELL_CHARM            7
#define SPELL_CHILL_TOUCH        8
#define SPELL_COLOR_SPRAY        10
#define SPELL_CONTROL_WEATHER        11
#define SPELL_CREATE_FOOD        12
#define SPELL_CREATE_WATER        13
#define SPELL_REMOVE_BLINDNESS        14
#define SPELL_CURE_CRITIC        15
#define SPELL_CURE_LIGHT        16
#define SPELL_BANE            17
#define SPELL_DETECT_ALIGN        18
#define SPELL_SEE_INVIS            19
#define SPELL_DETECT_MAGIC        20
#define SPELL_DETECT_POISON        21
#define SPELL_DISPEL_EVIL        22
#define SPELL_EARTHQUAKE        23
#define SPELL_ENCHANT_WEAPON        24
#define SPELL_ENERGY_DRAIN        25
#define SPELL_FIREBALL            26
#define SPELL_HARM            27
#define SPELL_HEAL            28
#define SPELL_INVISIBLE            29
#define SPELL_LIGHTNING_BOLT        30
#define SPELL_LOCATE_OBJECT        31
#define SPELL_MAGIC_MISSILE        32
#define SPELL_POISON            33
#define SPELL_PROT_FROM_EVIL        34
#define SPELL_REMOVE_CURSE        35
#define SPELL_SANCTUARY            36
#define SPELL_SHOCKING_GRASP        37
#define SPELL_SLEEP            38
#define SPELL_BULL_STRENGTH        39
#define SPELL_SUMMON            40
#define SPELL_VENTRILOQUATE        41
#define SPELL_WORD_OF_RECALL        42
#define SPELL_NEUTRALIZE_POISON        43
#define SPELL_SENSE_LIFE        44
#define SPELL_ANIMATE_DEAD        45
#define SPELL_DISPEL_GOOD        46
#define SPELL_GROUP_ARMOR        47
#define SPELL_MASS_HEAL            48
#define SPELL_GROUP_RECALL        49
#define SPELL_DARKVISION        50
#define SPELL_WATERWALK            51
#define SPELL_PORTAL            52
#define SPELL_PARALYZE            53
#define SPELL_INFLICT_LIGHT        54
#define SPELL_INFLICT_CRITIC        55
#define SPELL_IDENTIFY            56
#define SPELL_FAERIE_FIRE        57
#define ABIL_TURNING            58
#define ABIL_LAY_HANDS            59
#define SPELL_RESISTANCE        60
#define SPELL_ACID_SPLASH        61
#define SPELL_DAZE            62
#define SPELL_FLARE            63
#define SPELL_RAY_OF_FROST        64
#define SPELL_DISRUPT_UNDEAD        65
#define SPELL_LESSER_GLOBE_OF_INVUL    66
#define SPELL_STONESKIN            67
#define SPELL_MINOR_CREATION        68
#define SPELL_SUMMON_MONSTER_I        69
#define SPELL_SUMMON_MONSTER_II        70
#define SPELL_SUMMON_MONSTER_III    71
#define SPELL_SUMMON_MONSTER_IV        72
#define SPELL_SUMMON_MONSTER_V        73
#define SPELL_SUMMON_MONSTER_VI        74
#define SPELL_SUMMON_MONSTER_VII    75
#define SPELL_SUMMON_MONSTER_VIII    76
#define SPELL_SUMMON_MONSTER_IX        77
#define SPELL_FIRE_SHIELD        78
#define SPELL_ICE_STORM            79
#define SPELL_SHOUT            80
#define SPELL_FEAR            81
#define SPELL_CLOUDKILL            82
#define SPELL_MAJOR_CREATION        83
#define SPELL_HOLD_MONSTER        84
#define SPELL_CONE_OF_COLD        85
#define SPELL_ANIMAL_GROWTH        86
#define SPELL_BALEFUL_POLYMORPH        87
#define SPELL_PASSWALL            88
#define SPELL_BESTOW_CURSE        89
#define SPELL_SENSU                     90
#define SPELL_HAYASA                    91

#define MIN_LANGUAGES            SKILL_LANG_COMMON
#define SKILL_LANG_COMMON        141
#define SKILL_LANG_ELVEN        142
#define SKILL_LANG_GNOME        143
#define SKILL_LANG_DWARVEN        144
#define SKILL_LANG_HALFLING         145
#define SKILL_LANG_ORC            146
#define SKILL_LANG_DRUIDIC          147
#define SKILL_LANG_DRACONIC         148
#define MAX_LANGUAGES            SKILL_LANG_DRACONIC


#define SKILL_WP_UNARMED        179 /* Barehanded weapon group        */

#define SPELL_FIRE_BREATH        202
#define SPELL_GAS_BREATH        203
#define SPELL_FROST_BREATH        204
#define SPELL_ACID_BREATH        205
#define SPELL_LIGHTNING_BREATH        206

#define MAX_SPELLS            SPELL_SENSU

/*
 * to make an affect induced by dg_affect look correct on 'stat' we need
 * to define it with a 'spellname'.
 */
#define SPELL_DG_AFFECT            298

/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     300
#define TYPE_STING                   301
#define TYPE_WHIP                    302
#define TYPE_SLASH                   303
#define TYPE_BITE                    304
#define TYPE_BLUDGEON                305
#define TYPE_CRUSH                   306
#define TYPE_POUND                   307
#define TYPE_CLAW                    308
#define TYPE_MAUL                    309
#define TYPE_THRASH                  310
#define TYPE_PIERCE                  311
#define TYPE_BLAST             312
#define TYPE_PUNCH             313
#define TYPE_STAB             314
/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING             399

#define SKILL_FLEX            400
#define SKILL_GENIUS            401
#define SKILL_SOLARF            402
#define SKILL_MIGHT            403
#define SKILL_BALANCE            404
#define SKILL_BUILD            405
#define SKILL_TSKIN            406
#define SKILL_CONCENTRATION        407
#define SKILL_KAIOKEN            408
#define SKILL_SPOT            409
#define SKILL_FIRST_AID        410
#define SKILL_DISGUISE            411
#define SKILL_ESCAPE_ARTIST        412
#define SKILL_APPRAISE            413
#define SKILL_HEAL            414
#define SKILL_FORGERY            415
#define SKILL_HIDE            416
#define SKILL_BLESS            417
#define SKILL_CURSE            418
#define SKILL_LISTEN            419
#define SKILL_EAVESDROP            420
#define SKILL_POISON            421
#define SKILL_CURE            422
#define SKILL_OPEN_LOCK            423
#define SKILL_VIGOR            424
#define SKILL_REGENERATE        425
#define SKILL_KEEN            426
#define SKILL_SEARCH            427
#define SKILL_MOVE_SILENTLY        428
#define SKILL_ABSORB            429
#define SKILL_SLEIGHT_OF_HAND        430
#define SKILL_INGEST            431
#define SKILL_REPAIR            432
#define SKILL_SENSE                433
#define SKILL_SURVIVAL            434
#define SKILL_YOIK                435
#define SKILL_CREATE            436
#define SKILL_SPIT                437
#define SKILL_POTENTIAL        438
#define SKILL_TELEPATHY        439
#define SKILL_RENZO            440
#define SKILL_MASENKO        441
#define SKILL_DODONPA            442
#define SKILL_BARRIER        443
#define SKILL_GALIKGUN        444
#define SKILL_THROW                     445
#define SKILL_DODGE                     446
#define SKILL_PARRY                     447
#define SKILL_BLOCK                     448
#define SKILL_PUNCH                     449
#define SKILL_KICK                      450
#define SKILL_ELBOW                     451
#define SKILL_KNEE                      452
#define SKILL_ROUNDHOUSE                453
#define SKILL_UPPERCUT                  454
#define SKILL_SLAM                      455
#define SKILL_HEELDROP                  456
#define SKILL_FOCUS                     457
#define SKILL_KIBALL                    458
#define SKILL_KIBLAST                   459
#define SKILL_BEAM                      460
#define SKILL_TSUIHIDAN                 461
#define SKILL_SHOGEKIHA                 462
#define SKILL_ZANZOKEN                  463
#define SKILL_KAMEHAMEHA                464
#define SKILL_DAGGER                    465
#define SKILL_SWORD                     466
#define SKILL_CLUB                      467
#define SKILL_SPEAR                     468
#define SKILL_GUN                       469
#define SKILL_BRAWL                     470
#define SKILL_INSTANTT                  471
#define SKILL_DEATHBEAM                 472
#define SKILL_ERASER                    473
#define SKILL_TSLASH                    474
#define SKILL_PSYBLAST                  475
#define SKILL_HONOO                     476
#define SKILL_DUALBEAM                  477
#define SKILL_ROGAFUFUKEN               478
#define SKILL_POSE                      479
#define SKILL_BAKUHATSUHA               480
#define SKILL_KIENZAN                   481
#define SKILL_TRIBEAM                   482
#define SKILL_SBC                       483
#define SKILL_FINALFLASH                484
#define SKILL_CRUSHER                   485
#define SKILL_DDSLASH                   486
#define SKILL_PBARRAGE                  487
#define SKILL_HELLFLASH                 488
#define SKILL_HELLSPEAR                 489
#define SKILL_KAKUSANHA                 490
#define SKILL_HASSHUKEN                 491
#define SKILL_SCATTER                   492
#define SKILL_BIGBANG                   493
#define SKILL_PSLASH                    494
#define SKILL_DEATHBALL                 495
#define SKILL_SPIRITBALL                496
#define SKILL_GENKIDAMA                 497
#define SKILL_GENOCIDE                  498
#define SKILL_DUALWIELD                 499
#define SKILL_KURA                      500
#define SKILL_TAILWHIP                  501
#define SKILL_KOUSENGAN                 502
#define SKILL_TAISHA                    503
#define SKILL_PARALYZE                  505
#define SKILL_INFUSE                    506
#define SKILL_ROLL                      507
#define SKILL_TRIP                      508
#define SKILL_GRAPPLE                   509
#define SKILL_WSPIKE                    510
#define SKILL_SELFD                     511
#define SKILL_SPIRAL                    512
#define SKILL_BREAKER                   513
#define SKILL_ENLIGHTEN                 514
#define SKILL_COMMUNE                   515
#define SKILL_MIMIC                     516
#define SKILL_WRAZOR                    517
#define SKILL_KOTEIRU                   518
#define SKILL_DIMIZU                    519
#define SKILL_HYOGA_KABE                520
#define SKILL_WELLSPRING                521
#define SKILL_AQUA_BARRIER              522
#define SKILL_WARP                      523
#define SKILL_HSPIRAL                   524
#define SKILL_ARMOR                     525
#define SKILL_FIRESHIELD                526
#define SKILL_COOKING                   527
#define SKILL_SEISHOU                   528
#define SKILL_SILK                      529
#define SKILL_BASH                      530
#define SKILL_HEADBUTT                  531
#define SKILL_ENSNARE                   532
#define SKILL_STARNOVA                  533
#define SKILL_PURSUIT                   534
#define SKILL_ZEN                       535
#define SKILL_SUNDER                    536
#define SKILL_WITHER                    537
#define SKILL_TWOHAND                   538
#define SKILL_STYLE                     539
#define SKILL_METAMORPH                 540
#define SKILL_HEALGLOW                  541
#define SKILL_RUNIC                     542
#define SKILL_EXTRACT                   543
#define SKILL_GARDENING                 544
#define SKILL_ENERGIZE                  545
#define SKILL_MALICE                    549
#define SKILL_HAYASA                    550
#define SKILL_HANDLING                  551
#define SKILL_MYSTICMUSIC               552
#define SKILL_LIGHTGRENADE              553
#define SKILL_MULTIFORM                 554
#define SKILL_SPIRITCONTROL             555
#define SKILL_BALEFIRE                  556
#define SKILL_BLESSEDHAMMER             557

#define ART_STUNNING_FIST        1000
#define ART_WHOLENESS_OF_BODY        1001
#define ART_ABUNDANT_STEP        1002
#define ART_QUIVERING_PALM        1003
#define ART_EMPTY_BODY            1004

#define SAVING_FORTITUDE    0
#define SAVING_REFLEX        1
#define SAVING_WILL        2

#define SAVING_OBJ_IMPACT       0
#define SAVING_OBJ_HEAT         1
#define SAVING_OBJ_COLD         2
#define SAVING_OBJ_BREATH       3
#define SAVING_OBJ_SPELL        4

#define TAR_IGNORE      (1 << 0)
#define TAR_CHAR_ROOM   (1 << 1)
#define TAR_CHAR_WORLD  (1 << 2)
#define TAR_FIGHT_SELF  (1 << 3)
#define TAR_FIGHT_VICT  (1 << 4)
#define TAR_SELF_ONLY   (1 << 5) /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF    (1 << 6) /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     (1 << 7)
#define TAR_OBJ_ROOM    (1 << 8)
#define TAR_OBJ_WORLD   (1 << 9)
#define TAR_OBJ_EQUIP    (1 << 10)

#define SKTYPE_NONE        0
#define SKTYPE_SPELL        (1 << 0)
#define SKTYPE_SKILL        (1 << 1)
#define SKTYPE_LANG        (1 << 2)
#define SKTYPE_WEAPON        (1 << 3)
#define SKTYPE_ART        (1 << 4)

#define SKFLAG_NEEDTRAIN    (1 << 0) /* Disallow use of 0 skill with only stat mod */
#define SKFLAG_STRMOD        (1 << 1)
#define SKFLAG_DEXMOD        (1 << 2)
#define SKFLAG_CONMOD        (1 << 3)
#define SKFLAG_INTMOD        (1 << 4)
#define SKFLAG_WISMOD        (1 << 5)
#define SKFLAG_CHAMOD        (1 << 6)
#define SKFLAG_ARMORBAD        (1 << 7)
#define SKFLAG_ARMORALL        (1 << 8)
#define SKFLAG_TIER1            (1 << 9)
#define SKFLAG_TIER2            (1 << 10)
#define SKFLAG_TIER3            (1 << 11)
#define SKFLAG_TIER4            (1 << 12)
#define SKFLAG_TIER5            (1 << 13)

#define SKLEARN_CANT        0 /* This class can't learn this skill */
#define SKLEARN_CROSSCLASS    1 /* Cross-class skill for this class */
#define SKLEARN_CLASS        2 /* Class skill for this class */
#define SKLEARN_BOOL        3 /* Skill is known or not */

struct spell_info_type {
    int8_t min_position;    /* Position for caster	 */
    int mana_min;        /* Min amount of mana used by a spell (highest lev) */
    int mana_max;    /* Max amount of mana used by a spell (lowest lev) */
    int mana_change;     /* Change in mana used by spell from lev to lev */
    int ki_min;        /* Min amount of mana used by a spell (highest lev) */
    int ki_max;        /* Max amount of mana used by a spell (lowest lev) */
    int ki_change;    /* Change in mana used by spell from lev to lev */

    int min_level[NUM_CLASSES];
    int routines;
    int8_t violent;
    int targets;         /* See below for use with TAR_XXX  */
    const char *name;    /* Input size not limited. Originates from string constants. */
    const char *wear_off_msg;    /* Input size not limited. Originates from string constants. */
    int race_can_learn[NUM_RACES];
    int skilltype;       /* Is it a spell, skill, art, feat, or what? used as bitvector */
    int flags;
    int save_flags;
    int comp_flags;
    int8_t can_learn_skill[NUM_CLASSES];
    int spell_level;
    int school;
    int domain;
};

extern struct spell_info_type spell_info[SKILL_TABLE_SIZE];

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
    const char *singular;
    const char *plural;
};


#define ASPELL(spellname) \
extern void spellname(int level, struct char_data *ch, \
          struct char_data *victim, struct obj_data *obj, const char *arg)

#define MANUAL_SPELL(spellname)    spellname(level, caster, cvict, ovict, arg);

ASPELL(spell_create_water);

ASPELL(spell_recall);

ASPELL(spell_teleport);

ASPELL(spell_summon);

ASPELL(spell_locate_object);

ASPELL(spell_charm);

ASPELL(spell_information);

ASPELL(spell_identify);

ASPELL(spell_enchant_weapon);

ASPELL(spell_detect_poison);

ASPELL(spell_portal);

ASPELL(art_abundant_step);

/* basic magic calling functions */

extern int find_skill_num(char *name, int sktype);

extern int mag_damage(int level, struct char_data *ch, struct char_data *victim,
                      int spellnum);

extern void mag_affects(int level, struct char_data *ch, struct char_data *victim,
                        int spellnum);

extern void mag_groups(int level, struct char_data *ch, int spellnum);

extern void mag_masses(int level, struct char_data *ch, int spellnum);

extern void mag_areas(int level, struct char_data *ch, int spellnum);

extern void mag_summons(int level, struct char_data *ch, struct obj_data *obj, int spellnum, char *arg);

extern void mag_points(int level, struct char_data *ch, struct char_data *victim,
                       int spellnum);

extern void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
                          int spellnum);

extern void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
                           int spellnum);

extern void mag_creations(int level, struct char_data *ch, int spellnum);

extern void mag_affectsv(int level, struct char_data *ch, struct char_data *victim,
                         int spellnum);

extern int call_magic(struct char_data *caster, struct char_data *cvict,
                      struct obj_data *ovict, int spellnum, int level, int casttype, char *arg);

extern void mag_objectmagic(struct char_data *ch, struct obj_data *obj,
                            char *argument);

extern int cast_spell(struct char_data *ch, struct char_data *tch,
                      struct obj_data *tobj, int spellnum, char *arg);

extern int mag_newsaves(struct char_data *ch, struct char_data *victim, int spellnum,
                        int level, int cast_stat);


/* other prototypes */
extern void skill_level(int spell, int chclass, int level);

extern void skill_race_class(int spell, int race, int learntype);

extern void skill_class(int skill, int chclass, int learntype);

const char *skill_name(int num);

extern int roll_skill(struct char_data *ch, int snum);

extern int roll_resisted(struct char_data *actor, int sact, struct char_data *resistor, int sres);

extern int skill_type(int skill);
