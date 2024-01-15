/************************************************************************
 * Generic OLC Library - Rooms / genwld.c			v1.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/genwld.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/genolc.h"
#include "dbat/shop.h"
#include "dbat/dg_olc.h"


/*
 * This function will copy the strings so be sure you free your own
 * copies of the description, title, and such.
 */
room_rnum add_room(struct room_data *room) {
    struct char_data *tch;
    struct obj_data *tobj;
    vnum j, found = false;
    room_rnum i;

    if (!room)
        return NOWHERE;

    if (world.contains(room->vn)) {
        auto &ro = world[room->vn];
        if (SCRIPT(&ro))
            extract_script(&ro, WLD_TRIGGER);
        tch = ro.people;
        tobj = ro.contents;
        copy_room(&ro, room);
        ro.people = tch;
        ro.contents = tobj;
        ro.save();
        basic_mud_log("GenOLC: add_room: Updated existing room #%d.", room->vn);
        return i;
    }

    auto &r = world[room->vn];
    r = *room;
    basic_mud_log("GenOLC: add_room: Added room %d.", room->vn);
    r.save();

    /*
     * Return what array entry we placed the new room in.
     */
    return found;
}

/* -------------------------------------------------------------------------- */

int delete_room(room_rnum rnum) {
    room_rnum i;
    int j;
    struct char_data *ppl, *next_ppl;
    struct obj_data *obj, *next_obj;
    struct room_data *room;

    if (!world.count(rnum))    /* Can't delete void yet. */
        return false;

    room = &world[rnum];
    room->save();

    /* This is something you might want to read about in the logs. */
    basic_mud_log("GenOLC: delete_room: Deleting room #%d (%s).", room->vn, room->name);

    if (r_mortal_start_room == rnum) {
        basic_mud_log("WARNING: GenOLC: delete_room: Deleting mortal start room!");
        r_mortal_start_room = 0;    /* The Void */
    }
    if (r_immort_start_room == rnum) {
        basic_mud_log("WARNING: GenOLC: delete_room: Deleting immortal start room!");
        r_immort_start_room = 0;    /* The Void */
    }
    if (r_frozen_start_room == rnum) {
        basic_mud_log("WARNING: GenOLC: delete_room: Deleting frozen start room!");
        r_frozen_start_room = 0;    /* The Void */
    }

    /*
     * Dump the contents of this room into the Void.  We could also just
     * extract the people, mobs, and objects here.
     */
    for (obj = world[rnum].contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        obj_from_room(obj);
        obj_to_room(obj, 0);
    }
    for (ppl = world[rnum].people; ppl; ppl = next_ppl) {
        next_ppl = ppl->next_in_room;
        char_from_room(ppl);
        char_to_room(ppl, 0);
    }

    if (SCRIPT(room))
        extract_script(room, WLD_TRIGGER);
    free_proto_script(room, WLD_TRIGGER);

    /*
     * Change any exit going to this room to go the void.
     * Also fix all the exits pointing to rooms above this.
     */

    for(auto &r : world) {
        for (j = 0; j < NUM_OF_DIRS; j++) {
            auto &e = r.second.dir_option[j];
            if (!e || e->to_room != rnum)
                continue;
            if ((!e->keyword || !*e->keyword) &&
                (!e->general_description || !*e->general_description)) {
                /* no description, remove exit completely */
                if (e->keyword)
                    free(e->keyword);
                if (e->general_description)
                    free(e->general_description);
                free(e);
                e = nullptr;
            } else {
                /* description is set, just point to nowhere */
                e->to_room = NOWHERE;
            }
        }

    };

    /*
     * Remove this room from all shop lists.
     */
    for (auto &[i, sh] : shop_index) {
        sh.in_room.erase(rnum);
    }

    world.erase(rnum);
    return true;
}


int save_rooms(zone_rnum zone_num) {
    auto z = zone_table.find(zone_num);

    if (z == zone_table.end()) {
        basic_mud_log("SYSERR: GenOLC: save_rooms: Invalid zone number %d passed!", zone_num);
        return false;
    }

    dirty_rooms.insert(z->second.rooms.begin(), z->second.rooms.end());
    z->second.save_rooms();
    return true;
}

int copy_room(struct room_data *to, struct room_data *from) {
    free_room_strings(to);
    *to = *from;
    copy_room_strings(to, from);

    /* Don't put people and objects in two locations.
       Am thinking this shouldn't be done here... */
    from->people = nullptr;
    from->contents = nullptr;

    return true;
}

/* -------------------------------------------------------------------------- */

/*
 * Copy strings over so bad things don't happen.  We do not free the
 * existing strings here because copy_room() did a shallow copy previously
 * and we'd be freeing the very strings we're copying.  If this function
 * is used elsewhere, be sure to free_room_strings() the 'dest' room first.
 */
int copy_room_strings(struct room_data *dest, struct room_data *source) {
    int i;

    if (dest == nullptr || source == nullptr) {
        basic_mud_log("SYSERR: GenOLC: copy_room_strings: nullptr values passed.");
        return false;
    }

    dest->look_description = str_udup(source->look_description);
    dest->name = str_udup(source->name);

    for (i = 0; i < NUM_OF_DIRS; i++) {
        if (!R_EXIT(source, i))
            continue;

        CREATE(R_EXIT(dest, i), struct room_direction_data, 1);
        *R_EXIT(dest, i) = *R_EXIT(source, i);
        if (R_EXIT(source, i)->general_description)
            R_EXIT(dest, i)->general_description = strdup(R_EXIT(source, i)->general_description);
        if (R_EXIT(source, i)->keyword)
            R_EXIT(dest, i)->keyword = strdup(R_EXIT(source, i)->keyword);
    }

    if (source->ex_description)
        copy_ex_descriptions(&dest->ex_description, source->ex_description);

    return true;
}

int free_room_strings(struct room_data *room) {
    int i;

    /* Free descriptions. */
    if (room->name)
        free(room->name);
    if (room->look_description)
        free(room->look_description);
    if (room->ex_description)
        free_ex_descriptions(room->ex_description);

    /* Free exits. */
    for (i = 0; i < NUM_OF_DIRS; i++) {
        if (room->dir_option[i]) {
            if (room->dir_option[i]->general_description)
                free(room->dir_option[i]->general_description);
            if (room->dir_option[i]->keyword)
                free(room->dir_option[i]->keyword);
            free(room->dir_option[i]);
            room->dir_option[i] = nullptr;
        }
    }

    return true;
}

room_direction_data::~room_direction_data() {
    if (general_description)
        free(general_description);
    if (keyword)
        free(keyword);
}

nlohmann::json room_direction_data::serialize() {
    nlohmann::json j;

    if(general_description && strlen(general_description)) j["general_description"] = general_description;
    if(keyword && strlen(keyword)) j["keyword"] = keyword;
    if(exit_info) j["exit_info"] = exit_info;
    if(key > 0) j["key"] = key;
	if(to_room != NOWHERE) j["to_room"] = to_room;
    if(dclock) j["dclock"] = dclock;
    if(dchide) j["dchide"] = dchide;
    if(dcskill) j["dcskill"] = dcskill;
    if(dcmove) j["dcmove"] = dcmove;
    if(failsavetype) j["failsavetype"] = failsavetype;
    if(dcfailsave) j["dcfailsave"] = dcfailsave;
    if(failroom > 0) j["failroom"] = failroom;
    if(totalfailroom > 0) j["totalfailroom"] = totalfailroom;

    return j;
}

room_direction_data::room_direction_data(const nlohmann::json &j) : room_direction_data() {
    if(j.contains("general_description")) general_description = strdup(j["general_description"].get<std::string>().c_str());
    if(j.contains("keyword")) keyword = strdup(j["keyword"].get<std::string>().c_str());
    if(j.contains("exit_info")) exit_info = j["exit_info"].get<int16_t>();
    if(j.contains("key")) key = j["key"];
    if(j.contains("to_room")) to_room = j["to_room"];
    if(j.contains("dclock")) dclock = j["dclock"];
    if(j.contains("dchide")) dchide = j["dchide"];
    if(j.contains("dcskill")) dcskill = j["dcskill"];
    if(j.contains("dcmove")) dcmove = j["dcmove"];
    if(j.contains("failsavetype")) failsavetype = j["failsavetype"];
    if(j.contains("dcfailsave")) dcfailsave = j["dcfailsave"];
    if(j.contains("failroom")) failroom = j["failroom"];
    if(j.contains("totalfailroom")) totalfailroom = j["totalfailroom"];
}

nlohmann::json room_data::serialize() {
    auto j = serializeUnit();

    if(sector_type) j["sector_type"] = sector_type;

    for(auto i = 0; i < NUM_OF_DIRS; i++) {
        if(dir_option[i]) j["dir_option"].push_back(std::make_pair(i, dir_option[i]->serialize()));
    }

    for (size_t i = 0; i < room_flags.size(); ++i) {
        if (room_flags[i]) {
            j["room_flags"].push_back(i);
        }
    }

    for(auto p :proto_script) {
        if(trig_index.contains(p)) j["proto_script"].push_back(p);
    }

    return j;
}


room_data::room_data(const nlohmann::json &j) {
    deserializeUnit(j);

    if(j.contains("sector_type")) sector_type = j["sector_type"];

    if(j.contains("dir_option")) {
        // this is an array of (<number>, <json>) pairs, with number matching the dir_option array index.
        // Thankfully we can pass the json straight into the room_direction_data constructor...
        for(auto &d : j["dir_option"]) {
            dir_option[d[0]] = new room_direction_data(d[1]);
        }
    }

    if(j.contains("room_flags")) {
        for(auto &f : j["room_flags"]) {
            room_flags.set(f.get<int>());
        }
    }

    if(j.contains("proto_script")) {
        for(auto p : j["proto_script"]) proto_script.emplace_back(p.get<trig_vnum>());
    }

    if(!proto_script.empty() || vn == 0) {
        if(!script) script = new script_data(this);
    }


}

room_data::~room_data() {
    // fields like name are handled by the base destructor...
    // we just need to clean up exits.
    for(auto d : dir_option) {
        delete d;
    }
}

std::optional<vnum> room_data::getMatchingArea(std::function<bool(const area_data &)> f) {
    std::optional<vnum> parent = area;
    while(parent) {
        auto &a = areas[parent.value()];
        if(f(a)) return parent;
        if ((a.type == AreaType::Structure || a.type == AreaType::Vehicle) && a.extraVn) {
            // we need to find the a.objectVnum in the world by scanning object_list...
            if (auto obj = get_obj_num(a.extraVn.value()); obj) {
                if(world.contains(obj->in_room)) {
                    auto &r = world[obj->in_room];
                    return r.getMatchingArea(f);
                }
            }
        }
        parent = a.parent;
    }
    return std::nullopt;
}

static bool checkGravity(const area_data &a) {
    return a.gravity.has_value();
}

double room_data::getGravity() {
    // check for a gravity generator...
    for(auto c = contents; c; c = c->next_content) {
        if(c->gravity) return c->gravity.value();
    }

    // what about area rules?
    if(std::optional<vnum> gravArea = getMatchingArea(checkGravity); gravArea) {
        auto &a = areas[gravArea.value()];
        return a.gravity.value();
    }

    // special cases here..
    if (vn >= 64000 && vn <= 64006) {
        return 100.0;
    }
    if (vn >= 64007 && vn <= 64016) {
        return 300.0;
    }
    if (vn >= 64017 && vn <= 64030) {
        return 500.0;
    }
    if (vn >= 64031 && vn <= 64048) {
        return 1000.0;
    }
    if (vn >= 64049 && vn <= 64070) {
        return 5000.0;
    }
    if (vn >= 64071 && vn <= 64096) {
        return 10000.0;
    }
    if (vn == 64097) {
        return 1000.0;
    }

    return 1.0;
}

void room_data::deserializeContents(const nlohmann::json& j, bool isActive) {
    for(const auto& jo : j) {
        auto obj = new obj_data();
        obj->deserializeInstance(jo, isActive);
        // Since we're deserializing items in a room, we'll just add them straight to it.
        obj_to_room(obj, vn);
    }
}

std::string room_data::getUID(bool active) {
    return fmt::format("#R{}:{}{}", vn, generation, active ? "" : "!");
}

bool room_data::isActive() {
    return world.contains(vn);
}


void room_data::save() {
    dirty_rooms.insert(vn);
}

int room_data::getDamage() {
    return dmg;
}

int room_data::setDamage(int amount) {
    auto before = dmg;
    dmg = std::clamp<int>(amount, 0, 100);
    // if(dmg != before) save();
    return dmg;
}

int room_data::modDamage(int amount) {
    return setDamage(dmg + amount);
}

struct room_data* room_direction_data::getDestination() {
    auto found = world.find(to_room);
    if(found != world.end()) return &found->second;
    return nullptr;
}


bool room_data::isSunken() {
    return sector_type == SECT_UNDERWATER || geffect < 0;
}

std::optional<room_vnum> room_data::getLaunchDestination() {
    if(!area) return NOWHERE;
    if(!areas.contains(area.value())) return NOWHERE;
    auto &a = areas[area.value()];
    return a.getLaunchDestination();
}

std::list<char_data *> room_data::getPeople() {
    std::list<struct char_data*> out;
    for(auto c = people; c; c = c->next_in_room) {
        out.push_back(c);
    }
    return out;
}

static const std::set<int> inside_sectors = {SECT_INSIDE, SECT_UNDERWATER, SECT_IMPORTANT, SECT_SHOP, SECT_SPACE};

MoonCheck room_data::checkMoon() {
    for(auto f : {ROOM_INDOORS, ROOM_UNDERGROUND, ROOM_SPACE}) if(room_flags.test(f)) return MoonCheck::NoMoon;
    if(inside_sectors.contains(sector_type)) return MoonCheck::NoMoon;
    auto check_planet = getMatchingArea(area_data::isPlanet);
    if(!check_planet) return MoonCheck::NoMoon;
    auto &area = areas[*check_planet];
    if(!area.flags.test(AREA_MOON)) return MoonCheck::NoMoon;

    return MOON_TIMECHECK() ? MoonCheck::Full : MoonCheck::NotFull;

}