//
// Created by volund on 10/20/21.
//
#pragma once

#include "structs.h"

extern void hedit_parse(struct descriptor_data *, char *);

static void hedit_setup_new(struct descriptor_data *);

static void hedit_setup_existing(struct descriptor_data *, int);

static void hedit_save_to_disk(struct descriptor_data *);

static void hedit_save_internally(struct descriptor_data *);

extern void hedit_string_cleanup(struct descriptor_data *, int);

extern ACMD(do_oasis_hedit);

extern ACMD(do_helpcheck);

extern ACMD(do_hindex);
