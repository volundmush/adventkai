#pragma once

#include "sysdep.h"
#include "typestubs.h"

extern bool tech_handle_android_absorb(char_data *ch, char_data *vict);

extern bool tech_handle_zanzoken(char_data *ch, char_data *vict, const std::string &name);

extern void tech_handle_posmodifier(char_data *vict, int &pry, int &blk, int &dge, int &prob);

extern void tech_handle_fireshield(char_data *ch, char_data *vict, const std::string &part = "body");

extern bool tech_handle_targeting(char_data *ch, char *arg, char_data **vict, obj_data **obj);

extern bool tech_handle_charge(char_data *ch, char *arg, double minimum, double *attperc);

extern void tech_handle_crashdown(char_data *ch, char_data *vict);
