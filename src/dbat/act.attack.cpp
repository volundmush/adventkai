/* ************************************************************************
*   File: act.attack.c                                  Part of DBAT      *
*  Usage: player-level commands of an offensive nature                    *
*         created because the size act.offensive.c was getting too large. *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  -While this is an original file and is credited to me it is basically  *
*   just act.offensive.c part 2. So all original credit is due to the     *
*   credits found in act.offensive.c except for the commands added in-    *
*                                                   ~~Iovan               *
************************************************************************ */
#include "dbat/act.attack.h"
#include "dbat/fight.h"
#include "dbat/dg_comm.h"
#include "dbat/act.item.h"
#include "dbat/interpreter.h"
#include "dbat/utils.h"
#include "dbat/handler.h"
#include "dbat/comm.h"
#include "dbat/constants.h"
#include "dbat/combat.h"
#include "dbat/class.h"
#include "dbat/techniques.h"

ACMD(do_lightgrenade) {
    int perc, dge = 2, count = 0, skill;
    int64_t dmg;
    double attperc = .35, minimum = .15;
    struct char_data *vict = nullptr, *targ = nullptr, *next_v = nullptr;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_LIGHTGRENADE)) {
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

    targ = nullptr;
    if (!*arg || !(targ = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        if (FIGHTING(ch) && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
            targ = FIGHTING(ch);
        } else {
            send_to_char(ch, "Nobody around here by that name.\r\n");
            return;
        }
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_LIGHTGRENADE); /* Set skill value */

    perc = chance_to_hit(ch);

    if (skill < perc) {
        act("@WYou quickly bring your hands in front of your body and cup them a short distance from each other. A flash of @Ggreen @Ylight@W can be seen as your ki is condensed between your hands before a @Yg@yo@Yl@yd@Ye@yn@W orb of ki replaces the green light. Your concentration waivers however and the energy slips away from your control harmlessly!@n",
            true, ch, nullptr, targ, TO_CHAR);
        act("@C$n@W quickly brings $s hands in front of $s body and cups them a short distance from each other. A flash of @Ggreen @Ylight@W can be seen as ki is condensed between $s hands before a @Yg@yo@Yl@yd@Ye@yn@W orb of ki replaces the green light. Suddenly $s concentration seems to waiver and the energy slips away from $s control harmlessly!@n",
            true, ch, nullptr, targ, TO_VICT);
        act("@C$n@W quickly brings $s hands in front of $s body and cups them a short distance from each other. A flash of @Ggreen @Ylight@W can be seen as ki is condensed between $s hands before a @Yg@yo@Yl@yd@Ye@yn@W orb of ki replaces the green light. Suddenly $s concentration seems to waiver and the energy slips away from $s control harmlessly!@n",
            true, ch, nullptr, targ, TO_NOTVICT);
        pcost(ch, attperc, 0);

        improve_skill(ch, SKILL_LIGHTGRENADE, 0);
        return;
    }

    act("@WYou quickly bring your hands in front of your body and cup them a short distance from each other. A flash of @Ggreen @Ylight@W can be seen as your ki is condensed between your hands before a @Yg@yo@Yl@yd@Ye@yn@W orb of ki replaces the green light. You shout @r'@YLIGHT GRENADE@r'@W as the orb launches from your hands at @C$N@W!@n",
        true, ch, nullptr, targ, TO_CHAR);
    act("@C$n@W quickly brings $s hands in front of $s body and cups them a short distance from each other. A flash of @Ggreen @Ylight@W can be seen as ki is condensed between $s hands before a @Yg@yo@Yl@yd@Ye@yn@W orb of ki replaces the green light. @C$n shouts @r'@YLIGHT GRENADE@r'@W as the orb launches from $s hands at YOU!@n",
        true, ch, nullptr, targ, TO_VICT);
    act("@C$n@W quickly brings $s hands in front of $s body and cups them a short distance from each other. A flash of @Ggreen @Ylight@W can be seen as ki is condensed between $s hands before a @Yg@yo@Yl@yd@Ye@yn@W orb of ki replaces the green light. @C$n shouts @r'@YLIGHT GRENADE@r'@W as the orb launches from $s hands at @c$N@W!@n",
        true, ch, nullptr, targ, TO_NOTVICT);

    dmg = damtype(ch, 57, skill, attperc);

    for (vict = ch->getRoom()->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (vict == ch) {
            continue;
        }
        if (MOB_FLAGGED(vict, MOB_NOKILL)) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_GROUP)) {
            if (vict->master == ch || ch->master == vict || ch->master == vict->master) {
                if (vict == targ) {
                    send_to_char(ch, "Leave the group if you want to murder them.\r\n");
                }
                continue;
            }
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
            act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr, vict,
                TO_CHAR);
            act("@cYou disappear, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr, vict,
                TO_VICT);
            act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr, vict,
                TO_NOTVICT);
            ch->affected_by.reset(AFF_ZANZOKEN);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            if (vict == targ) {
                pcost(ch, attperc, 0);
                handle_cooldown(ch, 9);
                return;
            } else {
                continue;
            }
        } else if (dge > axion_dice(skill * 0.5) && vict == targ) {
            send_to_char(ch, "DGE: %d\n", dge);
            act("@c$N@W manages to dodge the light grenade!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@WYou manages to dodge the light grenade!@n", true, ch, nullptr, vict, TO_VICT);
            act("@c$N@W manages to dodge the light grenade!@n", true, ch, nullptr, vict, TO_NOTVICT);
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            improve_skill(vict, SKILL_DODGE, 0);
            pcost(ch, attperc, 0);
            handle_cooldown(ch, 9);
            return;
        } else if (dge > axion_dice(skill * 0.5) && vict != targ) {
            act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@WYou manage to escape the explosion!@n", true, ch, nullptr, vict, TO_VICT);
            act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            improve_skill(vict, SKILL_DODGE, 0);
            continue;
        } else if (vict == targ) {
            act("@R$N@r is hit by the light grenade which explodes all around $m!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@RYou are hit by the light grenade which explodes all around you!@n", true, ch, nullptr, vict,
                TO_VICT);
            act("@R$N@r is hit by the light grenade which explodes all around $m!@n", true, ch, nullptr, vict,
                TO_NOTVICT);
            if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING && rand_number(1, 4) == 4) {
                handle_knockdown(vict);
            }
            hurt(0, 0, ch, vict, nullptr, dmg, 1);
            continue;
        } else {
            act("@R$N@r is caught by the light grenade's explosion!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@RYou are caught by the light grenade's explosion!@n", true, ch, nullptr, vict, TO_VICT);
            act("@R$N@r is caught by the light grenade's explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
            if (!AFF_FLAGGED(vict, AFF_FLYING) && GET_POS(vict) == POS_STANDING && rand_number(1, 4) == 4) {
                handle_knockdown(vict);
            }
            hurt(0, 0, ch, vict, nullptr, dmg * 0.5, 1);
            continue;
        }
    } /* Hitting targets! */
    pcost(ch, attperc, 0);

    improve_skill(ch, SKILL_LIGHTGRENADE, 0);
    handle_cooldown(ch, 9);
}

ACMD(do_energize) {

    if (IS_NPC(ch))
        return;

    if (GET_PREFERENCE(ch) != PREFERENCE_THROWING) {
        send_to_char(ch, "You aren't dedicated to throwing!\r\n");
        return;
    }

    if (!GET_SKILL(ch, SKILL_ENERGIZE)) {
        if (GET_SKILL(ch, SKILL_FOCUS) >= 30) {
            int result = rand_number(10, 14);
            SET_SKILL(ch, SKILL_ENERGIZE, result);
            send_to_char(ch,
                         "You learn the basics for energizing thrown weapons! Now use the energize command again.\r\n");
            return;
        } else {
            send_to_char(ch, "You need a Focus skill level of 30 to figure out the basics of this technique.\r\n");
            return;
        }
    } else {
        ch->pref.flip(PRF_ENERGIZE);
        if (ch->pref.test(PRF_ENERGIZE)) {
            send_to_char(ch, "You start focusing your latent ki into your fingertips.\r\n");
        } else {
            send_to_char(ch, "You stop focusing ki into your fingertips.\r\n");
        }
    }
}

ACMD(do_breath) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = GET_MAX_HIT(ch) / 5000;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!IS_NPC(ch)) {
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

    skill = init_skill(ch, SKILL_KNEE);

    vict = nullptr;
    obj = nullptr;
    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;
    handle_cooldown(ch, 10);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
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
        prob += 15;

        if (!tech_handle_zanzoken(ch, vict, "breath")) {
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@WYou move quickly and block @C$n's@W fiery breath!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W fiery breath!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 6, skill, attperc);
                    dmg = dmg * 0.8;
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
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@WYou dodge the fiery jets of flames coming from @C$n's@W mouth!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge the fiery jets of flames coming from @c$n's@W mouth!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@C$n@W moves to breath flames on you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to breath flames on @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@C$n@W moves to breath flames on you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to breath flames on @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 8, skill, attperc);
            dmg += dmg * 2;
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@C$n@W aims $s mouth at you and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s mouth at @c$N@W and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto @c$N's@W body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@C$n@W aims $s mouth at you and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto YOUR face!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s mouth at @c$N@W and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto @c$N's@W face!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@C$n@W aims $s mouth at you and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s mouth at @c$N@W and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto @c$N's@W body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@C$n@W aims $s mouth at you and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s mouth at @c$N@W and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto @c$N's@W arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@C$n@W aims $s mouth at you and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W aims $s mouth at @c$N@W and opens it wide slowly. A high pitched sound can be heard as the mouth opens, and as the throat is exposed a bright white flame can be seen burning there. Suddenly @C$n@W breathes a jet of @rf@Ri@Ye@Rr@ry@W flames onto @c$N's@W leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, 0, stcost);
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
        act("@C$n@W breathes flames on $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_ram) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = GET_MAX_HIT(ch) / 200;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!IS_NPC(ch)) {
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

    skill = init_skill(ch, SKILL_KNEE);

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

        prob -= 5;

        if (!tech_handle_zanzoken(ch, vict, "ram")) {
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);

            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > axion_dice(10)) {
                    act("@WYou move quickly and block @C$n's@W body as $e tries to ram YOU!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W body as $e tries to ram $M!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 6, skill, attperc);
                    dmg -= dmg * 0.2;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@WYou dodge @C$n's@W attempted ram!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W attempted ram!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@C$n@W moves to ram you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to ram @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@C$n@W moves to ram you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to ram @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 8, skill, attperc);
            dmg += dmg * 1.1;
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@C$n@W aims $s body at YOU and rams into YOUR body!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W aims $s body at @C$N@W and rams into $S body!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@C$n@W aims $s body at YOU and rams into YOUR face!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W aims $s body at @C$N@W and rams into $S face!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@C$n@W aims $s body at YOU and rams into YOUR body!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W aims $s body at @C$N@W and rams into $S body!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@C$n@W aims $s body at YOU and rams into YOUR arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W aims $s body at @C$N@W and rams into $S arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@C$n@W aims $s body at YOU and rams into YOUR leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W aims $s body at @C$N@W and rams into $S leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 190, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, 0, stcost);
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
        act("@C$n@W rams $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_strike) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = GET_MAX_HIT(ch) / 400;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!IS_NPC(ch)) {
        return;
    }
    if (!can_grav(ch)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 400)) {
        return;
    }

    skill = init_skill(ch, SKILL_KNEE);

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

        prob += 5;

        if (!tech_handle_zanzoken(ch, vict, "fang strike")) {
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@WYou parry @C$n's@W fang strike with a punch of your own!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W parries @c$n's@W fang strike with a punch of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > axion_dice(10)) {
                    act("@WYou move quickly and block @C$n's@W fang strike!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W fang strike!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 6, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > axion_dice(10)) {
                    act("@WYou dodge @C$n's@W fang strike!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W fang strike!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@C$n@W moves to fang strike you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W moves to fang strike @C$N@W, but somehow misses!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@C$n@W moves to fang strike you, but misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W moves to fang strike @C$N@W, but somehow misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                pcost(ch, 0, stcost / 2);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 0);
            return;
        } else {
            dmg = damtype(ch, 8, skill, attperc);
            dmg += dmg * 0.5;
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@C$n@W launches $s body at YOU and sinks $s fang strike into YOUR body!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@c$n@W launches $s body at @C$N@W and sinks $s fang strike into $S body!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@C$n@W launches $s body at YOU and sinks $s fang strike into YOUR face!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@c$n@W launches $s body at @C$N@W and sinks $s fang strike into $S face!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@C$n@W launches $s body at YOU and sinks $s fang strike into YOUR body!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@c$n@W launches $s body at @C$N@W and sinks $s fang strike into $S body!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@C$n@W launches $s body at YOU and sinks $s fang strike into YOUR arm!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@c$n@W launches $s body at @C$N@W and sinks $s fang strike into $S arm!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak 2 */
                    act("@C$n@W launches $s body at YOU and sinks $s fang strike into YOUR leg!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@c$n@W launches $s body at @C$N@W and sinks $s fang strike into $S leg!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            pcost(ch, 0, stcost);
            vict->decCurST(dmg * .25);
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
        act("@C$n@W fang strikes $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

/* End NPC skills */

ACMD(do_combine) {

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct follow_type *f;
    int fire = false, temp = -1, temp2 = -1;

    two_arguments(argument, arg, arg2);

    if (!has_group(ch)) {
        send_to_char(ch, "You need to be in a group!\r\n");
        return;
    } else {
        if (!*arg || (!ch->master && !*arg2)) {
            send_to_char(ch, "Follower Syntax: combine (attack)\r\n");
            send_to_char(ch, "Leader Syntax: combine (attack) (target)\r\n");
            send_to_char(ch, "Cancel Syntax: combine stop\r\n");
        } else {
            if (!strcasecmp(arg, "stop") && ch->master) {
                if (GET_COMBINE(ch) == -1) {
                    send_to_char(ch, "You are not trying to combine any attacks...\r\n");
                    return;
                } else {
                    send_to_char(ch, "You stop your preparations to combine your attack with a group attack.\r\n");
                    send_to_char(ch->master, "@Y%s@C is no longer prepared to combine an attack with the group!@n\r\n",
                                 get_i_name(ch->master, ch));
                    for (f = ch->master->followers; f; f = f->next) {
                        if (ch != f->follower)
                            send_to_char(f->follower,
                                         "@Y%s@C is no longer prepared to combine an attack with the group!@n\r\n",
                                         get_i_name(f->follower, ch));
                    }
                    GET_COMBINE(ch) = -1;
                    return;
                }
            } else if (!strcasecmp(arg, "stop") && !ch->master) {
                send_to_char(ch, "You do not need to stop as you haven't prepared anything.\r\n");
                return;
            }
            int i = 0;
            for (i = 0; i < 14; i++) {
                if (strstr(arg, attack_names[i])) {
                    if (i == 5) {
                        if (!GET_EQ(ch, WEAR_WIELD1)) {
                            send_to_char(ch, "You need to wield a sword to use this technique.\r\n");
                            return;
                        }
                        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) != TYPE_SLASH - TYPE_HIT) {
                            send_to_char(ch, "You are not wielding a sword, you need one to use this technique.\r\n");
                            return;
                        } else {
                            temp = i;
                            i = 15;
                        }
                    }
                    temp = i;
                    i = 15;
                }
            }
            if (temp == -1) {
                send_to_char(ch, "Follower Syntax: combine (attack)\r\n");
                send_to_char(ch, "Leader Syntax: combine (attack) (target)\r\n");
                send_to_char(ch, "Follower Cancel Syntax: combine stop\r\n");
                return;
            } else if (!GET_SKILL(ch, attack_skills[temp])) {
                send_to_char(ch, "You do not know that skill.\r\n");
                return;
            } else if (attack_skills[temp] == 440 && !IS_NAIL(ch)) {
                send_to_char(ch, "Only students of Nail know how to combine that attack effectively.\r\n");
                return;
            } else if (GET_CHARGE(ch) < GET_MAX_MANA(ch) * 0.05) {
                send_to_char(ch, "You need to have the minimum of 5%s ki charged to combine.\r\n", "%");
            }
            if (!ch->master) {
                if (!(vict = get_char_vis(ch, arg2, nullptr, FIND_CHAR_ROOM))) {
                    send_to_char(ch, "Who will your combined attack be targeting?\r\n");
                    return;
                } else if (vict == ch) {
                    send_to_char(ch, "No targeting yourself...\r\n");
                    return;
                }
                GET_COMBINE(ch) = temp;
                for (f = ch->followers; f; f = f->next) {
                    if (!AFF_FLAGGED(f->follower, AFF_GROUP)) {
                        continue;
                    } else if (GET_COMBINE(f->follower) != -1 &&
                               GET_CHARGE(f->follower) >= GET_MAX_MANA(f->follower) * 0.05) {
                        fire = true;
                    }
                } /* End follow for statement */
                if (fire == true) {
                    combine_attacks(ch, vict);
                    return;
                } else {
                    send_to_char(ch,
                                 "You do not have any followers who have readied an attack to combine or they do not have enough ki anymore to combine said attack.\r\n");
                    return;
                }
            } else if (ch->master) {
                if (GET_CHARGE(ch) >= GET_MAX_MANA(ch) * 0.05) {
                    act("@C$n@c appears to be concentrating hard and focusing $s energy!@n\r\n", true, ch, nullptr,
                        nullptr, TO_ROOM);
                    send_to_char(ch->master,
                                 "@BCOMBINE@c: @Y%s@C has prepared to combine a @c'@G%s@c'@C with the next group attack!@n\r\n",
                                 get_i_name(ch->master, ch), attack_names[temp]);
                    for (f = ch->master->followers; f; f = f->next) {
                        if (ch != f->follower)
                            send_to_char(f->follower,
                                         "@BCOMBINE@c: @Y%s@C has prepared to combine a @c'@G%s@c'@C with the next group attack!@n\r\n",
                                         get_i_name(f->follower, ch), attack_names[temp]);
                    }
                    GET_COMBINE(ch) = temp;
                } else {
                    send_to_char(ch, "You do not have the minimum 5%s ki charged.\r\n", "%");
                    return;
                }
            }
        }
    }
} /* End */

ACMD(do_sunder) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .25, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SUNDER)) {
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

    skill = init_skill(ch, SKILL_SUNDER); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 6);

    /* There is a player/mob targeted */
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_SUNDER, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = axion_dice(0);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Sundering Force")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Sundering Force, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Sundering Force, letting it fly harmlessly by!@n", false, ch, nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Sundering Force, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Sundering Force misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Sundering Force at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Sundering Force at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Sundering Force misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Sundering Force at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Sundering Force at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            int chance = rand_number(2, 12);
            dmg = damtype(ch, 55, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou raise both hands and aim the flats of your palms toward @c$N@W. As you concentrate your charged ki long arcing beams of blue energy shoot out and form a field around $M. With a quick motion you move your hands in opposite directions and wrench @c$N's@W body with the force of your energy!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward YOU. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around YOU. With a quick motion $e moves $s hands in opposite directions and wrenches YOUR body with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward @c$N@W. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around @c$N@W. With a quick motion @C$n@W moves $s hands in opposite directions and wrenches @c$N's@W body with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    if (chance >= 10) {
                        hurt(0, 160, ch, vict, nullptr, dmg, 1);
                    } else if (chance >= 8) {
                        hurt(1, 160, ch, vict, nullptr, dmg, 1);
                    } else {
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    }
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou raise both hands and aim the flats of your palms toward @c$N@W. As you concentrate your charged ki long arcing beams of blue energy shoot out and form a field around $M. With a quick motion you move your hands in opposite directions and wrench @c$N's@W head with the force of your energy!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward YOU. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around YOU. With a quick motion $e moves $s hands in opposite directions and wrenches YOUR head with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward @c$N@W. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around @c$N@W. With a quick motion @C$n@W moves $s hands in opposite directions and wrenches @c$N's@W head with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou raise both hands and aim the flats of your palms toward @c$N@W. As you concentrate your charged ki long arcing beams of blue energy shoot out and form a field around $M. With a quick motion you move your hands in opposite directions and wrench @c$N's@W body with the force of your energy!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward YOU. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around YOU. With a quick motion $e moves $s hands in opposite directions and wrenches YOUR body with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward @c$N@W. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around @c$N@W. With a quick motion @C$n@W moves $s hands in opposite directions and wrenches @c$N's@W body with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    if (chance >= 10) {
                        hurt(0, 160, ch, vict, nullptr, dmg, 1);
                    } else if (chance >= 8) {
                        hurt(1, 160, ch, vict, nullptr, dmg, 1);
                    } else {
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    }
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou raise both hands and aim the flats of your palms toward @c$N@W. As you concentrate your charged ki long arcing beams of blue energy shoot out and form a field around $M. With a quick motion you move your hands in opposite directions and wrench @c$N's@W arm with the force of your energy!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward YOU. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around YOU. With a quick motion $e moves $s hands in opposite directions and wrenches YOUR arm with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward @c$N@W. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around @c$N@W. With a quick motion @C$n@W moves $s hands in opposite directions and wrenches @c$N's@W arm with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 160, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak */
                    act("@WYou raise both hands and aim the flats of your palms toward @c$N@W. As you concentrate your charged ki long arcing beams of blue energy shoot out and form a field around $M. With a quick motion you move your hands in opposite directions and wrench @c$N's@W leg with the force of your energy!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward YOU. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around YOU. With a quick motion $e moves $s hands in opposite directions and wrenches YOUR leg with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises both hands and aims the flats of $s palms toward @c$N@W. As $e concentrates $s charged ki long arcing beams of blue energy shoot out and form a field around @c$N@W. With a quick motion @C$n@W moves $s hands in opposite directions and wrenches @c$N's@W leg with the force of $s energy!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(1, 160, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
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
        dmg = damtype(ch, 10, skill, attperc);
        dmg /= 10;
        act("@WYou fire a Sundering Force at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Sundering Force at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_zen) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .2, minimum = .1;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_ZEN)) {
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

    if (GET_SKILL_PERF(ch, SKILL_ZEN) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3) {
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

    skill = init_skill(ch, SKILL_ZEN); /* Set skill value */

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
        improve_skill(ch, SKILL_ZEN, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_ZEN) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Zen Blade Strike")) {
            if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > rand_number(1, 130)) {
                    act("@C$N@W moves quickly and blocks your Zen Blade Strike!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Zen Blade Strike!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Zen Blade Strike!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 54, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Zen Blade Strike, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Zen Blade Strike, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Zen Blade Strike, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 19, skill, SKILL_ZEN); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    improve_skill(vict, SKILL_DODGE, 0);

                    if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your Zen Blade Strike misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Zen Blade Strike at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Zen Blade Strike at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your Zen Blade Strike misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Zen Blade Strike at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Zen Blade Strike at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 54, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@CRaising your blade above your head, and closing your eyes, you focus ki into its edge. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as you open your eyes and instantly move past @g$N's@C body while slashing with the pure energy of your resolve! A large explosion of energy erupts across $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past YOUR body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past @g$N's@C body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across @g$N's@C body!@n",
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
                    act("@CRaising your blade above your head, and closing your eyes, you focus ki into its edge. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as you open your eyes and instantly move past @g$N's@C body while slashing with the pure energy of your resolve! A large explosion of energy erupts across $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past YOUR body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past @g$N's@C body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across @g$N's@C head!@n",
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
                    act("@CRaising your blade above your head, and closing your eyes, you focus ki into its edge. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as you open your eyes and instantly move past @g$N's@C body while slashing with the pure energy of your resolve! A large explosion of energy erupts across $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past YOUR body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past @g$N's@C body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across @g$N's@C body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@CRaising your blade above your head, and closing your eyes, you focus ki into its edge. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as you open your eyes and instantly move past @g$N's@C body while slashing with the pure energy of your resolve! A large explosion of energy erupts across $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past YOUR body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past @g$N's@C body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across @g$N's@C arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (rand_number(1, 100) >= 80 && !IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
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
                    act("@CRaising your blade above your head, and closing your eyes, you focus ki into its edge. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as you open your eyes and instantly move past @g$N's@C body while slashing with the pure energy of your resolve! A large explosion of energy erupts across $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past YOUR body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@G$n @Craises $s blade above $s head, and closes $s eyes. The edge of the blade begins to glow a soft blue as the blade begins to throb with excess energy. Peels of lighting begin to arc from the blade in all directions as $e opens $s eyes and instantly moves past @g$N's@C body while slashing with the pure energy of $s resolve! A large explosion of energy erupts across @g$N's@C leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    if (rand_number(1, 100) >= 80 && !IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
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
            if (GET_SKILL_PERF(ch, SKILL_ZEN) == 3 && attperc > minimum) {
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
        act("@WYou fire a Zen Blade Strike at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Zen Blade Strike at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_malice) {
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

    if (!check_skill(ch, SKILL_MALICE)) {
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

    skill = init_skill(ch, SKILL_MALICE); /* Set skill value */

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
        improve_skill(ch, SKILL_MALICE, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (time_info.hours <= 15 || time_info.hours > 22) {
            prob += 5;
        }

        if (!tech_handle_zanzoken(ch, vict, "Malice Breaker")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > rand_number(1, 130)) {
                    act("@C$N@W moves quickly and blocks your Malice Breaker!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Malice Breaker!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Malice Breaker!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 36, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);

                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Malice Breaker, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Malice Breaker, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Malice Breaker, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 36, skill, SKILL_MALICE); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(20);

                    improve_skill(vict, SKILL_DODGE, 0);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                } else {
                    act("@WYou can't believe it but your Malice Breaker misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Malice Breaker at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Malice Breaker at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);

                    return;
                }
            } else {
                act("@WYou can't believe it but your Malice Breaker misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Malice Breaker at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Malice Breaker at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);

            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 36, skill, attperc);
            if (time_info.hours <= 15) {
                dmg *= 1.25;
            } else if (time_info.hours <= 22) {
                dmg *= 1.4;
            }
            switch (rand_number(1, 6)) {
                case 1:
                    act("@WYou rush forward at @c$N@W, building ki into your arm. As you slam an open palm into $S chest, you send the charged energy into $S body. A few small explosions seem to hit across $S entire body, forcing $M to stumble back. Finally, you launch $M into the air, pointing a forefinger at $m like a pistol, and shout '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of your first strike on @c$N@W's chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wrushes forward at you, building ki into $s arm. As $e slams an open palm into your chest, $e sends the charged energy into your body. A few small explosions seem to hit across your entire body, forcing you to stumble back. Finally, @C$n@W launches you into the air, pointing a forefinger at your body like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on your chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W rushes forward at $N@W, building ki into $s arm. As $e slam an open palm into @c$N's@W chest, $e sends the charged energy into $S body. A few small explosions seem to hit across @c$N's@W entire body, forcing $M to stumble back. Finally, @C$n@W launches $M into the air, pointing a forefinger at $m like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on @c$N@W's chest!@n",
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
                    act("@WYou rush forward at @c$N@W, building ki into your arm. As you slam an open palm into $S head, you send the charged energy into $S body. A few small explosions seem to hit across $S entire body, forcing $M to stumble back. Finally, you launch $M into the air, pointing a forefinger at $m like a pistol, and shout '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of your first strike on @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wrushes forward at you, building ki into $s arm. As $e slams an open palm into your head, $e sends the charged energy into your body. A few small explosions seem to hit across your entire body, forcing you to stumble back. Finally, @C$n@W launches you into the air, pointing a forefinger at your body like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on your head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W rushes forward at $N@W, building ki into $s arm. As $e slam an open palm into @c$N's@W head, $e sends the charged energy into $S body. A few small explosions seem to hit across @c$N's@W entire body, forcing $M to stumble back. Finally, @C$n@W launches $M into the air, pointing a forefinger at $m like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on @c$N@W's head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4:
                    act("@WYou rush forward at @c$N@W, building ki into your arm. As you slam an open palm into $S gut, you send the charged energy into $S body. A few small explosions seem to hit across $S entire body, forcing $M to stumble back. Finally, you launch $M into the air, pointing a forefinger at $m like a pistol, and shout '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of your first strike on @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wrushes forward at you, building ki into $s arm. As $e slams an open palm into your gut, $e sends the charged energy into your body. A few small explosions seem to hit across your entire body, forcing you to stumble back. Finally, @C$n@W launches you into the air, pointing a forefinger at your body like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on your gut!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W rushes forward at $N@W, building ki into $s arm. As $e slam an open palm into @c$N's@W gut, $e sends the charged energy into $S body. A few small explosions seem to hit across @c$N's@W entire body, forcing $M to stumble back. Finally, @C$n@W launches $M into the air, pointing a forefinger at $m like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on @c$N@W's gut!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak */
                    act("@WYou rush forward at @c$N@W, building ki into your arm. As you slam an open palm into $S arm, you send the charged energy into $S body. A few small explosions seem to hit across $S entire body, forcing $M to stumble back. Finally, you launch $M into the air, pointing a forefinger at $m like a pistol, and shout '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of your first strike on @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wrushes forward at you, building ki into $s arm. As $e slams an open palm into your arm, $e sends the charged energy into your body. A few small explosions seem to hit across your entire body, forcing you to stumble back. Finally, @C$n@W launches you into the air, pointing a forefinger at your body like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W rushes forward at $N@W, building ki into $s arm. As $e slam an open palm into @c$N's@W arm, $e sends the charged energy into $S body. A few small explosions seem to hit across @c$N's@W entire body, forcing $M to stumble back. Finally, @C$n@W launches $M into the air, pointing a forefinger at $m like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on @c$N@W's arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 170, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 6: /* Weak 2 */
                    act("@WYou rush forward at @c$N@W, building ki into your arm. As you slam an open palm into $S leg, you send the charged energy into $S body. A few small explosions seem to hit across $S entire body, forcing $M to stumble back. Finally, you launch $M into the air, pointing a forefinger at $m like a pistol, and shout '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of your first strike on @c$N@W's leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wrushes forward at you, building ki into $s arm. As $e slams an open palm into your leg, $e sends the charged energy into your body. A few small explosions seem to hit across your entire body, forcing you to stumble back. Finally, @C$n@W launches you into the air, pointing a forefinger at your body like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W rushes forward at $N@W, building ki into $s arm. As $e slam an open palm into @c$N's@W leg, $e sends the charged energy into $S body. A few small explosions seem to hit across @c$N's@W entire body, forcing $M to stumble back. Finally, @C$n@W launches $M into the air, pointing a forefinger at $m like a pistol, and shouts '@MM@ma@Dl@wi@Wce Br@we@Dak@me@Mr@W!' as a dark, violet explosion erupts at the epicenter of $s first strike on @c$N@W's leg!@n",
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
        act("@WYou fire a Malice Breaker at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Malice Breaker at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);

    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_nova) {
    int perc, dge = 2, count = 0, skill;
    int64_t dmg;
    double attperc = .20, minimum = .1;
    struct char_data *vict = nullptr, *next_v = nullptr;
    char arg2[MAX_INPUT_LENGTH];

    one_argument(argument, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_STARNOVA)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_STARNOVA) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_STARNOVA) == 3) {
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

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
        send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_STARNOVA); /* Set skill value */
    if (GET_SKILL_PERF(ch, SKILL_STARNOVA) == 2) {
        skill += 5;
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
        if (time_info.hours <= 15 && time_info.hours > 22) {
            skill += 5;
        }
        handle_cooldown(ch, 6);
        if (skill < perc) {
            act("@WYou gather your charged energy and clench your upheld fists at either side of your body while crouching down. A hot glow of energy begins to form around your body before you lose your concentration and fail to create a @yS@Yt@Wa@wr @cN@Co@Wv@wa@W!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@W gathers $s charged energy and clenches $s upheld fists at either side of $s body while crouching down. A hot glow of energy begins to form around $s body before $e seems to lose $s concentration and fail to create a @yS@Yt@Wa@wr @cN@Co@Wv@wa@W!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            if (GET_SKILL_PERF(ch, SKILL_STARNOVA) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }

            improve_skill(ch, SKILL_STARNOVA, 0);
            return;
        }

        act("@WYou gather your charged energy and clench your upheld fists at either side of your body while crouching down. A hot glow of energy begins to form around your body in the shape of a sphere! Suddenly a shockwave of heat and energy erupts out into the surrounding area as your glorious @yS@Yt@Wa@wr @cN@Co@Wv@wa@W is born!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W gathers $s charged energy and clenches $s upheld fists at either side of $s body while crouching down. A hot glow of energy begins to form around $s body in the shape of a sphere! Suddenly a shockwave of heat and energy erupts out into the surrounding area as @C$n's@W glorious @yS@Yt@Wa@wr @cN@Co@Wv@wa@W is born!@n",
            true, ch, nullptr, nullptr, TO_ROOM);

        dmg = damtype(ch, 53, skill, attperc);

        if (time_info.hours <= 15) {
            dmg *= 1.25;
        } else if (time_info.hours <= 22) {
            dmg *= 1.4;
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
                    vict, TO_CHAR);
                act("@cYou disappear, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr, vict,
                    TO_VICT);
                act("@C$N@c disappears, avoiding the explosion before reappearing elsewhere!@n", false, ch, nullptr,
                    vict, TO_NOTVICT);
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

        if (GET_SKILL_PERF(ch, SKILL_STARNOVA) == 3 && attperc > minimum) {
            pcost(ch, attperc - 0.05, 0);
        } else {
            pcost(ch, attperc, 0);
        }

        improve_skill(ch, SKILL_STARNOVA, 0);
        return;
    } /* We have targets! Attempt to kills them! */

}

ACMD(do_head) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = physical_cost(ch, SKILL_HEADBUTT);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;
    int mult;
    one_argument(argument, arg);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_HEADBUTT)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 100)) {
        return;
    }

    skill = init_skill(ch, SKILL_HEADBUTT);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    if (!IS_KURZAK(ch) || GET_SKILL(ch, SKILL_HEADBUTT) < 100) {
        handle_cooldown(ch, 6);
    } else if (GET_SKILL(ch, SKILL_HEADBUTT) >= 100) {
        handle_cooldown(ch, 5);
    }
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_HEADBUTT, 0);
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

        if (!tech_handle_zanzoken(ch, vict, "headbutt")) {
            COMBO(ch) = -1;
            COMBHITS(ch) = 0;
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your headbutt with an attack of $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W headbutt with an attack of your own!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W parries @c$n's@W headbutt with an attack of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > rand_number(1, 130)) {
                    act("@C$N@W blocks your headbutt!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou block @C$n's@W headbutt!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W blocks @c$n's@W headbutt!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 3, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W dodges your headbutt!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W headbutt!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W dodges @c$n's@W headbutt!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it, your headbutt misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W throws a headbutt at you, but thankfully misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W throws a headbutt at @C$N@W, but misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it, your headbutt misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W throws a headbutt at you, but thankfully misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W throws a headbutt at @C$N@W, but misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, 0, 0);

                pcost(ch, 0, 0);
            }
            return;
        } else {
            dmg = damtype(ch, 52, skill, attperc);

            if (IS_KURZAK(ch)) {
                if (GET_SKILL(ch, SKILL_HEADBUTT) >= 60) {
                    dmg += dmg * 0.1;
                } else if (GET_SKILL(ch, SKILL_HEADBUTT) >= 40) {
                    dmg += dmg * 0.05;
                }
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou grab @c$N@W by the shoulders and slam your head into $S chest!@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@C$n@W grabs YOU by the shoulders and slams $s head into YOUR chest!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@C$n@W grabs @c$N@W by the shoulders and slams $s head into @c$N's@W chest!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WYou grab @c$N@W by the shoulders and slam your head into $S face!@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@C$n@W grabs YOU by the shoulders and slams $s head into YOUR face!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@C$n@W grabs @c$N@W by the shoulders and slams $s head into @c$N's@W face!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    if (!AFF_FLAGGED(vict, AFF_KNOCKED) &&
                        (rand_number(1, 7) >= 4 && (GET_HIT(vict) > GET_HIT(ch) / 5) &&
                         !AFF_FLAGGED(vict, AFF_SANCTUARY))) {
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@WYou are knocked out!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        vict->setStatusKnockedOut();
                    }
                    mult = calc_critical(ch, 0);
                    if (IS_KURZAK(ch) && !IS_NPC(ch)) {
                        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                            mult += 1;
                    }
                    dmg *= mult;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WYou grab @c$N@W by the shoulders and slam your head into $S chest!@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@C$n@W grabs YOU by the shoulders and slams $s head into YOUR chest!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@C$n@W grabs @c$N@W by the shoulders and slams $s head into @c$N's@W chest!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WYou grab @c$N@W and barely manage to slam your head into $S leg!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W grabs YOU and barely manages to slam $s head into YOUR leg!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$n@W grabs @c$N@W and barely manages to slam $s head into @c$N's@W leg!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak */
                    act("@WYou grab @c$N@W and barely manage to slam your head into $S arm!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@C$n@W grabs YOU and barely manages to slam $s head into YOUR arm!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$n@W grabs @c$N@W and barely manages to slam $s head into @c$N's@W arm!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "head");
            pcost(ch, 0, stcost);
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou headbutt $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W headbutt $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

        return;
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_bash) {
    int prob, perc, avo, index = 0, pry = 2, dge = 2, blk = 2, skill = 0;
    int64_t dmg, stcost = physical_cost(ch, SKILL_BASH);
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    double attperc = 0;

    one_argument(argument, arg);

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BASH)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!check_points(ch, 0, GET_MAX_HIT(ch) / 70)) {
        return;
    }

    skill = init_skill(ch, SKILL_BASH);

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 6);
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 0)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_BASH, 0);
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

        if (!tech_handle_zanzoken(ch, vict, "bash")) {
            COMBO(ch) = -1;
            COMBHITS(ch) = 0;
            pcost(ch, 0, stcost / 2);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (pry > rand_number(1, 140) && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    act("@C$N@W parries your bash with an attack of $S own!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou parry @C$n's@W bash with an attack of your own!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W parries @c$n's@W bash with an attack of $S own!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    improve_skill(vict, SKILL_PARRY, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(vict, -2, skill, attperc);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, vict, ch, nullptr, dmg, -1);

                    return;
                } else if (blk > rand_number(1, 130)) {
                    act("@C$N@W blocks your bash!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou block @C$n's@W bash!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W blocks @c$n's@W bash!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_BLOCK, 0);
                    pcost(ch, 0, stcost / 2);
                    pcost(vict, 0, GET_MAX_HIT(vict) / 500);
                    dmg = damtype(ch, 3, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);

                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W dodges your bash!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W bash!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W dodges @c$n's@W bash!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    improve_skill(vict, SKILL_DODGE, 0);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                } else {
                    act("@WYou can't believe it, your bash misses!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W throws a bash at you, but thankfully misses!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W throws a bash at @C$N@W, but misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    pcost(ch, 0, stcost / 2);
                    hurt(0, 0, ch, vict, nullptr, 0, 0);

                    return;
                }
            } else {
                act("@WYou can't believe it, your bash misses!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W throws a bash at you, but thankfully misses!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$n@W throws a bash at @C$N@W, but misses!@n", true, ch, nullptr, vict, TO_NOTVICT);
                hurt(0, 0, ch, vict, nullptr, 0, 0);

                pcost(ch, 0, 0);
            }
            return;
        } else {
            dmg = damtype(ch, 51, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WBending over slightly you aim your body at @c$N@W and instantly launch yourself toward $M at full speed! You slam into $S body with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bends over slightly aiming $s body at YOU and then instantly launches $mself toward YOU at full speed! @C$n@W slams into YOUR body with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bends over slightly aiming $s body at @c$N@W and then instantly launches $mself toward $M at full speed! @C$n@W slams into $S body with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2: /* Critical */
                    act("@WBending over slightly you aim your body at @c$N@W and instantly launch yourself toward $M at full speed! You slam into $S head with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bends over slightly aiming $s body at YOU and then instantly launches $mself toward YOU at full speed! @C$n@W slams into YOUR head with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bends over slightly aiming $s body at @c$N@W and then instantly launches $mself toward $M at full speed! @C$n@W slams into $S head with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 3:
                    act("@WBending over slightly you aim your body at @c$N@W and instantly launch yourself toward $M at full speed! You slam into $S gut with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bends over slightly aiming $s body at YOU and then instantly launches $mself toward YOU at full speed! @C$n@W slams into YOUR gut with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bends over slightly aiming $s body at @c$N@W and then instantly launches $mself toward $M at full speed! @C$n@W slams into $S gut with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 4: /* Weak */
                    act("@WBending over slightly you aim your body at @c$N@W and instantly launch yourself toward $M at full speed! You slam into $S leg with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bends over slightly aiming $s body at YOU and then instantly launches $mself toward YOU at full speed! @C$n@W slams into YOUR leg with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bends over slightly aiming $s body at @c$N@W and then instantly launches $mself toward $M at full speed! @C$n@W slams into $S leg with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 2);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5: /* Weak */
                    act("@WBending over slightly you aim your body at @c$N@W and instantly launch yourself toward $M at full speed! You slam into $S arm with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W bends over slightly aiming $s body at YOU and then instantly launches $mself toward YOU at full speed! @C$n@W slams into YOUR arm with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W bends over slightly aiming $s body at @c$N@W and then instantly launches $mself toward $M at full speed! @C$n@W slams into $S arm with a crashing impact!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 0);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
            }
            tech_handle_fireshield(ch, vict, "body");

            if (vict && rand_number(1, 5) >= 4)
                tech_handle_crashdown(vict, ch);

            if (rand_number(1, 5) >= 5)
                tech_handle_crashdown(ch, vict);

            pcost(ch, 0, stcost);
            handle_multihit(ch, vict);

            return;
        }
    } else if (obj) {
        if (!can_kill(ch, nullptr, obj, 0)) {
            return;
        }
        dmg = ((GET_HIT(ch) / 10000) + (GET_STR(ch)));
        act("@WYou bash $p@W as hard as you can!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W bash $p@W extremely hard!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, 0, stcost);

        return;
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_seishou) {
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

    if (!check_skill(ch, SKILL_SEISHOU)) {
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

    skill = init_skill(ch, SKILL_SEISHOU); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 4);

    /* There is a player/mob targeted */
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_SEISHOU, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Seishou Enko")) {
            pcost(ch, attperc / 4, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Seishou Enko, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Seishou Enko, letting it fly harmlessly by!@n", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Seishou Enko, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc / 4, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Seishou Enko misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Seishou Enko at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Seishou Enko at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc / 4, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Seishou Enko misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Seishou Enko at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Seishou Enko at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc / 4, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 50, skill, attperc);
            if (GET_MOLT_LEVEL(ch) >= 150) {
                dmg *= 2;
            }
            switch (rand_number(1, 7)) {
                case 1:
                    act("@WYou aim your mouth at @C$N@W and focus your charged ki. In an instant you fire a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S body with searing heat!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Waims $s mouth at YOU and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at YOU! Almost instantly it blasts into YOUR body with searing heat!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Waims $s mouth at @c$N@W and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S body with searing heat!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 2:
                case 3:
                case 4: /* Critical */
                    act("@WYou aim your mouth at @C$N@W and focus your charged ki. In an instant you fire a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S head with searing heat!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Waims $s mouth at YOU and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at YOU! Almost instantly it blasts into YOUR head with searing heat!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Waims $s mouth at @c$N@W and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S head with searing heat!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 5:
                    act("@WYou aim your mouth at @C$N@W and focus your charged ki. In an instant you fire a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S gut with searing heat!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Waims $s mouth at YOU and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at YOU! Almost instantly it blasts into YOUR gut with searing heat!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Waims $s mouth at @c$N@W and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S gut with searing heat!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 6: /* Weak */
                    act("@WYou aim your mouth at @C$N@W and focus your charged ki. In an instant you fire a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S arm with searing heat!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Waims $s mouth at YOU and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at YOU! Almost instantly it blasts into YOUR arm with searing heat!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Waims $s mouth at @c$N@W and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S arm with searing heat!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 1);
                    /* dam_eq_loc: 1 Arms, 2 legs, 3 head, and 4 body. */
                    break;
                case 7: /* Weak 2 */
                    act("@WYou aim your mouth at @C$N@W and focus your charged ki. In an instant you fire a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S leg with searing heat!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Waims $s mouth at YOU and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at YOU! Almost instantly it blasts into YOUR leg with searing heat!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Waims $s mouth at @c$N@W and seems to focus $s ki. In an instant $e fires a large @Rr@re@Rd@W @rS@Re@Wi@ws@rh@Ro@Wu @wE@rn@Rk@Wo at $M! Almost instantly it blasts into $S leg with searing heat!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
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
        dmg = damtype(ch, 10, skill, attperc);
        dmg /= 10;
        act("@WYou fire a Seishou Enko at $p@W!@n", true, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires a Seishou Enko at $p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_throw) {
    struct char_data *vict = nullptr, *tch = nullptr;
    struct obj_data *obj = nullptr;
    char arg[MAX_INPUT_LENGTH];
    char arg2[1000], chunk[2000], arg3[1000];
    int odam = 0, miss = true, perc = 0, prob = 0, perc2 = 0, grab = false;
    int64_t damage;

    half_chop(argument, arg, chunk);

    if (!*arg) {
        send_to_char(ch, "Throw what?\r\n");
        return;
    }

    if (is_sparring(ch)) {
        send_to_char(ch, "You can not spar with throw.\r\n");
        return;
    }

    if (*chunk) {
        two_arguments(chunk, arg2, arg3);
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        if (!(tch = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
            send_to_char(ch, "You do not have that object or character to throw!\r\n");
            return;
        }
    }

    if (!(vict = get_char_vis(ch, arg2, nullptr, FIND_CHAR_ROOM))) {
        if (FIGHTING(ch) && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char(ch, "Who do you want to target?\r\n");
            return;
        }
    }

    if (GET_HIT(vict) <= 1) {
        return;
    }

    if (!can_kill(ch, vict, nullptr, 1)) {
        return;
    }

    if (handle_defender(vict, ch)) {
        struct char_data *def = GET_DEFENDER(vict);
        vict = def;
    }

    auto gravity = ch->currentGravity();

    /* We are throwing an object. */
    if (obj) {
        if (ch->throws == -1) {
            ch->throws = 0;
            return;
        }

        if ((ch->getCurST()) < ((GET_MAX_HIT(ch) / 200) + GET_OBJ_WEIGHT(obj))) {
            send_to_char(ch, "You do not have enough stamina to do it...\r\n");
            return;
        }
        if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "That is broken and useless to throw!\r\n");
            return;
        }
        if (!ch->canCarryWeight(obj)) {
            send_to_char(ch, "The gravity has made that too heavy for you to throw!\r\n");
            return;
        } else {
            int penalty = 0, chance = axion_dice(0) + axion_dice(0), wtype = 0, wlvl = 1, multithrow = true;
            handle_cooldown(ch, 5);
            improve_skill(ch, SKILL_THROW, 0);
            damage = ((GET_OBJ_WEIGHT(obj) / 3) * (GET_STR(ch)) * (GET_CHA(ch) / 3)) + (GET_MAX_HIT(ch) * 0.01);
            damage += (damage * 0.01) * (gravity / 4);

            if (GET_PREFERENCE(ch) == PREFERENCE_THROWING) {
                chance -= chance * 0.25;
            }

            if (OBJ_FLAGGED(obj, ITEM_WEAPLVL1)) {
                damage += damage * 0.1;
                wlvl = 1;
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL2)) {
                damage += damage * 0.2;
                wlvl = 2;
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL3)) {
                damage += damage * 0.3;
                wlvl = 3;
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL4)) {
                damage += damage * 0.4;
                wlvl = 4;
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL5)) {
                damage += damage * 0.5;
                wlvl = 5;
            }
            if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
                if (GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) == TYPE_PIERCE - TYPE_HIT) {
                    wtype = 1;
                } else if (GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) == TYPE_SLASH - TYPE_HIT) {
                    wtype = 2;
                } else if (GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) == TYPE_CRUSH - TYPE_HIT) {
                    wtype = 3;
                } else if (GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) == TYPE_STAB - TYPE_HIT) {
                    wtype = 4;
                } else if (GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) == TYPE_BLAST - TYPE_HIT) {
                    wtype = 5;
                    damage = ((GET_OBJ_WEIGHT(obj) * GET_STR(ch)) * (GET_CHA(ch) / 3)) + (GET_MAX_HIT(ch) * 0.01);
                    damage += gravity * (gravity / 2);
                } else {
                    wtype = 6;
                }
            }
            if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_STEEL) {
                odam = rand_number(5, 30);
            } else if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_IRON) {
                odam = rand_number(18, 50);
            } else if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_MITHRIL) {
                odam = rand_number(5, 15);
            } else if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_KACHIN) {
                odam = rand_number(5, 15);
            } else if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_STONE) {
                odam = rand_number(20, 50);
            } else if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_DIAMOND) {
                odam = rand_number(5, 20);
            } else if (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL) == MATERIAL_ENERGY) {
                if (rand_number(1, 2) == 2) {
                    odam = 0;
                } else {
                    odam = rand_number(1, 3);
                }
            } else {
                odam = rand_number(90, 100);
            }
            if (!OBJ_FLAGGED(obj, ITEM_THROW)) {
                penalty = 15;
                multithrow = false;
                damage = damage * 0.45;
            } else {
                odam = rand_number(0, 1);
                damage += (GET_STR(ch)) * ((GET_HIT(ch) * 0.00012) + rand_number(1, 20));
                damage += wlvl * (damage * 0.1);
            }

            if (wlvl == 5) {
                damage += 25000;
            } else if (wlvl == 4) {
                damage += 16000;
            } else if (wlvl == 3) {
                damage += 10000;
            } else if (wlvl == 2) {
                damage += 5000;
            } else if (wlvl == 1) {
                damage += 1000;
            }

            int hot = false;
            if (OBJ_FLAGGED(obj, ITEM_HOT)) {
                hot = true;
            }


            if (wtype > 0 && wtype != 5 && odam > 1) {
                odam = 1;
            }
            perc = init_skill(ch, SKILL_THROW);
            perc2 = init_skill(vict, SKILL_DODGE);
            prob = axion_dice(penalty);
            if (*arg3) {
                if (!strcasecmp(arg3, "1") || !strcasecmp(arg3, "single")) {
                    multithrow = false;
                } else {
                    send_to_char(ch,
                                 "Syntax: throw (obj | character) (target) <-- This will multithrow if able\nSyntax: throw (obj) (target) (1 | single) <-- This will not multi throw)\r\n");
                    return;
                }
            }

            if (!tech_handle_zanzoken(ch, vict, "$p")) {
                COMBO(ch) = -1;
                COMBHITS(ch) = 0;
                int stcost = ((GET_MAX_HIT(ch) / 200) + GET_OBJ_WEIGHT(obj));
                vict->affected_by.reset(AFF_ZANZOKEN);
                pcost(ch, 0, stcost / 2);
                pcost(vict, 0, GET_MAX_HIT(vict) / 200);
                obj_from_char(obj);
                obj_to_room(obj, IN_ROOM(vict));
                return;
            }

            if (perc - (perc2 / 10) < prob) {
                if (OBJ_FLAGGED(obj, ITEM_ICE) && IS_DEMON(vict)) {
                    act("You throw $p at $N@n, but it melts before touching $M!", true, ch, obj, vict, TO_CHAR);
                    act("$n@n throws $p at $N@n, but it melts before touching $M!", true, ch, obj, vict, TO_NOTVICT);
                    act("$n@n throws $p at you, but it melts before touching you!", true, ch, obj, vict, TO_VICT);
                    ch->decCurST(((GET_MAX_HIT(ch) / 100) + GET_OBJ_WEIGHT(obj)));
                    extract_obj(obj);
                    return;
                }
                if (perc2 > 0) {
                    act("You throw $p at $N@n, but $E manages to dodge it easily!", true, ch, obj, vict, TO_CHAR);
                    act("$n@n throws $p at $N@n, but $E manages to dodge it easily!", true, ch, obj, vict, TO_NOTVICT);
                    act("$n@n throws $p at you, but you easily dodge it.", true, ch, obj, vict, TO_VICT);
                } else if (perc2 <= 0) {
                    act("You throw $p at $N@n, but unfortunatly miss!", true, ch, obj, vict, TO_CHAR);
                    act("$n@n throws $p at $N@n, but unfortunatly misses!", true, ch, obj, vict, TO_NOTVICT);
                    act("$n@n throws $p at you, but thankfully misses you.", true, ch, obj, vict, TO_VICT);
                }
                ch->decCurST(((GET_MAX_HIT(ch) / 100) + GET_OBJ_WEIGHT(obj)));
                if (!OBJ_FLAGGED(obj, ITEM_UNBREAKABLE)) {
                    GET_OBJ_VAL(obj, VAL_ALL_HEALTH) -= odam / 2;
                }
                LASTATK(ch) = -50;
                hurt(0, 0, ch, vict, nullptr, 0, 0);
                obj_from_char(obj);
                obj_to_room(obj, IN_ROOM(vict));
                ch->decCurST(((GET_MAX_HIT(ch) / 200) + GET_OBJ_WEIGHT(obj)));
                if (!GET_EQ(ch, WEAR_WIELD1) && !GET_EQ(ch, WEAR_WIELD2))
                    perc += 20;
                if (perc + GET_CHA(ch) >= chance + penalty && multithrow == true && GET_HIT(vict) > 1 &&
                    ch->throws > 1) {
                    do_throw(ch, argument, 0, 0);
                    ch->throws -= 1;
                } else if (perc + GET_CHA(ch) >= chance + penalty && multithrow == true && GET_HIT(vict) > 1 &&
                           ch->throws == 1) {
                    do_throw(ch, argument, 0, 0);
                    ch->throws = -1;
                } else {
                    ch->throws = 0;
                }
                WAIT_STATE(ch, PULSE_3SEC);
                return;
            } else if (perc - (perc2 / 10) > prob) {
                miss = false;
            }
            if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ENERGIZE) && (ch->getCurKI()) >= GET_MAX_MANA(ch) * 0.02) {
                damage += (damage * (0.0016 * GET_SKILL(ch, SKILL_ENERGIZE)));
                act("You charge $p with the energy in your fingertips! As it begins to @Yglow a bright hot @Rred@n you throw $p at $N@n full speed, and watch it smash into $M!",
                    true, ch, obj, vict, TO_CHAR);
                act("$n@n charges $p with the energy in $s fingertips! As it begins to @Yglow a bright hot @Rred@n $e throws $p at $N@n full speed, and watches it smash into $M!",
                    true, ch, obj, vict, TO_NOTVICT);
                act("$n@n charges $p with the energy in $s fingertips! As it begins to @Yglow a bright hot @Rred@n $e throws $p at YOU@n full speed, and watches it smash into YOU!!",
                    true, ch, obj, vict, TO_VICT);
                if (GET_MAX_MANA(ch) * 0.02 > 0) {
                    ch->decCurKI(ch->getMaxKI() * .02);
                } else {
                    ch->decCurKI(1);
                }
                improve_skill(ch, SKILL_ENERGIZE, 0);
            } else if (wtype == 0) {
                act("You throw $p at $N@n full speed, and watch it smash into $M!", true, ch, obj, vict, TO_CHAR);
                act("$n@n throws $p at $N@n full speed, and watches it smash into $M!", true, ch, obj, vict,
                    TO_NOTVICT);
                act("$n@n throws $p at you full speed. You reel as it smashes into your body!", true, ch, obj, vict,
                    TO_VICT);
            } else if (wtype == 1 || wtype == 2) {
                act("You pull out and throw $p at $N@n full speed, and watch it sink into $M!", true, ch, obj, vict,
                    TO_CHAR);
                act("$n@n pulls out and throws $p at $N@n full speed, and watches it sink into $M!", true, ch, obj,
                    vict, TO_NOTVICT);
                act("$n@n pulls out and throws $p at you full speed. You reel as it sink into your body!", true, ch,
                    obj, vict, TO_VICT);
            } else if (wtype == 3) {
                act("You swing $p overhead and throw it at $N@n full speed, and watch it slam into $M!", true, ch, obj,
                    vict, TO_CHAR);
                act("$n@n swings $p overhead and throws it at $N@n full speed, and watches it slam into $M!", true, ch,
                    obj, vict, TO_NOTVICT);
                act("$n@n swings $p overhead and throws it at you full speed. You reel as it slam into your body!",
                    true, ch, obj, vict, TO_VICT);
            } else if (wtype == 4) {
                act("You bring $p over your shoulder and throw it at $N@n full speed, and watch it sink into $M!", true,
                    ch, obj, vict, TO_CHAR);
                act("$n@n brings $p over $s shoulder and throws it at $N@n full speed, and watches it sink into $M!",
                    true, ch, obj, vict, TO_NOTVICT);
                act("$n@n brings $p over $s shoulder and throws $p at you full speed. You reel as it sink into your body!",
                    true, ch, obj, vict, TO_VICT);
            } else if (wtype == 5) {
                act("You pull out and throw $p at $N@n full speed, and watch it hit $M!", true, ch, obj, vict, TO_CHAR);
                act("$n@n pulls out and throws $p at $N@n full speed, and watches it hit $M!", true, ch, obj, vict,
                    TO_NOTVICT);
                act("$n@n pulls out and throws $p at you full speed. You reel as it hits your body!", true, ch, obj,
                    vict, TO_VICT);
            }
            if (!OBJ_FLAGGED(obj, ITEM_UNBREAKABLE)) {
                GET_OBJ_VAL(obj, VAL_ALL_HEALTH) -= odam;
            }
            LASTATK(ch) = -50;
            if ((GET_OBJ_VAL(obj, VAL_ALL_HEALTH) - odam) <= 0 && !OBJ_FLAGGED(obj, ITEM_UNBREAKABLE)) {
                act("You smile as $p breaks on $N's@n face!", true, ch, obj, vict, TO_CHAR);
                act("$n@n smiles as $p breaks on $N's@n face!", true, ch, obj, vict, TO_NOTVICT);
                act("$n@n smiles as $p breaks on your face!", true, ch, obj, vict, TO_VICT);
                obj->extra_flags.flip(ITEM_BROKEN);
            } else if (GET_DEX(ch) >= axion_dice(0)) {
                if (IS_ANDROID(vict) || IS_MECHANICAL(vict)) {
                    act("@RSome pieces of metal are sent flying!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RSome pieces of metal are sent flying!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@RSome pieces of metal are sent flying!@n", true, ch, nullptr, vict, TO_NOTVICT);
                } else if (IS_MAJIN(vict)) {
                    act("@RA wide hole is left in $S gooey flesh!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RA wide hole is left is your gooey flesh!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@RA wide hole is left in $N@R's gooey flesh@n", true, ch, nullptr, vict, TO_NOTVICT);
                } else {
                    act("@RBlood flies out from the impact!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RBlood flies out from the impact!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@RBlood flies out from the impact!@n", true, ch, nullptr, vict, TO_NOTVICT);
                }
                if (OBJ_FLAGGED(obj, ITEM_ICE)) {
                    if (!IS_ANDROID(vict) && !IS_ICER(vict)) {
                        vict->decCurST((vict->getMaxST() * .005) + GET_OBJ_WEIGHT(obj));
                        act("@mYou lose some stamina to the @ccold@m!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@C$N@m loses some stamina to the @ccold@m!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@C$N@m loses some stamina to the @ccold@m!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    }
                }
                damage *= calc_critical(ch, 0);
            }
            if (hot == true) {
                if (!IS_DEMON(vict) && !GET_BONUS(vict, BONUS_FIREPROOF)) {
                    act("@R$N@R is burned by it!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYou are burned by it!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@R is burned by it!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    vict->affected_by.set(AFF_BURNED);
                    damage += damage * 0.4;
                }
            }
            if (GET_PREFERENCE(ch) == PREFERENCE_KI) {
                damage -= damage * 0.20;
            }
            if (GET_OBJ_VNUM(obj) == 5899 || GET_OBJ_VNUM(obj) == 5898) {
                damage *= 0.35;
            }
            hurt(0, 0, ch, vict, nullptr, damage, 0);
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(vict));

            ch->decCurST(((GET_MAX_HIT(ch) / 200) + GET_OBJ_WEIGHT(obj)));
            if (!GET_EQ(ch, WEAR_WIELD1) && !GET_EQ(ch, WEAR_WIELD2))
                perc += 12;
            if (perc + GET_CHA(ch) >= chance + penalty && multithrow == true && GET_HIT(vict) > 1 && ch->throws > 1) {
                do_throw(ch, argument, 0, 0);
                ch->throws -= 1;
            } else if (perc + GET_CHA(ch) >= chance + penalty && multithrow == true && GET_HIT(vict) > 1 &&
                       ch->throws == 1) {
                do_throw(ch, argument, 0, 0);
                ch->throws = -1;
            } else {
                ch->throws = 0;
            }
            WAIT_STATE(ch, PULSE_3SEC);
            return;
        }
    } /* End object section. */

    /* We are throwing a character at someone else. */
    if (tch) {
        if (tch == vict) {
            send_to_char(ch, "You can't throw someone at theirself.\r\n");
            return;
        }

        if (!can_kill(ch, tch, nullptr, 0)) {
            send_to_char(ch, "The one you are throwing can't be harmed.\r\n");
            return;
        }

        if (GET_SPEEDI(tch) < GET_SPEEDI(ch) && rand_number(1, 106) < GET_SKILL(ch, SKILL_THROW)) {
            grab = true;
        }

        if ((ch->getCurST()) < ((GET_MAX_HIT(ch) / 100) + tch->getTotalWeight())) {
            send_to_char(ch, "You do not have enough stamina to do it...\r\n");
            return;
        }
        if (!ch->canCarryWeight(tch)) {
            send_to_char(ch, "The gravity has made them too heavy for you to throw!\r\n");
            return;
        }
        if (grab == false) {
            act("@WYou try to grab @C$N@W and throw them, but they manage to dodge your attempt!@n", true, ch, nullptr,
                tch, TO_CHAR);
            act("@C$n@W tries to @RGRAB@W you and @RTHROW@W you, but you manage to dodge the attempt!@n", true, ch,
                nullptr, tch, TO_VICT);
            act("@C$n@W tries to @RGRAB@W @c$N@W and @RTHROW@W $M, but $E manages to dodge the attempt!@n", true, ch,
                nullptr, tch, TO_NOTVICT);
            hurt(0, 0, ch, tch, nullptr, 0, 0);
            handle_cooldown(ch, 5);
            ch->decCurST((GET_MAX_HIT(ch) / 200) + tch->getWeight());
            return;
        } else {
            handle_cooldown(ch, 5);
            improve_skill(ch, SKILL_THROW, 0);
            damage = ((tch->getWeight() * GET_STR(ch)) * (GET_CHA(ch) / 3)) + (GET_MAX_HIT(ch) / 100);
            damage += gravity * (gravity / 2);
            perc = init_skill(ch, SKILL_THROW);
            perc2 = init_skill(vict, SKILL_DODGE);
            prob = rand_number(1, 106);
            if (perc - (perc2 / 10) < prob) {
                if (perc2 > 0) {
                    act("@WYou grab @C$N@W and spinning around quickly you throw $M!@n", true, ch, nullptr, tch,
                        TO_CHAR);
                    act("@C$n@W grabs YOU and spinning around quickly $e throws you!@n", true, ch, nullptr, tch,
                        TO_VICT);
                    act("@C$n@W grabs @c$N@W and spinning around quickly $e throws $M!@n", true, ch, nullptr, tch,
                        TO_NOTVICT);
                    act("@WThrown through the air, YOU fly at @c$N@W, but $E manages to dodge and you manage recover your bearings a moment later!@n",
                        true, tch, nullptr, vict, TO_CHAR);
                    act("@WThrown through the air, @C$n@W flies at YOU, but you manage to dodge and @C$n@W recovers $s bearings a moment later!@n",
                        true, tch, nullptr, vict, TO_VICT);
                    act("@WThrown through the air, @C$n@W flies at @c$N@W, but $E manages to dodge and @C$n@W recovers $s bearingsa moment later!@n",
                        true, tch, nullptr, vict, TO_NOTVICT);
                } else if (perc2 <= 0) {
                    act("@WYou grab @C$N@W and spinning around quickly you throw $M!@n", true, ch, nullptr, tch,
                        TO_CHAR);
                    act("@C$n@W grabs YOU and spinning around quickly $e throws you!@n", true, ch, nullptr, tch,
                        TO_VICT);
                    act("@C$n@W grabs @c$N@W and spinning around quickly $e throws $M!@n", true, ch, nullptr, tch,
                        TO_NOTVICT);
                    act("@WThrown through the air, YOU fly at @c$N@W, but the throw is a miss! You manage recover your bearings a moment later!@n",
                        true, tch, nullptr, vict, TO_CHAR);
                    act("@WThrown through the air, @C$n@W flies at YOU, but the throw is a miss! @C$n@W recovers $s bearings a moment later!@n",
                        true, tch, nullptr, vict, TO_VICT);
                    act("@WThrown through the air, @C$n@W flies at @c$N@W, but the throw is a miss! @C$n@W recovers $s bearingsa moment later!@n",
                        true, tch, nullptr, vict, TO_NOTVICT);
                }
                ch->decCurST(((GET_MAX_HIT(ch) / 100) + tch->getWeight()));
                act("@W--@R$N@W--@n", true, ch, nullptr, vict, TO_CHAR);
                act("@W--@R$N@W--@n", true, tch, nullptr, vict, TO_CHAR);
                act("@W--@RYOU@W--@n", true, vict, nullptr, nullptr, TO_CHAR);
                hurt(0, 0, ch, vict, nullptr, 0, 0);
                act("@W--@R$N@W--@n", true, ch, nullptr, tch, TO_CHAR);
                act("@W--@R$N@W--@n", true, vict, nullptr, tch, TO_CHAR);
                act("@W--@RYOU@W--@n", true, tch, nullptr, nullptr, TO_CHAR);
                hurt(0, 0, ch, tch, nullptr, 0, 0);
                return;
            } else if (perc - (perc2 / 10) >= prob) {
                miss = false;
            }
            if (miss == false) {
                act("@WYou grab @C$N@W and spinning around quickly you throw $M!@n", true, ch, nullptr, tch, TO_CHAR);
                act("@C$n@W grabs YOU and spinning around quickly $e throws you!@n", true, ch, nullptr, tch, TO_VICT);
                act("@C$n@W grabs @c$N@W and spinning around quickly $e throws $M!@n", true, ch, nullptr, tch,
                    TO_NOTVICT);
                act("@WThrown through the air, YOU fly at @c$N@W and smash into $M!@n", true, tch, nullptr, vict,
                    TO_CHAR);
                act("@WThrown through the air, @C$n@W flies at YOU and smashes into YOU!@n", true, tch, nullptr, vict,
                    TO_VICT);
                act("@WThrown through the air, @C$n@W flies at @c$N@W and smashes into $M!@n", true, tch, nullptr, vict,
                    TO_NOTVICT);
                act("@W--@R$N@W--@n", true, ch, nullptr, vict, TO_CHAR);
                act("@W--@R$N@W--@n", true, tch, nullptr, vict, TO_CHAR);
                act("@W--@RYOU@W--@n", true, vict, nullptr, nullptr, TO_CHAR);
                hurt(0, 0, ch, vict, nullptr, damage, 0);
                act("@W--@R$N@W--@n", true, ch, nullptr, tch, TO_CHAR);
                if (vict) {
                    act("@W--@R$N@W--@n", true, vict, nullptr, tch, TO_CHAR);
                }
                act("@W--@RYOU@W--@n", true, tch, nullptr, nullptr, TO_CHAR);
                hurt(0, 0, ch, tch, nullptr, damage, 0);
            }
            ch->decCurST(((GET_MAX_HIT(ch) / 200) + tch->getWeight()));
            WAIT_STATE(ch, PULSE_3SEC);
        }
    } /* End throwing character. */

    /* Whoops!? */
    if (!obj && !tch) {
        send_to_imm("ERROR: Throw resolved without character or object.");
        return;
    }
}

ACMD(do_selfd) {

    struct char_data *tch = nullptr, *next_v = nullptr;
    int64_t dmg = 0;

    if (IS_NPC(ch))
        return;

    if (IN_ARENA(ch)) {
        send_to_char(ch, "You can not use self destruct in the arena.\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_SPIRIT)) {
        send_to_char(ch, "You are already dead!\r\n");
        return;
    }

    if (GET_LEVEL(ch) < 9) {
        send_to_char(ch, "You can't self destruct while protected by the newbie shield!\r\n");
        return;
    }

    /*Andros Start*/
    if (!GET_SDCOOLDOWN(ch) <= 0) {
        send_to_char(ch, "Your body is still recovering from the last self destruct!\r\n");
        return;
    } /*Andros End*/


    if (!GET_SKILL(ch, SKILL_SELFD)) {
        int num = rand_number(10, 20);
        SET_SKILL(ch, SKILL_SELFD, num);
    }

    if (!PLR_FLAGGED(ch, PLR_SELFD)) {
        act("@RYour body starts to glow @wwhite@R and flash. The flashes start out slowly but steadilly increase in speed. Your aura begins to burn around your body at the same time in a violent fashion!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@R$n's body starts to glow @wwhite@R and flash. The flashes start out slowly but steadilly increase in speed. $n's aura begins to burn around $s body at the same time in a violent fashion!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->playerFlags.set(PLR_SELFD);
        return;
    } else if (!PLR_FLAGGED(ch, PLR_SELFD2)) {
        act("@wYour body slowly stops flashing. Steam rises from your skin as you slowly let off the energy you built up in a safe manner.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@w$n's body slowly stops flashing. Steam rises from $s skin as $e slowly lets off the energy $e built up in a safe manner.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->playerFlags.reset(PLR_SELFD);
        return;
    } else if (GRAPPLING(ch) != nullptr && !can_kill(ch, GRAPPLING(ch), nullptr, 3)) {
        act("@wYour body slowly stops flashing. Steam rises from your skin as you slowly let off the energy you built up in a safe manner.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@w$n's body slowly stops flashing. Steam rises from $s skin as $e slowly lets off the energy $e built up in a safe manner.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        send_to_char(ch, "You can't kill them, the immortals won't allow it!\r\n");
        ch->playerFlags.reset(PLR_SELFD);
        return;
    } else if (GRAPPLING(ch) != nullptr) {
        tch = GRAPPLING(ch);
        dmg += GET_CHARGE(ch);
        GET_CHARGE(ch) = 0;
        dmg += (ch->getBasePL()) * 0.6;
        dmg += (ch->getBaseST());
        ch->decCurHealthPercent(1, 1);
        GET_SUPPRESS(ch) = 0;
        act("@RYou EXPLODE! The explosion concentrates on @r$N@R, engulfing $M in a sphere of deadly energy!@n", true,
            ch, nullptr, tch, TO_CHAR);
        act("@R$n EXPLODES! The explosion concentrates on YOU, engulfing your body in a sphere of deadly energy!@n",
            true, ch, nullptr, tch, TO_VICT);
        act("@R$n EXPLODES! The explosion concentrates on @r$N@R, engulfing $M in a sphere of deadly energy!@n", true,
            ch, nullptr, tch, TO_NOTVICT);
        hurt(0, 0, ch, tch, nullptr, dmg, 1);
        ch->playerFlags.reset(PLR_SELFD);
        ch->playerFlags.reset(PLR_SELFD2);
        if (PLR_FLAGGED(ch, PLR_IMMORTAL)) {
            GET_SDCOOLDOWN(ch) = 600;
        }
        if ((IS_MAJIN(ch) || IS_BIO(ch)) && ch->getCurLFPercent() > 0.5) {
            ch->decCurLFPercent(2, -1);
            ch->playerFlags.set(PLR_GOOP);
            ch->gooptime = 70;
        } else {
            die(ch, nullptr);
        }
        int num = rand_number(10, 20) + GET_SKILL(ch, SKILL_SELFD);
        if (GET_SKILL(ch, SKILL_SELFD) + num <= 100) {
            SET_SKILL(ch, SKILL_SELFD, num);
        } else {
            SET_SKILL(ch, SKILL_SELFD, 100);
        }
        return;
    } else {
        dmg += GET_CHARGE(ch);
        GET_CHARGE(ch) = 0;
        dmg += (ch->getBasePL()) * 0.6;
        dmg += (ch->getBaseST());
        dmg *= 1.5;
        ch->decCurHealthPercent(1, 1);
        GET_SUPPRESS(ch) = 0;
        act("@RYou EXPLODE! The explosion expands outward burning up all surroundings for a large distance. The explosion takes on the shape of a large energy dome with you at its center!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@R$n EXPLODES! The explosion expands outward burning up all surroundings for a large distance. The explosion takes on the shape of a large energy dome with $n at its center!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        for (tch = ch->getRoom()->people; tch; tch = next_v) {
            next_v = tch->next_in_room;
            if (tch == ch) {
                continue;
            }
            if (!can_kill(ch, tch, nullptr, 3)) {
                continue;
            }
            if (MOB_FLAGGED(tch, MOB_NOKILL)) {
                continue;
            } else {
                act("@r$N@R is caught in the explosion!@n", true, ch, nullptr, tch, TO_CHAR);
                act("@RYou are caught in the explosion!@n", true, ch, nullptr, tch, TO_VICT);
                act("@r$N@R is caught in the explosion!@n", true, ch, nullptr, tch, TO_NOTVICT);
                hurt(0, 0, ch, tch, nullptr, dmg, 1);
            }
        }
        if (PLR_FLAGGED(ch, PLR_IMMORTAL)) {
            GET_SDCOOLDOWN(ch) = 600;
        }
        die(ch, nullptr);
        ch->playerFlags.reset(PLR_SELFD);
        ch->playerFlags.reset(PLR_SELFD2);
        int num = rand_number(10, 20) + GET_SKILL(ch, SKILL_SELFD);
        if (GET_SKILL(ch, SKILL_SELFD) + num <= 100) {
            SET_SKILL(ch, SKILL_SELFD, num);
        } else {
            SET_SKILL(ch, SKILL_SELFD, 100);
        }
        return;
    }
}

ACMD(do_razor) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    double attperc = .14, minimum = .05;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_WRAZOR)) {
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

    skill = init_skill(ch, SKILL_WRAZOR); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 6);
    /* There is a player/mob targeted */
    if (vict) {
        if (IS_ANDROID(vict)) {
            send_to_char(ch, "There is not a necessary amount of water in cybernetic creatures.\r\n");
            return;
        }
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_WRAZOR, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Water Razor")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Water Razor, letting it fly harmlessly by!@n", false, ch, nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Water Razor, letting it fly harmlessly by!@n", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Water Razor, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Water Razor misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Water Razor at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Water Razor at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Water Razor misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Water Razor at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Water Razor at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            int64_t reduction = GET_HIT(vict);
            dmg = damtype(ch, 47, skill, attperc);
            if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
                if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 100) {
                    GET_BARRIER(ch) += dmg * 0.1;
                } else if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 60) {
                    GET_BARRIER(ch) += dmg * 0.05;
                } else if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 40) {
                    GET_BARRIER(ch) += dmg * 0.02;
                }
                if (GET_BARRIER(ch) > GET_MAX_MANA(ch)) {
                    GET_BARRIER(ch) = GET_MAX_MANA(ch);
                }
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou raise your hand toward @c$N@W with the palm open as if you are ready to grab the air between the two of you. You then focus your ki into $S body and close your hand in an instant! The @Bwater@W in $S body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand toward YOU with the palm open as if $e is ready to grab the air between the two of you. $e then focus $s ki into YOUR body and close $s hand in an instant! The @Bwater@W in your body instantly takes the shape of millions of microscopic @Dblades@W that cut up YOUR insides!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand toward @c$N@W with the palm open as if $e is ready to grab the air between the two of them. $e then focus $s ki into @c$N's@W body and close $s hand in an instant! The @Bwater@W in @c$N's@W body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    break;
                case 2: /* Critical */
                    act("@WYou raise your hand toward @c$N@W with the palm open as if you are ready to grab the air between the two of you. You then focus your ki into $S body and close your hand in an instant! The @Bwater@W in $S body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides! Blood sprays out in a mist from every pore of $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand toward YOU with the palm open as if $e is ready to grab the air between the two of you. $e then focus $s ki into YOUR body and close $s hand in an instant! The @Bwater@W in your body instantly takes the shape of millions of microscopic @Dblades@W that cut up YOUR insides! Blood sprays out of your pores into a fine mist!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand toward @c$N@W with the palm open as if $e is ready to grab the air between the two of them. $e then focus $s ki into @c$N's@W body and close $s hand in an instant! The @Bwater@W in @c$N's@W body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides! Blood sprays out of $S pores into a fine mist!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    break;
                case 3:
                    act("@WYou raise your hand toward @c$N@W with the palm open as if you are ready to grab the air between the two of you. You then focus your ki into $S body and close your hand in an instant! The @Bwater@W in $S body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides! Blood sprays out in a mist from every pore of $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand toward YOU with the palm open as if $e is ready to grab the air between the two of you. $e then focus $s ki into YOUR body and close $s hand in an instant! The @Bwater@W in your body instantly takes the shape of millions of microscopic @Dblades@W that cut up YOUR insides! Blood sprays out of your pores into a fine mist!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand toward @c$N@W with the palm open as if $e is ready to grab the air between the two of them. $e then focus $s ki into @c$N's@W body and close $s hand in an instant! The @Bwater@W in @c$N's@W body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides! Blood sprays out of $S pores into a fine mist!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    break;
                case 4: /* Weak */
                case 5: /* Weak 2 */
                    act("@WYou raise your hand toward @c$N@W with the palm open as if you are ready to grab the air between the two of you. You then focus your ki into $S body and close your hand in an instant! The @Bwater@W in $S body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s hand toward YOU with the palm open as if $e is ready to grab the air between the two of you. $e then focus $s ki into YOUR body and close $s hand in an instant! The @Bwater@W in your body instantly takes the shape of millions of microscopic @Dblades@W that cut up YOUR insides!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s hand toward @c$N@W with the palm open as if $e is ready to grab the air between the two of them. $e then focus $s ki into @c$N's@W body and close $s hand in an instant! The @Bwater@W in @c$N's@W body instantly takes the shape of millions of microscopic @Dblades@W that cut up $S insides!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    break;
            }
            pcost(ch, attperc, 0);
            if (vict) {
                reduction = reduction - GET_HIT(vict);

                if ((!IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_SPIRIT)) || (IS_NPC(vict) && GET_HIT(vict) > 0)) {
                    vict->decCurKI(reduction);
                    vict->decCurST(reduction);
                }
            }
            return;
        }
    } else if (obj) {
        send_to_char(ch, "You can not hurt an inanimate object with this technique.\r\n");
        return;
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_spike) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int64_t dmg;
    double attperc = .14, minimum = .05;

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_WSPIKE)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 1) {
        attperc += 0.05;
    } else if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
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

    skill = init_skill(ch, SKILL_WSPIKE); /* Set skill value */

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
        improve_skill(ch, SKILL_WSPIKE, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 2) {
            prob += 5;
        }
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "attack")) {
            if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
                ch->incCurKI((ch->getMaxKI() * attperc) * .3);
            } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
                ch->incCurKI((ch->getMaxKI() * attperc) * .1);
            } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
                ch->incCurKI((ch->getMaxKI() * attperc) * .05);
            }
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                WAIT_STATE(ch, PULSE_3SEC);
            }
            return;
        }


        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > rand_number(1, 130)) {
                    act("@C$N@W moves quickly and blocks your water spikes!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W water spikes!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W water spikes!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .3);
                    } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .1);
                    } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .05);
                    }
                    dmg = damtype(ch, 10, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your water spikes, letting them slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W water spikes, letting them slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W water spikes, letting them slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 43, skill, SKILL_WSPIKE); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .3);
                    } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .1);
                    } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .05);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your water spikes miss, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fire water spikes at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fire water spikes at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3 && attperc > minimum) {
                        pcost(ch, attperc - 0.05, 0);
                    } else {
                        pcost(ch, attperc, 0);
                    }
                    if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .3);
                    } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .1);
                    } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
                        ch->incCurKI((ch->getMaxKI() * attperc) * .05);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your water spikes miss, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires water spikes at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires water spikes at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict, TO_NOTVICT);
                if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3 && attperc > minimum) {
                    pcost(ch, attperc - 0.05, 0);
                } else {
                    pcost(ch, attperc, 0);
                }
                if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
                    ch->incCurKI((ch->getMaxKI() * attperc) * .3);
                } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
                    ch->incCurKI((ch->getMaxKI() * attperc) * .1);
                } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
                    ch->incCurKI((ch->getMaxKI() * attperc) * .05);
                }
            }
            if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                WAIT_STATE(ch, PULSE_3SEC);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 43, skill, attperc);
            if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
                if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 100) {
                    GET_BARRIER(ch) += dmg * 0.1;
                } else if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 60) {
                    GET_BARRIER(ch) += dmg * 0.05;
                } else if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 40) {
                    GET_BARRIER(ch) += dmg * 0.02;
                }
                if (GET_BARRIER(ch) > GET_MAX_MANA(ch)) {
                    GET_BARRIER(ch) = GET_MAX_MANA(ch);
                }
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@CYou slam both your hands together in front of you, palms flat. As you pull them apart ki flows from your palms and forms into a giant ball of water. You raise your hand above your head with the ball of water. You command the water to form several sharp spikes which freeze upon forming. The spikes then launch at @R$N@C and slam into $S chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @RYOU@C and slam into YOUR chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @R$N@C and slam into $s chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 2: /* Critical */
                    act("@CYou slam both your hands together in front of you, palms flat. As you pull them apart ki flows from your palms and forms into a giant ball of water. You raise your hand above your head with the ball of water. You command the water to form several sharp spikes which freeze upon forming. The spikes then launch at @R$N@C and slam into $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @RYOU@C and slam into YOUR head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @R$N@C and slam into $s head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    if (!AFF_FLAGGED(vict, AFF_KNOCKED) &&
                        (rand_number(1, 3) >= 2 && (GET_HIT(vict) > GET_HIT(ch) / 5) &&
                         !AFF_FLAGGED(vict, AFF_SANCTUARY))) {
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@WYou are knocked out!@n", true, ch, nullptr, vict, TO_VICT);
                        act("@C$N@W is knocked out!@n", true, ch, nullptr, vict, TO_NOTVICT);
                        vict->setStatusKnockedOut();
                    }
                    break;
                case 3:
                    act("@CYou slam both your hands together in front of you, palms flat. As you pull them apart ki flows from your palms and forms into a giant ball of water. You raise your hand above your head with the ball of water. You command the water to form several sharp spikes which freeze upon forming. The spikes then launch at @R$N@C and slam into $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @RYOU@C and slam into YOUR body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @R$N@C and slam into $s body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 4: /* Weak */
                    act("@CYou slam both your hands together in front of you, palms flat. As you pull them apart ki flows from your palms and forms into a giant ball of water. You raise your hand above your head with the ball of water. You command the water to form several sharp spikes which freeze upon forming. The spikes then launch at @R$N@C and slam into $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @RYOU@C and slam into YOUR arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @R$N@C and slam into $s arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dam_eq_loc(vict, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    break;
                case 5: /* Weak 2 */
                    act("@CYou slam both your hands together in front of you, palms flat. As you pull them apart ki flows from your palms and forms into a giant ball of water. You raise your hand above your head with the ball of water. You command the water to form several sharp spikes which freeze upon forming. The spikes then launch at @R$N@C and slam into $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @RYOU@C and slam into YOUR leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C slams both $s hands together in front of $mself, palms flat. As $e pulls them apart ki flows from the palms and forms into a giant ball of water. Then $e raises $s hand above $s head with the ball of water. The water forms several sharp spikes which freeze instantly as they take shape. The spikes then launch at @R$N@C and slam into $s leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dam_eq_loc(vict, 2);
                    if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
                        WAIT_STATE(ch, PULSE_3SEC);
                    }
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    break;
            }
            if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3 && attperc > minimum) {
                pcost(ch, attperc - 0.05, 0);
            } else {
                pcost(ch, attperc, 0);
            }
            if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
                ch->incCurKI((ch->getMaxKI() * attperc) * .3);
            } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
                ch->incCurKI((ch->getMaxKI() * attperc) * .1);
            } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
                ch->incCurKI((ch->getMaxKI() * attperc) * .05);
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
        dmg = damtype(ch, 43, skill, attperc);
        dmg /= 10;
        act("@WYou fire water spikes at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires water spikes at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        if (GET_SKILL_PERF(ch, SKILL_WSPIKE) == 3) {
            WAIT_STATE(ch, PULSE_3SEC);
        }
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
        if (GET_SKILL(ch, SKILL_WSPIKE) >= 100) {
            ch->incCurKI((ch->getMaxKI() * attperc) * .3);
        } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 60) {
            ch->incCurKI((ch->getMaxKI() * attperc) * .1);
        } else if (GET_SKILL(ch, SKILL_WSPIKE) >= 40) {
            ch->incCurKI((ch->getMaxKI() * attperc) * .05);
        }
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_koteiru) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    double attperc = .3, minimum = .1;

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_KOTEIRU)) {
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

    skill = init_skill(ch, SKILL_KOTEIRU); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 7);
    /* There is a player/mob targeted */
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_KOTEIRU, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Koteiru Bakuha")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Koteiru Bakuha, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Koteiru Bakuha, letting it fly harmlessly by!@n", false, ch, nullptr,
                        vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Koteiru Bakuha, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Koteiru Bakuha misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Koteiru Bakuha at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Koteiru Bakuha at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Koteiru Bakuha misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Koteiru Bakuha at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Koteiru Bakuha at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 48, skill, attperc);
            if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
                if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 100) {
                    GET_BARRIER(ch) += dmg * 0.1;
                } else if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 60) {
                    GET_BARRIER(ch) += dmg * 0.05;
                } else if (GET_SKILL(ch, SKILL_AQUA_BARRIER) >= 40) {
                    GET_BARRIER(ch) += dmg * 0.02;
                }
                if (GET_BARRIER(ch) > GET_MAX_MANA(ch)) {
                    GET_BARRIER(ch) = GET_MAX_MANA(ch);
                }
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@CYou hold your hands outstretched in front of your body and with your ki begin to create turbulant waters that hover around your body. You begin a sort of dance as you control the water more and more until you have created a huge floating wave. In an instant you swing the wave at @W$N@C! As the wave slams into $S chest it freezes solid around $M!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at YOU! As the wave slams into YOUR chest it freezes solid around YOU!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at @W$N@C! As the wave slams into $S chest it freezes solid around @W$N@C!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 2: /* Critical */
                    act("@CYou hold your hands outstretched in front of your body and with your ki begin to create turbulant waters that hover around your body. You begin a sort of dance as you control the water more and more until you have created a huge floating wave. In an instant you swing the wave at @W$N@C! As the wave slams into $S head it freezes solid around $M!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at YOU! As the wave slams into YOUR head it freezes solid around YOU!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at @W$N@C! As the wave slams into $S head it freezes solid around @W$N@C!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    break;
                case 3:
                    act("@CYou hold your hands outstretched in front of your body and with your ki begin to create turbulant waters that hover around your body. You begin a sort of dance as you control the water more and more until you have created a huge floating wave. In an instant you swing the wave at @W$N@C! As the wave slams into $S gut it freezes solid around $M!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at YOU! As the wave slams into YOUR gut it freezes solid around YOU!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at @W$N@C! As the wave slams into $S gut it freezes solid around @W$N@C!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 4: /* Weak */
                    act("@CYou hold your hands outstretched in front of your body and with your ki begin to create turbulant waters that hover around your body. You begin a sort of dance as you control the water more and more until you have created a huge floating wave. In an instant you swing the wave at @W$N@C! As the wave slams into $S arm it freezes solid around $M!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at YOU! As the wave slams into YOUR arm it freezes solid around YOU!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at @W$N@C! As the wave slams into $S arm it freezes solid around @W$N@C!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    dam_eq_loc(vict, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    break;
                case 5: /* Weak 2 */
                    act("@CYou hold your hands outstretched in front of your body and with your ki begin to create turbulant waters that hover around your body. You begin a sort of dance as you control the water more and more until you have created a huge floating wave. In an instant you swing the wave at @W$N@C! As the wave slams into $S leg it freezes solid around $M!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at YOU! As the wave slams into YOUR leg it freezes solid around YOU!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n@C holds $s hands outstretched in front of $s body and with $s ki begins to create turbulant waters that hover around $s body. @c$n@C begins a sort of dance as $e controls the water more and more until $e has created a huge floating wave. In an instant $e swings the wave at @W$N@C! As the wave slams into $S leg it freezes solid around @W$N@C!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    dam_eq_loc(vict, 2);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    break;
            }
            pcost(ch, attperc, 0);
            if (vict) {
                if (GET_HIT(vict) > 1) {
                    if (rand_number(1, 4) == 1 && !AFF_FLAGGED(vict, AFF_FROZEN) && !IS_DEMON(vict)) {
                        act("@CYour body completely freezes!@n", true, vict, nullptr, nullptr, TO_CHAR);
                        act("@c$n's@C body completely freezes!@n", true, vict, nullptr, nullptr, TO_ROOM);
                        vict->affected_by.set(AFF_FROZEN);
                    }
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
        dmg = damtype(ch, 48, skill, attperc);
        dmg /= 10;
        act("@WYou fire Koteiru Bakuha at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires Koteiru Bakuha at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_hspiral) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    double attperc = .3, minimum = .15;

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_HSPIRAL)) {
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

    skill = init_skill(ch, SKILL_HSPIRAL); /* Set skill value */

    if (!tech_handle_targeting(ch, arg, &vict, &obj)) return;

    handle_cooldown(ch, 6);
    /* There is a player/mob targeted */
    if (vict) {
        if (!can_kill(ch, vict, nullptr, 1)) {
            return;
        }
        if (handle_defender(vict, ch)) {
            struct char_data *def = GET_DEFENDER(vict);
            vict = def;
        }
        improve_skill(ch, SKILL_HSPIRAL, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Hell Spiral")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Hell Spiral, letting it fly harmlessly by!@n", false, ch, nullptr,
                        vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Hell Spiral, letting it fly harmlessly by!@n", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Hell Spiral, letting it fly harmlessly by!@n", false, ch,
                        nullptr, vict, TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Hell Spiral misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Hell Spiral at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Hell Spiral at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Hell Spiral misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Hell Spiral at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Hell Spiral at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 49, skill, attperc);
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou hold out your hand, palm upward, while looking toward @C$N@W and grinning as @Rred@W energy begins to pool in the center of your palm. As the large orb of energy swells to about three feet in diameter you move your hand away! You quickly punch the ball of energy and send it flying into @C$N's@W chest where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward YOU and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into YOUR chest where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward @c$N@W and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into @c$N's@W chest where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 2: /* Critical */
                    act("@WYou hold out your hand, palm upward, while looking toward @C$N@W and grinning as @Rred@W energy begins to pool in the center of your palm. As the large orb of energy swells to about three feet in diameter you move your hand away! You quickly punch the ball of energy and send it flying into @C$N's@W head where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward YOU and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into YOUR head where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward @c$N@W and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into @c$N's@W head where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    break;
                case 3:
                    act("@WYou hold out your hand, palm upward, while looking toward @C$N@W and grinning as @Rred@W energy begins to pool in the center of your palm. As the large orb of energy swells to about three feet in diameter you move your hand away! You quickly punch the ball of energy and send it flying into @C$N's@W body where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward YOU and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into YOUR body where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward @c$N@W and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into @c$N's@W body where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 4: /* Weak */
                    act("@WYou hold out your hand, palm upward, while looking toward @C$N@W and grinning as @Rred@W energy begins to pool in the center of your palm. As the large orb of energy swells to about three feet in diameter you move your hand away! You quickly punch the ball of energy and send it flying into @C$N's@W arm where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward YOU and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into YOUR arm where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward @c$N@W and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into @c$N's@W arm where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    dam_eq_loc(vict, 1);
                    hurt(0, 120, ch, vict, nullptr, dmg, 1);
                    break;
                case 5: /* Weak 2 */
                    act("@WYou hold out your hand, palm upward, while looking toward @C$N@W and grinning as @Rred@W energy begins to pool in the center of your palm. As the large orb of energy swells to about three feet in diameter you move your hand away! You quickly punch the ball of energy and send it flying into @C$N's@W leg where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward YOU and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into YOUR leg where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n @Wholds out $s hand, palm upward, while looking toward @c$N@W and grinning as @Rred@W energy begins to pool in the center of $s palm. As the large orb of energy swells to about three feet in diameter $e moves $s hand away! Then $e quickly punches the ball of energy and sends it flying into @c$N's@W leg where it @re@Rx@Dp@rl@Ro@Dd@re@Rs@W in a flash of light!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    dam_eq_loc(vict, 2);
                    hurt(1, 120, ch, vict, nullptr, dmg, 1);
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
        dmg = damtype(ch, 49, skill, attperc);
        dmg /= 10;
        act("@WYou fire Hell Spiral at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires Hell Spiral at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}

ACMD(do_spiral) {
    int skill;
    struct char_data *vict;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_SPIRAL)) {
        return;
    }

    if (!limb_ok(ch, 0)) {
        return;
    }

    if (!*arg && !FIGHTING(ch)) {
        send_to_char(ch, "Direct it at who?\r\n");
        return;
    }

    if (!check_points(ch, GET_MAX_MANA(ch) * .5, 0)) {
        return;
    }

    /* Passed sanity checks for doing the technique */

    skill = init_skill(ch, SKILL_SPIRAL); /* Set skill value */

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char(ch, "Nothing around here by that name.");
            return;
        }
    }

    /* There is a player/mob targeted */
    if (!can_kill(ch, vict, nullptr, 3)) {
        return;
    }

    if (handle_defender(vict, ch)) {
        struct char_data *def = GET_DEFENDER(vict);
        vict = def;
    }

    ch->playerFlags.set(PLR_SPIRAL);
    improve_skill(ch, SKILL_SPIRAL, 0);
    act("@mFlying to a spot above your intended target you begin to move so fast all that can be seen of you are trails of color. You focus your movements into a vortex and prepare to attack!@n",
        true, ch, nullptr, nullptr, TO_CHAR);
    act("@w$n@m flies to a spot above and begins to move so fast all that can be seen of $m are trails of color. Suddenly $e focuses $s movements into a spinning vortex and you lose track of $s movements entirely!@n",
        true, ch, nullptr, nullptr, TO_ROOM);
    handle_spiral(ch, vict, skill, true);
    handle_cooldown(ch, 8);
}

ACMD(do_breaker) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2, skill;
    int64_t dmg;
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    double attperc = .14, minimum = .05;

    two_arguments(argument, arg, arg2);

    /* Can they do the technique? */

    if (!can_grav(ch)) {
        return;
    }

    if (!check_skill(ch, SKILL_BREAKER)) {
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

    skill = init_skill(ch, SKILL_BREAKER); /* Set skill value */

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
        improve_skill(ch, SKILL_BREAKER, 0);

        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = roll_accuracy(ch, skill, true);
        perc = chance_to_hit(ch);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        prob -= avo;
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Star Breaker")) {
            pcost(ch, attperc, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc - 20) {
            if ((vict->getCurST()) > 0) {
                if (blk > rand_number(1, 130)) {
                    act("@C$N@W moves quickly and blocks your Star Breaker!@n", false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Star Breaker!@n", false, ch, nullptr, vict, TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Star Breaker!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    dmg = damtype(ch, 10, skill, attperc);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Star Breaker, letting it slam into the surroundings!@n", false,
                        ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Star Breaker, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Star Breaker, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 46, skill, SKILL_BREAKER); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);

                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Star Breaker misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Star Breaker at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Star Breaker at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, attperc, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Star Breaker misses, flying through the air harmlessly!@n", false,
                    ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Star Breaker at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Star Breaker at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, attperc, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            dmg = damtype(ch, 46, skill, attperc);
            int64_t theft = 0;
            int64_t taken = 0;
            if (GET_LEVEL(ch) - 30 > GET_LEVEL(vict)) {
                theft = 1;
            } else if (GET_LEVEL(ch) - 20 > GET_LEVEL(vict)) {
                theft = GET_EXP(vict) / 1000;
            } else if (GET_LEVEL(ch) - 10 > GET_LEVEL(vict)) {
                theft = GET_EXP(vict) / 100;
            } else if (GET_LEVEL(ch) >= GET_LEVEL(vict)) {
                theft = GET_EXP(vict) / 50;
            } else if (GET_LEVEL(ch) + 10 >= GET_LEVEL(vict)) {
                theft = GET_EXP(vict) / 500;
            } else if (GET_LEVEL(ch) + 20 >= GET_LEVEL(vict)) {
                theft = GET_EXP(vict) / 1000;
            } else {
                theft = GET_EXP(vict) / 2000;
            }
            if(theft > 0) {
                taken = vict->modExperience(-theft);
            }
            int hitspot = 1;
            hitspot = roll_hitloc(ch, vict, skill);
            switch (hitspot) {
                case 1:
                    act("@WYou raise your right hand above your head with it slightly cupped. Dark @rred@W energy from the Eldritch Star begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by your ki flows up your left arm as you raise it. You slam both of your hands together, combining the energy, and then toss your @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S chest!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then toss a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at YOU! It engulfs your chest!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then tosses a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S chest!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 2: /* Critical */
                    act("@WYou raise your right hand above your head with it slightly cupped. Dark @rred@W energy from the Eldritch Star begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by your ki flows up your left arm as you raise it. You slam both of your hands together, combining the energy, and then toss your @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S head!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then toss a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at YOU! It engulfs your head!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then tosses a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S head!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 0);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    break;
                case 3:
                    act("@WYou raise your right hand above your head with it slightly cupped. Dark @rred@W energy from the Eldritch Star begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by your ki flows up your left arm as you raise it. You slam both of your hands together, combining the energy, and then toss your @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S body!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then toss a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at YOU! It engulfs your body!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then tosses a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S body!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_BONUS(ch, BONUS_SOFT)) {
                        dmg *= calc_critical(ch, 2);
                    }
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 4: /* Weak */
                    act("@WYou raise your right hand above your head with it slightly cupped. Dark @rred@W energy from the Eldritch Star begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by your ki flows up your left arm as you raise it. You slam both of your hands together, combining the energy, and then toss your @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S arm!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then toss a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at YOU! It engulfs your arm!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then tosses a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S arm!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    dam_eq_loc(vict, 1);
                    hurt(0, 180, ch, vict, nullptr, dmg, 1);
                    break;
                case 5: /* Weak 2 */
                    act("@WYou raise your right hand above your head with it slightly cupped. Dark @rred@W energy from the Eldritch Star begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by your ki flows up your left arm as you raise it. You slam both of your hands together, combining the energy, and then toss your @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S leg!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then toss a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at YOU! It engulfs your leg!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W raises $s right hand above $s head with it slightly cupped. Dark @rred@W energy begins to pool there and form a growing orb of energy. At the same time @mpurple@W arcs of electricity formed by $s ki flows up $s left arm as $e raises it. Suddenly $e slams both of $s hands together, combining the energy, and then tosses a @YS@yta@Yr @rB@Rr@De@ra@Rk@De@rr@W at @c$N@W! It engulfs $S leg!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= calc_critical(ch, 1);
                    dam_eq_loc(vict, 2);
                    hurt(1, 180, ch, vict, nullptr, dmg, 1);
                    break;
            }

            if (level_exp(ch, GET_LEVEL(ch) + 1) - (GET_EXP(ch)) > 0 || GET_LEVEL(ch) >= 100) {
                auto xp = ch->modExperience(theft * 2, false); // we will not apply bonuses to stolen exp.
                send_to_char(ch, "The returning Eldritch energy blesses you with some experience. @D[@G%s@D]@n\r\n",
                             add_commas(xp).c_str());

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
        dmg = damtype(ch, 46, skill, attperc);
        dmg /= 10;
        act("@WYou fire Star Breaker at $p@W!@n", false, ch, obj, nullptr, TO_CHAR);
        act("@C$n@W fires Star Breaker at $p@W!@n", false, ch, obj, nullptr, TO_ROOM);
        hurt(0, 0, ch, nullptr, obj, dmg, 0);
        pcost(ch, attperc, 0);
    } else {
        send_to_char(ch, "Error! Please report.\r\n");
        return;
    }
}
