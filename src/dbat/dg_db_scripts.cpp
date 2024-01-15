/* ************************************************************************
*  File: dg_db_scripts.c                         Part of Death's Gate MUD *
*                                                                         *
*  Usage: Contains routines to handle db functions for scripts and trigs  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Death's Gate MUD is based on CircleMUD, Copyright (C) 1993, 94.        *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  $Author: Mark A. Heilpern/egreen/Welcor $                              *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
************************************************************************ */

#include "dbat/structs.h"
#include "dbat/dg_scripts.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/dg_event.h"
#include "dbat/comm.h"
#include "dbat/constants.h"

extern bitvector_t asciiflag_conv(char *flag);

/* local functions */
void trig_data_init(trig_data *this_data);

void parse_trigger(FILE *trig_f, trig_vnum nr) {
    int t[2], k, attach_type;
    char line[256], *cmds, *s, flags[256], errors[MAX_INPUT_LENGTH];
    struct cmdlist_element *cle;
    auto &idx = trig_index[nr];
    idx.vn = nr;

    auto trig = new trig_data();

    idx.proto = trig;
    trig->vn = nr;
    auto &z = zone_table[real_zone_by_thing(nr)];
    z.triggers.insert(nr);

    snprintf(errors, sizeof(errors), "trig vnum %d", nr);

    trig->name = fread_string(trig_f, errors);

    get_line(trig_f, line);
    k = sscanf(line, "%d %s %d", &attach_type, flags, t);
    trig->attach_type = (int8_t) attach_type;
    trig->trigger_type = (long) asciiflag_conv(flags);
    trig->narg = (k == 3) ? t[0] : 0;

    trig->arglist = fread_string(trig_f, errors);

    cmds = s = fread_string(trig_f, errors);

    CREATE(trig->cmdlist, struct cmdlist_element, 1);
    trig->cmdlist->cmd = strdup(strtok(s, "\r\n"));
    cle = trig->cmdlist;

    while ((s = strtok(nullptr, "\r\n"))) {
        CREATE(cle->next, struct cmdlist_element, 1);
        cle = cle->next;
        cle->cmd = strdup(s);
    }

    free(cmds);
}


/*
 * create a new trigger from a prototype.
 * nr is the real number of the trigger.
 */
trig_data *read_trigger(int nr) {

    auto idx = trig_index.find(nr);
    if(idx == trig_index.end()) return nullptr;

    auto trig = new trig_data();

    trig_data_copy(trig, idx->second.proto);
    insert_vnum(scriptVnumIndex, trig);

    return trig;
}


void trig_data_init(trig_data *this_data) {
    this_data->vn = NOTHING;
    this_data->data_type = 0;
    this_data->name = nullptr;
    this_data->trigger_type = 0;
    this_data->cmdlist = nullptr;
    this_data->curr_state = nullptr;
    this_data->narg = 0;
    this_data->arglist = nullptr;
    this_data->depth = 0;
    this_data->waiting = 0.0;
    this_data->purged = false;
    this_data->var_list = nullptr;

    this_data->next = nullptr;
}


void trig_data_copy(trig_data *this_data, const trig_data *trg) {
    trig_data_init(this_data);

    this_data->vn = trg->vn;
    this_data->attach_type = trg->attach_type;
    this_data->data_type = trg->data_type;
    if (trg->name)
        this_data->name = strdup(trg->name);
    else {
        this_data->name = strdup("unnamed trigger");
        basic_mud_log("Trigger with no name! (%d)", trg->vn);
    }
    this_data->trigger_type = trg->trigger_type;
    this_data->cmdlist = trg->cmdlist;
    this_data->narg = trg->narg;
    if (trg->arglist) this_data->arglist = strdup(trg->arglist);
}

/* for mobs and rooms: */
void dg_read_trigger(FILE *fp, struct unit_data *proto, int type) {
    char line[READ_SIZE];
    char junk[8];
    int vnum, rnum, count;
    char_data *mob;
    room_data *room;
    struct trig_proto_list *trg_proto, *new_trg;

    get_line(fp, line);
    count = sscanf(line, "%7s %d", junk, &vnum);

    if (count != 2) {
        mudlog(BRF, ADMLVL_BUILDER, true,
               "SYSERR: Error assigning trigger! - Line was\n  %s", line);
        return;
    }

    rnum = real_trigger(vnum);
    if (rnum == NOTHING) {
        switch (type) {
            case MOB_TRIGGER:
                mudlog(BRF, ADMLVL_BUILDER, true,
                       "SYSERR: dg_read_trigger: Trigger vnum #%d asked for but non-existant! (mob: %s - %d)",
                       vnum, GET_NAME((char_data *) proto), GET_MOB_VNUM((char_data *) proto));
                break;
            case WLD_TRIGGER:
                mudlog(BRF, ADMLVL_BUILDER, true,
                       "SYSERR: dg_read_trigger: Trigger vnum #%d asked for but non-existant! (room:%d)",
                       vnum, GET_ROOM_VNUM(((room_data *) proto)->vn));
                break;
            default:
                mudlog(BRF, ADMLVL_BUILDER, true,
                       "SYSERR: dg_read_trigger: Trigger vnum #%d asked for but non-existant! (?)", vnum);
                break;
        }
        return;
    }

    proto->proto_script.push_back(rnum);
}

void dg_obj_trigger(char *line, struct obj_data *obj) {
    char junk[8];
    int vnum, rnum, count;
    struct trig_proto_list *trg_proto, *new_trg;

    count = sscanf(line, "%s %d", junk, &vnum);

    if (count != 2) {
        mudlog(BRF, ADMLVL_BUILDER, true,
               "SYSERR: dg_obj_trigger() : Error assigning trigger! - Line was:\n  %s", line);
        return;
    }

    rnum = real_trigger(vnum);
    if (rnum == NOTHING) {
        mudlog(BRF, ADMLVL_BUILDER, true,
               "SYSERR: Trigger vnum #%d asked for but non-existant! (Object: %s - %d)",
               vnum, obj->short_description, GET_OBJ_VNUM(obj));
        return;
    }

    obj->proto_script.push_back(rnum);
}

void assign_triggers(struct unit_data *i, int type) {

    if(!SCRIPT(i)) {
        switch (type) {
            case MOB_TRIGGER:
                i->script = new script_data((char_data*)i);
                break;
            case OBJ_TRIGGER:
                i->script = new script_data((obj_data*)i);
                break;
            case WLD_TRIGGER:
                i->script = new script_data((room_data*)i);
                break;
            default:
                mudlog(BRF, ADMLVL_BUILDER, true,
                       "SYSERR: unknown type for assign_triggers()");
                return;
        }
    }

    // remove all duplicates from i->proto_script but do not change its order otherwise.
    std::set<trig_vnum> alreadySeen;
    auto it = i->proto_script.begin();
    while(it != i->proto_script.end()) {
        if(alreadySeen.contains(*it)) {
            it = i->proto_script.erase(it);
        } else {
            alreadySeen.insert(*it);
            ++it;
        }
    }

    std::set<trig_vnum> existVnums;
    for(auto t = SCRIPT(i)->trig_list; t; t = t->next) existVnums.insert(t->vn);

    for(auto p : i->proto_script) {
        // only add if they don't already have one...
        if(!existVnums.contains(p)) {
            add_trigger(SCRIPT(i), read_trigger(p), -1);
            existVnums.insert(p);
        }
    }
}
