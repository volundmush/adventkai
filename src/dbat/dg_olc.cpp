/**************************************************************************
*  File: dg_olc.c                                                         *
*                                                                         *
*  Usage: this source file is used in extending Oasis style OLC for       *
*  dg-scripts onto a CircleMUD that already has dg-scripts (as released   *
*  by Mark Heilpern on 1/1/98) implemented.                               *
*                                                                         *
*  Parts of this file by Chris Jacobson of _Aliens vs Predator: The MUD_  *
*                                                                         *
*  $Author: Chris Jacobsen/Mark A. Heilpern/egreen/Welcor $               *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
**************************************************************************/

#include "dbat/dg_olc.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/genolc.h"
#include "dbat/interpreter.h"
#include "dbat/oasis.h"
#include "dbat/dg_event.h"
#include "dbat/constants.h"
#include "dbat/act.wizard.h"
#include "dbat/modify.h"

/* local functions */
static void trigedit_disp_menu(struct descriptor_data *d);

static void trigedit_disp_types(struct descriptor_data *d);


/* ***********************************************************************
 * trigedit
 * ***********************************************************************/

ACMD(do_oasis_trigedit) {
    int number, real_num;
    struct descriptor_data *d;

    /*
     * Parse any arguments.
     */
    skip_spaces(&argument);
    if (!*argument || !isdigit(*argument)) {
        send_to_char(ch, "Specify a trigger VNUM to edit.\r\n");
        return;
    }

    number = atoi(argument);

    /*
     * Check that it isn't already being edited.
     */
    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) == CON_TRIGEDIT) {
            if (d->olc && OLC_NUM(d) == number) {
                send_to_char(ch, "That trigger is currently being edited by %s.\r\n",
                             GET_NAME(d->character));
                return;
            }
        }
    }
    d = ch->desc;
    /*
     * Give descriptor an OLC structure.
     */
    if (d->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true,
               "SYSERR: do_oasis_trigedit: Player already had olc structure.");
        free(d->olc);
    }
    CREATE(d->olc, struct oasis_olc_data, 1);

    /*
     * Find the zone.
     */
    if ((OLC_ZNUM(d) = real_zone_by_thing(number)) == NOWHERE) {
        send_to_char(ch, "Sorry, there is no zone for that number!\r\n");
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /*
     * Everyone but IMPLs can only edit zones they have been assigned.
     */
    if (!can_edit_zone(ch, OLC_ZNUM(d))) {
        send_cannot_edit(ch, zone_table[OLC_ZNUM(d)].number);
        free(d->olc);
        d->olc = nullptr;
        return;
    }
    OLC_NUM(d) = number;

    /*
     *  If this is a new trigger, setup a new one,
     *  otherwise, setup the a copy of the existing trigger
     */
    if ((real_num = real_trigger(number)) == NOTHING)
        trigedit_setup_new(d);
    else
        trigedit_setup_existing(d, real_num);
    int disp = 0;
    if (disp == 0) {
        trigedit_disp_menu(d);
        STATE(d) = CON_TRIGEDIT;
        disp = 1;
    }

    act("$n starts using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);
    ch->playerFlags.set(PLR_WRITING);

    mudlog(CMP, ADMLVL_IMMORT, true, "OLC: %s starts editing zone %d [trigger](allowed zone %d)",
           GET_NAME(ch), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(ch));
}

/* called when a mob or object is being saved to disk, so its script can */
/* be saved */
void script_save_to_disk(FILE *fp, struct unit_data *item, int type) {

    for (auto p : item->proto_script) {
        fprintf(fp, "T %d\n", p);
    }
}

void trigedit_setup_new(struct descriptor_data *d) {
    auto trig = new trig_data();

    trig->vn = NOWHERE;

    /*
     * Set up some defaults
     */
    trig->name = strdup("new trigger");
    trig->trigger_type = MTRIG_GREET;

    /* cmdlist will be a large char string until the trigger is saved */
    CREATE(OLC_STORAGE(d), char, MAX_CMD_LENGTH);
    strncpy(OLC_STORAGE(d),
            "%echo% This trigger commandlist is not complete!\r\n", MAX_CMD_LENGTH - 1);
    trig->narg = 100;

    OLC_TRIG(d) = trig;
    OLC_VAL(d) = 0;  /* Has changed flag. (It hasn't so far, we just made it.) */
}

void trigedit_setup_existing(struct descriptor_data *d, int rtrg_num) {
    struct trig_data *trig;
    struct cmdlist_element *c;
    /*
     * Allocate a scratch trigger structure
     */
    CREATE(trig, struct trig_data, 1);

    trig_data_copy(trig, trig_index[rtrg_num].proto);

    /* convert cmdlist to a char string */
    c = trig->cmdlist;
    CREATE(OLC_STORAGE(d), char, MAX_CMD_LENGTH);
    strcpy(OLC_STORAGE(d), "");

    while (c) {
        strcat(OLC_STORAGE(d), c->cmd);
        strcat(OLC_STORAGE(d), "\r\n");
        c = c->next;
    }
    /* now trig->cmdlist is something to pass to the text editor */
    /* it will be converted back to a real cmdlist_element list later */

    OLC_TRIG(d) = trig;
    OLC_VAL(d) = 0;  /* Has changed flag. (It hasn't so far, we just made it.) */

    /*trigedit_disp_menu(d);*/
}


static void trigedit_disp_menu(struct descriptor_data *d) {
    struct trig_data *trig = OLC_TRIG(d);
    char *attach_type;
    char trgtypes[256];

    if (trig->attach_type == OBJ_TRIGGER) {
        attach_type = "Objects";
        sprintbit(GET_TRIG_TYPE(trig), otrig_types, trgtypes, sizeof(trgtypes));
    } else if (trig->attach_type == WLD_TRIGGER) {
        attach_type = "Rooms";
        sprintbit(GET_TRIG_TYPE(trig), wtrig_types, trgtypes, sizeof(trgtypes));
    } else {
        attach_type = "Mobiles";
        sprintbit(GET_TRIG_TYPE(trig), trig_types, trgtypes, sizeof(trgtypes));
    }

    clear_screen(d);

    write_to_output(d,
                    "Trigger Editor [@c%d@n]\r\n\r\n"
                    "@g1@n) Name         : @y%s\r\n"
                    "@g2@n) Intended for : @y%s\r\n"
                    "@g3@n) Trigger types: @y%s\r\n"
                    "@g4@n) Numeric Arg  : @y%d\r\n"
                    "@g5@n) Arguments    : @y%s\r\n"
                    "@g6@n) Commands:\r\n@c%s\r\n"
                    "@gW@n) Copy Trigger\r\n"
                    "@gZ@n) Wiznet\r\n"
                    "@gQ@n) Quit\r\n"
                    "Enter Choice :",

                    OLC_NUM(d),                /* vnum on the title line */
                    GET_TRIG_NAME(trig),            /* name                   */
                    attach_type,                /* attach type            */
                    trgtypes,                /* greet/drop/etc         */
                    trig->narg,                /* numeric arg            */
                    trig->arglist ? trig->arglist : "",    /* strict arg             */
                    OLC_STORAGE(d));            /* the command list       */

    OLC_MODE(d) = TRIGEDIT_MAIN_MENU;
}

static void trigedit_disp_types(struct descriptor_data *d) {
    int i, columns = 0;
    const char **types;
    char bitbuf[MAX_STRING_LENGTH];

    switch (OLC_TRIG(d)->attach_type) {
        case WLD_TRIGGER:
            types = wtrig_types;
            break;
        case OBJ_TRIGGER:
            types = otrig_types;
            break;
        case MOB_TRIGGER:
        default:
            types = trig_types;
            break;
    }

    clear_screen(d);

    for (i = 0; i < NUM_TRIG_TYPE_FLAGS; i++) {
        write_to_output(d, "@g%2d@n) %-20.20s  %s", i + 1, types[i],
                        !(++columns % 2) ? "\r\n" : "");
    }
    sprintbit(GET_TRIG_TYPE(OLC_TRIG(d)), types, bitbuf, sizeof(bitbuf));
    write_to_output(d, "\r\nCurrent types : @c%s@n\r\nEnter type (0 to quit) : ", bitbuf);

}

void trigedit_parse(struct descriptor_data *d, char *arg) {
    int i = 0;

    switch (OLC_MODE(d)) {
        case TRIGEDIT_MAIN_MENU:
            switch (tolower(*arg)) {
                case 'q':
                    if (OLC_VAL(d)) { /* Anything been changed? */
                        if (!GET_TRIG_TYPE(OLC_TRIG(d))) {
                            write_to_output(d, "Invalid Trigger Type! Answer a to abort quit!\r\n");
                        }
                        write_to_output(d, "Do you wish to save your changes? : ");
                        OLC_MODE(d) = TRIGEDIT_CONFIRM_SAVESTRING;
                    } else
                        cleanup_olc(d, CLEANUP_ALL);
                    return;
                case '1':
                    OLC_MODE(d) = TRIGEDIT_NAME;
                    write_to_output(d, "Name: ");
                    break;
                case '2':
                    OLC_MODE(d) = TRIGEDIT_INTENDED;
                    write_to_output(d, "0: Mobiles, 1: Objects, 2: Rooms: ");
                    break;
                case '3':
                    OLC_MODE(d) = TRIGEDIT_TYPES;
                    trigedit_disp_types(d);
                    break;
                case '4':
                    OLC_MODE(d) = TRIGEDIT_NARG;
                    write_to_output(d, "Numeric argument: ");
                    break;
                case '5':
                    OLC_MODE(d) = TRIGEDIT_ARGUMENT;
                    write_to_output(d, "Argument: ");
                    break;
                case '6':
                    OLC_MODE(d) = TRIGEDIT_COMMANDS;
                    write_to_output(d, "Enter trigger commands: (/s saves /h for help)\r\n\r\n");
                    d->backstr = nullptr;
                    if (OLC_STORAGE(d)) {
                        write_to_output(d, "%s", OLC_STORAGE(d));
                        d->backstr = strdup(OLC_STORAGE(d));
                    }
                    d->str = &OLC_STORAGE(d);
                    d->max_str = MAX_CMD_LENGTH;
                    d->mail_to = 0;
                    OLC_VAL(d) = 1;

                    break;
                case 'w':
                case 'W':
                    write_to_output(d, "Copy what trigger? ");
                    OLC_MODE(d) = TRIGEDIT_COPY;
                    break;
                case 'Z':
                case 'z':
                    search_replace(arg, "z ", "");
                    do_wiznet(d->character, arg, 0, 0);
                    break;
                default:
                    trigedit_disp_menu(d);
                    return;
            }
            return;

        case TRIGEDIT_CONFIRM_SAVESTRING:
            switch (tolower(*arg)) {
                case 'y':
                    trigedit_save(d);
                    mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(d->character)), true,
                           "OLC: %s edits trigger %d", GET_NAME(d->character),
                           OLC_NUM(d));
                    /* fall through */
                case 'n':
                    cleanup_olc(d, CLEANUP_ALL);
                    return;
                case 'a': /* abort quitting */
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\n");
                    write_to_output(d, "Do you wish to save the trigger? : ");
                    return;
            }
            break;

        case TRIGEDIT_NAME:
            smash_tilde(arg);
            if (OLC_TRIG(d)->name)
                free(OLC_TRIG(d)->name);
            OLC_TRIG(d)->name = strdup((arg && *arg) ? arg : "undefined");
            OLC_VAL(d)++;
            break;

        case TRIGEDIT_INTENDED:
            if ((atoi(arg) >= MOB_TRIGGER) || (atoi(arg) <= WLD_TRIGGER))
                OLC_TRIG(d)->attach_type = atoi(arg);
            OLC_VAL(d)++;
            break;

        case TRIGEDIT_NARG:
            OLC_TRIG(d)->narg = LIMIT(atoi(arg), 0, 100);
            OLC_VAL(d)++;
            break;

        case TRIGEDIT_ARGUMENT:
            smash_tilde(arg);
            OLC_TRIG(d)->arglist = (*arg ? strdup(arg) : nullptr);
            OLC_VAL(d)++;
            break;

        case TRIGEDIT_TYPES:
            if ((i = atoi(arg)) == 0)
                break;
            else if (!((i < 0) || (i > NUM_TRIG_TYPE_FLAGS)))
                TOGGLE_BIT((GET_TRIG_TYPE(OLC_TRIG(d))), 1 << (i - 1));
            OLC_VAL(d)++;
            trigedit_disp_types(d);
            return;

        case TRIGEDIT_COMMANDS:
            break;

        case TRIGEDIT_COPY:
            if ((i = real_trigger(atoi(arg))) != NOTHING) {
                trigedit_setup_existing(d, i);
            } else
                write_to_output(d, "That trigger does not exist.\r\n");
            break;
    }

    OLC_MODE(d) = TRIGEDIT_MAIN_MENU;
    trigedit_disp_menu(d);
}


/* save the zone's triggers to internal memory and to disk */
void trigedit_save(struct descriptor_data *d) {
    int i;
    trig_rnum rnum = OLC_NUM(d);
    int found = 0;
    char *s;
    trig_data *proto;
    trig_data *trig = OLC_TRIG(d);
    trig_data *live_trig;
    struct cmdlist_element *cmd, *next_cmd;
    struct index_data **new_index;
    struct descriptor_data *dsc;
    FILE *trig_file;
    int zone, top;
    char buf[MAX_CMD_LENGTH];
    char bitBuf[MAX_INPUT_LENGTH];
    char fname[MAX_INPUT_LENGTH];

    auto exists = trig_index.count(OLC_NUM(d));
    auto &t = trig_index[OLC_NUM(d)];
    if (exists) {
        proto = t.proto;
        for (cmd = proto->cmdlist; cmd; cmd = next_cmd) {
            next_cmd = cmd->next;
            if (cmd->cmd)
                free(cmd->cmd);
            free(cmd);
        }

        free(proto->arglist);
        free(proto->name);

        /* Recompile the command list from the new script */
        s = OLC_STORAGE(d);

        CREATE(trig->cmdlist, struct cmdlist_element, 1);
        if (s) {
            char *t = strtok(s, "\r\n"); /* strtok returns nullptr if s is "\r\n" */
            if (t)
                trig->cmdlist->cmd = strdup(t);
            else
                trig->cmdlist->cmd = strdup("* No script");

            cmd = trig->cmdlist;
            while ((s = strtok(nullptr, "\r\n"))) {
                CREATE(cmd->next, struct cmdlist_element, 1);
                cmd = cmd->next;
                cmd->cmd = strdup(s);
            }
        } else
            trig->cmdlist->cmd = strdup("* No Script");

        /* make the prorotype look like what we have */
        trig_data_copy(proto, trig);

        /* go through the mud and replace existing triggers         */
        for (live_trig = trigger_list; live_trig; live_trig = live_trig->next_in_world) {
            if(live_trig->vn != rnum) continue;
            if (live_trig->arglist) {
                free(live_trig->arglist);
                live_trig->arglist = nullptr;
            }
            if (live_trig->name) {
                free(live_trig->name);
                live_trig->name = nullptr;
            }

            if (proto->arglist)
                live_trig->arglist = strdup(proto->arglist);
            if (proto->name)
                live_trig->name = strdup(proto->name);

            /* anything could have happened so we don't want to keep these */
            triggers_waiting.erase(live_trig);

            if (live_trig->var_list) {
                free_varlist(live_trig->var_list);
                live_trig->var_list = nullptr;
            }

            live_trig->cmdlist = proto->cmdlist;
            live_trig->curr_state = live_trig->cmdlist;
            live_trig->trigger_type = proto->trigger_type;
            live_trig->attach_type = proto->attach_type;
            live_trig->narg = proto->narg;
            live_trig->data_type = proto->data_type;
            live_trig->depth = 0;
        }
    } else {
        {
            t.vn = OLC_NUM(d);
            t.proto = new trig_data();
            trig_data_copy(t.proto, trig);
        }

    }
    dirty_dgscript_prototypes.insert(t.vn);
    write_to_output(d, "Trigger saved to disk.\r\n");
    create_world_index(zone, "trg");
}

void dg_olc_script_copy(struct descriptor_data *d) {
    if (OLC_ITEM_TYPE(d) == MOB_TRIGGER)
        OLC_SCRIPT(d) = OLC_MOB(d)->proto_script;
    else if (OLC_ITEM_TYPE(d) == OBJ_TRIGGER)
        OLC_SCRIPT(d) = OLC_OBJ(d)->proto_script;
    else OLC_SCRIPT(d) = OLC_ROOM(d)->proto_script;
}

void dg_script_menu(struct descriptor_data *d) {
    int i = 0;

    /* make sure our input parser gets used */
    OLC_MODE(d) = OLC_SCRIPT_EDIT;
    OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;

    clear_screen(d);
    write_to_output(d, "     Script Editor\r\n\r\n     Trigger List:\r\n");

    auto editscript = OLC_SCRIPT(d);

    for (auto p : OLC_SCRIPT(d)) {
        auto t = trig_index.find(p);
        if(t == trig_index.end()) continue;
        write_to_output(d, "     %2d) [@c%d@n] @c%s@n", ++i, p,
                        t->second.proto->name);
        if (t->second.proto->attach_type != OLC_ITEM_TYPE(d))
            write_to_output(d, "   @g** Mis-matched Trigger Type **@n\r\n");
        else
            write_to_output(d, "\r\n");
    }
    if (i == 0)
        write_to_output(d, "     <none>\r\n");

    write_to_output(d, "\r\n"
                       " @gN@n)  New trigger for this script\r\n"
                       " @gD@n)  Delete a trigger in this script\r\n"
                       " @gX@n)  Exit Script Editor\r\n\r\n"
                       "     Enter choice :");
}

int dg_script_edit_parse(struct descriptor_data *d, char *arg) {
    struct trig_proto_list *trig, *currtrig;
    int count, pos, vnum;

    auto tfind = std::find(OLC_SCRIPT(d).begin(), OLC_SCRIPT(d).end(), -1);

    switch (OLC_SCRIPT_EDIT_MODE(d)) {
        case SCRIPT_MAIN_MENU:
            switch (tolower(*arg)) {
                case 'x':
                    /* this was buggy.
                       First we created a copy of a thing, but maintained pointers to scripts,
                       then if we altered the scripts, we freed the pointers and added new ones
                       to the OLC_THING. If we then chose _NOT_ to save the changes, the
                       pointers in the original thing pointed to garbage. If we saved changes
                       the pointers were updated correctly.

                       Solution:
                       Here we just point the working copies to the new proto_scripts
                       We only update the original when choosing to save internally,
                       then free the unused memory there.

                       Welcor

                       Thanks to
                       Jeremy Stanley - fungi@yuggoth.org and
                       Torgny Bjers - artovil@arcanerealms.org
                       for the bug report.

                       After updating to OasisOLC 2.0.3 I discovered some malfunctions
                       in this code, so I restructured it a bit. Now things work like this:
                       OLC_SCRIPT(d) is assigned a copy of the edited things' proto_script.
                       OLC_OBJ(d), etc.. are initalized with proto_script = nullptr;
                       On save, the saved copy is updated with OLC_SCRIPT(d) as new proto_script (freeing the old one).
                       On quit/nosave, OLC_SCRIPT is free()'d, and the prototype not touched.

                     */
                    return 0;
                case 'n':
                    write_to_output(d, "\r\nPlease enter position, vnum   (ex: 1, 200):");
                    OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_NEW_TRIGGER;
                    break;
                case 'd':
                    write_to_output(d, "     Which entry should be deleted?  0 to abort :");
                    OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_DEL_TRIGGER;
                    break;
                default:
                    dg_script_menu(d);
                    break;
            }
            return 1;

        case SCRIPT_NEW_TRIGGER:
            vnum = -1;
            count = sscanf(arg, "%d, %d", &pos, &vnum);
            if (count == 1) {
                vnum = pos;
                pos = 999;
            }

            if (pos <= 0) break; /* this aborts a new trigger entry */

            if (vnum == 0) break; /* this aborts a new trigger entry */

            if (real_trigger(vnum) == NOTHING) {
                write_to_output(d, "Invalid Trigger VNUM!\r\n"
                                   "Please enter position, vnum   (ex: 1, 200):");
                return 1;
            }

            /* add the new info in position */
            OLC_SCRIPT(d).emplace_back(vnum);
            break;

        case SCRIPT_DEL_TRIGGER:
            pos = atoi(arg);
            if (pos <= 0) break;
            tfind = OLC_SCRIPT(d).begin() + pos;
            if(tfind != OLC_SCRIPT(d).end()) OLC_SCRIPT(d).erase(tfind);
            break;
    }

    dg_script_menu(d);
    return 1;
}

void trigedit_string_cleanup(struct descriptor_data *d, int terminator) {
    switch (OLC_MODE(d)) {
        case TRIGEDIT_COMMANDS:
            trigedit_disp_menu(d);
            break;
    }
}

#if 0 /* change to 1 if you get messages telling you you don't have strncasecmp() */
int strncasecmp (const char *s1, const char *s2, int n)
{
    unsigned char c1, c2;
    while(*s1 && *s2 && n--) {
        c1 = ((*s1 >= 'A') && (*s1 <= 'Z')) ? (*s1++) + ('a' - 'A') : (*s1++);
        c2 = ((*s2 >= 'A') && (*s2 <= 'Z')) ? (*s2++) + ('a' - 'A') : (*s2++);
        if (c1 != c2)
            return (c1 > c2) ? 1 : -1;
    }
    if (*s1 && !*s2)
        return 1;
    if (!*s1 && *s2)
        return -1;
    return 0;
}
#endif

int format_script(struct descriptor_data *d) {
    char nsc[MAX_CMD_LENGTH], *t, line[READ_SIZE];
    char *sc;
    size_t len = 0, nlen = 0, llen = 0;
    int indent = 0, indent_next = false, found_case = false, i, line_num = 0;

    if (!d->str || !*d->str)
        return false;

    sc = strdup(*d->str); /* we work on a copy, because of strtok() */
    t = strtok(sc, "\r\n");
    *nsc = '\0';

    while (t) {
        line_num++;
        skip_spaces(&t);
        if (!strncasecmp(t, "if ", 3) ||
            !strncasecmp(t, "switch ", 7)) {
            indent_next = true;
        } else if (!strncasecmp(t, "while ", 6)) {
            found_case = true;  /* so you can 'break' a loop without complains */
            indent_next = true;
        } else if (!strncasecmp(t, "end", 3) ||
                   !strncasecmp(t, "done", 4)) {
            if (!indent) {
                write_to_output(d, "Unmatched 'end' or 'done' (line %d)!\r\n", line_num);
                free(sc);
                return false;
            }
            indent--;
            indent_next = false;
        } else if (!strncasecmp(t, "else", 4)) {
            if (!indent) {
                write_to_output(d, "Unmatched 'else' (line %d)!\r\n", line_num);
                free(sc);
                return false;
            }
            indent--;
            indent_next = true;
        } else if (!strncasecmp(t, "case", 4) ||
                   !strncasecmp(t, "default", 7)) {
            if (!indent) {
                write_to_output(d, "Case/default outside switch (line %d)!\r\n", line_num);
                free(sc);
                return false;
            }
            if (!found_case) /* so we don't indent multiple case statements without a break */
                indent_next = true;
            found_case = true;
        } else if (!strncasecmp(t, "break", 5)) {
            if (!found_case || !indent) {
                write_to_output(d, "Break not in case (line %d)!\r\n", line_num);
                free(sc);
                return false;
            }
            found_case = false;
            indent--;
        }

        *line = '\0';
        for (nlen = 0, i = 0; i < indent; i++) {
            strncat(line, "  ", sizeof(line) - 1);
            nlen += 2;
        }
        llen = snprintf(line + nlen, sizeof(line) - nlen, "%s\r\n", t);
        if (llen < 0 || llen + nlen + len > d->max_str - 1) {
            write_to_output(d, "String too long, formatting aborted\r\n");
            free(sc);
            return false;
        }
        len = len + nlen + llen;
        strcat(nsc, line);  /* strcat OK, size checked above */

        if (indent_next) {
            indent++;
            indent_next = false;
        }
        t = strtok(nullptr, "\r\n");
    }

    if (indent)
        write_to_output(d, "Unmatched if, while or switch ignored.\r\n");

    free(*d->str);
    *d->str = strdup(nsc);
    free(sc);

    return true;
}
