//
// Created by volund on 10/20/21.
//
#pragma once

#include "structs.h"
#include "house.h"

extern void hsedit_setup_new(struct descriptor_data *d);

extern void hsedit_setup_existing(struct descriptor_data *d, int real_num);

extern void hsedit_save_internally(struct descriptor_data *d);

extern void hsedit_save_to_disk();

extern void hsedit_disp_type_menu(struct descriptor_data *d);

extern void hsedit_disp_menu(struct descriptor_data *d);

extern void hsedit_parse(struct descriptor_data *d, char *arg);

extern void hsedit_disp_flags_menu(struct descriptor_data *d);

extern void hsedit_disp_val0_menu(struct descriptor_data *d);

extern void hsedit_disp_val1_menu(struct descriptor_data *d);

extern void hsedit_disp_val2_menu(struct descriptor_data *d);

extern void hsedit_disp_val3_menu(struct descriptor_data *d);

extern void free_house(struct house_control_rec *house);
