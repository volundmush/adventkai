/*************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <fstream>
#include "dbat/db.h"
#include "dbat/utils.h"
#include "dbat/feats.h"
#include "dbat/config.h"
#include "dbat/players.h"
#include "dbat/spec_assign.h"
#include "dbat/act.informative.h"
#include "dbat/act.other.h"
#include "dbat/act.social.h"
#include "dbat/random.h"
#include "dbat/assemblies.h"
#include "dbat/house.h"
#include "dbat/dg_event.h"
#include "dbat/reset.h"
#include "dbat/class.h"
#include "dbat/comm.h"
#include "dbat/dg_scripts.h"
#include "dbat/interpreter.h"
#include "dbat/genolc.h"
#include "dbat/shop.h"
#include "dbat/guild.h"
#include "dbat/handler.h"
#include "dbat/mail.h"
#include "dbat/clan.h"
#include "dbat/boards.h"
#include "dbat/objsave.h"
#include "dbat/constants.h"
#include "dbat/genmob.h"
#include "dbat/spells.h"
#include "dbat/races.h"
#include "dbat/spell_parser.h"
#include "dbat/genobj.h"
#include "dbat/area.h"
#include "dbat/account.h"
#include <boost/regex.hpp>

/**************************************************************************
*  declarations of most of the 'global' variables                         *
**************************************************************************/

std::shared_ptr<SQLite::Database> assetDb, stateDb, logDb;
std::set<room_vnum> dirty_rooms;
std::set<obj_vnum> dirty_item_prototypes;
std::set<mob_vnum> dirty_npc_prototypes;
std::set<zone_vnum> dirty_zones;
std::set<vnum> dirty_areas;
std::set<trig_vnum> dirty_dgscript_prototypes;
std::set<guild_vnum> dirty_guilds;
std::set<shop_vnum> dirty_shops;
std::set<int64_t> dirty_players;
std::set<vnum> dirty_accounts;

std::set<int64_t> dirty_characters;
std::set<int64_t> dirty_items;
std::set<int64_t> dirty_dgscripts;

bool gameIsLoading = true;
bool saveAll = false;

struct config_data config_info; /* Game configuration list.    */

std::map<room_vnum, room_data> world;    /* array of rooms		 */
std::map<vnum, area_data> areas;    /* area information		 */

struct char_data *character_list = nullptr; /* global linked list of chars	 */
struct char_data *affect_list = nullptr; /* global linked list of chars with affects */
struct char_data *affectv_list = nullptr; /* global linked list of chars with round-based affects */
std::map<mob_vnum, struct index_data> mob_index;    /* index table for mobile file	 */
std::map<mob_vnum, struct char_data> mob_proto;    /* prototypes for mobs		 */

VnumIndex<obj_data> objectVnumIndex;
VnumIndex<char_data> characterVnumIndex;
VnumIndex<trig_data> scriptVnumIndex;

struct obj_data *object_list = nullptr;    /* global linked list of objs	 */
std::map<obj_vnum, struct index_data> obj_index;    /* index table for object file	 */
std::map<obj_vnum, struct obj_data> obj_proto;    /* prototypes for objs		 */

std::unordered_map<int64_t, std::pair<time_t, struct char_data*>> uniqueCharacters;
/* hash tree for fast obj lookup */
std::unordered_map<int64_t, std::pair<time_t, struct obj_data*>> uniqueObjects;

std::map<zone_vnum, struct zone_data> zone_table;    /* zone table			 */

std::map<trig_vnum, struct index_data> trig_index; /* index table for triggers      */
struct trig_data *trigger_list = nullptr;  /* all attached triggers */
std::map<int64_t, std::pair<time_t, struct trig_data*>> uniqueScripts;


int dg_owner_purged;            /* For control of scripts */

int no_mail = 0;        /* mail disabled?		 */
int mini_mud = 0;        /* mini-mud mode?		 */
int no_rent_check = 0;        /* skip rent check on boot?	 */
time_t boot_time = 0;        /* time of mud boot		 */
int circle_restrict = 0;    /* level of game restriction	 */
int dballtime = 0;              /* used by dragonball load system*/
int SHENRON = false;            /* Shenron has been summoned     */
int DRAGONR = 0;                /* Room Shenron has been summoned to */
int DRAGONZ = 0;                /* Zone Shenron has been summoned to */
int WISH[2] = {0, 0};           /* Keeps track of wishes granted */
int DRAGONC = 0;                /* Keeps count of Shenron's remaining time */
struct char_data *EDRAGON = nullptr;      /* This is Shenron when he is loaded */
room_rnum r_mortal_start_room;    /* rnum of mortal start room	 */
room_rnum r_immort_start_room;    /* rnum of immort start room	 */
room_rnum r_frozen_start_room;    /* rnum of frozen start room	 */
int converting = false;

char *credits = nullptr;        /* game credits			 */
char *news = nullptr;        /* mud news			 */
char *motd = nullptr;        /* message of the day - mortals  */
char *imotd = nullptr;        /* message of the day - immorts  */
char *GREETINGS = nullptr;        /* opening credits screen	 */
char *GREETANSI = nullptr;        /* ansi opening credits screen	 */
char *help = nullptr;        /* help screen			 */
char *info = nullptr;        /* info page			 */
char *wizlist = nullptr;        /* list of higher gods		 */
char *immlist = nullptr;        /* list of peon gods		 */
char *background = nullptr;    /* background story		 */
char *handbook = nullptr;        /* handbook for new immortals	 */
char *policies = nullptr;        /* policies page		 */
char *ihelp = nullptr;        /* help screen (immortals)	 */

struct help_index_element *help_table = nullptr;    /* the help table	 */
int top_of_helpt = 0;

struct social_messg *soc_mess_list = nullptr;      /* list of socials */
int top_of_socialt = -1;                        /* number of socials */

struct time_info_data old_time_info;/* the infomation about the time    */
struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;    /* the infomation about the weather */
std::set<zone_vnum> zone_reset_queue;

std::vector<obj_vnum> dbVnums = {20, 21, 22, 23, 24, 25, 26};

/* local functions */
static void mob_stats(struct char_data *mob);

static void dragon_level(struct char_data *ch);

static int check_bitvector_names(bitvector_t bits, size_t namecount, const char *whatami, const char *whatbits);

static int check_object_spell_number(struct obj_data *obj, int val);

static int check_object_level(struct obj_data *obj, int val);

static void setup_dir(FILE *fl, room_vnum room, int dir);

static void discrete_load(FILE *fl, int mode, char *filename);

static int check_object(struct obj_data *);

static void parse_room(FILE *fl, room_vnum virtual_nr);

static void parse_mobile(FILE *mob_f, mob_vnum nr);

static char *parse_object(FILE *obj_f, obj_vnum nr);

static void load_zones(FILE *fl, char *zonename);

static int file_to_string(const char *name, char *buf);

static int file_to_string_alloc(const char *name, char **buf);

static int count_alias_records(FILE *fl);

static int count_hash_records(FILE *fl);

static bitvector_t asciiflag_conv_aff(char *flag);

static int parse_simple_mob(FILE *mob_f, struct char_data *ch, mob_vnum nr);

static void interpret_espec(const char *keyword, const char *value, struct char_data *ch, mob_vnum nr);

static void parse_espec(char *buf, struct char_data *ch, mob_vnum nr);

static int parse_enhanced_mob(FILE *mob_f, struct char_data *ch, mob_vnum nr);

static void get_one_line(FILE *fl, char *buf);

static void check_start_rooms();

static void log_zone_error(zone_rnum zone, int cmd_no, const char *message);

static void reset_time();

static void free_obj_unique_hash();

static void mob_autobalance(struct char_data *ch);
/* external functions */

void mag_assign_spells();

void create_command_list();

void sort_spells();

void load_banned();

void Read_Invalid_List();

int hsort(const void *a, const void *b);

void prune_crlf(char *txt);

void boot_the_guilds(FILE *gm_f, char *filename, int rec_count);

void destroy_guilds();

void assign_the_guilds();

void memorize_add(struct char_data *ch, int spellnum, int timer);

void assign_feats();

void free_feats();

void sort_feats();

void free_assemblies();

/* external vars */



static void dragon_level(struct char_data *ch) {
    struct descriptor_data *d;
    int level = 0, count = 0;

    for (d = descriptor_list; d; d = d->next) {
        if (IS_PLAYING(d) && GET_ADMLEVEL(d->character) < 1) {
            level += GET_LEVEL(d->character);
            count += 1;
        }
    }

    if (level > 0 && count > 0) {
        level = level / count;
    } else {
        level = rand_number(60, 110);
    }

    if (level < 50) {
        level = rand_number(40, 60);
    }

    ch->set(CharNum::Level, level + rand_number(5, 20));
}

static void mob_stats(struct char_data *mob) {
    int start = GET_LEVEL(mob) * 0.5, finish = GET_LEVEL(mob);

    if (finish < 20)
        finish = 20;

    std::unordered_map<CharAttribute, int> setTo;

    if (!IS_HUMANOID(mob)) {
        setTo[CharAttribute::Strength] = rand_number(start, finish);
        setTo[CharAttribute::Intelligence] = rand_number(start, finish) - 30;
        setTo[CharAttribute::Wisdom] = rand_number(start, finish) - 30;
        setTo[CharAttribute::Agility] = rand_number(start + 5, finish);
        setTo[CharAttribute::Constitution] = rand_number(start + 5, finish);
        setTo[CharAttribute::Speed] = rand_number(start, finish);
    } else {
        if (IS_SAIYAN(mob)) {
            setTo[CharAttribute::Strength] = rand_number(start + 10, finish);
            setTo[CharAttribute::Intelligence] = rand_number(start, finish - 10);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish - 5);
            setTo[CharAttribute::Agility] = rand_number(start, finish);
            setTo[CharAttribute::Constitution] = rand_number(start + 5, finish);
            setTo[CharAttribute::Speed] = rand_number(start + 5, finish);
        } else if (IS_KONATSU(mob)) {
            setTo[CharAttribute::Strength] = rand_number(start, finish - 10);
            setTo[CharAttribute::Intelligence] = rand_number(start, finish);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish);
            setTo[CharAttribute::Agility] = rand_number(start + 10, finish);
            setTo[CharAttribute::Constitution] = rand_number(start, finish);
            setTo[CharAttribute::Speed] = rand_number(start, finish);
        } else if (IS_ANDROID(mob)) {
            setTo[CharAttribute::Strength] = rand_number(start, finish);
            setTo[CharAttribute::Intelligence] = rand_number(start, finish);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish - 10);
            setTo[CharAttribute::Agility] = rand_number(start, finish);
            setTo[CharAttribute::Constitution] = rand_number(start, finish);
            setTo[CharAttribute::Speed] = rand_number(start, finish);
        } else if (IS_MAJIN(mob)) {
            setTo[CharAttribute::Strength] = rand_number(start, finish);
            setTo[CharAttribute::Intelligence] = rand_number(start, finish - 10);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish - 5);
            setTo[CharAttribute::Agility] = rand_number(start, finish);
            setTo[CharAttribute::Constitution] = rand_number(start + 15, finish);
            setTo[CharAttribute::Speed] = rand_number(start, finish);
        } else if (IS_TRUFFLE(mob)) {
            setTo[CharAttribute::Strength] = rand_number(start, finish - 10);
            setTo[CharAttribute::Intelligence] = rand_number(start + 15, finish);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish);
            setTo[CharAttribute::Agility] = rand_number(start, finish);
            setTo[CharAttribute::Constitution] = rand_number(start, finish);
            setTo[CharAttribute::Speed] = rand_number(start, finish);
        } else if (IS_ICER(mob)) {
            setTo[CharAttribute::Strength] = rand_number(start + 5, finish);
            setTo[CharAttribute::Intelligence] = rand_number(start, finish);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish);
            setTo[CharAttribute::Agility] = rand_number(start, finish);
            setTo[CharAttribute::Constitution] = rand_number(start, finish);
            setTo[CharAttribute::Speed] = rand_number(start + 10, finish);
        } else {
            setTo[CharAttribute::Strength] = rand_number(start, finish);
            setTo[CharAttribute::Intelligence] = rand_number(start, finish);
            setTo[CharAttribute::Wisdom] = rand_number(start, finish);
            setTo[CharAttribute::Agility] = rand_number(start, finish);
            setTo[CharAttribute::Constitution] = rand_number(start, finish);
            setTo[CharAttribute::Speed] = rand_number(start, finish);
        }
    }

    for(auto &[attr, val] : setTo) {
        if(val > 100) {
            val = 100;
        } else if(val < 5) {
            val = rand_number(5, 8);
        }
        mob->set(attr, val);
    }
}


/*************************************************************************
*  routines for booting the system                                       *
*************************************************************************/

/* this is necessary for the autowiz system */
void reboot_wizlists() {
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
}

/* Wipe out all the loaded text files, for shutting down. */
void free_text_files() {
    char **textfiles[] = {
            &wizlist, &immlist, &news, &credits, &motd, &imotd, &help, &info,
            &policies, &handbook, &background, &GREETINGS, &GREETANSI, &ihelp, nullptr
    };
    int rf;

    for (rf = 0; textfiles[rf]; rf++)
        if (*textfiles[rf]) {
            free(*textfiles[rf]);
            *textfiles[rf] = nullptr;
        }
}


/*
 * Too bad it doesn't check the return values to let the user
 * know about -1 values.  This will result in an 'Okay.' to a
 * 'reload' command even when the string was not replaced.
 * To fix later, if desired. -gg 6/24/99
 */
ACMD(do_reboot) {
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!strcasecmp(arg, "all") || *arg == '*') {
        if (file_to_string_alloc(GREETINGS_FILE, &GREETINGS) == 0)
            prune_crlf(GREETINGS);
        if (file_to_string_alloc(GREETANSI_FILE, &GREETANSI) == 0)
            prune_crlf(GREETANSI);
        if (file_to_string_alloc(WIZLIST_FILE, &wizlist) < 0)
            send_to_char(ch, "Cannot read wizlist\r\n");
        if (file_to_string_alloc(IMMLIST_FILE, &immlist) < 0)
            send_to_char(ch, "Cannot read immlist\r\n");
        if (file_to_string_alloc(NEWS_FILE, &news) < 0)
            send_to_char(ch, "Cannot read news\r\n");
        if (file_to_string_alloc(CREDITS_FILE, &credits) < 0)
            send_to_char(ch, "Cannot read credits\r\n");
        if (file_to_string_alloc(MOTD_FILE, &motd) < 0)
            send_to_char(ch, "Cannot read motd\r\n");
        if (file_to_string_alloc(IMOTD_FILE, &imotd) < 0)
            send_to_char(ch, "Cannot read imotd\r\n");
        if (file_to_string_alloc(HELP_PAGE_FILE, &help) < 0)
            send_to_char(ch, "Cannot read help front page\r\n");
        if (file_to_string_alloc(INFO_FILE, &info) < 0)
            send_to_char(ch, "Cannot read info file\r\n");
        if (file_to_string_alloc(POLICIES_FILE, &policies) < 0)
            send_to_char(ch, "Cannot read policies\r\n");
        if (file_to_string_alloc(HANDBOOK_FILE, &handbook) < 0)
            send_to_char(ch, "Cannot read handbook\r\n");
        if (file_to_string_alloc(BACKGROUND_FILE, &background) < 0)
            send_to_char(ch, "Cannot read background\r\n");
        if (help_table)
            free_help_table();
        index_boot(DB_BOOT_HLP);
    } else if (!strcasecmp(arg, "wizlist")) {
        if (file_to_string_alloc(WIZLIST_FILE, &wizlist) < 0)
            send_to_char(ch, "Cannot read wizlist\r\n");
    } else if (!strcasecmp(arg, "immlist")) {
        if (file_to_string_alloc(IMMLIST_FILE, &immlist) < 0)
            send_to_char(ch, "Cannot read immlist\r\n");
    } else if (!strcasecmp(arg, "news")) {
        if (file_to_string_alloc(NEWS_FILE, &news) < 0)
            send_to_char(ch, "Cannot read news\r\n");
    } else if (!strcasecmp(arg, "credits")) {
        if (file_to_string_alloc(CREDITS_FILE, &credits) < 0)
            send_to_char(ch, "Cannot read credits\r\n");
    } else if (!strcasecmp(arg, "motd")) {
        if (file_to_string_alloc(MOTD_FILE, &motd) < 0)
            send_to_char(ch, "Cannot read motd\r\n");
    } else if (!strcasecmp(arg, "imotd")) {
        if (file_to_string_alloc(IMOTD_FILE, &imotd) < 0)
            send_to_char(ch, "Cannot read imotd\r\n");
    } else if (!strcasecmp(arg, "help")) {
        if (file_to_string_alloc(HELP_PAGE_FILE, &help) < 0)
            send_to_char(ch, "Cannot read help front page\r\n");
    } else if (!strcasecmp(arg, "info")) {
        if (file_to_string_alloc(INFO_FILE, &info) < 0)
            send_to_char(ch, "Cannot read info\r\n");
    } else if (!strcasecmp(arg, "policy")) {
        if (file_to_string_alloc(POLICIES_FILE, &policies) < 0)
            send_to_char(ch, "Cannot read policy\r\n");
    } else if (!strcasecmp(arg, "handbook")) {
        if (file_to_string_alloc(HANDBOOK_FILE, &handbook) < 0)
            send_to_char(ch, "Cannot read handbook\r\n");
    } else if (!strcasecmp(arg, "background")) {
        if (file_to_string_alloc(BACKGROUND_FILE, &background) < 0)
            send_to_char(ch, "Cannot read background\r\n");
    } else if (!strcasecmp(arg, "greetings")) {
        if (file_to_string_alloc(GREETINGS_FILE, &GREETINGS) == 0)
            prune_crlf(GREETINGS);
        else
            send_to_char(ch, "Cannot read greetings.\r\n");
    } else if (!strcasecmp(arg, "greetansi")) {
        if (file_to_string_alloc(GREETANSI_FILE, &GREETANSI) == 0)
            prune_crlf(GREETANSI);
        else
            send_to_char(ch, "Cannot read greetings.\r\n");
    } else if (!strcasecmp(arg, "xhelp")) {
        if (help_table)
            free_help_table();
        index_boot(DB_BOOT_HLP);
    } else if (!strcasecmp(arg, "ihelp")) {
        if (file_to_string_alloc(IHELP_PAGE_FILE, &ihelp) < 0)
            send_to_char(ch, "Cannot read help front page\r\n");
    } else {
        send_to_char(ch, "Unknown reload option.\r\n");
        return;
    }

    send_to_char(ch, "%s", CONFIG_OK);
}

static void db_load_accounts() {
    SQLite::Statement q(*stateDb, "SELECT id,data FROM accounts");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            accounts.emplace(id, j);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing account %ld: %s", id, e.what());
            continue;
        }

    }

}

static void db_load_players() {
    SQLite::Statement q(*stateDb, "SELECT id,data FROM playerCharacters");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            players.emplace(id, j);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing player %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_characters_initial() {
    SQLite::Statement q(*stateDb, "SELECT id,generation,isPlayer,data FROM characters");

    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto generation = q.getColumn(1).getInt();
        bool isPlayer = q.getColumn(2).getInt();
        auto data = q.getColumn(3).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto c = new char_data();
            if(isPlayer) {
                c->deserializePlayer(j, false);
                auto p = players.find(id);
                if(p != players.end()) {
                    p->second.character = c;
                }
            } else {
                c->deserializeInstance(j, true);
            }
            uniqueCharacters[id] = std::make_pair(generation, c);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing character %ld: %s", id, e.what());
            continue;
        }
    }

}

static void db_load_characters_finish() {
    SQLite::Statement q(*stateDb, "SELECT id,generation,isPlayer,location,relations FROM characters");

    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto generation = q.getColumn(1).getInt();
        bool isPlayer = q.getColumn(2).getInt();
        auto location = q.getColumn(3).getString();
        auto relations = q.getColumn(4).getString();
        try {
            auto cf = uniqueCharacters.find(id);
            if(cf == uniqueCharacters.end()) {
                basic_mud_log("Error finishing character %ld: not found", id);
                continue;
            }
            if(cf->second.first != generation) {
                basic_mud_log("Error finishing character %ld: generation mismatch", id);
                continue;
            }
            auto c = cf->second.second;

            auto j = nlohmann::json::parse(relations);
            auto j2 = nlohmann::json::parse(location);

            if(c) {
                c->deserializeRelations(j);
                c->deserializeLocation(j2);
            }
        } catch(std::exception& e) {
            basic_mud_log("Error finishing character %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_items_initial() {
    SQLite::Statement q(*stateDb, "SELECT id,generation,data FROM items");

    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto generation = q.getColumn(1).getInt();
        auto data = q.getColumn(2).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto i = new obj_data();
            i->deserializeInstance(j, false);
            uniqueObjects[id] = std::make_pair(generation, i);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing item %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_items_finish() {
    SQLite::Statement q(*stateDb, "SELECT id,generation,location,slot,relations FROM items");

    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto generation = q.getColumn(1).getInt();
        auto location = q.getColumn(2).getString();
        auto slot = q.getColumn(3).getInt();
        auto relations = q.getColumn(4).getString();
        try {
            auto cf = uniqueObjects.find(id);
            if(cf == uniqueObjects.end()) {
                basic_mud_log("Error finishing item %ld: not found", id);
                continue;
            }
            if(cf->second.first != generation) {
                basic_mud_log("Error finishing item %ld: generation mismatch", id);
                continue;
            }
            auto i = cf->second.second;

            auto j = nlohmann::json::parse(relations);

            if(i) {
                i->deserializeRelations(j);
                i->deserializeLocation(location, slot);
            }
        } catch(std::exception& e) {
            basic_mud_log("Error finishing item %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_activate_entities() {
    // activate all items which ended up "in the world".
    for(auto &[id, r] : world) {
        if(r.script) r.script->activate();
        assign_triggers(&r, WLD_TRIGGER);
        r.activateContents();
        for(auto c = r.people; c; c = c->next_in_room) {
            if(IS_NPC(c)) {
                c->activate();
            }
        }
    }
}

static void db_load_dgscripts_initial() {
    SQLite::Statement q(*stateDb, "SELECT id,generation,data FROM dgScripts");

    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto generation = q.getColumn(1).getInt();
        auto data = q.getColumn(2).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto t = new trig_data();
            t->deserializeInstance(j);
            uniqueScripts[id] = std::make_pair(generation, t);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing dgscript %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_dgscripts_finish() {
    SQLite::Statement q(*stateDb, "SELECT id,generation,location FROM dgScripts ORDER BY num DESC");

    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto generation = q.getColumn(1).getInt();
        auto location = q.getColumn(2).getString();
        try {
            auto cf = uniqueScripts.find(id);
            if(cf == uniqueScripts.end()) {
                basic_mud_log("Error finishing dgscript %l: not found", id);
                continue;
            }
            if(cf->second.first != generation) {
                basic_mud_log("Error finishing dgscript %ld: generation mismatch", id);
                continue;
            }
            auto t = cf->second.second;

            if(t) {
                t->deserializeLocation(location);
                struct room_data *r;
                struct obj_data *o;
                struct char_data *c;
                switch(t->owner.index()) {
                    case 0:
                        r = std::get<0>(t->owner);
                        t->next = r->script->trig_list;
                        r->script->trig_list = t;
                        r->script->types |= GET_TRIG_TYPE(t);
                        break;
                    case 1:
                        o = std::get<1>(t->owner);
                        t->next = o->script->trig_list;
                        o->script->trig_list = t;
                        o->script->types |= GET_TRIG_TYPE(t);
                        break;
                    case 2:
                        c = std::get<2>(t->owner);
                        t->next = c->script->trig_list;
                        c->script->trig_list = t;
                        c->script->types |= GET_TRIG_TYPE(t);
                        break;
                }
            }
        } catch(std::exception& e) {
            basic_mud_log("Error finishing dgscript %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_globaldata() {
    SQLite::Statement q(*stateDb, "SELECT name,data FROM globalData");

    while(q.executeStep()) {
        auto name = q.getColumn(0).getString();
        auto data = q.getColumn(1).getString();

        try {
            auto j = nlohmann::json::parse(data);
            if(boost::iequals(name, "time")) {
                time_info.deserialize(j);
            } else if(boost::iequals(name, "weather")) {
                weather_info.deserialize(j);
            } else if(boost::iequals(name, "dgGlobals")) {
                auto room = world.find(0);
                if(room != world.end()) {
                    if(!room->second.script) room->second.script = new script_data(&(room->second));
                    deserializeVars(&(room->second.script->global_vars), j);
                }
            } else {
                basic_mud_log("Unknown global data %s", name.c_str());
            }

        } catch(std::exception& e) {
            basic_mud_log("Error parsing global data %s: %s", name.c_str(), e.what());
            continue;
        }
    }
}


static void db_load_zones() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM zones");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            zone_table.emplace(id, j);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing zone %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_dgscript_prototypes() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM dgScriptPrototypes");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto &t = trig_index[id];
            t.vn = id;
            t.proto = new trig_data(j);;
        } catch(std::exception& e) {
            basic_mud_log("Error parsing dgscript %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_rooms() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM rooms");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto r = world.emplace(id, j);
            r.first->second.zone = real_zone_by_thing(id);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing room %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_shops() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM shops");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            shop_index.emplace(id, j);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing shop %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_guilds() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM guilds");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            guild_index.emplace(id, j);
        } catch(std::exception& e) {
            basic_mud_log("Error parsing guild %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_item_prototypes() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM itemPrototypes");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto p = obj_proto.emplace(id, j);
            p.first->second.zone = real_zone_by_thing(id);
            auto &i = obj_index[id];
            i.vn = id;
        } catch(std::exception& e) {
            basic_mud_log("Error parsing item prototype %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_npc_prototypes() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM npcPrototypes");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto p = mob_proto.emplace(id, j);
            p.first->second.zone = real_zone_by_thing(id);
            auto &i = mob_index[id];
            i.vn = id;
        } catch(std::exception& e) {
            basic_mud_log("Error parsing npc prototype %ld: %s", id, e.what());
            continue;
        }
    }
}

static void db_load_areas() {
    SQLite::Statement q(*assetDb, "SELECT id,data FROM areas");
    while(q.executeStep()) {
        auto id = q.getColumn(0).getInt64();
        auto data = q.getColumn(1).getString();
        try {
            auto j = nlohmann::json::parse(data);
            auto a = areas.emplace(id, j);
            auto &area = a.first->second;
            for(auto &r : area.rooms) {
                auto room = world.find(r);
                if(room != world.end()) {
                    room->second.area = id;
                }
            }
        } catch(std::exception& e) {
            basic_mud_log("Error parsing area %ld: %s", id, e.what());
            continue;
        }
    }

    for(auto &[vn, a] : areas) {
        if(a.parent) {
            auto parent = areas.find(a.parent.value());
            if(parent != areas.end()) {
                parent->second.children.insert(vn);
            }
        }
    }
}

static std::vector<std::filesystem::path> getStateFiles() {
    std::filesystem::path dir = "state"; // Change to your directory
    std::vector<std::filesystem::path> files;

    auto pattern = fmt::format("{}-", config::stateDbName);
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().filename().string().starts_with(pattern)) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end(), std::greater<>());
    return files;
}

static void load_new_database() {

    auto files = getStateFiles();

    if (!files.empty()) {
        std::cout << "Newest state file: " << files.front() << '\n';
    } else {
        std::cout << "No matching state files found.\n";
        return;
    }

    try {
        stateDb = std::make_shared<SQLite::Database>(files.front().string(), SQLite::OPEN_READONLY);
    } catch(std::exception& e) {
        logger->critical("Error opening state database: {}", e.what());
        shutdown_game(1);
    }

    basic_mud_log("Loading global data...");
    db_load_globaldata();

    basic_mud_log("Loading accounts.");
    db_load_accounts();

    basic_mud_log("Loading players.");
    db_load_players();

    basic_mud_log("Loading characters initial...");
    db_load_characters_initial();

    basic_mud_log("Loading items initial...");
    db_load_items_initial();

    basic_mud_log("Loading dgscript initial...");
    db_load_dgscripts_initial();

    // Now that all of the game entities have been spawned, we can finish loading
    // relations between them.

    basic_mud_log("Loading characters finish...");
    db_load_characters_finish();

    basic_mud_log("Loading items finish...");
    db_load_items_finish();

    basic_mud_log("Loading dgscript finish...");
    db_load_dgscripts_finish();

    basic_mud_log("Running activation of entities...");
    db_load_activate_entities();
}

static bool newStyle = false;

void boot_world() {

    broadcast("Your vision of the world expands across a vast expanse of numerous existences.\r\n");

    basic_mud_log("Loading Zones...");
    db_load_zones();

    newStyle = !zone_table.empty();

    if(newStyle) {
        basic_mud_log("Loading DgScripts and generating index.");
        db_load_dgscript_prototypes();

        basic_mud_log("Loading mobs and generating index.");
        broadcast("You feel the presence of many beings around you in this strange journey, but cannot quite see them.\r\n");
        db_load_npc_prototypes();

        basic_mud_log("Loading objs and generating index.");
        broadcast("As the world rushes by, countless treasures flicker through your thoughts. Can they one day be yours?\r\n");
        db_load_item_prototypes();

        basic_mud_log("Loading rooms.");
        db_load_rooms();

        broadcast("Names for these wondrous places race through your mind, but you cannot grasp most.\r\n");
        basic_mud_log("Loading areas.");
        db_load_areas();

        load_new_database();


    } else {
        basic_mud_log("Loading legacy world data...");
        basic_mud_log("Loading zone table.");
        index_boot(DB_BOOT_ZON);

        basic_mud_log("Loading triggers and generating index.");
        index_boot(DB_BOOT_TRG);

        basic_mud_log("Loading rooms.");
        index_boot(DB_BOOT_WLD);
    }

    basic_mud_log("Checking start rooms.");
    check_start_rooms();

    if(!newStyle) {
        basic_mud_log("Loading mobs and generating index.");
        index_boot(DB_BOOT_MOB);

        basic_mud_log("Loading objs and generating index.");
        index_boot(DB_BOOT_OBJ);
    }

    basic_mud_log("Loading disabled commands list...");
    load_disabled();

    if (!no_specials) {
        if(newStyle) {
            basic_mud_log("Loading shops.");
            db_load_shops();

            basic_mud_log("Loading guild masters.");
            db_load_guilds();
        } else {
            basic_mud_log("Loading shops.");
            index_boot(DB_BOOT_SHP);

            basic_mud_log("Loading guild masters.");
            index_boot(DB_BOOT_GLD);
        }

    }
    if (SELFISHMETER >= 10) {
        basic_mud_log("Loading Shadow Dragons.");
        load_shadow_dragons();
    }
}


void free_extra_descriptions(struct extra_descr_data *edesc) {
    struct extra_descr_data *enext;

    for (; edesc; edesc = enext) {
        enext = edesc->next;

        free(edesc->keyword);
        free(edesc->description);
        free(edesc);
    }
}


/* Free the world, in a memory allocation sense. */
void destroy_db() {
    ssize_t cnt, itr;
    struct char_data *chtmp;
    struct obj_data *objtmp;

    /* Active Mobiles & Players */
    while (character_list) {
        chtmp = character_list;
        character_list = character_list->next;
        if (chtmp->master)
            stop_follower(chtmp);
        free_char(chtmp);
    }

    /* Active Objects */
    while (object_list) {
        objtmp = object_list;
        object_list = object_list->next;
        free_obj(objtmp);
    }

    uniqueObjects.clear();

    /* Rooms */
    for (auto &r : world) {
        if (r.second.name)
            free(r.second.name);
        if (r.second.look_description)
            free(r.second.look_description);
        free_extra_descriptions(r.second.ex_description);

        /* free any assigned scripts */
        if (SCRIPT(&r.second))
            extract_script(&r.second, WLD_TRIGGER);

        for (itr = 0; itr < NUM_OF_DIRS; itr++) {
            if (!r.second.dir_option[itr])
                continue;

            if (r.second.dir_option[itr]->general_description)
                free(r.second.dir_option[itr]->general_description);
            if (r.second.dir_option[itr]->keyword)
                free(r.second.dir_option[itr]->keyword);
            free(r.second.dir_option[itr]);
        }
    }
    world.clear();

    /* Objects */
    for (auto &o : obj_proto) {
        if (o.second.name)
            free(o.second.name);
        if (o.second.room_description)
            free(o.second.room_description);
        if (o.second.short_description)
            free(o.second.short_description);
        if (o.second.look_description)
            free(o.second.look_description);
        if (o.second.ex_description)
            free_extra_descriptions(o.second.ex_description);
        if (o.second.sbinfo) free(o.second.sbinfo);
    }
    obj_proto.clear();
    obj_index.clear();
    
    /* Mobiles */
    for (auto &m : mob_proto) {
        if (m.second.name)
            free(m.second.name);
        if (m.second.title)
            free(m.second.title);
        if (m.second.short_description)
            free(m.second.short_description);
        if (m.second.room_description)
            free(mob_proto[cnt].room_description);
        if (m.second.look_description)
            free(m.second.look_description);

        while (m.second.affected)
            affect_remove(&m.second, m.second.affected);
    }
    mob_proto.clear();
    mob_index.clear();
    /* Shops */
    destroy_shops();

    /* Guilds */
    destroy_guilds();

    /* Zones */
    /* zone table reset queue */
    zone_reset_queue.clear();

    zone_table.clear();


    /* Triggers */
    for (auto &t : trig_index) {
        if (t.second.proto) {
            /* make sure to nuke the command list (memory leak) */
            /* free_trigger() doesn't free the command list */
            if (t.second.proto->cmdlist) {
                struct cmdlist_element *i, *j;
                i = t.second.proto->cmdlist;
                while (i) {
                    j = i->next;
                    if (i->cmd)
                        free(i->cmd);
                    free(i);
                    i = j;
                }
            }
            free_trigger(t.second.proto);
        }

    }
    trig_index.clear();


    /* context sensitive help system */
    free_context_help();

    free_feats();

    basic_mud_log("Freeing Assemblies.");
    free_assemblies();

}


/* You can define this to anything you want; 1 would work but it would
   be very inefficient. I would recommend that it actually be close to
   your total number of in-game objects if not double or triple it just
   to minimize collisions. The only O(n) [n=NUM_OBJ_UNIQUE_POOLS]
   operation is initialization of the hash table, all other operations
   that have to traverse are O(n) [n=num elements in pool], so more
   pools are better.
     - Elie Rosenblum Dec. 12 2003 */
#define NUM_OBJ_UNIQUE_POOLS 5000

/* body of the booting system */
void boot_db() {
    zone_rnum i;

    basic_mud_log("Boot db -- BEGIN.");

    basic_mud_log("Resetting the game time:");
    broadcast("Your sense of time accelerates and dilates paradoxically as the world unravels and reforms.\r\n");
    reset_time();

    basic_mud_log("Reading news, credits, help, ihelp, bground, info & motds.");
    file_to_string_alloc(NEWS_FILE, &news);
    file_to_string_alloc(CREDITS_FILE, &credits);
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(IMOTD_FILE, &imotd);
    file_to_string_alloc(HELP_PAGE_FILE, &help);
    file_to_string_alloc(INFO_FILE, &info);
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
    file_to_string_alloc(POLICIES_FILE, &policies);
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
    file_to_string_alloc(BACKGROUND_FILE, &background);
    file_to_string_alloc(IHELP_PAGE_FILE, &ihelp);
    file_to_string_alloc(GREETINGS_FILE, &GREETINGS);
    file_to_string_alloc(GREETANSI_FILE, &GREETANSI);

    basic_mud_log("Loading spell definitions.");
    mag_assign_spells();

    basic_mud_log("Loading feats.");
    assign_feats();

    boot_world();

    basic_mud_log("Loading help entries.");
    index_boot(DB_BOOT_HLP);

    basic_mud_log("Setting up context sensitive help system for OLC");
    boot_context_help();

    ERAPLAYERS = players.size();

    //insure_directory(LIB_PLROBJS "CRASH", 0);

    basic_mud_log("Booting mail system.");
    if (!scan_file()) {
        basic_mud_log("    Mail boot failed -- Mail system disabled");
        no_mail = 1;
    }

    basic_mud_log("Loading social messages.");
    boot_social_messages();

    basic_mud_log("Loading Clans.");
    clanBoot();

    basic_mud_log("Building command list.");
    create_command_list(); /* aedit patch -- M. Scott */

    basic_mud_log("Assigning function pointers:");

    if (!no_specials) {
        basic_mud_log("   Mobiles.");
        assign_mobiles();
        basic_mud_log("   Shopkeepers.");
        assign_the_shopkeepers();
        basic_mud_log("   Objects.");
        assign_objects();
        basic_mud_log("   Rooms.");
        assign_rooms();
        basic_mud_log("   Guildmasters.");
        assign_the_guilds();
    }

    basic_mud_log("Booting assembled objects.");
    assemblyBootAssemblies();

    basic_mud_log("Sorting command list and spells.");
    sort_commands();
    sort_spells();
    sort_feats();

    basic_mud_log("Booting boards system.");
    init_boards();

    basic_mud_log("Reading banned site and invalid-name list.");
    load_banned();
    Read_Invalid_List();

    if (!no_rent_check) {
        basic_mud_log("Deleting timed-out crash and rent files:");
        update_obj_file();
        basic_mud_log("   Done.");
    }

    /* Moved here so the object limit code works. -gg 6/24/98 */
    if (!mini_mud) {
        basic_mud_log("Booting houses.");
        House_boot(!newStyle);
    }

    boot_time = time(nullptr);

    broadcast("Database load complete!\r\n");
}


/* save the auction file */
void auc_save() {
    FILE *fl;

    if ((fl = fopen(AUCTION_FILE, "w")) == nullptr)
        basic_mud_log("SYSERR: Can't write to '%s' auction file.", AUCTION_FILE);
    else {
        struct obj_data *obj, *next_obj;

        for (obj = world[real_room(80)].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (obj) {
                fprintf(fl, "%" I64T " %s %d %d %d %d %ld\n", obj->id, GET_AUCTERN(obj), GET_AUCTER(obj),
                        GET_CURBID(obj), GET_STARTBID(obj), GET_BID(obj), GET_AUCTIME(obj));
            }
        }
        fprintf(fl, "~END~\n");
        fclose(fl);
    }
}

/* load from auction file */
void auc_load(struct obj_data *obj) {
    char line[500], filler[50];
    int64_t oID;
    time_t timer;
    int aID, bID, cost, startc;
    FILE *fl;

    if ((fl = fopen(AUCTION_FILE, "r")) == nullptr)
        basic_mud_log("SYSERR: Can't read from '%s' auction file.", AUCTION_FILE);
    else {
        while (!feof(fl)) {
            get_line(fl, line);
            sscanf(line, "%" I64T " %s %d %d %d %d %ld\n", &oID, filler, &aID, &bID, &startc, &cost, &timer);
            if (obj->id == oID) {
                GET_AUCTERN(obj) = strdup(filler);
                GET_AUCTER(obj) = aID;
                GET_CURBID(obj) = bID;
                GET_STARTBID(obj) = startc;
                GET_BID(obj) = cost;
                GET_AUCTIME(obj) = timer;
            }
        }
        fclose(fl);
    }
}

time_t old_beginning_of_time;

/* reset the time in the game from file */
static void reset_time() {
    time_t beginning_of_time = 0;
    FILE *bgtime;

    if ((bgtime = fopen(TIME_FILE, "r")) == nullptr)
        basic_mud_log("SYSERR: Can't read from '%s' time file.", TIME_FILE);
    else {
        fscanf(bgtime, "%ld\n", &beginning_of_time);
        fscanf(bgtime, "%ld\n", &NEWSUPDATE);
        fscanf(bgtime, "%ld\n", &BOARDNEWMORT);
        fscanf(bgtime, "%ld\n", &BOARDNEWDUO);
        fscanf(bgtime, "%ld\n", &BOARDNEWCOD);
        fscanf(bgtime, "%ld\n", &BOARDNEWBUI);
        fscanf(bgtime, "%ld\n", &BOARDNEWIMM);
        fscanf(bgtime, "%ld\n", &INTERESTTIME);
        fscanf(bgtime, "%ld\n", &LASTINTEREST);
        fscanf(bgtime, "%d\n", &HIGHPCOUNT);
        fscanf(bgtime, "%ld\n", &PCOUNTDATE);
        fscanf(bgtime, "%d\n", &WISHTIME);
        fscanf(bgtime, "%d\n", &PCOUNT);
        fscanf(bgtime, "%ld\n", &LASTPAYOUT);
        fscanf(bgtime, "%d\n", &LASTPAYTYPE);
        fscanf(bgtime, "%d\n", &LASTNEWS);
        fscanf(bgtime, "%d\n", &dballtime);
        fscanf(bgtime, "%d\n", &SELFISHMETER);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON1);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON2);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON3);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON4);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON5);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON6);
        fscanf(bgtime, "%d\n", &SHADOW_DRAGON7);
        fscanf(bgtime, "%d\n", &ERAPLAYERS);
        fclose(bgtime);
    }

    if (dballtime == 0)
        dballtime = 604800;

    if (beginning_of_time == 0)
        beginning_of_time = 650336715;
    old_beginning_of_time = beginning_of_time;

}


/* Write the time in 'when' to the MUD-time file. */
void save_mud_time(struct time_info_data *when) {
    FILE *bgtime;

    if ((bgtime = fopen(TIME_FILE, "w")) == nullptr)
        basic_mud_log("SYSERR: Can't write to '%s' time file.", TIME_FILE);
    else {
        fprintf(bgtime, "%ld\n", mud_time_to_secs(when));
        fprintf(bgtime, "%ld\n", NEWSUPDATE);
        fprintf(bgtime, "%ld\n", BOARDNEWMORT);
        fprintf(bgtime, "%ld\n", BOARDNEWDUO);
        fprintf(bgtime, "%ld\n", BOARDNEWCOD);
        fprintf(bgtime, "%ld\n", BOARDNEWBUI);
        fprintf(bgtime, "%ld\n", BOARDNEWIMM);
        fprintf(bgtime, "%ld\n", INTERESTTIME);
        fprintf(bgtime, "%ld\n", LASTINTEREST);
        fprintf(bgtime, "%d\n", HIGHPCOUNT);
        fprintf(bgtime, "%ld\n", PCOUNTDATE);
        fprintf(bgtime, "%d\n", WISHTIME);
        fprintf(bgtime, "%d\n", PCOUNT);
        fprintf(bgtime, "%ld\n", LASTPAYOUT);
        fprintf(bgtime, "%d\n", LASTPAYTYPE);
        fprintf(bgtime, "%d\n", LASTNEWS);
        fprintf(bgtime, "%d\n", dballtime);
        fprintf(bgtime, "%d\n", SELFISHMETER);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON1);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON2);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON3);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON4);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON5);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON6);
        fprintf(bgtime, "%d\n", SHADOW_DRAGON7);
        fprintf(bgtime, "%d\n", ERAPLAYERS);
        fclose(bgtime);
    }
}


/*
 * Thanks to Andrey (andrey@alex-ua.com) for this bit of code, although I
 * did add the 'goto' and changed some "while()" into "do { } while()".
 *	-gg 6/24/98 (technically 6/25/98, but I care not.)
 */
static int count_alias_records(FILE *fl) {
    char key[READ_SIZE], next_key[READ_SIZE];
    char line[READ_SIZE], *scan;
    int total_keywords = 0;

    /* get the first keyword line */
    get_one_line(fl, key);

    while (*key != '$') {
        /* skip the text */
        do {
            get_one_line(fl, line);
            if (feof(fl))
                goto ackeof;
        } while (*line != '#');

        /* now count keywords */
        scan = key;
        do {
            scan = one_word(scan, next_key);
            if (*next_key)
                ++total_keywords;
        } while (*next_key);

        /* get next keyword line (or $) */
        get_one_line(fl, key);

        if (feof(fl))
            goto ackeof;
    }

    return (total_keywords);

    /* No, they are not evil. -gg 6/24/98 */
    ackeof:
    basic_mud_log("SYSERR: Unexpected end of help file.");
    exit(1);    /* Some day we hope to handle these things better... */
}

/* function to count how many hash-mark delimited records exist in a file */
static int count_hash_records(FILE *fl) {
    char buf[128];
    int count = 0;

    while (fgets(buf, 128, fl))
        if (*buf == '#')
            count++;

    return (count);
}


void index_boot(int mode) {
    const char *index_filename, *prefix = nullptr;    /* nullptr or egcs 1.1 complains */
    FILE *db_index, *db_file;
    int rec_count = 0, size[2];
    char buf2[PATH_MAX], buf1[MAX_STRING_LENGTH];

    switch (mode) {
        case DB_BOOT_WLD:
            prefix = WLD_PREFIX;
            break;
        case DB_BOOT_MOB:
            prefix = MOB_PREFIX;
            break;
        case DB_BOOT_OBJ:
            prefix = OBJ_PREFIX;
            break;
        case DB_BOOT_ZON:
            prefix = ZON_PREFIX;
            break;
        case DB_BOOT_SHP:
            prefix = SHP_PREFIX;
            break;
        case DB_BOOT_HLP:
            prefix = HLP_PREFIX;
            break;
        case DB_BOOT_TRG:
            prefix = TRG_PREFIX;
            break;
        case DB_BOOT_GLD:
            prefix = GLD_PREFIX;
            break;
        default:
            basic_mud_log("SYSERR: Unknown subcommand %d to index_boot!", mode);
            exit(1);
    }

    if (mini_mud)
        index_filename = MINDEX_FILE;
    else
        index_filename = INDEX_FILE;

    snprintf(buf2, sizeof(buf2), "%s%s", prefix, index_filename);
    if (!(db_index = fopen(buf2, "r"))) {
        basic_mud_log("SYSERR: opening index file '%s': %s", buf2, strerror(errno));
        exit(1);
    }

    /* first, count the number of records in the file so we can malloc */
    fscanf(db_index, "%s\n", buf1);
    while (*buf1 != '$') {
        snprintf(buf2, sizeof(buf2), "%s%s", prefix, buf1);
        if (!(db_file = fopen(buf2, "r"))) {
            basic_mud_log("SYSERR: File '%s' listed in '%s%s': %s", buf2, prefix,
                index_filename, strerror(errno));
            fscanf(db_index, "%s\n", buf1);
            continue;
        } else {
            if (mode == DB_BOOT_ZON)
                rec_count++;
            else if (mode == DB_BOOT_HLP)
                rec_count += count_alias_records(db_file);
            else
                rec_count += count_hash_records(db_file);
        }

        fclose(db_file);
        fscanf(db_index, "%s\n", buf1);
    }

    /* Exit if 0 records, unless this is shops */
    if (!rec_count) {
        if (mode == DB_BOOT_SHP || mode == DB_BOOT_GLD)
            return;
        basic_mud_log("SYSERR: boot error - 0 records counted in %s/%s.", prefix,
            index_filename);
        exit(1);
    }

    /*
   * NOTE: "bytes" does _not_ include strings or other later malloc'd things.
   */
    switch (mode) {
        case DB_BOOT_TRG:
            break;
        case DB_BOOT_WLD:
            size[0] = sizeof(struct room_data) * rec_count;
            basic_mud_log("   %d rooms, %d bytes.", rec_count, size[0]);
            break;
        case DB_BOOT_MOB:
            size[0] = sizeof(struct index_data) * rec_count;
            size[1] = sizeof(struct char_data) * rec_count;
            basic_mud_log("   %d mobs, %d bytes in index, %d bytes in prototypes.", rec_count, size[0], size[1]);
            break;
        case DB_BOOT_OBJ:
            size[0] = sizeof(struct index_data) * rec_count;
            size[1] = sizeof(struct obj_data) * rec_count;
            basic_mud_log("   %d objs, %d bytes in index, %d bytes in prototypes.", rec_count, size[0], size[1]);
            break;
        case DB_BOOT_ZON:
            size[0] = sizeof(struct zone_data) * rec_count;
            basic_mud_log("   %d zones, %d bytes.", rec_count, size[0]);
            break;
        case DB_BOOT_HLP:
            CREATE(help_table, struct help_index_element, rec_count);
            size[0] = sizeof(struct help_index_element) * rec_count;
            basic_mud_log("   %d entries, %d bytes.", rec_count, size[0]);
            break;
    }

    rewind(db_index);
    fscanf(db_index, "%s\n", buf1);
    while (*buf1 != '$') {
        snprintf(buf2, sizeof(buf2), "%s%s", prefix, buf1);
        if (!(db_file = fopen(buf2, "r"))) {
            basic_mud_log("SYSERR: %s: %s", buf2, strerror(errno));
            exit(1);
        }
        switch (mode) {
            case DB_BOOT_WLD:
            case DB_BOOT_OBJ:
            case DB_BOOT_MOB:
            case DB_BOOT_TRG:
                discrete_load(db_file, mode, buf2);
                break;
            case DB_BOOT_ZON:
                load_zones(db_file, buf2);
                break;
            case DB_BOOT_HLP:
                load_help(db_file, buf2);
                break;
            case DB_BOOT_SHP:
                boot_the_shops(db_file, buf2, rec_count);
                break;
            case DB_BOOT_GLD:
                boot_the_guilds(db_file, buf2, rec_count);
                break;
        }

        fclose(db_file);
        fscanf(db_index, "%s\n", buf1);
    }
    fclose(db_index);

    /* Sort the help index. */
    if (mode == DB_BOOT_HLP) {
        qsort(help_table, top_of_helpt, sizeof(struct help_index_element), hsort);
        top_of_helpt--;
    }
}


static void discrete_load(FILE *fl, int mode, char *filename) {
    int nr = -1, last;
    char line[READ_SIZE];

    const char *modes[] = {"world", "mob", "obj", "ZON", "SHP", "HLP", "trg"};
    /* modes positions correspond to DB_BOOT_xxx in db.h */

    for (;;) {
        /*
     * we have to do special processing with the obj files because they have
     * no end-of-record marker :(
     */
        if (mode != DB_BOOT_OBJ || nr < 0)
            if (!get_line(fl, line)) {
                if (nr == -1) {
                    basic_mud_log("SYSERR: %s file %s is empty!", modes[mode], filename);
                } else {
                    basic_mud_log("SYSERR: Format error in %s after %s #%d\n"
                        "...expecting a new %s, but file ended!\n"
                        "(maybe the file is not terminated with '$'?)", filename,
                        modes[mode], nr, modes[mode]);
                }
                exit(1);
            }
        if (*line == '$')
            return;

        if (*line == '#') {
            last = nr;
            if (sscanf(line, "#%d", &nr) != 1) {
                basic_mud_log("SYSERR: Format error after %s #%d", modes[mode], last);
                exit(1);
            }
            if (nr >= 99999)
                return;
            else
                switch (mode) {
                    case DB_BOOT_WLD:
                        parse_room(fl, nr);
                        break;
                    case DB_BOOT_MOB:
                        parse_mobile(fl, nr);
                        break;
                    case DB_BOOT_TRG:
                        parse_trigger(fl, nr);
                        break;
                    case DB_BOOT_OBJ:
                        strlcpy(line, parse_object(fl, nr), sizeof(line));
                        break;
                }
        } else {
            basic_mud_log("SYSERR: Format error in %s file %s near %s #%d", modes[mode],
                filename, modes[mode], nr);
            basic_mud_log("SYSERR: ... offending line: '%s'", line);
            exit(1);
        }
    }
}

char fread_letter(FILE *fp) {
    char c;
    do {
        c = getc(fp);
    } while (isspace(c));
    return c;
}

bitvector_t asciiflag_conv(char *flag) {
    bitvector_t flags = 0;
    int is_num = true;
    char *p;

    for (p = flag; *p; p++) {
        if (islower(*p))
            flags |= 1 << (*p - 'a');
        else if (isupper(*p))
            flags |= 1 << (26 + (*p - 'A'));

        if (!(isdigit(*p) || (*p == '-')))
            is_num = false;
    }

    if (is_num)
        flags = atol(flag);

    return (flags);
}

static bitvector_t asciiflag_conv_aff(char *flag) {
    bitvector_t flags = 0;
    int is_num = true;
    char *p;

    for (p = flag; *p; p++) {
        if (islower(*p))
            flags |= 1 << (1 + (*p - 'a'));
        else if (isupper(*p))
            flags |= 1 << (26 + (*p - 'A'));

        if (!(isdigit(*p) || (*p == '-')))
            is_num = false;
    }

    if (is_num)
        flags = atol(flag);

    return (flags);
}

/* load the rooms */
static void parse_room(FILE *fl, room_vnum virtual_nr) {
    int t[10], i, retval;
    char line[READ_SIZE], flags[128], flags2[128], flags3[128];
    char flags4[128], buf2[MAX_STRING_LENGTH], buf[128];
    bitvector_t roomFlagsHolder[4];
    struct extra_descr_data *new_descr;
    char letter;

    /* This really had better fit or there are other problems. */
    snprintf(buf2, sizeof(buf2), "room #%d", virtual_nr);

    auto zone = real_zone_by_thing(virtual_nr);
    if (zone == NOWHERE) {
        basic_mud_log("SYSERR: Room #%d is outside any zone.", virtual_nr);
        exit(1);
    }

    if(world.count(virtual_nr)) {
        basic_mud_log("SYSERR: Room #%d already exists, cannot parse!", virtual_nr);
        exit(1);
    }
    auto &z = zone_table[zone];
    auto &r = world[virtual_nr];
    z.rooms.insert(virtual_nr);

    r.zone = zone;
    r.vn = virtual_nr;
    r.name = fread_string(fl, buf2);
    r.look_description = fread_string(fl, buf2);

    if (!get_line(fl, line)) {
        basic_mud_log("SYSERR: Expecting roomflags/sector type of room #%d but file ended!",
            virtual_nr);
        exit(1);
    }

    if ((retval = sscanf(line, " %d %s %s %s %s %d ", t, flags, flags2, flags3, flags4, t + 2)) == 6) {
        int taeller;
        roomFlagsHolder[0] = asciiflag_conv(flags);
        roomFlagsHolder[1] = asciiflag_conv(flags2);
        roomFlagsHolder[2] = asciiflag_conv(flags3);
        roomFlagsHolder[3] = asciiflag_conv(flags4);

        for(auto i = 0; i < NUM_ROOM_FLAGS; i++) if(IS_SET_AR(roomFlagsHolder, i)) r.room_flags.set(i);

        r.sector_type = t[2];
        sprintf(flags, "object #%d", virtual_nr);    /* sprintf: OK (until 399-bit integers) */
        //check_bitvector_names(r.room_flags, room_bits_count, flags, "room");
    } else {
        basic_mud_log("SYSERR: Format error in roomflags/sector type of room #%d", virtual_nr);
        exit(1);
    }

    r.func = nullptr;
    r.contents = nullptr;
    r.people = nullptr;
    r.timed = -1;

    for (i = 0; i < NUM_OF_DIRS; i++)
        r.dir_option[i] = nullptr;

    r.ex_description = nullptr;

    snprintf(buf, sizeof(buf), "SYSERR: Format error in room #%d (expecting D/E/S)", virtual_nr);

    while(true) {
        if (!get_line(fl, line)) {
            basic_mud_log("%s", buf);
            exit(1);
        }
        switch (*line) {
            case 'D':
                setup_dir(fl, virtual_nr, atoi(line + 1));
                break;
            case 'E':
                CREATE(new_descr, struct extra_descr_data, 1);
                new_descr->keyword = fread_string(fl, buf2);
                new_descr->description = fread_string(fl, buf2);
                /* fix for crashes in the editor when formatting
       * - e-descs are assumed to end with a \r\n
       * -- Welcor 09/03
       */
                {
                    char *tmp = strchr(new_descr->description, '\0');
                    if (tmp > new_descr->description && *(tmp - 1) != '\n') {
                        CREATE(tmp, char, strlen(new_descr->description) + 3);
                        sprintf(tmp, "%s\r\n", new_descr->description); /* sprintf ok : size checked above*/
                        free(new_descr->description);
                        new_descr->description = tmp;
                    }
                }
                new_descr->next = r.ex_description;
                r.ex_description = new_descr;
                break;
            case 'S':            /* end of room */
                /* DG triggers -- script is defined after the end of the room */
                letter = fread_letter(fl);
                ungetc(letter, fl);
                while (letter == 'T') {
                    dg_read_trigger(fl, &world[virtual_nr], WLD_TRIGGER);
                    letter = fread_letter(fl);
                    ungetc(letter, fl);
                }
                return;
            default:
                basic_mud_log("%s", buf);
                exit(1);
        }
    }
}

/* read direction data */
static void setup_dir(FILE *fl, room_vnum room, int dir) {
    int t[11], retval;
    char line[READ_SIZE], buf2[128];

    snprintf(buf2, sizeof(buf2), "room #%d, direction D%d", room, dir);
    
    auto &r = world[room];

    CREATE(r.dir_option[dir], struct room_direction_data, 1);
    
    auto d = r.dir_option[dir];
    
    d->general_description = fread_string(fl, buf2);
    d->keyword = fread_string(fl, buf2);

    if (!get_line(fl, line)) {
        basic_mud_log("SYSERR: Format error, %s", buf2);
        exit(1);
    }
    if (((retval = sscanf(line, " %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7,
                          t + 8, t + 9, t + 10)) == 3) && (bitwarning == true)) {
        basic_mud_log("SYSERR: Format error, %s", buf2);
        exit(1);
    } else if (bitwarning == false) {

        if (t[0] == 1)
            d->exit_info = EX_ISDOOR;
        else if (t[0] == 2)
            d->exit_info = EX_ISDOOR | EX_PICKPROOF;
        else if (t[0] == 3)
            d->exit_info = EX_ISDOOR | EX_SECRET;
        else if (t[0] == 4)
            d->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_SECRET;
        else
            d->exit_info = 0;

        d->key = ((t[1] == -1 || t[1] == 65535) ? NOTHING : t[1]);
        d->to_room = ((t[2] == -1 || t[2] == 65535) ? NOWHERE : t[2]);

        if (retval == 3) {
            basic_mud_log("Converting world files to include DC add ons.");
            d->dclock = 20;
            d->dchide = 20;
            d->dcskill = 0;
            d->dcmove = 0;
            d->failsavetype = 0;
            d->dcfailsave = 0;
            d->failroom = NOWHERE;
            d->totalfailroom = NOWHERE;
            if (bitsavetodisk) {
                r.save();
                converting = true;
            }
        } else if (retval == 5) {
            d->dclock = t[3];
            d->dchide = t[4];
            d->dcskill = 0;
            d->dcmove = 0;
            d->failsavetype = 0;
            d->dcfailsave = 0;
            d->failroom = NOWHERE;
            d->totalfailroom = NOWHERE;
            if (bitsavetodisk) {
                r.save();
                converting = true;
            }
        } else if (retval == 7) {
            d->dclock = t[3];
            d->dchide = t[4];
            d->dcskill = t[5];
            d->dcmove = t[6];
            d->failsavetype = 0;
            d->dcfailsave = 0;
            d->failroom = NOWHERE;
            d->totalfailroom = NOWHERE;
            if (bitsavetodisk) {
                r.save();
                converting = true;
            }
        } else if (retval == 11) {
            d->dclock = t[3];
            d->dchide = t[4];
            d->dcskill = t[5];
            d->dcmove = t[6];
            d->failsavetype = t[7];
            d->dcfailsave = t[8];
            d->failroom = t[9];
            d->totalfailroom = t[10];
        }
    }
}


/* make sure the start rooms exist & resolve their vnums to rnums */
static void check_start_rooms() {
    if ((r_mortal_start_room = real_room(CONFIG_MORTAL_START)) == NOWHERE) {
        basic_mud_log("SYSERR:  Mortal start room does not exist.  Change mortal_start_room in lib/etc/config.");
        exit(1);
    }
    if ((r_immort_start_room = real_room(CONFIG_IMMORTAL_START)) == NOWHERE) {
        if (!mini_mud)
            basic_mud_log("SYSERR:  Warning: Immort start room does not exist.  Change immort_start_room in /lib/etc/config.");
        r_immort_start_room = r_mortal_start_room;
    }
    if ((r_frozen_start_room = real_room(CONFIG_FROZEN_START)) == NOWHERE) {
        if (!mini_mud)
            basic_mud_log("SYSERR:  Warning: Frozen start room does not exist.  Change frozen_start_room in /lib/etc/config.");
        r_frozen_start_room = r_mortal_start_room;
    }
}


#define ZCMD2 zone_table[zone].cmd[cmd_no]

/*
 * "resulve vnums into rnums in the zone reset tables"
 *
 * Or in English: Once all of the zone reset tables have been loaded, we
 * resolve the virtual numbers into real numbers all at once so we don't have
 * to do it repeatedly while the game is running.  This does make adding any
 * room, mobile, or object a little more difficult while the game is running.
 *
 * NOTE 1: Assumes NOWHERE == NOBODY == NOTHING.
 * NOTE 2: Assumes sizeof(room_rnum) >= (sizeof(mob_rnum) and sizeof(obj_rnum))
 */

static void mob_autobalance(struct char_data *ch) {

}

static int parse_simple_mob(FILE *mob_f, struct char_data *ch, mob_vnum nr) {
    int j, t[10];
    char line[READ_SIZE];

    if (!get_line(mob_f, line)) {
        basic_mud_log("SYSERR: Format error in mob #%d, file ended after S flag!", nr);
        return 0;
    }

    if (sscanf(line, " %ld %ld %ld %ldd%ld+%ld %ldd%ld+%ld ",
               t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9) {
        basic_mud_log("SYSERR: Format error in mob #%d, first line after S flag\n"
            "...expecting line of form '# # # #d#+# #d#+#'", nr);
        return 0;
    }

    ch->set(CharNum::Level, t[0]);

    /* max hit = 0 is a flag that H, M, V is xdy+z */
    ch->set(CharStat::PowerLevel, t[3]);
    ch->set(CharStat::Ki, t[4]);
    ch->set(CharStat::Stamina, t[5]);
    ch->health = 1.0;
    ch->energy = 1.0;
    ch->stamina = 1.0;

    ch->mob_specials.damnodice = t[6];
    ch->mob_specials.damsizedice = t[7];
    GET_DAMAGE_MOD(ch) = t[8];

    if (!get_line(mob_f, line)) {
        basic_mud_log("SYSERR: Format error in mob #%d, second line after S flag\n"
            "...expecting line of form '# #', but file ended!", nr);
        return 0;
    }

    if (sscanf(line, " %ld %ld %ld %ld", t, t + 1, t + 2, t + 3) != 4) {
        basic_mud_log("SYSERR: Format error in mob #%d, second line after S flag\n"
            "...expecting line of form '# # # #'", nr);
        return 0;
    }

    ch->set(CharMoney::Carried, t[0]);
    ch->race = static_cast<RaceID>(t[2]);

    ch->chclass = static_cast<SenseiID>(t[3]);
    GET_SAVE_BASE(ch, SAVING_FORTITUDE) = 0;
    GET_SAVE_BASE(ch, SAVING_REFLEX) = 0;
    GET_SAVE_BASE(ch, SAVING_WILL) = 0;

    /* GET_CLASS_RANKS(ch, t[3]) = GET_LEVEL(ch); */

    if (!IS_HUMAN(ch))
        ch->affected_by.set(AFF_INFRAVISION);

    SPEAKING(ch) = SKILL_LANG_COMMON;

    if (!get_line(mob_f, line)) {
        basic_mud_log("SYSERR: Format error in last line of mob #%d\n"
            "...expecting line of form '# # #', but file ended!", nr);
        return 0;
    }

    if (sscanf(line, " %ld %ld %ld ", t, t + 1, t + 2) != 3) {
        basic_mud_log("SYSERR: Format error in last line of mob #%d\n"
            "...expecting line of form '# # #'", nr);
        return 0;
    }

    GET_POS(ch) = t[0];
    GET_DEFAULT_POS(ch) = t[1];
    ch->set(CharAppearance::Sex, t[2]);

    SPEAKING(ch) = MIN_LANGUAGES;
    set_height_and_weight_by_race(ch);

    for (j = 0; j < 3; j++)
        GET_SAVE_MOD(ch, j) = 0;

    if (MOB_FLAGGED(ch, MOB_AUTOBALANCE)) {
        mob_autobalance(ch);
    }

    return 1;
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 *
 * CASE		: Requires a parameter through 'value'.
 * BOOL_CASE	: Being specified at all is its value.
 */

#define CASE(test)    \
    if (value && !matched && !strcasecmp(keyword, test) && (matched = false))

#define BOOL_CASE(test)    \
    if (!value && !matched && !strcasecmp(keyword, test) && (matched = true))

#define RANGE(low, high)    \
    (num_arg = MAX((low), MIN((high), (num_arg))))

static void interpret_espec(const char *keyword, const char *value, struct char_data *ch, mob_vnum nr) {
    int num_arg = 0, matched = false;
    int num, num2, num3, num4, num5, num6;
    struct affected_type af;

    /*
   * If there isn't a colon, there is no value.  While Boolean options are
   * possible, we don't actually have any.  Feel free to make some.
  */
    if (value)
        num_arg = atoi(value);

    CASE("BareHandAttack") {
        RANGE(0, 99);
        ch->mob_specials.attack_type = num_arg;
    }

    CASE("Size") {
        RANGE(SIZE_UNDEFINED, NUM_SIZES - 1);
        ch->setSize(num_arg);
    }

    CASE("Str") {
        RANGE(0, 200);
        ch->set(CharAttribute::Strength, num_arg);
    }

    CASE("StrAdd") {
        basic_mud_log("mob #%d trying to set StrAdd, rebalance its strength.",
            GET_MOB_VNUM(ch));
    }

    CASE("Int") {
        RANGE(0, 200);
        ch->set(CharAttribute::Intelligence, num_arg);
    }

    CASE("Wis") {
        RANGE(0, 200);
        ch->set(CharAttribute::Wisdom, num_arg);
    }

    CASE("Dex") {
        RANGE(0, 200);
        ch->set(CharAttribute::Agility, num_arg);
    }

    CASE("Con") {
        RANGE(0, 200);
        ch->set(CharAttribute::Constitution, num_arg);
    }

    CASE("Cha") {
        RANGE(0, 200);
        ch->set(CharAttribute::Speed, num_arg);
    }

    CASE("Hit") {
        RANGE(0, 99999);
        //GET_HIT(ch) = num_arg;
    }

    CASE("Mana") {
        RANGE(0, 99999);
        //GET_MANA(ch) = num_arg;
    }

    CASE("Moves") {
        RANGE(0, 99999);
        //GET_MOVE(ch) = num_arg;
    }

    CASE("Affect") {
        num = num2 = num3 = num4 = num5 = num6 = 0;
        sscanf(value, "%d %d %d %d %d %d", &num, &num2, &num3, &num4, &num5, &num6);
        if (num > 0) {
            af.type = num;
            af.duration = num2;
            af.modifier = num3;
            af.location = num4;
            af.bitvector = num5;
            af.specific = num6;
            affect_to_char(ch, &af);
        }
    }

    CASE("AffectV") {
        num = num2 = num3 = num4 = num5 = num6 = 0;
        sscanf(value, "%d %d %d %d %d %d", &num, &num2, &num3, &num4, &num5, &num6);
        if (num > 0) {
            af.type = num;
            af.duration = num2;
            af.modifier = num3;
            af.location = num4;
            af.bitvector = num5;
            af.specific = num6;
            affectv_to_char(ch, &af);
        }
    }

    CASE("Feat") {
        sscanf(value, "%d %d", &num, &num2);
        HAS_FEAT(ch, num) = num2;
    }

    CASE("Skill") {
        sscanf(value, "%d %d", &num, &num2);
        SET_SKILL(ch, num, num2);
    }

    CASE("SkillMod") {
        sscanf(value, "%d %d", &num, &num2);
        SET_SKILL_BONUS(ch, num, num2);
    }

    if (!matched) {
        basic_mud_log("SYSERR: Warning: unrecognized espec keyword %s in mob #%d",
            keyword, nr);
    }
}

#undef CASE
#undef BOOL_CASE
#undef RANGE

static void parse_espec(char *buf, struct char_data *ch, mob_vnum nr) {
    char *ptr;

    if ((ptr = strchr(buf, ':')) != nullptr) {
        *(ptr++) = '\0';
        while (isspace(*ptr))
            ptr++;
    }
    interpret_espec(buf, ptr, ch, nr);
}

static int parse_enhanced_mob(FILE *mob_f, struct char_data *ch, mob_vnum nr) {
    char line[READ_SIZE];

    parse_simple_mob(mob_f, ch, nr);

    while (get_line(mob_f, line)) {
        if (!strcmp(line, "E"))    /* end of the enhanced section */
            return 1;
        else if (*line == '#') {    /* we've hit the next mob, maybe? */
            basic_mud_log("SYSERR: Unterminated E section in mob #%d", nr);
            return 0;
        } else
            parse_espec(line, ch, nr);
    }

    basic_mud_log("SYSERR: Unexpected end of file reached after mob #%d", nr);
    return 0;
}

int parse_mobile_from_file(FILE *mob_f, struct char_data *ch) {
    int j, t[10], retval;
    char line[READ_SIZE], *tmpptr, letter;
    char f1[128], f2[128], f3[128], f4[128], f5[128], f6[128];
    char f7[128], f8[128], buf2[128];
    mob_vnum nr = ch->vn;
    auto &z = zone_table[real_zone_by_thing(nr)];
    z.mobiles.insert(nr);

    /*
   * Mobiles should NEVER use anything in the 'player_specials' structure.
   * The only reason we have every mob in the game share this copy of the
   * structure is to save newbie coders from themselves. -gg 2/25/98
   */

    sprintf(buf2, "mob vnum %d", nr);   /* sprintf: OK (for 'buf2 >= 19') */

    /***** String data *****/
    ch->name = fread_string(mob_f, buf2);
    tmpptr = ch->short_description = fread_string(mob_f, buf2);
    if (tmpptr && *tmpptr)
        if (!strcasecmp(fname(tmpptr), "a") || !strcasecmp(fname(tmpptr), "an") ||
            !strcasecmp(fname(tmpptr), "the"))
            *tmpptr = LOWER(*tmpptr);
    ch->room_description = fread_string(mob_f, buf2);
    ch->look_description = fread_string(mob_f, buf2);

    /* *** Numeric data *** */
    if (!get_line(mob_f, line)) {
        basic_mud_log("SYSERR: Format error after string section of mob #%d\n"
            "...expecting line of form '# # # {S | E}', but file ended!", nr);
        return 0;
    }

    if ((retval = sscanf(line, "%s %s %s %s %s %s %s %s %d %c", f1, f2, f3, f4, f5, f6, f7, f8, t + 2, &letter)) == 10) {
        int taeller;

        bitvector_t mf[4], aff[4];

        mf[0] = asciiflag_conv(f1);
        mf[1] = asciiflag_conv(f2);
        mf[2] = asciiflag_conv(f3);
        mf[3] = asciiflag_conv(f4);
        for (taeller = 0; taeller < AF_ARRAY_MAX; taeller++)
            check_bitvector_names(MOB_FLAGS(ch)[taeller], action_bits_count, buf2, "mobile");
        for(auto i = 0; i < ch->mobFlags.size(); i++) ch->mobFlags.set(i, IS_SET_AR(mf, i));

        aff[0] = asciiflag_conv(f5);
        aff[1] = asciiflag_conv(f6);
        aff[2] = asciiflag_conv(f7);
        aff[3] = asciiflag_conv(f8);
        for(auto i = 0; i < ch->affected_by.size(); i++) ch->affected_by.set(i, IS_SET_AR(aff, i));

        ch->set(CharAlign::GoodEvil, t[2]);

        for (taeller = 0; taeller < AF_ARRAY_MAX; taeller++)
            check_bitvector_names(AFF_FLAGS(ch)[taeller], affected_bits_count, buf2, "mobile affect");
    } else {
        basic_mud_log("SYSERR: Format error after string section of mob #%d\n"
            "...expecting line of form '# # # {S | E}'", nr);
        exit(1);
    }

    ch->mobFlags.set(MOB_ISNPC);
    if (MOB_FLAGGED(ch, MOB_NOTDEADYET)) {
        /* Rather bad to load mobiles with this bit already set. */
        basic_mud_log("SYSERR: Mob #%d has reserved bit MOB_NOTDEADYET set.", nr);
        ch->mobFlags.reset(MOB_NOTDEADYET);
    }

    /* AGGR_TO_ALIGN is ignored if the mob is AGGRESSIVE.
  if (MOB_FLAGGED(mob_proto + i, MOB_AGGRESSIVE) && MOB_FLAGGED(mob_proto + i, MOB_AGGR_GOOD | MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL))
    log("SYSERR: Mob #%d both Aggressive and Aggressive_to_Alignment.", nr); */

    /* Convert mobs to use AUTOBALANCE. Uncomment and reboot to flag all mobs AUTOBALANCE.
   * if (!MOB_FLAGGED(ch, MOB_AUTOBALANCE)) {
   *   SET_BIT_AR(MOB_FLAGS(ch), MOB_AUTOBALANCE);
   * } */

    switch (UPPER(letter)) {
        case 'S':    /* Simple monsters */
            parse_simple_mob(mob_f, ch, nr);
            break;
        case 'E':    /* Circle3 Enhanced monsters */
            parse_enhanced_mob(mob_f, ch, nr);
            mob_stats(ch);
            break;
            /* add new mob types here.. */
        default:
            basic_mud_log("SYSERR: Unsupported mob type '%c' in mob #%d", letter, nr);
            exit(1);
    }

    /* DG triggers -- script info follows mob S/E section */
    letter = fread_letter(mob_f);
    ungetc(letter, mob_f);
    while (letter == 'T') {
        dg_read_trigger(mob_f, ch, MOB_TRIGGER);
        letter = fread_letter(mob_f);
        ungetc(letter, mob_f);
    }

    for (j = 0; j < NUM_WEARS; j++)
        ch->equipment[j] = nullptr;

    /* Uncomment to force all mob files to be rewritten. Good for initial AUTOBALANCE setup.
   * if (bitsavetodisk) {
   *   add_to_save_list(zone_table[real_zone_by_thing(nr)].number, 0);
   *   converting = TRUE;
   * } */

    return 1;
}


static void parse_mobile(FILE *mob_f, mob_vnum nr) {
    auto &idx = mob_index[nr];
    idx.vn = nr;
    
    auto &m = mob_proto[nr];

    m.vn = nr;
    m.desc = nullptr;

    if (parse_mobile_from_file(mob_f, &m)) {

    } else { /* We used to exit in the file reading code, but now we do it here */
        exit(1);
    }
}


/* read all objects from obj file; generate index and prototypes */
static char *parse_object(FILE *obj_f, obj_vnum nr) {
    static char line[READ_SIZE];
    int64_t t[NUM_OBJ_VAL_POSITIONS + 2], j, retval;
    char *tmpptr, buf2[128];
    char f1[READ_SIZE], f2[READ_SIZE], f3[READ_SIZE], f4[READ_SIZE];
    char f5[READ_SIZE], f6[READ_SIZE], f7[READ_SIZE], f8[READ_SIZE];
    char f9[READ_SIZE], f10[READ_SIZE], f11[READ_SIZE], f12[READ_SIZE];
    struct extra_descr_data *new_descr;
    
    auto &o = obj_proto[nr];
    auto &idx = obj_index[nr];
    
    idx.vn = nr;
    o.vn = nr;
    
    sprintf(buf2, "object #%d", nr);    /* sprintf: OK (for 'buf2 >= 19') */

    /* *** string data *** */
    if ((o.name = fread_string(obj_f, buf2)) == nullptr) {
        basic_mud_log("SYSERR: Null obj name or format error at or near %s", buf2);
        exit(1);
    }
    auto &z = zone_table[real_zone_by_thing(nr)];
    z.objects.insert(nr);
    tmpptr = o.short_description = fread_string(obj_f, buf2);
    if (tmpptr && *tmpptr)
        if (!strcasecmp(fname(tmpptr), "a") || !strcasecmp(fname(tmpptr), "an") ||
            !strcasecmp(fname(tmpptr), "the"))
            *tmpptr = LOWER(*tmpptr);

    tmpptr = o.room_description = fread_string(obj_f, buf2);
    if (tmpptr && *tmpptr)
        CAP(tmpptr);
    o.look_description = fread_string(obj_f, buf2);

    /* *** numeric data *** */
    if (!get_line(obj_f, line)) {
        basic_mud_log("SYSERR: Expecting first numeric line of %s, but file ended!", buf2);
        exit(1);
    }
    if ((retval = sscanf(line, " %d %s %s %s %s %s %s %s %s %s %s %s %s", t, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10,
                                f11, f12)) == 13) {
        bitvector_t extraFlags[4], wearFlags[4], permFlags[4];

        extraFlags[0] = asciiflag_conv(f1);
        extraFlags[1] = asciiflag_conv(f2);
        extraFlags[2] = asciiflag_conv(f3);
        extraFlags[3] = asciiflag_conv(f4);
        for(auto i = 0; i < o.extra_flags.size(); i++) if(IS_SET_AR(extraFlags, i)) o.extra_flags.set(i);

        wearFlags[0] = asciiflag_conv(f5);
        wearFlags[1] = asciiflag_conv(f6);
        wearFlags[2] = asciiflag_conv(f7);
        wearFlags[3] = asciiflag_conv(f8);
        for(auto i = 0; i < o.wear_flags.size(); i++) if(IS_SET_AR(wearFlags, i)) o.wear_flags.set(i);

        permFlags[0] = asciiflag_conv(f9);
        permFlags[1] = asciiflag_conv(f10);
        permFlags[2] = asciiflag_conv(f11);
        permFlags[3] = asciiflag_conv(f12);
        for(auto i = 0; i < o.bitvector.size(); i++) if(IS_SET_AR(permFlags, i)) o.bitvector.set(i);

    } else {
        basic_mud_log("SYSERR: Format error in first numeric line (expecting 13 args, got %d), %s", retval, buf2);
        exit(1);
    }

    /* Object flags checked in check_object(). */
    GET_OBJ_TYPE(&o) = t[0];

    if (!get_line(obj_f, line)) {
        basic_mud_log("SYSERR: Expecting second numeric line of %s, but file ended!", buf2);
        exit(1);
    }

    for (j = 0; j < NUM_OBJ_VAL_POSITIONS; j++)
        t[j] = 0;

    if ((retval = sscanf(line, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", t, t + 1, t + 2, t + 3, t + 4, t + 5,
                         t + 6, t + 7, t + 8, t + 9, t + 10, t + 11, t + 12, t + 13, t + 14, t + 15)) >
        NUM_OBJ_VAL_POSITIONS) {
        basic_mud_log("SYSERR: Format error in second numeric line (expecting <=%d args, got %d), %s", NUM_OBJ_VAL_POSITIONS,
            retval, buf2);
        exit(1);
    }

    for (j = 0; j < NUM_OBJ_VAL_POSITIONS; j++)
        GET_OBJ_VAL(&o, j) = t[j];

    if ((GET_OBJ_TYPE(&o) == ITEM_PORTAL || \
       GET_OBJ_TYPE(&o) == ITEM_HATCH) && \
       (!GET_OBJ_VAL(&o, VAL_DOOR_DCLOCK) || \
        !GET_OBJ_VAL(&o, VAL_DOOR_DCHIDE))) {
        GET_OBJ_VAL(&o, VAL_DOOR_DCLOCK) = 20;
        GET_OBJ_VAL(&o, VAL_DOOR_DCHIDE) = 20;
        if (bitsavetodisk) {
            dirty_item_prototypes.insert(nr);
            converting = true;
        }
    }

    /* Convert old CWG-SunTzu style armor values to CWG-Rasputin. Should no longer be needed I think.
   * if (GET_OBJ_TYPE(obj_proto + i) == ITEM_ARMOR) {
   *   if (suntzu_armor_convert(obj_proto + i)) {
   *     if(bitsavetodisk) {
   *       add_to_save_list(zone_table[real_zone_by_thing(nr)].number, 1);
   *       converting = TRUE;
   *     }
   *   }
   * }*/

    if (!get_line(obj_f, line)) {
        basic_mud_log("SYSERR: Expecting third numeric line of %s, but file ended!", buf2);
        exit(1);
    }
    if ((retval = sscanf(line, "%ld %ld %ld %ld", t, t + 1, t + 2, t + 3)) != 4) {
        if (retval == 3)
            t[3] = 0;
        else {
            basic_mud_log("SYSERR: Format error in third numeric line (expecting 4 args, got %d), %s", retval, buf2);
            exit(1);
        }
    }
    GET_OBJ_WEIGHT(&o) = t[0];
    GET_OBJ_COST(&o) = t[1];
    GET_OBJ_RENT(&o) = t[2];
    GET_OBJ_LEVEL(&o) = t[3];
    GET_OBJ_SIZE(&o) = SIZE_MEDIUM;

    /* check to make sure that weight of containers exceeds curr. quantity */
    if (GET_OBJ_TYPE(&o) == ITEM_DRINKCON ||
        GET_OBJ_TYPE(&o) == ITEM_FOUNTAIN) {
        if (GET_OBJ_WEIGHT(&o) < GET_OBJ_VAL(&o, 1))
            GET_OBJ_WEIGHT(&o) = GET_OBJ_VAL(&o, 1) + 5;
    }
    /* *** make sure portal objects have their timer set correctly *** */
    if (GET_OBJ_TYPE(&o) == ITEM_PORTAL) {
        GET_OBJ_TIMER(&o) = -1;
    }

    /* *** extra descriptions and affect fields *** */

    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        o.affected[j].location = APPLY_NONE;
        o.affected[j].modifier = 0;
        o.affected[j].specific = 0;
    }

    strcat(buf2, ", after numeric constants\n"    /* strcat: OK (for 'buf2 >= 87') */
                 "...expecting 'E', 'A', '$', or next object number");
    j = 0;

    for (;;) {
        if (!get_line(obj_f, line)) {
            basic_mud_log("SYSERR: Format error in %s", buf2);
            exit(1);
        }
        switch (*line) {
            case 'E':
                CREATE(new_descr, struct extra_descr_data, 1);
                new_descr->keyword = fread_string(obj_f, buf2);
                new_descr->description = fread_string(obj_f, buf2);
                new_descr->next = o.ex_description;
                o.ex_description = new_descr;
                break;
            case 'A':
                if (j >= MAX_OBJ_AFFECT) {
                    basic_mud_log("SYSERR: Too many A fields (%d max), %s", MAX_OBJ_AFFECT, buf2);
                    exit(1);
                }
                if (!get_line(obj_f, line)) {
                    basic_mud_log("SYSERR: Format error in 'A' field, %s\n"
                        "...expecting 2 numeric constants but file ended!", buf2);
                    exit(1);
                }

                t[1] = 0;
                if ((retval = sscanf(line, " %ld %ld %ld ", t, t + 1, t + 2)) != 3) {
                    if (retval != 2) {
                        basic_mud_log("SYSERR: Format error in 'A' field, %s\n"
                            "...expecting 2 numeric arguments, got %d\n"
                            "...offending line: '%s'", buf2, retval, line);
                        exit(1);
                    }
                }

                if (t[0] >= APPLY_DAMAGE_PERC && t[0] <= APPLY_DEFENSE_PERC) {
                    basic_mud_log("Warning: object #%d (%s) uses deprecated saving throw applies",
                        nr, GET_OBJ_SHORT(&o));
                }
                o.affected[j].location = t[0];
                o.affected[j].modifier = t[1];
                o.affected[j].specific = t[2];
                j++;
                break;
            case 'S':  /* Spells for Spellbooks*/
                if (j >= SPELLBOOK_SIZE) {
                    basic_mud_log("SYSERR: Unknown spellbook slot in S field, %s", buf2);
                    exit(1);
                }
                if (!get_line(obj_f, line)) {
                    basic_mud_log("SYSERR: Format error in 'S' field, %s\n"
                        "...expecting 2 numeric constants but file ended!", buf2);
                    exit(1);
                }

                if ((retval = sscanf(line, " %d %d ", t, t + 1)) != 2) {
                    basic_mud_log("SYSERR: Format error in 'S' field, %s\n"
                        "...expecting 2 numeric arguments, got %d\n"
                        "...offending line: '%s'", buf2, retval, line);
                    exit(1);
                }
                if (!o.sbinfo) {
                    CREATE(o.sbinfo, struct obj_spellbook_spell, SPELLBOOK_SIZE);
                }
                o.sbinfo[j].spellname = t[0];
                o.sbinfo[j].pages = t[1];
                j++;
                break;
            case 'T':  /* DG triggers */
                dg_obj_trigger(line, &o);
                break;
            case 'Z':
                if (!get_line(obj_f, line)) {
                    basic_mud_log("SYSERR: Format error in 'Z' field, %s\n"
                        "...expecting numeric constant but file ended!", buf2);
                    exit(1);
                }
                if (sscanf(line, "%d", t) != 1) {
                    basic_mud_log("SYSERR: Format error in 'Z' field, %s\n"
                        "...expecting numeric argument\n"
                        "...offending line: '%s'", buf2, line);
                    exit(1);
                }
                GET_OBJ_SIZE(&o) = t[0];
                break;
            case '$':
            case '#':
                /* Objects that set CHARM on players are bad. */
                if (OBJAFF_FLAGGED(&o, AFF_CHARM)) {
                    basic_mud_log("SYSERR: Object #%d has reserved bit AFF_CHARM set.", nr);
                    o.bitvector.reset(AFF_CHARM);
                }
                check_object(&o);
                return (line);
            default:
                basic_mud_log("SYSERR: Format error in (%c): %s", *line, buf2);
                exit(1);
        }
    }
    return line;
}

/* load the zone table and command tables */
static void load_zones(FILE *fl, char *zonename) {
    int cmd_no, num_of_cmds = 0, line_num = 0, tmp, error, arg_num, version = 1;
    char *ptr, buf[READ_SIZE], zname[READ_SIZE], buf2[MAX_STRING_LENGTH];
    int zone_fix = false;
    char t1[80], t2[80], line[MAX_STRING_LENGTH];

    strlcpy(zname, zonename, sizeof(zname));

    line_num += get_line(fl, buf);

    if (*buf == '@') {
        if (sscanf(buf, "@Version: %d", &version) != 1) {
            basic_mud_log("SYSERR: Format error in %s (version)", zname);
            basic_mud_log("SYSERR: ...Line: %s", line);
            exit(1);
        }
        line_num += get_line(fl, buf);
    }
    zone_vnum v;

    if (sscanf(buf, "#%hd", &v) != 1) {
        basic_mud_log("SYSERR: FFFFFF Format error in %s, line %d", zname, line_num);
        exit(1);
    }
    snprintf(buf2, sizeof(buf2)-1, "beginning of zone #%d", v);

    auto &z = zone_table[v];
    z.number = v;

    line_num += get_line(fl, buf);
    if ((ptr = strchr(buf, '~')) != nullptr)    /* take off the '~' if it's there */
        *ptr = '\0';
    z.builders = strdup(buf);

    line_num += get_line(fl, buf);
    if ((ptr = strchr(buf, '~')) != nullptr)    /* take off the '~' if it's there */
        *ptr = '\0';
    z.name = strdup(buf);

    line_num += get_line(fl, buf);
    if (version >= 2) {

        char zbuf1[MAX_STRING_LENGTH];
        char zbuf2[MAX_STRING_LENGTH];
        char zbuf3[MAX_STRING_LENGTH];
        char zbuf4[MAX_STRING_LENGTH];

        if (sscanf(buf, " %hd %hd %d %d %s %s %s %s %d %d", &z.bot, &z.top, &z.lifespan,
                   &z.reset_mode, zbuf1, zbuf2, zbuf3, zbuf4, &z.min_level, &z.max_level) != 10) {
            basic_mud_log("SYSERR: Format error in 10-constant line of %s", zname);
            exit(1);
        }

        z.zone_flags[0] = asciiflag_conv(zbuf1);
        z.zone_flags[1] = asciiflag_conv(zbuf2);
        z.zone_flags[2] = asciiflag_conv(zbuf3);
        z.zone_flags[3] = asciiflag_conv(zbuf4);

    } else if (sscanf(buf, " %hd %hd %d %d ", &z.bot, &z.top, &z.lifespan, &z.reset_mode) != 4) {
        /*
     * This may be due to the fact that the zone has no builder.  So, we just attempt
     * to fix this by copying the previous 2 last reads into this variable and the
     * last one.
     */
        basic_mud_log("SYSERR: Format error in numeric constant line of %s, attempting to fix.", zname);
        if (sscanf(z.name, " %hd %hd %d %d ", &z.bot, &z.top, &z.lifespan, &z.reset_mode) != 4) {
            basic_mud_log("SYSERR: Could not fix previous error, aborting game.");
            exit(1);
        } else {
            free(z.name);
            z.name = strdup(z.builders);
            free(z.builders);
            z.builders = strdup("None.");
            zone_fix = true;
        }
    }
    if (z.bot > z.top) {
        basic_mud_log("SYSERR: Zone %d bottom (%d) > top (%d).", z.number, z.bot, z.top);
        exit(1);
    }

    for (auto c = 0;true;c++) {
        get_line(fl, buf);
        if(buf[0] == '*') continue;

        if(strchr("$S", buf[0])) {
            break;
        }

        auto &zc = z.cmd.emplace_back();
        zc.command = buf[0];

        if(zc.command == 'V') { /* a string-arg command */
            if (sscanf(&buf[1], " %d %d %d %d %d %d %79s %79[^\f\r\n\t\v]", &tmp,&zc.arg1, &zc.arg2, &zc.arg3,
                       &zc.arg4, &zc.arg5, t1, t2) != 8)
                error = 1;
            else {
                zc.sarg1 = t1;
                zc.sarg2 = t2;
            }
        } else {
            if ((arg_num = sscanf(&buf[1], " %d %d %d %d %d %d ", &tmp, &zc.arg1, &zc.arg2, &zc.arg3, &zc.arg4,
                                  &zc.arg5)) != 6) {
                if (arg_num != 5) {
                    error = 1;
                } else {
                    zc.arg5 = 0;
                }
            }
        }

        zc.if_flag = tmp;

        if (error) {
            basic_mud_log("SYSERR: Format error in %s, line %d: '%s'", zname, c, buf);
            exit(1);
        }
        zc.line = c;
    }
}



static void get_one_line(FILE *fl, char *buf) {
    if (fgets(buf, READ_SIZE, fl) == nullptr) {
        basic_mud_log("SYSERR: error reading help file: not terminated with $?");
        exit(1);
    }

    buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}

void free_help(struct help_index_element *cmhelp) {
    if (cmhelp->keywords)
        free(cmhelp->keywords);
    if (cmhelp->entry && !cmhelp->duplicate)
        free(cmhelp->entry);

    free(cmhelp);
}

void free_help_table() {
    if (help_table) {
        int hp;
        for (hp = 0; hp < top_of_helpt; hp++) {
            if (help_table[hp].keywords)
                free(help_table[hp].keywords);
            if (help_table[hp].entry && !help_table[hp].duplicate)
                free(help_table[hp].entry);
        }
        free(help_table);
        help_table = nullptr;
    }
    top_of_helpt = 0;
}

void load_help(FILE *fl, char *name) {
    char key[READ_SIZE + 1], next_key[READ_SIZE + 1], entry[32384];
    size_t entrylen;
    char line[READ_SIZE + 1], hname[READ_SIZE + 1], *scan;
    struct help_index_element el;

    strlcpy(hname, name, sizeof(hname));

    get_one_line(fl, key);
    while (*key != '$') {
        strcat(key, "\r\n"); /* strcat: OK (READ_SIZE - "\n"  "\r\n" == READ_SIZE  1) */
        entrylen = strlcpy(entry, key, sizeof(entry));

        /* Read in the corresponding help entry. */
        get_one_line(fl, line);
        while (*line != '#' && entrylen < sizeof(entry) - 1) {
            entrylen += strlcpy(entry + entrylen, line, sizeof(entry) - entrylen);

            if (entrylen + 2 < sizeof(entry) - 1) {
                strcpy(entry + entrylen, "\r\n"); /* strcpy: OK (size checked above) */
                entrylen += 2;
            }
            get_one_line(fl, line);
        }

        if (entrylen >= sizeof(entry) - 1) {
            int keysize;
            const char *truncmsg = "\r\n*TRUNCATED*\r\n";

            strcpy(entry + sizeof(entry) - strlen(truncmsg) - 1,
                   truncmsg); /* strcpy: OK (assuming sane 'entry' size) */

            keysize = strlen(key) - 2;
            basic_mud_log("SYSERR: Help entry exceeded buffer space: %.*s", keysize, key);

            /* If we ran out of buffer space, eat the rest of the entry. */
            while (*line != '#')
                get_one_line(fl, line);
        }

        if (*line == '#') {
            if (sscanf(line, "#%d", &el.min_level) != 1) {
                basic_mud_log("SYSERR: Help entry does not have a min level. %s", key);
                el.min_level = 0;
            }
        }

        el.duplicate = 0;
        el.entry = strdup(entry);
        scan = one_word(key, next_key);

        while (*next_key) {
            el.keywords = strdup(next_key);
            help_table[top_of_helpt++] = el;
            el.duplicate++;
            scan = one_word(scan, next_key);
        }
        get_one_line(fl, key);
    }
}

int hsort(const void *a, const void *b) {
    const struct help_index_element *a1, *b1;

    a1 = (const struct help_index_element *) a;
    b1 = (const struct help_index_element *) b;

    return (strcasecmp(a1->keywords, b1->keywords));
}

/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*************************************************************************/


int vnum_mobile(char *searchname, struct char_data *ch) {
    int found = 0;

    for (auto &m : mob_proto)
        if (isname(searchname, m.second.name))
            send_to_char(ch, "%3d. [%5d] %-40s %s\r\n",
                         ++found, m.first, m.second.short_description,
                         !m.second.proto_script.empty() ? m.second.scriptString().c_str() : "");

    return (found);
}


int vnum_object(char *searchname, struct char_data *ch) {
    int found = 0;

    for (auto &o : obj_proto)
        if (isname(searchname, o.second.name))
            send_to_char(ch, "%3d. [%5d] %-40s %s\r\n",
                         ++found, o.first, o.second.short_description,
                         !o.second.proto_script.empty() ? o.second.scriptString().c_str() : "");

    return (found);
}


int vnum_material(char *searchname, struct char_data *ch) {
    int found = 0;

    for (auto &o : obj_proto)
        if (isname(searchname, material_names[o.second.value[VAL_ALL_MATERIAL]])) {
            send_to_char(ch, "%3d. [%5d] %-40s %s\r\n",
                         ++found, o.first, o.second.short_description,
                         !o.second.proto_script.empty() ? o.second.scriptString().c_str() : "");
        }

    return (found);
}


int vnum_weapontype(char *searchname, struct char_data *ch) {
    int found = 0;

    for (auto &o : obj_proto)
        if (o.second.type_flag == ITEM_WEAPON) {
            if (isname(searchname, weapon_type[o.second.value[VAL_WEAPON_SKILL]])) {
                send_to_char(ch, "%3d. [%5d] %-40s %s\r\n",
                             ++found, o.first, o.second.short_description,
                             !o.second.proto_script.empty() ? o.second.scriptString().c_str() : "");
            }
        }

    return (found);
}


int vnum_armortype(char *searchname, struct char_data *ch) {
    int found = 0;

    for (auto &o : obj_proto)
        if (o.second.type_flag == ITEM_ARMOR) {
            if (isname(searchname, armor_type[o.second.value[VAL_ARMOR_SKILL]])) {
                send_to_char(ch, "%3d. [%5d] %-40s %s\r\n",
                             ++found, o.first, o.second.short_description,
                             !o.second.proto_script.empty() ? o.second.scriptString().c_str() : "");
            }
        }

    return (found);
}

/* create a character, and add it to the char list */
struct char_data *create_char(bool activate) {
    auto ch = new char_data();

    if(activate) {
        ch->id = nextCharID();
        ch->generation = time(nullptr);
        check_unique_id(ch);
        add_unique_id(ch);
        ch->activate();
    }


    return ch;
}


/* create a new mobile from a prototype */
struct char_data *read_mobile(mob_vnum nr, int type) /* and mob_rnum */
{
    struct char_data *mob;

    auto proto = mob_proto.find(nr);

    if(proto == mob_proto.end()) {
        basic_mud_log("WARNING: Mobile vnum %d does not exist in database.", nr);
        return (nullptr);
    }

    mob = new char_data();

    *mob = proto->second;
    mob->id = nextCharID();
    mob->generation = time(nullptr);
    check_unique_id(mob);
    add_unique_id(mob);
    mob->activate();

    std::map<CharAppearance, int> setNumsTo;

    if (!(IS_HOSHIJIN(mob) && GET_SEX(mob) == SEX_MALE)) {
        setNumsTo[CharAppearance::HairLength] = rand_number(0, 4);
        setNumsTo[CharAppearance::HairColor] = rand_number(1, 13);
        setNumsTo[CharAppearance::HairStyle] = rand_number(1, 11);
    }

    setNumsTo[CharAppearance::EyeColor] = rand_number(0, 11);

    GET_ABSORBS(mob) = 0;
    ABSORBING(mob) = nullptr;
    ABSORBBY(mob) = nullptr;
    SITS(mob) = nullptr;
    BLOCKED(mob) = nullptr;
    BLOCKS(mob) = nullptr;

    if (!IS_HUMAN(mob) && !IS_SAIYAN(mob) && !IS_HALFBREED(mob) && !IS_NAMEK(mob)) {
        setNumsTo[CharAppearance::SkinColor] = rand_number(0, 11);
    }
    if (IS_NAMEK(mob)) {
        setNumsTo[CharAppearance::SkinColor] = 2;
    }
    if (IS_HUMAN(mob) || IS_SAIYAN(mob) || IS_HALFBREED(mob)) {
        if (rand_number(1, 5) <= 2) {
            setNumsTo[CharAppearance::SkinColor] = rand_number(0, 1);
        } else if (rand_number(1, 5) <= 4) {
            setNumsTo[CharAppearance::SkinColor] = rand_number(4, 5);
        } else if (rand_number(1, 5) <= 5) {
            setNumsTo[CharAppearance::SkinColor] = rand_number(9, 10);
        }
    }
    if (IS_SAIYAN(mob)) {
        setNumsTo[CharAppearance::HairColor] = rand_number(1, 2);
        setNumsTo[CharAppearance::EyeColor] = 1;
    }

    for(auto &[cint, val] : setNumsTo) {
        mob->set(cint, val);
    }

    if (GET_MOB_VNUM(mob) >= 81 && GET_MOB_VNUM(mob) <= 87) {
        dragon_level(mob);
    }

    int64_t mult = 0;

    switch (GET_LEVEL(mob)) {
        case 1:
            mult = rand_number(50, 80);
            break;
        case 2:
            mult = rand_number(90, 120);
            break;
        case 3:
            mult = rand_number(100, 140);
            break;
        case 4:
            mult = rand_number(120, 180);
            break;
        case 5:
            mult = rand_number(200, 250);
            break;
        case 6:
            mult = rand_number(240, 300);
            break;
        case 7:
            mult = rand_number(280, 350);
            break;
        case 8:
            mult = rand_number(320, 400);
            break;
        case 9:
            mult = rand_number(380, 480);
            break;
        case 10:
            mult = rand_number(500, 600);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            mult = rand_number(1200, 1600);
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            mult = rand_number(2400, 3000);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            mult = rand_number(5500, 8000);
            break;
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
            mult = rand_number(10000, 14000);
            break;
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
            mult = rand_number(16000, 20000);
            break;
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
            mult = rand_number(22000, 30000);
            break;
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
            mult = rand_number(50000, 70000);
            break;
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
            mult = rand_number(95000, 140000);
            break;
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
            mult = rand_number(180000, 250000);
            break;
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
            mult = rand_number(400000, 480000);
            break;
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
            mult = rand_number(700000, 900000);
            break;
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
            mult = rand_number(1400000, 1600000);
            break;
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            mult = rand_number(2200000, 2500000);
            break;
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
            mult = rand_number(3000000, 3500000);
            break;
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
            mult = rand_number(4250000, 4750000);
            break;
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
            mult = rand_number(6500000, 8500000);
            break;
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
            mult = rand_number(15000000, 18000000);
            break;
        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
            mult = rand_number(22000000, 30000000);
            break;
        case 101:
            mult = rand_number(32000000, 40000000);
            break;
        case 102:
            mult = rand_number(42000000, 55000000);
            break;
        case 103:
            mult = rand_number(80000000, 95000000);
            break;
        case 104:
            mult = rand_number(150000000, 200000000);
            break;
        case 105:
            mult = rand_number(220000000, 250000000);
            break;
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
            mult = rand_number(500000000, 750000000);
            break;
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
        case 120:
            mult = rand_number(800000000, 900000000);
            break;
        default:
            if (GET_LEVEL(mob) >= 150) {
                mult = rand_number(1500000000, 2000000000);
            } else {
                mult = rand_number(1250000000, 1500000000);
            }
            break;
    }

    GET_LPLAY(mob) = time(nullptr);
    bool autoset = mob->get(CharStat::PowerLevel) <= 1;
    if(autoset) {
        for(auto c : {CharStat::PowerLevel, CharStat::Ki, CharStat::Stamina}) {
            stat_t base = GET_LEVEL(mob) * mult;
            if (GET_LEVEL(mob) > 140) {
                base *= 8;
            } else if (GET_LEVEL(mob) > 130) {
                base *= 6;
            } else if (GET_LEVEL(mob) > 120) {
                base *= 3;
            } else if (GET_LEVEL(mob) > 110) {
                base *= 2;
            }
            mob->set(c, base);
        }
    }

    if (GET_MOB_VNUM(mob) == 2245) {
        for(auto c : {CharStat::PowerLevel, CharStat::Ki, CharStat::Stamina}) {
            mob->set(c, rand_number(1, 4));
        }
    }

    int base = 0;
    switch (GET_LEVEL(mob)) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            base = rand_number(80, 120);
            break;
        case 6:
            base = rand_number(200, 280);
            break;
        case 7:
            base = rand_number(250, 350);
            break;
        case 8:
            base = rand_number(275, 375);
            break;
        case 9:
            base = rand_number(300, 400);
            break;
        case 10:
            base = rand_number(325, 450);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            base = rand_number(500, 700);
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            base = rand_number(700, 1000);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            base = rand_number(1000, 1200);
            break;
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
            base = rand_number(1200, 1400);
            break;
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
            base = rand_number(1400, 1600);
            break;
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
            base = rand_number(1600, 1800);
            break;
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
            base = rand_number(1800, 2000);
            break;
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
            base = rand_number(2000, 2200);
            break;
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
            base = rand_number(2200, 2500);
            break;
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
            base = rand_number(2500, 2800);
            break;
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
            base = rand_number(2800, 3000);
            break;
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
            base = rand_number(3000, 3200);
            break;
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            base = rand_number(3200, 3500);
            break;
        case 76:
        case 77:
        case 78:
        case 79:
            base = rand_number(3500, 3800);
            break;
        case 80:
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
            base = rand_number(4000, 4500);
            break;
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
            base = rand_number(4500, 5500);
            break;
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
            base = rand_number(5500, 7000);
            break;
        case 96:
        case 97:
        case 98:
        case 99:
            base = rand_number(8000, 10000);
            break;
        case 100:
            base = rand_number(10000, 15000);
            break;
        case 101:
            base = rand_number(15000, 25000);
            break;
        case 102:
            base = rand_number(35000, 40000);
            break;
        case 103:
            base = rand_number(40000, 50000);
            break;
        case 104:
            base = rand_number(60000, 80000);
            break;
        case 105:
            base = rand_number(80000, 100000);
            break;
        default:
            base = rand_number(130000, 180000);
            break;
    }
    MOB_COOLDOWN(mob) = 0;
    auto money = GET_GOLD(mob);
    if (money <= 0 && !MOB_FLAGGED(mob, MOB_DUMMY)) {
        if (GET_LEVEL(mob) < 4) {
            money = GET_LEVEL(mob) * rand_number(1, 2);
        } else if (GET_LEVEL(mob) < 10) {
            money = (GET_LEVEL(mob) * rand_number(1, 2)) - 1;
        } else if (GET_LEVEL(mob) < 20) {
            money = (GET_LEVEL(mob) * rand_number(1, 3)) - 2;
        } else if (GET_LEVEL(mob) < 30) {
            money = (GET_LEVEL(mob) * rand_number(1, 3)) - 4;
        } else if (GET_LEVEL(mob) < 40) {
            money = (GET_LEVEL(mob) * rand_number(1, 3)) - 6;
        } else if (GET_LEVEL(mob) < 50) {
            money = (GET_LEVEL(mob) * rand_number(2, 3)) - 25;
        } else if (GET_LEVEL(mob) < 60) {
            money = (GET_LEVEL(mob) * rand_number(2, 3)) - 40;
        } else if (GET_LEVEL(mob) < 70) {
            money = (GET_LEVEL(mob) * rand_number(2, 3)) - 50;
        } else if (GET_LEVEL(mob) < 80) {
            money = (GET_LEVEL(mob) * rand_number(2, 4)) - 60;
        } else if (GET_LEVEL(mob) < 90) {
            money = (GET_LEVEL(mob) * rand_number(2, 4)) - 70;
        } else {
            money = (GET_LEVEL(mob) * rand_number(3, 4)) - 85;
        }
        if (!IS_HUMANOID(mob)) {
            money = GET_GOLD(mob) * 0.5;
            if (GET_GOLD(mob) <= 0)
                money = 1;
        }
        mob->set(CharMoney::Carried, money);
    }

    
    if (GET_EXP(mob) <= 0 && !MOB_FLAGGED(mob, MOB_DUMMY)) {
        int64_t mexp = GET_LEVEL(mob) * base;
        mexp = mexp * .9;
        mexp += GET_LEVEL(mob) / 2;
        mexp += GET_LEVEL(mob) / 3;
        if (IS_DRAGON(mob)) {
            mexp *= 1.4;
        } else if (IS_ANDROID(mob)) {
            mexp *= 1.25;
        } else if (IS_SAIYAN(mob)) {
            mexp *= 1.1;
        } else if (IS_BIO(mob)) {
            mexp *= 1.2;
        } else if (IS_MAJIN(mob)) {
            mexp *= 1.25;
        } else if (IS_DEMON(mob)) {
            mexp *= 1.1;
        }
        if (GET_CLASS(mob) == SenseiID::Commoner && IS_HUMANOID(mob) && !IS_DRAGON(mob)) {
            if (!IS_ANDROID(mob) && !IS_SAIYAN(mob) && !IS_BIO(mob) && !IS_MAJIN(mob)) {
                mexp *= 0.75;
            }
        }

        if (GET_LEVEL(mob) > 90) {
            mexp = mexp * .7;
        } else if (GET_LEVEL(mob) > 80) {
            mexp = mexp * .75;
        } else if (GET_LEVEL(mob) > 70) {
            mexp = mexp * .8;
        } else if (GET_LEVEL(mob) > 60) {
            mexp = mexp * .85;
        } else if (GET_LEVEL(mob) > 40) {
            mexp = mexp * .9;
        } else if (GET_LEVEL(mob) > 30) {
            mexp = mexp * .95;
        }

        if (GET_EXP(mob) > 20000000) {
            mexp = 20000000;
        }
        mob->setExperience(mexp);
    }

    mob->setAge(birth_age(mob));
    mob->time.created = mob->time.logon = time(nullptr); /* why not */
    mob->time.played = 0.0;
    mob->time.logon = time(nullptr);
    MOB_LOADROOM(mob) = NOWHERE;

    if (IS_HUMANOID(mob)) {
        for(auto f : {MOB_RARM, MOB_LARM, MOB_RLEG, MOB_LLEG}) mob->mobFlags.set(f);
    }

    copy_proto_script(&proto->second, mob, MOB_TRIGGER);
    assign_triggers(mob, MOB_TRIGGER);
    racial_body_parts(mob);

    if (GET_MOB_VNUM(mob) >= 800 && GET_MOB_VNUM(mob) <= 805) {
        number_of_assassins += 1;
    }

    return mob;
}

void add_unique_id(struct obj_data *obj) {
    if(obj->id == -1) {
        obj->id = nextObjID();
        obj->generation = time(nullptr);
        logger->warn("Object Found with ID -1. Automatically fixed to ID {}", obj->id);
    }

    auto &o = uniqueObjects[obj->id];
    o.first = obj->generation;
    o.second = obj;
}

void remove_unique_id(struct obj_data *obj) {
    uniqueObjects.erase(obj->id);
}

void log_dupe_objects(struct obj_data *obj1, struct obj_data *obj2) {
    mudlog(BRF, ADMLVL_GOD, true, "DUPE: Dupe object found: %s [%d] [%" TMT ":%" I64T "]",
           obj1->short_description ? obj1->short_description : "<No name>",
           GET_OBJ_VNUM(obj1), obj1->generation, obj1->id);
    mudlog(BRF, ADMLVL_GOD, true, "DUPE: First: In room: %d (%s), "
                                  "In object: %s, Carried by: %s, Worn by: %s",
           GET_ROOM_VNUM(IN_ROOM(obj1)),
           IN_ROOM(obj1) == NOWHERE ? "Nowhere" : obj1->getRoom()->name,
           obj1->in_obj ? obj1->in_obj->short_description : "None",
           obj1->carried_by ? GET_NAME(obj1->carried_by) : "Nobody",
           obj1->worn_by ? GET_NAME(obj1->worn_by) : "Nobody");
    mudlog(BRF, ADMLVL_GOD, true, "DUPE: Newer: In room: %d (%s), "
                                  "In object: %s, Carried by: %s, Worn by: %s",
           GET_ROOM_VNUM(IN_ROOM(obj2)),
           IN_ROOM(obj2) == NOWHERE ? "Nowhere" : obj2->getRoom()->name,
           obj2->in_obj ? obj2->in_obj->short_description : "None",
           obj2->carried_by ? GET_NAME(obj2->carried_by) : "Nobody",
           obj2->worn_by ? GET_NAME(obj2->worn_by) : "Nobody");

    // assign a new unique ID to obj2.
    obj2->id = nextObjID();
    mudlog(BRF, ADMLVL_GOD, true, "Conflicting object assigned new id: %d", obj2->id);
}

void check_unique_id(struct obj_data *obj) {
    if(obj->id == -1) {
        obj->id = nextObjID();
        obj->generation = time(nullptr);
        logger->warn("Object Found with ID -1. Automatically fixed to ID {}", obj->id);
    }
    auto find = uniqueObjects.find(obj->id);

    if(find != uniqueObjects.end() && find->second.first == obj->generation) {
        log_dupe_objects(find->second.second, obj);
    }
}

static void log_dupe_characters(struct char_data *ch1, struct char_data *ch2) {
    mudlog(BRF, ADMLVL_GOD, true, "DUPE: Dupe character found: %s [%d] [%" TMT ":%" I64T "]",
           ch1->short_description ? ch1->short_description : "<No name>",
           ch1->vn, ch1->generation, ch1->id);

    // assign a new unique ID to obj2.
    ch2->id = nextCharID();
    mudlog(BRF, ADMLVL_GOD, true, "Conflicting character assigned new id: %d", ch2->id);
}

void check_unique_id(struct char_data *ch) {
    if(ch->id == -1) {
        ch->id = nextCharID();
        ch->generation = time(nullptr);
        logger->warn("Character Found with ID -1. Automatically fixed to ID {}", ch->id);
    }
    auto find = uniqueCharacters.find(ch->id);

    if(find != uniqueCharacters.end() && find->second.first == ch->generation) {
        log_dupe_characters(find->second.second, ch);
    }
}

void add_unique_id(struct char_data *ch) {
    if(ch->id == -1) {
        ch->id = nextCharID();
        ch->generation = time(nullptr);
        logger->warn("Character Found with ID -1. Automatically fixed to ID {}", ch->id);
    }
    auto &o = uniqueCharacters[ch->id];
    o.first = ch->generation;
    o.second = ch;
}

char *sprintuniques(int low, int high) {
    return strdup("Temporarily disabled.");
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(bool activate) {
    auto obj = new obj_data();

    if(activate) {
        obj->id = nextObjID();
        obj->generation = time(nullptr);
        obj->activate();
        check_unique_id(obj);
        add_unique_id(obj);
    }

    assign_triggers(obj, OBJ_TRIGGER);

    return (obj);
}


/* create a new object from a prototype */
struct obj_data *read_object(obj_vnum nr, int type, bool activate) /* and obj_rnum */
{
    auto i = nr;
    int j;

    auto proto = obj_proto.find(i);

    if (proto == obj_proto.end()) {
        basic_mud_log("Object (%c) %d does not exist in database.", type == VIRTUAL ? 'V' : 'R', nr);
        return (nullptr);
    }
    
    auto obj = new obj_data();
    *obj = proto->second;
    OBJ_LOADROOM(obj) = NOWHERE;
    if(activate) {
        obj->id = nextObjID();
        obj->generation = time(nullptr);
        check_unique_id(obj);
        add_unique_id(obj);
        obj->activate();
    }

    if (proto->second.sbinfo) {
        CREATE(obj->sbinfo, struct obj_spellbook_spell, SPELLBOOK_SIZE);
        for (j = 0; j < SPELLBOOK_SIZE; j++) {
            obj->sbinfo[j].spellname = proto->second.sbinfo[j].spellname;
            obj->sbinfo[j].pages = proto->second.sbinfo[j].pages;
        }
    }
    copy_proto_script(&proto->second, obj, OBJ_TRIGGER);
    assign_triggers(obj, OBJ_TRIGGER);
    if (GET_OBJ_VNUM(obj) == 65) {
        HCHARGE(obj) = 20;
    }
    if (GET_OBJ_TYPE(obj) == ITEM_FOOD) {
        if (GET_OBJ_VAL(obj, 1) == 0) {
            GET_OBJ_VAL(obj, 1) = GET_OBJ_VAL(obj, VAL_FOOD_FOODVAL);
        }
        FOOB(obj) = GET_OBJ_VAL(obj, 1);
    }
    return (obj);
}


#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(uint64_t heartPulse, double deltaTime) {

    for (auto &[vn, z] : zone_table) {
        z.age += deltaTime;
        auto secs = (z.lifespan * 60);
        if(z.age < secs) continue;
        z.age -= secs;

        bool doReset = false;
        switch(z.reset_mode) {
            case 0:
                // Never reset.
            break;
            case 1:
                // reset only if zone is empty.
                if(is_empty(vn)) doReset = true;
            break;
            case 2:
                // Always reset.
                doReset = true;
            break;
            default:
                // This shouldn't happen.
                    break;
        }
        if(doReset) {
            reset_zone(vn);
            mudlog(CMP, ADMLVL_GOD, false, "Auto zone reset: %s (Zone %d)",
               z.name, vn);
        }
    }
}

static void log_zone_error(zone_rnum zone, int cmd_no, const char *message) {
    mudlog(NRM, ADMLVL_GOD, true, "SYSERR: zone file: %s", message);
    mudlog(NRM, ADMLVL_GOD, true, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
           ZCMD2.command, zone_table[zone].number, ZCMD2.line);
}

#define ZONE_ERROR(message) \
    { log_zone_error(zone, cmd_no, message); last_cmd = 0; }

/* execute the reset command table of a given zone */
void reset_zone(zone_rnum zone) {
    int cmd_no, last_cmd = 0;
    struct char_data *mob = nullptr;
    struct obj_data *obj, *obj_to;
    room_vnum rvnum;
    room_rnum rrnum;
    struct char_data *tmob = nullptr; /* for trigger assignment */
    struct obj_data *tobj = nullptr;  /* for trigger assignment */
    int mob_load = false; /* ### */
    int obj_load = false; /* ### */
    auto oproto = obj_proto.find(-1);
    auto mproto = mob_proto.find(-1);
    auto room = world.find(-1);

    auto &z = zone_table[zone];

    if (pre_reset(z.number) == false) {

        for (auto &c : z.cmd) {
            if(c.command == 'S') break;

            if (c.if_flag && !last_cmd && !mob_load && !obj_load)
                continue;

            if (!c.if_flag) { /* ### */
                mob_load = false;
                obj_load = false;
            }

            /*  This is the list of actual zone commands.  If any new
     *  zone commands are added to the game, be certain to update
     *  the list of commands in load_zone() so that the counting
     *  will still be correct. - ae.
     */
            try {
                switch (c.command) {
                case '*':            /* ignore command */
                    last_cmd = 0;
                    break;

                case 'M':            /* read a mobile */
                    room = world.find(c.arg3);
                    if (mob_proto.contains(c.arg1) && (get_vnum_count(characterVnumIndex, c.arg1) < c.arg2) && room != world.end() &&
                        (rand_number(1, 100) >= c.arg5)) {
                        int room_max = 0;
                        struct char_data *i;

                        /* First find out how many mobs of VNUM are in the mud with this rooms */
                        /* VNUM as a load point for max from room checks. */
                        /* Let's only count if room_max is in use.  If left at zero, max_in_mud will handle*/

                        if (c.arg4 > 0) {
                            for (auto i : get_vnum_list(characterVnumIndex, c.arg1)) {
                                if (MOB_LOADROOM(i) == c.arg3) {
                                    if(++room_max >= c.arg4) {
                                        // no need to keep counting more at this point...
                                        break;
                                    }
                                }
                                if (room_max >= c.arg4) {
                                    break;
                                }
                            }
                            /* Break out if room_max has been met, ignore room_max if zero */
                            if (room_max >= c.arg4) {
                                last_cmd = 0;
                                break;
                            }
                        }

                        mob = read_mobile(c.arg1, REAL);
                        /*  Set the mobs loadroom for room_max checks. */
                        MOB_LOADROOM(mob) = c.arg3;
                        char_to_room(mob, c.arg3);

                        load_mtrigger(mob);
                        tmob = mob;
                        last_cmd = 1;
                        mob_load = true;
                    } else
                        last_cmd = 0;
                    tobj = nullptr;
                    break;

                case 'O':            /* read an object */
                    room = world.find(c.arg3);
                    oproto = obj_proto.find(c.arg1);
                    if(oproto != obj_proto.end()) {
                        if(oproto->second.type_flag == ITEM_HATCH || oproto->second.type_flag == ITEM_CONTROL
                        || oproto->second.type_flag == ITEM_WINDOW || oproto->second.type_flag == ITEM_VEHICLE) {
                            c.arg2 = 1;
                            c.arg4 = 1;
                        }
                    }

                    if (obj_proto.contains(c.arg1) && get_vnum_count(objectVnumIndex, c.arg1) < c.arg2 &&
                        room != world.end() && (rand_number(1, 100) >= c.arg5)) {
                        int room_max = 0;
                        struct obj_data *k;


                        /* First find out how many obj of VNUM are in the mud with this rooms */
                        /* VNUM as a load point for max from room checks. */
                        /* Let's only count if room_max is in use.  If left at zero, max_in_mud will handle*/

                        if (c.arg4 > 0) {
                            for (auto k : get_vnum_list(objectVnumIndex, c.arg1)) {
                                if (OBJ_LOADROOM(k) == c.arg3 && (IN_ROOM(k) == c.arg3)) {
                                    if(++room_max >= c.arg4) {
                                        // no need to keep counting more at this point...
                                        break;
                                    }
                                }
                                if (room_max >= c.arg4) {
                                    /* Get rid of it if room_max has been met. */
                                    break;
                                }
                            }
                            if (room_max >= c.arg4) {
                                /* Get rid of it if room_max has been met. */
                                last_cmd = 0;
                                break;
                            }
                        }

                        obj = read_object(c.arg1, REAL);
                        obj_to_room(obj, c.arg3);
                        /* Set the loadroom for room_max checks */
                        OBJ_LOADROOM(obj) = c.arg3;

                        last_cmd = 1;
                        load_otrigger(obj);
                        tobj = obj;
                        obj_load = true;
                    } else
                        last_cmd = 0;
                    tmob = nullptr;
                    break;

                case 'P':            /* object to object */
                    if(obj_proto.contains(c.arg1) && (get_vnum_count(objectVnumIndex, c.arg1) < c.arg2) &&
                        obj_load && (rand_number(1, 100) >= c.arg5)) {

                        if (!(obj_to = get_obj_num(c.arg3))) {
                            ZONE_ERROR("target obj not found, command disabled");
                            c.command = '*';
                            break;
                        }
                        obj = read_object(c.arg1, REAL);
                        obj_to_obj(obj, obj_to);
                        last_cmd = 1;
                        load_otrigger(obj);
                        tobj = obj;
                    } else
                        last_cmd = 0;
                    tmob = nullptr;
                    break;

                case 'G':            /* obj_to_char */
                    if (!mob) {
                        ZONE_ERROR("attempt to give obj to non-existant mob, command disabled");
                        c.command = '*';
                        break;
                    }
                    if (obj_proto.contains(c.arg1) && (get_vnum_count(objectVnumIndex, c.arg1) < c.arg2) &&
                        mob_load && (rand_number(1, 100) >= c.arg5)) {
                        obj = read_object(c.arg1, REAL);
                        obj_to_char(obj, mob);
                        if (GET_MOB_SPEC(mob) != shop_keeper) {
                            randomize_eq(obj);
                        }
                        last_cmd = 1;
                        load_otrigger(obj);
                        tobj = obj;
                    } else
                        last_cmd = 0;
                    tmob = nullptr;
                    break;

                case 'E':            /* object to equipment list */
                    if (!mob) {
                        ZONE_ERROR("trying to equip non-existant mob, command disabled");
                        c.command = '*';
                        break;
                    }
                    if (obj_proto.contains(c.arg1) && (get_vnum_count(objectVnumIndex, c.arg1) < c.arg2) &&
                        mob_load && (rand_number(1, 100) >= c.arg5)) {
                        if (c.arg3 < 0 || c.arg3 >= NUM_WEARS) {
                            ZONE_ERROR("invalid equipment pos number");
                        } else {
                            obj = read_object(c.arg1, REAL);
                            IN_ROOM(obj) = IN_ROOM(mob);
                            load_otrigger(obj);
                            if (wear_otrigger(obj, mob, c.arg3)) {
                                IN_ROOM(obj) = NOWHERE;
                                equip_char(mob, obj, c.arg3);
                            } else
                                obj_to_char(obj, mob);
                            tobj = obj;
                            last_cmd = 1;
                        }
                    } else
                        last_cmd = 0;
                    tmob = nullptr;
                    break;

                case 'R': /* rem obj from room */
                    if ((obj = get_obj_in_list_num(c.arg2, world[c.arg1].contents)) != nullptr)
                        extract_obj(obj);
                    last_cmd = 1;
                    tmob = nullptr;
                    tobj = nullptr;
                    break;


                case 'D':            /* set state of door */
                    room = world.find(c.arg1);
                    if (room == world.end() || c.arg2 < 0 || c.arg2 >= NUM_OF_DIRS ||
                        (room->second.dir_option[c.arg2] == nullptr)) {
                        ZONE_ERROR("room or door does not exist, command disabled");
                        c.command = '*';
                    } else
                        switch (c.arg3) {
                            case 0:
                                REMOVE_BIT(room->second.dir_option[c.arg2]->exit_info,
                                           EX_LOCKED);
                                REMOVE_BIT(room->second.dir_option[c.arg2]->exit_info,
                                           EX_CLOSED);
                                break;
                            case 1:
                                SET_BIT(room->second.dir_option[c.arg2]->exit_info,
                                        EX_CLOSED);
                                REMOVE_BIT(room->second.dir_option[c.arg2]->exit_info,
                                           EX_LOCKED);
                                break;
                            case 2:
                                SET_BIT(room->second.dir_option[c.arg2]->exit_info,
                                        EX_LOCKED);
                                SET_BIT(room->second.dir_option[c.arg2]->exit_info,
                                        EX_CLOSED);
                                break;
                        }
                    last_cmd = 1;
                    tmob = nullptr;
                    tobj = nullptr;
                    break;

                case 'T': /* trigger command */
                    if (c.arg1 == MOB_TRIGGER && tmob) {
                        if (!SCRIPT(tmob)) tmob->script = new script_data(tmob);
                        add_trigger(SCRIPT(tmob), read_trigger(c.arg2), -1);
                        last_cmd = 1;
                    } else if (c.arg1 == OBJ_TRIGGER && tobj) {
                        if (!SCRIPT(tobj)) tobj->script = new script_data(tobj);
                        add_trigger(SCRIPT(tobj), read_trigger(c.arg2), -1);
                        last_cmd = 1;
                    } else if (c.arg1 == WLD_TRIGGER) {
                        room = world.find(c.arg3);
                        if (room == world.end()) {
                            ZONE_ERROR("Invalid room number in trigger assignment");
                        }
                        if (room->second.script) room->second.script = new script_data(&room->second);
                        add_trigger(room->second.script, read_trigger(c.arg2), -1);
                        last_cmd = 1;
                    }

                    break;

                case 'V':
                    if (c.arg1 == MOB_TRIGGER
                    && tmob) {
                        if (!SCRIPT(tmob)) {
                            ZONE_ERROR("Attempt to give a variable to scriptless mobile");
                        } else
                            add_var(&(SCRIPT(tmob)->global_vars), (char*)c.sarg1.c_str(), (char*)c.sarg2.c_str(),
                                    c.arg3);
                        last_cmd = 1;
                    } else if (c.arg1 == OBJ_TRIGGER && tobj) {
                        if (!SCRIPT(tobj)) {
                            ZONE_ERROR("Attempt to give variable to scriptless object");
                        } else
                            add_var(&(SCRIPT(tobj)->global_vars), (char*)c.sarg1.c_str(), (char*)c.sarg2.c_str(),
                                    c.arg3);
                        last_cmd = 1;
                    } else if (c.arg1 == WLD_TRIGGER) {
                        if (!world.count(c.arg3)) {
                            ZONE_ERROR("Invalid room number in variable assignment");
                        } else {
                            if (!(world[c.arg3].script)) {
                                ZONE_ERROR("Attempt to give variable to scriptless object");
                            } else
                                add_var(&(world[c.arg3].script->global_vars),
                                        (char*)c.sarg1.c_str(), (char*)c.sarg2.c_str(), c.arg2);
                            last_cmd = 1;
                        }
                    }
                    break;

                default: ZONE_ERROR("unknown cmd in reset table; cmd disabled");
                    c.command = '*';
                    break;
            }
            } catch (const std::exception &e) {
                logger->critical("Exception thrown in reset_zone '{}' line {}", zone, c.line);
                shutdown_game(1);
            }
        }

        z.age = 0;

        /* handle reset_wtrigger's */

        for(auto &rvnum : z.rooms) {
            room = world.find(rvnum);
            if(room == world.end()) continue;
            rrnum = rvnum;

            reset_wtrigger(&room->second);
            if (room->second.room_flags.test(ROOM_AURA) && rand_number(1, 5) >= 4) {
                send_to_room(rrnum, "The aura of regeneration covering the surrounding area disappears.\r\n");
                room->second.room_flags.reset(ROOM_AURA);
            }
            if (room->second.sector_type == SECT_LAVA) {
                room->second.geffect = 5;
            }
            if (room->second.geffect < -1) {
                send_to_room(rrnum, "The area loses some of the water flooding it.\r\n");
                room->second.geffect += 1;
            } else if (room->second.geffect == -1) {
                send_to_room(rrnum, "The area loses the last of the water flooding it in one large rush.\r\n");
                room->second.geffect = 0;
            }

            if(auto dmg = room->second.getDamage(); dmg > 0) {
                int toRepair = 0;
                if(dmg >= 100) toRepair = rand_number(5, 10);
                else if(dmg >= 10) toRepair = rand_number(1, 10);
                else if(dmg > 1) toRepair = rand_number(1, dmg);
                else toRepair = 1;
                room->second.modDamage(-toRepair);
                send_to_room(rrnum, "The area gets rebuilt a little.\r\n");
            }


            if (room->second.geffect >= 1 && rand_number(1, 4) == 4 && !room->second.isSunken() && room->second.sector_type != SECT_LAVA) {
                send_to_room(rrnum, "The lava has cooled and become solid rock.\r\n");
                room->second.geffect = 0;
            } else if (room->second.geffect >= 1 && rand_number(1, 2) == 2 && room->second.isSunken() &&
                    room->second.sector_type != SECT_LAVA) {
                send_to_room(rrnum, "The water has cooled the lava and it has become solid rock.\r\n");
                room->second.geffect = 0;
            }
        }
    } else {
        /* even if reset is blocked, age should be reset */
        z.age = 0;
    }
    post_reset(z.number);
}


/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(zone_rnum zone_nr) {
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING)
            continue;
        if (IN_ROOM(i->character) == NOWHERE)
            continue;
        if (i->character->getRoom()->zone != zone_nr)
            continue;
        /*
     * if an immortal has nohassle off, he counts as present
     * added for testing zone reset triggers - Welcor
     */
        if (IS_NPC(i->character))
            continue; /* immortal switched into a mob */

        if ((GET_ADMLEVEL(i->character) >= ADMLVL_IMMORT) && (PRF_FLAGGED(i->character, PRF_NOHASSLE)))
            continue;

        return (0);
    }

    return (1);
}


/************************************************************************
*  funcs of a (more or less) general utility nature			*
************************************************************************/


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl, const char *error) {
    char buf[MAX_STRING_LENGTH], tmp[520];
    char *point;
    int done = 0, length = 0, templength;

    *buf = *tmp = '\0';

    do {
        if (!fgets(tmp, 512, fl)) {
            basic_mud_log("SYSERR: fread_string: format error at string (pos %ld): %s at or near %s",
                ftell(fl), feof(fl) ? "EOF" : ferror(fl) ? "read error" : "unknown error", error);
            exit(1);
        }
        /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
        /* now only removes trailing ~'s -- Welcor */
        for (point = tmp; *point && *point != '\r' && *point != '\n'; point++);
        if (point > tmp && point[-1] == '~') {
            *(--point) = '\0';
            done = 1;
        } else {
            *point = '\r';
            *(++point) = '\n';
            *(++point) = '\0';
        }

        templength = point - tmp;

        if (length + templength >= MAX_STRING_LENGTH) {
            basic_mud_log("SYSERR: fread_string: string too large (db.c)");
            basic_mud_log("%s", error);
            exit(1);
        } else {
            strcat(buf + length, tmp);    /* strcat: OK (size checked above) */
            length += templength;
        }
    } while (!done);

    /* allocate space for the new string and copy it */
    return (strlen(buf) ? strdup(buf) : nullptr);
}

/* Called to free all allocated follow_type structs - Update by Jamie Nelson */
void free_followers(struct follow_type *k) {
    if (!k)
        return;

    if (k->next)
        free_followers(k->next);

    k->follower = nullptr;
    free(k);
}


/* release memory allocated for a char struct */
void free_char(struct char_data *ch) {
    int i;
    uniqueCharacters.erase(ch->id);

    if(ch->vn == NOBODY) {
        if (GET_NAME(ch))
            free(GET_NAME(ch));
        if (GET_VOICE(ch))
            free(GET_VOICE(ch));
        if (GET_CLAN(ch))
            free(GET_CLAN(ch));
        if (ch->title)
            free(ch->title);
        if (ch->short_description)
            free(ch->short_description);
        if (ch->room_description)
            free(ch->room_description);
        if (ch->look_description)
            free(ch->look_description);

    } else {
        auto &m = mob_proto[ch->vn];
        if (ch->name && ch->name != m.name)
            free(ch->name);
        if (ch->title && ch->title != m.title)
            free(ch->title);
        if (ch->short_description && ch->short_description != m.short_description)
            free(ch->short_description);
        if (ch->room_description && ch->room_description != m.room_description)
            free(ch->room_description);
        if (ch->look_description && ch->look_description != m.look_description)
            free(ch->look_description);
    }

    while (ch->affected)
        affect_remove(ch, ch->affected);

    /* free any assigned scripts */
    if (SCRIPT(ch))
        extract_script(ch, MOB_TRIGGER);

    /* new version of free_followers take the followers pointer as arg */
    free_followers(ch->followers);

    if (ch->desc)
        ch->desc->character = nullptr;

    /* find_char helper */
    /*
  * when free_char is called with a blank character struct, ID is set
  * to 0, and has not yet been added to the lookup table.
  */

    delete ch;
}


/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj) {
    remove_unique_id(obj);
    if (GET_OBJ_RNUM(obj) == NOWHERE) {
        free_object_strings(obj);
    } else {
        free_object_strings_proto(obj);

    }

    /* Let's make sure that we free up this memory */
    if (obj->auctname) {
        free(obj->auctname);
    }

    /* free any assigned scripts */
    if (SCRIPT(obj))
        extract_script(obj, OBJ_TRIGGER);

    if (obj->sbinfo)
        free(obj->sbinfo);

    delete obj;
}

/*
 * Steps:
 *   1: Read contents of a text file.
 *   2: Make sure no one is using the pointer in paging.
 *   3: Allocate space.
 *   4: Point 'buf' to it.
 *
 * We don't want to free() the string that someone may be
 * viewing in the pager.  page_string() keeps the internal
 * strdup()'d copy on ->showstr_head and it won't care
 * if we delete the original.  Otherwise, strings are kept
 * on ->showstr_vector but we'll only match if the pointer
 * is to the string we're interested in and not a copy.
 *
 * If someone is reading a global copy we're trying to
 * replace, give everybody using it a different copy so
 * as to avoid special cases.
 */
static int file_to_string_alloc(const char *name, char **buf) {
    int temppage;
    char temp[MAX_STRING_LENGTH];

    /* Lets not free() what used to be there unless we succeeded. */
    if (file_to_string(name, temp) < 0)
        return (-1);

    if (*buf)
        free(*buf);

    *buf = strdup(temp);
    return (0);
}


/* read contents of a text file, and place in buf */
static int file_to_string(const char *name, char *buf) {
    try {
        std::ifstream f(name);
        std::string out;
        // read all of f to out.
        std::getline(f, out, '\0');
        boost::trim_right(out);
        strcpy(buf, out.c_str());
        return 0;
    } catch (std::exception &e) {
        return -1;
    }
}


/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch) {
    int i;

    ch->followers = nullptr;
    ch->master = nullptr;
    IN_ROOM(ch) = NOWHERE;
    ch->next = nullptr;
    ch->next_fighting = nullptr;
    ch->next_in_room = nullptr;
    FIGHTING(ch) = nullptr;
    ch->position = POS_STANDING;
    ch->mob_specials.default_pos = POS_STANDING;
    ch->time.logon = time(nullptr);

    GET_LAST_TELL(ch) = NOBODY;
}


/*
 * Called during character creation after picking character class
 * (and then never again for that character).
 */
void init_char(struct char_data *ch) {
    int i;

    GET_CRANK(ch) = 0;
    GET_CLAN(ch) = strdup("None.");
    GET_ABSORBS(ch) = 0;
    ABSORBING(ch) = nullptr;
    ABSORBBY(ch) = nullptr;
    SITS(ch) = nullptr;
    BLOCKED(ch) = nullptr;
    BLOCKS(ch) = nullptr;

    /* If this is our first player make him LVL_IMPL. */
    if (players.size() == 0) {
        admin_set(ch, ADMLVL_IMPL);

        /* The implementor never goes through do_start(). */
        for(auto c : {CharStat::PowerLevel, CharStat::Ki, CharStat::Stamina}) {
            ch->set(c, 1000);
        }
    }

    set_title(ch, nullptr);
    ch->short_description = nullptr;
    ch->room_description = nullptr;
    ch->look_description = nullptr;

    /*ch->time.birth = time(0) - birth_age(ch);*/
    ch->time.logon = ch->time.created = time(nullptr);
    ch->time.played = 0.0;

    GET_HOME(ch) = 1;

    set_height_and_weight_by_race(ch);

    for (i = 1; i < SKILL_TABLE_SIZE; i++) {
        if (GET_ADMLEVEL(ch) < ADMLVL_IMPL)
            SET_SKILL(ch, i, 0);
        else
            SET_SKILL(ch, i, 100);
        SET_SKILL_BONUS(ch, i, 0);
    }

    for (i = 0; i < AF_ARRAY_MAX; i++)
        AFF_FLAGS(ch)[i] = 0;

    for (i = 0; i < 3; i++)
        GET_SAVE_MOD(ch, i) = 0;

    for (i = 0; i < 3; i++)
        GET_COND(ch, i) = (GET_ADMLEVEL(ch) == ADMLVL_IMPL ? -1 : 24);

    GET_LOADROOM(ch) = NOWHERE;
    SPEAKING(ch) = SKILL_LANG_COMMON;
}

/* returns the real number of the room with given virtual number */
room_rnum real_room(room_vnum vnum) {
    return world.count(vnum) ? vnum : NOWHERE;
}


/* returns the real number of the monster with given virtual number */
mob_rnum real_mobile(mob_vnum vnum) {
    return mob_proto.count(vnum) ? vnum : NOBODY;
}


/* returns the real number of the object with given virtual number */
obj_rnum real_object(obj_vnum vnum) {
    return obj_proto.count(vnum) ? vnum : NOTHING;
}

/* returns the real number of the room with given virtual number */
zone_rnum real_zone(zone_vnum vnum) {
    return zone_table.count(vnum) ? vnum : NOWHERE;
}


/*
 * Extend later to include more checks.
 *
 * TODO: Add checks for unknown bitvectors.
 */
static int check_object(struct obj_data *obj) {
    char objname[MAX_INPUT_LENGTH + 32];
    int error = false, y;

    if (GET_OBJ_WEIGHT(obj) < 0 && (error = true))
        basic_mud_log("SYSERR: Object #%d (%s) has negative weight (%" I64T ").",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_WEIGHT(obj));

    if (GET_OBJ_RENT(obj) < 0 && (error = true))
        basic_mud_log("SYSERR: Object #%d (%s) has negative cost/day (%d).",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_RENT(obj));

    snprintf(objname, sizeof(objname), "Object #%d (%s)", GET_OBJ_VNUM(obj), obj->short_description);
    for (y = 0; y < TW_ARRAY_MAX; y++) {
        error |= check_bitvector_names(GET_OBJ_WEAR(obj)[y], wear_bits_count, objname, "object wear");
        error |= check_bitvector_names(GET_OBJ_EXTRA(obj)[y], extra_bits_count, objname, "object extra");
        error |= check_bitvector_names(GET_OBJ_PERM(obj)[y], affected_bits_count, objname, "object affect");
    }

    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_DRINKCON: {
            char onealias[MAX_INPUT_LENGTH], *space = strrchr(obj->name, ' ');

            strlcpy(onealias, space ? space + 1 : obj->name, sizeof(onealias));
            if (search_block(onealias, drinknames, true) < 0 && (error = true)) {
                //log("SYSERR: Object #%d (%s) doesn't have drink type as last alias. (%s)", GET_OBJ_VNUM(obj), obj->short_description, obj->name);
            }
        }
            /* Fall through. */
        case ITEM_FOUNTAIN:
            if ((GET_OBJ_VAL(obj, 0) > 0) && (GET_OBJ_VAL(obj, 1) > GET_OBJ_VAL(obj, 0) && (error = true)))
                basic_mud_log("SYSERR: Object #%d (%s) contains (%d) more than maximum (%d).",
                    GET_OBJ_VNUM(obj), obj->short_description,
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 0));
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
            error |= check_object_level(obj, 0);
            error |= check_object_spell_number(obj, 1);
            error |= check_object_spell_number(obj, 2);
            error |= check_object_spell_number(obj, 3);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            error |= check_object_level(obj, 0);
            error |= check_object_spell_number(obj, 3);
            if (GET_OBJ_VAL(obj, 2) > GET_OBJ_VAL(obj, 1) && (error = true))
                basic_mud_log("SYSERR: Object #%d (%s) has more charges (%d) than maximum (%d).",
                    GET_OBJ_VNUM(obj), obj->short_description,
                    GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 1));
            break;
    }

    return (error);
}


static int check_object_spell_number(struct obj_data *obj, int val) {
    int error = false;
    const char *spellname;

    if (GET_OBJ_VAL(obj, val) == -1 ||
        GET_OBJ_VAL(obj, val) == 0)    /* i.e.: no spell */
        return (error);

    /*
   * Check for negative spells, spells beyond the top define, and any
   * spell which is actually a skill.
   */
    if (GET_OBJ_VAL(obj, val) < 0)
        error = true;
    if (GET_OBJ_VAL(obj, val) >= SKILL_TABLE_SIZE)
        error = true;
    if (skill_type(GET_OBJ_VAL(obj, val)) != SKTYPE_SPELL)
        error = true;
    if (error)
        basic_mud_log("SYSERR: Object #%d (%s) has out of range spell #%d.",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));

    /*
   * This bug has been fixed, but if you don't like the special behavior...
   */
#if 0
                                                                                                                            if (GET_OBJ_TYPE(obj) == ITEM_STAFF &&
	HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, val), MAG_AREAS | MAG_MASSES))
    log("... '%s' (#%d) uses %s spell '%s'.",
	obj->short_description,	GET_OBJ_VNUM(obj),
	HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, val), MAG_AREAS) ? "area" : "mass",
	skill_name(GET_OBJ_VAL(obj, val)));
#endif

    if (scheck)        /* Spell names don't exist in syntax check mode. */
        return (error);

    /* Now check for unnamed spells. */
    spellname = skill_name(GET_OBJ_VAL(obj, val));

    if ((spellname == unused_spellname || !strcasecmp("UNDEFINED", spellname)) && (error = true))
        basic_mud_log("SYSERR: Object #%d (%s) uses '%s' spell #%d.",
            GET_OBJ_VNUM(obj), obj->short_description, spellname,
            GET_OBJ_VAL(obj, val));

    return (error);
}

static int check_object_level(struct obj_data *obj, int val) {
    int error = false;

    if ((GET_OBJ_VAL(obj, val) < 0) && (error = true))
        basic_mud_log("SYSERR: Object #%d (%s) has out of range level #%d.",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));

    return (error);
}

static int check_bitvector_names(bitvector_t bits, size_t namecount, const char *whatami, const char *whatbits) {
    unsigned int flagnum;
    bool error = false;

    /* See if any bits are set above the ones we know about. */
    if (bits <= (~(bitvector_t) 0 >> (sizeof(bitvector_t) * 8 - namecount)))
        return (false);

    for (flagnum = namecount; flagnum < sizeof(bitvector_t) * 8; flagnum++)
        if ((1 << flagnum) & bits) {
            basic_mud_log("SYSERR: %s has unknown %s flag, bit %d (0 through %" SZT " known).", whatami, whatbits, flagnum,
                namecount - 1);
            error = true;
        }

    return (error);
}

int my_obj_save_to_disk(FILE *fp, struct obj_data *obj, int locate) {
    int counter2, i;
    struct extra_descr_data *ex_desc;
    char buf1[MAX_STRING_LENGTH + 1];
    char ebuf0[MAX_STRING_LENGTH], ebuf1[MAX_STRING_LENGTH];
    char ebuf2[MAX_STRING_LENGTH], ebuf3[MAX_STRING_LENGTH];


    if (obj->look_description) {
        strcpy(buf1, obj->look_description);
        strip_string(buf1);
    } else
        *buf1 = 0;

    sprintascii(ebuf0, GET_OBJ_EXTRA(obj)[0]);
    sprintascii(ebuf1, GET_OBJ_EXTRA(obj)[1]);
    sprintascii(ebuf2, GET_OBJ_EXTRA(obj)[2]);
    sprintascii(ebuf3, GET_OBJ_EXTRA(obj)[3]);

    fprintf(fp,
            "#%d\n"
            "%ld %ld %ld %ld %ld %ld %ld %ld %ld %s %s %s %s %ld %ld %ld %ld %ld %ld %d %d\n",
            GET_OBJ_VNUM(obj), locate, GET_OBJ_VAL(obj, 0), GET_OBJ_VAL(obj, 1),
            GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 4),
            GET_OBJ_VAL(obj, 5), GET_OBJ_VAL(obj, 6), GET_OBJ_VAL(obj, 7),
            ebuf0, ebuf1, ebuf2, ebuf3,
            GET_OBJ_VAL(obj, 8), GET_OBJ_VAL(obj, 9),
            GET_OBJ_VAL(obj, 10), GET_OBJ_VAL(obj, 11), GET_OBJ_VAL(obj, 12),
            GET_OBJ_VAL(obj, 13), GET_OBJ_VAL(obj, 14), GET_OBJ_VAL(obj, 15));

    if (!(OBJ_FLAGGED(obj, ITEM_UNIQUE_SAVE)) && !GET_OBJ_TYPE(obj) == ITEM_SPELLBOOK) {
        return 1;
    }

    fprintf(fp,
            "XAP\n"
            "%s~\n"
            "%s~\n"
            "%s~\n"
            "%s~\n"
            "%d %d %d %d %d %" I64T " %d %d\n", obj->name ? obj->name : "undefined",
            obj->short_description ? obj->short_description : "undefined",
            obj->room_description ? obj->room_description : "undefined",
            buf1, GET_OBJ_TYPE(obj), GET_OBJ_WEAR(obj)[0],
            GET_OBJ_WEAR(obj)[1], GET_OBJ_WEAR(obj)[2], GET_OBJ_WEAR(obj)[3],
            GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj));

    fprintf(fp, "G\n%ld\n", obj->generation);
    fprintf(fp, "U\n%" I64T "\n", obj->id);

    fprintf(fp, "Z\n%d\n", GET_OBJ_SIZE(obj));

    /* Do we have affects? */
    for (counter2 = 0; counter2 < MAX_OBJ_AFFECT; counter2++)
        if (obj->affected[counter2].location != APPLY_NONE)
            fprintf(fp, "A\n"
                        "%d %d %d\n",
                    obj->affected[counter2].location, obj->affected[counter2].modifier,
                    obj->affected[counter2].specific);

    /* Do we have extra descriptions? */
    if (obj->ex_description) {        /*. Yep, save them too . */
        for (ex_desc = obj->ex_description; ex_desc; ex_desc = ex_desc->next) {
            /*. Sanity check to prevent nasty protection faults . */
            if (!*ex_desc->keyword || !*ex_desc->description) {
                continue;
            }
            strcpy(buf1, ex_desc->description);
            strip_string(buf1);
            fprintf(fp, "E\n"
                        "%s~\n"
                        "%s~\n",
                    ex_desc->keyword,
                    buf1);
        }
    }

    /* Do we have spells? */
    if (obj->sbinfo) {        /*. Yep, save them too . */
        for (i = 0; i < SPELLBOOK_SIZE; i++) {
            if (obj->sbinfo[i].spellname == 0) {
                break;
            }
            fprintf(fp, "S\n" "%d %d\n", obj->sbinfo[i].spellname, obj->sbinfo[i].pages);
        }
    }
    return 1;
}

/* This procedure removes the '\r\n' from a string so that it may be
   saved to a file.  Use it only on buffers, not on the orginal
   strings. */

void strip_string(char *buffer) {
    char *ptr, *str;

    ptr = buffer;
    str = ptr;

    while ((*str = *ptr)) {
        str++;
        ptr++;
        if (*ptr == '\r')
            ptr++;
    }
}

/* External variables from config.c */

void load_default_config() {
    /****************************************************************************/
    /** This function is called only once, at boot-time.                       **/
    /** - We assume config_info is empty                          -- Welcor    **/
    /****************************************************************************/
    /****************************************************************************/
    /** Game play options.                                                     **/
    /****************************************************************************/
    CONFIG_PK_ALLOWED = pk_allowed;
    CONFIG_PT_ALLOWED = pt_allowed;
    CONFIG_LEVEL_CAN_SHOUT = level_can_shout;
    CONFIG_HOLLER_MOVE_COST = holler_move_cost;
    CONFIG_TUNNEL_SIZE = tunnel_size;
    CONFIG_MAX_EXP_GAIN = max_exp_gain;
    CONFIG_MAX_EXP_LOSS = max_exp_loss;
    CONFIG_MAX_NPC_CORPSE_TIME = max_npc_corpse_time;
    CONFIG_MAX_PC_CORPSE_TIME = max_pc_corpse_time;
    CONFIG_IDLE_VOID = idle_void;
    CONFIG_IDLE_RENT_TIME = idle_rent_time;
    CONFIG_IDLE_MAX_LEVEL = idle_max_level;
    CONFIG_DTS_ARE_DUMPS = dts_are_dumps;
    CONFIG_LOAD_INVENTORY = load_into_inventory;
    CONFIG_OK = strdup(OK);
    CONFIG_NOPERSON = strdup(NOPERSON);
    CONFIG_NOEFFECT = strdup(NOEFFECT);
    CONFIG_TRACK_T_DOORS = track_through_doors;
    CONFIG_LEVEL_CAP = level_cap;
    CONFIG_STACK_MOBS = show_mob_stacking;
    CONFIG_STACK_OBJS = show_obj_stacking;
    CONFIG_MOB_FIGHTING = mob_fighting;
    CONFIG_DISP_CLOSED_DOORS = disp_closed_doors;
    CONFIG_REROLL_PLAYER_CREATION = reroll_status;
    CONFIG_INITIAL_POINTS_POOL = initial_points;
    CONFIG_ENABLE_COMPRESSION = enable_compression;
    CONFIG_ENABLE_LANGUAGES = enable_languages;
    CONFIG_ALL_ITEMS_UNIQUE = all_items_unique;
    CONFIG_EXP_MULTIPLIER = exp_multiplier;
    /****************************************************************************/
    /** Rent / crashsave options.                                              **/
    /****************************************************************************/
    CONFIG_FREE_RENT = free_rent;
    CONFIG_MAX_OBJ_SAVE = max_obj_save;
    CONFIG_MIN_RENT_COST = min_rent_cost;
    CONFIG_AUTO_SAVE = auto_save;
    CONFIG_AUTOSAVE_TIME = autosave_time;
    CONFIG_CRASH_TIMEOUT = crash_file_timeout;
    CONFIG_RENT_TIMEOUT = rent_file_timeout;

    /****************************************************************************/
    /** Room numbers.                                                          **/
    /****************************************************************************/
    CONFIG_MORTAL_START = mortal_start_room;
    CONFIG_IMMORTAL_START = immort_start_room;
    CONFIG_FROZEN_START = frozen_start_room;
    CONFIG_DON_ROOM_1 = donation_room_1;
    CONFIG_DON_ROOM_2 = donation_room_2;
    CONFIG_DON_ROOM_3 = donation_room_3;

    /****************************************************************************/
    /** Game operation options.                                                **/
    /****************************************************************************/
    CONFIG_DFLT_PORT = DFLT_PORT;

    if (DFLT_IP)
        CONFIG_DFLT_IP = strdup(DFLT_IP);
    else
        CONFIG_DFLT_IP = nullptr;

    CONFIG_DFLT_DIR = strdup(DFLT_DIR);

    if (LOGNAME)
        CONFIG_LOGNAME = strdup(LOGNAME);
    else
        CONFIG_LOGNAME = nullptr;

    CONFIG_MAX_PLAYING = max_playing;
    CONFIG_MAX_FILESIZE = max_filesize;
    CONFIG_MAX_BAD_PWS = max_bad_pws;
    CONFIG_SITEOK_ALL = siteok_everyone;
    CONFIG_NS_IS_SLOW = nameserver_is_slow;
    CONFIG_NEW_SOCIALS = use_new_socials;
    CONFIG_OLC_SAVE = auto_save_olc;
    CONFIG_MENU = strdup(MENU);
    CONFIG_WELC_MESSG = strdup(WELC_MESSG);
    CONFIG_START_MESSG = strdup(START_MESSG);
    CONFIG_IMC_ENABLED = imc_is_enabled;
    CONFIG_EXP_MULTIPLIER = 1.0;

    /****************************************************************************/
    /** Autowiz options.                                                       **/
    /****************************************************************************/
    CONFIG_USE_AUTOWIZ = use_autowiz;
    CONFIG_MIN_WIZLIST_LEV = min_wizlist_lev;

    /****************************************************************************/
    /** Character advancement options.                                         **/
    /****************************************************************************/
    CONFIG_ALLOW_MULTICLASS = allow_multiclass;
    CONFIG_ALLOW_PRESTIGE = allow_prestige;

    /****************************************************************************/
    /** ticks menu                                                             **/
    /****************************************************************************/
    CONFIG_PULSE_VIOLENCE = pulse_violence;
    CONFIG_PULSE_MOBILE = pulse_mobile;
    CONFIG_PULSE_ZONE = pulse_zone;
    CONFIG_PULSE_CURRENT = pulse_current;
    CONFIG_PULSE_SANITY = pulse_sanity;
    CONFIG_PULSE_IDLEPWD = pulse_idlepwd;
    CONFIG_PULSE_AUTOSAVE = pulse_autosave;
    CONFIG_PULSE_USAGE = pulse_usage;
    CONFIG_PULSE_TIMESAVE = pulse_timesave;

    /****************************************************************************/
    /** Character Creation Method                                              **/
    /****************************************************************************/
    CONFIG_CREATION_METHOD = method;
}

void load_config() {
    FILE *fl;
    char line[MAX_STRING_LENGTH];
    char tag[MAX_INPUT_LENGTH];
    int num;
    float fum;
    char buf[MAX_INPUT_LENGTH];

    load_default_config();

    snprintf(buf, sizeof(buf), "%s/%s", DFLT_DIR, CONFIG_CONFFILE);
    if (!(fl = fopen(CONFIG_CONFFILE, "r")) && !(fl = fopen(buf, "r"))) {
        snprintf(buf, sizeof(buf), "Game Config File: %s", CONFIG_CONFFILE);
        perror(buf);
        return;
    }

    /****************************************************************************/
    /** Load the game configuration file.                                      **/
    /****************************************************************************/
    while (get_line(fl, line)) {
        split_argument(line, tag);
        num = atoi(line);
        fum = atof(line);

        switch (LOWER(*tag)) {
            case 'a':
                if (!strcasecmp(tag, "auto_save"))
                    CONFIG_AUTO_SAVE = num;
                else if (!strcasecmp(tag, "autosave_time"))
                    CONFIG_AUTOSAVE_TIME = num;
                else if (!strcasecmp(tag, "auto_save_olc"))
                    CONFIG_OLC_SAVE = num;
                else if (!strcasecmp(tag, "allow_multiclass"))
                    CONFIG_ALLOW_MULTICLASS = num;
                else if (!strcasecmp(tag, "allow_prestige"))
                    CONFIG_ALLOW_PRESTIGE = num;
                else if (!strcasecmp(tag, "auto_level"))
                    basic_mud_log("ignoring obsolete config option auto_level");
                else if (!strcasecmp(tag, "all_items_unique"))
                    CONFIG_ALL_ITEMS_UNIQUE = num;
                break;

            case 'c':
                if (!strcasecmp(tag, "crash_file_timeout"))
                    CONFIG_CRASH_TIMEOUT = num;
                else if (!strcasecmp(tag, "compression")) {
                    CONFIG_ENABLE_COMPRESSION = num;
                }
                break;

            case 'd':
                if (!strcasecmp(tag, "disp_closed_doors"))
                    CONFIG_DISP_CLOSED_DOORS = num;
                else if (!strcasecmp(tag, "dts_are_dumps"))
                    CONFIG_DTS_ARE_DUMPS = num;
                else if (!strcasecmp(tag, "donation_room_1"))
                    if (num == -1)
                        CONFIG_DON_ROOM_1 = NOWHERE;
                    else
                        CONFIG_DON_ROOM_1 = num;
                else if (!strcasecmp(tag, "donation_room_2"))
                    if (num == -1)
                        CONFIG_DON_ROOM_2 = NOWHERE;
                    else
                        CONFIG_DON_ROOM_2 = num;
                else if (!strcasecmp(tag, "donation_room_3"))
                    if (num == -1)
                        CONFIG_DON_ROOM_3 = NOWHERE;
                    else
                        CONFIG_DON_ROOM_3 = num;
                else if (!strcasecmp(tag, "dflt_dir")) {
                    if (CONFIG_DFLT_DIR)
                        free(CONFIG_DFLT_DIR);
                    if (line != nullptr && *line)
                        CONFIG_DFLT_DIR = strdup(line);
                    else
                        CONFIG_DFLT_DIR = strdup(DFLT_DIR);
                } else if (!strcasecmp(tag, "dflt_ip")) {
                    if (CONFIG_DFLT_IP)
                        free(CONFIG_DFLT_IP);
                    if (line != nullptr && *line)
                        CONFIG_DFLT_IP = strdup(line);
                    else
                        CONFIG_DFLT_IP = nullptr;
                } else if (!strcasecmp(tag, "dflt_port"))
                    CONFIG_DFLT_PORT = num;
                break;

            case 'e':
                if (!strcasecmp(tag, "enable_languages"))
                    CONFIG_ENABLE_LANGUAGES = num;
                else if (!strcasecmp(tag, "exp_multiplier"))
                    CONFIG_EXP_MULTIPLIER = fum;
                break;

            case 'f':
                if (!strcasecmp(tag, "free_rent"))
                    CONFIG_FREE_RENT = num;
                else if (!strcasecmp(tag, "frozen_start_room"))
                    CONFIG_FROZEN_START = num;
                break;

            case 'h':
                if (!strcasecmp(tag, "holler_move_cost"))
                    CONFIG_HOLLER_MOVE_COST = num;
                break;

            case 'i':
                if (!strcasecmp(tag, "idle_void"))
                    CONFIG_IDLE_VOID = num;
                else if (!strcasecmp(tag, "idle_rent_time"))
                    CONFIG_IDLE_RENT_TIME = num;
                else if (!strcasecmp(tag, "idle_max_level")) {
                    if (num >= CONFIG_LEVEL_CAP)
                        num += 1 - CONFIG_LEVEL_CAP;
                    CONFIG_IDLE_MAX_LEVEL = num;
                } else if (!strcasecmp(tag, "immort_level_ok"))
                    basic_mud_log("Ignoring immort_level_ok obsolete config");
                else if (!strcasecmp(tag, "immort_start_room"))
                    CONFIG_IMMORTAL_START = num;
                else if (!strcasecmp(tag, "imc_enabled"))
                    CONFIG_IMC_ENABLED = num;
                else if (!strcasecmp(tag, "initial_points"))
                    CONFIG_INITIAL_POINTS_POOL = num;
                break;

            case 'l':
                if (!strcasecmp(tag, "level_can_shout"))
                    CONFIG_LEVEL_CAN_SHOUT = num;
                else if (!strcasecmp(tag, "level_cap"))
                    CONFIG_LEVEL_CAP = num;
                else if (!strcasecmp(tag, "load_into_inventory"))
                    CONFIG_LOAD_INVENTORY = num;
                else if (!strcasecmp(tag, "logname")) {
                    if (CONFIG_LOGNAME)
                        free(CONFIG_LOGNAME);
                    if (line != nullptr && *line)
                        CONFIG_LOGNAME = strdup(line);
                    else
                        CONFIG_LOGNAME = nullptr;
                }
                break;

            case 'm':
                if (!strcasecmp(tag, "max_bad_pws"))
                    CONFIG_MAX_BAD_PWS = num;
                else if (!strcasecmp(tag, "max_exp_gain"))
                    CONFIG_MAX_EXP_GAIN = num;
                else if (!strcasecmp(tag, "max_exp_loss"))
                    CONFIG_MAX_EXP_LOSS = num;
                else if (!strcasecmp(tag, "max_filesize"))
                    CONFIG_MAX_FILESIZE = num;
                else if (!strcasecmp(tag, "max_npc_corpse_time"))
                    CONFIG_MAX_NPC_CORPSE_TIME = num;
                else if (!strcasecmp(tag, "max_obj_save"))
                    CONFIG_MAX_OBJ_SAVE = num;
                else if (!strcasecmp(tag, "max_pc_corpse_time"))
                    CONFIG_MAX_PC_CORPSE_TIME = num;
                else if (!strcasecmp(tag, "max_playing"))
                    CONFIG_MAX_PLAYING = num;
                else if (!strcasecmp(tag, "menu")) {
                    if (CONFIG_MENU)
                        free(CONFIG_MENU);
                    strncpy(buf, "Reading menu in load_config()", sizeof(buf));
                    CONFIG_MENU = fread_string(fl, buf);
                } else if (!strcasecmp(tag, "min_rent_cost"))
                    CONFIG_MIN_RENT_COST = num;
                else if (!strcasecmp(tag, "min_wizlist_lev")) {
                    if (num >= CONFIG_LEVEL_CAP)
                        num += 1 - CONFIG_LEVEL_CAP;
                    CONFIG_MIN_WIZLIST_LEV = num;
                } else if (!strcasecmp(tag, "mob_fighting"))
                    CONFIG_MOB_FIGHTING = num;
                else if (!strcasecmp(tag, "mortal_start_room"))
                    CONFIG_MORTAL_START = num;
                else if (!strcasecmp(tag, "method"))
                    CONFIG_CREATION_METHOD = num;
                break;

            case 'n':
                if (!strcasecmp(tag, "nameserver_is_slow"))
                    CONFIG_NS_IS_SLOW = num;
                else if (!strcasecmp(tag, "noperson")) {
                    char tmp[READ_SIZE];
                    if (CONFIG_NOPERSON)
                        free(CONFIG_NOPERSON);
                    snprintf(tmp, sizeof(tmp), "%s\r\n", line);
                    CONFIG_NOPERSON = strdup(tmp);
                } else if (!strcasecmp(tag, "noeffect")) {
                    char tmp[READ_SIZE];
                    if (CONFIG_NOEFFECT)
                        free(CONFIG_NOEFFECT);
                    snprintf(tmp, sizeof(tmp), "%s\r\n", line);
                    CONFIG_NOEFFECT = strdup(tmp);
                }
                break;

            case 'o':
                if (!strcasecmp(tag, "ok")) {
                    char tmp[READ_SIZE];
                    if (CONFIG_OK)
                        free(CONFIG_OK);
                    snprintf(tmp, sizeof(tmp), "%s\r\n", line);
                    CONFIG_OK = strdup(tmp);
                }
                break;

            case 'p':
                if (!strcasecmp(tag, "pk_allowed"))
                    CONFIG_PK_ALLOWED = num;
                else if (!strcasecmp(tag, "pt_allowed"))
                    CONFIG_PT_ALLOWED = num;
                else if (!strcasecmp(tag, "pulse_viol"))
                    CONFIG_PULSE_VIOLENCE = num;
                else if (!strcasecmp(tag, "pulse_mobile"))
                    CONFIG_PULSE_MOBILE = num;
                else if (!strcasecmp(tag, "pulse_current"))
                    CONFIG_PULSE_CURRENT = num;
                else if (!strcasecmp(tag, "pulse_zone"))
                    CONFIG_PULSE_ZONE = num;
                else if (!strcasecmp(tag, "pulse_autosave"))
                    CONFIG_PULSE_AUTOSAVE = num;
                else if (!strcasecmp(tag, "pulse_usage"))
                    CONFIG_PULSE_USAGE = num;
                else if (!strcasecmp(tag, "pulse_sanity"))
                    CONFIG_PULSE_SANITY = num;
                else if (!strcasecmp(tag, "pulse_timesave"))
                    CONFIG_PULSE_TIMESAVE = num;
                else if (!strcasecmp(tag, "pulse_idlepwd"))
                    CONFIG_PULSE_IDLEPWD = num;
                break;

            case 'r':
                if (!strcasecmp(tag, "rent_file_timeout"))
                    CONFIG_RENT_TIMEOUT = num;
                else if (!strcasecmp(tag, "reroll_stats"))
                    CONFIG_REROLL_PLAYER_CREATION = num;
                break;

            case 's':
                if (!strcasecmp(tag, "siteok_everyone"))
                    CONFIG_SITEOK_ALL = num;
                else if (!strcasecmp(tag, "start_messg")) {
                    strncpy(buf, "Reading start message in load_config()", sizeof(buf));
                    if (CONFIG_START_MESSG)
                        free(CONFIG_START_MESSG);
                    CONFIG_START_MESSG = fread_string(fl, buf);
                } else if (!strcasecmp(tag, "stack_mobs"))
                    CONFIG_STACK_MOBS = num;
                else if (!strcasecmp(tag, "stack_objs"))
                    CONFIG_STACK_OBJS = num;
                break;

            case 't':
                if (!strcasecmp(tag, "tunnel_size"))
                    CONFIG_TUNNEL_SIZE = num;
                else if (!strcasecmp(tag, "track_through_doors"))
                    CONFIG_TRACK_T_DOORS = num;
                break;

            case 'u':
                if (!strcasecmp(tag, "use_autowiz"))
                    CONFIG_USE_AUTOWIZ = num;
                else if (!strcasecmp(tag, "use_new_socials"))
                    CONFIG_NEW_SOCIALS = num;
                break;

            case 'w':
                if (!strcasecmp(tag, "welc_messg")) {
                    strncpy(buf, "Reading welcome message in load_config()", sizeof(buf));
                    if (CONFIG_WELC_MESSG)
                        free(CONFIG_WELC_MESSG);
                    CONFIG_WELC_MESSG = fread_string(fl, buf);
                }
                break;

            default:
                break;
        }
    }

    fclose(fl);
}

static std::vector<std::string> assetSchema = {
        "CREATE TABLE IF NOT EXISTS zones ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS areas ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS itemPrototypes ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS npcPrototypes ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS shops ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS guilds ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS rooms ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS dgScriptPrototypes ("
        "	id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");"
};

static std::vector<std::string> stateSchema = {
        "CREATE TABLE IF NOT EXISTS accounts ("
        "   id INTEGER PRIMARY KEY,"
        "	data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS playerCharacters ("
        "   id INTEGER NOT NULL PRIMARY KEY,"
        "   data TEXT NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS characters ("
        "   id INTEGER PRIMARY KEY,"
        "   generation INTEGER NOT NULL,"
        "   vnum INTEGER NOT NULL DEFAULT -1,"
        "   name TEXT,"
        "   shortDesc TEXT,"
        "   isPlayer INTEGER NOT NULL,"
        "   data TEXT NOT NULL,"
        "   location TEXT NOT NULL DEFAULT '{}',"
        "   relations TEXT NOT NULL DEFAULT '{}'"
        ");",

        "CREATE TABLE IF NOT EXISTS items ("
        "   id INTEGER PRIMARY KEY,"
        "   generation INTEGER NOT NULL,"
        "   vnum INTEGER NOT NULL DEFAULT -1,"
        "   name TEXT,"
        "   shortDesc TEXT,"
        "   data TEXT NOT NULL,"
        "   location TEXT NOT NULL DEFAULT '',"
        "   slot INTEGER NOT NULL DEFAULT 0,"
        "   relations TEXT NOT NULL DEFAULT '{}'"
        ");",

        "CREATE TABLE IF NOT EXISTS dgScripts ("
        "	id INTEGER PRIMARY KEY,"
        "   generation INTEGER NOT NULL,"
        "   vnum INTEGER NOT NULL DEFAULT -1,"
        "   name TEXT,"
        "	data TEXT NOT NULL,"
        "   location TEXT NOT NULL,"
        "   num INTEGER NOT NULL"
        ");",

        "CREATE TABLE IF NOT EXISTS globalData ("
        "   name TEXT PRIMARY KEY,"
        "   data TEXT NOT NULL"
        ");"
};

static void runQuery(std::string_view query, const std::shared_ptr<SQLite::Database>& db) {
    try {
        db->exec(query.data());
    }
    catch (const std::exception& e) {
        basic_mud_log("Error executing query: %s", e.what());
        basic_mud_log("For statement: %s", query.data());
        exit(1);
    }
}

void create_schema() {
    SQLite::Transaction transaction(*assetDb);
    for (const auto& query: assetSchema) {
        runQuery(query, assetDb);
    }
    transaction.commit();
}

static void create_state_schema() {
    for (const auto& query: stateSchema) {
        runQuery(query, stateDb);
    }
}

void dirty_all() {
    for(auto &[vn, r] : world) {
        r.save();
    }

    for(auto &[vn, obj] : obj_proto) {
        dirty_item_prototypes.insert(vn);
    }

    for(auto &[vn, npc] : mob_proto) {
        dirty_npc_prototypes.insert(vn);
    }

    for(auto &[vn, zone] : zone_table) {
        dirty_zones.insert(vn);
    }

    for(auto &[vn, area] : areas) {
        dirty_areas.insert(vn);
    }

    for(auto &[vn, dg] : trig_index) {
        dirty_dgscript_prototypes.insert(vn);
    }

    for(auto &[vn, guild] : guild_index) {
        dirty_guilds.insert(vn);
    }

    for(auto &[vn, shop] : shop_index) {
        dirty_shops.insert(vn);
    }

    for(auto &[vn, player] : players) {
        dirty_players.insert(vn);
    }

    for(auto &[vn, acc] : accounts) {
        dirty_accounts.insert(vn);
    }
}

static void process_dirty_rooms() {
    SQLite::Statement q1(*assetDb, "INSERT OR REPLACE INTO rooms (id, data) VALUES (?, ?)");
    SQLite::Statement q3(*assetDb, "DELETE FROM rooms WHERE id = ?");

    for(auto v : dirty_rooms) {
        auto r = world.find(v);
        if(r == world.end()) {
            // This room has been deleted.
            q3.bind(1, v);
            q3.exec();
            q3.reset();
            continue;
        }
        else {
            // we'll be using q1...
            q1.bind(1, v);
            q1.bind(2, r->second.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
            q1.exec();
            q1.reset();
        }
    }

    dirty_rooms.clear();
}

static void process_dirty_item_prototypes() {
	SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO itemPrototypes (id, data) VALUES (?,?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM itemPrototypes WHERE id = ?");

    for(auto v : dirty_item_prototypes) {
        auto r = obj_proto.find(v);
        if(r == obj_proto.end()) {
            // This item prototype has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.serializeProto().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_item_prototypes.clear();
}

static void process_dirty_npc_prototypes() {
    SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO npcPrototypes (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM npcPrototypes WHERE id = ?");

    for(auto v : dirty_npc_prototypes) {
        auto r = mob_proto.find(v);
        if(r == mob_proto.end()) {
            // This npc prototype has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.serializeProto().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_npc_prototypes.clear();

}

static void process_dirty_shops() {
    SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO shops (id, data) VALUES (?,?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM shops WHERE id = ?");

    for(auto v : dirty_shops) {
        auto r = shop_index.find(v);
        if(r == shop_index.end()) {
            // This shop has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_shops.clear();

}

static void process_dirty_guilds() {
    SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO guilds (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM guilds WHERE id = ?");

    for(auto v : dirty_guilds) {
        auto r = guild_index.find(v);
        if(r == guild_index.end()) {
            // This guild has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_guilds.clear();

}

static void process_dirty_zones() {
	SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO zones (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM zones WHERE id = ?");

    for(auto v : dirty_zones) {
        auto r = zone_table.find(v);
        if(r == zone_table.end()) {
            // This zone has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_zones.clear();
}

static void process_dirty_areas() {
    SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO areas (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM areas WHERE id = ?");

    for(auto v : dirty_areas) {
        auto r = areas.find(v);
        if(r == areas.end()) {
            // This area has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_areas.clear();

}

static void process_dirty_dgscript_prototypes() {
    SQLite::Statement q(*assetDb, "INSERT OR REPLACE INTO dgScriptPrototypes (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*assetDb, "DELETE FROM dgScriptPrototypes WHERE id = ?");

    for(auto v : dirty_dgscript_prototypes) {
        auto r = trig_index.find(v);
        if(r == trig_index.end()) {
            // This dgscript has been deleted.
            q1.bind(1, v);
            q1.exec();
            q1.reset();
            continue;
        }
        q.bind(1, v);
        q.bind(2, r->second.proto->serializeProto().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
    dirty_dgscript_prototypes.clear();

}

static void dump_state_accounts() {
    SQLite::Statement q(*stateDb, "INSERT OR REPLACE INTO accounts (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*stateDb, "DELETE FROM accounts WHERE id = ?");

    for(auto &[v, r] : accounts) {
        q.bind(1, v);
        q.bind(2, r.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
}

static void dump_state_players() {
    SQLite::Statement q(*stateDb, "INSERT OR REPLACE INTO playerCharacters (id, data) VALUES (?, ?)");
    SQLite::Statement q1(*stateDb, "DELETE FROM playerCharacters WHERE id = ?");

    for(auto &[v, r] : players) {
        q.bind(1, v);
        q.bind(2, r.serialize().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
}

static void dump_state_characters() {
    SQLite::Statement q(*stateDb, "INSERT OR REPLACE INTO characters (id, generation, vnum, name, shortDesc, isPlayer, data, location, relations) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    SQLite::Statement q1(*stateDb, "DELETE FROM characters WHERE id = ?");

    for(auto &[v, r] : uniqueCharacters) {
        if(v != r.second->id) r.second->id = v;
        q.bind(1, v);
        q.bind(2, r.first);
        q.bind(3, r.second->vn);
        q.bind(4, r.second->name);
        q.bind(5, r.second->short_description);
        q.bind(6, !IS_NPC(r.second));
        q.bind(7, r.second->serializeInstance().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.bind(8, r.second->serializeLocation().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.bind(9, r.second->serializeRelations().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }

}

static void dump_state_items() {
    SQLite::Statement q(*stateDb, "INSERT OR REPLACE INTO items (id, generation, vnum, name, shortDesc, data, location, slot, relations) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    SQLite::Statement q1(*stateDb, "DELETE FROM items WHERE id = ?");

    for(auto &[v, r] : uniqueObjects) {
        if(v != r.second->id) r.second->id = v;
        q.bind(1, v);
        q.bind(2, r.first);
        q.bind(3, r.second->vn);
        q.bind(4, r.second->name);
        q.bind(5, r.second->short_description);
        q.bind(6, r.second->serializeInstance().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.bind(7, r.second->serializeLocation());
        q.bind(8, r.second->worn_on);
        q.bind(9, r.second->serializeRelations().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
}

static void dump_state_dgscripts() {
    SQLite::Statement q(*stateDb, "INSERT OR REPLACE INTO dgscripts (id, generation, vnum, name, data, location, num) VALUES (?, ?, ?, ?, ?, ?, ?)");
    SQLite::Statement q1(*stateDb, "DELETE FROM dgscripts WHERE id = ?");

    for(auto &[v, r] : uniqueScripts) {
        if(v != r.second->id) r.second->id = v;
        q.bind(1, v);
        q.bind(2, r.first);
        q.bind(3, r.second->vn);
        q.bind(4, r.second->name);
        q.bind(5, r.second->serializeInstance().dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.bind(6, r.second->serializeLocation());
        q.bind(7, r.second->order);
        q.exec();
        q.reset();
    }
    dirty_dgscripts.clear();
}


void process_dirty() {
    if(saveAll) {
        logger->info("Performing full save of all game assets.");
        send_to_all("Saving all game assets, please wait...\r\n");
    }
    auto startTime = std::chrono::high_resolution_clock::now();

    if(!dirty_rooms.empty()) {
        process_dirty_rooms();
    }

    if(!dirty_item_prototypes.empty()) {
        process_dirty_item_prototypes();
    }

    if(!dirty_npc_prototypes.empty()) {
        process_dirty_npc_prototypes();
    }

    if(!dirty_shops.empty()) {
        process_dirty_shops();
    }

    if(!dirty_guilds.empty()) {
        process_dirty_guilds();
    }

    if(!dirty_zones.empty()) {
        process_dirty_zones();
    }

    if(!dirty_areas.empty()) {
        process_dirty_areas();
    }

    if(!dirty_dgscript_prototypes.empty()) {
        process_dirty_dgscript_prototypes();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    // time ins econds as a double...
    auto duration = std::chrono::duration<double>(endTime - startTime).count();

    if(saveAll) {
        logger->info("Full save of all game assets completed in {} seconds.", duration);
        send_to_all("Saving all game assets completed in %f seconds.\r\n", duration);
    }
}

void dump_state_globalData() {
    SQLite::Statement q(*stateDb, "INSERT OR REPLACE INTO globalData (name, data) VALUES (?, ?)");
    SQLite::Statement q1(*stateDb, "DELETE FROM globalData WHERE name = ?");

    std::map<std::string, nlohmann::json> globalData;

    globalData["time"] = time_info.serialize();
    globalData["weather"] = weather_info.serialize();
    auto gRoom = world.find(0);
    if(gRoom != world.end()) {
        if(gRoom->second.script && gRoom->second.script->global_vars) {
            globalData["dgGlobals"] = serializeVars(gRoom->second.script->global_vars);
        }
    }

    for(auto &[v, r] : globalData) {
        q.bind(1, v);
        q.bind(2, r.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }
}

static void cleanup_state() {
    auto vecFiles = getStateFiles();
    std::list<std::filesystem::path> files(vecFiles.begin(), vecFiles.end());

    // If we have more than 20 state files, we want to purge the oldest one(s) until we have just 20.
    while(files.size() > 20) {
        std::filesystem::remove(files.back());
        files.pop_back();
    }
}

void dump_state() {

    //send_to_all("Saving game...\r\n");
    logger->info("Beginning dump of state to disk.");

    // Open up a new database file as <cwd>/state/<timestamp>.sqlite3 and dump the state into it.
    auto path = std::filesystem::current_path() / "state";
    std::filesystem::create_directories(path);
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now = *std::localtime(&time_t_now);

    auto tempPath = path / fmt::format("{}.sqlite3", config::stateDbName);
    auto journalPath = path / fmt::format("{}.sqlite3-journal", config::stateDbName);
    std::filesystem::remove(tempPath);
    std::filesystem::remove(journalPath);

    auto newPath = path / fmt::format("{}-{:04}{:02}{:02}{:02}{:02}{:02}.sqlite3",
                                      config::stateDbName,
                                      tm_now.tm_year + 1900,
                                      tm_now.tm_mon + 1,
                                      tm_now.tm_mday,
                                      tm_now.tm_hour,
                                      tm_now.tm_min,
                                      tm_now.tm_sec);

    bool success = false;
    double duration;

    try {
        stateDb = std::make_shared<SQLite::Database>(tempPath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        auto startTime = std::chrono::high_resolution_clock::now();
        SQLite::Transaction trans(*stateDb);
        create_state_schema();
        dump_state_accounts();
        dump_state_characters();
        dump_state_players();
        dump_state_dgscripts();
        dump_state_items();
        dump_state_globalData();
        trans.commit();
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration<double>(endTime - startTime).count();
        success = true;
    } catch (std::exception &e) {
        logger->critical("(GAME HAS NOT BEEN SAVED!) Exception in dump_state(): {}", e.what());
        send_to_all("Warning, a critical error occurred during save! Please alert staff!\r\n");
        return;
    }

    stateDb.reset();

    if(success) {
        std::filesystem::rename(tempPath, newPath);
        logger->info("Finished dumping state to {} in {} seconds.", newPath.string(), duration);
        //send_to_all("Game saved in %f seconds.\r\n", duration);
    }

    cleanup_state();

}

int64_t nextObjID() {
    int64_t id = 0;
    while(uniqueObjects.contains(id)) id++;
    return id;
}

int64_t nextCharID() {
    int64_t id = 0;
    while(uniqueCharacters.contains(id)) id++;
    return id;
}
// ^#(?<type>[ROC])(?<id>\d+)(?::(?<generation>\d+)?)?
static boost::regex uid_regex(R"(^#(?<type>[ROC])(?<id>\d+)(?::(?<generation>\d+)?)?(?<active>!)?)", boost::regex::icase);

bool isUID(const std::string& uid) {
    return boost::regex_match(uid, uid_regex);
}

std::optional<UID> resolveUID(const std::string& uid) {
    // First we need to check if it matches or not.
    boost::smatch match;

    if(!boost::regex_search(uid, match, uid_regex)) {
        return std::nullopt;
    }
    // extract the type as a char, id and generation as int64_t and time_t respectively.
    char type = toupper(match["type"].str()[0]);
    int64_t id = std::stoll(match["id"].str());
    bool active = match["active"].matched;
    time_t generation = 0;
    if(match["generation"].matched) {
        generation = std::stoll(match["generation"].str());
    }

    if(type == 'R') {
        if(world.contains(id)) return &world[id];
    } else if(type == 'O') {
        auto find = uniqueObjects.find(id);
        if(find != uniqueObjects.end()) {
            if(!generation || (find->second.first == generation)) {
                if(active && !find->second.second->isActive())
                    return std::nullopt;
                return find->second.second;
            }
        }
    } else if(type == 'C') {
        auto find = uniqueCharacters.find(id);
        if(find != uniqueCharacters.end()) {
            if(!generation || (find->second.first == generation)) {
                if(active && !find->second.second->isActive())
                    return std::nullopt;
                return find->second.second;
            }
        }
    }
    return std::nullopt;
}

struct obj_data* obj_ref::get(bool checkActive) {
    auto find = uniqueObjects.find(id);
    if(find != uniqueObjects.end()) {
        if(find->second.first == generation) {
            if(checkActive && !find->second.second->isActive())
                return nullptr;
            return find->second.second;
        }
    }
    return nullptr;
}

struct char_data* char_ref::get(bool checkActive) {
    auto find = uniqueCharacters.find(id);
    if(find != uniqueCharacters.end()) {
        if(find->second.first == generation) {
            if(checkActive && !find->second.second->isActive())
                return nullptr;
            return find->second.second;
        }
    }
    return nullptr;
}

struct room_data* room_ref::get(bool checkActive) {
    auto find = world.find(id);
    if(find != world.end()) {
        return &find->second;
    }
    return nullptr;
}