//
// Created by volund on 10/11/22.
//
#pragma once

#include "sysdep.h"

/*
 * Intended use of this macro is to allow external packages to work with
 * a variety of CircleMUD versions without modifications.  For instance,
 * an IS_CORPSE() macro was introduced in pl13.  Any future code add-ons
 * could take into account the CircleMUD version and supply their own
 * definition for the macro if used on an older version of CircleMUD.
 * You are supposed to compare this with the macro CIRCLEMUD_VERSION()
 * in utils.h.  See there for usage.
 */
#define _CIRCLEMUD    0x030100 /* Major/Minor/Patchlevel - MMmmPP */

/*
 * If you want equipment to be automatically equipped to the same place
 * it was when players rented, set the define below to 1.  Please note
 * that this will require erasing or converting all of your rent files.
 * And of course, you have to recompile everything.  We need this feature
 * for CircleMUD to be complete but we refuse to break binary file
 * compatibility.
 */
#define USE_AUTOEQ    1    /* TRUE/FALSE aren't defined yet. */

/* CWG Version String */
#define CWG_VERSION "CWG Rasputin - 3.5.31"
#define DBAT_VERSION "DBAT - 3.0"

/* preamble *************************************************************/

/*
 * As of bpl20, it should be safe to use unsigned data types for the
 * various virtual and real number data types.  There really isn't a
 * reason to use signed anymore so use the unsigned types and get
 * 65,535 objects instead of 32,768.
 *
 * NOTE: This will likely be unconditionally unsigned later.
 */

enum class RaceID : uint8_t {
 Human = 0,
 Saiyan = 1,
 Icer = 2,
 Konatsu = 3,
 Namekian = 4,
 Mutant = 5,
 Kanassan = 6,
 Halfbreed = 7,
 BioAndroid = 8,
 Android = 9,
 Demon = 10,
 Majin = 11,
 Kai = 12,
 Tuffle = 13,
 Hoshijin = 14,
 Animal = 15,
 Saiba = 16,
 Serpent = 17,
 Ogre = 18,
 Yardratian = 19,
 Arlian = 20,
 Dragon = 21,
 Mechanical = 22,
 Spirit = 23
};

enum class SenseiID : uint8_t {
    Roshi = 0,
    Piccolo = 1,
    Krane = 2,
    Nail = 3,
    Bardock = 4,
    Ginyu = 5,
    Frieza = 6,
    Tapion = 7,
    Sixteen = 8,
    Dabura = 9,
    Kibito = 10,
    Jinto = 11,
    Tsuna = 12,
    Kurzak = 13,
    Commoner = 14
};

enum class FormID : uint8_t {
 // Universal,
 Base = 0,
 Custom1 = 1,
 Custom2 = 2,
 Custom3 = 3,
 Custom4 = 4,
 Custom5 = 5,
 Custom6 = 6,
 Custom7 = 7,
 Custom8 = 8,
 Custom9 = 9,

 // Saiyan'y forms.
 Oozaru = 10,
 GoldenOozaru = 11,
 SuperSaiyan = 12,
 SuperSaiyan2 = 13,
 SuperSaiyan3 = 14,
 SuperSaiyan4 = 15,
 SuperSaiyanGod = 16,
 SuperSaiyanBlue = 17,

 // Human'y Forms
 SuperHuman = 18,
 SuperHuman2 = 19,
 SuperHuman3 = 20,
 SuperHuman4 = 21,


 // Icer'y Forms
 IcerFirst = 22,
 IcerSecond = 23,
 IcerThird = 24,
 IcerFourth = 25,
 IcerMetal = 26,
 IcerGolden = 27,
 IcerBlack = 28,

 // Konatsu
 ShadowFirst = 29,
 ShadowSecond = 30,
 ShadowThird = 31,

 // Namekian
 SuperNamekian = 32,
 SuperNamekian2 = 33,
 SuperNamekian3 = 34,
 SuperNamekian4 = 35,

 // Mutant
 MutateFirst = 36,
 MutateSecond = 37,
 MutateThird = 38,

 // BioAndroid
 BioMature = 39,
 BioSemiPerfect = 40,
 BioPerfect = 41,
 BioSuperPerfect = 42,

 // Android
 Android10 = 43,
 Android20 = 44,
 Android30 = 45,
 Android40 = 46,
 Android50 = 47,
 Android60 = 48,

 // Majin
 MajAffinity = 49,
 MajSuper = 50,
 MajTrue = 51,


 // Kai
 MysticFirst = 52,
 MysticSecond = 53,
 MysticThird = 54,

 // Tuffle
 AscendFirst = 55,
 AscendSecond = 56,
 AscendThird = 57
};


#define SG_MIN        2 /* Skill gain check must be less than this
			     number in order to be successful.
			     IE: 1% of a skill gain */

/* Ocarina Songs */
#define SONG_SAFETY             1
#define SONG_SHIELDING          2
#define SONG_SHADOW_STITCH      3
#define SONG_TELEPORT_EARTH     4
#define SONG_TELEPORT_KONACK    5
#define SONG_TELEPORT_ARLIA     6
#define SONG_TELEPORT_NAMEK     7
#define SONG_TELEPORT_VEGETA    8
#define SONG_TELEPORT_FRIGID    9
#define SONG_TELEPORT_AETHER   10
#define SONG_TELEPORT_KANASSA  11

/* Fighting Preferences */
#define PREFERENCE_THROWING     1
#define PREFERENCE_H2H          2
#define PREFERENCE_KI           3
#define PREFERENCE_WEAPON       4

/* Ingredient vnums for recipes */
#define RCP_TOMATO      17212
#define RCP_POTATO      17213
#define RCP_ONION       17215
#define RCP_CUCUMBER    17217
#define RCP_CHILIPEPPER 17219
#define RCP_FOUSTAFI    17221
#define RCP_CARROT      17223
#define RCP_GREENBEAN   17225
#define RCP_NORMAL_MEAT 1612
#define RCP_GOOD_MEAT   6500
#define RCP_BLACKBASS   1000
#define RCP_SILVERTROUT 1001
#define RCP_STRIPEDBASS 1002
#define RCP_BLUECATFISH 1003
#define RCP_FLOUNDER    1004
#define RCP_SILVEREEL   1005
#define RCP_COBIA       1006
#define RCP_TAMBOR      1007
#define RCP_NARRI       1008
#define RCP_VALBISH     1009
#define RCP_GUSBLAT     1010
#define RCP_REPEEIL     1011
#define RCP_GRAVELREBOI 1012
#define RCP_VOOSPIKE    1013
#define RCP_SHADOWFISH  1014
#define RCP_SHADEEEL    1015
#define RCP_BROWNMUSH   1608
#define RCP_GARLIC      1131
#define RCP_RICE        1590
#define RCP_FLOUR       1591
#define RCP_LETTUCE     17227
#define RCP_APPLEPLUM   8001
#define RCP_FROZENBERRY 4901
#define RCP_CARAMBOLA   3416

/* Meal recipes */
#define RECIPE_TOMATO_SOUP       1
#define RECIPE_POTATO_SOUP       2
#define RECIPE_VEGETABLE_SOUP    3
#define RECIPE_MEAT_STEW         4
#define RECIPE_STEAK             5
#define RECIPE_ROAST             6
#define RECIPE_CHILI_SOUP        7
#define RECIPE_GRILLED_NORMFISH  8
#define RECIPE_GRILLED_GOODFISH  9
#define RECIPE_GRILLED_GREATFISH 10
#define RECIPE_GRILLED_BESTFISH  11
#define RECIPE_COOKED_RICE       12
#define RECIPE_SUSHI             13
#define RECIPE_BREAD             14
#define RECIPE_SALAD             15
#define RECIPE_APPLEPLUM         16
#define RECIPE_FBERRY_MUFFIN     17
#define RECIPE_CARAMBOLA_BREAD   18

/* Completed Meal Object Vnums */
#define MEAL_START               1220
#define MEAL_TOMATO_SOUP         1220
#define MEAL_STEAK               1221
#define MEAL_POTATO_SOUP         1222
#define MEAL_VEGETABLE_SOUP      1223
#define MEAL_MEAT_STEW           1224
#define MEAL_ROAST               1225
#define MEAL_CHILI_SOUP          1226
#define MEAL_NORM_FISH           1227
#define MEAL_GOOD_FISH           1228
#define MEAL_GREAT_FISH          1229
#define MEAL_BEST_FISH           1230
#define MEAL_COOKED_RICE         1231
#define MEAL_SUSHI               1232
#define MEAL_BREAD               1233
#define MEAL_SALAD               1234
#define MEAL_APPLEPLUM           1235
#define MEAL_FBERRY_MUFFIN       1236
#define MEAL_CARAMBOLA_BREAD     1237


#define MEAL_LAST                1234

/* Fishing Defines */
#define FISH_NOFISH             0
#define FISH_BITE               1
#define FISH_HOOKED             2
#define FISH_REELING            3


/* Shadow Dragon Defines */

#define SHADOW_DRAGON1_VNUM 81
#define SHADOW_DRAGON2_VNUM 82
#define SHADOW_DRAGON3_VNUM 83
#define SHADOW_DRAGON4_VNUM 84
#define SHADOW_DRAGON5_VNUM 85
#define SHADOW_DRAGON6_VNUM 86
#define SHADOW_DRAGON7_VNUM 87

/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHWEST      6
#define NORTHEAST      7
#define SOUTHEAST      8
#define SOUTHWEST      9
#define INDIR         10
#define OUTDIR        11

/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK        0   /* Dark			*/
#define ROOM_DEATH        1   /* Death trap		*/
#define ROOM_NOMOB        2   /* MOBs not allowed		*/
#define ROOM_INDOORS    3   /* Indoors			*/
#define ROOM_PEACEFUL    4   /* Violence not allowed	*/
#define ROOM_SOUNDPROOF    5   /* Shouts, gossip blocked	*/
#define ROOM_NOTRACK    6   /* Track won't go through	*/
#define ROOM_NOINSTANT    7   /* IT not allowed		*/
#define ROOM_TUNNEL        8   /* room for only 1 pers	*/
#define ROOM_PRIVATE    9   /* Can't teleport in		*/
#define ROOM_GODROOM    10  /* LVL_GOD+ only allowed	*/
#define ROOM_HOUSE        11  /* (R) Room is a house	*/
#define ROOM_HOUSE_CRASH 12  /* (R) House needs saving	*/
#define ROOM_ATRIUM        13  /* (R) The door to a house	*/
#define ROOM_OLC        14  /* (R) Modifyable/!compress	*/
#define ROOM_BFS_MARK    15  /* (R) breath-first srch mrk	*/
#define ROOM_VEHICLE    16  /* Requires a vehicle to pass       */
#define ROOM_UNDERGROUND        17  /* Room is below ground      */
#define ROOM_CURRENT        18  /* Room move with random currents	*/
#define ROOM_TIMED_DT        19  /* Room has a timed death trap  	*/
#define ROOM_EARTH              20  /* Room is on Earth */
#define ROOM_VEGETA             21  /* Room is on Vegeta */
#define ROOM_FRIGID             22  /* Room is on Frigid */
#define ROOM_KONACK             23  /* Room is on Konack */
#define ROOM_NAMEK              24  /* Room is on Namek */
#define ROOM_NEO                25  /* Room is on Neo */
#define ROOM_AL                 26  /* Room is on AL */
#define ROOM_SPACE              27  /* Room is on Space */
#define ROOM_HELL               28  /* Room is Punishment Hell*/
#define ROOM_REGEN              29  /* Better regen */
#define ROOM_RHELL              30  /* Room is HELLLLLLL */
#define ROOM_GRAVITYX10         31  /* For rooms that have 10x grav */
#define ROOM_AETHER        32  /* Room is on Aether */
#define ROOM_HBTC               33  /* Room is extra special training area */
#define ROOM_PAST               34  /* Inside the pendulum room */
#define ROOM_CBANK              35  /* This room is a clan bank */
#define ROOM_SHIP               36  /* This room is a private ship room */
#define ROOM_YARDRAT            37  /* This room is on planet Yardrat   */
#define ROOM_KANASSA            38  /* This room is on planet Kanassa   */
#define ROOM_ARLIA              39  /* This room is on planet Arlia     */
#define ROOM_AURA               40  /* This room has an aura around it  */
#define ROOM_EORBIT             41  /* Earth Orbit                      */
#define ROOM_FORBIT             42  /* Frigid Orbit                     */
#define ROOM_KORBIT             43  /* Konack Orbit                     */
#define ROOM_NORBIT             44  /* Namek  Orbit                     */
#define ROOM_VORBIT             45  /* Vegeta Orbit                     */
#define ROOM_AORBIT             46  /* Aether Orbit                     */
#define ROOM_YORBIT             47  /* Yardrat Orbit                    */
#define ROOM_KANORB             48  /* Kanassa Orbit                    */
#define ROOM_ARLORB             49  /* Arlia Orbit                      */
#define ROOM_NEBULA             50  /* Nebulae                          */
#define ROOM_ASTERO             51  /* Asteroid                         */
#define ROOM_WORMHO             52  /* Wormhole                         */
#define ROOM_STATION            53  /* Space Station                    */
#define ROOM_STAR               54  /* Is a star                        */
#define ROOM_CERRIA             55  /* This room is on planet Cerria    */
#define ROOM_CORBIT             56  /* This room is in Cerria's Orbit   */
#define ROOM_BEDROOM            57  /* +25% regen                       */
#define ROOM_WORKOUT            58  /* Workout Room                     */
#define ROOM_GARDEN1            59  /* 8 plant garden                   */
#define ROOM_GARDEN2            60  /* 20 plant garden                  */
#define ROOM_FERTILE1           61
#define ROOM_FERTILE2           62
#define ROOM_FISHING            63
#define ROOM_FISHFRESH          64
#define ROOM_CANREMODEL         65
#define ROOM_LANDING            66
#define ROOM_SAVE               67 /* room saves contents */

#define NUM_ROOM_FLAGS          68

#define AREA_MOON              0 /* FOR CelestialBody: Planet has Moon for Oozaru. */
#define AREA_ETHER             1 /* FOR CelestialBody: Planet has Ether Stream for Hoshijin. */
#define AREA_EARTH_DBALL       2 /* (Not Inherited): Earth DragonBalls may spawn here. */
#define AREA_NAMEK_DBALL       3 /* (Not Inherited): Namek DragonBalls may spawn here. */
#define AREA_SHADOW_DRAGON     4 */ (Not Inherited): The Shadow Dragons of Black Smoke Shenron may spawn here. */

#define NUM_AREA_FLAGS        5

/* Zone info: Used in zone_data.zone_flags */
#define ZONE_CLOSED        0
#define ZONE_NOIMMORT        1
#define ZONE_QUEST        2
#define ZONE_DBALLS        3


/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR        (1 << 0)   /* Exit is a door		*/
#define EX_CLOSED        (1 << 1)   /* The door is closed	*/
#define EX_LOCKED        (1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF        (1 << 3)   /* Lock can't be picked	*/
#define EX_SECRET        (1 << 4)   /* The door is hidden        */

#define NUM_EXIT_FLAGS 5

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0           /* Indoors			*/
#define SECT_CITY            1           /* In a city			*/
#define SECT_FIELD           2           /* In a field		*/
#define SECT_FOREST          3           /* In a forest		*/
#define SECT_HILLS           4           /* In the hills		*/
#define SECT_MOUNTAIN        5           /* On a mountain		*/
#define SECT_WATER_SWIM      6           /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7           /* Water - need a boat	*/
#define SECT_FLYING         8           /* Wheee!			*/
#define SECT_UNDERWATER         9           /* Underwater		*/
#define SECT_SHOP            10            /* Shop                      */
#define SECT_IMPORTANT       11            /* Important Rooms           */
#define SECT_DESERT          12            /* A desert                  */
#define SECT_SPACE           13            /* This is a space room      */
#define SECT_LAVA            14            /* This room always has lava */

#define NUM_ROOM_SECTORS     15


/* char and mob-related defines *****************************************/

/* PC classes */
/* Taken from the SRD under OGL, see ../doc/srd.txt for information */
#define CLASS_ROSHI             SenseiID::Roshi
#define CLASS_PICCOLO           SenseiID::Piccolo
#define CLASS_KRANE             SenseiID::Krane
#define CLASS_NAIL              SenseiID::Nail
#define CLASS_BARDOCK           SenseiID::Bardock
#define CLASS_GINYU             SenseiID::Ginyu
#define CLASS_FRIEZA            SenseiID::Frieza
#define CLASS_TAPION            SenseiID::Tapion
#define CLASS_ANDSIX            SenseiID::Sixteen
#define CLASS_DABURA            SenseiID::Dabura
#define CLASS_KABITO            SenseiID::Kibito
#define CLASS_JINTO             SenseiID::Jinto
#define CLASS_TSUNA             SenseiID::Tsuna
#define CLASS_KURZAK            SenseiID::Kurzak
#define CLASS_ASSASSIN          14
#define CLASS_BLACKGUARD        15
#define CLASS_DRAGON_DISCIPLE   16
#define CLASS_DUELIST           17
#define CLASS_DWARVEN_DEFENDER  18
#define CLASS_ELDRITCH_KNIGHT   19
#define CLASS_HIEROPHANT        20
#define CLASS_HORIZON_WALKER    21
#define CLASS_LOREMASTER        22
#define CLASS_MYSTIC_THEURGE    23
#define CLASS_SHADOWDANCER      24
#define CLASS_THAUMATURGIST     25
#define CLASS_NPC_EXPERT    26
#define CLASS_NPC_ADEPT        27
#define CLASS_NPC_COMMONER    28
#define CLASS_NPC_ARISTOCRAT    29
#define CLASS_NPC_WARRIOR    30

#define MAX_SENSEI              15 /* Used by Sensei Style */


#define NUM_CLASSES             31
#define NUM_NPC_CLASSES    4
#define NUM_PRESTIGE_CLASSES    15
#define NUM_BASIC_CLASSES    (14)


/* Gauntlet crap */
#define GAUNTLET_ZONE  24    /* The gauntlet zone vnum */
#define GAUNTLET_START 2403  /* The waiting room at the start of the gauntlet */
#define GAUNTLET_END   2404  /* The treasure room at the end of the gauntlet  */

/* Death Types for producing corpses with depth */
#define DTYPE_NORMAL      0    /* Default Death Type */
#define DTYPE_HEAD        1    /* Lost their head    */
#define DTYPE_HALF        2    /* Blown in half      */
#define DTYPE_VAPOR       3    /* Vaporized by attack*/
#define DTYPE_PULP        4    /* Beat to a pulp     */

/* Character Creation Styles */
/* Let's define bonuses/negatives */
#define BONUS_THRIFTY           0
#define BONUS_PRODIGY           1
#define BONUS_QUICK_STUDY       2
#define BONUS_DIEHARD           3
#define BONUS_BRAWLER           4
#define BONUS_DESTROYER         5
#define BONUS_HARDWORKER        6
#define BONUS_HEALER            7
#define BONUS_LOYAL             8
#define BONUS_BRAWNY            9
#define BONUS_SCHOLARLY        10
#define BONUS_SAGE             11
#define BONUS_AGILE            12
#define BONUS_QUICK            13
#define BONUS_STURDY           14
#define BONUS_THICKSKIN        15
#define BONUS_RECIPE           16
#define BONUS_FIREPROOF        17
#define BONUS_POWERHIT         18
#define BONUS_HEALTHY          19
#define BONUS_INSOMNIAC        20
#define BONUS_EVASIVE          21
#define BONUS_WALL             22
#define BONUS_ACCURATE         23
#define BONUS_LEECH            24
#define BONUS_GMEMORY          25
#define BONUS_SOFT             26
#define BONUS_LATE             27
#define BONUS_IMPULSE          28
#define BONUS_SICKLY           29
#define BONUS_PUNCHINGBAG      30
#define BONUS_PUSHOVER         31
#define BONUS_POORDEPTH        32
#define BONUS_THINSKIN         33
#define BONUS_FIREPRONE        34
#define BONUS_INTOLERANT       35
#define BONUS_COWARD           36
#define BONUS_ARROGANT         37
#define BONUS_UNFOCUSED        38
#define BONUS_SLACKER          39
#define BONUS_SLOW_LEARNER     40
#define BONUS_MASOCHISTIC      41
#define BONUS_MUTE             42
#define BONUS_WIMP             43
#define BONUS_DULL             44
#define BONUS_FOOLISH          45
#define BONUS_CLUMSY           46
#define BONUS_SLOW             47
#define BONUS_FRAIL            48
#define BONUS_SADISTIC         49
#define BONUS_LONER            50
#define BONUS_BMEMORY          51

#define MAX_BONUSES            52

/* Distinguishing Feature */
#define DISTFEA_EYE             0
#define DISTFEA_HAIR            1
#define DISTFEA_SKIN            2
#define DISTFEA_HEIGHT          3
#define DISTFEA_WEIGHT          4

/* Custom Aura */
#define AURA_WHITE              0
#define AURA_BLUE               1
#define AURA_RED                2
#define AURA_GREEN              3
#define AURA_PINK               4
#define AURA_PURPLE             5
#define AURA_YELLOW             6
#define AURA_BLACK              7
#define AURA_ORANGE             8

/* Eye Color */
#define EYE_UNDEFINED           (-1)
#define EYE_BLUE                0
#define EYE_BLACK               1
#define EYE_GREEN               2
#define EYE_BROWN               3
#define EYE_RED                 4
#define EYE_AQUA                5
#define EYE_PINK                6
#define EYE_PURPLE              7
#define EYE_CRIMSON             8
#define EYE_GOLD                9
#define EYE_AMBER               10
#define EYE_EMERALD             11

/*Hair Length */
#define HAIRL_UNDEFINED         (-1)
#define HAIRL_BALD              0
#define HAIRL_SHORT             1
#define HAIRL_MEDIUM            2
#define HAIRL_LONG              3
#define HAIRL_RLONG             4


/*Hair Color */
#define HAIRC_UNDEFINED         (-1)
#define HAIRC_NONE              0
#define HAIRC_BLACK             1
#define HAIRC_BROWN             2
#define HAIRC_BLONDE            3
#define HAIRC_GREY              4
#define HAIRC_RED               5
#define HAIRC_ORANGE            6
#define HAIRC_GREEN             7
#define HAIRC_BLUE              8
#define HAIRC_PINK              9
#define HAIRC_PURPLE            10
#define HAIRC_SILVER            11
#define HAIRC_CRIMSON           12
#define HAIRC_WHITE             13

/* Hair Style */
#define HAIRS_UNDEFINED         (-1)
#define HAIRS_NONE              0
#define HAIRS_PLAIN             1
#define HAIRS_MOHAWK            2
#define HAIRS_SPIKY             3
#define HAIRS_CURLY             4
#define HAIRS_UNEVEN            5
#define HAIRS_PONYTAIL          6
#define HAIRS_AFRO              7
#define HAIRS_FADE              8
#define HAIRS_CREW              9
#define HAIRS_FEATHERED         10
#define HAIRS_DRED              11


/* Skin Color */
#define SKIN_UNDEFINED          (-1)
#define SKIN_WHITE              0
#define SKIN_BLACK              1
#define SKIN_GREEN              2
#define SKIN_ORANGE             3
#define SKIN_YELLOW             4
#define SKIN_RED                5
#define SKIN_GREY               6
#define SKIN_BLUE               7
#define SKIN_AQUA               8
#define SKIN_PINK               9
#define SKIN_PURPLE             10
#define SKIN_TAN                11

/* Annual Sign Phase */

#define PHASE_PURITY            0
#define PHASE_BRAVERY           1
#define PHASE_HATRED            2
#define PHASE_DOMINANCE         3
#define PHASE_GUARDIAN          4
#define PHASE_LOVE              5
#define PHASE_STRENGTH          6

/* Races */
#define RACE_HUMAN        RaceID::Human
#define RACE_SAIYAN        RaceID::Saiyan
#define RACE_ICER        RaceID::Icer
#define RACE_KONATSU        RaceID::Konatsu
#define RACE_NAMEK        RaceID::Namekian
#define RACE_MUTANT        RaceID::Mutant
#define RACE_KANASSAN        RaceID::Kanassan
#define RACE_HALFBREED        RaceID::Halfbreed
#define RACE_BIO        RaceID::BioAndroid
#define RACE_ANDROID        RaceID::Android
#define RACE_DEMON        RaceID::Demon
#define RACE_MAJIN        RaceID::Majin
#define RACE_KAI        RaceID::Kai
#define RACE_TRUFFLE        RaceID::Tuffle
#define RACE_GOBLIN        RaceID::Hoshijin
#define RACE_ANIMAL        RaceID::Animal
#define RACE_SAIBA        RaceID::Saiba
#define RACE_SERPENT        RaceID::Serpent
#define RACE_OGRE        RaceID::Ogre
#define RACE_YARDRATIAN        RaceID::Yardratian
#define RACE_ARLIAN        RaceID::Arlian
#define RACE_DRAGON        RaceID::Dragon
#define RACE_MECHANICAL    RaceID::Mechanical
#define RACE_FAERIE        RaceID::Spirit

#define NUM_RACES        24

#define ALIGN_SAINT              0
#define ALIGN_VALIANT            1
#define ALIGN_HERO               2
#define ALIGN_DOGOOD             3
#define ALIGN_NEUTRAL            4
#define ALIGN_CROOK              5
#define ALIGN_VILLAIN            6
#define ALIGN_TERRIBLE           7
#define ALIGN_HORRIBLE           8

#define NUM_ALIGNS               9

/* Taken from the SRD under OGL, see ../doc/srd.txt for information */
#define SIZE_UNDEFINED    (-1)
#define SIZE_FINE    0
#define SIZE_DIMINUTIVE    1
#define SIZE_TINY    2
#define SIZE_SMALL    3
#define SIZE_MEDIUM    4
#define SIZE_LARGE    5
#define SIZE_HUGE    6
#define SIZE_GARGANTUAN    7
#define SIZE_COLOSSAL    8

#define NUM_SIZES         9

#define WIELD_NONE        0
#define WIELD_LIGHT       1
#define WIELD_ONEHAND     2
#define WIELD_TWOHAND     3

/* Number of weapon types */
#define MAX_WEAPON_TYPES            26

/* Critical hit types */
#define CRIT_X2        0
#define CRIT_X3        1
#define CRIT_X4        2

#define MAX_CRIT_TYPE    CRIT_X4
#define NUM_CRIT_TYPES 3

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

#define NUM_SEX       3

/* Positions */
#define POS_DEAD       0    /* dead			*/
#define POS_MORTALLYW  1    /* mortally wounded	*/
#define POS_INCAP      2    /* incapacitated	*/
#define POS_STUNNED    3    /* stunned		*/
#define POS_SLEEPING   4    /* sleeping		*/
#define POS_RESTING    5    /* resting		*/
#define POS_SITTING    6    /* sitting		*/
#define POS_FIGHTING   7    /* fighting		*/
#define POS_STANDING   8    /* standing		*/

#define NUM_POSITIONS  9

/* AUCTIONING STATES */
#define AUC_NULL_STATE        0   /* not doing anything */
#define AUC_OFFERING        1   /* object has been offfered */
#define AUC_GOING_ONCE        2    /* object is going once! */
#define AUC_GOING_TWICE        3    /* object is going twice! */
#define AUC_LAST_CALL        4    /* last call for the object! */
#define AUC_SOLD        5
/* AUCTION CANCEL STATES */
#define AUC_NORMAL_CANCEL    6    /* normal cancellation of auction */
#define AUC_QUIT_CANCEL        7    /* auction canclled because player quit */
#define AUC_WIZ_CANCEL        8    /* auction cancelled by a god */
/* OTHER JUNK */
#define AUC_STAT        9
#define AUC_BID            10

/* Player flags: used by char_data.act */
#define PLR_KILLER    0   /* Player is a player-killer        */
#define PLR_THIEF    1   /* Player is a player-thief         */
#define PLR_FROZEN    2   /* Player is frozen                 */
#define PLR_DONTSET    3   /* Don't EVER set (ISNPC bit) 	*/
#define PLR_WRITING    4   /* Player writing (board/mail/olc)  */
#define PLR_MAILING    5   /* Player is writing mail           */
#define PLR_CRASH    6   /* Player needs to be crash-saved   */
#define PLR_SITEOK    7   /* Player has been site-cleared     */
#define PLR_NOSHOUT    8   /* Player not allowed to shout/goss */
#define PLR_NOTITLE    9   /* Player not allowed to set title  */
#define PLR_DELETED    10  /* Player deleted - space reusable  */
#define PLR_LOADROOM    11  /* Player uses nonstandard loadroom */
#define PLR_NOWIZLIST    12  /* Player shouldn't be on wizlist  	*/
#define PLR_NODELETE    13  /* Player shouldn't be deleted     	*/
#define PLR_INVSTART    14  /* Player should enter game wizinvis*/
#define PLR_CRYO    15  /* Player is cryo-saved (purge prog)*/
#define PLR_NOTDEADYET    16  /* (R) Player being extracted.     	*/
#define PLR_AGEMID_G    17  /* Player has had pos of middle age	*/
#define PLR_AGEMID_B    18  /* Player has had neg of middle age	*/
#define PLR_AGEOLD_G    19  /* Player has had pos of old age	*/
#define PLR_AGEOLD_B    20  /* Player has had neg of old age	*/
#define PLR_AGEVEN_G    21  /* Player has had pos of venerable age	*/
#define PLR_AGEVEN_B    22  /* Player has had neg of venerable age	*/
#define PLR_OLDAGE    23  /* Player is dead of old age	*/
#define PLR_RARM        24  /* Player has a right arm           */
#define PLR_LARM        25  /* Player has a left arm            */
#define PLR_RLEG        26  /* Player has a right leg           */
#define PLR_LLEG        27  /* Player has a left leg            */
#define PLR_HEAD        28  /* Player has a head                */
#define PLR_STAIL       29  /* UNUSED         */
#define PLR_TAIL        30  /* Player has a tail     */
#define PLR_PILOTING    31  /* Player is sitting in the pilots chair */
#define PLR_SKILLP      32  /* Player made a good choice in CC  */
#define PLR_SPAR        33  /* Player is in a spar stance       */
#define PLR_CHARGE      34  /* Player is charging               */
#define PLR_TRANS1      35  /* UNUSED                 */
#define PLR_TRANS2      36  /* UNUSED                 */
#define PLR_TRANS3      37  /* UNUSED                 */
#define PLR_TRANS4      38  /* UNUSED                 */
#define PLR_TRANS5      39  /* UNUSED                 */
#define PLR_TRANS6      40  /* UNUSED                 */
#define PLR_ABSORB      41  /* Absorb model                     */
#define PLR_REPAIR      42  /* Repair model                     */
#define PLR_SENSEM      43  /* Sense-Powersense model           */
#define PLR_POWERUP     44  /* Powering Up                      */
#define PLR_KNOCKED     45  /* Knocked OUT                      */
#define PLR_CRARM       46  /* Cybernetic Right Arm             */
#define PLR_CLARM       47  /* Cybernetic Left Arm              */
#define PLR_CRLEG       48  /* Cybernetic Right Leg             */
#define PLR_CLLEG       49  /* Cybernetic Left Leg              */
#define PLR_FPSSJ       50  /* Full Power Super Saiyan          */
#define PLR_IMMORTAL    51  /* The player is immortal           */
#define PLR_EYEC        52  /* The player has their eyes closed */
#define PLR_DISGUISED   53  /* The player is disguised          */
#define PLR_BANDAGED    54  /* THe player has been bandaged     */
#define PLR_PR          55  /* Has had their potential released */
#define PLR_HEALT       56  /* Is inside a healing tank         */
#define PLR_FURY        57  /* Is in fury mode                  */
#define PLR_POSE        58  /* Ginyu Pose Effect                */
#define PLR_OOZARU      59 // UNUSED
#define PLR_ABSORBED    60
#define PLR_MULTP       61
#define PLR_PDEATH      62
#define PLR_THANDW      63
#define PLR_SELFD       64
#define PLR_SELFD2      65
#define PLR_SPIRAL      66
#define PLR_BIOGR       67
#define PLR_LSSJ        68
#define PLR_REPLEARN    69
#define PLR_FORGET      70
#define PLR_TRANSMISSION 71
#define PLR_FISHING     72
#define PLR_GOOP        73
#define PLR_MULTIHIT    74
#define PLR_AURALIGHT   75
#define PLR_RDISPLAY    76
#define PLR_STOLEN      77
#define PLR_TAILHIDE    78  /* Hides tail for S & HB            */
#define PLR_NOGROW      79  /* Halt Growth for S & HB           */

#define NUM_PLR_FLAGS 80

/* Mob Personalty */
#define MAX_PERSONALITIES 5

/* Mobile flags: used by char_data.act */
#define MOB_SPEC        0  /* Mob has a callable spec-proc   	*/
#define MOB_SENTINEL        1  /* Mob should not move            	*/
#define MOB_NOSCAVENGER        2  /* Mob won't pick up items from rooms*/
#define MOB_ISNPC        3  /* (R) Automatically set on all Mobs */
#define MOB_AWARE        4  /* Mob can't be backstabbed          */
#define MOB_AGGRESSIVE        5  /* Mob auto-attacks everybody nearby	*/
#define MOB_STAY_ZONE        6  /* Mob shouldn't wander out of zone  */
#define MOB_WIMPY        7  /* Mob flees if severely injured  	*/
#define MOB_AGGR_EVIL        8  /* Auto-attack any evil PC's		*/
#define MOB_AGGR_GOOD        9  /* Auto-attack any good PC's      	*/
#define MOB_AGGR_NEUTRAL    10 /* Auto-attack any neutral PC's   	*/
#define MOB_MEMORY        11 /* remember attackers if attacked    */
#define MOB_HELPER        12 /* attack PCs fighting other NPCs    */
#define MOB_NOCHARM        13 /* Mob can't be charmed         	*/
#define MOB_NOSUMMON        14 /* Mob can't be summoned             */
#define MOB_NOSLEEP        15 /* Mob can't be slept           	*/
#define MOB_AUTOBALANCE        16 /* Mob stats autobalance		*/
#define MOB_NOBLIND        17 /* Mob can't be blinded         	*/
#define MOB_NOKILL        18 /* Mob can't be killed               */
#define MOB_NOTDEADYET        19 /* (R) Mob being extracted.          */
#define MOB_MOUNTABLE        20 /* Mob is mountable.			*/
#define MOB_RARM                21 /* Player has a right arm            */
#define MOB_LARM                22 /* Player has a left arm             */
#define MOB_RLEG                23 /* Player has a right leg            */
#define MOB_LLEG                24 /* Player has a left leg             */
#define MOB_HEAD                25 /* Player has a head                 */
#define MOB_JUSTDESC            26 /* Mob doesn't use auto desc         */
#define MOB_HUSK                27 /* Is an extracted Husk              */
#define MOB_SPAR                28 /* This is mob sparring              */
#define MOB_DUMMY               29 /* This mob will not fight back      */
#define MOB_ABSORB              30 /* Absorb type android               */
#define MOB_REPAIR              31 /* Repair type android               */
#define MOB_NOPOISON            32 /* No poison                         */
#define MOB_KNOWKAIO            33 /* Knows kaioken                     */
#define MOB_POWERUP             34 /* Is powering up                    */

#define NUM_MOB_FLAGS 35

/*  flags: used by char_data.player_specials.pref */
#define PRF_BRIEF    0  /* Room descs won't normally be shown	*/
#define PRF_COMPACT    1  /* No extra CRLF pair before prompts		*/
#define PRF_DEAF    2  /* Can't hear shouts              		*/
#define PRF_NOTELL    3  /* Can't receive tells		    	*/
#define PRF_DISPHP    4  /* Display hit points in prompt  		*/
#define PRF_DISPMANA    5  /* Display mana points in prompt    		*/
#define PRF_DISPMOVE    6  /* Display move points in prompt 		*/
#define PRF_AUTOEXIT    7  /* Display exits in a room          		*/
#define PRF_NOHASSLE    8  /* Aggr mobs won't attack           		*/
#define PRF_QUEST    9  /* On quest					*/
#define PRF_SUMMONABLE    10 /* Can be summoned				*/
#define PRF_NOREPEAT    11 /* No repetition of comm commands		*/
#define PRF_HOLYLIGHT    12 /* Can see in dark				*/
#define PRF_COLOR    13 /* Color					*/
#define PRF_SPARE    14 /* Used to be second color bit		*/
#define PRF_NOWIZ    15 /* Can't hear wizline			*/
#define PRF_LOG1    16 /* On-line System Log (low bit)		*/
#define PRF_LOG2    17 /* On-line System Log (high bit)		*/
#define PRF_NOAUCT    18 /* Can't hear auction channel		*/
#define PRF_NOGOSS    19 /* Can't hear gossip channel			*/
#define PRF_NOGRATZ    20 /* Can't hear grats channel			*/
#define PRF_ROOMFLAGS    21 /* Can see room flags (ROOM_x)		*/
#define PRF_DISPAUTO    22 /* Show prompt HP, MP, MV when < 30%.	*/
#define PRF_CLS         23 /* Clear screen in OasisOLC 			*/
#define PRF_BUILDWALK   24 /* Build new rooms when walking		*/
#define PRF_AFK         25 /* Player is AFK				*/
#define PRF_AUTOLOOT    26 /* Loot everything from a corpse		*/
#define PRF_AUTOGOLD    27 /* Loot gold from a corpse			*/
#define PRF_AUTOSPLIT   28 /* Split gold with group			*/
#define PRF_FULL_EXIT   29 /* Shows full autoexit details		*/
#define PRF_AUTOSAC     30 /* Sacrifice a corpse 			*/
#define PRF_AUTOMEM     31 /* Memorize spells				*/
#define PRF_VIEWORDER   32 /* if you want to see the newest first 	*/
#define PRF_NOCOMPRESS  33 /* If you want to force MCCP2 off          	*/
#define PRF_AUTOASSIST  34 /* Auto-assist toggle                      	*/
#define PRF_DISPKI    35 /* Display ki points in prompt 		*/
#define PRF_DISPEXP    36 /* Display exp points in prompt 		*/
#define PRF_DISPTNL    37 /* Display TNL exp points in prompt 		*/
#define PRF_TEST        38 /* Sets triggers safety off for imms         */
#define PRF_HIDE        39 /* Hide on who from other mortals            */
#define PRF_NMWARN      40 /* No mail warning                           */
#define PRF_HINTS       41 /* Receives hints                            */
#define PRF_FURY        42 /* Sees fury meter                           */
#define PRF_NODEC       43
#define PRF_NOEQSEE     44
#define PRF_NOMUSIC     45
#define PRF_LKEEP       46
#define PRF_DISTIME     47 /* Part of Prompt Options */
#define PRF_DISGOLD     48 /* Part of Prompt Options */
#define PRF_DISPRAC     49 /* Part of Prompt Options */
#define PRF_NOPARRY     50
#define PRF_DISHUTH     51 /* Part of Prompt Options */
#define PRF_DISPERC     52 /* Part of Prompt Options */
#define PRF_CARVE       53
#define PRF_ARENAWATCH  54
#define PRF_NOGIVE      55
#define PRF_INSTRUCT    56
#define PRF_GHEALTH     57
#define PRF_IHEALTH     58
#define PRF_ENERGIZE    59

#define NUM_PRF_FLAGS   60

/* Player autoexit levels: used as an index to exitlevels           */
#define EXIT_OFF        0       /* Autoexit off                     */
#define EXIT_NORMAL     1       /* Brief display (stock behaviour)  */
#define EXIT_NA         2       /* Not implemented - do not use     */
#define EXIT_COMPLETE   3       /* Full display                     */

#define exitlevel(ch) (!IS_NPC(ch) ? (PRF_FLAGGED((ch),PRF_AUTOEXIT) ? 1 : 0 ) + (PRF_FLAGGED((ch),PRF_FULL_EXIT) ? 2 : 0 ) : 0 )
#define EXIT_LEV(ch) (exitlevel(ch))


/* Affect bits: used in char_data.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_DONTUSE           0    /* DON'T USE! 		*/
#define AFF_BLIND             1    /* (R) Char is blind         */
#define AFF_INVISIBLE         2    /* Char is invisible         */
#define AFF_DETECT_ALIGN      3    /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      4    /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      5    /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        6    /* Char can sense hidden life*/
#define AFF_WATERWALK         7    /* Char can walk on water    */
#define AFF_SANCTUARY         8    /* Char protected by sanct.  */
#define AFF_GROUP             9    /* (R) Char is grouped       */
#define AFF_CURSE             10   /* Char is cursed            */
#define AFF_INFRAVISION       11   /* Char can see in dark      */
#define AFF_POISON            12   /* (R) Char is poisoned      */
#define AFF_WEAKENED_STATE    13   /* Char protected from evil  */
#define AFF_PROTECT_GOOD      14   /* Char protected from good  */
#define AFF_SLEEP             15   /* (R) Char magically asleep */
#define AFF_NOTRACK           16   /* Char can't be tracked     */
#define AFF_UNDEAD            17   /* Char is undead 		*/
#define AFF_PARALYZE          18   /* Char is paralized		*/
#define AFF_SNEAK             19   /* Char can move quietly     */
#define AFF_HIDE              20   /* Char is hidden            */
#define AFF_UNUSED20          21   /* Room for future expansion */
#define AFF_CHARM             22   /* Char is charmed         	*/
#define AFF_FLYING            23   /* Char is flying         	*/
#define AFF_WATERBREATH       24   /* Char can breath non O2    */
#define AFF_ANGELIC           25   /* Char is an angelic being  */
#define AFF_ETHEREAL          26   /* Char is ethereal          */
#define AFF_MAGICONLY         27   /* Char only hurt by magic   */
#define AFF_NEXTPARTIAL       28   /* Next action cannot be full*/
#define AFF_NEXTNOACTION      29   /* Next action cannot attack (took full action between rounds) */
#define AFF_STUNNED           30   /* Char is stunned		*/
#define AFF_TAMED             31   /* Char has been tamed	*/
#define AFF_CDEATH            32   /* Char is undergoing creeping death */
#define AFF_SPIRIT            33   /* Char has no body          */
#define AFF_STONESKIN         34   /* Char has temporary DR     */
#define AFF_SUMMONED          35   /* Char is summoned (i.e. transient */
#define AFF_CELESTIAL         36   /* Char is celestial         */
#define AFF_FIENDISH          37   /* Char is fiendish          */
#define AFF_FIRE_SHIELD       38   /* Char has fire shield      */
#define AFF_LOW_LIGHT         39   /* Char has low light eyes   */
#define AFF_ZANZOKEN          40   /* Char is ready to zanzoken */
#define AFF_KNOCKED           41   /* Char is knocked OUT!      */
#define AFF_MIGHT             42   /* Strength +3               */
#define AFF_FLEX              43   /* Agility +3                */
#define AFF_GENIUS            44   /* Intelligence +3           */
#define AFF_BLESS             45   /* Bless for better regen    */
#define AFF_BURNT             46   /* Disintergrated corpse     */
#define AFF_BURNED            47   /* Burned by honoo or similar skill */
#define AFF_MBREAK            48   /* Can't charge while flagged */
#define AFF_HASS              49   /* Does double punch damage  */
#define AFF_FUTURE            50   /* Future Sight */
#define AFF_PARA              51   /* Real Paralyze */
#define AFF_INFUSE            52   /* Ki infused attacks */
#define AFF_ENLIGHTEN         53   /* Enlighten */
#define AFF_FROZEN            54   /* They got frozededed */
#define AFF_FIRESHIELD        55   /* They have a blazing personality */
#define AFF_ENSNARED          56   /* They have silk ensnaring their arms! */
#define AFF_HAYASA            57   /* They are speedy!                */
#define AFF_PURSUIT           58   /* Being followed */
#define AFF_WITHER            59   /* Their body is withered */
#define AFF_HYDROZAP          60   /* Custom Skill Kanso Suru */
#define AFF_POSITION          61   /* Better combat position */
#define AFF_SHOCKED           62   /* Psychic Shock          */
#define AFF_METAMORPH         63   /* Metamorphisis, Demon's Ripoff Custom Skill */
#define AFF_HEALGLOW          64   /* Healing Glow */
#define AFF_EARMOR            65   /* Ethereal Armor */
#define AFF_ECHAINS           66   /* Ethereal Chains */
#define AFF_WUNJO             67   /* Wunjo rune */
#define AFF_POTENT            68   /* Purisaz rune */
#define AFF_ASHED             69   /* Leaves ash */
#define AFF_PUKED             70
#define AFF_LIQUEFIED         71
#define AFF_SHELL             72
#define AFF_IMMUNITY          73
#define AFF_SPIRITCONTROL     74
#define AFF_POSE              75
#define AFF_KYODAIKA          76
#define AFF_SHADOWSTITCH       77
#define AFF_ECHAINS_DEBUFF     78
#define AFF_STARPHASE          79
#define AFF_MBREAK_DEBUFF      80

#define NUM_AFF_FLAGS 81

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING     0    /* Playing - Nominal state		*/
#define CON_CLOSE     1    /* User disconnect, remove character.	*/
#define CON_GET_NAME     2    /* By what name ..?			*/
#define CON_NAME_CNFRM     3    /* Did I get that right, x?		*/
#define CON_PASSWORD     4    /* Password:				*/
#define CON_NEWPASSWD     5    /* Give me a password for x		*/
#define CON_CNFPASSWD     6    /* Please retype password:		*/
#define CON_QSEX     7    /* Sex?					*/
#define CON_QCLASS     8    /* Class?				*/
#define CON_RMOTD     9    /* PRESS RETURN after MOTD		*/
#define CON_MENU     10    /* Your choice: (main menu)		*/
#define CON_EXDESC     11    /* Enter a new description:		*/
#define CON_CHPWD_GETOLD 12    /* Changing passwd: get old		*/
#define CON_CHPWD_GETNEW 13    /* Changing passwd: get new		*/
#define CON_CHPWD_VRFY   14    /* Verify new password			*/
#define CON_DELCNF1     15    /* Delete confirmation 1		*/
#define CON_DELCNF2     16    /* Delete confirmation 2		*/
#define CON_DISCONNECT     17    /* In-game link loss (leave character)	*/
#define CON_OEDIT     18    /* OLC mode - object editor		*/
#define CON_REDIT     19    /* OLC mode - room editor		*/
#define CON_ZEDIT     20    /* OLC mode - zone info editor		*/
#define CON_MEDIT     21    /* OLC mode - mobile editor		*/
#define CON_SEDIT     22    /* OLC mode - shop editor		*/
#define CON_TEDIT     23    /* OLC mode - text editor		*/
#define CON_CEDIT     24    /* OLC mode - config editor		*/
#define CON_QRACE        25     /* Race? 				*/
#define CON_ASSEDIT      26     /* OLC mode - Assemblies                */
#define CON_AEDIT        27    /* OLC mode - social (action) edit      */
#define CON_TRIGEDIT     28    /* OLC mode - trigger edit              */
#define CON_RACE_HELP    29    /* Race Help 				*/
#define CON_CLASS_HELP   30    /* Class Help 				*/
#define CON_QANSI     31    /* Ask for ANSI support     */
#define CON_GEDIT     32    /* OLC mode - guild editor 		*/
#define CON_QROLLSTATS     33    /* Reroll stats 			*/
#define CON_IEDIT        34    /* OLC mode - individual edit		*/
#define CON_LEVELUP     35    /* Level up menu			*/
#define CON_QSTATS     36    /* Assign starting stats        	*/
#define CON_HAIRL        37     /* Choose your hair length        */
#define CON_HAIRS        38     /* Choose your hair style         */
#define CON_HAIRC        39     /* Choose your hair color         */
#define CON_SKIN         40     /* Choose your skin color         */
#define CON_EYE          41     /* Choose your eye color          */
#define CON_Q1           42     /* Make a life choice!            */
#define CON_Q2           43     /* Make a second life choice!     */
#define CON_Q3           44     /* Make a third life choice!      */
#define CON_Q4           45     /* Make a fourth life choice!     */
#define CON_Q5           46     /* Make a fifth life choice!      */
#define CON_Q6           47     /* Make a sixth life choice!      */
#define CON_Q7           48     /* Make a seventh life choice!    */
#define CON_Q8           49     /* Make an eighth life choice!    */
#define CON_Q9           50     /* Make a ninth life choice!      */
#define CON_QX           51     /* Make a tenth life choice!      */
#define CON_HSEDIT       52     /* House Olc                      */
#define CON_ALPHA        53     /* Alpha Password                 */
#define CON_ALPHA2       54     /* Alpha Password For Newb        */
#define CON_ANDROID      55
#define CON_HEDIT        56     /* OLC mode - help edit           */
#define CON_GET_USER     57
#define CON_GET_EMAIL    58
#define CON_UMENU        59
#define CON_USER_CONF    60
#define CON_DISTFEA      61
#define CON_HEIGHT       62
#define CON_WEIGHT       63
#define CON_AURA         64
#define CON_BONUS        65
#define CON_NEGATIVE     66
#define CON_NEWSEDIT     67
#define CON_RACIAL       68
#define CON_POBJ         69
#define CON_ALIGN        70
#define CON_SKILLS       71
#define CON_USER_TITLE   72
#define CON_GENOME       73
#define CON_COPYOVER     74
#define CON_LOGIN        75
#define CON_QUITGAME     76

#define NUM_CON_TYPES 77

/* Colors that the player can define */
#define COLOR_NORMAL            0
#define COLOR_ROOMNAME            1
#define COLOR_ROOMOBJS            2
#define COLOR_ROOMPEOPLE        3
#define COLOR_HITYOU            4
#define COLOR_YOUHIT            5
#define COLOR_OTHERHIT            6
#define COLOR_CRITICAL            7
#define COLOR_HOLLER            8
#define COLOR_SHOUT            9
#define COLOR_GOSSIP            10
#define COLOR_AUCTION            11
#define COLOR_CONGRAT            12
#define COLOR_TELL            13
#define COLOR_YOUSAY            14
#define COLOR_ROOMSAY            15

#define NUM_COLOR            16

/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_UNUSED0    0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_UNUSED1   11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WEAR_WIELD1    16
#define WEAR_WIELD2    17
#define WEAR_BACKPACK  18
#define WEAR_EAR_R     19
#define WEAR_EAR_L     20
#define WEAR_SH        21
#define WEAR_EYE       22

#define NUM_WEARS      23    /* This must be the # of eq positions!! */

#define SPELL_LEVEL_0     0
#define SPELL_LEVEL_1     1
#define SPELL_LEVEL_2     2
#define SPELL_LEVEL_3     3
#define SPELL_LEVEL_4     4
#define SPELL_LEVEL_5     5
#define SPELL_LEVEL_6     6
#define SPELL_LEVEL_7     7
#define SPELL_LEVEL_8     8
#define SPELL_LEVEL_9     9

#define MAX_SPELL_LEVEL   10                    /* how many spell levels */
#define MAX_MEM          (MAX_SPELL_LEVEL * 10) /* how many total spells */

#define DOMAIN_UNDEFINED    (-1)
#define DOMAIN_AIR        0
#define DOMAIN_ANIMAL        1
#define DOMAIN_CHAOS        2
#define DOMAIN_DEATH        3
#define DOMAIN_DESTRUCTION    4
#define DOMAIN_EARTH        5
#define DOMAIN_EVIL        6
#define DOMAIN_FIRE        7
#define DOMAIN_GOOD        8
#define DOMAIN_HEALING        9
#define DOMAIN_KNOWLEDGE    10
#define DOMAIN_LAW        11
#define DOMAIN_LUCK        12
#define DOMAIN_MAGIC        13
#define DOMAIN_PLANT        14
#define DOMAIN_PROTECTION    15
#define DOMAIN_STRENGTH        16
#define DOMAIN_SUN        17
#define DOMAIN_TRAVEL        18
#define DOMAIN_TRICKERY        19
#define DOMAIN_UNIVERSAL    20
#define DOMAIN_WAR        22
#define DOMAIN_WATER        23
#define DOMAIN_ARTIFACE         24
#define DOMAIN_CHARM            25
#define DOMAIN_COMMUNITY        26
#define DOMAIN_CREATION         27
#define DOMAIN_DARKNESS         28
#define DOMAIN_GLORY            29
#define DOMAIN_LIBERATION       30
#define DOMAIN_MADNESS          31
#define DOMAIN_NOBILITY         32
#define DOMAIN_REPOSE           33
#define DOMAIN_RUNE             34
#define DOMAIN_SCALYKIND        35
#define DOMAIN_WEATHER          36

#define NUM_DOMAINS        37

#define SCHOOL_UNDEFINED    (-1)
#define SCHOOL_ABJURATION    0
#define SCHOOL_CONJURATION    1
#define SCHOOL_DIVINATION    2
#define SCHOOL_ENCHANTMENT    3
#define SCHOOL_EVOCATION    4
#define SCHOOL_ILLUSION        5
#define SCHOOL_NECROMANCY    6
#define SCHOOL_TRANSMUTATION    7
#define SCHOOL_UNIVERSAL    8

#define NUM_SCHOOLS        10

#define DEITY_UNDEFINED            (-1)

#define NUM_DEITIES            0

/* Combat feats that apply to a specific weapon type */
#define CFEAT_IMPROVED_CRITICAL            0
#define CFEAT_WEAPON_FINESSE            1
#define CFEAT_WEAPON_FOCUS            2
#define CFEAT_WEAPON_SPECIALIZATION        3
#define CFEAT_GREATER_WEAPON_FOCUS        4
#define CFEAT_GREATER_WEAPON_SPECIALIZATION    5

#define CFEAT_MAX                5

/* Spell feats that apply to a specific school of spells */
#define CFEAT_SPELL_FOCUS            0
#define CFEAT_GREATER_SPELL_FOCUS        1

#define SFEAT_MAX                1

/* object-related defines ********************************************/


/* Item types: used by obj_data.type_flag */
#define ITEM_LIGHT      1        /* Item is a light source	*/
#define ITEM_SCROLL     2        /* Item is a scroll		*/
#define ITEM_WAND       3        /* Item is a wand		*/
#define ITEM_STAFF      4        /* Item is a staff		*/
#define ITEM_WEAPON     5        /* Item is a weapon		*/
#define ITEM_FIREWEAPON 6        /* Unimplemented		*/
#define ITEM_CAMPFIRE   7        /* Burn things for fun!		*/
#define ITEM_TREASURE   8        /* Item is a treasure, not gold	*/
#define ITEM_ARMOR      9        /* Item is armor		*/
#define ITEM_POTION    10        /* Item is a potion		*/
#define ITEM_WORN      11        /* Unimplemented		*/
#define ITEM_OTHER     12        /* Misc object			*/
#define ITEM_TRASH     13        /* Trash - shopkeeps won't buy	*/
#define ITEM_TRAP      14        /* Unimplemented		*/
#define ITEM_CONTAINER 15        /* Item is a container		*/
#define ITEM_NOTE      16        /* Item is note 		*/
#define ITEM_DRINKCON  17        /* Item is a drink container	*/
#define ITEM_KEY       18        /* Item is a key		*/
#define ITEM_FOOD      19        /* Item is food			*/
#define ITEM_MONEY     20        /* Item is money (gold)		*/
#define ITEM_PEN       21        /* Item is a pen		*/
#define ITEM_BOAT      22        /* Item is a boat		*/
#define ITEM_FOUNTAIN  23        /* Item is a fountain		*/
#define ITEM_VEHICLE   24               /* Item is a vehicle            */
#define ITEM_HATCH     25               /* Item is a vehicle hatch      */
#define ITEM_WINDOW    26               /* Item is a vehicle window     */
#define ITEM_CONTROL   27               /* Item is a vehicle control    */
#define ITEM_PORTAL    28               /* Item is a portal	        */
#define ITEM_SPELLBOOK 29               /* Item is a spellbook	        */
#define ITEM_BOARD     30               /* Item is a message board 	*/
#define ITEM_CHAIR     31               /* Is a chair                   */
#define ITEM_BED       32               /* Is a bed                     */
#define ITEM_YUM       33               /* This was good food           */
#define ITEM_PLANT     34               /* This will grow!              */
#define ITEM_FISHPOLE  35               /* FOR FISHING                  */
#define ITEM_FISHBAIT  36               /* DITTO                        */

#define NUM_ITEM_TYPES 37

/* Take/Wear flags: used by obj_data.wear_flags */
#define ITEM_WEAR_TAKE        0  /* Item can be taken         */
#define ITEM_WEAR_FINGER      1  /* Can be worn on finger     */
#define ITEM_WEAR_NECK        2  /* Can be worn around neck   */
#define ITEM_WEAR_BODY        3  /* Can be worn on body       */
#define ITEM_WEAR_HEAD        4  /* Can be worn on head       */
#define ITEM_WEAR_LEGS        5  /* Can be worn on legs       */
#define ITEM_WEAR_FEET        6  /* Can be worn on feet       */
#define ITEM_WEAR_HANDS       7  /* Can be worn on hands      */
#define ITEM_WEAR_ARMS        8  /* Can be worn on arms       */
#define ITEM_WEAR_SHIELD      9  /* Can be used as a shield   */
#define ITEM_WEAR_ABOUT       10 /* Can be worn about body    */
#define ITEM_WEAR_WAIST       11 /* Can be worn around waist  */
#define ITEM_WEAR_WRIST       12 /* Can be worn on wrist      */
#define ITEM_WEAR_WIELD       13 /* Can be wielded            */
#define ITEM_WEAR_HOLD        14 /* Can be held               */
#define ITEM_WEAR_PACK        15 /* Can be worn as a backpack */
#define ITEM_WEAR_EAR         16 /* Can be worn as an earring */
#define ITEM_WEAR_SH          17 /* Can be worn as wings      */
#define ITEM_WEAR_EYE         18 /* Can be worn as a mask     */

#define NUM_ITEM_WEARS 19

/* Extra object flags: used by obj_data.extra_flags */
#define ITEM_GLOW            0  /* Item is glowing              */
#define ITEM_HUM             1  /* Item is humming              */
#define ITEM_NORENT          2  /* Item cannot be rented        */
#define ITEM_NODONATE        3  /* Item cannot be donated       */
#define ITEM_NOINVIS         4  /* Item cannot be made invis    */
#define ITEM_INVISIBLE       5  /* Item is invisible            */
#define ITEM_MAGIC           6  /* Item is magical              */
#define ITEM_NODROP          7  /* Item is cursed: can't drop   */
#define ITEM_BLESS           8  /* Item is blessed              */
#define ITEM_ANTI_GOOD       9  /* Not usable by good people    */
#define ITEM_ANTI_EVIL       10 /* Not usable by evil people    */
#define ITEM_ANTI_NEUTRAL    11 /* Not usable by neutral people */
#define ITEM_ANTI_WIZARD     12 /* Not usable by mages          */
#define ITEM_ANTI_CLERIC     13 /* Not usable by clerics        */
#define ITEM_ANTI_ROGUE      14 /* Not usable by thieves        */
#define ITEM_ANTI_FIGHTER    15 /* Not usable by warriors       */
#define ITEM_NOSELL          16 /* Shopkeepers won't touch it   */
#define ITEM_ANTI_DRUID      17 /* Not usable by druids         */
#define ITEM_2H              18 /* Requires two free hands      */
#define ITEM_ANTI_BARD       19 /* Not usable by bards          */
#define ITEM_ANTI_RANGER     20 /* Not usable by rangers        */
#define ITEM_ANTI_PALADIN    21 /* Not usable by paladins       */
#define ITEM_ANTI_HUMAN      22 /* Not usable by humans         */
#define ITEM_ANTI_ICER       23 /* Not usable by dwarves        */
#define ITEM_ANTI_SAIYAN     24 /* Not usable by elves          */
#define ITEM_ANTI_KONATSU    25 /* Not usable by gnomes         */
#define ITEM_UNIQUE_SAVE     26    /* unique object save           */
#define ITEM_BROKEN          27 /* Item is broken hands         */
#define ITEM_UNBREAKABLE     28 /* Item is unbreakable          */
#define ITEM_ANTI_MONK       29 /* Not usable by monks          */
#define ITEM_ANTI_BARBARIAN  30 /* Not usable by barbarians     */
#define ITEM_ANTI_SORCERER   31 /* Not usable by sorcerers      */
#define ITEM_DOUBLE          32 /* Double weapon                */
#define ITEM_ONLY_WIZARD     33 /* Only usable by mages         */
#define ITEM_ONLY_CLERIC     34 /* Only usable by clerics       */
#define ITEM_ONLY_ROGUE      35 /* Only usable by thieves       */
#define ITEM_ONLY_FIGHTER    36 /* Only usable by warriors      */
#define ITEM_ONLY_DRUID      37 /* Only usable by druids        */
#define ITEM_ONLY_BARD       38 /* Only usable by bards         */
#define ITEM_ONLY_RANGER     39 /* Only usable by rangers       */
#define ITEM_ONLY_PALADIN    40 /* Only usable by paladins      */
#define ITEM_ONLY_HUMAN      41 /* Only usable by humans        */
#define ITEM_ONLY_ICER       42 /* Only usable by dwarves       */
#define ITEM_ONLY_SAIYAN     43 /* Only usable by elves         */
#define ITEM_ONLY_KONATSU    44 /* Only usable by gnomes        */
#define ITEM_ONLY_MONK       45 /* Only usable by monks         */
#define ITEM_ONLY_BARBARIAN  46 /* Only usable by barbarians    */
#define ITEM_ONLY_SORCERER   47 /* Only usable by sorcerers     */
#define ITEM_ANTI_ARCANE_ARCHER        48 /* Not usable by Aracane Archers          */
#define ITEM_ANTI_ARCANE_TRICKSTER     49 /* Not usable by Aracane tricksters       */
#define ITEM_ANTI_ARCHMAGE             50 /* Not usable by Archmages                */
#define ITEM_ANTI_ASSASSIN             51 /* Not usable by Assassins                */
#define ITEM_ANTI_BLACKGUARD           52 /* Not usable by Blackguards              */
#define ITEM_ANTI_DRAGON_DISCIPLE      53 /* Not usable by Dragon disciples         */
#define ITEM_ANTI_DUELIST              54 /* Not usable by Duelists                 */
#define ITEM_ANTI_DWARVEN_DEFENDER     55 /* Not usable by Dwarven defenders        */
#define ITEM_ANTI_ELDRITCH_KNIGHT      56 /* Not usable by Eldritch knights         */
#define ITEM_ANTI_HIEROPHANT           57 /* Not usable by Hierophants              */
#define ITEM_ANTI_HORIZON_WALKER       58 /* Not usable by Horizon walkers          */
#define ITEM_ANTI_LOREMASTER           59 /* Not usable by Loremasters              */
#define ITEM_ANTI_MYSTIC_THEURGE       60 /* Not usable by Mystic theurges          */
#define ITEM_ANTI_SHADOWDANCER         61 /* Not useable by shadowdancers           */
#define ITEM_ANTI_THAUMATURGIST        62 /* Not useable by thaumaturgists          */
#define ITEM_BSCOUTER                  63
#define ITEM_MSCOUTER                  64
#define ITEM_ASCOUTER                  65
#define ITEM_USCOUTER                  66
#define ITEM_WEAPLVL1                  67
#define ITEM_WEAPLVL2                  68
#define ITEM_WEAPLVL3                  69
#define ITEM_WEAPLVL4                  70
#define ITEM_WEAPLVL5                  71
#define ITEM_CBOARD                    72
#define ITEM_FORGED                    73
#define ITEM_SHEATH                    74
#define ITEM_ONLY_JINTO                75
#define ITEM_BURIED                    76
#define ITEM_SLOT1                     77
#define ITEM_SLOT2                     78
#define ITEM_TOKEN                     79
#define ITEM_SLOT_ONE                  80
#define ITEM_SLOTS_FILLED              81
#define ITEM_RESTRING                  82
#define ITEM_CUSTOM                    83
#define ITEM_PROTECTED                 84
#define ITEM_NORANDOM                  85
#define ITEM_THROW                     86
#define ITEM_HOT                       87
#define ITEM_PURGE                     88
#define ITEM_ICE                       89
#define ITEM_DUPLICATE                 90
#define ITEM_MATURE                    91
#define ITEM_CARDCASE                  92
#define ITEM_NOPICKUP                  93
#define ITEM_NOSTEAL                   94

#define NUM_ITEM_FLAGS 96

/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0    /* No effect			*/
#define APPLY_STR               1    /* Apply to strength		*/
#define APPLY_DEX               2    /* Apply to dexterity		*/
#define APPLY_INT               3    /* Apply to intelligence	*/
#define APPLY_WIS               4    /* Apply to wisdom		*/
#define APPLY_CON               5    /* Apply to constitution	*/
#define APPLY_CHA               6    /* Apply to charisma		*/
#define APPLY_SPI               7    /* Reserved			*/
#define APPLY_LEVEL             8    /* Reserved			*/
#define APPLY_AGE               9    /* Apply to age			*/
#define APPLY_CHAR_WEIGHT      10    /* Apply to weight		*/
#define APPLY_CHAR_HEIGHT      11    /* Apply to height		*/
#define APPLY_MANA             12    /* Apply to max mana		*/
#define APPLY_HIT              13    /* Apply to max hit points	*/
#define APPLY_MOVE             14    /* Apply to max move points	*/
#define APPLY_GOLD             15    /* Reserved			*/
#define APPLY_EXP_GAIN_MULT         16    /* Bonus/Penalty to XP gain. +/- % */
#define APPLY_AC               17    /* Apply to Armor Class		*/
#define APPLY_ACCURACY         18    /* Apply to accuracy		*/
#define APPLY_DAMAGE           19    /* Apply to damage 		*/
#define APPLY_REGEN            20    /* Regen Rate Buffed            */
#define APPLY_TRAIN            21    /* Skill training rate buffed   */
#define APPLY_LIFEMAX          22    /* Life Force max buffed        */
#define APPLY_DAMAGE_PERC      23    /* Modify damage inflicted by -/+%			*/
#define APPLY_DEFENSE_PERC     24    /* -/+% damage resistance.		*/
#define APPLY_PL_MULT          25       /* Apply to race                */
#define APPLY_KI_MULT          26       /* Apply to turn undead         */
#define APPLY_ST_MULT          27       /* Apply to spell cast per day  */
#define APPLY_LF_MULT          28       /* Apply to spell cast per day  */
#define APPLY_VITALS_MULT      29       /* Apply to spell cast per day  */
#define APPLY_WEIGHT_MULT      30       /* Apply to spell cast per day  */
#define APPLY_HEIGHT_MULT      31       /* Apply to spell cast per day  */
#define APPLY_PHYS_DAM_PERC    32       /* Bonus percentage to physical damage output.  */
#define APPLY_KI_DAM_PERC      33       /* Bonus percentage to ki damage output.  */
#define APPLY_PHYS_DAM_RES     34       /* Bonus resistance percentage to physical damage received.  */
#define APPLY_KI_DAM_RES       35       /* Bonus resistance percentage to ki damage received. */
#define APPLY_DAM_ATK_TIER     36       /* Bonus perc applied to damage output of a specific attack tier.  */
#define APPLY_KI               37    /* Apply to max ki		*/
#define APPLY_FORTITUDE        38    /* Apply to fortitue save	*/
#define APPLY_REFLEX           39    /* Apply to reflex save		*/
#define APPLY_WILL             40    /* Apply to will save		*/
#define APPLY_SKILL            41       /* Apply to a specific skill    */
#define APPLY_FEAT             42       /* Apply to a specific feat     */
#define APPLY_ALLSAVES         43       /* Apply to all 3 save types 	*/
#define APPLY_RESISTANCE       44       /* Apply to resistance	 	*/
#define APPLY_ALL_ATTRS        45       /* Apply to all attributes	*/
#define APPLY_ALL_VITALS         46       // Apply to all CharStats base.
#define APPLY_SKILL_SLOTS      47     // Add/Remove Skill Slots.
#define APPLY_ATTR_TRAIN_COST  48     // Add/Reduce cost in train points to increase attribute by percent.
#define APPLY_PS_GAIN_MULT          49     // Add/Reduce gained PS from things that grant PS.
#define APPLY_TRANS_ST_UPKEEP  50    // Add/Reduce perc of stamina by % for transformation upkeep.
#define APPLY_VITALS_GAIN_MULT 51     // improves gains to PL/KI/ST/LF by percent.
#define APPLY_PL_GAIN_MULT     52
#define APPLY_ST_GAIN_MULT     53
#define APPLY_KI_GAIN_MULT     54
#define APPLY_LF_GAIN_MULT     55

#define NUM_APPLIES 56

/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)    /* Container can be closed	*/
#define CONT_PICKPROOF      (1 << 1)    /* Container is pickproof	*/
#define CONT_CLOSED         (1 << 2)    /* Container is closed		*/
#define CONT_LOCKED         (1 << 3)    /* Container is locked		*/

#define NUM_CONT_FLAGS 4

/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15

#define NUM_LIQ_TYPES 16

#define MATERIAL_BONE           0
#define MATERIAL_CERAMIC        1
#define MATERIAL_COPPER         2
#define MATERIAL_DIAMOND        3
#define MATERIAL_GOLD           4
#define MATERIAL_IRON           5
#define MATERIAL_LEATHER        6
#define MATERIAL_MITHRIL        7
#define MATERIAL_OBSIDIAN       8
#define MATERIAL_STEEL          9
#define MATERIAL_STONE          10
#define MATERIAL_SILVER         11
#define MATERIAL_WOOD           12
#define MATERIAL_GLASS          13
#define MATERIAL_ORGANIC        14
#define MATERIAL_CURRENCY       15
#define MATERIAL_PAPER          16
#define MATERIAL_COTTON         17
#define MATERIAL_SATIN          18
#define MATERIAL_SILK           19
#define MATERIAL_BURLAP         20
#define MATERIAL_VELVET         21
#define MATERIAL_PLATINUM       22
#define MATERIAL_ADAMANTINE     23
#define MATERIAL_WOOL           24
#define MATERIAL_ONYX           25
#define MATERIAL_IVORY          26
#define MATERIAL_BRASS          27
#define MATERIAL_MARBLE         28
#define MATERIAL_BRONZE         29
#define MATERIAL_KACHIN         30
#define MATERIAL_RUBY           31
#define MATERIAL_SAPPHIRE       32
#define MATERIAL_EMERALD        33
#define MATERIAL_GEMSTONE       34
#define MATERIAL_GRANITE        35
#define MATERIAL_ENERGY         36
#define MATERIAL_HEMP           37
#define MATERIAL_CRYSTAL        38
#define MATERIAL_EARTH    39
#define MATERIAL_LIQUID    40
#define MATERIAL_CLOTH    41
#define MATERIAL_METAL    42
#define MATERIAL_WAX        43
#define MATERIAL_OTHER        44
#define MATERIAL_FOOD        45
#define MATERIAL_OIL        46

#define NUM_MATERIALS           47

/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define HUNGER       1
#define THIRST       2


/* Sun state for weather_data */
#define SUN_DARK    0
#define SUN_RISE    1
#define SUN_LIGHT    2
#define SUN_SET        3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS    0
#define SKY_CLOUDY    1
#define SKY_RAINING    2
#define SKY_LIGHTNING    3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


/* for the 128bits */
#define RF_ARRAY_MAX    4
#define PM_ARRAY_MAX    4
#define PR_ARRAY_MAX    4
#define AF_ARRAY_MAX    4
#define TW_ARRAY_MAX    4
#define EF_ARRAY_MAX    4
#define AD_ARRAY_MAX    4
#define FT_ARRAY_MAX    4
#define ZF_ARRAY_MAX    4


/* History */
#define HIST_ALL       0
#define HIST_SAY       1
#define HIST_GOSSIP    2
#define HIST_WIZNET    3
#define HIST_TELL      4
#define HIST_SHOUT     5
#define HIST_GRATS     6
#define HIST_HOLLER    7
#define HIST_AUCTION   8
#define HIST_SNET      9

#define NUM_HIST      10

/* other #defined constants **********************************************/

/*
 * ADMLVL_IMPL should always be the HIGHEST possible admin level, and
 * ADMLVL_IMMORT should always be the LOWEST immortal level.
 */
#define ADMLVL_NONE        0
#define ADMLVL_IMMORT        1
#define ADMLVL_BUILDER          2
#define ADMLVL_GOD        3
#define ADMLVL_VICE             4
#define ADMLVL_GRGOD        5
#define ADMLVL_IMPL        6

/* First character level that forces epic levels */
#define LVL_EPICSTART        101

/*
 * ADM flags - define admin privs for chars
 */
#define ADM_TELLALL        0    /* Can use 'tell all' to broadcast GOD */
#define ADM_SEEINV        1    /* Sees other chars inventory IMM */
#define ADM_SEESECRET        2    /* Sees secret doors IMM */
#define ADM_KNOWWEATHER        3    /* Knows details of weather GOD */
#define ADM_FULLWHERE        4    /* Full output of 'where' command IMM */
#define ADM_MONEY        5    /* Char has a bottomless wallet GOD */
#define ADM_EATANYTHING    6    /* Char can eat anything GOD */
#define ADM_NOPOISON        7    /* Char can't be poisoned IMM */
#define ADM_WALKANYWHERE    8    /* Char has unrestricted walking IMM */
#define ADM_NOKEYS        9    /* Char needs no keys for locks GOD */
#define ADM_INSTANTKILL        10    /* "kill" command is instant IMPL */
#define ADM_NOSTEAL        11    /* Char cannot be stolen from IMM */
#define ADM_TRANSALL        12    /* Can use 'trans all' GRGOD */
#define ADM_SWITCHMORTAL    13    /* Can 'switch' to a mortal PC body IMPL */
#define ADM_FORCEMASS        14    /* Can force rooms or all GRGOD */
#define ADM_ALLHOUSES        15    /* Can enter any house GRGOD */
#define ADM_NODAMAGE        16    /* Cannot be damaged IMM */
#define ADM_ALLSHOPS        17    /* Can use all shops GOD */
#define ADM_CEDIT        18    /* Can use cedit IMPL */

#define NUM_ADMFLAGS            19

/* Level of the 'freeze' command */
#define ADMLVL_FREEZE    ADMLVL_GRGOD

#define NUM_OF_DIRS    12    /* number of directions in a room (nsewud) */

/*
 * OPT_USEC determines how many commands will be processed by the MUD per
 * second and how frequently it does socket I/O.  A low setting will cause
 * actions to be executed more frequently but will increase overhead due to
 * more cycling to check.  A high setting (e.g. 1 Hz) may upset your players
 * as actions (such as large speedwalking chains) take longer to be executed.
 * You shouldn't need to adjust this.
 */
#define OPT_USEC    100000        /* 10 passes per second */
#define PASSES_PER_SEC    (1000000 / OPT_USEC)
#define RL_SEC        * PASSES_PER_SEC
#define CD_TICK         * PASSES_PER_SEC

#define PULSE_ZONE    (CONFIG_PULSE_ZONE RL_SEC)
#define PULSE_MOBILE    (CONFIG_PULSE_MOBILE RL_SEC)
#define PULSE_VIOLENCE  (CONFIG_PULSE_VIOLENCE RL_SEC)
#define PULSE_AUCTION    (15 RL_SEC)
#define PULSE_AUTOSAVE    (CONFIG_PULSE_AUTOSAVE RL_SEC)
#define PULSE_IDLEPWD    (CONFIG_PULSE_IDLEPWD RL_SEC)
#define PULSE_SANITY    (CONFIG_PULSE_SANITY RL_SEC)
#define PULSE_USAGE    (CONFIG_PULSE_SANITY * 60 RL_SEC)   /* 5 mins */
#define PULSE_TIMESAVE    (CONFIG_PULSE_TIMESAVE * 300 RL_SEC) /* should be >= SECS_PER_MUD_HOUR */
#define PULSE_CURRENT    (CONFIG_PULSE_CURRENT RL_SEC)
#define PULSE_1SEC      (1 RL_SEC)
#define PULSE_2SEC      (2 RL_SEC)
#define PULSE_3SEC      (3 RL_SEC)
#define PULSE_4SEC      (4 RL_SEC)
#define PULSE_5SEC      (5 RL_SEC)
#define PULSE_6SEC      (6 RL_SEC)
#define PULSE_7SEC      (7 RL_SEC)

/* Cool Down Ticks */
#define PULSE_CD1       (1 CD_TICK)
#define PULSE_CD2       (2 CD_TICK)
#define PULSE_CD3       (3 CD_TICK)
#define PULSE_CD4       (4 CD_TICK) /* This and the 3 above are for safety */
#define PULSE_CD5       (5 CD_TICK) /* Punch */
#define PULSE_CD6       (6 CD_TICK)
#define PULSE_CD7       (7 CD_TICK)
#define PULSE_CD8       (8 CD_TICK)
#define PULSE_CD9       (9 CD_TICK)
#define PULSE_CD10      (10 CD_TICK)
#define PULSE_CD11      (11 CD_TICK)
#define PULSE_CD12      (12 CD_TICK)
/* End CD Ticks    */


/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (96 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       1024          /* Max length of prompt        */
#define GARBAGE_SPACE        512          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE        6020        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE       (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define HISTORY_SIZE        5    /* Keep last 5 commands. */
#define MAX_STRING_LENGTH    64936
#define MAX_INPUT_LENGTH    2048    /* Max length per *line* of
input */
#define MAX_RAW_INPUT_LENGTH    4096    /* Max size of *raw* input */
#define MAX_MESSAGES        100
#define MAX_NAME_LENGTH        20
#define MAX_PWD_LENGTH        30
#define MAX_TITLE_LENGTH    120
#define HOST_LENGTH        40
#define EXDSCR_LENGTH        16384
#define MAX_TONGUE        3
#define MAX_SKILLS        200
#define MAX_AFFECT        32
#define MAX_OBJ_AFFECT        6
#define MAX_NOTE_LENGTH        6000    /* arbitrary */
#define SKILL_TABLE_SIZE    1000
#define SPELLBOOK_SIZE        50
#define MAX_FEATS            750
#define MAX_HELP_KEYWORDS       256
#define MAX_HELP_ENTRY          MAX_STRING_LENGTH
#define NUM_FEATS_DEFINED       252
#define MAX_ARMOR_TYPES         5
#define NUM_CONFIG_SECTIONS     7
#define NUM_CREATION_METHODS    5
#define NUM_ATTACK_TYPES        15
#define NUM_MTRIG_TYPES         22
#define NUM_OTRIG_TYPES         22
#define NUM_WTRIG_TYPES         22
#define NUM_ZONE_FLAGS          36
#define NUM_TRADERS             78
#define NUM_SHOP_FLAGS          3
#define NUM_DOOR_CMD            5
#define MAX_ASSM                11
#define NUM_FULLNESS        5
#define NUM_WEEK_DAYS        7
#define NUM_MONTHS        12
#define NUM_CONDITIONS        3
#define NUM_WIELD_NAMES        4

/* define the largest set of commands for a trigger */
#define MAX_CMD_LENGTH          16384 /* 16k should be plenty and then some */


/*
 * A MAX_PWD_LENGTH of 10 will cause BSD-derived systems with MD5 passwords
 * and GNU libc 2 passwords to be truncated.  On BSD this will enable anyone
 * with a name longer than 5 character to log in with any password.  If you
 * have such a system, it is suggested you change the limit to 20.
 *
 * Please note that this will erase your player files.  If you are not
 * prepared to do so, simply erase these lines but heed the above warning.
 */

/* object-related structures ******************************************/
#define NUM_OBJ_VAL_POSITIONS 16

#define VAL_ALL_HEALTH                4
#define VAL_ALL_MAXHEALTH             5
#define VAL_ALL_MATERIAL              7
/*
 * Uses for generic object values on specific object types
 * Please use these instead of numbers to prevent overlaps.
 */
#define VAL_LIGHT_TIME                0
#define VAL_LIGHT_UNUSED2             1
#define VAL_LIGHT_HOURS               2
#define VAL_LIGHT_UNUSED4             3
#define VAL_LIGHT_HEALTH              4
#define VAL_LIGHT_MAXHEALTH           5
#define VAL_LIGHT_UNUSED7             6
#define VAL_LIGHT_MATERIAL            7
#define VAL_SCROLL_LEVEL              0
#define VAL_SCROLL_SPELL1             1
#define VAL_SCROLL_SPELL2             2
#define VAL_SCROLL_SPELL3             3
#define VAL_SCROLL_HEALTH             4
#define VAL_SCROLL_MAXHEALTH          5
#define VAL_SCROLL_UNUSED7            6
#define VAL_SCROLL_MATERIAL           7
#define VAL_WAND_LEVEL                0
#define VAL_WAND_MAXCHARGES           1
#define VAL_WAND_CHARGES              2
#define VAL_WAND_SPELL                3
#define VAL_WAND_HEALTH               4
#define VAL_WAND_MAXHEALTH            5
#define VAL_WAND_UNUSED7              6
#define VAL_WAND_MATERIAL             7
#define VAL_STAFF_LEVEL               0
#define VAL_STAFF_MAXCHARGES          1
#define VAL_STAFF_CHARGES             2
#define VAL_STAFF_SPELL               3
#define VAL_STAFF_HEALTH              4
#define VAL_STAFF_MAXHEALTH           5
#define VAL_STAFF_UNUSED7             6
#define VAL_STAFF_MATERIAL            7
#define VAL_WEAPON_SKILL              0
#define VAL_WEAPON_DAMDICE            1
#define VAL_WEAPON_DAMSIZE            2
#define VAL_WEAPON_DAMTYPE            3
#define VAL_WEAPON_HEALTH             4
#define VAL_WEAPON_MAXHEALTH          5
#define VAL_WEAPON_CRITTYPE           6
#define VAL_WEAPON_MATERIAL           7
#define VAL_WEAPON_CRITRANGE          8
#define VAL_FIREWEAPON_UNUSED1        0
#define VAL_FIREWEAPON_UNUSED2        1
#define VAL_FIREWEAPON_UNUSED3        2
#define VAL_FIREWEAPON_UNUSED4        3
#define VAL_FIREWEAPON_HEALTH         4
#define VAL_FIREWEAPON_MAXHEALTH      5
#define VAL_FIREWEAPON_UNUSED7        6
#define VAL_FIREWEAPON_MATERIAL       7
#define VAL_MISSILE_UNUSED1           0
#define VAL_MISSILE_UNUSED2           1
#define VAL_MISSILE_UNUSED3           2
#define VAL_MISSILE_UNUSED4           3
#define VAL_MISSILE_HEALTH            4
#define VAL_MISSILE_MAXHEALTH         5
#define VAL_MISSILE_UNUSED7           6
#define VAL_MISSILE_MATERIAL          7
#define VAL_TREASURE_UNUSED1          0
#define VAL_TREASURE_UNUSED2          1
#define VAL_TREASURE_UNUSED3          2
#define VAL_TREASURE_UNUSED4          3
#define VAL_TREASURE_HEALTH           4
#define VAL_TREASURE_MAXHEALTH        5
#define VAL_TREASURE_UNUSED7          6
#define VAL_TREASURE_MATERIAL         7
#define VAL_ARMOR_APPLYAC             0
#define VAL_ARMOR_SKILL               1
#define VAL_ARMOR_MAXDEXMOD           2
#define VAL_ARMOR_CHECK               3
#define VAL_ARMOR_HEALTH              4
#define VAL_ARMOR_MAXHEALTH           5
#define VAL_ARMOR_SPELLFAIL           6
#define VAL_ARMOR_MATERIAL            7
#define VAL_POTION_LEVEL              0
#define VAL_POTION_SPELL1             1
#define VAL_POTION_SPELL2             2
#define VAL_POTION_SPELL3             3
#define VAL_POTION_HEALTH             4
#define VAL_POTION_MAXHEALTH          5
#define VAL_POTION_UNUSED7            6
#define VAL_POTION_MATERIAL           7
#define VAL_WORN_UNUSED1              0
#define VAL_WORN_UNUSED2              1
#define VAL_WORN_UNUSED3              2
#define VAL_WORN_UNUSED4              3
#define VAL_WORN_HEALTH               4
#define VAL_WORN_MAXHEALTH            5
#define VAL_WORN_UNUSED7              6
#define VAL_WORN_MATERIAL             7
#define VAL_OTHER_UNUSED1             0
#define VAL_OTHER_UNUSED2             1
#define VAL_OTHER_UNUSED3             2
#define VAL_OTHER_UNUSED4             3
#define VAL_OTHER_HEALTH              4
#define VAL_OTHER_MAXHEALTH           5
#define VAL_OTHER_UNUSED7             6
#define VAL_OTHER_MATERIAL            7
#define VAL_TRASH_UNUSED1             0
#define VAL_TRASH_UNUSED2             1
#define VAL_TRASH_UNUSED3             2
#define VAL_TRASH_UNUSED4             3
#define VAL_TRASH_HEALTH              4
#define VAL_TRASH_MAXHEALTH           5
#define VAL_TRASH_UNUSED7             6
#define VAL_TRASH_MATERIAL            7
#define VAL_TRAP_SPELL                0
#define VAL_TRAP_HITPOINTS            1
#define VAL_TRAP_UNUSED3              2
#define VAL_TRAP_UNUSED4              3
#define VAL_TRAP_HEALTH               4
#define VAL_TRAP_MAXHEALTH            5
#define VAL_TRAP_UNUSED7              6
#define VAL_TRAP_MATERIAL             7
#define VAL_CONTAINER_CAPACITY        0
#define VAL_CONTAINER_FLAGS           1
#define VAL_CONTAINER_KEY             2
#define VAL_CONTAINER_CORPSE          3
#define VAL_CONTAINER_HEALTH          4
#define VAL_CONTAINER_MAXHEALTH       5
#define VAL_CONTAINER_UNUSED7         6
#define VAL_CONTAINER_MATERIAL        7
#define VAL_CONTAINER_OWNER           8
#define VAL_NOTE_LANGUAGE             0
#define VAL_NOTE_UNUSED2              1
#define VAL_NOTE_UNUSED3              2
#define VAL_NOTE_UNUSED4              3
#define VAL_NOTE_HEALTH               4
#define VAL_NOTE_MAXHEALTH            5
#define VAL_NOTE_UNUSED7              6
#define VAL_NOTE_MATERIAL             7
#define VAL_DRINKCON_CAPACITY         0
#define VAL_DRINKCON_HOWFULL          1
#define VAL_DRINKCON_LIQUID           2
#define VAL_DRINKCON_POISON           3
#define VAL_DRINKCON_HEALTH           4
#define VAL_DRINKCON_MAXHEALTH        5
#define VAL_DRINKCON_UNUSED7          6
#define VAL_DRINKCON_MATERIAL         7
#define VAL_KEY_UNUSED1               0
#define VAL_KEY_UNUSED2               1
#define VAL_KEY_KEYCODE               2
#define VAL_KEY_UNUSED4               3
#define VAL_KEY_HEALTH                4
#define VAL_KEY_MAXHEALTH             5
#define VAL_KEY_UNUSED7               6
#define VAL_KEY_MATERIAL              7
#define VAL_FOOD_FOODVAL              0
#define VAL_FOOD_UNUSED2              1
#define VAL_FOOD_UNUSED3              2
#define VAL_FOOD_POISON               3
#define VAL_FOOD_HEALTH               4
#define VAL_FOOD_MAXHEALTH            5
#define VAL_FOOD_UNUSED7              6
#define VAL_FOOD_MATERIAL             7
#define VAL_FOOD_CANDY_PL             8
#define VAL_FOOD_CANDY_KI             9
#define VAL_FOOD_CANDY_ST            10
#define VAL_MONEY_SIZE                0
#define VAL_MONEY_UNUSED2             1
#define VAL_MONEY_UNUSED3             2
#define VAL_MONEY_UNUSED4             3
#define VAL_MONEY_HEALTH              4
#define VAL_MONEY_MAXHEALTH           5
#define VAL_MONEY_UNUSED7             6
#define VAL_MONEY_MATERIAL            7
#define VAL_PEN_UNUSED1               0
#define VAL_PEN_UNUSED2               1
#define VAL_PEN_UNUSED3               2
#define VAL_PEN_UNUSED4               3
#define VAL_PEN_HEALTH                4
#define VAL_PEN_MAXHEALTH             5
#define VAL_PEN_UNUSED7               6
#define VAL_PEN_MATERIAL              7
#define VAL_BOAT_UNUSED1              0
#define VAL_BOAT_UNUSED2              1
#define VAL_BOAT_UNUSED3              2
#define VAL_BOAT_UNUSED4              3
#define VAL_BOAT_HEALTH               4
#define VAL_BOAT_MAXHEALTH            5
#define VAL_BOAT_UNUSED7              6
#define VAL_BOAT_MATERIAL             7
#define VAL_FOUNTAIN_CAPACITY         0
#define VAL_FOUNTAIN_HOWFULL          1
#define VAL_FOUNTAIN_LIQUID           2
#define VAL_FOUNTAIN_POISON           3
#define VAL_FOUNTAIN_HEALTH           4
#define VAL_FOUNTAIN_MAXHEALTH        5
#define VAL_FOUNTAIN_UNUSED7          6
#define VAL_FOUNTAIN_MATERIAL         7
#define VAL_VEHICLE_ROOM              0
#define VAL_VEHICLE_UNUSED2           1
#define VAL_VEHICLE_UNUSED3           2
#define VAL_VEHICLE_APPEAR            3
#define VAL_VEHICLE_HEALTH            4
#define VAL_VEHICLE_MAXHEALTH         5
#define VAL_VEHICLE_UNUSED7           6
#define VAL_VEHICLE_MATERIAL          7
#define VAL_HATCH_DEST                0
#define VAL_HATCH_FLAGS               1
#define VAL_HATCH_DCSKILL             2
#define VAL_HATCH_DCMOVE              3
#define VAL_HATCH_HEALTH              4
#define VAL_HATCH_MAXHEALTH           5
#define VAL_HATCH_UNUSED7             6
#define VAL_HATCH_MATERIAL            7
#define VAL_HATCH_DCLOCK              8
#define VAL_HATCH_DCHIDE              9
#define VAL_WINDOW_UNUSED1            0
#define VAL_WINDOW_UNUSED2            1
#define VAL_WINDOW_UNUSED3            2
#define VAL_WINDOW_UNUSED4            3
#define VAL_WINDOW_HEALTH             4
#define VAL_WINDOW_MAXHEALTH          5
#define VAL_WINDOW_UNUSED7            6
#define VAL_WINDOW_MATERIAL           7
#define VAL_CONTROL_UNUSED1           0
#define VAL_CONTROL_UNUSED2           1
#define VAL_CONTROL_UNUSED3           2
#define VAL_CONTROL_UNUSED4           3
#define VAL_CONTROL_HEALTH            4
#define VAL_CONTROL_MAXHEALTH         5
#define VAL_CONTROL_UNUSED7           6
#define VAL_CONTROL_MATERIAL          7
#define VAL_PORTAL_DEST               0
#define VAL_PORTAL_DCSKILL            1
#define VAL_PORTAL_DCMOVE             2
#define VAL_PORTAL_APPEAR             3
#define VAL_PORTAL_HEALTH             4
#define VAL_PORTAL_MAXHEALTH          5
#define VAL_PORTAL_UNUSED7            6
#define VAL_PORTAL_MATERIAL           7
#define VAL_PORTAL_DCLOCK             8
#define VAL_PORTAL_DCHIDE             9
#define VAL_BOARD_READ                0
#define VAL_BOARD_WRITE               1
#define VAL_BOARD_ERASE               2
#define VAL_BOARD_UNUSED4             3
#define VAL_BOARD_HEALTH              4
#define VAL_BOARD_MAXHEALTH           5
#define VAL_BOARD_UNUSED7             6
#define VAL_BOARD_MATERIAL            7
#define VAL_DOOR_DCLOCK               8
#define VAL_DOOR_DCHIDE               9
#define VAL_CORPSE_HEAD               8
#define VAL_CORPSE_RARM               9
#define VAL_CORPSE_LARM              10
#define VAL_CORPSE_RLEG              11
#define VAL_CORPSE_LLEG              12
#define VAL_GROWTH                    0
#define VAL_MATGOAL                   1
#define VAL_MATURITY                  2
#define VAL_MAXMATURE                 3
#define VAL_WATERLEVEL                6
#define VAL_SOILQ                     8


#define LEVELTYPE_CLASS    1
#define LEVELTYPE_RACE    2


// The IDs in this enum are designed to correlate with the APPLY_<STAT> defines.
// Each ID is APPLY_<STAT> - 1
using attribute_t = uint8_t;
enum class CharAttribute : uint8_t {
 Strength = 0,
 Agility = 1, // lots of code references this as Dexterity. dex
 Intelligence = 2,
 Wisdom = 3,
 Constitution = 4,
 Speed = 5 // this is actually Charisma in the code. cha
};

using attribute_train_t = uint32_t;
enum class CharTrain : uint8_t {
 Strength = 0,
 Agility = 1, // lots of code references this as Dexterity. dex
 Intelligence = 2,
 Wisdom = 3,
 Constitution = 4,
 Speed = 5 // this is actually Charisma in the code. cha
};

using appearance_t = uint8_t;
enum class CharAppearance : uint8_t {
 Sex = 0,
 HairLength = 1,
 HairStyle = 2,
 HairColor = 3,
 SkinColor = 4,
 EyeColor = 5,
 DistinguishingFeature = 6,
 Aura = 7
};

using align_t = int16_t;
enum class CharAlign : uint8_t {
 GoodEvil = 0,
 LawChaos = 1,
};

using money_t = uint64_t;
enum class CharMoney : uint8_t {
 Carried = 0,
 Bank = 1
};

using stat_t = int64_t;
enum class CharStat : uint8_t {
 PowerLevel = 0,
 Ki = 1,
 Stamina = 2
};

using num_t = int;
enum class CharNum : uint8_t {
 Level = 0,
 Wait = 1,
 AdmLevel = 2,
 Height = 3,
 RacialPref = 4,
 MysticMelody = 5,
 GroupKills = 6,
 ArmorWishes = 7

};

using effect_t = uint16_t;