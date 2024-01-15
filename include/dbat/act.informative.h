#pragma once

#include "structs.h"

// defines
#define MAX_PORTAL_TYPES        6
/* For show_obj_to_char 'mode'.	/-- arbitrary */
#define SHOW_OBJ_LONG        0
#define SHOW_OBJ_SHORT        1
#define SHOW_OBJ_ACTION        2

#define HIST_LENGTH 100

// global variables
extern int *cmd_sort_info;
extern char *default_color_choices[NUM_COLOR + 1];

// functions
extern int readIntro(struct char_data *ch, struct char_data *vict);

extern int check_disabled(const struct command_info *command);

extern void sort_commands();

extern char *find_exdesc(char *word, struct extra_descr_data *list);

extern void add_history(struct char_data *ch, char *str, int type);

extern void introWrite(struct char_data *ch, struct char_data *vict, char *name);

extern void look_at_room(struct room_data *rm, struct char_data *ch, int ignore_brief);
extern void look_at_room(room_rnum target_room, struct char_data *ch, int ignore_brief);

extern int perf_skill(int skill);

extern int search_help(const char *argument, int level);


// commands
extern ACMD(do_desc);

extern ACMD(do_look);

extern ACMD(do_examine);

extern ACMD(do_gold);

extern ACMD(do_score);

extern ACMD(do_status);

extern ACMD(do_inventory);

extern ACMD(do_equipment);

extern ACMD(do_time);

extern ACMD(do_weather);

extern ACMD(do_help);

extern ACMD(do_who);

extern ACMD(do_users);

extern ACMD(do_gen_ps);

extern ACMD(do_where);

extern ACMD(do_levels);

extern ACMD(do_consider);

extern ACMD(do_diagnose);

extern ACMD(do_color);

extern ACMD(do_toggle);

extern ACMD(do_commands);

extern ACMD(do_exits);

extern ACMD(do_autoexit);

extern ACMD(do_history);

extern ACMD(do_map);

extern ACMD(do_rptrans);

extern ACMD(do_finger);

extern ACMD(do_perf);

extern ACMD(do_nickname);

extern ACMD(do_table);

extern ACMD(do_play);

extern ACMD(do_post);

extern ACMD(do_hand);

extern ACMD(do_shuffle);

extern ACMD(do_draw);

extern ACMD(do_kyodaika);

extern ACMD(do_mimic);

extern ACMD(do_rdisplay);

extern ACMD(do_evolve);

extern ACMD(do_showoff);

extern ACMD(do_intro);

extern ACMD(do_scan);

extern ACMD(do_toplist);

extern ACMD(do_whois);

extern ACMD(do_oaffects);