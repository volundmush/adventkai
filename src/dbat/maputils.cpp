/***************************************************** 
* maputils.c --- implementation file for ascii maps  * 
*				     		     * 
* Kyle Goodwin, (c) 1998 All Rights Reserved         * 
* vbmasta@earthlink.net - Head Implemenor FirocMUD   * 
*				     		     * 
* Paolo Libardi - pinkpallin@libero.it		     * 
*****************************************************/
#include "dbat/maputils.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/db.h"
#include "dbat/vehicles.h"


int mapnums[MAP_ROWS + 1][MAP_COLS + 1];

void ping_ship(int vnum, int vnum2) {

    struct char_data *tch, *next_ch;
    struct obj_data *controls = nullptr, *obj = nullptr;
    int found = false;

    if (vnum2 == -1) {
        return;
    }

    for (tch = character_list; tch; tch = next_ch) {
        next_ch = tch->next;
        if (found == false) {
            if (!(obj = find_control(tch))) {
                continue;
            } else {
                if (GET_OBJ_VAL(obj, 0) == vnum && vnum != vnum2) {
                    controls = obj;
                    found = true;
                }
            }
        }
    }

    if (found == true) {
        send_to_room(IN_ROOM(controls), "@D[@RALERT@D: @YAn unknown radar signal has been detected!@D]@n");
    }
}

int checkship(int rnum, int vnum) {
    struct obj_data *i = nullptr;
    int there = false;

    for (i = world[rnum].contents; i; i = i->next_content) {
        if (!ROOM_FLAGGED(rnum, ROOM_NEBULA)) {
            if (GET_OBJ_TYPE(i) == ITEM_VEHICLE && there != true) {
                there = true;
                ping_ship(GET_OBJ_VNUM(i), vnum);
            }
        }
    }

    i = nullptr;

    return there;
}

char *getmapchar(int rnum, struct char_data *ch, int start, int vnum) {
    static char mapchar[50];
    int there = false, enemy = false;

    if (rnum == start) {
        there = true;
    }
    if (checkship(rnum, vnum)) {
        enemy = true;
    }

    if (rnum == real_room(GET_RADAR1(ch)) || rnum == real_room(GET_RADAR2(ch)) || rnum == real_room(GET_RADAR3(ch))) {
        if (there) {
            sprintf(mapchar, "@WB@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@WB@r#");
        } else {
            sprintf(mapchar, "@WBB");
        }
    } /* End Buoy */
    else if (ROOM_FLAGGED(rnum, ROOM_EORBIT)) {
        if (there) {
            sprintf(mapchar, "@GE@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@GE@r#");
        } else {
            sprintf(mapchar, "@GEE");
        }
    } /* End Earth Character */
    else if (ROOM_FLAGGED(rnum, ROOM_CORBIT)) {
        if (there) {
            sprintf(mapchar, "@MC@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@MC@r#");
        } else {
            sprintf(mapchar, "@MCC");
        }
    } /* End Cerria Character */
    else if (ROOM_FLAGGED(rnum, ROOM_FORBIT)) {
        if (there) {
            sprintf(mapchar, "@CF@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@CF@r#");
        } else {
            sprintf(mapchar, "@CFF");
        }
    } /* End Frigid Character */
    else if (ROOM_FLAGGED(rnum, ROOM_KORBIT)) {
        if (there) {
            sprintf(mapchar, "@mK@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@mK@r#");
        } else {
            sprintf(mapchar, "@mKK");
        }
    } /* End Konack Character */
    else if (ROOM_FLAGGED(rnum, ROOM_NORBIT)) {
        if (there) {
            sprintf(mapchar, "@gN@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@gN@r#");
        } else {
            sprintf(mapchar, "@gNN");
        }
    } /* End Namek Character */
    else if (GET_ROOM_VNUM(rnum) == 50772) {
        if (there) {
            sprintf(mapchar, "@cZ@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@cZ@r#");
        } else {
            sprintf(mapchar, "@cZZ");
        }
    } /* End Zenith Character */
    else if (ROOM_FLAGGED(rnum, ROOM_VORBIT)) {
        if (there) {
            sprintf(mapchar, "@YV@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@YV@r#");
        } else {
            sprintf(mapchar, "@YVV");
        }
    } /* End Vegeta Character */
    else if (ROOM_FLAGGED(rnum, ROOM_AORBIT)) {
        if (there) {
            sprintf(mapchar, "@BA@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@BA@r#");
        } else {
            sprintf(mapchar, "@BAA");
        }
    } /* End Aether Character */
    else if (ROOM_FLAGGED(rnum, ROOM_YORBIT)) {
        if (there) {
            sprintf(mapchar, "@MY@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@MY@r#");
        } else {
            sprintf(mapchar, "@MYY");
        }
    } /* End Yardrat Character */
    else if (ROOM_FLAGGED(rnum, ROOM_KANORB)) {
        if (there) {
            sprintf(mapchar, "@CK@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@CK@r#");
        } else {
            sprintf(mapchar, "@CKK");
        }
    } /* End Kanassa Character */
    else if (ROOM_FLAGGED(rnum, ROOM_ARLORB)) {
        if (there) {
            sprintf(mapchar, "@mA@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@mA@r#");
        } else {
            sprintf(mapchar, "@mAA");
        }
    } /* End Arlia Character */
    else if (ROOM_FLAGGED(rnum, ROOM_NEBULA)) {
        if (there) {
            sprintf(mapchar, "@m&@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@m&@r#");
        } else {
            sprintf(mapchar, "@m&&");
        }
    } /* End Nebula Character */
    else if (GET_ROOM_VNUM(rnum) == 38028) {
        if (there) {
            sprintf(mapchar, "@yQ@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@yQ@r#");
        } else {
            sprintf(mapchar, "@yQQ");
        }
    } /* End Namek Character */
    else if (ROOM_FLAGGED(rnum, ROOM_ASTERO)) {
        if (there) {
            sprintf(mapchar, "@y:@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@y:@r#");
        } else {
            sprintf(mapchar, "@y::");
        }
    } /* End Asteroid Character */
    else if (ROOM_FLAGGED(rnum, ROOM_WORMHO)) {
        if (there) {
            sprintf(mapchar, "@b@1*@RX@n");
        } else if (enemy == true) {
            sprintf(mapchar, "@b@1*@r#@n");
        } else {
            sprintf(mapchar, "@b@1**@n");
        }
    } /* End Wormhole Character */
    else if (ROOM_FLAGGED(rnum, ROOM_STATION)) {
        if (there) {
            sprintf(mapchar, "@DS@RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@DS@r#");
        } else {
            sprintf(mapchar, "@DSS");
        }
    } /* End Station Character */
    else if (ROOM_FLAGGED(rnum, ROOM_STAR)) {
        if (there) {
            sprintf(mapchar, "@6 @RX@n");
        } else if (enemy == true) {
            sprintf(mapchar, "@6 @r#@n");
        } else {
            sprintf(mapchar, "@6  @n");
        }
    } /* End Star Character */
    else {
        if (there) {
            sprintf(mapchar, "@w @RX");
        } else if (enemy == true) {
            sprintf(mapchar, "@w @r#");
        } else {
            int color = rand_number(1, 30);
            if (rand_number(1, 40) == 2) {
                sprintf(mapchar, "%s. ", color > 15 ? "@w" : (color >= 7 ? "@Y" : (color > 3 ? "@R" : "@B")));
            } else if (rand_number(1, 40) == 2) {
                sprintf(mapchar, "%s .", color > 15 ? "@w" : (color >= 7 ? "@Y" : (color > 3 ? "@R" : "@B")));
            } else {
                sprintf(mapchar, "@w  ");
            }
        }
    } /* End default Character */
    return mapchar;
}

MapStruct findcoord(int rnum) {
    int x, y;
    MapStruct coords;

    coords.x = 0;
    coords.y = 0;

    for (y = 0; y <= MAP_ROWS; y++) {
        for (x = 0; x <= MAP_COLS; x++) {
            if (mapnums[y][x] == rnum) {
                coords.y = y;
                coords.x = x;
                return coords;
            }
        }
    }

    basic_mud_log("SYSERR: findcoord for non-map rnum");
    return coords;
}

void printmap(int rnum, struct char_data *ch, int type, int vnum) {
    int x = 0, lasty = -1;
    int y = 0;
    int sightradius;
    int count = 0, initline = 0;
    char buf[MAX_STRING_LENGTH * 2];
    char buf2[512];
    MapStruct coord;

    int start = rnum;

    coord = findcoord(rnum);
    strcpy(buf, "\n");
    if (type == 0) {
        sightradius = 12;
    } else {
        sightradius = 4;
    }

    if (type == 0) {
        send_to_char(ch, "@b______________________________________________________________________@n\n");
    }

    for (y = coord.y - sightradius; y <= coord.y + sightradius; y++) {
        if (type == 0) {
            if (count == initline) {
                strcat(buf, "@b     [@CR. Key@b]     | ");
            } else if (count == initline + 1) {
                strcat(buf, "@GEE@D:@w Earth@b         | ");
            } else if (count == initline + 2) {
                strcat(buf, "@gNN@D:@w Namek@b         | ");
            } else if (count == initline + 3) {
                strcat(buf, "@YVV@D:@w Vegeta@b        | ");
            } else if (count == initline + 4) {
                strcat(buf, "@CFF@D:@w Frigid@b        | ");
            } else if (count == initline + 5) {
                strcat(buf, "@mKK@D:@w Konack@b        | ");
            } else if (count == initline + 6) {
                strcat(buf, "@BAA@D:@w Aether@b        | ");
            } else if (count == initline + 7) {
                strcat(buf, "@MYY@D:@w Yardrat@b       | ");
            } else if (count == initline + 8) {
                strcat(buf, "@CKK@D:@w Kanassa@b       | ");
            } else if (count == initline + 9) {
                strcat(buf, "@mAA@D:@w Arlia@b         | ");
            } else if (count == initline + 10) {
                strcat(buf, "@cZZ@D:@w Zenith@b        | ");
            } else if (count == initline + 11) {
                strcat(buf, "@MCC@D:@w Cerria@b        | ");
            } else if (count == initline + 12) {
                strcat(buf, "@WBB@D:@w Buoy@b          | ");
            } else if (count == initline + 13) {
                strcat(buf, "@m&&@D:@w Nebula@b        | ");
            } else if (count == initline + 14) {
                strcat(buf, "@yQQ@D:@w Asteroid@b      | ");
            } else if (count == initline + 15) {
                strcat(buf, "@y::@D:@w Asteroid Field@b| ");
            } else if (count == initline + 16) {
                strcat(buf, "@b@1**@n@D:@w Wormhole@b      | ");
            } else if (count == initline + 17) {
                strcat(buf, "@DSS@D:@w S. Station@b    | ");
            } else if (count == initline + 18) {
                strcat(buf, " @r#@D:@w Unknown Ship@b  | ");
            } else if (count == initline + 19) {
                strcat(buf, "@6  @n@D:@w Star@b          | ");
            } else {
                strcat(buf, "                  @b| ");
            }
            count++;
        } else {
            if (count == 0) {
                strcat(buf, "      @RCompass@n           ");
            } else if (count == 2) {
                sprintf(buf2, "@w       @w|%s@w|            ", (W_EXIT(rnum, 0) ? " @CN " : "   "));
                strcat(buf, buf2);
            } else if (count == 3) {
                sprintf(buf2, "@w @w|%s@w| |%s@w| |%s@w|      ", (W_EXIT(rnum, 6) ? " @CNW" : "   "),
                        (W_EXIT(rnum, 4) ? " @YU " : "   "), (W_EXIT(rnum, 7) ? "@CNE " : "   "));
                strcat(buf, buf2);
            } else if (count == 4) {
                sprintf(buf2, "@w @w|%s@w| |%s@w| |%s@w|      ", (W_EXIT(rnum, 3) ? "  @CW" : "   "),
                        (W_EXIT(rnum, 10) ? "@m I " : (W_EXIT(rnum, 11) ? "@mOUT" : "   ")),
                        (W_EXIT(rnum, 1) ? "@CE  " : "   "));
                strcat(buf, buf2);
            } else if (count == 5) {
                sprintf(buf2, "@w @w|%s@w| |%s@w| |%s@w|      ", (W_EXIT(rnum, 9) ? " @CSW" : "   "),
                        (W_EXIT(rnum, 5) ? " @YD " : "   "), (W_EXIT(rnum, 8) ? "@CSE " : "   "));
                strcat(buf, buf2);
            } else if (count == 6) {
                sprintf(buf2, "@w       @w|%s@w|            ", (W_EXIT(rnum, 2) ? " @CS " : "   "));
                strcat(buf, buf2);
            } else {
                strcat(buf, "                        ");
            }
            count++;
        }
        for (x = coord.x - sightradius; x <= coord.x + sightradius; x++) {
            if (x == coord.x && y == coord.y) {
                strcat(buf, getmapchar(mapnums[y][x], ch, start, vnum));
            } else if (x > MAP_COLS || x < 0) {
                if (lasty != true && y > -1 && y < 200) {
                    strcat(buf, "@D?");
                    lasty = true;
                }
            } else if (y > MAP_ROWS || y < 0) {
                if (y == -1 || y == 200) {
                    strcat(buf, "@D??");
                }
            } else
                strcat(buf, getmapchar(mapnums[y][x], ch, start, vnum));
        }
        strcat(buf, "\n");
        lasty = false;
    }

    send_to_char(ch, buf);
    *buf2 = '\0';
    *buf = '\0';
    if (type == 0) {
        send_to_char(ch, "\n@b______________________________________________________________________@n");
    }
}
