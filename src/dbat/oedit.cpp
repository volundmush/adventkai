/************************************************************************
 * OasisOLC - Objects / oedit.c					v2.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/structs.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/spells.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/boards.h"
#include "dbat/constants.h"
#include "dbat/shop.h"
#include "dbat/genolc.h"
#include "dbat/genobj.h"
#include "dbat/genzon.h"
#include "dbat/oasis.h"
#include "dbat/improved-edit.h"
#include "dbat/dg_olc.h"
#include "dbat/feats.h"
#include "dbat/act.informative.h"
#include "dbat/act.wizard.h"
#include "dbat/races.h"
#include "dbat/fight.h"

/*------------------------------------------------------------------------*/

/*
 * Handy macros.
 */
#define S_PRODUCT(s, i) ((s)->producing[(i)])

/*------------------------------------------------------------------------*\
  Utility and exported functions
\*------------------------------------------------------------------------*/

ACMD(do_oasis_oedit) {
    int number = NOWHERE, save = 0, real_num;
    struct descriptor_data *d;
    char *buf3;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    /****************************************************************************/
    /** Parse any arguments.                                                   **/
    /****************************************************************************/
    buf3 = two_arguments(argument, buf1, buf2);

    /****************************************************************************/
    /** If there aren't any arguments...well...they can't modify nothing now   **/
    /** can they?                                                              **/
    /****************************************************************************/
    if (!*buf1) {
        send_to_char(ch, "Specify an object VNUM to edit.\r\n");
        return;
    } else if (!isdigit(*buf1)) {
        if (strcasecmp("save", buf1) != 0) {
            send_to_char(ch, "Yikes!  Stop that, someone will get hurt!\r\n");
            return;
        }

        save = true;

        if (is_number(buf2))
            number = atoi(buf2);
        else if (GET_OLC_ZONE(ch) > 0) {
            zone_rnum zlok;

            if ((zlok = real_zone(GET_OLC_ZONE(ch))) == NOWHERE)
                number = NOWHERE;
            else
                number = genolc_zone_bottom(zlok);
        }

        if (number == NOWHERE) {
            send_to_char(ch, "Save which zone?\r\n");
            return;
        }
    }

    /****************************************************************************/
    /** If a numeric argument was given, get it.                               **/
    /****************************************************************************/
    if (number == NOWHERE)
        number = atoi(buf1);

    /****************************************************************************/
    /** Check that whatever it is isn't already being edited.                  **/
    /****************************************************************************/
    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) == CON_OEDIT) {
            if (d->olc && OLC_NUM(d) == number) {
                send_to_char(ch, "That object is currently being edited by %s.\r\n",
                             PERS(d->character, ch));
                return;
            }
        }
    }

    /****************************************************************************/
    /** Point d to the builder's descriptor (for easier typing later).         **/
    /****************************************************************************/
    d = ch->desc;

    /****************************************************************************/
    /** Give the descriptor an OLC structure.                                  **/
    /****************************************************************************/
    if (d->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true,
               "SYSERR: do_oasis: Player already had olc structure.");
        free(d->olc);
    }

    CREATE(d->olc, struct oasis_olc_data, 1);

    /****************************************************************************/
    /** Find the zone.                                                         **/
    /****************************************************************************/
    OLC_ZNUM(d) = save ? real_zone(number) : real_zone_by_thing(number);
    if (OLC_ZNUM(d) == NOWHERE) {
        send_to_char(ch, "Sorry, there is no zone for that number!\r\n");

        /**************************************************************************/
        /** Free the descriptor's OLC structure.                                 **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /****************************************************************************/
    /** Everyone but IMPLs can only edit zones they have been assigned.        **/
    /****************************************************************************/
    if (!can_edit_zone(ch, OLC_ZNUM(d))) {
        send_cannot_edit(ch, zone_table[OLC_ZNUM(d)].number);

        /**************************************************************************/
        /** Free the descriptor's OLC structure.                                 **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /****************************************************************************/
    /** If we need to save, save the objects.                                  **/
    /****************************************************************************/
    if (save) {
        send_to_char(ch, "Saving all objects in zone %d.\r\n",
                     zone_table[OLC_ZNUM(d)].number);
        mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(ch)), true,
               "OLC: %s saves object info for zone %d.", GET_NAME(ch),
               zone_table[OLC_ZNUM(d)].number);

        /**************************************************************************/
        /** Save the objects in this zone.                                       **/
        /**************************************************************************/
        save_objects(OLC_ZNUM(d));

        /**************************************************************************/
        /** Free the descriptor's OLC structure.                                 **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    OLC_NUM(d) = number;

    /****************************************************************************/
    /** If this is a new object, setup a new object, otherwise setup the       **/
    /** existing object.                                                       **/
    /****************************************************************************/
    if ((real_num = real_object(number)) != NOTHING)
        oedit_setup_existing(d, real_num);
    else
        oedit_setup_new(d);

    oedit_disp_menu(d);
    STATE(d) = CON_OEDIT;

    /****************************************************************************/
    /** Send the OLC message to the players in the same room as the builder.   **/
    /****************************************************************************/
    act("$n starts using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);
    ch->playerFlags.set(PLR_WRITING);

    /****************************************************************************/
    /** Log the OLC message.                                                   **/
    /****************************************************************************/
    mudlog(BRF, ADMLVL_IMMORT, true, "OLC: %s starts editing zone %d allowed zone %d",
           GET_NAME(ch), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(ch));
}

void oedit_setup_new(struct descriptor_data *d) {
    OLC_OBJ(d) = new obj_data();

    OLC_OBJ(d)->name = strdup("unfinished object");
    OLC_OBJ(d)->room_description = strdup("An unfinished object is lying here.");
    OLC_OBJ(d)->short_description = strdup("an unfinished object");
    OLC_OBJ(d)->wear_flags.set(ITEM_WEAR_TAKE);
    OLC_VAL(d) = 0;
    OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
    GET_OBJ_TYPE(OLC_OBJ(d)) = ITEM_WORN;
    GET_OBJ_VAL(OLC_OBJ(d), VAL_ALL_HEALTH) = 100;
    GET_OBJ_VAL(OLC_OBJ(d), VAL_ALL_MAXHEALTH) = 100;
    GET_OBJ_VAL(OLC_OBJ(d), VAL_ALL_MATERIAL) = MATERIAL_STEEL;
    GET_OBJ_SIZE(OLC_OBJ(d)) = SIZE_MEDIUM;

    SCRIPT(OLC_OBJ(d)) = nullptr;
    OLC_OBJ(d)->proto_script.clear();
    OLC_SCRIPT(d).clear();
}

/*------------------------------------------------------------------------*/

void oedit_setup_existing(struct descriptor_data *d, int real_num) {
    struct obj_data *obj = new obj_data();
    copy_object(obj, &obj_proto[real_num]);

    /*
     * Attach new object to player's descriptor.
     */
    OLC_OBJ(d) = obj;
    OLC_VAL(d) = 0;
    OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
    dg_olc_script_copy(d);
    /*
     * The edited obj must not have a script.
     * It will be assigned to the updated obj later, after editing.
     */
    SCRIPT(obj) = nullptr;
    OLC_OBJ(d)->proto_script.clear();
}

/*------------------------------------------------------------------------*/

void oedit_save_internally(struct descriptor_data *d) {
    int i;
    obj_rnum robj_num;
    struct descriptor_data *dsc;
    struct obj_data *obj;

    i = (real_object(OLC_NUM(d)) == NOTHING);

    if ((robj_num = add_object(OLC_OBJ(d), OLC_NUM(d))) == NOTHING) {
        basic_mud_log("oedit_save_internally: add_object failed.");
        return;
    }

    /* Update triggers : */
    /* Free old proto list  */
    if (obj_proto[robj_num].proto_script != OLC_SCRIPT(d))
        free_proto_script(&obj_proto[robj_num], OBJ_TRIGGER);
    /* this will handle new instances of the object: */
    obj_proto[robj_num].proto_script = OLC_SCRIPT(d);

    /* this takes care of the objects currently in-game */
    for (obj = object_list; obj; obj = obj->next) {
        if (obj->vn != robj_num)
            continue;
        /* remove any old scripts */
        if (SCRIPT(obj))
            extract_script(obj, OBJ_TRIGGER);

        free_proto_script(obj, OBJ_TRIGGER);
        copy_proto_script(&obj_proto[robj_num], obj, OBJ_TRIGGER);
        assign_triggers(obj, OBJ_TRIGGER);
    }
    /* end trigger update */

    if (!i)    /* If it's not a new object, don't renumber. */
        return;

    /* Update other people in zedit too. From: C.Raehl 4/27/99 */
    for (dsc = descriptor_list; dsc; dsc = dsc->next)
        if (STATE(dsc) == CON_ZEDIT)
            for (i = 0; OLC_ZONE(dsc)->cmd[i].command != 'S'; i++)
                switch (OLC_ZONE(dsc)->cmd[i].command) {
                    case 'P':
                        OLC_ZONE(dsc)->cmd[i].arg3 += (OLC_ZONE(dsc)->cmd[i].arg3 >= robj_num);
                        /* Fall through. */
                    case 'E':
                    case 'G':
                    case 'O':
                        OLC_ZONE(dsc)->cmd[i].arg1 += (OLC_ZONE(dsc)->cmd[i].arg1 >= robj_num);
                        break;
                    case 'R':
                        OLC_ZONE(dsc)->cmd[i].arg2 += (OLC_ZONE(dsc)->cmd[i].arg2 >= robj_num);
                        break;
                    default:
                        break;
                }
}

/*------------------------------------------------------------------------*/

void oedit_save_to_disk(int zone_num) {
    save_objects(zone_num);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * For container flags.
 */
void oedit_disp_container_flags_menu(struct descriptor_data *d) {
    char bits[MAX_STRING_LENGTH];
    clear_screen(d);

    sprintbit(GET_OBJ_VAL(OLC_OBJ(d), 1), container_bits, bits, sizeof(bits));
    write_to_output(d,
                    "@g1@n) CLOSEABLE\r\n"
                    "@g2@n) PICKPROOF\r\n"
                    "@g3@n) CLOSED\r\n"
                    "@g4@n) LOCKED\r\n"
                    "Container flags: @c%s@n\r\n"
                    "Enter flag, 0 to quit : ",
                    bits);
}

/*
 * For extra descriptions.
 */
void oedit_disp_extradesc_menu(struct descriptor_data *d) {
    struct extra_descr_data *extra_desc = OLC_DESC(d);

    clear_screen(d);
    write_to_output(d,
                    "Extra desc menu\r\n"
                    "@g1@n) Keyword: @y%s@n\r\n"
                    "@g2@n) Description:\r\n@y%s@n\r\n"
                    "@g3@n) Goto next description: %s\r\n"
                    "@g0@n) Quit\r\n"
                    "Enter choice : ",

                    (extra_desc->keyword && *extra_desc->keyword) ? extra_desc->keyword : "<NONE>",
                    (extra_desc->description && *extra_desc->description) ? extra_desc->description : "<NONE>",
                    !extra_desc->next ? "Not set." : "Set.");
    OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

/*
 * Ask for *which* apply to edit.
 */
void oedit_disp_prompt_apply_menu(struct descriptor_data *d) {
    char apply_buf[MAX_STRING_LENGTH];
    int counter;

    clear_screen(d);

    for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
        if (OLC_OBJ(d)->affected[counter].modifier) {
            sprinttype(OLC_OBJ(d)->affected[counter].location, apply_types, apply_buf, sizeof(apply_buf));
            write_to_output(d, " @g%d@n) %+d to @b%s@n", counter + 1,
                            OLC_OBJ(d)->affected[counter].modifier, apply_buf);
            switch (OLC_OBJ(d)->affected[counter].location) {
                case APPLY_FEAT:
                    write_to_output(d, " (%s)", feat_list[OLC_OBJ(d)->affected[counter].specific].name);
                    break;
                case APPLY_SKILL:
                    write_to_output(d, " (%s)", spell_info[OLC_OBJ(d)->affected[counter].specific].name);
                    break;
            }
            write_to_output(d, "\r\n");
        } else {
            write_to_output(d, " @g%d@n) None.\r\n", counter + 1);
        }
    }
    write_to_output(d, "\r\nEnter affection to modify (0 to quit) : ");
    OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

void oedit_disp_prompt_spellbook_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < SPELLBOOK_SIZE; counter++) {
        if (OLC_OBJ(d)->sbinfo && OLC_OBJ(d)->sbinfo[counter].spellname != 0 &&
            OLC_OBJ(d)->sbinfo[counter].spellname < MAX_SPELLS) {
            write_to_output(d, " @g%3d@n) %-20.20s %s", counter + 1,
                            spell_info[OLC_OBJ(d)->sbinfo[counter].spellname].name, !(++columns % 3) ? "\r\n" : "");
        } else {
            write_to_output(d, " @g%3d@n) None.%s", counter + 1, !(++columns % 3) ? "\r\n" : "");
        }
    }
    write_to_output(d, "\r\nEnter spell to modify (0 to quit) : ");
    OLC_MODE(d) = OEDIT_PROMPT_SPELLBOOK;
}

void oedit_disp_spellbook_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < SKILL_TABLE_SIZE; counter++) {
        if (spell_info[counter].skilltype == SKTYPE_SPELL)
            write_to_output(d, "@g%3d@n) @y%-20.20s@n%s", counter,
                            spell_info[counter].name, !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "@n\r\nEnter spell number (0 is no spell) : ");
    OLC_MODE(d) = OEDIT_SPELLBOOK;
}


/*
 * Some applies require parameters (skills, feats)
 */
void oedit_disp_apply_spec_menu(struct descriptor_data *d) {
    char *buf;

    switch (OLC_OBJ(d)->affected[OLC_VAL(d)].location) {
        case APPLY_FEAT:
            buf = "What feat should be modified : ";
            break;
        case APPLY_SKILL:
            buf = "What skill should be modified : ";
            break;
        default:
            oedit_disp_prompt_apply_menu(d);
            return;
    }

    write_to_output(d, "\r\n%s", buf);
    OLC_MODE(d) = OEDIT_APPLYSPEC;
}

/*
 * Ask for liquid type.
 */
void oedit_liquid_type(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_LIQ_TYPES; counter++) {
        write_to_output(d, " @g%2d@n) @y%-20.20s@n%s", counter,
                        drinks[counter], !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\n@nEnter drink type : ");
    OLC_MODE(d) = OEDIT_VALUE_3;
}

/*
 * The actual apply to set.
 */
void oedit_disp_apply_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_APPLIES; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter,
                        apply_types[counter], !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter apply type (0 is no apply) : ");
    OLC_MODE(d) = OEDIT_APPLY;
}

/*
 * Weapon critical type.
 */
void oedit_disp_crittype_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter <= MAX_CRIT_TYPE; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter,
                        crit_type[counter], !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter critical type : ");
}

/*
 * Weapon type.
 */
void oedit_disp_weapon_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_ATTACK_TYPES; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter,
                        attack_hit_text[counter].singular,
                        !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter weapon type : ");
}

/*
 * Armor type.
 */
void oedit_disp_armor_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter <= MAX_ARMOR_TYPES; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter,
                        armor_type[counter],
                        !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter armor proficiency type : ");
}

/*
 * Spell type.
 */
void oedit_disp_spells_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < SKILL_TABLE_SIZE; counter++) {
        if (IS_SET(skill_type(counter), SKTYPE_SPELL))
            write_to_output(d, "@g%2d@n) @y%-20.20s@n%s", counter,
                            spell_info[counter].name, !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\n@nEnter spell choice (-1 for none) : ");
}

/*
 * Material type.
 */
void oedit_disp_material_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_MATERIALS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s%s", counter,
                        material_names[counter],
                        !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\n@nEnter material type : ");
}

/*
 * Object value #1
 */
void oedit_disp_val1_menu(struct descriptor_data *d) {
    int counter, columns = 0;
    OLC_MODE(d) = OEDIT_VALUE_1;
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_LIGHT:
            /*
             * values 0 and 1 are unused.. jump to 2
             */
            oedit_disp_val3_menu(d);
            break;
        case ITEM_SCROLL:
        case ITEM_WAND:
        case ITEM_STAFF:
        case ITEM_POTION:
            write_to_output(d, "Spell level : ");
            break;
        case ITEM_WEAPON:
            /*
             * This is now used to control the weapon type used by the weapon object
             */
            for (counter = 0; counter <= MAX_WEAPON_TYPES; counter++) {
                write_to_output(d, "@g%2d@n) %-20.20s %s", counter,
                                weapon_type[counter], !(++columns % 3) ? "\r\n" : "");
            }
            write_to_output(d, "\r\nEnter the weapon type for determining proficiencies: \r\n");
            break;
        case ITEM_ARMOR:
            write_to_output(d, "Apply to AC : ");
            break;
        case ITEM_CONTAINER:
            write_to_output(d, "Max weight to contain (-1 for unlimited) : ");
            break;
        case ITEM_DRINKCON:
        case ITEM_FOUNTAIN:
            write_to_output(d, "Max drink units (-1 for unlimited) : ");
            break;
        case ITEM_FOOD:
            write_to_output(d, "Hours to fill stomach : ");
            break;
        case ITEM_MONEY:
            write_to_output(d, "Number of zenni : ");
            break;
        case ITEM_NOTE:
            /*
             * This is supposed to be language, but it's unused.
             */
            break;
        case ITEM_VEHICLE:
            write_to_output(d, "Enter room vnum of vehicle interior : ");
            break;
        case ITEM_HATCH:
            write_to_output(d, "Enter vnum of the vehicle this hatch belongs to : ");
            break;
        case ITEM_WINDOW:
            write_to_output(d,
                            "Enter vnum of the vehicle this window belongs to, or -1 to specify the viewport room : ");
            break;
        case ITEM_CONTROL:
            write_to_output(d, "Enter vnum of the vehicle these controls belong to : ");
            break;
        case ITEM_PORTAL:
            write_to_output(d, "Which room number is the destination? : ");
            break;
        case ITEM_BOARD:
            write_to_output(d, "Enter the minimum admin level to read this board (0 for mortals) : ");
            break;
        default:
            oedit_disp_val5_menu(d);
    }
}

/*
 * Object value #2
 */
void oedit_disp_val2_menu(struct descriptor_data *d) {
    OLC_MODE(d) = OEDIT_VALUE_2;
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_SCROLL:
        case ITEM_POTION:
            oedit_disp_spells_menu(d);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            write_to_output(d, "Max number of charges : ");
            break;
        case ITEM_WEAPON:
            write_to_output(d, "Number of damage dice : ");
            break;
        case ITEM_ARMOR:
            oedit_disp_armor_menu(d);
            break;
        case ITEM_FOOD:
            /*
             * Values 2 and 3 are unused, jump to 4...Odd.
             */
            oedit_disp_val4_menu(d);
            break;
        case ITEM_CONTROL:
            write_to_output(d, "Enter Engine Type ( 1, 2, 3) : ");
            break;
        case ITEM_CONTAINER:
        case ITEM_VEHICLE:
        case ITEM_HATCH:
        case ITEM_WINDOW:
        case ITEM_PORTAL:
            /*
             * These are flags, needs a bit of special handling.
             */
            oedit_disp_container_flags_menu(d);
            break;
        case ITEM_DRINKCON:
        case ITEM_FOUNTAIN:
            write_to_output(d, "Initial drink units : ");
            break;
        case ITEM_BOARD:
            write_to_output(d, "Minimum admin level to write (0 for mortals) : ");
            break;
        default:
            oedit_disp_val5_menu(d);
    }
}

/*
 * Object value #3
 */
void oedit_disp_val3_menu(struct descriptor_data *d) {
    OLC_MODE(d) = OEDIT_VALUE_3;
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_LIGHT:
            write_to_output(d, "Number of hours (0 = burnt, -1 is infinite) : ");
            break;
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            write_to_output(d, "Number of charges remaining : ");
            break;
        case ITEM_WEAPON:
            write_to_output(d, "Size of damage dice : ");
            break;
        case ITEM_ARMOR:
            write_to_output(d, "Max dex bonus : ");
            break;
        case ITEM_CONTAINER:
            write_to_output(d, "Vnum of key to open container (-1 for no key) : ");
            break;
        case ITEM_DRINKCON:
        case ITEM_FOUNTAIN:
            oedit_liquid_type(d);
            break;
        case ITEM_VEHICLE:
            write_to_output(d, "Vnum of key to unlock vehicle (-1 for no key) : ");
            break;
        case ITEM_HATCH:
            write_to_output(d, "Vnum of key to unlock hatch (-1 for no key) : ");
            break;
        case ITEM_WINDOW:
            write_to_output(d, "Vnum of key to unlock window (-1 for no key) : ");
            break;
        case ITEM_PORTAL:
            write_to_output(d, "Vnum of the key to unlock portal (-1 for no key) : ");
            break;
        case ITEM_BOARD:
            write_to_output(d, "Minimum admin level to remove messages (0 for mortals) : ");
            break;
        default:
            oedit_disp_val5_menu(d);
    }
}

/*
 * Object value #4
 */
void oedit_disp_val4_menu(struct descriptor_data *d) {
    OLC_MODE(d) = OEDIT_VALUE_4;
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_WAND:
        case ITEM_STAFF:
            oedit_disp_spells_menu(d);
            break;
        case ITEM_WEAPON:
            oedit_disp_weapon_menu(d);
            break;
        case ITEM_ARMOR:
            write_to_output(d, "Armor check penalty : ");
            break;
        case ITEM_DRINKCON:
        case ITEM_FOUNTAIN:
        case ITEM_FOOD:
            write_to_output(d, "Poisoned (0 = not poison) : ");
            break;
        case ITEM_VEHICLE:
            write_to_output(d, "What is the vehicle's appearance? (-1 for transparent) : ");
            break;
        case ITEM_HATCH:
            write_to_output(d, "Enter default vehicle load room : ");
            break;
        case ITEM_PORTAL:
            write_to_output(d, "What is the portal's appearance? (-1 for transparent) : ");
            break;
        case ITEM_WINDOW:
            if (GET_OBJ_VAL(OLC_OBJ(d), 0) < 0)
                write_to_output(d, "What is the viewport room vnum (-1 for default location) : ");
            else
                oedit_disp_menu(d);
            break;
        default:
            oedit_disp_val5_menu(d);
    }
}

/*
 * Object value #5
 */
void oedit_disp_val5_menu(struct descriptor_data *d) {
    OLC_MODE(d) = OEDIT_VALUE_5;
    write_to_output(d, "Enter object default quality percentage (100%% MAX): ");
}

/*
 * Object value #7
 */
void oedit_disp_val7_menu(struct descriptor_data *d) {
    OLC_MODE(d) = OEDIT_VALUE_7;

    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_WEAPON:
            oedit_disp_crittype_menu(d);
            break;
        case ITEM_ARMOR:
            write_to_output(d, "Arcane spell failure %% : ");
            break;
        default:
            oedit_disp_val9_menu(d);
            break;
    }
}

/*
 * Object value #9
 */
void oedit_disp_val9_menu(struct descriptor_data *d) {
    OLC_MODE(d) = OEDIT_VALUE_9;

    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_WEAPON:
            write_to_output(d, "Default crit is only on natural 20. Extend this range by: ");
            break;
        default:
            oedit_disp_menu(d);
            break;
    }
}

/*
 * Object type.
 */
void oedit_disp_type_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_ITEM_TYPES; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter,
                        item_types[counter], !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter object type : ");
}

/*
 * Object extra flags.
 */
void oedit_disp_extra_menu(struct descriptor_data *d) {
    char bits[MAX_STRING_LENGTH];
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_ITEM_FLAGS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter + 1,
                        extra_bits[counter], !(++columns % 3) ? "\r\n" : "");
    }
    sprintbitarray(GET_OBJ_EXTRA(OLC_OBJ(d)), extra_bits, EF_ARRAY_MAX, bits);
    write_to_output(d, "\r\nObject flags: @c%s@n\r\n"
                       "Enter object extra flag (0 to quit) : ",
                    bits);
}

/*
 * Object perm flags.
 */
void oedit_disp_perm_menu(struct descriptor_data *d) {
    char bitbuf[MAX_STRING_LENGTH];
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 1; counter < NUM_AFF_FLAGS; counter++) {
        /* Setting AFF_CHARM on objects like this is dangerous. */
        if (counter == AFF_CHARM)
            continue;
        write_to_output(d, "@g%2d@n) %-20.20s%s", counter,
                        affected_bits[counter], !(++columns % 3) ? "\r\n" : "");
    }
    sprintbitarray(GET_OBJ_PERM(OLC_OBJ(d)), affected_bits, EF_ARRAY_MAX, bitbuf);
    write_to_output(d, "\r\nObject permanent flags: @c%s@n\r\n"
                       "Enter object perm flag (0 to quit) : ", bitbuf);
}

/*
 * Object size
 */
void oedit_disp_size_menu(struct descriptor_data *d) {
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_SIZES; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s%s", counter + 1,
                        size_names[counter], !(++columns % 3) ? "\r\n" : "");
    }
    write_to_output(d, "\r\nEnter object size : ");
}

/*
 * Object wear flags.
 */
void oedit_disp_wear_menu(struct descriptor_data *d) {
    char bits[MAX_STRING_LENGTH];
    int counter, columns = 0;

    clear_screen(d);

    for (counter = 0; counter < NUM_ITEM_WEARS; counter++) {
        write_to_output(d, "@g%2d@n) %-20.20s %s", counter + 1,
                        wear_bits[counter], !(++columns % 3) ? "\r\n" : "");
    }
    sprintbitarray(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, TW_ARRAY_MAX, bits);
    write_to_output(d, "\r\nWear flags: @c%s@n\r\n"
                       "Enter wear flag, 0 to quit : ", bits);
}

/*
 * Display main menu.
 */
void oedit_disp_menu(struct descriptor_data *d) {
    char tbitbuf[MAX_INPUT_LENGTH], ebitbuf[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    obj = OLC_OBJ(d);
    clear_screen(d);

    /*
     * Build buffers for first part of menu.
     */
    sprinttype(GET_OBJ_TYPE(obj), item_types, tbitbuf, sizeof(tbitbuf));
    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, ebitbuf);

    /*
     * Build first half of menu.
     */
    write_to_output(d,
                    "-- Item number : [@c%d@n]\r\n"
                    "@g1@n) Namelist : @y%s@n\r\n"
                    "@g2@n) S-Desc   : @y%s@n\r\n"
                    "@g3@n) L-Desc   :-\r\n@y%s@n\r\n"
                    "@g4@n) A-Desc   :-\r\n@y%s@n"
                    "@g5@n) Type        : @c%s@n\r\n"
                    "@g6@n) Extra flags : @c%s@n\r\n",

                    OLC_NUM(d),
                    (obj->name && *obj->name) ? obj->name : "undefined",
                    (obj->short_description && *obj->short_description) ? obj->short_description : "undefined",
                    (obj->room_description && *obj->room_description) ? obj->room_description : "undefined",
                    (obj->look_description && *obj->look_description) ? obj->look_description : "Not Set.\r\n",
                    tbitbuf,
                    ebitbuf
    );
    /*
     * Send first half.
     */

    /*
     * Build second half of menu.
     */
    sprintbitarray(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, EF_ARRAY_MAX, tbitbuf);
    sprintbitarray(GET_OBJ_PERM(OLC_OBJ(d)), affected_bits, EF_ARRAY_MAX, ebitbuf);

    write_to_output(d,
                    "@g7@n) Wear flags  : @c%s@n\r\n"
                    "@g8@n) Weight      : @c%-4" I64T "@n, 	@g9@n) Cost        : @c%-4d@n\r\n"
                    "@gA@n) Cost/Day    : @c%-4d@n, 	@gB@n) Timer       : @c%-4d@n\r\n"
                    "@gC@n) Values      : @c%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d@n\r\n"
                    "@gD@n) Applies menu@n\r\n"
                    "@gE@n) Extra descriptions menu %s\r\n"
                    "@gM@n) Min Level   : @c%d@n\r\n"
                    "@gN@n) Material    : @c%s@n\r\n"
                    "@gP@n) Perm Affects: @c%s@n\r\n"
                    "@gS@n) Script      : @c%s@n\r\n"
                    "@gT@n) Spellbook menu\r\n"
                    "@gW@n) Copy object        ,	@gX@n) Delete object\r\n"
                    "@gY@n) Size        : @c%s@n\r\n"
                    "@gZ@n) Wiznet      :\r\n"
                    "@gQ@n) Quit\r\n"
                    "Enter choice : ",

                    tbitbuf, GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj),
                    GET_OBJ_TIMER(obj), GET_OBJ_VAL(obj, 0), GET_OBJ_VAL(obj, 1),
                    GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 4),
                    GET_OBJ_VAL(obj, 5), GET_OBJ_VAL(obj, 6), GET_OBJ_VAL(obj, 7),
                    GET_OBJ_VAL(obj, 8), GET_OBJ_VAL(obj, 9), GET_OBJ_VAL(obj, 10),
                    GET_OBJ_VAL(obj, 11), GET_OBJ_VAL(obj, 12), GET_OBJ_VAL(obj, 13),
                    GET_OBJ_VAL(obj, 14), GET_OBJ_VAL(obj, 15), obj->extra_flags.any() ? "Set." : "Not Set.",
                    GET_OBJ_LEVEL(obj), material_names[(int) GET_OBJ_MATERIAL(obj)],
                    ebitbuf, !OLC_SCRIPT(d).empty() ? "Set." : "Not Set.",
                    size_names[GET_OBJ_SIZE(obj)]
    );
    OLC_MODE(d) = OEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/

void oedit_parse(struct descriptor_data *d, char *arg) {
    int number, max_val, min_val;
    char *oldtext = nullptr;
    struct board_info *tmp;
    struct obj_data *obj;
    obj_rnum robj;

    switch (OLC_MODE(d)) {

        case OEDIT_CONFIRM_SAVESTRING:
            switch (*arg) {
                case 'y':
                case 'Y':
                    oedit_save_internally(d);
                    mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(d->character)), true,
                           "OLC: %s edits obj %d", GET_NAME(d->character), OLC_NUM(d));
                    if (CONFIG_OLC_SAVE) {
                        oedit_save_to_disk(real_zone_by_thing(OLC_NUM(d)));
                        write_to_output(d, "Object saved to disk.\r\n");
                    } else
                        write_to_output(d, "Object saved to memory.\r\n");
                    if (GET_OBJ_TYPE(OLC_OBJ(d)) == ITEM_BOARD) {
                        if ((tmp = locate_board(GET_OBJ_VNUM(OLC_OBJ(d)))) != nullptr) {
                            save_board(tmp);
                        } else {
                            tmp = create_new_board(GET_OBJ_VNUM(OLC_OBJ(d)));
                            BOARD_NEXT(tmp) = bboards;
                            bboards = tmp;
                        }
                    }
                    cleanup_olc(d, CLEANUP_ALL);
                    return;
                case 'n':
                case 'N':
                    /* If not saving, we must free the script_proto list. */
                    OLC_OBJ(d)->proto_script = OLC_SCRIPT(d);
                    free_proto_script(OLC_OBJ(d), OBJ_TRIGGER);
                    cleanup_olc(d, CLEANUP_ALL);
                    return;
                case 'a': /* abort quit */
                case 'A':
                    oedit_disp_menu(d);
                    return;
                default:
                    write_to_output(d, "Invalid choice!\r\n");
                    write_to_output(d, "Do you wish to save your changes? : \r\n");
                    return;
            }

        case OEDIT_COPY:
            if ((number = real_object(atoi(arg))) != NOWHERE) {
                oedit_setup_existing(d, number);
            } else
                write_to_output(d, "That object does not exist.\r\n");
            break;

        case OEDIT_DELETE:
            if (*arg == 'y' || *arg == 'Y') {
                if (delete_object(GET_OBJ_RNUM(OLC_OBJ(d))) != NOTHING)
                    write_to_output(d, "Object deleted.\r\n");
                else
                    write_to_output(d, "Couldn't delete the object!\r\n");

                cleanup_olc(d, CLEANUP_ALL);
            } else if (*arg == 'n' || *arg == 'N') {
                oedit_disp_menu(d);
                OLC_MODE(d) = OEDIT_MAIN_MENU;
            } else
                write_to_output(d, "Please answer 'Y' or 'N': ");
            return;

        case OEDIT_MAIN_MENU:
            /*
             * Throw us out to whichever edit mode based on user input.
             */
            switch (*arg) {
                case 'q':
                case 'Q':
                    if (STATE(d) != CON_IEDIT) {
                        if (OLC_VAL(d)) {    /* Something has been modified. */
                            write_to_output(d, "Do you wish to save your changes? : ");
                            OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
                        } else
                            cleanup_olc(d, CLEANUP_ALL);
                    } else {
                        send_to_char(d->character, "\r\nCommitting iedit changes.\r\n");
                        obj = OLC_IOBJ(d);
                        *obj = *(OLC_OBJ(d));
                        ((obj)->id) = nextObjID();
                        /* find_obj helper */
                        if (GET_OBJ_VNUM(obj) != NOTHING) {
                            /* remove any old scripts */
                            if (SCRIPT(obj)) {
                                extract_script(obj, OBJ_TRIGGER);
                                SCRIPT(obj) = nullptr;
                            }

                            free_proto_script(obj, OBJ_TRIGGER);
                            robj = real_object(GET_OBJ_VNUM(obj));
                            copy_proto_script(&obj_proto[robj], obj, OBJ_TRIGGER);
                            assign_triggers(obj, OBJ_TRIGGER);
                        }
                        obj->extra_flags.set(ITEM_UNIQUE_SAVE);
                        /* Xap - ought to save the old pointer, free after assignment I suppose */
                        mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(d->character)), true,
                               "OLC: %s iedit a unique #%d", GET_NAME(d->character), GET_OBJ_VNUM(obj));
                        if (d->character) {
                            d->character->playerFlags.reset(PLR_WRITING);
                            STATE(d) = CON_PLAYING;
                            act("$n stops using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);
                        }
                        free(d->olc);
                        d->olc = nullptr;
                    }
                    return;
                case '1':
                    write_to_output(d, "Enter namelist : ");
                    OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
                    break;
                case '2':
                    write_to_output(d, "Enter short desc : ");
                    OLC_MODE(d) = OEDIT_SHORTDESC;
                    break;
                case '3':
                    write_to_output(d, "Enter long desc :-\r\n| ");
                    OLC_MODE(d) = OEDIT_LONGDESC;
                    break;
                case '4':
                    OLC_MODE(d) = OEDIT_ACTDESC;
                    send_editor_help(d);
                    write_to_output(d, "Enter action description:\r\n\r\n");
                    if (OLC_OBJ(d)->look_description) {
                        write_to_output(d, "%s", OLC_OBJ(d)->look_description);
                        oldtext = strdup(OLC_OBJ(d)->look_description);
                    }
                    string_write(d, &OLC_OBJ(d)->look_description, MAX_MESSAGE_LENGTH, 0, oldtext);
                    OLC_VAL(d) = 1;
                    break;
                case '5':
                    oedit_disp_type_menu(d);
                    OLC_MODE(d) = OEDIT_TYPE;
                    break;
                case '6':
                    oedit_disp_extra_menu(d);
                    OLC_MODE(d) = OEDIT_EXTRAS;
                    break;
                case '7':
                    oedit_disp_wear_menu(d);
                    OLC_MODE(d) = OEDIT_WEAR;
                    break;
                case '8':
                    write_to_output(d, "Enter weight : ");
                    OLC_MODE(d) = OEDIT_WEIGHT;
                    break;
                case '9':
                    write_to_output(d, "Enter cost : ");
                    OLC_MODE(d) = OEDIT_COST;
                    break;
                case 'a':
                case 'A':
                    write_to_output(d, "Enter cost per day : ");
                    OLC_MODE(d) = OEDIT_COSTPERDAY;
                    break;
                case 'b':
                case 'B':
                    write_to_output(d, "Enter timer : ");
                    OLC_MODE(d) = OEDIT_TIMER;
                    break;
                case 'c':
                case 'C':
                    /*
                     * Clear any old values
                     */
                    GET_OBJ_VAL(OLC_OBJ(d), 0) = 0;
                    GET_OBJ_VAL(OLC_OBJ(d), 1) = 0;
                    GET_OBJ_VAL(OLC_OBJ(d), 2) = 0;
                    GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
                    GET_OBJ_VAL(OLC_OBJ(d), 4) = 0;
                    GET_OBJ_VAL(OLC_OBJ(d), 5) = 0;
                    GET_OBJ_VAL(OLC_OBJ(d), 6) = 0;
                    OLC_VAL(d) = 1;
                    oedit_disp_val1_menu(d);
                    break;
                case 'd':
                case 'D':
                    oedit_disp_prompt_apply_menu(d);
                    break;
                case 'e':
                case 'E':
                    /*
                     * If extra descriptions don't exist.
                     */
                    if (OLC_OBJ(d)->ex_description == nullptr) {
                        CREATE(OLC_OBJ(d)->ex_description, struct extra_descr_data, 1);
                        OLC_OBJ(d)->ex_description->next = nullptr;
                    }
                    OLC_DESC(d) = OLC_OBJ(d)->ex_description;
                    oedit_disp_extradesc_menu(d);
                    break;
                case 'm':
                case 'M':
                    write_to_output(d, "Enter new minimum level: ");
                    OLC_MODE(d) = OEDIT_LEVEL;
                    break;
                case 'n':
                case 'N':
                    OLC_MODE(d) = OEDIT_MATERIAL;
                    oedit_disp_material_menu(d);
                    break;
                case 'p':
                case 'P':
                    oedit_disp_perm_menu(d);
                    OLC_MODE(d) = OEDIT_PERM;
                    break;
                case 's':
                case 'S':
                    if (STATE(d) != CON_IEDIT) {
                        OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
                        dg_script_menu(d);
                    } else {
                        write_to_output(d, "\r\nScripts cannot be modified on individual objects.\r\nEnter choice : ");
                    }
                    return;
                case 't':
                case 'T':
                    oedit_disp_prompt_spellbook_menu(d);
                    break;
                case 'w':
                case 'W':
                    write_to_output(d, "Copy what object? ");
                    OLC_MODE(d) = OEDIT_COPY;
                    break;
                case 'x':
                case 'X':
                    write_to_output(d, "Are you sure you want to delete this object? ");
                    OLC_MODE(d) = OEDIT_DELETE;
                    break;
                case 'y':
                case 'Y':
                    oedit_disp_size_menu(d);
                    OLC_MODE(d) = OEDIT_SIZE;
                    break;
                case 'Z':
                case 'z':
                    search_replace(arg, "z ", "");
                    do_wiznet(d->character, arg, 0, 0);
                    break;
                default:
                    oedit_disp_menu(d);
                    break;
            }
            return;            /*
				 * end of OEDIT_MAIN_MENU 
				 */

        case OLC_SCRIPT_EDIT:
            if (dg_script_edit_parse(d, arg)) return;
            break;

        case OEDIT_EDIT_NAMELIST:
            if (!genolc_checkstring(d, arg))
                break;
            if (OLC_OBJ(d)->name)
                free(OLC_OBJ(d)->name);
            OLC_OBJ(d)->name = str_udup(arg);
            break;

        case OEDIT_SHORTDESC:
            if (!genolc_checkstring(d, arg))
                break;
            if (OLC_OBJ(d)->short_description)
                free(OLC_OBJ(d)->short_description);
            OLC_OBJ(d)->short_description = str_udup(arg);
            break;

        case OEDIT_LONGDESC:
            if (!genolc_checkstring(d, arg))
                break;
            if (OLC_OBJ(d)->room_description)
                free(OLC_OBJ(d)->room_description);
            OLC_OBJ(d)->room_description = str_udup(arg);
            break;

        case OEDIT_TYPE:
            number = atoi(arg);
            if ((number < 1) || (number >= NUM_ITEM_TYPES)) {
                write_to_output(d, "Invalid choice, try again : ");
                return;
            } else
                GET_OBJ_TYPE(OLC_OBJ(d)) = number;
            /* what's the boundschecking worth if we don't do this ? -- Welcor */
            GET_OBJ_VAL(OLC_OBJ(d), 0) = GET_OBJ_VAL(OLC_OBJ(d), 1) =
            GET_OBJ_VAL(OLC_OBJ(d), 2) = GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
            break;

        case OEDIT_EXTRAS:
            number = atoi(arg);
            if ((number < 0) || (number > NUM_ITEM_FLAGS)) {
                oedit_disp_extra_menu(d);
                return;
            } else if (number == 0)
                break;
            else {
                OLC_OBJ(d)->extra_flags.flip(number-1);
                oedit_disp_extra_menu(d);
                return;
            }

        case OEDIT_WEAR:
            number = atoi(arg);
            if ((number < 0) || (number > NUM_ITEM_WEARS)) {
                write_to_output(d, "That's not a valid choice!\r\n");
                oedit_disp_wear_menu(d);
                return;
            } else if (number == 0)    /* Quit. */
                break;
            else {
                OLC_OBJ(d)->wear_flags.flip(number-1);
                oedit_disp_wear_menu(d);
                return;
            }

        case OEDIT_WEIGHT:
            GET_OBJ_WEIGHT(OLC_OBJ(d)) = std::clamp(atoll(arg), 0LL, MAX_OBJ_WEIGHT);
            break;

        case OEDIT_COST:
            GET_OBJ_COST(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_COST);
            break;

        case OEDIT_COSTPERDAY:
            GET_OBJ_RENT(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_RENT);
            break;

        case OEDIT_TIMER:
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_PORTAL:
                    GET_OBJ_TIMER(OLC_OBJ(d)) = LIMIT(atoi(arg), -1, MAX_OBJ_TIMER);
                    break;
                default:
                    GET_OBJ_TIMER(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_TIMER);
                    break;
            }
            break;


        case OEDIT_LEVEL:
            GET_OBJ_LEVEL(OLC_OBJ(d)) = MAX(atoi(arg), 0);
            break;

        case OEDIT_MATERIAL:
            GET_OBJ_MATERIAL(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, NUM_MATERIALS);
            break;

        case OEDIT_PERM:
            if ((number = atoi(arg)) == 0)
                break;
            if (number > 0 && number <= NUM_AFF_FLAGS) {
                /* Setting AFF_CHARM on objects like this is dangerous. */
                if (number != AFF_CHARM) {
                    OLC_OBJ(d)->bitvector.flip(number);
                }
            }
            oedit_disp_perm_menu(d);
            return;

        case OEDIT_SIZE:
            number = atoi(arg) - 1;
            GET_OBJ_SIZE(OLC_OBJ(d)) = LIMIT(number, 0, NUM_SIZES - 1);
            break;

        case OEDIT_VALUE_1:
            /*
             * Lucky, I don't need to check any of these for out of range values.
             * Hmm, I'm not so sure - Rv
             */
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_WEAPON:
                    GET_OBJ_VAL(OLC_OBJ(d), 0) = LIMIT(atoi(arg), WEAPON_TYPE_UNARMED, MAX_WEAPON_TYPES);
                    break;
                case ITEM_CONTAINER:
                    GET_OBJ_VAL(OLC_OBJ(d), 0) = std::clamp<int64_t>(atoll(arg), -1, MAX_CONTAINER_SIZE);
                    break;
                default:
                    GET_OBJ_VAL(OLC_OBJ(d), 0) = atoi(arg);
            }
            /*
             * proceed to menu 2
             */
            oedit_disp_val2_menu(d);
            return;
        case OEDIT_VALUE_2:
            /*
             * Here, I do need to check for out of range values.
             */
            number = atoi(arg);
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_SCROLL:
                case ITEM_POTION:
                    if (number == 0 || number == -1)
                        GET_OBJ_VAL(OLC_OBJ(d), 1) = -1;
                    else
                        GET_OBJ_VAL(OLC_OBJ(d), 1) = LIMIT(number, 1, SKILL_TABLE_SIZE);
                    oedit_disp_val3_menu(d);
                    break;
                case ITEM_CONTROL:
                    if (number <= 0)
                        GET_OBJ_VAL(OLC_OBJ(d), 1) = 1;
                    else if (number > 3)
                        GET_OBJ_VAL(OLC_OBJ(d), 1) = 3;
                    else
                        GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
                    oedit_disp_val5_menu(d);
                    break;
                case ITEM_CONTAINER:
                case ITEM_VEHICLE:
                case ITEM_WINDOW:
                case ITEM_HATCH:
                case ITEM_PORTAL:
                    /*
                     * Needs some special handling since we are dealing with flag values
                     * here.
                     */
                    if (number < 0 || number > 4)
                        oedit_disp_container_flags_menu(d);
                    else if (number != 0) {
                        TOGGLE_BIT(GET_OBJ_VAL(OLC_OBJ(d), 1), 1 << (number - 1));
                        OLC_VAL(d) = 1;
                        oedit_disp_val2_menu(d);
                    } else
                        oedit_disp_val3_menu(d);
                    break;
                case ITEM_WEAPON:
                    GET_OBJ_VAL(OLC_OBJ(d), 1) = LIMIT(number, 1, MAX_WEAPON_NDICE);
                    oedit_disp_val3_menu(d);
                    break;

                default:
                    GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
                    oedit_disp_val3_menu(d);
            }
            return;

        case OEDIT_VALUE_3:
            number = atoi(arg);
            /*
             * Quick'n'easy error checking.
             */
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_WEAPON:
                    min_val = 1;
                    max_val = MAX_WEAPON_SDICE;
                    break;
                case ITEM_ARMOR:
                    min_val = 0;
                    max_val = 100;
                case ITEM_WAND:
                case ITEM_STAFF:
                    min_val = 0;
                    max_val = 20;
                    break;
                case ITEM_DRINKCON:
                case ITEM_FOUNTAIN:
                    min_val = 0;
                    max_val = NUM_LIQ_TYPES - 1;
                    break;
                case ITEM_KEY:
                    min_val = 0;
                    max_val = 60000;
                    break;
                default:
                    min_val = -32000;
                    max_val = 60000;
            }
            GET_OBJ_VAL(OLC_OBJ(d), 2) = LIMIT(number, min_val, max_val);
            oedit_disp_val4_menu(d);
            return;

        case OEDIT_VALUE_4:
            number = atoi(arg);
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_HATCH:
                    min_val = 1;
                    max_val = 60000;
                    break;
                case ITEM_WAND:
                case ITEM_STAFF:
                    min_val = 1;
                    max_val = SKILL_TABLE_SIZE - 1;
                    break;
                case ITEM_WEAPON:
                    min_val = 0;
                    max_val = NUM_ATTACK_TYPES - 1;
                    break;
                case ITEM_ARMOR:
                    if (number < 0) /* We're storing armor checks as positive numbers */
                        number = 0 - number;
                    min_val = 0;
                    max_val = 20;
                    break;
                default:
                    min_val = -32000;
                    max_val = 32000;
                    break;
            }
            GET_OBJ_VAL(OLC_OBJ(d), 3) = LIMIT(number, min_val, max_val);
            oedit_disp_val5_menu(d);
            return;

        case OEDIT_VALUE_5:
            min_val = 1;
            max_val = 100;
            GET_OBJ_VAL(OLC_OBJ(d), 4) = LIMIT(atoi(arg), min_val, max_val);
            GET_OBJ_VAL(OLC_OBJ(d), 5) = max_val;
            oedit_disp_val7_menu(d);
            return;

        case OEDIT_VALUE_7:
            number = atoi(arg);
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_WEAPON:
                    min_val = 0;
                    max_val = MAX_CRIT_TYPE;
                    break;
                case ITEM_ARMOR:
                    min_val = -100; /* Want to allow weird armor that improves casting */
                    max_val = 100;
                    break;
                default:
                    min_val = -32000;
                    max_val = 32000;
                    break;
            }
            GET_OBJ_VAL(OLC_OBJ(d), 6) = LIMIT(atoi(arg), min_val, max_val);
            oedit_disp_val9_menu(d);
            return;

        case OEDIT_VALUE_9:
            number = atoi(arg);
            switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
                case ITEM_WEAPON:
                    min_val = 0;
                    max_val = 19;
                    break;
                default:
                    min_val = -32000;
                    max_val = 32000;
                    break;
            }
            GET_OBJ_VAL(OLC_OBJ(d), 8) = LIMIT(atoi(arg), min_val, max_val);
            break;

        case OEDIT_PROMPT_APPLY:
            if ((number = atoi(arg)) == 0)
                break;
            else if (number < 0 || number > MAX_OBJ_AFFECT) {
                oedit_disp_prompt_apply_menu(d);
                return;
            }
            OLC_VAL(d) = number - 1;
            OLC_MODE(d) = OEDIT_APPLY;
            oedit_disp_apply_menu(d);
            return;

        case OEDIT_APPLY:
            if ((number = atoi(arg)) == 0) {
                OLC_OBJ(d)->affected[OLC_VAL(d)].location = 0;
                OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = 0;
                oedit_disp_prompt_apply_menu(d);
            } else if (number < 0 || number >= NUM_APPLIES)
                oedit_disp_apply_menu(d);
            else {
                int counter;

                /* add in check here if already applied.. deny builders another */
                if (GET_ADMLEVEL(d->character) < ADMLVL_GRGOD) {
                    for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
                        if (OLC_OBJ(d)->affected[counter].location == number) {
                            write_to_output(d, "Object already has that apply.");
                            return;
                        }
                    }
                }

                OLC_OBJ(d)->affected[OLC_VAL(d)].location = number;
                write_to_output(d, "Modifier : ");
                OLC_MODE(d) = OEDIT_APPLYMOD;
            }
            return;

        case OEDIT_APPLYMOD:
            OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = atoi(arg);
            oedit_disp_apply_spec_menu(d);
            return;

        case OEDIT_APPLYSPEC:
            if (isdigit(*arg))
                OLC_OBJ(d)->affected[OLC_VAL(d)].specific = atoi(arg);
            else
                switch (OLC_OBJ(d)->affected[OLC_VAL(d)].location) {
                    case APPLY_SKILL:
                        number = find_skill_num(arg, SKTYPE_SKILL);
                        if (number > -1)
                            OLC_OBJ(d)->affected[OLC_VAL(d)].specific = number;
                        break;
                    case APPLY_FEAT:
                        number = find_feat_num(arg);
                        if (number > -1)
                            OLC_OBJ(d)->affected[OLC_VAL(d)].specific = number;
                        break;
                    default:
                        OLC_OBJ(d)->affected[OLC_VAL(d)].specific = 0;
                        break;
                }
            oedit_disp_prompt_apply_menu(d);
            return;

        case OEDIT_EXTRADESC_KEY:
            if (genolc_checkstring(d, arg)) {
                if (OLC_DESC(d)->keyword)
                    free(OLC_DESC(d)->keyword);
                OLC_DESC(d)->keyword = str_udup(arg);
            }
            oedit_disp_extradesc_menu(d);
            return;

        case OEDIT_EXTRADESC_MENU:
            switch ((number = atoi(arg))) {
                case 0:
                    if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
                        struct extra_descr_data *temp;

                        if (OLC_DESC(d)->keyword)
                            free(OLC_DESC(d)->keyword);
                        if (OLC_DESC(d)->description)
                            free(OLC_DESC(d)->description);

                        /*
                         * Clean up pointers
                         */
                        REMOVE_FROM_LIST(OLC_DESC(d), OLC_OBJ(d)->ex_description, next, temp);
                        free(OLC_DESC(d));
                        OLC_DESC(d) = nullptr;
                    }
                    break;

                case 1:
                    OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
                    write_to_output(d, "Enter keywords, separated by spaces :-\r\n| ");
                    return;

                case 2:
                    OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
                    send_editor_help(d);
                    write_to_output(d, "Enter the extra description:\r\n\r\n");
                    if (OLC_DESC(d)->description) {
                        write_to_output(d, "%s", OLC_DESC(d)->description);
                        oldtext = strdup(OLC_DESC(d)->description);
                    }
                    string_write(d, &OLC_DESC(d)->description, MAX_MESSAGE_LENGTH, 0, oldtext);
                    OLC_VAL(d) = 1;
                    return;

                case 3:
                    /*
                     * Only go to the next description if this one is finished.
                     */
                    if (OLC_DESC(d)->keyword && OLC_DESC(d)->description) {
                        struct extra_descr_data *new_extra;

                        if (OLC_DESC(d)->next)
                            OLC_DESC(d) = OLC_DESC(d)->next;
                        else {    /* Make new extra description and attach at end. */
                            CREATE(new_extra, struct extra_descr_data, 1);
                            OLC_DESC(d)->next = new_extra;
                            OLC_DESC(d) = OLC_DESC(d)->next;
                        }
                    }
                    /*
                     * No break - drop into default case.
                     */
                default:
                    oedit_disp_extradesc_menu(d);
                    return;
            }
            break;

        case OEDIT_PROMPT_SPELLBOOK:
            if ((number = atoi(arg)) == 0)
                break;
            else if (number < 0 || number > SKILL_TABLE_SIZE) {
                oedit_disp_prompt_spellbook_menu(d);
                return;
            }
            OLC_VAL(d) = number - 1;
            OLC_MODE(d) = OEDIT_SPELLBOOK;
            oedit_disp_spellbook_menu(d);
            return;

        case OEDIT_SPELLBOOK:
            if ((number = atoi(arg)) == 0) {
                if (OLC_OBJ(d)->sbinfo) {
                    OLC_OBJ(d)->sbinfo[OLC_VAL(d)].spellname = 0;
                    OLC_OBJ(d)->sbinfo[OLC_VAL(d)].pages = 0;
                } else {
                    CREATE(OLC_OBJ(d)->sbinfo, struct obj_spellbook_spell, SPELLBOOK_SIZE);
                    OLC_OBJ(d)->sbinfo[OLC_VAL(d)].spellname = 0;
                    OLC_OBJ(d)->sbinfo[OLC_VAL(d)].pages = 0;
                }
                oedit_disp_prompt_spellbook_menu(d);
            } else if (number < 0 || number >= SKILL_TABLE_SIZE) {
                oedit_disp_spellbook_menu(d);
            } else {
                int counter;

                /* add in check here if already applied.. deny builders another */
                if (GET_LEVEL(d->character) < ADMLVL_IMPL) {
                    for (counter = 0; counter < SKILL_TABLE_SIZE; counter++) {
                        if (OLC_OBJ(d)->sbinfo && OLC_OBJ(d)->sbinfo[counter].spellname == number) {
                            write_to_output(d, "Object already has that spell.");
                            return;
                        }
                    }
                }

                if (!OLC_OBJ(d)->sbinfo) {
                    CREATE(OLC_OBJ(d)->sbinfo, struct obj_spellbook_spell, SPELLBOOK_SIZE);
                }

                OLC_OBJ(d)->sbinfo[OLC_VAL(d)].spellname = number;
                OLC_OBJ(d)->sbinfo[OLC_VAL(d)].pages = MAX(1, spell_info[number].spell_level * 2);
                oedit_disp_prompt_spellbook_menu(d);
            }
            return;

        default:
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: Reached default case in oedit_parse()!");
            write_to_output(d, "Oops...\r\n");
            break;
    }

    /*
     * If we get here, we have changed something.
     */
    OLC_VAL(d) = 1;
    oedit_disp_menu(d);
}

void oedit_string_cleanup(struct descriptor_data *d, int terminator) {
    switch (OLC_MODE(d)) {
        case OEDIT_ACTDESC:
            oedit_disp_menu(d);
            break;
        case OEDIT_EXTRADESC_DESCRIPTION:
            oedit_disp_extradesc_menu(d);
            break;
    }
}

/* this is all iedit stuff */
void iedit_setup_existing(struct descriptor_data *d, struct obj_data *real_num) {
    struct obj_data *obj;

    OLC_IOBJ(d) = real_num;

    obj = create_obj();
    copy_object(obj, real_num);

    /* free any assigned scripts */
    if (SCRIPT(obj))
        extract_script(obj, OBJ_TRIGGER);
    SCRIPT(obj) = nullptr;

    OLC_OBJ(d) = obj;
    OLC_IOBJ(d) = real_num;
    OLC_VAL(d) = 0;
    oedit_disp_menu(d);
}

ACMD(do_iedit) {
    struct obj_data *k;
    int found = 0;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg || !*argument) {
        send_to_char(ch, "You must supply an object name.\r\n");
    }

    if ((k = get_obj_in_equip_vis(ch, arg, nullptr, ch->equipment))) {
        found = 1;
    } else if ((k = get_obj_in_list_vis(ch, arg, nullptr, ch->contents))) {
        found = 1;
    } else if ((k = get_obj_in_list_vis(ch, arg, nullptr, ch->getRoom()->contents))) {
        found = 1;
    } else if ((k = get_obj_vis(ch, arg, nullptr))) {
        found = 1;
    }

    if (!found) {
        send_to_char(ch, "Couldn't find that object. Sorry.\r\n");
        return;
    }

    /* set up here */
    CREATE(OLC(ch->desc), struct oasis_olc_data, 1);
    k->extra_flags.set(ITEM_UNIQUE_SAVE);

    ch->playerFlags.set(PLR_WRITING);
    iedit_setup_existing(ch->desc, k);
    OLC_VAL(ch->desc) = 0;

    act("$n starts using OLC.", true, ch, nullptr, nullptr, TO_ROOM);

    STATE(ch->desc) = CON_IEDIT;

    return;
}


