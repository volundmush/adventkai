/* ************************************************************************
*  File: combat.c                    Part of Dragonball Advent Truth      *
*  Usage: Combat utilities and common functions for act.offensive.c and   *
*  act.attack.c                                                           *
*                                                                         *
*  All rights reserved to Iovan that are not due to anyone else.          *
*                                                                         *
*  This file was first written on 2011 and aside for a few instances only *
*  contains code written by Iovan for use with the Real Dragonball Battle *
*  System (RDBS) of the MUD Dragonball Advent Truth.                      *
************************************************************************ */

#include "dbat/combat.h"
#include "dbat/act.movement.h"
#include "dbat/act.item.h"
#include "dbat/mobact.h"
#include "dbat/fight.h"
#include "dbat/act.attack.h"
#include "dbat/dg_comm.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/act.item.h"
#include "dbat/handler.h"
#include "dbat/constants.h"
#include "dbat/genzon.h"
#include "dbat/dg_scripts.h"
#include "dbat/class.h"
#include "dbat/techniques.h"
#include "dbat/act.informative.h"

/* local functions */
void damage_weapon(struct char_data *ch, struct obj_data *obj, struct char_data *vict) {

    if (obj) {
        if (OBJ_FLAGGED(obj, ITEM_UNBREAKABLE))
            return;
    }
    int ranking = 0, material = 1;
    int64_t PL10 = 2000000000, PL9 = 2000000000, PL8 = 2000000000;

    PL10 = PL10 * 5;
    PL9 = PL9 * 4;
    PL8 = PL8 * 2;

    if (GET_HIT(vict) >= PL10) {
        ranking = 10;
    } else if (GET_HIT(vict) >= PL9) {
        ranking = 9;
    } else if (GET_HIT(vict) >= PL8) {
        ranking = 8;
    } else if (GET_HIT(vict) >= 2000000000) {
        ranking = 7;
    } else if (GET_HIT(vict) >= 500000000) {
        ranking = 6;
    } else if (GET_HIT(vict) >= 250000000) {
        ranking = 5;
    } else if (GET_HIT(vict) >= 100000000) {
        ranking = 4;
    } else if (GET_HIT(vict) >= 50000000) {
        ranking = 3;
    } else if (GET_HIT(vict) >= 25000000) {
        ranking = 2;
    } else if (GET_HIT(vict) >= 1000000) {
        ranking = 1;
    }

    switch (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL)) {
        case MATERIAL_STEEL:
            material = 4;
            break;
        case MATERIAL_IRON:
        case MATERIAL_COPPER:
        case MATERIAL_BRASS:
        case MATERIAL_METAL:
            material = 2;
            break;
        case MATERIAL_SILVER:
            material = 5;
            break;
        case MATERIAL_KACHIN:
            material = 9;
            break;
        case MATERIAL_CRYSTAL:
            material = 7;
            break;
        case MATERIAL_DIAMOND:
            material = 8;
            break;
        case MATERIAL_PAPER:
        case MATERIAL_COTTON:
        case MATERIAL_SATIN:
        case MATERIAL_SILK:
        case MATERIAL_BURLAP:
        case MATERIAL_VELVET:
        case MATERIAL_HEMP:
        case MATERIAL_WAX:
            material = 0;
            break;
        default:
            break;
    }

    int result = ranking - material;

    if (AFF_FLAGGED(ch, AFF_CURSE)) {
        result += 3;
    } else if (AFF_FLAGGED(ch, AFF_BLESS) && rand_number(1, 3) == 3) {
        if (result > 1) {
            result = 1;
        }
    } else if (AFF_FLAGGED(ch, AFF_BLESS)) {
        result = 0;
    }

    if (GET_SKILL(ch, SKILL_HANDLING) >= axion_dice(10)) {
        act("@GYour superior handling prevents @C$p@G from being damaged.@n", true, ch, obj, nullptr, TO_CHAR);
        act("@g$n's@G superior handling prevents @C$p@G from being damaged.@n", true, ch, obj, nullptr, TO_ROOM);
        result = 0;
    }

    if (result > 0) {
        GET_OBJ_VAL(obj, VAL_ALL_HEALTH) -= result;
        if (GET_OBJ_VAL(obj, VAL_ALL_HEALTH) <= 0) {
            act("@RYour @C$p@R shatters on @r$N's@R body!@n", true, ch, obj, vict, TO_CHAR);
            act("@r$n's@R @C$p@R shatters on YOUR body!@n", true, ch, obj, vict, TO_VICT);
            act("@r$n's@R @C$p@R shatters on @r$N's@R body!@n", true, ch, obj, vict, TO_NOTVICT);
            obj->extra_flags.set(ITEM_BROKEN);
            perform_remove(vict, 16);
            perform_remove(vict, 17);
        } else if (result >= 8) {
            act("@RYour @C$p@R cracks loudly from striking @r$N's@R body!@n", true, ch, obj, vict, TO_CHAR);
            act("@r$n's@R @C$p@R cracks loudly from striking YOUR body!@n", true, ch, obj, vict, TO_VICT);
            act("@r$n's@R @C$p@R cracks loudly from striking @r$N's@R body!@n", true, ch, obj, vict, TO_NOTVICT);
        } else if (result >= 6) {
            act("@RYour @C$p@R chips from striking @r$N's@R body!@n", true, ch, obj, vict, TO_CHAR);
            act("@r$n's@R @C$p@R cracks from striking YOUR body!@n", true, ch, obj, vict, TO_VICT);
            act("@r$n's@R @C$p@R cracks from striking @r$N's@R body!@n", true, ch, obj, vict, TO_NOTVICT);
        } else if (result >= 3) {
            act("@RYour @C$p@R loses a piece from striking @r$N's@R body!@n", true, ch, obj, vict, TO_CHAR);
            act("@r$n's@R @C$p@R loses a piece from striking YOUR body!@n", true, ch, obj, vict, TO_VICT);
            act("@r$n's@R @C$p@R loses a piece from striking @r$N's@R body!@n", true, ch, obj, vict, TO_NOTVICT);
        } else if (result >= 6) {
            act("@RYour @C$p@R has a nick in it from hitting @r$N's@R body!@n", true, ch, obj, vict, TO_CHAR);
            act("@r$n's@R @C$p@R has a nick in it from hitting YOUR body!@n", true, ch, obj, vict, TO_VICT);
            act("@r$n's@R @C$p@R has a nick in it from hitting @r$N's@R body!@n", true, ch, obj, vict, TO_NOTVICT);
        }
    }
}

void handle_multihit(struct char_data *ch, struct char_data *vict) {

    int perc = GET_DEX(ch), prob = GET_DEX(vict);

    /* Let's give the victim a bit of automatic favor due to "luck" */
    prob += rand_number(1, 15);
    /* Some otherwise inclined "luck" for the attacker */
    perc += rand_number(-5, 5);

    if (ch->throws >= 3) {
        ch->throws = 0;
        return;
    } else if (ch->throws == -1) {
        ch->throws = 0;
    }

    /* Racial bonuses */
    if (IS_KONATSU(ch)) {
        perc *= 1.5;
    }
    if (IS_BIO(ch) && (GET_GENOME(ch, 0) == 8 || GET_GENOME(ch, 1) == 8)) {
        perc *= 1.4;
    }

    if (IS_NPC(ch)) {
        perc -= perc * 0.3;
    }

    /* Weapons have less chance to multihit */
    if (LASTATK(ch) == -1) {
        perc *= 0.75;
    }

    int amt = 70;

    if (GET_SKILL(ch, SKILL_STYLE) >= 100) {
        amt -= amt * 0.1;
    } else if (GET_SKILL(ch, SKILL_STYLE) >= 80) {
        amt -= amt * 0.08;
    } else if (GET_SKILL(ch, SKILL_STYLE) >= 60) {
        amt -= amt * 0.06;
    } else if (GET_SKILL(ch, SKILL_STYLE) >= 40) {
        amt -= amt * 0.04;
    } else if (GET_SKILL(ch, SKILL_STYLE) >= 20) {
        amt -= amt * 0.02;
    }

    /* Critical Failure*/
    if (axion_dice(0) < amt) {
        prob += 500;
    }

    /* Success! */
    if (perc >= prob) {
        char buf[MAX_INPUT_LENGTH];
        act("@Y...in a lightning flash of speed you attack @y$N@Y again!@n", true, ch, nullptr, vict, TO_CHAR);
        act("@Y...in a lightning flash of speed @y$n@Y attacks YOU again!@n", true, ch, nullptr, vict, TO_VICT);
        act("@Y...in a lightning flash of speed @y$n@Y attacks @y$N@Y again!@n", true, ch, nullptr, vict, TO_NOTVICT);
        ch->throws += 1;
        ch->playerFlags.set(PLR_MULTIHIT);
        if (COMBO(ch) > -1) {
            switch (COMBO(ch)) {
                case 0:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_punch(ch, buf, 0, 0);
                    break;
                case 1:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_kick(ch, buf, 0, 0);
                    break;
                case 2:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_elbow(ch, buf, 0, 0);
                    break;
                case 3:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_knee(ch, buf, 0, 0);
                    break;
                case 4:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_roundhouse(ch, buf, 0, 0);
                    break;
                case 5:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_uppercut(ch, buf, 0, 0);
                    break;
                case 6:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_slam(ch, buf, 0, 0);
                    ch->throws += 1;
                    break;
                case 8:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_heeldrop(ch, buf, 0, 0);
                    ch->throws += 1;
                    break;
                case 51:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_bash(ch, buf, 0, 0);
                    ch->throws += 1;
                    break;
                case 52:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_head(ch, buf, 0, 0);
                    break;
                case 56:
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_tailwhip(ch, buf, 0, 0);
                    ch->throws += 1;
                    break;
            }
        } else {
            if (LASTATK(ch) == -1) {
                sprintf(buf, "%s", GET_NAME(vict));
                do_attack(ch, buf, 0, 0);
            } else {
                if (rand_number(1, 3) == 2 && GET_SKILL(ch, SKILL_KICK) > 0) {
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_kick(ch, buf, 0, 0);
                } else {
                    sprintf(buf, "%s", GET_NAME(vict));
                    do_punch(ch, buf, 0, 0);
                }
            }
        }
    }

}

int handle_defender(struct char_data *vict, struct char_data *ch) {

    int result = false;

    if (GET_DEFENDER(vict)) {
        struct char_data *def = GET_DEFENDER(vict);
        int64_t defnum = (GET_SPEEDI(def) * 0.01) * rand_number(-10, 10);
        int64_t chnum = (GET_SPEEDI(ch) * 0.01) * rand_number(-5, 10);
        if (GET_SPEEDI(def) + defnum > GET_SPEEDI(ch) + chnum && IN_ROOM(def) == IN_ROOM(vict) &&
            GET_POS(def) > POS_SITTING) {
            act("@YYou move to and manage to intercept the attack aimed at @y$N@Y!@n", true, def, nullptr, vict,
                TO_CHAR);
            act("@y$n@Y moves to and manages to intercept the attack aimed at YOU!@n", true, def, nullptr, vict,
                TO_VICT);
            act("@y$n@Y moves to and manages to intercept the attack aimed at @y$N@Y!@n", true, def, nullptr, vict,
                TO_NOTVICT);
            result = true;
        } else if (IN_ROOM(def) == IN_ROOM(vict) && GET_POS(def) > POS_SITTING) {
            act("@YYou move to intercept the attack aimed at @y$N@Y, but just not fast enough!@n", true, def, nullptr,
                vict, TO_CHAR);
            act("@y$n@Y moves to intercept the attack aimed at YOU, but $e wasn't fast enough!@n", true, def, nullptr,
                vict, TO_VICT);
            act("@y$n@Y moves to intercept the attack aimed at @y$N@Y, but $e wasn't fast enough!@n", true, def,
                nullptr, vict, TO_NOTVICT);
        }
    }

    return (result);
}

void handle_disarm(struct char_data *ch, struct char_data *vict) {

    int roll1 = rand_number(-10, 10), roll2 = rand_number(-10, 10), handled = false;
    roll1 += GET_STR(ch) + GET_DEX(ch);
    roll2 += GET_STR(vict) + GET_DEX(vict);

    if (!IS_NPC(ch)) {
        if (PLR_FLAGGED(ch, PLR_THANDW)) {
            roll1 += 5;
        }
    }

    if (rand_number(1, 100) <= 50 && !IS_KONATSU(ch)) {
        roll1 = -500;
    } else if (rand_number(1, 100) <= 75) {
        roll1 *= 1.5;
    }

    if (IS_KONATSU(vict)) {
        roll1 *= 0.75;
    }

    if (GET_SKILL(ch, SKILL_HANDLING) >= axion_dice(10)) {
        handled = true;
    }

    if (roll1 < roll2) {
        struct obj_data *obj;
        if (GET_EQ(ch, WEAR_WIELD1) && handled == false) {
            obj = GET_EQ(ch, WEAR_WIELD1);
            act("@y$N@Y manages to disarm you! The @w$p@Y falls from your grasp!@n", true, ch, obj, vict, TO_CHAR);
            act("@y$N@Y manages to disarm @R$n@Y! The @w$p@Y falls from $s grasp!@n", true, ch, obj, vict, TO_NOTVICT);
            act("@YYou manage to disarm @R$n@Y! The @w$p@Y falls from $s grasp!@n", true, ch, obj, vict, TO_VICT);
            perform_remove(ch, 16);
            if (GET_OBJ_VNUM(obj) != 20098) {
                obj_from_char(obj);
                obj_to_room(obj, IN_ROOM(ch));
            }
        } else if (GET_EQ(ch, WEAR_WIELD1) && handled == true) {
            obj = GET_EQ(ch, WEAR_WIELD1);
            act("@y$N@Y almosts disarms you, but your handling of @w$p@Y saves it from slipping from your grasp!@n",
                true, ch, obj, vict, TO_CHAR);
            act("@y$N@Y almost disarms @R$n@Y, but $s handling of @w$p@Y saves it from slipping from $s grasp!@n", true,
                ch, obj, vict, TO_NOTVICT);
            act("@YYou almost disarm @R$n@Y, but $s handling of @w$p@Y saves it from slipping from $s grasp!@n", true,
                ch, obj, vict, TO_VICT);
        } else if (GET_EQ(ch, WEAR_WIELD2) && handled == true) {
            obj = GET_EQ(ch, WEAR_WIELD2);
            act("@y$N@Y almosts disarms you, but your handling of @w$p@Y saves it from slipping from your grasp!@n",
                true, ch, obj, vict, TO_CHAR);
            act("@y$N@Y almost disarms @R$n@Y, but $s handling of @w$p@Y saves it from slipping from $s grasp!@n", true,
                ch, obj, vict, TO_NOTVICT);
            act("@YYou almost disarm @R$n@Y, but $s handling of @w$p@Y saves it from slipping from $s grasp!@n", true,
                ch, obj, vict, TO_VICT);
        } else if (GET_EQ(ch, WEAR_WIELD2)) {
            obj = GET_EQ(ch, WEAR_WIELD2);
            act("@y$N@Y manages to disarm you! The @w$p@Y falls from your grasp!@n", true, ch, obj, vict, TO_CHAR);
            act("@y$N@Y manages to disarm @R$n@Y! The @w$p@Y falls from $s grasp!@n", true, ch, obj, vict, TO_NOTVICT);
            act("@YYou manage to disarm @R$n@Y! The @w$p@Y falls from $s grasp!@n", true, ch, obj, vict, TO_VICT);
            perform_remove(ch, 17);
            if (GET_OBJ_VNUM(obj) != 20098) {
                obj_from_char(obj);
                obj_to_room(obj, IN_ROOM(ch));
            }
        }
    }

}

void combine_attacks(struct char_data *ch, struct char_data *vict) {

    struct follow_type *f;
    char chbuf[MAX_INPUT_LENGTH], victbuf[MAX_INPUT_LENGTH], rmbuf[MAX_INPUT_LENGTH];
    int64_t bonus = 0;
    double maxki = 0.0;
    int totalmem = 1;
    int attspd = 0, blockable = true, same = true, attsk = 0, attavg = GET_SKILL(ch, attack_skills[GET_COMBINE(ch)]);
    int burn = false, shocked = false;

    switch (GET_COMBINE(ch)) {
        case 0: /* Kamehameha */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You cup your hands at your sides and a ball of @Benergy@W forms there. You chant @B'@CKaaaaameeeehaaaameeee@B'@W and then fire a @RKamehameha@W wave at @r$N@W while screaming @B'@CHAAAAAAAAAAAAAAAAAAAAA!@B'@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W cups $s hands be $s side and chants @B'@CKaaaameeeehaaaameee@B'@W. A ball of energy forms in $s hands and he quickly brings them forward and fires a @RKamehameha @Wat @rYOU@W while screaming @B'@CHAAAAAAAAAAAAAAAAAAAAA!@B'@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W cups $s hands be $s side and chants @B'@CKaaaameeeehaaaameee@B'@W. A ball of energy forms in $s hands and he quickly brings them forward and fires a @RKamehameha @Wat @r$N@W while screaming @B'@CHAAAAAAAAAAAAAAAAAAAAA!@B'@n@n");
            maxki = 0.15;
            attspd += 2;
            bonus += GET_MAX_MOVE(ch) * 0.02;
            break;
        case 1: /* Galik Gun */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You throw your hands forward and launch a purple beam of energy at @r$N@n while shouting @B'@mGalik Gun!@B'@W");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W throws $s arms forward and launches a purple beam at @rYOU@W while shouting @B'@mGalik Gun!@B'@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W throws $s arms forward and launches a purple beam at @r$N@W while shouting @B'@mGalik Gun!@B'@n");
            maxki = 0.15;
            attspd += 1;
            bonus += GET_MAX_MANA(ch) * 0.5;
            break;
        case 2: /* Masenko */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You raise your hands above your head with one resting atop the other and begin to pour your charged energy to that point. As soon as the energy is ready you shout @B'@RMasenko Ha!@B'@W and bringing your hands down you launch a bright reddish orange beam at @r$N@W!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W raises $s hands above $s head and energy quicly pools there. Suddenly $e brings $s hands down and shouts @B'@RMasenko Ha!@B'@W as a bright reddish orange beam launches toward @rYOU!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W raises $s hands above $s head and energy quicly pools there. Suddenly $e brings $s hands down and shouts @B'@RMasenko Ha!@B'@W as a bright reddish orange beam launches toward @r$N!@n");
            maxki = 0.15;
            attspd += 1;
            bonus += GET_MAX_MANA(ch) * 0.5;
            break;
        case 3: /* Deathbeam */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! With a quick motion you point at @r$N@W and launch a lightning fast @MDeathbeam@W!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! With a quick motion $e points $s finger at @rYOU@W and launches a lightning fast @MDeathbeam@W!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! With a quick motion $e points $s finger at @r$N@W and launches a lightning fast @MDeathbeam@W!@n");
            maxki = 0.1;
            attspd += 4;
            break;
        case 4: /* Honoo */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! With your energy ready you breath out toward @r$N@W jets of incredibly hot flames in the form of a deadly @rHonoo@W!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! Sudden jets of flame burst forth from $s mouth at @RYOU@W in the form of a deadly @rHonoo@W!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! Sudden jets of flame burst forth from $s mouth at @R$N@W in the form of a deadly @rHonoo@W!@n");
            maxki = 0.125;
            attspd += 2;
            burn = true;
            break;
        case 5: /* Twin Slash */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! With your energy prepared you poor it into your blade and accelerate your body to incredible speeds toward @r$N! You leave two glowing green marks behind on $S body in a single instant as your @GTwin Slash@W hits!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! Raising $s sword @Y$n@W accelerates toward @rYOU@W with incredible speed! Two glowing green slashes are left on YOUR body from $s successful @GTwin Slash@W!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! Raising $s sword @Y$n@W accelerates toward @r$N@W with incredible speed! Two glowing green slashes are left on @R$N's@W body from the successful @GTwin Slash@W!@n");
            maxki = 0.125;
            attspd += 2;
            blockable = false;
            break;
        case 6: /* Hell Flash */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You stick one of each of your hands in your armpits and detach them. With your hands detached your point the exposed arm cannons at @r$N@W and launch a massive @RHell Flash@W at $M!");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W sticks one of each of $s hands in $s armpits and detaches them there. With the hands detached $e aims $s exposed arm cannons at @RYOU@W and launches a massive @RHell Flash@W!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W sticks one of each of $s hands in $s armpits and detaches them there. With the hands detached $e aims $s exposed arm cannons at @R$N@W and launches a massive @RHell Flash@W!@n");
            maxki = 0.2;
            attspd += 1;
            break;
        case 7: /* Psychic Blast */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! With your energy ready you look at @R$N@W as the blue light of your @CPsychic Blast@W launches from your head toward $S!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! A blue light, identifying a @CPsychic Blast@W, launches from @Y$n's@W toward @RYOUR HEAD@W!");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! A blue light, identifying a @CPsychic Blast@W, launches from @Y$n's@W toward @R$N's@W head!");
            maxki = 0.125;
            attspd += 1;
            shocked = true;
            break;
        case 8: /* Crusher Ball */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! Pooling your energy you form a large ball of red energy above an upraised palm. Slamming your other hand into it you launch it toward @r$N@W while shouting @B'@RCrusher Ball@B'@W!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W raises a palm above his head and red energy begins to pool there. As the energy completes the formation of a ball @Y$n@W slams $s other hand into it and launches it at @rYOU@W while shouting @B'@RCrusher Ball@B'@W!");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W raises a palm above his head and red energy begins to pool there. As the energy completes the formation of a ball @Y$n@W slams $s other hand into it and launches it at @r$N@W while shouting @B'@RCrusher Ball@B'@W!");
            maxki = 0.2;
            break;
        case 9: /* Water Spikes */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! Using your energy to form a ball of water between your hands you then raise the ball above your head. Several spiked of ice form from the ball of water and you hurl them at @r$N@W!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! Forming a ball of water between $s palms with $s energy @Y$n@W then raises the ball of water above $s head. Suddenly several spikes of ice form from the water and $e launches them at @rYOU@W!");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! Forming a ball of water between $s palms with $s energy @Y$n@W then raises the ball of water above $s head. Suddenly several spikes of ice form from the water and $e launches them at @r$N@W!");
            maxki = 0.14;
            break;
        case 10: /* Tribeam */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You form a triangle with your hands and aim the center of the triangle at @r$N@W. With the sudden shout @B'@YTribeam@B'@W you release your prepared energy at $M in the form of a beam!");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W forms a triangle with $s hands and aims the center at @rYOU@W! With the sudden shout @B'@YTribeam@B'@W a large beam of energy flashes toward @rYOU!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W forms a triangle with $s hands and aims the center at @r$N@W! With the sudden shout @B'@YTribeam@B'@W a large beam of energy flashes toward @r$N!@n");
            maxki = 0.2;
            attspd += 2;
            bonus += GET_MAX_HIT(ch) * 0.5;
            break;
        case 11: /* Starbreaker */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group, you call out to your allies to launch a group attack! You raise your right hand above your head as dark red energy begins to pool in your slightly cupped hand, while purple arcs of electricity flow up your left arm. Slamming both hands together, you shout @B'@YStarbreaker@B'@W and release your prepared energy at $M in the form of a ball!@n");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W raises $s right hand, pooling dark red energy in the palm. @Y$n@W slams both their hands together, shouting @B'@YStarbreaker@B'@W, a ball of energy flashes toward @rYOU!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W raises their right hand above their head, pooling dark red energy. @Y$n@W slams both their hands together, shouting @B'@YStarbreaker@B'@W, a ball of energy flashes toward @r$N!@n");
            maxki = 0.2;
            bonus += GET_MAX_MANA(ch) * 0.6;
            break;
        case 12: /* Seishou Enko */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You open your mouth and aim at @r$N@W. You grunt as you release your prepared energy at $M in the form of a beam!");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W opens $s mouth and aims at @rYOUW! With the sudden grunt, a large beam flashes towards @rYOU@n!");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W opens $s mouth and aims at @rYOUW! With the sudden grunt, a large beam flashes toward @r$N@n!");
            maxki = 0.125;
            attspd += 35;
            break;
        case 13: /* Renzokou Energy Dan */
            sprintf(chbuf,
                    "@WPositioning yourself in the center of your group you call out to your allies to launch a group attack! You slam your hands together and aim at @r$n@W. With the sudden shout @B'@YRenzoku Energy Dan@B'@W you release your prepared energy in the form of hundreds of ki blasts!");
            sprintf(victbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @rYOU@W! @Y$n@W slams both $s hands together and aims at @rYOUW! With the sudden shout @B'@YRenzoku Energy Dan@B'@W hundreds of ki blasts flash towards @rYOU!@n");
            sprintf(rmbuf,
                    "@Y$n@W calls out to $s allies to launch a group attack against @r$N@W! @Y$n@W slams $s hands together and aims at @r$N@W! With the sudden shout @B'@YRenzoku Energy Dan@B'@W hundreds of ki blasts flash towards @r$N!@n");
            maxki = 0.125;
            attspd += 6;
            break;
        default:
            send_to_imm("ERROR: Combine attacks failure for: %s", GET_NAME(ch));
            send_to_char(ch, "An error has been logged. Be patient while waiting for Iovan's response.\r\n");
            return;
    }

    int64_t totki = 0;

    act(chbuf, true, ch, nullptr, vict, TO_CHAR);
    act(victbuf, true, ch, nullptr, vict, TO_VICT);
    act(rmbuf, true, ch, nullptr, vict, TO_NOTVICT);

    if (GET_CHARGE(ch) >= GET_MAX_MANA(ch) * maxki) {
        totki += GET_MAX_MANA(ch) * maxki;
        GET_CHARGE(ch) -= GET_MAX_MANA(ch) * maxki;
    } else {
        totki += GET_CHARGE(ch);
        GET_CHARGE(ch) = 0;
    }

    for (f = ch->followers; f; f = f->next) {
        if (!AFF_FLAGGED(f->follower, AFF_GROUP)) {
            continue;
        } else {
            if (GET_COMBINE(f->follower) != GET_COMBINE(ch)) {
                same = false;
            }
            if (GET_CHARGE(f->follower) >= GET_MAX_MANA(f->follower) * maxki) {
                totki += GET_MAX_MANA(f->follower) * maxki;
                GET_CHARGE(f->follower) -= GET_MAX_MANA(f->follower) * maxki;
            } else {
                totki += GET_CHARGE(f->follower);
                GET_CHARGE(f->follower) = 0;
            }
            totalmem += 1;
            attavg += GET_SKILL(f->follower, attack_skills[GET_COMBINE(f->follower)]);
            char folbuf[MAX_INPUT_LENGTH], folbuf2[MAX_INPUT_LENGTH];
            sprintf(folbuf, "@Y$n@W times and merges $s @B'@R%s@B'@W into the group attack!@n",
                    attack_names_comp[GET_COMBINE(f->follower)]);
            sprintf(folbuf2, "@WYou time and merge your @B'@R%s@B'@W into the group attack!@n",
                    attack_names_comp[GET_COMBINE(f->follower)]);
            act(folbuf, true, f->follower, nullptr, nullptr, TO_ROOM);
            act(folbuf2, true, f->follower, nullptr, nullptr, TO_CHAR);
        }
    }

    totki += bonus;
    if (same == true) {
        totki += bonus;
    }
    attsk = attavg / totalmem;

    if (GET_COMBINE(ch) != 5) {
        if (attspd + attsk < GET_SKILL(vict, SKILL_DODGE) + (GET_CHA(ch) / 10)) {
            act("@GYou manage to dodge nimbly through the combined attack of your enemies!@n", true, vict, nullptr,
                nullptr, TO_CHAR);
            act("@r$n@G manages to dodge nimbly through the combined attack!@n", true, vict, nullptr, nullptr, TO_ROOM);
            return;
        } else if (blockable == true && attspd + attsk < GET_SKILL(vict, SKILL_BLOCK) + (GET_STR(ch) / 10)) {
            act("@GYou manage to effectivly block the combined attack of your enemies with the help of your great strength!@n",
                true, vict, nullptr, nullptr, TO_CHAR);
            act("@r$n@G manages to dodge nimbly through the combined attack!@n", true, vict, nullptr, nullptr, TO_ROOM);
            return;
        }
    }
    if (burn == true) {
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
    }
    if (shocked == true) {
        if (!AFF_FLAGGED(vict, AFF_SHOCKED) && rand_number(1, 4) == 4 && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
            act("@MYour mind has been shocked!@n", true, vict, nullptr, nullptr, TO_CHAR);
            act("@M$n@m's mind has been shocked!@n", true, vict, nullptr, nullptr, TO_ROOM);
            vict->affected_by.set(AFF_SHOCKED);
        }
    }
    hurt(0, 0, ch, vict, nullptr, totki, 1);
    if (same == true) {
        for (f = ch->followers; f; f = f->next) {
            send_to_char(f->follower, "@YS@yy@Yn@ye@Yr@yg@Yi@ys@Yt@yi@Yc @yB@Yo@yn@Yu@ys@Y!@n\r\n");
        }
        send_to_char(ch, "@YS@yy@Yn@ye@Yr@yg@Yi@ys@Yt@yi@Yc @yB@Yo@yn@Yu@ys@Y!@n\r\n");
    }
}

int check_ruby(struct char_data *ch) {

    auto isHotRuby = [](const auto&o) {return o->vn == 6600 && OBJ_FLAGGED(o, ITEM_HOT);};
    auto ruby = ch->findObject(isHotRuby);

    if (ruby) {
        act("@RYour $p@R flares up and disappears. Your fire attack has been aided!@n", true, ch, ruby, nullptr,
            TO_CHAR);
        act("@R$n's@R $p@R flares up and disappears!@n", true, ch, ruby, nullptr, TO_ROOM);
        extract_obj(ruby);
        return (1);
    } else {
        return (0);
    }

}

int64_t combo_damage(struct char_data *ch, int64_t damage, int type) {
    int64_t bonus = 0;

    if (type == 0) { /* Not a finish */
        int hits = COMBHITS(ch);

        if (hits >= 30) {
            bonus += damage * (hits * 0.15);
            bonus += damage * 12;
        } else if (hits >= 20) {
            bonus = damage * (hits * 0.1);
            bonus += damage * 10;
        } else if (hits >= 10) {
            bonus = damage * (hits * 0.1);
            bonus += damage * 5;
        } else if (hits >= 6) {
            bonus = damage * (hits * 0.1);
            bonus += damage * 1.5;
        } else if (hits >= 2) {
            bonus = damage * (hits * 0.05);
            bonus += damage * 0.2;
        }
    } else if (type == 1) {
        bonus = damage * 15;
    }

    return (bonus);
}


/* For getting into a better combat position */
int roll_balance(struct char_data *ch) {

    int chance = 0;

    if (IS_NPC(ch)) {
        if (GET_LEVEL(ch) >= 100) {
            chance = rand_number(80, 100);
        } else if (GET_LEVEL(ch) >= 80) {
            chance = rand_number(75, 90);
        } else if (GET_LEVEL(ch) >= 70) {
            chance = rand_number(70, 80);
        } else if (GET_LEVEL(ch) >= 60) {
            chance = rand_number(65, 75);
        } else if (GET_LEVEL(ch) >= 50) {
            chance = rand_number(50, 60);
        }
    } else {
        if (GET_SKILL(ch, SKILL_BALANCE) > 50) {
            chance = GET_SKILL(ch, SKILL_BALANCE);
        }
    }

    return (chance);

}

void handle_knockdown(struct char_data *ch) {
    int chance = 0;

    if (IS_NPC(ch)) {
        if (GET_LEVEL(ch) >= 100) {
            chance = rand_number(35, 45);
        } else if (GET_LEVEL(ch) >= 90) {
            chance = rand_number(25, 35);
        } else if (GET_LEVEL(ch) >= 70) {
            chance = rand_number(15, 25);
        } else if (GET_LEVEL(ch) >= 50) {
            chance = rand_number(10, 15);
        } else if (GET_LEVEL(ch) >= 30) {
            chance = rand_number(5, 10);
        }
    } else {
        chance = GET_SKILL(ch, SKILL_BALANCE) * 0.5;
    }

    if (chance > axion_dice(0)) {
        act("@mYou are @GALMOST@m knocked off your feet, but your great balance helps you keep your footing!@n", true,
            ch, nullptr, nullptr, TO_CHAR);
        act("@W$n@m is @GALMOST@m knocked off $s feet, but $s great balance helps $m keep $s footing!@n", true, ch,
            nullptr, nullptr, TO_ROOM);
    } else {
        act("@mYou are knocked off your feet!@n", true, ch, nullptr, nullptr, TO_CHAR);
        act("@W$n@m is knocked off $s feet!@n", true, ch, nullptr, nullptr, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
    }

}

int boom_headshot(struct char_data *ch) {

    int skill = GET_SKILL_BASE(ch, SKILL_TWOHAND);

    if (skill >= 100 && rand_number(1, 5) >= 3)
        return (1);
    else if (skill < 100 && skill >= 75 && rand_number(1, 5) == 5)
        return (1);
    else if (skill < 75 && skill >= 50 && rand_number(1, 6) == 6)
        return (1);
    else
        return (0);

}

int64_t gun_dam(struct char_data *ch, int wlvl) {
    int64_t dmg = 100;

    switch (wlvl) {
        case 1:
            dmg = 50;
            break;
        case 2:
            dmg = 200;
            break;
        case 3:
            dmg = 750;
            break;
        case 4:
            dmg = 2000;
            break;
        case 5:
            dmg = 7500;
            break;
    }

    if (GET_SKILL(ch, SKILL_GUN) >= 100)
        dmg *= 2;
    else if (GET_SKILL(ch, SKILL_GUN) >= 50)
        dmg += dmg * 0.5;

    int64_t dmg_prior = 0;

    dmg_prior = (dmg * GET_DEX(ch)) * ((GET_LEVEL(ch) / 5) + 1);

    if (dmg_prior <= GET_MAX_HIT(ch) * 0.4)
        dmg = dmg_prior;
    else
        dmg = GET_MAX_HIT(ch) * 0.4;

    return (dmg);
}

void club_stamina(struct char_data *ch, struct char_data *vict, int wlvl, int64_t dmg) {

    double drain = 0.0;
    int64_t drained = 0;

    switch (wlvl) {
        case 1:
            drain = 0.05;
            break;
        case 2:
            drain = 0.1;
            break;
        case 3:
            drain = 0.15;
            break;
        case 4:
            drain = 0.2;
            break;
        case 5:
            drain = 0.25;
            break;
    }

    if (GET_SKILL(ch, SKILL_CLUB) >= 100)
        drain += 0.1;
    else if (GET_SKILL(ch, SKILL_CLUB) >= 50)
        drain += 0.05;

    drained = dmg * drain;
    vict->decCurST(drained);

    send_to_char(ch, "@D[@YVictim's @GStamina @cLoss@W: @g%s@D]@n\r\n", add_commas(drained).c_str());
    send_to_char(vict, "@D[@rYour @GStamina @cLoss@W: @g%s@D]@n\r\n", add_commas(drained).c_str());

}

int backstab(struct char_data *ch, struct char_data *vict, int wlvl, int64_t dmg) {

    int chance = 0, roll_to_beat = rand_number(1, 100);
    double bonus = 0.0;

    if (GET_BACKSTAB_COOL(ch) > 0)
        return (0);

    switch (wlvl) {
        case 1:
            chance = 10;
            bonus += 0.5;
            break;
        case 2:
            chance = 15;
            bonus += 2;
            break;
        case 3:
            chance = 20;
            bonus += 3;
            break;
        case 4:
            chance = 25;
            bonus += 4;
            break;
        case 5:
            chance = 30;
            bonus += 4;
            break;
    }

    if (GET_BONUS(ch, BONUS_POWERHIT)) {
        bonus += 2;
    }

    if (GET_SKILL(ch, SKILL_DAGGER) >= 100)
        chance += 20;
    else if (GET_SKILL(ch, SKILL_DAGGER) >= 50)
        chance += 10;

    GET_BACKSTAB_COOL(ch) = 10;

    if (chance >= roll_to_beat) {
        int attacker_roll =
                GET_SKILL(ch, SKILL_MOVE_SILENTLY) + GET_SKILL(ch, SKILL_SPOT) + GET_DEX(ch) + rand_number(-5, 5);
        int defender_roll =
                GET_SKILL(vict, SKILL_SPOT) + GET_SKILL(vict, SKILL_LISTEN) + GET_DEX(ch) + rand_number(-5, 5);

        if (attacker_roll > defender_roll) {
            act("@RYou manage to sneak behind @r$N@R and stab $M in the back!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@RYou feel @r$n's@R dagger thrust into your back unexpectantly!@n", true, ch, nullptr, vict, TO_VICT);
            act("@r$n@R sneaks up behind @r$N@R and stabs $M in the back!@n", true, ch, nullptr, vict, TO_NOTVICT);
            dmg += dmg * bonus;
            hurt(0, 0, ch, vict, nullptr, dmg, 0);
            return (1);
        } else {
            return (0);
        }
    } else {
        return (0);
    }
}

void cut_limb(struct char_data *ch, struct char_data *vict, int wlvl, int hitspot) {

    int chance = 0, decap = 0, decapitate = false;
    int roll_to_beat = rand_number(1, 10000);

    if (wlvl == 1) {
        chance = 25;
    } else if (wlvl == 2) {
        chance = 50;
    } else if (wlvl == 3) {
        chance = 100;
        decap = 5;
    } else if (wlvl == 4) {
        chance = 200;
        decap = 10;
    } else if (wlvl == 5) {
        chance = 200;
        decap = 50;
    }

    if (GET_SKILL(ch, SKILL_SWORD) >= 100) {
        chance += 100;
    } else if (GET_SKILL(ch, SKILL_SWORD) >= 50) {
        chance += 50;
    }

    if (decap >= roll_to_beat && hitspot == 4) {
        decapitate = true;
    } else if (chance < roll_to_beat) {
        return;
    }

    if (GET_HIT(vict) <= 1) {
        return;
    }

    if (decapitate == true) {
        act("@R$N's@r head is cut off in the attack!@n", true, ch, nullptr, vict, TO_CHAR);
        act("@RYOUR head is cut off in the attack!@n", true, ch, nullptr, vict, TO_VICT);
        act("@R$N's@rhead is cut off in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);

        remove_limb(vict, 0);
        die(vict, ch);
        if (AFF_FLAGGED(ch, AFF_GROUP)) {
            group_gain(ch, vict);
        } else {
            solo_gain(ch, vict);
        }
        char corp[256];
        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
            sprintf(corp, "all.zenni corpse");
            do_get(ch, corp, 0, 0);
        }
        if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
            sprintf(corp, "all corpse");
            do_get(ch, corp, 0, 0);
        }
        return;
    } else { /* We've only succeeded in removing a limb. */
        if (!IS_NPC(vict)) {
            if (HAS_ARMS(vict) && rand_number(1, 2) == 2) {
                if (GET_LIMBCOND(vict, 1) > 0) {
                    GET_LIMBCOND(vict, 1) = 0;
                    vict->playerFlags.reset(PLR_CLARM);
                    act("@R$N@r loses $s left arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your left arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s left arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    remove_limb(vict, 2);
                } else if (GET_LIMBCOND(vict, 0) > 0) {
                    GET_LIMBCOND(vict, 0) = 100;
                    vict->playerFlags.reset(PLR_CRARM);
                    act("@R$N@r loses $s right arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your right arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s right arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    remove_limb(vict, 1);
                }
            } else { /* It's a leg */
                if (GET_LIMBCOND(vict, 3) > 0) {
                    GET_LIMBCOND(vict, 3) = 100;
                    vict->playerFlags.reset(PLR_CLLEG);
                    act("@R$N@r loses $s left leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your left leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s left leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    remove_limb(vict, 4);
                } else if (GET_LIMBCOND(vict, 2) > 0) {
                    GET_LIMBCOND(vict, 2) = 100;
                    vict->playerFlags.reset(PLR_CRLEG);
                    act("@R$N@r loses $s right leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your right leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s right leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    remove_limb(vict, 3);
                }
            }
        } else { /* It's a npc */
            if (HAS_ARMS(vict) && rand_number(1, 2) == 2) {
                if (MOB_FLAGGED(vict, MOB_LARM)) {
                    vict->mobFlags.reset(MOB_LARM);
                    remove_limb(vict, 2);
                    act("@R$N@r loses $s left arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your left arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s left arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                } else if (MOB_FLAGGED(vict, MOB_RARM)) {
                    vict->mobFlags.reset(MOB_RARM);
                    remove_limb(vict, 1);
                    act("@R$N@r loses $s right arm!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your right arm!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s right arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                }
            } else {
                if (MOB_FLAGGED(vict, MOB_LLEG)) {
                    vict->mobFlags.reset(MOB_LLEG);
                    remove_limb(vict, 4);
                    act("@R$N@r loses $s left leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your left leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s left leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                } else if (MOB_FLAGGED(vict, MOB_RLEG)) {
                    vict->mobFlags.reset(MOB_RLEG);
                    remove_limb(vict, 3);
                    act("@R$N@r loses $s right leg!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@RYOU lose your right leg!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r loses $s right leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                }
            }
        }
    }

}

int count_physical(struct char_data *ch) {
    int count = 0;

    if (GET_SKILL(ch, SKILL_PUNCH) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_KICK) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_KNEE) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_ELBOW) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_ROUNDHOUSE) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_SLAM) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_UPPERCUT) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_TAILWHIP) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_BASH) >= 1) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_HEADBUTT) >= 1) {
        count += 1;
    }

    return (count);
}

int physical_mastery(struct char_data *ch) {

    int count = 22;

    if (GET_SKILL(ch, SKILL_PUNCH) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_KICK) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_KNEE) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_ELBOW) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_ROUNDHOUSE) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_SLAM) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_UPPERCUT) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_TAILWHIP) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_BASH) >= 100) {
        count += 1;
    }
    if (GET_SKILL(ch, SKILL_HEADBUTT) >= 100) {
        count += 1;
    }

    if (count == 26)
        count += 1;
    else if (count >= 27)
        count += 2;

    return (count);

}

int64_t advanced_energy(struct char_data *ch, int64_t dmg) {

    if (ch == nullptr) {
        return (false);
    }

    double rate = 0.00;
    int count = GET_LEVEL(ch);
    int64_t add = 0;

    if (GET_BONUS(ch, BONUS_LEECH)) {
        rate = (double) (count) * 0.2;

        if (rate > 0.00) {
            add = dmg * rate;
            if (GET_CHARGE(ch) + add > GET_MAX_MANA(ch)) {
                if (GET_CHARGE(ch) < GET_MAX_MANA(ch)) {
                    GET_CHARGE(ch) = GET_MAX_MANA(ch);
                    act("@MYou leech some of the energy away!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@m$n@M leeches some of the energy away!@n", true, ch, nullptr, nullptr, TO_ROOM);
                } else {
                    send_to_char(ch,
                                 "@MYou can't leech because there is too much charged energy for you to handle!@n\r\n");
                }
            } else {
                GET_CHARGE(ch) += add;
                act("@MYou leech some of the energy away!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@m$n@M leeches some of the energy away!@n", true, ch, nullptr, nullptr, TO_ROOM);
            }

        } /* End of rate check */

    } /* End of Leech Bonus */


    if (GET_BONUS(ch, BONUS_INTOLERANT)) {
        rate = (double) (count) * 0.2;

        if (rate > 0.00) {
            if (GET_CHARGE(ch) > 0 && rand_number(1, 100) <= 10) {
                act("@MThe attack causes your weak control to slip and you are shocked by your own charged energy!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@m$n@M suffers shock from their own charged energy!@n", true, ch, nullptr, nullptr, TO_ROOM);
                ch->decCurHealth(GET_CHARGE(ch) / 4, 1);
            }
            add = dmg * rate;
        }
    } /* End of Energy Intolerant bonus */

    return (add);
} /* End of advanced_energy function */

int roll_accuracy(struct char_data *ch, int skill, bool kiatt) {

    if (!IS_NPC(ch)) {
        if (GET_BONUS(ch, BONUS_ACCURATE)) {
            if (kiatt == true)
                skill += skill * 0.10;
            else
                skill += skill * 0.20;
        } else if (GET_BONUS(ch, BONUS_POORDEPTH)) {
            if (kiatt == true)
                skill -= skill * 0.10;
            else
                skill -= skill * 0.20;
        }
    }

    if (skill < 40) {
        skill += rand_number(3, 10);
    }

    return (skill);
}

long double calc_critical(struct char_data *ch, int loc) {

    int roll = rand_number(1, 100);
    long double multi = 1;

    if (loc == 0) { /* Head */
        if (GET_BONUS(ch, BONUS_POWERHIT)) {
            if (roll >= 15) {
                multi = 4;
            } else if (GET_BONUS(ch, BONUS_SOFT)) {
                multi = 1;
            } else {
                multi = 2;
            }
        } else if (GET_BONUS(ch, BONUS_SOFT)) {
            multi = 1;
        } else {
            multi = 2;
        }
    } else if (loc == 1) { /* Limb */
        if (GET_BONUS(ch, BONUS_SOFT)) {
            multi = 0.25;
        } else {
            multi = 0.5;
        }
    } else { /* Body*/
        if (GET_BONUS(ch, BONUS_SOFT)) {
            multi = 0.5;
        }
    }

    return (multi);
}

int roll_hitloc(struct char_data *ch, struct char_data *vict, int skill) {

    int location = 4, critmax = 1000;
    int critical = 0;

    if (IS_NPC(ch)) {
        if (GET_LEVEL(ch) > 100)
            skill = rand_number(GET_LEVEL(ch), GET_LEVEL(ch) + 10);
        else
            skill = rand_number(GET_LEVEL(ch), 100);
    }

    if (IS_DABURA(ch) && !IS_NPC(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            critmax -= 200;
    }

    critical = rand_number(80, critmax);

    if (skill >= critical) {
        location = 2;
    } else if (skill >= rand_number(50, 750)) {
        location = 2;
    } else if (skill >= rand_number(50, 350)) {
        location = 1;
    } else if (skill >= rand_number(30, 200)) {
        location = 3;
    } else {
        location = rand_number(4, 5);
    }

    if (!IS_NPC(vict)) {
        if (location == 4 && GET_LIMBCOND(vict, 0) <= 0 && GET_LIMBCOND(vict, 1) <= 0) { /* No arms */
            location = 5;
        }

        if (location == 5 && GET_LIMBCOND(vict, 2) <= 0 && GET_LIMBCOND(vict, 3) <= 0) { /* No legs */
            location = 4;
        }

        if (location == 4 && GET_LIMBCOND(vict, 0) <= 0 && GET_LIMBCOND(vict, 1) <= 0) { /* Both failed, make body */
            location = 1;
        }
    }

    if (IS_NPC(vict)) {
        if (location == 4 && !MOB_FLAGGED(vict, MOB_RARM) && !MOB_FLAGGED(vict, MOB_LARM))
            location = 5;

        if (location == 5 && !MOB_FLAGGED(vict, MOB_RLEG) && !MOB_FLAGGED(vict, MOB_LLEG))
            location = 4;

        if (location == 5 && !MOB_FLAGGED(vict, MOB_RARM) && !MOB_FLAGGED(vict, MOB_LARM))
            location = 1;
    }

    return (location);

}

int64_t armor_calc(struct char_data *ch, int64_t dmg, int type) {
    if (IS_NPC(ch))
        return (0);

    int64_t reduce = 0;

    if (GET_ARMOR(ch) < 1000) {
        reduce = GET_ARMOR(ch) * 0.5;
    } else if (GET_ARMOR(ch) < 2000) {
        reduce = GET_ARMOR(ch) * .75;
    } else if (GET_ARMOR(ch) < 5000) {
        reduce = GET_ARMOR(ch);
    } else if (GET_ARMOR(ch) < 10000) {
        reduce = GET_ARMOR(ch) * 2;
    } else if (GET_ARMOR(ch) < 20000) {
        reduce = GET_ARMOR(ch) * 4;
    } else if (GET_ARMOR(ch) < 30000) {
        reduce = GET_ARMOR(ch) * 8;
    } else if (GET_ARMOR(ch) < 40000) {
        reduce = GET_ARMOR(ch) * 12;
    } else if (GET_ARMOR(ch) < 60000) {
        reduce = GET_ARMOR(ch) * 25;
    } else if (GET_ARMOR(ch) < 100000) {
        reduce = GET_ARMOR(ch) * 50;
    } else if (GET_ARMOR(ch) < 150000) {
        reduce = GET_ARMOR(ch) * 75;
    } else if (GET_ARMOR(ch) < 200000) {
        reduce = GET_ARMOR(ch) * 150;
    } else if (GET_ARMOR(ch) >= 200000) {
        reduce = GET_ARMOR(ch) * 250;
    }

    /* loc: 0 = Physical Bonus, 1 = Ki Bonus, 2 = Bonus To Both */
    int i, loc = -1;
    double bonus = 0.0;

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i)) {
            struct obj_data *obj = GET_EQ(ch, i);
            switch (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL)) {
                case MATERIAL_STEEL:
                    loc = 0;
                    bonus = 0.05;
                    break;
                case MATERIAL_IRON:
                    loc = 0;
                    bonus = 0.025;
                    break;
                case MATERIAL_COPPER:
                case MATERIAL_BRASS:
                case MATERIAL_METAL:
                    loc = 0;
                    bonus = 0.01;
                    break;
                case MATERIAL_SILVER:
                    loc = 1;
                    bonus = 0.1;
                    break;
                case MATERIAL_KACHIN:
                    loc = 2;
                    bonus = 0.2;
                    break;
                case MATERIAL_CRYSTAL:
                    loc = 1;
                    bonus = 0.05;
                    break;
                case MATERIAL_DIAMOND:
                    loc = 2;
                    bonus = 0.05;
                    break;
                case MATERIAL_PAPER:
                case MATERIAL_COTTON:
                case MATERIAL_SATIN:
                case MATERIAL_SILK:
                case MATERIAL_BURLAP:
                case MATERIAL_VELVET:
                case MATERIAL_HEMP:
                case MATERIAL_WAX:
                    loc = 2;
                    bonus = -0.05;
                    break;
                default:
                    break;
            }
        }
    }

    if (bonus > 0.95)
        bonus = 0.95;

    if (loc != -1) {
        switch (type) {
            case 0:
                if (loc == 0 || loc == 2) {
                    reduce += reduce * bonus;
                }
                break;
            case 1:
                if (loc == 1 || loc == 2) {
                    reduce += reduce * bonus;
                    reduce /= 2;
                }
                break;
        }
    }

    return (reduce);
}

/* For calculating the difficulty the player has to hit with the skill. */
int chance_to_hit(struct char_data *ch) {

    int num = axion_dice(0);

    if (IS_NPC(ch))
        return (num);

    if (GET_COND(ch, DRUNK) > 4) {
        num += GET_COND(ch, DRUNK);
    }

    return (num);
}

/* For calculating the speed of the attacker and defender */
int handle_speed(struct char_data *ch, struct char_data *vict) {

    if (ch == nullptr || vict == nullptr) { /* Ruh roh*/
        return (0);
    }

    if (GET_SPEEDI(ch) > GET_SPEEDI(vict) * 4) {
        return (15);
    } else if (GET_SPEEDI(ch) > GET_SPEEDI(vict) * 2) {
        return (10);
    } else if (GET_SPEEDI(ch) > GET_SPEEDI(vict)) {
        return (5);
    } else if (GET_SPEEDI(ch) * 4 < GET_SPEEDI(vict)) {
        return (-15);
    } else if (GET_SPEEDI(ch) * 2 < GET_SPEEDI(vict)) {
        return (-10);
    } else if (GET_SPEEDI(ch) < GET_SPEEDI(vict)) {
        return (-5);
    }

    return (0);
}

/* For Destroying or Breaking Limbs */
void hurt_limb(struct char_data *ch, struct char_data *vict, int chance, int area, int64_t power) {
    if (!vict || IS_NPC(vict)) {
        return;
    }

    int dmg = 0;

    if (chance > axion_dice(100)) {
        return;
    }

    if (power > (vict->getEffMaxPL()) * 0.5) {
        dmg = rand_number(25, 40);
    } else if (power > (vict->getEffMaxPL()) * 0.25) {
        dmg = rand_number(15, 24);
    } else if (power > (vict->getEffMaxPL()) * 0.10) {
        dmg = rand_number(8, 14);
    } else if (power > (vict->getEffMaxPL()) * 0.05) {
        dmg = rand_number(4, 7);
    } else {
        dmg = rand_number(1, 3);;
    }

    if (GET_ARMOR(vict) > 50000) {
        dmg -= 5;
    } else if (GET_ARMOR(vict) > 40000) {
        dmg -= 4;
    } else if (GET_ARMOR(vict) > 30000) {
        dmg -= 3;
    } else if (GET_ARMOR(vict) > 20000) {
        dmg -= 2;
    } else if (GET_ARMOR(vict) > 10000) {
        dmg -= 1;
    } else if (GET_ARMOR(vict) > 5000) {
        dmg -= rand_number(0, 1);
    }

    if (dmg <= 0) {
        return;
    }

    if (!is_sparring(ch)) {
        if (area == 0) { /* Arms */
            if (GET_LIMBCOND(vict, 1) - dmg <= 0) {
                act("@RYour attack @YDESTROYS @r$N's@R left arm!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack @YDESTROYS@R YOUR left arm!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack @YDESTROYS @r$N's@R left arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                GET_LIMBCOND(vict, 1) = 0;
                for(auto f : {PLR_THANDW, PLR_CLARM}) vict->playerFlags.reset(f);
                remove_limb(vict, 2);
            } else if (GET_LIMBCOND(vict, 1) > 0) {
                GET_LIMBCOND(vict, 1) -= dmg;
                act("@RYour attack hurts @r$N's@R left arm!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack hurts YOUR left arm!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack hurts @r$N's@R left arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
            } else if (GET_LIMBCOND(vict, 0) - dmg <= 0) {
                act("@RYour attack @YDESTROYS @r$N's@R right arm!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack @YDESTROYS@R YOUR right arm!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack @YDESTROYS @r$N's@R right arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
                GET_LIMBCOND(vict, 0) = 0;
                for(auto f : {PLR_THANDW, PLR_CRARM}) vict->playerFlags.reset(f);
                remove_limb(vict, 2);
            } else if (GET_LIMBCOND(vict, 0) > 0) {
                GET_LIMBCOND(vict, 0) -= dmg;
                act("@RYour attack hurts @r$N's@R right arm!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack hurts YOUR right arm!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack hurts @r$N's@R right arm!@n", true, ch, nullptr, vict, TO_NOTVICT);
            }
        } else if (area == 1) { /* Legs */
            if (GET_LIMBCOND(vict, 3) - dmg <= 0) {
                act("@RYour attack @YDESTROYS @r$N's@R left leg!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack @YDESTROYS@R YOUR left leg!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack @YDESTROYS @r$N's@R left leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                GET_LIMBCOND(vict, 3) = 0;
                for(auto f : {PLR_THANDW, PLR_CLLEG}) vict->playerFlags.reset(f);
                remove_limb(vict, 2);
            } else if (GET_LIMBCOND(vict, 3) > 0) {
                GET_LIMBCOND(vict, 3) -= dmg;
                act("@RYour attack hurts @r$N's@R left leg!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack hurts YOUR left leg!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack hurts @r$N's@R left leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
            } else if (GET_LIMBCOND(vict, 2) - dmg <= 0) {
                act("@RYour attack @YDESTROYS @r$N's@R right leg!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack @YDESTROYS@R YOUR right leg!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack @YDESTROYS @r$N's@R right leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
                GET_LIMBCOND(vict, 2) = 0;
                for(auto f : {PLR_THANDW, PLR_CRLEG}) vict->playerFlags.reset(f);
                remove_limb(vict, 2);
            } else if (GET_LIMBCOND(vict, 2) > 0) {
                GET_LIMBCOND(vict, 2) -= dmg;
                act("@RYour attack hurts @r$N's@R right leg!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@r$n's@R attack hurts YOUR right leg!@n", true, ch, nullptr, vict, TO_VICT);
                act("@r$n's@R attack hurts @r$N's@R right leg!@n", true, ch, nullptr, vict, TO_NOTVICT);
            }
        }
    }
}

/* For damaging equipment when hit */
void dam_eq_loc(struct char_data *vict, int area) {
    int location = 0, num = 0;
    /* Area is 4 possible hit locations in an attack.
    1 Arms, 2 legs, 3 head, and 4 body. */

    if (!vict || vict == nullptr || GET_HIT(vict) <= 0) {
        return;
    }

    switch (area) {
        case 1:
            num = rand_number(1, 8);
            switch (num) {
                case 1:
                    location = WEAR_FINGER_R;
                    break;
                case 2:
                    location = WEAR_FINGER_L;
                    break;
                case 3:
                    location = WEAR_ARMS;
                    break;
                case 4:
                    location = WEAR_WRIST_R;
                    break;
                case 5:
                    location = WEAR_WRIST_L;
                    break;
                case 6:
                case 7:
                case 8:
                    location = WEAR_HANDS;
                    break;
            }
            break;
        case 2:
            num = rand_number(1, 3);
            switch (num) {
                case 1:
                    location = WEAR_LEGS;
                    break;
                case 2:
                    location = WEAR_FEET;
                    break;
                case 3:
                    location = WEAR_WAIST;
                    break;
            }
            break;
        case 3:
            num = rand_number(1, 6);
            switch (num) {
                case 1:
                    location = WEAR_HEAD;
                    break;
                case 2:
                    location = WEAR_NECK_1;
                    break;
                case 3:
                    location = WEAR_NECK_2;
                    break;
                case 4:
                    location = WEAR_EAR_R;
                    break;
                case 5:
                    location = WEAR_EAR_L;
                    break;
                case 6:
                    location = WEAR_EYE;
                    break;
            }
            break;
        case 4:
            num = rand_number(1, 4);
            switch (num) {
                case 1:
                    location = WEAR_BODY;
                    break;
                case 2:
                    location = WEAR_ABOUT;
                    break;
                case 3:
                    location = WEAR_BACKPACK;
                    break;
                case 4:
                    location = WEAR_SH;
                    break;
            }
            break;
        default:
            location = WEAR_BODY;
            break;
    }
    damage_eq(vict, location);
}

void damage_eq(struct char_data *vict, int location) {

    if (GET_EQ(vict, location) && rand_number(1, 20) >= 19 && !AFF_FLAGGED(vict, AFF_SANCTUARY)) {
        struct obj_data *eq = GET_EQ(vict, location);
        if (OBJ_FLAGGED(eq, ITEM_UNBREAKABLE)) {
            return;
        }

        int loss = rand_number(2, 5);

        if (GET_OBJ_VNUM(eq) == 20099 || GET_OBJ_VNUM(eq) == 20098)
            loss = 1;

        if (AFF_FLAGGED(vict, AFF_CURSE)) {
            loss *= 3;
        } else if (AFF_FLAGGED(vict, AFF_BLESS) && rand_number(1, 3) == 3) {
            loss = 1;
        } else if (AFF_FLAGGED(vict, AFF_BLESS)) {
            return;
        }


        GET_OBJ_VAL(eq, VAL_ALL_HEALTH) -= loss;
        if (GET_OBJ_VAL(eq, VAL_ALL_HEALTH) <= 0) {
            GET_OBJ_VAL(eq, VAL_ALL_HEALTH) = 0;
            eq->extra_flags.set(ITEM_BROKEN);
            act("@WYour $p@W completely breaks!@n", false, nullptr, eq, vict, TO_VICT);
            act("@C$N's@W $p@W completely breaks!@n", false, nullptr, eq, vict, TO_NOTVICT);
            perform_remove(vict, location);
            if (!IS_NPC(vict)) {
                vict->save();
            }
        } else if (GET_OBJ_VAL(eq, VAL_ALL_MATERIAL) == MATERIAL_LEATHER ||
                   GET_OBJ_VAL(eq, VAL_ALL_MATERIAL) == MATERIAL_COTTON ||
                   GET_OBJ_VAL(eq, VAL_ALL_MATERIAL) == MATERIAL_SILK) {
            act("@WYour $p@W rips a little!@n", false, nullptr, eq, vict, TO_VICT);
            act("@C$N's@W $p@W rips a little!@n", false, nullptr, eq, vict, TO_NOTVICT);
            if (AFF_FLAGGED(vict, AFF_BLESS)) {
                send_to_char(vict, "@c...But your blessing seems to have partly mended this damage.@n\r\n");
                act("@c...but @C$N's@c body glows blue for a moment and the damage mends a little.@n", true, nullptr,
                    nullptr, vict, TO_NOTVICT);
            } else if (AFF_FLAGGED(vict, AFF_CURSE)) {
                send_to_char(vict, "@r...and your curse seems to have made the damage three times worse!@n\r\n");
                act("@c...but @C$N's@c body glows red for a moment and the damage grow three times worse!@n", true,
                    nullptr, nullptr, vict, TO_NOTVICT);
            }
        } else {
            act("@WYour $p@W cracks a little!@n", false, nullptr, eq, vict, TO_VICT);
            act("@C$N's@W $p@W cracks a little!@n", false, nullptr, eq, vict, TO_NOTVICT);
            if (AFF_FLAGGED(vict, AFF_BLESS)) {
                send_to_char(vict, "@c...But your blessing seems to have partly mended this damage.@n\r\n");
                act("@c...but @C$N's@c body glows blue for a moment and the damage mends a little.@n", true, nullptr,
                    nullptr, vict, TO_NOTVICT);
            } else if (AFF_FLAGGED(vict, AFF_CURSE)) {
                send_to_char(vict, "@r...and your curse seems to have made the damage three times worse!@n\r\n");
                act("@c...but @C$N's@c body glows red for a moment and the damage grow three times worse!@n", true,
                    nullptr, nullptr, vict, TO_NOTVICT);
            }
        }
    }
}

/* This is for huge attacks that are slowly descending on a room */
void huge_update(uint64_t heartPulse, double deltaTime) {
    int dge = 0, skill = 0, bonus = 1, count = 0;
    int64_t dmg = 0;
    struct obj_data *k;
    struct char_data *ch, *vict, *next_v;

    /* Checking the object list for any huge ki attacks */
    for (k = object_list; k; k = k->next) {
        if (GET_AUCTER(k) > 0 && GET_AUCTIME(k) + 604800 <= time(nullptr)) {
            if (IN_ROOM(k) && GET_ROOM_VNUM(IN_ROOM(k)) == 80) {
                room_vnum inroom = IN_ROOM(k);
                extract_obj(k);
                continue;
            }
        }
        if (KICHARGE(k) <= 0) {
            continue;
        }
        if (GET_OBJ_VNUM(k) != 82 && GET_OBJ_VNUM(k) != 83) {
            continue;
        } else if (KIDIST(k) <= 0) {
            /* Genki Dama Section */
            if (KITYPE(k) == 497) {
                if (IN_ROOM(TARGET(k)) == IN_ROOM(k)) {
                    ch = USER(k);
                    if (GET_ROOM_VNUM(IN_ROOM(ch)) == GET_ROOM_VNUM(IN_ROOM(k))) {
                        bonus = 2;
                    }

                    act("@WThe large @cS@Cp@wi@cr@Ci@wt @cB@Co@wm@cb@W descends on YOU! It eclipses everything above you as it crushes down into you! You struggle against it with all your might!@n",
                        true, TARGET(k), nullptr, nullptr, TO_CHAR);
                    act("@WThe large @cS@Cp@wi@cr@Ci@wt @cB@Co@wm@cb@W descends on @C$n@W! It completely obscures $m from view as it crushes into $s body! It appears to be facing some resistance from $m!@n",
                        true, TARGET(k), nullptr, nullptr, TO_ROOM);
                    send_to_room(IN_ROOM(k), "\r\n");
                    if (GET_HIT(TARGET(k)) * bonus < KICHARGE(k) * 5) {

                        act("@WYour strength is no match for the power of the attack! It slowly grinds into you before exploding into a massive blast!@n",
                            true, TARGET(k), nullptr, nullptr, TO_CHAR);
                        act("@C$n@W's strength is no match for the power of the attack! It slowly grinds into $m before exploding into a massive blast!@n",
                            true, TARGET(k), nullptr, nullptr, TO_ROOM);
                        skill = init_skill(ch, SKILL_GENKIDAMA); /* Set skill value */
                        dmg = KICHARGE(k) * 1.25;
                        hurt(0, 0, ch, TARGET(k), nullptr, dmg, 1);
                        dmg /= 2;

                        /* Hit those in the current room. */
                        for (vict = k->getRoom()->people; vict; vict = next_v) {
                            next_v = vict->next_in_room;

                            if (vict == ch) {
                                continue;
                            }
                            if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
                                continue;
                            }
                            if (vict == TARGET(k)) {
                                continue;
                            }
                            if (AFF_FLAGGED(vict, AFF_GROUP)) {
                                if (vict->master == ch) {
                                    continue;
                                } else if (ch->master == vict) {
                                    continue;
                                } else if (vict->master == ch->master) {
                                    continue;
                                }
                            }
                            if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
                                continue;
                            }
                            if (MOB_FLAGGED(vict, MOB_NOKILL)) {
                                continue;
                            }
                            dge = handle_dodge(vict);
                            if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) ||
                                 AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                                (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                                act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict, TO_CHAR);
                                act("@cYou disappear, avoiding the explosion!@n", false, ch, nullptr, vict, TO_VICT);
                                act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict,
                                    TO_NOTVICT);
                                vict->affected_by.reset(AFF_ZANZOKEN);
                                pcost(vict, 0, GET_MAX_HIT(vict) / 200);
                                hurt(0, 0, ch, vict, nullptr, 0, 1);
                                continue;
                            } else if (dge + rand_number(-10, 5) > skill) {
                                act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                                act("@WYou manage to escape the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                                act("@c$N@W manages to escape the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                                hurt(0, 0, ch, vict, nullptr, 0, 1);
                                improve_skill(vict, SKILL_DODGE, 0);
                                continue;
                            } else {
                                count += 1;
                                if (IS_NPC(vict) && count > 10) {
                                    if (GET_HIT(vict) < dmg) {
                                        double loss = 0.0;

                                        if (count >= 30) {
                                            loss = 0.80;
                                        } else if (count >= 20) {
                                            loss = 0.6;
                                        } else if (count >= 15) {
                                            loss = 0.4;
                                        } else if (count >= 10) {
                                            loss = 0.25;
                                        }
                                        vict->modExperience(-(GET_EXP(vict) * loss));
                                    }
                                }
                                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                                act("@RYou are caught by the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                                hurt(0, 0, ch, vict, nullptr, dmg, 1);
                                continue;
                            }
                        }
                        ROOM_DAMAGE(IN_ROOM(k)) = 100;
                        int zone = 0;
                        if ((zone = real_zone_by_thing(GET_ROOM_VNUM(IN_ROOM(ch)))) != NOWHERE) {
                            send_to_zone("A MASSIVE explosion shakes the entire area!\r\n", zone);
                        }

                        extract_obj(k);
                        continue;
                    } /* It isn't stopped! */
                    else {
                        act("@WYou manage to overpower the attack! You lift up into the sky slowly with it and toss it up and away out of sight!@n",
                            true, TARGET(k), nullptr, nullptr, TO_CHAR);
                        act("@C$n@W manages to unbelievably overpower the attack! It is lifted up into the sky and tossed away dramaticly!@n",
                            true, TARGET(k), nullptr, nullptr, TO_ROOM);
                        hurt(0, 0, ch, TARGET(k), nullptr, 0, 1);
                        TARGET(k)->decCurST(KICHARGE(k) / 4);
                        extract_obj(k);
                        continue;
                    }
                } else if (IN_ROOM(TARGET(k)) != IN_ROOM(k)) {
                    ch = USER(k);

                    send_to_room(IN_ROOM(k),
                                 "@WThe large @cS@Cp@wi@cr@Ci@wt @cB@Co@wm@cb@W descends on the area! It slowly burns into the ground before exploding magnificently!@n\r\n");


                    skill = init_skill(ch, SKILL_GENKIDAMA); /* Set skill value */
                    dmg = KICHARGE(k);
                    dmg /= 2;

                    /* Hit those in the current room. */
                    for (vict = k->getRoom()->people; vict; vict = next_v) {
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
                        if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) ||
                             AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                            (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                            act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict, TO_CHAR);
                            act("@cYou disappear, avoiding the explosion!@n", false, ch, nullptr, vict, TO_VICT);
                            act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict, TO_NOTVICT);
                            vict->affected_by.reset(AFF_ZANZOKEN);
                            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
                            hurt(0, 0, ch, vict, nullptr, 0, 1);
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
                            hurt(0, 0, ch, vict, nullptr, dmg, 1);
                            continue;
                        }
                    }
                    ROOM_DAMAGE(IN_ROOM(k)) = 100;
                    int zone = 0;
                    if ((zone = real_zone_by_thing(GET_ROOM_VNUM(IN_ROOM(ch)))) != NOWHERE) {
                        send_to_zone("A MASSIVE explosion shakes the entire area!\r\n", zone);
                    }
                    extract_obj(k);
                    continue;
                }/* End Genki */
                continue;
            }
            /* Genocide Section */
            if (KITYPE(k) == 498) {
                if (IN_ROOM(TARGET(k)) == IN_ROOM(k)) {
                    ch = USER(k);
                    if (GET_ROOM_VNUM(IN_ROOM(ch)) == GET_ROOM_VNUM(IN_ROOM(k))) {
                        bonus = 2;
                    }

                    act("@WThe large @mG@Me@wn@mo@Mc@wi@md@Me@W descends on YOU! It eclipses everything above you as it crushes down into you! You struggle against it with all your might!@n",
                        true, TARGET(k), nullptr, nullptr, TO_CHAR);
                    act("@WThe large @mG@Me@wn@mo@Mc@wi@md@Me@W descends on @C$n@W! It completely obscures $m from view as it crushes into $s body! It appears to be facing some resistance from $m!@n",
                        true, TARGET(k), nullptr, nullptr, TO_ROOM);
                    send_to_room(IN_ROOM(k), "\r\n");
                    if (GET_HIT(TARGET(k)) * bonus < KICHARGE(k) * 10) {

                        act("@WYour strength is no match for the power of the attack! It slowly grinds into you before exploding into a massive blast!@n",
                            true, TARGET(k), nullptr, nullptr, TO_CHAR);
                        act("@C$n@W's strength is no match for the power of the attack! It slowly grinds into $m before exploding into a massive blast!@n",
                            true, TARGET(k), nullptr, nullptr, TO_ROOM);
                        skill = init_skill(ch, SKILL_GENOCIDE); /* Set skill value */
                        dmg = KICHARGE(k);
                        hurt(0, 0, ch, TARGET(k), nullptr, dmg, 1);
                        dmg /= 2;

                        /* Hit those in the current room. */
                        for (vict = k->getRoom()->people; vict; vict = next_v) {
                            next_v = vict->next_in_room;

                            if (vict == ch) {
                                continue;
                            }
                            if (AFF_FLAGGED(vict, AFF_SPIRIT) && !IS_NPC(vict)) {
                                continue;
                            }
                            if (vict == TARGET(k)) {
                                continue;
                            }
                            if (AFF_FLAGGED(vict, AFF_GROUP)) {
                                if (vict->master == ch) {
                                    continue;
                                } else if (ch->master == vict) {
                                    continue;
                                } else if (vict->master == ch->master) {
                                    continue;
                                }
                            }
                            if (GET_LEVEL(vict) <= 8 && !IS_NPC(vict)) {
                                continue;
                            }
                            if (MOB_FLAGGED(vict, MOB_NOKILL)) {
                                continue;
                            }
                            dge = handle_dodge(vict);
                            if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) ||
                                 AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                                (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                                act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict, TO_CHAR);
                                act("@cYou disappear, avoiding the explosion!@n", false, ch, nullptr, vict, TO_VICT);
                                act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict,
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
                                count += 1;
                                if (IS_NPC(vict) && count > 10) {
                                    if (GET_HIT(vict) < dmg) {
                                        double loss = 0.0;

                                        if (count >= 30) {
                                            loss = 0.80;
                                        } else if (count >= 20) {
                                            loss = 0.6;
                                        } else if (count >= 15) {
                                            loss = 0.4;
                                        } else if (count >= 10) {
                                            loss = 0.25;
                                        }
                                        vict->modExperience(-(GET_EXP(vict) * loss));
                                    }
                                }
                                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                                act("@RYou are caught by the explosion!@n", true, ch, nullptr, vict, TO_VICT);
                                act("@R$N@r is caught by the explosion!@n", true, ch, nullptr, vict, TO_NOTVICT);
                                hurt(0, 0, ch, vict, nullptr, dmg, 1);
                                continue;
                            }
                        }
                        ROOM_DAMAGE(IN_ROOM(k)) = 100;
                        int zone = 0;
                        if ((zone = real_zone_by_thing(GET_ROOM_VNUM(IN_ROOM(ch)))) != NOWHERE) {
                            send_to_zone("A MASSIVE explosion shakes the entire area!\r\n", zone);
                        }
                        extract_obj(k);
                        continue;
                    } /* It isn't stopped! */
                    else {
                        act("@WYou manage to overpower the attack! You lift up into the sky slowly with it and toss it up and away out of sight!@n",
                            true, TARGET(k), nullptr, nullptr, TO_CHAR);
                        act("@C$n@W manages to unbelievably overpower the attack! It is lifted up into the sky and tossed away dramaticly!@n",
                            true, TARGET(k), nullptr, nullptr, TO_ROOM);
                        hurt(0, 0, ch, TARGET(k), nullptr, 0, 1);
                        TARGET(k)->decCurST(KICHARGE(k) / 4);
                        extract_obj(k);
                        continue;
                    }
                } else if (IN_ROOM(TARGET(k)) != IN_ROOM(k)) {
                    ch = USER(k);

                    send_to_room(IN_ROOM(k),
                                 "@WThe large @mG@Me@wn@mo@Mc@wi@md@Me@W descends on the area! It slowly burns into the ground before exploding magnificantly!@n\r\n");


                    skill = init_skill(ch, SKILL_GENOCIDE); /* Set skill value */
                    dmg = KICHARGE(k);
                    dmg /= 2;

                    /* Hit those in the current room. */
                    for (vict = k->getRoom()->people; vict; vict = next_v) {
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
                        if (((!IS_NPC(vict) && IS_ICER(vict) && rand_number(1, 30) >= 28) ||
                             AFF_FLAGGED(vict, AFF_ZANZOKEN)) &&
                            (vict->getCurST()) >= 1 && GET_POS(vict) != POS_SLEEPING) {
                            act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict, TO_CHAR);
                            act("@cYou disappear, avoiding the explosion!@n", false, ch, nullptr, vict, TO_VICT);
                            act("@C$N@c disappears, avoiding the explosion!@n", false, ch, nullptr, vict, TO_NOTVICT);
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
                            hurt(0, 0, ch, vict, nullptr, dmg, 1);
                            continue;
                        }
                    }
                    ROOM_DAMAGE(IN_ROOM(k)) = 100;
                    int zone = 0;
                    if ((zone = real_zone_by_thing(GET_ROOM_VNUM(IN_ROOM(ch)))) != NOWHERE) {
                        send_to_zone("A MASSIVE explosion shakes the entire area!\r\n", zone);
                    }
                    extract_obj(k);
                    continue;
                }/* End Genocide */
                continue;
            } else {
                extract_obj(k);
                continue;
            }
        }
        act("$p@W descends slowly towards the ground!@n", true, nullptr, k, nullptr, TO_ROOM);
        KIDIST(k)--;
    }

}
/* End huge ki attack update */

/* For handling homing attacks */
void homing_update(uint64_t heartPulse, double deltaTime) {
    struct obj_data *k;

    for (k = object_list; k; k = k->next) {
        if (!k || k == nullptr)
            continue;

        if (KICHARGE(k) <= 0) {
            continue;
        }

        if (GET_OBJ_VNUM(k) != 80 && GET_OBJ_VNUM(k) != 81 && GET_OBJ_VNUM(k) != 84) {
            continue;
        } else if (TARGET(k) && USER(k)) {
            struct char_data *ch = USER(k);
            struct char_data *vict = TARGET(k);

            if (GET_OBJ_VNUM(k) == 80) { // Tsuihidan
                if (IN_ROOM(k) != IN_ROOM(vict)) {
                    act("@wThe $p@w pursues after you!@n", true, vict, k, nullptr, TO_CHAR);
                    act("@wThe $p@W pursues after @C$n@w!@n", true, vict, k, nullptr, TO_ROOM);
                    obj_from_room(k);
                    obj_to_room(k, IN_ROOM(vict));
                    continue;
                } else {
                    act("@RThe $p@R makes a tight turn and rockets straight for you!@n", true, vict, k, nullptr,
                        TO_CHAR);
                    act("@RThe $p@R makes a tight turn and rockets straight for @r$n@n", true, vict, k, nullptr,
                        TO_ROOM);
                    if (handle_parry(vict) < rand_number(1, 140)) {
                        act("@rThe $p@r slams into your body, exploding in a flash of bright light!@n", true, vict, k,
                            nullptr, TO_CHAR);
                        act("@rThe $p@r slams into @R$n's@r body, exploding in a flash of bright light!@n", true, vict,
                            k, nullptr, TO_ROOM);
                        int64_t dmg = KICHARGE(k);
                        extract_obj(k);
                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        continue;
                    } else if (rand_number(1, 3) > 1) {
                        act("@wYou manage to deflect the $p@W sending it flying away and depleting some of its energy.@n",
                            true, vict, k, nullptr, TO_CHAR);
                        act("@C$n @wmanages to deflect the $p@w sending it flying away and depleting some of its energy.@n",
                            true, vict, k, nullptr, TO_ROOM);
                        KICHARGE(k) -= KICHARGE(k) / 10;
                        if (KICHARGE(k) <= 0) {
                            send_to_room(IN_ROOM(k), "%s has lost all its energy and disappears.\r\n",
                                         k->short_description);
                            extract_obj(k);
                        }
                        continue;
                    } else {
                        act("@wYou manage to deflect the $p@w sending it flying away into the nearby surroundings!@n",
                            true, vict, k, nullptr, TO_CHAR);
                        act("@C$n @wmanages to deflect the $p@w sending it flying away into the nearby surroundings!@n",
                            true, vict, k, nullptr, TO_ROOM);
                        if (ROOM_DAMAGE(IN_ROOM(vict)) <= 95) {
                            ROOM_DAMAGE(IN_ROOM(vict)) += 5;
                        }
                        extract_obj(k);
                        continue;
                    }
                }
                continue;
            } else if (GET_OBJ_VNUM(k) == 81 || GET_OBJ_VNUM(k) == 84) { // Spirit Ball
                if (IN_ROOM(k) != IN_ROOM(vict)) {
                    act("@wYou lose sight of @C$N@W and let $p@W fly away!@n", true, ch, k, vict, TO_CHAR);
                    act("@wYou manage to escape @C$n's@W $p@W!@n", true, ch, k, vict, TO_VICT);
                    act("@C$n@W loses sight of @c$N@W and lets $s $p@W fly away!@n", true, ch, k, vict, TO_NOTVICT);
                    extract_obj(k);
                    continue;
                } else {
                    act("@RYou move your hand and direct $p@R after @r$N@R!@n", true, ch, k, vict, TO_CHAR);
                    act("@r$n@R moves $s hand and directs $p@R after YOU!@n", true, ch, k, vict, TO_VICT);
                    act("@r$n@R moves $s hand and directs $p@R after @r$N@R!@n", true, ch, k, vict, TO_NOTVICT);
                    if (handle_parry(vict) < rand_number(1, 140)) {
                        if (GET_OBJ_VNUM(k) != 84) {
                            act("@rThe $p@r slams into your body, exploding in a flash of bright light!@n", true, vict,
                                k, nullptr, TO_CHAR);
                            act("@rThe $p@r slams into @R$n's@r body, exploding in a flash of bright light!@n", true,
                                vict, k, nullptr, TO_ROOM);
                            int64_t dmg = KICHARGE(k);
                            extract_obj(k);
                            hurt(0, 0, ch, vict, nullptr, dmg, 1);
                        } else if (GET_OBJ_VNUM(k) == 84) {
                            int64_t dmg = KICHARGE(k);
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
                                    act("@R$N@r is cut in half by the attack but regenerates a moment later!@n", true,
                                        ch, nullptr, vict, TO_CHAR);
                                    act("@rYou are cut in half by the attack but regenerate a moment later!@n", true,
                                        ch, nullptr, vict, TO_VICT);
                                    act("@R$N@r is cut in half by the attack but regenerates a moment later!@n", true,
                                        ch, nullptr, vict, TO_NOTVICT);
                                    vict->decCurKI(vict->getMaxKI() / 40);
                                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                                } else if (dmg > GET_MAX_HIT(vict) / 5 && (IS_MAJIN(vict) || IS_BIO(vict))) {
                                    if (GET_SKILL(vict, SKILL_REGENERATE) > rand_number(1, 101) &&
                                        (vict->getCurKI()) >= GET_MAX_MANA(vict) / 40) {
                                        act("@R$N@r is cut in half by the attack but regenerates a moment later!@n",
                                            true, ch, nullptr, vict, TO_CHAR);
                                        act("@rYou are cut in half by the attack but regenerate a moment later!@n",
                                            true, ch, nullptr, vict, TO_VICT);
                                        act("@R$N@r is cut in half by the attack but regenerates a moment later!@n",
                                            true, ch, nullptr, vict, TO_NOTVICT);
                                        vict->decCurKI(vict->getMaxKI() / 40);
                                        hurt(0, 0, ch, vict, nullptr, dmg, 1);
                                    } else {
                                        act("@R$N@r is cut in half by the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                                        act("@rYou are cut in half by the attack!@n", true, ch, nullptr, vict, TO_VICT);
                                        act("@R$N@r is cut in half by the attack!@n", true, ch, nullptr, vict,
                                            TO_NOTVICT);
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
                                }
                            } else {
                                act("@rThe $p@r slams into your body, exploding in a flash of bright light!@n", true,
                                    vict, k, nullptr, TO_CHAR);
                                act("@rThe $p@r slams into @R$n's@r body, exploding in a flash of bright light!@n",
                                    true, vict, k, nullptr, TO_ROOM);
                                hurt(0, 0, ch, vict, nullptr, dmg, 1);
                            }
                            extract_obj(k);
                        }
                        continue;
                    } else if (rand_number(1, 3) > 1) {
                        act("@wYou manage to deflect the $p@W sending it flying away and depleting some of its energy.@n",
                            true, vict, k, nullptr, TO_CHAR);
                        act("@C$n @wmanages to deflect the $p@w sending it flying away and depleting some of its energy.@n",
                            true, vict, k, nullptr, TO_ROOM);
                        KICHARGE(k) -= KICHARGE(k) / 10;
                        if (KICHARGE(k) <= 0) {
                            send_to_room(IN_ROOM(k), "%s has lost all its energy and disappears.\r\n",
                                         k->short_description);
                            extract_obj(k);
                        }
                        continue;
                    } else {
                        act("@wYou manage to deflect the $p@w sending it flying away into the nearby surroundings!@n",
                            true, vict, k, nullptr, TO_CHAR);
                        act("@C$n @wmanages to deflect the $p@w sending it flying away into the nearby surroundings!@n",
                            true, vict, k, nullptr, TO_ROOM);
                        if (ROOM_DAMAGE(IN_ROOM(vict)) <= 95) {
                            ROOM_DAMAGE(IN_ROOM(vict)) += 5;
                        }
                        extract_obj(k);
                        continue;
                    }
                }
            } // Spiritball attack
        } // pursue the target.
    } // End for
}

/* For checking if they have enough free limbs to preform the technique. */
// type 0 is arms, type 1 is legs. 2 is both. 3 is tail.
int limb_ok(struct char_data *ch, int type) {
    if (IS_NPC(ch)) {
        if (AFF_FLAGGED(ch, AFF_ENSNARED) && rand_number(1, 100) <= 90) {
            return false;
        }
        return true;
    }
    if (GRAPPLING(ch) && GRAPTYPE(ch) != 3) {
        send_to_char(ch, "You are too busy grappling!\r\n");
        return false;
    }
    if (GRAPPLED(ch) && (GRAPTYPE(ch) == 1 || GRAPTYPE(ch) == 4)) {
        send_to_char(ch, "You are unable to move while in this hold! Try using 'escape' to get out of it!\r\n");
        return false;
    }
    if (GET_SONG(ch) > 0) {
        send_to_char(ch, "You are currently playing a song! Enter the song command in order to stop!\r\n");
        return false;
    }

    // Arms
    if (type == 0 || type == 2) {
        if (!HAS_ARMS(ch)) {
            send_to_char(ch, "You have no available arms!\r\n");
            return false;
        }
        if (AFF_FLAGGED(ch, AFF_ENSNARED) && rand_number(1, 100) <= 90) {
            send_to_char(ch, "You are unable to move your arms while bound by this strong silk!\r\n");
            WAIT_STATE(ch, PULSE_1SEC);
            return false;
        } else if (AFF_FLAGGED(ch, AFF_ENSNARED)) {
            act("You manage to break the silk ensnaring your arms!", true, ch, nullptr, nullptr, TO_CHAR);
            act("$n manages to break the silk ensnaring $s arms!", true, ch, nullptr, nullptr, TO_ROOM);
            ch->affected_by.reset(AFF_ENSNARED);
        }
        if (GET_EQ(ch, WEAR_WIELD1) && GET_EQ(ch, WEAR_WIELD2)) {
            send_to_char(ch, "Your hands are full!\r\n");
            return false;
        }
    }

    // Legs
    if (type == 1 || type == 2) {
        if (!HAS_LEGS(ch)) {
            send_to_char(ch, "You have no working legs!\r\n");
            return false;
        }
    }

    // tail
    if(type == 3) {
        if(!ch->hasTail()) {
            send_to_char(ch, "You have no tail!\r\n");
            return false;
        }
    }

    return true;
}

int init_skill(struct char_data *ch, int snum) {
    int skill = 0;

    if (!IS_NPC(ch)) {
        skill = GET_SKILL(ch, snum);

        if (PLR_FLAGGED(ch, PLR_TRANSMISSION)) {
            skill += 4;
        }

        if (skill > 118)
            skill = 118;

        return (skill);
    }

    auto l = GET_LEVEL(ch);
    
    if (l <= 10) {
        return rand_number(30, 50);
    } else if (l <= 20) {
        return rand_number(45, 65);
    } else if (l <= 30) {
        return rand_number(55, 70);
    } else if (l <= 50) {
        return rand_number(65, 80);
    } else if (l <= 70) {
        return rand_number(75, 90);
    } else if (l <= 80) {
        return rand_number(85, 100);
    } else if (l <= 90) {
        return rand_number(90, 100);
    } else if (l <= 100) {
        return rand_number(95, 100);
    } else if (l <= 110) {
        return rand_number(95, 105);
    } else {
        return rand_number(100, 110);
    }
}

int handle_block(struct char_data *ch) {

    if (axion_dice(0) <= 4) { /* Critical failure */
        return (1);
    }

    if (!IS_NPC(ch)) { /* Players */
        if (!GET_SKILL(ch, SKILL_BLOCK)) {
            return (0);
        } else {
            int num = GET_SKILL(ch, SKILL_BLOCK);
            if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 3 || GET_GENOME(ch, 1) == 3)) {
                num += 10;
            }
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100) {
                num += 5;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 80) {
                num += 4;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 60) {
                num += 3;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40) {
                num += 2;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 20) {
                num += 1;
            }
            return (num);
        }
    } else { /* Mobs */
        if (!IS_HUMANOID(ch)) { /* Animal/monster types */
            int top = GET_LEVEL(ch) / 4;

            if (top < 5)
                top = 6;
            return (rand_number(5, top));
        } else { /* Intelligent Skills Mobs */
            if (GET_LEVEL(ch) >= 110) {
                return (rand_number(95, 105));
            } else if (GET_LEVEL(ch) >= 100) {
                return (rand_number(85, 95));
            } else if (GET_LEVEL(ch) >= 90) {
                return (rand_number(70, 85));
            } else if (GET_LEVEL(ch) >= 75) {
                return (rand_number(50, 70));
            } else if (GET_LEVEL(ch) >= 40) {
                return (rand_number(40, 50));
            } else {
                int top = GET_LEVEL(ch);

                if (top < 15)
                    top = 16;
                return (rand_number(15, top));
            }
        }
    }
}

int handle_dodge(struct char_data *ch) {

    if (axion_dice(0) <= 4) { /* Critical failure */
        return (1);
    }

    if (!IS_NPC(ch)) { /* Players */
        if (!GET_SKILL(ch, SKILL_DODGE)) {
            return (0);
        } else {
            int num = GET_SKILL(ch, SKILL_DODGE);
            if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 3 || GET_GENOME(ch, 1) == 3)) {
                num += 10;
            }
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100) {
                num += 5;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 80) {
                num += 4;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 60) {
                num += 3;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40) {
                num += 2;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 20) {
                num += 1;
            }
            if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 100) {
                num += 3;
            } else if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 75) {
                num += 2;
            } else if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 50) {
                num += 1;
            }
            if (GET_SKILL_BASE(ch, SKILL_ROLL) >= 100) {
                num += 5;
            } else if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 80) {
                num += 4;
            } else if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 60) {
                num += 3;
            } else if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 40) {
                num += 2;
            } else if (GET_SKILL_BASE(ch, SKILL_SURVIVAL) >= 20) {
                num += 1;
            }
            if (group_bonus(ch, 2) == 8) {
                num += num * 0.05;
            }
            return (num);
        }
    } else { /* Mobs */
        if (!IS_HUMANOID(ch)) { /* Animal/monster types */
            int top = (GET_LEVEL(ch) + 1) / 8;

            if (top < 5)
                top = 6;
            return (rand_number(5, top));
        } else { /* Intelligent Skills Mobs */
            if (GET_LEVEL(ch) >= 110) {
                return (rand_number(95, 105));
            } else if (GET_LEVEL(ch) >= 100) {
                return (rand_number(75, 95));
            } else if (GET_LEVEL(ch) >= 90) {
                return (rand_number(50, 85));
            } else if (GET_LEVEL(ch) >= 75) {
                return (rand_number(30, 70));
            } else if (GET_LEVEL(ch) >= 40) {
                return (rand_number(20, 50));
            } else {
                int top = GET_LEVEL(ch);

                if (top < 15)
                    top = 16;
                return (rand_number(15, top));
            }
        }
    }
}

int check_def(struct char_data *vict) {
    int index = 0;
    int pry = handle_parry(vict), dge = handle_dodge(vict), blk = handle_block(vict);

    index = pry + dge + blk;

    if (index > 0)
        index /= 3;

    if (AFF_FLAGGED(vict, AFF_KNOCKED)) {
        index = 0;
    }
    return index;
}

void handle_defense(struct char_data *vict, int *pry, int *blk, int *dge) {

    *pry = handle_parry(vict);

    *blk = handle_block(vict);

    *dge = handle_dodge(vict);

    if (!IS_NPC(vict)) {
        if (GET_BONUS(vict, BONUS_WALL)) {
            *blk += GET_SKILL(vict, SKILL_BLOCK) * 0.20;
        }

        if (GET_BONUS(vict, BONUS_PUSHOVER)) {
            *blk -= GET_SKILL(vict, SKILL_BLOCK) * 0.20;
        }

        if (!GET_EQ(vict, WEAR_WIELD1) && !GET_EQ(vict, WEAR_WIELD2)) {
            *blk += 4;
        }

        if (*blk > 110) {
            *blk = 110;
        }

        if (GET_BONUS(vict, BONUS_EVASIVE)) {
            *dge += (GET_SKILL(vict, SKILL_DODGE) * 0.15);
        }

        if (GET_BONUS(vict, BONUS_PUNCHINGBAG)) {
            *dge -= GET_SKILL(vict, SKILL_DODGE) * 0.15;
        }

        if (*dge > 110) {
            *dge = 110;
        }

        if (*pry > 110) {
            *pry = 110;
        }
        if (PLR_FLAGGED(vict, PLR_GOOP) && rand_number(1, 100) >= 15) {
            *dge += 100;
            *blk += 100;
            *pry += 100;
        }
    }

    return;
}

void
parry_ki(double attperc, struct char_data *ch, struct char_data *vict, char sname[1000], int prob, int perc, int skill,
         int type) {
    char buf[200];
    char buf2[200];
    char buf3[200];
    int foundv = false, foundo = false;
    int64_t dmg = 0;
    struct obj_data *tob, *next_obj;
    struct char_data *tch, *next_v;

    for (tch = ch->getRoom()->people; tch; tch = next_v) {
        next_v = tch->next_in_room;

        if (tch == ch)
            continue;
        if (tch == vict)
            continue;
        if (!can_kill(ch, tch, nullptr, 1))
            continue;

        if (rand_number(1, 101) >= 90 && foundv == false) {
            if (handle_parry(tch) > rand_number(1, 140)) {
                sprintf(buf, "@C$N@W deflects your %s, sending it flying away!@n", sname);
                sprintf(buf2, "@WYou deflect @C$n's@W %s sending it flying away!@n", sname);
                sprintf(buf3, "@C$N@W deflects @c$n's@W %s sending it flying away!@n", sname);
                act(buf, true, ch, nullptr, tch, TO_CHAR);
                act(buf2, true, ch, nullptr, tch, TO_VICT);
                act(buf3, true, ch, nullptr, tch, TO_NOTVICT);
                foundv = false;
            } else {
                foundv = true;
                sprintf(buf,
                        "@WYou watch as the deflected %s slams into @C$N@W, exploding with a roar of blinding light!@n",
                        sname);
                sprintf(buf2,
                        "@c$n@W watches as the deflected %s slams into you! The %s explodes with a roar of blinding light!@n",
                        sname, sname);
                sprintf(buf3,
                        "@c$n@W watches as the deflected %s slams into @C$N@W! The %s explodes with a roar of blinding light!@n",
                        sname, sname);
                act(buf, true, vict, nullptr, tch, TO_CHAR);
                act(buf2, true, vict, nullptr, tch, TO_VICT);
                act(buf3, true, vict, nullptr, tch, TO_NOTVICT);
                dmg = damtype(ch, type, skill, attperc);
                hurt(0, 0, ch, tch, nullptr, dmg, 1);
                return;
            }
        }
    }

    for (tob = ch->getRoom()->contents; tob; tob = next_obj) {
        next_obj = tob->next_content;
        if (OBJ_FLAGGED(tob, ITEM_UNBREAKABLE))
            continue;
        if (foundo == true)
            continue;
        if (rand_number(1, 101) >= 80) {
            foundo = true;
            sprintf(buf,
                    "@WYou watch as the deflected %s slams into @g$p@W, exploding with a roar of blinding light!@n",
                    sname);
            sprintf(buf2,
                    "@c$n@W watches as the deflected %s slams into @g$p@W, exploding with a roar of blinding light!@n",
                    sname);
            act(buf, true, vict, tob, nullptr, TO_CHAR);
            act(buf2, true, vict, tob, nullptr, TO_ROOM);
            hurt(0, 0, ch, nullptr, tob, 25, 1);
            return;
        }
    }

    if ((foundo == false || foundv == false) && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE)) {
        sprintf(buf,
                "@WYou watch as the deflected %s slams into the ground, exploding with a roar of blinding light!@n",
                sname);
        sprintf(buf2, "@WThe deflected %s slams into the ground, exploding with a roar of blinding light!@n", sname);
        act(buf, true, vict, nullptr, nullptr, TO_CHAR);
        act(buf2, true, vict, nullptr, nullptr, TO_ROOM);
        if (SECT(IN_ROOM(vict)) != SECT_INSIDE && SECT(IN_ROOM(vict)) != SECT_UNDERWATER &&
            SECT(IN_ROOM(vict)) != SECT_WATER_SWIM && SECT(IN_ROOM(vict)) != SECT_WATER_NOSWIM &&
            SECT(IN_ROOM(vict)) != SECT_UNDERWATER && SECT(IN_ROOM(vict)) != SECT_WATER_SWIM &&
            SECT(IN_ROOM(vict)) != SECT_WATER_NOSWIM) {
            impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
            switch (rand_number(1, 8)) {
                case 1:
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_ROOM);
                    break;
                case 2:
                    if (rand_number(1, 4) == 4 && ROOM_EFFECT(IN_ROOM(vict)) == 0) {
                        ROOM_EFFECT(IN_ROOM(vict)) = 1;
                        act("Lava spews up through cracks in the ground, roaring into the sky as a large column of molten rock!",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("Lava spews up through cracks in the ground, roaring into the sky as a large column of molten rock!",
                            true, ch, nullptr, vict, TO_ROOM);
                    }
                    break;
                case 3:
                    act("A cloud of dust envelopes the entire area!", true, ch, nullptr, vict, TO_CHAR);
                    act("A cloud of dust envelopes the entire area!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 4:
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 5:
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 6:
                    act("The explosion continues to burn spreading out and devouring some more of the ground before dying out.",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("The explosion continues to burn spreading out and devouring some more of the ground before dying out.",
                        true, ch, nullptr, vict, TO_ROOM);
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
                    act("The water collapses in on the hole created!", true, ch, nullptr, vict, TO_CHAR);
                    act("The water collapses in on the hole create!", true, ch, nullptr, vict, TO_ROOM);
                    break;
            }
        }
        if (SECT(IN_ROOM(vict)) == SECT_WATER_SWIM || SECT(IN_ROOM(vict)) == SECT_WATER_NOSWIM) {
            switch (rand_number(1, 3)) {
                case 1:
                    act("A huge column of water erupts from the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("A huge column of water erupts from the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 2:
                    act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr, vict, TO_CHAR);
                    act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 3:
                    act("A huge depression forms in the water and erupts into a wave from the impact!", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("A huge depression forms in the water and erupts into a wave from the impact!", true, ch,
                        nullptr, vict, TO_ROOM);
                    break;
            }
        }
        if (SECT(IN_ROOM(vict)) == SECT_INSIDE) {
            impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
            switch (rand_number(1, 8)) {
                case 1:
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_ROOM);
                    break;
                case 2:
                    act("The structure of the surrounding room cracks and quakes from the blast!", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("The structure of the surrounding room cracks and quakes from the blast!", true, ch, nullptr,
                        vict, TO_ROOM);
                    break;
                case 3:
                    act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr, vict, TO_CHAR);
                    act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 4:
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 5:
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 6:
                    act("The walls of the surrounding room crack in the same instant!", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("The walls of the surrounding room crack in the same instant!", true, ch, nullptr, vict,
                        TO_ROOM);
                    break;
                default:
                    /* we want no message for the default */
                    break;
            }
        }
        if (ROOM_DAMAGE(IN_ROOM(ch)) <= 95) {
            ROOM_DAMAGE(IN_ROOM(ch)) += 5;
        }
        int zone = 0;
        if ((zone = real_zone_by_thing(GET_ROOM_VNUM(IN_ROOM(ch)))) != NOWHERE) {
            send_to_zone("An explosion shakes the entire area!\r\n", zone);
        }
        return;
    }
}

void dodge_ki(struct char_data *ch, struct char_data *vict, int type, int type2, int skill, int skill2) {
    if (type == 0 && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE)) {
        if (SECT(IN_ROOM(ch)) != SECT_INSIDE) {
            impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
            switch (rand_number(1, 8)) {
                case 1:
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_ROOM);
                    break;
                case 2:
                    if (rand_number(1, 4) == 4 && ROOM_EFFECT(IN_ROOM(vict)) == 0) {
                        ROOM_EFFECT(IN_ROOM(vict)) = 1;
                        act("Lava spews up through cracks in the ground, roaring into the sky as a large column of molten rock!",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("Lava spews up through cracks in the ground, roaring into the sky as a large column of molten rock!",
                            true, ch, nullptr, vict, TO_ROOM);
                    }
                    break;
                case 3:
                    act("A cloud of dust envelopes the entire area!", true, ch, nullptr, vict, TO_CHAR);
                    act("A cloud of dust envelopes the entire area!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 4:
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 5:
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 6:
                    act("The explosion continues to burn spreading out and devouring some more of the ground before dying out.",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("The explosion continues to burn spreading out and devouring some more of the ground before dying out.",
                        true, ch, nullptr, vict, TO_ROOM);
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
                    act("The water collapses in on the hole created!", true, ch, nullptr, vict, TO_CHAR);
                    act("The water collapses in on the hole create!", true, ch, nullptr, vict, TO_ROOM);
                    break;
            }
        }
        if (SECT(IN_ROOM(vict)) == SECT_WATER_SWIM || SECT(IN_ROOM(vict)) == SECT_WATER_NOSWIM) {
            switch (rand_number(1, 3)) {
                case 1:
                    act("A huge column of water erupts from the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("A huge column of water erupts from the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 2:
                    act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr, vict, TO_CHAR);
                    act("The impact briefly causes a swirling vortex of water!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 3:
                    act("A huge depression forms in the water and erupts into a wave from the impact!", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("A huge depression forms in the water and erupts into a wave from the impact!", true, ch,
                        nullptr, vict, TO_ROOM);
                    break;
            }
        }
        if (SECT(IN_ROOM(ch)) == SECT_INSIDE) {
            impact_sound(ch, "@wA loud roar is heard nearby!@n\r\n");
            switch (rand_number(1, 8)) {
                case 1:
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("Debris is thrown into the air and showers down thunderously!", true, ch, nullptr, vict,
                        TO_ROOM);
                    break;
                case 2:
                    act("The structure of the surrounding room cracks and quakes from the blast!", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("The structure of the surrounding room cracks and quakes from the blast!", true, ch, nullptr,
                        vict, TO_ROOM);
                    break;
                case 3:
                    act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr, vict, TO_CHAR);
                    act("Parts of the ceiling collapse, crushing into the floor!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 4:
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The surrounding area roars and shudders from the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 5:
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_CHAR);
                    act("The ground shatters apart from the stress of the impact!", true, ch, nullptr, vict, TO_ROOM);
                    break;
                case 6:
                    act("The walls of the surrounding room crack in the same instant!", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("The walls of the surrounding room crack in the same instant!", true, ch, nullptr, vict,
                        TO_ROOM);
                    break;
                default:
                    /* we want no message for the default */
                    break;
            }
        }
        if (ROOM_DAMAGE(IN_ROOM(ch)) <= 95) {
            ROOM_DAMAGE(IN_ROOM(ch)) += 5;
        }
        int zone = 0;
        if ((zone = real_zone_by_thing(GET_ROOM_VNUM(IN_ROOM(ch)))) != NOWHERE) {
            send_to_zone("An explosion shakes the entire area!\r\n", zone);
        }
    }
    if (type == 1) {
        if (rand_number(1, 3) != 2) {
            act("@RIt turns around at the last second and begins to pursue @r$N@R!@n", true, ch, nullptr, vict,
                TO_CHAR);
            act("@RIt turns around at the last second and begins to pursue YOU!@n", true, ch, nullptr, vict, TO_VICT);
            act("@RIt turns around at the last second and begins to pursue @r$N@R!@n", true, ch, nullptr, vict,
                TO_NOTVICT);
            struct obj_data *obj;
            int num = 0;

            switch (skill2) {
                case 461:
                    num = 80;
                    break;
                default:
                    num = 80;
                    break;
            }

            obj = read_object(num, VIRTUAL);
            obj_to_room(obj, IN_ROOM(ch));

            TARGET(obj) = vict;
            KICHARGE(obj) = damtype(ch, type2, skill, .2);
            KITYPE(obj) = skill2;
            USER(obj) = ch;
        } else {
            act("@RIt fails to follow after @r$N@R!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@RIt fails to follow after YOU!@n", true, ch, nullptr, vict, TO_VICT);
            act("@RIt fails to follow after @r$N@R!@n", true, ch, nullptr, vict, TO_NOTVICT);
        }
    }
    if (type == 2 && (skill2 != 481 || IS_FRIEZA(ch))) {
        if (skill2 == 481) {
            int chance = rand_number(25, 50), prob = axion_dice(0);
            if (GET_SKILL(ch, SKILL_KIENZAN) >= 100) {
                chance += chance * 0.8;
            } else if (GET_SKILL(ch, SKILL_KIENZAN) >= 60) {
                chance += chance * 0.5;
            } else if (GET_SKILL(ch, SKILL_KIENZAN) >= 40) {
                chance += chance * 0.25;
            }
            if (chance < prob) {
                return;
            }
        }
        act("@RYou turn it around and send it back after @r$N@R!@n", true, ch, nullptr, vict, TO_CHAR);
        act("@W$n @Rturns it around and sends it back after YOU!@n", true, ch, nullptr, vict, TO_VICT);
        act("@W$n @Rturns it around and sends it back after @r$N@R!@n", true, ch, nullptr, vict, TO_NOTVICT);
        struct obj_data *obj;
        int num = 0;

        switch (skill2) {
            case 496:
                num = 81;
                break;
            case 481:
                num = 84;
                break;
            default:
                num = 81;
                break;
        }

        obj = read_object(num, VIRTUAL);
        obj_to_room(obj, IN_ROOM(ch));

        TARGET(obj) = vict;
        KICHARGE(obj) = damtype(ch, type2, skill, .3);
        KITYPE(obj) = skill2;
        USER(obj) = ch;
    }
}

static void damtype_unarmed_infuse(char_data *ch, int64_t *dam) {
    if (AFF_FLAGGED(ch, AFF_INFUSE)) {
        *dam += (*dam / 100) * (GET_SKILL(ch, SKILL_INFUSE) / 2);
        if (IS_JINTO(ch)) {
            if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100) {
                *dam += ((*dam * 0.01) * (GET_SKILL(ch, SKILL_INFUSE) / 2)) * 0.5;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 60) {
                *dam += ((*dam * 0.01) * (GET_SKILL(ch, SKILL_INFUSE) / 2)) * 0.25;
            } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40) {
                *dam += ((*dam * 0.01) * (GET_SKILL(ch, SKILL_INFUSE) / 2)) * 0.05;
            }
        }
    }
}

static void damtype_unarmed_hasshuken(char_data *ch, int64_t *dam) {
    if (AFF_FLAGGED(ch, AFF_HASS)) {
        *dam *= 2;
        if (IS_KRANE(ch)) {
            if (GET_SKILL(ch, SKILL_HASSHUKEN) >= 100) {
                *dam += *dam * 0.3;
            } else if (GET_SKILL(ch, SKILL_HASSHUKEN) >= 60) {
                *dam += *dam * 0.2;
            } else if (GET_SKILL(ch, SKILL_HASSHUKEN) >= 40) {
                *dam += *dam * 0.1;
            }
        }
    }
}

static void damtype_unarmed_hasshuken_or_infuse(char_data *ch, int64_t *dam) {
    if (AFF_FLAGGED(ch, AFF_HASS)) {
        damtype_unarmed_hasshuken(ch, dam);
    } else {
        damtype_unarmed_infuse(ch, dam);
    }
}

static void damtype_unarmed_preference(char_data *ch, int64_t *dam) {
    if (GET_PREFERENCE(ch) == PREFERENCE_THROWING) {
        *dam -= *dam * 0.15;
    } else if (GET_PREFERENCE(ch) == PREFERENCE_H2H) {
        *dam += *dam * 0.20;
    }
}

static void damtype_focus(char_data *ch, int64_t *dam, int64_t focus, int divby) {
    if (focus > 0) {
        dam += focus * (*dam / divby);
    }
}

static void damtype_unarmed(char_data *ch, int skill, int64_t *dam) {
    // General Arlian bonus.
    if (IS_ARLIAN(ch)) {
        *dam += *dam * 0.02;
    }

    // Sixteen's Iron Hand Bonus.
    if (IS_ANDSIX(ch)) {
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
            *dam += *dam * 0.1;
    }

    // Brawler bonus
    if (GET_BONUS(ch, BONUS_BRAWLER) > 0) {
        *dam += *dam * .2;
    }

    switch (skill) {
        // Punch.
        case 0:
            damtype_unarmed_hasshuken_or_infuse(ch, dam);
            // Kame Arts bonus.
            if (IS_ROSHI(ch)) {
                if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                    *dam += *dam * 0.2;
            }
            break;
            // Kick
        case 1:
            damtype_unarmed_infuse(ch, dam);
            // Crane Arts
            if (IS_KRANE(ch)) {
                if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 75)
                    *dam += *dam * 0.2;
            }
            break;
        case 2: // Elbow
        case 5: // Uppercut
            damtype_unarmed_hasshuken_or_infuse(ch, dam);
            break;
        case 3:
        case 4:
        case 6:
        case 8:
        case 51:
        case 52:
            damtype_unarmed_infuse(ch, dam);
    }

    damtype_unarmed_preference(ch, dam);

}

static void damtype_human_grandmaster(char_data *ch, int skill, int64_t *dam) {
    if (IS_HUMAN(ch)) {
        switch (skill) {
            case 101:
                *dam = *dam * 1.1;
            case 102:
                *dam = *dam * 1.2;
            case 103:
                *dam = *dam * 1.3;
        }
    }
}

static void damtype_human_ki(char_data *ch, int64_t *dam, int bon) {
    if (IS_HUMAN(ch)) {
        *dam += (*dam / 100) * bon;
    }
}

static void damtype_saiyan_ki(char_data *ch, int64_t *dam, int bon) {
    if (IS_SAIYAN(ch)) {
        *dam += (*dam / 100) * bon;
    }
}

static void damtype_kai_ki(char_data *ch, int64_t *dam, int bon) {
    if (IS_KAI(ch)) {
        *dam += (*dam / 100) * bon;
    }
}

static void damtype_icer_ki(char_data *ch, int64_t *dam, int bon) {
    if (IS_ICER(ch) || (IS_BIO(ch) && (GET_GENOME(ch, 0) == 4 || GET_GENOME(ch, 1) == 4))) {
        *dam += (*dam / 100) * bon;
    }
}

/* Damage for player and NPC attacks  */
int64_t damtype(struct char_data *ch, int type, int skill, double percent) {
    int64_t dam = 0, cou1 = 0, cou2 = 0, focus = 0;

    /* Player damages based on attack */
    if (!IS_NPC(ch)) {
        if (GET_SKILL(ch, SKILL_FOCUS)) {
            focus = GET_SKILL(ch, SKILL_FOCUS);
        }
        if (type != -2) {
            LASTATK(ch) = type;
        } else {
            type = 0;
        }

        // split switch approach to compress code.
        switch (type) {
            // big list of h2h stuff for starters.
            case -1:
                cou1 = 1 + ((skill / 4) * ((GET_HIT(ch) / 1200) + GET_STR(ch)));
                cou2 = 1 + ((skill / 4) * ((GET_HIT(ch) / 1000) + GET_STR(ch)));
                break;
            case 0: /* Punch */
                cou1 = 15 + ((skill / 4) * ((GET_HIT(ch) / 1600) + GET_STR(ch)));
                cou2 = 15 + ((skill / 4) * ((GET_HIT(ch) / 1300) + GET_STR(ch)));
                break;
            case 1: /* Kick */
                cou1 = 40 + ((skill / 4) * ((GET_HIT(ch) / 1200) + GET_STR(ch)));
                cou2 = 40 + ((skill / 4) * ((GET_HIT(ch) / 1000) + GET_STR(ch)));
                break;
            case 2: /* Elbow */
                cou1 = 100 + ((skill / 4) * ((GET_HIT(ch) / 1300) + GET_STR(ch)));
                cou2 = 100 + ((skill / 4) * ((GET_HIT(ch) / 1050) + GET_STR(ch)));
                break;
            case 3: /* Knee */
                cou1 = 150 + ((skill / 4) * ((GET_HIT(ch) / 1100) + GET_STR(ch)));
                cou2 = 150 + ((skill / 4) * ((GET_HIT(ch) / 1000) + GET_STR(ch)));
                break;
            case 4: /* Roundhouse */
                cou1 = 500 + ((skill / 4) * ((GET_HIT(ch) / 1000) + GET_STR(ch)));
                cou2 = 500 + ((skill / 4) * ((GET_HIT(ch) / 800) + GET_STR(ch)));
                break;
            case 5: /* Uppercut */
                cou1 = 350 + ((skill / 4) * ((GET_HIT(ch) / 1100) + GET_STR(ch)));
                cou2 = 350 + ((skill / 4) * ((GET_HIT(ch) / 900) + GET_STR(ch)));
                break;
            case 6: /* Slam */
                cou1 = 8000 + ((skill / 4) * ((GET_HIT(ch) / 800) + GET_STR(ch)));
                cou2 = 8000 + ((skill / 4) * ((GET_HIT(ch) / 500) + GET_STR(ch)));
                break;
            case 8: /* Heeldrop */
                cou1 = 12500 + ((skill / 4) * ((GET_HIT(ch) / 700) + GET_STR(ch)));
                cou2 = 12500 + ((skill / 4) * ((GET_HIT(ch) / 400) + GET_STR(ch)));
                break;
            case 51: /* Bash */
                cou1 = 1000 + ((skill / 4) * ((GET_HIT(ch) / 700) + GET_STR(ch)));
                cou2 = 1000 + ((skill / 4) * ((GET_HIT(ch) / 550) + GET_STR(ch)));
                break;
            case 52: /* Headbutt */
                cou1 = 800 + ((skill / 4) * ((GET_HIT(ch) / 900) + GET_STR(ch)));
                cou2 = 800 + ((skill / 4) * ((GET_HIT(ch) / 650) + GET_STR(ch)));
                break;
            case 56: /* TAILWHIP */
                cou1 = 400 + ((skill / 4) * ((GET_HIT(ch) / 1100) + GET_STR(ch)));
                cou2 = 400 + ((skill / 4) * ((GET_HIT(ch) / 1000) + GET_STR(ch)));
                break;
        }

        bool ki = false;
        // Set initial damage value.
        switch (type) {
            // h2h commonalities
            case -1:
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 8:
                dam = large_rand(cou1, cou2);
                dam += GET_STR(ch) * (dam * 0.005);
                break;
            case 51:
            case 52:
            case 56: /* TAILWHIP */
                dam = large_rand(cou1, cou2);
                dam += GET_LEVEL(ch) * 100;
                dam += GET_STR(ch) * (dam * 0.005);
                break;
            default: // all ki abilities.
                dam = GET_MAX_MANA(ch) * percent;
                ki = true;
        }

        // ki type move pre-processing
        switch (type) {
            case 11: /* Tsuihidan */
            case 12: /* Renzo */
            case 23: /* Rogafufuken */
            case 25: /* Kienzan */
                dam += GET_LEVEL(ch) * 500;
                break;
            case 13: /* Kamehameha */
            case 16: /* Galik Gun */
            case 26: /* Tribeam */
            case 50: /* Seishou Enko */
                dam += GET_LEVEL(ch) * 800;
                break;
            case 14: /* Masenko */
            case 30: /* Darkness Dragon Slash */
            case 44: /* Spiral Comet 1 */
            case 45: /* Spiral Comet 2 */
            case 43: /* Water Spikes */
                dam += GET_LEVEL(ch) * 1000;
                break;
            case 15: /* Dodonpa */
            case 17: /* Deathbeam */
            case 19: /* Twin Slash */
                dam += GET_LEVEL(ch) * 650;
                break;
            case 18: /* Eraser Cannon */
            case 33: /* Hell Spear Blast */
            case 54: /* Zen Blade */
            case 55: /* Sundering Force */
                dam += GET_LEVEL(ch) * 700;
                break;
            case 20: /* Psychic Blast */
            case 27: /* Special Beam Cannon */
            case 29: /* Crusher Ball */
            case 37: /* Phoenix Slash */
                dam += GET_LEVEL(ch) * 1200;
                break;
            case 21: /* Honoo */
            case 39: /* Spirit ball */
            case 47: /* Water Razor */
            case 48: /* Koteiru Bakuha */
            case 49: /* Hell Spiral */
                dam += GET_LEVEL(ch) * 900;
                break;
            case 22: /* Dual Beam */
            case 24: /* Bakuhatsuha */
                dam += GET_LEVEL(ch) * 600;
                break;
            case 28: /* Final Flash */
                dam += GET_LEVEL(ch) * 1500;
            case 31: /* Psychic Barrage */
            case 36: /* Big Bang */
                dam += GET_LEVEL(ch) * 1100;
                break;
            case 32: /* Hell Flash */
            case 46: /* Star Breaker */
                dam += GET_LEVEL(ch) * 1400;
                break;
            case 34: /* Kakusanha */
                dam += GET_LEVEL(ch) * 1050;
                break;
            case 35: /* Scatter Shot */
            case 53: /* Star Nova */
                dam += GET_LEVEL(ch) * 1600;
                break;
            case 38: /* Deathball */
                dam += GET_LEVEL(ch) * 1700;
                break;
            case 40: /* Genki Dama */
            case 41: /* Genocide */
                dam += GET_LEVEL(ch) * 2000;
                break;
            case 42: /* Kousengan */
                dam += GET_LEVEL(ch) * 550;
                break;
            case 57: /* Light Grenade */
                dam += GET_LEVEL(ch) * 1700;
                break;
        }

        if (ki) dam *= 1.25;


        switch (type) {
            case -1:
                if (!PLR_FLAGGED(ch, PLR_THANDW))
                    damtype_unarmed_hasshuken_or_infuse(ch, &dam);

                if (GET_BONUS(ch, BONUS_BRAWLER) > 0) {
                    dam += dam * .2;
                }
                if (GET_PREFERENCE(ch) == PREFERENCE_KI) {
                    dam -= dam * 0.20;
                }
                if (GET_PREFERENCE(ch) == PREFERENCE_WEAPON && GET_CHARGE(ch) >= GET_MAX_MANA(ch) * 0.05) {
                    dam += GET_MAX_MANA(ch) * 0.05;
                    GET_CHARGE(ch) -= GET_MAX_MANA(ch) * 0.05;
                } else if (GET_PREFERENCE(ch) == PREFERENCE_WEAPON && GET_CHARGE(ch) > 0) {
                    dam += GET_CHARGE(ch);
                    GET_CHARGE(ch) -= 0;
                }
                if (group_bonus(ch, 2) == 8) {
                    dam += dam * 0.02;
                }
                break;
            case 0: /* Punch */
            case 1: /* Kick */
            case 2: /* Elbow */
            case 3: /* Knee */
            case 4: /* Roundhouse */
            case 5: /* Uppercut */
            case 6: /* Slam */
            case 8: /* Heeldrop */
            case 51: /* Bash */
            case 52: /* Headbutt */
                damtype_unarmed(ch, type, &dam);
                break;
            case 56: /* TAILWHIP */
                damtype_unarmed_infuse(ch, &dam);
                damtype_unarmed_preference(ch, &dam);
                break;

            case 7: /* Kiball */
                damtype_focus(ch, &dam, focus, 1000);
                damtype_human_ki(ch, &dam, 25);
                break;
            case 9: /* Kiblast */
                damtype_focus(ch, &dam, focus, 500);
                damtype_human_ki(ch, &dam, 25);
                break;
            case 10: /* Beam/Shog */
            case 11: /* Tsuihidan */
            case 12: /* Renzo */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_ki(ch, &dam, 25);
                break;
            case 13: /* Kamehameha */
                if (focus > 0) {
                    dam += (dam * 0.005) * focus;
                }
                damtype_human_grandmaster(ch, skill, &dam);
                damtype_human_ki(ch, &dam, 15);
                break;
            case 14: /* Masenko */
            case 15: /* Dodonpa */
            case 16: /* Galik Gun */
            case 17: /* Deathbeam */
            case 18: /* Eraser Cannon */
            case 19: /* Twin Slash */
            case 20: /* Psychic Blast */
            case 21: /* Honoo */
            case 24: /* Bakuhatsuha */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_ki(ch, &dam, 15);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 22: /* Dual Beam */
                damtype_focus(ch, &dam, focus, 200);
                break;
            case 23: /* Rogafufuken */
                dam += (dam / 100) * GET_STR(ch);
                damtype_focus(ch, &dam, focus, 200);
                if (GET_BONUS(ch, BONUS_BRAWLER) > 0) {
                    dam += dam * .2;
                }
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 25: /* Kienzan */
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 26: /* Tribeam */

                if (!IS_NPC(ch) && percent > 0.15) {
                    double hitperc = (percent - 0.15) * 5;
                    int64_t amount = ch->getEffMaxPL() * hitperc;
                    int64_t difference = GET_HIT(ch) - amount;

                    ch->decCurHealth(amount, 1);

                    damtype_focus(ch, &dam, focus, 200);
                } else {
                    damtype_focus(ch, &dam, focus, 200);
                }
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 27: /* Special Beam Cannon */
                dam += (dam / 100) * GET_INT(ch);
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 28: /* Final Flash */
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 29: /* Crusher Ball */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 30: /* Darkness Dragon Slash */
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 31: /* Psychic Barrage */
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 32: /* Hell Flash */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 33: /* Hell Spear Blast */
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                break;
            case 34: /* Kakusanha */
                damtype_focus(ch, &dam, focus, 200);
                damtype_icer_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 35: /* Scatter Shot */
                damtype_focus(ch, &dam, focus, 200);
                damtype_icer_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 36: /* Big Bang */
                damtype_focus(ch, &dam, focus, 200);
                damtype_icer_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 37: /* Phoenix Slash */
                damtype_focus(ch, &dam, focus, 200);
                damtype_icer_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 38: /* Deathball */
                damtype_focus(ch, &dam, focus, 200);
                damtype_icer_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 39: /* Spirit ball */
                damtype_focus(ch, &dam, focus, 200);
                damtype_icer_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 40: /* Genki Dama */
                damtype_focus(ch, &dam, focus, 200);
                damtype_kai_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 41: /* Genocide */
                damtype_focus(ch, &dam, focus, 200);
                damtype_kai_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 42: /* Kousengan */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 43: /* Water Spikes */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 44: /* Spiral Comet 1 */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 45: /* Spiral Comet 2 */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 46: /* Star Breaker */
                damtype_focus(ch, &dam, focus, 200);
                break;
            case 47: /* Water Razor */
                damtype_focus(ch, &dam, focus, 200);
                break;
            case 48: /* Koteiru Bakuha */
                damtype_focus(ch, &dam, focus, 200);
                break;
            case 49: /* Hell Spiral */
                damtype_focus(ch, &dam, focus, 200);
                switch(ch->form) {
                    case FormID::Android10:
                        dam += (dam * 0.01) * 5;
                        break;
                    case FormID::Android20:
                        dam += (dam * 0.01) * 15;
                        break;
                    case FormID::Android30:
                        dam += (dam * 0.01) * 25;
                        break;
                    case FormID::Android40:
                        dam += (dam * 0.01) * 50;
                        break;
                    case FormID::Android50:
                        dam += (dam * 0.01) * 75;
                        break;
                    case FormID::Android60:
                        dam += dam;
                        break;
                    default:
                        break;

                }
                break;
            case 50: /* Seishou Enko */
                damtype_focus(ch, &dam, focus, 200);
                break;
            case 53: /* Star Nova */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_ki(ch, &dam, 15);
                break;
            case 54: /* Zen Blade */
                damtype_focus(ch, &dam, focus, 200);
                damtype_saiyan_ki(ch, &dam, 20);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 55: /* Sundering Force */
                damtype_focus(ch, &dam, focus, 200);
                damtype_human_grandmaster(ch, skill, &dam);
                break;
            case 57: /* Light Grenade */
                damtype_focus(ch, &dam, focus, 200);
                break;
        }
    } else {
        dam = (GET_HIT(ch) * 0.05) + (GET_MAX_HIT(ch) * 0.025);
        dam += (dam * 0.005) * GET_STR(ch);
        if (GET_LEVEL(ch) >= 120) {
            dam *= 0.25;
        } else if (GET_LEVEL(ch) >= 110) {
            dam *= 0.45;
        } else if (GET_LEVEL(ch) >= 100) {
            dam *= 0.75;
        }
    }

    if (IS_NPC(ch)) {

        if (type == 0 || type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6 || type == 8 ||
            type == 51 || type == 52 || type == 56) {
            dam += GET_STR(ch) * (dam * 0.005);
        } else {
            dam += GET_INT(ch) * (dam * 0.005);
        }

        auto mob_hit = GET_HIT(ch);
        auto max_hit = GET_MAX_HIT(ch);
        int64_t mobperc = (mob_hit * 100) / max_hit;
        if (mobperc < 98 && mobperc >= 90) {
            dam = dam * 0.95;
        } else if (mobperc < 90 && mobperc >= 80) {
            dam = dam * 0.90;
        } else if (mobperc < 80 && mobperc >= 790) {
            dam = dam * 0.85;
        } else if (mobperc < 70 && mobperc >= 50) {
            dam = dam * 0.80;
        } else if (mobperc < 50 && mobperc >= 30) {
            dam = dam * 0.70;
        } else if (mobperc <= 29) {
            dam = dam * 0.60;
        }

        if (GET_CLASS(ch) != SenseiID::Commoner) {
            dam += dam * 0.3;
        }
    }


    if (GET_KAIOKEN(ch) > 0) {
        dam += (dam / 200) * GET_KAIOKEN(ch);
    }

    /* Start of Fury Mode for halfbreeds */
    if (PLR_FLAGGED(ch, PLR_FURY) &&
        (type == 0 || type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6 || type == 8 ||
         type == 51 || type == 52)) {
        dam *= 1.5;
        act("Your rage magnifies your attack power!", true, ch, nullptr, nullptr, TO_CHAR);
        act("Swirling energy flows around $n as $e releases $s rage in the attack!", true, ch, nullptr, nullptr,
            TO_ROOM);
        if (rand_number(1, 10) >= 7) {
            send_to_char(ch, "You feel less angry.\r\n");
            ch->affected_by.reset(PLR_FURY);
        }
    } else if (PLR_FLAGGED(ch, PLR_FURY)) {
        dam *= 2;
        act("Your rage magnifies your attack power!", true, ch, nullptr, nullptr, TO_CHAR);
        act("Swirling energy flows around $n as $e releases $s rage in the attack!", true, ch, nullptr, nullptr,
            TO_ROOM);
        ch->affected_by.reset(PLR_FURY);
    }
    /* End of Fury Mode for halfbreeds */

    if ((type == -1 || type == 0 || type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6 ||
         type == 8)) {
        if (!IS_NPC(ch))
            dam -= dam * 0.08;
        if (!IS_NPC(ch) && dam > GET_MAX_HIT(ch) * 0.1)
            dam *= 0.6;
    } else {
        dam += (dam * 0.005) * GET_INT(ch);
        if (GET_PREFERENCE(ch) == PREFERENCE_WEAPON) {
            dam -= dam * 0.25;
        } else if (GET_PREFERENCE(ch) == PREFERENCE_THROWING) {
            dam -= dam * 0.15;
        }
    }

    return dam;
}

void saiyan_gain(struct char_data *ch, struct char_data *vict) {
    int gain = rand_number(GET_LEVEL(ch) * 6, GET_LEVEL(ch) * 8);
    int weak = false;

    if (!vict)
        return;

    if (IS_NPC(ch))
        return;

    if (GET_MAX_HIT(vict) < GET_MAX_HIT(ch) / 10) {
        weak = true;
    }

    if (GET_LEVEL(ch) > 99) {
        gain += rand_number(GET_LEVEL(ch) * 300, GET_LEVEL(ch) * 500);
    } else if (GET_LEVEL(ch) > 80) {
        gain += rand_number(GET_LEVEL(ch) * 150, GET_LEVEL(ch) * 200);
    } else if (GET_LEVEL(ch) > 60) {
        gain += rand_number(GET_LEVEL(ch) * 80, GET_LEVEL(ch) * 100);
    } else if (GET_LEVEL(ch) > 50) {
        gain += rand_number(GET_LEVEL(ch) * 20, GET_LEVEL(ch) * 25);
    } else if (GET_LEVEL(ch) > 40) {
        gain += rand_number(GET_LEVEL(ch) * 8, GET_LEVEL(ch) * 10);
    } else if (GET_LEVEL(ch) > 30) {
        gain += rand_number(GET_LEVEL(ch) * 5, GET_LEVEL(ch) * 8);
    }

    if (IS_BIO(ch) && (GET_GENOME(ch, 0) == 2 || GET_GENOME(ch, 1) == 2)) {
        gain /= 2;
    }
    if (!(rand_number(1, 22) >= 18 && (GET_LEVEL(ch) == 100 || level_exp(ch, GET_LEVEL(ch) + 1) - (GET_EXP(ch)) > 0))) {
        return;
    }
    if (weak) {
        send_to_char(ch, "@D[@YSaiyan @RBlood@D] @WThey are too weak to inspire your saiyan soul!@n\r\n");
        return;
    }

    std::vector<int64_t> stats;
    for (const auto stat: {0, 1, 2}) {
        if (!ch->is_soft_cap(stat, 1.5))
            stats.push_back(stat);
    }

    if (stats.empty()) {
        send_to_char(ch, "@D[@YSaiyan @RBlood@D] @WYou feel you have reached your current limits.@n\r\n");
        return;
    }

    auto itr = Random::get(stats);

    switch (*itr) {
        case 0:
            ch->gainBasePL(gain);
            send_to_char(ch, "@D[@YSaiyan @RBlood@D] @WYou feel slightly stronger. @D[@G+%s@D]@n\r\n",
                         add_commas(gain).c_str());
            break;
        case 1:
            ch->gainBaseKI(gain);
            send_to_char(ch, "@D[@YSaiyan @RBlood@D] @WYou feel your spirit grow. @D[@G+%s@D]@n\r\n",
                         add_commas(gain).c_str());
            break;
        case 2:
            ch->gainBaseST(gain);
            send_to_char(ch, "@D[@YSaiyan @RBlood@D] @WYou feel slightly more vigorous. @D[@G+%s@D]@n\r\n",
                         add_commas(gain).c_str());
            break;
    }

}


static void spar_helper(struct char_data *ch, struct char_data *vict, int type, int64_t dmg) {
    int chance = 0, gmult, gravity, bonus = 1, pscost = 2, difference = 0;
    int64_t gain = 0, pl = 0, ki = 0, st = 0, gaincalc = 0;

	//If damage is greater than a tenth of the opponents health, there's a greater chance to proc the spar gains
    if (dmg > GET_MAX_HIT(vict) / 10) {
        chance = rand_number(20, 100);
    } else if (dmg <= GET_MAX_HIT(vict) / 10) {
        chance = rand_number(1, 75);
    }

	//The chance is reduced if you keep farming in one area via Relaxcount
    if (GET_RELAXCOUNT(ch) >= 464) {
        chance = 0;
    } else if (GET_RELAXCOUNT(ch) >= 232) {
        chance -= chance * 0.5;
    } else if (GET_RELAXCOUNT(ch) >= 116) {
        chance -= chance * 0.2;
    }

	//gmult sets the gain for vital increases, based on the character's level x6
    gmult = (GET_LEVEL(ch) * 6);

	//This is then improved by the burden ratio
    auto ratio = ch->getBurdenRatio();
    gmult += gmult * ratio;

    if (auto obj = GET_EQ(ch, WEAR_SH); obj) {
		//If you are using a spar booster
        if (GET_OBJ_VNUM(obj) == 1127) {
            gmult *= 4;
        }
    }

	//Bonuses by room to vital gains
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_WORKOUT) || (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HBTC))) {
        if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 19100 && GET_ROOM_VNUM(IN_ROOM(ch)) <= 19199) {
            gmult *= 1.75;
            pscost += 2;
        } else {
            gmult *= 1.25;
            pscost += 1;
        }
        pl = large_rand(gmult * .8, gmult * 1.2);
        ki = large_rand(gmult * .8, gmult * 1.2);
        st = large_rand(gmult * .8, gmult * 1.2);
    } else {
        pl = large_rand(gmult * .4, gmult * .8);
        ki = large_rand(gmult * .4, gmult * .8);
        st = large_rand(gmult * .4, gmult * .8);
    }

	//Logic for earning xp through sparring, based on the character's level, the curve is roughly exponential
    if (chance >= rand_number(60, 75)) {
        int64_t num = 0, maxnum = 500000;
        if (GET_LEVEL(ch) >= 70) {
            num += GET_LEVEL(ch) * 2250;
        } else if (GET_LEVEL(ch) >= 60) {
            num += GET_LEVEL(ch) * 2000;
        } else if (GET_LEVEL(ch) >= 50) {
            num += GET_LEVEL(ch) * 1750;
        } else if (GET_LEVEL(ch) >= 45) {
            num += GET_LEVEL(ch) * 1500;
        } else if (GET_LEVEL(ch) >= 40) {
            num += GET_LEVEL(ch) * 1250;
        } else if (GET_LEVEL(ch) >= 35) {
            num += GET_LEVEL(ch) * 1000;
        } else if (GET_LEVEL(ch) >= 30) {
            num += GET_LEVEL(ch) * 750;
        } else if (GET_LEVEL(ch) >= 25) {
            num += GET_LEVEL(ch) * 550;
        } else if (GET_LEVEL(ch) >= 20) {
            num += GET_LEVEL(ch) * 400;
        } else if (GET_LEVEL(ch) >= 15) {
            num += GET_LEVEL(ch) * 250;
        } else if (GET_LEVEL(ch) >= 10) {
            num += GET_LEVEL(ch) * 120;
        } else if (GET_LEVEL(ch) >= 5) {
            num += GET_LEVEL(ch) * 70;
        } else {
            num += GET_LEVEL(ch) * 30;
        }
        if (num > maxnum) {
            num = maxnum;
        }

        bool isLethal = !(is_sparring(ch) && is_sparring(vict));

		//Check if victim is NPC or Player, both have different logic here.
        if (vict != nullptr && IS_NPC(vict)) {
			//Work out how challenging of an npc to fight this is, if the victim is stronger we want to give more xp. This doesn't affect attributes.
			float plRatio = vict->getMaxPLTrans() / ch->getMaxPLTrans();
            float plGain = 1;

            //The lowest you can get from a fight is 0.5 times the xp, so long as they aren't 5x weaker than you
            if (plRatio > 0.2) {
                plGain = 0.5 + plRatio / 2;
                if (plGain > 3) plGain = 3;
            }
            else {
                send_to_char(ch, "This foe is not strong enough for you to learn anything.\r\n");
                plGain = 0;
            }

			//Rewarded if your opponent is fighting for the kill
			float deadlyBonus = isLethal ? 1.2 : 1;

            //Average out the bonus to limit the exponential gain
            float gearGain = gear_exp(ch, 100) / 100;
            if (gearGain <= 0) {
                gearGain = 0.1;
            }

            gaincalc = (plGain + gearGain) / 2;

            gaincalc = num * gaincalc * deadlyBonus;
            type = 3;
        } else if (vict != nullptr && !IS_NPC(vict)) {
			//Fighting against players has randomised gains. Does not get a bonus for higher power level in spars

			//Rewarded if your opponent is fighting for the kill
			float deadlyBonus = isLethal ? 2 : 1;

            gaincalc = large_rand(num * deadlyBonus, 1.4 * num * deadlyBonus);
            gaincalc = gear_exp(ch, gaincalc);
            if (GET_LEVEL(ch) > GET_LEVEL(vict))
                difference = GET_LEVEL(ch) - GET_LEVEL(vict);
            else if (GET_LEVEL(ch) < GET_LEVEL(vict))
                difference = GET_LEVEL(vict) - GET_LEVEL(ch);
        } else {
            gaincalc = 0;
        }
        if (vict != nullptr) {
			//You are penalized for level difference in sparring? Remove this perhaps as it disincentivises oldbies teaching newbies. And it's against the lore, that happens a lot.
            if (difference >= 51) {
                send_to_char(ch, "The difference in your levels is too great for you to gain anything.\r\n");
                return;
            } else if (difference >= 40) {
                gaincalc = gaincalc * 0.05;
            } else if (difference >= 30) {
                gaincalc = gaincalc * 0.10;
            } else if (difference >= 20) {
                gaincalc = gaincalc * 0.25;
            } else if (difference >= 10) {
                gaincalc = gaincalc * 0.50;
            }
            if (!IS_NPC(vict) && !IS_NPC(ch)) {
				//Logic for instructing and giving a bonus to your sparring partner, it costs practices to the 'victim'
                if (!IS_NPC(ch) && PRF_FLAGGED(vict, PRF_INSTRUCT)) {
                    if (GET_PRACTICES(vict) > 10) {
                        send_to_char(vict, "You instruct them in proper fighting techniques and strategies.\r\n");
                        act("You take $N's instruction to heart and gain more experience.\r\n", false, ch, nullptr,
                            vict, TO_CHAR);
                        vict->modPractices(-10);
                        bonus = 2;
                    }
                }
            }
        }

		//Saiyan's and Halfies get boosted gains from sparring, Icers and Bio's with the same gene get a debuff
        if (IS_SAIYAN(ch)) {
            gaincalc = gaincalc * 1.25;
        }
        if (IS_HALFBREED(ch)) {
            gaincalc = gaincalc * 1.2;
        }
        if (IS_ICER(ch) || (IS_BIO(ch) && (GET_GENOME(ch, 0) == 4 || GET_GENOME(ch, 1) == 4))) {
            gaincalc = gaincalc * 0.9;
        }
		//Room bonuses to xp gain
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_WORKOUT) || (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HBTC))) {
            if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 19100 && GET_ROOM_VNUM(IN_ROOM(ch)) <= 19199) {
                gaincalc *= 1.5;
            } else {
                gaincalc *= 1.25;
            }
        }
        gain = gaincalc * Random::get<double>(0.8, 1.2);
        gain = gain * bonus;
		//Gain the xp
        ch->modExperience(gain);
        send_to_char(ch, "@D[@Y+ @G%s @mExp@D]@n ", add_commas(gain).c_str());

		//Handling for awarding vitals to the player
        std::vector<int64_t> stats;
        for (const auto stat: {0, 1, 2}) {
            if (!ch->is_soft_cap(stat, 1.0))
                stats.push_back(stat);
        }

        if(!stats.empty() && rand_number(1, 5) >= 4) {

            auto itr = Random::get(stats);

            switch(*itr) {
                case 0:
                    send_to_char(ch, "@D[@Y+ @R%s @rPL@D]@n\r\n", add_commas(pl).c_str());
                    ch->gainBasePL(pl);
                    break;
                case 1:
                    send_to_char(ch, "@D[@Y+ @C%s @cKI@D]@n\r\n", add_commas(ki).c_str());
                    ch->gainBaseKI(ki);
                    break;
                case 2:
                    send_to_char(ch, "@D[@Y+ @C%s @cST@D]@n\r\n", add_commas(st).c_str());
                    ch->gainBaseST(st);
                    break;
            }
        } else {
            send_to_char(ch, "\r\n");
        }
    }
}

void spar_gain(struct char_data *ch, struct char_data *vict, int type, int64_t dmg) {
    for(auto c : {ch, vict}) {
        if(GET_POS(c) < POS_FIGHTING) return;
        for(auto a : {AFF_SLEEP, AFF_PARALYZE, AFF_STUNNED, AFF_KNOCKED, AFF_PARA, AFF_FROZEN}) if(AFF_FLAGGED(c, a)) return;
    }

    if(AFF_FLAGGED(ch, AFF_FLYING) != AFF_FLAGGED(vict, AFF_FLYING)) return;

    spar_helper(ch, vict, type, dmg);
    spar_helper(vict, ch, type, dmg);
}

bool can_grav(struct char_data *ch) {
    auto result = ch->getBurdenRatio() <= 1.0;
    if(!result) {
        send_to_char(ch, "You are too burdened to even think about it!\r\n");
    }
    return result;
}

/* If they can preform the attack or perform the attack on target. */
int can_kill(struct char_data *ch, struct char_data *vict, struct obj_data *obj, int num) {
    /* Target Related */
    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return 0;
    }
    if (IS_CARRYING_W(ch) > CAN_CARRY_W(ch)) {
        send_to_char(ch, "You are weighted down too much!\r\n");
        return 0;
    }
    if (vict) {
        if (GET_HIT(vict) <= 0 && FIGHTING(vict)) {
            return 0;
        }
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
            send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
            return 0;
        } else if (vict == ch) {
            send_to_char(ch, "That's insane, don't hurt yourself. Hurt others! That's the key to life ^_^\r\n");
            return 0;
        } else if (vict->gooptime > 0) {
            send_to_char(ch, "It seems like it'll be hard to kill them right now...\r\n");
            return 0;
        } else if (CARRYING(ch)) {
            send_to_char(ch, "You are too busy protecting the person on your shoulder!\r\n");
            return 0;
        } else if (CARRIED_BY(vict)) {
            send_to_char(ch, "They are being protected by someone else!\r\n");
            return 0;
        } else if (AFF_FLAGGED(vict, AFF_PARALYZE)) {
            send_to_char(ch, "They are a statue, just leave them alone...\r\n");
            return 0;
        } else if (MOB_FLAGGED(vict, MOB_NOKILL)) {
            send_to_char(ch, "But they are not to be killed!\r\n");
            return 0;
        } else if (MAJINIZED(ch) == ((vict)->id)) {
            send_to_char(ch, "You can not harm your master!\r\n");
            return 0;
        } else if (GET_BONUS(ch, BONUS_COWARD) > 0 && GET_MAX_HIT(vict) > GET_MAX_HIT(ch) + (GET_MAX_HIT(ch) * .5) &&
                   !FIGHTING(ch)) {
            send_to_char(ch, "You are too cowardly to start anything with someone so much stronger than yourself!\r\n");
            return 0;
        } else if (MAJINIZED(vict) == ((ch)->id)) {
            send_to_char(ch, "You can not harm your servant.\r\n");
            return 0;
        } else if ((GRAPPLING(ch) && GRAPTYPE(ch) != 3) || (GRAPPLED(ch) && (GRAPTYPE(ch) == 1 || GRAPTYPE(ch) == 4))) {
            send_to_char(ch, "You are too busy grappling!%s\r\n", GRAPPLED(ch) != nullptr ? " Try 'escape'!" : "");
            return 0;
        } else if (GRAPPLING(ch) && GRAPPLING(ch) != vict) {
            send_to_char(ch, "You can't reach that far in your current position!\r\n");
            return 0;
        } else if (GRAPPLED(ch) && GRAPPLED(ch) != vict) {
            send_to_char(ch, "You can't reach that far in your current position!\r\n");
            return 0;
        } else if (!IS_NPC(ch) && !IS_NPC(vict) && AFF_FLAGGED(ch, AFF_SPIRIT) &&
                   (!is_sparring(ch) || !is_sparring(vict)) && num != 2) {
            send_to_char(ch, "You can not fight other players in AL/Hell.\r\n");
            return 0;
        } else if (GET_LEVEL(vict) <= 8 && !IS_NPC(ch) && !IS_NPC(vict) && (!is_sparring(ch) || !is_sparring(vict))) {
            send_to_char(ch, "Newbie Shield Protects them!\r\n");
            return 0;
        } else if (GET_LEVEL(ch) <= 8 && !IS_NPC(ch) && !IS_NPC(vict) && (!is_sparring(ch) || !is_sparring(vict))) {
            send_to_char(ch, "Newbie Shield Protects you until level 8.\r\n");
            return 0;
        } else if (PLR_FLAGGED(vict, PLR_SPIRAL) && num != 3) {
            send_to_char(ch,
                         "Due to the nature of their current technique anything less than a Tier 4 or AOE attack will not work on them.\r\n");
            return 0;
        } else if (ABSORBING(ch)) {
            send_to_char(ch, "You are too busy absorbing %s!\r\n", GET_NAME(ABSORBING(ch)));
            return 0;
        } else if (ABSORBBY(ch)) {
            send_to_char(ch, "You are too busy being absorbed by %s!\r\n", GET_NAME(ABSORBBY(ch)));
            return 0;
        } else if ((std::abs(GET_ALT(vict) - GET_ALT(ch)) == 1) && (IS_NAMEK(ch) || (IS_BIO(ch) && (GET_GENOME(ch, 0) == 3 || GET_GENOME(ch, 1) == 3)))) {
            act("@GYou stretch your limbs toward @g$N@G in an attempt to hit $M!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@g$n@G stretches $s limbs toward @RYOU@G in an attempt to land a hit!@n", true, ch, nullptr, vict,
                TO_VICT);
            act("@g$n@G stretches $s limbs toward @g$N@G in an attempt to hit $M!@n", true, ch, nullptr, vict,
                TO_NOTVICT);
            return 1;
        } else if (AFF_FLAGGED(ch, AFF_FLYING) && !AFF_FLAGGED(vict, AFF_FLYING) && num == 0) {
            send_to_char(ch, "You are too far above them.\r\n");
            return 0;
        } else if (!AFF_FLAGGED(ch, AFF_FLYING) && AFF_FLAGGED(vict, AFF_FLYING) && num == 0) {
            send_to_char(ch, "They are too far above you.\r\n");
            return 0;
        } else if (!IS_NPC(ch) && GET_ALT(ch) > GET_ALT(vict) && !IS_NPC(vict) && num == 0) {
            if (GET_ALT(vict) < 0) {
                GET_ALT(vict) = GET_ALT(ch);
                return 1;
            } else {
                send_to_char(ch, "You are too far above them.\r\n");
                return 0;
            }
        } else if (!IS_NPC(ch) && GET_ALT(ch) < GET_ALT(vict) && !IS_NPC(vict) && num == 0) {
            if (GET_ALT(vict) > 2) {
                GET_ALT(vict) = GET_ALT(ch);
                return 1;
            } else {
                send_to_char(ch, "They are too far above you.\r\n");
                return 0;
            }
        } else {
            return 1;
        }
    }
    if (obj) {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
            send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
            return 0;
        } else if (OBJ_FLAGGED(obj, ITEM_UNBREAKABLE) && GET_OBJ_VNUM(obj) != 87 && GET_OBJ_VNUM(obj) != 80 &&
                   GET_OBJ_VNUM(obj) != 81 && GET_OBJ_VNUM(obj) != 82 && GET_OBJ_VNUM(obj) != 83) {
            send_to_char(ch, "You can't hit that, it is protected by the immortals!\r\n");
            return 0;
        } else if (AFF_FLAGGED(ch, AFF_FLYING)) {
            send_to_char(ch, "You are too far above it.\r\n");
            return 0;
        } else if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
            send_to_char(ch, "It is already broken!\r\n");
            return 0;
        } else {
            return 1;
        }
    } else {
        send_to_char(ch, "Error: Report to imm.");
        return 0;
    }
}

/* Whether they know the skill they are trying to use */
int check_skill(struct char_data *ch, int skill) {
    if (!know_skill(ch, skill) && !IS_NPC(ch)) {
        return 0;
    } else {
        return 1;
    }
}

/* Whether they have enough stamina or charged ki to preform the skill */
bool check_points(struct char_data *ch, int64_t ki, int64_t st) {

    if (GET_PREFERENCE(ch) == PREFERENCE_H2H && GET_CHARGE(ch) >= GET_MAX_MANA(ch) * 0.1) {
        st *= 0.5;
    }

    int fail = false;
    if (IS_NPC(ch)) {
        if ((ch->getCurKI()) < ki) {
            send_to_char(ch, "You do not have enough ki!\r\n");
            return false;
        }
        if ((ch->getCurST()) < st) {
            send_to_char(ch, "You do not have enough stamina!\r\n");
            return false;
        }
    }

    if (GET_CHARGE(ch) < ki) {
        send_to_char(ch, "You do not have enough ki charged.\r\n");
        int64_t perc = GET_MAX_MANA(ch) * 0.01;
        if (ki >= perc * 49) {
            send_to_char(ch, "You need at least 50 percent charged.\r\n");
        } else if (ki >= perc * 44) {
            send_to_char(ch, "You need at least 45 percent charged.\r\n");
        } else if (ki >= perc * 39) {
            send_to_char(ch, "You need at least 40 percent charged.\r\n");
        } else if (ki >= perc * 34) {
            send_to_char(ch, "You need at least 35 percent charged.\r\n");
        } else if (ki >= perc * 29) {
            send_to_char(ch, "You need at least 30 percent charged.\r\n");
        } else if (ki >= perc * 24) {
            send_to_char(ch, "You need at least 25 percent charged.\r\n");
        } else if (ki >= perc * 19) {
            send_to_char(ch, "You need at least 20 percent charged.\r\n");
        } else if (ki >= perc * 14) {
            send_to_char(ch, "You need at least 15 percent charged.\r\n");
        } else if (ki >= perc * 9) {
            send_to_char(ch, "You need at least 10 percent charged.\r\n");
        } else if (ki >= perc * 4) {
            send_to_char(ch, "You need at least 5 percent charged.\r\n");
        } else if (ki >= 1) {
            send_to_char(ch, "You need at least 1 percent charged.\r\n");
        }
        fail = true;
    }

    if (IS_ICER(ch)) {
        switch(ch->form) {
            case FormID::IcerFirst:
                st *= 1.05;
                break;
            case FormID::IcerSecond:
                st *= 1.1;
                break;
            case FormID::IcerThird:
                st *= 1.15;
                break;
            case FormID::IcerFourth:
                st *= 1.20;
                break;
            default:
                break;
        }
    }

    // Below section is commented out due to not using these flags anymore.
    // TODO: Review and re-implement.

    /*
    if (IS_NONPTRANS(ch)) {
        if (PLR_FLAGGED(ch, PLR_TRANS1)) {
            st *= 0.8;
        } else if (PLR_FLAGGED(ch, PLR_TRANS2)) {
            st *= 0.6;
        } else if (PLR_FLAGGED(ch, PLR_TRANS3)) {
            st *= 0.4;
        } else if (PLR_FLAGGED(ch, PLR_TRANS4)) {
            st *= 0.2;
        }
    }
     */
    auto ratio = ch->getBurdenRatio();
    // increase the stamina costs by ratio. Ratio can be 0.0 to 1.0 or more.
    // If ratio is 0, then no change. If ratio is 1.0, then double the cost.
    st += st * ratio;

    if ((ch->getCurST()) < st) {
        send_to_char(ch, "You do not have enough stamina.\r\n@C%s@n needed.\r\n", add_commas(st).c_str());
        fail = true;
    }

    return !fail;
}

/* Subtract the stamina or ki required */
void pcost(struct char_data *ch, double ki, int64_t st) {
    if (IS_NPC(ch)) {
        if(ki) ch->decCurKI(ki);
        if(st) ch->decCurST(st);
        return;
    }

    int before = 0;
    if (GET_LEVEL(ch) > 1 && !IS_NPC(ch)) {
        if (ki == 0) {
            before = (ch->getCurST());
        }
        if (GET_CHARGE(ch) <= (GET_MAX_MANA(ch) * ki)) {
            GET_CHARGE(ch) = 0;
        }
        if (GET_CHARGE(ch) > (GET_MAX_MANA(ch) * ki)) {
            GET_CHARGE(ch) -= (GET_MAX_MANA(ch) * ki);
        }
        if (GET_CHARGE(ch) < 0) {
            GET_CHARGE(ch) = 0;
        }
        if (GET_KAIOKEN(ch) > 0) {
            st += (st / 20) * GET_KAIOKEN(ch);
        }
        if (AFF_FLAGGED(ch, AFF_HASS)) {
            st += st * .3;
        }
        if (!IS_NPC(ch) && GET_BONUS(ch, BONUS_HARDWORKER) > 0) {
            st -= st * .25;
        } else if (!IS_NPC(ch) && GET_BONUS(ch, BONUS_SLACKER) > 0) {
            st += st * .25;
        }

        if (GET_PREFERENCE(ch) == PREFERENCE_H2H && GET_CHARGE(ch) >= GET_MAX_MANA(ch) * 0.1) {
            st -= st * 0.5;
            GET_CHARGE(ch) -= st;
            if (GET_CHARGE(ch) < 0)
                GET_CHARGE(ch) = 0;
        }

        if (IS_ICER(ch)) {
            switch(ch->form) {
                case FormID::IcerFirst:
                    st *= 1.05;
                    break;
                case FormID::IcerSecond:
                    st *= 1.1;
                    break;
                case FormID::IcerThird:
                    st *= 1.15;
                    break;
                case FormID::IcerFourth:
                    st *= 1.20;
                    break;
                default:
                    break;
            }
        }

        // TODO: reimplement
        /*
        if (IS_NONPTRANS(ch)) {
            if (PLR_FLAGGED(ch, PLR_TRANS1)) {
                st -= st * 0.2;
            } else if (PLR_FLAGGED(ch, PLR_TRANS2)) {
                st -= st * 0.4;
            } else if (PLR_FLAGGED(ch, PLR_TRANS3)) {
                st -= st * 0.6;
            } else if (PLR_FLAGGED(ch, PLR_TRANS4)) {
                st -= st * 0.8;
            }
        }
         */


        auto ratio = ch->getBurdenRatio();
        st += st * ratio;
        ch->decCurST(st);
    }
}

/* Main damage function for RDBS 'Real Dragonball Battle System' */
void hurt(int limb, int chance, struct char_data *ch, struct char_data *vict, struct obj_data *obj, int64_t dmg, int type) {
    int64_t index = 0;
    int64_t maindmg = dmg, beforered = dmg;
    int dead = false;

    /* If a character is trageted */

    if (type <= 0) {
        if (IS_SAIYAN(ch) && PLR_FLAGGED(ch, PLR_TAIL)) {
            dmg += dmg * .15;
        }
        if (IS_NAMEK(ch) && !GET_EQ(ch, WEAR_HEAD)) {
            dmg += dmg * .25;
        }
        if (group_bonus(ch, 2) == 4) {
            dmg += dmg * .1;
        } else if (group_bonus(ch, 2) == 12) {
            dmg -= dmg * .1;
        }
    } else {
        /* human racial bonus on hold */
        /*if (IS_HUMAN(ch) && !IS_NPC(ch)) {
   if (PLR_FLAGGED(ch, PLR_TRANS3)) {
    dmg += dmg * 0.45;
   }
  }*/
        dmg = dmg * .6;
        if (group_bonus(ch, 2) == 9) {
            dmg -= dmg * 0.1;
        }
        if (AFF_FLAGGED(ch, AFF_POTENT)) {
            dmg += dmg * 0.3;
            send_to_room(IN_ROOM(ch), "@wThere is a bright flash of @Yyellow@w light in the wake of the attack!@n\r\n");
        }
    }

    if (AFF_FLAGGED(ch, AFF_INFUSE) && !AFF_FLAGGED(ch, AFF_HASS) && type <= 0) {
        auto infuse_cost = ch->getPercentOfMaxKI(.005);
        if (dmg > 0) {
            if (ch->getCurKI() - infuse_cost) {
                ch->decCurKI(infuse_cost);
                send_to_room(IN_ROOM(ch), "@CA swirl of ki explodes from the attack!@n\r\n");
            } else {
                act("@wYou can no longer infuse ki into your attacks!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@c$n@w can no longer infuse ki into $s attacks!@n", true, ch, nullptr, nullptr, TO_ROOM);
                ch->affected_by.reset(AFF_INFUSE);
            }
        }
    }

    if (vict) {
        int64_t gain = IS_NPC(ch) ? vict->getExperience() : 0;

        if (GET_ROOM_VNUM(IN_ROOM(vict)) == 17875) {
            return;
        }

        reveal_hiding(vict, 0);
        if (AFF_FLAGGED(vict, AFF_PARALYZE)) {
            send_to_char(ch, "They are a statue and can't be harmed\r\n");
            return;
        }

        if (GET_KAIOKEN(ch) > 0) {
            dmg += (dmg / 100) * (GET_KAIOKEN(ch) * 2);
        }

        if (IS_MUTANT(vict) && (GET_GENOME(vict, 0) == 8 || GET_GENOME(vict, 1) == 8) && type == 0) {
            int64_t drain = dmg * 0.1;
            dmg -= drain;
            ch->decCurST(drain, 1);
            act("@Y$N's rubbery body makes hitting it tiring!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@Y$n's stamina is sapped a bit by hitting your rubbery body!@n", true, ch, nullptr, vict, TO_VICT);
        }


        dmg *= (1.0 + ch->getAffectModifier(APPLY_DAMAGE_PERC));
        dmg *= (1.0 + vict->getAffectModifier(APPLY_DEFENSE_PERC));


        if (type > -1) {
            if (LASTATK(ch) != 11 && LASTATK(ch) != 39 && LASTATK(ch) != 500 && LASTATK(ch) < 1000) {
                if (handle_combo(ch, vict) > 0) {
                    if (beforered <= 1) {
                        COMBO(ch) = -1;
                        COMBHITS(ch) = 0;
                        send_to_char(ch, "@RYou have cut your combo short because you missed your last hit!@n\r\n");
                    } else if (COMBHITS(ch) < physical_mastery(ch)) {
                        dmg += combo_damage(ch, dmg, 0);
                        if ((COMBHITS(ch) == 10 || COMBHITS(ch) == 20 || COMBHITS(ch) == 30) &&
                            (level_exp(ch, GET_LEVEL(ch) + 1) - GET_EXP(ch) > 0 || GET_LEVEL(ch) == 100)) {
                            int64_t gain = GET_LEVEL(ch) * 1000;
                            if (GET_SKILL(ch, SKILL_STYLE) >= 100) {
                                gain += gain * 2;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 80) {
                                gain += gain * 0.4;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 60) {
                                gain += gain * 0.3;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 40) {
                                gain += gain * 0.2;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 20) {
                                gain += gain * 0.1;
                            }
                            auto gained = ch->modExperience(gain);
                            send_to_char(ch, "@D[@mExp@W: @G%s@D]@n\r\n", add_commas(gained).c_str());
                        }
                    } else {
                        dmg += combo_damage(ch, dmg, 1);
                        if ((COMBHITS(ch) == 10 || COMBHITS(ch) == 20 || COMBHITS(ch) == 30) &&
                            (level_exp(ch, GET_LEVEL(ch) + 1) - GET_EXP(ch) > 0 || GET_LEVEL(ch) == 100)) {
                            int64_t gain = GET_LEVEL(ch) * 1000;
                            if (GET_SKILL(ch, SKILL_STYLE) >= 100) {
                                gain += gain * 2;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 80) {
                                gain += gain * 0.4;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 60) {
                                gain += gain * 0.3;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 40) {
                                gain += gain * 0.2;
                            } else if (GET_SKILL(ch, SKILL_STYLE) >= 20) {
                                gain += gain * 0.1;
                            }
                            ch->modExperience(gain);
                            send_to_char(ch, "@D[@mExp@W: @G%s@D]@n\r\n", add_commas(gain).c_str());
                        }
                        COMBO(ch) = -1;
                        COMBHITS(ch) = 0;
                    }
                }
            } else if (COMBHITS(ch) > 0 && LASTATK(ch) < 1000) {
                send_to_char(ch, "@RYou have cut your combo short because you used the wrong attack!@n\r\n");
                COMBO(ch) = -1;
                COMBHITS(ch) = 0;
            }
        }

        if (LASTATK(ch) >= 1000) {
            LASTATK(ch) -= 1000;
        }

        if (GET_PREFERENCE(ch) == PREFERENCE_KI && GET_CHARGE(ch) > 0) {
            dmg -= dmg * 0.08;
        }

        if (AFF_FLAGGED(vict, AFF_SANCTUARY)) {
            if (GET_SKILL(vict, SKILL_AQUA_BARRIER)) {
                if (!SUNKEN(IN_ROOM(ch))) {
                    dmg = dmg * 0.85;
                } else {
                    dmg = dmg * 0.75;
                }
            }
            if (GET_BARRIER(vict) - dmg > 0) {
                act("@c$N's@C barrier absorbs the damage!@n", true, ch, nullptr, vict, TO_CHAR);
                char barr[MAX_INPUT_LENGTH];
                sprintf(barr, "@CYour barrier absorbs the damage! @D[@B%s@D]@n", add_commas(dmg).c_str());
                act(barr, true, ch, nullptr, vict, TO_VICT);
                act("@c$N's@C barrier absorbs the damage!@n", true, ch, nullptr, vict, TO_NOTVICT);
                GET_BARRIER(vict) -= dmg;
                dmg = 0;
            } else if (GET_BARRIER(vict) - dmg <= 0) {
                dmg -= GET_BARRIER(vict);
                GET_BARRIER(vict) = 0;
                act("@c$N's@C barrier bursts!@n", true, ch, nullptr, vict, TO_CHAR);
                act("@CYour barrier bursts!@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$N's@C barrier bursts!@n", true, ch, nullptr, vict, TO_NOTVICT);
                vict->affected_by.reset(AFF_SANCTUARY);
            }
        }
        if (AFF_FLAGGED(vict, AFF_FIRESHIELD) && rand_number(1, 200) < GET_SKILL(vict, SKILL_FIRESHIELD)) {
            act("@c$N's@C fireshield repels the damage!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@CYour fireshield repels the damage!@n", true, ch, nullptr, vict, TO_VICT);
            act("@c$N's@C fireshield repels the damage!@n", true, ch, nullptr, vict, TO_NOTVICT);
            if (rand_number(1, 3) == 3) {
                act("@c$N's@C fireshield disappears...@n", true, ch, nullptr, vict, TO_CHAR);
                act("@CYour fireshield disappears...@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$N's@C fireshield disappears...@n", true, ch, nullptr, vict, TO_NOTVICT);
                vict->affected_by.reset(AFF_FIRESHIELD);
            }
            dmg = 0;
        }

        int64_t conlimit = 2000000000;

        if (type == 0) {
            if (GET_MAX_HIT(vict) < conlimit) {
                index += (GET_MAX_HIT(vict) / 1500) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 2) {
                index += (GET_MAX_HIT(vict) / 2500) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 3) {
                index += (GET_MAX_HIT(vict) / 3500) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 5) {
                index += (GET_MAX_HIT(vict) / 6000) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 10) {
                index += (GET_MAX_HIT(vict) / 8500) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 15) {
                index += (GET_MAX_HIT(vict) / 10000) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 20) {
                index += (GET_MAX_HIT(vict) / 12500) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 25) {
                index += (GET_MAX_HIT(vict) / 16000) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) < conlimit * 30) {
                index += (GET_MAX_HIT(vict) / 22000) * (GET_CON(vict) / 2);
            } else if (GET_MAX_HIT(vict) > conlimit * 30) {
                index += (GET_MAX_HIT(vict) / 25000) * (GET_CON(vict) / 2);
            }
        }

        if (IS_NPC(vict) && GET_LEVEL(vict) > 0) {
            index /= 3;
        } else if (IS_NPC(vict) && GET_LEVEL(vict) < 40) {
            index /= 3;
        }

        index += armor_calc(vict, dmg, type);

        if (AFF_FLAGGED(vict, AFF_STONESKIN)) {
            if (GET_LEVEL(vict) < 20) {
                index += GET_LEVEL(vict) * 250;
            } else if (GET_LEVEL(vict) < 30) {
                index += GET_LEVEL(vict) * 500;
            } else if (GET_LEVEL(vict) < 50) {
                index += GET_LEVEL(vict) * 1000;
            } else if (GET_LEVEL(vict) < 60) {
                index += GET_LEVEL(vict) * 2000;
            } else if (GET_LEVEL(vict) < 70) {
                index += GET_LEVEL(vict) * 5000;
            } else if (GET_LEVEL(vict) < 90) {
                index += GET_LEVEL(vict) * 10000;
            } else if (GET_LEVEL(vict) <= 100) {
                index += GET_LEVEL(vict) * 25000;
            }
        }

        if (AFF_FLAGGED(vict, AFF_SHELL)) {
            dmg -= dmg * 0.25;
        }

        if (AFF_FLAGGED(vict, AFF_WITHER)) {
            dmg += (dmg * 0.01) * 20;
        }

        if (!IS_NPC(vict) && GET_COND(vict, DRUNK) > 4) {
            dmg -= (dmg * 0.001) * GET_COND(vict, DRUNK);
        }

        if (AFF_FLAGGED(vict, AFF_EARMOR)) {
            dmg -= dmg * 0.1;
        }

        if (type > 0) {
            advanced_energy(vict, dmg);
            dmg -= (dmg * 0.0005) * GET_WIS(vict);
        }

        if (IS_MUTANT(vict)) {
            if (type <= 0) {
                dmg -= dmg * 0.3;
            } else if (type > 0) {
                dmg -= dmg * 0.25;
            }
        }

        if (GET_BONUS(vict, BONUS_THICKSKIN)) {
            if (type <= 0) {
                dmg -= dmg * 0.20;
            } else {
                dmg -= dmg * 0.10;
            }
        } else if (GET_BONUS(vict, BONUS_THINSKIN)) {
            if (type <= 0) {
                dmg += dmg * 0.20;
            } else {
                dmg += dmg * 0.10;
            }
        }

        if (PLR_FLAGGED(vict, PLR_FURY)) {
            dmg -= dmg * 0.1;
        }

        if (IS_MAJIN(vict)) {
            if (type <= 0) {
                dmg -= dmg * 0.5;
            }
        }

        if (IS_KAI(vict)) {
            dmg += dmg * 0.15;
        }

        if (GRAPPLING(ch) == vict && GRAPTYPE(ch) == 3) {
            dmg += (dmg / 100) * 20;
        }

        if (GET_CLAN(vict) != nullptr && !strcasecmp(GET_CLAN(vict), "Heavenly Kaios")) {
            if ((vict->getCurKI()) >= GET_MAX_MANA(vict) / 2) {
                dmg -= (dmg / 100) * 20;
                act("@wYou are covered in a pristine @Cglow@w.@n", true, vict, nullptr, nullptr, TO_CHAR);
                act("@w$n is covered in a pristine @Cglow@w!@n", true, vict, nullptr, nullptr, TO_ROOM);
            }
        }

        if (!IS_NPC(vict) && GET_SKILL(vict, SKILL_ARMOR)) {
            int nanite = GET_SKILL(vict, SKILL_ARMOR), perc = rand_number(1, 220);
            if (PLR_FLAGGED(vict, PLR_SENSEM)) {
                perc = rand_number(1, 176);
            }
            if (nanite >= perc) {
                std::string amount = "none";
                int mult = 0;
                switch(vict->form) {
                    case FormID::Base:
                        mult = 5;
                        amount = "a tiny bit";
                        break;
                    case FormID::Android10:
                        mult = 10;
                        amount = "a bit";
                        break;
                    case FormID::Android20:
                        mult = 20;
                        amount = "some";
                        break;
                    case FormID::Android30:
                        mult = 25;
                        amount = "a good deal";
                        break;
                    case FormID::Android40:
                        mult = 30;
                        amount = "a lot";
                        break;
                    case FormID::Android50:
                        mult = 40;
                        amount = "a great deal";
                        break;
                    case FormID::Android60:
                        mult = 50;
                        amount = "MOST";
                        break;
                }

                auto victMsg = fmt::format("@WYour @gn@Ga@Wn@wite @Da@Wr@wm@Do@wr@W reacts in time to block {} of the damage!@n", amount);
                auto roomMsg = fmt::format("@W$n's @gn@Ga@Wn@wite @Da@Wr@wm@Do@wr@W reacts in time to block {} of the damage!@n", amount);
                act(victMsg.c_str(), true,
                    vict, nullptr, nullptr, TO_CHAR);
                act(roomMsg.c_str(), true,
                    vict, nullptr, nullptr, TO_ROOM);
                dmg -= (dmg * 0.01) * mult;
            }
        }

        if (!AFF_FLAGGED(vict, AFF_KNOCKED) && (GET_POS(vict) == POS_SITTING || GET_POS(vict) == POS_RESTING) &&
            GET_SKILL(vict, SKILL_ROLL) > axion_dice(0)) {
            int64_t rollcost = (vict->getMaxHealth() / 300) * (GET_STR(ch) / 2);
            if ((vict->getCurST()) >= rollcost) {
                act("@GYou roll to your feet in an agile fashion!@n", true, vict, nullptr, nullptr, TO_CHAR);
                act("@G$n rolls to $s feet in an agile fashion!@n", true, vict, nullptr, nullptr, TO_ROOM);
                do_stand(vict, nullptr, 0, 0);
                vict->decCurST(rollcost);
            }
        }

        if (IS_NPC(vict)) {
            hitprcnt_mtrigger(vict);
        }

        if (IS_HUMANOID(vict) && !IS_NPC(ch) && IS_NPC(vict) && (!is_sparring(ch) || !is_sparring(vict))) {
            remember(vict, ch);
        }
        if (IS_NPC(vict) && GET_HIT(vict) > ((vict->getEffMaxPL())) / 4) {
            LASTHIT(vict) = GET_IDNUM(ch);
        }
        if (AFF_FLAGGED(vict, AFF_SLEEP) && rand_number(1, 2) == 2) {
            affect_from_char(vict, SPELL_SLEEP);
            act("@c$N@W seems to be more aware now.@n", true, ch, nullptr, vict, TO_CHAR);
            act("@WYou are no longer so sleepy.@n", true, ch, nullptr, vict, TO_VICT);
            act("@c$N@W seems to be more aware now.@n", true, ch, nullptr, vict, TO_NOTVICT);
        }
        if (AFF_FLAGGED(vict, AFF_KNOCKED) && rand_number(1, 12) >= 11) {
            vict->cureStatusKnockedOut(true);
            if (IS_NPC(vict) && rand_number(1, 20) >= 12) {
                act("@W$n@W stands up.@n", false, vict, nullptr, nullptr, TO_ROOM);
                GET_POS(vict) = POS_STANDING;
            }
        }
        if (IS_NPC(ch)) {
            if (GET_LEVEL(ch) > 10) {
                if (dmg - index > 0) {
                    dmg -= index;
                } else if (dmg - index <= 0 && dmg >= 1) {
                    dmg = 1;
                }
            } else if (GET_LEVEL(ch) <= 10) {
                dmg = (dmg * .8);
            }
        }
        if (!IS_NPC(ch)) {
            if (dmg >= 1) {
                if ((dmg + (dmg * 0.5)) - index <= 0) {
                    dmg = 1;
                } else if ((dmg + (dmg * 0.4)) - index <= 0) {
                    dmg = dmg * 0.04;
                } else if ((dmg + (dmg * 0.3)) - index <= 0) {
                    dmg = dmg * 0.08;
                } else if ((dmg + (dmg * 0.2)) - index <= 0) {
                    dmg = dmg * 0.12;
                } else if ((dmg + (dmg * 0.1)) - index <= 0) {
                    dmg = dmg * 0.16;
                } else if (dmg - index <= 0) {
                    dmg = dmg * 0.2;
                } else if (dmg - index > dmg * 0.25) {
                    dmg -= index;
                } else {
                    dmg = dmg * 0.25;
                }
            }
        }
        if (dmg < 1) {
            dmg = 0;
        }
        if (dmg >= 50 && chance > 0) {
            hurt_limb(ch, vict, chance, limb, dmg);
        }

        if (IS_NPC(vict) && dmg > vict->getMaxHealth() * .7 && GET_BONUS(ch, BONUS_SADISTIC) > 0) {
            gain /= 2;
        } else if (IS_NPC(vict) && dmg > vict->getCurHealth() && vict->isFullHealth() * .5 &&
                   GET_BONUS(ch, BONUS_SADISTIC) > 0) {
            gain /= 2;
        }


        if (CARRYING(vict) && dmg > (((vict->getEffMaxPL())) * 0.01) && rand_number(1, 10) >= 8) {
            carry_drop(vict, 2);
        }

        if (GET_POS(vict) == POS_SITTING && IS_NPC(vict) && vict->getCurHealth() >= ((vict->getEffMaxPL())) * .98) {
            do_stand(vict, nullptr, 0, 0);
        }
        bool suppresso = (GET_SUPPRESS(vict) > 0);
        if (is_sparring(ch) && is_sparring(vict) && (GET_SUPPRESS(vict) + vict->getCurHealth()) - dmg <= 0) {
            if (!IS_NPC(vict)) {
                act("@c$N@w falls down unconscious, and you stop sparring with $M.@n", true, ch, nullptr, vict,
                    TO_CHAR);
                act("@C$n@w stops sparring with you as you fall unconscious.@n", true, ch, nullptr, vict, TO_VICT);
                act("@c$N@w falls down unconscious, and @C$n@w stops sparring with $M.@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                vict->setCurHealth(1);
                if (GET_SUPPRESS(vict)) {
                    GET_SUPPRESS(vict) = 0;
                }
                if (FIGHTING(vict)) {
                    stop_fighting(vict);
                }
                if (FIGHTING(ch)) {
                    stop_fighting(ch);
                }
                GET_POS(vict) = POS_SLEEPING;
                if (!IS_NPC(ch)) {
                    vict->affected_by.set(AFF_KNOCKED);
                }
            } else {
                act("@c$N@w admits defeat to you, stops sparring, and stumbles away.@n", true, ch, nullptr, vict,
                    TO_CHAR);
                act("@c$N@w admits defeat to $n, stops sparring, and stumbles away.@n", true, ch, nullptr, vict,
                    TO_NOTVICT);
                solo_gain(ch, vict);
                struct obj_data *rew, *next_rew;
                int founded = 0;
                for (rew = vict->contents; rew; rew = next_rew) {
                    next_rew = rew->next_content;
                    if (rew) {
                        obj_from_char(rew);
                        obj_to_room(rew, IN_ROOM(vict));
                        founded = 1;
                    }
                }
                if (founded == 1) {
                    act("@c$N@w leaves a reward behind out of respect.@n", true, ch, nullptr, vict, TO_CHAR);
                }
                vict->setCurHealth(0);
                extract_char(vict);
                return;
            }
        } else if (is_sparring(ch) && (GET_SUPPRESS(vict) + GET_HIT(vict)) - dmg <= 0) {
            act("@c$N@w falls down unconscious, and you spare $S life.@n", true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@w spares your life as you fall unconscious.@n", true, ch, nullptr, vict, TO_VICT);
            act("@c$N@w falls down unconscious, and @C$n@w spares $S life.@n", true, ch, nullptr, vict, TO_NOTVICT);
            vict->setCurHealth(1);
            if (FIGHTING(vict)) {
                stop_fighting(vict);
            }
            if (FIGHTING(ch)) {
                stop_fighting(ch);
            }
            GET_POS(vict) = POS_SLEEPING;
            if (!IS_NPC(ch)) {
                vict->affected_by.set(AFF_KNOCKED);
            }
        } else if (is_sparring(ch) && !is_sparring(vict) && IS_NPC(ch)) {
            act("@w$n@w stops sparring!@n", true, ch, nullptr, vict, TO_ROOM);
            ch->mobFlags.reset(MOB_SPAR);
        } else if (!is_sparring(ch) && is_sparring(vict) && IS_NPC(vict)) {
            act("@w$n@w stops sparring!@n", true, ch, nullptr, vict, TO_ROOM);
            vict->mobFlags.reset(MOB_SPAR);
        }

        if (PLR_FLAGGED(vict, PLR_IMMORTAL) && !is_sparring(ch) && vict->getCurHealth() - dmg <= 0) {
            if (IN_ARENA(vict)) {
                send_to_all("@R%s@r manages to defeat @R%s@r in the Arena!@n\r\n", GET_NAME(ch), GET_NAME(vict));
                char_from_room(ch);
                char_to_room(ch, real_room(17875));
                look_at_room(IN_ROOM(ch), ch, 0);
                char_from_room(vict);
                char_to_room(vict, real_room(17875));
                vict->setCurHealth(1);
                look_at_room(IN_ROOM(vict), vict, 0);
                if (FIGHTING(vict)) {
                    stop_fighting(vict);
                }
                if (FIGHTING(ch)) {
                    stop_fighting(ch);
                }
                return;
            } else {
                act("@c$N@w disappears right before dying. $N appears to be immortal.@n", true, ch, nullptr, vict,
                    TO_CHAR);
                act("@CYou disappear right before death, having been saved by your immortality.@n", true, ch, nullptr,
                    vict, TO_VICT);
                act("@c$N@w disappears right before dying. $N appears to be immortal.@n.", true, ch, nullptr, vict,
                    TO_NOTVICT);
                vict->decCurHealthPercent(1, 1);
                vict->decCurSTPercent(1, 1);
                vict->decCurKIPercent(1, 1);

                if (FIGHTING(vict)) {
                    stop_fighting(vict);
                }
                if (FIGHTING(ch)) {
                    stop_fighting(ch);
                }
                GET_POS(vict) = POS_SITTING;
                char_from_room(vict);
                char_to_room(vict, real_room(sensei::getStartRoom(vict->chclass)));
            }
            return;
        }

        if (GRAPPLING(vict) && GRAPPLING(vict) != ch && type == 1) {
            act("@YThe attack hurts YOU as well because you are grappling with $M!@n", true, vict, nullptr,
                GRAPPLING(vict), TO_VICT);
            act("@YThe attack hurts @y$N@Y as well because $n is grappling with $m!@n", true, vict, nullptr,
                GRAPPLING(vict), TO_NOTVICT);
            maindmg = maindmg / 2;
            hurt(0, 0, ch, GRAPPLING(vict), nullptr, maindmg, 3);
        }
        if (GRAPPLED(vict) && GRAPPLED(vict) != ch && type == 1) {
            act("@YThe attack hurts YOU as well because you are being grappled by $M!@n", true, vict, nullptr,
                GRAPPLED(vict), TO_VICT);
            act("@YThe attack hurts @y$N@Y as well because $n is being grappled by $m!@n", true, vict, nullptr,
                GRAPPLED(vict), TO_NOTVICT);
            maindmg = maindmg / 2;
            hurt(0, 0, ch, GRAPPLED(vict), nullptr, maindmg, 3);
        }
        if (!is_sparring(ch) && !PLR_FLAGGED(vict, PLR_IMMORTAL) && GET_HIT(vict) - dmg <= 0) {
            if ((GET_HIT(vict) - dmg <= 0 && suppresso == false) || (suppresso == true && (GET_HIT(vict) * GET_SUPPRESS(ch)) - dmg <= 0)) {
                vict->decCurHealthPercent(1, 0);
                if (!IS_NPC(vict) && (vict->getCurLF()) - (dmg - GET_HIT(vict)) >= 0) {
                    act("@c$N@w barely clings to life!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@CYou barely cling to life!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@c$N@w barely clings to life!@n.", true, ch, nullptr, vict, TO_NOTVICT);
                    int64_t lifeloss = dmg - GET_HIT(vict);
                    vict->decCurLF(lifeloss);
                    send_to_char(vict, "@D[@CLifeforce@D: @R-%s@D]\n", add_commas(lifeloss).c_str());
                    if ((vict->getCurLF()) >= (vict->getMaxLF()) * 0.05) {
                        send_to_char(vict, "@YYou recover a bit thanks to your strong life force.@n\r\n");
                        vict->incCurHealth((vict->getMaxLF()) * .05);
                        vict->decCurLFPercent(.05);
                    } else {
                        vict->incCurHealth(GET_LEVEL(vict) * 100);
                    }
                    return;
                }
                if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                    GET_DEATH_TYPE(vict) = 0;
                }
                if (type <= 0 && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    handle_death_msg(ch, vict, 0);
                } else if (type > 0 && (!IS_NPC(vict) || !MOB_FLAGGED(vict, MOB_DUMMY))) {
                    handle_death_msg(ch, vict, 1);
                } else {
                    act("@R$N@w self destructs with a mild explosion!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@R$N@w self destructs with a mild explosion!@n", true, ch, nullptr, vict, TO_ROOM);
                }
                if (dmg > 1) {
                    if (type <= 0 && GET_HIT(ch) >= ((ch->getEffMaxPL()) * 0.5)) {
                        int64_t raise = (GET_MAX_MANA(ch) * 0.005) + 1;
                        ch->incCurKI(raise);
                    }
                    send_to_char(ch, "@D[@GDamage@W: @R%s@D]@n\r\n", add_commas(dmg).c_str());
                    send_to_char(vict, "@D[@rDamage@W: @R%s@D]@n\r\n", add_commas(dmg).c_str());
                    int64_t healhp = (long double) (GET_MAX_HIT(vict)) * 0.12;
                    if (AFF_FLAGGED(ch, AFF_METAMORPH) && GET_HIT(ch) <= GET_MAX_HIT(ch)) {
                        act("@RYour dark aura saps some of @r$N's@R life energy!@n", true, ch, nullptr, vict, TO_CHAR);
                        act("@r$n@R's dark aura saps some of your life energy!@n", true, ch, nullptr, vict, TO_VICT);
                        ch->incCurHealth(healhp);
                    }
                    if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 10 || GET_GENOME(ch, 1) == 10)) {
                        ch->incCurKI(dmg * .05);
                    }
                    if (!is_sparring(ch) && IS_NPC(vict)) {
                        if (type == 0 && rand_number(1, 100) >= 97) {
                            send_to_char(ch,
                                         "@YY@yo@Yu @yg@Ya@yi@Yn@y s@Yo@ym@Ye @yb@Yo@yn@Yu@ys @Ye@yx@Yp@ye@Yr@yi@Ye@yn@Yc@ye@Y!@n\r\n");
                            gain = gain * 0.05;
                            gain += 1;
                            ch->modExperience(gain);
                        } else if (type != 0 && rand_number(1, 100) >= 93) {
                            gain = gain * 0.05;
                            gain += 1;
                            ch->modExperience(gain);
                        }
                    }
                    if (AFF_FLAGGED(vict, AFF_ECHAINS)) {
                        if(type == 0) {
                            if(IS_NPC(ch)) {
                                assign_affect(ch, AFF_ECHAINS_DEBUFF, SKILL_RUNIC, -1, 0, 0, 0, 0, 0, -2);
                            } else {
                                WAIT_STATE(ch, PULSE_3SEC);
                            }
                            act("@CEthereal chains burn into existence! They quickly latch onto @RYOUR@C body and begin temporarily hampering $s actions!@n",
                                    true, ch, nullptr, vict, TO_CHAR);
                            act("@CEthereal chains burn into existence! They quickly latch onto @c$n's@C body and begin temporarily hampering $s actions!@n",
                                true, ch, nullptr, vict, TO_ROOM);
                        }
                    }
                } else if (dmg <= 1) {
                    send_to_char(ch, "@D[@GDamage@W: @BPitiful...@D]@n\r\n");
                    send_to_char(vict, "@D[@rDamage@W: @BPitiful...@D]@n\r\n");
                }

                vict->decCurHealthPercent(1, 0);

                if (AFF_FLAGGED(ch, AFF_GROUP)) {
                    group_gain(ch, vict);
                } else {
                    solo_gain(ch, vict);
                }
                if (IS_DEMON(ch) && type == 1) {
                    vict->mobFlags.set(AFF_ASHED);
                }
                die(vict, ch);
                dead = true;
            }
        } else if (GET_HIT(vict) - dmg > 0 || suppresso == true) {
            if (suppresso == false) {
                vict->decCurHealth(dmg);
            }
            if (FIGHTING(ch) == nullptr) {
                set_fighting(ch, vict);
            } else if (FIGHTING(ch) != vict) {
                set_fighting(ch, vict);
            }
            if (FIGHTING(vict) == nullptr) {
                set_fighting(vict, ch);
            } else if (FIGHTING(vict) != ch) {
                set_fighting(vict, ch);
            }
            if (dmg > 1 && suppresso == false) {
                if (type == 0 && GET_HIT(ch) >= ((ch->getEffMaxPL()) * 0.5)) {
                    int64_t raise = (GET_MAX_MANA(ch) * 0.005) + 1;
                    ch->incCurKI(raise);
                }
                if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 10 || GET_GENOME(ch, 1) == 10)) {
                    ch->incCurKI(dmg * .05);
                }
                send_to_char(ch, "@D[@GDamage@W: @R%s@D]@n", add_commas(dmg).c_str());
                send_to_char(vict, "@D[@rDamage@W: @R%s@D]@n\r\n", add_commas(dmg).c_str());
                //int64_t healhp = GET_HIT(vict) * 0.12;
                if (GET_EQ(ch, WEAR_EYE) && vict && !PRF_FLAGGED(ch, PRF_NODEC)) {
                    if (IS_ANDROID(vict)) {
                        send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                    } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_BSCOUTER) && GET_HIT(vict) >= 150000) {
                        send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                    } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_MSCOUTER) && GET_HIT(vict) >= 5000000) {
                        send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                    } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_ASCOUTER) && GET_HIT(vict) >= 15000000) {
                        send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                    } else {
                        send_to_char(ch, " @D<@YProcessing@D: @c%s@D>@n\r\n", add_commas(GET_HIT(vict)).c_str());
                    }
                } else {
                    send_to_char(ch, "\r\n");
                }
            } else if (!IS_NPC(ch)) {
                if (dmg <= 1 && suppresso == false && !PRF_FLAGGED(ch, PRF_NODEC)) {
                    send_to_char(ch, "@D[@GDamage@W: @BPitiful...@D]@n");
                    send_to_char(vict, "@D[@rDamage@W: @BPitiful...@D]@n\r\n");
                    if (GET_EQ(ch, WEAR_EYE) && vict && !PRF_FLAGGED(ch, PRF_NODEC)) {
                        if (IS_ANDROID(vict)) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_BSCOUTER) && GET_HIT(vict) >= 150000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_MSCOUTER) && GET_HIT(vict) >= 5000000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_ASCOUTER) && GET_HIT(vict) >= 15000000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else {
                            send_to_char(ch, " @D<@YProcessing@D: @c%s@D>@n\r\n", add_commas(GET_HIT(vict)).c_str());
                        }
                    } else {
                        send_to_char(ch, "\r\n");
                    }
                } else if (dmg > 1 && suppresso == true && !PRF_FLAGGED(ch, PRF_NODEC)) {
                    send_to_char(ch, "@D[@GDamage@W: @R%s@D]@n", add_commas(dmg).c_str());
                    send_to_char(vict, "@D[@rDamage@W: @R%s @c-Suppression-@D]@n\r\n", add_commas(dmg).c_str());
                    //int64_t healhp = GET_HIT(vict) * 0.12;
                    //Translate the damage into a percentage of max LF, remove that from the player instead
                    int percentageDamage = GET_MAX_HIT(vict) / dmg;
                    vict->decCurLFPercent(percentageDamage);

                    if (GET_EQ(ch, WEAR_EYE) && vict && !PRF_FLAGGED(ch, PRF_NODEC)) {
                        if (IS_ANDROID(vict)) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_BSCOUTER) && GET_HIT(vict) >= 150000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_MSCOUTER) && GET_HIT(vict) >= 5000000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_ASCOUTER) && GET_HIT(vict) >= 15000000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else {
                            send_to_char(ch, " @D<@YProcessing@D: @c%s@D>@n\r\n", add_commas(GET_HIT(vict)).c_str());
                        }
                    } else {
                        send_to_char(ch, "\r\n");
                    }
                } else if (dmg <= 1 && suppresso == true && !PRF_FLAGGED(ch, PRF_NODEC)) {
                    send_to_char(ch, "@D[@GDamage@W: @BPitiful...@D]@n");
                    if (GET_EQ(ch, WEAR_EYE) && vict) {
                        if (IS_ANDROID(vict) && !PRF_FLAGGED(ch, PRF_NODEC)) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_BSCOUTER) && GET_HIT(vict) >= 150000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_MSCOUTER) && GET_HIT(vict) >= 5000000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else if (OBJ_FLAGGED(GET_EQ(ch, WEAR_EYE), ITEM_ASCOUTER) && GET_HIT(vict) >= 15000000) {
                            send_to_char(ch, " @D<@YProcessing@D: @c?????????????@D>@n\r\n");
                        } else {
                            send_to_char(ch, " @D<@YProcessing@D: @c%s@D>@n\r\n", add_commas(GET_HIT(vict)).c_str());
                        }
                    } else {
                        send_to_char(ch, "\r\n");
                    }
                }
            }
        }
        if (GET_SKILL(ch, SKILL_FOCUS) && type == 1) {
            improve_skill(ch, SKILL_FOCUS, 1);
        }

        /* Increases GET_FURY for halfbreeds who get damaged. */

        if (!is_sparring(ch) && IS_HALFBREED(vict) && GET_FURY(vict) < 100 && !PLR_FLAGGED(vict, PLR_FURY)) {
            send_to_char(vict, "@RYour fury increases a little bit!@n\r\n");
            GET_FURY(vict) += 1;
        }

        /* Ends GET_FURY increase for halfbreeds who got damaged */

        if (LASTATK(ch) != -1) {
            spar_gain(ch, vict, type, dmg);
        }

        if(dmg > 0 && !IS_NPC(ch)) {
            if((IS_SAIYAN(ch) || (IS_BIO(ch) && (GET_GENOME(ch, 0) == 2 || GET_GENOME(ch, 1) == 2)))) {
                if (GET_POS(ch) != POS_RESTING && GET_POS(vict) != POS_RESTING) {
                    saiyan_gain(ch, vict);
                }
            }
        }

        if (IS_ARLIAN(vict) && dead != true && !is_sparring(vict) && !is_sparring(ch)) {
            handle_evolution(vict, dmg);
        }
        if (dead == true) {
            char corp[256];
            if (!PLR_FLAGGED(ch, PLR_SELFD2)) {
                if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
                    sprintf(corp, "all.zenni corpse");
                    do_get(ch, corp, 0, 0);
                }
                if (!IS_NPC(ch) && (ch != vict) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
                    sprintf(corp, "all corpse");
                    do_get(ch, corp, 0, 0);
                }
            }
        }
    }
        /* If an object is targeted */
    else if (obj) {
        switch (GET_OBJ_VAL(obj, VAL_ALL_MATERIAL)) {
            case MATERIAL_STEEL:
                dmg = dmg / 4;
                break;
            case MATERIAL_MITHRIL:
                dmg = dmg / 6;
                break;
            case MATERIAL_IRON:
                dmg = dmg / 3;
                break;
            case MATERIAL_STONE:
                dmg = dmg / 2;
                break;
            case MATERIAL_DIAMOND:
                dmg = dmg / 10;
                break;
        }
        if (dmg <= 0) {
            dmg = 1;
        }
        if (OBJ_FLAGGED(obj, ITEM_UNBREAKABLE)) {
            act("$p@w seems unaffected.@n", true, ch, obj, nullptr, TO_CHAR);
            act("$p@w seems unaffected.@n", true, ch, obj, nullptr, TO_ROOM);
        } else if (GET_OBJ_VNUM(obj) == 79) {
            if (GET_OBJ_WEIGHT(obj) - dmg > 0) {
                if (type <= 0) {
                    if (AFF_FLAGGED(ch, AFF_INFUSE))
                        dmg *= 10;
                    act("$p@w cracks some.@n", true, ch, obj, nullptr, TO_CHAR);
                    act("$p@w cracks some.@n", true, ch, obj, nullptr, TO_ROOM);
                    GET_OBJ_WEIGHT(obj) -= dmg;
                    if (GET_FELLOW_WALL(obj)) {
                        struct obj_data *wall;
                        wall = GET_FELLOW_WALL(obj);
                        GET_OBJ_WEIGHT(wall) -= dmg;
                        act("$p@w cracks some. A humanoid shadow can be seen moving on the other side.@n", true,
                            nullptr, obj, nullptr, TO_ROOM);
                    }
                } else {
                    dmg *= 30;
                    act("$p@w melts some.@n", true, ch, obj, nullptr, TO_CHAR);
                    act("$p@w melts some.@n", true, ch, obj, nullptr, TO_ROOM);
                    GET_OBJ_WEIGHT(obj) -= dmg;
                    if (GET_FELLOW_WALL(obj)) {
                        struct obj_data *wall;
                        wall = GET_FELLOW_WALL(obj);
                        GET_OBJ_WEIGHT(wall) -= dmg;
                        act("$p@w melts some.@n", true, ch, obj, nullptr, TO_ROOM);
                    }
                }
            } else {
                if (type <= 0) {
                    act("$p@w breaks completely apart and then melts away.@n", true, ch, obj, nullptr, TO_CHAR);
                    act("$p@w breaks completely apart and then melts away.@n", true, ch, obj, nullptr, TO_ROOM);
                    extract_obj(obj);
                } else {
                    act("$p@w is blown away into snow and water!@n", true, ch, obj, nullptr, TO_CHAR);
                    act("$p@w is blown away into snow and water!@n", true, ch, obj, nullptr, TO_ROOM);
                    extract_obj(obj);
                }
            }
        } else if (GET_OBJ_VAL(obj, VAL_ALL_HEALTH) - dmg > 0) {
            act("$p@w cracks some.@n", true, ch, obj, nullptr, TO_CHAR);
            act("$p@w cracks some.@n", true, ch, obj, nullptr, TO_ROOM);
            GET_OBJ_VAL(obj, VAL_ALL_HEALTH) -= dmg;
        } else {
            if (type <= 0) {
                act("$p@w shatters apart!@n", true, ch, obj, nullptr, TO_CHAR);
                act("$p@w shatters apart!@n", true, ch, obj, nullptr, TO_ROOM);
                GET_OBJ_VAL(obj, VAL_ALL_HEALTH) = 0;
                obj->extra_flags.set(ITEM_BROKEN);
                if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON && GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) {
                    GET_OBJ_VAL(obj, VAL_DRINKCON_HOWFULL) = 0;
                }
            } else if (type != 0) {
                act("$p@w is disintegrated!@n", true, ch, obj, nullptr, TO_CHAR);
                act("$p@w is disintegrated!@n", true, ch, obj, nullptr, TO_ROOM);
                extract_obj(obj);
            }
        }
    } else {
        basic_mud_log("Log: Error with hurt.\n");
    }
}

/* This handles the length of time between attacks and other actions *
* players AND non-players will have to endure. Allowing for a more   *
* balanced and interesting attack cooldown system. - Iovan 2/25/2011 */
void handle_cooldown(struct char_data *ch, int cooldown) {

    /* Let's clear any cooldown they may accidently have so it doesn't stack *
   * This is only for NPCs as player cooldown is handled through the stock *
   * descriptor_list command interpreter. This also initializes cooldown   */

    if (!IS_NPC(ch)) {
        if (PLR_FLAGGED(ch, PLR_MULTIHIT)) {
            ch->playerFlags.reset(PLR_MULTIHIT);
            return;
        }
    }

    if (IS_NPC(ch)) {
        MOB_COOLDOWN(ch) = 0;
    }

    reveal_hiding(ch, 0);
    int waitCalc = 10, base = cooldown;
    int64_t cspd = 0;

    /* Ok calculating speed. */
    cspd = GET_SPEEDI(ch);

    if (cspd > 10000000) { /* WTF Fast */
        waitCalc -= 9;
    } else if (cspd > 5000000) {
        waitCalc -= 8;
    } else if (cspd > 2500000) {
        waitCalc -= 7;
    } else if (cspd > 1000000) {
        waitCalc -= 6;
    } else if (cspd > 500000) {
        waitCalc -= 5;
    } else if (cspd > 100000) {
        waitCalc -= 4;
    } else if (cspd > 50000) {
        waitCalc -= 3;
    } else if (cspd > 25000) {
        waitCalc -= 2;
    } else if (cspd > 50) {
        waitCalc -= 1;
    }

    base *= 10;

    if (base >= 100) {
        base = 30;
    } else if (base >= 70) {
        base = 20;
    } else if (base >= 30) {
        base = 10;
    }

    /* Alright now let's determine the cooldown based on the wait and cooldown assigned *
   * by the attack which called handle_cooldown.                                      */
    if (!IS_NPC(ch)) {
        cooldown *= waitCalc;
        cooldown += base;
        if (cooldown <= 0) { /* Can't have this. */
            cooldown = 10;
        }
        if (cooldown >= 120) {
            WAIT_STATE(ch, PULSE_CD12);
        } else if (cooldown >= 110) {
            WAIT_STATE(ch, PULSE_CD11);
        } else if (cooldown >= 100) {
            WAIT_STATE(ch, PULSE_CD10);
        } else if (cooldown >= 90) {
            WAIT_STATE(ch, PULSE_CD9);
        } else if (cooldown >= 80) {
            WAIT_STATE(ch, PULSE_CD8);
        } else if (cooldown >= 70) {
            WAIT_STATE(ch, PULSE_CD7);
        } else if (cooldown >= 60) {
            WAIT_STATE(ch, PULSE_CD6);
        } else if (cooldown >= 50) {
            WAIT_STATE(ch, PULSE_CD5);
        } else if (cooldown >= 40) {
            WAIT_STATE(ch, PULSE_CD4);
        } else if (cooldown >= 30) {
            WAIT_STATE(ch, PULSE_CD3);
        } else if (cooldown >= 20) {
            WAIT_STATE(ch, PULSE_CD2);
        } else {
            WAIT_STATE(ch, PULSE_CD1);
        }

    } else { /* We handle NPCs differently. */
        cooldown *= waitCalc;
        cooldown += base;
        if (cooldown >= 120) {
            MOB_COOLDOWN(ch) = 12;
        } else if (cooldown >= 110) {
            MOB_COOLDOWN(ch) = 11;
        } else if (cooldown >= 100) {
            MOB_COOLDOWN(ch) = 10;
        } else if (cooldown >= 90) {
            MOB_COOLDOWN(ch) = 9;
        } else if (cooldown >= 80) {
            MOB_COOLDOWN(ch) = 8;
        } else if (cooldown >= 70) {
            MOB_COOLDOWN(ch) = 7;
        } else if (cooldown >= 60) {
            MOB_COOLDOWN(ch) = 6;
        } else if (cooldown >= 50) {
            MOB_COOLDOWN(ch) = 5;
        } else if (cooldown >= 40) {
            MOB_COOLDOWN(ch) = 4;
        } else if (cooldown >= 30) {
            MOB_COOLDOWN(ch) = 3;
        } else if (cooldown >= 20) {
            MOB_COOLDOWN(ch) = 2;
        } else {
            MOB_COOLDOWN(ch) = 1;
        }
    }
}

/* This handles whether parry is turned on */
int handle_parry(struct char_data *ch) {

    if (axion_dice(0) <= 4) { /* Critical failure */
        return (1);
    }

    if (IS_NPC(ch)) {

        /*  Non-humanoids are only rarely capable of parrying against weak players. Never
   * against strong players. Humanoids get progressively better at parry the higher
   * level they are.
   **/
        if (!IS_HUMANOID(ch)) {
            return (rand_number(0, 5));
        } else {
            if (GET_LEVEL(ch) >= 110) {
                return (rand_number(90, 105));
            } else if (GET_LEVEL(ch) >= 100) {
                return (rand_number(85, 95));
            } else if (GET_LEVEL(ch) >= 80) {
                return (rand_number(70, 85));
            } else if (GET_LEVEL(ch) >= 40) {
                return (rand_number(50, 70));
            } else {
                int top = GET_LEVEL(ch);

                if (top < 15)
                    top = 16;
                return (rand_number(15, top));
            }
        }
    }

    if (PRF_FLAGGED(ch, PRF_NOPARRY)) {
        return (-2);
    } else {
        int num = GET_SKILL(ch, SKILL_PARRY);
        if (IS_MUTANT(ch) && (GET_GENOME(ch, 0) == 3 || GET_GENOME(ch, 1) == 3)) {
            num += 10;
        }
        if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 100) {
            num += 5;
        } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 80) {
            num += 4;
        } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 60) {
            num += 3;
        } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 40) {
            num += 2;
        } else if (GET_SKILL_BASE(ch, SKILL_STYLE) >= 20) {
            num += 1;
        }
        return (num);
    }

}

/* This handles whether a step of the combo was preformed. */
int handle_combo(struct char_data *ch, struct char_data *vict) {
    int success = false, pass = false;

    if (IS_NPC(ch))
        return 0;

    switch (LASTATK(ch)) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
        case 51:
        case 52:
        case 56:
            pass = true;
            break;
        default:
            if (COMBO(ch) != -1) {
                send_to_char(ch, "@RYou have cut your combo short with the wrong attack!@n\r\n");
            }
            COMBO(ch) = -1;
            COMBHITS(ch) = 0;
            pass = false;
            break;
    }

    if (pass == false)
        return 0;

    if (count_physical(ch) < 3)
        return 0;

    int chance;
    int speedPercentage = ((double) (GET_SPEEDI(ch)) / (double) GET_SPEEDI(vict)) * (double) 100;
    if (speedPercentage < 1) {
        chance = 1;
    } else if (speedPercentage > 100) {
        chance = 100;
    } else {
        chance = speedPercentage;
    }
    chance = 100 - chance;
    if (chance < 1) {
        chance = 1;
    }
    chance += 25;
    if (LASTATK(ch) == 0 || LASTATK(ch) == 1) {
        chance -= 10;
    }
    if (LASTATK(ch) == 2 || LASTATK(ch) == 3) {
        chance -= 5;
    }

    if (COMBO(ch) <= -1 && rand_number(1, 100) > chance) {
        while (success == false) {
            switch (rand_number(1, 24)) {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    if (GET_SKILL(ch, SKILL_PUNCH) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try a@R punch @Gnext!@n\r\n");
                        COMBO(ch) = 0;
                        success = true;
                    }
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                    if (GET_SKILL(ch, SKILL_KICK) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try a@R kick @Gnext!@n\r\n");
                        COMBO(ch) = 1;
                        success = true;
                    }
                    break;
                case 11:
                case 12:
                case 13:
                case 14:
                    if (GET_SKILL(ch, SKILL_ELBOW) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try an@R elbow @Gnext!@n\r\n");
                        COMBO(ch) = 2;
                        success = true;
                    }
                    break;
                case 15:
                case 16:
                case 17:
                    if (GET_SKILL(ch, SKILL_KNEE) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try a@R knee @Gnext!@n\r\n");
                        COMBO(ch) = 3;
                        success = true;
                    }
                    break;
                case 18:
                case 19:
                    if (GET_SKILL(ch, SKILL_ROUNDHOUSE) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try a@R roundhouse @Gnext!@n\r\n");
                        COMBO(ch) = 4;
                        success = true;
                    }
                    break;
                case 20:
                case 21:
                    if (GET_SKILL(ch, SKILL_UPPERCUT) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try an@R uppercut @Gnext!@n\r\n");
                        COMBO(ch) = 5;
                        success = true;
                    }
                    break;
                case 22:
                    if (GET_SKILL(ch, SKILL_HEELDROP) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try a@R heeldrop @Gnext!@n\r\n");
                        COMBO(ch) = 8;
                        success = true;
                    }
                    break;
                case 24:
                    if (GET_SKILL(ch, SKILL_SLAM) > 0) {
                        send_to_char(ch, "@GYou have a chance for a COMBO! Try a@R slam @Gnext!@n\r\n");
                        COMBO(ch) = 6;
                        success = true;
                    }
                    break;
            }
        }
        return 0;
    } else if (LASTATK(ch) == COMBO(ch) && COMBHITS(ch) < physical_mastery(ch)) {
        COMBHITS(ch) += 1;
        while (success == false) {
            if (COMBHITS(ch) >= 20) { /* We're kicking ass! */
                switch (rand_number(1, 34)) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        if (GET_SKILL(ch, SKILL_ELBOW) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R elbow@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 2;
                            success = true;
                        }
                        break;
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                        if (GET_SKILL(ch, SKILL_KNEE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rknee@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 3;
                            success = true;
                        }
                        break;
                    case 17:
                    case 18:
                    case 19:
                    case 20:
                    case 21:
                        if (GET_SKILL(ch, SKILL_UPPERCUT) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R uppercut@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 5;
                            success = true;
                        }
                        break;
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                    case 26:
                        if (GET_SKILL(ch, SKILL_ROUNDHOUSE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rroundhouse@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 4;
                            success = true;
                        }
                        break;
                    case 27:
                    case 28:
                    case 29:
                        if (GET_SKILL(ch, SKILL_BASH) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try bash@G!@n\r\n", COMBHITS(ch));
                            COMBO(ch) = 51;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_TAILWHIP) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rtailwhip@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 56;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEADBUTT) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheadbutt@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 52;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_SLAM) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rslam@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 6;
                            success = true;
                        }
                        break;
                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                        if (GET_SKILL(ch, SKILL_BASH) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try bash@G!@n\r\n", COMBHITS(ch));
                            COMBO(ch) = 51;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_TAILWHIP) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rtailwhip@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 56;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEADBUTT) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheadbutt@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 52;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEELDROP) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheeldrop@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 8;
                            success = true;
                        }
                        break;
                } /* Switch End */
            } else if (COMBHITS(ch) >= 15) { /* We're doing good! */
                switch (rand_number(1, 36)) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                        if (GET_SKILL(ch, SKILL_ELBOW) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R elbow@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 2;
                            success = true;
                        }
                        break;
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 20:
                        if (GET_SKILL(ch, SKILL_KNEE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rknee@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 3;
                            success = true;
                        }
                        break;
                    case 21:
                    case 22:
                    case 23:
                        if (GET_SKILL(ch, SKILL_PUNCH) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rpunch@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 0;
                            success = true;
                        }
                        break;
                    case 25:
                    case 26:
                    case 27:
                        if (GET_SKILL(ch, SKILL_KICK) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rkick@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 1;
                            success = true;
                        }
                        break;
                    case 29:
                    case 30:
                        if (GET_SKILL(ch, SKILL_UPPERCUT) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R uppercut@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 5;
                            success = true;
                        }
                        break;
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                        if (GET_SKILL(ch, SKILL_ROUNDHOUSE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rroundhouse@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 4;
                            success = true;
                        }
                        break;
                    case 35:
                        if (GET_SKILL(ch, SKILL_BASH) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try bash@G!@n\r\n", COMBHITS(ch));
                            COMBO(ch) = 51;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_TAILWHIP) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rtailwhip@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 56;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEADBUTT) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheadbutt@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 52;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_SLAM) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rslam@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 6;
                            success = true;
                        }
                        break;
                    case 36:
                        if (GET_SKILL(ch, SKILL_BASH) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try bash@G!@n\r\n", COMBHITS(ch));
                            COMBO(ch) = 51;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_TAILWHIP) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rtailwhip@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 56;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEADBUTT) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheadbutt@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 52;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEELDROP) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheeldrop@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 8;
                            success = true;
                        }
                        break;
                }
            } else if (COMBHITS(ch) >= 10) { /* We're on a roll */
                switch (rand_number(1, 34)) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        if (GET_SKILL(ch, SKILL_ELBOW) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R elbow@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 2;
                            success = true;
                        }
                        break;
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                        if (GET_SKILL(ch, SKILL_KNEE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rknee@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 3;
                            success = true;
                        }
                        break;
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                        if (GET_SKILL(ch, SKILL_PUNCH) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rpunch@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 0;
                            success = true;
                        }
                        break;
                    case 19:
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                    case 26:
                        if (GET_SKILL(ch, SKILL_KICK) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rkick@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 1;
                            success = true;
                        }
                        break;
                    case 27:
                    case 28:
                    case 29:
                        if (GET_SKILL(ch, SKILL_UPPERCUT) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R uppercut@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 5;
                            success = true;
                        }
                        break;
                    case 30:
                    case 31:
                        if (GET_SKILL(ch, SKILL_ROUNDHOUSE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rroundhouse@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 4;
                            success = true;
                        }
                        break;
                    case 32:
                    case 33:
                        if (GET_SKILL(ch, SKILL_BASH) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try bash@G!@n\r\n", COMBHITS(ch));
                            COMBO(ch) = 51;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_TAILWHIP) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rtailwhip@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 56;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEADBUTT) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheadbutt@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 52;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_SLAM) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rslam@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 6;
                            success = true;
                        }
                        break;
                    case 34:
                        if (GET_SKILL(ch, SKILL_BASH) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try bash@G!@n\r\n", COMBHITS(ch));
                            COMBO(ch) = 51;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_TAILWHIP) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rtailwhip@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 56;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEADBUTT) > 0 && rand_number(1, 2) == 2) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheadbutt@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 52;
                            success = true;
                        } else if (GET_SKILL(ch, SKILL_HEELDROP) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rheeldrop@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 8;
                            success = true;
                        }
                        break;
                }
            } else if (COMBHITS(ch) >= 5) { /* We're staring off well */
                switch (rand_number(1, 30)) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        if (GET_SKILL(ch, SKILL_ELBOW) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R elbow@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 2;
                            success = true;
                        }
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        if (GET_SKILL(ch, SKILL_KNEE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rknee@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 3;
                            success = true;
                        }
                        break;
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                        if (GET_SKILL(ch, SKILL_PUNCH) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rpunch@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 0;
                            success = true;
                        }
                        break;
                    case 19:
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                    case 28:
                        if (GET_SKILL(ch, SKILL_KICK) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rkick@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 1;
                            success = true;
                        }
                        break;
                    case 29:
                        if (GET_SKILL(ch, SKILL_UPPERCUT) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R uppercut@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 5;
                            success = true;
                        }
                        break;
                    case 30:
                        if (GET_SKILL(ch, SKILL_ROUNDHOUSE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rroundhouse@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 4;
                            success = true;
                        }
                        break;
                }
            } else { /* We just the combo not long ago */
                switch (rand_number(1, 30)) {
                    case 1:
                    case 2:
                    case 3:
                        if (GET_SKILL(ch, SKILL_ELBOW) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try an@R elbow@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 2;
                            success = true;
                        }
                        break;
                    case 4:
                    case 5:
                    case 6:
                        if (GET_SKILL(ch, SKILL_KNEE) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rknee@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 3;
                            success = true;
                        }
                        break;
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                        if (GET_SKILL(ch, SKILL_PUNCH) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rpunch@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 0;
                            success = true;
                        }
                        break;
                    case 19:
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                    case 28:
                    case 29:
                    case 30:
                        if (GET_SKILL(ch, SKILL_KICK) > 0) {
                            send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Next try a @Rkick@G!@n\r\n",
                                         COMBHITS(ch));
                            COMBO(ch) = 1;
                            success = true;
                        }
                        break;
                }
            } /* End continued hits section */
        }
        return COMBHITS(ch);
    } else if (LASTATK(ch) == COMBO(ch) && COMBHITS(ch) >= physical_mastery(ch)) {
        COMBHITS(ch) += 1;
        send_to_char(ch, "@D(@GC-c-combo Bonus @gx%d@G!@D)@C Combo FINISHED for massive damage@G!@n\r\n", COMBHITS(ch));
    } else if (COMBO(ch) != LASTATK(ch) && COMBO(ch) > -1) {
        send_to_char(ch, "@GCombo failed!@n\r\n");
        COMBO(ch) = -1;
        COMBHITS(ch) = 0;
        return 0;
    } else {
        COMBO(ch) = -1;
        COMBHITS(ch) = 0;
        return 0;
    }
    return 0;
}

void handle_spiral(struct char_data *ch, struct char_data *vict, int skill, int first) {
    int prob, perc, avo, index, pry = 2, dge = 2, blk = 2;
    int64_t dmg;
    double amount = 0.0;

    if (first == false) {
        amount = 0.05;
    } else {
        amount = 0.5;
    }

    if (vict == nullptr && FIGHTING(ch)) {
        vict = FIGHTING(ch);
    } else if (vict == nullptr) {
        act("@WHaving lost your target you slow down until your vortex disappears, and end your attack.@n", true, ch,
            nullptr, nullptr, TO_CHAR);
        act("@C$n@W slows down until $s vortex disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->playerFlags.reset(PLR_SPIRAL);
        return;
    }

    if (GET_CHARGE(ch) <= 0) {
        act("@WHaving no more charged ki you slow down until your vortex disappears, and end your attack.@n", true, ch,
            nullptr, nullptr, TO_CHAR);
        act("@C$n@W slows down until $s vortex disappears.@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->playerFlags.reset(PLR_SPIRAL);
        return;
    }

    if (vict) {
        index = check_def(vict); /* Check parry/block/dodge of vict */

        prob = skill;
        perc = axion_dice(0);

        index -= handle_speed(ch, vict);

        avo = index / 4;

        handle_defense(vict, &pry, &blk, &dge);

        if (avo > 0 && avo < 70) {
            prob -= avo;
        } else if (avo >= 70) {
            prob -= 69;
        }
        tech_handle_posmodifier(vict, pry, blk, dge, prob);

        if (!tech_handle_zanzoken(ch, vict, "Spiral Comet Blast")) {
            pcost(ch, amount, 0);
            pcost(vict, 0, GET_MAX_HIT(vict) / 200);
            return;
        }

        if (prob < perc) {
            if ((vict->getCurST()) > 0) {
                if (blk > rand_number(1, 130)) {
                    act("@C$N@W moves quickly and blocks your Spiral Comet blast!@n", false, ch, nullptr, vict,
                        TO_CHAR);
                    act("@WYou move quickly and block @C$n's@W Spiral Comet blast!@n", false, ch, nullptr, vict,
                        TO_VICT);
                    act("@C$N@W moves quickly and blocks @c$n's@W Spiral Comet blast!@n", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, amount, 0);
                    dmg = damtype(ch, 10, skill, .05);
                    dmg /= 4;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    return;
                } else if (dge > rand_number(1, 130)) {
                    act("@C$N@W manages to dodge your Spiral Comet blast, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@WYou dodge @C$n's@W Spiral Comet blast, letting it slam into the surroundings!@n", false, ch,
                        nullptr, vict, TO_VICT);
                    act("@C$N@W manages to dodge @c$n's@W Spiral Comet blast, letting it slam into the surroundings!@n",
                        false, ch, nullptr, vict, TO_NOTVICT);
                    send_to_room(IN_ROOM(vict), "@wA bright explosion erupts from the impact!\r\n");

                    dodge_ki(ch, vict, 0, 45, skill, SKILL_SPIRAL); /* Effects on the room from dodging a ki attack
                               Num 1: [ 0 for non-homing, 1 for homing ki attacks, 2 for guided ]
                               Num 2: [ Number of attack for damtype ]*/

                    ch->getRoom()->modDamage(5);

                    pcost(ch, amount, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                } else {
                    act("@WYou can't believe it but your Spiral Comet blast misses, flying through the air harmlessly!@n",
                        false, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W fires a Spiral Comet blast at you, but misses!@n ", false, ch, nullptr, vict, TO_VICT);
                    act("@c$n@W fires a Spiral Comet blast at @C$N@W, but somehow misses!@n ", false, ch, nullptr, vict,
                        TO_NOTVICT);
                    pcost(ch, amount, 0);
                    hurt(0, 0, ch, vict, nullptr, 0, 1);
                    return;
                }
            } else {
                act("@WYou can't believe it but your Spiral Comet blast misses, flying through the air harmlessly!@n",
                    false, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W fires a Spiral Comet blast at you, but misses!@n", false, ch, nullptr, vict, TO_VICT);
                act("@c$n@W fires a Spiral Comet blast at @C$N@W, but somehow misses!@n", false, ch, nullptr, vict,
                    TO_NOTVICT);
                pcost(ch, amount, 0);
            }
            hurt(0, 0, ch, vict, nullptr, 0, 1);
            return;
        } else {
            if (first == true) {
                dmg = damtype(ch, 44, skill, .5);
            } else {
                dmg = damtype(ch, 45, skill, .01);
            }
            switch (rand_number(1, 5)) {
                case 1:
                    act("@WYou launch a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S chest and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at YOU! It slams into YOUR chest and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S chest and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 2: /* Critical */
                    act("@WYou launch a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S head and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at YOU! It slams into YOUR head and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S head and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg *= 2;
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 3);
                    break;
                case 3:
                    act("@WYou launch a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S body and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at YOU! It slams into YOUR body and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S body and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    hurt(0, 0, ch, vict, nullptr, dmg, 1);
                    dam_eq_loc(vict, 4);
                    break;
                case 4: /* Weak */
                    act("@WYou launch a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S arm and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at YOU! It slams into YOUR arm and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S arm and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg /= 2;
                    dam_eq_loc(vict, 1);
                    hurt(0, 190, ch, vict, nullptr, dmg, 1);
                    break;
                case 5: /* Weak 2 */
                    act("@WYou launch a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S leg and explodes!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at YOU! It slams into YOUR leg and explodes!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@C$n@W launches a bright @mp@Mu@mr@Mp@ml@Me@W ball of energy down at @c$N@W! It slams into $S leg and explodes!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    dmg /= 2;
                    dam_eq_loc(vict, 2);
                    hurt(1, 190, ch, vict, nullptr, dmg, 1);
                    break;
            }
            pcost(ch, amount, 0);
            return;
        }
    } else {
        return;
    }
}

void handle_death_msg(struct char_data *ch, struct char_data *vict, int type) {
    if (type == 0) {
        if (!SUNKEN(IN_ROOM(vict)) && SECT(IN_ROOM(vict)) != SECT_WATER_SWIM &&
            SECT(IN_ROOM(vict)) != SECT_WATER_NOSWIM && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE) &&
            SECT(IN_ROOM(vict)) != SECT_FLYING) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r coughs up blood before falling to the ground dead.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYou cough up blood before falling to the ground dead.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r coughs up blood before falling down dead.@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 2:
                    act("@R$N@r crumples to the ground dead.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou crumple to the ground dead.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r crumples to the ground dead.@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 3:
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou cry out your last breath before dying.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 4:
                    act("@R$N@r writhes on the ground screaming in pain before finally dying!@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@rYou writhe on the ground screaming in pain before finally dying!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r writhes on the ground screaming in pain before finally dying!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
                case 5:
                    act("@R$N@r hits the ground dead with such force that blood flies into the air briefly!@n", true,
                        ch, nullptr, vict, TO_CHAR);
                    act("@rYou hit the ground dead with such force that blood flies into the air briefly!@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@R$N@r hits the ground dead with such force that blood flies into the air briefly!@n", true,
                        ch, nullptr, vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
            }
        } else if (SECT(IN_ROOM(vict)) == SECT_WATER_SWIM || SECT(IN_ROOM(vict)) == SECT_WATER_NOSWIM) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r coughs up blood and dies before falling down to the water. A large splash accompanies $S body hitting the water!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou cough up blood and die before falling down to the water. A large splash accompanies your body hitting the water!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r coughs up blood and dies before falling down to the water. A large splash accompanies $S body hitting the water!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 2:
                    act("@R$N@r crumples down to the water, with the signs of life leaving $S eyes as $E floats in the water.@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou crumple down to the water and die.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r crumples down to the water, with the signs of life leaving $S eyes as $E floats in the water.@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 3:
                    act("@R$N@r cries out $S last breath before dying and leaving a floating corpse behind.@n", true,
                        ch, nullptr, vict, TO_CHAR);
                    act("@rYou cry out your last breath before dying.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r cries out $S last breath before dying and leaving a floating corpse behind.@n", true,
                        ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 4:
                    act("@R$N@r writhes in the water screaming in pain before finally dying!@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@rYou writhe in the water screaming in pain before finally dying!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r writhes in the water screaming in pain before finally dying!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
                case 5:
                    act("@R$N@r hits the water dead with such force that blood mixed with water flies into the air briefly!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou hit the water dead with such force that blood mixed with water flies into the air briefly!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r hits the water dead with such force that blood mixed with water flies into the air briefly!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
            }
        } else if (ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE)) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r coughs up blood and dies. The blood freezes and floats freely through space...@n", true,
                        ch, nullptr, vict, TO_CHAR);
                    act("@rYou cough up blood and die. The blood freezes and floats freely through space...@n", true,
                        ch, nullptr, vict, TO_VICT);
                    act("@R$N@r coughs up blood and dies. The blood freezes and floats freely through space...@n", true,
                        ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 2:
                    act("@R$N@r dies and leaves $S corpse floating freely in space.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYou die and leave your corpse floating freely in space.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r dies and leaves $S corpse floating freely in space.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
                case 3:
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou cry out your last breath before dying.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 4:
                    act("@R$N@r writhes in space trying to scream in pain before $e finally dies!@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@rYou writhe in space trying to scream in pain before you finally die!@n", true, ch, nullptr,
                        vict, TO_VICT);
                    act("@R$N@r writhes in space trying to scream in pain before $e finally dies!@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
                case 5:
                    act("@R$N@r dies suddenly leaving behind a badly damaged corpse floating in space!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@rYou die suddenly leaving behind a badly damaged corpse floating in space!@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@R$N@r dies suddenly leaving behind a badly damaged corpse floating in space!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
            }
        } else if (SECT(IN_ROOM(vict)) == SECT_FLYING) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r coughs up blood before $s corpse starts to fall to the ground far below.@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@rYou coughs up blood before your corpse starts to fall to the ground far below.@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@R$N@r coughs up blood before $s corpse starts to fall to the ground far below.@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    break;
                case 2:
                    act("@R$N@r dies and $S corpse begins to fall to the ground below.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYou die and your corpse begins to fall to the ground below.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r dies and $S corpse begins to fall to the ground below.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
                case 3:
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou cry out your last breath before dying.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 4:
                    act("@R$N@r writhes in midair screaming in pain before finally dying!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYou writhe in midair screaming in pain before finally dying!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r writhes in midair screaming in pain before finally dying!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
                case 5:
                    act("@R$N@r snaps back and dies with such force that blood flies into the air briefly!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@rYou snap back and die with such force that blood flies into the air briefly!@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@R$N@r hits the ground dead with such force that blood flies into the air briefly!@n", true,
                        ch, nullptr, vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
            }
        } else {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r coughs up blood before $s corpse starts to float limply in the water.@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@rYou coughs up blood before your corpse starts to float limply in the water.@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@R$N@r coughs up blood before $s corpse starts to float limply in the water.@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    break;
                case 2:
                    act("@R$N@r dies and $S corpse begins to float limply in the water.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYou die and your corpse begins to float limply in the water.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r dies and $S corpse begins to float limply in the water.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
                case 3:
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou cry out your last breath before dying.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r cries out $S last breath before dying.@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 4:
                    act("@R$N@r writhes and thrases in the water trying to scream before finally dying!@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@rYou writhe and thrash in the water trying to scream before finally dying!@n", true, ch,
                        nullptr, vict, TO_VICT);
                    act("@R$N@r writhes and thrashes in the water trying to scream before finally dying!@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
                case 5:
                    act("@R$N@r snaps back and dies with such force that blood floods out of $S body into the water!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou snap back and die with such force that blood floods out of your body into the water!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r hits the ground dead with such force that blood floods out of $S body into the water!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    if (GET_DEATH_TYPE(vict) != DTYPE_HEAD) {
                        GET_DEATH_TYPE(vict) = DTYPE_PULP;
                    }
                    break;
            }
        }
    } else {
        if (!SUNKEN(IN_ROOM(vict)) && SECT(IN_ROOM(vict)) != SECT_WATER_SWIM &&
            SECT(IN_ROOM(vict)) != SECT_WATER_NOSWIM && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE) &&
            SECT(IN_ROOM(vict)) != SECT_FLYING) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r explodes and chunks of $M shower to the ground.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou explode leaving only chunks behind.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r explodes and chunks of $M shower to the ground.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 2:
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rThe bottom half of your body is all that remains as you die.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_HALF;
                    break;
                case 3:
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body completely disintegrates in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 4:
                    act("@R$N@r falls down as a smoldering corpse!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body falls down as a smoldering corpse!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r falls down as a smoldering corpse!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 5:
                    act("@rWhat's left of @R$N@r's body slams into the ground as $E dies!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rWhat's left of your body slams into the ground as you die!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rWhat's left of @R$N@r's body slams into the ground as $E dies!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
            }
        } else if (SECT(IN_ROOM(vict)) == SECT_WATER_SWIM || SECT(IN_ROOM(vict)) == SECT_WATER_NOSWIM) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r explodes and chunks of $M shower to the ground.@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYou explode leaving only chunks behind.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r explodes and chunks of $M shower to the ground.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 2:
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rThe bottom half of your body is all that remains as you die.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_HALF;
                    break;
                case 3:
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body completely disintegrates in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 4:
                    act("@R$N@r falls down as a smoldering corpse!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body falls down as a smoldering corpse!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r falls down as a smoldering corpse!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 5:
                    act("@rWhat's left of @R$N@r's body slams into the ground as $E dies!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rWhat's left of your body slams into the ground as you die!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rWhat's left of @R$N@r's body slams into the ground as $E dies!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
            }
        } else if (ROOM_FLAGGED(IN_ROOM(vict), ROOM_SPACE)) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r explodes and chunks of $M shower out into every direction of space.@n", true, ch,
                        nullptr, vict, TO_CHAR);
                    act("@rYou explode leaving only chunks behind.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r explodes and chunks of $M shower out into every direction of space.@n", true, ch,
                        nullptr, vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 2:
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rThe bottom half of your body is all that remains as you die.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_HALF;
                    break;
                case 3:
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body completely disintegrates in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 4:
                    act("@R$N@r floats away as a smoldering corpse!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body floats away as a smoldering corpse!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r floats away as a smoldering corpse!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 5:
                    act("@rWhat's left of @R$N@r's body floats away through space!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rWhat's left of your body floats away through space!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@rWhat's left of @R$N@r's body floats away through space!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
            }
        } else if (SECT(IN_ROOM(vict)) == SECT_FLYING) {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r explodes and chunks of $M shower towards the ground far below.@n", true, ch, nullptr,
                        vict, TO_CHAR);
                    act("@rYou explode leaving only chunks behind.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r explodes and chunks of $M shower toward the ground far below.@n", true, ch, nullptr,
                        vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 2:
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rThe bottom half of your body is all that remains as you die.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_HALF;
                    break;
                case 3:
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body completely disintegrates in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 4:
                    act("@R$N@r falls down toward the ground as a smoldering corpse!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYour body falls down toward the ground as a smoldering corpse!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@R$N@r falls down toward the ground as a smoldering corpse!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
                case 5:
                    act("@rWhat's left of @R$N@r's body falls toward the ground as $E dies!@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rWhat's left of yor body falls toward the ground as you die!@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rWhat's left of @R$N@r's body falls toward the ground as $E dies!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
            }
        } else {
            switch (rand_number(1, 5)) {
                case 1:
                    act("@R$N@r explodes and chunks of $M float freely through the water.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rYou explode leaving only chunks behind.@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r explodes and chunks of $M float freely through the water.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 2:
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_CHAR);
                    act("@rThe bottom half of your body is all that remains as you die.@n", true, ch, nullptr, vict,
                        TO_VICT);
                    act("@rThe bottom half of @R$N@r is all that remains as $E dies.@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_HALF;
                    break;
                case 3:
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body completely disintegrates in the attack!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r is completely disintegrated in the attack!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    GET_DEATH_TYPE(vict) = DTYPE_VAPOR;
                    break;
                case 4:
                    act("@R$N@r falls back as a smoldering corpse!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rYour body falls back as a smoldering corpse!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@R$N@r falls back as a smoldering corpse!@n", true, ch, nullptr, vict, TO_NOTVICT);
                    break;
                case 5:
                    act("@rWhat's left of @R$N@r's body floats limply as $E dies!@n", true, ch, nullptr, vict, TO_CHAR);
                    act("@rWhat's left of yor body floats limply as you die!@n", true, ch, nullptr, vict, TO_VICT);
                    act("@rWhat's left of @R$N@r's body floats limply as $E dies!@n", true, ch, nullptr, vict,
                        TO_NOTVICT);
                    break;
            }
        }
    }
}
