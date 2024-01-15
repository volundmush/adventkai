/************************************************************************
 * Generic OLC Library - Guilds / gengld.c			v1.0	*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/
#include "dbat/gengld.h"
#include "dbat/shop.h"
#include "dbat/genolc.h"
#include "dbat/genzon.h"
#include "dbat/utils.h"

/*
 * NOTE (gg): Didn't modify sedit much. Don't consider it as 'recent'
 * 	as the other editors with regard to updates or style.
 */

/*-------------------------------------------------------------------*/

void copy_guild(struct guild_data *tgm, struct guild_data *fgm) {
    *tgm = *fgm;
}

/*-------------------------------------------------------------------*/

/* returns the real number of the guild with given virtual number 
 *
 * We take so good care to keep it sorted - let's use it :) - Welcor
 */
guild_rnum real_guild(guild_vnum vnum) {
    return guild_index.count(vnum) ? vnum : NOTHING;
}

/*-------------------------------------------------------------------*/

/*. Generic string modifyer for guild master messages . */

void gedit_modify_string(std::string &str, char *new_g) {
    char *pointer;
    char buf[MAX_STRING_LENGTH];

    /*. Check the '%s' is present, if not, add it . */
    if (*new_g != '%') {
        snprintf(buf, sizeof(buf), "%%s %s", new_g);
        pointer = buf;
    } else
        pointer = new_g;

    str = pointer;
}

/*-------------------------------------------------------------------*/

int add_guild(struct guild_data *ngld) {
    guild_rnum rguild = G_NUM(ngld);
    zone_rnum rznum = real_zone_by_thing(rguild);
    auto exists = guild_index.contains(rguild);
    auto &g = guild_index[rguild];

    auto &z = zone_table[rznum];
    z.guilds.insert(rguild);

    dirty_guilds.insert(rguild);
    if(!exists) mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: GenOLC: Creating new guild.");

    copy_guild(&g, ngld);

    return rguild;
}

/*-------------------------------------------------------------------*/

int save_guilds(zone_rnum zone_num) {
    auto z = zone_table.find(zone_num);

    if (z == zone_table.end())
    {
        basic_mud_log("SYSERR: GenOLC: save_guilds: Invalid real zone number %d.", zone_num);
        return false;
    }

    dirty_guilds.insert(z->second.guilds.begin(), z->second.guilds.end());
    z->second.save_guilds();
}
