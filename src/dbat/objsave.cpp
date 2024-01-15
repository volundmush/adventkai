/* ************************************************************************
*   File: objsave.c                                     Part of CircleMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/objsave.h"
#include "unistd.h"
#include "errno.h"
#include "dbat/structs.h"
#include "dbat/comm.h"
#include "dbat/handler.h"
#include "dbat/db.h"
#include "dbat/interpreter.h"
#include "dbat/utils.h"
#include "dbat/spells.h"
#include "dbat/players.h"
#include "dbat/class.h"
#include "dbat/act.social.h"
#include "dbat/act.item.h"

/* local functions */

static int Crash_offer_rent(struct char_data *ch, struct char_data *recep, int display, int factor);

static int Crash_report_unrentables(struct char_data *ch, struct char_data *recep, struct obj_data *obj);

static void
Crash_report_rent(struct char_data *ch, struct char_data *recep, struct obj_data *obj, long *cost, long *nitems,
                  int display, int factor);

static int gen_receptionist(struct char_data *ch, struct char_data *recep, int cmd, char *arg, int mode);

static int Crash_save(struct obj_data *obj, FILE *fp, int location);

static void Crash_rent_deadline(struct char_data *ch, struct char_data *recep, long cost);

static void Crash_restore_weight(struct obj_data *obj);

static void Crash_extract_objs(struct obj_data *obj);

static int Crash_is_unrentable(struct obj_data *obj);

static void Crash_extract_norents(struct obj_data *obj);

static void Crash_extract_expensive(struct obj_data *obj);

static void Crash_calculate_rent(struct obj_data *obj, int *cost);

static int inv_backup(struct char_data *ch);

static int load_inv_backup(struct char_data *ch);


void delete_inv_backup(struct char_data *ch) {
    FILE *source;
    char source_file[20480];
    char alpha[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    sprintf(name, GET_NAME(ch));

    if (name[0] == 'a' || name[0] == 'A' || name[0] == 'b' || name[0] == 'B' || name[0] == 'c' || name[0] == 'C' ||
        name[0] == 'd' || name[0] == 'D' || name[0] == 'e' || name[0] == 'E') {
        sprintf(alpha, "A-E");
    } else if (name[0] == 'f' || name[0] == 'F' || name[0] == 'g' || name[0] == 'G' || name[0] == 'h' ||
               name[0] == 'H' || name[0] == 'i' || name[0] == 'I' || name[0] == 'j' || name[0] == 'J') {
        sprintf(alpha, "F-J");
    } else if (name[0] == 'k' || name[0] == 'K' || name[0] == 'l' || name[0] == 'L' || name[0] == 'm' ||
               name[0] == 'M' || name[0] == 'n' || name[0] == 'N' || name[0] == 'o' || name[0] == 'O') {
        sprintf(alpha, "K-O");
    } else if (name[0] == 'p' || name[0] == 'P' || name[0] == 'q' || name[0] == 'Q' || name[0] == 'r' ||
               name[0] == 'R' || name[0] == 's' || name[0] == 'S' || name[0] == 't' || name[0] == 'T') {
        sprintf(alpha, "P-T");
    } else if (name[0] == 'u' || name[0] == 'U' || name[0] == 'v' || name[0] == 'V' || name[0] == 'w' ||
               name[0] == 'W' || name[0] == 'x' || name[0] == 'X' || name[0] == 'y' || name[0] == 'Y' ||
               name[0] == 'z' || name[0] == 'Z') {
        sprintf(alpha, "U-Z");
    }
    sprintf(source_file, "plrobjs" SLASH"%s" SLASH"%s.copy",
            alpha, ch->name);

    if (!(source = fopen(source_file, "r"))) {
        return;
    }
    fclose(source);

    if (remove(source_file) < 0 && errno != ENOENT)
        basic_mud_log("ERROR: Couldn't delete backup inv.");
    /*  SYSERR_DESC:
     *  When an alias file cannot be removed, this error will occur,
     *  and the reason why will be the tail end of the error.
     */

    return;
}

int load_inv_backup(struct char_data *ch) {
    if (GET_LEVEL(ch) < 2)
        return (-1);

    char chx;
    FILE *source, *target;
    char source_file[20480], target_file[20480], buf2[20480];
    char alpha[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    sprintf(name, GET_NAME(ch));

    if (name[0] == 'a' || name[0] == 'A' || name[0] == 'b' || name[0] == 'B' || name[0] == 'c' || name[0] == 'C' ||
        name[0] == 'd' || name[0] == 'D' || name[0] == 'e' || name[0] == 'E') {
        sprintf(alpha, "A-E");
    } else if (name[0] == 'f' || name[0] == 'F' || name[0] == 'g' || name[0] == 'G' || name[0] == 'h' ||
               name[0] == 'H' || name[0] == 'i' || name[0] == 'I' || name[0] == 'j' || name[0] == 'J') {
        sprintf(alpha, "F-J");
    } else if (name[0] == 'k' || name[0] == 'K' || name[0] == 'l' || name[0] == 'L' || name[0] == 'm' ||
               name[0] == 'M' || name[0] == 'n' || name[0] == 'N' || name[0] == 'o' || name[0] == 'O') {
        sprintf(alpha, "K-O");
    } else if (name[0] == 'p' || name[0] == 'P' || name[0] == 'q' || name[0] == 'Q' || name[0] == 'r' ||
               name[0] == 'R' || name[0] == 's' || name[0] == 'S' || name[0] == 't' || name[0] == 'T') {
        sprintf(alpha, "P-T");
    } else if (name[0] == 'u' || name[0] == 'U' || name[0] == 'v' || name[0] == 'V' || name[0] == 'w' ||
               name[0] == 'W' || name[0] == 'x' || name[0] == 'X' || name[0] == 'y' || name[0] == 'Y' ||
               name[0] == 'z' || name[0] == 'Z') {
        sprintf(alpha, "U-Z");
    }

    sprintf(source_file, "plrobjs" SLASH"%s" SLASH"%s.copy",
            alpha, ch->name);
    if (!get_filename(buf2, sizeof(buf2), NEW_OBJ_FILES, GET_NAME(ch)))
        return -1;
    sprintf(target_file, "%s", buf2);

    if (!(source = fopen(source_file, "r"))) {
        basic_mud_log("Source in load_inv_backup failed to load.");
        basic_mud_log(source_file);
        return -1;
    }

    if (!(target = fopen(target_file, "w"))) {
        basic_mud_log("Target in load_inv_backup failed to load.");
        basic_mud_log(target_file);
        return -1;
    }

    while ((chx = fgetc(source)) != EOF)
        fputc(chx, target);

    basic_mud_log("Inventory backup restore successful.");

    fclose(source);
    fclose(target);

    return 1;
}

static int inv_backup(struct char_data *ch) {
    FILE *backup;
    char buf[20480];

    char alpha[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    sprintf(name, GET_NAME(ch));

    if (name[0] == 'a' || name[0] == 'A' || name[0] == 'b' || name[0] == 'B' || name[0] == 'c' || name[0] == 'C' ||
        name[0] == 'd' || name[0] == 'D' || name[0] == 'e' || name[0] == 'E') {
        sprintf(alpha, "A-E");
    } else if (name[0] == 'f' || name[0] == 'F' || name[0] == 'g' || name[0] == 'G' || name[0] == 'h' ||
               name[0] == 'H' || name[0] == 'i' || name[0] == 'I' || name[0] == 'j' || name[0] == 'J') {
        sprintf(alpha, "F-J");
    } else if (name[0] == 'k' || name[0] == 'K' || name[0] == 'l' || name[0] == 'L' || name[0] == 'm' ||
               name[0] == 'M' || name[0] == 'n' || name[0] == 'N' || name[0] == 'o' || name[0] == 'O') {
        sprintf(alpha, "K-O");
    } else if (name[0] == 'p' || name[0] == 'P' || name[0] == 'q' || name[0] == 'Q' || name[0] == 'r' ||
               name[0] == 'R' || name[0] == 's' || name[0] == 'S' || name[0] == 't' || name[0] == 'T') {
        sprintf(alpha, "P-T");
    } else if (name[0] == 'u' || name[0] == 'U' || name[0] == 'v' || name[0] == 'V' || name[0] == 'w' ||
               name[0] == 'W' || name[0] == 'x' || name[0] == 'X' || name[0] == 'y' || name[0] == 'Y' ||
               name[0] == 'z' || name[0] == 'Z') {
        sprintf(alpha, "U-Z");
    }

    sprintf(buf, "plrobjs" SLASH"%s" SLASH"%s.copy", alpha,
            ch->name);

    if (!(backup = fopen(buf, "r")))
        return -1;

    fclose(backup);

    return 1;
}

int Obj_to_store(struct obj_data *obj, FILE *fl, int location) {
    my_obj_save_to_disk(fl, obj, location);
    return (1);
}

/*
 * AutoEQ by Burkhard Knopf <burkhard.knopf@informatik.tu-clausthal.de>
 */
void auto_equip(struct char_data *ch, struct obj_data *obj, int location) {
    int j;

    /* Lots of checks... */
    if (location > 0) {    /* Was wearing it. */
        switch (j = (location - 1)) {
            case WEAR_UNUSED0:
                j = WEAR_WIELD2;
                break;
            case WEAR_FINGER_R:
            case WEAR_FINGER_L:
                if (!CAN_WEAR(obj, ITEM_WEAR_FINGER)) /* not fitting :( */
                    location = LOC_INVENTORY;
                break;
            case WEAR_NECK_1:
            case WEAR_NECK_2:
                if (!CAN_WEAR(obj, ITEM_WEAR_NECK))
                    location = LOC_INVENTORY;
                break;
            case WEAR_BODY:
                if (!CAN_WEAR(obj, ITEM_WEAR_BODY))
                    location = LOC_INVENTORY;
                break;
            case WEAR_HEAD:
                if (!CAN_WEAR(obj, ITEM_WEAR_HEAD))
                    location = LOC_INVENTORY;
                break;
            case WEAR_LEGS:
                if (!CAN_WEAR(obj, ITEM_WEAR_LEGS))
                    location = LOC_INVENTORY;
                break;
            case WEAR_FEET:
                if (!CAN_WEAR(obj, ITEM_WEAR_FEET))
                    location = LOC_INVENTORY;
                break;
            case WEAR_HANDS:
                if (!CAN_WEAR(obj, ITEM_WEAR_HANDS))
                    location = LOC_INVENTORY;
                break;
            case WEAR_ARMS:
                if (!CAN_WEAR(obj, ITEM_WEAR_ARMS))
                    location = LOC_INVENTORY;
                break;
            case WEAR_UNUSED1:
                if (!CAN_WEAR(obj, ITEM_WEAR_SHIELD))
                    location = LOC_INVENTORY;
                j = WEAR_WIELD2;
                break;
            case WEAR_ABOUT:
                if (!CAN_WEAR(obj, ITEM_WEAR_ABOUT))
                    location = LOC_INVENTORY;
                break;
            case WEAR_WAIST:
                if (!CAN_WEAR(obj, ITEM_WEAR_WAIST))
                    location = LOC_INVENTORY;
                break;
            case WEAR_WRIST_R:
            case WEAR_WRIST_L:
                if (!CAN_WEAR(obj, ITEM_WEAR_WRIST))
                    location = LOC_INVENTORY;
                break;
            case WEAR_WIELD1:
                if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
                    location = LOC_INVENTORY;
                break;
            case WEAR_WIELD2:
                break;
            case WEAR_EYE:
                if (!CAN_WEAR(obj, ITEM_WEAR_EYE))
                    location = LOC_INVENTORY;
                break;
            case WEAR_BACKPACK:
                if (!CAN_WEAR(obj, ITEM_WEAR_PACK))
                    location = LOC_INVENTORY;
                break;
            case WEAR_SH:
                if (!CAN_WEAR(obj, ITEM_WEAR_SH))
                    location = LOC_INVENTORY;
                break;
            case WEAR_EAR_R:
            case WEAR_EAR_L:
                if (!CAN_WEAR(obj, ITEM_WEAR_EAR))
                    location = LOC_INVENTORY;
                break;
            default:
                location = LOC_INVENTORY;
        }

        if (location > 0) {        /* Wearable. */
            if (!GET_EQ(ch, j)) {
                /*
                 * Check the characters's alignment to prevent them from being
                 * zapped through the auto-equipping.
                     */
                if (invalid_align(ch, obj) || invalid_class(ch, obj))
                    location = LOC_INVENTORY;
                else
                    equip_char(ch, obj, j);
            } else {    /* Oops, saved a player with double equipment? */
                mudlog(BRF, ADMLVL_IMMORT, true, "SYSERR: autoeq: '%s' already equipped in position %d.", GET_NAME(ch),
                       location);
                location = LOC_INVENTORY;
            }
        }
    }
    if(world.contains(-1)) {
        basic_mud_log("World contains -1");
    }

    if (location <= 0)    /* Inventory */
        obj_to_char(obj, ch);

    if(world.contains(-1)) {
        basic_mud_log("World contains -1");
    }
}


int Crash_delete_file(char *name) {
    char filename[50];
    FILE *fl;

    //if (!xap_objs) {
    //if (!get_filename(filename, sizeof(filename), CRASH_FILE, name))
    //return (0);
    //} else {
    if (!get_filename(filename, sizeof(filename), NEW_OBJ_FILES, name))
        return (0);
    //}

    if (!(fl = fopen(filename, "rb"))) {
        if (errno != ENOENT)    /* if it fails but NOT because of no file */
            basic_mud_log("SYSERR: deleting crash file %s (1): %s", filename, strerror(errno));
        return (0);
    }
    fclose(fl);

    /* if it fails, NOT because of no file */
    if (remove(filename) < 0 && errno != ENOENT)
        basic_mud_log("SYSERR: deleting crash file %s (2): %s", filename, strerror(errno));

    return (1);
}


int Crash_delete_crashfile(struct char_data *ch) {
    char filename[MAX_INPUT_LENGTH];
    FILE *fl;
    int rentcode, timed, netcost, gold, account, nitems;
    char line[MAX_INPUT_LENGTH];

    if (!get_filename(filename, sizeof(filename), NEW_OBJ_FILES, GET_NAME(ch)))
        return (0);

    if (!(fl = fopen(filename, "rb"))) {
        if (errno != ENOENT)    /* if it fails, NOT because of no file */
            basic_mud_log("SYSERR: checking for crash file %s (3): %s", filename, strerror(errno));
        return (0);
    }

    if (!feof(fl))
        get_line(fl, line);
    sscanf(line, "%d %d %d %d %d %d", &rentcode, &timed, &netcost, &gold,
           &account, &nitems);
    fclose(fl);

    if (rentcode == RENT_CRASH)
        Crash_delete_file(GET_NAME(ch));

    return (1);
}

int Crash_clean_file(char *name) {
    char filename[MAX_STRING_LENGTH];
    FILE *fl;
    int rentcode, timed, netcost, gold, account, nitems;
    char line[MAX_STRING_LENGTH];

    if (!get_filename(filename, sizeof(filename), NEW_OBJ_FILES, name))
        return (0);

    if (!(fl = fopen(filename, "r+b"))) {
        if (errno != ENOENT)    /* if it fails, NOT because of no file */
            basic_mud_log("SYSERR: OPENING OBJECT FILE %s (4): %s", filename, strerror(errno));
        return (0);
    }

    if (!feof(fl)) {
        get_line(fl, line);
        sscanf(line, "%d %d %d %d %d %d", &rentcode, &timed, &netcost,
               &gold, &account, &nitems);
        fclose(fl);

        if ((rentcode == RENT_CRASH) ||
            (rentcode == RENT_FORCED) || (rentcode == RENT_TIMEDOUT)) {
            if (timed < time(nullptr) - (CONFIG_CRASH_TIMEOUT * SECS_PER_REAL_DAY)) {
                const char *filetype;

                Crash_delete_file(name);
                switch (rentcode) {
                    case RENT_CRASH:
                        filetype = "crash";
                        break;
                    case RENT_FORCED:
                        filetype = "forced rent";
                        break;
                    case RENT_TIMEDOUT:
                        filetype = "idlesave";
                        break;
                    default:
                        filetype = "UNKNOWN!";
                        break;
                }
                basic_mud_log("    Deleting %s's %s file.", name, filetype);
                return (1);
            }
            /* Must retrieve rented items w/in 30 days */
        } else if (rentcode == RENT_RENTED)
            if (timed < time(nullptr) - (CONFIG_RENT_TIMEOUT * SECS_PER_REAL_DAY)) {
                Crash_delete_file(name);
                basic_mud_log("    Deleting %s's rent file.", name);
                return (1);
            }
    }
    return (0);
}


void update_obj_file() {
    int i;

    for (auto &[id, p] : players)
        Crash_clean_file((char*)p.name.c_str());
}


void Crash_listrent(struct char_data *ch, char *name) {
    FILE *fl = nullptr;
    char filename[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *obj;
    int rentcode, timed, netcost, gold, account, nitems, len;
    int t[10], nr;
    char line[MAX_STRING_LENGTH];
    char *sdesc;

    if (get_filename(filename, sizeof(filename), NEW_OBJ_FILES, name))
        fl = fopen(filename, "rb");
    if (!fl) {
        send_to_char(ch, "%s has no rent file.\r\n", name);
        return;
    }

    send_to_char(ch, "%s\r\n", filename);

    if (!feof(fl)) {
        get_line(fl, line);
        sscanf(line, "%d %d %d %d %d %d", &rentcode, &timed, &netcost,
               &gold, &account, &nitems);
    }

    switch (rentcode) {
        case RENT_RENTED:
            send_to_char(ch, "Rent\r\n");
            break;
        case RENT_CRASH:
            send_to_char(ch, "Crash\r\n");
            break;
        case RENT_CRYO:
            send_to_char(ch, "Cryo\r\n");
            break;
        case RENT_TIMEDOUT:
        case RENT_FORCED:
            send_to_char(ch, "TimedOut\r\n");
            break;
        default:
            send_to_char(ch, "Undef\r\n");
            break;
    }
    buf[0] = 0;
    len = 0;

    while (!feof(fl)) {
        get_line(fl, line);
        if (*line == '#') { /* swell - its an item */
            sscanf(line, "#%d", &nr);
            if (nr != NOTHING) {  /* then we can dispense with it easily */
                if (real_object(nr) != NOTHING) {
                    obj = read_object(nr, VIRTUAL);
                    if (len + 255 < sizeof(buf)) {
                        len += snprintf(buf + len, sizeof(buf) - len, "[%5d] (%5dau) %-20s\r\n",
                                        nr, GET_OBJ_RENT(obj), obj->short_description);
                    } else {
                        snprintf(buf + len, sizeof(buf) - len, "** Excessive rent listing. **\r\n");
                        break;
                    }
                    extract_obj(obj);
                } else {
                    if (len + 255 < sizeof(buf)) {
                        len += snprintf(buf + len, sizeof(buf) - len,
                                        "%s[-----] NONEXISTANT OBJECT #%d\r\n", buf, nr);
                    } else {
                        snprintf(buf + len, sizeof(buf) - len, "** Excessive rent listing. **\r\n");
                        break;
                    }
                }
            } else { /* its nothing, and a unique item. bleh. partial parse.*/
                get_line(fl, line);    /* this is obj+val */
                get_line(fl, line);    /* this is XAP */
                fread_string(fl, ", listrent reading name");  /* screw the name */
                sdesc = fread_string(fl, ", listrent reading sdesc");
                fread_string(fl, ", listrent reading desc"); /* screw the long desc */
                fread_string(fl, ", listrent reading adesc"); /* screw the action desc. */
                get_line(fl, line);    /* this is an important line.rent..*/
                sscanf(line, "%ld %ld %ld %ld %ld", t, t + 1, t + 2, t + 3, t + 4);
                /* great we got it all, make the buf */
                if (len + 255 < sizeof(buf)) {
                    len += snprintf(buf + len, sizeof(buf) - len,
                                    "%s[%5d] (%5dau) %-20s\r\n", buf, nr, t[4], sdesc);
                } else {
                    snprintf(buf + len, sizeof(buf) - len, "** Excessive rent listing. **\r\n");
                    break;
                }
                /* best of all, we don't care if there's descs, or stuff..*/
                /* since we're only doing operations on lines beginning in # */
                /* i suppose you don't want to make exdescs start with # .:) */
            }
        }
    }

    write_to_output(ch->desc, buf);
    fclose(fl);
}


static int Crash_save(struct obj_data *obj, FILE *fp, int location) {
    return 0;
}


static void Crash_restore_weight(struct obj_data *obj) {

}

/*
 * Get !RENT items from equipment to inventory and
 * extract !RENT out of worn containers.
 */
void Crash_extract_norent_eq(struct char_data *ch) {
    int j;

    for (j = 0; j < NUM_WEARS; j++) {
        if (GET_EQ(ch, j) == nullptr)
            continue;

        if (Crash_is_unrentable(GET_EQ(ch, j)))
            obj_to_char(unequip_char(ch, j), ch);
        else
            Crash_extract_norents(GET_EQ(ch, j));
    }
}

static void Crash_extract_objs(struct obj_data *obj) {
    if (obj) {
        Crash_extract_objs(obj->contents);
        Crash_extract_objs(obj->next_content);
        extract_obj(obj);
    }
}


static int Crash_is_unrentable(struct obj_data *obj) {
    if (!obj)
        return (0);
#if CIRCLE_UNSIGNED_INDEX
    if (OBJ_FLAGGED(obj, ITEM_NORENT) || GET_OBJ_RENT(obj) < 0 ||
        (GET_OBJ_RNUM(obj) == NOTHING &&
         !IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE))) {
        return (1);
#else
        if (OBJ_FLAGGED(obj, ITEM_NORENT) || GET_OBJ_RENT(obj) < 0 ||
                (GET_OBJ_RNUM(obj) <= NOTHING &&
                !IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE))) {
          return (1);
#endif
    }
    return (0);
}


static void Crash_extract_norents(struct obj_data *obj) {
    if (obj) {
        Crash_extract_norents(obj->contents);
        Crash_extract_norents(obj->next_content);
        if (Crash_is_unrentable(obj))
            extract_obj(obj);
    }
}


static void Crash_calculate_rent(struct obj_data *obj, int *cost) {
    if (obj) {
        *cost += MAX(0, GET_OBJ_RENT(obj));
        Crash_calculate_rent(obj->contents, cost);
        Crash_calculate_rent(obj->next_content, cost);
    }
}


void Crash_crashsave(struct char_data *ch) {
    return;
}


/* ************************************************************************
* Routines used for the receptionist					  *
************************************************************************* */

static void Crash_rent_deadline(struct char_data *ch, struct char_data *recep,
                                long cost) {
    char buf[256];
    long rent_deadline;

    if (!cost)
        return;

    rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / cost);
    snprintf(buf, sizeof(buf), "$n tells you, 'You can rent for %ld day%s with the gold you have\r\n"
                               "on hand and in the bank.'\r\n", rent_deadline, rent_deadline != 1 ? "s" : "");
    act(buf, false, recep, nullptr, ch, TO_VICT);
}

static int Crash_report_unrentables(struct char_data *ch, struct char_data *recep,
                                    struct obj_data *obj) {
    int has_norents = 0;

    if (obj) {
        if (Crash_is_unrentable(obj)) {
            char buf[128];

            has_norents = 1;
            snprintf(buf, sizeof(buf), "$n tells you, 'You cannot store %s.'", OBJS(obj, ch));
            act(buf, false, recep, nullptr, ch, TO_VICT);
        }
        has_norents += Crash_report_unrentables(ch, recep, obj->contents);
        has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
    }
    return (has_norents);
}


static void Crash_report_rent(struct char_data *ch, struct char_data *recep,
                              struct obj_data *obj, long *cost, long *nitems, int display, int factor) {
    if (obj) {
        if (!Crash_is_unrentable(obj)) {
            (*nitems)++;
            *cost += MAX(0, (GET_OBJ_RENT(obj) * factor));
            if (display) {
                char buf[256];

                snprintf(buf, sizeof(buf), "$n tells you, '%5d zenni for %s..'", GET_OBJ_RENT(obj) * factor,
                         OBJS(obj, ch));
                act(buf, false, recep, nullptr, ch, TO_VICT);
            }
        }
        Crash_report_rent(ch, recep, obj->contents, cost, nitems, display, factor);
        Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
    }
}


static int Crash_offer_rent(struct char_data *ch, struct char_data *recep,
                            int display, int factor) {
    int i;
    long totalcost = 0, numitems = 0, norent;

    norent = Crash_report_unrentables(ch, recep, ch->contents);
    for (i = 0; i < NUM_WEARS; i++)
        norent += Crash_report_unrentables(ch, recep, GET_EQ(ch, i));

    if (norent)
        return (0);

    totalcost = CONFIG_MIN_RENT_COST * factor;

    Crash_report_rent(ch, recep, ch->contents, &totalcost, &numitems, display, factor);

    for (i = 0; i < NUM_WEARS; i++)
        Crash_report_rent(ch, recep, GET_EQ(ch, i), &totalcost, &numitems, display, factor);

    if (!numitems) {
        act("$n tells you, 'But you are not carrying anything!  Just quit!'",
            false, recep, nullptr, ch, TO_VICT);
        return (0);
    }
    if (numitems > CONFIG_MAX_OBJ_SAVE) {
        char buf[256];

        snprintf(buf, sizeof(buf), "$n tells you, 'Sorry, but I cannot store more than %d items.'",
                 CONFIG_MAX_OBJ_SAVE);
        act(buf, false, recep, nullptr, ch, TO_VICT);
        return (0);
    }
    if (display) {
        char buf[256];

        snprintf(buf, sizeof(buf), "$n tells you, 'Plus, my %d zenni fee..'", CONFIG_MIN_RENT_COST * factor);
        act(buf, false, recep, nullptr, ch, TO_VICT);

        snprintf(buf, sizeof(buf), "$n tells you, 'For a total of %ld zenni.'", totalcost);
        act(buf, false, recep, nullptr, ch, TO_VICT);

        if (totalcost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
            act("$n tells you, '...which I see you can't afford.'", false, recep, nullptr, ch, TO_VICT);
            return (0);
        } else if (factor == RENT_FACTOR)
            Crash_rent_deadline(ch, recep, totalcost);
    }
    return (totalcost);
}


static int gen_receptionist(struct char_data *ch, struct char_data *recep,
                            int cmd, char *arg, int mode) {
    int cost;
    const char *action_table[] = {"smile", "dance", "sigh", "blush", "burp",
                                  "cough", "fart", "twiddle", "yawn"};

    if (!cmd && !rand_number(0, 5)) {
        do_action(recep, nullptr, find_command(action_table[rand_number(0, 8)]), 0);
        return (false);
    }

    if (!ch->desc || IS_NPC(ch))
        return (false);

    if (!CMD_IS("offer") && !CMD_IS("rent"))
        return (false);

    if (!AWAKE(recep)) {
        send_to_char(ch, "%s is unable to talk to you...\r\n", HSSH(recep));
        return (true);
    }

    if (!CAN_SEE(recep, ch)) {
        act("$n says, 'I don't deal with people I can't see!'", false, recep, nullptr, nullptr, TO_ROOM);
        return (true);
    }

    if (CONFIG_FREE_RENT) {
        act("$n tells you, 'Rent is free here.  Just quit, and your objects will be saved!'",
            false, recep, nullptr, ch, TO_VICT);
        return (1);
    }

    if (CMD_IS("rent")) {
        char buf[128];

        if (!(cost = Crash_offer_rent(ch, recep, false, mode)))
            return (true);
        if (mode == RENT_FACTOR)
            snprintf(buf, sizeof(buf), "$n tells you, 'Rent will cost you %d zenni per day.'", cost);
        else if (mode == CRYO_FACTOR)
            snprintf(buf, sizeof(buf), "$n tells you, 'It will cost you %d zenni to be frozen.'", cost);
        act(buf, false, recep, nullptr, ch, TO_VICT);

        if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
            act("$n tells you, '...which I see you can't afford.'",
                false, recep, nullptr, ch, TO_VICT);
            return (true);
        }
        if (cost && (mode == RENT_FACTOR))
            Crash_rent_deadline(ch, recep, cost);

        if (mode == RENT_FACTOR) {
            act("$n stores your belongings and helps you into your private chamber.", false, recep, nullptr, ch,
                TO_VICT);
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "%s has rented (%d/day, %d tot.)",
                   GET_NAME(ch), cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
        } else {            /* cryo */
            act("$n stores your belongings and helps you into your private chamber.\r\n"
                "A white mist appears in the room, chilling you to the bone...\r\n"
                "You begin to lose consciousness...",
                false, recep, nullptr, ch, TO_VICT);
            //Crash_cryosave(ch, cost);
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "%s has cryo-rented.", GET_NAME(ch));
            ch->playerFlags.set(PLR_CRYO);
        }

        act("$n helps $N into $S private chamber.", false, recep, nullptr, ch, TO_NOTVICT);

        extract_char(ch);    /* It saves. */
    } else {
        Crash_offer_rent(ch, recep, true, mode);
        act("$N gives $n an offer.", false, ch, nullptr, recep, TO_ROOM);
    }
    return (true);
}


SPECIAL(receptionist) {
    return (gen_receptionist(ch, (struct char_data *) me, cmd, argument, RENT_FACTOR));
}


SPECIAL(cryogenicist) {
    return (gen_receptionist(ch, (struct char_data *) me, cmd, argument, CRYO_FACTOR));
}


void Crash_save_all(uint64_t heartPulse, double deltaTime) {
    struct descriptor_data *d;
    for (d = descriptor_list; d; d = d->next) {
        if ((STATE(d) == CON_PLAYING) && !IS_NPC(d->character)) {
            if (PLR_FLAGGED(d->character, PLR_CRASH)) {
                Crash_crashsave(d->character);
                d->character->playerFlags.reset(PLR_CRASH);
            }
        }
    }
}

static std::set<struct char_data*> crashLoaded;

int Crash_load(struct char_data *ch) {
    if(crashLoaded.contains(ch)) {
        logger->critical("OOPS!");
    }
    crashLoaded.insert(ch);
    FILE *fl;
    char cmfname[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char line[256];
    int64_t t[30], danger, zwei = 0, num_of_days;
    int orig_rent_code;
    struct obj_data *temp;
    int locate = 0, j, nr, k, cost, num_objs = 0;
    struct obj_data *obj1;
    struct obj_data *cont_row[MAX_BAG_ROWS];
    struct extra_descr_data *new_descr;
    int rentcode, timed, netcost, gold, account, nitems;
    char f1[READ_SIZE], f2[READ_SIZE], f3[READ_SIZE], f4[READ_SIZE];

    if (!get_filename(cmfname, sizeof(cmfname), NEW_OBJ_FILES, GET_NAME(ch)))
        return 1;

    if (!(fl = fopen(cmfname, "r+b"))) {
        if (errno != ENOENT) {    /* if it fails, NOT because of no file */
            sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", cmfname);
            perror(buf1);
            send_to_char(ch,
                         "\r\n********************* NOTICE *********************\r\n"
                         "There was a problem loading your objects from disk.\r\n"
                         "Contact a God for assistance.\r\n");
        }
        if (GET_LEVEL(ch) > 1) {
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true,
                   "%s entering game with no equipment. Loading backup.", GET_NAME(ch));
        }
        if (!inv_backup(ch)) {
            return -1;
        } else {
            if (!load_inv_backup(ch))
                return -1;
            else if (!(fl = fopen(cmfname, "r+b"))) {
                if (errno != ENOENT) {      /* if it fails, NOT because of no file */
                    sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", cmfname);
                    perror(buf1);
                    send_to_char(ch,
                                 "\r\n********************* NOTICE *********************\r\n"
                                 "There was a problem loading your objects from disk.\r\n"
                                 "Contact a God for assistance.\r\n");
                }
                return -1;
            }
        }
    }

    if (!feof(fl))
        get_line(fl, line);

    sscanf(line, "%d %d %d %d %d %d", &rentcode, &timed,
           &netcost, &gold, &account, &nitems);

    if (rentcode == RENT_RENTED || rentcode == RENT_TIMEDOUT) {
        num_of_days = (float) (time(nullptr) - timed) / SECS_PER_REAL_DAY;
        cost = 0;
    }
    switch (orig_rent_code = rentcode) {
        case RENT_RENTED:
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "%s un-renting and entering game.", GET_NAME(ch));
            break;
        case RENT_CRASH:
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true,
                   "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
            break;
        case RENT_CRYO:
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "%s un-cryo'ing and entering game.", GET_NAME(ch));
            break;
        case RENT_FORCED:
        case RENT_TIMEDOUT:
            mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true,
                   "%s retrieving force-saved items and entering game.", GET_NAME(ch));
            break;
        default:
            mudlog(BRF, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true,
                   "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
            break;
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
                GET_OBJ_SIZE(temp) = SIZE_MEDIUM;
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

            get_line(fl, line);
            /* read line check for xap. */
            if (!strcmp("XAP", line)) {  /* then this is a Xap Obj, requires
                                       special care */
                if ((temp->name = fread_string(fl, "rented object name")) == nullptr) {
                    temp->name = "undefined";
                }

                if ((temp->short_description = fread_string(fl, "rented object short desc")) == nullptr) {
                    temp->short_description = "undefined";
                }

                if ((temp->room_description = fread_string(fl, "rented object desc")) == nullptr) {
                    temp->room_description = "undefined";
                }

                if ((temp->look_description = fread_string(fl, "rented object adesc")) == nullptr) {
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

                /* we're clearing these for good luck */

                for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                    temp->affected[j].location = APPLY_NONE;
                    temp->affected[j].modifier = 0;
                    temp->affected[j].specific = 0;
                }

                /* maybe clear spellbook for good luck too? */
                if (GET_OBJ_TYPE(temp) == ITEM_SPELLBOOK) {
                    if (!temp->sbinfo) {
                        CREATE(temp->sbinfo, struct obj_spellbook_spell, SPELLBOOK_SIZE);
                        memset((char *) temp->sbinfo, 0, SPELLBOOK_SIZE * sizeof(struct obj_spellbook_spell));
                    }
                    for (j = 0; j < SPELLBOOK_SIZE; j++) {
                        temp->sbinfo[j].spellname = 0;
                        temp->sbinfo[j].pages = 0;
                    }
                    temp->sbinfo[0].spellname = SPELL_DETECT_MAGIC;
                    temp->sbinfo[0].pages = 1;
                }

                temp->ex_description = nullptr;

                get_line(fl, line);
                for (k = j = zwei = 0; !zwei && !feof(fl);) {
                    switch (*line) {
                        case 'E':
                            CREATE(new_descr, struct extra_descr_data, 1);
                            sprintf(buf2, "rented object edesc keyword for object #%d", nr);
                            new_descr->keyword = fread_string(fl, buf2);
                            sprintf(buf2, "rented object edesc text for object #%d keyword %s", nr, new_descr->keyword);
                            new_descr->description = fread_string(fl, buf2);
                            new_descr->next = temp->ex_description;
                            temp->ex_description = new_descr;
                            get_line(fl, line);
                            break;
                        case 'A':
                            if (j >= MAX_OBJ_AFFECT) {
                                basic_mud_log("SYSERR: Too many object affectations in loading rent file");
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
                num_objs++;
                check_unique_id(temp);
                add_unique_id(temp);
                temp->activate();

                if (GET_OBJ_TYPE(temp) == ITEM_DRINKCON) {
                    name_from_drinkcon(temp);
                    if (GET_OBJ_VAL(temp, 1) != 0)
                        name_to_drinkcon(temp, GET_OBJ_VAL(temp, 2));
                }
                if (GET_OBJ_VNUM(temp) == 20099 || GET_OBJ_VNUM(temp) == 20098)
                    if (OBJ_FLAGGED(temp, ITEM_UNBREAKABLE))
                        temp->extra_flags.reset(ITEM_UNBREAKABLE);
                auto_equip(ch, temp, locate);
            } else {
                continue;
            }
            /*
              what to do with a new loaded item:

              if there's a list with <locate> less than 1 below this:
                (equipped items are assumed to have <locate>==0 here) then its
                container has disappeared from the file   *gasp*
                 -> put all the list back to ch's inventory
              if there's a list of contents with <locate> 1 below this:
                check if it's a container
                - if so: get it from ch, fill it, and give it back to ch (this way the
                    container has its correct weight before modifying ch)
                - if not: the container is missing -> put all the list to ch's inventory

              for items with negative <locate>:
                if there's already a list of contents with the same <locate> put obj to it
                if not, start a new list

            Confused? Well maybe you can think of some better text to be put here ...

            since <locate> for contents is < 0 the list indices are switched to
            non-negative
            */

            if (locate > 0) { /* item equipped */
                for (j = MAX_BAG_ROWS - 1; j > 0; j--)
                    if (cont_row[j]) { /* no container -> back to ch's inventory */
                        for (; cont_row[j]; cont_row[j] = obj1) {
                            obj1 = cont_row[j]->next_content;
                            obj_to_char(cont_row[j], ch);
                        }
                        cont_row[j] = nullptr;
                    }
                if (cont_row[0]) { /* content list existing */
                    if (GET_OBJ_TYPE(temp) == ITEM_CONTAINER) {
                        /* rem item ; fill ; equip again */
                        temp = unequip_char(ch, locate - 1);
                        temp->contents = nullptr; /* should be empty - but who knows */
                        for (; cont_row[0]; cont_row[0] = obj1) {
                            obj1 = cont_row[0]->next_content;
                            obj_to_obj(cont_row[0], temp);
                        }
                        equip_char(ch, temp, locate - 1);
                    } else { /* object isn't container -> empty content list */
                        for (; cont_row[0]; cont_row[0] = obj1) {
                            obj1 = cont_row[0]->next_content;
                            obj_to_char(cont_row[0], ch);
                        }
                        cont_row[0] = nullptr;
                    }
                }
            } else { /* locate <= 0 */
                for (j = MAX_BAG_ROWS - 1; j > -locate; j--)
                    if (cont_row[j]) { /* no container -> back to ch's inventory */
                        for (; cont_row[j]; cont_row[j] = obj1) {
                            obj1 = cont_row[j]->next_content;
                            obj_to_char(cont_row[j], ch);
                        }
                        cont_row[j] = nullptr;
                    }

                if (j == -locate && cont_row[j]) { /* content list existing */
                    if (GET_OBJ_TYPE(temp) == ITEM_CONTAINER) {
                        /* take item ; fill ; give to char again */
                        obj_from_char(temp);
                        temp->contents = nullptr;
                        for (; cont_row[j]; cont_row[j] = obj1) {
                            obj1 = cont_row[j]->next_content;
                            obj_to_obj(cont_row[j], temp);
                        }
                        obj_to_char(temp, ch); /* add to inv first ... */
                    } else { /* object isn't container -> empty content list */
                        for (; cont_row[j]; cont_row[j] = obj1) {
                            obj1 = cont_row[j]->next_content;
                            obj_to_char(cont_row[j], ch);
                        }
                        cont_row[j] = nullptr;
                    }
                }

                if (locate < 0 && locate >= -MAX_BAG_ROWS) {
                    /* let obj be part of content list
                       but put it at the list's end thus having the items
                       in the same order as before renting */
                    obj_from_char(temp);
                    if ((obj1 = cont_row[-locate - 1])) {
                        while (obj1->next_content)
                            obj1 = obj1->next_content;
                        obj1->next_content = temp;
                    } else
                        cont_row[-locate - 1] = temp;
                }
            } /* locate less than zero */
        } else {
            get_line(fl, line);
        }
    }

    /* Little hoarding check. -gg 3/1/98 */
    mudlog(NRM, MAX(ADMLVL_IMMORT, GET_INVIS_LEV(ch)), true, "%s (level %d) has %d objects (max %d).",
           GET_NAME(ch), GET_LEVEL(ch), num_objs, CONFIG_MAX_OBJ_SAVE);

    fclose(fl);

    Crash_crashsave(ch);

    if ((orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO))
        return 0;
    else
        return 1;
}
