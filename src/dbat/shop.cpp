/*************************************************************************
*   File: shop.c                                        Part of CircleMUD *
*  Usage: shopkeepers: loading config files, spec procs.                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/***
 * The entire shop rewrite for Circle 3.0 was done by Jeff Fink.  Thanks Jeff!
 ***/
#include "dbat/shop.h"
#include "dbat/comm.h"
#include "dbat/handler.h"
#include "dbat/db.h"
#include "dbat/interpreter.h"
#include "dbat/utils.h"
#include "dbat/constants.h"
#include "dbat/spells.h"
#include "dbat/feats.h"
#include "dbat/act.comm.h"
#include "dbat/act.wizard.h"
#include "dbat/act.social.h"
#include "dbat/dg_comm.h"
#include "dbat/act.other.h"
#include "dbat/class.h"
#include "dbat/genzon.h"
#include "dbat/genshp.h"

/* Forward/External function declarations */
static void sort_keeper_objs(struct char_data *keeper, vnum shop_nr);

/* Local variables */
std::map<shop_vnum, struct shop_data> shop_index;
shop_vnum top_shop = NOTHING;
int cmd_say, cmd_tell, cmd_emote, cmd_slap, cmd_puke;

/* local functions */
static char *read_shop_message(int mnum, room_vnum shr, FILE *shop_f, const char *why);

static int read_list(FILE *shop_f, struct shop_buy_data *list, int new_format, int max, int type);

static void shopping_list(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr);

static void shopping_value(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr);

static void shopping_sell(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr);

static struct obj_data *
get_selling_obj(struct char_data *ch, char *name, struct char_data *keeper, vnum shop_nr, int msg);

static struct obj_data *slide_obj(struct obj_data *obj, struct char_data *keeper, vnum shop_nr);

static void shopping_buy(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr);

static void shopping_app(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr);

static struct obj_data *
get_purchase_obj(struct char_data *ch, char *arg, struct char_data *keeper, vnum shop_nr, int msg);

static struct obj_data *get_hash_obj_vis(struct char_data *ch, char *name, struct obj_data *list);

static struct obj_data *get_slide_obj_vis(struct char_data *ch, char *name, struct obj_data *list);

static char *customer_string(vnum shop_nr, int detailed);

static void list_all_shops(struct char_data *ch);

static void list_detailed_shop(struct char_data *ch, vnum shop_nr);

static int is_ok_char(struct char_data *keeper, struct char_data *ch, vnum shop_nr);

static int is_ok_obj(struct char_data *keeper, struct char_data *ch, struct obj_data *obj, vnum shop_nr);

static int is_open(struct char_data *keeper, vnum shop_nr, int msg);

static int is_ok(struct char_data *keeper, struct char_data *ch, vnum shop_nr);

static void push(struct stack_data *stack, int pushval);

static int top(struct stack_data *stack);

static int pop(struct stack_data *stack);

static void evaluate_operation(struct stack_data *ops, struct stack_data *vals);

static int find_oper_num(char token);

static int evaluate_expression(struct obj_data *obj, char *expr);

static int trade_with(struct obj_data *item, vnum shop_nr);

static int same_obj(struct obj_data *obj1, struct obj_data *obj2);

static int transaction_amt(char *arg);

static char *times_message(struct obj_data *obj, char *name, int num);

static int buy_price(struct obj_data *obj, vnum shop_nr, struct char_data *keeper, struct char_data *buyer);

static int sell_price(struct obj_data *obj, vnum shop_nr, struct char_data *keeper, struct char_data *seller);

static char *list_object(struct obj_data *obj, int cnt, int oindex, vnum shop_nr, struct char_data *keeper,
                         struct char_data *seller);

static int add_to_list(struct shop_buy_data *list, int type, int *len, int *val);

static int end_read_list(struct shop_buy_data *list, int len, int error);

static void read_line(FILE *shop_f, const char *string, void *data);


/* config arrays */
static const char *operator_str[] = {
        "[({",
        "])}",
        "|+",
        "&*",
        "^'"
};

/* Constant list for printing out who we sell to */
const char *trade_letters[NUM_TRADERS + 1] = {
        "No Good",                 /* First, the alignment based ones */
        "No Evil",
        "No Neutral",
        "No Roshi",           /* Then the class based ones */
        "No Piccolo",
        "No Krane",
        "No Nail",
        "No Human",                /* And finally the race based ones */
        "No Icer",
        "No Saiyan",
        "No Konatsu",
        "No Namek",
        "No Mutant",
        "No Kanassan",
        "No Bio",
        "No Android",
        "No Demon",
        "No Majin",
        "No Kai",
        "No Truffle",
        "No Goblin",
        "No Animal",
        "No Orc",
        "No Snake",
        "No Halfbreed",
        "No Minotaur",
        "No Kobold",
        "No Lizardfolk",
        "No Bardock",
        "No Ginyu",
        "UNUSED",
        "Must be Roshi",
        "Must be Piccolo",
        "Must be Krane",
        "Must be Nail",
        "Must be Bardock",
        "Must be Ginyu",
        "No Frieza",
        "No Tapion",
        "No Android 16",
        "No Dabura",
        "No Kabito",
        "Must be Frieza",
        "Must be Tapion",
        "Must be Android 16",
        "Must be Dabura",
        "Must be Kabito",
        "Must be Jinto",
        "Must be Tsuna",
        "Must be Kurzak",
        "Must be Assassin",
        "Must be Blackguard",
        "Must be Dragon Disciple",
        "Must be Duelist",
        "Must be Dwarven Defender",
        "Must be Eldritch Knight",
        "Must be Hierophant",
        "Must be Horizon Walker",
        "Must be Loremaster",
        "Must be Mystic Theurge",
        "Must be Shadowdancer",
        "Must be Thaumaturgist",
        "No Jinto",
        "No Tsuna",
        "No Kurzak",
        "No Assassin",
        "No Blackguard",
        "No Dragon Disciple",
        "No Duelist",
        "No Dwarven Defender",
        "No Eldritch Knight",
        "No Hierophant",
        "No Horizon Walker",
        "No Loremaster",
        "No Mystic Theurge",
        "No Shadowdancer",
        "No Thaumaturgist",
        "\n"
};


const char *shop_bits[] = {
        "WILL_FIGHT",
        "USES_BANK",
        "ALLOW_STEAL",
        "\n"
};

static int is_ok_char(struct char_data *keeper, struct char_data *ch, vnum shop_nr) {
    char buf[MAX_INPUT_LENGTH];

    if (!CAN_SEE(keeper, ch)) {
        char actbuf[MAX_INPUT_LENGTH] = MSG_NO_SEE_CHAR;
        do_say(keeper, actbuf, cmd_say, 0);
        return (false);
    }
    if (ADM_FLAGGED(ch, ADM_ALLSHOPS))
        return (true);

    if ((GET_ALIGNMENT(ch) > 0 && NOTRADE_GOOD(shop_nr)) ||
        (GET_ALIGNMENT(ch) < 0 && NOTRADE_EVIL(shop_nr)) ||
        (GET_ALIGNMENT(ch) == 0 && NOTRADE_NEUTRAL(shop_nr))) {
        snprintf(buf, sizeof(buf), "%s %s", GET_NAME(ch), MSG_NO_SELL_ALIGN);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }
    if (IS_NPC(ch))
        return (true);

    if ((IS_ROSHI(ch) && NOTRADE_WIZARD(shop_nr)) ||
        (IS_PICCOLO(ch) && NOTRADE_CLERIC(shop_nr)) ||
        (IS_KRANE(ch) && NOTRADE_ROGUE(shop_nr)) ||
        (IS_BARDOCK(ch) && NOTRADE_MONK(shop_nr)) ||
        (IS_GINYU(ch) && NOTRADE_PALADIN(shop_nr)) ||
        (IS_NAIL(ch) && NOTRADE_FIGHTER(shop_nr)) ||
        (IS_KABITO(ch) && NOTRADE_BARBARIAN(shop_nr)) ||
        (IS_FRIEZA(ch) && NOTRADE_SORCERER(shop_nr)) ||
        (IS_ANDSIX(ch) && NOTRADE_BARD(shop_nr)) ||
        (IS_DABURA(ch) && NOTRADE_RANGER(shop_nr)) ||
        (IS_TAPION(ch) && NOTRADE_DRUID(shop_nr)) ||
        (IS_NAIL(ch) && NOTRADE_FIGHTER(shop_nr)) ||
        (IS_JINTO(ch) && NOTRADE_ARCANE_ARCHER(shop_nr)) ||
        (IS_TSUNA(ch) && NOTRADE_ARCANE_TRICKSTER(shop_nr)) ||
        (IS_KURZAK(ch) && NOTRADE_ARCHMAGE(shop_nr))) {

        snprintf(buf, sizeof(buf), "%s %s", GET_NAME(ch), MSG_NO_SELL_CLASS);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }
    if ((IS_HUMAN(ch) && NOTRADE_HUMAN(shop_nr)) ||
        (IS_ICER(ch) && NOTRADE_ICER(shop_nr)) ||
        (IS_SAIYAN(ch) && NOTRADE_SAIYAN(shop_nr)) ||
        (IS_KONATSU(ch) && NOTRADE_KONATSU(shop_nr))) {
        snprintf(buf, sizeof(buf), "%s %s", GET_NAME(ch), MSG_NO_SELL_RACE);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }

    return (true);
}

static int is_ok_obj(struct char_data *keeper, struct char_data *ch, struct obj_data *obj, vnum shop_nr) {
    char buf[MAX_INPUT_LENGTH];

    if (OBJ_FLAGGED(obj, ITEM_BROKEN) && NOTRADE_BROKEN(shop_nr)) {
        snprintf(buf, sizeof(buf), "%s %s", GET_NAME(ch), MSG_NO_BUY_BROKEN);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }
    if (OBJ_FLAGGED(obj, ITEM_FORGED)) {
        snprintf(buf, sizeof(buf), "%s that piece of junk is an obvious forgery!", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }

    return (true);
}

static int is_open(struct char_data *keeper, vnum shop_nr, int msg) {
    char buf[MAX_INPUT_LENGTH];

    *buf = '\0';
    if (SHOP_OPEN1(shop_nr) > time_info.hours)
        strlcpy(buf, MSG_NOT_OPEN_YET, sizeof(buf));
    else if (SHOP_CLOSE1(shop_nr) < time_info.hours) {
        if (SHOP_OPEN2(shop_nr) > time_info.hours)
            strlcpy(buf, MSG_NOT_REOPEN_YET, sizeof(buf));
        else if (SHOP_CLOSE2(shop_nr) < time_info.hours)
            strlcpy(buf, MSG_CLOSED_FOR_DAY, sizeof(buf));
    }
    if (!*buf)
        return (true);
    if (msg)
        do_say(keeper, buf, cmd_tell, 0);
    return (false);
}

static int is_ok(struct char_data *keeper, struct char_data *ch, vnum shop_nr) {
    if (is_open(keeper, shop_nr, true))
        return (is_ok_char(keeper, ch, shop_nr));
    else
        return (false);
}

static void push(struct stack_data *stack, int pushval) {
    S_DATA(stack, S_LEN(stack)++) = pushval;
}

static int top(struct stack_data *stack) {
    if (S_LEN(stack) > 0)
        return (S_DATA(stack, S_LEN(stack) - 1));
    else
        return (-1);
}

static int pop(struct stack_data *stack) {
    if (S_LEN(stack) > 0)
        return (S_DATA(stack, --S_LEN(stack)));
    else {
        basic_mud_log("SYSERR: Illegal expression %d in shop keyword list.", S_LEN(stack));
        return (0);
    }
}

static void evaluate_operation(struct stack_data *ops, struct stack_data *vals) {
    int oper;

    if ((oper = pop(ops)) == OPER_NOT)
        push(vals, !pop(vals));
    else {
        int val1 = pop(vals),
                val2 = pop(vals);

        /* Compiler would previously short-circuit these. */
        if (oper == OPER_AND)
            push(vals, val1 && val2);
        else if (oper == OPER_OR)
            push(vals, val1 || val2);
    }
}

static int find_oper_num(char token) {
    int oindex;

    for (oindex = 0; oindex <= MAX_OPER; oindex++)
        if (strchr(operator_str[oindex], token))
            return (oindex);
    return (NOTHING);
}

static int evaluate_expression(struct obj_data *obj, char *expr) {
    struct stack_data ops, vals;
    char *ptr, *end, name[MAX_STRING_LENGTH];
    int temp, eindex;

    if (!expr || !*expr)    /* Allows opening ( first. */
        return (true);

    ops.len = vals.len = 0;
    ptr = expr;
    while (*ptr) {
        if (isspace(*ptr))
            ptr++;
        else {
            if ((temp = find_oper_num(*ptr)) == NOTHING) {
                end = ptr;
                while (*ptr && !isspace(*ptr) && find_oper_num(*ptr) == NOTHING)
                    ptr++;
                strncpy(name, end, ptr - end);    /* strncpy: OK (name/end:MAX_STRING_LENGTH) */
                name[ptr - end] = '\0';
                for (eindex = 0; *extra_bits[eindex] != '\n'; eindex++)
                    if (!strcasecmp(name, extra_bits[eindex])) {
                        push(&vals, OBJ_FLAGGED(obj, eindex));
                        break;
                    }
                if (*extra_bits[eindex] == '\n')
                    push(&vals, isname(name, obj->name));
            } else {
                if (temp != OPER_OPEN_PAREN)
                    while (top(&ops) > temp)
                        evaluate_operation(&ops, &vals);

                if (temp == OPER_CLOSE_PAREN) {
                    if ((temp = pop(&ops)) != OPER_OPEN_PAREN) {
                        basic_mud_log("SYSERR: Illegal parenthesis in shop keyword expression.");
                        return (false);
                    }
                } else
                    push(&ops, temp);
                ptr++;
            }
        }
    }
    while (top(&ops) != -1)
        evaluate_operation(&ops, &vals);
    temp = pop(&vals);
    if (top(&vals) != -1) {
        basic_mud_log("SYSERR: Extra operands left on shop keyword expression stack.");
        return (false);
    }
    return (temp);
}

static int trade_with(struct obj_data *item, vnum shop_nr) {
    int counter;


    if (GET_OBJ_COST(item) < 1)
        return (OBJECT_NOVAL);

    if (OBJ_FLAGGED(item, ITEM_NOSELL))
        return (OBJECT_NOTOK);
    auto &shop = shop_index[shop_nr];

    for (const auto &product : shop.type)
        if (product.type == GET_OBJ_TYPE(item)) {
            if (GET_OBJ_VAL(item, VAL_WAND_CHARGES) == 0 &&
                (GET_OBJ_TYPE(item) == ITEM_WAND ||
                 GET_OBJ_TYPE(item) == ITEM_STAFF))
                return (OBJECT_DEAD);
            else if (evaluate_expression(item, (char*)SHOP_BUYWORD(shop_nr, counter)))
                return (OBJECT_OK);
        }
    return (OBJECT_NOTOK);
}

static int same_obj(struct obj_data *obj1, struct obj_data *obj2) {
    int aindex, i;
    int ef1, ef2;

    if (!obj1 || !obj2)
        return (obj1 == obj2);

    if (GET_OBJ_RNUM(obj1) != GET_OBJ_RNUM(obj2))
        return (false);

    if (GET_OBJ_COST(obj1) != GET_OBJ_COST(obj2))
        return (false);

    for (i = 0; i < EF_ARRAY_MAX; i++) {
        ef1 = GET_OBJ_EXTRA_AR(obj1, i);
        ef2 = GET_OBJ_EXTRA_AR(obj2, i);
        if (i == Q_FIELD(ITEM_UNIQUE_SAVE)) {
            ef1 &= ~Q_BIT(ITEM_UNIQUE_SAVE);
            ef2 &= ~Q_BIT(ITEM_UNIQUE_SAVE);
        }
        if (ef1 != ef2)
            return (false);
    }

    for (aindex = 0; aindex < MAX_OBJ_AFFECT; aindex++)
        if ((obj1->affected[aindex].location != obj2->affected[aindex].location) ||
            (obj1->affected[aindex].modifier != obj2->affected[aindex].modifier))
            return (false);

    return (true);
}

int shop_producing(struct obj_data *item, vnum shop_nr) {
    int counter;

    if (GET_OBJ_RNUM(item) == NOTHING)
        return (false);
    auto &shop = shop_index[shop_nr];

    auto find = std::ranges::find_if(shop.producing, [&](const auto &product) {
        return (product == item->vn);
    });
    return (find != shop.producing.end());
}

static int transaction_amt(char *arg) {
    char buf[MAX_INPUT_LENGTH];

    char *buywhat;

    /*
     * If we have two arguments, it means 'buy 5 3', or buy 5 of #3.
     * We don't do that if we only have one argument, like 'buy 5', buy #5.
     * Code from Andrey Fidrya <andrey@ALEX-UA.COM>
     */
    buywhat = one_argument(arg, buf);
    if (*buywhat && *buf && is_number(buf)) {
        strcpy(arg, arg + strlen(buf) + 1);    /* strcpy: OK (always smaller) */
        return (atoi(buf));
    }
    return (1);
}

static char *times_message(struct obj_data *obj, char *name, int num) {
    static char buf[256];
    size_t len;
    char *ptr;

    if (obj)
        len = strlcpy(buf, obj->short_description, sizeof(buf));
    else {
        if ((ptr = strchr(name, '.')) == nullptr)
            ptr = name;
        else
            ptr++;
        len = snprintf(buf, sizeof(buf), "%s %s", AN(ptr), ptr);
    }

    if (num > 1 && len < sizeof(buf))
        snprintf(buf + len, sizeof(buf) - len, " (x %d)", num);

    return (buf);
}

static struct obj_data *get_slide_obj_vis(struct char_data *ch, char *name,
                                          struct obj_data *list) {
    struct obj_data *i, *last_match = nullptr;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strlcpy(tmpname, name, sizeof(tmpname));
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
        return (nullptr);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
        if (isname(tmp, i->name))
            if (CAN_SEE_OBJ(ch, i) && !same_obj(last_match, i)) {
                if (j == number)
                    return (i);
                last_match = i;
                j++;
            }
    return (nullptr);
}

static struct obj_data *get_hash_obj_vis(struct char_data *ch, char *name,
                                         struct obj_data *list) {
    struct obj_data *loop, *last_obj = nullptr;
    int qindex;

    if (is_number(name))
        qindex = atoi(name);
    else if (is_number(name + 1))
        qindex = atoi(name + 1);
    else
        return (nullptr);

    for (loop = list; loop; loop = loop->next_content)
        if (CAN_SEE_OBJ(ch, loop) && GET_OBJ_COST(loop) > 0)
            if (!same_obj(last_obj, loop)) {
                if (--qindex == 0)
                    return (loop);
                last_obj = loop;
            }
    return (nullptr);
}

static struct obj_data *get_purchase_obj(struct char_data *ch, char *arg,
                                         struct char_data *keeper, vnum shop_nr, int msg) {
    char name[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    one_argument(arg, name);
    do {
        if (*name == '#' || is_number(name))
            obj = get_hash_obj_vis(ch, name, keeper->contents);
        else
            obj = get_slide_obj_vis(ch, name, keeper->contents);
        if (!obj) {
            if (msg) {
                char buf[MAX_INPUT_LENGTH];

                snprintf(buf, sizeof(buf), shop_index[shop_nr].no_such_item1, GET_NAME(ch));
                do_tell(keeper, buf, cmd_tell, 0);
            }
            return (nullptr);
        }
        if (GET_OBJ_COST(obj) <= 0) {
            extract_obj(obj);
            obj = nullptr;
        }
    } while (!obj);
    return (obj);
}

/*
 * Shop purchase adjustment, based on charisma-difference from buyer to keeper.
 *
 * for i in `seq 15 -15`; do printf " * %3d: %6.4f\n" $i \
 * `echo "scale=4; 1+$i/70" | bc`; done
 *
 *  Shopkeeper higher charisma (markup)
 *  ^  15: 1.2142  14: 1.2000  13: 1.1857  12: 1.1714  11: 1.1571
 *  |  10: 1.1428   9: 1.1285   8: 1.1142   7: 1.1000   6: 1.0857
 *  |   5: 1.0714   4: 1.0571   3: 1.0428   2: 1.0285   1: 1.0142
 *  +   0: 1.0000
 *  |  -1: 0.9858  -2: 0.9715  -3: 0.9572  -4: 0.9429  -5: 0.9286
 *  |  -6: 0.9143  -7: 0.9000  -8: 0.8858  -9: 0.8715 -10: 0.8572
 *  v -11: 0.8429 -12: 0.8286 -13: 0.8143 -14: 0.8000 -15: 0.7858
 *  Player higher charisma (discount)
 *
 * Most mobiles have 11 charisma so an 18 charisma player would get a 10%
 * discount beyond the basic price.  That assumes they put a lot of points
 * into charisma, because on the flip side they'd get 11% inflation by
 * having a 3.
 */
static int buy_price(struct obj_data *obj, vnum shop_nr, struct char_data *keeper, struct char_data *buyer) {
    int cost = (GET_OBJ_COST(obj) * SHOP_BUYPROFIT(shop_nr));

    double adjust = 1.0;
    struct obj_data *k;

    for (k = object_list; k; k = k->next) {
        if (GET_OBJ_VNUM(k) == GET_OBJ_VNUM(obj)) {
            adjust -= 0.00025;
        }
    }
    if (adjust < 0.015) {
        adjust = 0.5;
    }

    cost = cost * adjust;

    if (!IS_NPC(buyer) && GET_BONUS(buyer, BONUS_THRIFTY) > 0) {
        if (IS_ARLIAN(buyer)) {
            cost += cost * 0.2;
        }
        cost -= cost * 0.1;
        return (cost);
    } else if (!IS_NPC(buyer) && GET_BONUS(buyer, BONUS_IMPULSE)) {
        cost += cost * 0.25;
        return (cost);
    } else if (!IS_NPC(buyer) && IS_ARLIAN(buyer)) {
        cost += cost * 0.20;
        return (cost);
    } else {
        return (int) (GET_OBJ_COST(obj) * SHOP_BUYPROFIT(shop_nr));
    }
}

/*
 * When the shopkeeper is buying, we reverse the discount. Also make sure
 * we don't buy for more than we sell for, to prevent infinite money-making.
 */
static int sell_price(struct obj_data *obj, vnum shop_nr, struct char_data *keeper, struct char_data *seller) {
    float sell_cost_modifier = SHOP_SELLPROFIT(shop_nr);
    float buy_cost_modifier = SHOP_BUYPROFIT(shop_nr);

    if (sell_cost_modifier > buy_cost_modifier)
        sell_cost_modifier = buy_cost_modifier;

    double adjust = 1.0;
    struct obj_data *k;

    for (k = object_list; k; k = k->next) {
        if (GET_OBJ_VNUM(k) == GET_OBJ_VNUM(obj)) {
            adjust -= 0.00025;
        }
    }
    if (adjust < 0.15) {
        adjust = 0.15;
    }

    if (!IS_NPC(seller) && GET_BONUS(seller, BONUS_THRIFTY) > 0) {
        int haggle = (GET_OBJ_COST(obj) * (sell_cost_modifier / 2));
        if (IS_ARLIAN(seller)) {
            haggle -= haggle * 0.2;
        }
        haggle += haggle * 0.1;
        haggle = haggle * adjust;
        return (haggle);
    } else if (!IS_NPC(seller) && IS_ARLIAN(seller)) {
        int haggle = (GET_OBJ_COST(obj) * (sell_cost_modifier / 2));
        haggle -= haggle * 0.2;
        haggle = haggle * adjust;
        return (haggle);
    } else {
        return (int) ((GET_OBJ_COST(obj) * (sell_cost_modifier / 2)) * adjust);
    }
}

static void shopping_app(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr) {
    struct obj_data *obj;
    int i, found = false;
    char buf[MAX_STRING_LENGTH];

    if (!is_ok(keeper, ch, shop_nr))
        return;

    if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
        sort_keeper_objs(keeper, shop_nr);

    if (!*arg) {
        char buf[MAX_INPUT_LENGTH];

        snprintf(buf, sizeof(buf), "%s What do you want to appraise?", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    if (!(obj = get_purchase_obj(ch, arg, keeper, shop_nr, true))) {
        do_appraise(ch, arg, 0, 0);
        return;
    }

    act("@C$N@W hands you @G$p@W for a moment and let's you examine it before taking it back.@n", true, ch, obj, keeper,
        TO_CHAR);
    act("@c$N@W hands @C$n@W @G$p@W for a moment and let's $m examine it before taking it back.@n", true, ch, obj,
        keeper, TO_ROOM);

    if (!GET_SKILL(ch, SKILL_APPRAISE)) {
        send_to_char(ch, "You are unskilled at appraising.\r\n");
        return;
    }
    improve_skill(ch, SKILL_APPRAISE, 1);
    if (GET_SKILL(ch, SKILL_APPRAISE) < rand_number(1, 101)) {
        send_to_char(ch, "@wYou were completely stumped about the worth of %s@n\r\n", obj->short_description);
        WAIT_STATE(ch, PULSE_2SEC);
        return;
    } else {
        int displevel = GET_OBJ_LEVEL(obj);

        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON && OBJ_FLAGGED(obj, ITEM_CUSTOM))
            displevel = 20;

        send_to_char(ch, "@c---------------------------------------------------------------@n\n");
        send_to_char(ch, "@GItem Name   @W: @w%s@n\n", obj->short_description);
        send_to_char(ch, "@GTrue Value  @W: @Y%s@n\n", add_commas(GET_OBJ_COST(obj)).c_str());
        send_to_char(ch, "@GItem Min LVL@W: @w%d@n\n", displevel);
        if (GET_OBJ_VAL(obj, VAL_ALL_HEALTH) >= 100) {
            send_to_char(ch, "@GCondition   @W: @C%d%s@n\n", GET_OBJ_VAL(obj, VAL_ALL_HEALTH), "%");
        } else if (GET_OBJ_VAL(obj, VAL_ALL_HEALTH) >= 50) {
            send_to_char(ch, "@GCondition   @W: @y%d%s@n\n", GET_OBJ_VAL(obj, VAL_ALL_HEALTH), "%");
        } else if (GET_OBJ_VAL(obj, VAL_ALL_HEALTH) >= 1) {
            send_to_char(ch, "@GCondition   @W: @r%d%s@n\n", GET_OBJ_VAL(obj, VAL_ALL_HEALTH), "%");
        } else {
            send_to_char(ch, "@GCondition   @W: @D%d%s@n\n", GET_OBJ_VAL(obj, VAL_ALL_HEALTH), "%");
        }
        send_to_char(ch, "@GItem Size   @W:@w %s@n\n", size_names[GET_OBJ_SIZE(obj)]);
        send_to_char(ch, "@GItem Weight @W: @w%s@n\n", add_commas(GET_OBJ_WEIGHT(obj)).c_str());
        if (OBJ_FLAGGED(obj, ITEM_SLOT1) && !OBJ_FLAGGED(obj, ITEM_SLOTS_FILLED)) {
            send_to_char(ch, "GToken Slots  @W: @m0/1@n\n");
        } else if (OBJ_FLAGGED(obj, ITEM_SLOT1) && OBJ_FLAGGED(obj, ITEM_SLOTS_FILLED)) {
            send_to_char(ch, "GToken Slots  @W: @m1/1@n\n");
        } else if (OBJ_FLAGGED(obj, ITEM_SLOT2) && !OBJ_FLAGGED(obj, ITEM_SLOT_ONE) &&
                   !OBJ_FLAGGED(obj, ITEM_SLOTS_FILLED)) {
            send_to_char(ch, "GToken Slots  @W: @m0/2@n\n");
        } else if (OBJ_FLAGGED(obj, ITEM_SLOT2) && OBJ_FLAGGED(obj, ITEM_SLOT_ONE) &&
                   !OBJ_FLAGGED(obj, ITEM_SLOTS_FILLED)) {
            send_to_char(ch, "GToken Slots  @W: @m1/2@n\n");
        } else if (OBJ_FLAGGED(obj, ITEM_SLOT2) && !OBJ_FLAGGED(obj, ITEM_SLOTS_FILLED)) {
            send_to_char(ch, "GToken Slots  @W: @m2/2@n\n");
        }
        char bits[MAX_STRING_LENGTH];
        sprintbitarray(GET_OBJ_WEAR(obj), wear_bits, TW_ARRAY_MAX, bits);
        search_replace(bits, "TAKE", "");
        send_to_char(ch, "@GWear Loc.   @W:@w%s\n", bits);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
            if (OBJ_FLAGGED(obj, ITEM_WEAPLVL1)) {
                send_to_char(ch, "@GWeapon Level@W: @D[@C1@D]\n@GDamage Bonus@W: @D[@w5%s@D]@n\r\n", "%");
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL2)) {
                send_to_char(ch, "@GWeapon Level@W: @D[@C2@D]\n@GDamage Bonus@W: @D[@w10%s@D]@n\r\n", "%");
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL3)) {
                send_to_char(ch, "@GWeapon Level@W: @D[@C3@D]\n@GDamage Bonus@W: @D[@w20%s@D]@n\r\n", "%");
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL4)) {
                send_to_char(ch, "@GWeapon Level@W: @D[@C4@D]\n@GDamage Bonus@W: @D[@w30%s@D]@n\r\n", "%");
            } else if (OBJ_FLAGGED(obj, ITEM_WEAPLVL5)) {
                send_to_char(ch, "@GWeapon Level@W: @D[@C5@D]\n@GDamage Bonus@W: @D[@w50%s@D]@n\r\n", "%");
            }
        }
        send_to_char(ch, "@GItem Bonuses@W:@w");
        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].modifier) {
                sprinttype(obj->affected[i].location, apply_types, buf, sizeof(buf));
                send_to_char(ch, "%s %+f to %s", found++ ? "," : "", obj->affected[i].modifier, buf);
                switch (obj->affected[i].location) {
                    case APPLY_FEAT:
                        send_to_char(ch, " (%s)", feat_list[obj->affected[i].specific].name);
                        break;
                    case APPLY_SKILL:
                        send_to_char(ch, " (%s)", spell_info[obj->affected[i].specific].name);
                        break;
                }
            }
        }
        if (!found)
            send_to_char(ch, " None@n");
        else
            send_to_char(ch, "@n");
        char buf2[MAX_STRING_LENGTH];
        sprintbitarray(GET_OBJ_PERM(obj), affected_bits, AF_ARRAY_MAX, buf2);
        send_to_char(ch, "\n@GSpecial     @W:@w %s", buf2);
        send_to_char(ch, "\n@c---------------------------------------------------------------@n\n");
    }
}

static void shopping_buy(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr) {
    char tempstr[MAX_INPUT_LENGTH], tempbuf[MAX_INPUT_LENGTH];
    struct obj_data *obj, *last_obj = nullptr;
    int goldamt = 0, buynum, bought = 0;

    if (!is_ok(keeper, ch, shop_nr))
        return;

    if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
        sort_keeper_objs(keeper, shop_nr);

    if ((buynum = transaction_amt(arg)) < 0) {
        char buf[MAX_INPUT_LENGTH];

        snprintf(buf, sizeof(buf), "%s A negative amount?  Try selling me something.",
                 GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    if (!*arg || !buynum) {
        char buf[MAX_INPUT_LENGTH];

        snprintf(buf, sizeof(buf), "%s What do you want to buy?", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    if (!(obj = get_purchase_obj(ch, arg, keeper, shop_nr, true)))
        return;

    if (buy_price(obj, shop_nr, keeper, ch) > GET_GOLD(ch) && !ADM_FLAGGED(ch, ADM_MONEY)) {
        char actbuf[MAX_INPUT_LENGTH];

        auto &sh = shop_index[shop_nr];

        snprintf(actbuf, sizeof(actbuf), sh.missing_cash2 ? sh.missing_cash2 : "You can't afford that!", GET_NAME(ch));
        do_tell(keeper, actbuf, cmd_tell, 0);

        switch (SHOP_BROKE_TEMPER(shop_nr)) {
            case 0:
                do_action(keeper, strcpy(actbuf, GET_NAME(ch)), cmd_puke,
                          0);    /* strcpy: OK (MAX_NAME_LENGTH < MAX_INPUT_LENGTH) */
                return;
            case 1:
                do_echo(keeper, strcpy(actbuf, "smokes on his joint."), cmd_emote, SCMD_EMOTE);    /* strcpy: OK */
                return;
            default:
                return;
        }
    }
    if (IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch)) {
        send_to_char(ch, "%s: You can't carry any more items.\r\n", fname(obj->name));
        return;
    }
    if (!ch->canCarryWeight(obj)) {
        send_to_char(ch, "%s: You can't carry that much weight.\r\n", fname(obj->name));
        return;
    }
    while (obj && (GET_GOLD(ch) >= buy_price(obj, shop_nr, keeper, ch) || ADM_FLAGGED(ch, ADM_MONEY))
           && IS_CARRYING_N(ch) < CAN_CARRY_N(ch) && bought < buynum
           && ch->canCarryWeight(obj)) {
        int charged;

        bought++;
        /* Test if producing shop ! */
        if (shop_producing(obj, shop_nr)) {
            obj = read_object(GET_OBJ_RNUM(obj), REAL);
        } else {
            obj_from_char(obj);
            SHOP_SORT(shop_nr)--;
        }
        obj_to_char(obj, ch);
        if (OBJ_FLAGGED(obj, ITEM_MATURE)) {
            GET_OBJ_VAL(obj, VAL_MAXMATURE) = 6;
        }

        charged = buy_price(obj, shop_nr, keeper, ch);
        goldamt += charged;
        if (!ADM_FLAGGED(ch, ADM_MONEY))
            ch->mod(CharMoney::Carried, -charged);
        else {
            send_to_imm("IMM PURCHASE: %s has purchased %s for free.", GET_NAME(ch), obj->short_description);
            log_imm_action("IMM PURCHASE: %s has purchased %s for free.", GET_NAME(ch), obj->short_description);
        }

        last_obj = obj;
        obj = get_purchase_obj(ch, arg, keeper, shop_nr, false);
        if (!same_obj(obj, last_obj))
            break;
    }

    if (bought < buynum) {
        char buf[MAX_INPUT_LENGTH];

        if (!obj || !same_obj(last_obj, obj))
            snprintf(buf, sizeof(buf), "%s I only have %d to sell you.", GET_NAME(ch), bought);
        else if (GET_GOLD(ch) < buy_price(obj, shop_nr, keeper, ch))
            snprintf(buf, sizeof(buf), "%s You can only afford %d.", GET_NAME(ch), bought);
        else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
            snprintf(buf, sizeof(buf), "%s You can only hold %d.", GET_NAME(ch), bought);
        else if (!ch->canCarryWeight(obj))
            snprintf(buf, sizeof(buf), "%s You can only carry %d.", GET_NAME(ch), bought);
        else
            snprintf(buf, sizeof(buf), "%s Something screwy only gave you %d.", GET_NAME(ch), bought);
        do_tell(keeper, buf, cmd_tell, 0);
    }
    if (!ADM_FLAGGED(ch, ADM_MONEY))
        keeper->mod(CharMoney::Carried, goldamt);

    strlcpy(tempstr, times_message(ch->contents, nullptr, bought), sizeof(tempstr));

    snprintf(tempbuf, sizeof(tempbuf), "$n buys %s.", tempstr);
    act(tempbuf, false, ch, obj, nullptr, TO_ROOM);

    snprintf(tempbuf, sizeof(tempbuf), shop_index[shop_nr].message_buy, GET_NAME(ch), goldamt);
    do_tell(keeper, tempbuf, cmd_tell, 0);

    send_to_char(ch, "You now have %s.\r\n", tempstr);

    if (SHOP_USES_BANK(shop_nr))
        if (GET_GOLD(keeper) > MAX_OUTSIDE_BANK) {
            SHOP_BANK(shop_nr) += (GET_GOLD(keeper) - MAX_OUTSIDE_BANK);
            keeper->set(CharMoney::Carried, MAX_OUTSIDE_BANK);
        }
}

static struct obj_data *
get_selling_obj(struct char_data *ch, char *name, struct char_data *keeper, vnum shop_nr, int msg) {
    char buf[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int result;

    if (!(obj = get_obj_in_list_vis(ch, name, nullptr, ch->contents))) {
        if (msg) {
            char tbuf[MAX_INPUT_LENGTH];

            snprintf(tbuf, sizeof(tbuf), shop_index[shop_nr].no_such_item2, GET_NAME(ch));
            do_tell(keeper, tbuf, cmd_tell, 0);
        }
        return (nullptr);
    }
    if ((result = trade_with(obj, shop_nr)) == OBJECT_OK)
        return (obj);

    if (!msg)
        return (nullptr);

    switch (result) {
        case OBJECT_NOVAL:
            snprintf(buf, sizeof(buf), "%s You've got to be kidding, that thing is worthless!", GET_NAME(ch));
            break;
        case OBJECT_NOTOK:
            snprintf(buf, sizeof(buf), shop_index[shop_nr].do_not_buy, GET_NAME(ch));
            break;
        case OBJECT_DEAD:
            snprintf(buf, sizeof(buf), "%s %s", GET_NAME(ch), MSG_NO_USED_WANDSTAFF);
            break;
        default:
            basic_mud_log("SYSERR: Illegal return value of %d from trade_with() (%s)", result,
                __FILE__);    /* Someone might rename it... */
            snprintf(buf, sizeof(buf), "%s An error has occurred.", GET_NAME(ch));
            break;
    }
    do_tell(keeper, buf, cmd_tell, 0);
    return (nullptr);
}

static struct obj_data *slide_obj(struct obj_data *obj, struct char_data *keeper,
                                  vnum shop_nr)
/*
   This function is a slight hack!  To make sure that duplicate items are
   only listed once on the "list", this function groups "identical"
   objects together on the shopkeeper's inventory list.  The hack involves
   knowing how the list is put together, and manipulating the order of
   the objects on the list.  (But since most of DIKU is not encapsulated,
   and information hiding is almost never used, it isn't that big a deal) -JF
*/
{
    struct obj_data *loop;
    int temp;

    if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
        sort_keeper_objs(keeper, shop_nr);

    /* Extract the object if it is identical to one produced */
    if (shop_producing(obj, shop_nr)) {
        temp = GET_OBJ_RNUM(obj);
        extract_obj(obj);
        return (&obj_proto[temp]);
    }
    SHOP_SORT(shop_nr)++;
    loop = keeper->contents;
    obj_to_char(obj, keeper);
    keeper->contents = loop;
    while (loop) {
        if (same_obj(obj, loop)) {
            obj->next_content = loop->next_content;
            loop->next_content = obj;
            return (obj);
        }
        loop = loop->next_content;
    }
    keeper->contents = obj;
    return (obj);
}

static void sort_keeper_objs(struct char_data *keeper, vnum shop_nr) {
    struct obj_data *list = nullptr, *temp;

    while (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper)) {
        temp = keeper->contents;
        obj_from_char(temp);
        temp->next_content = list;
        list = temp;
    }

    while (list) {
        temp = list;
        list = list->next_content;
        if (shop_producing(temp, shop_nr) &&
            !get_obj_in_list_num(GET_OBJ_RNUM(temp), keeper->contents)) {
            obj_to_char(temp, keeper);
            SHOP_SORT(shop_nr)++;
        } else
            slide_obj(temp, keeper, shop_nr);
    }
}

static void shopping_sell(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr) {
    char tempstr[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], tempbuf[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int sellnum, sold = 0, goldamt = 0;

    if (!(is_ok(keeper, ch, shop_nr)))
        return;

    if ((sellnum = transaction_amt(arg)) < 0) {
        char buf[MAX_INPUT_LENGTH];

        snprintf(buf, sizeof(buf), "%s A negative amount?  Try buying something.", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    if (!*arg || !sellnum) {
        char buf[MAX_INPUT_LENGTH];

        snprintf(buf, sizeof(buf), "%s What do you want to sell??", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    one_argument(arg, name);
    if (!(obj = get_selling_obj(ch, name, keeper, shop_nr, true)))
        return;

    if (GET_OBJ_TYPE(obj) == ITEM_PLANT && GET_OBJ_VAL(obj, VAL_WATERLEVEL) <= -10) {
        char buf[MAX_INPUT_LENGTH];
        snprintf(buf, sizeof(buf), "%s That thing is dead!", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }

    if (!(is_ok_obj(keeper, ch, obj, shop_nr)))
        return;

    if (GET_GOLD(keeper) + SHOP_BANK(shop_nr) < sell_price(obj, shop_nr, keeper, ch)) {
        char buf[MAX_INPUT_LENGTH];

        snprintf(buf, sizeof(buf), shop_index[shop_nr].missing_cash1, GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    while (obj && GET_GOLD(keeper) + SHOP_BANK(shop_nr) >= sell_price(obj, shop_nr, keeper, ch) && sold < sellnum) {
        int charged = sell_price(obj, shop_nr, keeper, ch);

        goldamt += charged;
        keeper->mod(CharMoney::Carried, -charged);

        sold++;
        obj_from_char(obj);
        slide_obj(obj, keeper, shop_nr);    /* Seems we don't use return value. */
        obj = get_selling_obj(ch, name, keeper, shop_nr, false);
    }

    if (sold < sellnum) {
        char buf[MAX_INPUT_LENGTH];

        if (!obj)
            snprintf(buf, sizeof(buf), "%s You only have %d of those.", GET_NAME(ch), sold);
        else if (GET_GOLD(keeper) + SHOP_BANK(shop_nr) < sell_price(obj, shop_nr, keeper, ch))
            snprintf(buf, sizeof(buf), "%s I can only afford to buy %d of those.", GET_NAME(ch), sold);
        else
            snprintf(buf, sizeof(buf), "%s Something really screwy made me buy %d.", GET_NAME(ch), sold);

        do_tell(keeper, buf, cmd_tell, 0);
    }
    strlcpy(tempstr, times_message(nullptr, name, sold), sizeof(tempstr));
    snprintf(tempbuf, sizeof(tempbuf), "$n sells something to %s.\r\n", GET_NAME(keeper));
    act(tempbuf, false, ch, obj, nullptr, TO_ROOM);

    snprintf(tempbuf, sizeof(tempbuf), shop_index[shop_nr].message_sell, GET_NAME(ch), goldamt);
    do_tell(keeper, tempbuf, cmd_tell, 0);

    send_to_char(ch, "The shopkeeper gives you %s zenni.\r\n", add_commas(goldamt).c_str());
    if (GET_GOLD(ch) + goldamt > GOLD_CARRY(ch)) {
        goldamt = (GET_GOLD(ch) + goldamt) - GOLD_CARRY(ch);
        ch->set(CharMoney::Carried, GOLD_CARRY(ch));
        ch->mod(CharMoney::Bank, goldamt);
        send_to_char(ch, "You couldn't hold all of the money. The rest was deposited for you.\r\n");
    } else {
        ch->mod(CharMoney::Carried, goldamt);
    }

    if (GET_GOLD(keeper) < MIN_OUTSIDE_BANK) {
        goldamt = MIN(MAX_OUTSIDE_BANK - GET_GOLD(keeper), SHOP_BANK(shop_nr));
        SHOP_BANK(shop_nr) -= goldamt;
        keeper->mod(CharMoney::Carried, goldamt);
    }
}

static void shopping_value(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr) {
    char buf[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
    struct obj_data *obj;

    if (!is_ok(keeper, ch, shop_nr))
        return;

    if (!*arg) {
        snprintf(buf, sizeof(buf), "%s What do you want me to evaluate??", GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }
    one_argument(arg, name);
    if (!(obj = get_selling_obj(ch, name, keeper, shop_nr, true)))
        return;

    if (!is_ok_obj(keeper, ch, obj, shop_nr))
        return;

    snprintf(buf, sizeof(buf), "%s I'll give you %d zenni for that!", GET_NAME(ch),
             sell_price(obj, shop_nr, keeper, ch));
    do_tell(keeper, buf, cmd_tell, 0);
}

static char *
list_object(struct obj_data *obj, int cnt, int aindex, vnum shop_nr, struct char_data *keeper, struct char_data *ch) {
    static char result[256];
    char itemname[128],
            quantity[16];    /* "Unlimited" or "%d" */

    if (shop_producing(obj, shop_nr))
        strcpy(quantity, "Unlimited");    /* strcpy: OK (for 'quantity >= 10') */
    else
        sprintf(quantity, "%d", cnt);    /* sprintf: OK (for 'quantity >= 11', 32-bit int) */

    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_DRINKCON:
            if (GET_OBJ_VAL(obj, VAL_DRINKCON_HOWFULL))
                snprintf(itemname, sizeof(itemname), "%s", obj->short_description);
            else
                strlcpy(itemname, obj->short_description, sizeof(itemname));
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            snprintf(itemname, sizeof(itemname), "%s%s", obj->short_description,
                     GET_OBJ_VAL(obj, VAL_WAND_CHARGES) < GET_OBJ_VAL(obj, VAL_WAND_MAXCHARGES) ? " (partially used)"
                                                                                                : "");
            break;

        default:
            strlcpy(itemname, obj->short_description, sizeof(itemname));
            break;
    }
    if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
        char titemname[128];
        strlcpy(titemname, itemname, sizeof(titemname));
        snprintf(itemname, sizeof(itemname), "%s [broken]", titemname);
    }

    CAP(itemname);

    int displevel = GET_OBJ_LEVEL(obj);

    if (GET_OBJ_TYPE(obj) == ITEM_WEAPON && OBJ_FLAGGED(obj, ITEM_CUSTOM))
        displevel = 20;

    snprintf(result, sizeof(result), " %2d)  %9s %-*s %3d %13s\r\n", aindex, quantity,
             count_color_chars(itemname) + 36, itemname, displevel, add_commas(buy_price(obj, shop_nr, keeper, ch)).c_str());
    return (result);
}

static void shopping_list(char *arg, struct char_data *ch, struct char_data *keeper, vnum shop_nr) {
    char buf[MAX_STRING_LENGTH * 4], name[MAX_INPUT_LENGTH];
    struct obj_data *obj, *last_obj = nullptr;
    int cnt = 0, lindex = 0, found = false;
    size_t len;
    /* cnt is the number of that particular object available */

    if (!is_ok(keeper, ch, shop_nr))
        return;

    if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
        sort_keeper_objs(keeper, shop_nr);

    one_argument(arg, name);

    len = strlcpy(buf, " ##   Available   Item                             Min. Lvl       Cost\r\n"
                       "----------------------------------------------------------------------\r\n", sizeof(buf));
    if (keeper->contents)
        for (obj = keeper->contents; obj; obj = obj->next_content)
            if (CAN_SEE_OBJ(ch, obj) && GET_OBJ_COST(obj) > 0) {
                if (!last_obj) {
                    last_obj = obj;
                    cnt = 1;
                } else if (same_obj(last_obj, obj))
                    cnt++;
                else {
                    lindex++;
                    if (!*name || isname(name, last_obj->name)) {
                        strncat(buf, list_object(last_obj, cnt, lindex, shop_nr, keeper, ch),
                                sizeof(buf) - len - 1);    /* strncat: OK */
                        len = strlen(buf);
                        if (len + 1 >= sizeof(buf))
                            break;
                        found = true;
                    }
                    cnt = 1;
                    last_obj = obj;
                }
            }
    lindex++;
    if (!last_obj)    /* we actually have nothing in our list for sale, period */
        send_to_char(ch, "Currently, there is nothing for sale.\r\n");
    else if (*name && !found)    /* nothing the char was looking for was found */
        send_to_char(ch, "Presently, none of those are for sale.\r\n");
    else {
        char zen[80];
        if (!*name || isname(name, last_obj->name))    /* show last obj */
            if (len < sizeof(buf)) {
                strncat(buf, list_object(last_obj, cnt, lindex, shop_nr, keeper, ch),
                        sizeof(buf) - len - 1);    /* strncat: OK */
            }
        if (len < sizeof(buf)) {
            sprintf(zen, "@W[@wYour Zenni@D: @Y%s@W]", add_commas(GET_GOLD(ch)).c_str());
            strncat(buf, zen, sizeof(buf) - len - 1);
        }
        write_to_output(ch->desc, buf);
    }
}

int ok_shop_room(vnum shop_nr, room_vnum room) {
    int mindex;
    auto &sh = shop_index[shop_nr];

    return sh.in_room.contains(room);
}

SPECIAL(shop_keeper) {
    auto keeper = (struct char_data *) me;
    vnum shop_nr = NOTHING;

    for (auto &[nr, sh] : shop_index) {
        if (sh.keeper == keeper->vn) {
            shop_nr = nr;
            break;
        }
    }


    if (!shop_index.count(shop_nr))
        return false;

    auto &sh = shop_index[shop_nr];

    if (sh.func)    /* Check secondary function */
        if (sh.func(ch, me, cmd, argument))
            return (true);

    if (keeper == ch) {
        if (cmd)
            SHOP_SORT(shop_nr) = 0;    /* Safety in case "drop all" */
        return (false);
    }
    if (!ok_shop_room(shop_nr, GET_ROOM_VNUM(IN_ROOM(ch))))
        return (0);

    if (!AWAKE(keeper))
        return (false);

    if (CMD_IS("steal")) {
        char argm[MAX_INPUT_LENGTH];

        if (!SHOP_ALLOW_STEAL(shop_nr)) {
            snprintf(argm, sizeof(argm), "$N shouts '%s'", MSG_NO_STEAL_HERE);
            act(argm, false, ch, nullptr, keeper, TO_CHAR);
            act(argm, false, ch, nullptr, keeper, TO_ROOM);
            do_action(keeper, GET_NAME(ch), cmd_slap, 0);

            return (true);
        } else {
            return (false);
        }
    }

    if (CMD_IS("buy")) {
        shopping_buy(argument, ch, keeper, shop_nr);
        return (true);
    } else if (CMD_IS("sell")) {
        shopping_sell(argument, ch, keeper, shop_nr);
        return (true);
    } else if (CMD_IS("value")) {
        shopping_value(argument, ch, keeper, shop_nr);
        return (true);
    } else if (CMD_IS("list")) {
        shopping_list(argument, ch, keeper, shop_nr);
        return (true);
    } else if (CMD_IS("appraise")) {
        shopping_app(argument, ch, keeper, shop_nr);
        return (true);
    }
    return (false);
}

int ok_damage_shopkeeper(struct char_data *ch, struct char_data *victim) {
    shop_vnum sindex;

    if (!IS_MOB(victim) || mob_index[GET_MOB_RNUM(victim)].func != shop_keeper)
        return (true);

    /* Prevent "invincible" shopkeepers if they're charmed. */
    if (AFF_FLAGGED(victim, AFF_CHARM))
        return (true);

    for (auto &sh : shop_index) {
        sindex = sh.first;
        if (GET_MOB_RNUM(victim) == SHOP_KEEPER(sindex) && !SHOP_KILL_CHARS(sindex)) {
            char buf[MAX_INPUT_LENGTH];

            snprintf(buf, sizeof(buf), "%s %s", GET_NAME(ch), MSG_CANT_KILL_KEEPER);
            do_tell(victim, buf, cmd_tell, 0);

            do_action(victim, GET_NAME(ch), cmd_slap, 0);
            return (false);
        }
    }


    return (true);
}

/* val == obj_vnum and obj_rnum (?) */
static int add_to_list(struct shop_buy_data *list, int type, int *len, int *val) {
    if (*val != NOTHING && *val >= 0) { /* necessary after changing to unsigned v/rnums -- Welcor */
        if (*len < MAX_SHOP_OBJ) {
            if (type == LIST_PRODUCE)
                *val = real_object(*val);
            if (*val != NOTHING) {
                BUY_TYPE(list[*len]) = *val;
            } else
                *val = NOTHING;
            return (false);
        } else
            return (true);
    }
    return (false);
}

static int end_read_list(struct shop_buy_data *list, int len, int error) {
    if (error)
        basic_mud_log("SYSERR: Raise MAX_SHOP_OBJ constant in shop.h to %d", len + error);
    return (len);
}


static void read_line(FILE *shop_f, const char *string, void *data) {
    char buf[READ_SIZE];

    if (!get_line(shop_f, buf) || !sscanf(buf, string, data)) {
        basic_mud_log("SYSERR: Error in shop #%d, near '%s' with '%s'", SHOP_NUM(top_shop), buf, string);
        exit(1);
    }
}

static int read_list(FILE *shop_f, struct shop_buy_data *list, int new_format,
                     int max, int type) {
    int count, temp, len = 0, error = 0;

    if (new_format) {
        for (;;) {
            read_line(shop_f, "%d", &temp);
            if (temp < 0)    /* Always "-1" the string. */
                break;
            error += add_to_list(list, type, &len, &temp);
        }
    } else
        for (count = 0; count < max; count++) {
            read_line(shop_f, "%d", &temp);
            error += add_to_list(list, type, &len, &temp);
        }
    return (end_read_list(list, len, error));
}

static char *read_shop_message(int mnum, room_vnum shr, FILE *shop_f, const char *why) {
    int cht, ss = 0, ds = 0, err = 0;
    char *tbuf;

    if (!(tbuf = fread_string(shop_f, why)))
        return (nullptr);

    for (cht = 0; tbuf[cht]; cht++) {
        if (tbuf[cht] != '%')
            continue;

        if (tbuf[cht + 1] == 's')
            ss++;
        else if (tbuf[cht + 1] == 'd' && (mnum == 5 || mnum == 6)) {
            if (ss == 0) {
                basic_mud_log("SYSERR: Shop #%d has %%d before %%s, message #%d.", shr, mnum);
                err++;
            }
            ds++;
        } else if (tbuf[cht + 1] != '%') {
            basic_mud_log("SYSERR: Shop #%d has invalid format '%%%c' in message #%d.", shr, tbuf[cht + 1], mnum);
            err++;
        }
    }

    if (ss > 1 || ds > 1) {
        basic_mud_log("SYSERR: Shop #%d has too many specifiers for message #%d. %%s=%d %%d=%d", shr, mnum, ss, ds);
        err++;
    }

    if (err) {
        free(tbuf);
        return (nullptr);
    }
    return (tbuf);
}

void boot_the_shops(FILE *shop_f, char *filename, int rec_count) {
    char *buf, buf2[256], *p;
    shop_vnum temp, count, new_format = false;
    int shop_temp;
    struct shop_buy_data list[MAX_SHOP_OBJ + 1]{};
    int done = false;

    snprintf(buf2, sizeof(buf2), "beginning of shop file %s", filename);

    while (!done) {
        buf = fread_string(shop_f, buf2);
        if (*buf == '#') {        /* New shop */
            sscanf(buf, "#%ld\n", &temp);
            snprintf(buf2, sizeof(buf2)-1, "shop #%ld in shop file %s", temp, filename);
            auto &sh = shop_index[temp];
            free(buf);        /* Plug memory leak! */
            sh.vnum = temp;
            auto &z = zone_table[real_zone_by_thing(sh.vnum)];
            z.shops.insert(sh.vnum);
            top_shop = temp;
            while(true) {
                read_line(shop_f, "%ld", &shop_temp);
                if(shop_temp == -1) break;
                temp = (shop_vnum)shop_temp;
                if(obj_index.count(temp)) sh.producing.push_back(temp);
            }

            read_line(shop_f, "%f", &SHOP_BUYPROFIT(top_shop));
            read_line(shop_f, "%f", &SHOP_SELLPROFIT(top_shop));

            while(true) {
                read_line(shop_f, "%ld", &shop_temp);
                if(shop_temp == -1) break;
                auto &t = sh.type.emplace_back();
                t.type = shop_temp;
            }

            sh.no_such_item1 = read_shop_message(0, SHOP_NUM(top_shop), shop_f, buf2);
            sh.no_such_item2 = read_shop_message(1, SHOP_NUM(top_shop), shop_f, buf2);
            sh.do_not_buy = read_shop_message(2, SHOP_NUM(top_shop), shop_f, buf2);
            sh.missing_cash1 = read_shop_message(3, SHOP_NUM(top_shop), shop_f, buf2);
            sh.missing_cash2 = read_shop_message(4, SHOP_NUM(top_shop), shop_f, buf2);
            sh.message_buy = read_shop_message(5, SHOP_NUM(top_shop), shop_f, buf2);
            sh.message_sell = read_shop_message(6, SHOP_NUM(top_shop), shop_f, buf2);
            read_line(shop_f, "%d", &SHOP_BROKE_TEMPER(top_shop));
            read_line(shop_f, "%ld", &SHOP_BITVECTOR(top_shop));
            read_line(shop_f, "%hd", &SHOP_KEEPER(top_shop));

            SHOP_KEEPER(top_shop) = real_mobile(SHOP_KEEPER(top_shop));
            CREATE(buf, char, READ_SIZE);
            get_line(shop_f, buf);
            p = buf;
            for (temp = 0; temp < SW_ARRAY_MAX; temp++) {
                if (!p || !*p)
                    break;
                if (sscanf(p, "%d", &count) != 1) {
                    basic_mud_log("SYSERR: Can't parse TRADE_WITH line in %s: '%s'", buf2, buf);
                    break;
                }
                SHOP_TRADE_WITH(top_shop)[temp] = count;
                while (isdigit(*p) || *p == '-') {
                    p++;
                }
                while (*p && !(isdigit(*p) || *p == '-')) {
                    p++;
                }
            }
            free(buf);
            while (temp < SW_ARRAY_MAX)
                SHOP_TRADE_WITH(top_shop)[temp++] = 0;

            while(true) {
                read_line(shop_f, "%ld", &shop_temp);
                if(shop_temp == -1) break;
                if(world.contains(shop_temp)) sh.in_room.insert(shop_temp);

            }


            read_line(shop_f, "%d", &SHOP_OPEN1(top_shop));
            read_line(shop_f, "%d", &SHOP_CLOSE1(top_shop));
            read_line(shop_f, "%d", &SHOP_OPEN2(top_shop));
            read_line(shop_f, "%d", &SHOP_CLOSE2(top_shop));

            SHOP_BANK(top_shop) = 0;
            SHOP_SORT(top_shop) = 0;
            SHOP_FUNC(top_shop) = nullptr;
        } else {
            if (*buf == '$')        /* EOF */
                done = true;
            else if (strstr(buf, VERSION3_TAG))    /* New format marker */
                new_format = true;
            free(buf);        /* Plug memory leak! */
        }
    }
}

void assign_the_shopkeepers() {
    cmd_say = find_command("say");
    cmd_tell = find_command("tell");
    cmd_emote = find_command("emote");
    cmd_slap = find_command("slap");
    cmd_puke = find_command("puke");

    for (auto &[vn, sh] : shop_index) {
        if (sh.keeper == NOBODY)
            continue;

        /* Having SHOP_FUNC() as 'shop_keeper' will cause infinite recursion. */
        if (mob_index[sh.keeper].func && mob_index[sh.keeper].func != shop_keeper)
            sh.func = mob_index[sh.keeper].func;

        mob_index[sh.keeper].func = shop_keeper;
    }
}

static char *customer_string(vnum shop_nr, int detailed) {
    int sindex = 0, flag = 0, nlen;
    size_t len = 0;
    static char buf[256];

    while (*trade_letters[sindex] != '\n' && len + 1 < sizeof(buf)) {
        if (detailed) {
            if (!IS_SET_AR(SHOP_TRADE_WITH(shop_nr), flag)) {
                nlen = snprintf(buf + len, sizeof(buf) - len, ", %s", trade_letters[sindex]);

                if (len + nlen >= sizeof(buf) || nlen < 0)
                    break;

                len += nlen;
            }
        } else {
            buf[len++] = (IS_SET_AR(SHOP_TRADE_WITH(shop_nr), flag) ? '_' : *trade_letters[sindex]);
            buf[len] = '\0';

            if (len >= sizeof(buf))
                break;
        }

        sindex++;
        flag += 1;
    }

    buf[sizeof(buf) - 1] = '\0';
    return (buf);
}

/* END_OF inefficient */
static void list_all_shops(struct char_data *ch) {
    const char *list_all_shops_header =
            " ##   Virtual   Where    Keeper    Buy   Sell   Customers\r\n"
            "---------------------------------------------------------\r\n";
    vnum shop_nr, headerlen = strlen(list_all_shops_header);
    size_t len = 0;
    char buf[MAX_STRING_LENGTH], buf1[16];

    *buf = '\0';
    for (auto &[shop_nr, sh] : shop_index) {
        if(!(len < sizeof(buf))) break;
        /* New page in page_string() mechanism, print the header again. */
        if (!(shop_nr % (PAGE_LENGTH - 2))) {
            /*
             * If we don't have enough room for the header, or all we have room left
             * for is the header, then don't add it and just quit now.
             */
            if (len + headerlen + 1 >= sizeof(buf))
                break;
            strcpy(buf + len, list_all_shops_header);    /* strcpy: OK (length checked above) */
            len += headerlen;
        }

        if (sh.keeper == NOBODY) {
            strcpy(buf1, "<NONE>");    /* strcpy: OK (for 'buf1 >= 7') */
        } else {
            sprintf(buf1, "%6d", sh.keeper);    /* sprintf: OK (for 'buf1 >= 11', 32-bit int) */
        }

        auto first = sh.in_room.begin();
        auto in_room = NOWHERE;
        if(first != sh.in_room.end()) in_room = *first;

        len += snprintf(buf + len, sizeof(buf) - len,
                        "%3d   %6d   %6d    %s   %3.2f   %3.2f    %s\r\n",
                        shop_nr + 1, SHOP_NUM(shop_nr), in_room, buf1,
                        SHOP_SELLPROFIT(shop_nr), SHOP_BUYPROFIT(shop_nr),
                        customer_string(shop_nr, false));
    }

    write_to_output(ch->desc, buf);
}

static void list_detailed_shop(struct char_data *ch, vnum shop_nr) {
    struct char_data *k;
    int sindex, column;
    char *ptrsave;

    send_to_char(ch, "Vnum:       [%5d], Rnum: [%5d]\r\n", SHOP_NUM(shop_nr), shop_nr + 1);


    send_to_char(ch, "Rooms:      ");
    column = 12;    /* ^^^ strlen ^^^ */
    auto &sh = shop_index[shop_nr];
	for(auto r : sh.in_room) {
        char buf1[128];
        int linelen, temp;

        if (sindex) {
            send_to_char(ch, ", ");
            column += 2;
        }
        if(world.contains(r)) {
            linelen = snprintf(buf1, sizeof(buf1), "%s (#%d)", world[r].name, r);
        } else {
            linelen = snprintf(buf1, sizeof(buf1), "<UNKNOWN> (#%d)", r);
        }
        /* Implementing word-wrapping: assumes screen-size == 80 */
        if (linelen + column >= 78 && column >= 20) {
            send_to_char(ch, "\r\n            ");
            /* 12 is to line up with "Rooms:" printed first, and spaces above. */
            column = 12;
        }

        if (!send_to_char(ch, "%s", buf1))
            return;
        column += linelen;
    }
    if (sh.in_room.empty())
        send_to_char(ch, "Rooms:      None!");

    send_to_char(ch, "\r\nShopkeeper: ");
    if (sh.keeper != NOBODY) {
        send_to_char(ch, "%s (#%d), Special Function: %s\r\n",
                     GET_NAME(&mob_proto[SHOP_KEEPER(shop_nr)]),
                     sh.keeper,
                     YESNO(SHOP_FUNC(shop_nr)));

        if ((k = get_char_num(sh.keeper)))
            send_to_char(ch, "Coins:      [%9d], Bank: [%9d] (Total: %d)\r\n",
                         GET_GOLD(k), SHOP_BANK(shop_nr), GET_GOLD(k) + SHOP_BANK(shop_nr));
    } else
        send_to_char(ch, "<NONE>\r\n");


    send_to_char(ch, "Customers:  %s\r\n", (ptrsave = customer_string(shop_nr, true)) ? ptrsave : "None");


    send_to_char(ch, "Produces:   ");
    column = 12;    /* ^^^ strlen ^^^ */
    sindex = 0;
    for (auto &p : sh.producing) {
        if(!obj_proto.contains(p)) continue;
        char buf1[128];
        int linelen;

        if (sindex) {
            send_to_char(ch, ", ");
            column += 2;
        }
        linelen = snprintf(buf1, sizeof(buf1), "%s (#%d)",
                           obj_proto[p].short_description,
                           p);

        /* Implementing word-wrapping: assumes screen-size == 80 */
        if (linelen + column >= 78 && column >= 20) {
            send_to_char(ch, "\r\n            ");
            /* 12 is to line up with "Produces:" printed first, and spaces above. */
            column = 12;
        }

        if (!send_to_char(ch, "%s", buf1))
            return;
        column += linelen;
    }
    if (!sindex)
        send_to_char(ch, "Produces:   Nothing!");

    send_to_char(ch, "\r\nBuys:       ");
    column = 12;    /* ^^^ strlen ^^^ */
    sindex = 0;
    for (auto &t : sh.type) {
        char buf1[128];
        size_t linelen;

        if (sindex) {
            send_to_char(ch, ", ");
            column += 2;
        }

        linelen = snprintf(buf1, sizeof(buf1), "%s (#%d) [%s]",
                           item_types[t.type],
                           t.type,
                           !t.keywords.empty() ? t.keywords.c_str() : "all");

        /* Implementing word-wrapping: assumes screen-size == 80 */
        if (linelen + column >= 78 && column >= 20) {
            send_to_char(ch, "\r\n            ");
            /* 12 is to line up with "Buys:" printed first, and spaces above. */
            column = 12;
        }

        if (!send_to_char(ch, "%s", buf1))
            return;
        column += linelen;
        sindex++;
    }
    if (!sindex)
        send_to_char(ch, "Buys:       Nothing!");

    send_to_char(ch, "\r\nBuy at:     [%4.2f], Sell at: [%4.2f], Open: [%d-%d, %d-%d]\r\n",
                 SHOP_SELLPROFIT(shop_nr), SHOP_BUYPROFIT(shop_nr), SHOP_OPEN1(shop_nr),
                 SHOP_CLOSE1(shop_nr), SHOP_OPEN2(shop_nr), SHOP_CLOSE2(shop_nr));


    /* Need a local buffer. */
    {
        char buf1[128];
        sprintbit(SHOP_BITVECTOR(shop_nr), shop_bits, buf1, sizeof(buf1));
        send_to_char(ch, "Bits:       %s\r\n", buf1);
    }
}

void show_shops(struct char_data *ch, char *arg) {
    vnum shop_nr = NOTHING;

    if (!*arg)
        list_all_shops(ch);
    else {
        if (!strcasecmp(arg, ".")) {
            for (auto &sh : shop_index) {
                if (ok_shop_room(sh.first, GET_ROOM_VNUM(IN_ROOM(ch)))) {
                    shop_nr = sh.first;
                    break;
                }
            }

            if (shop_nr == NOTHING) {
                send_to_char(ch, "This isn't a shop!\r\n");
                return;
            }
        } else if (is_number(arg))
            shop_nr = atol(arg);
        else
            shop_nr = NOTHING;

        if (!shop_index.count(shop_nr)) {
            send_to_char(ch, "Illegal shop number.\r\n");
            return;
        }
        list_detailed_shop(ch, shop_nr);
    }
}

void destroy_shops() {
    shop_index.clear();
}


shop_data::~shop_data() {
    free_shop_strings(this);
}

void shop_data::add_product(obj_vnum v) {
    producing.push_back(v);
}

void shop_data::remove_product(obj_vnum v) {
    std::remove_if(producing.begin(), producing.end(), [&](obj_vnum &o) {return o == v;});
}

nlohmann::json shop_buy_data::serialize() {
    nlohmann::json j;

    if(type) j["type"] = type;
    if(!keywords.empty()) j["keywords"] = keywords;

    return j;
}

shop_buy_data::shop_buy_data(const nlohmann::json &j) : shop_buy_data() {
    if(j.contains("type")) type = j["type"];
    if(j.contains("keywords")) keywords = j["keywords"];
}

nlohmann::json shop_data::serialize() {
    nlohmann::json j;

    j["vnum"] = vnum;
    for(auto i : producing) j["producing"].push_back(i);
    if(profit_buy) j["profit_buy"] = profit_buy;
    if(profit_sell) j["profit_sell"] = profit_sell;
    for(auto &t : type) j["type"].push_back(t.serialize());
    if(no_such_item1 && strlen(no_such_item1)) j["no_such_item1"] = no_such_item1;
    if(no_such_item2 && strlen(no_such_item2)) j["no_such_item2"] = no_such_item2;
    if(missing_cash1 && strlen(missing_cash1)) j["missing_cash1"] = missing_cash1;
    if(missing_cash2 && strlen(missing_cash2)) j["missing_cash2"] = missing_cash2;
    if(do_not_buy && strlen(do_not_buy)) j["do_not_buy"] = do_not_buy;
    if(message_buy && strlen(message_buy)) j["message_buy"] = message_buy;
    if(message_sell && strlen(message_sell)) j["message_sell"] = message_sell;
    if(temper1) j["temper1"] = temper1;
    if(bitvector) j["bitvector"] = bitvector;
    if(keeper != NOBODY) j["keeper"] = keeper;
    for(auto i = 0; i < 79; i++) if(IS_SET_AR(with_who, i)) j["with_who"].push_back(i);
    for(auto r : in_room) j["in_room"].push_back(r);
    if(open1) j["open1"] = open1;
    if(close1) j["close1"] = close1;
    if(open2) j["open2"] = open2;
    if(close2) j["close2"] = close2;
    if(bankAccount) j["bankAccount"] = bankAccount;
    if(lastsort) j["lastsort"] = lastsort;

    return j;
}

shop_data::shop_data(const nlohmann::json &j) : shop_data() {
    if(j.contains("vnum")) vnum = j["vnum"];
    if(j.contains("producing")) for(const auto& i : j["producing"]) producing.emplace_back(i);
    if(j.contains("profit_buy")) profit_buy = j["profit_buy"];
    if(j.contains("profit_sell")) profit_sell = j["profit_sell"];
    if(j.contains("type")) for(const auto& i : j["type"]) type.emplace_back(i);
    if(j.contains("no_such_item1")) no_such_item1 = strdup(j["no_such_item1"].get<std::string>().c_str());
    if(j.contains("no_such_item2")) no_such_item2 = strdup(j["no_such_item2"].get<std::string>().c_str());
    if(j.contains("missing_cash1")) missing_cash1 = strdup(j["missing_cash1"].get<std::string>().c_str());
    if(j.contains("missing_cash2")) missing_cash2 = strdup(j["missing_cash2"].get<std::string>().c_str());
    if(j.contains("do_not_buy")) do_not_buy = strdup(j["do_not_buy"].get<std::string>().c_str());
    if(j.contains("message_buy")) message_buy = strdup(j["message_buy"].get<std::string>().c_str());
    if(j.contains("message_sell")) message_sell = strdup(j["message_sell"].get<std::string>().c_str());
    if(j.contains("temper1")) temper1 = j["temper1"];
    if(j.contains("bitvector")) bitvector = j["bitvector"];
    if(j.contains("keeper")) keeper = j["keeper"];
    if(j.contains("with_who")) for(const auto& i : j["with_who"]) SET_BIT_AR(with_who, i.get<int>());
    if(j.contains("in_room")) for(const auto& i : j["in_room"]) in_room.insert(i.get<int>());
    if(j.contains("open1")) open1 = j["open1"];
    if(j.contains("close1")) close1 = j["close1"];
    if(j.contains("open2")) open2 = j["open2"];
    if(j.contains("close2")) close2 = j["close2"];
    if(j.contains("bankAccount")) bankAccount = j["bankAccount"];
    if(j.contains("lastsort")) lastsort = j["lastsort"];
}