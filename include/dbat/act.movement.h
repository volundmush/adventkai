#pragma once

#include "structs.h"

// global variables
extern const char *cmd_door[NUM_DOOR_CMD];

// functions
extern void handle_teleport(struct char_data *ch, struct char_data *tar, int location);

extern std::optional<room_vnum> land_location(char *arg, std::set<room_vnum>& rooms);

extern void carry_drop(struct char_data *ch, int type);

extern int has_o2(struct char_data *ch);

extern int do_simple_move(struct char_data *ch, int dir, int need_specials_check);

extern int perform_move(struct char_data *ch, int dir, int need_specials_check);

extern std::optional<vnum> governingAreaTypeFor(struct room_data *rd, std::function<bool(area_data&)>& func);

extern std::optional<vnum> governingAreaTypeFor(struct char_data *ch, std::function<bool(area_data&)>& func);

extern std::optional<vnum> governingAreaTypeFor(struct obj_data *obj, std::function<bool(area_data&)>& func);

extern std::size_t recurseScanRooms(area_data &start, std::set<room_vnum>& fill, std::function<bool(room_data&)>& func);


// commands
extern ACMD(do_gen_door);

extern ACMD(do_enter);

extern ACMD(do_leave);

extern ACMD(do_stand);

extern ACMD(do_fly);

extern ACMD(do_sit);

extern ACMD(do_rest);

extern ACMD(do_sleep);

extern ACMD(do_wake);

extern ACMD(do_follow);

extern ACMD(do_flee);

extern ACMD(do_carry);

extern ACMD(do_land);

extern ACMD(do_move);
