/* ************************************************************************
*   File: constants.h                                   Part of CircleMUD *
*  Usage: Header file for constants.                                      *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  tbaMUD is based on CircleMUD and DikuMUD, Copyright (C) 1990, 1991.    *
************************************************************************ */
#pragma once

#include "structs.h"
#include "interpreter.h"    /* alias_data */

extern const char *circlemud_version;
extern const char *oasisolc_version;
extern const char *ascii_pfiles_version;
extern const char *attack_names_comp[];
extern const char *attack_names[];
extern const int attack_skills[];

extern const char *dirs[NUM_OF_DIRS + 1];
extern const char *abbr_dirs[NUM_OF_DIRS + 1];
extern const char *room_bits[NUM_ROOM_FLAGS + 1];
extern const char *exit_bits[NUM_EXIT_FLAGS + 1];
extern const char *sector_types[NUM_ROOM_SECTORS + 1];
extern const char *genders[NUM_SEX + 1];
extern const char *position_types[NUM_POSITIONS + 1];
extern const char *player_bits[NUM_PLR_FLAGS + 1];
extern const char *action_bits[NUM_MOB_FLAGS + 1];
extern const char *preference_bits[NUM_PRF_FLAGS + 1];
extern const char *affected_bits[NUM_AFF_FLAGS + 1];
extern const char *connected_types[NUM_CON_TYPES + 1];
extern const char *wear_where[NUM_WEARS + 1];
extern const char *equipment_types[NUM_WEARS + 1];
extern const char *item_types[NUM_ITEM_TYPES + 1];
extern const char *wear_bits[NUM_ITEM_WEARS + 1];
extern const char *extra_bits[NUM_ITEM_FLAGS + 1];
extern const char *apply_types[NUM_APPLIES + 1];
extern const char *container_bits[NUM_CONT_FLAGS + 1];
extern const char *drinks[NUM_LIQ_TYPES + 1];
extern const char *drinknames[NUM_LIQ_TYPES + 1];
extern const char *color_liquid[NUM_LIQ_TYPES + 1];
extern const char *fullness[NUM_FULLNESS + 1];
extern const char *weekdays[NUM_WEEK_DAYS];
extern const char *month_name[NUM_MONTHS];
extern const char *trig_types[NUM_MTRIG_TYPES + 1];
extern const char *otrig_types[NUM_OTRIG_TYPES + 1];
extern const char *wtrig_types[NUM_WTRIG_TYPES + 1];
extern const char *size_names[NUM_SIZES + 1];
extern const char *domains[NUM_DOMAINS + 1];
extern const char *schools[NUM_SCHOOLS + 1];
extern int rev_dir[NUM_OF_DIRS];
extern int movement_loss[NUM_ROOM_SECTORS];
extern const char *admin_flag_names[];
extern int drink_aff[NUM_LIQ_TYPES][NUM_CONDITIONS];
extern size_t room_bits_count;
extern size_t action_bits_count;
extern size_t affected_bits_count;
extern size_t extra_bits_count;
extern size_t wear_bits_count;
extern const char *AssemblyTypes[MAX_ASSM + 1];
extern const char *alignments[NUM_ALIGNS + 1];
extern const char *admin_level_names[ADMLVL_IMPL + 2];
extern const struct aging_data racial_aging_data[NUM_RACES];
extern const char *spell_schools[NUM_SCHOOLS + 1];
extern const char *cchoice_names[NUM_COLOR + 1];
extern const char *creation_methods[NUM_CREATION_METHODS + 1];
extern const char *zone_bits[NUM_ZONE_FLAGS + 1];
extern const char *history_types[NUM_HIST + 1];
extern const char *weapon_type[MAX_WEAPON_TYPES + 2];
extern const char *armor_type[MAX_ARMOR_TYPES + 1];
extern const char *wield_names[NUM_WIELD_NAMES + 1];
extern const char *material_names[NUM_MATERIALS + 1];
extern const char *admin_flags[NUM_ADMFLAGS + 1];
extern const char *crit_type[NUM_CRIT_TYPES + 1];
extern const char *hairl_types[];
extern const char *eye_types[];
extern const char *FHA_types[];
extern const char *hairc_types[];
extern const char *hairs_types[];
extern const char *skin_types[];
extern const char *aura_types[];
extern const char *npc_personality[MAX_PERSONALITIES + 1];
extern const char *song_types[];
extern const char *list_bonus[];
extern const int list_bonus_cost[];

extern const std::unordered_map<std::string, CharAttribute> CharAttributeNames;
extern const std::unordered_map<std::string, CharTrain> CharTrainNames;
extern const std::unordered_map<std::string, CharAppearance> CharAppearanceNames;
extern const std::unordered_map<std::string, CharAlign> CharAlignNames;
extern const std::unordered_map<std::string, CharMoney> CharMoneyNames;
extern const std::unordered_map<std::string, CharStat> CharStatNames;