#pragma once

#include "structs.h"


// functions
extern void npc_steal(struct char_data *ch, struct char_data *victim);

extern int num_players_in_room(room_vnum room);

extern bool check_mob_in_room(mob_vnum mob, room_vnum room);

extern bool check_obj_in_room(obj_vnum obj, room_vnum room);


// specials
extern SPECIAL(guild);

extern SPECIAL(dump);

extern SPECIAL(mayor);

extern SPECIAL(snake);

extern SPECIAL(thief);

extern SPECIAL(magic_user);

extern SPECIAL(guild_guard);

extern SPECIAL(puff);

extern SPECIAL(fido);

extern SPECIAL(janitor);

extern SPECIAL(cityguard);

extern SPECIAL(pet_shops);

extern SPECIAL(auction);

extern SPECIAL(bank);

extern SPECIAL(gravity);

extern SPECIAL(augmenter);

extern SPECIAL(gauntlet_room);

extern SPECIAL(gauntlet_end);

extern SPECIAL(gauntlet_rest);

extern SPECIAL(magic_user_orig);

extern SPECIAL(healtank);

extern SPECIAL(augmenter);

extern SPECIAL(gravity);

extern SPECIAL(bank);

extern SPECIAL(cleric_marduk);

extern SPECIAL(cleric_ao);

extern SPECIAL(dziak);

extern SPECIAL(azimer);

extern SPECIAL(lyrzaxyn);
