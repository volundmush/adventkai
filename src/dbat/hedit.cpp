/**************************************************************************
*  File: hedit.c                                        Part of CircleMUD *
*  Usage: Oasis OLC Help Editor.                                          *
* Author: Steve Wolfe, Scott Meisenholder, Rhade                          *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 2007 by Rhade                                            *
*  InfoTechMUD is based on CircleMUD, Copyright (C) 1993, 1994.           *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "dbat/hedit.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/commands.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/boards.h"
#include "dbat/oasis.h"
#include "dbat/genolc.h"
#include "dbat/improved-edit.h"
#include "dbat/config.h"
#include "dbat/dg_comm.h"
#include "dbat/act.informative.h"

/* external functions */

/* local functions */

static void hedit_disp_menu(struct descriptor_data *);

ACMD(do_oasis_hedit) {
    char arg[MAX_INPUT_LENGTH];
    struct descriptor_data *d;

    /* No building as a mob or while being forced. */
    if (IS_NPC(ch) || !ch->desc || STATE(ch->desc) != CON_PLAYING)
        return;

    if (HEDITS == true) {
        send_to_char(ch, "Sorry, only one person can edit help files at a time.\r\n");
        return;
    }

    if (GET_ADMLEVEL(ch) < 4 && (!strcasecmp("Tepsih", GET_NAME(ch)) && !strcasecmp("Rogoshen", GET_NAME(ch)))) {
        send_to_char(ch, "Sorry you are incapable of editing help files at this time.\r\n");
        return;
    }

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char(ch, "Please specify a help entry to edit.\r\n");
        return;
    }

    d = ch->desc;

    if (!strcasecmp("save", arg)) {
        mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(ch)), true, "OLC: %s saves help files.",
               GET_NAME(ch));
        hedit_save_to_disk(d);
        send_to_char(ch, "Saving help files.\r\n");
        return;
    }

    /* Give descriptor an OLC structure. */
    if (d->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true, "SYSERR: do_oasis: Player already had olc structure.");
        free(d->olc);
    }

    CREATE(d->olc, struct oasis_olc_data, 1);
    OLC_NUM(d) = 0;
    OLC_STORAGE(d) = strdup(arg);
    OLC_ZNUM(d) = search_help((const char *) OLC_STORAGE(d), ADMLVL_IMPL);

    if (OLC_ZNUM(d) == NOWHERE) {
        send_to_char(ch, "Do you wish to add the '%s' help file? ", OLC_STORAGE(d));
        OLC_MODE(d) = HEDIT_CONFIRM_ADD;
    } else {
        send_to_char(ch, "Do you wish to edit the '%s' help file? ", help_table[OLC_ZNUM(d)].keywords);
        OLC_MODE(d) = HEDIT_CONFIRM_EDIT;
    }

    STATE(d) = CON_HEDIT;
    /*send_to_room(IN_ROOM(ch), "%s starts using OLC.\r\n", GET_NAME(ch));*/
    act("$n starts using OLC.", true, ch, nullptr, nullptr, TO_ROOM);
    HEDITS = true;
    ch->playerFlags.set(PLR_WRITING);
    mudlog(CMP, ADMLVL_IMMORT, true, "OLC: %s starts editing help files.", GET_NAME(ch));
}

static void hedit_setup_new(struct descriptor_data *d) {
    CREATE(OLC_HELP(d), struct help_index_element, 1);

    char buf3[MAX_INPUT_LENGTH];
    sprintf(buf3, "<<X<< Put helpfile keywords here in caps");

    OLC_HELP(d)->keywords = strdup(buf3);
    /*strdup(OLC_STORAGE(d));*/
    OLC_HELP(d)->entry = strdup("\r\nThis help file is unfinished.\r\n");
    OLC_HELP(d)->min_level = 0;
    OLC_HELP(d)->duplicate = 0;
    OLC_VAL(d) = 0;

    hedit_disp_menu(d);
}

static void hedit_setup_existing(struct descriptor_data *d, int rnum) {
    CREATE(OLC_HELP(d), struct help_index_element, 1);

    OLC_HELP(d)->keywords = str_udup(help_table[rnum].keywords);
    OLC_HELP(d)->entry = str_udup(help_table[rnum].entry);
    OLC_HELP(d)->duplicate = help_table[rnum].duplicate;
    OLC_HELP(d)->min_level = help_table[rnum].min_level;
    OLC_VAL(d) = 0;

    hedit_disp_menu(d);
}

static void hedit_save_internally(struct descriptor_data *d) {
    struct help_index_element *new_help_table = nullptr;
    if (OLC_ZNUM(d) == NOWHERE) {
        /*if (OLC_ZNUM(d) > top_of_helpt) {*/
        int i;
        CREATE(new_help_table, struct help_index_element, top_of_helpt + 2);

        for (i = 0; i < top_of_helpt; i++)
            new_help_table[i] = help_table[i];
        /*new_help_table[++top_of_helpt] = *OLC_HELP(d);*/
        new_help_table[top_of_helpt++] = *OLC_HELP(d);
        free(help_table);
        help_table = new_help_table;
    } else
        help_table[OLC_ZNUM(d)] = *OLC_HELP(d);

    add_to_save_list(HEDIT_PERMISSION, SL_HLP);
    hedit_save_to_disk(d);
}

static void hedit_save_to_disk(struct descriptor_data *d) {
    FILE *fp;
    char buf1[MAX_STRING_LENGTH], index_name[READ_SIZE];
    int i;

    snprintf(index_name, sizeof(index_name), "%s%s", HLP_PREFIX, HELP_FILE);
    if (!(fp = fopen(index_name, "w"))) {
        basic_mud_log("SYSERR: Could not write help index file");
        return;
    }

    for (i = 0; i < top_of_helpt; i++) {
        if (help_table[i].duplicate)
            continue;
        strncpy(buf1, help_table[i].entry ? help_table[i].entry : "Empty\r\n", sizeof(buf1) - 1);
        strip_cr(buf1);

        /* Forget making a buffer, lets just write the thing now. */
        fprintf(fp, "%s#%d\n", buf1, help_table[i].min_level);
    }
    /* Write final line and close. */
    fprintf(fp, "$~\n");
    fclose(fp);

    remove_from_save_list(HEDIT_PERMISSION, SL_HLP);

    /* Reboot the help files. */
    free_help_table();
    index_boot(DB_BOOT_HLP);
}

/* The main menu. */
static void hedit_disp_menu(struct descriptor_data *d) {
    write_to_output(d,
                    "@n-- Help file editor\r\n"
                    "@g1@n) Keywords    : @y%s\n"
                    "@g2@n) Entry       :\r\n@y%s"
                    "@g3@n) Min Level   : @y%d\r\n"
                    "@gQ@n) Quit\r\n"
                    "Enter choice : ",
                    OLC_HELP(d)->keywords,
                    OLC_HELP(d)->entry,
                    OLC_HELP(d)->min_level
    );
    OLC_MODE(d) = HEDIT_MAIN_MENU;
}

void hedit_parse(struct descriptor_data *d, char *arg) {
    char *oldtext = '\0';
    int number, change = true;


    switch (OLC_MODE(d)) {
        case HEDIT_CONFIRM_SAVESTRING:
            switch (*arg) {
                case 'y':
                case 'Y':
                    if (OLC_HELP(d)->keywords == nullptr) {
                        hedit_disp_menu(d);
                        write_to_output(d, "\n@RYou must fill in the keywords before you save.@n\r\n");
                    } else if (strstr(OLC_HELP(d)->keywords, "undefined")) {
                        hedit_disp_menu(d);
                        write_to_output(d, "\n@RYou must fill in the keywords before you save.@n\r\n");
                    } else if (strstr(OLC_HELP(d)->keywords, "<<X<<")) {
                        hedit_disp_menu(d);
                        write_to_output(d, "\n@RYou must fill in the keywords before you save.@n\r\n");
                    } else {
                        write_to_output(d, "Help saved to disk.\r\n");
                        char buf[MAX_INPUT_LENGTH];
                        *buf = '\0';
                        sprintf(buf, "%s", OLC_HELP(d)->keywords);
                        send_to_imm("@gHedit@D: @w%s@G has just edited and saved, @Y%s@G.@n", d->character->name, buf);
                        hedit_save_internally(d);
                        /* Do not free strings, just the help structure. */
                        cleanup_olc(d, CLEANUP_STRUCTS);
                        HEDITS = false;
                    }
                    break;
                case 'n':
                case 'N':
                    /* Free everything up, including strings, etc. */
                    cleanup_olc(d, CLEANUP_ALL);
                    HEDITS = false;
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\nDo you wish to save your changes? : \r\n");
                    break;
            }
            return;

        case HEDIT_CONFIRM_EDIT:
            switch (*arg) {
                case 'y':
                case 'Y':
                    hedit_setup_existing(d, OLC_ZNUM(d));
                    break;
                case 'q':
                case 'Q':
                    cleanup_olc(d, CLEANUP_ALL);
                    break;
                case 'n':
                case 'N':
                    OLC_ZNUM(d)++;
                    for (; OLC_ZNUM(d) < top_of_helpt; OLC_ZNUM(d)++)
                        if (is_abbrev(OLC_STORAGE(d), help_table[OLC_ZNUM(d)].keywords))
                            break;
                        else
                            OLC_ZNUM(d) = top_of_helpt + 1;

                    if (OLC_ZNUM(d) > top_of_helpt) {
                        write_to_output(d, "Do you wish to add the '%s' help file? ",
                                        OLC_STORAGE(d));
                        OLC_MODE(d) = HEDIT_CONFIRM_ADD;
                    } else {
                        write_to_output(d, "Do you wish to edit the '%s' help file? ",
                                        help_table[OLC_ZNUM(d)].keywords);
                        OLC_MODE(d) = HEDIT_CONFIRM_EDIT;
                    }
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\n"
                                       "Do you wish to edit the '%s' help file? ",
                                    help_table[OLC_ZNUM(d)].keywords);
                    break;
            }
            return;

        case HEDIT_CONFIRM_ADD:
            switch (*arg) {
                case 'y':
                case 'Y':
                    hedit_setup_new(d);
                    break;
                case 'n':
                case 'N':
                case 'q':
                case 'Q':
                    cleanup_olc(d, CLEANUP_ALL);
                    HEDITS = false;
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\n"
                                       "Do you wish to add the '%s' help file? ",
                                    OLC_STORAGE(d));
                    break;
            }
            return;

        case HEDIT_MAIN_MENU:
            switch (*arg) {
                case 'q':
                case 'Q':
                    if (OLC_VAL(d)) {
                        /* Something has been modified. */
                        write_to_output(d, "Do you wish to save your changes? : ");
                        OLC_MODE(d) = HEDIT_CONFIRM_SAVESTRING;
                    } else {
                        write_to_output(d, "No changes made.\r\n");
                        cleanup_olc(d, CLEANUP_ALL);
                        HEDITS = false;
                    }
                    break;
                case '1':
                    OLC_MODE(d) = HEDIT_KEYWORDS;
                    clear_screen(d);
                    write_to_output(d, "Enter help file keywords: ");
                    break;
                case '2':
                    OLC_MODE(d) = HEDIT_ENTRY;
                    clear_screen(d);
                    send_editor_help(d);
                    write_to_output(d, "Enter help entry: (/s saves /h for help)\r\n");
                    if (OLC_HELP(d)->entry) {
                        write_to_output(d, "%s", OLC_HELP(d)->entry);
                        oldtext = strdup(OLC_HELP(d)->entry);
                    }
                    string_write(d, &OLC_HELP(d)->entry, MAX_MESSAGE_LENGTH, 0, oldtext);
                    OLC_VAL(d) = 1;
                    break;
                case '3':
                    write_to_output(d, "Enter min level : ");
                    OLC_MODE(d) = HEDIT_MIN_LEVEL;
                    break;
                default:
                    write_to_output(d, "Invalid choice!\r\n");
                    hedit_disp_menu(d);
                    break;
            }
            return;

        case HEDIT_KEYWORDS:
            if (OLC_HELP(d)->keywords)
                free(OLC_HELP(d)->keywords);
            if (strlen(arg) > MAX_HELP_KEYWORDS)
                arg[MAX_HELP_KEYWORDS - 1] = '\0';
            strip_cr(arg);
            OLC_HELP(d)->keywords = str_udup(arg);
            char buf4[MAX_MESSAGE_LENGTH * 2];
            if (strstr(OLC_HELP(d)->keywords, "undefined")) {
                OLC_MODE(d) = HEDIT_KEYWORDS;
                clear_screen(d);
                write_to_output(d, "@RYou must at least enter SOME keywords.@n\n");
                write_to_output(d, "Keywords: ");
                change = false;
            } else if (strstr(OLC_HELP(d)->keywords, "<<X<<")) {
                OLC_MODE(d) = HEDIT_KEYWORDS;
                clear_screen(d);
                write_to_output(d, "@RLet's not joke around with help files now.@n\n");
                write_to_output(d, "Keywords: ");
                change = false;
            } else if (strstr(OLC_HELP(d)->keywords, "<<x<<")) {
                OLC_MODE(d) = HEDIT_KEYWORDS;
                clear_screen(d);
                write_to_output(d, "@RLet's not joke around with help files now.@n\n");
                write_to_output(d, "Keywords: ");
                change = false;
            } else {
                sprintf(buf4, "%s\r\n----------\r\n\r\n%s", OLC_HELP(d)->keywords, OLC_HELP(d)->entry);
                OLC_HELP(d)->entry = strdup(buf4);
            }
            break;

        case HEDIT_ENTRY:
            /* We will NEVER get here, we hope. */
            mudlog(true, ADMLVL_BUILDER, BRF, "SYSERR: Reached HEDIT_ENTRY case in parse_hedit");
            break;

        case HEDIT_MIN_LEVEL:
            number = atoi(arg);
            if ((number < 0) || (number > ADMLVL_IMPL))
                write_to_output(d, "That is not a valid choice!\r\nEnter min level:-\r\n] ");
            else {
                OLC_HELP(d)->min_level = number;
                break;
            }
            return;

        default:
            /* We should never get here. */
            mudlog(true, ADMLVL_BUILDER, BRF, "SYSERR: Reached default case in parse_hedit");
            break;
    }

    /* If we get this far, something has been changed. */
    if (change == true) {
        OLC_VAL(d) = 1;
        hedit_disp_menu(d);
    }
}

void hedit_string_cleanup(struct descriptor_data *d, int terminator) {
    switch (OLC_MODE(d)) {
        case HEDIT_ENTRY:
            hedit_disp_menu(d);
            break;
    }
}

ACMD(do_helpcheck) {
    ACMD(do_action);

    char buf[MAX_STRING_LENGTH];
    int i, count = 0;
    size_t len = 0, nlen;

    send_to_char(ch, "Commands without help entries:\r\n");

    for (i = 1; *(complete_cmd_info[i].command) != '\n'; i++) {
        if (complete_cmd_info[i].command_pointer != do_action && complete_cmd_info[i].minimum_level >= 0) {
            if (search_help((char *) (complete_cmd_info[i].command), ADMLVL_IMPL) == NOWHERE) {
/*if (search_help(complete_cmd_info[i].command, ADMLVL_IMPL) == 
NOWHERE) {*/
                nlen = snprintf(buf + len, sizeof(buf) - len, "%-20.20s%s", complete_cmd_info[i].command,
                                (++count % 3 ? "" : "\r\n"));
                if (len + nlen >= sizeof(buf))
                    break;
                len += nlen;
            }
        }
    }
    if (count % 3 && len < sizeof(buf))
        nlen = snprintf(buf + len, sizeof(buf) - len, "\r\n");

    if (ch->desc) {
        write_to_output(ch->desc, buf);
    }

    *buf = '\0';
}


ACMD(do_hindex) {
    int len, count = 0, i, num = 0;
    char buf[MAX_STRING_LENGTH];

    skip_spaces(&argument);

    if (!*argument) {
        send_to_char(ch, "Usage: hindex <string>\r\n");
        for (i = 0; i < top_of_helpt; i++) {
            num++;
        }
        if (num > 0 && GET_ADMLEVEL(ch) > 0) {
            send_to_char(ch, "\r\n@D[@Y%d@y Help files in index.@D]@n\r\n", num);
        }
        return;
    }

    len = sprintf(buf, "Help index entries based on '%s':\r\n", argument);
    for (i = 0; i < top_of_helpt; i++) {
        num++;
        if (is_abbrev(argument, help_table[i].keywords)
            && (GET_ADMLEVEL(ch) >= help_table[i].min_level))
            len +=
                    snprintf(buf + len, sizeof(buf) - len, "%-20.20s%s", help_table[i].keywords,
                             (++count % 3 ? "" : "\r\n"));
    }
    if (count % 3)
        len += snprintf(buf + len, sizeof(buf) - len, "\r\n");

    if (!count)
        len += snprintf(buf + len, sizeof(buf) - len, "  None.\r\n");

    if (count > 0 && GET_ADMLEVEL(ch) > 0) {
        len += snprintf(buf + len, sizeof(buf) - len, "  %d Help files in index.\r\n", count);
    }
    write_to_output(ch->desc, buf);
}
