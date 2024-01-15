/* ************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "dbat/weather.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/interpreter.h"
#include "dbat/db.h"
#include "dbat/dg_comm.h"
#include "dbat/config.h"
#include "dbat/races.h"
#include "dbat/dg_scripts.h"
#include "dbat/transformation.h"

static void phase_powerup(struct char_data *ch, int type, int phase);

static void grow_plants();


static void grow_plants() {
    struct obj_data *k;

    for (k = object_list; k; k = k->next) {
        if (IN_ROOM(k) == NOWHERE) {
            continue;
        } else if (k->carried_by || k->in_obj) {
            continue;
        } else if (ROOM_FLAGGED(IN_ROOM(k), ROOM_GARDEN1) || ROOM_FLAGGED(IN_ROOM(k), ROOM_GARDEN2)) {
            if (GET_OBJ_VAL(k, VAL_WATERLEVEL) < 0 && GET_OBJ_VAL(k, VAL_WATERLEVEL) > -10) {
                GET_OBJ_VAL(k, VAL_WATERLEVEL) -= 1;
                if (GET_OBJ_VAL(k, VAL_WATERLEVEL) > -10) {
                    send_to_room(IN_ROOM(k), "%s@y withers a bit.\r\n", k->short_description);
                } else {
                    send_to_room(IN_ROOM(k), "%s@y has withered to a dried up dead husk.\r\n", k->short_description);
                }
            } else if (GET_OBJ_VAL(k, VAL_WATERLEVEL) >= 0) {
                GET_OBJ_VAL(k, VAL_WATERLEVEL) -= 1;
                if (GET_OBJ_VAL(k, VAL_GROWTH) < GET_OBJ_VAL(k, VAL_MATGOAL) &&
                    GET_OBJ_VAL(k, VAL_MATURITY) < GET_OBJ_VAL(k, VAL_MAXMATURE)) {
                    GET_OBJ_VAL(k, VAL_GROWTH) += 1;
                    if (GET_OBJ_VAL(k, VAL_GROWTH) >= GET_OBJ_VAL(k, VAL_MATGOAL)) {
                        GET_OBJ_VAL(k, VAL_GROWTH) = 0;
                        GET_OBJ_VAL(k, VAL_MATURITY) += 1;
                    }
                    if (GET_OBJ_VAL(k, VAL_MATURITY) >= GET_OBJ_VAL(k, VAL_MAXMATURE)) {
                        send_to_room(IN_ROOM(k), "%s@G is now fully grown!@n\r\n", k->short_description);
                    }
                }
            }
        }
    }

}

nlohmann::json weather_data::serialize() {
    auto j = nlohmann::json::object();
    j["pressure"] = pressure;
    j["change"] = change;
    j["sky"] = sky;
    j["sunlight"] = sunlight;
    return j;
}

void weather_data::deserialize(const nlohmann::json &j) {
    if(j.contains("pressure")) pressure = j["pressure"];
    if(j.contains("change")) change = j["change"];
    if(j.contains("sky")) sky = j["sky"];
    if(j.contains("sunlight")) sunlight = j["sunlight"];
}

void time_info_data::deserialize(const nlohmann::json &j) {
    if(j.contains("remainder")) remainder = j["remainder"];
    if(j.contains("seconds")) seconds = j["seconds"];
    if(j.contains("minutes")) minutes = j["minutes"];
    if(j.contains("hours")) hours = j["hours"];
    if(j.contains("day")) day = j["day"];
    if(j.contains("month")) month = j["month"];
    if(j.contains("year")) year = j["year"];
}

nlohmann::json time_info_data::serialize() {
    auto j = nlohmann::json::object();
    j["remainder"] = remainder;
    j["seconds"] = seconds;
    j["minutes"] = minutes;
    j["hours"] = hours;
    j["day"] = day;
    j["month"] = month;
    j["year"] = year;
    return j;
}





static void weather_change() {
    int diff, change;
    if ((time_info.month >= 9) && (time_info.month <= 16))
        diff = (weather_info.pressure > 985 ? -2 : 2);
    else
        diff = (weather_info.pressure > 1015 ? -2 : 2);

    weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

    weather_info.change = MIN(weather_info.change, 12);
    weather_info.change = MAX(weather_info.change, -12);

    weather_info.pressure += weather_info.change;

    weather_info.pressure = MIN(weather_info.pressure, 1040);
    weather_info.pressure = MAX(weather_info.pressure, 960);

    change = 0;

    switch (weather_info.sky) {
        case SKY_CLOUDLESS:
            if (weather_info.pressure < 990)
                change = 1;
            else if (weather_info.pressure < 1010)
                if (dice(1, 4) == 1)
                    change = 1;
            break;
        case SKY_CLOUDY:
            if (weather_info.pressure < 970)
                change = 2;
            else if (weather_info.pressure < 990) {
                if (dice(1, 4) == 1)
                    change = 2;
                else
                    change = 0;
            } else if (weather_info.pressure > 1030)
                if (dice(1, 4) == 1)
                    change = 3;

            break;
        case SKY_RAINING:
            if (weather_info.pressure < 970) {
                if (dice(1, 4) == 1)
                    change = 4;
                else
                    change = 0;
            } else if (weather_info.pressure > 1030)
                change = 5;
            else if (weather_info.pressure > 1010)
                if (dice(1, 4) == 1)
                    change = 5;

            break;
        case SKY_LIGHTNING:
            if (weather_info.pressure > 1010)
                change = 6;
            else if (weather_info.pressure > 990)
                if (dice(1, 4) == 1)
                    change = 6;

            break;
        default:
            change = 0;
            weather_info.sky = SKY_CLOUDLESS;
            break;
    }

    switch (change) {
        case 0:
            break;
        case 1:
            send_to_outdoor("The sky starts to get cloudy.\r\n");
            weather_info.sky = SKY_CLOUDY;
            break;
        case 2:
            send_to_outdoor("It starts to rain.\r\n");
            weather_info.sky = SKY_RAINING;
            break;
        case 3:
            send_to_outdoor("The clouds disappear.\r\n");
            weather_info.sky = SKY_CLOUDLESS;
            break;
        case 4:
            send_to_outdoor("Lightning starts to show in the sky.\r\n");
            weather_info.sky = SKY_LIGHTNING;
            break;
        case 5:
            send_to_outdoor("The rain stops.\r\n");
            weather_info.sky = SKY_CLOUDY;
            break;
        case 6:
            send_to_outdoor("The lightning stops.\r\n");
            weather_info.sky = SKY_RAINING;
            break;
        default:
            break;
    }
}

void oozaru_revert(char_data *ch) {
    if (!(ch->form == FormID::Oozaru || ch->form == FormID::GoldenOozaru))
        return;

    auto &data = ch->transforms[ch->form];
    data.blutz = 0.0;

    act("@CYour body begins to shrink back to its normal form as the power of the Oozaru leaves you. You fall asleep shortly after returning to normal!@n",
        true, ch, nullptr, nullptr, TO_CHAR);
    act("@c$n@C's body begins to shrink and return to normal. Their giant ape features fading back into humanoid features until $e is left normal.@n",
        true, ch, nullptr, nullptr, TO_ROOM);

    ch->form = FormID::Base;

}


/* This controls the powering up of Hoshi-jin from their Eldritch Star */
void star_phase(struct char_data *ch, int type) {
    struct descriptor_data *d;

    if (ch == nullptr) {
        for (d = descriptor_list; d; d = d->next) {
            if (!IS_PLAYING(d)) {
                continue;
            }
            if (IS_NPC(d->character)) {
                continue;
            }
            if (GET_LEVEL(d->character) < 2) {
                continue;
            }
            if (IS_HOSHIJIN(d->character)) {
                ch = d->character;
                switch (type) {
                    case 0:
                        if (GET_PHASE(ch) > 0) {
                            act("@WYour eyes and the glyphs on your skin slowly start to lose their glow. You feel the power received from the @GE@gl@Dd@wri@Dt@gc@Gh @YS@yta@Yr@W drain away from your body. It has apparently entered the @rDeath Phase@W of its cycle...@n",
                                true, ch, nullptr, nullptr, TO_CHAR);
                            act("@c$n@W's eyes and the glyphs on $s skin slowly start to lose their glow. You notice that $e seems weaker now for some reason.@n",
                                true, ch, nullptr, nullptr, TO_ROOM);
                            phase_powerup(ch, 0, GET_PHASE(ch));
                        }
                        break; // Drop Powerup
                    case 1:
                        if (GET_PHASE(ch) != 1) {
                            act("@WYou suddenly feel a @RSURGE@W of power through your body. You feel the @GE@gl@Dd@wri@Dt@gc@Gh @YS@yta@Yr@W come into its @CBirth Phase@W and its power is flowing into your body! Finally your eyes and the glyphs on your skin begin to glow an electric @bb@Bl@Cue@W!@n",
                                true, ch, nullptr, nullptr, TO_CHAR);
                            act("@c$n@W suddenly seems to grow stronger for some reason. You notice $s eyes begin to glow an electric @bb@Bl@Cue@W. Suddenly glyphs start to appear all over $s skin and glow with the same light!@n",
                                true, ch, nullptr, nullptr, TO_ROOM);
                            phase_powerup(ch, 0, GET_PHASE(ch));
                            phase_powerup(ch, 1, 1);
                        }
                        break; // Powerup Phase 1
                    case 2:
                        if (GET_PHASE(ch) != 2) {
                            act("@WYou suddenly feel a @RSURGE@W of power through your body. You feel the @GE@gl@Dd@wri@Dt@gc@Gh @YS@yta@Yr@W come into its @GLife Phase@W and its power is flowing into your body! Finally your eyes and the glyphs on your skin begin to glow an fiery @Rr@re@Rd@W!@n",
                                true, ch, nullptr, nullptr, TO_CHAR);
                            act("@c$n@W suddenly seems to grow stronger for some reason. You notice $s eyes begin to glow a fiery @rR@Re@rd@W. Suddenly glyphs start to appear all over $s skin and glow with the same light!@n",
                                true, ch, nullptr, nullptr, TO_ROOM);
                            phase_powerup(ch, 0, GET_PHASE(ch));
                            phase_powerup(ch, 1, 2);
                        }
                        break; // Powerup Phase 2
                    default:
                        send_to_imm("Strange Error in star_phase by: %s", GET_NAME(ch));
                        break; // Error
                }

            } // End of is HOSHIJIN
        } // End of descriptor_list for
        return;
    } else if (ch != nullptr && !IS_NPC(ch) && GET_LEVEL(ch) > 1) {
        if (IS_HOSHIJIN(ch)) {
            switch (type) {
                case 0:
                    if (GET_PHASE(ch) > 0) {
                        act("@WYour eyes and the glyphs on your skin slowly start to lose their glow. You feel the power received from the @GE@gl@Dd@wri@Dt@gc@Gh @YS@yta@Yr@W drain away from your body. It has apparently entered the @rDeath Phase@W of its cycle...@n",
                            true, ch, nullptr, nullptr, TO_CHAR);
                        act("@c$n@W's eyes and the glyphs on $s skin slowly start to lose their glow. You notice that $e seems weaker now for some reason.@n",
                            true, ch, nullptr, nullptr, TO_ROOM);
                        phase_powerup(ch, 0, GET_PHASE(ch));
                    }
                    break; // Drop Powerup
                case 1:
                    if (GET_PHASE(ch) != 1) {
                        act("@WYou suddenly feel a @RSURGE@W of power through your body. You feel the @GE@gl@Dd@wri@Dt@gc@Gh @YS@yta@Yr@W come into its @CBirth Phase@W and its power is flowing into your body! Finally your eyes and the glyphs on your skin begin to glow an electric @bb@Bl@Cue@W!@n",
                            true, ch, nullptr, nullptr, TO_CHAR);
                        act("@c$n@W suddenly seems to grow stronger for some reason. You notice $s eyes begin to glow an electric @bb@Bl@Cue@W. Suddenly glyphs start to appear all over $s skin and glow with the same light!@n",
                            true, ch, nullptr, nullptr, TO_ROOM);
                        phase_powerup(ch, 0, GET_PHASE(ch));
                        phase_powerup(ch, 1, 1);
                    }
                    break; // Powerup Phase 1
                case 2:
                    if (GET_PHASE(ch) != 2) {
                        act("@WYou suddenly feel a @RSURGE@W of power through your body. You feel the @GE@gl@Dd@wri@Dt@gc@Gh @YS@yta@Yr@W come into its @GLife Phase@W and its power is flowing into your body! Finally your eyes and the glyphs on your skin begin to glow an fiery @Rr@re@Rd@W!@n",
                            true, ch, nullptr, nullptr, TO_CHAR);
                        act("@c$n@W suddenly seems to grow stronger for some reason. You notice $s eyes begin to glow a fiery @rR@Re@rd@W. Suddenly glyphs start to appear all over $s skin and glow with the same light!@n",
                            true, ch, nullptr, nullptr, TO_ROOM);
                        phase_powerup(ch, 0, GET_PHASE(ch));
                        phase_powerup(ch, 1, 2);
                    }
                    break; // Powerup Phase 2
                default:
                    send_to_imm("Strange Error in star_phase by: %s", GET_NAME(ch));
                    break; // Error
            }
            return;
        } // End of is HOSHIJIN
        else {
            return;
        }
    } // End of ch is/isn't nullptr

    return;
}

/* This handles powering up a Hoshijin or powering them down */
static void phase_powerup(struct char_data *ch, int type, int phase) {
    if (!ch) {
        return;
    }
    if (IS_NPC(ch)) {
        return;
    }

    int bonus = 0;
    double mult = 0.0;

    switch (phase) {
        case 0:
            return;
        case 1:
            bonus = 5;
            mult = 1.0;
            break;
        case 2:
            bonus = 8;
            mult = 2.0;
            break;
        default:
            send_to_imm("Error: phase_powerup called with GET_PHASE equal to zero by: %s", GET_NAME(ch));
            return;
    }

    if (type == 0) { // Drop their stats
        null_affect(ch, AFF_STARPHASE);
        GET_PHASE(ch) = 0;
    } else { // Alter their stats
        assign_affect(ch, AFF_STARPHASE, 0, -1, bonus, 0, 0, 0, 0, bonus);
        if (IN_ROOM(ch) != NOWHERE && ETHER_STREAM(ch))
            mult += .5;

        struct affected_type aff;
        aff.location = APPLY_VITALS_MULT;
        aff.modifier = mult;
        aff.bitvector = AFF_STARPHASE;
        affect_join(ch, &aff, false, false, false, false);
        GET_PHASE(ch) = phase;
    }
}

static void secondChanged() {

}

static void minuteChanged() {
    switch(time_info.minutes) {
        case 0:
        case 15:
        case 30:
        case 45:
            check_interval_triggers(MTRIG_QUARTER);
        break;

    }
}

static void hourChanged() {
    grow_plants();
    weather_change();
    check_time_triggers();
    check_interval_triggers(MTRIG_HOURLY);

    switch (time_info.hours) {
        case 4:
            if (MOON_DATE) {
                send_to_moon("The full moon disappears.\r\n");
            } else if (time_info.day == 22) {
                send_to_moon("The full moon disappears.\r\n");
            }
            break;
        case 5:
            weather_info.sunlight = SUN_RISE;
            send_to_outdoor("The sun rises in the east.\r\n");
            if (time_info.day <= 14) {
                star_phase(nullptr, 1);
            } else if (time_info.day <= 21) {
                star_phase(nullptr, 2);
            } else {
                star_phase(nullptr, 0);
            }
            break;
        case 6:
            weather_info.sunlight = SUN_LIGHT;
            send_to_outdoor("The day has begun.\r\n");
            break;
        case 19:
            weather_info.sunlight = SUN_SET;
            send_to_outdoor("The sun slowly disappears in the west.\r\n");
            break;
        case 20:
            weather_info.sunlight = SUN_DARK;
            send_to_outdoor("The night has begun.\r\n");
            break;
        case 21:
            if (MOON_DATE) {
                send_to_moon("The full moon has risen.\r\n");
            }
            break;
        default:
            break;
    }
}

static void dayChanged() {

}

static void monthChanged() {

}

static void yearChanged() {

}

static void ageAllCharacters(double addedTime) {
    for(const auto &[id, p] : uniqueCharacters) {
        p.second->ageBy(addedTime);
    }
}


void advanceClock(uint64_t heartPulse, double deltaTime) {
    const auto addedTime = deltaTime * MUD_TIME_ACCELERATION;
    time_info.remainder += addedTime;

    while(time_info.remainder >= 1.0) {
        bool secondChangedCheck = false;
        bool minuteChangedCheck = false;
        bool hourChangedCheck = false;
        bool dayChangedCheck = false;
        bool monthChangedCheck = false;
        bool yearChangedCheck = false;

        time_info.remainder -= 1.0;
        time_info.seconds++;
        secondChangedCheck = true;

        if(time_info.seconds >= SECONDS_PER_MINUTE) {
            time_info.seconds -= SECONDS_PER_MINUTE;
            time_info.minutes++;
            minuteChangedCheck = true;

            if (time_info.minutes >= MINUTES_PER_HOUR) {
                time_info.minutes -= MINUTES_PER_HOUR;
                time_info.hours++;
                hourChangedCheck = true;

                if (time_info.hours >= HOURS_PER_DAY) {
                    time_info.hours -= HOURS_PER_DAY;
                    time_info.day++;
                    dayChangedCheck = true;

                    if (time_info.day >= DAYS_PER_MONTH) {
                        time_info.day -= DAYS_PER_MONTH;
                        time_info.month++;
                        monthChangedCheck = true;

                        if (time_info.month >= MONTHS_PER_YEAR) {
                            time_info.month -= MONTHS_PER_YEAR;
                            time_info.year++;
                            yearChangedCheck = true;
                        }
                    }
                }
            }
        }

        if(secondChangedCheck) secondChanged();
        if(minuteChangedCheck) minuteChanged();
        if(hourChangedCheck) hourChanged();
        if(dayChangedCheck) dayChanged();
        if(monthChangedCheck) monthChanged();
        if(yearChangedCheck) yearChanged();

    }

    ageAllCharacters(addedTime);

}

int64_t time_info_data::current() {
    int64_t s = 0;

    s += year * (int64_t)(SECS_PER_DAY * DAYS_PER_MONTH * MONTHS_PER_YEAR);
    s += month * (int64_t)(SECS_PER_DAY * DAYS_PER_MONTH);
    s += day * (int64_t)SECS_PER_DAY;
    s += hours * (int64_t)SECS_PER_HOUR;
    s += minutes * (int64_t)SECONDS_PER_MINUTE;
    s += seconds;

    return s;
}

time_info_data::time_info_data(int64_t timestamp) {
    // We have been given a number of seconds since year 0.
    // We must convert this to a year, month, day, hour, minute, second format.
    year = timestamp / (SECS_PER_DAY * DAYS_PER_MONTH * MONTHS_PER_YEAR);
    timestamp -= year * (SECS_PER_DAY * DAYS_PER_MONTH * MONTHS_PER_YEAR);
    month = timestamp / (SECS_PER_DAY * DAYS_PER_MONTH);
    timestamp -= month * (SECS_PER_DAY * DAYS_PER_MONTH);
    day = timestamp / SECS_PER_DAY;
    timestamp -= day * SECS_PER_DAY;
    hours = timestamp / SECS_PER_HOUR;
    timestamp -= hours * SECS_PER_HOUR;
    minutes = timestamp / SECONDS_PER_MINUTE;
    timestamp -= minutes * SECONDS_PER_MINUTE;
    seconds = timestamp;
}

int time_data::currentAge() {
    return secondsAged / SECS_PER_GAME_YEAR;
}