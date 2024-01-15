/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "dbat/act.comm.h"
#include "dbat/dg_comm.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/spells.h"
#include "dbat/interpreter.h"
#include "dbat/commands.h"
#include "dbat/db.h"
#include "dbat/config.h"
#include "dbat/act.wizard.h"
#include "dbat/act.informative.h"
#include "dbat/handler.h"
#include "dbat/dg_scripts.h"
#include "dbat/boards.h"
#include "dbat/improved-edit.h"
#include "dbat/class.h"

/* local functions */
static void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);

static int is_tell_ok(struct char_data *ch, struct char_data *vict);

static void handle_whisper(char *buf, struct char_data *ch, struct char_data *vict);

static char *overhear(char *buf, int type);

static void list_languages(struct char_data *ch);

static void garble_text(char *string, int known, int lang);


static const char *languages[] =
        {
                "common",
                "elven",
                "gnomish",
                "dwarven",
                "halfling",
                "orc",
                "druid",
                "draconic",
                "\n"
        };

static void list_languages(struct char_data *ch) {
    int a = 0, i;

    send_to_char(ch, "Languages:\r\n[");
    for (i = MIN_LANGUAGES; i <= MAX_LANGUAGES; i++)
        if (GET_SKILL(ch, i))
            send_to_char(ch, "%s %s%s%s",
                         a++ != 0 ? "," : "",
                         SPEAKING(ch) == i ? "@r" : "@n",
                         languages[i - MIN_LANGUAGES], "@n");
    send_to_char(ch, "%s ]\r\n", a == 0 ? " None!" : "");
}

ACMD(do_voice) {
    skip_spaces(&argument);

    if (IS_NPC(ch))
        return;


    if (GET_BONUS(ch, BONUS_MUTE) > 0) {
        send_to_char(ch, "You're mute. You don't need to describe your voice.\r\n");
        return;
    }

    if (!*argument) {
        send_to_char(ch, "What are you changing your voice description to?\r\n");
        return;
    } else if (strlen(argument) > 75) {
        send_to_char(ch, "Your voice description can not be longer than 75 characters.\r\n");
        return;
    } else if (strstr(argument, "@")) {
        send_to_char(ch, "You can not use colorcode in voice descriptions.\r\n");
        return;
    } else if (GET_VOICE(ch) != nullptr && GET_RP(ch) < 1) {
        send_to_char(ch, "Your voice has already been set. You will need at least 1 RPP to be able to change it.\r\n");
        return;
    } else if (GET_VOICE(ch) != nullptr) {
        send_to_char(ch, "Your voice has now been set to: %s\r\n", argument);
        if (GET_VOICE(ch)) {
            free(GET_VOICE(ch));
        }
        GET_VOICE(ch) = strdup(argument);
        ch->modRPP(-1);
        send_to_char(ch, "@D(@cRPP@W: @w-1@D)@n\n\n");
        return;
    } else {
        send_to_char(ch, "Your voice has now been set to: %s\r\n", argument);
        if (GET_VOICE(ch)) {
            free(GET_VOICE(ch));
        }
        GET_VOICE(ch) = strdup(argument);
        return;
    }

}

ACMD(do_languages) {
    int i, found = false;
    char arg[MAX_STRING_LENGTH];

    if (CONFIG_ENABLE_LANGUAGES) {
        one_argument(argument, arg);
        if (!*arg)
            list_languages(ch);
        else {
            for (i = MIN_LANGUAGES; i <= MAX_LANGUAGES; i++) {
                if ((search_block(arg, languages, false) == i - MIN_LANGUAGES) && GET_SKILL(ch, i)) {
                    SPEAKING(ch) = i;
                    send_to_char(ch, "You now speak %s.\r\n", languages[i - MIN_LANGUAGES]);
                    found = true;
                    break;
                }
            }
            if (!found) {
                send_to_char(ch, "You do not know of any such language.\r\n");
                return;
            }
        }
    } else {
        send_to_char(ch, "But everyone already understands everyone else!\r\n");
        return;
    }

}

static void garble_text(char *string, int known, int lang) {
    char letters[50] = "";
    int i;

    switch (lang) {
        case SKILL_LANG_DWARVEN:
            strcpy(letters, "hprstwxyz");
            break;
        case SKILL_LANG_ELVEN:
            strcpy(letters, "aefhilnopstu");
            break;
        default:
            strcpy(letters, "aehiopstuwxyz");
            break;
    }

    for (i = 0; i < (int) strlen(string); ++i) {
        if (isalpha(string[i]) && (!known)) {
            string[i] = letters[rand_number(0, (int) strlen(letters) - 1)];
        }
    }
}

ACMD(do_osay) {

    skip_spaces(&argument);

    if (IS_NPC(ch))
        return;

    if (!*argument) {
        send_to_char(ch, "Yes, but WHAT do you want to osay?\r\n");
        return;
    } else {
        char buf[MAX_INPUT_LENGTH];
        char buf2[MAX_INPUT_LENGTH];

        sprintf(buf, "@WYou @D[@mOSAY@D] @W'@w%s@W'@n", argument);
        if (!PRF_FLAGGED(ch, PRF_HIDE)) {
            sprintf(buf2, "@W%s @D[@mOSAY@D] @W'@w%s@W'@n", GET_ADMLEVEL(ch) > 0 ? GET_NAME(ch) : GET_USER(ch),
                    argument);
        }
        if (PRF_FLAGGED(ch, PRF_HIDE)) {
            sprintf(buf2, "@WAnonymous @D[@mOSAY@D] @W'@w%s@W'@n", argument);
        }
        act(buf, false, ch, nullptr, nullptr, TO_CHAR);
        act(buf2, false, ch, nullptr, nullptr, TO_ROOM);
    }
}

ACMD(do_say) {
    struct descriptor_data *d;
    struct char_data *wch = nullptr, *wch2 = nullptr, *wch3 = nullptr, *tch = nullptr, *sch = nullptr;
    struct obj_data *obj = nullptr;
    int granted = false, found = false;
    char buf2[MAX_INPUT_LENGTH];
    *buf2 = '\0';

    skip_spaces(&argument);

    if (GET_BONUS(ch, BONUS_MUTE) > 0 && GET_ROOM_VNUM(IN_ROOM(ch)) > 160) {
        send_to_char(ch, "You are mute and unable to talk though.\r\n");
        return;
    } else if (GET_BONUS(ch, BONUS_MUTE) > 0) {
        send_to_char(ch, "You are mute and unable to talk though. You will be allowed to just for MUD School.");
    }

    if (!*argument) {
        send_to_char(ch, "Yes, but WHAT do you want to say?\r\n");
        return;
    } else {
        char buf[MAX_INPUT_LENGTH + 70];
        char verb[10];

        if (argument[strlen(argument) - 1] == '!') {
            strcpy(verb, "exclaim");
        } else if (argument[strlen(argument) - 1] == '?') {
            strcpy(verb, "ask");
        } else {
            strcpy(verb, "say");
        }

        for (tch = ch->getRoom()->people; tch; tch = tch->next_in_room) {
            if (tch != ch && tch->desc) {
                char sayto[100];
                sprintf(sayto, "to %s ", GET_NAME(tch));
                if (strstr(argument, sayto)) {
                    char saytoo[200];
                    *verb = '\0';
                    sprintf(saytoo, "says to @g%s@W", GET_NAME(tch));
                    search_replace(argument, sayto, "");
                    strcpy(verb, saytoo);
                    sch = tch;
                } else if (!IS_NPC(tch) && !IS_NPC(ch)) {
                    if (readIntro(ch, tch) == 1) {
                        sprintf(sayto, "to %s ", get_i_name(ch, tch));
                    }
                    if (strstr(argument, sayto)) {
                        char saytoo[200];
                        *verb = '\0';
                        sprintf(saytoo, "says to @g%s@W", GET_NAME(tch));
                        search_replace(argument, sayto, "");
                        strcpy(verb, saytoo);
                        sch = tch;
                    }
                }
            }
        }
        if (!sch) {
            snprintf(buf, sizeof(buf), "@w$n @W%ss, '@C%s@W'@n", verb, argument);
            act(buf, true, ch, nullptr, nullptr, TO_ROOM);
        } else {
            snprintf(buf, sizeof(buf), "@w$n @Wsays to @g$N@W, '@C%s@W'@n", argument);
            snprintf(buf2, sizeof(buf2), "@w$n @Wsays to @gyou@W, '@C%s@W'@n", argument);
            act(buf2, true, ch, nullptr, sch, TO_VICT);
            act(buf, true, ch, nullptr, sch, TO_NOTVICT);
        }
        if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT)) {
            send_to_char(ch, "%s", CONFIG_OK);
        } else {
            if (strstr(verb, "says to")) {
                char saytoo[200];
                *verb = '\0';
                sprintf(saytoo, "say to @g%s@W", GET_NAME(sch));
                strcpy(verb, saytoo);
            }
            snprintf(buf, sizeof(buf), "@WYou %s, '@C%s@W'@n\r\n", verb, argument);
            send_to_char(ch, "%s", buf);
            add_history(ch, buf, HIST_SAY);
            if (SHENRON == true) {
                if (GET_ROOM_VNUM(IN_ROOM(ch)) == DRAGONR && GET_ROOM_VNUM(IN_ROOM(EDRAGON)) == DRAGONR) {
                    if (strstr(argument, "wish")) {

                        for (d = descriptor_list; d; d = d->next) {
                            if (STATE(d) != CON_PLAYING)
                                continue;

                            if (strstr(argument, GET_NAME(d->character)) && wch == nullptr) {
                                wch = d->character;
                                found = true;
                            } else if (strstr(argument, GET_NAME(d->character)) && wch2 == nullptr) {
                                wch2 = d->character;
                            } else if (strstr(argument, GET_NAME(d->character)) && wch3 == nullptr) {
                                wch3 = d->character;
                            }
                        } /* end repeat for */

                        if (wch == nullptr && strstr(argument, "myself")) {
                            wch = ch;
                        }
                        if (wch == nullptr) {
                            return;
                        }

                        if (granted == false && strstr(argument, "knowledge")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s now has more knowledge!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->modPractices(rand_number(2000, 5000));
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a knowledge wish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));;
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end knowledge wish if */

                        if (granted == false && strstr(argument, "speed")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now faster!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharAttribute::Speed, 10);
                                wch->save();
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a speed wish on %s.", GET_NAME(ch),
                                       GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end speed wish if */

                        if (granted == false && strstr(argument, "tough")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now tougher!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharNum::ArmorWishes, 1);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a tough wish on %s.", GET_NAME(ch),
                                       GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end tough wish if */

                        if (granted == false && strstr(argument, "strength")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s has more strength!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharAttribute::Strength, 10);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a strength wish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end strength wish if */

                        if (granted == false && strstr(argument, "intelligence")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now smarter!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharAttribute::Intelligence, 10);
                                wch->save();
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a intelligence wish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end intelligence wish if */

                        if (granted == false && strstr(argument, "wisdom")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now wiser!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharAttribute::Wisdom, 10);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a wisdom wish on %s.", GET_NAME(ch),
                                       GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end wisdom wish if */

                        if (granted == false && strstr(argument, "agility")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now more agile!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharAttribute::Agility, 10);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a agility wish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end agility wish if */

                        if (granted == false && strstr(argument, "constitution")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s has more guts!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharAttribute::Constitution, 10);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a constitutionwish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end constitution wish if */

                        if (granted == false && strstr(argument, "skill")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s has more skill!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                int roll = rand_number(1, 3);
                                send_to_char(wch, "@GYou suddenly feel like you could learn %d more skills!@n\r\n",
                                             roll);
                                GET_SLOTS(wch) += roll;
                                wch->save();
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a skill wish on %s.", GET_NAME(ch),
                                       GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end skill wish if */
/* Rillao: transloc, add new transes here */
                        if (granted == false && strstr(argument, "power")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish cannot be granted, You might want to try something else instead, mortal!@w'@n\r\n");
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end power wish if */

                        if (granted == false && strstr(argument, "money")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s now has become richer!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->mod(CharMoney::Carried, 1000000);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a money wish on %s.", GET_NAME(ch),
                                       GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end money wish if */

                        if (granted == false && strstr(argument, "immunity")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s now has immunity to Burn, Freezing, Mind Break, Poison, Blindness, Yoikominminken, and Paralysis!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->affected_by.set(AFF_IMMUNITY);
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a immunity wish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end money wish if */

                        if (granted == false && strstr(argument, "vitality")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish cannot be granted, You might want to try something else instead, mortal!%s@w'@n\r\n");
                                /*send_to_room(real_room(DRAGONR), "@wShenron says, '@CYour wish has been granted, %s now will never hunger or thirst again!%s@w'@n\r\n", GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                GET_COND(ch, HUNGER) = -1;
                                GET_COND(ch, THIRST) = -1;
                                granted = TRUE;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, TRUE, "Shenron: %s has made a vitality wish on %s.", GET_NAME(ch), GET_NAME(wch));*/
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end vitality if */

                        if (granted == false && strstr(argument, "revive")) {
                            int count = 0;
                            if (wch != nullptr) {
                                count += 1;
                            }
                            if (wch2 != nullptr) {
                                count += 1;
                            }
                            if (wch3 != nullptr) {
                                count += 1;
                            }
                            if (count == 1) {
                                if (!AFF_FLAGGED(wch, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@C%s is not dead, and can not be revived.@w'@n\r\n",
                                                 GET_NAME(wch));
                                } else {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@CYour wish has been granted, %s has returned to life!%s@w'@n\r\n",
                                                 GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                    if (real_room(GET_DROOM(wch)) == NOWHERE) {
                                        GET_DROOM(wch) = 300;
                                    }
                                    if (real_room(GET_DROOM(wch)) != NOWHERE) {
                                        char_from_room(wch);
                                        if (GET_DROOM(wch) > 0) {
                                            char_to_room(wch, real_room(GET_DROOM(wch)));
                                        } else {
                                            char_to_room(wch, real_room(300));
                                        }
                                        look_at_room(IN_ROOM(wch), wch, 0);
                                        send_to_char(wch,
                                                     "@wYou smile as the golden halo above your head disappears! You have returned to life where you had last died!@n\r\n");
                                        wch->affected_by.reset(AFF_SPIRIT);
                                        wch->affected_by.reset(AFF_ETHEREAL);
                                    }
                                    granted = true;
                                    SELFISHMETER -= 2;
                                    mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a revive wish on %s.",
                                           GET_NAME(ch), GET_NAME(wch));
                                }
                            } /* is there a target for the wish? */
                            if (count == 2) {
                                if (!AFF_FLAGGED(wch, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@C%s is not dead, and can not be revived.@w'@n\r\n",
                                                 GET_NAME(wch));
                                }
                                if (!AFF_FLAGGED(wch2, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@C%s is not dead, and can not be revived.@w'@n\r\n",
                                                 GET_NAME(wch2));
                                } else if (AFF_FLAGGED(wch, AFF_SPIRIT) && AFF_FLAGGED(wch2, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@CYour wish has been granted, %s and %s have returned to life!%s@w'@n\r\n",
                                                 GET_NAME(wch), GET_NAME(wch2),
                                                 WISH[0] ? "" : " Now make your second wish.");
                                    if (real_room(GET_DROOM(wch)) == NOWHERE) {
                                        GET_DROOM(wch) = 300;
                                    }
                                    if (real_room(GET_DROOM(wch)) != NOWHERE) {
                                        char_from_room(wch);
                                        char_to_room(wch, real_room(GET_DROOM(wch)));
                                        look_at_room(IN_ROOM(wch), wch, 0);
                                        send_to_char(wch,
                                                     "@wYou smile as the golden halo above your head disappears! You have returned to life where you had last died!@n\r\n");
                                        for(auto f : {AFF_SPIRIT, AFF_ETHEREAL}) wch->affected_by.reset(f);
                                    }
                                    if (real_room(GET_DROOM(wch2)) == NOWHERE) {
                                        GET_DROOM(wch2) = 300;
                                    }
                                    if (real_room(GET_DROOM(wch2)) != NOWHERE) {
                                        char_from_room(wch2);
                                        char_to_room(wch2, real_room(GET_DROOM(wch2)));
                                        look_at_room(IN_ROOM(wch2), wch2, 0);
                                        send_to_char(wch2,
                                                     "@wYou smile as the golden halo above your head disappears! You have returned to life where you had last died!@n\r\n");
                                        for(auto f : {AFF_SPIRIT, AFF_ETHEREAL}) wch2->affected_by.reset(f);
                                    }
                                    granted = true;
                                    SELFISHMETER -= 3;
                                    mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a revive wish on %s.",
                                           GET_NAME(ch), GET_NAME(wch2));
                                    WAIT_STATE(ch, PULSE_4SEC);
                                }
                            } /* is there two targets for the wish? */
                            if (count == 3) {
                                if (!AFF_FLAGGED(wch, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@C%s is not dead, and can not be revived.@w'@n\r\n",
                                                 GET_NAME(wch));
                                }
                                if (!AFF_FLAGGED(wch2, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@C%s is not dead, and can not be revived.@w'@n\r\n",
                                                 GET_NAME(wch2));
                                }
                                if (!AFF_FLAGGED(wch3, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@C%s is not dead, and can not be revived.@w'@n\r\n",
                                                 GET_NAME(wch3));
                                } else if (AFF_FLAGGED(wch, AFF_SPIRIT) && AFF_FLAGGED(wch2, AFF_SPIRIT) &&
                                           AFF_FLAGGED(wch3, AFF_SPIRIT)) {
                                    send_to_room(real_room(DRAGONR),
                                                 "@wShenron says, '@CYour wish has been granted, %s, %s, and %s have returned to life!!%s@w'@n\r\n",
                                                 GET_NAME(wch), GET_NAME(wch2), GET_NAME(wch3),
                                                 WISH[0] ? "" : " Now make your second wish.");
                                    if (real_room(GET_DROOM(wch)) == NOWHERE) {
                                        GET_DROOM(wch) = 300;
                                    }
                                    if (real_room(GET_DROOM(wch)) != NOWHERE) {
                                        char_from_room(wch);
                                        char_to_room(wch, real_room(GET_DROOM(wch)));
                                        look_at_room(IN_ROOM(wch), wch, 0);
                                        send_to_char(wch,
                                                     "@wYou smile as the golden halo above your head disappears! You have returned to life where you had last died!@n\r\n");
                                        for(auto f : {AFF_SPIRIT, AFF_ETHEREAL}) wch->affected_by.reset(f);
                                    }
                                    if (real_room(GET_DROOM(wch2)) == NOWHERE) {
                                        GET_DROOM(wch2) = 300;
                                    }
                                    if (real_room(GET_DROOM(wch2)) != NOWHERE) {
                                        char_from_room(wch2);
                                        char_to_room(wch2, real_room(GET_DROOM(wch2)));
                                        look_at_room(IN_ROOM(wch2), wch2, 0);
                                        send_to_char(wch2,
                                                     "@wYou smile as the golden halo above your head disappears! You have returned to life where you had last died!@n\r\n");
                                        for(auto f : {AFF_SPIRIT, AFF_ETHEREAL}) wch2->affected_by.reset(f);
                                    }
                                    if (real_room(GET_DROOM(wch3)) == NOWHERE) {
                                        GET_DROOM(wch3) = 300;
                                    }
                                    if (real_room(GET_DROOM(wch3)) != NOWHERE) {
                                        char_from_room(wch3);
                                        char_to_room(wch3, real_room(GET_DROOM(wch3)));
                                        look_at_room(IN_ROOM(wch3), wch3, 0);
                                        send_to_char(wch3,
                                                     "@wYou smile as the golden halo above your head disappears! You have returned to life where you had last died!@n\r\n");
                                        for(auto f : {AFF_SPIRIT, AFF_ETHEREAL}) wch3->affected_by.reset(f);
                                    }
                                    granted = true;
                                    SELFISHMETER -= 3;
                                    mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a revive wish on %s and %s.",
                                           GET_NAME(ch), GET_NAME(wch2), GET_NAME(wch3));
                                    WAIT_STATE(ch, PULSE_4SEC);
                                }
                            } /* is there three targets for the wish? */
                        } /* end revival if */

                        if (granted == false && strstr(argument, "immortal") && WISH[0] == 0) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now immortal!@w'@n\r\n",
                                             GET_NAME(wch));
                                wch->playerFlags.set(PLR_IMMORTAL);
                                WISH[0] = 1;
                                WISH[1] = 1;
                                granted = true;
                                SELFISHMETER += 4;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a immortal wish on %s.",
                                       GET_NAME(ch), GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end immortal wish if */

                        if (granted == false && strstr(argument, "immortal") && WISH[0] == 1) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CI can not grant that wish, there is not enough remaining power in this summoning!@w'@n\r\n");
                            } /* is there a target for the wish? */
                        }

                        if (granted == false && strstr(argument, " mortal")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s is now mortal!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                wch->playerFlags.reset(PLR_IMMORTAL);
                                granted = true;
                                SELFISHMETER += 4;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a mortal wish on %s.", GET_NAME(ch),
                                       GET_NAME(wch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            } /* is there a target for the wish? */
                        } /* end mortal wish if */

                        if (granted == false && strstr(argument, "senzu")) {
                            if (wch != nullptr) {
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                obj = read_object(1, VIRTUAL);
                                obj_to_char(obj, ch);
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s now possesses 10 senzus!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                granted = true;
                                SELFISHMETER += 1;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a senzu wish.", GET_NAME(ch));
                            } /* is there a target for the wish? */
                        } /* end senzu wish if */

                        if (granted == false && strstr(argument, "roleplay")) {
                            if (wch != nullptr) {
                                send_to_room(real_room(DRAGONR),
                                             "@wShenron says, '@CYour wish has been granted, %s!%s@w'@n\r\n",
                                             GET_NAME(wch), WISH[0] ? "" : " Now make your second wish.");
                                granted = true;
                                mudlog(NRM, ADMLVL_GOD, true, "Shenron: %s has made a roleplay wish.", GET_NAME(ch));
                                WAIT_STATE(ch, PULSE_4SEC);
                            }
                        }

                        if (granted == true) {
                            if (WISH[0] == 1) {
                                WISH[1] = 1;
                            } else {
                                WISH[0] = 1;
                            } /*end WISH if */
                            if (wch != nullptr) {
                                wch->save();
                            }
                            if (wch2 != nullptr) {
                                wch->save();
                            }
                            if (wch3 != nullptr) {
                                wch->save();
                            }
                            save_mud_time(&time_info);
                        } else if (wch == nullptr) {
                            send_to_room(real_room(DRAGONR),
                                         "@wShenron says, '@CThat person does not exist, make another wish.'@n\r\n");
                        } else {
                            send_to_room(real_room(DRAGONR),
                                         "@wShenron says, '@CDo not waste my time with wishes I can not grant...@w'@n\r\n");
                        }
                    }
                } /* end DRAGONR if */
            } /* end SHENRON if */

        }
    }

    /* trigger check */
    speech_mtrigger(ch, argument);
    speech_wtrigger(ch, argument);
    if (SHENRON == false || (SHENRON == true && IN_ROOM(ch) != real_room(DRAGONR))) {
        mob_talk(ch, argument);
    }
}

ACMD(do_gsay) {
    struct char_data *k;
    struct follow_type *f;
    char blah[MAX_INPUT_LENGTH];

    skip_spaces(&argument);

    if (!AFF_FLAGGED(ch, AFF_GROUP)) {
        send_to_char(ch, "But you are not the member of a group!\r\n");
        return;
    }
    if (IN_ARENA(ch)) {
        send_to_char(ch, "Lol, no.\r\n");
        return;
    }
    if (!*argument)
        send_to_char(ch, "Yes, but WHAT do you want to group-say?\r\n");
    else {
        char buf[MAX_STRING_LENGTH];

        if (ch->master)
            k = ch->master;
        else
            k = ch;

        strcpy(buf, argument);

        sprintf(blah, "$n@W tells the group @W'@G%s@W'@n\r\n", buf);

        if (AFF_FLAGGED(k, AFF_GROUP) && (k != ch) && AWAKE(k)) {
            if (CONFIG_ENABLE_LANGUAGES) {
                send_to_char(k, "%s@W tells the group%s @W'@G%s@W'@n\r\n", CAN_SEE(k, ch) ? GET_NAME(ch) : "Someone",
                             GET_SKILL(k, SPEAKING(ch)) ? "," : ", in an unfamiliar tongue,", buf);
            } else {
                act(blah, true, ch, nullptr, k, TO_VICT);
            }
        }
        for (f = k->followers; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_GROUP) && (f->follower != ch) && AWAKE(f->follower)) {
                if (!IS_NPC(ch) && !IS_NPC(f->follower) && CONFIG_ENABLE_LANGUAGES) {
                    garble_text(buf, GET_SKILL(f->follower, SPEAKING(ch)), SPEAKING(ch));
                } else {
                    garble_text(buf, 1, MIN_LANGUAGES);
                }
                if (CONFIG_ENABLE_LANGUAGES) {
                    send_to_char(f->follower, "%s@W tells the group%s @W'%s@W'@n\r\n",
                                 CAN_SEE(f->follower, ch) ? GET_NAME(ch) : "Someone",
                                 GET_SKILL(f->follower, SPEAKING(ch)) ? "," : ", in an unfamiliar tongue,", buf);
                } else {
                    act(blah, true, ch, nullptr, f->follower, TO_VICT | TO_SLEEP);
                }
            }

        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(ch, "%s", CONFIG_OK);
        else
            send_to_char(ch, "@WYou tell the group, '@G%s@W'@n\r\n", argument);
    }
}

static void perform_tell(struct char_data *ch, struct char_data *vict, char *arg) {
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    strcpy(buf, arg);

    if (CONFIG_ENABLE_LANGUAGES) {
        snprintf(buf2, sizeof(buf2), "@[13]%s tells you%s '%s@[13]'@n\r\n",
                 CAN_SEE(vict, ch) ? GET_NAME(ch) : "Someone",
                 GET_SKILL(vict, SPEAKING(ch)) ? "," : ", in an unfamiliar tongue,", buf);
        send_to_char(vict, "%s", buf2);
        add_history(vict, buf2, HIST_TELL);
    } else if (!IS_NPC(ch) && GET_ADMLEVEL(vict) < 1) {
        snprintf(buf2, sizeof(buf2), "@Y%s@Y tells you '%s'@n\r\n",
                 GET_ADMLEVEL(ch) > 0 ? GET_NAME(ch) : GET_USER(ch), buf);
        send_to_char(vict, "%s", buf2);
        add_history(vict, buf2, HIST_TELL);
    } else if (!IS_NPC(ch) && GET_ADMLEVEL(vict) >= 1) {
        snprintf(buf2, sizeof(buf2), "@Y%s(%s)@Y tells you '%s'@n\r\n", GET_USER(ch), GET_NAME(ch), buf);
        send_to_char(vict, "%s", buf2);
        add_history(vict, buf2, HIST_TELL);
    } else if (IS_NPC(ch)) {
        snprintf(buf2, sizeof(buf2), "@Y%s@Y tells you '%s'@n\r\n", GET_NAME(ch), buf);
        send_to_char(vict, "%s", buf2);
    }

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT)) {
        send_to_char(ch, "%s", CONFIG_OK);
    } else {
        if (!IS_NPC(ch)) {
            snprintf(buf2, sizeof(buf2), "@YYou tell %s, '%s'@n\r\n",
                     GET_ADMLEVEL(vict) > 0 ? GET_NAME(vict) : (vict->desc->account ? GET_USER(vict) : "ERROR"), arg);
            if (GET_ADMLEVEL(ch) < 5 && GET_ADMLEVEL(vict) < 5 && !IS_NPC(ch) && !IS_NPC(vict)) {
                send_to_imm("@GTELL: @C%s@G tells @c%s, @W'@w%s@W'@n",
                            GET_ADMLEVEL(ch) > 0 ? GET_NAME(ch) : GET_USER(ch),
                            GET_ADMLEVEL(vict) > 0 ? GET_NAME(vict) : GET_USER(vict), arg);
            }
            send_to_char(ch, "%s", buf2);
            add_history(ch, buf2, HIST_TELL);
        } else {
            send_to_char(ch, "%s", CONFIG_OK);
        }
    }

    if (!IS_NPC(vict) && !IS_NPC(ch))
        GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

static int is_tell_ok(struct char_data *ch, struct char_data *vict) {

    if (ch == vict)
        send_to_char(ch, "You try to tell yourself something.\r\n");
    else if (!IS_NPC(ch) && GET_LEVEL(ch) < 3 && GET_ADMLEVEL(vict) < 1)
        send_to_char(ch, "You need to be level 3 or higher to send or receive tells");
    else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AFK) && GET_ADMLEVEL(ch) < 1)
        send_to_char(ch, "You can't send tells when AFK.\r\n");
    else if (!IS_NPC(ch) && PRF_FLAGGED(vict, PRF_AFK) && GET_ADMLEVEL(ch) < 1)
        send_to_char(ch, "They are AFK right now, try later.\r\n");
    else if (!IS_NPC(ch) && PRF_FLAGGED(vict, PRF_AFK) && GET_ADMLEVEL(ch) >= 1)
        return (true);
    else if (!IS_NPC(vict) && GET_LEVEL(vict) < 3 && GET_ADMLEVEL(ch) < 1)
        send_to_char(ch, "They need to be level 3 or higher to send or receive tells");
    else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOTELL) && GET_ADMLEVEL(vict) < 1)
        send_to_char(ch, "You can't tell other people while you have notell on.\r\n");
    else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SOUNDPROOF) && GET_ADMLEVEL(vict) < 1)
        send_to_char(ch, "The walls seem to absorb your words.\r\n");
    else if (IS_NPC(vict))
        send_to_char(ch, "You can't send tells to mobs.\r\n");
    else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
        act("$E's linkless at the moment.", false, ch, nullptr, vict, TO_CHAR | TO_SLEEP);
    else if (PLR_FLAGGED(vict, PLR_WRITING))
        act("$E's writing a message right now; try again later.", false, ch, nullptr, vict, TO_CHAR | TO_SLEEP);
    else if ((!IS_NPC(vict) && GET_ADMLEVEL(ch) < 1 && PRF_FLAGGED(vict, PRF_NOTELL)) ||
             ROOM_FLAGGED(IN_ROOM(vict), ROOM_SOUNDPROOF))
        act("$E can't hear you.", false, ch, nullptr, vict, TO_CHAR | TO_SLEEP);
    else
        return (true);

    return (false);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell) {
    struct char_data *vict = nullptr;
    char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2) {
        send_to_char(ch, "Who do you wish to tell what??\r\n");
        return;
    }
    struct descriptor_data *k;
    int found = false;
    if (!IS_NPC(ch)) {
        sprintf(buf, "%s", CAP(buf));
        for (k = descriptor_list; k; k = k->next) {
            if (IS_NPC(k->character))
                continue;
            if (STATE(k) != CON_PLAYING)
                continue;
            if (!k->account)
                continue;
            if (found == false && !IS_NPC(ch) && (!strcasecmp(k->account->name.c_str(), buf) || strstr(k->account->name.c_str(), buf))) {
                vict = k->character;
                found = true;
            } else if (!IS_NPC(ch) && found == false &&
                       (!strcasecmp(GET_NAME(k->character), buf) || strstr(GET_NAME(k->character), buf)) &&
                       GET_ADMLEVEL(k->character) > 0) {
                vict = k->character;
                found = true;
            }
        }
    }
    if (found == false && !IS_NPC(ch))
        send_to_char(ch, "No user around with that name.");
    else if (IS_NPC(ch) && !(vict = get_player_vis(ch, buf, nullptr, FIND_CHAR_WORLD)))
        send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (is_tell_ok(ch, vict))
        perform_tell(ch, vict, buf2);

}

ACMD(do_reply) {
    struct char_data *tch = character_list;

    if (IS_NPC(ch))
        return;

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HBTC)) {
        send_to_char(ch, "This is a different dimension!\r\n");
        return;
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PAST)) {
        send_to_char(ch, "This is the past, you can't send tells!\r\n");
        return;
    }

    skip_spaces(&argument);

    if (GET_LAST_TELL(ch) == NOBODY)
        send_to_char(ch, "You have nobody to reply to!\r\n");
    else if (!*argument)
        send_to_char(ch, "What is your reply?\r\n");
    else {
        /*
         * Make sure the person you're replying to is still playing by searching
         * for them.  Note, now last tell is stored as player IDnum instead of
         * a pointer, which is much better because it's safer, plus will still
         * work if someone logs out and back in again.
         */

        /*
         * XXX: A descriptor list based search would be faster although
         *      we could not find link dead people.  Not that they can
         *      hear tells anyway. :) -gg 2/24/98
         */
        while (tch != nullptr && (IS_NPC(tch) || GET_IDNUM(tch) != GET_LAST_TELL(ch)))
            tch = tch->next;

        if (tch == nullptr)
            send_to_char(ch, "They are no longer playing.\r\n");
        else if (is_tell_ok(ch, tch))
            perform_tell(ch, tch, argument);
    }
}

ACMD(do_spec_comm) {
    char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
    struct char_data *vict;
    const char *action_sing, *action_plur, *action_others;

    if (GET_BONUS(ch, BONUS_MUTE) > 0) {
        send_to_char(ch, "You are mute and unable to talk though.\r\n");
        return;
    }

    switch (subcmd) {
        case SCMD_WHISPER:
            action_sing = "whisper to";
            action_plur = "whispers to";
            action_others = "$n whispers something to $N.";
            break;

        case SCMD_ASK:
            action_sing = "ask";
            action_plur = "asks";
            action_others = "$n asks $N a question.";
            break;

        default:
            action_sing = "oops";
            action_plur = "oopses";
            action_others = "$n is tongue-tied trying to speak with $N.";
            break;
    }

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2)
        send_to_char(ch, "Whom do you want to %s.. and what??\r\n", action_sing);
    else if (!(vict = get_char_vis(ch, buf, nullptr, FIND_CHAR_ROOM)))
        send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (vict == ch)
        send_to_char(ch, "You can't get your mouth close enough to your ear...\r\n");
    else {
        char buf1[MAX_STRING_LENGTH];
        char obuf[MAX_STRING_LENGTH];

        if (CONFIG_ENABLE_LANGUAGES) {
            strcpy(obuf, buf2);
            garble_text(obuf, GET_SKILL(vict, SPEAKING(ch)), SPEAKING(ch));
            snprintf(buf1, sizeof(buf1), "$n %s you%s '%s'", action_plur,
                     GET_SKILL(vict, SPEAKING(ch)) ? "," : ", in an unfamiliar tongue,", obuf);
        } else {
            snprintf(buf1, sizeof(buf1), "@c$n @W%s you '@m%s@W'@n", action_plur, buf2);
        }

        act(buf1, false, ch, nullptr, vict, TO_VICT);

        if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT)) {
            send_to_char(ch, "%s", CONFIG_OK);
        } else {
            char blum[MAX_INPUT_LENGTH];
            sprintf(blum, "@WYou %s @C$N@W, '@m%s@W'@n\r\n", action_sing, buf2);
            act(blum, true, ch, nullptr, vict, TO_CHAR);
        }

        if (subcmd == SCMD_WHISPER) {
            act(action_others, false, ch, nullptr, vict, TO_NOTVICT);
            handle_whisper(buf2, ch, vict);
        } else {
            act(action_others, false, ch, nullptr, vict, TO_NOTVICT);
        }
    }
}

static void handle_whisper(char *buf, struct char_data *ch, struct char_data *vict) {
    struct char_data *tch;

    for (tch = ch->getRoom()->people; tch; tch = tch->next_in_room) {
        if (IS_NPC(tch)) {
            continue;
        }
        if (tch == ch) {
            continue;
        }
        if (tch == vict) {
            continue;
        }
        if (!GET_SKILL(tch, SKILL_LISTEN)) {
            continue;
        }
        if (GET_SKILL(tch, SKILL_LISTEN)) {
            int skill = GET_SKILL(tch, SKILL_LISTEN);
            int roll1 = rand_number(10, 30);
            int roll = rand_number(roll1, 110);

            if (skill >= roll) {
                send_to_char(tch, "@WYou overhear everything whispered, @W'@m%s@W'@n\r\n", overhear(buf, 3));
            } else if (skill + 10 >= roll) {
                send_to_char(tch, "@WYou overhear a lot of what is whispered, @W'@m%s@W'@n\r\n", overhear(buf, 2));
            } else if (skill + 20 >= roll) {
                send_to_char(tch, "@WYou overhear some of what is whispered, @W'@m%s@W'@n\r\n", overhear(buf, 1));
            } else if (skill + 30 >= roll) {
                send_to_char(tch, "@WYou overhear little of what is whispered, @W'@m%s@W'@n\r\n", overhear(buf, 0));
            } else {
                send_to_char(tch, "@WYou were unable to overhear anything that was whispered.@n\r\n");
            }
        }
    }

}

static char *overhear(char *buf, int type) {

    switch (type) {
        case 0:
            if (rand_number(1, 10) >= 5) {
                search_replace(buf, "a", "..");
                search_replace(buf, "A", "..");
                search_replace(buf, "h", "..");
                search_replace(buf, "H", "..");
                search_replace(buf, "e", "..");
                search_replace(buf, "E", "..");
                search_replace(buf, "m", "..");
                search_replace(buf, "M", "..");
                search_replace(buf, "o", "..");
                search_replace(buf, "O", "..");
                search_replace(buf, "p", "..");
                search_replace(buf, "P", "..");
                search_replace(buf, "y", "..");
                search_replace(buf, "Y", "..");
                search_replace(buf, "j", "..");
                search_replace(buf, "J", "..");
                search_replace(buf, "k", "..");
                search_replace(buf, "K", "..");
                search_replace(buf, "d", "..");
                search_replace(buf, "D", "..");
                search_replace(buf, "w", "..");
                search_replace(buf, "W", "..");
            } else if (rand_number(1, 10) >= 5) {
                search_replace(buf, "e", "..");
                search_replace(buf, "E", "..");
                search_replace(buf, "r", "..");
                search_replace(buf, "R", "..");
                search_replace(buf, "k", "..");
                search_replace(buf, "K", "..");
                search_replace(buf, "m", "..");
                search_replace(buf, "M", "..");
                search_replace(buf, "o", "..");
                search_replace(buf, "O", "..");
                search_replace(buf, "p", "..");
                search_replace(buf, "P", "..");
                search_replace(buf, "y", "..");
                search_replace(buf, "Y", "..");
                search_replace(buf, "j", "..");
                search_replace(buf, "J", "..");
                search_replace(buf, "k", "..");
                search_replace(buf, "K", "..");
                search_replace(buf, "d", "..");
                search_replace(buf, "D", "..");
                search_replace(buf, "w", "..");
                search_replace(buf, "W", "..");
            } else {
                search_replace(buf, "s", "..");
                search_replace(buf, "S", "..");
                search_replace(buf, "r", "..");
                search_replace(buf, "R", "..");
                search_replace(buf, "c", "..");
                search_replace(buf, "C", "..");
                search_replace(buf, "q", "..");
                search_replace(buf, "Q", "..");
                search_replace(buf, "l", "..");
                search_replace(buf, "L", "..");
                search_replace(buf, "u", "..");
                search_replace(buf, "U", "..");
                search_replace(buf, "i", "..");
                search_replace(buf, "I", "..");
                search_replace(buf, "z", "..");
                search_replace(buf, "Z", "..");
                search_replace(buf, "t", "..");
                search_replace(buf, "T", "..");
            }
            return buf;
            break;
        case 1:
            if (rand_number(1, 10) >= 5) {
                search_replace(buf, "b", "..");
                search_replace(buf, "B", "..");
                search_replace(buf, "f", "..");
                search_replace(buf, "F", "..");
                search_replace(buf, "g", "..");
                search_replace(buf, "G", "..");
                search_replace(buf, "v", "..");
                search_replace(buf, "V", "..");
                search_replace(buf, "j", "..");
                search_replace(buf, "J", "..");
                search_replace(buf, "k", "..");
                search_replace(buf, "K", "..");
            } else if (rand_number(1, 10) >= 5) {
                search_replace(buf, "d", "..");
                search_replace(buf, "D", "..");
                search_replace(buf, "y", "..");
                search_replace(buf, "Y", "..");
                search_replace(buf, "m", "..");
                search_replace(buf, "M", "..");
                search_replace(buf, "h", "..");
                search_replace(buf, "H", "..");
                search_replace(buf, "s", "..");
                search_replace(buf, "S", "..");
                search_replace(buf, "t", "..");
                search_replace(buf, "T", "..");
            } else {
                search_replace(buf, "a", "..");
                search_replace(buf, "A", "..");
                search_replace(buf, "r", "..");
                search_replace(buf, "R", "..");
                search_replace(buf, "n", "..");
                search_replace(buf, "N", "..");
                search_replace(buf, "o", "..");
                search_replace(buf, "O", "..");
            }
            return buf;
            break;
        case 2:
            if (rand_number(1, 10) >= 5) {
                search_replace(buf, "q", "..");
                search_replace(buf, "Q", "..");
                search_replace(buf, "o", "..");
                search_replace(buf, "O", "..");
                search_replace(buf, "i", "..");
                search_replace(buf, "I", "..");
                search_replace(buf, "g", "..");
                search_replace(buf, "G", "..");
            } else if (rand_number(1, 10) >= 5) {
                search_replace(buf, "a", "..");
                search_replace(buf, "A", "..");
                search_replace(buf, "e", "..");
                search_replace(buf, "E", "..");
                search_replace(buf, "i", "..");
                search_replace(buf, "I", "..");
                search_replace(buf, "o", "..");
                search_replace(buf, "O", "..");
            } else {
                search_replace(buf, "k", "..");
                search_replace(buf, "K", "..");
                search_replace(buf, "m", "..");
                search_replace(buf, "M", "..");
                search_replace(buf, "b", "..");
                search_replace(buf, "B", "..");
            }
            return buf;
            break;
        case 3:
            return buf;
            break;
    }

    return ("Nothing");
}

/*
 * buf1, buf2 = MAX_OBJECT_NAME_LENGTH
 *	(if it existed)
 */
ACMD(do_write) {
    struct obj_data *paper, *pen = nullptr;
    char *papername, *penname;
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    auto isBoard = [](const auto& o) {
        return GET_OBJ_TYPE(o) == ITEM_BOARD;
    };

    auto obj = ch->findObject(isBoard);
    if(!obj) ch->getRoom()->findObject(isBoard);


    if (obj) {                /* then there IS a board! */
        write_board_message(GET_OBJ_VNUM(obj), ch, argument);
        act("$n begins to write a note on $p.", true, ch, obj, nullptr, TO_ROOM);
        return;
    }

    papername = buf1;
    penname = buf2;

    two_arguments(argument, papername, penname);

    if (!ch->desc)
        return;

    if (!*papername) {        /* nothing was delivered */
        send_to_char(ch, "write on [what] with [what pen?]\r\n");
        return;
    }
    if (*penname) {        /* there were two arguments */
        if (!(paper = get_obj_in_list_vis(ch, papername, nullptr, ch->contents))) {
            send_to_char(ch, "You have no %s.\r\n", papername);
            return;
        }
        if (!(pen = get_obj_in_list_vis(ch, penname, nullptr, ch->contents))) {
            send_to_char(ch, "You have no %s.\r\n", penname);
            return;
        }
    } else {        /* there was one arg.. let's see what we can find */
        if (!(paper = get_obj_in_list_vis(ch, papername, nullptr, ch->contents))) {
            send_to_char(ch, "There is no %s in your inventory.\r\n", papername);
            return;
        }
        if (GET_OBJ_TYPE(paper) == ITEM_PEN) {    /* oops, a pen.. */
            pen = paper;
            paper = nullptr;
        } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
            send_to_char(ch, "That thing has nothing to do with writing.\r\n");
            return;
        }
        /* One object was found.. now for the other one. */
        if (!GET_EQ(ch, WEAR_WIELD2)) {
            send_to_char(ch, "You can't write with %s %s alone.\r\n", AN(papername), papername);
            return;
        }
        if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_WIELD2))) {
            send_to_char(ch, "The stuff in your hand is invisible!  Yeech!!\r\n");
            return;
        }
        if (pen)
            paper = GET_EQ(ch, WEAR_WIELD2);
        else
            pen = GET_EQ(ch, WEAR_WIELD2);
    }


    /* ok.. now let's see what kind of stuff we've found */
    if (GET_OBJ_TYPE(pen) != ITEM_PEN)
        act("$p is no good for writing with.", false, ch, pen, nullptr, TO_CHAR);
    else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
        act("You can't write on $p.", false, ch, paper, nullptr, TO_CHAR);
    else {
        char *backstr = nullptr;

        /* Something on it, display it as that's in input buffer. */
        if (paper->look_description) {
            backstr = strdup(paper->look_description);
            send_to_char(ch, "There's something written on it already:\r\n");
            send_to_char(ch, "%s", paper->look_description);
        }

        /* we can write - hooray! */
        act("$n begins to jot down a note.", true, ch, nullptr, nullptr, TO_ROOM);
        paper->extra_flags.set(ITEM_UNIQUE_SAVE);
        send_editor_help(ch->desc);
        string_write(ch->desc, &paper->look_description, MAX_NOTE_LENGTH, 0, backstr);
    }
}

ACMD(do_page) {
    struct descriptor_data *d;
    struct char_data *vict;
    char buf2[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];

    half_chop(argument, arg, buf2);

    if (IS_NPC(ch))
        send_to_char(ch, "Monsters can't page.. go away.\r\n");
    else if (!*arg)
        send_to_char(ch, "Whom do you wish to page?\r\n");
    else {
        char buf[MAX_STRING_LENGTH];

        snprintf(buf, sizeof(buf), "\007\007*$n* %s", buf2);
        if (!strcasecmp(arg, "all")) {
            if (ADM_FLAGGED(ch, ADM_TELLALL)) {
                for (d = descriptor_list; d; d = d->next)
                    if (STATE(d) == CON_PLAYING && d->character)
                        act(buf, false, ch, nullptr, d->character, TO_VICT);
            } else
                send_to_char(ch, "You will never be godly enough to do that!\r\n");
            return;
        }
        if ((vict = get_char_vis(ch, arg, nullptr, FIND_CHAR_WORLD)) != nullptr) {
            act(buf, false, ch, nullptr, vict, TO_VICT);
            if (PRF_FLAGGED(ch, PRF_NOREPEAT))
                send_to_char(ch, "%s", CONFIG_OK);
            else
                act(buf, false, ch, nullptr, vict, TO_CHAR);
        } else
            send_to_char(ch, "There is no such person in the game!\r\n");
    }
}

/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm) {
    struct descriptor_data *i;
    char color_on[24];
    char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], *msg;

    *buf1 = '\0';
    *buf2 = '\0';

    /* Array of flags which must _not_ be set in order for comm to be heard */
    int channels[] = {
            PRF_NOMUSIC,
            PRF_DEAF,
            PRF_NOGOSS,
            PRF_NOAUCT,
            PRF_NOGRATZ,
            0
    };
    int hist_type[] = {
            HIST_HOLLER,
            HIST_SHOUT,
            HIST_GOSSIP,
            HIST_AUCTION,
            HIST_GRATS,
    };

    /*
     * com_msgs: [0] Message if you can't perform the action because of noshout
     *           [1] name of the action
     *           [2] message if you're not on the channel
     *           [3] a color string.
     */
    const char *com_msgs[][4] = {
            {"You cannot music!!\r\n",
                    "@D[@mMUSIC@D]",
                    "You aren't even on the channel!\r\n",
                    "@[10]"},

            {"You cannot shout!!\r\n",
                    "shout",
                    "Turn off your noshout flag first!\r\n",
                    "@[9]"},

            {"You cannot ooc!!\r\n",
                    "@D[@BOOC@D]",
                    "You aren't even on the channel!\r\n",
                    "@[10]"},

            {"You cannot newbie!!\r\n",
                    "newbie",
                    "You aren't even on the channel!\r\n",
                    "@[11]"},

            {"You cannot congratulate!\r\n",
                    "congrat",
                    "You aren't even on the channel!\r\n",
                    "@[12]"}
    };

    /* to keep pets, etc from being ordered to shout */
    if (!ch->desc)
        return;

    if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
        send_to_char(ch, "%s", com_msgs[subcmd][0]);
        return;
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SOUNDPROOF)) {
        send_to_char(ch, "The walls seem to absorb your words.\r\n");
        return;
    }

    if (subcmd == SCMD_SHOUT && GET_BONUS(ch, BONUS_MUTE) > 0) {
        send_to_char(ch, "You are mute and are incapable of speech.\r\n");
        return;
    }

    /* skip leading spaces */
    skip_spaces(&argument);

    if (subcmd == SCMD_GOSSIP && (*argument == '*')) {
        subcmd = SCMD_GEMOTE;
    }

    if (subcmd == SCMD_GEMOTE) {
        ACMD(do_gmote);
        if (*argument == '*' || *argument == ':')
            do_gmote(ch, argument + 1, 0, 1);
        else
            do_gmote(ch, argument, 0, 1);

        return;
    }

    /* level_can_shout defined in config.c */
    if (GET_LEVEL(ch) < CONFIG_LEVEL_CAN_SHOUT) {
        send_to_char(ch, "You must be at least level %d before you can %s.\r\n", CONFIG_LEVEL_CAN_SHOUT,
                     com_msgs[subcmd][1]);
        return;
    }
    /* make sure the char is on the channel */
    if (!IS_NPC(ch) && PRF_FLAGGED(ch, channels[subcmd])) {
        send_to_char(ch, "%s", com_msgs[subcmd][2]);
        return;
    }

    /* make sure that there is something there to say! */
    if (!*argument) {
        send_to_char(ch, "Yes, %s, fine, %s we must, but WHAT???\r\n", com_msgs[subcmd][1], com_msgs[subcmd][1]);
        return;
    }
    delete_doubledollar(argument);
    /* set up the color on code */
    strlcpy(color_on, com_msgs[subcmd][3], sizeof(color_on));

    /* first, set up strings to be given to the communicator */
    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT)) {
        send_to_char(ch, "%s", CONFIG_OK);
    } else {
        snprintf(buf2, sizeof(buf2), "%s@WYou %s@W, '@w%s@W'%s@n\r\n", color_on, com_msgs[subcmd][1], argument,
                 color_on);
        send_to_char(ch, "%s", buf2);
        add_history(ch, buf2, hist_type[subcmd]);
    }

    /* now send all the strings out */
    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) == CON_PLAYING && i != ch->desc && i->character &&
            (IS_NPC(i->character) || !PRF_FLAGGED(i->character, channels[subcmd])) &&
            (IS_NPC(i->character) || !PLR_FLAGGED(i->character, PLR_WRITING)) &&
            !ROOM_FLAGGED(IN_ROOM(i->character), ROOM_SOUNDPROOF)) {

            if (subcmd == SCMD_SHOUT &&
                ((ch->getRoom()->zone != i->character->getRoom()->zone) ||
                 !AWAKE(i->character)))
                continue;

            if (CONFIG_ENABLE_LANGUAGES) {
                garble_text(argument, GET_SKILL(i->character, SPEAKING(ch)), SPEAKING(ch));
                snprintf(buf1, sizeof(buf1), "%s%s %ss%s '%s@n'%s", color_on,
                         GET_ADMLEVEL(ch) > 0 ? GET_NAME(ch) : GET_USER(ch), com_msgs[subcmd][1],
                         GET_SKILL(i->character, SPEAKING(ch)) ? "," : ", in an unfamiliar tongue,", argument,
                         color_on);
            } else if (subcmd == SCMD_SHOUT && IN_ROOM(i->character) != IN_ROOM(ch)) {
                snprintf(buf1, sizeof(buf1), "%s@WSomeone nearby %ss@W, '@w%s@W'@n%s", color_on, com_msgs[subcmd][1],
                         argument, color_on);
            } else if (subcmd == SCMD_SHOUT && IN_ROOM(i->character) == IN_ROOM(ch)) {
                snprintf(buf1, sizeof(buf1), "%s@W$n@W %ss@W, '@w%s@W'@n%s", color_on, com_msgs[subcmd][1], argument,
                         color_on);
            } else {
                if (GET_ADMLEVEL(ch) > 0) {
                    snprintf(buf1, sizeof(buf1), "%s@W$n %ss@W, '@w%s@W'@n%s", color_on, com_msgs[subcmd][1], argument,
                             color_on);
                } else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_HIDE) && GET_ADMLEVEL(i->character) < ADMLVL_IMMORT &&
                           ch != i->character) {
                    snprintf(buf1, sizeof(buf1), "%s@WAnonymous Player %ss@W, '@w%s@W'@n%s", color_on,
                             com_msgs[subcmd][1], argument, color_on);
                } else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_HIDE) && GET_ADMLEVEL(i->character) >= ADMLVL_IMMORT &&
                           ch != i->character) {
                    snprintf(buf1, sizeof(buf1), "%s@W%s(H) %ss@W, '@w%s@W'@n%s", color_on,
                             GET_ADMLEVEL(i->character) > 0 ? GET_NAME(ch) : GET_USER(ch), com_msgs[subcmd][1],
                             argument, color_on);
                } else if (GET_ADMLEVEL(i->character) > 0) {
                    snprintf(buf1, sizeof(buf1), "%s@W%s ($n) %ss@W, '@w%s@W'@n%s", color_on, GET_USER(ch),
                             com_msgs[subcmd][1], argument, color_on);
                } else {
                    snprintf(buf1, sizeof(buf1), "%s@W%s %ss@W, '@w%s@W'@n%s", color_on,
                             GET_ADMLEVEL(i->character) > 0 ? GET_NAME(ch) : GET_USER(ch), com_msgs[subcmd][1],
                             argument, color_on);
                }
            }

            msg = act(buf1, false, ch, nullptr, i->character, TO_VICT | TO_SLEEP);
            add_history(i->character, msg, hist_type[subcmd]);
        }
    }
    if (GET_SPAM(ch) >= 3 && GET_ADMLEVEL(ch) < 1) {
        send_to_imm("SPAMMING: %s has been frozen for spamming!\r\n", GET_NAME(ch));
        send_to_all("@rSPAMMING@D: @C%s@w has been frozen for spamming, let that be a lesson to 'em.@n\r\n",
                    GET_NAME(ch));
        ch->playerFlags.set(PLR_FROZEN);
        GET_FREEZE_LEV(ch) = 1;
    } else if (GET_SPAM(ch) < 3) {
        GET_SPAM(ch) += 1;
    }
}

ACMD(do_qcomm) {
    if (!PRF_FLAGGED(ch, PRF_QUEST)) {
        send_to_char(ch, "You aren't even part of the quest!\r\n");
        return;
    }
    skip_spaces(&argument);

    if (!*argument)
        send_to_char(ch, "%c%s?  Yes, fine, %s we must, but WHAT??\r\n", UPPER(*CMD_NAME), CMD_NAME + 1, CMD_NAME);
    else {
        char buf[MAX_STRING_LENGTH];
        struct descriptor_data *i;

        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(ch, "%s", CONFIG_OK);
        else if (subcmd == SCMD_QSAY) {
            snprintf(buf, sizeof(buf), "You quest-say, '%s'", argument);
            act(buf, false, ch, nullptr, argument, TO_CHAR);
        } else
            act(argument, false, ch, nullptr, argument, TO_CHAR);

        if (subcmd == SCMD_QSAY)
            snprintf(buf, sizeof(buf), "$n quest-says, '%s'", argument);
        else
            strlcpy(buf, argument, sizeof(buf));

        for (i = descriptor_list; i; i = i->next)
            if (STATE(i) == CON_PLAYING && i != ch->desc && PRF_FLAGGED(i->character, PRF_QUEST))
                act(buf, 0, ch, nullptr, i->character, TO_VICT | TO_SLEEP);
    }
}

ACMD(do_respond) {
    int mnum = 0;
    char number[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char(ch, "As a mob, you never bothered to learn to read or write.\r\n");
        return;
    }

    auto isBoard = [](const auto& o) { return GET_OBJ_TYPE(o) == ITEM_BOARD; };

    auto obj = ch->findObject(isBoard);
    if(!obj) obj = ch->getRoom()->findObject(isBoard);

    /* No board in the room? Send generic message -spl */
    if (!obj) {
        send_to_char(ch, "Sorry, you may only reply to messages posted on a board.\r\n");
        return;
    }

    argument = one_argument(argument, number);
    if (!*number) {
        send_to_char(ch, "Respond to what?\r\n");
        return;
    }
    if (!isdigit(*number) || (!(mnum = atoi(number)))) {
        send_to_char(ch, "You must type the number of the message you wish to reply to.\r\n");
        return;
    }
    board_respond(GET_OBJ_VNUM(obj), ch, mnum);
}
