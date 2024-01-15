#pragma once

#include "structs.h"

// global variables
extern struct player_index_element *player_table;

extern void remove_player(int pfilepos);

extern DebugMap<int64_t, player_data> players;

struct char_data *findPlayer(const std::string& name);

void build_player_index();

OpResult<> validate_pc_name(const std::string& name);