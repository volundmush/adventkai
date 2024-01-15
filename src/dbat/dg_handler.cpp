/**************************************************************************
*  File: dg_handler.c                                                     *
*                                                                         *
*  Usage: contains functions to handle memory for scripts.                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Death's Gate MUD is based on CircleMUD, Copyright (C) 1993, 94.        *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  $Author: Mark A. Heilpern/egreen/Welcor $                              *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
***************************************************************************/

#include "dbat/structs.h"
#include "dbat/dg_scripts.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/dg_event.h"


/* frees memory associated with var */
void free_var_el(struct trig_var_data *var) {
    if (var->name)
        free(var->name);
    if (var->value)
        free(var->value);
    free(var);
}

/* release memory allocated for a variable list */
void free_varlist(struct trig_var_data *vd) {
    struct trig_var_data *i, *j;

    for (i = vd; i;) {
        j = i;
        i = i->next;
        free_var_el(j);
    }
}

/*
 * remove var name from var_list
 * returns 1 if found, else 0
 */
int remove_var(struct trig_var_data **var_list, char *name) {
    struct trig_var_data *i, *j;

    for (j = nullptr, i = *var_list; i && strcasecmp(name, i->name);
         j = i, i = i->next);

    if (i) {
        if (j) {
            j->next = i->next;
            free_var_el(i);
        } else {
            *var_list = i->next;
            free_var_el(i);
        }

        return 1;
    }

    return 0;
}

/*
 * Return memory used by a trigger
 * The command list is free'd when changed and when
 * shutting down.
 */
void free_trigger(struct trig_data *trig) {
    free(trig->name);
    trig->name = nullptr;

    if (trig->arglist) {
        free(trig->arglist);
        trig->arglist = nullptr;
    }
    if (trig->var_list) {
        free_varlist(trig->var_list);
        trig->var_list = nullptr;
    }
    triggers_waiting.erase(trig);

    delete trig;
}


/* remove a single trigger from a mob/obj/room */
void extract_trigger(struct trig_data *trig) {
    struct trig_data *temp;

    triggers_waiting.erase(trig);
    erase_vnum(scriptVnumIndex, trig);

    /* walk the trigger list and remove this one */
    REMOVE_FROM_LIST(trig, trigger_list, next_in_world, temp);

    auto found = uniqueScripts.find(trig->id);
    if (found != uniqueScripts.end()) {
        uniqueScripts.erase(found);
    }

    free_trigger(trig);
}

/* remove all triggers from a mob/obj/room */
void extract_script(void *thing, int type) {
    struct script_data *sc = nullptr;
    struct trig_data *trig, *next_trig;
    char_data *mob;
    obj_data *obj;
    room_data *room;

    switch (type) {
        case MOB_TRIGGER:
            mob = (struct char_data *) thing;
            sc = SCRIPT(mob);
            SCRIPT(mob) = nullptr;
            break;
        case OBJ_TRIGGER:
            obj = (struct obj_data *) thing;
            sc = SCRIPT(obj);
            SCRIPT(obj) = nullptr;
            break;
        case WLD_TRIGGER:
            room = (struct room_data *) thing;
            sc = SCRIPT(room);
            SCRIPT(room) = nullptr;
            break;
    }

#if 1 /* debugging */
    {
        struct char_data *i = character_list;
        struct obj_data *j = object_list;
        room_rnum k;
        if (sc) {
            for (; i; i = i->next)
                assert(sc != SCRIPT(i));

            for (; j; j = j->next)
                assert(sc != SCRIPT(j));

            for (auto &r : world)
                assert(sc != SCRIPT(&r.second));
        }
    }
#endif
    for (trig = TRIGGERS(sc); trig; trig = next_trig) {
        next_trig = trig->next;
        extract_trigger(trig);
    }
    TRIGGERS(sc) = nullptr;

    /* Thanks to James Long for tracking down this memory leak */
    free_varlist(sc->global_vars);

    delete sc;
}

/* erase the script memory of a mob */
void extract_script_mem(struct script_memory *sc) {
    struct script_memory *next;
    while (sc) {
        next = sc->next;
        if (sc->cmd) free(sc->cmd);
        free(sc);
        sc = next;
    }
}

void free_proto_script(struct unit_data *thing, int type) {
    thing->proto_script.clear();
}

void copy_proto_script(struct unit_data *source, struct unit_data *dest, int type) {
    dest->proto_script = source->proto_script;
}

