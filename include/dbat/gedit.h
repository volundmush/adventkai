#pragma once

#include "structs.h"
#include "guild.h"

extern void gedit_setup_new(struct descriptor_data *d);

extern void gedit_setup_existing(struct descriptor_data *d, int rgm_num);

extern void gedit_parse(struct descriptor_data *d, char *arg);

extern void gedit_disp_menu(struct descriptor_data *d);

extern void gedit_no_train_menu(struct descriptor_data *d);

extern void gedit_save_internally(struct descriptor_data *d);

extern void gedit_save_to_disk(int num);

extern void copy_guild(struct guild_data *tgm, struct guild_data *fgm);

extern void free_guild(struct guild_data *guild);

extern void gedit_modify_string(std::string &str, char *new_g);
