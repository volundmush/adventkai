/* ************************************************************************
*   File: ban.c                                         Part of CircleMUD *
*  Usage: banning/unbanning/checking sites and player names               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <fstream>
#include "dbat/ban.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/handler.h"
#include "dbat/db.h"

/* local globals */
struct ban_list_element *ban_list = nullptr;

/* local functions */
static void _write_one_node(FILE *fp, struct ban_list_element *node);

static void write_ban_list();


static const char *ban_types[] = {
        "no",
        "new",
        "select",
        "all",
        "ERROR"
};


void load_banned() {
    FILE *fl;
    int i, date;
    char site_name[BANNED_SITE_LENGTH + 1], ban_type[100];
    char name[MAX_NAME_LENGTH + 1];
    struct ban_list_element *next_node;

    ban_list = nullptr;

    if (!(fl = fopen(BAN_FILE, "r"))) {
        if (errno != ENOENT) {
            basic_mud_log("SYSERR: Unable to open banfile '%s': %s", BAN_FILE, strerror(errno));
        } else
            basic_mud_log("   Ban file '%s' doesn't exist.", BAN_FILE);
        return;
    }
    while (fscanf(fl, " %s %s %d %s ", ban_type, site_name, &date, name) == 4) {
        CREATE(next_node, struct ban_list_element, 1);
        strncpy(next_node->site, site_name, BANNED_SITE_LENGTH);    /* strncpy: OK (n_n->site:BANNED_SITE_LENGTH+1) */
        next_node->site[BANNED_SITE_LENGTH] = '\0';
        strncpy(next_node->name, name, MAX_NAME_LENGTH);    /* strncpy: OK (n_n->name:MAX_NAME_LENGTH+1) */
        next_node->name[MAX_NAME_LENGTH] = '\0';
        next_node->date = date;

        for (i = BAN_NOT; i <= BAN_ALL; i++)
            if (!strcmp(ban_type, ban_types[i]))
                next_node->type = i;

        next_node->next = ban_list;
        ban_list = next_node;
    }

    fclose(fl);
}


int isbanned(char *hostname) {
    int i;
    struct ban_list_element *banned_node;
    char *nextchar;

    if (!hostname || !*hostname)
        return (0);

    i = 0;
    for (nextchar = hostname; *nextchar; nextchar++)
        *nextchar = LOWER(*nextchar);

    for (banned_node = ban_list; banned_node; banned_node = banned_node->next)
        if (strstr(hostname, banned_node->site))    /* if hostname is a substring */
            i = MAX(i, banned_node->type);

    return (i);
}


static void _write_one_node(FILE *fp, struct ban_list_element *node) {
    if (node) {
        _write_one_node(fp, node->next);
        fprintf(fp, "%s %s %ld %s\n", ban_types[node->type],
                node->site, (long) node->date, node->name);
    }
}

static void write_ban_list() {
    FILE *fl;

    if (!(fl = fopen(BAN_FILE, "w"))) {
        perror("SYSERR: Unable to open '" BAN_FILE "' for writing");
        return;
    }
    _write_one_node(fl, ban_list);/* recursively write from end to start */
    fclose(fl);
    return;
}

#define BAN_LIST_FORMAT "%-40.40s  %-8.8s  %-10.10s  %-16.16s\r\n"

ACMD(do_ban) {
    char flag[MAX_INPUT_LENGTH], site[MAX_INPUT_LENGTH], *nextchar;
    char timestr[16];
    int i;
    struct ban_list_element *ban_node;

    if (!*argument) {
        if (!ban_list) {
            send_to_char(ch, "No sites are banned.\r\n");
            return;
        }
        send_to_char(ch, BAN_LIST_FORMAT,
                     "Banned Site Name",
                     "Ban Type",
                     "Banned On",
                     "Banned By");
        send_to_char(ch, BAN_LIST_FORMAT,
                     "---------------------------------",
                     "---------------------------------",
                     "---------------------------------",
                     "---------------------------------");

        for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
            if (ban_node->date) {
                strlcpy(timestr, asctime(localtime(&(ban_node->date))), 10);
                timestr[10] = '\0';
            } else
                strcpy(timestr, "Unknown");    /* strcpy: OK (strlen("Unknown") < 16) */

            send_to_char(ch, BAN_LIST_FORMAT, ban_node->site, ban_types[ban_node->type], timestr, ban_node->name);
        }
        return;
    }

    two_arguments(argument, flag, site);
    if (!*site || !*flag) {
        send_to_char(ch, "Usage: ban {all | select | new} site_name\r\n");
        return;
    }
    if (!(!strcasecmp(flag, "select") || !strcasecmp(flag, "all") || !strcasecmp(flag, "new"))) {
        send_to_char(ch, "Flag must be ALL, SELECT, or NEW.\r\n");
        return;
    }
    for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
        if (!strcasecmp(ban_node->site, site)) {
            send_to_char(ch, "That site has already been banned -- unban it to change the ban type.\r\n");
            return;
        }
    }

    CREATE(ban_node, struct ban_list_element, 1);
    strncpy(ban_node->site, site, BANNED_SITE_LENGTH);    /* strncpy: OK (b_n->site:BANNED_SITE_LENGTH+1) */
    for (nextchar = ban_node->site; *nextchar; nextchar++)
        *nextchar = LOWER(*nextchar);
    ban_node->site[BANNED_SITE_LENGTH] = '\0';
    strncpy(ban_node->name, GET_NAME(ch), MAX_NAME_LENGTH);    /* strncpy: OK (b_n->size:MAX_NAME_LENGTH+1) */
    ban_node->name[MAX_NAME_LENGTH] = '\0';
    ban_node->date = time(nullptr);

    for (i = BAN_NEW; i <= BAN_ALL; i++)
        if (!strcasecmp(flag, ban_types[i]))
            ban_node->type = i;

    ban_node->next = ban_list;
    ban_list = ban_node;

    mudlog(NRM, MAX(ADMLVL_GOD, GET_INVIS_LEV(ch)), true, "%s has banned %s for %s players.",
           GET_NAME(ch), site, ban_types[ban_node->type]);
    send_to_char(ch, "Site banned.\r\n");
    write_ban_list();
}

#undef BAN_LIST_FORMAT


ACMD(do_unban) {
    char site[MAX_INPUT_LENGTH];
    struct ban_list_element *ban_node, *temp;
    int found = 0;

    one_argument(argument, site);
    if (!*site) {
        send_to_char(ch, "A site to unban might help.\r\n");
        return;
    }
    ban_node = ban_list;
    while (ban_node && !found) {
        if (!strcasecmp(ban_node->site, site))
            found = 1;
        else
            ban_node = ban_node->next;
    }

    if (!found) {
        send_to_char(ch, "That site is not currently banned.\r\n");
        return;
    }
    REMOVE_FROM_LIST(ban_node, ban_list, next, temp);
    send_to_char(ch, "Site unbanned.\r\n");
    mudlog(NRM, MAX(ADMLVL_GOD, GET_INVIS_LEV(ch)), true, "%s removed the %s-player ban on %s.",
           GET_NAME(ch), ban_types[ban_node->type], ban_node->site);

    free(ban_node);
    write_ban_list();
}


/**************************************************************************
 *  Code to check for invalid names (i.e., profanity, etc.)		  *
 *  Written by Sharon P. Goza						  *
 **************************************************************************/

std::set<std::string> invalid_list;

/* What's with the wacky capitalization in here? */
void Free_Invalid_List() {
    invalid_list.clear();
}

void Read_Invalid_List() {
    // this function will attempt to open the "misc/xnames" file from cwd and read each
    // non-empty line into invalid_list (a std::set<std::string>)
    // use std::filesystem!
    std::filesystem::path path = "misc/xnames";

    // Check if the file exists
    if (!std::filesystem::exists(path))
    {
        logger->error("SYSERR: Unable to open '" XNAME_FILE "' for reading");
        return;
    }

    std::ifstream file(path);

    // Check if the file was opened successfully
    if (!file)
    {
        logger->error("SYSERR: Unable to open '" XNAME_FILE "' for reading");
        return;
    }

    std::string line;
    while(std::getline(file, line))
    {
        boost::trim(line);
        if(!line.empty())
        {
            invalid_list.insert(line);
        }
    }

}
