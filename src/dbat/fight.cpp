/*************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/fight.h"
#include "dbat/dg_comm.h"
#include "dbat/act.attack.h"
#include "dbat/act.other.h"
#include "dbat/act.misc.h"
#include "dbat/act.movement.h"
#include "dbat/utils.h"
#include "dbat/spells.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/config.h"
#include "dbat/races.h"
#include "dbat/handler.h"
#include "dbat/combat.h"
#include "dbat/class.h"
#include "dbat/dg_scripts.h"
#include "dbat/objsave.h"
#include "dbat/act.informative.h"
#include "dbat/transformation.h"

/* Structures */
struct char_data *combat_list = nullptr;    /* head of l-list of fighting chars */
struct char_data *next_combat_list = nullptr;

/* local functions */
static void perform_group_gain(struct char_data *ch, int base, struct char_data *victim);

static void check_killer(struct char_data *ch, struct char_data *vict);

static void make_corpse(struct char_data *ch, struct char_data *tch);

static void handle_corpse_condition(struct obj_data *corpse, struct char_data *ch);

static void make_pcorpse(struct char_data *ch);

static void change_alignment(struct char_data *ch, struct char_data *victim);

static void final_combat_resolve(struct char_data *ch);

static void shadow_dragons_live();

static void cleanup_arena_watch(struct char_data *ch);

static void mob_attack(struct char_data *ch, char *buf);

static int pick_n_throw(struct char_data *ch, char *buf);


int group_bonus(struct char_data *ch, int type) {
    struct follow_type *k, *next;

    if (!AFF_FLAGGED(ch, AFF_GROUP))
        return (false);

    if (ch->followers) {
        for (k = ch->followers; k; k = next) {
            next = k->next;
            if (!AFF_FLAGGED(k->follower, AFF_GROUP)) {
                continue;
            } else {
                if (type == 0) {
                    k->follower->incCurLFPercent(.25);
                    send_to_char(k->follower, "@CIncensed by the death of your comrade your life force swells!@n");
                    return (true);
                } else if (type == 1) {
                    k->follower->incCurLFPercent(.4);
                    send_to_char(k->follower, "@CIncensed by the death of your comrade your life force swells!@n");
                    return (true);
                } else if (type == 2) {
                    if (IS_ROSHI(ch)) {
                        return (2);
                    } else if (IS_KRANE(ch)) {
                        return (3);
                    } else if (IS_BARDOCK(ch)) {
                        return (4);
                    } else if (IS_NAIL(ch)) {
                        return (5);
                    } else if (IS_KABITO(ch)) {
                        return (6);
                    } else if (IS_ANDSIX(ch)) {
                        return (7);
                    } else if (IS_TAPION(ch)) {
                        return (8);
                    } else if (IS_FRIEZA(ch)) {
                        return (9);
                    } else if (IS_TSUNA(ch)) {
                        return (10);
                    } else if (IS_PICCOLO(ch)) {
                        return (11);
                    } else if (IS_KURZAK(ch)) {
                        return (12);
                    } else if (IS_JINTO(ch)) {
                        return (13);
                    } else if (IS_DABURA(ch)) {
                        return (14);
                    }
                }
                return (false);
            }
        }
    } else if (ch->master) {
        if (!AFF_FLAGGED(ch->master, AFF_GROUP))
            return (false);
        else {
            if (type == 0) {
                group_bonus(ch->master, 0);
            } else if (type == 2) {
                if (IS_ROSHI(ch->master)) {
                    return (2);
                } else if (IS_KRANE(ch->master)) {
                    return (3);
                } else if (IS_BARDOCK(ch->master)) {
                    return (4);
                } else if (IS_NAIL(ch->master)) {
                    return (5);
                } else if (IS_KABITO(ch->master)) {
                    return (6);
                } else if (IS_ANDSIX(ch->master)) {
                    return (7);
                } else if (IS_TAPION(ch->master)) {
                    return (8);
                } else if (IS_FRIEZA(ch->master)) {
                    return (9);
                } else if (IS_TSUNA(ch->master)) {
                    return (10);
                } else if (IS_PICCOLO(ch->master)) {
                    return (11);
                } else if (IS_KURZAK(ch->master)) {
                    return (12);
                } else if (IS_JINTO(ch->master)) {
                    return (13);
                } else if (IS_DABURA(ch->master)) {
                    return (14);
                }
            }
            return (true);
        }
    }
    return (false);
}

void mutant_limb_regen(struct char_data *ch) {
    if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50) {
        act("The bones in your right arm have mended them selves.", true, ch, nullptr, nullptr, TO_CHAR);
        act("$n starts moving $s right arm gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
        GET_LIMBCOND(ch, 0) = 100;
    } else if (GET_LIMBCOND(ch, 0) <= 0) {
        act("Your right arm begins to grow back very quickly. Within moments it is whole again!", true, ch, nullptr,
            nullptr, TO_CHAR);
        act("$n's right arm starts to regrow! Within moments the arm is whole again!.", true, ch, nullptr, nullptr,
            TO_ROOM);
        GET_LIMBCOND(ch, 0) = 100;
    }
    if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50) {
        act("The bones in your left arm have mended them selves.", true, ch, nullptr, nullptr, TO_CHAR);
        act("$n starts moving $s left arm gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
        GET_LIMBCOND(ch, 1) = 100;
    } else if (GET_LIMBCOND(ch, 1) <= 0) {
        act("Your right arm begins to grow back very quickly. Within moments it is whole again!", true, ch, nullptr,
            nullptr, TO_CHAR);
        act("$n's right arm starts to regrow! Within moments the arm is whole again!.", true, ch, nullptr, nullptr,
            TO_ROOM);
        GET_LIMBCOND(ch, 1) = 100;
    }
    if (GET_LIMBCOND(ch, 2) > 0 && GET_LIMBCOND(ch, 2) < 50) {
        act("The bones in your right leg have mended them selves.", true, ch, nullptr, nullptr, TO_CHAR);
        act("$n starts moving $s right leg gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
        GET_LIMBCOND(ch, 2) = 100;
    } else if (GET_LIMBCOND(ch, 2) <= 0) {
        act("Your right arm begins to grow back very quickly. Within moments it is whole again!", true, ch, nullptr,
            nullptr, TO_CHAR);
        act("$n's right arm starts to regrow! Within moments the arm is whole again!.", true, ch, nullptr, nullptr,
            TO_ROOM);
        GET_LIMBCOND(ch, 2) = 100;
    }
    if (GET_LIMBCOND(ch, 3) > 0 && GET_LIMBCOND(ch, 3) < 50) {
        act("The bones in your left leg have mended them selves.", true, ch, nullptr, nullptr, TO_CHAR);
        act("$n starts moving $s left leg gingerly for a moment.", true, ch, nullptr, nullptr, TO_ROOM);
        GET_LIMBCOND(ch, 3) = 100;
    } else if (GET_LIMBCOND(ch, 3) <= 0) {
        act("Your right arm begins to grow back very quickly. Within moments it is whole again!", true, ch, nullptr,
            nullptr, TO_CHAR);
        act("$n's right arm starts to regrow! Within moments the arm is whole again!.", true, ch, nullptr, nullptr,
            TO_ROOM);
        GET_LIMBCOND(ch, 3) = 100;
    }
}

static int pick_n_throw(struct char_data *ch, char *buf) {
    struct obj_data *cont;
    char buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];;

    if (rand_number(1, 20) < 18) {
        return (false);
    }


    for (cont = ch->getRoom()->contents; cont; cont = cont->next_content) {
        if (ch->canCarryWeight(cont)) {
            sprintf(buf2, "%s", cont->name);
            do_get(ch, buf2, 0, 0);
            sprintf(buf3, "%s %s", buf2, buf);
            do_throw(ch, buf3, 0, 0);
            return (true);
        }
    }

    return (false);
}

static void mob_attack(struct char_data *ch, char *buf) {

    int power = rand_number(1, 5);
    int bonus = GET_LEVEL(ch) * 0.1;
    int special = 0;
    char buf2[MAX_INPUT_LENGTH];

    power += bonus;

    if (rand_number(1, 4) == 4)
        power += 10;

    if (power > 20)
        power = 20;

    if (GET_CLASS(ch) == SenseiID::Commoner)
        special = 0;

    int dragonpass = true;

    if (IS_DRAGON(ch)) {
        if (GET_MOB_VNUM(ch) == 81 || GET_MOB_VNUM(ch) == 82 || GET_MOB_VNUM(ch) == 83 || GET_MOB_VNUM(ch) == 84 ||
            GET_MOB_VNUM(ch) == 85 || GET_MOB_VNUM(ch) == 86 || GET_MOB_VNUM(ch) == 87) {
            dragonpass = true;
            special = rand_number(40, 100);
        } else {
            dragonpass = false;
        }
    }

    if (axion_dice(-10) > 90 && ch->getCurHealthPercent() <= .5 && !MOB_FLAGGED(ch, MOB_POWERUP) &&
        GET_MOB_VNUM(ch) != 25 &&
        !(IS_ANDROID(ch) || IS_ANIMAL(ch) || ch->chclass == SenseiID::Commoner)) {
        do_powerup(ch, nullptr, 0, 0);
        return;
    }

    if ((ch->getCurKI()) >= GET_MAX_MANA(ch) * 0.05 && IS_HUMANOID(ch) && (!IS_DRAGON(ch) || dragonpass == true)) {
        if (ch->mobcharge <= 0 && rand_number(1, 10) >= 8) {
            act("@wAn aura flares up around @R$n@w!@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->mobcharge += 1;
            if (GET_LEVEL(ch) > 80) {
                ch->mobcharge += 1;
            }
        } else if (ch->mobcharge <= 5) {
            act("@wThe aura burns brighter around @R$n@w!@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->mobcharge += 1;
            if (GET_LEVEL(ch) > 80) {
                ch->mobcharge += 1;
            }
        } else if (ch->mobcharge == 6) {
            act("@wThe aura around @R$n@w flashes!@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->mobcharge += 1;
            special = 100;
        }
    }

    if (IS_HUMANOID(ch) && dragonpass == true) { /* Is a humanoid mob */
        if (AFF_FLAGGED(ch, AFF_PARALYZE)) {
            return;
        } else if (AFF_FLAGGED(ch, AFF_ENSNARED)) {
            return;
        } else if (special < 100) { /* Normal physical attack */
            if (IS_ANDROID(ch) && MOB_FLAGGED(ch, MOB_REPAIR) && GET_HIT(ch) <= (ch->getEffMaxPL()) * 0.5 &&
                       rand_number(1, 20) >= 16) {
                do_srepair(ch, nullptr, 0, 0);
            } else if (IS_ANDROID(ch) && MOB_FLAGGED(ch, MOB_ABSORB) && rand_number(1, 20) >= 19) {
                do_absorb(ch, buf2, 0, 0);
            } else if ((IS_BIO(ch) || IS_MAJIN(ch)) && GET_HIT(ch) <= (ch->getEffMaxPL()) * 0.5 &&
                       rand_number(1, 20) >= 17) {
                do_regenerate(ch, "25", 0, 0);
            } else if (IS_NAMEK(ch) && GET_HIT(ch) <= (ch->getEffMaxPL()) * 0.5 && rand_number(1, 20) == 20) {
                do_regenerate(ch, "25", 0, 0);
            } else if (pick_n_throw(ch, buf)) {
                /* This determines if they throw and also handles it */
            } else if (MOB_FLAGGED(ch, MOB_KNOWKAIO) && rand_number(1, 50) >= 46) {
                if (rand_number(1, 10) == 10) {
                    do_kaioken(ch, "20", 0, 0);
                } else if (rand_number(1, 10) >= 8) {
                    do_kaioken(ch, "10", 0, 0);
                } else {
                    do_kaioken(ch, "5", 0, 0);
                }
            } else {
                switch (power) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        if (GET_EQ(ch, WEAR_WIELD1))
                            do_attack(ch, buf, 0, 0);
                        else if (rand_number(1, 5) == 5)
                            do_kick(ch, buf, 0, 0);
                        else if (rand_number(1, 10) == 10)
                            do_elbow(ch, buf, 0, 0);
                        else
                            do_punch(ch, buf, 0, 0);
                        break;
                    case 6:
                    case 7:
                    case 8:
                        if (GET_EQ(ch, WEAR_WIELD1))
                            do_attack(ch, buf, 0, 0);
                        else if (rand_number(1, 5) == 5)
                            do_punch(ch, buf, 0, 0);
                        else if (rand_number(1, 10) == 10)
                            do_knee(ch, buf, 0, 0);
                        else
                            do_kick(ch, buf, 0, 0);
                        break;
                    case 9:
                    case 10:
                        if (rand_number(1, 5) == 5)
                            do_knee(ch, buf, 0, 0);
                        else if (rand_number(1, 10) == 10)
                            do_uppercut(ch, buf, 0, 0);
                        else
                            do_elbow(ch, buf, 0, 0);
                        break;
                    case 11:
                    case 12:
                        if (rand_number(1, 5) == 5)
                            do_elbow(ch, buf, 0, 0);
                        else if (rand_number(1, 10) == 10)
                            do_roundhouse(ch, buf, 0, 0);
                        else if (rand_number(1, 8) == 8)
                            do_trip(ch, buf, 0, 0);
                        else
                            do_knee(ch, buf, 0, 0);
                        break;
                    case 13:
                    case 14:
                        if ((IS_BARDOCK(ch) || IS_KURZAK(ch)) && rand_number(1, 2) == 2)
                            do_head(ch, buf, 0, 0);
                        else if ((IS_ICER(ch) || IS_BIO(ch)) && rand_number(1, 2) == 2)
                            do_tailwhip(ch, buf, 0, 0);
                        else if (rand_number(1, 8) == 8)
                            do_trip(ch, buf, 0, 0);
                        else
                            do_uppercut(ch, buf, 0, 0);
                        break;
                    case 15:
                    case 16:
                        if ((IS_BARDOCK(ch) || IS_KURZAK(ch)) && rand_number(1, 2) == 2)
                            do_head(ch, buf, 0, 0);
                        else if ((IS_ICER(ch) || IS_BIO(ch)) && rand_number(1, 2) == 2)
                            do_tailwhip(ch, buf, 0, 0);
                        else if (rand_number(1, 8) >= 7)
                            do_trip(ch, buf, 0, 0);
                        else
                            do_roundhouse(ch, buf, 0, 0);
                        break;
                    case 17:
                    case 18:
                        do_slam(ch, buf, 0, 0);
                        break;
                    case 19:
                    case 20:
                        do_heeldrop(ch, buf, 0, 0);
                        break;
                }
            }
        } else {
            mob_specials_used += 1;
            switch (power) {
                case 1:
                case 2:
                case 3:
                case 4:
                    if (special > 80)
                        do_zanzoken(ch, buf, 0, 0);
                    if (ch->mobcharge == 7) {
                        ch->mobcharge = 0;
                        do_kiball(ch, buf, 0, 0);
                    }
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                    if (special > 80)
                        do_zanzoken(ch, buf, 0, 0);
                    if (ch->mobcharge == 7) {
                        ch->mobcharge = 0;
                        do_kiblast(ch, buf, 0, 0);
                    }
                    break;
                case 9:
                case 10:
                case 11:
                    if (special > 80)
                        do_zanzoken(ch, buf, 0, 0);
                    if (IS_DRAGON(ch) && rand_number(1, 4) == 4) {
                        do_breath(ch, buf, 0, 0);
                    } else {
                        if (ch->mobcharge == 7) {
                            ch->mobcharge = 0;
                            do_beam(ch, buf, 0, 0);
                        }
                    }
                    break;
                case 12:
                case 13:
                case 14:
                    if (special > 80)
                        do_zanzoken(ch, buf, 0, 0);
                    if (IS_DRAGON(ch) && rand_number(1, 4) == 4) {
                        do_breath(ch, buf, 0, 0);
                    } else {
                        if (ch->mobcharge == 7) {
                            ch->mobcharge = 0;
                            do_renzo(ch, buf, 0, 0);
                        }
                    }
                    break;
                case 15:
                case 16:
                    if (IS_DRAGON(ch) && rand_number(1, 4) == 4) {
                        do_breath(ch, buf, 0, 0);
                    } else {
                        if (ch->mobcharge == 7) {
                            ch->mobcharge = 0;
                            do_tsuihidan(ch, buf, 0, 0);
                        }
                    }
                    break;
                case 17:
                case 18:
                    if (IS_DRAGON(ch) && rand_number(1, 4) == 4) {
                        do_breath(ch, buf, 0, 0);
                    } else {
                        if (ch->mobcharge == 7) {
                            ch->mobcharge = 0;
                            do_shogekiha(ch, buf, 0, 0);
                        }
                    }
                    break;
                case 19:
                case 20:
                    if (IS_DRAGON(ch)) {
                        do_breath(ch, buf, 0, 0);
                    }
                    if (ch->mobcharge == 7) {
                        ch->mobcharge = 0;
                        switch (GET_CLASS(ch)) {
                            case SenseiID::Roshi:
                                if (special >= 100)
                                    do_kakusanha(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_kienzan(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_kamehameha(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Frieza:
                                if (special >= 100)
                                    do_deathball(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_kienzan(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_deathbeam(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Krane:
                                if (special >= 100)
                                    do_tribeam(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_hass(ch, nullptr, 0, 0);
                                else if (special >= 70)
                                    do_dodonpa(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Piccolo:
                                if (special >= 100)
                                    do_scatter(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_sbc(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_masenko(ch, buf, 0, 0);
                                else if (special >= 100)
                                    do_balefire(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Bardock:
                                if (special >= 100)
                                    do_final(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_bigbang(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_galikgun(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Sixteen:
                                if (special >= 100)
                                    do_hellflash(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_kousengan(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_dualbeam(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Nail:
                                if (special >= 100)
                                    do_regenerate(ch, "50", 0, 0);
                                else if (special >= 80)
                                    do_heal(ch, "self", 0, 0);
                                else if (special >= 70)
                                    do_masenko(ch, buf, 0, 0);
                                else
                                    do_zanzoken(ch, nullptr, 0, 0);
                                break;
                            case SenseiID::Kurzak:
                                if (special >= 100)
                                    do_ensnare(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_seishou(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_renzo(ch, buf, 0, 0);
                                else if (special >= 50)
                                    do_barrier(ch, "40", 0, 0);
                                else
                                    do_barrier(ch, "25", 0, 0);
                                break;
                            case SenseiID::Jinto:
                                if (special >= 100)
                                    do_nova(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_breaker(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_trip(ch, buf, 0, 0);
                                else
                                    do_zanzoken(ch, "40", 0, 0);
                                break;
                            case SenseiID::Tsuna:
                                if (special >= 100)
                                    do_koteiru(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_razor(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_spike(ch, buf, 0, 0);
                                else
                                    do_barrier(ch, "20", 0, 0);
                                break;
                            case SenseiID::Tapion:
                                if (special >= 100)
                                    do_pslash(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_ddslash(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_tslash(ch, buf, 0, 0);
                                else
                                    do_zanzoken(ch, "40", 0, 0);
                                break;
                            case SenseiID::Kibito:
                                if (special >= 100)
                                    do_pbarrage(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_psyblast(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_heal(ch, buf, 0, 0);
                                else
                                    do_zanzoken(ch, "40", 0, 0);
                                break;
                            case SenseiID::Dabura:
                                if (special >= 100)
                                    do_hellspear(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_honoo(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_fireshield(ch, buf, 0, 0);
                                else
                                    do_zanzoken(ch, "40", 0, 0);
                                break;
                            case SenseiID::Ginyu:
                                if (special >= 100)
                                    do_spiral(ch, buf, 0, 0);
                                else if (special >= 80)
                                    do_crusher(ch, buf, 0, 0);
                                else if (special >= 70)
                                    do_eraser(ch, buf, 0, 0);
                                else
                                    do_zanzoken(ch, "40", 0, 0);
                                break;
                        }
                    }
                    break;
            } /* End power switch */
        } /* End special attacks */
    } else if (!IS_HUMANOID(ch) || dragonpass == false) { /* Is not a humanoid mob */
        if (IS_SERPENT(ch) && rand_number(1, 5) == 5) {
            do_strike(ch, buf, 0, 0);
        } else if (IS_DRAGON(ch) && rand_number(1, 12) >= 10 && GET_MOB_VNUM(ch) != 17917) {
            do_breath(ch, buf, 0, 0);
        } else {
            if (rand_number(1, 10) >= 7 && GET_LEVEL(ch) >= 10) {
                do_ram(ch, buf, 0, 0);
            } else {
                do_bite(ch, buf, 0, 0);
            }
        }
    }

    fight_mtrigger(ch);

} /* End mob_attack */

static void cleanup_arena_watch(struct char_data *ch) {
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {

        if (STATE(d) != CON_PLAYING)
            continue;

        if (PRF_FLAGGED(d->character, PRF_ARENAWATCH)) {
            if (ARENA_IDNUM(d->character) == GET_IDNUM(ch)) {
                d->character->pref.reset(PRF_ARENAWATCH);
                ARENA_IDNUM(d->character) = -1;
            }
        }
    }
}

static void shadow_dragons_live() {
    int value = 0;
    if (SHADOW_DRAGON1 != -1 || SHADOW_DRAGON2 != -1 || SHADOW_DRAGON3 != -1 || SHADOW_DRAGON4 != -1 ||
        SHADOW_DRAGON5 != -1 || SHADOW_DRAGON6 != -1 || SHADOW_DRAGON7 != -1) {
        value = 1;
    }

    if (value == 0) {
        SELFISHMETER = 0;
        save_mud_time(&time_info);
    }
}

/* For announcing the sounds of battle to nearby rooms */
void impact_sound(struct char_data *ch, char *mssg) {
    int door;
    for (door = 0; door < NUM_OF_DIRS; door++)
        if (CAN_GO(ch, door))
            send_to_room(ch->getRoom()->dir_option[door]->to_room, "%s", mssg);
}

/* For removing body parts */
void remove_limb(struct char_data *vict, int num) {
    /* 0 = head, 1 = rarm, 2 = larm, 3 = rleg, 4 = lleg , 5 = tail, 6 = tail*/

    struct obj_data *body_part;
    char part[1000];
    char buf[1000];
    char buf2[1000];

    body_part = create_obj();
    body_part->vn = NOTHING;
    IN_ROOM(body_part) = NOWHERE;

    switch (num) {
        case 0:
            snprintf(part, sizeof(part), "@C%s@w's bloody head@n", GET_NAME(vict));
            snprintf(buf, sizeof(buf), "%s bloody head", GET_NAME(vict));
            break;
        case 1:
            snprintf(part, sizeof(part), "@w%s right arm@n", race::getName(vict->race).c_str());
            snprintf(buf, sizeof(buf), "right arm");
        vict->pref.reset(PLR_CRARM);
            break;
        case 2:
            snprintf(part, sizeof(part), "@w%s left arm@n", race::getName(vict->race).c_str());
            snprintf(buf, sizeof(buf), "left arm");
            vict->pref.reset(PLR_CLARM);
            break;
        case 3:
            snprintf(part, sizeof(part), "@w%s right leg@n", race::getName(vict->race).c_str());
            snprintf(buf, sizeof(buf), "right leg");
            vict->pref.reset(PLR_CRLEG);
            break;
        case 4:
            snprintf(part, sizeof(part), "@w%s left leg@n", race::getName(vict->race).c_str());
            snprintf(buf, sizeof(buf), "left leg");
            vict->pref.reset(PLR_CLLEG);
            break;
        case 5:
            snprintf(part, sizeof(part), "@wA %s tail@n", race::getName(vict->race).c_str());
            snprintf(buf, sizeof(buf), "tail");
            break;
        case 6:
            snprintf(buf, sizeof(buf), "tail");
            break;
        default:
            snprintf(part, sizeof(part), "@w%s body part@n", race::getName(vict->race).c_str());
            snprintf(buf, sizeof(buf), "body part");
            break;
    }

    body_part->name = strdup(buf);
    if (num > 0) {
        snprintf(buf2, sizeof(buf2), "@wA %s is lying here@n", part);
    } else {
        snprintf(buf2, sizeof(buf2), "%s@w is lying here@n", part);
    }
    body_part->room_description = strdup(buf2);
    body_part->short_description = strdup(part);

    GET_OBJ_TYPE(body_part) = ITEM_OTHER;
    body_part->wear_flags.set(ITEM_WEAR_TAKE);
    body_part->extra_flags.set(ITEM_UNIQUE_SAVE);
    GET_OBJ_VAL(body_part, 0) = 0;
    GET_OBJ_VAL(body_part, 1) = 0;
    GET_OBJ_VAL(body_part, 2) = 0;
    GET_OBJ_VAL(body_part, 3) = 0;
    GET_OBJ_VAL(body_part, 4) = 1;
    GET_OBJ_VAL(body_part, 5) = 1;
    GET_OBJ_WEIGHT(body_part) = rand_number(4, 10);
    GET_OBJ_RENT(body_part) = 0;
    obj_to_room(body_part, IN_ROOM(vict));
}

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[NUM_ATTACK_TYPES] =
        {
                {"hit",      "hits"},        /* 0 */
                {"sting",    "stings"},
                {"whip",     "whips"},
                {"slash",    "slashes"},
                {"bite",     "bites"},
                {"bludgeon", "bludgeons"},    /* 5 */
                {"crush",    "crushes"},
                {"pound",    "pounds"},
                {"claw",     "claws"},
                {"maul",     "mauls"},
                {"thrash",   "thrashes"},    /* 10 */
                {"pierce",   "pierces"},
                {"blast",    "blasts"},
                {"punch",    "punches"},
                {"stab",     "stabs"}
        };

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))


/* The Fight related routines */
void fight_stack(uint64_t heartPulse, double deltaTime) {
    int perc = 0;
    struct char_data *ch;
    struct char_data *tch;
    struct char_data *wch;

    for (tch = character_list; tch; tch = tch->next) {
        ch = tch;

        if (GET_POS(ch) == POS_FIGHTING) {
            GET_POS(ch) = POS_STANDING;
        }
        if (PLR_FLAGGED(ch, PLR_SPIRAL)) {
            handle_spiral(ch, nullptr, GET_SKILL(ch, SKILL_SPIRAL), false);
        }
        if (IS_NPC(ch) && MOB_COOLDOWN(ch) > 0) {
            MOB_COOLDOWN(ch) -= 1;
            if (rand_number(1, 2) == 2 && MOB_COOLDOWN(ch) > 0) {
                MOB_COOLDOWN(ch) -= 1;
            }
            if (MOB_COOLDOWN(ch) > 0) {
                continue;
            }
        }
        if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_POWERUP) && axion_dice(0) >= 90) {
            if (GET_HIT(ch) >= GET_MAX_HIT(ch)) {
                act("@g$n@ finishes powering up as $s aura flashes brightly filling the entire area briefly with its light!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                ch->restoreHealth(false);
                ch->mobFlags.set(MOB_POWERUP);
            } else if (GET_HIT(ch) >= GET_MAX_HIT(ch) / 2) {
                act("@g$n@G continues powering up as torrents of energy crackle within $s aura.@n", true, ch, nullptr,
                    nullptr, TO_ROOM);
                ch->incCurHealthPercent(.1);
            } else if (GET_HIT(ch) > GET_MAX_HIT(ch) / 4) {
                act("@g$n@G powers up as a steady aura around $s body grow brighter.@n", true, ch, nullptr, nullptr,
                    TO_ROOM);
                ch->incCurHealthPercent(.125);
            } else if (GET_HIT(ch) > 0) {
                act("@g$n@G powers up, as a weak aura flickers around $s body.@n", true, ch, nullptr, nullptr, TO_ROOM);
                ch->incCurHealthPercent(.2);
            }
        }
        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_FROZEN)) {
            continue;
        }
        if (!GRAPPLING(ch) && !GRAPPLED(ch) && !FIGHTING(ch) && !PLR_FLAGGED(ch, PLR_CHARGE) &&
            !PLR_FLAGGED(ch, PLR_POWERUP) && GET_CHARGE(ch) <= 0 && !IS_TRANSFORMED(ch)) {
            continue;
        }
        if (FIGHTING(ch) && (IN_ROOM(FIGHTING(ch)) != IN_ROOM(ch))) {
            wch = FIGHTING(ch);
            stop_fighting(wch);
            stop_fighting(ch);
        }
        if (FIGHTING(ch) && DRAGGING(ch)) {
            act("@WYou are forced to stop dragging @C$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
            act("@C$n@W is forced to stop dragging @c$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
            DRAGGED(DRAGGING(ch)) = nullptr;
            DRAGGING(ch) = nullptr;
        }

        if (GET_LIFEPERC(ch) > 0 && ch->health < (double) GET_LIFEPERC(ch) / 100 && (ch->getCurLF()) > 0 &&
            !IS_ANDROID(ch)) {
            if (rand_number(1, 15) >= 14) {
                if ((ch->getCurLF()) >= (ch->getMaxLF()) * 0.05 || AFF_FLAGGED(ch, AFF_HEALGLOW) || (IS_KANASSAN(ch) &&
                                                                                                     (ch->getCurLF()) >=
                                                                                                     (ch->getMaxLF()) *
                                                                                                     0.03)) {
                    int64_t refill = 0, lfcost = (ch->getMaxLF()) * 0.05;
                    if (GET_BONUS(ch, BONUS_DIEHARD) > 0 &&
                        (!IS_MUTANT(ch) || (GET_GENOME(ch, 0) != 2 && GET_GENOME(ch, 1) != 2))) {
                        refill = (ch->getMaxLF()) * 0.1;
                    } else if (GET_BONUS(ch, BONUS_DIEHARD) > 0 && IS_MUTANT(ch) &&
                               (GET_GENOME(ch, 0) == 2 || GET_GENOME(ch, 1) == 2)) {
                        refill = (ch->getMaxLF()) * 0.17;
                    } else if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 2 || GET_GENOME(ch, 1) == 2)) {
                        refill = (ch->getMaxLF()) * 0.12;
                    } else if (IS_KANASSAN(ch)) {
                        lfcost = (ch->getMaxLF()) * 0.03;
                        refill = (ch->getMaxLF()) * 0.03;
                    } else {
                        refill = (ch->getMaxLF()) * 0.05;
                    }
                    ch->incCurHealth(refill);
                    if (!AFF_FLAGGED(ch, AFF_HEALGLOW)) {
                        ch->decCurLF(lfcost);
                    }
                } else {
                    ch->incCurHealth((ch->getCurLF()));
                    ch->decCurLFPercent(2, -1);
                }

                send_to_char(ch, "@YYour life force has kept you strong@n!\r\n");
            }
        }

        if (!AFF_FLAGGED(ch, AFF_POSITION)) {
            if (roll_balance(ch) > axion_dice(0) && rand_number(1, 10) >= 7) {
                if (FIGHTING(ch)) {
                    if (!AFF_FLAGGED(FIGHTING(ch), AFF_POSITION)) {
                        act("@YYou manage to move into an advantageous position!@n", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@y$n@Y manages to move into an advantageous position!@n", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        ch->affected_by.set(AFF_POSITION);
                    } else {
                        struct char_data *vict = FIGHTING(ch);
                        if (roll_balance(ch) > roll_balance(vict)) {
                            act("@YYou struggle to gain a better position than @y$N@Y and succeed!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@y$n@Y struggles to gain a better position than you and succeeds!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@y$n@Y struggles to gain a better position than @y$N@Y and succeeds!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            vict->affected_by.reset(AFF_POSITION);
                            ch->affected_by.set(AFF_POSITION);
                        }
                    }
                }
            }
        } else {
            if (roll_balance(ch) < axion_dice(-30) || GET_POS(ch) < POS_STANDING) {
                act("@YYou are moved out of your position!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@y$n@Y is moved out of $s position!@n", true, ch, nullptr, nullptr, TO_ROOM);
                ch->affected_by.reset(AFF_POSITION);
            }
        }
        if (GRAPPLING(ch) && GRAPTYPE(ch) == 2 && rand_number(1, 11) >= 8) {
            if ((((ch)->grappling)->getCurST()) >= GET_MAX_MOVE(GRAPPLING(ch)) / 8) {
                act("@WYou choke @C$N@W!@n", true, ch, nullptr, GRAPPLING(ch), TO_CHAR);
                act("@C$n@W chokes YOU@W!@n", true, ch, nullptr, GRAPPLING(ch), TO_VICT);
                act("@C$n@W chokes @c$N@W!@n", true, ch, nullptr, GRAPPLING(ch), TO_NOTVICT);
                GRAPPLING(ch)->decCurST(GRAPPLING(ch)->getMaxST() / 8);
            } else {
                act("@WYou choke @C$N@W, and $E passes out!@n", true, ch, nullptr, GRAPPLING(ch), TO_CHAR);
                act("@C$n@W chokes YOU@W, and you pass out!@n", true, ch, nullptr, GRAPPLING(ch), TO_VICT);
                act("@C$n@W chokes @c$N@W, and $E passes out!@n", true, ch, nullptr, GRAPPLING(ch), TO_NOTVICT);
                ch->affected_by.set(AFF_KNOCKED);
                GET_POS(GRAPPLING(ch)) = POS_SLEEPING;
                GRAPTYPE(GRAPPLING(ch)) = -1;
                GRAPPLED(GRAPPLING(ch)) = nullptr;
                GRAPPLING(ch) = nullptr;
                GRAPTYPE(ch) = -1;
            }
        } else if (GRAPPLING(ch) && GRAPTYPE(ch) == 4 && rand_number(1, 12) >= 8) {
            act("@WYou crush @C$N@W some more!@n", true, ch, nullptr, GRAPPLING(ch), TO_CHAR);
            act("@C$n@W crushes YOU@W some more!@n", true, ch, nullptr, GRAPPLING(ch), TO_VICT);
            act("@C$n@W crushes @c$N@W some more!@n", true, ch, nullptr, GRAPPLING(ch), TO_NOTVICT);
            int64_t damg = GET_STR(ch) * (10 + (GET_MAX_HIT(ch) * 0.005));
            hurt(0, 0, ch, GRAPPLING(ch), nullptr, damg, 0);
        }
        if (GRAPPLED(ch) && rand_number(1, 2) == 2) {
            send_to_char(ch, "@CTry 'escape' to break free from the hold!@n\r\n");
        }
        if (IS_HALFBREED(ch) && PLR_FLAGGED(ch, PLR_FURY)) {
            GET_RMETER(ch) += 1;
            if (GET_RMETER(ch) >= 1000) {

                ch->incCurHealthPercent(.15);
                ch->incCurKIPercent(.15);
                ch->incCurSTPercent(.15);
                send_to_char(ch, "Your fury has called forth more of your hidden power and you feel better!\r\n");
            }
        }

        if (!IS_NPC(ch) && GET_WIMP_LEV(ch) && GET_HIT(ch) < GET_WIMP_LEV(ch) && GET_HIT(ch) > 0 && FIGHTING(ch)) {
            send_to_char(ch, "You wimp out, and attempt to flee!\r\n");
            do_flee(ch, nullptr, 0, 0);
        }
        if (IS_NPC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 10 && GET_HIT(ch) > 0 && FIGHTING(ch) &&
            !MOB_FLAGGED(ch, MOB_SENTINEL)) {
            if (rand_number(1, 30) >= 25 && GET_POS(ch) > POS_SITTING) {
                do_flee(ch, nullptr, 0, 0);
            }
        }
        if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 6 || GET_GENOME(ch, 1) == 6) && rand_number(1, 200) >= 175) {
            mutant_limb_regen(ch);
        }
        if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DISGUISED) && GET_SKILL(ch, SKILL_DISGUISE) < rand_number(1, 125)) {
            send_to_char(ch, "Your disguise comes off because of your swift movements!\r\n");
            ch->playerFlags.reset(PLR_DISGUISED);
            act("@W$n's@W disguise comes off because of $s swift movements!@n", false, ch, nullptr, nullptr, TO_ROOM);
        }
        if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_BLIND) && rand_number(1, 200) >= 190) {
            act("@W$n@W is no longer blind.@n", false, ch, nullptr, nullptr, TO_ROOM);
            ch->affected_by.reset(AFF_BLIND);
        }

        if (AFF_FLAGGED(ch, AFF_KNOCKED) && rand_number(1, 200) >= 195) {
            ch->cureStatusKnockedOut(true);
            if (IS_NPC(ch) && rand_number(1, 20) >= 12) {
                act("@W$n@W stands up.@n", false, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_STANDING;
            }
        }

        if (!IS_NPC(ch) && !(ch->desc) && GET_POS(ch) > POS_STUNNED && !IS_AFFECTED(ch, AFF_FROZEN)) {
            if (FIGHTING(ch)) {
                do_flee(ch, nullptr, 0, 0);
            }
        }
        /* Mobile Defense System */
        if (IS_NPC(ch) && GRAPPLED(ch) && !MOB_FLAGGED(ch, MOB_DUMMY) && rand_number(1, 5) >= 4) {
            do_escape(ch, nullptr, 0, 0);
            continue;
        }
        if (FIGHTING(ch) && IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_DUMMY)) {
            if (AFF_FLAGGED(FIGHTING(ch), AFF_FLYING) && !AFF_FLAGGED(ch, AFF_FLYING) && IS_HUMANOID(ch) &&
                GET_LEVEL(ch) > 10) {
                do_fly(ch, nullptr, 0, 0);
                continue;
            }
            if (!AFF_FLAGGED(FIGHTING(ch), AFF_FLYING) && AFF_FLAGGED(ch, AFF_FLYING)) {
                do_fly(ch, nullptr, 0, 0);
                continue;
            }
            if (AFF_FLAGGED(FIGHTING(ch), AFF_FLYING) && AFF_FLAGGED(ch, AFF_FLYING) &&
                GET_ALT(ch) < GET_ALT(FIGHTING(ch))) {
                do_fly(ch, "high", 0, 0);
                continue;
            }
            if (AFF_FLAGGED(FIGHTING(ch), AFF_FLYING) && !IS_HUMANOID(ch) && !AFF_FLAGGED(ch, AFF_FLYING) &&
                GET_POS(ch) > POS_RESTING) {
                if (rand_number(1, 30) >= 22 && !block_calc(ch)) {
                    act("$n@G flees in terror and you lose sight of $m!", true, ch, nullptr, nullptr, TO_ROOM);
                    while (ch->contents)
                        extract_obj(ch->contents);

                    extract_char(ch);
                    continue;
                }
            }
            if (AFF_FLAGGED(FIGHTING(ch), AFF_FLYING) && IS_HUMANOID(ch) && GET_LEVEL(ch) <= 10) {
                if (rand_number(1, 30) >= 22 && !block_calc(ch)) {
                    act("$n@G turns and runs away. You lose sight of $m!", true, ch, nullptr, nullptr, TO_ROOM);
                    while (ch->contents)
                        extract_obj(ch->contents);
                    extract_char(ch);
                    continue;
                }
            }
            if (GET_POS(ch) == POS_SITTING && sec_roll_check(ch) == 1) {
                do_stand(ch, nullptr, 0, 0);
                continue;
            }
            if (GET_POS(ch) == POS_RESTING && sec_roll_check(ch) == 1) {
                do_stand(ch, nullptr, 0, 0);
                continue;
            }
            if (IS_AFFECTED(ch, AFF_PARA) && IS_NPC(ch) && GET_INT(ch) + 10 < rand_number(1, 60)) {
                act("@yYou fail to overcome your paralysis!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@Y$n @ystruggles with $s paralysis!@n", true, ch, nullptr, nullptr, TO_ROOM);
                continue;
            }
            if (GET_POS(ch) == POS_SLEEPING && !AFF_FLAGGED(ch, AFF_KNOCKED) && sec_roll_check(ch) == 1) {
                do_wake(ch, nullptr, 0, 0);
                do_stand(ch, nullptr, 0, 0);
                continue;
            }
            struct char_data *vict;
            char buf[100];

            vict = FIGHTING(ch);
            sprintf(buf, "%s", GET_NAME(vict));
            if (IN_ROOM(ch) == IN_ROOM(vict) && !MOB_FLAGGED(ch, MOB_DUMMY) && !AFF_FLAGGED(ch, AFF_KNOCKED) &&
                GET_POS(ch) != POS_SITTING && GET_POS(ch) != POS_RESTING && GET_POS(ch) != POS_SLEEPING) {

                if (IS_NPC(ch) && rand_number(1, 30) <= 12)
                    continue;

                mob_attack(ch, buf);

            }//end if
            else {
                continue;
            }
        }
        if (GET_POS(ch) <= POS_RESTING && PLR_FLAGGED(ch, PLR_POWERUP)) {
            ch->playerFlags.reset(PLR_POWERUP);
        }

        if (GET_BARRIER(ch) > 0) {
            improve_skill(ch, SKILL_BARRIER, 0);
        }

        if (PLR_FLAGGED(ch, PLR_POWERUP) && rand_number(1, 3) == 3) {
            char buf3[MAX_STRING_LENGTH];
            if (GET_HIT(ch) >= (ch->getEffMaxPL()) && (ch->getCurKI()) >= GET_MAX_MANA(ch) / 20 &&
                GET_PREFERENCE(ch) != PREFERENCE_KI) {
                if ((ch->getCurKI()) >= GET_MAX_MANA(ch) * 0.5) {
                    int64_t raise = GET_MAX_MOVE(ch) * 0.02;
                    ch->incCurST(raise);
                }
                ch->restoreHealth(false);
                ch->decCurKI(ch->getMaxKI() / 20);
                dispel_ash(ch);
                act("@RYou have reached your maximum!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n stops powering up in a flash of light!@n", true, ch, nullptr, nullptr, TO_ROOM);
                send_to_sense(0, "You sense someone stop powering up", ch);
                sprintf(buf3, "@D[@GBlip@D]@r Rising Powerlevel Final@D: [@Y%s@D]", add_commas(GET_HIT(ch)).c_str());
                send_to_scouter(buf3, ch, 1, 0);
                ch->playerFlags.reset(PLR_POWERUP);
            } else if (GET_HIT(ch) >= (ch->getEffMaxPL()) && (ch->getCurKI()) >= (GET_MAX_MANA(ch) * 0.0375) + 1 &&
                       GET_PREFERENCE(ch) == PREFERENCE_KI) {
                if ((ch->getCurKI()) >= (GET_MAX_MANA(ch) * 0.0375) + 1) {
                    int64_t raise = GET_MAX_MOVE(ch) * 0.02;
                    ch->incCurST(raise);
                }
                ch->restoreHealth(false);
                ch->decCurKI((GET_MAX_MANA(ch) * 0.0375) + 1);
                dispel_ash(ch);
                act("@RYou have reached your maximum!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n stops powering up in a flash of light!@n", true, ch, nullptr, nullptr, TO_ROOM);
                send_to_sense(0, "You sense someone stop powering up", ch);
                sprintf(buf3, "@D[@GBlip@D]@r Rising Powerlevel Final@D: [@Y%s@D]", add_commas(GET_HIT(ch)).c_str());
                send_to_scouter(buf3, ch, 1, 0);
                ch->playerFlags.reset(PLR_POWERUP);
            }
            if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 20 && GET_PREFERENCE(ch) != PREFERENCE_KI) {
                ch->decCurKI(ch->getMaxKI() / 20);
                act("@RYou have run out of ki.@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n stops powering up in a flash of light!@n", true, ch, nullptr, nullptr, TO_ROOM);
                send_to_sense(0, "You sense someone stop powering up", ch);
                sprintf(buf3, "@D[@GBlip@D]@r Rising Powerlevel Final@D: [@Y%s@D]", add_commas(GET_HIT(ch)).c_str());
                send_to_scouter(buf3, ch, 1, 0);
                ch->playerFlags.reset(PLR_POWERUP);
            } else if ((ch->getCurKI()) < (GET_MAX_MANA(ch) * 0.0375) + 1 && GET_PREFERENCE(ch) == PREFERENCE_KI) {
                ch->decCurKI((GET_MAX_MANA(ch) * 0.0375) + 1);
                act("@RYou have run out of ki.@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n stops powering up in a flash of light!@n", true, ch, nullptr, nullptr, TO_ROOM);
                send_to_sense(0, "You sense someone stop powering up", ch);
                sprintf(buf3, "@D[@GBlip@D]@r Rising Powerlevel Final@D: [@Y%s@D]", add_commas(GET_HIT(ch)).c_str());
                send_to_scouter(buf3, ch, 1, 0);
                ch->playerFlags.reset(PLR_POWERUP);
            }
            if (GET_HIT(ch) < (ch->getEffMaxPL()) && ((GET_PREFERENCE(ch) != PREFERENCE_KI &&
                                                       (ch->getCurKI()) >= GET_MAX_MANA(ch) / 20) ||
                                                      (GET_PREFERENCE(ch) == PREFERENCE_KI &&
                                                       (ch->getCurKI()) >= (GET_MAX_MANA(ch) * 0.0375) + 1))) {
                ch->incCurHealthPercent(.1);
                if (GET_PREFERENCE(ch) != PREFERENCE_KI) {
                    ch->decCurKI(ch->getMaxKI() / 20);
                } else {
                    ch->decCurKI(ch->getMaxKI() * .0375);
                }
                if ((ch->getCurKI()) >= GET_MAX_MANA(ch) * 0.5) {
                    int64_t raise = GET_MAX_MOVE(ch) * 0.02;
                    ch->incCurST(raise);
                }
                if (GET_MAX_HIT(ch) < 50000) {
                    act("@RYou continue to powerup, as wind billows out from around you!@n", true, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@R$n continues to powerup, as wind billows out from around $m!@n", true, ch, nullptr, nullptr,
                        TO_ROOM);
                } else if (GET_MAX_HIT(ch) < 500000) {
                    act("@RYou continue to powerup, as the ground splits beneath you!@n", true, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@R$n continues to powerup, as the ground splits beneath $m!@n", true, ch, nullptr, nullptr,
                        TO_ROOM);
                } else if (GET_MAX_HIT(ch) < 5000000) {
                    act("@RYou continue to powerup, as the ground shudders and splits beneath you!@n", true, ch,
                        nullptr, nullptr, TO_CHAR);
                    act("@R$n continues to powerup, as the ground shudders and splits beneath $m!@n", true, ch, nullptr,
                        nullptr, TO_ROOM);
                } else if (GET_MAX_HIT(ch) < 50000000) {
                    act("@RYou continue to powerup, as a huge depression forms beneath you!@n", true, ch, nullptr,
                        nullptr, TO_CHAR);
                    act("@R$n continues to powerup, as a huge depression forms beneath $m!@n", true, ch, nullptr,
                        nullptr, TO_ROOM);
                } else if (GET_MAX_HIT(ch) < 100000000) {
                    act("@RYou continue to powerup, as the entire area quakes around you!@n", true, ch, nullptr,
                        nullptr, TO_CHAR);
                    act("@R$n continues to powerup, as the entire area quakes around $m!@n", true, ch, nullptr, nullptr,
                        TO_ROOM);
                } else if (GET_MAX_HIT(ch) < 300000000) {
                    act("@RYou continue to powerup, as huge chunks of ground are ripped apart beneath you!@n", true, ch,
                        nullptr, nullptr, TO_CHAR);
                    act("@R$n continues to powerup, as huge chunks of ground are ripped apart beanth $m!@n", true, ch,
                        nullptr, nullptr, TO_ROOM);
                } else {
                    act("@RYou continue to powerup, as the very air around you crackles and burns!@n", true, ch,
                        nullptr, nullptr, TO_CHAR);
                    act("@R$n continues to powerup, as the very air around $m crackles and burns!@n", true, ch, nullptr,
                        nullptr, TO_ROOM);
                }
                send_to_sense(0, "You sense someone powering up", ch);
                send_to_worlds(ch);
                sprintf(buf3, "@D[@GBlip@D]@r Rising Powerlevel Detected@D: [@Y%s@D]", add_commas(GET_HIT(ch)).c_str());
                send_to_scouter(buf3, ch, 1, 0);
                dispel_ash(ch);
            }
        }
        if ((GET_POS(ch) == POS_SLEEPING || GET_POS(ch) == POS_RESTING) &&
            (PLR_FLAGGED(ch, PLR_CHARGE) || GET_CHARGE(ch) >= 1)) {
            send_to_char(ch, "You stop charging and release all your pent up energy!\r\n");
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
            ch->incCurKI(GET_CHARGE(ch));
            GET_CHARGE(ch) = 0;
            GET_CHARGETO(ch) = 0;
        }
        if (PLR_FLAGGED(ch, PLR_CHARGE) && GET_BONUS(ch, BONUS_UNFOCUSED) > 0 && rand_number(1, 80) >= 70) {
            send_to_char(ch, "You lose concentration due to your unfocused mind and release your charged energy!\r\n");
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
            ch->incCurKI(GET_CHARGE(ch));
            GET_CHARGE(ch) = 0;
            GET_CHARGETO(ch) = 0;
        }
        if (GET_CHARGE(ch) >= ch->getMaxKI() / 2) {
            improve_skill(ch, SKILL_CONCENTRATION, 1);
        }
        if (!PLR_FLAGGED(ch, PLR_CHARGE) && rand_number(1, 40) >= 38 && !FIGHTING(ch) &&
            (GET_PREFERENCE(ch) != PREFERENCE_KI || GET_CHARGE(ch) > GET_MAX_MANA(ch) * 0.1)) {
            if (GET_CHARGE(ch) >= GET_MAX_MANA(ch) / 100) {
                int64_t loss = 0;
                send_to_char(ch, "You lose some of your energy slowly.\r\n");
                switch (rand_number(1, 3)) {
                    case 1:
                        act("$n@w's aura flickers weakly.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    case 2:
                        act("$n@w's aura sheds energy.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                    case 3:
                        act("$n@w's aura flickers brightly before growing dimmer.@n", true, ch, nullptr, nullptr,
                            TO_ROOM);
                        break;
                    default:
                        act("$n@w's aura shrinks some.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        break;
                }
                loss = GET_CHARGE(ch) / 20;
                GET_CHARGE(ch) -= loss;
            } else if (GET_CHARGE(ch) < GET_MAX_MANA(ch) / 100 && GET_CHARGE(ch) != 0) {
                send_to_char(ch, "Your charged energy is completely gone as your aura fades.\r\n");
                act("$n@w's aura fades away dimmly.@n", true, ch, nullptr, nullptr, TO_ROOM);
                GET_CHARGE(ch) = 0;
            }
        }
        if (PLR_FLAGGED(ch, PLR_CHARGE)) {
            if ((GET_SKILL(ch, SKILL_CONCENTRATION) > 74)) {
                perc = 10;
            } else if ((GET_SKILL(ch, SKILL_CONCENTRATION) > 49)) {
                perc = 5;
            } else if ((GET_SKILL(ch, SKILL_CONCENTRATION) > 24)) {
                perc = 2;
            } else {
                perc = 1;
            }
            if (IS_TRUFFLE(ch) && perc == 10) {
                perc += 10;
            }
            if (IS_TRUFFLE(ch) && perc == 5) {
                perc += 5;
            }
            if (IS_TRUFFLE(ch) && perc == 2) {
                perc += 3;
            }
            if (IS_TRUFFLE(ch) && perc == 1) {
                perc += 1;
            }
            if (perc > 1 && GET_PREFERENCE(ch) == PREFERENCE_H2H) {
                perc = perc * 0.5;
            }
        }
        if (PLR_FLAGGED(ch, PLR_CHARGE)) {
            if ((GET_SKILL(ch, SKILL_CONCENTRATION) > 74)) {
                perc = 10;
            } else if ((GET_SKILL(ch, SKILL_CONCENTRATION) > 49)) {
                perc = 5;
            } else if ((GET_SKILL(ch, SKILL_CONCENTRATION) > 24)) {
                perc = 2;
            } else {
                perc = 1;
            }
            if (IS_MUTANT(ch) && perc == 10) {
                perc -= 1;
            }
            if (IS_MUTANT(ch) && perc == 5) {
                perc -= 1;
            }
            if (IS_MUTANT(ch) && perc == 2) {
                perc -= 1;
            }
            if (perc > 1 && GET_PREFERENCE(ch) == PREFERENCE_H2H) {
                perc = perc * 0.5;
            }
            if ((ch->getCurKI()) <= 0) {
                send_to_char(ch, "You can not charge anymore, you have charged all your energy!\r\n");
                act("$n@w's aura grows calm.@n", true, ch, nullptr, nullptr, TO_ROOM);
                ch->playerFlags.reset(PLR_CHARGE);
            } else if (((GET_MAX_MANA(ch) * 0.01) * perc) >= (ch->getCurKI())) {
                send_to_char(ch, "You have charged the last that you can.\r\n");
                act("$n@w's aura @Yflashes@w spectacularly, rushing upwards in torrents!@n", true, ch, nullptr, nullptr,
                    TO_ROOM);
                GET_CHARGE(ch) += (ch->getCurKI());
                ch->decCurKIPercent(1);
                GET_CHARGETO(ch) = 0;
                ch->playerFlags.reset(PLR_CHARGE);
            } else {
                if (GET_CHARGE(ch) >= GET_CHARGETO(ch)) {
                    send_to_char(ch, "You have already reached the maximum that you wished to charge.\r\n");
                    act("$n@w's aura burns steadily.@n", true, ch, nullptr, nullptr, TO_ROOM);
                    GET_CHARGETO(ch) = 0;
                    ch->playerFlags.reset(PLR_CHARGE);
                } else if (GET_CHARGE(ch) + (((GET_MAX_MANA(ch) * 0.01) * perc) + 1) >= GET_CHARGETO(ch)) {
                    ch->decCurKI(GET_CHARGETO(ch) - GET_CHARGE(ch));
                    GET_CHARGE(ch) = GET_CHARGETO(ch);
                    send_to_char(ch, "You stop charging as you reach the maximum that you wished to charge.\r\n");
                    act("$n@w's aura flares up brightly and then burns steadily.@n", true, ch, nullptr, nullptr,
                        TO_ROOM);
                    GET_CHARGETO(ch) = 0;
                    ch->playerFlags.reset(PLR_CHARGE);
                } else {
                    ch->decCurKI(((GET_MAX_MANA(ch) * 0.01) * perc) + 1);
                    GET_CHARGE(ch) += ((GET_MAX_MANA(ch) * 0.01) * perc) + 1;
                    switch (rand_number(1, 3)) {
                        case 1:
                            act("$n@w's aura ripples magnificantly while growing brighter!@n", true, ch, nullptr,
                                nullptr, TO_ROOM);
                            send_to_char(ch, "Your aura grows bright as you charge more ki.\r\n");
                            break;
                        case 2:
                            act("$n@w's aura ripples with power as it grows larger!@n", true, ch, nullptr, nullptr,
                                TO_ROOM);
                            send_to_char(ch, "Your aura ripples with power as you charge more ki.\r\n");
                            break;
                        case 3:
                            act("$n@w's aura throws sparks off violently!.@n", true, ch, nullptr, nullptr, TO_ROOM);
                            send_to_char(ch, "Your aura throws sparks off violently as you charge more ki.\r\n");
                            break;
                        default:
                            break;
                    }
                    if (GET_CHARGE(ch) >= GET_CHARGETO(ch)) {
                        GET_CHARGE(ch) = GET_CHARGETO(ch);
                        GET_CHARGE(ch) += GET_LEVEL(ch);
                        send_to_char(ch, "You have finished charging!\r\n");
                        act("$n@w's aura burns brightly and then evens out.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        ch->playerFlags.reset(PLR_CHARGE);
                        GET_CHARGETO(ch) = 0;
                    }
                }
                if (GET_SKILL(ch, SKILL_CONCENTRATION)) {
                    improve_skill(ch, SKILL_CONCENTRATION, 1);
                }
            }
        }
    }
}

void appear(struct char_data *ch) {
    if (affected_by_spell(ch, SPELL_INVISIBLE))
        affect_from_char(ch, SPELL_INVISIBLE);

    for(auto f : {AFF_INVISIBLE, AFF_HIDE}) ch->affected_by.reset(f);

    act("$n slowly fades into existence.", false, ch, nullptr, nullptr, TO_ROOM);
}

void update_pos(struct char_data *victim) {
    if (AFF_FLAGGED(victim, AFF_KNOCKED)) {
        return;
    }
    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
        return;
    else if (GET_POS(victim) == POS_SITTING && FIGHTING(victim))
        return;
    else if (GET_POS(victim) == POS_SITTING && FIGHTING(victim))
        return;
    else if (GET_HIT(victim) > 0)
        GET_POS(victim) = POS_STANDING;
    else if (GET_HIT(victim) <= -11)
        GET_POS(victim) = POS_DEAD;
    else if (GET_HIT(victim) <= -6)
        GET_POS(victim) = POS_MORTALLYW;
    else if (GET_HIT(victim) <= -3)
        GET_POS(victim) = POS_INCAP;
    else
        GET_POS(victim) = POS_STUNNED;
}


static void check_killer(struct char_data *ch, struct char_data *vict) {
    if (PLR_FLAGGED(vict, PLR_KILLER) || PLR_FLAGGED(vict, PLR_THIEF))
        return;
    if (PLR_FLAGGED(ch, PLR_KILLER) || IS_NPC(ch) || IS_NPC(vict) || ch == vict)
        return;

}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict) {
    if (ch == vict)
        return;

    if (FIGHTING(ch)) {
        core_dump();
        return;
    }

    ch->next_fighting = combat_list;
    combat_list = ch;

    FIGHTING(ch) = vict;

    if (GET_POS(ch) == POS_SITTING) {
        GET_POS(ch) = POS_SITTING;
    } else if (GET_POS(ch) == POS_SLEEPING) {
        GET_POS(ch) = POS_SLEEPING;
    }

    if (!CONFIG_PK_ALLOWED)
        check_killer(ch, vict);
}


/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch) {
    struct char_data *temp;

    if (ch == next_combat_list)
        next_combat_list = ch->next_fighting;

    if (IS_NPC(ch)) {
        COMBO(ch) = -1;
        COMBHITS(ch) = 0;
    }
    REMOVE_FROM_LIST(ch, combat_list, next_fighting, temp);
    ch->next_fighting = nullptr;
    FIGHTING(ch) = nullptr;
    ch->affected_by.reset(AFF_POSITION);
    update_pos(ch);
}

static void make_pcorpse(struct char_data *ch) {

    struct obj_data *corpse;
    struct obj_data *money;
    int x, y;


    corpse = create_obj();

    corpse->vn = NOTHING;
    IN_ROOM(corpse) = NOWHERE;

    /* This handles how the corpse is viewed - Iovan */
    handle_corpse_condition(corpse, ch);

    if (AFF_FLAGGED(ch, AFF_ASHED)) {
        act("@WSome ashes fall off the corpse.@n", true, ch, nullptr, nullptr, TO_ROOM);
        struct obj_data *ashes;
        if (rand_number(1, 3) == 2) {
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
        } else if (rand_number(1, 2) == 2) {
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
        } else {
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
        }
    }

    GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
    GET_OBJ_SIZE(corpse) = get_size(ch);
    for (x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++) {
        if (x < EF_ARRAY_MAX)
            GET_OBJ_EXTRA_AR(corpse, x) = 0;
        if (y < TW_ARRAY_MAX)
            corpse->wear_flags[y] = 0;
    }

    corpse->wear_flags.set(ITEM_WEAR_TAKE);
    for(auto f : {ITEM_NODONATE, ITEM_UNIQUE_SAVE}) corpse->extra_flags.set(f);
    GET_OBJ_VAL(corpse, VAL_CONTAINER_CAPACITY) = 0;      /* You can't store stuff in a corpse */
    GET_OBJ_VAL(corpse, VAL_CONTAINER_CORPSE) = 1;        /* corpse identifier */
    GET_OBJ_VAL(corpse, VAL_CONTAINER_OWNER) = ch->id;  /* corpse identifier */
    GET_OBJ_WEIGHT(corpse) = ch->getTotalWeight();
    GET_OBJ_RENT(corpse) = 100000;
    GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;


    struct obj_data *obj, *next_obj;

    for (obj = ch->contents; obj; obj = next_obj) {
        next_obj = obj->next_content;

        if (obj && GET_OBJ_VNUM(obj) < 19900 && GET_OBJ_VNUM(obj) != 17998) {
            if ((GET_OBJ_VNUM(obj) >= 18800 && GET_OBJ_VNUM(obj) <= 18999) ||
                (GET_OBJ_VNUM(obj) >= 19100 && GET_OBJ_VNUM(obj) <= 19199)) {
                continue;
            } else {
                obj_from_char(obj);
                obj_to_obj(obj, corpse);
                continue;
            }
        } else {
            continue;
        }
    }

    /* transfer gold */
    if (GET_GOLD(ch) > 0) {
        /*
         * following 'if' clause added to fix gold duplication loophole
         * The above line apparently refers to the old "partially log in,
         * kill the game character, then finish login sequence" duping
         * bug. The duplication has been fixed (knock on wood) but the
         * test below shall live on, for a while. -gg 3/3/2002
         */
        if (IS_NPC(ch) || ch->desc) {
            money = create_money(GET_GOLD(ch));
            obj_to_obj(money, corpse);
        }
        ch->set(CharMoney::Carried, 0);
    }

    obj_to_room(corpse, IN_ROOM(ch));
}

/* This handles how corpses are viewed. How many limbs they have. If they were *
 * disintergrated, blown in half, beat to a pulp, etc.        - Iovan 3/2/2011 */
static void handle_corpse_condition(struct obj_data *corpse, struct char_data *ch) {

    char buf2[MAX_NAME_LENGTH + 128];
    char descBuf[512];

    GET_OBJ_VAL(corpse, VAL_CORPSE_HEAD) = 1;
    GET_OBJ_VAL(corpse, VAL_CORPSE_RARM) = 1;
    GET_OBJ_VAL(corpse, VAL_CORPSE_LARM) = 1;
    GET_OBJ_VAL(corpse, VAL_CORPSE_RLEG) = 1;
    GET_OBJ_VAL(corpse, VAL_CORPSE_LLEG) = 1;

    switch (GET_DEATH_TYPE(ch)) {
        case DTYPE_HEAD:
            *buf2 = '\0';
            snprintf(buf2, sizeof(buf2), "headless corpse %s", GET_NAME(ch));
            corpse->name = strdup(buf2);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The headless corpse of %s is lying here", GET_NAME(ch));
            corpse->room_description = strdup(descBuf);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The headless remains of %s's corpse", GET_NAME(ch));
            corpse->short_description = strdup(descBuf);
            GET_OBJ_VAL(corpse, VAL_CORPSE_HEAD) = 0;
            break;
        case DTYPE_HALF:
            *buf2 = '\0';
            snprintf(buf2, sizeof(buf2), "half corpse %s", GET_NAME(ch));
            corpse->name = strdup(buf2);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "Half of %s's corpse is lying here", GET_NAME(ch));
            corpse->room_description = strdup(descBuf);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "Half of %s's corpse", GET_NAME(ch));
            corpse->short_description = strdup(descBuf);
            break;
        case DTYPE_VAPOR:
            *buf2 = '\0';
            snprintf(buf2, sizeof(buf2), "burnt chunks corpse %s", GET_NAME(ch));
            corpse->name = strdup(buf2);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The burnt chunks of %s's corpse are scattered here", GET_NAME(ch));
            corpse->room_description = strdup(descBuf);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The burnt chunks of %s's corpse", GET_NAME(ch));
            corpse->short_description = strdup(descBuf);
            break;
        case DTYPE_PULP:
            *buf2 = '\0';
            snprintf(buf2, sizeof(buf2), "beaten bloody corpse %s", GET_NAME(ch));
            corpse->name = strdup(buf2);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The bloody and beaten corpse of %s is lying here", GET_NAME(ch));
            corpse->room_description = strdup(descBuf);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The bloody and beaten remains of %s's corpse", GET_NAME(ch));
            corpse->short_description = strdup(descBuf);
            break;
        default:
            snprintf(buf2, sizeof(buf2), "corpse %s", GET_NAME(ch));
            corpse->name = strdup(buf2);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "The corpse of %s is lying here", GET_NAME(ch));
            corpse->room_description = strdup(descBuf);

            *descBuf = '\0';
            snprintf(descBuf, sizeof(descBuf), "the remains of %s's corpse", GET_NAME(ch));
            corpse->short_description = strdup(descBuf);
            break;
    }

    if (!IS_NPC(ch)) { /* Let's set the corpse's limbs */
        if (GET_LIMBCOND(ch, 0) <= 0) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_RARM) = 0;
        } else if (GET_LIMBCOND(ch, 0) > 0 && GET_LIMBCOND(ch, 0) < 50) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_RARM) = 2;
        }
        if (GET_LIMBCOND(ch, 1) <= 0) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_LARM) = 0;
        } else if (GET_LIMBCOND(ch, 1) > 0 && GET_LIMBCOND(ch, 1) < 50) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_LARM) = 2;
        }
        if (GET_LIMBCOND(ch, 2) <= 0) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_RLEG) = 0;
        } else if (GET_LIMBCOND(ch, 2) > 0 && GET_LIMBCOND(ch, 2) < 50) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_RLEG) = 2;
        }
        if (GET_LIMBCOND(ch, 3) <= 0) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_LLEG) = 0;
        } else if (GET_LIMBCOND(ch, 3) > 0 && GET_LIMBCOND(ch, 3) < 50) {
            GET_OBJ_VAL(corpse, VAL_CORPSE_LLEG) = 2;
        }
        return;
    } else { /* Do nothing else! */
        return;
    }
}

static void make_corpse(struct char_data *ch, struct char_data *tch) {
    struct obj_data *corpse, *o;
    struct obj_data *money;
    struct obj_data *obj, *next_obj;
    int i, x, y;

    corpse = create_obj();

    corpse->vn = NOTHING;
    IN_ROOM(corpse) = NOWHERE;

    /* This handles how the corpse is viewed - Iovan */
    handle_corpse_condition(corpse, ch);

    if (AFF_FLAGGED(ch, AFF_ASHED)) {
        act("@WSome ashes fall off the corpse.@n", true, ch, nullptr, nullptr, TO_ROOM);
        struct obj_data *ashes;
        if (rand_number(1, 3) == 2) {
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
        } else if (rand_number(1, 2) == 2) {
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
        } else {
            ashes = read_object(1305, VIRTUAL);
            obj_to_room(ashes, IN_ROOM(ch));
        }
    }

    /* Let's have a chance to give animals meat */
    if (tch != nullptr) {
        if (!IS_NPC(tch) && GET_SKILL(tch, SKILL_SURVIVAL)) {
            int skill = GET_SKILL(tch, SKILL_SURVIVAL);
            if (!IS_HUMANOID(ch) && PRF_FLAGGED(tch, PRF_CARVE) && axion_dice(0) < skill) {
                bool chance = false;
                int repeats = 0;

                if (skill < 25) {
                    chance = true;
                }
                else if (skill < 50) {
                    repeats = 1;
                }
                else if (skill < 75) {
                    chance = true;
                    repeats = 1;
                }
                else if (skill < 100) {
                    repeats = 2;
                }
                else if (skill == 100) {
                    chance = true;
                    repeats = 2;
                }

                if (chance && rand_number(1, 10) > 5) {
                    repeats += 1;
                }

                for (int ind = 0; ind < repeats; ind++) {
                    struct obj_data *meat;
                    send_to_char(tch, "The choice edible meat is preserved because of your skill.\r\n");
                    meat = read_object(1612, VIRTUAL);
                    obj_to_char(meat, ch);
                    char nick[MAX_INPUT_LENGTH], nick2[MAX_INPUT_LENGTH], nick3[MAX_INPUT_LENGTH];
                    sprintf(nick, "@RRaw %s@R Steak@n", GET_NAME(ch));
                    sprintf(nick2, "Raw %s Steak", ch->name);
                    sprintf(nick3, "@wA @Rraw %s@R steak@w is lying here@n", GET_NAME(ch));
                    meat->short_description = strdup(nick);
                    meat->name = strdup(nick2);
                    meat->room_description = strdup(nick3);
                    GET_OBJ_MATERIAL(meat) = 14;
                }

            }
        }
    }

    GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
    GET_OBJ_SIZE(corpse) = get_size(ch);
    for (x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++) {
        if (x < EF_ARRAY_MAX)
            GET_OBJ_EXTRA_AR(corpse, x) = 0;
        if (y < TW_ARRAY_MAX)
            corpse->wear_flags[y] = 0;
    }

    corpse->wear_flags.set(ITEM_WEAR_TAKE);
    for(auto f : {ITEM_NODONATE, ITEM_UNIQUE_SAVE}) corpse->extra_flags.set(f);
    GET_OBJ_VAL(corpse, VAL_CONTAINER_CAPACITY) = 0;    /* You can't store stuff in a corpse */
    GET_OBJ_VAL(corpse, VAL_CONTAINER_CORPSE) = 1;    /* corpse identifier */
    GET_OBJ_VAL(corpse, VAL_CONTAINER_OWNER) = -1;    /* corpse identifier */
    GET_OBJ_WEIGHT(corpse) = ch->getTotalWeight();
    GET_OBJ_RENT(corpse) = 100000;
    if (IS_NPC(ch))
        GET_OBJ_TIMER(corpse) = CONFIG_MAX_NPC_CORPSE_TIME;
    else
        GET_OBJ_TIMER(corpse) = rand_number(CONFIG_MAX_PC_CORPSE_TIME / 2, CONFIG_MAX_PC_CORPSE_TIME);

    if (MOB_FLAGGED(ch, MOB_HUSK)) {
        for (obj = ch->contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            obj_from_char(obj);
            extract_obj(obj);
        }
    }

    if (!MOB_FLAGGED(ch, MOB_HUSK)) {
        /* transfer character's inventory to the corpse */
        corpse->contents = ch->contents;
        for (o = corpse->contents; o != nullptr; o = o->next_content) {
            o->in_obj = corpse;
        }
        object_list_new_owner(corpse, nullptr);

        /* transfer character's equipment to the corpse */
        int eqdrop = false;
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                remove_otrigger(GET_EQ(ch, i), ch);
                obj_to_obj(unequip_char(ch, i), corpse);
                eqdrop = true;
            }
    }
    /* transfer gold */
    if (GET_GOLD(ch) > 0 && !MOB_FLAGGED(ch, MOB_HUSK)) {
        /*
         * following 'if' clause added to fix gold duplication loophole
         * The above line apparently refers to the old "partially log in,
         * kill the game character, then finish login sequence" duping
         * bug. The duplication has been fixed (knock on wood) but the
         * test below shall live on, for a while. -gg 3/3/2002
         */
        if (IS_NPC(ch) || ch->desc) {
            money = create_money(GET_GOLD(ch));
            obj_to_obj(money, corpse);
        }
        ch->set(CharMoney::Carried, 0);
    }
    if (!MOB_FLAGGED(ch, MOB_HUSK)) {
        ch->contents = nullptr;
    }
    obj_to_room(corpse, IN_ROOM(ch));

}


void loadmap(struct char_data *ch) {

}


/* When ch kills victim */
static void change_alignment(struct char_data *ch, struct char_data *victim) {
    /*
     * If you kill a monster with alignment A, you move 1/20th of the way to
     * having alignment -A.
     * Ethical alignments of killer and victim make this faster or slower.
     */

    /*if (GET_ALIGNMENT(ch) < -1000) {
      GET_ALIGNMENT(ch) = -1000;
     }
     if (GET_ALIGNMENT(ch) > 1000) {
      GET_ALIGNMENT(ch) = 1000;
     }*/
}


void death_cry(struct char_data *ch) {
    int door;
    for (door = 0; door < NUM_OF_DIRS; door++)
        if (CAN_GO(ch, door))
            send_to_room(ch->getRoom()->dir_option[door]->to_room,
                         "Your blood freezes as you hear someone's death cry.\r\n");
}

/* Let's clean up necessary things after "death" */
static void final_combat_resolve(struct char_data *ch) {
    struct obj_data *chair;

    if (SITS(ch)) {
        chair = SITS(ch);
        SITS(ch) = nullptr;
        SITTING(chair) = nullptr;
    }
    if (!IS_NPC(ch) && !ch->clones.empty()) {
        auto clones = ch->clones;
        for(auto &c : clones) {
            handle_multi_merge(c);
        }
    }
    if (CARRYING(ch)) {
        carry_drop(ch, 2);
    }
    if (CARRIED_BY(ch)) {
        carry_drop(CARRIED_BY(ch), 2);
    }
    if (DRAGGING(ch)) {
        DRAGGED(DRAGGING(ch)) = nullptr;
        DRAGGING(ch) = nullptr;
    }
    if (DRAGGED(ch)) {
        DRAGGING(DRAGGED(ch)) = nullptr;
        DRAGGED(ch) = nullptr;
    }
    if (GRAPPLING(ch)) {
        GRAPTYPE(GRAPPLING(ch)) = -1;
        GRAPPLED(GRAPPLING(ch)) = nullptr;
        GRAPPLING(ch) = nullptr;
        GRAPTYPE(ch) = -1;
    }
    if (GRAPPLED(ch)) {
        GRAPTYPE(GRAPPLED(ch)) = -1;
        GRAPPLING(GRAPPLED(ch)) = nullptr;
        GRAPPLED(ch) = nullptr;
        GRAPTYPE(ch) = -1;
    }
    if (BLOCKED(ch)) {
        BLOCKS(BLOCKED(ch)) = nullptr;
        BLOCKED(ch) = nullptr;
    }
    if (BLOCKS(ch)) {
        BLOCKED(BLOCKS(ch)) = nullptr;
        BLOCKS(ch) = nullptr;
    }
    if (ABSORBING(ch)) {
        ABSORBBY(ABSORBING(ch)) = nullptr;
        ABSORBING(ch) = nullptr;
    }
    if (ABSORBBY(ch)) {
        ABSORBING(ABSORBBY(ch)) = nullptr;
        ABSORBBY(ch) = nullptr;
    }

}

enum DeathType : uint8_t {
    Afterlife = 0,
    Northran = 1,
    Past = 2,
    Newbie = 3
};

void raw_kill(struct char_data *ch, struct char_data *killer) {
    struct char_data *k, *temp;

    if (FIGHTING(ch))
        stop_fighting(ch);

    while (ch->affected)
        affect_remove(ch, ch->affected);

    while (ch->affectedv)
        affectv_remove(ch, ch->affectedv);

    /* To make ordinary commands work in scripts.  welcor*/
    if (GET_POS(ch) != POS_SITTING && GET_POS(ch) != POS_SLEEPING && GET_POS(ch) != POS_RESTING)
        GET_POS(ch) = POS_STANDING;

    if (killer && !IS_NPC(killer)) {
        if (!IS_NPC(killer) && !IS_NPC(ch)) {
            send_to_imm("[PK] %s killed %s at room [%d]\r\n", GET_NAME(killer), GET_NAME(ch),
                        GET_ROOM_VNUM(IN_ROOM(killer)));
        }
        if ((IS_SAIYAN(killer) && rand_number(1, 2) == 2) || !IS_SAIYAN(killer)) {
            if (rand_number(1, 6) >= 5 &&
                (level_exp(killer, GET_LEVEL(killer) + 1) - GET_EXP(killer) > 0 || GET_LEVEL(killer) == 100)) {
                int psreward = GET_WIS(killer) * 0.35;
                if (GET_LEVEL(killer) > GET_LEVEL(ch) + 5) {
                    psreward *= 0.2;
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 2) {
                    psreward *= 0.5;
                }
                if (IS_HUMAN(killer) ||
                    (IS_BIO(killer) && (GET_GENOME(killer, 0) == 1 || GET_GENOME(killer, 1) == 1))) {
                    psreward *= 1.25;
                }
                if (IS_HALFBREED(ch)) {
                    psreward -= psreward * 0.4;
                }
                if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_HUSK) && GET_PRACTICES(killer) > 50 &&
                    IS_BIO(ch)) {
                    psreward = 0;
                    send_to_char(killer, "@D[@G+0 @BPS @cCapped at 50 for Absorb@D]@n\r\n");
                } else {
                    killer->modPractices(psreward);
                    send_to_char(killer, "@D[@G+%d @BPS@D]@n\r\n", psreward);
                }
            }
        }
        if (IS_ANDROID(killer) && !IS_NPC(killer) && !PLR_FLAGGED(killer, PLR_ABSORB)) {
            if (PLR_FLAGGED(killer, PLR_REPAIR)) {
                if (GET_LEVEL(killer) > GET_LEVEL(ch) + 15) {
                    send_to_char(killer, "@D[@G+0 @mUpgrade Point @r-WEAK-@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 10) {
                    GET_UP(killer) += 3;
                    send_to_char(killer, "@D[@G+3 @mUpgrade Point@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 8) {
                    GET_UP(killer) += 6;
                    send_to_char(killer, "@D[@G+6 @mUpgrade Points@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 4) {
                    GET_UP(killer) += 12;
                    send_to_char(killer, "@D[@G+12 @mUpgrade Points@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 2) {
                    GET_UP(killer) += 16;
                    send_to_char(killer, "@D[@G+16 @mUpgrade Points@D]@n\r\n");
                } else {
                    GET_UP(killer) += 28;
                    send_to_char(killer, "@D[@G+28 @mUpgrade Points@D]@n\r\n");
                }
            } else {
                if (GET_LEVEL(killer) > GET_LEVEL(ch) + 15) {
                    send_to_char(killer, "@D[@G+0 @mUpgrade Point @r-WEAK-@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 10) {
                    GET_UP(killer) += 5;
                    send_to_char(killer, "@D[@G+5 @mUpgrade Point@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 6) {
                    GET_UP(killer) += 12;
                    send_to_char(killer, "@D[@G+12 @mUpgrade Points@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 4) {
                    GET_UP(killer) += 18;
                    send_to_char(killer, "@D[@G+18 @mUpgrade Points@D]@n\r\n");
                } else if (GET_LEVEL(killer) > GET_LEVEL(ch) + 2) {
                    GET_UP(killer) += 28;
                    send_to_char(killer, "@D[@G+28 @mUpgrade Points@D]@n\r\n");
                } else {
                    GET_UP(killer) += 36;
                    send_to_char(killer, "@D[@G+36 @mUpgrade Points@D]@n\r\n");
                }
            }
        }
        if (death_mtrigger(ch, killer))
            death_cry(ch);
    } else
        death_cry(ch);

    update_pos(ch);

    if (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_DUMMY)) {
        int shadowed = false;
        ch->decCurHealthPercent(1);
        if (IS_SHADOW_DRAGON1(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON1 = -1;
            send_to_room(IN_ROOM(ch), "@YThe one star dragon ball falls to the ground!@n\r\n");

            obj = read_object(20, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        } else if (IS_SHADOW_DRAGON2(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON2 = -1;
            send_to_room(IN_ROOM(ch), "@YThe two star dragon ball falls to the ground!@n\r\n");

            obj = read_object(21, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        } else if (IS_SHADOW_DRAGON3(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON3 = -1;
            send_to_room(IN_ROOM(ch), "@YThe three star dragon ball falls to the ground!@n\r\n");

            obj = read_object(22, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        } else if (IS_SHADOW_DRAGON4(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON4 = -1;
            send_to_room(IN_ROOM(ch), "@YThe four star dragon ball falls to the ground!@n\r\n");

            obj = read_object(23, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        } else if (IS_SHADOW_DRAGON5(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON5 = -1;
            send_to_room(IN_ROOM(ch), "@YThe five star dragon ball falls to the ground!@n\r\n");

            obj = read_object(24, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        } else if (IS_SHADOW_DRAGON6(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON6 = -1;
            send_to_room(IN_ROOM(ch), "@YThe six star dragon ball falls to the ground!@n\r\n");

            obj = read_object(25, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        } else if (IS_SHADOW_DRAGON7(ch)) {
            struct obj_data *obj = nullptr;
            SHADOW_DRAGON7 = -1;
            send_to_room(IN_ROOM(ch), "@YThe seven star dragon ball falls to the ground!@n\r\n");

            obj = read_object(26, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));
            shadowed = true;
        }
        make_corpse(ch, killer);
        purge_homing(ch);
        extract_char(ch);
        if (shadowed == true) {
            shadow_dragons_live();
        }
    } else if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_DUMMY)) {
        ch->decCurHealthPercent(1);
        extract_char(ch);
    } else {
        if (!AFF_FLAGGED(ch, AFF_SPIRIT) && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_PAST) &&
            (GET_ROOM_VNUM(IN_ROOM(ch)) < 17900 || GET_ROOM_VNUM(IN_ROOM(ch)) > 17999)) {
            if (!PLR_FLAGGED(ch, PLR_ABSORBED)) {
                make_pcorpse(ch);
                loadmap(ch);
            } else {
                ch->playerFlags.reset(PLR_ABSORBED);
            }
        }
        final_combat_resolve(ch);
        if (FIGHTING(ch))
            stop_fighting(ch);

        for (k = combat_list; k; k = temp) {
            temp = k->next_fighting;
            if (FIGHTING(k) == ch)
                stop_fighting(k);
        }

        bool android_lose = true;
        DeathType death_type = Afterlife;
        if (ch->in_past())
            death_type = Past;
        else if (ch->in_northran())
            death_type = Northran;
        else if (ch->is_newbie())
            death_type = Newbie;


        switch (death_type) {
            case Afterlife:
                ch->ghostify();
                purge_homing(ch);
                if (GET_LEVEL(ch) > 0 && has_group(ch)) {
                    if (ch->master != nullptr) {
                        group_bonus(ch, 1);
                    } else {
                        group_bonus(ch, 0);
                    }
                }
                ch->teleport_to(6000);
                break;
            case Northran:
                ch->restore(false);
                ch->teleport_to(17900);
                android_lose = false;
                send_to_char(ch, "You wake up and realise that you didn't die, how or why are a mystery.\r\n");
                break;
            case Past:
                ch->restore(false);
                ch->teleport_to(1561);
                android_lose = false;
                send_to_char(ch, "You wake up and realise that you died, but only in your mind.\r\n");
                break;
            case Newbie:
                ch->restore(false);
                ch->teleport_to(sensei::getStartRoom(ch->chclass));
                send_to_char(ch, "\r\n@RYou should beware, when you reach level 9, you will actually die. So you\r\n"
                                 "should learn to be more careful. Since when you die past that point and\r\n"
                                 "actually reach the afterlife you need to realise that being revived will\r\n"
                                 "not be very easy. So treat your character's dying with as much care as\r\n"
                                 "possible.@n\r\n");
                break;
        }

        if (!IS_NPC(ch)) {
            if (IS_ANDROID(ch) && !PLR_FLAGGED(ch, PLR_ABSORB) && android_lose && GET_UP(ch) > 5) {
                int loss = GET_UP(ch) / 5;
                GET_UP(ch) -= loss;
                send_to_char(ch, "@rYou lose @R%s@r upgrade points!@n\r\n", add_commas(loss).c_str());
            }
            Crash_delete_crashfile(ch);
            ch->save();
        }
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

void die(struct char_data *ch, struct char_data *killer) {
    if (!IS_NPC(ch)) {
        ch->playerFlags.reset(PLR_HEALT);
        if ((IS_MAJIN(ch) || IS_BIO(ch)) &&
            ((ch->getCurLF()) >= (ch->getMaxLF()) * 0.75 || (PLR_FLAGGED(ch, PLR_SELFD2) &&
                                                             (ch->getCurLF()) >= (ch->getMaxLF()) * 0.5))) {
            ch->decCurLFPercent(2, -1);
            ch->decCurHealthPercent(1, 1);
            ch->playerFlags.set(PLR_GOOP);
            ch->gooptime = 32;
            return;
        }
        if (PLR_FLAGGED(ch, PLR_IMMORTAL)) {
            act("@c$n@w disappears right before dying. $n appears to be immortal.@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@c$n@w disappears right before dying. $n appears to be immortal.@n.", true, ch, nullptr, nullptr,
                TO_ROOM);
            ch->decCurHealthPercent(1, 1);
            ch->decCurKIPercent(1, 1);
            ch->decCurSTPercent(1, 1);
            null_affect(ch, AFF_POISON);
            if (GET_COND(ch, HUNGER) >= 0) {
                GET_COND(ch, HUNGER) = 48;
            }
            if (GET_COND(ch, THIRST) >= 0) {
                GET_COND(ch, THIRST) = 48;
            }
            if (FIGHTING(ch)) {
                stop_fighting(ch);
            }
            GET_POS(ch) = POS_SITTING;
            ch->teleport_to(sensei::getStartRoom(ch->chclass));
            return;
        }
        for(auto f : {PLR_KILLER, PLR_THIEF}) ch->playerFlags.reset(f);
        for(auto f : {AFF_KNOCKED, AFF_SLEEP, AFF_PARALYZE}) ch->affected_by.reset(f);
        if (!AFF_FLAGGED(ch, AFF_SPIRIT) && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_PAST) && GET_LEVEL(ch) > 8) {
            if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 2002 && GET_ROOM_VNUM(IN_ROOM(ch)) <= 2011) {
                GET_DTIME(ch) = time(nullptr);
            } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_AL) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_HELL)) {
                send_to_char(ch, "Your soul is saved from destruction by King Yemma. Why? Who knows.\r\n");
            } else if (IN_ARENA(ch)) {
                cleanup_arena_watch(ch);
                if (killer != nullptr) {
                    cleanup_arena_watch(killer);
                    send_to_all("@R%s@r manages to defeat @R%s@r in the Arena!@n\r\n", GET_NAME(killer), GET_NAME(ch));
                    final_combat_resolve(killer);
                    final_combat_resolve(ch);
                    killer->teleport_to(17875);
                } else {
                    send_to_all("@R%s@r dies in the water of the Arena and is disqualified!@n\r\n", GET_NAME(ch));
                }
                char_from_room(ch);
                char_to_room(ch, real_room(17875));
                ch->decCurHealthPercent(1, 1);
                look_at_room(IN_ROOM(ch), ch, 0);
                final_combat_resolve(ch);
                return;
            } else {
                if (killer != nullptr && IS_NPC(killer)) {
                    GET_DTIME(ch) = time(nullptr) + 28800;
                    GET_DCOUNT(ch) += 1;
                } else if (killer != nullptr && !IS_NPC(killer)) {
                    GET_DTIME(ch) = time(nullptr) + 1123200;
                    ch->playerFlags.set(PLR_PDEATH);
                    GET_DCOUNT(ch) += 1;
                } else {
                    if (GET_DCOUNT(ch) <= 0) {
                        GET_DTIME(ch) = time(nullptr) + 28800;
                    } else if (GET_DCOUNT(ch) <= 1) {
                        GET_DTIME(ch) = time(nullptr) + 43200;
                    } else if (GET_DCOUNT(ch) <= 3) {
                        GET_DTIME(ch) = time(nullptr) + 86400;
                    } else if (GET_DCOUNT(ch) <= 5) {
                        GET_DTIME(ch) = time(nullptr) + 172800;
                    } else if (GET_DCOUNT(ch) > 5) {
                        GET_DTIME(ch) = time(nullptr) + 604800;
                    }
                    GET_DCOUNT(ch) += 1;
                }
            }
            if (GET_COND(ch, HUNGER) >= 0) {
                GET_COND(ch, HUNGER) = 48;
            }
            if (GET_COND(ch, THIRST) >= 0) {
                GET_COND(ch, THIRST) = 48;
            }
        }
    }


    raw_kill(ch, killer);
}

static void perform_group_gain(struct char_data *ch, int base, struct char_data *victim) {
    int64_t share;

    if (IN_ARENA(ch)) {
        return;
    }
    auto leader = ch->master ? ch->master : ch;

    /*share = MIN(CONFIG_MAX_EXP_GAIN, MAX(1, base * GET_LEVEL(ch)));*/
    share = MIN(2000000, base * GET_LEVEL(ch));
    if (!IS_NPC(ch)) {
        if (GET_LEVEL(ch) >= 100 && GET_MAX_HIT(ch) * .025 >= GET_MAX_HIT(victim)) {
            share *= .05;
        } else if (GET_MAX_HIT(ch) * .025 >= GET_MAX_HIT(victim)) {
            share = 1;
        } else if (GET_MAX_HIT(ch) * .05 >= GET_MAX_HIT(victim)) {
            share *= .05;
        } else if (GET_MAX_HIT(ch) * .1 >= GET_MAX_HIT(victim)) {
            share *= .1;
        } else if (GET_MAX_HIT(ch) * .15 >= GET_MAX_HIT(victim)) {
            share *= .15;
        } else if (GET_MAX_HIT(ch) * .25 >= GET_MAX_HIT(victim)) {
            share *= .25;
        } else if (GET_MAX_HIT(ch) * .5 >= GET_MAX_HIT(victim)) {
            share *= .5;
        } else if (GET_MAX_HIT(ch) * .9 >= GET_MAX_HIT(victim)) {
            share *= .65;
        } else if (GET_MAX_HIT(ch) >= GET_MAX_HIT(victim)) {
            share *= .7;
        }
    }
    if (LASTHIT(victim) != 0 && LASTHIT(victim) != GET_IDNUM(ch)) {
        struct follow_type *f;
        int checkit = false;
        for (f = ch->followers; f; f = f->next) {
            if (AFF_FLAGGED(f->follower, AFF_GROUP) && LASTHIT(victim) == GET_IDNUM(f->follower)) {
                checkit = true;
            }
        }
        if (checkit == false && ch->master != nullptr && GET_IDNUM(ch->master) == LASTHIT(victim)) {
            checkit = true;
        }
        if (checkit == false && ch->master != nullptr) {
            struct char_data *master = ch->master;
            for (f = master->followers; f; f = f->next) {
                if (f->follower != ch) {
                    if (AFF_FLAGGED(f->follower, AFF_GROUP) && LASTHIT(victim) == GET_IDNUM(f->follower)) {
                        checkit = true;
                    }
                }
            }
        }
        if (checkit == false) {
            send_to_char(ch, "@RYou didn't do most of the work for this kill.@n\r\n");
            share = 1;
        }
    }
    if (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_HUSK)) {
        share /= 10;
    }
    if (GET_BONUS(ch, BONUS_PRODIGY) > 0) {
        share = share + (share * .25);
    }
    if (IS_SAIYAN(ch)) {
        share = share + (share * .50);
    }
    if (IS_HALFBREED(ch)) {
        share = share + (share * .40);
    }
    if (IS_ICER(ch)) {
        share = share - (share * .20);
    }
    if (GET_BONUS(ch, BONUS_LOYAL) > 0 && ch->master != nullptr) {
        share += share * 0.2;
    }
    if (ch->master != nullptr && ch->master != ch) {
        share += share * 0.15;
    }
    if (MOB_FLAGGED(victim, MOB_KNOWKAIO)) {
        share += share * .25;
    }
    ch->mod(CharNum::GroupKills, 1);
    if ((GET_GROUPKILLS(ch) + 1) / 20 > share * 0.16) {
        share += share * 0.16;
    } else {
        share += (share * 0.02) * ((GET_GROUPKILLS(ch) + 1) / 20);
    }
    if (group_bonus(ch, 2) == 2) {
        send_to_char(ch, "You receive a bonus from your group's leader! @D[@G+2 PS!@D]@n\r\n");
        ch->modPractices(2);
    } else if (group_bonus(ch, 2) == 3) {
        send_to_char(ch, "You receive a bonus from your group's leader! @D[@G+5%s Exp!@D]@n\r\n", "%");
        share += share * 0.05;
    } else if (group_bonus(ch, 2) == 5) {
        ch->incCurKIPercent(.04);
        send_to_char(ch, "You receive a bonus from your group's leader! @D[@G4%s Ki Regenerated!@D]@n\r\n", "%");
    } else if (group_bonus(ch, 2) == 6) {
        ch->incCurKIPercent(.02);
        ch->incCurSTPercent(.02);
        ch->incCurHealthPercent(.02);
        send_to_char(ch, "You receive a bonus from your group's leader! @D[@G2%s PL/ST/Ki Regenerated!@D]@n\r\n", "%");
    } else if (group_bonus(ch, 2) == 7) {
        if (IS_ANDROID(ch)) {
            if (PLR_FLAGGED(leader, PLR_ABSORB)) {
                ch->incCurKIPercent(.02);
                ch->incCurSTPercent(.02);
                send_to_char(ch, "You receive a bonus from your group's leader! @D[@G2%s PL/ST/Ki Recovered!@D]@n\r\n",
                             "%");
            } else if (PLR_FLAGGED(leader, PLR_REPAIR)) {
                ch->incCurHealthPercent(.02);
                send_to_char(ch, "You receive a bonus from your group's leader! @D[@G5%s PL Repaired@D]@n\r\n", "%");
            } else if (PLR_FLAGGED(leader, PLR_SENSEM) && !PLR_FLAGGED(ch, PLR_ABSORB)) {
                GET_UP(ch) += 5;
                send_to_char(ch, "You receive a bonus from your group's leader! @D[@G+5 @mUpgrade Points@D]@n\r\n");
            }
        } else {
            ch->incCurHealthPercent(.01);
            ch->incCurKIPercent(.01);
            ch->incCurSTPercent(.01);
        }
    } else if (group_bonus(ch, 2) == 11) {
        ch->incCurSTPercent(.04);
        send_to_char(ch, "You receive a bonus from your group's leader! @D[@G4%s ST Regenerated!@D]@n\r\n", "%");
    } else if (group_bonus(ch, 2) == 13) {
        if (GET_PHASE(leader) == 1) {
            share += share * 0.05;
            send_to_char(ch, "You receive a bonus from your group's leader! @D[@G+5%s Exp!@D]@n\r\n", "%");
        } else if (GET_PHASE(leader) == 2) {
            share += share * 0.1;
            send_to_char(ch, "You receive a bonus from your group's leader! @D[@G+10%s Exp!@D]@n\r\n", "%");
        }
    }
    share = gear_exp(ch, share);
    if (share > 1)
        send_to_char(ch, "You receive your share of experience -- %s points.\r\n", add_commas(share).c_str());
    else
        send_to_char(ch, "You receive your share of experience -- one measly little point!\r\n");

    ch->modExperience(share);
    /*change_alignment(ch, victim);*/
}

void group_gain(struct char_data *ch, struct char_data *victim) {
    int tot_levels, tot_members;
    int64_t tot_gain, base;
    struct char_data *k;
    struct follow_type *f;

    if (!(k = ch->master))
        k = ch;

    if (AFF_FLAGGED(k, AFF_GROUP) && (IN_ROOM(k) == IN_ROOM(ch))) {
        tot_levels = GET_LEVEL(k);
        tot_members = 1;
    } else {
        tot_levels = 0;
        tot_members = 0;
    }

    for (f = k->followers; f; f = f->next)
        if (AFF_FLAGGED(f->follower, AFF_GROUP) && IN_ROOM(f->follower) == IN_ROOM(ch)) {
            if (!IS_WEIGHTED(f->follower)) {
                tot_levels += GET_LEVEL(f->follower);
                tot_members++;
            } else if ((f->follower->getEffMaxPL()) >= (ch->getEffMaxPL()) * 0.5) {
                tot_levels += GET_LEVEL(f->follower);
                tot_members++;
            }
        }

    if (tot_members == 1 || IN_ARENA(ch)) {
        solo_gain(ch, victim);
        return;
    }

    /* round up to the next highest tot_members */
    tot_gain = (GET_EXP(victim)) + tot_members - 1;

    /* prevent illegal xp creation when killing players */
    if (!IS_NPC(victim))
        tot_gain = MIN(CONFIG_MAX_EXP_LOSS * 2, tot_gain);


    if (tot_levels >= 1) {
        base = MAX(1, tot_gain / tot_levels);
        int perc = 20 * tot_members;
        if (perc >= 80) {
            perc = 60;
        }
        base += (base / 100) * perc;
    } else
        base = 0;

    /*
    if (AFF_FLAGGED(k, AFF_GROUP) && IN_ROOM(k) == IN_ROOM(ch)) {
     if (!IS_WEIGHTED(k)) {
      perform_group_gain(k, base, victim);
     } else if (k != ch && (k->getEffMaxPL()) >= (ch->getEffMaxPL()) * 0.5) {
      perform_group_gain(k, base, victim);
     } else if (k == ch && (k->getEffMaxPL()) >= GET_MAX_HIT(ch) * 0.5) {
      perform_group_gain(k, base, victim);
     } else {
      if (k == ch) {
       send_to_char(ch, "You can not group gain while your powerlevel is weighted down more than half of your max.\r\n");
      } else {
       send_to_char(ch, "You can not group gain while your powerlevel is weighted down more than half of the leader's adjusted powerlevel.\r\n");
      }
     }
    }
     */
    perform_group_gain(k, base, victim);

    for (f = k->followers; f; f = f->next) {
        if (AFF_FLAGGED(f->follower, AFF_GROUP) && IN_ROOM(f->follower) == IN_ROOM(ch)) {
            //if ((f->follower->getEffMaxPL()) >= GET_MAX_HIT(ch) * 0.5)
            perform_group_gain(f->follower, base, victim);
        }
    }
}


void solo_gain(struct char_data *ch, struct char_data *victim) {
    return; // disabled for now.

    if (IS_NPC(ch)) {
        if (GET_ORIGINAL(ch)) {
            ch = GET_ORIGINAL(ch);
        }
    }
    int64_t exp = MIN(2000000, GET_EXP(victim));

    exp *= 0.25;


    /* Calculate level-difference penalty */
    if (!IS_NPC(ch)) {
        if (GET_LEVEL(ch) >= 100 && GET_MAX_HIT(ch) * .025 >= GET_MAX_HIT(victim)) {
            exp *= 0.04;
        } else if (GET_MAX_HIT(ch) * .025 >= GET_MAX_HIT(victim)) {
            exp = 1;
        } else if (GET_MAX_HIT(ch) * .05 >= GET_MAX_HIT(victim)) {
            exp *= 0.05;
        } else if (GET_MAX_HIT(ch) * .1 >= GET_MAX_HIT(victim)) {
            exp *= .1;
        } else if (GET_MAX_HIT(ch) * .15 >= GET_MAX_HIT(victim)) {
            exp *= .15;
        } else if (GET_MAX_HIT(ch) * .25 >= GET_MAX_HIT(victim)) {
            exp *= .25;
        } else if (GET_MAX_HIT(ch) * .5 >= GET_MAX_HIT(victim)) {
            exp *= .5;
        }
    }
    if (LASTHIT(victim) != 0 && LASTHIT(victim) != GET_IDNUM(ch)) {
        send_to_char(ch, "@RYou didn't do most of the work for this victory.@n\r\n");
        exp = 1;
    }
    if (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_HUSK)) {
        exp /= 10;
    }
    if (GET_BONUS(ch, BONUS_PRODIGY) > 0) {
        exp = exp + (exp * .25);
    }
    if (IS_SAIYAN(ch)) {
        exp = exp + (exp * .50);
    }
    if (IS_HALFBREED(ch)) {
        exp = exp + (exp * .40);
    }
    if (IS_ICER(ch)) {
        exp = exp - (exp * .20);
    }
    if (MOB_FLAGGED(victim, MOB_KNOWKAIO)) {
        exp += exp * .25;
    }
    exp = gear_exp(ch, exp);
    exp = MAX(exp, 1);

    if (exp > 1)
        send_to_char(ch, "You receive %s experience points.\r\n", add_commas(exp).c_str());
    else {
        send_to_char(ch, "You receive one lousy experience point. That fight was hardly worth it...\r\n");
    }
    if (!IS_NPC(ch)) {
        ch->modExperience(exp);
    }
    if (IS_NPC(victim)) {
        victim->modExperience(-exp);
    }
    if (!IS_NPC(victim)) {
        exp = exp / 5;
        victim->modExperience(-exp);
    }
    /*change_alignment(ch, victim);*/
}
