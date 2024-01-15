/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/spells.h"
#include "dbat/handler.h"
#include "dbat/db.h"
#include "dbat/constants.h"
#include "dbat/interpreter.h"
#include "dbat/dg_scripts.h"
#include "dbat/feats.h"
#include "dbat/oasis.h"
#include "dbat/config.h"
#include "dbat/act.item.h"
#include "dbat/act.movement.h"
#include "dbat/races.h"
#include "dbat/act.informative.h"

/* external variables */

/*
 * Special spells appear below.
 */

ASPELL(spell_create_water) {
    int water;

    if (ch == nullptr || obj == nullptr)
        return;
    /* level = MAX(MIN(level, LVL_IMPL), 1);	 - not used */

    if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
        if ((GET_OBJ_VAL(obj, VAL_DRINKCON_LIQUID) != LIQ_WATER) && (GET_OBJ_VAL(obj, VAL_DRINKCON_HOWFULL) != 0)) {
            name_from_drinkcon(obj);
            GET_OBJ_VAL(obj, VAL_DRINKCON_LIQUID) = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);
        } else {
            water = MAX(GET_OBJ_VAL(obj, VAL_DRINKCON_CAPACITY) - GET_OBJ_VAL(obj, VAL_DRINKCON_HOWFULL), 0);
            if (water > 0) {
                if (GET_OBJ_VAL(obj, VAL_DRINKCON_HOWFULL) >= 0)
                    name_from_drinkcon(obj);
                GET_OBJ_VAL(obj, VAL_DRINKCON_LIQUID) = LIQ_WATER;
                GET_OBJ_VAL(obj, VAL_DRINKCON_HOWFULL) += water;
                name_to_drinkcon(obj, LIQ_WATER);
                weight_change_object(obj, water);
                act("$p is filled.", false, ch, obj, nullptr, TO_CHAR);
            }
        }
    }
}


ASPELL(spell_recall) {
    if (victim == nullptr || IS_NPC(victim))
        return;

    act("$n disappears.", true, victim, nullptr, nullptr, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, real_room(CONFIG_MORTAL_START));
    act("$n appears in the middle of the room.", true, victim, nullptr, nullptr, TO_ROOM);
    look_at_room(IN_ROOM(victim), victim, 0);
    entry_memory_mtrigger(victim);
    greet_mtrigger(victim, -1);
    greet_memory_mtrigger(victim);
}


ASPELL(spell_teleport) {
    room_rnum to_room;

    if (victim == nullptr || IS_NPC(victim))
        return;

    do {
        to_room = rand_number(0, world.size());
    } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_GODROOM));

    act("$n slowly fades out of existence and is gone.",
        false, victim, nullptr, nullptr, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, to_room);
    act("$n slowly fades into existence.", false, victim, nullptr, nullptr, TO_ROOM);
    look_at_room(IN_ROOM(victim), victim, 0);
    entry_memory_mtrigger(victim);
    greet_mtrigger(victim, -1);
    greet_memory_mtrigger(victim);

}

#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon) {
    if (ch == nullptr || victim == nullptr)
        return;

    if (GET_LEVEL(victim) > level + 3) {
        send_to_char(ch, "%s", SUMMON_FAIL);
        return;
    }

    if (!CONFIG_PK_ALLOWED) {
        if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
            act("As the words escape your lips and $N travels\r\n"
                "through time and space towards you, you realize that $E is\r\n"
                "aggressive and might harm you, so you wisely send $M back.",
                false, ch, nullptr, victim, TO_CHAR);
            return;
        }
        if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE) &&
            !PLR_FLAGGED(victim, PLR_KILLER)) {
            send_to_char(victim, "%s just tried to summon you to: %s.\r\n"
                                 "%s failed because you have summon protection on.\r\n"
                                 "Type NOSUMMON to allow other players to summon you.\r\n",
                         GET_NAME(ch), ch->getRoom()->name,
                         (GET_SEX(ch) == SEX_MALE) ? "He" : "She");

            send_to_char(ch, "You failed because %s has summon protection on.\r\n", GET_NAME(victim));
            mudlog(BRF, ADMLVL_IMMORT, true, "%s failed summoning %s to %s.", GET_NAME(ch), GET_NAME(victim),
                   ch->getRoom()->name);
            return;
        }
    }

    if (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
        (IS_NPC(victim) && mag_newsaves(ch, victim, SPELL_SUMMON, level, GET_INT(ch)))) {
        send_to_char(ch, "%s", SUMMON_FAIL);
        return;
    }

    act("$n disappears suddenly.", true, victim, nullptr, nullptr, TO_ROOM);

    char_from_room(victim);
    char_to_room(victim, IN_ROOM(ch));

    act("$n arrives suddenly.", true, victim, nullptr, nullptr, TO_ROOM);
    act("$n has summoned you!", false, ch, nullptr, victim, TO_VICT);
    look_at_room(IN_ROOM(victim), victim, 0);
    entry_memory_mtrigger(victim);
    greet_mtrigger(victim, -1);
    greet_memory_mtrigger(victim);

}


ASPELL(spell_locate_object) {
    struct obj_data *i;
    char name[MAX_INPUT_LENGTH];
    int j;

    /*
     * FIXME: This is broken.  The spell parser routines took the argument
     * the player gave to the spell and located an object with that keyword.
     * Since we're passed the object and not the keyword we can only guess
     * at what the player originally meant to search for. -gg
     */
    if (!obj) {
        send_to_char(ch, "You sense nothing.\r\n");
        return;
    }

    strlcpy(name, fname(obj->name), sizeof(name));
    j = level / 2;

    for (i = object_list; i && (j > 0); i = i->next) {
        if (!isname(name, i->name))
            continue;

        send_to_char(ch, "%c%s", UPPER(*i->short_description), (i->short_description) + 1);

        if (i->carried_by)
            send_to_char(ch, " is being carried by %s.\r\n", PERS(i->carried_by, ch));
        else if (IN_ROOM(i) != NOWHERE)
            send_to_char(ch, " is in %s.\r\n", i->getRoom()->name);
        else if (i->in_obj)
            send_to_char(ch, " is in %s.\r\n", i->in_obj->short_description);
        else if (i->worn_by)
            send_to_char(ch, " is being worn by %s.\r\n", PERS(i->worn_by, ch));
        else
            send_to_char(ch, "'s location is uncertain.\r\n");

        j--;
    }

    if (j == level / 2)
        send_to_char(ch, "You sense nothing.\r\n");
}


ASPELL(spell_charm) {
    struct affected_type af;

    if (victim == nullptr || ch == nullptr)
        return;

    if (victim == ch)
        send_to_char(ch, "You like yourself even better!\r\n");
    else if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE))
        send_to_char(ch, "You fail because SUMMON protection is on!\r\n");
    else if (AFF_FLAGGED(victim, AFF_SANCTUARY))
        send_to_char(ch, "Your victim is protected by sanctuary!\r\n");
    else if (MOB_FLAGGED(victim, MOB_NOCHARM))
        send_to_char(ch, "Your victim resists!\r\n");
    else if (AFF_FLAGGED(ch, AFF_CHARM))
        send_to_char(ch, "You can't have any followers of your own!\r\n");
    else if (AFF_FLAGGED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
        send_to_char(ch, "You fail.\r\n");
        /* player charming another player - no legal reason for this */
    else if (!CONFIG_PK_ALLOWED && !IS_NPC(victim))
        send_to_char(ch, "You fail - shouldn't be doing it anyway.\r\n");
    else if (IS_SAIYAN(victim) && rand_number(1, 100) <= 90)
        send_to_char(ch, "Your victim resists!\r\n");
    else if (circle_follow(victim, ch))
        send_to_char(ch, "Sorry, following in circles cannot be allowed.\r\n");
    else if (mag_newsaves(ch, victim, SPELL_CHARM, level, GET_INT(ch)))
        send_to_char(ch, "Your victim resists!\r\n");
    else {
        if (victim->master)
            stop_follower(victim);

        add_follower(victim, ch);
        victim->master_id = GET_IDNUM(ch);

        af.type = SPELL_CHARM;
        af.duration = 24 * 2;
        if (GET_CHA(ch))
            af.duration *= GET_CHA(ch);
        if (GET_INT(victim))
            af.duration /= GET_INT(victim);
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", false, ch, nullptr, victim, TO_VICT);
        if (IS_NPC(victim))
            victim->mobFlags.reset(MOB_SPEC);

    }
}


ASPELL(spell_identify) {
    int i, found;
    size_t len;

    if (obj) {
        char bitbuf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];

        sprinttype(GET_OBJ_TYPE(obj), item_types, bitbuf, sizeof(bitbuf));
        send_to_char(ch, "You feel informed:\r\nObject '%s', Item type: %s\r\n", obj->short_description, bitbuf);

        if (obj->bitvector.any()) {
            sprintbitarray(GET_OBJ_PERM(obj), affected_bits, AF_ARRAY_MAX, bitbuf);
            send_to_char(ch, "Item will give you following abilities:  %s\r\n", bitbuf);
        }

        sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, bitbuf);
        send_to_char(ch, "Item is: %s\r\n", bitbuf);

        send_to_char(ch, "Weight: %" I64T ", Value: %d, Rent: %d, Min Level: %d\r\n", GET_OBJ_WEIGHT(obj),
                     GET_OBJ_COST(obj), GET_OBJ_RENT(obj), GET_OBJ_LEVEL(obj));

        switch (GET_OBJ_TYPE(obj)) {
            case ITEM_SCROLL:
            case ITEM_POTION:
                len = i = 0;

                if (GET_OBJ_VAL(obj, VAL_SCROLL_SPELL1) >= 1) {
                    i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s",
                                 skill_name(GET_OBJ_VAL(obj, VAL_SCROLL_SPELL1)));
                    if (i >= 0)
                        len += i;
                }

                if (GET_OBJ_VAL(obj, VAL_SCROLL_SPELL2) >= 1 && len < sizeof(bitbuf)) {
                    i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s",
                                 skill_name(GET_OBJ_VAL(obj, VAL_SCROLL_SPELL2)));
                    if (i >= 0)
                        len += i;
                }

                if (GET_OBJ_VAL(obj, VAL_SCROLL_SPELL3) >= 1 && len < sizeof(bitbuf)) {
                    i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s",
                                 skill_name(GET_OBJ_VAL(obj, VAL_SCROLL_SPELL3)));
                    if (i >= 0)
                        len += i;
                }

                send_to_char(ch, "This %s casts: %s\r\n", item_types[(int) GET_OBJ_TYPE(obj)], bitbuf);
                break;
            case ITEM_WAND:
            case ITEM_STAFF:
                send_to_char(ch, "This %s casts: %s\r\nIt has %d maximum charge%s and %d remaining.\r\n",
                             item_types[(int) GET_OBJ_TYPE(obj)], skill_name(GET_OBJ_VAL(obj, VAL_WAND_SPELL)),
                             GET_OBJ_VAL(obj, VAL_WAND_MAXCHARGES),
                             GET_OBJ_VAL(obj, VAL_WAND_MAXCHARGES) == 1 ? "" : "s", GET_OBJ_VAL(obj, VAL_WAND_CHARGES));
                break;
            case ITEM_WEAPON:
                send_to_char(ch, "Damage Dice is '%dD%d' for an average per-round damage of %.1f.\r\n",
                             GET_OBJ_VAL(obj, VAL_WEAPON_DAMDICE), GET_OBJ_VAL(obj, VAL_WEAPON_DAMSIZE),
                             ((GET_OBJ_VAL(obj, VAL_WEAPON_DAMSIZE) + 1) / 2.0) * GET_OBJ_VAL(obj, VAL_WEAPON_DAMDICE));
                break;
            case ITEM_ARMOR:
                send_to_char(ch, "AC-apply is %.1f\r\n", ((float) GET_OBJ_VAL(obj, VAL_ARMOR_APPLYAC)) / 10);
                break;
        }
        found = false;
        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if ((obj->affected[i].location != APPLY_NONE) &&
                (obj->affected[i].modifier != 0)) {
                if (!found) {
                    send_to_char(ch, "Can affect you as :\r\n");
                    found = true;
                }
                sprinttype(obj->affected[i].location, apply_types, bitbuf, sizeof(bitbuf));
                switch (obj->affected[i].location) {
                    case APPLY_FEAT:
                        snprintf(buf2, sizeof(buf2), " (%s)", feat_list[obj->affected[i].specific].name);
                        break;
                    case APPLY_SKILL:
                        snprintf(buf2, sizeof(buf2), " (%s)", spell_info[obj->affected[i].specific].name);
                        break;
                    default:
                        buf2[0] = 0;
                        break;
                }
                send_to_char(ch, "   Affects: %s%s By %d\r\n", bitbuf, buf2,
                             obj->affected[i].modifier);
            }
        }
    }
}



/*
 * Cannot use this spell on an equipped object or it will mess up the
 * wielding character's hit/dam totals.
 */
ASPELL(spell_enchant_weapon) {
    int i;

    if (ch == nullptr || obj == nullptr)
        return;

    /* Either already enchanted or not a weapon. */
    if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || OBJ_FLAGGED(obj, ITEM_MAGIC))
        return;

    /* Make sure no other affections. */
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
        if (obj->affected[i].location != APPLY_NONE)
            return;

    obj->extra_flags.set(ITEM_MAGIC);

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (obj->affected[i].location == APPLY_NONE) {
            obj->affected[i].location = APPLY_ACCURACY;
            obj->affected[i].modifier = 1 + (level >= 18);
            break;
        }
    }

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (obj->affected[i].location == APPLY_NONE) {
            obj->affected[i].location = APPLY_DAMAGE;
            obj->affected[i].modifier = 1 + (level >= 20);
            break;
        }
    }

    if (IS_GOOD(ch)) {
        obj->extra_flags.set(ITEM_ANTI_EVIL);
        act("$p glows blue.", false, ch, obj, nullptr, TO_CHAR);
    } else if (IS_EVIL(ch)) {
        obj->extra_flags.set(ITEM_ANTI_GOOD);
        act("$p glows red.", false, ch, obj, nullptr, TO_CHAR);
    } else
        act("$p glows yellow.", false, ch, obj, nullptr, TO_CHAR);
}


ASPELL(spell_detect_poison) {
    if (victim) {
        if (victim == ch) {
            if (AFF_FLAGGED(victim, AFF_POISON))
                send_to_char(ch, "You can sense poison in your blood.\r\n");
            else
                send_to_char(ch, "You feel healthy.\r\n");
        } else {
            if (AFF_FLAGGED(victim, AFF_POISON))
                act("You sense that $E is poisoned.", false, ch, nullptr, victim, TO_CHAR);
            else
                act("You sense that $E is healthy.", false, ch, nullptr, victim, TO_CHAR);
        }
    }

    if (obj) {
        switch (GET_OBJ_TYPE(obj)) {
            case ITEM_DRINKCON:
            case ITEM_FOUNTAIN:
            case ITEM_FOOD:
                if (GET_OBJ_VAL(obj, VAL_FOOD_POISON))
                    act("You sense that $p has been contaminated.", false, ch, obj, nullptr, TO_CHAR);
                else
                    act("You sense that $p is safe for consumption.", false, ch, obj, nullptr,
                        TO_CHAR);
                break;
            default:
                send_to_char(ch, "You sense that it should not be consumed.\r\n");
        }
    }
}

ASPELL(spell_portal) {
    struct obj_data *portal, *tportal;
    struct room_data *rm;
    rm = victim->getRoom();

    if (ch == nullptr || victim == nullptr)
        return;

    if (!can_edit_zone(ch, rm->zone) && ZONE_FLAGGED(rm->zone, ZONE_QUEST)) {
        send_to_char(ch, "That target is in a quest zone.\r\n");
        return;
    }

    if (ZONE_FLAGGED(rm->zone, ZONE_CLOSED) && GET_ADMLEVEL(ch) < ADMLVL_IMMORT) {
        send_to_char(ch, "That target is in a closed zone.\r\n");
        return;
    }

    if (ZONE_FLAGGED(rm->zone, ZONE_NOIMMORT) && GET_ADMLEVEL(ch) < ADMLVL_GRGOD) {
        send_to_char(ch, "That target is in a zone closed to all.\r\n");
        return;
    }

    /* create the portal */
    portal = read_object(portal_object, VIRTUAL);
    GET_OBJ_VAL(portal, VAL_PORTAL_DEST) = GET_ROOM_VNUM(IN_ROOM(victim));
    GET_OBJ_VAL(portal, VAL_PORTAL_HEALTH) = 100;
    GET_OBJ_VAL(portal, VAL_PORTAL_MAXHEALTH) = 100;
    GET_OBJ_TIMER(portal) = (int) (level / 10);
    obj_to_room(portal, IN_ROOM(ch));
    act("$n opens a portal in thin air.",
        true, ch, nullptr, nullptr, TO_ROOM);
    act("You open a portal out of thin air.",
        true, ch, nullptr, nullptr, TO_CHAR);
    /* create the portal at the other end */
    tportal = read_object(portal_object, VIRTUAL);
    GET_OBJ_VAL(tportal, VAL_PORTAL_DEST) = GET_ROOM_VNUM(IN_ROOM(ch));
    GET_OBJ_VAL(tportal, VAL_PORTAL_HEALTH) = 100;
    GET_OBJ_VAL(tportal, VAL_PORTAL_MAXHEALTH) = 100;
    GET_OBJ_TIMER(tportal) = (int) (level / 10);
    obj_to_room(tportal, IN_ROOM(victim));
    act("A shimmering portal appears out of thin air.",
        true, victim, nullptr, nullptr, TO_ROOM);
    act("A shimmering portal opens here for you.",
        true, victim, nullptr, nullptr, TO_CHAR);
}


ASPELL(art_abundant_step) {
    int steps, i = 0, j, rep, max;
    room_rnum r, nextroom;
    char buf[MAX_INPUT_LENGTH], tc;
    const char *p;

    steps = 0;
    r = IN_ROOM(ch);
    p = arg;

    while (p && *p && !isdigit(*p) && !isalpha(*p)) p++;

    if (!p || !*p) {
        send_to_char(ch, "You must give directions from your current location. Examples:\r\n"
                         "  w w nw n e\r\n"
                         "  2w nw n e\r\n");
        return;
    }

    while (*p) {
        while (*p && !isdigit(*p) && !isalpha(*p)) p++;
        if (isdigit(*p)) {
            rep = atoi(p);
            while (isdigit(*p)) p++;
        } else
            rep = 1;
        if (isalpha(*p)) {
            for (i = 0; isalpha(*p); i++, p++) buf[i] = LOWER(*p);
            j = i;
            tc = buf[i];
            buf[i] = 0;
            for (i = 1;
                 complete_cmd_info[i].command_pointer == do_move && strcmp(complete_cmd_info[i].sort_as, buf); i++);
            if (complete_cmd_info[i].command_pointer == do_move) {
                i = complete_cmd_info[i].subcmd - 1;
            } else
                i = -1;
            buf[j] = tc;
        }
        if (i > -1)
            while (rep--) {
                if (++steps > max)
                    break;
                if (!W_EXIT(r, i)) {
                    send_to_char(ch, "Invalid step. Skipping.\r\n");
                    break;
                }
                nextroom = W_EXIT(r, i)->to_room;
                if (nextroom == NOWHERE)
                    break;
                r = nextroom;
            }
        if (steps > max)
            break;
    }
    send_to_char(ch, "Your will bends reality as you travel through the ethereal plane.\r\n");
    act("$n is suddenly absent.", true, ch, nullptr, nullptr, TO_ROOM);

    char_from_room(ch);
    char_to_room(ch, r);

    act("$n is suddenly present.", true, ch, nullptr, nullptr, TO_ROOM);

    look_at_room(IN_ROOM(ch), ch, 0);

    return;
}


int roll_skill(struct char_data *ch, int snum) {
    int roll, skval, i;
    if (!IS_NPC(ch)) {
        skval = GET_SKILL(ch, snum);
        if (SKILL_SPOT == snum) {
            if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 4 || GET_GENOME(ch, 1) == 4)) {
                skval += 5;
            }
        } else if (SKILL_HIDE == snum) {
            if (AFF_FLAGGED(ch, AFF_LIQUEFIED)) {
                skval += 5;
            } else if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 5 || GET_GENOME(ch, 1) == 5)) {
                skval += 10;
            }
        }
    } else if (IS_NPC(ch)) {
        int numb = 0;
        if (GET_LEVEL(ch) <= 10) {
            numb = rand_number(15, 30);
        }
        if (GET_LEVEL(ch) <= 20) {
            numb = rand_number(20, 40);
        }
        if (GET_LEVEL(ch) <= 30) {
            numb = rand_number(40, 60);
        }
        if (GET_LEVEL(ch) <= 60) {
            numb = rand_number(60, 80);
        }
        if (GET_LEVEL(ch) <= 80) {
            numb = rand_number(70, 90);
        }
        if (GET_LEVEL(ch) <= 90) {
            numb = rand_number(80, 95);
        }
        if (GET_LEVEL(ch) <= 100) {
            numb = rand_number(90, 100);
        }
        skval = numb;
    }
    if (snum == SKILL_SPOT && GET_SKILL(ch, SKILL_LISTEN)) {
        skval += GET_SKILL(ch, SKILL_LISTEN) / 10;
    }
    if (snum < 0 || snum >= SKILL_TABLE_SIZE)
        return 0;
    if (IS_SET(spell_info[snum].skilltype, SKTYPE_SPELL)) {
        /*
         * There's no real roll for a spell to succeed, so instead we will
         * return the spell resistance roll; the defender must have resistance
         * higher than this roll to avoid it. Most spells should also have some
         * kind of save called after roll_skill.
         */

        return roll + rand_number(1, 20);
    } else if (IS_SET(spell_info[snum].skilltype, SKTYPE_SKILL)) {
        if (!skval && IS_SET(spell_info[snum].flags, SKFLAG_NEEDTRAIN)) {
            return -1;
        } else {
            roll = skval;
            if (IS_SET(spell_info[snum].flags, SKFLAG_STRMOD))
                roll += ability_mod_value(GET_STR(ch));
            if (IS_SET(spell_info[snum].flags, SKFLAG_DEXMOD))
                roll += dex_mod_capped(ch);
            if (IS_SET(spell_info[snum].flags, SKFLAG_CONMOD))
                roll += ability_mod_value(GET_CON(ch));
            if (IS_SET(spell_info[snum].flags, SKFLAG_INTMOD))
                roll += ability_mod_value(GET_INT(ch));
            if (IS_SET(spell_info[snum].flags, SKFLAG_WISMOD))
                roll += ability_mod_value(GET_WIS(ch));
            if (IS_SET(spell_info[snum].flags, SKFLAG_CHAMOD))
                roll += ability_mod_value(GET_CHA(ch));
            if (IS_SET(spell_info[snum].flags, SKFLAG_ARMORALL))
                roll -= GET_ARMORCHECKALL(ch);
            else if (IS_SET(spell_info[snum].flags, SKFLAG_ARMORBAD))
                roll -= GET_ARMORCHECK(ch);
            return roll + rand_number(1, 20);
        }
    } else {
        basic_mud_log("Trying to roll uncategorized skill/spell #%d for %s", snum, GET_NAME(ch));
        return 0;
    }
}

int roll_resisted(struct char_data *actor, int sact, struct char_data *resistor, int sres) {
    return roll_skill(actor, sact) >= roll_skill(resistor, sres);
}
