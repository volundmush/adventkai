/************************************************************************
 * Generic OLC Library - Objects / genobj.c			v1.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/genobj.h"
#include "dbat/genolc.h"
#include "dbat/genzon.h"
#include "dbat/utils.h"
#include "dbat/handler.h"
#include "dbat/dg_olc.h"
#include "dbat/shop.h"
#include "dbat/objsave.h"

static int copy_object_main(struct obj_data *to, struct obj_data *from, int free_object);


obj_rnum add_object(struct obj_data *newobj, obj_vnum ovnum) {
    int found = NOTHING;
    zone_rnum rznum = real_zone_by_thing(ovnum);

    /*
     * Write object to internal tables.
     */
    if ((newobj->vn = real_object(ovnum)) != NOTHING) {
        copy_object(&obj_proto[newobj->vn], newobj);
        update_objects(&obj_proto[newobj->vn]);
        dirty_item_prototypes.insert(ovnum);
        return newobj->vn;
    }

    found = insert_object(newobj, ovnum);

    dirty_item_prototypes.insert(ovnum);
    auto &z = zone_table[rznum];
    z.objects.insert(ovnum);
    return found;
}

/* ------------------------------------------------------------------------------------------------------------------------------ */

/*
 * Fix all existing objects to have these values.
 * We need to run through each and every object currently in the
 * game to see which ones are pointing to this prototype.
 * if object is pointing to this prototype, then we need to replace it
 * with the new one.
 */
int update_objects(struct obj_data *refobj) {
    struct obj_data *obj, swap;
    int count = 0;

    for (obj = object_list; obj; obj = obj->next) {
        if (obj->vn != refobj->vn)
            continue;

        count++;

        /* Update the existing object but save a copy for private information. */
        swap = *obj;
        *obj = *refobj;

        /* Copy game-time dependent variables over. */
        IN_ROOM(obj) = swap.in_room;
        obj->carried_by = swap.carried_by;
        obj->worn_by = swap.worn_by;
        obj->worn_on = swap.worn_on;
        obj->in_obj = swap.in_obj;
        obj->contents = swap.contents;
        obj->next_content = swap.next_content;
        obj->next = swap.next;
    }

    return count;
}

/* ------------------------------------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------------------------------------ */

/*
 * Function handle the insertion of an object within the prototype framework.  Note that this does not adjust internal values
 * of other objects, use add_object() for that.
 */
obj_rnum insert_object(struct obj_data *obj, obj_vnum ovnum) {

    auto exists = obj_proto.count(ovnum);
    auto &o = obj_proto[ovnum];
    o = *obj;

    /* Not found, place at 0. */
    return false;
}


/* ------------------------------------------------------------------------------------------------------------------------------ */

int save_objects(zone_rnum zone_num) {
    auto z = zone_table.find(zone_num);

    if (z == zone_table.end()) {
        basic_mud_log("SYSERR: OasisOLC: save_objects: Invalid real zone number %d.", zone_num);
        return false;
    }

    dirty_item_prototypes.insert(z->second.objects.begin(), z->second.objects.end());
    z->second.save_objects();
    return true;
}

/*
 * Free all, unconditionally.
 */
void free_object_strings(struct obj_data *obj) {
#if 0 /* Debugging, do not enable. */
    extern struct obj_data *object_list;
    struct obj_data *t;
    int i = 0;

    for (t = object_list; t; t = t->next) {
      if (t == obj) {
        i++;
        continue;
      }
      assert(obj->name != t->name);
      assert(obj->description != t->description);
      assert(obj->short_description != t->short_description);
      assert(obj->action_description != t->action_description);
      assert(obj->ex_description != t->ex_description);
    }
    assert(i <= 1);
#endif

    if (obj->name)
        free(obj->name);
    if (obj->room_description)
        free(obj->room_description);
    if (obj->short_description)
        free(obj->short_description);
    if (obj->look_description)
        free(obj->look_description);
    if (obj->ex_description)
        free_ex_descriptions(obj->ex_description);
}

/*
 * For object instances that are not the prototype.
 */
void free_object_strings_proto(struct obj_data *obj) {
    int robj_num = GET_OBJ_RNUM(obj);

    if (obj->name && obj->name != obj_proto[robj_num].name)
        free(obj->name);
    if (obj->room_description && obj->room_description != obj_proto[robj_num].room_description)
        free(obj->room_description);
    if (obj->short_description && obj->short_description != obj_proto[robj_num].short_description)
        free(obj->short_description);
    if (obj->look_description && obj->look_description != obj_proto[robj_num].look_description)
        free(obj->look_description);
    if (obj->ex_description) {
        struct extra_descr_data *thised, *plist, *next_one; /* O(horrible) */
        int ok_key, ok_desc, ok_item;
        for (thised = obj->ex_description; thised; thised = next_one) {
            next_one = thised->next;
            for (ok_item = ok_key = ok_desc = 1, plist = obj_proto[robj_num].ex_description; plist; plist = plist->next) {
                if (plist->keyword == thised->keyword)
                    ok_key = 0;
                if (plist->description == thised->description)
                    ok_desc = 0;
                if (plist == thised)
                    ok_item = 0;
            }
            if (thised->keyword && ok_key)
                free(thised->keyword);
            if (thised->description && ok_desc)
                free(thised->description);
            if (ok_item)
                free(thised);
        }
    }
}

void copy_object_strings(struct obj_data *to, struct obj_data *from) {
    to->name = from->name ? strdup(from->name) : nullptr;
    to->room_description = from->room_description ? strdup(from->room_description) : nullptr;
    to->short_description = from->short_description ? strdup(from->short_description) : nullptr;
    to->look_description = from->look_description ? strdup(from->look_description) : nullptr;

    if (from->ex_description)
        copy_ex_descriptions(&to->ex_description, from->ex_description);
    else
        to->ex_description = nullptr;
}

int copy_object(struct obj_data *to, struct obj_data *from) {
    free_object_strings(to);
    return copy_object_main(to, from, true);
}

int copy_object_preserve(struct obj_data *to, struct obj_data *from) {
    return copy_object_main(to, from, false);
}

static int copy_object_main(struct obj_data *to, struct obj_data *from, int free_object) {
    *to = *from;
    copy_object_strings(to, from);
    return true;
}

int delete_object(obj_rnum rnum) {
    obj_rnum i;
    zone_rnum zrnum;
    struct obj_data *obj, *tmp;
    int shop, j, zone, cmd_no;

    if (!obj_proto.count(rnum))
        return NOTHING;

    obj = &obj_proto[rnum];

    zrnum = real_zone_by_thing(rnum);

    /* This is something you might want to read about in the logs. */
    basic_mud_log("GenOLC: delete_object: Deleting object #%d (%s).", GET_OBJ_VNUM(obj), obj->short_description);

    for (tmp = object_list; tmp; tmp = tmp->next) {
        if (tmp->vn != obj->vn)
            continue;

        /* extract_obj() will just axe contents. */
        if (tmp->contents) {
            struct obj_data *this_content, *next_content;
            for (this_content = tmp->contents; this_content; this_content = next_content) {
                next_content = this_content->next_content;
                if (IN_ROOM(tmp)) {
                    /* Transfer stuff from object to room. */
                    obj_from_obj(this_content);
                    obj_to_room(this_content, IN_ROOM(tmp));
                } else if (tmp->worn_by || tmp->carried_by) {
                    /* Transfer stuff from object to person inventory. */
                    obj_from_char(this_content);
                    obj_to_char(this_content, tmp->carried_by);
                } else if (tmp->in_obj) {
                    /* Transfer stuff from object to containing object. */
                    obj_from_obj(this_content);
                    obj_to_obj(this_content, tmp->in_obj);
                }
            }
        }
        /* Remove from object_list, etc. - handles weight changes, and similar. */
        extract_obj(tmp);
    }

    /* Make sure all are removed. */

    assert(get_vnum_count(objectVnumIndex, rnum) == 0);
    obj_proto.erase(rnum);
    obj_index.erase(rnum);
    save_objects(zrnum);

    return rnum;
}

void obj_affected_type::deserialize(const nlohmann::json &j) {
    if(j.count("location")) location = j["location"];
    if(j.count("modifier")) modifier = j["modifier"];
    if(j.count("specific")) specific = j["specific"];
}

obj_affected_type::obj_affected_type(const nlohmann::json &j) {
    deserialize(j);
}

nlohmann::json obj_affected_type::serialize() {
    nlohmann::json j;

    if(location) j["location"] = location;
    if(modifier != 0.0) j["modifier"] = modifier;
    if(specific) j["specific"] = specific;

    return j;
}

nlohmann::json obj_data::serializeBase() {
    auto j = serializeUnit();

    for(auto i = 0; i < NUM_OBJ_VAL_POSITIONS; i++) {
        if(value[i]) j["value"].push_back(std::make_pair(i, value[i]));
    }

    if(type_flag) j["type_flag"] = type_flag;
    if(level) j["level"] = level;

    for(auto i = 0; i < wear_flags.size(); i++)
        if(wear_flags.test(i)) j["wear_flags"].push_back(i);

    for(auto i = 0; i < extra_flags.size(); i++)
        if(extra_flags.test(i)) j["extra_flags"].push_back(i);

    if(weight != 0.0) j["weight"] = weight;
    if(cost) j["cost"] = cost;
    if(cost_per_day) j["cost_per_day"] = cost_per_day;

    for(auto i = 0; i < bitvector.size(); i++)
        if(bitvector.test(i)) j["bitvector"].push_back(i);

    for(auto & i : affected) {
        if(i.location == APPLY_NONE) continue;
        j["affected"].push_back(i.serialize());
    }

    return j;
}

nlohmann::json obj_data::serializeInstance() {
    auto j = serializeBase();
    if(id == -1) {
        id = nextObjID();
        generation = time(nullptr);
        check_unique_id(this);
        add_unique_id(this);
    }

    if(generation) j["generation"] = generation;

    if(script && script->global_vars) {
        j["dgvariables"] = serializeVars(script->global_vars);
    }

    if(world.contains(room_loaded)) j["room_loaded"] = room_loaded;

    return j;
}

nlohmann::json obj_data::serializeProto() {
    auto j = serializeBase();

    for(auto p : proto_script) {
        if(trig_index.contains(p)) j["proto_script"].push_back(p);
    }

    return j;
}

void obj_data::deserializeBase(const nlohmann::json &j) {
    deserializeUnit(j);

    if(j.contains("value")) {
        for(auto & i : j["value"]) {
            value[i[0].get<int>()] = i[1];
        }
    }

    if(j.contains("type_flag")) type_flag = j["type_flag"];
    if(j.contains("level")) level = j["level"];

    if(j.contains("wear_flags")) {
        for(auto & i : j["wear_flags"]) {
            wear_flags.set(i.get<int>());
        }
    }

    if(j.contains("extra_flags")) {
        for(auto & i : j["extra_flags"]) {
            extra_flags.set(i.get<int>());
        }
    }

    if(j.contains("weight")) weight = j["weight"];
    if(j.contains("cost")) cost = j["cost"];
    if(j.contains("cost_per_day")) cost_per_day = j["cost_per_day"];

    if(j.contains("bitvector")) {
        for(auto & i : j["bitvector"]) {
            bitvector.set(i.get<int>());
        }
    }

    if(j.contains("affected")) {
        int counter = 0;
        for(auto & i : j["affected"]) {
            affected[counter].deserialize(i);
            counter++;
        }
    }

}


void obj_data::deserializeProto(const nlohmann::json& j) {
    deserializeBase(j);

    if(j.contains("proto_script")) {
        for(auto p : j["proto_script"]) proto_script.emplace_back(p.get<trig_vnum>());
    }
}


obj_data::obj_data(const nlohmann::json &j) : obj_data() {
    deserializeProto(j);

    if ((GET_OBJ_TYPE(this) == ITEM_PORTAL || \
       GET_OBJ_TYPE(this) == ITEM_HATCH) && \
       (!GET_OBJ_VAL(this, VAL_DOOR_DCLOCK) || \
        !GET_OBJ_VAL(this, VAL_DOOR_DCHIDE))) {
        GET_OBJ_VAL(this, VAL_DOOR_DCLOCK) = 20;
        GET_OBJ_VAL(this, VAL_DOOR_DCHIDE) = 20;
    }

    GET_OBJ_SIZE(this) = SIZE_MEDIUM;

/* check to make sure that weight of containers exceeds curr. quantity */
    if (GET_OBJ_TYPE(this) == ITEM_DRINKCON ||
        GET_OBJ_TYPE(this) == ITEM_FOUNTAIN) {
        if (GET_OBJ_WEIGHT(this) < GET_OBJ_VAL(this, 1))
            GET_OBJ_WEIGHT(this) = GET_OBJ_VAL(this, 1) + 5;
    }
    /* *** make sure portal objects have their timer set correctly *** */
    if (GET_OBJ_TYPE(this) == ITEM_PORTAL) {
        GET_OBJ_TIMER(this) = -1;
    }
    
}

std::optional<vnum> obj_data::getMatchingArea(const std::function<bool(const area_data &)>& f) {
    if(auto room = getAbsoluteRoom(); room) {
        return room->getMatchingArea(f);
    }
    return std::nullopt;
}

void obj_data::activate() {
    if(active) {
        logger->warn("Attempted to activate an already active item.");
        return;
    }
    active = true;
    next = object_list;
    object_list = this;

    if(script) script->activate();

    if(obj_proto.contains(vn)) {
        insert_vnum(objectVnumIndex, this);
    }

    if(contents) activateContents();
}

void obj_data::deactivate() {
    if(!active) return;
    active = false;
    struct obj_data *temp;
    REMOVE_FROM_LIST(this, object_list, next, temp);

    if(obj_proto.contains(vn)) {
        erase_vnum(objectVnumIndex, this);
    }

    if(script && script->trig_list) {
        struct trig_data *next_trig;
        for (auto trig = TRIGGERS(script); trig; trig = next_trig) {
            next_trig = trig->next;
            extract_trigger(trig);
        }
        TRIGGERS(script) = nullptr;
    }
    if(contents) deactivateContents();
}

void obj_data::deserializeInstance(const nlohmann::json &j, bool isActive) {
    deserializeBase(j);

    if(j.contains("generation")) generation = j["generation"];
    check_unique_id(this);
    add_unique_id(this);

    if(j.contains("dgvariables")) {
        if(!script) script = new script_data(this);
        deserializeVars(&script->global_vars, j["dgvariables"]);
    }

    if(j.contains("room_loaded")) room_loaded = j["room_loaded"];

    auto proto = obj_proto.find(vn);
    if(proto != obj_proto.end()) {
        proto_script = proto->second.proto_script;
    }

    if(isActive) activate();

}

void obj_data::deserializeContents(const nlohmann::json &j, bool isActive) {
    for(const auto& jo : j) {
        auto obj = new obj_data();
        obj->deserializeInstance(jo, isActive);
        obj_to_obj(obj, this);
    }
}

int obj_data::getAffectModifier(int location, int specific) {
    int modifier = 0;
    for(auto &aff : affected) {
        if(aff.location == location && (specific == -1 || aff.specific == specific)) {
            modifier += aff.modifier;
        }
    }
    return modifier;
}

weight_t obj_data::getWeight() {
    return weight;
}

weight_t obj_data::getTotalWeight() {
    return getWeight() + getInventoryWeight() + (sitting ? sitting->getTotalWeight() : 0);
}

std::string obj_data::getUID(bool active) {
    return fmt::format("#O{}:{}{}", id, generation, active ? "" : "!");
}

bool obj_data::isActive() {
    return active;
}

std::string obj_data::serializeLocation() {
    if(in_obj) {
        return in_obj->getUID();
    } else if(carried_by) {
        return carried_by->getUID();
    } else if(worn_by) {
        return worn_by->getUID();
    } else if(world.contains(in_room)) {
        return world[in_room].getUID();
    } else {
        return ""; // this should NEVER happen!
    }
}

nlohmann::json obj_data::serializeRelations() {
    auto j = nlohmann::json::object();

    if(posted_to) j["posted_to"] = posted_to->getUID();
    if(fellow_wall) j["fellow_wall"] = fellow_wall->getUID();

    return j;
}

void obj_data::deserializeLocation(const std::string& txt, int16_t slot) {
    auto check = resolveUID(txt);
    if(!check) return;
    auto idx = check->index();
    if(idx == 0) {
        auto &r = std::get<0>(*check);
        obj_to_room(this, r->vn);
    } else if(idx == 1) {
        obj_to_obj(this, std::get<1>(*check));
    } else if(idx == 2) {
        auto &c = std::get<2>(*check);
        auto_equip(c, this, slot+1);
    }
}

void obj_data::deserializeRelations(const nlohmann::json& j) {
    if(j.contains("posted_to")) {
        auto check = resolveUID(j["posted_to"]);
        if(check) posted_to = std::get<1>(*check);
    }
    if(j.contains("fellow_wall")) {
        auto check = resolveUID(j["fellow_wall"]);
        if(check) fellow_wall = std::get<1>(*check);
    }
}

void obj_data::save() {
    if(id == NOTHING) return;
    dirty_items.insert(id);
}

bool obj_data::isProvidingLight() {
    return GET_OBJ_TYPE(this) == ITEM_LIGHT && GET_OBJ_VAL(this, VAL_LIGHT_HOURS);
}

struct room_data* obj_data::getRoom() {
    auto roomFound = world.find(in_room);
    if(roomFound != world.end()) return &roomFound->second;
    return nullptr;
}

struct room_data* obj_data::getAbsoluteRoom() {
    if(auto room = getRoom(); room) {
        return room;
    }
    if(in_obj) return in_obj->getAbsoluteRoom();
    else if(carried_by) return carried_by->getRoom();
    else if(worn_by) return worn_by->getRoom();
    return nullptr;
}

double obj_data::currentGravity() {
    if(auto room = getAbsoluteRoom(); room) {
        return room->getGravity();
    }
    return 1.0;
}

bool obj_data::isWorking() {
    return !(OBJ_FLAGGED(this, ITEM_BROKEN) || OBJ_FLAGGED(this, ITEM_FORGED));
}

void obj_data::clearLocation() {
    if(in_obj) obj_from_obj(this);
    else if(carried_by) obj_from_char(this);
    else if(worn_by) unequip_char(worn_by, worn_on);
    else if(world.contains(in_room)) obj_from_room(this);
}