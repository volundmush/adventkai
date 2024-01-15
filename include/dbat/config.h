#pragma once

#include "structs.h"

namespace config {
    // If this is set to true, the server will use multiple threads to run the ASIO executor.
    // it is recommended that at least 2 threads be active so that networking is separate from
    // the game logic.
    extern bool enableMultithreading;
    // If enableMultithreading is set to true, this will be the number of threads used to run the game.
    // if set to <1, the number of threads will be equal to the number of cores on the machine.
    extern int threadsCount;
    // This will be true if multithreading has been successfully engaged.
    extern bool usingMultithreading;
    // the duration - in milliseconds - between calls to the heartbeat.
    extern std::chrono::milliseconds heartbeatInterval;

    extern std::string serverAddress;
    extern uint16_t serverPort;

    extern std::string logFile;
    // the filename for the game save database.
    extern std::string assetDbName;
    extern std::string stateDbName;
    extern bool logEgregiousTimings;

    extern bool testMode;
}

// global variables
extern int pk_allowed, pt_allowed, level_can_shout, holler_move_cost, CURRENT_ERA;
extern int mob_specials_used, number_of_assassins, TOPLOADED;

extern char last_user_freed[MAX_INPUT_LENGTH];

extern char *topname[25];
extern int64_t toppoint[25];

extern int ERAPLAYERS, TOPCOUNTDOWN, HEDITS, WISHTIME, TOP_OF_NEWS, LASTNEWS;
extern char *NEWS_TITLE;
extern int crash_file_timeout, rent_file_timeout, imc_is_enabled;
extern time_t BOARDNEWMORT, BOARDNEWIMM, BOARDNEWCOD, BOARDNEWDUO, BOARDNEWBUI, NEWSUPDATE;
extern time_t INTERESTTIME, LASTINTEREST, LASTPAYOUT;
extern int LASTPAYTYPE;

extern int SAIYAN_ALLOWED, MAJIN_ALLOWED, MOON_UP, DEATHPHASE, PCOUNT, HIGHPCOUNT;
extern time_t PCOUNTDATE, PCOUNTDAY;

extern int SELFISHMETER, SHADOW_DRAGON1, SHADOW_DRAGON2, SHADOW_DRAGON3, SHADOW_DRAGON4, SHADOW_DRAGON5;
extern int SHADOW_DRAGON6, SHADOW_DRAGON7;

extern int DBALL_HUNTER1, DBALL_HUNTER2, DBALL_HUNTER3, DBALL_HUNTER4;
extern int DBALL_HUNTER1_VNUM, DBALL_HUNTER2_VNUM, DBALL_HUNTER3_VNUM, DBALL_HUNTER4_VNUM;

extern int tunnel_size, max_exp_gain, max_exp_loss, max_npc_corpse_time, max_pc_corpse_time;

extern int idle_void, idle_rent_time, idle_max_level, dts_are_dumps, pulse_violence;

extern int pulse_zone, pulse_mobile, pulse_autosave, pulse_idlepwd, pulse_sanity, pulse_usage;
extern int pulse_timesave, pulse_current, load_into_inventory;

extern const char *OK, *NOPERSON, *NOEFFECT;

extern int track_through_doors, level_cap, show_obj_stacking, show_mob_stacking, mob_fighting;

extern int free_rent, max_obj_save, min_rent_cost, auto_save, autosave_time;

extern int auto_pwipe, selfdelete_fastwipe;

extern const struct pclean_criteria_data pclean_criteria[7];

extern room_vnum death_start_room, mortal_start_room, immort_start_room, frozen_start_room;

extern room_vnum donation_room_1, donation_room_2, donation_room_3;

extern int bitwarning, bitsavetodisk;

extern uint16_t DFLT_PORT;

extern const char *DFLT_IP, *DFLT_DIR, *LOGNAME, *ANSIQUESTION, *MENU, *WELC_MESSG, *START_MESSG;

extern int max_playing, max_filesize, max_bad_pws, siteok_everyone, nameserver_is_slow;

extern int auto_save_olc, use_new_socials, use_autowiz, min_wizlist_lev, initial_points;

extern obj_vnum portal_object;

extern int disp_closed_doors, reroll_status, allow_multiclass, allow_prestige, auto_level;

extern int enable_compression, enable_languages, all_items_unique, method;

extern float exp_multiplier;
