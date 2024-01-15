/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/spell_parser.h"
#include "dbat/utils.h"
#include "dbat/interpreter.h"
#include "dbat/spells.h"
#include "dbat/handler.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/dg_scripts.h"
#include "dbat/fight.h"
#include "dbat/act.other.h"
#include "dbat/class.h"

/* extern globals */

/* local globals */
struct spell_info_type spell_info[SKILL_TABLE_SIZE];

/* local functions */
void say_spell(struct char_data *ch, int spellnum, struct char_data *tch, struct obj_data *tobj);

int mag_manacost(struct char_data *ch, int spellnum);

void mag_nextstrike(int level, struct char_data *caster, int spellnum);

void unused_spell(int spl);

void mag_assign_spells();

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, 200 should provide
 * ample slots for skills.
 */

struct syllable {
    const char *org;
    const char *news;
};


struct syllable syls[] = {
        {" ",       " "},
        {"ar",      "abra"},
        {"ate",     "i"},
        {"cau",     "kada"},
        {"blind",   "nose"},
        {"bur",     "mosa"},
        {"cu",      "judi"},
        {"de",      "oculo"},
        {"dis",     "mar"},
        {"ect",     "kamina"},
        {"en",      "uns"},
        {"gro",     "cra"},
        {"light",   "dies"},
        {"lo",      "hi"},
        {"magi",    "kari"},
        {"mon",     "bar"},
        {"mor",     "zak"},
        {"move",    "sido"},
        {"ness",    "lacri"},
        {"ning",    "illa"},
        {"per",     "duda"},
        {"ra",      "gru"},
        {"re",      "candus"},
        {"son",     "sabru"},
        {"tect",    "infra"},
        {"tri",     "cula"},
        {"ven",     "nofo"},
        {"word of", "inset"},
        {"a",       "i"},
        {"b",       "v"},
        {"c",       "q"},
        {"d",       "m"},
        {"e",       "o"},
        {"f",       "y"},
        {"g",       "t"},
        {"h",       "p"},
        {"i",       "u"},
        {"j",       "y"},
        {"k",       "t"},
        {"l",       "r"},
        {"m",       "w"},
        {"n",       "b"},
        {"o",       "a"},
        {"p",       "s"},
        {"q",       "d"},
        {"r",       "f"},
        {"s",       "g"},
        {"t",       "h"},
        {"u",       "e"},
        {"v",       "z"},
        {"w",       "x"},
        {"x",       "n"},
        {"y",       "l"},
        {"z",       "k"},
        {"",        ""}
};

const char *unused_spellname = "!UNUSED!"; /* So we can get &unused_spellname */

int mag_manacost(struct char_data *ch, int spellnum) {
    return 0;
}


int mag_kicost(struct char_data *ch, int spellnum) {
    int i, min, tval;
    SenseiID whichclass;
    return MAX(SINFO.ki_max - (SINFO.ki_change *
                               (GET_LEVEL(ch) - SINFO.min_level[(int) GET_CLASS(ch)])),
               SINFO.ki_min);
}


void mag_nextstrike(int level, struct char_data *caster, int spellnum) {
    if (!caster)
        return;
    if (caster->actq) {
        send_to_char(caster, "You can't perform more than one special attack at a time!");
        return;
    }
    CREATE(caster->actq, struct queued_act, 1);
    caster->actq->level = level;
    caster->actq->spellnum = spellnum;
}


void say_spell(struct char_data *ch, int spellnum, struct char_data *tch,
               struct obj_data *tobj) {
    char lbuf[256], buf[256], buf1[256], buf2[256];    /* FIXME */
    const char *format;

    struct char_data *i;
    int j, ofs = 0;

    *buf = '\0';
    strlcpy(lbuf, skill_name(spellnum), sizeof(lbuf));

    while (lbuf[ofs]) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
                strcat(buf, syls[j].news);    /* strcat: BAD */
                ofs += strlen(syls[j].org);
                break;
            }
        }
        /* i.e., we didn't find a match in syls[] */
        if (!*syls[j].org) {
            basic_mud_log("No entry in syllable table for substring of '%s'", lbuf);
            ofs++;
        }
    }

    if (tch != nullptr && IN_ROOM(tch) == IN_ROOM(ch)) {
        if (tch == ch)
            format = "$n closes $s eyes and utters the words, '%s'.";
        else
            format = "$n stares at $N and utters the words, '%s'.";
    } else if (tobj != nullptr &&
               ((IN_ROOM(tobj) == IN_ROOM(ch)) || (tobj->carried_by == ch)))
        format = "$n stares at $p and utters the words, '%s'.";
    else
        format = "$n utters the words, '%s'.";

    snprintf(buf1, sizeof(buf1), format, skill_name(spellnum));
    snprintf(buf2, sizeof(buf2), format, buf);

    for (i = ch->getRoom()->people; i; i = i->next_in_room) {
        if (i == ch || i == tch || !i->desc || !AWAKE(i))
            continue;
        /* This should really check spell type vs. target ranks */
        perform_act(buf1, ch, tobj, tch, i);
    }

    if (tch != nullptr && tch != ch && IN_ROOM(tch) == IN_ROOM(ch)) {
        snprintf(buf1, sizeof(buf1), "$n stares at you and utters the words, '%s'.",
                 GET_LEVEL(ch) ? skill_name(spellnum) : buf);
        act(buf1, false, ch, nullptr, tch, TO_VICT);
    }
}

/*
 * This function should be used anytime you are not 100% sure that you have
 * a valid spell/skill number.  A typical for() loop would not need to use
 * this because you can guarantee > 0 and < SKILL_TABLE_SIZE
 */
const char *skill_name(int num) {
    if (num > 0 && num < SKILL_TABLE_SIZE)
        return (spell_info[num].name);
    else if (num == -1)
        return ("UNUSED");
    else
        return ("UNDEFINED");
}

int find_skill_num(char *name, int sktype) {
    int skindex, ok;
    char *temp, *temp2;
    char first[256], first2[256], tempbuf[256];

    for (skindex = 1; skindex < SKILL_TABLE_SIZE; skindex++) {
        if (is_abbrev(name, spell_info[skindex].name) && (spell_info[skindex].skilltype & sktype)) {
            return (skindex);
        }

        ok = true;
        strlcpy(tempbuf, spell_info[skindex].name, sizeof(tempbuf));        /* strlcpy: OK */
        temp = any_one_arg(tempbuf, first);
        temp2 = any_one_arg(name, first2);
        while (*first && *first2 && ok) {
            if (!is_abbrev(first2, first))
                ok = false;
            temp = any_one_arg(temp, first);
            temp2 = any_one_arg(temp2, first2);
        }

        if (ok && !*first2 && (spell_info[skindex].skilltype & sktype)) {
            return (skindex);
        }
    }

    return (-1);
}


/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data *caster, struct char_data *cvict,
               struct obj_data *ovict, int spellnum, int level, int casttype, char *arg) {
    if (spellnum < 1 || spellnum > SKILL_TABLE_SIZE)
        return (0);

    if (!cast_wtrigger(caster, cvict, ovict, spellnum))
        return 0;
    if (!cast_otrigger(caster, ovict, spellnum))
        return 0;
    if (!cast_mtrigger(caster, cvict, spellnum))
        return 0;

    if (ROOM_FLAGGED(IN_ROOM(caster), ROOM_PEACEFUL) && GET_ADMLEVEL(caster) < ADMLVL_IMPL &&
        (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
        send_to_char(caster, "A flash of white light fills the room, dispelling your violent magic!\r\n");
        act("White light from no particular source suddenly fills the room, then vanishes.", false, caster, nullptr,
            nullptr, TO_ROOM);
        return (0);
    }

    if (IS_SET(SINFO.routines, MAG_NEXTSTRIKE) && casttype != CAST_STRIKE) {
        mag_nextstrike(level, caster, spellnum);
        return 1;
    }

    if (IS_SET(SINFO.routines, MAG_DAMAGE))
        if (mag_damage(level, caster, cvict, spellnum) == -1)
            return (-1);    /* Successful and target died, don't cast again. */

    if (IS_SET(SINFO.routines, MAG_AFFECTS))
        mag_affects(level, caster, cvict, spellnum);

    if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
        mag_unaffects(level, caster, cvict, spellnum);

    if (IS_SET(SINFO.routines, MAG_POINTS))
        mag_points(level, caster, cvict, spellnum);

    if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
        mag_alter_objs(level, caster, ovict, spellnum);

    if (IS_SET(SINFO.routines, MAG_GROUPS))
        mag_groups(level, caster, spellnum);

    if (IS_SET(SINFO.routines, MAG_MASSES))
        mag_masses(level, caster, spellnum);

    if (IS_SET(SINFO.routines, MAG_AREAS))
        mag_areas(level, caster, spellnum);

    if (IS_SET(SINFO.routines, MAG_SUMMONS))
        mag_summons(level, caster, ovict, spellnum, arg);

    if (IS_SET(SINFO.routines, MAG_CREATIONS))
        mag_creations(level, caster, spellnum);

    if (IS_SET(SINFO.routines, MAG_MANUAL))
        switch (spellnum) {
            case SPELL_CHARM:
                MANUAL_SPELL(spell_charm);
                break;
            case SPELL_CREATE_WATER:
                MANUAL_SPELL(spell_create_water);
                break;
            case SPELL_DETECT_POISON:
                MANUAL_SPELL(spell_detect_poison);
                break;
            case SPELL_ENCHANT_WEAPON:
                MANUAL_SPELL(spell_enchant_weapon);
                break;
            case SPELL_IDENTIFY:
                MANUAL_SPELL(spell_identify);
                break;
            case SPELL_LOCATE_OBJECT:
                MANUAL_SPELL(spell_locate_object);
                break;
            case SPELL_SUMMON:
                MANUAL_SPELL(spell_summon);
                break;
            case SPELL_WORD_OF_RECALL:
                MANUAL_SPELL(spell_recall);
                break;
            case SPELL_TELEPORT:
                MANUAL_SPELL(spell_teleport);
                break;
            case SPELL_PORTAL:
                MANUAL_SPELL(spell_portal);
                break;
            case ART_ABUNDANT_STEP:
                MANUAL_SPELL(art_abundant_step);
                break;
        }

    if (IS_SET(SINFO.routines, MAG_AFFECTSV))
        mag_affectsv(level, caster, cvict, spellnum);


    return (1);
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */
void mag_objectmagic(struct char_data *ch, struct obj_data *obj,
                     char *argument) {
    char arg[MAX_INPUT_LENGTH];
    int i, k;
    struct char_data *tch = nullptr, *next_tch;
    struct obj_data *tobj = nullptr;

    one_argument(argument, arg);

    k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                          FIND_OBJ_EQUIP, ch, &tch, &tobj);

    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_STAFF:
            act("You tap $p three times on the ground.", false, ch, obj, nullptr, TO_CHAR);
            if (obj->look_description)
                act(obj->look_description, false, ch, obj, nullptr, TO_ROOM);
            else
                act("$n taps $p three times on the ground.", false, ch, obj, nullptr, TO_ROOM);

            if (GET_OBJ_VAL(obj, VAL_STAFF_CHARGES) <= 0) {
                send_to_char(ch, "It seems powerless.\r\n");
                act("Nothing seems to happen.", false, ch, obj, nullptr, TO_ROOM);
            } else {
                GET_OBJ_VAL(obj, VAL_STAFF_CHARGES)--;
                ch->affected_by.set(AFF_NEXTNOACTION);
                /* Level to cast spell at. */
                k = GET_OBJ_VAL(obj, VAL_STAFF_LEVEL) ? GET_OBJ_VAL(obj, VAL_STAFF_LEVEL) : DEFAULT_STAFF_LVL;

                /*
                 * Problem : Area/mass spells on staves can cause crashes.
                 * Solution: Remove the special nature of area/mass spells on staves.
                 * Problem : People like that behavior.
                 * Solution: We special case the area/mass spells here.
                 */
                if (HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, VAL_STAFF_SPELL), MAG_MASSES | MAG_AREAS)) {
                    for (i = 0, tch = ch->getRoom()->people; tch; tch = tch->next_in_room)
                        i++;
                    while (i-- > 0)
                        call_magic(ch, nullptr, nullptr, GET_OBJ_VAL(obj, VAL_STAFF_SPELL), k, CAST_STAFF, nullptr);
                } else {
                    for (tch = ch->getRoom()->people; tch; tch = next_tch) {
                        next_tch = tch->next_in_room;
                        if (ch != tch)
                            call_magic(ch, tch, nullptr, GET_OBJ_VAL(obj, VAL_STAFF_SPELL), k, CAST_STAFF, nullptr);
                    }
                }
            }
            break;
        case ITEM_WAND:
            if (k == FIND_CHAR_ROOM) {
                if (tch == ch) {
                    act("You point $p at yourself.", false, ch, obj, nullptr, TO_CHAR);
                    act("$n points $p at $mself.", false, ch, obj, nullptr, TO_ROOM);
                } else {
                    act("You point $p at $N.", false, ch, obj, tch, TO_CHAR);
                    if (obj->look_description)
                        act(obj->look_description, false, ch, obj, tch, TO_ROOM);
                    else
                        act("$n points $p at $N.", true, ch, obj, tch, TO_ROOM);
                }
            } else if (tobj != nullptr) {
                act("You point $p at $P.", false, ch, obj, tobj, TO_CHAR);
                if (obj->look_description)
                    act(obj->look_description, false, ch, obj, tobj, TO_ROOM);
                else
                    act("$n points $p at $P.", true, ch, obj, tobj, TO_ROOM);
            } else if (IS_SET(spell_info[GET_OBJ_VAL(obj, VAL_WAND_SPELL)].routines, MAG_AREAS | MAG_MASSES)) {
                /* Wands with area spells don't need to be pointed. */
                act("You point $p outward.", false, ch, obj, nullptr, TO_CHAR);
                act("$n points $p outward.", true, ch, obj, nullptr, TO_ROOM);
            } else {
                act("At what should $p be pointed?", false, ch, obj, nullptr, TO_CHAR);
                return;
            }

            if (GET_OBJ_VAL(obj, VAL_WAND_CHARGES) <= 0) {
                send_to_char(ch, "It seems powerless.\r\n");
                act("Nothing seems to happen.", false, ch, obj, nullptr, TO_ROOM);
                return;
            }
            GET_OBJ_VAL(obj, VAL_WAND_CHARGES)--;
            ch->affected_by.set(AFF_NEXTNOACTION);
            if (GET_OBJ_VAL(obj, VAL_WAND_LEVEL))
                call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, VAL_WAND_SPELL),
                           GET_OBJ_VAL(obj, VAL_WAND_LEVEL), CAST_WAND, nullptr);
            else
                call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, VAL_WAND_SPELL),
                           DEFAULT_WAND_LVL, CAST_WAND, nullptr);
            break;
        case ITEM_SCROLL:
            if (*arg) {
                if (!k) {
                    act("There is nothing to here to affect with $p.", false,
                        ch, obj, nullptr, TO_CHAR);
                    return;
                }
            } else
                tch = ch;

            act("You recite $p which dissolves.", true, ch, obj, nullptr, TO_CHAR);
            if (obj->look_description)
                act(obj->look_description, false, ch, obj, tch, TO_ROOM);
            else
                act("$n recites $p.", false, ch, obj, nullptr, TO_ROOM);

            ch->affected_by.set(AFF_NEXTNOACTION);
            for (i = 1; i <= 3; i++)
                if (call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
                               GET_OBJ_VAL(obj, VAL_SCROLL_LEVEL), CAST_SCROLL, nullptr) <= 0)
                    break;

            if (obj != nullptr)
                extract_obj(obj);
            break;
        case ITEM_POTION:
            tch = ch;

            if (!consume_otrigger(obj, ch, OCMD_QUAFF))  /* check trigger */
                return;

            act("You swallow $p.", false, ch, obj, nullptr, TO_CHAR);
            if (obj->look_description)
                act(obj->look_description, false, ch, obj, nullptr, TO_ROOM);
            else
                act("$n swallows $p.", true, ch, obj, nullptr, TO_ROOM);

            ch->affected_by.set(AFF_NEXTNOACTION);
            for (i = 1; i <= 3; i++)
                if (call_magic(ch, ch, nullptr, GET_OBJ_VAL(obj, i),
                               GET_OBJ_VAL(obj, VAL_POTION_LEVEL), CAST_POTION, nullptr) <= 0)
                    break;

            if (obj != nullptr)
                extract_obj(obj);
            break;
        default:
            basic_mud_log("SYSERR: Unknown object_type %d in mag_objectmagic.",
                GET_OBJ_TYPE(obj));
            break;
    }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */
int cast_spell(struct char_data *ch, struct char_data *tch,
               struct obj_data *tobj, int spellnum, char *arg) {
    int whichclass = -1, i, j, diff = -1, lvl = 1;

    if (spellnum < 0 || spellnum >= SKILL_TABLE_SIZE) {
        basic_mud_log("SYSERR: cast_spell trying to call out of range spellnum %d/%d.", spellnum,
            SKILL_TABLE_SIZE);
        return (0);
    }

    if (!IS_SET(SINFO.skilltype, SKTYPE_SPELL | SKTYPE_ART)) {
        basic_mud_log("SYSERR: cast_spell trying to call nonspell spellnum %d/%d.", spellnum,
            SKILL_TABLE_SIZE);
        return (0);
    }

    if (GET_POS(ch) < SINFO.min_position) {
        switch (GET_POS(ch)) {
            case POS_SLEEPING:
                send_to_char(ch, "You dream about great magical powers.\r\n");
                break;
            case POS_RESTING:
                send_to_char(ch, "You cannot concentrate while resting.\r\n");
                break;
            case POS_SITTING:
                send_to_char(ch, "You can't do this sitting!\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Impossible!  You can't concentrate enough!\r\n");
                break;
            default:
                send_to_char(ch, "You can't do much of anything like this!\r\n");
                break;
        }
        return (0);
    }
    if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == tch)) {
        send_to_char(ch, "You are afraid you might hurt your master!\r\n");
        return (0);
    }
    if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
        send_to_char(ch, "You can only cast this spell upon yourself!\r\n");
        return (0);
    }
    if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
        send_to_char(ch, "You cannot cast this spell upon yourself!\r\n");
        return (0);
    }
    if (IS_SET(SINFO.routines, MAG_GROUPS) && !AFF_FLAGGED(ch, AFF_GROUP)) {
        send_to_char(ch, "You can't cast this spell if you're not in a group!\r\n");
        return (0);
    }

    if (IS_SET(SINFO.skilltype, SKTYPE_SPELL)) {
        for (i = 0; i < NUM_CLASSES; i++) {
            j = GET_LEVEL(ch) - spell_info[spellnum].min_level[i];
            if (j > diff) {
                whichclass = i;
                diff = j;
                lvl = GET_LEVEL(ch);
            }
        }
    } else if (IS_SET(SINFO.skilltype, SKTYPE_ART))
        lvl = GET_LEVEL(ch) / 2;
    else
        lvl = GET_LEVEL(ch);

    send_to_char(ch, "%s", CONFIG_OK);
    if (IS_SET(SINFO.skilltype, SKTYPE_SPELL)) {
        say_spell(ch, spellnum, tch, tobj);
    }

    return (call_magic(ch, tch, tobj, spellnum, lvl, CAST_SPELL, arg));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */
ACMD(do_cast) {
    struct char_data *tch = nullptr;
    struct obj_data *tobj = nullptr;
    char *s, *t, buffer[25];
    /* char export[256];
    int mana, percent; */
    int ki = 0;
    int spellnum, i, target = 0, innate = false;


    /* get: blank, spell name, target name */
    s = strtok(argument, "'");

    if (s == nullptr) {
        if (subcmd == SCMD_ART)
            send_to_char(ch, "Use what ability?\r\n");
        else
            send_to_char(ch, "Cast what where?\r\n");
        return;
    }
    s = strtok(nullptr, "'");
    if (s == nullptr) {
        if (subcmd == SCMD_ART)
            send_to_char(ch, "You must enclose the ability name in quotes: '\r\n");
        else
            send_to_char(ch, "Spell names must be enclosed in the Holy Magic Symbols: '\r\n");
        return;
    }
    t = strtok(nullptr, "\0");

    /* spellnum = search_block(s, spells, 0); */
    spellnum = find_skill_num(s, SKTYPE_SPELL | SKTYPE_ART);

    sprintf(buffer, "%d", spellnum);

    if (subcmd == SCMD_ART) {
        if (!IS_KABITO(ch) && GET_ADMLEVEL(ch) < ADMLVL_IMMORT) {
            send_to_char(ch, "You are not trained in that.\r\n");
            return;
        }

        if ((spellnum < 1) || (spellnum >= SKILL_TABLE_SIZE) || !(SINFO.skilltype & SKTYPE_ART) || !*s) {
            send_to_char(ch, "I don't recognize that martial art or ability.\r\n");
            return;
        }

        if (!GET_SKILL(ch, spellnum)) {
            send_to_char(ch, "You do not have that ability.\r\n");
            return;
        }
    } else {
        if (GET_ADMLEVEL(ch) < ADMLVL_IMMORT) {
            send_to_char(ch, "You are not able to cast spells.\r\n");
            return;
        }

        if ((spellnum < 1) || (spellnum >= SKILL_TABLE_SIZE) || !(SINFO.skilltype & SKTYPE_SPELL)) {
            send_to_char(ch, "Cast what?!?\r\n");
            return;
        }

        /* if spell NOT memorized */
    }

    if (subcmd == SCMD_ART) {
        ki = mag_kicost(ch, spellnum);
        if ((ki > 0) && (GET_KI(ch) < ki) && (GET_ADMLEVEL(ch) < ADMLVL_IMMORT)) {
            send_to_char(ch, "You haven't the energy to cast that spell!\r\n");
            return;
        }
    }

    /* Find the target */
    if (t != nullptr) {
        char arg[MAX_INPUT_LENGTH];

        strlcpy(arg, t, sizeof(arg));
        one_argument(arg, t);
        skip_spaces(&t);
    }
    if (IS_SET(SINFO.targets, TAR_IGNORE)) {
        target = true;
    } else if (t != nullptr && *t) {
        if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
            if ((tch = get_char_vis(ch, t, nullptr, FIND_CHAR_ROOM)) != nullptr)
                target = true;
        }
        if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
            if ((tch = get_char_vis(ch, t, nullptr, FIND_CHAR_WORLD)) != nullptr)
                target = true;

        if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
            if ((tobj = get_obj_in_list_vis(ch, t, nullptr, ch->contents)) != nullptr)
                target = true;

        if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
            for (i = 0; !target && i < NUM_WEARS; i++)
                if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
                    tobj = GET_EQ(ch, i);
                    target = true;
                }
        }
        if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
            if ((tobj = get_obj_in_list_vis(ch, t, nullptr, ch->getRoom()->contents)) != nullptr)
                target = true;

        if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
            if ((tobj = get_obj_vis(ch, t, nullptr)) != nullptr)
                target = true;

    } else {            /* if target string is empty */
        if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
            if (FIGHTING(ch) != nullptr) {
                tch = ch;
                target = true;
            }
        if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
            if (FIGHTING(ch) != nullptr) {
                tch = FIGHTING(ch);
                target = true;
            }
        /* if no target specified, and the spell isn't violent, default to self */
        if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
            !SINFO.violent) {
            tch = ch;
            target = true;
        }
        if (!target) {
            send_to_char(ch, "Upon %s should the spell be cast?\r\n",
                         IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD | TAR_OBJ_EQUIP) ? "what"
                                                                                                           : "who");
            return;
        }
    }

    if (target && (tch == ch) && SINFO.violent) {
        send_to_char(ch, "You shouldn't cast that on yourself -- could be bad for your health!\r\n");
        return;
    }

    if (!target) {
        send_to_char(ch, "Cannot find the target of your spell!\r\n");
        return;
    }


    if (SINFO.violent && tch && IS_NPC(tch)) {
        if (!FIGHTING(tch))
            set_fighting(tch, ch);
        if (!FIGHTING(ch))
            set_fighting(ch, tch);
    }

    if (IS_SET(SINFO.comp_flags, MAGCOMP_SOMATIC) && rand_number(1, 100) <= GET_SPELLFAIL(ch)) {
        if (IS_SET(SINFO.routines, MAG_ACTION_FULL | MAG_ACTION_PARTIAL))
            ch->affected_by.set(AFF_NEXTPARTIAL);
        else if (IS_SET(SINFO.routines, MAG_ACTION_FULL | MAG_ACTION_FULL))
            ch->affected_by.set(AFF_NEXTNOACTION);
        send_to_char(ch, "Your armor interferes with your casting, and you fail!\r\n");
    } else {
        if (cast_spell(ch, tch, tobj, spellnum, t) && GET_ADMLEVEL(ch) < ADMLVL_IMMORT) {
            if (IS_SET(SINFO.routines, MAG_ACTION_FULL | MAG_ACTION_PARTIAL))
                ch->affected_by.set(AFF_NEXTPARTIAL);
            else if (IS_SET(SINFO.routines, MAG_ACTION_FULL | MAG_ACTION_FULL))
                ch->affected_by.set(AFF_NEXTNOACTION);
            if (subcmd == SCMD_CAST) {
                send_to_char(ch, "The magical energy from the spell leaves your mind.\r\n");
                if (PRF_FLAGGED(ch, PRF_AUTOMEM)) {
                    send_to_char(ch, "You begin to commit the spell again to your mind.\r\n");
                }
            }
        }
    }
}

void skill_race_class(int spell, int race, int learntype) {
    int bad = 0;

    if (spell < 0 || spell >= SKILL_TABLE_SIZE) {
        basic_mud_log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, SKILL_TABLE_SIZE);
        return;
    }

    if (race < 0 || race >= NUM_RACES) {
        basic_mud_log("SYSERR: assigning '%s' to illegal race %d/%d.", skill_name(spell),
            race, NUM_RACES - 1);
        bad = 1;
    }

    if (!bad)
        spell_info[spell].race_can_learn[race] = learntype;
}

void spell_level(int spell, int chclass, int level) {
    int bad = 0;

    if (spell < 0 || spell > SKILL_TABLE_SIZE) {
        basic_mud_log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, SKILL_TABLE_SIZE);
        return;
    }

    if (chclass < 0 || chclass >= NUM_CLASSES) {
        basic_mud_log("SYSERR: assigning '%s' to illegal class %d/%d.", skill_name(spell),
            chclass, NUM_CLASSES - 1);
        bad = 1;
    }

    if (level < 1) {
        basic_mud_log("SYSERR: assigning '%s' to illegal level %d.", skill_name(spell),
            level);
        bad = 1;
    }

    if (!bad)
        spell_info[spell].min_level[chclass] = level;
}

void skill_class(int skill, int chclass, int learntype) {
    int bad = 0;

    if (skill < 0 || skill > SKILL_TABLE_SIZE) {
        basic_mud_log("SYSERR: attempting assign to illegal skillnum %d/%d", skill, SKILL_TABLE_SIZE);
        return;
    }

    if (chclass < 0 || chclass >= NUM_CLASSES) {
        basic_mud_log("SYSERR: assigning '%s' to illegal class %d/%d.", skill_name(skill),
            chclass, NUM_CLASSES - 1);
        bad = 1;
    }

    if (learntype < 0 || learntype > SKLEARN_CLASS) {
        basic_mud_log("SYSERR: assigning skill '%s' illegal learn type %d for class %d.", skill_name(skill),
            learntype, chclass);
        bad = 1;
    }

    if (!bad)
        spell_info[skill].can_learn_skill[chclass] = learntype;
}

int skill_type(int snum) {
    return spell_info[snum].skilltype;
}

void set_skill_type(int snum, int sktype) {
    spell_info[snum].skilltype = sktype;
}


/* Assign the spells on boot up */
void
spello(int spl, const char *name, int max_mana, int min_mana, int mana_change, int minpos, int targets, int violent,
       int routines, int save_flags, int comp_flags, const char *wearoff, int cmspell_level, int school, int domain) {
    int i;

    for (i = 0; i < NUM_CLASSES; i++)
        spell_info[spl].min_level[i] = CONFIG_LEVEL_CAP;
    for (i = 0; i < NUM_RACES; i++)
        spell_info[spl].race_can_learn[i] = CONFIG_LEVEL_CAP;
    spell_info[spl].mana_max = max_mana;
    spell_info[spl].mana_min = min_mana;
    spell_info[spl].mana_change = mana_change;
    spell_info[spl].ki_max = 0;
    spell_info[spl].ki_min = 0;
    spell_info[spl].ki_change = 0;
    spell_info[spl].min_position = minpos;
    spell_info[spl].targets = targets;
    spell_info[spl].violent = violent;
    spell_info[spl].routines = routines;
    spell_info[spl].name = name;
    spell_info[spl].wear_off_msg = wearoff;
    spell_info[spl].skilltype = SKTYPE_SPELL;
    spell_info[spl].flags = 0;
    spell_info[spl].save_flags = save_flags;
    spell_info[spl].comp_flags = comp_flags;
    spell_info[spl].spell_level = cmspell_level;
    spell_info[spl].school = school;
    spell_info[spl].domain = domain;
}


void arto(int spl, const char *name, int max_ki, int min_ki, int ki_change, int minpos, int targets, int violent,
          int routines, int save_flags, int comp_flags, const char *wearoff) {
    spello(spl, name, 0, 0, 0, minpos, targets, violent, routines, save_flags, comp_flags, wearoff, 0, 0, 0);
    set_skill_type(spl, SKTYPE_ART);
    spell_info[spl].ki_max = max_ki;
    spell_info[spl].ki_min = min_ki;
    spell_info[spl].ki_change = ki_change;
}


void unused_spell(int spl) {
    int i;

    for (i = 0; i < NUM_CLASSES; i++) {
        spell_info[spl].min_level[i] = CONFIG_LEVEL_CAP;
        spell_info[spl].can_learn_skill[i] = SKLEARN_CROSSCLASS;
    }
    for (i = 0; i < NUM_RACES; i++)
        spell_info[spl].race_can_learn[i] = SKLEARN_CROSSCLASS;
    spell_info[spl].mana_max = 0;
    spell_info[spl].mana_min = 0;
    spell_info[spl].mana_change = 0;
    spell_info[spl].ki_max = 0;
    spell_info[spl].ki_min = 0;
    spell_info[spl].ki_change = 0;
    spell_info[spl].min_position = 0;
    spell_info[spl].targets = 0;
    spell_info[spl].violent = 0;
    spell_info[spl].routines = 0;
    spell_info[spl].name = unused_spellname;
    spell_info[spl].skilltype = SKTYPE_NONE;
    spell_info[spl].flags = 0;
    spell_info[spl].save_flags = 0;
    spell_info[spl].comp_flags = 0;
    spell_info[spl].spell_level = 0;
    spell_info[spl].school = 0;
    spell_info[spl].domain = 0;
}


void skillo(int skill, const char *name, int flags) {
    spello(skill, name, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr, 0, 0, 0);
    spell_info[skill].skilltype = SKTYPE_SKILL;
    spell_info[skill].flags = flags;
}

/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell).
 *
 * minmana :  The minimum mana this spell will take, no matter how high
 * level the caster is.
 *
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * spellname: The name of the spell.
 *
 * school: The school of the spell.
 *
 * domain: The domain of the spell.
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

/*
 * NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
 * In order to make this cleaner, as well as to make adding new classes
 * much easier, spell levels are now assigned in class.c.  You only need
 * a spello() call to define a new spell; to decide who gets to use a spell
 * or skill, look in class.c.  -JE 5 Feb 1996
 */

void mag_assign_spells() {
    int i;

    /* Do not change the loop below. */
    for (i = 0; i < SKILL_TABLE_SIZE; i++)
        unused_spell(i);
    /* Do not change the loop above. */

    spello(SPELL_ANIMATE_DEAD, "animate dead", 35, 10, 3, POS_STANDING,
           TAR_OBJ_ROOM, false,
           MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           3, SCHOOL_NECROMANCY, DOMAIN_DEATH);

    spello(SPELL_MAGE_ARMOR, "mage armor", 30, 15, 3, POS_FIGHTING,
           TAR_CHAR_ROOM, false,
           MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "You feel less protected.",
           1, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_BLESS, "blessed", 35, 5, 3, POS_STANDING,
           TAR_CHAR_ROOM | TAR_OBJ_INV, false,
           MAG_ACTION_FULL | MAG_AFFECTS | MAG_ALTER_OBJS, 0, 0,
           "You feel less righteous.",
           1, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SKILL_SPIRITCONTROL, "spirit control", 35, 5, 3, POS_STANDING,
           TAR_CHAR_ROOM | TAR_OBJ_INV, false,
           MAG_ACTION_FULL | MAG_AFFECTS | MAG_ALTER_OBJS, 0, 0,
           "You no longer have full control of your spirit.",
           1, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_BLINDNESS, "blindness", 35, 25, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, false,
           MAG_ACTION_FULL | MAG_AFFECTS, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "You feel a cloak of blindness dissolve.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNIVERSAL);

    spello(SKILL_SOLARF, "blind", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "You are no longer blind!",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SPELL_BURNING_HANDS, "burning hands", 30, 10, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true,
           MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_REFLEX | MAGSAVE_HALF, 0,
           nullptr,
           1, SCHOOL_TRANSMUTATION, DOMAIN_FIRE);

    spello(SPELL_CALL_LIGHTNING, "call lightning", 40, 25, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true,
           MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_REFLEX | MAGSAVE_HALF, 0,
           nullptr,
           3, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_INFLICT_CRITIC, "inflict critic", 30, 10, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, true,
           MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_WILL | MAGSAVE_HALF, 0,
           nullptr,
           4, SCHOOL_UNDEFINED, DOMAIN_HEALING);

    spello(SPELL_INFLICT_LIGHT, "inflict light", 30, 10, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, true,
           MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_WILL | MAGSAVE_HALF, 0,
           nullptr,
           1, SCHOOL_UNDEFINED, DOMAIN_HEALING);

    spello(SPELL_CHARM, "charm person", 75, 50, 2, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true,
           MAG_ACTION_FULL | MAG_MANUAL, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           "You feel more self-confident.",
           1, SCHOOL_ENCHANTMENT, DOMAIN_UNDEFINED);

    spello(SPELL_CHILL_TOUCH, "chill touch", 30, 10, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true,
           MAG_ACTION_FULL | MAG_DAMAGE | MAG_AFFECTS, MAGSAVE_FORT | MAGSAVE_PARTIAL, 0,
           "You feel your strength return.",
           1, SCHOOL_NECROMANCY, DOMAIN_UNDEFINED);

    spello(SPELL_COLOR_SPRAY, "color spray", 30, 15, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true,
           MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           nullptr,
           1, SCHOOL_ILLUSION, DOMAIN_UNDEFINED);

    spello(SPELL_CONTROL_WEATHER, "control weather", 75, 25, 5, POS_STANDING,
           TAR_IGNORE, false,
           MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           7, SCHOOL_TRANSMUTATION, DOMAIN_AIR);

    spello(SPELL_CREATE_FOOD, "create food", 30, 5, 4, POS_STANDING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_CREATIONS, 0, 0,
           nullptr,
           3, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_CREATE_WATER, "create water", 30, 5, 4, POS_STANDING,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_REMOVE_BLINDNESS, "remove blindness", 30, 5, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_UNAFFECTS, 0, 0,
           nullptr,
           3, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_CURE_CRITIC, "cure critic", 30, 10, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_POINTS, 0, 0,
           nullptr,
           4, SCHOOL_UNDEFINED, DOMAIN_HEALING);

    spello(SPELL_CURE_LIGHT, "cure light", 30, 10, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_POINTS, 0, 0,
           nullptr,
           1, SCHOOL_UNDEFINED, DOMAIN_HEALING);

    spello(SPELL_BESTOW_CURSE, "bestow curse", 80, 50, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, true,
           MAG_AFFECTS, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           "You feel more optimistic.",
           8, SCHOOL_NECROMANCY, DOMAIN_DESTRUCTION);

    spello(SPELL_BANE, "bane", 80, 50, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, true,
           MAG_AFFECTS, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           "You feel more optimistic.",
           8, SCHOOL_ENCHANTMENT, DOMAIN_CHARM);

    spello(SPELL_DETECT_ALIGN, "detect alignment", 20, 10, 2, POS_STANDING,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "You feel less aware.",
           1, SCHOOL_DIVINATION, DOMAIN_UNIVERSAL);

    spello(SPELL_SEE_INVIS, "see invisibility", 20, 10, 2, POS_STANDING,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "Your eyes stop tingling.",
           2, SCHOOL_DIVINATION, DOMAIN_UNDEFINED);

    spello(SPELL_DETECT_MAGIC, "detect magic", 20, 10, 2, POS_STANDING,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "The detect magic wears off.",
           0, SCHOOL_UNIVERSAL, DOMAIN_UNIVERSAL);

    spello(SPELL_DETECT_POISON, "detect poison", 15, 5, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           "The detect poison wears off.",
           0, SCHOOL_DIVINATION, DOMAIN_UNIVERSAL);

    spello(SPELL_DISPEL_EVIL, "dispel evil", 40, 25, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           nullptr,
           5, SCHOOL_UNDEFINED, DOMAIN_GOOD);

    spello(SPELL_DISPEL_GOOD, "dispel good", 40, 25, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           nullptr,
           5, SCHOOL_UNDEFINED, DOMAIN_EVIL);

    spello(SPELL_EARTHQUAKE, "earthquake", 40, 25, 3, POS_FIGHTING,
           TAR_IGNORE, true, MAG_ACTION_FULL | MAG_AREAS, MAGSAVE_REFLEX | MAGSAVE_HALF, 0,
           nullptr,
           8, SCHOOL_UNDEFINED, DOMAIN_DESTRUCTION | DOMAIN_EARTH);

    spello(SPELL_ENCHANT_WEAPON, "enchant weapon", 150, 100, 10, POS_STANDING,
           TAR_OBJ_INV, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           9, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SPELL_ENERGY_DRAIN, "energy drain", 40, 25, 1, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true,
           MAG_DAMAGE | MAG_MANUAL, MAGSAVE_FORT | MAGSAVE_NONE, 0, nullptr,
           9, SCHOOL_NECROMANCY, DOMAIN_UNIVERSAL);

    spello(SPELL_GROUP_ARMOR, "group armor", 50, 30, 2, POS_STANDING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_GROUPS, 0, 0,
           nullptr,
           5, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_FAERIE_FIRE, "faerie fire", 20, 10, 2, POS_STANDING,
           TAR_CHAR_WORLD | TAR_NOT_SELF, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           1, SCHOOL_EVOCATION, DOMAIN_UNIVERSAL);

    spello(SPELL_FIREBALL, "fireball", 40, 30, 2, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_REFLEX | MAGSAVE_HALF, 0,
           nullptr,
           3, SCHOOL_EVOCATION, DOMAIN_UNDEFINED);

    spello(SPELL_MASS_HEAL, "mass heal", 80, 60, 5, POS_STANDING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_GROUPS, 0, 0,
           nullptr,
           6, SCHOOL_CONJURATION, DOMAIN_HEALING);

    spello(SPELL_HARM, "harm", 75, 45, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           nullptr,
           6, SCHOOL_UNDEFINED, DOMAIN_DESTRUCTION);

    spello(SPELL_HEAL, "heal", 60, 40, 3, POS_FIGHTING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_POINTS | MAG_UNAFFECTS, 0, 0,
           nullptr,
           6, SCHOOL_UNDEFINED, DOMAIN_HEALING);

    spello(SPELL_SENSU, "sensu", 1, 0, 3, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_POINTS, 0, 0,
           nullptr,
           6, SCHOOL_DIVINATION, DOMAIN_HEALING);

    spello(SPELL_IDENTIFY, "identify", 50, 25, 5, POS_STANDING,
           TAR_OBJ_INV | TAR_OBJ_ROOM, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           2, SCHOOL_DIVINATION, DOMAIN_MAGIC);

    spello(SPELL_DARKVISION, "darkvision", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "Your night vision seems to fade.",
           2, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_INVISIBLE, "invisibility", 35, 25, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS | MAG_ALTER_OBJS, 0, 0,
           "You feel yourself exposed.",
           2, SCHOOL_ILLUSION, DOMAIN_TRICKERY);

    spello(SPELL_LIGHTNING_BOLT, "lightning bolt", 30, 15, 1, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, MAGSAVE_REFLEX | MAGSAVE_HALF, 0,
           nullptr,
           3, SCHOOL_EVOCATION, DOMAIN_UNDEFINED);

    spello(SPELL_LOCATE_OBJECT, "locate object", 25, 20, 1, POS_STANDING,
           TAR_OBJ_WORLD, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           3, SCHOOL_DIVINATION, DOMAIN_TRAVEL);

    spello(SPELL_MAGIC_MISSILE, "magic missile", 25, 10, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, 0, 0,
           nullptr,
           1, SCHOOL_EVOCATION, DOMAIN_UNDEFINED);

    spello(SPELL_PARALYZE, "stone", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your body is no longer petrified.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_HASSHUKEN, "hasshuken", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your arms slow down.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_CURSE, "curse", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "You are no longer cursed!",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_MIGHT, "might", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your strength fades.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_PARALYZE, "paralyze", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your feel like you are able to move again.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_POISON, "poison", 50, 20, 3, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, true,
           MAG_ACTION_FULL | MAG_AFFECTS | MAG_ALTER_OBJS, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "You feel like you got over something.",
           4, SCHOOL_NECROMANCY, DOMAIN_UNIVERSAL);

    spello(SKILL_POISON, "dark metamorphosis", 50, 20, 3, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, true,
           MAG_ACTION_FULL | MAG_AFFECTS | MAG_ALTER_OBJS, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your dark metamorphosis fades.",
           4, SCHOOL_NECROMANCY, DOMAIN_UNIVERSAL);

    spello(SKILL_POISON, "healing glow", 50, 20, 3, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, true,
           MAG_ACTION_FULL | MAG_AFFECTS | MAG_ALTER_OBJS, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your healing glow fades.",
           4, SCHOOL_NECROMANCY, DOMAIN_UNIVERSAL);

    spello(SKILL_ENLIGHTEN, "enlighten", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "You feel less wise.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_GENIUS, "genius", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your thoughts feel less cohesive.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SKILL_FLEX, "flex", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "You feel less agile.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SPELL_PORTAL, "portal", 75, 75, 0, POS_STANDING,
           TAR_CHAR_WORLD | TAR_NOT_SELF, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           7, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_PROT_FROM_EVIL, "protection from evil", 40, 10, 3, POS_STANDING,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "You feel less protected.",
           1, SCHOOL_ABJURATION, DOMAIN_GOOD);

    spello(SPELL_REMOVE_CURSE, "remove curse", 45, 25, 5, POS_STANDING,
           TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, false,
           MAG_ACTION_FULL | MAG_UNAFFECTS | MAG_ALTER_OBJS, 0, 0,
           nullptr,
           3, SCHOOL_ABJURATION, DOMAIN_UNIVERSAL);

    spello(SPELL_NEUTRALIZE_POISON, "neutralize poison", 40, 8, 4, POS_STANDING,
           TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, false, MAG_ACTION_FULL | MAG_UNAFFECTS | MAG_ALTER_OBJS, 0, 0,
           nullptr,
           4, SCHOOL_CONJURATION, DOMAIN_UNIVERSAL);

    spello(SPELL_SANCTUARY, "sanctuary", 110, 85, 5, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "The white aura around your body fades.",
           9, SCHOOL_UNDEFINED, DOMAIN_PROTECTION);

    spello(SPELL_SENSE_LIFE, "sense life", 20, 10, 2, POS_STANDING,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "You feel less aware of your surroundings.",
           2, SCHOOL_DIVINATION, DOMAIN_UNIVERSAL);

    spello(SPELL_SHOCKING_GRASP, "shocking grasp", 30, 15, 3, POS_FIGHTING,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, true, MAG_ACTION_FULL | MAG_DAMAGE, 0, 0,
           nullptr,
           1, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SPELL_SLEEP, "sleep", 40, 25, 5, POS_FIGHTING,
           TAR_CHAR_ROOM, true, MAG_ACTION_FULL | MAG_AFFECTS, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           "You feel like you can wake up again.",
           1, SCHOOL_ENCHANTMENT, DOMAIN_UNDEFINED);

    spello(SPELL_HAYASA, "hayasa", 40, 25, 5, POS_FIGHTING,
           TAR_CHAR_ROOM, true, MAG_ACTION_FULL | MAG_AFFECTS, MAGSAVE_WILL | MAGSAVE_NONE, 0,
           "You feel your speed decrease as Hayasa fades.",
           1, SCHOOL_ENCHANTMENT, DOMAIN_UNDEFINED);

    spello(SKILL_TSKIN, "tough skin", 25, 10, 1, POS_STANDING,
           TAR_CHAR_ROOM | TAR_NOT_SELF, true, MAG_ACTION_FULL | MAG_AFFECTSV, MAGSAVE_FORT | MAGSAVE_NONE, 0,
           "Your skin isn't quite so thick anymore.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_UNDEFINED);

    spello(SPELL_BULL_STRENGTH, "bull strength", 35, 30, 1, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "You feel weaker.",
           2, SCHOOL_TRANSMUTATION, DOMAIN_STRENGTH);

    spello(SPELL_SUMMON, "summon", 75, 50, 3, POS_STANDING,
           TAR_CHAR_WORLD | TAR_NOT_SELF, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           7, SCHOOL_CONJURATION, DOMAIN_UNIVERSAL);

    spello(SPELL_TELEPORT, "teleport", 75, 50, 3, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           5, SCHOOL_TRANSMUTATION, DOMAIN_TRAVEL);

    spello(SPELL_WATERWALK, "waterwalk", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           "Your feet seem less buoyant.",
           3, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_WORD_OF_RECALL, "word of recall", 20, 10, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_MANUAL, 0, 0,
           nullptr,
           6, SCHOOL_UNDEFINED, DOMAIN_UNIVERSAL);

    spello(SPELL_RESISTANCE, "resistance", 40, 20, 0, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTSV,
           0, MAGCOMP_MATERIAL | MAGCOMP_SOMATIC | MAGCOMP_VERBAL | MAGCOMP_DIVINE_FOCUS,
           nullptr, 0, SCHOOL_ABJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_ACID_SPLASH, "acid splash", 40, 20, 0, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS,
           0, MAGCOMP_SOMATIC | MAGCOMP_VERBAL,
           nullptr,
           0, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_DAZE, "daze", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTSV,
           MAGSAVE_WILL | MAGSAVE_NONE, MAGCOMP_MATERIAL | MAGCOMP_SOMATIC | MAGCOMP_VERBAL,
           nullptr, 0, SCHOOL_ENCHANTMENT, DOMAIN_UNDEFINED);

    spello(SPELL_FLARE, "flare", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTSV,
           MAGSAVE_FORT | MAGSAVE_NONE, MAGCOMP_VERBAL,
           nullptr,
           0, SCHOOL_EVOCATION, DOMAIN_UNDEFINED);

    spello(SPELL_RAY_OF_FROST, "ray of frost", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS,
           0, MAGCOMP_SOMATIC | MAGCOMP_VERBAL,
           nullptr,
           0, SCHOOL_EVOCATION, DOMAIN_UNDEFINED);

    spello(SPELL_DISRUPT_UNDEAD, "disrupt undead", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS,
           0, MAGCOMP_SOMATIC | MAGCOMP_VERBAL,
           nullptr,
           0, SCHOOL_NECROMANCY, DOMAIN_UNDEFINED);

    /*spello(SPELL_LESSER_GLOBE_OF_INVUL, "lesser globe of invulnerability", 40, 20, 2, POS_STANDING,
      TAR_CHAR_ROOM, FALSE, MAG_ACTION_FULL | MAG_AFFECTS,
      0, MAGCOMP_MATERIAL | MAGCOMP_SOMATIC | MAGCOMP_VERBAL,
      nullptr,
      0, SCHOOL_ABJURATION, DOMAIN_UNDEFINED); */

    /*spello(SPELL_MINOR_CREATION, "minor creation", 40, 20, 2, POS_STANDING,
      TAR_CHAR_ROOM, FALSE, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
      nullptr,
      0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED); */

    spello(SPELL_SUMMON_MONSTER_I, "summon monster i", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           1, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_II, "summon monster ii", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           2, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_III, "summon monster iii", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           3, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_IV, "summon monster iv", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           4, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_V, "summon monster v", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           5, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_VI, "summon monster vi", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           6, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_VII, "summon monster vii", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           7, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_VIII, "summon monster viii", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           8, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_SUMMON_MONSTER_IX, "summon monster ix", 40, 20, 2, POS_FIGHTING,
           TAR_IGNORE, false, MAG_ACTION_FULL | MAG_SUMMONS, 0, 0,
           nullptr,
           9, SCHOOL_CONJURATION, DOMAIN_UNDEFINED);

    spello(SPELL_FIRE_SHIELD, "fire shield", 40, 20, 2, POS_FIGHTING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTSV, 0, 0,
           nullptr,
           4, SCHOOL_EVOCATION, DOMAIN_FIRE);

    spello(SPELL_ICE_STORM, "ice storm", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_SHOUT, "shout", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_FEAR, "fear", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_CLOUDKILL, "cloudkill", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_MAJOR_CREATION, "major creation", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_HOLD_MONSTER, "hold monster", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_CONE_OF_COLD, "cone of cold", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_ANIMAL_GROWTH, "animal growth", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_BALEFUL_POLYMORPH, "baleful polymorph", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);

    spello(SPELL_PASSWALL, "passwall", 40, 20, 2, POS_STANDING,
           TAR_CHAR_ROOM, false, MAG_ACTION_FULL | MAG_AFFECTS, 0, 0,
           nullptr,
           0, SCHOOL_UNDEFINED, DOMAIN_UNDEFINED);
    /*
     * These spells are currently not used, not implemented, and not castable.
     * Values for the 'breath' spells are filled in assuming a dragon's breath.
     */

    spello(SPELL_FIRE_BREATH, "fire breath", 0, 0, 0, POS_SITTING,
           TAR_IGNORE, true, 0, 0, 0,
           nullptr,
           0, 0, 0);

    spello(SPELL_GAS_BREATH, "gas breath", 0, 0, 0, POS_SITTING,
           TAR_IGNORE, true, 0, 0, 0,
           nullptr,
           0, 0, 0);

    spello(SPELL_FROST_BREATH, "frost breath", 0, 0, 0, POS_SITTING,
           TAR_IGNORE, true, 0, 0, 0,
           nullptr,
           0, 0, 0);

    spello(SPELL_ACID_BREATH, "acid breath", 0, 0, 0, POS_SITTING,
           TAR_IGNORE, true, 0, 0, 0,
           nullptr,
           0, 0, 0);

    spello(SPELL_LIGHTNING_BREATH, "lightning breath", 0, 0, 0, POS_SITTING,
           TAR_IGNORE, true, 0, 0, 0,
           nullptr,
           0, 0, 0);

    /* you might want to name this one something more fitting to your theme -Welcor*/
    spello(SPELL_DG_AFFECT, "Script-inflicted", 0, 0, 0, POS_SITTING,
           TAR_IGNORE, true, 0, 0, 0,
           nullptr,
           0, 0, 0);


    /*
     * Declaration of skills - this actually doesn't do anything except
     * set it up so that immortals can use these skills by default.  The
     * min level to use the skill for other classes is set up in class.c.
     */

    /*
     * skillo does spello and then marks the skill as a new style skill with
     * the appropriate flags.
     */
    /* Buff/Neg Skills */
    skillo(SKILL_FLEX, "flex", SKFLAG_CHAMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_GENIUS, "genius", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_ENLIGHTEN, "enlighten", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_TSKIN, "tough skin", SKFLAG_STRMOD | SKFLAG_ARMORALL);
    skillo(SKILL_KAIOKEN, "kaioken", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BLESS, "bless", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_CURSE, "curse", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_POISON, "poison", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_VIGOR, "vigor", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_POSE, "special pose", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_HASSHUKEN, "hasshuken", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);

    /* Effect Skills */
    skillo(SKILL_GARDENING, "gardening", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_EXTRACT, "extract", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_RUNIC, "runic", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_COMMUNE, "commune", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SOLARF, "solar flare", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_MIGHT, "might", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BALANCE, "balance", SKFLAG_DEXMOD | SKFLAG_ARMORALL);
    skillo(SKILL_BUILD, "build", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_CONCENTRATION, "concentration", SKFLAG_CONMOD);
    skillo(SKILL_SPOT, "spot", SKFLAG_WISMOD);
    skillo(SKILL_FIRST_AID, "first aid", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_DISGUISE, "disguise", SKFLAG_CHAMOD);
    skillo(SKILL_ESCAPE_ARTIST, "escape", SKFLAG_DEXMOD | SKFLAG_ARMORALL);
    skillo(SKILL_APPRAISE, "appraise", SKFLAG_INTMOD);
    skillo(SKILL_HEAL, "heal", SKFLAG_WISMOD | SKFLAG_ARMORBAD);
    skillo(SKILL_FORGERY, "forgery", SKFLAG_INTMOD);
    skillo(SKILL_HIDE, "hide", SKFLAG_DEXMOD | SKFLAG_ARMORALL);
    skillo(SKILL_LISTEN, "listen", SKFLAG_WISMOD);
    skillo(SKILL_EAVESDROP, "eavesdrop", SKFLAG_INTMOD);
    skillo(SKILL_CURE, "cure poison", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_OPEN_LOCK, "open lock", SKFLAG_DEXMOD | SKFLAG_NEEDTRAIN | SKFLAG_ARMORBAD);
    skillo(SKILL_REGENERATE, "regenerate", SKFLAG_CONMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_KEEN, "keen sight", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SEARCH, "search", SKFLAG_INTMOD);
    skillo(SKILL_MOVE_SILENTLY, "move silently", SKFLAG_DEXMOD | SKFLAG_ARMORALL);
    skillo(SKILL_ABSORB, "absorb", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SLEIGHT_OF_HAND, "sleight of hand", SKFLAG_DEXMOD | SKFLAG_ARMORALL);
    skillo(SKILL_INGEST, "ingest", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_REPAIR, "fix", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SENSE, "sense", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SURVIVAL, "survival", SKFLAG_WISMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_YOIK, "yoikominminken", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_CREATE, "create", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SPIT, "stone spit", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_POTENTIAL, "potential release", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_TELEPATHY, "telepathy", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_FOCUS, "focus", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_INSTANTT, "instant transmission", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);

    /* Weapon Skills */
    skillo(SKILL_SWORD, "sword", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_DAGGER, "dagger", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_CLUB, "club", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SPEAR, "spear", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_GUN, "gun", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BRAWL, "brawl", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);

    /* Defensive Skills */
    skillo(SKILL_DODGE, "dodge", SKFLAG_CHAMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_PARRY, "parry", SKFLAG_DEXMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BLOCK, "block", SKFLAG_DEXMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_ZANZOKEN, "zanzoken", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BARRIER, "barrier", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);

    /* Offensive Skills */
    skillo(SKILL_THROW, "throw", SKFLAG_DEXMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_PUNCH, "punch", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_KICK, "kick", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_ELBOW, "elbow", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_KNEE, "knee", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_ROUNDHOUSE, "roundhouse", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_UPPERCUT, "uppercut", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SLAM, "slam", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_HEELDROP, "heeldrop", SKFLAG_STRMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_KIBALL, "kiball", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_KIBLAST, "kiblast", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BEAM, "beam", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_TSUIHIDAN, "tsuihidan", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SHOGEKIHA, "shogekiha", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_RENZO, "renzokou energy dan", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_MASENKO, "masenko", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_DODONPA, "dodonpa", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_GALIKGUN, "galik gun", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_KAMEHAMEHA, "kamehameha", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_DEATHBEAM, "deathbeam", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_ERASER, "eraser cannon", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_TSLASH, "twin slash", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_PSYBLAST, "psychic blast", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_HONOO, "honoo", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_DUALBEAM, "dual beam", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_ROGAFUFUKEN, "rogafufuken", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_BAKUHATSUHA, "bakuhatsuha", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_KIENZAN, "kienzan", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_TRIBEAM, "tribeam", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_SBC, "special beam cannon", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_FINALFLASH, "final flash", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_CRUSHER, "crusher ball", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_DDSLASH, "darkness dragon slash", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_PBARRAGE, "psychic barrage", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_HELLFLASH, "hell flash", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_HELLSPEAR, "hell spear blast", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_KAKUSANHA, "kakusanha", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_SCATTER, "scatter shot", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_BIGBANG, "big bang", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_PSLASH, "phoenix slash", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_DEATHBALL, "deathball", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_SPIRITBALL, "spirit ball", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_GENKIDAMA, "genki dama", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER5);
    skillo(SKILL_GENOCIDE, "genocide", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER5);
    skillo(SKILL_DUALWIELD, "dual wield", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_TWOHAND, "twohand", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_STYLE, "fighting arts", SKFLAG_INTMOD);
    skillo(SKILL_KURA, "kuraiiro seiki", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_TAILWHIP, "tailwhip", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER1);
    skillo(SKILL_KOUSENGAN, "kousengan", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER1);
    skillo(SKILL_TAISHA, "taisha reiki", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_PARALYZE, "paralyze", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_INFUSE, "infuse", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_ROLL, "roll", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_TRIP, "trip", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_GRAPPLE, "grapple", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_WSPIKE, "water spikes", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_SELFD, "self destruct", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_SPIRAL, "spiral comet", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_BREAKER, "star breaker", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_MIMIC, "mimic", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_WRAZOR, "water razor", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_KOTEIRU, "koteiru bakuha", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_DIMIZU, "dimizu toride", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_HYOGA_KABE, "hyoga kabe", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_WELLSPRING, "wellspring", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_AQUA_BARRIER, "aqua barrier", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_WARP, "warp pool", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_HSPIRAL, "hell spiral", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_ARMOR, "nanite armor", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_FIRESHIELD, "fireshield", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_COOKING, "cooking", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_SEISHOU, "seishou enko", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER2);
    skillo(SKILL_SILK, "silk", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_BASH, "bash", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_HEADBUTT, "headbutt", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_ENSNARE, "ensnare", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_STARNOVA, "starnova", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_MALICE, "malice breaker", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_ZEN, "zen blade strike", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER3);
    skillo(SKILL_SUNDER, "sundering force", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_WITHER, "wither", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_METAMORPH, "dark metamorphosis", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_HAYASA, "hayasa", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_ENERGIZE, "energize throwing", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_PURSUIT, "pursuit", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_HEALGLOW, "healing glow", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_HANDLING, "handling", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_MYSTICMUSIC, "mystic music", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN);
    skillo(SKILL_LIGHTGRENADE, "light grenade", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_MULTIFORM, "multiform", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER1);
    skillo(SKILL_SPIRITCONTROL, "spirit control", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER1);
    skillo(SKILL_BALEFIRE, "balefire", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER4);
    skillo(SKILL_BLESSEDHAMMER, "blessed hammer", SKFLAG_INTMOD | SKFLAG_NEEDTRAIN | SKFLAG_TIER1);
}
