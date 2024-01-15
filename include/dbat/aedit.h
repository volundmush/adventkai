#pragma once

#include "structs.h"

extern void aedit_disp_menu(struct descriptor_data *d);

extern void aedit_parse(struct descriptor_data *d, char *arg);

extern void aedit_setup_new(struct descriptor_data *d);

extern void aedit_setup_existing(struct descriptor_data *d, int real_num);

extern void aedit_save_to_disk(struct descriptor_data *d);

extern void aedit_save_internally(struct descriptor_data *d);

extern int aedit_find_command(const char *txt);

// commands
extern ACMD(do_oasis_aedit);
