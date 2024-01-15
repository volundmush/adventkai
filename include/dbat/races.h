#pragma once

#include "structs.h"

// global variables
extern const struct guild_info_type guild_info[6];

// functions
extern void racial_body_parts(struct char_data *ch);

extern void set_height_and_weight_by_race(struct char_data *ch);

extern int invalid_race(struct char_data *ch, struct obj_data *obj);

// C++ conversion

namespace race {

    extern std::string getName(RaceID id);
    extern std::string getAbbr(RaceID id);
    extern bool isPlayable(RaceID id);
    extern std::vector<RaceID> getAll();
    extern std::vector<RaceID> getPlayable();
    extern std::set<int> getValidSexes(RaceID id);
    extern bool isValidMimic(RaceID id);
    extern bool isPeople(RaceID id);
    extern bool hasTail(RaceID id);
    extern int getRPPCost(RaceID id);
    extern int getRPPRefund(RaceID id);
    extern int64_t getSoftCap(RaceID id, int level);
    extern bool isSenseable(RaceID id);
    extern int getSize(RaceID id);
    extern bool exists(RaceID id);

    extern double getModifier(struct char_data *ch, int location, int specific = 0);

    extern std::vector<RaceID> filterRaces(std::function<bool(RaceID)> func);
    extern std::optional<RaceID> findRace(const std::string& arg, std::function<bool(RaceID)> func);

}
