/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include <boost/algorithm/string.hpp>

#include "dbat/interpreter.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/utils.h"
#include "dbat/spells.h"
#include "dbat/handler.h"
#include "dbat/mail.h"
#include "dbat/oasis.h"
#include "dbat/dg_scripts.h"
#include "dbat/guild.h"
#include "dbat/clan.h"
#include "dbat/class.h"
#include "dbat/races.h"
#include "dbat/config.h"
#include "dbat/weather.h"
#include "dbat/act.informative.h"
#include "dbat/players.h"
#include "dbat/assedit.h"
#include "dbat/obj_edit.h"
#include "dbat/commands.h"
#include "dbat/act.wizard.h"

/* local global variables */
DISABLED_DATA *disabled_first = nullptr;

/* local functions */
int perform_dupe_check(struct descriptor_data *d);

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias_data *a);

int reserved_word(char *argument);

int command_pass(char *cmd, struct char_data *ch);

void payout(int num);



/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

const char *fill[] =
        {
                "in",
                "into",
                "from",
                "with",
                "the",
                "on",
                "at",
                "to",
                "\n"
        };

const char *reserved[] =
        {
                "a",
                "an",
                "self",
                "me",
                "all",
                "room",
                "someone",
                "something",
                "\n"
        };


/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument) {
    int cmd, length;
    int skip_ld = 0;
    char *line;
    char arg[MAX_INPUT_LENGTH];

    switch (GET_POS(ch)) {
        case POS_DEAD:
        case POS_INCAP:
        case POS_MORTALLYW:
        case POS_STUNNED:
            GET_POS(ch) = POS_SITTING;
            break;
    }

    /* just drop to next line for hitting CR */
    skip_spaces(&argument);
    if (!*argument)
        return;

    /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
    if (!isalpha(*argument)) {
        arg[0] = argument[0];
        arg[1] = '\0';
        line = argument + 1;
    } else
        line = any_one_arg(argument, arg);


    if (!strcasecmp(arg, "-")) {
        return;
    }
    /* Since all command triggers check for valid_dg_target before acting, the levelcheck
   * here has been removed.
   */
    /* otherwise, find the command */
    {
        int cont;                                            /* continue the command checks */
        cont = command_wtrigger(ch, arg, line);              /* any world triggers ? */
        if (!cont) cont = command_mtrigger(ch, arg, line);   /* any mobile triggers ? */
        if (!cont) cont = command_otrigger(ch, arg, line);   /* any object triggers ? */
        if (cont) return;                                    /* yes, command trigger took over */
    }
    for (length = strlen(arg), cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++) {
        if (!strncmp(complete_cmd_info[cmd].command, arg, length))
            if (GET_LEVEL(ch) >= complete_cmd_info[cmd].minimum_level &&
                GET_ADMLEVEL(ch) >= complete_cmd_info[cmd].minimum_admlevel)
                break;
    }

    char blah[MAX_INPUT_LENGTH];

    sprintf(blah, "%s", complete_cmd_info[cmd].command);
    if (!strcasecmp(blah, "throw"))
        ch->throws = rand_number(1, 3);


    if (*complete_cmd_info[cmd].command == '\n') {
        send_to_char(ch, "Huh!?!\r\n");
        return;
    }

    if (!command_pass(blah, ch) && GET_ADMLEVEL(ch) < 1)
        send_to_char(ch, "It's unfortunate...\r\n");
    else if (check_disabled(&complete_cmd_info[cmd]))    /* is it disabled? */
        send_to_char(ch, "This command has been temporarily disabled.\r\n");
    else if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_GOOP) && GET_ADMLEVEL(ch) < ADMLVL_IMPL)
        send_to_char(ch, "You only have your internal thoughts until your body has finished regenerating!\r\n");
    else if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_FROZEN) && GET_ADMLEVEL(ch) < ADMLVL_IMPL)
        send_to_char(ch, "You try, but the mind-numbing cold prevents you...\r\n");
    else if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_SPIRAL))
        send_to_char(ch, "You are occupied with your Spiral Comet attack!\r\n");
    else if (complete_cmd_info[cmd].command_pointer == nullptr)
        send_to_char(ch, "Sorry, that command hasn't been implemented yet.\r\n");
    else if (IS_NPC(ch) && complete_cmd_info[cmd].minimum_admlevel >= ADMLVL_IMMORT)
        send_to_char(ch, "You can't use immortal commands while switched.\r\n");
    else if (GET_POS(ch) < complete_cmd_info[cmd].minimum_position && GET_POS(ch) != POS_FIGHTING) {
        switch (GET_POS(ch)) {
            case POS_DEAD:
                send_to_char(ch, "Lie still; you are DEAD!!! :-(\r\n");
                break;
            case POS_INCAP:
            case POS_MORTALLYW:
                send_to_char(ch, "You are in a pretty bad shape, unable to do anything!\r\n");
                break;
            case POS_STUNNED:
                send_to_char(ch, "All you can do right now is think about the stars!\r\n");
                break;
            case POS_SLEEPING:
                send_to_char(ch, "In your dreams, or what?\r\n");
                break;
            case POS_RESTING:
                send_to_char(ch, "Nah... You feel too relaxed to do that..\r\n");
                break;
            case POS_SITTING:
                send_to_char(ch, "Maybe you should get on your feet first?\r\n");
                break;
            case POS_FIGHTING:
                send_to_char(ch, "No way!  You're fighting for your life!\r\n");
                break;
        }
    } else if (no_specials || !special(ch, cmd, line)) {
        if (!skip_ld) {
            ((*complete_cmd_info[cmd].command_pointer)(ch, line, cmd, complete_cmd_info[cmd].subcmd));
        }
    }
}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/

/* The interface to the outside world: do_alias */
ACMD(do_alias) {
    char arg[MAX_INPUT_LENGTH];
    char *repl;
    struct alias_data *a, *temp;

    if (IS_NPC(ch))
        return;

    auto &p = players[ch->id];

    repl = any_one_arg(argument, arg);

    if (!*arg) {            /* no argument specified -- list currently defined aliases */
        send_to_char(ch, "Currently defined aliases:\r\n");
        int count = 0;
        for(auto &a : p.aliases) {
            count++;
            send_to_char(ch, "%-15s %s\r\n", a.name.c_str(), a.replacement.c_str());
        }
        if(!count) {
            send_to_char(ch, " None.\r\n");
        }
        return;
    }
    /* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    auto &aliases = p.aliases;
    auto find = std::find_if(aliases.begin(), aliases.end(), [&](const auto &a) {
        return boost::iequals(a.name, arg);
    });

    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
        if(find == aliases.end())
            send_to_char(ch, "No such alias.\r\n");
        else {
			aliases.erase(find);
            send_to_char(ch, "Alias deleted.\r\n");
            dirty_players.insert(ch->id);
        }
        return;
    }

    /* otherwise, either add or redefine an alias */
    if (!strcasecmp(arg, "alias")) {
        send_to_char(ch, "You can't alias 'alias'.\r\n");
        return;
    }

    delete_doubledollar(repl);
    // type is ALIAS_SIMPLE if repl contains no ; otherwiise it's ALIAS_COMPLEX
    auto type = (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR)) ? ALIAS_COMPLEX : ALIAS_SIMPLE;
    if(find != aliases.end()) {
        find->name = arg;
        find->replacement = repl;
        find->type = type;
        send_to_char(ch, "Alias redefined.\r\n");
    } else {
        auto &a = aliases.emplace_back();
        a.name = arg;
        a.replacement = repl;
        // type is ALIAS_SIMPLE if repl contains no ; otherwiise it's ALIAS_COMPLEX
        a.type = type;
        send_to_char(ch, "Alias added.\r\n");
    }
    dirty_players.insert(ch->id);
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

static void perform_complex_alias(struct descriptor_data *d, char *orig, struct alias_data *a) {
    struct txt_q temp_queue;
    char *tokens[NUM_TOKENS], *temp, *write_point;
    char buf2[MAX_RAW_INPUT_LENGTH], buf[MAX_RAW_INPUT_LENGTH];    /* raw? */
    int num_of_tokens = 0, num;

    /* First, parse the original string */
    strcpy(buf2, orig);    /* strcpy: OK (orig:MAX_INPUT_LENGTH < buf2:MAX_RAW_INPUT_LENGTH) */
    temp = strtok(buf2, " ");
    while (temp != nullptr && num_of_tokens < NUM_TOKENS) {
        tokens[num_of_tokens++] = temp;
        temp = strtok(nullptr, " ");
    }

    /* initialize */
    write_point = buf;
    temp_queue.head = temp_queue.tail = nullptr;

    /* now parse the alias */
    auto r = (char*)a->replacement.c_str();
    for (temp = r; *temp; temp++) {
        if (*temp == ALIAS_SEP_CHAR) {
            *write_point = '\0';
            buf[MAX_INPUT_LENGTH - 1] = '\0';
            write_to_q(buf, &temp_queue, 1);
            write_point = buf;
        } else if (*temp == ALIAS_VAR_CHAR) {
            temp++;
            if ((num = *temp - '1') < num_of_tokens && num >= 0) {
                strcpy(write_point, tokens[num]);    /* strcpy: OK */
                write_point += strlen(tokens[num]);
            } else if (*temp == ALIAS_GLOB_CHAR) {
                strcpy(write_point, orig);        /* strcpy: OK */
                write_point += strlen(orig);
            } else if ((*(write_point++) = *temp) == '$')    /* redouble $ for act safety */
                *(write_point++) = '$';
        } else
            *(write_point++) = *temp;
    }

    *write_point = '\0';
    buf[MAX_INPUT_LENGTH - 1] = '\0';
    write_to_q(buf, &temp_queue, 1);

    /* push our temp_queue on to the _front_ of the input queue */
    for(auto q = temp_queue.head; q; q = q->next)
        d->input_queue.emplace_back(q->text);
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
void perform_alias(struct descriptor_data *d, char *orig) {
    char first_arg[MAX_INPUT_LENGTH], *ptr;
    struct alias_data *a, *tmp;

    if(!d->character) {
        d->input_queue.emplace_back(orig);
        return;
    }


    /* Mobs don't have alaises. */
    if (IS_NPC(d->character)) {
        d->input_queue.emplace_back(orig);
        return;
    }
    auto &p = players[d->character->id];
    auto &aliases = p.aliases;

    /* bail out immediately if the guy doesn't have any aliases */
    if (aliases.empty()) {
        d->input_queue.emplace_back(orig);
        return;
    }

    /* find the alias we're supposed to match */
    ptr = any_one_arg(orig, first_arg);

    /* bail out if it's null */
    if (!*first_arg) {
        d->input_queue.emplace_back(orig);
        return;
    }

    auto find = std::find_if(aliases.begin(), aliases.end(), [&](const auto &a) {
        return boost::iequals(a.name, first_arg);
    });

    /* if the first arg is not an alias, return without doing anything */
    if (find == aliases.end()) {
        d->input_queue.emplace_back(orig);
        return;
    }

    if (find->type == ALIAS_SIMPLE) {
        d->input_queue.emplace_back(find->replacement);
    } else {
        perform_complex_alias(d, ptr, &*find);
    }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, const char **list, int exact) {
    int i, l;

    /*  We used to have \r as the first character on certain array items to
   *  prevent the explicit choice of that point.  It seems a bit silly to
   *  dump control characters into arrays to prevent that, so we'll just
   *  check in here to see if the first character of the argument is '!',
   *  and if so, just blindly return a '-1' for not found. - ae.
   */
    if (*arg == '!')
        return (-1);

    /* Make into lower case, and get length of string */
    for (l = 0; *(arg + l); l++)
        *(arg + l) = LOWER(*(arg + l));

    if (exact) {
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strcmp(arg, *(list + i)))
                return (i);
    } else {
        if (!l)
            l = 1;            /* Avoid "" to match the first available
				 * string */
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strncmp(arg, *(list + i), l))
                return (i);
    }

    return (-1);
}

int is_number(const char *str) {
    while (*str)
        if (!isdigit(*(str++)))
            return (0);

    return (1);
}

/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string) {
    for (; **string && isspace(**string); (*string)++);
}


/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string) {
    char *ddread, *ddwrite;

    /* If the string has no dollar signs, return immediately */
    if ((ddwrite = strchr(string, '$')) == nullptr)
        return (string);

    /* Start from the location of the first dollar sign */
    ddread = ddwrite;


    while (*ddread)   /* Until we reach the end of the string... */
        if ((*(ddwrite++) = *(ddread++)) == '$') /* copy one char */
            if (*ddread == '$')
                ddread++; /* skip if we saw 2 $'s in a row */

    *ddwrite = '\0';

    return (string);
}


int fill_word(char *argument) {
    return (search_block(argument, fill, true) >= 0);
}

void topLoad() {
    FILE *file;
    char fname[40], line[256], filler[50];
    int x = 0;

    /* Read Toplist File */
    if (!get_filename(fname, sizeof(fname), INTRO_FILE, "toplist")) {
        basic_mud_log("ERROR: Toplist file does not exist.");
        return;
    } else if (!(file = fopen(fname, "r"))) {
        basic_mud_log("ERROR: Toplist file does not exist.");
        return;
    }


    TOPLOADED = true;

    while (!feof(file)) {
        get_line(file, line);
        sscanf(line, "%s %" I64T "\n", filler, &toppoint[x]);
        topname[x] = strdup(filler);
        *filler = '\0';
        x++;
    }
    fclose(file);
}

/* Write the toplist to file */
void topWrite(struct char_data *ch) {
    if (GET_ADMLEVEL(ch) > 0 || IS_NPC(ch))
        return;

    if (TOPLOADED == false) {
        return;
    }

    char fname[40];
    FILE *fl;
    char *positions[25];
    int64_t points[25] = {0};
    int x = 0, writeEm = false, placed = false, start = 0, finish = 25, location = -1;
    int progress = false;

    if (!ch) {
        return;
    }

    if (!ch->desc || !GET_USER(ch)) {
        return;
    }

    for (x = start; x < finish; x++) { /* Save the places as they are right now */
        positions[x] = strdup(topname[x]);
        points[x] = toppoint[x];
    }

    /* Powerlevel Section */
    /* Set the start and finish for this section */
    start = 0;
    finish = 5;

    for (x = start; x < finish; x++) { /* Save the new spots */
        if (placed == false) { /* They Haven't Placed */
            if (strcasecmp(topname[x], GET_NAME(ch))) { /* Name doesn't match */
                if (GET_MAX_HIT(ch) > toppoint[x]) {
                    free(topname[x]);
                    toppoint[x] = GET_MAX_HIT(ch);
                    topname[x] = strdup(GET_NAME(ch));
                    placed = true;
                    writeEm = true;
                    location = x;
                }
            } else { /* This is their spot already */
                placed = true;
                location = finish;
            }
        } else { /* They have placed */
            if (x < finish && location < finish) {
                if (strcasecmp(positions[location], GET_NAME(ch))) { /* This isn't their old spot */
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                } else { /* This IS their old spot */
                    progress = true;
                    location += 1;
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                }
            }
        }
    } /* End Save New Spots*/

    if (progress == true) {
        send_to_all("@D[@GToplist@W: @C%s @Whas moved up in rank in the powerlevel section.@D]\r\n", GET_NAME(ch));
    } else if (placed == true && location != finish) {
        send_to_all("@D[@GToplist@W: @C%s @Whas placed in the powerlevel section.@D]\r\n", GET_NAME(ch));
    }

    location = -1;
    placed = false;
    progress = false;
    /* Ki Section         */
    /* Set the start and finish for this section */
    start = 5;
    finish = 10;

    for (x = start; x < finish; x++) { /* Save the new spots */
        if (placed == false) { /* They Haven't Placed */
            if (strcasecmp(topname[x], GET_NAME(ch))) { /* Name doesn't match */
                if (GET_MAX_MANA(ch) > toppoint[x]) {
                    free(topname[x]);
                    toppoint[x] = GET_MAX_MANA(ch);
                    topname[x] = strdup(GET_NAME(ch));
                    placed = true;
                    writeEm = true;
                    location = x;
                }
            } else { /* This is their spot already */
                placed = true;
                location = finish;
            }
        } else { /* They have placed */
            if (x < finish && location < finish) {
                if (strcasecmp(positions[location], GET_NAME(ch))) { /* This isn't their old spot */
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                } else { /* This IS their old spot */
                    progress = true;
                    location += 1;
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                }
            }
        }
    } /* End Save New Spots*/

    if (progress == true) {
        send_to_all("@D[@GToplist@W: @C%s @Whas moved up in rank in the ki section.@D]\r\n", GET_NAME(ch));
    } else if (placed == true && location != finish) {
        send_to_all("@D[@GToplist@W: @C%s @Whas placed in the ki section.@D]\r\n", GET_NAME(ch));
    }

    location = -1;
    placed = false;
    progress = false;

    /* Stamina Section    */
    /* Set the start and finish for this section */
    start = 10;
    finish = 15;

    for (x = start; x < finish; x++) { /* Save the new spots */
        if (placed == false) { /* They Haven't Placed */
            if (strcasecmp(topname[x], GET_NAME(ch))) { /* Name doesn't match */
                if (GET_MAX_MOVE(ch) > toppoint[x]) {
                    free(topname[x]);
                    toppoint[x] = GET_MAX_MOVE(ch);
                    topname[x] = strdup(GET_NAME(ch));
                    placed = true;
                    writeEm = true;
                    location = x;
                }
            } else { /* This is their spot already */
                placed = true;
                location = finish;
            }
        } else { /* They have placed */
            if (x < finish && location < finish) {
                if (strcasecmp(positions[location], GET_NAME(ch))) { /* This isn't their old spot */
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                } else { /* This IS their old spot */
                    progress = true;
                    location += 1;
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                }
            }
        }
    } /* End Save New Spots*/

    if (progress == true) {
        send_to_all("@D[@GToplist@W: @C%s @Whas moved up in rank in the stamina section.@D]\r\n", GET_NAME(ch));
    } else if (placed == true && location != finish) {
        send_to_all("@D[@GToplist@W: @C%s @Whas placed in the stamina section.@D]\r\n", GET_NAME(ch));
    }

    location = -1;
    placed = false;
    progress = false;

    /* Zenni Section      */
    /* Set the start and finish for this section */
    start = 15;
    finish = 20;

    for (x = start; x < finish; x++) { /* Save the new spots */
        if (placed == false) { /* They Haven't Placed */
            if (strcasecmp(topname[x], GET_NAME(ch))) { /* Name doesn't match */
                if (GET_BANK_GOLD(ch) + GET_GOLD(ch) > toppoint[x]) {
                    free(topname[x]);
                    toppoint[x] = GET_BANK_GOLD(ch) + GET_GOLD(ch);
                    topname[x] = strdup(GET_NAME(ch));
                    placed = true;
                    writeEm = true;
                    location = x;
                }
            } else { /* This is their spot already */
                placed = true;
                location = finish;
            }
        } else { /* They have placed */
            if (x < finish && location < finish) {
                if (strcasecmp(positions[location], GET_NAME(ch))) { /* This isn't their old spot */
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                } else { /* This IS their old spot */
                    progress = true;
                    location += 1;
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                }
            }
        }
    } /* End Save New Spots*/

    if (progress == true) {
        send_to_all("@D[@GToplist@W: @C%s @Whas moved up in rank in the zenni section.@D]\r\n", GET_NAME(ch));
    } else if (placed == true && location != finish) {
        send_to_all("@D[@GToplist@W: @C%s @Whas placed in the zenni section.@D]\r\n", GET_NAME(ch));
    }

    location = -1;
    placed = false;
    progress = false;

    /* RPP Section        */
    /* Set the start and finish for this section */
    start = 20;
    finish = 25;

    for (x = start; x < finish; x++) { /* Save the new spots */
        if (placed == false) { /* They Haven't Placed */
            if (strcasecmp(topname[x], GET_USER(ch))) { /* Name doesn't match */
                if (ch->getRPP() > toppoint[x]) {
                    free(topname[x]);
                    toppoint[x] = ch->getRPP();
                    topname[x] = strdup(GET_USER(ch));
                    placed = true;
                    writeEm = true;
                    location = x;
                }
            } else { /* This is their spot already */
                placed = true;
                location = finish;
            }
        } else { /* They have placed */
            if (x < finish && location < finish) {
                if (strcasecmp(positions[location], GET_USER(ch))) { /* This isn't their old spot */
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                } else { /* This IS their old spot */
                    progress = true;
                    location += 1;
                    free(topname[x]);
                    toppoint[x] = points[location];
                    topname[x] = strdup(positions[location]);
                    location += 1;
                }
            }
        }
    } /* End Save New Spots*/

    if (progress == true) {
        send_to_all("@D[@GToplist@W: @C%s @Whas moved up in rank in the RPP section.@D]\r\n", GET_USER(ch));
    } else if (placed == true && location != finish) {
        send_to_all("@D[@GToplist@W: @C%s @Whas placed in the RPP section.@D]\r\n", GET_USER(ch));
    }

    location = -1;
    placed = false;
    progress = false;

    for (x = 0; x < 25; x++) {
        free(positions[x]);
    }

    if (writeEm == true) {
        if (!get_filename(fname, sizeof(fname), INTRO_FILE, "toplist"))
            return;

        if (!(fl = fopen(fname, "w"))) {
            basic_mud_log("ERROR: could not save Toplist File, %s.", fname);
            return;
        }
        x = 0;
        while (x < 25) {
            fprintf(fl, "%s %" I64T "\n", topname[x], toppoint[x]);
            x++;
        }

        fclose(fl);
    }
    return;
}

int reserved_word(char *argument) {
    return (search_block(argument, reserved, true) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg) {
    char *begin = first_arg;

    if (!argument) {
        *first_arg = '\0';
        return (nullptr);
    }

    do {
        skip_spaces(&argument);

        first_arg = begin;
        while (*argument && !isspace(*argument)) {
            *(first_arg++) = LOWER(*argument);
            argument++;
        }

        *first_arg = '\0';
    } while (fill_word(begin));

    return (argument);
}


/*
 * one_word is like any_one_arg, except that words in quotes ("") are
 * considered one word.
 *
 * No longer ignores fill words.  -dak, 6 Jan 2003.
 */
char *one_word(char *argument, char *first_arg) {
    skip_spaces(&argument);

    if (*argument == '\"') {
        argument++;
        while (*argument && *argument != '\"') {
            *(first_arg++) = LOWER(*argument);
            argument++;
        }
        argument++;
    } else {
        while (*argument && !isspace(*argument)) {
            *(first_arg++) = LOWER(*argument);
            argument++;
        }
    }

    *first_arg = '\0';
    return (argument);
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg) {
    skip_spaces(&argument);

    while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
    }

    *first_arg = '\0';

    return (argument);
}

/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg) {
    return (one_argument(one_argument(argument, first_arg), second_arg)); /* :-) */
}

/*
 * Same as two_arguments only, well you get the idea... - Iovan
 *
 */
char *three_arguments(char *argument, char *first_arg, char *second_arg, char *third_arg) {
    return (one_argument(one_argument(one_argument(argument, first_arg), second_arg), third_arg)); /* >.> */
}


/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 *
 * returns 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(const char *arg1, const char *arg2) {
    if (!*arg1)
        return (0);

    for (; *arg1 && *arg2; arg1++, arg2++)
        if (LOWER(*arg1) != LOWER(*arg2))
            return (0);

    if (!*arg1)
        return (1);
    else
        return (0);
}

/*
 * Return first space-delimited token in arg1; remainder of string in arg2.
 *
 * NOTE: Requires sizeof(arg2) >= sizeof(string)
 */
void half_chop(char *string, char *arg1, char *arg2) {
    char *temp;

    temp = any_one_arg(string, arg1);
    skip_spaces(&temp);
    if (arg2 != temp)
        strcpy(arg2, temp);    /* strcpy: OK (documentation) */
}


/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(const char *command) {
    int cmd;
    for (cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
        if (!strcmp(complete_cmd_info[cmd].command, command))
            return (cmd);
    return (-1);
}


int special(struct char_data *ch, int cmd, char *arg) {
    /* special in room? */
    auto room = ch->getRoom();

    if (room && room->func)
        if (room->func(ch, ch->getRoom(), cmd, arg))
            return 1;

    /* special in equipment list? */
    for (auto j = 0; j < NUM_WEARS; j++) {
        if(auto obj = GET_EQ(ch, j); obj)
            if(auto func = GET_OBJ_SPEC(obj); func)
                if (func(ch, obj, cmd, arg))
                    return 1;
    }

    /* special in inventory? */
    for (auto obj : ch->getContents())
        if (auto func = GET_OBJ_SPEC(obj))
            if (func(ch, obj, cmd, arg))
                return 1;

    /* special in mobile present? */
    if(room) {
        for (auto mob : room->getPeople())
            if (IS_NPC(mob) && !MOB_FLAGGED(mob, MOB_NOTDEADYET))
                if (auto func = GET_MOB_SPEC(mob); func)
                    if(func(ch, mob, cmd, arg))
                        return 1;

        for (auto obj : room->getContents())
            if(auto func = GET_OBJ_SPEC(obj); func)
                if (func(ch, obj, cmd, arg))
                    return 1;
    }

    return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* This function needs to die. */
int _parse_name(char *arg, char *name) {
    int i;

    skip_spaces(&arg);
    for (i = 0; (*name = *arg); arg++, i++, name++)
        if (!isalpha(*arg))
            return (1);

    if (!i)
        return (1);

    return (0);
}


#define RECON        1
#define USURP        2
#define UNSWITCH    3

/* This function seems a bit over-extended. */
int perform_dupe_check(struct descriptor_data *d) {
    struct descriptor_data *k, *next_k;
    struct char_data *target = nullptr, *ch, *next_ch;
    int mode = 0;
    int count = 0, oldcount = HIGHPCOUNT;
    int id = GET_IDNUM(d->character);

    /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

    for (k = descriptor_list; k; k = next_k) {
        next_k = k->next;

        if (k == d)
            continue;

        if (k->original && (GET_IDNUM(k->original) == id)) {
            /* Original descriptor was switched, booting it and restoring normal body control. */

            write_to_output(d, "\r\nMultiple login detected -- disconnecting.\r\n");
            STATE(k) = CON_CLOSE;
            if (!target) {
                target = k->original;
                mode = UNSWITCH;
            }
            if (k->character)
                k->character->desc = nullptr;
            k->character = nullptr;
            k->original = nullptr;
        } else if (k->character && GET_IDNUM(k->character) == id && k->original) {
            /* Character taking over their own body, while an immortal was switched to it. */

            do_return(k->character, nullptr, 0, 0);
        } else if (k->character && GET_IDNUM(k->character) == id) {
            /* Character taking over their own body. */

            if (!target && STATE(k) == CON_PLAYING) {
                write_to_output(k, "\r\nThis body has been usurped!\r\n");
                if (k->snoop_by) {
                    k->snoop_by->snooping = d;
                    d->snoop_by = k->snoop_by;
                    k->snoop_by = nullptr;
                }
                target = k->character;
                mode = USURP;
            }
            k->character->desc = nullptr;
            k->character = nullptr;
            k->original = nullptr;
            write_to_output(k, "\r\nMultiple login detected -- disconnecting.\r\n");
            STATE(k) = CON_CLOSE;
        }
    }

    /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

    for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;

        if (IS_NPC(ch))
            continue;
        if (GET_IDNUM(ch) != id)
            continue;

        /* ignore chars with descriptors (already handled by above step) */
        if (ch->desc)
            continue;

        /* don't extract the target char we've found one already */
        if (ch == target)
            continue;

        /* we don't already have a target and found a candidate for switching */
        if (!target) {
            target = ch;
            mode = RECON;
            continue;
        }

        /* we've found a duplicate - blow him away, dumping his eq in limbo. */
        if (IN_ROOM(ch) != NOWHERE)
            char_from_room(ch);
        char_to_room(ch, 1);
        extract_char(ch);
    }

    /* no target for switching into was found - allow login to continue */
    if (!target)
        return (0);

    /* Okay, we've found a target.  Connect d to target. */
    free_char(d->character); /* get rid of the old char */
    d->character = target;
    d->character->desc = d;
    d->original = nullptr;
    d->character->timer = 0;
    for(auto f : {PLR_MAILING, PLR_WRITING}) d->character->pref.reset(f);
    d->character->affected_by.reset(AFF_GROUP);
    STATE(d) = CON_PLAYING;

    switch (mode) {
        case RECON:
            write_to_output(d, "Reconnecting.\r\n");
            /*~~~ For PCOUNT and HIGHPCOUNT ~~~*/
            count = 0, oldcount = HIGHPCOUNT;
            struct descriptor_data *k;

            for (k = descriptor_list; k; k = k->next) {
                if (!IS_NPC(k->character) && GET_LEVEL(k->character) > 3) {
                    count += 1;
                }

                if (count > PCOUNT) {
                    PCOUNT = count;
                }

                if (PCOUNT >= HIGHPCOUNT) {
                    oldcount = HIGHPCOUNT;
                    HIGHPCOUNT = PCOUNT;
                    PCOUNTDATE = time(nullptr);
                }

            }
            if (PCOUNT < HIGHPCOUNT && PCOUNT >= HIGHPCOUNT - 4) {
                payout(0);
            }
            if (PCOUNT == HIGHPCOUNT) {
                payout(1);
            }
            if (PCOUNT > oldcount) {
                payout(2);
            }
            /*~~~ End PCOUNT and HIGHPCOUNT ~~~*/
            d->character->time.logon = time(nullptr);
            act("$n has reconnected.", true, d->character, nullptr, nullptr, TO_ROOM);
            mudlog(NRM, MAX(ADMLVL_NONE, GET_INVIS_LEV(d->character)), true, "%s [%s] has reconnected.",
                   GET_NAME(d->character), d->host);

            if (has_mail(GET_IDNUM(d->character)))
                write_to_output(d, "You have mail waiting.\r\n");
            if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWIMM > GET_BOARD(d->character, 1))
                send_to_char(d->character,
                             "\r\n@GMake sure to check the immortal board, there is a new post there.@n\r\n");
            if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWCOD > GET_BOARD(d->character, 2))
                send_to_char(d->character, "\r\n@GMake sure to check the request file, it has been updated.@n\r\n");
            if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWBUI > GET_BOARD(d->character, 4))
                send_to_char(d->character,
                             "\r\n@GMake sure to check the builder board, there is a new post there.@n\r\n");
            if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWDUO > GET_BOARD(d->character, 3))
                send_to_char(d->character,
                             "\r\n@GMake sure to check punishment board, there is a new post there.@n\r\n");
            if (BOARDNEWMORT > GET_BOARD(d->character, 0))
                send_to_char(d->character, "\r\n@GThere is a new bulletin board post.@n\r\n");
            if (NEWSUPDATE > GET_LPLAY(d->character))
                send_to_char(d->character,
                             "\r\n@GThe NEWS file has been updated, type 'news %d' to see the latest entry or 'news list' to see available entries.@n\r\n",
                             LASTNEWS);
            if (LASTINTEREST != 0 && LASTINTEREST > GET_LINTEREST(d->character)) {
                int diff = (LASTINTEREST - GET_LINTEREST(d->character));
                int mult = 0;
                while (diff > 0) {
                    if ((diff - 86400) < 0 && mult == 0) {
                        mult = 1;
                    } else if ((diff - 86400) >= 0) {
                        diff -= 86400;
                        mult++;
                    } else {
                        diff = 0;
                    }
                }
                if (mult > 3) {
                    mult = 3;
                }
                GET_LINTEREST(d->character) = LASTINTEREST;
                if (GET_BANK_GOLD(d->character) > 0) {
                    int inc = ((GET_BANK_GOLD(d->character) / 100) * 2);
                    if (inc >= 7500) {
                        inc = 7500;
                    }
                    inc *= mult;
                    d->character->mod(CharMoney::Bank, inc);
                    send_to_char(d->character, "Interest happened while you were away, %d times.\r\n"
                                               "@cBank Interest@D: @Y%s@n\r\n", mult, add_commas(inc).c_str());
                }
            }
            break;
        case USURP:
            write_to_output(d, "You take over your own body, already in use!\r\n");
            act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
                "$n's body has been taken over by a new spirit!",
                true, d->character, nullptr, nullptr, TO_ROOM);

            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(d->character)), true,
                   "%s has re-logged in ... disconnecting old socket.", GET_NAME(d->character));
            break;
        case UNSWITCH:
            write_to_output(d, "Reconnecting to unswitched char.");
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(d->character)), true, "%s [%s] has reconnected.",
                   GET_NAME(d->character), d->host);
            break;
    }

    return (1);
}

/* load the player, put them in the right room - used by copyover_recover too */
void enter_player_game(struct descriptor_data *d) {
    IDXTYPE load_room;
    struct char_data *check;

    d->character->timer = 0;
    reset_char(d->character);
    if(!d->character->script) d->character->script = new script_data(d->character);

    racial_body_parts(d->character);

    if (PLR_FLAGGED(d->character, PLR_INVSTART))
        GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);

    /*
       * We have to place the character in a room before equipping them
       * or equip_char() will gripe about the person in NOWHERE.
       */

    if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
        load_room = real_room(load_room);

    /* If char was saved with NOWHERE, or real_room above failed... */
    if (load_room == NOWHERE) {
        if (GET_ADMLEVEL(d->character))
            load_room = real_room(CONFIG_IMMORTAL_START);
        else
            load_room = real_room(CONFIG_MORTAL_START);
    }

    if (PLR_FLAGGED(d->character, PLR_FROZEN))
        load_room = real_room(CONFIG_FROZEN_START);

    d->character->activate();
    char_to_room(d->character, load_room);

    /*load_char_pets(d->character);*/
    for (check = character_list; check; check = check->next)
        if (!check->master && IS_NPC(check) && check->master_id == GET_IDNUM(d->character) &&
            AFF_FLAGGED(check, AFF_CHARM) && !circle_follow(check, d->character))
            add_follower(check, d->character);

    GET_COMBINE(d->character) = -1;
    GET_SLEEPT(d->character) = 8;
    GET_FOODR(d->character) = 2;
    if (AFF_FLAGGED(d->character, AFF_FLYING)) {
        GET_ALT(d->character) = 1;
    } else {
        GET_ALT(d->character) = 0;
    }

    for(auto f : {AFF_POSITION, AFF_SANCTUARY, AFF_ZANZOKEN}) d->character->affected_by.reset(f);
    d->character->playerFlags.reset(PLR_KNOCKED);

    if (IS_ANDROID(d->character) && !AFF_FLAGGED(d->character, AFF_INFRAVISION)) {
        d->character->affected_by.set(AFF_INFRAVISION);
    }

    ABSORBING(d->character) = nullptr;
    ABSORBBY(d->character) = nullptr;
    SITS(d->character) = nullptr;
    BLOCKED(d->character) = nullptr;
    BLOCKS(d->character) = nullptr;
    GET_OVERFLOW(d->character) = false;
    GET_SPAM(d->character) = 0;
    GET_RMETER(d->character) = 0;
    if (!d->character->affected) {
        d->character->affected_by.reset(AFF_HEALGLOW);
    }
    if (AFF_FLAGGED(d->character, AFF_HAYASA)) {
        GET_SPEEDBOOST(d->character) = GET_SPEEDCALC(d->character) * 0.5;
    } else {
        GET_SPEEDBOOST(d->character) = 0;
    }

    d->character->playerFlags.reset(PLR_HEALT);

    if (GET_ADMLEVEL(d->character) > 0) {
        d->level = 1;
    }

    if (GET_CLAN(d->character) != nullptr && !strstr(GET_CLAN(d->character), "None")) {
        if (!clanIsMember(GET_CLAN(d->character), d->character)) {
            if (!clanIsModerator(GET_CLAN(d->character), d->character)) {
                if (!checkCLAN(d->character)) {
                    write_to_output(d, "Your clan no longer exists.\r\n");
                    GET_CLAN(d->character) = strdup("None.");
                }
            }
        }
    }

    if (IS_HOSHIJIN(d->character)) {
        if (time_info.day <= 14) {
            star_phase(d->character, 1);
        } else if (time_info.day <= 21) {
            star_phase(d->character, 2);
        } else {
            star_phase(d->character, 0);
        }
    }

    if (IS_ICER(d->character) && !GET_SKILL(d->character, SKILL_TAILWHIP)) {
        int numb = rand_number(20, 30);
        SET_SKILL(d->character, SKILL_TAILWHIP, numb);
    } else if (!IS_ICER(d->character) && GET_SKILL(d->character, SKILL_TAILWHIP)) {
        SET_SKILL(d->character, SKILL_TAILWHIP, 0);
    }

    if (IS_MUTANT(d->character) && (GET_GENOME(d->character, 0) == 9 || GET_GENOME(d->character, 1) == 9) &&
        !GET_SKILL(d->character, SKILL_TELEPATHY)) {
        SET_SKILL(d->character, SKILL_TELEPATHY, 50);
    }

    if (IS_BIO(d->character) && (GET_GENOME(d->character, 0) == 7 || GET_GENOME(d->character, 1) == 7) &&
        !GET_SKILL(d->character, SKILL_TELEPATHY) && !GET_SKILL(d->character, SKILL_FOCUS)) {
        SET_SKILL(d->character, SKILL_TELEPATHY, 30);
        SET_SKILL(d->character, SKILL_FOCUS, 30);
    }

    COMBO(d->character) = -1;
}

int readUserIndex(char *name) {
    char fname[40];
    FILE *fl;

    /* Read User Index */
    if (!get_filename(fname, sizeof(fname), USER_FILE, name)) {
        return 0;
    } else if (!(fl = fopen(fname, "r"))) {
        return 0;
    }
    fclose(fl);
    return 1;
}

void payout(int num) {

    struct descriptor_data *k;
    if (LASTPAYOUT == 0) {
        LASTPAYOUT = time(nullptr) + 86400;
        LASTPAYTYPE = num;
    } else if (num > LASTPAYTYPE) {
        LASTPAYOUT = time(nullptr) + 86400;
        LASTPAYTYPE = num;
    } else if (LASTPAYOUT <= time(nullptr)) {
        LASTPAYOUT = time(nullptr) + 86400;
        LASTPAYTYPE = num;
    }
    for (k = descriptor_list; k; k = k->next) {
        if (GET_ADMLEVEL(k->character) <= 0 && IS_PLAYING(k) && GET_RTIME(k->character) < LASTPAYOUT) {
            if (num == 0) {
                k->account->modRPP(1);
                send_to_char(k->character,
                             "@D[@G+ 1 RPP@D] @cA total logon count within 4 of the highest has been achieved.@n\r\n");
            } else if (num == 1) {
                k->account->modRPP(2);
                send_to_char(k->character,
                             "@D[@G+ 2 RPP@D] @cThe total logon count has tied with the highest ever.@n\r\n");
            } else {
                k->account->modRPP(3);
                send_to_char(k->character, "@D[@G+ 3 RPP@D] @cA new logon count record has been achieved!@n\r\n");
            }
            GET_RTIME(k->character) = LASTPAYOUT;
        }
    }
}

int command_pass(char *cmd, struct char_data *ch) {

    if (AFF_FLAGGED(ch, AFF_LIQUEFIED)) {
        if (strcasecmp(cmd, "liquefy") && strcasecmp(cmd, "ingest") && strcasecmp(cmd, "look") &&
            strcasecmp(cmd, "score") && strcasecmp(cmd, "ooc") && strcasecmp(cmd, "osay") && strcasecmp(cmd, "emote") &&
            strcasecmp(cmd, "smote") && strcasecmp(cmd, "status")) {
            send_to_char(ch, "You are not capable of performing that action while liquefied!\r\n");
            return (false);
        }
    } else if (IS_AFFECTED(ch, AFF_PARALYZE)) {
        if (strcasecmp(cmd, "look") && strcasecmp(cmd, "score") && strcasecmp(cmd, "ooc") && strcasecmp(cmd, "osay") &&
            strcasecmp(cmd, "emote") && strcasecmp(cmd, "smote") && strcasecmp(cmd, "status")) {
            send_to_char(ch, "You are not capable of performing that action while petrified!\r\n");
            return (false);
        }
    } else if (IS_AFFECTED(ch, AFF_FROZEN)) {
        if (strcasecmp(cmd, "look") && strcasecmp(cmd, "score") && strcasecmp(cmd, "ooc") && strcasecmp(cmd, "osay") &&
            strcasecmp(cmd, "emote") && strcasecmp(cmd, "smote") && strcasecmp(cmd, "status")) {
            send_to_char(ch, "You are not capable of performing that action while a frozen block of ice!\r\n");
            return (false);
        }
    } else if (IS_AFFECTED(ch, AFF_PARA) && GET_INT(ch) < rand_number(1, 60)) {
        if (strcasecmp(cmd, "look") && strcasecmp(cmd, "score") && strcasecmp(cmd, "ooc") && strcasecmp(cmd, "osay") &&
            strcasecmp(cmd, "emote") && strcasecmp(cmd, "smote") && strcasecmp(cmd, "status")) {
            act("@yYou fail to overcome your paralysis!@n", true, ch, nullptr, nullptr, TO_CHAR);
            act("@Y$n @ystruggles with $s paralysis!@n", true, ch, nullptr, nullptr, TO_ROOM);
            return (false);
        }
    }

    return (true);
}

int lockRead(char *name) {
    char fname[40], filler[50], line[256];
    int known = false;
    FILE *fl;

    /* Read Introduction File */

    if (!get_filename(fname, sizeof(fname), INTRO_FILE, "lockout")) {
        return 0;
    } else if (!(fl = fopen(fname, "r"))) {
        return 0;
    }

    while (!feof(fl)) {
        get_line(fl, line);
        sscanf(line, "%s\n", filler);
        if (!strcasecmp(CAP(name), CAP(filler))) {
            known = true;
        }
    }
    fclose(fl);

    if (known == true)
        return 1;
    else
        return 0;
}

/* For transfering money or doing things with an offline player */
char *rIntro(struct char_data *ch, char *arg) {
    char fname[40], filler[50], scrap[100], line[256];
    static char name[80];
    int known = false;
    FILE *fl;

    /* Read Introduction File */
    if (IS_NPC(ch)) {
        return "NOTHING";
    }

    if (!get_filename(fname, sizeof(fname), INTRO_FILE, GET_NAME(ch))) {
        return "NOTHING";
    } else if (!(fl = fopen(fname, "r"))) {
        return "NOTHING";
    }

    while (!feof(fl)) {
        get_line(fl, line);
        sscanf(line, "%s %s\n", filler, scrap);
        if (!strcasecmp(arg, scrap)) {
            known = true;
            sprintf(name, "%s", filler);
        }
    }
    fclose(fl);

    if (known == true)
        return (name);
    else
        return "NOTHING";
}


void fingerUser(struct char_data *ch, struct account_data *account) {
    send_to_char(ch, "@D[@gUsername   @D: @w%-30s@D]@n\r\n", account->name.c_str());
    send_to_char(ch, "@D[@gEmail      @D: @w%-30s@D]@n\r\n", account->email.c_str());
    send_to_char(ch, "@D[@gTotal Slots@D: @w%-30d@D]@n\r\n", account->slots);
    send_to_char(ch, "@D[@gRP Points  @D: @w%-30d@D]@n\r\n", account->rpp);

    if (GET_ADMLEVEL(ch) > 0) {
        int counter = 0;
        for(auto c : account->characters) {
            auto p = players.find(c);
            if(p == players.end()) continue;
            send_to_char(ch, "@D[@gCh. Slot %d @D: @w%-30s@D]@n\r\n", ++counter, p->second.character->name);
        }
        send_to_char(ch, "\n");
    }
}

/* Return -1 if not an acceptable menu option *
 * Return 31 if selection is X                 *
 * Return other value if Bonus/Negative        */


/* Handle CC point exchange for Bonus/negative */

static struct {
    int state;
    void (*func)(struct descriptor_data *, char *);
} olc_functions[] = {
        {CON_OEDIT,    oedit_parse},
        {CON_IEDIT,    oedit_parse},
        {CON_ZEDIT,    zedit_parse},
        {CON_SEDIT,    sedit_parse},
        {CON_MEDIT,    medit_parse},
        {CON_REDIT,    redit_parse},
        {CON_CEDIT,    cedit_parse},
        {CON_AEDIT,    aedit_parse},
        {CON_TRIGEDIT, trigedit_parse},
        {CON_ASSEDIT,  assedit_parse},
        {CON_GEDIT,    gedit_parse},
        {CON_LEVELUP,  levelup_parse},
        {CON_HEDIT,    hedit_parse},
        {CON_HSEDIT,   hsedit_parse},
        {CON_POBJ,     pobj_edit_parse},
        {-1,           nullptr}
};

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg) {
    int load_result = -1;    /* Overloaded variable */
    int total, rr, moveon = false, penalty = false;
    int player_i;
    int value, roll = rand_number(1, 6); /* For parse_bonuses */
    struct descriptor_data *k;

    int count = 0, oldcount = HIGHPCOUNT;
    /* OasisOLC states */

    skip_spaces(&arg);

    /*
   * Quick check for the OLC states.
   */
    for (player_i = 0; olc_functions[player_i].state >= 0; player_i++)
        if (STATE(d) == olc_functions[player_i].state) {
            /* send context-sensitive help if need be */
            if (context_help(d, arg)) return;
            (*olc_functions[player_i].func)(d, arg);
            return;
        }

    /* Not in OLC. */
    switch (STATE(d)) {

        case CON_CLOSE:
        case CON_DISCONNECT:
            break;

        case CON_ASSEDIT:
            assedit_parse(d, arg);
            break;

        case CON_GEDIT:
            gedit_parse(d, arg);
            break;

        default:
            basic_mud_log("SYSERR: Nanny: illegal state of con'ness (%d) for '%s'; closing connection.",
                STATE(d), d->character ? GET_NAME(d->character) : "<unknown>");
            STATE(d) = CON_DISCONNECT;    /* Safest to do. */
            break;
    }
}

/*
 * Code to disable or enable buggy commands on the run, saving
 * a list of disabled commands to disk. Originally created by
 * Erwin S. Andreasen (erwin@andreasen.org) for Merc. Ported to
 * CircleMUD by Alexei Svitkine (Myrdred), isvitkin@sympatico.ca.
 *
 * Syntax is:
 *   disable - shows disabled commands
 *   disable <command> - toggles disable status of command
 *
 */

ACMD(do_disable) {
    int i, length;
    DISABLED_DATA *p, *temp;

    if (IS_NPC(ch)) {
        send_to_char(ch, "Monsters can't disable commands, silly.\r\n");
        return;
    }

    skip_spaces(&argument);

    if (!*argument) { /* Nothing specified. Show disabled commands. */
        if (!disabled_first) /* Any disabled at all ? */
            send_to_char(ch, "There are no disabled commands.\r\n");
        else {
            send_to_char(ch,
                         "Commands that are currently disabled:\r\n\r\n"
                         " Command       Disabled by     Level\r\n"
                         "-----------   --------------  -------\r\n");
            for (p = disabled_first; p; p = p->next)
                send_to_char(ch, " %-12s   %-12s    %3d\r\n", p->command->command, p->disabled_by, p->level);
        }
        return;
    }

    /* command given - first check if it is one of the disabled commands */
    for (length = strlen(argument), p = disabled_first; p; p = p->next)
        if (!strncmp(argument, p->command->command, length))
            break;

    if (p) { /* this command is disabled */

        /* Was it disabled by a higher level imm? */
        if (GET_ADMLEVEL(ch) < p->level) {
            send_to_char(ch, "This command was disabled by a higher power.\r\n");
            return;
        }

        REMOVE_FROM_LIST(p, disabled_first, next, temp);
        send_to_char(ch, "Command '%s' enabled.\r\n", p->command->command);
        mudlog(BRF, ADMLVL_IMMORT, true, "(GC) %s has enabled the command '%s'.",
               GET_NAME(ch), p->command->command);
        free(p->disabled_by);
        free(p);
        save_disabled(); /* save to disk */

    } else { /* not a disabled command, check if the command exists */

        for (length = strlen(argument), i = 0; *cmd_info[i].command != '\n'; i++)
            if (!strncmp(cmd_info[i].command, argument, length))
                if (GET_LEVEL(ch) >= cmd_info[i].minimum_level &&
                    GET_ADMLEVEL(ch) >= cmd_info[i].minimum_admlevel)
                    break;

        /*  Found?     */
        if (*cmd_info[i].command == '\n') {
            send_to_char(ch, "You don't know of any such command.\r\n");
            return;
        }

        if (!strcmp(cmd_info[i].command, "disable")) {
            send_to_char(ch, "You cannot disable the disable command.\r\n");
            return;
        }

        /* Disable the command */
        CREATE(p, struct disabled_data, 1);
        p->command = &cmd_info[i];
        p->disabled_by = strdup(GET_NAME(ch)); /* save name of disabler  */
        p->level = GET_ADMLEVEL(ch);           /* save level of disabler */
        p->subcmd = cmd_info[i].subcmd;       /* the subcommand if any  */
        p->next = disabled_first;
        disabled_first = p; /* add before the current first element */
        send_to_char(ch, "Command '%s' disabled.\r\n", p->command->command);
        mudlog(BRF, ADMLVL_IMMORT, true, "(GC) %s has disabled the command '%s'.",
               GET_NAME(ch), p->command->command);
        save_disabled(); /* save to disk */
    }
}

/* check if a command is disabled */
int check_disabled(const struct command_info *command) {
    DISABLED_DATA *p;

    for (p = disabled_first; p; p = p->next)
        if (p->command->command_pointer == command->command_pointer)
            if (p->command->subcmd == command->subcmd)
                return true;

    return false;
}

/* Load disabled commands */
void load_disabled() {
    FILE *fp;
    DISABLED_DATA *p;
    int i;
    char line[READ_SIZE], name[MAX_INPUT_LENGTH], temp[MAX_INPUT_LENGTH];

    if (disabled_first)
        free_disabled();

    if ((fp = fopen(DISABLED_FILE, "r")) == nullptr)
        return; /* No disabled file.. no disabled commands. */

    while (get_line(fp, line)) {
        if (!strcasecmp(line, END_MARKER))
            break; /* break loop if we encounter the END_MARKER */
        CREATE(p, struct disabled_data, 1);
        sscanf(line, "%s %d %hd %s", name, &(p->subcmd), &(p->level), temp);
        /* Find the command in the table */
        for (i = 0; *cmd_info[i].command != '\n'; i++)
            if (!strcasecmp(cmd_info[i].command, name))
                break;
        if (*cmd_info[i].command == '\n') { /* command does not exist? */
            basic_mud_log("WARNING: load_disabled(): Skipping unknown disabled command - '%s'!", name);
            free(p);
        } else { /* add new disabled command */
            p->disabled_by = strdup(temp);
            p->command = &cmd_info[i];
            p->next = disabled_first;
            disabled_first = p;
        }
    }
    fclose(fp);
}

/* Save disabled commands */
void save_disabled() {
    FILE *fp;
    DISABLED_DATA *p;

    if (!disabled_first) {
        /* delete file if no commands are disabled */
        std::filesystem::remove(DISABLED_FILE);
        return;
    }

    if ((fp = fopen(DISABLED_FILE, "w")) == nullptr) {
        basic_mud_log("SYSERR: Could not open " DISABLED_FILE " for writing");
        return;
    }

    for (p = disabled_first; p; p = p->next)
        fprintf(fp, "%s %d %d %s\n", p->command->command, p->subcmd, p->level, p->disabled_by);
    fprintf(fp, "%s\n", END_MARKER);
    fclose(fp);
}

/* free all disabled commands from memory */
void free_disabled() {
    DISABLED_DATA *p;

    while (disabled_first) {
        p = disabled_first;
        disabled_first = disabled_first->next;
        free(p->disabled_by);
        free(p);
    }
}

