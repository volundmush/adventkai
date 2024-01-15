/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */

#include "dbat/class.h"

#include <utility>
#include "dbat/db.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/spells.h"
#include "dbat/commands.h"
#include "dbat/constants.h"
#include "dbat/handler.h"
#include "dbat/feats.h"
#include "dbat/oasis.h"
#include "dbat/act.wizard.h"
#include "dbat/dg_comm.h"
#include "dbat/act.other.h"

/* Names first */

const char *class_abbrevs[NUM_CLASSES + 1] = {
        "Ro",
        "Pi",
        "Kr",
        "Na",
        "Ba",
        "Gi",
        "Fr",
        "Ta",
        "An",
        "Da",
        "Ki",
        "Ji",
        "Ts",
        "Ku",
        "As",
        "Bl",
        "Dd",
        "Du",
        "Dw",
        "Ek",
        "Ht",
        "Hw",
        "Lo",
        "Mt",
        "Sh",
        "Th",
        "Ex",
        "Ad",
        "Co",
        "Ar",
        "Wa",
        "\n"
};

/* Copied from the SRD under OGL, see ../doc/srd.txt for information */
const char *class_names[NUM_CLASSES + 1] = {
        "roshi",
        "piccolo",
        "Krane",
        "nail",
        "bardock",
        "ginyu",
        "frieza",
        "Tapion",
        "android 16",
        "Dabura",
        "kibito",
        "Jinto",
        "tsuna",
        "kurzak",
        "assassin",
        "blackguard",
        "dragon disciple",
        "duelist",
        "dwarven defender",
        "eldritch knight",
        "hierophant",
        "horizon walker",
        "loremaster",
        "mystic theurge",
        "shadowdancer",
        "thaumaturgist",
        "artisan",
        "magi",
        "normal",
        "noble",
        "soldier",
        "\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *class_display[NUM_CLASSES] = {
        "@B1@W) @MRoshi\r\n",
        "@B2@W) @WPiccolo\r\n",
        "@B3@W) @YKrane\r\n",
        "@B4@W) @BNail\r\n",
        "@B5@W) @BBardock\r\n",
        "@B6@W) @BGinyu\r\n",
        "@B7@W) @WFrieza\r\n",
        "@B8@W) @YTapion\r\n",
        "@B9@W) @BAndroid 16\r\n",
        "@B10@W) @BDabura\r\n",
        "@B11@W) @BKibito\r\n",
        "@B12@W) @BJinto\r\n",
        "@B13@W) @BTsuna\r\n",
        "@B14@W) @BKurzak\r\n",
        "assassin (P)\r\n",
        "blackguard (P)\r\n",
        "dragon disciple (P)\r\n",
        "duelist (P)\r\n",
        "dwarven defender (P)\r\n",
        "eldritch knight (P)\r\n",
        "hierophant (P)\r\n",
        "horizon walker (P)\r\n",
        "loremaster (P)\r\n",
        "mystic theurge (P)\r\n",
        "shadowdancer (P)\r\n",
        "thaumaturgist (P)\r\n",
        "Artisan NPC\r\n",
        "Magi NPC\r\n",
        "Normal NPC\r\n",
        "Noble NPC\r\n",
        "Soldier NPC\r\n",
};

/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only WIZARDS are allowed
 * to go south.
 *
 * Don't forget to visit spec_assign.c if you create any new mobiles that
 * should be a guild master or guard so they can act appropriately. If you
 * "recycle" the existing mobs that are used in other guilds for your new
 * guild, then you don't have to change that file, only here.
 */
const struct guild_info_type guild_info[6] = {

/* Kortaal */
        {(int)CLASS_ROSHI,     3017, SCMD_EAST},
        {(int)CLASS_PICCOLO,   3004, SCMD_NORTH},
        {(int)CLASS_KRANE,     3027, SCMD_EAST},
        {(int)CLASS_NAIL,      3021, SCMD_EAST},

/* Brass Dragon */
        {-999 /* all */ , 5065, SCMD_WEST},

/* this must go last -- add new guards above! */
        {-1, NOWHERE, -1}
};
/* 
 * These tables hold the various level configuration setting;
 * experience points, base hit values, character saving throws.
 * They are read from a configuration file (normally etc/levels)
 * as part of the boot process.  The function load_levels() at
 * the end of this file reads in the actual values.
 */

const char *config_sect[NUM_CONFIG_SECTIONS + 1] = {
        "version",
        "experience",
        "vernum",
        "fortitude",
        "reflex",
        "will",
        "basehit",
        "\n"
};

#define CONFIG_LEVEL_VERSION    0
#define CONFIG_LEVEL_EXPERIENCE    1
#define CONFIG_LEVEL_VERNUM    2
#define CONFIG_LEVEL_FORTITUDE    3
#define CONFIG_LEVEL_REFLEX    4
#define CONFIG_LEVEL_WILL    5
#define CONFIG_LEVEL_BASEHIT    6

static char level_version[READ_SIZE];


#define SAVE_MANUAL 0
#define SAVE_LOW 1
#define SAVE_HIGH 2

static const char *save_type_names[] = {
        "manual",
        "low",
        "high"
};

#define BASEHIT_MANUAL 0
#define BASEHIT_LOW     1
#define BASEHIT_MEDIUM  2
#define BASEHIT_HIGH    3

static const char *basehit_type_names[] = {
        "manual",
        "low",
        "medium",
        "high"
};

/* Class Template Attribute values were created so all default PC classes would
   total 80 before racial modifiers. Non defaults add up to 60. */
static const int class_template[NUM_BASIC_CLASSES][6] = {
/* 		      S,  D,  C,  I,  W,  C */
/* Wizard 	*/ {10, 13, 13, 18, 16, 10},
/* Cleric 	*/
                 {13, 10, 13, 10, 18, 16},
/* Rogue 	*/
                 {13, 18, 13, 16, 10, 10},
/* Fighter 	*/
                 {18, 13, 16, 10, 13, 10},
/* Monk 	*/
                 {13, 16, 13, 10, 18, 10},
/* Paladin 	*/
                 {18, 10, 13, 10, 16, 13},
/* Sorcerer 	*/
                 {10, 13, 13, 18, 16, 10},
/* Druid 	*/
                 {13, 10, 13, 10, 18, 16},
/* Bard 	*/
                 {13, 18, 13, 16, 10, 10},
/* Ranger 	*/
                 {13, 13, 13, 10, 16, 10},
/* Barbarian 	*/
                 {13, 12, 18, 10, 12, 10}
};

/* Race Template Attribute values were created so all default PC races would
   total 80 before racial modifiers. Non defaults add up to 60. */
static const int race_template[NUM_RACES][6] = {
/* 		      S,  D,  C,  I,  W,  C */
/* Human 	*/ {13, 13, 13, 13, 13, 13},
/* Saiyan  	*/
                {16, 12, 14, 10, 12, 12},
/* Icer 	*/
                {14, 14, 12, 12, 12, 12},
/* Konatsu 	*/
                {10, 16, 10, 13, 14, 14},
/* Namek 	*/
                {14, 12, 13, 12, 14, 12},
/* Mutant 	*/
                {12, 12, 15, 13, 13, 13},
/* Kanassan 	*/
                {10, 14, 10, 15, 13, 10},
/* Halfbreed 	*/
                {14, 13, 14, 12, 13, 12},
/* Bio  	*/
                {15, 10, 15, 12, 12, 10},
/* Android 	*/
                {14, 14, 14, 12, 10, 12},
/* Demon 	*/
                {14, 13, 14, 10, 12, 10},
/* Majin 	*/
                {15, 10, 15, 10, 12, 14},
/* Kai  	*/
                {11, 14, 10, 14, 14, 11},
/* Truffle 	*/
                {10, 14, 10, 16, 16, 12},
/* Goblin 	*/
                {13, 13, 13, 13, 13, 13},
/* Insect 	*/
                {10, 10, 10, 10, 10, 10},
/* Orc  	*/
                {10, 10, 10, 10, 10, 10},
/* Snake  	*/
                {10, 10, 10, 10, 10, 10},
/* Troll  	*/
                {10, 10, 10, 10, 10, 10},
/* Minotaur  	*/
                {10, 10, 10, 10, 10, 10},
/* Arlian  	*/
                {13, 13, 13, 12, 12, 14},
/* Lizardfolk  	*/
                {10, 10, 10, 10, 10, 10},
/* Warhost  	*/
                {10, 10, 10, 10, 10, 10},
/* Faerie 	*/
                {10, 10, 10, 10, 10, 10}
};

void cedit_creation(struct char_data *ch) {
    switch (CONFIG_CREATION_METHOD) {
        case CEDIT_CREATION_METHOD_3: /* Points Pool */
            break;
        case CEDIT_CREATION_METHOD_4: /* Racial based template */
            break;
        case CEDIT_CREATION_METHOD_5: /* Class based template */
            break;
        case CEDIT_CREATION_METHOD_2: /* Random rolls, player can adjust */
        case CEDIT_CREATION_METHOD_1: /* Standard random roll, system assigned */
        default:
            break;
    }
    racial_body_parts(ch);
}

/*
 * Roll the 6 stats for a character... each stat is made of some combination
 * of 6-sided dice with various rolls discarded.  Each class then decides
 * which priority will be given for the best to worst stats.
 */
/* This character creation method is original */

/* Taken from the SRD under OGL, see ../doc/srd.txt for information */
const int class_hit_die_size[NUM_CLASSES] = {
/* Wi */ 4,
/* Cl */ 8,
/* Ro */ 6,
/* Fi */ 10,
/* Mo */ 8,
/* Pa */ 10,
/* So */ 4,
/* Dr */ 8,
/* Ba */ 6,
/* Ra */ 8,
/* Ba */ 12,
/* AA */ 8,
/* AT */ 4,
/* AM */ 4,
/* AS */ 6,
/* BG */ 10,
/* DD */ 12,
/* Du */ 10,
/* Dw */ 12,
/* EK */ 6,
/* HE */ 8,
/* HW */ 8,
/* LM */ 4,
/* MT */ 4,
/* SD */ 8,
/* TH */ 4,
/* Ex */ 6,
/* Ad */ 6,
/* Co */ 4,
/* Ar */ 8,
/* Wa */ 8
};

/* Some initializations for characters, including initial skills */
void do_start(struct char_data *ch) {
    int punch;
    struct obj_data *obj;

    ch->set(CharNum::Level, 1);
    ch->setExperience(1);

    if (IS_ANDROID(ch)) {
        GET_COND(ch, HUNGER) = -1;
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, DRUNK) = -1;
    } else {
        GET_COND(ch, THIRST) = 48;
        GET_COND(ch, HUNGER) = 48;
        GET_COND(ch, DRUNK) = 0;
    }

    for(auto f : {PRF_VIEWORDER, PRF_DISPMOVE, PRF_AUTOEXIT, PRF_HINTS, PRF_NOMUSIC, PRF_DISPHP,
    PRF_DISPKI, PRF_DISPEXP, PRF_DISPTNL}) ch->pref.set(f);

    GET_LIMBCOND(ch, 0) = 100;
    GET_LIMBCOND(ch, 1) = 100;
    GET_LIMBCOND(ch, 2) = 100;
    GET_LIMBCOND(ch, 3) = 100;
    ch->playerFlags.set(PLR_HEAD);

    GET_SLOTS(ch) = 30;

    if (GET_RACE(ch) == RACE_HUMAN) {
        GET_SLOTS(ch) += 1;
    } else if (GET_RACE(ch) == RACE_SAIYAN) {
        GET_SLOTS(ch) -= 1;
    } else if (GET_RACE(ch) == RACE_TRUFFLE) {
        GET_SLOTS(ch) += 2;
    } else if (GET_RACE(ch) == RACE_HALFBREED) {
        GET_SLOTS(ch) += 1;
    } else if (GET_RACE(ch) == RACE_MAJIN) {
        GET_SLOTS(ch) -= 1;
    } else if (GET_RACE(ch) == RACE_KAI) {
        GET_SLOTS(ch) += 4;
    }


    if (IS_TSUNA(ch) || IS_KABITO(ch) || IS_NAIL(ch))
        GET_SLOTS(ch) += 5;

    if (GET_BONUS(ch, BONUS_GMEMORY))
        GET_SLOTS(ch) += 2;
    if (GET_BONUS(ch, BONUS_BMEMORY))
        GET_SLOTS(ch) -= 5;

    ch->gainTail(false);

    if (GET_RACE(ch) == RACE_MAJIN) {
        GET_ABSORBS(ch) = 0;
        GET_INGESTLEARNED(ch) = 0;

    }
    if (GET_RACE(ch) == RACE_BIO) {
        GET_ABSORBS(ch) = 3;
    }


    if (!PLR_FLAGGED(ch, PLR_FORGET)) {
        if (ch->choice == 1) {
            punch = rand_number(30, 40);
            SET_SKILL(ch, SKILL_BLOCK, punch);
        }
        if (ch->choice == 2) {
            punch = rand_number(10, 20);
            SET_SKILL(ch, SKILL_PUNCH, punch);
        }
        if (ch->choice == 3) {
            punch = rand_number(30, 40);
            SET_SKILL(ch, SKILL_KICK, punch);
        }
        if (ch->choice == 4) {
            punch = rand_number(20, 30);
            SET_SKILL(ch, SKILL_SLAM, punch);
        }
        if (ch->choice == 5) {
            punch = rand_number(20, 30);
            SET_SKILL(ch, SKILL_FOCUS, punch);
        }
        if (IS_HUMAN(ch)) {
            punch = rand_number(5, 15);
            SET_SKILL(ch, SKILL_BUILD, punch);
        }
        if (IS_TRUFFLE(ch)) {
            punch = rand_number(15, 25);
            SET_SKILL(ch, SKILL_BUILD, punch);
        }
        if (IS_KONATSU(ch)) {
            punch = rand_number(50, 60);
            SET_SKILL(ch, SKILL_SWORD, punch);
            punch = rand_number(10, 30);
            SET_SKILL(ch, SKILL_MOVE_SILENTLY, punch);
            punch = rand_number(10, 30);
            SET_SKILL(ch, SKILL_HIDE, punch);
        }
        if (IS_TAPION(ch) || IS_GINYU(ch) || IS_DABURA(ch) || IS_KURZAK(ch)) {
            punch = rand_number(30, 40);
            if (IS_KURZAK(ch) || IS_TAPION(ch)) {
                punch += rand_number(5, 10);
            }
            SET_SKILL(ch, SKILL_THROW, punch);
        }
        if (IS_KAI(ch) || IS_KANASSAN(ch)) {
            punch = rand_number(40, 60);
            SET_SKILL(ch, SKILL_FOCUS, punch);
        }
        if (IS_KANASSAN(ch)) {
            punch = rand_number(40, 60);
            SET_SKILL(ch, SKILL_CONCENTRATION, punch);
        }
        if (IS_KAI(ch)) {
            punch = rand_number(30, 50);
            SET_SKILL(ch, SKILL_HEAL, punch);
        }
        if (IS_DEMON(ch)) {
            punch = rand_number(50, 60);
            SET_SKILL(ch, SKILL_SPEAR, punch);
            SET_SKILL(ch, SKILL_HONOO, punch);
        }

        punch = 0;
        punch = rand_number(50, 70);
        SET_SKILL(ch, SKILL_PUNCH, GET_SKILL_BASE(ch, SKILL_PUNCH) + punch);
    } /* End CC skills */
    else {
        ch->playerFlags.reset(PLR_FORGET);
    }

    if (IS_KAI(ch) || IS_KANASSAN(ch)) {
        punch = rand_number(15, 30);
        SET_SKILL(ch, SKILL_TELEPATHY, punch);
    }
    if (IS_MAJIN(ch) || IS_NAMEK(ch) || IS_BIO(ch)) {
        punch = rand_number(10, 16);
        SET_SKILL(ch, SKILL_REGENERATE, punch);
    }
    if (IS_ANDROID(ch) && PLR_FLAGGED(ch, PLR_ABSORB)) {
        punch = rand_number(25, 35);
        SET_SKILL(ch, SKILL_ABSORB, punch);
    }
    if (IS_BIO(ch)) {
        punch = rand_number(15, 25);
        SET_SKILL(ch, SKILL_ABSORB, punch);
    }
    if (IS_ARLIAN(ch)) {
        punch = rand_number(30, 50);
        SET_SKILL(ch, SKILL_SEISHOU, punch);
    }
    if (IS_ICER(ch)) {
        punch = rand_number(20, 30);
        SET_SKILL(ch, SKILL_TAILWHIP, punch);
    }

    if (GET_ALIGNMENT(ch) < 51 && GET_ALIGNMENT(ch) > -51) {
        set_title(ch, "the Warrior");
    }
    if (GET_ALIGNMENT(ch) >= 51) {
        set_title(ch, "the Hero");
    }
    if (GET_ALIGNMENT(ch) <= -51) {
        set_title(ch, "The Villain");
    }
    /* roll_real_abils(ch); */
    if (GET_GOLD(ch) <= 0) {
        ch->set(CharMoney::Carried, dice(3, 6) * 10);
    }

    /* Derived from the SRD under OGL, see ../doc/srd.txt for information */
    switch (GET_RACE(ch)) {
        case RACE_HUMAN:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_SAIYAN:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_HALFBREED:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_ICER:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_KONATSU:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_NAMEK:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_MUTANT:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        case RACE_ANDROID:
            ch->affected_by.set(AFF_INFRAVISION);
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
        default:
            SET_SKILL(ch, SKILL_LANG_COMMON, 1);
            break;
    }

    SPEAKING(ch) = SKILL_LANG_COMMON;

    GET_LIFEPERC(ch) = 75;

    /* assign starting items etc...*/
    obj = read_object(17, VIRTUAL);
    obj_to_char(obj, ch);
    if (IS_HOSHIJIN(ch)) {
        obj = read_object(3428, VIRTUAL);
        obj_to_char(obj, ch);
    }
    struct obj_data *obj2;
    obj2 = read_object(17998, VIRTUAL);
    obj_to_char(obj2, ch);

    if (IS_TAPION(ch) || IS_GINYU(ch)) {
        struct obj_data *throw_obj;
        throw_obj = read_object(19050, VIRTUAL);
        obj_to_char(throw_obj, ch);
        if (rand_number(1, 2) == 2) {
            throw_obj = nullptr;
            throw_obj = read_object(19050, VIRTUAL);
            obj_to_char(throw_obj, ch);
        }
        if (rand_number(1, 2) == 2) {
            throw_obj = nullptr;
            throw_obj = read_object(19050, VIRTUAL);
            obj_to_char(throw_obj, ch);
        }

    } else if (IS_DABURA(ch)) {
        struct obj_data *throw_obj;
        throw_obj = read_object(19055, VIRTUAL);
        obj_to_char(throw_obj, ch);
        throw_obj = nullptr;
        throw_obj = read_object(19055, VIRTUAL);
        obj_to_char(throw_obj, ch);
    }

    send_to_imm("New character created, %s, by user, %s.", GET_NAME(ch), GET_USER(ch));
    advance_level(ch);
    /*mudlog(BRF, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));*/

    for(auto c : {CharStat::PowerLevel, CharStat::Ki, CharStat::Stamina}) {
        if(ch->get(c) < 90) ch->set(c, 100);
    }

    if (IS_ANDROID(ch) && PLR_FLAGGED(ch, PLR_SENSEM)) {
        SET_SKILL(ch, SKILL_SENSE, 100);
        ch->gainBasePL(rand_number(400, 500));
        ch->gainBaseST(rand_number(400, 500));
        ch->gainBaseKI(rand_number(400, 500));
    }

    for(auto attr : {CharAttribute::Strength, CharAttribute::Agility, CharAttribute::Constitution,
        CharAttribute::Intelligence, CharAttribute::Wisdom, CharAttribute::Speed}) {
        auto val = ch->get(attr);
        ch->set(attr, std::clamp<attribute_t>(val, 8, 20));
    }

    ch->transBonus = Random::get<double>(-0.3, 0.3);

    if (CONFIG_SITEOK_ALL)
        ch->playerFlags.set(PLR_SITEOK);

    if (GET_RACE(ch) == RACE_SAIYAN && rand_number(1, 100) >= 95) {
        ch->playerFlags.set(PLR_LSSJ);
        write_to_output(ch->desc, "@GYou were one of the few born a Legendary Super Saiyan!@n\r\n");
    }
    ch->restoreVitals();

    GET_OLC_ZONE(ch) = NOWHERE;
}


static const int free_start_feats_wizard[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_SCRIBE_SCROLL,
        0
};

static const int free_start_feats_sorcerer[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        0
};


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
static const int free_start_feats_cleric[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_HEAVY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_MEDIUM,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
static const int free_start_feats_rogue[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        0
};

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
static const int free_start_feats_fighter[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_MARTIAL_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_HEAVY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_MEDIUM,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};

static const int free_start_feats_paladin[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_MARTIAL_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_HEAVY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_MEDIUM,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};


static const int free_start_feats_barbarian[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_MARTIAL_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_MEDIUM,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};

static const int free_start_feats_bard[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};

static const int free_start_feats_ranger[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_MARTIAL_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
static const int free_start_feats_monk[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_MARTIAL_WEAPON_PROFICIENCY,
        FEAT_IMPROVED_GRAPPLE,
        0
};

static const int free_start_feats_druid[] = {
        FEAT_SIMPLE_WEAPON_PROFICIENCY,
        FEAT_ARMOR_PROFICIENCY_LIGHT,
        FEAT_ARMOR_PROFICIENCY_MEDIUM,
        FEAT_ARMOR_PROFICIENCY_SHIELD,
        0
};

static const int no_free_start_feats[] = {
        0
};


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
static const int *free_start_feats[] = {
        /* CLASS_ROSHI	*/ free_start_feats_wizard,
        /* CLASS_PICCOLO	*/ free_start_feats_cleric,
        /* CLASS_KRANE		*/ free_start_feats_rogue,
        /* CLASS_NAIL	*/ free_start_feats_fighter,
        /* CLASS_ANDSIX		*/ free_start_feats_monk,
        /* CLASS_GINYU	*/ free_start_feats_paladin,
        /* CLASS_FRIEZA      */ free_start_feats_sorcerer,
        /* CLASS_TAPION         */ free_start_feats_druid,
        /* CLASS_ANDSIX          */ free_start_feats_bard,
        /* CLASS_DABURA        */ free_start_feats_ranger,
        /* CLASS_KABITO     */ free_start_feats_barbarian,
        /* CLASS_ARCANE_AR	*/ no_free_start_feats,
        /* CLASS_ARCANE_TR	*/ no_free_start_feats,
        /* CLASS_ARCHMAGE	*/ free_start_feats_wizard,
        /* CLASS_ASSASSIN	*/ no_free_start_feats,
        /* CLASS_BLACKGUARD	*/ no_free_start_feats,
        /* CLASS_DRAGON_D 	*/ no_free_start_feats,
        /* CLASS_DUELIST       */ no_free_start_feats,
        /* CLASS_DWARVEN_DEF   */ no_free_start_feats,
        /* CLASS_ELDRITCH_KN   */ no_free_start_feats,
        /* CLASS_HIEROPHANT    */ no_free_start_feats,
        /* CLASS_HORIZON_WALK  */ no_free_start_feats,
        /* CLASS_LOREMASTER    */ no_free_start_feats,
        /* CLASS_MYSTIC_THEU   */ no_free_start_feats,
        /* CLASS_SHADOWDANCER  */ no_free_start_feats,
        /* CLASS_THAUMATURGIST */ no_free_start_feats
};

/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
/* Rillao: transloc, add new transes here */
void advance_level(struct char_data *ch) {
    int64_t add_hp = 0, add_move = 0, add_mana = 0, add_ki = 0;
    int add_prac = 1, add_train, i, j = 0, ranks;
    int add_gen_feats = 0, add_class_feats = 0;
    char buf[MAX_STRING_LENGTH];


    /* Derived from the SRD under OGL, see ../doc/srd.txt for information */
    if (GET_LEVEL(ch) >= 1) {
        double pl_percent = 10, ki_percent = 10, st_percent = 10, prac_reward = std::max<int>(10, GET_WIS(ch));

        if (GET_LEVEL(ch) >= 91) {
            pl_percent -= 7.2;
            ki_percent -= 7.2;
            st_percent -= 7.2;
        } else if (GET_LEVEL(ch) >= 81) {
            pl_percent -= 5.8;
            ki_percent -= 5.8;
            st_percent -= 5.8;
        } else if (GET_LEVEL(ch) >= 71) {
            pl_percent -= 5;
            ki_percent -= 5;
            st_percent -= 5;
        } else if (GET_LEVEL(ch) >= 61) {
            pl_percent -= 4;
            ki_percent -= 4;
            st_percent -= 4;
        } else if (GET_LEVEL(ch) >= 51) {
            pl_percent -= 3;
            ki_percent -= 3;
            st_percent -= 3;
        } else if (GET_LEVEL(ch) >= 41) {
            pl_percent -= 1.7;
            ki_percent -= 1.7;
            st_percent -= 1.7;
        } else if (GET_LEVEL(ch) >= 31) {
            pl_percent -= 0.8;
            ki_percent -= 0.8;
            st_percent -= 0.8;
        } else if (GET_LEVEL(ch) >= 21) {
            pl_percent -= 0.35;
            ki_percent -= 0.35;
            st_percent -= 0.35;
        } else if (GET_LEVEL(ch) >= 11) {
            pl_percent -= 0.15;
            ki_percent -= 0.15;
            st_percent -= 0.15;
        }

        switch (GET_RACE(ch)) {
            case RACE_HUMAN:
                ki_percent += 3;
                break;
            case RACE_SAIYAN:
            case RACE_MAJIN:
                pl_percent += 3;
                break;
            case RACE_MUTANT:
                st_percent += 3;
                break;
            case RACE_HALFBREED:
                pl_percent += 1.5;
                ki_percent += 1.5;
                prac_reward -= prac_reward * 0.4;
                break;
            case RACE_TRUFFLE:
                prac_reward += prac_reward * 0.5;
                break;
        }

        if (!IS_HUMAN(ch)) {
            add_hp = ((ch->getBasePL()) * 0.01) * pl_percent;
        } else if (IS_HUMAN(ch)) {
            add_hp = (((ch->getBasePL()) * 0.01) * pl_percent) * 0.8;
        }
        add_mana = ((ch->getBaseKI()) * 0.01) * ki_percent;
        add_move = ((ch->getBaseST()) * 0.01) * st_percent;
        add_prac = prac_reward + GET_INT(ch);
    }
    if (add_hp >= 300000 && add_hp < 600000) {
        add_hp *= .75;
        if (add_hp < 300000) {
            add_hp = rand_number(300000, 330000);
        }
    } else if (add_hp >= 600000 && add_hp < 1000000) {
        add_hp *= .70;
        if (add_hp < 600000) {
            add_hp = rand_number(600000, 650000);
        }
    } else if (add_hp >= 1000000 && add_hp < 2000000) {
        add_hp *= .65;
        if (add_hp < 1000000) {
            add_hp = rand_number(1000000, 1250000);
        }
    } else if (add_hp >= 2000000) {
        add_hp *= .45;
        if (add_hp < 2000000) {
            add_hp = rand_number(2000000, 2250000);
        }
    }
    if (add_hp >= 15000000) {
        add_hp = 15000000;
    }
    if (add_move >= 300000 && add_move < 600000) {
        add_move *= .75;
        if (add_move < 300000) {
            add_move = rand_number(300000, 330000);
        }
    } else if (add_move >= 600000 && add_move < 1000000) {
        add_move *= .70;
        if (add_move < 600000) {
            add_move = rand_number(600000, 650000);
        }
    } else if (add_move >= 1000000 && add_move < 2000000) {
        add_move *= .65;
        if (add_move < 1000000) {
            add_move = rand_number(1000000, 1250000);
        }
    } else if (add_move >= 2000000) {
        add_move *= .45;
        if (add_move < 2000000) {
            add_move = rand_number(2000000, 2250000);
        }
    }
    if (add_move >= 15000000) {
        add_move = 15000000;
    }
    if (add_mana >= 300000 && add_mana < 600000) {
        add_mana *= .75;
        if (add_mana < 300000) {
            add_mana = rand_number(300000, 330000);
        }
    } else if (add_mana >= 600000 && add_mana < 1000000) {
        add_mana *= .70;
        if (add_mana < 600000) {
            add_mana = rand_number(600000, 650000);
        }
    } else if (add_mana >= 1000000 && add_mana < 2000000) {
        add_mana *= .65;
        if (add_mana < 1000000) {
            add_mana = rand_number(1000000, 1250000);
        }
    } else if (add_mana >= 2000000) {
        add_mana *= .45;
        if (add_mana < 2000000) {
            add_mana = rand_number(2000000, 2250000);
        }
    }
    if (add_mana >= 15000000) {
        add_mana = 15000000;
    }
    switch (GET_LEVEL(ch)) {
        case 5:
            add_hp += rand_number(600, 1000);
            add_move += rand_number(600, 1000);
            add_mana += rand_number(600, 1000);
            break;
        case 10:
            add_hp += rand_number(5000, 8000);
            add_move += rand_number(5000, 8000);
            add_mana += rand_number(5000, 8000);
            break;
        case 20:
            add_hp += rand_number(15000, 18000);
            add_move += rand_number(15000, 18000);
            add_mana += rand_number(15000, 18000);
            break;
        case 30:
            add_hp += rand_number(20000, 30000);
            add_move += rand_number(20000, 30000);
            add_mana += rand_number(20000, 30000);
            break;
        case 40:
            add_hp += rand_number(50000, 60000);
            add_move += rand_number(50000, 60000);
            add_mana += rand_number(50000, 60000);
            break;
        case 50:
            add_hp += rand_number(60000, 70000);
            add_move += rand_number(60000, 70000);
            add_mana += rand_number(60000, 70000);
            break;
        case 60:
            add_hp += rand_number(80000, 100000);
            add_move += rand_number(80000, 100000);
            add_mana += rand_number(80000, 100000);
            break;
        case 70:
            add_hp += rand_number(100000, 150000);
            add_move += rand_number(100000, 150000);
            add_mana += rand_number(100000, 150000);
            break;
        case 80:
            add_hp += rand_number(150000, 200000);
            add_move += rand_number(150000, 200000);
            add_mana += rand_number(150000, 200000);
            break;
        case 90:
            add_hp += rand_number(150000, 200000);
            add_move += rand_number(150000, 200000);
            add_mana += rand_number(150000, 200000);
            break;
        case 99:
            add_hp += rand_number(1500000, 2000000);
            add_move += rand_number(1500000, 2000000);
            add_mana += rand_number(1500000, 2000000);
            break;
        case 100:
            add_hp += rand_number(5000000, 6000000);
            add_move += rand_number(5000000, 6000000);
            add_mana += rand_number(5000000, 6000000);
            break;
    }
    /* Derived from the SRD under OGL, see ../doc/srd.txt for information */
    if (GET_LEVEL(ch) == 1 || !(GET_LEVEL(ch) % 3)) {
        add_gen_feats += 1;
    }

    /* Derived from the SRD under OGL, see ../doc/srd.txt for information */
    if (GET_RACE(ch) == RACE_HUMAN) /* Humans get 4 extra at level 1 and  */
        add_prac += 2;                /* 1 extra per level for adaptability */

    /* Derived from the SRD under OGL, see ../doc/srd.txt for information */
    i = ability_mod_value(GET_CON(ch));
    if (GET_LEVEL(ch) > 1) {
        /* blah */
    } else {
        ch->gainBasePL(rand_number(1, 20));

        for(auto c : {CharStat::PowerLevel, CharStat::Ki, CharStat::Stamina}) {
            if(ch->get(c) < 250) ch->set(c, 250);
        }

        add_prac = 5;
        if (PLR_FLAGGED(ch, PLR_SKILLP)) {
            ch->playerFlags.reset(PLR_SKILLP);
            add_prac *= 5;
        } else {
            add_prac *= 2;
        }
    }

    /* Derived from the SRD under OGL, see ../doc/srd.txt for information */
    if (rand_number(1, 4) == 4) {
        send_to_char(ch, "@D[@mPractice Session Bonus!@D]@n\r\n");
        add_prac += rand_number(4, 12);
    }

    if ((IS_DEMON(ch) || IS_KANASSAN(ch)) && GET_LEVEL(ch) > 80) {
        add_hp *= 2;
        add_mana *= 2;
        add_move *= 2;
    } else if ((IS_DEMON(ch) || IS_KANASSAN(ch)) && GET_LEVEL(ch) > 60) {
        add_hp *= 1.75;
        add_mana *= 1.75;
        add_move *= 1.75;
    } else if ((IS_DEMON(ch) || IS_KANASSAN(ch)) && GET_LEVEL(ch) > 50) {
        add_hp *= 1.5;
        add_mana *= 1.5;
        add_move *= 1.5;
    }
        /* Rillao: transloc, add new transes here */
    else if ((IS_DEMON(ch) || IS_KANASSAN(ch)) && GET_LEVEL(ch) > 40) {
        add_hp *= 1.25;
        add_mana *= 1.25;
        add_move *= 1.25;
    }
    ch->modPractices(add_prac);
    ch->gainBasePL(add_hp, true);
    ch->gainBaseKI(add_mana, true);
    ch->gainBaseST(add_move, true);
    int nhp = add_hp;
    int nma = add_mana;
    int nmo = add_move;
    add_hp = nhp;
    add_mana = nma;
    add_move = nmo;

    if (GET_ADMLEVEL(ch) >= ADMLVL_IMMORT) {
        for (i = 0; i < 3; i++)
            GET_COND(ch, i) = (char) -1;
        ch->pref.set(PRF_HOLYLIGHT);
    }

    sprintf(buf, "@D[@YGain@D: @RPl@D(@G%s@D) @gSt@D(@G%s@D) @CKi@D(@G%s@D) @bPS@D(@G%s@D)]", add_commas(add_hp).c_str(),
            add_commas(add_move).c_str(), add_commas(add_mana).c_str(), add_commas(add_prac).c_str());
    if (GET_BONUS(ch, BONUS_GMEMORY) &&
        (GET_LEVEL(ch) == 20 || GET_LEVEL(ch) == 40 || GET_LEVEL(ch) == 60 || GET_LEVEL(ch) == 80 ||
         GET_LEVEL(ch) == 100)) {
        GET_SLOTS(ch) += 1;
        send_to_char(ch, "@CYou feel like you could remember a new skill!@n\r\n");
    }
    if (IS_NAMEK(ch) && rand_number(1, 100) <= 5) {
        GET_SLOTS(ch) += 1;
        send_to_char(ch, "@CYou feel as though you could learn another skill.@n\r\n");
    }
    if (IS_ICER(ch) && rand_number(1, 100) <= 25) {
        bring_to_cap(ch);
        send_to_char(ch, "@GYou feel your body obtain its current optimal strength!@n\r\n");
    }

    int gain_stat = false;

    switch (GET_LEVEL(ch)) {
        case 10:
        case 20:
        case 30:
        case 40:
        case 50:
        case 60:
        case 70:
        case 80:
        case 90:
        case 100:
            gain_stat = true;
            break;
    }

    if (gain_stat == true) {
        int raise = false, stat_fail = 0;
        if (IS_KONATSU(ch)) {
            while (raise == false) {
                if (auto agi = ch->get(CharAttribute::Agility, true); agi < 100 && rand_number(1, 2) == 2 && stat_fail != 1) {
                    if (agi < 45 || GET_BONUS(ch, BONUS_CLUMSY) <= 0) {
                        ch->mod(CharAttribute::Agility, 1);
                        send_to_char(ch, "@GYou feel your agility increase!@n\r\n");
                        raise = true;
                    } else {
                        stat_fail += 1;
                    }
                } else if (auto speed = ch->get(CharAttribute::Speed, true); speed < 100 && raise == false && stat_fail < 2) {
                    if (speed < 45 || GET_BONUS(ch, BONUS_SLOW) > 0) {
                        ch->mod(CharAttribute::Speed, 1);
                        send_to_char(ch, "@GYou feel your speed increase!@n\r\n");
                        raise = true;
                    } else {
                        stat_fail += 2;
                    }
                } else if (stat_fail == 3) {
                    send_to_char(ch, "@RBoth agility and speed are capped!@n");
                    raise = true;
                }
            } // End while
        } // End Konatsu

        else if (IS_MUTANT(ch)) {
            while (raise == false) {
                if (auto con = ch->get(CharAttribute::Constitution, true); con < 100 && rand_number(1, 2) == 2 && stat_fail != 1) {
                    if (con < 45 || GET_BONUS(ch, BONUS_FRAIL) <= 0) {
                        ch->mod(CharAttribute::Constitution, 1);
                        send_to_char(ch, "@GYou feel your constitution increase!@n\r\n");
                        raise = true;
                    } else {
                        stat_fail += 1;
                    }
                } else if (auto speed = ch->get(CharAttribute::Speed, true); speed < 100 && raise == false && stat_fail < 2) {
                    if (speed < 45 || GET_BONUS(ch, BONUS_SLOW) > 0) {
                        ch->mod(CharAttribute::Speed, 1);
                        send_to_char(ch, "@GYou feel your speed increase!@n\r\n");
                        raise = true;
                    } else {
                        stat_fail += 2;
                    }
                } else if (stat_fail == 3) {
                    send_to_char(ch, "@RBoth constitution and speed are capped!@n");
                    raise = true;
                }
            } // End while
        } // End Mutant

        else if (IS_HOSHIJIN(ch)) {
            while (raise == false) {
                if (auto str = ch->get(CharAttribute::Strength, true) ; str < 100 && rand_number(1, 2) == 2 && stat_fail != 1) {
                    if (str < 45 || GET_BONUS(ch, BONUS_WIMP) <= 0) {
                        ch->mod(CharAttribute::Strength, 1);
                        send_to_char(ch, "@GYou feel your strength increase!@n\r\n");
                        raise = true;
                    } else {
                        stat_fail += 1;
                    }
                } else if (auto agi = ch->get(CharAttribute::Agility, true) ; agi < 100 && raise == false && stat_fail < 2) {
                    if (agi < 45 || GET_BONUS(ch, BONUS_SLOW) > 0) {
                        ch->mod(CharAttribute::Agility, 1);
                        send_to_char(ch, "@GYou feel your agility increase!@n\r\n");
                        raise = true;
                    } else {
                        stat_fail += 2;
                    }
                } else if (stat_fail == 3) {
                    send_to_char(ch, "@RBoth strength and agility are capped!@n");
                    raise = true;
                }
            } // End while
        } // End Mutant

    } // End stat bonus on level gain

    strcat(buf, ".\r\n");
    send_to_char(ch, "%s", buf);

    if (GET_SKILL(ch, SKILL_POTENTIAL) && rand_number(1, 4) == 4) {
        send_to_char(ch, "You can now perform another Potential Release.\r\n");
        GET_BOOSTS(ch) += 1;
    }
    if (IS_MAJIN(ch) && ((GET_LEVEL(ch) % 25) == 0)) {
        send_to_char(ch, "You can now perform another Majinization.\r\n");
        GET_BOOSTS(ch) += 1;
    }

    if ((GET_LEVEL(ch) % 10) == 0) {
        // every 10 levels...
        const std::map<int, std::pair<CharAttribute, std::string>> checks = {
            {BONUS_BRAWNY, {CharAttribute::Strength, "@GYour muscles have grown stronger!@n"}},
            {BONUS_SCHOLARLY, {CharAttribute::Intelligence, "@GYour mind has grown sharper!@n"}},
            {BONUS_SAGE, {CharAttribute::Wisdom, "@GYour understanding about life has improved!@n"}},
            {BONUS_AGILE, {CharAttribute::Agility, "@GYour body has grown more agile!@n"}},
            {BONUS_QUICK, {CharAttribute::Speed, "@GYou feel like your speed has improved!@n"}},
            {BONUS_STURDY, {CharAttribute::Constitution, "@GYour body feels tougher now!@n"}},
        };
        for (auto& [trait, res]: checks) {
            if (GET_BONUS(ch, trait)) {
                ch->mod(res.first, 2);
                send_to_char(ch, "%s\r\n", res.second.c_str());
            }
        }
    }


    snoop_check(ch);
    ch->save();
}

/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */
int invalid_class(struct char_data *ch, struct obj_data *obj) {
    if (OBJ_FLAGGED(obj, ITEM_ANTI_WIZARD) && IS_ROSHI(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_CLERIC) && IS_PICCOLO(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_FIGHTER) && IS_NAIL(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_ROGUE) && IS_KRANE(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_MONK) && IS_BARDOCK(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_MONK) && !IS_BARDOCK(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_WIZARD) && !IS_ROSHI(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_JINTO) && !IS_JINTO(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_CLERIC) && !IS_PICCOLO(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_ROGUE) && !IS_KRANE(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_ASSASSIN) && !IS_KURZAK(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_FIGHTER) && !IS_NAIL(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_PALADIN) && !IS_GINYU(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_WIZARD) && IS_ROSHI(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_PALADIN) && IS_GINYU(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_BARBARIAN) && IS_KABITO(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_BARD) && IS_ANDSIX(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ONLY_BARD) && !IS_ANDSIX(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_RANGER) && IS_DABURA(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_DRUID) && IS_TAPION(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_ARCANE_ARCHER) && IS_JINTO(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_ARCANE_TRICKSTER) && IS_TSUNA(ch))
        return true;

    if (OBJ_FLAGGED(obj, ITEM_ANTI_ARCHMAGE) && IS_KURZAK(ch))
        return true;


    return false;
}


/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */

/* Function to return the exp required for each class/level */
int64_t level_exp(struct char_data *ch, int level) {
    int req = 1;

    switch (level) {
        case 0:
            req = 0;
            break;
        case 1:
            req = 1;
            break;
        case 2:
            req = 999;
            break;
        case 3:
            req = 2000;
            break;
        case 4:
            req = 4500;
            break;
        case 5:
            req = 6000;
            break;
        case 6:
            req = 10000;
            break;
        case 7:
            req = 15000;
            break;
        case 8:
            req = 25000;
            break;
        case 9:
            req = 35000;
            break;
        case 10:
            req = 50000;
            break;
        case 11:
            req = 75000;
            break;
        case 12:
            req = 100000;
            break;
        case 13:
            req = 125000;
            break;
        case 14:
            req = 150000;
            break;
        case 15:
            req = 175000;
            break;
        case 16:
            req = 225000;
            break;
        case 17:
            req = 300000;
            break;
        case 18:
            req = 350000;
            break;
        case 19:
            req = 400000;
            break;
        case 20:
            req = 500000;
            break;
        case 21:
            req = 650000;
            break;
        case 22:
            req = 800000;
            break;
        case 23:
            req = 1000000;
            break;
        case 24:
            req = 1250000;
            break;
        case 25:
            req = 1500000;
            break;
        case 26:
            req = 1750000;
            break;
        case 27:
            req = 2000000;
            break;
        case 28:
            req = 2250000;
            break;
        case 29:
            req = 2500000;
            break;
        case 30:
            req = 2750000;
            break;
        case 31:
            req = 3000000;
            break;
        case 32:
            req = 3250000;
            break;
        case 33:
            req = 3500000;
            break;
        case 34:
            req = 3750000;
            break;
        case 35:
            req = 4000000;
            break;
        case 36:
            req = 4250000;
            break;
        case 37:
            req = 4500000;
            break;
        case 38:
            req = 4750000;
            break;
        case 39:
            req = 5000000;
            break;
        case 40:
            req = 5500000;
            break;
        case 41:
            req = 6000000;
            break;
        case 42:
            req = 6500000;
            break;
        case 43:
            req = 7000000;
            break;
        case 44:
            req = 7500000;
            break;
        case 45:
            req = 8000000;
            break;
        case 46:
            req = 8500000;
            break;
        case 47:
            req = 9000000;
            break;
        case 48:
            req = 9500000;
            break;
        case 49:
            req = 10000000;
            break;
        case 50:
            req = 10500000;
            break;
        case 51:
            req = 11000000;
            break;
        case 52:
            req = 11500000;
            break;
        case 53:
            req = 12000000;
            break;
        case 54:
            req = 12500000;
            break;
        case 55:
            req = 13000000;
            break;
        case 56:
            req = 13500000;
            break;
        case 57:
            req = 14000000;
            break;
        case 58:
            req = 14500000;
            break;
        case 59:
            req = 15000000;
            break;
        case 60:
            req = 18000000;
            break;
        case 61:
            req = 25000000;
            break;
        case 62:
            req = 28000000;
            break;
        case 63:
            req = 31000000;
            break;
        case 64:
            req = 34000000;
            break;
        case 65:
            req = 37000000;
            break;
        case 66:
            req = 40000000;
            break;
        case 67:
            req = 43000000;
            break;
        case 68:
            req = 46000000;
            break;
        case 69:
            req = 49000000;
            break;
        case 70:
            req = 52000000;
            break;
        case 71:
            req = 55000000;
            break;
        case 72:
            req = 58000000;
            break;
        case 73:
            req = 61000000;
            break;
        case 74:
            req = 64000000;
            break;
        case 75:
            req = 67000000;
            break;
        case 76:
            req = 70000000;
            break;
        case 77:
            req = 73000000;
            break;
        case 78:
            req = 76000000;
            break;
        case 79:
            req = 79000000;
            break;
        case 80:
            req = 82000000;
            break;
        case 81:
            req = 88000000;
            break;
        case 82:
            req = 94000000;
            break;
        case 83:
            req = 100000000;
            break;
        case 84:
            req = 106000000;
            break;
        case 85:
            req = 112000000;
            break;
        case 86:
            req = 118000000;
            break;
        case 87:
            req = 124000000;
            break;
        case 88:
            req = 130000000;
            break;
        case 89:
            req = 136000000;
            break;
        case 90:
            req = 142000000;
            break;
        case 91:
            req = 150000000;
            break;
        case 92:
            req = 175000000;
            break;
        case 93:
            req = 200000000;
            break;
        case 94:
            req = 225000000;
            break;
        case 95:
            req = 250000000;
            break;
        case 96:
            req = 300000000;
            break;
        case 97:
            req = 400000000;
            break;
        case 98:
            req = 500000000;
            break;
        case 99:
            req = 600000000;
            break;
        case 100:
            req = 800000000;
            break;
    }

    if (IS_KAI(ch)) {
        req += req * 0.15;
    }

    return (req);
}


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
int8_t ability_mod_value(int abil) {
    return ((int) (abil / 2)) - 5;
}

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
int8_t dex_mod_capped(const struct char_data *ch) {
    int8_t mod;
    struct obj_data *armor;
    mod = ability_mod_value(GET_DEX(ch));
    armor = GET_EQ(ch, WEAR_BODY);
    if (armor && GET_OBJ_TYPE(armor) == ITEM_ARMOR) {
        mod = MIN(mod, GET_OBJ_VAL(armor, VAL_ARMOR_MAXDEXMOD));
    }
    return mod;
}

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
int highest_skill_value(int level, int type) {
    if (level >= 60)
        return 100;
    else if (level >= 20)
        return level + 40;
    else if (level >= 10)
        return level + 30;
    else if (level >= 1)
        return level + 25;
    else
        return 0;
}

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
time_t birth_age(struct char_data *ch) {
    int tmp;

    tmp = rand_number(16, 18);

    return tmp;
}


namespace sensei {

    bool isValidSenseiForRace(SenseiID id, RaceID race) {
        switch (id) {
            case SenseiID::Sixteen:
                return race == RaceID::Android;
            case SenseiID::Dabura:
                return race == RaceID::Demon;
            case SenseiID::Tsuna:
                return race == RaceID::Kanassan;
            case SenseiID::Kurzak:
                return race == RaceID::Arlian;
            case SenseiID::Jinto:
                return race == RaceID::Hoshijin;
            default:
                return race != RaceID::Android;
        }
    }

    static const std::vector<SenseiID> all_senseis = {
            SenseiID::Roshi, SenseiID::Piccolo, SenseiID::Krane,
            SenseiID::Nail, SenseiID::Bardock, SenseiID::Ginyu,
            SenseiID::Frieza, SenseiID::Tapion, SenseiID::Sixteen,
            SenseiID::Dabura, SenseiID::Kibito, SenseiID::Jinto,
            SenseiID::Tsuna, SenseiID::Kurzak, SenseiID::Commoner
    };

    bool exists(SenseiID id) {
        auto found = std::find(all_senseis.begin(), all_senseis.end(), id);
        return found != all_senseis.end();
    }

    std::vector<SenseiID> filterSenseis(std::function<bool(SenseiID)> func) {
        std::vector<SenseiID> out;
        std::copy_if(all_senseis.begin(), all_senseis.end(), std::back_inserter(out), std::move(func));
        return out;
    }

    std::optional<SenseiID> findSensei(const std::string& arg, const std::function<bool(SenseiID)>& func) {
        for(auto s : all_senseis) {
            if(!func(s)) continue;
            if(boost::iequals(arg, getName(s))) return s;
        }

        return {};
    }

    static const std::unordered_map<SenseiID, room_vnum> sensei_start = {
            {SenseiID::Roshi, 1130},
            {SenseiID::Kibito, 12098},
            {SenseiID::Nail, 11683},
            {SenseiID::Bardock, 2268},
            {SenseiID::Krane, 13009},
            {SenseiID::Tapion, 8231},
            {SenseiID::Piccolo, 1659},
            {SenseiID::Sixteen, 1713},
            {SenseiID::Dabura, 6486},
            {SenseiID::Frieza, 4282},
            {SenseiID::Ginyu, 4289},
            {SenseiID::Jinto, 3499},
            {SenseiID::Kurzak, 16100},
            {SenseiID::Tsuna, 15009},
            {SenseiID::Commoner, 300}
    };

    room_vnum getStartRoom(SenseiID id) {
        if(auto found = sensei_start.find(id); found != sensei_start.end()) {
            return found->second;
        }
        return 300;
    }

    static const std::unordered_map<SenseiID, room_vnum> sensei_location = {
            {SenseiID::Roshi, 1131},
            {SenseiID::Kibito, 12098},
            {SenseiID::Nail, 11683},
            {SenseiID::Bardock, 2267},
            {SenseiID::Krane, 13012},
            {SenseiID::Tapion, 8233},
            {SenseiID::Piccolo, 1662},
            {SenseiID::Sixteen, 1714},
            {SenseiID::Dabura, 6487},
            {SenseiID::Frieza, 4283},
            {SenseiID::Ginyu, 4290},
            {SenseiID::Jinto, 3499},
            {SenseiID::Kurzak, 16100},
            {SenseiID::Tsuna, 15009},
            {SenseiID::Commoner, 300}
    };

    room_vnum getLocation(SenseiID id) {
        if(auto found = sensei_location.find(id); found != sensei_location.end()) {
            return found->second;
        }
        return 300;
    }

    static const std::unordered_map<SenseiID, std::string> sensei_name = {
            {SenseiID::Roshi, "Roshi"},
            {SenseiID::Piccolo, "Piccolo"},
            {SenseiID::Krane, "Krane"},
            {SenseiID::Nail, "Nail"},
            {SenseiID::Bardock, "Bardock"},
            {SenseiID::Ginyu, "Ginyu"},
            {SenseiID::Frieza, "Frieza"},
            {SenseiID::Tapion, "Tapion"},
            {SenseiID::Sixteen, "Android 16"},
            {SenseiID::Dabura, "Dabura"},
            {SenseiID::Kibito, "Kibito"},
            {SenseiID::Jinto, "Jinto"},
            {SenseiID::Tsuna, "Tsuna"},
            {SenseiID::Kurzak, "Kurzak"},
            {SenseiID::Commoner, "Commoner"}
    };

    static const std::unordered_map<SenseiID, std::string> sensei_abbr = {
            {SenseiID::Roshi, "Ro"},
            {SenseiID::Piccolo, "Pi"},
            {SenseiID::Krane, "Kr"},
            {SenseiID::Nail, "Na"},
            {SenseiID::Bardock, "Ba"},
            {SenseiID::Ginyu, "Gi"},
            {SenseiID::Frieza, "Fr"},
            {SenseiID::Tapion, "Ta"},
            {SenseiID::Sixteen, "16"},
            {SenseiID::Dabura, "Da"},
            {SenseiID::Kibito, "Ki"},
            {SenseiID::Jinto, "Ji"},
            {SenseiID::Tsuna, "Ts"},
            {SenseiID::Kurzak, "Ku"},
            {SenseiID::Commoner, "--"}
    };

    static const std::unordered_map<SenseiID, std::string> sensei_arts = {
            {SenseiID::Roshi, "Kame Arts"},
            {SenseiID::Piccolo, "Demon Taijutsu"},
            {SenseiID::Krane, "Crane Arts"},
            {SenseiID::Nail, "Tranquil Palm"},
            {SenseiID::Bardock, "Brutal Beast"},
            {SenseiID::Ginyu, "Flaunted Style"},
            {SenseiID::Frieza, "Frozen Fist"},
            {SenseiID::Tapion, "Shadow Grappling"},
            {SenseiID::Sixteen, "Iron Hand"},
            {SenseiID::Dabura, "Devil Dance"},
            {SenseiID::Kibito, "Gentle Fist"},
            {SenseiID::Jinto, "Star's Radiance"},
            {SenseiID::Tsuna, "Sacred Tsunami"},
            {SenseiID::Kurzak, "Adaptive Taijutsu"},
            {SenseiID::Commoner, "Like a Bum"}
    };

    std::string getName(SenseiID id) {
        if(auto found = sensei_name.find(id); found != sensei_name.end()) {
            return found->second;
        }
        return "Unknown";
    }

    std::string getAbbr(SenseiID id) {
        if(auto found = sensei_abbr.find(id); found != sensei_abbr.end()) {
            return found->second;
        }
        return "Unknown";
    }

    std::string getStyle(SenseiID id) {
        if(auto found = sensei_arts.find(id); found != sensei_arts.end()) {
            return found->second;
        }
        return "Unknown";
    }

    bool isPlayable(SenseiID id) {
        return id != SenseiID::Commoner;
    }

    struct sen_affect_type {
        int location{};
        double modifier{};
        int specific{-1};
        std::function<double(struct char_data *ch)> func{};
    };

    static std::unordered_map<SenseiID, std::vector<sen_affect_type>> sensei_affects = {};

    double getModifier(char_data* ch, int location, int specific) {
        double out = 0.0;
        if(auto found = sensei_affects.find(ch->chclass); found != sensei_affects.end()) {

            for(auto& affect : found->second) {
                if(affect.location == location) {
                    if(specific != -1 && specific != affect.specific) continue;
                    out += affect.modifier;
                    if(affect.func) {
                        out += affect.func(ch);
                    }
                }
            }
        }

        return out;
    }

}