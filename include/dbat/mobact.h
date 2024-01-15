//
// Created by volund on 10/20/21.
//
#pragma once

#include "structs.h"


// functions
extern void remember(struct char_data *ch, struct char_data *victim);

extern void mobile_activity(uint64_t heartPulse, double deltaTime);

extern void forget(struct char_data *ch, struct char_data *victim);

extern void mob_taunt(struct char_data *ch);
