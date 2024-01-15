/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#include "structs.h"
#include "commands.h"

#define CMD_NAME (complete_cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, complete_cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (complete_cmd_info[cmdnum].command_pointer == do_move)

extern const char *list_bonus[];

extern void perform_alias(struct descriptor_data *d, char *orig);

extern void topLoad(void);

extern void topWrite(struct char_data *ch);

extern char *rIntro(struct char_data *ch, char *arg);

extern int special(struct char_data *ch, int cmd, char *arg);

void payout(int num);
int lockRead(char *name);
extern void fingerUser(struct char_data *ch, struct account_data *acc);

extern int readUserIndex(char *name);

extern void command_interpreter(struct char_data *ch, char *argument);

extern int search_block(char *arg, const char **list, int exact);

extern char lower(char c);

extern char *one_argument(char *argument, char *first_arg);

extern char *one_word(char *argument, char *first_arg);

extern char *any_one_arg(char *argument, char *first_arg);

extern char *two_arguments(char *argument, char *first_arg, char *second_arg);

extern char *three_arguments(char *argument, char *first_arg, char *second_arg, char *third_arg);

extern int fill_word(char *argument);

extern void half_chop(char *string, char *arg1, char *arg2);

extern void nanny(struct descriptor_data *d, char *arg);

extern int is_abbrev(const char *arg1, const char *arg2);

extern int is_number(const char *str);

extern int find_command(const char *command);

extern void skip_spaces(char **string);

extern char *delete_doubledollar(char *string);

/* WARNING: if you have added diagonal directions and have them at the
 * beginning of the command list.. change this value to 11 or 15 (depending) */
/* reserve these commands to come straight from the cmd list then start
 * sorting */
#define RESERVE_CMDS               15

/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)


/*
 * Necessary for CMD_IS macro.  Borland needs the structure defined first
 * so it has been moved down here.
 */
extern struct command_info *complete_cmd_info;

/*
 * Alert! Changed from 'struct alias' to 'struct alias_data' in bpl15
 * because a Windows 95 compiler gives a warning about it having similiar
 * named member.
 */


