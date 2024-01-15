/**************************************************************************
*  File: dg_misc.c                                                        *
*  Usage: contains general functions for script usage.                    *
*                                                                         *
*  $Author: Mark A. Heilpern/egreen/Welcor $                              *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
**************************************************************************/

#include "dbat/structs.h"
#include "dbat/dg_scripts.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"
#include "dbat/dg_event.h"
#include "dbat/db.h"
#include "dbat/spells.h"
#include "dbat/constants.h"
#include "dbat/fight.h"

/* copied from spell_parser.c: */
#define SINFO spell_info[spellnum]


/* cast a spell; can be called by mobiles, objects and rooms, and no   */
/* level check is required. Note that mobs should generally use the    */
/* normal 'cast' command (which must be patched to allow mobs to cast  */
/* spells) as the spell system is designed to have a character caster, */
/* and this cast routine may overlook certain issues.                  */
/* LIMITATION: a target MUST exist for the spell unless the spell is   */
/* set to TAR_IGNORE. Also, group spells are not permitted             */
/* code borrowed from do_cast() */
void do_dg_cast(void *go, struct script_data *sc, trig_data *trig,
                int type, char *cmd) {
    struct char_data *caster = nullptr;
    struct char_data *tch = nullptr;
    struct obj_data *tobj = nullptr;
    struct room_data *caster_room = nullptr;
    char *s, *t;
    int spellnum, target = 0;
    char buf2[MAX_STRING_LENGTH], orig_cmd[MAX_INPUT_LENGTH];

    /* need to get the caster or the room of the temporary caster */
    switch (type) {
        case MOB_TRIGGER:
            caster = (struct char_data *) go;
            break;
        case WLD_TRIGGER:
            caster_room = (struct room_data *) go;
            break;
        case OBJ_TRIGGER:
            caster_room = dg_room_of_obj((struct obj_data *) go);
            if (!caster_room) {
                script_log("dg_do_cast: unknown room for object-caster!");
                return;
            }
            break;
        default:
            script_log("dg_do_cast: unknown trigger type!");
            return;
    }

    strcpy(orig_cmd, cmd);
    /* get: blank, spell name, target name */
    s = strtok(cmd, "'");
    if (s == nullptr) {
        script_log("Trigger: %s, VNum %d. dg_cast needs spell name.",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig));
        return;
    }
    s = strtok(nullptr, "'");
    if (s == nullptr) {
        script_log("Trigger: %s, VNum %d. dg_cast needs spell name in `'s.",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig));
        return;
    }
    t = strtok(nullptr, "\0");

    /* spellnum = search_block(s, spells, 0); */
    spellnum = find_skill_num(s, SKTYPE_SPELL);
    if ((spellnum < 1) || (spellnum >= SKILL_TABLE_SIZE || (!IS_SET(skill_type(spellnum), SKTYPE_SPELL)))) {
        script_log("Trigger: %s, VNum %d. dg_cast: invalid spell name (%s)",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), orig_cmd);
        return;
    }

    /* Find the target */
    if (t != nullptr) {
        one_argument(strcpy(buf2, t), t);
        skip_spaces(&t);
    }
    if (IS_SET(SINFO.targets, TAR_IGNORE)) {
        target = true;
    } else if (t != nullptr && *t) {
        if (!target &&
            (IS_SET(SINFO.targets, TAR_CHAR_ROOM) ||
             IS_SET(SINFO.targets, TAR_CHAR_WORLD))) {
            if ((tch = get_char(t)) != nullptr)
                target = true;
        }

        if (!target &&
            (IS_SET(SINFO.targets, TAR_OBJ_INV) ||
             IS_SET(SINFO.targets, TAR_OBJ_EQUIP) ||
             IS_SET(SINFO.targets, TAR_OBJ_ROOM) ||
             IS_SET(SINFO.targets, TAR_OBJ_WORLD))) {
            if ((tobj = get_obj(t)) != nullptr)
                target = true;
        }

        if (!target) {
            script_log("Trigger: %s, VNum %d. dg_cast: target not found (%s)",
                       GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), orig_cmd);
            return;
        }
    }

    if (IS_SET(SINFO.routines, MAG_GROUPS)) {
        script_log("Trigger: %s, VNum %d. dg_cast: group spells not permitted (%s)",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), orig_cmd);
        return;
    }

    if (!caster) {
        caster = read_mobile(DG_CASTER_PROXY, VIRTUAL);
        if (!caster) {
            script_log("dg_cast: Cannot load the caster mob!");
            return;
        }
        /* set the caster's name to that of the object, or the gods.... */
        if (type == OBJ_TRIGGER)
            caster->short_description =
                    strdup(((struct obj_data *) go)->short_description);
        else if (type == WLD_TRIGGER)
            caster->short_description = strdup("The gods");
        caster->next_in_room = caster_room->people;
        caster_room->people = caster;
        IN_ROOM(caster) = real_room(caster_room->vn);
        call_magic(caster, tch, tobj, spellnum, DG_SPELL_LEVEL, CAST_SPELL, t);
        extract_char(caster);
    } else
        call_magic(caster, tch, tobj, spellnum, GET_LEVEL(caster), CAST_SPELL, t);
}


/* modify an affection on the target. affections can be of the AFF_x  */
/* variety or APPLY_x type. APPLY_x's have an integer value for them  */
/* while AFF_x's have boolean values. In any case, the duration MUST  */
/* be non-zero.                                                       */
/* usage:  apply <target> <property> <value> <duration>               */
#define APPLY_TYPE    1
#define AFFECT_TYPE    2

void do_dg_affect(void *go, struct script_data *sc, trig_data *trig, int script_type, char *cmd) {
    struct char_data *ch = nullptr;
    int value = 0, duration = 0;
    char junk[MAX_INPUT_LENGTH]; /* will be set to "dg_affect" */
    char charname[MAX_INPUT_LENGTH], property[MAX_INPUT_LENGTH];
    char value_p[MAX_INPUT_LENGTH], duration_p[MAX_INPUT_LENGTH];
    int i = 0, type = 0;
    struct affected_type af;


    half_chop(cmd, junk, cmd);
    half_chop(cmd, charname, cmd);
    half_chop(cmd, property, cmd);
    half_chop(cmd, value_p, duration_p);

    /* make sure all parameters are present */
    if (charname == nullptr || !*charname || property == nullptr || !*property ||
        value_p == nullptr || !*value_p || duration_p == nullptr || !*duration_p) {
        script_log("Trigger: %s, VNum %d. dg_affect usage: <target> <property> <value> <duration>",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig));
        return;
    }

    value = atoi(value_p);
    duration = atoi(duration_p);
    if (duration <= 0) {
        script_log("Trigger: %s, VNum %d. dg_affect: need positive duration!",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig));
        script_log("Line was: dg_affect %s %s %s %s (%d)",
                   charname, property, value_p, duration_p, duration);
        return;
    }

    /* find the property -- first search apply_types */
    i = 0;
    while (strcasecmp(apply_types[i], "\n")) {
        if (!strcasecmp(apply_types[i], property)) {
            type = APPLY_TYPE;
            break;
        }
        i++;
    }

    if (!type) { /* search affect_types now */
        i = 0;
        while (strcasecmp(affected_bits[i], "\n")) {
            if (!strcasecmp(affected_bits[i], property)) {
                type = AFFECT_TYPE;
                break;
            }
            i++;
        }
    }

    if (!type) { /* property not found */
        script_log("Trigger: %s, VNum %d. dg_affect: unknown property '%s'!",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), property);
        return;
    }


    /* locate the target */
    ch = get_char(charname);
    if (!ch) {
        script_log("Trigger: %s, VNum %d. dg_affect: cannot locate target!",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig));
        return;
    }

    if (!strcasecmp(value_p, "off")) {
        affect_from_char(ch, SPELL_DG_AFFECT);
        return;
    }

    /* add the affect */
    af.type = SPELL_DG_AFFECT;
    af.duration = duration - 1;
    af.modifier = value;

    if (type == APPLY_TYPE) {
        af.location = i;
        af.bitvector = 0;
    } else {
        af.location = 0;
        af.bitvector = i;
    }

    affect_to_char(ch, &af);
}

void send_char_pos(struct char_data *ch, int dam) {
    switch (GET_POS(ch)) {
        case POS_MORTALLYW:
            act("$n is mortally wounded, and will die soon, if not aided.", true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "You are mortally wounded, and will die soon, if not aided.\r\n");
            break;
        case POS_INCAP:
            act("$n is incapacitated and will slowly die, if not aided.", true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "You are incapacitated and will slowly die, if not aided.\r\n");
            break;
        case POS_STUNNED:
            act("$n is stunned, but will probably regain consciousness again.", true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "You're stunned, but will probably regain consciousness again.\r\n");
            break;
        case POS_DEAD:
            act("$n is dead!  R.I.P.", false, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "You are dead!  Sorry...\r\n");
            break;
        default:                        /* >= POSITION SLEEPING */
            if (dam > (GET_MAX_HIT(ch) >> 2))
                act("That really did HURT!", false, ch, nullptr, nullptr, TO_CHAR);
            if (GET_HIT(ch) < (GET_MAX_HIT(ch) >> 2))
                send_to_char(ch, "@rYou wish that your wounds would stop BLEEDING so much!@n\r\n");
    }
}


/* Used throughout the xxxcmds.c files for checking if a char
 * can be targetted
 * - allow_gods is false when called by %force%, for instance,
 * while true for %teleport%.  -- Welcor
 */
int valid_dg_target(struct char_data *ch, int bitvector) {
    if (IS_NPC(ch))
        return true;  /* all npcs are allowed as targets */
    else if (GET_ADMLEVEL(ch) < ADMLVL_IMMORT)
        return true;  /* as well as all mortals */
    else if (!IS_SET(bitvector, DG_ALLOW_GODS) &&
             (GET_ADMLEVEL(ch) >= 2 &&
              !PRF_FLAGGED(ch, PRF_TEST))) /* LVL_GOD has the advance command. Can't allow them to be forced. */
        return false; /* but not always the highest gods */
    else if (!PRF_FLAGGED(ch, PRF_NOHASSLE) || PRF_FLAGGED(ch, PRF_TEST))
        return true;  /* the ones in between as allowed as long as they have no-hassle off.   */
    else
        return false;  /* The rest are gods with nohassle on... */
}


void script_damage(struct char_data *vict, int dam) {
    if (ADM_FLAGGED(vict, ADM_NODAMAGE) && (dam > 0)) {
        send_to_char(vict, "Being the cool immortal you are, you sidestep a trap, "
                           "obviously placed to kill you.\r\n");
        return;
    }

    vict->decCurHealth(dam);

    update_pos(vict);
    send_char_pos(vict, dam);

    if (GET_POS(vict) == POS_DEAD) {
        if (!IS_NPC(vict))
            mudlog(BRF, 0, true, "%s killed by script at %s",
                   GET_NAME(vict), vict->getRoom()->name);
        die(vict, nullptr);
    }
}

