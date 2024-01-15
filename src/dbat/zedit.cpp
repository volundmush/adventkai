/************************************************************************
 * OasisOLC - Zones / zedit.c					v2.0	*
 * Copyright 1996 Harvey Gilpin						*
 * Copyright 1997-2001 George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/structs.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/constants.h"
#include "dbat/genolc.h"
#include "dbat/genzon.h"
#include "dbat/oasis.h"
#include "dbat/dg_scripts.h"
#include "dbat/act.informative.h"
#include "dbat/act.wizard.h"
#include "dbat/handler.h"

/*
 * Nasty internal macros to clean up the code.
 */
#define MYCMD        (OLC_ZONE(d)->cmd[subcmd])
#define OLC_CMD(d)    (OLC_ZONE(d)->cmd[OLC_VAL(d)])

/* Prototypes. */
int start_change_command(struct descriptor_data *d, int pos);


/*-------------------------------------------------------------------*/

ACMD(do_oasis_zedit) {
    int number = NOWHERE, save = 0, real_num;
    struct descriptor_data *d;
    char *buf3;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    /****************************************************************************/
    /** Parse any arguments.                                                   **/
    /****************************************************************************/
    buf3 = two_arguments(argument, buf1, buf2);

    /****************************************************************************/
    /** If no argument was given, use the zone the builder is standing in.     **/
    /****************************************************************************/
    if (!*buf1)
        number = GET_ROOM_VNUM(IN_ROOM(ch));
    else if (!isdigit(*buf1)) {
        if (strcasecmp("save", buf1) == 0) {
            save = true;

            if (is_number(buf2))
                number = atoi(buf2);
            else if (GET_OLC_ZONE(ch) > 0) {
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
        } else if (GET_ADMLEVEL(ch) >= ADMLVL_IMPL) {
            if (strcasecmp("new", buf1) || !buf3 || !*buf3)
                send_to_char(ch, "Format: zedit new <zone number> <bottom-room> "
                                 "<upper-room>\r\n");
            else {
                char sbot[MAX_INPUT_LENGTH], stop[MAX_INPUT_LENGTH];
                room_vnum bottom, top;

                skip_spaces(&buf3);
                two_arguments(buf3, sbot, stop);

                number = atoi(buf2);
                if (number < 0)
                    number = NOWHERE;
                bottom = atoi(sbot);
                top = atoi(stop);

                /**********************************************************************/
                /** Setup the new zone (displays the menu to the builder).           **/
                /**********************************************************************/
                zedit_new_zone(ch, number, bottom, top);
            }

            /************************************************************************/
            /** Done now, exit the function.                                       **/
            /************************************************************************/
            return;

        } else {
            send_to_char(ch, "Yikes!  Stop that, someone will get hurt!\r\n");
            return;
        }
    }

    /****************************************************************************/
    /** If a numeric argument was given, retrieve it.                           **/
    /****************************************************************************/
    if (number == NOWHERE)
        number = atoi(buf1);

    /****************************************************************************/
    /** Check that nobody is currently editing this zone.                      **/
    /****************************************************************************/
    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) == CON_ZEDIT) {
            if (d->olc && OLC_NUM(d) == number) {
                send_to_char(ch, "That zone is currently being edited by %s.\r\n",
                             PERS(d->character, ch));
                return;
            }
        }
    }

    /****************************************************************************/
    /** Store the builder's descriptor in d.                                   **/
    /****************************************************************************/
    d = ch->desc;

    /****************************************************************************/
    /** Give the builder's descriptor an OLC structure.                        **/
    /****************************************************************************/
    if (d->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true, "SYSERR: do_oasis_zedit: Player already "
                                         "had olc structure.");
        free(d->olc);
    }

    CREATE(d->olc, struct oasis_olc_data, 1);

    /****************************************************************************/
    /** Find the zone.                                                         **/
    /****************************************************************************/
    OLC_ZNUM(d) = save ? real_zone(number) : real_zone_by_thing(number);
    if (OLC_ZNUM(d) == NOWHERE) {
        send_to_char(ch, "Sorry, there is no zone for that number!\r\n");

        /**************************************************************************/
        /** Free the descriptor's OLC structure.                                 **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /****************************************************************************/
    /** Everyone but IMPLs can only edit zones they have been assigned.        **/
    /****************************************************************************/
    if (!can_edit_zone(ch, OLC_ZNUM(d))) {
        send_cannot_edit(ch, zone_table[OLC_ZNUM(d)].number);
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /****************************************************************************/
    /** If we need to save, then save the zone.                                **/
    /****************************************************************************/
    if (save) {
        send_to_char(ch, "Saving all zone information for zone %d.\r\n",
                     zone_table[OLC_ZNUM(d)].number);
        mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(ch)), true,
               "OLC: %s saves zone information for zone %d.", GET_NAME(ch),
               zone_table[OLC_ZNUM(d)].number);

        /**************************************************************************/
        /** Save the zone information to the zone file.                          **/
        /**************************************************************************/
        save_zone(OLC_ZNUM(d));

        /**************************************************************************/
        /** Free the descriptor's OLC structure.                                 **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    OLC_NUM(d) = number;

    if ((real_num = real_room(number)) == NOWHERE) {
        write_to_output(d, "That room does not exist.\r\n");

        /**************************************************************************/
        /** Free the descriptor's OLC structure.                                 **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    zedit_setup(d, real_num);
    STATE(d) = CON_ZEDIT;

    act("$n starts using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);
    ch->playerFlags.set(PLR_WRITING);

    mudlog(CMP, ADMLVL_IMMORT, true, "OLC: %s starts editing zone %d allowed zone %d",
           GET_NAME(ch), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(ch));
}

void zedit_setup(struct descriptor_data *d, int room_num) {
    int subcmd = 0, count = 0, cmd_room = NOWHERE;

    /*
     * Allocate one scratch zone structure.
     */

    auto &z = zone_table[OLC_ZNUM(d)];
    auto zd = OLC_ZONE(d) = new zone_data();
    /*
     * Copy all the zone header information over.
     */
    zd->name = strdup(z.name);
    if (z.builders)
        zd->builders = strdup(z.builders);
    zd->lifespan = z.lifespan;
    zd->bot = z.bot;
    zd->top = z.top;
    zd->reset_mode = z.reset_mode;
    zd->zone_flags[0] = z.zone_flags[0];
    zd->zone_flags[1] = z.zone_flags[1];
    zd->zone_flags[2] = z.zone_flags[2];
    zd->zone_flags[3] = z.zone_flags[3];
    zd->min_level = z.min_level;
    zd->max_level = z.max_level;
    /*
     * The remaining fields are used as a 'has been modified' flag
     */
    zd->number = 0;    /* Header information has changed.	*/
    zd->age = 0;    /* The commands have changed.		*/

    /*
   * Add all entries in zone_table that relate to this room.
   */
    
    auto filterFunc = [&](struct reset_com& c) {
        switch(c.command) {
            case 'M':
            case 'O':
            case 'T':
            case 'V':
                return room_num == c.arg3;
            case 'D':
            case 'R':
                return room_num == c.arg1;
            default:
                return false;
        }
    };
    
    // Copy all elements of z.cmd that pass filterFunc to zd->cmd.
    std::copy_if(z.cmd.begin(), z.cmd.end(), std::back_inserter(zd->cmd), filterFunc);
    

    
    /*
     * Display main menu.
     */
    zedit_disp_menu(d);
}

/*------------------------------------------------------------------*/
void zedit_disp_flag_menu(struct descriptor_data *d) {
    int counter, columns = 0;
    char bits[MAX_STRING_LENGTH];

    clear_screen(d);
    for (counter = 0; counter < NUM_ZONE_FLAGS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter + 1,
                        zone_bits[counter], !(++columns % 2) ? "\r\n" : "");
    }

    sprintbitarray(OLC_ZONE(d)->zone_flags, zone_bits, ZF_ARRAY_MAX, bits);
    write_to_output(d, "\r\nZone flags: @c%s@n\r\n"
                       "Enter Zone flags, 0 to quit : ", bits);
    OLC_MODE(d) = ZEDIT_ZONE_FLAGS;
}

/*-------------------------------------------------------------------*/

/*
 * Create a new zone.
 */
void zedit_new_zone(struct char_data *ch, zone_vnum vzone_num, room_vnum bottom, room_vnum top) {
    int result;
    const char *error;
    struct descriptor_data *dsc;

    if ((result = create_new_zone(vzone_num, bottom, top, &error)) == NOWHERE) {
        write_to_output(ch->desc, error);
        return;
    }

    for (dsc = descriptor_list; dsc; dsc = dsc->next) {
        switch (STATE(dsc)) {
            case CON_REDIT:
                OLC_ROOM(dsc)->zone += (OLC_ZNUM(dsc) >= result);
                /* Fall through. */
            case CON_ZEDIT:
            case CON_MEDIT:
            case CON_SEDIT:
            case CON_OEDIT:
            case CON_TRIGEDIT:
            case CON_GEDIT:
                OLC_ZNUM(dsc) += (OLC_ZNUM(dsc) >= result);
                break;
            default:
                break;
        }
    }

    zedit_save_to_disk(result); /* save to disk .. */

    mudlog(BRF, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(ch)), true, "OLC: %s creates new zone #%d", GET_NAME(ch), vzone_num);
    write_to_output(ch->desc, "Zone created successfully.\r\n");
}

/*-------------------------------------------------------------------*/

/*
 * Save all the information in the player's temporary buffer back into
 * the current zone table.
 */
void zedit_save_internally(struct descriptor_data *d) {
    int mobloaded = false,
            objloaded = false,
            subcmd;
    room_rnum room_num = real_room(OLC_NUM(d));

    if (room_num == NOWHERE) {
        basic_mud_log("SYSERR: zedit_save_internally: OLC_NUM(d) room %d not found.", OLC_NUM(d));
        return;
    }
    
    auto zd = OLC_ZONE(d);

    remove_room_zone_commands(OLC_ZNUM(d), room_num);
    auto &z = zone_table[OLC_ZNUM(d)];

    auto it = std::remove_if(zd->cmd.begin(), zd->cmd.end(), [&](auto &c) {
        switch (c.command) {
            /* Possible fail cases. */
            case 'G':
            case 'E':
                if (mobloaded)
                    break;
                write_to_output(d, "Equip/Give command not saved since no mob was loaded first.\r\n");
                return true;
            case 'P':
                if (objloaded)
                    break;
                write_to_output(d, "Put command not saved since another object was not loaded first.\r\n");
                return true;
                /* Pass cases. */
            case 'M':
                mobloaded = true;
                break;
            case 'O':
                objloaded = true;
                break;
            default:
                mobloaded = objloaded = false;
                break;
        }
        return false;
    });

    z.cmd.insert(z.cmd.begin(), zd->cmd.begin(), it);
    int line = 1;
    // reorder the lines.
    for(auto c : z.cmd) c.line = line++;


    /*
     * Finally, if zone headers have been changed, copy over
     */
    if (zd->number) {
        free(z.name);
        free(z.builders);

        z.name = strdup(zd->name);
        z.builders = strdup(zd->builders);
        z.bot = zd->bot;
        z.top = zd->top;
        z.reset_mode = zd->reset_mode;
        z.lifespan = zd->lifespan;
        z.zone_flags[0] = zd->zone_flags[0];
        z.zone_flags[1] = zd->zone_flags[1];
        z.zone_flags[2] = zd->zone_flags[2];
        z.zone_flags[3] = zd->zone_flags[3];
        z.min_level = zd->min_level;
        z.max_level = zd->max_level;
    }
    dirty_zones.insert(z.number);
}

/*-------------------------------------------------------------------*/

void zedit_save_to_disk(int zone) {
    save_zone(zone);
}

/*-------------------------------------------------------------------*/

/*
 * Error check user input and then setup change  
 */
int start_change_command(struct descriptor_data *d, int pos) {
    if (pos < 0 || pos >= OLC_ZONE(d)->cmd.size())
        return 0;

    /*
     * Ok, let's get editing.
     */
    OLC_VAL(d) = pos;
    return 1;
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * the main menu 
 */
void zedit_disp_menu(struct descriptor_data *d) {
    int subcmd = 0, room, counter = 0;
    char buf1[MAX_STRING_LENGTH];

    clear_screen(d);
    room = real_room(OLC_NUM(d));
    sprintbitarray(OLC_ZONE(d)->zone_flags, zone_bits, ZF_ARRAY_MAX, buf1);
    auto &z = zone_table[OLC_ZNUM(d)];
    /*
     * Menu header
     */
    send_to_char(d->character,
                 "Room number: [@c%d@n]		Room zone: @c%d\r\n"
                 "@g1@n) Builders       : @y%s\r\n"
                 "@gA@n) Zone name      : @y%s\r\n"
                 "@gL@n) Lifespan       : @y%d minutes\r\n"
                 "@gB@n) Bottom of zone : @y%d\r\n"
                 "@gT@n) Top of zone    : @y%d\r\n"
                 "@gR@n) Reset Mode     : @y%s@n\r\n"
                 "@gF@n) Zone Flags     : @y%s@n\r\n"
                 "@gM@n) Min Level      : @y%d@n\r\n"
                 "@gX@n) Max Level      : @y%d@n\r\n"
                 "@gZ@n) Wiznet         :\r\n"
                 "[Command list]\r\n",

                 OLC_NUM(d),
                 z.number,
                 OLC_ZONE(d)->builders ? OLC_ZONE(d)->builders : "None.",
                 OLC_ZONE(d)->name ? OLC_ZONE(d)->name : "<NONE!>",
                 OLC_ZONE(d)->lifespan,
                 OLC_ZONE(d)->bot,
                 OLC_ZONE(d)->top,
                 OLC_ZONE(d)->reset_mode ? ((OLC_ZONE(d)->reset_mode == 1) ? "Reset when no players are in zone."
                                                                           : "Normal reset.") : "Never reset",
                 buf1, OLC_ZONE(d)->min_level, OLC_ZONE(d)->max_level
    );

    /*
     * Print the commands for this room into display buffer.
     */
    for (auto &c : OLC_ZONE(d)->cmd) {
        /*
         * Translate what the command means.
         */
        write_to_output(d, "@n%d - @y", counter++);
        switch (c.command) {
            case 'M':
                write_to_output(d, "%sLoad %s@y [@c%d@y], Max : %d, MaxR %d, Chance %d",
                                c.if_flag ? " then " : "",
                                mob_proto[c.arg1].short_description,
                                mob_index[c.arg1].vn, c.arg2, c.arg4, c.arg5
                );
                break;
            case 'G':
                write_to_output(d, "%sGive it %s@y [@c%d@y], Max : %d, Chance %d",
                                c.if_flag ? " then " : "",
                                obj_proto[c.arg1].short_description,
                                obj_index[c.arg1].vn,
                                c.arg2, c.arg5
                );
                break;
            case 'O':
                write_to_output(d, "%sLoad %s@y [@c%d@y], Max : %d, MaxR %d, Chance %d",
                                c.if_flag ? " then " : "",
                                obj_proto[c.arg1].short_description,
                                obj_index[c.arg1].vn,
                                c.arg2, c.arg4, c.arg5
                );
                break;
            case 'E':
                write_to_output(d, "%sEquip with %s@y [@c%d@n], %s, Max : %d, Chance %d",
                                c.if_flag ? " then " : "",
                                obj_proto[c.arg1].short_description,
                                obj_index[c.arg1].vn,
                                equipment_types[c.arg3],
                                c.arg2, c.arg5
                );
                break;
            case 'P':
                write_to_output(d, "%sPut %s@y [@c%d@n] in %s [@c%d@n], Max : %d, %% Chance %d",
                                c.if_flag ? " then " : "",
                                obj_proto[c.arg1].short_description,
                                obj_index[c.arg1].vn,
                                obj_proto[c.arg3].short_description,
                                obj_index[c.arg3].vn,
                                c.arg2, c.arg5
                );
                break;
            case 'R':
                write_to_output(d, "%sRemove %s@y [@c%d@n] from room.",
                                c.if_flag ? " then " : "",
                                obj_proto[c.arg2].short_description,
                                obj_index[c.arg2].vn
                );
                break;
            case 'D':
                write_to_output(d, "%sSet door %s@y as %s.",
                                c.if_flag ? " then " : "",
                                dirs[c.arg2],
                                c.arg3 ? ((c.arg3 == 1) ? "closed" : "locked") : "open"
                );
                break;
            case 'T':
                write_to_output(d, "%sAttach trigger @c%s@y [@c%d@y] to %s, %% Chance %d",
                                c.if_flag ? " then " : "",
                                trig_index[c.arg2].proto->name,
                                trig_index[c.arg2].vn,
                                ((c.arg1 == MOB_TRIGGER) ? "mobile" :
                                 ((c.arg1 == OBJ_TRIGGER) ? "object" :
                                  ((c.arg1 == WLD_TRIGGER) ? "room" : "????"))), c.arg5);
                break;
            case 'V':
                write_to_output(d, "%sAssign global %s:%d to %s = %s, %% Chance %d",
                                c.if_flag ? " then " : "",
                                c.sarg1.c_str(), c.arg2,
                                ((c.arg1 == MOB_TRIGGER) ? "mobile" :
                                 ((c.arg1 == OBJ_TRIGGER) ? "object" :
                                  ((c.arg1 == WLD_TRIGGER) ? "room" : "????"))),
                                c.sarg2.c_str(), c.arg5);
                break;
            default:
                write_to_output(d, "<Unknown Command>");
                break;
        }
        write_to_output(d, "\r\n");
        subcmd++;
    }
    /*
     * Finish off menu
     */
    write_to_output(d,
                    "@n%d - <END OF LIST>\r\n"
                    "@gN@n) Insert new command.\r\n"
                    "@gE@n) Edit a command.\r\n"
                    "@gD@n) Delete a command.\r\n"
                    "@gQ@n) Quit\r\nEnter your choice : ", counter);

    OLC_MODE(d) = ZEDIT_MAIN_MENU;
}

/*-------------------------------------------------------------------*/

/*
 * Print the command type menu and setup response catch. 
 */
void zedit_disp_comtype(struct descriptor_data *d) {
    clear_screen(d);
    write_to_output(d,
                    "\r\n"
                    "@gM@n) Load Mobile to room             @gO@n) Load Object to room\r\n"
                    "@gE@n) Equip mobile with object        @gG@n) Give an object to a mobile\r\n"
                    "@gP@n) Put object in another object    @gD@n) Open/Close/Lock a Door\r\n"
                    "@gR@n) Remove an object from the room\r\n"
                    "@gT@n) Assign a trigger                @gV@n) Set a global variable\r\n"
                    "\r\n"
                    "What sort of command will this be? : "
    );
    OLC_MODE(d) = ZEDIT_COMMAND_TYPE;
}

/*-------------------------------------------------------------------*/

/*
 * Print the appropriate message for the command type for arg1 and set
 * up the input catch clause  
 */
void zedit_disp_arg1(struct descriptor_data *d) {
    write_to_output(d, "\r\n");

    switch (OLC_CMD(d).command) {
        case 'M':
            write_to_output(d, "Input mob's vnum : ");
            OLC_MODE(d) = ZEDIT_ARG1;
            break;
        case 'O':
        case 'E':
        case 'P':
        case 'G':
            write_to_output(d, "Input object vnum : ");
            OLC_MODE(d) = ZEDIT_ARG1;
            break;
        case 'D':
        case 'R':
            /*
             * Arg1 for these is the room number, skip to arg2
             */
            OLC_CMD(d).arg1 = real_room(OLC_NUM(d));
            zedit_disp_arg2(d);
            break;
        case 'T':
        case 'V':
            write_to_output(d, "Input trigger type (0:mob, 1:obj, 2:room) :");
            OLC_MODE(d) = ZEDIT_ARG1;
            break;
        default:
            /*
             * We should never get here.
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_disp_arg1(): Help!");
            write_to_output(d, "Oops...\r\n");
            return;
    }
}

/*-------------------------------------------------------------------*/

/*
 * Print the appropriate message for the command type for arg2 and set
 * up the input catch clause.
 */
void zedit_disp_arg2(struct descriptor_data *d) {
    int i;

    switch (OLC_CMD(d).command) {
        case 'M':
        case 'O':
        case 'E':
        case 'P':
        case 'G':
            write_to_output(d, "Input the maximum number that can exist on the mud : ");
            break;
        case 'D':
            for (i = 0; *dirs[i] != '\n'; i++) {
                write_to_output(d, "%d) Exit %s.\r\n", i, dirs[i]);
            }
            write_to_output(d, "Enter exit number for door : ");
            break;
        case 'R':
            write_to_output(d, "Input object's vnum : ");
            break;
        case 'T':
            write_to_output(d, "Enter the trigger VNum : ");
            break;
        case 'V':
            write_to_output(d, "Global's context (0 for none) : ");
            break;
        default:
            /*
             * We should never get here, but just in case...
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_disp_arg2(): Help!");
            write_to_output(d, "Oops...\r\n");
            return;
    }
    OLC_MODE(d) = ZEDIT_ARG2;
}

/*-------------------------------------------------------------------*/

/*
 * Print the appropriate message for the command type for arg3 and set
 * up the input catch clause.
 */
void zedit_disp_arg3(struct descriptor_data *d) {
    int i = 0;

    write_to_output(d, "\r\n");

    switch (OLC_CMD(d).command) {
        case 'E':
            while (*equipment_types[i] != '\n') {
                write_to_output(d, "%2d) %26.26s %2d) %26.26s\r\n", i,
                                equipment_types[i], i + 1, (*equipment_types[i + 1] != '\n') ?
                                                           equipment_types[i + 1] : "");
                if (*equipment_types[i + 1] != '\n')
                    i += 2;
                else
                    break;
            }
            write_to_output(d, "Location to equip : ");
            break;
        case 'P':
            write_to_output(d, "Virtual number of the container : ");
            break;
        case 'D':
            write_to_output(d, "0)  Door open\r\n"
                               "1)  Door closed\r\n"
                               "2)  Door locked\r\n"
                               "Enter state of the door : ");
            break;
        case 'V':
        case 'T':
        case 'M':
        case 'O':
        case 'R':
        case 'G':
        default:
            /*
             * We should never get here, just in case.
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_disp_arg3(): Help!");
            write_to_output(d, "Oops...\r\n");
            return;
    }
    OLC_MODE(d) = ZEDIT_ARG3;
}
/*-------------------------------------------------------------------*/

/*
 * Print the appropriate message for the command type for arg4 and set
 * up the input catch clause.
 */
void zedit_disp_arg4(struct descriptor_data *d) {
    write_to_output(d, "\r\n");

    switch (OLC_CMD(d).command) {
        case 'M':
        case 'O':
            write_to_output(d, "Input the max allowed to load from this room (Pressing enter == 0(ignore)) : ");
            break;
        case 'E':
        case 'P':
        case 'G':
        case 'T':
        case 'V':
            zedit_disp_arg5(d);
            break;
        default:
            /*
             * We should never get here, but just in case...
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_disp_arg4(): Help!");
            write_to_output(d, "Oops...\r\n");
            return;
    }
    OLC_MODE(d) = ZEDIT_ARG4;
}

/*-------------------------------------------------------------------*/

/*
 * Print the appropriate message for the command type for arg5 and set
 * up the input catch clause.
 */
void zedit_disp_arg5(struct descriptor_data *d) {
    write_to_output(d, "\r\n");

    switch (OLC_CMD(d).command) {
        case 'M':
        case 'O':
        case 'E':
        case 'P':
        case 'G':
            write_to_output(d, "Input the percentage chance of the load NOT occurring : ");
            break;
        case 'T':
        case 'V':
        default:
            /*
             * We should never get here, but just in case...
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_disp_arg5(): Help!");
            write_to_output(d, "Oops...\r\n");
            return;
    }
    OLC_MODE(d) = ZEDIT_ARG5;
}

/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/

void zedit_parse(struct descriptor_data *d, char *arg) {
    int pos, i = 0;
    int number;

    switch (OLC_MODE(d)) {
/*-------------------------------------------------------------------*/
        case ZEDIT_CONFIRM_SAVESTRING:
            switch (*arg) {
                case 'y':
                case 'Y':
                    /*
                     * Save the zone in memory, hiding invisible people.
                     */
                    zedit_save_internally(d);
                    if (CONFIG_OLC_SAVE) {
                        write_to_output(d, "Saving zone info to disk.\r\n");
                        zedit_save_to_disk(OLC_ZNUM(d));
                    } else
                        write_to_output(d, "Saving zone info in memory.\r\n");

                    mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(d->character)), true,
                           "OLC: %s edits zone info for room %d.", GET_NAME(d->character), OLC_NUM(d));
                    /* FALL THROUGH */
                case 'n':
                case 'N':
                    cleanup_olc(d, CLEANUP_ALL);
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\n");
                    write_to_output(d, "Do you wish to save your changes? : ");
                    break;
            }
            break;
            /* End of ZEDIT_CONFIRM_SAVESTRING */

/*-------------------------------------------------------------------*/
        case ZEDIT_MAIN_MENU:
            switch (*arg) {
                case 'q':
                case 'Q':
                    if (OLC_ZONE(d)->age || OLC_ZONE(d)->number) {
                        write_to_output(d, "Do you wish to save your changes? : ");
                        OLC_MODE(d) = ZEDIT_CONFIRM_SAVESTRING;
                    } else {
                        write_to_output(d, "No changes made.\r\n");
                        cleanup_olc(d, CLEANUP_ALL);
                    }
                    break;
                case 'n':
                case 'N':
                    /*
                     * New entry.
                     */
                    if (OLC_ZONE(d)->cmd[0].command == 'S') {
                        /* first command */
                        if (new_command(OLC_ZONE(d), 0) && start_change_command(d, 0)) {
                            zedit_disp_comtype(d);
                            OLC_ZONE(d)->age = 1;
                            break;
                        }
                    }
                    write_to_output(d, "What number in the list should the new command be? : ");
                    OLC_MODE(d) = ZEDIT_NEW_ENTRY;
                    break;
                case 'e':
                case 'E':
                    /*
                     * Change an entry.
                     */
                    write_to_output(d, "Which command do you wish to change? : ");
                    OLC_MODE(d) = ZEDIT_CHANGE_ENTRY;
                    break;
                case 'd':
                case 'D':
                    /*
                     * Delete an entry.
                     */
                    write_to_output(d, "Which command do you wish to delete? : ");
                    OLC_MODE(d) = ZEDIT_DELETE_ENTRY;
                    break;
                case 'a':
                case 'A':
                    /*
                     * Edit zone name.
                     */
                    write_to_output(d, "Enter new zone name : ");
                    OLC_MODE(d) = ZEDIT_ZONE_NAME;
                    break;
                case '1':
                    /*
                     * Edit zone builders.
                     * We do not want to allow baby builders to tweak this.
                     */
                    if (GET_ADMLEVEL(d->character) <= ADMLVL_BUILDER) {
                        OLC_MODE(d) = ZEDIT_MAIN_MENU;
                        write_to_output(d, "Access Denied.\r\n");
                    } else {
                        write_to_output(d, "Enter new builders list : ");
                        OLC_MODE(d) = ZEDIT_ZONE_BUILDERS;
                    }
                    break;
                case 'b':
                case 'B':
                    /*
                     * Edit bottom of zone.
                     */
                    if (GET_ADMLEVEL(d->character) < ADMLVL_IMPL)
                        zedit_disp_menu(d);
                    else {
                        write_to_output(d, "Enter new bottom of zone : ");
                        OLC_MODE(d) = ZEDIT_ZONE_BOT;
                    }
                    break;
                case 't':
                case 'T':
                    /*
                     * Edit top of zone.
                     */
                    if (GET_ADMLEVEL(d->character) < ADMLVL_IMPL)
                        zedit_disp_menu(d);
                    else {
                        write_to_output(d, "Enter new top of zone : ");
                        OLC_MODE(d) = ZEDIT_ZONE_TOP;
                    }
                    break;
                case 'l':
                case 'L':
                    /*
                     * Edit zone lifespan.
                     */
                    write_to_output(d, "Enter new zone lifespan : ");
                    OLC_MODE(d) = ZEDIT_ZONE_LIFE;
                    break;
                case 'r':
                case 'R':
                    /*
                     * Edit zone reset mode.
                     */
                    write_to_output(d, "\r\n"
                                       "0) Never reset\r\n"
                                       "1) Reset only when no players in zone\r\n"
                                       "2) Normal reset\r\n"
                                       "Enter new zone reset type : ");
                    OLC_MODE(d) = ZEDIT_ZONE_RESET;
                    break;
                case 'm':
                case 'M':
                    /*** Edit zone min level. ***/
                    write_to_output(d, "Enter Minimum level to enter zone : \r\n");
                    OLC_MODE(d) = ZEDIT_MIN_LEVEL;
                    break;
                case 'f':
                case 'F':
                    zedit_disp_flag_menu(d);
                    break;
                case 'Z':
                case 'z':
                    search_replace(arg, "z ", "");
                    do_wiznet(d->character, arg, 0, 0);
                    break;
                case 'x':
                case 'X':
                    /*** Edit zone max level. ***/
                    write_to_output(d, "Enter Maximum level to enter zone : \r\n");
                    OLC_MODE(d) = ZEDIT_MAX_LEVEL;
                    break;
                default:
                    zedit_disp_menu(d);
                    break;
            }
            break;
            /* End of ZEDIT_MAIN_MENU */

/*-------------------------------------------------------------------*/
        case ZEDIT_NEW_ENTRY:
            /*
             * Get the line number and insert the new line.
             */
            pos = atoi(arg);
            if (isdigit(*arg) && new_command(OLC_ZONE(d), pos)) {
                if (start_change_command(d, pos)) {
                    zedit_disp_comtype(d);
                    OLC_ZONE(d)->age = 1;
                }
            } else
                zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_DELETE_ENTRY:
            /*
             * Get the line number and delete the line.
             */
            pos = atoi(arg);
            if (isdigit(*arg)) {
                delete_zone_command(OLC_ZONE(d), pos);
                OLC_ZONE(d)->age = 1;
            }
            zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_CHANGE_ENTRY:
            /*
             * Parse the input for which line to edit, and goto next quiz.
             */
            /*
             *  Abort edit, and return to main menu
             * - idea from Mark Garringer zizazat@hotmail.com
             */
            if (toupper(*arg) == 'A') {
                if (OLC_CMD(d).command == 'N') {
                    OLC_CMD(d).command = '*';
                }
                zedit_disp_menu(d);
                break;
            }

            pos = atoi(arg);
            if (isdigit(*arg) && start_change_command(d, pos)) {
                zedit_disp_comtype(d);
                OLC_ZONE(d)->age = 1;
            } else
                zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_COMMAND_TYPE:
            /*
             * Parse the input for which type of command this is, and goto next
             * quiz.
             */
            OLC_CMD(d).command = toupper(*arg);
            if (!OLC_CMD(d).command || (strchr("MOPEDGRTV", OLC_CMD(d).command) == nullptr)) {
                write_to_output(d, "Invalid choice, try again : ");
            } else {
                if (OLC_VAL(d)) {    /* If there was a previous command. */
                    if (OLC_CMD(d).command == 'T' || OLC_CMD(d).command == 'V') {
                        OLC_CMD(d).if_flag = 1;
                        zedit_disp_arg1(d);
                    } else {
                        write_to_output(d, "Is this command dependent on the success of the previous one? (y/n)\r\n");
                        OLC_MODE(d) = ZEDIT_IF_FLAG;
                    }
                } else {    /* 'if-flag' not appropriate. */
                    OLC_CMD(d).if_flag = 0;
                    zedit_disp_arg1(d);
                }
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_IF_FLAG:
            /*
             * Parse the input for the if flag, and goto next quiz.
             */
            switch (*arg) {
                case 'y':
                case 'Y':
                    OLC_CMD(d).if_flag = 1;
                    break;
                case 'n':
                case 'N':
                    OLC_CMD(d).if_flag = 0;
                    break;
                default:
                    write_to_output(d, "Try again : ");
                    return;
            }
            zedit_disp_arg1(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ARG1:
            /*
             * Parse the input for arg1, and goto next quiz.
             */
            if (!isdigit(*arg)) {
                write_to_output(d, "Must be a numeric value, try again : ");
                return;
            }
            switch (OLC_CMD(d).command) {
                case 'M':
                    if ((pos = real_mobile(atoi(arg))) != NOBODY) {
                        OLC_CMD(d).arg1 = pos;
                        zedit_disp_arg2(d);
                    } else
                        write_to_output(d, "That mobile does not exist, try again : ");
                    break;
                case 'O':
                case 'P':
                case 'E':
                case 'G':
                    if ((pos = real_object(atoi(arg))) != NOTHING) {
                        OLC_CMD(d).arg1 = pos;
                        zedit_disp_arg2(d);
                    } else
                        write_to_output(d, "That object does not exist, try again : ");
                    break;
                case 'T':
                case 'V':
                    if (atoi(arg) < MOB_TRIGGER || atoi(arg) > WLD_TRIGGER)
                        write_to_output(d, "Invalid input.");
                    else {
                        OLC_CMD(d).arg1 = atoi(arg);
                        zedit_disp_arg2(d);
                    }
                    break;
                case 'D':
                case 'R':
                default:
                    /*
                     * We should never get here.
                     */
                    cleanup_olc(d, CLEANUP_ALL);
                    mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_parse(): case ARG1: Ack!");
                    write_to_output(d, "Oops...\r\n");
                    break;
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ARG2:
            /*
             * Parse the input for arg2, and goto next quiz.
             */
            if (!isdigit(*arg)) {
                write_to_output(d, "Must be a numeric value, try again : ");
                return;
            }
            switch (OLC_CMD(d).command) {
                case 'M':
                case 'O':
                    OLC_CMD(d).arg2 = MIN(MAX_DUPLICATES, atoi(arg));
                    OLC_CMD(d).arg3 = real_room(OLC_NUM(d));
                    zedit_disp_arg4(d);
                    break;
                case 'G':
                    OLC_CMD(d).arg2 = MIN(MAX_DUPLICATES, atoi(arg));
                    zedit_disp_arg5(d);
                    break;
                case 'P':
                case 'E':
                    OLC_CMD(d).arg2 = MIN(MAX_DUPLICATES, atoi(arg));
                    zedit_disp_arg3(d);
                    break;
                case 'V':
                    OLC_CMD(d).arg2 = atoi(arg); /* context */
                    OLC_CMD(d).arg3 = real_room(OLC_NUM(d));
                    write_to_output(d, "Enter the global name : ");
                    OLC_MODE(d) = ZEDIT_SARG1;
                    break;
                case 'T':
                    if (real_trigger(atoi(arg)) != NOTHING) {
                        OLC_CMD(d).arg2 = real_trigger(atoi(arg)); /* trigger */
                        OLC_CMD(d).arg3 = real_room(OLC_NUM(d));
                        zedit_disp_menu(d);
                    } else
                        write_to_output(d, "That trigger does not exist, try again : ");
                    break;
                case 'D':
                    pos = atoi(arg);
                    /*
                     * Count directions.
                     */
                    if (pos < 0 || pos > NUM_OF_DIRS)
                        write_to_output(d, "Try again : ");
                    else {
                        OLC_CMD(d).arg2 = pos;
                        zedit_disp_arg3(d);
                    }
                    break;
                case 'R':
                    if ((pos = real_object(atoi(arg))) != NOTHING) {
                        OLC_CMD(d).arg2 = pos;
                        zedit_disp_menu(d);
                    } else
                        write_to_output(d, "That object does not exist, try again : ");
                    break;
                default:
                    /*
                     * We should never get here, but just in case...
                     */
                    cleanup_olc(d, CLEANUP_ALL);
                    mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_parse(): case ARG2: Ack!");
                    write_to_output(d, "Oops...\r\n");
                    break;
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ARG3:
            /*
             * Parse the input for arg3, and go back to main menu.
             */
            if (!isdigit(*arg)) {
                write_to_output(d, "Must be a numeric value, try again : ");
                return;
            }
            switch (OLC_CMD(d).command) {
                case 'E':
                    pos = atoi(arg);
                    /*
                     * Count number of wear positions.  We could use NUM_WEARS, this is
                     * more reliable.
                     */
                    while (*equipment_types[i] != '\n')
                        i++;
                    if (pos < 0 || pos > i)
                        write_to_output(d, "Try again : ");
                    else {
                        OLC_CMD(d).arg3 = pos;
                        zedit_disp_arg5(d);
                    }
                    break;
                case 'P':
                    if ((pos = real_object(atoi(arg))) != NOTHING) {
                        OLC_CMD(d).arg3 = pos;
                        zedit_disp_arg5(d);
                    } else
                        write_to_output(d, "That object does not exist, try again : ");
                    break;
                case 'D':
                    pos = atoi(arg);
                    if (pos < 0 || pos > 2)
                        write_to_output(d, "Try again : ");
                    else {
                        OLC_CMD(d).arg3 = pos;
                        zedit_disp_menu(d);
                    }
                    break;
                case 'M':
                case 'O':
                case 'G':
                case 'R':
                case 'T':
                case 'V':
                default:
                    /*
                     * We should never get here, but just in case...
                     */
                    cleanup_olc(d, CLEANUP_ALL);
                    mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_parse(): case ARG3: Ack!");
                    write_to_output(d, "Oops...\r\n");
                    break;
            }
            break;
/*-------------------------------------------------------------------*/
        case ZEDIT_ARG4:
            /*
             * Parse the input for arg4, and goto next quiz.
             */
            if (!*arg)
                arg = "0";

            if (!isdigit(*arg)) {
                write_to_output(d, "Must be a numeric value, try again : ");
                return;
            }
            switch (OLC_CMD(d).command) {
                case 'M':
                case 'O':
                    OLC_CMD(d).arg4 = MIN(MAX_FROM_ROOM, atoi(arg));
                    zedit_disp_arg5(d);
                    break;
                case 'G':
                case 'P':
                case 'E':
                case 'V':
                case 'T':
                default:
                    /*
                     * We should never get here, but just in case...
                     */
                    cleanup_olc(d, CLEANUP_ALL);
                    mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_parse(): case ARG4: Ack!");
                    write_to_output(d, "Oops...\r\n");
                    break;
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ARG5:
            /*
             * Parse the input for arg5, and goto next quiz.
             */
            if (!isdigit(*arg)) {
                write_to_output(d, "Must be a numeric value, try again : ");
                return;
            }
            if (atoi(arg) < 0 || atoi(arg) > 100) {
                write_to_output(d, "Value must be from 0 to 100");
                return;
            }
            switch (OLC_CMD(d).command) {
                case 'M':
                case 'O':
                case 'G':
                case 'P':
                case 'E':
                    OLC_CMD(d).arg5 = atoi(arg);
                    zedit_disp_menu(d);
                    break;
                case 'V':
                case 'T':
                default:
                    /*
                     * We should never get here, but just in case...
                     */
                    cleanup_olc(d, CLEANUP_ALL);
                    mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_parse(): case ARG5: Ack!");
                    write_to_output(d, "Oops...\r\n");
                    break;
            }
            break;


/*-------------------------------------------------------------------*/
        case ZEDIT_SARG1:
            if (strlen(arg)) {
                OLC_CMD(d).sarg1 = arg;
                OLC_MODE(d) = ZEDIT_SARG2;
                write_to_output(d, "Enter the global value : ");
            } else
                write_to_output(d, "Must have some name to assign : ");
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_SARG2:
            if (strlen(arg)) {
                OLC_CMD(d).sarg2 = arg;
                zedit_disp_arg4(d);
            } else
                write_to_output(d, "Must have some value to set it to :");
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_NAME:
            /*
             * Add new name and return to main menu.
             */
            if (genolc_checkstring(d, arg)) {
                if (OLC_ZONE(d)->name)
                    free(OLC_ZONE(d)->name);
                else
                    basic_mud_log("SYSERR: OLC: ZEDIT_ZONE_NAME: no name to free!");
                OLC_ZONE(d)->name = strdup(arg);
                OLC_ZONE(d)->number = 1;
            }
            zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_BUILDERS:
            /*
             * Add new builders list and return to main menu.
             */
            if (genolc_checkstring(d, arg)) {
                if (OLC_ZONE(d)->builders)
                    free(OLC_ZONE(d)->builders);
                else
                    basic_mud_log("SYSERR: OLC: ZEDIT_ZONE_BUILDERS: no builders list to free!");
                OLC_ZONE(d)->builders = strdup(arg);
                OLC_ZONE(d)->number = 1;
            }
            zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_RESET:
            /*
             * Parse and add new reset_mode and return to main menu.
             */
            pos = atoi(arg);
            if (!isdigit(*arg) || pos < 0 || pos > 2)
                write_to_output(d, "Try again (0-2) : ");
            else {
                OLC_ZONE(d)->reset_mode = pos;
                OLC_ZONE(d)->number = 1;
                zedit_disp_menu(d);
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_LIFE:
            /*
             * Parse and add new lifespan and return to main menu.
             */
            pos = atoi(arg);
            if (!isdigit(*arg) || pos < 0 || pos > 240)
                write_to_output(d, "Try again (0-240) : ");
            else {
                OLC_ZONE(d)->lifespan = pos;
                OLC_ZONE(d)->number = 1;
                zedit_disp_menu(d);
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_BOT:
            /*
             * Parse and add new bottom room in zone and return to main menu.
             */
            if (OLC_ZNUM(d) == 0)
                OLC_ZONE(d)->bot = LIMIT(atoi(arg), 0, OLC_ZONE(d)->top);
            else
                OLC_ZONE(d)->bot = LIMIT(atoi(arg), zone_table[OLC_ZNUM(d) - 1].top + 1, OLC_ZONE(d)->top);
            OLC_ZONE(d)->number = 1;
            zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_TOP:
            /*
             * Parse and add new top room in zone and return to main menu.
             */
            OLC_ZONE(d)->top = LIMIT(atoi(arg), genolc_zonep_bottom(OLC_ZONE(d)),
                                     genolc_zone_bottom(OLC_ZNUM(d) + 1) - 1);
            OLC_ZONE(d)->number = 1;
            zedit_disp_menu(d);
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_ZONE_FLAGS:
            number = atoi(arg);
            if (number < 0 || number > NUM_ZONE_FLAGS) {
                write_to_output(d, "That is not a valid choice!\r\n");
                zedit_disp_flag_menu(d);
            } else if (number == 0) {
                zedit_disp_menu(d);
                break;
            } else {
                /*
                 * Toggle the bit.
                 */
                TOGGLE_BIT_AR(OLC_ZONE(d)->zone_flags, number - 1);
                OLC_ZONE(d)->number = 1;
                zedit_disp_flag_menu(d);
            }
            return;

/*-------------------------------------------------------------------*/
        case ZEDIT_MIN_LEVEL:
            /* Parse new min level of zone. */
            pos = atoi(arg);
            if (!isdigit(*arg) || pos < 0 || pos > CONFIG_LEVEL_CAP)
                write_to_output(d, "Try again (0 - CONFIG_LEVEL_CAP) : \r\n");
            else {
                OLC_ZONE(d)->min_level = pos;
                OLC_ZONE(d)->number = 1;
                zedit_disp_menu(d);
            }
            break;

/*-------------------------------------------------------------------*/
        case ZEDIT_MAX_LEVEL:
            /* Parse new max level of zone. */
            pos = atoi(arg);
            if (!isdigit(*arg) || pos < 0 || pos > CONFIG_LEVEL_CAP)
                write_to_output(d, "Try again (0 - CONFIG_LEVEL_CAP) : \r\n");
            else if (OLC_ZONE(d)->min_level > pos)
                write_to_output(d, "Max level can not be lower the Min level! :\r\n");
            else {
                OLC_ZONE(d)->max_level = pos;
                OLC_ZONE(d)->number = 1;
                zedit_disp_menu(d);
            }
            break;

        default:
            /*
             * We should never get here, but just in case...
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: zedit_parse(): Reached default case!");
            write_to_output(d, "Oops...\r\n");
            break;
    }
}

/******************************************************************************/
/** End of parse_zedit()                                                     **/
/******************************************************************************/
