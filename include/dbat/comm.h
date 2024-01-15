/* ************************************************************************
*   File: comm.h                                        Part of CircleMUD *
*  Usage: header file: prototypes of public communication functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#include "structs.h"

#if CIRCLE_GNU_LIBC_MEMORY_TRACK
# include <mcheck.h>
#endif

/*
 * Note, most includes for all platforms are in sysdep.h.  The list of
 * files that is included is controlled by conf.h for that platform.
 */

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

/* mccp defines */
#define COMPRESS2 86

#define NUM_RESERVED_DESCS    8
#define COPYOVER_FILE "copyover.dat"

/* comm.c */
void enter_player_game(struct descriptor_data *d);

extern int arena_watch(struct char_data *ch);

extern void
perform_act(const char *orig, struct char_data *ch, struct obj_data *obj, const void *vict_obj, struct char_data *to);

extern char *
act(const char *str, int hide_invisible, struct char_data *ch, struct obj_data *obj, const void *vict_obj, int type);

extern void close_socket(struct descriptor_data *d);

#define TO_ROOM        1
#define TO_VICT        2
#define TO_NOTVICT    3
#define TO_CHAR        4
#define TO_GMOTE    5
#define TO_SLEEP        (2 << 7)    /* to char, even if sleeping */
#define DG_NO_TRIG      (2 << 8)    /* don't check act trigger   */
#define TO_SNEAKRESIST  (2 << 9)    /* resisted sneaking roll    */
#define TO_HIDERESIST   (2 << 10)    /* resisted hiding roll      */

/* I/O functions */
extern void write_to_q(const char *txt, struct txt_q *queue, int aliased);

extern std::function<void>()> gameFunc;

extern void string_add(struct descriptor_data *d, char *str);

extern void string_write(struct descriptor_data *d, char **txt, size_t len, long mailto, void *data);

#define PAGE_LENGTH    22
#define PAGE_WIDTH    79


extern void show_help(std::shared_ptr<net::Connection>& co, const char *entry);

/* variables */
extern uint64_t pulse;
extern FILE *logfile;
extern int circle_shutdown, circle_reboot;
extern socklen_t mother_desc;
extern uint16_t port;
extern int buf_switches, buf_largecount, buf_overflows;
extern int no_specials, scheck;
extern bool fCopyOver;
extern char *last_act_message;
extern const char RANDOM_COLORS[];
extern const char CCODE[];
extern char *ANSI[];

// functions
extern int process_output(struct descriptor_data *t);

extern void record_usage(uint64_t heartPulse, double deltaTime);

extern char *make_prompt(struct descriptor_data *point);

extern void free_bufpool();


void broadcast(const std::string& txt);

void shutdown_game(int code);

namespace game {
    void init_locale();
    bool init_sodium();
    void init_database();
    void init_zones();
    void run_loop_once(double deltaTime);
}