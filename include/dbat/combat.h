/* ************************************************************************
*  File: combat.h                    Part of Dragonball Advent Truth      *
*  Usage: Combat utilities and common functions for act.offensive.c and   *
*  act.attack.c                                                           *
*                                                                         *
*  All rights reserved to Iovan that are not due to anyone else.          *
*                                                                         *
*  This file was first written on 2011 and aside for a few instances only *
*  contains code written by Iovan for use with the Real Dragonball Battle *
*  System (RDBS) of the MUD Dragonball Advent Truth.                      *
************************************************************************ */
#pragma once

#include "structs.h"

/* combat.c functions */
extern void homing_update(uint64_t heartPulse, double deltaTime);

extern void damage_weapon(struct char_data *ch, struct obj_data *obj, struct char_data *vict);

extern int64_t advanced_energy(struct char_data *ch, int64_t dmg);

extern int roll_accuracy(struct char_data *ch, int skill, bool kiatt);

extern void handle_death_msg(struct char_data *ch, struct char_data *vict, int type);

extern int handle_combo(struct char_data *ch, struct char_data *vict);

extern int handle_parry(struct char_data *ch);

extern void handle_spiral(struct char_data *ch, struct char_data *vict, int skill, int first);

extern void handle_cooldown(struct char_data *ch, int cooldown);

extern void
parry_ki(double attperc, struct char_data *ch, struct char_data *vict, char sname[1000], int prob, int perc, int skill,
         int type);

extern void dodge_ki(struct char_data *ch, struct char_data *vict, int type, int type2, int skill, int skill2);

extern void damage_eq(struct char_data *vict, int location);

extern void remove_limb(struct char_data *vict, int num);

extern int check_skill(struct char_data *ch, int skill);

extern bool check_points(struct char_data *ch, int64_t ki, int64_t st);

extern void pcost(struct char_data *ch, double ki, int64_t st);

extern void
hurt(int limb, int chance, struct char_data *ch, struct char_data *vict, struct obj_data *obj, int64_t dmg, int type);

extern int64_t damtype(struct char_data *ch, int type, int skill, double percent);

extern int can_kill(struct char_data *ch, struct char_data *vict, struct obj_data *obj, int num);

extern void huge_update(uint64_t heartPulse, double deltaTime);

extern int init_skill(struct char_data *ch, int snum);

extern bool can_grav(struct char_data *ch);

extern int limb_ok(struct char_data *ch, int type);

extern int check_def(struct char_data *vict);

extern void dam_eq_loc(struct char_data *vict, int area);

extern void hurt_limb(struct char_data *ch, struct char_data *vict, int chance, int area, int64_t power);

extern int handle_speed(struct char_data *ch, struct char_data *vict);

extern void handle_defense(struct char_data *vict, int *pry, int *blk, int *dge);

extern void spar_gain(struct char_data *ch, struct char_data *vict, int type, int64_t dmg);

extern int chance_to_hit(struct char_data *ch);

extern int roll_hitloc(struct char_data *ch, struct char_data *vict, int skill);

long double calc_critical(struct char_data *ch, int loc);

extern int backstab(struct char_data *ch, struct char_data *vict, int wlvl, int64_t dmg);

extern int64_t gun_dam(struct char_data *ch, int wlvl);

extern int physical_mastery(struct char_data *ch);

extern int count_physical(struct char_data *ch);

extern int handle_dodge(struct char_data *ch);

extern int handle_block(struct char_data *ch);

extern void cut_limb(struct char_data *ch, struct char_data *vict, int wlvl, int hitspot);

extern void club_stamina(struct char_data *ch, struct char_data *vict, int wlvl, int64_t dmg);

extern int boom_headshot(struct char_data *ch);

extern void handle_knockdown(struct char_data *ch);

extern int roll_balance(struct char_data *ch);

extern int64_t combo_damage(struct char_data *ch, int64_t damage, int type);

extern int check_ruby(struct char_data *ch);

extern void combine_attacks(struct char_data *ch, struct char_data *vict);

extern void handle_disarm(struct char_data *ch, struct char_data *vict);

extern int handle_defender(struct char_data *vict, struct char_data *ch);

extern void handle_multihit(struct char_data *ch, struct char_data *vict);

extern int64_t armor_calc(struct char_data *ch, int64_t dmg, int type);
