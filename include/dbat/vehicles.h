//
// Created by volund on 10/20/21.
//
#pragma once

#include "structs.h"


// functions
extern void drive_in_direction(struct char_data *ch, struct obj_data *vehicle, int dir);

struct obj_data *find_control(struct char_data *ch);

struct obj_data *find_vehicle_by_vnum(int vnum);

struct obj_data *find_hatch_by_vnum(int vnum);

struct obj_data *get_obj_in_list_type(int type, struct obj_data *list);


// commands
extern ACMD(do_warp);

extern ACMD(do_drive);

extern ACMD(do_ship_fire);
