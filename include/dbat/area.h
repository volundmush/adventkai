#pragma once
#include "structs.h"

void save_areas();
void load_areas();

vnum getNextAreaVnum();

ACMD(do_arlist);
ACMD(do_arcreate);
ACMD(do_ardelete);
ACMD(do_arset);
ACMD(do_arbind);