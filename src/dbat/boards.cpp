/* ************************************************************************
*   File: boards.c                                      Part of CircleMUD *
*  Usage: handling of multiple bulletin boards                            *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* FEATURES & INSTALLATION INSTRUCTIONS ***********************************

This board code has many improvements over the infamously buggy standard
Diku board code.  Features include:

- Arbitrary number of boards handled by one set of generalized routines.
  Adding a new board is as easy as adding another entry to an array.
- Safe removal of messages while other messages are being written.
- Does not allow messages to be removed by someone of a level less than
  the poster's level.


TO ADD A NEW BOARD, simply follow our easy 4-step program:

1 - Create a new board object in the object files

2 - Increase the NUM_OF_BOARDS constant in boards.h

3 - Add a new line to the board_info array below.  The fields, in order, 
are:

	Board's virtual number.
	Min level one must be to look at this board or read messages on 
it.
	Min level one must be to post a message to the board.
	Min level one must be to remove other people's messages from this
		board (but you can always remove your own message).
	Filename of this board, in quotes.
	Last field must always be 0.

4 - In spec_assign.c, find the section which assigns the special procedure
    gen_board to the other bulletin boards, and add your new one in a
    similar fashion.

*/

#include "dbat/boards.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/boards.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"
#include "dbat/improved-edit.h"
#include "dbat/clan.h"
#include "dbat/dg_comm.h"
#include "dbat/config.h"

struct board_info *bboards = nullptr;  /* our global board structure */

void init_boards() {
    int i, j;
    vnum board_vnum;
    struct board_info *tmp_board;
    char dir_name[128];

    auto path = std::filesystem::current_path() / "etc" / "boards";
    std::filesystem::create_directory(path);

    // Iterate over all files within path that have integer filenames.
    for(auto &p: std::filesystem::directory_iterator(path)) {
        if(p.is_regular_file()) {
            try {
                board_vnum = std::stoi(p.path().filename().string());
            } catch(std::invalid_argument &e) {
                continue;
            }

            if ((tmp_board = load_board(board_vnum)) != nullptr) {
                tmp_board->next = bboards;
                bboards = tmp_board;
            }
        }
    }

    /* just logs some summary data about the boards */
    look_at_boards();
}

struct board_info *create_new_board(obj_vnum board_vnum) {
    char buf[512];
    FILE *fl;
    struct board_info *temp = nullptr, *backup;
    struct obj_data *obj = nullptr;

    /* object exists, but no board file (yet) */

    if ((fl = fopen(buf, "r"))) {
        fclose(fl);
        basic_mud_log("Preexisting board file when attempting to create new board [vnum: %d]. Attempting to correct.",
            board_vnum);

        /* unlink file, clear existing board */
        std::filesystem::remove(buf);

        for (temp = bboards, backup = nullptr; temp && !backup; temp = temp->next) {
            if (BOARD_VNUM(temp) == board_vnum) {
                backup = temp;
            }
        }
        if (backup) {
            REMOVE_FROM_LIST(backup, bboards, next, temp);
            clear_one_board(backup);
        }
    }
    CREATE(temp, struct board_info, 1);
    if (real_object(board_vnum) == NOTHING) {
        basic_mud_log("Creating board [vnum: %d] though no associated object with that vnum can be found. Using defaults.",
            board_vnum);
        READ_LVL(temp) = CONFIG_LEVEL_CAP;
        WRITE_LVL(temp) = CONFIG_LEVEL_CAP;
        REMOVE_LVL(temp) = CONFIG_LEVEL_CAP;
    } else {
        obj = &(obj_proto[real_object(board_vnum)]);
        READ_LVL(temp) = GET_OBJ_VAL(obj, VAL_BOARD_READ);
        WRITE_LVL(temp) = GET_OBJ_VAL(obj, VAL_BOARD_WRITE);
        REMOVE_LVL(temp) = GET_OBJ_VAL(obj, VAL_BOARD_ERASE);
    }
    BOARD_VNUM(temp) = board_vnum;
    BOARD_MNUM(temp) = 0;
    BOARD_VERSION(temp) = CURRENT_BOARD_VER;
    temp->next = nullptr;
    BOARD_MESSAGES(temp) = nullptr;

    if (!save_board(temp)) {
        basic_mud_log("Hm. Error while creating new board file [vnum: %d]. Unable to create new file.", board_vnum);
        free(temp);
        return nullptr;
    }
    return temp;
}

int save_board(struct board_info *ts) {
    struct board_msg *message;
    struct board_memory *memboard;
    FILE *fl;
    char buf[512];
    int i = 1;

    sprintf(buf, "%s%d", BOARD_DIRECTORY, BOARD_VNUM(ts));

    if (!(fl = fopen(buf, "wb"))) {
        basic_mud_log("Hm. Error while creating attempting to save board [vnum: %d].  Unable to create file '%s'", BOARD_VNUM(ts),
            buf);
        return 0;
    }

    fprintf(fl, "Board File\n%d %d %d %d %d\n", READ_LVL(ts),
            WRITE_LVL(ts), REMOVE_LVL(ts), BOARD_MNUM(ts), CURRENT_BOARD_VER);

    for (message = BOARD_MESSAGES(ts); message; message = MESG_NEXT(message)) {
        if (BOARD_VERSION(ts) != CURRENT_BOARD_VER)
            MESG_POSTER_NAME(message) = get_name_by_id(MESG_POSTER(message));
        if (message)
            fprintf(fl, "#%d\n"
                        "%s\n"
                        "%ld\n"
                        "%s\n"
                        "%s~\n",
                    i++, MESG_POSTER_NAME(message), MESG_TIMESTAMP(message),
                    MESG_SUBJECT(message), MESG_DATA(message));
    }
    /* now write out the saved info.. */
    for (i = 0; i != 301; i++) {
        memboard = BOARD_MEMORY(ts, i);
        while (memboard) {
            fprintf(fl, "S%d %s %d\n", i, MEMORY_READER_NAME(memboard),
                    +MEMORY_TIMESTAMP(memboard));
            memboard = MEMORY_NEXT(memboard);
        }
    }
    fclose(fl);
    return 1;
}

/* a fairly messy function                         */
/* see accompanying document for board file format */

struct board_info *load_board(obj_vnum board_vnum) {
    struct board_info *temp_board = nullptr;
    struct board_msg *bmsg = nullptr;
    struct obj_data *obj = nullptr;
    struct stat st{};
    struct board_memory *memboard = nullptr, *list = nullptr;
    int t[10], mnum, poster = 0, timestamp = 0, msg_num = 0, retval = 0;
    char filebuf[512], buf[512], poster_name[128];
    FILE *fl = nullptr;
    int sflag = 0;

    sprintf(filebuf, "%s%d", BOARD_DIRECTORY, board_vnum);
    if (!(fl = fopen(filebuf, "r"))) {
        basic_mud_log("Request to open board [vnum %d] failed - unable to open file '%s'.", board_vnum, filebuf);
        return nullptr;
    }
    /* this won't be the most graceful thing you've ever seen .. */
    get_line(fl, buf);
    if (strcmp("Board File", buf)) {
        basic_mud_log("Invalid board file '%s' [vnum: %d] - failed to load.", filebuf, board_vnum);
        return nullptr;
    }

    CREATE(temp_board, struct board_info, 1);
    temp_board->vnum = board_vnum;
    get_line(fl, buf);
    /* oddly enough, most errors in board files can be ignored, setting
       defaults */

    if ((retval = sscanf(buf, "%d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4)) != 5) {
        if (retval == 4) {
            basic_mud_log("Parse error on board [vnum: %d], file '%s' - attempting to correct [4] args expecting 5.", board_vnum,
                filebuf);
            t[4] = 1;
        } else if (retval != 4) {
            basic_mud_log("Parse error on board [vnum: %d], file '%s' - attempting to correct [< 4] args expecting 5.",
                board_vnum, filebuf);
            t[0] = t[1] = t[2] = CONFIG_LEVEL_CAP;
            t[3] = -1;
            t[4] = 1;
        }
    }
    /* if the objcet exists, the object trumps the board file settings */

    if (real_object(board_vnum) == NOTHING) {
        basic_mud_log("No associated object exists when attempting to create a board [vnum %d].", board_vnum);
        /* previously we just erased it, but lets do a tiny bit of checking, just in case           */
        /* auto delete only if the file has hasn't been modified in the last 7 days */


        stat(filebuf, &st);
        if (time(nullptr) - st.st_mtime > (60 * 60 * 24 * 7)) {
            basic_mud_log("Deleting old board file '%s' [vnum %d].  7 days without modification & no associated object.", filebuf,
                board_vnum);
            std::filesystem::remove(filebuf);
            free(temp_board);
            return nullptr;
        }
        READ_LVL(temp_board) = t[0];
        WRITE_LVL(temp_board) = t[1];
        REMOVE_LVL(temp_board) = t[2];
        BOARD_MNUM(temp_board) = t[3];
        BOARD_VERSION(temp_board) = t[4];
        basic_mud_log("Board vnum %d, Version %d", BOARD_VNUM(temp_board), BOARD_VERSION(temp_board));
    } else {
        obj = &(obj_proto[real_object(board_vnum)]);
        /* double check one or two things */
        if (t[0] != GET_OBJ_VAL(obj, VAL_BOARD_READ) ||
            t[1] != GET_OBJ_VAL(obj, VAL_BOARD_WRITE) ||
            t[2] != GET_OBJ_VAL(obj, VAL_BOARD_ERASE)) {
            basic_mud_log("Mismatch in board <-> object read/write/remove settings for board [vnum: %d]. Correcting.",
                board_vnum);
        }
        READ_LVL(temp_board) = GET_OBJ_VAL(obj, VAL_BOARD_READ);
        WRITE_LVL(temp_board) = GET_OBJ_VAL(obj, VAL_BOARD_WRITE);
        REMOVE_LVL(temp_board) = GET_OBJ_VAL(obj, VAL_BOARD_ERASE);
        BOARD_MNUM(temp_board) = t[3];
        BOARD_VERSION(temp_board) = t[4];
    }

    BOARD_NEXT(temp_board) = nullptr;
    BOARD_MESSAGES(temp_board) = nullptr;

    /* now loop and parse messages and memory */
    msg_num = 0;
    while (get_line(fl, buf)) {
        if (*buf == 'S' && BOARD_VERSION(temp_board) != CURRENT_BOARD_VER) {
            if (sscanf(buf, "S %d %d %d ", &mnum, &poster, &timestamp) == 3) {
                CREATE(memboard, struct board_memory, 1);
                MEMORY_READER(memboard) = poster;
                MEMORY_TIMESTAMP(memboard) = timestamp;
            }
        } else if (*buf == 'S' && BOARD_VERSION(temp_board) == CURRENT_BOARD_VER) {
            if (sscanf(buf, "S %d %s %d ", &mnum, poster_name, &timestamp) == 3) {
                CREATE(memboard, struct board_memory, 1);
                MEMORY_READER_NAME(memboard) = strdup(poster_name);
                MEMORY_TIMESTAMP(memboard) = timestamp;
                /* now, validate the memory => insure that for this slot, id, and timestamp there
                   is a valid message, and poster.  Memory is deleted for mundane reasons; character
                   deletions, message deletions, etc.  'Failures' will not be logged */
                if ((get_name_by_id(poster) == nullptr) && (BOARD_VERSION(temp_board) != CURRENT_BOARD_VER)) {
                    free(memboard);
                } else if ((poster_name == nullptr) && (BOARD_VERSION(temp_board) == CURRENT_BOARD_VER)) {
                    free(memboard);
                } else {
                    /* locate specific message this pertains to - therefore, messages MUST be loaded first! */

                    if (BOARD_VERSION(temp_board) == CURRENT_BOARD_VER) {
                        for (bmsg = BOARD_MESSAGES(temp_board), sflag = 0; bmsg && !sflag; bmsg = MESG_NEXT(bmsg)) {
                            if (MESG_TIMESTAMP(bmsg) == MEMORY_TIMESTAMP(memboard)
                                && (mnum == ((MESG_TIMESTAMP(bmsg) % 301 +
                                              get_id_by_name(MESG_POSTER_NAME(bmsg)) % 301) % 301))) {
                                sflag = 1;
                            }
                        }
                    } else {
                        for (bmsg = BOARD_MESSAGES(temp_board), sflag = 0; bmsg && !sflag; bmsg = MESG_NEXT(bmsg)) {
                            if (MESG_TIMESTAMP(bmsg) == MEMORY_TIMESTAMP(memboard)
                                && (mnum == ((MESG_TIMESTAMP(bmsg) % 301 +
                                              MESG_POSTER(bmsg) % 301) % 301))) {
                                sflag = 1;
                            }
                        }
                    }
                    if (sflag) {
                        if (BOARD_MEMORY(temp_board, mnum)) {
                            list = BOARD_MEMORY(temp_board, mnum);
                            BOARD_MEMORY(temp_board, mnum) = memboard;
                            MEMORY_NEXT(memboard) = list;
                        } else {
                            BOARD_MEMORY(temp_board, mnum) = memboard;
                            MEMORY_NEXT(memboard) = nullptr;
                        }
                    } else {
                        free(memboard);
                    }
                }
            }
        } else if (*buf == '#') {
            if (parse_message(fl, temp_board)) {
                msg_num++;
            }
        }
    }/* End of While */

    /* now we've completely parsed our file */
    fclose(fl);
    if (msg_num != BOARD_MNUM(temp_board)) {
        basic_mud_log("Board [vnum: %d] message count (%d) not equal to actual message count (%d). Correcting.",
            BOARD_VNUM(temp_board), BOARD_MNUM(temp_board), msg_num);
        BOARD_MNUM(temp_board) = msg_num;
    }
    /* if the error flag is set, we need to save the board again */
    save_board(temp_board);
    return temp_board;
}

int parse_message(FILE *fl, struct board_info *temp_board) {
    struct board_msg *tmsg, *t2msg;
    char subject[81];
    char buf[MAX_MESSAGE_LENGTH + 1], poster[128];
    /* arbitrairy max message length */

    CREATE(tmsg, struct board_msg, 1);

    /* what about our error checking? */
    if (BOARD_VERSION(temp_board) != CURRENT_BOARD_VER) {
        if (fscanf(fl, "%ld\n", &(MESG_POSTER(tmsg))) != 1 ||
            fscanf(fl, "%ld\n", &(MESG_TIMESTAMP(tmsg))) != 1) {
            basic_mud_log("Parse error in message for board [vnum: %d].  Skipping.", BOARD_VNUM(temp_board));
            free(tmsg);
            return 0;
        }
    } else {
        if (fscanf(fl, "%s\n", poster) != 1 ||
            fscanf(fl, "%ld\n", &(MESG_TIMESTAMP(tmsg))) != 1) {
            basic_mud_log("Parse error in message for board [vnum: %d].  Skipping.", BOARD_VNUM(temp_board));
            free(tmsg);
            return 0;
        }
        MESG_POSTER_NAME(tmsg) = strdup(poster);
    }
    get_line(fl, subject);

    MESG_SUBJECT(tmsg) = strdup(subject);
    MESG_DATA(tmsg) = fread_string(fl, buf);
    MESG_NEXT(tmsg) = nullptr;

    /* always add to the END of the list. */
    MESG_NEXT(tmsg) = MESG_PREV(tmsg) = nullptr;
    if (BOARD_MESSAGES(temp_board)) {
        t2msg = BOARD_MESSAGES(temp_board);
        while (MESG_NEXT(t2msg)) {
            t2msg = MESG_NEXT(t2msg);
        }
        MESG_NEXT(t2msg) = tmsg;
        MESG_PREV(tmsg) = t2msg;
    } else {
        MESG_PREV(tmsg) = nullptr;
        BOARD_MESSAGES(temp_board) = tmsg;
    }
    return 1;
}

void look_at_boards() {
    int counter, messages = 0;
    struct board_info *tboard = bboards;
    struct board_msg *msg;

    for (counter = 0; tboard; counter++) {
        msg = BOARD_MESSAGES(tboard);
        while (msg) {
            messages++;
            msg = MESG_NEXT(msg);
        }
        tboard = BOARD_NEXT(tboard);
    }
    basic_mud_log("There are %d boards located; %d messages", counter, messages);
}

void clear_boards() {
    struct board_info *tmp, *tmp2;
    for (tmp = bboards; tmp; tmp = tmp2) {
        tmp2 = tmp->next;
        clear_one_board(tmp);
    }
}


void clear_one_board(struct board_info *tmp) {
    struct board_msg *m1, *m2;
    struct board_memory *mem1, *mem2;
    int i;

    /* before we clear this board, we need to disconnect anyone writing/etc to it */
    /* xapxapxap take care of this later */

    /* clear the messages */
    for (m1 = BOARD_MESSAGES(tmp); m1; m1 = m2) {
        m2 = m1->next;
        free(m1->subject);
        free(m1->data);
        free(m1);
    }
    /* clear the memory */
    for (i = 0; i < 301; i++) {
        for (mem1 = BOARD_MEMORY(tmp, i); mem1; mem1 = mem2) {
            mem2 = mem1->next;
            free(mem1);
        }
    }
    free(tmp);
    tmp = nullptr;
}

void show_board(obj_vnum board_vnum, struct char_data *ch) {
    struct board_info *thisboard;
    struct board_msg *message;
    char *tmstr;
    int msgcount = 0, yesno = 0, bnum = 0;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char name[127];

    *buf = '\0';
    *buf2 = '\0';
    *name = '\0';

    /* board locate */
    if (IS_NPC(ch)) {
        send_to_char(ch, "Gosh.. now .. if only mobs could read.. you'd be doing good.\r\n");
        return;
    }
    thisboard = locate_board(board_vnum);
    if (!thisboard) {
        basic_mud_log("Creating new board - board #%d", board_vnum);
        thisboard = create_new_board(board_vnum);
        thisboard->next = bboards;
        bboards = thisboard;
    }
    if (GET_ADMLEVEL(ch) < READ_LVL(thisboard)) {
        send_to_char(ch, "You try but fail to understand the holy words.\r\n");
        return;
    }

    /* send the standard board boilerplate */
    struct obj_data *obj;
    int num = board_vnum;
    if ((board_vnum = real_object(num)) == NOTHING) {
        basic_mud_log("SYSERR: DEFUNCT BOARD VNUM.\r\n");
        send_to_char(ch, "@W                  This is a bulletin board.\r\n");
        send_to_char(ch, "@rO@b============================================================================@rO@n\n");
        send_to_char(ch, "     @D[@GX@D] means you have read the message, @D[@RX@D] means you have not.\r\n"
                         "     @WUsage@D:@CREAD@D/@cREMOVE @D<@Wmessg #@D>@W, @CRESPOND @D<@Wmessg #@D>@W, @CWRITE @D<@Wheader@D>@W.@n\r\n"
                         "     @CVieworder@W, this changes the order in which posts are listed to you.@n\r\n");
    } else {
        obj = read_object(board_vnum, REAL);
        bnum = GET_OBJ_VNUM(obj);
        char clan[120];
        if (OBJ_FLAGGED(obj, ITEM_CBOARD)) {
            if (GET_CLAN(ch) != nullptr) {
                sprintf(clan, "%s", GET_CLAN(ch));
            }
            if (!strstr(obj->look_description, clan)) {
                send_to_char(ch, "You are incapable of reading this board!\r\n");
                return;
            }
        }
        send_to_char(ch, "@W                  This is the %20s\r\n", obj->short_description);
        send_to_char(ch, "@rO@b============================================================================@rO@n\n"
                         "     @D[@GX@D] means you have read the message, @D[@RX@D] means you have not.\r\n"
                         "     @WUsage@D:@CREAD@D/@cREMOVE @D<@Wmessg #@D>@W, @CRESPOND @D<@Wmessg #@D>@W, @CWRITE @D<@Wheader@D>@W.@n\r\n"
                         "     @CVieworder@W, this changes the order in which posts are listed to you.@n\r\n"
                         "     @D----------------------------------------------------------------\n");
        extract_obj(obj);
    }

    if (!BOARD_MNUM(thisboard) || !BOARD_MESSAGES(thisboard)) {
        sprintf(buf, "                  @WThe board is empty.@n\r\n");
        send_to_char(ch, buf);
        return;
    } else {
        send_to_char(ch, "                  @WThere %s %d %s on the board.@n\r\n",
                     (BOARD_MNUM(thisboard) == 1) ? "is" : "are", BOARD_MNUM(thisboard),
                     (BOARD_MNUM(thisboard) == 1) ? "message" : "messages");

    }
    message = BOARD_MESSAGES(thisboard);
    if (PRF_FLAGGED(ch, PRF_VIEWORDER)) {
        while (MESG_NEXT(message)) {
            message = MESG_NEXT(message);
        }
    }

    while (message) {
        tmstr = (char *) asctime(localtime(&MESG_TIMESTAMP(message)));
        *(tmstr + strlen(tmstr) - 1) = '\0';
        yesno = mesglookup(message, ch, thisboard);
        if (BOARD_VERSION(thisboard) != CURRENT_BOARD_VER)
            snprintf(name, sizeof(name), "%s", get_name_by_id(MESG_POSTER(message)));
        else
            snprintf(name, sizeof(name), "%s", MESG_POSTER_NAME(message));
        if (msgcount < 1) {
            sprintf(buf, "@D[%s] (@C%2d@D) : @W%6.10s @D(@G%-10s@D) ::@w %-45s\r\n", yesno ? "@GX@D" : "@RX@D",
                    ++msgcount, tmstr, CAP(name), MESG_SUBJECT(message) ? MESG_SUBJECT(message) : "No Subject");
        } else {
            sprintf(buf + strlen(buf), "@D[%s] (@C%2d@D) : @W%6.10s @D(@G%-10s@D) ::@w %-45s\r\n",
                    yesno ? "@GX@D" : "@RX@D", ++msgcount, tmstr, CAP(name),
                    MESG_SUBJECT(message) ? MESG_SUBJECT(message) : "No Subject");
        }
        if (PRF_FLAGGED(ch, PRF_VIEWORDER)) {
            message = MESG_PREV(message);
        } else {
            message = MESG_NEXT(message);
        }
    }
    sprintf(buf + strlen(buf),
            "@rO@b============================================================================@rO@n\n");
    strcpy(buf2, buf);
    write_to_output(ch->desc, buf2);
    if (bnum == 3092) {
        GET_BOARD(ch, 0) = time(nullptr);
        /* Mort board */
    }
    if (bnum == 3099) {
        GET_BOARD(ch, 3) = time(nullptr);
        /* Duo's board */
    }
    if (bnum == 3098) {
        GET_BOARD(ch, 1) = time(nullptr);
        /* Imm board */
    }
    if (bnum == 3090) {
        GET_BOARD(ch, 4) = time(nullptr);
        /* Builder board */
    }
    ch->save();
    return;

}

void board_display_msg(obj_vnum board_vnum, struct char_data *ch, int arg) {
    struct board_info *thisboard = bboards;
    struct board_msg *message;
    char *tmstr;
    int msgcount, mem, sflag;
    char name[127];
    struct board_memory *mboard_type, *list;
    char buf[MAX_STRING_LENGTH + 1];

    if (IS_NPC(ch)) {
        send_to_char(ch, "Silly mob - reading is for pcs!\r\n");
        return;
    }
    /* guess we'll have to locate the board now in the list */
    thisboard = locate_board(board_vnum);
    if (!thisboard) {
        basic_mud_log("Creating new board - board #%d", board_vnum);
        thisboard = create_new_board(board_vnum);
    }

    if (GET_ADMLEVEL(ch) < READ_LVL(thisboard)) {
        send_to_char(ch, "You try but fail to understand the holy words.\r\n");
        return;

    }
    if (!BOARD_MESSAGES(thisboard)) {
        send_to_char(ch, "The board is empty!\r\n");
        return;
    }

    struct obj_data *obj;
    int num = board_vnum;
    int bnum = 0;
    if ((board_vnum = real_object(num)) == NOTHING) {
        send_to_imm("Error with %d board, object doesn't exist.", board_vnum);
    } else {
        obj = read_object(board_vnum, REAL);
        bnum = GET_OBJ_VNUM(obj);
        char clan[200];
        if (OBJ_FLAGGED(obj, ITEM_CBOARD)) {
            if (GET_CLAN(ch) != nullptr) {
                sprintf(clan, "%s", GET_CLAN(ch));
            }
            if (!strstr(obj->look_description, clan)) {
                send_to_char(ch, "You are incapable of reading this board!\r\n");
                return;
            }
        }
        extract_obj(obj);
    }

    /* now we locate the message.*/
    message = BOARD_MESSAGES(thisboard);
    if (arg < 1) {
        send_to_char(ch, "You must specify the (positive) number of the message to be read!\r\n");
        return;
    }

    if (PRF_FLAGGED(ch, PRF_VIEWORDER)) {
        while (MESG_NEXT(message)) {
            message = MESG_NEXT(message);
        }
    }

    for (msgcount = arg; message && msgcount != 1; msgcount--) {
        if (PRF_FLAGGED(ch, PRF_VIEWORDER)) {
            message = MESG_PREV(message);
        } else {
            message = MESG_NEXT(message);
        }
    }

    if (!message) {
        send_to_char(ch, "That message exists only in your imagination.\r\n");
        return;
    }          /* Have message, let's add the fact that this player read the mesg */

    if (BOARD_VERSION(thisboard) != CURRENT_BOARD_VER)
        mem = ((MESG_TIMESTAMP(message) % 301 + MESG_POSTER(message) % 301) % 301);
    else
        mem = ((MESG_TIMESTAMP(message) % 301 + get_id_by_name(MESG_POSTER_NAME(message)) % 301) % 301);
    /*make the new node */
    CREATE(mboard_type, struct board_memory, 1);
    if (BOARD_VERSION(thisboard) != CURRENT_BOARD_VER)
        MEMORY_READER(mboard_type) = GET_IDNUM(ch);
    else
        MEMORY_READER_NAME(mboard_type) = strdup(GET_NAME(ch));
    MEMORY_TIMESTAMP(mboard_type) = MESG_TIMESTAMP(message);
    MEMORY_NEXT(mboard_type) = nullptr;
    /* Let's make sure that we don't already have this memory recorded */

    list = BOARD_MEMORY(thisboard, mem);
    sflag = 1;
    while (list && sflag) {
        if (BOARD_VERSION(thisboard) != CURRENT_BOARD_VER) {
            if (MEMORY_READER(list) == MEMORY_READER(mboard_type) &&
                MEMORY_TIMESTAMP(list) == MEMORY_TIMESTAMP(mboard_type)) {
                /* nope, slot, reader, and timestamp equal, so already saved */
                sflag = 0;
            }
        } else {
            if (!strcmp(MEMORY_READER_NAME(list), MEMORY_READER_NAME(mboard_type)) &&
                MEMORY_TIMESTAMP(list) == MEMORY_TIMESTAMP(mboard_type)) {
                /* nope, slot, reader, and timestamp equal, so already saved */
                sflag = 0;
            }
        }
        list = MEMORY_NEXT(list);
    }

    if (sflag) {
        list = BOARD_MEMORY(thisboard, mem);
        BOARD_MEMORY(thisboard, mem) = mboard_type;
        MEMORY_NEXT(mboard_type) = list;
    } else {
        if (mboard_type) {
        }
    }

    /* before we print out the message, we may as well restore a human
       readable timestamp. */
    tmstr = (char *) asctime(localtime(&MESG_TIMESTAMP(message)));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    if (BOARD_VERSION(thisboard) != CURRENT_BOARD_VER)
        snprintf(name, sizeof(name), "%s", get_name_by_id(MESG_POSTER(message)));
    else
        snprintf(name, sizeof(name), "%s", MESG_POSTER_NAME(message));
    sprintf(buf,
            "@r_____________________________________________________________________________@n\r\n\r\n@gMessage @W[@Y%2d@W] @D: @c%6.10s @D(@C%s@D)\r\n@GTopic        @D: @w%s\r\n@r-------------------------------------------------------------------------@n\r\n\r\n%s\n@r_____________________________________________________________________________@n\r\n",
            arg,
            tmstr,
            CAP(name),
            MESG_SUBJECT(message) ? MESG_SUBJECT(message) : "No Subject",
            MESG_DATA(message) ? MESG_DATA(message) : "Looks like this message is empty.");
    write_to_output(ch->desc, buf);
    if (bnum == 3092) {
        GET_BOARD(ch, 0) = time(nullptr);
        /* Mort board */
    }
    if (bnum == 3099) {
        GET_BOARD(ch, 3) = time(nullptr);
        /* Duo's board */
    }
    if (bnum == 3098) {
        GET_BOARD(ch, 1) = time(nullptr);
        /* Imm board */
    }
    if (bnum == 3090) {
        GET_BOARD(ch, 4) = time(nullptr);
        /* Builder board */
    }
    /* really it's not so important to save after each view even if something WAS updated */
    /* might be better to just save them with zone resets? */
    /* for now if sflag is triggered, we know that we added new memory */
    if (sflag) {
        save_board(thisboard);
    }
    return;

}


int mesglookup(struct board_msg *message, struct char_data *ch, struct board_info *board) {
    int mem = 0;
    struct board_memory *mboard_type;
    char *tempname = nullptr;

    if (BOARD_VERSION(board) != CURRENT_BOARD_VER)
        mem = ((MESG_TIMESTAMP(message) % 301 + MESG_POSTER(message) % 301) % 301);
    else
        mem = ((MESG_TIMESTAMP(message) % 301 + get_id_by_name(MESG_POSTER_NAME(message)) % 301) % 301);
    /* now, we check the mem slot. If its null, we return no, er.. 0..
       if its full, we double check against the timestamp and reader -mislabled as poster, but who cares...
       if they're not true, we go to the linked next slot, and repeat */

    mboard_type = BOARD_MEMORY(board, mem);
    while (mboard_type && BOARD_VERSION(board) != CURRENT_BOARD_VER) {
        if (MEMORY_READER(mboard_type) == GET_IDNUM(ch) &&
            MEMORY_TIMESTAMP(mboard_type) == MESG_TIMESTAMP(message)) {
            return 1;
        } else {
            mboard_type = MEMORY_NEXT(mboard_type);
        }
    }

    tempname = strdup(GET_NAME(ch));
    while (mboard_type && BOARD_VERSION(board) == CURRENT_BOARD_VER) {
        if (!strcmp(MEMORY_READER_NAME(mboard_type), tempname) &&
            MEMORY_TIMESTAMP(mboard_type) == MESG_TIMESTAMP(message)) {
            return 1;
        } else {
            mboard_type = MEMORY_NEXT(mboard_type);
        }
    }

    free(tempname);
    return 0;
}

void write_board_message(obj_vnum board_vnum, struct char_data *ch, char *arg) {
    struct board_info *thisboard = bboards;
    struct board_msg *message;

    if (IS_NPC(ch)) {
        send_to_char(ch, "Orwellian police thwart your attempt at free speech.\r\n");
        return;
    }
    thisboard = locate_board(board_vnum);

    if (!thisboard) {
        send_to_char(ch, "Error: Your board could not be found. Please report.\n");
        basic_mud_log("Error in write_board_msg - board #%d", board_vnum);
        return;
    }

    if (GET_ADMLEVEL(ch) < WRITE_LVL(thisboard)) {
        send_to_char(ch, "You are not holy enough to write on this board.\r\n");

        return;
    }
    if (!*arg || !arg) {
        sprintf(arg, "No Subject");
    }
    if (strlen(arg) > 46) {
        send_to_char(ch, "Your subject can only be 45 characters long(including colorcode).\r\n");
        return;
    }
    act("@C$n@w starts writing on the board.@n", true, ch, nullptr, nullptr, TO_ROOM);
    skip_spaces(&arg);
    delete_doubledollar(arg);
    arg[81] = '\0';


    CREATE(message, struct board_msg, 1);
    MESG_POSTER_NAME(message) = strdup(GET_NAME(ch));
    MESG_TIMESTAMP(message) = time(nullptr);
    MESG_SUBJECT(message) = strdup(arg);
    MESG_NEXT(message) = nullptr;
    MESG_PREV(message) = nullptr;
    MESG_DATA(message) = nullptr;
    BOARD_MNUM(thisboard) = MAX(BOARD_MNUM(thisboard) + 1, 1);

    MESG_NEXT(message) = BOARD_MESSAGES(thisboard);

    if (BOARD_MESSAGES(thisboard)) {
        MESG_PREV(BOARD_MESSAGES(thisboard)) = message;
    }

    BOARD_MESSAGES(thisboard) = message;
    send_to_char(ch, "Write your message.  (/s saves /h for help)\r\n");

    ch->playerFlags.set(PLR_WRITING);
    string_write(ch->desc, &(MESG_DATA(message)), MAX_MESSAGE_LENGTH, board_vnum + BOARD_MAGIC, nullptr);
    if (board_vnum == 3092) {
        BOARDNEWMORT = time(nullptr);
    }
    if (board_vnum == 3098) {
        BOARDNEWIMM = time(nullptr);
    }
    if (board_vnum == 3099) {
        BOARDNEWDUO = time(nullptr);
    }
    if (board_vnum == 3090) {
        BOARDNEWBUI = time(nullptr);
    }
    save_mud_time(&time_info);
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
        if (!IS_PLAYING(d))
            continue;
        if (PLR_FLAGGED(d->character, PLR_WRITING))
            continue;
        if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWIMM > GET_BOARD(d->character, 1) && board_vnum == 3098)
            send_to_char(d->character, "\r\n@GThere is a new Immortal Board Post.@n\r\n");
        if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWBUI > GET_BOARD(d->character, 4) && board_vnum == 3090)
            send_to_char(d->character, "\r\n@GThere is a new Builder Board Post.@n\r\n");
        if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWDUO > GET_BOARD(d->character, 3) && board_vnum == 3099)
            send_to_char(d->character, "\r\n@GThere is a new Punishment Board Post.@n\r\n");
        if (BOARDNEWMORT > GET_BOARD(d->character, 0) && board_vnum == 3092)
            send_to_char(d->character, "\r\n@GThere is a new Mortal Board Post.@n\r\n");
    }


    return;
}

void board_respond(long board_vnum, struct char_data *ch, int mnum) {
    struct board_info *thisboard = bboards;
    struct board_msg *message, *other;
    char number[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int gcount = 0;

    thisboard = locate_board(board_vnum);

    if (!thisboard) {
        send_to_char(ch, "Error: Your board could not be found. Please report.\n");
        basic_mud_log("Error in board_respond - board #%ld", board_vnum);
        return;
    }
    if (GET_ADMLEVEL(ch) < WRITE_LVL(thisboard)) {
        send_to_char(ch, "You are not holy enough to write on this board.\r\n");
        return;
    }

    if (GET_ADMLEVEL(ch) < READ_LVL(thisboard)) {
        send_to_char(ch, "You are not holy enough to respond to this board.\r\n");
        return;
    }
    if (PRF_FLAGGED(ch, PRF_VIEWORDER)) {
        mnum = (BOARD_MNUM(thisboard) - mnum) + 1;
    }
    if (mnum < 0 || mnum > BOARD_MNUM(thisboard)) {
        send_to_char(ch, "You can only respond to an actual message.\r\n");

        return;
    }

    other = BOARD_MESSAGES(thisboard);

    /*locate message to be repsponded to */
    for (gcount = 0; other && gcount != (mnum - 1); gcount++)
        other = MESG_NEXT(other);

    CREATE(message, struct board_msg, 1);
    MESG_POSTER_NAME(message) = strdup(GET_NAME(ch));
    MESG_TIMESTAMP(message) = time(nullptr);
    sprintf(buf, "Re: %s", MESG_SUBJECT(other));
    MESG_SUBJECT(message) = strdup(buf);
    MESG_NEXT(message) = MESG_PREV(message) = nullptr;
    MESG_DATA(message) = nullptr;
    BOARD_MNUM(thisboard) = BOARD_MNUM(thisboard) + 1;
    MESG_NEXT(message) = BOARD_MESSAGES(thisboard);
    if (BOARD_MESSAGES(thisboard)) {
        MESG_PREV(BOARD_MESSAGES(thisboard)) = message;
    }
    BOARD_MESSAGES(thisboard) = message;

    send_to_char(ch, "Write your message.  (/s saves /h for help)\r\n\r\n");
    act("@C$n@w starts writing on the board.@n", true, ch, nullptr, nullptr, TO_ROOM);

    if (!IS_NPC(ch)) {
        ch->playerFlags.set(PLR_WRITING);
    }

    /* don't need number anymore, so we'll reuse it. */
    sprintf(number, "\t@D------- @cQuoted message @D-------@w\r\n%s\t@D   ------- @cEnd Quote @D-------@w\r\n",
            MESG_DATA(other));
    MESG_DATA(message) = strdup(number);
    ch->desc->backstr = strdup(number);
    write_to_output(ch->desc, number);

    string_write(ch->desc, &(MESG_DATA(message)), MAX_MESSAGE_LENGTH, board_vnum + BOARD_MAGIC, nullptr);
    if (board_vnum == 3092) {
        BOARDNEWMORT = time(nullptr);
    }
    if (board_vnum == 3098) {
        BOARDNEWIMM = time(nullptr);
    }
    if (board_vnum == 3099) {
        BOARDNEWDUO = time(nullptr);
    }
    if (board_vnum == 3090) {
        BOARDNEWBUI = time(nullptr);
    }
    save_mud_time(&time_info);
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
        if (!IS_PLAYING(d))
            continue;
        if (PLR_FLAGGED(d->character, PLR_WRITING))
            continue;
        if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWIMM > GET_BOARD(d->character, 1) && board_vnum == 3098)
            send_to_char(d->character, "\r\n@GThere is a new Immortal Board Post.@n\r\n");
        if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWBUI > GET_BOARD(d->character, 4) && board_vnum == 3090)
            send_to_char(d->character, "\r\n@GThere is a new Builder Board Post.@n\r\n");
        if (GET_ADMLEVEL(d->character) >= 1 && BOARDNEWDUO > GET_BOARD(d->character, 3) && board_vnum == 3099)
            send_to_char(d->character, "\r\n@GThere is a new Punishment Board Post.@n\r\n");
        if (BOARDNEWMORT > GET_BOARD(d->character, 0) && board_vnum == 3092)
            send_to_char(d->character, "\r\n@GThere is a new Mortal Board Post.@n\r\n");
    }

    return;
}

struct board_info *locate_board(obj_vnum board_vnum) {
    struct board_info *thisboard = bboards;

    while (thisboard) {
        if (BOARD_VNUM(thisboard) == board_vnum) {
            return thisboard;
        }
        thisboard = BOARD_NEXT(thisboard);

    }
    return nullptr;
}


void remove_board_msg(obj_vnum board_vnum, struct char_data *ch, int arg) {
    struct board_info *thisboard;
    struct board_msg *cur, *temp;
    struct descriptor_data *d;
    struct obj_data *obj;
    int msgcount;
    char buf[MAX_STRING_LENGTH + 1];

    if (IS_NPC(ch)) {
        send_to_char(ch, "Nuts.. looks like you forgot your eraser back in mobland...\r\n");
        return;
    }
    thisboard = locate_board(board_vnum);

    if (!thisboard) {
        send_to_char(ch, "Error: Your board could not be found. Please report.\n");
        basic_mud_log("Error in Board_remove_msg - board #%d", board_vnum);
        return;
    }

    cur = BOARD_MESSAGES(thisboard);

    if (arg < 1) {
        send_to_char(ch, "You must specify the (positive) number of the message to be read!\r\n");
        return;
    }

    if (PRF_FLAGGED(ch, PRF_VIEWORDER)) {
        arg = BOARD_MNUM(thisboard) - arg + 1;
    }
    for (msgcount = arg; cur && msgcount != 1; msgcount--) {
        cur = MESG_NEXT(cur);

    }
    if (!cur) {
        send_to_char(ch, "That message exists only in your imagination.\r\n");
        return;
    }
    /* perform check for mesg in creation */
    int num = board_vnum;
    if ((board_vnum = real_object(num)) == NOTHING) {
        basic_mud_log("Board doesn't exists! Weird.");
        return;
    } else {
        char clan[120];
        obj = read_object(board_vnum, REAL);
        if (OBJ_FLAGGED(obj, ITEM_CBOARD)) {
            if (GET_CLAN(ch) != nullptr) {
                sprintf(clan, "%s", GET_CLAN(ch));
            }
            if (clanIsModerator(clan, ch) && strstr(obj->look_description, clan)) {
                send_to_char(ch, "Exercising your clan leader powers....\r\n");
            } else if (GET_ADMLEVEL(ch) < REMOVE_LVL(thisboard) && strcmp(GET_NAME(ch), MESG_POSTER_NAME(cur))) {
                send_to_char(ch, "You can't remove other people's messages.\r\n");
                extract_obj(obj);
                return;
            }
        } else if (!OBJ_FLAGGED(obj, ITEM_CBOARD)) {
            if (GET_ADMLEVEL(ch) < REMOVE_LVL(thisboard) && strcmp(GET_NAME(ch), MESG_POSTER_NAME(cur))) {
                send_to_char(ch, "You can't remove other people's messages.\r\n");
                extract_obj(obj);
                return;
            }
        }
        extract_obj(obj);
    }

    for (d = descriptor_list; d; d = d->next) {
        if (!d->connected && d->str == &(MESG_DATA(cur))) {
            send_to_char(ch, "At least wait until the author is finished before removing it!\r\n");
            return;
        }
    }
    /* everything else is peachy, kill the message */

    REMOVE_FROM_DOUBLE_LIST(cur, BOARD_MESSAGES(thisboard), next, prev);

    free(cur);
    cur = nullptr;
    BOARD_MNUM (thisboard) = BOARD_MNUM (thisboard) - 1;
    send_to_char(ch, "Message removed.\r\n");
    sprintf(buf, "$n just removed message %d.", arg);
    act(buf, false, ch, nullptr, nullptr, TO_ROOM);
    save_board(thisboard);
    return;
}


