/* ************************************************************************
*   File: act.item.c                                    Part of CircleMUD *
*  Usage: object handling routines -- get/drop and container handling     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "dbat/act.item.h"
#include "dbat/vehicles.h"
#include "dbat/dg_comm.h"
#include "dbat/act.wizard.h"
#include "dbat/act.other.h"
#include "dbat/act.comm.h"
#include "dbat/act.informative.h"
#include "dbat/config.h"
#include "dbat/assemblies.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/spells.h"
#include "dbat/handler.h"
#include "dbat/class.h"
#include "dbat/feats.h"
#include "dbat/guild.h"
#include "dbat/constants.h"
#include "dbat/genzon.h"
#include "dbat/dg_scripts.h"
#include "dbat/boards.h"

/* global variables */
struct obj_data *obj_selling = nullptr;    /* current object for sale */
struct char_data *ch_selling = nullptr;    /* current character selling obj */
struct char_data *ch_buying = nullptr;    /* current character buying the object */

/* local vvariables  */
static int curbid = 0;                /* current bid on item being auctioned */
static int aucstat = AUC_NULL_STATE;        /* state of auction.. first_bid etc.. */

static const char *auctioneer[AUC_BID + 1] = {

        "@D[@CAUCTION@c: @C$n@W puts $p@W up for sale at @Y%d@W zenni.@D]@n",
        "@D[@CAUCTION@c: @W$p@W at @Y%d@W zenni going once!@D]@n",
        "@D[@CAUCTION@c: @W$p@W at @Y%d@W zenni going twice!@D]@n",
        "@D[@CAUCTION@c: @WLast call: $p@W going for @Y%d@W zenni.@D]@n",
        "@D[@CAUCTION@c: @WUnfortunately $p@W is unsold, returning it to $n. @D]@n",
        "@D[@CAUCTION@c: @WSOLD! $p@W to @C$n@W for @Y%d@W zenni!@D]@n",
        "@D[@CAUCTION@c: @WSorry, @C$n@W has cancelled the auction.@D]@n",
        "@D[@CAUCTION@c: @WSorry, @C$n@W has left us, the auction can't go on.@D]@n",
        "@D[@CAUCTION@c: @WSorry, $p@W has been confiscated, shame on you $n.@D]@n",
        "@D[@CAUCTION@c: @C$n@W is selling $p@W for @Y%d@W zenni.@D]@n",
        "@D[@CAUCTION@c: @C$n@W bids @Y%d@W zenni on $p@W.@D]@n"
};

/* local functions */
static void majin_gain(struct char_data *ch, struct obj_data *food, int foob);

static int can_take_obj(struct char_data *ch, struct obj_data *obj);

static void get_check_money(struct char_data *ch, struct obj_data *obj);

static void get_from_room(struct char_data *ch, char *arg, int howmany);

static void perform_give_gold(struct char_data *ch, struct char_data *vict, int amount);

static void perform_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj);

static int perform_drop(struct char_data *ch, struct obj_data *obj, int8_t mode, const char *sname, room_rnum RDR);

static void perform_drop_gold(struct char_data *ch, int amount, int8_t mode, room_rnum RDR);

static struct char_data *give_find_vict(struct char_data *ch, char *arg);

static void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont);

static void get_from_container(struct char_data *ch, struct obj_data *cont, char *arg, int mode, int howmany);

static void wear_message(struct char_data *ch, struct obj_data *obj, int where);

static void perform_get_from_container(struct char_data *ch, struct obj_data *obj, struct obj_data *cont, int mode);

static int hands(struct char_data *ch);

static void start_auction(struct char_data *ch, struct obj_data *obj, int
bid);

static void auc_stat(struct char_data *ch, struct obj_data *obj);

static void auc_send_to_all(char *messg, bool buyer);

static bool has_housekey(struct char_data *ch, struct obj_data *obj);

static void harvest_plant(struct char_data *ch, struct obj_data *plant);

static int can_harvest(struct obj_data *plant);

static char *find_exdesc_keywords(char *word, struct extra_descr_data *list);

/* local variables */
static char buf[MAX_STRING_LENGTH];

// definitions

ACMD(do_refuel) {

    struct obj_data *controls;

    if (!(controls = find_control(ch))) {
        send_to_char(ch, "@wYou need to be in the cockpit to place a new fuel canister into the ship.\r\n");
        return;
    }

    struct obj_data *rep = nullptr, *next_obj = nullptr, *fuel = nullptr;
    fuel = ch->findObjectVnum(17290);

    if (!fuel) {
        send_to_char(ch, "You do not have any fuel canisters on you.\r\n");
        return;
    }

    int max = 0;

    if (GET_OBJ_VNUM(controls) >= 44000 && GET_OBJ_VNUM(controls) <= 44199) {
        max = 300;
    } else if (GET_OBJ_VNUM(controls) >= 44200 && GET_OBJ_VNUM(controls) <= 44499) {
        max = 500;
    } else if (GET_OBJ_VNUM(controls) >= 44200 && GET_OBJ_VNUM(controls) <= 44999) {
        max = 1000;
    }

    if (GET_FUEL(controls) == max) {
        send_to_char(ch, "The ship is full on fuel!\r\n");
        return;
    } else {

        if (GET_FUEL(controls) + (GET_OBJ_WEIGHT(fuel) * 4) > max) {
            GET_FUEL(controls) = max;
        } else {
            GET_FUEL(controls) += (GET_OBJ_WEIGHT(fuel) * 4);
        }

        extract_obj(fuel);

        send_to_char(ch,
                     "You place the fuel canister into the ship. Within seconds the fuel has been extracted from the canister into the ships' internal tanks.\r\n");

    }
}

static int can_harvest(struct obj_data *plant) {

    switch (GET_OBJ_VNUM(plant)) {
        case 250:
        case 1129:
        case 17210:
        case 17211:
        case 17214:
        case 17216:
        case 17218:
        case 17220:
        case 17222:
        case 17224:
        case 17226:
        case 3702:
            return true;
    }

    return false;

}

static void harvest_plant(struct char_data *ch, struct obj_data *plant) {
    int extract = false, reward = rand_number(5, 15), count = reward;
    struct obj_data *fruit = nullptr;

    if (GET_OBJ_VAL(plant, VAL_SOILQ) > 7) {
        reward += 10;
        send_to_char(ch, "@GThe soil seems to have made the plant exteremely bountiful");
    } else if (GET_OBJ_VAL(plant, VAL_SOILQ) >= 5) {
        reward += 6;
        send_to_char(ch, "@GThe soil seems to have made the plant very bountiful");
    } else if (GET_OBJ_VAL(plant, VAL_SOILQ) >= 3) {
        reward += 4;
        send_to_char(ch, "@GThe soil seems to have made the plant bountiful");
    } else if (GET_OBJ_VAL(plant, VAL_SOILQ) > 0) {
        reward += 2;
        send_to_char(ch, "@GThe soil seems to have made the plant a bit more bountiful");
    }

    int skill = GET_SKILL(ch, SKILL_GARDENING);

    if (skill >= 100) {
        reward += 10;
        send_to_char(ch, " and your outstanding skill has helped the plant be more bountiful yet!@n\r\n");
    } else if (skill >= 90) {
        reward += 8;
        send_to_char(ch, " and your great skill has also helped the plant be more bountiful yet!@n\r\n");
    } else if (skill >= 80) {
        reward += 5;
        send_to_char(ch, " and your good skill has also helped the plant be more bountiful yet!@n\r\n");
    } else if (skill >= 50) {
        reward += 3;
        send_to_char(ch, " and your decent skill has also helped the plant be more bountiful yet!@n\r\n");
    } else if (skill >= 40) {
        reward += 2;
        send_to_char(ch,
                     " and your mastery of the basics of gardening has also helped the plant be more bountiful yet!@n\r\n");
    } else if (skill >= 30) {
        reward += 1;
        send_to_char(ch,
                     " and you somehow managed to make the plant slightly more bountiful with what little you know!@n\r\n");
    } else {
        send_to_char(ch, ".@n\r\n");
    }

    count = reward;

    switch (GET_OBJ_VNUM(plant)) {
        case 250:
            if (reward > 2) {
                reward = 2;
                count = 2;
            }
            while (count > 0) {
                fruit = read_object(1, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = false;
            break;
        case 1129:
            while (count > 0) {
                fruit = read_object(1131, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17210:
            reward += 2;
            count += 2;
            while (count > 0) {
                fruit = read_object(17212, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17211:
            reward += 2;
            count += 2;
            while (count > 0) {
                fruit = read_object(17213, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17214:
            reward += 1;
            count += 1;
            while (count > 0) {
                fruit = read_object(17215, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17216:
            while (count > 0) {
                fruit = read_object(17217, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17218:
            reward += 14;
            count += 14;
            while (count > 0) {
                fruit = read_object(17219, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17220:
            reward -= reward * 0.75;
            count = reward;
            while (count > 0) {
                fruit = read_object(17221, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17222:
            reward += rand_number(1, 3);
            count = reward;
            while (count > 0) {
                fruit = read_object(17223, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17224:
            reward += 10;
            count = reward;
            while (count > 0) {
                fruit = read_object(17225, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 17226:
            reward -= 8;
            if (reward < 0)
                reward = 1;
            count = reward;
            while (count > 0) {
                fruit = read_object(17227, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        case 3702:
            reward -= 2;
            count = reward;
            while (count > 0) {
                fruit = read_object(3703, VIRTUAL);
                obj_to_char(fruit, ch);
                count -= 1;
            }
            send_to_char(ch, "@YYou harvest @D[@G%d@D]@Y @g%s@Y!@n\r\n", reward, fruit->short_description);
            extract = true;
            break;
        default:
            send_to_imm("ERROR: Harvest plant called for illegitimate plant, VNUM %d.", GET_OBJ_VNUM(plant));
            break;
    }

    if (extract == true) {
        send_to_char(ch,
                     "@wThe harvesting process has killed the plant. Do not worry, this is normal for that type.@n\r\n");
        extract_obj(plant);
    } else {
        GET_OBJ_VAL(plant, VAL_MATURITY) = 3;
        GET_OBJ_VAL(plant, VAL_GROWTH) = 0;
    }

}

ACMD(do_garden) {

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    two_arguments(argument, arg, arg2);

    if (!GET_SKILL(ch, SKILL_GARDENING) && slot_count(ch) + 1 <= GET_SLOTS(ch)) {
        int numb = rand_number(8, 16);
        SET_SKILL(ch, SKILL_GARDENING, numb);
        send_to_char(ch, "@GYou learn the very basics of gardening.\r\n");
    } else if (!GET_SKILL(ch, SKILL_GARDENING) && slot_count(ch) + 1 > GET_SLOTS(ch)) {
        send_to_char(ch, "You need additional skill slots to pick up the skill linked with this.\r\n");
        return;
    }

    if (*arg) {
        if (!strcasecmp(arg, "collect")) {
            struct obj_data *obj2, *shovel = nullptr, *next_obj;
            int found = false;
            shovel = ch->findObjectVnum(254);
            if (!shovel) {
                send_to_char(ch, "You need a shovel in order to collect soil.\r\n");
                return;
            }
            if (SECT(IN_ROOM(ch)) != SECT_FOREST && SECT(IN_ROOM(ch)) != SECT_FIELD &&
                SECT(IN_ROOM(ch)) != SECT_MOUNTAIN && SECT(IN_ROOM(ch)) != SECT_HILLS) {
                send_to_char(ch, "You can not collect soil from this area.\r\n");
                return;
            }
            if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_FERTILE1)) {
                struct obj_data *soil = read_object(255, VIRTUAL);
                obj_to_char(soil, ch);
                act("@yYou sink your shovel into the soft ground and manage to dig up a pile of fertile soil!@n", true,
                    ch, nullptr, nullptr, TO_CHAR);
                act("@w$n@y sinks $s shovel into the soft ground and manages to dig up a pile of fertile soil!@n", true,
                    ch, nullptr, nullptr, TO_ROOM);
                GET_OBJ_VAL(soil, 0) = 8;
                WAIT_STATE(ch, PULSE_4SEC);
                return;
            } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_FERTILE2)) {
                struct obj_data *soil = read_object(255, VIRTUAL);
                obj_to_char(soil, ch);
                act("@yYou sink your shovel into the soft ground and manage to dig up a pile of good soil!@n", true, ch,
                    nullptr, nullptr, TO_CHAR);
                act("@w$n@y sinks $s shovel into the soft ground and manages to dig up a pile of good soil!@n", true,
                    ch, nullptr, nullptr, TO_ROOM);
                GET_OBJ_VAL(soil, 0) = rand_number(5, 7);
                WAIT_STATE(ch, PULSE_4SEC);
                return;
            } else {
                struct obj_data *soil = read_object(255, VIRTUAL);
                obj_to_char(soil, ch);
                act("@yYou sink your shovel into the soft ground and manage to dig up a pile of soil!@n", true, ch,
                    nullptr, nullptr, TO_CHAR);
                act("@w$n@y sinks $s shovel into the soft ground and manages to dig up a pile of soil!@n", true, ch,
                    nullptr, nullptr, TO_ROOM);
                GET_OBJ_VAL(soil, 0) = rand_number(0, 4);
                WAIT_STATE(ch, PULSE_4SEC);
                return;
            }
        }
    }

    if (!*arg || !*arg2) {
        send_to_char(ch, "Syntax: garden (plant) ( water | harvest | dig | plant | pick )\r\n");
        send_to_char(ch, "Syntax: garden collect [Will collect soil from a room with soil.\r\n");
        return;
    }

    if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN1) && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN2)) {
        send_to_char(ch, "You are not even in a garden!\r\n");
        return;
    }

    if (!strcasecmp(arg2, "plant")) {
        if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
            send_to_char(ch, "What are you trying to plant?\r\n");
            send_to_char(ch, "Syntax: garden (plant in inventory) plant\r\n");
            return;
        }
    } else {
        if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "That plant doesn't seem to be here.\r\n");
            return;
        }
    }

    if (!obj) {
        send_to_char(ch, "What plant are you gardening?\r\n");
        return;
    }

    int64_t cost = (GET_MAX_MOVE(ch) * 0.005) + rand_number(50, 150);
    int skill = GET_SKILL(ch, SKILL_GARDENING);

    if ((ch->getCurST()) < cost) {
        send_to_char(ch, "@WYou need at least @G%s@W stamina to garden.\r\n", add_commas(cost).c_str());
        return;
    } else {
        if (!strcasecmp(arg2, "water")) {
            struct obj_data *obj2, *water = nullptr, *next_obj;
            int found = false;
            water = ch->findObjectVnum(251);
            if (!water) {
                send_to_char(ch, "You do not have any grow water!\r\n");
                return;
            } else if (GET_OBJ_VAL(obj, VAL_WATERLEVEL) >= 500) {
                send_to_char(ch, "You stop as you realize that the plant already has enough water.\r\n");
                return;
            } else if (GET_OBJ_VAL(obj, VAL_WATERLEVEL) <= -10) {
                send_to_char(ch, "The plant is dead!\r\n");
                return;
            } else if (skill < axion_dice(0)) {
                act("@GAs you go to water @g$p@G you end up sloppily wasting about half of it on the ground.@n", true,
                    ch, obj, nullptr, TO_CHAR);
                act("@g$n@G takes a bottle of grow water and sloshes some of it on @g$p@G.@n", true, ch, obj, nullptr,
                    TO_ROOM);

                ch->decCurST(cost);
                GET_OBJ_VAL(obj, VAL_WATERLEVEL) += 40;
                if (GET_OBJ_VAL(obj, VAL_WATERLEVEL) > 500) {
                    GET_OBJ_VAL(obj, VAL_WATERLEVEL) = 500;
                    send_to_char(ch, "@YThe plant is now at full water level.@n\r\n");
                }
                extract_obj(water);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            } else {
                act("@GYou calmly and expertly pour the grow water on @g$p@G.@n", true, ch, obj, nullptr, TO_CHAR);
                act("@g$n@G calmly and expertly pours some grow water on @g$p@G.@n", true, ch, obj, nullptr, TO_ROOM);
                ch->decCurST(cost);
                GET_OBJ_VAL(obj, VAL_WATERLEVEL) += 225;
                if (GET_OBJ_VAL(obj, VAL_WATERLEVEL) >= 500) {
                    GET_OBJ_VAL(obj, VAL_WATERLEVEL) = 500;
                    send_to_char(ch, "@YThe plant is now at full water level.@n\r\n");
                }
                extract_obj(water);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            }
        } else if (!strcasecmp(arg2, "harvest")) {
            struct obj_data *obj2, *clippers = nullptr, *next_obj;
            int found = false;
            clippers = ch->findObjectVnum(253);
            if (!clippers) {
                send_to_char(ch, "You do not have any working gardening clippers!\r\n");
                return;
            } else if (can_harvest(obj) == false) {
                send_to_char(ch, "You can not harvest that plant. Instead, Syntax: garden (plant) (pick)\r\n");
                return;
            } else if (GET_OBJ_VAL(obj, VAL_WATERLEVEL) <= -10) {
                send_to_char(ch, "That plant is dead!\r\n");
                return;
            } else if (GET_OBJ_VAL(obj, VAL_MATURITY) < GET_OBJ_VAL(obj, VAL_MAXMATURE)) {
                send_to_char(ch, "You stop as you realize that the plant isn't mature enough to harvest.\r\n");
                return;
            } else if (skill < axion_dice(-5)) {
                act("@GAs you go to harvest @g$p@G you end up cutting it in half instead!@n", true, ch, obj, nullptr,
                    TO_CHAR);
                act("@g$n@G attempts to harvest @g$p@G with $s clippers, but accidently cuts the plant in half!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurST(cost);
                extract_obj(obj);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            } else {
                act("@GYou calmly and expertly harvest @g$p@G.@n", true, ch, obj, nullptr, TO_CHAR);
                act("@g$n@G calmly and expertly harvests @g$p@G.@n", true, ch, obj, nullptr, TO_ROOM);
                ch->decCurST(cost);
                GET_OBJ_VAL(clippers, VAL_ALL_HEALTH) -= 1;
                if (GET_OBJ_VAL(clippers, VAL_ALL_HEALTH) <= 0) {
                    send_to_char(ch, "The clippers are now too dull to use.\r\n");
                    return;
                }
                harvest_plant(ch, obj);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            }
        } else if (!strcasecmp(arg2, "dig")) {
            struct obj_data *obj2, *shovel = nullptr, *next_obj;
            int found = false;
            shovel = ch->findObjectVnum(254);
            if (!shovel) {
                send_to_char(ch, "You do not have any working gardening shovels!\r\n");
                return;
            } else {
                act("@GYou calmly dig up @g$p@G.@n", true, ch, obj, nullptr, TO_CHAR);
                act("@g$n@G calmly digs up @g$p@G.@n", true, ch, obj, nullptr, TO_ROOM);
                ch->decCurST(cost);
                obj_from_room(obj);
                obj_to_char(obj, ch);
                GET_OBJ_VAL(obj, VAL_SOILQ) = 0;
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            }
        } else if (!strcasecmp(arg2, "plant")) {
            struct obj_data *obj2, *shovel, *next_obj;
            int found = false;
            shovel = ch->findObjectVnum(254);

            if (found == false) {
                send_to_char(ch, "You do not have any working gardening shovels!\r\n");
                return;
            }
            found = false;
            struct obj_data *soil = nullptr;
            soil = ch->findObjectVnum(255);

            if (found == false) {
                send_to_char(ch, "You don't have any real soil.\r\n");
            } else if (check_saveroom_count(ch, nullptr) > 7 && ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN1)) {
                send_to_char(ch, "This room already has all its planters full. Try digging up some plants.\r\n");
                return;
            } else if (check_saveroom_count(ch, nullptr) > 19 && ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN2)) {
                send_to_char(ch, "This room already has all its planters full. Try digging up some plants.\r\n");
                return;
            } else if (skill < axion_dice(-5)) {
                act("@GYou end up digging a hole too shallow to hold @g$p@G. Better try again.@n", true, ch, obj,
                    nullptr, TO_CHAR);
                act("@g$n@G digs a very shallow hole in one of the planters and then realizes @g$p@G won't fit in it.@n",
                    true, ch, obj, nullptr, TO_ROOM);
                ch->decCurST(cost);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            } else {
                act("@GYou dig a proper sized hole and plant @g$p@G in it.@n", true, ch, obj, nullptr, TO_CHAR);
                act("@g$n@G digs a proper sized hole in a planter and plants @g$p@G in it.@n", true, ch, obj, nullptr,
                    TO_ROOM);
                obj_from_char(obj);
                obj_to_room(obj, IN_ROOM(ch));
                ch->decCurST(cost);
                GET_OBJ_VAL(obj, VAL_MAXMATURE) = 6;
                GET_OBJ_VAL(obj, VAL_MATGOAL) = 200;
                GET_OBJ_VAL(obj, VAL_SOILQ) = GET_OBJ_VAL(soil, 0);
                switch (GET_OBJ_VAL(obj, VAL_SOILQ)) {
                    case 1:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 10;
                        break;
                    case 2:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 15;
                        break;
                    case 3:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 20;
                        break;
                    case 4:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 25;
                        break;
                    case 5:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 50;
                        break;
                    case 6:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 60;
                        break;
                    case 7:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 70;
                        break;
                    default:
                        GET_OBJ_VAL(obj, VAL_MATGOAL) -= 80;
                        break;
                }
                extract_obj(soil);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
            }
        } else if (!strcasecmp(arg2, "pick")) {
            if (!OBJ_FLAGGED(obj, ITEM_MATURE)) {
                send_to_char(ch, "You can't pick that type of plant. Syntax: garden (plant) harvest\r\n");
                return;
            } else if (GET_OBJ_VAL(obj, VAL_MATURITY) < GET_OBJ_VAL(obj, VAL_MAXMATURE)) {
                send_to_char(ch, "That plant is not mature enough yet.\r\n");
                return;
            } else if (skill < axion_dice(-5)) {
                act("@GYou end up shredding @g$p@G with your clumsy unskilled hands.@n", true, ch, obj, nullptr,
                    TO_CHAR);
                act("@g$n@G grabs a hold of @g$p@G and shreds it in an attempt to pick it.@n", true, ch, obj, nullptr,
                    TO_ROOM);
                return;
            } else {
                act("@GYou grab a hold of @g$p@G and carefully pick it out of the soil.@n", true, ch, obj, nullptr,
                    TO_CHAR);
                act("@g$n@G grabs a hold of @g$p@G and carefully picks it out of the soil.@n", true, ch, obj, nullptr,
                    TO_ROOM);
                obj_from_room(obj);
                obj_to_char(obj, ch);
                ch->decCurST(cost);
                WAIT_STATE(ch, PULSE_3SEC);
                improve_skill(ch, SKILL_GARDENING, 0);
                return;
            }
        } else {
            send_to_char(ch, "Syntax: garden (plant) ( water | harvest | dig | plant | pick )\r\n");
            send_to_char(ch, "Syntax: garden collect [Will collect soil from a room with soil.\r\n");
            return;
        }
    }
}

static bool has_housekey(struct char_data *ch, struct obj_data *obj) {

    if(OBJ_FLAGGED(obj, ITEM_DUPLICATE)) return false;

    auto isHouseKey = [&](struct obj_data *obj2) -> bool {
        if(obj2->vn == 18800 && obj->vn == 18802) return true;
        if(obj2->vn == obj->vn -1) return true;
        return false;
    };

    return ch->findObject(isHouseKey) != nullptr;

}

ACMD(do_pack) {

    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Pack up which type of house capsule?\nSyntax: pack (target)\r\n");
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
        send_to_char(ch, "That house item doesn't seem to be around.\r\n");
        return;
    } else {
        struct obj_data *packed = nullptr;
        if ((GET_OBJ_VNUM(obj) >= 19090 && GET_OBJ_VNUM(obj) <= 19099) || GET_OBJ_VNUM(obj) == 11) {
            act("@CYou push a hidden button on $p@C and a cloud of smoke erupts and covers it. As the smoke clears a small capsule can be seen on the ground.@n",
                true, ch, obj, nullptr, TO_CHAR);
            act("@c$n@C pushes a hidden button on $p@C and a cloud of smokes erupts and covers it. As the smoke clears a small capsule can be seen on the ground.@n",
                true, ch, obj, nullptr, TO_ROOM);
            if (GET_OBJ_VNUM(obj) == 11) {
                extract_obj(obj);
                packed = read_object(19085, VIRTUAL);
                obj_to_room(packed, IN_ROOM(ch));
            } else {
                int fnum = GET_OBJ_VNUM(obj) - 10;
                packed = read_object(fnum, VIRTUAL);
                extract_obj(obj);
                obj_to_room(packed, IN_ROOM(ch));
            }
            return;
        } else if (GET_OBJ_VNUM(obj) >= 18800 && GET_OBJ_VNUM(obj) <= 19199 && GET_OBJ_TYPE(obj) == ITEM_VEHICLE) {
            if (!*arg2) {
                send_to_char(ch,
                             "This will sell off your house and delete everything inside. Are you sure? If you are then enter the command again with a yes at the end.\nSyntax: pack (house) yes\r\n");
                return;
            } else if (strcasecmp(arg2, "yes")) {
                send_to_char(ch,
                             "This will sell off your house and delete everything inside. Are you sure? If you are then enter the command again with a yes at the end.\nSyntax: pack (house) yes\r\n");
                return;
            } else if (has_housekey(ch, obj) == 0) {
                send_to_char(ch, "You do not own this house.\r\n");
                return;
            } else {
                struct obj_data *cont = nullptr;
                act("@CYou push a hidden button on $p@C and a cloud of smoke erupts and covers it. As the smoke clears a pile of money can be seen on the ground!@n",
                    true, ch, obj, nullptr, TO_CHAR);
                act("@c$n@C pushes a hidden button on $p@C and a cloud of smokes erupts and covers it. As the smoke clears a pile of money can be seen on the ground!@n",
                    true, ch, obj, nullptr, TO_ROOM);
                int money = 0, count = 0, rnum = GET_OBJ_VNUM(obj);
                if (GET_OBJ_VNUM(obj) >= 18800 && GET_OBJ_VNUM(obj) <= 18899) {
                    if (rnum == 18802) {
                        rnum = 18800;
                    } else {
                        rnum = rnum - 1;
                    }
                    money = 65000;
                    while (count < 4) {
                        while (world[real_room(rnum)].contents)
                            extract_obj(world[real_room(rnum)].contents);
                        count++;
                        rnum++;
                    }
                } else if (GET_OBJ_VNUM(obj) >= 18900 && GET_OBJ_VNUM(obj) <= 18999) {
                    rnum = rnum - 1;
                    money = 150000;
                    while (count < 4) {
                        while (world[real_room(rnum)].contents)
                            extract_obj(world[real_room(rnum)].contents);
                        count++;
                        rnum++;
                    }
                } else if (GET_OBJ_VNUM(obj) >= 19100 && GET_OBJ_VNUM(obj) <= 19199) {
                    rnum = rnum - 1;
                    money = 1000000;
                    while (count < 4) {
                        while (world[real_room(rnum)].contents)
                            extract_obj(world[real_room(rnum)].contents);
                        count++;
                        rnum++;
                    }
                }
                struct obj_data *obj2 = nullptr, *next_obj;
                for (obj2 = ch->contents; obj2; obj2 = next_obj) {
                    next_obj = obj2->next_content;
                    if (GET_OBJ_VNUM(obj) == 18802) {
                        if (GET_OBJ_VNUM(obj2) == 18800) {
                            extract_obj(obj2);
                        }
                    } else {
                        if (GET_OBJ_VNUM(obj2) == GET_OBJ_VNUM(obj) - 1) {
                            extract_obj(obj2);
                        }
                    }
                }
                struct obj_data *money_obj = create_money(money);
                obj_to_room(money_obj, IN_ROOM(ch));
                extract_obj(obj);
                return;
            }
        } else {
            send_to_char(ch, "That isn't something you can pack up!\r\n");
            return;
        }
    }

}

int check_insidebag(struct obj_data *cont, double mult) {

    struct obj_data *inside = nullptr, *next_obj2 = nullptr;
    int count = 0, containers = 0;

    for (inside = cont->contents; inside; inside = next_obj2) {
        next_obj2 = inside->next_content;
        if (GET_OBJ_TYPE(inside) == ITEM_CONTAINER) {
            count++;
            count += check_insidebag(inside, mult);
            containers++;
        } else {
            count++;
        }
    }

    count = count * mult;
    count += containers;

    return (count);
}

int check_saveroom_count(struct char_data *ch, struct obj_data *cont) {
    struct obj_data *obj, *next_obj = nullptr;
    int count = 0, was = 0;

    if (IN_ROOM(ch) == NOWHERE)
        return 0;
    else if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOUSE))
        return 0;

    for (obj = ch->getRoom()->contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        count++;
        if (!OBJ_FLAGGED(obj, ITEM_CARDCASE)) {
            count += check_insidebag(obj, 0.5);
        }
    }

    was = count;

    if (cont != nullptr) {
        if (!OBJ_FLAGGED(cont, ITEM_CARDCASE)) {
            count += check_insidebag(cont, 0.5);
        }
        count++;
    }

    return (count);
}

ACMD(do_deploy) {

    struct obj_data *obj3, *next_obj, *obj4, *obj = nullptr;
    int capsule = false, furniture = false;

    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);


    if (!*arg) {
        for (obj4 = ch->contents; obj4; obj4 = next_obj) {
            next_obj = obj4->next_content;
            if (GET_OBJ_VNUM(obj4) == 4 || GET_OBJ_VNUM(obj4) == 5 || GET_OBJ_VNUM(obj4) == 6) {
                obj = obj4;
                capsule = true;
            }
        }
    } else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        send_to_char(ch, "Syntax: deploy (no argument for houses)\nSyntax: deploy (target) <-- For furniture\r\n");
        return;
    }

    if (capsule == false && obj) {
        if (GET_OBJ_VNUM(obj) >= 19080 && GET_OBJ_VNUM(obj) <= 19099) {
            capsule = true;
            furniture = true;
        } else {
            send_to_char(ch, "That is not a furniture capsule!\r\n");
            return;
        }
    }

    if (capsule == false) {
        send_to_char(ch, "You do not have any house type capsules to deploy.@n\r\n");
        return;
    } else if (GET_RP(ch) < 10 && furniture == false) {
        send_to_char(ch, "You are required to have (not spend) 10 RPP in order to place a house.\r\n");
        return;
    } else if (furniture == true && (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOUSE) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_SHIP))) {
        send_to_char(ch, "You can't deploy house furniture capsules here.\r\n");
        return;
    } else if (furniture == true &&
               (ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN1) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN2))) {
        send_to_char(ch, "You can't deploy house furniture capsules here.\r\n");
        return;
    } else if (furniture == false && (SECT(IN_ROOM(ch)) == SECT_INSIDE || SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
                                      SECT(IN_ROOM(ch)) == SECT_WATER_SWIM || SECT(IN_ROOM(ch)) == SECT_SPACE)) {
        send_to_char(ch, "You can not deploy that in this kind of area. Try an area more suitable for a house.\r\n");
        return;
    }

    if (furniture == true) {
        int fnum = 0;
        if (GET_OBJ_VNUM(obj) == 19080) {
            fnum = 19090;
        } else if (GET_OBJ_VNUM(obj) == 19081) {
            fnum = 19091;
        } else if (GET_OBJ_VNUM(obj) == 19082) {
            fnum = 19092;
        } else if (GET_OBJ_VNUM(obj) == 19083) {
            fnum = 19093;
        } else if (GET_OBJ_VNUM(obj) == 19085) {
            fnum = 11;
        }
        if (fnum != 0) {
            struct obj_data *furn = read_object(fnum, VIRTUAL);
            act("@CYou click the capsule's button and toss it to the floor. A puff of smoke erupts immediately and quickly dissipates to reveal, $p@C.@n",
                true, ch, furn, nullptr, TO_CHAR);
            act("@c$n@C clicks a capsule's button and tosses it to the floor. A puff of smoke erupts immediately and quickly dissipates to reveal, $p@C.@n",
                true, ch, furn, nullptr, TO_ROOM);
            obj_to_room(furn, IN_ROOM(ch));
            extract_obj(obj);
            return;
        } else {
            send_to_imm("ERROR: Furniture failed to deploy at %d.", GET_ROOM_VNUM(IN_ROOM(ch)));
            return;
        }
    }

    int rnum = 18800, giveup = false, cont = false, found = false, type = 0;

    if (GET_OBJ_VNUM(obj) == 4) {
        type = 0;
    } else if (GET_OBJ_VNUM(obj) == 5) {
        rnum = 18900;
        type = 1;
    } else if (GET_OBJ_VNUM(obj) == 6) {
        rnum = 19100;
        type = 2;
    }

    int final = rnum + 99;

    while (giveup == false && cont == false) {
        for (obj3 = world[real_room(rnum)].contents; obj3; obj3 = next_obj) {
            next_obj = obj3->next_content;
            if (GET_OBJ_VNUM(obj3) == 18801) {
                found = true;
            }
        }
        if (found == true && rnum < final) {
            if (type == 0) {
                rnum += 4;
            } else {
                rnum += 5;
            }
            found = false;
        } else if (rnum >= final) {
            giveup = true;
        } else {
            cont = true;
        }
    } /* End while */

    if (cont == true) {
        int hnum = GET_ROOM_VNUM(IN_ROOM(ch));
        struct obj_data *door = read_object(18801, VIRTUAL);

        GET_OBJ_VAL(door, 6) = GET_ROOM_VNUM(IN_ROOM(ch));
        if (rnum != 18800)
            GET_OBJ_VAL(door, 0) = rnum + 1;
        else
            GET_OBJ_VAL(door, 0) = 18802;
        GET_OBJ_VAL(door, 2) = rnum;
        obj_to_room(door, real_room(rnum));
        struct obj_data *key = read_object(rnum, VIRTUAL);
        obj_to_char(key, ch);
        act("@WYou click the capsule and toss it to the ground. A large cloud of smoke erupts from the capsule and after it clears a house is visible in its place!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n@W clicks a capsule and then tosses it to the ground. A large cloud of smoke erupts from the capsule and after it clears a house is visible in its place!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        struct obj_data *foun = read_object(18803, VIRTUAL);
        obj_to_room(foun, real_room(rnum + 1));
        extract_obj(obj);
    } else {
        send_to_char(ch,
                     "@ROOC@D: @wSorry for the inconvenience, but it appears there are no houses available. Please contact Iovan.@n\r\n");
        return;
    }

}

ACMD(do_twohand) {

    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are busy grappling with someone!\r\n");
        return;
    }

    if (ABSORBING(ch) || ABSORBBY(ch)) {
        send_to_char(ch, "You are busy struggling with someone!\r\n");
        return;
    }

    if (!GET_EQ(ch, WEAR_WIELD1) && !PLR_FLAGGED(ch, PLR_THANDW)) {
        send_to_char(ch, "You need to wield a sword to use this.\r\n");
        return;
    } else if (GET_EQ(ch, WEAR_WIELD2) && !PLR_FLAGGED(ch, PLR_THANDW)) {
        send_to_char(ch, "You have something in your offhand already and can't two hand wield your main weapon.\r\n");
        return;
    } else if ((GET_LIMBCOND(ch, 0) <= 0 || GET_LIMBCOND(ch, 1) <= 0) && !PLR_FLAGGED(ch, PLR_THANDW)) {
        send_to_char(ch, "Kind of hard with only one arm...\r\n");
        return;
    } else if (PLR_FLAGGED(ch, PLR_THANDW)) {
        send_to_char(ch, "You stop wielding your weapon with both hands.\r\n");
        act("$n stops wielding $s weapon with both hands.", true, ch, nullptr, nullptr, TO_ROOM);
        ch->playerFlags.reset(PLR_THANDW);
        return;
    } else {
        send_to_char(ch, "You grab your weapon with both hands.\r\n");
        act("$n starts wielding $s weapon with both hands.", true, ch, nullptr, nullptr, TO_ROOM);
        ch->playerFlags.set(PLR_THANDW);
        return;
    }
}

static void start_auction(struct char_data *ch, struct obj_data *obj, int bid) {
    /* Take object from character and set variables */

    obj_from_char(obj);
    obj_selling = obj;
    ch_selling = ch;
    ch_buying = nullptr;
    curbid = bid;

    /* Tell th character where his item went */
    sprintf(buf, "%s magicly flies away from your hands to be auctioned!\r\n", obj_selling->short_description);
    CAP(buf);
    send_to_char(ch_selling, buf);

    /* Anounce the item is being sold */
    sprintf(buf, auctioneer[AUC_NULL_STATE], curbid);
    auc_send_to_all(buf, false);

    aucstat = AUC_OFFERING;
}

void check_auction(uint64_t heartPulse, double deltaTime) {
    switch (aucstat) {
        case AUC_NULL_STATE:
            return;
        case AUC_OFFERING: {
            if (obj_selling == nullptr) {
                auc_send_to_all(
                        "@RThe auction has stopped because someone has made off with the auctioned object!@n\r\n",
                        false);
                curbid = 0;
                ch_selling = nullptr;
                ch_buying = nullptr;
                aucstat = AUC_NULL_STATE;
                return;
            }
            sprintf(buf, auctioneer[AUC_OFFERING], curbid);
            CAP(buf);
            auc_send_to_all(buf, false);
            aucstat = AUC_GOING_ONCE;
            return;
        }
        case AUC_GOING_ONCE: {
            if (obj_selling == nullptr) {
                auc_send_to_all(
                        "@RThe auction has stopped because someone has made off with the auctioned object!@n\r\n",
                        false);
                curbid = 0;
                ch_selling = nullptr;
                ch_buying = nullptr;
                aucstat = AUC_NULL_STATE;
                return;
            }

            sprintf(buf, auctioneer[AUC_GOING_ONCE], curbid);
            CAP(buf);
            auc_send_to_all(buf, false);
            aucstat = AUC_GOING_TWICE;
            return;
        }
        case AUC_GOING_TWICE: {
            if (obj_selling == nullptr) {
                auc_send_to_all(
                        "@RThe auction has stopped because someone has made off with the auctioned object!@n\r\n",
                        false);
                curbid = 0;
                ch_selling = nullptr;
                ch_buying = nullptr;
                aucstat = AUC_NULL_STATE;
                return;
            }

            sprintf(buf, auctioneer[AUC_GOING_TWICE], curbid);
            CAP(buf);
            auc_send_to_all(buf, false);
            aucstat = AUC_LAST_CALL;
            return;
        }
        case AUC_LAST_CALL: {
            if (obj_selling == nullptr) {
                auc_send_to_all(
                        "@RThe auction has stopped because someone has made off with the auctioned object!@n\r\n",
                        false);
                curbid = 0;
                ch_selling = nullptr;
                ch_buying = nullptr;
                aucstat = AUC_NULL_STATE;
                return;
            }
            if (ch_buying == nullptr) {

                sprintf(buf, auctioneer[AUC_LAST_CALL], curbid);

                CAP(buf);
                auc_send_to_all(buf, false);

                sprintf(buf, "%s flies out the sky and into your hands.\r\n", obj_selling->short_description);
                CAP(buf);
                send_to_char(ch_selling, buf);
                obj_to_char(obj_selling, ch_selling);

                /* Reset auctioning values */
                obj_selling = nullptr;
                ch_selling = nullptr;
                ch_buying = nullptr;
                curbid = 0;
                aucstat = AUC_NULL_STATE;
                return;
            } else {

                sprintf(buf, auctioneer[AUC_SOLD], curbid);
                auc_send_to_all(buf, true);

                /* Give the object to the buyer */
                obj_to_char(obj_selling, ch_buying);
                sprintf(buf, "%s flies out the sky and into your hands, what a steal!\r\n",
                        obj_selling->short_description);
                CAP(buf);
                send_to_char(ch_buying, buf);

                sprintf(buf, "Congrats! You have sold %s for @Y%d@W zenni!\r\n", obj_selling->short_description,
                        curbid);
                send_to_char(ch_selling, buf);

                /* Give selling char the money for his stuff */
                if (GET_GOLD(ch_selling) + curbid > GOLD_CARRY(ch_selling)) {
                    send_to_char(ch_buying,
                                 "You couldn't hold all the zenni, so some of it was deposited for you.\r\n");
                    int diff = 0;
                    diff = (GET_GOLD(ch_selling) + curbid) - GOLD_CARRY(ch_selling);
                    ch_selling->set(CharMoney::Carried, GOLD_CARRY(ch_selling));
                    ch_selling->mod(CharMoney::Bank, diff);
                } else if (GET_GOLD(ch_selling) + curbid <= GOLD_CARRY(ch_selling)) {
                    ch_selling->mod(CharMoney::Carried, curbid);
                }
                /* Reset auctioning values */
                obj_selling = nullptr;
                ch_selling = nullptr;
                ch_buying = nullptr;
                curbid = 0;
                aucstat = AUC_NULL_STATE;
                return;
            }

        }
    }
}

void dball_load(uint64_t heartPulse, double deltaTime) {
    int found1 = false, found2 = false, found3 = false;
    int found4 = false, found5 = false, load = false, num = -1;
    int found6 = false, found7 = false, room = 0, loaded = false;
    int hunter1 = false, hunter2 = false;
    struct obj_data *k;

    if (SELFISHMETER >= 10) {
        return;
    }

    if (dballtime == 0) {
        struct char_data *hunter = nullptr;
        mob_rnum r_num;

        WISHTIME = 0;
        for (k = object_list; k; k = k->next) {
            if (OBJ_FLAGGED(k, ITEM_FORGED)) {
                continue;
            }
            if (GET_OBJ_VNUM(k) == 20) {
                found1 = true;
            } else if (GET_OBJ_VNUM(k) == 21) {
                found2 = true;
            } else if (GET_OBJ_VNUM(k) == 22) {
                found3 = true;
            } else if (GET_OBJ_VNUM(k) == 23) {
                found4 = true;
            } else if (GET_OBJ_VNUM(k) == 24) {
                found5 = true;
            } else if (GET_OBJ_VNUM(k) == 25) {
                found6 = true;
            } else if (GET_OBJ_VNUM(k) == 26) {
                found7 = true;
            } else if (IN_ROOM(k) != NOWHERE && ROOM_EFFECT(IN_ROOM(k)) == 6 && !OBJ_FLAGGED(k, ITEM_UNBREAKABLE)) {
                send_to_room(IN_ROOM(k), "@R%s@r melts in the lava!@n\r\n", k->short_description);
                extract_obj(k);
            } else {
                continue;
            }
        }
        if (found1 == false) {
            load = false;
            int zone = 0;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if ((zone = real_zone_by_thing(real_room(num))) != NOWHERE) {
                        if (ZONE_FLAGGED(zone, ZONE_DBALLS)) {
                            room = num;
                            load = true;
                            num = rand_number(200, 20000);
                        } else {
                            num = rand_number(200, 20000);
                        }
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(200, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(20, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(20, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(20, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(20, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        if (found2 == false) {
            load = false;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if (ROOM_FLAGGED(real_room(num), ROOM_EARTH) || ROOM_FLAGGED(real_room(num), ROOM_VEGETA) ||
                        ROOM_FLAGGED(real_room(num), ROOM_FRIGID) || ROOM_FLAGGED(real_room(num), ROOM_AETHER) ||
                        ROOM_FLAGGED(real_room(num), ROOM_NAMEK) || ROOM_FLAGGED(real_room(num), ROOM_KONACK) ||
                        ROOM_FLAGGED(real_room(num), ROOM_YARDRAT)) {
                        room = num;
                        load = true;
                        num = rand_number(200, 20000);
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(20, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(21, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(21, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(21, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(21, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        if (found3 == false) {
            load = false;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if (ROOM_FLAGGED(real_room(num), ROOM_EARTH) || ROOM_FLAGGED(real_room(num), ROOM_VEGETA) ||
                        ROOM_FLAGGED(real_room(num), ROOM_FRIGID) || ROOM_FLAGGED(real_room(num), ROOM_AETHER) ||
                        ROOM_FLAGGED(real_room(num), ROOM_NAMEK) || ROOM_FLAGGED(real_room(num), ROOM_KONACK) ||
                        ROOM_FLAGGED(real_room(num), ROOM_YARDRAT)) {
                        room = num;
                        load = true;
                        num = rand_number(200, 20000);
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(20, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(22, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(22, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(22, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(22, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        if (found4 == false) {
            load = false;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if (ROOM_FLAGGED(real_room(num), ROOM_EARTH) || ROOM_FLAGGED(real_room(num), ROOM_VEGETA) ||
                        ROOM_FLAGGED(real_room(num), ROOM_FRIGID) || ROOM_FLAGGED(real_room(num), ROOM_AETHER) ||
                        ROOM_FLAGGED(real_room(num), ROOM_NAMEK) || ROOM_FLAGGED(real_room(num), ROOM_KONACK) ||
                        ROOM_FLAGGED(real_room(num), ROOM_YARDRAT)) {
                        room = num;
                        load = true;
                        num = rand_number(200, 20000);
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(20, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(23, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(23, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(23, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(23, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        if (found5 == false) {
            load = false;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if (ROOM_FLAGGED(real_room(num), ROOM_EARTH) || ROOM_FLAGGED(real_room(num), ROOM_VEGETA) ||
                        ROOM_FLAGGED(real_room(num), ROOM_FRIGID) || ROOM_FLAGGED(real_room(num), ROOM_AETHER) ||
                        ROOM_FLAGGED(real_room(num), ROOM_NAMEK) || ROOM_FLAGGED(real_room(num), ROOM_KONACK) ||
                        ROOM_FLAGGED(real_room(num), ROOM_YARDRAT)) {
                        room = num;
                        load = true;
                        num = rand_number(200, 20000);
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(20, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(24, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(24, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(24, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(24, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        if (found6 == false) {
            load = false;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if (ROOM_FLAGGED(real_room(num), ROOM_EARTH) || ROOM_FLAGGED(real_room(num), ROOM_VEGETA) ||
                        ROOM_FLAGGED(real_room(num), ROOM_FRIGID) || ROOM_FLAGGED(real_room(num), ROOM_AETHER) ||
                        ROOM_FLAGGED(real_room(num), ROOM_NAMEK) || ROOM_FLAGGED(real_room(num), ROOM_KONACK) ||
                        ROOM_FLAGGED(real_room(num), ROOM_YARDRAT)) {
                        room = num;
                        load = true;
                        num = rand_number(200, 20000);
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(20, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(25, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(25, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(25, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(25, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        if (found7 == false) {
            load = false;
            while (load == false) {
                if (real_room(num) != NOWHERE) {
                    if (ROOM_FLAGGED(real_room(num), ROOM_EARTH) || ROOM_FLAGGED(real_room(num), ROOM_VEGETA) ||
                        ROOM_FLAGGED(real_room(num), ROOM_FRIGID) || ROOM_FLAGGED(real_room(num), ROOM_AETHER) ||
                        ROOM_FLAGGED(real_room(num), ROOM_NAMEK) || ROOM_FLAGGED(real_room(num), ROOM_KONACK) ||
                        ROOM_FLAGGED(real_room(num), ROOM_YARDRAT)) {
                        room = num;
                        load = true;
                        num = rand_number(200, 20000);
                    } else {
                        num = rand_number(200, 20000);
                    }
                } else {
                    num = rand_number(20, 20000);
                }
            }
            if (rand_number(1, 10) > 8) {
                if (hunter1 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER1_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter1 = true;
                    DBALL_HUNTER1 = room;
                    k = read_object(26, VIRTUAL);
                    obj_to_char(k, hunter);
                } else if (hunter2 == false) {
                    if ((r_num = real_mobile(DBALL_HUNTER2_VNUM)) == NOBODY) {
                        return;
                    }
                    hunter = read_mobile(r_num, REAL);
                    char_to_room(hunter, real_room(room));
                    hunter2 = true;
                    DBALL_HUNTER2 = room;
                    k = read_object(26, VIRTUAL);
                    obj_to_char(k, hunter);
                } else {
                    k = read_object(26, VIRTUAL);
                    obj_to_room(k, real_room(room));
                }
            } else {
                k = read_object(26, VIRTUAL);
                obj_to_room(k, real_room(room));
            }
            loaded = true;
        }
        dballtime = 604800;
    } else if (dballtime == 518400 || dballtime == 432000 || dballtime == 345600 || dballtime == 259200 ||
               dballtime == 172800 || dballtime == 86400) {
        dballtime -= 1;
    } else {
        if (WISHTIME == 0) {
            WISHTIME = dballtime - 1;
        } else if (WISHTIME > 0 && dballtime != WISHTIME) {
            dballtime = WISHTIME;
        }
        WISHTIME -= 1;
        dballtime -= 1;
    }
}

ACMD(do_auction) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int bid = 0;

    two_arguments(argument, arg1, arg2);

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HBTC)) {
        send_to_char(ch, "This is a different dimension!\r\n");
        return;
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PAST)) {
        send_to_char(ch, "You are in the past!\r\n");
        return;
    }
    if (PRF_FLAGGED(ch, PRF_HIDE)) {
        send_to_char(ch, "The auctioneer will not accept items from hidden people.\r\n");
        return;
    }

    if (!*arg1) {
        send_to_char(ch, "Auction what?\r\n");
        send_to_char(ch, "[ Auction: <item> | <cancel> ]\r\n");
        return;
    } else if (is_abbrev(arg1, "cancel") || is_abbrev(arg1, "stop")) {
        if ((ch != ch_selling && GET_ADMLEVEL(ch) <= ADMLVL_GRGOD) || aucstat == AUC_NULL_STATE) {
            send_to_char(ch, "You're not even selling anything!\r\n");
            return;
        } else if (ch == ch_selling) {
            stop_auction(AUC_NORMAL_CANCEL, nullptr);
            return;
        } else {
            stop_auction(AUC_WIZ_CANCEL, ch);
        }
    } else if (is_abbrev(arg1, "stats") || is_abbrev(arg1, "identify")) {
        auc_stat(ch, obj_selling);
        return;
    } else if (!(obj = get_obj_in_list_vis(ch, arg1, nullptr, ch->contents))) {
        sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
        send_to_char(ch, buf);
        return;
    } else if (!*arg2) {
        sprintf(buf, "What should be the minimum bid?\r\n");
        send_to_char(ch, buf);
        return;
    } else if (*arg2 && (bid = atoi(arg2)) <= 0) {
        send_to_char(ch, "Come on? One zenni at least?\r\n");
        return;
    } else if (aucstat != AUC_NULL_STATE) {
        sprintf(buf, "Sorry, but %s is already auctioning %s at @Y%d@W zenni!\r\n", GET_NAME(ch_selling),
                obj_selling->short_description, bid);
        send_to_char(ch, buf);
        return;
    } else if (OBJ_FLAGGED(obj, ITEM_NOSELL)) {
        send_to_char(ch, "Sorry but you can't sell that!\r\n");
        return;
    } else if (GET_OBJ_VAL(obj, VAL_CONTAINER_CORPSE) == 1) {
        send_to_char(ch, "Sorry but you can't sell that!\r\n");
        return;
    } else {
        send_to_char(ch, "Ok.\r\n");
        start_auction(ch, obj, bid);
        return;
    }
}

ACMD(do_bid) {
    struct obj_data *obj, *next_obj, *obj2 = nullptr;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int found = false, list = 0, masterList = 0;
    room_vnum auct_room;

    auct_room = real_room(80);

    if (IS_NPC(ch))
        return;

    if (!GET_EQ(ch, WEAR_EYE)) {
        send_to_char(ch, "You need a scouter to make an auction bid.\r\n");
        return;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HBTC)) {
        send_to_char(ch, "This is a different dimension!\r\n");
        return;
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PAST)) {
        send_to_char(ch, "This is the past, nothing is being auctioned!\r\n");
        return;
    }

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "Syntax: bid [ list | # ] (amt)\r\nOr...\r\nSyntax: bid appraise (list number)\r\n");
        return;
    }
    for (obj = world[auct_room].contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj) {
            list++;
        }
    }
    masterList = list;
    list = 0;

    if (!strcasecmp(arg, "list")) {
        send_to_char(ch, "@Y                                   Auction@n\r\n");
        send_to_char(ch, "@c------------------------------------------------------------------------------@n\r\n");
        for (obj = world[auct_room].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (obj) {
                if (GET_AUCTER(obj) <= 0) {
                    continue;
                }
                list++;
                if (GET_AUCTIME(obj) + 86400 > time(nullptr) && GET_CURBID(obj) <= -1) {
                    send_to_char(ch,
                                 "@D[@R#@W%3d@D][@mOwner@W: @w%10s@D][@GItem Name@W: @w%-*s@D][@GCost@W: @Y%s@D]@n\r\n",
                                 list, get_name_by_id(GET_AUCTER(obj)) != nullptr ? CAP(get_name_by_id(GET_AUCTER(obj)))
                                                                                  : "Nobody",
                                 count_color_chars(obj->short_description) + 30, obj->short_description,
                                 add_commas(GET_BID(obj)).c_str());
                } else if (GET_AUCTIME(obj) + 86400 > time(nullptr) && GET_CURBID(obj) > -1) {
                    send_to_char(ch,
                                 "@D[@R#@W%3d@D][@mOwner@W: @w%10s@D][@GItem Name@W: @w%-*s@D][@RTop Bid@W: %s @Y%s@D]@n\r\n",
                                 list, get_name_by_id(GET_AUCTER(obj)) != nullptr ? CAP(get_name_by_id(GET_AUCTER(obj)))
                                                                                  : "Nobody",
                                 count_color_chars(obj->short_description) + 30, obj->short_description,
                                 get_name_by_id(GET_CURBID(obj)) != nullptr ? CAP(get_name_by_id(GET_CURBID(obj)))
                                                                            : "Nobody", add_commas(GET_BID(obj)).c_str());
                } else if (GET_AUCTIME(obj) + 86400 < time(nullptr) && GET_CURBID(obj) > -1) {
                    send_to_char(ch,
                                 "@D[@R#@W%3d@D][@mOwner@W: @w%10s@D][@GItem Name@W: @w%-*s@D][@RBid Winner@W: %s @Y%s@D]@n\r\n",
                                 list, get_name_by_id(GET_AUCTER(obj)) != nullptr ? CAP(get_name_by_id(GET_AUCTER(obj)))
                                                                                  : "Nobody",
                                 count_color_chars(obj->short_description) + 30, obj->short_description,
                                 get_name_by_id(GET_CURBID(obj)) != nullptr ? CAP(get_name_by_id(GET_CURBID(obj)))
                                                                            : "Nobody", add_commas(GET_BID(obj)).c_str());
                } else {
                    send_to_char(ch, "@D[@R#@W%3d@D][@mOwner@W: @w%10s@D][@GItem Name@W: @w%-*s@D][@RClosed@D]@n\r\n",
                                 list, get_name_by_id(GET_AUCTER(obj)) != nullptr ? CAP(get_name_by_id(GET_AUCTER(obj)))
                                                                                  : "Nobody",
                                 count_color_chars(obj->short_description) + 30, obj->short_description);
                }
                found = true;
            }
        }
        if (found == false) {
            send_to_char(ch, "No items are currently being auctioned.\r\n");
        }
        send_to_char(ch, "@c------------------------------------------------------------------------------@n\r\n");
    } else if (!strcasecmp(arg, "appraise")) {
        if (!*arg2) {
            send_to_char(ch, "Syntax: bid [ list | # ] (amt)\r\nOr...\r\nSyntax: bid appraise (list number)\r\n");
            send_to_char(ch, "What item number did you want to appraise?\r\n");
            return;
        } else if (atoi(arg2) < 0 || atoi(arg2) > masterList) {
            send_to_char(ch, "Syntax: bid [ list | # ] (amt)\r\nOr...\r\nSyntax: bid appraise (list number)\r\n");
            send_to_char(ch, "That item number doesn't exist.\r\n");
            return;
        }

        for (obj = world[auct_room].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (obj) {
                if (GET_AUCTER(obj) <= 0) {
                    continue;
                }
                list++;
                if (atoi(arg2) == list) {
                    obj2 = obj;
                }
            }
        }
        if (!obj2) {
            send_to_char(ch, "That item number is not found.\r\n");
            return;
        } else {
            if (!GET_SKILL(ch, SKILL_APPRAISE)) {
                send_to_char(ch, "You are unskilled at appraising.\r\n");
                return;
            }
            improve_skill(ch, SKILL_APPRAISE, 1);
            if (GET_SKILL(ch, SKILL_APPRAISE) < rand_number(1, 101)) {
                send_to_char(ch, "You look at the images for %s and fail to perceive its worth..\r\n",
                             obj2->short_description);
                act("@c$n@w looks stumped about something they viewed on their scouter screen.@n", true, ch, nullptr,
                    nullptr, TO_ROOM);
                WAIT_STATE(ch, PULSE_2SEC);
                return;
            } else {
                send_to_char(ch, "You look at images of the object on your scouter.\r\n");
                act("@c$n@w looks at something on their scouter screen.@n", true, ch, nullptr, nullptr, TO_ROOM);
                send_to_char(ch, "@c------------------------------------------------------------------------\n");
                send_to_char(ch, "@GOwner       @W: @w%s@n\n",
                             get_name_by_id(GET_AUCTER(obj2)) != nullptr ? CAP(get_name_by_id(GET_AUCTER(obj2)))
                                                                         : "Nobody");
                send_to_char(ch, "@GItem Name   @W: @w%s@n\n", obj2->short_description);
                send_to_char(ch, "@GCurrent Bid @W: @Y%s@n\n", add_commas(GET_BID(obj2)).c_str());
                send_to_char(ch, "@GStore Value @W: @Y%s@n\n", add_commas(GET_OBJ_COST(obj2)).c_str());
                send_to_char(ch, "@GItem Min LVL@W: @w%d@n\n", GET_OBJ_LEVEL(obj2));
                if (GET_OBJ_VAL(obj2, VAL_ALL_HEALTH) >= 100) {
                    send_to_char(ch, "@GCondition   @W: @C%d%s@n\n", GET_OBJ_VAL(obj2, VAL_ALL_HEALTH), "%");
                } else if (GET_OBJ_VAL(obj2, VAL_ALL_HEALTH) >= 50) {
                    send_to_char(ch, "@GCondition   @W: @y%d%s@n\n", GET_OBJ_VAL(obj2, VAL_ALL_HEALTH), "%");
                } else if (GET_OBJ_VAL(obj2, VAL_ALL_HEALTH) >= 1) {
                    send_to_char(ch, "@GCondition   @W: @r%d%s@n\n", GET_OBJ_VAL(obj2, VAL_ALL_HEALTH), "%");
                } else {
                    send_to_char(ch, "@GCondition   @W: @D%d%s@n\n", GET_OBJ_VAL(obj2, VAL_ALL_HEALTH), "%");
                }
                send_to_char(ch, "@GItem Weight @W: @w%s@n\n", add_commas(GET_OBJ_WEIGHT(obj2)).c_str());
                char bits[MAX_STRING_LENGTH];
                sprintbitarray(GET_OBJ_WEAR(obj2), wear_bits, TW_ARRAY_MAX, bits);
                search_replace(bits, "TAKE", "");
                send_to_char(ch, "@GWear Loc.   @W:@w%s\n", bits);
                if (GET_OBJ_TYPE(obj2) == ITEM_WEAPON) {
                    if (OBJ_FLAGGED(obj2, ITEM_WEAPLVL1)) {
                        send_to_char(ch, "@GWeapon Level@W: @D[@C1@D]\n@GDamage Bonus@W: @D[@w5%s@D]@n\r\n", "%");
                    } else if (OBJ_FLAGGED(obj2, ITEM_WEAPLVL2)) {
                        send_to_char(ch, "@GWeapon Level@W: @D[@C1@D]\n@GDamage Bonus@W: @D[@w10%s@D]@n\r\n", "%");
                    } else if (OBJ_FLAGGED(obj2, ITEM_WEAPLVL3)) {
                        send_to_char(ch, "@GWeapon Level@W: @D[@C1@D]\n@GDamage Bonus@W: @D[@w20%s@D]@n\r\n", "%");
                    } else if (OBJ_FLAGGED(obj2, ITEM_WEAPLVL4)) {
                        send_to_char(ch, "@GWeapon Level@W: @D[@C1@D]\n@GDamage Bonus@W: @D[@w30%s@D]@n\r\n", "%");
                    } else if (OBJ_FLAGGED(obj2, ITEM_WEAPLVL5)) {
                        send_to_char(ch, "@GWeapon Level@W: @D[@C1@D]\n@GDamage Bonus@W: @D[@w50%s@D]@n\r\n", "%");
                    }
                }
                int i, found = false;
                send_to_char(ch, "@GItem Size   @W:@w %s@n\r\n", size_names[GET_OBJ_SIZE(obj2)]);
                send_to_char(ch, "@GItem Bonuses@W:@w");
                for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                    if (obj2->affected[i].modifier) {
                        sprinttype(obj2->affected[i].location, apply_types, buf, sizeof(buf));
                        send_to_char(ch, "%s %+d to %s", found++ ? "," : "", obj2->affected[i].modifier, buf);
                        switch (obj2->affected[i].location) {
                            case APPLY_FEAT:
                                send_to_char(ch, " (%s)", feat_list[obj2->affected[i].specific].name);
                                break;
                            case APPLY_SKILL:
                                send_to_char(ch, " (%s)", spell_info[obj2->affected[i].specific].name);
                                break;
                        }
                    }
                }
                if (!found)
                    send_to_char(ch, " None@n");
                else
                    send_to_char(ch, "@n");
                char buf2[MAX_STRING_LENGTH];
                sprintbitarray(GET_OBJ_PERM(obj2), affected_bits, AF_ARRAY_MAX, buf2);
                send_to_char(ch, "\n@GSpecial     @W:@w %s\n", buf2);
                send_to_char(ch, "@c------------------------------------------------------------------------\n");
                return;
            }
        }
    } else {
        if (!*arg2) {
            send_to_char(ch, "Syntax: bid [ list | # ] (amt)\r\nOr...\r\nSyntax: bid appraise (list number)\r\n");
            send_to_char(ch, "What amount did you want to bid?\r\n");
            return;
        } else if (atoi(arg) < 0 || atoi(arg) > masterList) {
            send_to_char(ch, "Syntax: bid [ list | # ] (amt)\r\nOr...\r\nSyntax: bid appraise (list number)\r\n");
            send_to_char(ch, "That item number is not found.\r\n");
            return;
        }

        for (obj = world[auct_room].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (obj) {
                if (GET_AUCTER(obj) <= 0) {
                    continue;
                }
                list++;
                if (atoi(arg) == list) {
                    obj2 = obj;
                }
            }
        }

        if (!obj2) {
            send_to_char(ch, "That item number is not found.\r\n");
            return;
        } else if (GET_CURBID(obj2) == ((ch)->id)) {
            send_to_char(ch, "You already have the highest bid.\r\n");
            return;
        } else if (GET_AUCTER(obj2) == ((ch)->id)) {
            send_to_char(ch, "You auctioned the item, go to the auction house and cancel if you can.\r\n");
            return;
        } else if (GET_CURBID(obj2) > 0 && atoi(arg2) <= (GET_BID(obj2) + (GET_BID(obj2) * .1)) &&
                   GET_CURBID(obj2) > -1) {
            send_to_char(ch, "You have to bid at least 10 percent over the current bid.\r\n");
            return;
        } else if (atoi(arg2) < GET_BID(obj2) && GET_CURBID(obj2) <= -1) {
            send_to_char(ch, "You have to bid at least the starting bid.\r\n");
            return;
        } else if (atoi(arg2) >
                   (((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / 100) * 50) + (GET_GOLD(ch) + GET_BANK_GOLD(ch))) {
            send_to_char(ch, "You can not bid more than 150%s of your total money (on hand and in the bank).\r\n", "%");
            return;
        } else if (GET_AUCTIME(obj2) + 86400 <= time(nullptr)) {
            send_to_char(ch, "Bidding on that object has been closed.\r\n");
            return;
        } else {
            GET_BID(obj2) = atoi(arg2);
            GET_CURBID(obj2) = ((ch)->id);
            auc_save();
            struct descriptor_data *d;
            int bid = atoi(arg2);
            basic_mud_log("AUCTION: %s has bid %s on %s", GET_NAME(ch), obj2->short_description, add_commas(bid).c_str());
            for (d = descriptor_list; d; d = d->next) {
                if (STATE(d) != CON_PLAYING || IS_NPC(d->character))
                    continue;
                if (d->character == ch) {
                    if (GET_EQ(d->character, WEAR_EYE)) {
                        send_to_char(d->character, "@RScouter Auction News@D: @GYou have bid @Y%s@G on @w%s@G@n\r\n",
                                     add_commas(GET_BID(obj2)).c_str(), obj2->short_description);
                    }
                    continue;
                }
                if (GET_EQ(d->character, WEAR_EYE)) {
                    send_to_char(d->character,
                                 "@RScouter Auction News@D: @GThe bid on, @w%s@G, has been raised to @Y%s@n\r\n",
                                 obj2->short_description, add_commas(GET_BID(obj2)).c_str());
                }
            }
        }

    }

}

void stop_auction(int type, struct char_data *ch) {
    if (obj_selling == nullptr) {
        auc_send_to_all("@RThe auction has stopped because someone has made off with the auctioned object!@n\r\n",
                        false);
        curbid = 0;
        ch_selling = nullptr;
        ch_buying = nullptr;
        aucstat = AUC_NULL_STATE;
        return;
    }
    switch (type) {

        case AUC_NORMAL_CANCEL: {

            sprintf(buf, auctioneer[AUC_NORMAL_CANCEL]);
            auc_send_to_all(buf, false);
            break;
        }
        case AUC_QUIT_CANCEL: {

            sprintf(buf, auctioneer[AUC_QUIT_CANCEL]);
            auc_send_to_all(buf, false);
            break;
        }
        case AUC_WIZ_CANCEL: {

            sprintf(buf, auctioneer[AUC_WIZ_CANCEL]);
            auc_send_to_all(buf, false);
            break;
        }
        default: {
            send_to_char(ch, "Sorry, that is an unrecognised cancel command, please report.");
            return;
        }
    }


    if (type != AUC_WIZ_CANCEL) {
        sprintf(buf, "%s flies out the sky and into your hands.\r\n", obj_selling->short_description);
        CAP(buf);
        send_to_char(ch_selling, buf);
        obj_to_char(obj_selling, ch_selling);
    } else {
        sprintf(buf, "%s flies out the sky and into your hands.\r\n", obj_selling->short_description);
        CAP(buf);
        send_to_char(ch, buf);
        obj_to_char(obj_selling, ch);
    }


    if (!(ch_buying == nullptr))
        ch_buying->mod(CharMoney::Carried, curbid);

    obj_selling = nullptr;
    ch_selling = nullptr;
    ch_buying = nullptr;
    curbid = 0;

    aucstat = AUC_NULL_STATE;

}

static void auc_stat(struct char_data *ch, struct obj_data *obj) {

    if (aucstat == AUC_NULL_STATE) {
        send_to_char(ch, "Nothing is being auctioned!\r\n");
        return;
    } else if (ch == ch_selling) {
        send_to_char(ch, "You should have found that out BEFORE auctioning it!\r\n");
        return;
    } else if (GET_GOLD(ch) < 500) {
        send_to_char(ch, "You can't afford to find the stats on that, it costs 500 zenni!\r\n");
        return;
    } else {
        /* auctioneer tells the character the auction details */
        sprintf(buf, auctioneer[AUC_STAT], curbid);
        act(buf, true, ch_selling, obj, ch, TO_VICT | TO_SLEEP);
        ch->mod(CharMoney::Carried, -500);

        /*call_magic(ch, nullptr, obj_selling, SPELL_IDENTIFY, 30, CAST_SPELL);*/
    }
}

static void auc_send_to_all(char *messg, bool buyer) {
    struct descriptor_data *i;

    if (messg == nullptr)
        return;

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING)
            continue;
        if (ROOM_FLAGGED(IN_ROOM(i->character), ROOM_HBTC))
            continue;
        if (ROOM_FLAGGED(IN_ROOM(i->character), ROOM_PAST))
            continue;
        if (buyer)
            act(messg, true, ch_buying, obj_selling, i->character, TO_VICT | TO_SLEEP);
        else
            act(messg, true, ch_selling, obj_selling, i->character, TO_VICT | TO_SLEEP);

    }
}


ACMD(do_assemble) {
    long lVnum = NOTHING;
    struct obj_data *pObject = nullptr;
    char buf[MAX_STRING_LENGTH];

    int roll = 0;

    skip_spaces(&argument);

    struct obj_data *tools = nullptr, *tool = nullptr, *next_obj;

    for (tools = ch->contents; tools; tools = next_obj) {
        next_obj = tools->next_content;
        if (GET_OBJ_VNUM(tools) == 386 && GET_OBJ_VAL(tools, VAL_ALL_HEALTH) > 0) {
            tool = tools;
            act("@WYou open up your toolkit and take out the necessary tools.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@W opens up $s toolkit and takes out the necessary tools.@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    }

    if (*argument == '\0') {
        send_to_char(ch, "What would you like to %s?\r\n", CMD_NAME);
        return;
    } else if ((lVnum = assemblyFindAssembly(argument)) < 0) {
        send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
        return;
    } else if (assemblyGetType(lVnum) != subcmd) {
        send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
        return;
    } else if (!assemblyCheckComponents(lVnum, ch, false)) {
        send_to_char(ch, "You haven't got all the things you need.\r\n");
        return;
    } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE)) {
        send_to_char(ch, "You can't do that in space.");
        return;
    } else if (!GET_SKILL(ch, SKILL_SURVIVAL) && !strcasecmp(argument, "campfire")) {
        send_to_char(ch, "You know nothing about building campfires.\r\n");
        return;
    }

    if (strstr(argument, "Signal") || strstr(argument, "signal")) {
        if (GET_SKILL(ch, SKILL_BUILD) < 70) {
            send_to_char(ch, "You need at least a build skill level of 70.\r\n");
            return;
        }
    }

    if (tool == nullptr) {
        send_to_char(ch, "You wish you had tools, but make the best out of what you do have anyway...\r\n");
        roll = 20;
    }

    if (strcasecmp(argument, "campfire")) {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE) || SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM || SUNKEN(IN_ROOM(ch))) {
            send_to_char(ch, "This area will not allow a fire to burn properly.\r\n");
            return;
        }

        if (GET_SKILL(ch, SKILL_SURVIVAL) >= 90) {
            roll += axion_dice(0);
        } else if (GET_SKILL(ch, SKILL_SURVIVAL) < 90) {
            roll += axion_dice(0);
        }
        improve_skill(ch, SKILL_BUILD, 1);
        if (GET_SKILL(ch, SKILL_BUILD) <= roll) {
            if ((pObject = read_object(lVnum, VIRTUAL)) == nullptr) {
                send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
                return;
            }

            extract_obj(pObject);
            send_to_char(ch, "You start to %s %s %s, but mess up royally!\r\n", CMD_NAME, AN(argument), argument);

            if (assemblyCheckComponents(lVnum, ch, true)) {
                roll = 9001; /* Just a place holder */
            }
            return;
        }
    } else {
        if (GET_SKILL(ch, SKILL_SURVIVAL) >= 90) {
            roll += axion_dice(0);
        } else if (GET_SKILL(ch, SKILL_SURVIVAL) < 90) {
            roll += axion_dice(-10);
        }
        improve_skill(ch, SKILL_BUILD, 1);
        if (GET_SKILL(ch, SKILL_SURVIVAL) <= roll) {
            if ((pObject = read_object(lVnum, VIRTUAL)) == nullptr) {
                send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
                return;
            }

            extract_obj(pObject);
            send_to_char(ch, "You start to %s %s %s, but mess up royally!\r\n", CMD_NAME, AN(argument), argument);

            if (tool && rand_number(1, 3) == 3 && GET_OBJ_VAL(tool, VAL_ALL_HEALTH) > 0) {
                GET_OBJ_VAL(tool, VAL_ALL_HEALTH) -= rand_number(1, 5);
                act("@RYour toolset is looking a bit more worn.@n", true, ch, nullptr, nullptr, TO_CHAR);
                if (GET_OBJ_VAL(tool, VAL_ALL_HEALTH) <= 0) {
                    GET_OBJ_VAL(tool, VAL_ALL_HEALTH) = 0;
                }
            }
            if (assemblyCheckComponents(lVnum, ch, true)) {
                roll = 9001; /* Just a place holder */
            }
            return;
        }
    }

    if (axion_dice(0) - (GET_INT(ch) / 5) > 95) {
        if ((pObject = read_object(lVnum, VIRTUAL)) == nullptr) {
            send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
            return;
        }

        extract_obj(pObject);
        send_to_char(ch, "You start to %s %s %s, but forget a couple of steps. You take it apart and give up.\r\n",
                     CMD_NAME, AN(argument), argument);
        WAIT_STATE(ch, PULSE_6SEC);
        return;
    } else if (rand_number(1, 100) >= 92) {
        if ((pObject = read_object(lVnum, VIRTUAL)) == nullptr) {
            send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
            return;
        }

        extract_obj(pObject);
        send_to_char(ch,
                     "You start to %s %s %s, but put it together wrong and have to stop. You take it apart and give up.\r\n",
                     CMD_NAME, AN(argument), argument);
        WAIT_STATE(ch, PULSE_4SEC);
        return;
    }

    /* Create the assembled object. */
    if ((pObject = read_object(lVnum, VIRTUAL)) == nullptr) {
        send_to_char(ch, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
        return;
    }

    /* Now give the object to the character. */
    if (GET_OBJ_VNUM(pObject) != 1611) {
        obj_to_char(pObject, ch);
        if (IS_TRUFFLE(ch)) {
            int count = 0, plused = false, hasstat = 0, failsafe = 0;

            while (count < 6) {
                if (pObject->affected[count].location > 0 && rand_number(1, 6) <= 2 && plused == false) {
                    pObject->affected[count].modifier += rand_number(1, 3);
                    plused = true;
                } else if (pObject->affected[count].location > 0) {
                    hasstat += 1;
                }
                failsafe++;
                count++;
                if (plused == true) {
                    send_to_char(ch, "@YYour intuitive skill with building has made this item even better!@n\r\n");
                    count = 6;
                } else if (failsafe >= 12) {
                    send_to_char(ch, "@yIt seems this item could not be upgraded with your truffle knowledge...@n\r\n");
                    count = 6;
                } else if (failsafe == 11 && plused == false) {
                    send_to_char(ch, "@YYour intuitive skill with building has made this item even better!@n\r\n");
                    pObject->affected[count].location = rand_number(1, 6);
                    pObject->affected[count].modifier += rand_number(1, 3);
                    plused = true;
                    count = 6;
                } else if (count == 6 && hasstat > 0) {
                    count = 0;
                }
            }
        }
    } else {
        obj_to_room(pObject, IN_ROOM(ch));
        GET_OBJ_TIMER(pObject) = (GET_SKILL(ch, SKILL_SURVIVAL) * 0.12);
    }

    /*if (wearable_obj(pObject)) {
   GET_OBJ_SIZE(pObject) = get_size(ch);
  }*/

    /* Tell the character they made something. */
    sprintf(buf, "You %s $p.", CMD_NAME);
    act(buf, false, ch, pObject, nullptr, TO_CHAR);

    /* Tell the room the character made something. */
    sprintf(buf, "$n %ss $p.", CMD_NAME);
    act(buf, false, ch, pObject, nullptr, TO_ROOM);

    if (assemblyCheckComponents(lVnum, ch, true)) {
        roll = 9001; /* Just a place holder */
    }

    if (!IS_TRUFFLE(ch) && axion_dice(8) > GET_SKILL(ch, SKILL_BUILD)) {
        send_to_char(ch, "@yYou've made an inferior product. Its value will be somewhat less.@n\r\n");
        GET_OBJ_COST(pObject) -= GET_OBJ_COST(pObject) * 0.25;
    } else if (IS_TRUFFLE(ch) && axion_dice(18) > GET_SKILL(ch, SKILL_BUILD)) {
        send_to_char(ch, "@yYou've made an inferior product. Its value will be somewhat less.@n\r\n");
        GET_OBJ_COST(pObject) -= GET_OBJ_COST(pObject) * 0.12;
    } else if (IS_TRUFFLE(ch) < GET_SKILL(ch, SKILL_BUILD)) {
        send_to_char(ch, "@YYou've made an excellent product. Its value will be somewhat more.@n\r\n");
        GET_OBJ_COST(pObject) += GET_OBJ_COST(pObject) * 0.12;
    }

    if (IS_TRUFFLE(ch) && rand_number(1, 5) >= 4 && GET_OBJ_COST(pObject) >= 500) {
        if (GET_LEVEL(ch) < 100 && level_exp(ch, GET_LEVEL(ch) + 1) - GET_EXP(ch) > 0) {
            int64_t gain = level_exp(ch, GET_LEVEL(ch) + 1) * 0.011;
            send_to_char(ch, "@RExp Bonus@D: @G%s@n\r\n", add_commas(gain).c_str());
            ch->modExperience(gain);
        } else {
            ch->modExperience(1375000);
            send_to_char(ch, "@RExp Bonus@D: @G%s@n\r\n", add_commas(1375000).c_str());
        }
    }
}

static void perform_put(struct char_data *ch, struct obj_data *obj,
                        struct obj_data *cont) {

    int dball[7] = {20, 21, 22, 23, 24, 25, 26};

    if (!drop_otrigger(obj, ch))
        return;

    if (!obj) /* object might be extracted by drop_otrigger */
        return;
    if (OBJ_FLAGGED(cont, ITEM_FORGED)) {
        act("$P is forged and won't hold anything.", false, ch, nullptr, cont, TO_CHAR);
        return;
    }
    if (OBJ_FLAGGED(cont, ITEM_SHEATH) && GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
        send_to_char(ch, "That is made to only hold weapons.\r\n");
        return;
    }
    if (OBJ_FLAGGED(cont, ITEM_SHEATH)) {
        struct obj_data *obj2 = nullptr, *next_obj = nullptr;
        int count = 0, minus = 0;
        for (obj2 = cont->contents; obj2; obj2 = next_obj) {
            next_obj = obj2->next_content;
            minus += GET_OBJ_WEIGHT(obj2);
            count++;
        }
        obj2 = nullptr;
        int holds = GET_OBJ_WEIGHT(cont) - minus;
        if (count >= holds) {
            send_to_char(ch, "It can only hold %d weapon%s at a time.\r\n", holds, holds > 1 ? "s" : "");
            return;
        }
    }
    if ((GET_OBJ_TYPE(cont) == ITEM_CONTAINER) && (GET_OBJ_VAL(cont, VAL_CONTAINER_CAPACITY) == 0))
        act("$p won't fit in $P.", false, ch, obj, cont, TO_CHAR);
    else if (GET_OBJ_VNUM(cont) >= 600 && GET_OBJ_VNUM(cont) <= 603)
        send_to_char(ch, "You can't put cards on a duel table. You have to @Gplay@n them.\r\n");
    else if ((GET_OBJ_VNUM(cont) == 697 || GET_OBJ_VNUM(cont) == 698 || GET_OBJ_VNUM(cont) == 682 ||
              GET_OBJ_VNUM(cont) == 683 || GET_OBJ_VNUM(cont) == 684 || OBJ_FLAGGED(cont, ITEM_CARDCASE)) &&
             !OBJ_FLAGGED(obj, ITEM_ANTI_HIEROPHANT))
        send_to_char(ch, "You can only put cards in a case.\r\n");
    else if ((GET_OBJ_TYPE(cont) == ITEM_CONTAINER) &&
             (GET_OBJ_VAL(cont, VAL_CONTAINER_CAPACITY) > 0) &&
             (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, VAL_CONTAINER_CAPACITY)))
        act("$p won't fit in $P.", false, ch, obj, cont, TO_CHAR);
    else if (OBJ_FLAGGED(obj, ITEM_NODROP) && IN_ROOM(cont) != NOWHERE)
        act("You can't get $p out of your hand.", false, ch, obj, nullptr, TO_CHAR);
    else if (GET_OBJ_VNUM(obj) == dball[0] || GET_OBJ_VNUM(obj) == dball[1] || GET_OBJ_VNUM(obj) == dball[2] ||
             GET_OBJ_VNUM(obj) == dball[3] || GET_OBJ_VNUM(obj) == dball[4] || GET_OBJ_VNUM(obj) == dball[5] ||
             GET_OBJ_VNUM(obj) == dball[6])
        send_to_char(ch, "You can not bag dragon balls.\r\n");
    else if (OBJ_FLAGGED(obj, ITEM_NORENT))
        send_to_char(ch, "That isn't worth bagging. Better keep that close if you wanna keep it at all.\r\n");
    else if (!cont->carried_by && check_saveroom_count(ch, obj) > 150) {
        send_to_char(ch,
                     "The save room can not hold anymore items. (150 max, count of items in containers is halved)\r\n");
    } else {
        obj_from_char(obj);
        obj_to_obj(obj, cont);

        if (!OBJ_FLAGGED(obj, ITEM_ANTI_HIEROPHANT)) {
            act("$n puts $p in $P.", true, ch, obj, cont, TO_ROOM);
        } else {
            act("$n puts an @DA@wd@cv@Ce@Wnt @DD@wu@ce@Cl @mC@Ma@Wr@wd@n in $P.", true, ch, obj, cont, TO_ROOM);
        }

        /* Yes, I realize this is strange until we have auto-equip on rent. -gg */
        if (OBJ_FLAGGED(obj, ITEM_NODROP) && !OBJ_FLAGGED(cont, ITEM_NODROP)) {
            cont->extra_flags.set(ITEM_NODROP);
            act("You get a strange feeling as you put $p in $P.", false,
                ch, obj, cont, TO_CHAR);
        } else
            act("You put $p in $P.", false, ch, obj, cont, TO_CHAR);
        /* If object placed in portal or vehicle, move it to the portal destination */
        if ((GET_OBJ_TYPE(cont) == ITEM_PORTAL) ||
            (GET_OBJ_TYPE(cont) == ITEM_VEHICLE)) {
            obj_from_obj(obj);
            obj_to_room(obj, real_room(GET_OBJ_VAL(cont, VAL_CONTAINER_CAPACITY)));
            if (GET_OBJ_TYPE(cont) == ITEM_PORTAL) {
                act("What? $U$p disappears from $P in a puff of smoke!",
                    true, ch, obj, cont, TO_ROOM);
                act("What? $U$p disappears from $P in a puff of smoke!",
                    false, ch, obj, cont, TO_CHAR);
            }
        }
    }
}


/* The following put modes are supported by the code below:

	1) put <object> <container>
	2) put all.<object> <container>
	3) put all <container>

	<container> must be in inventory or on ground.
	all objects to be put into container must be in inventory.
*/

ACMD(do_put) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    struct obj_data *obj, *next_obj, *cont;
    struct char_data *tmp_char;
    int obj_dotmode, cont_dotmode, found = 0, howmany = 1;
    char *theobj, *thecont;

    one_argument(two_arguments(argument, arg1, arg2), arg3);    /* three_arguments */

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }

    if (*arg3 && is_number(arg1)) {
        howmany = atoi(arg1);
        theobj = arg2;
        thecont = arg3;
    } else {
        theobj = arg1;
        thecont = arg2;
    }
    obj_dotmode = find_all_dots(theobj);
    cont_dotmode = find_all_dots(thecont);

    if (!*theobj)
        send_to_char(ch, "Put what in what?\r\n");
    else if (cont_dotmode != FIND_INDIV)
        send_to_char(ch, "You can only put things into one container at a time.\r\n");
    else if (!*thecont) {
        send_to_char(ch, "What do you want to put %s in?\r\n", obj_dotmode == FIND_INDIV ? "it" : "them");
    } else {
        generic_find(thecont, FIND_OBJ_INV | FIND_OBJ_EQUIP | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
        if (!cont)
            send_to_char(ch, "You don't see %s %s here.\r\n", AN(thecont), thecont);
        else if ((GET_OBJ_TYPE(cont) != ITEM_CONTAINER) &&
                 (GET_OBJ_TYPE(cont) != ITEM_PORTAL) &&
                 (GET_OBJ_TYPE(cont) != ITEM_VEHICLE))
            act("$p is not a container.", false, ch, cont, nullptr, TO_CHAR);
        else if (OBJVAL_FLAGGED(cont, CONT_CLOSED))
            send_to_char(ch, "You'd better open it first!\r\n");
        else {
            if (obj_dotmode == FIND_INDIV) {    /* put <obj> <container> */
                if (!(obj = get_obj_in_list_vis(ch, theobj, nullptr, ch->contents)))
                    send_to_char(ch, "You aren't carrying %s %s.\r\n", AN(theobj), theobj);
                else if (obj == cont && howmany == 1)
                    send_to_char(ch, "You attempt to fold it into itself, but fail.\r\n");
                else {
                    while (obj && howmany) {
                        next_obj = obj->next_content;
                        if (obj != cont) {
                            howmany--;
                            perform_put(ch, obj, cont);
                        }
                        obj = get_obj_in_list_vis(ch, theobj, nullptr, next_obj);
                    }
                }
            } else {
                for (obj = ch->contents; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
                        (obj_dotmode == FIND_ALL || isname(theobj, obj->name))) {
                        found = 1;
                        perform_put(ch, obj, cont);
                    }
                }
                if (!found) {
                    if (obj_dotmode == FIND_ALL)
                        send_to_char(ch, "You don't seem to have anything to put in it.\r\n");
                    else
                        send_to_char(ch, "You don't seem to have any %ss.\r\n", theobj);
                }
            }
        }
    }
}

static int can_take_obj(struct char_data *ch, struct obj_data *obj) {
    if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
        act("$p: you can't take that!", false, ch, obj, nullptr, TO_CHAR);
        return (0);
    } else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
        act("$p: your arms are full!", false, ch, obj, nullptr, TO_CHAR);
        return (0);
    } else if (!ch->canCarryWeight(obj)) {
        act("$p: you can't carry that much weight.", false, ch, obj, nullptr, TO_CHAR);
        return (0);
    }
    return (1);
}

static void get_check_money(struct char_data *ch, struct obj_data *obj) {
    int value = GET_OBJ_VAL(obj, VAL_MONEY_SIZE);

    if (GET_OBJ_TYPE(obj) != ITEM_MONEY || value <= 0)
        return;

    if (GET_GOLD(ch) + value > GOLD_CARRY(ch)) {
        send_to_char(ch, "You can only carry %s zenni at your current level, and leave the rest.\r\n",
                     add_commas(GOLD_CARRY(ch)).c_str());
        act("@w$n @wdrops some onto the ground.@n", false, ch, nullptr, nullptr, TO_ROOM);
        extract_obj(obj);
        int diff = 0;
        diff = (GET_GOLD(ch) + value) - GOLD_CARRY(ch);
        obj = create_money(diff);
        obj_to_room(obj, IN_ROOM(ch));
        ch->set(CharMoney::Carried, GOLD_CARRY(ch));
        return;
    }


    ch->mod(CharMoney::Carried, value);
    extract_obj(obj);

    if (value == 1) {
        send_to_char(ch, "There was 1 zenni.\r\n");
    } else {
        send_to_char(ch, "There were %d zenni.\r\n", value);
        if (IS_AFFECTED(ch, AFF_GROUP) && PRF_FLAGGED(ch, PRF_AUTOSPLIT)) {
            char split[MAX_INPUT_LENGTH];
            sprintf(split, "%d", value);
            do_split(ch, split, 0, 0);
        }
    }
}

static void perform_get_from_container(struct char_data *ch, struct obj_data *obj,
                                       struct obj_data *cont, int mode) {
    if (mode == FIND_OBJ_INV || mode == FIND_OBJ_EQUIP || can_take_obj(ch, obj)) {
        if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
            act("$p: you can't hold any more items.", false, ch, obj, nullptr, TO_CHAR);
            return;
        }
        if (SITS(ch) && GET_OBJ_VNUM(SITS(ch)) > 603 && GET_OBJ_VNUM(SITS(ch)) < 608 &&
            GET_OBJ_VNUM(SITS(ch)) - 4 != GET_OBJ_VNUM(cont) && GET_OBJ_VNUM(cont) > 599 && GET_OBJ_VNUM(cont) < 604) {
            send_to_char(ch, "You aren't playing at that table!\r\n");
            return;
        } else if (get_otrigger(obj, ch)) {
            obj_from_obj(obj);
            obj_to_char(obj, ch);
            if (OBJ_FLAGGED(cont, ITEM_SHEATH)) {
                act("You draw $p from $P.", false, ch, obj, cont, TO_CHAR);
                act("$n draws $p from $P.", true, ch, obj, cont, TO_ROOM);
            } else {
                act("You get $p from $P.", false, ch, obj, cont, TO_CHAR);
                act("$n gets $p from $P.", true, ch, obj, cont, TO_ROOM);
            }
            if (OBJ_FLAGGED(obj, ITEM_HOT)) {
                if (GET_BONUS(ch, BONUS_FIREPROOF) <= 0 && !IS_DEMON(ch)) {
                    ch->decCurHealthPercent(.25);
                    if (GET_BONUS(ch, BONUS_FIREPRONE) > 0)
                        ch->decCurHealthPercent(1, 1);

                    ch->affected_by.set(AFF_BURNED);
                    act("@RYou are burned by it!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@R$n@R is burned by it!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            if (IS_NPC(ch)) {
                item_check(obj, ch);
            }
            get_check_money(ch, obj);
        }
    }
}


static void get_from_container(struct char_data *ch, struct obj_data *cont,
                               char *arg, int mode, int howmany) {
    struct obj_data *obj, *next_obj;
    int obj_dotmode, found = 0;

    obj_dotmode = find_all_dots(arg);

    if (OBJVAL_FLAGGED(cont, CONT_CLOSED))
        act("$p is closed.", false, ch, cont, nullptr, TO_CHAR);
    else if (obj_dotmode == FIND_INDIV) {
        if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, cont->contents))) {
            char buf[MAX_STRING_LENGTH];

            snprintf(buf, sizeof(buf), "There doesn't seem to be %s %s in $p.", AN(arg), arg);
            act(buf, false, ch, cont, nullptr, TO_CHAR);
        } else {
            struct obj_data *obj_next;
            while (obj && howmany--) {
                obj_next = obj->next_content;
                perform_get_from_container(ch, obj, cont, mode);
                obj = get_obj_in_list_vis(ch, arg, nullptr, obj_next);
            }
        }
    } else {
        if (obj_dotmode == FIND_ALLDOT && !*arg) {
            send_to_char(ch, "Get all of what?\r\n");
            return;
        }
        for (obj = cont->contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) &&
                (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
                found = 1;
                perform_get_from_container(ch, obj, cont, mode);
            }
        }
        if (!found) {
            if (obj_dotmode == FIND_ALL)
                act("$p seems to be empty.", false, ch, cont, nullptr, TO_CHAR);
            else {
                char buf[MAX_STRING_LENGTH];

                snprintf(buf, sizeof(buf), "You can't seem to find any %ss in $p.", arg);
                act(buf, false, ch, cont, nullptr, TO_CHAR);
            }
        }
    }
}


int perform_get_from_room(struct char_data *ch, struct obj_data *obj) {

    if (SITTING(obj)) {
        send_to_char(ch, "Someone is on that!\r\n");
        return (0);
    }

    if (OBJ_FLAGGED(obj, ITEM_BURIED)) {
        send_to_char(ch, "Get what?\r\n");
        return (0);
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN1) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN2)) {
        send_to_char(ch, "You can't get things from a garden. Help garden.\r\n");
        return (0);
    }

    if (can_take_obj(ch, obj) && get_otrigger(obj, ch)) {
        obj_from_room(obj);
        obj_to_char(obj, ch);
        act("You get $p.", false, ch, obj, nullptr, TO_CHAR);
        act("$n gets $p.", true, ch, obj, nullptr, TO_ROOM);

        if (OBJ_FLAGGED(obj, ITEM_HOT)) {
            if (GET_BONUS(ch, BONUS_FIREPROOF) <= 0 && !IS_DEMON(ch)) {
                ch->decCurHealthPercent(.25, 1);
                if (GET_BONUS(ch, BONUS_FIREPRONE) > 0)
                    ch->decCurHealthPercent(1, 1);

                ch->affected_by.set(AFF_BURNED);
                act("@RYou are burned by it!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n@R is burned by it!@n", true, ch, nullptr, nullptr, TO_ROOM);
            }
        }

        if (IS_NPC(ch))
            item_check(obj, ch);
        get_check_money(ch, obj);
    }
    return (0);
}

static char *find_exdesc_keywords(char *word, struct extra_descr_data *list) {
    struct extra_descr_data *i;

    for (i = list; i; i = i->next)
        if (isname(word, i->keyword))
            return (i->keyword);

    return (nullptr);
}

static void get_from_room(struct char_data *ch, char *arg, int howmany) {
    struct obj_data *obj, *next_obj;
    int dotmode, found = 0;
    char *descword;

    /* Are they trying to take something in a room extra description? */
    if (find_exdesc(arg, ch->getRoom()->ex_description) != nullptr) {
        send_to_char(ch, "You can't take %s %s.\r\n", AN(arg), arg);
        return;
    }

    dotmode = find_all_dots(arg);

    if (dotmode == FIND_INDIV) {
        if ((descword = find_exdesc_keywords(arg, ch->getRoom()->ex_description)) != nullptr)
            send_to_char(ch, "%s: you can't take that!\r\n", fname(descword));
        else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents)))
            send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
        else {
            struct obj_data *obj_next;
            while (obj && howmany--) {
                obj_next = obj->next_content;
                perform_get_from_room(ch, obj);
                obj = get_obj_in_list_vis(ch, arg, nullptr, obj_next);
            }
        }
    } else {
        if (dotmode == FIND_ALLDOT && !*arg) {
            send_to_char(ch, "Get all of what?\r\n");
            return;
        }
        for (obj = ch->getRoom()->contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) &&
                (dotmode == FIND_ALL || isname(arg, obj->name))) {
                found = 1;
                perform_get_from_room(ch, obj);
            }
        }
        if (!found) {
            if (dotmode == FIND_ALL)
                send_to_char(ch, "There doesn't seem to be anything here.\r\n");
            else
                send_to_char(ch, "You don't see any %ss here.\r\n", arg);
        }
    }
}

ACMD(do_get) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    int cont_dotmode, found = 0, mode;
    struct obj_data *cont;
    struct char_data *tmp_char;

    one_argument(two_arguments(argument, arg1, arg2), arg3);    /* three_arguments */

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }

    if (!*arg1)
        send_to_char(ch, "Get what?\r\n");
    else if (!*arg2)
        get_from_room(ch, arg1, 1);
    else if (is_number(arg1) && !*arg3)
        get_from_room(ch, arg2, atoi(arg1));
    else {
        int amount = 1;
        if (is_number(arg1)) {
            amount = atoi(arg1);
            strcpy(arg1, arg2);    /* strcpy: OK (sizeof: arg1 == arg2) */
            strcpy(arg2, arg3);    /* strcpy: OK (sizeof: arg2 == arg3) */
        }
        cont_dotmode = find_all_dots(arg2);
        if (cont_dotmode == FIND_INDIV) {
            mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
            if (!cont)
                send_to_char(ch, "You don't have %s %s.\r\n", AN(arg2), arg2);
            else if (GET_OBJ_TYPE(cont) == ITEM_VEHICLE)
                send_to_char(ch, "You will need to enter it first.\r\n");
            else if ((GET_OBJ_TYPE(cont) != ITEM_CONTAINER) &&
                     !((GET_OBJ_TYPE(cont) == ITEM_PORTAL) && (OBJVAL_FLAGGED(cont, CONT_CLOSEABLE))))
                act("$p is not a container.", false, ch, cont, nullptr, TO_CHAR);
            else
                get_from_container(ch, cont, arg1, mode, amount);
        } else {
            if (cont_dotmode == FIND_ALLDOT && !*arg2) {
                send_to_char(ch, "Get from all of what?\r\n");
                return;
            }
            for (cont = ch->contents; cont; cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) &&
                    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
                    if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
                        found = 1;
                        get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
                    } else if (cont_dotmode == FIND_ALLDOT) {
                        found = 1;
                        act("$p is not a container.", false, ch, cont, nullptr, TO_CHAR);
                    }
                }
            for (cont = ch->getRoom()->contents; cont; cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) &&
                    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
                    if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
                        get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
                        found = 1;
                    } else if (cont_dotmode == FIND_ALLDOT) {
                        act("$p is not a container.", false, ch, cont, nullptr, TO_CHAR);
                        found = 1;
                    }
                }
            if (!found) {
                if (cont_dotmode == FIND_ALL)
                    send_to_char(ch, "You can't seem to find any containers.\r\n");
                else
                    send_to_char(ch, "You can't seem to find any %ss here.\r\n", arg2);
            }
        }
    }
}

static void perform_drop_gold(struct char_data *ch, int amount,
                              int8_t mode, room_rnum RDR) {
    struct obj_data *obj;

    if (amount <= 0)
        send_to_char(ch, "Heh heh heh.. we are jolly funny today, eh?\r\n");
    else if (GET_GOLD(ch) < amount)
        send_to_char(ch, "You don't have that many zenni!\r\n");
    else {
        if (mode != SCMD_JUNK) {
            WAIT_STATE(ch, PULSE_1SEC);    /* to prevent zenni-bombing */
            obj = create_money(amount);
            if (mode == SCMD_DONATE) {
                send_to_char(ch, "You throw some zenni into the air where it disappears in a puff of smoke!\r\n");
                act("$n throws some gold into the air where it disappears in a puff of smoke!",
                    false, ch, nullptr, nullptr, TO_ROOM);
                obj_to_room(obj, RDR);
                act("$p suddenly appears in a puff of orange smoke!", 0, nullptr, obj, nullptr, TO_ROOM);
            } else {
                char buf[MAX_STRING_LENGTH];

                if (!drop_wtrigger(obj, ch)) {
                    extract_obj(obj);
                    return;
                }

                if (!drop_wtrigger(obj, ch) && (obj)) { /* obj may be purged */
                    extract_obj(obj);
                    return;
                }

                snprintf(buf, sizeof(buf), "$n drops %s.", money_desc(amount));
                act(buf, true, ch, nullptr, nullptr, TO_ROOM);

                send_to_char(ch, "You drop some zenni.\r\n");
                obj_to_room(obj, IN_ROOM(ch));
                if (GET_ADMLEVEL(ch) > 0) {
                    send_to_imm("IMM DROP: %s dropped %s in room [%d]", GET_NAME(ch), obj->short_description,
                                GET_ROOM_VNUM(IN_ROOM(obj)));
                    log_imm_action("IMM DROP: %s dropped %s in room [%d]", GET_NAME(ch), obj->short_description,
                                   GET_ROOM_VNUM(IN_ROOM(obj)));
                    if (check_insidebag(obj, 0.0) > 1) {
                        send_to_imm("IMM DROP: Object contains %d other items.", check_insidebag(obj, 0.0));
                        log_imm_action("IMM DROP: Object contains %d other items.", check_insidebag(obj, 0.0));
                    }
                }
            }
        } else {
            char buf[MAX_STRING_LENGTH];

            snprintf(buf, sizeof(buf), "$n drops %s which disappears in a puff of smoke!", money_desc(amount));
            act(buf, false, ch, nullptr, nullptr, TO_ROOM);

            send_to_char(ch, "You drop some zenni which disappears in a puff of smoke!\r\n");
        }
        ch->mod(CharMoney::Carried, -amount);
    }
}

#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
              "  It vanishes in a puff of smoke!" : "")

static int perform_drop(struct char_data *ch, struct obj_data *obj,
                        int8_t mode, const char *sname, room_rnum RDR) {
    char buf[MAX_STRING_LENGTH];
    int value;

    if (!drop_otrigger(obj, ch))
        return 0;

    if ((mode == SCMD_DROP) && !drop_wtrigger(obj, ch))
        return 0;

    if (GET_OBJ_VNUM(obj) == 17 || GET_OBJ_VNUM(obj) == 17998) {
        snprintf(buf, sizeof(buf), "You can't %s $p, it is grafted into your soul :P", sname);
        act(buf, false, ch, obj, nullptr, TO_CHAR);
        return (0);
    }
    if (GET_OBJ_VNUM(obj) == 20 || GET_OBJ_VNUM(obj) == 21 || GET_OBJ_VNUM(obj) == 22 || GET_OBJ_VNUM(obj) == 23 ||
        GET_OBJ_VNUM(obj) == 24 || GET_OBJ_VNUM(obj) == 25 || GET_OBJ_VNUM(obj) == 26) {
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE)) {
            snprintf(buf, sizeof(buf), "You can't %s $p in space!", sname);
            act(buf, false, ch, obj, nullptr, TO_CHAR);
            return (0);
        }
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN1) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN2)) {
            snprintf(buf, sizeof(buf), "You can't %s $p in here. Read help garden.", sname);
            act(buf, false, ch, obj, nullptr, TO_CHAR);
            return (0);
        }
        if ((mode == SCMD_DROP) && OBJ_FLAGGED(obj, ITEM_NORENT)) {
            snprintf(buf, sizeof(buf), "You drop $p but it gets lost on the ground!");
            act(buf, false, ch, obj, nullptr, TO_CHAR);
            obj_from_char(obj);
            extract_obj(obj);
            return (0);
        }
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOINSTANT)) {
            snprintf(buf, sizeof(buf), "You can't %s $p in this protected area!", sname);
            act(buf, false, ch, obj, nullptr, TO_CHAR);
            return (0);
        }
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SHIP)) {
            snprintf(buf, sizeof(buf), "You can't %s $p on a private ship!", sname);
            act(buf, false, ch, obj, nullptr, TO_CHAR);
            return (0);
        }
        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOUSE)) {
            snprintf(buf, sizeof(buf), "You can't %s $p in a private house!", sname);
            act(buf, false, ch, obj, nullptr, TO_CHAR);
            return (0);
        }
    }
    if (OBJ_FLAGGED(obj, ITEM_NODROP) && GET_ADMLEVEL(ch) < 1) {
        snprintf(buf, sizeof(buf), "You can't %s $p, it must be CURSED!", sname);
        act(buf, false, ch, obj, nullptr, TO_CHAR);
        return (0);
    }
    if ((mode == SCMD_DONATE || mode == SCMD_JUNK) && OBJ_FLAGGED(obj, ITEM_NODONATE)) {
        snprintf(buf, sizeof(buf), "You can't %s $p!", sname);
        act(buf, false, ch, obj, nullptr, TO_CHAR);
        return (0);
    }

    snprintf(buf, sizeof(buf), "You %s $p.", sname);
    act(buf, false, ch, obj, nullptr, TO_CHAR);

    snprintf(buf, sizeof(buf), "$n %ss $p.", sname);
    act(buf, true, ch, obj, nullptr, TO_ROOM);

    obj_from_char(obj);

    switch (mode) {
        case SCMD_DROP:
            if (!OBJ_FLAGGED(obj, ITEM_UNBREAKABLE) && ROOM_EFFECT(IN_ROOM(ch)) == 6) {
                act("$p melts in the lava!", false, ch, obj, nullptr, TO_CHAR);
                act("$p melts in the lava!", false, ch, obj, nullptr, TO_ROOM);
                extract_obj(obj);
            } else if (ROOM_EFFECT(IN_ROOM(ch)) == 6) {
                act("$p plops down on some cooled lava!", false, ch, obj, nullptr, TO_CHAR);
                act("$p plops down on some cooled lava!", false, ch, obj, nullptr, TO_ROOM);
                obj_to_room(obj, IN_ROOM(ch));
                if (GET_ADMLEVEL(ch) > 0) {
                    send_to_imm("IMM DROP: %s dropped %s in room [%d]", GET_NAME(ch), obj->short_description,
                                GET_ROOM_VNUM(IN_ROOM(obj)));
                    log_imm_action("IMM DROP: %s dropped %s in room [%d]", GET_NAME(ch), obj->short_description,
                                   GET_ROOM_VNUM(IN_ROOM(obj)));
                }
            } else {
                obj_to_room(obj, IN_ROOM(ch));
                if (GET_ADMLEVEL(ch) > 0) {
                    send_to_imm("IMM DROP: %s dropped %s in room [%d]", GET_NAME(ch), obj->short_description,
                                GET_ROOM_VNUM(IN_ROOM(obj)));
                    log_imm_action("IMM DROP: %s dropped %s in room [%d]", GET_NAME(ch), obj->short_description,
                                   GET_ROOM_VNUM(IN_ROOM(obj)));
                }
            }
            return (0);
        case SCMD_DONATE:
            obj_to_room(obj, RDR);
            act("$p suddenly appears in a puff a smoke!", false, nullptr, obj, nullptr, TO_ROOM);
            return (0);
        case SCMD_JUNK:
            value = MAX(1, MIN(200, GET_OBJ_COST(obj) / 16));
            extract_obj(obj);
            return (value);
        default:
            basic_mud_log("SYSERR: Incorrect argument %d passed to perform_drop.", mode);
            /*  SYSERR_DESC:
     *  This error comes from perform_drop() and is output when perform_drop()
     *  is called with an illegal 'mode' argument.
     */
            break;
    }

    return (0);
}

ACMD(do_drop) {
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *obj, *next_obj;
    room_rnum RDR = 0;
    int8_t mode = SCMD_DROP;
    int dotmode, amount = 0, multi, num_don_rooms;
    const char *sname;

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN1) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_GARDEN2)) {
        send_to_char(ch, "You can not do that in a garden.\r\n");
        return;
    }

    switch (subcmd) {
        case SCMD_JUNK:
            sname = "junk";
            mode = SCMD_JUNK;
            break;
        case SCMD_DONATE:
            sname = "donate";
            mode = SCMD_DONATE;
            /* fail + double chance for room 1   */
            num_don_rooms = (CONFIG_DON_ROOM_1 != NOWHERE) * 2 +
                            (CONFIG_DON_ROOM_2 != NOWHERE) +
                            (CONFIG_DON_ROOM_3 != NOWHERE) + 1;
            switch (rand_number(0, num_don_rooms)) {
                case 0:
                    mode = SCMD_JUNK;
                    break;
                case 1:
                case 2:
                    RDR = real_room(CONFIG_DON_ROOM_1);
                    break;
                case 3:
                    RDR = real_room(CONFIG_DON_ROOM_2);
                    break;
                case 4:
                    RDR = real_room(CONFIG_DON_ROOM_3);
                    break;

            }
            if (RDR == NOWHERE) {
                send_to_char(ch, "Sorry, you can't donate anything right now.\r\n");
                return;
            }
            break;
        default:
            sname = "drop";
            break;
    }

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "What do you want to %s?\r\n", sname);
        return;
    } else if (is_number(arg)) {
        multi = atoi(arg);
        one_argument(argument, arg);
        if (!strcasecmp("zenni", arg) || !strcasecmp("gold", arg))
            perform_drop_gold(ch, multi, mode, RDR);
        else if (multi <= 0)
            send_to_char(ch, "Yeah, that makes sense.\r\n");
        else if (!*arg)
            send_to_char(ch, "What do you want to %s %d of?\r\n", sname, multi);
        else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))
            send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
        else if (check_saveroom_count(ch, obj) > 150)
            send_to_char(ch,
                         "The save room you are in can not hold anymore items! (150 max, count of items in containers is halved)\r\n");
        else {
            do {
                next_obj = get_obj_in_list_vis(ch, arg, nullptr, obj->next_content);
                amount += perform_drop(ch, obj, mode, sname, RDR);
                obj = next_obj;
            } while (obj && --multi);
        }
    } else {
        dotmode = find_all_dots(arg);

        /* Can't junk or donate all */
        if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
            if (subcmd == SCMD_JUNK)
                send_to_char(ch, "Go to the dump if you want to junk EVERYTHING!\r\n");
            else
                send_to_char(ch, "Go do the donation room if you want to donate EVERYTHING!\r\n");
            return;
        }
        if (dotmode == FIND_ALL) {
            int fail = false;

            if (!ch->contents)
                send_to_char(ch, "You don't seem to be carrying anything.\r\n");
            else {
                for (obj = ch->contents; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (check_saveroom_count(ch, obj) > 150) {
                        fail = true;
                    } else {
                        amount += perform_drop(ch, obj, mode, sname, RDR);
                    }
                }
                if (fail == true) {
                    send_to_char(ch,
                                 "Some of the items couldn't be dropped into this save room. It is too full. (150 max, containers half the count inside)\r\n");
                }
            }
        } else if (dotmode == FIND_ALLDOT) {
            int fail = false;

            if (!*arg) {
                send_to_char(ch, "What do you want to %s all of?\r\n", sname);
                return;
            }
            if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))
                send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);

            while (obj) {
                next_obj = get_obj_in_list_vis(ch, arg, nullptr, obj->next_content);
                if (check_saveroom_count(ch, obj) > 150) {
                    fail = true;
                } else {
                    amount += perform_drop(ch, obj, mode, sname, RDR);
                }
                obj = next_obj;
            }
            if (fail == true) {
                send_to_char(ch,
                             "Some of the items couldn't be dropped into this save room. It is too full. (150 max, containers half the count inside)\r\n");
            }
        } else {
            if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
                send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
            } else if (check_saveroom_count(ch, obj) > 150) {
                send_to_char(ch,
                             "The item couldn't be dropped into this save room. It is too full. (150 max, containers half the count inside)\r\n");
            } else {
                amount += perform_drop(ch, obj, mode, sname, RDR);
            }
        }
    }
}

static void perform_give(struct char_data *ch, struct char_data *vict,
                         struct obj_data *obj) {
    if (!give_otrigger(obj, ch, vict))
        return;
    if (!receive_mtrigger(vict, ch, obj))
        return;

    if (OBJ_FLAGGED(obj, ITEM_NODROP)) {
        act("You can't let go of $p!!  Yeech!", false, ch, obj, nullptr, TO_CHAR);
        return;
    }
    if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
        act("$N seems to have $S hands full.", false, ch, nullptr, vict, TO_CHAR);
        if (IS_NPC(ch)) {
            act("$n@n drops $p because you can't carry anymore.", true, ch, obj, vict, TO_VICT);
            act("$n@n drops $p on the ground since $N's unable to carry it.", true, ch, obj, vict, TO_NOTVICT);
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
        }
        return;
    }
    if (IS_NPC(vict) && (OBJ_FLAGGED(obj, ITEM_FORGED) || OBJ_FLAGGED(obj, ITEM_FORGED))) {
        act("$n tries to hand $p to $N.", true, ch, obj, vict, TO_NOTVICT);
        do_say(vict, "I don't want that piece of junk.", 0, 0);
        return;
    }
    if (!vict->canCarryWeight(obj)) {
        act("$E can't carry that much weight.", false, ch, nullptr, vict, TO_CHAR);
        if (IS_NPC(ch)) {
            act("$n@n drops $p because you can't carry anymore.", true, ch, obj, vict, TO_VICT);
            act("$n@n drops $p on the ground since $N's unable to carry it.", true, ch, obj, vict, TO_NOTVICT);
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
        }
        return;
    }
    if (!vict->canCarryWeight(obj)) {
        act("$E can't carry that much weight because of the gravity.", false, ch, nullptr, vict, TO_CHAR);
        if (IS_NPC(ch)) {
            act("$n@n drops $p because you can't carry anymore.", true, ch, obj, vict, TO_VICT);
            act("$n@n drops $p on the ground since $N's unable to carry it.", true, ch, obj, vict, TO_NOTVICT);
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
        }
        return;
    }
    if (!IS_NPC(vict) && !IS_NPC(ch)) {
        if (PRF_FLAGGED(vict, PRF_NOGIVE)) {
            act("$N refuses to accept $p at this time.", false, ch, obj, vict, TO_CHAR);
            act("$n tries to give you $p, but you are refusing to be handed things.", false, ch, obj, vict, TO_VICT);
            act("$n tries to give $N, $p, but $E is refusing to be handed things right now.", false, ch, obj, vict,
                TO_NOTVICT);
            return;
        }
    }
    obj_from_char(obj);
    obj_to_char(obj, vict);
    act("You give $p to $N.", false, ch, obj, vict, TO_CHAR);
    act("$n gives you $p.", false, ch, obj, vict, TO_VICT);
    act("$n gives $p to $N.", true, ch, obj, vict, TO_NOTVICT);

    if (OBJ_FLAGGED(obj, ITEM_HOT)) {
        if (GET_BONUS(vict, BONUS_FIREPROOF) <= 0 && !IS_DEMON(vict)) {
            ch->decCurHealthPercent(.25, 1);
            if (GET_BONUS(vict, BONUS_FIREPRONE) > 0)
                ch->decCurHealthPercent(1, 1);


            vict->affected_by.set(AFF_BURNED);
            act("@RYou are burned by it!@n", true, vict, nullptr, nullptr, TO_CHAR);
            act("@R$n@R is burned by it!@n", true, vict, nullptr, nullptr, TO_ROOM);
        }
    }


}

/* utility function for give */
static struct char_data *give_find_vict(struct char_data *ch, char *arg) {
    struct char_data *vict;

    skip_spaces(&arg);
    if (!*arg)
        send_to_char(ch, "To who?\r\n");
    else if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "%s", CONFIG_NOPERSON);
        if (IS_NPC(ch))
            send_to_imm("Mob Give: Victim, %s, doesn't exist.", arg);
    } else if (vict == ch)
        send_to_char(ch, "What's the point of that?\r\n");
    else
        return (vict);

    return (nullptr);
}

static void perform_give_gold(struct char_data *ch, struct char_data *vict,
                              int amount) {
    char buf[MAX_STRING_LENGTH];

    if (amount <= 0) {
        send_to_char(ch, "Heh heh heh ... we are jolly funny today, eh?\r\n");
        return;
    }
    if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || !ADM_FLAGGED(ch, ADM_MONEY))) {
        send_to_char(ch, "You don't have that much zenni!\r\n");
        return;
    }
    if (GET_GOLD(vict) + amount > GOLD_CARRY(vict)) {
        send_to_char(ch, "They can't carry that much zenni.\r\n");
        return;
    }
    send_to_char(ch, "%s", CONFIG_OK);

    snprintf(buf, sizeof(buf), "$n gives you %d zenni.", amount);
    act(buf, false, ch, nullptr, vict, TO_VICT);

    snprintf(buf, sizeof(buf), "$n gives %s to $N.", money_desc(amount));
    act(buf, true, ch, nullptr, vict, TO_NOTVICT);

    if (IS_NPC(ch) || !ADM_FLAGGED(ch, ADM_MONEY))
        ch->mod(CharMoney::Carried, -amount);
    vict->mod(CharMoney::Carried, amount);

    bribe_mtrigger(vict, ch, amount);
}


ACMD(do_give) {
    char arg[MAX_STRING_LENGTH];
    int amount, dotmode;
    struct char_data *vict;
    struct obj_data *obj, *next_obj;

    argument = one_argument(argument, arg);

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }


    reveal_hiding(ch, 0);
    if (!*arg)
        send_to_char(ch, "Give what to who?\r\n");
    else if (is_number(arg)) {
        amount = atoi(arg);
        argument = one_argument(argument, arg);
        if (!strcasecmp("zenni", arg) || !strcasecmp("gold", arg)) {
            one_argument(argument, arg);
            if ((vict = give_find_vict(ch, arg)) != nullptr) {
                perform_give_gold(ch, vict, amount);
                if (GET_ADMLEVEL(ch) > 0 && !IS_NPC(vict)) {
                    send_to_imm("IMM GIVE: %s has given %s zenni to %s.", GET_NAME(ch), add_commas(amount).c_str(),
                                GET_NAME(vict));
                    log_imm_action("IMM GIVE: %s has given %s zenni to %s.", GET_NAME(ch), add_commas(amount).c_str(),
                                   GET_NAME(vict));
                }
            }
            return;
        } else if (!*arg)    /* Give multiple code. */
            send_to_char(ch, "What do you want to give %d of?\r\n", amount);
        else if (!(vict = give_find_vict(ch, argument)))
            return;
        else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))
            send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
        else {
            while (obj && amount--) {
                next_obj = get_obj_in_list_vis(ch, arg, nullptr, obj->next_content);
                perform_give(ch, vict, obj);
                if (GET_ADMLEVEL(ch) > 0 && !IS_NPC(vict)) {
                    send_to_imm("IMM GIVE: %s has given %s to %s.", GET_NAME(ch), obj->short_description,
                                GET_NAME(vict));
                    log_imm_action("IMM GIVE: %s has given %s to %s.", GET_NAME(ch), obj->short_description,
                                   GET_NAME(vict));
                }
                obj = next_obj;
            }
        }
    } else {
        char buf1[MAX_INPUT_LENGTH];

        one_argument(argument, buf1);
        if (!(vict = give_find_vict(ch, buf1)))
            return;
        dotmode = find_all_dots(arg);
        if (dotmode == FIND_INDIV) {
            if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))
                send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
            else {
                perform_give(ch, vict, obj);
                if (GET_ADMLEVEL(ch) > 0 && !IS_NPC(vict)) {
                    send_to_imm("IMM GIVE: %s has given %s to %s.", GET_NAME(ch), obj->short_description,
                                GET_NAME(vict));
                    log_imm_action("IMM GIVE: %s has given %s to %s.", GET_NAME(ch), obj->short_description,
                                   GET_NAME(vict));
                }
            }
        } else {
            if (dotmode == FIND_ALLDOT && !*arg) {
                send_to_char(ch, "All of what?\r\n");
                return;
            }
            if (!ch->contents)
                send_to_char(ch, "You don't seem to be holding anything.\r\n");
            else
                for (obj = ch->contents; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (CAN_SEE_OBJ(ch, obj) &&
                        ((dotmode == FIND_ALL || isname(arg, obj->name)))) {
                        perform_give(ch, vict, obj);
                        if (GET_ADMLEVEL(ch) > 0 && !IS_NPC(vict)) {
                            send_to_imm("IMM GIVE: %s has given %s to %s.", GET_NAME(ch), obj->short_description,
                                        GET_NAME(vict));
                            log_imm_action("IMM GIVE: %s has given %s to %s.", GET_NAME(ch), obj->short_description,
                                           GET_NAME(vict));
                        }
                    }
                }
        }
    }
}

void weight_change_object(struct obj_data *obj, int weight) {
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

    if (IN_ROOM(obj) != NOWHERE) {
        GET_OBJ_WEIGHT(obj) += weight;
    } else if ((tmp_ch = obj->carried_by)) {
        obj_from_char(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_char(obj, tmp_ch);
    } else if ((tmp_obj = obj->in_obj)) {
        obj_from_obj(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_obj(obj, tmp_obj);
    } else {
        basic_mud_log("SYSERR: Unknown attempt to subtract weight from an object.");
        /*  SYSERR_DESC:
     *  weight_change_object() outputs this error when weight is attempted to
     *  be removed from an object that is not carried or in another object.
     */
    }
}

void name_from_drinkcon(struct obj_data *obj) {
    char *new_name, *cur_name, *next;
    const char *liqname;
    int liqlen, cpylen;

    if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
        return;

    liqname = drinknames[GET_OBJ_VAL(obj, VAL_DRINKCON_LIQUID)];
    if (!isname(liqname, obj->name)) {
        /*log("SYSERR: Can't remove liquid '%s' from '%s' (%d) item.", liqname, obj->name, obj->item_number);*/
        /*  SYSERR_DESC:
     *  From name_from_drinkcon(), this error comes about if the object
     *  noted (by keywords and item vnum) does not contain the liquid string
     *  being searched for.
     */
        return;
    }

    liqlen = strlen(liqname);
    CREATE(new_name, char, strlen(obj->name) - strlen(liqname)); /* +1 for NUL, -1 for space */

    for (cur_name = obj->name; cur_name; cur_name = next) {
        if (*cur_name == ' ')
            cur_name++;

        if ((next = strchr(cur_name, ' ')))
            cpylen = next - cur_name;
        else
            cpylen = strlen(cur_name);

        if (!strncasecmp(cur_name, liqname, liqlen))
            continue;

        if (*new_name)
            strcat(new_name, " ");    /* strcat: OK (size precalculated) */
        strncat(new_name, cur_name, cpylen);    /* strncat: OK (size precalculated) */
    }

    if (GET_OBJ_RNUM(obj) == NOTHING || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
        free(obj->name);
    obj->name = new_name;
}

void name_to_drinkcon(struct obj_data *obj, int type) {
    char *new_name;

    if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
        return;

    CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
    sprintf(new_name, "%s %s", obj->name, drinknames[type]);    /* sprintf: OK */

    if (GET_OBJ_RNUM(obj) == NOTHING || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
        free(obj->name);

    obj->name = new_name;
}

ACMD(do_drink) {
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *temp;
    struct affected_type af;
    int amount, weight, wasthirsty = 0;
    int on_ground = 0;

    one_argument(argument, arg);
    if (IS_NPC(ch)) {
        return;
    }
    if (IS_ANDROID(ch) || GET_COND(ch, THIRST) < 0) {
        send_to_char(ch, "You need not drink!\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }
    if (GET_COND(ch, HUNGER) <= 1 && GET_COND(ch, THIRST) >= 2 && !IS_NAMEK(ch) && GET_GENOME(ch, 0) != 3 &&
        GET_GENOME(ch, 1) != 3) {
        send_to_char(ch, "You need to eat first!\r\n");
        return;
    }
    wasthirsty = GET_COND(ch, THIRST);
    if (!*arg && !IS_NPC(ch)) {
        char buf[MAX_STRING_LENGTH];
        switch (SECT(IN_ROOM(ch))) {
            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
            case SECT_UNDERWATER:
                snprintf(buf, sizeof(buf), "$n takes a refreshing drink from the surrounding water.");
                act(buf, true, ch, nullptr, nullptr, TO_ROOM);
                send_to_char(ch, "You take a refreshing drink from the surrounding water.\r\n");
                gain_condition(ch, THIRST, 1);
                if (GET_SKILL(ch, SKILL_WELLSPRING) && (ch->getCurKI()) < GET_MAX_MANA(ch) && wasthirsty <= 30 &&
                    subcmd != SCMD_SIP) {

                    ch->incCurKI(((GET_MAX_MANA(ch) * 0.005) + (GET_WIS(ch) * rand_number(80, 100))) *
                                 GET_SKILL(ch, SKILL_WELLSPRING));

                    send_to_char(ch, "You feel your ki return to full strength.\r\n");
                }
                if (GET_COND(ch, THIRST) >= 48)
                    send_to_char(ch, "You don't feel thirsty anymore.\r\n");
                return;
            default:
                if (!SUNKEN(IN_ROOM(ch))) {
                    send_to_char(ch, "Drink from what?\r\n");
                    return;
                } else {
                    snprintf(buf, sizeof(buf), "$n takes a refreshing drink from the surrounding water.");
                    act(buf, true, ch, nullptr, nullptr, TO_ROOM);
                    send_to_char(ch, "You take a refreshing drink from the surrounding water.\r\n");
                    gain_condition(ch, THIRST, 1);
                    if (GET_SKILL(ch, SKILL_WELLSPRING) && !ch->isFullKI() && wasthirsty <= 30 && subcmd != SCMD_SIP) {
                        if (ch->incCurKI(((GET_MAX_MANA(ch) * 0.005) + (GET_WIS(ch) * rand_number(80, 100))) *
                                         GET_SKILL(ch, SKILL_WELLSPRING)) == ch->getMaxKI()) {
                            send_to_char(ch, "You feel your ki return to full strength.\r\n");
                        } else {
                            send_to_char(ch, "You feel your ki has rejuvenated.\r\n");
                        }
                    }
                    if (GET_COND(ch, THIRST) >= 48)
                        send_to_char(ch, "You don't feel thirsty anymore.\r\n");
                }
                return;;
        }
    }
    if (!(temp = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        if (!(temp = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "You can't find it!\r\n");
            return;
        } else
            on_ground = 1;
    }
    if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
        (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
        if (GET_OBJ_VNUM(temp) == 86 && on_ground != 1) {
            act("@wYou uncork the $p and tip it to your lips. Drinking it down you feel a warmth flow through your body and your ki returns to full strength .@n",
                true, ch, temp, nullptr, TO_CHAR);
            act("@C$n@w uncorks the $p and tips it to $s lips. Drinking it down and then smiling.@n", true, ch, temp,
                nullptr, TO_ROOM);
            obj_from_char(temp);
            extract_obj(temp);
            ch->restoreKI();
            GET_COND(ch, THIRST) += 8;
            return;
        } else if (GET_OBJ_VNUM(temp) == 86 && on_ground == 1) {
            send_to_char(ch, "You need to pick that up first.\r\n");
            return;
        } else {
            send_to_char(ch, "You can't drink from that!\r\n");
            return;
        }
    }
    if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
        send_to_char(ch, "You have to be holding that to drink from it.\r\n");
        return;
    }
    if (OBJ_FLAGGED(temp, ITEM_BROKEN)) {
        send_to_char(ch, "Seems like it's broken!\r\n");
        return;
    }
    if (OBJ_FLAGGED(temp, ITEM_FORGED)) {
        send_to_char(ch, "Seems like it doesn't work, maybe it is fake...\r\n");
        return;
    }
    if (IS_NPC(ch)) {     /* Cannot use GET_COND() on mobs. */
        act("$n@w drinks from $p.", true, ch, temp, nullptr, TO_ROOM);
        obj_from_char(temp);
        extract_obj(temp);
        return;
    }
    if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
        /* The pig is drunk */
        send_to_char(ch, "You can't seem to get close enough to your mouth.\r\n");
        act("$n tries to drink but misses $s mouth!", true, ch, nullptr, nullptr, TO_ROOM);
        return;
    }
    if (GET_OBJ_VAL(temp, VAL_DRINKCON_HOWFULL) <= 0 && (GET_OBJ_VAL(temp, VAL_DRINKCON_CAPACITY) >= 1)) {
        send_to_char(ch, "It's empty.\r\n");
        return;
    }

    if (!consume_otrigger(temp, ch, OCMD_DRINK))  /* check trigger */
        return;

    if (subcmd == SCMD_DRINK) {
        char buf[MAX_STRING_LENGTH];

        snprintf(buf, sizeof(buf), "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)]);
        act(buf, true, ch, temp, nullptr, TO_ROOM);

        send_to_char(ch, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)]);
        if (temp->look_description)
            act(temp->look_description, true, ch, temp, nullptr, TO_CHAR);

        /* if (drink_aff[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)][DRUNK] > 0)
      amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)][DRUNK];
    else
      amount = rand_number(3, 10);*/


        amount = 4;

    } else {
        act("$n sips from $p.", true, ch, temp, nullptr, TO_ROOM);
        send_to_char(ch, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)]);
        if (GET_OBJ_VAL(temp, VAL_DRINKCON_POISON)) {
            send_to_char(ch, "It has a sickening taste! Better not eat it...");
            return;
        }
        amount = 1;
    }

    amount = MIN(amount, GET_OBJ_VAL(temp, VAL_DRINKCON_HOWFULL));

    /* You can't subtract more than the object weighs
     Objects that are eternal (max capacity -1) don't get a
     weight subtracted */
    if (GET_OBJ_VAL(temp, VAL_DRINKCON_CAPACITY) > 0) {
        weight = MIN(amount, GET_OBJ_WEIGHT(temp));
        weight_change_object(temp, -weight);    /* Subtract amount */
    }

    gain_condition(ch, DRUNK, drink_aff[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)][DRUNK] * amount);
    gain_condition(ch, HUNGER, drink_aff[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)][HUNGER] * amount);
    gain_condition(ch, THIRST, drink_aff[GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID)][THIRST] * amount);
    if (GET_FOODR(ch) == 0 && subcmd != SCMD_SIP) {
        ch->incCurST((ch->getMaxST() / 100) * amount);
        GET_FOODR(ch) = 2;
        send_to_char(ch, "You feel rejuvinated by it.\r\n");
    }
    if (GET_SKILL(ch, SKILL_WELLSPRING) && (ch->getCurKI()) < GET_MAX_MANA(ch) && wasthirsty <= 30 &&
        subcmd != SCMD_SIP) {
        if (GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID) == 0 || GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID) == 14 ||
            GET_OBJ_VAL(temp, VAL_DRINKCON_LIQUID) == 15) {

            if (ch->incCurKI(((GET_MAX_MANA(ch) * 0.005) + (GET_WIS(ch) * rand_number(80, 100))) *
                             GET_SKILL(ch, SKILL_WELLSPRING)) == ch->getMaxKI()) {
                send_to_char(ch, "You feel your ki return to full strength.\r\n");
            } else {
                send_to_char(ch, "You feel your ki has rejuvenated.\r\n");
            }
        }
    }

    if (GET_COND(ch, DRUNK) > 10)
        send_to_char(ch, "You feel drunk.\r\n");

    if (GET_COND(ch, THIRST) >= 48)
        send_to_char(ch, "You don't feel thirsty anymore.\r\n");

    if (GET_OBJ_VAL(temp, VAL_DRINKCON_POISON) &&
        (!IS_MUTANT(ch) || (GET_GENOME(ch, 0) != 7 && GET_GENOME(ch, 1) != 7))) {    /* The crap was poisoned
! */
        send_to_char(ch, "Oops, it tasted rather strange!\r\n");
        act("$n chokes and utters some strange sounds.", true, ch, nullptr, nullptr, TO_ROOM);

        af.type = SPELL_POISON;
        af.duration = amount * 3;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_join(ch, &af, false, false, false, false);
    }
    /* empty the container, and no longer poison.
     Only remove if it's max capacity > 0, not eternal */
    if (GET_OBJ_VAL(temp, VAL_DRINKCON_CAPACITY) > 0) {
        GET_OBJ_VAL(temp, VAL_DRINKCON_HOWFULL) -= amount;
        if (!GET_OBJ_VAL(temp, VAL_DRINKCON_HOWFULL)) {    /* The last bit */
            name_from_drinkcon(temp);
            GET_OBJ_VAL(temp, VAL_DRINKCON_POISON) = 0;
        }
    }
    return;
}

ACMD(do_eat) {
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *food;
    struct affected_type af;
    int amount;

    one_argument(argument, arg);

    //Set up the conditions in which you cannot eat
    if (IS_NPC(ch))    /* Cannot use GET_COND() on mobs. */
        return;

    if (IS_ANDROID(ch) || GET_COND(ch, HUNGER) < 0) {
        send_to_char(ch, "You need not eat!\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }
    if (AFF_FLAGGED(ch, AFF_POISON)) {
        send_to_char(ch, "You feel too sick from the poison to eat!\r\n");
        return;
    }
    if (!*arg) {
        send_to_char(ch, "Eat what?\r\n");
        return;
    }
    if (!(food = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
        return;
    }
    if (GET_COND(ch, THIRST) <= 1 && GET_COND(ch, HUNGER) >= 2) {
        send_to_char(ch, "You need to drink first!\r\n");
        return;
    }
    if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
                                 (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
        do_drink(ch, argument, 0, SCMD_SIP);
        return;
    }
    if ((GET_OBJ_TYPE(food) != ITEM_FOOD)) {
        send_to_char(ch, "You can't eat THAT!\r\n");
        return;
    }

    //Base how much a player can eat on their con
    float maxCapacity = 48 * ((GET_CON(ch) / 50) + 1);

    //If they're over the limit and not majin they can no longer eat
    if (GET_COND(ch, HUNGER) >= maxCapacity && !IS_MAJIN(ch)) {
        send_to_char(ch, "You can't stomach to eat any more!\r\n");
        return;
    }

    if (!consume_otrigger(food, ch, OCMD_EAT))  /* check trigger */
        return;

    if (subcmd == SCMD_EAT) {
        act("You eat some of $p.", false, ch, food, nullptr, TO_CHAR);
        if (food->look_description)
            act(food->look_description, false, ch, food, nullptr, TO_CHAR);
        act("$n eats some of $p.", true, ch, food, nullptr, TO_ROOM);
    } else {
        act("You nibble a little bit of $p.", false, ch, food, nullptr, TO_CHAR);
        act("$n tastes a little bit of $p.", true, ch, food, nullptr, TO_ROOM);
        if (GET_OBJ_VAL(food, VAL_FOOD_POISON)) {
            send_to_char(ch, "It has a sickening taste! Better not eat it...");
            return;
        }
    }

    //Calculate how much hunger the character has left
    int foob = maxCapacity - GET_COND(ch, HUNGER);
    amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, VAL_FOOD_FOODVAL) : 1);

    gain_condition(ch, HUNGER, amount);
    if (GET_FOODR(ch) == 0 && subcmd != SCMD_TASTE) {
        ch->incCurST((ch->getMaxST() / 100) * amount);
        GET_FOODR(ch) = 2;
        if (OBJ_FLAGGED(food, ITEM_YUM)) {
            ch->incCurSTPercent(.25);
        }
        send_to_char(ch, "You feel rejuvinated by it.\r\n");
    }

    //Logic for food that will give PS or Exp
    if (GET_OBJ_VNUM(food) >= MEAL_START && GET_OBJ_VNUM(food) <= MEAL_LAST && 
        (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_AL) && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_RHELL))) {
        if (subcmd != SCMD_TASTE) {
            int psbonus = GET_OBJ_VAL(food, 1);
            int expbonus = GET_OBJ_VAL(food, 2) * ((GET_LEVEL(ch) * 0.4) + 1);
            int capped = false, pscapped = false;
            if (level_exp(ch, GET_LEVEL(ch) + 1) - (GET_EXP(ch)) <= 0 && GET_LEVEL(ch) < 100) {
                expbonus = 1;
                capped = true;
            } else {
                expbonus = GET_LEVEL(ch) * 1000;
            }

            if (GET_PRACTICES(ch) >= 1000) {
                psbonus = 0;
                pscapped = true;
            }

            ch->modExperience(expbonus);
            ch->modPractices(psbonus);
            send_to_char(ch, "That was exceptionally delicious! @D[@mPS@D: @C+%d@D] [@gEXP@D: @G+%s@D]@n\r\n",
                         psbonus, add_commas(expbonus).c_str());
            if (capped == true)
                send_to_char(ch, "Experience capped due to negative TNL.\r\n");
            if (pscapped == true)
                send_to_char(ch, "Practice Sessions capped for food at 1000 PS.\r\n");
        }
        //Good food can heal you
        if (!GET_OBJ_VAL(food, VAL_FOOD_POISON) && GET_HIT(ch) < (ch->getEffMaxPL()) && subcmd != SCMD_TASTE) {
            int64_t suppress = ((ch->getEffMaxPL()) * 0.01) * GET_SUPPRESS(ch);
            if (GET_WEIGHT(food) < 6) {
                ch->incCurHealthPercent(.05);
            } else {
                ch->incCurHealthPercent(.1);
            }
            if (OBJ_FLAGGED(food, ITEM_YUM)) {
                ch->incCurHealthPercent(.2);
            }

            send_to_char(ch, "@MYou feel some of your strength return!@n\r\n");
        }
    }

    if (GET_OBJ_VAL(food, VAL_FOOD_POISON) && !ADM_FLAGGED(ch, ADM_NOPOISON)) {
        /* The crap was poisoned ! */
        send_to_char(ch, "Oops, that tasted rather strange!\r\n");
        act("$n coughs and utters some strange sounds.", false, ch, nullptr, nullptr, TO_ROOM);

        af.type = SPELL_POISON;
        af.duration = amount * 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_join(ch, &af, false, false, false, false);
    }

    std::set<obj_vnum> candies = {53, 93, 94, 95};

    if(candies.contains(food->vn)) {
        if(IS_MAJIN(ch)) foob = GET_OBJ_VAL(food, VAL_FOOD_FOODVAL);
        majin_gain(ch, food, foob);
    }

    if (subcmd == SCMD_EAT) {
        if (foob >= GET_OBJ_VAL(food, VAL_FOOD_FOODVAL)) {
            send_to_char(ch, "You finish the last bite.\r\n");
            extract_obj(food);
        } else {
            GET_OBJ_VAL(food, VAL_FOOD_FOODVAL) -= foob;
        }
    } else {
        if (!(--GET_OBJ_VAL(food, VAL_FOOD_FOODVAL))) {
            send_to_char(ch, "There's nothing left now.\r\n");
            extract_obj(food);
        }
    }
}

static void majin_gain(struct char_data *ch, struct obj_data *food, int foob) {
    if (!IS_MAJIN(ch) || IS_NPC(ch)) {
        return;
    }

    auto soft_cap = ch->calc_soft_cap();
    auto current = (ch->getBasePL() + ch->getBaseKI() + ch->getBaseST());

    auto available = soft_cap - current;

    if (available <= 0.0) {
        send_to_char(ch, "You can not gain anymore from candy consumption at your current level.\r\n");
        return;
    }

    int64_t st = food->value[VAL_FOOD_CANDY_ST], ki = food->value[VAL_FOOD_CANDY_KI], pl = food->value[VAL_FOOD_CANDY_PL];

    st *= 0.05;
    ki *= 0.05;
    pl *= 0.05;

    std::vector<int> toAdd = {0, 1, 2};
    Random::shuffle(toAdd);

    // 0: pl, 1: ki, 2: stamina.

    int64_t addPL = 0;
    int64_t addKI = 0;
    int64_t addST = 0;

    for(auto &t : toAdd) {
        switch(t) {
            case 0:
                addPL = std::min(pl, available);
                available -= addPL;
                if(addPL <= 0) addPL = 1;
                break;
            case 1:
                addKI = std::min(ki, available);
                available -= addKI;
                if(addKI <= 0) addKI = 1;
                break;
            case 2:
                addST = std::min(st, available);
                available -= addST;
                if(addST <= 0) addST = 1;
                break;
        }
    }

    ch->gainBasePL(addPL, true);
    ch->gainBaseKI(addKI, true);
    ch->gainBaseST(addST, true);

    send_to_char(ch,
                 "@mYou feel stronger after consuming the candy @D[@RPL@W: @r%s @CKi@D: @c%s @GSt@D: @g%s@D]@m!@n\r\n",
                 add_commas(addPL).c_str(), add_commas(addKI).c_str(), add_commas(addST).c_str());
}

ACMD(do_pour) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct obj_data *from_obj = nullptr, *to_obj = nullptr;
    int amount = 0;

    two_arguments(argument, arg1, arg2);

    if (subcmd == SCMD_POUR) {
        if (!*arg1) {        /* No arguments */
            send_to_char(ch, "From what do you want to pour?\r\n");
            return;
        }
        if (!(from_obj = get_obj_in_list_vis(ch, arg1, nullptr, ch->contents))) {
            send_to_char(ch, "You can't find it!\r\n");
            return;
        }
        if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
            send_to_char(ch, "You can't pour from that!\r\n");
            return;
        }
    }
    if (subcmd == SCMD_FILL) {
        if (!*arg1) {        /* no arguments */
            send_to_char(ch, "What do you want to fill?  And what are you filling it from?\r\n");
            return;
        }
        if (!(to_obj = get_obj_in_list_vis(ch, arg1, nullptr, ch->contents))) {
            send_to_char(ch, "You can't find it!\r\n");
            return;
        }
        if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
            act("You can't fill $p!", false, ch, to_obj, nullptr, TO_CHAR);
            return;
        }
        if (!*arg2) {        /* no 2nd argument */
            act("What do you want to fill $p from?", false, ch, to_obj, nullptr, TO_CHAR);
            return;
        }
        if (!(from_obj = get_obj_in_list_vis(ch, arg2, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
            return;
        }
        if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN && !OBJ_FLAGGED(from_obj, ITEM_BROKEN)) {
            act("You can't fill something from $p.", false, ch, from_obj, nullptr, TO_CHAR);
            return;
        } else if (GET_OBJ_TYPE(from_obj) == ITEM_FOUNTAIN && OBJ_FLAGGED(from_obj, ITEM_BROKEN)) {
            act("You can't fill something from a broken fountain.", false, ch, from_obj, nullptr, TO_CHAR);
            return;
        }
    }
    if (GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL) == 0) {
        act("The $p is empty.", false, ch, from_obj, nullptr, TO_CHAR);
        return;
    }
    if (subcmd == SCMD_POUR) {    /* pour */
        if (!*arg2) {
            send_to_char(ch, "Where do you want it?  Out or in what?\r\n");
            return;
        }
        if (!strcasecmp(arg2, "out")) {
            if (GET_OBJ_VAL(from_obj, VAL_DRINKCON_CAPACITY) > 0) {
                act("$n empties $p.", true, ch, from_obj, nullptr, TO_ROOM);
                act("You empty $p.", false, ch, from_obj, nullptr, TO_CHAR);

                weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL)); /* Empty */

                name_from_drinkcon(from_obj);
                GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL) = 0;
                GET_OBJ_VAL(from_obj, VAL_DRINKCON_LIQUID) = 0;
                GET_OBJ_VAL(from_obj, VAL_DRINKCON_POISON) = 0;
            } else {
                send_to_char(ch, "You can't possibly pour that container out!\r\n");
            }

            return;
        }
        if (!(to_obj = get_obj_in_list_vis(ch, arg2, nullptr, ch->contents))) {
            send_to_char(ch, "You can't find it!\r\n");
            return;
        }
        if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
            (GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
            send_to_char(ch, "You can't pour anything into that.\r\n");
            return;
        }
    }
    if (to_obj == from_obj) {
        send_to_char(ch, "A most unproductive effort.\r\n");
        return;
    }
    if ((GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL) != 0) &&
        (GET_OBJ_VAL(to_obj, VAL_DRINKCON_LIQUID) != GET_OBJ_VAL(from_obj, VAL_DRINKCON_LIQUID))) {
        send_to_char(ch, "There is already another liquid in it!\r\n");
        return;
    }
    if ((GET_OBJ_VAL(to_obj, VAL_DRINKCON_CAPACITY) < 0) ||
        (!(GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL) < GET_OBJ_VAL(to_obj, VAL_DRINKCON_CAPACITY)))) {
        send_to_char(ch, "There is no room for more.\r\n");
        return;
    }
    if (subcmd == SCMD_POUR)
        send_to_char(ch, "You pour the %s into the %s.", drinks[GET_OBJ_VAL(from_obj, VAL_DRINKCON_LIQUID)], arg2);

    if (subcmd == SCMD_FILL) {
        act("You gently fill $p from $P.", false, ch, to_obj, from_obj, TO_CHAR);
        act("$n gently fills $p from $P.", true, ch, to_obj, from_obj, TO_ROOM);
    }
    /* New alias */
    if (GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL) == 0)
        name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, VAL_DRINKCON_LIQUID));

    /* First same type liq. */
    GET_OBJ_VAL(to_obj, VAL_DRINKCON_LIQUID) = GET_OBJ_VAL(from_obj, VAL_DRINKCON_LIQUID);

    /* Then how much to pour */
    if (GET_OBJ_VAL(from_obj, VAL_DRINKCON_CAPACITY) > 0) {
        GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL) -= (amount =
                                                                (GET_OBJ_VAL(to_obj, VAL_DRINKCON_CAPACITY) -
                                                                 GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL)));

        GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL) = GET_OBJ_VAL(to_obj, VAL_DRINKCON_CAPACITY);

        if (GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL) < 0) {    /* There was too little */
            GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL) += GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL);
            amount += GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL);
            name_from_drinkcon(from_obj);
            GET_OBJ_VAL(from_obj, VAL_DRINKCON_HOWFULL) = 0;
            GET_OBJ_VAL(from_obj, VAL_DRINKCON_LIQUID) = 0;
            GET_OBJ_VAL(from_obj, VAL_DRINKCON_POISON) = 0;
        }
    } else {
        GET_OBJ_VAL(to_obj, VAL_DRINKCON_HOWFULL) = GET_OBJ_VAL(to_obj, VAL_DRINKCON_CAPACITY);
    }

    /* Then the poison boogie */
    GET_OBJ_VAL(to_obj, VAL_DRINKCON_POISON) =
            (GET_OBJ_VAL(to_obj, VAL_DRINKCON_POISON) || GET_OBJ_VAL(from_obj, VAL_DRINKCON_POISON));

    /* And the weight boogie for non-eternal from_objects */
    if (GET_OBJ_VAL(from_obj, VAL_DRINKCON_CAPACITY) > 0) {
        weight_change_object(from_obj, -amount);
    }
    weight_change_object(to_obj, amount);    /* Add weight */
}

static void wear_message(struct char_data *ch, struct obj_data *obj, int where) {
    const char *wear_messages[][2] = {
            {"$n lights $p and holds it.",
                    "You light $p and hold it."},

            {"$n slides $p on to $s right ring finger.",
                    "You slide $p on to your right ring finger."},

            {"$n slides $p on to $s left ring finger.",
                    "You slide $p on to your left ring finger."},

            {"$n wears $p around $s neck.",
                    "You wear $p around your neck."},

            {"$n wears $p around $s neck.",
                    "You wear $p around your neck."},

            {"$n wears $p on $s body.",
                    "You wear $p on your body."},

            {"$n wears $p on $s head.",
                    "You wear $p on your head."},

            {"$n puts $p on $s legs.",
                    "You put $p on your legs."},

            {"$n wears $p on $s feet.",
                    "You wear $p on your feet."},

            {"$n puts $p on $s hands.",
                    "You put $p on your hands."},

            {"$n wears $p on $s arms.",
                    "You wear $p on your arms."},

            {"$n straps $p around $s arm as a shield.",
                    "You start to use $p as a shield."},

            {"$n wears $p about $s body.",
                    "You wear $p around your body."},

            {"$n wears $p around $s waist.",
                    "You wear $p around your waist."},

            {"$n puts $p on around $s right wrist.",
                    "You put $p on around your right wrist."},

            {"$n puts $p on around $s left wrist.",
                    "You put $p on around your left wrist."},

            {"$n wields $p.",
                    "You wield $p."},

            {"$n grabs $p.",
                    "You grab $p."},

            {"$n wears $p on $s back.",
                    "You wear $p on your back."},

            {"$n puts $p in $s right ear.",
                    "You put $p in your right ear."},

            {"$n puts $p in $s left ear.",
                    "You put $p in your left ear."},

            {"$n wears $p as a cape.",
                    "You wear $p as a cape."},

            {"$n covers $s left eye with $p.",
                    "You wear $p over your left eye."}

    };

    act(wear_messages[where][0], true, ch, obj, nullptr, TO_ROOM);
    act(wear_messages[where][1], false, ch, obj, nullptr, TO_CHAR);
}

static int hands(struct char_data *ch) {
    int x;

    if (GET_EQ(ch, WEAR_WIELD1)) {
        if (OBJ_FLAGGED(GET_EQ(ch, WEAR_WIELD1), ITEM_2H) ||
            wield_type(get_size(ch), GET_EQ(ch, WEAR_WIELD1)) == WIELD_TWOHAND) {
            x = 2;
        } else
            x = 1;
    } else x = 0;

    if (GET_EQ(ch, WEAR_WIELD2)) {
        if (OBJ_FLAGGED(GET_EQ(ch, WEAR_WIELD2), ITEM_2H) ||
            wield_type(get_size(ch), GET_EQ(ch, WEAR_WIELD2)) == WIELD_TWOHAND) {
            x += 2;
        } else
            x += 1;
    }

    return x;
}

void perform_wear(struct char_data *ch, struct obj_data *obj, int where) {
    /*
   * ITEM_WEAR_TAKE is used for objects that do not require special bits
   * to be put into that position (e.g. you can hold any object, not just
   * an object with a HOLD bit.)
   */

    int wear_bitvectors[] = {
            ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
            ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
            ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
            ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
            ITEM_WEAR_TAKE, ITEM_WEAR_TAKE, ITEM_WEAR_PACK, ITEM_WEAR_EAR,
            ITEM_WEAR_EAR, ITEM_WEAR_SH, ITEM_WEAR_EYE
    };

    const char *already_wearing[] = {
            "You're already using a light.\r\n",
            "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
            "You're already wearing something on both of your ring fingers.\r\n",
            "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
            "You can't wear anything else around your neck.\r\n",
            "You're already wearing something on your body.\r\n",
            "You're already wearing something on your head.\r\n",
            "You're already wearing something on your legs.\r\n",
            "You're already wearing something on your feet.\r\n",
            "You're already wearing something on your hands.\r\n",
            "You're already wearing something on your arms.\r\n",
            "You're already using a shield.\r\n",
            "You're already wearing something about your body.\r\n",
            "You already have something around your waist.\r\n",
            "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
            "You're already wearing something around both of your wrists.\r\n",
            "You're already wielding a weapon.\r\n",
            "You're already holding something.\r\n",
            "You're already wearing something on your back.\r\n",
            "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
            "You're already wearing something in both ears.\r\n",
            "You're already wearing something on your shoulders.\r\n",
            "You're already wearing something as a scouter.\r\n"
    };

    /* first, make sure that the wear position is valid. */
    if (!CAN_WEAR(obj, wear_bitvectors[where])) {
        act("You can't wear $p there.", false, ch, obj, nullptr, TO_CHAR);
        return;
    }
    /* do they even have that wear slot by race? */
    if (!BODY_FLAGGED(ch, where)) {
        send_to_char(ch, "Seems like your body type doesn't really allow that.\r\n");
        return;
    }
    /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
    if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R) ||
        (where == WEAR_EAR_R) || (where == WEAR_WIELD1))
        if (GET_EQ(ch, where))
            where++;

    /* checks for 2H sanity */
    if ((OBJ_FLAGGED(obj, ITEM_2H) || (wield_type(get_size(ch), obj) == WIELD_TWOHAND)) && hands(ch) > 0) {
        send_to_char(ch, "Seems like you might not have enough free hands.\r\n");
        return;
    }
    if (where == WEAR_WIELD2 && PLR_FLAGGED(ch, PLR_THANDW)) {
        send_to_char(ch, "Seems like you might not have enough free hands.\r\n");
        return;
    }

    if (((where == WEAR_WIELD1) ||
         (where == WEAR_WIELD2)) && (hands(ch) > 1)) {
        send_to_char(ch, "Seems like you might not have enough free hands.\r\n");
        return;
    }

    if (GET_EQ(ch, where)) {
        send_to_char(ch, "%s", already_wearing[where]);
        return;
    }

    /* See if a trigger disallows it */
    if (!wear_otrigger(obj, ch, where) || (obj->carried_by != ch))
        return;

    if (GET_OBJ_TYPE(obj) == ITEM_WEAPON && OBJ_FLAGGED(obj, ITEM_CUSTOM)) {
        if (GET_LEVEL(ch) < 20) {
            send_to_char(ch, "You are not experienced enough to hold that.\r\n");
            return;
        }
    }

    if (GET_OBJ_TYPE(obj) != ITEM_LIGHT && GET_OBJ_SIZE(obj) > get_size(ch)) {
        send_to_char(ch, "Seems like it is too big for you.\r\n");
        return;
    }
    if (GET_OBJ_SIZE(obj) < get_size(ch) && GET_OBJ_TYPE(obj) != ITEM_LIGHT) {
        send_to_char(ch, "Seems like it is too small for you.\r\n");
        return;
    }

    wear_message(ch, obj, where);
    obj_from_char(obj);
    equip_char(ch, obj, where);
}

int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg) {
    int where = -1;

    const char *keywords[] = {
            "!RESERVED!",
            "finger",
            "!RESERVED!",
            "neck",
            "!RESERVED!",
            "body",
            "head",
            "legs",
            "feet",
            "hands",
            "arms",
            "shield",
            "about",
            "waist",
            "wrist",
            "!RESERVED!",
            "!RESERVED!",
            "!RESERVED!",
            "back",
            "ear",
            "\r!RESERVED!",
            "shoulders",
            "scouter",
            "\n"
    };

    if (!arg || !*arg) {
        if (CAN_WEAR(obj, ITEM_WEAR_FINGER) && BODY_FLAGGED(ch, WEAR_FINGER_R)) return WEAR_FINGER_R;
        if (CAN_WEAR(obj, ITEM_WEAR_NECK) && BODY_FLAGGED(ch, WEAR_NECK_1)) return WEAR_NECK_1;
        if (CAN_WEAR(obj, ITEM_WEAR_BODY) && BODY_FLAGGED(ch, WEAR_BODY)) return WEAR_BODY;
        if (CAN_WEAR(obj, ITEM_WEAR_HEAD) && BODY_FLAGGED(ch, WEAR_HEAD)) return WEAR_HEAD;
        if (CAN_WEAR(obj, ITEM_WEAR_LEGS) && BODY_FLAGGED(ch, WEAR_LEGS)) return WEAR_LEGS;
        if (CAN_WEAR(obj, ITEM_WEAR_FEET) && BODY_FLAGGED(ch, WEAR_FEET)) return WEAR_FEET;
        if (CAN_WEAR(obj, ITEM_WEAR_HANDS) && BODY_FLAGGED(ch, WEAR_HANDS)) return WEAR_HANDS;
        if (CAN_WEAR(obj, ITEM_WEAR_ARMS) && BODY_FLAGGED(ch, WEAR_ARMS)) return WEAR_ARMS;
        if (CAN_WEAR(obj, ITEM_WEAR_SHIELD) && BODY_FLAGGED(ch, WEAR_WIELD2)) return WEAR_WIELD2;
        if (CAN_WEAR(obj, ITEM_WEAR_ABOUT) && BODY_FLAGGED(ch, WEAR_ABOUT)) return WEAR_ABOUT;
        if (CAN_WEAR(obj, ITEM_WEAR_WAIST) && BODY_FLAGGED(ch, WEAR_WAIST)) return WEAR_WAIST;
        if (CAN_WEAR(obj, ITEM_WEAR_WRIST) && BODY_FLAGGED(ch, WEAR_WRIST_R)) return WEAR_WRIST_R;
        if (CAN_WEAR(obj, ITEM_WEAR_HOLD) && BODY_FLAGGED(ch, WEAR_WIELD2)) return WEAR_WIELD2;
        if (CAN_WEAR(obj, ITEM_WEAR_PACK) && BODY_FLAGGED(ch, WEAR_BACKPACK)) return WEAR_BACKPACK;
        if (CAN_WEAR(obj, ITEM_WEAR_EAR) && BODY_FLAGGED(ch, WEAR_EAR_R)) return WEAR_EAR_R;
        if (CAN_WEAR(obj, ITEM_WEAR_SH) && BODY_FLAGGED(ch, WEAR_SH)) return WEAR_SH;
        if (CAN_WEAR(obj, ITEM_WEAR_EYE) && BODY_FLAGGED(ch, WEAR_EYE)) return WEAR_EYE;
    } else if (((where = search_block(arg, keywords, false)) < 0) || (*arg == '!')) {
        send_to_char(ch, "'%s'?  What part of your body is THAT?\r\n", arg);
        return -1;
    }

    return (where);
}

ACMD(do_wear) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj, *next_obj;
    int where, dotmode, items_worn = 0;

    two_arguments(argument, arg1, arg2);

    if (!*arg1) {
        send_to_char(ch, "Wear what?\r\n");
        return;
    }
    dotmode = find_all_dots(arg1);

    if (*arg2 && (dotmode != FIND_INDIV)) {
        send_to_char(ch, "You can't specify the same body location for more than one item!\r\n");
        return;
    }
    if (dotmode == FIND_ALL) {
        for (obj = ch->contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, nullptr)) >= 0) {
                if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj)) {
                    act("$p: you are not experienced enough to use that.",
                        false, ch, obj, nullptr, TO_CHAR);
                    send_to_char(ch, "You need to be at least %d level to use it.\r\n", GET_OBJ_LEVEL(obj));
                } else if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
                    act("$p: it seems to be broken.", false, ch, obj, nullptr, TO_CHAR);
                } else if (OBJ_FLAGGED(obj, ITEM_FORGED)) {
                    act("$p: it seems to be fake...", false, ch, obj, nullptr, TO_CHAR);
                } else {
                    items_worn++;
                    if (!is_proficient_with_armor(ch, GET_OBJ_VAL(obj, VAL_ARMOR_SKILL))
                        && GET_OBJ_TYPE(obj) == ITEM_ARMOR)
                        send_to_char(ch,
                                     "You have no proficiency with this type of armor.\r\nYour fighting and physical skills will be greatly impeded.\r\n");
                    perform_wear(ch, obj, where);
                }
            }
        }
        if (!items_worn)
            send_to_char(ch, "You don't seem to have anything wearable.\r\n");
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg1) {
            send_to_char(ch, "Wear all of what?\r\n");
            return;
        }
        if (!(obj = get_obj_in_list_vis(ch, arg1, nullptr, ch->contents)))
            send_to_char(ch, "You don't seem to have any %ss.\r\n", arg1);
        else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
            send_to_char(ch, "You are not experienced enough to use that.\r\n");
        else
            while (obj) {
                next_obj = get_obj_in_list_vis(ch, arg1, nullptr, obj->next_content);
                if ((where = find_eq_pos(ch, obj, nullptr)) >= 0) {
                    if (!is_proficient_with_armor(ch, GET_OBJ_VAL(obj, VAL_ARMOR_SKILL))
                        && GET_OBJ_TYPE(obj) == ITEM_ARMOR)
                        send_to_char(ch,
                                     "You have no proficiency with this type of armor.\r\nYour fighting and physical skills will be greatly impeded.\r\n");
                    perform_wear(ch, obj, where);
                } else
                    act("You can't wear $p.", false, ch, obj, nullptr, TO_CHAR);
                obj = next_obj;
            }
    } else {
        if (!(obj = get_obj_in_list_vis(ch, arg1, nullptr, ch->contents)))
            send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
        else if (OBJ_FLAGGED(obj, ITEM_BROKEN))
            send_to_char(ch, "But it seems to be broken!\r\n");
        else if (OBJ_FLAGGED(obj, ITEM_FORGED))
            send_to_char(ch, "But it seems to be fake!\r\n");
        else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
            send_to_char(ch, "You are not experienced enough to use that.\r\n");
        else {
            if ((where = find_eq_pos(ch, obj, arg2)) >= 0) {
                if (!is_proficient_with_armor(ch, GET_OBJ_VAL(obj, VAL_ARMOR_SKILL))
                    && GET_OBJ_TYPE(obj) == ITEM_ARMOR)
                    send_to_char(ch,
                                 "You have no proficiency with this type of armor.\r\nYour fighting and physical skills will be greatly impeded.\r\n");
                perform_wear(ch, obj, where);
            } else if (!*arg2)
                act("You can't wear $p.", false, ch, obj, nullptr, TO_CHAR);
        }
    }
}

ACMD(do_wield) {
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!*arg)
        send_to_char(ch, "Wield what?\r\n");
    else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))
        send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    else {
        if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
            send_to_char(ch, "You can't wield that.\r\n");
        else if (GET_OBJ_WEIGHT(obj) > max_carry_weight(ch))
            send_to_char(ch, "It's too heavy for you to use.\r\n");
        else if (OBJ_FLAGGED(obj, ITEM_BROKEN))
            send_to_char(ch, "But it seems to be broken!\r\n");
        else if (OBJ_FLAGGED(obj, ITEM_FORGED))
            send_to_char(ch, "But it seems to be fake!\r\n");
        else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
            send_to_char(ch, "You are not experienced enough to use that.\r\n");
        else if (PLR_FLAGGED(ch, PLR_THANDW))
            send_to_char(ch, "You are holding a weapon with two hands right now!\r\n");
        else {
            if (!IS_NPC(ch) && !is_proficient_with_weapon(ch, GET_OBJ_VAL(obj, VAL_WEAPON_SKILL))
                && GET_OBJ_TYPE(obj) == ITEM_ARMOR)
                send_to_char(ch,
                             "You have no proficiency with this type of weapon.\r\nYour attack accuracy will be greatly reduced.\r\n");
            perform_wear(ch, obj, WEAR_WIELD1);
        }
    }
}

ACMD(do_grab) {
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!*arg)
        send_to_char(ch, "Hold what?\r\n");
    else if (!(obj = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))
        send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
        send_to_char(ch, "You are not experienced enough to use that.\r\n");
    else if (PLR_FLAGGED(ch, PLR_THANDW))
        send_to_char(ch, "You are wielding a weapon with both hands currently.\r\n");
    else {
        if (GET_OBJ_TYPE(obj) == ITEM_LIGHT) {
            perform_wear(ch, obj, WEAR_WIELD2);
            if (GET_OBJ_VAL(obj, VAL_LIGHT_HOURS) > 0 || GET_OBJ_VAL(obj, VAL_LIGHT_HOURS) < 0) {
                act("@wYou light $p@w.", true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@w lights $p@w.", true, ch, obj, nullptr, TO_ROOM);
            }
            if (GET_OBJ_VAL(obj, VAL_LIGHT_HOURS) == 0) {
                act("@wYou try to light $p@w but it is burnt out.", true, ch, obj, nullptr, TO_CHAR);
                act("@C$n@w tries to light $p@w but nothing happens.", true, ch, obj, nullptr, TO_ROOM);
            }
        } else {
            if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE(obj) != ITEM_WAND &&
                GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
                GET_OBJ_TYPE(obj) != ITEM_POTION)
                send_to_char(ch, "You can't hold that.\r\n");
            else
                perform_wear(ch, obj, WEAR_WIELD2);
        }
    }
}

void perform_remove(struct char_data *ch, int pos) {
    struct obj_data *obj;

    int64_t previous = GET_HIT(ch);

    if (!(obj = GET_EQ(ch, pos)))
        basic_mud_log("SYSERR: perform_remove: bad pos %d passed.", pos);
        /*  SYSERR_DESC:
     *  This error occurs when perform_remove() is passed a bad 'pos'
     *  (location) to remove an object from.
     */
    else if (OBJ_FLAGGED(obj, ITEM_NODROP) && GET_ADMLEVEL(ch) < 1)
        act("You can't remove $p, it must be CURSED!", false, ch, obj, nullptr, TO_CHAR);
    else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
        act("$p: your arms are full!", false, ch, obj, nullptr, TO_CHAR);
    else {
        if (!remove_otrigger(obj, ch))
            return;

        if (pos == WEAR_WIELD1 && PLR_FLAGGED(ch, PLR_THANDW)) {
            ch->playerFlags.reset(PLR_THANDW);
        }
        obj_to_char(unequip_char(ch, pos), ch);
        act("You stop using $p.", false, ch, obj, nullptr, TO_CHAR);
        act("$n stops using $p.", true, ch, obj, nullptr, TO_ROOM);
        if (previous > GET_HIT(ch)) {
            char drop[MAX_INPUT_LENGTH];
            sprintf(drop, "@RYour powerlevel has dropped from removing $p@R! @D[@r-%s@D]\r\n",
                    add_commas(previous - GET_HIT(ch)).c_str());
            act(drop, false, ch, obj, nullptr, TO_CHAR);
        }
    }
}

ACMD(do_remove) {
    struct obj_data *obj;
    char arg[MAX_INPUT_LENGTH];
    int i, dotmode, found = 0, msg;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Remove what?\r\n");
        return;
    }

    auto isBoard = [&](const auto &o) {return GET_OBJ_TYPE(o) == ITEM_BOARD;};

    obj = ch->findObject(isBoard);
    /* lemme check for a board FIRST */
    if (!obj) ch->getRoom()->findObject(isBoard);

    if (obj) {
        if (!isdigit(*arg) || (!(msg = atoi(arg)))) {
            found = 0;
        } else {
            remove_board_msg(GET_OBJ_VNUM (obj), ch, msg);
        }
    }
    if(found) return;

    dotmode = find_all_dots(arg);

    if (dotmode == FIND_ALL) {
        found = 0;
        for (i = 0; i < NUM_WEARS; i++) {
            if (GET_EQ(ch, i)) {
                perform_remove(ch, i);
                found = 1;
            }
        }
        if (!found) {
            send_to_char(ch, "You're not using anything.\r\n");
        }
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg) {
            send_to_char(ch, "Remove all of what?\r\n");
        } else {
            found = 0;
            for (i = 0; i < NUM_WEARS; i++) {
                if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
                    isname(arg, GET_EQ(ch, i)->name)) {
                    perform_remove(ch, i);
                    found = 1;
                }
            }
            if (!found) {
                send_to_char(ch, "You don't seem to be using any %ss.\r\n", arg);
            }
        }
    } else {
        if ((i = get_obj_pos_in_equip_vis(ch, arg, nullptr, ch->equipment)) < 0) {
            send_to_char(ch, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
        } else {
            perform_remove(ch, i);
        }
    }
}

ACMD(do_sac) {
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *j;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Sacrifice what?\r\n");
        return;
    }

    if (!(j = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents)) &&
        (!(j = get_obj_in_list_vis(ch, arg, nullptr, ch->contents)))) {
        send_to_char(ch, "It doesn't seem to be here.\r\n");
        return;
    }

    if (!CAN_WEAR(j, ITEM_WEAR_TAKE)) {
        send_to_char(ch, "You can't sacrifice that!\r\n");
        return;
    }

    act("$n sacrifices $p.", false, ch, j, nullptr, TO_ROOM);

    if (!GET_OBJ_COST(j) && !IS_CORPSE(j)) {
        send_to_char(ch, "Zizazat mocks your sacrifice. Try again, try harder.\r\n");
        return;
    }

    if (!IS_CORPSE(j)) {
        switch (rand_number(0, 5)) {
            case 0:
                send_to_char(ch, "You sacrifice %s to the Gods.\r\nYou receive one zenni for your humility.\r\n",
                             GET_OBJ_SHORT(j));
                ch->mod(CharMoney::Carried, 1);
                break;
            case 1:
                send_to_char(ch, "You sacrifice %s to the Gods.\r\nThe Gods ignore your sacrifice.\r\n",
                             GET_OBJ_SHORT(j));
                break;
            case 2:
                send_to_char(ch, "You sacrifice %s to the Gods.\r\nZizazat gives you %d experience points.\r\n",
                             GET_OBJ_SHORT(j), (2 * GET_OBJ_COST(j)));
                ch->modExperience(2 * GET_OBJ_COST(j));
                break;
            case 3:
                send_to_char(ch, "You sacrifice %s to the Gods.\r\nYou receive %d experience points.\r\n",
                             GET_OBJ_SHORT(j), GET_OBJ_COST(j));
                ch->modExperience(GET_OBJ_COST(j));
                break;
            case 4:
                send_to_char(ch, "Your sacrifice to the Gods is rewarded with %d zenni.\r\n", GET_OBJ_COST(j));
                ch->mod(CharMoney::Carried, GET_OBJ_COST(j));
                break;
            case 5:
                send_to_char(ch, "Your sacrifice to the Gods is rewarded with %d zenni\r\n", (2 * GET_OBJ_COST(j)));
                ch->mod(CharMoney::Carried, (2 * GET_OBJ_COST(j)));
                break;
            default:
                send_to_char(ch, "You sacrifice %s to the Gods.\r\nYou receive one zenni for your humility.\r\n",
                             GET_OBJ_SHORT(j));
                ch->mod(CharMoney::Carried, 1);
                break;
        }
    } else {
        /* No longer transfer corpse contents to room. Sac it, sac it all. */
        send_to_char(ch, "You send the corpse on to the next life!\r\n");
    }
    extract_obj(j);
}

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
const int max_carry_load[] = {
        0,
        10,
        20,
        30,
        40,
        50,
        60,
        70,
        80,
        90,
        100,
        115,
        130,
        150,
        175,
        200,
        230,
        260,
        300,
        350,
        400,
        460,
        520,
        600,
        700,
        800,
        920,
        1040,
        1200,
        1400,
        1640,
};

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
int64_t max_carry_weight(struct char_data *ch) {
    int64_t abil;
    int total;
/*  abil = MAX(0, MIN(100, GET_STR(ch)));
  total = 1;
  while (abil > 30) {
    abil -= 10;
    total *= 4;
  }*/

    abil = (GET_MAX_HIT(ch) / 200) + (GET_STR(ch) * 50);
    total = 1;
    return (total * abil);
}
