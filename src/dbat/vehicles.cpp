/*************************************************************************
*  File: vehicles.c                                    Part of CircleMUD *
*  Usage: Vechicle related code						 *
*									 *
*  All rights reserved.  See license.doc for complete information.	 *
*									 *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University*
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.              *
*  Vehicle.c written by Chris Jacobson <fear@athenet.net>		 *
*************************************************************************/
#include "dbat/vehicles.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"
#include "dbat/db.h"
#include "dbat/constants.h"
#include "dbat/class.h"
#include "dbat/races.h"
#include "dbat/act.informative.h"

#ifndef EXITN
#  define EXITN(room, door)        (world[room].dir_option[door])
#endif

static int ship_land_location(struct char_data *ch, struct obj_data *vehicle, char *arg);

static void disp_ship_locations(struct char_data *ch, struct obj_data *vehicle);

/* This shows the player what locations the planet has to land at. */
static void disp_ship_locations(struct char_data *ch, struct obj_data *vehicle) {
    if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 50) { // Above Earth
        send_to_char(ch, "@D------------------[ @GEarth@D ]------------------@c\n");
        send_to_char(ch, "Nexus City, South Ocean, Nexus field, Cherry Blossom Mountain,\n");
        send_to_char(ch, "Sandy Desert, Northern Plains, Korin's Tower, Kami's Lookout,\n");
        send_to_char(ch, "Shadow Forest, Decrepit Area, West City, Hercule Beach, Satan City.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 51) { // Above Frigid
        send_to_char(ch, "@D------------------[ @CFrigid@D ]------------------@c\n");
        send_to_char(ch, "Ice Crown City, Ice Highway, Topica Snowfield, Glug's Volcano,\n");
        send_to_char(ch, "Platonic Sea, Slave City, Acturian Woods, Desolate Demesne,\n");
        send_to_char(ch, "Chateau Ishran, Wyrm Spine Mountain, Cloud Ruler Temple, Koltoan mine.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 52) { // Above Konack
        send_to_char(ch, "@D------------------[ @MKonack@D ]------------------@c\n");
        send_to_char(ch, "Great Oroist Temple, Elzthuan Forest, Mazori Farm, Dres,\n");
        send_to_char(ch, "Colvian Farm, St Alucia, Meridius Memorial, Desert of Illusion,\n");
        send_to_char(ch, "Plains of Confusion, Turlon Fair, Wetlands, Kerberos,\n");
        send_to_char(ch, "Shaeras Mansion, Slavinus Ravine, Furian Citadel.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 53) { // Above Vegeta
        send_to_char(ch, "@D------------------[ @YVegeta@D ]------------------@c\n");
        send_to_char(ch, "Vegetos City, Blood Dunes, Ancestral Mountains, Destopa Swamp,\n");
        send_to_char(ch, "Pride Forest, Pride tower, Ruby Cave.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 198) { // Above Cerria
        send_to_char(ch, "@D------------------[ @MCerria@D ]------------------@c\n");
        send_to_char(ch, "Cerria Colony, Fistarl Volcano, Crystalline Forest.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 54) { // Above Namek
        send_to_char(ch, "@D------------------[ @gNamek@D ]------------------@c\n");
        send_to_char(ch, "Senzu Village, Guru's House, Crystalline Cave, Elder Village,\n");
        send_to_char(ch, "Frieza's Ship, Kakureta Village.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 55) { // Above Aether
        send_to_char(ch, "@D------------------[ @BAether@D ]-----------------@c\n");
        send_to_char(ch, "Haven City, Serenity Lake, Kaiju Forest, Ortusian Temple,\n");
        send_to_char(ch, "Silent Glade.\n");
        send_to_char(ch, "@D--------------------------------------------@n\n");
        send_to_char(ch, "@D------------------[ @BAether@D ]-----------------@c\n");
        send_to_char(ch, "Haven City, Serenity Lake, Kaiju Forest, Ortusian Temple,\n");
        send_to_char(ch, "Silent Glade.\n");
        send_to_char(ch, "@D--------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 56) { // Above Yardrat
        send_to_char(ch, "@D-----------------[ @mYardrat@D ]-----------------@c\n");
        send_to_char(ch, "Yardra City, Jade Forest, Jade Cliffs, Mount Valaria.\n");
        send_to_char(ch, "@D-------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 57) { // Above Zennith
        send_to_char(ch, "@D-----------------[ @CZennith@D ]-----------------@c\n");
        send_to_char(ch, "Utatlan City, Zenith Jungle, Ancient Castle.\n");
        send_to_char(ch, "@D-------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 58) { // Above Kanassa
        send_to_char(ch, "@D-----------------[ @CKanassa@D ]-----------------@c\n");
        send_to_char(ch, "Aquis City, Yunkai Pirate Base.\n");
        send_to_char(ch, "@D-------------------------------------------@n\n");
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 59) { // Above Arlia
        send_to_char(ch, "@D------------------[ @MArlia@D ]------------------@c\n");
        send_to_char(ch, "Janacre, Arlian Wasteland, Arlia Mine, Kemabra Wastes.\n");
        send_to_char(ch, "@D---------------------------------------------@n\n");
    } else {
        send_to_char(ch, "You are not above a planet!\r\n");
    }
}

static int ship_land_location(struct char_data *ch, struct obj_data *vehicle, char *arg) {
    int landspot = 50;
    if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 50) { // Above Earth
        if (!strcasecmp(arg, "Nexus City")) {
            landspot = 300;
            landspot += rand_number(0, 63);
            return (landspot);
        } else if (!strcasecmp(arg, "South Ocean")) {
            landspot = 800;
            landspot += rand_number(0, 99);
            return (landspot);
        } else if (!strcasecmp(arg, "Nexus Field")) {
            landspot = 1150;
            landspot += rand_number(-16, 28);
            return (landspot);
        } else if (!strcasecmp(arg, "Cherry Blossom Mountain")) {
            landspot = 1180;
            landspot += rand_number(0, 19);
            return (landspot);
        } else if (!strcasecmp(arg, "Sandy Desert")) {
            landspot = 1287;
            landspot += rand_number(0, 64);
            return (landspot);
        } else if (!strcasecmp(arg, "Northern Plains")) {
            landspot = 1428;
            landspot += rand_number(0, 55);
            return (landspot);
        } else if (!strcasecmp(arg, "Korin's Tower")) {
            return (1456);
        } else if (!strcasecmp(arg, "Kami's Lookout")) {
            landspot = 1506;
            landspot += rand_number(0, 30);
            return (landspot);
        } else if (!strcasecmp(arg, "Shadow Forest")) {
            landspot = 1600;
            landspot += rand_number(0, 66);
            return (landspot);
        } else if (!strcasecmp(arg, "Decrepit Area")) {
            return (1710);
        } else if (!strcasecmp(arg, "West City")) {
            return (19510);
        } else if (!strcasecmp(arg, "Hercule Beach")) {
            landspot = 2141;
            landspot += rand_number(0, 53);
            return (landspot);
        } else if (!strcasecmp(arg, "Satan City")) {
            landspot = 1150;
            landspot += rand_number(-16, 28);
            return (landspot);
            return (13020);
        } else {
            send_to_char(ch, "You don't know where that made up place is, but decided to land anyway.");
            return (300);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 51) { // Above Frigid
        if (!strcasecmp(arg, "Ice Crown City")) {
            return (4264);
        } else if (!strcasecmp(arg, "Ice Highway")) {
            return (4300);
        } else if (!strcasecmp(arg, "Topica Snowfield")) {
            return (4351);
        } else if (!strcasecmp(arg, "Glug's Volcano")) {
            return (4400);
        } else if (!strcasecmp(arg, "Platonic Sea")) {
            return (4600);
        } else if (!strcasecmp(arg, "Slave City")) {
            return (4800);
        } else if (!strcasecmp(arg, "Acturian Woods")) {
            return (5100);
        } else if (!strcasecmp(arg, "Desolate Demesne")) {
            return (5150);
        } else if (!strcasecmp(arg, "Chateau Ishran")) {
            return (5165);
        } else if (!strcasecmp(arg, "Wyrm Spine Mountain")) {
            return (5200);
        } else if (!strcasecmp(arg, "Cloud Ruler Temple")) {
            return (5500);
        } else if (!strcasecmp(arg, "Koltoan Mine")) {
            return (4944);
        } else {
            send_to_char(ch, "You don't know where that made up place is, but decided to land anyway.");
            return (4264);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 52) { // Above Konack
        if (!strcasecmp(arg, "Tiranoc City")) {
            return (8006);
        } else if (!strcasecmp(arg, "Great Oroist Temple")) {
            return (8300);
        } else if (!strcasecmp(arg, "Elzthuan Forest")) {
            return (8400);
        } else if (!strcasecmp(arg, "Mazori Farm")) {
            return (8447);
        } else if (!strcasecmp(arg, "Dres")) {
            return (8500);
        } else if (!strcasecmp(arg, "Colvian Farm")) {
            return (8600);
        } else if (!strcasecmp(arg, "St Alucia")) {
            return (8700);
        } else if (!strcasecmp(arg, "Meridius Memorial")) {
            return (8800);
        } else if (!strcasecmp(arg, "Desert of Illusion")) {
            return (8900);
        } else if (!strcasecmp(arg, "Plains of Confusion")) {
            return (8954);
        } else if (!strcasecmp(arg, "Turlon Fair")) {
            return (9200);
        } else if (!strcasecmp(arg, "Wetlands")) {
            return (9700);
        } else if (!strcasecmp(arg, "Kerberos")) {
            return (9855);
        } else if (!strcasecmp(arg, "Shaeras Mansion")) {
            return (9864);
        } else if (!strcasecmp(arg, "Slavinus Ravine")) {
            return (9900);
        } else if (!strcasecmp(arg, "Furian Citadel")) {
            return (9949);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (8006);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 53) { // Above Vegeta
        if (!strcasecmp(arg, "Vegetos City")) {
            return (2226);
        } else if (!strcasecmp(arg, "Blood Dunes")) {
            return (2600);
        } else if (!strcasecmp(arg, "Ancestral Mountains")) {
            return (2616);
        } else if (!strcasecmp(arg, "Destopa Swamp")) {
            return (2709);
        } else if (!strcasecmp(arg, "Pride forest")) {
            return (2800);
        } else if (!strcasecmp(arg, "Pride Tower")) {
            return (2899);
        } else if (!strcasecmp(arg, "Ruby Cave")) {
            return (2615);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (2226);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 54) { // Above Namek
        if (!strcasecmp(arg, "Senzu Village")) {
            return (11600);
        } else if (!strcasecmp(arg, "Guru's House")) {
            return (10182);
        } else if (!strcasecmp(arg, "Crystalline Cave")) {
            return (10474);
        } else if (!strcasecmp(arg, "Elder Village")) {
            return (13300);
        } else if (!strcasecmp(arg, "Frieza's Ship")) {
            return (10203);
        } else if (!strcasecmp(arg, "Kakureta Village")) {
            return (10922);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (11600);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 55) { // Above Aether
        if (!strcasecmp(arg, "Haven City")) {
            return (12010);
        } else if (!strcasecmp(arg, "Serenity Lake")) {
            return (12103);
        } else if (!strcasecmp(arg, "Kaiju Forest")) {
            return (12300);
        } else if (!strcasecmp(arg, "Ortusian Temple")) {
            return (12400);
        } else if (!strcasecmp(arg, "Silent Glade")) {
            return (12480);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (12010);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 56) { // Above Yardrat
        if (!strcasecmp(arg, "Yardra City")) {
            return (14008);
        } else if (!strcasecmp(arg, "Jade Forest")) {
            return (14100);
        } else if (!strcasecmp(arg, "Jade Cliffs")) {
            return (14200);
        } else if (!strcasecmp(arg, "Mount Valaria")) {
            return (14300);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (14008);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 198) { // Above Cerria
        if (!strcasecmp(arg, "Cerria Colony")) {
            return (17531);
        } else if (!strcasecmp(arg, "Crystalline Forest")) {
            return (7950);
        } else if (!strcasecmp(arg, "Fistarl Volcano")) {
            return (17420);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (17531);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 57) { // Above Zennith
        if (!strcasecmp(arg, "Utatlan City")) {
            return (3412);
        } else if (!strcasecmp(arg, "Zenith Jungle")) {
            return (3520);
        } else if (!strcasecmp(arg, "Ancient Castle")) {
            return (19600);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (3412);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 58) { // Above Kanassa
        if (!strcasecmp(arg, "Aquis City")) {
            return (14904);
        } else if (!strcasecmp(arg, "Yunkai Pirate Base")) {
            return (15655);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (14904);
        }
    } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 59) { // Above Arlia
        if (!strcasecmp(arg, "Janacre")) {
            return (16009);
        } else if (!strcasecmp(arg, "Arlian Wasteland")) {
            return (16544);
        } else if (!strcasecmp(arg, "Arlia Mine")) {
            return (16600);
        } else if (!strcasecmp(arg, "Kemabra Wastes")) {
            return (16816);
        } else {
            send_to_char(ch, "you don't know where that made up place is, but decided to land anyway.");
            return (16009);
        }
    } else {
        send_to_char(ch, "You are not above a planet!\r\n");
        return (-1);
    }
}

struct obj_data *find_vehicle_by_vnum(int vnum) {
    auto o = get_last_inserted(objectVnumIndex, vnum);
    if(o && GET_OBJ_TYPE(o) == ITEM_VEHICLE) return o;
    return nullptr;
}

struct obj_data *find_hatch_by_vnum(int vnum) {
    auto o = get_last_inserted(objectVnumIndex, vnum);
    if(o && GET_OBJ_TYPE(o) == ITEM_HATCH) return o;
    return nullptr;
}

/* Search the given list for an object type, and return a ptr to that obj*/
struct obj_data *get_obj_in_list_type(int type, struct obj_data *list) {
    struct obj_data *i;

    for (i = list; i; i = i->next_content)
        if (GET_OBJ_TYPE(i) == type)
            return i;

    return nullptr;
}

/* Search the player's room, inventory and equipment for a control */
struct obj_data *find_control(struct char_data *ch) {
    struct obj_data *controls, *obj;
    int j;

    controls = get_obj_in_list_type(ITEM_CONTROL, ch->getRoom()->contents);
    if (!controls)
        for (obj = ch->contents; obj && !controls; obj = obj->next_content)
            if (CAN_SEE_OBJ(ch, obj) && GET_OBJ_TYPE(obj) == ITEM_CONTROL)
                controls = obj;
    if (!controls)
        for (j = 0; j < NUM_WEARS && !controls; j++)
            if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)) &&
                GET_OBJ_TYPE(GET_EQ(ch, j)) == ITEM_CONTROL)
                controls = GET_EQ(ch, j);
    return controls;
}

/* Drive our vehicle into another vehicle */
static void drive_into_vehicle(struct char_data *ch, struct obj_data *vehicle, char *arg) {
    struct obj_data *vehicle_in_out;
    int was_in, is_in, is_going_to;
    char buf[MAX_INPUT_LENGTH];

    if (!*arg) {
        send_to_char(ch, "@wDrive into what?\r\n");
        return;
    }
    if (!(vehicle_in_out = get_obj_in_list_vis(ch, arg, nullptr, vehicle->getRoom()->contents))) {
        send_to_char(ch, "@wNothing here by that name!\r\n");
        return;
    }

    if (GET_OBJ_TYPE(vehicle_in_out) != ITEM_VEHICLE) {
        send_to_char(ch, "@wThat's not a ship.\r\n");
        return;
    }

    if (vehicle == vehicle_in_out) {
        send_to_char(ch, "@wMy, we are in a clever mood today, aren't we.\r\n");
        return;
    }

    is_going_to = real_room(GET_OBJ_VAL(vehicle_in_out, 0));
    if (!ROOM_FLAGS(is_going_to).test(ROOM_VEHICLE)) {
        send_to_char(ch, "@wThat ship can't carry other ships.");
        return;
    }

    sprintf(buf, "%s @wenters %s.\r\n", vehicle->short_description,
            vehicle_in_out->short_description);
    send_to_room(IN_ROOM(vehicle), buf);

    was_in = IN_ROOM(vehicle);
    obj_from_room(vehicle);
    obj_to_room(vehicle, is_going_to);
    is_in = IN_ROOM(vehicle);
    if (ch->desc != nullptr)
        act("", true, ch, nullptr, nullptr, TO_ROOM);
    send_to_char(ch, "@wThe ship flies onward:\r\n");
    look_at_room(IN_ROOM(vehicle), ch, 0);
    sprintf(buf, "%s @wenters.\r\n", vehicle->short_description);
    send_to_room(is_in, buf);

}

/* Drive our vehicle out of another vehicle */
static void drive_outof_vehicle(struct char_data *ch, struct obj_data *vehicle) {
    struct obj_data *hatch, *vehicle_in_out;
    char buf[MAX_INPUT_LENGTH];

    auto room = vehicle->getRoom();

    if (!(hatch = get_obj_in_list_type(ITEM_HATCH, room->contents))) {
        send_to_char(ch, "@wNowhere to pilot out of.\r\n");
        return;
    }

    if (!(vehicle_in_out = find_vehicle_by_vnum(GET_OBJ_VAL(hatch, 0)))) {
        send_to_char(ch, "@wYou can't pilot out anywhere!\r\n");
        return;
    }

    sprintf(buf, "%s @wexits %s.\r\n", vehicle->short_description,
            vehicle_in_out->short_description);
    send_to_room(room->vn, buf);

    obj_from_room(vehicle);
    obj_to_room(vehicle, IN_ROOM(vehicle_in_out));

    room = vehicle->getRoom();

    if (ch->desc != nullptr)
        act("@wThe @De@Wn@wg@Di@wn@We@Ds@w of the ship @rr@Ro@ra@Rr@w as it moves.", true, ch, nullptr, nullptr,
            TO_ROOM);
    send_to_char(ch, "@wThe ship flies onward:\r\n");
    look_at_room(IN_ROOM(vehicle), ch, 0);
    int door;
    for (door = 0; door < NUM_OF_DIRS; door++) {
        auto e = room->dir_option[door];
        if(!e) continue;
        auto dest = e->getDestination();
        if(!dest) continue;
        if(!IS_SET(e->exit_info, EX_CLOSED)) continue;

        send_to_room(dest->vn, "@wThe @De@Wn@wg@Di@wn@We@Ds@w of the ship @rr@Ro@ra@Rr@w as it moves.\r\n");
    }
    sprintf(buf, "%s @wflies out of %s.\r\n", vehicle->short_description,
            vehicle_in_out->short_description);
    send_to_room(IN_ROOM(vehicle), buf);

}

/* Drive out vehicle in a certain direction */
void drive_in_direction(struct char_data *ch, struct obj_data *vehicle, int dir) {
    char buf[MAX_INPUT_LENGTH];
    auto room = vehicle->getRoom();

    auto d = room->dir_option[dir];
    if(!d) {
        send_to_char(ch, "@wApparently %s doesn't exist there.\r\n", dirs[dir]);
        return;
    }
    auto dest = d->getDestination();
    if(!dest) {
        send_to_char(ch, "@wApparently %s doesn't exist there.\r\n", dirs[dir]);
        return;
    }

    if(IS_SET(d->exit_info, EX_CLOSED)) {
        if (d->keyword)
            send_to_char(ch, "@wThe %s seems to be closed.\r\n", fname(d->keyword));
        else
            send_to_char(ch, "@wIt seems to be closed.\r\n");
        return;
    }

    if (!dest->room_flags.test(ROOM_VEHICLE) && !dest->room_flags.test(ROOM_SPACE)) {
        /* But the vehicle can't go that way*/
        send_to_char(ch, "@wThe ship can't fit there!\r\n");
        return;
    }

    int was_in, is_in;

    sprintf(buf, "%s @wflies %s.\r\n", vehicle->short_description, dirs[dir]);
    send_to_room(IN_ROOM(vehicle), buf);

    obj_from_room(vehicle);
    obj_to_room(vehicle, dest->vn);
    struct obj_data *controls;
    if ((controls = find_control(ch))) {
        if (GET_FUELCOUNT(controls) < 5) {
            GET_FUELCOUNT(controls) += 1;
        } else {
            GET_FUELCOUNT(controls) = 0;
            GET_FUEL(controls) -= 1;
            if (GET_FUEL(controls) < 0) {
                GET_FUEL(controls) = 0;
            }
        }
    }

    struct obj_data *hatch = nullptr;

    for (hatch = world[real_room(GET_OBJ_VAL(vehicle, 0))].contents; hatch; hatch = hatch->next_content) {
        if (GET_OBJ_TYPE(hatch) == ITEM_HATCH) {
            GET_OBJ_VAL(hatch, 3) = GET_ROOM_VNUM(IN_ROOM(vehicle));
        }
    }

    is_in = IN_ROOM(vehicle);

    if (ch->desc != nullptr)
        act("@wThe @De@Wn@wg@Di@wn@We@Ds@w of the ship @rr@Ro@ra@Rr@w as it moves.", true, ch, nullptr, nullptr,
            TO_ROOM);
    send_to_char(ch, "@wThe ship flies onward:\r\n");
    look_at_room(is_in, ch, 0);
    if (controls) {
        send_to_char(ch, "@RFUEL@D: %s%s@n\r\n",
                     GET_FUEL(controls) >= 200 ? "@G" : GET_FUEL(controls) >= 100 ? "@Y" : "@r",
                     add_commas(GET_FUEL(controls)).c_str());
    }
    int door;
    room = vehicle->getRoom();
    for (door = 0; door < NUM_OF_DIRS; door++) {
        auto e = room->dir_option[door];
        if(!e) continue;
        dest = e->getDestination();
        if(!dest) continue;
        if(!IS_SET(e->exit_info, EX_CLOSED)) continue;

        send_to_room(dest->vn, "@wThe @De@Wn@wg@Di@wn@We@Ds@w of the ship @rr@Ro@ra@Rr@w as it moves.\r\n");
    }
    sprintf(buf, "%s @wflies in from the %s.\r\n",
            vehicle->short_description, dirs[rev_dir[dir]]);

    send_to_room(is_in, buf);

}

ACMD(do_warp) {
    struct obj_data *vehicle, *controls;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (IS_NPC(ch))
        return;

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }
    if (!PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "@wYou need to be seated in the pilot's seat.\r\n[Enter: Pilot ready/unready]\r\n");
        return;
    }

    if (!(controls = find_control(ch))) {
        send_to_char(ch, "@wYou have nothing to control here!\r\n");
        return;
    }
    if (!(vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(controls, 0)))) {
        send_to_char(ch, "@wYou can't find anything to pilot.\r\n");
        return;
    } else if (!*arg) {
        send_to_char(ch,
                     "Syntax: shipwarp [ earth | vegeta | namek | konack | aether | frigid | buoy1 | buoy2 | buoy3 ]\r\n");
        return;
    } else if (strcasecmp(arg, "earth") && strcasecmp(arg, "vegeta") && strcasecmp(arg, "namek") &&
               strcasecmp(arg, "konack") && strcasecmp(arg, "frigid") && strcasecmp(arg, "aether") &&
               strcasecmp(arg, "buoy1") && strcasecmp(arg, "buoy2") && strcasecmp(arg, "buoy3")) {
        send_to_char(ch,
                     "Syntax: shipwarp [ earth | vegeta | namek | konack | aether | frigid | buoy1 | buoy2 | buoy3 ]\r\n");
        return;
    } else if (ROOM_FLAGGED(!IN_ROOM(vehicle), ROOM_SPACE)) {
        send_to_char(ch, "Your ship needs to be in space to utilize its Instant Travel Warp Accelerator.\r\n");
        return;
    } else if (GET_OBJ_VNUM(vehicle) != 18400) {
        send_to_char(ch, "Your ship is not outfitted with an Instant Travel Warp Accelerator.\r\n");
        return;
    } else {
        if (!strcasecmp(arg, "earth")) {
            if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 40979 || GET_ROOM_VNUM(IN_ROOM(vehicle)) == 50) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(40979));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "namek")) {
            if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 42880 || GET_ROOM_VNUM(IN_ROOM(vehicle)) == 54) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(42880));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "frigid")) {
            if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 30889 || GET_ROOM_VNUM(IN_ROOM(vehicle)) == 51) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(30889));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "konack")) {
            if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 27065 || GET_ROOM_VNUM(IN_ROOM(vehicle)) == 52) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(27065));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "vegeta")) {
            if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 32365 || GET_ROOM_VNUM(IN_ROOM(vehicle)) == 53) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(32365));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "aether")) {
            if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == 41959 || GET_ROOM_VNUM(IN_ROOM(vehicle)) == 55) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(41959));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "buoy1")) {
            if (GET_RADAR1(ch) <= 0) {
                send_to_char(ch, "You have not launched that buoy!\r\n");
                return;
            } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == GET_RADAR1(ch)) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(GET_RADAR1(ch)));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "buoy2")) {
            if (GET_RADAR2(ch) <= 0) {
                send_to_char(ch, "You have not launched that buoy!\r\n");
                return;
            } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == GET_RADAR2(ch)) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(GET_RADAR2(ch)));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else if (!strcasecmp(arg, "buoy3")) {
            if (GET_RADAR3(ch) <= 0) {
                send_to_char(ch, "You have not launched that buoy!\r\n");
                return;
            } else if (GET_ROOM_VNUM(IN_ROOM(vehicle)) == GET_RADAR3(ch)) {
                send_to_char(ch, "Your ship is already there!\r\n");
                return;
            } else {
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find your ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_CHAR);
                act("@BA glow of blue light floods in through the window for an instant. You feel a strange shift as the light disappears and you find the ship in a new location!@n",
                    true, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle),
                             "%s @Bbegins to glow bright blue before disappearing in a flash of light!@n\r\n",
                             vehicle->short_description);
                obj_from_room(vehicle);
                obj_to_room(vehicle, real_room(GET_RADAR3(ch)));
                send_to_room(IN_ROOM(vehicle), "@BSuddenly in a flash of blue light @n%s @B appears instantly!@n\r\n",
                             vehicle->short_description);
            }
        } else {
            basic_mud_log("ERROR: Ship Instant Warp Failure! Unknown argument!");
            send_to_char(ch, "ERROR\r\n");
            return;
        }
    }
}

ACMD(do_drive) {
    int dir, confirmed = false, count = 0;
    struct obj_data *vehicle, *controls;
    char arg3[MAX_INPUT_LENGTH];

    one_argument(argument, arg3);

    if (!HAS_ARMS(ch)) {
        send_to_char(ch, "You have no arms!\r\n");
        return;
    }

    if (!strcasecmp(arg3, "unready") && !IS_NPC(ch)) {
        if (!PLR_FLAGGED(ch, PLR_PILOTING)) {
            send_to_char(ch, "You are already not flying the ship!\r\n");
            return;
        } else if (PLR_FLAGGED(ch, PLR_PILOTING)) {
            act("@w$n stands up and stops piloting the ship.", true, ch, nullptr, nullptr, TO_ROOM);
            send_to_char(ch, "@wYou stand up from the pilot's seat.\r\n");
            GET_POS(ch) = POS_STANDING;
            ch->playerFlags.reset(PLR_PILOTING);
            return;
        }
    }
    if (!strcasecmp(arg3, "ready") && !IS_NPC(ch)) {
        if (!(controls = find_control(ch))) {
            send_to_char(ch, "@wYou have nothing to control here!\r\n");
            return;
        }
        struct descriptor_data *d;
        if (PLR_FLAGGED(ch, PLR_PILOTING)) {
            send_to_char(ch, "@wYou are already piloting the ship, try [pilot unready].\r\n");
            return;
        }
        if (CARRYING(ch)) {
            send_to_char(ch, "@wYou are busy carrying someone.\r\n");
            return;
        }
        if (DRAGGING(ch)) {
            send_to_char(ch, "@wYou are busy dragging someone.\r\n");
            return;
        }
        for (d = descriptor_list; d; d = d->next) {
            if (!IS_PLAYING(d)) {
                continue;
            }
            if (d->character == ch) {
                continue;
            }
            if (PLR_FLAGGED(d->character, PLR_PILOTING) && IN_ROOM(d->character) == IN_ROOM(ch)) {
                send_to_char(ch, "@w%s is already piloting the ship!\r\n", GET_NAME(d->character));
                count = 1;
                return;
            }
        }
        if (count == 0) {
            confirmed = true;
        }
    }
    if (confirmed == true) {
        ch->playerFlags.set(PLR_PILOTING);
        act("@w$n sits down and begins piloting the ship.", true, ch, nullptr, nullptr, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        send_to_char(ch, "@wYou take a seat in the pilot's chair.\r\n");
        return;
    } else if (!PLR_FLAGGED(ch, PLR_PILOTING)) {
        send_to_char(ch, "@wYou need to be seated in the pilot's seat.\r\n[Enter: Pilot ready/unready]\r\n");
    } else if (GET_POS(ch) < POS_SLEEPING) {
        send_to_char(ch, "@wYou can't see anything but stars!\r\n");
    } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char(ch, "@wYou can't see a damned thing, you're blind!\r\n");
    } else if (IS_DARK(IN_ROOM(ch)) && !CAN_SEE_IN_DARK(ch)) {
        send_to_char(ch, "@wIt is pitch black...\r\n");
    } else if (!(controls = find_control(ch))) {
        send_to_char(ch, "@wYou have nothing to control here!\r\n");
    } else if (invalid_align(ch, controls) ||
               invalid_class(ch, controls) ||
               invalid_race(ch, controls)) {
        act("@wYou are zapped by $p@w and instantly step away from it.", false, ch, controls, nullptr, TO_CHAR);
        act("@w$n@w is zapped by $p@w and instantly steps away from it.", false, ch, controls, nullptr, TO_ROOM);
    } else if (!(vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(controls, 0)))) {
        send_to_char(ch, "@wYou can't find anything to pilot.\r\n");
    } else {

        char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

        /*argument = any_one_arg(argument, arg);*/
        half_chop(argument, arg, arg2);

        if (GET_FUEL(controls) <= 0) {
            send_to_char(ch, "Your ship doesn't have enough fuel to move.\r\n");
            return;
        }

        if (!*arg) {
            send_to_char(ch, "@wPilot, yes, but where?\r\n");
        } else if (is_abbrev(arg, "into") ||
                   is_abbrev(arg, "onto")) {
            /* Driving into another vehicle */
            drive_into_vehicle(ch, vehicle, arg2);
        } else if (is_abbrev(arg, "out") && !EXIT(vehicle, OUTDIR)) {
            drive_outof_vehicle(ch, vehicle);
        } else {
            if (!OBJVAL_FLAGGED(vehicle, CONT_CLOSED)) {
                send_to_char(ch, "@wThe hatch is open, are you insane!?\r\n");
                return;
            }

            if (!strcasecmp(arg, "north") || !strcasecmp(arg, "n")) {
                dir = 0;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "east") || !strcasecmp(arg, "e")) {
                dir = 1;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "south") || !strcasecmp(arg, "s")) {
                dir = 2;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "west") || !strcasecmp(arg, "w")) {
                dir = 3;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "up") || !strcasecmp(arg, "u")) {
                dir = 4;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "down") || !strcasecmp(arg, "d")) {
                dir = 5;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "northwest") || !strcasecmp(arg, "nw") || !strcasecmp(arg, "northw")) {
                dir = 6;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "northeast") || !strcasecmp(arg, "ne") || !strcasecmp(arg, "northe")) {
                dir = 7;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "southeast") || !strcasecmp(arg, "se") || !strcasecmp(arg, "southe")) {
                dir = 8;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "southwest") || !strcasecmp(arg, "sw") || !strcasecmp(arg, "southw")) {
                dir = 9;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "inside")) {
                dir = 10;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            } else if (!strcasecmp(arg, "outside")) {
                dir = 11;
                drive_in_direction(ch, vehicle, dir);
                if (GET_OBJ_VAL(controls, 1) == 1) {
                    WAIT_STATE(ch, PULSE_2SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 2) {
                    WAIT_STATE(ch, PULSE_2SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 3) {
                    WAIT_STATE(ch, PULSE_1SEC * 1.5);
                } else if (GET_OBJ_VAL(controls, 1) == 4) {
                    WAIT_STATE(ch, PULSE_1SEC);
                } else if (GET_OBJ_VAL(controls, 1) == 5) {
                    WAIT_STATE(ch, PULSE_1SEC * 0.5);
                }
            }
                /*else if (dir = search_block(arg, dirs, FALSE) >= 0) {*/
                /* Drive in a direction... */
                /*}*/
            else if (!strcasecmp(arg, "land")) {

                if (!*arg2) {
                    if (GET_OBJ_VNUM(vehicle) >= 46000 && GET_OBJ_VNUM(vehicle) <= 46099) {
                        send_to_char(ch, "@wLand on which pad? 1, 2, 3 or 4?\r\n");
                        send_to_char(ch,
                                     "@CSpecial Ship Ability@D: @wpilot land (area name)\n@GExample@D: @wpilot land Nexus City\r\n");
                        disp_ship_locations(ch, vehicle);
                    } else {
                        send_to_char(ch, "@wLand on which pad? 1, 2, 3 or 4?\r\n");
                    }
                    return;
                }
                char blah[MAX_INPUT_LENGTH];
                int land_location = 50;
                if (GET_OBJ_VNUM(vehicle) > 46099) {
                    if (strcasecmp(arg2, "1") && strcasecmp(arg2, "2") && strcasecmp(arg2, "3") &&
                        strcasecmp(arg2, "4")) {
                        send_to_char(ch, "@wLand on which pad? 1, 2, 3 or 4?\r\n");
                        return;
                    }
                } else if (strcasecmp(arg2, "1") && strcasecmp(arg2, "2") && strcasecmp(arg2, "3") &&
                           strcasecmp(arg2, "4")) {
                    land_location = ship_land_location(ch, vehicle, arg2);
                }
                char buf3[MAX_INPUT_LENGTH];
                *buf3 = '\0';

                /* For Earth */
                act("@wYou set the controls to descend.@n", false, ch, nullptr, nullptr, TO_CHAR);
                act("@C$n @wmanipulates the ship controls.@n", false, ch, nullptr, nullptr, TO_ROOM);
                act("@RThe ship rocks and shakes as it descends through the atmosphere!@n", false, ch, nullptr, nullptr,
                    TO_CHAR);
                act("@RThe ship rocks and shakes as it descends through the atmosphere!@n", false, ch, nullptr, nullptr,
                    TO_ROOM);
                if (land_location <= 50) {
                    act("@wThe ship has landed.@n", false, ch, nullptr, nullptr, TO_CHAR);
                    act("@wThe ship has landed.@n", false, ch, nullptr, nullptr, TO_ROOM);
                }
                if (land_location > 50) {
                    act("@wThe ship slams into the ground and forms a small crater!@n", false, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@wThe ship slams into the ground and forms a small crater!@n", false, ch, nullptr, nullptr,
                        TO_ROOM);
                    obj_from_room(vehicle);
                    obj_to_room(vehicle, real_room(land_location));
                } else if (IN_ROOM(vehicle) == real_room(50)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(409));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(411));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(412));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(410));
                    } else if (!strcasecmp(arg2, "4365")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(18904));
                    } else if (!strcasecmp(arg2, "6329")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(18925));
                    } else if (!strcasecmp(arg2, "1983")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(18995));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }
                    /* For Zenith */
                else if (IN_ROOM(vehicle) == real_room(57)) {
                    obj_from_room(vehicle);
                    obj_to_room(vehicle, real_room(3508));
                }
                    /* For Cerria*/
                else if (IN_ROOM(vehicle) == real_room(198)) {
                    obj_from_room(vehicle);
                    obj_to_room(vehicle, real_room(17420));
                }
                    /* For Kanassa */
                else if (IN_ROOM(vehicle) == real_room(58)) {
                    obj_from_room(vehicle);
                    obj_to_room(vehicle, real_room(14904));
                }
                    /* For Vegeta */
                else if (IN_ROOM(vehicle) == real_room(53)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(2319));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(2318));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(2320));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(2322));
                    } else if (!strcasecmp(arg2, "4126")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(18212));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }

                    /* For Yardrat */
                else if (IN_ROOM(vehicle) == real_room(56)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(14003));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(14004));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(14005));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(14006));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }

                    /* For Aether */
                else if (IN_ROOM(vehicle) == real_room(55)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(12003));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(12004));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(12006));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(12005));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }

                    /* For Arlia */
                else if (IN_ROOM(vehicle) == real_room(59)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(16065));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(16066));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(16067));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(16068));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }


                    /* For Frigid */
                else if (IN_ROOM(vehicle) == real_room(51)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(4264));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(4263));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(4261));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(4262));
                    } else if (!strcasecmp(arg2, "1337")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(18116));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }
                    /* For Namek */
                else if (IN_ROOM(vehicle) == real_room(54)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(11628));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(11629));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(11630));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(11627));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                }
                    /* For Konack */
                else if (IN_ROOM(vehicle) == real_room(52)) {
                    if (!strcasecmp(arg2, "1")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(8195));
                    } else if (!strcasecmp(arg2, "2")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(8196));
                    } else if (!strcasecmp(arg2, "3")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(8197));
                    } else if (!strcasecmp(arg2, "4")) {
                        obj_from_room(vehicle);
                        obj_to_room(vehicle, real_room(8198));
                    } else {
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_CHAR);
                        act("@wLanding sequence aborted, improper coordinates.@n", false, ch, nullptr, nullptr,
                            TO_ROOM);
                    }
                } else {
                    send_to_char(ch, "@wYou are not where you can land, you need to be in a planet's low orbit.@n\r\n");
                }
                if (land_location <= 50) {
                    sprintf(buf3, "%s @wcomes in from above and slowly settles on the launch-pad.@n\r\n",
                            vehicle->short_description);
                    look_at_room(IN_ROOM(vehicle), ch, 0);
                    send_to_room(IN_ROOM(vehicle), buf3);
                } else {
                    sprintf(buf3, "%s @wcomes in from above and slams into the ground!@n\r\n",
                            vehicle->short_description);
                    ROOM_DAMAGE(IN_ROOM(vehicle)) += 1;
                    if (ROOM_DAMAGE(IN_ROOM(vehicle)) >= 10) {
                        ROOM_DAMAGE(IN_ROOM(vehicle)) = 10;
                    }
                    look_at_room(IN_ROOM(vehicle), ch, 0);
                    send_to_room(IN_ROOM(vehicle), buf3);
                }
            } else if (!strcasecmp(arg, "launch")) {
                int lnum = 0;
                int rnum = 0;
                auto room = vehicle->getRoom();
                auto dest = room->getLaunchDestination();
                if(!dest) {
                    send_to_char(ch, "@wYou are not on a planet.@n\r\n");
                    return;
                }
                rnum = dest.value();
                act("@wYou set the controls to launch.@n", false, ch, nullptr, nullptr, TO_CHAR);
                act("@C$n @wmanipulates the ship controls.@n", false, ch, nullptr, nullptr, TO_ROOM);
                act("@RThe ship shudders as it launches up into the sky!@n", false, ch, nullptr, nullptr, TO_CHAR);
                act("@RThe ship shudders as it launches up into the sky!@n", false, ch, nullptr, nullptr, TO_ROOM);
                act("@wThe ship has reached low orbit.@n", false, ch, nullptr, nullptr, TO_CHAR);
                act("@wThe ship has reached low orbit.@n", false, ch, nullptr, nullptr, TO_ROOM);
                send_to_room(IN_ROOM(vehicle), "@R%s @Rshudders before blasting off into the sky!@n",
                             vehicle->short_description);
                if (GET_FUELCOUNT(controls) < 5) {
                    GET_FUELCOUNT(controls) += 1;
                } else {
                    GET_FUELCOUNT(controls) = 0;
                    GET_FUEL(controls) -= 1;
                    if (GET_FUEL(controls) < 0) {
                        GET_FUEL(controls) = 0;
                    }
                }
                obj_from_room(vehicle);
                obj_to_room(vehicle, rnum);
                look_at_room(IN_ROOM(vehicle), ch, 0);
                send_to_char(ch, "@RFUEL@D: %s%s@n\r\n",
                             GET_FUEL(controls) >= 200 ? "@G" : GET_FUEL(controls) >= 100 ? "@Y" : "@r",
                             add_commas(GET_FUEL(controls)).c_str());
            } else if (!strcasecmp(arg, "mark")) {
                int rnum = 0;
                if (!*arg2) {
                    send_to_char(ch, "@wWhich marker are you wanting to launch? 1, 2, or 3?\r\n");
                    return;
                }
                if (!!strcasecmp(arg2, "1") && !!strcasecmp(arg2, "2") && !!strcasecmp(arg2, "3")) {
                    send_to_char(ch, "@wWhich marker are you wanting to launch? 1, 2, or 3?\r\n");
                    return;
                }

                if (!ROOM_FLAGGED(IN_ROOM(vehicle), ROOM_SPACE)) {
                    send_to_char(ch, "@wYou need to be in space to launch a marker buoy.\r\n");
                    return;
                }

                rnum = IN_ROOM(vehicle);

                if (GET_RADAR1(ch) > 0 && !strcasecmp(arg2, "1")) {
                    send_to_char(ch, "@wYou need to 'deactivate' that marker.\r\n");
                    return;
                } else if (GET_RADAR1(ch) <= 0 && !strcasecmp(arg2, "1")) {
                    act("@wYou enter a unique code and launch a marker buoy.@n\r\n", false, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@w manipulates the ship controls.@n\r\n", false, ch, nullptr, nullptr, TO_ROOM);
                    GET_RADAR1(ch) = GET_ROOM_VNUM(IN_ROOM(vehicle));
                }
                if (GET_RADAR2(ch) > 0 && !strcasecmp(arg2, "2")) {
                    send_to_char(ch, "@wYou need to 'deactivate' that marker.\r\n");
                    return;
                } else if (GET_RADAR2(ch) <= 0 && !strcasecmp(arg2, "2")) {
                    act("@wYou enter a unique code and launch a marker buoy.@n\r\n", false, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@w manipulates the ship controls.@n\r\n", false, ch, nullptr, nullptr, TO_ROOM);
                    GET_RADAR2(ch) = GET_ROOM_VNUM(IN_ROOM(vehicle));
                }
                if (GET_RADAR3(ch) > 0 && !strcasecmp(arg2, "3")) {
                    send_to_char(ch, "@wYou need to 'deactivate' that marker.\r\n");
                    return;
                } else if (GET_RADAR3(ch) <= 0 && !strcasecmp(arg2, "3")) {
                    act("@wYou enter a unique code and launch a marker buoy.@n\r\n", false, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@w manipulates the ship controls.@n\r\n", false, ch, nullptr, nullptr, TO_ROOM);
                    GET_RADAR3(ch) = GET_ROOM_VNUM(IN_ROOM(vehicle));
                }
            } else if (!strcasecmp(arg, "deactivate")) {
                if (!*arg2) {
                    send_to_char(ch, "@wWhich marker are you wanting to launch? 1, 2, or 3?\r\n");
                    return;
                }
                if (!!strcasecmp(arg2, "1") && !!strcasecmp(arg2, "2") && !!strcasecmp(arg2, "3")) {
                    send_to_char(ch, "@wWhich marker are you wanting to deactivate? 1, 2, or 3?\r\n");
                    return;
                }

                if (GET_RADAR1(ch) <= 0 && !strcasecmp(arg2, "1")) {
                    send_to_char(ch, "@wYou haven't launched that buoy yet.\r\n");
                    return;
                } else if (GET_RADAR1(ch) > 0 && !strcasecmp(arg2, "1")) {
                    act("@wYou enter buoy one's code and command it to deactivate.@n\r\n", false, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@w manipulates the ship controls.@n\r\n", false, ch, nullptr, nullptr, TO_ROOM);
                    GET_RADAR1(ch) = 0;
                }
                if (GET_RADAR2(ch) <= 0 && !strcasecmp(arg2, "2")) {
                    send_to_char(ch, "@wYou haven't launched that buoy yet.\r\n");
                    return;
                } else if (GET_RADAR2(ch) > 0 && !strcasecmp(arg2, "2")) {
                    act("@wYou enter buoy two's code and command it to deactivate.@n\r\n", false, ch, nullptr, nullptr,
                        TO_CHAR);
                    act("@C$n@w manipulates the ship controls.@n\r\n", false, ch, nullptr, nullptr, TO_ROOM);
                    GET_RADAR2(ch) = 0;
                }
                if (GET_RADAR3(ch) <= 0 && !strcasecmp(arg2, "3")) {
                    send_to_char(ch, "@wYou haven't launched that buoy yet.\r\n");
                    return;
                } else if (GET_RADAR3(ch) > 0 && !strcasecmp(arg2, "3")) {
                    act("@wYou enter buoy three's code and command it to deactivate.@n\r\n", false, ch, nullptr,
                        nullptr, TO_CHAR);
                    act("@C$n@w manipulates the ship controls.@n\r\n", false, ch, nullptr, nullptr, TO_ROOM);
                    GET_RADAR3(ch) = 0;
                }
            } else {
                send_to_char(ch, "@wThats not a valid direction.\r\n");
                send_to_char(ch, "Try one of these.\r\n");
                send_to_char(ch, "[ north/n  | south/s  | east/e  |  west/w  ]\r\n");
                send_to_char(ch, "[ up/u | down/d | northeast/ne/northe | northwest/nw/northw]\r\n");
                send_to_char(ch, "[  southeast/se/southe  |  southwest/sw/southw]\r\n");
                send_to_char(ch, "[  into  |  onto  |  inside  |  outside  ]@n\r\n");
                send_to_char(ch, "[ land | launch ]@n\r\n");
            }
        }
    }
}

ACMD(do_ship_fire) {
    struct obj_data *vehicle, *controls;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(argument, arg1, arg2);

    if (!(controls = find_control(ch))) {
        send_to_char(ch, "@wYou must be near the comm station in the cockpit.\r\n");
        return;
    }

    if (!(vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(controls, 0)))) {
        send_to_char(ch, "@wSomething cosmic is jamming your signal! Quick call Iovan to repair it!\r\n");
        return;
    }

    struct obj_data *obj = nullptr, *obj2 = nullptr, *next_obj = nullptr;
    int shot = false;

    for (obj = ch->getRoom()->contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (shot == false) {
            if (GET_OBJ_TYPE(obj) == ITEM_VEHICLE && obj != vehicle) {
                if (!strcasecmp(arg1, obj->name)) {
                    obj2 = obj;
                    shot = true;
                }
            }
        }
    }
}
