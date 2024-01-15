#pragma once

#include "structs.h"

/* act.item.c */
// global variables
extern struct obj_data *obj_selling;
extern struct char_data *ch_selling, *ch_buying;

// functions
extern int check_saveroom_count(struct char_data *ch, struct obj_data *cont);

extern void dball_load(uint64_t heartPulse, double deltaTime);

extern int check_insidebag(struct obj_data *cont, double mult);

extern int perform_get_from_room(struct char_data *ch, struct obj_data *obj);

extern void weight_change_object(struct obj_data *obj, int weight);

extern void name_from_drinkcon(struct obj_data *obj);

extern void name_to_drinkcon(struct obj_data *obj, int type);

extern void perform_wear(struct char_data *ch, struct obj_data *obj, int where);

extern int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);

extern void perform_remove(struct char_data *ch, int pos);

extern int64_t max_carry_weight(struct char_data *ch);

extern void stop_auction(int type, struct char_data *ch);

extern void check_auction(uint64_t heartPulse, double deltaTime);

// commands
extern ACMD(do_split);

extern ACMD(do_auction);

extern ACMD(do_bid);

extern ACMD(do_assemble);

extern ACMD(do_remove);

extern ACMD(do_put);

extern ACMD(do_get);

extern ACMD(do_drop);

extern ACMD(do_give);

extern ACMD(do_drink);

extern ACMD(do_eat);

extern ACMD(do_pour);

extern ACMD(do_wear);

extern ACMD(do_wield);

extern ACMD(do_grab);

extern ACMD(do_twohand);

extern ACMD(do_deploy);

extern ACMD(do_pack);

extern ACMD(do_garden);

extern ACMD(do_refuel);

extern ACMD(do_sac);
