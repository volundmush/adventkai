/*************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "dbat/act.movement.h"
#include "dbat/dg_comm.h"
#include "dbat/vehicles.h"
#include "dbat/oasis_copy.h"
#include "dbat/handler.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/fight.h"
#include "dbat/spells.h"
#include "dbat/oasis.h"
#include "dbat/guild.h"
#include "dbat/dg_scripts.h"
#include "dbat/local_limits.h"
#include "dbat/house.h"
#include "dbat/constants.h"
#include "dbat/class.h"
#include "dbat/act.informative.h"
#include <boost/algorithm/string.hpp>

/* local functions */
static void handle_fall(struct char_data *ch);

static int check_swim(struct char_data *ch);

static void disp_locations(struct char_data *ch, vnum areaVnum, std::set<room_vnum>& rooms);

static int has_boat(struct char_data *ch);

static int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname);

static int has_key(struct char_data *ch, obj_vnum key);

static void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd);

static int ok_pick(struct char_data *ch, obj_vnum keynum, int pickproof, int dclock, int scmd, struct obj_data *obj);

static int has_flight(struct char_data *ch);

static int do_simple_enter(struct char_data *ch, struct obj_data *obj, int need_specials_check);

static int perform_enter_obj(struct char_data *ch, struct obj_data *obj, int need_specials_check);

static int do_simple_leave(struct char_data *ch, struct obj_data *obj, int need_specials_check);

static int perform_leave_obj(struct char_data *ch, struct obj_data *obj, int need_specials_check);

static int64_t calcNeedMovementGravity(struct char_data *ch) {
    if(IS_NPC(ch)) return 0.0;
    auto gravity = ch->currentGravity();
    return (gravity * gravity) * ch->getBurdenRatio();
}

/* This handles teleporting players with instant transmission or skills like it. */
void handle_teleport(struct char_data *ch, struct char_data *tar, int location) {
    int success = false;

    if (location != 0) { /* Teleport to a particular room */
        char_from_room(ch);
        char_to_room(ch, real_room(location));
        success = true;
    } else if (tar != nullptr) { /* Teleport to a particular character */
        char_from_room(ch);
        char_to_room(ch, IN_ROOM(tar));
        success = true;
    }

    if (success == true) { /* We have made it. */
        act("@w$n@w appears in an instant out of nowhere!@n", true, ch, nullptr, nullptr, TO_ROOM);
        if (DRAGGING(ch) && !IS_NPC(DRAGGING(ch))) {
            char_from_room(DRAGGING(ch));
            char_to_room(DRAGGING(ch), IN_ROOM(ch));
            act("@w$n@w appears in an instant out of nowhere being dragged by $N!@n", true, DRAGGING(ch), nullptr, ch,
                TO_NOTVICT);
        }
        if (GRAPPLING(ch) && !IS_NPC(GRAPPLING(ch))) {
            char_from_room(GRAPPLING(ch));
            char_to_room(GRAPPLING(ch), IN_ROOM(ch));
            act("@w$n@w appears in an instant out of nowhere being grappled by $N!@n", true, GRAPPLING(ch), nullptr, ch,
                TO_NOTVICT);
        }
        if (CARRYING(ch)) {
            char_from_room(CARRYING(ch));
            char_to_room(CARRYING(ch), IN_ROOM(ch));
            act("@w$n@w appears in an instant out of nowhere being carried by $N!@n", true, CARRYING(ch), nullptr, ch,
                TO_NOTVICT);
        }
        if (GRAPPLED(ch) && !IS_NPC(GRAPPLED(ch))) {
            char_from_room(GRAPPLED(ch));
            char_to_room(GRAPPLED(ch), IN_ROOM(ch));
            act("@w$n@w appears in an instant out of nowhere being grappled by $N!@n", true, GRAPPLED(ch), nullptr, ch,
                TO_NOTVICT);
        }
        if (DRAGGING(ch) && IS_NPC(DRAGGING(ch))) {
            act("@WYou stop dragging @C$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
            act("@C$n@W stops dragging @c$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
            DRAGGED(DRAGGING(ch)) = nullptr;
            DRAGGING(ch) = nullptr;
        }
        if (GRAPPLING(ch) && IS_NPC(GRAPPLING(ch))) {
            GRAPTYPE(GRAPPLING(ch)) = -1;
            GRAPPLED(GRAPPLING(ch)) = nullptr;
            GRAPPLING(ch) = nullptr;
            GRAPTYPE(ch) = -1;
        }
        if (GRAPPLED(ch) && IS_NPC(GRAPPLED(ch))) {
            GRAPTYPE(GRAPPLED(ch)) = -1;
            GRAPPLING(GRAPPLED(ch)) = nullptr;
            GRAPPLED(ch) = nullptr;
            GRAPTYPE(ch) = -1;
        }
    } else { /* Wut... */
        basic_mud_log("ERROR: handle_teleport called without a destination.");
        return;
    }

}

/* Let's carry someone! Why not? - Iovan */
ACMD(do_carry) {

    if (IS_NPC(ch))
        return;

    struct char_data *vict = nullptr;
    char arg[MAX_INPUT_LENGTH];

    if (DRAGGING(ch)) {
        send_to_char(ch, "You are busy dragging someone at the moment.\r\n");
        return;
    }

    if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "You are busy piloting a ship!\r\n");
        return;
    }

    if (CARRYING(ch)) { /* Already carrying someone. Put them down. Simple and clean. */
        if (GET_ALIGNMENT(ch) > 50) {
            carry_drop(ch, 0);
        } else {
            carry_drop(ch, 1);
        }
        return;
    } else { /* So not carrying already. */
        one_argument(argument, arg);

        if (!*arg) {
            send_to_char(ch, "You want to carry who?\r\n");
            return;
        }

        if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM))) {
            send_to_char(ch, "That person isn't here.\r\n");
            return;
        }

        if (IS_NPC(vict)) {
            send_to_char(ch, "There's no point in carrying them.\r\n");
            return;
        }

        if (CARRIED_BY(vict) != nullptr) {
            send_to_char(ch, "Someone is already carrying them!\r\n");
            return;
        }

        if (GET_POS(vict) > POS_SLEEPING) {
            send_to_char(ch, "They are not unconcious.\r\n");
            return;
        }

        if (vict->getTotalWeight() > CAN_CARRY_W(ch)) {
            act("@WYou try to pick up @C$N@W but have to put them down. They are too heavy for you at the moment.@n",
                true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W tries to pick up @c$N@W. After struggling for a moment $e has to put $M down.@n", true, ch,
                nullptr, vict, TO_NOTVICT);
            WAIT_STATE(ch, PULSE_1SEC);
            return;
        } else { /* Let's carry that mofo! */
            act("@WYou pick up @C$N@W and put $M over your shoulder.@n", true, ch, nullptr, vict, TO_CHAR);
            act("@C$n@W picks up $c$N@W and puts $M over $s shoulder.@n", true, ch, nullptr, vict, TO_NOTVICT);
            if (SITS(vict)) {
                struct obj_data *chair = SITS(vict);
                SITTING(chair) = nullptr;
                SITS(vict) = nullptr;
            }
            CARRYING(ch) = vict;
            CARRIED_BY(vict) = ch;
            WAIT_STATE(ch, PULSE_1SEC);
            return;
        }

    } /* End new carry target. */
}

/* Handles dropping someone you are carrying. */
void carry_drop(struct char_data *ch, int type) {


    struct char_data *vict = nullptr;

    vict = CARRYING(ch);

    switch (type) {
        case 0: /* Awww we were gentle >.> */
            act("@WYou gently set @C$N@W down on the ground.@n", true, ch, nullptr, vict, TO_CHAR);
            act("@C$n @Wgently sets you down on the ground.@n", true, ch, nullptr, vict, TO_VICT);
            act("@C$n @Wgently sets @c$N@W down on the ground.@n", true, ch, nullptr, vict, TO_NOTVICT);
            break;
        case 1: /* We're not super nice. */
            act("@WYou set @C$N@W hastily onto the ground.@n", true, ch, nullptr, vict, TO_CHAR);
            act("@C$n @Wsets you hastily onto the ground.@n", true, ch, nullptr, vict, TO_VICT);
            act("@C$n @Wsets @c$N@W hastily onto the ground.@n", true, ch, nullptr, vict, TO_NOTVICT);
            break;
        case 2: /* Uh oh we dropped them from being hit! */
            act("@WYou have @C$N@W knocked out of your arms and onto the ground!@n", true, ch, nullptr, vict, TO_CHAR);
            act("@WYou are knocked out of @C$n's@W arms and onto the ground!@n", true, ch, nullptr, vict, TO_VICT);
            act("@C$n @Whas @c$N@W knocked out of $s arms and onto the ground!@n", true, ch, nullptr, vict, TO_NOTVICT);
            break;
        case 3: /* Uh oh they are being extracted! */
            act("@WYou stop carrying @C$N@W for some reason.@n", true, ch, nullptr, vict, TO_CHAR);
            act("@C$n @Wstops carrying you for some reason.@n", true, ch, nullptr, vict, TO_VICT);
            act("@C$n @Wstops carrying @c$N@W for some reason.@n", true, ch, nullptr, vict, TO_NOTVICT);
            break;
    }
    CARRYING(ch) = nullptr;
    CARRIED_BY(vict) = nullptr;
}

std::optional<room_vnum> land_location(char *arg, std::set<room_vnum>& rooms) {
    std::vector<std::pair<room_vnum, std::string>> names;
    for(auto r : rooms) {
        auto room = world.find(r);
        if(room == world.end()) continue;
        names.emplace_back(r, processColors(room->second.name, false, nullptr));
    }

    std::sort(names.begin(), names.end(), [](const std::pair<room_vnum, std::string>& a, const std::pair<room_vnum, std::string>& b) {
        return a.second < b.second;
    });

    for(auto& name : names) {
        if(boost::istarts_with(name.second, arg)) {
            return name.first;
        }
    }
    return std::nullopt;

}

std::optional<vnum> governingAreaTypeFor(struct room_data *rd, std::function<bool(area_data&)>& func) {
    if(!rd->area) return std::nullopt;
    auto &a = areas[rd->area.value()];
    while(true) {
        if (func(a)) return a.vn;
        if ((a.type == AreaType::Structure || a.type == AreaType::Vehicle) && a.extraVn) {
            // we need to find the a.objectVnum in the world by scanning object_list...
            if (auto obj = get_obj_num(a.extraVn.value()); obj) {
                return governingAreaTypeFor(obj, func);
            }
        }
        if (!a.parent) return std::nullopt;
        a = areas[a.parent.value()];
    }
}

std::optional<vnum> governingAreaTypeFor(struct char_data *ch, std::function<bool(area_data&)>& func) {
    auto room = ch->getRoom();
    if(!room) return std::nullopt;
    return governingAreaTypeFor(room, func);
}

std::optional<vnum> governingAreaTypeFor(struct obj_data *obj, std::function<bool(area_data&)>& func) {
	auto room = obj->getAbsoluteRoom();
    if(!room) return std::nullopt;
    return governingAreaTypeFor(room, func);
}

std::size_t recurseScanRooms(area_data &start, std::set<room_vnum>& fill, std::function<bool(room_data&)>& func) {
    std::size_t count = 0;
    for(auto r : start.rooms) {
        auto room = world.find(r);
        if(room == world.end()) continue;
        if(func(room->second)) {
            fill.insert(r);
            count++;
        }
    }
    for(auto &child : start.children) {
        count += recurseScanRooms(areas[child], fill, func);
    }
    return count;
}

/* This shows the player what locations the planet has to land at. */
static void disp_locations(struct char_data *ch, vnum areaVnum, std::set<room_vnum>& rooms) {
	auto &a = areas[areaVnum];
    if(rooms.empty()) {
        send_to_char(ch, "There are no landing locations on this planet.\r\n");
        return;
    }

    std::vector<std::string> names;
    for(auto r : rooms) {
        auto room = world.find(r);
        if(room == world.end()) continue;
        names.emplace_back(room->second.name);
    }
    // Sort the names vector...
    std::sort(names.begin(), names.end());
    send_to_char(ch, "@D------------------[ %s ]@D------------------\n", a.name.c_str());
    for(auto &name : names) {
        send_to_char(ch, "%s\n", name.c_str());
    }
}

ACMD(do_land) {

    int above_planet = true, inroom = GET_ROOM_VNUM(IN_ROOM(ch));
    skip_spaces(&argument);
    std::function<bool(area_data&)> governingCelestial = [&](area_data& area) {
        return area.type == AreaType::CelestialBody;
    };
    auto onPlanet = governingAreaTypeFor(ch, governingCelestial);

    std::set<room_vnum> rooms;
    std::function<bool(room_data&)> scan = [&](room_data &r) {
        return r.room_flags.test(ROOM_LANDING);
    };
    std::size_t count = 0;

    if(onPlanet) {
        auto &a = areas[onPlanet.value()];
        count = recurseScanRooms(a, rooms, scan);
        above_planet = (a.extraVn && inroom == a.extraVn.value());
    } else {
        above_planet = false;
    }

    if (!*argument) {
        if (above_planet == true) {
            send_to_char(ch, "Land where?\n");
            disp_locations(ch, onPlanet.value(), rooms);
            return;
        } else {
            send_to_char(ch, "You are not even in the lower atmosphere of a planet!\r\n");
            return;
        }
    }

    auto checkLanding = land_location(argument, rooms);
    if(!checkLanding) {
        send_to_char(ch, "You can't land there.\r\n");
        return;
    }
    auto landing = checkLanding.value();

    if (landing != NOWHERE) {
        auto was_in = GET_ROOM_VNUM(IN_ROOM(ch));
        auto &r = world[landing];
        send_to_char(ch,
                     "You descend through the upper atmosphere, and coming down through the clouds you land quickly on the ground below.\r\n");
        std::string landName = "UNKNOWN";
        if(r.area) {
            auto &a = areas[r.area.value()];
            landName = a.name;
        }
        char sendback[MAX_INPUT_LENGTH];
        sprintf(sendback, "@C$n@Y flies down through the atmosphere toward @G%s@Y!@n", landName.c_str());
        act(sendback, true, ch, nullptr, nullptr, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, real_room(landing));
        fly_planet(landing, "can be seen landing from space nearby!@n\r\n", ch);
        send_to_sense(1, "landing on the planet", ch);
        send_to_scouter("A powerlevel signal has been detected landing on the planet", ch, 0, 1);
        act("$n comes down from high above in the sky and quickly lands on the ground.", true, ch, nullptr, nullptr,
            TO_ROOM);
        return;
    }
}


/* simple function to determine if char can walk on water */
static int has_boat(struct char_data *ch) {
    struct obj_data *obj;
    int i;

/*
  if (ROOM_IDENTITY(IN_ROOM(ch)) == DEAD_SEA)
    return (1);
*/

    if (ADM_FLAGGED(ch, ADM_WALKANYWHERE) || GET_ADMLEVEL(ch) > 4)
        return (1);

    if (AFF_FLAGGED(ch, AFF_WATERWALK))
        return (1);

    /* non-wearable boats in inventory will do it */
    auto isBoat = [](const auto& o) {return GET_OBJ_TYPE(o) == ITEM_BOAT;};
    if(ch->findObject(isBoat)) return true;

    return false;
}

/* simple function to determine if char can fly */
static int has_flight(struct char_data *ch) {
    struct obj_data *obj;

    if (ADM_FLAGGED(ch, ADM_WALKANYWHERE))
        return (1);

    if (AFF_FLAGGED(ch, AFF_FLYING) &&
        (ch->getCurKI()) >= (GET_LEVEL(ch) + (GET_MAX_MANA(ch) / (GET_LEVEL(ch) * 30))) && !IS_ANDROID(ch) &&
        !IS_NPC(ch)) {
        return (1);
    }
    if (AFF_FLAGGED(ch, AFF_FLYING) && (ch->getCurKI()) < (GET_LEVEL(ch) + (GET_MAX_MANA(ch) / (GET_LEVEL(ch) * 30))) &&
        !IS_ANDROID(ch) && !IS_NPC(ch)) {
        act("@WYou crash to the ground, too tired to fly anymore!@n", true, ch, nullptr, nullptr, TO_CHAR);
        act("@W$n@W crashes to the ground!@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->affected_by.reset(AFF_FLYING);
        handle_fall(ch);
        return (0);
    }
    if (AFF_FLAGGED(ch, AFF_FLYING) && IS_ANDROID(ch)) {
        return (1);
    }
    if (AFF_FLAGGED(ch, AFF_FLYING) && IS_NPC(ch)) {
        return (1);
    }

    /* flying items in inventory will do it */
    auto givesFlight = [](const auto&o) {return OBJAFF_FLAGGED(o, AFF_FLYING);};
    return ch->findObject(givesFlight) != nullptr;
}

/* simple function to determine if char can breathe non-o2 */
int has_o2(struct char_data *ch) {
    if (ADM_FLAGGED(ch, ADM_WALKANYWHERE))
        return (1);

    if (AFF_FLAGGED(ch, AFF_WATERBREATH))
        return (1);

    if (IS_KANASSAN(ch) || IS_ANDROID(ch) || IS_ICER(ch) || IS_MAJIN(ch))
        return (1);

    return (0);
}

/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */

int do_simple_move(struct char_data *ch, int dir, int need_specials_check) {
    char throwaway[MAX_INPUT_LENGTH] = ""; /* Functions assume writable. */
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    room_rnum was_in = IN_ROOM(ch);
    int need_movement;
    struct room_data *rm;

    /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
    if (need_specials_check && special(ch, dir + 1, throwaway))
        return (0);

    /* blocked by a leave trigger ? */
    if (!leave_mtrigger(ch, dir) || IN_ROOM(ch) != was_in) /* prevent teleport crashes */
        return 0;
    if (!leave_wtrigger(ch->getRoom(), ch, dir) || IN_ROOM(ch) != was_in) /* prevent teleport crashes */
        return 0;
    if (!leave_otrigger(ch->getRoom(), ch, dir) || IN_ROOM(ch) != was_in) /* prevent teleport crashes */
        return 0;
    /* charmed? */
    if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master && IN_ROOM(ch) == IN_ROOM(ch->master)) {
        send_to_char(ch, "The thought of leaving your master makes you weep.\r\n");
        act("$n bursts into tears.", false, ch, nullptr, nullptr, TO_ROOM);
        return (0);
    }

    auto r = ch->getRoom();
    auto e = r->dir_option[dir];
    auto dest = e->getDestination();

    int willfall = false;
    /* if this room or the one we're going to needs flight, check for it */
    if ((r->sector_type == SECT_FLYING) || (dest->sector_type == SECT_FLYING)) {
        if (!has_flight(ch)) {
            if (dir != 4) {
                willfall = true;
            } else {
                send_to_char(ch, "You need to fly to go there!\r\n");
                return (0);
            }
        }
    }

    if (((r->sector_type == SECT_WATER_NOSWIM) || (dest->sector_type == SECT_WATER_NOSWIM)) &&
        IS_HUMANOID(ch)) {
        if (IS_KANASSAN(ch) && !has_flight(ch)) {
            act("@CYou swim swiftly.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C swims swiftly.@n", true, ch, nullptr, nullptr, TO_ROOM);
        } else if (IS_ICER(ch) && !has_flight(ch)) {
            act("@CYou swim swiftly.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@c$n@C swims swiftly.@n", true, ch, nullptr, nullptr, TO_ROOM);
        } else if (!IS_KANASSAN(ch) && !IS_ICER(ch) && !has_flight(ch)) {
            if (!check_swim(ch)) {
                return (0);
            } else {
                act("@CYou swim through the cold water.@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@c$n@C swim through the cold water.@n", true, ch, nullptr, nullptr, TO_ROOM);
                WAIT_STATE(ch, PULSE_1SEC);
            }
        }
    }

    if (r->room_flags.test(ROOM_SPACE)) {
        if (!IS_ANDROID(ch)) {
            if (!check_swim(ch)) {
                return (0);
            }
        }
    }

    if (dest->geffect == 6 && !IS_HUMANOID(ch) && IS_NPC(ch)) {
        return (0);
    }

    if (IS_NPC(ch) && dest->room_flags.test(ROOM_NOMOB) && !ch->master) {
        return (0);
    }

    if (r->isSunken() || dest->isSunken()) {
        if (!has_o2(ch) &&
            ((group_bonus(ch, 2) != 10 && (ch->getCurKI()) < GET_MAX_MANA(ch) / 200) || (group_bonus(ch, 2) == 10 &&
                                                                                         (ch->getCurKI()) <
                                                                                         GET_MAX_MANA(ch) / 800))) {
            if (ch->decCurHealthPercent(0.05) > 0) {
                send_to_char(ch, "@RYou struggle to breath!@n\r\n");
            }
            else {
                send_to_char(ch, "@rYou drown!@n\r\n");
                die(ch, nullptr);
                return (0);
            }
        }
        if (!has_o2(ch) &&
            ((group_bonus(ch, 2) != 10 && (ch->getCurKI()) >= GET_MAX_MANA(ch) / 200) || (group_bonus(ch, 2) == 10 &&
                                                                                          (ch->getCurKI()) >=
                                                                                          GET_MAX_MANA(ch) / 800))) {
            send_to_char(ch, "@CYou hold your breath!@n\r\n");
            if (group_bonus(ch, 2) == 10) {
                ch->decCurKI(ch->getMaxKI() / 800);
            } else {
                ch->decCurKI(ch->getMaxKI() / 200);
            }
        }
    }

    /* move points needed is avg. move loss for src and destination sect type */
    if(!IS_NPC(ch)) {
        auto gravity = r->getGravity();
        need_movement = (gravity * gravity) * ch->getBurdenRatio();
    }

    if (GET_LEVEL(ch) <= 1) {
        need_movement = 0;
    }
    /* Stealth increases your move cost, less if you are good at it */
    if (AFF_FLAGGED(ch, AFF_HIDE))
        need_movement *= ((roll_skill(ch, SKILL_HIDE) > 15) ? 2 : 4);

    if (AFF_FLAGGED(ch, AFF_SNEAK))
        need_movement *= ((roll_skill(ch, SKILL_MOVE_SILENTLY) > 15) ? 1.2 : 2);

    int flight_cost = 0;

    if (AFF_FLAGGED(ch, AFF_FLYING) && !IS_ANDROID(ch)) {
        if (!GET_SKILL(ch, SKILL_CONCENTRATION) && !GET_SKILL(ch, SKILL_FOCUS)) {
            flight_cost = GET_MAX_MANA(ch) / 100;
        } else if (GET_SKILL(ch, SKILL_CONCENTRATION) && !GET_SKILL(ch, SKILL_FOCUS)) {
            flight_cost = GET_MAX_MANA(ch) / (GET_SKILL(ch, SKILL_CONCENTRATION) * 2);
        } else if (!GET_SKILL(ch, SKILL_CONCENTRATION) && GET_SKILL(ch, SKILL_FOCUS)) {
            flight_cost = GET_MAX_MANA(ch) / (GET_SKILL(ch, SKILL_FOCUS) * 3);
        } else {
            flight_cost =
                    GET_MAX_MANA(ch) / ((GET_SKILL(ch, SKILL_CONCENTRATION) * 2) + (GET_SKILL(ch, SKILL_FOCUS) * 3));
        }
    }

    if (AFF_FLAGGED(ch, AFF_FLYING) && ((ch->getCurKI()) < flight_cost) && !IS_ANDROID(ch)) {
        ch->decCurKI(flight_cost);
        act("@WYou crash to the ground, too tired to fly anymore!@n", true, ch, nullptr, nullptr, TO_CHAR);
        act("@W$n@W crashes to the ground!@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->affected_by.reset(AFF_FLYING);
    } else if (AFF_FLAGGED(ch, AFF_FLYING) && !IS_ANDROID(ch)) {
        ch->decCurKI(flight_cost);
    }

    if ((ch->getCurST()) < need_movement && !AFF_FLAGGED(ch, AFF_FLYING) && !IS_NPC(ch)) {
        if (need_specials_check && ch->master) {
            send_to_char(ch, "You are too exhausted to follow.\r\n");
        } else {
            send_to_char(ch, "You are too exhausted.\r\n");
        }

        return (0);
    }

    /* Check if the character needs a skill check to go that way. */
    if (e->dcskill != 0) {
        if (e->dcmove > roll_skill(ch, e->dcskill)) {
            send_to_char(ch, "Your skill in %s isn't enough to move that way!\r\n",
                         spell_info[e->dcskill].name);
            /* A failed skill check still spends the movement points! */
            if (!ADM_FLAGGED(ch, ADM_WALKANYWHERE) && !IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_FLYING))
                ch->decCurST(need_movement);
            return (0);
        } else {
            send_to_char(ch, "Your skill in %s aids in your movement.\r\n", spell_info[e->dcskill].name);
        }
    }

    if (r->room_flags.test(ROOM_ATRIUM)) {
        if (!House_can_enter(ch, GET_ROOM_VNUM(e->to_room))) {
            send_to_char(ch, "That's private property -- no trespassing!\r\n");
            return (0);
        }
    }
    if (dest->room_flags.test(ROOM_TUNNEL) && (num_pc_in_room(dest) >= CONFIG_TUNNEL_SIZE)) {
        if (CONFIG_TUNNEL_SIZE > 1)
            send_to_char(ch, "There isn't enough room for you to go there!\r\n");
        else
            send_to_char(ch, "There isn't enough room there for more than one person!\r\n");
        return (0);
    }
    /* Mortals and low level gods cannot enter greater god rooms. */
    if (dest->room_flags.test(ROOM_GODROOM) &&
        GET_ADMLEVEL(ch) < ADMLVL_GRGOD) {
        send_to_char(ch, "You aren't godly enough to use that room!\r\n");
        return (0);
    }

    /******* Zone flag checks *******/

    rm = dest;

    if (!IS_NPC(ch) && (GET_ADMLEVEL(ch) < ADMLVL_IMMORT) &&
        (GET_LEVEL(ch) < ZONE_MINLVL(rm->zone)) && (ZONE_MINLVL(rm->zone) > 0)) {
        send_to_char(ch, "Sorry, you are too low a level to enter this zone.\r\n");
        return (0);
    }

    if ((GET_ADMLEVEL(ch) < ADMLVL_IMMORT) && (GET_LEVEL(ch) > ZONE_MAXLVL(rm->zone)) &&
        (ZONE_MAXLVL(rm->zone) > 0)) {
        send_to_char(ch, "Sorry, you are too high a level to enter this zone.\r\n");
        return (0);
    }

    if ((GET_ADMLEVEL(ch) < ADMLVL_IMMORT) && ZONE_FLAGGED(rm->zone, ZONE_CLOSED)) {
        send_to_char(ch, "This zone is currently closed to mortals.\r\n");
        return (0);
    }

    if ((GET_ADMLEVEL(ch) >= ADMLVL_IMMORT && GET_ADMLEVEL(ch) < ADMLVL_GRGOD)
        && ZONE_FLAGGED(rm->zone, ZONE_NOIMMORT)) {
        send_to_char(ch, "This zone is closed to all.\r\n");
        return (0);
    }

    /* No low level immortal scouting */
    if ((GET_ADMLEVEL(ch) >= ADMLVL_IMMORT && GET_ADMLEVEL(ch) < ADMLVL_GOD) &&
        !can_edit_zone(ch, rm->zone) && ZONE_FLAGGED(rm->zone, ZONE_QUEST)) {
        send_to_char(ch, "This is a Quest zone.\r\n");
        return (0);
    }

    /* Now we know we're allowed to go into the room. */
    if (!ADM_FLAGGED(ch, ADM_WALKANYWHERE) && !IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_FLYING)) {
        ch->decCurST(need_movement);
    }

    if (AFF_FLAGGED(ch, AFF_SNEAK) && !IS_NPC(ch)) {
        sprintf(buf2, "$n sneaks %s.", dirs[dir]);
        if (GET_SKILL(ch, SKILL_MOVE_SILENTLY)) {
            improve_skill(ch, SKILL_MOVE_SILENTLY, 0);
        } else if (slot_count(ch) + 1 > GET_SLOTS(ch)) {
            send_to_char(ch, "@RYour skill slots are full. You can not learn Move Silently.\r\n");
            ch->affected_by.reset(AFF_SNEAK);
        } else {
            send_to_char(ch, "@GYou learn the very basics of moving silently.@n\r\n");
            SET_SKILL(ch, SKILL_MOVE_SILENTLY, rand_number(5, 10));
            act(buf2, true, ch, nullptr, nullptr, TO_ROOM | TO_SNEAKRESIST);
            if (GET_DEX(ch) < rand_number(1, 30)) {
                WAIT_STATE(ch, PULSE_1SEC);
            }
        }
    }

    if (!AFF_FLAGGED(ch, AFF_SNEAK) && !AFF_FLAGGED(ch, AFF_FLYING)) {
        sprintf(buf2, "$n leaves %s.", dirs[dir]);
        act(buf2, true, ch, nullptr, nullptr, TO_ROOM);
    }
    if (!AFF_FLAGGED(ch, AFF_SNEAK) && AFF_FLAGGED(ch, AFF_FLYING)) {
        sprintf(buf2, "$n flies %s.", dirs[dir]);
        act(buf2, true, ch, nullptr, nullptr, TO_ROOM);
    }

    was_in = IN_ROOM(ch);
    if (DRAGGING(ch)) {
        act("@C$n@w drags @c$N@w with $m.@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
    }
    if (CARRYING(ch)) {
        act("@C$n@w carries @c$N@w with $m.@n", true, ch, nullptr, CARRYING(ch), TO_ROOM);
    }
    ch->affected_by.set(AFF_PURSUIT);
    char_from_room(ch);
    char_to_room(ch, dest->vn);
    if ((ch->getRoom()->zone != world[was_in].zone) && !IS_NPC(ch) && !IS_ANDROID(ch)) {
        send_to_sense(0, "You sense someone", ch);
        sprintf(buf3, "@D[@GBlip@D]@Y %s\r\n@RSomeone has entered your scouter detection range@n.",
                add_commas(GET_HIT(ch)).c_str());
        send_to_scouter(buf3, ch, 0, 0);
    }
    /* move them first, then move them back if they aren't allowed to go. */
    /* see if an entry trigger disallows the move */
    if (!entry_mtrigger(ch) || !enter_wtrigger(ch->getRoom(), ch, dir)) {
        char_from_room(ch);
        char_to_room(ch, was_in);
        ch->affected_by.reset(AFF_PURSUIT);
        return 0;
    }

    snprintf(buf2, sizeof(buf2), "%s%s",
             ((dir == UP) || (dir == DOWN) ? "" : "the "),
             (dir == UP ? "below" :
              (dir == DOWN) ? "above" : dirs[rev_dir[dir]]));
    act("$n arrives from $T.", true, ch, nullptr, buf2, TO_ROOM | TO_SNEAKRESIST);
    if (FIGHTING(ch)) {
        if (SECT(world[was_in].dir_option[dir]->to_room) != SECT_FLYING &&
            SECT(world[was_in].dir_option[dir]->to_room) != SECT_WATER_NOSWIM &&
            ROOM_EFFECT(world[was_in].dir_option[dir]->to_room) == 0) {
            roll_pursue(FIGHTING(ch), ch);
        }
        ch->affected_by.reset(AFF_PURSUIT);
    }
    if (DRAGGING(ch)) {
        act("@wYou drag @C$N@w with you.@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@w drags @c$N@w with $m.@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        char_from_room(DRAGGING(ch));
        char_to_room(DRAGGING(ch), IN_ROOM(ch));
        if (SITS(DRAGGING(ch))) {
            obj_from_room(SITS(DRAGGING(ch)));
            obj_to_room(SITS(DRAGGING(ch)), IN_ROOM(ch));
        }
        if (!AFF_FLAGGED(DRAGGING(ch), AFF_KNOCKED) && !AFF_FLAGGED(DRAGGING(ch), AFF_SLEEP) && rand_number(1, 3)) {
            send_to_char(DRAGGING(ch), "You feel your sleeping body being moved.\r\n");
            if (IS_NPC(DRAGGING(ch)) && !FIGHTING(DRAGGING(ch))) {
                set_fighting(DRAGGING(ch), ch);
            }
        }
    }
    if (CARRYING(ch)) {
        act("@wYou carry @C$N@w with you.@n", true, ch, nullptr, CARRYING(ch), TO_CHAR);
        act("@C$n@w carries @c$N@w with $m.@n", true, ch, nullptr, CARRYING(ch), TO_ROOM);
        char_from_room(CARRYING(ch));
        char_to_room(CARRYING(ch), IN_ROOM(ch));
        if (!AFF_FLAGGED(CARRYING(ch), AFF_KNOCKED) && !AFF_FLAGGED(CARRYING(ch), AFF_SLEEP) && rand_number(1, 3)) {
            send_to_char(CARRYING(ch), "You feel your sleeping body being moved.\r\n");
        }
    }

    if (ch->desc != nullptr) {
        look_at_room(IN_ROOM(ch), ch, 0);
        if (AFF_FLAGGED(ch, AFF_SNEAK) && !IS_NPC(ch) && GET_SKILL(ch, SKILL_MOVE_SILENTLY) &&
            GET_SKILL(ch, SKILL_MOVE_SILENTLY) < rand_number(1, 101)) {
            send_to_char(ch, "@wYou make a noise as you arrive and are no longer sneaking!@n\r\n");
            act("@c$n@w makes a noise revealing $s sneaking!@n", true, ch, nullptr, nullptr, TO_ROOM | TO_SNEAKRESIST);
            reveal_hiding(ch, 0);
            ch->affected_by.reset(AFF_SNEAK);
        }
    }

    if (ROOM_EFFECT(IN_ROOM(ch)) == 6 || ROOM_EFFECT(was_in) == 6) {
        if (!IS_DEMON(ch) && !AFF_FLAGGED(ch, AFF_FLYING) && group_bonus(ch, 2) != 14) {
            act("@rYour legs are burned by the lava!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@R$n@r's legs are burned by the lava!@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (IS_NPC(ch) && IS_HUMANOID(ch) && rand_number(1, 2) == 2) {
                do_fly(ch, nullptr, 0, 0);
            }
            ch->decCurHealth(ch->getEffMaxPL() / 20);
            if (GET_HIT(ch) <= 0) {
                act("@rYou have burned to death!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n@r has burned to death!@n", true, ch, nullptr, nullptr, TO_ROOM);
                die(ch, nullptr);
            }
        }
        if (DRAGGING(ch) && !IS_DEMON(DRAGGING(ch))) {
            act("@R$N@r gets burned!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
            act("@R$N@r gets burned!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
            DRAGGING(ch)->decCurHealth(DRAGGING(ch)->getEffMaxPL() / 20);
            if (GET_HIT(DRAGGING(ch)) < 0) {
                act("@rYou have burned to death!@n", true, DRAGGING(ch), nullptr, nullptr, TO_CHAR);
                act("@R$n@r has burned to death!@n", true, DRAGGING(ch), nullptr, nullptr, TO_ROOM);
                die(DRAGGING(ch), nullptr);
            }
        }
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_TIMED_DT) && !ADM_FLAGGED(ch, ADM_WALKANYWHERE))
        timed_dt(nullptr);

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_DEATH) && !ADM_FLAGGED(ch, ADM_WALKANYWHERE)) {
        log_death_trap(ch);
        death_cry(ch);
        extract_char(ch);
        return (0);
    }

    entry_memory_mtrigger(ch);
    if (!greet_mtrigger(ch, dir)) {
        char_from_room(ch);
        char_to_room(ch, was_in);
        look_at_room(IN_ROOM(ch), ch, 0);
    } else greet_memory_mtrigger(ch);
    if (willfall == true) {
        handle_fall(ch);
        if (DRAGGING(ch)) {
            handle_fall(DRAGGING(ch));
        }
    }
    return (1);
}

int perform_move(struct char_data *ch, int dir, int need_specials_check) {
    room_rnum was_in;
    struct follow_type *k, *next;

    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return (0);
    }

    if (ABSORBING(ch) || ABSORBBY(ch)) {
        send_to_char(ch, "You are struggling with someone!\r\n");
        return (0);
    }

    if (!AFF_FLAGGED(ch, AFF_SNEAK) ||
        (AFF_FLAGGED(ch, AFF_SNEAK) && GET_SKILL(ch, SKILL_MOVE_SILENTLY) < axion_dice(0))) {
        reveal_hiding(ch, 0);
    }

    if (ch == nullptr || dir < 0 || dir >= NUM_OF_DIRS)
        return (0);
    else if ((!EXIT(ch, dir) && !buildwalk(ch, dir)) || EXIT(ch, dir)->to_room == NOWHERE ||
             (EXIT_FLAGGED(EXIT(ch, dir), EX_SECRET) && (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED))))
        send_to_char(ch, "Alas, you cannot go that way...\r\n");
    else if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED)) {
        if (EXIT(ch, dir)->keyword)
            send_to_char(ch, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));
        else
            send_to_char(ch, "It seems to be closed.\r\n");
    } else if (GET_ROOM_VNUM(EXIT(ch, dir)->to_room) == 0 || GET_ROOM_VNUM(EXIT(ch, dir)->to_room) == 1) {
        send_to_char(ch, "Report this direction, it is illegal.\r\n");
    } else {

        struct obj_data *wall;
        for (wall = ch->getRoom()->contents; wall; wall = wall->next_content) {
            if (GET_OBJ_VNUM(wall) == 79) {
                if (GET_OBJ_COST(wall) == dir) {
                    send_to_char(ch, "That direction has a glacial wall blocking it.\r\n");
                    return (0);
                }
            }
        }

        if (!ch->followers)
            return (do_simple_move(ch, dir, need_specials_check));

        was_in = IN_ROOM(ch);
        if (!do_simple_move(ch, dir, need_specials_check))
            return (0);

        for (k = ch->followers; k; k = next) {
            next = k->next;
            if ((IN_ROOM(k->follower) == was_in) &&
                (GET_POS(k->follower) >= POS_STANDING) &&
                (!AFF_FLAGGED(ch, AFF_ZANZOKEN) ||
                 (AFF_FLAGGED(ch, AFF_GROUP) && AFF_FLAGGED(k->follower, AFF_GROUP)))) {
                act("You follow $N.\r\n", false, k->follower, nullptr, ch, TO_CHAR);
                perform_move(k->follower, dir, 1);
            } else if ((IN_ROOM(k->follower) == was_in) &&
                       (GET_POS(k->follower) >= POS_STANDING) &&
                       (AFF_FLAGGED(ch, AFF_ZANZOKEN) && AFF_FLAGGED(k->follower, AFF_ZANZOKEN)) &&
                       (!AFF_FLAGGED(ch, AFF_GROUP) || !AFF_FLAGGED(k->follower, AFF_GROUP))) {
                act("$N tries to zanzoken and escape, but your zanzoken matches $S!\r\n", false, k->follower, nullptr,
                    ch, TO_CHAR);
                act("$N tries to zanzoken and escape, but $n's zanzoken matches $S!\r\n", false, k->follower, nullptr,
                    ch, TO_NOTVICT);
                act("You zanzoken to try and escape, but $n's zanzoken matches yours!\r\n", false, k->follower, nullptr,
                    ch, TO_VICT);
                for(auto c : {ch, k->follower}) c->affected_by.reset(AFF_ZANZOKEN);
                perform_move(k->follower, dir, 1);
            } else if ((IN_ROOM(k->follower) == was_in) &&
                       (GET_POS(k->follower) >= POS_STANDING) &&
                       (AFF_FLAGGED(ch, AFF_ZANZOKEN) && !AFF_FLAGGED(k->follower, AFF_ZANZOKEN))) {
                act("You try to follow $N, but $E disappears in a flash of movement!\r\n", false, k->follower, nullptr,
                    ch, TO_CHAR);
                act("$n tries to follow $N, but $E disappears in a flash of movement!\r\n", false, k->follower, nullptr,
                    ch, TO_NOTVICT);
                act("$n tries to follow you, but you manage to zanzoken away!\r\n", false, k->follower, nullptr, ch,
                    TO_VICT);
                ch->affected_by.reset(AFF_ZANZOKEN);
            }
        }
        return (1);
    }
    return (0);
}

ACMD(do_move) {
    if (IS_NPC(ch)) {
        perform_move(ch, subcmd - 1, 0);
        return;
    }
    if (PLR_FLAGGED(ch, PLR_SELFD)) {
        send_to_char(ch, "You are preparing to blow up!\r\n");
        return;
    }
    if (AFF_FLAGGED(ch, AFF_LIQUEFIED)) {
        send_to_char(ch, "You are liquefied right now!\r\n");
        return;
    }
    if (GET_CHARGE(ch) >= GET_MAX_MANA(ch) * .51) {
        send_to_char(ch,
                     "You have too much ki charged. You can't concentrate on keeping it charged while also traveling.\r\n");
        return;
    } else if ((GET_CHARGE(ch) >= GET_MAX_MANA(ch) * .5 && GET_CHARGE(ch) < GET_MAX_MANA(ch) * .51) &&
               GET_SKILL(ch, SKILL_CONCENTRATION) < 100) {
        send_to_char(ch,
                     "You have too much ki charged. You can't concentrate on keeping it charged while also traveling.\r\n");
        return;
    } else if ((GET_CHARGE(ch) >= GET_MAX_MANA(ch) * .4 && GET_CHARGE(ch) < GET_MAX_MANA(ch) * .5) &&
               GET_SKILL(ch, SKILL_CONCENTRATION) < 80) {
        send_to_char(ch,
                     "You have too much ki charged. You can't concentrate on keeping it charged while also traveling.\r\n");
        return;
    } else if ((GET_CHARGE(ch) >= GET_MAX_MANA(ch) * .3 && GET_CHARGE(ch) < GET_MAX_MANA(ch) * .4) &&
               GET_SKILL(ch, SKILL_CONCENTRATION) < 70) {
        send_to_char(ch,
                     "You have too much ki charged. You can't concentrate on keeping it charged while also traveling.\r\n");
        return;
    } else if ((GET_CHARGE(ch) >= GET_MAX_MANA(ch) * .2 && GET_CHARGE(ch) < GET_MAX_MANA(ch) * .3) &&
               GET_SKILL(ch, SKILL_CONCENTRATION) < 60) {
        send_to_char(ch,
                     "You have too much ki charged. You can't concentrate on keeping it charged while also traveling.\r\n");
        return;
    }

    if (GET_COND(ch, DRUNK) > 4 && (rand_number(1, 9) + GET_COND(ch, DRUNK)) >= rand_number(14, 20)) {
        send_to_char(ch, "You wobble around and then fall on your ass.\r\n");
        act("@C$n@W wobbles around before falling on $s ass@n.", true, ch, nullptr, nullptr, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        return;
    }

    if (FIGHTING(ch) && !IS_NPC(ch)) {
        char blah[MAX_INPUT_LENGTH];
        sprintf(blah, "%s", dirs[subcmd - 1]);
        do_flee(ch, blah, 0, 0);
        return;
    }

    /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
    if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        struct obj_data *vehicle = nullptr, *controls = nullptr;
        int noship = false;
        if (!(controls = find_control(ch)) && GET_ADMLEVEL(ch) < 1) {
            noship = true;
        } else if (!(vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(controls, 0)))) {
            noship = true;
        }
        if (noship == true) {
            send_to_char(ch, "Your ship controls are not here or your ship was not found, report to Iovan!\r\n");
            return;
        } else if (controls != nullptr && vehicle != nullptr) {
            if (GET_FUEL(controls) <= 0) {
                send_to_char(ch, "The ship is out of fuel!\r\n");
                return;
            }
            drive_in_direction(ch, vehicle, subcmd - 1);
            if (GET_OBJ_VAL(controls, 1) == 1) {
                WAIT_STATE(ch, PULSE_2SEC);
            } else if (GET_OBJ_VAL(controls, 1) == 2) {
                WAIT_STATE(ch, PULSE_1SEC);
            }
            controls = nullptr;
            vehicle = nullptr;
            return;
        }
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }
    if (!IS_NPC(ch)) {
        int fail = false;
        struct obj_data *obj, *next_obj;
        for (obj = ch->getRoom()->contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (KICHARGE(obj) > 0 && USER(obj) == ch) {
                fail = true;
            }
        }
        if (fail == true) {
            send_to_char(ch, "You are too busy controlling your attack!\r\n");
            return;
        }
    }

    if (!IS_NPC(ch) && GET_LIMBCOND(ch, 0) <= 0 && GET_LIMBCOND(ch, 1) <= 0 && GET_LIMBCOND(ch, 2) <= 0 &&
        GET_LIMBCOND(ch, 3) <= 0 && !AFF_FLAGGED(ch, AFF_FLYING)) {
        send_to_char(ch, "Unless you fly, you can't get far with no limbs.\r\n");
        return;
    }
    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return;
    }
    if (ABSORBING(ch)) {
        send_to_char(ch, "You are busy absorbing from %s!\r\n", GET_NAME(ABSORBING(ch)));
        return;
    }
    if (ABSORBBY(ch)) {
        if (axion_dice(0) < GET_SKILL(ABSORBBY(ch), SKILL_ABSORB)) {
            send_to_char(ch, "You are being held by %s, they are absorbing you!\r\n", GET_NAME(ABSORBBY(ch)));
            send_to_char(ABSORBBY(ch), "%s struggles in your grasp!\r\n", GET_NAME(ch));
            WAIT_STATE(ch, PULSE_2SEC);
            return;
        } else {
            act("@c$N@W manages to break loose of @C$n's@W hold!@n", true, ABSORBBY(ch), nullptr, ch, TO_NOTVICT);
            act("@WYou manage to break loose of @C$n's@W hold!@n", true, ABSORBBY(ch), nullptr, ch, TO_VICT);
            act("@c$N@W manages to break loose of your hold!@n", true, ABSORBBY(ch), nullptr, ch, TO_CHAR);
            ABSORBING(ABSORBBY(ch)) = nullptr;
            ABSORBBY(ch) = nullptr;
        }
    }
    if (!block_calc(ch)) {
        return;
    }
    if (GET_EAVESDROP(ch) > 0) {
        send_to_char(ch, "You stop eavesdropping.\r\n");
        GET_EAVESDROP(ch) = real_room(0);
    }
    if (!IS_NPC(ch)) {
        if (PRF_FLAGGED(ch, PRF_ARENAWATCH)) {
            ch->pref.reset(PRF_ARENAWATCH);
            ARENA_IDNUM(ch) = -1;
        }
        if (GET_ROOM_VNUM(IN_ROOM(ch)) != NOWHERE && GET_ROOM_VNUM(IN_ROOM(ch)) != 0 &&
            GET_ROOM_VNUM(IN_ROOM(ch)) != 1) {
            GET_LOADROOM(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
        }

        auto ratio = ch->getBurdenRatio();
        if(ratio >= 1.0) {
            send_to_char(ch, "Your immense burden hinders your progress.\r\n");
            WAIT_STATE(ch, std::min<int>(PULSE_3SEC * ratio, PULSE_5SEC));
        }

        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE) && GET_ADMLEVEL(ch) < 1) {
            send_to_char(ch, "You struggle to cross the vast distance.\r\n");
            WAIT_STATE(ch, PULSE_6SEC);
        } else if ((GET_LIMBCOND(ch, 2) <= 0 && GET_LIMBCOND(ch, 3) <= 0) && GET_LIMBCOND(ch, 0) <= 0 &&
                   !AFF_FLAGGED(ch, AFF_FLYING)) {
            act("@wYou slowly pull yourself along with your arm...@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w slowly pulls $mself along with one arm...@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (GET_LIMBCOND(ch, 1) < 50) {
                send_to_char(ch, "@RYour left arm is damaged by the forced use!@n\r\n");
                GET_LIMBCOND(ch, 1) -= rand_number(1, 5);
                if (GET_LIMBCOND(ch, 0) <= 0) {
                    act("@RYour left arm falls apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@r$n's@R left arm falls apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            WAIT_STATE(ch, PULSE_5SEC);
        } else if ((GET_LIMBCOND(ch, 2) <= 0 && GET_LIMBCOND(ch, 3) <= 0) && GET_LIMBCOND(ch, 1) <= 0 &&
                   !AFF_FLAGGED(ch, AFF_FLYING)) {
            act("@wYou slowly pull yourself along with your arm...@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w slowly pulls $mself along with one arm...@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (GET_LIMBCOND(ch, 0) < 50) {
                send_to_char(ch, "@RYour right arm is damaged by the forced use!@n\r\n");
                GET_LIMBCOND(ch, 0) -= rand_number(1, 5);
                if (GET_LIMBCOND(ch, 0) <= 0) {
                    act("@RYour right arm falls apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@r$n's@R right arm falls apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            WAIT_STATE(ch, PULSE_5SEC);
        } else if ((GET_LIMBCOND(ch, 2) <= 0 && GET_LIMBCOND(ch, 3) <= 0) && !AFF_FLAGGED(ch, AFF_FLYING)) {
            act("@wYou slowly pull yourself along with your arms...@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w slowly pulls $mself along with one arms...@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (GET_LIMBCOND(ch, 1) < 50) {
                send_to_char(ch, "@RYour left arm is damaged by the forced use!@n\r\n");
                GET_LIMBCOND(ch, 1) -= rand_number(1, 5);
                if (GET_LIMBCOND(ch, 1) <= 0) {
                    act("@RYour left arm falls apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@r$n's@R left arm falls apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            if (GET_LIMBCOND(ch, 0) < 50) {
                send_to_char(ch, "@RYour right arm is damaged by the forced use!@n\r\n");
                GET_LIMBCOND(ch, 0) -= rand_number(1, 5);
                if (GET_LIMBCOND(ch, 0) <= 0) {
                    act("@RYour right arm falls apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@r$n's@R right arm falls apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            WAIT_STATE(ch, PULSE_3SEC);
        } else if (GET_LIMBCOND(ch, 2) <= 0 && !AFF_FLAGGED(ch, AFF_FLYING)) {
            act("@wYou hop on one leg...@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w hops on one leg...@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (GET_LIMBCOND(ch, 3) < 50) {
                send_to_char(ch, "@RYour left leg is damaged by the forced use!@n\r\n");
                GET_LIMBCOND(ch, 3) -= rand_number(1, 5);
                if (GET_LIMBCOND(ch, 3) <= 0) {
                    act("@RYour left leg falls apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@r$n's@R left leg falls apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            WAIT_STATE(ch, PULSE_2SEC);
        } else if (GET_LIMBCOND(ch, 3) <= 0 && !AFF_FLAGGED(ch, AFF_FLYING)) {
            act("@wYou hop on one leg...@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w hops on one leg...@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (GET_LIMBCOND(ch, 2) < 50) {
                send_to_char(ch, "@RYour right leg is damaged by the forced use!@n\r\n");
                GET_LIMBCOND(ch, 2) -= rand_number(1, 5);
                if (GET_LIMBCOND(ch, 2) <= 0) {
                    act("@RYour right leg falls apart!@n", true, ch, nullptr, nullptr, TO_CHAR);
                    act("@r$n's@R right leg falls apart!@n", true, ch, nullptr, nullptr, TO_ROOM);
                }
            }
            WAIT_STATE(ch, PULSE_2SEC);
        } else if (GET_POS(ch) == POS_RESTING) {
            act("@wYou crawl on your hands and knees.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w crawls on $s hands and knees.@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (SITS(ch)) {
                struct obj_data *chair = SITS(ch);
                SITTING(chair) = nullptr;
                SITS(ch) = nullptr;
            }
            WAIT_STATE(ch, PULSE_3SEC);
        } else if (GET_POS(ch) == POS_SITTING) {
            act("@wYou shuffle on your hands and knees.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@w shuffles on $s hands and knees.@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (SITS(ch)) {
                struct obj_data *chair = SITS(ch);
                SITTING(chair) = nullptr;
                SITS(ch) = nullptr;
            }
            WAIT_STATE(ch, PULSE_2SEC);
        } else if (GET_POS(ch) < POS_RESTING) {
            send_to_char(ch, "You are in no condition to move! Try standing...\r\n");
            return;
        }
    }
    perform_move(ch, subcmd - 1, 0);
    if (GET_RDISPLAY(ch)) {
        if (GET_RDISPLAY(ch) != "Empty") {
            GET_RDISPLAY(ch) = "Empty";
        }
    }
}

static int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname) {
    int door;

    if (*dir) {            /* a direction was specified */
        if ((door = search_block(dir, dirs, false)) < 0 &&
            (door = search_block(dir, abbr_dirs, false)) < 0) {    /* Partial Match */
            send_to_char(ch, "That's not a direction.\r\n");
            return (-1);
        }
        if (EXIT(ch, door)) {    /* Braces added according to indent. -gg */
            if (EXIT(ch, door)->keyword) {
                if (is_name(type, EXIT(ch, door)->keyword))
                    return (door);
                else {
                    send_to_char(ch, "I see no %s there.\r\n", type);
                    return (-1);
                }
            } else
                return (door);
        } else {
            send_to_char(ch, "I really don't see how you can %s anything there.\r\n", cmdname);
            return (-1);
        }
    } else {            /* try to locate the keyword */
        if (!*type) {
            send_to_char(ch, "What is it you want to %s?\r\n", cmdname);
            return (-1);
        }
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXIT(ch, door))
                if (EXIT(ch, door)->keyword)
                    if (is_name(type, EXIT(ch, door)->keyword))
                        return (door);

        send_to_char(ch, "There doesn't seem to be %s %s that could be manipulated in that way here.\r\n", AN(type),
                     type);
        return (-1);
    }
}

static int has_key(struct char_data *ch, obj_vnum key) {
    return ch->findObjectVnum(key) != nullptr;
}

#define NEED_OPEN    (1 << 0)
#define NEED_CLOSED    (1 << 1)
#define NEED_UNLOCKED    (1 << 2)
#define NEED_LOCKED    (1 << 3)

const char *cmd_door[NUM_DOOR_CMD] =
        {
                "open",
                "close",
                "unlock",
                "lock",
                "pick"
        };

static const int flags_door[] =
        {
                NEED_CLOSED | NEED_UNLOCKED,
                NEED_OPEN,
                NEED_CLOSED | NEED_LOCKED,
                NEED_CLOSED | NEED_UNLOCKED,
                NEED_CLOSED | NEED_LOCKED
        };


#define EXITN(room, door)        (world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)    ((obj) ?\
        (REMOVE_BIT(GET_OBJ_VAL(obj, VAL_CONTAINER_FLAGS), CONT_CLOSED)) :\
        (REMOVE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define CLOSE_DOOR(room, obj, door)    ((obj) ?\
        (SET_BIT(GET_OBJ_VAL(obj, VAL_CONTAINER_FLAGS), CONT_CLOSED)) :\
        (SET_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)    ((obj) ?\
        (SET_BIT(GET_OBJ_VAL(obj, VAL_CONTAINER_FLAGS), CONT_LOCKED)) :\
        (SET_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))
#define UNLOCK_DOOR(room, obj, door)    ((obj) ?\
        (REMOVE_BIT(GET_OBJ_VAL(obj, VAL_CONTAINER_FLAGS), CONT_LOCKED)) :\
        (REMOVE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))
#define TOGGLE_LOCK(room, obj, door)    ((obj) ?\
        (TOGGLE_BIT(GET_OBJ_VAL(obj, VAL_CONTAINER_FLAGS), CONT_LOCKED)) :\
        (TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

static void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd) {
    char buf[MAX_STRING_LENGTH];
    size_t len;
    int num = 0;
    room_rnum other_room = NOWHERE;
    struct room_direction_data *back = nullptr;
    struct obj_data *hatch = nullptr, *obj2 = nullptr, *next_obj, *vehicle = nullptr;

    if ((obj) && GET_OBJ_TYPE(obj) == ITEM_HATCH) {
        vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(obj, VAL_HATCH_DEST));
    } else if ((obj) && GET_OBJ_TYPE(obj) == ITEM_VEHICLE) {
        if (real_room(GET_OBJ_VAL(obj, VAL_PORTAL_DEST)) != NOWHERE) {
            num = IN_ROOM(ch);
            char_from_room(ch);
            char_to_room(ch, real_room(GET_OBJ_VAL(obj, VAL_PORTAL_DEST)));
        }
        for (obj2 = world[IN_ROOM(ch)].contents; obj2; obj2 = next_obj) {
            next_obj = obj2->next_content;
            if (GET_OBJ_TYPE(obj2) == ITEM_HATCH) {
                hatch = obj2;
            }
        }
        obj2 = nullptr;
    }

    if (!door_mtrigger(ch, scmd, door))
        return;

    if (!door_wtrigger(ch, scmd, door))
        return;

    len = snprintf(buf, sizeof(buf), "$n %ss ", cmd_door[scmd]);
    if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE)) {
        if ((back = world[other_room].dir_option[rev_dir[door]]) != nullptr)
            if (back->to_room != IN_ROOM(ch))
                back = nullptr;
    }
    switch (scmd) {
        case SCMD_OPEN:
            if (obj) {
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_HATCH && (vehicle)) {
                    OPEN_DOOR(IN_ROOM(ch), vehicle, door);
                    if (GET_OBJ_VNUM(obj) > 19199) {
                        send_to_room(IN_ROOM(ch),
                                     "@wThe ship hatch opens slowly and settles onto the ground outside.\r\n");
                        send_to_room(IN_ROOM(vehicle),
                                     "@wThe ship hatch opens slowly and settles onto the ground.\r\n");
                        if (ROOM_FLAGGED(IN_ROOM(vehicle), ROOM_SPACE)) {
                            send_to_room(IN_ROOM(ch),
                                         "@wA great vortex forms as air begins to get sucked out into the void!\r\n");
                        }
                    } else {
                        act("@wYou open @c$p@w.", true, ch, obj, 0, TO_CHAR);
                        act("@C$n@w opens @c$p@w.", true, ch, obj, 0, TO_ROOM);
                        send_to_room(IN_ROOM(vehicle), "@wThe door to %s@w is opened from the other side.\r\n",
                                     vehicle->short_description);
                    }
                    vehicle = nullptr;
                }
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_VEHICLE && (hatch)) {
                    OPEN_DOOR(IN_ROOM(ch), hatch, door);
                    char_from_room(ch);
                    char_to_room(ch, num);
                    if (GET_OBJ_VNUM(obj) > 19199) {
                        send_to_room(num, "@wThe ship hatch opens slowly and settles onto the ground.\r\n");
                        send_to_room(IN_ROOM(hatch), "@wThe ship hatch opens slowly.\r\n");
                        if (ROOM_FLAGGED(IN_ROOM(obj), ROOM_SPACE)) {
                            send_to_room(num, "@wThe air starts getting sucked out into space as the hatch opens!\r\n");
                        }
                    } else {
                        act("@wYou open @c$p@w.", true, ch, obj, 0, TO_CHAR);
                        act("@C$n@w opens @c$p@w.", true, ch, obj, 0, TO_ROOM);
                        send_to_room(IN_ROOM(hatch), "@wThe door is opened from the other side.\r\n");
                    }
                    hatch = nullptr;
                }
            }
            OPEN_DOOR(IN_ROOM(ch), obj, door);
            if (back) {
                OPEN_DOOR(other_room, obj, rev_dir[door]);
            }
            if (!obj) {
                send_to_char(ch, "You open the %s that leads %s.\r\n",
                             EXIT(ch, door)->keyword ? EXIT(ch, door)->keyword : "door", dirs[door]);
            } else if (GET_OBJ_TYPE(obj) != ITEM_VEHICLE && GET_OBJ_TYPE(obj) != ITEM_HATCH) {
                send_to_char(ch, "You open %s.\r\n", obj->short_description);
            }
            break;

        case SCMD_CLOSE:
            if (obj) {
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_HATCH && (vehicle)) {
                    CLOSE_DOOR(IN_ROOM(ch), vehicle, door);
                    if (GET_OBJ_VNUM(obj) > 19199) {
                        send_to_room(IN_ROOM(ch),
                                     "@wThe ship hatch slowly closes, sealing the ship from the outside.\r\n");
                        send_to_room(IN_ROOM(vehicle), "@wThe ship hatch slowly closes, sealing the ship.\r\n");
                        if (ROOM_FLAGGED(IN_ROOM(vehicle), ROOM_SPACE)) {
                            send_to_room(IN_ROOM(ch),
                                         "@wThe air stops getting sucked out into space as the hatch seals!\r\n");
                        }
                    } else {
                        act("@wYou close @c$p@w.", true, ch, obj, 0, TO_CHAR);
                        act("@C$n@w closes @c$p@w.", true, ch, obj, 0, TO_ROOM);
                        send_to_room(IN_ROOM(vehicle), "@wThe door to %s@w is closed from the other side.\r\n",
                                     vehicle->short_description);
                    }
                    vehicle = NULL;
                }
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_VEHICLE && (hatch)) {
                    CLOSE_DOOR(IN_ROOM(ch), hatch, door);
                    char_from_room(ch);
                    char_to_room(ch, num);
                    if (GET_OBJ_VNUM(obj) > 19199) {
                        send_to_room(num, "@wThe ship hatch slowly closes, sealing the ship.\r\n");
                        send_to_room(IN_ROOM(hatch),
                                     "@wThe ship hatch slowly closes, sealing the ship from the outside.\r\n");
                        if (ROOM_FLAGGED(IN_ROOM(obj), ROOM_SPACE)) {
                            send_to_room(num, "@wAir stops getting sucked out into space as the hatch seals!\r\n");
                        }
                    } else {
                        act("@wYou close @c$p@w.", true, ch, obj, 0, TO_CHAR);
                        act("@C$n@w closes @c$p@w.", true, ch, obj, 0, TO_ROOM);
                        send_to_room(IN_ROOM(hatch), "@wThe door to %s@w is closed from the other side.\r\n",
                                     hatch->short_description);
                    }
                    hatch = NULL;
                }
            }
            CLOSE_DOOR(IN_ROOM(ch), obj, door);
            if (back) {
                CLOSE_DOOR(other_room, obj, rev_dir[door]);
            }
            if (!obj) {
                send_to_char(ch, "You close the %s that leads %s.\r\n",
                             EXIT(ch, door)->keyword ? EXIT(ch, door)->keyword : "door", dirs[door]);
            } else if (GET_OBJ_TYPE(obj) != ITEM_VEHICLE && GET_OBJ_TYPE(obj) != ITEM_HATCH) {
                send_to_char(ch, "You close %s.\r\n", obj->short_description);
            }
            break;

        case SCMD_LOCK:
            if (obj) {
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_HATCH && (vehicle)) {
                    LOCK_DOOR(IN_ROOM(ch), vehicle, door);
                    vehicle = NULL;
                }
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_VEHICLE && (hatch)) {
                    LOCK_DOOR(IN_ROOM(ch), hatch, door);
                    char_from_room(ch);
                    char_to_room(ch, num);
                    hatch = NULL;
                }
            }
            LOCK_DOOR(IN_ROOM(ch), obj, door);
            if (back) {
                LOCK_DOOR(other_room, obj, rev_dir[door]);
            }
            if (!obj) {
                send_to_char(ch, "You lock the %s that leads %s.\r\n",
                             EXIT(ch, door)->keyword ? EXIT(ch, door)->keyword : "door", dirs[door]);
            } else {
                send_to_char(ch, "You lock %s.\r\n", obj->short_description);
            }
            break;

        case SCMD_UNLOCK:
            if (obj) {
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_HATCH && (vehicle)) {
                    UNLOCK_DOOR(IN_ROOM(ch), vehicle, door);
                    vehicle = NULL;
                }
                if ((obj) && GET_OBJ_TYPE(obj) == ITEM_VEHICLE && (hatch)) {
                    UNLOCK_DOOR(IN_ROOM(ch), hatch, door);
                    char_from_room(ch);
                    char_to_room(ch, num);
                    hatch = NULL;
                }
            }
            UNLOCK_DOOR(IN_ROOM(ch), obj, door);
            if (back) {
                UNLOCK_DOOR(other_room, obj, rev_dir[door]);
            }
            if (!obj) {
                send_to_char(ch, "You unlock the %s that leads %s.\r\n",
                             EXIT(ch, door)->keyword ? EXIT(ch, door)->keyword : "door", dirs[door]);
            } else {
                send_to_char(ch, "You unlock %s.\r\n", obj->short_description);
            }
            break;

        case SCMD_PICK:
            TOGGLE_LOCK(IN_ROOM(ch), obj, door);
            if (back)
                TOGGLE_LOCK(other_room, obj, rev_dir[door]);
            send_to_char(ch, "The lock quickly yields to your skills.\r\n");
            len = strlcpy(buf, "$n skillfully picks the lock on ", sizeof(buf));
            break;
    }

    /* Notify the room. */
    char dbuf[100];
    if (!obj) {
        sprintf(dbuf, "%s", dirs[door]);
    }
    if (len < sizeof(buf))
        snprintf(buf + len, sizeof(buf) - len, "%s%s%s%s.",
                 obj ? "" : "the ", obj ? "$p" : EXIT(ch, door)->keyword ? "$F" : "door", obj ? "" : " that leads ",
                 obj ? "" : dbuf);
    if (!obj || IN_ROOM(obj) != NOWHERE)
        act(buf, false, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

    /* Notify the other room */
    if (back && (scmd == SCMD_OPEN || scmd == SCMD_CLOSE)) {
        send_to_room(EXIT(ch, door)->to_room, "The %s that leads %s is %s%s from the other side.\r\n",
                     back->keyword ? fname(back->keyword) : "door", dbuf, cmd_door[scmd],
                     scmd == SCMD_CLOSE ? "d" : "ed");
    } else if (back && (scmd == SCMD_LOCK || scmd == SCMD_UNLOCK)) {
        send_to_room(EXIT(ch, door)->to_room, "The %s that leads %s is %sed from the other side.\r\n",
                     back->keyword ? fname(back->keyword) : "door", dbuf, cmd_door[scmd]);
    }
    *dbuf = '\0';
}

static int ok_pick(struct char_data *ch, obj_vnum keynum, int pickproof, int dclock, int scmd, struct obj_data *hatch) {
    int skill_lvl, found = false;
    struct obj_data *obj, *next_obj;
    obj = ch->findObjectVnum(18);

    if (scmd != SCMD_PICK)
        return (1);

    /* PICKING_LOCKS is not an untrained skill */
    if (!GET_SKILL(ch, SKILL_OPEN_LOCK)) {
        send_to_char(ch, "You have no idea how!\r\n");
        return (0);
    }
    if (found == false) {
        send_to_char(ch, "You need a lock picking kit.\r\n");
        return (0);
    }
    if (hatch != nullptr && (GET_OBJ_TYPE(hatch) == ITEM_HATCH || GET_OBJ_TYPE(hatch) == ITEM_VEHICLE)) {
        send_to_char(ch, "No picking ship hatches.\r\n");
        hatch = nullptr;
        return (0);
    }
    skill_lvl = roll_skill(ch, SKILL_OPEN_LOCK);
    if (dclock == 0) {
        dclock = rand_number(1, 101);
    }

    if (keynum == NOTHING) {
        send_to_char(ch, "Odd - you can't seem to find a keyhole.\r\n");
    } else if (pickproof) {
        send_to_char(ch, "It resists your attempts to pick it.\r\n");
        act("@c$n@w puts a set of lockpick tools away.@n", true, ch, nullptr, nullptr, TO_ROOM);
        /* The -2 is here because that is a penality for not having a set of
   * thieves' tools. If the player has them, that modifier will be accounted
   * for in roll_skill, and negate (or surpass) this.
   */
    } else if ((ch->getCurST()) < GET_MAX_MOVE(ch) / 30) {
        send_to_char(ch, "You don't have the stamina to try, it takes percision to pick locks."
                         "Not shaking tired hands.\r\n");
    } else if (dclock > (skill_lvl - 2)) {
        send_to_char(ch, "You failed to pick the lock...\r\n");
        act("@c$n@w puts a set of lockpick tools away.@n", true, ch, nullptr, nullptr, TO_ROOM);
        ch->decCurST(ch->getCurST() / 30);
    } else {
        ch->decCurST(ch->getCurST() / 30);
        return (1);
    }

    return (0);
}


#define DOOR_IS_OPENABLE(ch, obj, door)    ((obj) ? \
            ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
            OBJVAL_FLAGGED(obj, CONT_CLOSEABLE))   || \
                        ((GET_OBJ_TYPE(obj) == ITEM_VEHICLE)   && \
                        OBJVAL_FLAGGED(obj, CONT_CLOSEABLE))   || \
                        ((GET_OBJ_TYPE(obj) == ITEM_HATCH)     && \
                        OBJVAL_FLAGGED(obj, CONT_CLOSEABLE))   || \
                        ((GET_OBJ_TYPE(obj) == ITEM_WINDOW)    && \
                        OBJVAL_FLAGGED(obj, CONT_CLOSEABLE))   || \
                        ((GET_OBJ_TYPE(obj) == ITEM_PORTAL)    && \
            OBJVAL_FLAGGED(obj, CONT_CLOSEABLE)) :\
            (EXIT_FLAGGED(EXIT(ch, door), EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)    ((obj) ? \
            (!OBJVAL_FLAGGED(obj, CONT_CLOSED)) :\
            (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door)    ((obj) ? \
            (!OBJVAL_FLAGGED(obj, CONT_LOCKED)) :\
            (!EXIT_FLAGGED(EXIT(ch, door), EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
            (OBJVAL_FLAGGED(obj, CONT_PICKPROOF)) : \
            (EXIT_FLAGGED(EXIT(ch, door), EX_PICKPROOF)))
#define DOOR_IS_SECRET(ch, obj, door) ((obj) ? \
                        (OBJVAL_FLAGGED(obj, CONT_SECRET)) : \
                        (EXIT_FLAGGED(EXIT(ch, door), EX_SECRET)))

#define DOOR_IS_CLOSED(ch, obj, door)    (!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)    (!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)        ((obj) ? (GET_OBJ_VAL(obj, VAL_KEY_KEYCODE)) : \
                    (EXIT(ch, door)->key))
#define DOOR_DCLOCK(ch, obj, door)    ((obj) ? (GET_OBJ_VAL(obj, VAL_DOOR_DCLOCK)) : EXIT(ch, door)->dclock)

ACMD(do_gen_door) {
    int door = -1;
    obj_vnum keynum;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = nullptr;
    struct char_data *victim = nullptr;

    skip_spaces(&argument);
    if (!*argument) {
        send_to_char(ch, "%c%s what?\r\n", UPPER(*cmd_door[subcmd]), cmd_door[subcmd] + 1);
        return;
    }
    two_arguments(argument, type, dir);
    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = find_door(ch, type, dir, cmd_door[subcmd]);

    auto r = ch->getRoom();

    if ((obj) &&
        (GET_OBJ_TYPE(obj) != ITEM_CONTAINER && GET_OBJ_TYPE(obj) != ITEM_VEHICLE && GET_OBJ_TYPE(obj) != ITEM_HATCH)) {
        obj = nullptr;
        door = find_door(ch, type, dir, cmd_door[subcmd]);
    }

    if ((obj) || (door >= 0)) {
        keynum = DOOR_KEY(ch, obj, door);
        if (!DOOR_DCLOCK(ch, obj, door)) {
            if (obj) {
                GET_OBJ_VAL(obj, VAL_DOOR_DCLOCK) = 20;
            } else {
                r->dir_option[door]->dclock = 20;
            }
        }
        if (!(DOOR_IS_OPENABLE(ch, obj, door)))
            act("You can't $F that!", false, ch, nullptr, cmd_door[subcmd], TO_CHAR);
        else if (!DOOR_IS_OPEN(ch, obj, door) &&
                 IS_SET(flags_door[subcmd], NEED_OPEN))
            send_to_char(ch, "But it's already closed!\r\n");
        else if (!DOOR_IS_CLOSED(ch, obj, door) &&
                 IS_SET(flags_door[subcmd], NEED_CLOSED))
            send_to_char(ch, "But it's currently open!\r\n");
        else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
                 IS_SET(flags_door[subcmd], NEED_LOCKED))
            send_to_char(ch, "Oh.. it wasn't locked, after all..\r\n");
        else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
                 IS_SET(flags_door[subcmd], NEED_UNLOCKED))
            send_to_char(ch, "It seems to be locked.\r\n");
        else if (!has_key(ch, keynum) && !ADM_FLAGGED(ch, ADM_NOKEYS) &&
                 ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
            send_to_char(ch, "You don't seem to have the proper key.\r\n");
        else if (!obj &&
                 ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), DOOR_DCLOCK(ch, obj, door), subcmd, nullptr))
            do_doorcmd(ch, obj, door, subcmd);
        else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), DOOR_DCLOCK(ch, obj, door), subcmd, obj) && obj)
            do_doorcmd(ch, obj, door, subcmd);
    }
}

static int do_simple_enter(struct char_data *ch, struct obj_data *obj, int need_specials_check) {
    room_rnum dest_room = real_room(GET_OBJ_VAL(obj, VAL_PORTAL_DEST));
    room_rnum was_in = IN_ROOM(ch);
    int need_movement = 0;

    /* charmed? */
    if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master &&
        IN_ROOM(ch) == IN_ROOM(ch->master)) {
        send_to_char(ch, "The thought of leaving your master makes you weep.\r\n");
        act("$n bursts into tears.", false, ch, nullptr, nullptr, TO_ROOM);
        return (0);
    }

    /* move points needed is avg. move loss for src and destination sect type */
    need_movement = calcNeedMovementGravity(ch);

    if (GET_LEVEL(ch) <= 1) {
        need_movement = 0;
    }
    if ((ch->getCurST()) < need_movement && !AFF_FLAGGED(ch, AFF_FLYING) && !IS_NPC(ch)) {
        if (need_specials_check && ch->master)
            send_to_char(ch, "You are too exhausted to follow.\r\n");
        else
            send_to_char(ch, "You are too exhausted.\r\n");

        return (0);
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ATRIUM)) {
        if (!House_can_enter(ch, GET_ROOM_VNUM(dest_room))) {
            send_to_char(ch, "That's private property -- no trespassing!\r\n");
            return (0);
        }
    }
    if (ROOM_FLAGGED(dest_room, ROOM_TUNNEL) &&
        num_pc_in_room(&(world[dest_room])) >= CONFIG_TUNNEL_SIZE) {
        if (CONFIG_TUNNEL_SIZE > 1)
            send_to_char(ch, "There isn't enough room for you to go there!\r\n");
        else
            send_to_char(ch, "There isn't enough room there for more than one person!\r\n");
        return (0);
    }
    /* Mortals and low level gods cannot enter greater god rooms. */
    if (ROOM_FLAGGED(dest_room, ROOM_GODROOM) &&
        GET_ADMLEVEL(ch) < ADMLVL_GRGOD) {
        send_to_char(ch, "You aren't godly enough to use that room!\r\n");
        return (0);
    }
    /* Now we know we're allowed to go into the room. */
    if (!(IS_NPC(ch) || ADM_FLAGGED(ch, ADM_WALKANYWHERE)) && !AFF_FLAGGED(ch, AFF_FLYING))
        ch->decCurST(need_movement);

    act("$n enters $p.", true, ch, obj, nullptr, TO_ROOM | TO_SNEAKRESIST);

    if (DRAGGING(ch)) {
        act("@C$n@w drags @c$N@w with $m.@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
    }
    if (CARRYING(ch)) {
        act("@C$n@w carries @c$N@w with $m.@n", true, ch, nullptr, CARRYING(ch), TO_ROOM);
    }
    char_from_room(ch);
    char_to_room(ch, dest_room);

    /* move them first, then move them back if they aren't allowed to go. */
    /* see if an entry trigger disallows the move */
    if (!entry_mtrigger(ch)) {
        char_from_room(ch);
        char_to_room(ch, was_in);
        return 0;
    }

    if (GET_OBJ_TYPE(obj) == ITEM_PORTAL)
        act("$n arrives from $p.", false, ch, obj, nullptr, TO_ROOM | TO_SNEAKRESIST);
    else
        act("$n arrives from outside.", false, ch, nullptr, nullptr, TO_ROOM | TO_SNEAKRESIST);
    if (DRAGGING(ch)) {
        act("@wYou drag @C$N@w with you.@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@w drags @c$N@w with $m.@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        if (!AFF_FLAGGED(DRAGGING(ch), AFF_KNOCKED) && !AFF_FLAGGED(DRAGGING(ch), AFF_SLEEP) && rand_number(1, 3)) {
            send_to_char(DRAGGING(ch), "You feel your sleeping body being moved.\r\n");
            if (IS_NPC(DRAGGING(ch)) && !FIGHTING(DRAGGING(ch))) {
                set_fighting(DRAGGING(ch), ch);
            }
        }
        char_from_room(DRAGGING(ch));
        char_to_room(DRAGGING(ch), IN_ROOM(ch));
        if (SITS(DRAGGING(ch))) {
            obj_from_room(SITS(DRAGGING(ch)));
            obj_to_room(SITS(DRAGGING(ch)), IN_ROOM(ch));
        }
    }
    if (CARRYING(ch)) {
        act("@wYou carry @C$N@w with you.@n", true, ch, nullptr, CARRYING(ch), TO_CHAR);
        act("@C$n@w carries @c$N@w with $m.@n", true, ch, nullptr, CARRYING(ch), TO_ROOM);
        if (!AFF_FLAGGED(CARRYING(ch), AFF_KNOCKED) && !AFF_FLAGGED(CARRYING(ch), AFF_SLEEP) && rand_number(1, 3)) {
            send_to_char(CARRYING(ch), "You feel your sleeping body being moved.\r\n");
        }
        char_from_room(CARRYING(ch));
        char_to_room(CARRYING(ch), IN_ROOM(ch));
        if (SITS(CARRYING(ch))) {
            obj_from_room(SITS(CARRYING(ch)));
            obj_to_room(SITS(CARRYING(ch)), IN_ROOM(ch));
        }
    }

    if (ch->desc != nullptr)
        look_at_room(IN_ROOM(ch), ch, 0);

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_DEATH) && !ADM_FLAGGED(ch, ADM_WALKANYWHERE)) {
        log_death_trap(ch);
        death_cry(ch);
        extract_char(ch);
        return 0;
    }

    entry_memory_mtrigger(ch);
    greet_memory_mtrigger(ch);

    return 1;
}

static int perform_enter_obj(struct char_data *ch, struct obj_data *obj, int need_specials_check) {
    room_rnum was_in = IN_ROOM(ch);
    int could_move = false;
    struct follow_type *k;

    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return (0);
    }

    if (GET_OBJ_TYPE(obj) == ITEM_VEHICLE ||
        GET_OBJ_TYPE(obj) == ITEM_PORTAL) {
        if (OBJVAL_FLAGGED(obj, CONT_CLOSED)) {
            send_to_char(ch, "But it's closed!\r\n");
        } else if ((GET_OBJ_VAL(obj, VAL_PORTAL_DEST) != NOWHERE) &&
                   (real_room(GET_OBJ_VAL(obj, VAL_PORTAL_DEST)) != NOWHERE)) {
            if (GET_OBJ_VAL(obj, VAL_PORTAL_DEST) >= 45000 && GET_OBJ_VAL(obj, VAL_PORTAL_DEST) <= 45099) {
                struct char_data *tch, *next_v;
                int filled = false;
                for (tch = world[real_room(GET_OBJ_VAL(obj, VAL_PORTAL_DEST))].people; tch; tch = next_v) {
                    next_v = tch->next_in_room;
                    if (tch) {
                        filled = true;
                    }
                }
                if (filled == true) {
                    send_to_char(ch, "Only one person can fit in there at a time.\r\n");
                    return (0);
                }
            }
            if ((could_move = do_simple_enter(ch, obj, need_specials_check)))
                for (k = ch->followers; k; k = k->next)
                    if ((IN_ROOM(k->follower) == was_in) &&
                        (GET_POS(k->follower) >= POS_STANDING)) {
                        act("You follow $N.\r\n", false, k->follower, nullptr, ch, TO_CHAR);
                        perform_enter_obj(k->follower, obj, 1);
                    }
        } else {
            send_to_char(ch,
                         "It doesn't look like you can enter it at the moment.\r\n");
        }
    } else {
        send_to_char(ch, "You can't enter that!\r\n");
    }
    return could_move;
}

ACMD(do_enter) {
    struct obj_data *obj = nullptr;
    char buf[MAX_INPUT_LENGTH];
    int door, move_dir = -1;

    one_argument(argument, buf);
    auto r = ch->getRoom();

    if (*buf) { /* an argument was supplied, search for door keyword */
        /* Is the object in the room? */
        obj = get_obj_in_list_vis(ch, buf, nullptr, ch->getRoom()->contents);
        /* Is the object in the character's inventory? */
        if (!obj)
            obj = get_obj_in_list_vis(ch, buf, nullptr, ch->contents);
        /* Is the character carrying the object? */
        if (!obj)
            obj = get_obj_in_equip_vis(ch, buf, nullptr, ch->equipment);
        /* We have an object to enter */
        if (obj)
            perform_enter_obj(ch, obj, 0);
            /* Is there a door to enter? */
        else {
            for (door = 0; door < NUM_OF_DIRS; door++)
                if (r->dir_option[door])
                    if (r->dir_option[door]->keyword)
                        if (isname(buf, r->dir_option[door]->keyword))
                            move_dir = door;
            /* Did we find what they wanted to enter. */
            if (move_dir > -1)
                perform_move(ch, move_dir, 1);
            else
                send_to_char(ch, "There is no %s here.\r\n", buf);
        }
    } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_INDOORS)) {
        send_to_char(ch, "You are already indoors.\r\n");
    } else {
        /* try to locate an entrance */
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (r->dir_option[door])
                if (r->dir_option[door]->to_room != NOWHERE)
                    if (!EXIT_FLAGGED(r->dir_option[door], EX_CLOSED) &&
                        ROOM_FLAGGED(r->dir_option[door]->to_room, ROOM_INDOORS))
                        move_dir = door;
        if (move_dir > -1)
            perform_move(ch, move_dir, 1);
        else
            send_to_char(ch, "You can't seem to find anything to enter.\r\n");
    }
}

static int do_simple_leave(struct char_data *ch, struct obj_data *obj, int need_specials_check) {
    room_rnum was_in = IN_ROOM(ch), dest_room = NOWHERE;
    int need_movement = 0;
    struct obj_data *vehicle = nullptr;

    if (GET_OBJ_TYPE(obj) != ITEM_PORTAL) {
        vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(obj, VAL_HATCH_DEST));
    }

    if (vehicle == nullptr && GET_OBJ_TYPE(obj) != ITEM_PORTAL) {
        send_to_char(ch, "That doesn't appear to lead anywhere.\r\n");
        return 0;
    }

    if (GET_OBJ_TYPE(obj) == ITEM_PORTAL && OBJVAL_FLAGGED(obj, CONT_CLOSED)) {
        send_to_char(ch, "But it's closed!\r\n");
        return 0;
    }

    if (vehicle != nullptr) {
        if ((dest_room = IN_ROOM(vehicle)) == NOWHERE) {
            send_to_char(ch, "That doesn't appear to lead anywhere.\r\n");
            return 0;
        }
    }
    if (vehicle == nullptr) {
        if ((dest_room = real_room(GET_OBJ_VAL(obj, VAL_PORTAL_DEST))) == NOWHERE) {
            send_to_char(ch, "That doesn't appear to lead anywhere.\r\n");
            return 0;
        }
    }

    /* charmed? */
    if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master &&
        IN_ROOM(ch) == IN_ROOM(ch->master)) {
        send_to_char(ch, "The thought of leaving your master makes you weep.\r\n");
        act("$n bursts into tears.", false, ch, nullptr, nullptr, TO_ROOM);
        return (0);
    }

    /* move points needed is avg. move loss for src and destination sect type */
    need_movement = calcNeedMovementGravity(ch);
    if (GET_LEVEL(ch) <= 1) {
        need_movement = 0;
    }
    if ((ch->getCurST()) < need_movement && !AFF_FLAGGED(ch, AFF_FLYING) && !IS_NPC(ch)) {
        if (need_specials_check && ch->master)
            send_to_char(ch, "You are too exhausted to follow.\r\n");
        else
            send_to_char(ch, "You are too exhausted.\r\n");

        return (0);
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ATRIUM)) {
        if (!House_can_enter(ch, GET_ROOM_VNUM(dest_room))) {
            send_to_char(ch, "That's private property -- no trespassing!\r\n");
            return (0);
        }
    }
    if (ROOM_FLAGGED(dest_room, ROOM_TUNNEL) &&
        num_pc_in_room(&(world[dest_room])) >= CONFIG_TUNNEL_SIZE) {
        if (CONFIG_TUNNEL_SIZE > 1)
            send_to_char(ch, "There isn't enough room for you to go there!\r\n");
        else
            send_to_char(ch, "There isn't enough room there for more than one person!\r\n");
        return (0);
    }
    /* Now we know we're allowed to go into the room. */
    if (!(IS_NPC(ch) || ADM_FLAGGED(ch, ADM_WALKANYWHERE)) && !AFF_FLAGGED(ch, AFF_FLYING))
        ch->decCurST(need_movement);

    act("$n leaves $p.", true, ch, vehicle, nullptr, TO_ROOM | TO_SNEAKRESIST);

    if (DRAGGING(ch)) {
        act("@C$n@w drags @c$N@w with $m.@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
    }
    if (CARRYING(ch)) {
        act("@C$n@w carries @c$N@w with $m.@n", true, ch, nullptr, CARRYING(ch), TO_ROOM);
    }
    char_from_room(ch);
    char_to_room(ch, dest_room);

    /* move them first, then move them back if they aren't allowed to go. */
    /* see if an entry trigger disallows the move */
    if (!entry_mtrigger(ch)) {
        char_from_room(ch);
        char_to_room(ch, was_in);
        return 0;
    }

    if (vehicle) {
        act("$n arrives from inside $p.", true, ch, vehicle, nullptr, TO_ROOM | TO_SNEAKRESIST);
    } else {
        act("$n arrives from inside", true, ch, nullptr, nullptr, TO_ROOM | TO_SNEAKRESIST);
    }
    if (DRAGGING(ch)) {
        act("@wYou drag @C$N@w with you.@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@w drags @c$N@w with $m.@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        char_from_room(DRAGGING(ch));
        char_to_room(DRAGGING(ch), IN_ROOM(ch));
        if (SITS(DRAGGING(ch))) {
            obj_from_room(SITS(DRAGGING(ch)));
            obj_to_room(SITS(DRAGGING(ch)), IN_ROOM(ch));
        }
        if (!AFF_FLAGGED(DRAGGING(ch), AFF_KNOCKED) && !AFF_FLAGGED(DRAGGING(ch), AFF_SLEEP) && rand_number(1, 3)) {
            send_to_char(DRAGGING(ch), "You feel your sleeping body being moved.\r\n");
            if (IS_NPC(DRAGGING(ch)) && !FIGHTING(DRAGGING(ch))) {
                set_fighting(DRAGGING(ch), ch);
            }
        }
    }
    if (CARRYING(ch)) {
        act("@wYou carry @C$N@w with you.@n", true, ch, nullptr, CARRYING(ch), TO_CHAR);
        act("@C$n@w carries @c$N@w with $m.@n", true, ch, nullptr, CARRYING(ch), TO_ROOM);
        char_from_room(CARRYING(ch));
        char_to_room(CARRYING(ch), IN_ROOM(ch));
        if (SITS(CARRYING(ch))) {
            obj_from_room(SITS(CARRYING(ch)));
            obj_to_room(SITS(CARRYING(ch)), IN_ROOM(ch));
        }
        if (!AFF_FLAGGED(CARRYING(ch), AFF_KNOCKED) && !AFF_FLAGGED(CARRYING(ch), AFF_SLEEP) && rand_number(1, 3)) {
            send_to_char(CARRYING(ch), "You feel your sleeping body being moved.\r\n");
        }
    }

    char buf3[MAX_STRING_LENGTH];
    send_to_sense(0, "You sense someone ", ch);
    sprintf(buf3, "@D[@GBlip@D]@Y %s\r\n@RSomeone has entered your scouter detection range.@n",
            add_commas(GET_HIT(ch)).c_str());
    send_to_scouter(buf3, ch, 0, 0);

    if (ch->desc != nullptr) {
        act(obj->look_description, true, ch, obj, nullptr, TO_CHAR);
        look_at_room(IN_ROOM(ch), ch, 0);
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_DEATH) && !ADM_FLAGGED(ch, ADM_WALKANYWHERE)) {
        log_death_trap(ch);
        death_cry(ch);
        extract_char(ch);
        return 0;
    }

    entry_memory_mtrigger(ch);
    greet_memory_mtrigger(ch);

    return 1;
}

static int perform_leave_obj(struct char_data *ch, struct obj_data *obj, int need_specials_check) {
    room_rnum was_in = IN_ROOM(ch);
    int could_move = false;
    struct follow_type *k;

    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You are grappling with someone!\r\n");
        return (0);
    }

    if (OBJVAL_FLAGGED(obj, CONT_CLOSED)) {
        send_to_char(ch, "But the way out is closed.\r\n");
    } else {
        if (GET_OBJ_VAL(obj, VAL_HATCH_DEST) != NOWHERE)
            if ((could_move = do_simple_leave(ch, obj, need_specials_check)))
                for (k = ch->followers; k; k = k->next)
                    if ((IN_ROOM(k->follower) == was_in) &&
                        (GET_POS(k->follower) >= POS_STANDING)) {
                        act("You follow $N.\r\n", false, k->follower, nullptr, ch, TO_CHAR);
                        perform_leave_obj(k->follower, obj, 1);
                    }
    }
    return could_move;
}


ACMD(do_leave) {
    int door;
    struct obj_data *obj = nullptr;

    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }

    auto r = ch->getRoom();

    for (obj = r->contents; obj; obj = obj->next_content)
        if (CAN_SEE_OBJ(ch, obj))
            if (GET_OBJ_TYPE(obj) == ITEM_HATCH || GET_OBJ_TYPE(obj) == ITEM_PORTAL) {
                perform_leave_obj(ch, obj, 0);
                return;
            }

    if (OUTSIDE(ch)) {
        send_to_char(ch, "You are outside.. where do you want to go?\r\n");
        return;
    }

    for (door = 0; door < NUM_OF_DIRS; door++) {
        auto e = r->dir_option[door];
        if(!e) continue;
        auto dest = e->getDestination();
        if(!dest) continue;
        if (!EXIT_FLAGGED(e, EX_CLOSED) && !dest->room_flags.test(ROOM_INDOORS)) {
            perform_move(ch, door, 1);
            return;
        }
    }

    send_to_char(ch, "I see no obvious exits to the outside.\r\n");
}

static void handle_fall(struct char_data *ch) {
    int room = -1;
    while (EXIT(ch, 5) && SECT(IN_ROOM(ch)) == SECT_FLYING) {
        room = GET_ROOM_VNUM(EXIT(ch, 5)->to_room);
        char_from_room(ch);
        char_to_room(ch, real_room(room));
        if (CARRYING(ch)) {
            char_from_room(CARRYING(ch));
            char_to_room(CARRYING(ch), real_room(room));
        }
        if (!EXIT(ch, 5) || SECT(IN_ROOM(ch)) != SECT_FLYING) {
            act("@r$n slams into the ground!@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurHealth(ch->getEffMaxPL() / 20, 1);

            act("@rYou slam into the ground!@n", true, ch, nullptr, nullptr, TO_CHAR);
            look_at_room(IN_ROOM(ch), ch, 0);
        } else {
            act("@r$n pummets down toward the ground below!@n", true, ch, nullptr, nullptr, TO_ROOM);
        }
    }
    if (SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM && !CARRIED_BY(ch) && !IS_KANASSAN(ch)) {
        if ((ch->getCurST()) >= (ch->getCarriedWeight())) {
            act("@bYou swim in place.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@b swims in place.@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->decCurST(ch->getCarriedWeight());
            act("@RYou are drowning!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@C$n@b gulps water as $e struggles to stay above the water line.@n", true, ch, nullptr, nullptr,
                TO_ROOM);
            if (GET_HIT(ch) - ((ch->getEffMaxPL()) / 3) <= 0) {
                act("@rYou drown!@n", true, ch, nullptr, nullptr, TO_CHAR);
                act("@R$n@r drowns!@n", true, ch, nullptr, nullptr, TO_ROOM);
                die(ch, nullptr);
                ch->decCurHealthPercent(1, 1);
            } else {
                ch->decCurHealthPercent(.33);
            }
        }
    }

}

static int check_swim(struct char_data *ch) {
    auto can = false;

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SPACE)) {
        auto space_cost = (GET_MAX_MANA(ch) / 1000) + ((ch->getCarriedWeight()) / 2);
        if (ch->getCurKI() >= space_cost)
            can = true;
        ch->decCurKI(space_cost);
        if (!can) send_to_char(ch, "You do not have enough ki to fly through space. You are drifting helplessly.\r\n");
        return can;
    } else {
        auto swim_cost = (ch->getCarriedWeight()) - 1;
        if (ch->getCurST() >= swim_cost)
            can = true;
        ch->decCurST(swim_cost);
        if (!can) send_to_char(ch, "You are too tired to swim!\r\n");
        return can;
    }
}

static bool isPlanet(const area_data& a) {
    return a.type == AreaType::CelestialBody;
}

ACMD(do_fly) {
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (ABSORBING(ch) || ABSORBBY(ch)) {
        send_to_char(ch, "You can't fly, you are struggling with someone right now!");
        return;
    }
    if (GRAPPLING(ch) || GRAPPLED(ch)) {
        send_to_char(ch, "You can't fly, you are struggling with someone right now!");
        return;
    }
    if (!IS_NPC(ch)) {
        if (PLR_FLAGGED(ch, PLR_HEALT)) {
            send_to_char(ch, "You are inside a healing tank!\r\n");
            return;
        }
        if (PLR_FLAGGED(ch, PLR_PILOTING)) {
            send_to_char(ch, "You are busy piloting a ship!\r\n");
            return;
        }
    }

    if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_FOCUS) < 30 && !IS_ANDROID(ch)) {
        send_to_char(ch, "You do not have enough focus to hold yourself aloft.\r\n");
        send_to_char(ch, "@wOOC@D: @WYou need the skill Focus at @m30@W.@n\r\n");
        return;
    }

    if (!*arg) {
        if (AFF_FLAGGED(ch, AFF_FLYING) && SECT(IN_ROOM(ch)) != SECT_FLYING && SECT(IN_ROOM(ch)) != SECT_SPACE) {
            act("@WYou slowly settle down to the ground.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@W$n slowly settles down to the ground.@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->affected_by.reset(AFF_FLYING);
            GET_ALT(ch) = 0;
            return;
        }

        if (AFF_FLAGGED(ch, AFF_FLYING) && SECT(IN_ROOM(ch)) == SECT_FLYING) {
            act("@WYou begin to plummet to the ground!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@W$n starts to pummet to the ground below!@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->affected_by.reset(AFF_FLYING);
            GET_ALT(ch) = 0;
            handle_fall(ch);
            return;
        }
        if (AFF_FLAGGED(ch, AFF_FLYING) && SECT(IN_ROOM(ch)) == SECT_SPACE) {
            act("@WYou let yourself drift aimlessly through space.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@W$n starts to drift slowly.!@n", true, ch, nullptr, nullptr, TO_ROOM);
            ch->affected_by.reset(AFF_FLYING);
            GET_ALT(ch) = 0;
            return;
        }
        if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 100 && !IS_ANDROID(ch)) {
            send_to_char(ch, "You do not have the ki to fly.");
            return;
        } else {
            reveal_hiding(ch, 0);
            act("@WYou slowly take off into the sky.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@W$n slowly takes off into the sky.@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (SITS(ch)) {
                SITTING(SITS(ch)) = nullptr;
                SITS(ch) = nullptr;
            }
            if (GET_POS(ch) < POS_STANDING) {
                GET_POS(ch) = POS_STANDING;
            }
            ch->affected_by.set(AFF_FLYING);
            GET_ALT(ch) = 1;
            ch->decCurKI(ch->getMaxKI() / 100);
        }
    }
    if (!strcasecmp("high", arg)) {
        if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 100 && !IS_ANDROID(ch)) {
            send_to_char(ch, "You do not have the ki to fly.");
            return;
        } else {
            reveal_hiding(ch, 0);
            act("@WYou rocket high into the sky.@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@W$n rockets high into the sky.@n", true, ch, nullptr, nullptr, TO_ROOM);
            if (SITS(ch)) {
                SITTING(SITS(ch)) = nullptr;
                SITS(ch) = nullptr;
            }
            if (GET_POS(ch) < POS_STANDING) {
                GET_POS(ch) = POS_STANDING;
            }
            ch->affected_by.set(AFF_FLYING);
            GET_ALT(ch) = 2;
            ch->decCurKI(ch->getMaxKI() / 100);
        }
    }
    if (!strcasecmp("space", arg)) {
        if (!OUTSIDE(ch)) {
            send_to_char(ch, "You are not outside!");
            return;
        }
        if ((ch->getCurKI()) < GET_MAX_MANA(ch) / 10 && !IS_ANDROID(ch)) {
            send_to_char(ch, "You do not have the ki to fly to space.");
            return;
        }
        if (FIGHTING(ch)) {
            send_to_char(ch, "You are too busy fighting!");
            return;
        }

        auto r = ch->getRoom();
        auto dest = r->getLaunchDestination();

        auto planet = ch->getMatchingArea(area_data::isPlanet);
        if(!dest) {
            send_to_char(ch, "You can't fly to space from here!");
            return;
        }

        reveal_hiding(ch, 0);
        GET_ALT(ch) = 2;
        ch->affected_by.set(AFF_FLYING);
        if (!block_calc(ch)) {
            return;
        }
        GET_ALT(ch) = 0;
        ch->affected_by.reset(AFF_FLYING);

        if(planet) {
            fly_planet(IN_ROOM(ch), "can be seen blasting off into space!@n\r\n", ch);
            send_to_sense(1, "leaving the planet", ch);
            send_to_scouter("A powerlevel signal has left the planet", ch, 0, 2);
        }


        act("@CYou blast off from the ground and rocket through the air. Your speed increases until you manage to reach the brink of space!@n",
            true, ch, nullptr, nullptr, TO_CHAR);
        act("@C$n blasts off from the ground and rockets through the air. You quickly lose sight of $m as $e continues upward!@n",
            true, ch, nullptr, nullptr, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, dest.value());
        if(planet) {
            act("@C$n blasts up from the atmosphere below and then comes to a stop.@n", true, ch, nullptr, nullptr,
            TO_ROOM);
            send_to_char(ch, "@mOOC: Use the command 'land' to return to the planet from here.@n\r\n");
        }

        if (!IS_ANDROID(ch)) {
            ch->decCurKI(ch->getMaxKI() / 10);
        }
        WAIT_STATE(ch, PULSE_3SEC);
        return;
    }
}

static void autochair(struct char_data *ch, struct obj_data *chair) {
    // TODO: Make this configurable.
    SITTING(chair) = nullptr;
    SITS(ch) = nullptr;
    if (CAN_WEAR(chair, ITEM_WEAR_TAKE) && GET_OBJ_TYPE(chair) != ITEM_CHAIR && ch->canCarryWeight(chair)) {
        obj_from_room(chair);
        obj_to_char(chair, ch);
        act("You pick up $p.", true, ch, chair, nullptr, TO_CHAR);
        act("$n picks up $p.", true, ch, chair, nullptr, TO_ROOM);
    }
}

ACMD(do_stand) {
    auto chair = SITS(ch);
    if (AFF_FLAGGED(ch, AFF_KNOCKED)) {
        send_to_char(ch, "You are knocked out cold for right now!\r\n");
        return;
    }
    if (!IS_NPC(ch) && GET_LIMBCOND(ch, 2) <= 0 && GET_LIMBCOND(ch, 3) <= 0) {
        send_to_char(ch, "With what legs will you be standing up on?\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "You are busy piloting a ship!\r\n");
        return;
    }

    switch (GET_POS(ch)) {
        case POS_STANDING:
            send_to_char(ch, "You are already standing.\r\n");
            break;
        case POS_SITTING:
            reveal_hiding(ch, 0);
            send_to_char(ch, "You stand up.\r\n");
            act("$n clambers to $s feet.", true, ch, nullptr, nullptr, TO_ROOM);
            if (chair) autochair(ch, chair);
            /* May be sitting for some reason and may still be fighting. */
            GET_POS(ch) = FIGHTING(ch) ? POS_FIGHTING : POS_STANDING;
            break;
        case POS_RESTING:
            send_to_char(ch, "You stop resting, and stand up.\r\n");
            act("$n stops resting, and clambers to $s feet.", true, ch, nullptr, nullptr, TO_ROOM);
            if (chair) autochair(ch, chair);
            GET_POS(ch) = POS_STANDING;
            break;
        case POS_SLEEPING:
            send_to_char(ch, "You have to wake up first!\r\n");
            break;
        default:
            send_to_char(ch, "You stop floating around, and put your feet on the ground.\r\n");
            act("$n stops floating around, and puts $s feet on the ground.",
                true, ch, nullptr, nullptr, TO_ROOM);
            GET_POS(ch) = POS_STANDING;
            break;
    }
}

ACMD(do_sit) {
    struct obj_data *chair = nullptr;
    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg);

    if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "You are busy piloting a ship!\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }

    if (DRAGGING(ch)) {
        act("@WYou stop dragging @C$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@W stops dragging @c$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        DRAGGED(DRAGGING(ch)) = nullptr;
        DRAGGING(ch) = nullptr;
    }
    if (CARRYING(ch)) {
        send_to_char(ch, "You are busy carrying someone!\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_FLYING)) {
        do_fly(ch, nullptr, 0, 0);
    }

    if (!*arg) {
        switch (GET_POS(ch)) {
            case POS_STANDING:
                reveal_hiding(ch, 0);
                send_to_char(ch, "You sit down.\r\n");
                act("$n sits down.", false, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SITTING;
                break;
            case POS_SITTING:
                send_to_char(ch, "You're sitting already.\r\n");
                break;
            case POS_RESTING:
                send_to_char(ch, "You stop resting, and sit up.\r\n");
                act("$n stops resting.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SITTING;
                break;
            case POS_SLEEPING:
                send_to_char(ch, "You have to wake up first.\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Sit down while fighting? Are you MAD?\r\n");
                break;
            default:
                send_to_char(ch, "You stop floating around, and sit down.\r\n");
                act("$n stops floating around, and sits down.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SITTING;
                break;
        }
    } else {
        if (SITS(ch)) {
            send_to_char(ch, "You are already on something!\r\n");
            return;
        }
        if (!(chair = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "That isn't here.\r\n");
            return;
        }
        if (GET_OBJ_VNUM(chair) == 65) {
            send_to_char(ch, "You can't get on that!\r\n");
            return;
        }
        if (SITTING(chair)) {
            send_to_char(ch, "Someone is already on that one!\r\n");
            return;
        }
        if (GET_OBJ_TYPE(chair) != ITEM_CHAIR && GET_OBJ_TYPE(chair) != ITEM_BED) {
            send_to_char(ch, "You can't sit on that!\r\n");
            return;
        }
        if (GET_OBJ_SIZE(chair) + 1 < get_size(ch)) {
            send_to_char(ch, "You are too large for it!\r\n");
            return;
        }
        switch (GET_POS(ch)) {
            case POS_STANDING:
                reveal_hiding(ch, 0);
                act("You sit down on $p.", false, ch, chair, nullptr, TO_CHAR);
                act("$n sits down on $p.", false, ch, chair, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SITTING;
                SITS(ch) = chair;
                SITTING(chair) = ch;
                break;
            case POS_SITTING:
                send_to_char(ch, "You should stand up first.\r\n");
                break;
            case POS_RESTING:
                send_to_char(ch, "You should stand up first.\r\n");
                break;
            case POS_SLEEPING:
                send_to_char(ch, "You have to wake up first.\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Sit down while fighting? Are you MAD?\r\n");
                break;
            default:
                send_to_char(ch, "You stop floating around, and sit down.\r\n");
                act("$n stops floating around, and sits down.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SITTING;
                break;
        }
    }
}

ACMD(do_rest) {
    struct obj_data *chair = nullptr;
    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg);

    if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "You are busy piloting a ship!\r\n");
        return;
    }
    if (SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM) {
        send_to_char(ch, "You can't rest here!\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
        if (GET_SKILL(ch, SKILL_BARRIER)) {
            send_to_char(ch, "You have a barrier around you and can't rest.\r\n");
            return;
        } else {
            GET_BARRIER(ch) = 0;
            ch->affected_by.reset(AFF_SANCTUARY);
        }
    }
    if (FIGHTING(ch)) {
        send_to_char(ch, "You are a bit busy at the moment!\r\n");
        return;
    }
    if (GET_KAIOKEN(ch) > 0) {
        send_to_char(ch, "You are utilizing kaioken and can't rest!\r\n");
        return;
    }

    if (DRAGGING(ch)) {
        act("@WYou stop dragging @C$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@W stops dragging @c$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        DRAGGED(DRAGGING(ch)) = nullptr;
        DRAGGING(ch) = nullptr;
    }

    if (CARRYING(ch)) {
        send_to_char(ch, "You are carrying someone!\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_FLYING)) {
        do_fly(ch, nullptr, 0, 0);
    }

    if (!*arg) {
        if (SITS(ch)) {
            chair = SITS(ch);
            if (GET_OBJ_TYPE(chair) != ITEM_BED) {
                send_to_char(ch, "You can't lay on that!\r\n");
                return;
            }
        }
        switch (GET_POS(ch)) {
            case POS_STANDING:
                reveal_hiding(ch, 0);
                send_to_char(ch, "You lay down and rest your tired bones.\r\n");
                act("$n lays down and rests.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_RESTING;
                break;
            case POS_SITTING:
                send_to_char(ch, "You rest your tired bones.\r\n");
                act("$n rests.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_RESTING;
                break;
            case POS_RESTING:
                send_to_char(ch, "You are already resting.\r\n");
                break;
            case POS_SLEEPING:
                send_to_char(ch, "You have to wake up first.\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Rest while fighting?  Are you MAD?\r\n");
                break;
            default:
                send_to_char(ch, "You stop floating around, and stop to rest your tired bones.\r\n");
                act("$n stops floating around, and rests.", false, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_RESTING;
                break;
        }
    } else {
        if (SITS(ch)) {
            send_to_char(ch, "You are already on something!\r\n");
            return;
        }
        if (!(chair = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "That isn't here.\r\n");
            return;
        }
        if (GET_OBJ_VNUM(chair) == 65) {
            send_to_char(ch, "You can't get on that!\r\n");
            return;
        }
        if (SITTING(chair)) {
            send_to_char(ch, "Someone is already on that one!\r\n");
            return;
        }
        if (GET_OBJ_TYPE(chair) != ITEM_BED) {
            send_to_char(ch, "You can't lay on that!\r\n");
            return;
        }
        if (GET_OBJ_SIZE(chair) + 1 < get_size(ch)) {
            send_to_char(ch, "You are too large for it!\r\n");
            return;
        }
        switch (GET_POS(ch)) {
            case POS_STANDING:
                reveal_hiding(ch, 0);
                act("You lay down and rest on $p.", true, ch, chair, nullptr, TO_CHAR);
                act("$n lays down and rests on $p.", true, ch, chair, nullptr, TO_ROOM);
                SITS(ch) = chair;
                SITTING(chair) = ch;
                GET_POS(ch) = POS_RESTING;
                break;
            case POS_SITTING:
                send_to_char(ch, "You should get up first.\r\n");
                break;
            case POS_RESTING:
                send_to_char(ch, "You are already resting.\r\n");
                break;
            case POS_SLEEPING:
                send_to_char(ch, "You have to wake up first.\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Rest while fighting?  Are you MAD?\r\n");
                break;
            default:
                send_to_char(ch, "You stop floating around, and stop to rest your tired bones.\r\n");
                act("$n stops floating around, and rests.", false, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_RESTING;
                break;
        }
    }
}

ACMD(do_sleep) {
    struct obj_data *chair = nullptr;
    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg);

    if (!IS_NPC(ch)) {
        if (PRF_FLAGGED(ch, PRF_ARENAWATCH)) {
            ch->pref.reset(PRF_ARENAWATCH);
            ARENA_IDNUM(ch) = -1;
            send_to_char(ch, "You stop watching the arena action.\r\n");
        }
    }

    if (GET_BONUS(ch, BONUS_INSOMNIAC)) {
        send_to_char(ch, "You don't feel the least bit tired.\r\n");
        return;
    }

    if (SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM) {
        send_to_char(ch, "You can't rest here!\r\n");
        return;
    }

    if (PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "You are busy piloting a ship!\r\n");
        return;
    }
    if (FIGHTING(ch)) {
        send_to_char(ch, "You are a bit busy at the moment!\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }
    if (PLR_FLAGGED(ch, PLR_POWERUP)) {
        send_to_char(ch, "You are busy powering up!\r\n");
        return;
    }
    if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
        if (GET_SKILL(ch, SKILL_BARRIER) > 0) {
            send_to_char(ch, "You have a barrier around you and can't sleep.\r\n");
            return;
        } else {
            GET_BARRIER(ch) = 0;
            ch->affected_by.reset(AFF_SANCTUARY);
        }
    }
    if (GET_KAIOKEN(ch) > 0) {
        send_to_char(ch, "You are utilizing kaioken and can't sleep!\r\n");
        return;
    }
    if (GET_SLEEPT(ch) > 0) {
        send_to_char(ch, "You aren't sleepy enough.\r\n");
        return;
    }
    if (DRAGGING(ch)) {
        act("@WYou stop dragging @C$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@W stops dragging @c$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        DRAGGED(DRAGGING(ch)) = nullptr;
        DRAGGING(ch) = nullptr;
    }
    if (CARRYING(ch)) {
        send_to_char(ch, "You are carrying someone!\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_FLYING)) {
        do_fly(ch, nullptr, 0, 0);
    }

    if (!*arg) {
        if (SITS(ch)) {
            chair = SITS(ch);
            if (GET_OBJ_TYPE(chair) != ITEM_BED) {
                send_to_char(ch, "You can't sleep on %s.\r\n", chair->short_description);
                return;
            }
        }
        switch (GET_POS(ch)) {
            case POS_STANDING:
            case POS_SITTING:
            case POS_RESTING:
                reveal_hiding(ch, 0);
                send_to_char(ch, "You go to sleep.\r\n");
                act("$n lies down and falls asleep.", true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SLEEPING;
                /* Fury Mode Loss for halfbreeds */

                if (PLR_FLAGGED(ch, PLR_FURY)) {
                    send_to_char(ch,
                                 "Your fury subsides for now. Next time try to take advantage of it before you calm down.\r\n");
                    ch->playerFlags.reset(PLR_FURY);
                }

                /* Fury Mode Loss for halfbreeds */

                if (GET_STUPIDKISS(ch) > 0) {
                    GET_STUPIDKISS(ch) = 0;
                    send_to_char(ch, "You forget about that stupid kiss.\r\n");
                }
                break;
            case POS_SLEEPING:
                send_to_char(ch, "You are already sound asleep.\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Sleep while fighting?  Are you MAD?\r\n");
                break;
            default:
                send_to_char(ch, "You stop floating around, and lie down to sleep.\r\n");
                act("$n stops floating around, and lie down to sleep.",
                    true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SLEEPING;
                break;
        }
    } else {
        if (SITS(ch)) {
            send_to_char(ch, "You are already on something!\r\n");
            return;
        }
        if (!(chair = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
            send_to_char(ch, "That isn't here.\r\n");
            return;
        }
        if (GET_OBJ_VNUM(chair) == 65) {
            send_to_char(ch, "You can't get on that!\r\n");
            return;
        }
        if (SITTING(chair)) {
            send_to_char(ch, "Someone is already on that one!\r\n");
            return;
        }
        if (GET_OBJ_TYPE(chair) != ITEM_BED) {
            send_to_char(ch, "You can't sleep on that!\r\n");
            return;
        }
        if (GET_OBJ_SIZE(chair) + 1 < get_size(ch)) {
            send_to_char(ch, "You are too large for it!\r\n");
            return;
        }
        switch (GET_POS(ch)) {
            case POS_RESTING:
            case POS_SITTING:
                send_to_char(ch, "You need to get up first!\r\n");
                break;
            case POS_STANDING:
                reveal_hiding(ch, 0);
                act("You lay down on $p and sleep.", false, ch, chair, nullptr, TO_CHAR);
                act("$n lays down on $p and sleeps.", false, ch, chair, nullptr, TO_ROOM);
                /* Fury Mode Loss for halfbreeds */

                if (PLR_FLAGGED(ch, PLR_FURY)) {
                    send_to_char(ch,
                                 "Your fury subsides for now. Next time try to take advantage of it before you calm down.\r\n");
                    ch->playerFlags.reset(PLR_FURY);
                }

                /* Fury Mode Loss for halfbreeds */

                if (GET_STUPIDKISS(ch) > 0) {
                    GET_STUPIDKISS(ch) = 0;
                    send_to_char(ch, "You forget about that stupid kiss.\r\n");
                }
                SITS(ch) = chair;
                SITTING(chair) = ch;
                GET_POS(ch) = POS_SLEEPING;
                break;
            case POS_SLEEPING:
                send_to_char(ch, "You are already sound asleep.\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "Sleep while fighting?  Are you MAD?\r\n");
                break;
            default:
                send_to_char(ch, "You stop floating around, and lie down to sleep.\r\n");
                act("$n stops floating around, and lie down to sleep.",
                    true, ch, nullptr, nullptr, TO_ROOM);
                GET_POS(ch) = POS_SLEEPING;
                break;
        }
    }
}

ACMD(do_wake) {
    char arg[MAX_INPUT_LENGTH];
    struct char_data *vict;
    int self = 0;

    one_argument(argument, arg);

    if (AFF_FLAGGED(ch, AFF_KNOCKED)) {
        send_to_char(ch, "You are knocked out cold for right now!\r\n");
        return;
    }

    if (GET_BONUS(ch, BONUS_LATE) && GET_POS(ch) == POS_SLEEPING) {
        send_to_char(ch, "Nah you're enjoying sleeping too much.\r\n");
        return;
    }

    if (*arg) {
        if (GET_POS(ch) == POS_SLEEPING)
            send_to_char(ch, "Maybe you should wake yourself up first.\r\n");
        else if ((vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM)) == nullptr)
            send_to_char(ch, "%s", CONFIG_NOPERSON);
        else if (vict == ch)
            self = 1;
        else if (AWAKE(vict))
            act("$E is already awake.", false, ch, nullptr, vict, TO_CHAR);
        else if (AFF_FLAGGED(vict, AFF_SLEEP))
            act("You can't wake $M up!", false, ch, nullptr, vict, TO_CHAR);
        else if (GET_POS(vict) < POS_SLEEPING)
            act("$E's in pretty bad shape!", false, ch, nullptr, vict, TO_CHAR);
        else if (AFF_FLAGGED(vict, AFF_KNOCKED))
            send_to_char(ch, "They are knocked out cold for right now!\r\n");
        else if (GET_BONUS(ch, BONUS_LATE))
            send_to_char(ch, "They say 'Yeah yeah...' and then roll back over.\r\n");
        else {
            act("You wake $M up.", false, ch, nullptr, vict, TO_CHAR);
            act("You are awakened by $n.", false, ch, nullptr, vict, TO_VICT | TO_SLEEP);
            GET_POS(vict) = POS_SITTING;
            if (DRAGGED(vict)) {
                act("@WYou stop dragging @C$N@W!@n", true, DRAGGED(vict), nullptr, vict, TO_CHAR);
                act("@C$n@W stops dragging @c$N@W!@n", true, DRAGGED(vict), nullptr, vict, TO_ROOM);
                DRAGGING(DRAGGED(vict)) = nullptr;
                DRAGGED(vict) = nullptr;
            }
            if (CARRIED_BY(vict)) {
                if (GET_ALIGNMENT(CARRIED_BY(vict)) > 50) {
                    carry_drop(CARRIED_BY(vict), 0);
                } else {
                    carry_drop(CARRIED_BY(vict), 1);
                }
            }
        }
        if (!self)
            return;
    }
    if (AFF_FLAGGED(ch, AFF_SLEEP))
        send_to_char(ch, "You can't wake up!\r\n");
    else if (GET_POS(ch) > POS_SLEEPING)
        send_to_char(ch, "You are already awake...\r\n");
    else {
        send_to_char(ch, "You awaken, and sit up.\r\n");
        act("$n awakens.", true, ch, nullptr, nullptr, TO_ROOM);
        if (DRAGGED(ch)) {
            act("@WYou stop dragging @C$N@W!@n", true, DRAGGED(ch), nullptr, ch, TO_CHAR);
            act("@C$n@W stops dragging you!@n", true, DRAGGED(ch), nullptr, ch, TO_VICT);
            act("@C$n@W stops dragging @c$N@W!@n", true, DRAGGED(ch), nullptr, ch, TO_NOTVICT);
            DRAGGING(DRAGGED(ch)) = nullptr;
            DRAGGED(ch) = nullptr;
        }
        if (CARRIED_BY(ch)) {
            if (GET_ALIGNMENT(CARRIED_BY(ch)) > 50) {
                carry_drop(CARRIED_BY(ch), 0);
            } else {
                carry_drop(CARRIED_BY(ch), 1);
            }
        }
        GET_POS(ch) = POS_SITTING;
    }
}

ACMD(do_follow) {
    char buf[MAX_INPUT_LENGTH];
    struct char_data *leader;

    one_argument(argument, buf);

    if (PLR_FLAGGED(ch, PLR_HEALT)) {
        send_to_char(ch, "You are inside a healing tank!\r\n");
        return;
    }

    if (*buf) {
        if (!(leader = get_char_vis(ch, buf, nullptr, FIND_CHAR_ROOM))) {
            send_to_char(ch, "%s", CONFIG_NOPERSON);
            return;
        }
    } else {
        send_to_char(ch, "Whom do you wish to follow?\r\n");
        return;
    }

    if (ch->master == leader) {
        act("You are already following $M.", false, ch, nullptr, leader, TO_CHAR);
        return;
    }
    if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master)) {
        act("But you only feel like following $N!", false, ch, nullptr, ch->master, TO_CHAR);
    } else {            /* Not Charmed follow person */
        if (leader == ch) {
            if (!ch->master) {
                send_to_char(ch, "You are already following yourself.\r\n");
                return;
            }
            stop_follower(ch);
        } else {
            if (circle_follow(ch, leader)) {
                send_to_char(ch, "Sorry, but following in loops is not allowed.\r\n");
                return;
            }
            if (ch->master)
                stop_follower(ch);
            ch->affected_by.reset(AFF_GROUP);
            reveal_hiding(ch, 0);
            add_follower(ch, leader);
        }
    }
}
