#include "dbat/structs.h"
#include "dbat/db.h"
#include "dbat/comm.h"
#include "dbat/utils.h"

vnum area_data::getNextID() {
    vnum vn = 0;
    while(areas.contains(vn)) vn++;
    return vn;
}

bool area_data::isPlanet(const area_data& a) {
    return a.type == AreaType::CelestialBody;
}

area_data::area_data(const nlohmann::json& j) {
    if(j.contains("vn")) vn = j["vn"];
    if(j.contains("name")) name = j["name"];
    if(j.contains("rooms")) {
        // "rooms" is an array of numbers. We need to iterate them and fill the rooms set.
        for(auto& room : j["rooms"]) {
            rooms.insert(room.get<vnum>());
        }
    }
    if(j.contains("gravity")) gravity = j["gravity"];
    if(j.contains("parent")) parent = j["parent"];
    if(j.contains("type")) type = j["type"];
    if(j.contains("extraVn")) extraVn = j["extraVn"];
    if(j.contains("ether")) ether = j["ether"];
}

nlohmann::json area_data::serialize() {
    nlohmann::json j;
    j["vn"] = vn;
    if(!name.empty()) j["name"] = name;
    for(auto r : rooms) j["rooms"].push_back(r);
    if(gravity) j["gravity"] = gravity.value();
    if(parent) j["parent"] = parent.value();
    j["type"] = type;
    if(extraVn) j["extraVn"] = extraVn.value();
    if(ether) j["ether"] = ether;

    return j;
}

static std::string areaTypeName(AreaType a) {
    switch(a) {
        case AreaType::Dimension:
            return "Dimen";
        case AreaType::CelestialBody:
            return "ClBod";
        case AreaType::Region:
            return "Regn";
        case AreaType::Structure:
            return "Struc";
        case AreaType::Vehicle:
            return "Vehic";

    }
}

std::optional<room_vnum> area_data::getLaunchDestination() {
    switch(type) {
        case AreaType::Dimension:
            return {};
        case AreaType::CelestialBody:
            return extraVn;
        case AreaType::Region:
            if(parent && areas.contains(parent.value())) {
                auto& p = areas[parent.value()];
                return p.getLaunchDestination();
            }
            return {};
        case AreaType::Structure:
        case AreaType::Vehicle: {
            if(extraVn ) {
                if(auto obj = get_last_inserted(objectVnumIndex, extraVn.value()); obj) {
                    auto r = obj->getRoom();
                    if(r) return r->vn;
                }
            }
        }
        return {};
    }
    return {};
}


static void render_area_line(struct char_data *ch, const area_data &a) {
    auto atype = areaTypeName(a.type);
    auto colorcount = countColors(a.name);

    auto format = "%-5d %-"+std::to_string(20+colorcount)+"s %-5d %-7d %-6d %-5s %-6d %-5d %-5d\r\n";
    send_to_char(ch, format.c_str(),
                 a.vn, a.name.c_str(), a.rooms.size(), a.children.size(), a.parent.value_or(NOTHING),
                 atype.c_str(), a.extraVn.value_or(NOTHING), a.flags.test(AREA_MOON), a.flags.test(AREA_ETHER));
}

static void list_areas(struct char_data *ch, std::optional<vnum> parent) {
    static std::string header =
        "VN    Name                 Rooms Childrn Parent Type  ExtrVN Moon  Ether\r\n"
        "----- -------------------- ----- ------- ------ ----- ------ ----- -----\r\n";

    if(parent) {
        if(!areas.contains(parent.value())) {
            send_to_char(ch, "No such area.\r\n");
            return;
        }
        send_to_char(ch, header.c_str());
        auto &a = areas[parent.value()];
        for(auto v : a.children) {
            auto f = areas.find(v);
            if(f == areas.end()) continue;
            render_area_line(ch, f->second);
        }
    } else {
        send_to_char(ch, header.c_str());
        for(auto& [vn, area] : areas) {
            if(area.parent) continue;
            render_area_line(ch, area);
        }
    }
}

ACMD(do_arlist) {
    if(!argument || !*argument) {
        list_areas(ch, std::nullopt);
        return;
    }
    vnum vn = atoi(argument);
    auto a = areas.find(vn);
    if(a == areas.end()) {
        send_to_char(ch, "No such area.\r\n");
        return;
    }
    send_to_char(ch, "Showing children of %s.\r\n", a->second.name.c_str());
    list_areas(ch, a->first);
}