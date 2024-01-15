/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/local_limits.h"
#include "dbat/utils.h"
#include "dbat/spells.h"
#include "dbat/comm.h"
#include "dbat/dg_comm.h"
#include "dbat/act.other.h"
#include "dbat/act.item.h"
#include "dbat/vehicles.h"
#include "dbat/act.movement.h"
#include "dbat/constants.h"
#include "dbat/class.h"
#include "dbat/fight.h"
#include "dbat/objsave.h"
#include "dbat/handler.h"
#include "dbat/dg_scripts.h"

/* local defines */
#define sick_fail       2

/* local functions */
static void heal_limb(struct char_data *ch);

static int64_t move_gain(struct char_data *ch);

static int64_t mana_gain(struct char_data *ch);

static int64_t hit_gain(struct char_data *ch);

static void update_flags(struct char_data *ch);

static int wearing_stardust(struct char_data *ch);

static void healthy_check(struct char_data *ch);

static void barrier_shed(struct char_data *ch);

static void check_idling(struct char_data *ch);

static void barrier_shed(struct char_data *ch) {

    if (!AFF_FLAGGED(ch, AFF_SANCTUARY)) {
        return;
    }

    if (GET_SKILL(ch, SKILL_AQUA_BARRIER) > 0) {
        return;
    }

    int chance = axion_dice(0), barrier = GET_SKILL(ch, SKILL_BARRIER), concentrate = GET_SKILL(ch,
                                                                                                SKILL_CONCENTRATION);
    double rate = 0.3;

    if (barrier >= 100) {
        rate = 0.01;
    } else if (barrier >= 95) {
        rate = 0.02;
    } else if (barrier >= 90) {
        rate = 0.04;
    } else if (barrier >= 80) {
        rate = 0.08;
    } else if (barrier >= 70) {
        rate = 0.10;
    } else if (barrier >= 60) {
        rate = 0.15;
    } else if (barrier >= 50) {
        rate = 0.20;
    } else if (barrier >= 40) {
        rate = 0.25;
    } else if (barrier >= 30) {
        rate = 0.27;
    } else if (barrier >= 20) {
        rate = 0.29;
    }

    int64_t loss = (long double) (GET_BARRIER(ch)) * rate, recharge = 0;

    if (concentrate >= chance) {
        recharge = loss * 0.5;
    }

    GET_BARRIER(ch) -= loss;

    if (GET_BARRIER(ch) <= 0) {
        GET_BARRIER(ch) = 0;
        act("@cYour barrier disappears.@n", true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@c's barrier disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
    } else {
        act("@cYour barrier loses some energy.@n", true, ch, nullptr, nullptr, TO_CHAR);
        send_to_char(ch, "@D[@C%s@D]@n\r\n", add_commas(loss).c_str());
        act("@c$n@c's barrier sends some sparks into the air as it seems to get a bit weaker.@n", true, ch, nullptr,
            nullptr, TO_ROOM);
    }

    if (recharge > 0 && (ch->getCurKI()) < GET_MAX_MANA(ch)) {
        ch->incCurKI(recharge);
        send_to_char(ch, "@CYou reabsorb some of the energy lost into your body!@n\r\n");
    }
}

/* If they have the Healthy trait then they have a chance to lose each of these */
static void healthy_check(struct char_data *ch) {

    if (!GET_BONUS(ch, BONUS_HEALTHY) || GET_POS(ch) != POS_SLEEPING) {
        return;
    }

    int chance = 70, roll = rand_number(1, 100), change = false;

    if (AFF_FLAGGED(ch, AFF_SHOCKED) && roll >= chance) {
        ch->affected_by.reset(AFF_SHOCKED);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_MBREAK) && roll >= chance) {
        ch->affected_by.reset(AFF_MBREAK);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_WITHER) && roll >= chance) {
        null_affect(ch, AFF_WITHER);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_CURSE) && roll >= chance) {
        ch->affected_by.reset(AFF_CURSE);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_POISON) && roll >= chance) {
        null_affect(ch, AFF_POISON);
        change = true;
    }
    if (IS_AFFECTED(ch, AFF_PARALYZE) && roll >= chance) {
        null_affect(ch, AFF_PARALYZE);
        change = true;
    }
    if (IS_AFFECTED(ch, AFF_PARA) && roll >= chance) {
        null_affect(ch, AFF_PARA);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_BLIND) && roll >= chance) {
        null_affect(ch, AFF_BLIND);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_HYDROZAP) && roll >= chance) {
        null_affect(ch, AFF_HYDROZAP);
        change = true;
    }
    if (AFF_FLAGGED(ch, AFF_KNOCKED) && roll >= chance) {
        ch->affected_by.reset(AFF_KNOCKED);
        GET_POS(ch) = POS_SITTING;
        change = true;
    }
    if (change == true) {
        send_to_char(ch, "@CYou feel your body recover from all its ailments!@n\r\n");
    }
    return;
}

static int wearing_stardust(struct char_data *ch) {

    int count = 0, i;

    for (i = 1; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i)) {
            struct obj_data *obj = GET_EQ(ch, i);
            switch (GET_OBJ_VNUM(obj)) {
                case 1110:
                case 1111:
                case 1112:
                case 1113:
                case 1114:
                case 1115:
                case 1116:
                case 1117:
                case 1118:
                case 1119:
                    count += 1;
                    break;
            }
        }
    }

    if (count == 26)
        return (1);
    else
        return (0);

}

/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
static int64_t mana_gain(struct char_data *ch) {
    int64_t gain = 0;

    if (IS_NPC(ch)) {
        /* Neat and fast */
        gain = GET_MAX_MANA(ch) / 70;
    } else {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_REGEN) ||
            (GET_BONUS(ch, BONUS_DESTROYER) > 0 && ROOM_DAMAGE(IN_ROOM(ch)) >= 75)) {
            if (IS_KONATSU(ch)) {
                gain = GET_MAX_MANA(ch) / 12;
            }
            if (IS_MUTANT(ch)) {
                gain = GET_MAX_MANA(ch) / 11;
            }
            if (IS_ARLIAN(ch)) {
                gain = GET_MAX_MANA(ch) / 30;
            }
            if (!IS_KONATSU(ch) && !IS_MUTANT(ch)) {
                gain = GET_MAX_MANA(ch) / 10;
            }
        } else if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_REGEN)) {
            if (IS_KONATSU(ch)) {
                gain = GET_MAX_MANA(ch) / 15;
            }
            if (IS_MUTANT(ch)) {
                gain = GET_MAX_MANA(ch) / 13;
            }
            if (!IS_KONATSU(ch) && !IS_MUTANT(ch)) {
                gain = GET_MAX_MANA(ch) / 12;
            }
            if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_BEDROOM)) {
                gain += gain * 0.25;
            }
            if (IS_ARLIAN(ch)) {
                gain = GET_MAX_MANA(ch) / 40;
            }
        }
        /* Position calculations    */
        switch (GET_POS(ch)) {
            case POS_STANDING:
                if (!IS_HOSHIJIN(ch) || (IS_HOSHIJIN(ch) && GET_PHASE(ch) <= 0)) {
                    gain = gain / 4;
                } else {
                    gain += (gain / 2);
                }
                break;
            case POS_FIGHTING:
                gain = gain / 4;
                break;
            case POS_SLEEPING:
                if (!SITS(ch)) {
                    gain *= 2;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090) {
                    gain *= 3;
                    gain += gain * 0.1;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19092) {
                    gain *= 3;
                    gain += gain * 0.3;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain *= 3;
                }
                break;
            case POS_RESTING:
                if (!SITS(ch)) {
                    gain += (gain / 2);
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090 && !IS_ARLIAN(ch)) {
                    gain *= 2;
                    gain += gain * 0.1;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19092 && !IS_ARLIAN(ch)) {
                    gain *= 2;
                    gain += gain * 0.3;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain *= 2;
                }
                break;
            case POS_SITTING:
                if (!SITS(ch)) {
                    gain += (gain / 4);
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090) {
                    gain += gain * 0.6;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19092) {
                    gain += gain * 0.8;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain += gain * 0.5;
                }
                break;
        }
    }

    if (IN_ROOM(ch) != NOWHERE) {
        if (cook_element(IN_ROOM(ch)) == 1) {
            gain += (gain * 0.2);
        }
    }

    if (IS_ARLIAN(ch) && IS_FEMALE(ch) && OUTSIDE(ch)) {
        gain *= 4;
    }

    if (IS_KANASSAN(ch) && weather_info.sky == SKY_RAINING && OUTSIDE(ch)) {
        gain += gain * 0.1;
    }
    if (IS_KANASSAN(ch) && SUNKEN(IN_ROOM(ch))) {
        gain *= 16;
    }

    if (IS_HOSHIJIN(ch) && GET_PHASE(ch) > 0) {
        gain *= 2;
    }

    if (PLR_FLAGGED(ch, PLR_HEALT) && SITS(ch) != nullptr) {
        gain *= 20;
    }
    if (AFF_FLAGGED(ch, AFF_POSE) && axion_dice(0) > GET_SKILL(ch, SKILL_POSE)) {
        null_affect(ch, AFF_POSE);
        send_to_char(ch, "You feel slightly less confident now.\r\n");
    }
    if (AFF_FLAGGED(ch, AFF_HYDROZAP) && rand_number(1, 4) >= 4) {
        null_affect(ch, AFF_HYDROZAP);
    }

    if (GET_SKILL(ch, SKILL_CONCENTRATION) >= 100) {
        gain += gain / 2;
    } else if (GET_SKILL(ch, SKILL_CONCENTRATION) >= 75) {
        gain += gain / 4;
    } else if (GET_SKILL(ch, SKILL_CONCENTRATION) >= 50) {
        gain += gain / 6;
    } else if (GET_SKILL(ch, SKILL_CONCENTRATION) >= 25) {
        gain += gain / 8;
    } else if (GET_SKILL(ch, SKILL_CONCENTRATION) < 25 && GET_SKILL(ch, SKILL_CONCENTRATION) > 0) {
        gain += gain / 10;
    }

    if (AFF_FLAGGED(ch, AFF_BLESS)) {
        gain *= 2;
    }
    if (AFF_FLAGGED(ch, AFF_CURSE)) {
        gain /= 5;
    }

    if (GET_FOODR(ch) > 0 && rand_number(1, 2) == 2) {
        GET_FOODR(ch) -= 1;
    }

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_HINTS) && rand_number(1, 5) == 5) {
        hint_system(ch, 0);
    }

    if (AFF_FLAGGED(ch, AFF_POISON))
        gain /= 4;

    if (cook_element(IN_ROOM(ch)) == 1)
        gain *= 2;

    return (gain);
}

/* Hitpoint gain pr. game hour */
int64_t hit_gain(struct char_data *ch) {
    int64_t gain = 0;

    if (IS_NPC(ch)) {
        /* Neat and fast */
        gain = GET_MAX_HIT(ch) / 70;
    } else {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_REGEN) ||
            (GET_BONUS(ch, BONUS_DESTROYER) > 0 && ROOM_DAMAGE(IN_ROOM(ch)) >= 75)) {
            if (IS_HUMAN(ch)) {
                gain = GET_MAX_HIT(ch) / 20;
            }
            if (IS_ARLIAN(ch)) {
                gain = GET_MAX_HIT(ch) / 30;
            }
            if (IS_NAMEK(ch)) {
                gain = GET_MAX_HIT(ch) / 2;
            }
            if (IS_MUTANT(ch)) {
                gain = GET_MAX_HIT(ch) / 11;
            }
            if (!IS_HUMAN(ch) && !IS_NAMEK(ch) && !IS_MUTANT(ch)) {
                gain = GET_MAX_HIT(ch) / 10;
            }
        } else if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_REGEN)) {
            if (IS_HUMAN(ch)) {
                gain = GET_MAX_HIT(ch) / 30;
            }
            if (IS_NAMEK(ch)) {
                gain = GET_MAX_HIT(ch) / 4;
            }
            if (IS_MUTANT(ch)) {
                gain = GET_MAX_HIT(ch) / 16;
            }
            if (IS_ARLIAN(ch)) {
                gain = GET_MAX_HIT(ch) / 40;
            }
            if (!IS_HUMAN(ch) && !IS_NAMEK(ch) && !IS_MUTANT(ch)) {
                gain = GET_MAX_HIT(ch) / 15;
            }
            if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_BEDROOM)) {
                gain += gain * 0.25;
            }
        }

        /* Position calculations    */
        switch (GET_POS(ch)) {
            case POS_STANDING:
                if (!IS_HOSHIJIN(ch) || (IS_HOSHIJIN(ch) && GET_PHASE(ch) <= 0)) {
                    gain = gain / 4;
                } else if (IS_ANDROID(ch) && PLR_FLAGGED(ch, PLR_ABSORB)) {
                    gain = gain / 3;
                } else {
                    gain += (gain / 2);
                }
                break;
            case POS_FIGHTING:
                gain = gain / 4;
                break;
            case POS_SLEEPING:
                if (IS_ARLIAN(ch)) {
                    gain *= 3;
                } else if (!SITS(ch)) {
                    gain *= 2;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090) {
                    gain *= 3;
                    gain += gain * 0.1;
                } else if (SITS(ch)) {
                    gain *= 3;
                }
                break;
            case POS_RESTING:
                if (!SITS(ch)) {
                    gain += (gain / 2);
                } else if (IS_ANDROID(ch) && PLR_FLAGGED(ch, PLR_ABSORB)) {
                    gain = gain * 1.5;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090 && !IS_ARLIAN(ch)) {
                    gain += gain * 1.1;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain *= 2;
                }
                break;
            case POS_SITTING:
                if (!SITS(ch)) {
                    gain += (gain / 4);
                } else if (IS_ANDROID(ch) && PLR_FLAGGED(ch, PLR_ABSORB)) {
                    gain = gain * 0.5;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090 && !IS_ARLIAN(ch)) {
                    gain += gain * 0.6;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain += (gain * 0.5);
                }
        }
    }
    healthy_check(ch);

    if (IS_ARLIAN(ch) && IS_FEMALE(ch) && OUTSIDE(ch)) {
        gain *= 4;
    }

    if (IS_KANASSAN(ch) && weather_info.sky == SKY_RAINING && OUTSIDE(ch)) {
        gain += gain * 0.1;
    }
    if (IS_KANASSAN(ch) && SUNKEN(IN_ROOM(ch))) {
        gain *= 16;
    }

    if (IS_HOSHIJIN(ch) && GET_PHASE(ch) > 0) {
        gain *= 2;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT) && SITS(ch) != nullptr) {
        gain *= 20;
    }

    if (AFF_FLAGGED(ch, AFF_BLESS)) {
        gain *= 2;
    }
    if (AFF_FLAGGED(ch, AFF_CURSE)) {
        gain /= 5;
    }

    /* Fury Mode Loss for halfbreeds */

    if (PLR_FLAGGED(ch, PLR_FURY)) {
        send_to_char(ch, "Your fury subsides for now. Next time try to take advantage of it before you calm down.\r\n");
        ch->playerFlags.reset(PLR_FURY);
    }

    /* Fury Mode Loss for halfbreeds */

    if (AFF_FLAGGED(ch, AFF_POISON))
        gain /= 4;
    if (cook_element(IN_ROOM(ch)) == 1)
        gain *= 2;

    if (!IS_NPC(ch)) {
        if (PLR_FLAGGED(ch, PLR_ABSORB)) {
            gain = gain / 8;
        }
    }

    if (GET_REGEN(ch) > 0) {
        gain += (gain * 0.01) * GET_REGEN(ch);
    }

    return (gain);
}

/* move gain pr. game hour */
static int64_t move_gain(struct char_data *ch) {
    int64_t gain = 0;

    if (IS_NPC(ch)) {
        /* Neat and fast */
        gain = GET_MAX_MOVE(ch) / 70;
    } else {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_REGEN) ||
            (GET_BONUS(ch, BONUS_DESTROYER) > 0 && ROOM_DAMAGE(IN_ROOM(ch)) >= 75)) {
            if (IS_MUTANT(ch)) {
                gain = GET_MAX_MOVE(ch) / 7;
            }
            if (IS_ARLIAN(ch)) {
                gain = GET_MAX_MOVE(ch) / 4;
            }
            if (!IS_MUTANT(ch)) {
                gain = GET_MAX_MOVE(ch) / 6;
            }
        } else if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_REGEN)) {
            if (IS_MUTANT(ch)) {
                gain = GET_MAX_MOVE(ch) / 9;
            }
            if (!IS_MUTANT(ch)) {
                gain = GET_MAX_MOVE(ch) / 8;
            }
            if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_BEDROOM)) {
                gain += gain * 0.25;
            }
        }

        /* Position calculations    */
        switch (GET_POS(ch)) {
            case POS_STANDING:
                if (!IS_HOSHIJIN(ch) || (IS_HOSHIJIN(ch) && GET_PHASE(ch) <= 0)) {
                    gain = gain / 4;
                } else {
                    gain += (gain / 2);
                }
                break;
            case POS_FIGHTING:
                gain = gain / 4;
                break;
            case POS_SLEEPING:
                if (!SITS(ch)) {
                    gain *= 2;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090 && !IS_ARLIAN(ch)) {
                    gain *= 3;
                    gain += gain * 0.1;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19091 && !IS_ARLIAN(ch)) {
                    gain *= 3;
                    gain += gain * 0.3;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain *= 3;
                }
                break;
            case POS_RESTING:
                if (!SITS(ch)) {
                    gain += (gain / 2);
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090 && !IS_ARLIAN(ch)) {
                    gain += gain * 1.1;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19091 && !IS_ARLIAN(ch)) {
                    gain += gain * 1.3;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain += gain;
                }
                break;
            case POS_SITTING:
                if (!SITS(ch)) {
                    gain += (gain / 4);
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19090 && !IS_ARLIAN(ch)) {
                    gain += gain * 0.6;
                } else if (GET_OBJ_VNUM(SITS(ch)) == 19091 && !IS_ARLIAN(ch)) {
                    gain += gain * 0.8;
                } else if (SITS(ch) || IS_ARLIAN(ch)) {
                    gain += (gain / 2);
                }
        }
    }

    if (IS_ARLIAN(ch) && IS_FEMALE(ch) && OUTSIDE(ch)) {
        gain *= 2;
    }

    if (IS_NAMEK(ch)) {
        gain = gain * 0.5;
    }

    if (IS_KANASSAN(ch) && weather_info.sky == SKY_RAINING && OUTSIDE(ch)) {
        gain += gain * 0.1;
    }
    if (IS_KANASSAN(ch) && SUNKEN(IN_ROOM(ch))) {
        gain *= 16;
    }

    if (IS_HOSHIJIN(ch) && GET_PHASE(ch) > 0) {
        gain *= 2;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT) && SITS(ch) != nullptr) {
        gain *= 20;
    }

    if (AFF_FLAGGED(ch, AFF_BLESS)) {
        gain *= 2;
    }
    if (AFF_FLAGGED(ch, AFF_CURSE)) {
        gain /= 5;
    }

    if (AFF_FLAGGED(ch, AFF_POISON))
        gain /= 4;

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_AURA)) {
        gain = GET_MAX_MOVE(ch) - (ch->getCurST());
    }
    if (cook_element(IN_ROOM(ch)) == 1)
        gain *= 2;


    if (GET_REGEN(ch) > 0) {
        gain += (gain * 0.01) * GET_REGEN(ch);
    }

    return (gain);
}

static void update_flags(struct char_data *ch) {
    if (ch == nullptr) {
        send_to_imm("ERROR: Empty ch variable sent to update_flags.");
        return;
    }

    if (GET_BONUS(ch, BONUS_LATE) && GET_POS(ch) == POS_SLEEPING && rand_number(1, 3) == 3) {
        if (GET_HIT(ch) >= (ch->getEffMaxPL()) && (ch->getCurST()) >= GET_MAX_MOVE(ch) &&
            (ch->getCurKI()) >= GET_MAX_MANA(ch)) {
            send_to_char(ch, "You FINALLY wake up.\r\n");
            act("$n wakes up.", true, ch, nullptr, nullptr, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
        }
    }

    if (AFF_FLAGGED(ch, AFF_KNOCKED) && !FIGHTING(ch)) {
        ch->cureStatusKnockedOut(true);
    }

    barrier_shed(ch);

    if (AFF_FLAGGED(ch, AFF_FIRESHIELD) && !FIGHTING(ch) && rand_number(1, 101) > GET_SKILL(ch, SKILL_FIRESHIELD)) {
        send_to_char(ch, "Your fireshield disappears.\r\n");
        ch->affected_by.reset(AFF_FIRESHIELD);
    }
    if (AFF_FLAGGED(ch, AFF_ZANZOKEN) && !FIGHTING(ch) && rand_number(1, 3) == 2) {
        send_to_char(ch, "You lose concentration and no longer are ready to zanzoken.\r\n");
        ch->affected_by.reset(AFF_ZANZOKEN);
    }
    if (AFF_FLAGGED(ch, AFF_ENSNARED) && rand_number(1, 3) == 2) {
        send_to_char(ch, "The silk ensnaring your arms disolves enough for you to break it!\r\n");
        ch->affected_by.reset(AFF_ENSNARED);
    }

    if ((IS_SAIYAN(ch) || IS_HALFBREED(ch)) && (ch->form == FormID::SuperSaiyan) && !PLR_FLAGGED(ch, PLR_FPSSJ)) {
        GET_ABSORBS(ch) += 1;
        if (GET_ABSORBS(ch) >= 300) {
            send_to_char(ch,
                         "You have mastered the base Super Saiyan transformation and achieved Full Power Super Saiyan! Super Saiyan First can now be maintained effortlessly.\r\n");
            ch->playerFlags.set(PLR_FPSSJ);
            GET_ABSORBS(ch) = 0;
        }
    }

    if(race::hasTail(ch->race) && !PLR_FLAGGED(ch, PLR_TAIL) && !PLR_FLAGGED(ch, PLR_NOGROW)) {
        int growth = 0;
        if(IS_HALFBREED(ch) && RACIAL_PREF(ch) == 1)
            growth = 1 ? rand_number(1, 50) >= 40 : 0;
        else
            growth = 1;
        ch->tail_growth += growth;
        if(ch->tail_growth >= 10) {
            ch->gainTail(true);
            ch->tail_growth = 0;
        }
    }

    if (AFF_FLAGGED(ch, AFF_MBREAK) && rand_number(1, 3 + sick_fail) == 2) {
        send_to_char(ch, "@wYour mind is no longer in turmoil, you can charge ki again.@n\r\n");
        ch->affected_by.reset(AFF_MBREAK);
        if (GET_SKILL(ch, SKILL_TELEPATHY) <= 0) {
            bool condition1 = rand_number(1, 2) == 2;
            bool condition2 = rand_number(1, 20) == 1;
            if (condition1 || condition2) {
                send_to_char(ch, "@RYour senses are still a little addled... (-2 Int and Wis for 6 game hours.)@n\r\n");
                assign_affect(ch, AFF_MBREAK_DEBUFF, 0, 6, 0, 0, -2, 0, -2, 0);
            }
        }
    }
    if (AFF_FLAGGED(ch, AFF_SHOCKED) && rand_number(1, 4) == 4) {
        send_to_char(ch, "@wYour mind is no longer shocked.@n\r\n");
        if (GET_SKILL(ch, SKILL_TELEPATHY) > 0) {
            int skill = GET_SKILL(ch, SKILL_TELEPATHY), stop = false;
            improve_skill(ch, SKILL_TELEPATHY, 0);
            while (stop == false) {
                if (rand_number(1, 8) == 5)
                    stop = true;
                else
                    improve_skill(ch, SKILL_TELEPATHY, 0);
            }
            if (skill < GET_SKILL(ch, SKILL_TELEPATHY))
                send_to_char(ch, "Your mental damage and recovery has taught you things about your own mind.\r\n");
        }
        ch->affected_by.reset(AFF_SHOCKED);
    }
    if (AFF_FLAGGED(ch, AFF_FROZEN) && rand_number(1, 2) == 2) {
        send_to_char(ch, "@wYou realize you have thawed enough and break out of the ice holding you prisoner!\r\n");
        act("$n@W breaks out of the ice holding $m prisoner!", true, ch, nullptr, nullptr, TO_ROOM);
        ch->affected_by.reset(AFF_FROZEN);
    }
    if (AFF_FLAGGED(ch, AFF_WITHER) && rand_number(1, 6 + sick_fail) == 2) {
        send_to_char(ch, "@wYour body returns to normal and you beat the withering that plagued you.\r\n");
        act("$n@W's looks more fit now.", true, ch, nullptr, nullptr, TO_ROOM);
        null_affect(ch, AFF_WITHER);
    }
    if (wearing_stardust(ch) == 1) {
        ch->affected_by.set(AFF_ZANZOKEN);
        send_to_char(ch, "The stardust armor blesses you with a free zanzoken when you next need it.\r\n");
    }

}


void set_title(struct char_data *ch, char *title) {
    if (ch) {
        send_to_char(ch,
                     "Title is disabled for the time being while Iovan works on a brand new and fancier title system.\r\n");
        return;
    }
    /*
     if (title == nullptr) {
       title = class_desc_str(ch, 2, TRUE);
     }

     if (strlen(title) > MAX_TITLE_LENGTH)
       title[MAX_TITLE_LENGTH] = '\0';

     if (GET_TITLE(ch) != nullptr)
       free(GET_TITLE(ch));

     GET_TITLE(ch) = strdup(title);
    */
}

void gain_level(struct char_data *ch) {
    if (GET_LEVEL(ch) < 100 && GET_EXP(ch) >= level_exp(ch, GET_LEVEL(ch) + 1)) {
        ch->mod(CharNum::Level, 1);
        advance_level(ch);
        mudlog(BRF, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "%s advanced level to level %d.",
               GET_NAME(ch), GET_LEVEL(ch));
        send_to_char(ch, "You rise a level!\r\n");
        ch->modExperience(-level_exp(ch, GET_LEVEL(ch)));
    }
}

void run_autowiz() {

}

void gain_condition(struct char_data *ch, int condition, int value) {
    bool intoxicated;

    //Set-ups for when you cannot gain sustenance
    if (IS_NPC(ch))
        return;
    else if (IS_ANDROID(ch)) {
        return;
    } else if (GET_COND(ch, condition) < 0) {    /* No change */
        return;
    } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_RHELL)) {
        return;
    } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HELL)) {
        return;
    } else if (AFF_FLAGGED(ch, AFF_SPIRIT)) {
        return;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) <= 1) {
        return;
    }
    if (PLR_FLAGGED(ch, PLR_WRITING))
        return;

    else {
        intoxicated = (GET_COND(ch, DRUNK) > 0);

        //If there is a food value, restore that much hunger
        if (value > 0) {
            if (GET_COND(ch, condition) >= 0) {
                GET_COND(ch, condition) += value;
            }
        }

        //For food with a negative value we roll survival. On a success it cannot reduce the condition below 0
        if (!AFF_FLAGGED(ch, AFF_SPIRIT) &&
            (!GET_SKILL(ch, SKILL_SURVIVAL) || (GET_SKILL(ch, SKILL_SURVIVAL) < rand_number(1, 140)))) {
            if (value <= 0) {
                if (GET_COND(ch, condition) >= 0) {
                    if (GET_COND(ch, condition) + value < 0) {
                        GET_COND(ch, condition) = 0;
                    } else {
                        GET_COND(ch, condition) += value;
                    }
                }
            }
            //Send out hunger and thirst messages
            bool getsHungry = !(IS_NAMEK(ch));

            switch (condition) {
                case HUNGER:
                    if (getsHungry) {
                        switch (GET_COND(ch, condition)) {
                            case 0:
                                if ((ch->getCurST()) >= GET_MAX_MOVE(ch) / 3) {
                                    send_to_char(ch, "@RYou are starving to death!@n\r\n");
                                    ch->decCurSTPercent(.33);
                                }
                                else if ((ch->getCurST()) < GET_MAX_MOVE(ch) / 3) {
                                    send_to_char(ch, "@RYou are starving to death!@n\r\n");
                                    ch->decCurSTPercent(1, 0);
                                    ch->decCurHealthPercent(.34);
                                }
                                break;
                            case 1:
                                send_to_char(ch, "You are extremely hungry!\r\n");
                                break;
                            case 2:
                                send_to_char(ch, "You are very hungry!\r\n");
                                break;
                            case 3:
                                send_to_char(ch, "You are pretty hungry!\r\n");
                                break;
                            case 4:
                                send_to_char(ch, "You are hungry!\r\n");
                                break;
                            case 5:
                            case 6:
                            case 7:
                            case 8:
                                send_to_char(ch, "Your stomach is growling!\r\n");
                                break;
                            case 9:
                            case 10:
                            case 11:
                                send_to_char(ch, "You could use something to eat.\r\n");
                                break;
                            case 12:
                            case 13:
                            case 14:
                            case 15:
                            case 16:
                            case 17:
                                send_to_char(ch, "You could use a bite to eat.\r\n");
                                break;
                            case 18:
                            case 19:
                            case 20:
                                send_to_char(ch, "You could use a snack.\r\n");
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case THIRST:
                    switch (GET_COND(ch, condition)) {
                        case 0:
                            if ((ch->getCurST()) >= GET_MAX_MOVE(ch) / 3) {
                                send_to_char(ch, "@RYou are dehydrated!@n\r\n");
                                ch->decCurSTPercent(.33);
                            } else if ((ch->getCurST()) < GET_MAX_MOVE(ch) / 3) {
                                send_to_char(ch, "@RYou are dehydrated!@n\r\n");
                                ch->decCurSTPercent(1, 0);
                                ch->decCurHealthPercent(.34);
                            }
                            break;
                        case 1:
                            send_to_char(ch, "You are extremely thirsty!\r\n");
                            break;
                        case 2:
                            send_to_char(ch, "You are very thirsty!\r\n");
                            break;
                        case 3:
                            send_to_char(ch, "You are pretty thirsty!\r\n");
                            break;
                        case 4:
                            send_to_char(ch, "You are thirsty!\r\n");
                            break;
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                            send_to_char(ch, "Your throat is pretty dry!\r\n");
                            break;
                        case 9:
                        case 10:
                        case 11:
                            send_to_char(ch, "You could use something to drink.\r\n");
                            break;
                        case 12:
                        case 13:
                        case 14:
                        case 15:
                        case 16:
                        case 17:
                            send_to_char(ch, "Your mouth feels pretty dry.\r\n");
                            break;
                        case 18:
                        case 19:
                        case 20:
                            send_to_char(ch, "You could use a sip of water.\r\n");
                            break;
                        default:
                            break;
                    }
                    break;
                case DRUNK:
                    if (intoxicated) {
                        if (GET_COND(ch, DRUNK) <= 0) {
                            send_to_char(ch, "You are now sober.\r\n");
                        }
                    }
                    break;
                default:
                    break;
            }
            //If you starve or dehydrate, die and reset your conditions
            if (GET_HIT(ch) <= 0 && GET_COND(ch, HUNGER) == 0) {
                send_to_char(ch, "You have starved to death!\r\n");
                ch->decCurSTPercent(1, 0);
                act("@W$n@W falls down dead before you...@n", false, ch, nullptr, nullptr, TO_ROOM);
                die(ch, nullptr);
                if (GET_COND(ch, HUNGER) != -1) {
                    GET_COND(ch, HUNGER) = 48;
                }
                if (GET_COND(ch, THIRST) != -1) {
                    GET_COND(ch, THIRST) = 48;
                }
            }
            if (GET_HIT(ch) <= 0 && GET_COND(ch, THIRST) == 0) {
                send_to_char(ch, "You have died of dehydration!\r\n");
                ch->decCurSTPercent(1, 0);
                act("@W$n@W falls down dead before you...@n", false, ch, nullptr, nullptr, TO_ROOM);
                die(ch, nullptr);
                if (GET_COND(ch, HUNGER) != -1) {
                    GET_COND(ch, HUNGER) = 48;
                }
                GET_COND(ch, THIRST) = 48;
            }
        }
    }
}

static void check_idling(struct char_data *ch) {

    /*

    if (!dball_count(ch).empty()) {
        return;
    }
    if (++(ch->timer) > CONFIG_IDLE_VOID && IN_ROOM(ch) != 1) {
        if (GET_WAS_IN(ch) == NOWHERE && IN_ROOM(ch) != NOWHERE) {
            GET_WAS_IN(ch) = IN_ROOM(ch);
            if (FIGHTING(ch)) {
                stop_fighting(FIGHTING(ch));
                stop_fighting(ch);
            }

            act("$n disappears into the void.", true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "You have been idle, and are pulled into a void.\r\n");
            char_from_room(ch);
            char_to_room(ch, 1);
        } else if (ch->timer > CONFIG_IDLE_RENT_TIME && IN_ROOM(ch) == 1) {
            send_to_char(ch, "You are idle and are extracted safely from the game.\r\n");
            mudlog(CMP, ADMLVL_GOD, true, "%s force-rented and extracted (idle).", GET_NAME(ch));
            extract_char(ch);
        }
    }

    */
}

static void heal_limb(struct char_data *ch) {
    int healrate = 0, recovered = false;

    if (PLR_FLAGGED(ch, PLR_BANDAGED)) {
        healrate += 10;
    }

    if (GET_POS(ch) == POS_SITTING) {
        healrate += 1;
    } else if (GET_POS(ch) == POS_RESTING) {
        healrate += 3;
    } else if (GET_POS(ch) == POS_SLEEPING) {
        healrate += 5;
    }

    if (healrate > 0) {
        if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50) {
            if (GET_LIMBCOND(ch, 0) + healrate >= 50) {
                act("You realize your right arm is no longer broken.", true, ch, nullptr, nullptr, TO_CHAR);
                act("$n starts moving $s right arm gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_LIMBCOND(ch, 0) += healrate;
                recovered = true;
            } else {
                GET_LIMBCOND(ch, 0) += healrate;
                send_to_char(ch, "Your right arm feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n",
                             GET_LIMBCOND(ch, 0), "%", "%");
            }
        } else if (GET_LIMBCOND(ch, 0) + healrate < 100) {
            GET_LIMBCOND(ch, 0) += healrate;
            send_to_char(ch, "Your right arm feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n", GET_LIMBCOND(ch, 0),
                         "%", "%");
        } else if (GET_LIMBCOND(ch, 0) < 100 && GET_LIMBCOND(ch, 0) + healrate >= 100) {
            GET_LIMBCOND(ch, 0) = 100;
            send_to_char(ch, "Your right arm has fully recovered.\r\n");
        }

        if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50) {
            if (GET_LIMBCOND(ch, 1) + healrate >= 50) {
                act("You realize your left arm is no longer broken.", true, ch, nullptr, nullptr, TO_CHAR);
                act("$n starts moving $s left arm gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_LIMBCOND(ch, 1) += healrate;
                recovered = true;
            } else {
                GET_LIMBCOND(ch, 1) += healrate;
                send_to_char(ch, "Your left arm feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n",
                             GET_LIMBCOND(ch, 0), "%", "%");
            }
        } else if (GET_LIMBCOND(ch, 1) + healrate < 100) {
            GET_LIMBCOND(ch, 1) += healrate;
            send_to_char(ch, "Your left arm feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n", GET_LIMBCOND(ch, 1),
                         "%", "%");
        } else if (GET_LIMBCOND(ch, 1) < 100 && GET_LIMBCOND(ch, 1) + healrate >= 100) {
            GET_LIMBCOND(ch, 1) = 100;
            send_to_char(ch, "Your left arm has fully recovered.\r\n");
        }

        if (GET_LIMBCOND(ch, 2) > 0 && GET_LIMBCOND(ch, 2) < 50) {
            if (GET_LIMBCOND(ch, 2) + healrate >= 50) {
                act("You realize your right leg is no longer broken.", true, ch, nullptr, nullptr, TO_CHAR);
                act("$n starts moving $s right leg gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_LIMBCOND(ch, 2) += healrate;
                recovered = true;
            } else {
                GET_LIMBCOND(ch, 2) += healrate;
                send_to_char(ch, "Your right leg feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n",
                             GET_LIMBCOND(ch, 0), "%", "%");
            }
        } else if (GET_LIMBCOND(ch, 2) + healrate < 100) {
            GET_LIMBCOND(ch, 2) += healrate;
            send_to_char(ch, "Your right leg feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n", GET_LIMBCOND(ch, 2),
                         "%", "%");
        } else if (GET_LIMBCOND(ch, 2) < 100 && GET_LIMBCOND(ch, 2) + healrate >= 100) {
            GET_LIMBCOND(ch, 2) = 100;
            send_to_char(ch, "Your right leg has fully recovered.\r\n");
        }

        if (GET_LIMBCOND(ch, 3) > 0 && GET_LIMBCOND(ch, 3) < 50) {
            if (GET_LIMBCOND(ch, 3) + healrate >= 50) {
                act("You realize your left leg is no longer broken.", true, ch, nullptr, nullptr, TO_CHAR);
                act("$n starts moving $s left leg gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_LIMBCOND(ch, 3) += healrate;
                recovered = true;
            } else {
                GET_LIMBCOND(ch, 3) += healrate;
                send_to_char(ch, "Your left leg feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n",
                             GET_LIMBCOND(ch, 0), "%", "%");
            }
        } else if (GET_LIMBCOND(ch, 3) + healrate < 100) {
            GET_LIMBCOND(ch, 3) += healrate;
            send_to_char(ch, "Your left leg feels a little better @D[@G%d%s@D/@g100%s@D]@n.\r\n", GET_LIMBCOND(ch, 3),
                         "%", "%");
        } else if (GET_LIMBCOND(ch, 3) < 100 && GET_LIMBCOND(ch, 3) + healrate >= 100) {
            GET_LIMBCOND(ch, 3) = 100;
            send_to_char(ch, "Your left leg as fully recovered.\r\n");
        }

        if (!PLR_FLAGGED(ch, PLR_BANDAGED) && recovered == true) {
            if (axion_dice(-10) > GET_CON(ch)) {
                ch->mod(CharAttribute::Strength, -1);
                ch->mod(CharAttribute::Speed, -1);
                ch->mod(CharAttribute::Agility, -1);
                send_to_char(ch, "@RYou lose 1 Strength, Agility, and Speed!\r\n");
                ch->save();
            }
        }
    }

    if (PLR_FLAGGED(ch, PLR_BANDAGED) && recovered == true) {
        ch->playerFlags.reset(PLR_BANDAGED);
        send_to_char(ch, "You remove your bandages.\r\n");
        return;
    }
}

/* Update PCs, NPCs, and objects */
void point_update(uint64_t heartPulse, double deltaTime) {
    struct char_data *i, *next_char;
    struct obj_data *j, *next_thing, *jj, *next_thing2, *vehicle = nullptr;
    int change = false;
    /* characters */

    for (i = character_list; i; i = next_char) {
        next_char = i->next;

        if (!IS_NPC(i) && IN_ROOM(i) != NOWHERE) {
            if (ROOM_FLAGGED(IN_ROOM(i), ROOM_HOUSE)) {
                GET_RELAXCOUNT(i) += 1;
            } else if (GET_RELAXCOUNT(i) >= 464) {
                GET_RELAXCOUNT(i) -= 4;
            } else if (GET_RELAXCOUNT(i) >= 232) {
                GET_RELAXCOUNT(i) -= 3;
            } else if (GET_RELAXCOUNT(i) > 0 && rand_number(1, 3) == 3) {
                GET_RELAXCOUNT(i) -= 2;
            } else {
                GET_RELAXCOUNT(i) -= 1;
            }

            if (GET_RELAXCOUNT(i) < 0) {
                GET_RELAXCOUNT(i) = 0;
            }
        }
        // making it so that you don't get hungry/thirsty if you're just leisurely idling, rping, etc.
        if (!i->isFullHealth()) {
            if (rand_number(1, 2) == 2) {
                gain_condition(i, HUNGER, -1);
            }
            if (rand_number(1, 2) == 2) {
                gain_condition(i, THIRST, -1);
            }
        }
        if (rand_number(1, 2) == 2) {
            gain_condition(i, DRUNK, -1);
        }
        if (IS_NPC(i)) {
            i->aggtimer = 0;
        }

        if (GET_POS(i) >= POS_STUNNED) {
            change = false;
            update_flags(i);
            if (!IS_NPC(i)) {
                if (!i->isFullVitals()) {
                    change = true;
                }
            }

            if (PLR_FLAGGED(i, PLR_AURALIGHT)) {
                if ((i->getCurKI()) > (mana_gain(i) + i->getPercentOfMaxKI(.05))) {
                    send_to_char(i, "You send more energy into your aura to keep the light active.\r\n");
                    i->decCurKI(mana_gain(i) + i->getPercentOfMaxKI(.05));
                } else {
                    send_to_char(i, "You don't have enough energy to keep the aura active.\r\n");
                    act("$n's aura slowly stops shining and fades.\r\n", true, i, nullptr, nullptr, TO_ROOM);
                    i->playerFlags.reset(PLR_AURALIGHT);
                }
            }
            if (IS_MUTANT(i) && (GET_GENOME(i, 0) == 6 || GET_GENOME(i, 1) == 6)) {
                mutant_limb_regen(i);
            }

            int x = (GET_KAIOKEN(i) * 5) + 5;

            if (GET_SLEEPT(i) > 0 && GET_POS(i) != POS_SLEEPING) {
                GET_SLEEPT(i) -= 1;
            }
            if (GET_SLEEPT(i) < 8 && GET_POS(i) == POS_SLEEPING) {
                GET_SLEEPT(i) += rand_number(2, 4);
                if (GET_SLEEPT(i) > 8) {
                    GET_SLEEPT(i) = 8;
                }
            }

            if (GET_KAIOKEN(i) > 0) {
                improve_skill(i, SKILL_KAIOKEN, -1);
                if ((GET_SKILL(i, SKILL_KAIOKEN) < rand_number(1, x) || (i->getCurST()) <= GET_MAX_MOVE(i) / 10))
                    i->remove_kaioken(2);
            }

            if (AFF_FLAGGED(i, AFF_BURNED)) {
                if (rand_number(1, 5) >= 4) {
                    send_to_char(i, "Your burns are healed now.\r\n");
                    act("$n@w's burns are now healed.@n", true, i, nullptr, nullptr, TO_ROOM);
                    i->affected_by.reset(AFF_BURNED);
                }
            }

            i->incCurHealth(hit_gain(i));
            i->incCurST(move_gain(i));
            i->incCurKI(mana_gain(i));

            if (!IS_NPC(i)) {
                heal_limb(i);
            }

            if (SECT(IN_ROOM(i)) == SECT_WATER_NOSWIM && !CARRIED_BY(i) && !IS_KANASSAN(i)) {
                if ((i->getCurST()) >= (i->getCarriedWeight())) {
                    act("@bYou swim in place.@n", true, i, nullptr, nullptr, TO_CHAR);
                    act("@C$n@b swims in place.@n", true, i, nullptr, nullptr, TO_ROOM);
                    i->decCurST(i->getCarriedWeight());

                } else {
                    i->decCurST(i->getCarriedWeight());
                    act("@RYou are drowning!@n", true, i, nullptr, nullptr, TO_CHAR);
                    act("@C$n@b gulps water as $e struggles to stay above the water line.@n", true, i, nullptr, nullptr,
                        TO_ROOM);
                    if (i->decCurHealthPercent(0.33) <= 0) {
                        act("@rYou drown!@n", true, i, nullptr, nullptr, TO_CHAR);
                        act("@R$n@r drowns!@n", true, i, nullptr, nullptr, TO_ROOM);
                        die(i, nullptr);
                    }
                }
            }
            if (!has_o2(i) && SUNKEN(IN_ROOM(i)) && !ROOM_FLAGGED(IN_ROOM(i), ROOM_SPACE)) {
                if (((i->getCurKI()) - mana_gain(i)) > GET_MAX_MANA(i) / 200) {
                    send_to_char(i, "Your ki holds an atmosphere around you.\r\n");
                    i->decCurKI(mana_gain(i) + i->getPercentOfMaxKI(.005));
                } else {
                    if ((GET_HIT(i) - hit_gain(i)) > (i->getEffMaxPL()) * 0.05) {
                        send_to_char(i, "You struggle trying to hold your breath!\r\n");
                        i->decCurHealth(hit_gain(i) + i->getPercentOfMaxHealth(.05));
                    } else if (GET_HIT(i) <= GET_MAX_HIT(i) / 20) {
                        send_to_char(i, "You have drowned!\r\n");
                        act("@W$n@W drowns right in front of you.@n", false, i, nullptr, nullptr, TO_ROOM);
                        die(i, nullptr);
                    }
                }
            }
            if (!has_o2(i) && ROOM_FLAGGED(IN_ROOM(i), ROOM_SPACE)) {
                if (((i->getCurKI()) - mana_gain(i)) > GET_MAX_MANA(i) * 0.005) {
                    send_to_char(i, "Your ki holds an atmosphere around you.\r\n");
                    i->decCurKI(mana_gain(i) + i->getPercentOfMaxKI(.005));
                } else {
                    if ((GET_HIT(i) - hit_gain(i)) > (i->getEffMaxPL()) * 0.05) {
                        send_to_char(i, "You struggle trying to hold your breath!\r\n");
                        i->decCurHealth(hit_gain(i) + i->getPercentOfMaxHealth(.05));
                    } else if (GET_HIT(i) <= GET_MAX_HIT(i) / 20) {
                        send_to_char(i, "You have drowned!\r\n");
                        i->decCurHealthPercent(1, 1);
                        act("@W$n@W drowns right in front of you.@n", false, i, nullptr, nullptr, TO_ROOM);
                        die(i, nullptr);
                    }
                }
            }
            if (!AFF_FLAGGED(i, AFF_FLYING) && ROOM_EFFECT(IN_ROOM(i)) == 6 && !MOB_FLAGGED(i, MOB_NOKILL) &&
                !IS_DEMON(i)) {
                act("@rYour legs are burned by the lava!@n", true, i, nullptr, nullptr, TO_CHAR);
                act("@R$n@r's legs are burned by the lava!@n", true, i, nullptr, nullptr, TO_ROOM);
                if (IS_NPC(i) && IS_HUMANOID(i) && rand_number(1, 2) == 2) {
                    do_fly(i, nullptr, 0, 0);
                }
                i->decCurHealthPercent(.05);
                if (GET_HIT(i) < 0) {
                    act("@rYou have burned to death!@n", true, i, nullptr, nullptr, TO_CHAR);
                    act("@R$n@r has burned to death!@n", true, i, nullptr, nullptr, TO_ROOM);
                    die(i, nullptr);
                }
            }
            if (change && !AFF_FLAGGED(i, AFF_POISON)) {
                if (PLR_FLAGGED(i, PLR_HEALT) && SITS(i) != nullptr) {
                    send_to_char(i, "@wThe healing tank works wonders on your injuries.@n\r\n");
                    HCHARGE(SITS(i)) -= rand_number(1, 2);
                    if (HCHARGE(SITS(i)) == 0) {
                        send_to_char(i, "@wThe healing tank is now too low on energy to heal you.\r\n");
                        act("You step out of the now empty healing tank.", true, i, nullptr, nullptr, TO_CHAR);
                        act("@C$n@w steps out of the now empty healing tank.@n", true, i, nullptr, nullptr, TO_ROOM);
                        i->playerFlags.reset(PLR_HEALT);
                        SITTING(SITS(i)) = nullptr;
                        SITS(i) = nullptr;
                    } else if (i->isFullVitals()) {
                        send_to_char(i, "@wYou are fully recovered now.\r\n");
                        act("You step out of the now empty healing tank.", true, i, nullptr, nullptr, TO_CHAR);
                        act("@C$n@w steps out of the now empty healing tank.@n", true, i, nullptr, nullptr, TO_ROOM);
                        i->playerFlags.reset(PLR_HEALT);
                        SITTING(SITS(i)) = nullptr;
                        SITS(i) = nullptr;
                    }
                } else if (PLR_FLAGGED(i, PLR_HEALT) && SITS(i) == nullptr) {
                    i->playerFlags.reset(PLR_HEALT);
                } else if (GET_POS(i) == POS_SLEEPING) {
                    send_to_char(i, "@wYour sleep does you some good.@n\r\n");
                    if (!IS_ANDROID(i) && !FIGHTING(i))
                        i->restoreLF(false);
                } else if (GET_POS(i) == POS_RESTING) {
                    send_to_char(i, "@wYou feel relaxed and better.@n\r\n");
                    if (!i->isFullLF()) {
                        if (!IS_ANDROID(i) && !FIGHTING(i) && GET_SUPPRESS(i) <= 0 &&
                            GET_HIT(i) != (i->getEffMaxPL())) {
                            i->incCurLFPercent(.15);
                            send_to_char(i, "@CYou feel more lively.@n\r\n");
                        }
                    }
                } else if (GET_POS(i) == POS_SITTING)
                    send_to_char(i, "@wYou feel rested and better.@n\r\n");
                else
                    send_to_char(i, "You feel slightly better.\r\n");
            }
            if (AFF_FLAGGED(i, AFF_POISON)) {
                double cost = 0.0;
                if (GET_CON(i) >= 100) {
                    cost = 0.01;
                } else if (GET_CON(i) >= 80) {
                    cost = 0.02;
                } else if (GET_CON(i) >= 50) {
                    cost = 0.03;
                } else if (GET_CON(i) >= 30) {
                    cost = 0.04;
                } else if (GET_CON(i) >= 20) {
                    cost = 0.05;
                } else {
                    cost = 0.06;
                }
                if (GET_HIT(i) - GET_MAX_HIT(i) * cost > 0) {
                    send_to_char(i, "You puke as the poison burns through your blood.\r\n");
                    act("$n shivers and then pukes.", true, i, nullptr, nullptr, TO_ROOM);
                    i->decCurHealth(i->getEffMaxPL() * cost);
                } else {
                    send_to_char(i, "The poison claims your life!\r\n");
                    act("$n pukes up blood and falls down dead!", true, i, nullptr, nullptr, TO_ROOM);
                    if (i->poisonby) {
                        if (AFF_FLAGGED(i->poisonby, AFF_GROUP)) {
                            group_gain(i->poisonby, i);
                        } else {
                            solo_gain(i->poisonby, i);
                        }
                        die(i, i->poisonby);
                    } else {
                        die(i, nullptr);
                    }
                }
            }
            if (GET_POS(i) <= POS_STUNNED)
                update_pos(i);
        } else if (GET_POS(i) == POS_INCAP) {
            continue;
        } else if (GET_POS(i) == POS_MORTALLYW) {
            continue;
        }
        if ((i->getCurKI()) >= GET_MAX_MANA(i) * 0.5 && GET_CHARGE(i) < GET_MAX_MANA(i) * 0.1 &&
            GET_PREFERENCE(i) == PREFERENCE_KI && !PLR_FLAGGED(i, PLR_AURALIGHT)) {
            GET_CHARGE(i) = GET_MAX_MANA(i) * 0.1;
        }
        if (!IS_NPC(i)) {
            update_char_objects(i);
            if (GET_ADMLEVEL(i) < CONFIG_IDLE_MAX_LEVEL)
                check_idling(i);
            else
                (i->timer)++;
        }
    }

    /* objects */
    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;    /* Next in object list */


        /* Let's get rid of dropped norent items. */
        if (OBJ_FLAGGED(j, ITEM_NORENT) && j->worn_by == nullptr && j->carried_by == nullptr && obj_selling != j &&
            GET_OBJ_VNUM(j) != 7200) {
            time_t diff = 0;

            diff = time(nullptr) - GET_LAST_LOAD(j);
            if (diff > 240 && GET_LAST_LOAD(j) > 0) {
                basic_mud_log("No rent object (%s) extracted from room (%d)", j->short_description, GET_ROOM_VNUM(IN_ROOM(j)));
                extract_obj(j);
            }
        }

        if (GET_OBJ_TYPE(j) == ITEM_HATCH) {
            if ((vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(j, VAL_HATCH_DEST)))) {
                GET_OBJ_VAL(j, 3) = GET_ROOM_VNUM(IN_ROOM(vehicle));
            }
        }

        /* If this is a corpse */
        if (IS_CORPSE(j)) {
            /* timer count down */
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;
            if (!strstr(j->name, "android") && !strstr(j->name, "Android") && !OBJ_FLAGGED(j, ITEM_BURIED)) {
                if (GET_OBJ_TIMER(j) == 5) {
                    if ((IN_ROOM(j) != NOWHERE) && (j->getRoom()->people)) {
                        act("@DFlies start to gather around $p@D.@n", true, j->getRoom()->people, j, nullptr,
                            TO_CHAR);
                        act("@DFlies start to gather around $p@D.@n", true, j->getRoom()->people, j, nullptr,
                            TO_ROOM);
                    }
                }
                if (GET_OBJ_TIMER(j) == 3) {
                    if ((IN_ROOM(j) != NOWHERE) && (j->getRoom()->people)) {
                        act("@DA cloud of flies has formed over $p@D.@n", true, j->getRoom()->people, j, nullptr,
                            TO_CHAR);
                        act("@DA cloud of flies has formed over $p@D.@n", true, j->getRoom()->people, j, nullptr,
                            TO_ROOM);
                    }
                }
                if (GET_OBJ_TIMER(j) == 2) {
                    if ((IN_ROOM(j) != NOWHERE) && (j->getRoom()->people)) {
                        act("@DMaggots can be seen crawling all over $p@D.@n", true, j->getRoom()->people, j,
                            nullptr, TO_CHAR);
                        act("@DMaggots can be seen crawling all over $p@D.@n", true, j->getRoom()->people, j,
                            nullptr, TO_ROOM);
                    }
                }
                if (GET_OBJ_TIMER(j) == 1) {
                    if ((IN_ROOM(j) != NOWHERE) && (j->getRoom()->people)) {
                        act("@DMaggots have nearly stripped $p of all its flesh@D.@n", true, j->getRoom()->people,
                            j, nullptr, TO_CHAR);
                        act("@DMaggots have nearly stripped $p of all its flesh@D.@n", true, j->getRoom()->people,
                            j, nullptr, TO_ROOM);
                    }
                }
            }
            if (!GET_OBJ_TIMER(j)) {

                if (j->carried_by) {
                    if (!strstr(j->name, "android")) {
                        act("$p decays in your hands.", false, j->carried_by, j, nullptr, TO_CHAR);
                        if ((IN_ROOM(j) != NOWHERE) && (j->getRoom()->people)) {
                            act("A quivering horde of maggots consumes $p.",
                                true, j->getRoom()->people, j, nullptr, TO_ROOM);
                            act("A quivering horde of maggots consumes $p.",
                                true, j->getRoom()->people, j, nullptr, TO_CHAR);
                        }
                    } else {
                        act("$p decays in your hands.", false, j->carried_by, j, nullptr, TO_CHAR);
                        if ((IN_ROOM(j) != NOWHERE) && (j->getRoom()->people)) {
                            act("$p breaks down completely into a pile of junk.",
                                true, j->getRoom()->people, j, nullptr, TO_ROOM);
                            act("$p breaks down completely into a pile of junk.",
                                true, j->getRoom()->people, j, nullptr, TO_CHAR);
                        }
                    }
                }
                for (jj = j->contents; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content;    /* Next in inventory */
                    obj_from_obj(jj);

                    if (j->in_obj)
                        obj_to_obj(jj, j->in_obj);
                    else if (j->carried_by)
                        obj_to_room(jj, IN_ROOM(j->carried_by));
                    else if (IN_ROOM(j) != NOWHERE)
                        obj_to_room(jj, IN_ROOM(j));
                    else
                        core_dump();
                }
                extract_obj(j);
            }
        }

        if (GET_OBJ_VNUM(j) == 65) {
            if (HCHARGE(j) < 20 && !SITTING(j)) {
                HCHARGE(j) += rand_number(0, 1);
            }
        }
        if (GET_OBJ_TYPE(j) == ITEM_PORTAL) {
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;

            if (GET_OBJ_TIMER(j) == 0) {
                act("A glowing portal fades from existence.",
                    true, j->getRoom()->people, j, nullptr, TO_ROOM);
                act("A glowing portal fades from existence.",
                    true, j->getRoom()->people, j, nullptr, TO_CHAR);
                extract_obj(j);
            }
        } else if (GET_OBJ_VNUM(j) == 1306) {
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;

            if (GET_OBJ_TIMER(j) == 0) {
                act("The $p@n settles to the ground and goes out.",
                    true, j->getRoom()->people, j, nullptr, TO_ROOM);
                act("A $p@n settles to the ground and goes out.",
                    true, j->getRoom()->people, j, nullptr, TO_CHAR);
                extract_obj(j);
            }
        } else if (OBJ_FLAGGED(j, ITEM_ICE)) {
            if (GET_OBJ_VNUM(j) == 79 && rand_number(1, 2) == 2) {
                if (ROOM_EFFECT(IN_ROOM(j)) >= 1 && ROOM_EFFECT(IN_ROOM(j)) <= 5) {
                    send_to_room(IN_ROOM(j),
                                 "The heat from the lava melts a great deal of the glacial wall and the lava cools a bit in turn.\r\n");
                    ROOM_EFFECT(IN_ROOM(j)) -= 1;
                    if (GET_OBJ_WEIGHT(j) - (5 + (GET_OBJ_WEIGHT(j) * 0.025)) > 0) {
                        GET_OBJ_WEIGHT(j) -= 5 + (GET_OBJ_WEIGHT(j) * 0.025);
                    } else {
                        send_to_room(IN_ROOM(j),
                                     "The glacial wall blocking off the %s direction melts completely away.\r\n",
                                     dirs[GET_OBJ_COST(j)]);
                        extract_obj(j);
                    }
                } else if (GET_OBJ_WEIGHT(j) - (5 + (GET_OBJ_WEIGHT(j) * 0.025)) > 0) {
                    GET_OBJ_WEIGHT(j) -= 5 + (GET_OBJ_WEIGHT(j) * 0.025);
                    send_to_room(IN_ROOM(j), "The glacial wall blocking off the %s direction melts some what.\r\n",
                                 dirs[GET_OBJ_COST(j)]);
                } else {
                    send_to_room(IN_ROOM(j),
                                 "The glacial wall blocking off the %s direction melts completely away.\r\n",
                                 dirs[GET_OBJ_COST(j)]);
                    extract_obj(j);
                }
            } else if (GET_OBJ_VNUM(j) != 79) {
                if (j->carried_by && !j->in_obj) {
                    int melt = 5 + (GET_OBJ_WEIGHT(j) * 0.02);
                    if (GET_OBJ_WEIGHT(j) - (5 + (GET_OBJ_WEIGHT(j) * 0.02)) > 0) {
                        GET_OBJ_WEIGHT(j) -= melt;
                        send_to_char(j->carried_by, "%s @wmelts a little.\r\n", j->short_description);
                    } else {
                        send_to_char(j->carried_by, "%s @wmelts completely away.\r\n", j->short_description);
                        int remainder = melt - GET_OBJ_WEIGHT(j);
                        extract_obj(j);
                    }
                } else if (IN_ROOM(j) != NOWHERE) {
                    if (GET_OBJ_WEIGHT(j) - (5 + (GET_OBJ_WEIGHT(j) * 0.02)) > 0) {
                        GET_OBJ_WEIGHT(j) -= 5 + (GET_OBJ_WEIGHT(j) * 0.02);
                        send_to_room(IN_ROOM(j), "%s @wmelts a little.\r\n", j->short_description);
                    } else {
                        send_to_room(IN_ROOM(j), "%s @wmelts completely away.\r\n", j->short_description);
                        extract_obj(j);
                    }
                }
            }
        }

            /* If the timer is set, count it down and at 0, try the trigger */
            /* note to .rej hand-patchers: make this last in your point-update() */
        else if (GET_OBJ_TIMER(j) > 0) {
            GET_OBJ_TIMER(j)--;
            if (!GET_OBJ_TIMER(j))
                timer_otrigger(j);
        }
    }
}

void timed_dt(struct char_data *ch) {
    struct char_data *vict;
    room_rnum rrnum;

    if (ch == nullptr) {
        /* BY -WELCOR
          first make sure all rooms in the world have thier 'timed'
          value decreased if its not -1.
          */

        for (auto &r : world)
            r.second.timed -= (r.second.timed != -1);

        for (vict = character_list; vict; vict = vict->next) {
            if (IS_NPC(vict))
                continue;

            if (IN_ROOM(vict) == NOWHERE)
                continue;

            if (!ROOM_FLAGGED(IN_ROOM(vict), ROOM_TIMED_DT))
                continue;

            timed_dt(vict);
        }
        return;
    }

    /*Called with a non-null ch. let's check the room. */

    /*if the room wasn't triggered (i.e timed wasn't set), just set it
      and return again.
    */

    if (ch->getRoom()->timed < 0) {
        ch->getRoom()->timed = rand_number(2, 5);
        return;
    }

    /* We know ch is in a dt room with timed >= 0 - see if its the end.
    *
    */
    if (ch->getRoom()->timed == 0) {
        for (vict = ch->getRoom()->people; vict; vict = vict->next_in_room) {
            if (IS_NPC(vict))
                continue;
            if (GET_ADMLEVEL(vict) >= ADMLVL_IMMORT)
                continue;

            /* Skip those alread dead people */
            /* extract char() jest sets the bit*/
            if (PLR_FLAGGED(vict, PLR_NOTDEADYET))
                continue;

            log_death_trap(vict);
            death_cry(vict);
            extract_char(vict);
        }
    }
}

