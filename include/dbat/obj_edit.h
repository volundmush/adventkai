/* ************************************************************************
*  File: obj_edit.h                  Part of Dragonball Advent Truth      *
*  Usage: Player level object editing utilities and common functions for  *
*  obj_edit.c                                                             *
*                                                                         *
*  This is an original file created by me for Dragonball Advent Truth     *
*  to house all player level object editing defines/declarations          *
*                                                            Iovan 1/6/13 *
************************************************************************ */
#pragma once

#include "structs.h"


/* obj_edit.c functions */
extern void pobj_edit_parse(struct descriptor_data *d, char *arg);

extern void disp_custom_menu(struct descriptor_data *d);

extern void disp_restring_menu(struct descriptor_data *d);

/*-------------  obj_edit.c defines -------------*/

/* What object edit menu are we in? */
#define EDIT_NONE         0
#define EDIT_CUSTOM       1
#define EDIT_RESTRING     2

/* Menu Values */
/* Custom Equipment */
#define EDIT_CUSTOM_MAIN   1
#define EDIT_CUSTOM_NAME   2
#define EDIT_CUSTOM_SDESC  3
#define EDIT_CUSTOM_LDESC  4
#define EDIT_CUSTOM_TYPE   5
#define EDIT_CUSTOM_WEAPON 6
#define EDIT_CUSTOM_QUIT   7

/* Restring Equipment */
#define EDIT_RESTRING_MAIN  1
#define EDIT_RESTRING_NAME  2
#define EDIT_RESTRING_SDESC 3
#define EDIT_RESTRING_LDESC 4
#define EDIT_RESTRING_QUIT  5
#define EDIT_RESTRING_CONF  6
