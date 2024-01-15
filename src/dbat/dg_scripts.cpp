/**************************************************************************
*  File: dg_scripts.c                                                     *
*  Usage: contains the main script driver interface.                      *
*                                                                         *
*                                                                         *
*  $Author: Mark A. Heilpern/egreen/Welcor $                              *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
**************************************************************************/

#include <boost/algorithm/string/predicate.hpp>
#include "dbat/dg_scripts.h"
#include "dbat/act.wizard.h"
#include "dbat/dg_event.h"
#include "dbat/utils.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"
#include "dbat/constants.h"
#include "dbat/comm.h"
#include "dbat/players.h"
#include <boost/regex.hpp>

#define PULSES_PER_MUD_HOUR     (SECS_PER_MUD_HOUR*PASSES_PER_SEC)


/* Local functions not used elsewhere */
void do_stat_trigger(struct char_data *ch, trig_data *trig);

void script_stat(char_data *ch, struct script_data *sc);

int remove_trigger(struct script_data *sc, char *name);

bool is_num(const std::string &arg);

void eval_op(char *op, char *lhs, char *rhs, char *result, void *go,
             struct script_data *sc, trig_data *trig);

char *matching_paren(char *p);

void eval_expr(char *line, char *result, void *go, struct script_data *sc,
               trig_data *trig, int type);

int eval_lhs_op_rhs(char *expr, char *result, void *go, struct script_data *sc,
                    trig_data *trig, int type);

int process_if(char *cond, void *go, struct script_data *sc,
               trig_data *trig, int type);

struct cmdlist_element *find_end(trig_data *trig, struct cmdlist_element *cl);

struct cmdlist_element *find_else_end(trig_data *trig,
                                      struct cmdlist_element *cl, void *go,
                                      struct script_data *sc, int type);

void process_wait(void *go, trig_data *trig, int type, char *cmd,
                  struct cmdlist_element *cl);

void process_set(struct script_data *sc, trig_data *trig, char *cmd);

void process_attach(void *go, struct script_data *sc, trig_data *trig,
                    int type, char *cmd);

void process_detach(void *go, struct script_data *sc, trig_data *trig,
                    int type, char *cmd);

int process_return(trig_data *trig, char *cmd);

void process_unset(struct script_data *sc, trig_data *trig, char *cmd);

void process_remote(struct script_data *sc, trig_data *trig, char *cmd);

void process_rdelete(struct script_data *sc, trig_data *trig, char *cmd);

void process_global(struct script_data *sc, trig_data *trig, char *cmd, long id);

void process_context(struct script_data *sc, trig_data *trig, char *cmd);

void extract_value(struct script_data *sc, trig_data *trig, char *cmd);

void dg_letter_value(struct script_data *sc, trig_data *trig, char *cmd);

struct cmdlist_element *
find_case(struct trig_data *trig, struct cmdlist_element *cl,
          void *go, struct script_data *sc, int type, char *cond);

struct cmdlist_element *find_done(struct cmdlist_element *cl);

int fgetline(FILE *file, char *p);

ACMD(do_attach);

ACMD(do_detach);

ACMD(do_vdelete);

ACMD(do_tstat);

/* Return pointer to first occurrence of string ct in */
/* cs, or nullptr if not present.  Case insensitive */
char *str_str(char *cs, char *ct) {
    char *s, *t;

    if (!cs || !ct || !*ct)
        return nullptr;

    while (*cs) {
        t = ct;

        while (*cs && (LOWER(*cs) != LOWER(*t)))
            cs++;

        s = cs;

        while (*t && *cs && (LOWER(*cs) == LOWER(*t))) {
            t++;
            cs++;
        }

        if (!*t)
            return s;
    }

    return nullptr;
}


int trgvar_in_room(room_vnum vnum) {
    room_rnum rnum = real_room(vnum);
    int i = 0;
    char_data *ch;

    if (rnum == NOWHERE) {
        script_log("people.vnum: world[rnum] does not exist");
        return (-1);
    }

    for (ch = world[rnum].people; ch != nullptr; ch = ch->next_in_room)
        i++;

    return i;
}

obj_data *get_obj_in_list(char *name, obj_data *list) {
    obj_data *i;
    int32_t id;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 1) return nullptr;
        auto obj = std::get<1>(*uidResult);

        for (i = list; i; i = i->next_content)
            if(i == obj) return obj;
    } else {
        for (i = list; i; i = i->next_content)
            if (isname(name, i->name))
                return i;
    }

    return nullptr;
}

obj_data *get_object_in_equip(char_data *ch, char *name) {
    int j, n = 0, number;
    obj_data *obj;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;
    int32_t id;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 1) return nullptr;
        auto o = std::get<1>(*uidResult);

        for (j = 0; j < NUM_WEARS; j++)
            if ((obj = GET_EQ(ch, j)))
                if (o == obj) return obj;
    } else if (is_number(name)) {
        obj_vnum ovnum = atoi(name);
        for (j = 0; j < NUM_WEARS; j++)
            if ((obj = GET_EQ(ch, j)))
                if (GET_OBJ_VNUM(obj) == ovnum)
                    return (obj);
    } else {
        snprintf(tmpname, sizeof(tmpname), "%s", name);
        if (!(number = get_number(&tmp)))
            return nullptr;

        for (j = 0; (j < NUM_WEARS) && (n <= number); j++)
            if ((obj = GET_EQ(ch, j)))
                if (isname(tmp, obj->name))
                    if (++n == number)
                        return (obj);
    }

    return nullptr;
}

static struct eq_pos_list {
    const char *pos;
    int where;
} eq_pos[] = {
        {"hold",      WEAR_WIELD2},
        {"held",      WEAR_WIELD2},
        {"light",     WEAR_WIELD2},
        {"wield",     WEAR_WIELD1},
        {"rfinger",   WEAR_FINGER_R},
        {"lfinger",   WEAR_FINGER_L},
        {"neck1",     WEAR_NECK_1},
        {"neck2",     WEAR_NECK_2},
        {"body",      WEAR_BODY},
        {"head",      WEAR_HEAD},
        {"legs",      WEAR_LEGS},
        {"feet",      WEAR_FEET},
        {"hands",     WEAR_HANDS},
        {"arms",      WEAR_ARMS},
        {"shield",    WEAR_WIELD2},
        {"about",     WEAR_ABOUT},
        {"waist",     WEAR_WAIST},
        {"rwrist",    WEAR_WRIST_R},
        {"lwrist",    WEAR_WRIST_L},
        {"backpack",  WEAR_BACKPACK},
        {"rear",      WEAR_EAR_R},
        {"lear",      WEAR_EAR_L},
        {"shoulders", WEAR_SH},
        {"scouter",   WEAR_EYE},
        {"none", -1}
};

/* Handles 'held', 'light' and 'wield' positions - Welcor
   After idea from Byron Ellacott - bje@apnic.net */
int find_eq_pos_script(char *arg) {
    int i;

    if (is_number(arg) && (i = atoi(arg)) >= 0 && i < NUM_WEARS)
        return i;

    for (i = 0; eq_pos[i].where != -1; i++) {
        if (!strcasecmp(eq_pos[i].pos, arg))
            return eq_pos[i].where;
    }
    return (-1);
}

int can_wear_on_pos(struct obj_data *obj, int pos) {
    switch (pos) {
        case WEAR_WIELD1:
            return CAN_WEAR(obj, ITEM_WEAR_WIELD);
        case WEAR_WIELD2:
            return CAN_WEAR(obj, ITEM_WEAR_TAKE);
        case WEAR_FINGER_R:
        case WEAR_FINGER_L:
            return CAN_WEAR(obj, ITEM_WEAR_FINGER);
        case WEAR_NECK_1:
        case WEAR_NECK_2:
            return CAN_WEAR(obj, ITEM_WEAR_NECK);
        case WEAR_BODY:
            return CAN_WEAR(obj, ITEM_WEAR_BODY);
        case WEAR_HEAD:
            return CAN_WEAR(obj, ITEM_WEAR_HEAD);
        case WEAR_LEGS:
            return CAN_WEAR(obj, ITEM_WEAR_LEGS);
        case WEAR_FEET:
            return CAN_WEAR(obj, ITEM_WEAR_FEET);
        case WEAR_HANDS:
            return CAN_WEAR(obj, ITEM_WEAR_HANDS);
        case WEAR_ARMS:
            return CAN_WEAR(obj, ITEM_WEAR_ARMS);
        case WEAR_ABOUT:
            return CAN_WEAR(obj, ITEM_WEAR_ABOUT);
        case WEAR_WAIST:
            return CAN_WEAR(obj, ITEM_WEAR_WAIST);
        case WEAR_WRIST_R:
        case WEAR_WRIST_L:
            return CAN_WEAR(obj, ITEM_WEAR_WRIST);
        case WEAR_BACKPACK:
            return CAN_WEAR(obj, ITEM_WEAR_PACK);
        case WEAR_EAR_R:
        case WEAR_EAR_L:
            return CAN_WEAR(obj, ITEM_WEAR_EAR);
        case WEAR_SH:
            return CAN_WEAR(obj, ITEM_WEAR_SH);
        case WEAR_EYE:
            return CAN_WEAR(obj, ITEM_WEAR_EYE);
        default:
            return false;
    }
}

/************************************************************
 * generic searches based only on name
 ************************************************************/

/* search the entire world for a char, and return a pointer */
char_data *get_char(char *name) {
    char_data *i;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 2) return nullptr;
        i = std::get<2>(*uidResult);

        if (i && valid_dg_target(i, DG_ALLOW_GODS))
            return i;
    } else {
        for (i = character_list; i; i = i->next)
            if (isname(name, i->name) &&
                valid_dg_target(i, DG_ALLOW_GODS))
                return i;
    }

    return nullptr;
}

/*
 * Finds a char in the same room as the object with the name 'name'
 */
char_data *get_char_near_obj(obj_data *obj, char *name) {
    char_data *ch;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 2) return nullptr;
        ch = std::get<2>(*uidResult);

        if (ch && valid_dg_target(ch, DG_ALLOW_GODS))
            return ch;
    } else {
        room_rnum num;
        if ((num = obj_room(obj)) != NOWHERE)
            for (ch = world[num].people; ch; ch = ch->next_in_room)
                if (isname(name, ch->name) &&
                    valid_dg_target(ch, DG_ALLOW_GODS))
                    return ch;
    }

    return nullptr;
}


/*
 * returns a pointer to the first character in world by name name,
 * or nullptr if none found.  Starts searching in room room first
 */
char_data *get_char_in_room(room_data *room, char *name) {
    char_data *ch;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 2) return nullptr;
        ch = std::get<2>(*uidResult);

        if (ch && valid_dg_target(ch, DG_ALLOW_GODS))
            return ch;
    } else {
        for (ch = room->people; ch; ch = ch->next_in_room)
            if (isname(name, ch->name) &&
                valid_dg_target(ch, DG_ALLOW_GODS))
                return ch;
    }

    return nullptr;
}

/* searches the room with the object for an object with name 'name'*/

obj_data *get_obj_near_obj(obj_data *obj, char *name) {
    obj_data *i = nullptr;
    char_data *ch;
    room_vnum rm;
    int32_t id;

    if (!strcasecmp(name, "self") || !strcasecmp(name, "me"))
        return obj;

    /* is it inside ? */
    if (obj->contents && (i = get_obj_in_list(name, obj->contents)))
        return i;

    /* or outside ? */
    if (obj->in_obj) {
        if (*name == UID_CHAR) {
            std::optional<DgUID> result;
            result = resolveUID(name);
            auto uidResult = result;
            if(!uidResult) return nullptr;
            if(uidResult->index() != 1) return nullptr;
            auto o = std::get<1>(*uidResult);
            if(o == obj->in_obj) return o;
        } else if (isname(name, obj->in_obj->name))
            return obj->in_obj;
    }
        /* or worn ?*/
    else if (obj->worn_by && (i = get_object_in_equip(obj->worn_by, name)))
        return i;
        /* or carried ? */
    else if (obj->carried_by &&
             (i = get_obj_in_list(name, obj->carried_by->contents)))
        return i;
    else if ((rm = obj_room(obj)) != NOWHERE) {
        /* check the floor */
        if ((i = get_obj_in_list(name, world[rm].contents)))
            return i;

        /* check peoples' inventory */
        for (ch = world[rm].people; ch; ch = ch->next_in_room)
            if ((i = get_object_in_equip(ch, name)))
                return i;
    }
    return nullptr;
}

/* returns the object in the world with name name, or nullptr if not found */
obj_data *get_obj(char *name) {
    obj_data *obj;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 1) return nullptr;
        return std::get<1>(*uidResult);
    }
    else {
        for (obj = object_list; obj; obj = obj->next)
            if (isname(name, obj->name))
                return obj;
    }

    return nullptr;
}


/* finds room by id or vnum.  returns nullptr if not found */
room_data *get_room(char *name) {
    room_rnum nr;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 0) return nullptr;
        auto r = std::get<0>(*uidResult);
        return r;
    }
    else if ((nr = real_room(atoi(name))) == NOWHERE)
        return nullptr;
    else
        return &world[nr];
}


/*
 * returns a pointer to the first character in world by name name,
 * or nullptr if none found.  Starts searching with the person owing the object
 */
char_data *get_char_by_obj(obj_data *obj, char *name) {
    char_data *ch;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 2) return nullptr;
        ch = std::get<2>(*uidResult);

        if (ch && valid_dg_target(ch, DG_ALLOW_GODS))
            return ch;
    } else {
        if (obj->carried_by &&
            isname(name, obj->carried_by->name) &&
            valid_dg_target(obj->carried_by, DG_ALLOW_GODS))
            return obj->carried_by;

        if (obj->worn_by &&
            isname(name, obj->worn_by->name) &&
            valid_dg_target(obj->worn_by, DG_ALLOW_GODS))
            return obj->worn_by;

        for (ch = character_list; ch; ch = ch->next)
            if (isname(name, ch->name) &&
                valid_dg_target(ch, DG_ALLOW_GODS))
                return ch;
    }

    return nullptr;
}


/*
 * returns a pointer to the first character in world by name name,
 * or nullptr if none found.  Starts searching in room room first
 */
char_data *get_char_by_room(room_data *room, char *name) {
    char_data *ch;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 2) return nullptr;
        ch = std::get<2>(*uidResult);

        if (ch && valid_dg_target(ch, DG_ALLOW_GODS))
            return ch;
    } else {
        for (ch = room->people; ch; ch = ch->next_in_room)
            if (isname(name, ch->name) &&
                valid_dg_target(ch, DG_ALLOW_GODS))
                return ch;

        for (ch = character_list; ch; ch = ch->next)
            if (isname(name, ch->name) &&
                valid_dg_target(ch, DG_ALLOW_GODS))
                return ch;
    }

    return nullptr;
}


/*
 * returns the object in the world with name name, or nullptr if not found
 * search based on obj
 */
obj_data *get_obj_by_obj(obj_data *obj, char *name) {
    obj_data *i = nullptr;
    room_vnum rm;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 1) return nullptr;
        return std::get<1>(*uidResult);
    }

    if (!strcasecmp(name, "self") || !strcasecmp(name, "me"))
        return obj;

    if (obj->contents && (i = get_obj_in_list(name, obj->contents)))
        return i;

    if (obj->in_obj && isname(name, obj->in_obj->name))
        return obj->in_obj;

    if (obj->worn_by && (i = get_object_in_equip(obj->worn_by, name)))
        return i;

    if (obj->carried_by &&
        (i = get_obj_in_list(name, obj->carried_by->contents)))
        return i;

    if (((rm = obj_room(obj)) != NOWHERE) &&
        (i = get_obj_in_list(name, world[rm].contents)))
        return i;

    return get_obj(name);
}

/* only searches the room */
obj_data *get_obj_in_room(room_data *room, char *name) {
    obj_data *obj;
    int32_t id;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 1) return nullptr;
        auto o = std::get<1>(*uidResult);
        for (obj = room->contents; obj; obj = obj->next_content)
            if (o == obj)
                return obj;
    } else {
        for (obj = room->contents; obj; obj = obj->next_content)
            if (isname(name, obj->name))
                return obj;
    }

    return nullptr;
}

/* returns obj with name - searches room, then world */
obj_data *get_obj_by_room(room_data *room, char *name) {
    obj_data *obj;

    if (*name == UID_CHAR) {
        std::optional<DgUID> result;
        result = resolveUID(name);
        auto uidResult = result;
        if(!uidResult) return nullptr;
        if(uidResult->index() != 1) return nullptr;
        return std::get<1>(*uidResult);
    }

    for (obj = room->contents; obj; obj = obj->next_content)
        if (isname(name, obj->name))
            return obj;

    for (obj = object_list; obj; obj = obj->next)
        if (isname(name, obj->name))
            return obj;

    return nullptr;
}

/* checks every PULSE_SCRIPT for random triggers */
void script_trigger_check(uint64_t heartPulse, double deltaTime) {
    char_data *ch;
    obj_data *obj;
    struct room_data *room = nullptr;
    int nr;
    struct script_data *sc;

    for (ch = character_list; ch; ch = ch->next) {
        if (SCRIPT(ch)) {
            sc = SCRIPT(ch);

            if (IS_SET(SCRIPT_TYPES(sc), WTRIG_RANDOM) &&
                (!is_empty(ch->getRoom()->zone) ||
                 IS_SET(SCRIPT_TYPES(sc), WTRIG_GLOBAL)))
                random_mtrigger(ch);
        }
    }

    for (obj = object_list; obj; obj = obj->next) {
        if (SCRIPT(obj)) {
            sc = SCRIPT(obj);

            if (IS_SET(SCRIPT_TYPES(sc), OTRIG_RANDOM))
                random_otrigger(obj);
        }
    }

    for (auto &r : world) {
        if (SCRIPT(&r.second)) {
            room = &r.second;
            sc = SCRIPT(room);

            if (IS_SET(SCRIPT_TYPES(sc), WTRIG_RANDOM) &&
                (!is_empty(room->zone) ||
                 IS_SET(SCRIPT_TYPES(sc), WTRIG_GLOBAL)))
                random_wtrigger(room);
        }
    }
}

void check_time_triggers() {
    char_data *ch;
    obj_data *obj;
    struct room_data *room = nullptr;
    int nr;
    struct script_data *sc;

    for (ch = character_list; ch; ch = ch->next) {
        if (SCRIPT(ch)) {
            sc = SCRIPT(ch);

            if (IS_SET(SCRIPT_TYPES(sc), MTRIG_TIME) &&
                (!is_empty(ch->getRoom()->zone) ||
                 IS_SET(SCRIPT_TYPES(sc), MTRIG_GLOBAL)))
                time_mtrigger(ch);
        }
    }

    for (obj = object_list; obj; obj = obj->next) {
        if (SCRIPT(obj)) {
            sc = SCRIPT(obj);

            if (IS_SET(SCRIPT_TYPES(sc), OTRIG_TIME))
                time_otrigger(obj);
        }
    }

    for (auto &r : world) {
        if (SCRIPT(&r.second)) {
            room = &r.second;
            sc = SCRIPT(room);

            if (IS_SET(SCRIPT_TYPES(sc), WTRIG_TIME) &&
                (!is_empty(room->zone) ||
                 IS_SET(SCRIPT_TYPES(sc), WTRIG_GLOBAL)))
                time_wtrigger(room);
        }
    }
}

void check_interval_triggers(int trigFlag) {

    for (auto ch = character_list; ch; ch = ch->next) {
        if (SCRIPT(ch)) {
            auto sc = SCRIPT(ch);

            if (IS_SET(SCRIPT_TYPES(sc), trigFlag) &&
                (!is_empty(ch->getRoom()->zone) ||
                 IS_SET(SCRIPT_TYPES(sc), MTRIG_GLOBAL)))
                interval_mtrigger(ch, trigFlag);
        }
    }

    for (auto obj = object_list; obj; obj = obj->next) {
        if (SCRIPT(obj)) {
            auto sc = SCRIPT(obj);

            if (IS_SET(SCRIPT_TYPES(sc), trigFlag))
                interval_otrigger(obj, trigFlag);
        }
    }

    for (auto &[vn, r] : world) {
        if (SCRIPT(&r)) {
            auto sc = SCRIPT(&r);

            if (IS_SET(SCRIPT_TYPES(sc), trigFlag) &&
                (!is_empty(r.zone) ||
                 IS_SET(SCRIPT_TYPES(sc), WTRIG_GLOBAL)))
                interval_wtrigger(&r, trigFlag);
        }
    }
}


void do_stat_trigger(struct char_data *ch, trig_data *trig) {
    struct cmdlist_element *cmd_list;
    char sb[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int len = 0;

    if (!trig) {
        basic_mud_log("SYSERR: nullptr trigger passed to do_stat_trigger.");
        return;
    }

    len += snprintf(sb, sizeof(sb), "Name: '@y%s@n',  VNum: [@g%5d@n], RNum: [%5d]\r\n",
                    GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), GET_TRIG_RNUM(trig));

    if (trig->attach_type == OBJ_TRIGGER) {
        len += snprintf(sb + len, sizeof(sb) - len, "Trigger Intended Assignment: Objects\r\n");
        sprintbit(GET_TRIG_TYPE(trig), otrig_types, buf, sizeof(buf));
    } else if (trig->attach_type == WLD_TRIGGER) {
        len += snprintf(sb + len, sizeof(sb) - len, "Trigger Intended Assignment: Rooms\r\n");
        sprintbit(GET_TRIG_TYPE(trig), wtrig_types, buf, sizeof(buf));
    } else {
        len += snprintf(sb + len, sizeof(sb) - len, "Trigger Intended Assignment: Mobiles\r\n");
        sprintbit(GET_TRIG_TYPE(trig), trig_types, buf, sizeof(buf));
    }

    len += snprintf(sb + len, sizeof(sb) - len, "Trigger Type: %s, Numeric Arg: %d, Arg list: %s\r\n",
                    buf, GET_TRIG_NARG(trig),
                    ((GET_TRIG_ARG(trig) && *GET_TRIG_ARG(trig))
                     ? GET_TRIG_ARG(trig) : "None"));

    len += snprintf(sb + len, sizeof(sb) - len, "Commands:\r\n");

    cmd_list = trig->cmdlist;
    while (cmd_list) {
        if (cmd_list->cmd)
            len += snprintf(sb + len, sizeof(sb) - len, "%s\r\n", cmd_list->cmd);

        if (len > MAX_STRING_LENGTH - 80) {
            len += snprintf(sb + len, sizeof(sb) - len, "*** Overflow - script too long! ***\r\n");
            break;
        }
        cmd_list = cmd_list->next;
    }

    ch->desc->sendText(sb);
}


/* find the name of what the uid points to */
void find_uid_name(char *uid, char *name, size_t nlen) {
    char_data *ch;
    obj_data *obj;

    if ((ch = get_char(uid)))
        snprintf(name, nlen, "%s", ch->name);
    else if ((obj = get_obj(uid)))
        snprintf(name, nlen, "%s", obj->name);
    else
        snprintf(name, nlen, "uid = %s, (not found)", uid + 1);
}


/* general function to display stats on script sc */
void script_stat(char_data *ch, struct script_data *sc) {
    struct trig_var_data *tv;
    trig_data *t;
    char name[MAX_INPUT_LENGTH];
    char namebuf[512];
    char buf1[MAX_STRING_LENGTH];

    send_to_char(ch, "Global Variables: %s\r\n", sc->global_vars ? "" : "None");
    send_to_char(ch, "Global context: %ld\r\n", sc->context);

    for (tv = sc->global_vars; tv; tv = tv->next) {
        snprintf(namebuf, sizeof(namebuf), "%s:%ld", tv->name, tv->context);
        if (*(tv->value) == UID_CHAR) {
            std::optional<DgUID> result;
            result = resolveUID(tv->value);
            auto uidResult = result;
            if(uidResult) {
				auto idx = uidResult->index();
                std::string n;
                if(idx == 0) {
                    // Room.
                    auto thing = std::get<0>(*uidResult);
                    n = thing->name;
                } else if(idx == 1) {
                    // object
                    auto thing = std::get<1>(*uidResult);
                    n = thing->name;
                } else if(idx == 2) {
                    // character or player...
                    auto thing = std::get<2>(*uidResult);
                    n = thing->name;
                }
                send_to_char(ch, "    %15s:  %s\r\n", tv->context ? namebuf : tv->name, n.c_str());
            } else {
                send_to_char(ch, "   -BAD UID: %s", tv->value);
            }
        } else
            send_to_char(ch, "    %15s:  %s\r\n", tv->context ? namebuf : tv->name, tv->value);
    }

    for (t = TRIGGERS(sc); t; t = t->next) {
        send_to_char(ch, "\r\n  Trigger: @y%s@n, VNum: [@y%5d@n], RNum: [%5d]\r\n",
                     GET_TRIG_NAME(t), GET_TRIG_VNUM(t), GET_TRIG_RNUM(t));

        if (t->attach_type == OBJ_TRIGGER) {
            send_to_char(ch, "  Trigger Intended Assignment: Objects\r\n");
            sprintbit(GET_TRIG_TYPE(t), otrig_types, buf1, sizeof(buf1));
        } else if (t->attach_type == WLD_TRIGGER) {
            send_to_char(ch, "  Trigger Intended Assignment: Rooms\r\n");
            sprintbit(GET_TRIG_TYPE(t), wtrig_types, buf1, sizeof(buf1));
        } else {
            send_to_char(ch, "  Trigger Intended Assignment: Mobiles\r\n");
            sprintbit(GET_TRIG_TYPE(t), trig_types, buf1, sizeof(buf1));
        }

        send_to_char(ch, "  Trigger Type: %s, Numeric Arg: %d, Arg list: %s\r\n",
                     buf1, GET_TRIG_NARG(t),
                     ((GET_TRIG_ARG(t) && *GET_TRIG_ARG(t)) ? GET_TRIG_ARG(t) :
                      "None"));

        if (t->waiting != 0.0) {
            send_to_char(ch, "    Wait: %f seconds, Current line: %s\r\n",
                         t->waiting,
                         t->curr_state ? t->curr_state->cmd : "End of Script");
            send_to_char(ch, "  Variables: %s\r\n", GET_TRIG_VARS(t) ? "" : "None");

            for (tv = GET_TRIG_VARS(t); tv; tv = tv->next) {
                if (*(tv->value) == UID_CHAR) {
                    std::optional<DgUID> result;
                    result = resolveUID(tv->value);
                    auto uidResult = result;
                    if(uidResult) {
                        auto idx = uidResult->index();
                        std::string n;
                        if(idx == 0) {
                            // Room.
                            auto thing = std::get<0>(*uidResult);
                            n = thing->name;
                        } else if(idx == 1) {
                            // object
                            auto thing = std::get<1>(*uidResult);
                            n = thing->name;
                        } else if(idx == 2) {
                            // character or player...
                            auto thing = std::get<2>(*uidResult);
                            n = thing->name;
                        }
                        send_to_char(ch, "    %15s:  %s\r\n", tv->name, n.c_str());
                    } else {
                        send_to_char(ch, "   -BAD UID: %s", tv->value);
                    }
                } else {
                    send_to_char(ch, "    %15s:  %s\r\n", tv->name, tv->value);
                }

            }
        }
    }
}


void do_sstat(struct char_data *ch, struct unit_data *ud) {
    send_to_char(ch, "Triggers:\r\n");
    if (!SCRIPT(ud)) {
        send_to_char(ch, "  None.\r\n");
        return;
    }

    script_stat(ch, SCRIPT(ud));
}

int64_t nextTrigID() {
    int64_t id = 0;
    while(uniqueScripts.contains(id)) id++;
    return id;
}

/*
 * adds the trigger t to script sc in in location loc.  loc = -1 means
 * add to the end, loc = 0 means add before all other triggers.
 */
void add_trigger(struct script_data *sc, trig_data *t, int loc) {

    // Gather up all existing triggers from the manual linked list...
    std::list<trig_data*> triggers;
    for(auto t2 = TRIGGERS(sc); t2; t2 = t2->next) {
        triggers.push_back(t2);
    }

    // Now insert t in the right spot...
    if(loc == -1) {
        triggers.push_back(t);
    } else if(loc == 0) {
        triggers.push_front(t);
    } else {
        auto it = triggers.begin();
        std::advance(it, loc);
        triggers.insert(it, t);
    }

    sc->trig_list = nullptr;
    // Reverse iterate through triggers to rebuild sc->trig_list...
    for(auto it = triggers.rbegin(); it != triggers.rend(); it++) {
        auto t2 = *it;
        t2->next = sc->trig_list;
        sc->trig_list = t2;
    }

    SCRIPT_TYPES(sc) |= GET_TRIG_TYPE(t);
    t->activate();
    t->owner = sc->owner;
    t->id = nextTrigID();
    t->generation = time(nullptr);

    uniqueScripts[t->id] = std::make_pair(t->generation, t);

    int order = 0;
    for(auto t2 = TRIGGERS(sc); t2; t2 = t2->next) {
        t2->order = order++;
    }

}


ACMD(do_attach) {
    char_data *victim;
    obj_data *object;
    room_data *room;
    trig_data *trig;
    char targ_name[MAX_INPUT_LENGTH], trig_name[MAX_INPUT_LENGTH];
    char loc_name[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
    int loc, tn, rn, num_arg;
    room_rnum rnum;

    argument = two_arguments(argument, arg, trig_name);
    two_arguments(argument, targ_name, loc_name);

    if (!*arg || !*targ_name || !*trig_name) {
        send_to_char(ch, "Usage: attach { mob | obj | room } { trigger } { name } [ location ]\r\n");
        return;
    }

    num_arg = atoi(targ_name);
    tn = atoi(trig_name);
    loc = (*loc_name) ? atoi(loc_name) : -1;

    if (is_abbrev(arg, "mobile") || is_abbrev(arg, "mtr")) {
        victim = get_char_vis(ch, targ_name, nullptr, FIND_CHAR_WORLD);
        if (!victim) { /* search room for one with this vnum */
            for (victim = ch->getRoom()->people; victim; victim = victim->next_in_room)
                if (GET_MOB_VNUM(victim) == num_arg)
                    break;

            if (!victim) {
                send_to_char(ch, "That mob does not exist.\r\n");
                return;
            }
        }
        if (!IS_NPC(victim)) {
            send_to_char(ch, "Players can't have scripts.\r\n");
            return;
        }
        if (!can_edit_zone(ch, ch->getRoom()->zone)) {
            send_to_char(ch, "You can only attach triggers in your own zone\r\n");
            return;
        }
        /* have a valid mob, now get trigger */
        rn = real_trigger(tn);
        if ((rn == NOTHING) || !(trig = read_trigger(rn))) {
            send_to_char(ch, "That trigger does not exist.\r\n");
            return;
        }

        if (!SCRIPT(victim)) victim->script = new script_data(victim);
        add_trigger(SCRIPT(victim), trig, loc);

        send_to_char(ch, "Trigger %d (%s) attached to %s [%d].\r\n",
                     tn, GET_TRIG_NAME(trig), GET_SHORT(victim), GET_MOB_VNUM(victim));
    } else if (is_abbrev(arg, "object") || is_abbrev(arg, "otr")) {
        object = get_obj_vis(ch, targ_name, nullptr);
        if (!object) { /* search room for one with this vnum */
            for (object = ch->getRoom()->contents; object; object = object->next_content)
                if (GET_OBJ_VNUM(object) == num_arg)
                    break;

            if (!object) { /* search inventory for one with this vnum */
                for (object = ch->contents; object; object = object->next_content)
                    if (GET_OBJ_VNUM(object) == num_arg)
                        break;

                if (!object) {
                    send_to_char(ch, "That object does not exist.\r\n");
                    return;
                }
            }
        }

        if (!can_edit_zone(ch, ch->getRoom()->zone)) {
            send_to_char(ch, "You can only attach triggers in your own zone\r\n");
            return;
        }
        /* have a valid obj, now get trigger */
        rn = real_trigger(tn);
        if ((rn == NOTHING) || !(trig = read_trigger(rn))) {
            send_to_char(ch, "That trigger does not exist.\r\n");
            return;
        }

        if (!SCRIPT(object)) object->script = new script_data(object);
        add_trigger(SCRIPT(object), trig, loc);

        send_to_char(ch, "Trigger %d (%s) attached to %s [%d].\r\n",
                     tn, GET_TRIG_NAME(trig),
                     (object->short_description ?
                      object->short_description : object->name),
                     GET_OBJ_VNUM(object));
    } else if (is_abbrev(arg, "room") || is_abbrev(arg, "wtr")) {
        if (strchr(targ_name, '.'))
            rnum = IN_ROOM(ch);
        else if (isdigit(*targ_name))
            rnum = find_target_room(ch, targ_name);
        else
            rnum = NOWHERE;

        if (rnum == NOWHERE) {
            send_to_char(ch, "You need to supply a room number or . for current room.\r\n");
            return;
        }

        if (!can_edit_zone(ch, world[rnum].zone)) {
            send_to_char(ch, "You can only attach triggers in your own zone\r\n");
            return;
        }
        /* have a valid room, now get trigger */
        rn = real_trigger(tn);
        if ((rn == NOTHING) || !(trig = read_trigger(rn))) {
            send_to_char(ch, "That trigger does not exist.\r\n");
            return;
        }

        room = &world[rnum];

        if (!SCRIPT(room)) room->script = new script_data(room);
        add_trigger(SCRIPT(room), trig, loc);

        send_to_char(ch, "Trigger %d (%s) attached to room %d.\r\n",
                     tn, GET_TRIG_NAME(trig), world[rnum].vn);
    } else
        send_to_char(ch, "Please specify 'mob', 'obj', or 'room'.\r\n");
}


/*
 *  removes the trigger specified by name, and the script of o if
 *  it removes the last trigger.  name can either be a number, or
 *  a 'silly' name for the trigger, including things like 2.beggar-death.
 *  returns 0 if did not find the trigger, otherwise 1.  If it matters,
 *  you might need to check to see if all the triggers were removed after
 *  this function returns, in order to remove the script.
 */
int remove_trigger(struct script_data *sc, char *name) {
    trig_data *i, *j;
    int num = 0, string = false, n;
    char *cname;


    if (!sc)
        return 0;

    if ((cname = strstr(name, ".")) || (!isdigit(*name))) {
        string = true;
        if (cname) {
            *cname = '\0';
            num = atoi(name);
            name = ++cname;
        }
    } else
        num = atoi(name);

    for (n = 0, j = nullptr, i = TRIGGERS(sc); i; j = i, i = i->next) {
        if (string) {
            if (isname(name, GET_TRIG_NAME(i)))
                if (++n >= num)
                    break;
        }

            /* this isn't clean... */
            /* a numeric value will match if it's position OR vnum */
            /* is found. originally the number was position-only */
        else if (++n >= num)
            break;
        else if (trig_index[i->vn].vn == num)
            break;
    }

    if (i) {
        if (j) {
            j->next = i->next;
            extract_trigger(i);
        }

            /* this was the first trigger */
        else {
            TRIGGERS(sc) = i->next;
            extract_trigger(i);
        }

        /* update the script type bitvector */
        SCRIPT_TYPES(sc) = 0;
        for (i = TRIGGERS(sc); i; i = i->next)
            SCRIPT_TYPES(sc) |= GET_TRIG_TYPE(i);

        return 1;
    } else
        return 0;
}

ACMD(do_detach) {
    char_data *victim = nullptr;
    obj_data *object = nullptr;
    struct room_data *room;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
    char *trigger = nullptr;
    int num_arg;

    argument = two_arguments(argument, arg1, arg2);
    one_argument(argument, arg3);

    if (!*arg1 || !*arg2) {
        send_to_char(ch, "Usage: detach [ mob | object | room ] { target } { trigger |"
                         " 'all' }\r\n");
        return;
    }

    /* vnum of mob/obj, if given */
    num_arg = atoi(arg2);

    if (!strcasecmp(arg1, "room") || !strcasecmp(arg1, "wtr")) {
        room = ch->getRoom();
        if (!can_edit_zone(ch, room->zone)) {
            send_to_char(ch, "You can only detach triggers in your own zone\r\n");
            return;
        }
        if (!SCRIPT(room))
            send_to_char(ch, "This room does not have any triggers.\r\n");
        else if (!strcasecmp(arg2, "all")) {
            extract_script(room, WLD_TRIGGER);
            send_to_char(ch, "All triggers removed from room.\r\n");
        } else if (remove_trigger(SCRIPT(room), arg2)) {
            send_to_char(ch, "Trigger removed.\r\n");
            if (!TRIGGERS(SCRIPT(room))) {
                extract_script(room, WLD_TRIGGER);
            }
        } else
            send_to_char(ch, "That trigger was not found.\r\n");
    } else {
        if (is_abbrev(arg1, "mobile") || !strcasecmp(arg1, "mtr")) {
            victim = get_char_vis(ch, arg2, nullptr, FIND_CHAR_WORLD);
            if (!victim) { /* search room for one with this vnum */
                for (victim = ch->getRoom()->people; victim; victim = victim->next_in_room)
                    if (GET_MOB_VNUM(victim) == num_arg)
                        break;

                if (!victim) {
                    send_to_char(ch, "No such mobile around.\r\n");
                    return;
                }
            }

            if (arg3 == nullptr || !*arg3)
                send_to_char(ch, "You must specify a trigger to remove.\r\n");
            else
                trigger = arg3;
        } else if (is_abbrev(arg1, "object") || !strcasecmp(arg1, "otr")) {
            object = get_obj_vis(ch, arg2, nullptr);
            if (!object) { /* search room for one with this vnum */
                for (object = ch->getRoom()->contents; object; object = object->next_content)
                    if (GET_OBJ_VNUM(object) == num_arg)
                        break;

                if (!object) { /* search inventory for one with this vnum */
                    for (object = ch->contents; object; object = object->next_content)
                        if (GET_OBJ_VNUM(object) == num_arg)
                            break;

                    if (!object) { /* give up */
                        send_to_char(ch, "No such object around.\r\n");
                        return;
                    }
                }
            }

            if (arg3 == nullptr || !*arg3)
                send_to_char(ch, "You must specify a trigger to remove.\r\n");
            else
                trigger = arg3;
        } else {
            /* Thanks to Carlos Myers for fixing the line below */
            if ((object = get_obj_in_equip_vis(ch, arg1, nullptr, ch->equipment)));
            else if ((object = get_obj_in_list_vis(ch, arg1, nullptr, ch->contents)));
            else if ((victim = get_char_room_vis(ch, arg1, nullptr)));
            else if ((object = get_obj_in_list_vis(ch, arg1, nullptr, ch->getRoom()->contents)));
            else if ((victim = get_char_vis(ch, arg1, nullptr, FIND_CHAR_WORLD)));
            else if ((object = get_obj_vis(ch, arg1, nullptr)));
            else
                send_to_char(ch, "Nothing around by that name.\r\n");

            trigger = arg2;
        }

        if (victim) {
            if (!IS_NPC(victim))
                send_to_char(ch, "Players don't have triggers.\r\n");

            else if (!SCRIPT(victim))
                send_to_char(ch, "That mob doesn't have any triggers.\r\n");
            else if (!can_edit_zone(ch, real_zone_by_thing(GET_MOB_VNUM(victim)))) {
                send_to_char(ch, "You can only detach triggers in your own zone\r\n");
                return;
            } else if (trigger && !strcasecmp(trigger, "all")) {
                extract_script(victim, MOB_TRIGGER);
                send_to_char(ch, "All triggers removed from %s.\r\n", GET_SHORT(victim));
            } else if (trigger && remove_trigger(SCRIPT(victim), trigger)) {
                send_to_char(ch, "Trigger removed.\r\n");
                if (!TRIGGERS(SCRIPT(victim))) {
                    extract_script(victim, MOB_TRIGGER);
                }
            } else
                send_to_char(ch, "That trigger was not found.\r\n");
        } else if (object) {
            if (!SCRIPT(object))
                send_to_char(ch, "That object doesn't have any triggers.\r\n");

            else if (!can_edit_zone(ch, real_zone_by_thing(GET_OBJ_VNUM(object)))) {
                send_to_char(ch, "You can only detach triggers in your own zone\r\n");
                return;
            } else if (trigger && !strcasecmp(trigger, "all")) {
                extract_script(object, OBJ_TRIGGER);
                send_to_char(ch, "All triggers removed from %s.\r\n",
                             object->short_description ? object->short_description :
                             object->name);
            } else if (remove_trigger(SCRIPT(object), trigger)) {
                send_to_char(ch, "Trigger removed.\r\n");
                if (!TRIGGERS(SCRIPT(object))) {
                    extract_script(object, OBJ_TRIGGER);
                }
            } else
                send_to_char(ch, "That trigger was not found.\r\n");
        }
    }
}

/*
 *  Logs any errors caused by scripts to the system log.
 *  Will eventually allow on-line view of script errors.
 */
void script_vlog(const char *format, va_list args) {
    char output[MAX_STRING_LENGTH];
    struct descriptor_data *i;

    snprintf(output, sizeof(output), "SCRIPT ERR: %s", format);

    // wrap up basic_mud_vlog in a va copy...
    va_list args_copy;
    va_copy(args_copy, args);
    basic_mud_vlog(output, args_copy);
    va_end(args_copy);

    /* the rest is mostly a rip from basic_mud_log() */
    strcpy(output, "[ ");            /* strcpy: OK */
    vsnprintf(output + 2, sizeof(output) - 6, format, args);
    strcat(output, " ]\r\n");        /* strcat: OK */

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || IS_NPC(i->character)) /* switch */
            continue;
        if (GET_ADMLEVEL(i->character) < ADMLVL_BUILDER)
            continue;
        if (PLR_FLAGGED(i->character, PLR_WRITING))
            continue;
        if (NRM > (PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0))
            continue;

        send_to_char(i->character, "@g%s@n", output);
    }
}


void script_log(const char *format, ...) {
    va_list args;

    va_start(args, format);
    script_vlog(format, args);
    va_end(args);
}

bool is_num(const std::string& arg) {
    if (arg.empty()) return false;

    bool decimal_point_found = false;
    for (char c : arg) {
        if (c == '.') {
            if (decimal_point_found) {
                // More than one decimal point found, not a number
                return false;
            }
            decimal_point_found = true;
        } else if (!std::isdigit(c)) {
            // Non-digit character found, not a number
            return false;
        }
    }
    return true;
}

static void eval_numeric_op(char *op, char *lhs, char *rhs, char *result) {
    auto l = atof(lhs);
    auto r = atof(rhs);

    std::string res;

    /* find the op, and figure out the value */
    if (!strcmp("||", op)) {
        res = fmt::format("{}", (l || r) ? 1 : 0);
    } else if (!strcmp("&&", op)) {
        res = fmt::format("{}", (l && r) ? 1 : 0);
    } else if (!strcmp("==", op)) {
        res = fmt::format("{}", (l == r) ? 1 : 0);
    } else if (!strcmp("!=", op)) {
        res = fmt::format("{}", (l != r) ? 1 : 0);
    } else if (!strcmp("<=", op)) {
        res = fmt::format("{}", (l <= r) ? 1 : 0);
    } else if (!strcmp(">=", op)) {
        res = fmt::format("{}", (l >= r) ? 1 : 0);
    } else if (!strcmp("<", op)) {
        res = fmt::format("{}", (l < r) ? 1 : 0);
    } else if (!strcmp(">", op)) {
        res = fmt::format("{}", (l > r) ? 1 : 0);
    }else if (!strcmp("*", op))
        res = fmt::format("{}", l * r);
    else if (!strcmp("/", op))
        res = fmt::format("{}", (int64_t)l / (int64_t)r);
    else if (!strcmp("+", op))
        res = fmt::format("{}", l + r);
    else if (!strcmp("-", op))
        res = fmt::format("{}", l - r);
    else if (!strcmp("!", op)) {
        res = fmt::format("{}", (r != 0.0) ? 0 : 1);
    }

    if(!res.empty()) strcpy(result, res.c_str());

}

static bool check_truthy(const char *txt) {
    if(!txt || !strlen(txt)) return false;

    // Try to convert to a number and check if it's zero
    std::stringstream str(txt);
    double num;
    str >> num;
    if (!str.fail() && num == 0.0) return false;

    // If not null, not empty, and not zero, it's truthy
    return true;

}


/* evaluates 'lhs op rhs', and copies to result */
void eval_op(char *op, char *lhs, char *rhs, char *result, void *go,
             struct script_data *sc, trig_data *trig) {
    unsigned char *p;
    int n;

    /* strip off extra spaces at begin and end */
    while (*lhs && isspace(*lhs))
        lhs++;
    while (*rhs && isspace(*rhs))
        rhs++;

    for (p = (unsigned char *) lhs; *p; p++);
    for (--p; isspace(*p) && ((char *) p > lhs); *p-- = '\0');
    for (p = (unsigned char *) rhs; *p; p++);
    for (--p; isspace(*p) && ((char *) p > rhs); *p-- = '\0');


    if(is_num(lhs) && is_num(rhs)) {
        eval_numeric_op(op, lhs, rhs, result);
        return;
    }

    /* find the op, and figure out the value */
    if (!strcmp("||", op)) {
        if ((!*lhs || (*lhs == '0')) && (!*rhs || (*rhs == '0')))
            strcpy(result, "0");
        else
            strcpy(result, "1");
    } else if (!strcmp("&&", op)) {
        if (!*lhs || (*lhs == '0') || !*rhs || (*rhs == '0'))
            strcpy(result, "0");
        else
            strcpy(result, "1");
    } else if (!strcmp("==", op)) {
        sprintf(result, "%d", !strcasecmp(lhs, rhs));
    } else if (!strcmp("!=", op)) {
        sprintf(result, "%d", strcasecmp(lhs, rhs));
    } else if (!strcmp("<=", op)) {
        sprintf(result, "%lld", strcasecmp(lhs, rhs) <= 0);
    } else if (!strcmp(">=", op)) {
        sprintf(result, "%lld", strcasecmp(lhs, rhs) <= 0);
    } else if (!strcmp("<", op)) {
        sprintf(result, "%d", strcasecmp(lhs, rhs) < 0);
    } else if (!strcmp(">", op)) {
        sprintf(result, "%d", strcasecmp(lhs, rhs) > 0);
    } else if (!strcmp("/=", op))
        sprintf(result, "%c", str_str(lhs, rhs) ? '1' : '0');
    else if (!strcmp("!", op)) {
        sprintf(result, "%d", !check_truthy(rhs));
    }
}


/*
 * p points to the first quote, returns the matching
 * end quote, or the last non-null char in p.
*/
char *matching_quote(char *p) {
    for (p++; *p && (*p != '"'); p++) {
        if (*p == '\\')
            p++;
    }

    if (!*p)
        p--;

    return p;
}

/*
 * p points to the first paren.  returns a pointer to the
 * matching closing paren, or the last non-null char in p.
 */
char *matching_paren(char *p) {
    int i;

    for (p++, i = 1; *p && i; p++) {
        if (*p == '(')
            i++;
        else if (*p == ')')
            i--;
        else if (*p == '"')
            p = matching_quote(p);
    }

    return --p;
}


/* evaluates line, and returns answer in result */
void eval_expr(char *line, char *result, void *go, struct script_data *sc,
               trig_data *trig, int type) {
    char expr[MAX_INPUT_LENGTH], *p;

    while (*line && isspace(*line))
        line++;

    if (*line == '(') {
        p = strcpy(expr, line);
        p = matching_paren(expr);
        *p = '\0';
        eval_expr(expr + 1, result, go, sc, trig, type);
    } else if (eval_lhs_op_rhs(line, result, go, sc, trig, type)) {

    } else
        var_subst(go, sc, trig, type, line, result);
}

/*
 * evaluates expr if it is in the form lhs op rhs, and copies
 * answer in result.  returns 1 if expr is evaluated, else 0
 */
int eval_lhs_op_rhs(char *expr, char *result, void *go, struct script_data *sc,
                    trig_data *trig, int type)
{
    char *p, *tokens[MAX_INPUT_LENGTH];
    char line[MAX_INPUT_LENGTH], lhr[MAX_INPUT_LENGTH], rhr[MAX_INPUT_LENGTH];
    int i, j;

    /*
     * valid operands, in order of priority
     * each must also be defined in eval_op()
     */
    static char *ops[] = {
        "||",
        "&&",
        "==",
        "!=",
        "<=",
        ">=",
        "<",
        ">",
        "/=",
        "-",
        "+",
        "/",
        "*",
        "!",
        "\n"
      };

    p = strcpy(line, expr);

    /*
     * initialize tokens, an array of pointers to locations
     * in line where the ops could possibly occur.
     */

    /* Might be game breaking - Iovan
    *lhr = '\0'; */

    for (j = 0; *p; j++) {
        tokens[j] = p;
        if (*p == '(')
            p = matching_paren(p) + 1;
        else if (*p == '"')
            p = matching_quote(p) + 1;
        else if (isalnum(*p))
            for (p++; *p && (isalnum(*p) || isspace(*p)); p++);
        else
            p++;
    }
    tokens[j] = nullptr;

    for (i = 0; *ops[i] != '\n'; i++)
        for (j = 0; tokens[j]; j++)
            if (!strncasecmp(ops[i], tokens[j], strlen(ops[i]))) {
                *tokens[j] = '\0';
                p = tokens[j] + strlen(ops[i]);

                eval_expr(line, lhr, go, sc, trig, type);
                eval_expr(p, rhr, go, sc, trig, type);
                eval_op(ops[i], lhr, rhr, result, go, sc, trig);

                return 1;
            }

    return 0;
}


/* returns 1 if cond is true, else 0 */
int process_if(char *cond, void *go, struct script_data *sc,
               trig_data *trig, int type) {
    char result[MAX_INPUT_LENGTH], *p;

    eval_expr(cond, result, go, sc, trig, type);

    p = result;
    skip_spaces(&p);

    if (!*p || *p == '0')
        return 0;
    else
        return 1;
}


/*
 * scans for end of if-block.
 * returns the line containg 'end', or the last
 * line of the trigger if not found.
 */
struct cmdlist_element *find_end(trig_data *trig, struct cmdlist_element *cl) {
    struct cmdlist_element *c;
    char *p;

    if (!(cl->next)) { /* rryan: if this is the last line, theres no end. */
        script_log("Trigger VNum %lld has 'if' without 'end'. (error 1)", GET_TRIG_VNUM(trig));
        return cl;
    }

    for (c = cl->next; c; c = c->next) {
        for (p = c->cmd; *p && isspace(*p); p++);

        if (!strncasecmp("if ", p, 3))
            c = find_end(trig, c);
        else if (!strncasecmp("end", p, 3))
            return c;

        /* thanks to Russell Ryan for this fix */
        if (!c->next) { /* rryan: this is the last line, we didn't find an end. */
            script_log("Trigger VNum %lld has 'if' without 'end'. (error 2)", GET_TRIG_VNUM(trig));
            return c;
        }
    }

    /* rryan: we didn't find an end. */
    script_log("Trigger VNum %lld has 'if' without 'end'. (error 3)", GET_TRIG_VNUM(trig));
    return c;
}


/*
 * searches for valid elseif, else, or end to continue execution at.
 * returns line of elseif, else, or end if found, or last line of trigger.
 */
struct cmdlist_element *find_else_end(trig_data *trig,
                                      struct cmdlist_element *cl, void *go,
                                      struct script_data *sc, int type) {
    struct cmdlist_element *c;
    char *p;

    if (!(cl->next))
        return cl;

    for (c = cl->next; c->next; c = c->next) {
        for (p = c->cmd; *p && isspace(*p); p++); /* skip spaces */

        if (!strncasecmp("if ", p, 3))
            c = find_end(trig, c);

        else if (!strncasecmp("elseif ", p, 7)) {
            if (process_if(p + 7, go, sc, trig, type)) {
                GET_TRIG_DEPTH(trig)++;
                return c;
            }
        } else if (!strncasecmp("else", p, 4)) {
            GET_TRIG_DEPTH(trig)++;
            return c;
        } else if (!strncasecmp("end", p, 3))
            return c;

        /* thanks to Russell Ryan for this fix */
        if (!c->next) { /* rryan: this is the last line, return. */
            script_log("Trigger VNum %lld has 'if' without 'end'. (error 4)", GET_TRIG_VNUM(trig));
            return c;
        }
    }

    /* rryan: if we got here, it's the last line, if its not an end, script_log it. */
    for (p = c->cmd; *p && isspace(*p); p++); /* skip spaces */
    if (strncasecmp("end", p, 3))
        script_log("Trigger VNum %lld has 'if' without 'end'. (error 5)", GET_TRIG_VNUM(trig));
    return c;
}


/* processes any 'wait' commands in a trigger */
void process_wait(void *go, trig_data *trig, int type, char *cmd,
                  struct cmdlist_element *cl) {
    char buf[MAX_INPUT_LENGTH], *arg;
    struct wait_event_data *wait_event_obj;
    long when, hr, min, ntime;
    char c;

    arg = any_one_arg(cmd, buf);
    skip_spaces(&arg);

    if (!*arg) {
        script_log("Trigger: %s, VNum %d. wait w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cl->cmd);
        return;
    }

    if (!strncasecmp(arg, "until ", 6)) {

        /* valid forms of time are 14:30 and 1430 */
        if (sscanf(arg, "until %ld:%ld", &hr, &min) == 2)
            min += (hr * 60);
        else
            min = (hr % 100) + ((hr / 100) * 60);

        // Convert current MUD time to seconds since midnight
        double current_mud_seconds = time_info.seconds +
                                     (time_info.minutes * SECS_PER_MINUTE) +
                                     (time_info.hours * SECS_PER_HOUR);

        // Convert target MUD time to seconds since midnight
        double target_mud_seconds = min * SECS_PER_MINUTE;

        // Determine waiting time in MUD seconds
        double waiting_mud_seconds;
        if (current_mud_seconds >= target_mud_seconds) {
            // If the target time has already passed, wait until the next day
            waiting_mud_seconds = (SECS_PER_DAY - current_mud_seconds) + target_mud_seconds;
        } else {
            // If the target time is in the future, wait until that time
            waiting_mud_seconds = target_mud_seconds - current_mud_seconds;
        }

        // Convert waiting time to real seconds
        trig->waiting = waiting_mud_seconds * SECS_PER_MUD_SECOND;

    } else {
        if (sscanf(arg, "%ld %c", &when, &c) == 2) {
            if (c == 't')
                when *= PULSES_PER_MUD_HOUR;
            else if (c == 's')
                when *= PASSES_PER_SEC;
        }
        // We need to convert 'when' into a double of seconds-to-wait by dividing by PASSES_PER_SEC.
        trig->waiting = (double) when / (double) PASSES_PER_SEC;
    }

    // we're replacing the old wait_event_obj.

    triggers_waiting.insert(trig);

    trig->curr_state = cl->next;
}


/* processes a script set command */
void process_set(struct script_data *sc, trig_data *trig, char *cmd) {
    char arg[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], *value;

    value = two_arguments(cmd, arg, name);

    skip_spaces(&value);

    if (!*name) {
        script_log("Trigger: %s, VNum %d. set w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    add_var(&GET_TRIG_VARS(trig), name, value, sc ? sc->context : 0);

}

/* processes a script eval command */
void process_eval(void *go, struct script_data *sc, trig_data *trig,
                  int type, char *cmd) {
    char arg[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    char result[MAX_INPUT_LENGTH], *expr;

    expr = one_argument(cmd, arg); /* cut off 'eval' */
    expr = one_argument(expr, name); /* cut off name */

    skip_spaces(&expr);

    if (!*name) {
        script_log("Trigger: %s, VNum %d. eval w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    if(isUID(expr)) {
        add_var(&GET_TRIG_VARS(trig), name, expr, sc ? sc->context : 0);
    } else {
        eval_expr(expr, result, go, sc, trig, type);
        add_var(&GET_TRIG_VARS(trig), name, result, sc ? sc->context : 0);
    }
}


/* script attaching a trigger to something */
void process_attach(void *go, struct script_data *sc, trig_data *trig,
                    int type, char *cmd) {
    char arg[MAX_INPUT_LENGTH], trignum_s[MAX_INPUT_LENGTH];
    char result[MAX_INPUT_LENGTH], *id_p;
    trig_data *newtrig;
    char_data *c = nullptr;
    obj_data *o = nullptr;
    room_data *r = nullptr;
    long trignum, id;

    id_p = two_arguments(cmd, arg, trignum_s);
    skip_spaces(&id_p);

    if (!*trignum_s) {
        script_log("Trigger: %s, VNum %d. attach w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    if(isUID(id_p)) {
        snprintf(result, sizeof(result), "%s", id_p);
    } else {
        /* parse and locate the id specified */
        eval_expr(id_p, result, go, sc, trig, type);
    }

    /* parse and locate the id specified */
    std::optional<DgUID> result1 = resolveUID(result);
    auto uidResult = result1;
    if(!uidResult) {
        script_log("Trigger: %s, VNum %d. attach invalid id arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    switch(uidResult->index()) {
        case 0:
            r = std::get<0>(*uidResult);
            break;
        case 1:
            o = std::get<1>(*uidResult);
            break;
        case 2:
            c = std::get<2>(*uidResult);
            break;
    }

    /* locate and load the trigger specified */
    trignum = real_trigger(atoi(trignum_s));
    if (trignum == NOTHING || !(newtrig = read_trigger(trignum))) {
        script_log("Trigger: %s, VNum %d. attach invalid trigger: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), trignum_s);
        return;
    }

    if (c) {
        if (!IS_NPC(c)) {
            script_log("Trigger: %s, VNum %d. attach invalid target: '%s'",
                       GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), GET_NAME(c));
            return;
        }
        if (!SCRIPT(c)) c->script = new script_data(c);
        add_trigger(SCRIPT(c), newtrig, -1);
        return;
    }

    if (o) {
        if (!SCRIPT(o)) o->script = new script_data(o);
        add_trigger(SCRIPT(o), newtrig, -1);
        return;
    }

    if (r) {
        if (!SCRIPT(r)) r->script = new script_data(r);
        add_trigger(SCRIPT(r), newtrig, -1);
        return;
    }

}


/* script detaching a trigger from something */
void process_detach(void *go, struct script_data *sc, trig_data *trig,
                    int type, char *cmd) {
    char arg[MAX_INPUT_LENGTH], trignum_s[MAX_INPUT_LENGTH];
    char result[MAX_INPUT_LENGTH], *id_p;
    char_data *c = nullptr;
    obj_data *o = nullptr;
    room_data *r = nullptr;
    long id;

    id_p = two_arguments(cmd, arg, trignum_s);
    skip_spaces(&id_p);

    if (!*trignum_s) {
        script_log("Trigger: %s, VNum %d. detach w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    if(isUID(id_p)) {
        snprintf(result, sizeof(result), "%s", id_p);
    } else {
        /* parse and locate the id specified */
        eval_expr(id_p, result, go, sc, trig, type);
    }

    /* parse and locate the id specified */
    std::optional<DgUID> result1 = resolveUID(result);

    auto uidResult = result1;
    if(!uidResult) {
        script_log("Trigger: %s, VNum %d. detach invalid id arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    switch(uidResult->index()) {
        case 0:
            r = std::get<0>(*uidResult);
            break;
        case 1:
            o = std::get<1>(*uidResult);
            break;
        case 2:
            c = std::get<2>(*uidResult);
            break;
    }

    if (c && SCRIPT(c)) {
        if (!strcmp(trignum_s, "all")) {
            extract_script(c, MOB_TRIGGER);
            return;
        }
        if (remove_trigger(SCRIPT(c), trignum_s)) {
            if (!TRIGGERS(SCRIPT(c))) {
                extract_script(c, MOB_TRIGGER);
            }
        }
        return;
    }

    if (o && SCRIPT(o)) {
        if (!strcmp(trignum_s, "all")) {
            extract_script(o, OBJ_TRIGGER);
            return;
        }
        if (remove_trigger(SCRIPT(o), trignum_s)) {
            if (!TRIGGERS(SCRIPT(o))) {
                extract_script(o, OBJ_TRIGGER);
            }
        }
        return;
    }

    if (r && SCRIPT(r)) {
        if (!strcmp(trignum_s, "all")) {
            extract_script(r, WLD_TRIGGER);
            return;
        }
        if (remove_trigger(SCRIPT(r), trignum_s)) {
            if (!TRIGGERS(SCRIPT(r))) {
                extract_script(r, WLD_TRIGGER);
            }
        }
        return;
    }

}

struct room_data *dg_room_of_obj(struct obj_data *obj) {
    return obj->getAbsoluteRoom();
}


/*
 * processes a script return command.
 * returns the new value for the script to return.
 */
int process_return(trig_data *trig, char *cmd) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    two_arguments(cmd, arg1, arg2);

    if (!*arg2) {
        script_log("Trigger: %s, VNum %d. return w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);

        return 1;
    }

    return atoi(arg2);
}


/*
 * removes a variable from the global vars of sc,
 * or the local vars of trig if not found in global list.
 */
void process_unset(struct script_data *sc, trig_data *trig, char *cmd) {
    char arg[MAX_INPUT_LENGTH], *var;

    var = any_one_arg(cmd, arg);

    skip_spaces(&var);

    if (!*var) {
        script_log("Trigger: %s, VNum %d. unset w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    if (!remove_var(&(sc->global_vars), var))
        remove_var(&GET_TRIG_VARS(trig), var);
}


/*
 * copy a locally owned variable to the globals of another script
 *     'remote <variable_name> <uid>'
 */
void process_remote(struct script_data *sc, trig_data *trig, char *cmd) {
    struct trig_var_data *vd;
    struct script_data *sc_remote = nullptr;
    char *line, *var, *uid_p;
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
    long uid, context;
    room_data *room;
    char_data *mob;
    obj_data *obj;

    line = any_one_arg(cmd, arg);
    two_arguments(line, buf, buf2);
    var = buf;
    uid_p = buf2;
    skip_spaces(&var);
    skip_spaces(&uid_p);


    if (!*buf || !*buf2) {
        script_log("Trigger: %s, VNum %d. remote: invalid arguments '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    /* find the locally owned variable */
    for (vd = GET_TRIG_VARS(trig); vd; vd = vd->next)
        if (!strcasecmp(vd->name, buf))
            break;

    if (!vd)
        for (vd = sc->global_vars; vd; vd = vd->next)
            if (!strcasecmp(vd->name, var) &&
                (vd->context == 0 || vd->context == sc->context))
                break;

    if (!vd) {
        script_log("Trigger: %s, VNum %d. local var '%s' not found in remote call",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), buf);
        return;
    }
    /* find the target script from the uid number */
    std::optional<DgUID> result;
    result = resolveUID(buf2);
    auto uidResult = result;
    if(!uidResult) {
        script_log("Trigger: %s, VNum %d. remote: illegal uid '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), buf2);
        return;
    }

    /* for all but PC's, context comes from the existing context. */
    /* for PC's, context is 0 (global) */
    context = vd->context;


    switch(uidResult->index()) {
        case 0:
            sc_remote = SCRIPT(std::get<0>(*uidResult));
            break;
        case 1:
            sc_remote = SCRIPT(std::get<1>(*uidResult));
            break;
        case 2:
            sc_remote = SCRIPT(std::get<2>(*uidResult));
            break;
    }

    if (sc_remote == nullptr) return; /* no script to assign */

    add_var(&(sc_remote->global_vars), vd->name, vd->value, context);
}


/*
 * command-line interface to rdelete
 * named vdelete so people didn't think it was to delete rooms
 */
ACMD(do_vdelete) {
    struct trig_var_data *vd, *vd_prev = nullptr;
    struct script_data *sc_remote = nullptr;
    char *var, *uid_p;
    char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
    long uid, context;
    room_data *room;
    char_data *mob;
    obj_data *obj;

    argument = two_arguments(argument, buf, buf2);
    var = buf;
    uid_p = buf2;
    skip_spaces(&var);
    skip_spaces(&uid_p);


    if (!*buf || !*buf2) {
        send_to_char(ch, "Usage: vdelete { <variablename> | * | all } <id>\r\n");
        return;
    }

    std::optional<DgUID> result;
    result = resolveUID(buf2);
    auto uidResult = result;
    if(!uidResult) {
        send_to_char(ch, "vdelete: illegal id specified.\r\n");
        return;
    }
    switch(uidResult->index()) {
        case 0:
            sc_remote = SCRIPT(std::get<0>(*uidResult));
            break;
        case 1:
            sc_remote = SCRIPT(std::get<1>(*uidResult));
            break;
        case 2:
            sc_remote = SCRIPT(std::get<2>(*uidResult));
            break;
    }

    if(!sc_remote) {
        send_to_char(ch, "vdelete: cannot resolve specified id.\r\n");
        return;
    }

    if (sc_remote->global_vars == nullptr) {
        send_to_char(ch, "That id represents no global variables.(2)\r\n");
        return;
    }

    if (*var == '*' || is_abbrev(var, "all")) {
        struct trig_var_data *vd_next;
        for (vd = sc_remote->global_vars; vd; vd = vd_next) {
            vd_next = vd->next;
            free(vd->value);
            free(vd->name);
            free(vd);
        }
        sc_remote->global_vars = nullptr;
        send_to_char(ch, "All variables deleted from that id.\r\n");
        return;
    }

    /* find the global */
    for (vd = sc_remote->global_vars; vd; vd_prev = vd, vd = vd->next)
        if (!strcasecmp(vd->name, var))
            break;

    if (!vd) {
        send_to_char(ch, "That variable cannot be located.\r\n");
        return;
    }

    /* ok, delete the variable */
    if (vd_prev) vd_prev->next = vd->next;
    else sc_remote->global_vars = vd->next;

    /* and free up the space */
    free(vd->value);
    free(vd->name);
    free(vd);

    send_to_char(ch, "Deleted.\r\n");
}

/*
 * Called from do_set - return 0 for failure, 1 for success.
 * ch and vict are verified
 */
int perform_set_dg_var(struct char_data *ch, struct char_data *vict, char *val_arg) {
    char var_name[MAX_INPUT_LENGTH], *var_value;

    var_value = any_one_arg(val_arg, var_name);

    if (var_name == nullptr || !*var_name || var_value == nullptr || !*var_value) {
        send_to_char(ch, "Usage: set <char> <varname> <value>\r\n");
        return 0;
    }
    if (!SCRIPT(vict)) vict->script = new script_data(vict);

    add_var(&(SCRIPT(vict)->global_vars), var_name, var_value, 0);
    return 1;
}

/*
 * delete a variable from the globals of another script
 *     'rdelete <variable_name> <uid>'
 */
void process_rdelete(struct script_data *sc, trig_data *trig, char *cmd) {
    struct trig_var_data *vd, *vd_prev = nullptr;
    struct script_data *sc_remote = nullptr;
    char *line, *var, *uid_p;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    long uid, context;
    room_data *room;
    char_data *mob;
    obj_data *obj;

    line = any_one_arg(cmd, arg);
    two_arguments(line, buf, buf2);
    var = buf;
    uid_p = buf2;
    skip_spaces(&var);
    skip_spaces(&uid_p);


    if (!*buf || !*buf2) {
        script_log("Trigger: %s, VNum %d. rdelete: invalid arguments '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    std::optional<DgUID> result;
    result = resolveUID(buf2);
    auto uidResult = result;
    if(!uidResult) {
        script_log("Trigger: %s, VNum %d. rdelete: illegal uid '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), buf2);
        return;
    }

    switch(uidResult->index()) {
        case 0:
            sc_remote = SCRIPT(std::get<0>(*uidResult));
            break;
        case 1:
            sc_remote = SCRIPT(std::get<1>(*uidResult));
            break;
        case 2:
            sc_remote = SCRIPT(std::get<2>(*uidResult));
            break;
    }

    if (sc_remote == nullptr) return; /* no script to delete a trigger from */
    if (sc_remote->global_vars == nullptr) return; /* no script globals */

    /* find the global */
    for (vd = sc_remote->global_vars; vd; vd_prev = vd, vd = vd->next)
        if (!strcasecmp(vd->name, var) &&
            (vd->context == 0 || vd->context == sc->context))
            break;

    if (!vd) return; /* the variable doesn't exist, or is the wrong context */

    /* ok, delete the variable */
    if (vd_prev) vd_prev->next = vd->next;
    else sc_remote->global_vars = vd->next;

    /* and free up the space */
    free(vd->value);
    free(vd->name);
    free(vd);
}


/*
 * makes a local variable into a global variable
 */
void process_global(struct script_data *sc, trig_data *trig, char *cmd, long id) {
    struct trig_var_data *vd;
    char arg[MAX_INPUT_LENGTH], *var;

    var = any_one_arg(cmd, arg);

    skip_spaces(&var);

    if (!*var) {
        script_log("Trigger: %s, VNum %d. global w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    for (vd = GET_TRIG_VARS(trig); vd; vd = vd->next)
        if (!strcasecmp(vd->name, var))
            break;

    if (!vd) {
        script_log("Trigger: %s, VNum %d. local var '%s' not found in global call",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), var);
        return;
    }

    add_var(&(sc->global_vars), vd->name, vd->value, id);
    remove_var(&GET_TRIG_VARS(trig), vd->name);
}


/* set the current context for a script */
void process_context(struct script_data *sc, trig_data *trig, char *cmd) {
    char arg[MAX_INPUT_LENGTH], *var;

    var = any_one_arg(cmd, arg);

    skip_spaces(&var);

    if (!*var) {
        script_log("Trigger: %s, VNum %d. context w/o an arg: '%s'",
                   GET_TRIG_NAME(trig), GET_TRIG_VNUM(trig), cmd);
        return;
    }

    sc->context = atol(var);
}

void extract_value(struct script_data *sc, trig_data *trig, char *cmd) {
    char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
    char *buf3;
    char to[128];
    int num;

    buf3 = any_one_arg(cmd, buf);
    half_chop(buf3, buf2, buf);
    strcpy(to, buf2);

    num = atoi(buf);
    if (num < 1) {
        script_log("extract number < 1!");
        return;
    }

    half_chop(buf, buf3, buf2);

    while (num > 0) {
        half_chop(buf2, buf, buf2);
        num--;
    }

    add_var(&GET_TRIG_VARS(trig), to, buf, sc ? sc->context : 0);
}

/*
  Thanks to Jamie Nelson for 4 dimensions for this addition

  Syntax :
    dg_letter <new varname> <letter position> <string to get from>

    ie:
    set string L337-String
    dg_letter var1 4 %string%
    dg_letter var2 11 %string%

    now %var1% == 7 and %var2% == g

    Note that the index starts at 1.

*/

void dg_letter_value(struct script_data *sc, trig_data *trig, char *cmd) {
    //set the letter/number at position 'num' as the variable.
    char junk[MAX_INPUT_LENGTH];
    char varname[MAX_INPUT_LENGTH];
    char num_s[MAX_INPUT_LENGTH];
    char string[MAX_INPUT_LENGTH];
    int num;

    half_chop(cmd, junk, cmd);   /* "dg_letter" */
    half_chop(cmd, varname, cmd);
    half_chop(cmd, num_s, string);

    num = atoi(num_s);

    script_log("The use of dg_letter is deprecated");
    script_log("- Use 'set <new variable> %%<text/var>.charat(index)%%' instead.");


    if (num < 1) {
        script_log("Trigger #%d : dg_letter number < 1!", GET_TRIG_VNUM(trig));
        return;
    }

    if (num > strlen(string)) {
        script_log("Trigger #%d : dg_letter number > strlen!", GET_TRIG_VNUM(trig));
        return;
    }

    *junk = string[num - 1];
    *(junk + 1) = '\0';
    add_var(&GET_TRIG_VARS(trig), varname, junk, sc->context);
}

/*  This is the core driver for scripts. */
/*  Arguments:
    void *go_adress
      A pointer to a pointer to the entity running the script.
      The reason for this approcah is that we want to be able to see
      from the calling function, if the entity has been free'd.

    trig_data *trig
      A pointer to the current running trigger.

    int type
      MOB_TRIGGER, OBJ_TRIGGER or WLD_TRIGGER, respectively.

    int mode
      TRIG_NEW     just started from dg_triggers.c
      TRIG_RESTART restarted after a 'wait'
*/



static int true_script_driver(void *go_adress, trig_data *trig, int type, int mode) {
    static int depth = 0;
    int ret_val = 1;
    struct cmdlist_element *cl;
    char cmd[MAX_INPUT_LENGTH], *p;
    struct script_data *sc = nullptr;
    struct cmdlist_element *temp;
    unsigned long loops = 0;
    void *go = nullptr;

    void obj_command_interpreter(obj_data *obj, char *argument);
    void wld_command_interpreter(struct room_data *room, char *argument);

    switch (type) {
        case MOB_TRIGGER:
            go = *(char_data **) go_adress;
            sc = SCRIPT((char_data *) go);
            break;
        case OBJ_TRIGGER:
            go = *(obj_data **) go_adress;
            sc = SCRIPT((obj_data *) go);
            break;
        case WLD_TRIGGER:
            go = *(room_data **) go_adress;
            sc = SCRIPT((room_data *) go);
            break;
    }

    if (depth > MAX_SCRIPT_DEPTH) {
        script_log("Trigger %d recursed beyond maximum allowed depth.", GET_TRIG_VNUM(trig));
        switch (type) {
            case MOB_TRIGGER:
                script_log("It was attached to %s [%d]",
                           GET_NAME((char_data *) go), GET_MOB_VNUM((char_data *) go));
                break;
            case OBJ_TRIGGER:
                script_log("It was attached to %s [%d]",
                           ((obj_data *) go)->short_description, GET_OBJ_VNUM((obj_data *) go));
                break;
            case WLD_TRIGGER:
                script_log("It was attached to %s [%d]",
                           ((room_data *) go)->name, ((room_data *) go)->vn);
                break;
        }

        extract_script(go, type);

        /*
           extract_script() works on rooms, but on mobiles and objects,
           it will be called again if the
           caller is load_mtrigger or load_otrigger
           if it is one of these, we must make sure the script
           is not just reloaded on the next mob

           We make the calling code decide how to handle it, so it doesn't
           get totally removed unless it's a load_xtrigger().
         */

        return SCRIPT_ERROR_CODE;
    }

    depth++;

    if (mode == TRIG_NEW) {
        GET_TRIG_DEPTH(trig) = 1;
        GET_TRIG_LOOPS(trig) = 0;
        sc->context = 0;
    }

    dg_owner_purged = 0;

    for (cl = (mode == TRIG_NEW) ? trig->cmdlist : trig->curr_state;
         cl && GET_TRIG_DEPTH(trig); cl = cl->next) {
        for (p = cl->cmd; *p && isspace(*p); p++);

        if (*p == '*') /* comment */
            continue;

        else if (!strncasecmp(p, "if ", 3)) {
            if (process_if(p + 3, go, sc, trig, type))
                GET_TRIG_DEPTH(trig)++;
            else
                cl = find_else_end(trig, cl, go, sc, type);
        } else if (!strncasecmp("elseif ", p, 7) ||
                   !strncasecmp("else", p, 4)) {
            /*
             * if not in an if-block, ignore the extra 'else[if]' and warn about it
             */
            if (GET_TRIG_DEPTH(trig) == 1) {
                script_log("Trigger VNum %lld has 'else' without 'if'.",
                           GET_TRIG_VNUM(trig));
                continue;
            }
            cl = find_end(trig, cl);
            GET_TRIG_DEPTH(trig)--;
        } else if (!strncasecmp("while ", p, 6)) {
            temp = find_done(cl);
            if (!temp) {
                script_log("Trigger VNum %lld has 'while' without 'done'.",
                           GET_TRIG_VNUM(trig));
                return ret_val;
            }
            if (process_if(p + 6, go, sc, trig, type)) {
                temp->original = cl;
            } else {
                cl = temp;
                loops = 0;
            }
        } else if (!strncasecmp("switch ", p, 7)) {
            cl = find_case(trig, cl, go, sc, type, p + 7);
        } else if (!strncasecmp("end", p, 3)) {
            /*
             * if not in an if-block, ignore the extra 'end' and warn about it.
             */
            if (GET_TRIG_DEPTH(trig) == 1) {
                script_log("Trigger VNum %lld has 'end' without 'if'.",
                           GET_TRIG_VNUM(trig));
                continue;
            }
            GET_TRIG_DEPTH(trig)--;
        } else if (!strncasecmp("done", p, 4)) {
            /* if in a while loop, cl->original is non-nullptr */
            if (cl->original) {
                char *orig_cmd = cl->original->cmd;
                while (*orig_cmd && isspace(*orig_cmd)) orig_cmd++;
                if (cl->original && process_if(orig_cmd + 6, go, sc, trig,
                                               type)) {
                    cl = cl->original;
                    loops++;
                    GET_TRIG_LOOPS(trig)++;
                    if (loops == 40) {
                        process_wait(go, trig, type, "wait 1", cl);
                        depth--;
                        return ret_val;
                    }
                    if (GET_TRIG_LOOPS(trig) >= 5000) {
                        script_log("Trigger VNum %d has looped 5,000 times!!!",
                                   GET_TRIG_VNUM(trig));
                        break;
                    }
                } else {
                    /* if we're falling through a switch statement, this ends it. */
                }
            }
        } else if (!strncasecmp("break", p, 5)) {
            cl = find_done(cl);
        } else if (!strncasecmp("case", p, 4)) {
            /* Do nothing, this allows multiple cases to a single instance */
        } else {

            var_subst(go, sc, trig, type, p, cmd);

            if (!strncasecmp(cmd, "eval ", 5))
                process_eval(go, sc, trig, type, cmd);

            else if (!strncasecmp(cmd, "nop ", 4)); /* nop: do nothing */

            else if (!strncasecmp(cmd, "extract ", 8))
                extract_value(sc, trig, cmd);

            else if (!strncasecmp(cmd, "dg_letter ", 10))
                dg_letter_value(sc, trig, cmd);

            else if (!strncasecmp(cmd, "halt", 4))
                break;

            else if (!strncasecmp(cmd, "dg_cast ", 8))
                do_dg_cast(go, sc, trig, type, cmd);

            else if (!strncasecmp(cmd, "dg_affect ", 10))
                do_dg_affect(go, sc, trig, type, cmd);

            else if (!strncasecmp(cmd, "global ", 7))
                process_global(sc, trig, cmd, sc->context);

            else if (!strncasecmp(cmd, "context ", 8))
                process_context(sc, trig, cmd);

            else if (!strncasecmp(cmd, "remote ", 7))
                process_remote(sc, trig, cmd);

            else if (!strncasecmp(cmd, "rdelete ", 8))
                process_rdelete(sc, trig, cmd);

            else if (!strncasecmp(cmd, "return ", 7))
                ret_val = process_return(trig, cmd);

            else if (!strncasecmp(cmd, "set ", 4))
                process_set(sc, trig, cmd);

            else if (!strncasecmp(cmd, "unset ", 6))
                process_unset(sc, trig, cmd);

            else if (!strncasecmp(cmd, "wait ", 5)) {
                process_wait(go, trig, type, cmd, cl);
                depth--;
                return ret_val;
            } else if (!strncasecmp(cmd, "attach ", 7))
                process_attach(go, sc, trig, type, cmd);

            else if (!strncasecmp(cmd, "detach ", 7))
                process_detach(go, sc, trig, type, cmd);

            else if (!strncasecmp(cmd, "version", 7))
                mudlog(NRM, ADMLVL_GOD, true, "%s", DG_SCRIPT_VERSION);

            else {
                switch (type) {
                    case MOB_TRIGGER:
                        command_interpreter((char_data *) go, cmd);
                        break;
                    case OBJ_TRIGGER:
                        obj_command_interpreter((obj_data *) go, cmd);
                        break;
                    case WLD_TRIGGER:
                        wld_command_interpreter((struct room_data *) go, cmd);
                        break;
                }
                if (dg_owner_purged) {
                    depth--;
                    if (type == OBJ_TRIGGER)
                        *(obj_data **) go_adress = nullptr;
                    return ret_val;
                }
            }

        }
    }

    switch (type) { /* the script may have been detached */
        case MOB_TRIGGER:
            sc = SCRIPT((char_data *) go);
            break;
        case OBJ_TRIGGER:
            sc = SCRIPT((obj_data *) go);
            break;
        case WLD_TRIGGER:
            sc = SCRIPT((room_data *) go);
            break;
    }
    if (sc)
        free_varlist(GET_TRIG_VARS(trig));
    GET_TRIG_VARS(trig) = nullptr;
    GET_TRIG_DEPTH(trig) = 0;

    depth--;
    return ret_val;
}

int script_driver(void *go_adress, trig_data *trig, int type, int mode) {
    auto result = true_script_driver(go_adress, trig, type, mode);
    dirty_dgscripts.insert(trig->id);
    return result;
}

/* returns the real number of the trigger with given virtual number */
trig_rnum real_trigger(trig_vnum vnum) {
    return trig_index.count(vnum) ? vnum : NOTHING;
}

ACMD(do_tstat) {
    int rnum;
    char str[MAX_INPUT_LENGTH];

    half_chop(argument, str, argument);
    if (*str) {
        rnum = real_trigger(atoi(str));
        if (rnum == NOTHING) {
            send_to_char(ch, "That vnum does not exist.\r\n");
            return;
        }

        do_stat_trigger(ch, trig_index[rnum].proto);
    } else
        send_to_char(ch, "Usage: tstat <vnum>\r\n");
}

/*
* scans for a case/default instance
* returns the line containg the correct case instance, or the last
* line of the trigger if not found.
*/
struct cmdlist_element *
find_case(struct trig_data *trig, struct cmdlist_element *cl,
          void *go, struct script_data *sc, int type, char *cond) {
    char result[MAX_INPUT_LENGTH];
    struct cmdlist_element *c;
    char *p, *buf;

    eval_expr(cond, result, go, sc, trig, type);

    if (!(cl->next))
        return cl;

    for (c = cl->next; c->next; c = c->next) {
        for (p = c->cmd; *p && isspace(*p); p++);

        if (!strncasecmp("while ", p, 6) || !strncasecmp("switch", p, 6))
            c = find_done(c);
        else if (!strncasecmp("case ", p, 5)) {
            buf = (char *) malloc(MAX_STRING_LENGTH);
            eval_op("==", result, p + 5, buf, go, sc, trig);
            if (*buf && *buf != '0') {
                free(buf);
                return c;
            }
            free(buf);
        } else if (!strncasecmp("default", p, 7))
            return c;
        else if (!strncasecmp("done", p, 3))
            return c;
    }
    return c;
}

/*
* scans for end of while/switch-blocks.
* returns the line containg 'end', or the last
* line of the trigger if not found.
* Malformed scripts may cause nullptr to be returned.
*/
struct cmdlist_element *find_done(struct cmdlist_element *cl) {
    struct cmdlist_element *c;
    char *p;

    if (!cl || !(cl->next))
        return cl;

    for (c = cl->next; c && c->next; c = c->next) {
        for (p = c->cmd; *p && isspace(*p); p++);

        if (!strncasecmp("while ", p, 6) || !strncasecmp("switch ", p, 7))
            c = find_done(c);
        else if (!strncasecmp("done", p, 3))
            return c;
    }

    return c;
}


/* read a line in from a file, return the number of chars read */
int fgetline(FILE *file, char *p) {
    int count = 0;

    do {
        *p = fgetc(file);
        if (*p != '\n' && !feof(file)) {
            p++;
            count++;
        }
    } while (*p != '\n' && !feof(file));

    if (*p == '\n') *p = '\0';

    return count;
}


/* load in a character's saved variables */
void read_saved_vars(struct char_data *ch) {
    FILE *file;
    long context;
    char fn[127];
    char input_line[1024], *temp, *p;
    char varname[32];
    char context_str[16];

    /*
     * If getting to the menu from inside the game, the vars aren't removed.
     * So let's not allocate them again.
     */
    if (SCRIPT(ch))
        return;

    /* create the space for the script structure which holds the vars */
    /* We need to do this first, because later calls to 'remote' will need */
    /* a script already assigned. */
    ch->script = new script_data(ch);

    /* find the file that holds the saved variables and open it*/
    get_filename(fn, sizeof(fn), SCRIPT_VARS_FILE, GET_NAME(ch));
    file = fopen(fn, "r");

    /* if we failed to open the file, return */
    if (!file) {
        basic_mud_log("%s had no variable file", GET_NAME(ch));
        return;
    }
    /* walk through each line in the file parsing variables */
    do {
        if (get_line(file, input_line) > 0) {
            p = temp = strdup(input_line);
            temp = any_one_arg(temp, varname);
            temp = any_one_arg(temp, context_str);
            skip_spaces(&temp); /* temp now points to the rest of the line */

            context = atol(context_str);
            add_var(&(SCRIPT(ch)->global_vars), varname, temp, context);
            free(p); /* plug memory hole */
        }
    } while (!feof(file));

    /* close the file and return */
    fclose(file);
}

/* save a characters variables out to disk */
void save_char_vars(struct char_data *ch) {
    FILE *file;
    char fn[127];
    struct trig_var_data *vars;

    /* immediate return if no script (and therefore no variables) structure */
    /* has been created. this will happen when the player is logging in */
    if (SCRIPT(ch) == nullptr) return;

    /* we should never be called for an NPC, but just in case... */
    if (IS_NPC(ch)) return;

    get_filename(fn, sizeof(fn), SCRIPT_VARS_FILE, GET_NAME(ch));
    std::filesystem::remove(fn);

    /* make sure this char has global variables to save */
    if (ch->script->global_vars == nullptr) return;
    vars = ch->script->global_vars;

    file = fopen(fn, "wt");
    if (!file) {
        mudlog(NRM, ADMLVL_GOD, true,
               "SYSERR: Could not open player variable file %s for writing.:%s",
               fn, strerror(errno));
        return;
    }
    /* note that currently, context will always be zero. this may change */
    /* in the future */
    while (vars) {
        if (*vars->name != '-') /* don't save if it begins with - */
            fprintf(file, "%s %ld %s\n", vars->name, vars->context, vars->value);
        vars = vars->next;
    }

    fclose(file);
}

/* load in a character's saved variables from an ASCII pfile*/
void read_saved_vars_ascii(FILE *file, struct char_data *ch, int count) {
    long context;
    char input_line[1024], *temp, *p;
    char varname[READ_SIZE];
    char context_str[READ_SIZE];
    int i;

    /*
     * If getting to the menu from inside the game, the vars aren't removed.
     * So let's not allocate them again.
     */
    if (SCRIPT(ch))
        return;

    /* create the space for the script structure which holds the vars */
    /* We need to do this first, because later calls to 'remote' will need */
    /* a script already assigned. */
    ch->script = new script_data(ch);

    /* walk through each line in the file parsing variables */
    for (i = 0; i < count; i++) {
        if (get_line(file, input_line) > 0) {
            p = temp = strdup(input_line);
            temp = any_one_arg(temp, varname);
            temp = any_one_arg(temp, context_str);
            skip_spaces(&temp); /* temp now points to the rest of the line */

            context = atol(context_str);
            add_var(&(SCRIPT(ch)->global_vars), varname, temp, context);
            free(p); /* plug memory hole */
        }
    }
}

/* save a characters variables out to an ASCII pfile */
void save_char_vars_ascii(FILE *file, struct char_data *ch) {
    struct trig_var_data *vars;
    int count = 0;
    /* immediate return if no script (and therefore no variables) structure */
    /* has been created. this will happen when the player is logging in */
    if (SCRIPT(ch) == nullptr) return;

    /* we should never be called for an NPC, but just in case... */
    if (IS_NPC(ch)) return;

    /* make sure this char has global variables to save */
    if (ch->script->global_vars == nullptr) return;

    /* note that currently, context will always be zero. this may change */
    /* in the future */
    for (vars = ch->script->global_vars; vars; vars = vars->next)
        if (*vars->name != '-')
            count++;

    if (count != 0) {
        fprintf(file, "Vars: %d\n", count);

        for (vars = ch->script->global_vars; vars; vars = vars->next)
            if (*vars->name != '-') /* don't save if it begins with - */
                fprintf(file, "%s %ld %s\n", vars->name, vars->context, vars->value);
    }
}

/* find_char() helpers */

// Must be power of 2
#define BUCKET_COUNT 64
// to recognize an empty bucket
#define UID_OUT_OF_RANGE 1000000000


int check_flags_by_name_ar(bitvector_t *array, int numflags, char *search, const char *namelist[]) {
    int i, item = -1;

    for (i = 0; i < numflags && item < 0; i++)
        if (!strcmp(search, namelist[i]))
            item = i;

    if (item < 0)
        return false;

    if (IS_SET_AR(array, item))
        return item;

    return false;
}

nlohmann::json trig_var_data::serialize() {
    nlohmann::json j;

    if(name && strlen(name)) j["name"] = name;
    if(value && strlen(value)) j["value"] = value;
    if(context) j["context"] = context;

    return j;
}

trig_var_data::trig_var_data(const nlohmann::json& j) : trig_var_data() {
    if(j.contains("name")) name = strdup(j["name"].get<std::string>().c_str());
    if(j.contains("value")) value = strdup(j["value"].get<std::string>().c_str());
    if(j.contains("context")) context = j["context"].get<long>();
}

nlohmann::json trig_data::serializeProto() {
    auto j = nlohmann::json::object();

    if(vn != NOTHING) j["vn"] = vn;
    if(name && strlen(name)) j["name"] = name;
    if(attach_type) j["attach_type"] = attach_type;
    if(data_type) j["data_type"] = data_type;
    if(trigger_type) j["trigger_type"] = trigger_type;
    if(narg) j["narg"] = narg;
    if(arglist && strlen(arglist)) j["arglist"] = arglist;

    for(auto c = cmdlist; c; c = c->next) {
        j["cmdlist"].push_back(c->cmd);
    }

    return j;
}

int trig_data::countLine(struct cmdlist_element *c) {
    int count = 0;
    for(auto cl = cmdlist; cl; cl = cl->next) {
        if(cl == c) return count;
        count++;
    }
    // This should never, EVER happen...
    return -1;
}

nlohmann::json trig_data::serializeInstance() {
    auto j = nlohmann::json::object();

    j["vn"] = vn;

    j["id"] = id;
    j["generation"] = generation;
    j["order"] = order;

    if(depth) j["depth"] = depth;
    if(loops) j["loops"] = loops;
    if(waiting != 0.0) j["waiting"] = waiting;

    if(!(curr_state == cmdlist || !curr_state)) {
        j["curr_state"] = countLine(curr_state);
        if(curr_state->original) j["curr_state_original"] = countLine(curr_state->original);
    }

    if(var_list) j["var_list"] = serializeVars(var_list);

    return j;
}

void trig_data::deserializeInstance(const nlohmann::json &j) {
    if(j.contains("vn")) vn = j["vn"].get<int>();
    auto &t = trig_index[vn];
    auto p = t.proto;
    if(p->name && strlen(p->name)) name = strdup(p->name);
    attach_type = p->attach_type;
    data_type = p->data_type;
    trigger_type = p->trigger_type;
    narg = p->narg;
    if(p->arglist && strlen(p->arglist)) arglist = strdup(p->arglist);

    cmdlist = p->cmdlist;
    curr_state = p->cmdlist;

    if(j.contains("id")) id = j["id"].get<long>();
    if(j.contains("generation")) generation = j["generation"].get<long>();
    if(j.contains("order")) order = j["order"].get<long>();

    if(j.contains("waiting")) waiting = j["waiting"].get<double>();
    if(j.contains("depth")) depth = j["depth"].get<int>();
    if(j.contains("loops")) loops = j["loops"].get<int>();

    if(j.contains("curr_state")) {
        int curr_state_num = j["curr_state"].get<int>();
        if(curr_state_num > 0) {
            for(int i = 0; i < curr_state_num; i++) {
                curr_state = curr_state->next;
            }
        }
    }

    if(j.contains("curr_state_original")) {
        int curr_state_num = j["curr_state_original"].get<int>();
        curr_state->original = cmdlist;
        if(curr_state_num > 0) {
            for(int i = 0; i < curr_state_num; i++) {
                curr_state->original = curr_state->original->next;
            }
        }
    }

    if(j.contains("var_list")) {
        deserializeVars(&var_list, j["var_list"]);
    }
}

std::string trig_data::serializeLocation() {
    switch(owner.index()) {
        case 0:
            return std::get<0>(owner)->getUID();
            break;
        case 1:
            return std::get<1>(owner)->getUID();
            break;
        case 2:
            return std::get<2>(owner)->getUID();
            break;
    }
}

void trig_data::deserializeLocation(const std::string &txt) {
    auto uid = resolveUID(txt);
    if(!uid) return;
    owner = *uid;
    struct room_data *r;
    struct obj_data *o;
    struct char_data *c;
    switch(owner.index()) {
        case 0:
            r = std::get<0>(owner);
            if(!SCRIPT(r)) r->script = new script_data(r);
            break;
        case 1:
            o = std::get<1>(owner);
            if(!SCRIPT(o)) o->script = new script_data(o);
            break;
        case 2:
            c = std::get<2>(owner);
            if(!SCRIPT(c)) c->script = new script_data(c);
            break;
    }
}

trig_data::trig_data(const nlohmann::json &j) : trig_data() {
    if(j.contains("vn")) vn = j["vn"].get<int>();
    if(j.contains("name")) name = strdup(j["name"].get<std::string>().c_str());
    if(j.contains("attach_type")) attach_type = j["attach_type"].get<int>();
    if(j.contains("data_type")) data_type = j["data_type"].get<int>();
    if(j.contains("trigger_type")) trigger_type = j["trigger_type"].get<int>();
    if(j.contains("narg")) narg = j["narg"].get<int>();
    if(j.contains("arglist")) arglist = strdup(j["arglist"].get<std::string>().c_str());

    if(j.contains("cmdlist")) {
        auto &cl = j["cmdlist"];
        for(auto c = cl.rbegin(); c != cl.rend(); c++) {
            auto cle = new cmdlist_element();
            cle->cmd = strdup(c->get<std::string>().c_str());
            cle->next = cmdlist;
            cmdlist = cle;
        }
    }
}

void ADD_UID_VAR(char *buf, struct trig_data *trig, struct unit_data *thing, char *name, long context) {
	auto uid = thing->getUID(false);
    add_var(&GET_TRIG_VARS(trig), name, (char*)uid.c_str(), context);
}

// Note: Trigger instances are meant to be set all active or inactive on a per room/character/item basis,
// not individually.
void trig_data::activate() {
    if(active) {
        basic_mud_log("SYSERR: Attempt to activate already-active trigger %ld", id);
        return;
    }
    active = true;
    next_in_world = trigger_list;
    trigger_list = this;
    if(waiting != 0.0) triggers_waiting.insert(this);
}

void trig_data::deactivate() {
    if(!active) {
        basic_mud_log("SYSERR: Attempt to deactivate already-inactive trigger %ld", id);
        return;
    }
    active = false;
    struct trig_data *temp;
    triggers_waiting.erase(this);
    REMOVE_FROM_LIST(this, trigger_list, next_in_world, temp);
}

nlohmann::json serializeVars(struct trig_var_data *vd) {
    auto j = nlohmann::json::array();;
    for(auto v = vd; v; v = v->next) {
        j.push_back(v->serialize());
    }
    return j;
}

void deserializeVars(struct trig_var_data **vd, const nlohmann::json &j) {
    for(auto it = j.rbegin(); it != j.rend(); ++it) {
        auto v = new trig_var_data(*it);
        v->next = *vd;
        *vd = v;
    }
}

void script_data::activate() {
    for(auto t = trig_list; t; t = t->next) {
        t->activate();
    }
}

void script_data::deactivate() {
    for(auto t = trig_list; t; t = t->next) {
        t->deactivate();
    }
}