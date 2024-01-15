/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and constants                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#include "net.h"

/**********************************************************************
* Structures                                                          *
**********************************************************************/
struct account_data {
    account_data() = default;
    explicit account_data(const nlohmann::json& j);
    vnum vn{NOTHING};
    std::string name;
    std::string passHash;
    std::string email;
    time_t created{};
    time_t lastLogin{};
    time_t lastLogout{};
    time_t lastPasswordChanged{};
    double totalPlayTime{};
    std::string disabledReason;
    time_t disabledUntil{0};
    int adminLevel{};
    int rpp{};
    int slots{3};
    std::vector<std::string> customs;
    std::vector<vnum> characters;
    std::set<descriptor_data*> descriptors;
    std::set<net::Connection*> connections;

    nlohmann::json serialize();
    void deserialize(const nlohmann::json& j);

    void modRPP(int amt);

    bool checkPassword(const std::string& password);
    bool setPassword(const std::string& password);

    static int getNextID();

};

struct player_data {
    player_data() = default;
    explicit player_data(const nlohmann::json& j);
    int64_t id{NOTHING};
    std::string name;
    struct account_data *account{};
    struct char_data* character{};
    std::vector<struct alias_data> aliases;    /* Character's aliases                  */
    std::set<int64_t> sensePlayer;
    std::set<mob_vnum> senseMemory;
    std::map<int64_t, std::string> dubNames;
    char *color_choices[NUM_COLOR]{}; /* Choices for custom colors		*/
    struct txt_block *comm_hist[NUM_HIST]{}; /* Player's communications history     */

    nlohmann::json serialize();
};

enum class AreaType {
    Dimension = 0,
    CelestialBody = 1,
    Region = 2,
    Structure = 3,
    Vehicle = 4
};

struct area_data {
    area_data() = default;
    explicit area_data(const nlohmann::json &j);
    vnum vn{NOTHING}; /* virtual number of this area		*/
    std::string name; /* name of this area			*/
    std::set<room_vnum> rooms; /* rooms in this area			*/
    std::set<vnum> children; /* child areas				*/
    std::optional<double> gravity; /* gravity in this area			*/
    std::optional<vnum> parent; /* parent area				*/
    AreaType type{AreaType::Dimension}; /* type of area				*/
    std::optional<vnum> extraVn; /* vehicle or house outer object vnum, orbit for CelBody */
    bool ether{false}; /* is this area etheric?			*/
    std::bitset<NUM_AREA_FLAGS> flags; /* area flags				*/
    nlohmann::json serialize();
    static vnum getNextID();
    static bool isPlanet(const area_data &area);
    std::optional<room_vnum> getLaunchDestination();
};

/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
    char *keyword;                 /* Keyword in look/examine          */
    char *description;             /* What to see                      */
    struct extra_descr_data *next; /* Next in list                     */
};


struct obj_affected_type {
    obj_affected_type() = default;
    explicit obj_affected_type(const nlohmann::json& j);
    void deserialize(const nlohmann::json& j);
    nlohmann::json serialize();
    int location{};       /* Which ability to change (APPLY_XXX) */
    int specific{};       /* Some locations have parameters      */
    double modifier{};       /* How much it changes by              */
};

struct obj_spellbook_spell {
    int spellname;    /* Which spell is written */
    int pages;        /* How many pages does it take up */
};

struct obj_ref {
    int64_t id{NOTHING};
    time_t generation{};
    struct obj_data* get(bool checkActive);
};

struct char_ref {
    int64_t id{NOTHING};
    time_t generation{};
    struct char_data* get(bool checkActive);
};

struct room_ref {
    int64_t id{NOTHING};
    time_t generation{};
    struct room_data* get(bool checkActive);
};

struct unit_data {
    unit_data() = default;
    virtual ~unit_data() = default;
    vnum vn{NOTHING}; /* Where in database */
    zone_vnum zone{NOTHING};
    char *name{};
    char *room_description{};      /* When thing is listed in room */
    char *look_description{};      /* what to show when looked at */
    char *short_description{};     /* when displayed in list or action message. */
    bool exists{true}; // used for deleted objects. invalid ones are !exists
    struct extra_descr_data *ex_description{}; /* extra descriptions     */

    std::vector<trig_vnum> proto_script; /* list of default triggers  */
    struct script_data *script{};  /* script info for the object */

    struct obj_data *contents{};     /* Contains objects  */
    weight_t getInventoryWeight();
    int64_t getInventoryCount();

    std::list<struct obj_data*> getContents();

    int64_t id{NOTHING}; /* used by DG triggers	*/
    time_t generation{};             /* creation time for dupe check     */

    nlohmann::json serializeUnit();
    nlohmann::json serializeContents();

    void activateContents();
    void deactivateContents();

    void deserializeUnit(const nlohmann::json& j);
    std::string scriptString();

    virtual std::string getUID(bool active = true) = 0;
    virtual bool isActive() = 0;
    virtual void save() = 0;

    nlohmann::json serializeScripts();
    void deserializeScripts();

    struct obj_data* findObjectVnum(obj_vnum objVnum, bool working = true);
    virtual struct obj_data* findObject(const std::function<bool(struct obj_data*)> &func, bool working = true);
    virtual std::set<struct obj_data*> gatherObjects(const std::function<bool(struct obj_data*)> &func, bool working = true);

};


/* ================== Memory Structure for Objects ================== */
struct obj_data : public unit_data {
    obj_data() = default;
    explicit obj_data(const nlohmann::json& j);

    nlohmann::json serializeBase();
    nlohmann::json serializeInstance();
    nlohmann::json serializeProto();

    std::string serializeLocation();
    nlohmann::json serializeRelations();

    void deserializeLocation(const std::string& txt, int16_t slot);
    void deserializeRelations(const nlohmann::json& j);

    void deserializeBase(const nlohmann::json& j);
    void deserializeProto(const nlohmann::json& j);
    void deserializeInstance(const nlohmann::json& j, bool isActive);
    void deserializeContents(const nlohmann::json& j, bool isActive);

    void activate();

    void deactivate();

    int getAffectModifier(int location, int specific = -1);

    std::string getUID(bool active = true) override;
    bool active{false};
    bool isActive() override;
    void save() override;

    struct room_data* getAbsoluteRoom();
    struct room_data* getRoom();
    bool isWorking();
    void clearLocation();

    obj_ref ref() { return obj_ref{id, generation}; }


    room_rnum in_room{NOWHERE};        /* In what room -1 when conta/carr	*/
    room_vnum room_loaded{NOWHERE};    /* Room loaded in, for room_max checks	*/

    int64_t value[NUM_OBJ_VAL_POSITIONS]{};   /* Values of the item (see list)    */
    int8_t type_flag{};      /* Type of item                        */
    int level{}; /* Minimum level of object.            */
    std::bitset<NUM_ITEM_WEARS> wear_flags{}; /* Where you can wear it     */
    std::bitset<NUM_ITEM_FLAGS> extra_flags{}; /* If it hums, glows, etc.  */
    weight_t weight{};         /* Weight what else                     */
    weight_t getWeight();
    weight_t getTotalWeight();
    int cost{};           /* Value when sold (gp.)               */
    int cost_per_day{};   /* Cost to keep pr. real day           */
    int timer{};          /* Timer for object                    */
    std::bitset<NUM_AFF_FLAGS> bitvector{}; /* To set chars bits          */
    int size{SIZE_MEDIUM};           /* Size class of object                */

    struct obj_affected_type affected[MAX_OBJ_AFFECT]{};  /* affects */

    struct obj_data *in_obj{};       /* In what object nullptr when none    */
    struct char_data *carried_by{};  /* Carried by :nullptr in room/conta   */
    struct char_data *worn_by{};      /* Worn by? */
    int16_t worn_on{-1};          /* Worn where?		      */

    struct obj_data *next_content{}; /* For 'contains' lists             */
    struct obj_data *next{};         /* For the object list              */

    struct obj_spellbook_spell *sbinfo{};  /* For spellbook info */
    struct char_data *sitting{};       /* Who is sitting on me? */
    int scoutfreq{};
    time_t lload{};
    int healcharge{};
    int64_t kicharge{};
    int kitype{};
    struct char_data *user{};
    struct char_data *target{};
    int distance{};
    int foob{};
    int64_t aucter{};
    int64_t curBidder{};
    time_t aucTime{};
    int bid{};
    int startbid{};
    char *auctname{};
    int posttype{};
    struct obj_data *posted_to{};
    struct obj_data *fellow_wall{};

    std::optional<double> gravity;

    std::optional<vnum> getMatchingArea(const std::function<bool(const area_data&)>& f);

    bool isProvidingLight();
    double currentGravity();
};
/* ======================================================================= */


/* room-related structures ************************************************/



struct room_direction_data {
    room_direction_data() = default;
    explicit room_direction_data(const nlohmann::json &j);
    ~room_direction_data();
    char *general_description{};       /* When look DIR.			*/
    char *keyword{};        /* for open/close			*/

    int16_t exit_info{};        /* Exit info			*/
    obj_vnum key{NOTHING};        /* Key's number (-1 for no key)		*/
    room_rnum to_room{NOWHERE};        /* Where direction leads (NOWHERE)	*/
    int dclock{};            /* DC to pick the lock			*/
    int dchide{};            /* DC to find hidden			*/
    int dcskill{};            /* Skill req. to move through exit	*/
    int dcmove{};            /* DC for skill to move through exit	*/
    int failsavetype{};        /* Saving Throw type on skill fail	*/
    int dcfailsave{};        /* DC to save against on fail		*/
    room_vnum failroom{NOWHERE};        /* Room # to put char in when fail > 5  */
    room_vnum totalfailroom{NOWHERE};        /* Room # if char fails save < 5	*/

    struct room_data* getDestination();


    nlohmann::json serialize();
};

enum class MoonCheck : uint8_t {
    NoMoon = 0,
    NotFull = 1,
    Full = 2
};


/* ================== Memory Structure for room ======================= */
struct room_data : public unit_data {
    room_data() = default;
    ~room_data() override;
    explicit room_data(const nlohmann::json &j);
    int sector_type{};            /* sector type (move/hide)            */
    std::array<room_direction_data*, NUM_OF_DIRS> dir_option{}; /* Directions */
    std::bitset<NUM_ROOM_FLAGS> room_flags{};   /* DEATH,DARK ... etc */
    SpecialFunc func{};
    struct char_data *people{};    /* List of NPC / PC in room */
    int timed{};                   /* For timed Dt's                     */
    int dmg{};                     /* How damaged the room is            */
    int geffect{};            /* Effect of ground destruction       */
    std::optional<vnum> area;      /* Area number; empty for unassigned     */

    std::optional<double> gravity;

    bool isSunken();

    int getDamage();
    int setDamage(int amount);
    int modDamage(int amount);

    double getGravity();

    nlohmann::json serialize();
    void deserializeContents(const nlohmann::json& j, bool isActive);

    std::optional<vnum> getMatchingArea(std::function<bool(const area_data&)> f);
    std::string getUID(bool active = true) override;
    bool isActive() override;
    void save() override;

    room_ref ref() { return room_ref{id, generation}; }

    std::optional<room_vnum> getLaunchDestination();

    std::list<struct char_data*> getPeople();

    MoonCheck checkMoon();

};
/* ====================================================================== */


/* char-related structures ************************************************/


/* memory structure for characters */
struct memory_rec_struct {
    int32_t id;
    struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
    time_info_data() = default;
    explicit time_info_data(int64_t timestamp);
    double remainder{};
    int seconds{}, minutes{}, hours{}, day{}, month{};
    int64_t year{};
    void deserialize(const nlohmann::json& j);
    nlohmann::json serialize();
    // The number of seconds since year 0. Can be negative.
    int64_t current();
};


/* These data contain information about a players time data */
struct time_data {
    time_data() = default;
    explicit time_data(const nlohmann::json &j);
    void deserialize(const nlohmann::json& j);
    int64_t birth{};    /* NO LONGER USED This represents the characters current IC age        */
    time_t created{};    /* This does not change                              */
    int64_t maxage{};    /* This represents death by natural causes (UNUSED) */
    time_t logon{};    /* Time of the last logon (used to calculate played) */
    double played{};    /* This is the total accumulated time played in secs */
    double secondsAged{}; // The player's current IC age, in seconds.
    int currentAge();
    nlohmann::json serialize();
};


/* The pclean_criteria_data is set up in config.c and used in db.c to
   determine the conditions which will cause a player character to be
   deleted from disk if the automagic pwipe system is enabled (see config.c).
*/
struct pclean_criteria_data {
    int level;        /* max level for this time limit	*/
    int days;        /* time limit in days			*/
};


/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs. This structure
 * can be changed freely.
 */
struct alias_data {
    alias_data() = default;
    explicit alias_data(const nlohmann::json &j);
    std::string name;
    std::string replacement;
    int type{};
    nlohmann::json serialize();
};

/* this can be used for skills that can be used per-day */
struct memorize_node {
    int timer;            /* how many ticks till memorized */
    int spell;            /* the spell number */
    struct memorize_node *next;    /* link to the next node */
};

struct innate_node {
    int timer;
    int spellnum;
    struct innate_node *next;
};

/* Specials used by NPCs, not PCs */
struct mob_special_data {
    mob_special_data() = default;
    explicit mob_special_data(const nlohmann::json& j);
    nlohmann::json serialize();
    void deserialize(const nlohmann::json& j);
    memory_rec *memory{};        /* List of attackers to remember	       */
    int8_t attack_type{};        /* The Attack Type Bitvector for NPC's     */
    int8_t default_pos{POS_STANDING};        /* Default position for NPC                */
    int8_t damnodice{};          /* The number of damage dice's	       */
    int8_t damsizedice{};        /* The size of the damage dice's           */
    bool newitem{};             /* Check if mob has new inv item       */
};

/* An affect structure. */
struct affected_type {
    affected_type() = default;
    explicit affected_type(const nlohmann::json& j);
    int16_t type{};          /* The type of spell that caused this      */
    int16_t duration{};      /* For how long its effects will last      */
    double modifier{};         /* This is added to apropriate ability     */
    int location{};         /* Tells which ability to change(APPLY_XXX)*/
    int specific{};         /* Some locations have parameters          */
    bitvector_t bitvector{}; /* Tells which bits to set (AFF_XXX) */
    nlohmann::json serialize();
    struct affected_type *next{};
};

/* Queued spell entry */
struct queued_act {
    int level;
    int spellnum;
};

/* Structure used for chars following other chars */
struct follow_type {
    struct char_data *follower;
    struct follow_type *next;
};


enum ResurrectionMode : uint8_t {
    Costless = 0,
    Basic = 1,
    RPP = 2
};

struct skill_data {
    skill_data() = default;
    explicit skill_data(const nlohmann::json& j);
    int16_t level{0};
    int16_t perfs{0};
    nlohmann::json serialize();
    void deserialize(const nlohmann::json& j);
};

struct trans_data {
    trans_data() = default;
    explicit trans_data(const nlohmann::json& j);

    double timeSpentInForm{0.0};

    double blutz{0.0}; // The number of seconds you can spend in Oozaru.

    nlohmann::json serialize();
    void deserialize(const nlohmann::json& j);
};


/* ================== Structure for player/non-player ===================== */
struct char_data : public unit_data {
    char_data() = default;
    // this constructor below is to be used only for the mob_proto map.
    explicit char_data(const nlohmann::json& j);
    nlohmann::json serializeBase();
    nlohmann::json serializeInstance();
    nlohmann::json serializeProto();
    nlohmann::json serializePlayer();
    nlohmann::json serializeEquipment();
    void deserializeBase(const nlohmann::json& j);
    void deserializeProto(const nlohmann::json& j);
    void deserializeInstance(const nlohmann::json& j, bool isActive);
    void deserializeMobile(const nlohmann::json& j);
    void deserializePlayer(const nlohmann::json& j, bool isActive);
    void deserializeContents(const nlohmann::json& j, bool isActive);
    void deserializeEquipment(const nlohmann::json& j, bool isActive);
    void activate();
    void deactivate();
    std::optional<vnum> getMatchingArea(std::function<bool(const area_data&)> f);
    void login();

    nlohmann::json serializeLocation();
    nlohmann::json serializeRelations();

    void sendGMCP(const std::string &cmd, const nlohmann::json &j);

    void deserializeLocation(const nlohmann::json& j);
    void deserializeRelations(const nlohmann::json& j);

    std::string getUID(bool active = true) override;

    bool active{false};
    bool isActive() override;
    void save() override;

    void ageBy(double addedTime);
    void setAge(double newAge);

    struct room_data* getRoom();

    struct obj_data* findObject(const std::function<bool(struct obj_data*)> &func, bool working = true) override;
    std::set<struct obj_data*> gatherObjects(const std::function<bool(struct obj_data*)> &func, bool working = true) override;

    char_ref ref() { return char_ref{id, generation}; }


    char *title{};
    RaceID race{RaceID::Spirit};
    SenseiID chclass{SenseiID::Commoner};


    /* PC / NPC's weight                    */
    weight_t weight{0};
    weight_t getWeight(bool base = false);
    weight_t getTotalWeight();
    weight_t getCurrentBurden();
    double getBurdenRatio();
    bool canCarryWeight(struct obj_data *obj);
    bool canCarryWeight(struct char_data *obj);
    bool canCarryWeight(weight_t val);

    int getHeight(bool base = false);
    int setHeight(int val);
    int modHeight(int val);

    int getArmor();

    std::unordered_map<CharNum, num_t> nums{};

    num_t get(CharNum stat);
    num_t set(CharNum stat, num_t val);
    num_t mod(CharNum stat, num_t val);

    struct mob_special_data mob_specials{};

    int size{SIZE_UNDEFINED};
    int getSize();
    int setSize(int val);

    std::unordered_map<CharMoney, money_t> moneys;
    money_t get(CharMoney type);
    money_t set(CharMoney type, money_t val);
    money_t mod(CharMoney type, money_t val);

    std::unordered_map<CharAlign, align_t> aligns;
    align_t get(CharAlign type);
    align_t set(CharAlign type, align_t val);
    align_t mod(CharAlign type, align_t val);

    std::unordered_map<CharAppearance, appearance_t> appearances;
    appearance_t get(CharAppearance type);
    appearance_t set(CharAppearance type, appearance_t val);
    appearance_t mod(CharAppearance type, appearance_t val);

    std::bitset<NUM_AFF_FLAGS> affected_by{};/* Bitvector for current affects	*/

    std::unordered_map<CharStat, stat_t> stats;
    stat_t get(CharStat type, bool base = true);
    stat_t set(CharStat type, stat_t val);
    stat_t mod(CharStat type, stat_t val);

    // Instance-relevant fields below...
    room_vnum in_room{NOWHERE};        /* Location (real room number)		*/
    room_vnum was_in_room{NOWHERE};    /* location for linkdead people		*/

    std::bitset<NUM_ADMFLAGS> admflags{};    /* Bitvector for admin privs		*/
    room_vnum hometown{NOWHERE};        /* PC Hometown / NPC spawn room         */
    struct time_data time{};    /* PC's AGE in days			*/
    struct affected_type *affected{};
    /* affected by what spells		*/
    struct affected_type *affectedv{};
    /* affected by what combat spells	*/
    struct queued_act *actq{};    /* queued spells / other actions	*/

    /* Equipment array			*/
    struct obj_data *equipment[NUM_WEARS]{};

    std::map<int, struct obj_data*> getEquipment();
    struct obj_data* getEquipSlot(int slot);

    struct descriptor_data *desc{};    /* nullptr for mobiles			*/

    struct script_memory *memory{};    /* for mob memory triggers		*/

    struct char_data *next_in_room{};
    /* For room->people - list		*/
    struct char_data *next{};    /* For either monster or ppl-list	*/
    struct char_data *next_fighting{};
    /* For fighting list			*/
    struct char_data *next_affect{};/* For affect wearoff			*/
    struct char_data *next_affectv{};
    /* For round based affect wearoff	*/

    struct follow_type *followers{};/* List of chars followers		*/
    struct char_data *master{};    /* Who is char following?		*/
    int64_t master_id{};

    struct memorize_node *memorized{};
    struct innate_node *innate{};

    struct char_data *fighting{};    /* Opponent				*/

    int8_t position{POS_STANDING};        /* Standing, fighting, sleeping, etc.	*/

    int timer{};            /* Timer for update			*/

    struct obj_data *sits{};      /* What am I sitting on? */
    struct char_data *blocks{};    /* Who am I blocking?    */
    struct char_data *blocked{};   /* Who is blocking me?    */
    struct char_data *absorbing{}; /* Who am I absorbing */
    struct char_data *absorbby{};  /* Who is absorbing me */
    struct char_data *carrying{};
    struct char_data *carried_by{};

    int8_t feats[MAX_FEATS + 1]{};    /* Feats (booleans and counters)	*/
    int combat_feats[CFEAT_MAX + 1][FT_ARRAY_MAX]{};
    /* One bitvector array per CFEAT_ type	*/
    int school_feats[SFEAT_MAX + 1]{};/* One bitvector array per CFEAT_ type	*/

    std::map<uint16_t, skill_data> skill;

    std::bitset<NUM_PLR_FLAGS> playerFlags{}; /* act flag for NPC's; player flag for PC's */
    std::bitset<NUM_MOB_FLAGS> mobFlags{};

    std::bitset<NUM_WEARS> bodyparts{};  /* Bitvector for current bodyparts      */
    int16_t saving_throw[3]{};    /* Saving throw				*/
    int16_t apply_saving_throw[3]{};    /* Saving throw bonuses			*/

    int armor{0};        /* Internally stored *10		*/

    int64_t exp{};            /* The experience of the player		*/
    int64_t getExperience();
    int64_t setExperience(int64_t value);
    int64_t modExperience(int64_t value, bool applyBonuses = true);

    int accuracy{};            /* Base hit accuracy			*/
    int accuracy_mod{};        /* Any bonus or penalty to the accuracy	*/
    int damage_mod{};        /* Any bonus or penalty to the damage	*/

    FormID form{FormID::Base};        /* Current form of the character		*/
    double transBonus{0.0};   // Varies from -0.3 to 0.3
    void gazeAtMoon();

    // Data stored about different forms.
    std::unordered_map<FormID, trans_data> transforms;

    int16_t spellfail{};        /* Total spell failure %                 */
    int16_t armorcheck{};        /* Total armorcheck penalty with proficiency forgiveness */
    int16_t armorcheckall{};    /* Total armorcheck penalty regardless of proficiency */

    /* All below added by Iovan for sure o.o */
    int64_t charge{};
    int64_t chargeto{};
    int64_t barrier{};
    char *clan{};
    room_vnum droom{};
    int choice{};
    int sleeptime{};
    int foodr{};
    int altitude{};
    int overf{};
    int spam{};

    room_vnum radar1{};
    room_vnum radar2{};
    room_vnum radar3{};
    int ship{};
    room_vnum shipr{};
    time_t lastpl{};
    time_t lboard[5]{};

    room_vnum listenroom{};
    int crank{};
    int kaioken{};
    int absorbs{};
    int boosts{};
    int upgrade{};
    time_t lastint{};
    int majinize{};
    short fury{};
    short btime{};
    int eavesdir{};
    time_t deathtime{};

    int64_t suppression{};
    struct char_data *drag{};
    struct char_data *dragged{};
    struct char_data *mindlink{};
    int lasthit{};
    int dcount{};
    char *voice{};                  /* PC's snet voice */
    int limbs[4]{};                 /* 0 Right Arm, 1 Left Arm, 2 Right Leg, 3 Left Leg */
    time_t rewtime{};
    struct char_data *grappling{};
    struct char_data *grappled{};
    int grap{};
    int genome[2]{};                /* Bio racial bonus, Genome */
    int combo{};
    int lastattack{};
    int combhits{};
    int ping{};
    int starphase{};
    std::optional<RaceID> mimic{};
    std::bitset<MAX_BONUSES> bonuses{};

    int cooldown{};
    int death_type{};

    int64_t moltexp{};
    int moltlevel{};

    char *loguser{};                /* What user was I last saved as?      */
    int arenawatch{};
    int64_t majinizer{};
    int speedboost{};
    int skill_slots{};
    int tail_growth{};
    int rage_meter{};
    char *feature{};

    int armor_last{};
    int forgeting{};
    int forgetcount{};
    int backstabcool{};
    int con_cooldown{};
    short stupidkiss{};
    char *temp_prompt{};

    int personality{};
    int combine{};
    int linker{};
    int fishstate{};
    int throws{};

    struct char_data *defender{};
    struct char_data *defending{};

    int lifeperc{};
    int gooptime{};
    int blesslvl{};
    struct char_data *poisonby{};
    std::set<struct char_data*> poisoned;

    int mobcharge{};
    int preference{};
    int aggtimer{};

    int lifebonus{};
    int asb{};
    int regen{};
    int con_sdcooldown{};

    int8_t limb_condition[4]{};

    char *rdisplay{};

    struct char_data *original{};

    std::set<struct char_data*> clones{};
    int relax_count{};
    int ingestLearned{};

    int64_t last_tell{-1};        /* idnum of last tell from              */
    void *last_olc_targ{};        /* olc control                          */
    int last_olc_mode{};        /* olc control                          */
    int olc_zone{};            /* Zone where OLC is permitted		*/
    int gauntlet{};                 /* Highest Gauntlet Position */
    char *poofin{};            /* Description on arrival of a god.     */
    char *poofout{};        /* Description upon a god's exit.       */
    int speaking{};            /* Language currently speaking		*/

    int8_t conditions[NUM_CONDITIONS]{};        /* Drunk, full, thirsty			*/
    int practice_points{};        /* Skill points earned from race HD	*/

    int wimp_level{0};        /* Below this # of hit points, flee!	*/
    int8_t freeze_level{};        /* Level of god who froze char, if any	*/
    int16_t invis_level{};        /* level of invisibility		*/
    room_vnum load_room{NOWHERE};        /* Which room to place char in		*/
    std::bitset<NUM_PRF_FLAGS> pref{};    /* preference flags for PC's.		*/

    room_vnum normalizeLoadRoom(room_vnum in);

    double getAffectModifier(int location, int specific = -1);

    std::unordered_map<CharAttribute, attribute_t> attributes;
    attribute_t get(CharAttribute attr, bool base = false);
    attribute_t set(CharAttribute attr, attribute_t val);
    attribute_t mod(CharAttribute attr, attribute_t val);

    std::unordered_map<CharTrain, attribute_train_t> trains;
    attribute_train_t get(CharTrain attr);
    attribute_train_t set(CharTrain attr, attribute_train_t val);
    attribute_train_t mod(CharTrain attr, attribute_train_t val);

    // C++ reworking
    std::string juggleRaceName(bool capitalized);

    void restore(bool announce);

    void ghostify();

    void restore_by(char_data *ch);

    void gainTail(bool announce = true);
    void loseTail();
    bool hasTail();

    void resurrect(ResurrectionMode mode);

    void teleport_to(IDXTYPE rnum);

    bool in_room_range(IDXTYPE low_rnum, IDXTYPE high_rnum);

    bool in_past();

    bool is_newbie();

    bool in_northran();

    bool can_tolerate_gravity(int grav);

    int calcTier();

    int64_t calc_soft_cap();

    bool is_soft_cap(int64_t type, long double mult);

    bool is_soft_cap(int64_t type);

    int wearing_android_canister();

    int64_t calcGravCost(int64_t num);

    // Stats stuff

    int64_t getCurHealth();

    int64_t getMaxHealth();

    double getCurHealthPercent();

    int64_t getPercentOfCurHealth(double amt);

    int64_t getPercentOfMaxHealth(double amt);

    bool isFullHealth();

    int64_t setCurHealth(int64_t amt);

    int64_t setCurHealthPercent(double amt);

    int64_t incCurHealth(int64_t amt, bool limit_max = true);

    int64_t decCurHealth(int64_t amt, int64_t floor = 0);

    int64_t incCurHealthPercent(double amt, bool limit_max = true);

    int64_t decCurHealthPercent(double amt, int64_t floor = 0);

    void restoreHealth(bool announce = true);

    int64_t healCurHealth(int64_t amt);

    int64_t harmCurHealth(int64_t amt);

    int64_t getMaxPL();

    int64_t getMaxPLTrans();

    int64_t getCurPL();

    int64_t getBasePL();

    int64_t getEffBasePL();

    double getCurPLPercent();

    int64_t getPercentOfCurPL(double amt);

    int64_t getPercentOfMaxPL(double amt);

    bool isFullPL();

    int64_t getCurKI();

    int64_t getMaxKI();

    int64_t getBaseKI();

    int64_t getEffBaseKI();

    double getCurKIPercent();

    int64_t getPercentOfCurKI(double amt);

    int64_t getPercentOfMaxKI(double amt);

    bool isFullKI();

    int64_t setCurKI(int64_t amt);

    int64_t setCurKIPercent(double amt);

    int64_t incCurKI(int64_t amt, bool limit_max = true);

    int64_t decCurKI(int64_t amt, int64_t floor = 0);

    int64_t incCurKIPercent(double amt, bool limit_max = true);

    int64_t decCurKIPercent(double amt, int64_t floor = 0);

    void restoreKI(bool announce = true);

    int64_t getCurST();

    int64_t getMaxST();

    int64_t getBaseST();

    int64_t getEffBaseST();

    double getCurSTPercent();

    int64_t getPercentOfCurST(double amt);

    int64_t getPercentOfMaxST(double amt);

    bool isFullST();

    int64_t setCurST(int64_t amt);

    int64_t setCurSTPercent(double amt);

    int64_t incCurST(int64_t amt, bool limit_max = true);

    int64_t decCurST(int64_t amt, int64_t floor = 0);

    int64_t incCurSTPercent(double amt, bool limit_max = true);

    int64_t decCurSTPercent(double amt, int64_t floor = 0);

    void restoreST(bool announce = true);

    int64_t getCurLF();

    int64_t getMaxLF();

    double getCurLFPercent();

    int64_t getPercentOfCurLF(double amt);

    int64_t getPercentOfMaxLF(double amt);

    bool isFullLF();

    int64_t setCurLF(int64_t amt);

    int64_t setCurLFPercent(double amt);

    int64_t incCurLF(int64_t amt, bool limit_max = true);

    int64_t decCurLF(int64_t amt, int64_t floor = 0);

    int64_t incCurLFPercent(double amt, bool limit_max = true);

    int64_t decCurLFPercent(double amt, int64_t floor = 0);

    void restoreLF(bool announce = true);


    bool isFullVitals();

    void restoreVitals(bool announce = true);

    void restoreStatus(bool announce = true);

    void restoreLimbs(bool announce = true);

    int64_t gainBasePL(int64_t amt, bool trans_mult = false);

    int64_t gainBaseKI(int64_t amt, bool trans_mult = false);

    int64_t gainBaseST(int64_t amt, bool trans_mult = false);

    void gainBaseAll(int64_t amt, bool Ftrans_mult = false);

    int64_t loseBasePL(int64_t amt, bool trans_mult = false);

    int64_t loseBaseKI(int64_t amt, bool trans_mult = false);

    int64_t loseBaseST(int64_t amt, bool trans_mult = false);

    void loseBaseAll(int64_t amt, bool trans_mult = false);

    int64_t gainBasePLPercent(double amt, bool trans_mult = false);

    int64_t gainBaseKIPercent(double amt, bool trans_mult = false);

    int64_t gainBaseSTPercent(double amt, bool trans_mult = false);

    void gainBaseAllPercent(double amt, bool trans_mult = false);

    int64_t loseBasePLPercent(double amt, bool trans_mult = false);

    int64_t loseBaseKIPercent(double amt, bool trans_mult = false);

    int64_t loseBaseSTPercent(double amt, bool trans_mult = false);

    void loseBaseAllPercent(double amt, bool trans_mult = false);

    // status stuff
    void cureStatusKnockedOut(bool announce = true);

    void cureStatusBurn(bool announce = true);

    void cureStatusPoison(bool announce = true);

    void setStatusKnockedOut();

    // stats refactor stuff
    weight_t getMaxCarryWeight();

    weight_t getEquippedWeight();

    weight_t getCarriedWeight();

    weight_t getAvailableCarryWeight();

    double speednar();

    int64_t getEffMaxPL();

    bool isWeightedPL();

    void apply_kaioken(int times, bool announce);

    void remove_kaioken(int8_t announce);

    double health = 1;
    double energy = 1;
    double stamina = 1;
    double life = 1;

    int getRPP();
    void modRPP(int amt);
    int getPractices();
    void modPractices(int amt);

    bool isProvidingLight();
    double currentGravity();

};


/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
    char *text;
    int aliased;
    struct txt_block *next;
};


struct txt_q {
    struct txt_block *head;
    struct txt_block *tail;
};


struct descriptor_data {
    int64_t id{NOTHING};
    std::map<int64_t, std::shared_ptr<net::Connection>> conns;
    void onConnectionLost(int64_t);
    void onConnectionClosed(int64_t);

    char host[HOST_LENGTH + 1];    /* hostname				*/
    int connected{CON_PLAYING};        /* mode of 'connectedness'		*/

    time_t login_time{time(nullptr)};        /* when the person connected		*/
    char **str{};            /* for the modify-str system		*/
    char *backstr{};        /* backup string for modify-str system	*/
    size_t max_str{};            /* maximum size of string in modify-str	*/
    int32_t mail_to{};        /* name for mail system			*/
    bool has_prompt{true};        /* is the user at a prompt?             */
    std::string last_input;        /* the last input			*/
    std::unique_ptr<net::Channel<std::string>> raw_input_queue;        /* queue of raw unprocessed input		*/
    std::list<std::string> input_queue;
    std::string output;        /* ptr to the current output buffer	*/
    std::list<std::string> history;        /* History of commands, for ! mostly.	*/
    struct char_data *character{};    /* linked to char			*/
    struct char_data *original{};    /* original char if switched		*/
    struct descriptor_data *snooping{}; /* Who is this char snooping	*/
    struct descriptor_data *snoop_by{}; /* And who is snooping this char	*/
    struct descriptor_data *next{}; /* link to next descriptor		*/
    struct oasis_olc_data *olc{};   /* OLC info                            */
    struct account_data *account{}; /* Account info                        */
    int level{};
    char *newsbuf{};
    /*---------------Player Level Object Editing Variables-------------------*/
    int obj_editval{};
    int obj_editflag;
    char *obj_was{};
    char *obj_name{};
    char *obj_short{};
    char *obj_long{};
    int obj_type{};
    int obj_weapon{};
    struct obj_data *obj_point{};
    char *title{};
    double timeoutCounter{0};
    void handle_input();
    void start();
    void handleLostLastConnection(bool graceful);
    void sendText(const std::string &txt);
    void sendGMCP(const std::string &cmd, const nlohmann::json &j);
};

/* used in the socials */
struct social_messg {
    int act_nr;
    char *command;               /* holds copy of activating command */
    char *sort_as;              /* holds a copy of a similar command or
                               * abbreviation to sort by for the parser */
    int hide;                   /* ? */
    int min_victim_position;    /* Position of victim */
    int min_char_position;      /* Position of char */
    int min_level_char;          /* Minimum level of socialing char */

    /* No argument was supplied */
    char *char_no_arg;
    char *others_no_arg;

    /* An argument was there, and a victim was found */
    char *char_found;
    char *others_found;
    char *vict_found;

    /* An argument was there, as well as a body part, and a victim was found */
    char *char_body_found;
    char *others_body_found;
    char *vict_body_found;

    /* An argument was there, but no victim was found */
    char *not_found;

    /* The victim turned out to be the character */
    char *char_auto;
    char *others_auto;

    /* If the char cant be found search the char's inven and do these: */
    char *char_obj_found;
    char *others_obj_found;
};


struct weather_data {
    int pressure{};    /* How is the pressure ( Mb ) */
    int change{};    /* How fast and what way does it change. */
    int sky{};    /* How is the sky. */
    int sunlight{};    /* And how much sun. */
    nlohmann::json serialize();
    void deserialize(const nlohmann::json &j);
};


/*
 * Element in monster and object index-tables.
 *
 * NOTE: Assumes sizeof(mob_vnum) >= sizeof(obj_vnum)
 */
struct index_data {
    mob_vnum vn{NOTHING};    /* virtual number of this mob/obj		*/
    SpecialFunc func;

    char *farg;         /* string argument for special function     */
    struct trig_data *proto;     /* for triggers... the trigger     */
};

/* linked list for mob/object prototype trigger lists */
struct trig_proto_list {
    int vnum;                             /* vnum of the trigger   */
    struct trig_proto_list *next;         /* next trigger          */
};

struct guild_info_type {
    int pc_class;
    room_vnum guild_room;
    int direction;
};

/*
 * Config structs
 * 
 */

/*
* The game configuration structure used for configurating the game play
* variables.
*/
struct game_data {
    int pk_allowed;         /* Is player killing allowed? 	  */
    int pt_allowed;         /* Is player thieving allowed?	  */
    int level_can_shout;      /* Level player must be to shout.	  */
    int holler_move_cost;      /* Cost to holler in move points.	  */
    int tunnel_size;        /* Number of people allowed in a tunnel.*/
    int max_exp_gain;       /* Maximum experience gainable per kill.*/
    int max_exp_loss;       /* Maximum experience losable per death.*/
    int max_npc_corpse_time;/* Num tics before NPC corpses decompose*/
    int max_pc_corpse_time; /* Num tics before PC corpse decomposes.*/
    int idle_void;          /* Num tics before PC sent to void(idle)*/
    int idle_rent_time;     /* Num tics before PC is autorented.	  */
    int idle_max_level;     /* Level of players immune to idle.     */
    int dts_are_dumps;      /* Should items in dt's be junked?	  */
    int load_into_inventory;/* Objects load in immortals inventory. */
    int track_through_doors;/* Track through doors while closed?    */
    int level_cap;          /* You cannot level to this level       */
    int stack_mobs;      /* Turn mob stacking on                 */
    int stack_objs;      /* Turn obj stacking on                 */
    int mob_fighting;       /* Allow mobs to attack other mobs.     */
    char *OK;               /* When player receives 'Okay.' text.	  */
    char *NOPERSON;         /* 'No-one by that name here.'	  */
    char *NOEFFECT;         /* 'Nothing seems to happen.'	          */
    int disp_closed_doors;  /* Display closed doors in autoexit?	  */
    int reroll_player;      /* Players can reroll stats on creation */
    int initial_points;      /* Initial points pool size		  */
    int enable_compression; /* Enable MCCP2 stream compression      */
    int enable_languages;   /* Enable spoken languages              */
    int all_items_unique;   /* Treat all items as unique 		  */
    float exp_multiplier;     /* Experience gain  multiplier	  */
};


/*
 * The rent and crashsave options.
 */
struct crash_save_data {
    int free_rent;          /* Should the MUD allow rent for free?  */
    int max_obj_save;       /* Max items players can rent.          */
    int min_rent_cost;      /* surcharge on top of item costs.	  */
    int auto_save;          /* Does the game automatically save ppl?*/
    int autosave_time;      /* if auto_save=TRUE, how often?        */
    int crash_file_timeout; /* Life of crashfiles and idlesaves.    */
    int rent_file_timeout;  /* Lifetime of normal rent files in days*/
};


/*
 * The room numbers. 
 */
struct room_numbers {
    room_vnum mortal_start_room;    /* vnum of room that mortals enter at.  */
    room_vnum immort_start_room;  /* vnum of room that immorts enter at.  */
    room_vnum frozen_start_room;  /* vnum of room that frozen ppl enter.  */
    room_vnum donation_room_1;    /* vnum of donation room #1.            */
    room_vnum donation_room_2;    /* vnum of donation room #2.            */
    room_vnum donation_room_3;    /* vnum of donation room #3.	        */
};


/*
 * The game operational constants.
 */
struct game_operation {
    uint16_t DFLT_PORT;      /* The default port to run the game.  */
    char *DFLT_IP;            /* Bind to all interfaces.		  */
    char *DFLT_DIR;           /* The default directory (lib).	  */
    char *LOGNAME;            /* The file to log messages to.	  */
    int max_playing;          /* Maximum number of players allowed. */
    int max_filesize;         /* Maximum size of misc files.	  */
    int max_bad_pws;          /* Maximum number of pword attempts.  */
    int siteok_everyone;        /* Everyone from all sites are SITEOK.*/
    int nameserver_is_slow;   /* Is the nameserver slow or fast?	  */
    int use_new_socials;      /* Use new or old socials file ?      */
    int auto_save_olc;        /* Does OLC save to disk right away ? */
    char *MENU;               /* The MAIN MENU.			  */
    char *WELC_MESSG;        /* The welcome message.		  */
    char *START_MESSG;        /* The start msg for new characters.  */
    int imc_enabled; /**< Is connection to IMC allowed ? */
};

/*
 * The Autowizard options.
 */
struct autowiz_data {
    int use_autowiz;        /* Use the autowiz feature?		*/
    int min_wizlist_lev;    /* Minimun level to show on wizlist.	*/
};

/* This is for the tick system.
 *
 */

struct tick_data {
    int pulse_violence;
    int pulse_mobile;
    int pulse_zone;
    int pulse_autosave;
    int pulse_idlepwd;
    int pulse_sanity;
    int pulse_usage;
    int pulse_timesave;
    int pulse_current;
};

/*
 * The character advancement (leveling) options.
 */
struct advance_data {
    int allow_multiclass; /* Allow advancement in multiple classes     */
    int allow_prestige;   /* Allow advancement in prestige classes     */
};

/*
 * The new character creation method options.
 */
struct creation_data {
    int method; /* What method to use for new character creation */
};

/*
 * The main configuration structure;
 */
struct config_data {
    char *CONFFILE;    /* config file path	 */
    struct game_data play;        /* play related config   */
    struct crash_save_data csd;        /* rent and save related */
    struct room_numbers room_nums;    /* room numbers          */
    struct game_operation operation;    /* basic operation       */
    struct autowiz_data autowiz;    /* autowiz related stuff */
    struct advance_data advance;   /* char advancement stuff */
    struct tick_data ticks;        /* game tick stuff 	 */
    struct creation_data creation;    /* char creation method	 */
};

/*
 * Data about character aging
 */
struct aging_data {
    int adult;        /* Adulthood */
    int classdice[3][2];    /* Dice info for starting age based on class age type */
    int middle;        /* Middle age */
    int old;        /* Old age */
    int venerable;    /* Venerable age */
    int maxdice[2];    /* For roll to determine natural death beyond venerable */
};

#ifdef MEMORY_DEBUG
#include "zmalloc.h"
#endif
