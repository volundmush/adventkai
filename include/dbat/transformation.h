#pragma once
#include "structs.h"

namespace trans {
    extern std::string getName(struct char_data *ch, FormID form);
    extern std::string getAbbr(struct char_data *ch, FormID form);

    extern bool unlock(struct char_data *ch, FormID form);

    extern double getModifier(struct char_data *ch, int location, int specific = 0);

    extern double getStaminaDrain(struct char_data *ch, FormID form, bool upkeep = false);

    extern std::optional<int> getAppearanceMod(struct char_data *ch, FormID form, CharAppearance mode);

    extern void handleEchoTransform(struct char_data *ch, FormID form);
    extern void handleEchoRevert(struct char_data *ch, FormID form);

    extern void displayForms(struct char_data *ch);
    extern int64_t getRequiredPL(struct char_data* ch, FormID trans);

    extern bool blockRevertDisallowed(struct char_data *ch, FormID form);

    extern std::optional<FormID> findForm(struct char_data *ch, const std::string& arg);

    extern void gamesys_transform(uint64_t heartPulse, double deltaTime);
    extern void gamesys_oozaru(uint64_t heartPulse, double deltaTime);

}