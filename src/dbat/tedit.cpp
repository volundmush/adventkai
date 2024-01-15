/*
 * Originally written by: Michael Scott -- Manx.
 * Last known e-mail address: scottm@workcomm.net
 *
 * XXX: This needs Oasis-ifying.
 */

#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/interpreter.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/genolc.h"
#include "dbat/oasis.h"
#include "dbat/improved-edit.h"
#include "dbat/tedit.h"
#include "dbat/config.h"

void news_string_cleanup(struct descriptor_data *d, int terminator) {
    FILE *fl;
    char *storage = NEWS_FILE;

    if (!storage)
        terminator = STRINGADD_ABORT;

    switch (terminator) {
        case STRINGADD_SAVE:
            if (!(fl = fopen(storage, "a")))
                mudlog(CMP, ADMLVL_IMPL, true, "SYSERR: Can't write file '%s'.", storage);
            if (!*d->str)
                mudlog(CMP, ADMLVL_IMPL, true, "SYSERR: Can't write file '%s'.", storage);
            else {
                char *tmstr;
                time_t mytime = time(nullptr);
                tmstr = (char *) asctime(localtime(&mytime));
                *(tmstr + strlen(tmstr) - 1) = '\0';

                fprintf(fl, "#%d %s\n@cUpdated By@D: @C%-13s @cDate@D: @Y%s@n\n", TOP_OF_NEWS, d->newsbuf,
                        GET_NAME(d->character), tmstr);
                free(d->newsbuf);
                d->newsbuf = nullptr;
                strip_cr(*d->str);
                fprintf(fl, "%s\n", *d->str);
                *d->str = nullptr;
                fclose(fl);

                NEWSUPDATE = time(nullptr);
                save_mud_time(&time_info);
                struct descriptor_data *i;

                for (i = descriptor_list; i; i = i->next) {
                    if (!IS_PLAYING(i))
                        continue;
                    if (PLR_FLAGGED(i->character, PLR_WRITING))
                        continue;
                    if (NEWSUPDATE > GET_LPLAY(i->character))
                        send_to_char(i->character,
                                     "\r\n@GA news entry has been made by %s, type 'news %d' to see it.@n\r\n",
                                     GET_NAME(d->character), TOP_OF_NEWS);
                }
                do_reboot(d->character, "news", 0, 0);
            }
            break;
        case STRINGADD_ABORT:
            write_to_output(d, "Edit aborted.\r\n");
            act("$n stops editing the news.", true, d->character, nullptr, nullptr, TO_ROOM);
            break;
        default:
            basic_mud_log("SYSERR: news_string_cleanup: Unknown terminator status.");
            break;
    }

    /* Common cleanup code. */
    STATE(d) = CON_PLAYING;
}

void tedit_string_cleanup(struct descriptor_data *d, int terminator) {
    FILE *fl;
    char *storage = OLC_STORAGE(d);

    if (!storage)
        terminator = STRINGADD_ABORT;

    switch (terminator) {
        case STRINGADD_SAVE:
            if (!(fl = fopen(storage, "w")))
                mudlog(CMP, ADMLVL_IMPL, true, "SYSERR: Can't write file '%s'.", storage);
            else {
                if (*d->str && !(strcmp(storage, "text/news"))) {
                    char *tmstr;
                    time_t mytime = time(nullptr);
                    tmstr = (char *) asctime(localtime(&mytime));
                    *(tmstr + strlen(tmstr) - 1) = '\0';

                    strip_cr(*d->str);
                    fprintf(fl,
                            "\n-----------------------------------------------\n@Y%s @cUpdated By@D: @C%s@n\r\n-----------------------------------------------\n%s\n",
                            tmstr, GET_NAME(d->character), *d->str);
                } else if (*d->str) {
                    strip_cr(*d->str);
                    fputs(*d->str, fl);
                }
                fclose(fl);
                mudlog(CMP, ADMLVL_GOD, true, "OLC: %s saves '%s'.", GET_NAME(d->character), storage);
                write_to_output(d, "Saved.\r\n");

                if (!(strcmp(storage, "text/news"))) {
                    NEWSUPDATE = time(nullptr);
                    save_mud_time(&time_info);
                    struct descriptor_data *i;

                    for (i = descriptor_list; i; i = i->next) {
                        if (!IS_PLAYING(i))
                            continue;
                        if (PLR_FLAGGED(i->character, PLR_WRITING))
                            continue;
                        if (NEWSUPDATE > GET_LPLAY(i->character))
                            send_to_char(i->character,
                                         "\r\n@GThe NEWS file has been updated, type 'news' to see it.@n\r\n");
                    }
                    do_reboot(d->character, "all", 0, 0);
                }
            }
            break;
        case STRINGADD_ABORT:
            write_to_output(d, "Edit aborted.\r\n");
            act("$n stops editing some scrolls.", true, d->character, nullptr, nullptr, TO_ROOM);
            break;
        default:
            basic_mud_log("SYSERR: tedit_string_cleanup: Unknown terminator status.");
            break;
    }

    /* Common cleanup code. */
    cleanup_olc(d, CLEANUP_ALL);
    STATE(d) = CON_PLAYING;
}

ACMD(do_tedit) {
    int l, i = 0;
    char field[MAX_INPUT_LENGTH];
    char *backstr = nullptr;

    struct {
        char *cmd;
        char level;
        char **buffer;
        int size;
        char *filename;
    } fields[] = {
            /* edit the lvls to your own needs */
            {"credits",    ADMLVL_IMPL,  &credits,    24000, CREDITS_FILE},
            {"donottouch", 6,            &news,       24000, NEWS_FILE},
            {"motd",       ADMLVL_IMPL,  &motd,       24000, MOTD_FILE},
            {"imotd",      ADMLVL_IMPL,  &imotd,      24000, IMOTD_FILE},
            {"help",       ADMLVL_GRGOD, &help,       24000, HELP_PAGE_FILE},
            {"info",       ADMLVL_GRGOD, &info,       24000, INFO_FILE},
            {"background", ADMLVL_IMPL,  &background, 24000, BACKGROUND_FILE},
            {"handbook",   ADMLVL_IMPL,  &handbook,   24000, HANDBOOK_FILE},
            {"update",     ADMLVL_IMPL,  &policies,   24000, POLICIES_FILE},
            {"ihelp",      ADMLVL_GRGOD, &ihelp,      24000, IHELP_PAGE_FILE},
            {"\n",         0,            nullptr,     0, nullptr}
    };

    if (ch->desc == nullptr)
        return;

    one_argument(argument, field);

    if (!*field) {
        send_to_char(ch, "Files available to be edited:\r\n");
        for (l = 0; *fields[l].cmd != '\n'; l++) {
            if (GET_ADMLEVEL(ch) >= fields[l].level) {
                send_to_char(ch, "%-11.11s ", fields[l].cmd);
                if (!(++i % 7))
                    send_to_char(ch, "\r\n");
            }
        }
        if (i % 7)
            send_to_char(ch, "\r\n");
        if (i == 0)
            send_to_char(ch, "None.\r\n");
        return;
    }
    for (l = 0; *(fields[l].cmd) != '\n'; l++)
        if (!strncmp(field, fields[l].cmd, strlen(field)))
            break;

    if (*fields[l].cmd == '\n') {
        send_to_char(ch, "Invalid text editor option.\r\n");
        return;
    }

    if (GET_ADMLEVEL(ch) < fields[l].level) {
        send_to_char(ch, "You are not godly enough for that!\r\n");
        return;
    }

    /* set up editor stats */
    clear_screen(ch->desc);
    send_editor_help(ch->desc);
    send_to_char(ch, "Edit file below:\r\n\r\n");

    if (ch->desc->olc) {
        mudlog(BRF, ADMLVL_IMMORT, true, "SYSERR: do_tedit: Player already had olc structure.");
        free(ch->desc->olc);
    }
    CREATE(ch->desc->olc, struct oasis_olc_data, 1);

    if (*fields[l].buffer) {
        send_to_char(ch, "%s", *fields[l].buffer);
        backstr = strdup(*fields[l].buffer);
    }

    OLC_STORAGE(ch->desc) = strdup(fields[l].filename);
    string_write(ch->desc, fields[l].buffer, fields[l].size, 0, backstr);

    act("$n begins editing a text file.", true, ch, nullptr, nullptr, TO_ROOM);
    ch->playerFlags.set(PLR_WRITING);
    STATE(ch->desc) = CON_TEDIT;
}
