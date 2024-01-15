/* ************************************************************************
*  File: act.misc.c                                    Part of DBAT       *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                   ---                                   *
*  This is an original file created by me for Dragonball Advent Truth     *
*  original credits maintained where relevant for act.other.c as this is  *
*  practically an act.other.c part two - Iovan 3/20/2011                  *
************************************************************************ */
#include "dbat/act.misc.h"
#include "dbat/dg_comm.h"
#include "dbat/act.wizard.h"
#include "dbat/act.movement.h"
#include "dbat/utils.h"
#include "dbat/spells.h"
#include "dbat/comm.h"
#include "dbat/handler.h"
#include "dbat/combat.h"
#include "dbat/constants.h"
#include "dbat/obj_edit.h"
#include "dbat/fight.h"
#include "dbat/class.h"
#include "dbat/act.informative.h"

/* local functions  */
static void generate_multiform(struct char_data *ch, int count);

static void resolve_song(struct char_data *ch);

static int campfire_cook(int recipe);

static int valid_recipe(struct char_data *ch, int recipe, int type);

static int has_pole(struct char_data *ch);

static void catch_fish(struct char_data *ch, int quality);

static int valid_silk(struct obj_data *obj);


ACMD(do_spiritcontrol) {

    if (!GET_SKILL(ch, SKILL_SPIRITCONTROL)) {
        send_to_char(ch, "You do not know how to perform that technique.\r\n");
        return;
    } else {
        if (AFF_FLAGGED(ch, AFF_SPIRITCONTROL)) {
            send_to_char(ch, "You have already concentrated and have full control of your spirit.\r\n");
            return;
        } else {
            int64_t cost = GET_MAX_MANA(ch) * 0.2;
            if ((ch->getCurST()) < cost) {
                send_to_char(ch, "You need at least 20%s of your max ki in stamina to prepare this skill.\r\n", "%");
                return;
            } else {
                ch->decCurST(cost);
                act("@YYou concentrate and quantify every last bit of your spiritual and mental energies. You have full control of them and can bring them forth in an instant.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@y$n@Y seems to concentrate hard for a moment.@n", true, ch, nullptr, nullptr, TO_ROOM);
                int duration = rand_number(2, 4);
                assign_affect(ch, AFF_SPIRITCONTROL, SKILL_SPIRITCONTROL, duration, 0, 0, 0, 0, 0, 0);
            }
        }
    }
}

ACMD(do_tailhide) {

    if (IS_NPC(ch))
        return;

    if (!(IS_SAIYAN(ch)) && !(IS_HALFBREED(ch))) {
        send_to_char(ch, "You have no need to hide your tail!\r\n");
    }
    if ((IS_SAIYAN(ch) || IS_HALFBREED(ch)) && !(PLR_FLAGGED(ch, PLR_TAILHIDE))) {
        ch->playerFlags.set(PLR_TAILHIDE);
        send_to_char(ch, "You have decided to hide your tail!\r\n");
    } else if ((IS_SAIYAN(ch) || IS_HALFBREED(ch)) && PLR_FLAGGED(ch, PLR_TAILHIDE)) {
        ch->playerFlags.reset(PLR_TAILHIDE);
        send_to_char(ch, "You have decided to display your tail for all to see!\r\n");
    }
}

ACMD(do_nogrow) {

    if (IS_NPC(ch))
        return;

    if (!(IS_SAIYAN(ch)) && !(IS_HALFBREED(ch))) {
        send_to_char(ch, "What do you mean?\r\n");
    }
    if ((IS_SAIYAN(ch) || IS_HALFBREED(ch)) && !(PLR_FLAGGED(ch, PLR_NOGROW))) {
        ch->playerFlags.set(PLR_NOGROW);
        send_to_char(ch, "You have decided to halt your tail growth!\r\n");
    } else if ((IS_SAIYAN(ch) || IS_HALFBREED(ch)) && PLR_FLAGGED(ch, PLR_NOGROW)) {
        ch->playerFlags.reset(PLR_NOGROW);
        send_to_char(ch, "You have decided to regrow your tail!\r\n");
    }
}

ACMD(do_restring) {

    char arg[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int pay = 0;

    one_argument(argument, arg);

    if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 178 && GET_ROOM_VNUM(IN_ROOM(ch)) <= 184) {
        pay = 5000;
        if (GET_GOLD(ch) < pay) {
            send_to_char(ch, "You need at least 5,000 zenni to initiate an equipment restring.\r\n");
            return;
        } else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
            send_to_char(ch, "You don't have a that equipment to restring in your inventory.\r\n");
            send_to_char(ch, "Syntax: restring (obj name)\r\n");
            return;
        } else if (OBJ_FLAGGED(obj, ITEM_CUSTOM)) {
            send_to_char(ch,
                         "You can not restring a custom piece. Why? Because you already restrung it you dummy.\r\n");
            return;
        } else {
            STATE(ch->desc) = CON_POBJ;
            char thename[MAX_INPUT_LENGTH], theshort[MAX_INPUT_LENGTH], thelong[MAX_INPUT_LENGTH];

            *thename = '\0';
            *theshort = '\0';
            *thelong = '\0';

            sprintf(thename, "%s", obj->name);
            sprintf(theshort, "%s", obj->short_description);
            sprintf(thelong, "%s", obj->room_description);

            ch->desc->obj_name = strdup(thename);
            ch->desc->obj_was = strdup(theshort);
            ch->desc->obj_short = strdup(theshort);
            ch->desc->obj_long = strdup(thelong);
            ch->desc->obj_point = obj;
            ch->desc->obj_type = 1;
            ch->desc->obj_weapon = 0;
            disp_restring_menu(ch->desc);
            ch->desc->obj_editflag = EDIT_RESTRING;
            ch->desc->obj_editval = EDIT_RESTRING_MAIN;
            return;
        }
    }
}

ACMD(do_multiform) {

    if (!IS_NPC(ch) && !GET_SKILL(ch, SKILL_MULTIFORM)) {
        send_to_char(ch, "You do not know how to perform that technique.\r\n");
        return;
    }

    std::vector<char_data *> multis;
    struct char_data *tch = nullptr, *next_v = nullptr;

    for (tch = ch->getRoom()->people; tch; tch = next_v) {
        next_v = tch->next_in_room;
        if (tch == ch || !IS_NPC(tch)) {
            continue;
        }
        if (GET_ORIGINAL(tch) == ch) {
            multis.push_back(tch);
        }
    }

    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg);

    if (!strcasecmp(arg, "merge")) {
        if (multis.empty()) {
            send_to_char(ch, "You have no multiforms present to merge with!\r\n");
            return;
        }
        for (const auto &m: multis) {
            handle_multi_merge(m);
        }
        return;
    }

    if (!strcasecmp(arg, "split")) {
        int64_t cost = (GET_MAX_MANA(ch) * 0.005) + (GET_MAX_MOVE(ch) * 0.005) + 2;
        int penalty = 0;

        if (FIGHTING(ch)) {
            penalty = rand_number(8, 15);
        }

        int roll = axion_dice(penalty);

        cost *= (GET_SKILL(ch, SKILL_MULTIFORM) * 0.2);

        if ((ch->getCurKI()) < cost) {
            send_to_char(ch, "You do not have enough ki to split!\r\n");
            return;
        }
        if ((ch->getCurST()) < cost) {
            send_to_char(ch, "You do not have enough stamina to split!\r\n");
            return;
        }
        improve_skill(ch, SKILL_MULTIFORM, 1);


        if (GET_SKILL(ch, SKILL_MULTIFORM) < roll) {
            act("@YYou focus your ki into your body while concentrating on the image of your body splitting into two. @yYou lose your concentration and fail to split though...@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@y$n@Y seems to concentrate really hard for a moment, before relaxing.@n", true, ch, nullptr, nullptr,
                TO_ROOM);
            ch->decCurST(cost);
            ch->decCurKI(cost);
            return;
        }
        act("@YYou focus your ki into your body while concentrating on the image of your body splitting into two. Another you splits out of your body!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@YSuddenly @y$n@Y seems to concentrates really and after a brief moment splits into two copies of $mself!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        generate_multiform(ch, 1);
        return;
    } else {
        send_to_char(ch, "Huh? Try help multiform\r\n");
        return;
    }

}

static void generate_multiform(struct char_data *ch, int count) {
    char blamo[MAX_INPUT_LENGTH];
    sprintf(blamo, "p.%s", GET_NAME(ch));

    mob_rnum r_num;
    if ((r_num = real_mobile(25)) == NOBODY) {
        send_to_imm("Multiform Clone doesn't exist!");
        return;
    }

    auto clone_name = fmt::format("{}'s Clone", ch->name);
    auto clone_sdesc = fmt::format("{}'s @CClone@n", ch->name);
    auto clone_ldesc = fmt::format("{}'s @CClone@w is standing here.@n\n", ch->name);

    for (int i = 0; i < count; i++) {
        char_data *clone = nullptr;
        clone = read_mobile(r_num, REAL);

        clone->name = strdup(clone_name.c_str());
        clone->short_description = strdup(clone_sdesc.c_str());
        clone->room_description = strdup(clone_ldesc.c_str());
        if (ch->look_description)
            clone->look_description = strdup(ch->look_description);
        clone->race = ch->race;
        clone->chclass = ch->chclass;
        clone->stats = ch->stats;
        clone->nums = ch->nums;

        clone->appearances = ch->appearances;
        clone->aligns = ch->aligns;
        clone->size = ch->size;
        clone->attributes = ch->attributes;
        clone->trains = ch->trains;

        clone->weight = ch->weight;
        clone->time = ch->time;

        clone->tail_growth = ch->tail_growth;

        // Copying these values, but it shouldn't matter because clones no longer work this way.


        // Bioandroid Genome copy...
        clone->genome[0] = ch->genome[0];
        clone->genome[1] = ch->genome[1];

        // Limb copy...
        for (int l = 0; l < 3; l++) {
            clone->limbs[l] = ch->limbs[l];
            clone->limb_condition[l] = ch->limb_condition[l];
        }

        ch->clones.insert(clone);

        GET_ORIGINAL(clone) = ch;
        char_to_room(clone, IN_ROOM(ch));
        add_follower(clone, ch);
    }
}

void handle_multi_merge(struct char_data *form) {
    struct char_data *ch = GET_ORIGINAL(form);

    if (!ch) {
        extract_char(form);
        return;
    }

    send_to_char(ch, "@YYou merge with one of your forms!@n\r\n");
    act("@y$n@Y merges with one of his multiforms!@n\r\n", true, ch, nullptr, nullptr, TO_ROOM);

    extract_char(form);
}

void handle_songs(uint64_t heartPulse, double deltaTime) {
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
        if (!IS_PLAYING(d))
            continue;
        if (!d->character)
            continue;
        if (GET_SONG(d->character) > 0) {
            resolve_song(d->character);
        }
    }

}

static void resolve_song(struct char_data *ch) {

    struct char_data *vict = nullptr, *next_v = nullptr;
    struct obj_data *obj2 = nullptr, *next_obj;
    int diceroll = axion_dice(0);
    int skill = GET_SKILL(ch, SKILL_MYSTICMUSIC);
    int instrument = 0;

    int stopplaying = false;
    char buf[MAX_INPUT_LENGTH];

    if (GET_SONG(ch) <= 0) {
        return;
    }

    for (obj2 = ch->contents; obj2; obj2 = next_obj) {
        next_obj = obj2->next_content;
        if (GET_OBJ_VNUM(obj2) == 8802 || GET_OBJ_VNUM(obj2) == 8807) {
            instrument = GET_OBJ_VNUM(obj2);
        }
    }

    if (instrument == 0) {
        send_to_char(ch, "You do not have an instrument.\r\n");
        act("@c$n@C stops playing $s song.@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->set(CharNum::MysticMelody, 0);
        return;
    }

    if (skill > diceroll) {
        sprintf(buf, "@c$n@C continues playing @y'@Y%s@y'@C.@n",
                GET_SONG(ch) == SONG_SAFETY ? "Song of Safety" : (GET_SONG(ch) == SONG_SHIELDING ? "Song of Shielding"
                                                                                                 : (GET_SONG(ch) ==
                                                                                                    SONG_SHADOW_STITCH
                                                                                                    ? "Shadow Stitch Minuet"
                                                                                                    : "Teleportation Melody")));
        act("@CYou continue playing your song.@n", true, ch, nullptr, nullptr, TO_CHAR);
        act(buf, true, ch, nullptr, nullptr, TO_ROOM);
    } else {
        act("@CYou mess up a portion of the song, but continue playing.@n", true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C messes up a portion of $s song, but continues to play.@n", true, ch, nullptr, nullptr, TO_ROOM);
        return;
    }

    for (vict = ch->getRoom()->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        switch ((int)GET_SONG(ch)) {
            case SONG_SAFETY:
                if ((ch->master == vict->master || ch == vict->master || vict == ch->master) || vict == ch) {
                    if ((AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) || vict == ch) {
                        if (ch == vict->master || ch->master == vict || ch->master == vict->master || vict == ch) {
                            if (skill > diceroll) {
                                int64_t restore = (10 * skill) + ((GET_MAX_MANA(ch) * 0.0004) * skill);
                                if (vict != ch) {
                                    act("@CYour skillfully playing of the Song of Safety has an effect on @c$N@C.@n",
                                        true, ch, nullptr, vict, TO_CHAR);
                                } else {
                                    act("@CYour skillfully playing of the Song of Safety has an effect on your own body@C.@n",
                                        true, ch, nullptr, vict, TO_CHAR);
                                }
                                vict->incCurHealth(restore);
                                vict->incCurST(restore * .5);

                                if (GET_LIMBCOND(vict, 0) < 100) {
                                    GET_LIMBCOND(vict, 0) += 1 + (skill * 0.1);
                                    if (GET_LIMBCOND(vict, 0) > 100) {
                                        send_to_char(vict, "Your right arm is no longer broken!@n\r\n");
                                        GET_LIMBCOND(vict, 0) = 100;
                                    }
                                }
                                if (GET_LIMBCOND(vict, 1) < 100) {
                                    GET_LIMBCOND(vict, 1) += 1 + (skill * 0.1);
                                    if (GET_LIMBCOND(vict, 1) > 100) {
                                        send_to_char(vict, "Your left arm is no longer broken!@n\r\n");
                                        GET_LIMBCOND(vict, 1) = 100;
                                    }
                                }
                                if (GET_LIMBCOND(vict, 2) < 100) {
                                    GET_LIMBCOND(vict, 2) += 1 + (skill * 0.1);
                                    if (GET_LIMBCOND(vict, 2) > 100) {
                                        send_to_char(vict, "Your right leg is no longer broken!@n\r\n");
                                        GET_LIMBCOND(vict, 2) = 100;
                                    }
                                }
                                if (GET_LIMBCOND(vict, 0) < 100) {
                                    GET_LIMBCOND(vict, 3) += 1 + (skill * 0.1);
                                    if (GET_LIMBCOND(vict, 3) > 100) {
                                        send_to_char(vict, "Your left leg is no longer broken!@n\r\n");
                                        GET_LIMBCOND(vict, 3) = 100;
                                    }
                                }
                                if (vict != ch) {
                                    act("@c$n's@C soothing Song of Safety has recovered some of your powerlevel, stamina, and limb condition.",
                                        true, ch, nullptr, vict, TO_VICT);
                                }
                                act("@c$n@C continues playing $s ocarina!@n", true, ch, nullptr, vict, TO_NOTVICT);
                                improve_skill(ch, SKILL_MYSTICMUSIC, 2);
                                ch->decCurKI((ch->getMaxKI() * .0003) + skill);
                            }
                        }
                    }
                }
                if ((ch->getCurKI()) <= 0) {
                    send_to_char(ch, "You no longer have the ki necessary to play your song.\r\n");
                    act("@c$n@C stops playing $s song.@n", true, ch, nullptr, nullptr, TO_ROOM);
                    ch->set(CharNum::MysticMelody, 0);
                    return;
                }
                break;
            case SONG_SHADOW_STITCH: {
                auto applyShadow = [skill](struct char_data *user, struct char_data *target) {
                    user->decCurKI(user->getPercentOfMaxKI(.001) + skill);
                    if (!IS_NPC(target)) {
                        WAIT_STATE(target, PULSE_2SEC);
                    } else {
                        assign_affect(target, AFF_SHADOWSTITCH, 0, -1, 0, 0, 0, 0, 0, -2);
                    }
                };
                if (ch->master && vict->master) {
                    if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                        if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                            continue;
                        }

                        if (skill > diceroll + 10) {
                            act("@CYour forboding music has caused @c$N's@C shadows to stitch into $S body, slowing $S actions!@n",
                                true, ch, nullptr, vict, TO_CHAR);
                            act("@c$n's@C forboding music has caused YOUR shadows to stitch into YOUR body, slow YOUR actions down!@n",
                                true, ch, nullptr, vict, TO_VICT);
                            act("@c$n's@C forboding music has caused @c$N's@C shadows to stitch into $S body, slowing $S actions!@n",
                                true, ch, nullptr, vict, TO_NOTVICT);
                            applyShadow(ch, vict);
                        }
                    } else if (skill > diceroll + 10) {
                        act("@CYour forboding music has caused @c$N's@C shadows to stitch into $S body, slowing $S actions!@n",
                            true, ch, nullptr, vict, TO_CHAR);
                        act("@c$n's@C forboding music has caused YOUR shadows to stitch into YOUR body, slow YOUR actions down!@n",
                            true, ch, nullptr, vict, TO_VICT);
                        act("@c$n's@C forboding music has caused @c$N's@C shadows to stitch into $S body, slowing $S actions!@n",
                            true, ch, nullptr, vict, TO_NOTVICT);
                        applyShadow(ch, vict);
                    }
                } else if (skill > diceroll + 10) {
                    act("@CYour forboding music has caused @c$N's@C shadows to stitch into $S body, slowing $S actions!@n",
                        true, ch, nullptr, vict, TO_CHAR);
                    act("@c$n's@C forboding music has caused YOUR shadows to stitch into YOUR body, slow YOUR actions down!@n",
                        true, ch, nullptr, vict, TO_VICT);
                    act("@c$n's@C forboding music has caused @c$N's@C shadows to stitch into $S body, slowing $S actions!@n",
                        true, ch, nullptr, vict, TO_NOTVICT);
                    applyShadow(ch, vict);
                }
                if ((ch->getCurKI()) <= 0) {
                    send_to_char(ch, "You no longer have the ki necessary to play your song.\r\n");
                    act("@c$n@C stops playing $s song.@n", true, ch, nullptr, nullptr, TO_ROOM);
                    ch->set(CharNum::MysticMelody, 0);
                    return;
                }
            }

                break;
            case SONG_TELEPORT_EARTH:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Earth in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Earth in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(300));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_VEGETA:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Vegeta in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Vegeta in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(2234));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_FRIGID:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Frigid in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Frigid in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(4047));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_KONACK:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Konack in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Konack in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(8003));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_NAMEK:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Namek in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Namek in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(10182));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_ARLIA:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Arlia in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Arlia in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(16087));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_AETHER:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Aether in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Aether in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(12025));
                        }
                    }
                }
                break;
            case SONG_TELEPORT_KANASSA:
                if (vict == ch)
                    continue;
                if (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP)) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master) {
                        if (skill > diceroll) {
                            act("@CYour Teleportation Melody has transported @c$N@C to Kanassa in a flash!@n", true, ch,
                                nullptr, vict, TO_CHAR);
                            act("@c$n's@C Teleportation Melody has transported you to Kanassa in a flash!@n", true, ch,
                                nullptr, vict, TO_VICT);
                            act("@c$n's@C Teleportation Melody has transported @c$N@C away in a flash!@n", true, ch,
                                nullptr, vict, TO_NOTVICT);
                            char_from_room(vict);
                            char_to_room(vict, real_room(14910));
                        }
                    }
                }
                break;
            case SONG_SHIELDING:
                if (vict == ch || (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(vict, AFF_GROUP))) {
                    if (ch == vict->master || ch->master == vict || ch->master == vict->master || vict == ch) {
                        if (skill > diceroll) {
                            if (vict != ch) {
                                act("@CYour triumphant and soaring music has powered a barrier around @c$N@C!@n", true,
                                    ch, nullptr, vict, TO_CHAR);
                                act("@c$n's@C triumphant and soaring music has powered a barrier around you!@n", true,
                                    ch, nullptr, vict, TO_VICT);
                            } else {
                                act("@CYour triumphant and soaring music has powered a barrier around yourself@C!@n",
                                    true, ch, nullptr, vict, TO_CHAR);
                            }
                            act("@c$n's@C triumphant and soaring music has powered a barrier around @c$N@C!@n", true,
                                ch, nullptr, vict, TO_NOTVICT);
                            GET_BARRIER(vict) += ((GET_MAX_MANA(ch) * 0.005) * (skill * 0.25)) + skill;
                            if (GET_BARRIER(vict) >= GET_MAX_MANA(vict) * 0.75) {
                                GET_BARRIER(vict) = GET_MAX_MANA(vict) * 0.75;
                            }
                            vict->affected_by.set(AFF_SANCTUARY);
                            ch->incCurKI(ch->getPercentOfMaxKI(.02) + skill);
                        }
                    }
                }
                if ((ch->getCurKI()) <= 0) {
                    send_to_char(ch, "You no longer have the ki necessary to play your song.\r\n");
                    act("@c$n@C stops playing $s song.@n", true, ch, nullptr, nullptr, TO_ROOM);
                    ch->set(CharNum::MysticMelody, 0);
                    return;
                }
                break;
        }
    }


    if (GET_SONG(ch) >= 4 && skill > diceroll) {
        switch ((int)GET_SONG(ch)) {
            case SONG_TELEPORT_EARTH:
                char_from_room(ch);
                char_to_room(ch, real_room(300));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Earth and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_VEGETA:
                char_from_room(ch);
                char_to_room(ch, real_room(2234));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Vegeta and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_FRIGID:
                char_from_room(ch);
                char_to_room(ch, real_room(4047));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Frigid and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_NAMEK:
                char_from_room(ch);
                char_to_room(ch, real_room(10182));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Namek and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_KANASSA:
                char_from_room(ch);
                char_to_room(ch, real_room(14910));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Kanassa and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_AETHER:
                char_from_room(ch);
                char_to_room(ch, real_room(12025));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Aether and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_ARLIA:
                char_from_room(ch);
                char_to_room(ch, real_room(16087));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Arlia and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
            case SONG_TELEPORT_KONACK:
                char_from_room(ch);
                char_to_room(ch, real_room(8003));
                ch->set(CharNum::MysticMelody, 0);
                act("@CFinally as the last of your comrades has been teleported you teleport yourself to Konack and stop your song.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                break;
        }
    }
}

ACMD(do_song) {

    if (!GET_SKILL(ch, SKILL_MYSTICMUSIC)) {
        send_to_char(ch, "You do not know how to play mystical music.\r\n");
        return;
    }

    struct obj_data *obj2 = nullptr, *next_obj;
    int instrument = 0;

    for (obj2 = ch->contents; obj2; obj2 = next_obj) {
        next_obj = obj2->next_content;
        if (GET_OBJ_VNUM(obj2) == 8802 || GET_OBJ_VNUM(obj2) == 8807) {
            instrument = GET_OBJ_VNUM(obj2);
        }
    }

    if (instrument == 0) {
        send_to_char(ch, "You do not have an instrument.\r\n");
        return;
    }

    if (GET_SONG(ch)) {
        act("@cYou stop playing your ocarina.@n", true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n stops playing their ocarina.@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->set(CharNum::MysticMelody, 0);
        return;
    } else {

        char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
        int skill = GET_SKILL(ch, SKILL_MYSTICMUSIC);

        two_arguments(argument, arg, arg2);
        if (!*arg) {
            send_to_char(ch, "@YSongs Known\n@c-------------------@n\r\n");
            send_to_char(ch, "@W%s%s%sSong of Safety\r\n", skill > 99 ? "Song of Shielding\n" : "",
                         skill > 80 ? "Melody of Teleportation\n" : "", skill > 50 ? "Shadow Stitch Minuet\n" : "");
            send_to_char(ch, "@wSyntax: song (shielding | safety | teleport | shadow )\r\n");
            return;
        } else {
            int64_t cost = GET_MAX_MANA(ch) * 0.01;
            int modifier = 1;
            if (!strcasecmp(arg, "shielding")) {
                modifier = 20;
                cost *= modifier;
            } else if (!strcasecmp(arg, "teleport")) {
                modifier = 50;
                cost *= modifier;
            } else if (!strcasecmp(arg, "shadow")) {
                modifier = 8;
                cost *= modifier;
            } else if (!strcasecmp(arg, "safety")) {
                modifier = 3;
                cost *= modifier;
            }

            if (instrument == 8802) {
                cost -= cost * 0.5;
            }

            if (modifier == 0) {
                send_to_char(ch, "@wSyntax: song (shielding | safety | teleport | shadow )\r\n");
                return;
            } else if (modifier == 3 && (ch->getCurKI()) < cost) {
                send_to_char(ch, "@wYou do not have enough ki to power the instrument for that song!@n\r\n");
                return;
            } else if (modifier == 3) {
                act("@CYou begin to play the Song of Safety! Your fingers lightly glide over the ocarina and as you blow into it sweet music similar to a lullaby issues forth from the intrument.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@c$n@C begins to play a song on $s ocarina. The music seems to be some sort of lullaby.@n", true,
                    ch, nullptr, nullptr, TO_ROOM);
                ch->set(CharNum::MysticMelody, SONG_SAFETY);
                ch->decCurKI(cost);
                return;
            } else if (modifier == 8 && (ch->getCurKI()) < cost) {
                send_to_char(ch, "@wYou do not have enough ki to power the instrument for that song!@n\r\n");
                return;
            } else if (modifier == 8 && skill <= 49) {
                send_to_char(ch, "You do not posess the skill to play such a song!\r\n");
                return;
            } else if (modifier == 8) {
                act("@CYou begin to play the Shadow Stitch Minuet! Your fingers lightly glide over the ocarina and as you blow into it forboding low toned music issues forth.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@c$n@C begins to play a song on $s ocarina. Depressing low toned music issues forth from the ocarina.@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                ch->set(CharNum::MysticMelody, SONG_SHADOW_STITCH);
                ch->decCurKI(cost);
                return;
            } else if (modifier == 50 && (ch->getCurKI()) < cost) {
                send_to_char(ch, "@wYou do not have enough ki to power the instrument for that song!@n\r\n");
                return;
            } else if (modifier == 50 && skill <= 79) {
                send_to_char(ch, "You do not posess the skill to play such a song!\r\n");
                return;
            } else if (modifier == 50) {
                if (!*arg2) {
                    send_to_char(ch,
                                 "Where would you like to teleport to?\nSyntax: song teleport (earth | vegeta | kanassa | arlia | aether | namek | konack| frigid)\r\n");
                    return;
                } else if (AFF_FLAGGED(ch, AFF_SPIRIT)) {
                    send_to_char(ch, "Not while you're dead!\r\n");
                    return;
                } else if (!strcasecmp(arg2, "earth")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_EARTH);
                } else if (!strcasecmp(arg2, "frigid")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_FRIGID);
                } else if (!strcasecmp(arg2, "vegeta")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_VEGETA);
                } else if (!strcasecmp(arg2, "namek")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_NAMEK);
                } else if (!strcasecmp(arg2, "arlia")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_ARLIA);
                } else if (!strcasecmp(arg2, "kanassa")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_KANASSA);
                } else if (!strcasecmp(arg2, "konack")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_KONACK);
                } else if (!strcasecmp(arg2, "aether")) {
                    ch->set(CharNum::MysticMelody, SONG_TELEPORT_AETHER);
                } else {
                    send_to_char(ch,
                                 "Syntax: song teleport (earth | vegeta | namek | aether | konack | kanassa | arlia | frigid)\r\n");
                    return;
                }
                act("@CYou begin to play the Melody of Teleportation! Your fingers lightly glide over the ocarina and as you blow into it a repeating light hearted melody issues forth.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@c$n@C begins to play a song on $s ocarina. A light hearted melody can be heard sounding from the ocarina as it is played.@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                ch->decCurKI(cost);
                return;
            } else if (modifier == 20 && (ch->getCurKI()) < cost) {
                send_to_char(ch, "@wYou do not have enough ki to power the instrument for that song!@n\r\n");
                return;
            } else if (modifier == 20 && skill <= 98) {
                send_to_char(ch, "You do not posess the skill to play such a song!\r\n");
                return;
            } else if (modifier == 20) {
                act("@CYou begin to play the Song of Shielding! Your fingers lightly glide over the ocarina and as you blow into it a triumphant series of notes issues forth.@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@c$n@C begins to play a song on $s ocarina. A triumphant song full of soaring sounds from the ocarina as it is played.@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                ch->set(CharNum::MysticMelody, SONG_SHIELDING);
                ch->decCurKI(cost);
                return;
            }

        }

    }
}

ACMD(do_preference) {

    if (IS_NPC(ch))
        return;

    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Syntax: preference (throw | weapon | hand | ki)\r\n");
        return;
    }

    if (GET_PREFERENCE(ch) > 0) {
        send_to_char(ch, "You've already chosen a specialization. No going back.\r\n");
        return;
    }

    if (!strcasecmp(arg, "throw")) {
        send_to_char(ch, "You will now favor throwing weapons as fighting specialization. You're sure to nail it.\r\n");
        GET_PREFERENCE(ch) = PREFERENCE_THROWING;
        auto &s = ch->skill[SKILL_THROW];
        if(s.level <= 90) s.level += 10;
        else if(s.level < 100) s.level = 100;
        return;
    } else if (!strcasecmp(arg, "hand")) {
        send_to_char(ch, "You will now favor your body as your fighting specialization. Your body is ready.\r\n");
        GET_PREFERENCE(ch) = PREFERENCE_H2H;
        return;
    } else if (!strcasecmp(arg, "ki")) {
        send_to_char(ch,
                     "You will now favor your ki energy as your fighting specialization. I expect more than a few smoldering craters.\r\n");
        GET_PREFERENCE(ch) = PREFERENCE_KI;
        return;
    } else if (!strcasecmp(arg, "weapon")) {
        send_to_char(ch, "You will now favor your weapons as your fighting specialization. Let the blood fly!\r\n");
        GET_PREFERENCE(ch) = PREFERENCE_WEAPON;
        return;
    } else {
        send_to_char(ch, "Syntax: preference (throw | weapon | hand | ki)\r\n");
        return;
    }
}

ACMD(do_moondust) {
    int64_t cost = GET_MAX_MOVE(ch) * 0.02, heal = 0;

    /* Can they do the technique? */

    if (!IS_ARLIAN(ch) || GET_SEX(ch) != SEX_FEMALE) {
        send_to_char(ch, "You are not an arlian female.\r\n");
        return;
    }

    if (!AFF_FLAGGED(ch, AFF_GROUP)) {
        send_to_char(ch, "You need to be in a group to use this skill!\r\n");
        return;
    }

    cost += (ch->getMaxLF()) * 0.02;
    heal = cost * 3;

    if (GET_HIT(ch) >= (ch->getEffMaxPL()) * 0.8) {
        cost = cost * 0.5;
    }

    if ((ch->getCurST()) < cost) {
        send_to_char(ch, "You do not have enough stamina to perform this technique.\r\n");
        return;
    }

    int chance = axion_dice(0);

    if (chance > GET_WIS(ch) + rand_number(1, 10)) {
        act("@GYou spread your wings and begin to concentrate. Your wings begin to glow a soft sea green color. As you prepare to release a cloud of your charged wing dust you lose focus and the power you had begun to charge into your wings dissipates.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@g$n@G spreads $s wings and seems to concentrate for a moment. Suddenly $s wings begin to glow a soft sea green color. This soft glow grows brighter for a second before fading completely.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->decCurST(cost);
        WAIT_STATE(ch, PULSE_1SEC);
        return;
    }
    ch->incCurHealth(heal);
    ch->decCurST(cost);
    WAIT_STATE(ch, PULSE_1SEC);

    act("@GYou spread your wings and begin to concentrate. Your wings begin to glow a soft sea green color. As your wings grow brighter you focus your charged bio energy in a shockwave the unleashes a cloud of glowing green dust. You breath in the dust and feel it rejuvinate your body's cells!@n",
        true, ch, nullptr, nullptr, TO_CHAR);
    act("@g$n@G spreads $s wings and seems to concentrate for a moment. Suddenly $s wings begin to glow a soft sea green color. This soft glow grows brighter and as $e flexes $s wings to their full extent a shockwave of energy explodes outward. Carried on this shockwave is a cloud of glowing dust! You notice some of the dust being breathed in by $s!@n",
        true, ch, nullptr, nullptr, TO_ROOM);
    send_to_char(ch, "@RHeal@Y: @C%s@n\r\n", add_commas(heal).c_str());

    struct char_data *vict = nullptr, *next_v = nullptr;

    for (vict = ch->getRoom()->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (vict == ch) {
            continue;
        }
        if (AFF_FLAGGED(vict, AFF_GROUP)) {
            if (ch->master == vict->master || vict->master == ch || ch->master == vict) {
                vict->incCurHealth(heal);
                act("@CYou breathe in the dust and are healed by it somewhat!@n", true, vict, nullptr, nullptr,
                    TO_CHAR);
                act("@c$n@C breathes in the dust and is healed somewhat!@n", true, vict, nullptr, nullptr, TO_ROOM);
                send_to_char(vict, "@RHeal@Y: @C%s@n\r\n", add_commas(heal).c_str());
            }
        }
    }
}

ACMD(do_shell) {

    if (!IS_ARLIAN(ch)) {
        send_to_char(ch, "You are not capable of doing that!\r\n");
        return;
    }

    if (GET_SEX(ch) == SEX_FEMALE) {
        send_to_char(ch, "Sorry, you can't do that.\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_SHELL)) {
        act("@mYou quickly absorb the armor carapace covering your body back inside.@n", true, ch, nullptr, nullptr,
            TO_CHAR);
        act("@M$n's@m armored carapce retreats back to its original size.@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->affected_by.reset(AFF_SHELL);
        return;
    }

    if ((ch->getCurST()) < GET_MAX_MOVE(ch) * 0.2) {
        send_to_char(ch, "You do not have enough stamina to grow your armored carapace.@n\r\n");
        return;
    } else if (axion_dice(0) > GET_CON(ch) + rand_number(1, 10)) {
        act("@mYou crouch down and begin to focus on your body's carapace cells encouraging them to multiply! However your control is lacking and you ultimately fail to grow your armor very much.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@M$n@m crouches down and seems to strain for a moment before giving up and resuming $s normal stance.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        return;
    } else {
        act("@mYou crouch down and begin to focus on your body's carapace cells, encouraging them to multiply! Very quickly millions of new carapace cells have been born and your armored carapace extends over all parts of your body!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@M$n@m crouches down and after a few moments of straining $s body's carapace armor starts to grow thicker and extends to cover all parts of $s body!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->decCurSTPercent(.2);
        ch->affected_by.set(AFF_SHELL);
        return;
    }
}

ACMD(do_liquefy) {

    if (!IS_MAJIN(ch)) {
        send_to_char(ch,
                     "You are not capable of liquefying yourself right now. Try finding a giant blender maybe?\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_LIQUEFIED)) {
        act("@MSuddenly large chunks of goo start to hover up slowly. These very same chunks quickly begin to fly into each other, piling on as the ball of goo grows. Suddenly @m$n@M emerges as the ball of goo takes $s shape!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        act("@MYou begin to pull the liquid chunks of your body together. Those chunks hover upward and merge into each other until a large ball of goo is formed. Slowly your body emerges as the pieces of your body take on their old form!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        ch->affected_by.reset(AFF_LIQUEFIED);
        WAIT_STATE(ch, PULSE_3SEC);
        WAIT_STATE(ch, PULSE_3SEC);
        WAIT_STATE(ch, PULSE_3SEC);
        WAIT_STATE(ch, PULSE_3SEC);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    }

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Syntax: liquefy hide\nSyntax: liquefy explode (target)\r\n");
        return;
    }

    if ((ch->getCurKI()) < (GET_MAX_MANA(ch) * 0.002) + 150) {
        send_to_char(ch, "You do not have enough ki to manage this level of body control!\r\n");
        return;
    }

    if (!strcasecmp(arg, "hide")) {
        if (GRAPPLED(ch)) {
            GRAPPLING(GRAPPLED(ch)) = nullptr;
            GRAPPLED(ch) = nullptr;
        }
        if (GRAPPLING(ch)) {
            GRAPPLED(GRAPPLING(ch)) = nullptr;
            GRAPPLING(ch) = nullptr;
        }
        if (DRAGGING(ch)) {
            DRAGGED(DRAGGING(ch)) = nullptr;
            DRAGGING(ch) = nullptr;
        }
        if (axion_dice(0) > GET_LEVEL(ch)) {
            act("@MYour body starts to become loose and sag, but you lose focus and it reverts to its original shape!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@m$n@M's body starts to become loose and sag, but $e seems to return normal a moment later.@n", true,
                ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI((GET_MAX_MANA(ch) * .002) + 150);

            return;
        } else {
            act("@MYour body starts to become loose and sag. It continues to droop down until it begins to run down like a river of goo flowing from where your body was.@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@m$n@M's body starts to become loose and sag. Much of $s body begins to pour down and scatter around as pools of goo.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI((GET_MAX_MANA(ch) * .002) + 150);
            ch->affected_by.set(AFF_LIQUEFIED);
            return;
        }
    } else if (!strcasecmp(arg, "explode")) {
        struct char_data *vict;
        if (GRAPPLED(ch)) {
            GRAPPLING(GRAPPLED(ch)) = nullptr;
            GRAPPLED(ch) = nullptr;
        }
        if (GRAPPLING(ch)) {
            GRAPPLED(GRAPPLING(ch)) = nullptr;
            GRAPPLING(ch) = nullptr;
        }
        if (DRAGGING(ch)) {
            DRAGGED(DRAGGING(ch)) = nullptr;
            DRAGGING(ch) = nullptr;
        }
        if (!*arg2) {
            send_to_char(ch, "Syntax: liquefy hide\nSyntax: liquefy explode (target)\r\n");
            return;
        } else if ((ch->getCurKI()) < (GET_MAX_MANA(ch) * 0.10) + 150) {
            send_to_char(ch, "You do not have enough ki for that action!@n\r\n");
            return;
        } else if (!(vict = get_char_vis(ch, arg2, nullptr, FIND_CHAR_ROOM))) {
            send_to_char(ch, "That target isn't here.\r\n");
            return;
        } else if (!can_kill(ch, vict, nullptr, 1)) {
            send_to_char(ch, "You can't kill them!\r\n");
            return;
        } else if (axion_dice(0) > GET_LEVEL(ch)) {
            act("@MYour body starts to become loose and sag, but you lose focus and it reverts to its original shape!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@m$n@M's body starts to become loose and sag, but $e seems to return normal a moment later.@n", true,
                ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI((GET_MAX_MANA(ch) * .002) + 150);
            WAIT_STATE(ch, PULSE_3SEC);
            return;
        } else if (GET_SPEEDI(ch) < GET_SPEEDI(vict)) {
            act("@MYour body rapidly turns to liquid and flies for @R$N's@M open mouth! However $E easily dodges and avoids your attempt!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@m$n@M's body rapidly turns to liquid and flies for @RYOUR@M open mouth! However you are faster and managed to dodge the attempt.@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@m$n@M's body rapidly turns into liquid and flies for @R$N's@M open mouth! However $E easily dodges and avoids @m$n's@M attempt!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            ch->decCurKI((GET_MAX_MANA(ch) * .002) + 150);
            if (!FIGHTING(ch)) {
                set_fighting(ch, vict);
            }
            if (!FIGHTING(vict)) {
                set_fighting(vict, ch);
            }
            WAIT_STATE(ch, PULSE_3SEC);
            WAIT_STATE(ch, PULSE_3SEC);
            return;
        } else if (GET_HIT(ch) < GET_HIT(vict) * 2) {
            act("@MYour body rapidly turns to liquid and flies for @R$N's@M open mouth! However as you force yourself in through $S mouth $E successfully resists and forces your back out!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@m$n@M's body rapidly turns to liquid and flies for @RYOUR@M open mouth! However you think quickly and force $m out before $e has a chance to get fully into your body!@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@m$n@M's body rapidly turns into liquid and flies for @R$N's@M open mouth! However as $e forces $mself in through @R$N's@M mouth $E manages to resist and force @m$n@M back out!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            ch->decCurKI((GET_MAX_MANA(ch) * .002) + 150);
            int64_t dmg = GET_MAX_HIT(ch) * 0.08;
            hurt(0, 0, ch, vict, nullptr, dmg, 0);
            WAIT_STATE(ch, PULSE_3SEC);
            return;
        } else {
            act("@MYour body rapidly turns to liquid and flies for @R$N's@M open mouth! As you fill $S body you expand outward until $s body explodes into a gory mess!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@m$n@M's body rapidly turns to liquid and flies for @RYOUR@M open mouth! As $e fills your body it begins to expand until it is unable to take the strain any longer and explodes!@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@m$n@M's body rapidly turns into liquid and flies for @R$N's@M open mouth! As $e forces $mself in through @R$N's@M mouth $S body begins to expand until it can't take the strain any longer and explodes!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            ch->decCurKI((GET_MAX_MANA(ch) * .002) + 150);
            if (AFF_FLAGGED(ch, AFF_GROUP)) {
                group_gain(ch, vict);
            } else {
                solo_gain(ch, vict);
            }
            die(vict, ch);
            ch->affected_by.set(AFF_LIQUEFIED);
            WAIT_STATE(ch, PULSE_3SEC);
            handle_cooldown(ch, 9);
            return;
        }
    } else {
        send_to_char(ch, "Syntax: liquefy hide\nSyntax: liquefy explode (target)\r\n");
        return;
    }
}

ACMD(do_lifeforce) {

    char arg[MAX_INPUT_LENGTH];
    int setting = 0;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Syntax: life (0 - 99)\n0 is off.\r\n");
        return;
    }

    setting = atoi(arg);

    if (setting > 99) {
        send_to_char(ch, "Syntax: life (1 - 99)\n%s isn't an acceptable percent.\r\n", add_commas(setting).c_str());
        return;
    } else if (setting <= 0) {
        send_to_char(ch,
                     "Your will just isn't in the fight, huh?\nYou will not use up life force to maintain your PL period.\r\n");
        GET_LIFEPERC(ch) = 0;
        return;
    } else {
        send_to_char(ch, "Your life force will automatically kick in at %d%s of your optimal PL.\r\n", setting, "%");
        GET_LIFEPERC(ch) = setting;
        return;
    }
}

ACMD(do_defend) {
    struct char_data *vict;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg && GET_DEFENDING(ch) == nullptr) {
        send_to_char(ch, "Defend who?\r\n");
        return;
    } else if (!*arg && GET_DEFENDING(ch)) {
        act("@YYou stop defending @y$N@Y.@n", true, ch, nullptr, GET_DEFENDING(ch), TO_CHAR);
        act("@y$n@Y stops defending you.@n", true, ch, nullptr, GET_DEFENDING(ch), TO_VICT);
        act("@y$n@Y stops defending @y$N@Y.@n", true, ch, nullptr, GET_DEFENDING(ch), TO_NOTVICT);
        GET_DEFENDER(GET_DEFENDING(ch)) = nullptr;
        GET_DEFENDING(ch) = nullptr;
        return;
    }

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "You can't seem to find that person.\r\n");
        return;
    } else if (vict == ch) {
        send_to_char(ch, "Well hopefully you are smart enough to defend yourself.\r\n");
        return;
    } else {
        act("@YYou start defending @y$N@Y.@n", true, ch, nullptr, vict, TO_CHAR);
        act("@y$n@Y starts defending you.@n", true, ch, nullptr, vict, TO_VICT);
        act("@y$n@Y starts defending @y$N@Y.@n", true, ch, nullptr, vict, TO_NOTVICT);
        GET_DEFENDER(vict) = ch;
        GET_DEFENDING(ch) = vict;
        return;
    }

}

ACMD(do_fish) {

    if (IS_NPC(ch))
        return;

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Syntax: fish ( cast | hook | reel | apply | stop)\r\n");
        return;
    }

    if (!strcasecmp(arg, "cast")) {
        if (PLR_FLAGGED(ch, PLR_FISHING)) {
            send_to_char(ch, "You are already fishing! Syntax: fish stop\r\n");
            return;
        } else if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_FISHING)) {
            send_to_char(ch, "This is not an area you can fish at.\r\n");
            return;
        } else if (AFF_FLAGGED(ch, AFF_FLYING)) {
            send_to_char(ch, "You can't fish while flying.\r\n");
            return;
        } else if (FIGHTING(ch)) {
            send_to_char(ch, "You can't fish while fighting!\r\n");
            return;
        } else if (!GET_EQ(ch, WEAR_WIELD2)) {
            send_to_char(ch, "You are not holding a fishing pole.\r\n");
            return;
        } else {
            struct obj_data *pole = GET_EQ(ch, WEAR_WIELD2);
            if (GET_OBJ_TYPE(pole) != ITEM_FISHPOLE) {
                send_to_char(ch, "You do not have a fishing pole in your hand!\r\n");
                return;
            } else if (GET_OBJ_VAL(pole, 0) == 0) {
                send_to_char(ch, "There is no bait on your line!\r\n");
                return;
            }
            reveal_hiding(ch, 0);
            act("@CYou pull your arm back and then spring it forward, casting the baited line. A moment later there is a splash as the hook enters the water.@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C pulls $s arm back and then springs it foward, casting the line of $s fishing pole into the water.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            GET_FISHD(ch) = rand_number(30, 80);
            ch->playerFlags.set(PLR_FISHING);
            send_to_char(ch, "@D[@wDistance@D: @Y%d@D]@n\r\n", GET_FISHD(ch));
            return;
        }
    } else if (!strcasecmp(arg, "hook")) {
        if (!PLR_FLAGGED(ch, PLR_FISHING)) {
            send_to_char(ch, "You are not even fishing!\r\n");
            return;
        } else if (GET_FISHSTATE(ch) == FISH_NOFISH) {
            send_to_char(ch, "You do not have a fish biting on your line.\r\n");
            return;
        } else if (GET_FISHSTATE(ch) == FISH_REELING) {
            send_to_char(ch, "You are already trying to reel the fish in!\r\n");
            return;
        } else if (GET_FISHSTATE(ch) == FISH_HOOKED) {
            send_to_char(ch, "You already have the fish hooked! Reel it in!\r\n");
            return;
        } else if (axion_dice(-18) > GET_POLE_BONUS(ch)) {
            reveal_hiding(ch, 0);
            act("@CYou pull hard but the fish spits the hook out a second before you pull! You return to waiting for a bite...@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C pulls hard on $s fishing line, but a moment later $e frowns and returns to fishing.@n", true,
                ch, nullptr, nullptr, TO_ROOM);
            GET_FISHSTATE(ch) = FISH_NOFISH;
            return;
        } else {
            reveal_hiding(ch, 0);
            act("@CYou pull hard on your line and feel that you have managed to hook the fish! Better @Greel@C it in!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C pulls hard on $s fishing line and starts to struggle with the fish on the other end!@n", true,
                ch, nullptr, nullptr, TO_ROOM);
            GET_FISHSTATE(ch) = FISH_HOOKED;
            return;
        }
    } else if (!strcasecmp(arg, "reel")) {
        if (!PLR_FLAGGED(ch, PLR_FISHING)) {
            send_to_char(ch, "You are not even fishing!\r\n");
            return;
        } else if (GET_FISHSTATE(ch) == FISH_NOFISH) {
            send_to_char(ch, "You do not have a fish biting on your line.\r\n");
            return;
        } else if (GET_FISHSTATE(ch) == FISH_REELING) {
            send_to_char(ch, "You are already trying to reel the fish in!\r\n");
            return;
        } else if (GET_FISHSTATE(ch) != FISH_HOOKED) {
            send_to_char(ch, "You don't have a fish hooked!\r\n");
            return;
        } else {
            reveal_hiding(ch, 0);
            act("@CYou begin reeling the fish in slowly.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C begins to reel the line on $s pole in.@n", true, ch, nullptr, nullptr, TO_ROOM);
            GET_FISHSTATE(ch) = FISH_REELING;
            return;
        }
    } else if (!strcasecmp(arg, "apply")) {
        if (!GET_EQ(ch, WEAR_WIELD2)) {
            send_to_char(ch, "You are not holding a fishing pole.\r\n");
            return;
        } else {
            struct obj_data *pole = GET_EQ(ch, WEAR_WIELD2);
            if (GET_OBJ_TYPE(pole) != ITEM_FISHPOLE) {
                send_to_char(ch, "You do not have a fishing pole in your hand!\r\n");
                return;
            } else if (GET_OBJ_VAL(pole, 0) != 0) {
                send_to_char(ch, "Your fishing pole already has bait on its hook.\r\n");
                return;
            } else {
                struct obj_data *bait;

                if (!*arg2) {
                    send_to_char(ch, "Syntax: fish apply (bait)\r\n");
                    return;
                }
                if (!(bait = get_obj_in_list_vis(ch, arg2, nullptr, ch->contents))) {
                    send_to_char(ch, "You don't have that bait.\r\n");
                    return;
                } else if (GET_OBJ_TYPE(bait) != ITEM_FISHBAIT) {
                    send_to_char(ch, "That isn't fishing bait!\r\n");
                    return;
                } else {
                    reveal_hiding(ch, 0);
                    act("@CYou carefully apply the $p@C to your hook.@n", true, ch, bait, nullptr, TO_CHAR);
                    act("@c$n@C carefully applies $p@C to $s fishing pole's hook.@n", true, ch, bait, nullptr, TO_ROOM);
                    GET_OBJ_VAL(pole, 0) = GET_OBJ_COST(bait);
                    extract_obj(bait);
                    return;
                } /* End we applied bait */
            } /* End Applying bait */
        } /* end has pole */
    } else if (!strcasecmp(arg, "stop")) {
        if (!PLR_FLAGGED(ch, PLR_FISHING)) {
            send_to_char(ch, "You are not even fishing!\r\n");
            return;
        } else {
            reveal_hiding(ch, 0);
            act("@CYou reel in your line and stop fishing.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reels in $s fishing line and stops fishing.@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->playerFlags.reset(PLR_FISHING);
            GET_FISHSTATE(ch) = FISH_NOFISH;
            GET_FISHD(ch) = 0;
            return;
        }
    } else {
        send_to_char(ch, "Syntax: fish ( cast | hook | reel | apply | stop )\r\n");
        return;
    }
} /* End fish */

static int has_pole(struct char_data *ch) {

    if (GET_EQ(ch, WEAR_WIELD2)) {
        struct obj_data *pole = GET_EQ(ch, WEAR_WIELD2);
        if (GET_OBJ_TYPE(pole) == ITEM_FISHPOLE) {
            return (true);
        }
    }

    return (false);
}

void fish_update(uint64_t heartPulse, double deltaTime) {

    struct char_data *i, *next_char, *ch = nullptr;
    int quality = 0;

    for (i = character_list; i; i = next_char) {
        next_char = i->next;
        if (ROOM_FLAGGED(IN_ROOM(i), ROOM_FISHING)) {
            if (PLR_FLAGGED(i, PLR_FISHING) && has_pole(i) == true) {
                ch = i;
                if (GET_FISHD(ch) <= 0 && GET_FISHSTATE(ch) == FISH_REELING) { /* We've caught it */
                    if (GET_POLE_BONUS(ch) >= rand_number(60, 100)) {
                        quality = rand_number(0, 3) + rand_number(0, 3) + rand_number(0, 3);
                    } else if (GET_POLE_BONUS(ch) >= rand_number(45, 60)) {
                        quality = rand_number(0, 3) + rand_number(0, 3);
                    } else {
                        quality = rand_number(0, 3);
                    }
                    catch_fish(ch, quality);
                } else if (rand_number(1, 5) >= 3) { /* Reeling section */
                    if (GET_FISHSTATE(ch) == FISH_REELING && rand_number(1, 100) <= 80) {
                        if (GET_POLE_BONUS(ch) >= 80) {
                            GET_FISHD(ch) -= rand_number(6, 10);
                        } else if (GET_POLE_BONUS(ch) >= 40) {
                            GET_FISHD(ch) -= rand_number(5, 8);
                        } else {
                            GET_FISHD(ch) -= rand_number(1, 4);
                        }
                        act("@CYou reel the line on your pole some.@n", true, ch, nullptr, nullptr, TO_CHAR);
                        act("@c$n@C reels the line on $s pole slowly.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        send_to_char(ch, "@D[@wDistance@D: @Y%d@D]@n\r\n", GET_FISHD(ch) > 0 ? GET_FISHD(ch) : 0);
                    } else if (GET_FISHSTATE(ch) == FISH_REELING && rand_number(1, 58) <= 55) {
                        act("@CYou struggle as the fish fights against your attempts to reel it in!@n", true, ch,
                            nullptr, nullptr, TO_CHAR);
                        act("@c$n@C struggles with the fish on the end of $s pole!@n", true, ch, nullptr, nullptr,
                            TO_ROOM);
                    } else if (GET_FISHSTATE(ch) == FISH_REELING) { /* Lose the fish */
                        act("@CYou feel the line go slack and realize you've lost the fish! You reel your line back in...@n",
                            true, ch, nullptr, nullptr, TO_CHAR);
                        act("@c$n@C frowns and then begins to reel in $s line.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        GET_FISHD(ch) = 0;
                        GET_FISHSTATE(ch) = FISH_NOFISH;
                        ch->playerFlags.reset(PLR_FISHING);
                        if (has_pole(ch) == true) {
                            struct obj_data *pole = GET_EQ(ch, WEAR_WIELD2);
                            GET_OBJ_VAL(pole, 0) = 0;
                        }
                    } else if (GET_FISHSTATE(ch) == FISH_HOOKED && rand_number(1, 20) >= 12) {
                        act("@CYou feel the line go slack and realize you've lost the fish! You reel your line back in...@n",
                            true, ch, nullptr, nullptr, TO_CHAR);
                        act("@c$n@C frowns and then begins to reel in $s line.@n", true, ch, nullptr, nullptr, TO_ROOM);
                        GET_FISHD(ch) = 0;
                        GET_FISHSTATE(ch) = FISH_NOFISH;
                        ch->playerFlags.reset(PLR_FISHING);
                    } else if (GET_FISHSTATE(ch) == FISH_BITE && rand_number(1, 20) >= 12) {
                        act("@CYou feel as if the fish has stopped biting...@n", true, ch, nullptr, nullptr, TO_CHAR);
                        GET_FISHSTATE(ch) = FISH_NOFISH;
                    } else if (GET_FISHSTATE(ch) != FISH_HOOKED && GET_FISHSTATE(ch) != FISH_BITE &&
                               ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_FISHFRESH) && rand_number(1, 10) >= 8) ||
                                (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_FISHFRESH) && rand_number(1, 20) >= 18))) {
                        act("@CYou feel a fish biting on your line! Better @Ghook@C it!@n", true, ch, nullptr, nullptr,
                            TO_CHAR);
                        GET_FISHSTATE(ch) = FISH_BITE;
                    }
                } /* End reel section */
            } else if (PLR_FLAGGED(i, PLR_FISHING) && has_pole(i) == false) { /* End of, Is Fishing */
                i->playerFlags.reset(PLR_FISHING);
                GET_FISHD(i) = 0;
                GET_FISHSTATE(i) = FISH_NOFISH;
            }
        } else { /* Is not in a fishing room */
            if (PLR_FLAGGED(i, PLR_FISHING)) {
                i->playerFlags.reset(PLR_FISHING);
                GET_FISHD(i) = 0;
                GET_FISHSTATE(i) = FISH_NOFISH;
            }
        }
    } /* End of for */

}

static void catch_fish(struct char_data *ch, int quality) {
    struct obj_data *fish = nullptr;
    int num = 1000;

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_FISHFRESH)) {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_EARTH)) {
            switch (rand_number(1, 10)) {
                case 1:
                case 2:
                case 3:
                case 4:
                    num = 1000;
                    break;
                case 5:
                case 6:
                case 7:
                    num = 1001;
                    break;
                case 8:
                case 9:
                    num = 1002;
                    break;
                case 10:
                    num = 1003;
                    break;
            }
        } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_AETHER)) {
            switch (rand_number(1, 10)) {
                case 1:
                case 2:
                case 3:
                case 4:
                    num = 1012;
                    break;
                case 5:
                case 6:
                case 7:
                    num = 1013;
                    break;
                case 8:
                case 9:
                    num = 1014;
                    break;
                case 10:
                    num = 1015;
                    break;
            }
        }
    } else {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_EARTH)) {
            switch (rand_number(1, 10)) {
                case 1:
                case 2:
                case 3:
                case 4:
                    num = 1004;
                    break;
                case 5:
                case 6:
                case 7:
                    num = 1005;
                    break;
                case 8:
                case 9:
                    num = 1006;
                    break;
                case 10:
                    num = 1007;
                    break;
            }
        } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NAMEK)) {
            switch (rand_number(1, 10)) {
                case 1:
                case 2:
                case 3:
                case 4:
                    num = 1008;
                    break;
                case 5:
                case 6:
                case 7:
                    num = 1009;
                    break;
                case 8:
                case 9:
                    num = 1010;
                    break;
                case 10:
                    num = 1011;
                    break;
            }
        }
    }

    fish = read_object(num, VIRTUAL);

    if (!fish) {
        send_to_imm("Error: Fish success with no fish! Report to Iovan!\r\n");
        return;
    }

    act("@CYou manage to pull a $p@C from the water and onto the ground in front of you!@n", true, ch, fish, nullptr,
        TO_CHAR);
    act("@c$n@C manages to pull a $p@C from the water and onto the ground in front of $m!@n", true, ch, fish, nullptr,
        TO_ROOM);

    int weight = 1;

    if (quality <= 0 && rand_number(1, 20) >= 17) {
        quality += rand_number(2, 7);
    }

    struct obj_data *pole = GET_EQ(ch, WEAR_WIELD2);

    if (GET_OBJ_VAL(pole, 0) * 2 >= axion_dice(0)) {
        quality += 2;
    } else if (GET_OBJ_VAL(pole, 0) >= axion_dice(0)) {
        quality += 1;
    }

    switch (quality) {
        case 0:
        case 1:
            weight = rand_number(0, 2);
            break;
        case 2:
        case 3:
            weight = rand_number(3, 4);
            GET_OBJ_COST(fish) += GET_OBJ_COST(fish) * 0.20;
            GET_OBJ_VAL(fish, 0) += 1;
            break;
        case 4:
        case 5:
        case 6:
            weight = rand_number(5, 9);
            GET_OBJ_COST(fish) += GET_OBJ_COST(fish) * 0.5;
            GET_OBJ_VAL(fish, 0) += 3;
            break;
        default:
            weight = rand_number(10, 15);
            GET_OBJ_COST(fish) += GET_OBJ_COST(fish) * 2;
            GET_OBJ_VAL(fish, 0) += 5;
            break;
    }

    GET_OBJ_WEIGHT(fish) += weight;

    GET_OBJ_VAL(pole, 0) = 0;
    obj_to_room(fish, IN_ROOM(ch));
    do_get(ch, "fish", 0, 0);
    send_to_char(ch, "@D[@cFish Weight@D: @G%" I64T "@D]@n\r\n", GET_OBJ_WEIGHT(fish));
    ch->playerFlags.reset(PLR_FISHING);
    GET_FISHD(ch) = 0;
    GET_FISHSTATE(ch) = FISH_NOFISH;
}

ACMD(do_extract) {

    if (!GET_SKILL(ch, SKILL_EXTRACT)) {
        send_to_char(ch, "You do not know how to extract!\r\n");
        return;
    }

    char arg[MAX_INPUT_LENGTH], argu[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
    struct obj_data *obj = nullptr, *obj2 = nullptr;
    int skill = GET_SKILL(ch, SKILL_EXTRACT), chance = axion_dice(0);

    half_chop(argument, arg, argu);
    two_arguments(argu, arg2, arg3);

    if (!*arg) {
        send_to_char(ch, "Syntax: extract (object)\r\n");
        send_to_char(ch, "Syntax: extract combine (bottle1) (bottle2)\r\n");
        return;
    }

    if (!strcasecmp(arg, "combine")) {
        if (!(obj = get_obj_in_list_vis(ch, arg2, nullptr, ch->contents))) {
            send_to_char(ch, "You do not have the first bottle that you were wanting to combine.\r\n");
            return;
        }
        if (!(obj2 = get_obj_in_list_vis(ch, arg3, nullptr, ch->contents))) {
            send_to_char(ch, "You do not have the second bottle that you were wanting to combine.\r\n");
            return;
        }
        if (obj && obj2) {
            if (GET_OBJ_VNUM(obj) != 3424) {
                send_to_char(ch, "That is not an ink bottle!\r\n");
                return;
            } else if (GET_OBJ_VNUM(obj2) != 3424) {
                send_to_char(ch, "That is not an ink bottle!\r\n");
                return;
            } else if (GET_OBJ_VAL(obj, 6) <= 0) {
                send_to_char(ch, "There isn't any ink in the first bottle!\r\n");
                return;
            } else if (GET_OBJ_VAL(obj2, 6) <= 0) {
                send_to_char(ch, "There isn't any ink in the second bottle!\r\n");
                return;
            } else {
                if (GET_OBJ_VAL(obj, 6) >= GET_OBJ_VAL(obj2, 6)) {
                    GET_OBJ_VAL(obj, 6) += GET_OBJ_VAL(obj2, 6);
                    if (GET_OBJ_VAL(obj, 6) > 24) {
                        GET_OBJ_VAL(obj, 6) = 24;
                    }
                    send_to_char(ch,
                                 "You combine the ink of the two bottles into one bottle, and discard the leftovers.\r\n");
                    extract_obj(obj2);
                } else if (GET_OBJ_VAL(obj2, 6) > GET_OBJ_VAL(obj, 6)) {
                    GET_OBJ_VAL(obj2, 6) += GET_OBJ_VAL(obj, 6);
                    if (GET_OBJ_VAL(obj2, 6) > 24) {
                        GET_OBJ_VAL(obj2, 6) = 24;
                    }
                    send_to_char(ch,
                                 "You combine the ink of the two bottles into one bottle, and discard the leftovers.\r\n");
                    extract_obj(obj);
                }
                return;
            }
        }
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        send_to_char(ch, "You do not have that item.\r\n");
        return;
    } else {
        if (GET_OBJ_VNUM(obj) == 3425) {
            if (GET_OBJ_VAL(obj, VAL_MAXMATURE) != 0 &&
                GET_OBJ_VAL(obj, VAL_MATURITY) < GET_OBJ_VAL(obj, VAL_MAXMATURE)) {
                send_to_char(ch, "It's not mature enough to extract from!\r\n");
                return;
            }
            struct obj_data *bottle = nullptr, *next_obj, *obj2;
            int found = false;

            for (obj2 = ch->contents; obj2; obj2 = next_obj) {
                next_obj = obj2->next_content;
                if (GET_OBJ_VNUM(obj2) == 3423) {
                    bottle = obj2;
                    found = true;
                }
            }

            int64_t cost = ((GET_MAX_MANA(ch) * 0.35) + 500);

            if (found == false) {
                send_to_char(ch, "You do not have an empty bottle to put the extracted ink in.\r\n");
                return;
            }

            int64_t extra = 0;

            if (GET_OBJ_VAL(bottle, 6) + 4 >= 24)
                extra = GET_MAX_MANA(ch) * 0.5;

            cost += extra;

            if ((ch->getCurKI()) < cost) {
                send_to_char(ch, "You do not have enough ki! @D[@rNeeded@D: @R%s@D]@n\r\n", add_commas(cost).c_str());
                return;
            } else if (skill < chance) {
                ch->decCurKI(cost);
                act("@WWith your ki flowing carefully into your hands you take a hold of the @G$p@W and begin to strip it of its leaves. Once it has been stripped you go to squeeze the ink carefully from the leaves into the bottle, but unfortunately the ink explodes into a mess instead!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W takes a hold of the @G$p@W and begins to strip it of its leaves. Once it has been stripped $e bundles up the leaves in $s hands and begins to squeeze. A nasty explosion of a mess is all that follows!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                improve_skill(ch, SKILL_EXTRACT, 0);
                extract_obj(obj);
                WAIT_STATE(ch, PULSE_3SEC);
                return;
            } else {
                ch->decCurKI(cost);
                act("@WWith your ki flowing carefully into your hands you take a hold of the @G$p@W and begin to strip it of its leaves. Once it has been stripped you go to squeeze the ink carefully from the leaves into the bottle, and manage to get every last drop of ink into it.@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W takes a hold of the @G$p@W and begins to strip it of its leaves. Once it has been stripped $e bundles up the leaves in $s hands and begins to squeeze ink carefully from the leaves into a bottle.@n",
                    true, ch, obj, nullptr, TO_ROOM);
                extract_obj(obj);
                GET_OBJ_VAL(bottle, 6) += rand_number(4, 6);
                if (GET_OBJ_VAL(bottle, 6) >= 24) {
                    struct obj_data *filled = read_object(3424, VIRTUAL);
                    extract_obj(bottle);
                    GET_OBJ_VAL(filled, 6) = 24;
                    obj_to_char(filled, ch);
                    ch->decCurKI(0);
                    act("@GAs the last of the ink fills the bottle you infuse a final burst of ki into the bottle.@n",
                        true, ch, filled, nullptr, TO_CHAR);
                    act("@GAs the last of the ink fills the bottle @g$n@G infuses a final burst of ki into the bottle.@n ",
                        true, ch, filled, nullptr, TO_ROOM);
                } else {
                    send_to_char(ch,
                                 "You will need to fill the bottle before giving it a final infusion of ki to complete the process.\r\n");
                }
                improve_skill(ch, SKILL_EXTRACT, 0);
                WAIT_STATE(ch, PULSE_3SEC);
            }
        } else {
            send_to_char(ch, "That is not something you can extract from.\r\n");
            return;
        }
    }
}

ACMD(do_runic) {

    if (!GET_SKILL(ch, SKILL_RUNIC)) {
        send_to_char(ch, "You do not know how to write down runes.\r\n");
        return;
    }

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int skill = GET_SKILL(ch, SKILL_RUNIC), bonus = 0;

    two_arguments(argument, arg, arg2);

    if (FIGHTING(ch)) {
        send_to_char(ch, "You are too busy fighting to write runes!\r\n");
        return;
    }

    if (!*arg || !*arg2) {
        send_to_char(ch, "Syntax: runic (target) (skill)\r\n");
        send_to_char(ch, "@D----@GRunic Skills@D----@n\r\n");
        send_to_char(ch, "@Rkenaz\n%s\n%s\n%s\n%s\n%s\n%s@n\n", skill >= 40 ? "@Galgiz" : "",
                     skill >= 40 ? "@moagaz" : "", skill >= 50 ? "@CLaguz" : "", skill >= 60 ? "@Ywunjo" : "",
                     skill >= 80 ? "@rpurisaz" : "", skill >= 100 ? "@mgebo" : "");
        return;
    }

    struct obj_data *obj, *next_obj, *bottle = nullptr;
    int found = false, amount = 0, brush = false;

    for (obj = ch->contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (GET_OBJ_VNUM(obj) == 3424) {
            if (GET_OBJ_VAL(obj, 6) > amount) {
                bottle = obj;
                found = true;
                amount = GET_OBJ_VAL(bottle, 6);
            }
        }
    }

    for (obj = ch->contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (GET_OBJ_VNUM(obj) == 3427) {
            brush = true;
        }
    }

    if (found == false) {
        send_to_char(ch, "You do not have a bottle with enough ink in it.\r\n");
        return;
    } else if (brush == false) {
        send_to_char(ch, "You do not have a brush!\r\n");
        return;
    }

    int64_t cost = GET_MAX_MANA(ch) * 0.05;
    int inkcost = 0;

    if (IS_HOSHIJIN(ch))
        bonus = 10;
    else
        inkcost += 2;

    struct char_data *vict;

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "You can't seem to find that person.\r\n");
        return;
    } else if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki to write runes.\r\n");
        return;
    } else if (skill + bonus < axion_dice(0) && rand_number(1, 5) == 5) {
        act("@BYou dip your brush into the ink, but as you infuse your ki you balance the flow wrong and end up destroying the ink bottle!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@b$n@B dips $s runic brush into a bottle filled with shimmering ink. @b$n@B appears to concentrate for a moment before a look of panic dons $s face. Just at that moment the bottle of ink explodes! Strange...@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        extract_obj(bottle);
        improve_skill(ch, SKILL_RUNIC, 1);
        ch->decCurKI(cost);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (skill + bonus < axion_dice(0)) {
        ch->decCurKI(cost);
        act("@BYou dip your brush into the ink, but as you infuse your ki you balance the flow wrong and end up evaporating some ink!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@b$n@B dips $s runic brush into a bottle filled with shimmering ink. @b$n@B appears to concentrate for a moment before some ink evaporates. Strange...@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        improve_skill(ch, SKILL_RUNIC, 1);
        GET_OBJ_VAL(bottle, 6) -= rand_number(1, 3);
        if (GET_OBJ_VAL(bottle, 6) < 0)
            GET_OBJ_VAL(bottle, 6) = 0;
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (!strcasecmp(arg2, "kenaz") || !strcasecmp(arg2, "Kenaz")) {
        inkcost += 1;
        if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CKenaz@D'@B rune on your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CKenaz@D'@B rune on $s skin.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.16;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou can now see in the dark! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_INFRAVISION, SKILL_RUNIC, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        } else {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CKenaz@D'@B rune on @b$N's@B skin!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CKenaz@D'@B rune on @RYOUR@B skin.@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CKenaz@D'@B rune on @b$N's@B skin.@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.16;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou can now see in the dark! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_INFRAVISION, SKILL_RUNIC, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
        improve_skill(ch, SKILL_RUNIC, 1);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (!strcasecmp(arg2, "algiz") || !strcasecmp(arg2, "Algiz")) {
        inkcost += 2;
        if (amount < inkcost) {
            send_to_char(ch, "You do not have a bottle with enough ink. @D[@bInkcost@D: @R%d@D]@n\r\n", inkcost);
            return;
        } else if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CAlgiz@D'@B rune on your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CAlgiz@D'@B rune on $s skin.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.05;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou now have Ethereal Armor! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_EARMOR, SKILL_PUNCH, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        } else {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CAlgiz@D'@B rune on @b$N's@B skin!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CAlgiz@D'@B rune on @RYOUR@B skin.@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CAlgiz@D'@B rune on @b$N's@B skin.@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.05;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou now have Ethereal Armor! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_EARMOR, SKILL_PUNCH, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
        improve_skill(ch, SKILL_RUNIC, 1);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (!strcasecmp(arg2, "oagaz") || !strcasecmp(arg2, "Oagaz")) {
        inkcost += 3;
        if (amount < inkcost) {
            send_to_char(ch, "You do not have a bottle with enough ink. @D[@bInkcost@D: @R%d@D]@n\r\n", inkcost);
            return;
        } else if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@COagaz@D'@B rune on your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@COagaz@D'@B rune on $s skin.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.04;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou now are protected by Ethereal Chains! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_ECHAINS, SKILL_KNEE, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
    } else if (!strcasecmp(arg2, "laguz") || !strcasecmp(arg2, "Laguz")) {
        inkcost += 4;
        if (amount < inkcost) {
            send_to_char(ch, "You do not have a bottle with enough ink. @D[@bInkcost@D: @R%d@D]@n\r\n", inkcost);
            return;
        } else if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CLaguz@D'@B rune on your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CLaguz@D'@B rune on $s skin.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.04;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou now have water breathing! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_WATERBREATH, SKILL_SBC, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        } else {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@COagaz@D'@B rune on @b$N's@B skin!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@COagaz@D'@B rune on @RYOUR@B skin.@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@COagaz@D'@B rune on @b$N's@B skin.@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.04;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou now are protected by Ethereal Chains! @D(@WLasts@D: @w%d@D)@n\r\n", duration);
            assign_affect(vict, AFF_ECHAINS, SKILL_KNEE, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
        improve_skill(ch, SKILL_RUNIC, 1);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (!strcasecmp(arg2, "wunjo") || !strcasecmp(arg2, "Wunjo")) {
        inkcost += 4;
        if (amount < inkcost) {
            send_to_char(ch, "You do not have a bottle with enough ink. @D[@bInkcost@D: @R%d@D]@n\r\n", inkcost);
            return;
        } else if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CWunjo@D'@B rune on your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CWunjo@D'@B rune on $s skin.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.08;
            if (duration < 1)
                duration = 1;
            send_to_char(vict,
                         "@GYou are now blessed with a deeper understanding of things you experience! @D(@WLasts@D: @w%d@D)@n\r\n",
                         duration);
            assign_affect(vict, AFF_WUNJO, SKILL_SLAM, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        } else {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CWunjo@D'@B rune on @b$N's@B skin!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CWunjo@D'@B rune on @RYOUR@B skin.@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CWunjo@D'@B rune on @b$N's@B skin.@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.08;
            if (duration < 1)
                duration = 1;
            send_to_char(vict,
                         "@GYou are now blessed with a deeper understanding of things you experience! @D(@WLasts@D: @w%d@D)@n\r\n",
                         duration);
            assign_affect(vict, AFF_WUNJO, SKILL_SLAM, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
        improve_skill(ch, SKILL_RUNIC, 1);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (!strcasecmp(arg2, "purisaz") || !strcasecmp(arg2, "Purisaz")) {
        inkcost += 4;
        if (amount < inkcost) {
            send_to_char(ch, "You do not have a bottle with enough ink. @D[@bInkcost@D: @R%d@D]@n\r\n", inkcost);
            return;
        } else if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CPurisaz@D'@B rune on your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CPurisaz@D'@B rune on $s skin.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.06;
            if (duration < 1)
                duration = 1;
            send_to_char(vict, "@GYou feel as if your inner energy is more potent! @D(@WLasts@D: @w%d@D)@n\r\n",
                         duration);
            assign_affect(vict, AFF_POTENT, SKILL_HEELDROP, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        } else {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CPurisaz@D'@B rune on @b$N's@B skin!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CPurisaz@D'@B rune on @RYOUR@B skin.@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CPUrisaz@D'@B rune on @b$N's@B skin.@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            int duration = skill * 0.06;
            send_to_char(vict, "@GYou feel as if your inner energy is more potent! @D(@WLasts@D: @w%d@D)@n\r\n",
                         duration);
            if (duration < 1)
                duration = 1;
            assign_affect(vict, AFF_POTENT, SKILL_HEELDROP, duration, 0, 0, 0, 0, 0, 0);
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
        improve_skill(ch, SKILL_RUNIC, 1);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else if (!strcasecmp(arg2, "gebo") || !strcasecmp(arg2, "Gebo")) {
        inkcost += 10;
        if (amount < inkcost) {
            send_to_char(ch, "You do not have a bottle with enough ink. @D[@bInkcost@D: @R%d@D]@n\r\n", inkcost);
            return;
        } else if (vict == ch) {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CGebo@D'@B rune on your skin! The rune flashes out of existence immediately!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CGebo@D'@B rune on $s skin. The rune flashes out of existence immediately!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            vict->modPractices(125);
            send_to_char(vict,
                         "@GYou feel like you've just gained a lot of knowledge. Now if only you could apply it. @D[@m+125 PS@D]@n\r\n");
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        } else {
            ch->decCurKI(cost);
            act("@BYou dip your brush into the ink and infuse your ki skillfully into it. You pull the brush out and paint the @D'@CGebo@D'@B rune on @b$N's@B skin! The rune flashes out of existence immediately!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CGebo@D'@B rune on @RYOUR@B skin. The rune flashes out of existence immediately!@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@b$n@B dips $s brush into a bottle of ink and at the same time the ink starts to glow. Skillfully $e then writes the @D'@CGebo@D'@B rune on @b$N's@B skin. The rune flashes out of existence immediately!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            send_to_char(ch, "@D[@B%d@b ink used.@D]@n\r\n", inkcost);
            vict->modPractices(125);
            send_to_char(vict,
                         "@GYou feel like you've just gained a lot of knowledge. Now if only you could apply it. @D[@m+125 PS@D]@n\r\n");
            GET_OBJ_VAL(bottle, 6) -= inkcost;
            if (GET_OBJ_VAL(bottle, 6) <= 0) {
                extract_obj(bottle);
                struct obj_data *empty = read_object(3423, VIRTUAL);
                obj_to_char(empty, ch);
            }
        }
        improve_skill(ch, SKILL_RUNIC, 1);
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    } else {
        send_to_char(ch, "Syntax: runic (target) (skill)\r\n");
        send_to_char(ch, "@D----@GRunic Skills@D----@n\r\n");
        send_to_char(ch, "@Rkenaz\n%s\n%s\n%s\n%s\n%s@n\n", skill >= 40 ? "@Galgiz" : "", skill >= 40 ? "@moagaz" : "",
                     skill >= 60 ? "@Ywunjo" : "", skill >= 80 ? "@rpurisaz" : "", skill >= 100 ? "@mgebo" : "");
        return;
    }

}

ACMD(do_scry) {

    if (strcasecmp(CAP(GET_NAME(ch)), "Galeos")) {
        send_to_char(ch, "You do not know how to perform that technique.\r\n");
        return;
    }

    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Syntax: scry (target)\r\n");
        return;
    }

    struct char_data *vict;

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "Who are you using Oracle Scry on?\r\n");
        return;
    }

    if (vict == ch) {
        send_to_char(ch, "You can't do that to yourself!\r\n");
        return;
    }

    if (IS_NPC(vict)) {
        send_to_char(ch, "No using this on mobs!\r\n");
        return;
    }

    int cost = 2000;

    if (GET_PRACTICES(ch) < cost) {
        send_to_char(ch, "You do not have enough PS to Oracle Scry!\r\n");
        return;
    } else {
        reveal_hiding(ch, 0);
        act("@GYou focus your mind and begin to allow the flood of images and energy to roar through your mind. You then allow those thoughts to make their way into the mind of @c$N@G. You can hardly comprehend the vastness of the information flooding in, yet still glimpse bits and pieces of your own destiny.@n",
            true, ch, nullptr, vict, TO_CHAR);
        act("@GYou see @C$n@G begin to focus, and then without warning, your mind is flooded painfully with images, energy and information. The data streams in a mad torrent through your psyche, and just when you think snapping is possible, the voice of @C$n@G comes to you and eases and guides you. You see images of potential futures, information not yet known, knowledge yet undiscovered. Though you could not fully  grasp what is to come, you feel more prepared at facing the unknown.@n",
            true, ch, nullptr, vict, TO_VICT);
        act("@C$n@W appears to be performing some sort of ritual or something with @c$N@W.@n", true, ch, nullptr, vict,
            TO_NOTVICT);
        int64_t boost = GET_INT(ch) * 0.5;

        vict->gainBasePL((vict->getBasePL() * .01) * boost);
        vict->gainBaseKI((vict->getBaseKI() * .01) * boost);
        vict->gainBaseST((vict->getBaseST() * .01) * boost);

        send_to_char(vict,
                     "Your Powerlevel, Ki, and Stamina have improved drastically! On top of that your Intelligence and Wisdom have improved permanantly!\r\n");
        vict->mod(CharAttribute::Intelligence, 2);
        vict->mod(CharAttribute::Wisdom, 2);
        ch->modPractices(-2000);
        if (GET_LEVEL(ch) < 100) {
            send_to_char(ch, "@D[@mPractice Sessions@D:@R -2000@D]@n\r\n");
            if (level_exp(ch, GET_LEVEL(ch) + 1) - GET_EXP(ch) > 0) {
                ch->modExperience(level_exp(ch, GET_LEVEL(ch) + 1) - GET_EXP(ch));
                send_to_char(ch, "The remaining experience needed for your next level up has been gained!@n\r\n");
            } else {
                send_to_char(ch, "Due to already having enough experience to level up you gain no expereince.\r\n");
            }
        } else {
            ch->gainBaseAllPercent(.025, true);
            send_to_char(ch, "Your Powerlevel, Ki, and Stamina have improved!\r\n");
        }
    }

}

void ash_burn(struct char_data *ch) {

    if(!ch) return;
    if(IS_DEMON(ch) || IS_ANDROID(ch)) return;
    auto room = ch->getRoom();
    if(!room) return;

    auto ash = room->findObjectVnum(1306);
    if(!ash) return;

    if(!(axion_dice(0) > GET_CON(ch))) return;

    if(!IS_ICER(ch)) {
        reveal_hiding(ch, 0);
        ch->decCurST(((GET_MAX_MOVE(ch) * 0.005) + 20) * GET_OBJ_COST(ash));
        act("@RYou choke on the the burning hot @Da@Ws@wh@Dc@Wl@wo@Du@Wd@R!@n", true, ch, nullptr,
            nullptr, TO_CHAR);
        act("@r$n@R chokes on the burning hot @Da@Ws@wh@Dc@Wl@wo@Du@Wd@R!@n", true, ch, nullptr,
            nullptr, TO_ROOM);
    }

    if(!IS_NPC(ch) && !PLR_FLAGGED(ch, PLR_EYEC) && !AFF_FLAGGED(ch, AFF_BLIND)) {
        reveal_hiding(ch, 0);
        act("@DYour eyes sting from the hot ash! You can't see!@n", true, ch, nullptr, nullptr,
            TO_CHAR);
        act("@r$n@D eyes appear to have been hurt by the ash!@n", true, ch, nullptr, nullptr,
            TO_ROOM);
        int duration = 1;
        assign_affect(ch, AFF_BLIND, SKILL_SOLARF, duration, 0, 0, 0, 0, 0, 0);
    }

}

ACMD(do_ashcloud) {

    if (!IS_DEMON(ch)) {
        send_to_char(ch, "You are not trained in the use of ash and fire!\r\n");
        return;
    }
    int level = 1;

    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Syntax: ashcloud (1 | 2 | 3)\r\n");
        return;
    }

    struct obj_data *ash = nullptr, *obj, *next_obj;
    int there = false;

    ash = ch->findObjectVnum(1305);
    if (!ash) {
        send_to_char(ch, "You do not have any ash!\r\n");
        return;
    }

    auto room = ch->getRoom();

    if (room->findObjectVnum(1306)) {
        send_to_char(ch, "You can not pile more ash into the air without causing it to clump together and settle.\r\n");
        return;
    }

    level = atoi(arg);

    int64_t mult = 5;
    double initial = 0.0;

    int otimer = 0;
    int ocost = 0;

    switch (level) {
        case 1:
            otimer = 1;
            ocost = 1;
            mult = 20;
            initial = 0.25;
            break;
        case 2:
            otimer = 2;
            ocost = 2;
            mult = 10;
            initial = 0.10;
            break;
        case 3:
            otimer = 4;
            ocost = 3;
            mult = 5;
            initial = 0.05;
            break;
        default:
            send_to_char(ch, "Syntax: ashcloud (1 | 2 | 3)\r\n");
            return;
    }

    int64_t cost = (GET_MAX_MANA(ch) * initial) + (GET_INT(ch) * mult);

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki!\r\n");
        return;
    }

    if (room->isSunken()) {
        send_to_char(ch, "You can not create an ashcloud here, because it is too wet.\r\n");
        return;
    }

    if (room->sector_type == SECT_SPACE) {
        send_to_char(ch, "You can not create an ashcloud in space.\r\n");
        return;
    }

    if (GET_INT(ch) < axion_dice(-10)) {
        reveal_hiding(ch, 0);
        act("@RYou take a handful of ashes, and when you go to blow flames at it you lose focus. The ashes are blown from your hands by your huge gust of breath.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@r$n@R takes a handful of ashes from $s belongings and blows it out of $s hands with a strong gust of air. @YStrange.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        extract_obj(ash);
        ch->decCurKI(cost);
        return;
    }

    struct obj_data *ashcloud;
    reveal_hiding(ch, 0);

    ch->decCurKI(cost);
    act("@RYou take a handful of ashes and you create a fierce heat within your lungs. With the heat ready you breathe ki infused flames at the pile of ashes! The flames and ashes mix and fill the surrounding area with a hot burning ash!@n",
        true, ch, nullptr, nullptr, TO_CHAR);
    act("@r$n@R takes a handful of ashes and $e breathes ki infused flames at the pile of ashes! The flames and ashes mix and fill the surrounding area with a hot burning ash!@n",
        true, ch, nullptr, nullptr, TO_ROOM);
    ashcloud = read_object(1306, VIRTUAL);
    obj_to_room(ashcloud, IN_ROOM(ch));
    extract_obj(ash);
    GET_OBJ_TIMER(ashcloud) = otimer;
    GET_OBJ_COST(ashcloud) = ocost;

    switch(level) {
        case 3:
            send_to_room(IN_ROOM(ch), "@WThe ashes ripple with an intense aftershock of power.@n\r\n");
            break;
        case 2:
            send_to_room(IN_ROOM(ch), "@WThe ashes ripple with a strong aftershock of power.@n\r\n");
            break;
        default:
            break;
    }

    ash_burn(ch);

}

ACMD(do_resize) {
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    two_arguments(argument, arg, arg2);

    if (!GET_SKILL(ch, SKILL_BUILD)) {
        send_to_char(ch, "You do not have the skill to resize equipment!\r\n");
        return;
    } else if (GET_SKILL(ch, SKILL_BUILD) < 80) {
        send_to_char(ch, "Your build skill must be at least level 80 before you can resize equipment.\r\n");
        return;
    } else {
        if (!*arg || !*arg2) {
            send_to_char(ch, "Syntax: resize (obj) (small | medium)\r\n");
            return;
        }
        if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
            send_to_char(ch, "You don't have that object!\r\n");
            return;
        } else {
            if (!wearable_obj(obj)) {
                send_to_char(ch, "That is not equipment! You can only resize equipment.\r\n");
                return;
            } else {
                if ((ch->getCurST()) < GET_OBJ_WEIGHT(obj) + (GET_MAX_MOVE(ch) / 40)) {
                    send_to_char(ch, "You do not have enough stamina to resize this object at this time.\r\n");
                    return;
                } else if (!strcasecmp(arg2, "small")) {
                    if (GET_OBJ_SIZE(obj) == SIZE_SMALL) {
                        send_to_char(ch, "The equipment is already small sized.\r\n");
                        return;
                    } else {
                        act("@WYou carefully adjust the size of @c$p@W.@n", true, ch, obj, nullptr, TO_CHAR);
                        act("@C$n@W carefully adjusts the size of @c$p@W.@n", true, ch, obj, nullptr, TO_ROOM);
                        GET_OBJ_SIZE(obj) = SIZE_SMALL;
                        ch->decCurST(GET_OBJ_WEIGHT(obj) + (GET_MAX_MOVE(ch) / 40));
                    }
                } else if (!strcasecmp(arg2, "medium")) {
                    if (GET_OBJ_SIZE(obj) == SIZE_MEDIUM) {
                        send_to_char(ch, "The equipment is already medium sized.\r\n");
                        return;
                    } else {
                        act("@WYou carefully adjust the size of @c$p@W.@n", true, ch, obj, nullptr, TO_CHAR);
                        act("@C$n@W carefully adjusts the size of @c$p@W.@n", true, ch, obj, nullptr, TO_ROOM);
                        GET_OBJ_SIZE(obj) = SIZE_MEDIUM;
                        ch->decCurST(GET_OBJ_WEIGHT(obj) + (GET_MAX_MOVE(ch) / 40));
                    }
                } else {
                    send_to_char(ch, "Syntax: resize (obj) (small | medium)\r\n");
                }
            }
        }
    }

}

ACMD(do_healglow) {
    struct char_data *vict;
    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_HEALGLOW)) {
        send_to_char(ch, "You do not know how to perform that technique.\r\n");
        return;
    }

    if (!*arg) {
        vict = ch;
    } else if (GET_SKILL(ch, SKILL_HEALGLOW) < 100) {
        send_to_char(ch,
                     "You can not target anyone except yourself unless you are a master of this technique.\nSyntax: healingglow\r\n");
        return;
    } else if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "Nobody around by that name.\r\n");
        return;
    }

    if (AFF_FLAGGED(vict, AFF_HEALGLOW) && vict == ch) {
        send_to_char(ch, "You already have a healing glow surrounding your body.\r\n");
        return;
    } else if (AFF_FLAGGED(vict, AFF_HEALGLOW)) {
        send_to_char(ch, "They already have a healing glow surrounding their body.\r\n");
        return;
    }

    if (FIGHTING(vict) && vict == ch) {
        send_to_char(ch, "You are too busy fighting!@n\r\n");
        return;
    } else if (FIGHTING(vict)) {
        send_to_char(ch, "They are too busy fighting!@n\r\n");
        return;
    }

    int64_t cost = GET_MAX_MANA(ch) * 0.5;

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki. It requires at least 50%s of your ki in cost.\r\n", "%");
        return;
    } else {
        if (vict == ch) {
            act("@CPlacing your hands on your body you begin to focus your energies. Slowly a strong blue glow glistens and shines across your skin!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C places $s hands on $s body. Slowly a strong blue glow glistens and shines across $s skin!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            vict->affected_by.set(AFF_HEALGLOW);
            int duration = (GET_SKILL(ch, SKILL_HEALGLOW) * 0.1);
            if (duration <= 0)
                duration = 1;
            assign_affect(ch, AFF_HEALGLOW, SKILL_HEALGLOW, duration, 0, 0, 0, 0, 0, 0);
            ch->decCurKI(cost);
        } else {
            act("@CPlacing your hands on @c$N's@C body you begin to focus your energies. Slowly a strong blue glow glistens and shines across $S skin!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@c$n@C places $s hands on YOUR body. Slowly a strong blue glow glistens and shines across your skin!@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@c$n@C places $s hands on @c$N's@C body. Slowly a strong blue glow glistens and shines across $S skin!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            int duration = (GET_SKILL(ch, SKILL_HEALGLOW) * 0.1);
            duration += rand_number(-2, 1);
            if (duration <= 0)
                duration = 1;
            assign_affect(ch, AFF_HEALGLOW, SKILL_HEALGLOW, duration, 0, 0, 0, 0, 0, 0);
            ch->decCurKI(cost);
        }
    }
}

/* Kay's lame skill */
ACMD(do_amnisiac) {

    if (strcasecmp(GET_NAME(ch), "Kanashimi")) {
        send_to_char(ch, "You do not know how to perform that technique.\r\n");
        return;
    }

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct char_data *vict;
    int skill;

    two_arguments(argument, arg, arg2);

    if (!*arg || !*arg2) {
        send_to_char(ch, "Syntax: amnesiac (target) (skill)\r\n");
        return;
    }

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "Perform amnesiac kiss on whom?\r\n");
        return;
    }

    skill = find_skill_num(arg2, SKTYPE_SKILL);

    if (skill <= 0) {
        send_to_char(ch, "That is not a skill\r\n");
        return;
    }

    int chance = axion_dice(0), perc = 10 + GET_INT(ch), cost = GET_MAX_MANA(ch) * 0.18;

    if (cost > (ch->getCurKI())) {
        send_to_char(ch, "You do not have enough ki!\r\n");
        return;
    } else if (perc < chance) {
        act("@WYou attempt to grab @C$N@W to kiss $M, but $E evades!@n", true, ch, nullptr, vict, TO_CHAR);
        act("@M$n@W attempts to grab you and leans in with puckered lips, but you managed to evade!@n", true, ch,
            nullptr, vict, TO_VICT);
        act("@M$n@W attempts to grab @C$N@W and leans in with puckered lips, but $E manages to evade!@n", true, ch,
            nullptr, vict, TO_NOTVICT);
        ch->decCurKI(cost);
        return;
    } else {
        act("@WYou reach out quickly, grabbing @C$N@W and pulling them in close to you. Just as quick, you pull their head forcefully towards yours, planting a deep and heavy kiss. The fool wobbles a bit, shocked.  It is unlikely that @c$E@W will be able to focus very well on that skill, not with the thought of your lips on their mind.",
            true, ch, nullptr, vict, TO_CHAR);
        act("@M$n@W grabs you, giving you a deep, passionate kiss. Your mind is suddenly overwhelmed, and in your shock you seem to forget a few of your tricks.",
            true, ch, nullptr, vict, TO_VICT);
        act("@WYou see @C$n@W quickly grab @C$N@W, pulling them into a deep, almost passionate kiss. @C$N@W seems shocked, and wobbles a bit, grabbing at @c$s@W head once @C$n@W lets go.",
            true, ch, nullptr, vict, TO_NOTVICT);
        ch->decCurKI(cost);
        GET_STUPIDKISS(vict) = skill;
        return;
    }

}

/* Demon's lame skill */
ACMD(do_metamorph) {

    if (IS_NPC(ch))
        return;

    if (!know_skill(ch, SKILL_METAMORPH)) {
        return;
    }

    if (GET_ALIGNMENT(ch) >= 51) {
        send_to_char(ch, "Your heart is too pure to use that technique!\r\n");
        return;
    }

    int64_t cost = (GET_MAX_MANA(ch) * 0.16);

    if (AFF_FLAGGED(ch, AFF_METAMORPH)) {
        send_to_char(ch, "You are already surrounded by a dark aura!\r\n");
        return;
    }

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki. You need %s.\r\n", add_commas(cost).c_str());
        return;
    }

    int chance = axion_dice(0), perc = (GET_WIS(ch) * 2);

    if (perc < 100 && perc > 60)
        perc += 100 - perc;
    else if (perc < 100)
        perc += 10;

    ch->decCurKI(cost / 2);
    if (perc < chance) {
        act("@WYou focus your energies and prepare your @RDark Metamorphisis@W but screw up your focus!@n", true, ch,
            nullptr, nullptr, TO_CHAR);
        act("@WA dark @Rred@W glow starts to surround @C$n@W, but it fades quickly.@n", true, ch, nullptr, nullptr,
            TO_ROOM);

        return;
    } else {
        act("'@RDark@W...' An explosion of sanguine aura erupts over the surface of your body, your eyes darkening to a bleeding crimson. The flaring glow emanating from your body pronounces the shadows cast, a darkening umbrage that threatens a malicious promise. Fists clench tightly, muscles bulking as you hiss; You complete the transition, relaxing visibly, '...@RMetamorphosis@W'@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("'@RDark@W...' An explosion of sanguine aura erupts over the surface of @C$n@W's body, $s eyes darkening to a bleeding crimson. The flaring glow emanating from $s body pronounces the shadows cast, a darkening umbrage that threatens a malicious promise. Fists clench tightly, muscles bulking as $e hisses; $e completes the transition, relaxing visibly, '...@RMetamorphosis@W'@n",
            true, ch, nullptr, nullptr, TO_ROOM);

        int duration = GET_INT(ch) / 12;
        assign_affect(ch, AFF_METAMORPH, SKILL_METAMORPH, duration, 0, 0, 0, 0, 0, 0);
        ch->incCurHealthPercent(.6);
        return;
    }

}

ACMD(do_shimmer) {

    int skill = 0, perc = 0, location = 0;
    int64_t cost = 0;
    struct char_data *tar = nullptr;

    char arg[MAX_INPUT_LENGTH] = "";

    one_argument(argument, arg);

    if (!IS_NPC(ch)) {
        if (PRF_FLAGGED(ch, PRF_ARENAWATCH)) {
            ch->pref.set(PRF_ARENAWATCH);
            ARENA_IDNUM(ch) = -1;
            send_to_char(ch, "You stop watching the arena action.\r\n");
        }
    }
    if (strcasecmp(GET_NAME(ch), "Anubis")) {
        send_to_char(ch, "You do not even know how to perform that skill!\r\n");
        return;
    } else if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "You are busy piloting a ship!\r\n");
        return;
    } else if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 19800 && GET_ROOM_VNUM(IN_ROOM(ch)) <= 19899) {
        send_to_char(ch, "@rYou are in a pocket dimension!@n\r\n");
        return;
    } else if (!*arg) {
        send_to_char(ch, "Who or where do you want to shimmer to? [target | planet-(planet name) | afterlife]\r\n");
        send_to_char(ch, "Example: shimmer goku\nExample 2: shimmer planet-earth\r\n");
        return;
    }

    cost = GET_MAX_MANA(ch) / 40;

    if ((ch->getCurKI()) - cost < 0) {
        send_to_char(ch, "You do not have enough ki to instantaneously move.\r\n");
        return;
    }

    perc = axion_dice(0);
    skill = 100;

    if (!strcasecmp(arg, "planet-earth")) {
        location = 300;
    } else if (!strcasecmp(arg, "planet-namek")) {
        location = 10222;
    } else if (!strcasecmp(arg, "planet-frigid")) {
        location = 4017;
    } else if (!strcasecmp(arg, "planet-vegeta")) {
        location = 2200;
    } else if (!strcasecmp(arg, "planet-konack")) {
        location = 8006;
    } else if (!strcasecmp(arg, "planet-aether")) {
        location = 12024;
    } else if (!strcasecmp(arg, "afterlife")) {
        location = 6000;
    } else if (!(tar = get_char_vis(ch, arg, nullptr, FIND_CHAR_WORLD))) {
        send_to_char(ch, "@RThat target doesn't exist.@n\r\n");
        send_to_char(ch, "Who or where do you want to shimmer to? [target | planet-(planet name) | afterlife]\r\n");
        send_to_char(ch, "Example: shimmer goku\nExample 2: shimmer planet-earth\r\n");
        return;
    }

    if (skill < perc || (FIGHTING(ch) && rand_number(1, 2) <= 1)) {
        if (tar != nullptr) {
            if (tar != ch) {
                send_to_char(ch,
                             "You prepare to move instantly but mess up the process and waste some of your ki!\r\n");
                ch->decCurKI(cost);
                WAIT_STATE(ch, PULSE_2SEC);
                return;
            } else {
                send_to_char(ch,
                             "Moving to yourself would be kinda impossible wouldn't it? If not that then it would at least be pointless.\r\n");
                return;
            }
        } else {
            send_to_char(ch, "You prepare to move instantly but mess up the process and waste some of your ki!\r\n");
            ch->decCurKI(cost);
            WAIT_STATE(ch, PULSE_2SEC);
            return;
        }
    }

    reveal_hiding(ch, 0);
    WAIT_STATE(ch, PULSE_2SEC);
    if (tar != nullptr) {
        if (tar == ch) {
            send_to_char(ch,
                         "Moving to yourself would be kinda impossible wouldn't it? If not that then it would at least be pointless.\r\n");
            return;
        } else if (GRAPPLING(ch) && GRAPPLING(ch) == tar) {
            send_to_char(ch, "You are already in the same room with them and are grappling with them!\r\n");
            return;
        } else if (GET_ADMLEVEL(tar) > 0 && GET_ADMLEVEL(ch) < 1) {
            send_to_char(ch, "That immortal prevents you from reaching them.\r\n");
            return;
        } else if (ROOM_FLAGGED(IN_ROOM(tar), ROOM_NOINSTANT)) {
            send_to_char(ch, "You can not go there as it is a protected area!\r\n");
            return;
        } else if (GRAPPLING(ch) && AFF_FLAGGED(GRAPPLING(ch), AFF_SPIRIT)) {
            send_to_char(ch, "You can not take the dead with you!\r\n");
            return;
        } else if (DRAGGING(ch) && AFF_FLAGGED(DRAGGING(ch), AFF_SPIRIT)) {
            send_to_char(ch, "You can not take the dead with you!\r\n");
            return;
        } else if (GRAPPLED(ch) && AFF_FLAGGED(GRAPPLED(ch), AFF_SPIRIT)) {
            send_to_char(ch, "You can not take the dead with you!\r\n");
            return;
        }

        ch->decCurKI(cost);
        act("@wYour body begins to fade away almost appearing ghost like, before a ripple passes through your image and your are gone in an instant!@n",
            true, ch, nullptr, tar, TO_CHAR);
        act("@w$n@w appears in an instant out of nowhere right next to you!@n", true, ch, nullptr, tar, TO_VICT);
        act("@w$n@w body begins to fade away almost appearing ghost like, before a ripple passes through $s image and $e is gone in an instant!@n",
            true, ch, nullptr, tar, TO_NOTVICT);
        ch->playerFlags.set(PLR_TRANSMISSION);
        handle_teleport(ch, tar, 0);
    } else {
        ch->decCurKI(cost);
        act("@wYour body begins to fade away almost appearing ghost like, before a ripple passes through your image and your are gone in an instant!@n",
            true, ch, nullptr, tar, TO_CHAR);
        act("@w$n@w body begins to fade away almost appearing ghost like, before a ripple passes through $s image and $e is gone in an instant!@n",
            true, ch, nullptr, tar, TO_NOTVICT);
        handle_teleport(ch, nullptr, location);
    }

}

ACMD(do_channel) {

    if (!IS_DEMON(ch) || GET_SKILL_BASE(ch, SKILL_STYLE) < 40) {
        send_to_char(ch, "You are not a Demon!\r\n");
        return;
    }

    if (GET_SKILL_BASE(ch, SKILL_STYLE) < 40) {
        send_to_char(ch, "This requires a fighting style at level 40 or more!!\r\n");
        return;
    }

    int64_t cost = GET_MAX_MANA(ch) * 0.15;

    int chance = axion_dice(0), skill = GET_SKILL(ch, SKILL_STYLE);

    if (cost > (ch->getCurKI())) {
        send_to_char(ch, "You do not have enough ki to channel with!\r\n");
        return;
    }

    struct obj_data *obj, *next_obj = nullptr, *ruby = nullptr;
    int found = false;

    for (obj = ch->contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (found == false && GET_OBJ_VNUM(obj) == 6600) {
            if (!OBJ_FLAGGED(obj, ITEM_HOT)) {
                found = true;
                ruby = obj;
            }
        }
    }

    if (found == false) {
        send_to_char(ch, "You do not have any uncharged blood rubies.\r\n");
        return;
    }

    if (ROOM_EFFECT(IN_ROOM(ch)) <= 0) {
        send_to_char(ch, "There is no lava here!\r\n");
        return;
    }

    if (ruby) {
        if (skill < chance) {
            act("@RAs you move your ki through the lava you begin to draw heat away from it into the ruby. You screw up the rate of heating though and cause the ruby to crumble to dust!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@RAs $n@R moves $s ki through the lava $e begins to draw heat away from it into a blood ruby. However $e screws up the rate of heating and causes the ruby to crumble to dust!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            extract_obj(ruby);
        } else {
            act("@RAs you move your ki through the lava you begin to draw heat away from it into the ruby. You do so at an even rate and end up with a glowing red hot blood ruby!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@RAs $n@R moves $s ki through the lava $e begins to draw heat away from it into a blood ruby. The ruby glows red hot as $e finishes the process of channeling the heat!@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ROOM_EFFECT(IN_ROOM(ch)) = 0;
            ruby->extra_flags.set(ITEM_HOT);
        }
        ch->decCurKI(cost);
        WAIT_STATE(ch, PULSE_1SEC);
    }

}

ACMD(do_hydromancy) {

    if (!IS_TSUNA(ch) || GET_SKILL_BASE(ch, SKILL_STYLE) <= 0) {
        send_to_char(ch, "You know nothing about hydromancy!\r\n");
        return;
    }
    auto r = ch->getRoom();
    int skill = GET_SKILL_BASE(ch, SKILL_STYLE), chance = axion_dice(0);
    int64_t cost = 0;

    cost = (GET_MAX_MANA(ch) / 12) - (GET_INT(ch) * GET_LEVEL(ch));

    if (r->geffect >= 0 && r->sector_type != SECT_WATER_SWIM &&
            r->sector_type != SECT_WATER_NOSWIM) {
        if (r->sector_type != SECT_UNDERWATER) {
            send_to_char(ch, "There is not sufficient water here.\r\n");
            return;
        } else {
            send_to_char(ch, "There is too much water here to control!\r\n");
            return;
        }
    }

    if (cost <= 0)
        cost = 100;

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki to manipulate any water around you.\r\n");
        return;
    }

    if (GET_COOLDOWN(ch) > 0) {
        send_to_char(ch, "You must wait a short period before concentrating again.\r\n");
        return;
    }


    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Syntax 1: hydromancy flood (direction)\r\n");
        send_to_char(ch, "Example: hydromancy flood nw\r\n");
        send_to_char(ch, "\nSyntax 2: hydromancy spike\r\n");
        return;
    }

    int attempt = 0;

    if (!strcasecmp(arg, "spike")) {
        struct obj_data *obj;

        cost = 100 + (GET_SKILL(ch, SKILL_STYLE) / (1 + (GET_MAX_MANA(ch) * 0.5)));

        if ((ch->getCurKI()) < cost) {
            send_to_char(ch, "You do not have enough ki to form an ice spike.\r\n");
            return;
        }

        if (skill < chance) {
            ch->decCurKI(cost);
            act("@CYou press your palms together in front of your body but you fail to produce the proper control to form the spike!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C presses $s palms together and then slowly pulls them apart. Nothing important appears to have happened.",
                true, ch, nullptr, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_STYLE, 2);
            return;
        }

        if (skill >= 100) {
            obj = read_object(19058, VIRTUAL);
        } else if (skill >= 50) {
            obj = read_object(19057, VIRTUAL);
        } else if (skill >= 1) {
            obj = read_object(19056, VIRTUAL);
        }

        ch->decCurKI(cost);
        act("@CYou press your palms together in front of your body and focusing ki you force water up along your body. That water pools between your palms and as pull your palms apart a @c$p@C forms!@n",
            true, ch, obj, nullptr, TO_CHAR);
        act("@c$n@C presses $s palms together in front of $s body and water begins to flow up $s body and pools between $s palms. Slowly pulling them apart reveals a @c$p@C as it forms between them!@n",
            true, ch, obj, nullptr, TO_VICT);
        if (GET_OBJ_WEIGHT(obj) + (ch->getCarriedWeight()) <= CAN_CARRY_W(ch))
            obj_to_char(obj, ch);
        else {
            send_to_char(ch, "You are unable to hold it and so let it go at your feet.\r\n");
            act("@C$n@w drops an ice spike.@n", true, ch, nullptr, nullptr, TO_ROOM);
            obj_to_room(obj, IN_ROOM(ch));
        }
        improve_skill(ch, SKILL_STYLE, 1);
        GET_COOLDOWN(ch) = 10;
    } else if (!strcasecmp(arg, "flood")) {
        if (!*arg2) {
            send_to_char(ch, "Syntax 1: hydromancy flood (direction)\r\n");
            send_to_char(ch, "Example: hydromancy flood nw\r\n");
            send_to_char(ch, "\nSyntax 2: hydromancy spike\r\n");
            return;
        }

        attempt = search_block(arg2, dirs, false);
        auto e = r->dir_option[attempt];
        if(!e) {
            send_to_char(ch, "You can not flood the water that direction!\r\n");
            return;
        }
        auto dest = e->getDestination();
        if(!dest) {
            send_to_char(ch, "You can not flood the water that direction!\r\n");
            return;
        }
        if(EXIT_FLAGGED(e, EX_CLOSED)) {
            send_to_char(ch, "You can not flood the water that direction!\r\n");
            return;
        }

        struct char_data *vict, *next_v;

        int last = LASTATK(ch);
        LASTATK(ch) = 500;
        char bun[MAX_STRING_LENGTH], bunn[MAX_STRING_LENGTH];

        if (skill < chance) {
            act("@BUsing your ki you attempt to create a rush of water! @RYou fail!@n", true, ch, nullptr, nullptr,
                TO_CHAR);
            act("@b$n@B seems to attempt to create water with $s ki! @RHowever, $e fails!@n", true, ch, nullptr,
                nullptr, TO_ROOM);
            ch->decCurKI(cost);
            WAIT_STATE(ch, PULSE_2SEC);
        } else {
            ch->decCurKI(cost);
            sprintf(bun, "@BUsing your ki you create a rush of water flooding away toward the @C%s@B!@n",
                    dirs[attempt]);
            sprintf(bunn, "@B$n@B uses $s ki to create a rush of water flooding away toward the @C%s@B!@n",
                    dirs[attempt]);
            act(bun, true, ch, nullptr, nullptr, TO_CHAR);
            act(bunn, true, ch, nullptr, nullptr, TO_ROOM);

            for (vict = r->people; vict; vict = next_v) {
                next_v = vict->next_in_room;
                if (vict == ch)
                    continue;
                if (!can_kill(ch, vict, nullptr, 1)) {
                    act("@CYou are protected from the water!@n", true, vict, nullptr, nullptr, TO_VICT);
                    act("@C$n@C is protected from the water!@n", true, vict, nullptr, nullptr, TO_ROOM);
                } else if (IS_KANASSAN(vict)) {
                    act("@CYou effortlessly swim against the current.@n", true, vict, nullptr, nullptr, TO_CHAR);
                    act("@C$n@C effortlessly swims against the current.@n", true, vict, nullptr, nullptr, TO_ROOM);
                } else if (GET_SKILL_BASE(vict, SKILL_BALANCE) >= axion_dice(-10)) {
                    act("@CYou manage to keep your balance and are not swept away!@n", true, vict, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@C manages to keep $s balance and is not swept away!@n", true, vict, nullptr, nullptr,
                        TO_ROOM);
                } else if (AFF_FLAGGED(ch, AFF_FLYING)) {
                    act("@CYou fly above the rushing waters and are untouched.@n", true, vict, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@C flies above the rushing waters and is untouched.@n", true, vict, nullptr, nullptr,
                        TO_ROOM);
                } else {
                    act("@cYou are caught by the rushing waters and sent tumbling away!@n", true, vict, nullptr,
                        nullptr, TO_CHAR);
                    act("@c$n@c is caught by the rushing waters and sent tumbling away!@n", true, vict, nullptr,
                        nullptr, TO_ROOM);
                    do_simple_move(vict, attempt, true);
                    hurt(0, 0, ch, vict, nullptr, cost * 4, 1);
                }
            }
            dest->geffect = -3;
            LASTATK(ch) = last;
            WAIT_STATE(ch, PULSE_2SEC);
            GET_COOLDOWN(ch) = 15;
        }
    } else {
        send_to_char(ch, "Syntax 1: hydromancy (flood) (direction)\r\n");
        send_to_char(ch, "Example: hydromancy flood nw\r\n");
        send_to_char(ch, "\nSyntax 2: hydromancy spike\r\n");
        return;
    }

}

ACMD(do_kanso) {

    if (IS_NPC(ch)) /* No mobs */
        return;

    if (strcasecmp(GET_NAME(ch), "Levanthoth")) { /* NOT the right player */
        send_to_char(ch, "You do not know how to perform that technique. \r\n");
        return;
    }

    struct char_data *vict;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "Perform Kanso Suru on who?\r\n");
        return;
    }

    if (IS_ANDROID(vict)) {
        send_to_char(ch, "Mechanical beings are not effected by this technique.\r\n");
        return;
    }

    if (!can_kill(ch, vict, nullptr, 0)) {
        return;
    }

    int64_t cost = GET_MAX_MANA(ch) / GET_INT(ch);
    int dice = axion_dice(-5), skill = GET_INT(ch), pdice = axion_dice(0);
    int dam = rand_number(1, 4);
    struct affected_type af;

    /* Bonus based on wisdom */
    if (GET_WIS(ch) > axion_dice(-5))
        dam += 1;

    if ((ch->getCurKI()) < cost) { /* Not enough ki */
        send_to_char(ch, "You do not have enough ki.\r\n");
        return;
    }

    if (skill < dice) { /* Failed the technique, user's fault */
        act("You close your eyes and focus, before bounding effortlessly toward $N. Closing the distance, you place your hands on $N's chest but nothing happens!\r\n",
            true, ch, nullptr, vict, TO_CHAR); /* Message Character $n sees */
        act("$n closes $s eyes and bounds effortlessly towards you. Closing the distance, $e places $s hands on your chest but nothing happens!\r\n",
            true, ch, nullptr, vict, TO_VICT); /* Message Vict $N Sees */
        act("$n closes $s eyes and bounds toward $N. Smirking, $e puts $m hands on $N's chest but nothing seems to happen.\r\n",
            true, ch, nullptr, vict, TO_NOTVICT); /* Message everyone else sees */
        ch->decCurKI(cost);
        WAIT_STATE(ch, PULSE_2SEC); /* 2 second lag for the technique */
        return;
    } else { /* Success! */
        /* Main Attack Success Messages */
        act("You close your eyes and focus, before effortlessly bounding towards $N. Closing the distance, you smirk at $N and place your hands on their chest. Electricity flows into their body as you draw water out of their very cells!\r\n",
            true, ch, nullptr, vict, TO_CHAR); /* Message Character $n sees */
        act("$n closes $s eyes and focuses, before effortlessly bounding toward you. Closing the distance, $e smirks and places both of $s hands on your chest. Electricity begins to pulse through your body and a great thirst takes hold, as if $n is drawing the water from your body!\r\n",
            true, ch, nullptr, vict, TO_VICT); /* Message Vict $N Sees */
        act("$n closes $s eyes, before effortlessly bounding toward $N. Closing the distance, $n smirks and places both $s hands on $N's chest. Electricity seems to pass from $n's body to $N's!\r\n",
            true, ch, nullptr, vict, TO_NOTVICT); /* Message everyone else sees */
        /* End Main Messages */

        ch->decCurKI(cost);

        /* Handle the thirst aspect */
        if (GET_COND(vict, THIRST) - dam >= 0)
            GET_COND(vict, THIRST) -= dam;
        else
            GET_COND(vict, THIRST) = 0;
        if (GET_COND(ch, THIRST) + dam <= 48)
            GET_COND(ch, THIRST) += dam;
        else
            GET_COND(ch, THIRST) = 48;

        /* Heal the user */
        ch->incCurHealth((ch->getEffMaxPL() * .01) * dam);

        WAIT_STATE(ch, PULSE_2SEC); /* 2 second lag for the technique */

        if (!AFF_FLAGGED(vict, AFF_HYDROZAP)) { /* Drop their AGL/CON if not already lowered */
            send_to_char(vict, "@RYou feel less agile and your muscles ache!@n\r\n");
            assign_affect(vict, AFF_HYDROZAP, 0, -1, 0, -4, 0, -4, 0, 0);
        }

        if (skill > pdice && !AFF_FLAGGED(vict, AFF_PARA)) { /* Paralyze them too */
            act("@R$N@W is paralyzed by the attack!@n", true, ch, nullptr, vict,
                TO_CHAR); /* Message Character $n sees */
            act("@RYou are paralyzed by the attack!@n", true, ch, nullptr, vict, TO_VICT); /* Message Vict $N Sees */
            act("@R$N@Wis paralyzed by the attack!@n", true, ch, nullptr, vict,
                TO_NOTVICT); /* Message everyone else sees */
            af.type = SKILL_PARALYZE;
            af.duration = rand_number(1, 3);
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PARA;
            affect_join(vict, &af, false, false, false, false);
        }

    }

}

void rpp_feature(struct char_data *ch, const char *arg) {
    int cost = 0, change = false;

    if (!*arg) {
        send_to_char(ch,
                     "Syntax: rpp 13 (description)\nExample: rpp 13 a large red scar on his face\nDisplayed to others: He has a large red scar on his face.\r\n");
        return;
    }

    if (strlen(arg) > 60) {
        send_to_char(ch, "Please limit it to 60 characters.\r\n");
        return;
    }

    if (!GET_FEATURE(ch)) {
        cost = 2;
    } else {
        cost = 2;
        change = true;
    }

    if (cost > ch->getRPP()) {
        send_to_char(ch, "You do not have enough RPP in your Bank for that!\r\n");
        return;
    } else {
        char sex[128], buf8[MAX_INPUT_LENGTH];
        sprintf(sex, "%s", GET_SEX(ch) == SEX_FEMALE ? "She" : GET_SEX(ch) == SEX_MALE ? "He" : "It");

        ch->modRPP(-cost);
        sprintf(buf8, "...%s has %s.", sex, arg);
        send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
        send_to_char(ch,
                     "You now have the following line underneath you when someone sees you in a room as:\n@C%s@n\r\n",
                     buf8);
        GET_FEATURE(ch) = strdup(buf8);
        if (change == true) {
            send_to_imm(
                    "%s has altered their extra description. Make sure the reason is legit! If it is then reimb them 2 RPP.\r\n",
                    GET_USER(ch));
            send_to_char(ch,
                         "The immortals have been notified about this change. It had better have been for a good reason.\r\n");
        }
        basic_mud_log("%s RPP Feature: '%s' Check for rule compliance.", GET_USER(ch), buf8);
        return;
    }

}

ACMD(do_instill) {

    if (IS_NPC(ch))
        return;

    struct obj_data *obj, *token;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!*arg || !*arg2) {
        send_to_char(ch, "Syntax: instill (token) (target)\r\n");
        return;
    }

    if (!(token = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        send_to_char(ch, "Syntax: instill (token) (target)\r\n");
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg2, nullptr, ch->contents))) {
        send_to_char(ch, "Syntax: instill (token) (target)\r\n");
        return;
    }

    if (!OBJ_FLAGGED(token, ITEM_TOKEN)) {
        send_to_char(ch, "That is not a token.\r\n");
        return;
    }

    if (OBJ_FLAGGED(token, ITEM_FORGED)) {
        send_to_char(ch, "That token is a forgery!\r\n");
        return;
    }

    if (!wearable_obj(obj)) {
        send_to_char(ch, "You can only instill tokens into equipment.\r\n");
        return;
    }

    if (!OBJ_FLAGGED(obj, ITEM_SLOT1) && !OBJ_FLAGGED(obj, ITEM_SLOT2)) {
        send_to_char(ch, "That piece of equipment does not have any slots.\r\n");
        return;
    }

    if (OBJ_FLAGGED(obj, ITEM_SLOTS_FILLED)) {
        send_to_char(ch, "That piece of equipment has already had its token slots filled. This can not be reversed.");
        return;
    } else { /* It has at least one open slot. */
        int stat = 0, raise = 0;
        stat = token->affected[0].location;

        if (obj->affected[0].location != 0 && obj->affected[1].location != 0 && obj->affected[2].location != 0 &&
            obj->affected[3].location != 0 && obj->affected[4].location != 0 && obj->affected[5].location != 0) {
            if (obj->affected[0].location != stat && obj->affected[1].location != stat &&
                obj->affected[2].location != stat && obj->affected[3].location != stat &&
                obj->affected[4].location != stat && obj->affected[5].location != stat) {
                send_to_char(ch, "This already has as many different stats as it can hold.\r\n");
                return;
            }
        }

        act("@GYou instill the token into @g$p@G. It glows @ggreen@G for a moment before returning to normal. The token disappears with the glow.@n",
            true, ch, obj, nullptr, TO_CHAR);
        act("@g$n@G instills a token into @g$p@G. It glows @ggreen@G for a moment before returning to normal. The token disappears with the glow.@n",
            true, ch, obj, nullptr, TO_ROOM);
        raise = token->affected[0].modifier;
        extract_obj(token);

        if (OBJ_FLAGGED(obj, ITEM_SLOT1))
            obj->extra_flags.set(ITEM_SLOTS_FILLED);
        else if (OBJ_FLAGGED(obj, ITEM_SLOT2) && !OBJ_FLAGGED(obj, ITEM_SLOT_ONE))
            obj->extra_flags.set(ITEM_SLOT_ONE);
        else if (OBJ_FLAGGED(obj, ITEM_SLOT2) && OBJ_FLAGGED(obj, ITEM_SLOT_ONE))
            obj->extra_flags.set(ITEM_SLOTS_FILLED);

        /* Check it's slots for the appropriate stat and add to it if possible */
        if (obj->affected[0].location == stat) {
            obj->affected[0].modifier += raise;
        } else if (obj->affected[1].location == stat) {
            obj->affected[1].modifier += raise;
        } else if (obj->affected[2].location == stat) {
            obj->affected[2].modifier += raise;
        } else if (obj->affected[3].location == stat) {
            obj->affected[3].modifier += raise;
        } else if (obj->affected[4].location == stat) {
            obj->affected[4].modifier += raise;
        } else if (obj->affected[5].location == stat) {
            obj->affected[5].modifier += raise;
        } else if (obj->affected[0].location == 0) { /* It's empty, put it here regardless */
            obj->affected[0].location = stat;
            obj->affected[0].modifier = raise;
        } else if (obj->affected[1].location == 0) { /* It's empty, put it here regardless */
            obj->affected[1].location = stat;
            obj->affected[1].modifier = raise;
        } else if (obj->affected[2].location == 0) { /* It's empty, put it here regardless */
            obj->affected[2].location = stat;
            obj->affected[2].modifier = raise;
        } else if (obj->affected[3].location == 0) { /* It's empty, put it here regardless */
            obj->affected[3].location = stat;
            obj->affected[3].modifier = raise;
        } else if (obj->affected[4].location == 0) { /* It's empty, put it here regardless */
            obj->affected[4].location = stat;
            obj->affected[4].modifier = raise;
        } else if (obj->affected[5].location == 0) { /* It's empty, put it here regardless */
            obj->affected[5].location = stat;
            obj->affected[5].modifier = raise;
        }

    }

}

ACMD(do_hayasa) {

    if (!IS_NPC(ch) && !GET_SKILL(ch, SKILL_HAYASA)) {
        send_to_char(ch, "You do not know how to perform this technique!\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_HAYASA)) {
        send_to_char(ch, "You are already focusing ki to continually speed up your movements.\r\n");
        return;
    }

    int skill = GET_SKILL(ch, SKILL_HAYASA), prob = axion_dice(0);
    int64_t cost = GET_MAX_MANA(ch) / (skill / 2);
    int duration = 1;

    if (skill >= 100) {
        duration = 6;
    } else if (skill >= 80) {
        duration = 5;
    } else if (skill >= 50) {
        duration = 4;
    } else if (skill >= 25) {
        duration = 3;
    } else {
        duration = 2;
    }

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki.\r\n");
        return;
    } else if (skill < prob) {
        ch->decCurKI(cost);
        act("@CYou close your eyes for a brief moment and focus your ki around your body as a soft blue glow. The glow disappears though as you fail to maintain the effect...@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C closes $s eyes for a brief moment and a soft blue glow begins to form around $s body. The glow disappears a second later though and $e frowns.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        improve_skill(ch, SKILL_HAYASA, 1);
        WAIT_STATE(ch, PULSE_2SEC);
    } else {
        struct affected_type af;

        ch->decCurKI(cost);
        af.type = SPELL_HAYASA;
        af.duration = duration;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_HAYASA;
        affect_join(ch, &af, false, false, false, false);
        GET_SPEEDBOOST(ch) = GET_SPEEDCALC(ch) * 0.5;
        reveal_hiding(ch, 0);
        act("@CYou close your eyes for a brief moment and focus your ki around your body as a soft blue glow. All your movements are faster now!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C closes $s eyes for a brief moment and a soft blue glow begins to form around $s body. The glow pulsates gently as $e opens his eyes and smiles.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        improve_skill(ch, SKILL_HAYASA, 1);
        WAIT_STATE(ch, PULSE_2SEC);
    }
}

/* This is the mortal dig command. */
ACMD(do_bury) {

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }

    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are busy grappling with someone!\r\n");
        return;
    }

    if (ABSORBING(ch) || ABSORBBY(ch)) {
        send_to_char(ch, "You are busy struggling with someone!\r\n");
        return;
    }

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Syntax: dig [bury (item) | uncover]\r\n");
        return;
    }

    if (SECT(IN_ROOM(ch)) != SECT_FIELD && SECT(IN_ROOM(ch)) != SECT_HILLS && SECT(IN_ROOM(ch)) != SECT_FOREST &&
        SECT(IN_ROOM(ch)) != SECT_DESERT && SECT(IN_ROOM(ch)) != SECT_MOUNTAIN) {
        send_to_char(ch, "You are not in a room with enough available dirt or sand to dig.\r\n");
        return;
    }

    struct obj_data *obj = nullptr, *buried = nullptr, *fobj = nullptr, *next_obj;

    for (buried = ch->getRoom()->contents; buried; buried = next_obj) {
        next_obj = buried->next_content;
        if (OBJ_FLAGGED(buried, ITEM_BURIED)) {
            fobj = buried;
        }
    }

    if (!strcasecmp(arg, "bury")) {
        if (!*arg2) {
            send_to_char(ch, "Bury what?\r\n");
            return;
        } else if (!(obj = get_obj_in_list_vis(ch, arg2, nullptr, ch->contents))) {
            send_to_char(ch, "You don't have that object to bury.\r\n");
            return;
        } else if (fobj != nullptr) {
            send_to_char(ch, "There is already something buried near here.\r\n");
            return;
        } else {
            if (SECT(IN_ROOM(ch)) != SECT_DESERT) {
                act("@yYou start digging in a spot of soft dirt. Once you have an appropriately sized hole you drop @G$p@y in and then cover it.@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@y starts digging in a spot of soft dirt. Once $e has an appropriately sized hole $e drops @G$p@y in and then covers it.@n",
                    true, ch, obj, nullptr, TO_ROOM);
            } else {
                act("@YYou start digging in a spot of soft sand. Once you have an appropriately sized hole you drop @G$p@Y in and then cover it.@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@Y starts digging in a spot of soft sand. Once $e has an appropriately sized hole $e drops @G$p@Y in and then covers it.@n",
                    true, ch, obj, nullptr, TO_ROOM);
            }
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
            obj->extra_flags.set(ITEM_BURIED);
        }
    } else if (!strcasecmp(arg, "uncover")) {
        if (fobj == nullptr) {
            send_to_char(ch, "There is nothing buried here.\r\n");
            return;
        } else {
            if (SECT(IN_ROOM(ch)) != SECT_DESERT) {
                act("@yYou slowly dig and reveal @G$p@y buried in the dirt! You pull it out and set it on the ground before covering the hole back up.@n",
                    true, ch, fobj, nullptr, TO_CHAR);
                act("@C$n@y starts digging and shortly reveals @G$p@y buried in the dirt! Quickly $e pulls it out and sets it on the ground before covering the hole back up.@n",
                    true, ch, fobj, nullptr, TO_ROOM);
            } else {
                act("@YYou slowly dig and reveal @G$p@Y buried in the sand! You pull it out and set it on the ground before covering the hole back up.@n",
                    true, ch, fobj, nullptr, TO_CHAR);
                act("@C$n@Y starts digging and shortly reveals @G$p@Y buried in the sand! Quickly $e pulls it out and sets it on the ground before covering the hole back up.@n",
                    true, ch, fobj, nullptr, TO_ROOM);
            }
            fobj->extra_flags.reset(ITEM_BURIED);
        }
    } else {
        send_to_char(ch, "Syntax: dig [bury (item) | uncover]\r\n");
        return;
    }

}

ACMD(do_arena) {

    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (IN_ARENA(ch)) {
        send_to_char(ch, "You are too busy competing to be a spectator.\r\n");
        return;
    }

    if (!*arg) {
        send_to_char(ch,
                     "Syntax: arena (fighter number of participant)\r\n        arena look\r\n        arena scan\r\n        arena stop\r\n");
        return;
    } else if (!strcasecmp(arg, "stop")) {
        send_to_char(ch, "You stop viewing what's going on in the arena.\r\n");
        ch->pref.reset(PRF_ARENAWATCH);
        ARENA_IDNUM(ch) = -1;
        return;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) != 17875) {
        send_to_char(ch, "You are not close enough to the arena floor to see it.\r\n");
        return;
    } else if (!strcasecmp(arg, "look")) {
        if (!PRF_FLAGGED(ch, PRF_ARENAWATCH)) {
            send_to_char(ch, "You are not even watching anyone in the arena.\r\n");
            return;
        } else if (arena_watch(ch) != NOWHERE) {
            look_at_room(real_room(arena_watch(ch)), ch, 0);
        }
    } else if (!strcasecmp(arg, "scan")) {
        if (GET_ROOM_VNUM(IN_ROOM(ch)) == 17875) {
            int found = false;
            struct descriptor_data *d;

            send_to_char(ch, "@D---@CFighters in the arena@D---@n\r\n");
            for (d = descriptor_list; d; d = d->next) {
                if (STATE(d) != CON_PLAYING)
                    continue;

                if (IN_ARENA(d->character)) {
                    char buf[MAX_INPUT_LENGTH];
                    sprintf(buf, "@YFighter Number@D: @w%d, $N.@n", GET_IDNUM(d->character));
                    act(buf, true, ch, nullptr, d->character, TO_CHAR);
                    found = true;
                }
            }

            if (found == false) {
                send_to_char(ch, "@wNone.@n\r\n");
            }
        } else {
            send_to_char(ch, "You are not close enough to see what fighters are in the arena.\r\n");
            return;
        }
    } else {
        int num = atoi(arg);

        if (num < 0) {
            send_to_char(ch, "That is not a valid fighter number\r\n");
            return;
        } else {
            struct descriptor_data *d;
            int found = false;

            for (d = descriptor_list; d; d = d->next) {
                if (STATE(d) != CON_PLAYING)
                    continue;

                if (GET_IDNUM(d->character) == num) {
                    if (IN_ARENA(d->character)) {
                        found = true;
                    }
                }

            }

            if (found == true) {
                act("@wYou start watching the action surrounding that particular fighter in the arena.@n", true, ch,
                    nullptr, nullptr, TO_CHAR);
                act("@C$n@w starts watching the action in the arena.@n", true, ch, nullptr, nullptr, TO_ROOM);
                ch->pref.set(PRF_ARENAWATCH);
                ARENA_IDNUM(ch) = num;
            } else {
                send_to_char(ch, "A fighter with such a number was not found in the arena.\r\n");
                return;
            }
        } /* Secondary else end */
    } /* Main Else end */
} /* End of Arena Function */

ACMD(do_ensnare) {

    if (!know_skill(ch, SKILL_ENSNARE)) {
        return;
    }

    struct obj_data *weave, *obj = nullptr, *next_obj;
    int found = false;

    for (weave = ch->contents; weave; weave = next_obj) {
        next_obj = weave->next_content;
        if (found == false && valid_silk(weave) && !OBJ_FLAGGED(weave, ITEM_FORGED)) {
            found = true;
            obj = weave;
        }
    }

    if (found == false) {
        send_to_char(ch, "You do not have a bundle of silk to ensnare an opponent with!\r\n");
        return;
    } else {
        int prob = GET_SKILL(ch, SKILL_ENSNARE), perc = axion_dice(0);
        char arg[MAX_INPUT_LENGTH];
        struct char_data *vict;

        one_argument(argument, arg);

        if (!*arg) {
            send_to_char(ch, "Syntax: ensnare (target)\r\n");
            return;
        }

        if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
            send_to_char(ch, "Who are you trying to target with ensnare?\r\n");
            return;
        } else if (AFF_FLAGGED(vict, AFF_ENSNARED)) {
            send_to_char(ch, "They are already ensnared!\r\n");
            return;
        } else if (!HAS_ARMS(vict)) {
            send_to_char(ch, "They don't have arms to ensnare!\r\n");
            return;
        } else if (prob <= perc) {
            act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! Unfortunately you miss and lose the bundle...@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! Fortunately $e misses and loses the bundle...@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! Fortunately $e misses and loses the bundle...@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            extract_obj(obj);
            WAIT_STATE(ch, PULSE_3SEC);
            improve_skill(ch, SKILL_ENSNARE, 0);
        } else if (AFF_FLAGGED(vict, AFF_ZANZOKEN) && !AFF_FLAGGED(ch, AFF_ZANZOKEN)) {
            act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! Unfortunately @c$N@W zanzokens away avoiding it and you lose the bundle...@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! Fortunately you zanzoken away avoiding it and @C$n@W loses the bundle...@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! Fortunately @c$N@W zanzokens away avoiding it and @C$n@W loses the bundle...@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            extract_obj(obj);
            WAIT_STATE(ch, PULSE_3SEC);
            improve_skill(ch, SKILL_ENSNARE, 0);
            vict->affected_by.reset(AFF_ZANZOKEN);
        } else if (AFF_FLAGGED(vict, AFF_ZANZOKEN) && AFF_FLAGGED(ch, AFF_ZANZOKEN)) {
            if (GET_SPEEDI(ch) + rand_number(1, 100) < GET_SPEEDI(vict) + rand_number(1, 100)) {
                act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! You both zanzoken! Unfortunately @c$N@W manages to avoid it and you lose the bundle...@n",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! You both zanzoken! Fortunately you manage to avoid it and @C$n@W loses the bundle...@n",
                    true, ch, nullptr, vict, TO_VICT);
                act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! They both zanzoken! Fortunately @c$N@W manages to avoid it and @C$n@W loses the bundle...@n",
                    true, ch, nullptr, vict, TO_NOTVICT);
                extract_obj(obj);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_ENSNARE, 0);
                for(auto c : {vict, ch}) c->affected_by.reset(AFF_ZANZOKEN);
            } else {
                act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! Fortunately you manage to hit $M! You both zanzoken! Quickly you spin around $M and ensnare $S arms with the silk!@n",
                    true, ch, nullptr, vict, TO_CHAR);
                act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! Unfortunately $e manages to hit YOU! You both zanzoken! Quickly $e spins around you and ensnares your arms with the silk!@n",
                    true, ch, nullptr, vict, TO_VICT);
                act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! Unfortunately $e manages to hit $M! They both zanzoken! Quickly $e spins around @c$N@W and ensnares $S arms with the silk!@n",
                    true, ch, nullptr, vict, TO_NOTVICT);
                extract_obj(obj);
                vict->affected_by.set(AFF_ENSNARED);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_ENSNARE, 0);
                for(auto c : {vict, ch}) c->affected_by.reset(AFF_ZANZOKEN);
            }
        } else if (AFF_FLAGGED(ch, AFF_ZANZOKEN) && !AFF_FLAGGED(vict, AFF_ZANZOKEN)) {
            act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! Fortunately you manage to hit $M! Quickly you zanzoken and spin around $M and ensnare $S arms with the silk!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! Unfortunately $e manages to hit YOU! Quickly $e zanzokens and spins around you and ensnares your arms with the silk!@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! Unfortunately $e manages to hit $M! Quickly $e zanzokens and spins around @c$N@W and ensnares $S arms with the silk!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            extract_obj(obj);
            vict->affected_by.set(AFF_ENSNARED);
            WAIT_STATE(ch, PULSE_3SEC);
            improve_skill(ch, SKILL_ENSNARE, 0);
            ch->affected_by.reset(AFF_ZANZOKEN);
        } else if (GET_SPEEDI(ch) + rand_number(1, 100) < GET_SPEEDI(vict) + rand_number(1, 100)) {
            act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! Unfortunately @c$N@W manages to avoid it and you lose the bundle...@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! Fortunately you manage to avoid it and @C$n@W loses the bundle...@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! Fortunately @c$N@W manages to avoid it and @C$n@W loses the bundle...@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            extract_obj(obj);
            WAIT_STATE(ch, PULSE_3SEC);
            improve_skill(ch, SKILL_ENSNARE, 0);
        } else {
            act("@WYou unwind your bundle of silk and grab a loose end of it. Splitting that end to reveal the sticky innards of the strand you swing the strand at @c$N@W! Fortunately you manage to hit $M! Quickly you spin around $M and ensnare $S arms with the silk!@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at YOU! Unfortunately $e manages to hit YOU! Quickly $e spins around you and ensnares your arms with the silk!@n",
                true, ch, nullptr, vict, TO_VICT);
            act("@C$n@W unwinds a bundle of silk and grabs a loose end of it. Splitting that end to reveal the sticky innards of the strand $e swings the strand at @c$N@W! Unfortunately $e manages to hit $M! Quickly $e spins around @c$N@W and ensnares $S arms with the silk!@n",
                true, ch, nullptr, vict, TO_NOTVICT);
            extract_obj(obj);
            vict->affected_by.set(AFF_ENSNARED);
            WAIT_STATE(ch, PULSE_3SEC);
            improve_skill(ch, SKILL_ENSNARE, 0);
        }
    } /* Main else function */
}

/* This determines of an object is a suitable bundle of silk or not */
static int valid_silk(struct obj_data *obj) {
    int value = 0;

    switch (GET_OBJ_VNUM(obj)) {
        case 16700:
        case 16701:
        case 16702:
        case 16703:
        case 16704:
        case 16708:
            value = 1;
            break;
    }

    return (value);
}

ACMD(do_silk) {

    if (!know_skill(ch, SKILL_SILK)) {
        return;
    }

    struct obj_data *obj = nullptr, *weave = nullptr, *next_obj = nullptr, *weaved = nullptr;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);
    int prob = GET_SKILL(ch, SKILL_SILK), perc = rand_number(1, 120);

    if (!*arg) {
        send_to_char(ch, "Syntax: silk (weave | bundle)\r\n");
        return;
    }

    if (!strcasecmp(arg, "weave")) {

        if (!*arg2) {
            send_to_char(ch, "Syntax: silk weave (head | wrist | belt)\r\n");
            return;
        }

        int found = false, armor = 500, str = 0, intel = 0, olevel = 0;
        double price = 1;

        for (weave = ch->contents; weave; weave = next_obj) {
            next_obj = weave->next_content;
            if (found == false && valid_silk(weave) && !OBJ_FLAGGED(weave, ITEM_FORGED)) {
                found = true;
                obj = weave;
            }
        }

        if (found == false) {
            send_to_char(ch, "You do not have an acceptable bundle of silk in your inventory!\r\n");
            return;
        } else {
            if (!strcasecmp(arg2, "head")) {
                if (prob <= perc) {
                    act("@WYou attempt to weave $p@W into the desired piece but end up ruining the entire bundle instead.@n",
                        true, ch, obj, nullptr, TO_CHAR);
                    act("@C$n@W attempts to weave $p@W into some type of clothing but ends up ruining the entire bundle instead.@n",
                        true, ch, obj, nullptr, TO_ROOM);
                    extract_obj(obj);
                    WAIT_STATE(ch, PULSE_4SEC);
                    return;
                } else {
                    weaved = read_object(16705, VIRTUAL);
                    obj_to_room(weaved, IN_ROOM(ch));
                    if (GET_OBJ_VNUM(obj) == 16708) {
                        armor *= 20;
                        str = 4;
                        intel = 4;
                        price = 4;
                        olevel = 80;
                    } else if (GET_OBJ_VNUM(obj) == 16700) {
                        armor *= 15;
                        str = 3;
                        intel = 3;
                        price = 4;
                        olevel = 75;
                    } else if (GET_OBJ_VNUM(obj) == 16701) {
                        armor *= 10;
                        str = 3;
                        intel = 3;
                        price = 3;
                        olevel = 50;
                    } else if (GET_OBJ_VNUM(obj) == 16702) {
                        armor *= 6;
                        str = 2;
                        intel = 2;
                        price = 2;
                        olevel = 25;
                    } else if (GET_OBJ_VNUM(obj) == 16703) {
                        armor *= 4;
                        str = 1;
                        intel = 1;
                        price = 1.5;
                        olevel = 5;
                    }
                    weaved->affected[0].location = 17;
                    weaved->affected[0].modifier = armor;
                    GET_OBJ_COST(weaved) *= price;
                    GET_OBJ_VAL(weaved, 0) = olevel;
                    weaved->level = olevel;
                    if (str > 0) {
                        weaved->affected[1].location = 1;
                        weaved->affected[1].modifier = str;
                    }
                    if (intel > 0) {
                        weaved->affected[2].location = 3;
                        weaved->affected[2].modifier = intel;
                    }
                    act("@WYou attempt to weave the bundle and manage to create $p@W!@n", true, ch, weaved, nullptr,
                        TO_CHAR);
                    act("@C$n@W attempts to weave a bundle into something and manages to create $p@W!@n", true, ch,
                        weaved, nullptr, TO_ROOM);
                    do_get(ch, "headsash", 0, 0);
                    extract_obj(obj);
                    WAIT_STATE(ch, PULSE_4SEC);
                }
            } else if (!strcasecmp(arg2, "wrist")) {
                if (prob <= perc) {
                    act("@WYou attempt to weave $p@W into the desired piece but end up ruining the entire bundle instead.@n",
                        true, ch, obj, nullptr, TO_CHAR);
                    act("@C$n@W attempts to weave $p@W into some type of clothing but ends up ruining the entire bundle instead.@n",
                        true, ch, obj, nullptr, TO_ROOM);
                    extract_obj(obj);
                    WAIT_STATE(ch, PULSE_4SEC);
                    return;
                } else {
                    weaved = read_object(16706, VIRTUAL);
                    obj_to_room(weaved, IN_ROOM(ch));
                    if (GET_OBJ_VNUM(obj) == 16708) {
                        armor *= 20;
                        str = 4;
                        intel = 4;
                        price = 4;
                        olevel = 80;
                    } else if (GET_OBJ_VNUM(obj) == 16700) {
                        armor *= 15;
                        str = 3;
                        intel = 3;
                        price = 4;
                        olevel = 75;
                    } else if (GET_OBJ_VNUM(obj) == 16701) {
                        armor *= 10;
                        str = 3;
                        intel = 3;
                        price = 3;
                        olevel = 50;
                    } else if (GET_OBJ_VNUM(obj) == 16702) {
                        armor *= 6;
                        str = 2;
                        intel = 2;
                        price = 2;
                        olevel = 25;
                    } else if (GET_OBJ_VNUM(obj) == 16703) {
                        armor *= 4;
                        str = 1;
                        intel = 1;
                        price = 1.5;
                        olevel = 5;
                    }
                    weaved->affected[0].location = 17;
                    weaved->affected[0].modifier = armor;
                    GET_OBJ_COST(weaved) *= price;
                    GET_OBJ_VAL(weaved, 0) = olevel;
                    weaved->level = olevel;
                    if (str > 0) {
                        weaved->affected[1].location = 1;
                        weaved->affected[1].modifier = str;
                    }
                    if (intel > 0) {
                        weaved->affected[2].location = 3;
                        weaved->affected[2].modifier = intel;
                    }
                    act("@WYou attempt to weave the bundle and manage to create $p@W!@n", true, ch, weaved, nullptr,
                        TO_CHAR);
                    act("@C$n@W attempts to weave a bundle into something and manages to create $p@W!@n", true, ch,
                        weaved, nullptr, TO_ROOM);
                    do_get(ch, "wristband", 0, 0);
                    extract_obj(obj);
                    WAIT_STATE(ch, PULSE_4SEC);
                }
            } else if (!strcasecmp(arg2, "belt")) {
                if (prob <= perc) {
                    act("@WYou attempt to weave $p@W into the desired piece but end up ruining the entire bundle instead.@n",
                        true, ch, obj, nullptr, TO_CHAR);
                    act("@C$n@W attempts to weave $p@W into some type of clothing but ends up ruining the entire bundle instead.@n",
                        true, ch, obj, nullptr, TO_ROOM);
                    extract_obj(obj);
                    WAIT_STATE(ch, PULSE_4SEC);
                    return;
                } else {
                    weaved = read_object(16707, VIRTUAL);
                    obj_to_room(weaved, IN_ROOM(ch));
                    if (GET_OBJ_VNUM(obj) == 16708) {
                        armor *= 20;
                        str = 4;
                        intel = 4;
                        price = 4;
                        olevel = 80;
                    } else if (GET_OBJ_VNUM(obj) == 16700) {
                        armor *= 15;
                        str = 3;
                        intel = 3;
                        price = 4;
                        olevel = 75;
                    } else if (GET_OBJ_VNUM(obj) == 16701) {
                        armor *= 10;
                        str = 3;
                        intel = 3;
                        price = 3;
                        olevel = 50;
                    } else if (GET_OBJ_VNUM(obj) == 16702) {
                        armor *= 6;
                        str = 2;
                        intel = 2;
                        price = 2;
                        olevel = 25;
                    } else if (GET_OBJ_VNUM(obj) == 16703) {
                        armor *= 4;
                        str = 1;
                        intel = 1;
                        price = 1.5;
                        olevel = 5;
                    }
                    weaved->affected[0].location = 17;
                    weaved->affected[0].modifier = armor;
                    GET_OBJ_COST(weaved) *= price;
                    GET_OBJ_VAL(weaved, 0) = olevel;
                    weaved->level = olevel;
                    if (str > 0) {
                        weaved->affected[1].location = 1;
                        weaved->affected[1].modifier = str;
                    }
                    if (intel > 0) {
                        weaved->affected[2].location = 3;
                        weaved->affected[2].modifier = intel;
                    }
                    act("@WYou attempt to weave the bundle and manage to create $p@W!@n", true, ch, weaved, nullptr,
                        TO_CHAR);
                    act("@C$n@W attempts to weave a bundle into something and manages to create $p@W!@n", true, ch,
                        weaved, nullptr, TO_ROOM);
                    do_get(ch, "belt", 0, 0);
                    extract_obj(obj);
                    WAIT_STATE(ch, PULSE_4SEC);
                }
            } else {
                send_to_char(ch, "Syntax: silk weave (head | wrist | belt)");
                return;
            }
            return;
        } ////

    } else if (!strcasecmp(arg, "bundle")) {
        int64_t cost = ((GET_MAX_MANA(ch) * 0.01) * (prob * 0.20)) + (GET_INT(ch) * GET_LEVEL(ch));

        if ((ch->getCurKI()) < cost) {
            send_to_char(ch, "You do not have enough ki to weave any bundles of silk.\r\n");
            return;
        } else {
            WAIT_STATE(ch, PULSE_3SEC);
            int super = false, superoll = rand_number(1, 100);
            if (IS_KURZAK(ch)) {
                if (GET_SKILL(ch, SKILL_SILK) >= 100) {
                    if (8 > superoll) {
                        super = true;
                    }
                } else if (GET_SKILL(ch, SKILL_SILK) >= 60) {
                    if (6 > superoll) {
                        super = true;
                    }
                } else if (GET_SKILL(ch, SKILL_SILK) >= 40) {
                    if (3 > superoll) {
                        super = true;
                    }
                }
            }
            if (super == true) {
                obj = read_object(16708, VIRTUAL);
                obj_to_room(obj, IN_ROOM(ch));
                act("@YYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You gently weave the silk and in no time at all you have a $p@Y piled at your feet!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                send_to_char(ch, "@YIt's SUPER grand!@n\r\n");
                act("@C$n@W seems to concentrate for a moment before spitting out a golden colored silk from $s mouth. Gently $e weaves the silk and in no time at all $e has a $p@W piled at $s feet!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
            } else if (prob > perc && prob >= 100) { /* Second Best Quality */
                obj = read_object(16700, VIRTUAL);
                obj_to_room(obj, IN_ROOM(ch));
                act("@WYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You gently weave the silk and in no time at all you have a $p@W piled at your feet!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W seems to concentrate for a moment before spitting out a golden colored silk from $s mouth. Gently $e weaves the silk and in no time at all $e has a $p@W piled at $s feet!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
            } else if (prob > perc && prob >= 90) { /* Great Quality */
                obj = read_object(16701, VIRTUAL);
                obj_to_room(obj, IN_ROOM(ch));
                act("@WYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You gently weave the silk and in no time at all you have a $p@W piled at your feet!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W seems to concentrate for a moment before spitting out a golden colored silk from $s mouth. Gently $e weaves the silk and in no time at all $e has a $p@W piled at $s feet!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
            } else if (prob > perc && prob >= 80) { /* Good Quality */
                obj = read_object(16702, VIRTUAL);
                obj_to_room(obj, IN_ROOM(ch));
                act("@WYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You gently weave the silk and in no time at all you have a $p@W piled at your feet!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W seems to concentrate for a moment before spitting out a golden colored silk from $s mouth. Gently $e weaves the silk and in no time at all $e has a $p@W piled at $s feet!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
            } else if (prob > perc && prob >= 50) { /* Decent Quality */
                obj = read_object(16703, VIRTUAL);
                obj_to_room(obj, IN_ROOM(ch));
                act("@WYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You gently weave the silk and in no time at all you have a $p@W piled at your feet!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W seems to concentrate for a moment before spitting out a golden colored silk from $s mouth. Gently $e weaves the silk and in no time at all $e has a $p@W piled at $s feet!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
            } else if (prob > perc) { /* Bad Quality */
                obj = read_object(16704, VIRTUAL);
                obj_to_room(obj, IN_ROOM(ch));
                act("@WYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You gently weave the silk and in no time at all you have a $p@W piled at your feet!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W seems to concentrate for a moment before spitting out a golden colored silk from $s mouth. Gently $e weaves the silk and in no time at all $e has a $p@W piled at $s feet!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
            } else {
                act("@WYou concentrate your ki into your silk sacs and begin to spit silk out of your mouth. You end up making a poorly formed puddle of goo...@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@W seems to concentrate for a moment before spitting out a poorly formed puddle of goo...@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurKI(cost);
                improve_skill(ch, SKILL_SILK, 1);
            }
        }
    } else {
        send_to_char(ch, "Syntax: silk (weave | bundle)\r\n");
        return;
    }

}

/* Let's an Arlian trade Stamina for either PL or Ki*/
ACMD(do_adrenaline) {

    if (!IS_ARLIAN(ch) && (!IS_BIO(ch) || (IS_BIO(ch) && (GET_GENOME(ch, 0) != 6 && GET_GENOME(ch, 1) != 6)))) {
        send_to_char(ch, "You are not an arlian and do not possess this ability\r\n");
        return;
    } else {
        char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
        two_arguments(argument, arg, arg2);

        if (!*arg || !*arg2) {
            send_to_char(ch, "Syntax: adrenaline (pl or ki) (percent)\r\nExample: adrenaline pl 10\r\n");
            return;
        } else {
            if (atoi(arg2) < 0 || atoi(arg2) > 100) {
                send_to_char(ch, "The percent must be between 1 and 100%s.\r\n", "%");
                return;
            }

            double percent = atoi(arg2) * 0.01;

            if ((ch->getCurST() - (ch->getBasePL() * percent)) < 0) {
                send_to_char(ch, "You do not have enough stamina to trade for adrenaline!\r\n");
                return;
            }

            int64_t trade = ch->getBaseST() * percent;

            if (!strcasecmp(arg, "pl")) {
                act("@GYou focus your mind and begin to overwork your powerful adrenal glands and your wounds begin to heal!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@g$n@G seems to concentrate and $s wounds begin to heal!@n", true, ch, nullptr, nullptr, TO_ROOM);

                if (GET_HIT(ch) + trade > (ch->getEffMaxPL()))
                    send_to_char(ch, "Some of your stamina was wasted because your powerlevel maxed out.\r\n");
                ch->incCurHealth(trade);
                ch->decCurST(trade);

            } else if (!strcasecmp(arg, "ki")) {
                act("@GYou focus your mind and begin to overwork your powerful adrenal glands and you feel your ki replenish!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@g$n@G seems to concentrate and $e appears energized!@n", true, ch, nullptr, nullptr, TO_ROOM);

                if ((ch->getCurKI()) + trade > GET_MAX_MANA(ch))
                    send_to_char(ch, "Some of your stamina was wasted because your ki maxed out.\r\n");
                ch->incCurKI(trade);
                ch->decCurST(trade);
            }
        } /* End inner else */

    } /* end main else */
}

/* This handles displaying the rpp item store to a player. */
void disp_rpp_store(struct char_data *ch) {

    send_to_char(ch, "@m                        RPP Item Store@n\n");
    send_to_char(ch, "@D~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~@n\n");
    send_to_char(ch, "@GItem Name                      @gRPP Cost        @cChoice Number   @yMin Lvl@n\n");
    send_to_char(ch, "@WStardust Equipment Set         @D[@Y20@D]            @D[ @C1@D]            @w50@n\n");
    send_to_char(ch, "@WPlatinum Masamune (Sword Skill)@D[@Y 5@D]            @D[ @C2@D]            @w40@n\n");
    send_to_char(ch, "@WObsidian Dirk (Dagger Skill)   @D[@Y 5@D]            @D[ @C3@D]            @w40@n\n");
    send_to_char(ch, "@WEmerald Javelin (Spear Skill)  @D[@Y 5@D]            @D[ @C4@D]            @w40@n\n");
    send_to_char(ch, "@WIvory Cane (Club Skill)        @D[@Y 5@D]            @D[ @C5@D]            @w40@n\n");
    send_to_char(ch, "@WHyper X65 Cannon (Gun Skill)   @D[@Y 5@D]            @D[ @C6@D]            @w40@n\n");
    send_to_char(ch, "@WJagged Rock (Brawl skill)      @D[@Y 5@D]            @D[ @C7@D]            @w40@n\n");
    send_to_char(ch, "@WKachin Mountain                @D[@Y 8@D]            @D[ @C8@D]@n\n");
    send_to_char(ch, "@WSpar Booster                   @D[@Y15@D]            @D[ @C9@D]@n\n");
    send_to_char(ch, "@D~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~@n\n");

    send_to_char(ch, "@wSyntax: rpp 12 (choice number)@n\r\n");
}

/* This handles buying an item from the rpp item store. */
void handle_rpp_store(struct char_data *ch, int choice) {
    struct obj_data *obj;
    int objnum = 0, cost = 0;

    switch (choice) { /* Find the cost of their selection. */
        case 1:
            cost = 20;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            cost = 5;
            break;
        case 8:
            cost = 8;
            break;
        case 9:
            cost = 15;
            break;
        default:
            send_to_char(ch, "That is not a selection option!\r\n");
            return;
    }


    if (GET_RP(ch) < cost) { /* They can't afford it. */
        send_to_char(ch, "You do not have enough RPP to afford that option.\r\n");
        return;
    } else { /* They can afford it. */
        switch (choice) {
            case 1:
                if (!ch->canCarryWeight(26)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 13 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else if (GET_LEVEL(ch) < 50) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    for (objnum = 1110; objnum < 1120; objnum++) {
                        if (objnum <= 1116) {
                            obj = read_object(objnum, VIRTUAL);
                            obj_to_char(obj, ch);
                            GET_OBJ_SIZE(obj) = get_size(ch);
                            obj = nullptr;
                        } else {
                            obj = read_object(objnum, VIRTUAL);
                            obj_to_char(obj, ch);
                            GET_OBJ_SIZE(obj) = get_size(ch);
                            obj = nullptr;
                            obj = read_object(objnum, VIRTUAL);
                            obj_to_char(obj, ch);
                            GET_OBJ_SIZE(obj) = get_size(ch);
                        }
                    }
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 2:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else if (GET_LEVEL(ch) < 40) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    obj = read_object(1120, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP from your Bank paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 3:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else if (GET_LEVEL(ch) < 40) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    obj = read_object(1121, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 4:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else if (GET_LEVEL(ch) < 40) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    obj = read_object(1122, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 5:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "@R%d@W RPP from your Bank paid for your selection. Enjoy!@n\r\n", cost);
                } else if (GET_LEVEL(ch) < 40) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    obj = read_object(1123, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 6:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else if (GET_LEVEL(ch) < 40) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    obj = read_object(1124, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 7:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else if (GET_LEVEL(ch) < 40) {
                    send_to_char(ch, "You are below the minimum level to equip it.\r\n");
                } else {
                    obj = read_object(1125, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
            case 8: {
                auto &o = obj_proto[1126];
                if (!ch->canCarryWeight(o.weight)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else {
                    obj = read_object(1126, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
            }
                break;
            case 9:
                if (!ch->canCarryWeight(2)) {
                    send_to_char(ch, "You can not carry that much weight at this moment.\r\n");
                } else if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
                    send_to_char(ch, "You have too many items on you to carry anymore at this moment.\r\n");
                } else {
                    obj = read_object(1127, VIRTUAL);
                    obj_to_char(obj, ch);
                    GET_OBJ_SIZE(obj) = get_size(ch);
                    ch->modRPP(-cost);
                    ch->save();
                    send_to_char(ch, "@R%d@W RPP paid for your selection. Enjoy!@n\r\n", cost);
                    send_to_imm("RPP Purchase: %s %d", GET_NAME(ch), cost);
                }
                break;
        } /* End switch */
    }
}

static int valid_recipe(struct char_data *ch, int recipe, int type) {
    /* Plant Variables */
    int tomato = -1, cucumber = -1, onion = -1, greenbean = -1, garlic = -1, redpep = -1;
    int potato = -1, carrot = -1, brownmush = -1, lettuce = -1;
    /* Meat Variables  */
    int normmeat = -1, goodmeat = -1, normfish = -1, goodfish = -1, greatfish = -1, bestfish = -1;
    /* Store */
    int rice = -1, flour = -1, appleplum = -1, fberry = -1, carambola = -1;

    struct obj_data *obj2, *next_obj;
    int pass = false;

    /* Determine ingredients needed for recipe */
    switch (recipe) {
        case RECIPE_TOMATO_SOUP:
            tomato = 2;
            break;
        case RECIPE_STEAK:
            normmeat = 1;
            break;
        case RECIPE_POTATO_SOUP:
            potato = 2;
            break;
        case RECIPE_VEGETABLE_SOUP:
            potato = 1;
            tomato = 1;
            carrot = 1;
            greenbean = 1;
            onion = 1;
            break;
        case RECIPE_MEAT_STEW:
            normmeat = 1;
            potato = 1;
            tomato = 1;
            garlic = 1;
            break;
        case RECIPE_ROAST:
            normmeat = 1;
            potato = 2;
            garlic = 1;
            onion = 1;
            greenbean = 3;
            break;
        case RECIPE_CHILI_SOUP:
            normmeat = 1;
            redpep = 4;
            tomato = 2;
            break;
        case RECIPE_GRILLED_NORMFISH:
            normfish = 1;
            break;
        case RECIPE_GRILLED_GOODFISH:
            goodfish = 1;
            break;
        case RECIPE_GRILLED_GREATFISH:
            greatfish = 1;
            break;
        case RECIPE_GRILLED_BESTFISH:
            bestfish = 1;
            break;
        case RECIPE_COOKED_RICE:
            rice = 1;
            break;
        case RECIPE_SUSHI:
            rice = 1;
            normfish = 1;
            break;
        case RECIPE_BREAD:
            flour = 1;
            break;
        case RECIPE_SALAD:
            tomato = 1;
            cucumber = 1;
            carrot = 1;
            lettuce = 1;
            break;
        case RECIPE_APPLEPLUM:
            flour = 1;
            appleplum = 1;
            break;
        case RECIPE_FBERRY_MUFFIN:
            flour = 1;
            fberry = 1;
            break;
        case RECIPE_CARAMBOLA_BREAD:
            flour = 1;
            carambola = 1;
            break;
    }

    if (type == 0) {
        /* Check for ingredients in inventory */
        for (obj2 = ch->contents; obj2; obj2 = next_obj) {
            next_obj = obj2->next_content;
            switch (GET_OBJ_VNUM(obj2)) {
                case RCP_TOMATO:
                    if (tomato > 0) {
                        tomato -= 1;
                    }
                    break;
                case RCP_NORMAL_MEAT:
                    if (normmeat > 0) {
                        normmeat -= 1;
                    }
                    break;
                case RCP_POTATO:
                    if (potato > 0) {
                        potato -= 1;
                    }
                    break;
                case RCP_ONION:
                    if (onion > 0) {
                        onion -= 1;
                    }
                    break;
                case RCP_CUCUMBER:
                    if (cucumber > 0) {
                        cucumber -= 1;
                    }
                    break;
                case RCP_CHILIPEPPER:
                    if (redpep > 0) {
                        redpep -= 1;
                    }
                    break;
                case RCP_CARROT:
                    if (carrot > 0) {
                        carrot -= 1;
                    }
                    break;
                case RCP_GREENBEAN:
                    if (greenbean > 0) {
                        greenbean -= 1;
                    }
                    break;
                case RCP_BLACKBASS:
                case RCP_FLOUNDER:
                case RCP_NARRI:
                case RCP_GRAVELREBOI:
                    if (normfish > 0) {
                        normfish -= 1;
                    }
                    break;
                case RCP_SILVERTROUT:
                case RCP_SILVEREEL:
                case RCP_VALBISH:
                case RCP_VOOSPIKE:
                    if (goodfish > 0) {
                        goodfish -= 1;
                    }
                    break;
                case RCP_STRIPEDBASS:
                case RCP_COBIA:
                case RCP_GUSBLAT:
                case RCP_SHADOWFISH:
                    if (greatfish > 0) {
                        greatfish -= 1;
                    }
                    break;
                case RCP_BLUECATFISH:
                case RCP_TAMBOR:
                case RCP_REPEEIL:
                case RCP_SHADEEEL:
                    if (bestfish > 0) {
                        bestfish -= 1;
                    }
                    break;
                case RCP_BROWNMUSH:
                    if (brownmush > 0) {
                        brownmush -= 1;
                    }
                    break;
                case RCP_GARLIC:
                    if (garlic > 0) {
                        garlic -= 1;
                    }
                    break;
                case RCP_RICE:
                    if (rice > 0) {
                        rice -= 1;
                    }
                    break;
                case RCP_FLOUR:
                    if (flour > 0) {
                        flour -= 1;
                    }
                    break;
                case RCP_LETTUCE:
                    if (lettuce > 0) {
                        lettuce -= 1;
                    }
                    break;
                case RCP_APPLEPLUM:
                    if (appleplum > 0) {
                        appleplum -= 1;
                    }
                    break;
                case RCP_FROZENBERRY:
                    if (fberry > 0) {
                        fberry -= 1;
                    }
                    break;
                case RCP_CARAMBOLA:
                    if (carambola > 0) {
                        carambola -= 1;
                    }
                    break;
            }
        }
    } else { /* We know the ingredients are there, remove and exit. */
        for (obj2 = ch->contents; obj2; obj2 = next_obj) {
            next_obj = obj2->next_content;
            switch (GET_OBJ_VNUM(obj2)) {
                case RCP_TOMATO:
                    if (tomato > 0) {
                        tomato -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_NORMAL_MEAT:
                    if (normmeat > 0) {
                        normmeat -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_POTATO:
                    if (potato > 0) {
                        potato -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_ONION:
                    if (onion > 0) {
                        onion -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_CUCUMBER:
                    if (cucumber > 0) {
                        cucumber -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_CHILIPEPPER:
                    if (redpep > 0) {
                        redpep -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_CARROT:
                    if (carrot > 0) {
                        carrot -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_GREENBEAN:
                    if (greenbean > 0) {
                        greenbean -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_BLACKBASS:
                case RCP_FLOUNDER:
                case RCP_NARRI:
                case RCP_GRAVELREBOI:
                    if (normfish > 0) {
                        normfish -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_SILVERTROUT:
                case RCP_SILVEREEL:
                case RCP_VALBISH:
                case RCP_VOOSPIKE:
                    if (goodfish > 0) {
                        goodfish -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_STRIPEDBASS:
                case RCP_COBIA:
                case RCP_GUSBLAT:
                case RCP_SHADOWFISH:
                    if (greatfish > 0) {
                        greatfish -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_BLUECATFISH:
                case RCP_TAMBOR:
                case RCP_REPEEIL:
                case RCP_SHADEEEL:
                    if (bestfish > 0) {
                        bestfish -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_BROWNMUSH:
                    if (brownmush > 0) {
                        brownmush -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_GARLIC:
                    if (garlic > 0) {
                        garlic -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_RICE:
                    if (rice > 0) {
                        rice -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_FLOUR:
                    if (flour > 0) {
                        flour -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_LETTUCE:
                    if (lettuce > 0) {
                        lettuce -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_APPLEPLUM:
                    if (appleplum > 0) {
                        appleplum -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_FROZENBERRY:
                    if (fberry > 0) {
                        fberry -= 1;
                        extract_obj(obj2);
                    }
                    break;
                case RCP_CARAMBOLA:
                    if (carambola > 0) {
                        carambola -= 1;
                        extract_obj(obj2);
                    }
                    break;
            }
        }
        return (true);
        /* We'll exit here after removing the ingredients, for safety */
    }

    /* Make sure all ingredients were accounted for and pass if so */
    switch (recipe) {
        case RECIPE_TOMATO_SOUP:
            if (tomato == 0) {
                pass = true;
            }
            break;
        case RECIPE_STEAK:
            if (normmeat == 0) {
                pass = true;
            }
            break;
        case RECIPE_POTATO_SOUP:
            if (potato == 0) {
                pass = true;
            }
            break;
        case RECIPE_VEGETABLE_SOUP:
            if (potato == 0 && tomato == 0 && carrot == 0 && greenbean == 0 && onion == 0) {
                pass = true;
            }
            break;
        case RECIPE_MEAT_STEW:
            if (normmeat == 0 && potato == 0 && tomato == 0 && garlic == 0)
                pass = true;
            break;
        case RECIPE_ROAST:
            if (normmeat == 0 && potato == 0 && garlic == 0 && onion == 0 && greenbean == 0)
                pass = true;
            break;
        case RECIPE_CHILI_SOUP:
            if (normmeat == 0 && redpep == 0 && tomato == 0)
                pass = true;
            break;
        case RECIPE_GRILLED_NORMFISH:
            if (normfish == 0)
                pass = true;
            break;
        case RECIPE_GRILLED_GOODFISH:
            if (goodfish == 0)
                pass = true;
            break;
        case RECIPE_GRILLED_GREATFISH:
            if (greatfish == 0)
                pass = true;
            break;
        case RECIPE_GRILLED_BESTFISH:
            if (bestfish == 0)
                pass = true;
            break;
        case RECIPE_COOKED_RICE:
            if (rice == 0)
                pass = true;
            break;
        case RECIPE_SUSHI:
            if (rice == 0 && normfish == 0)
                pass = true;
            break;
        case RECIPE_BREAD:
            if (flour == 0)
                pass = true;
            break;
        case RECIPE_SALAD:
            if (tomato == 0 && cucumber == 0 && carrot == 0 && lettuce == 0)
                pass = true;
            break;
        case RECIPE_APPLEPLUM:
            if (flour == 0 && appleplum == 0)
                pass = true;
            break;
        case RECIPE_FBERRY_MUFFIN:
            if (flour == 0 && fberry == 0)
                pass = true;
            break;
        case RECIPE_CARAMBOLA_BREAD:
            if (flour == 0 && carambola == 0)
                pass = true;
            break;
    }

    if (pass == false) {
        if (tomato > 0) {
            send_to_char(ch, "@WYou need @m%d@W tomato%s for this recipe.@n\r\n", tomato, tomato > 1 ? "es" : "");
        }
        if (potato > 0) {
            send_to_char(ch, "@WYou need @m%d@W potato%s for this recipe.@n\r\n", potato, potato > 1 ? "es" : "");
        }
        if (onion > 0) {
            send_to_char(ch, "@WYou need @m%d@W onion%s for this recipe.@n\r\n", onion, onion > 1 ? "s" : "");
        }
        if (appleplum > 0) {
            send_to_char(ch, "@WYou need @m%d@W appleplum%s for this recipe.@n\r\n", appleplum,
                         appleplum > 1 ? "s" : "");
        }
        if (fberry > 0) {
            send_to_char(ch, "@WYou need @m%d@W frozen berry%s for this recipe.@n\r\n", fberry, fberry > 1 ? "s" : "");
        }
        if (carambola > 0) {
            send_to_char(ch, "@WYou need @m%d@W carambola%s for this recipe.@n\r\n", carambola,
                         carambola > 1 ? "s" : "");
        }
        if (lettuce > 0) {
            send_to_char(ch, "@WYou need @m%d@W head%s of lettuce for this recipe.@n\r\n", lettuce,
                         lettuce > 1 ? "s" : "");
        }
        if (flour > 0) {
            send_to_char(ch, "@WYou need @m%d@W cup%s of white flour for this recipe.@n\r\n", flour,
                         flour > 1 ? "s" : "");
        }
        if (rice > 0) {
            send_to_char(ch, "@WYou need @m%d@W cup%s of white rice for this recipe.@n\r\n", rice, rice > 1 ? "s" : "");
        }
        if (garlic > 0) {
            send_to_char(ch, "@WYou need @m%d@W garlic clove%s for this recipe.@n\r\n", garlic, garlic > 1 ? "s" : "");
        }
        if (carrot > 0) {
            send_to_char(ch, "@WYou need @m%d@W carrot%s for this recipe.@n\r\n", carrot, carrot > 1 ? "s" : "");
        }
        if (cucumber > 0) {
            send_to_char(ch, "@WYou need @m%d@W cucumber%s for this recipe.@n\r\n", cucumber, cucumber > 1 ? "s" : "");
        }
        if (greenbean > 0) {
            send_to_char(ch, "@WYou need @m%d@W green bean%s for this recipe.@n\r\n", greenbean,
                         greenbean > 1 ? "s" : "");
        }
        if (normmeat > 0) {
            send_to_char(ch, "@WYou need @m%d@W normal raw steak%s for this recipe.@n\r\n", normmeat,
                         normmeat > 1 ? "s" : "");
        }
        if (goodmeat > 0) {
            send_to_char(ch, "@WYou need @m%d@W good raw steak%s for this recipe.@n\r\n", goodmeat,
                         goodmeat > 1 ? "s" : "");
        }
        if (redpep > 0) {
            send_to_char(ch, "@WYou need @m%d@W chili pepper%s for this recipe.@n\r\n", redpep, redpep > 1 ? "s" : "");
        }
        if (normfish > 0) {
            send_to_char(ch, "@WYou need @m%d@W black bass, flounder, narri, or gravel reboi for this recipe.@n\r\n",
                         normfish);
        }
        if (goodfish > 0) {
            send_to_char(ch, "@WYou need @m%d@W silver trout, silver eel, valbish, or voos pike for this recipe.@n\r\n",
                         goodfish);
        }
        if (greatfish > 0) {
            send_to_char(ch, "@WYou need @m%d@W striped bass, cobia, gusblat, or shadowfish for this recipe.@n\r\n",
                         greatfish);
        }
        if (bestfish > 0) {
            send_to_char(ch, "@WYou need @m%d@W blue catfish, tambor, repeeil, or shadeeel for this recipe.@n\r\n",
                         bestfish);
        }
        if (brownmush > 0) {
            send_to_char(ch, "@WYou need @m%d@W brown mushroom%s for this recipe.@n\r\n", brownmush,
                         brownmush > 1 ? "s" : "");
        }
        return (false);
    } else {
        return (true);
    }
}

static int campfire_cook(int recipe) {

    switch (recipe) {
        case RECIPE_STEAK:
        case RECIPE_GRILLED_NORMFISH:
        case RECIPE_GRILLED_GOODFISH:
        case RECIPE_GRILLED_GREATFISH:
        case RECIPE_GRILLED_BESTFISH:
        case RECIPE_ROAST:
            return (true);
            break;
    }

    return (false);
}

ACMD(do_cook) {
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (IS_NPC(ch))
        return;

    if (!cook_element(IN_ROOM(ch))) {
        send_to_char(ch, "You need a campfire or Flambus Stove nearby to cook.\r\n");
        return;
    }

    if (!GET_SKILL(ch, SKILL_COOKING)) {
        send_to_char(ch, "You don't even know the basics!\r\n");
        return;
    }

    int skill = GET_SKILL(ch, SKILL_COOKING), prob = axion_dice(0);

    if (!*arg) {
        send_to_char(ch, "@D---------------------@RCooking@D---------------------@n\r\n");
        send_to_char(ch, "@Y 1@B) @CCooked Steak		@Y17@B) @CCarambola Bread@n\r\n");
        send_to_char(ch, "@Y 2@B) @CTomato Soup		@n\r\n");
        send_to_char(ch, "@Y 3@B) @CPotato Soup		@n\r\n");
        send_to_char(ch, "@Y 4@B) @CVegetable Soup		@n\r\n");
        send_to_char(ch, "@Y 5@B) @CMeat Stew			@n\r\n");
        send_to_char(ch, "@Y 6@B) @CChili Soup		@n\r\n");
        send_to_char(ch, "@Y 7@B) @CGrilled Fish		@n\r\n");
        send_to_char(ch, "@Y 8@B) @CGood Grilled Fish		@n\r\n");
        send_to_char(ch, "@Y 9@B) @CGreat Grilled Fish	@n\r\n");
        send_to_char(ch, "@Y10@B) @CMagnificent Grilled Fish	@n\r\n");
        send_to_char(ch, "@Y11@B) @CCooked White Rice		@n\r\n");
        send_to_char(ch, "@Y12@B) @CSushi			@n\r\n");
        send_to_char(ch, "@Y13@B) @CWhite Bread		@n\r\n");
        send_to_char(ch, "@Y14@B) @CBasic Salad		@n\r\n");
        send_to_char(ch, "@Y15@B) @CAppleplum Chasan		@n\r\n");
        send_to_char(ch, "@Y16@B) @CFrozen Berry Muffin	@n\r\n");
        send_to_char(ch, "@wSyntax: cook (recipe number)@n\r\n");
        return;
    } else {
        int num = atoi(arg), pass = false;
        struct obj_data *meal = nullptr;

        int recipe = -1;
        switch (num) {
            case 1:
                recipe = RECIPE_STEAK;
                prob += 8;
                break;
            case 2:
                recipe = RECIPE_TOMATO_SOUP;
                break;
            case 3:
                recipe = RECIPE_POTATO_SOUP;
                break;
            case 4:
                recipe = RECIPE_VEGETABLE_SOUP;
                break;
            case 5:
                recipe = RECIPE_MEAT_STEW;
                break;
            case 6:
                recipe = RECIPE_CHILI_SOUP;
                break;
            case 7:
                recipe = RECIPE_GRILLED_NORMFISH;
                prob += 6;
                break;
            case 8:
                recipe = RECIPE_GRILLED_GOODFISH;
                prob += 10;
                break;
            case 9:
                recipe = RECIPE_GRILLED_GREATFISH;
                prob += 12;
                break;
            case 10:
                recipe = RECIPE_GRILLED_BESTFISH;
                prob += 16;
                break;
            case 11:
                recipe = RECIPE_COOKED_RICE;
                break;
            case 12:
                recipe = RECIPE_SUSHI;
                break;
            case 13:
                recipe = RECIPE_BREAD;
                break;
            case 14:
                recipe = RECIPE_SALAD;
                break;
            case 15:
                recipe = RECIPE_APPLEPLUM;
                break;
            case 16:
                recipe = RECIPE_FBERRY_MUFFIN;
                break;
            case 17:
                recipe = RECIPE_CARAMBOLA_BREAD;
                break;
        }

        if (recipe == -1) {
            send_to_char(ch, "That is not a valid dish!\r\n");
            return;
        }

        if (!valid_recipe(ch, recipe, 0)) {
            return;
        } else if (cook_element(IN_ROOM(ch)) == 1 && !campfire_cook(recipe)) {
            send_to_char(ch, "You can not cook that dish over a campfire.\r\n");
            return;
        } else {
            valid_recipe(ch, recipe, 1);
            pass = true;
        }
        if (pass == true) {
            //Calculate failure chance, above level 80 there is no chance to fail
            if (80 + (skill / 2) < prob) {
                act("@wYou screw up the preparation of the recipe and end up wasting the ingredients!@n", true, ch,
                    nullptr, nullptr, TO_CHAR);
                act("@C$n@w starts to prepare some food, but ends up ruining the ingredients instead!@n", true, ch,
                    nullptr, nullptr, TO_ROOM);
                improve_skill(ch, SKILL_COOKING, 0);
                WAIT_STATE(ch, PULSE_2SEC);
                return;
            } /* End failed to cook it right */

            int psbonus = 0, expbonus = 0;

            switch (num) {
                case 1:
                    meal = read_object(MEAL_STEAK, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 1;
                    expbonus = 5;
                    break;
                case 2:
                    meal = read_object(MEAL_TOMATO_SOUP, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 2;
                    expbonus = 15;
                    break;
                case 3:
                    meal = read_object(MEAL_POTATO_SOUP, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 1;
                    expbonus = 20;
                    break;
                case 4:
                    meal = read_object(MEAL_VEGETABLE_SOUP, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 3;
                    expbonus = 45;
                    break;
                case 5:
                    meal = read_object(MEAL_MEAT_STEW, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 2;
                    expbonus = 50;
                    break;
                case 6:
                    meal = read_object(MEAL_CHILI_SOUP, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 0;
                    expbonus = 100;
                    break;
                case 7:
                    meal = read_object(MEAL_NORM_FISH, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 2;
                    expbonus = 12;
                    break;
                case 8:
                    meal = read_object(MEAL_GOOD_FISH, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 3;
                    expbonus = 40;
                    break;
                case 9:
                    meal = read_object(MEAL_GREAT_FISH, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 5;
                    expbonus = 80;
                    break;
                case 10:
                    meal = read_object(MEAL_BEST_FISH, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 7;
                    expbonus = 125;
                    break;
                case 11:
                    meal = read_object(MEAL_COOKED_RICE, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 1;
                    expbonus = 8;
                    break;
                case 12:
                    meal = read_object(MEAL_SUSHI, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 2;
                    expbonus = 20;
                    break;
                case 13:
                    meal = read_object(MEAL_BREAD, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 1;
                    expbonus = 8;
                    break;
                case 14:
                    meal = read_object(MEAL_SALAD, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 5;
                    expbonus = 8;
                    break;
                case 15:
                    meal = read_object(MEAL_APPLEPLUM, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 1;
                    expbonus = 9;
                    break;
                case 16:
                    meal = read_object(MEAL_FBERRY_MUFFIN, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 3;
                    expbonus = 12;
                    break;
                case 17:
                    meal = read_object(MEAL_CARAMBOLA_BREAD, VIRTUAL);
                    obj_to_char(meal, ch);
                    psbonus = 1;
                    expbonus = 9;
                    break;
                default:
                    send_to_char(ch, "That is not a valid dish!\r\n");
                    return;
            }
            if (GET_BONUS(ch, BONUS_RECIPE)) {
                psbonus += 1;
                expbonus += 3;
            }
            act("@wYou carefully prepare the ingredients and then start cooking them. After a while of patience  and skillful care you successfully make @D'@C$p@D'@w!@n",
                true, ch, meal, nullptr, TO_CHAR);
            act("@C$n@w carefully prepares some ingredients and starts cooking them. After a while of patience and skillful care $e succeeds in making @D'@C$p@D'@w!@n",
                true, ch, meal, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_COOKING, 0);
            //Calculate psbonus and expbonus to add to the food, skill level is significantly important in both
            float masterBonus = (skill == 100) ? 1.2 : 1;
            if (psbonus > 0) {
                if (skill * 0.10 > 0)
                    psbonus = (skill * 0.10) * psbonus * masterBonus;
            }
            if (expbonus > 0) {
                expbonus = skill * expbonus * masterBonus;
            }

            GET_OBJ_VAL(meal, 1) = psbonus;
            GET_OBJ_VAL(meal, 2) = expbonus;

            WAIT_STATE(ch, PULSE_2SEC);
        } /* End has ingredients */
    }
}

/* This allows a player to cover their body in a shield of fire. */
ACMD(do_fireshield) {

    if (!know_skill(ch, SKILL_FIRESHIELD)) {
        return;
    }

    if (AFF_FLAGGED(ch, AFF_FIRESHIELD)) {
        send_to_char(ch, "You are already covered in a fireshield!\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
        send_to_char(ch, "You are covered in a barrier!\r\n");
        return;
    }

    if (SUNKEN(IN_ROOM(ch))) {
        send_to_char(ch, "There is way too much water here!\r\n");
        return;
    }

    int64_t cost = GET_MAX_MANA(ch) * 0.03;

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki!\r\n");
        return;
    }

    int skill = init_skill(ch, SKILL_FIRESHIELD), prob = axion_dice(0);

    if (skill <= prob) {
        act("@WYou hold your hands up in front of you on either side and try to summon defensive @rf@Rl@Ya@rm@Re@Ys@W to cover your body. Yet you screw up and the technique fails!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@W holds $s hands up in front of $m on either side and tries to summon defensive @rf@Rl@Ya@rm@Re@Ys@W to cover $s body. Yet $e seems to screw up and the technique fails!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        improve_skill(ch, SKILL_FIRESHIELD, 0);
        ch->decCurKI(cost);
        return;
    } else {
        act("@WYou hold your hands up in front of you on either side and try to summon defensive @rf@Rl@Ya@rm@Re@ys@W to cover your body. The ki you have gathered pours out of your body and creates intense black @rf@Rl@Ya@rm@Re@Ys@W that cover your entire body in a protective layer!",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@W holds $s hands up in front of $m on either side and tries to summon defensive @rf@Rl@Ya@rm@Re@ys@W to cover $s body. The ki $e has gathered pours out of $s body and creates intense black @rf@Rl@Ya@rm@Re@Ys@W that cover $s entire body in a protective layer!",
            true, ch, nullptr, nullptr, TO_ROOM);
        improve_skill(ch, SKILL_FIRESHIELD, 0);
        ch->decCurKI(cost);
        ch->affected_by.set(AFF_FIRESHIELD);
        return;
    }

}

/* This allows a player to warp from one ocean/sea to another. */
ACMD(do_warppool) {

    if (IS_NPC(ch))
        return;

    if (!know_skill(ch, SKILL_WARP)) {
        return;
    }

    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return;
    }

    if (ABSORBING(ch) || ABSORBBY(ch)) {
        send_to_char(ch, "You are struggling with someone!\r\n");
        return;
    }

    if (SITS(ch)) {
        send_to_char(ch, "You should get up first.\r\n");
        return;
    }

    int perc = GET_SKILL(ch, SKILL_WARP);
    int prob = axion_dice(0), cost = GET_MAX_MANA(ch) / 20, pass = false;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "What planet are you wanting to warp to?\n[ earth | frigid | kanassa | namek | aether ]\r\n");
        return;
    }

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki to perform the technique.\r\n");
        return;
    }

    if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 4600 && GET_ROOM_VNUM(IN_ROOM(ch)) < 4700) {
        pass = true;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 795 && GET_ROOM_VNUM(IN_ROOM(ch)) < 1099) {
        pass = true;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 15100 && GET_ROOM_VNUM(IN_ROOM(ch)) < 15299) {
        pass = true;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 13155 && GET_ROOM_VNUM(IN_ROOM(ch)) < 13199) {
        pass = true;
    } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NAMEK) && SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM) {
        pass = true;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 12103 && GET_ROOM_VNUM(IN_ROOM(ch)) < 12289) {
        pass = true;
    }

    if (pass == false) {
        send_to_char(ch, "You must be on or in a sea or ocean for warp pool to work.\r\n");
        return;
    }

    if (!strcasecmp("earth", arg) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_EARTH)) {
        send_to_char(ch, "You are already on Earth!\r\n");
        return;
    } else if (!strcasecmp("frigid", arg) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_FRIGID)) {
        send_to_char(ch, "You are already on Frigid!\r\n");
        return;
    } else if (!strcasecmp("kanassa", arg) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_KANASSA)) {
        send_to_char(ch, "You are already on Kanasssa!\r\n");
        return;
    } else if (!strcasecmp("namek", arg) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_NAMEK)) {
        send_to_char(ch, "You are already on Namek!\r\n");
        return;
    } else if (!strcasecmp("aether", arg) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_AETHER)) {
        send_to_char(ch, "You are already on Aether!\r\n");
        return;
    } else if (!strcasecmp("earth", arg)) {
        if (prob > perc) {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. You lose your concentration and the ritual fails!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly a puzzled look comes across @c$n's @Cface and the water returns to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
            improve_skill(ch, SKILL_WARP, 1);
        } else {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. As you complete the ritual you connect the water you disturbed with the water you envisioned and warp between the two points!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly @c$n@C vanishes into this water! A moment later the waters return to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_WARP, 1);
            char_from_room(ch);
            char_to_room(ch, real_room(850));
            act("@CSuddenly a large whirlpool of flashing water begins to form nearby. After a few seconds @c$n@C pops out of the center of the pool! The water then return to normal a moment laterr...@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
        }
    } else if (!strcasecmp("frigid", arg)) {
        if (prob > perc) {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. You lose your concentration and the ritual fails!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly a puzzled look comes across @c$n's @Cface and the water returns to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
            improve_skill(ch, SKILL_WARP, 1);
        } else {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. As you complete the ritual you connect the water you disturbed with the water you envisioned and warp between the two points!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly @c$n@C vanishes into this water! A moment later the waters return to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_WARP, 1);
            char_from_room(ch);
            char_to_room(ch, real_room(4609));
            act("@CSuddenly a large whirlpool of flashing water begins to form nearby. After a few seconds @c$n@C pops out of the center of the pool! The water then return to normal a moment laterr...@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
        }
    } else if (!strcasecmp("namek", arg)) {
        if (prob > perc) {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. You lose your concentration and the ritual fails!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly a puzzled look comes across @c$n's @Cface and the water returns to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
            improve_skill(ch, SKILL_WARP, 1);
        } else {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. As you complete the ritual you connect the water you disturbed with the water you envisioned and warp between the two points!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly @c$n@C vanishes into this water! A moment later the waters return to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_WARP, 1);
            char_from_room(ch);
            char_to_room(ch, real_room(10904));
            act("@CSuddenly a large whirlpool of flashing water begins to form nearby. After a few seconds @c$n@C pops out of the center of the pool! The water then return to normal a moment laterr...@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
        }
    } else if (!strcasecmp("kanassa", arg)) {
        if (prob > perc) {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. You lose your concentration and the ritual fails!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly a puzzled look comes across @c$n's @Cface and the water returns to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
            improve_skill(ch, SKILL_WARP, 1);
        } else {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. As you complete the ritual you connect the water you disturbed with the water you envisioned and warp between the two points!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly @c$n@C vanishes into this water! A moment later the waters return to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_WARP, 1);
            char_from_room(ch);
            char_to_room(ch, real_room(15100));
            act("@CSuddenly a large whirlpool of flashing water begins to form nearby. After a few seconds @c$n@C pops out of the center of the pool! The water then return to normal a moment laterr...@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
        }
    } else if (!strcasecmp("aether", arg)) {
        if (prob > perc) {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. You lose your concentration and the ritual fails!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly a puzzled look comes across @c$n's @Cface and the water returns to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
            improve_skill(ch, SKILL_WARP, 1);
        } else {
            act("@CYou reach your hand out and begin to swirl nearby water with it. At the same time you release ki into the water and focus your mind on sensing out the distant body of water you wish to travel to. As you complete the ritual you connect the water you disturbed with the water you envisioned and warp between the two points!@n",
                true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C reaches $s hand out and begins to swirl nearby water with it. The water that is being swirled begins to glow @wbright@B blue@C and has a distinct separation from the rest of the waters. Suddenly @c$n@C vanishes into this water! A moment later the waters return to normal.@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            improve_skill(ch, SKILL_WARP, 1);
            char_from_room(ch);
            char_to_room(ch, real_room(12252));
            act("@CSuddenly a large whirlpool of flashing water begins to form nearby. After a few seconds @c$n@C pops out of the center of the pool! The water then return to normal a moment laterr...@n",
                true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurKI(cost);
        }
    } else {
        send_to_char(ch,
                     "That is not an acceptable choice. It must be a planet with a large body of water.\n[ earth | frigid | kanassa | namek | aether ]\r\n");
        return;
    }
}

/* This allows a player to block off the exit of a room */
ACMD(do_obstruct) {

    if (IS_NPC(ch))
        return;

    if (!know_skill(ch, SKILL_HYOGA_KABE)) {
        return;
    }

    auto r = ch->getRoom();

    if (r->room_flags.test(ROOM_PEACEFUL)) {
        send_to_char(ch, "You can not use this in such a peaceful area.\r\n");
        return;
    }

    if (r->sector_type == SECT_SPACE || r->room_flags.test(ROOM_SPACE)) {
        send_to_char(ch, "You can not wall off the vastness of space.\r\n");
        return;
    }

    if (r->sector_type == SECT_FLYING) {
        send_to_char(ch, "You can not create gravity defying glacial walls.\r\n");
        return;
    }

    char arg[MAX_INPUT_LENGTH];
    int skill = GET_SKILL(ch, SKILL_HYOGA_KABE);
    int prob = axion_dice(0), cost = ((GET_MAX_MANA(ch) / skill) * 2.5);

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch,
                     "What direction are you wanting to block off?\n[ N | E | S | W | NE | NW | SE | SW | U | D | I | O ]\r\n");
        return;
    }

    if ((ch->getCurKI()) < cost) {
        send_to_char(ch, "You do not have enough ki to perform the technique.\r\n");
        return;
    }

    int dir = search_block(arg, dirs, false);
    if(dir < 0) {
        send_to_char(ch,
                     "That is not an acceptable direction.\n[ N | E | S | W | NE | NW | SE | SW | U | D | I | O ]\r\n");
        return;
    }
    int dir2 = rev_dir[dir];

    auto e = r->dir_option[dir];
    if(!e) {
        send_to_char(ch, "That direction does not exist here.\r\n");
        return;
    }
    auto dest = e->getDestination();
    if(!dest) {
        send_to_char(ch, "That leads nowhere.\r\n");
        return;
    }

    if (skill < prob) {
        act("@CYou channel your ki and start to create a wall of water, but lose your concentration and the water promptly disappears.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C channels $s ki and starts to create a wall of water, but loses $s concentration and the water promptly disappears.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->decCurKI(cost);
        improve_skill(ch, SKILL_HYOGA_KABE, 0);
        return;
    }
    struct obj_data *obj;
    int newroom = dest->vn;

    if (ROOM_FLAGGED(newroom, ROOM_PEACEFUL)) {
        send_to_char(ch, "You can not block off a peaceful area.\r\n");
        return;
    }

    for (obj = dest->contents; obj; obj = obj->next_content) {
        if (GET_OBJ_VNUM(obj) == 79) {
            if (GET_OBJ_COST(obj) == dir2) {
                if (skill < prob) {
                    act("@CYou place your hands on the glacial wall and concentrate. You fail to undo the composition of the wall!@n",
                        true, ch, nullptr, nullptr, TO_CHAR);
                    act("@c$n@C places $s hands on the glacial wall and concentrates. Nothing happens...@n", true,
                        ch, nullptr, nullptr, TO_ROOM);
                    ch->decCurKI(cost / 2);
                } else {
                    act("@CYou place your hands on the glacial wall and concentrate. You unfreeze the wall and evaporate the water effortlessly.@n",
                        true, ch, nullptr, nullptr, TO_CHAR);
                    act("@c$n@C places $s hands on the glacial wall and concentrates. Suddenly the wall melts and then evaporates!@n",
                        true, ch, nullptr, nullptr, TO_ROOM);
                    ch->decCurKI(cost / 2);
                    extract_obj(obj);
                }
                return;
            }
        }
    }

    struct obj_data *obj2, *obj3;

    obj2 = read_object(79, VIRTUAL);
    obj_to_room(obj2, newroom);
    obj3 = read_object(79, VIRTUAL);
    obj_to_room(obj3, IN_ROOM(ch));

    int64_t strength = (((GET_INT(ch) * skill) * GET_WIS(ch)) * 20) + (GET_MAX_MANA(ch) * 0.001);

    if (strength > GET_MAX_HIT(ch) * 20) {
        strength = GET_MAX_HIT(ch) + (strength / 20);
    } else if (strength > GET_MAX_HIT(ch) * 15) {
        strength = GET_MAX_HIT(ch) + (strength / 15);
    } else if (strength > GET_MAX_HIT(ch) * 10) {
        strength = GET_MAX_HIT(ch) + (strength / 10);
    } else if (strength > GET_MAX_HIT(ch) * 5) {
        strength = GET_MAX_HIT(ch) + (strength / 5);
    } else if (strength > GET_MAX_HIT(ch) * 2) {
        strength = GET_MAX_HIT(ch) + (strength / 2);
    }

    GET_OBJ_COST(obj2) = dir2;
    GET_OBJ_WEIGHT(obj2) = strength;
    GET_OBJ_COST(obj3) = dir;
    GET_OBJ_WEIGHT(obj3) = strength;
    GET_FELLOW_WALL(obj2) = obj3;
    GET_FELLOW_WALL(obj3) = obj2;
    act("@CYou concentrate and channel your ki. A wall of water starts to form in such a way to block off the direction of your choice. As the wall becomes complete it freezes solid by your will!@n",
        true, ch, nullptr, nullptr, TO_CHAR);
    act("@c$n@C concentrates and channels $s ki. A wall of water starts to form in such a way to block off one of the directions of this area. As the wall becomes complete it freezes solid by @c$n's@C will!@n",
        true, ch, nullptr, nullptr, TO_ROOM);
    send_to_room(newroom,
                 "@cA wall of water forms slowly upward blocking off the %s direction. This wall of water then freezes instantly once it stops growing.@n\r\n",
                 dirs[dir2]);
    improve_skill(ch, SKILL_HYOGA_KABE, 0);
    ch->decCurKI(cost);
}

/* This allows a player to flood a room. */
ACMD(do_dimizu) {

    if (IS_NPC(ch))
        return;

    if (!know_skill(ch, SKILL_DIMIZU)) {
        return;
    }

    int skill = GET_SKILL(ch, SKILL_DIMIZU);
    int prob = axion_dice(0);

    if (ROOM_EFFECT(IN_ROOM(ch)) < 0) {
        act("@CYou concentrate and distabilie the water, separating the hydrogen and oxygen. The gases dissipate quickly.",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C concentrates and the water filling the area seems to shudder. Suddenly the water begins to evaporate as the hydrogen and oxygen are separated.",
            true, ch, nullptr, nullptr, TO_ROOM);
        ROOM_EFFECT(IN_ROOM(ch)) = 0;
        WAIT_STATE(ch, PULSE_1SEC);
        return;
    } else if (SECT(IN_ROOM(ch)) == SECT_UNDERWATER) {
        send_to_char(ch, "The area is already underwater!\r\n");
        return;
    } else if (SECT(IN_ROOM(ch)) == SECT_SPACE || ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE)) {
        send_to_char(ch, "You can't flood space!\r\n");
        return;
    } else if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 12) {
        send_to_char(ch, "You do not have enough ki to perform the technique.\r\n");
        return;
    } else if (skill < prob) {
        act("@CYou gather your ki and concentrate on creating water from it. Water begins to flow upward around the entire area, but you lose your concentration and it all goes flooding away!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C gathers $s ki and concentrates on creating water from it. Water begins to flow upward around the entire area, but $e loses $s concentration and all the water goes flooding away!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->decCurKI(ch->getMaxKI() / 12);
        improve_skill(ch, SKILL_DIMIZU, 0);
        return;
    } else {
        act("@CYou gather your ki and concentrate on creating water from it. Water begins to flow upward around the entire area. You form the water into a perfect cube with barely any ripples in its walls. It will maintain this form for a while.@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@c$n@C gathers $s ki and concentrates on creating water from it. Water begins to flow upward around the entire area. @c$n@C forms the water into a perfect cube with barely any ripples in its walls. It appears the water will maintain this form for a while.@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        ch->decCurKI(ch->getMaxKI() / 12);
        ROOM_EFFECT(IN_ROOM(ch)) = -3;
        improve_skill(ch, SKILL_DIMIZU, 0);
        return;
    }
}

/* Allows a player to place a "beacon" on a room they want to return to if
 * they revive from death. */
ACMD(do_beacon) {

    if (IS_NPC(ch))
        return;

    if (AFF_FLAGGED(ch, AFF_SPIRIT)) {
        send_to_char(ch, "You are dead. You can not stake out a room to return to upon revival.\r\n");
        return;
    } else if (GET_ROOM_VNUM(IN_ROOM(ch)) >= 0 && GET_ROOM_VNUM(IN_ROOM(ch)) <= 14) {
        send_to_char(ch, "You can not stake out an immortal room to be revived in.\r\n");
        return;
    } else {
        send_to_char(ch, "You stake out the room you are in and will return to it if you die and are revived.\r\n");
        GET_DROOM(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
        return;
    }

}

/* Feed senzu to someone you are grouped with. Why? TEAMWORK! */
ACMD(do_feed) {

    if (IS_NPC(ch))
        return;

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Feed a senzu to whom?\r\n");
        return;
    }

    if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "That target isn't here.\r\n");
        return;
    }

    if (IS_ANDROID(vict)) {
        send_to_char(ch, "They are unaffected by senzu beans.\r\n");
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg2, nullptr, ch->contents))) {
        send_to_char(ch, "You need to give them a senzu.\r\n");
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_POTION) {
        send_to_char(ch, "You can only feed senzu beans.\r\n");
        return;
    }

    if (OBJ_FLAGGED(obj, ITEM_FORGED)) {
        send_to_char(ch, "They can't swallow that, it is fake!\r\n");
        return;
    }

    if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
        send_to_char(ch, "They can't swallow that, it is broken!\r\n");
        return;
    }

    if (FIGHTING(vict)) {
        send_to_char(ch, "They are a bit busy at the moment!\r\n");
        return;
    }

    if (vict->master != ch && ch->master != vict && ch->master != vict->master) {
        send_to_char(ch, "You need to be grouped with them first.\r\n");
        return;
    }

    if (!AFF_FLAGGED(vict, AFF_GROUP) || !AFF_FLAGGED(ch, AFF_GROUP)) {
        send_to_char(ch, "You need to be grouped with them first.\r\n");
        return;
    }

    act("@WYou take $p@W and pop it into @C$N@W's mouth!@n", true, ch, obj, vict, TO_CHAR);
    act("@C$n@W takes $p@W and pops it into YOUR mouth!@n", true, ch, obj, vict, TO_VICT);
    act("@C$n@W takes $p@W and pops it into @c$N@W's mouth!@n", true, ch, obj, vict, TO_NOTVICT);
    mag_objectmagic(vict, obj, "");
}

/* This allows players to decapitate a corpse for a sick trophy. */
ACMD(do_spoil) {

    if (IS_NPC(ch))
        return;

    char arg[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int type = 0;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "What corpse do you want to decapitate?\r\n");
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
        send_to_char(ch, "No corpse around here by that name.\r\n");
        return;
    }

    if (GET_OBJ_VAL(obj, VAL_CORPSE_HEAD) == 0) {
        send_to_char(ch, "That corpse is already missing its head.\r\n");
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD1)) {
        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_SLASH - TYPE_HIT) {
            type = 1;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_PIERCE - TYPE_HIT) {
            type = 1;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD1), VAL_WEAPON_DAMTYPE) == TYPE_STAB - TYPE_HIT) {
            type = 1;
        }
    } else if (GET_EQ(ch, WEAR_WIELD2)) {
        if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_SLASH - TYPE_HIT) {
            type = 2;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_PIERCE - TYPE_HIT) {
            type = 2;
        } else if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD2), VAL_WEAPON_DAMTYPE) == TYPE_STAB - TYPE_HIT) {
            type = 2;
        }
    }

    if (type == 0) {
        act("@C$n@W reaches down and @rtears@W the head off of @R$p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        act("@WYou reach down and @rtear@W the head off of @R$p@W!@n", true, ch, obj, nullptr, TO_CHAR);
    } else if (type == 1) {
        act("@C$n@W reaches down and @rcuts@W the head off of @R$p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        act("@WYou reach down and @rcut@W the head off of @R$p@W!@n", true, ch, obj, nullptr, TO_CHAR);
    } else if (type == 2) {
        act("@C$n@W reaches down and @rcuts@W the head off of @R$p@W!@n", true, ch, obj, nullptr, TO_ROOM);
        act("@WYou reach down and @rcut@W the head off of @R$p@W!@n", true, ch, obj, nullptr, TO_CHAR);
    }

    GET_OBJ_VAL(obj, VAL_CORPSE_HEAD) = 0;

    struct obj_data *body_part;
    char part[1000];
    char buf[1000];
    char buf2[1000];
    char buf3[1000];

    *part = '\0';
    *buf = '\0';
    *buf2 = '\0';
    *buf3 = '\0';

    body_part = create_obj();
    body_part->vn = NOTHING;
    IN_ROOM(body_part) = NOWHERE;
    snprintf(part, sizeof(part), "%s", obj->name);
    search_replace(part, "headless", "");
    search_replace(part, "corpse", "");
    search_replace(part, "half", "");
    search_replace(part, "burnt", "");
    search_replace(part, "chunks", "");
    search_replace(part, "beaten", "");
    search_replace(part, "bloody", "");
    trim(part);
    snprintf(buf, sizeof(buf), "bloody head %s", part);
    snprintf(buf2, sizeof(buf2), "@wThe bloody head of %s@w is lying here@n", part);
    snprintf(buf3, sizeof(buf3), "@wThe bloody head of %s@w@n", part);

    body_part->name = strdup(buf);
    body_part->room_description = strdup(buf2);
    body_part->short_description = strdup(buf3);

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
    obj_to_room(body_part, IN_ROOM(ch));
    obj_from_room(body_part);
    obj_to_char(body_part, ch);

}
