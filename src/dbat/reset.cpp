/************************************************************************* 
*  File: reset.c                             Designed for CircleMUD 3.5  * 
*  Usage: implementation of pre-zone resets and post-zone resets         * 
*         functions called before and after a normal zone reset          * 
*                                                                        * 
*  All rights reserved.                                                  * 
*                                                                        * 
*  Copyright (C) 2007 Stefan Cole (a.k.a. Jamdog)                        * 
*  To see this in action, check out AderonMUD                            * 
************************************************************************ */

#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/reset.h"
#include "dbat/spec_procs.h"


/* pre_reset is called before a zone is reset - returns TRUE to prevent a normal reset of the zone */
bool pre_reset(zone_vnum znum) {
    /* By default, a normal zone reset follows this function */
    bool ret_value = PERFORM_NORMAL_RESET;

    switch (znum) {
        /* Gauntlet zone reset type determined by players in the zone */
        case RESET_GAUNTLET:
            ret_value = prereset_gauntlet_zone();
            break;

        default: /* No special zone reset to perform for this zone number */
            ret_value = PERFORM_NORMAL_RESET;
            break;
    }
    return ret_value;
}


/* post_reset is called after a normal zone reset */
void post_reset(zone_vnum znum) {
    switch (znum) {
        default:
            break;
    }
}

bool prereset_gauntlet_zone() {
    int i, gauntlet_players = 0;
    struct char_data *mob;
    /* The mobs in the gauntlet are ALL reset here... */
    struct gauntlet_mob gauntlet_mobs[NUM_GAUNTLET_ROOMS] = {
            /* room vnum   mob vnum */
            {2403, 2400},
            {2405, 2401},
            {2407, 2402},
            {2409, 2403},
            {2411, 2404},
            {2413, 2405},
            {2415, 2406},
            {2417, 2407},
            {2419, 2408},
            {2421, 2409},
            {2423, 2410},
            {2425, 2411},
            {2427, 2412},
            {2429, 2413},
            {2431, 2414},
            {2433, 2415},
            {2435, 2416},
            {2437, 2417},
            {2439, 2418},
            {2441, 2419}
    };

    basic_mud_log("Special Reset: zone %d: Resetting Gauntlet", RESET_GAUNTLET);

    /* Count the number of players currently in the gauntlet */
    for (i = 0; i < NUM_GAUNTLET_ROOMS; i++) {
        gauntlet_players += num_players_in_room(gauntlet_mobs[i].vroom);
    }

    /* No players in the gauntlet - normal reset will do */
    if (gauntlet_players == 0) {
        basic_mud_log("Special Reset: zone %d: No players in Gauntlet - executing normal reset", RESET_GAUNTLET);
        return PERFORM_NORMAL_RESET;
    }

    basic_mud_log("Special Reset: zone %d: %d players in Gauntlet - special reset only", RESET_GAUNTLET, gauntlet_players);

    for (i = 0; i < NUM_GAUNTLET_ROOMS; i++) {
        if (check_mob_in_room(gauntlet_mobs[i].vmob, gauntlet_mobs[i].vroom) == false) {
            /* Mob isn't in the room, can we reset it? */
            if (num_players_in_room(gauntlet_mobs[i].vroom) == 0) {
                /* Yep - no players, reset this room only */
                if ((real_mobile(gauntlet_mobs[i].vmob)) && (real_room(gauntlet_mobs[i].vroom))) {
                    /* Load the correct mob */
                    if ((mob = read_mobile(gauntlet_mobs[i].vmob, VIRTUAL)) != nullptr) {
                        /* And put it in the room */
                        char_to_room(mob, real_room(gauntlet_mobs[i].vroom));
                        basic_mud_log("Special Reset: zone %d: Gauntlet mob reset (%d, %s)", RESET_GAUNTLET,
                            gauntlet_mobs[i].vmob, GET_NAME(mob));

                    }
                }
            }
        }
    }
    return BLOCK_NORMAL_RESET;
}
