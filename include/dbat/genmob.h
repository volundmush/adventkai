/************************************************************************
 * Generic OLC Library - Mobiles / genmob.h			v1.0	*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/
#pragma once

#include "structs.h"


extern int delete_mobile(mob_rnum);

extern int copy_mobile(struct char_data *to, struct char_data *from);

extern int add_mobile(struct char_data *, mob_vnum);

extern int copy_mob_strings(struct char_data *to, struct char_data *from);

extern int free_mobile_strings(struct char_data *mob);

extern int free_mobile(struct char_data *mob);

extern int save_mobiles(zone_rnum rznum);

/* Handy macros. */
#define GET_NDD(mob)    ((mob)->mob_specials.damnodice)
#define GET_SDD(mob)    ((mob)->mob_specials.damsizedice)
#define GET_ALIAS(mob)    ((mob)->name)
#define GET_SDESC(mob)    ((mob)->short_description)
#define GET_LDESC(mob)    ((mob)->room_description)
#define GET_DDESC(mob)    ((mob)->look_description)
#define GET_ATTACK(mob)    ((mob)->mob_specials.attack_type)
