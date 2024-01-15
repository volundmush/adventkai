/* 
************************************************************************
*   File: act.social.c                                  Part of CircleMUD *
*  Usage: Functions to handle socials                                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "dbat/act.social.h"
#include "dbat/comm.h"
#include "dbat/utils.h"
#include "dbat/commands.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"

/* local functions */
char *fread_action(FILE *fl, int nr);

static int find_action(int cmd);

ACMD(do_action) {
    char arg[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    int act_nr;
    struct social_messg *action;
    struct char_data *vict;
    struct obj_data *targ;

    if ((act_nr = find_action(cmd)) < 0) {
        send_to_char(ch, "That action is not supported.\r\n");
        return;
    }

    action = &soc_mess_list[act_nr];

    if (!argument || !*argument) {
        send_to_char(ch, "%s\r\n", action->char_no_arg);
        act(action->others_no_arg, action->hide, ch, nullptr, nullptr, TO_ROOM);
        return;
    }

    two_arguments(argument, arg, part);

    if ((!action->char_body_found) && (*part)) {
        send_to_char(ch, "Sorry, this social does not support body parts.\r\n");
        return;
    }

    if (!action->char_found)
        *arg = '\0';

    if (action->char_found && argument)
        one_argument(argument, arg);
    else
        *arg = '\0';

    vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM);
    if (!vict) {
        if (action->char_obj_found) {
            targ = get_obj_in_list_vis(ch, arg, nullptr, ch->contents);
            if (!targ) targ = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents);
            if (targ) {
                act(action->char_obj_found, action->hide, ch, targ, nullptr, TO_CHAR);
                act(action->others_obj_found, action->hide, ch, targ, nullptr, TO_ROOM);
                return;
            }
        }
        if (action->not_found)
            send_to_char(ch, "%s\r\n", action->not_found);
        else
            send_to_char(ch, "I don't see anything by that name here.\r\n");
        return;
    }

    if (vict == ch) {
        if (action->char_auto)
            send_to_char(ch, "%s\r\n", action->char_auto);
        else
            send_to_char(ch, "Erm, no.\r\n");
        act(action->others_auto, action->hide, ch, nullptr, nullptr, TO_ROOM);
        return;
    }

    if (GET_POS(vict) < action->min_victim_position)
        act("$N is not in a proper position for that.", false, ch, nullptr, vict, TO_CHAR | TO_SLEEP);
    else {
        if (*part) {
            act(action->char_body_found, 0, ch, (struct obj_data *) part, vict, TO_CHAR | TO_SLEEP);
            act(action->others_body_found, action->hide, ch, (struct obj_data *) part, vict, TO_NOTVICT);
            act(action->vict_body_found, action->hide, ch, (struct obj_data *) part, vict, TO_VICT);
        } else {
            act(action->char_found, 0, ch, nullptr, vict, TO_CHAR | TO_SLEEP);
            act(action->others_found, action->hide, ch, nullptr, vict, TO_NOTVICT);
            act(action->vict_found, action->hide, ch, nullptr, vict, TO_VICT);
        }
    }
}

ACMD(do_insult) {
    char arg[MAX_INPUT_LENGTH];
    struct char_data *victim;

    one_argument(argument, arg);

    if (*arg) {
        if (!(victim = get_char_vis(ch, arg, nullptr, FIND_CHAR_ROOM)))
            send_to_char(ch, "Can't hear you!\r\n");
        else {
            if (victim != ch) {
                send_to_char(ch, "You insult %s.\r\n", GET_NAME(victim));

                switch (rand_number(0, 2)) {
                    case 0:
                        if (GET_SEX(ch) == SEX_MALE) {
                            if (GET_SEX(victim) == SEX_MALE)
                                act("$n accuses you of fighting like a woman!", false, ch, nullptr, victim, TO_VICT);
                            else
                                act("$n says that women can't fight.", false, ch, nullptr, victim, TO_VICT);
                        } else {        /* Ch == Woman */
                            if (GET_SEX(victim) == SEX_MALE)
                                act("$n accuses you of having the smallest... (brain?)",
                                    false, ch, nullptr, victim, TO_VICT);
                            else
                                act("$n tells you that you'd lose a beauty contest against a troll.",
                                    false, ch, nullptr, victim, TO_VICT);
                        }
                        break;
                    case 1:
                        act("$n calls your mother a bitch!", false, ch, nullptr, victim, TO_VICT);
                        break;
                    default:
                        act("$n tells you to get lost!", false, ch, nullptr, victim, TO_VICT);
                        break;
                }            /* end switch */

                act("$n insults $N.", true, ch, nullptr, victim, TO_NOTVICT);
            } else {            /* ch == victim */
                send_to_char(ch, "You feel insulted.\r\n");
            }
        }
    } else
        send_to_char(ch, "I'm sure you don't want to insult *everybody*...\r\n");
}

void boot_social_messages() {
    FILE *fl;
    int nr = 0, hide, min_char_pos, min_pos, min_lvl, curr_soc = -1;
    char next_soc[MAX_STRING_LENGTH], sorted[MAX_INPUT_LENGTH];

    if (CONFIG_NEW_SOCIALS == true) {
        /* open social file */
        if (!(fl = fopen(SOCMESS_FILE_NEW, "r"))) {
            basic_mud_log("SYSERR: can't open socials file '%s': %s", SOCMESS_FILE_NEW, strerror(errno));
            /*  SYSERR_DESC:
             *  This error, from boot_social_messages(), occurs when the server
             *  fails to open the file containing the social messages.  The error
             *  at the end will indicate the reason why.
             */
            exit(1);
        }
        /* count socials */
        *next_soc = '\0';
        while (!feof(fl)) {
            fgets(next_soc, MAX_STRING_LENGTH, fl);
            if (*next_soc == '~') top_of_socialt++;
        }
    } else { /* old style */

        /* open social file */
        if (!(fl = fopen(SOCMESS_FILE, "r"))) {
            basic_mud_log("SYSERR: can't open socials file '%s': %s", SOCMESS_FILE, strerror(errno));
            exit(1);
        }
        /* count socials */
        while (!feof(fl)) {
            fgets(next_soc, MAX_STRING_LENGTH, fl);
            if (*next_soc == '\n' || *next_soc == '\r') top_of_socialt++; /* all socials are followed by a blank line */
        }
    }

    basic_mud_log("Social table contains %d socials.", top_of_socialt);
    rewind(fl);

    CREATE(soc_mess_list, struct social_messg, top_of_socialt + 1);

    /* now read 'em */
    for (;;) {
        fscanf(fl, " %s ", next_soc);
        if (*next_soc == '$') break;

        if (CONFIG_NEW_SOCIALS == true) {
            if (fscanf(fl, " %s %d %d %d %d \n",
                       sorted, &hide, &min_char_pos, &min_pos, &min_lvl) != 5) {
                basic_mud_log("SYSERR: format error in social file near social '%s'", next_soc);
                /*  SYSERR_DESC:
                 *  From boot_social_messages(), this error is output when the
                 *  server is expecting to find the remainder of the first line of the
                 *  social ('hide' and 'minimum position').  These must follow the
                 *  name of the social with a single space such as: 'accuse 0 5\n'.
                 *  This error often occurs when one of the numbers is missing or the
                 *  social name has a space in it (i.e., 'bend over').
                 */
                exit(1);
            }
            curr_soc++;
            soc_mess_list[curr_soc].command = strdup(next_soc + 1);
            soc_mess_list[curr_soc].sort_as = strdup(sorted);
            soc_mess_list[curr_soc].hide = hide;
            soc_mess_list[curr_soc].min_char_position = min_char_pos;
            soc_mess_list[curr_soc].min_victim_position = min_pos;
            soc_mess_list[curr_soc].min_level_char = min_lvl;
        } else {  /* old style */
            if (fscanf(fl, " %d %d \n", &hide, &min_pos) != 2) {
                basic_mud_log("SYSERR: format error in social file near social '%s'", next_soc);
                exit(1);
            }
            curr_soc++;
            soc_mess_list[curr_soc].command = strdup(next_soc);
            soc_mess_list[curr_soc].sort_as = strdup(next_soc);
            soc_mess_list[curr_soc].hide = hide;
            soc_mess_list[curr_soc].min_char_position = POS_RESTING;
            soc_mess_list[curr_soc].min_victim_position = min_pos;
            soc_mess_list[curr_soc].min_level_char = 0;
        }

#ifdef CIRCLE_ACORN
        if (fgetc(fl) != '\n')
          log("SYSERR: Acorn bug workaround failed.");
          /*  SYSERR_DESC:
           *  The only time that this error should ever arise is if you are running
           *  your CircleMUD on the Acorn platform.  The error arises when the
           *  server cannot properly read a '\n' out of the file at the end of the
           *  first line of the social (that with 'hide' and 'min position').  This
           *  is in boot_social_messages().
           */
#endif

        soc_mess_list[curr_soc].char_no_arg = fread_action(fl, nr);
        soc_mess_list[curr_soc].others_no_arg = fread_action(fl, nr);
        soc_mess_list[curr_soc].char_found = fread_action(fl, nr);

        /* if no char_found, the rest is to be ignored */
        if (CONFIG_NEW_SOCIALS == false && !soc_mess_list[curr_soc].char_found)
            continue;

        soc_mess_list[curr_soc].others_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].vict_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].not_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].char_auto = fread_action(fl, nr);
        soc_mess_list[curr_soc].others_auto = fread_action(fl, nr);

        if (CONFIG_NEW_SOCIALS == false)
            continue;

        soc_mess_list[curr_soc].char_body_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].others_body_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].vict_body_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].char_obj_found = fread_action(fl, nr);
        soc_mess_list[curr_soc].others_obj_found = fread_action(fl, nr);
    }

    /* close file & set top */
    fclose(fl);
    assert(curr_soc <= top_of_socialt);
    top_of_socialt = curr_soc;
}

void create_command_list() {
    int i, j, k;
    struct social_messg temp;

    /* free up old command list */
    if (complete_cmd_info)
        free_command_list();

    /* re check the sort on the socials */
    for (j = 0; j < top_of_socialt; j++) {
        k = j;
        for (i = j + 1; i <= top_of_socialt; i++)
            if (strcasecmp(soc_mess_list[i].sort_as, soc_mess_list[k].sort_as) < 0)
                k = i;
        if (j != k) {
            temp = soc_mess_list[j];
            soc_mess_list[j] = soc_mess_list[k];
            soc_mess_list[k] = temp;
        }
    }

    /* count the commands in the command list */
    i = 0;
    while (*cmd_info[i].command != '\n') i++;
    i++;

    CREATE(complete_cmd_info, struct command_info, top_of_socialt + i + 2);

    /* this loop sorts the socials and commands together into one big list */
    i = 0;
    j = 0;
    k = 0;

    while (*cmd_info[i].command != '\n') {
        complete_cmd_info[k++] = cmd_info[i++];
    }

    while (j <= top_of_socialt) {
        soc_mess_list[j].act_nr = k;
        complete_cmd_info[k].command = soc_mess_list[j].command;
        complete_cmd_info[k].sort_as = soc_mess_list[j].sort_as;
        complete_cmd_info[k].minimum_position = soc_mess_list[j].min_char_position;
        complete_cmd_info[k].command_pointer = do_action;
        complete_cmd_info[k].minimum_level = soc_mess_list[j++].min_level_char;
        complete_cmd_info[k].minimum_admlevel = ADMLVL_NONE;
        complete_cmd_info[k++].subcmd = 0;
    }

    complete_cmd_info[k].command = "\n";
    complete_cmd_info[k].sort_as = "zzzzzzz";
    complete_cmd_info[k].minimum_position = 0;
    complete_cmd_info[k].command_pointer = nullptr;
    complete_cmd_info[k].minimum_level = 0;
    complete_cmd_info[k].minimum_admlevel = 0;
    complete_cmd_info[k].subcmd = 0;
    basic_mud_log("Command info rebuilt, %d total commands.", k);
}

void free_command_list() {
    free(complete_cmd_info);
    complete_cmd_info = nullptr;
}

char *fread_action(FILE *fl, int nr) {
    char buf[MAX_STRING_LENGTH];

    fgets(buf, MAX_STRING_LENGTH, fl);
    if (feof(fl)) {
        basic_mud_log("SYSERR: fread_action: unexpected EOF near action #%d", nr);
        /*  SYSERR_DESC:
         *  fread_action() will fail if it discovers an end of file marker
         *  before it is able to read in the expected string.  This can be
         *  caused by a truncated socials file.
         */
        exit(1);
    }
    if (*buf == '#')
        return (nullptr);

    buf[strlen(buf) - 1] = '\0';
    return (strdup(buf));
}

void free_social_messages() {
    struct social_messg *mess;
    int i;

    for (i = 0; i <= top_of_socialt; i++) {
        mess = &soc_mess_list[i];
        free_action(mess);
    }
    free(soc_mess_list);

}

void free_action(struct social_messg *mess) {
    if (mess->command) free(mess->command);
    if (mess->sort_as) free(mess->sort_as);
    if (mess->char_no_arg) free(mess->char_no_arg);
    if (mess->others_no_arg) free(mess->others_no_arg);
    if (mess->char_found) free(mess->char_found);
    if (mess->others_found) free(mess->others_found);
    if (mess->vict_found) free(mess->vict_found);
    if (mess->char_body_found) free(mess->char_body_found);
    if (mess->others_body_found) free(mess->others_body_found);
    if (mess->vict_body_found) free(mess->vict_body_found);
    if (mess->not_found) free(mess->not_found);
    if (mess->char_auto) free(mess->char_auto);
    if (mess->others_auto) free(mess->others_auto);
    if (mess->char_obj_found) free(mess->char_obj_found);
    if (mess->others_obj_found) free(mess->others_obj_found);
    memset(mess, 0, sizeof(struct social_messg));
}

static int find_action(int cmd) {
    int bot, top, mid;

    bot = 0;
    top = top_of_socialt;

    if (top < 0)
        return (-1);

    for (;;) {
        mid = (bot + top) / 2;

        if (soc_mess_list[mid].act_nr == cmd)
            return (mid);
        if (bot >= top)
            return (-1);

        if (soc_mess_list[mid].act_nr > cmd)
            top = --mid;
        else
            bot = ++mid;
    }
}

struct social_messg *find_social(const char *name) {
    int cmd, socidx;

    if ((cmd = find_command(name)) < 0)
        return nullptr;

    if ((socidx = find_action(cmd)) < 0)
        return nullptr;

    return &soc_mess_list[socidx];
}

ACMD(do_gmote) {
    int act_nr, length;
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    struct social_messg *action;
    struct char_data *vict = nullptr;

    half_chop(argument, buf, arg);

    if (subcmd)
        for (length = strlen(buf), cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
            if (!strncmp(complete_cmd_info[cmd].command, buf, length))
                break;

    if ((act_nr = find_action(cmd)) < 0) {
        snprintf(buf, sizeof(buf), "@D[@BOOC@D: @g%s %s@n@D]", GET_ADMLEVEL(ch) < 1 ? GET_USER(ch) : GET_NAME(ch),
                 argument);
        act(buf, false, ch, nullptr, vict, TO_GMOTE);
        return;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SOUNDPROOF)) {
        send_to_char(ch, "The walls seem to absorb your actions.\r\n");
        return;
    }

    action = &soc_mess_list[act_nr];

    if (!action->char_found)
        *arg = '\0';

    if (!*arg) {
        if (!action->others_no_arg || !*action->others_no_arg) {
            send_to_char(ch, "Who are you going to do that to?\r\n");
            return;
        }
        snprintf(buf, sizeof(buf), "@D[@BOOC@D: @g%s@D]@n", action->others_no_arg);
    } else if (!(vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_WORLD))) {
        send_to_char(ch, "%s\r\n", action->not_found);
        return;
    } else if (vict == ch) {
        if (!action->others_auto || !*action->others_auto) {
            send_to_char(ch, "%s\r\n", action->char_auto);
            return;
        }
        snprintf(buf, sizeof(buf), "@D[@BOOC@D: @g%s@D]@n", action->others_auto);
    } else {
        if (GET_POS(vict) < action->min_victim_position) {
            act("$N is not in a proper position for that.",
                false, ch, nullptr, vict, TO_CHAR | TO_SLEEP);
            return;
        }
        snprintf(buf, sizeof(buf), "@D[@BOOC@D: @g%s@D]@n", action->others_found);
    }
    act(buf, false, ch, nullptr, vict, TO_GMOTE);
}
