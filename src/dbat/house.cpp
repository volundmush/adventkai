/* ************************************************************************
*   File: house.c                                       Part of CircleMUD *
*  Usage: Handling of player houses                                       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "dbat/house.h"
#include "dbat/comm.h"
#include "dbat/handler.h"
#include "dbat/db.h"
#include "dbat/interpreter.h"
#include "dbat/utils.h"
#include "dbat/constants.h"
#include "dbat/objsave.h"

/* local globals */
struct house_control_rec house_control[MAX_HOUSES];
int num_of_houses = 0;

/* local functions */
int House_get_filename(room_vnum vnum, char *filename, size_t maxlen);

int House_load(room_vnum vnum);

int House_save(struct obj_data *obj, FILE *fp, int location);

void House_restore_weight(struct obj_data *obj);

void House_delete_file(room_vnum vnum);

int find_house(room_vnum vnum);

void House_save_control();

void hcontrol_build_house(struct char_data *ch, char *arg);

void hcontrol_destroy_house(struct char_data *ch, char *arg);

void hcontrol_pay_house(struct char_data *ch, char *arg);


#define MAX_BAG_ROWS    5

/* First, the basics: finding the filename; loading/saving objects */

/* Return a filename given a house vnum */
int House_get_filename(room_vnum vnum, char *filename, size_t maxlen) {
    if (vnum == NOWHERE)
        return (0);

    snprintf(filename, maxlen, LIB_HOUSE"%d.house", vnum);
    return (1);
}


/* Save all objects for a house (recursive; initial call must be followed
   by a call to House_restore_weight)  Assumes file is open already. */
int House_save(struct obj_data *obj, FILE *fp, int location) {
    struct obj_data *tmp;
    int result;
    if (obj) {
        if (OBJ_FLAGGED(obj, ITEM_NORENT)) {
            obj = obj->next_content;
        }
    }
    if (obj) {
        House_save(obj->next_content, fp, location);
        House_save(obj->contents, fp, MIN(0, location) - 1);
        result = Obj_to_store(obj, fp, location);
        if (!result)
            return (0);

        for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
            GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    }
    return (1);
}


/* restore weight of containers after House_save has changed them for saving */
void House_restore_weight(struct obj_data *obj) {
    if (obj) {
        House_restore_weight(obj->contents);
        House_restore_weight(obj->next_content);
        if (obj->in_obj)
            GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
    }
}


/* Save all objects in a house */
void House_crashsave(room_vnum vnum) {
    int rnum;
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    auto find = world.find(vnum);
    if (find == world.end())
        return;

    if (!House_get_filename(vnum, buf, sizeof(buf)))
        return;

    if (!(fp = fopen(buf, "wb"))) {
        perror("SYSERR: Error saving house file");
        return;
    }
    if (!House_save(find->second.contents, fp, 0)) {
        fclose(fp);
        return;
    }
    fclose(fp);
    House_restore_weight(find->second.contents);
    find->second.room_flags.reset(ROOM_HOUSE_CRASH);
}


/* Delete a house save file */
void House_delete_file(room_vnum vnum) {
    char filename[MAX_INPUT_LENGTH];
    FILE *fl;

    if (!House_get_filename(vnum, filename, sizeof(filename)))
        return;
    if (!(fl = fopen(filename, "rb"))) {
        if (errno != ENOENT)
            basic_mud_log("SYSERR: Error deleting house file #%d. (1): %s", vnum, strerror(errno));
        return;
    }
    fclose(fl);
    if (remove(filename) < 0)
        basic_mud_log("SYSERR: Error deleting house file #%d. (2): %s", vnum, strerror(errno));
}


/******************************************************************
 *  Functions for house administration (creation, deletion, etc.  *
 *****************************************************************/

int find_house(room_vnum vnum) {
    int i;

    for (i = 0; i < num_of_houses; i++)
        if (house_control[i].vn == vnum)
            return (i);

    return (NOWHERE);
}


/* Save the house control information */
void House_save_control() {
    FILE *fl;

    if (!(fl = fopen(HCONTROL_FILE, "wb"))) {
        perror("SYSERR: Unable to open house control file.");
        return;
    }
    /* write all the house control recs in one fell swoop.  Pretty nifty, eh? */
    fwrite(house_control, sizeof(struct house_control_rec), num_of_houses, fl);

    fclose(fl);
}


/* call from boot_db - will load control recs, load objs, set atrium bits */
/* should do sanity checks on vnums & remove invalid records */
void House_boot(bool legacy) {
    struct house_control_rec temp_house;
    room_rnum real_house;
    FILE *fl;

    memset((char *) house_control, 0, sizeof(struct house_control_rec) * MAX_HOUSES);

    if (!(fl = fopen(HCONTROL_FILE, "rb"))) {
        if (errno == ENOENT)
            basic_mud_log("   No houses to load. File '%s' does not exist.", HCONTROL_FILE);
        else
            perror("SYSERR: " HCONTROL_FILE);
        return;
    }
    while (!feof(fl) && num_of_houses < MAX_HOUSES) {
        fread(&temp_house, sizeof(struct house_control_rec), 1, fl);

        if (feof(fl))
            break;

        if (!world.contains(temp_house.vn))
            continue;            /* this vnum doesn't exist -- skip */

        if (find_house(temp_house.vn) != NOWHERE)
            continue;            /* this vnum is already a house -- skip */

        house_control[num_of_houses++] = temp_house;

        auto &r = world[temp_house.vn];
        r.room_flags.set(ROOM_HOUSE);
        r.room_flags.set(ROOM_SAVE);

        if(legacy) House_load(temp_house.vn);
    }

    fclose(fl);
    House_save_control();
}


/* "House Control" functions */

const char *HCONTROL_FORMAT =
        "Usage: hcontrol build <house vnum> <exit direction> <player name>\r\n"
        "       hcontrol destroy <house vnum>\r\n"
        "       hcontrol pay <house vnum>\r\n"
        "       hcontrol show\r\n";

void hcontrol_list_houses(struct char_data *ch) {
    int i;
    char *timestr, *temp;
    char built_on[128], last_pay[128], own_name[MAX_NAME_LENGTH + 1];

    if (!num_of_houses) {
        send_to_char(ch, "No houses have been defined.\r\n");
        return;
    }
    send_to_char(ch,
                 "Address  Atrium  Build Date  Guests  Owner        Last Paymt\r\n"
                 "-------  ------  ----------  ------  ------------ ----------\r\n");

    for (i = 0; i < num_of_houses; i++) {
        /* Avoid seeing <UNDEF> entries from self-deleted people. -gg 6/21/98 */
        if ((temp = get_name_by_id(house_control[i].owner)) == nullptr)
            continue;

        if (house_control[i].built_on) {
            timestr = asctime(localtime(&(house_control[i].built_on)));
            *(timestr + 10) = '\0';
            strlcpy(built_on, timestr, sizeof(built_on));
        } else
            strcpy(built_on, "Unknown");    /* strcpy: OK (for 'strlen("Unknown") < 128') */

        if (house_control[i].last_payment) {
            timestr = asctime(localtime(&(house_control[i].last_payment)));
            *(timestr + 10) = '\0';
            strlcpy(last_pay, timestr, sizeof(last_pay));
        } else
            strcpy(last_pay, "None");    /* strcpy: OK (for 'strlen("None") < 128') */

        /* Now we need a copy of the owner's name to capitalize. -gg 6/21/98 */
        strcpy(own_name, temp);    /* strcpy: OK (names guaranteed <= MAX_NAME_LENGTH+1) */
        send_to_char(ch, "%7d %-10s    %2d    %-12s %s\r\n",
                     house_control[i].vn, built_on,
                     house_control[i].num_of_guests, CAP(own_name), last_pay);

        House_list_guests(ch, i, true);
    }
}


void hcontrol_build_house(struct char_data *ch, char *arg) {
    char arg1[MAX_INPUT_LENGTH];
    struct house_control_rec temp_house;
    room_vnum virt_house;
    room_rnum real_house;
    int16_t exit_num;
    long owner;

    if (num_of_houses >= MAX_HOUSES) {
        send_to_char(ch, "Max houses already defined.\r\n");
        return;
    }

    /* first arg: house's vnum */
    arg = one_argument(arg, arg1);
    if (!*arg1) {
        send_to_char(ch, "%s", HCONTROL_FORMAT);
        return;
    }
    virt_house = atoi(arg1);
    auto room = world.find(virt_house);
    if (room == world.end()) {
        send_to_char(ch, "No such room exists.\r\n");
        return;
    }

    if ((find_house(virt_house)) != NOWHERE) {
        send_to_char(ch, "House already exists.\r\n");
        return;
    }

    auto &r = world[virt_house];

    /* second arg: direction of house's exit */
    arg = one_argument(arg, arg1);
    if (!*arg1) {
        send_to_char(ch, "%s", HCONTROL_FORMAT);
        return;
    }
    if ((exit_num = search_block(arg1, dirs, false)) < 0 &&
        (exit_num = search_block(arg1, abbr_dirs, false)) < 0) {
        send_to_char(ch, "'%s' is not a valid direction.\r\n", arg1);
        return;
    }
    if (r.dir_option[exit_num]->to_room == NOWHERE) {
        send_to_char(ch, "There is no exit %s from room %d.\r\n", dirs[exit_num], virt_house);
        return;
    }


    /* third arg: player's name */
    one_argument(arg, arg1);
    if (!*arg1) {
        send_to_char(ch, "%s", HCONTROL_FORMAT);
        return;
    }
    if ((owner = get_id_by_name(arg1)) < 0) {
        send_to_char(ch, "Unknown player '%s'.\r\n", arg1);
        return;
    }

    temp_house.mode = HOUSE_PRIVATE;
    temp_house.vn = virt_house;
    temp_house.exit_num = exit_num;
    temp_house.built_on = time(nullptr);
    temp_house.last_payment = 0;
    temp_house.owner = owner;
    temp_house.num_of_guests = 0;

    house_control[num_of_houses++] = temp_house;

    r.room_flags.set(ROOM_HOUSE);
    r.room_flags.set(ROOM_SAVE);
    House_crashsave(virt_house);

    send_to_char(ch, "House built.  Mazel tov!\r\n");
    House_save_control();
}


void hcontrol_destroy_house(struct char_data *ch, char *arg) {
    int i, j;
    room_rnum real_atrium, real_house;

    if (!*arg) {
        send_to_char(ch, "%s", HCONTROL_FORMAT);
        return;
    }
    if ((i = find_house(atoi(arg))) == NOWHERE) {
        send_to_char(ch, "Unknown house.\r\n");
        return;
    }
    if ((real_atrium = real_room(house_control[i].atrium)) == NOWHERE)
        basic_mud_log("SYSERR: House %d had invalid atrium %d!", atoi(arg), house_control[i].atrium);
    else
        ROOM_FLAGS(real_atrium).reset(ROOM_ATRIUM);

    if ((real_house = real_room(house_control[i].vn)) == NOWHERE)
        basic_mud_log("SYSERR: House %d had invalid vnum %d!", atoi(arg), house_control[i].vn);
    else {
        ROOM_FLAGS(real_house).reset(ROOM_HOUSE);
        ROOM_FLAGS(real_house).reset(ROOM_HOUSE_CRASH);
    }

    House_delete_file(house_control[i].vn);

    for (j = i; j < num_of_houses - 1; j++)
        house_control[j] = house_control[j + 1];

    num_of_houses--;

    send_to_char(ch, "House deleted.\r\n");
    House_save_control();

    /*
     * Now, reset the ROOM_ATRIUM flag on all existing houses' atriums,
     * just in case the house we just deleted shared an atrium with another
     * house.  --JE 9/19/94
     */
    for (i = 0; i < num_of_houses; i++)
        if ((real_atrium = real_room(house_control[i].atrium)) != NOWHERE)
            ROOM_FLAGS(real_atrium).set(ROOM_ATRIUM);
}


void hcontrol_pay_house(struct char_data *ch, char *arg) {
    int i;

    if (!*arg)
        send_to_char(ch, "%s", HCONTROL_FORMAT);
    else if ((i = find_house(atoi(arg))) == NOWHERE)
        send_to_char(ch, "Unknown house.\r\n");
    else {
        mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "Payment for house %s collected by %s.", arg,
               GET_NAME(ch));

        house_control[i].last_payment = time(nullptr);
        House_save_control();
        send_to_char(ch, "Payment recorded.\r\n");
    }
}


/* The hcontrol command itself, used by imms to create/destroy houses */
ACMD(do_hcontrol) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    half_chop(argument, arg1, arg2);

    if (is_abbrev(arg1, "build"))
        hcontrol_build_house(ch, arg2);
    else if (is_abbrev(arg1, "destroy"))
        hcontrol_destroy_house(ch, arg2);
    else if (is_abbrev(arg1, "pay"))
        hcontrol_pay_house(ch, arg2);
    else if (is_abbrev(arg1, "show"))
        hcontrol_list_houses(ch);
    else
        send_to_char(ch, "%s", HCONTROL_FORMAT);
}


/* The house command, used by mortal house owners to assign guests */
ACMD(do_house) {
    char arg[MAX_INPUT_LENGTH];
    int i, j, id;

    one_argument(argument, arg);

    if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOUSE))
        send_to_char(ch, "You must be in your house to set guests.\r\n");
    else if ((i = find_house(GET_ROOM_VNUM(IN_ROOM(ch)))) == NOWHERE)
        send_to_char(ch, "Um.. this house seems to be screwed up.\r\n");
    else if (GET_IDNUM(ch) != house_control[i].owner)
        send_to_char(ch, "Only the primary owner can set guests.\r\n");
    else if (!*arg)
        House_list_guests(ch, i, false);
    else if ((id = get_id_by_name(arg)) < 0)
        send_to_char(ch, "No such player.\r\n");
    else if (id == GET_IDNUM(ch))
        send_to_char(ch, "It's your house!\r\n");
    else {
        for (j = 0; j < house_control[i].num_of_guests; j++)
            if (house_control[i].guests[j] == id) {
                for (; j < house_control[i].num_of_guests; j++)
                    house_control[i].guests[j] = house_control[i].guests[j + 1];
                house_control[i].num_of_guests--;
                House_save_control();
                send_to_char(ch, "Guest deleted.\r\n");
                return;
            }
        if (house_control[i].num_of_guests == MAX_GUESTS) {
            send_to_char(ch, "You have too many guests.\r\n");
            return;
        }
        j = house_control[i].num_of_guests++;
        house_control[i].guests[j] = id;
        House_save_control();
        send_to_char(ch, "Guest added.\r\n");
    }
}



/* Misc. administrative functions */


/* crash-save all the houses */
void House_save_all(uint64_t heartPulse, double deltaTime) {
    for(auto &[vn, room] : world) {
        if(room.room_flags.test(ROOM_SAVE)) room.save();
    }
}


/* note: arg passed must be house vnum, so there. */
int House_can_enter(struct char_data *ch, room_vnum house) {
    int i, j;

    if (ADM_FLAGGED(ch, ADM_ALLHOUSES) || (i = find_house(house)) == NOWHERE)
        return (1);

    switch (house_control[i].mode) {
        case HOUSE_CLAN:
        case HOUSE_UNOWNED:
            return (1);
        case HOUSE_PRIVATE:
            if (GET_IDNUM(ch) == house_control[i].owner)
                return (1);
            for (j = 0; j < house_control[i].num_of_guests; j++)
                if (GET_IDNUM(ch) == house_control[i].guests[j])
                    return (1);
    }

    return (0);
}

void House_list_guests(struct char_data *ch, int i, int quiet) {
    int j, num_printed;
    char *temp;

    if (house_control[i].num_of_guests == 0) {
        if (!quiet)
            send_to_char(ch, "  Guests: None\r\n");
        return;
    }

    send_to_char(ch, "  Guests: ");

    for (num_printed = j = 0; j < house_control[i].num_of_guests; j++) {
        /* Avoid <UNDEF>. -gg 6/21/98 */
        if ((temp = get_name_by_id(house_control[i].guests[j])) == nullptr)
            continue;

        num_printed++;
        send_to_char(ch, "%c%s ", UPPER(*temp), temp + 1);
    }

    if (num_printed == 0)
        send_to_char(ch, "all dead");

    send_to_char(ch, "\r\n");
}

int House_load(room_vnum rvnum) {
    FILE *fl;
    char f1[READ_SIZE], f2[READ_SIZE], f3[READ_SIZE], f4[READ_SIZE];
    char cmfname[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char line[256];
    int64_t t[21], danger, zwei = 0;
    struct obj_data *temp;
    int locate = 0, j, nr, k, num_objs = 0;
    struct obj_data *obj1;
    struct obj_data *cont_row[MAX_BAG_ROWS];
    struct extra_descr_data *new_descr;
    room_rnum rrnum = rvnum;

    if (!world.contains(rvnum))
        return 0;

    if (!House_get_filename(rvnum, cmfname, sizeof(cmfname)))
        return 0;

    if (!(fl = fopen(cmfname, "r+b"))) {
        if (errno != ENOENT) {  /* if it fails, NOT because of no file */
            sprintf(buf1, "SYSERR: READING HOUSE FILE %s (5)", cmfname);
            perror(buf1);
        }
        return 0;
    }

    for (j = 0; j < MAX_BAG_ROWS; j++)
        cont_row[j] = nullptr; /* empty all cont lists (you never know ...) */

    if (!feof(fl))
        get_line(fl, line);
    while (!feof(fl)) {
        temp = nullptr;
        /* first, we get the number. Not too hard. */
        if (*line == '#') {
            if (sscanf(line, "#%d", &nr) != 1) {
                continue;
            }
            /* we have the number, check it, load obj. */
            if (nr == NOTHING) {   /* then it is unique */
                temp = create_obj(false);
                temp->vn = NOTHING;
            } else if (nr < 0) {
                continue;
            } else {
                if (nr >= 999999)
                    continue;
                temp = read_object(nr, VIRTUAL, false);
                if (!temp) {
                    get_line(fl, line);
                    continue;
                }
            }

            get_line(fl, line);
            sscanf(line, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %s %s %s %s %ld %ld %ld %ld %ld %ld %ld %ld", t, t + 1, t + 2, t + 3,
                   t + 4, t + 5, t + 6, t + 7, t + 8, f1, f2, f3, f4, t + 13, t + 14, t + 15, t + 16, t + 17, t + 18,
                   t + 19, t + 20);
            locate = t[0];
            GET_OBJ_VAL(temp, 0) = t[1];
            GET_OBJ_VAL(temp, 1) = t[2];
            GET_OBJ_VAL(temp, 2) = t[3];
            GET_OBJ_VAL(temp, 3) = t[4];
            GET_OBJ_VAL(temp, 4) = t[5];
            GET_OBJ_VAL(temp, 5) = t[6];
            GET_OBJ_VAL(temp, 6) = t[7];
            GET_OBJ_VAL(temp, 7) = t[8];
            bitvector_t ex[4];
            ex[0] = asciiflag_conv(f1);
            ex[1] = asciiflag_conv(f2);
            ex[2] = asciiflag_conv(f3);
            ex[3] = asciiflag_conv(f4);
            for(auto i = 0; i < temp->extra_flags.size(); i++) temp->extra_flags.set(i, IS_SET_AR(ex, i));
            GET_OBJ_VAL(temp, 8) = t[13];
            GET_OBJ_VAL(temp, 9) = t[14];
            GET_OBJ_VAL(temp, 10) = t[15];
            GET_OBJ_VAL(temp, 11) = t[16];
            GET_OBJ_VAL(temp, 12) = t[17];
            GET_OBJ_VAL(temp, 13) = t[18];
            GET_OBJ_VAL(temp, 14) = t[19];
            GET_OBJ_VAL(temp, 15) = t[20];
            GET_OBJ_POSTED(temp) = nullptr;
            GET_OBJ_POSTTYPE(temp) = 0;

            get_line(fl, line);
            /* read line check for xap. */
            if (!strcmp("XAP", line)) {  /* then this is a Xap Obj, requires
                                       special care */
                if ((temp->name = fread_string(fl, buf2)) == nullptr) {
                    temp->name = "undefined";
                }

                if ((temp->short_description = fread_string(fl, buf2)) == nullptr) {
                    temp->short_description = "undefined";
                }

                if ((temp->room_description = fread_string(fl, buf2)) == nullptr) {
                    temp->room_description = "undefined";
                }

                if ((temp->look_description = fread_string(fl, buf2)) == nullptr) {
                    temp->look_description = nullptr;
                }


                if (!get_line(fl, line) ||
                    (sscanf(line, "%ld %ld %ld %ld %ld %ld %ld %ld", t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7) !=
                     8)) {
                    fprintf(stderr, "Format error in first numeric line (expecting _x_ args)");
                    return 0;
                }
                temp->type_flag = t[0];
                bitvector_t wear[4];
                wear[0] = t[1];
                wear[1] = t[2];
                wear[2] = t[3];
                wear[3] = t[4];
                for(auto i = 0; i < temp->wear_flags.size(); i++) temp->wear_flags.set(i, IS_SET_AR(wear, i));
                temp->weight = t[5];
                temp->cost = t[6];
                temp->cost_per_day = t[7];

                /* buf2 is error codes pretty much */
                //strcat(buf2, ", after numeric constants (expecting E/#xxx)");

                /*add_unique_id(temp);*/
                /* we're clearing these for good luck */

                for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                    temp->affected[j].location = APPLY_NONE;
                    temp->affected[j].modifier = 0;
                    temp->affected[j].specific = 0;
                }

                temp->ex_description = nullptr;

                get_line(fl, line);
                for (k = j = zwei = 0; !zwei && !feof(fl);) {
                    switch (*line) {
                        case 'E':
                            CREATE(new_descr, struct extra_descr_data, 1);
                            new_descr->keyword = fread_string(fl, buf2);
                            new_descr->description = fread_string(fl, buf2);
                            new_descr->next = temp->ex_description;
                            temp->ex_description = new_descr;
                            get_line(fl, line);
                            break;
                        case 'A':
                            if (j >= MAX_OBJ_AFFECT) {
                                basic_mud_log("SYSERR: Too many object affectations in loading house file");
                                danger = 1;
                            }
                            get_line(fl, line);
                            sscanf(line, "%ld %ld %ld", t, t + 1, t + 2);
                            temp->affected[j].location = t[0];
                            temp->affected[j].modifier = t[1];
                            temp->affected[j].specific = t[2];
                            j++;
                            get_line(fl, line);
                            break;

                        case 'G':
                            get_line(fl, line);
                            sscanf(line, "%" TMT, &temp->generation);
                            get_line(fl, line);
                            break;
                        case 'U':
                            get_line(fl, line);
                            sscanf(line, "%" I64T, &temp->id);
                            get_line(fl, line);
                            break;
                        case 'S':
                            if (j >= SPELLBOOK_SIZE) {
                                basic_mud_log("SYSERR: Too many spells in spellbook loading rent file");
                                danger = 1;
                            }
                            get_line(fl, line);
                            sscanf(line, "%d %d", t, t + 1);

                            if (!temp->sbinfo) {
                                CREATE(temp->sbinfo, struct obj_spellbook_spell, SPELLBOOK_SIZE);
                                memset((char *) temp->sbinfo, 0, SPELLBOOK_SIZE * sizeof(struct obj_spellbook_spell));
                            }
                            temp->sbinfo[j].spellname = t[0];
                            temp->sbinfo[j].pages = t[1];
                            j++;
                            get_line(fl, line);
                            break;
                        case 'Z':
                            get_line(fl, line);
                            sscanf(line, "%d", &GET_OBJ_SIZE(temp));
                            get_line(fl, line);
                            break;
                        case '$':
                        case '#':
                            zwei = 1;
                            break;
                        default:
                            zwei = 1;
                            break;
                    }
                }      /* exit our for loop */
            }   /* exit our xap loop */
            if (temp != nullptr) {
                check_unique_id(temp);
                add_unique_id(temp);
                temp->activate();
                num_objs++;
                obj_to_room(temp, rrnum);
            }

/*No need to check if its equipped since rooms can't equip things --firebird_223*/
            for (j = MAX_BAG_ROWS - 1; j > -locate; j--)
                if (cont_row[j]) { /* no container -> back to ch's inventory */
                    for (; cont_row[j]; cont_row[j] = obj1) {
                        obj1 = cont_row[j]->next_content;
                        obj_to_room(cont_row[j], rrnum);
                    }
                    cont_row[j] = nullptr;
                }

            if (j == -locate && cont_row[j]) { /* content list existing */
                if (GET_OBJ_TYPE(temp) == ITEM_CONTAINER) {
                    /* take item ; fill ; give to char again */
                    obj_from_room(temp);
                    temp->contents = nullptr;
                    for (; cont_row[j]; cont_row[j] = obj1) {
                        obj1 = cont_row[j]->next_content;
                        obj_to_obj(cont_row[j], temp);
                    }
                    obj_to_room(temp, rrnum); /* add to inv first ... */
                } else { /* object isn't container -> empty content list */
                    for (; cont_row[j]; cont_row[j] = obj1) {
                        obj1 = cont_row[j]->next_content;
                        obj_to_room(cont_row[j], rrnum);
                    }
                    cont_row[j] = nullptr;
                }
            }

            if (locate < 0 && locate >= -MAX_BAG_ROWS) {
                /* let obj be part of content list
                   but put it at the list's end thus having the items
                   in the same order as before renting */
                obj_from_room(temp);
                if ((obj1 = cont_row[-locate - 1])) {
                    while (obj1->next_content)
                        obj1 = obj1->next_content;
                    obj1->next_content = temp;
                } else
                    cont_row[-locate - 1] = temp;
            }
        } else {
            get_line(fl, line);
        }
    }


    fclose(fl);

    return 1;
}

