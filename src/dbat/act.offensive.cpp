/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/act.offensive.h"
#include "dbat/interpreter.h"
#include "dbat/combat.h"
#include "dbat/comm.h"
#include "dbat/spells.h"
#include "dbat/utils.h"
#include "dbat/handler.h"
#include "dbat/constants.h"
#include "dbat/fight.h"
#include "dbat/guild.h"
#include "dbat/class.h"
#include "dbat/techniques.h"
#include "dbat/attack.h"

/* Combat commands below this line */

ACMD(do_galikgun) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .15, minimum = .1;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_GALIKGUN)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_GALIKGUN); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);

    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_GALIKGUN, 0);
        index = check_def(vict); /* Check parry/block/dodge of vict */
        prob = roll_accuracy(ch, skill, true);

        if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 2) {
            prob += 5;
        }

        perc = chance_to_hit(ch);
        index -= handle_speed(ch, vict);
        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Galik Gun")) {
            if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your Galik Gun!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Galik Gun!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Galik Gun!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 16, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your Galik Gun, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Galik Gun, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Galik Gun, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 16, skill, SKILL_GALIKGUN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Galik Gun misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Galik Gun at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Galik Gun at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Galik Gun misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Galik Gun at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Galik Gun at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 16, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou take your charged ki and form a sparkling purple shroud of energy around your body! You swing your arms towards @c$N@W with your palms flatly facing $M and shout '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around your body forms into a beam and crashes into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards you with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into your body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards @c$N@W with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou take your charged ki and form a sparkling purple shroud of energy around your body! You swing your arms towards @c$N@W with your palms flatly facing $M and shout '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around your body forms into a beam and crashes into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards you with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into your face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards @c$N@W with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou take your charged ki and form a sparkling purple shroud of energy around your body! You swing your arms towards @c$N@W with your palms flatly facing $M and shout '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around your body forms into a beam and crashes into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards you with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into your gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards @c$N@W with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou take your charged ki and form a sparkling purple shroud of energy around your body! You swing your arms towards @c$N@W with your palms flatly facing $M and shout '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around your body forms into a beam and crashes into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards you with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards @c$N@W with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou take your charged ki and form a sparkling purple shroud of energy around your body! You swing your arms towards @c$N@W with your palms flatly facing $M and shout '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around your body forms into a beam and crashes into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards you with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W takes $s charged ki and forms a sparkling purple shroud of energy around $mself! $e swings $s arms towards @c$N@W with palms facing out flatly and shouts '@mG@Mal@wik @mG@Mu@wn@W!' as the energy around $s body forms into a beam and crashes into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (GET_SKILL_PERF(ch, SKILL_GALIKGUN) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 16, skill, attperc);
        dmg /= 10;
        act("@WYou fire a Galik Gun at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Galik Gun at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_honoo) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .1;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_HONOO)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_HONOO) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }
    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_HONOO); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_HONOO, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_HONOO) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Honoo")) {
            if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            if (ROOM_EFFECT(IN_ROOM(ch)) < -1) {
                send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates some!\r\n");
                ROOM_EFFECT(IN_ROOM(ch)) += 1;
            } else if (ROOM_EFFECT(IN_ROOM(ch)) == -1) {
                send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates completely away!\r\n");
                ROOM_EFFECT(IN_ROOM(ch)) = 0;
            }
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your honoo!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W honoo!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W honoo!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 21, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    if (ROOM_EFFECT(IN_ROOM(ch)) < -1) {
                        send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates some!\r\n");
                        ROOM_EFFECT(IN_ROOM(ch)) += 1;
                    } else if (ROOM_EFFECT(IN_ROOM(ch)) == -1) {
                        send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates completely away!\r\n");
                        ROOM_EFFECT(IN_ROOM(ch)) = 0;
                    }
                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your honoo, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W honoo, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W honoo, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 21, skill, SKILL_HONOO); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    if (ROOM_EFFECT(IN_ROOM(ch)) < -1) {
                        send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates some!\r\n");
                        ROOM_EFFECT(IN_ROOM(ch)) += 1;
                    } else if (ROOM_EFFECT(IN_ROOM(ch)) == -1) {
                        send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates completely away!\r\n");
                        ROOM_EFFECT(IN_ROOM(ch)) = 0;
                    }

                    return;
                } else {
                    act("@WYou can't believe it but your honoo misses, flying through the air harmlessly!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a honoo at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a honoo at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict, TO_NOTVICT);

                    if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    if (ROOM_EFFECT(IN_ROOM(ch)) < -1) {
                        send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates some!\r\n");
                        ROOM_EFFECT(IN_ROOM(ch)) += 1;
                    } else if (ROOM_EFFECT(IN_ROOM(ch)) == -1) {
                        send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates completely away!\r\n");
                        ROOM_EFFECT(IN_ROOM(ch)) = 0;
                    }

                    return;
                }
            } else {
                act("@WYou can't believe it but your honoo misses, flying through the air harmlessly!@n", false, ch,
                    nullptr,
                    vict, TO_CHAR);
                act("@C$n@W fires a honoo at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a honoo at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
            }
            if (ROOM_EFFECT(IN_ROOM(ch)) < -1) {
                send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates some!\r\n");
                ROOM_EFFECT(IN_ROOM(ch)) += 1;
            } else if (ROOM_EFFECT(IN_ROOM(ch)) == -1) {
                send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates completely away!\r\n");
                ROOM_EFFECT(IN_ROOM(ch)) = 0;
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 21, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            if (check_ruby(ch) == 1) {
                dmg += dmg * 0.2;
            }
            if (GET_BONUS(vict, BONUS_FIREPROOF)) {
                dmg -= dmg * 0.4;
            } else if (GET_BONUS(vict, BONUS_FIREPRONE)) {
                dmg += dmg * 0.4;
            }

            vict->affected_by.set(AFF_ASHED);
            switch (hitspot) {
                case 1:
                    act("@WYou gather your charged ki and bring it up into your throat while mixing it with the air in your lungs. You grin evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from your lips! @c$N@W's body is engulfed!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at you before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! YOUR body is engulfed!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! @c$N@W's body is engulfed!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou gather your charged ki and bring it up into your throat while mixing it with the air in your lungs. You grin evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from your lips! @c$N@W's face is engulfed!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at you before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! YOUR face is engulfed!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! @c$N@W's face is engulfed!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou gather your charged ki and bring it up into your throat while mixing it with the air in your lungs. You grin evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from your lips! @c$N@W's gut is engulfed!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at you before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! YOUR gut is engulfed!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! @c$N@W's gut is engulfed!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou gather your charged ki and bring it up into your throat while mixing it with the air in your lungs. You grin evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from your lips! @c$N@W's arm is engulfed!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at you before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! YOUR arm is engulfed!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! @c$N@W's arm is engulfed!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou gather your charged ki and bring it up into your throat while mixing it with the air in your lungs. You grin evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from your lips! @c$N@W's leg is engulfed!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at you before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! YOUR leg is engulfed!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki and brings it up into $s throat while mixing it with the air in $s lungs. $e grins evily at @c$N@W before unleashing a massive jet of @rf@Rl@Ya@rm@Re@Ys@W from $s lips! @c$N@W's leg is engulfed!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!AFF_FLAGGED(vict, AFF_BURNED) && rand_number(1, 4) == 3 && !IS_DEMON(vict) &&
                !GET_BONUS(vict, BONUS_FIREPROOF)) {
                send_to_char(vict, "@RYou are burned by the attack!@n\r\n");
                send_to_char(ch, "@RThey are burned by the attack!@n\r\n");
                vict->affected_by.set(AFF_BURNED);
            } else if (GET_BONUS(vict, BONUS_FIREPROOF) || IS_DEMON(vict)) {
                send_to_char(ch, "@RThey appear to be fireproof!@n\r\n");
            } else if (GET_BONUS(vict, BONUS_FIREPRONE)) {
                send_to_char(vict, "@RYou are extremely flammable and are burned by the attack!@n\r\n");
                send_to_char(ch, "@RThey are easily burned!@n\r\n");
                vict->affected_by.set(AFF_BURNED);
            }
            if (GET_SKILL_PERF(ch, SKILL_HONOO) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            if (ROOM_EFFECT(IN_ROOM(ch)) < -1) {
                send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates some!\r\n");
                ROOM_EFFECT(IN_ROOM(ch)) += 1;
            } else if (ROOM_EFFECT(IN_ROOM(ch)) == -1) {
                send_to_room(IN_ROOM(ch), "The water surrounding the area evaporates completely away!\r\n");
                ROOM_EFFECT(IN_ROOM(ch)) = 0;
            }
            vict->affected_by.reset(AFF_ASHED);
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 21, skill, attperc);
        dmg /= 10;
        if (GET_OBJ_VNUM(obj) == 79)
            dmg *= 3;
        act("@WYou fire a honoo at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a honoo at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_psyblast) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .1;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_PSYBLAST)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_PSYBLAST); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_PSYBLAST, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Psychic Blast")) {
            if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your psychic blast!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W psychic blast!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W psychic blast!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 20, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your psychic blast, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W psychic blast, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W psychic blast, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 20, skill, SKILL_PSYBLAST); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your psychic blast misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a psychic blast at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a psychic blast at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your psychic blast misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a psychic blast at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a psychic blast at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 20, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou gather your charged ki into your brain as a flash of @bb@Bl@wue@W light shoots from your forehead and slams into @c$N@W's body! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into YOUR body! You scream for a moment as terrifying images sear through your mind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into @c$N@W's body! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou gather your charged ki into your brain as a flash of @bb@Bl@wue@W light shoots from your forehead and slams into @c$N@W's head! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into YOUR head! You scream for a moment as terrifying images sear through your mind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into @c$N@W's head! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou gather your charged ki into your brain as a flash of @bb@Bl@wue@W light shoots from your forehead and slams into @c$N@W's gut! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into YOUR gut! You scream for a moment as terrifying images sear through your mind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into @c$N@W's gut! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou gather your charged ki into your brain as a flash of @bb@Bl@wue@W light shoots from your forehead and slams into @c$N@W's arm! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into YOUR arm! You scream for a moment as terrifying images sear through your mind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into @c$N@W's arm! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou gather your charged ki into your brain as a flash of @bb@Bl@wue@W light shoots from your forehead and slams into @c$N@W's leg! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into YOUR leg! You scream for a moment as terrifying images sear through your mind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s brain as a flash of @bb@Bl@wue@W light shoots from $s forehead and slams into @c$N@W's leg! $E screams for a moment as terrifying images sear through $S mind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }

            if (GET_CHARGE(vict) > 0 && rand_number(1, 3) == 2) {
                GET_CHARGE(vict) -= dmg / 5;
                if (GET_CHARGE(vict) < 0) {
                    GET_CHARGE(vict) = 0;
                }
                send_to_char(vict, "@RYou lose some of your charged ki!@n\r\n");
            }

            if (!AFF_FLAGGED(vict, AFF_SHOCKED) && rand_number(1, 4) == 4 && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                act("@MYour mind has been shocked!@n", true, vict, nullptr, nullptr, TO_CHAR);
                act("@M$n@m's mind has been shocked!@n", true, vict, nullptr, nullptr, TO_ROOM);
                vict->affected_by.set(AFF_SHOCKED);
            }
            if (GET_SKILL_PERF(ch, SKILL_PSYBLAST) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 20, skill, attperc);
        dmg /= 10;
        act("@WYou fire a psychic blast at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a psychic blast at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_tslash) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .1;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_TSLASH)) {
        return;
    }

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no available arms!\r\n");
        return;
    } else if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50 && GET_LIMBCOND(ch, 1) < 0) {
        send_to_char(ch, "Using your broken right arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 0) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 0) < 0) {
            act("@RYour right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    } else if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50 && GET_LIMBCOND(ch, 0) < 0) {
        send_to_char(ch, "Using your broken left arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 1) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 1) < 0) {
            act("@RYour left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!GET_EQ(ch, WEAR_WIELD1)) {
        send_to_char(ch, "You need to wield a sword to use this.\r\n");
        return;
    }
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) != TYPE_SLASH - TYPE_HIT) {
        send_to_char(ch, "You are not wielding a sword, you need one to use this technique.\r\n");
        return;
    }

    auto wobj = GET_EQ(ch, WEAR_WIELD1);
    int wlvl = 0;
    if (OBJ_FLAGGED(wobj, ITEM_WEAPLVL1)) {
        wlvl = 1;
    } else if (OBJ_FLAGGED(wobj, ITEM_WEAPLVL2)) {
        wlvl = 2;
    } else if (OBJ_FLAGGED(wobj, ITEM_WEAPLVL3)) {
        wlvl = 3;
    } else if (OBJ_FLAGGED(wobj, ITEM_WEAPLVL4)) {
        wlvl = 4;
    } else if (OBJ_FLAGGED(wobj, ITEM_WEAPLVL5)) {
        wlvl = 5;
    }

    if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_TSLASH); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_TSLASH, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Twin Slash")) {
            if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your twin slash!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W twin slash!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W twin slash!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 19, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your twin slash, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W twin slash, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W twin slash, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 19, skill, SKILL_TSLASH); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your twin slash misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a twin slash at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a twin slash at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your twin slash misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a twin slash at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a twin slash at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 19, skill, attperc);
            if (GET_SKILL(ch, SKILL_TSLASH) >= 100) {
                dmg += (dmg * 0.05) * wlvl;
            } else if (GET_SKILL(ch, SKILL_TSLASH) >= 60) {
                dmg += (dmg * 0.02) * wlvl;
            } else if (GET_SKILL(ch, SKILL_TSLASH) >= 40) {
                dmg += (dmg * 0.01) * wlvl;
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou channel your charged ki into the blade of your sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as you draw it up to attack. Two blindingly quick slashes connect with @c$N@W's body as you fly past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with YOUR body as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with @c$N@W's body as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (vict->hasTail()) {
                        act("@rYou cut off $S tail!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@rYour tail is cut off!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@R$N@r's tail is cut off!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        vict->loseTail();
                    }
                    break;
                case 2: /* Critical */
                    act("@WYou channel your charged ki into the blade of your sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as you draw it up to attack. Two blindingly quick slashes connect with @c$N@W's face as you fly past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with YOUR face as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with @c$N@W's face as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    if (dmg > GET_MAX_HIT(vict) / 5 && (!IS_MAJIN(vict) && !IS_BIO(vict))) {
                        act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@rYou have your head cut off by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        GET_DEATH_TYPE(vict) = DTYPE_HEAD;
                        remove_limb(vict, 0);
                        die(vict, ch);
                        if (AFF_FLAGGED(ch, AFF_GROUP)) {
                            group_gain(ch, vict);
                        } else {
                            solo_gain(ch, vict);
                        }
                        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                            do_get(ch, "all.zenni corpse", 0, 0);
                        }
                        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                            do_get(ch, "all corpse", 0, 0);
                        }
                    } else if (dmg > GET_MAX_HIT(vict) / 5 && (IS_MAJIN(vict) || IS_BIO(vict))) {
                        if (GET_SKILL(vict, SKILL_REGENERATE) > rand_number(1, 101) &&
                            (vict->getCurKI()) >= GET_MAX_MANA(vict) / 40) {
                            act("@R$N@r has $S head cut off by the attack but regenerates a moment later!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@rYou have your head cut off by the attack but regenerate a moment later!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@R$N@r has $S head cut off by the attack but regenerates a moment later!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            vict->decCurKI(vict->getMaxKI() / 40);
                            hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        } else {
                            act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@rYou have your head cut off by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            GET_DEATH_TYPE(vict) = DTYPE_HEAD;
                            die(vict, ch);
                            if (AFF_FLAGGED(ch, AFF_GROUP)) {
                                group_gain(ch, vict);
                            } else {
                                solo_gain(ch, vict);
                            }
                            if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                                do_get(ch, "all.zenni corpse", 0, 0);
                            }
                            if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                                do_get(ch, "all corpse", 0, 0);
                            }
                        }
                    } else {
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    }
                    dmg *= calc_critical(ch, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou channel your charged ki into the blade of your sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as you draw it up to attack. Two blindingly quick slashes connect with @c$N@W's gut as you fly past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with YOUR gut as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with @c$N@W's gut as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou channel your charged ki into the blade of your sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as you draw it up to attack. Two blindingly quick slashes connect with @c$N@W's arm as you fly past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with YOUR arm as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with @c$N@W's arm as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (rand_number(1, 100) >= 70 && !IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                        if (GET_LIMBCOND(vict, 1) > 0 && !is_sparring(ch) && rand_number(1, 1) == 2) {
                            act("@RYour attack severs $N's left arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severs your left arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's left arm is severered in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            GET_LIMBCOND(vict, 1) = 0;
                            remove_limb(vict, 2);
                        } else if (GET_LIMBCOND(vict, 0) > 0 && !is_sparring(ch)) {
                            act("@RYour attack severs $N's right arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severs your right arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's right arm is severered in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            GET_LIMBCOND(vict, 0) = 0;
                            remove_limb(vict, 1);
                        }
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou channel your charged ki into the blade of your sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as you draw it up to attack. Two blindingly quick slashes connect with @c$N@W's leg as you fly past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with YOUR leg as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wchannels $s charged ki into the blade of $s sword. @rF@Rl@Ya@rm@Ri@Yn@rg @gg@Gre@wen@W energy burns around the blade as $e draws it up to attack. Two blindingly quick slashes connect with @c$N@W's leg as $e flies past, leaving a green after-image behind!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (rand_number(1, 100) >= 70 && !IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                        if (GET_LIMBCOND(vict, 3) > 0 && !is_sparring(ch) && rand_number(1, 1) == 2) {
                            act("@RYour attack severs $N's left leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severs your left leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's left leg is severered in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            GET_LIMBCOND(vict, 3) = 0;
                            remove_limb(vict, 4);
                        } else if (GET_LIMBCOND(vict, 2) > 0 && !is_sparring(ch)) {
                            act("@RYour attack severs $N's right leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severs your right leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's right leg is severered in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            GET_LIMBCOND(vict, 2) = 0;
                            remove_limb(vict, 3);
                        }
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (GET_SKILL_PERF(ch, SKILL_TSLASH) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 19, skill, attperc);
        dmg /= 10;
        act("@WYou fire a twin slash at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a twin slash at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_eraser) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .1;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_ERASER)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_ERASER); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_ERASER, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Eraser Cannon")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
                ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
            } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
                ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
            } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
                ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
            }

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your eraser cannon!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W eraser cannon!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W eraser cannon!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
                    } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                    } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                    }
                    dmg = damtype(ch, 18, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your eraser cannon, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W eraser cannon, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W eraser cannon, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 18, skill, SKILL_ERASER); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
                    } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                    } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your eraser cannon misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a eraser cannon at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a eraser cannon at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
                    } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                    } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
                        ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your eraser cannon misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a eraser cannon at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a eraser cannon at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);
                if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
                    ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
                } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
                    ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
                    ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                }

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 18, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou face @c$N@W quickly and open your mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in your throat, growing brighter as it rises up and out your mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wfaces you quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into your body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wfaces @c$N@W quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                case 3:
                    act("@WYou face @c$N@W quickly and open your mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in your throat, growing brighter as it rises up and out your mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wfaces you quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into your face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wfaces @c$N@W quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou face @c$N@W quickly and open your mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in your throat, growing brighter as it rises up and out your mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wfaces you quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wfaces @c$N@W quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou face @c$N@W quickly and open your mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in your throat, growing brighter as it rises up and out your mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wfaces you quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wfaces @c$N@W quickly and opens $s mouth wide. A @Yg@yo@Yl@yd@Ye@yn@W glow forms from deep in $s throat, growing brighter as it rises up and out $s mouth. Suddenly a powerful Eraser Cannon erupts and slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);
            if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
                ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
            } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
                ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
            } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
                ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
            }

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 18, skill, attperc);
        dmg /= 10;
        act("@WYou fire a eraser cannon at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a eraser cannon at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
        if (GET_SKILL(ch, SKILL_ERASER) >= 100) {
            ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.15);
        } else if (GET_SKILL(ch, SKILL_ERASER) >= 60) {
            ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
        } else if (GET_SKILL(ch, SKILL_ERASER) >= 40) {
            ch->decCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
        }

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_pbarrage) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .15;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_PBARRAGE)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_PBARRAGE); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_PBARRAGE, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Psychic Barrage")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your psychic barrage!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W psychic barrage!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W psychic barrage!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 31, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your psychic barrage, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W psychic barrage, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W psychic barrage, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 31, skill, SKILL_PBARRAGE); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your psychic barrage misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a psychic barrage at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a psychic barrage at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your psychic barrage misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a psychic barrage at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a psychic barrage at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 31, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou close your eyes for a moment as your charged ki is pooled into your brain. Your eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and look at @c$N@W intensly. Invisible waves of psychic energy slam into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at YOU intensly. Invisible waves of psychic energy slam into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at @c$N@W intensly. Invisible waves of psychic energy slam into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou close your eyes for a moment as your charged ki is pooled into your brain. Your eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and look at @c$N@W intensly. Invisible waves of psychic energy slam into $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at YOU intensly. Invisible waves of psychic energy slam into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at @c$N@W intensly. Invisible waves of psychic energy slam into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou close your eyes for a moment as your charged ki is pooled into your brain. Your eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and look at @c$N@W intensly. Invisible waves of psychic energy slam into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at YOU intensly. Invisible waves of psychic energy slam into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at @c$N@W intensly. Invisible waves of psychic energy slam into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou close your eyes for a moment as your charged ki is pooled into your brain. Your eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and look at @c$N@W intensly. Invisible waves of psychic energy slam into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at YOU intensly. Invisible waves of psychic energy slam into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at @c$N@W intensly. Invisible waves of psychic energy slam into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou close your eyes for a moment as your charged ki is pooled into your brain. Your eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and look at @c$N@W intensly. Invisible waves of psychic energy slam into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at YOU intensly. Invisible waves of psychic energy slam into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W closes $s eyes for a moment as $s charged ki is pooled into $s brain. @C$n@W's eyes snap open, flashing with @bb@Bl@Wu@we @yl@Yi@Wg@wht@W, and looks at @c$N@W intensly. Invisible waves of psychic energy slam into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!AFF_FLAGGED(vict, AFF_MBREAK) && rand_number(1, 4) == 4 && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                act("@mYour mind's eye has been shattered, you can't charge ki until you recover!@n", true, vict,
                    nullptr, nullptr,
                    TO_CHAR);
                act("@M$n@m's mind has been damaged by the attack!@n", true, vict, nullptr, nullptr, TO_ROOM);
                vict->affected_by.set(AFF_MBREAK);
            } else if (!AFF_FLAGGED(vict, AFF_SHOCKED) && rand_number(1, 4) == 4 && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                act("@MYour mind has been shocked!@n", true, vict, nullptr, nullptr, TO_CHAR);
                act("@M$n@m's mind has been shocked!@n", true, vict, nullptr, nullptr, TO_ROOM);
                vict->affected_by.set(AFF_SHOCKED);
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 31, skill, attperc);
        dmg /= 10;
        act("@WYou fire a psychic barrage at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a psychic barrage at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_geno) {

    int perc, prob;
    double attperc = 0.5, minimum = .4;
    struct char_data *vict = nullptr;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);
    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_GENOCIDE)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    vict = nullptr;
    if (!*arg || !(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        if (FIGHTING(ch) && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char(ch, "No one around here by that name.\r\n");
            return;
        }
    }

    if (!can_kill(ch, vict, nullptr, 3)) {
        return;
    }

    if (handle_defender(vict, ch)) {
        struct char_data *def = GET_DEFENDER(vict);
        vict = def;
    }

    prob = init_skill(ch, SKILL_GENOCIDE); /* Set skill value */
    perc = rand_number(1, 115);

    if (prob < perc - 20) {
        act("@WYou raise one arm above your head and pour your charged ki there. A large swirling pink ball of energy begins to form above your raised hand. You lose concentration and the ball of energy dissipates!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W raises one arm above $s head and pours $s charged ki there. A large swirling pink ball of energy begins to form above $s raised hand. @C$n@W loses concentration and the ball of energy dissipates!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        hurt(0, 0, ch, vict, nullptr, 0, 1);
        pcost(ch, attperc, 0);

        improve_skill(ch, SKILL_GENOCIDE, 2);
        return;
    }

    struct obj_data *obj;
    int dista = 15 - (GET_INT(ch) * 0.1);

    if (GET_SKILL(ch, SKILL_GENOCIDE) >= 100) {
        dista -= 3;
    } else if (GET_SKILL(ch, SKILL_GENOCIDE) >= 60) {
        dista -= 2;
    } else if (GET_SKILL(ch, SKILL_GENOCIDE) >= 40) {
        dista -= 1;
    }

    obj = read_object(83, VIRTUAL);
    obj_to_room(obj, IN_ROOM(vict));

    GET_CHARGE(ch) += GET_MAX_HIT(ch) / 10;
    TARGET(obj) = vict;
    KICHARGE(obj) = damtype(ch, 41, prob, attperc);
    KITYPE(obj) = SKILL_GENOCIDE;
    USER(obj) = ch;
    KIDIST(obj) = dista;
    pcost(ch, attperc, 0);
    act("@WYou raise one arm above your head and pour your charged ki there. A large swirling pink ball of energy begins to form above your raised hand. You grin viciously as the @mG@Me@wn@mo@Mc@wi@md@Me@W attack is complete and you toss it at @c$N@W!@n",
        true, ch, nullptr, vict, TO_CHAR);
    act("@C$n@W raises one arm above $s head and pours $s charged ki there. A large swirling pink ball of energy begins to form above $s raised hand. @C$n@W grins viciously as the @mG@Me@wn@mo@Mc@wi@md@Me@W attack is complete and $e tosses it at YOU!@n",
        true, ch, nullptr, vict, TO_VICT);
    act("@C$n@W raises one arm above $s head and pours $s charged ki there. A large swirling pink ball of energy begins to form above $s raised hand. @C$n@W grins viciously as the @mG@Me@wn@mo@Mc@wi@md@Me@W attack is complete and $e tosses it at @c$N@W!@n",
        true, ch, nullptr, vict, TO_NOTVICT);

    improve_skill(ch, SKILL_GENOCIDE, 2);
}

ACMD(do_genki) {

    int perc, prob;
    double attperc = .5, minimum = .4;
    struct char_data *friend_char = nullptr, *vict = nullptr, *next_v = nullptr;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);
    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_GENKIDAMA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    vict = nullptr;
    if (!*arg || !(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        if (FIGHTING(ch) && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char(ch, "No one around here by that name.\r\n");
            return;
        }
    }

    if (!can_kill(ch, vict, nullptr, 3)) {
        return;
    }

    if (handle_defender(vict, ch)) {
        struct char_data *def = GET_DEFENDER(vict);
        vict = def;
    }

    prob = init_skill(ch, SKILL_GENKIDAMA); /* Set skill value */
    perc = rand_number(1, 115);

    if (prob < perc - 20) {
        act("@WYou raise both your arms upwards and begin to pool your charged ki there. You also start calling on the ki of all living beings in the vicinity who are willing to help. Your concentration wavers though and you waste the energy you have!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W raises both $s arms upwards and begin to pool $s charged ki there. @C$n@W also starts calling on the ki of all living beings in the vicinity who are willing to help. @C$n@W's concentration wavers though and $e wastes the energy $e has!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        hurt(0, 0, ch, vict, nullptr, 0, 1);
        pcost(ch, attperc, 0);
        handle_cooldown(ch, 10);
        improve_skill(ch, SKILL_GENKIDAMA, 2);
        return;
    }

    for (friend_char = ch->getRoom()->people; friend_char; friend_char = next_v) {
        next_v = friend_char->next_in_room;
        if (friend_char == ch) {
            continue;
        }
        if (AFF_FLAGGED(friend_char, AFF_GROUP) &&
            (friend_char->master == ch || ch->master == friend_char || friend_char->master == ch->master)) {
            GET_CHARGE(ch) += (ch->getCurKI()) / 10;
            ch->decCurKI(ch->getCurKI() / 20);
        }
    }

    int dista = 15 - (GET_INT(ch) * 0.1);

    if (GET_SKILL(ch, SKILL_GENKIDAMA) >= 100) {
        dista -= 3;
    } else if (GET_SKILL(ch, SKILL_GENKIDAMA) >= 60) {
        dista -= 2;
    } else if (GET_SKILL(ch, SKILL_GENKIDAMA) >= 40) {
        dista -= 1;
    }

    struct obj_data *obj;

    obj = read_object(82, VIRTUAL);
    obj_to_room(obj, IN_ROOM(vict));

    TARGET(obj) = vict;
    KICHARGE(obj) = damtype(ch, 40, prob, attperc);
    KITYPE(obj) = SKILL_GENKIDAMA;
    USER(obj) = ch;
    KIDIST(obj) = dista;
    pcost(ch, attperc, 0);
    act("@WYou raise both your arms upwards and begin to pool your charged ki there. You also start calling on the ki of all living beings in the vicinity who are willing to help. A large @cS@Cp@wi@cr@Ci@wt @cB@Co@wm@cb@W forms above your hands, when it is finished you lob it toward @c$N@W!@n",
        true, ch, nullptr, vict, TO_CHAR);
    act("@C$n@W raises both $s arms upwards and begin to pool $s charged ki there. @C$n@W also starts calling on the ki of all living beings in the vicinity who are willing to help. A large @cS@Cp@wi@cr@Ci@wt @cB@Co@wm@cb@W forms above $s hands, when it is finished $e lobs it toward YOU!@n",
        true, ch, nullptr, vict, TO_VICT);
    act("@C$n@W raises both $s arms upwards and begin to pool $s charged ki there. @C$n@W also starts calling on the ki of all living beings in the vicinity who are willing to help. A large @cS@Cp@wi@cr@Ci@wt @cB@Co@wm@cb@W forms above $s hands, when it is finished $e lobs it toward @c$N@W!@n",
        true, ch, nullptr, vict, TO_NOTVICT);
    handle_cooldown(ch, 10);
    improve_skill(ch, SKILL_GENKIDAMA, 2);
}

ACMD(do_spiritball) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .25, minimum = .20;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SPIRITBALL)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_SPIRITBALL); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 8);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 3)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }

        improve_skill(ch, SKILL_SPIRITBALL, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Spirit Ball")) {
            pcost(ch, attperc, 0);
            dodge_ki(ch, vict, 2, 39, skill, SKILL_SPIRITBALL); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W deflects your Spirit Ball, sending it flying away!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@WYou deflect @C$n's@W Spirit Ball sending it flying away!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W deflects @c$n's@W Spirit Ball sending it flying away!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 200);

                    parry_ki(attperc, ch, vict, "spiritball", prob, perc, skill, 39);
                    /*      User/target/skill name/skill/hurt type */
                    pcost(ch, attperc, 0);

                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks the Spirit Ball!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Spirit Ball!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Spirit Ball!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 39, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your Spirit Ball, letting it slam into the surroundings!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Spirit Ball, letting it slam into the surroundings!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Spirit Ball, letting it slam into the surroundings!@n", true,
                        ch, nullptr, vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your spirit ball misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a spirit ball at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a spirit ball at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    dodge_ki(ch, vict, 2, 39, skill, SKILL_SPIRITBALL); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your spirit ball misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a spirit ball at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a spirit ball at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                dodge_ki(ch, vict, 2, 39, skill, SKILL_SPIRITBALL); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/
                pcost(ch, attperc, 0);
                hurt(0, 0, ch, vict, nullptr, 0, 1);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 39, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou raise your palm upward with your arm bent in front of you. Your charged ki slowly begins to creep up your arm and form a large glowing orb of energy above your upraised palm. With your @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted you move your hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into $S body, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at YOU! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at you, slamming into YOUR body, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into @c$N@W's body, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou raise your palm upward with your arm bent in front of you. Your charged ki slowly begins to creep up your arm and form a large glowing orb of energy above your upraised palm. With your @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted you move your hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into $S head, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at YOU! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at you, slamming into YOUR head, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into @c$N@W's head, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou raise your palm upward with your arm bent in front of you. Your charged ki slowly begins to creep up your arm and form a large glowing orb of energy above your upraised palm. With your @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted you move your hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into $S gut, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at YOU! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at you, slamming into YOUR gut, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into @c$N@W's gut, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou raise your palm upward with your arm bent in front of you. Your charged ki slowly begins to creep up your arm and form a large glowing orb of energy above your upraised palm. With your @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted you move your hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into $S arm, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at YOU! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at you, slamming into YOUR arm, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into @c$N@W's arm, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou raise your palm upward with your arm bent in front of you. Your charged ki slowly begins to creep up your arm and form a large glowing orb of energy above your upraised palm. With your @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted you move your hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into $S leg, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at YOU! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at you, slamming into YOUR leg, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s palm upward with $s arm bent in front of $m. @C$n@W's charged ki slowly begins to creep up $s arm and form a large glowing orb of energy above $s upraised palm. With $s @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wcompleted $e moves $s hand with index and middle fingers pointing at @c$N@W! The @yS@Yp@Wi@wri@yt @YB@Wa@wll @Wflies at $M, slamming into @c$N@W's leg, and explodes with a load roar!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 39, skill, attperc);
        dmg /= 10;
        act("@WYou fire a spirit ball at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a spirit ball at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_deathball) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .3, minimum = .15;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_DEATHBALL)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_DEATHBALL); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 8);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 3)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_DEATHBALL, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);
        prob -= rand_number(8, 10);

        if (!tech_handle_zanzoken(ch, vict, "Deathball")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your deathball, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W deathball, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W deathball, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 38, skill, SKILL_DEATHBALL); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(20);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your deathball misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a deathball at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a deathball at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your deathball misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a deathball at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a deathball at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 38, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou raise your hand with its index finger extended upwards. Your charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as you look down on @c$N@W. You move the hand that formed the attack and point at $M as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above you follows the movement! It descends on $M and crushes into $S body before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on YOU. @C$n@W moves the hand that formed the attack and points at YOU as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on YOU and crushes into YOUR body before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on @c$N@W. @C$n@W moves the hand that formed the attack and points at @c$N@W as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on @c$N@W and crushes into $S body before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou raise your hand with its index finger extended upwards. Your charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as you look down on @c$N@W. You move the hand that formed the attack and point at $M as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above you follows the movement! It descends on $M and crushes into $S head before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on YOU. @C$n@W moves the hand that formed the attack and points at YOU as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on YOU and crushes into YOUR head before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on @c$N@W. @C$n@W moves the hand that formed the attack and points at @c$N@W as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on @c$N@W and crushes into $S head before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou raise your hand with its index finger extended upwards. Your charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as you look down on @c$N@W. You move the hand that formed the attack and point at $M as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above you follows the movement! It descends on $M and crushes into $S gut before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on YOU. @C$n@W moves the hand that formed the attack and points at YOU as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on YOU and crushes into YOUR gut before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on @c$N@W. @C$n@W moves the hand that formed the attack and points at @c$N@W as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on @c$N@W and crushes into $S gut before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou raise your hand with its index finger extended upwards. Your charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as you look down on @c$N@W. You move the hand that formed the attack and point at $M as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above you follows the movement! It descends on $M and crushes into $S arm before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on YOU. @C$n@W moves the hand that formed the attack and points at YOU as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on YOU and crushes into YOUR arm before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on @c$N@W. @C$n@W moves the hand that formed the attack and points at @c$N@W as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on @c$N@W and crushes into $S arm before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou raise your hand with its index finger extended upwards. Your charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as you look down on @c$N@W. You move the hand that formed the attack and point at $M as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above you follows the movement! It descends on $M and crushes into $S leg before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on YOU. @C$n@W moves the hand that formed the attack and points at YOU as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on YOU and crushes into YOUR leg before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand with its index finger extended upwards. @C$n@W's charged ki begins to pool above that finger, forming a small @rred@W orb of energy. The orb of energy quickly grows to an enormous size as $e looks down on @c$N@W. @C$n@W moves the hand that formed the attack and points at @c$N@W as the @rD@Re@Da@rt@Rh@Db@ra@Rl@Dl@W above $m follows the movement! It descends on @c$N@W and crushes into $S leg before exploding into a massive blast!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 38, skill, attperc);
        dmg /= 10;
        act("@WYou fire a deathball at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a deathball at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_pslash) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .3, minimum = .15;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_PSLASH)) {
        return;
    }

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no available arms!\r\n");
        return;
    } else if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50 && GET_LIMBCOND(ch, 1) < 0) {
        send_to_char(ch, "Using your broken right arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 0) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 0) < 0) {
            act("@RYour right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    } else if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50 && GET_LIMBCOND(ch, 0) < 0) {
        send_to_char(ch, "Using your broken left arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 1) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 1) < 0) {
            act("@RYour left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }
    if (!GET_EQ(ch, WEAR_WIELD1)) {
        send_to_char(ch, "You need to wield a sword to use this.\r\n");
        return;
    }
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) != TYPE_SLASH - TYPE_HIT) {
        send_to_char(ch, "You are not wielding a sword, you need one to use this technique.\r\n");
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_PSLASH); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 8);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 3)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_PSLASH, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Phoenix Slash")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your Phoenix Slash!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Phoenix Slash!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Phoenix Slash!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 37, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your Phoenix Slash, letting it fly harmlessly by!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Phoenix Slash, letting it letting it fly harmlessly by!@n", false, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Phoenix Slash, letting it fly harmlessly by!@n", false, ch,
                        nullptr,
                        vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your Phoenix Slash misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Phoenix Slash at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Phoenix Slash at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your Phoenix Slash misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Phoenix Slash at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Phoenix Slash at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 37, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            if (check_ruby(ch) == 1) {
                dmg += dmg * 0.2;
            }
            if (GET_BONUS(vict, BONUS_FIREPROOF)) {
                dmg -= dmg * 0.4;
            } else if (GET_BONUS(vict, BONUS_FIREPRONE)) {
                dmg += dmg * 0.4;
            }
            vict->affected_by.set(AFF_ASHED);
            switch (hitspot) {
                case 1:
                    act("@WYou pour your charged ki into your sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surround the entire sword in the same instant that you pull the blade back and extend it behind your body. Suddenly you swing the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs $S body in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward YOU, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards YOU! The Phoenix Slash engulfs YOUR body in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs @c$N@W's body in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou pour your charged ki into your sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surround the entire sword in the same instant that you pull the blade back and extend it behind your body. Suddenly you swing the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs $S head in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward YOU, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards YOU! The Phoenix Slash engulfs YOUR head in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs @c$N@W's head in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou pour your charged ki into your sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surround the entire sword in the same instant that you pull the blade back and extend it behind your body. Suddenly you swing the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs $S gut in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward YOU, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards YOU! The Phoenix Slash engulfs YOUR gut in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs @c$N@W's gut in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou pour your charged ki into your sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surround the entire sword in the same instant that you pull the blade back and extend it behind your body. Suddenly you swing the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs $S arm in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward YOU, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards YOU! The Phoenix Slash engulfs YOUR arm in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs @c$N@W's arm in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou pour your charged ki into your sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surround the entire sword in the same instant that you pull the blade back and extend it behind your body. Suddenly you swing the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs $S leg in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward YOU, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards YOU! The Phoenix Slash engulfs YOUR leg in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged ki into $s sword's blade. @YF@ri@Re@Yr@ry @Rf@Yl@ra@Rm@Ye@rs@W surrounds the entire sword in the same instant that $e pulls the blade back and extends it behind $s body. Suddenly $e swings the blade forward toward @c$N@W, unleashing a large wave of flames! The flames take the shape of a large phoenix soaring towards @c$N@W! The Phoenix Slash engulfs @c$N@W's leg in flames a moment later!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!AFF_FLAGGED(vict, AFF_BURNED) && rand_number(1, 4) == 4 && !IS_DEMON(vict) &&
                !GET_BONUS(vict, BONUS_FIREPROOF)) {
                send_to_char(vict, "@RYou are burned by the attack!@n\r\n");
                send_to_char(ch, "@RThey are burned by the attack!@n\r\n");
                vict->affected_by.set(AFF_BURNED);
            } else if (GET_BONUS(vict, BONUS_FIREPROOF) || IS_DEMON(vict)) {
                send_to_char(ch, "@RThey appear to be fireproof!@n\r\n");
            } else if (GET_BONUS(vict, BONUS_FIREPRONE)) {
                send_to_char(vict, "@RYou are extremely flammable and are burned by the attack!@n\r\n");
                send_to_char(ch, "@RThey are easily burned!@n\r\n");
                vict->affected_by.set(AFF_BURNED);
            }
            pcost(ch, attperc, 0);
            vict->affected_by.reset(AFF_ASHED);
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 37, skill, attperc);
        dmg /= 10;
        if (GET_OBJ_VNUM(obj) == 79)
            dmg *= 3;
        act("@WYou fire a Phoenix Slash at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Phoenix Slash at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_bigbang) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .3, minimum = .15;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BIGBANG)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_BIGBANG); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 8);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 3)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_BIGBANG, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Big Bang")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your Big Bang!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Big Bang!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Big Bang!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 36, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your Big Bang, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Big Bang, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Big Bang, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 36, skill, SKILL_BIGBANG); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(20);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your Big Bang misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Big Bang at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Big Bang at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your Big Bang misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Big Bang at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Big Bang at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 36, skill, attperc);
            switch (rand_number(1, 6)) {
                case 1:
                    act("@WYou aim your hand at @c$N@W palm flat up at a ninety degree angle. Your charged ki pools there rapidly before a massive ball of energy explodes from your palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's body, and explodes leaving behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W aims $s hand at YOU palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into YOUR body, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s hand at @c$N@W palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's body, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                case 3:
                    act("@WYou aim your hand at @c$N@W palm flat up at a ninety degree angle. Your charged ki pools there rapidly before a massive ball of energy explodes from your palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's head, and explodes leaving behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W aims $s hand at YOU palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into YOUR head, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s hand at @c$N@W palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's head, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4:
                    act("@WYou aim your hand at @c$N@W palm flat up at a ninety degree angle. Your charged ki pools there rapidly before a massive ball of energy explodes from your palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's gut, and explodes leaving behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W aims $s hand at YOU palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into YOUR gut, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s hand at @c$N@W palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's gut, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak */
                    act("@WYou aim your hand at @c$N@W palm flat up at a ninety degree angle. Your charged ki pools there rapidly before a massive ball of energy explodes from your palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's arm, and explodes leaving behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W aims $s hand at YOU palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into YOUR arm, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s hand at @c$N@W palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's arm, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 6: /* Weak 2 */
                    act("@WYou aim your hand at @c$N@W palm flat up at a ninety degree angle. Your charged ki pools there rapidly before a massive ball of energy explodes from your palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's leg, and explodes leaving behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W aims $s hand at YOU palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into YOUR leg, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s hand at @c$N@W palm flat up at a ninety degree angle. @C$n@W's charged ki pools there rapidly before a massive ball of energy explodes from $s palm! The @yB@Yi@wg @yB@Ya@wn@yg @Wattack crosses the distance rapidly, slams into @c$N@W's leg, and explodes leading behind a mushroom cloud shortly after!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 36, skill, attperc);
        dmg /= 10;
        act("@WYou fire a Big Bang at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Big Bang at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_scatter) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .3, minimum = .15;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SCATTER)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_SCATTER); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    int cool = 8;

    if (IS_PICCOLO(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100)
            cool -= 3;
        else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            cool -= 2;
        else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40)
            cool -= 1;
    }

    if (cool < 1)
        cool = 1;

    handle_cooldown(ch, cool);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 3)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_SCATTER, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        prob += rand_number(10, 20);

        if (!tech_handle_zanzoken(ch, vict, "Scatter Shot")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks every kiball of your scatter shot!@n", false, ch, nullptr,
                        vict,
                        TO_CHAR);
                    act("@WYou move quickly and block every kiball of @C$n's@W scatter shot!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W moves quickly and blocks every kiball of @c$n's@W scatter shot!@n", false, ch, nullptr,
                        vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 35, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your scatter shot kiballs, letting them slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W scatter shot kiballs, letting them slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W scatter shot kiballs, letting them slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wBright explosions erupts from the impacts!\r\n");

                    dodge_ki(ch, vict, 0, 35, skill, SKILL_SCATTER); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(20);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but all the kiballs of your scatter shot miss, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires scatter shot kiballs at you, but they miss!@n ", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@c$n@W fires scatter shot kiballs at @C$N@W, but somehow they miss!@n ", false, ch, nullptr,
                        vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but all the kiballs of your scatter shot miss, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires scatter shot kiballs at you, but they miss!@n ", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires scatter shot kiballs at @C$N@W, but somehow they miss!@n ", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 35, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou bring your charged ki to your palms and furiously begin to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! You hold out your hand and clench it dramatically as your @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at YOU! The kiballs surround you from every direction in a sphere, preventing your escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou bring your charged ki to your palms and furiously begin to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! You hold out your hand and clench it dramatically as your @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at YOU! The kiballs surround you from every direction in a sphere, preventing your escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou bring your charged ki to your palms and furiously begin to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! You hold out your hand and clench it dramatically as your @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at YOU! The kiballs surround you from every direction in a sphere, preventing your escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou bring your charged ki to your palms and furiously begin to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! You hold out your hand and clench it dramatically as your @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at YOU! The kiballs surround you from every direction in a sphere, preventing your escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou bring your charged ki to your palms and furiously begin to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! You hold out your hand and clench it dramatically as your @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at YOU! The kiballs surround you from every direction in a sphere, preventing your escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W brings $s charged ki to $s palms and furiously begins to throw hundreds of kiballs at @C$N@W! The kiballs surround $M from every direction in a sphere, preventing $S escape! @C$n@W holds out $s hand and clenches it dramatically as $s @yS@Yc@ra@Rt@yt@Yer @yS@Yh@ro@Rt@W kiballs close in and explode against @C$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 35, skill, attperc);
        dmg /= 10;
        act("@WYou fire a scatter shot at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a scatter shot at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_balefire) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .3, minimum = .15;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BALEFIRE)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_BALEFIRE); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    int cool = 8;

    if (IS_PICCOLO(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100)
            cool -= 3;
        else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            cool -= 2;
        else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40)
            cool -= 1;
    }

    if (cool < 1)
        cool = 1;

    handle_cooldown(ch, cool);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 3)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_BALEFIRE, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        prob += rand_number(10, 20);

        if (!tech_handle_zanzoken(ch, vict, "Balefire")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks every kiball of your scatter shot!@n", false, ch, nullptr,
                        vict,
                        TO_CHAR);
                    act("@WYou move quickly and block every kiball of @C$n's@W scatter shot!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W moves quickly and blocks every kiball of @c$n's@W scatter shot!@n", false, ch, nullptr,
                        vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 35, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your balefire, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W balefire, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W balefire, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wBright explosions erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 35, skill, SKILL_BALEFIRE); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(20);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but all the kiballs of your scatter shot miss, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires scatter shot kiballs at you, but they miss!@n ", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@c$n@W fires scatter shot kiballs at @C$N@W, but somehow they miss!@n ", false, ch, nullptr,
                        vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but all of your balefire misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires balefire at you, but they miss!@n ", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires balefire at @C$N@W, but somehow they miss!@n ", false, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 35, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou open yourself to the source and begin tracing weaves, the most complex combination, with just the right amounts of each! Your innate talent finishes the final weave and a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from your hands! Your @WB@Ra@Yl@Wef@Yi@Rr@We@W crosses the distance quickly and slams into @C$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into @C$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou open yourself to the source and begin tracing weaves, the most complex combination, with just the right amounts of each! Your innate talent finishes the final weave and a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from your hands! Your @WB@Ra@Yl@Wef@Yi@Rr@We@W crosses the distance quickly and slams into @C$N@W's head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into @C$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou open yourself to the source and begin tracing weaves, the most complex combination, with just the right amounts of each! Your innate talent finishes the final weave and a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from your hands! Your @WB@Ra@Yl@Wef@Yi@Rr@We@W crosses the distance quickly and slams into @C$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into @C$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou open yourself to the source and begin tracing weaves, the most complex combination, with just the right amounts of each! Your innate talent finishes the final weave and a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from your hands! Your @WB@Ra@Yl@Wef@Yi@Rr@We@W crosses the distance quickly and slams into @C$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into @C$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou open yourself to the source and begin tracing weaves, the most complex combination, with just the right amounts of each! Your innate talent finishes the final weave and a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from your hands! Your @WB@Ra@Yl@Wef@Yi@Rr@We@W crosses the distance quickly and slams into @C$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W begins to glow with a soft, white aura! Their hands begin manipulating something unseen when suddenly a giant bar of @WB@Ra@Yl@Wef@Yi@Rr@We@W lances out from $s hands, crossing the distance and slamming into @C$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 35, skill, attperc);
        dmg /= 10;
        act("@WYou fire balefire at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires balefire at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}
/* Rillao: End Balefire */

ACMD(do_kakusanha) {
    int perc, dge = 2, count = 0, skill;
    int64_t dmg;
    double attperc = .3, minimum = .10;
    struct char_data *vict = nullptr, *next_v = nullptr;
    char arg2[MAX_INPUT_LENGTH];

    one_argument(argument, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KAKUSANHA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_KAKUSANHA); /* Set skill value */

    for (vict = ch->getRoom()->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (vict == ch) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
            continue;
        }
        if (GET_HIT(vict) <= 0) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_GROUP) && !IS_NPC(vict)) {
            if (vict->master == ch) {
                continue;
            } else if (ch->master == vict) {
                continue;
            } else if (vict->master == ch->master) {
                continue;
            }
        }
        if (MOB_FLAGGED(vict, MOB_NOKILL)) {
            continue;
        }
        if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
            continue;
        } else {
            count += 1;
        }
    }
    if (count <= 0) {
        send_to_char(ch, "There is no one worth targeting around.\r\n");
        return;
    } /* No one worth targeting */
    else {
        handle_cooldown(ch, 5);
        int hits = 0;
        perc = chance_to_hit(ch);

        if (skill < perc) {
            act("@WYou pour your charged ki into your hands and bring them both forward quickly. @yG@Yo@Wl@wden@W orbs of energy form at the extent of your palms. You fire one massive beam of @yg@Yo@Wl@wden @Wenergy from combining both orbs. The beam flies forward a short distance before you swing your arms upward and the beam follows suit. The beam flies off harmlessly through the air as you lose control over it!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@W pours $s charged ki into $s hands and brings them both forward quickly. @yG@Yo@Wl@wden@W orbs of energy form at the extent of $s palms. @C$n@W fires one massive beam of @yg@Yo@Wl@wden @Wenergy from combining both orbs. The beam flies forward a short distance before $e swings $s arms upward and the beam follows suit. The beam flies off harmlessly through the air as $e loses control over it!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            pcost(ch, attperc, 0);

            improve_skill(ch, SKILL_KAKUSANHA, 0);
            return;
        }

        act("@WYou pour your charged ki into your hands and bring them both forward quickly. @yG@Yo@Wl@wden@W orbs of energy form at the extent of your palms. You fire one massive beam of @yg@Yo@Wl@wden @Wenergy from combining both orbs. The beam flies forward a short distance before you swing your arms upward and the beam follows suit. Above your targets the beam breaks apart into five seperate pieces that follow their victims!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W pours $s charged ki into $s hands and brings them both forward quickly. @yG@Yo@Wl@wden@W orbs of energy form at the extent of $s palms. @C$n@W fires one massive beam of @yg@Yo@Wl@wden @Wenergy from combining both orbs. The beam flies forward a short distance before $e swings $s arms upward and the beam follows suit. Above you the beam breaks apart into five seperate pieces that follow their victims!@n",
            true, ch, nullptr, nullptr, TO_ROOM);

        dmg = damtype(ch, 34, skill, attperc);
        if (count >= 3) {
            dmg = (dmg / 100) * 40;
        } else if (count > 1) {
            dmg = (dmg / 100) * 60;
        }

        for (vict = ch->getRoom()->people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (vict == ch) {
                continue;
            }
            if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
                continue;
            }
            if (AFF_FLAGGED(vict, AFF_GROUP) && (vict->master == ch || ch->master == vict)) {
                continue;
            }
            if (MOB_FLAGGED(vict, MOB_NOKILL)) {
                continue;
            }
            if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
                continue;
            }
            if (GET_HIT(vict) <= 0) {
                continue;
            }
            if (hits >= 5) {
                continue;
            }
            dge = handle_dodge(vict);
            if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) || AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                hits++;
                act("@C$N@c disappears, avoiding the beam chasing $M!@n", false, ch, nullptr, vict, TO_CHAR);
                act("@cYou disappear, avoiding the beam chasing you!@n", false, ch, nullptr, vict, TO_VICT);
                act("@C$N@c disappears, avoiding the beam chasing $M!@n", false, ch, nullptr, vict, TO_NOTVICT);
                vict->affected_by.reset(AFF_ZANZOKEN);
                pcost(vict, 0, GET_MAX_HIT(vict) / 200);
                hurt(0, 0, ch, vict, nullptr, 0, 1);
                continue;
            } else if (dge + rand_number(-10, 5) > skill) {
                hits++;
                act("@c$N@W manages to escape the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@WYou manage to escape the attack!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$N@W manages to escape the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                improve_skill(vict, SKILL_DODGE, 0);
                hurt(0, 0, ch, vict, nullptr, 0, 1);
                continue;
            } else {
                hits++;
                act("@R$N@r is slammed by one of the beams!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@RYou are slammed by one of the beams!@n", true, ch, nullptr, vict, TO_VICT);
                act("@R$N@r is slammed by one of the beams!@n", true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
                continue;
            }
        } /* Hitting targets! */

        if (count < 5 && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE)) {
            send_to_room(IN_ROOM(ch), "The rest of the beams slam into the ground!@n\r\n");
            send_to_room(IN_ROOM(ch), "@wBright explosions erupt from the impacts!\r\n");

            if (SECT(IN_ROOM(ch)) != SECT_INSIDE) {
                impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
                switch (rand_number(1, 8)) {
                    case 1:
                        act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 2:
                        if (rand_number(1, 4) == 4 && ROOM_EFFECT(IN_ROOM(ch)) == 0) {
                            ROOM_EFFECT(IN_ROOM(ch)) = 5;
                            act("Lava spews up through cracks in the ground, roaring into the sky as a large column of molten rock!",
                                true, ch, nullptr, nullptr, TO_CHAR);
                            act("Lava spews up through cracks in the ground, roaring into the sky as a large column of molten rock!",
                                true, ch, nullptr, nullptr, TO_ROOM);
                        }
                        break;
                    case 3:
                        act("A cloud of dust envelopes the entire area!", true, ch, nullptr, nullptr, TO_CHAR);
                        act("A cloud of dust envelopes the entire area!", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    case 4:
                        act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 5:
                        act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 6:
                        act("The explosion continues to burn spreading out and devouring some more of the ground before dying out.",
                            true, ch, nullptr, nullptr, TO_CHAR);
                        act("The explosion continues to burn spreading out and devouring some more of the ground before dying out.",
                            true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    default:
                        /* we want no message for the default */
                        break;
                }
            }
            if (SECT(IN_ROOM(ch)) == SECT_UNDERWATER) {
                switch (rand_number(1, 3)) {
                    case 1:
                        act("The water churns violently!", true, ch, nullptr, nullptr, TO_CHAR);
                        act("The water churns violently!", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    case 2:
                        act("Large bubbles rise from the movement!", true, ch, nullptr, nullptr, TO_CHAR);
                        act("Large bubbles rise from the movement!", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    case 3:
                        act("The water collapses in on the hole created!", true, ch, nullptr, nullptr, TO_CHAR);
                        act("The water collapses in on the hole create!", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                }
            }
            if (SECT(IN_ROOM(ch)) == SECT_WATER_SWIM || SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM) {
                switch (rand_number(1, 3)) {
                    case 1:
                        act("A huge column of water erupts from the impact!", true, ch, nullptr, nullptr, TO_CHAR);
                        act("A huge column of water erupts from the impact!", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    case 2:
                        act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 3:
                        act("A huge depression forms in the water and erupts into a wave from the impact!", true, ch,
                            nullptr,
                            nullptr, TO_CHAR);
                        act("A huge depression forms in the water and erupts into a wave from the impact!", true, ch,
                            nullptr,
                            nullptr, TO_ROOM);
                        break;
                }
            }
            if (SECT(IN_ROOM(ch)) == SECT_INSIDE) {
                impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
                switch (rand_number(1, 8)) {
                    case 1:
                        act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 2:
                        act("The structure of the surrounding room cracks and quakes from the blast!", true, ch,
                            nullptr, nullptr,
                            TO_CHAR);
                        act("The structure of the surrounding room cracks and quakes from the blast!", true, ch,
                            nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 3:
                        act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 4:
                        act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 5:
                        act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    case 6:
                        act("The walls of the surrounding room crack in the same instant!", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("The walls of the surrounding room crack in the same instant!", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    default:
                        /* we want no message for the default */
                        break;
                }
            }

            ch->getRoom()->modDamage((5 - count) * 5);
        }

        pcost(ch, attperc, 0);

        improve_skill(ch, SKILL_KAKUSANHA, 0);
        handle_cooldown(ch, 5);
        return;
    } /* We have targets! Attempt to kills them! */

}

ACMD(do_hellspear) {
    int perc, dge = 2, count = 0, skill;
    int64_t dmg;
    double attperc = .3, minimum = .1;
    struct char_data *vict = nullptr, *next_v = nullptr;
    char arg2[MAX_INPUT_LENGTH];

    one_argument(argument, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_HELLSPEAR)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
        send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_HELLSPEAR); /* Set skill value */

    for (vict = ch->getRoom()->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (vict == ch) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
            continue;
        }
        if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
            continue;
        }
        if (MOB_FLAGGED(vict, MOB_NOKILL)) {
            continue;
        } else {
            count += 1;
        }
    }
    if (count <= 0) {
        send_to_char(ch, "There is no one worth targeting around.\r\n");
        return;
    } /* No one worth targeting */
    else {
        perc = chance_to_hit(ch);

        if (skill < perc) {
            act("@WYou fly up higher in the air while holding your hand up above your head. Your charged ki is condensed and materialized in the grasp of your raised hand forming a spear of energy. The spear disolves as you screw up the technique!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@W flies up higher in the air while holding $s hand up above $s head. @C$n@W's charged ki is condensed and materialized in the grasp of $s raised hand forming a spear of energy. The spear disolves as $e screws up the technique!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            pcost(ch, attperc, 0);

            improve_skill(ch, SKILL_HELLSPEAR, 0);
            return;
        }

        act("@WYou fly up higher in the air while holding your hand above your head. Your charged ki is condensed and materialized in the grasp of your raised hand forming a spear of energy. Grinning evily you aim the spear at the ground below and throw it. As the red spear of energy slams into the ground your laughter rings throughout the area, and the @rH@Re@Dl@Wl @rS@Rp@De@Wa@wr B@rl@Ra@Ds@wt@W erupts with a roar!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W flies up higher in the air while holding $s hand above $s head. @C$n@W's charged ki is condensed and materialized in the grasp of $s raised hand forming a spear of energy. Grinning evily $e aims the spear at the ground below and throws it. As the red spear of energy slams into the ground $s laughter rings throughout the area, and the @rH@Re@Dl@Wl @rS@Rp@De@Wa@wr B@rl@Ra@Ds@wt@W erupts with a roar!@n",
            true, ch, nullptr, nullptr, TO_ROOM);

        dmg = damtype(ch, 33, skill, attperc);

        for (vict = ch->getRoom()->people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (vict == ch) {
                continue;
            }
            if (MOB_FLAGGED(vict, MOB_NOKILL)) {
                continue;
            }
            if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
                continue;
            }
            if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
                continue;
            }
            dge = handle_dodge(vict);

            if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) || AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr,
                    vict,
                    TO_CHAR);
                act("@cYou disappear, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr, vict,
                    TO_VICT);
                act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr,
                    vict,
                    TO_NOTVICT);
                vict->affected_by.reset(AFF_ZANZOKEN);
                pcost(vict, 0, GET_MAX_HIT(vict) / 200);
                continue;
            } else if (dge + rand_number(-10, 5) > skill) {
                act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@WYou manage to escape the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, 0, 1);
                improve_skill(vict, SKILL_DODGE, 0);
                continue;
            } else {
                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@RYou are caught by the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING && rand_number(1, 4) == 4) {
                    handle_knockdown(vict);
                }
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
                continue;
            }
        } /* Hitting targets! */

        pcost(ch, attperc, 0);

        improve_skill(ch, SKILL_HELLSPEAR, 0);
        handle_cooldown(ch, 5);
        return;
    } /* We have targets! Attempt to kills them! */

}

ACMD(do_hellflash) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .10;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_HELLFLASH)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_HELLFLASH); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_HELLFLASH, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Hell Flash")) {
            if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your Hell Flash!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Hell Flash!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Hell Flash!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 32, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your Hell Flash, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Hell Flash, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Hell Flash, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 32, skill, SKILL_HELLFLASH); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }

                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your Hell Flash misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Hell Flash at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Hell Flash at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your Hell Flash misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Hell Flash at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Hell Flash at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 32, skill, attperc);
            if (GET_BARRIER(vict) > 0) {
                GET_BARRIER(vict) -= dmg;
                if (GET_BARRIER(vict) <= 0) {
                    GET_BARRIER(vict) = 1;
                }
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou stick one of your hands in each of your armpits and by twisting you remove them. Then you aim your exposed wrist cannons at @c$N@W as your charged energy begins to be channeled into the cannons. Shouting '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' two large blasts of energy explode from the cannons and slam into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at YOU as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at @c$N@W as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou stick one of your hands in each of your armpits and by twisting you remove them. Then you aim your exposed wrist cannons at @c$N@W as your charged energy begins to be channeled into the cannons. Shouting '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' two large blasts of energy explode from the cannons and slam into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at YOU as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at @c$N@W as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou stick one of your hands in each of your armpits and by twisting you remove them. Then you aim your exposed wrist cannons at @c$N@W as your charged energy begins to be channeled into the cannons. Shouting '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' two large blasts of energy explode from the cannons and slam into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at YOU as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at @c$N@W as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou stick one of your hands in each of your armpits and by twisting you remove them. Then you aim your exposed wrist cannons at @c$N@W as your charged energy begins to be channeled into the cannons. Shouting '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' two large blasts of energy explode from the cannons and slam into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at YOU as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at @c$N@W as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou stick one of your hands in each of your armpits and by twisting you remove them. Then you aim your exposed wrist cannons at @c$N@W as your charged energy begins to be channeled into the cannons. Shouting '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' two large blasts of energy explode from the cannons and slam into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at YOU as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W sticks one of $s hands in each of $s armpits and by twisting $e removes them. Then $e aims $s exposed wrist cannons at @c$N@W as $s charged energy begins to be channeled into the cannons. @C$n@W shouts '@YH@re@Rl@Yl @rF@Rl@Ya@rs@Rh@W' as two large blasts of energy explode from the cannons and slam into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (GET_SKILL_PERF(ch, SKILL_HELLFLASH) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 32, skill, attperc);
        dmg /= 10;
        act("@WYou fire a Hell Flash at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Hell Flash at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);


    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_ddslash) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .10;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_DDSLASH)) {
        return;
    }

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no available arms!\r\n");
        return;
    } else if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50 && GET_LIMBCOND(ch, 1) < 0) {
        send_to_char(ch, "Using your broken right arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 0) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 0) < 0) {
            act("@RYour right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    } else if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50 && GET_LIMBCOND(ch, 0) < 0) {
        send_to_char(ch, "Using your broken left arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 1) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 1) < 0) {
            act("@RYour left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    if (!GET_EQ(ch, WEAR_WIELD1)) {
        send_to_char(ch, "You need to wield a sword to use this.\r\n");
        return;
    }
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) != TYPE_SLASH - TYPE_HIT) {
        send_to_char(ch, "You are not wielding a sword, you need one to use this technique.\r\n");
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_DDSLASH); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_DDSLASH, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Darkness Dragon Slash")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your Darkness Dragon Slash!@n", false, ch, nullptr, vict,
                        TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Darkness Dragon Slash!@n", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Darkness Dragon Slash!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 30, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your Darkness Dragon Slash, letting it fly harmlessly by!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Darkness Dragon Slash, letting it letting it fly harmlessly by!@n", false,
                        ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Darkness Dragon Slash, letting it fly harmlessly by!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your Darkness Dragon Slash misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Darkness Dragon Slash at you, but misses!@n ", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@c$n@W fires a Darkness Dragon Slash at @C$N@W, but somehow misses!@n ", false, ch, nullptr,
                        vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your Darkness Dragon Slash misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Darkness Dragon Slash at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Darkness Dragon Slash at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 30, skill, attperc);
            if (GET_SKILL(ch, SKILL_DDSLASH) >= 100) {
                dmg += dmg * 0.15;
            } else if (GET_SKILL(ch, SKILL_DDSLASH) >= 60) {
                dmg += dmg * 0.10;
            } else if (GET_SKILL(ch, SKILL_DDSLASH) >= 40) {
                dmg += dmg * 0.05;
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou channel your charged ki into the blade of your sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging your sword at @c$N@W you unleash a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at YOU $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at @c$N@W $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou channel your charged ki into the blade of your sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging your sword at @c$N@W you unleash a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at YOU $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at @c$N@W $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou channel your charged ki into the blade of your sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging your sword at @c$N@W you unleash a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at YOU $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at @c$N@W $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou channel your charged ki into the blade of your sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging your sword at @c$N@W you unleash a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at YOU $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at @c$N@W $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou channel your charged ki into the blade of your sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging your sword at @c$N@W you unleash a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at YOU $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W channels $s charged ki into the blade of $s sword. The energy takes the form of @md@Ma@Wr@wk@W @mf@Ml@Wa@wmes@W burning along the very blade's edge. Swinging $s sword at @c$N@W $e unleashes a serpentine dragon of @md@Ma@Wr@wk @mf@Ml@Wa@wmes@W! The fiery dragon slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (rand_number(1, 3) == 3 && !AFF_FLAGGED(vict, AFF_BLIND)) {
                act("@mYou are struck blind temporarily!@n", true, vict, nullptr, nullptr, TO_CHAR);
                act("@c$n@m is struck blind by the attack!@n", true, vict, nullptr, nullptr, TO_ROOM);
                int duration = 1;
                assign_affect(vict, AFF_BLIND, SKILL_SOLARF, duration, 0, 0, 0, 0, 0, 0);
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 30, skill, attperc);
        dmg /= 10;
        act("@WYou fire a Darkness Dragon Slash at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Darkness Dragon Slash at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_crusher) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .10;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_CRUSHER)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_CRUSHER); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_CRUSHER, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Crusher Ball")) {
            if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your crusher ball!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W crusher ball!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W crusher ball!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 29, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your crusher ball, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W crusher ball, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W crusher ball, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 29, skill, SKILL_CRUSHER); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(10);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your crusher ball misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a crusher ball at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a crusher ball at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your crusher ball misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a crusher ball at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a crusher ball at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 29, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou zoom up into the air higher than @c$N@W and raise your hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W you slam your hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W zooms up into the air higher than you and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on YOU $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W zooms up into the air higher than @c$N@W and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou zoom up into the air higher than @c$N@W and raise your hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W you slam your hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W zooms up into the air higher than you and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on YOU $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W zooms up into the air higher than @c$N@W and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou zoom up into the air higher than @c$N@W and raise your hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W you slam your hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W zooms up into the air higher than you and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on YOU $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W zooms up into the air higher than @c$N@W and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou zoom up into the air higher than @c$N@W and raise your hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W you slam your hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W zooms up into the air higher than you and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on YOU $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W zooms up into the air higher than @c$N@W and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou zoom up into the air higher than @c$N@W and raise your hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W you slam your hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W zooms up into the air higher than you and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on YOU $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W zooms up into the air higher than @c$N@W and raises $s hand. Charged @rred@W ki pools above that hand in a large blazing ball. Looking down on @c$N@W $e slams $s hand into the ball of energy while shouting '@rC@Rr@Wu@wsher @rB@Ra@Wl@wl!@W' Moments later the ball of energy slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (rand_number(1, 3) == 3) {
                tech_handle_crashdown(ch, vict);
            }

            if (GET_SKILL_PERF(ch, SKILL_CRUSHER) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 29, skill, attperc);
        dmg /= 10;
        act("@WYou fire a crusher ball at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a crusher ball at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);


    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_final) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .10;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_FINALFLASH)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_FINALFLASH); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_FINALFLASH, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Final Flash")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your final flash!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W final flash!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W final flash!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 28, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your final flash, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W final flash, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W final flash, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 28, skill, SKILL_FINALFLASH); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(10);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your final flash misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a final flash at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a final flash at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your final flash misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a final flash at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a final flash at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 28, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou grin as you bring either hand to the sides of your body. Your charged ki begins to pool there as @bblue@W orbs of energy form in your palms. You quickly bring both hands forward slamming your wrists together with the palms flat out facing @c$N@W! You shout '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing YOU! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing @c$N@W! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou grin as you bring either hand to the sides of your body. Your charged ki begins to pool there as @bblue@W orbs of energy form in your palms. You quickly bring both hands forward slamming your wrists together with the palms flat out facing @c$N@W! You shout '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing YOU! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing @c$N@W! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (!GET_BONUS(ch, BONUS_POWERHIT)) {
                        dmg *= 3;
                    } else {
                        dmg *= 5;
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou grin as you bring either hand to the sides of your body. Your charged ki begins to pool there as @bblue@W orbs of energy form in your palms. You quickly bring both hands forward slamming your wrists together with the palms flat out facing @c$N@W! You shout '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing YOU! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing @c$N@W! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou grin as you bring either hand to the sides of your body. Your charged ki begins to pool there as @bblue@W orbs of energy form in your palms. You quickly bring both hands forward slamming your wrists together with the palms flat out facing @c$N@W! You shout '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing YOU! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing @c$N@W! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou grin as you bring either hand to the sides of your body. Your charged ki begins to pool there as @bblue@W orbs of energy form in your palms. You quickly bring both hands forward slamming your wrists together with the palms flat out facing @c$N@W! You shout '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing YOU! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W grins as $e brings either hand to the sides of $s body. @C$n@W's charged ki begins to pool there as @bblue@W orbs of energy form in $s palms. @C$n@W quickly brings both hands forward slamming $s wrists together with the palms flat out facing @c$N@W! @C$n@W shouts '@DF@ci@Cn@Da@cl @CF@Dl@ca@Cs@Dh@W!' as a massive wave of energy erupts all over @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 28, skill, attperc);
        dmg /= 10;
        act("@WYou fire a final flash at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a final flash at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_sbc) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .15, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SBC)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_SBC); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_SBC, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Special Beam Cannon")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your special beam cannon, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W special beam cannon, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W special beam cannon, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 27, skill, SKILL_SBC); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(10);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your special beam cannon misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a special beam cannon at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a special beam cannon at @C$N@W, but somehow misses!@n ", false, ch, nullptr,
                        vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your special beam cannon misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a special beam cannon at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a special beam cannon at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 27, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou place your index and middle fingers against your forehead and pool your charged ki there. Sparks and light surround your fingertips as the technique becomes ready. You point your fingers at @c$N@W and yell '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from your fingers and slams into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at YOU and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at @c$N@W and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou place your index and middle fingers against your forehead and pool your charged ki there. Sparks and light surround your fingertips as the technique becomes ready. You point your fingers at @c$N@W and yell '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from your fingers and slams into $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at YOU and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at @c$N@W and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou place your index and middle fingers against your forehead and pool your charged ki there. Sparks and light surround your fingertips as the technique becomes ready. You point your fingers at @c$N@W and yell '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from your fingers and slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at YOU and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at @c$N@W and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou place your index and middle fingers against your forehead and pool your charged ki there. Sparks and light surround your fingertips as the technique becomes ready. You point your fingers at @c$N@W and yell '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from your fingers and slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at YOU and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at @c$N@W and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou place your index and middle fingers against your forehead and pool your charged ki there. Sparks and light surround your fingertips as the technique becomes ready. You point your fingers at @c$N@W and yell '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from your fingers and slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at YOU and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s index and middle fingers against $s forehead and pools $s charged ki there. Sparks and light surround the fingertips as the technique becomes ready. @C$n@W points $s fingers at @c$N@W and yells '@YS@yp@De@Yc@yi@Da@Yl @yB@De@Ya@ym @DC@Ya@yn@Dn@Yo@yn@W!' A spiraling beam of energy fires from the fingers and slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 27, skill, attperc);
        dmg /= 10;
        act("@WYou fire a special beam cannon at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a special beam cannon at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_tribeam) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .10;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_TRIBEAM)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_TRIBEAM); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_TRIBEAM, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Tribeam")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your tribeam!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W tribeam!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W tribeam!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 26, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your tribeam, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W tribeam, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W tribeam, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 26, skill, SKILL_TRIBEAM); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(10);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your tribeam misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a tribeam at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a tribeam at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your tribeam misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a tribeam at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a tribeam at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 26, skill, attperc);
            if (GET_SKILL(ch, SKILL_TRIBEAM) >= 100) {
                dmg += (ch->getMaxLF()) * 0.20;
            } else if (GET_SKILL(ch, SKILL_TRIBEAM) >= 60) {
                dmg += (ch->getMaxLF()) * 0.10;
            } else if (GET_SKILL(ch, SKILL_TRIBEAM) >= 40) {
                dmg += (ch->getMaxLF()) * 0.05;
            }

            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou gather your charged ki as you form a triangle with your hands. You aim the gap between your hands at @c$N@W and shout '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from your hands and slams into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at YOU and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at @c$N@W and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou gather your charged ki as you form a triangle with your hands. You aim the gap between your hands at @c$N@W and shout '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from your hands and slams into $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at YOU and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at @c$N@W and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou gather your charged ki as you form a triangle with your hands. You aim the gap between your hands at @c$N@W and shout '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from your hands and slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at YOU and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at @c$N@W and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou gather your charged ki as you form a triangle with your hands. You aim the gap between your hands at @c$N@W and shout '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from your hands and slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at YOU and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at @c$N@W and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou gather your charged ki as you form a triangle with your hands. You aim the gap between your hands at @c$N@W and shout '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from your hands and slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at YOU and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki as $e forms a triangle with $s hands. @C$n@W aims the gap between $s hands at @c$N@W and shouts '@rT@RR@YI@rB@RE@YA@rM@W!'. A bright blast of energy explodes from $s hands and slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 180, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!IS_NPC(vict)) {
                WAIT_STATE(vict, PULSE_2SEC);
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 26, skill, attperc);
        dmg /= 10;
        act("@WYou fire a tribeam at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a tribeam at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_kienzan) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KIENZAN)) {
        return;
    }

    if(ch->playerFlags.test(PLR_SPAR) || ch->mobFlags.test(MOB_SPAR)) {
        send_to_char(ch, "That technique is a little too lethal for a spar, don't you think?\r\n");
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_KIENZAN); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_KIENZAN, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Kienzan")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your kienzan, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W kienzan, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W kienzan, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wThe kienzan expands, cutting through everything in its path!\r\n");

                    dodge_ki(ch, vict, 2, 25, skill, SKILL_KIENZAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your kienzan misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a kienzan at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a kienzan at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    dodge_ki(ch, vict, 2, 25, skill, SKILL_KIENZAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                              Num 2: [ Number of attack for damtype ]*/

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your kienzan misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a kienzan at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a kienzan at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);

                dodge_ki(ch, vict, 2, 25, skill, SKILL_KIENZAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 25, skill, attperc);
            if (GET_SKILL(ch, SKILL_KIENZAN) >= 100) {
                dmg += dmg * 0.25;
            } else if (GET_SKILL(ch, SKILL_KIENZAN) >= 60) {
                dmg += dmg * 0.15;
            } else if (GET_SKILL(ch, SKILL_KIENZAN) >= 40) {
                dmg += dmg * 0.05;
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou raise your hand above your head and pool your charged ki above your flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete you shout '@yK@Yi@we@yn@Yz@wa@yn@W!' and throw it! You watch as it slices into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (dmg > GET_MAX_HIT(vict) / 5 && (!IS_MAJIN(vict) && !IS_BIO(vict))) {
                        act("@R$N@r is cut in half by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@rYou are cut in half by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@R$N@r is cut in half by the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        die(vict, ch);
                        if (AFF_FLAGGED(ch, AFF_GROUP)) {
                            group_gain(ch, vict);
                        } else {
                            solo_gain(ch, vict);
                        }
                        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                            do_get(ch, "all.zenni corpse", 0, 0);
                        }
                        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                            do_get(ch, "all corpse", 0, 0);
                        }
                    } else if (dmg > GET_MAX_HIT(vict) / 5 && (IS_MAJIN(vict) || IS_BIO(vict))) {
                        if (GET_SKILL(vict, SKILL_REGENERATE) > rand_number(1, 101) &&
                            (vict->getCurKI()) >= GET_MAX_MANA(vict) / 40) {
                            act("@R$N@r is cut in half by the attack but regenerates a moment later!@n", true, ch,
                                nullptr,
                                vict, TO_CHAR);
                            act("@rYou are cut in half by the attack but regenerate a moment later!@n", true, ch,
                                nullptr,
                                vict, TO_VICT);
                            act("@R$N@r is cut in half by the attack but regenerates a moment later!@n", true, ch,
                                nullptr,
                                vict, TO_NOTVICT);
                            vict->decCurKI(vict->getMaxKI() / 40);
                            hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        } else {
                            act("@R$N@r is cut in half by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@rYou are cut in half by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N@r is cut in half by the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            die(vict, ch);
                            if (AFF_FLAGGED(ch, AFF_GROUP)) {
                                group_gain(ch, vict);
                            } else {
                                solo_gain(ch, vict);
                            }
                            if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                                do_get(ch, "all.zenni corpse", 0, 0);
                            }
                            if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                                do_get(ch, "all corpse", 0, 0);
                            }
                        }
                    } else {
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        if (vict->hasTail()) {
                            act("@rYou cut off $S tail!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@rYour tail is cut off!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N@r's tail is cut off!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            vict->loseTail();
                        }
                    }
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou raise your hand above your head and pool your charged ki above your flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete you shout '@yK@Yi@we@yn@Yz@wa@yn@W!' and throw it! You watch as it slices into @c$N@W's neck!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into YOUR neck!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into @c$N@W's neck!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    if (dmg > GET_MAX_HIT(vict) / 5 && (!IS_MAJIN(vict) && !IS_BIO(vict))) {
                        act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@rYou have your head cut off by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        GET_DEATH_TYPE(vict) = DTYPE_HEAD;
                        remove_limb(vict, 0);
                        die(vict, ch);
                        if (AFF_FLAGGED(ch, AFF_GROUP)) {
                            group_gain(ch, vict);
                        } else {
                            solo_gain(ch, vict);
                        }
                        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                            do_get(ch, "all.zenni corpse", 0, 0);
                        }
                        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                            do_get(ch, "all corpse", 0, 0);
                        }
                    } else if (dmg > GET_MAX_HIT(vict) / 5 && (IS_MAJIN(vict) || IS_BIO(vict))) {
                        if (GET_SKILL(vict, SKILL_REGENERATE) > rand_number(1, 101) &&
                            (vict->getCurKI()) >= GET_MAX_MANA(vict) / 40) {
                            act("@R$N@r has $S head cut off by the attack but regenerates a moment later!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@rYou have your head cut off by the attack but regenerate a moment later!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@R$N@r has $S head cut off by the attack but regenerates a moment later!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            vict->decCurKI(vict->getMaxKI() / 40);
                            hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        } else {
                            act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@rYou have your head cut off by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N@r has $S head cut off by the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            GET_DEATH_TYPE(vict) = DTYPE_HEAD;
                            die(vict, ch);
                            if (AFF_FLAGGED(ch, AFF_GROUP)) {
                                group_gain(ch, vict);
                            } else {
                                solo_gain(ch, vict);
                            }
                            if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                                do_get(ch, "all.zenni corpse", 0, 0);
                            }
                            if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                                do_get(ch, "all corpse", 0, 0);
                            }
                        }
                    } else {
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    }
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou raise your hand above your head and pool your charged ki above your flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete you shout '@yK@Yi@we@yn@Yz@wa@yn@W!' and throw it! You watch as it slices into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou raise your hand above your head and pool your charged ki above your flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete you shout '@yK@Yi@we@yn@Yz@wa@yn@W!' and throw it! You watch as it slices into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (rand_number(1, 100) >= 70 && !IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                        if (GET_LIMBCOND(vict, 1) > 0 && !is_sparring(ch) && rand_number(1, 1) == 2) {
                            act("@RYour attack severes $N's left arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severes your left arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's left arm is severed in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            GET_LIMBCOND(vict, 1) = 0;
                            remove_limb(vict, 2);
                        } else if (GET_LIMBCOND(vict, 0) > 0 && !is_sparring(ch)) {
                            act("@RYour attack severes $N's right arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severes your right arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's right arm is severed in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            GET_LIMBCOND(vict, 0) = 0;
                            remove_limb(vict, 1);
                        }
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou raise your hand above your head and pool your charged ki above your flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete you shout '@yK@Yi@we@yn@Yz@wa@yn@W!' and throw it! You watch as it slices into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand above $s head and pools $s charged ki above $s flattened palm. Slowly a golden spinning disk of energy grows from the ki. With the attack complete $e shouts '@yK@Yi@we@yn@Yz@wa@yn@W!' and throws it! @C$n@W watches as it slices into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (rand_number(1, 100) >= 70 && !IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                        if (GET_LIMBCOND(vict, 3) > 0 && !is_sparring(ch) && rand_number(1, 1) == 2) {
                            act("@RYour attack severes $N's left leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severes your left leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's left leg is severed in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            GET_LIMBCOND(vict, 3) = 0;
                            remove_limb(vict, 4);
                        } else if (GET_LIMBCOND(vict, 2) > 0 && !is_sparring(ch)) {
                            act("@RYour attack severes $N's right leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@R$n's attack severes your right leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@R$N's right leg is severed in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            GET_LIMBCOND(vict, 2) = 0;
                            remove_limb(vict, 3);
                        }
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 25, skill, attperc);
        dmg /= 10;
        act("@WYou fire a kienzan at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a kienzan at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_baku) {
    int perc, dge = 2, count = 0, skill;
    int64_t dmg;
    double attperc = .15, minimum = .05;
    struct char_data *vict = nullptr, *next_v = nullptr;
    char arg2[MAX_INPUT_LENGTH];

    one_argument(argument, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BAKUHATSUHA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
        send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_BAKUHATSUHA); /* Set skill value */

    for (vict = ch->getRoom()->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (vict == ch) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_GROUP) && (vict->master == ch || ch->master == vict)) {
            continue;
        }
        if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
            continue;
        }
        if (MOB_FLAGGED(vict, MOB_NOKILL)) {
            continue;
        } else {
            count += 1;
        }
    }
    if (count <= 0) {
        send_to_char(ch, "There is no one worth targeting around.\r\n");
        return;
    } /* No one worth targeting */
    else {
        perc = chance_to_hit(ch);
        handle_cooldown(ch, 6);
        if (skill < perc) {
            act("@WYou raise your hand with index and middle fingers extended upwards. You try releasing your charged ki in a @yB@Ya@Wk@wuh@ya@Yt@Ws@wuh@ya@W but mess up and waste the ki!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@W raises $s hand with index and middle fingers extended upwards. @C$n@W tries releasing $s charged ki in a @yB@Ya@Wk@wuh@ya@Yt@Ws@wuh@ya@W but messes up and wastes the ki!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            pcost(ch, attperc, 0);

            improve_skill(ch, SKILL_BAKUHATSUHA, 0);
            return;
        }

        act("@WYou raise your hand with index and middle fingers extended upwards. A sudden burst rushes up from the ground as your charged ki explodes in the form of a @yB@Ya@Wk@wuh@ya@Yt@Ws@wuh@ya@W!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W raises $s hand with index and middle fingers extended upwards. A sudden burst rushes up from the ground as $s charged ki explodes in the form of a @yB@Ya@Wk@wuh@ya@Yt@Ws@wuh@ya@W!@n",
            true, ch, nullptr, nullptr, TO_ROOM);

        dmg = damtype(ch, 24, skill, attperc);

        if (GET_SKILL(ch, SKILL_BAKUHATSUHA) >= 100) {
            dmg += GET_MAX_HIT(ch) * 0.08;
        } else if (GET_SKILL(ch, SKILL_BAKUHATSUHA) >= 60) {
            dmg += GET_MAX_HIT(ch) * 0.04;
        } else if (GET_SKILL(ch, SKILL_BAKUHATSUHA) >= 40) {
            dmg += GET_MAX_HIT(ch) * 0.02;
        }

        switch (count) {
            case 1:
                dmg = dmg;
                break;
            case 2:
                dmg = (dmg / 100) * 75;
                break;
            case 3:
                dmg = (dmg / 100) * 50;
                break;
            default:
                dmg = (dmg / 100) * 25;
                break;
        }
        for (vict = ch->getRoom()->people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (vict == ch) {
                continue;
            }
            if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
                continue;
            }
            if (AFF_FLAGGED(vict, AFF_GROUP) &&
                (vict->master == ch || ch->master == vict || vict->master == ch->master)) {
                continue;
            }
            if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
                continue;
            }
            if (MOB_FLAGGED(vict, MOB_NOKILL)) {
                continue;
            }
            dge = handle_dodge(vict);
            if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) || AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr,
                    vict,
                    TO_CHAR);
                act("@cYou disappear, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr, vict,
                    TO_VICT);
                act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr,
                    vict,
                    TO_NOTVICT);
                vict->affected_by.reset(AFF_ZANZOKEN);
                pcost(vict, 0, GET_MAX_HIT(vict) / 200);
                hurt(0, 0, ch, vict, nullptr, 0, 1);
                continue;
            } else if (dge + rand_number(-15, 5) > skill) {
                act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@WYou manage to escape the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, 0, 1);
                improve_skill(vict, SKILL_DODGE, 0);
                continue;
            } else {
                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@RYou are caught by the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING) {
                    handle_knockdown(vict);
                }
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
                continue;
            }
        } /* Hitting targets! */

        pcost(ch, attperc, 0);

        improve_skill(ch, SKILL_BAKUHATSUHA, 0);
        return;
    } /* We have targets! Attempt to kills them! */

}

ACMD(do_rogafufuken) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .05, minimum = .01;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_ROGAFUFUKEN)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, GET_MAX_HIT(ch) / 50)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_ROGAFUFUKEN); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_ROGAFUFUKEN, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Rogafufuken")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your rogafufuken with a punch of $s own!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W rogafufuken with a punch of your own!@n", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W parries @c$n's@W rogafufuken with a punch of $s own!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, GET_MAX_HIT(vict) / 300);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your rogafufuken!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W rogafufuken!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W rogafufuken!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 23, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your rogafufuken!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W rogafufuken!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W rogafufuken!@n", false, ch, nullptr, vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your rogafufuken misses!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a rogafufuken at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a rogafufuken at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your rogafufuken misses!@n", false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a rogafufuken at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a rogafufuken at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 23, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou pour your charged energy into your hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as you leap towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. You unleash a flurry of hand strikes on @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards YOU while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou pour your charged energy into your hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as you leap towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. You unleash a flurry of hand strikes on @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards YOU while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou pour your charged energy into your hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as you leap towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. You unleash a flurry of hand strikes on @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards YOU while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou pour your charged energy into your hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as you leap towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. You unleash a flurry of hand strikes on @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards YOU while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou pour your charged energy into your hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as you leap towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. You unleash a flurry of hand strikes on @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards YOU while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W pours $s charged energy into $s hands and feet. A @rr@Re@wd@W glow trails behind the movements of either as $e leaps towards @c$N@W while yelling '@cW@Co@Wl@wf @DFang @rFist@W!'. $e unleashes a flurry of hand strikes on @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "hands and feet");
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 23, skill, attperc);
        dmg /= 10;
        act("@WYou fire a rogafufuken at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a rogafufuken at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_dualbeam) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .1, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_DUALBEAM)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_DUALBEAM); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_DUALBEAM, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Dualbeam")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        int hits = 3;
        while (hits > 0) {
            hits -= 1;
            if (hits == 1) {
                prob -= prob / 5;
            } else if (hits <= 0) {
                return;
            } else if (!vict) {
                return;
            } else if ((AFF_FLAGGED(vict, AFF_SPIRIT) || GET_HIT(vict) <= 0) && !IS_NPC(vict)) {
                return;
            }
            if (prob < perc - 20) {
                if ((vict->getCurST()) > 0) {
                    if (blk > axion_dice(10)) {
                        act("@C$N@W moves quickly and blocks your dualbeam!@n", false, ch, nullptr, vict, TO_CHAR);
                        act("@WYou move quickly and block @C$n's@W dualbeam!@n", false, ch, nullptr, vict, TO_VICT);
                        act("@C$N@W moves quickly and blocks @c$n's@W dualbeam!@n", false, ch, nullptr, vict,
                            TO_NOTVICT);
                        if (hits == 1) {
                            pcost(ch, attperc, 0);
                        }
                        pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                        dmg = damtype(ch, 22, skill, attperc);
                        dmg /= 4;
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);

                        continue;
                    } else if (dge > axion_dice(10)) {
                        act("@C$N@W manages to dodge your dualbeam, letting it slam into the surroundings!@n", false,
                            ch, nullptr, vict, TO_CHAR);
                        act("@WYou dodge @C$n's@W dualbeam, letting it slam into the surroundings!@n", false, ch,
                            nullptr,
                            vict, TO_VICT);
                        act("@C$N@W manages to dodge @c$n's@W dualbeam, letting it slam into the surroundings!@n",
                            false, ch, nullptr, vict, TO_NOTVICT);
                        send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                        dodge_ki(ch, vict, 0, 22, skill, SKILL_DUALBEAM); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                        ch->getRoom()->modDamage(5);
                        improve_skill(vict, SKILL_DODGE, 0);
                        if (hits == 1) {
                            pcost(ch, attperc, 0);
                        }
                        hurt(0, 0, ch, vict, nullptr, 0, 1);

                        continue;
                    } else {
                        act("@WYou can't believe it but your dualbeam misses, flying through the air harmlessly!@n",
                            false, ch, nullptr, vict, TO_CHAR);
                        act("@C$n@W fires a dualbeam at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                        act("@c$n@W fires a dualbeam at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                            TO_NOTVICT);

                        if (hits == 1) {
                            pcost(ch, attperc, 0);
                        }
                        hurt(0, 0, ch, vict, nullptr, 0, 1);

                        continue;
                    }
                } else {
                    act("@WYou can't believe it but your dualbeam misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a dualbeam at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a dualbeam at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (hits == 1) {
                        pcost(ch, attperc, 0);
                    }

                }
                hurt(0, 0, ch, vict, nullptr, 0, 1);
                continue;
            } else {
                dmg = damtype(ch, 22, skill, attperc);
                int hitspot = 1;
                hitspot = roll_hitloc(ch, vict, skill);
                if (GET_SKILL(ch, SKILL_DUALBEAM) >= 100) {
                    if (rand_number(1, 100) >= 60) {
                        hitspot = 2;
                    }
                } else if (GET_SKILL(ch, SKILL_DUALBEAM) >= 60) {
                    if (rand_number(1, 100) >= 80) {
                        hitspot = 2;
                    }
                } else if (GET_SKILL(ch, SKILL_DUALBEAM) >= 40) {
                    if (rand_number(1, 100) >= 95) {
                        hitspot = 2;
                    }
                }
                switch (hitspot) {
                    case 1:
                        act("@WYou gather your charged energy up through the circuits in your arms. A @gg@Gr@We@wen @Wglow appears around your hand right as you aim it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S body in that instant!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at YOU. A @gg@Gr@We@wen @Wbeam blasts out and slams into YOUR body in that instant!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S body in that instant!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        if (GET_BONUS(ch, BONUS_SOFT)) {
                            dmg *= calc_critical(ch, 2);
                        }
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 4);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 2: /* Critical */
                        act("@WYou gather your charged energy up through the circuits in your arms. A @gg@Gr@We@wen @Wglow appears around your hand right as you aim it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S face in that instant!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at YOU. A @gg@Gr@We@wen @Wbeam blasts out and slams into YOUR face in that instant!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S face in that instant!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        dmg *= calc_critical(ch, 0);
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 3);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 3:
                        act("@WYou gather your charged energy up through the circuits in your arms. A @gg@Gr@We@wen @Wglow appears around your hand right as you aim it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S gut in that instant!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at YOU. A @gg@Gr@We@wen @Wbeam blasts out and slams into YOUR gut in that instant!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S gut in that instant!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        if (GET_BONUS(ch, BONUS_SOFT)) {
                            dmg *= calc_critical(ch, 2);
                        }
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 4);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 4: /* Weak */
                        act("@WYou gather your charged energy up through the circuits in your arms. A @gg@Gr@We@wen @Wglow appears around your hand right as you aim it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S arm in that instant!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at YOU. A @gg@Gr@We@wen @Wbeam blasts out and slams into YOUR arm in that instant!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S arm in that instant!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        dmg *= calc_critical(ch, 1);
                        hurt(0, 190, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 1);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 5: /* Weak 2 */
                        act("@WYou gather your charged energy up through the circuits in your arms. A @gg@Gr@We@wen @Wglow appears around your hand right as you aim it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S leg in that instant!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at YOU. A @gg@Gr@We@wen @Wbeam blasts out and slams into YOUR leg in that instant!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@C$n@W gathers $s charged energy up through the circuits in $s arms. A @gg@Gr@We@wen @Wglow appears around $s hand right as $e aims it at @c$N@W. A @gg@Gr@We@wen @Wbeam blasts out and slams into $S leg in that instant!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        dmg *= calc_critical(ch, 1);
                        hurt(1, 190, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 2);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                }
                if (hits == 1) {
                    pcost(ch, attperc, 0);
                }
                if (GET_HIT(vict) <= 0) {
                    if (hits == 2) {
                        pcost(ch, attperc, 0);
                    }
                    hits = 0;
                }

                continue;
            }
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 22, skill, attperc);
        dmg /= 10;
        act("@WYou fire a dualbeam at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a dualbeam at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

/* Chimera */
ACMD(do_blessedhammer) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .05, minimum = .01;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BLESSEDHAMMER)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_BLESSEDHAMMER); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_BLESSEDHAMMER, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob += 15; /* kousengan bonus */

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "@WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@C")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@n!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@n!@n", false,
                        ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@n!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 42, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 17, skill, SKILL_BLESSEDHAMMER); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W at you, but misses!@n ", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@c$n@W fires a @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W at @C$N@W, but somehow misses!@n ",
                        false, ch, nullptr, vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W at you, but misses!@n", false, ch,
                    nullptr,
                    vict, TO_VICT);
                act("@c$n@W fires a @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr@W at @C$N@W, but somehow misses!@n", false,
                    ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 42, skill, attperc);
            if (AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                dmg *= calc_critical(ch, 1);
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WFocusing your attention on @R$N@W, you reach back and form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer before hurling it with all your might into their chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W narrows $s eyes and focuses $s energy. They reach back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at you with all their @Rmight@W into YOUR chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W narrows their eyes at @c$N@W. $n reaches back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at @c$N@W with all their might into @c$N@W's chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    if (!GET_BONUS(ch, BONUS_POWERHIT)) {
                        dmg *= 3;
                    } else {
                        dmg *= 5;
                    }
                    act("@WFocusing your attention on @R$N@W, you reach back and form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer before hurling it with all your might into their face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W narrows $s eyes and focuses $s energy. They reach back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at you with all their @Rmight@W into YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W narrows their eyes at @c$N@W. $n reaches back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at @c$N@W with all their might into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WFocusing your attention on @R$N@W, you reach back and form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer before hurling it with all your might into their gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W narrows $s eyes and focuses $s energy. They reach back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at you with all their @Rmight@W into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W narrows their eyes at @c$N@W. $n reaches back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at @c$N@W with all their might into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WFocusing your attention on @R$N@W, you reach back and form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer before hurling it with all your might into their arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W narrows $s eyes and focuses $s energy. They reach back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at you with all their @Rmight@W into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W narrows their eyes at @c$N@W. $n reaches back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at @c$N@W with all their might into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WFocusing your attention on @R$N@W, you reach back and form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer before hurling it with all your might into their leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W narrows $s eyes and focuses $s energy. They reach back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at you with all their @Rmight@W into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W narrows their eyes at @c$N@W. $n reaches back to form an @ne@Dt@nh@De@nr@De@na@Dl @Whammer which they hurl at @c$N@W with all their might into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!AFF_FLAGGED(vict, AFF_BURNED) && rand_number(1, 4) == 3 && !IS_DEMON(vict)) {
                send_to_char(vict, "@RYou are burned by the attack!@n\r\n");
                send_to_char(ch, "@RThey are burned by the attack!@n\r\n");
                vict->affected_by.set(AFF_BURNED);
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 42, skill, attperc);
        dmg /= 10;
        act("@WYou fire a @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a @WB@Dl@We@Ds@Ws@De@Wd @DH@Wa@Dm@Wm@De@Wr at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_kousengan) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .05, minimum = .01;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KOUSENGAN)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_KOUSENGAN); /* Set skill value */



    /* There is a player/mob targeted */
    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_KOUSENGAN, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob += 15; /* kousengan bonus */

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "kousengan")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your kousengan!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W kousengan!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W kousengan!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 42, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your kousengan, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W kousengan, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W kousengan, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 17, skill, SKILL_KOUSENGAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your kousengan misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a kousengan at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a kousengan at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your kousengan misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a kousengan at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a kousengan at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 42, skill, attperc);
            if (AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                dmg *= calc_critical(ch, 1);
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou look at @c$N@W and grin. Then bright @Mpink@W lasers shoot from your eyes and slam into $S chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W looks at YOU and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into YOUR chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W looks at @c$N@W and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into @c$N@W's chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    if (!GET_BONUS(ch, BONUS_POWERHIT)) {
                        dmg *= 3;
                    } else {
                        dmg *= 5;
                    }
                    act("@WYou look at @c$N@W and grin. Then bright @Mpink@W lasers shoot from your eyes and slam into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W looks at YOU and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W looks at @c$N@W and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou look at @c$N@W and grin. Then bright @Mpink@W lasers shoot from your eyes and slam into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W looks at YOU and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W looks at @c$N@W and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou look at @c$N@W and grin. Then bright @Mpink@W lasers shoot from your eyes and slam into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W looks at YOU and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W looks at @c$N@W and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou look at @c$N@W and grin. Then bright @Mpink@W lasers shoot from your eyes and slam into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W looks at YOU and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W looks at @c$N@W and grins. Then bright @Mpink@W lasers shoot from $s eyes and slam into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!AFF_FLAGGED(vict, AFF_BURNED) && rand_number(1, 4) == 3 && !IS_DEMON(vict)) {
                send_to_char(vict, "@RYou are burned by the attack!@n\r\n");
                send_to_char(ch, "@RThey are burned by the attack!@n\r\n");
                vict->affected_by.set(AFF_BURNED);
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 42, skill, attperc);
        dmg /= 10;
        act("@WYou fire a kousengan at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a kousengan at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_deathbeam) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .1, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_DEATHBEAM)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_DEATHBEAM); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_DEATHBEAM, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob += 15; /* deathbeam bonus */

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Deathbeam")) {
            if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                WAIT_STATE(ch, PULSE_3SEC);
            }

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your deathbeam!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W deathbeam!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W deathbeam!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 17, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your deathbeam, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W deathbeam, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W deathbeam, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 17, skill, SKILL_DEATHBEAM); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }

                    return;
                } else {
                    act("@WYou can't believe it but your deathbeam misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a deathbeam at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a deathbeam at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }

                    return;
                }
            } else {
                act("@WYou can't believe it but your deathbeam misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a deathbeam at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a deathbeam at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                WAIT_STATE(ch, PULSE_3SEC);
            }
            return;
        } else {
            dmg = damtype(ch, 17, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou move swiftly, drawing your charged ki to your index finger, and point at @c$N@W! You fire a @Rred@W Deathbeam from your finger which slams into $S body and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and point at you@W! $e fires a @Rred@W Deathbeam from $s finger which slams into your body and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and point at @c$N@W! $e fires a @Rred@W Deathbeam from $s finger which slams into @c$N@W's body and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
                case 2: /* Critical */
                    act("@WYou move swiftly, drawing your charged ki to your index finger, and point at @c$N@W! You fire a @Rred@W Deathbeam from your finger which slams into $S face and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at you@W! $e fires a @Rred@W Deathbeam from $s finger which slams into your face and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at @c$N@W! $e fires a @Rred@W Deathbeam from $s finger which slams into @c$N@W's face and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
                case 3:
                    act("@WYou move swiftly, drawing your charged ki to your index finger, and point at @c$N@W! You fire a @Rred@W Deathbeam from your finger which slams into $S gut and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at you@W! $e fires a @Rred@W Deathbeam from $s finger which slams into your gut and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at @c$N@W! $e fires a @Rred@W Deathbeam from $s finger which slams into @c$N@W's gut and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
                case 4: /* Weak */
                    act("@WYou move swiftly, drawing your charged ki to your index finger, and point at @c$N@W! You fire a @Rred@W Deathbeam from your finger which slams into $S arm and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at you@W! $e fires a @Rred@W Deathbeam from $s finger which slams into your arm and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at @c$N@W! $e fires a @Rred@W Deathbeam from $s finger which slams into @c$N@W's arm and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
                case 5: /* Weak 2 */
                    act("@WYou move swiftly, drawing your charged ki to your index finger, and point at @c$N@W! You fire a @Rred@W Deathbeam from your finger which slams into $S leg and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at you@W! $e fires a @Rred@W Deathbeam from $s finger which slams into your leg and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W moves swiftly, drawing charged ki to $s index finger, and points at @c$N@W! $e fires a @Rred@W Deathbeam from $s finger which slams into @c$N@W's leg and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
            }
            if (GET_SKILL(ch, SKILL_DEATHBEAM) >= 100 && GET_HIT(vict) >= 2) {
                ch->decCurLF(dmg * .4, -1);
            } else if (GET_SKILL(ch, SKILL_DEATHBEAM) >= 60 && GET_HIT(vict) >= 2) {
                ch->decCurLF(dmg * .2, -1);
            } else if (GET_SKILL(ch, SKILL_DEATHBEAM) >= 40 && GET_HIT(vict) >= 2) {
                ch->decCurLF(dmg * .05, -1);
            }
            if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 17, skill, attperc);
        dmg /= 10;
        act("@WYou fire a deathbeam at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a deathbeam at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
        if (GET_SKILL_PERF(ch, SKILL_DEATHBEAM) == 3) {
            WAIT_STATE(ch, PULSE_3SEC);
        }

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_dodonpa) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .1, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_DODONPA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }
    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_DODONPA); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_DODONPA, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3) {
            WAIT_STATE(ch, PULSE_3SEC);
        }

        if (!tech_handle_zanzoken(ch, vict, "Dodonpa")) {
            if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your dodonpa!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W dodonpa!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W dodonpa!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 15, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your dodonpa, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W dodonpa, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W dodonpa, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 15, skill, SKILL_DODONPA); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your dodonpa misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a dodonpa at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a dodonpa at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your dodonpa misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a dodonpa at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a dodonpa at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 15, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou gather your charged ki into your index finger and point at @c$N@W. A @Ygolden@W glow forms around your finger tip right before you shout '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fire a large golden beam! The beam slams into $S body and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at you. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into your body and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at @c$N@W. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into @c$N@W's body and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou gather your charged ki into your index finger and point at @c$N@W. A @Ygolden@W glow forms around your finger tip right before you shout '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fire a large golden beam! The beam slams into $S face and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at you. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into your face and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at @c$N@W. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into @c$N@W's face and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou gather your charged ki into your index finger and point at @c$N@W. A @Ygolden@W glow forms around your finger tip right before you shout '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fire a large golden beam! The beam slams into $S gut and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at you. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into your gut and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at @c$N@W. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into @c$N@W's gut and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou gather your charged ki into your index finger and point at @c$N@W. A @Ygolden@W glow forms around your finger tip right before you shout '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fire a large golden beam! The beam slams into $S arm and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at you. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into your arm and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at @c$N@W. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into @c$N@W's arm and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou gather your charged ki into your index finger and point at @c$N@W. A @Ygolden@W glow forms around your finger tip right before you shout '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fire a large golden beam! The beam slams into $S leg and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at you. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into your leg and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W gathers $s charged ki into $s index finger and points at @c$N@W. A @Ygolden@W glow forms around the finger tip right before $e shouts '@YD@yo@Yd@yo@Yn@yp@Ya@W!' and fires a large golden beam! The beam slams into @c$N@W's leg and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (rand_number(1, 3) == 2) {
                vict->decCurKI(dmg / 4);
                send_to_char(vict, "@RYou feel some of your ki drained away by the attack!@n\r\n");
            }
            if (GET_SKILL_PERF(ch, SKILL_DODONPA) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 15, skill, attperc);
        dmg /= 10;
        act("@WYou fire a dodonpa at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a dodonpa at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_masenko) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .15, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_MASENKO)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_MASENKO); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_MASENKO, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Masenko")) {
            if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your masenko!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W masenko!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W masenko!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 14, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your masenko, letting it slam into the surroundings!@n", false, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W masenko, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W masenko, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 14, skill, SKILL_MASENKO); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }

                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your masenko misses, flying through the air harmlessly!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a masenko at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a masenko at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your masenko misses, flying through the air harmlessly!@n", false, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a masenko at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a masenko at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 14, skill, attperc);
            if (GET_SKILL(ch, SKILL_MASENKO) >= 100) {
                dmg += dmg * 0.08;
            } else if (GET_SKILL(ch, SKILL_MASENKO) >= 100) {
                dmg += dmg * 0.05;
            } else if (GET_SKILL(ch, SKILL_MASENKO) >= 100) {
                dmg += dmg * 0.03;
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            if (IS_PICCOLO(ch)) {
                if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100)
                    dmg += (GET_MAX_MANA(ch) * 0.08);
                else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 60)
                    dmg += (GET_MAX_MANA(ch) * 0.05);
                else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40)
                    dmg += (GET_MAX_MANA(ch) * 0.03);
            }
            switch (hitspot) {
                case 1:
                    act("@WYou place your hands on top of each other and raise them over your head. Energy that you have charged gathers into your palms just before you bring them down aimed at @c$N@W! You scream '@RMasenko @rHa!@W' as you release your attack. A bright red wave of energy flows from your hands and slams into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at you! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at @c$N@W! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into @c$N@W's body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou place your hands on top of each other and raise them over your head. Energy that you have charged gathers into your palms just before you bring them down aimed at @c$N@W! You scream '@RMasenko @rHa!@W' as you release your attack. A bright red wave of energy flows from your hands and slams into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at you! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at @c$N@W! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into @c$N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou place your hands on top of each other and raise them over your head. Energy that you have charged gathers into your palms just before you bring them down aimed at @c$N@W! You scream '@RMasenko @rHa!@W' as you release your attack. A bright red wave of energy flows from your hands and slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at you! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into YOUR gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at @c$N@W! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou place your hands on top of each other and raise them over your head. Energy that you have charged gathers into your palms just before you bring them down aimed at @c$N@W! You scream '@RMasenko @rHa!@W' as you release your attack. A bright red wave of energy flows from your hands and slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at you! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at @c$N@W! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou place your hands on top of each other and raise them over your head. Energy that you have charged gathers into your palms just before you bring them down aimed at @c$N@W! You scream '@RMasenko @rHa!@W' as you release your attack. A bright red wave of energy flows from your hands and slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at you! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W places $s hands on top of each other and raises them over $s head. Energy that $e had charged gathers into $s palms just before $e brings them down aimed at @c$N@W! $e screams '@RMasenko @rHa!@W' as $e releases $s attack. A bright red wave of energy flows from $s hands and slams into @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (rand_number(1, 2) == 2 && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                send_to_char(vict, "@RThe attack seems to have taken a toll on your stamina!@n\r\n");
                vict->decCurST(dmg / 4);
            }
            if (GET_SKILL_PERF(ch, SKILL_MASENKO) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 14, skill, attperc);
        dmg /= 10;
        act("@WYou fire a masenko at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a masenko at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_kamehameha) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .15, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KAMEHAMEHA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
        minimum -= 0.05;
        if (minimum <= 0.0) {
            minimum = 0.01;
        }
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_KAMEHAMEHA); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_KAMEHAMEHA, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "KameHameHa Wave")) {
            if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
                ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
            } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
                ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
            } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
                ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
            }
            if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                WAIT_STATE(ch, PULSE_3SEC);
            }
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your kamehameha!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W kamehameha!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W kamehameha!@n", false, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
                    } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                    } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                    }
                    dmg = damtype(ch, 13, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your kamehameha, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W kamehameha, letting it slam into the surroundings!@n", false, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W kamehameha, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 13, skill, SKILL_KAMEHAMEHA); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
                    } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                    } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                    }
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your kamehameha misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a kamehameha at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a kamehameha at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
                    } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                    } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
                        ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your kamehameha misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a kamehameha at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a kamehameha at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
                if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
                    ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
                } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
                    ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
                } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
                    ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
                }
            }
            if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                WAIT_STATE(ch, PULSE_3SEC);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 13, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou cup your hands at your side and begin to pool your charged ki there. As the ki pools there you begin to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly you bring your hands forward facing @c$N@W and shout '@BHAAAA!!!@W' while releasing a bright blue kamehameha at $M! It slams into $S body and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing you and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into your body and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing @c$N@W and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into $S body and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou cup your hands at your side and begin to pool your charged ki there. As the ki pools there you begin to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly you bring your hands forward facing @c$N@W and shout '@BHAAAA!!!@W' while releasing a bright blue kamehameha at $M! It slams into $S face and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing you and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into your face and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing @c$N@W and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into $S face and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou cup your hands at your side and begin to pool your charged ki there. As the ki pools there you begin to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly you bring your hands forward facing @c$N@W and shout '@BHAAAA!!!@W' while releasing a bright blue kamehameha at $M! It slams into $S gut and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing you and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into your gut and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing @c$N@W and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into $S gut and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
                case 4: /* Weak */
                    act("@WYou cup your hands at your side and begin to pool your charged ki there. As the ki pools there you begin to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly you bring your hands forward facing @c$N@W and shout '@BHAAAA!!!@W' while releasing a bright blue kamehameha at $M! It slams into $S arm and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing you and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into your arm and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing @c$N@W and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into $S arm and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou cup your hands at your side and begin to pool your charged ki there. As the ki pools there you begin to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly you bring your hands forward facing @c$N@W and shout '@BHAAAA!!!@W' while releasing a bright blue kamehameha at $M! It slams into $S leg and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing you and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into your leg and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W cups $s hands at $s side and begins to pool charged ki there. As the ki pools there $e begins to chant, '@BKaaaaa@bmeeee@Bhaaaaaaa@bmeeeee@W'. Suddenly $e brings $s hands forward facing @c$N@W and shouts '@BHAAAA!!!@W' while releasing a bright blue kamehameha! It slams into $S leg and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    break;
            }
            if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
                ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
            } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
                ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
            } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
                ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
            }

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 13, skill, attperc);
        dmg /= 10;
        act("@WYou fire a kamehameha at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a kamehameha at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        if (GET_SKILL_PERF(ch, SKILL_KAMEHAMEHA) == 3) {
            WAIT_STATE(ch, PULSE_3SEC);
        }
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
        if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 100) {
            ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.25);
        } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 60) {
            ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.1);
        } else if (GET_SKILL(ch, SKILL_KAMEHAMEHA) >= 40) {
            ch->incCurKI((GET_MAX_MANA(ch) * attperc) * 0.05);
        }
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_renzo) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_RENZO)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }
    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_RENZO); /* Set skill value */



    /* There is a player/mob targeted */
    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_RENZO, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        /* Renzokou Energy Dan Mastery */

        int master_roll = rand_number(1, 100), master_chance = 0, half_chance = 0, master_pass = 0;

        if (skill >= 100) {
            master_chance = 10;
            half_chance = 20;
        } else if (skill >= 75) {
            master_chance = 5;
            half_chance = 10;
        } else if (skill >= 50) {
            master_chance = 5;
            half_chance = 5;
        }

        if (master_chance >= master_roll)
            master_pass = 1;
        else if (half_chance >= master_roll)
            master_pass = 2;

        if (master_pass == 1)
            send_to_char(ch, "@GYour mastery of the technique has made your use of energy more efficient!@n\r\n");
        else if (master_pass == 2)
            send_to_char(ch,
                         "@GYour mastery of the technique has made your use of energy as efficient as possible!@n\r\n");

        /*=============================*/

        if (!tech_handle_zanzoken(ch, vict, "Renzokou Energy Dan")) {
            if (master_pass == 1)
                pcost(ch, attperc * 0.25, 0);
            else if (master_pass == 2)
                pcost(ch, attperc * 0.5, 0);
            else
                pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        int count = 0;
        if (prob + 20 < perc) {
            count = 0;
        } else if (prob + 15 < perc) {
            count = 10;
        } else if (prob + 10 < perc) {
            count = 25;
        } else if (prob + 5 < perc) {
            count = 50;
        } else if (prob < perc - 20) {
            count = 75;
        } else if (prob > perc) {
            count = 100;
        }
        if (count > 0) {
            if (IS_NAIL(ch)) {
                if (GET_SKILL(ch, SKILL_RENZO) >= 100) {
                    count += 200;
                } else if (GET_SKILL(ch, SKILL_RENZO) >= 60) {
                    count += 100;
                } else if (GET_SKILL(ch, SKILL_RENZO) >= 40) {
                    count += 40;
                }
            }
            if (rand_number(1, 5) >= 5) { /* Random boost or neg for everyone */
                count += rand_number(-15, 25);
            }
        }
        if (count == 0) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W deflects every shot of your renzokou energy dan, sending them flying away!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou deflect every shot @C$n's@W renzokou energy dan sending them flying away!@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W deflects every shot of @c$n's@W renzokou energy dan sending them flying away!@n", true,
                        ch, nullptr, vict, TO_NOTVICT);
                    if (master_pass == 1)
                        pcost(ch, attperc * 0.25, 0);
                    else if (master_pass == 2)
                        pcost(ch, attperc * 0.5, 0);
                    else
                        pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 200);


                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks every renzokou energy dan shot with $S arms!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W renzokou energy dan!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W renzokou energy dan with $S arms!@n", true, ch,
                        nullptr,
                        vict, TO_NOTVICT);
                    if (master_pass == 1)
                        pcost(ch, attperc * 0.25, 0);
                    else if (master_pass == 2)
                        pcost(ch, attperc * 0.5, 0);
                    else
                        pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 12, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge all your renzokou energy dan shots, letting them slam into the surroundings!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge all of @C$n's @Wrenzokou energy dan shots, letting them slam into the surroundings!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge all @c$n's@W renzokou energy dan shots, letting them slam into the surroundings!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (master_pass == 1)
                        pcost(ch, attperc * 0.25, 0);
                    else if (master_pass == 2)
                        pcost(ch, attperc * 0.5, 0);
                    else
                        pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but all your renzokou energy dan shots miss, flying through the air harmlessly!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires hundreds of renzokou energy dan shots at you, but misses!@n ", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@c$n@W fires hundreds of renzokou energy dan shots at @C$N@W, but somehow misses!@n ", true,
                        ch, nullptr, vict, TO_NOTVICT);
                    if (master_pass == 1)
                        pcost(ch, attperc * 0.25, 0);
                    else if (master_pass == 2)
                        pcost(ch, attperc * 0.5, 0);
                    else
                        pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but all your renzokou energy dan shots miss, flying through the air harmlessly!@n",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires hundreds of renzokou energy dan shots at you, but misses!@n", true, ch, nullptr, vict,
                    TO_VICT);
                act("@c$n@W fires hundreds of renzokou energy dan shots at @C$N@W, but somehow misses!@n", true, ch,
                    nullptr,
                    vict, TO_NOTVICT);
                if (master_pass == 1)
                    pcost(ch, attperc * 0.25, 0);
                else if (master_pass == 2)
                    pcost(ch, attperc * 0.5, 0);
                else
                    pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 12, skill, attperc);
            if (count >= 100) {
                dmg = dmg * 0.01;
                dmg *= count;
                act("@WYou gather your charged energy into your hands as a golden glow appears around each. You slam your hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! All of the shots hit!",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at you! All of the shots hit!",
                    true, ch, nullptr, vict, TO_VICT);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! All of the shots hit!",
                    true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
            }
            if (count >= 75 && count < 100) {
                dmg = dmg * 0.01;
                dmg *= count;
                act("@WYou gather your charged energy into your hands as a golden glow appears around each. You slam your hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! Most of the shots hit, but some of them are avoided by $M!",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at you! Most of the shots hit, but some of them you manage to avoid!",
                    true, ch, nullptr, vict, TO_VICT);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! Most of the shots hit, but some of them are avoided by $M!",
                    true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
            }
            if (count >= 50 && count < 75) {
                dmg = dmg * 0.01;
                dmg *= count;
                act("@WYou gather your charged energy into your hands as a golden glow appears around each. You slam your hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! About half of the shots hit, the rest are avoided by $M!",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at you! About half of the shots hit, the rest you manage to avoid!",
                    true, ch, nullptr, vict, TO_VICT);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! About half of the shots hit, the rest are avoided by $M!",
                    true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
            }
            if (count >= 25 && count < 50) {
                dmg = dmg * 0.01;
                dmg *= count;
                act("@WYou gather your charged energy into your hands as a golden glow appears around each. You slam your hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! Few of the shots hit, the rest are avoided by $M!",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at you! Few of the shots hit, the rest you manage to avoid!",
                    true, ch, nullptr, vict, TO_VICT);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! Few of the shots hit, the rest are avoided by $M!",
                    true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
            }
            if (count >= 10 && count < 25) {
                dmg /= 100;
                dmg *= count;
                act("@WYou gather your charged energy into your hands as a golden glow appears around each. You slam your hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! Very few of the shots hit, the rest are avoided by $M!",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at you! Very few of the shots hit, the rest you manage to avoid!",
                    true, ch, nullptr, vict, TO_VICT);
                act("@w$n gathers charged energy into $s hands as a golden glow appears around each. $e slams $s hands forward rapidly firing hundreds of Renzokou Energy Dan shots at $N@W! Very few of the shots hit, the rest are avoided by $M!",
                    true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, dmg, 1);
            }
            if (master_pass == 1)
                pcost(ch, attperc * 0.25, 0);
            else if (master_pass == 2)
                pcost(ch, attperc * 0.5, 0);
            else
                pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 12, skill, attperc);
        dmg /= 10;
        act("@WYou fire hundreds of renzokou energy dan shots at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires hundreds of renzokou energy dan shots at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_heeldrop) {
    int prob, perc, avo, index = 0, pry = 0, dge = 0, blk = 0, skill = 0;
    int64_t dmg, stcost = physical_cost(ch, SKILL_HEELDROP);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);


    if (!check_skill(ch, SKILL_HEELDROP)) {
        return;
    }
    if (!can_grav(ch)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 90)) {
        return;
    }

    skill = init_skill(ch, SKILL_HEELDROP);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (IS_PICCOLO(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            handle_cooldown(ch, 7);
        else
            handle_cooldown(ch, 9);
    } else {
        handle_cooldown(ch, 9);
    }
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_HEELDROP, 1);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        if (IS_KABITO(ch) && !IS_NPC(ch)) {
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                perc -= perc * 0.2;
        }

        index -= handle_speed(ch, vict);

        avo = index / 4;
        if (!IS_NPC(vict)) {
            pry = handle_parry(vict);
            blk = GET_SKILL(vict, SKILL_BLOCK);
            dge = GET_SKILL(vict, SKILL_DODGE);
        }
        if (IS_NPC(vict) && GET_LEVEL(ch) <= 10) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(20, 40);
                blk = rand_number(20, 40);
            }
            dge = rand_number(20, 40);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 20) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(20, 60);
                blk = rand_number(20, 60);
            }
            dge = rand_number(20, 60);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 30) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(20, 70);
                blk = rand_number(20, 70);
            }
            dge = rand_number(20, 70);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 40) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(30, 70);
                blk = rand_number(30, 70);
            }
            dge = rand_number(30, 70);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 40) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(40, 70);
                blk = rand_number(40, 70);
            }
            dge = rand_number(40, 70);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 60) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(40, 80);
                blk = rand_number(40, 80);
            }
            dge = rand_number(40, 80);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 70) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(50, 80);
                blk = rand_number(50, 80);
            }
            dge = rand_number(50, 80);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 80) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(50, 90);
                blk = rand_number(50, 90);
            }
            dge = rand_number(50, 90);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 90) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(60, 90);
                blk = rand_number(60, 90);
            }
            dge = rand_number(60, 90);
        } else if (IS_NPC(vict) && GET_LEVEL(ch) <= 100) {
            if (IS_HUMANOID(vict)) {
                pry = rand_number(70, 100);
                blk = rand_number(70, 100);
            }
            dge = rand_number(70, 100);
        }
        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Heeldrop")) {
            COMBO(ch) = -1;
            COMBHITS(ch) = 0;
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your heeldrop with a punch of $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W heeldrop with a punch of your own!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W parries @c$n's@W heeldrop with a punch of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your heeldrop!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W heeldrop!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W heeldrop!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 8, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your heeldrop!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W heeldrop!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W heeldrop!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it but your heeldrop misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves to heeldrop you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to heeldrop @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it but your heeldrop misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W moves to heeldrop you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to heeldrop @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 8, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou disappear, appearing above @C$N@W you spin and heeldrop $M in the face!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@C$n@W disappears, only to appear above you, spinning quickly and heeldropping you in the face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W disappears, only to appear above @C$N@W, spinning quickly and heeldropping $M in the face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    tech_handle_crashdown(ch, vict);

                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou disappear, reappearing in front of @C$N@W, you flip upside down and slam your heel into the top of $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W disappears, reappearing in front of you, $e flips upside down and slams $s heel into the top of your head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W disappears, reappearing in front of @C$N@W, $e flips upside down and slams $s heel into the top of @C$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    tech_handle_crashdown(ch, vict);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou fly at @C$N@W, heeldropping $S gut as you fly!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W flies at you, heeldropping your gut as $e flies!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W flies at @C$N@W, heeldropping $S gut as $e flies!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou heeldrop @C$N@W, hitting $M in the arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W heeldrops you, hitting you in the arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W heeldrops @C$N@W, hitting $M in the arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou heeldrop @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W heeldrops your leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W heeldrops @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 180, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "leg");
            pcost(ch, 0, stcost);
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou heeldrop $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W heeldrops $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_attack) {
    int prob, perc, avo, index = 0, pry = 0, dge = 0, blk = 0, skill = 0, wtype = 0, gun = false, gun2 = false;
    int dualwield = 0, wielded = 0, guncost = 0;
    int64_t stcost = (GET_MAX_HIT(ch) / 150);
    int64_t dmg;
    struct char_data *vict;
    struct obj_data *obj = nullptr;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    if (IS_ANDROID(ch)) {
        stcost *= 0.25;
    }
    one_argument(argument, arg);

    if (!GET_EQ(ch, WEAR_WIELD1) && !GET_EQ(ch, WEAR_WIELD2)) {
        send_to_char(ch,
                     "You need to wield a weapon to use this, without one try punch, kick, or other no weapon attacks.\r\n");
        return;
    }

    if (!can_grav(ch)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }
    if (GET_EQ(ch, WEAR_WIELD1)) {
        if (!IS_ANDROID(ch)) {
            stcost += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD1));
        } else {
            stcost += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD1)) * 0.25;
        }
        if (!check_points(ch, 0, stcost)) {
            return;
        }
        wielded = 1;
    } else if (GET_EQ(ch, WEAR_WIELD2)) {
        if (!IS_ANDROID(ch)) {
            stcost += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD2));
        } else {
            stcost += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD2)) * 0.25;
        }
        if (!check_points(ch, 0, stcost)) {
            return;
        }
    }

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (GET_EQ(ch, WEAR_WIELD1)) {
        if (vict) {
            if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
                if (!can_kill(ch, vict, nullptr, 1)) {
                    return;
                }
            } else {
                if (!can_kill(ch, vict, nullptr, 0)) {
                    return;
                }
            }
        }
        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_PIERCE - TYPE_HIT) {
            skill = init_skill(ch, SKILL_DAGGER);
            improve_skill(ch, SKILL_DAGGER, 1);
            wtype = 1;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_SLASH - TYPE_HIT) {
            skill = init_skill(ch, SKILL_SWORD);
            improve_skill(ch, SKILL_SWORD, 1);
            wtype = 0;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_CRUSH - TYPE_HIT) {
            skill = init_skill(ch, SKILL_CLUB);
            improve_skill(ch, SKILL_CLUB, 1);
            wtype = 2;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_STAB - TYPE_HIT) {
            skill = init_skill(ch, SKILL_SPEAR);
            improve_skill(ch, SKILL_SPEAR, 1);
            wtype = 3;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
            gun = true;
            skill = init_skill(ch, SKILL_GUN);
            improve_skill(ch, SKILL_GUN, 1);
            wtype = 4;
        } else {
            skill = init_skill(ch, SKILL_BRAWL);
            improve_skill(ch, SKILL_BRAWL, 1);
            wtype = 5;
        }
    } else if (GET_EQ(ch, WEAR_WIELD2)) {
        if (vict) {
            if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
                if (!can_kill(ch, vict, nullptr, 1)) {
                    return;
                }
            } else {
                if (!can_kill(ch, vict, nullptr, 0)) {
                    return;
                }
            }
        }
        if (wielded == 1)
            wielded = 2;
        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_PIERCE - TYPE_HIT) {
            skill = init_skill(ch, SKILL_DAGGER);
            improve_skill(ch, SKILL_DAGGER, 1);
            wtype = 1;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_SLASH - TYPE_HIT) {
            skill = init_skill(ch, SKILL_SWORD);
            improve_skill(ch, SKILL_SWORD, 1);
            wtype = 0;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_CRUSH - TYPE_HIT) {
            skill = init_skill(ch, SKILL_CLUB);
            improve_skill(ch, SKILL_CLUB, 1);
            wtype = 2;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_STAB - TYPE_HIT) {
            skill = init_skill(ch, SKILL_SPEAR);
            improve_skill(ch, SKILL_SPEAR, 1);
            wtype = 3;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
            gun2 = true;
            skill = init_skill(ch, SKILL_GUN);
            improve_skill(ch, SKILL_GUN, 1);
            wtype = 4;
        } else {
            skill = init_skill(ch, SKILL_BRAWL);
            improve_skill(ch, SKILL_BRAWL, 1);
            wtype = 5;
        }
    }
    if (wielded == 2 && gun == false) {
        if (GET_SKILL_BASE(ch, SKILL_DUALWIELD) >= 100) {
            dualwield = 3;
            stcost -= stcost * 0.30;
        } else if (GET_SKILL_BASE(ch, SKILL_DUALWIELD) >= 75) {
            dualwield = 2;
            stcost -= stcost * 0.25;
        }
    }

    int wlvl = 0;
    struct obj_data *weap = nullptr;
    if (GET_EQ(ch, WEAR_WIELD1)) {
        weap = GET_EQ(ch, WEAR_WIELD1);
    } else {
        weap = GET_EQ(ch, WEAR_WIELD2);
    }
    if (OBJ_FLAGGED(weap, ITEM_WEAPLVL1)) {
        wlvl = 1;
    } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL2)) {
        wlvl = 2;
    } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL3)) {
        wlvl = 3;
    } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL4)) {
        wlvl = 4;
    } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL5)) {
        wlvl = 5;
    }

    if (GET_PREFERENCE(ch) != PREFERENCE_H2H) {
        handle_cooldown(ch, 4);
    } else {
        handle_cooldown(ch, 8);
    }

    if (wielded == 1 && (gun == true || gun2 == true)) {
        if (wlvl == 5) {
            guncost = 12;
        } else if (wlvl == 4) {
            guncost = 6;
        } else if (wlvl == 3) {
            guncost = 4;
        } else if (wlvl == 2) {
            guncost = 2;
        }
        if (GET_GOLD(ch) < guncost) {
            send_to_char(ch, "You do not have enough zenni. You need %d zenni per shot for that level of gun.\r\n",
                         guncost);
            return;
        } else {
            ch->mod(CharMoney::Carried, -guncost);
        }
    } else if (wielded == 2 && gun == true) {
        if (wlvl == 5) {
            guncost = 12;
        } else if (wlvl == 4) {
            guncost = 6;
        } else if (wlvl == 3) {
            guncost = 4;
        } else if (wlvl == 2) {
            guncost = 2;
        }
        if (GET_GOLD(ch) < guncost) {
            send_to_char(ch, "You do not have enough zenni. You need %d zenni per shot for that level of gun.\r\n",
                         guncost);
            return;
        } else {
            ch->mod(CharMoney::Carried, -guncost);
        }
    }

    if (vict) {
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }

        index = check_def(vict);
        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;
        handle_defense(vict, &pry, &blk, &dge);

        if (gun == true) {
            if (dualwield >= 2) {
                prob += prob * 0.1;
            }
        }

        prob -= avo;
        if (PLR_FLAGGED(ch, PLR_THANDW)) {
            perc += 15;
        }
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "attack")) {
            if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
                pcost(ch, 0, stcost / 3);
            pcost(vict, 0, GET_MAX_HIT(vict) / 150);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W intercepts and parries your attack with $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou intercept and parry @C$n's@W attack with one of your own!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W intercepts and parries @c$n's@W attack with one of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (wtype != 4)
                        handle_disarm(ch, vict);
                    improve_skill(vict, SKILL_PARRY, 0);
                    if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
                        pcost(ch, 0, stcost);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
                        pcost(ch, 0, stcost);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, -1, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
                        pcost(ch, 0, stcost);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it but your attack misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves to attack you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to attack @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
                        pcost(ch, 0, stcost / 3);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it but your attack misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W moves to attack you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to attack @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
                    pcost(ch, 0, stcost / 3);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, -1, skill, attperc);
            if (OBJ_FLAGGED(weap, ITEM_WEAPLVL1)) {
                dmg += dmg * 0.05;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL2)) {
                dmg += dmg * 0.1;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL3)) {
                dmg += dmg * 0.2;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL4)) {
                dmg += dmg * 0.3;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL5)) {
                dmg += dmg * 0.5;
            }
            if (wtype == 5) {
                if (GET_SKILL(ch, SKILL_BRAWL) >= 100) {
                    dmg += dmg * 0.5;
                    wlvl = 5;
                } else if (GET_SKILL(ch, SKILL_BRAWL) >= 50) {
                    dmg += dmg * 0.2;
                    wlvl = 3;
                }
            }
            if (wtype == 0 && IS_KONATSU(ch)) {
                dmg += dmg * .25;
            }
            if (PLR_FLAGGED(ch, PLR_THANDW)) {
                dmg += dmg * 1.2;
            }
            if (!IS_NPC(ch)) {
                if (PLR_FLAGGED(ch, PLR_THANDW) && gun == false && gun2 == false) {
                    if (GET_SKILL_BASE(ch, SKILL_TWOHAND) >= 100) {
                        dmg += dmg * 0.5;
                    } else if (GET_SKILL_BASE(ch, SKILL_TWOHAND) >= 75) {
                        dmg += dmg * 0.25;
                    } else if (GET_SKILL_BASE(ch, SKILL_TWOHAND) >= 50) {
                        dmg += dmg * 0.1;
                    }
                    if (wtype == 3) {
                        switch (wlvl) {
                            case 1:
                                dmg += dmg * 0.04;
                                break;
                            case 2:
                                dmg += dmg * 0.08;
                                break;
                            case 3:
                                dmg += dmg * 0.12;
                                break;
                            case 4:
                                dmg += dmg * 0.2;
                                break;
                            case 5:
                                dmg += dmg * 0.25;
                                break;
                        }
                    }
                }
            }
            if (wtype == 3) {
                if (skill >= 100)
                    dmg += dmg * 0.04;
                else if (skill >= 50)
                    dmg += dmg * 0.1;
            }
            int hitspot = 1;
            if (gun == true)
                dmg = gun_dam(ch, wlvl);
            hitspot = roll_hitloc(ch, vict, skill);
            int64_t beforepl = GET_HIT(vict);
            switch (hitspot) {
                case 1:
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    break;
                case 2: /* Head */
                    hitspot = 4;
                    break;
                case 3: /* Body */
                    hitspot = 5;
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    break;
                case 4: /* Arm */
                    hitspot = 2;
                    break;
                case 5: /* Leg */
                    hitspot = 3;
                    break;
            }
            if (PLR_FLAGGED(ch, PLR_THANDW) && gun == true) {
                if (hitspot != 4 && boom_headshot(ch)) {
                    hitspot = 4;
                    send_to_char(ch, "@GBoom headshot!@n\r\n");
                }
            }
            switch (wtype) {
                case 0:
                    switch (hitspot) {
                        case 1:
                            act("@WYou slash @C$N@W across the stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou slash @C$N@W across the arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou slash @C$N@W across the leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou slash @C$N@W across the face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            if (!IS_NPC(ch)) {
                                if (PLR_FLAGGED(ch, PLR_THANDW) && gun == false && gun2 == false) {
                                    if (GET_SKILL_BASE(ch, SKILL_TWOHAND) >= 100) {
                                        double mult = calc_critical(ch, 0);
                                        mult += 1.0;
                                        dmg *= mult;
                                    } else {
                                        dmg *= calc_critical(ch, 0);
                                    }
                                } else {
                                    dmg *= calc_critical(ch, 0);
                                }
                            } else {
                                dmg *= calc_critical(ch, 0);
                            }
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou slash @C$N@W across the chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end slash switch*/
                    if (beforepl - GET_HIT(vict) >= (vict->getEffMaxPL()) * 0.025) {
                        cut_limb(ch, vict, wlvl, hitspot);
                    }
                    break;
                case 1:
                    if (!FIGHTING(ch) && backstab(ch, vict, wlvl, dmg)) {
                        if (vict != nullptr && GET_HIT(vict) > 1 && axion_dice(0) < (GET_SKILL(ch, SKILL_DUALWIELD)) &&
                            GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
                            do_attack2(ch, nullptr, 0, 0);
                        }
                        pcost(ch, 0, stcost);
                        return;
                    }
                    dmg += (dmg * 0.01) * (GET_DEX(ch) * 0.5);
                    switch (hitspot) {
                        case 1:
                            act("@WYou pierce @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou pierce @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou pierce @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou pierce @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou pierce @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end pierce switch*/
                    break;
                case 2:
                    switch (hitspot) {
                        case 1:
                            act("@WYou crush @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W chest@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou crush @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou crush @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou crush @C$N@W in the face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes you in the face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N@W in the face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou crush @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W stomach@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end crush switch*/
                    club_stamina(ch, vict, wlvl, dmg);
                    break;
                case 3:
                    switch (hitspot) {
                        case 1:
                            act("@WYou stab @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou stab @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou stab @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou stab @C$N@W in the face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs you in the face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N@W in the face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou stab @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W stomach@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end stab switch*/
                    break;
                case 4:
                    switch (hitspot) {
                        case 1:
                            act("@WYou blast @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou blast @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou blast @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou blast @C$N@W in the face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts you in the face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N@W in the face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou blast @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W stomach@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end blast switch*/
                    break;
                case 5:
                    switch (hitspot) {
                        case 1:
                            act("@WYou whack @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou whack @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou whack @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou whack @C$N@W in the face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks you in the face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N@W in the face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            if (GET_SKILL(ch, SKILL_BRAWL) >= 100) {
                                double mult = calc_critical(ch, 0);
                                mult += 1.0;
                                dmg *= mult;
                            } else {
                                dmg *= calc_critical(ch, 0);
                            }
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou whack @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W stomach@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end brawl switch*/
                    break;
            }/* end switch one*/
        }
        if ((wielded == 2 && gun == false) || (gun2 == false && gun == false)) {
            if (GET_EQ(ch, WEAR_WIELD1)) {
                if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
                    !GET_BONUS(ch, BONUS_FIREPROOF) && !IS_DEMON(ch)) {
                    act("@c$N's@W fireshield burns your weapon!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n's@W weapon is burned by your fireshield!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n's@W weapon is burned by @C$N's@W fireshield!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    int damdam = GET_SKILL(vict, SKILL_FIRESHIELD) / 2;
                    hurt(0, 0, vict, nullptr, GET_EQ(ch, WEAR_WIELD1), damdam, 0);
                } else if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
                           (GET_BONUS(ch, BONUS_FIREPROOF) || IS_DEMON(ch))) {
                    send_to_char(vict, "@RThey appear to be fireproof!@n\r\n");
                }
                pcost(ch, 0, stcost);
            } else {
                if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
                    !GET_BONUS(ch, BONUS_FIREPROOF) && !IS_DEMON(ch)) {
                    act("@c$N's@W fireshield burns your weapon!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n's@W weapon is burned by your fireshield!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n's@W weapon is burned by @C$N's@W fireshield!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    int damdam = GET_SKILL(vict, SKILL_FIRESHIELD) / 2;
                    hurt(0, 0, vict, nullptr, GET_EQ(ch, WEAR_WIELD2), damdam, 0);
                }
                pcost(ch, 0, stcost);
            }
        }
        if (gun == false && gun2 == false) {
            damage_weapon(ch, weap, vict);
        }
        if (!IS_NPC(ch)) {
            if (PLR_FLAGGED(ch, PLR_THANDW)) {
                if (!GET_SKILL(ch, SKILL_TWOHAND) && slot_count(ch) + 1 <= GET_SLOTS(ch)) {
                    int numb = rand_number(10, 15);
                    SET_SKILL(ch, SKILL_TWOHAND, numb);
                    send_to_char(ch, "@GYou learn the very basics of two-handing your weapon!@n\r\n");
                } else {
                    improve_skill(ch, SKILL_TWOHAND, 0);
                }
            }
        }
        if (GET_EQ(ch, WEAR_WIELD2)) {
            if (!GET_SKILL(ch, SKILL_DUALWIELD) && slot_count(ch) + 1 <= GET_SLOTS(ch) &&
                (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD2)) != ITEM_LIGHT)) {
                int numb = rand_number(10, 15);
                SET_SKILL(ch, SKILL_DUALWIELD, numb);
                send_to_char(ch, "@GYou learn the very basics of dual-wielding!@n\r\n");
            } else {
                improve_skill(ch, SKILL_DUALWIELD, 0);
            }
            if (vict != nullptr && GET_HIT(vict) > 1 && axion_dice(0) < (GET_SKILL(ch, SKILL_DUALWIELD)) &&
                GET_EQ(ch, WEAR_WIELD1)) {
                do_attack2(ch, nullptr, 0, 0);
            }
        }
        handle_multihit(ch, vict);
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou attack $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W attacks $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        if ((wielded == 2 && gun == false) || (gun2 == false && gun == false))
            pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_shogekiha) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .125, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SHOGEKIHA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_SHOGEKIHA); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    if (!IS_KABITO(ch)) {
        handle_cooldown(ch, 5);
    } else {
        if (GET_SKILL(ch, SKILL_SHOGEKIHA) < 100) {
            handle_cooldown(ch, 5);
        }
    }
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }

        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_SHOGEKIHA, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        /* Shogekiha Mastery Bonus Area of Funstuff */
        int master_roll = rand_number(1, 100), master_chance = 0, master_pass = false;

        if (skill >= 100)
            master_chance = 20;
        else if (skill >= 75)
            master_chance = 10;
        else if (skill >= 50)
            master_chance = 5;

        if (master_chance >= master_roll)
            master_pass = true;

        /*==========================================*/

        if (!tech_handle_zanzoken(ch, vict, "Shogekiha")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks shogekiha!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W shogekiha!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W shogekiha!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 10, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your shogekiha, letting it slam into the surroundings!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W shogekiha, letting it slam into the surroundings!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W shogekiha, letting it slam into the surroundings!@n", true,
                        ch, nullptr, vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your shogekiha misses, flying through the air harmlessly!@n", true,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W aims $s hand and releases a shogekiha at you, but misses!@n ", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@c$n@W aims $s hand and releases a shogekiha at @C$N@W, but somehow misses!@n ", true, ch,
                        nullptr,
                        vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your shogekiha misses, flying through the air harmlessly!@n", true, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W aims $s hand and releases a shogekiha at you, but misses!@n", true, ch, nullptr, vict,
                    TO_VICT);
                act("@c$n@W aims $s hand and releases a shogekiha at @C$N@W, but somehow misses!@n", true, ch, nullptr,
                    vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 10, skill, attperc);
            if (IS_KABITO(ch)) {
                if (GET_SKILL(ch, SKILL_SHOGEKIHA) >= 100) {
                    ch->incCurKI((ch->getMaxKI() * attperc) * .15);
                } else if (GET_SKILL(ch, SKILL_SHOGEKIHA) >= 60) {
                    ch->incCurKI((ch->getMaxKI() * attperc) * .1);
                } else if (GET_SKILL(ch, SKILL_SHOGEKIHA) >= 40) {
                    ch->incCurKI((ch->getMaxKI() * attperc) * .05);
                }
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou aim your hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly you unleash a large shogekiha that slams into $S chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into your chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into $N@W's chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou aim your hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly you unleash a large shogekiha that slams into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into your face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into $N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou aim your hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly you unleash a large shogekiha that slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into your gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into $N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou aim your hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly you unleash a large shogekiha that slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into $N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou aim your hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly you unleash a large shogekiha that slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and nearby loose objects begin to be pushed out by an invisible force. Suddenly $e unleashes a large shogekiha that slams into $N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (master_pass == true) {
                act("@CYour skillful shogekiha disipated some of @c$N's@C charged ki!@n", true, ch, nullptr, vict,
                    TO_CHAR);
                act("@C$n@C's skillful shogekiha disipated some of YOUR charged ki!@n", true, ch, nullptr, vict,
                    TO_VICT);
                act("@C$n@C's skillful shogekiha disipated some of @c$N's@C charged ki!", true, ch, nullptr, vict,
                    TO_NOTVICT);
                GET_CHARGE(vict) -= GET_CHARGE(vict) * 0.25;
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        if (KICHARGE(obj) > 0 && GET_CHARGE(ch) > KICHARGE(obj)) {
            act("@WYou leap at $p@W with your arms spread out to your sides. As you are about to make contact with $p@W you scream and shatter the attack with your ki!@n",
                true, ch, obj, nullptr, TO_CHAR);
            act("@C$n@W leaps out at $p@W with $s arms spead out to $s sides. As $e is about to make contact with $p@W $e screams and shatters the attack with $s ki!@n",
                true, ch, obj, nullptr, TO_ROOM);
            KICHARGE(obj) -= GET_CHARGE(ch);
            extract_obj(obj);
        } else if (KICHARGE(obj) > 0 && GET_CHARGE(ch) < KICHARGE(obj)) {
            act("@WYou leap at $p@W with your arms spread out to your sides. As you are about to make contact with $p@W you scream and weaken the attack with your ki before taking the rest of the attack in the chest!@n",
                true, ch, obj, nullptr, TO_CHAR);
            act("@C$n@W leaps out at $p@W with $s arms spead out to $s sides. As $e is about to make contact with $p@W $e screams and weakens the attack with $s ki before taking the rest of the attack in the chest!@n",
                true, ch, obj, nullptr, TO_ROOM);
            KICHARGE(obj) -= GET_CHARGE(ch);
            GET_CHARGE(ch) = 0;
            dmg = KICHARGE(obj);
            hurt(0, 0, USER(obj), ch, nullptr, dmg, 0);
            extract_obj(obj);
        } else {
            dmg = damtype(ch, 10, skill, attperc);
            dmg /= 10;
            act("@WYou fire a shogekiha at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
            act("@C$n@W fires a shogekiha at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
            hurt(0, 0, ch, nullptr, obj, dmg, 0);
            pcost(ch, attperc, 0);
        }

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_tsuihidan) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .1, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_TSUIHIDAN)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_TSUIHIDAN); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    /* There is a player/mob targeted */
    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_TSUIHIDAN, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        /* Tsuihidan Mastery */

        int master_roll = rand_number(1, 100), master_chance = 0, master_pass = false;

        if (skill >= 100)
            master_chance = 20;
        else if (skill >= 75)
            master_chance = 10;
        else if (skill >= 50)
            master_chance = 5;

        if (master_chance >= master_roll)
            master_pass = true;

        /*===================*/

        if (!tech_handle_zanzoken(ch, vict, "Tsuihidan")) {
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            dodge_ki(ch, vict, 1, 11, skill, SKILL_TSUIHIDAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            pcost(ch, attperc, 0);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W deflects your tsuihidan, sending it flying away!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou deflect @C$n's@W tsuihidan sending it flying away!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W deflects @c$n's@W tsuihidan sending it flying away!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 200);

                    parry_ki(attperc, ch, vict, "tsuihidan", prob, perc, skill, 11);
                    /*      User/target/skill name/skill/hurt type */
                    pcost(ch, attperc, 0);


                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks tsuihidan!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W tsuihidan!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W tsuihidan!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 11, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your tsuihidan, letting it slam into the surroundings!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W tsuihidan, letting it slam into the surroundings!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W tsuihidan, letting it slam into the surroundings!@n", true,
                        ch, nullptr, vict, TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your tsuihidan misses, flying through the air harmlessly!@n", true,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a tsuihidan at you, but misses!@n ", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a tsuihidan at @C$N@W, but somehow misses!@n ", true, ch, nullptr, vict,
                        TO_NOTVICT);

                    dodge_ki(ch, vict, 1, 11, skill, SKILL_TSUIHIDAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your tsuihidan misses, flying through the air harmlessly!@n", true, ch,
                    nullptr, vict, TO_CHAR);
                act("@C$n@W fires a tsuihidan at you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a tsuihidan at @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                dodge_ki(ch, vict, 1, 11, skill, SKILL_TSUIHIDAN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                hurt(0, 0, ch, vict, nullptr, 0, 1);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 11, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large tsuihidan that slams into $s chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into your chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into $N@W's chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large tsuihidan that slams into $s face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into your face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into $N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large tsuihidan that slams into $s gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into your gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into $N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large tsuihidan that slams into $s arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into $N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 195, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large tsuihidan that slams into $s leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large tsuihidan that slams into $N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 195, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (master_pass == true) {
                vict->decCurST(dmg);
                act("@CYour tsuihidan hits a vital spot and seems to sap some of @c$N's@C stamina!@n", true, ch,
                    nullptr,
                    vict, TO_CHAR);
                act("@C$n's@C tsuihidan hits a vital spot and saps some of your stamina!@n", true, ch, nullptr, vict,
                    TO_VICT);
                act("@C$n's@C tsuihidan hits a vital spot and saps some of @c$N's@C stamina!", true, ch, nullptr, vict,
                    TO_NOTVICT);
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 11, skill, attperc);
        dmg /= 10;
        act("@WYou fire a tsuihidan at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a tsuihidan at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_attack2) {
    int prob, perc, avo, index = 0, pry = 0, dge = 0, blk = 0, skill = 0, wtype = 0, gun2 = false;
    int dualwield = 0;
    int64_t dmg;
    struct char_data *vict = nullptr;
    struct obj_data *obj = nullptr;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!GET_EQ(ch, WEAR_WIELD2)) {
        return;
    }

    int64_t stcost = ((GET_MAX_HIT(ch) / 150) + GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD2)));
    int64_t kicost = ((GET_MAX_HIT(ch) / 150) + GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD2)));

    if (IS_ANDROID(ch)) {
        stcost *= 0.25;
    }

    if (IS_ANDROID(ch) && gun2 == true) {
        kicost *= 0.25;
    }

    if (!can_grav(ch)) {
        return;
    }

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "With what arms!?\r\n");
        return;
    } else if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50 && GET_LIMBCOND(ch, 1) < 0) {
        send_to_char(ch, "Using your broken right arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 0) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 0) < 0) {
            act("@RYour right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's right arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    } else if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50 && GET_LIMBCOND(ch, 0) < 0) {
        send_to_char(ch, "Using your broken left arm has damaged it more!@n\r\n");
        GET_LIMBCOND(ch, 1) -= rand_number(3, 5);
        if (GET_LIMBCOND(ch, 1) < 0) {
            act("@RYour left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@r$n@R's left arm has fallen apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    }

    if (!FIGHTING(ch)) {
        return;
    }

    if (!check_points(ch, 0, stcost)) {
        return;
    }

    if (!IS_NPC(ch) || IS_NPC(ch)) {
        if (vict) {
            if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
                if (!can_kill(ch, vict, nullptr, 1)) {
                    return;
                }
            } else {
                if (!can_kill(ch, vict, nullptr, 0)) {
                    return;
                }
            }
        }

        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_PIERCE - TYPE_HIT) {
            skill = init_skill(ch, SKILL_DAGGER);
            improve_skill(ch, SKILL_DAGGER, 1);
            wtype = 1;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_SLASH - TYPE_HIT) {
            skill = init_skill(ch, SKILL_SWORD);
            improve_skill(ch, SKILL_SWORD, 1);
            wtype = 0;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_CRUSH - TYPE_HIT) {
            skill = init_skill(ch, SKILL_CLUB);
            improve_skill(ch, SKILL_CLUB, 1);
            wtype = 2;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_STAB - TYPE_HIT) {
            skill = init_skill(ch, SKILL_SPEAR);
            improve_skill(ch, SKILL_SPEAR, 1);
            wtype = 3;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
            gun2 = true;
            skill = init_skill(ch, SKILL_GUN);
            improve_skill(ch, SKILL_GUN, 1);
            wtype = 4;
        } else {
            skill = init_skill(ch, SKILL_BRAWL);
            improve_skill(ch, SKILL_BRAWL, 1);
            wtype = 5;
        }
    }

    if (gun2 == false) {
        if (GET_SKILL_BASE(ch, SKILL_DUALWIELD) >= 100) {
            dualwield = 3;
            stcost -= stcost * 0.30;
        } else if (GET_SKILL_BASE(ch, SKILL_DUALWIELD) >= 75) {
            dualwield = 2;
            stcost -= stcost * 0.25;
        } else if (GET_SKILL_BASE(ch, SKILL_DUALWIELD) >= 50) {
            dualwield = 1;
            stcost -= stcost * 0.25;
        }
    }

    if (IS_NPC(ch) && GET_LEVEL(ch) <= 10) {
        skill = rand_number(30, 50);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 20) {
        skill = rand_number(30, 60);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 30) {
        skill = rand_number(30, 70);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 50) {
        skill = rand_number(40, 80);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 70) {
        skill = rand_number(50, 90);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 80) {
        skill = rand_number(60, 100);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 90) {
        skill = rand_number(70, 100);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) <= 100) {
        skill = rand_number(80, 100);
    } else if (IS_NPC(ch) && GET_LEVEL(ch) > 100) {
        skill = rand_number(95, 100);
    }
    if (FIGHTING(ch) && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
        vict = FIGHTING(ch);
    }

    if (gun2 == true) {
        if (GET_GOLD(ch) < 1) {
            send_to_char(ch, "You do not have enough zenni. You need 1 zenni per shot.\r\n");
            return;
        } else {
            ch->mod(CharMoney::Carried, -1);
        }
    }

    if (GET_PREFERENCE(ch) != PREFERENCE_H2H) {
        handle_cooldown(ch, 4);
    } else {
        handle_cooldown(ch, 8);
    }

    if (vict) {
        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
            if (!can_kill(ch, vict, nullptr, 1)) {
                return;
            }
        } else {
            if (!can_kill(ch, vict, nullptr, 0)) {
                return;
            }
        }
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;
        handle_defense(vict, &pry, &blk, &dge);
        if (dualwield == 3) {
            pry -= pry * 0.1;
            blk -= pry * 0.1;
            dge -= pry * 0.1; /* pry, This is intentional */
        }

        if (gun2 == true) {
            if (dualwield >= 1) {
                prob += prob * 0.1;
            }
        }

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "attack")) {
            if (gun2 == false)
                pcost(ch, 0, stcost / 3);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W intercepts and parries your attack with $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou intercept and parry @C$n's@W attack with one of your own!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W intercepts and parries @c$n's@W attack with one of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (wtype != 4)
                        handle_disarm(ch, vict);
                    improve_skill(vict, SKILL_PARRY, 0);
                    if (gun2 == false)
                        pcost(ch, 0, stcost);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    if (gun2 == false)
                        pcost(ch, 0, stcost);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, -1, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    if (gun2 == false)
                        pcost(ch, 0, stcost);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it but your attack misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves to attack you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to attack @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    if (gun2 == false)
                        pcost(ch, 0, stcost / 3);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it but your attack misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W moves to attack you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to attack @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                if (gun2 == false)
                    pcost(ch, 0, stcost / 3);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, -1, skill, attperc);
            int wlvl = 0;
            struct obj_data *weap = GET_EQ(ch, WEAR_WIELD2);
            if (OBJ_FLAGGED(weap, ITEM_WEAPLVL1)) {
                dmg += dmg * .05;
                wlvl = 1;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL2)) {
                dmg += dmg * .1;
                wlvl = 2;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL3)) {
                dmg += dmg * .2;
                wlvl = 3;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL4)) {
                dmg += dmg * .3;
                wlvl = 4;
            } else if (OBJ_FLAGGED(weap, ITEM_WEAPLVL5)) {
                dmg += dmg * .5;
                wlvl = 5;
            }
            if (wtype == 5) {
                if (GET_SKILL(ch, SKILL_BRAWL) >= 100) {
                    dmg += dmg * 0.5;
                    wlvl = 5;
                } else if (GET_SKILL(ch, SKILL_BRAWL) >= 50) {
                    dmg += dmg * 0.2;
                    wlvl = 3;
                }
            }
            if (wtype == 0 && IS_KONATSU(ch)) {
                dmg += dmg * .25;
            }
            int hitspot = 1;
            if (gun2 == true)
                dmg = gun_dam(ch, wlvl);
            hitspot = roll_hitloc(ch, vict, skill);
            int64_t beforepl = GET_HIT(vict);
            if (wtype == 3) {
                if (skill >= 100)
                    dmg += dmg * 0.04;
                else if (skill >= 50)
                    dmg += dmg * 0.1;
            }
            if (gun2 == true) {
                if (dualwield == 3 && rand_number(1, 3) == 3) {
                    send_to_char(ch, "@GYour masterful aim scores a critical!@n\r\n");
                    hitspot = 2;
                }
            }
            switch (hitspot) {
                case 1:
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    break;
                case 2: /* Head */
                    hitspot = 4;
                    break;
                case 3: /* Body */
                    hitspot = 5;
                    break;
                case 4: /* Arm */
                    hitspot = 5;
                    break;
                case 5: /* Leg */
                    hitspot = 5;
                    break;
            }
            switch (wtype) {
                case 0:
                    switch (hitspot) {
                        case 1:
                            act("@WYou slash @C$N@W across the stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou slash @C$N@W across the arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou slash @C$N@W across the leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou slash @C$N@W across the face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou slash @C$N@W across the chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W slashes you across the chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W slashes @C$N@W across the chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end slash switch*/
                    if (beforepl - GET_HIT(vict) >= (vict->getEffMaxPL()) * 0.025) {
                        cut_limb(ch, vict, wlvl, hitspot);
                    }
                    break;
                case 1:
                    dmg += (dmg * 0.01) * (GET_DEX(ch) * 0.5);
                    switch (hitspot) {
                        case 1:
                            act("@WYou pierce @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou pierce @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N'@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou pierce @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou pierce @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou pierce @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W pierces your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W pierces @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end pierce switch*/
                    break;
                case 2:
                    switch (hitspot) {
                        case 1:
                            act("@WYou crush @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou crush @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou crush @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou crush @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N'@W face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou crush @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W crushes your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W crushes @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end crush switch*/
                    club_stamina(ch, vict, wlvl, dmg);
                    break;
                case 3:
                    switch (hitspot) {
                        case 1:
                            act("@WYou stab @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou stab @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou stab @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou stab @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N'@W face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou stab @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W stabs your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W stabs @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end stab switch*/
                    break;
                case 4:
                    switch (hitspot) {
                        case 1:
                            act("@WYou blast @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou blast @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou blast @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou blast @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N'@W face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 0);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou blast @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W blasts your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W blasts @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end blast switch*/
                    break;
                case 5:
                    switch (hitspot) {
                        case 1:
                            act("@WYou whack @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your stomach!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W stomach!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 2:
                            act("@WYou whack @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your arm!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 1);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 3:
                            act("@WYou whack @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your leg!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            dmg *= calc_critical(ch, 1);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 2);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 4:
                            act("@WYou whack @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your face!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N'@W face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            if (GET_SKILL(ch, SKILL_BRAWL) >= 100) {
                                double mult = calc_critical(ch, 0);
                                mult += 1.0;
                                dmg *= mult;
                            } else {
                                dmg *= calc_critical(ch, 0);
                            }
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 3);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                        case 5:
                            act("@WYou whack @C$N's@W chest!@n", true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n@W whacks your chest!@n", true, ch, nullptr, vict, TO_VICT);
                            act("@c$n@W whacks @C$N's@W chest!@n", true, ch, nullptr, vict, TO_NOTVICT);
                            hurt(0, 0, ch, vict, nullptr, dmg, 0);
                            dam_eq_loc(vict, 4);
                            /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                            break;
                    }/* end whacks switch*/
                    break;
            }/* end switch one*/
        }
        if (gun2 == false) {
            if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
                !GET_BONUS(ch, BONUS_FIREPROOF) && !IS_DEMON(ch)) {
                act("@c$N's@W fireshield burns your weapon!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n's@W weapon is burned by your fireshield!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n's@W weapon is burned by @C$N's@W fireshield!@n", true, ch, nullptr, vict, TO_NOTVICT);
                int damdam = GET_SKILL(vict, SKILL_FIRESHIELD) / 2;
                hurt(0, 0, vict, nullptr, GET_EQ(ch, WEAR_WIELD2), damdam, 0);
            } else if (GET_HIT(vict) > 0 && !AFF_FLAGGED(vict, AFF_SPIRIT) && AFF_FLAGGED(vict, AFF_FIRESHIELD) &&
                       (GET_BONUS(ch, BONUS_FIREPROOF) || IS_DEMON(ch))) {
                send_to_char(vict, "@RThey appear to be fireproof!@n\r\n");
            }
            pcost(ch, 0, stcost);
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou attack $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W attacks $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        if (gun2 == false)
            pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_bite) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = 20 + (GET_MAX_HIT(ch) / 500);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);


    if (!IS_NPC(ch) && (!IS_MUTANT(ch) || (GET_GENOME(ch, 0) != 7 && GET_GENOME(ch, 1) != 7))) {
        send_to_char(ch, "You don't want to put that in your mouth, you don't know where it has been!\r\n");
        return;
    }
    if (!can_grav(ch)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 200)) {
        return;
    }

    skill = init_skill(ch, SKILL_PUNCH);

    if (skill <= 0) {
        skill = 60;
    }

    vict = nullptr;
    obj = nullptr;
    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;
    handle_cooldown(ch, 4);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "bite")) {
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your bite with a punch of their own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W bite with a punch of your own!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W parries @c$n's@W bite with a punch of $S own!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -1, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your bite!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W bite!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W bite!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 6, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your bite!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W bite!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W bite!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou move to bite @C$N@W, but miss!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves to bite you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to bite @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@C$n@W moves to bite you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to bite @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 8, skill, attperc);
            if (!IS_NPC(ch)) {
                dmg = damtype(ch, 0, skill, attperc);
            }
            dmg += dmg * 0.25;
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou bite @C$N's@W face!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bites your face!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W bites into $N's face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou bite @C$N@W!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bites you!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bites @c$N@W!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou bite @C$N's@W body!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bites you on the body, sending blood flying!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bites @c$N@W on the body, sending blood flying!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou bite @C$N's@W arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bites you on the arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bites @c$N@W on the arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou bite @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bites into your leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W bites into @c$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (!IS_NPC(ch)) {
                if (axion_dice(0) > GET_CON(vict) && rand_number(1, 5) == 5) {
                    act("@R$N@r was poisoned by your bite!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou were poisoned by the bite!@n", true, ch, nullptr, vict, TO_VICT);
                    vict->poisonby = ch;
                    ch->poisoned.insert(vict);
                    int duration = (GET_INT(ch) / 50) + 1;
                    assign_affect(vict, AFF_POISON, SKILL_POISON, duration, 0, 0, 0, 0, 0, 0);
                }
            }

            pcost(ch, 0, stcost);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@C$n@W bites $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_kiball) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0, frompool = false;
    int64_t dmg;
    double attperc = .05, minimum = .01;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KIBALL)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
        send_to_char(ch, "Your hands are full!\r\n");
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }
    /*Rillao: Trying to make conc/focus no-charge for t1 ki */
    /* if ((GET_SKILL (ch, SKILL_CONCENTRATION) == 100) && (GET_SKILL (ch, SKILL_FOCUS) == 100)) {
     minimum -=0.01;
     if (GET_CHARGE(ch) <= 100) {
      GET_CHARGE(ch) = GET_MAX_MANA(ch) * 0.01;
      attperc = 0.01;
      if (frompool == TRUE) {
       GET_MANA(ch) -= GET_MAX_MANA(ch) * attperc;
       } else {
       pcost(ch, 0, GET_MAX_MANA(ch) * 0.01);
       }
     } */
    // log("Log 1 - attperc: %f, minimum: %f, charge: %" I64T "", attperc, minimum, GET_CHARGE(ch));
    //}

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (!frompool && GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    // log("Log 2 - attperc: %f, minimum: %f, charge: %" I64T "", attperc, minimum, GET_CHARGE(ch));
    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }
    // log("Log 3 - attperc: %f, minimum: %f, charge: %" I64T "", attperc, minimum, GET_CHARGE(ch));
    skill = init_skill(ch, SKILL_KIBALL);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_KIBALL, 0);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        /* Kiballs skill roll for multi-shot */
        int mult_roll = rand_number(1, 100), mult_count = 1, mult_chance = 0;

        if (skill >= 100) {
            mult_chance = 30;
        } else if (skill >= 75) {
            mult_chance = 15;
        } else if (skill >= 50) {
            mult_chance = 10;
        }

        if (mult_roll <= mult_chance)
            mult_count = rand_number(2, 3);

        /*-----------------------------------*/

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "kiball")) {
            if (frompool == true) {
                ch->decCurKI(ch->getMaxKI() * attperc);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }


        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W deflects your kiball, sending it flying away!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou deflect @C$n's@W kiball sending it flying away!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W deflects @c$n's@W kiball sending it flying away!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 200);

                    parry_ki(attperc, ch, vict, "kiball", prob, perc, skill, 7);
                    /*      User/target/skill name/skill/hurt type */
                    pcost(ch, attperc, 0);

                    ch->getRoom()->modDamage(2);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks kiball!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W kiball!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W kiball!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 7, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your kiball, letting it slam into the surroundings!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W kiball, letting it slam into the surroundings!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W kiball, letting it slam into the surroundings!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);

                    dodge_ki(ch, vict, 0, 7, skill, SKILL_KIBALL); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(2);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your kiball misses, flying through the air harmlessly!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a bright yellow kiball at you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a bright yellow kiball at @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your kiball misses, flying through the air harmlessly!@n", true, ch,
                    nullptr,
                    vict, TO_CHAR);
                act("@C$n@W fires a bright yellow kiball at you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a bright yellow kiball at @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            if (mult_count > 1) {
                act("@CYour expertise has allowed you to fire multiple shots in a row!@n", true, ch, nullptr, vict,
                    TO_CHAR);
                act("@C$n's@C expertise has allowed $m to fire multiple shots in a row!@n", true, ch, nullptr, vict,
                    TO_ROOM);
            }
            while (mult_count > 0) {
                mult_count -= 1;
                dmg = damtype(ch, 7, skill, attperc);
                int hitspot = 1;
                hitspot = roll_hitloc(ch, vict, skill);
                switch (hitspot) {
                    case 1:
                        act("@WYou hold out your hand towards @C$N@W, and fire a bright yellow kiball! The kiball slams into $M quickly and explodes with roaring light!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@c$n@W holds out $s hand towards you, and fires a bright yellow kiball! The kiball slams into you quickly and explodes with roaring light!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@c$n@W holds out $s hand towards @C$N@W, and fires a bright yellow kiball! The kiball slams into $M quickly and explodes with roaring light!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        if (GET_BONUS(ch, BONUS_SOFT)) {
                            dmg *= calc_critical(ch, 2);
                        }
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 4);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 2: /* Critical */
                        act("@WYou hold out your hand towards @C$N@W, and fire a bright yellow kiball! The kiball slams into $S face and explodes, shrouding $S head with smoke!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@c$n@W holds out $s hand towards you, and fires a bright yellow kiball! The kiball slams into your face and explodes, leaving you choking on smoke!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@c$n@W holds out $s hand towards @C$N@W, and fires a bright yellow kiball! The kiball slams into $S face and explodes, shrouding $S head with smoke!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        dmg *= calc_critical(ch, 0);
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 3);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 3:
                        act("@WYou hold out your hand towards @C$N@W, and fire a bright yellow kiball! The kiball slams into $S body and explodes with a loud roar!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@c$n@W holds out $s hand towards you, and fires a bright yellow kiball! The kiball slams into your body and explodes with a loud roar!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@c$n@W holds out $s hand towards @C$N@W, and fires a bright yellow kiball! The kiball slams into $S body and explodes with a loud roar!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        if (GET_BONUS(ch, BONUS_SOFT)) {
                            dmg *= calc_critical(ch, 2);
                        }
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 4);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 4: /* Weak */
                        act("@WYou hold out your hand towards @C$N@W, and fire a bright yellow kiball! The kiball grazes $S arm and explodes shortly after!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@c$n@W holds out $s hand towards you, and fires a bright yellow kiball! The kiball grazes your arm and explodes shortly after!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@c$n@W holds out $s hand towards @C$N@W, and fires a bright yellow kiball! The kiball grazes $S arm and explodes shortly after!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        dmg *= calc_critical(ch, 1);
                        hurt(0, 195, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 1);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                    case 5: /* Weak 2 */
                        act("@WYou hold out your hand towards @C$N@W, and fire a bright yellow kiball! The kiball grazes $S leg and explodes shortly after!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@c$n@W holds out $s hand towards you, and fires a bright yellow kiball! The kiball grazes your leg and explodes shortly after!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@c$n@W holds out $s hand towards @C$N@W, and fires a bright yellow kiball! The kiball grazes $S leg and explodes shortly after!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        dmg *= calc_critical(ch, 1);
                        hurt(1, 195, ch, vict, nullptr, dmg, 1);
                        dam_eq_loc(vict, 2);
                        /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                        break;
                }
                if (GET_HIT(vict) <= 0) {
                    mult_count = 0;
                }
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 7, skill, attperc);
        dmg /= 10;
        act("@WYou fire a kiball at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a kiball at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_beam) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg;
    double attperc = .1, minimum = .01;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BEAM)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
        send_to_char(ch, "Your hands are full!\r\n");
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    skill = init_skill(ch, SKILL_BEAM);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 5);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_BEAM, 0);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "beam")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W deflects your beam, sending it flying away!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou deflect @C$n's@W beam sending it flying away!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W deflects @c$n's@W beam sending it flying away!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 200);

                    parry_ki(attperc, ch, vict, "beam", prob, perc, skill, 10);
                    /*      User/target/skill name/skill/hurt type */
                    pcost(ch, attperc, 0);

                    ch->getRoom()->modDamage(5);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks beam!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W beam!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W beam!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 10, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your beam, letting it slam into the surroundings!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W beam, letting it slam into the surroundings!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W beam, letting it slam into the surroundings!@n", true, ch,
                        nullptr,
                        vict, TO_NOTVICT);

                    dodge_ki(ch, vict, 0, 10, skill, SKILL_BEAM); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your beam misses, flying through the air harmlessly!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a bright yellow beam at you, but misses!@n ", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a bright yellow beam at @C$N@W, but somehow misses!@n ", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your beam misses, flying through the air harmlessly!@n", true, ch,
                    nullptr,
                    vict, TO_CHAR);
                act("@C$n@W fires a bright yellow beam at you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a bright yellow beam at @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 10, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large beam that slams into $S chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into your chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into $N@W's chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large beam that slams into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into your face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into $N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large beam that slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into your gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into $N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large beam that slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into $N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 195, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large beam that slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large beam that slams into $N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 195, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, attperc, 0);

            int master_roll = rand_number(1, 100), master_chance = 0, master_pass = false;

            if (skill >= 100)
                master_chance = 20;
            else if (skill >= 75)
                master_chance = 10;
            else if (skill >= 50)
                master_chance = 5;

            if (master_chance >= master_roll)
                master_pass = true;

            if (GET_HIT(vict) > 0 && dmg > GET_MAX_HIT(vict) / 4 && master_pass == true) {
                int attempt = rand_number(0, NUM_OF_DIRS);  /* Select a random direction */
                int count = 0;
                while (count < 12) {
                    attempt = count;
                    if (CAN_GO(vict, attempt)) {
                        count = 12;
                    } else {
                        count++;
                    }
                }
                if (CAN_GO(vict, attempt)) {
                    act("$N@W is pushed away by the blast!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou are pushed away by the blast!@n", true, ch, nullptr, vict, TO_VICT);
                    act("$N@W is pushed away by the blast!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    do_simple_move(vict, attempt, true);
                } else {
                    act("$N@W is pushed away by the blast, but is slammed into an obstruction!@n", true, ch, nullptr,
                        vict,
                        TO_CHAR);
                    act("@WYou are pushed away by the blast, but are slammed into an obstruction!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("$N@W is pushed away by the blast, but is slammed into an obstruction!@n", true, ch, nullptr,
                        vict,
                        TO_NOTVICT);
                    dmg *= 2;
                    hurt(1, 195, ch, vict, nullptr, dmg, 1);
                }
            }
            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 10, skill, attperc);
        dmg /= 10;
        act("@WYou fire a beam at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a beam at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_kiblast) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg;
    double attperc = .075, minimum = .01;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KIBLAST)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
        send_to_char(ch, "Your hands are full!\r\n");
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!tech_handle_charge(ch, arg2, minimum, &attperc)) return;

    if (GET_MAX_MANA(ch) * attperc > GET_CHARGE(ch)) {
        attperc = (long double) (GET_CHARGE(ch)) / (long double) (GET_MAX_MANA(ch));
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * minimum, 0)) {
        return;
    }

    skill = init_skill(ch, SKILL_KIBLAST);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (!IS_ANDROID(ch) || GET_SKILL(ch, SKILL_KIBLAST) < 100) {
        handle_cooldown(ch, 5);
    }
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_KIBLAST, 0);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        /* Kiblast mastery Area */

        int mastery = rand_number(1, 100), master_pass = false, chance = 0;

        if (skill >= 100)
            chance = 30;
        else if (skill >= 75)
            chance = 20;
        else if (skill >= 50)
            chance = 15;

        if (mastery <= chance)
            master_pass = true;

        /*======================*/

        if (!tech_handle_zanzoken(ch, vict, "kiblast")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }


        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (tech_handle_android_absorb(ch, vict)) {
                    pcost(ch, 1, 0);
                    return;
                } else if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W deflects your kiblast, sending it flying away!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou deflect @C$n's@W kiblast sending it flying away!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W deflects @c$n's@W kiblast sending it flying away!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 200);

                    parry_ki(attperc, ch, vict, "kiblast", prob, perc, skill, 9);
                    /*      User/target/skill name/skill/hurt type */
                    pcost(ch, attperc, 0);

                    ch->getRoom()->modDamage(5);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks kiblast!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W kiblast!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W kiblast!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 9, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your kiblast, letting it slam into the surroundings!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W kiblast, letting it slam into the surroundings!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W kiblast, letting it slam into the surroundings!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);

                    dodge_ki(ch, vict, 0, 9, skill, SKILL_KIBLAST); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your kiblast misses, flying through the air harmlessly!@n", true,
                        ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a bright yellow kiblast at you, but misses!@n ", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@c$n@W fires a bright yellow kiblast at @C$N@W, but somehow misses!@n ", true, ch, nullptr,
                        vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your kiblast misses, flying through the air harmlessly!@n", true, ch,
                    nullptr,
                    vict, TO_CHAR);
                act("@C$n@W fires a bright yellow kiblast at you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a bright yellow kiblast at @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 9, skill, attperc);
            if (IS_ANDROID(ch)) {
                if (GET_SKILL(ch, SKILL_KIBLAST) >= 100) {
                    dmg += dmg * 0.15;
                } else if (GET_SKILL(ch, SKILL_KIBLAST) >= 60) {
                    dmg += dmg * 0.10;
                } else if (GET_SKILL(ch, SKILL_KIBLAST) >= 40) {
                    dmg += dmg * 0.05;
                }
            }
            int64_t record = GET_HIT(vict);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large kiblast that slams into $S chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into your chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into $N@W's chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large kiblast that slams into $S face!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into your face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into $N@W's face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large kiblast that slams into $S gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into your gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into $N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large kiblast that slams into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into $N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 195, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou aim your hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly you unleash a large kiblast that slams into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@W$n@W aims $s hand at you, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@W$n@W aims $s hand at $N@W, and bright @Yyellow@W energy begins to pool there. Suddenly $e unleashes a large kiblast that slams into $N@W's leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 195, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            if (master_pass == true && record > GET_HIT(vict) &&
                (record - GET_HIT(vict) > (vict->getEffMaxPL()) * 0.025)) {
                if (!AFF_FLAGGED(vict, AFF_KNOCKED) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
                    act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou are knocked out!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    vict->setStatusKnockedOut();
                }
            }
            pcost(ch, attperc, 0);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 1)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = damtype(ch, 10, skill, attperc);
        dmg /= 10;
        act("@WYou fire a kiblast at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a kiblast at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_slam) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = physical_cost(ch, SKILL_SLAM);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SLAM)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
        send_to_char(ch, "Your hands are full!\r\n");
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }
    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 100)) {
        return;
    }

    skill = init_skill(ch, SKILL_SLAM);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (IS_BARDOCK(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            handle_cooldown(ch, 7);
        else
            handle_cooldown(ch, 9);
    } else {
        handle_cooldown(ch, 9);
    }

    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_SLAM, 1);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        if (IS_KABITO(ch) && !IS_NPC(ch)) {
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                perc -= perc * 0.2;
        }

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);
        if (!tech_handle_zanzoken(ch, vict, "slam")) {
            COMBO(ch) = -1;
            COMBHITS(ch) = 0;
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your slam with a punch of $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W slam with a punch of your own!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W parries @c$n's@W slam with a punch of $S own!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your slam!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W slam!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W slam!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 6, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your slam!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W slam!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W slam!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it but your slam misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W moves to slam you with both $s fists, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to slam @C$N@W with both $s fists, but somehow misses!@n", true, ch, nullptr,
                        vict,
                        TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it but your slam misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W moves to slam you with both $s fists, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to slam @C$N@W with both $s fists, but somehow misses!@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 6, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou disappear, appearing above @C$N@W and slam a double fisted blow into $M!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@C$n@W disappears, only to appear above you, slamming a double fisted blow into you!@n", true,
                        ch, nullptr, vict, TO_VICT);
                    act("@c$n@W disappears, only to appear above @C$N@W, slamming a double fisted blow into $M!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou disappear, reappearing in front of @C$N@W, you grab $M! Spinning you send $M flying into the ground!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W disappears, reappearing in front of you, and $e grabs you! Spinning quickly $e sends you flying into the ground!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W disappears, reappearing in front of @C$N@W, and grabs $M! Spinning quickly $e sends $M flying into the ground!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    if (!AFF_FLAGGED(vict, AFF_KNOCKED) &&
                        (rand_number(1, 4) >= 3 && (GET_HIT(vict) > GET_HIT(ch) / 5) &&
                         !AFF_FLAGGED(vict, AFF_SANCTUARY))) {
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@WYou are knocked out!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        vict->setStatusKnockedOut();
                    } else if ((GET_POS(vict) == POS_STANDING || GET_POS(vict) == POS_FIGHTING) &&
                               !AFF_FLAGGED(vict, AFF_KNOCKED)) {
                        GET_POS(vict) = POS_SITTING;
                    }
                    if (ROOM_DAMAGE(IN_ROOM(vict)) <= 95 && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE)) {
                        act("@W$N@W slams into the ground forming a large crater with $S body!@n", true, ch, nullptr,
                            vict,
                            TO_CHAR);
                        act("@WYou slam into the ground forming a large crater with your body!@n", true, ch, nullptr,
                            vict,
                            TO_VICT);
                        act("@W$N@W slams into the ground forming a large crater with $S body!@n", true, ch, nullptr,
                            vict,
                            TO_NOTVICT);
                        if (SECT(IN_ROOM(vict)) != SECT_INSIDE && SECT(IN_ROOM(vict)) != SECT_UNDERWATER &&
                            SECT(IN_ROOM(vict)) != SECT_WATER_SWIM && SECT(IN_ROOM(vict)) != SECT_WATER_NOSWIM) {
                            impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
                            switch (rand_number(1, 8)) {
                                case 1:
                                    act("Debris is thrown into the air and showers down thunderously!", true, ch,
                                        nullptr,
                                        vict, TO_CHAR);
                                    act("Debris is thrown into the air and showers down thunderously!", true, ch,
                                        nullptr,
                                        vict, TO_ROOM);
                                    break;
                                case 2:
                                    if (rand_number(1, 4) == 4 && ROOM_EFFECT(IN_ROOM(vict)) == 0) {
                                        ROOM_EFFECT(IN_ROOM(vict)) = 1;
                                        act("Lava leaks up through cracks in the crater!", true, ch, nullptr, vict,
                                            TO_CHAR);
                                        act("Lava leaks up through cracks in the crater!", true, ch, nullptr, vict,
                                            TO_ROOM);
                                    }
                                    break;
                                case 3:
                                    act("A cloud of dust envelopes the entire area!", true, ch, nullptr, vict, TO_CHAR);
                                    act("A cloud of dust envelopes the entire area!", true, ch, nullptr, vict, TO_ROOM);
                                    break;
                                case 4:
                                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr,
                                        vict,
                                        TO_CHAR);
                                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr,
                                        vict,
                                        TO_ROOM);
                                    break;
                                case 5:
                                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr,
                                        vict,
                                        TO_CHAR);
                                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr,
                                        vict,
                                        TO_ROOM);
                                    break;
                                case 6:
                                    /* One less message */
                                    break;
                                default:
                                    /* we want no message for the default */
                                    break;
                            }
                        }
                        if (SECT(IN_ROOM(vict)) == SECT_UNDERWATER) {
                            switch (rand_number(1, 3)) {
                                case 1:
                                    act("The water churns violently!", true, ch, nullptr, vict, TO_CHAR);
                                    act("The water churns violently!", true, ch, nullptr, vict, TO_ROOM);
                                    break;
                                case 2:
                                    act("Large bubbles rise from the movement!", true, ch, nullptr, vict, TO_CHAR);
                                    act("Large bubbles rise from the movement!", true, ch, nullptr, vict, TO_ROOM);
                                    break;
                                case 3:
                                    act("The water collapses in on the hole created!", true, ch, nullptr, vict,
                                        TO_CHAR);
                                    act("The water collapses in on the hole create!", true, ch, nullptr, vict, TO_ROOM);
                                    break;
                            }
                        }
                        if (SECT(IN_ROOM(vict)) == SECT_WATER_SWIM || SECT(IN_ROOM(vict)) == SECT_WATER_NOSWIM) {
                            switch (rand_number(1, 3)) {
                                case 1:
                                    act("A huge column of water erupts from the impact!", true, ch, nullptr, vict,
                                        TO_CHAR);
                                    act("A huge column of water erupts from the impact!", true, ch, nullptr, vict,
                                        TO_ROOM);
                                    break;
                                case 2:
                                    act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr,
                                        vict,
                                        TO_CHAR);
                                    act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr,
                                        vict,
                                        TO_ROOM);
                                    break;
                                case 3:
                                    act("A huge depression forms in the water and erupts into a wave from the impact!",
                                        true, ch, nullptr, vict, TO_CHAR);
                                    act("A huge depression forms in the water and erupts into a wave from the impact!",
                                        true, ch, nullptr, vict, TO_ROOM);
                                    break;
                            }
                        }
                        if (SECT(IN_ROOM(vict)) == SECT_INSIDE) {
                            impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
                            switch (rand_number(1, 8)) {
                                case 1:
                                    act("Debris is thrown into the air and showers down thunderously!", true, ch,
                                        nullptr,
                                        vict, TO_CHAR);
                                    act("Debris is thrown into the air and showers down thunderously!", true, ch,
                                        nullptr,
                                        vict, TO_ROOM);
                                    break;
                                case 2:
                                    act("The structure of the surrounding room cracks and quakes from the impact!",
                                        true, ch, nullptr, vict, TO_CHAR);
                                    act("The structure of the surrounding room cracks and quakes from the impact!",
                                        true, ch, nullptr, vict, TO_ROOM);
                                    break;
                                case 3:
                                    act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr,
                                        vict,
                                        TO_CHAR);
                                    act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr,
                                        vict,
                                        TO_ROOM);
                                    break;
                                case 4:
                                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr,
                                        vict,
                                        TO_CHAR);
                                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr,
                                        vict,
                                        TO_ROOM);
                                    break;
                                case 5:
                                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr,
                                        vict,
                                        TO_CHAR);
                                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr,
                                        vict,
                                        TO_ROOM);
                                    break;
                                case 6:
                                    act("The walls of the surrounding room crack in the same instant!", true, ch,
                                        nullptr,
                                        vict, TO_CHAR);
                                    act("The walls of the surrounding room crack in the same instant!", true, ch,
                                        nullptr,
                                        vict, TO_ROOM);
                                    break;
                                default:
                                    /* we want no message for the default */
                                    break;
                            }
                        }
                        vict->getRoom()->modDamage(5);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou fly at @C$N@W, slamming both your fists into $S gut as you fly!@n", true, ch, nullptr,
                        vict,
                        TO_CHAR);
                    act("@C$n@W flies at you, slamming both $s fists into your gut as $e flies!@n", true, ch, nullptr,
                        vict,
                        TO_VICT);
                    act("@c$n@W flies at @C$N@W, slamming both $s fists into $S gut as $e flies!@n", true, ch, nullptr,
                        vict,
                        TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou slam both your fists into @C$N@W, hitting $M in the arm!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W slams both $s fists into you, hitting you in the arm!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@c$n@W slams both $s fists into @C$N@W, hitting $M in the arm!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 195, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou slam both your fists into @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W slams both $s fists into your leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W slams both $s fists into @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 195, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "hands");
            pcost(ch, 0, stcost);
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou slam $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W slams $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_uppercut) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = physical_cost(ch, SKILL_UPPERCUT);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_UPPERCUT)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
        send_to_char(ch, "Your hands are full!\r\n");
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 200)) {
        return;
    }

    skill = init_skill(ch, SKILL_UPPERCUT);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (IS_FRIEZA(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            handle_cooldown(ch, 5);
        else
            handle_cooldown(ch, 7);
    } else {
        handle_cooldown(ch, 7);
    }
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_UPPERCUT, 1);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        if (IS_KABITO(ch) && !IS_NPC(ch)) {
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                perc -= perc * 0.2;
        }

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "uppercut")) {
            COMBO(ch) = -1;
            COMBHITS(ch) = 0;
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your uppercut with a punch of $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W uppercut with a punch of your own!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W parries @c$n's@W uppercut with a punch of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your uppercut!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W uppercut!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W uppercut!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 5, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your uppercut!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W uppercut!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W uppercut!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it but your uppercut misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W throws an uppercut at you but somehow misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W throws an uppercut at @C$N@W but somehow misses!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it but your uppercut misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W throws an uppercut at you but somehow misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W throws an uppercut at @C$N@W but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 5, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou leap up and launch an uppercut into @C$N's@W body!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W leaps up and launches an uppercut into your body!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W leaps up and launches an uppercut into @C$N's@W body!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou smash an uppercut into @C$N's@W chin!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W smashes an uppercut into your chin!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W smashes an uppercut into @C$N's@W chin!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    if (!AFF_FLAGGED(vict, AFF_KNOCKED) &&
                        (rand_number(1, 8) >= 7 && (GET_HIT(vict) > GET_HIT(ch) / 5) &&
                         !AFF_FLAGGED(vict, AFF_SANCTUARY))) {
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@WYou are knocked out!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        vict->setStatusKnockedOut();
                    }
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou uppercut @C$N@W, hitting $M directly in chest!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W uppercuts you, hitting you directly in the chest!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W uppercuts @C$N@W, hitting $M directly in the chest!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYour poorly aimed uppercut hits @C$N@W in the arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W poorly aims an uppercut and hits you in the arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W poorly aims an uppercut and hits @C$N@W in the arm!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 195, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou slam an uppercut into @C$N's@W leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W slams an uppercut into your leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W slams an uppercut into @C$N's@W leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 195, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "hand");
            pcost(ch, 0, stcost);
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou uppercut $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W uppercuts $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_tailwhip) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = physical_cost(ch, SKILL_TAILWHIP);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_TAILWHIP)) {
        return;
    }
    if (!PLR_FLAGGED(ch, PLR_TAIL) && !IS_NPC(ch)) {
        send_to_char(ch, "You have no tail!\r\n");
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 120)) {
        return;
    }

    skill = init_skill(ch, SKILL_TAILWHIP);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 7);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_TAILWHIP, 1);
        index = check_def(vict);
        prob = roll_accuracy(ch, skill, false);
        perc = chance_to_hit(ch);

        if (IS_KABITO(ch) && !IS_NPC(ch)) {
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                perc -= perc * 0.2;
        }

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "tailwhip")) {
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your tailwhip with a punch of $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W tailwhip with a punch of your own!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W parries @c$n's@W tailwhip with a punch of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@C$N@W moves quickly and blocks your tailwhip!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W tailwhip!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W tailwhip!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 3, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@C$N@W manages to dodge your tailwhip!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W tailwhip!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W tailwhip!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it but your tailwhip misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W throws a tailwhip at you but somehow misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W throws a tailwhip at @C$N@W but somehow misses!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it but your tailwhip misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W throws a tailwhip at you but somehow misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W throws a tailwhip at @C$N@W but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 56, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou spin to swing your tail and slam it into @c$N@W's body!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W spins to swing $s tail and slams it into YOUR body!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$n@W spins to swing $s tail and slams it into @c$N@W's body!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING && rand_number(1, 8) >= 7) {
                        handle_knockdown(vict);
                    }
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou flip forward and slam your tail into the top of @c$N@W's head brutally!@n", true, ch,
                        nullptr,
                        vict, TO_CHAR);
                    act("@C$n@W flips forward and slams $s tail into the top of YOUR head brutally!@n", true, ch,
                        nullptr,
                        vict, TO_VICT);
                    act("@C$n@W flips forward and slams $s tail into the top of @c$N@W's head brutally!@n", true, ch,
                        nullptr,
                        vict, TO_NOTVICT);
                    if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING && rand_number(1, 8) >= 6) {
                        handle_knockdown(vict);
                    }
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou swing your tail and manage to slam it into @c$N@W's gut!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W swings $s tail and manages to slam it into YOUR gut!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$n@W swings $s tail and manages to slam it into @c$N@W's gut!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING && rand_number(1, 8) >= 7) {
                        handle_knockdown(vict);
                    }
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou swing your tail and manage to slam it into @c$N@W's arm!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W swings $s tail and manages to slam it into YOUR arm!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$n@W swings $s tail and manages to slam it into @c$N@W's arm!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 195, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@WYou swing your tail and manage to slam it into @c$N@W's leg!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W swings $s tail and manages to slam it into YOUR leg!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$n@W swings $s tail and manages to slam it into @c$N@W's leg!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 195, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "tail");
            pcost(ch, 0, stcost);
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is broken already!\r\n");
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou tailwhip $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W tailwhips $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_roundhouse) {
    atk::Roundhouse a(ch, argument);
    a.execute();
}

ACMD(do_elbow) {
    atk::Elbow a(ch, argument);
    a.execute();
}

ACMD(do_kick) {
    atk::Kick a(ch, argument);
    a.execute();
}

ACMD(do_knee) {
    atk::Knee a(ch, argument);
    a.execute();
}

ACMD(do_punch) {
    atk::Punch a(ch, argument);
    a.execute();
}

/* This handles charging for energy attacks */
ACMD(do_charge) {

    char arg[MAX_INPUT_LENGTH];
    int amt;

    one_argument(argument, arg);

    if (PLR_FLAGGED(ch, PLR_AURALIGHT)) {
        send_to_char(ch, "@WYou are concentrating too much on your aura to be able to charge.");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }
    if (AFF_FLAGGED(ch, AFF_MBREAK)) {
        send_to_char(ch, "Your mind is still strained from psychic attacks...\r\n");
        return;
    }
    if (AFF_FLAGGED(ch, AFF_POISON)) {
        send_to_char(ch, "You feel too sick from the poison to concentrate.\r\n");
        return;
    }
    if (!*arg) {
        send_to_char(ch, "Charge, yes. How much percent though?\r\n");
        send_to_char(ch, "[ 1 - 100 | cancel | release]\r\n");
        return;
    } else if (!strcasecmp("release", arg) && (PLR_FLAGGED(ch, PLR_CHARGE) && GET_CHARGE(ch) <= 0)) {
        send_to_char(ch, "Try cancel instead, you have nothing charged up yet!\r\n");
        return;
    } else if (!strcasecmp("release", arg) && (PLR_FLAGGED(ch, PLR_CHARGE) && GET_CHARGE(ch) > 0)) {
        send_to_char(ch, "You stop charging and release your pent up energy.\r\n");
        switch (rand_number(1, 3)) {
            case 1:
                act("$n@w's aura disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            case 2:
                act("$n@w's aura fades.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            case 3:
                act("$n@w's aura flickers brightly before disappearing.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            default:
                act("$n@w's aura disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
        }
        ch->incCurKI(GET_CHARGE(ch));
        GET_CHARGE(ch) = 0;
        GET_CHARGETO(ch) = 0;
        ch->playerFlags.reset(PLR_CHARGE);
        return;
    } else if (!strcasecmp("release", arg) && GET_CHARGE(ch) > 0) {
        send_to_char(ch, "You release your pent up energy.\r\n");
        switch (rand_number(1, 3)) {
            case 1:
                act("$n@w's aura disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            case 2:
                act("$n@w's aura fades.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            case 3:
                act("$n@w's aura flickers brightly before disappearing.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            default:
                act("$n@w's aura disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
        }
        ch->incCurKI(GET_CHARGE(ch));
        GET_CHARGE(ch) = 0;
        GET_CHARGETO(ch) = 0;
        return;
    } else if (!strcasecmp("cancel", arg) && PLR_FLAGGED(ch, PLR_CHARGE)) {
        send_to_char(ch, "You stop charging.\r\n");
        switch (rand_number(1, 3)) {
            case 1:
                act("$n@w's aura disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            case 2:
                act("$n@w's aura fades.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            case 3:
                act("$n@w's aura flickers brightly before disappearing.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
            default:
                act("$n@w's aura disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
                break;
        }
        ch->playerFlags.reset(PLR_CHARGE);
        GET_CHARGETO(ch) = 0;
        return;
    } else if (!strcasecmp("cancel", arg) && !PLR_FLAGGED(ch, PLR_CHARGE)) {
        send_to_char(ch, "You are not even charging!\r\n");
        return;
    } else if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 100) {
        send_to_char(ch, "You don't even have enough ki!\r\n");
        return;
    } else if ((amt = atoi(arg)) > 0) {
        if (amt >= 101) {
            send_to_char(ch, "You have set it too high!\r\n");
            return;
        } else if (AFF_FLAGGED(ch, AFF_SPIRITCONTROL)) {
            int64_t diff = 0;
            if ((ch->getCurKI()) < ((GET_MAX_MANA(ch) * 0.01) * amt) + 1) {
                diff = (((GET_MAX_MANA(ch) * 0.01) * amt) + 1) - (ch->getCurKI());
            }
            int chance = 15;
            chance -= GET_SKILL(ch, SKILL_SPIRITCONTROL) / 10;
            if (chance < 10)
                chance = 10;
            else if (chance > 15)
                chance = 15;

            if (chance > rand_number(1, 100)) {
                send_to_char(ch,
                             "The rush of ki that you try to pool temporarily overwhelms you and you lose control!\r\n");
                null_affect(ch, AFF_SPIRITCONTROL);
                return;
            } else {
                int64_t spiritcost = GET_MAX_MANA(ch) * 0.05;
                if (GET_SKILL(ch, SKILL_SPIRITCONTROL) >= 100) {
                    spiritcost = GET_MAX_MANA(ch) * 0.01;
                }
                reveal_hiding(ch, 0);
                send_to_char(ch,
                             "Your %s colored aura flashes up around you as you instantly take control of the ki you needed!\r\n",
                             aura_types[GET_AURA(ch)]);
                send_to_char(ch, "@D[@RCost@D: @r%s@D]@n\r\n", add_commas(spiritcost));
                char bloom[MAX_INPUT_LENGTH];
                sprintf(bloom, "@wA %s aura flashes up brightly around $n@w!@n", aura_types[GET_AURA(ch)]);
                act(bloom, true, ch, nullptr, nullptr, TO_ROOM);
                GET_CHARGE(ch) = (((GET_MAX_MANA(ch) * 0.01) * amt) + 1) - diff;
                ch->decCurKI((((GET_MAX_MANA(ch) * 0.01) * amt) + 1) - diff + spiritcost);
            }
        } else {
            reveal_hiding(ch, 0);
            send_to_char(ch, "You begin to charge some energy, as a %s aura begins to burn around you!\r\n",
                         aura_types[GET_AURA(ch)]);
            char bloom[MAX_INPUT_LENGTH];
            sprintf(bloom, "@wA %s aura flashes up brightly around $n@w!@n", aura_types[GET_AURA(ch)]);
            act(bloom, true, ch, nullptr, nullptr, TO_ROOM);
            GET_CHARGETO(ch) = (((GET_MAX_MANA(ch) * 0.01) * amt) + 1);
            GET_CHARGE(ch) += 1;
            ch->playerFlags.set(PLR_CHARGE);
        }
    } else if (amt < 1 && GET_ROOM_VNUM(IN_ROOM(ch)) != 1562) {
        send_to_char(ch, "You have set it too low!\r\n");
        return;
    } else {
        send_to_char(ch, "That is an invalid argument.\r\n");
    }
}

ACMD(do_powerup) {
    if (IS_NPC(ch)) {
        ch->mobFlags.set(MOB_POWERUP);
        if (GET_MAX_HIT(ch) < 50000) {
            act("@RYou begin to powerup, and air billows outward around you!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@R$n begins to powerup, and air billows outward around $m!@n", true, ch, nullptr, nullptr, TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 500000) {
            act("@RYou begin to powerup, and loose objects are lifted into the air!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and loose objects are lifted into the air!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 5000000) {
            act("@RYou begin to powerup, and torrents of energy crackle around you!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and torrents of energy crackle around $m!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 50000000) {
            act("@RYou begin to powerup, and the entire area begins to shudder!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and the entire area begins to shudder!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 100000000) {
            act("@RYou begin to powerup, and massive cracks begin to form beneath you!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and massive cracks begin to form beneath $m!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 300000000) {
            act("@RYou begin to powerup, and everything around you shudders from the power!@n", true, ch, nullptr,
                nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and everything around $m shudders from the power!@n", true, ch, nullptr,
                nullptr, TO_ROOM);
        } else {
            act("@RYou begin to powerup, and the very air around you begins to burn!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and the very air around $m begins to burn!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        }
        return;
    }
    if (PLR_FLAGGED(ch, PLR_AURALIGHT)) {
        send_to_char(ch, "@WYou are concentrating too much on your aura to be able to power up.");
        return;
    }
    if (IS_ANDROID(ch)) {
        send_to_char(ch, "@WYou are an android, you do not powerup.@n");
        return;
    }
    if (GET_SUPPRESS(ch) > 0) {
        send_to_char(ch, "@WYou currently have your powerlevel suppressed to %" I64T " percent.@n", GET_SUPPRESS(ch));
        return;
    }
    if (PLR_FLAGGED(ch, PLR_POWERUP)) {
        send_to_char(ch, "@WYou stop powering up.@n");
        ch->playerFlags.reset(PLR_POWERUP);
        return;
    }
    if (GET_HIT(ch) >= GET_MAX_HIT(ch)) {
        send_to_char(ch, "@WYou are already at max!@n");
        return;
    }
    if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 20) {
        send_to_char(ch, "@WYou do not have enough ki to powerup!@n");
        return;
    } else {
        reveal_hiding(ch, 0);
        if (GET_MAX_HIT(ch) < 50000) {
            act("@RYou begin to powerup, and air billows outward around you!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@R$n begins to powerup, and air billows outward around $m!@n", true, ch, nullptr, nullptr, TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 500000) {
            act("@RYou begin to powerup, and loose objects are lifted into the air!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and loose objects are lifted into the air!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 5000000) {
            act("@RYou begin to powerup, and torrents of energy crackle around you!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and torrents of energy crackle around $m!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 50000000) {
            act("@RYou begin to powerup, and the entire area begins to shudder!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and the entire area begins to shudder!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 100000000) {
            act("@RYou begin to powerup, and massive cracks begin to form beneath you!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and massive cracks begin to form beneath $m!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        } else if (GET_MAX_HIT(ch) < 300000000) {
            act("@RYou begin to powerup, and everything around you shudders from the power!@n", true, ch, nullptr,
                nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and everything around $m shudders from the power!@n", true, ch, nullptr,
                nullptr, TO_ROOM);
        } else {
            act("@RYou begin to powerup, and the very air around you begins to burn!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@R$n begins to powerup, and the very air around $m begins to burn!@n", true, ch, nullptr, nullptr,
                TO_ROOM);
        }
        ch->playerFlags.set(PLR_POWERUP);
        return;
    }
}

ACMD(do_rescue) {

    char arg[100];
    struct char_data *helpee, *opponent;

    one_argument(argument, arg);

    if (!*arg)
        send_to_char(ch, "Whom do you wish to rescue?\r\n");
    else if (!(helpee = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM)))
        send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (helpee == ch)
        send_to_char(ch, "You can't help yourself any more than this!\r\n");
    else if (!FIGHTING(helpee))
        send_to_char(ch, "They are not fighting anyone!\r\n");
    else if (FIGHTING(ch) && !IS_NPC(ch))
        send_to_char(ch, "You are a little too busy fighting for yourself!\r\n");
    else {
        opponent = FIGHTING(helpee);
        int mobbonus = 0;

        if (IS_NPC(ch)) {
            mobbonus = GET_SPEEDI(ch) * 0.2;
        }
        if (GET_SPEEDI(ch) + mobbonus < GET_SPEEDI(opponent) && rand_number(1, 3) != 3) {
            act("@GYou leap towards @g$N@G and try to rescue $M but are too slow!@n", true, ch, nullptr, helpee,
                TO_CHAR);
            act("@g$n@G leaps towards you! $n is too slow and fails to rescue you!@n", true, ch, nullptr, helpee,
                TO_VICT);
            act("@g$n@G leaps towards @g$N@G and tries to rescue $M but is too slow!@n", true, ch, nullptr, helpee,
                TO_NOTVICT);
            return;
        }
        act("@GYou leap in front of @g$N@G and rescue $M!@n", true, ch, nullptr, helpee, TO_CHAR);
        act("@g$n@G leaps in front of you! You are rescued!@n", true, ch, nullptr, helpee, TO_VICT);
        act("@g$n@G leaps in front of @g$N@G and rescues $M!@n", true, ch, nullptr, helpee, TO_NOTVICT);
        stop_fighting(opponent);
        hurt(0, 0, ch, opponent, nullptr, rand_number(1, GET_LEVEL(ch)), 1);
        hurt(0, 0, opponent, ch, nullptr, rand_number(1, GET_LEVEL(ch)), 1);
        return;
    }
}

ACMD(do_assist) {
    char arg[MAX_INPUT_LENGTH];
    struct char_data *helpee, *opponent;

    if (FIGHTING(ch)) {
        send_to_char(ch, "You're already fighting!  How can you assist someone else?\r\n");
        return;
    }
    one_argument(argument, arg);

    if (!*arg)
        send_to_char(ch, "Whom do you wish to assist?\r\n");
    else if (!(helpee = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM)))
        send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (helpee == ch)
        send_to_char(ch, "You can't help yourself any more than this!\r\n");
    else {
        /*
     * Hit the same enemy the person you're helping is.
     */
        if (FIGHTING(helpee))
            opponent = FIGHTING(helpee);
        else
            for (opponent = ch->getRoom()->people;
                 opponent && (FIGHTING(opponent) != helpee);
                 opponent = opponent->next_in_room);

        if (!opponent)
            act("But nobody is fighting $M!", true, ch, nullptr, helpee, TO_CHAR);
        else if (!CAN_SEE(ch, opponent))
            act("You can't see who is fighting $M!", true, ch, nullptr, helpee, TO_CHAR);
            /* prevent accidental pkill */
        else {
            reveal_hiding(ch, 0);
            send_to_char(ch, "You join the fight!\r\n");
            act("$N assists you!", 0, helpee, nullptr, ch, TO_CHAR);
            act("$n assists $N.", true, ch, nullptr, helpee, TO_NOTVICT);
            if (!FIGHTING(ch)) {
                set_fighting(ch, opponent);
            }
            if (!FIGHTING(opponent)) {
                set_fighting(opponent, ch);
            }
        }
    }
}

ACMD(do_kill) {
    char arg[MAX_INPUT_LENGTH];
    struct char_data *vict;

    if (IS_NPC(ch) || !ADM_FLAGGED(ch, ADM_INSTANTKILL)) {
        return;
    }
    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Kill who?\r\n");
    } else {
        if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM)))
            send_to_char(ch, "They aren't here.\r\n");
        else if (ch == vict)
            send_to_char(ch, "Your mother would be so sad.. :(\r\n");
        else {
            act("You chop $M to pieces!  Ah!  The blood!", true, ch, nullptr, vict, TO_CHAR);
            act("$N chops you to pieces!", true, vict, nullptr, ch, TO_CHAR);
            act("$n brutally slays $N!", true, ch, nullptr, vict, TO_NOTVICT);
            raw_kill(vict, ch);
        }
    }
}

ACMD(do_flee) {
    int i, attempt;
    struct char_data *was_fighting;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (GET_POS(ch) < POS_RESTING) {
        send_to_char(ch, "You are in pretty bad shape, unable to flee!\r\n");
        return;
    }

    if (GRAPPLING(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return;
    }

    if (GRAPPLED(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return;
    }

    if (ABSORBING(ch)) {
        send_to_char(ch, "You are absorbing from someone!\r\n");
        return;
    }

    if (ABSORBBY(ch)) {
        send_to_char(ch, "You are being absorbed from by someone!\r\n");
        return;
    }

    if (!IS_NPC(ch)) {
        int fail = false;
        auto isMyAttack = [&](const auto&o) {return KICHARGE(o) > 0 && USER(o) == ch;};
        if (auto obj = ch->getRoom()->findObject(isMyAttack); obj) {
            send_to_char(ch, "You are too busy controlling your attack!\r\n");
            return;
        }
    }

    for (i = 0; i < 12; i++) {
        if (*arg) {
            if ((attempt = search_block(arg, dirs, false) > -1)) {
                attempt = search_block(arg, dirs, false);
            } else if ((attempt = search_block(arg, abbr_dirs, false) > -1)) {
                attempt = search_block(arg, abbr_dirs, false);
            } else {
                attempt = rand_number(0, NUM_OF_DIRS - 1);  /* Select a random direction */
            }
        }
        if (!*arg) {
            attempt = rand_number(0, NUM_OF_DIRS - 1);    /* Select a random direction */
        }
        if (CAN_GO(ch, attempt) &&
            !ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH)) {
            act("$n panics, and attempts to flee!", true, ch, nullptr, nullptr, TO_ROOM);
            if (IS_NPC(ch) && ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_NOMOB)) {
                return;
            }
            was_fighting = FIGHTING(ch);

            auto isWall = [&](const auto&o) {return o->vn == 79 && GET_OBJ_COST(o) == attempt;};
            if(auto wall = ch->getRoom()->findObject(isWall); wall) {
                send_to_char(ch, "That direction has a glacial wall blocking it.\r\n");
                return;
            }

            if (!block_calc(ch)) {
                return;
            }

            if (ABSORBING(ch)) {
                send_to_char(ch, "You are busy absorbing from %s!\r\n", GET_NAME(ABSORBING(ch)));
                return;
            }
            if (ABSORBBY(ch)) {
                if (axion_dice(0) < GET_SKILL(ABSORBING(ch), SKILL_ABSORB)) {
                    send_to_char(ch, "You are being held by %s, they are absorbing you!\r\n", GET_NAME(ABSORBBY(ch)));
                    send_to_char(ABSORBBY(ch), "%s struggles in your grasp!\r\n", GET_NAME(ch));
                    WAIT_STATE(ch, PULSE_2SEC);
                    return;
                } else {
                    act("@c$N@W manages to break loose of @C$n's@W hold!@n", true, ABSORBBY(ch), nullptr, ch,
                        TO_NOTVICT);
                    act("@WYou manage to break loose of @C$n's@W hold!@n", true, ABSORBBY(ch), nullptr, ch, TO_VICT);
                    act("@c$N@W manages to break loose of your hold!@n", true, ABSORBBY(ch), nullptr, ch, TO_CHAR);
                    ABSORBING(ABSORBBY(ch)) = nullptr;
                    ABSORBBY(ch) = nullptr;
                }
            }
            if (do_simple_move(ch, attempt, true)) {
                send_to_char(ch, "You flee head over heels.\r\n");
                WAIT_STATE(ch, PULSE_2SEC);
            } else {
                act("$n tries to flee, but can't!", true, ch, nullptr, nullptr, TO_ROOM);
                WAIT_STATE(ch, PULSE_2SEC);
            }
            return;
        }
    }
    send_to_char(ch, "PANIC!  You couldn't escape!\r\n");

}
