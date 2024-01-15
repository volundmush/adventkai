//
// Created by volund on 10/20/21.
//
#pragma once

#include "structs.h"
#include "spells.h"

// global variables
extern struct char_data *combat_list, *next_combat_list;
extern struct attack_hit_type attack_hit_text[NUM_ATTACK_TYPES];

// functions
extern void death_cry(struct char_data *ch);

extern int group_bonus(struct char_data *ch, int type);

extern void die(struct char_data *ch, struct char_data *killer);

extern void remove_limb(struct char_data *vict, int num);

extern void impact_sound(struct char_data *ch, char *mssg);

extern void fight_stack(uint64_t heartPulse, double deltaTime);

extern void appear(struct char_data *ch);

extern void raw_kill(struct char_data *ch, struct char_data *killer);

extern void set_fighting(struct char_data *ch, struct char_data *victim);

extern void stop_fighting(struct char_data *ch);

extern void group_gain(struct char_data *ch, struct char_data *victim);

extern void solo_gain(struct char_data *ch, struct char_data *victim);

extern void mutant_limb_regen(struct char_data *ch);

// commands
extern ACMD(do_kousengan);

extern ACMD(do_heal);

extern ACMD(do_trip);

extern ACMD(do_koteiru);

extern ACMD(do_razor);

extern ACMD(do_spike);

extern ACMD(do_ddslash);

extern ACMD(do_kakusanha);

extern ACMD(do_psyblast);

extern ACMD(do_punch);

extern ACMD(do_powerup);

extern ACMD(do_srepair);

extern ACMD(do_absorb);

extern ACMD(do_kaioken);

extern ACMD(do_strike);

extern ACMD(do_kick);

extern ACMD(do_elbow);

extern ACMD(do_knee);

extern ACMD(do_uppercut);

extern ACMD(do_roundhouse);

extern ACMD(do_heeldrop);

extern ACMD(do_slam);

extern ACMD(do_tailwhip);

extern ACMD(do_head);

extern ACMD(do_bite);

extern ACMD(do_ram);

extern ACMD(do_breath);

extern ACMD(do_kiball);

extern ACMD(do_kiblast);

extern ACMD(do_beam);

extern ACMD(do_renzo);

extern ACMD(do_tsuihidan);

extern ACMD(do_shogekiha);

extern ACMD(do_kamehameha);

extern ACMD(do_galikgun);

extern ACMD(do_masenko);

extern ACMD(do_hellspear);

extern ACMD(do_hellflash);

extern ACMD(do_dualbeam);

extern ACMD(do_honoo);

extern ACMD(do_pbarrage);

extern ACMD(do_tslash);

extern ACMD(do_pslash);

extern ACMD(do_crusher);

extern ACMD(do_eraser);

extern ACMD(do_spiral);

extern ACMD(do_tribeam);

extern ACMD(do_dodonpa);

extern ACMD(do_hass);

extern ACMD(do_zanzoken);

extern ACMD(do_deathball);

extern ACMD(do_deathbeam);

extern ACMD(do_kienzan);

extern ACMD(do_bigbang);

extern ACMD(do_final);

extern ACMD(do_sbc);

extern ACMD(do_scatter);

extern ACMD(do_nova);

extern ACMD(do_breaker);

extern ACMD(do_seishou);

extern ACMD(do_ensnare);

extern ACMD(do_barrier);

extern ACMD(do_attack);

extern ACMD(do_stand);

extern ACMD(do_fly);

extern ACMD(do_wake);

extern ACMD(do_flee);

extern ACMD(do_get);

extern ACMD(do_split);

extern ACMD(do_sac);

extern ACMD(do_transform);

extern ACMD(do_kaioken);

extern ACMD(do_escape);

extern ACMD(do_balefire);

extern ACMD(do_blessedhammer);
