/* ************************************************************************
 *   File: Guild.c                                                         *
 *  Usage: GuildMaster's: loading files, assigning spec_procs, and handling*
 *                        practicing.                                      *
 *                                                                         *
 * Based on shop.c.  As such, the CircleMud License applies                *
 * Written by Jason Goodwin.   jgoodwin@expert.cc.purdue.edu               *
 ************************************************************************ */

#include "dbat/guild.h"
#include "dbat/utils.h"
#include "dbat/spells.h"
#include "dbat/comm.h"
#include "dbat/db.h"
#include "dbat/interpreter.h"
#include "dbat/gengld.h"
#include "dbat/local_limits.h"
#include "dbat/feats.h"
#include "dbat/act.comm.h"
#include "dbat/handler.h"
#include "dbat/shop.h"
#include "dbat/class.h"
#include "dbat/constants.h"
#include "dbat/genzon.h"

/* Local variables */
int spell_sort_info[SKILL_TABLE_SIZE + 1];
guild_vnum top_guild = NOTHING;
std::map<guild_vnum, struct guild_data> guild_index;

char *guild_customer_string(int guild_nr, int detailed);

int calculate_skill_cost(struct char_data *ch, int skill);

int calculate_skill_cost(struct char_data *ch, int skill) {
    int cost = 0;

    if (IS_SET(spell_info[skill].flags, SKFLAG_TIER2)) {
        cost = 8;
    } else if (IS_SET(spell_info[skill].flags, SKFLAG_TIER3)) {
        cost = 15;
    } else if (IS_SET(spell_info[skill].flags, SKFLAG_TIER4)) {
        if (GET_SKILL_BASE(ch, skill) == 0) {
            cost = 200;
        } else {
            cost = 25;
        }
    } else if (IS_SET(spell_info[skill].flags, SKFLAG_TIER5)) {
        if (GET_SKILL_BASE(ch, skill) == 0) {
            cost = 300;
        } else {
            cost = 40;
        }
    } else {
        cost = 4;
    }

    if (GET_SKILL_BASE(ch, skill) > 90)
        cost += 12;
    else if (GET_SKILL_BASE(ch, skill) > 80)
        cost += 10;
    else if (GET_SKILL_BASE(ch, skill) > 70)
        cost += 8;
    else if (GET_SKILL_BASE(ch, skill) > 50)
        cost += 6;
    else if (GET_SKILL_BASE(ch, skill) > 40)
        cost += 2;
    else if (GET_SKILL_BASE(ch, skill) > 30)
        cost += 1;

    if (GET_FORGETING(ch) != 0)
        cost += 6;

    if (skill == SKILL_RUNIC)
        cost += 6;
    if (skill == SKILL_EXTRACT)
        cost += 3;

    if (IS_HOSHIJIN(ch) &&
        (skill == SKILL_PUNCH || skill == SKILL_KICK || skill == SKILL_KNEE || skill == SKILL_ELBOW ||
         skill == SKILL_UPPERCUT || skill == SKILL_ROUNDHOUSE || skill == SKILL_SLAM || skill == SKILL_HEELDROP ||
         skill == SKILL_DAGGER || skill == SKILL_SWORD || skill == SKILL_CLUB || skill == SKILL_GUN ||
         skill == SKILL_SPEAR || skill == SKILL_BRAWL)) {
        cost += 5;
    }

    if (skill == SKILL_INSTANTT) {
        if (GET_SKILL_BASE(ch, skill) == 0) {
            cost = 2000;
        } else {
            cost = 50;
        }
    }
    if (skill == SKILL_MYSTICMUSIC) {
        cost = cost * 1.5;
    }

    return (cost);
}

void handle_ingest_learn(struct char_data *ch, struct char_data *vict) {

    int i = 1;
    send_to_char(ch, "@YAll your current skills improve somewhat!@n\r\n");

    for (i = 1; i <= SKILL_TABLE_SIZE; i++) {

        if (GET_SKILL_BASE(ch, i) > 0 && GET_SKILL_BASE(vict, i) > 0 && i != 141) {
            send_to_char(ch, "@YYou gained a lot of new knowledge about @y%s@Y!@n\r\n", spell_info[i].name);
			auto &s = ch->skill[i];
            if(s.level + 10 < 100) s.level += 10;
            else if(s.level > 0 && s.level < 100) s.level += 1;
            else s.level = 100;

        }
        if (((i >= 481 && i <= 489) || i == 517 || i == 535) &&
            ((GET_SKILL_BASE(ch, i) <= 0) && GET_SKILL_BASE(vict, i) > 0)) {
            SET_SKILL(ch, i, GET_SKILL_BASE(ch, i) + rand_number(10, 25));
            send_to_char(ch, "@YYou learned @y%s@Y from ingesting your target!@n\r\n", spell_info[i].name);
            GET_SLOTS(ch) += 1;
            GET_INGESTLEARNED(ch) = 1;

        }


    }
}

ACMD(do_teach) {

    if (IS_NPC(ch))
        return;

    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int skill = 100;
    struct char_data *vict;

    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char(ch, "What skill are you wanting to teach?\r\n");
        return;
    }

    skill = find_skill_num(arg, SKTYPE_SKILL);

    if (GET_SKILL_BASE(ch, skill) < 101) {
        send_to_char(ch, "You are not a Grand Master in that skill!\r\n");
        send_to_char(ch, "@wSyntax: teach (skill) (target)@n\r\n");
        return;
    }

    if (!*arg2) {
        send_to_char(ch, "@wWho are you wanting to teach @C%s@w to?@n\r\n", spell_info[skill].name);
        send_to_char(ch, "@wSyntax: teach (skill) (target)@n\r\n");
        return;
    }

    if (!(vict = get_char_vis(ch, arg2, nullptr, FIND_CHAR_ROOM))) {
        send_to_char(ch, "@wTeach who?@n\r\n");
        send_to_char(ch, "@wSyntax: teach (skill) (target)@n\r\n");
        return;
    }

    int cost = calculate_skill_cost(vict, skill), free = false;

    if (GET_SKILL_BASE(ch, skill) >= 103) {
        cost = cost * 0.5;
        if (rand_number(1, 4) == 4) {
            free = true;
        }
    } else if (GET_SKILL_BASE(ch, skill) == 102) {
        cost = cost * 0.5;
    } else {
        cost = cost * 0.75;
    }

    if (cost == 0) /* Just to be sure */
        cost = 1;

    if (!vict->master) {
        send_to_char(ch, "They must be following you in order for you to teach them.\r\n");
        return;
    } else if (vict->master != ch) {
        send_to_char(ch, "They must be following you in order for you to teach them.\r\n");
        return;
    } else if (GET_FORGETING(vict) == skill) {
        send_to_char(ch, "They are trying to forget that skill!\r\n");
        return;
    } else if (GET_PRACTICES(vict) < cost) {
        send_to_char(ch, "They do not have enough practice sessions for you to teach them.\r\n");
        return;
    } else if (GET_SKILL_BASE(vict, skill) >= 80) {
        send_to_char(ch, "You can not teach them anymore.\r\n");
        return;
    } else if (GET_SKILL_BASE(vict, skill) > 0) {
        char tochar[MAX_STRING_LENGTH], tovict[MAX_STRING_LENGTH], toother[MAX_STRING_LENGTH];
        sprintf(tochar, "@YYou instruct @y$N@Y in the finer points of @C%s@Y.@n\r\n", spell_info[skill].name);
        sprintf(tovict, "@y$n@Y instructs you in the finer points of @C%s@Y.@n\r\n", spell_info[skill].name);
        sprintf(toother, "@y$n@Y instructs @y$N@Y in the finer points of @C%s@Y.@n\r\n", spell_info[skill].name);
        act(tochar, true, ch, nullptr, vict, TO_CHAR);
        act(tovict, true, ch, nullptr, vict, TO_VICT);
        act(toother, true, ch, nullptr, vict, TO_NOTVICT);
        SET_SKILL(vict, skill, GET_SKILL_BASE(vict, skill) + 1);
        if (free == false) {
            vict->modPractices(-cost);
        } else {
            send_to_char(ch, "@GYou teach your lesson so well that it cost them nothing to learn from you!@n\r\n");
            send_to_char(vict, "@GYour teacher taught you the lesson so well that it cost you nothing!@n\r\n");
        }
    } else {
        send_to_char(ch, "They do not even know the basics. It's a waste of your teaching skills.\r\n");
        return;
    }
}

const char *how_good(int percent) {
    if (percent < 0)
        return " error)";
    if (percent == 0)
        return "(@Mnot@n)";
    if (percent <= 10)
        return "(@rawful@n)";
    if (percent <= 20)
        return "(@Rbad@n)";
    if (percent <= 40)
        return "(@ypoor@n)";
    if (percent <= 55)
        return "(@Yaverage@n)";
    if (percent <= 70)
        return "(@gfair@n)";
    if (percent <= 80)
        return "(@Ggood@n)";
    if (percent <= 85)
        return "(@bgreat@n)";
    if (percent <= 100)
        return "(@Bsuperb@n)";

    return "(@rinate@n)";
}

const char *prac_types[] = {
        "spell",
        "skill"
};


int compare_spells(const void *x, const void *y) {
    int a = *(const int *) x,
            b = *(const int *) y;

    return strcmp(spell_info[a].name, spell_info[b].name);
}


int print_skills_by_type(struct char_data *ch, char *buf, int maxsz, int sktype, char *argument) {
    char arg[1000];
    size_t len = 0;
    int t, known, nlen = 0, count = 0, canknow = 0;
    char buf2[READ_SIZE];

    one_argument(argument, arg);

    for (auto &[i, sk] : ch->skill) {
        t = spell_info[i].skilltype;

        if (t != sktype)
            continue;

        if ((t & SKTYPE_SKILL) || (t & SKTYPE_SPELL)) {

        } else {
            continue;
        }
        auto sklevel = GET_SKILL(ch, i);
        auto bonus = GET_SKILL_BONUS(ch, i);
        if (sklevel == 0 && bonus == 0) {
            continue;
        }
        if (*arg) {
            if (atoi(arg) <= 0 && !strstr(spell_info[i].name, arg)) {
                continue;
            } else if (atoi(arg) > GET_SKILL(ch, i)) {
                continue;
            }
        }

        if (t & SKTYPE_LANG) {
            nlen = snprintf(buf + len, maxsz - len, "%-20s  (%s)\r\n",
                            spell_info[i].name, sk.level ? "known" : "unknown");
        } else if (t & SKTYPE_SKILL) {
            if (bonus)
                snprintf(buf2, sizeof(buf2), " (base %d + bonus %d)", sk.level,
                         bonus);
            else
                buf2[0] = 0;

            count++;
            canknow = highest_skill_value(GET_LEVEL(ch), sklevel);
            nlen = snprintf(buf + len, maxsz - len, "@y(@Y%3d@y) @W%-30s  @C%3d@D/@c%3d   %s@n%s%s\r\n",
                            count, spell_info[i].name, sklevel, canknow,
                            sk.perfs > 0 ? (sk.perfs == 1 ? "@ROver Charge" : (sk.perfs == 2 ? "@BAccurate" : "@GEfficient")) : "",
                            sk.level > 100 ? " @D(@YGrand Master@D)@n" : "",
                            buf2);
        }
        if (len + nlen >= maxsz || nlen < 0)
            break;
        len += nlen;
    }

    return len;
}

int slot_count(struct char_data *ch) {
    int i, skills = -1, fail = false;
    int punch = false, kick = false, knee = false, elbow = false, kiball = false, kiblast = false, beam = false, renzo = false, shogekiha = false;

    for (i = 1; i <= SKILL_TABLE_SIZE; i++) {
        if (GET_SKILL_BASE(ch, i) > 0) {
            switch (i) {
                case SKILL_PUNCH:
                    fail = true;
                    punch = true;
                    break;
                case SKILL_KICK:
                    fail = true;
                    kick = true;
                    break;
                case SKILL_ELBOW:
                    fail = true;
                    elbow = true;
                    break;
                case SKILL_KNEE:
                    fail = true;
                    knee = true;
                    break;
                case SKILL_KIBALL:
                    fail = true;
                    kiball = true;
                    break;
                case SKILL_KIBLAST:
                    fail = true;
                    kiblast = true;
                    break;
                case SKILL_BEAM:
                    fail = true;
                    beam = true;
                    break;
                case SKILL_SHOGEKIHA:
                    fail = true;
                    shogekiha = true;
                    break;
                case SKILL_RENZO:
                    fail = true;
                    renzo = true;
                    break;
                case SKILL_TELEPATHY:
                    if (IS_KANASSAN(ch) || IS_KAI(ch)) {
                        fail = true;
                    }
                    break;
                case SKILL_ABSORB:
                    if (IS_BIO(ch) || IS_ANDROID(ch)) {
                        fail = true;
                    }
                    break;
                case SKILL_TAILWHIP:
                    if (IS_ICER(ch)) {
                        fail = true;
                    }
                    break;
                case SKILL_SEISHOU:
                    if (IS_ARLIAN(ch)) {
                        fail = true;
                    }
                    break;
                case SKILL_REGENERATE:
                    if (IS_MAJIN(ch) || IS_NAMEK(ch) || IS_BIO(ch)) {
                        fail = true;
                    }
                    break;
            }
            if (fail == false) {
                skills += 1;
            }
            fail = false;
        }
    }

    if (punch == true && kick == true && elbow == true && knee == true) {
        skills += 1;
    }
    if (kiball == true && kiblast == true && beam == true && shogekiha == true && renzo == true) {
        skills += 1;
    }

    return (skills);
}

void list_skills(struct char_data *ch, char *arg) {
    const char *overflow = "\r\n**OVERFLOW**\r\n";
    size_t len = 0;
    int slots = false;
    char buf2[MAX_STRING_LENGTH];

    len = snprintf(buf2, sizeof(buf2), "You have %d practice session%s remaining.\r\n",
                   GET_PRACTICES(ch), GET_PRACTICES(ch) == 1 ? "" : "s");

    len += snprintf(buf2 + len, sizeof(buf2) - len,
                    "\r\nYou know the following skills:      @CKnown@D/@cPrac. Max@n\r\n@w-------------------------------------------------------@n\r\n");

    len += print_skills_by_type(ch, buf2 + len, sizeof(buf2) - len, SKTYPE_SKILL, arg);

    if (slots == false) {
        len += snprintf(buf2 + len, sizeof(buf2) - len, "\r\n@DSkill Slots@W: @M%d@W/@m%d", slot_count(ch),
                        GET_SLOTS(ch));
    }

    if (len >= sizeof(buf2))
        strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

    write_to_output(ch->desc, "%s", buf2);
}


int is_guild_open(struct char_data *keeper, int guild_nr, int msg) {
    char buf[200];
    *buf = 0;

    if (GM_OPEN(guild_nr) > time_info.hours &&
        GM_CLOSE(guild_nr) < time_info.hours)
        strlcpy(buf, MSG_TRAINER_NOT_OPEN, sizeof(buf));

    if (!*buf)
        return (true);
    if (msg)
        do_say(keeper, buf, cmd_tell, 0);

    return (false);
}


int is_guild_ok_char(struct char_data *keeper, struct char_data *ch, int guild_nr) {
    char buf[200];

    if (!(CAN_SEE(keeper, ch))) {
        do_say(keeper, MSG_TRAINER_NO_SEE_CH, cmd_say, 0);
        return (false);
    }


    if (GET_LEVEL(ch) < GM_MINLVL(guild_nr)) {
        snprintf(buf, sizeof(buf), "%s %s",
                 GET_NAME(ch), MSG_TRAINER_MINLVL);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }


    if ((IS_GOOD(ch) && NOTRAIN_GOOD(guild_nr)) ||
        (IS_EVIL(ch) && NOTRAIN_EVIL(guild_nr)) ||
        (IS_NEUTRAL(ch) && NOTRAIN_NEUTRAL(guild_nr))) {
        snprintf(buf, sizeof(buf), "%s %s",
                 GET_NAME(ch), MSG_TRAINER_DISLIKE_ALIGN);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }

    if (IS_NPC(ch))
        return (false);

    if ((IS_ROSHI(ch) && NOTRAIN_WIZARD(guild_nr)) ||
        (IS_PICCOLO(ch) && NOTRAIN_CLERIC(guild_nr)) ||
        (IS_KRANE(ch) && NOTRAIN_ROGUE(guild_nr)) ||
        (IS_NAIL(ch) && NOTRAIN_FIGHTER(guild_nr)) ||
        (IS_GINYU(ch) && NOTRAIN_PALADIN(guild_nr)) ||
        (IS_FRIEZA(ch) && NOTRAIN_SORCERER(guild_nr)) ||
        (IS_TAPION(ch) && NOTRAIN_DRUID(guild_nr)) ||
        (IS_ANDSIX(ch) && NOTRAIN_BARD(guild_nr)) ||
        (IS_DABURA(ch) && NOTRAIN_RANGER(guild_nr)) ||
        (IS_BARDOCK(ch) && NOTRAIN_MONK(guild_nr)) ||
        (IS_KABITO(ch) && NOTRAIN_BARBARIAN(guild_nr)) ||
        (IS_JINTO(ch) && NOTRAIN_ARCANE_ARCHER(guild_nr)) ||
        (IS_TSUNA(ch) && NOTRAIN_ARCANE_TRICKSTER(guild_nr)) ||
        (IS_KURZAK(ch) && NOTRAIN_ARCHMAGE(guild_nr))) {

        snprintf(buf, sizeof(buf), "%s %s",
                 GET_NAME(ch), MSG_TRAINER_DISLIKE_CLASS);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }

    if ((!IS_ROSHI(ch) && TRAIN_WIZARD(guild_nr)) ||
        (!IS_PICCOLO(ch) && TRAIN_CLERIC(guild_nr)) ||
        (!IS_KRANE(ch) && TRAIN_ROGUE(guild_nr)) ||
        (!IS_BARDOCK(ch) && TRAIN_MONK(guild_nr)) ||
        (!IS_GINYU(ch) && TRAIN_PALADIN(guild_nr)) ||
        (!IS_NAIL(ch) && TRAIN_FIGHTER(guild_nr)) ||
        (!IS_FRIEZA(ch) && TRAIN_SORCERER(guild_nr)) ||
        (!IS_TAPION(ch) && TRAIN_DRUID(guild_nr)) ||
        (!IS_ANDSIX(ch) && TRAIN_BARD(guild_nr)) ||
        (!IS_DABURA(ch) && TRAIN_RANGER(guild_nr)) ||
        (!IS_KABITO(ch) && TRAIN_BARBARIAN(guild_nr)) ||
        (!IS_JINTO(ch) && TRAIN_ARCANE_ARCHER(guild_nr)) ||
        (!IS_TSUNA(ch) && TRAIN_ARCANE_TRICKSTER(guild_nr)) ||
        (!IS_KURZAK(ch) && TRAIN_ARCHMAGE(guild_nr))) {
        snprintf(buf, sizeof(buf), "%s %s",
                 GET_NAME(ch), MSG_TRAINER_DISLIKE_CLASS);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }

    if ((IS_HUMAN(ch) && NOTRAIN_HUMAN(guild_nr)) ||
        (IS_SAIYAN(ch) && NOTRAIN_SAIYAN(guild_nr)) ||
        (IS_ICER(ch) && NOTRAIN_ICER(guild_nr)) ||
        (IS_KONATSU(ch) && NOTRAIN_KONATSU(guild_nr)) ||
        (IS_NAMEK(ch) && NOTRAIN_NAMEK(guild_nr)) ||
        (IS_MUTANT(ch) && NOTRAIN_MUTANT(guild_nr)) ||
        (IS_KANASSAN(ch) && NOTRAIN_KANASSAN(guild_nr)) ||
        (IS_ANDROID(ch) && NOTRAIN_ANDROID(guild_nr)) ||
        (IS_BIO(ch) && NOTRAIN_BIO(guild_nr)) ||
        (IS_DEMON(ch) && NOTRAIN_DEMON(guild_nr)) ||
        (IS_MAJIN(ch) && NOTRAIN_MAJIN(guild_nr)) ||
        (IS_KAI(ch) && NOTRAIN_KAI(guild_nr)) ||
        (IS_TRUFFLE(ch) && NOTRAIN_TRUFFLE(guild_nr)) ||
        (IS_HOSHIJIN(ch) && NOTRAIN_GOBLIN(guild_nr)) ||
        (IS_ANIMAL(ch) && NOTRAIN_ANIMAL(guild_nr)) ||
        (IS_SAIBA(ch) && NOTRAIN_ORC(guild_nr)) ||
        (IS_SERPENT(ch) && NOTRAIN_SNAKE(guild_nr)) ||
        (IS_OGRE(ch) && NOTRAIN_TROLL(guild_nr)) ||
        (IS_HALFBREED(ch) && NOTRAIN_HALFBREED(guild_nr)) ||
        (IS_YARDRATIAN(ch) && NOTRAIN_MINOTAUR(guild_nr)) ||
        (IS_ARLIAN(ch) && NOTRAIN_KOBOLD(guild_nr))) {
        snprintf(buf, sizeof(buf), "%s %s",
                 GET_NAME(ch), MSG_TRAINER_DISLIKE_RACE);
        do_tell(keeper, buf, cmd_tell, 0);
        return (false);
    }
    return (true);
}


int is_guild_ok(struct char_data *keeper, struct char_data *ch, int guild_nr) {
    if (is_guild_open(keeper, guild_nr, true))
        return (is_guild_ok_char(keeper, ch, guild_nr));

    return (false);
}


int does_guild_know(int guild_nr, int i) {
    return guild_index[guild_nr].skills.count(i);
}

int does_guild_know_feat(int guild_nr, int i) {
    return guild_index[guild_nr].feats.count(i);
}


void sort_spells() {
    int a;

    /* initialize array, avoiding reserved. */
    for (a = 1; a < SKILL_TABLE_SIZE; a++)
        spell_sort_info[a] = a;

    qsort(&spell_sort_info[1], SKILL_TABLE_SIZE, sizeof(int), compare_spells);
}


/* this and list skills should probally be combined.  perhaps in the
 * next release?  */
void what_does_guild_know(int guild_nr, struct char_data *ch) {
    const char *overflow = "\r\n**OVERFLOW**\r\n";
    char buf2[MAX_STRING_LENGTH];
    int i, sortpos, canknow, j, k, count = 0, cost = 0;
    size_t nlen = 0, len = 0;

    len = snprintf(buf2, sizeof(buf2), "You have %d practice session%s remaining.\r\n",
                   GET_PRACTICES(ch), GET_PRACTICES(ch) == 1 ? "" : "s"); // ???

    nlen = snprintf(buf2 + len, sizeof(buf2) - len,
                    "You can practice these skills:     @CKnown@D/@cPrac. Max    @GPS Cost@n\r\n@w-------------------------------------------------------------@n\r\n");
    len += nlen;

    /* Need to check if trainer can train doesnt do it now ??? */
    for (sortpos = 0; sortpos < SKILL_TABLE_SIZE; sortpos++) {
        i = sortpos; /* spell_sort_info[sortpos]; */
        if (does_guild_know(guild_nr, i) && skill_type(i) == SKTYPE_SKILL) {
            canknow = highest_skill_value(GET_LEVEL(ch), k);
            count++;
            cost = calculate_skill_cost(ch, i);
            if (k == SKLEARN_CLASS) {
                if (GET_SKILL_BASE(ch, i) < canknow) {
                    nlen = snprintf(buf2 + len, sizeof(buf2) - len,
                                    "@y(@Y%2d@y) @W%-30s @y(@Y%2d@y) @C%d@D/@c%3d        @g%d%s@n\r\n", count,
                                    spell_info[i].name, count, GET_SKILL_BASE(ch, i), canknow, cost,
                                    GET_SKILL_BASE(ch, i) > 100 ? "  @D(@YGrand Master@D)@n" : "");
                    if (len + nlen >= sizeof(buf2) || nlen < 0)
                        break;
                    len += nlen;
                } else {
                    nlen = snprintf(buf2 + len, sizeof(buf2) - len,
                                    "@y(@Y%2d@y) @W%-30s @y(@Y%2d@y) @C%d@D/@c%3d        @g%d%s@n\r\n", count,
                                    spell_info[i].name, count, GET_SKILL_BASE(ch, i), canknow, cost,
                                    GET_SKILL_BASE(ch, i) > 100 ? "  @D(@YGrand Master@D)@n" : "");
                    if (len + nlen >= sizeof(buf2) || nlen < 0)
                        break;
                    len += nlen;
                }
            } else {
                nlen = snprintf(buf2 + len, sizeof(buf2) - len,
                                "@y(@Y%2d@y) @W%-30s @y(@Y%2d@y) @C%d@D/@c%3d        @g%d%s@n\r\n", count,
                                spell_info[i].name, count, GET_SKILL_BASE(ch, i), canknow, cost,
                                GET_SKILL_BASE(ch, i) > 100 ? "  @D(@YGrand Master@D)@n" : "");
                if (len + nlen >= sizeof(buf2) || nlen < 0)
                    break;
                len += nlen;
            }
        }
    }

    for (sortpos = 1; sortpos <= NUM_FEATS_DEFINED; sortpos++) {
        i = feat_sort_info[sortpos];
        if (does_guild_know_feat(guild_nr, i) && feat_is_available(ch, i, 0, nullptr) && feat_list[i].in_game &&
            feat_list[i].can_learn) {
            nlen = snprintf(buf2 + len, sizeof(buf2) - len, "@b%-20s@n\r\n", feat_list[i].name);
            if (len + nlen >= sizeof(buf2) || nlen < 0)
                break;
            len += nlen;
        }
    }

    if (CONFIG_ENABLE_LANGUAGES) {
        len += snprintf(buf2 + len, sizeof(buf2) - len, "\r\nand the following languages:\r\n");

        for (sortpos = 0; sortpos < SKILL_TABLE_SIZE; sortpos++) {
            i = sortpos; /* spell_sort_info[sortpos]; */
            if (does_guild_know(guild_nr, i) && IS_SET(skill_type(i), SKTYPE_LANG)) {
                //if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
                for (canknow = 0, j = 0; j < NUM_CLASSES; j++)
                    if (spell_info[i].can_learn_skill[j] > canknow)
                        canknow = spell_info[i].can_learn_skill[j];
                canknow = highest_skill_value(GET_LEVEL(ch), canknow);
                if (GET_SKILL_BASE(ch, i) < canknow) {
                    nlen = snprintf(buf2 + len, sizeof(buf2) - len, "%-20s %s\r\n", spell_info[i].name,
                                    GET_SKILL_BASE(ch, i) ? "known" : "unknown");
                    if (len + nlen >= sizeof(buf2) || nlen < 0)
                        break;
                    len += nlen;
                }
                //}
            }
        }
    }
    if (len >= sizeof(buf2))
        strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

    write_to_output(ch->desc, buf2);
}

int prereq_pass(struct char_data *ch, int snum) {
    if (snum == SKILL_KOUSENGAN || snum == SKILL_TSUIHIDAN || snum == SKILL_RENZO || snum == SKILL_SHOGEKIHA) {
        if (GET_SKILL_BASE(ch, SKILL_KIBALL) < 40 || GET_SKILL_BASE(ch, SKILL_KIBLAST) < 40 ||
            GET_SKILL_BASE(ch, SKILL_BEAM) < 40) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Kiball, Kiblast, and Beam to Skill LVL 40.");
            return 0;
        }
    } else if (snum == SKILL_INSTANTT) {
        if (GET_SKILL_BASE(ch, SKILL_FOCUS) < 90 || GET_SKILL_BASE(ch, SKILL_CONCENTRATION) < 90 ||
            GET_SKILL_BASE(ch, SKILL_ZANZOKEN) < 90) {
            send_to_char(ch,
                         "You can not train instant transmission until you have Focus, Concentration, and Zanzoken up to Skill LVL 90.");
            return 0;
        }
    } else if (snum == SKILL_SLAM) {
        if (GET_SKILL_BASE(ch, SKILL_UPPERCUT) < 50) {
            send_to_char(ch, "You can not train that skill until you at least have trained uppercut to Skill LVL 50.");
            return 0;
        }
    } else if (snum == SKILL_UPPERCUT) {
        if (GET_SKILL_BASE(ch, SKILL_ELBOW) < 40) {
            send_to_char(ch, "You can not train that skill until you at least have trained elbow to Skill LVL 40.");
            return 0;
        }
    } else if (snum == SKILL_HEELDROP) {
        if (GET_SKILL_BASE(ch, SKILL_ROUNDHOUSE) < 50) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained roundhouse to Skill LVL 50.");
            return 0;
        }
    } else if (snum == SKILL_ROUNDHOUSE) {
        if (GET_SKILL_BASE(ch, SKILL_KNEE) < 40) {
            send_to_char(ch, "You can not train that skill until you at least have trained knee to Skill LVL 40.");
            return 0;
        }
    } else if (snum == SKILL_KIBALL || snum == SKILL_KIBLAST || snum == SKILL_BEAM) {
        if (GET_SKILL_BASE(ch, SKILL_FOCUS) < 30) {
            send_to_char(ch, "You can not train that skill until you at least have trained focus to Skill LVL 30.");
            return 0;
        }
    } else if (IS_SET(spell_info[snum].flags, SKFLAG_TIER2) || IS_SET(spell_info[snum].flags, SKFLAG_TIER3)) {
        if (snum != 530 && snum != 531) {
            if (GET_SKILL_BASE(ch, SKILL_TSUIHIDAN) < 40 || GET_SKILL_BASE(ch, SKILL_RENZO) < 40 ||
                GET_SKILL_BASE(ch, SKILL_SHOGEKIHA) < 40) {
                send_to_char(ch,
                             "You can not train that skill until you at least have trained Tsuihidan, Renzokou Energy Dan, and Shogekiha to Skill LVL 40.");
                return 0;
            }
        }
    } else if (IS_SET(spell_info[snum].flags, SKFLAG_TIER4)) {
        if (IS_ROSHI(ch) && (GET_SKILL_BASE(ch, SKILL_KAMEHAMEHA) < 40 || GET_SKILL_BASE(ch, SKILL_KIENZAN) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Kamehameha and Kienzan to Skill LVL 40.");
            return 0;
        }
        if (IS_TSUNA(ch) && (GET_SKILL_BASE(ch, SKILL_WRAZOR) < 40 || GET_SKILL_BASE(ch, SKILL_WSPIKE) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Water Razor and Water Spikes to Skill LVL 40.");
            return 0;
        }
        if (IS_PICCOLO(ch) && (GET_SKILL_BASE(ch, SKILL_MASENKO) < 40 || GET_SKILL_BASE(ch, SKILL_SBC) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Masenko and Special Beam Cannon to Skill LVL 40.");
            return 0;
        }
        if (IS_FRIEZA(ch) && (GET_SKILL_BASE(ch, SKILL_DEATHBEAM) < 40 || GET_SKILL_BASE(ch, SKILL_KIENZAN) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Deathbeam and Kienzan to Skill LVL 40.");
            return 0;
        }
        if (IS_GINYU(ch) && (GET_SKILL_BASE(ch, SKILL_CRUSHER) < 40 || GET_SKILL_BASE(ch, SKILL_ERASER) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Crusher Ball and Eraser Cannon to Skill LVL 40.");
            return 0;
        }
        if (IS_BARDOCK(ch) && (GET_SKILL_BASE(ch, SKILL_GALIKGUN) < 40 || GET_SKILL_BASE(ch, SKILL_FINALFLASH) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Galik Gun and Final Flash to Skill LVL 40.");
            return 0;
        }
        if (IS_TAPION(ch) && (GET_SKILL_BASE(ch, SKILL_TSLASH) < 40 || GET_SKILL_BASE(ch, SKILL_DDSLASH) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Twin Slash and Darkness Dragon Slash to Skill LVL 40.");
            return 0;
        }
        if (IS_NAIL(ch) && (GET_SKILL_BASE(ch, SKILL_MASENKO) < 40 || GET_SKILL_BASE(ch, SKILL_KOUSENGAN) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Masenko and Kousengan to Skill LVL 40.");
            return 0;
        }
        if (IS_ANDROID(ch) && (GET_SKILL_BASE(ch, SKILL_DUALBEAM) < 40 || GET_SKILL_BASE(ch, SKILL_HELLFLASH) < 40)) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Dual Beam and Hell Flash to Skill LVL 40.");
            return 0;
        }
        if (IS_JINTO(ch) && GET_SKILL_BASE(ch, SKILL_BREAKER) < 40) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained Star Breaker to Skill LVL 40.");
            return 0;
        }
    } else if (IS_SET(spell_info[snum].flags, SKFLAG_TIER5)) {
        if (GET_SKILL_BASE(ch, SKILL_FOCUS) < 60 || GET_SKILL_BASE(ch, SKILL_CONCENTRATION) < 80) {
            send_to_char(ch,
                         "You can not train that skill until you at least have trained focus to Skill LVL 60 and concentration to Skill LVL 80.");
            return 0;
        }
    }
    return 1;
}


void handle_forget(struct char_data *keeper, int guild_nr, struct char_data *ch, char *argument) {

    int skill_num;

    skip_spaces(&argument);

    if (!*argument) {
        send_to_char(ch, "What skill do you want to start to forget?\r\n");
        return;
    }

    skill_num = find_skill_num(argument, SKTYPE_SKILL);

    if (GET_SKILL_BASE(ch, skill_num) > 30) {
        send_to_char(ch, "@MYou can not forget that skill, you know too much about it.@n\r\n");
        return;
    } else if (skill_num == SKILL_MIMIC && ch->mimic) {
        send_to_char(ch, "@MYou can not forget mimic while you are using it!\r\n");
    } else if (skill_num == SKILL_FOCUS) {
        send_to_char(ch, "@MYou can not forget such a fundamental skill!@n\r\n");
    } else if (GET_SKILL_BASE(ch, skill_num) <= 0) {
        send_to_char(ch, "@MYou can not forget a skill you don't know!@n\r\n");
    } else if (GET_FORGETING(ch) == skill_num) {
        send_to_char(ch, "@MYou stop forgetting %s@n\r\n", spell_info[skill_num].name);
        GET_FORGET_COUNT(ch) = 0;
        GET_FORGETING(ch) = 0;
    } else if (GET_FORGETING(ch) != 0) {
        send_to_char(ch, "@MYou stop forgetting %s, and start trying to forget %s.@n\r\n",
                     spell_info[GET_FORGETING(ch)].name, spell_info[skill_num].name);
        GET_FORGET_COUNT(ch) = 0;
        GET_FORGETING(ch) = skill_num;
    } else {
        send_to_char(ch, "@MYou start trying to forget %s.@n\r\n", spell_info[skill_num].name);
        GET_FORGET_COUNT(ch) = 0;
        GET_FORGETING(ch) = skill_num;
    }

}

void handle_grand(struct char_data *keeper, int guild_nr, struct char_data *ch, char *argument) {

    int skill_num;

    skip_spaces(&argument);

    if (!CAN_GRAND_MASTER(ch)) {
        send_to_char(ch, "Your race can not become a Grand Master in a skill through this process.\r\n");
        return;
    }

    if (!*argument) {
        send_to_char(ch, "What skill do you want to become a Grand Master in?");
        return;
    }

    skill_num = find_skill_num(argument, SKTYPE_SKILL);
    char buf[MAX_STRING_LENGTH];

    if (!(does_guild_know(guild_nr, skill_num))) {
        snprintf(buf, sizeof(buf), guild_index[guild_nr].no_such_skill.c_str(), GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }


    if (GET_SKILL_BASE(ch, skill_num) <= 0) {
        send_to_char(ch, "You do not know that skill!\r\n");
        return;
    } else if (GET_SKILL_BASE(ch, skill_num) < 100) {
        send_to_char(ch, "You haven't even mastered that skill. How can you become a Grand Master in it?\r\n");
        return;
    } else if (GET_SKILL_BASE(ch, skill_num) >= 103) {
        send_to_char(ch,
                     "You have already become a Grand Master in that skill and have progessed as far as possible in it.\r\n");
        return;
    } else if (GET_PRACTICES(ch) < 1000) {
        send_to_char(ch, "You need at least 1,000 practice sessions to rank up beyond 100 in a skill.\r\n");
        return;
    } else {
        if (GET_SKILL_BASE(ch, skill_num) == 100) {
            send_to_char(ch, "@YYou have ascended to Grand Master in the skill, @C%s@Y.\r\n",
                         spell_info[skill_num].name);
        } else {
            send_to_char(ch, "@YYou have ranked up in your Grand Mastery of the skill, @C%s@Y.\r\n",
                         spell_info[skill_num].name);
        }
        SET_SKILL(ch, skill_num, GET_SKILL_BASE(ch, skill_num) + 1);
        ch->modPractices(-1000);
    }

}

void handle_practice(struct char_data *keeper, int guild_nr, struct char_data *ch, char *argument) {
    //int percent = GET_SKILL(ch, skill);
    int skill_num, learntype, pointcost, highest, i;
    char buf[MAX_STRING_LENGTH];

    skip_spaces(&argument);

    if (!*argument) {
        what_does_guild_know(guild_nr, ch);
        return;
    }

    if (GET_PRACTICES(ch) <= 0) {
        send_to_char(ch, "You do not seem to be able to practice now.\r\n");
        return;
    }

    if (AFF_FLAGGED(ch, AFF_SHOCKED)) {
        send_to_char(ch, "You can not practice while your mind is shocked!\r\n");
        return;
    }

    skill_num = find_skill_num(argument, SKTYPE_SKILL);

    if (strstr(sensei::getStyle(ch->chclass).c_str(), argument)) {
        skill_num = 539;
    }

    if (skill_num == GET_FORGETING(ch)) {
        send_to_char(ch, "You can't practice that! You are trying to forget it!@n\r\n");
        return;
    }

    /****  Does the GM know the skill the player wants to learn?  ****/
    if (!(does_guild_know(guild_nr, skill_num))) {
        snprintf(buf, sizeof(buf), guild_index[guild_nr].no_such_skill.c_str(), GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
        return;
    }

    /**** Can the player learn the skill if the GM knows it?  ****/
    if (IS_SET(spell_info[skill_num].skilltype, SKTYPE_SKILL)) {
        for (learntype = 0, i = 0; i < NUM_CLASSES; i++)
            if (spell_info[skill_num].can_learn_skill[i] > learntype)
                learntype = spell_info[skill_num].can_learn_skill[i];
        switch (learntype) {
            case SKLEARN_CANT:
                snprintf(buf, sizeof(buf), guild_index[guild_nr].no_such_skill.c_str(), GET_NAME(ch));
                do_tell(keeper, buf, cmd_tell, 0);
                return;
            case SKLEARN_CROSSCLASS:
            case SKLEARN_CLASS:
                highest = highest_skill_value(GET_LEVEL(ch), learntype);
                break;
            default:
                basic_mud_log("Unknown SKLEARN type for skill %d in practice", skill_num);
                send_to_char(ch, "You can't learn that.\r\n");
                return;
        }
        pointcost = calculate_skill_cost(ch, skill_num);
        if (GET_PRACTICES(ch) >= pointcost) {
            if (!prereq_pass(ch, skill_num)) {
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= highest) {
                send_to_char(ch, "You cannot increase that skill again until you progress further.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 75 && GET_BONUS(ch, BONUS_MASOCHISTIC) > 0) {
                if (skill_num == SKILL_PARRY || skill_num == SKILL_ZANZOKEN || skill_num == SKILL_DODGE ||
                    skill_num == SKILL_BARRIER || skill_num == SKILL_BLOCK || skill_num == SKILL_TSKIN) {
                    send_to_char(ch,
                                 "You cannot increase that skill again because it would deny you the pain you enjoy.\r\n");
                    return;
                }
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 75 && IS_TAPION(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 75 && IS_DABURA(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 75 && IS_JINTO(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 75 && IS_TSUNA(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 50 && IS_FRIEZA(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 50 && IS_ANDSIX(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 50 && IS_KURZAK(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 50 && IS_GINYU(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 50 && IS_BARDOCK(ch) && skill_num == SKILL_SENSE) {
                send_to_char(ch, "You cannot practice that anymore.\r\n");
                return;
            }
            if (GET_SKILL_BASE(ch, skill_num) >= 100) {
                send_to_char(ch, "You know everything about that skill.\r\n");
                return;
            } else {
                if (GET_SKILL_BASE(ch, skill_num) == 0) {
                    if (slot_count(ch) < GET_SLOTS(ch)) {
                        if (skill_num != 539)
                            send_to_char(ch, "You practice and master the basics!\r\n");
                        else
                            send_to_char(ch, "You practice the basics of %s\r\n", sensei::getStyle(ch->chclass));
                        SET_SKILL(ch, skill_num, GET_SKILL_BASE(ch, skill_num) + rand_number(10, 25));
                        ch->modPractices(-pointcost);
                        if (GET_FORGETING(ch) != 0 && GET_SKILL_BASE(ch, GET_FORGETING(ch)) < 30) {
                            GET_FORGET_COUNT(ch) += 1;
                            if (GET_FORGET_COUNT(ch) >= 5) {
                                SET_SKILL(ch, GET_FORGETING(ch), 0);
                                send_to_char(ch, "@MYou have finally forgotten what little you knew of %s@n\r\n",
                                             spell_info[GET_FORGETING(ch)].name);
                                GET_FORGETING(ch) = 0;
                                GET_FORGET_COUNT(ch) = 0;
                                ch->save();
                            }
                        } else if (GET_SKILL_BASE(ch, GET_FORGETING(ch)) < 30) {
                            GET_FORGETING(ch) = 0;
                        }
                    } else {
                        send_to_char(ch,
                                     "You already know the maximum number of skills you can for the time being!\r\n");
                        return;
                    }
                } else {
                    if (skill_num != 539)
                        send_to_char(ch, "You practice for a while and manage to advance your technique. +1 (%d/%d)\r\n",
                                     GET_SKILL_BASE(ch, skill_num) + 1, highest);
                    else
                        send_to_char(ch, "You practice the basics of %s. +1 (%d/%d)\r\n", sensei::getStyle(ch->chclass),
                                     GET_SKILL_BASE(ch, skill_num) + 1, highest);
                    SET_SKILL(ch, skill_num, GET_SKILL_BASE(ch, skill_num) + 1);
                    ch->modPractices(-pointcost);
                    if (GET_SKILL_BASE(ch, skill_num) >= 100) {
                        send_to_char(ch, "You learned a lot by mastering that skill.\r\n");
                        if (IS_KONATSU(ch) && skill_num == SKILL_PARRY) {
                            SET_SKILL(ch, skill_num, GET_SKILL_BASE(ch, skill_num) + 5);
                        }
                        int64_t gain = level_exp(ch, GET_LEVEL(ch) + 1) / 20;
                        ch->modExperience(gain);
                    }
                    if (GET_FORGETING(ch) != 0) {
                        GET_FORGET_COUNT(ch) += 1;
                        if (GET_FORGET_COUNT(ch) >= 5) {
                            SET_SKILL(ch, GET_FORGETING(ch), 0);
                            send_to_char(ch, "@MYou have finally forgotten what little you knew of %s@n\r\n",
                                         spell_info[GET_FORGETING(ch)].name);
                            GET_FORGETING(ch) = 0;
                            GET_FORGET_COUNT(ch) = 0;
                            ch->save();
                        }
                    }
                }
            }
        } else {
            send_to_char(ch, "You need %d practice session%s to increase that skill.\r\n",
                         pointcost, (pointcost == 1) ? "" : "s");
        }
    } else {
        snprintf(buf, sizeof(buf), guild_index[guild_nr].no_such_skill.c_str(), GET_NAME(ch));
        do_tell(keeper, buf, cmd_tell, 0);
    }
}


void handle_train(struct char_data *keeper, int guild_nr, struct char_data *ch, char *argument) {

}


void handle_gain(struct char_data *keeper, int guild_nr, struct char_data *ch, char *argument) {
    skip_spaces(&argument);
    auto rpp_cost = rpp_to_level(ch);

    if (GET_LEVEL(ch) < 100 && GET_EXP(ch) >= level_exp(ch, GET_LEVEL(ch) + 1)) {
        if (GET_RP(ch) < rpp_cost) {
            send_to_char(ch, "You need at least %d RPP to gain the next level.\r\n", rpp_cost);
        } else if (rpp_cost <= GET_RP(ch)) {
            ch->modRPP(-rpp_cost);
            send_to_char(ch, "@D(@cRPP@W: @w-%d@D)@n\n\n", rpp_cost);
            gain_level(ch);
        } else {
            gain_level(ch);
        }
    } else {
        send_to_char(ch, "You are not yet ready for further advancement.\r\n");
    }
}

int rpp_to_level(struct char_data *ch) {

    switch (GET_LEVEL(ch)) {
        case 2:
            // charge the RPP races to level for the first time.
            return 0;
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
            return 3;
        case 96:
        case 97:
        case 98:
            return 4;
        case 99:
            return 5;
        default:
            return 0;
    }
}

void handle_study(struct char_data *keeper, int guild_nr, struct char_data *ch, char *argument) {

    int expcost = 25000, goldcost = 750, fail = false, reward = 25, goldadjust = 0, expadjust = 0;

    if (GET_LEVEL(ch) >= 100) {
        goldadjust = 500;
        //expadjust = 15000;
        expcost = 0;
    } else if (GET_LEVEL(ch) >= 91) {
        goldadjust = 450;
        expadjust = 12500;
    } else if (GET_LEVEL(ch) >= 81) {
        goldadjust = 400;
        expadjust = 10000;
    } else if (GET_LEVEL(ch) >= 71) {
        goldadjust = 350;
        expadjust = 7500;
    } else if (GET_LEVEL(ch) >= 61) {
        goldadjust = 300;
        expadjust = 5000;
    } else if (GET_LEVEL(ch) >= 51) {
        goldadjust = 250;
        expadjust = 2500;
    } else if (GET_LEVEL(ch) >= 41) {
        goldadjust = 200;
    } else if (GET_LEVEL(ch) >= 31) {
        goldadjust = 150;
    } else if (GET_LEVEL(ch) >= 21) {
        goldadjust = 100;
    } else if (GET_LEVEL(ch) >= 11) {
        goldadjust = 50;
    }

    goldcost += goldadjust;
    expcost += expadjust;

    if (GET_EXP(ch) < expcost) {
        send_to_char(ch, "You do not have enough experience to study. @D[@wCost@W: @G%s@D]@n\r\n", add_commas(expcost).c_str());
        fail = true;
    }

    if (GET_GOLD(ch) < goldcost) {
        send_to_char(ch, "You do not have enough zenni to study. @D[@wCost@W: @Y%s@D]@n\r\n", add_commas(goldcost).c_str());
        fail = true;
    }

    if (fail == true)
        return;

    ch->modExperience(-expcost);
    ch->mod(CharMoney::Carried, -goldcost);
    ch->modPractices(25);

    act("@c$N@W spends time lecturing you on various subjects.@n", true, ch, nullptr, keeper, TO_CHAR);
    act("@c$N@W spends time lecturing @C$n@W on various subjects.@n", true, ch, nullptr, keeper, TO_ROOM);
    send_to_char(ch, "@wYou have gained %d practice sessions in exchange for %s EXP and %s zenni.\r\n", reward,
                 add_commas(expcost).c_str(), add_commas(goldcost).c_str());

}

SPECIAL(guild) {
    char arg[MAX_INPUT_LENGTH];
    int guild_nr, i;
    struct char_data *keeper = (struct char_data *) me;
    struct {
        const char *cmd;

        void (*func)(struct char_data *, int, struct char_data *, char *);
    } guild_cmd_tab[] = {
            {"practice", handle_practice},
            {"gain",     handle_gain},
            {"forget",   handle_forget},
            {"study",    handle_study},
            {"grand",    handle_grand},
            {nullptr,    nullptr}
    };

    for (auto &[vn, g] : guild_index)
        if (g.gm == keeper->vn) {
            guild_nr = vn;
            break;
        }

    if (!guild_index.contains(guild_nr))
        return (false);

    if (GM_FUNC(guild_nr))
        if ((GM_FUNC(guild_nr))(ch, me, cmd, arg))
            return (true);

    /*** Is the GM able to train?    ****/
    if (!AWAKE(keeper))
        return (false);

    for (i = 0; guild_cmd_tab[i].cmd; i++)
        if (CMD_IS(guild_cmd_tab[i].cmd))
            break;

    if (!guild_cmd_tab[i].cmd)
        return (false);

    if (!(is_guild_ok(keeper, ch, guild_nr)))
        return (true);

    (guild_cmd_tab[i].func)(keeper, guild_nr, ch, argument);

    return (true);
}


/****  This is ripped off of read_line from shop.c.  They could be
 *  combined. But why? ****/

void read_guild_line(FILE *gm_f, char *string, void *data, char *type) {
    char buf[MAX_STRING_LENGTH];

    if (!get_line(gm_f, buf) || !sscanf(buf, string, data)) {
        fprintf(stderr, "Error in guild #%d, Could not get %s\n", GM_NUM(top_guild), type);
        exit(1);
    }
}


void boot_the_guilds(FILE *gm_f, char *filename, int rec_count) {
    char *buf, buf2[256], *p, buf3[READ_SIZE];
    int temp, val, t1, t2, rv;
    int done = false;

    snprintf(buf2, sizeof(buf2), "beginning of GM file %s", filename);

    buf = fread_string(gm_f, buf2);
    while (!done) {
        if (*buf == '#') {        /* New Trainer */
            sscanf(buf, "#%d\n", &temp);
            snprintf(buf2, sizeof(buf2), "GM #%d in GM file %s", temp, filename);
            free(buf);        /* Plug memory leak! */
            top_guild = temp;
            auto &g = guild_index[temp];
            auto &z = zone_table[real_zone_by_thing(temp)];
            z.guilds.insert(temp);

            GM_NUM(top_guild) = temp;
            
            get_line(gm_f, buf3);
            rv = sscanf(buf3, "%d %d", &t1, &t2);
            while (t1 > -1) {
                if (rv == 1) { /* old style guilds, only skills */
                    g.skills.insert(t1);
                } else if (rv == 2) { /* new style guilds, skills and feats */
                    if (t2 == 1) {
                        g.skills.insert(t1);
                    } else if (t2 == 2) {
                        g.feats.insert(t1);
                    } else {
                        basic_mud_log("SYSERR: Invalid 2nd arg in guild file!");
                        exit(1);
                    }
                } else {
                    basic_mud_log("SYSERR: Invalid format in guild file. Expecting 2 args but got %d!", rv);
                    exit(1);
                }
                get_line(gm_f, buf3);
                rv = sscanf(buf3, "%d %d", &t1, &t2);
            }
            read_guild_line(gm_f, "%f", &g.charge, "GM_CHARGE");
            g.no_such_skill = fread_string(gm_f, buf2);
            g.not_enough_gold = fread_string(gm_f, buf2);

            read_guild_line(gm_f, "%d", &g.minlvl, "GM_MINLVL");
            read_guild_line(gm_f, "%d", &g.gm, "GM_TRAINER");

            g.gm = real_mobile(g.gm);
            read_guild_line(gm_f, "%d", &g.with_who[0], "GM_WITH_WHO");

            read_guild_line(gm_f, "%d", &g.open, "GM_OPEN");
            read_guild_line(gm_f, "%d", &g.close, "GM_CLOSE");

            CREATE(buf, char, READ_SIZE);
            get_line(gm_f, buf);
            if (buf && *buf != '#' && *buf != '$') {
                p = buf;
                for (temp = 1; temp < GW_ARRAY_MAX; temp++) {
                    if (!p || !*p)
                        break;
                    if (sscanf(p, "%d", &val) != 1) {
                        basic_mud_log("SYSERR: Can't parse GM_WITH_WHO line in %s: '%s'", buf2, buf);
                        break;
                    }
                    g.with_who[temp] = val;
                    while (isdigit(*p) || *p == '-') {
                        p++;
                    }
                    while (*p && !(isdigit(*p) || *p == '-')) {
                        p++;
                    }
                }
                free(buf);
                buf = fread_string(gm_f, buf2);
            }
        } else {
            if (*buf == '$')        /* EOF */
                done = true;
            free(buf);        /* Plug memory leak! */
        }
    }
}


void assign_the_guilds() {
    guild_vnum gdindex;

    cmd_say = find_command("say");
    cmd_tell = find_command("tell");

    for (auto &[vn, g] : guild_index) {
        if (g.gm == NOBODY)
            continue;

        auto &gm = mob_index[g.gm];
        auto &p = mob_proto[g.gm];

        if (gm.func && gm.func != guild)
            g.func = gm.func;

        gm.func = guild;
    }
}

char *guild_customer_string(int guild_nr, int detailed) {
    int gindex = 0, flag = 0, nlen;
    size_t len = 0;
    static char buf[MAX_STRING_LENGTH];

    while (*trade_letters[gindex] != '\n' && len + 1 < sizeof(buf)) {
        if (detailed) {
            if (!IS_SET_AR(GM_WITH_WHO(guild_nr), flag)) {
                nlen = snprintf(buf + len, sizeof(buf) - len, ", %s", trade_letters[gindex]);

                if (len + nlen >= sizeof(buf) || nlen < 0)
                    break;

                len += nlen;
            }
        } else {
            buf[len++] = (IS_SET_AR(GM_WITH_WHO(guild_nr), flag) ? '_' : *trade_letters[gindex]);
            buf[len] = '\0';

            if (len >= sizeof(buf))
                break;
        }

        gindex++;
        flag += 1;
    }

    buf[sizeof(buf) - 1] = '\0';
    return (buf);
}

void list_all_guilds(struct char_data *ch) {
    const char *list_all_guilds_header =
            "Virtual   G.Master	Charge   Members\r\n"
            "----------------------------------------------------------------------\r\n";
    int headerlen = strlen(list_all_guilds_header);
    size_t len = 0;
    char buf[MAX_STRING_LENGTH], buf1[16];

    *buf = '\0';
    int counter = 0;
    for (auto &[gm_nr, g] : guild_index) {
        /* New page in page_string() mechanism, print the header again. */
        if (!(counter++ % (PAGE_LENGTH - 2))) {
            /*
             * If we don't have enough room for the header, or all we have room left
             * for is the header, then don't add it and just quit now.
             */
            if (len + headerlen + 1 >= sizeof(buf))
                break;
            strcpy(buf + len, list_all_guilds_header);    /* strcpy: OK (length checked above) */
            len += headerlen;
        }

        if (g.gm == NOBODY)
            strcpy(buf1, "<NONE>");  /* strcpy: OK (for 'buf1 >= 7') */
        else
            sprintf(buf1, "%6d", g.gm);  /* sprintf: OK (for 'buf1 >= 11', 32-bit int) */

        len += snprintf(buf + len, sizeof(buf) - len, "%6d	%s		%5.2f	%s\r\n",
                        gm_nr, buf1, g.charge, guild_customer_string(gm_nr, false));
    }

    write_to_output(ch->desc, buf);
}


void list_detailed_guild(struct char_data *ch, int gm_nr) {
    int i;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    if (GM_TRAINER(gm_nr) < NOBODY)
        strcpy(buf1, "<NONE>");
    else
        sprintf(buf1, "%6d   ", mob_index[GM_TRAINER(gm_nr)].vn);

    sprintf(buf, " Guild Master: %s\r\n", buf1);
    sprintf(buf, "%s Hours: %4d to %4d,  Surcharge: %5.2f\r\n", buf,
            GM_OPEN(gm_nr), GM_CLOSE(gm_nr), GM_CHARGE(gm_nr));
    sprintf(buf, "%s Min Level will train: %d\r\n", buf, GM_MINLVL(gm_nr));
    sprintf(buf, "%s Whom will train: %s\r\n", buf, guild_customer_string(gm_nr, true));

    /* now for the REAL reason why someone would want to see a Guild :) */

    sprintf(buf, "%s The GM can teach the following:\r\n", buf);

    *buf2 = '\0';
    for (i = 0; i < SKILL_TABLE_SIZE; i++) {
        if (does_guild_know(gm_nr, i))
            sprintf(buf2, "%s %s \r\n", buf2, spell_info[i].name);
    }

    strcat(buf, buf2);

    write_to_output(ch->desc, buf);
}


void show_guild(struct char_data *ch, char *arg) {
    vnum gm_num = NOBODY;

    if (!*arg)
        list_all_guilds(ch);
    else {
        if (is_number(arg))
            gm_num = atoi(arg);

        auto g = guild_index.find(gm_num);
        if(g == guild_index.end()) {
            send_to_char(ch, "Illegal guild master number.\r\n");
            return;
        }
        list_detailed_guild(ch, g->first);
    }
}

/*
 * List all guilds in a zone.                              
 */
void list_guilds(struct char_data *ch, zone_rnum rnum, guild_vnum vmin, guild_vnum vmax) {
    int i, bottom, top, counter = 0;

    auto glist = [&](const guild_data& g) {
        counter++;

        send_to_char(ch, "@g%4d@n) [@c%-5d@n]", counter, GM_NUM(i));

        /************************************************************************/
        /** Retrieve the list of rooms for this guild.                         **/
        /************************************************************************/

        send_to_char(ch, " @c[@y%d@c]@y %s@n",
                     (g.gm == NOBODY) ?
                     -1 : g.gm,
                     (g.gm == NOBODY) ?
                     "" : mob_proto[g.gm].short_description);

        send_to_char(ch, "\r\n");
    };

    send_to_char(ch,
                 "Index VNum    Guild Master\r\n"
                 "----- ------- ---------------------------------------------\r\n");

    if (rnum != NOWHERE) {
        auto &z = zone_table[rnum];
        for(auto vn : z.guilds) {
            auto &g = guild_index[vn];
            glist(g);
        }
    } else {
        for(auto &[vn, g] : guild_index) {
            if(vn < vmin || vn > vmax)
                continue;
            glist(g);
        }
    }

    if (counter == 0)
        send_to_char(ch, "None found.\r\n");
}

void destroy_guilds() {
    guild_index.clear();
}


void levelup_parse(struct descriptor_data *d, char *arg) {
}


void guild_data::toggle_skill(uint16_t skill_id) {
    if(skills.count(skill_id)) {
        skills.erase(skill_id);
    } else {
        skills.insert(skill_id);
    }
}

void guild_data::toggle_feat(uint16_t skill_id) {
    if(feats.count(skill_id)) {
        feats.erase(skill_id);
    } else {
        feats.insert(skill_id);
    }
}

nlohmann::json guild_data::serialize() {
    nlohmann::json j;

    j["vnum"] = vnum;
    for(auto s : skills) j["skills"].push_back(s);
    for(auto f : feats) j["feats"].push_back(f);
    if(charge != 1.0) j["charge"] = charge;
    if(!no_such_skill.empty()) j["no_such_skill"] = no_such_skill;
    if(!not_enough_gold.empty()) j["not_enough_gold"] = not_enough_gold;
    if(minlvl) j["minlvl"] = minlvl;
    if(gm != NOBODY) j["gm"] = gm;
    for(auto i = 0; i < 79; i++) if(IS_SET_AR(with_who, i)) j["with_who"].push_back(i);
    if(open) j["open"] = open;
    if(close) j["close"] = close;

    return j;
}

guild_data::guild_data(const nlohmann::json &j) : guild_data() {
    if(j.contains("vnum")) vnum = j["vnum"];
    if(j.contains("skills")) for(const auto& s : j["skills"]) skills.insert(s.get<int>());
    if(j.contains("feats")) for(const auto& f : j["feats"]) feats.insert(f.get<int>());
    if(j.count("charge")) charge = j["charge"];
    if(j.count("no_such_skill")) no_such_skill = strdup(j["no_such_skill"].get<std::string>().c_str());
    if(j.count("not_enough_gold")) not_enough_gold = strdup(j["not_enough_gold"].get<std::string>().c_str());
    if(j.count("minlvl")) minlvl = j["minlvl"];
    if(j.count("gm")) gm = j["gm"];
    if(j.contains("with_who")) for(const auto& i : j["with_who"]) SET_BIT_AR(with_who, i.get<int>());
    if(j.count("open")) open = j["open"];
    if(j.count("close")) close = j["close"];
}