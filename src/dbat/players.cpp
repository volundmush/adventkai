/* ************************************************************************
*   File: players.c                                     Part of CircleMUD *
*  Usage: Player loading/saving and utility routines                      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/players.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/handler.h"
#include "dbat/pfdefaults.h"
#include "dbat/dg_scripts.h"
#include "dbat/class.h"
#include "dbat/ban.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#define LOAD_HIT    0
#define LOAD_MANA    1
#define LOAD_MOVE    2
#define LOAD_KI        3
#define LOAD_LIFE       4

DebugMap<int64_t, player_data> players;

/* local functions */

int sprintascii(char *out, bitvector_t bits);

void tag_argument(char *argument, char *tag);

void load_affects(FILE *fl, struct char_data *ch, int violence);

void load_bonuses(FILE *fl, struct char_data *ch, bool mods);

void load_skills(FILE *fl, struct char_data *ch, bool mods);

void load_HMVS(struct char_data *ch, const char *line, int mode);

void load_BASE(struct char_data *ch, const char *line, int mode);

void load_molt(struct char_data *ch, const char *line);

void load_majin(struct char_data *ch, const char *line);


/* 'global' vars */
struct player_index_element *player_table = nullptr;    /* index to plr file	 */
int top_of_p_table = 0;        /* ref to top of table		 */
int top_of_p_file = 0;        /* ref of size of p file	 */
long top_idnum = 0;        /* highest idnum in use		 */


/* external ASCII Player Files vars */

/* ASCII Player Files - set this TRUE if you want poofin/poofout
   strings saved in the pfiles
 */
#define ASCII_SAVE_POOFS  false


/*************************************************************************
*  stuff related to the player index					 *
*************************************************************************/

/* new version to build player index for ASCII Player Files */
/* generate index table for the player file */
void build_player_index() {
    int rec_count = 0, i;
    FILE *plr_index;
    char index_name[40], line[256], bits[64];
    char arg2[80];

    sprintf(index_name, "%s%s", LIB_PLRFILES, INDEX_FILE);
    if (!(plr_index = fopen(index_name, "r"))) {
        top_of_p_table = -1;
        basic_mud_log("No player index file!  First new char will be IMP!");
        return;
    }

    while (get_line(plr_index, line))
        if (*line != '~')
            rec_count++;
    rewind(plr_index);

    if (rec_count == 0) {
        player_table = nullptr;
        top_of_p_table = -1;
        return;
    }

    CREATE(player_table, struct player_index_element, rec_count);
    for (i = 0; i < rec_count; i++) {
        get_line(plr_index, line);
        player_table[i].admlevel = ADMLVL_NONE; /* In case they're not in the index yet */
        sscanf(line, "%ld %s %d %s %ld %d %d %d %ld", &player_table[i].id, arg2,
               &player_table[i].level, bits, &player_table[i].last, &player_table[i].admlevel, &player_table[i].ship,
               &player_table[i].shiproom, &player_table[i].played);
        CREATE(player_table[i].name, char, strlen(arg2) + 1);
        strcpy(player_table[i].name, arg2);
        player_table[i].flags = asciiflag_conv(bits);
        top_idnum = MAX(top_idnum, player_table[i].id);
    }

    fclose(plr_index);
    top_of_p_file = top_of_p_table = i - 1;
}


void free_player_index() {
    int tp;

    if (!player_table)
        return;

    for (tp = 0; tp <= top_of_p_table; tp++) {
        if (player_table[tp].name)
            free(player_table[tp].name);
    }

    free(player_table);
    player_table = nullptr;
    top_of_p_table = 0;
}


long get_ptable_by_name(const char *name) {
    int i;

    for (i = 0; i <= top_of_p_table; i++)
        if (!strcasecmp(player_table[i].name, name))
            return (i);

    return (-1);
}


long get_id_by_name(const char *name) {
    auto find = findPlayer(name);
    if(!find) return -1;
    return find->id;
}


char *get_name_by_id(long id) {
    static char buf[128];
    auto find = players.find(id);
    if(find == players.end()) return nullptr;
    sprintf(buf, "%s", find->second.name.c_str());
    return buf;
}


void load_follower_from_file(FILE *fl, struct char_data *ch) {
    int nr;
    char line[MAX_INPUT_LENGTH + 1];
    struct char_data *newch;

    if (!get_line(fl, line))
        return;

    if (line[0] != '#' || !line[1])
        return;

    nr = atoi(line + 1);
    newch = create_char();
    newch->vn = real_mobile(nr);

    if (!parse_mobile_from_file(fl, newch)) {
        free(newch);
    } else {
        add_follower(newch, ch);
        newch->master_id = GET_IDNUM(ch);
        GET_POS(newch) = POS_STANDING;
    }
}


/*************************************************************************
*  stuff related to the save/load player system				 *
*************************************************************************/


#define NUM_OF_SAVE_THROWS    3

/* new load_char reads ASCII Player Files */
/* Load a char, TRUE if loaded, FALSE if not */
int load_char(const char *name, struct char_data *ch) {
    int id = 0, i, num = 0, num2 = 0, num3 = 0;
    FILE *fl = nullptr;
    char fname[READ_SIZE];
    char buf[128], buf2[128], line[MAX_INPUT_LENGTH], tag[6];
    char f1[128], f2[128], f3[128], f4[128];

    if ((id = get_ptable_by_name(name)) < 0)
        return (-1);
    else {
        if (!get_filename(fname, sizeof(fname), PLR_FILE, player_table[id].name))
            return (-1);
        if (!(fl = fopen(fname, "r"))) {
            mudlog(NRM, ADMLVL_GOD, true, "SYSERR: Couldn't open player file %s", fname);
            return (-1);
        }

        /* character initializations */
        /* initializations necessary to keep some things straight */
        ch->affected = nullptr;
        ch->affectedv = nullptr;
        for (i = 1; i <= SKILL_TABLE_SIZE; i++) {
            SET_SKILL(ch, i, 0);
            SET_SKILL_BONUS(ch, i, 0);
            SET_SKILL_PERF(ch, i, 0);
        }

        GET_LOG_USER(ch) = strdup("NOUSER");
        GET_SUPPRESS(ch) = PFDEF_SKIN;
        GET_FURY(ch) = PFDEF_HAIRL;
        GET_CLAN(ch) = strdup("None.");
        GET_HOME(ch) = PFDEF_HOMETOWN;
        GET_WEIGHT(ch) = PFDEF_WEIGHT;

        ch->health = 1.0;
        GET_RELAXCOUNT(ch) = PFDEF_EYE;
        GET_BLESSLVL(ch) = PFDEF_HEIGHT;
        ch->life = 1.0;
        // GET_LIFEFORCE(ch) = PFDEF_BASEPL;
        GET_LIFEPERC(ch) = PFDEF_WEIGHT;
        GET_STUPIDKISS(ch) = 0;
        GET_POS(ch) = POS_STANDING;
        GET_MAJINIZED(ch) = PFDEF_BASEPL;
        GET_GAUNTLET(ch) = PFDEF_GAUNTLET;
        ch->energy = 1.0;
        ch->stamina = 1.0;


        GET_LOADROOM(ch) = PFDEF_LOADROOM;
        GET_INVIS_LEV(ch) = PFDEF_INVISLEV;
        GET_FREEZE_LEV(ch) = PFDEF_FREEZELEV;
        GET_WIMP_LEV(ch) = PFDEF_WIMPLEV;

        GET_OLC_ZONE(ch) = PFDEF_OLC;

        ch->time.birth = ch->time.created = ch->time.maxage = 0;
        ch->followers = nullptr;

        auto &p = players[player_table[id].id];

        while (get_line(fl, line)) {
            bitvector_t flags[4];
            tag_argument(line, tag);

            switch (*tag) {
                case 'A':
                    if (!strcmp(tag, "Ac  ")) ;
                    else if (!strcmp(tag, "Act ")) {
                        sscanf(line, "%s %s %s %s", f1, f2, f3, f4);
                        flags[0] = asciiflag_conv(f1);
                        flags[1] = asciiflag_conv(f2);
                        flags[2] = asciiflag_conv(f3);
                        flags[3] = asciiflag_conv(f4);
                        for(auto f = 0; f < ch->playerFlags.size(); f++) {
                            if(IS_SET_AR(flags, f)) ch->playerFlags.set(f);
                        }
                    } else if (!strcmp(tag, "Aff ")) {
                        sscanf(line, "%s %s %s %s", f1, f2, f3, f4);
                        flags[0] = asciiflag_conv(f1);
                        flags[1] = asciiflag_conv(f2);
                        flags[2] = asciiflag_conv(f3);
                        flags[3] = asciiflag_conv(f4);
                        for(auto f = 0; f < ch->affected_by.size(); f++) {
                            if(IS_SET_AR(flags, f)) ch->affected_by.set(f);
                        }
                    } else if (!strcmp(tag, "Affs")) load_affects(fl, ch, 0);
                    else if (!strcmp(tag, "Affv")) load_affects(fl, ch, 1);
                    else if (!strcmp(tag, "AdmL")) ch->set(CharNum::AdmLevel, atoi(line));
                    else if (!strcmp(tag, "Abso")) GET_ABSORBS(ch) = atoi(line);
                    else if (!strcmp(tag, "AdmF")) {
                        sscanf(line, "%s %s %s %s", f1, f2, f3, f4);
                        flags[0] = asciiflag_conv(f1);
                        flags[1] = asciiflag_conv(f2);
                        flags[2] = asciiflag_conv(f3);
                        flags[3] = asciiflag_conv(f4);
                        for(auto f = 0; f < ch->admflags.size(); f++) {
                            if(IS_SET_AR(flags, f)) ch->admflags.set(f);
                        }
                    } else if (!strcmp(tag, "Alin")) ch->set(CharAlign::GoodEvil, atoi(line));
                    else if (!strcmp(tag, "Aura")) ch->set(CharAppearance::Aura, atoi(line));
                    break;

                case 'B':
                    if (!strcmp(tag, "Bank")) ch->set(CharMoney::Bank, atoi(line));
                    else if (!strcmp(tag, "Bki ")) load_BASE(ch, line, LOAD_MANA);
                    else if (!strcmp(tag, "Blss")) GET_BLESSLVL(ch) = atoi(line);
                    else if (!strcmp(tag, "Boam")) GET_BOARD(ch, 0) = atoi(line);
                    else if (!strcmp(tag, "Boai")) GET_BOARD(ch, 1) = atoi(line);
                    else if (!strcmp(tag, "Boac")) GET_BOARD(ch, 2) = atoi(line);
                    else if (!strcmp(tag, "Boad")) GET_BOARD(ch, 3) = atoi(line);
                    else if (!strcmp(tag, "Boab")) GET_BOARD(ch, 4) = atoi(line);
                    else if (!strcmp(tag, "Bonu")) load_bonuses(fl, ch, false);
                    else if (!strcmp(tag, "Boos")) GET_BOOSTS(ch) = atoi(line);
                    else if (!strcmp(tag, "Bpl ")) load_BASE(ch, line, LOAD_HIT);
                    else if (!strcmp(tag, "Brth")) ch->time.birth = atol(line);
                    else if (!strcmp(tag, "Bst ")) load_BASE(ch, line, LOAD_MOVE);
                    break;

                case 'C':
                    if (!strcmp(tag, "Cha ")) ch->set(CharAttribute::Speed, atoi(line));
                    else if (!strcmp(tag, "Clan")) GET_CLAN(ch) = strdup(line);
                    else if (!strcmp(tag, "Clar")) GET_CRANK(ch) = atoi(line);
                    else if (!strcmp(tag, "Clas"))
                        ch->chclass = static_cast<SenseiID>(std::min(14, atoi(line)));
                    else if (!strcmp(tag, "Colr")) {
                        sscanf(line, "%d %s", &num, buf2);
                        p.color_choices[num] = strdup(buf2);
                    } else if (!strcmp(tag, "Con ")) ch->set(CharAttribute::Constitution, atoi(line));
                    else if (!strcmp(tag, "Cool")) GET_COOLDOWN(ch) = atoi(line);
                    else if (!strcmp(tag, "Crtd")) ch->time.created = atol(line);
                    break;

                case 'D':
                    if (!strcmp(tag, "Deat")) GET_DTIME(ch) = atoi(line);
                    else if (!strcmp(tag, "Deac")) GET_DCOUNT(ch) = atoi(line);
                    else if (!strcmp(tag, "Desc")) ch->look_description = fread_string(fl, buf2);
                    else if (!strcmp(tag, "Dex ")) ch->set(CharAttribute::Agility, atoi(line));
                    else if (!strcmp(tag, "Drnk")) GET_COND(ch, DRUNK) = atoi(line);
                    else if (!strcmp(tag, "Damg")) GET_DAMAGE_MOD(ch) = atoi(line);
                    else if (!strcmp(tag, "Droo")) GET_DROOM(ch) = atoi(line);
                    break;

                case 'E':
                    if (!strcmp(tag, "Exp ")) ch->exp = atoi(line);
                    else if (!strcmp(tag, "Eali")) ch->set(CharAlign::LawChaos, atoi(line));
                    else if (!strcmp(tag, "Ecls")) {

                    } else if (!strcmp(tag, "Eye ")) ch->set(CharAppearance::EyeColor, atoi(line));
                    break;

                case 'F':
                    if (!strcmp(tag, "Fisd")) GET_FISHD(ch) = atoi(line);
                    else if (!strcmp(tag, "Frez")) GET_FREEZE_LEV(ch) = atoi(line);
                    else if (!strcmp(tag, "Forc")) GET_FORGET_COUNT(ch) = atoi(line);
                    else if (!strcmp(tag, "Forg")) GET_FORGETING(ch) = atoi(line);
                    else if (!strcmp(tag, "Fury")) GET_FURY(ch) = atoi(line);
                    break;

                case 'G':
                    if (!strcmp(tag, "Gold")) ch->set(CharMoney::Carried, atoi(line));
                    else if (!strcmp(tag, "Gaun")) GET_GAUNTLET(ch) = atoi(line);
                    else if (!strcmp(tag, "Geno")) GET_GENOME(ch, 0) = atoi(line);
                    else if (!strcmp(tag, "Gen1")) GET_GENOME(ch, 1) = atoi(line);
                    break;

                case 'H':
                    if (!strcmp(tag, "Hit ")) load_HMVS(ch, line, LOAD_HIT);
                    else if (!strcmp(tag, "Hite")) ch->setHeight(atoi(line));
                    else if (!strcmp(tag, "Home")) GET_HOME(ch) = atoi(line);
                    else if (!strcmp(tag, "Host")) {}
                    else if (!strcmp(tag, "Hrc ")) ch->set(CharAppearance::HairColor, atoi(line));
                    else if (!strcmp(tag, "Hrl ")) ch->set(CharAppearance::HairLength, atoi(line));
                    else if (!strcmp(tag, "Hrs ")) ch->set(CharAppearance::HairStyle, atoi(line));
                    else if (!strcmp(tag, "Hung")) GET_COND(ch, HUNGER) = atoi(line);
                    break;

                case 'I':
                    if (!strcmp(tag, "Id  ")) GET_IDNUM(ch) = atol(line);
                    else if (!strcmp(tag, "INGl")) GET_INGESTLEARNED(ch) = atoi(line);
                    else if (!strcmp(tag, "Int ")) ch->set(CharAttribute::Intelligence, atoi(line));
                    else if (!strcmp(tag, "Invs")) GET_INVIS_LEV(ch) = atoi(line);
                    break;

                case 'K':
                    if (!strcmp(tag, "Ki  ")) load_HMVS(ch, line, LOAD_KI);
                    else if (!strcmp(tag, "Kaio")) GET_KAIOKEN(ch) = atoi(line);
                    break;

                case 'L':
                    if (!strcmp(tag, "Last")) ch->time.logon = atol(line);
                    else if (!strcmp(tag, "Lern")) ch->modPractices(atoi(line));
                    else if (!strcmp(tag, "Levl")) ch->set(CharNum::Level, atoi(line));
                    else if (!strcmp(tag, "LF  ")) load_BASE(ch, line, LOAD_LIFE);
                    else if (!strcmp(tag, "LFPC")) GET_LIFEPERC(ch) = atoi(line);
                    else if (!strcmp(tag, "Lila")) GET_LIMBCOND(ch, 1) = atoi(line);
                    else if (!strcmp(tag, "Lill")) GET_LIMBCOND(ch, 3) = atoi(line);
                    else if (!strcmp(tag, "Lira")) GET_LIMBCOND(ch, 0) = atoi(line);
                    else if (!strcmp(tag, "Lirl")) GET_LIMBCOND(ch, 2) = atoi(line);
                    else if (!strcmp(tag, "Lint")) GET_LINTEREST(ch) = atoi(line);
                    else if (!strcmp(tag, "Lpla")) GET_LPLAY(ch) = atoi(line);

                    break;

                case 'M':
                    if (!strcmp(tag, "Mana")) load_HMVS(ch, line, LOAD_MANA);
                    else if (!strcmp(tag, "Mexp")) load_molt(ch, line);
                    else if (!strcmp(tag, "Mlvl")) GET_MOLT_LEVEL(ch) = atoi(line);
                    else if (!strcmp(tag, "Move")) load_HMVS(ch, line, LOAD_MOVE);
                    else if (!strcmp(tag, "Mcls")) {

                    } else if (!strcmp(tag, "Maji")) MAJINIZED(ch) = atoi(line);
                    else if (!strcmp(tag, "Majm")) load_majin(ch, line);
                    else if (!strcmp(tag, "Mimi"))
                        ch->mimic = (RaceID)atoi(line);
                    else if (!strcmp(tag, "MxAg")) ch->time.maxage = atol(line);
                    break;

                case 'N':
                    if (!strcmp(tag, "Name")) GET_PC_NAME(ch) = strdup(line);
                    break;

                case 'O':
                    if (!strcmp(tag, "Olc ")) GET_OLC_ZONE(ch) = atoi(line);
                    break;

                case 'P':
                    if (!strcmp(tag, "Phas")) ch->set(CharAppearance::DistinguishingFeature, atoi(line));
                    else if (!strcmp(tag, "Phse")) GET_PHASE(ch) = atoi(line);
                    else if (!strcmp(tag, "Plyd")) ch->time.played = atol(line);
#ifdef ASCII_SAVE_POOFS
                    else if (!strcmp(tag, "PfIn")) POOFIN(ch) = strdup(line);
                    else if (!strcmp(tag, "PfOt")) POOFOUT(ch) = strdup(line);
#endif
                    else if (!strcmp(tag, "Pole")) GET_POLE_BONUS(ch) = atoi(line);
                    else if (!strcmp(tag, "Posi")) GET_POS(ch) = atoi(line);
                    else if (!strcmp(tag, "Pref")) {
                        sscanf(line, "%s %s %s %s", f1, f2, f3, f4);
                        flags[0] = asciiflag_conv(f1);
                        flags[1] = asciiflag_conv(f2);
                        flags[2] = asciiflag_conv(f3);
                        flags[3] = asciiflag_conv(f4);
                        for(auto f = 0; f < ch->pref.size(); f++) {
                            if(IS_SET_AR(flags, f)) ch->pref.set(f);
                        }
                    } else if (!strcmp(tag, "Prff")) GET_PREFERENCE(ch) = atoi(line);
                    break;

                case 'R':
                    if (!strcmp(tag, "Race")) ch->race = (RaceID)atoi(line);
                    else if (!strcmp(tag, "Raci")) ch->set(CharNum::RacialPref, atoi(line));
                    else if (!strcmp(tag, "rDis")) GET_RDISPLAY(ch) = strdup(line);
                    else if (!strcmp(tag, "Rela")) GET_RELAXCOUNT(ch) = atoi(line);
                    else if (!strcmp(tag, "Rtim")) GET_RTIME(ch) = atoi(line);
                    else if (!strcmp(tag, "Rad1")) GET_RADAR1(ch) = atoi(line);
                    else if (!strcmp(tag, "Rad2")) GET_RADAR2(ch) = atoi(line);
                    else if (!strcmp(tag, "Rad3")) GET_RADAR3(ch) = atoi(line);
                    else if (!strcmp(tag, "Room")) GET_LOADROOM(ch) = atoi(line);
                    else if (!strcmp(tag, "RPfe")) GET_FEATURE(ch) = strdup(line);
                    break;

                case 'S':
                    if (!strcmp(tag, "Sex ")) ch->set(CharAppearance::Sex, atoi(line));
                    else if (!strcmp(tag, "Ship")) GET_SHIP(ch) = atoi(line);
                    else if (!strcmp(tag, "Scoo")) GET_SDCOOLDOWN(ch) = atoi(line);
                    else if (!strcmp(tag, "Shpr")) GET_SHIPROOM(ch) = atoi(line);
                    else if (!strcmp(tag, "Skil")) load_skills(fl, ch, false);
                    else if (!strcmp(tag, "Skn ")) ch->set(CharAppearance::SkinColor, atoi(line));
                    else if (!strcmp(tag, "Size")) ch->setSize(atoi(line));
                    else if (!strcmp(tag, "SklB")) load_skills(fl, ch, true);
                    else if (!strcmp(tag, "SkRc")) ch->modPractices(atoi(line));
                    else if (!strcmp(tag, "SkCl")) {
                        sscanf(line, "%d %d", &num2, &num3);
                        ch->modPractices(num3);
                    } else if (!strcmp(tag, "Slot")) ch->skill_slots = atoi(line);
                    else if (!strcmp(tag, "Spek")) SPEAKING(ch) = atoi(line);
                    else if (!strcmp(tag, "Str ")) ch->set(CharAttribute::Strength, atoi(line));
                    else if (!strcmp(tag, "Stuk")) ch->stupidkiss = atoi(line);
                    else if (!strcmp(tag, "Supp")) GET_SUPPRESS(ch) = atoi(line);
                    break;

                case 'T':
                    if (!strcmp(tag, "Tgro")) GET_TGROWTH(ch) = atoi(line);
                    else if (!strcmp(tag, "Tcla")) {
                        switch(atoi(line)) {
                            case 1: // great requirements... range is 0.2 to 0.3.
                                ch->transBonus = Random::get<double>(0.2, 0.3);
                            break;
                            case 2:
                                ch->transBonus = Random::get<double>(-0.1, 0.1);
                            break;
                            case 3:
                                ch->transBonus = Random::get<double>(-0.3, -0.2);
                            break;
                            default:
                                ch->transBonus = Random::get<double>(-0.3, 0.3);
                        }

                    }
                    else if (!strcmp(tag, "Tcos")) {
                        sscanf(line, "%d %d", &num2, &num3);
                        //GET_TRANSCOST(ch, num2) = num3;
                    } else if (!strcmp(tag, "Thir")) GET_COND(ch, THIRST) = atoi(line);
                    else if (!strcmp(tag, "Thr1")) GET_SAVE_MOD(ch, 0) = atoi(line);
                    else if (!strcmp(tag, "Thr2")) GET_SAVE_MOD(ch, 1) = atoi(line);
                    else if (!strcmp(tag, "Thr3")) GET_SAVE_MOD(ch, 2) = atoi(line);
                    else if (!strcmp(tag, "Thr4") || !strcmp(tag, "Thr5")); /* Discard extra saves */
                    else if (!strcmp(tag, "ThB1")) GET_SAVE_BASE(ch, 0) = atoi(line);
                    else if (!strcmp(tag, "ThB2")) GET_SAVE_BASE(ch, 1) = atoi(line);
                    else if (!strcmp(tag, "ThB3")) GET_SAVE_BASE(ch, 2) = atoi(line);
                    else if (!strcmp(tag, "Trag")) ch->set(CharTrain::Agility, atoi(line));
                    else if (!strcmp(tag, "Trco")) ch->set(CharTrain::Constitution, atoi(line));
                    else if (!strcmp(tag, "Trin")) ch->set(CharTrain::Intelligence, atoi(line));
                    else if (!strcmp(tag, "Trsp")) ch->set(CharTrain::Speed, atoi(line));
                    else if (!strcmp(tag, "Trst")) ch->set(CharTrain::Strength, atoi(line));
                    else if (!strcmp(tag, "Trwi")) ch->set(CharTrain::Wisdom, atoi(line));
                    break;
                case 'U':
                    if (!strcmp(tag, "Upgr")) GET_UP(ch) = atoi(line);
                    else if (!strcmp(tag, "User")) {
                        if (GET_LOG_USER(ch)) {
                            free(GET_LOG_USER(ch));
                        }
                        GET_LOG_USER(ch) = strdup(line);
                    }
                    break;
                case 'V':
                    if (!strcmp(tag, "Voic")) GET_VOICE(ch) = strdup(line);
                    break;

                case 'W':
                    if (!strcmp(tag, "Wate")) GET_WEIGHT(ch) = atoi(line);
                    else if (!strcmp(tag, "Wimp")) GET_WIMP_LEV(ch) = atoi(line);
                    else if (!strcmp(tag, "Wis ")) ch->set(CharAttribute::Wisdom, atoi(line));
                    break;

                default:
                    sprintf(buf, "SYSERR: Unknown tag %s in pfile %s", tag, name);
            }
        }
    }

    ch->id = player_table[id].id;

    if (!ch->time.created) {
        basic_mud_log("No creation timestamp for user %s, using current time", GET_NAME(ch));
        ch->time.created = time(nullptr);
    }

    ch->generation = ch->time.created;

    if (!ch->time.birth) {
        basic_mud_log("No birthday for user %s, using standard starting age determination", GET_NAME(ch));
        ch->time.birth = time(nullptr) - birth_age(ch);
    }

    /* initialization for imms */
    if (GET_ADMLEVEL(ch) >= ADMLVL_IMMORT) {
        for (i = 1; i <= SKILL_TABLE_SIZE; i++)
            SET_SKILL(ch, i, 100);
        GET_COND(ch, HUNGER) = -1;
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, DRUNK) = -1;
    }


    if (IS_ANDROID(ch)) {
        GET_COND(ch, HUNGER) = -1;
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, DRUNK) = -1;
    }
    fclose(fl);
    return (id);
}


/* remove ^M's from file output */
/* There may be a similar function in Oasis (and I'm sure
   it's part of obuild).  Remove this if you get a
   multiple definition error or if it you want to use a
   substitute
*/
void kill_ems(char *str) {
    char *ptr1, *ptr2, *tmp;

    tmp = str;
    ptr1 = str;
    ptr2 = str;

    while (*ptr1) {
        if ((*(ptr2++) = *(ptr1++)) == '\r')
            if (*ptr1 == '\r')
                ptr1++;
    }
    *ptr2 = '\0';
}


/* Separate a 4-character id tag from the data it precedes */
void tag_argument(char *argument, char *tag) {
    char *tmp = argument, *ttag = tag, *wrt = argument;
    int i;

    for (i = 0; i < 4; i++)
        *(ttag++) = *(tmp++);
    *ttag = '\0';

    while (*tmp == ':' || *tmp == ' ')
        tmp++;

    while (*tmp)
        *(wrt++) = *(tmp++);
    *wrt = '\0';
}


void load_affects(FILE *fl, struct char_data *ch, int violence) {
    int num, num2, num3, num4, num5, num6, i;
    char line[MAX_INPUT_LENGTH + 1];
    struct affected_type af;

    i = 0;
    do {
        get_line(fl, line);
        num = num2 = num3 = num4 = num5 = num6 = 0;
        sscanf(line, "%d %d %d %d %d %d", &num, &num2, &num3, &num4, &num5, &num6);
        if (num != 0) {
            af.type = num;
            af.duration = num2;
            af.modifier = num3;
            af.location = num4;
            af.bitvector = num5;
            af.specific = num6;
            if (violence)
                affectv_to_char(ch, &af);
            else
                affect_to_char(ch, &af);
            i++;
        }
    } while (num != 0);
}


void load_skills(FILE *fl, struct char_data *ch, bool mods) {
    int num = 0, num2 = 0, num3 = 0;
    char line[MAX_INPUT_LENGTH + 1];

    do {
        get_line(fl, line);
        sscanf(line, "%d %d %d", &num, &num2, &num3);
        if (num != 0) {
            if (mods) {
                SET_SKILL_BONUS(ch, num, num2);
            } else {
                SET_SKILL(ch, num, num2);
            }
            SET_SKILL_PERF(ch, num, num3);
        }
    } while (num != 0);
}

void load_bonuses(FILE *fl, struct char_data *ch, bool mods) {
    int num[52] = {0}, i;
    char line[MAX_INPUT_LENGTH + 1];

    get_line(fl, line);
    sscanf(line,
           "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
           &num[0], &num[1], &num[2], &num[3], &num[4], &num[5], &num[6], &num[7], &num[8], &num[9], &num[10], &num[11],
           &num[12], &num[13], &num[14], &num[15], &num[16], &num[17], &num[18], &num[19], &num[20], &num[21], &num[22],
           &num[23], &num[24], &num[25], &num[26], &num[27], &num[28], &num[29], &num[30], &num[31], &num[32], &num[33],
           &num[34], &num[35], &num[36], &num[37], &num[38], &num[39], &num[40], &num[41], &num[42], &num[43], &num[44],
           &num[45], &num[46], &num[47], &num[48], &num[49], &num[50], &num[51]);
    for (i = 0; i < 52; i++) {
        if (num[i] > 0) {
            GET_BONUS(ch, i) = num[i];
        }
    }
}

void load_HMVS(struct char_data *ch, const char *line, int mode) {
    int64_t num = 0, num2 = 0;

    sscanf(line, "%" I64T "/%" I64T "", &num, &num2);

    switch (mode) {
        case LOAD_HIT:
            //GET_HIT(ch) = num;
            break;

        case LOAD_MANA:
            //GET_MANA(ch) = num;
            break;

        case LOAD_MOVE:
            //GET_MOVE(ch) = num;
            break;

        case LOAD_KI:
            //GET_KI(ch) = num;
            break;
    }
}

void load_BASE(struct char_data *ch, const char *line, int mode) {
    int64_t num = 0;

    sscanf(line, "%" I64T "", &num);

    switch (mode) {
        case LOAD_HIT:
            ch->set(CharStat::PowerLevel, num);
            break;

        case LOAD_MANA:
            ch->set(CharStat::Ki, num);
            break;

        case LOAD_MOVE:
            ch->set(CharStat::Stamina, num);
            break;

        case LOAD_LIFE:
            //GET_LIFEFORCE(ch) = num;
            break;
    }
}

void load_majin(struct char_data *ch, const char *line) {
    int64_t num = 0;

    sscanf(line, "%" I64T "", &num);
    GET_MAJINIZED(ch) = num;

}

void load_molt(struct char_data *ch, const char *line) {
    int64_t num = 0;

    sscanf(line, "%" I64T "", &num);
    GET_MOLT_EXP(ch) = num;

}


/*************************************************************************
*  stuff related to the player file cleanup system			 *
*************************************************************************/


/*
 * remove_player() removes all files associated with a player who is
 * self-deleted, deleted by an immortal, or deleted by the auto-wipe
 * system (if enabled).
 */
void remove_player(int pfilepos) {
    char fname[40];
    int i;

    if (!*player_table[pfilepos].name)
        return;

    /* Unlink all player-owned files */
    for (i = 0; i < MAX_FILES; i++) {
        if (get_filename(fname, sizeof(fname), i, player_table[pfilepos].name))
            std::filesystem::remove(fname);
        if (get_filename(fname, sizeof(fname), i, CAP(player_table[pfilepos].name)))
            std::filesystem::remove(fname);
    }

    basic_mud_log("PCLEAN: %s Lev: %d Last: %s",
        player_table[pfilepos].name, player_table[pfilepos].level,
        asctime(localtime(&player_table[pfilepos].last)));
    player_table[pfilepos].name[0] = '\0';
}

struct char_data *findPlayer(const std::string& name) {
    for (auto& player : players) {
        if (boost::iequals(player.second.name, name)) {
            return player.second.character;
        }
    }
    return nullptr;
}

OpResult<> validate_pc_name(const std::string& name) {
    auto n = boost::trim_copy(name);
    // Cannot be empty.
    if(n.empty()) return {false, "Player names cannot be empty."};

    if(n.size() > 15) return {false, "Name is too long. 15 characters or less please."};

    // No whitespace allowed...
    if(std::any_of(n.begin(), n.end(), [](auto c) { return std::isspace(c); }))
        return {false, "Whitespace is not allowed in player names."};

    if(!boost::algorithm::all(n, boost::algorithm::is_alpha())) return {false, "No special symbols or numbers in names, please."};
    // And nothing from our badnames list...
    for(auto &badname : invalid_list) {
        if(boost::iequals(n, badname)) {
            return {false, "That name is disallowed. Nothing profane, lame, or conflicting with an official character please."};
        }
    }

    return {true, n};
}