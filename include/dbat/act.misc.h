#pragma once

#include "structs.h"

extern void handle_multi_merge(struct char_data *form);

extern void handle_songs(uint64_t heartPulse, double deltaTime);

extern void fish_update(uint64_t heartPulse, double deltaTime);

extern void disp_rpp_store(struct char_data *ch);

extern void handle_rpp_store(struct char_data *ch, int choice);

extern void rpp_feature(struct char_data *ch, const char *arg);

extern void ash_burn(struct char_data *ch);

// commands
extern ACMD(do_transform);

extern ACMD(do_follow);

extern ACMD(do_spoil);

extern ACMD(do_feed);

extern ACMD(do_beacon);

extern ACMD(do_dimizu);

extern ACMD(do_obstruct);

extern ACMD(do_warppool);

extern ACMD(do_fireshield);

extern ACMD(do_cook);

extern ACMD(do_adrenaline);

extern ACMD(do_ensnare);

extern ACMD(do_arena);

extern ACMD(do_bury);

extern ACMD(do_hayasa);

extern ACMD(do_instill);

extern ACMD(do_kanso);

extern ACMD(do_hydromancy);

extern ACMD(do_channel);

extern ACMD(do_shimmer);

extern ACMD(do_metamorph);

extern ACMD(do_amnisiac);

extern ACMD(do_healglow);

extern ACMD(do_resize);

extern ACMD(do_scry);

extern ACMD(do_runic);

extern ACMD(do_extract);

extern ACMD(do_fish);

extern ACMD(do_defend);

extern ACMD(do_lifeforce);

extern ACMD(do_liquefy);

extern ACMD(do_shell);

extern ACMD(do_moondust);

extern ACMD(do_preference);

extern ACMD(do_song);

extern ACMD(do_multiform);

extern ACMD(do_spiritcontrol);

extern ACMD(do_ashcloud);

extern ACMD(do_silk);
