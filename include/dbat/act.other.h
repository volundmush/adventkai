#pragma once

#include "structs.h"

// variables
extern const room_vnum freeres[NUM_ALIGNS];

// functions
extern void log_imm_action(char *messg, ...);

extern void hint_system(struct char_data *ch, int num);

extern std::set<struct obj_data*> dball_count(struct char_data *ch);

extern void log_custom(struct descriptor_data *d, struct obj_data *obj);

extern void wishSYS(uint64_t heartPulse, double deltaTime);

extern void bring_to_cap(struct char_data *ch);

extern void base_update(uint64_t heartPulse, double deltaTime);

extern void load_shadow_dragons();

// commands
extern ACMD(do_gen_comm);

extern ACMD(do_charge);

extern ACMD(do_wear);

extern ACMD(do_quit);

extern ACMD(do_save);

extern ACMD(do_not_here);

extern ACMD(do_hide);

extern ACMD(do_steal);

extern ACMD(do_practice);

extern ACMD(do_visible);

extern ACMD(do_title);

extern ACMD(do_group);

extern ACMD(do_ungroup);

extern ACMD(do_report);

extern ACMD(do_split);

extern ACMD(do_use);

extern ACMD(do_value);

extern ACMD(do_display);

extern ACMD(do_gen_write);

extern ACMD(do_gen_tog);

extern ACMD(do_file);

extern ACMD(do_scribe);

extern ACMD(do_scouter);

extern ACMD(do_snet);

extern ACMD(do_spar);

extern ACMD(do_pushup);

extern ACMD(do_situp);

extern ACMD(do_transform);

extern ACMD(do_summon);

extern ACMD(do_instant);

extern ACMD(do_barrier);

extern ACMD(do_heal);

extern ACMD(do_solar);

extern ACMD(do_eyec);

extern ACMD(do_zanzoken);

extern ACMD(do_eavesdrop);

extern ACMD(do_disguise);

extern ACMD(do_appraise);

extern ACMD(do_forgery);

extern ACMD(do_plant);

extern ACMD(do_kaioken);

extern ACMD(do_focus);

extern ACMD(do_regenerate);

extern ACMD(do_escape);

extern ACMD(do_absorb);

extern ACMD(do_ingest);

extern ACMD(do_upgrade);

extern ACMD(do_srepair);

extern ACMD(do_recharge);

extern ACMD(do_form);

extern ACMD(do_spit);

extern ACMD(do_majinize);

extern ACMD(do_potential);

extern ACMD(do_telepathy);

extern ACMD(do_fury);

extern ACMD(do_pose);

extern ACMD(do_implant);

extern ACMD(do_hass);

extern ACMD(do_suppress);

extern ACMD(do_drag);

extern ACMD(do_stop);

extern ACMD(do_future);

extern ACMD(do_candy);

extern ACMD(do_kura);

extern ACMD(do_taisha);

extern ACMD(do_paralyze);

extern ACMD(do_infuse);

extern ACMD(do_rip);

extern ACMD(do_train);

extern ACMD(do_trip);

extern ACMD(do_grapple);

extern ACMD(do_willpower);

extern ACMD(do_commune);

extern ACMD(do_rpp);

extern ACMD(do_meditate);

extern ACMD(do_aura);

extern ACMD(do_think);

extern ACMD(do_block);

extern ACMD(do_visible);

extern ACMD(do_compare);

extern ACMD(do_break);

extern ACMD(do_fix);

extern ACMD(do_resurrect);

extern ACMD(do_clan);

extern ACMD(do_aid);
