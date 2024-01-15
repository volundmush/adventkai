/***********************************************************************
** FEATS.H                                                            **
** Header file for the Gates of Krynn Feat System.                    **
** Initial code by Paladine (Stephen Squires)                         **
** Created Thursday, September 5, 2002                                **
**                                                                    **
***********************************************************************/
#pragma once

#include "structs.h"

/* Functions defined in feats.c */
extern int is_proficient_with_armor(const struct char_data *ch, int armor_type);

extern int is_proficient_with_weapon(const struct char_data *ch, int weapon_type);

extern int find_feat_num(char *name);

extern int feat_to_subfeat(int feat);

extern void assign_feats();

extern void sort_feats();

extern int feat_is_available(struct char_data *ch, int featnum, int iarg, char *sarg);

extern int feat_sort_info[MAX_FEATS + 1];

/* Feats defined below up to MAX_FEATS */

#define FEAT_UNDEFINED                0
#define FEAT_ALERTNESS                1
#define FEAT_ARMOR_PROFICIENCY_HEAVY        3 /* Not used */
#define FEAT_ARMOR_PROFICIENCY_LIGHT        4 /* Not used */
#define FEAT_ARMOR_PROFICIENCY_MEDIUM        5 /* Not used */
#define FEAT_BLIND_FIGHT            6
#define FEAT_BREW_POTION            7 /* Not used */
#define FEAT_CLEAVE                8 /* Not used */
#define FEAT_COMBAT_CASTING            9 /* Not used */
#define FEAT_COMBAT_REFLEXES            10
#define FEAT_CRAFT_MAGICAL_ARMS_AND_ARMOR    11 /* Not used */
#define FEAT_CRAFT_ROD                12 /* Not used */
#define FEAT_CRAFT_STAFF            13 /* Not used */
#define FEAT_CRAFT_WAND                14 /* Not used */
#define FEAT_CRAFT_WONDEROUS_ITEM        15 /* Not used */
#define FEAT_DEFLECT_ARROWS            16 /* Not used */
#define FEAT_DODGE                17
#define FEAT_EMPOWER_SPELL            18 /* Not used */
#define FEAT_ENDURANCE                19
#define FEAT_ENLARGE_SPELL            20 /* Not used */
#define FEAT_WEAPON_PROFICIENCY_BASTARD_SWORD    21 /* Not used */
#define FEAT_EXPERTISE                22
#define FEAT_EXTEND_SPELL            23 /* Not used */
#define FEAT_EXTRA_TURNING            24 /* Not used */
#define FEAT_FAR_SHOT                25 /* Not used */
#define FEAT_FORGE_RING                26 /* Not used */
#define FEAT_GREAT_CLEAVE            27 /* Not used */
#define FEAT_GREAT_FORTITUDE            28
#define FEAT_HEIGHTEN_SPELL            29 /* Not used */
#define FEAT_IMPROVED_BULL_RUSH            30 /* Not used */
#define FEAT_IMPROVED_CRITICAL            31
/* lots of free slots here */
#define FEAT_IMPROVED_DISARM            61 /* Not used */
#define FEAT_IMPROVED_INITIATIVE        62
#define FEAT_IMPROVED_TRIP            63
#define FEAT_IMPROVED_TWO_WEAPON_FIGHTING    64
#define FEAT_IMPROVED_UNARMED_STRIKE        65 /* Not used */
#define FEAT_IRON_WILL                66
#define FEAT_LEADERSHIP                67
#define FEAT_LIGHTNING_REFLEXES            68
#define FEAT_MARTIAL_WEAPON_PROFICIENCY        69 /* Not used */
#define FEAT_MAXIMIZE_SPELL            70 /* Not used */
#define FEAT_MOBILITY                71
#define FEAT_MOUNTED_ARCHERY            72 /* Not used */
#define FEAT_MOUNTED_COMBAT            73 /* Not used */
#define FEAT_POINT_BLANK_SHOT            74 /* Not used */
#define FEAT_POWER_ATTACK            75
#define FEAT_PRECISE_SHOT            76 /* Not used */
#define FEAT_QUICK_DRAW                77 /* Not used */
#define FEAT_QUICKEN_SPELL            78 /* Not used */
#define FEAT_RAPID_SHOT                79 /* Not used */
#define FEAT_RIDE_BY_ATTACK            80 /* Not used */
#define FEAT_RUN                81 /* Not used */
#define FEAT_SCRIBE_SCROLL            82 /* Not used */
#define FEAT_SHIELD_PROFICIENCY            83 /* Not used */
#define FEAT_SHOT_ON_THE_RUN            84 /* Not used */
#define FEAT_SILENT_SPELL            85 /* Not used */
#define FEAT_SIMPLE_WEAPON_PROFICIENCY        86 /* Not used */
#define FEAT_SKILL_FOCUS            87
#define FEAT_SPELL_FOCUS            88 /* Not used */
#define FEAT_SPELL_MASTERY            96 /* Not used */
#define FEAT_SPELL_PENETRATION            97 /* Not used */
#define FEAT_SPIRITED_CHARGE            98 /* Not used */
#define FEAT_SPRING_ATTACK            99
#define FEAT_STILL_SPELL            100 /* Not used */
#define FEAT_STUNNING_FIST            101 /* Not used */
#define FEAT_SUNDER                102 /* Not used */
#define FEAT_TOUGHNESS                103
#define FEAT_TRACK                104
#define FEAT_TRAMPLE                105 /* Not used */
#define FEAT_TWO_WEAPON_FIGHTING        106
#define FEAT_WEAPON_FINESSE            107
/* lots of free slots here */
#define FEAT_WEAPON_FOCUS            137 /* Not used */
/* lots of free slots here */
#define FEAT_WEAPON_SPECIALIZATION        167 /* Not used */
/* lots of free slots here */
#define FEAT_WHIRLWIND_ATTACK            197 /* Not used */
#define FEAT_WEAPON_PROFICIENCY_DRUID        198 /* Not used */
#define FEAT_WEAPON_PROFICIENCY_ROGUE        199 /* Not used */
#define FEAT_WEAPON_PROFICIENCY_MONK        200 /* Not used */
#define FEAT_WEAPON_PROFICIENCY_WIZARD        201 /* Not used */
#define FEAT_WEAPON_PROFICIENCY_ELF        202 /* Not used */
#define FEAT_ARMOR_PROFICIENCY_SHIELD        203 /* Not used */
#define FEAT_SNEAK_ATTACK            204
/* Evasion and improved evasion are not actually feats, but we treat them like feats
 * just to make it easier */
#define FEAT_EVASION                205
#define FEAT_IMPROVED_EVASION            206
#define FEAT_ACROBATIC                207
#define FEAT_AGILE                208
#define FEAT_ANIMAL_AFFINITY            209 /* Not used */
#define FEAT_ATHLETIC                210 /* Not used */
#define FEAT_AUGMENT_SUMMONING            211 /* Not used */
#define FEAT_COMBAT_EXPERTISE            212 /* Not used */
#define FEAT_DECEITFUL                213
#define FEAT_DEFT_HANDS                214 /* Not used */
#define FEAT_DIEHARD                215
#define FEAT_DILIGENT                216
#define FEAT_ESCHEW_MATERIALS            217 /* Not used */
#define FEAT_EXOTIC_WEAPON_PROFICIENCY        218 /* Not used */
#define FEAT_GREATER_SPELL_FOCUS        219 /* Not used */
#define FEAT_GREATER_SPELL_PENETRATION        220 /* Not used */
#define FEAT_GREATER_TWO_WEAPON_FIGHTING    221
#define FEAT_GREATER_WEAPON_FOCUS        222 /* Not used */
#define FEAT_GREATER_WEAPON_SPECIALIZATION    223 /* Not used */
#define FEAT_IMPROVED_COUNTERSPELL        224 /* Not used */
#define FEAT_IMPROVED_FAMILIAR            225 /* Not used */
#define FEAT_IMPROVED_FEINT            226
#define FEAT_IMPROVED_GRAPPLE            227
#define FEAT_IMPROVED_OVERRUN            228 /* Not used */
#define FEAT_IMPROVED_PRECISE_SHOT        229 /* Not used */
#define FEAT_IMPROVED_SHIELD_BASH        230 /* Not used */
#define FEAT_IMPROVED_SUNDER            231 /* Not used */
#define FEAT_IMPROVED_TURNING            232 /* Not used */
#define FEAT_INVESTIGATOR            233 /* Not used */
#define FEAT_MAGICAL_APTITUDE            234 /* Not used */
#define FEAT_MANYSHOT                235 /* Not used */
#define FEAT_NATURAL_SPELL            236 /* Not used */
#define FEAT_NEGOTIATOR                237 /* Not used */
#define FEAT_NIMBLE_FINGERS            238 /* Not used */
#define FEAT_PERSUASIVE                239 /* Not used */
#define FEAT_RAPID_RELOAD            240 /* Not used */
#define FEAT_SELF_SUFFICIENT            241 /* Not used */
#define FEAT_STEALTHY                242
#define FEAT_ARMOR_PROFICIENCY_TOWER_SHIELD    243 /* Not used */
#define FEAT_TWO_WEAPON_DEFENSE            244 /* Not used */
#define FEAT_WIDEN_SPELL            245 /* Not used */
/* These are not actually feats, they are really class special abilities */
#define FEAT_CRIPPLING_STRIKE            246
#define FEAT_DEFENSIVE_ROLL            247 /* Not used */
#define FEAT_OPPORTUNIST            248
#define FEAT_SKILL_MASTERY            249 /* Not used */
#define FEAT_SLIPPERY_MIND            250 /* Not used */
#define FEAT_KI_STRIKE                251 /* Not used */
/* End of non-feat block, more feats below */
#define FEAT_SNATCH_ARROWS            252 /* Not used */
#define FEAT_SENSE                              253


/* Below are the various weapon type defines */

#define    WEAPON_TYPE_UNDEFINED        0
#define WEAPON_TYPE_UNARMED        1
#define WEAPON_TYPE_DAGGER        2
#define WEAPON_TYPE_MACE        3
#define WEAPON_TYPE_SICKLE        4
#define WEAPON_TYPE_SPEAR        5
#define WEAPON_TYPE_STAFF        6
#define WEAPON_TYPE_CROSSBOW        7
#define WEAPON_TYPE_LONGBOW        8
#define WEAPON_TYPE_SHORTBOW        9
#define WEAPON_TYPE_SLING        10
#define WEAPON_TYPE_THROWN        11
#define WEAPON_TYPE_HAMMER        12
#define WEAPON_TYPE_LANCE        13
#define WEAPON_TYPE_FLAIL        14
#define WEAPON_TYPE_LONGSWORD        15
#define WEAPON_TYPE_SHORTSWORD        16
#define WEAPON_TYPE_GREATSWORD        17
#define WEAPON_TYPE_RAPIER        18
#define WEAPON_TYPE_SCIMITAR        19
#define WEAPON_TYPE_POLEARM        20
#define WEAPON_TYPE_CLUB        21
#define WEAPON_TYPE_BASTARD_SWORD    22
#define WEAPON_TYPE_MONK_WEAPON        23
#define WEAPON_TYPE_DOUBLE_WEAPON    24
#define WEAPON_TYPE_AXE            25
#define WEAPON_TYPE_WHIP        26

#define MAX_WEAPON_TYPES        26

/* Below are the various armor type defines */

#define    ARMOR_TYPE_UNDEFINED        0
#define    ARMOR_TYPE_LIGHT        1
#define    ARMOR_TYPE_MEDIUM        2
#define    ARMOR_TYPE_HEAVY        3
#define    ARMOR_TYPE_SHIELD        4

#define MAX_ARMOR_TYPES            5

/* Below is the structure for a feat */

struct feat_info {

    char *name;        /* The name of the feat to be displayed to players */
    int in_game;        /* TRUE or FALSE, is the feat in the game yet? */
    int can_learn;    /* TRUE or FALSE, can the feat be learned or is it an automatic feat? */
    int can_stack;    /* TRUE or FALSE, can the feat be learned more than once? */

};

extern struct feat_info feat_list[NUM_FEATS_DEFINED + 1];

#define HRANK_ANY       0
#define HRANK_ARCANE    1
#define HRANK_DIVINE    2
#define HRANK_CASTER    3

#define BASE_DC 10
