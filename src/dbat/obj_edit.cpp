/* ************************************************************************
*  File: obj_edit.c                                    Part of DBAT       *
*  Usage: Player level object editing                                     *
*                                                                         *
*  This is an original file created by me for Dragonball Advent Truth     *
*  to house all player level object editing functions -- Iovan 1/6/13     *
************************************************************************ */
#define __OBJ_EDIT_C__

#include "dbat/obj_edit.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"
#include "dbat/db.h"
#include "dbat/spells.h"
#include "dbat/screen.h"
#include "dbat/constants.h"
#include "dbat/obj_edit.h"
#include "dbat/dg_comm.h"
#include "dbat/act.other.h"

/* local functions  */
void disp_custom_menu(struct descriptor_data *d);

void pobj_edit_parse(struct descriptor_data *d, char *arg);

void disp_custom_menu(struct descriptor_data *d);

/* utility variables */

/* This is what equipment type it is */
const char *custom_types[17] = {
        "Error",
        "Armor Slot",
        "Gi Slot",
        "Wrist Slot",
        "Ear Slot",
        "Finger Slot",
        "Eye Slot",
        "Hands Slot",
        "Feet Slot",
        "Belt Slot",
        "Legs Slot",
        "Arms Slot",
        "Head Slot",
        "Neck Slot",
        "Back Slot",
        "Shoulder Slot",
        "Weapon"
};

/* This shows what weapon type it is */
const char *custom_weapon[7] = {
        "Not A Weapon",
        "Sword",
        "Dagger",
        "Spear",
        "Club",
        "Gun",
        "Brawling"
};

/* Below this is the custom equipment creation menu section of obj_edit.c */

/* This displays the custom equipment construction menu to the player. */
void disp_custom_menu(struct descriptor_data *d) {
    write_to_output(d, "@GCustom Equipment Construction Menu@n\n");
    write_to_output(d, "@D-----------------------------------------@n\n");
    write_to_output(d, "@C1@D) @WKeyword Name     @D: @w%s@n\n", d->obj_name);
    write_to_output(d, "@C2@D) @WShort Description@D: @w%s@n\n", d->obj_short);
    write_to_output(d, "@C3@D) @WLong Description @D: @w%s@n\n", d->obj_long);
    write_to_output(d, "@C4@D) @WEquipment Type   @D: @w%s@n\n", custom_types[d->obj_type]);
    write_to_output(d, "@C5@D) @WWeapon Type      @D: @w%s@n\n", custom_weapon[d->obj_weapon]);
    write_to_output(d, "@CQ@D) @WQuit Menu@n\n");
}

/* This displays the equipment restring menu, as if you couldn't tell. */
void disp_restring_menu(struct descriptor_data *d) {
    write_to_output(d, "@GEquipment Restring Menu@n\n");
    write_to_output(d, "@D-----------------------------------------@n\n");
    write_to_output(d, "@C1@D) @WKeyword Name     @D: @w%s@n\n", d->obj_name);
    write_to_output(d, "@C2@D) @WShort Description@D: @w%s@n\n", d->obj_short);
    write_to_output(d, "@C3@D) @WLong Description @D: @w%s@n\n", d->obj_long);
    write_to_output(d, "@CQ@D) @WQuit Menu@n\n");
}

/* This handles the player level object construction process. */
void pobj_edit_parse(struct descriptor_data *d, char *arg) {

    struct obj_data *obj = nullptr;
    char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];

    /* This section is dedicated to equipment restringing */
    if (d->obj_editflag == EDIT_RESTRING) {
        switch (d->obj_editval) {
            case EDIT_RESTRING_MAIN:
                switch (*arg) {
                    case '1': /* Name Editing */
                        write_to_output(d, "@wCurrent Equipment's Keyword Name @D<@Y%s@D>@n\r\n", d->obj_name);
                        write_to_output(d, "@wEnter Name @D(@RDo not use colorcode!@D)@w:@n \n");
                        d->obj_editval = EDIT_RESTRING_NAME;
                        break;

                    case '2': /* Short Desc Editing */
                        write_to_output(d, "@wCurrent Equipment's Short Desc @D<@Y%s@D>@n\r\n", d->obj_short);
                        write_to_output(d, "@wEnter Short Desc@w:@n \n");
                        d->obj_editval = EDIT_RESTRING_SDESC;
                        break;

                    case '3': /* Long Desc Editing */
                        write_to_output(d, "@wCurrent Equipment's Long Desc @D<@Y%s@D>@n\r\n", d->obj_long);
                        write_to_output(d, "@wEnter Long Desc@w:@n \n");
                        d->obj_editval = EDIT_RESTRING_LDESC;
                        break;

                    case 'Q': /* Quit Editing */
                    case 'q':
                        write_to_output(d, "Save current changes and be charged?\r\nYes or No\r\n");
                        d->obj_editval = EDIT_RESTRING_QUIT;
                        break;

                    default:
                        disp_restring_menu(d);
                        return;
                }
                break;

            case EDIT_RESTRING_NAME: /* Name menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Keeping what was previously entered.@n\r\n");
                    disp_restring_menu(d);
                    d->obj_editval = EDIT_RESTRING_MAIN;
                    return;
                } else if (strstr(arg, "@")) {
                    write_to_output(d, "@RNO COLORCODE IN THE KEYWORD NAME.@n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else if (strlen(arg) > 100) {
                    write_to_output(d, "@wToo long. Limit is 100 character.@n\r\n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else {
                    if (d->obj_name) {
                        free(d->obj_name);
                    }
                    *buf = '\0';
                    sprintf(buf, "%s", arg);
                    d->obj_name = strdup(buf);
                    disp_restring_menu(d);
                    d->obj_editval = EDIT_RESTRING_MAIN;
                }
                break;

            case EDIT_RESTRING_SDESC: /* Short Desc Menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Keeping what was previously entered.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_RESTRING_MAIN;
                    return;
                } else if (strlen(arg) > 150) {
                    write_to_output(d, "@wToo long. Limit is 200 character.@n\r\n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else {
                    if (d->obj_short) {
                        free(d->obj_short);
                    }
                    *buf = '\0';
                    snprintf(buf, sizeof(buf), "%s", arg);
                    d->obj_short = strdup(buf);
                    disp_restring_menu(d);
                    d->obj_editval = EDIT_RESTRING_MAIN;
                }
                break;

            case EDIT_RESTRING_LDESC: /* Long Desc Menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Keeping what was previously entered.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_RESTRING_MAIN;
                    return;
                } else if (strlen(arg) > 200) {
                    write_to_output(d, "@wToo long. Limit is 200 character.@n\r\n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else {
                    if (d->obj_long) {
                        free(d->obj_long);
                    }
                    *buf = '\0';
                    snprintf(buf, sizeof(buf), "%s", arg);
                    d->obj_long = strdup(buf);
                    disp_restring_menu(d);
                    d->obj_editval = EDIT_RESTRING_MAIN;
                }
                break;

            case EDIT_RESTRING_QUIT:
                if (!*arg) {
                    write_to_output(d, "Save current changes and be charged?\r\nYes or No\r\n");
                    return;
                } else if (!strcasecmp(arg, "yes") || !strcasecmp(arg, "Yes") || !strcasecmp(arg, "y") ||
                           !strcasecmp(arg, "Y")) {
                    obj = d->obj_point;

                    *buf = '\0';
                    sprintf(buf, "%s", d->obj_name);
                    obj->name = strdup(buf);

                    *buf2 = '\0';
                    sprintf(buf2, "%s", d->obj_short);
                    obj->short_description = strdup(buf2);

                    *buf3 = '\0';
                    sprintf(buf3, "%s", d->obj_long);
                    obj->room_description = strdup(buf3);

                    d->obj_editflag = EDIT_NONE;
                    d->obj_editval = EDIT_NONE;
                    d->character->mod(CharMoney::Carried,  -5000);
                    obj->extra_flags.set(ITEM_RESTRING);
                    write_to_output(d, "Purchase complete.");
                    send_to_imm("Restring Eq: %s has bought: %s, which was %s.", GET_NAME(d->character),
                                obj->short_description, d->obj_was);
                    STATE(d) = CON_PLAYING;
                } else if (!strcasecmp(arg, "No") || !strcasecmp(arg, "no") || !strcasecmp(arg, "n") ||
                           !strcasecmp(arg, "N")) {
                    write_to_output(d, "Canceling purchase at no cost.\r\n");
                    send_to_imm("Restring Eq: %s has canceled their equipment restring.", GET_NAME(d->character));
                    d->obj_editval = EDIT_NONE;
                    STATE(d) = CON_PLAYING;
                } else {
                    write_to_output(d, "Save current changes and be charged?\r\nYes or No\r\n");
                    return;
                }
                break;

            default: /* whoops */
                d->obj_editval = EDIT_RESTRING_MAIN;
                disp_restring_menu(d);
                return;
        } /* End main switch for restring */
    } /* End of equipment restring area */

    /* This section is dedicated to custom equipment construction */
    if (d->obj_editflag == EDIT_CUSTOM) {
        switch (d->obj_editval) {
            case EDIT_CUSTOM_MAIN: /* The main menu of custom equipment edit */
                switch (*arg) {
                    case '1': /* Let them name it. */
                        write_to_output(d, "@wEnter Equipment's Keyword Name @D(@RDo not use colorcode!@D)@w.@n\r\n");
                        write_to_output(d, "@wEnter: @n\n");
                        d->obj_editval = EDIT_CUSTOM_NAME;
                        break;
                    case '2': /* Let them give it a short description. */
                        write_to_output(d,
                                        "@wEnter Equipment's Short Description. This is the colored name you see in your inventory or when the eq is used.@n\r\n");
                        write_to_output(d, "@wEnter: @n\n");
                        d->obj_editval = EDIT_CUSTOM_SDESC;
                        break;
                    case '3': /* Let them give it a long description. */
                        write_to_output(d,
                                        "@wEnter Equipment's Long Description. This is the colored name you see when it's seen in a room. @D(@RNo punctuation at the end!@D)@n\r\n");
                        write_to_output(d, "@wEnter: @n\n");
                        d->obj_editval = EDIT_CUSTOM_LDESC;
                        break;
                    case '4': /* Let them choose what type of equipment it is. */
                        write_to_output(d, "@wEnter number to select type of equipment you want it to be: @n\n");
                        write_to_output(d,
                                        "@D[ @C--1@W) @cArmor Slot  @C--2@W) @cGi Slot     @C--3@W) @cWrist Slot   @D]@n\n");
                        write_to_output(d,
                                        "@D[ @C--4@W) @cEar Slot    @C--5@W) @cFinger Slot @C--6@W) @cEye Slot     @D]@n\n");
                        write_to_output(d,
                                        "@D[ @C--7@W) @cHands Slot  @C--8@W) @cFeet Slot   @C--9@W) @cBelt Slot    @D]@n\n");
                        write_to_output(d,
                                        "@D[ @C-10@W) @cLegs Slot   @C-11@W) @cArms Slot   @C-12@W) @cHead Slot    @D]@n\n");
                        write_to_output(d,
                                        "@D[ @C-13@W) @cNeck Slot   @C-14@W) @cBack Slot   @C-15@W) @cShoulder Slot@D]@n\n");
                        write_to_output(d, "@D[ @C-16@W) @cWeapon@n\r\n");
                        write_to_output(d, "@wEnter: @n\n");
                        d->obj_editval = EDIT_CUSTOM_TYPE;
                        break;
                    case '5': /* If it's a weapon then let them choose what type. */
                        if (d->obj_type != 16) {
                            write_to_output(d,
                                            "@wYou can only use this part of the menu if you select the weapon type.@n\r\n");
                            return;
                        } else {
                            write_to_output(d, "@wEnter number to select type of weapon you want it to be: @n\n");
                            write_to_output(d,
                                            "@D[ @C--1@W) @cSword       @C--2@W) @cDagger      @C--3@W) @cSpear        @D]@n\n");
                            write_to_output(d,
                                            "@D[ @C--4@W) @cClub        @C--5@W) @cGun         @C--6@W) @cBrawling     @D]@n\n");
                            write_to_output(d, "@wEnter: @n\n");
                            d->obj_editval = EDIT_CUSTOM_WEAPON;
                        }
                        break;
                    case 'q': /* They are exiting, time to finalize purchase or clear it. */
                    case 'Q':
                        write_to_output(d, "@wPurchase this custom piece? (Y or N)@n\r\n");
                        d->obj_editval = EDIT_CUSTOM_QUIT;
                        break;
                    default: /* They entered nothing, show them the menu. */
                        disp_custom_menu(d);
                        return;
                }
                break;

            case EDIT_CUSTOM_NAME: /* Name menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Keeping what was previously entered.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                    return;
                } else if (strstr(arg, "@")) {
                    write_to_output(d, "@RNO COLORCODE IN THE KEYWORD NAME.@n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else if (strlen(arg) > 100) {
                    write_to_output(d, "@wToo long. Limit is 100 character.@n\r\n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else {
                    if (d->obj_name) {
                        free(d->obj_name);
                    }
                    *buf = '\0';
                    sprintf(buf, "%s", arg);
                    d->obj_name = strdup(buf);
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                }
                break;

            case EDIT_CUSTOM_SDESC: /* Short Desc Menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Keeping what was previously entered.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                    return;
                } else if (strlen(arg) > 150) {
                    write_to_output(d, "@wToo long. Limit is 200 character.@n\r\n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else {
                    if (d->obj_short) {
                        free(d->obj_short);
                    }
                    *buf = '\0';
                    snprintf(buf, sizeof(buf), "%s", arg);
                    d->obj_short = strdup(buf);
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                }
                break;

            case EDIT_CUSTOM_LDESC: /* Long Desc Menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Keeping what was previously entered.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                    return;
                } else if (strlen(arg) > 200) {
                    write_to_output(d, "@wToo long. Limit is 200 character.@n\r\n");
                    write_to_output(d, "@wEnter: @n\n");
                    return;
                } else {
                    if (d->obj_long) {
                        free(d->obj_long);
                    }
                    *buf = '\0';
                    snprintf(buf, sizeof(buf), "%s", arg);
                    d->obj_long = strdup(buf);
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                }
                break;

            case EDIT_CUSTOM_TYPE: /* Type Menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Returning to main menu.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                    return;
                } else {
                    d->obj_type = atoi(arg);
                    if (d->obj_type < 1 || d->obj_type > 16) {
                        write_to_output(d, "@wValue must be between 1 and 16.@n\r\n");
                        write_to_output(d, "@wEnter: @n\n");
                        d->obj_type = 1;
                        return;
                    } else {
                        if (d->obj_type == 16) {
                            d->obj_weapon = 1;
                        }
                        disp_custom_menu(d);
                        d->obj_editval = EDIT_CUSTOM_MAIN;
                    }
                }
                break;

            case EDIT_CUSTOM_WEAPON: /* Weapon Menu */
                if (!*arg) {
                    write_to_output(d, "@wNothing entered. Returning to main menu.@n\r\n");
                    disp_custom_menu(d);
                    d->obj_editval = EDIT_CUSTOM_MAIN;
                    return;
                } else {
                    d->obj_weapon = atoi(arg);
                    if (d->obj_weapon < 1 || d->obj_weapon > 6) {
                        write_to_output(d, "@wValue must be between 1 and 6.@n\r\n");
                        write_to_output(d, "@wEnter: @n\n");
                        d->obj_weapon = 0;
                        return;
                    } else {
                        disp_custom_menu(d);
                        d->obj_editval = EDIT_CUSTOM_MAIN;
                    }
                }
                break;

            case EDIT_CUSTOM_QUIT: /* Quiting */
                if (!*arg) {
                    write_to_output(d, "@wPurchase this custom piece? (Y or N)@n\r\n");
                    return;
                } else if (!strcasecmp(arg, "y") || !strcasecmp(arg, "Y")) {
                    write_to_output(d, "@wPurchase complete.@n\r\n");
                    STATE(d) = CON_PLAYING;
                    if (d->obj_weapon == 0) {
                        obj = read_object(20099, VIRTUAL);
                        obj_to_char(obj, d->character);
                        switch (d->obj_type) {
                            case 1:
                                obj->wear_flags.set(ITEM_WEAR_BODY);
                                break;
                            case 2:
                                obj->wear_flags.set(ITEM_WEAR_ABOUT);
                                break;
                            case 3:
                                obj->wear_flags.set(ITEM_WEAR_WRIST);
                                break;
                            case 4:
                                obj->wear_flags.set(ITEM_WEAR_EAR);
                                break;
                            case 5:
                                obj->wear_flags.set(ITEM_WEAR_FINGER);
                                break;
                            case 6:
                                obj->wear_flags.set(ITEM_WEAR_EYE);
                                break;
                            case 7:
                                obj->wear_flags.set(ITEM_WEAR_HANDS);
                                break;
                            case 8:
                                obj->wear_flags.set(ITEM_WEAR_FEET);
                                break;
                            case 9:
                                obj->wear_flags.set(ITEM_WEAR_WAIST);
                                break;
                            case 10:
                                obj->wear_flags.set(ITEM_WEAR_LEGS);
                                break;
                            case 11:
                                obj->wear_flags.set(ITEM_WEAR_ARMS);
                                break;
                            case 12:
                                obj->wear_flags.set(ITEM_WEAR_HEAD);
                                break;
                            case 13:
                                obj->wear_flags.set(ITEM_WEAR_NECK);
                                break;
                            case 14:
                                obj->wear_flags.set(ITEM_WEAR_PACK);
                                break;
                            case 15:
                                obj->wear_flags.set(ITEM_WEAR_SH);
                                break;
                        }
                        *buf = '\0';
                        sprintf(buf, d->obj_name);
                        obj->name = strdup(buf);
                        *buf2 = '\0';
                        sprintf(buf2, d->obj_short);
                        obj->short_description = strdup(buf2);
                        *buf3 = '\0';
                        sprintf(buf3, d->obj_long);
                        obj->room_description = strdup(buf3);
                    } else {
                        obj = read_object(20098, VIRTUAL);
                        obj_to_char(obj, d->character);
                        *buf = '\0';
                        sprintf(buf, "%s", d->obj_name);
                        obj->name = strdup(buf);
                        *buf2 = '\0';
                        sprintf(buf2, "%s", d->obj_short);
                        obj->short_description = strdup(buf2);
                        *buf3 = '\0';
                        sprintf(buf3, "%s", d->obj_long);
                        obj->room_description = strdup(buf3);
                        switch (d->obj_weapon) {
                            case 1:
                                GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) = TYPE_SLASH - TYPE_HIT;
                                break;
                            case 2:
                                GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) = TYPE_PIERCE - TYPE_HIT;
                                break;
                            case 3:
                                GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) = TYPE_STAB - TYPE_HIT;
                                break;
                            case 4:
                                GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) = TYPE_CRUSH - TYPE_HIT;
                                break;
                            case 5:
                                GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) = TYPE_BLAST - TYPE_HIT;
                                break;
                            case 6:
                                GET_OBJ_VAL(obj, VAL_WEAPON_DAMTYPE) = TYPE_POUND - TYPE_HIT;
                                break;
                        }
                        GET_OBJ_LEVEL(obj) = 20;
                    }
                    for(auto f : {ITEM_SLOT2, ITEM_CUSTOM}) obj->extra_flags.set(f);
                    d->obj_editflag = EDIT_NONE;
                    d->obj_editval = EDIT_NONE;
                    d->character->modRPP(-20);
                    GET_OBJ_SIZE(obj) = get_size(d->character);
                    send_to_imm("Custom Eq: %s has bought: %s.", GET_NAME(d->character), obj->short_description);
                    d->account->customs.emplace_back(obj->short_description);
                    dirty_accounts.insert(d->account->vn);
                    log_custom(d, obj);
                } else if (!strcasecmp(arg, "n") || !strcasecmp(arg, "N")) {
                    write_to_output(d, "Canceling purchase at no cost.\r\n");
                    send_to_imm("Custom Eq: %s has canceled their custom eq construction.", GET_NAME(d->character));
                    d->obj_editval = EDIT_NONE;

                    STATE(d) = CON_PLAYING;
                }
                break;

        } /* End main custom switch */
    }/* End Custom Area */

} /* End pobj_edit_parse */
