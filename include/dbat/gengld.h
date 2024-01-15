/************************************************************************
 * Generic OLC Library - Guild / gengld.h			v1.0	*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/
#pragma once

#include "structs.h"
#include "guild.h"

extern void copy_guild(struct guild_data *tguild, struct guild_data *fguild);

extern void copy_list(vnum **tlist, vnum *flist);

extern void remove_from_int_list(vnum **list, vnum num);

extern void add_to_int_list(vnum **tlist, vnum newi);

extern void free_guild(struct guild_data *guild);

extern void modify_string(char **str, char *newstr);

extern int add_guild(struct guild_data *guild);

extern int save_guilds(zone_rnum zone_num);

extern guild_rnum real_guild(guild_vnum vnum);

/*
 * Handy macros.
 */

#define G_NUM(i)        ((i)->vnum)
#define G_TRAINER(i)    ((i)->gm)
#define G_OPEN(i)        ((i)->open)
#define G_CLOSE(i)        ((i)->close)
#define G_FUNC(i)        ((i)->func)
#define G_CHARGE(i)        ((i)->charge)
#define G_MINLVL(i)        ((i)->minlvl)
#define G_WITH_WHO(i)    ((i)->with_who)
#define G_NO_SKILL(i)     ((i)->no_such_skill)
#define G_NO_GOLD(i)     ((i)->not_enough_gold)

#define G_SK_AND_SP(i, j) ((i)->skills[j])
#define G_FEATS(i, j) ((i)->feats[j])

#define MSG_TRAINER_NOT_OPEN     "I'm busy! Come back later!"
#define MSG_TRAINER_NO_SEE_CH     "I don't train someone I can't see!"
#define MSG_TRAINER_DISLIKE_ALIGN  "Get out of here before I get angry, you are not aligned with me!"
#define MSG_TRAINER_DISLIKE_CLASS "I won't train those of your discipline!"
#define MSG_TRAINER_DISLIKE_RACE     "Get out of here, I don't help your kind!"
#define MSG_TRAINER_MINLVL     "You are not of a skilled enough level to recieve my training."

#define LEARNED_LEVEL   0    /* % known which is considered "learned" */
#define MAX_PER_PRAC    1    /* max percent gain in skill per practice */
#define MIN_PER_PRAC    2    /* min percent gain in skill per practice */
#define PRAC_TYPE       3    /* should it say 'spell' or 'skill'?	 */
