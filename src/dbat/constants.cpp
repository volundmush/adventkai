/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/constants.h"

const char *circlemud_version = "CircleMUD, version 3.1";

const char *oasisolc_version = "OasisOLC 2.0.6";

const char *ascii_pfiles_version = "ASCII Player Files 3.0.1";

/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */

/* Combine Attack Names */
const char *attack_names_comp[] = {
        "Kamehameha", /* 0 Roshi */
        "Galik Gun", /* 1 Bardock */
        "Masenko", /* 2 Piccolo */
        "Deathbeam", /* 3 Frieza */
        "Honoo", /* 4 Dabura */
        "Twin Slash", /* 5 Tapion */
        "Hell Flash", /* 6 Sixteen */
        "Psychic Blast", /* 7 Kibito */
        "Crusher Ball", /* 8 Ginyu */
        "Water Spikes", /* 9 Tsuna */
        "Tribeam", /* 10 Krane */
        "Star Breaker", /* 11 Jinto */
        "Seishou Enko", /* 12 Arlian */
        "Renzokou Energy Dan", /* 13 Nail */
        "\n"
};

/* Combine Attack Names */
const char *attack_names[] = {
        "kamehameha", /* 0 Roshi */
        "galik gun", /* 1 Bardock */
        "masenko", /* 2 Piccolo */
        "deathbeam", /* 3 Frieza */
        "honoo", /* 4 Dabura */
        "twin slash", /* 5 Tapion */
        "hell flash", /* 6 Sixteen */
        "psychic blast", /* 7 Kibito */
        "crusher ball", /* 8 Ginyu */
        "water spikes", /* 9 Tsuna */
        "tribeam", /* 10 Krane */
        "star breaker", /* 11 Jinto */
        "seishou enko", /* 12 Arlian */
        "renzokou energy dan", /* 13 Nail */
        "\n"
};

/* Combine Attack Skills */
const int attack_skills[] = {464, 444, 441, 472, 476, 474, 488, 475, 485, 510, 482, 533, 528, 440};

/* NPC Personality Types */
const char *npc_personality[MAX_PERSONALITIES + 1] = {
        "Basic",      /* 0 */
        "Careful",    /* 1 */
        "Aggressive", /* 2 */
        "Arrogant",   /* 3 */
        "Intelligent", /* 4 */
        "\n"
};


/* Alignments */
/* Taken from the SRD under OGL, see ../doc/srd.txt for information */
const char *alignments[NUM_ALIGNS + 1] = {
        "Saintly",
        "Valiant",
        "Hero",
        "Do-gooder",
        "Neutral",
        "Crook",
        "Villain",
        "Terrible",
        "Horribly Evil",
        "\n",
};

const char *aura_types[9] = {
        "white",
        "blue",
        "red",
        "green",
        "pink",
        "purple",
        "yellow",
        "black",
        "orange"
};

/* Armor Types */
const char *armor_type[MAX_ARMOR_TYPES + 1] = {
        "Undefined",
        "Light",
        "Medium",
        "Heavy",
        "Shield",
        "\n"
};

/* Weapon Types */
/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
const char *weapon_type[MAX_WEAPON_TYPES + 2] = {
        "undefined",
        "unarmed",
        "dagger",
        "mace",
        "sickle",
        "spear",
        "staff",
        "crossbow",
        "longbow",
        "shortbow",
        "sling",
        "shuriken",
        "hammer",
        "lance",
        "flail",
        "longsword",
        "shortsword",
        "greatsword",
        "rapier",
        "scimitar",
        "polearm",
        "club",
        "bastard sword",
        "monk weapon",
        "double weapon",
        "axe",
        "whip",
        "\n"
};

/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
const char *crit_type[NUM_CRIT_TYPES + 1] =
        {
                "x2",
                "x3",
                "x4",
                "\n"
        };

/* cardinal directions */
const char *dirs[NUM_OF_DIRS + 1] =
        {
                "north",
                "east",
                "south",
                "west",
                "up",
                "down",
                "northwest",
                "northeast",
                "southeast",
                "southwest",
                "inside",
                "outside",
                "\n"
        };

const char *abbr_dirs[NUM_OF_DIRS + 1] =
        {
                "n",
                "e",
                "s",
                "w",
                "u",
                "d",
                "nw",
                "ne",
                "se",
                "sw",
                "in",
                "out",
                "\n"
        };

/* For Distinguishing Features */
const char *eye_types[13] = {
        "blue",
        "black",
        "green",
        "brown",
        "red",
        "aqua",
        "pink",
        "purple",
        "crimson",
        "gold",
        "amber",
        "emerald",
        "\n"
};

const char *hairl_types[6] = {
        "bald",
        "short",
        "medium",
        "long",
        "really long",
        "\n"
};

const char *FHA_types[6] = {
        "stubby",
        "short",
        "medium",
        "long",
        "really long",
        "\n"
};

const char *hairc_types[15] = {
        "headed",
        "black",
        "brown",
        "blonde",
        "grey",
        "red",
        "orange",
        "green",
        "blue",
        "pink",
        "purple",
        "silver",
        "crimson",
        "white",
        "\n"
};

const char *hairs_types[13] = {
        "",
        "with a plain look",
        "in a mohawk",
        "with a spiky look",
        "with a curly look",
        "with an uneven look",
        "in a ponytail",
        "in an afro",
        "with a fade look",
        "in a crew cut",
        "with a feathered look",
        "in dred locks",
        "\n"
};

const char *skin_types[13] = {
        "white",
        "black",
        "green",
        "orange",
        "yellow",
        "red",
        "grey",
        "blue",
        "aqua",
        "pink",
        "purple",
        "tan",
        "\n"
};

/* ZONE_x */
const char *zone_bits[NUM_ZONE_FLAGS + 1] = {
        "CLOSED",
        "NO_IMMORT",
        "QUEST",
        "DBALL_LOAD",
        "SPARE2",
        "SPARE3",
        "SPARE4",
        "SPARE5",
        "SPARE6",
        "SPARE7",
        "SPARE8",
        "SPARE9",
        "SPARE10",
        "SPARE11",
        "SPARE12",
        "SPARE13",
        "SPARE14",
        "SPARE15",
        "SPARE16",
        "SPARE17",
        "SPARE18",
        "SPARE19",
        "SPARE20",
        "SPARE21",
        "SPARE22",
        "SPARE23",
        "SPARE24",
        "SPARE25",
        "SPARE26",
        "SPARE27",
        "SPARE28",
        "SPARE29",
        "SPARE30",
        "SPARE31",
        "SPARE32",
        "SPARE33",
        "\n"
};

/* ROOM_x */
const char *room_bits[NUM_ROOM_FLAGS + 1] = {
        "DARK",
        "DEATH",
        "NO_MOB",
        "LIGHT",
        "PEACEFUL",
        "SOUNDPROOF",
        "NO_TRACK",
        "NO_INSTANT",
        "TUNNEL",
        "PRIVATE",
        "GODROOM",
        "HOUSE",
        "HCRSH",
        "ATRIUM",
        "OLC",
        "*",                /* BFS MARK */
        "VEHICLE",
        "UNDERGROUND",
        "CURRENT",
        "TIMED_DT",
        "EARTH",
        "VEGETA",
        "FRIGID",
        "KONACK",
        "NAMEK",
        "NEO",
        "AL",
        "SPACE",
        "Punishment Hell",
        "Regen",
        "Hell",
        "GravX10",
        "Aether",
        "HBTC",
        "Past",
        "CBANK",
        "SHIP",
        "YARDRAT",
        "KANASSA",
        "ARLIA",
        "AURA",
        "EARTHORB",
        "FRIGIDOR",
        "KONACKOR",
        "NAMEKORB",
        "VEGETAOR",
        "AETHEROR",
        "YARDRAOR",
        "KANASSOR",
        "ARLIAORB",
        "NEBULA",
        "ASTEROID",
        "WORMHOLE",
        "SSTATION",
        "ISSTAR",
        "CERRIA",
        "CERORBIT",
        "BEDROOM",
        "WORKOUT",
        "GARDEN",
        "GREENHOUSE",
        "FERTILE1",
        "FERTILE2",
        "FISHING",
        "FRESHWATFISH",
        "CANREMODEL",
        "LANDING",
        "SAVE",
        "\n"
};


/* EX_x */
const char *exit_bits[NUM_EXIT_FLAGS + 1] = {
        "DOOR",
        "CLOSED",
        "LOCKED",
        "PICKPROOF",
        "SECRET",
        "\n"
};


/* SECT_ */
const char *sector_types[NUM_ROOM_SECTORS + 1] = {
        "Inside",
        "City",
        "Plain",
        "Forest",
        "Hills",
        "Mountains",
        "Shallows",
        "Water (Fly)",
        "Sky",
        "Underwater",
        "$Shop",
        "#Important",
        "Desert",
        "Space",
        "Lava",
        "\n"
};


/*
 * SEX_x
 * Not used in sprinttype() so no \n.
 */
const char *genders[NUM_SEX + 1] =
        {
                "neutral",
                "male",
                "female",
                "\n"
        };


/* POS_x */
const char *position_types[NUM_POSITIONS + 1] = {
        "Dead",
        "Mortally wounded",
        "Incapacitated",
        "Stunned",
        "Sleeping",
        "Resting",
        "Sitting",
        "Fighting",
        "Standing",
        "\n"
};

const char *song_types[] = {
        "\n",
        "Safety",
        "Shielding",
        "Shadow Stitch",
        "Teleportation",
        "Teleportation",
        "Teleportation",
        "Teleportation",
        "Teleportation",
        "Teleportation",
        "Teleportation",
        "Teleportation",
        "\n"
};

/* PLR_x */
const char *player_bits[NUM_PLR_FLAGS + 1] = {
        "KILLER",
        "ROGUE",
        "FROZEN",
        "DONTSET",
        "WRITING",
        "MAILING",
        "CSH",
        "SITEOK",
        "MUTED",
        "NOTITLE",
        "DELETED",
        "LOADRM",
        "NO_WIZL",
        "NO_DEL",
        "INVST",
        "CRYO",
        "DEAD",    /* You should never see this. */
        "MID_AGE_POS",
        "MID_AGE_NEG",
        "OLD_AGE_POS",
        "OLD_AGE_NEG",
        "VEN_AGE_POS",
        "VEN_AGE_NEG",
        "RIP_OLD_AGE",
        "RARM",
        "LARM",
        "RLEG",
        "LLEG",
        "HEAD",
        "STAIL",
        "TAIL",
        "PILOTING",
        "CCONLY",
        "Spar",
        "Charging",
        "Trans1",
        "Trans2",
        "Trans3",
        "Trans4",
        "Trans5",
        "Trans6",
        "Absorb",
        "Repair",
        "Powsen",
        "POWERUP",
        "KO",
        "CRARM",
        "CLARM",
        "CRLEG",
        "CLLEG",
        "FPSSJ",
        "IMMORTALITY",
        "EYECLOSED",
        "DISGUISED",
        "BANDAGE",
        "PR",
        "HEALT",
        "FURY",
        "POSE",
        "OOZARU",
        "ABSORBED",
        "MULTP",
        "PDEATH",
        "THANDW",
        "SELFD",
        "SELFD2",
        "SPIRAL",
        "BIOGR",
        "LSSJ",
        "RLEARN",
        "FORGET",
        "TRANSMISSION",
        "FISHING",
        "GOOCHUNKS",
        "MULTIHIT",
        "AURALIGHT",
        "RDISPLAY",
        "ROBBED",
        "TAILH",
        "NOGROW",
        "\n"
};


/* MOB_x */
const char *action_bits[NUM_MOB_FLAGS + 1] = {
        "SPEC",
        "SENTINEL",
        "NOSCAVENGER",
        "ISNPC",
        "AWARE",
        "AGGR",
        "STAY-ZONE",
        "WIMPY",
        "AGGR_EVIL",
        "AGGR_GOOD",
        "AGGR_NEUTRAL",
        "MEMORY",
        "HELPER",
        "NO_CHARM",
        "NO_SUMMN",
        "NO_SLEEP",
        "AUTOBALANCE",
        "NO_BLIND",
        "NO_KILL",
        "DEAD",    /* You should never see this. */
        "MOUNTABLE",
        "Right Arm",
        "Left Arm",
        "Right Leg",
        "Left Leg",
        "Head",
        "JUST_DESC",
        "HUSK",
        "SPAR",
        "DUMMY",
        "ABSORB",
        "REPAIR",
        "NOPOISON",
        "KNOWKAIO",
        "POWERINGUP",
        "\n"
};


/* PRF_x */
const char *preference_bits[NUM_PRF_FLAGS + 1] = {
        "BRIEF",
        "COMPACT",
        "DEAF",
        "NO_TELL",
        "D_HP",
        "D_MANA",
        "D_MOVE",
        "AUTOEX",
        "NO_HASS",
        "QUEST",
        "SUMN",
        "NO_REP",
        "LIGHT",
        "COLOR",
        "SPARE",
        "NO_WIZ",
        "L1",
        "L2",
        "NO_AUC",
        "NO_GOS",
        "NO_GTZ",
        "RMFLG",
        "D_AUTO",
        "CLS",
        "BLDWLK",
        "AFK",
        "AUTOLOOT",
        "AUTOGOLD",
        "AUTOSPLIT",
        "FULL_AUTOEX",
        "AUTOSAC",
        "AUTOMEM",
        "VIEWORDER",
        "NO_COMPRESS",
        "AUTOASSIST",
        "D_KI",
        "D_EXP",
        "D_TNL",
        "TESTING",
        "WHOHIDE",
        "NOMAILW",
        "HINTS",
        "FURYM",
        "NODEC",
        "NOEQSEE",
        "NOMUSIC",
        "LKEEP",
        "DISTIME",
        "DISGOLD",
        "DISPRAC",
        "NOPARRY",
        "H-T",
        "PERCENT",
        "CARVE",
        "ARENAW",
        "NOGIVE",
        "INSTRUCT",
        "GHEALTH",
        "IHEALTH",
        "ENERGIZE",
        "\n"
};


/* AFF_x */
/* Many are taken from the SRD under OGL, see ../doc/srd.txt for information */
const char *affected_bits[NUM_AFF_FLAGS + 1] =
        {
                "\0", /* DO NOT REMOVE!! */
                "BLIND",
                "INVIS",
                "DET-ALIGN",
                "DET-INVIS",
                "DET-MAGIC",
                "SENSE-LIFE",
                "WATWALK",
                "BARRIER",
                "GROUP",
                "CURSE",
                "INFRAVISION",
                "POISON",
                "WEAKENED",
                "PROT-GOOD",
                "SLEEP",
                "NO_TRACK",
                "UNDEAD",
                "STONED",
                "SNEAK",
                "HIDE",
                "UNUSED",
                "CHARM",
                "FLYING",
                "WATER-BREATH",
                "ANGELIC",
                "ETHEREAL",
                "MAGICONLY",
                "NEXTPARTIAL",
                "NEXTNOACT",
                "STUNNED",
                "TAME",
                "CDEATH",
                "SPIRIT",
                "STONESKIN",
                "SUMMONED",
                "CELESTIAL",
                "FIENDISH",
                "FIRESHIELD",
                "LOW_LIGHT",
                "ZANZOKEN",
                "KO",
                "MIGHT",
                "FLEX",
                "GENIUS",
                "BLESS",
                "BURNT",
                "BURNED",
                "MBREAK",
                "HASS",
                "FS",
                "PARALYZE",
                "INFUSED",
                "ENLIGHT",
                "FROZOWNED",
                "FIRESHIELD",
                "ENSNARED",
                "HAYASA",
                "PURSUED",
                "WITHERED",
                "HYDROZAP",
                "POSITION",
                "SHOCKED",
                "METAMORPH",
                "HEALING-GLOW",
                "ALGIZ RUNE",
                "OAGAZ RUNE",
                "WUNJO RUNE",
                "PURISAZ RUNE",
                "ASHED",
                "PUKED",
                "LIQUEFIED",
                "SHELLED",
                "IMMUNITY",
                "SPIRITCONTROL",
                "FIGHTINGPOSE",
                "KYODAIKA",
                "SHADOWSTITCH",
                "ETHEREALCHAINS_DEBUFF",
                "STARPHASE",
                "MINDBREAK_DEBUFF",
                "\n"
        };


/* CON_x */
const char *connected_types[NUM_CON_TYPES + 1] = {
        "Playing",
        "Disconnecting",
        "Get name",
        "Confirm name",
        "Get password",
        "Get new PW",
        "Confirm new PW",
        "Select sex",
        "Select class",
        "Reading MOTD",
        "Main Menu",
        "Get descript.",
        "Changing PW 1",
        "Changing PW 2",
        "Changing PW 3",
        "Self-Delete 1",
        "Self-Delete 2",
        "Disconnecting",
        "Object edit",
        "Room edit",
        "Zone edit",
        "Mobile edit",
        "Shop edit",
        "Text edit",
        "Config edit",
        "Select race",
        "Assembly Edit",
        "Social edit",
        "Trigger Edit",
        "Race help",
        "Class help",
        "Query ANSI",
        "Guild edit",
        "Reroll stats",
        "iObject Edit",
        "Level Up",
        "Qstats",
        "Hair Length",
        "Hair Color",
        "Hair Style",
        "Skin Color",
        "Eye Color",
        "Start Q1",
        "Start Q2",
        "Start Q3",
        "Start Q4",
        "Start Q5",
        "Start Q6",
        "Start Q7",
        "Start Q8",
        "Start Q9",
        "Start Q10",
        "House Edit",
        "Alpha",
        "Alpha 2",
        "Android Menu",
        "Help Edit",
        "Get User",
        "Get Email",
        "Get UMenu",
        "Confirm User",
        "Dist. Feature",
        "Height",
        "Weight",
        "Aura",
        "Bonus/Neg",
        "Bonus/Neg",
        "News Edit",
        "Racial Pref",
        "Player Obj Edit",
        "Align Menu",
        "Skills Menu",
        "User Title",
        "Genome Selection",
        "\n"
};


/*
 * WEAR_x - for eq list
 * Not use in sprinttype() so no \n.
 */
const char *wear_where[NUM_WEARS + 1] = {
        "@c<@CUsed Somewhere     @c>@n ",
        "@c<@CWorn On Finger     @c>@n ",
        "@c<@CWorn On Finger     @c>@n ",
        "@c<@CWorn Around Neck   @c>@n ",
        "@c<@CWorn Around Neck   @c>@n ",
        "@c<@CWorn On Body       @c>@n ",
        "@c<@CWorn On Head       @c>@n ",
        "@c<@CWorn On Legs       @c>@n ",
        "@c<@CWorn On Feet       @c>@n ",
        "@c<@CWorn On Hands      @c>@n ",
        "@c<@CWorn On Arms       @c>@n ",
        "@c<@CWorn Somewhere     @c>@n ",
        "@c<@CWorn About Body    @c>@n ",
        "@c<@CWorn About Waist   @c>@n ",
        "@c<@CWorn Around Wrist  @c>@n ",
        "@c<@CWorn Around Wrist  @c>@n ",
        "@c<@CWielded            @c>@n ",
        "@c<@COffhand            @c>@n ",
        "@c<@CWorn On Back       @c>@n ",
        "@c<@CWorn In Ear        @c>@n ",
        "@c<@CWorn In Ear        @c>@n ",
        "@c<@CWorn On Shoulders  @c>@n ",
        "@c<@CWorn On Eye        @c>@n ",
        "\n"
};


/* WEAR_x - for stat */
const char *equipment_types[NUM_WEARS + 1] = {
        "Used as light",
        "Worn on right finger",
        "Worn on left finger",
        "First worn around Neck",
        "Second worn around Neck",
        "Worn on body",
        "Worn on head",
        "Worn on legs",
        "Worn on feet",
        "Worn on hands",
        "Worn on arms",
        "Worn as shield",
        "Worn about body",
        "Worn around waist",
        "Worn around right wrist",
        "Worn around left wrist",
        "Wielded",
        "Held",
        "Worn on back",
        "Worn in ear",
        "Worn in ear",
        "Worn on shoulders",
        "Worn on eye",
        "\n"
};


/* ITEM_x (ordinal object types) */
const char *item_types[NUM_ITEM_TYPES + 1] = {
        "UNDEFINED",
        "LIGHT",
        "SCROLL",
        "WAND",
        "STAFF",
        "WEAPON",
        "FIRE WEAPON",
        "CAMPFIRE",
        "TREASURE",
        "ARMOR",
        "SENSU",
        "WORN",
        "OTHER",
        "TRASH",
        "TRAP",
        "CONTAINER",
        "NOTE",
        "LIQCONTAINER",
        "KEY",
        "FOOD",
        "MONEY",
        "PEN",
        "BOAT",
        "FOUNTAIN",
        "VEHICLE",
        "HATCH",
        "WINDOW",
        "CONTROL",
        "PORTAL",
        "SPELLBOOK",
        "BOARD",
        "CHAIR",
        "BED",
        "YUM",
        "PLANT",
        "FISHINGPOLE",
        "FISHBAIT",
        "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[NUM_ITEM_WEARS + 1] = {
        "TAKE",
        "FINGER",
        "NECK",
        "BODY",
        "HEAD",
        "LEGS",
        "FEET",
        "HANDS",
        "ARMS",
        "SHIELD",
        "ABOUT",
        "WAIST",
        "WRIST",
        "WIELD",
        "HOLD",
        "BACK",
        "EAR",
        "SHOULDERS",
        "SCOUTER",
        "\n"
};


/* ITEM_x (extra bits) */
const char *extra_bits[NUM_ITEM_FLAGS + 1] = {
        "GLOW",
        "HUM",
        "NO_RENT",
        "NO_DONATE",
        "NO_INVIS",
        "INVISIBLE",
        "MAGIC",
        "NO_DROP",
        "BLESS",
        "ANTI_GOOD",
        "ANTI_EVIL",
        "ANTI_NEUTRAL",
        "ANTI_ROSHI",
        "ANTI_PICCOLO",
        "ANTI_KRANE",
        "ANTI_NAIL",
        "NO_SELL",
        "ANTI_TAPION",
        "2H",
        "ANTI_ANDSIX",
        "ANTI_DABURA",
        "ANTI_GINYU",
        "ANTI_HUMAN",
        "ANTI_DWARF",
        "ANTI_ELF",
        "ANTI_GNOME",
        "UNIQUE",
        "BROKEN",
        "UNBREAKABLE",
        "ANTI_BARDOCK",
        "ANTI_KABITO",
        "ANTI_FRIEZA",
        "DOUBLE",
        "ONLY_ROSHI",
        "ONLY_PICCOLO",
        "ONLY_KRANE",
        "ONLY_NAIL",
        "ONLY_TAPION",
        "ONLY_ANDSIX",
        "ONLY_DABURA",
        "ONLY_GINYU",
        "ONLY_HUMAN",
        "ONLY_ICER",
        "ONLY_SAIYAN",
        "ONLY_KONATSU",
        "ONLY_BARDOCK",
        "ONLY_KABITO",
        "ONLY_FRIEZA",
        "ANTI_ARC_ARCH",
        "ANTI_ARC_TRICK",
        "ANTI_KURZAK",
        "ONLY_KURZAK",
        "ANTI_BLACKGUARD",
        "ANTI_DRAG_DIS",
        "ANTI_DUEL",
        "ANTI_DWAR_DEF",
        "ANTI_EL_KNIGHT",
        "ANTI_HIERO",
        "ANTI_HORI_WALK",
        "ANTI_LORE",
        "ANTI_MYSTIC_TH",
        "ANTI_SHADOW",
        "ANTI_THAUM",
        "Basic Scouter",
        "Midi Scouter",
        "Advanced Scouter",
        "Ultra Scouter",
        "Weapon Lvl 1",
        "Weapon Lvl 2",
        "Weapon Lvl 3",
        "Weapon Lvl 4",
        "Weapon Lvl 5",
        "Clan Board",
        "FORGED",
        "SHEATH",
        "ONLY_JINTO",
        "BURIED",
        "SLOT1",
        "SLOT2",
        "Token",
        "ONEFILL",
        "FILLED",
        "RESTRING",
        "CUSTOM",
        "PROTECTED",
        "NORANDOMSTATS",
        "THROW",
        "HOT",
        "DON'T USE",
        "MELTING",
        "DUPLICATE",
        "MATUREPLANT",
        "CARDCASE",
        "MOB_NOPICKUP",
        "NOSTEAL",
        "RESTRING",
        "\n"
};

/* APPLY_x */
const char *apply_types[NUM_APPLIES + 1] = {
        "NONE",
        "STR",
        "AGL",
        "INT",
        "WIS",
        "CON",
        "SPD",
        "SPIRIT",
        "LEVEL",
        "AGE",
        "CHAR_WEIGHT",
        "CHAR_HEIGHT",
        "MAXKI",
        "MAXPL",
        "MAXST",
        "GOLD",
        "EXP_GAIN_MULT",
        "ARMOR",
        "FISHBONUS",
        "DAMAGE",
        "REGEN RATE",
        "AUTO-TRAIN SKILL",
        "LIFEFORCE MAX",
        "DAMAGE_PERC",
        "DEFENSE_PERC",
        "PL_MULT",
        "KI_MULT",
        "ST_MULT",
        "LF_MULT",
        "VITALS_MULT",
        "HEIGHT_MULT",
        "WEIGHT_MULT",
        "PHYS_DAM_PERC",
        "KI_DAM_PERC",
        "PHYS_DAM_RES",
        "KI_DAM_RES",
        "DAM_ATK_TIER",
        "MAX_KI",
        "FORTITUDE",
        "REFLEX",
        "WILL",
        "SKILL",
        "FEAT",
        "ALL_SAVES",
        "RESISTANCE",
        "ALL_ATTRS",
        "ALL_VITALS",
        "SKILL_SLOTS",
        "ATTR_TRAIN_COST",
        "PS_GAIN_MULT",
        "TRANS_ST_UPKEEP",
        "VITALS_GAIN_MULT",
        "PL_GAIN_MULT",
        "ST_GAIN_MULT",
        "KI_GAIN_MULT",
        "LF_GAIN_MULT",

        "\n"
};


/* CONT_x */
const char *container_bits[NUM_CONT_FLAGS + 1] = {
        "CLOSEABLE",
        "PICKPROOF",
        "CLOSED",
        "LOCKED",
        "\n",
};


/* LIQ_x */
const char *drinks[NUM_LIQ_TYPES + 1] =
        {
                "water",
                "beer",
                "wine",
                "ale",
                "dark ale",
                "whisky",
                "lemonade",
                "firebreather",
                "local speciality",
                "juice",
                "milk",
                "tea",
                "coffee",
                "blood",
                "salt water",
                "clear water",
                "\n"
        };

/* MATERIAL_ */
const char *material_names[NUM_MATERIALS + 1] = {
        "bone",
        "ceramic",
        "copper",
        "diamond",
        "gold",
        "iron",
        "leather",
        "mithril",
        "obsidian",
        "steel",
        "stone",
        "silver",
        "wood",
        "glass",
        "organic material",
        "currency",
        "paper",
        "cotton",
        "satin",
        "silk",
        "burlap",
        "velvet",
        "platinum",
        "adamantine",
        "wool",
        "onyx",
        "ivory",
        "brass",
        "marble",
        "bronze",
        "kachin",
        "ruby",
        "sapphire",
        "emerald",
        "gemstone",
        "granite",
        "energy",
        "hemp",
        "crystal",
        "earth",
        "liquid",
        "cloth",
        "metal",
        "wax",
        "other",
        "food material",
        "oil",
        "\n"
};


/* Taken the SRD under OGL, see ../doc/srd.txt for information */
const char *domains[NUM_DOMAINS + 1] = {
        "Undefined",
        "Air",
        "Animal",
        "Chaos",
        "Death",
        "Destruction",
        "Earth",
        "Evil",
        "Fire",
        "Good",
        "Healing",
        "Knowledge",
        "Law",
        "Luck",
        "Magic",
        "Plant",
        "Protection",
        "Strength",
        "Sun",
        "Travel",
        "Trickery",
        "War",
        "Water",
        "\n",
};

/* Taken the SRD under OGL, see ../doc/srd.txt for information */
const char *schools[NUM_SCHOOLS + 1] = {
        "Undefined",
        "Abjuration",
        "Conjuration",
        "Divination",
        "Enchantment",
        "Evocation",
        "Illusion",
        "Necromancy",
        "Transmutation",
        "Universal",
        "\n",
};


const char *history_types[NUM_HIST + 1] = {
        "all",
        "say",
        "ooc",
        "wiznet",
        "tell",
        "shout",
        "grats",
        "holler",
        "newbie",
        "snet",
        "\n"
};


/* Constants for Assemblies    *****************************************/
const char *AssemblyTypes[MAX_ASSM + 1] = {
        "Don't Use",
        "Don't Use",
        "build",
        "Don't Use",
        "Don't Use",
        "Don't Use",
        "Don't Use",
        "Don't Use",
        "Don't Use",
        "Don't Use",
        "Don't Use",
        "\n"
};

/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[NUM_LIQ_TYPES + 1] =
        {
                "water",
                "beer",
                "wine",
                "ale",
                "ale",
                "whisky",
                "lemonade",
                "firebreather",
                "local",
                "juice",
                "milk",
                "tea",
                "coffee",
                "blood",
                "salt",
                "water",
                "\n"
        };


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
/* (Drunk, Hunger, Thirst) */
int drink_aff[NUM_LIQ_TYPES][NUM_CONDITIONS] = {
        {0, 0, 5},
        {3, 1, 1},
        {3, 1, 1},
        {3, 0, 2},
        {4, 0, 2},
        {6, 0, 0},
        {0, 0, 4},
        {6, 0, 0},
        {0, 0, 3},
        {0, 0, 4},
        {0, 1, 3},
        {0, 0, 3},
        {0, 0, 3},
        {0, 0, 2},
        {0, 0, 1},
        {0, 0, 8}
};

/* color of the various drinks */
const char *color_liquid[NUM_LIQ_TYPES + 1] =
        {
                "clear",
                "brown",
                "clear",
                "brown",
                "dark",
                "golden",
                "red",
                "green",
                "brown",
                "light green",
                "white",
                "brown",
                "black",
                "red",
                "clear",
                "crystal clear",
                "\n"
        };


/*
 * level of fullness for drink containers
 * Not used in sprinttype() so no \n.
 */
const char *fullness[NUM_FULLNESS + 1] =
        {
                "empty ",
                "less than half ",
                "half full ",
                "more than half full ",
                "full ",
                ""
        };


/* mob trigger types */
const char *trig_types[NUM_MTRIG_TYPES + 1] = {
        "Global",
        "Random",
        "Command",
        "Speech",
        "Act",
        "Death",
        "Greet",
        "Greet-All",
        "Entry",
        "Receive",
        "Fight",
        "HitPrcnt",
        "Bribe",
        "Load",
        "Memory",
        "Cast",
        "Leave",
        "Door",
        "UNUSED",
        "Time",
        "EveryHour",
        "Every15Min",
        "\n"
};


/* obj trigger types */
const char *otrig_types[NUM_OTRIG_TYPES + 1] = {
        "Global",
        "Random",
        "Command",
        "UNUSED",
        "UNUSED",
        "Timer",
        "Get",
        "Drop",
        "Give",
        "Wear",
        "UNUSED",
        "Remove",
        "UNUSED",
        "Load",
        "UNUSED",
        "Cast",
        "Leave",
        "UNUSED",
        "Consume",
        "Time",
        "EveryHour",
        "Every15Min",
        "\n"
};


/* wld trigger types */
const char *wtrig_types[NUM_WTRIG_TYPES + 1] = {
        "Global",
        "Random",
        "Command",
        "Speech",
        "UNUSED",
        "Zone Reset",
        "Enter",
        "Drop",
        "UNUSED",
        "UNUSED",
        "UNUSED",
        "UNUSED",
        "UNUSED",
        "UNUSED",
        "UNUSED",
        "Cast",
        "Leave",
        "Door",
        "UNUSED",
        "Time",
        "EveryHour",
        "Every15Min",
        "\n"
};


/* Taken the SRD under OGL, see ../doc/srd.txt for information */
const char *size_names[NUM_SIZES + 1] = {
        "fine",
        "diminutive",
        "tiny",
        "small",
        "medium",
        "large",
        "huge",
        "gargantuan",
        "colossal",
        "\n"
};


int rev_dir[NUM_OF_DIRS] =
        {
                /* North */ SOUTH,
                /* East  */ WEST,
                /* South */ NORTH,
                /* West  */ EAST,
                /* Up    */ DOWN,
                /* Down  */ UP,
                /* NW    */ SOUTHEAST,
                /* NE    */ SOUTHWEST,
                /* SE    */ NORTHWEST,
                /* SW    */ NORTHEAST,
                /* In    */ OUTDIR,
                /* Out   */ INDIR
        };

int movement_loss[NUM_ROOM_SECTORS] =
        {
                1,    /* Inside     */
                1,    /* City       */
                1,    /* Field      */
                1,    /* Forest     */
                1,    /* Hills      */
                1,    /* Mountains  */
                1,    /* Swimming   */
                1,    /* Unswimable */
                1,    /* Flying     */
                1,    /* Underwater */
                1,    /* Shop       */
                1,    /* Important  */
                1,    /* Desert     */
                1
        };

/* Not used in sprinttype(). */
const char *weekdays[NUM_WEEK_DAYS] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
};


/* Not used in sprinttype(). */
const char *month_name[NUM_MONTHS] = {
        "Month of January",        /* 0 */
        "Month of February",
        "Month of March",
        "Month of April",
        "Month of May",
        "Month of June",
        "Month of July",
        "Month of August",
        "Month of September",
        "Month of October",
        "Month of November",
        "Month of December"
};


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
const char *wield_names[NUM_WIELD_NAMES + 1] = {
        "if you were bigger",
        "with ease",
        "one-handed",
        "two-handed",
        "\n"
};


const char *admin_level_names[ADMLVL_IMPL + 2] = {
        "Mortal",
        "Enforcer",
        "1st C. Enforcer",
        "High Enforcer",
        "Vice-Admin",
        "Admin",
        "Implementor",
        "\n",
};


/* Derived from the SRD under OGL, see ../doc/srd.txt for information */
const struct aging_data racial_aging_data[NUM_RACES] = {
/*                   adult	start1	start2	start3	middle	old	vener.	maxdice	*/
/* HUMAN        */ {15,  {{1, 4}, {1, 6}, {2,  6}}, 35,  53,  70,  {2, 20}},
/* ELF          */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* GNOME        */
                   {40,  {{4, 6}, {6, 6}, {9,  6}}, 100, 150, 200, {3, 100}},
/* DWARF        */
                   {40,  {{3, 6}, {5, 6}, {7,  6}}, 125, 188, 250, {2, 100}},
/* HALF_ELF     */
                   {20,  {{1, 6}, {2, 6}, {3,  6}}, 62,  93,  125, {3, 20}},
/* HALFLING     */
                   {20,  {{2, 4}, {3, 6}, {4,  6}}, 50,  75,  100, {5, 20}},
/* DROW_ELF     */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* HALF_ORC     */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* ANIMAL       */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* CONSTRUCT    */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* DEMON        */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* DRAGON       */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* FISH         */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* GIANT        */
                   {15,  {{1, 4}, {1, 6}, {2,  6}}, 35,  53,  70,  {2, 20}},
/* GOBLIN       */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* INSECT       */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* ORC          */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* SNAKE        */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* TROLL        */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* MINOTAUR     */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* KOBOLD       */
                   {14,  {{1, 4}, {1, 6}, {2,  6}}, 30,  45,  60,  {2, 10}},
/* LIZARDFOLK   */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* WARHOST      */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
/* FAERIE       */
                   {110, {{4, 6}, {6, 6}, {10, 6}}, 175, 263, 350, {4, 100}},
};


/* Administrative flags */
const char *admin_flag_names[] = {
        "TellAll",
        "SeeInventory",
        "SeeSecret",
        "KnowWeather",
        "FullWhere",
        "Money",
        "EatAnything",
        "NoPoison",
        "WalkAnywhere",
        "NoKeys",
        "InstantKill",
        "NoSteal",
        "TransAll",
        "SwitchMortal",
        "ForceMass",
        "AllHouses",
        "NoDamage",
        "AllShops",
        "CEDIT",
        "\n"
};


const char *spell_schools[NUM_SCHOOLS + 1] = {
        "Abjuration",
        "Conjuration",
        "Divination",
        "Enchantment",
        "Evocation",
        "Illusion",
        "Necromancy",
        "Transmutation",
        "Universal",
        "\n"
};


const char *cchoice_names[NUM_COLOR + 1] = {
        "normal",
        "room names",
        "room objects",
        "room people",
        "someone hits you",
        "you hit someone",
        "other hit",
        "critical hit",
        "holler",
        "shout",
        "gossip channel",
        "auction channel",
        "congratulations",
        "tells",
        "you say",
        "room say",
        "\n"
};

/* --- End of constants arrays. --- */

/*
 * Various arrays we count so we can check the world files.  These
 * must be at the bottom of the file so they're pre-declared.
 */
size_t room_bits_count = sizeof(room_bits) / sizeof(room_bits[0]) - 1,
        action_bits_count = sizeof(action_bits) / sizeof(action_bits[0]) - 1,
        affected_bits_count = sizeof(affected_bits) / sizeof(affected_bits[0]) - 1,
        extra_bits_count = sizeof(extra_bits) / sizeof(extra_bits[0]) - 1,
        wear_bits_count = sizeof(wear_bits) / sizeof(wear_bits[0]) - 1;

const char *creation_methods[NUM_CREATION_METHODS + 1] =
        {
                "[Standard - Random rolls, ordered assignment]",
                "[Player Assigned - Random rolls, player adjust to taste]",
                "[Points Pool - Player assigns scores from points pool]",
                "[Racial Template - All races start from template values, adjusted by class]",
                "[Class Template - All classes start from template values, adjusted by race]",
                "\n"
        };

/* ADM_x */
const char *admin_flags[NUM_ADMFLAGS + 1] = {
        "You may use 'page all'",
        "You can see other players inventories",
        "You may see secret doors",
        "You may know weather data",
        "You know fully where this are",
        "You do not require money",
        "You can eat anything",
        "You cannot be poisoned",
        "You can walk anywhere",
        "You do not require keys",
        "You have the touch of death",
        "You cannot be stolen from",
        "You may use 'trans all'",
        "You can use 'switch'",
        "You may use 'force all'",
        "You may enter any house",
        "You do not take damage",
        "You may use any shop",
        "You may use cedit",
        "\n"
};

const char *list_bonus[] = {
        "Thrifty", /* Bonus 0 */
        "Prodigy", /* Bonus 1 */
        "Quick Study", /* Bonus 2 */
        "Die Hard", /* Bonus 3 */
        "Brawler", /* Bonus 4 */
        "Destroyer", /* Bonus 5 */
        "Hard Worker", /* Bonus 6 */
        "Healer", /* Bonus 7 */
        "Loyal", /* Bonus 8 */
        "Brawny", /* Bonus 9 */
        "Scholarly", /* Bonus 10 */
        "Sage", /* Bonus 11 */
        "Agile", /* Bonus 12 */
        "Quick", /* Bonus 13 */
        "Sturdy", /* Bonus 14 */
        "Thick Skin", /* Bonus 15 */
        "Recipe Int", /* Bonus 16 */
        "Fireproof", /* Bonus 17 */
        "Powerhitter", /* Bonus 18 */
        "Healthy", /* Bonus 19 */
        "Insomniac", /* Bonus 20 */
        "Evasive", /* Bonus 21 */
        "The Wall", /* Bonus 22 */
        "Accurate", /* Bonus 23 */
        "Energy Leech", /* Bonus 24 */
        "Good Memory", /* Bonus 25*/
        "Soft Touch", /* Neg 26 */
        "Late Sleeper", /* Neg 27 */
        "Impulse Shop", /* Neg 28 */
        "Sickly", /* Neg 29 */
        "Punching Bag", /* Neg 30 */
        "Pushover", /* Neg 31 */
        "Poor Depth Perception", /* Neg 32 */
        "Thin Skin", /* Neg 33 */
        "Fireprone", /* Neg 34 */
        "Energy Intollerant", /* Neg 35 */
        "Coward", /* Neg 36 */
        "Arrogant", /* Neg 37 */
        "Unfocused", /* Neg 38 */
        "Slacker", /* Neg 39 */
        "Slow Learner", /* Neg 40 */
        "Masochistic", /* Neg 41 */
        "Mute", /* Neg 42 */
        "Wimp", /* Neg 43 */
        "Dull", /* Neg 44 */
        "Foolish", /* Neg 45 */
        "Clumsy", /* Neg 46 */
        "Slow", /* Neg 47 */
        "Frail", /* Neg 48 */
        "Sadistic", /* Neg 49 */
        "Loner", /* Neg 50 */
        "Bad Memory" /* Neg 51 */
};

/* List cost of bonus/negative */
const int list_bonus_cost[] = {
        -2, /* Bonus 0 */
        -5, /* Bonus 1 */
        -3, /* Bonus 2 */
        -6, /* Bonus 3 */
        -4, /* Bonus 4 */
        -3, /* Bonus 5 */
        -3, /* Bonus 6 */
        -3, /* Bonus 7 */
        -2, /* Bonus 8 */
        -5, /* Bonus 9 */
        -5, /* Bonus 10 */
        -5, /* Bonus 11 */
        -4, /* Bonus 12 */
        -6, /* Bonus 13 */
        -5, /* Bonus 14 */
        -5, /* Bonus 15 */
        -2, /* Bonus 16 */
        -4, /* Bonus 17 */
        -4, /* Bonus 18 */
        -3, /* Bonus 19 */
        -2, /* Bonus 20 */
        -3, /* Bonus 21 */
        -3, /* Bonus 22 */
        -4, /* Bonus 23 */
        -5, /* Bonus 24 */
        -6, /* Bonus 25*/
        5, /* Negative 26 */
        5, /* Negative 27 */
        3, /* Negative 28 */
        5, /* Negative 29 */
        3, /* Negative 30 */
        3, /* Negative 31 */
        4, /* Negative 32 */
        4, /* Negative 33 */
        5, /* Negative 34 */
        6, /* Negative 35 */
        6, /* Negative 36 */
        1, /* Negative 37 */
        3, /* Negative 38 */
        3, /* Negative 39 */
        3, /* Negative 40 */
        5, /* Negative 41 */
        4, /* Negative 42 */
        6, /* Negative 43 */
        6, /* Negative 44 */
        6, /* Negative 45 */
        3, /* Negative 46 */
        6, /* Negative 47 */
        4, /* Negative 48 */
        3, /* Negative 49 */
        2, /* Negative 50 */
        6, /* Negative 51 */
};

const std::unordered_map<std::string, CharAttribute> CharAttributeNames = {
        {"Strength", CharAttribute::Strength},
        {"Agility", CharAttribute::Agility},
        {"Constitution", CharAttribute::Constitution},
        {"Intelligence", CharAttribute::Intelligence},
        {"Speed", CharAttribute::Speed},
        {"Wisdom", CharAttribute::Wisdom}
};

const std::unordered_map<std::string, CharTrain> CharTrainNames = {
        {"Strength", CharTrain::Strength},
        {"Agility", CharTrain::Agility},
        {"Constitution", CharTrain::Constitution},
        {"Intelligence", CharTrain::Intelligence},
        {"Speed", CharTrain::Speed},
        {"Wisdom", CharTrain::Wisdom}
};

const std::unordered_map<std::string, CharAppearance> CharAppearanceNames = {
        {"Sex", CharAppearance::Sex},
        {"HairLength", CharAppearance::HairLength},
        {"HairStyle", CharAppearance::HairStyle},
        {"HairColor", CharAppearance::HairColor},
        {"SkinColor", CharAppearance::SkinColor},
        {"EyeColor", CharAppearance::EyeColor},
        {"DistinguishingFeature", CharAppearance::DistinguishingFeature},
        {"Aura", CharAppearance::Aura}
};

const std::unordered_map<std::string, CharAlign> CharAlignNames = {
        {"GoodEvil", CharAlign::GoodEvil},
        {"LawChaos", CharAlign::LawChaos}
};

const std::unordered_map<std::string, CharMoney> CharMoneyNames = {
        {"Carried", CharMoney::Carried},
        {"Bank", CharMoney::Bank}
};

const std::unordered_map<std::string, CharStat> CharStatNames = {
        {"PowerLevel", CharStat::PowerLevel},
        {"Ki", CharStat::Ki},
        {"Stamina", CharStat::Stamina}
};