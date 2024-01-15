#pragma once

#include "structs.h"


#define MAX_HOUSES    1000
#define MAX_GUESTS    10

#define HOUSE_PRIVATE    0
#define HOUSE_GOD       1  /* Imm owned house             */
#define HOUSE_CLAN      2  /* Clan crash-save room        */
#define HOUSE_UNOWNED   3

#define NUM_HOUSE_TYPES 4


struct house_control_rec {
    room_vnum vn;        /* vnum of this house		*/
    room_vnum atrium;        /* vnum of atrium		*/
    int16_t exit_num;        /* direction of house's exit	*/
    time_t built_on;        /* date this house was built	*/
    int mode;            /* mode of ownership		*/
    long owner;            /* idnum of house's owner	*/
    int num_of_guests;        /* how many guests for house	*/
    long guests[MAX_GUESTS];    /* idnums of house's guests	*/
    time_t last_payment;        /* date of last house payment   */
    long bitvector;
    long builtby;
    long spare2;
    long spare3;
    long spare4;
    long spare5;
    long spare6;
    long spare7;
};


#define HOUSE_FLAGS(house) (house).bitvector

/* House can have up to 31 bitvectors - don't go higher */
#define HOUSE_NOGUESTS   (1 << 0)   /* Owner cannot add guests     */
#define HOUSE_FREE       (1 << 1)   /* House does not require payments */
#define HOUSE_NOIMMS     (1 << 2)   /* Imms below level 999 cannot enter */
#define HOUSE_IMPONLY    (1 << 3)   /* Imms below level 1000 cannot enter */
#define HOUSE_RENTFREE   (1 << 4)   /* No rent is charged on items left here */
#define HOUSE_SAVENORENT (1 << 5)   /* NORENT items are crashsaved too */
#define HOUSE_NOSAVE     (1 << 6)   /* Do not crash save this room - private only */

#define HOUSE_NUM_FLAGS 7

#define TOROOM(room, dir) (world[room].dir_option[dir] ? \
                world[room].dir_option[dir]->to_room : NOWHERE)

extern struct house_control_rec house_control[MAX_HOUSES];     /* house.c */
extern int num_of_houses;                                      /* house.c */

extern void House_listrent(struct char_data *ch, room_vnum vnum);

extern void House_boot(bool legacy);

extern void House_save_all(uint64_t heartPulse, double deltaTime);

extern int House_can_enter(struct char_data *ch, room_vnum house);

extern void House_crashsave(room_vnum vnum);

extern void House_list_guests(struct char_data *ch, int i, int quiet);

extern void hcontrol_list_houses(struct char_data *ch);

extern int find_house(room_vnum vnum);                                /* house.c */
extern void House_save_control();                                 /* house.c */
extern void House_delete_file(room_vnum vnum);                        /* house.c */

extern ACMD(do_hcontrol);

extern ACMD(do_house);
