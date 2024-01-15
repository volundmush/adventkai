/************************************************************************
 *  OasisOLC - Rooms / redit.c					v2.0	*
 *  Original author: Levork						*
 *  Copyright 1996 Harvey Gilpin					*
 *  Copyright 1997-2001 George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/db.h"
#include "dbat/boards.h"
#include "dbat/genolc.h"
#include "dbat/genwld.h"
#include "dbat/genzon.h"
#include "dbat/oasis.h"
#include "dbat/improved-edit.h"
#include "dbat/dg_olc.h"
#include "dbat/constants.h"
#include "dbat/spells.h"
#include "dbat/handler.h"
#include "dbat/races.h"
#include "dbat/act.wizard.h"
#include "dbat/act.informative.h"

/*------------------------------------------------------------------------*/

/*
 * External data structures.
 */

/*------------------------------------------------------------------------*/


/*------------------------------------------------------------------------*\
  Utils and exported functions.
\*------------------------------------------------------------------------*/

ACMD(do_oasis_redit) {
    char *buf3;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int number = NOWHERE, save = 0, real_num;
    struct descriptor_data *d;

    /* Parse any arguments. */
    if (GET_ADMLEVEL(ch) > 0) {
        buf3 = two_arguments(argument, buf1, buf2);
    }

    if (GET_ADMLEVEL(ch) < 1 && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_CANREMODEL)) {
        send_to_char(ch, "You can not remodel this room.\r\n");
        return;
    }

    struct obj_data *capsule = nullptr, *next_obj = nullptr, *remove = nullptr;
    int remodeling = false;

    for (capsule = ch->contents; capsule; capsule = next_obj) {
        next_obj = capsule->next_content;
        if (remove != nullptr) {
            continue;
        } else if (GET_OBJ_VNUM(capsule) == 19094) {
            remove = capsule;
        }
    }

    if (remove == nullptr && GET_ADMLEVEL(ch) < 1) {
        send_to_char(ch, "You do not have a R.A.D. Remodeling Assistance Droid.\r\n");
        return;
    } else if (GET_ADMLEVEL(ch) < 1) {
        act("@GYou open up the computer panel on the droid and begin to program its remodeling routine.@n", true, ch,
            nullptr, nullptr, TO_CHAR);
        act("@g$n@G opens up a computer panel on some kind of small spherical droid and begins to program it.@n", true,
            ch, nullptr, nullptr, TO_ROOM);
        extract_obj(remove);
        remodeling = true;
    }

    if (!*buf1 || GET_ADMLEVEL(ch) < 1)
        number = GET_ROOM_VNUM(IN_ROOM(ch));
    else if (!isdigit(*buf1)) {
        if (strcasecmp("save", buf1) != 0) {
            send_to_char(ch, "Yikes!  Stop that, someone will get hurt!\r\n");
            return;
        }

        save = true;

        if (is_number(buf2))
            number = atoi(buf2);
        else if (GET_OLC_ZONE(ch) >= 0) {
            zone_rnum zlok;

            if ((zlok = real_zone(GET_OLC_ZONE(ch))) == NOWHERE)
                number = NOWHERE;
            else
                number = genolc_zone_bottom(zlok);
        }

        if (number == NOWHERE) {
            send_to_char(ch, "Save which zone?\r\n");
            return;
        }
    }

    /*
     * If a numeric argument was given (like a room number), get it.
     */
    if (number == NOWHERE)
        number = atoi(buf1);

    /* Check to make sure the room isn't already being edited. */
    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) == CON_REDIT) {
            if (d->olc && OLC_NUM(d) == number) {
                send_to_char(ch, "That room is currently being edited by %s.\r\n",
                             PERS(d->character, ch));
                return;
            }
        }
    }

    /* Retrieve the player's descriptor. */
    d = ch->desc;

    /* Give the descriptor an OLC structure. */
    if (d->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true, "SYSERR: do_oasis_redit: Player already had olc structure.");
        free(d->olc);
    }

    /* Create the OLC structure. */
    CREATE(d->olc, struct oasis_olc_data, 1);

    /* Find the zone. */
    OLC_ZNUM(d) = save ? real_zone(number) : real_zone_by_thing(number);
    if (OLC_ZNUM(d) == NOWHERE) {
        send_to_char(ch, "Sorry, there is no zone for that number!\r\n");
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /* Make sure the builder is allowed to modify this zone. */
    if (!can_edit_zone(ch, OLC_ZNUM(d)) && remodeling == false) {
        send_cannot_edit(ch, zone_table[OLC_ZNUM(d)].number);
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    if (save) {
        send_to_char(ch, "Saving all rooms in zone %d.\r\n", zone_table[OLC_ZNUM(d)].number);
        mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(ch)), true, "OLC: %s saves room info for zone %d.", GET_NAME(ch),
               zone_table[OLC_ZNUM(d)].number);

        /* Save the rooms. */
        save_rooms(OLC_ZNUM(d));

        /* Free the olc data from the descriptor. */
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    OLC_NUM(d) = number;

    if ((real_num = real_room(number)) != NOWHERE)
        redit_setup_existing(d, real_num);
    else
        redit_setup_new(d);

    redit_disp_menu(d);
    STATE(d) = CON_REDIT;
    act("$n starts using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);
    ch->playerFlags.set(PLR_WRITING);

    mudlog(BRF, ADMLVL_IMMORT, true, "OLC: %s starts editing zone %d allowed zone %d",
           GET_NAME(ch), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(ch));
}

void redit_setup_new(struct descriptor_data *d) {
    OLC_ROOM(d) = new room_data();

    OLC_ROOM(d)->name = strdup("An unfinished room");
    OLC_ROOM(d)->look_description = strdup("You are in an unfinished room.\r\n");
    OLC_ROOM(d)->vn = NOWHERE;
    OLC_ITEM_TYPE(d) = WLD_TRIGGER;
    OLC_SCRIPT(d).clear();

    OLC_VAL(d) = 0;
}

/*------------------------------------------------------------------------*/

void redit_setup_existing(struct descriptor_data *d, int real_num) {
    int counter;

    /*
     * Build a copy of the room for editing.
     */
    auto room = new room_data();

    *room = world[real_num];
    /*
     * Allocate space for all strings.
     */
    room->name = str_udup(world[real_num].name);
    room->look_description = str_udup(world[real_num].look_description);

    /*
     * Exits - We allocate only if necessary.
     */
    for (counter = 0; counter < NUM_OF_DIRS; counter++) {
        if (world[real_num].dir_option[counter]) {
            CREATE(room->dir_option[counter], struct room_direction_data, 1);

            /*
             * Copy the numbers over.
             */
            *room->dir_option[counter] = *world[real_num].dir_option[counter];
            /*
             * Allocate the strings.
             */
            if (world[real_num].dir_option[counter]->general_description)
                room->dir_option[counter]->general_description = strdup(
                        world[real_num].dir_option[counter]->general_description);
            if (world[real_num].dir_option[counter]->keyword)
                room->dir_option[counter]->keyword = strdup(world[real_num].dir_option[counter]->keyword);
        }
    }

    /*
     * Extra descriptions, if necessary.
     */
    if (world[real_num].ex_description) {
        struct extra_descr_data *tdesc, *temp, *temp2;
        CREATE(temp, struct extra_descr_data, 1);

        room->ex_description = temp;
        for (tdesc = world[real_num].ex_description; tdesc; tdesc = tdesc->next) {
            temp->keyword = strdup(tdesc->keyword);
            temp->description = strdup(tdesc->description);
            if (tdesc->next) {
                CREATE(temp2, struct extra_descr_data, 1);
                temp->next = temp2;
                temp = temp2;
            } else
                temp->next = nullptr;
        }
    }
    /*
     * Attach copy of room to player's descriptor.
     */
    OLC_ROOM(d) = room;
    OLC_VAL(d) = 0;
    OLC_ITEM_TYPE(d) = WLD_TRIGGER;

    dg_olc_script_copy(d);
    room->proto_script.clear();
    SCRIPT(room) = nullptr;
}

/*------------------------------------------------------------------------*/

void redit_save_internally(struct descriptor_data *d) {
    int j, room_num, new_room = false;
    struct descriptor_data *dsc;

    if (OLC_ROOM(d)->vn == NOWHERE) {
        new_room = true;
    }
    OLC_ROOM(d)->vn = OLC_NUM(d);
    /* FIXME: Why is this not set elsewhere? */
    OLC_ROOM(d)->zone = OLC_ZNUM(d);

    if ((room_num = add_room(OLC_ROOM(d))) == NOWHERE) {
        write_to_output(d, "Something went wrong...\r\n");
        basic_mud_log("SYSERR: redit_save_internally: Something failed! (%d)", room_num);
        return;
    }

    /* Update triggers */
    /* Free old proto list */
    if (world[room_num].proto_script != OLC_SCRIPT(d))
        free_proto_script(&world[room_num], WLD_TRIGGER);

    world[room_num].proto_script = OLC_SCRIPT(d);
    assign_triggers(&world[room_num], WLD_TRIGGER);
    /* end trigger update */

    /* Don't adjust numbers on a room update. */
    if (!new_room)
        return;

    /* Idea contributed by C.Raehl 4/27/99 */
    for (dsc = descriptor_list; dsc; dsc = dsc->next) {
        if (dsc == d)
            continue;

        if (STATE(dsc) == CON_ZEDIT) {
            for (j = 0; OLC_ZONE(dsc)->cmd[j].command != 'S'; j++)
                switch (OLC_ZONE(dsc)->cmd[j].command) {
                    case 'O':
                    case 'M':
                    case 'T':
                    case 'V':
                        OLC_ZONE(dsc)->cmd[j].arg3 += (OLC_ZONE(dsc)->cmd[j].arg3 >= room_num);
                        break;
                    case 'D':
                        OLC_ZONE(dsc)->cmd[j].arg2 += (OLC_ZONE(dsc)->cmd[j].arg2 >= room_num);
                        /* Fall through */
                    case 'R':
                        OLC_ZONE(dsc)->cmd[j].arg1 += (OLC_ZONE(dsc)->cmd[j].arg1 >= room_num);
                        break;
                }
        } else if (STATE(dsc) == CON_REDIT) {
            for (j = 0; j < NUM_OF_DIRS; j++)
                if (OLC_ROOM(dsc)->dir_option[j])
                    if (OLC_ROOM(dsc)->dir_option[j]->to_room >= room_num)
                        OLC_ROOM(dsc)->dir_option[j]->to_room++;
        }
    }
}

/*------------------------------------------------------------------------*/

void redit_save_to_disk(zone_vnum zone_num) {
    save_rooms(zone_num);        /* :) */
    update_space();
}

/*------------------------------------------------------------------------*/

void free_room(struct room_data *room) {
    /* Free the strings (Mythran). */
    free_room_strings(room);

    if (SCRIPT(room))
        extract_script(room, WLD_TRIGGER);
    free_proto_script(room, WLD_TRIGGER);

    /* Free the room. */
    free(room);    /* XXX ? */
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * For extra descriptions.
 */
void redit_disp_extradesc_menu(struct descriptor_data *d) {
    struct extra_descr_data *extra_desc = OLC_DESC(d);

    clear_screen(d);
    write_to_output(d,
                    "@g1@n) Keyword: @y%s\r\n"
                    "@g2@n) Description:\r\n@y%s\r\n"
                    "@g3@n) Goto next description: ",

                    extra_desc->keyword ? extra_desc->keyword : "<NONE>",
                    extra_desc->description ? extra_desc->description : "<NONE>"
    );

    write_to_output(d, !extra_desc->next ? "Not Set.\r\n" : "Set.\r\n");
    write_to_output(d, "Enter choice (0 to quit) : ");
    OLC_MODE(d) = REDIT_EXTRADESC_MENU;
}

/*
 * For exits.
 */
void redit_disp_exit_menu(struct descriptor_data *d) {
    char door_buf[40];
    /*
     * if exit doesn't exist, alloc/create it
     */
    if (OLC_EXIT(d) == nullptr)
        CREATE(OLC_EXIT(d), struct room_direction_data, 1);

    sprintbit(OLC_EXIT(d)->exit_info, exit_bits, door_buf, sizeof(door_buf));

    clear_screen(d);
    write_to_output(d,
                    "@g1@n) Exit to     		: @c%d\r\n"
                    "@g2@n) Description 		:-\r\n@y%s\r\n"
                    "@g3@n) Door name   		: @y%s\r\n"
                    "@g4@n) Key         		: @c%d\r\n"
                    "@g5@n) Door flags  		: @c%s\r\n"
                    "@g6@n) Purge exit.\r\n"
                    "@g7@n) DC Lock		: @c%d\r\n"
                    "@g8@n) DC Hide     		: @c%d\r\n"
                    "@g9@n) DC Skill		: @c%s\r\n"
                    "@gA@n) DC Move		: @c%d\r\n"
                    "@gB@n) Skill Fail Save Type		: @c%d\r\n"
                    "@gC@n) DC Skill Save		: @c%d\r\n"
                    "@gD@n) Minor Fail Dest. Room	: @c%d\r\n"
                    "@gE@n) Major Fail Dest. Room	: @c%d@n\r\n"
                    "Enter choice, 0 to quit : ",

                    OLC_EXIT(d)->to_room != NOWHERE ? world[OLC_EXIT(d)->to_room].vn : -1,
                    OLC_EXIT(d)->general_description ? OLC_EXIT(d)->general_description : "<NONE>",
                    OLC_EXIT(d)->keyword ? OLC_EXIT(d)->keyword : "<NONE>",
                    OLC_EXIT(d)->key != NOTHING ? OLC_EXIT(d)->key : -1,
                    door_buf, OLC_EXIT(d)->dclock, OLC_EXIT(d)->dchide,
                    OLC_EXIT(d)->dcskill != 0 ? spell_info[OLC_EXIT(d)->dcskill].name : "<NONE>",
                    OLC_EXIT(d)->dcmove, OLC_EXIT(d)->failsavetype,
                    OLC_EXIT(d)->dcfailsave, OLC_EXIT(d)->failroom,
                    OLC_EXIT(d)->totalfailroom
    );

    OLC_MODE(d) = REDIT_EXIT_MENU;
}

/*
 * For exit flags.
 */
void redit_disp_exit_flag_menu(struct descriptor_data *d) {
    char bits[MAX_INPUT_LENGTH];
    int counter, columns = 0;

    clear_screen(d);
    for (counter = 0; counter < NUM_EXIT_FLAGS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s%s",
                        counter + 1,
                        exit_bits[counter],
                        !(++columns % 2) ? "\r\n" : "");
    }
    sprintbit(OLC_EXIT(d)->exit_info, exit_bits, bits, sizeof(bits));
    write_to_output(d, "\r\nExit flags: @c%s@n\r\n"
                       "Enter exit flags, 0 to quit : ",
                    bits);
    OLC_MODE(d) = REDIT_EXIT_DOORFLAGS;
}

/*
 * For room flags.
 */
void redit_disp_flag_menu(struct descriptor_data *d) {
    char bits[MAX_STRING_LENGTH];
    int counter, columns = 0;

    clear_screen(d);
    for (counter = 0; counter < NUM_ROOM_FLAGS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s%s", counter + 1,
                        room_bits[counter], !(++columns % 2) ? "\r\n" : "");
    }
    sprintbitarray(OLC_ROOM(d)->room_flags, room_bits, RF_ARRAY_MAX, bits);
    write_to_output(d, "\r\nRoom flags: @c%s@n\r\n"
                       "Enter room flags, 0 to quit : ", bits);
    OLC_MODE(d) = REDIT_FLAGS;
}

/*
 * For sector type.
 */
void redit_disp_sector_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);
    for (counter = 0; counter < NUM_ROOM_SECTORS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s%s", counter,
                        sector_types[counter], !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter sector type : ");
    OLC_MODE(d) = REDIT_SECTOR;
}

/*
 * The main menu.
 */
void redit_disp_menu(struct descriptor_data *d) {
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct room_data *room;

    clear_screen(d);
    room = OLC_ROOM(d);

    sprintbitarray(room->room_flags, room_bits, RF_ARRAY_MAX, buf1);
    sprinttype(room->sector_type, sector_types, buf2, sizeof(buf2));
    if (GET_ADMLEVEL(d->character) > 0) {
        write_to_output(d,
                        "-- Room number : [@c%d@n]  	Room zone: [@c%d@n]\r\n"
                        "@g1@n) Name        : @y%s\r\n"
                        "@g2@n) Description :\r\n@y%s"
                        "@g3@n) Room flags  : @c%s\r\n"
                        "@g4@n) Sector type : @c%s\r\n"
                        "@g5@n) Exit north  : [@c%6d@n],  @gB@n) Exit northwest : [@c%6d@n]\r\n"
                        "@g6@n) Exit east   : [@c%6d@n],  @gC@n) Exit northeast : [@c%6d@n]\r\n"
                        "@g7@n) Exit south  : [@c%6d@n],  @gD@n) Exit southeast : [@c%6d@n]\r\n"
                        "@g8@n) Exit west   : [@c%6d@n],  @gE@n) Exit southwest : [@c%6d@n]\r\n"
                        "@g9@n) Exit up     : [@c%6d@n],  @gF@n) Exit in        : [@c%6d@n]\r\n"
                        "@gA@n) Exit down   : [@c%6d@n],  @gG@n) Exit out       : [@c%6d@n]\r\n"
                        "@gH@n) Extra descriptions menu\r\n"
                        "@gW@n) Copy Room\r\n"
                        "@gX@n) Delete Room\r\n"
                        "@gS@n) Script      : @c%s\r\n"
                        "@gZ@n) Wiznet      :\r\n"
                        "@gQ@n) Quit\r\n"
                        "Enter choice : ",

                        OLC_NUM(d), zone_table[OLC_ZNUM(d)].number, room->name,
                        room->look_description, buf1, buf2,
                        room->dir_option[NORTH] && room->dir_option[NORTH]->to_room != NOWHERE ?
                        world[room->dir_option[NORTH]->to_room].vn : -1,
                        room->dir_option[NORTHWEST] && room->dir_option[NORTHWEST]->to_room != NOWHERE ?
                        world[room->dir_option[NORTHWEST]->to_room].vn : -1,
                        room->dir_option[EAST] && room->dir_option[EAST]->to_room != NOWHERE ?
                        world[room->dir_option[EAST]->to_room].vn : -1,
                        room->dir_option[NORTHEAST] && room->dir_option[NORTHEAST]->to_room != NOWHERE ?
                        world[room->dir_option[NORTHEAST]->to_room].vn : -1,
                        room->dir_option[SOUTH] && room->dir_option[SOUTH]->to_room != NOWHERE ?
                        world[room->dir_option[SOUTH]->to_room].vn : -1,
                        room->dir_option[SOUTHEAST] && room->dir_option[SOUTHEAST]->to_room != NOWHERE ?
                        world[room->dir_option[SOUTHEAST]->to_room].vn : -1,
                        room->dir_option[WEST] && room->dir_option[WEST]->to_room != NOWHERE ?
                        world[room->dir_option[WEST]->to_room].vn : -1,
                        room->dir_option[SOUTHWEST] && room->dir_option[SOUTHWEST]->to_room != NOWHERE ?
                        world[room->dir_option[SOUTHWEST]->to_room].vn : -1,
                        room->dir_option[UP] && room->dir_option[UP]->to_room != NOWHERE ?
                        world[room->dir_option[UP]->to_room].vn : -1,
                        room->dir_option[INDIR] && room->dir_option[INDIR]->to_room != NOWHERE ?
                        world[room->dir_option[INDIR]->to_room].vn : -1,
                        room->dir_option[DOWN] && room->dir_option[DOWN]->to_room != NOWHERE ?
                        world[room->dir_option[DOWN]->to_room].vn : -1,
                        room->dir_option[OUTDIR] && room->dir_option[OUTDIR]->to_room != NOWHERE ?
                        world[room->dir_option[OUTDIR]->to_room].vn : -1,
                        !OLC_SCRIPT(d).empty() ? "Set." : "Not Set."
        );
    } else {
        write_to_output(d,
                        "-- Room number : [@c%d@n]    Room zone: [@c%d@n]\r\n"
                        "@g1@n) Location Designation (Room Name)        : @y%s\r\n"
                        "@g2@n) Remodeling Routine (Room Description)   :\r\n@y%s"
                        "@gH@n) Extra descriptions menu\r\n"
                        "@gQ@n) Quit\r\n"
                        "Enter choice : ",

                        OLC_NUM(d), zone_table[OLC_ZNUM(d)].number, room->name,
                        room->look_description);
    }

    OLC_MODE(d) = REDIT_MAIN_MENU;
}

/**************************************************************************
  The main loop
 **************************************************************************/

void redit_parse(struct descriptor_data *d, char *arg) {
    int number;
    char *oldtext = nullptr;

    switch (OLC_MODE(d)) {
        case REDIT_CONFIRM_SAVESTRING:
            switch (*arg) {
                case 'y':
                case 'Y':
                    redit_save_internally(d);
                    mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(d->character)), true,
                           "OLC: %s edits room %d.", GET_NAME(d->character), OLC_NUM(d));
                    if (CONFIG_OLC_SAVE) {
                        redit_save_to_disk(real_zone_by_thing(OLC_NUM(d)));
                        if (GET_ADMLEVEL(d->character) < 1) {
                            write_to_output(d,
                                            "@GThe remodeling droid quickly launches about with remodeling the room to your specifications. It seems to finish in no time at all...@n\r\n");
                            act("@GThe remodeling droid quickly launches about with remodeling the room as to @g$n's@G specifications. It seems to finish in no time at all...@n",
                                true, d->character, nullptr, nullptr, TO_ROOM);
                        } else {
                            write_to_output(d, "Room saved to disk.\r\n");
                        }
                    } else
                        write_to_output(d, "Room saved to memory.\r\n");
                    /* Free everything. */
                    cleanup_olc(d, CLEANUP_ALL);
                    break;
                case 'n':
                case 'N':
                    /* If not saving, we must free the script_proto list. We do so by
                     * assigning it to the edited room and letting free_room in
                     * cleanup_olc handle it. */
                    OLC_ROOM(d)->proto_script = OLC_SCRIPT(d);
                    cleanup_olc(d, CLEANUP_ALL);
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d,
                                        "@GYou close the droids interface and put it back among the rest of your things.@n\r\n");
                        act("@g$n@G stops typing information into the droid in $s hands and closes it back up.@n\r\n",
                            true, d->character, nullptr, nullptr, TO_ROOM);
                        struct obj_data *obj;
                        obj = read_object(19094, VIRTUAL);
                        obj_to_char(obj, d->character);
                    }
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\nDo you wish to save your changes ? : ");
                    break;
            }
            return;

        case REDIT_MAIN_MENU:
            switch (*arg) {
                case 'q':
                case 'Q':
                    if (OLC_VAL(d)) { /* Something has been modified. */
                        write_to_output(d, "Do you wish to save your changes? : ");
                        OLC_MODE(d) = REDIT_CONFIRM_SAVESTRING;
                    } else
                        cleanup_olc(d, CLEANUP_ALL);
                    return;
                case '1':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "Enter Location Name:\r\n");
                    } else {
                        write_to_output(d, "Enter Room Name:-\r\n] ");
                    }
                    OLC_MODE(d) = REDIT_NAME;
                    break;
                case '2':
                    OLC_MODE(d) = REDIT_DESC;
                    clear_screen(d);
                    send_editor_help(d);
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "Set Remodel Parameters (Enter Room Description):-\r\n] ");
                    } else {
                        write_to_output(d, "Enter room description:\r\n\r\n");
                    }

                    if (OLC_ROOM(d)->look_description) {
                        write_to_output(d, "%s", OLC_ROOM(d)->look_description);
                        oldtext = strdup(OLC_ROOM(d)->look_description);
                    }
                    string_write(d, &OLC_ROOM(d)->look_description, MAX_ROOM_DESC, 0, oldtext);
                    OLC_VAL(d) = 1;
                    break;
                case '3':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    redit_disp_flag_menu(d);
                    break;
                case '4':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    redit_disp_sector_menu(d);
                    break;
                case '5':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = NORTH;
                    redit_disp_exit_menu(d);
                    break;
                case '6':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = EAST;
                    redit_disp_exit_menu(d);
                    break;
                case '7':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = SOUTH;
                    redit_disp_exit_menu(d);
                    break;
                case '8':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = WEST;
                    redit_disp_exit_menu(d);
                    break;
                case '9':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = UP;
                    redit_disp_exit_menu(d);
                    break;
                case 'a':
                case 'A':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = DOWN;
                    redit_disp_exit_menu(d);
                    break;
                case 'b':
                case 'B':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = NORTHWEST;
                    redit_disp_exit_menu(d);
                    break;
                case 'c':
                case 'C':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = NORTHEAST;
                    redit_disp_exit_menu(d);
                    break;
                case 'd':
                case 'D':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = SOUTHEAST;
                    redit_disp_exit_menu(d);
                    break;
                case 'e':
                case 'E':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = SOUTHWEST;
                    redit_disp_exit_menu(d);
                    break;
                case 'f':
                case 'F':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = INDIR;
                    redit_disp_exit_menu(d);
                    break;
                case 'g':
                case 'G':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_VAL(d) = OUTDIR;
                    redit_disp_exit_menu(d);
                    break;
                case 'h':
                case 'H':
                    /*
                     * If the extra description doesn't exist.
                     */
                    if (!OLC_ROOM(d)->ex_description)
                        CREATE(OLC_ROOM(d)->ex_description, struct extra_descr_data, 1);
                    OLC_DESC(d) = OLC_ROOM(d)->ex_description;
                    redit_disp_extradesc_menu(d);
                    break;
                case 'w':
                case 'W':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    write_to_output(d, "Disabled for the time being!");
                    /*OLC_MODE(d) = REDIT_COPY;*/
                    break;
                case 'x':
                case 'X':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    /* Delete the room, prompt first. */
                    write_to_output(d, "Are you sure you want to delete this room? ");
                    OLC_MODE(d) = REDIT_DELETE;
                    break;
                case 's':
                case 'S':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
                    dg_script_menu(d);
                    return;
                case 'Z':
                case 'z':
                    if (GET_ADMLEVEL(d->character) < 1) {
                        write_to_output(d, "That options isn't available to non-builders.\r\n");
                        break;
                    }
                    search_replace(arg, "z ", "");
                    do_wiznet(d->character, arg, 0, 0);
                    break;
                default:
                    write_to_output(d, "Invalid choice!");
                    redit_disp_menu(d);
                    break;
            }
            return;


        case OLC_SCRIPT_EDIT:
            if (dg_script_edit_parse(d, arg)) return;
            break;

        case REDIT_NAME:
            if (!genolc_checkstring(d, arg))
                break;
            if (OLC_ROOM(d)->name)
                free(OLC_ROOM(d)->name);
            arg[MAX_ROOM_NAME] = '\0';
            OLC_ROOM(d)->name = str_udup(arg);
            break;

        case REDIT_DESC:
            /*
             * We will NEVER get here, we hope.
             */
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: Reached REDIT_DESC case in parse_redit().");
            write_to_output(d, "Oops, in REDIT_DESC.\r\n");
            break;

        case REDIT_FLAGS:
            number = atoi(arg);
            if (number < 0 || number > NUM_ROOM_FLAGS) {
                write_to_output(d, "That is not a valid choice!\r\n");
                redit_disp_flag_menu(d);
            } else if (number == 0)
                break;
            else {
                /*
                 * Toggle the bit.
                 */
                OLC_ROOM(d)->room_flags.flip(number-1);
                redit_disp_flag_menu(d);
            }
            return;

        case REDIT_SECTOR:
            number = atoi(arg);
            if (number < 0 || number >= NUM_ROOM_SECTORS) {
                write_to_output(d, "Invalid choice!");
                redit_disp_sector_menu(d);
                return;
            }
            OLC_ROOM(d)->sector_type = number;
            break;

        case REDIT_EXIT_MENU:
            switch (*arg) {
                case '0':
                    break;
                case '1':
                    OLC_MODE(d) = REDIT_EXIT_NUMBER;
                    write_to_output(d, "Exit to room number : ");
                    return;
                case '2':
                    OLC_MODE(d) = REDIT_EXIT_DESCRIPTION;
                    send_editor_help(d);
                    write_to_output(d, "Enter exit description:\r\n\r\n");
                    if (OLC_EXIT(d)->general_description) {
                        write_to_output(d, "%s", OLC_EXIT(d)->general_description);
                        oldtext = strdup(OLC_EXIT(d)->general_description);
                    }
                    string_write(d, &OLC_EXIT(d)->general_description, MAX_EXIT_DESC, 0, oldtext);
                    return;
                case '3':
                    OLC_MODE(d) = REDIT_EXIT_KEYWORD;
                    write_to_output(d, "Enter keywords : ");
                    return;
                case '4':
                    OLC_MODE(d) = REDIT_EXIT_KEY;
                    write_to_output(d, "Enter key number : ");
                    return;
                case '5':
                    OLC_MODE(d) = REDIT_EXIT_DOORFLAGS;
                    redit_disp_exit_flag_menu(d);
                    return;
                case '6':
                    /*
                     * Delete an exit.
                     */
                    if (OLC_EXIT(d)->keyword)
                        free(OLC_EXIT(d)->keyword);
                    if (OLC_EXIT(d)->general_description)
                        free(OLC_EXIT(d)->general_description);
                    if (OLC_EXIT(d))
                        free(OLC_EXIT(d));
                    OLC_EXIT(d) = nullptr;
                    break;
                case '7':
                    OLC_MODE(d) = REDIT_EXIT_DCLOCK;
                    write_to_output(d, "Enter lock DC number : ");
                    return;
                case '8':
                    OLC_MODE(d) = REDIT_EXIT_DCHIDE;
                    write_to_output(d, "Enter door search DC number : ");
                    return;
                case '9':
                    OLC_MODE(d) = REDIT_EXIT_DCSKILL;
                    write_to_output(d, "Enter skill to be checked to pass through exit : ");
                    return;
                case 'a':
                case 'A':
                    OLC_MODE(d) = REDIT_EXIT_DCMOVE;
                    write_to_output(d, "Enter DC for skill required to pass through exit : ");
                    return;
                case 'b':
                case 'B':
                    OLC_MODE(d) = REDIT_EXIT_SAVETYPE;
                    write_to_output(d, "Enter the SAVE TYPE for failed skill checks : ");
                    return;
                case 'c':
                case 'C':
                    OLC_MODE(d) = REDIT_EXIT_DCSAVE;
                    write_to_output(d, "Enter the DC to beat for SAVE from failed skill checks : ");
                    return;
                case 'd':
                case 'D':
                    OLC_MODE(d) = REDIT_EXIT_FAILROOM;
                    write_to_output(d, "Enter the room to send play to for minor save failure : ");
                    return;
                case 'e':
                case 'E':
                    OLC_MODE(d) = REDIT_EXIT_TOTALFAILROOM;
                    write_to_output(d, "Enter the room to send play to for major save failure : ");
                    return;
                default:
                    write_to_output(d, "Try again : ");
                    return;
            }
            break;

        case REDIT_EXIT_NUMBER:
            if ((number = atoi(arg)) != -1)
                if ((number = real_room(number)) == NOWHERE) {
                    write_to_output(d, "That room does not exist, try again : ");
                    return;
                }
            OLC_EXIT(d)->to_room = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_DESCRIPTION:
            /*
             * We should NEVER get here, hopefully.
             */
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: Reached REDIT_EXIT_DESC case in parse_redit");
            write_to_output(d, "Oops, in REDIT_EXIT_DESCRIPTION.\r\n");
            break;

        case REDIT_EXIT_KEYWORD:
            if (OLC_EXIT(d)->keyword)
                free(OLC_EXIT(d)->keyword);
            OLC_EXIT(d)->keyword = str_udup(arg);
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_KEY:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->key = NOTHING;
            else
                OLC_EXIT(d)->key = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_DOORFLAGS:
            number = atoi(arg);
            if (number < 0 || number > NUM_EXIT_FLAGS) {
                write_to_output(d, "That's not a valid choice!\r\n");
                redit_disp_exit_flag_menu(d);
            } else if (number == 0) {
                redit_disp_exit_menu(d);
            } else {
                /* Toggle the bit. */
                TOGGLE_BIT(OLC_EXIT(d)->exit_info, 1 << (number - 1));
                redit_disp_exit_flag_menu(d);
            }
            return;

        case REDIT_EXTRADESC_KEY:
            if (genolc_checkstring(d, arg)) {
                if (OLC_DESC(d)->keyword)
                    free(OLC_DESC(d)->keyword);
                OLC_DESC(d)->keyword = str_udup(arg);
            }
            redit_disp_extradesc_menu(d);
            return;

        case REDIT_EXTRADESC_MENU:
            switch ((number = atoi(arg))) {
                case 0:
                    /*
                     * If something got left out, delete the extra description
                     * when backing out to the menu.
                     */
                    if (OLC_DESC(d)->keyword == nullptr || OLC_DESC(d)->description == nullptr) {
                        struct extra_descr_data *temp;
                        if (OLC_DESC(d)->keyword)
                            free(OLC_DESC(d)->keyword);
                        if (OLC_DESC(d)->description)
                            free(OLC_DESC(d)->description);

                        /*
                         * Clean up pointers.
                         */
                        REMOVE_FROM_LIST(OLC_DESC(d), OLC_ROOM(d)->ex_description, next, temp);
                        free(OLC_DESC(d));
                    }
                    break;
                case 1:
                    OLC_MODE(d) = REDIT_EXTRADESC_KEY;
                    write_to_output(d, "Enter keywords, separated by spaces : ");
                    return;
                case 2:
                    OLC_MODE(d) = REDIT_EXTRADESC_DESCRIPTION;
                    send_editor_help(d);
                    write_to_output(d, "Enter extra description:\r\n\r\n");
                    if (OLC_DESC(d)->description) {
                        write_to_output(d, "%s", OLC_DESC(d)->description);
                        oldtext = strdup(OLC_DESC(d)->description);
                    }
                    string_write(d, &OLC_DESC(d)->description, MAX_MESSAGE_LENGTH, 0, oldtext);
                    return;
                case 3:
                    if (OLC_DESC(d)->keyword == nullptr || OLC_DESC(d)->description == nullptr) {
                        write_to_output(d,
                                        "You can't edit the next extra description without completing this one.\r\n");
                        redit_disp_extradesc_menu(d);
                    } else {
                        struct extra_descr_data *new_extra;

                        if (OLC_DESC(d)->next)
                            OLC_DESC(d) = OLC_DESC(d)->next;
                        else {
                            /*
                             * Make new extra description and attach at end.
                             */
                            CREATE(new_extra, struct extra_descr_data, 1);
                            OLC_DESC(d)->next = new_extra;
                            OLC_DESC(d) = new_extra;
                        }
                        redit_disp_extradesc_menu(d);
                    }
                    return;
            }
            break;

        case REDIT_COPY:
            if ((number = real_room(atoi(arg))) != NOWHERE) {
                redit_setup_existing(d, number);
            } else
                write_to_output(d, "That room does not exist.\r\n");
            break;

        case REDIT_DELETE:
            if (*arg == 'y' || *arg == 'Y') {
                if (delete_room(real_room(OLC_ROOM(d)->vn)))
                    write_to_output(d, "Room deleted.\r\n");
                else
                    write_to_output(d, "Couldn't delete the room!\r\n");

                cleanup_olc(d, CLEANUP_ALL);
                return;
            } else if (*arg == 'n' || *arg == 'N') {
                redit_disp_menu(d);
                OLC_MODE(d) = REDIT_MAIN_MENU;
                return;
            } else
                write_to_output(d, "Please answer 'Y' or 'N': ");

            break;

        case REDIT_EXIT_DCLOCK:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->dclock = NOTHING;
            else
                OLC_EXIT(d)->dclock = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_DCHIDE:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->dchide = NOTHING;
            else
                OLC_EXIT(d)->dchide = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_DCSKILL:
            number = find_skill_num(arg, SKTYPE_SKILL);
            if (number < 1)
                OLC_EXIT(d)->dcskill = 0;
            else
                OLC_EXIT(d)->dcskill = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_DCMOVE:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->dcmove = NOTHING;
            else
                OLC_EXIT(d)->dcmove = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_SAVETYPE:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->failsavetype = NOTHING;
            else
                OLC_EXIT(d)->failsavetype = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_DCSAVE:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->dcfailsave = NOTHING;
            else
                OLC_EXIT(d)->dcfailsave = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_FAILROOM:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->failroom = NOWHERE;
            else
                OLC_EXIT(d)->failroom = number;
            redit_disp_exit_menu(d);
            return;

        case REDIT_EXIT_TOTALFAILROOM:
            number = atoi(arg);
            if (number < 0)
                OLC_EXIT(d)->totalfailroom = NOWHERE;
            else
                OLC_EXIT(d)->totalfailroom = number;
            redit_disp_exit_menu(d);
            return;

        default:
            /*
             * We should never get here.
             */
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: Reached default case in parse_redit");
            break;
    }
    /*
     * If we get this far, something has been changed.
     */
    OLC_VAL(d) = 1;
    redit_disp_menu(d);
}

void redit_string_cleanup(struct descriptor_data *d, int terminator) {
    switch (OLC_MODE(d)) {
        case REDIT_DESC:
            redit_disp_menu(d);
            break;
        case REDIT_EXIT_DESCRIPTION:
            redit_disp_exit_menu(d);
            break;
        case REDIT_EXTRADESC_DESCRIPTION:
            redit_disp_extradesc_menu(d);
            break;
    }
}
