/************************************************************************
 * OasisOLC - Mobiles / medit.c					v2.0	*
 * Copyright 1996 Harvey Gilpin						*
 * Copyright 1997-2001 George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/medit.h"
#include "dbat/interpreter.h"
#include "dbat/comm.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/genolc.h"
#include "dbat/genmob.h"
#include "dbat/genzon.h"
#include "dbat/genshp.h"
#include "dbat/oasis.h"
#include "dbat/constants.h"
#include "dbat/improved-edit.h"
#include "dbat/dg_olc.h"
#include "dbat/races.h"
#include "dbat/class.h"
#include "dbat/act.wizard.h"
#include "dbat/modify.h"
/*-------------------------------------------------------------------*/

/*
 * External variable declarations.
 */

/*-------------------------------------------------------------------*\
  utility functions 
\*-------------------------------------------------------------------*/

ACMD(do_oasis_medit) {
    int number = NOBODY, save = 0, real_num;
    struct descriptor_data *d;
    char *buf3;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    /****************************************************************************/
    /** Parse any arguments.                                                   **/
    /****************************************************************************/
    buf3 = two_arguments(argument, buf1, buf2);

    if (!*buf1) {
        send_to_char(ch, "Specify a mobile VNUM to edit.\r\n");
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
    /** If a numeric argument was given (like a room number), get it.          **/
    /****************************************************************************/
    if (number == NOBODY)
        number = atoi(buf1);

    /****************************************************************************/
    /** Check that whatever it is isn't already being edited.                  **/
    /****************************************************************************/
    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) == CON_MEDIT) {
            if (d->olc && OLC_NUM(d) == number) {
                send_to_char(ch, "That mobile is currently being edited by %s.\r\n",
                             GET_NAME(d->character));
                return;
            }
        }
    }

    d = ch->desc;

    /****************************************************************************/
    /** Give descriptor an OLC structure.                                      **/
    /****************************************************************************/
    if (d->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true,
               "SYSERR: do_oasis_medit: Player already had olc structure.");
        free(d->olc);
    }

    CREATE(d->olc, struct oasis_olc_data, 1);

    /****************************************************************************/
    /** Find the zone.                                                         **/
    /****************************************************************************/
    OLC_ZNUM(d) = save ? real_zone(number) : real_zone_by_thing(number);
    if (OLC_ZNUM(d) == NOWHERE) {
        send_to_char(ch, "Sorry, there is no zone for that number!\r\n");
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /****************************************************************************/
    /** Everyone but IMPLs can only edit zones they have been assigned.        **/
    /****************************************************************************/
    if (!can_edit_zone(ch, OLC_ZNUM(d))) {
        send_cannot_edit(ch, zone_table[OLC_ZNUM(d)].number);
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    /****************************************************************************/
    /** If save is TRUE, save the mobiles.                                     **/
    /****************************************************************************/
    if (save) {
        send_to_char(ch, "Saving all mobiles in zone %d.\r\n",
                     zone_table[OLC_ZNUM(d)].number);
        mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(ch)), true,
               "OLC: %s saves mobile info for zone %d.",
               GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);

        /**************************************************************************/
        /** Save the mobiles.                                                    **/
        /**************************************************************************/
        save_mobiles(OLC_ZNUM(d));

        /**************************************************************************/
        /** Free the olc structure stored in the descriptor.                     **/
        /**************************************************************************/
        free(d->olc);
        d->olc = nullptr;
        return;
    }

    OLC_NUM(d) = number;

    /****************************************************************************/
    /** If this is a new mobile, setup a new one, otherwise, setup the         **/
    /** existing mobile.                                                       **/
    /****************************************************************************/
    if ((real_num = real_mobile(number)) == NOBODY)
        medit_setup_new(d);
    else
        medit_setup_existing(d, real_num);

    medit_disp_menu(d);
    STATE(d) = CON_MEDIT;

    /****************************************************************************/
    /** Display the OLC messages to the players in the same room as the        **/
    /** builder and also log it.                                               **/
    /****************************************************************************/
    act("$n starts using OLC.", true, d->character, nullptr, nullptr, TO_ROOM);
    ch->playerFlags.set(PLR_WRITING);

    mudlog(BRF, ADMLVL_IMMORT, true, "OLC: %s starts editing zone %d allowed zone %d",
           GET_NAME(ch), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(ch));
}

void medit_save_to_disk(zone_vnum foo) {
    save_mobiles(real_zone(foo));
}

void medit_setup_new(struct descriptor_data *d) {
    struct char_data *mob = new char_data();
    init_mobile(mob);

    GET_MOB_RNUM(mob) = NOBODY;
    /*
     * Set up some default strings.
     */
    GET_ALIAS(mob) = strdup("mob unfinished");
    GET_SDESC(mob) = strdup("the unfinished mob");
    GET_LDESC(mob) = strdup("An unfinished mob stands here.\r\n");
    GET_DDESC(mob) = strdup("It looks unfinished.\r\n");
    mob->race = RaceID::Human;
    SCRIPT(mob) = nullptr;
    mob->proto_script.clear();
    OLC_SCRIPT(d).clear();

    OLC_MOB(d) = mob;
    /* Has changed flag. (It hasn't so far, we just made it.) */
    OLC_VAL(d) = false;
    OLC_ITEM_TYPE(d) = MOB_TRIGGER;
}

/*-------------------------------------------------------------------*/

void medit_setup_existing(struct descriptor_data *d, int rmob_num) {
    struct char_data *mob;

    /*
     * Allocate a scratch mobile structure.
     */
    mob = new char_data();

    copy_mobile(mob, &mob_proto[rmob_num]);

    OLC_MOB(d) = mob;
    OLC_ITEM_TYPE(d) = MOB_TRIGGER;
    dg_olc_script_copy(d);
    /*
     * The edited mob must not have a script.
     * It will be assigned to the updated mob later, after editing.
     */
    SCRIPT(mob) = nullptr;
    OLC_MOB(d)->proto_script.clear();
}

/*-------------------------------------------------------------------*/

/*
 * Ideally, this function should be in db.c, but I'll put it here for
 * portability.
 */
void init_mobile(struct char_data *mob) {

    //GET_HIT(mob) = 0;
    //GET_MAX_MANA(mob) = 0;
    GET_NDD(mob) = 0;
    mob->set(CharAppearance::Sex, SEX_MALE);
    mob->chclass = SenseiID::Commoner;

    GET_WEIGHT(mob) = rand_number(100, 200);
    mob->setHeight(rand_number(100, 200));

    auto base1 = rand_number(8, 16);
    auto base2 = rand_number(8, 16);
    for(auto attr : {CharAttribute::Strength, CharAttribute::Intelligence, CharAttribute::Wisdom}) {
        mob->set(attr, base1);
    }

    for(auto attr : {CharAttribute::Agility, CharAttribute::Constitution, CharAttribute::Speed}) {
        mob->set(attr, base2);
    }

    mob->mobFlags.set(MOB_ISNPC);
}

/*-------------------------------------------------------------------*/

/*
 * Save new/edited mob to memory.
 */
void medit_save_internally(struct descriptor_data *d) {
    int i;
    mob_rnum new_rnum;
    struct descriptor_data *dsc;
    struct char_data *mob;

    i = (real_mobile(OLC_NUM(d)) == NOBODY);

    if ((new_rnum = add_mobile(OLC_MOB(d), OLC_NUM(d))) == NOBODY) {
        basic_mud_log("medit_save_internally: add_mobile failed.");
        return;
    }



    /* Update triggers */
    /* Free old proto list  */
    if (mob_proto[new_rnum].proto_script != OLC_SCRIPT(d))
        free_proto_script(&mob_proto[new_rnum], MOB_TRIGGER);

    mob_proto[new_rnum].proto_script = OLC_SCRIPT(d);

    /* this takes care of the mobs currently in-game */
    for (mob = character_list; mob; mob = mob->next) {
        if (GET_MOB_RNUM(mob) != new_rnum)
            continue;

        /* remove any old scripts */
        if (SCRIPT(mob))
            extract_script(mob, MOB_TRIGGER);

        free_proto_script(mob, MOB_TRIGGER);
        copy_proto_script(&mob_proto[new_rnum], mob, MOB_TRIGGER);
        assign_triggers(mob, MOB_TRIGGER);
    }
    /* end trigger update */

    if (!i)    /* Only renumber on new mobiles. */
        return;

    /*
     * Update keepers in shops being edited and other mobs being edited.
     */
    for (dsc = descriptor_list; dsc; dsc = dsc->next) {
        if (STATE(dsc) == CON_SEDIT)
            S_KEEPER(OLC_SHOP(dsc)) += (S_KEEPER(OLC_SHOP(dsc)) != NOTHING && S_KEEPER(OLC_SHOP(dsc)) >= new_rnum);
        else if (STATE(dsc) == CON_MEDIT)
            GET_MOB_RNUM(OLC_MOB(dsc)) += (GET_MOB_RNUM(OLC_MOB(dsc)) != NOTHING &&
                                           GET_MOB_RNUM(OLC_MOB(dsc)) >= new_rnum);
    }

    /*
     * Update other people in zedit too. From: C.Raehl 4/27/99
     */
    for (dsc = descriptor_list; dsc; dsc = dsc->next)
        if (STATE(dsc) == CON_ZEDIT)
            for (i = 0; OLC_ZONE(dsc)->cmd[i].command != 'S'; i++)
                if (OLC_ZONE(dsc)->cmd[i].command == 'M')
                    if (OLC_ZONE(dsc)->cmd[i].arg1 >= new_rnum)
                        OLC_ZONE(dsc)->cmd[i].arg1++;
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * Display positions. (sitting, standing, etc)
 */
void medit_disp_positions(struct descriptor_data *d) {
    int i;

    clear_screen(d);

    for (i = 0; *position_types[i] != '\n'; i++) {
        write_to_output(d, "@g%2d@n) %s\r\n", i, position_types[i]);
    }
    write_to_output(d, "Enter position number : ");
}

/*-------------------------------------------------------------------*/

/*
 * Display the gender of the mobile.
 */
void medit_disp_sex(struct descriptor_data *d) {
    int i;

    clear_screen(d);

    for (i = 0; i < NUM_GENDERS; i++) {
        write_to_output(d, "@g%2d@n) %s\r\n", i, genders[i]);
    }
    write_to_output(d, "Enter gender number : ");
}

/*-------------------------------------------------------------------*/

/*
 * Display mob-flags menu.
 */
void medit_disp_mob_flags(struct descriptor_data *d) {
    int i, columns = 0;
    char flags[MAX_STRING_LENGTH];

    clear_screen(d);
    for (i = 0; i < NUM_MOB_FLAGS; i++) {
        write_to_output(d, "@g%2d@n) %-20.20s  %s", i + 1, action_bits[i],
                        !(++columns % 2) ? "\r\n" : "");
    }
    sprintbitarray(OLC_MOB(d)->mobFlags, action_bits, AF_ARRAY_MAX, flags);
    write_to_output(d, "\r\nCurrent flags : @c%s@n\r\nEnter mob flags (0 to quit) : ",
                    flags);
}

/*-------------------------------------------------------------------*/

void medit_disp_personality(struct descriptor_data *d) {

    write_to_output(d, "@GPersonalities\n");
    write_to_output(d, "@D--------------@n\n");
    write_to_output(d, "@w1@D) @WBasic@n\n");
    write_to_output(d, "@w1@D) @WCareful@n\n");
    write_to_output(d, "@w1@D) @WAggressive@n\n");
    write_to_output(d, "@w1@D) @WArrogant\n");
    write_to_output(d, "@w1@D) @WIntelligent@n\n");

}

/*
 * Display affection flags menu.
 */
void medit_disp_aff_flags(struct descriptor_data *d) {
    int i, columns = 0;
    char flags[MAX_STRING_LENGTH];

    clear_screen(d);
    for (i = 0; i < NUM_AFF_FLAGS; i++) {
        write_to_output(d, "@g%2d@n) %-20.20s  %s", i + 1, affected_bits[i + 1],
                        !(++columns % 2) ? "\r\n" : "");
    }
    sprintbitarray(AFF_FLAGS(OLC_MOB(d)), affected_bits, AF_ARRAY_MAX, flags);
    write_to_output(d, "\r\nCurrent flags   : @c%s@n\r\nEnter aff flags (0 to quit) : ",
                    flags);
}

/*-------------------------------------------------------------------*/

/*
 * Display class menu.
 */
void medit_disp_class(struct descriptor_data *d) {
    int i;
    char buf[MAX_INPUT_LENGTH];
    clear_screen(d);
    auto check = [](SenseiID id) {return true;};

    for (const auto cl: sensei::filterSenseis(check)) {
        sprintf(buf, "@g%2d@n) %s\r\n", cl, sensei::getName(cl).c_str());
        write_to_output(d, buf);
    }
    write_to_output(d, "Enter class number : ");
}
/*-------------------------------------------------------------------*/
/*
 * Display race menu.
 */
void medit_disp_race(struct descriptor_data *d) {
    int columns = 0;
    char buf[MAX_INPUT_LENGTH];

    clear_screen(d);
    auto check = [](RaceID id) {return true;};
    for (const auto &r: race::filterRaces(check)) {
        sprintf(buf, "@g%2d@n) %-20.20s  %s", r, race::getName(r).c_str(),
                !(++columns % 2) ? "\r\n" : "");
        write_to_output(d, buf);
    }
    write_to_output(d, "Enter race number : ");
}

/*-------------------------------------------------------------------*/
/*
 * Display size menu.
 */
void medit_disp_size(struct descriptor_data *d) {
    int i, columns = 0;
    char buf[MAX_INPUT_LENGTH];

    clear_screen(d);
    for (i = -1; i < NUM_SIZES; i++) {
        sprintf(buf, "@g%2d@n) %-20.20s  %s", i,
                (i == SIZE_UNDEFINED) ? "DEFAULT" : size_names[i],
                !(++columns % 2) ? "\r\n" : "");
        write_to_output(d, buf);
    }
    write_to_output(d, "Enter size number (-1 for default): ");
}

/*-------------------------------------------------------------------*/

/*
 * Display main menu.
 */
void medit_disp_menu(struct descriptor_data *d) {
    struct char_data *mob;
    char flags[MAX_STRING_LENGTH], flag2[MAX_STRING_LENGTH];

    mob = OLC_MOB(d);
    clear_screen(d);

    write_to_output(d,
                    "-- Mob Number:  [@c%d@n]\r\n"
                    "@g1@n) Sex: @y%-7.7s@n	         @g2@n) Alias: @y%s\r\n"
                    "@g3@n) S-Desc: @y%s\r\n"
                    "@g4@n) L-Desc:-\r\n@y%s"
                    "@g5@n) D-Desc:-\r\n@y%s"
                    "@g6@n) Level:       [@c%4d@n],  @g7@n) Alignment:    [@c%5d@n]\r\n"
                    "@g8@n) Accuracy Mod:[@c%4d@n],  @g9@n) Damage Mod:   [@c%5d@n]\r\n"
                    "@gA@n) NumDamDice:  [@c%4d@n],  @gB@n) SizeDamDice:  [@c%5d@n]\r\n"
                    "@gC@n) Num HP Dice: [@c%4" I64T "@n],  @gD@n) Size HP Dice: [@c%5" I64T "@n],  @gE@n) HP Bonus: [@c%5" I64T "@n]\r\n"
                    "@gF@n) Armor Class: [@c%4d@n],  @gG@n) Exp:      [@c%" I64T "@n],  @gH@n) Gold:  [@c%8d@n]\r\n",

                    OLC_NUM(d), genders[(int) GET_SEX(mob)], GET_ALIAS(mob),
                    GET_SDESC(mob), GET_LDESC(mob), GET_DDESC(mob), GET_LEVEL(mob),
                    GET_ALIGNMENT(mob), GET_FISHD(mob), GET_DAMAGE_MOD(mob),
                    GET_NDD(mob), GET_SDD(mob), GET_HIT(mob), (mob->getCurKI()),
                    (mob->getCurST()), GET_ARMOR(mob), GET_EXP(mob), GET_GOLD(mob)
    );
    sprintbitarray(mob->mobFlags, action_bits, AF_ARRAY_MAX, flags);
    sprintbitarray(mob->affected_by, affected_bits, AF_ARRAY_MAX, flag2);
    write_to_output(d,
                    "@gI@n) Position   : @y%-10s@n,	 @gJ@n) Default   : @y%-10s\r\n"
                    "@gK@n) Personality: @Y%s@n\r\n"
                    "@gL@n) NPC Flags  : @c%s\r\n"
                    "@gM@n) AFF Flags  : @c%s\r\n"
                    "@gN@n) Class      : @y%-10s@n,	 @gO@n) Race      : @y%-10s\r\n"
                    "@gS@n) Script     : @c%s\r\n"
                    "@gW@n) Copy mob              ,	 @gX@n) Delete mob\r\n"
                    "@gY@n) Size       : @y%s\r\n"
                    "@gZ@n) Wiznet     :\r\n"
                    "@gQ@n) Quit\r\n"
                    "Enter choice : ",

                    position_types[(int) GET_POS(mob)],
                    position_types[(int) GET_DEFAULT_POS(mob)],
                    npc_personality[GET_PERSONALITY(mob)],
                    flags, flag2, sensei::getName(mob->chclass).c_str(),
                    race::getName(mob->race).c_str(),
                    !OLC_SCRIPT(d).empty() ? "Set." : "Not Set.", size_names[get_size(mob)]
    );

    OLC_MODE(d) = MEDIT_MAIN_MENU;
}

/************************************************************************
 *			The GARGANTAUN event handler			*
 ************************************************************************/

void medit_parse(struct descriptor_data *d, char *arg) {
    int i = -1;
    char *oldtext = nullptr;

    if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE) {
        i = atoi(arg);
        if (!*arg || (!isdigit(arg[0]) && ((*arg == '-') && !isdigit(arg[1])))) {
            write_to_output(d, "Field must be numerical, try again : ");
            return;
        }
    } else {    /* String response. */
        if (!genolc_checkstring(d, arg))
            return;
    }

    switch (OLC_MODE(d)) {
/*-------------------------------------------------------------------*/
        case MEDIT_CONFIRM_SAVESTRING:
            /*
             * Ensure mob has MOB_ISNPC set or things will go pear shaped.
             */
            OLC_MOB(d)->mobFlags.set(MOB_ISNPC);
            switch (*arg) {
                case 'y':
                case 'Y':
                    /*
                     * Save the mob in memory and to disk.
                     */
                    medit_save_internally(d);
                    mudlog(CMP, MAX(ADMLVL_BUILDER, GET_INVIS_LEV(d->character)), true,
                           "OLC: %s edits mob %d", GET_NAME(d->character), OLC_NUM(d));
                    if (CONFIG_OLC_SAVE) {
                        medit_save_to_disk(zone_table[real_zone_by_thing(OLC_NUM(d))].number);
                        write_to_output(d, "Mobile saved to disk.\r\n");
                    } else
                        write_to_output(d, "Mobile saved to memory.\r\n");
                    cleanup_olc(d, CLEANUP_ALL);
                    return;
                case 'n':
                case 'N':
                    /* If not saving, we must free the script_proto list. We do so by
                     * assigning it to the edited mob and letting free_mobile in
                     * cleanup_olc handle it. */
                    OLC_MOB(d)->proto_script = OLC_SCRIPT(d);
                    cleanup_olc(d, CLEANUP_ALL);
                    return;
                default:
                    write_to_output(d, "Invalid choice!\r\n");
                    write_to_output(d, "Do you wish to save the mobile? : ");
                    return;
            }
            break;

/*-------------------------------------------------------------------*/
        case MEDIT_MAIN_MENU:
            i = 0;
            switch (*arg) {
                case 'q':
                case 'Q':
                    if (OLC_VAL(d)) {    /* Anything been changed? */
                        write_to_output(d, "Do you wish to save your changes? : ");
                        OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
                    } else
                        cleanup_olc(d, CLEANUP_ALL);
                    return;
                case '1':
                    OLC_MODE(d) = MEDIT_SEX;
                    medit_disp_sex(d);
                    return;
                case '2':
                    OLC_MODE(d) = MEDIT_ALIAS;
                    i--;
                    break;
                case '3':
                    OLC_MODE(d) = MEDIT_S_DESC;
                    i--;
                    break;
                case '4':
                    OLC_MODE(d) = MEDIT_L_DESC;
                    i--;
                    break;
                case '5':
                    OLC_MODE(d) = MEDIT_D_DESC;
                    send_editor_help(d);
                    write_to_output(d, "Enter mob description:\r\n\r\n");
                    if (OLC_MOB(d)->look_description) {
                        write_to_output(d, "%s", OLC_MOB(d)->look_description);
                        oldtext = strdup(OLC_MOB(d)->look_description);
                    }
                    string_write(d, &OLC_MOB(d)->look_description, MAX_MOB_DESC, 0, oldtext);
                    OLC_VAL(d) = 1;
                    return;
                case '6':
                    OLC_MODE(d) = MEDIT_LEVEL;
                    i++;
                    break;
                case '7':
                    OLC_MODE(d) = MEDIT_ALIGNMENT;
                    i++;
                    break;
                case '8':
                    OLC_MODE(d) = MEDIT_ACCURACY;
                    i++;
                    break;
                case '9':
                    OLC_MODE(d) = MEDIT_DAMAGE;
                    i++;
                    break;
                case 'a':
                case 'A':
                    OLC_MODE(d) = MEDIT_NDD;
                    i++;
                    break;
                case 'b':
                case 'B':
                    OLC_MODE(d) = MEDIT_SDD;
                    i++;
                    break;
                case 'c':
                case 'C':
                    OLC_MODE(d) = MEDIT_NUM_HP_DICE;
                    i++;
                    break;
                case 'd':
                case 'D':
                    OLC_MODE(d) = MEDIT_SIZE_HP_DICE;
                    i++;
                    break;
                case 'e':
                case 'E':
                    OLC_MODE(d) = MEDIT_ADD_HP;
                    i++;
                    break;
                case 'f':
                case 'F':
                    OLC_MODE(d) = MEDIT_AC;
                    i++;
                    break;
                case 'g':
                case 'G':
                    OLC_MODE(d) = MEDIT_EXP;
                    i++;
                    break;
                case 'h':
                case 'H':
                    OLC_MODE(d) = MEDIT_GOLD;
                    i++;
                    break;
                case 'i':
                case 'I':
                    OLC_MODE(d) = MEDIT_POS;
                    medit_disp_positions(d);
                    return;
                case 'j':
                case 'J':
                    OLC_MODE(d) = MEDIT_DEFAULT_POS;
                    medit_disp_positions(d);
                    return;
                case 'k':
                case 'K':
                    OLC_MODE(d) = MEDIT_PERSONALITY;
                    medit_disp_personality(d);
                    return;
                case 'l':
                case 'L':
                    OLC_MODE(d) = MEDIT_NPC_FLAGS;
                    medit_disp_mob_flags(d);
                    return;
                case 'm':
                case 'M':
                    OLC_MODE(d) = MEDIT_AFF_FLAGS;
                    medit_disp_aff_flags(d);
                    return;
                case 'n':
                case 'N':
                    OLC_MODE(d) = MEDIT_CLASS;
                    medit_disp_class(d);
                    return;
                case 'o':
                case 'O':
                    OLC_MODE(d) = MEDIT_RACE;
                    medit_disp_race(d);
                    return;
                case 's':
                case 'S':
                    OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
                    dg_script_menu(d);
                    return;
                case 'w':
                case 'W':
                    write_to_output(d, "Copy what mob? ");
                    OLC_MODE(d) = MEDIT_COPY;
                    return;
                case 'x':
                case 'X':
                    write_to_output(d, "Are you sure you want to delete this mobile? ");
                    OLC_MODE(d) = MEDIT_DELETE;
                    return;
                case 'y':
                case 'Y':
                    OLC_MODE(d) = MEDIT_SIZE;
                    medit_disp_size(d);
                    return;
                case 'Z':
                case 'z':
                    search_replace(arg, "z ", "");
                    do_wiznet(d->character, arg, 0, 0);
                    break;
                default:
                    medit_disp_menu(d);
                    return;
            }
            if (i == 0)
                break;
            else if (i == 1)
                write_to_output(d, "\r\nEnter new value : ");
            else if (i == -1)
                write_to_output(d, "\r\nEnter new text :\r\n] ");
            else
                write_to_output(d, "Oops...\r\n");
            return;
/*-------------------------------------------------------------------*/
        case OLC_SCRIPT_EDIT:
            if (dg_script_edit_parse(d, arg)) return;
            break;
/*-------------------------------------------------------------------*/
        case MEDIT_ALIAS:
            smash_tilde(arg);
            if (GET_ALIAS(OLC_MOB(d)))
                free(GET_ALIAS(OLC_MOB(d)));
            GET_ALIAS(OLC_MOB(d)) = str_udup(arg);
            break;
/*-------------------------------------------------------------------*/
        case MEDIT_S_DESC:
            smash_tilde(arg);
            if (GET_SDESC(OLC_MOB(d)))
                free(GET_SDESC(OLC_MOB(d)));
            GET_SDESC(OLC_MOB(d)) = str_udup(arg);
            break;
/*-------------------------------------------------------------------*/
        case MEDIT_L_DESC:
            smash_tilde(arg);
            if (GET_LDESC(OLC_MOB(d)))
                free(GET_LDESC(OLC_MOB(d)));
            if (arg && *arg) {
                char buf[MAX_INPUT_LENGTH];
                snprintf(buf, sizeof(buf), "%s\r\n", arg);
                GET_LDESC(OLC_MOB(d)) = strdup(buf);
            } else
                GET_LDESC(OLC_MOB(d)) = strdup("undefined");

            break;
/*-------------------------------------------------------------------*/
        case MEDIT_D_DESC:
            /*
             * We should never get here.
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: medit_parse(): Reached D_DESC case!");
            write_to_output(d, "Oops...\r\n");
            break;
/*-------------------------------------------------------------------*/
        case MEDIT_NPC_FLAGS:
            if ((i = atoi(arg)) <= 0)
                break;
            else if (i <= NUM_MOB_FLAGS)
                OLC_MOB(d)->mobFlags.flip(i-1);
            medit_disp_mob_flags(d);
            return;
/*-------------------------------------------------------------------*/
        case MEDIT_PERSONALITY:
            if ((i = atoi(arg)) <= 0)
                break;
            else if (i <= MAX_PERSONALITIES)
                GET_PERSONALITY(OLC_MOB(d)) = i;
            medit_disp_personality(d);
            return;
/*-------------------------------------------------------------------*/
        case MEDIT_AFF_FLAGS:
            if ((i = atoi(arg)) <= 0)
                break;
            else if (i <= NUM_AFF_FLAGS)
                OLC_MOB(d)->affected_by.flip(i-1);
            /* Remove unwanted bits right away. */
            for(auto f : {AFF_CHARM, AFF_POISON, AFF_GROUP, AFF_SLEEP}) OLC_MOB(d)->affected_by.reset(f);
            medit_disp_aff_flags(d);
            return;
/*-------------------------------------------------------------------*/

/*
 * Numerical responses.
 */

        case MEDIT_SEX:
            OLC_MOB(d)->set(CharAppearance::Sex, LIMIT(i, 0, NUM_GENDERS - 1));
            break;

        case MEDIT_ACCURACY:
            GET_FISHD(OLC_MOB(d)) = LIMIT(i, 0, 50);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_DAMAGE:
            GET_DAMAGE_MOD(OLC_MOB(d)) = LIMIT(i, 0, 50);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_NDD:
            GET_NDD(OLC_MOB(d)) = LIMIT(i, 0, 30);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_SDD:
            GET_SDD(OLC_MOB(d)) = LIMIT(i, 0, 127);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_NUM_HP_DICE:
            //GET_HIT(OLC_MOB(d)) = LIMIT(i, 0, CONFIG_LEVEL_CAP);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_SIZE_HP_DICE:
            //GET_MANA(OLC_MOB(d)) = LIMIT(i, 0, 1000);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_ADD_HP:
            //GET_MOVE(OLC_MOB(d)) = LIMIT(i, 0, 30000);
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_AC:
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_EXP:
            OLC_MOB(d)->setExperience(LIMIT(i, 0, MAX_MOB_EXP));
            OLC_MOB(d)->mobFlags.reset(MOB_AUTOBALANCE);
            break;

        case MEDIT_GOLD:
            OLC_MOB(d)->set(CharMoney::Carried, i);
            break;

        case MEDIT_POS:
            GET_POS(OLC_MOB(d)) = LIMIT(i, 0, NUM_POSITIONS - 1);
            break;

        case MEDIT_DEFAULT_POS:
            GET_DEFAULT_POS(OLC_MOB(d)) = LIMIT(i, 0, NUM_POSITIONS - 1);
            break;

        case MEDIT_ATTACK:
            GET_ATTACK(OLC_MOB(d)) = LIMIT(i, 0, NUM_ATTACK_TYPES - 1);
            break;

        case MEDIT_LEVEL:
            OLC_MOB(d)->set(CharNum::Level, i);
            /* Try to add some baseline defaults based on level choice. */
            break;

        case MEDIT_ALIGNMENT:
            OLC_MOB(d)->set(CharAlign::GoodEvil, LIMIT(i, -1000, 1000));
            break;

        case MEDIT_CLASS: {
            auto sensei = static_cast<SenseiID>(i);
            //Check if the sensei requested exists
            if (sensei::exists(sensei)) {
                //Set the mob's Sensei to the chosen sensei
                OLC_MOB(d)->chclass = sensei;
            }
            else {
                write_to_output(d, "Couldn't find the requested Sensei!\r\n");
            }
        }
            break;

        case MEDIT_COPY:
            if ((i = real_mobile(atoi(arg))) != NOWHERE) {
                medit_setup_existing(d, i);
            } else
                write_to_output(d, "That mob does not exist.\r\n");
            break;

        case MEDIT_DELETE:
            if (*arg == 'y' || *arg == 'Y') {
                if (delete_mobile(GET_MOB_RNUM(OLC_MOB(d))) != NOBODY)
                    write_to_output(d, "Mobile deleted.\r\n");
                else
                    write_to_output(d, "Couldn't delete the mobile!\r\n");

                cleanup_olc(d, CLEANUP_ALL);
                return;
            } else if (*arg == 'n' || *arg == 'N') {
                medit_disp_menu(d);
                OLC_MODE(d) = MEDIT_MAIN_MENU;
                return;
            } else
                write_to_output(d, "Please answer 'Y' or 'N': ");
            break;

        case MEDIT_RACE: {
            auto chosen_race = static_cast<RaceID>(i);
            if (!race::exists(chosen_race)) {
                write_to_output(d, "That's not a race!");
                break;
            }
            OLC_MOB(d)->race = chosen_race;
            /*  Change racial size based on race choice. */
            OLC_MOB(d)->setSize(race::getSize(OLC_MOB(d)->race));
        }
            break;

        case MEDIT_SIZE:
            OLC_MOB(d)->setSize(LIMIT(i, -1, NUM_SIZES - 1));
            break;

/*-------------------------------------------------------------------*/
        default:
            /*
             * We should never get here.
             */
            cleanup_olc(d, CLEANUP_ALL);
            mudlog(BRF, ADMLVL_BUILDER, true, "SYSERR: OLC: medit_parse(): Reached default case!");
            write_to_output(d, "Oops...\r\n");
            break;
    }
/*-------------------------------------------------------------------*/

/*
 * END OF CASE 
 * If we get here, we have probably changed something, and now want to
 * return to main menu.  Use OLC_VAL as a 'has changed' flag  
 */

    OLC_VAL(d) = true;
    medit_disp_menu(d);
}

void medit_string_cleanup(struct descriptor_data *d, int terminator) {
    switch (OLC_MODE(d)) {

        case MEDIT_D_DESC:
        default:
            medit_disp_menu(d);
            break;
    }
}

