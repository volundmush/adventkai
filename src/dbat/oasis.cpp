/************************************************************************
 * OasisOLC - General / oasis.c					v2.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/interpreter.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/shop.h"
#include "dbat/genmob.h"
#include "dbat/genshp.h"
#include "dbat/genzon.h"
#include "dbat/genobj.h"
#include "dbat/oasis.h"
#include "dbat/dg_olc.h"
#include "dbat/handler.h"
#include "dbat/guild.h"


/******************************************************************************/
/** Internal Data Structures                                                 **/
/******************************************************************************/
struct olc_scmd_info_t {
    const char *text;
    int con_type;
} olc_scmd_info[] = {
        {"room",    CON_REDIT},
        {"object",  CON_OEDIT},
        {"zone",    CON_ZEDIT},
        {"mobile",  CON_MEDIT},
        {"shop",    CON_SEDIT},
        {"config",  CON_CEDIT},
        {"trigger", CON_TRIGEDIT},
        {"action",  CON_AEDIT},
        {"guild",   CON_GEDIT},
        {"help",    CON_HEDIT},
        {"house",   CON_HSEDIT},
        {"\n", -1}
};

/******************************************************************************/
/** Internal Functions                                                       **/
/******************************************************************************/
void free_config(struct config_data *data);

/* -------------------------------------------------------------------------- */

/*
 * Only player characters should be using OLC anyway.
 */
void clear_screen(struct descriptor_data *d) {
    if (PRF_FLAGGED(d->character, PRF_CLS))
        write_to_output(d, "[H[J");
}

/* -------------------------------------------------------------------------- */

/*
 * Exported ACMD do_oasis function.
 *
 * This function is the OLC interface.  It deals with all the 
 * generic OLC stuff, then passes control to the sub-olc sections.
 *
 * UPDATE:
 *  I believe that yes, putting the code together that is common in all of the
 *  olc functions is good to a certain extent, but the do_oasis command was
 *  getting ridiculous.  Therefore, I have separated them into separate
 *  functions that get called from in do_oasis....yes, similar code, but it is
 *  easier to handle....   - Kip Potter
 */
ACMD(do_oasis) {
    /*
     * No screwing around as a mobile.
     */
    if (IS_NPC(ch) || !ch->desc)
        return;

    /*
     * Prevent forcing people in OLC to edit other stuff.
     * 'force' just lets command_interpreter() handle the input,
     * regardless of the state of the victim.
     * This can wreck havoc if people are i OLC already
     * - ie. their input should have been redirected by nanny(), and
     * never get to command_interpreter().
     * -- Welcor 09/03
     * - thanks to Mark Garringer (zizazat@hotmail.com) for the bug report.
     */
    if (STATE(ch->desc) != CON_PLAYING)
        return;


    switch (subcmd) {
        /* The command to see what needs to be saved, typically 'olc'.  */

        case SCMD_OASIS_CEDIT:
            do_oasis_cedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_ZEDIT:
            do_oasis_zedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_REDIT:
            do_oasis_redit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_OEDIT:
            do_oasis_oedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_MEDIT:
            do_oasis_medit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_SEDIT:
            do_oasis_sedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_AEDIT:
            do_oasis_aedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_HEDIT:
            do_oasis_hedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_HSEDIT:
            do_oasis_hsedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_RLIST:
        case SCMD_OASIS_MLIST:
        case SCMD_OASIS_OLIST:
        case SCMD_OASIS_SLIST:
        case SCMD_OASIS_ZLIST:
        case SCMD_OASIS_TLIST:
        case SCMD_OASIS_GLIST:
            do_oasis_list(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_TRIGEDIT:
            do_oasis_trigedit(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_LINKS:
            do_oasis_links(ch, argument, cmd, subcmd);
            break;

        case SCMD_OASIS_GEDIT:
            do_oasis_gedit(ch, argument, cmd, subcmd);
            break;

        default:
            basic_mud_log("SYSERR: (OLC) Invalid subcmd passed to do_oasis, subcmd - (%d)", subcmd);
            return;
    }

    return;
}

/*------------------------------------------------------------*\
 Exported utilities 
\*------------------------------------------------------------*/

/*
 * This procedure frees up the strings and/or the structures
 * attatched to a descriptor, sets all flags back to how they
 * should be.
 */
void cleanup_olc(struct descriptor_data *d, int8_t cleanup_type) {
    /*
     * Clean up WHAT?
     */
    if (d->olc == nullptr)
        return;

    /*
     * Check for a room. free_room doesn't perform
     * sanity checks, we must be careful here.
     */
    if (OLC_ROOM(d)) {
        switch (cleanup_type) {
            case CLEANUP_ALL:
                /* free(OLC_SCRIPT(d)) equivalent */
                free_proto_script(OLC_ROOM(d), WLD_TRIGGER);
                free_room(OLC_ROOM(d));
                break;
            case CLEANUP_STRUCTS:
                free(OLC_ROOM(d));
                break;
            case CLEANUP_CONFIG:
                free_config(OLC_CONFIG(d));
                break;
            default: /* The caller has screwed up. */
                basic_mud_log("SYSERR: cleanup_olc: Unknown type!");
                break;
        }
    }

    /*
     * Check for an existing object in the OLC.  The strings
     * aren't part of the prototype any longer.  They get added
     * with strdup().
     */
    if (OLC_OBJ(d)) {
        free_object_strings(OLC_OBJ(d));
        free(OLC_OBJ(d));
    }

    /*
     * Check for a mob.  free_mobile() makes sure strings are not in
     * the prototype.
     */
    if (OLC_MOB(d))
        free_mobile(OLC_MOB(d));

    /*
     * Check for a zone.  cleanup_type is irrelevant here, free() everything.
     */
    if (OLC_ZONE(d)) {
        if (OLC_ZONE(d)->builders)
            free(OLC_ZONE(d)->builders);
        if (OLC_ZONE(d)->name)
            free(OLC_ZONE(d)->name);
        OLC_ZONE(d)->cmd.clear();
        free(OLC_ZONE(d));
    }

    /*
     * Check for a shop.  free_shop doesn't perform sanity checks, we must
     * be careful here.
     * OLC_SHOP(d) is a _copy_ - no pointers to the original. Just go ahead
     * and free it all.
     */
    if (OLC_SHOP(d)) {
//    switch (cleanup_type) {
//    case CLEANUP_ALL:
        free_shop(OLC_SHOP(d));
//      break;
//    case CLEANUP_STRUCTS:
//      free(OLC_SHOP(d));
//      break;
//    default:
        /* The caller has screwed up but we already griped above. */
//      break;
//    }
    }
    /*. Check for a guild . */
    if (OLC_GUILD(d)) {
        switch (cleanup_type) {
            case CLEANUP_ALL:
            case CLEANUP_STRUCTS:
                delete OLC_GUILD(d);
                break;
            default:
                break;
        }
    }

    if (OLC_HOUSE(d)) { /*. free_house performs no sanity checks, must be careful here .*/
        switch (cleanup_type) {
            case CLEANUP_ALL:
                free_house(OLC_HOUSE(d));
                break;
            case CLEANUP_STRUCTS:
                free(OLC_HOUSE(d));
                break;
            default:
                /*. Caller has screwed up .*/
                break;
        }
    }

    /*. Check for aedit stuff -- M. Scott */
    if (OLC_ACTION(d)) {
        switch (cleanup_type) {
            case CLEANUP_ALL:
                free_action(OLC_ACTION(d));
                break;
            case CLEANUP_STRUCTS:
                free(OLC_ACTION(d));
                break;
            default:
                /* Caller has screwed up */
                break;
        }
    }

    /* free storage if allocated (for tedit and aedit) */
    /* and Triggers */
    /*
     * this is the command list - it's been copied to disk already,
     * so just free it -- Welcor
     */
    if (OLC_STORAGE(d)) {
        free(OLC_STORAGE(d));
        OLC_STORAGE(d) = nullptr;
    }
    /*
     * Free this one regardless. If we've left olc, we've either made
     * a fresh copy of it in the trig index, or we lost connection.
     * Either way, we need to get rid of this.
     */
    if (OLC_TRIG(d)) {
        free_trigger(OLC_TRIG(d));
        OLC_TRIG(d) = nullptr;
    }
    /*
      * OLC_SCRIPT is always set as trig_proto of OLC_OBJ/MOB/ROOM.
      * Therefore it should not be free'd here.
      */

    /*
     * Restore descriptor playing status.
     */
    if (d->character) {
        d->character->playerFlags.reset(PLR_WRITING);
        act("$n stops using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);

        if (cleanup_type == CLEANUP_CONFIG)
            mudlog(BRF, ADMLVL_IMMORT, true, "OLC: %s stops editing the game configuration", GET_NAME(d->character));
        else if (STATE(d) == CON_TEDIT)
            mudlog(BRF, ADMLVL_IMMORT, true, "OLC: %s stops editing text files.", GET_NAME(d->character));
        else if (STATE(d) == CON_HEDIT)
            mudlog(CMP, ADMLVL_IMMORT, true, "OLC: %s stops editing help files.", GET_NAME(d->character));
        else
            mudlog(BRF, ADMLVL_IMMORT, true, "OLC: %s stops editing zone %d allowed zone %d", GET_NAME(d->character),
                   zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(d->character));

        STATE(d) = CON_PLAYING;
    }

    free(d->olc);
    d->olc = nullptr;
}

/*
 * This function is an exact duplicate of the tag_argument function found in
 * one of the ascii patches located on the circlemud ftp website.
 */
void split_argument(char *argument, char *tag) {
    char *tmp = argument, *ttag = tag, *wrt = argument;
    int i;

    for (i = 0; *tmp; tmp++, i++) {
        if (*tmp != ' ' && *tmp != '=')
            *(ttag++) = *tmp;
        else if (*tmp == '=')
            break;
    }

    *ttag = '\0';

    while (*tmp == '=' || *tmp == ' ')
        tmp++;

    while (*tmp)
        *(wrt++) = *(tmp++);

    *wrt = '\0';
}

void free_config(struct config_data *data) {
    /****************************************************************************/
    /** Free strings.                                                          **/
    /****************************************************************************/
    free_strings(data, OASIS_CFG);

    /****************************************************************************/
    /** Free the data structure.                                               **/
    /****************************************************************************/
    free(data);
}

/******************************************************************************/
/**                                                                          **/
/** Function       : can_edit_zone()                                         **/
/**                                                                          **/
/** Description    : Checks to see if a builder can modify the specified     **/
/**                  zone.                                                   **/
/**                                                                          **/
/** Arguments      :                                                         **/
/**   ch                                                                     **/
/**     The character requesting access to modify this zone.                 **/
/**   rnum                                                                   **/
/**     The real number of the zone attempted to be modified.                **/
/**                                                                          **/
/** Returns        : Returns TRUE if the builder has access, otherwise       **/
/**                  FALSE.                                                  **/
/**                                                                          **/
/******************************************************************************/
int can_edit_zone(struct char_data *ch, zone_rnum rnum) {
    /* no access if called with bad arguments */
    if (!ch->desc || IS_NPC(ch) || rnum == NOWHERE)
        return false;

    if (rnum == HEDIT_PERMISSION)
        return true;

    /* always access if ch is high enough level */
    if (GET_ADMLEVEL(ch) >= ADMLVL_GRGOD)
        return (true);

    /* always access if a player helped build the zone in the first place */
    if (is_name(GET_NAME(ch), zone_table[rnum].builders))
        return (true);

    /* no access if you haven't been assigned a zone */
    if (GET_OLC_ZONE(ch) == NOWHERE)
        return false;

    /* no access if you're not at least LVL_BUILDER */
    if (GET_ADMLEVEL(ch) < ADMLVL_BUILDER)
        return false;

    /* always access if you're assigned to this zone */
    if (real_zone(GET_OLC_ZONE(ch)) == rnum)
        return true;

    return (false);
}

void send_cannot_edit(struct char_data *ch, zone_vnum zone) {
    send_to_char(ch, "You do not have permission to edit zone %d.", zone);
    if (GET_OLC_ZONE(ch) != NOWHERE)
        send_to_char(ch, "  Try zone %d.", GET_OLC_ZONE(ch));
    send_to_char(ch, "\r\n");
    mudlog(BRF, ADMLVL_IMPL, true, "OLC: %s tried to edit zone %d allowed zone %d",
           GET_NAME(ch), zone, GET_OLC_ZONE(ch));

}

