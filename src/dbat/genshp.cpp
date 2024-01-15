/************************************************************************
 * Generic OLC Library - Shops / genshp.c			v1.0	*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/genshp.h"
#include "dbat/utils.h"
#include "dbat/db.h"

#include "dbat/genolc.h"
#include "dbat/genshp.h"
#include "dbat/genzon.h"

/*
 * NOTE (gg): Didn't modify sedit much. Don't consider it as 'recent'
 * 	as the other editors with regard to updates or style.
 */

/*-------------------------------------------------------------------*/

void copy_shop(struct shop_data *tshop, struct shop_data *fshop, int free_old_strings) {
    int i;

    /*
     * Copy basic information over.
     */
    S_NUM(tshop) = S_NUM(fshop);
    S_KEEPER(tshop) = S_KEEPER(fshop);
    S_OPEN1(tshop) = S_OPEN1(fshop);
    S_CLOSE1(tshop) = S_CLOSE1(fshop);
    S_OPEN2(tshop) = S_OPEN2(fshop);
    S_CLOSE2(tshop) = S_CLOSE2(fshop);
    S_BANK(tshop) = S_BANK(fshop);
    S_BROKE_TEMPER(tshop) = S_BROKE_TEMPER(fshop);
    S_BITVECTOR(tshop) = S_BITVECTOR(fshop);
    for (i = 0; i < SW_ARRAY_MAX; i++)
        S_NOTRADE(tshop)[i] = S_NOTRADE(fshop)[i];
    S_SORT(tshop) = S_SORT(fshop);
    S_BUYPROFIT(tshop) = S_BUYPROFIT(fshop);
    S_SELLPROFIT(tshop) = S_SELLPROFIT(fshop);
    S_FUNC(tshop) = S_FUNC(fshop);

    /*
     * Copy lists over.
     */
    tshop->in_room = fshop->in_room;
    tshop->producing = fshop->producing;
    tshop->type = fshop->type;

    /*
     * Copy notification strings over.
     */
    if (free_old_strings)
        free_shop_strings(tshop);
    S_NOITEM1(tshop) = str_udup(S_NOITEM1(fshop));
    S_NOITEM2(tshop) = str_udup(S_NOITEM2(fshop));
    S_NOCASH1(tshop) = str_udup(S_NOCASH1(fshop));
    S_NOCASH2(tshop) = str_udup(S_NOCASH2(fshop));
    S_NOBUY(tshop) = str_udup(S_NOBUY(fshop));
    S_BUY(tshop) = str_udup(S_BUY(fshop));
    S_SELL(tshop) = str_udup(S_SELL(fshop));
}

/*-------------------------------------------------------------------*/

/*
 * Copy a 'NOTHING' terminated integer array list.
 */
void copy_list(IDXTYPE **tlist, IDXTYPE *flist) {
    int num_items, i;

    if (*tlist)
        free(*tlist);

    /*
     * Count number of entries.
     */
    for (i = 0; flist[i] != NOTHING; i++);
    num_items = i + 1;

    /*
     * Make space for entries.
     */
    CREATE(*tlist, IDXTYPE, num_items);

    /*
     * Copy entries over.
     */
    for (i = 0; i < num_items; i++)
        (*tlist)[i] = flist[i];
}

/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/


/*-------------------------------------------------------------------*/

void add_to_type_list(struct shop_buy_data **list, struct shop_buy_data *newl) {
    int i, num_items;
    struct shop_buy_data *nlist;

    /*
     * Count number of entries.
     */
    for (i = 0; (*list)[i].type != NOTHING; i++);
    num_items = i;

    /*
     * Make a new list and slot in the new entry.
     */
    CREATE(nlist, struct shop_buy_data, num_items + 2);

    for (i = 0; i < num_items; i++)
        nlist[i] = (*list)[i];
    nlist[num_items] = *newl;
    nlist[num_items + 1].type = NOTHING;

    /*
     * Out with the old, in with the new.
     */
    free(*list);
    *list = nlist;
}

/*-------------------------------------------------------------------*/

void add_to_int_list(IDXTYPE **list, IDXTYPE newi) {
    IDXTYPE i, num_items, *nlist;

    /*
     * Count number of entries.
     */
    for (i = 0; (*list)[i] != NOTHING; i++);
    num_items = i;

    /*
     * Make a new list and slot in the new entry.
     */
    CREATE(nlist, IDXTYPE, num_items + 2);

    for (i = 0; i < num_items; i++)
        nlist[i] = (*list)[i];
    nlist[num_items] = newi;
    nlist[num_items + 1] = NOTHING;

    /*
     * Out with the old, in with the new.
     */
    free(*list);
    *list = nlist;
}

/*-------------------------------------------------------------------*/

void remove_from_int_list(IDXTYPE **list, IDXTYPE num) {
    IDXTYPE i, num_items, *nlist;

    /*
     * Count number of entries.
     */
    for (i = 0; (*list)[i] != NOTHING; i++);

#if CIRCLE_UNSIGNED_INDEX
    if (num >= i)
#else
        if (num < 0 || num >= i)
#endif
        return;
    num_items = i;

    CREATE(nlist, IDXTYPE, num_items);

    for (i = 0; i < num_items; i++)
        nlist[i] = (i < num) ? (*list)[i] : (*list)[i + 1];

    free(*list);
    *list = nlist;
}

/*-------------------------------------------------------------------*/

/*
 * Free all the notice character strings in a shop structure.
 */
void free_shop_strings(struct shop_data *shop) {
    if (S_NOITEM1(shop)) {
        free(S_NOITEM1(shop));
        S_NOITEM1(shop) = nullptr;
    }
    if (S_NOITEM2(shop)) {
        free(S_NOITEM2(shop));
        S_NOITEM2(shop) = nullptr;
    }
    if (S_NOCASH1(shop)) {
        free(S_NOCASH1(shop));
        S_NOCASH1(shop) = nullptr;
    }
    if (S_NOCASH2(shop)) {
        free(S_NOCASH2(shop));
        S_NOCASH2(shop) = nullptr;
    }
    if (S_NOBUY(shop)) {
        free(S_NOBUY(shop));
        S_NOBUY(shop) = nullptr;
    }
    if (S_BUY(shop)) {
        free(S_BUY(shop));
        S_BUY(shop) = nullptr;
    }
    if (S_SELL(shop)) {
        free(S_SELL(shop));
        S_SELL(shop) = nullptr;
    }
}

/*-------------------------------------------------------------------*/


/*-------------------------------------------------------------------*/

/*
 * Free up the whole shop structure and it's content.
 */
void free_shop(struct shop_data *shop) {
    free_shop_strings(shop);

    delete shop;
}

/*-------------------------------------------------------------------*/

/* returns the real number of the shop with given virtual number 
 *
 * We take so good care to keep it sorted - let's use it :) - Welcor
 */
shop_rnum real_shop(shop_vnum vnum) {
    return shop_index.count(vnum) ? vnum : NOTHING;
}

/*-------------------------------------------------------------------*/

/*
 * Generic string modifier for shop keeper messages.
 */
void modify_string(char **str, char *new_s) {

    char buf[MAX_STRING_LENGTH];
    char *pointer;

    /*
     * Check the '%s' is present, if not, add it.
     */
    if (*new_s != '%') {
        snprintf(buf, sizeof(buf), "%%s %s", new_s);
        pointer = buf;
    } else
        pointer = new_s;

    if (*str)
        free(*str);
    *str = strdup(pointer);
}

/*-------------------------------------------------------------------*/

int add_shop(struct shop_data *nshp) {
    shop_rnum rshop;
    zone_rnum rznum = real_zone_by_thing(S_NUM(nshp));
    auto &z = zone_table[rznum];
    z.shops.insert(S_NUM(nshp));
    auto &sh = shop_index[S_NUM(nshp)];
    copy_shop(&shop_index[rshop], nshp, false);
    dirty_shops.insert(S_NUM(nshp));
    return S_NUM(nshp);
}

/*-------------------------------------------------------------------*/

int save_shops(zone_rnum zone_num) {
    auto &z = zone_table[zone_num];
    dirty_shops.insert(z.shops.begin(), z.shops.end());
    z.save_shops();
    return true;
}
