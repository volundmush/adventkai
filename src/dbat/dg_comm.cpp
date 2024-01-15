/* ************************************************************************
*  File: dg_comm.c                               Part of Death's Gate MUD *
*                                                                         *
*  Usage: Contains routines to handle mud to player communication         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Death's Gate MUD is based on CircleMUD, Copyright (C) 1993, 94.        *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  $Author: Mark A. Heilpern/egreen/Welcor $                              *
*  $Date: 2004/10/11 12:07:00$                                            *
*  $Revision: 1.0.14 $                                                    *
************************************************************************ */
#include "dbat/dg_comm.h"
#include "dbat/act.informative.h"
#include "dbat/utils.h"
#include "dbat/races.h"
#include "dbat/comm.h"
#include "dbat/dg_scripts.h"
#include "dbat/graph.h"
#include "dbat/spells.h"
#include "dbat/handler.h"

/* local functions */
void sub_write_to_char(struct char_data *ch, char *tokens[], void *otokens[], char type[]);


/* same as any_one_arg except that it stops at punctuation */
char *any_one_name(char *argument, char *first_arg) {
    char *arg;

    /* Find first non blank */
    while (isspace(*argument))
        argument++;

    /* Find length of first word */
    for (arg = first_arg;
         *argument && !isspace(*argument) &&
         (!ispunct(*argument) || *argument == '#' || *argument == '-');
         arg++, argument++)
        *arg = LOWER(*argument);
    *arg = '\0';

    return argument;
}


void sub_write_to_char(struct char_data *ch, char *tokens[],
                       void *otokens[], char type[]) {
    char sb[MAX_STRING_LENGTH];
    int i;

    strcpy(sb, "");

    for (i = 0; tokens[i + 1]; i++) {
        strcat(sb, tokens[i]);

        switch (type[i]) {
            case '~':
                if (!otokens[i])
                    strcat(sb, "someone");
                else if ((struct char_data *) otokens[i] == ch)
                    strcat(sb, "you");
                else
                    strcat(sb, PERS((struct char_data *) otokens[i], ch));
                break;

            case '|':
                if (!otokens[i])
                    strcat(sb, "someone's");
                else if ((struct char_data *) otokens[i] == ch)
                    strcat(sb, "your");
                else {
                    strcat(sb, PERS((struct char_data *) otokens[i], ch));
                    strcat(sb, "'s");
                }
                break;

            case '^':
                if (!otokens[i] || !CAN_SEE(ch, (struct char_data *) otokens[i]))
                    strcat(sb, "its");
                else if (otokens[i] == ch)
                    strcat(sb, "your");
                else
                    strcat(sb, HSHR((struct char_data *) otokens[i]));
                break;

            case '&':
                if (!otokens[i] || !CAN_SEE(ch, (struct char_data *) otokens[i]))
                    strcat(sb, "it");
                else if (otokens[i] == ch)
                    strcat(sb, "you");
                else
                    strcat(sb, HSSH((struct char_data *) otokens[i]));
                break;

            case '*':
                if (!otokens[i] || !CAN_SEE(ch, (struct char_data *) otokens[i]))
                    strcat(sb, "it");
                else if (otokens[i] == ch)
                    strcat(sb, "you");
                else
                    strcat(sb, HMHR((struct char_data *) otokens[i]));
                break;

            case '¨':
                if (!otokens[i])
                    strcat(sb, "something");
                else
                    strcat(sb, OBJS(((struct obj_data *) otokens[i]), ch));
                break;
        }
    }

    strcat(sb, tokens[i]);
    strcat(sb, "\r\n");
    sb[0] = toupper(sb[0]);
    send_to_char(ch, "%s", sb);
}


void sub_write(char *arg, struct char_data *ch, int8_t find_invis, int targets) {
    char str[MAX_INPUT_LENGTH * 2];
    char type[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    char *tokens[MAX_INPUT_LENGTH], *s, *p;
    void *otokens[MAX_INPUT_LENGTH];
    struct char_data *to;
    struct obj_data *obj;
    int i, tmp;
    int to_sleeping = 1; /* mainly for windows compiles */

    if (!arg)
        return;

    tokens[0] = str;

    for (i = 0, p = arg, s = str; *p;) {
        switch (*p) {
            case '~':
            case '|':
            case '^':
            case '&':
            case '*':
                /* get char_data, move to next token */
                type[i] = *p;
                *s = '\0';
                p = any_one_name(++p, name);
                otokens[i] =
                        find_invis ? (void *) get_char_in_room(ch->getRoom(), name) : (void *) get_char_room_vis(
                                ch, name, nullptr);
                tokens[++i] = ++s;
                break;

            case '¨':
                /* get obj_data, move to next token */
                type[i] = *p;
                *s = '\0';
                p = any_one_name(++p, name);

                if (find_invis) obj = get_obj_in_room(ch->getRoom(), name);
                else if (!(obj = get_obj_in_list_vis(ch, name, nullptr, ch->getRoom()->contents)));
                else if (!(obj = get_obj_in_equip_vis(ch, name, &tmp, ch->equipment)));
                else obj = get_obj_in_list_vis(ch, name, nullptr, ch->contents);

                otokens[i] = (void *) obj;
                tokens[++i] = ++s;
                break;

            case '\\':
                p++;
                *s++ = *p++;
                break;

            default:
                *s++ = *p++;
        }
    }

    *s = '\0';
    tokens[++i] = nullptr;

    if (IS_SET(targets, TO_CHAR) && SENDOK(ch))
        sub_write_to_char(ch, tokens, otokens, type);

    if (IS_SET(targets, TO_ROOM))
        for (to = ch->getRoom()->people;
             to; to = to->next_in_room)
            if (to != ch && SENDOK(to))
                sub_write_to_char(to, tokens, otokens, type);
}


void send_to_zone(char *messg, zone_rnum zone) {
    struct descriptor_data *i;

    if (!messg || !*messg)
        return;

    for (i = descriptor_list; i; i = i->next)
        if (!i->connected && i->character && AWAKE(i->character) &&
            (IN_ROOM(i->character) != NOWHERE) &&
            (i->character->getRoom()->zone == zone))
            write_to_output(i, "%s", messg);
}

static bool isPlanet(const area_data& a) {
    return a.type == AreaType::CelestialBody;
}

void fly_planet(room_vnum roomVnum, char *messg, struct char_data *ch) {
    if (!messg || !*messg)
        return;

    vnum areaVnum = NOWHERE;
    {
        auto &r = world[roomVnum];
        if(auto found = r.getMatchingArea(area_data::isPlanet); found) {
            areaVnum = *found;
        }
    }

    if(areaVnum == NOWHERE) {
        return;
    }

    for(auto i = descriptor_list; i; i = i->next) {
        if(!i->connected) continue;
        if(!i->character) continue;
        if(!AWAKE(i->character)) continue;
        if(IN_ROOM(i->character) == NOWHERE) continue;
        if(!OUTSIDE(i->character)) continue;

        auto found = i->character->getMatchingArea(area_data::isPlanet);
        if(!found) continue;
        if(*found != areaVnum) continue;
        if (PLR_FLAGGED(i->character, PLR_DISGUISED)) {
            write_to_output(i, "A disguised figure %s", messg);
        } else {
            write_to_output(i, "%s%s %s", readIntro(i->character, ch) == 1 ? "" : "A ",
                            get_i_name(i->character, ch), messg);
        }
    }
}

void fly_zone(zone_rnum zone, char *messg, struct char_data *ch) {
    struct descriptor_data *i;

    if (!messg || !*messg)
        return;

    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected && i->character && AWAKE(i->character) && OUTSIDE(i->character) &&
            (IN_ROOM(i->character) != NOWHERE) && (i->character->getRoom()->zone == zone) && i->character != ch) {
            if (PLR_FLAGGED(i->character, PLR_DISGUISED)) {
                write_to_output(i, "A disguised figure %s", messg);
            } else {
                write_to_output(i, "%s%s %s", readIntro(i->character, ch) == 1 ? "" : "A ",
                                get_i_name(i->character, ch), messg);
            }
        }
    }
}

void send_to_sense(int type, char *messg, struct char_data *ch) {
    if (!messg || !*messg)
        return;

    auto planet = ch->getMatchingArea(area_data::isPlanet);
    if(!planet && type == 0) {
        return;
    }

    for (auto i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING) {
            continue;
        }
        auto tch = i->character;
        if (tch == ch) {
            continue;
        }
        if (IN_ROOM(ch) == IN_ROOM(tch))
            continue;
        auto obj = GET_EQ(tch, WEAR_EYE);
        if (!GET_SKILL(tch, SKILL_SENSE)) {
            continue;
        }
        if(auto p = tch->getMatchingArea(area_data::isPlanet); type == 0) {
            if (!p) {
                continue;
            }
            if (*p != *planet) {
                continue;
            }
        }
        if (obj && type == 0) {
            continue;
        }
        if (IS_ANDROID(ch)) {
            continue;
        }

        if (GET_HIT(ch) < (GET_HIT(tch) * 0.001) + 1)
            continue;

        if (type == 0) {
            if (GET_MAX_HIT(ch) > GET_MAX_HIT(tch)) {
                write_to_output(i, "%s who is stronger than you. They are nearby.\r\n", messg);
            } else if (GET_MAX_HIT(ch) >= GET_MAX_HIT(tch) * .9) {
                write_to_output(i, "%s who is near your strength. They are nearby.\r\n", messg);
            } else if (GET_MAX_HIT(ch) >= GET_MAX_HIT(tch) * .6) {
                write_to_output(i, "%s who is a good bit weaker than you. They are nearby.\r\n", messg);
            } else if (GET_MAX_HIT(ch) >= GET_MAX_HIT(tch) * .4) {
                write_to_output(i, "%s who is a lot weaker than you. They are nearby.\r\n", messg);
            }
            if (readIntro(tch, ch) == 1) {
                write_to_output(i, "@YYou recognise this signal as @y%s@Y!@n\r\n", get_i_name(tch, ch));
            } else if (read_sense_memory(ch, tch)) {
                write_to_output(i, "@YYou recognise this signal, but don't seem to know their name.@n\r\n");
            }
        } else if (planet_check(ch, tch)) {
            std::string blah = "UNKNOWN";
            {
                auto av = tch->getMatchingArea([&](auto &a) {return true;});
                if(av) {
                    blah = areas[av.value()].name;
                }
            }
            char power[MAX_INPUT_LENGTH];
            char align[MAX_INPUT_LENGTH];
            if (GET_HIT(ch) > GET_HIT(tch) * 10) {
                sprintf(power, ", who is @Runbelievably stronger@Y than you");
            } else if (GET_HIT(ch) > GET_HIT(tch) * 5) {
                sprintf(power, ", who is much @Rstronger@Y than you");
            } else if (GET_HIT(ch) > GET_HIT(tch) * 2) {
                sprintf(power, ", who is more than twice as @Rstrong@Y as you");
            } else if (GET_HIT(ch) > GET_HIT(tch)) {
                sprintf(power, ", who is somewhat @mstronger@Y than you");
            } else if (GET_HIT(ch) * 10 < GET_HIT(tch)) {
                sprintf(power, ", who is @Munbelievably weaker@Y than you");
            } else if (GET_HIT(ch) * 5 < GET_HIT(tch)) {
                sprintf(power, ", who is much @Mweaker@Y than you");
            } else if (GET_HIT(ch) * 2 < GET_HIT(tch)) {
                sprintf(power, ", who is more than twice as @Mweak@Y as you");
            } else if (GET_HIT(ch) < GET_HIT(tch)) {
                sprintf(power, ", who is somewhat @Wweaker@Y than you");
            } else {
                sprintf(power, ", who is close to @Cequal@Y with you");
            }
            if (GET_ALIGNMENT(ch) >= 1000) {
                sprintf(align, ", with a @wsaintly@Y aura,");
            } else if (GET_ALIGNMENT(ch) >= 500) {
                sprintf(align, ", with a very @Cgood@Y aura,");
            } else if (GET_ALIGNMENT(ch) >= 200) {
                sprintf(align, ", with a @cgood@Y aura,");
            } else if (GET_ALIGNMENT(ch) > -100) {
                sprintf(align, ", with a near @Wneutral@Y aura,");
            } else if (GET_ALIGNMENT(ch) > -200) {
                sprintf(align, ", with a sorta @revil@Y aura,");
            } else if (GET_ALIGNMENT(ch) > -500) {
                sprintf(align, ", with an @revil@Y aura,");
            } else if (GET_ALIGNMENT(ch) > -900) {
                sprintf(align, ", with a @rvery evil@Y aura,");
            } else {
                sprintf(align, ", with a @rd@De@Wv@wil@Wi@Ds@rh@Y aura,");
            }
            if (strstr(messg, "land"))
                write_to_output(i, "@YYou sense %s%s%s %s! They appear to have landed at...@G%s@n\r\n",
                                readIntro(tch, ch) == 1 ? get_i_name(tch, ch) : "someone", power, align, messg, blah.c_str());
            else
                write_to_output(i, "@YYou sense %s%s%s %s!@n\r\n",
                                readIntro(tch, ch) == 1 ? get_i_name(tch, ch) : "someone", power, align, messg);
        }
    }
}

void send_to_scouter(char *messg, struct char_data *ch, int num, int type) {
    if (!messg || !*messg)
        return;

    auto planet = ch->getMatchingArea(area_data::isPlanet);
    if(!planet && type == 0) {
        return;
    }

    for (auto i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING) {
            continue;
        }
        auto tch = i->character;
        auto obj = GET_EQ(tch, WEAR_EYE);
        if (tch == ch) continue;
        if(!AWAKE(tch)) continue;

        if(auto p = tch->getMatchingArea(area_data::isPlanet); type == 0) {
            if (!p) {
                continue;
            }
            if (*p != *planet) {
                continue;
            }
        }

        if (GET_INVIS_LEV(ch) > GET_ADMLEVEL(tch)) {
            continue;
        }
        if (IS_ANDROID(ch)) continue;

        if (!obj) continue;
        if (IN_ROOM(ch) == IN_ROOM(tch)) continue;

        if (type == 0) {
            if (num == 1) {
                auto obj = GET_EQ(tch, WEAR_EYE);
                if (OBJ_FLAGGED(obj, ITEM_BSCOUTER) && GET_HIT(ch) >= 150000) {
                    write_to_output(i, "@D[@GBlip@D]@r Rising Powerlevel Detected@D:@Y ??????????\r\n");
                } else if (OBJ_FLAGGED(obj, ITEM_MSCOUTER) && GET_HIT(ch) >= 5000000) {
                    write_to_output(i, "@D[@GBlip@D]@r Rising Powerlevel Detected@D:@Y ??????????\r\n");
                } else if (OBJ_FLAGGED(obj, ITEM_ASCOUTER) && GET_HIT(ch) >= 15000000) {
                    write_to_output(i, "@D[@GBlip@D]@r Rising Powerlevel Detected@D:@Y ??????????\r\n");
                } else {
                    write_to_output(i, "%s@n", messg);
                }
            } else {
                if (OBJ_FLAGGED(obj, ITEM_BSCOUTER) && GET_HIT(ch) >= 150000) {
                    write_to_output(i, "@D[@GBlip@D]@r Nearby Powerlevel Detected@D:@Y ??????????\r\n");
                } else if (OBJ_FLAGGED(obj, ITEM_MSCOUTER) && GET_HIT(ch) >= 5000000) {
                    write_to_output(i, "@D[@GBlip@D]@r Nearby Powerlevel Detected@D:@Y ??????????\r\n");
                } else if (OBJ_FLAGGED(obj, ITEM_ASCOUTER) && GET_HIT(ch) >= 15000000) {
                    write_to_output(i, "@D[@GBlip@D]@r Nearby Powerlevel Detected@D:@Y ??????????\r\n");
                } else {
                    write_to_output(i, "%s\r\n", messg);
                }
            }
        } else if (type == 1 && GET_SKILL(tch, SKILL_SENSE) < 20) {
            if (OBJ_FLAGGED(obj, ITEM_BSCOUTER) && GET_HIT(ch) >= 150000) {
                write_to_output(i, "@D[@GBlip@D]@w %s. @RPL@D:@Y ??????????\r\n", messg);
            } else if (OBJ_FLAGGED(obj, ITEM_MSCOUTER) && GET_HIT(ch) >= 5000000) {
                write_to_output(i, "@D[@GBlip@D]@w %s. @RPL@D:@Y ??????????\r\n", messg);
            } else if (OBJ_FLAGGED(obj, ITEM_ASCOUTER) && GET_HIT(ch) >= 15000000) {
                write_to_output(i, "@D[@GBlip@D]@w %s. @RPL@D:@Y ??????????\r\n", messg);
            } else {
                write_to_output(i, "@D[Blip@D]@w %s. @RPL@D:@Y %s@n\r\n\r\n", messg, add_commas(GET_HIT(ch)).c_str());
            }
        } else if (type == 2 && GET_SKILL(tch, SKILL_SENSE) < 20) {
            std::string blah = "UNKNOWN";
            {
                auto av = tch->getMatchingArea([&](auto &a) {return true;});
                if(av) {
                    blah = areas[av.value()].name;
                }
            }
            if (OBJ_FLAGGED(obj, ITEM_BSCOUTER) && GET_HIT(ch) >= 150000) {
                write_to_output(i, "@D[@GBlip@D]@w %s at... @G%s. @RPL@D:@Y ??????????\r\n", messg, blah.c_str());
            } else if (OBJ_FLAGGED(obj, ITEM_MSCOUTER) && GET_HIT(ch) >= 5000000) {
                write_to_output(i, "@D[@GBlip@D]@w %s at... @G%s. @RPL@D:@Y ??????????\r\n", messg, blah.c_str());
            } else if (OBJ_FLAGGED(obj, ITEM_ASCOUTER) && GET_HIT(ch) >= 15000000) {
                write_to_output(i, "@D[@GBlip@D]@w %s at... @G%s. @RPL@D:@Y ??????????\r\n", messg, blah.c_str());
            } else {
                write_to_output(i, "@D[Blip@D]@w %s at... @G%s. @RPL@D:@Y %s@n\r\n\r\n", messg, blah.c_str(),
                                add_commas(GET_HIT(ch)).c_str());
            }
        }
    }
}

void send_to_worlds(struct char_data *ch) {
    char message[MAX_INPUT_LENGTH];

    if (GET_MAX_HIT(ch) > 2000000000) {
        sprintf(message, "@RThe whole planet begins to quake violently as if the world is ending!@n\r\n");
    } else if (GET_MAX_HIT(ch) > 1000000000) {
        sprintf(message, "@RThe whole planet begins to quake violently with a thunderous roar!@n\r\n");
    } else if (GET_MAX_HIT(ch) > 500000000) {
        sprintf(message, "@RThe whole planet begins to quake violently!@n\r\n");
    } else if (GET_MAX_HIT(ch) > 100000000) {
        sprintf(message, "@RThe whole planet rumbles and shakes!@n\r\n");
    } else if (GET_MAX_HIT(ch) > 50000000) {
        sprintf(message, "@RThe whole planet rumbles faintly!@n\r\n");
    } else {
        return;
    }

    auto p = ch->getMatchingArea(area_data::isPlanet);
    if(!p) return;

    for (auto i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING) {
            continue;
        }
        auto op = i->character->getMatchingArea(area_data::isPlanet);
        if(!op) continue;
        if(op.value() != p.value()) continue;
        send_to_char(i->character, "%s", message);
    }
}
