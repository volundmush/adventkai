/* ************************************************************************
*   File: guild.h                                                         *
*  Usage: GuildMaster definition for everything needed by guild.c         *
*                                                                         *
* Written by Jason Goodwin.   jgoodwin@expert.cc.purdue.edu               *
************************************************************************ */
#pragma once

#include "structs.h"

#define GW_ARRAY_MAX    4

struct guild_data {
    guild_data() = default;
    explicit guild_data(const nlohmann::json& j);
    nlohmann::json serialize();
    room_vnum vnum{NOBODY};                /* number of the guild */
    void toggle_skill(uint16_t skill_id);
    void toggle_feat(uint16_t skill_id);
    std::set<uint16_t> skills;  /* array to keep track of which feats things we'll train */
    float charge{1.0};                  /* charge * skill level = how much we'll charge */
    std::string no_such_skill{};           /* message when we don't teach that skill */
    std::string not_enough_gold{};         /* message when the student doesn't have enough gold */
    int minlvl{0};                    /* Minumum level guildmaster will train */
    mob_vnum gm{NOBODY};                   /* GM's vnum */
    bitvector_t with_who[GW_ARRAY_MAX]{};    /* whom we dislike */
    int open{0}, close{28};               /* when we will train */
    SpecialFunc func{};                /* secondary spec_proc for the GM */
    std::set<uint8_t> feats;  /* array to keep track of which feats things we'll train */
};

#define GM_NUM(i)  (guild_index[i].vnum)
#define GM_TRAINER(i) (guild_index[i].gm)
#define GM_OPEN(i) (guild_index[i].open)
#define GM_CLOSE(i) (guild_index[i].close)
#define GM_CHARGE(i) (guild_index[i].charge)
#define GM_MINLVL(i) (guild_index[i].minlvl)
#define GM_WITH_WHO(i) (guild_index[i].with_who)
#define GM_NO_SKILL(i)     (guild_index[i].no_such_skill)
#define GM_NO_GOLD(i)     (guild_index[i].not_enough_gold)
#define GM_COST(i, j, k) (int) (GM_CHARGE(i)*spell_info[j].min_level[(int)GET_CLASS(k)])
#define GM_FUNC(i) (guild_index[i].func)

#define NOTRAIN_GOOD(i)        (IS_SET_AR((GM_WITH_WHO(i)),TRADE_NOGOOD))
#define NOTRAIN_EVIL(i)        (IS_SET_AR((GM_WITH_WHO(i)),TRADE_NOEVIL))
#define NOTRAIN_NEUTRAL(i)    (IS_SET_AR((GM_WITH_WHO(i)),TRADE_NONEUTRAL))

#define TRAIN_WIZARD(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYWIZARD))
#define TRAIN_CLERIC(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYCLERIC))
#define TRAIN_ROGUE(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYROGUE))
#define TRAIN_FIGHTER(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYFIGHTER))
#define TRAIN_MONK(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYMONK))
#define TRAIN_PALADIN(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYPALADIN))
#define TRAIN_SORCERER(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYSORCERER))
#define TRAIN_DRUID(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYDRUID))
#define TRAIN_BARD(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYBARD))
#define TRAIN_RANGER(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYRANGER))
#define TRAIN_BARBARIAN(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYBARBARIAN))
#define TRAIN_ARCANE_ARCHER(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYARCANE_ARCHER))
#define TRAIN_ARCANE_TRICKSTER(i) (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYARCANE_TRICKSTER))
#define TRAIN_ARCHMAGE(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYARCHMAGE))
#define TRAIN_ASSASSIN(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYASSASSIN))
#define TRAIN_BLACKGUARD(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYBLACKGUARD))
#define TRAIN_DRAGON_DISCIPLE(i)  (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYDRAGON_DISCIPLE))
#define TRAIN_DUELIST(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYDUELIST))
#define TRAIN_DWARVEN_DEFENDER(i) (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYDWARVEN_DEFENDER))
#define TRAIN_ELDRITCH_KNIGHT(i)  (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYELDRITCH_KNIGHT))
#define TRAIN_HIEROPHANT(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYHIEROPHANT))
#define TRAIN_HORIZON_WALKER(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYHORIZON_WALKER))
#define TRAIN_LOREMASTER(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYLOREMASTER))
#define TRAIN_MYSTIC_THEURGE(i)   (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYMYSTIC_THEURGE))
#define TRAIN_SHADOWDANCER(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYSHADOWDANCER))
#define TRAIN_THAUMATURGIST(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_ONLYTHAUMATURGIST))


#define NOTRAIN_WIZARD(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOWIZARD))
#define NOTRAIN_CLERIC(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOCLERIC))
#define NOTRAIN_ROGUE(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOROGUE))
#define NOTRAIN_FIGHTER(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOFIGHTER))
#define NOTRAIN_MONK(i)          (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOMONK))
#define NOTRAIN_PALADIN(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOPALADIN))
#define NOTRAIN_SORCERER(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOSORCERER))
#define NOTRAIN_DRUID(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NODRUID))
#define NOTRAIN_BARD(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOBARD))
#define NOTRAIN_RANGER(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NORANGER))
#define NOTRAIN_BARBARIAN(i)      (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOBARBARIAN))
#define NOTRAIN_ARCANE_ARCHER(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOARCANE_ARCHER))
#define NOTRAIN_ARCANE_TRICKSTER(i) (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOARCANE_TRICKSTER))
#define NOTRAIN_ARCHMAGE(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOARCHMAGE))
#define NOTRAIN_ASSASSIN(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOASSASSIN))
#define NOTRAIN_BLACKGUARD(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOBLACKGUARD))
#define NOTRAIN_DRAGON_DISCIPLE(i)  (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NODRAGON_DISCIPLE))
#define NOTRAIN_DUELIST(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NODUELIST))
#define NOTRAIN_DWARVEN_DEFENDER(i) (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NODWARVEN_DEFENDER))
#define NOTRAIN_ELDRITCH_KNIGHT(i)  (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOELDRITCH_KNIGHT))
#define NOTRAIN_HIEROPHANT(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOHIEROPHANT))
#define NOTRAIN_HORIZON_WALKER(i)   (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOHORIZON_WALKER))
#define NOTRAIN_LOREMASTER(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOLOREMASTER))
#define NOTRAIN_MYSTIC_THEURGE(i)   (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOMYSTIC_THEURGE))
#define NOTRAIN_SHADOWDANCER(i)        (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOSHADOWDANCER))
#define NOTRAIN_THAUMATURGIST(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOTHAUMATURGIST))


#define NOTRAIN_HUMAN(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOHUMAN))
#define NOTRAIN_SAIYAN(i)       (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOSAIYAN))
#define NOTRAIN_ICER(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOICER))
#define NOTRAIN_KONATSU(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOKONATSU))
#define NOTRAIN_NAMEK(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NONAMEK))
#define NOTRAIN_MUTANT(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOMUTANT))
#define NOTRAIN_KANASSAN(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOKANASSAN))
#define NOTRAIN_BIO(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOBIO))
#define NOTRAIN_ANDROID(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOANDROID))
#define NOTRAIN_DEMON(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NODEMON))
#define NOTRAIN_MAJIN(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOMAJIN))
#define NOTRAIN_KAI(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOKAI))
#define NOTRAIN_TRUFFLE(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOTRUFFLE))
#define NOTRAIN_GOBLIN(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOGOBLIN))
#define NOTRAIN_ANIMAL(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOANIMAL))
#define NOTRAIN_ORC(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOORC))
#define NOTRAIN_SNAKE(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOSNAKE))
#define NOTRAIN_TROLL(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOTROLL))
#define NOTRAIN_HALFBREED(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOHALFBREED))
#define NOTRAIN_MINOTAUR(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOMINOTAUR))
#define NOTRAIN_KOBOLD(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOKOBOLD))
#define NOTRAIN_LIZARDFOLK(i)    (IS_SET_AR((GM_WITH_WHO(i)), TRADE_NOLIZARDFOLK))


extern std::map<guild_vnum, struct guild_data> guild_index;
extern guild_vnum top_guild;
extern int spell_sort_info[SKILL_TABLE_SIZE + 1];

/* Functions defined in guild.c */
extern int print_skills_by_type(struct char_data *ch, char *buf, int maxsz, int sktype, char *argument);

extern void levelup_parse(struct descriptor_data *d, char *arg);

extern int rpp_to_level(struct char_data *ch);

extern int slot_count(struct char_data *ch);

extern void show_guild(struct char_data *ch, char *arg);

extern void handle_ingest_learn(struct char_data *ch, struct char_data *vict);

extern void list_skills(struct char_data *ch, char *arg);

/*. External . */
extern SPECIAL(guild);
