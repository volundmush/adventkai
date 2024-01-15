/************************************************************************
 * Generic OLC Library - General / genolc.h			v1.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/
#pragma once

#include "structs.h"
#include "db.h"

#define STRING_TERMINATOR       '~'

#define CONFIG_GENOLC_MOBPROG    0

extern int genolc_checkstring(struct descriptor_data *d, char *arg);

extern int remove_from_save_list(zone_vnum, int type);

extern int add_to_save_list(zone_vnum, int type);

extern int in_save_list(zone_vnum, int type);

extern void strip_cr(char *);

extern void save_all();

extern char *str_udup(const char *);

extern void copy_ex_descriptions(struct extra_descr_data **to, struct extra_descr_data *from);

extern void free_ex_descriptions(struct extra_descr_data *head);

extern int sprintascii(char *out, bitvector_t bits);

struct save_list_data {
    zone_vnum zone;
    int type;
};

extern std::list<struct save_list_data> save_list;

/* save_list_data.type */
#define SL_MOB    0
#define SL_OBJ    1
#define SL_SHP    2
#define SL_WLD    3
#define SL_ZON    4
#define SL_CFG    5
#define SL_GLD    6
#define SL_MAX    SL_GLD
#define SL_ACT  SL_MAX + 1 /* must be above MAX */
#define SL_HLP  SL_MAX + 2 /* must be above MAX */

#define ZCMD(zon, cmds)    zone_table[(zon)].cmd[(cmds)]

#define LIMIT(var, low, high)    std::clamp<int64_t>(var, low, high)

extern room_vnum genolc_zone_bottom(zone_rnum rznum);

extern room_vnum genolc_zonep_bottom(struct zone_data *zone);

extern void create_world_index(int znum, const char *type);
