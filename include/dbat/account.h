#pragma once
#include "structs.h"

extern std::map<vnum, account_data> accounts;

struct account_data *findAccount(const std::string &name);