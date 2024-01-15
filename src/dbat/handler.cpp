/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "dbat/handler.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/interpreter.h"
#include "dbat/spells.h"
#include "dbat/dg_scripts.h"
#include "dbat/feats.h"
#include "dbat/races.h"
#include "dbat/class.h"
#include "dbat/objsave.h"
#include "dbat/fight.h"
#include "dbat/races.h"
#include "dbat/act.informative.h"
#include "dbat/act.misc.h"
#include "dbat/act.movement.h"
#include "dbat/players.h"

/* local vars */
static std::set<struct char_data*> extractions_pending;

/* external vars */

/* local functions */
static int apply_ac(struct char_data *ch, int eq_pos);

static void update_object(struct obj_data *obj, int use);


/* external functions */
struct obj_data *find_vehicle_by_vnum(int vnum);

void remove_follower(struct char_data *ch);

void clearMemory(struct char_data *ch);

ACMD(do_return);

void perform_wear(struct char_data *ch, struct obj_data *obj, int where);

void perform_remove(struct char_data *ch, int pos);

int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);

SPECIAL(shop_keeper);

/* For Getting An Intro Name */
const char *get_i_name(struct char_data *ch, struct char_data *vict) {
    static char name[50];

    /* Read Introduction File */
    if (!vict || vict == ch) {
        return ("");
    }

    if (IS_NPC(ch) || IS_NPC(vict)) {
        return (RACE(vict));
    }

    auto &p = players[ch->id];

    auto found = p.dubNames.find(vict->id);
    if(found == p.dubNames.end()) return RACE(vict);

    // print *found to name and return buf pointer.
    sprintf(name, "%s", found->second.c_str());
    return (name);
}

char *fname(const char *namelist) {
    static char holder[READ_SIZE];
    char *point;

    for (point = holder; isalpha(*namelist); namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return (holder);
}

/* Stock isname().  Leave this here even if you put in a newer  *
 * isname().  Used for OasisOLC.                                */
int is_name(const char *str, const char *namelist) {
    const char *curname, *curstr;

    if (!*str || !*namelist || !str || !namelist)
        return (0);

    curname = namelist;
    for (;;) {
        for (curstr = str;; curstr++, curname++) {
            if (!*curstr && !isalpha(*curname))
                return (1);

            if (!*curname)
                return (0);

            if (!*curstr || *curname == ' ')
                break;

            if (LOWER(*curstr) != LOWER(*curname))
                break;
        }

        /* skip to next name */
        for (; isalpha(*curname); curname++);
        if (!*curname)
            return (0);
        curname++;                  /* first char of new name */
    }
}

/* allow abbreviations */
#define WHITESPACE " \t"


static char* mystrsep(char** stringp, const char* delim)
{
    char* start = *stringp;
    char* p;

    p = (start != NULL) ? strpbrk(start, delim) : NULL;

    if (p == NULL)
    {
        *stringp = NULL;
    }
    else
    {
        *p = '\0';
        *stringp = p + 1;
    }

    return start;
}

int isname(const char *str, const char *namelist) {
    char *newlist;
    char *curtok;
    static char newlistbuf[MAX_STRING_LENGTH];

    if (!str || !*str || !namelist || !*namelist) {
        return 0;
    }

    if (!strcasecmp(str, namelist)) { /* the easy way */
        return 1;
    }

    strlcpy(newlistbuf, namelist, sizeof(newlistbuf));
    newlist = newlistbuf;
    for (curtok = mystrsep(&newlist, WHITESPACE); curtok; curtok = mystrsep(&newlist, WHITESPACE)) {
        if (curtok && is_abbrev(str, curtok)) {
            /* Don't allow abbreviated numbers, only alpha names need abbreviation */
            /* This, I just consider a bug fix, because abbreviating numbers is just*/
            /* asking for trouble. IE: 100 would return true on 1000 --Sryth*/
            if (isdigit(*str) && (atoi(str) != atoi(curtok))) {
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

void aff_apply_modify(struct char_data *ch, int loc, int mod, int spec, char *msg) {

}


void affect_modify(struct char_data *ch, int loc, int mod, int spec, long bitv, bool add) {
    if (add) {
        if (bitv != AFF_INFRAVISION || !IS_ANDROID(ch)) {
            ch->affected_by.set(bitv);
        }
    } else {
        if (bitv != AFF_INFRAVISION || !IS_ANDROID(ch)) {
            ch->affected_by.reset(bitv);
            mod = -mod;
        }
    }

    aff_apply_modify(ch, loc, mod, spec, "affect_modify");
}


void affect_modify_ar(struct char_data *ch, int loc, int mod, int spec, const std::bitset<NUM_AFF_FLAGS>& bitv, bool add) {
    int i, j;

    if (add) for (i = 0; i < bitv.size(); i++) if(bitv.test(i)) ch->affected_by.set(i);
    else {
        for (i = 0; i < bitv.size(); i++) if(bitv.test(i)) ch->affected_by.reset(i);
        mod = -mod;
    }

    aff_apply_modify(ch, loc, mod, spec, "affect_modify_ar");
}


/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch) {
    struct affected_type *af;
    int i, j;

    GET_SPELLFAIL(ch) = GET_ARMORCHECK(ch) = GET_ARMORCHECKALL(ch) = 0;

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i))
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                affect_modify_ar(ch, GET_EQ(ch, i)->affected[j].location,
                                 GET_EQ(ch, i)->affected[j].modifier,
                                 GET_EQ(ch, i)->affected[j].specific,
                                 GET_OBJ_PERM(GET_EQ(ch, i)), false);
    }


    for (af = ch->affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->specific, af->bitvector, false);


    GET_SAVE_MOD(ch, SAVING_FORTITUDE) = HAS_FEAT(ch, FEAT_GREAT_FORTITUDE) * 3;
    GET_SAVE_MOD(ch, SAVING_REFLEX) = HAS_FEAT(ch, FEAT_LIGHTNING_REFLEXES) * 3;
    GET_SAVE_MOD(ch, SAVING_WILL) = HAS_FEAT(ch, FEAT_IRON_WILL) * 3;

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i)) {
            if (GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_ARMOR) {
                GET_SPELLFAIL(ch) += GET_OBJ_VAL(GET_EQ(ch, i), VAL_ARMOR_SPELLFAIL);
                GET_ARMORCHECKALL(ch) += GET_OBJ_VAL(GET_EQ(ch, i), VAL_ARMOR_CHECK);
                if (!is_proficient_with_armor(ch, GET_OBJ_VAL(GET_EQ(ch, i), VAL_ARMOR_SKILL)))
                    GET_ARMORCHECK(ch) += GET_OBJ_VAL(GET_EQ(ch, i), VAL_ARMOR_CHECK);
            }
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                affect_modify_ar(ch, GET_EQ(ch, i)->affected[j].location,
                                 GET_EQ(ch, i)->affected[j].modifier,
                                 GET_EQ(ch, i)->affected[j].specific,
                                 GET_OBJ_PERM(GET_EQ(ch, i)), true);
        }
    }


    for (af = ch->affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->specific, af->bitvector, true);


}


/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data *ch, struct affected_type *af) {
    struct affected_type *affected_alloc;

    CREATE(affected_alloc, struct affected_type, 1);

    if (!ch->affected) {
        ch->next_affect = affect_list;
        affect_list = ch;
    }
    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier, af->specific, af->bitvector, true);
    affect_total(ch);
}


/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void affect_remove(struct char_data *ch, struct affected_type *af) {
    struct affected_type *cmtemp;

    if (ch->affected == nullptr) {
        core_dump();
        return;
    }

    affect_modify(ch, af->location, af->modifier, af->specific, af->bitvector, false);
    REMOVE_FROM_LIST(af, ch->affected, next, cmtemp);
    free(af);
    affect_total(ch);
    if (!ch->affected) {
        struct char_data *temp;
        REMOVE_FROM_LIST(ch, affect_list, next_affect, temp);
        ch->next_affect = nullptr;
    }
}


/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data *ch, int type) {
    struct affected_type *hjp, *next;

    for (hjp = ch->affected; hjp; hjp = next) {
        next = hjp->next;
        if (hjp->type == type)
            affect_remove(ch, hjp);
    }
}


/* Call affect_remove with every spell of spelltype "skill" */
void affectv_from_char(struct char_data *ch, int type) {
    struct affected_type *hjp, *next;

    for (hjp = ch->affectedv; hjp; hjp = next) {
        next = hjp->next;
        if (hjp->type == type)
            affectv_remove(ch, hjp);
    }
}


/*
 * Return TRUE if a char is affected by a spell (SPELL_XXX),
 * FALSE indicates not affected.
 */
bool affected_by_spell(struct char_data *ch, int type) {
    struct affected_type *hjp;

    for (hjp = ch->affected; hjp; hjp = hjp->next)
        if (hjp->type == type)
            return (true);

    return (false);
}


/*
 * Return TRUE if a char is affected by a spell (SPELL_XXX),
 * FALSE indicates not affected.
 */
bool affectedv_by_spell(struct char_data *ch, int type) {
    struct affected_type *hjp;

    for (hjp = ch->affectedv; hjp; hjp = hjp->next)
        if (hjp->type == type)
            return (true);

    return (false);
}


void affect_join(struct char_data *ch, struct affected_type *af,
                 bool add_dur, bool avg_dur, bool add_mod, bool avg_mod) {
    struct affected_type *hjp, *next;
    bool found = false;

    for (hjp = ch->affected; !found && hjp; hjp = next) {
        next = hjp->next;

        if ((hjp->type == af->type) && (hjp->location == af->location)) {
            if (add_dur)
                af->duration += hjp->duration;
            if (avg_dur)
                af->duration /= 2;

            if (add_mod)
                af->modifier += hjp->modifier;
            if (avg_mod)
                af->modifier /= 2;

            affect_remove(ch, hjp);
            affect_to_char(ch, af);
            found = true;
        }
    }
    if (!found)
        affect_to_char(ch, af);
}


/* move a player out of a room */
void char_from_room(struct char_data *ch) {
    struct char_data *temp;
    int i;
    struct room_data *r;

    if (ch == nullptr || !(r = ch->getRoom())) {
        basic_mud_log("SYSERR: nullptr character or NOWHERE in %s, char_from_room", __FILE__);
        return;
    }

    if (FIGHTING(ch) != nullptr && !AFF_FLAGGED(ch, AFF_PURSUIT))
        stop_fighting(ch);
    if (AFF_FLAGGED(ch, AFF_PURSUIT) && FIGHTING(ch) == nullptr)
        ch->affected_by.reset(AFF_PURSUIT);

    REMOVE_FROM_LIST(ch, r->people, next_in_room, temp);
    IN_ROOM(ch) = NOWHERE;
    ch->next_in_room = nullptr;
    
    if(!gameIsLoading) ch->save();
}

/* place a character in a room */
void char_to_room(struct char_data *ch, struct room_data* room) {
    int i;

    ch->next_in_room = room->people;
    room->people = ch;
    IN_ROOM(ch) = room->vn;

    /* Stop fighting now, if we left. */
    if (FIGHTING(ch) && IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)) && !AFF_FLAGGED(ch, AFF_PURSUIT)) {
        stop_fighting(FIGHTING(ch));
        stop_fighting(ch);
    }
    if (!IS_NPC(ch)) {
        if (PRF_FLAGGED(ch, PRF_ARENAWATCH)) {
            ch->pref.reset(PRF_ARENAWATCH);
            ARENA_IDNUM(ch) = -1;
        }
    }

}

/* place a character in a room */
void char_to_room(struct char_data *ch, room_rnum room) {
    if(!ch) return;
    if(!world.count(room)) return;
    char_to_room(ch, &world[room]);
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch) {
    if (object && ch) {
        object->next_content = ch->contents;
        ch->contents = object;
        object->carried_by = ch;
        IN_ROOM(object) = NOWHERE;
        if ((GET_KAIOKEN(ch) <= 0 && !AFF_FLAGGED(ch, AFF_METAMORPH)) && !OBJ_FLAGGED(object, ITEM_THROW)) {

        } else if (GET_HIT(ch) > (ch->getEffMaxPL())) {
            if (GET_KAIOKEN(ch) > 0) {
                send_to_char(ch, "@RThe strain of the weight has reduced your kaioken somewhat!@n\n");
            } else if (AFF_FLAGGED(ch, AFF_METAMORPH)) {
                send_to_char(ch, "@RYour metamorphosis strains under the additional weight!@n\n");
            }
        }

        /* set flag for crash-save system, but not on mobs! */
        if (GET_OBJ_VAL(object, 0) != 0) {
            if (GET_OBJ_VNUM(object) == 16705 || GET_OBJ_VNUM(object) == 16706 || GET_OBJ_VNUM(object) == 16707) {
                object->level = GET_OBJ_VAL(object, 0);
            }
        }
        if (!IS_NPC(ch))
            ch->playerFlags.set(PLR_CRASH);
    } else
        basic_mud_log("SYSERR: nullptr obj (%p) or char (%p) passed to obj_to_char.", object, ch);
}


/* take an object from a char */
void obj_from_char(struct obj_data *object) {
    struct obj_data *temp;

    if (object == nullptr) {
        basic_mud_log("SYSERR: nullptr object passed to obj_from_char.");
        return;
    }
    REMOVE_FROM_LIST(object, object->carried_by->contents, next_content, temp);

    /* set flag for crash-save system, but not on mobs! */
    if (!IS_NPC(object->carried_by))
        object->carried_by->playerFlags.set(PLR_CRASH);

    int64_t previous = (object->carried_by->getEffMaxPL());

    if (GET_OBJ_VAL(object, 0) != 0) {
        if (GET_OBJ_VNUM(object) == 16705 || GET_OBJ_VNUM(object) == 16706 || GET_OBJ_VNUM(object) == 16707) {
            object->level = GET_OBJ_VAL(object, 0);
        }
    }

    object->carried_by = nullptr;
    object->next_content = nullptr;
}


/* Return the effect of a piece of armor in position eq_pos */
static int apply_ac(struct char_data *ch, int eq_pos) {
    if (GET_EQ(ch, eq_pos) == nullptr) {
        core_dump();
        return (0);
    }

    if (!(GET_OBJ_TYPE(GET_EQ(ch, eq_pos)) == ITEM_ARMOR))
        return (0);

    /* The following code is an example of how to make the WEAR_ position of the
     * armor apply MORE AC value based on 'factor' then it's assigned value.
     * IE: An object with an AC value of 5 and a factor of 3 really gives 15 AC not 5.

    int factor;

    switch (eq_pos) {

    case WEAR_BODY:
      factor = 3;
      break;
    case WEAR_HEAD:
    case WEAR_LEGS:
      factor = 1;
      break;
    default:
      factor = 1;
      break;
    }

    return (factor * GET_OBJ_VAL(GET_EQ(ch, eq_pos), VAL_ARMOR_APPLYAC)); */
    return (GET_OBJ_VAL(GET_EQ(ch, eq_pos), VAL_ARMOR_APPLYAC));
}

int invalid_align(struct char_data *ch, struct obj_data *obj) {
    if (OBJ_FLAGGED(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
        return true;
    if (OBJ_FLAGGED(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
        return true;
    if (OBJ_FLAGGED(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))
        return true;
    return false;
}

void equip_char(struct char_data *ch, struct obj_data *obj, int pos) {
    int j;

    if (pos < 0 || pos >= NUM_WEARS) {
        core_dump();
        return;
    }

    if (GET_EQ(ch, pos)) {
        basic_mud_log("SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
            obj->short_description);
        return;
    }
    if (obj->carried_by) {
        basic_mud_log("SYSERR: EQUIP: Obj is carried_by when equip.");
        return;
    }
    if (IN_ROOM(obj) != NOWHERE) {
        basic_mud_log("SYSERR: EQUIP: Obj is in_room when equip.");
        return;
    }
    if (invalid_align(ch, obj) || invalid_class(ch, obj) || invalid_race(ch, obj)) {
        act("You stop wearing $p as something prevents you.", false, ch, obj, nullptr, TO_CHAR);
        act("$n stops wearing $p as something prevents $m.", false, ch, obj, nullptr, TO_ROOM);
        /* Changed to drop in inventory instead of the ground. */
        obj_to_char(obj, ch);
        return;
    }

    GET_EQ(ch, pos) = obj;
    obj->worn_by = ch;
    obj->worn_on = pos;
}


struct obj_data *unequip_char(struct char_data *ch, int pos) {
    int j;
    struct obj_data *obj;

    if ((pos < 0 || pos >= NUM_WEARS) || GET_EQ(ch, pos) == nullptr) {
        core_dump();
        return (nullptr);
    }

    obj = GET_EQ(ch, pos);
    obj->worn_by = nullptr;
    obj->worn_on = -1;

    GET_EQ(ch, pos) = nullptr;

    return (obj);
}


int get_number(char **name) {
    int i;
    char *ppos;
    char number[MAX_INPUT_LENGTH];

    *number = '\0';

    if ((ppos = strchr(*name, '.')) != nullptr) {
        *ppos++ = '\0';
        strlcpy(number, *name, sizeof(number));
        strcpy(*name, ppos);    /* strcpy: OK (always smaller) */

        for (i = 0; *(number + i); i++)
            if (!isdigit(*(number + i)))
                return (0);

        return (atoi(number));
    }
    return (1);
}


/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list) {
    struct obj_data *i;

    for (i = list; i; i = i->next_content)
        if (GET_OBJ_RNUM(i) == num)
            return (i);

    return (nullptr);
}


/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(obj_rnum nr) {
    return get_last_inserted(objectVnumIndex, nr);
}


/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int *number, room_rnum room) {
    struct char_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    if (*number == 0)
        return (nullptr);

    for (i = world[room].people; i && *number; i = i->next_in_room)
        if (isname(name, i->name))
            if (--(*number) == 0)
                return (i);

    return (nullptr);
}


/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(mob_rnum nr) {
    struct char_data *i;

    for (i = character_list; i; i = i->next)
        if (GET_MOB_RNUM(i) == nr)
            return (i);

    return (nullptr);
}


void obj_to_room(struct obj_data *object, struct room_data *room) {
    struct obj_data *vehicle = nullptr;

    if (ROOM_FLAGGED(room, ROOM_GARDEN1) || ROOM_FLAGGED(room, ROOM_GARDEN2)) {
        if (GET_OBJ_TYPE(object) != ITEM_PLANT) {
            send_to_room(room,
                         "%s @wDisappears in a puff of smoke! It seems the room was designed to vaporize anything not plant related. Strange...@n\r\n",
                         object->short_description);
            extract_obj(object);
            return;
        }
    }
    if (room->vn == real_room(80)) {
        auc_load(object);
    }

    object->next_content = room->contents;
    room->contents = object;
    IN_ROOM(object) = room->vn;
    object->carried_by = nullptr;
    GET_LAST_LOAD(object) = time(nullptr);

    if (GET_OBJ_TYPE(object) == ITEM_VEHICLE && !OBJ_FLAGGED(object, ITEM_UNBREAKABLE) &&
        GET_OBJ_VNUM(object) > 19199) {
        object->extra_flags.set(ITEM_UNBREAKABLE);
    }

    // This section is now only going to be called during migrations.
    if(gameFunc) {
        if (GET_OBJ_TYPE(object) == ITEM_HATCH && GET_OBJ_VNUM(object) <= 19199) {
            if ((GET_OBJ_VNUM(object) <= 18999 && GET_OBJ_VNUM(object) >= 18800) ||
                (GET_OBJ_VNUM(object) <= 19199 && GET_OBJ_VNUM(object) >= 19100)) {
                int hnum = GET_OBJ_VAL(object, 0);
                struct obj_data *house = read_object(hnum, VIRTUAL);
                obj_to_room(house, real_room(GET_OBJ_VAL(object, 6)));
                SET_BIT(GET_OBJ_VAL(object, VAL_CONTAINER_FLAGS), CONT_CLOSED);
                SET_BIT(GET_OBJ_VAL(object, VAL_CONTAINER_FLAGS), CONT_LOCKED);
            }
        }

        if (GET_OBJ_TYPE(object) == ITEM_HATCH && GET_OBJ_VAL(object, 0) > 1 && GET_OBJ_VNUM(object) > 19199) {
            if (!(vehicle = find_vehicle_by_vnum(GET_OBJ_VAL(object, VAL_HATCH_DEST)))) {
                if (real_room(GET_OBJ_VAL(object, 3)) != NOWHERE) {
                    vehicle = read_object(GET_OBJ_VAL(object, 0), VIRTUAL);
                    if(!vehicle) {
                        basic_mud_log("SYSERR: Vehicle %d not found for hatch %d", GET_OBJ_VAL(object, 0), GET_OBJ_VNUM(object));
                    }
                    obj_to_room(vehicle, real_room(GET_OBJ_VAL(object, 3)));
                    if (object->look_description) {
                        if (strlen(object->look_description)) {
                            char nick[MAX_INPUT_LENGTH], nick2[MAX_INPUT_LENGTH], nick3[MAX_INPUT_LENGTH];
                            if (GET_OBJ_VNUM(vehicle) <= 46099 && GET_OBJ_VNUM(vehicle) >= 46000) {
                                sprintf(nick, "Saiyan Pod %s", object->look_description);
                                sprintf(nick2, "@wA @Ys@ya@Yi@yy@Ya@yn @Dp@Wo@Dd@w named @D(@C%s@D)@w",
                                        object->look_description);
                            } else if (GET_OBJ_VNUM(vehicle) >= 46100 && GET_OBJ_VNUM(vehicle) <= 46199) {
                                sprintf(nick, "EDI Xenofighter MK. II %s", object->look_description);
                                sprintf(nick2,
                                        "@wAn @YE@yD@YI @CX@ce@Wn@Do@Cf@ci@Wg@Dh@Wt@ce@Cr @RMK. II @wnamed @D(@C%s@D)@w",
                                        object->look_description);
                            }
                            sprintf(nick3, "%s is resting here@w", nick2);
                            vehicle->name = strdup(nick);
                            vehicle->short_description = strdup(nick2);
                            vehicle->room_description = strdup(nick3);
                        }
                    }
                    SET_BIT(GET_OBJ_VAL(object, VAL_CONTAINER_FLAGS), CONT_CLOSED);
                    SET_BIT(GET_OBJ_VAL(object, VAL_CONTAINER_FLAGS), CONT_LOCKED);
                } else {
                    basic_mud_log("Hatch load: Hatch with no vehicle load room: #%d!", GET_OBJ_VNUM(object));
                }
            }
        }
    }

    if (EXIT(object, 5) &&
        (SECT(IN_ROOM(object)) == SECT_UNDERWATER || SECT(IN_ROOM(object)) == SECT_WATER_NOSWIM)) {
        act("$p @Bsinks to deeper waters.@n", true, nullptr, object, nullptr, TO_ROOM);
        int numb = GET_ROOM_VNUM(EXIT(object, 5)->to_room);
        obj_from_room(object);
        obj_to_room(object, real_room(numb));
    }
    if (EXIT(object, 5) && SECT(IN_ROOM(object)) == SECT_FLYING &&
        (GET_OBJ_VNUM(object) < 80 || GET_OBJ_VNUM(object) > 83)) {
        act("$p @Cfalls down.@n", true, nullptr, object, nullptr, TO_ROOM);
        int numb = GET_ROOM_VNUM(EXIT(object, 5)->to_room);
        obj_from_room(object);
        obj_to_room(object, real_room(numb));
        if (SECT(IN_ROOM(object)) != SECT_FLYING) {
            act("$p @Cfalls down and smacks the ground.@n", true, nullptr, object, nullptr, TO_ROOM);
        }
    }
    if (GET_OBJ_VAL(object, 0) != 0) {
        if (GET_OBJ_VNUM(object) == 16705 || GET_OBJ_VNUM(object) == 16706 || GET_OBJ_VNUM(object) == 16707) {
            object->level = GET_OBJ_VAL(object, 0);
        }
    }
}


/* put an object in a room */
void obj_to_room(struct obj_data *object, room_rnum room) {
    if(!object) return;
    if(!world.count(room)) return;
    obj_to_room(object, &world[room]);
}


/* Take an object from a room */
void obj_from_room(struct obj_data *object) {
    struct obj_data *temp;

    if (!object || IN_ROOM(object) == NOWHERE) {
        basic_mud_log("SYSERR: nullptr object (%p) or obj not in a room (%d) passed to obj_from_room",
            object, IN_ROOM(object));
        return;
    }

    if (GET_OBJ_POSTED(object) && object->in_obj == nullptr) {
        struct obj_data *obj = GET_OBJ_POSTED(object);
        if (GET_OBJ_POSTTYPE(object) <= 0) {
            send_to_room(IN_ROOM(obj), "%s@W shakes loose from %s@W.@n\r\n", obj->short_description,
                         object->short_description);
        } else {
            send_to_room(IN_ROOM(obj), "%s@W comes loose from %s@W.@n\r\n", object->short_description,
                         obj->short_description);
        }
        GET_OBJ_POSTED(obj) = nullptr;
        GET_OBJ_POSTTYPE(obj) = 0;
        GET_OBJ_POSTED(object) = nullptr;
        GET_OBJ_POSTTYPE(object) = 0;
    }

    REMOVE_FROM_LIST(object, object->getRoom()->contents, next_content, temp);

    IN_ROOM(object) = NOWHERE;
    object->next_content = nullptr;

    if(!gameIsLoading) object->save();

}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to) {
    struct obj_data *tmp_obj;

    if (!obj || !obj_to || obj == obj_to) {
        basic_mud_log("SYSERR: nullptr object (%p) or same source (%p) and target (%p VNUM: %d) obj passed to obj_to_obj.",
            obj, obj, obj_to, obj_to ? GET_OBJ_VNUM(obj_to) : -1);
        return;
    }

    obj->next_content = obj_to->contents;
    obj_to->contents = obj;
    obj->in_obj = obj_to;
    tmp_obj = obj->in_obj;

    if(!gameIsLoading) obj->save();
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj) {
    struct obj_data *temp, *obj_from;

    if (obj->in_obj == nullptr) {
        basic_mud_log("SYSERR: (%s): trying to illegally extract obj from obj.", __FILE__);
        return;
    }
    obj_from = obj->in_obj;
    temp = obj->in_obj;
    REMOVE_FROM_LIST(obj, obj_from->contents, next_content, temp);
    
    if(!gameIsLoading) obj->save();

    obj->in_obj = nullptr;
    obj->next_content = nullptr;
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch) {
    if (list) {
        object_list_new_owner(list->contents, ch);
        object_list_new_owner(list->next_content, ch);
        list->carried_by = ch;
    }
}


/* Extract an object from the world */
void extract_obj(struct obj_data *obj) {
    struct obj_data *temp;
    struct char_data *ch;
    obj->clearLocation();

    /* Get rid of the contents of the object, as well. */
    if (GET_FELLOW_WALL(obj) && GET_OBJ_VNUM(obj) == 79) {
        struct obj_data *trash;
        trash = GET_FELLOW_WALL(obj);
        GET_FELLOW_WALL(obj) = nullptr;
        GET_FELLOW_WALL(trash) = nullptr;
        extract_obj(trash);
    }
    if (SITTING(obj)) {
        ch = SITTING(obj);
        SITTING(obj) = nullptr;
        SITS(ch) = nullptr;
    }
    if (GET_OBJ_POSTED(obj) && obj->in_obj == nullptr) {
        struct obj_data *obj2 = GET_OBJ_POSTED(obj);
        GET_OBJ_POSTED(obj2) = nullptr;
        GET_OBJ_POSTTYPE(obj2) = 0;
        GET_OBJ_POSTED(obj) = nullptr;
    }
    if (TARGET(obj)) {
        TARGET(obj) = nullptr;
    }
    if (USER(obj)) {
        USER(obj) = nullptr;
    }

    while (obj->contents)
        extract_obj(obj->contents);

    obj->deactivate();

    if (GET_OBJ_RNUM(obj) != NOTHING)
        (obj_index[GET_OBJ_RNUM(obj)].vn)--;

    if (SCRIPT(obj))
        extract_script(obj, OBJ_TRIGGER);

    auto found = uniqueObjects.find(obj->id);
    if (found != uniqueObjects.end()) {
        uniqueObjects.erase(found);
    }

    free_obj(obj);
}


static void update_object(struct obj_data *obj, int use) {
    if (!obj)
        return;
    /* dont update objects with a timer trigger */
    if (!SCRIPT_CHECK(obj, OTRIG_TIMER) && (GET_OBJ_TIMER(obj) > 0))
        GET_OBJ_TIMER(obj) -= use;
    if (obj->contents)
        update_object(obj->contents, use);
    if (obj->next_content)
        update_object(obj->next_content, use);
}


void update_char_objects(struct char_data *ch) {
    int i, j;

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i)) {
            if (GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT && GET_OBJ_VAL(GET_EQ(ch, i), VAL_LIGHT_HOURS) > 0 &&
                GET_OBJ_VAL(GET_EQ(ch, i), VAL_LIGHT_TIME) <= 0) {
                j = --GET_OBJ_VAL(GET_EQ(ch, i), VAL_LIGHT_HOURS);
                GET_OBJ_VAL(GET_EQ(ch, i), VAL_LIGHT_TIME) = 3;
                if (j == 1) {
                    send_to_char(ch, "Your light begins to flicker and fade.\r\n");
                    act("$n's light begins to flicker and fade.", false, ch, nullptr, nullptr, TO_ROOM);
                } else if (j == 0) {
                    send_to_char(ch, "Your light sputters out and dies.\r\n");
                    act("$n's light sputters out and dies.", false, ch, nullptr, nullptr, TO_ROOM);
                }
            } else if (GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT && GET_OBJ_VAL(GET_EQ(ch, i), VAL_LIGHT_HOURS) > 0) {
                GET_OBJ_VAL(GET_EQ(ch, i), VAL_LIGHT_TIME) -= 1;
            }
            update_object(GET_EQ(ch, i), 2);
        }

    if (ch->contents)
        update_object(ch->contents, 1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char_final(struct char_data *ch) {
    struct char_data *k, *temp;
    struct obj_data *chair;
    struct descriptor_data *d;
    struct obj_data *obj;
    int i;

    if (IN_ROOM(ch) == NOWHERE) {
        basic_mud_log("SYSERR: NOWHERE extracting char %s. (%s, extract_char_final)",
            GET_NAME(ch), __FILE__);
        shutdown_game(1);
    }

    if(!IS_NPC(ch)) {
        // PCs have a very complicated set of places they're not allowed to log off from.
        // If they do, somehow, then this function will ensure they show up somewhere sane.
        ch->load_room = ch->normalizeLoadRoom(IN_ROOM(ch));
    }

    /*
     * We're booting the character of someone who has switched so first we
     * need to stuff them back into their own body.  This will set ch->desc
     * we're checking below this loop to the proper value.
     */
    if (!IS_NPC(ch) && !ch->desc) {
        for (d = descriptor_list; d; d = d->next)
            if (d->original == ch) {
                do_return(d->character, nullptr, 0, 0);
                break;
            }
    }

    if (ch->desc) {
        if (ch->desc->original)
            do_return(ch, nullptr, 0, 0);
    }
    /* On with the character's assets... */

    if (ch->followers || ch->master)
        die_follower(ch);

    if (SITS(ch)) {
        chair = SITS(ch);
        SITTING(chair) = nullptr;
        SITS(ch) = nullptr;
    }

    if (auto original = GET_ORIGINAL(ch); original) {
        original->clones.erase(ch);
    }

    if (!ch->clones.empty()) {
        auto clones = ch->clones;
        for(auto &c : clones) handle_multi_merge(c);
    }

    purge_homing(ch);

    if (MINDLINK(ch)) {
        MINDLINK(MINDLINK(ch)) = nullptr;
        MINDLINK(ch) = nullptr;
    }

    if (GRAPPLING(ch)) {
        act("@WYou stop grappling with @C$N@W!@n", true, ch, nullptr, GRAPPLING(ch), TO_CHAR);
        act("@C$n@W stops grappling with @c$N@W!@n", true, ch, nullptr, GRAPPLING(ch), TO_ROOM);
        GRAPTYPE(GRAPPLING(ch)) = -1;
        GRAPPLED(GRAPPLING(ch)) = nullptr;
        GRAPPLING(ch) = nullptr;
        GRAPTYPE(ch) = -1;
    }
    if (GRAPPLED(ch)) {
        act("@WYou stop being grappled with by @C$N@W!@n", true, ch, nullptr, GRAPPLED(ch), TO_CHAR);
        act("@C$n@W stops being grappled with by @c$N@W!@n", true, ch, nullptr, GRAPPLED(ch), TO_ROOM);
        GRAPTYPE(GRAPPLED(ch)) = -1;
        GRAPPLING(GRAPPLED(ch)) = nullptr;
        GRAPPLED(ch) = nullptr;
        GRAPTYPE(ch) = -1;
    }

    if (CARRYING(ch)) {
        carry_drop(ch, 3);
    }
    if (CARRIED_BY(ch)) {
        carry_drop(CARRIED_BY(ch), 3);
    }

    if (ch->poisonby) {
        ch->poisonby->poisoned.erase(ch);
        ch->poisonby = nullptr;
    }

    for(auto c : ch->poisoned) {
        c->poisonby = nullptr;
    }
    ch->poisoned.clear();

    if (DRAGGING(ch)) {
        act("@WYou stop dragging @C$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_CHAR);
        act("@C$n@W stops dragging @c$N@W!@n", true, ch, nullptr, DRAGGING(ch), TO_ROOM);
        DRAGGED(DRAGGING(ch)) = nullptr;
        DRAGGING(ch) = nullptr;
    }

    if (DRAGGED(ch)) {
        act("@WYou stop being dragged by @C$N@W!@n", true, ch, nullptr, DRAGGED(ch), TO_CHAR);
        act("@C$n@W stops being dragged by @c$N@W!@n", true, ch, nullptr, DRAGGED(ch), TO_ROOM);
        DRAGGING(DRAGGED(ch)) = nullptr;
        DRAGGED(ch) = nullptr;
    }

    if (GET_DEFENDER(ch)) {
        GET_DEFENDING(GET_DEFENDER(ch)) = nullptr;
        GET_DEFENDER(ch) = nullptr;
    }
    if (GET_DEFENDING(ch)) {
        GET_DEFENDER(GET_DEFENDING(ch)) = nullptr;
        GET_DEFENDING(ch) = nullptr;
    }

    if (BLOCKED(ch)) {
        BLOCKS(BLOCKED(ch)) = nullptr;
        BLOCKED(ch) = nullptr;
    }
    if (BLOCKS(ch)) {
        BLOCKED(BLOCKS(ch)) = nullptr;
        BLOCKS(ch) = nullptr;
    }
    if (ABSORBING(ch)) {
        ABSORBBY(ABSORBING(ch)) = nullptr;
        ABSORBING(ch) = nullptr;
    }
    if (ABSORBBY(ch)) {
        ABSORBING(ABSORBBY(ch)) = nullptr;
        ABSORBBY(ch) = nullptr;
    }

    /* transfer objects to room, if any */
    if(IS_NPC(ch)) {
        while (ch->contents) {
            obj = ch->contents;
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
        }

        /* transfer equipment to room, if any */
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i))
                obj_to_room(unequip_char(ch, i), IN_ROOM(ch));
    }

    if (FIGHTING(ch))
        stop_fighting(ch);

    for (k = combat_list; k; k = temp) {
        temp = k->next_fighting;
        if (FIGHTING(k) == ch)
            stop_fighting(k);
    }

    char_from_room(ch);

    if (IS_NPC(ch)) {
        if (GET_MOB_RNUM(ch) != NOTHING)    /* prototyped */
            erase_vnum(characterVnumIndex, ch);
        clearMemory(ch);
        if (SCRIPT(ch))
            extract_script(ch, MOB_TRIGGER);
        if (SCRIPT_MEM(ch))
            extract_script_mem(SCRIPT_MEM(ch));
    } else {
        ch->save();
    }

    /* If there's a descriptor, they're in the menu now. */
    if(ch->desc) {
        ch->desc->connected = CON_QUITGAME;
    }

    ch->deactivate();
    if (IS_NPC(ch)) {
        auto found = uniqueCharacters.find(ch->id);
        if (found != uniqueCharacters.end()) {
            uniqueCharacters.erase(found);
        }
        free_char(ch);
    }
}


/*
 * Q: Why do we do this?
 * A: Because trying to iterate over the character
 *    list with 'ch = ch->next' does bad things if
 *    the current character happens to die. The
 *    trivial workaround of 'vict = next_vict'
 *    doesn't work if the _next_ person in the list
 *    gets killed, for example, by an area spell.
 *
 * Q: Why do we leave them on the character_list?
 * A: Because code doing 'vict = vict->next' would
 *    get really confused otherwise.
 */
void extract_char(struct char_data *ch) {
    if(!ch->active) {
        logger->warn("Attempt to extract an inactive character.");
        return;
    }

    extractions_pending.insert(ch);

    for (auto foll = ch->followers; foll; foll = foll->next) {
        if (IS_NPC(foll->follower) && AFF_FLAGGED(foll->follower, AFF_CHARM) &&
            (IN_ROOM(foll->follower) == IN_ROOM(ch) || IN_ROOM(ch) == 1)) {
            /* transfer objects to char, if any */
            while (foll->follower->contents) {
                auto obj = foll->follower->contents;
                obj_from_char(obj);
                obj_to_char(obj, ch);
            }

            /* transfer equipment to char, if any */
            for (auto i = 0; i < NUM_WEARS; i++)
                if (GET_EQ(foll->follower, i)) {
                    auto obj = unequip_char(foll->follower, i);
                    obj_to_char(obj, ch);
                }

            extract_char(foll->follower);
        }
    }
}


/*
 * I'm not particularly pleased with the MOB/PLR
 * hoops that have to be jumped through but it
 * hardly calls for a completely new variable.
 * Ideally it would be its own list, but that
 * would change the '->next' pointer, potentially
 * confusing some code. Ugh. -gg 3/15/2001
 *
 * NOTE: This doesn't handle recursive extractions.
 */
void extract_pending_chars(uint64_t heartBeat, double deltaTime) {
    auto pending = extractions_pending;

    for(auto ch : pending) {
        extractions_pending.erase(ch);
        extract_char_final(ch);
    }
}


/* ***********************************************************************
* Here follows high-level versions of some earlier routines, ie functions*
* which incorporate the actual player-data                               *.
*********************************************************************** */


struct char_data *get_player_vis(struct char_data *ch, char *name, int *number, int inroom) {
    struct char_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    for (i = character_list; i; i = i->next) {
        if (IS_NPC(i))
            continue;
        if (inroom == FIND_CHAR_ROOM && IN_ROOM(i) != IN_ROOM(ch))
            continue;
        if (GET_ADMLEVEL(ch) < 1 && GET_ADMLEVEL(i) < 1 && !IS_NPC(ch) && !IS_NPC(i)) {
            if (strcasecmp(RACE(i), name) && !strstr(RACE(i), name)) {
                if (readIntro(ch, i) == 1) {
                    if (strcasecmp(get_i_name(ch, i), name) && !strstr(get_i_name(ch, i), name)) {
                        continue;
                    }
                } else {
                    continue;
                }
            }
        }
        if ((GET_ADMLEVEL(ch) >= 1 || GET_ADMLEVEL(i) >= 1 || IS_NPC(ch) || IS_NPC(i))) {
            if (strcasecmp(i->name, name) && !strstr(i->name, name)) {
                if (strcasecmp(RACE(i), name) && !strstr(RACE(i), name)) {
                    if (!IS_NPC(ch) && !IS_NPC(i) && readIntro(ch, i) == 1) {
                        if (strcasecmp(get_i_name(ch, i), name) && !strstr(get_i_name(ch, i), name)) {
                            continue;
                        }
                    } else {
                        continue;
                    }
                }
            }
        }
        if (!CAN_SEE(ch, i))
            continue;
        if (--(*number) != 0)
            continue;
        return (i);
    }

    return (nullptr);
}


struct char_data *get_char_room_vis(struct char_data *ch, char *name, int *number) {
    struct char_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    /* JE 7/18/94 :-) :-) */
    if (!strcasecmp(name, "self") || !strcasecmp(name, "me"))
        return (ch);

    /* 0.<name> means PC with name */
    if (*number == 0)
        return (get_player_vis(ch, name, nullptr, FIND_CHAR_ROOM));

    for (i = ch->getRoom()->people; i && *number; i = i->next_in_room) {
        if (!strcasecmp(name, "last") && LASTHIT(i) != 0 && LASTHIT(i) == GET_IDNUM(ch)) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (isname(name, i->name) && (IS_NPC(i) || IS_NPC(ch) || GET_ADMLEVEL(i) > 0 || GET_ADMLEVEL(ch) > 0) &&
                   i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (isname(name, i->name) && i == ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (!IS_NPC(i) && !IS_NPC(ch) && !strcasecmp(get_i_name(ch, i), CAP(name)) && i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (!IS_NPC(i) && !IS_NPC(ch) && strstr(get_i_name(ch, i), CAP(name)) && i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (!IS_NPC(i) && !(strcmp(RACE(i), CAP(name))) && i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (!IS_NPC(i) && strstr(RACE(i), CAP(name)) && i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (!IS_NPC(i) && !(strcmp(RACE(i), name)) && i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        } else if (!IS_NPC(i) && strstr(RACE(i), name) && i != ch) {
            if (CAN_SEE(ch, i))
                if (--(*number) == 0)
                    return (i);
        }
    }
    return (nullptr);
}


struct char_data *get_char_world_vis(struct char_data *ch, char *name, int *number) {
    struct char_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    if ((i = get_char_room_vis(ch, name, number)) != nullptr)
        return (i);

    if (*number == 0)
        return get_player_vis(ch, name, nullptr, 0);

    for (i = character_list; i && *number; i = i->next) {
        if (IN_ROOM(ch) == IN_ROOM(i))
            continue;
        if (GET_ADMLEVEL(ch) < 1 && GET_ADMLEVEL(i) < 1 && !IS_NPC(ch) && !IS_NPC(i)) {
            if (strcasecmp(RACE(i), name) && !strstr(RACE(i), name)) {
                if (readIntro(ch, i) == 1) {
                    if (strcasecmp(get_i_name(ch, i), name) && !strstr(get_i_name(ch, i), name)) {
                        continue;
                    }
                } else {
                    continue;
                }
            }
        }
        if ((GET_ADMLEVEL(ch) >= 1 || GET_ADMLEVEL(i) >= 1 || IS_NPC(ch) || IS_NPC(i))) {
            if (strcasecmp(i->name, name) && !strstr(i->name, name)) {
                if (strcasecmp(RACE(i), name) && !strstr(RACE(i), name)) {
                    if (!IS_NPC(ch) && !IS_NPC(i) && readIntro(ch, i) == 1) {
                        if (strcasecmp(get_i_name(ch, i), name) && !strstr(get_i_name(ch, i), name)) {
                            continue;
                        }
                    } else {
                        continue;
                    }
                }
            }
        }
        /*if (!CAN_SEE(ch, i))
          continue;*/
        if (--(*number) != 0)
            continue;

        return (i);
    }
    return (nullptr);
}


struct char_data *get_char_vis(struct char_data *ch, char *name, int *number, int where) {
    if (where == FIND_CHAR_ROOM)
        return get_char_room_vis(ch, name, number);
    else if (where == FIND_CHAR_WORLD)
        return get_char_world_vis(ch, name, number);
    else
        return (nullptr);
}


struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, int *number, struct obj_data *list) {
    struct obj_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    if (*number == 0)
        return (nullptr);

    for (i = list; i && *number; i = i->next_content)
        if (isname(name, i->name))
            if (CAN_SEE_OBJ(ch, i) || (GET_OBJ_TYPE(i) == ITEM_LIGHT))
                if (--(*number) == 0)
                    return (i);

    return (nullptr);
}


/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name, int *number) {
    struct obj_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    if (*number == 0)
        return (nullptr);

    /* scan items carried */
    if ((i = get_obj_in_list_vis(ch, name, number, ch->contents)) != nullptr)
        return (i);

    /* scan room */
    if ((i = get_obj_in_list_vis(ch, name, number, ch->getRoom()->contents)) != nullptr)
        return (i);

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list; i && *number; i = i->next)
        if (isname(name, i->name))
            if (CAN_SEE_OBJ(ch, i))
                if (--(*number) == 0)
                    return (i);

    return (nullptr);
}


struct obj_data *get_obj_in_equip_vis(struct char_data *ch, char *arg, int *number, struct obj_data *equipment[]) {
    int j, num;

    if (!number) {
        number = &num;
        num = get_number(&arg);
    }

    if (*number == 0)
        return (nullptr);

    for (j = 0; j < NUM_WEARS; j++)
        if (equipment[j] && CAN_SEE_OBJ(ch, equipment[j]) && isname(arg, equipment[j]->name))
            if (--(*number) == 0)
                return (equipment[j]);

    return (nullptr);
}


int get_obj_pos_in_equip_vis(struct char_data *ch, char *arg, int *number, struct obj_data *equipment[]) {
    int j, num;

    if (!number) {
        number = &num;
        num = get_number(&arg);
    }

    if (*number == 0)
        return (-1);

    for (j = 0; j < NUM_WEARS; j++)
        if (equipment[j] && CAN_SEE_OBJ(ch, equipment[j]) && isname(arg, equipment[j]->name))
            if (--(*number) == 0)
                return (j);

    return (-1);
}


const char *money_desc(int amount) {
    int cnt;
    struct {
        int limit;
        const char *description;
    } money_table[] = {
            {1,     "a single zenni"},
            {10,    "a tiny pile of zenni"},
            {20,    "a handful of zenni"},
            {75,    "a little pile of zenni"},
            {150,   "a small pile of zenni"},
            {250,   "a pile of zenni"},
            {500,   "a big pile of zenni"},
            {1000,  "a large heap of zenni"},
            {5000,  "a huge mound of zenni"},
            {10000, "an enormous mound of zenni"},
            {15000, "a small mountain of zenni"},
            {20000, "a mountain of zenni"},
            {25000, "a huge mountain of zenni"},
            {50000, "an enormous mountain of zenni"},
            {0,     nullptr},
    };

    if (amount <= 0) {
        basic_mud_log("SYSERR: Try to create negative or 0 money (%d).", amount);
        return (nullptr);
    }

    for (cnt = 0; money_table[cnt].limit; cnt++)
        if (amount <= money_table[cnt].limit)
            return (money_table[cnt].description);

    return ("an absolutely colossal mountain of zenni");
}


struct obj_data *create_money(int amount) {
    struct obj_data *obj;
    struct extra_descr_data *new_descr;
    char buf[200];
    int y;

    if (amount <= 0) {
        basic_mud_log("SYSERR: Try to create negative or 0 money. (%d)", amount);
        return (nullptr);
    }
    obj = create_obj();
    CREATE(new_descr, struct extra_descr_data, 1);

    if (amount == 1) {
        obj->name = strdup("zenni money");
        obj->short_description = strdup("a single zenni");
        obj->room_description = strdup("One miserable zenni is lying here");
        new_descr->keyword = strdup("zenni money");
        new_descr->description = strdup("It's just one miserable little zenni.");
    } else {
        obj->name = strdup("zenni money");
        obj->short_description = strdup(money_desc(amount));
        snprintf(buf, sizeof(buf), "%s is lying here", money_desc(amount));
        obj->room_description = strdup(CAP(buf));

        new_descr->keyword = strdup("zenni money");
        if (amount < 10)
            snprintf(buf, sizeof(buf), "There is %d zenni.", amount);
        else if (amount < 100)
            snprintf(buf, sizeof(buf), "There is about %d zenni.", 10 * (amount / 10));
        else if (amount < 1000)
            snprintf(buf, sizeof(buf), "It looks to be about %d zenni.", 100 * (amount / 100));
        else if (amount < 100000)
            snprintf(buf, sizeof(buf), "You guess there is, maybe, %d zenni.",
                     1000 * ((amount / 1000) + rand_number(0, (amount / 1000))));
        else
            strcpy(buf, "There are is LOT of zenni.");    /* strcpy: OK (is < 200) */
        new_descr->description = strdup(buf);
    }

    new_descr->next = nullptr;
    obj->ex_description = new_descr;

    GET_OBJ_TYPE(obj) = ITEM_MONEY;
    GET_OBJ_MATERIAL(obj) = MATERIAL_GOLD;
    GET_OBJ_VAL(obj, VAL_ALL_MAXHEALTH) = 100;
    GET_OBJ_VAL(obj, VAL_ALL_HEALTH) = 100;
    for (y = 0; y < TW_ARRAY_MAX; y++)
        obj->wear_flags[y] = 0;
    obj->wear_flags.set(ITEM_WEAR_TAKE);
    GET_OBJ_VAL(obj, VAL_MONEY_SIZE) = amount;
    GET_OBJ_COST(obj) = amount;
    obj->vn = NOTHING;

    return (obj);
}


/* Generic Find, designed to find any object/character
 *
 * Calling:
 *  *arg     is the pointer containing the string to be searched for.
 *           This string doesn't have to be a single word, the routine
 *           extracts the next word itself.
 *  bitv..   All those bits that you want to "search through".
 *           Bit found will be result of the function
 *  *ch      This is the person that is trying to "find"
 *  **tar_ch Will be nullptr if no character was found, otherwise points
 * **tar_obj Will be nullptr if no object was found, otherwise points
 *
 * The routine used to return a pointer to the next word in *arg (just
 * like the one_argument routine), but now it returns an integer that
 * describes what it filled in.
 */
int generic_find(char *arg, bitvector_t bitvector, struct char_data *ch,
                 struct char_data **tar_ch, struct obj_data **tar_obj) {
    int i, found, number;
    char name_val[MAX_INPUT_LENGTH];
    char *name = name_val;

    *tar_ch = nullptr;
    *tar_obj = nullptr;

    one_argument(arg, name);

    if (!*name)
        return (0);
    if (!(number = get_number(&name)))
        return (0);

    if (IS_SET(bitvector, FIND_CHAR_ROOM)) {    /* Find person in room */
        if ((*tar_ch = get_char_room_vis(ch, name, &number)) != nullptr)
            return (FIND_CHAR_ROOM);
    }

    if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
        if ((*tar_ch = get_char_world_vis(ch, name, &number)) != nullptr)
            return (FIND_CHAR_WORLD);
    }

    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
        for (found = false, i = 0; i < NUM_WEARS && !found; i++)
            if (GET_EQ(ch, i) && isname(name, GET_EQ(ch, i)->name) && --number == 0) {
                *tar_obj = GET_EQ(ch, i);
                found = true;
            }
        if (found)
            return (FIND_OBJ_EQUIP);
    }

    if (IS_SET(bitvector, FIND_OBJ_INV)) {
        if ((*tar_obj = get_obj_in_list_vis(ch, name, &number, ch->contents)) != nullptr)
            return (FIND_OBJ_INV);
    }

    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
        if ((*tar_obj = get_obj_in_list_vis(ch, name, &number, ch->getRoom()->contents)) != nullptr)
            return (FIND_OBJ_ROOM);
    }

    if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
        if ((*tar_obj = get_obj_vis(ch, name, &number)))
            return (FIND_OBJ_WORLD);
    }

    return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg) {
    if (!strcmp(arg, "all"))
        return (FIND_ALL);
    else if (!strncmp(arg, "all.", 4)) {
        strcpy(arg, arg + 4);    /* strcpy: OK (always less) */
        return (FIND_ALLDOT);
    } else
        return (FIND_INDIV);
}

void affectv_to_char(struct char_data *ch, struct affected_type *af) {
    struct affected_type *affected_alloc;

    CREATE(affected_alloc, struct affected_type, 1);

    if (!ch->affectedv) {
        ch->next_affectv = affectv_list;
        affectv_list = ch;
    }
    *affected_alloc = *af;
    affected_alloc->next = ch->affectedv;
    ch->affectedv = affected_alloc;

    affect_modify(ch, af->location, af->modifier, af->specific, af->bitvector, true);
    affect_total(ch);
}

void affectv_remove(struct char_data *ch, struct affected_type *af) {
    struct affected_type *cmtemp;

    if (ch->affectedv == nullptr) {
        core_dump();
        return;
    }

    affect_modify(ch, af->location, af->modifier, af->specific, af->bitvector, false);
    REMOVE_FROM_LIST(af, ch->affectedv, next, cmtemp);
    free(af);
    affect_total(ch);
    if (!ch->affectedv) {
        struct char_data *temp;
        REMOVE_FROM_LIST(ch, affectv_list, next_affectv, temp);
        ch->next_affectv = nullptr;
    }
}

void affectv_join(struct char_data *ch, struct affected_type *af,
                  bool add_dur, bool avg_dur, bool add_mod, bool avg_mod) {
    struct affected_type *hjp, *next;
    bool found = false;

    for (hjp = ch->affectedv; !found && hjp; hjp = next) {
        next = hjp->next;

        if ((hjp->type == af->type) && (hjp->location == af->location)) {
            if (add_dur)
                af->duration += hjp->duration;
            if (avg_dur)
                af->duration /= 2;

            if (add_mod)
                af->modifier += hjp->modifier;
            if (avg_mod)
                af->modifier /= 2;

            affectv_remove(ch, hjp);
            affectv_to_char(ch, af);
            found = true;
        }
    }
    if (!found)
        affectv_to_char(ch, af);
}

int is_better(struct obj_data *object, struct obj_data *object2) {
    int value1 = 0, value2 = 0;

    switch (GET_OBJ_TYPE(object)) {
        case ITEM_ARMOR:
            value1 = GET_OBJ_VAL(object, VAL_ARMOR_APPLYAC);
            value2 = GET_OBJ_VAL(object2, VAL_ARMOR_APPLYAC);
            break;
        case ITEM_WEAPON:
            value1 = (1 + GET_OBJ_VAL(object, VAL_WEAPON_DAMSIZE)) * GET_OBJ_VAL(object, VAL_WEAPON_DAMDICE);
            value2 = (1 + GET_OBJ_VAL(object2, VAL_WEAPON_DAMSIZE)) * GET_OBJ_VAL(object2, VAL_WEAPON_DAMDICE);
            break;
        default:
            break;
    }

    if (value1 > value2)
        return 1;
    else
        return 0;
}

/* check and see if this item is better */
void item_check(struct obj_data *object, struct char_data *ch) {
    int where = 0;

    if (IS_HUMANOID(ch) && !(mob_index[GET_MOB_RNUM(ch)].func == shop_keeper)) {
        if (invalid_align(ch, object) || invalid_class(ch, object))
            return;

        switch (GET_OBJ_TYPE(object)) {
            case ITEM_WEAPON:
                if (!GET_EQ(ch, WEAR_WIELD1)) {
                    perform_wear(ch, object, WEAR_WIELD1);
                } else {
                    if (is_better(object, GET_EQ(ch, WEAR_WIELD1))) {
                        perform_remove(ch, WEAR_WIELD1);
                        perform_wear(ch, object, WEAR_WIELD1);
                    }
                }
                break;
            case ITEM_ARMOR:
            case ITEM_WORN:
                where = find_eq_pos(ch, object, nullptr);
                if (!GET_EQ(ch, where)) {
                    perform_wear(ch, object, where);
                } else {
                    if (is_better(object, GET_EQ(ch, where))) {
                        perform_remove(ch, where);
                        perform_wear(ch, object, where);
                    }
                }

                break;
            default:
                break;
        }
    }
}
