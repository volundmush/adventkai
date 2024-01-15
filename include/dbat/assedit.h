#pragma once

#include "structs.h"

extern void assedit_setup(struct descriptor_data *d, int number);

extern void assedit_disp_menu(struct descriptor_data *d);

extern void assedit_delete(struct descriptor_data *d);

extern void assedit_edit_extract(struct descriptor_data *d);

extern void assedit_edit_inroom(struct descriptor_data *d);

extern void nodigit(struct descriptor_data *d);

extern void assedit_parse(struct descriptor_data *d, char *arg);

extern ACMD(do_assedit);
