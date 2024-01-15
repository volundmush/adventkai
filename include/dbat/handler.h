/* ************************************************************************
*   File: handler.h                                     Part of CircleMUD *
*  Usage: header file: prototypes of handling and utility functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#include "structs.h"

/* handling the affected-structures */
extern void update_char_objects(struct char_data *ch);    /* handler.c */
extern void item_check(struct obj_data *object, struct char_data *ch);

extern void affect_total(struct char_data *ch);

extern void affect_modify(struct char_data *ch, int loc, int mod, int spec, long bitv, bool add);

extern void affect_to_char(struct char_data *ch, struct affected_type *af);

extern void affect_remove(struct char_data *ch, struct affected_type *af);

extern void affect_from_char(struct char_data *ch, int type);

extern bool affected_by_spell(struct char_data *ch, int type);

extern bool affectedv_by_spell(struct char_data *ch, int type);

extern void affect_join(struct char_data *ch, struct affected_type *af,
                        bool add_dur, bool avg_dur, bool add_mod, bool avg_mod);

extern void affectv_join(struct char_data *ch, struct affected_type *af,
                         bool add_dur, bool avg_dur, bool add_mod, bool avg_mod);

extern void affectv_remove(struct char_data *ch, struct affected_type *af);

extern void affectv_to_char(struct char_data *ch, struct affected_type *af);

extern void affectv_from_char(struct char_data *ch, int type);


/* utility */
const char *money_desc(int amount);

struct obj_data *create_money(int amount);

extern int isname(const char *str, const char *namelist);

extern int is_name(const char *str, const char *namelist);

extern char *fname(const char *namelist);

extern int get_number(char **name);

const char *get_i_name(struct char_data *ch, struct char_data *vict);

/* ******** objects *********** */

extern void obj_to_char(struct obj_data *object, struct char_data *ch);

extern void obj_from_char(struct obj_data *object);

extern void equip_char(struct char_data *ch, struct obj_data *obj, int pos);

struct obj_data *unequip_char(struct char_data *ch, int pos);

extern int invalid_align(struct char_data *ch, struct obj_data *obj);

extern void obj_to_room(struct obj_data *object, struct room_data *room);
extern void obj_to_room(struct obj_data *object, room_rnum room);

extern void obj_from_room(struct obj_data *object);

extern void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);

extern void obj_from_obj(struct obj_data *obj);

extern void object_list_new_owner(struct obj_data *list, struct char_data *ch);

extern void extract_obj(struct obj_data *obj);

/* ******* characters ********* */

struct char_data *get_char_room(char *name, int *num, room_rnum room);

struct char_data *get_char_num(mob_rnum nr);

extern void char_from_room(struct char_data *ch);

extern void char_to_room(struct char_data *ch, struct room_data *room);
extern void char_to_room(struct char_data *ch, room_rnum room);

extern void extract_char(struct char_data *ch);

extern void extract_char_final(struct char_data *ch);

extern void extract_pending_chars(uint64_t heartBeat, double deltaTime);

/* find if character can see */
struct char_data *get_player_vis(struct char_data *ch, char *name, int *number, int inroom);

struct char_data *get_char_vis(struct char_data *ch, char *name, int *number, int where);

struct char_data *get_char_room_vis(struct char_data *ch, char *name, int *number);

struct char_data *get_char_world_vis(struct char_data *ch, char *name, int *number);

struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);

struct obj_data *get_obj_num(obj_rnum nr);

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, int *number, struct obj_data *list);

struct obj_data *get_obj_vis(struct char_data *ch, char *name, int *num);

struct obj_data *get_obj_in_equip_vis(struct char_data *ch, char *arg, int *number, struct obj_data *equipment[]);

extern int get_obj_pos_in_equip_vis(struct char_data *ch, char *arg, int *num, struct obj_data *equipment[]);

extern int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);


/* find all dots */

extern int find_all_dots(char *arg);

#define FIND_INDIV    0
#define FIND_ALL    1
#define FIND_ALLDOT    2


/* Generic Find */

extern int generic_find(char *arg, bitvector_t bitvector, struct char_data *ch,
                        struct char_data **tar_ch, struct obj_data **tar_obj);

#define FIND_CHAR_ROOM     (1 << 0)
#define FIND_CHAR_WORLD    (1 << 1)
#define FIND_OBJ_INV       (1 << 2)
#define FIND_OBJ_ROOM      (1 << 3)
#define FIND_OBJ_WORLD     (1 << 4)
#define FIND_OBJ_EQUIP     (1 << 5)
