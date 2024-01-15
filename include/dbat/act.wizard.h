#pragma once

#include "structs.h"

/* global variables */


// functions
extern void search_replace(char *string, const char *find, const char *replace);

extern void update_space(void);

extern room_rnum find_target_room(struct char_data *ch, char *rawroomstr);

extern void perform_immort_vis(struct char_data *ch);

extern void snoop_check(struct char_data *ch);

extern void copyover_check(uint64_t heartPulse, double deltaTime);

// commands
extern ACMD(do_echo);

extern ACMD(do_send);

extern ACMD(do_at);

extern ACMD(do_goto);

extern ACMD(do_trans);

extern ACMD(do_teleport);

extern ACMD(do_vnum);

extern ACMD(do_stat);

extern ACMD(do_shutdown);

extern ACMD(do_recall);

extern ACMD(do_snoop);

extern ACMD(do_switch);

extern ACMD(do_return);

extern ACMD(do_load);

extern ACMD(do_vstat);

extern ACMD(do_purge);

extern ACMD(do_syslog);

extern ACMD(do_advance);

extern ACMD(do_restore);

extern ACMD(do_invis);

extern ACMD(do_gecho);

extern ACMD(do_poofset);

extern ACMD(do_dc);

extern ACMD(do_wizlock);

extern ACMD(do_date);

extern ACMD(do_last);

extern ACMD(do_force);

extern ACMD(do_wiznet);

extern ACMD(do_zreset);

extern ACMD(do_wizutil);

extern ACMD(do_show);

extern ACMD(do_set);

extern ACMD(do_saveall);

extern ACMD(do_wizupdate);

extern ACMD(do_chown);

extern ACMD(do_zpurge);

extern ACMD(do_zcheck);

extern ACMD(do_checkloadstatus);

extern ACMD(do_spells);

extern ACMD(do_finddoor);

extern ACMD(do_interest);

extern ACMD(do_transobj);

extern ACMD(do_permission);

extern ACMD(do_reward);

extern ACMD(do_approve);

extern ACMD(do_newsedit);

extern ACMD(do_news);

extern ACMD(do_lag);

extern ACMD(do_hell);

extern ACMD(do_varstat);

extern ACMD(do_handout);

extern ACMD(do_ginfo);

extern ACMD(do_plist);

extern ACMD(do_peace);

extern ACMD(do_raise);

extern ACMD(do_boom);
