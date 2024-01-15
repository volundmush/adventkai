/******************************************************************************/
/** OasisOLC - InGame OLC Listings                                     v2.0  **/
/** Original author: Levork                                                  **/
/** Copyright 1996 Harvey Gilpin                                             **/
/** Copyright 1997-2001 George Greer (greerga@circlemud.org)                 **/
/** Copyright 2002 Kip Potter [Mythran] (kip_potter@hotmail.com)             **/
/******************************************************************************/

#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/db.h"
#include "dbat/oasis.h"
#include "dbat/shop.h"
#include "dbat/constants.h"
#include "dbat/dg_scripts.h"
#include "dbat/guild.h"
#include "dbat/races.h"
#include "dbat/class.h"


/******************************************************************************/
/** Internal Functions                                                       **/
/******************************************************************************/
void list_rooms(struct char_data *ch, zone_rnum rnum, room_vnum vmin, room_vnum vmax);

void list_mobiles(struct char_data *ch, zone_rnum rnum, mob_vnum vmin, mob_vnum vmax);

void list_objects(struct char_data *ch, zone_rnum rnum, obj_vnum vmin, obj_vnum vmax);

void list_shops(struct char_data *ch, zone_rnum rnum, shop_vnum vmin, shop_vnum vmax);

void list_triggers(struct char_data *ch, zone_rnum rnum, trig_vnum vmin, trig_vnum vmax);

void list_zones(struct char_data *ch);

void print_zone(struct char_data *ch, zone_vnum vnum);

void list_guilds(struct char_data *ch, zone_rnum rnum, guild_vnum vmin, guild_vnum vmax);


/******************************************************************************/
/** Ingame Commands                                                          **/
/******************************************************************************/
ACMD(do_oasis_list) {
    zone_rnum rzone = NOWHERE;
    room_rnum vmin = NOWHERE;
    room_rnum vmax = NOWHERE;
    char smin[MAX_INPUT_LENGTH];
    char smax[MAX_INPUT_LENGTH];

    two_arguments(argument, smin, smax);

    if (subcmd == SCMD_OASIS_ZLIST) { /* special case */
        if (smin != nullptr && *smin && is_number(smin))
            print_zone(ch, atoi(smin));
        else
            list_zones(ch);
        return;
    }

    if (!*smin || *smin == '.') {
        rzone = ch->getRoom()->zone;
    } else if (!*smax) {
        rzone = real_zone(atoi(smin));

        if (rzone == NOWHERE) {
            send_to_char(ch, "Sorry, there's no zone with that number\r\n");
            return;
        }
    } else {
        /** Listing by min vnum / max vnum.  Retrieve the numeric values.          **/
        vmin = atoi(smin);
        vmax = atoi(smax);

        if (vmin + 1500 < vmax) {
            send_to_char(ch, "Really? Over 1500?! You need to view that many at once? Come on...\r\n");
            return;
        }
        if (vmin > vmax) {
            send_to_char(ch, "List from %d to %d - Aren't we funny today!\r\n", vmin, vmax);
            return;
        }
    }

    switch (subcmd) {
        case SCMD_OASIS_RLIST:
            list_rooms(ch, rzone, vmin, vmax);
            break;
        case SCMD_OASIS_OLIST:
            list_objects(ch, rzone, vmin, vmax);
            break;
        case SCMD_OASIS_MLIST:
            list_mobiles(ch, rzone, vmin, vmax);
            break;
        case SCMD_OASIS_TLIST:
            list_triggers(ch, rzone, vmin, vmax);
            break;
        case SCMD_OASIS_SLIST:
            list_shops(ch, rzone, vmin, vmax);
            break;
        case SCMD_OASIS_GLIST:
            list_guilds(ch, rzone, vmin, vmax);
            break;
        default:
            send_to_char(ch, "You can't list that!\r\n");
            mudlog(BRF, ADMLVL_IMMORT, true,
                   "SYSERR: do_oasis_list: Unknown list option: %d", subcmd);
    }
}

ACMD(do_oasis_links) {
    zone_rnum zrnum;
    zone_vnum zvnum;
    room_rnum nr;
    int first, last, j;
    char arg[MAX_INPUT_LENGTH];

    skip_spaces(&argument);
    one_argument(argument, arg);

    if ((!arg || !*arg) || !strcmp(arg, ".")) {
        zvnum = ch->getRoom()->zone;
    } else {
        zvnum = atol(arg);
    }

    auto z = zone_table.find(zvnum);
    if (z == zone_table.end()) {
        send_to_char(ch, "No zone was found with that number.\r\n");
        return;
    }


    send_to_char(ch, "Zone %d is linked to the following zones:\r\n", z->second.number);
    for (auto nr : z->second.rooms) {
        auto r = world.find(nr);
        if (r == world.end()) {
            send_to_char(ch, "Room %d is not in the world.\r\n", nr);
            continue;
        }

        for (j = 0; j < NUM_OF_DIRS; j++) {
            auto d = r->second.dir_option[j];
            if(!d) continue;
            if(d->to_room == NOWHERE) continue;
            auto dest = d->getDestination();
            if(!dest) continue;
            if(dest->zone == zvnum) continue;

            auto z2 = zone_table.find(dest->zone);
            if (z2 == zone_table.end()) {
                send_to_char(ch, "Room %d has an invalid zone.\r\n", nr);
                continue;
            }

            send_to_char(ch, "%3d %-30s at %5d (%-5s) ---> %5d\r\n",
                         z2->first,z2->second.name,GET_ROOM_VNUM(nr), dirs[j], dest->vn);
        }
    }
}

/******************************************************************************/
/** Helper Functions                                                         **/
/******************************************************************************/


/*
 * List all rooms in a zone.                              
 */
void list_rooms(struct char_data *ch, zone_rnum rnum, zone_vnum vmin, zone_vnum vmax) {
    room_vnum i, j, bottom, top, counter = 0;
    /*
     * Expect a minimum / maximum number if the rnum for the zone is NOWHERE.
     */

    if (rnum != NOWHERE) {
        bottom = zone_table[rnum].bot;
        top = zone_table[rnum].top;
    } else {
        bottom = vmin;
        top = vmax;
    }

    send_to_char(ch,
                 "@nVNum    Room Name                                Exits\r\n"
                 "------- ---------------------------------------- -----@n\r\n");

    for (auto &[vn, r] : world) {

        /** Check to see if this room is one of the ones needed to be listed.    **/
        if ((vn >= bottom) && (vn <= top)) {
            counter++;

            auto sString = !r.proto_script.empty() ? fmt::format(" {}", r.scriptString()) : "";

            send_to_char(ch, "[@g%-5d@n] @[1]%-*s@n %s",
                         vn, count_color_chars(r.name) + 44,
                         r.name, sString.c_str());
            for (j = 0; j < NUM_OF_DIRS; j++) {
                auto d = r.dir_option[j];
                if(!d) continue;
                auto dest = d->getDestination();
                if(!dest) continue;

                if (dest->zone != r.zone)
                    send_to_char(ch, "(@y%d@n)", dest->vn);

            }

            send_to_char(ch, "\r\n");
        }
    }

    if (counter == 0) {
        send_to_char(ch, "No rooms found for zone/range specified.\r\n");
    }
}

/*
 * List all mobiles in a zone.                              
 */
void list_mobiles(struct char_data *ch, zone_rnum rnum, zone_vnum vmin, zone_vnum vmax) {
    mob_vnum i, bottom, top, counter = 0, admg;

    if (rnum != NOWHERE) {
        bottom = zone_table[rnum].bot;
        top = zone_table[rnum].top;
    } else {
        bottom = vmin;
        top = vmax;
    }

    send_to_char(ch,
                 "@nVnum    Cnt    Mobile Name                    Race      Class     Level\r\n"
                   "------- ----- -------------------------      --------- --------- -----\r\n");

    for (auto &[vn, m] : mob_proto) {
        if (vn >= bottom && vn <= top) {
            counter++;
            auto sString = !m.proto_script.empty() ? fmt::format(" {}", m.scriptString()) : "";
            send_to_char(ch, "@g%4d@n) [@g%-5d@n] @[3]%-*s @C%-9s @c%-9s @y[%4d]@n %s\r\n",
                         vn, get_vnum_count(characterVnumIndex, vn), count_color_chars(m.short_description) + 30,
                         m.short_description, TRUE_RACE(&m), sensei::getName(m.chclass).c_str(),
                         m.get(CharNum::Level),
                         sString.c_str());
        }
    }

    if (counter == 0) {
        send_to_char(ch, "None found.\r\n");
    }
}

/*
 * List all objects in a zone.                              
 */
void list_objects(struct char_data *ch, zone_rnum rnum, room_vnum vmin, room_vnum vmax) {
    obj_vnum i, bottom, top, counter = 0;

    if (rnum != NOWHERE) {
        bottom = zone_table[rnum].bot;
        top = zone_table[rnum].top;
    } else {
        bottom = vmin;
        top = vmax;
    }

    send_to_char(ch,
                 "@VNum   Cnt   Object Name                                  Object Type\r\n"
                 "------- ----- -------------------------------------------- ----------------\r\n");

    for (auto &[vn, o] : obj_proto) {
        if (vn >= bottom && vn <= top) {
            counter++;
            auto sString = !o.proto_script.empty() ? fmt::format(" {}", o.scriptString()) : "";
            send_to_char(ch, "@g%4d@n) [@g%-5d@n] @[2]%-*s @y[%s]@n%s\r\n",
                         vn, get_vnum_count(objectVnumIndex, vn), count_color_chars(o.short_description) + 44,
                         o.short_description, item_types[o.type_flag],
                         sString.c_str());
        }
    }

    if (counter == 0) {
        send_to_char(ch, "None found.\r\n");
    }
}


/*
 * List all shops in a zone.                              
 */
void list_shops(struct char_data *ch, zone_rnum rnum, shop_vnum vmin, shop_vnum vmax) {
    int i, j, bottom, top, counter = 0;

    if (rnum != NOWHERE) {
        bottom = zone_table[rnum].bot;
        top = zone_table[rnum].top;
    } else {
        bottom = vmin;
        top = vmax;
    }

    /****************************************************************************/
    /** Store the header for the shop listing.                                 **/
    /****************************************************************************/
    send_to_char(ch,
                 "Index VNum    Shop Room(s)\r\n"
                 "----- ------- ---------------------------------------------\r\n");

    for (auto &[i, sh] : shop_index) {

        if (i >= bottom && i <= top) {
            counter++;

            send_to_char(ch, "@g%4d@n) [@g%-5d@n]", counter, i);

            /************************************************************************/
            /** Retrieve the list of rooms for this shop.                          **/
            /************************************************************************/
			j = 0;
            for (auto r : sh.in_room)
                send_to_char(ch, "%s@c[@y%d@c]@n",
                             ((j > 0) && (j++ % 8 == 0)) ? "\r\n              " : " ", r);

            if (j == 0)
                send_to_char(ch, "@cNone.@n");

            send_to_char(ch, "\r\n");
        }
    }

    if (counter == 0)
        send_to_char(ch, "None found.\r\n");
}

/*
 * List all zones in the world (sort of like 'show zones').                              
 */
void list_zones(struct char_data *ch) {

    send_to_char(ch,
                 "VNum  Zone Name                      Builder(s)\r\n"
                 "----- ------------------------------ --------------------------------------\r\n");

    for (auto &z : zone_table)
        send_to_char(ch, "[@g%3d@n] @c%-*s @y%-1s@n\r\n",
                     z.first, count_color_chars(z.second.name) + 30, z.second.name,
                     z.second.builders ? z.second.builders : "None.");
}


/*
 * Prints all of the zone information for the selected zone.
 */
void print_zone(struct char_data *ch, zone_vnum vnum) {
    zone_rnum rnum;
    std::size_t size_rooms, size_objects, size_mobiles, i;
    std::size_t size_guilds, size_triggers, size_shops;
    char bits[MAX_STRING_LENGTH];

    if (!zone_table.count(vnum)) {
        send_to_char(ch, "Zone #%d does not exist in the database.\r\n", vnum);
        return;
    }
    auto &z = zone_table[vnum];
    sprintbitarray(z.zone_flags, zone_bits, ZF_ARRAY_MAX, bits);
    size_rooms = z.rooms.size();
    size_objects = z.objects.size();
    size_mobiles = z.mobiles.size();
    size_shops = z.shops.size();
    size_triggers = z.triggers.size();
    size_guilds = z.guilds.size();;


    /****************************************************************************/
    /** Display all of the zone information at once.                           **/
    /****************************************************************************/
    send_to_char(ch,
                 "@gVirtual Number = @c%d\r\n"
                 "@gName of zone   = @c%s\r\n"
                 "@gBuilders       = @c%s\r\n"
                 "@gLifespan       = @c%d\r\n"
                 "@gAge            = @c%d\r\n"
                 "@gBottom of Zone = @c%d\r\n"
                 "@gTop of Zone    = @c%d\r\n"
                 "@gReset Mode     = @c%s\r\n"
                 "@gMin Level      = @c%d\r\n"
                 "@gMax Level      = @c%d\r\n"
                 "@gZone Flags     = @c%s\r\n"
                 "@gSize\r\n"
                 "@g   Rooms       = @c%d\r\n"
                 "@g   Objects     = @c%d\r\n"
                 "@g   Mobiles     = @c%d\r\n"
                 "@g   Shops       = @c%d\r\n"
                 "@g   Triggers    = @c%d\r\n"
                 "@g   Guilds      = @c%d@n\r\n",
                 z.number, z.name,
                 z.builders, z.lifespan,
                 z.age, z.bot, z.top,
                 z.reset_mode ? ((z.reset_mode == 1) ?
                                                "Reset when no players are in zone." : "Normal reset.") : "Never reset",
                 z.min_level, z.max_level, bits,
                 size_rooms, size_objects, size_mobiles, size_shops, size_triggers, size_guilds);
}

/* List code by Ronald Evers - dlanor@xs4all.nl */
void list_triggers(struct char_data *ch, zone_rnum rnum, trig_vnum vmin, trig_vnum vmax) {
    int i, bottom, top, counter = 0;
    char trgtypes[256];

    /** Expect a minimum / maximum number if the rnum for the zone is NOWHERE. **/
    if (rnum != NOWHERE) {
        bottom = zone_table[rnum].bot;
        top = zone_table[rnum].top;
    } else {
        bottom = vmin;
        top = vmax;
    }


    /** Store the header for the room listing. **/
    send_to_char(ch,
                 "Index VNum    Trigger Name                        Type\r\n"
                 "----- ------- -------------------------------------------------------\r\n");


    /** Loop through the world and find each room. **/
    for (auto &t : trig_index) {
        /** Check to see if this room is one of the ones needed to be listed.    **/
        if ((t.first >= bottom) && (t.first <= top)) {
            counter++;

            send_to_char(ch, "%4d) [@g%5d@n] @[1]%-45.45s ",
                         counter, t.first, t.second.proto->name);

            if (t.second.proto->attach_type == OBJ_TRIGGER) {
                sprintbit(GET_TRIG_TYPE(t.second.proto), otrig_types, trgtypes, sizeof(trgtypes));
                send_to_char(ch, "obj @y%s@n\r\n", trgtypes);
            } else if (t.second.proto->attach_type == WLD_TRIGGER) {
                sprintbit(GET_TRIG_TYPE(t.second.proto), wtrig_types, trgtypes, sizeof(trgtypes));
                send_to_char(ch, "wld @y%s@n\r\n", trgtypes);
            } else {
                sprintbit(GET_TRIG_TYPE(t.second.proto), trig_types, trgtypes, sizeof(trgtypes));
                send_to_char(ch, "mob @y%s@n\r\n", trgtypes);
            }

        }
    }

    if (counter == 0) {
        send_to_char(ch, "No triggers found from %d to %d.\r\n", vmin, vmax);
    }
}
