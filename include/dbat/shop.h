/* ************************************************************************
*   File: shop.h                                        Part of CircleMUD *
*  Usage: shop file definitions, structures, constants                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#include "structs.h"

struct shop_buy_data {
    shop_buy_data() = default;
    explicit shop_buy_data(const nlohmann::json& j);
    nlohmann::json serialize();
    int type{};
    std::string keywords{};
};

#define BUY_TYPE(i)        ((i).type)
#define BUY_WORD(i)        ((i).keywords.c_str())

#define SW_ARRAY_MAX    4

struct shop_data {
    shop_data() = default;
    explicit shop_data(const nlohmann::json& j);
    nlohmann::json serialize();
    ~shop_data();
    void add_product(obj_vnum v);
    void remove_product(obj_vnum v);
    shop_vnum vnum{};        /* Virtual number of this shop		*/
    std::vector<obj_vnum> producing{};        /* Which item to produce (virtual)	*/
    float profit_buy{};        /* Factor to multiply cost with		*/
    float profit_sell{};        /* Factor to multiply cost with		*/
    std::vector<shop_buy_data> type{};    /* Which items to trade			*/
    char *no_such_item1{};        /* Message if keeper hasn't got an item	*/
    char *no_such_item2{};        /* Message if player hasn't got an item	*/
    char *missing_cash1{};        /* Message if keeper hasn't got cash	*/
    char *missing_cash2{};        /* Message if player hasn't got cash	*/
    char *do_not_buy{};        /* If keeper dosn't buy such things	*/
    char *message_buy{};        /* Message when player buys item	*/
    char *message_sell{};        /* Message when player sells item	*/
    int temper1{};        /* How does keeper react if no money	*/
    bitvector_t bitvector{};    /* Can attack? Use bank? Cast here?	*/
    mob_vnum keeper{NOBODY};    /* The mobile who owns the shop (rnum)	*/
    bitvector_t with_who[SW_ARRAY_MAX]{};/* Who does the shop trade with?	*/
    std::set<room_vnum> in_room;        /* Where is the shop?			*/
    int open1{}, open2{};        /* When does the shop open?		*/
    int close1{}, close2{};    /* When does the shop close?		*/
    int bankAccount{};        /* Store all gold over 15000 (disabled)	*/
    int lastsort{};        /* How many items are sorted in inven?	*/
    SpecialFunc func{};        /* Secondary spec_proc for shopkeeper	*/
};


#define MAX_TRADE    5    /* List maximums for compatibility	*/
#define MAX_PROD    5    /*	with shops before v3.0		*/
#define VERSION3_TAG    "v3.0"    /* The file has v3.0 shops in it!	*/
#define MAX_SHOP_OBJ    100    /* "Soft" maximum for list maximums	*/


/* Pretty general macros that could be used elsewhere */
#define END_OF(buffer)        ((buffer) + strlen((buffer)))


/* Possible states for objects trying to be sold */
#define OBJECT_DEAD        0
#define OBJECT_NOTOK        1
#define OBJECT_OK        2
#define OBJECT_NOVAL        3


/* Types of lists to read */
#define LIST_PRODUCE        0
#define LIST_TRADE        1
#define LIST_ROOM        2


/* Whom will we not trade with (bitvector for SHOP_TRADE_WITH()) */
#define TRADE_NOGOOD        0
#define TRADE_NOEVIL        1
#define TRADE_NONEUTRAL        2
#define TRADE_NOWIZARD        3
#define TRADE_NOCLERIC        4
#define TRADE_NOROGUE        5
#define TRADE_NOFIGHTER        6
#define TRADE_NOHUMAN           7
#define TRADE_NOICER            8
#define TRADE_NOSAIYAN          9
#define TRADE_NOKONATSU         10
#define TRADE_NONAMEK           11
#define TRADE_NOMUTANT          12
#define TRADE_NOKANASSAN        13
#define TRADE_NOBIO             14
#define TRADE_NOANDROID         15
#define TRADE_NODEMON           16
#define TRADE_NOMAJIN        17
#define TRADE_NOKAI             18
#define TRADE_NOTRUFFLE        19
#define TRADE_NOGOBLIN        20
#define TRADE_NOANIMAL        21
#define TRADE_NOORC            22
#define TRADE_NOSNAKE        23
#define TRADE_NOTROLL        24
#define TRADE_NOHALFBREED        25
#define TRADE_NOMINOTAUR        26
#define TRADE_NOKOBOLD        27
#define TRADE_NOLIZARDFOLK    28
#define TRADE_NOMONK        29
#define TRADE_NOPALADIN        30
#define TRADE_UNUSED        31
#define TRADE_ONLYWIZARD    32
#define TRADE_ONLYCLERIC    33
#define TRADE_ONLYROGUE        34
#define TRADE_ONLYFIGHTER    35
#define TRADE_ONLYMONK        36
#define TRADE_ONLYPALADIN    37
#define TRADE_NOSORCERER        38
#define TRADE_NODRUID           39
#define TRADE_NOBARD            40
#define TRADE_NORANGER          41
#define TRADE_NOBARBARIAN       42
#define TRADE_ONLYSORCERER      43
#define TRADE_ONLYDRUID         44
#define TRADE_ONLYBARD          45
#define TRADE_ONLYRANGER        46
#define TRADE_ONLYBARBARIAN     47
#define TRADE_ONLYARCANE_ARCHER 48
#define TRADE_ONLYARCANE_TRICKSTER 49
#define TRADE_ONLYARCHMAGE      50
#define TRADE_ONLYASSASSIN      51
#define TRADE_ONLYBLACKGUARD    52
#define TRADE_ONLYDRAGON_DISCIPLE 53
#define TRADE_ONLYDUELIST       54
#define TRADE_ONLYDWARVEN_DEFENDER 55
#define TRADE_ONLYELDRITCH_KNIGHT 56
#define TRADE_ONLYHIEROPHANT    57
#define TRADE_ONLYHORIZON_WALKER 58
#define TRADE_ONLYLOREMASTER    59
#define TRADE_ONLYMYSTIC_THEURGE 60
#define TRADE_ONLYSHADOWDANCER  61
#define TRADE_ONLYTHAUMATURGIST 62
#define TRADE_NOARCANE_ARCHER   63
#define TRADE_NOARCANE_TRICKSTER 64
#define TRADE_NOARCHMAGE        65
#define TRADE_NOASSASSIN        66
#define TRADE_NOBLACKGUARD      67
#define TRADE_NODRAGON_DISCIPLE 68
#define TRADE_NODUELIST         69
#define TRADE_NODWARVEN_DEFENDER 70
#define TRADE_NOELDRITCH_KNIGHT 71
#define TRADE_NOHIEROPHANT      72
#define TRADE_NOHORIZON_WALKER  73
#define TRADE_NOLOREMASTER      74
#define TRADE_NOMYSTIC_THEURGE  75
#define TRADE_NOSHADOWDANCER    76
#define TRADE_NOTHAUMATURGIST   77
#define TRADE_NOBROKEN        78

struct stack_data {
    int data[100];
    int len;
};

#define S_DATA(stack, index)    ((stack)->data[(index)])
#define S_LEN(stack)        ((stack)->len)


/* Which expression type we are now parsing */
#define OPER_OPEN_PAREN        0
#define OPER_CLOSE_PAREN    1
#define OPER_OR            2
#define OPER_AND        3
#define OPER_NOT        4
#define MAX_OPER        4


#define SHOP_NUM(i)        (shop_index[(i)].vnum)
#define SHOP_KEEPER(i)        (shop_index[(i)].keeper)
#define SHOP_OPEN1(i)        (shop_index[(i)].open1)
#define SHOP_CLOSE1(i)        (shop_index[(i)].close1)
#define SHOP_OPEN2(i)        (shop_index[(i)].open2)
#define SHOP_CLOSE2(i)        (shop_index[(i)].close2)
#define SHOP_ROOM(i, num)    (shop_index[(i)].in_room[(num)])
#define SHOP_BUYTYPE(i, num)    (BUY_TYPE(shop_index[(i)].type[(num)]))
#define SHOP_BUYWORD(i, num)    (BUY_WORD(shop_index[(i)].type[(num)]))
#define SHOP_PRODUCT(i, num)    (shop_index[(i)].producing[(num)])
#define SHOP_BANK(i)        (shop_index[(i)].bankAccount)
#define SHOP_BROKE_TEMPER(i)    (shop_index[(i)].temper1)
#define SHOP_BITVECTOR(i)    (shop_index[(i)].bitvector)
#define SHOP_TRADE_WITH(i)    (shop_index[(i)].with_who)
#define SHOP_SORT(i)        (shop_index[(i)].lastsort)
#define SHOP_BUYPROFIT(i)    (shop_index[(i)].profit_buy)
#define SHOP_SELLPROFIT(i)    (shop_index[(i)].profit_sell)
#define SHOP_FUNC(i)        (shop_index[(i)].func)

#define NOTRADE_GOOD(i)        (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOGOOD))
#define NOTRADE_EVIL(i)        (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOEVIL))
#define NOTRADE_NEUTRAL(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NONEUTRAL))
#define NOTRADE_WIZARD(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOWIZARD))
#define NOTRADE_CLERIC(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOCLERIC))
#define NOTRADE_ROGUE(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOROGUE))
#define NOTRADE_FIGHTER(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOFIGHTER))
#define NOTRADE_MONK(i)        (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOMONK))
#define NOTRADE_PALADIN(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOPALADIN))
#define NOTRADE_HUMAN(i)        (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOHUMAN))
#define NOTRADE_ICER(i)         (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOICER))
#define NOTRADE_SAIYAN(i)       (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOSAIYAN))
#define NOTRADE_KONATSU(i)      (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOKONATSU))
#define NOTRADE_SORCERER(i)   (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOSORCERER))
#define NOTRADE_DRUID(i)      (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NODRUID))
#define NOTRADE_BARD(i)       (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOBARD))
#define NOTRADE_RANGER(i)     (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NORANGER))
#define NOTRADE_BARBARIAN(i)  (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOBARBARIAN))
#define NOTRADE_HALFBREED(i)  (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOHALFBREED))
#define NOTRADE_NAMEK(i)      (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NONAMEK))
#define NOTRADE_MUTANT(i)     (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOMUTANT))
#define NOTRADE_ARCANE_ARCHER(i)   (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOARCANE_ARCHER))
#define NOTRADE_ARCANE_TRICKSTER(i)(IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOARCANE_TRICKSTER))
#define NOTRADE_ARCHMAGE(i)        (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOARCHMAGE))
#define NOTRADE_ASSASSIN(i)        (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOASSASSIN))
#define NOTRADE_BLACKGUARD(i)      (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOBLACKGUARD))
#define NOTRADE_DRAGON_DISCIPLE(i) (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NODRAGON_DISCIPLE))
#define NOTRADE_DUELIST(i)       (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NODUELIST))
#define NOTRADE_DWARVEN_DEFENDER(i)(IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NODWARVEN_DEFENDER))
#define NOTRADE_ELDRITCH_KNIGHT(i) (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOELDRITCH_KNIGHT))
#define NOTRADE_HIEROPHANT(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOHIEROPHANT))
#define NOTRADE_HORIZON_WALKER(i)  (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOHORIZON_WALKER))
#define NOTRADE_LOREMASTER(i)      (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOLOREMASTER))
#define NOTRADE_MYSTIC_THEURGE(i)  (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOMYSTIC_THEURGE))
#define NOTRADE_SHADOWDANCER(i)    (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOSHADOWDANCER))
#define NOTRADE_THAUMATURGIST(i)   (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOTHAUMATURGIST))
#define NOTRADE_BROKEN(i)       (IS_SET_AR(SHOP_TRADE_WITH((i)), TRADE_NOBROKEN))


#define WILL_START_FIGHT    (1 << 0)
#define WILL_BANK_MONEY        (1 << 1)
#define WILL_ALLOW_STEAL    (1 << 2)

#define SHOP_KILL_CHARS(i)    (IS_SET(SHOP_BITVECTOR(i), WILL_START_FIGHT))
#define SHOP_USES_BANK(i)    (IS_SET(SHOP_BITVECTOR(i), WILL_BANK_MONEY))
#define SHOP_ALLOW_STEAL(i)    (IS_SET(SHOP_BITVECTOR(i), WILL_ALLOW_STEAL))


#define MIN_OUTSIDE_BANK    5000
#define MAX_OUTSIDE_BANK    15000

#define MSG_NOT_OPEN_YET    "Come back later!"
#define MSG_NOT_REOPEN_YET    "Sorry, we have closed, but come back later."
#define MSG_CLOSED_FOR_DAY    "Sorry, come back tomorrow."
#define MSG_NO_STEAL_HERE    "$n is a bloody thief!"
#define MSG_NO_SEE_CHAR        "I don't trade with someone I can't see!"
#define MSG_NO_SELL_ALIGN    "Get out of here before I call the guards!"
#define MSG_NO_SELL_CLASS    "We don't serve your kind here!"
#define MSG_NO_SELL_RACE        "Get lost! We don't serve you kind here!"
#define MSG_NO_USED_WANDSTAFF    "I don't buy used up wands or staves!"
#define MSG_CANT_KILL_KEEPER    "Get out of here before I call the guards!"
#define MSG_NO_BUY_BROKEN    "Sorry, but I don't deal in broken items."

// global variables
extern const char *trade_letters[NUM_TRADERS + 1];
extern const char *shop_bits[];
extern std::map<shop_vnum, struct shop_data> shop_index;
extern int cmd_say, cmd_tell, cmd_emote, cmd_slap, cmd_puke;
extern shop_vnum top_shop;

// functions
extern int shop_producing(struct obj_data *item, vnum shop_nr);

extern int ok_damage_shopkeeper(struct char_data *ch, struct char_data *victim);

extern void boot_the_shops(FILE *shop_f, char *filename, int rec_count);

extern void assign_the_shopkeepers();

extern int ok_shop_room(vnum shop_nr, room_vnum room);

extern void destroy_shops();

extern void show_shops(struct char_data *ch, char *arg);

// special
extern SPECIAL(shop_keeper);
