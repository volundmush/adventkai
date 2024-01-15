/************************************************************************
 * Generic OLC Library - Mobiles / genmob.c			v1.0	*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "dbat/genmob.h"
#include "dbat/utils.h"
#include "dbat/db.h"
#include "dbat/genolc.h"
#include "dbat/shop.h"
#include "dbat/genzon.h"
#include "dbat/guild.h"
#include "dbat/dg_scripts.h"
#include "dbat/handler.h"
#include "dbat/dg_olc.h"
#include "dbat/class.h"
#include "dbat/races.h"
#include "dbat/spells.h"
#include "dbat/objsave.h"
#include "dbat/players.h"
#include "dbat/account.h"

/* From db.c */
int update_mobile_strings(struct char_data *t, struct char_data *f);

void check_mobile_strings(struct char_data *mob);

void check_mobile_string(mob_vnum i, char **string, const char *dscr);

int copy_mobile_strings(struct char_data *t, struct char_data *f);

#if CONFIG_GENOLC_MOBPROG
int write_mobile_mobprog(mob_vnum mvnum, struct char_data *mob, FILE *fd);
#endif

/* local functions */
void extract_mobile_all(mob_vnum vnum);

int add_mobile(struct char_data *mob, mob_vnum vnum) {
    mob_vnum rnum, found = false;
    struct char_data *live_mob;

    if ((rnum = real_mobile(vnum)) != NOBODY) {
        /* Copy over the mobile and free() the old strings. */
        copy_mobile(&mob_proto[rnum], mob);

        /* Now re-point all existing mobile strings to here. */
        for (live_mob = character_list; live_mob; live_mob = live_mob->next)
            if (rnum == live_mob->vn)
                update_mobile_strings(live_mob, &mob_proto[rnum]);

        dirty_npc_prototypes.insert(rnum);
        basic_mud_log("GenOLC: add_mobile: Updated existing mobile #%d.", vnum);
        return rnum;
    }

    auto &m = mob_proto[vnum];
    m = *mob;

    m.vn = 0;
    copy_mobile_strings(&m, mob);
    auto &ix = mob_index[vnum];
    ix.vn = vnum;

    basic_mud_log("GenOLC: add_mobile: Added mobile %d.", vnum);

#if CONFIG_GENOLC_MOBPROG
    GET_MPROG(OLC_MOB(d)) = OLC_MPROGL(d);
    GET_MPROG_TYPE(OLC_MOB(d)) = (OLC_MPROGL(d) ? OLC_MPROGL(d)->type : 0);
    while (OLC_MPROGL(d)) {
      GET_MPROG_TYPE(OLC_MOB(d)) |= OLC_MPROGL(d)->type;
      OLC_MPROGL(d) = OLC_MPROGL(d)->next;
    }
#endif

    auto zvnum = real_zone_by_thing(vnum);
    auto &z = zone_table[zvnum];
    z.mobiles.insert(vnum);
    dirty_npc_prototypes.insert(vnum);
    return found;
}

int copy_mobile(struct char_data *to, struct char_data *from) {
    free_mobile_strings(to);
    *to = *from;
    check_mobile_strings(from);
    copy_mobile_strings(to, from);
    return true;
}

void extract_mobile_all(mob_vnum vnum) {
    struct char_data *next, *ch;

    for (ch = character_list; ch; ch = next) {
        next = ch->next;
        if (GET_MOB_VNUM(ch) == vnum)
            extract_char(ch);
    }
}

int delete_mobile(mob_rnum refpt) {
    struct char_data *live_mob;
    int counter, cmd_no;
    mob_vnum vnum;
    zone_rnum zone;

    if (!mob_proto.count(refpt)) {
        basic_mud_log("SYSERR: GenOLC: delete_mobile: Invalid rnum %d.", refpt);
        return NOBODY;
    }

    vnum = mob_index[refpt].vn;
    extract_mobile_all(vnum);
    auto &z = zone_table[real_zone_by_thing(refpt)];
    z.mobiles.erase(refpt);

    /* Update live mobile rnums.  */
    for (live_mob = character_list; live_mob; live_mob = live_mob->next)
        GET_MOB_RNUM(live_mob) -= (GET_MOB_RNUM(live_mob) >= refpt);

    /* Update zone table.  */
    for (auto &[zone, z] : zone_table) {
        z.cmd.erase(std::remove_if(z.cmd.begin(), z.cmd.end(), [refpt](auto &cmd) { return cmd.command == 'M' && cmd.arg1 == refpt; }));
    }

    /* Update shop keepers.  */
    for (auto &sh : shop_index) {
        /* Find the shop for this keeper and reset it's keeper to
         * -1 to keep the shop so it could be assigned to someone else */
        if (sh.second.keeper == refpt) {
            sh.second.keeper = NOBODY;
        }
    }

    /* Update guild masters */
    for (auto &g : guild_index) {
        /* Find the guild for this trainer and reset it's trainer to
         * -1 to keep the guild so it could be assigned to someone else */
        if (g.second.gm == refpt) {
            g.second.gm = NOBODY;
        }
    }

    dirty_npc_prototypes.insert(vnum);
    mob_proto.erase(vnum);
    mob_index.erase(vnum);
    save_mobiles(real_zone_by_thing(vnum));

    return refpt;
}

int copy_mobile_strings(struct char_data *t, struct char_data *f) {
    if (f->name)
        t->name = strdup(f->name);
    if (f->title)
        t->title = strdup(f->title);
    if (f->short_description)
        t->short_description = strdup(f->short_description);
    if (f->room_description)
        t->room_description = strdup(f->room_description);
    if (f->look_description)
        t->look_description = strdup(f->look_description);
    return true;
}

int update_mobile_strings(struct char_data *t, struct char_data *f) {
    if (f->name)
        t->name = f->name;
    if (f->title)
        t->title = f->title;
    if (f->short_description)
        t->short_description = f->short_description;
    if (f->room_description)
        t->room_description = f->room_description;
    if (f->look_description)
        t->look_description = f->look_description;
    return true;
}

int free_mobile_strings(struct char_data *mob) {
    if (mob->name)
        free(mob->name);
    if (mob->title)
        free(mob->title);
    if (mob->short_description)
        free(mob->short_description);
    if (mob->room_description)
        free(mob->room_description);
    if (mob->look_description)
        free(mob->look_description);
    return true;
}


/* Free a mobile structure that has been edited. Take care of existing mobiles 
 * and their mob_proto!  */
int free_mobile(struct char_data *mob) {
    mob_rnum i;

    if (mob == nullptr)
        return false;

    /* Non-prototyped mobile.  Also known as new mobiles.  */
    if ((i = GET_MOB_RNUM(mob)) == NOBODY) {
        free_mobile_strings(mob);
        /* free script proto list */
        free_proto_script(mob, MOB_TRIGGER);
    } else {    /* Prototyped mobile. */
        if (mob->name && mob->name != mob_proto[i].name)
            free(mob->name);
        if (mob->title && mob->title != mob_proto[i].title)
            free(mob->title);
        if (mob->short_description && mob->short_description != mob_proto[i].short_description)
            free(mob->short_description);
        if (mob->room_description && mob->room_description != mob_proto[i].room_description)
            free(mob->room_description);
        if (mob->look_description && mob->look_description != mob_proto[i].look_description)
            free(mob->look_description);
    }
    while (mob->affected)
        affect_remove(mob, mob->affected);

    /* free any assigned scripts */
    if (SCRIPT(mob))
        extract_script(mob, MOB_TRIGGER);

    free(mob);
    return true;
}

int save_mobiles(zone_rnum zone_num) {
    auto z = zone_table.find(zone_num);

    if (z == zone_table.end()) {
        basic_mud_log("SYSERR: GenOLC: save_mobiles: Invalid real zone number %d.", zone_num);
        return false;
    }

    dirty_npc_prototypes.insert(z->second.mobiles.begin(), z->second.mobiles.end());
    z->second.save_mobiles();
    return true;
}

#if CONFIG_GENOLC_MOBPROG
int write_mobile_mobprog(mob_vnum mvnum, struct char_data *mob, FILE *fd)
{
  char wmmarg[MAX_STRING_LENGTH], wmmcom[MAX_STRING_LENGTH];
  MPROG_DATA *mob_prog;

  for (mob_prog = GET_MPROG(mob); mob_prog; mob_prog = mob_prog->next) {
    wmmarg[MAX_STRING_LENGTH - 1] = '\0';
    wmmcom[MAX_STRING_LENGTH - 1] = '\0';
    strip_cr(strncpy(wmmarg, mob_prog->arglist, MAX_STRING_LENGTH - 1));
    strip_cr(strncpy(wmmcom, mob_prog->comlist, MAX_STRING_LENGTH - 1));
    fprintf(fd,	"%s %s~\n"
        "%s%c\n",
    medit_get_mprog_type(mob_prog), wmmarg,
    wmmcom, STRING_TERMINATOR
    );
    if (mob_prog->next == nullptr)
      fputs("|\n", fd);
  }
  return TRUE;
}
#endif


void check_mobile_strings(struct char_data *mob) {
    mob_vnum mvnum = mob_index[mob->vn].vn;
    check_mobile_string(mvnum, &GET_LDESC(mob), "long description");
    check_mobile_string(mvnum, &GET_DDESC(mob), "detailed description");
    check_mobile_string(mvnum, &GET_ALIAS(mob), "alias list");
    check_mobile_string(mvnum, &GET_SDESC(mob), "short description");
}

void check_mobile_string(mob_vnum i, char **string, const char *dscr) {
    if (*string == nullptr || **string == '\0') {
        char smbuf[128];
        sprintf(smbuf, "GenOLC: Mob #%d has an invalid %s.", i, dscr);
        mudlog(BRF, ADMLVL_GOD, true, smbuf);
        if (*string)
            free(*string);
        *string = strdup("An undefined string.");
    }
}

nlohmann::json mob_special_data::serialize() {
    nlohmann::json j;
    if(attack_type) j["attack_type"] = attack_type;
    if(default_pos != POS_STANDING) j["default_pos"] = default_pos;
    if(damnodice) j["damnodice"] = damnodice;
    if(damsizedice) j["damsizedice"] = damsizedice;

    return j;
}

void mob_special_data::deserialize(const nlohmann::json &j) {
    if(j.contains("attack_type")) attack_type = j["attack_type"];
    if(j.contains("default_pos")) default_pos = j["default_pos"];
    if(j.contains("damnodice")) damnodice = j["damnodice"];
    if(j.contains("damsizedice")) damsizedice = j["damsizedice"];
}

mob_special_data::mob_special_data(const nlohmann::json &j) : mob_special_data() {
    deserialize(j);
}




nlohmann::json time_data::serialize() {
    nlohmann::json j;

    if(birth) j["birth"] = birth;
    if(created) j["created"] = created;
    if(maxage) j["maxage"] = maxage;
    if(logon) j["logon"] = logon;
    if(played != 0.0) j["played"] = played;
    if(secondsAged != 0.0) j["secondsAged"] = secondsAged;

    return j;
}

void time_data::deserialize(const nlohmann::json &j) {
    if(j.contains("birth")) birth = j["birth"];
    if(j.contains("created")) created = j["created"];
    if(j.contains("maxage")) maxage = j["maxage"];
    if(j.contains("logon")) logon = j["logon"];
    if(j.contains("played")) played = j["played"];
    if(j.contains("secondsAged")) secondsAged = j["secondsAged"];
}

time_data::time_data(const nlohmann::json &j) : time_data() {
    deserialize(j);
}


nlohmann::json char_data::serializeBase() {
    auto j = serializeUnit();

    for(auto &[id, train] : trains) {
        if(train) j["trains"].push_back(std::make_pair(id, train));
    }

    for(auto &[id, attr] : attributes) {
        if(attr) j["attributes"].push_back(std::make_pair(id, attr));
    }

    for(auto &[id, mon] : moneys) {
        if(mon) j["moneys"].push_back(std::make_pair(id, mon));
    }

    for(auto &[id, align] : aligns) {
        if(align) j["aligns"].push_back(std::make_pair(id, align));
    }

    for(auto &[id, app] : appearances) {
        if(app) j["appearances"].push_back(std::make_pair(id, app));
    }

    for(auto &[id, app] : stats) {
        if(app) j["stats"].push_back(std::make_pair(id, app));
    }

    for(auto &[id, app] : nums) {
        if(app) j["nums"].push_back(std::make_pair(id, app));
    }

    for(auto i = 0; i < mobFlags.size(); i++)
        if(mobFlags.test(i)) j["mobFlags"].push_back(i);

    for(auto i = 0; i < playerFlags.size(); i++)
        if(playerFlags.test(i)) j["playerFlags"].push_back(i);

    for(auto i = 0; i < pref.size(); i++)
        if(pref.test(i)) j["pref"].push_back(i);

    for(auto i = 0; i < bodyparts.size(); i++)
        if(bodyparts.test(i)) j["bodyparts"].push_back(i);

    if(title && strlen(title)) j["title"] = title;
    j["race"] = race;

    j["chclass"] = chclass;
    if(weight != 0.0) j["weight"] = weight;

    for(auto i = 0; i < affected_by.size(); i++)
        if(affected_by.test(i)) j["affected_by"].push_back(i);


    if(armor) j["armor"] = armor;
    if(damage_mod) j["damage_mod"] = damage_mod;

    return j;
}

void char_data::deserializeBase(const nlohmann::json &j) {
    deserializeUnit(j);

    if(j.contains("trains")) {
        for(auto j2 : j["trains"]) {
            auto id = j2[0].get<CharTrain>();
            trains[id] = j2[1].get<attribute_train_t>();
        }
    }

    if(j.contains("attributes")) {
        for(auto j2 : j["attributes"]) {
            auto id = j2[0].get<CharAttribute>();
            attributes[id] = j2[1].get<attribute_t>();
        }
    }

    if(j.contains("moneys")) {
        for(auto j2 : j["moneys"]) {
            auto id = j2[0].get<CharMoney>();
            moneys[id] = j2[1].get<money_t>();
        }
    }

    if(j.contains("aligns")) {
        for(auto j2 : j["aligns"]) {
            auto id = j2[0].get<CharAlign>();
            aligns[id] = j2[1].get<align_t>();
        }
    }

    if(j.contains("appearances")) {
        for(auto j2 : j["appearances"]) {
            auto id = j2[0].get<CharAppearance>();
            appearances[id] = j2[1].get<appearance_t>();
        }
    }

    if(j.contains("stats")) {
        for(auto j2 : j["stats"]) {
            auto id = j2[0].get<CharStat>();
            stats[id] = j2[1].get<stat_t>();
        }
    }

    if(j.contains("nums")) {
        for(auto j2 : j["nums"]) {
            auto id = j2[0].get<CharNum>();
            nums[id] = j2[1].get<num_t>();
        }
    }

    if(j.contains("title")) title = strdup(j["title"].get<std::string>().c_str());
    if(j.contains("race")) race = j["race"].get<RaceID>();

    if(j.contains("chclass")) chclass = static_cast<SenseiID>(std::min(14, j["chclass"].get<int>()));

    if(j.contains("weight")) weight = j["weight"];

    if(j.contains("affected_by"))
        for(auto &i : j["affected_by"])
            affected_by.set(i.get<int>());

    if(j.contains("armor")) armor = j["armor"];
    if(j.contains("damage_mod")) damage_mod = j["damage_mod"];
    if(j.contains("mob_specials")) mob_specials.deserialize(j["mob_specials"]);
    if(j.contains("mobFlags")) for(auto &i : j["mobFlags"]) mobFlags.set(i.get<int>());
    if(j.contains("playerFlags")) for(auto &i : j["playerFlags"]) playerFlags.set(i.get<int>());
    if(j.contains("pref")) for(auto &i : j["pref"]) pref.set(i.get<int>());
    if(j.contains("bodyparts")) for(auto &i : j["bodyparts"]) bodyparts.set(i.get<int>());
}

nlohmann::json char_data::serializeProto() {
    auto j = serializeBase();

    auto ms = mob_specials.serialize();
    if(!ms.empty()) j["mob_specials"] = ms;

    for(auto p : proto_script) {
        if(trig_index.contains(p)) j["proto_script"].push_back(p);
    }

    return j;
}

nlohmann::json char_data::serializeInstance() {
    auto j = serializeBase();
    if(generation) j["generation"] = generation;

    for(auto i = 0; i < NUM_ADMFLAGS; i++)
        if(admflags.test(i)) j["admflags"].push_back(i);

    if(health < 1.0) j["health"] = health;
    if(energy < 1.0) j["energy"] = energy;
    if(stamina < 1.0) j["stamina"] = stamina;
    if(life < 1.0) j["life"] = life;

    if(exp) j["exp"] = exp;

    if(was_in_room != NOWHERE) j["was_in_room"] = was_in_room;
    auto td = time.serialize();
    if(!td.empty()) j["time"] = td;



    for(auto i = 0; i < 4; i++) {
        if(limb_condition[i]) j["limb_condition"].push_back(std::make_pair(i, limb_condition[i]));
    }

    for(auto i = 0; i < NUM_CONDITIONS; i++) {
        if(conditions[i]) j["conditions"].push_back(std::make_pair(i, conditions[i]));
    }

    if(freeze_level) j["freeze_level"] = freeze_level;
    if(invis_level) j["invis_level"] = invis_level;
    if(wimp_level) j["wimp_level"] = wimp_level;
    if(world.contains(load_room)) j["load_room"] = load_room;
    if(world.contains(hometown)) j["hometown"] = hometown;

    for(auto &[skill_id, s] : skill) {
        auto sk = s.serialize();
        if(!sk.empty()) j["skill"].push_back(std::make_pair(skill_id, sk));
    }

    if(speaking) j["speaking"] = speaking;
    if(preference) j["preference"] = preference;

    if(practice_points) j["practice_points"] = practice_points;

    for(auto a = affected; a; a = a->next) {
        if(a->type) j["affected"].push_back(a->serialize());
    }

    for(auto a = affectedv; a; a = a->next) {
        if(a->type) j["affectedv"].push_back(a->serialize());
    }

    if(absorbs) j["absorbs"] = absorbs;
    if(blesslvl) j["blesslvl"] = blesslvl;
    for(auto i = 0; i < 5; i++) {
        if(lboard[i]) j["lboard"].push_back(std::make_pair(i, lboard[i]));
    }

    for(auto i = 0; i < MAX_BONUSES; i++) {
        if(bonuses[i]) j["bonuses"].push_back(i);
    }

    if(boosts) j["boosts"] = boosts;

    if(clan && strlen(clan)) j["clan"] = clan;
    if(crank) j["crank"] = crank;
    if(con_cooldown) j["con_cooldown"] = con_cooldown;
    if(deathtime) j["deathtime"] = deathtime;
    if(dcount) j["dcount"] = dcount;
    if(death_type) j["death_type"] = death_type;
    if(damage_mod) j["damage_mod"] = damage_mod;
    if(droom) j["droom"] = droom;
    if(accuracy_mod) j["accuracy_mod"] = accuracy_mod;
    for(auto i = 0; i < 2; i++) {
        if(genome[i]) j["genome"].push_back(std::make_pair(i, genome[i]));
    }
    if(gauntlet) j["gauntlet"] = gauntlet;
    if(ingestLearned) j["ingestLearned"] = ingestLearned;
    if(kaioken) j["kaioken"] = kaioken;
    if(lifeperc) j["lifeperc"] = lifeperc;
    if(lastint) j["lastint"] = lastint;
    if(lastpl) j["lastpl"] = lastpl;
    if(moltexp) j["moltexp"] = moltexp;
    if(moltlevel) j["moltlevel"] = moltlevel;
    if(majinize) j["majinize"] = majinize;
    if(majinizer) j["majinizer"] = majinizer;
    if(mimic) j["mimic"] = mimic.value();
    if(form != FormID::Base) j["form"] = form;
    if(olc_zone) j["olc_zone"] = olc_zone;
    if(starphase) j["starphase"] = starphase;
    if(accuracy) j["accuracy"] = accuracy;
    if(position) j["position"] = position;

    if(rdisplay) j["rdisplay"] = rdisplay;
    if(relax_count) j["relax_count"] = relax_count;
    if(radar1) j["radar1"] = radar1;
    if(radar2) j["radar2"] = radar2;
    if(radar3) j["radar3"] = radar3;
    if(feature) j["feature"] = feature;
    if(ship) j["ship"] = ship;
    if(con_sdcooldown) j["con_sdcooldown"] = con_sdcooldown;
    if(shipr) j["shipr"] = shipr;
    if(skill_slots) j["skill_slots"] = skill_slots;
    if(stupidkiss) j["stupidkiss"] = stupidkiss;
    if(suppression) j["suppression"] = suppression;
    if(tail_growth) j["tail_growth"] = tail_growth;

    for(auto i = 0; i < 3; i++) {
        if(saving_throw[i]) j["saving_throw"].push_back(std::make_pair(i, saving_throw[i]));
    }
    for(auto i = 0; i < 3; i++) {
        if(apply_saving_throw[i]) j["apply_saving_throw"].push_back(std::make_pair(i, apply_saving_throw[i]));
    }
    if(upgrade) j["upgrade"] = upgrade;
    if(voice && strlen(voice)) j["voice"] = voice;

    if(script && script->global_vars) {
        j["dgvariables"] = serializeVars(script->global_vars);
    }

    if(relax_count) j["relax_count"] = relax_count;
    if(ingestLearned) j["ingestLearned"] = ingestLearned;

    if (poofin && strlen(poofin)) j["poofin"] = poofin;
    if (poofout && strlen(poofout)) j["poofout"] = poofout;
    if(players.contains(last_tell)) j["last_tell"] = last_tell;

    j["transBonus"] = transBonus;
    for(auto &[frm, tra] : transforms) {
        j["transforms"].push_back(std::make_pair(static_cast<int>(frm), tra.serialize()));
    }

    return j;
}

nlohmann::json char_data::serializePlayer() {
    auto j = serializeInstance();



    return j;
}


void char_data::deserializeInstance(const nlohmann::json &j, bool isActive) {
    deserializeBase(j);

    if(j.contains("generation")) generation = j["generation"];
    check_unique_id(this);
    add_unique_id(this);

    if(j.contains("admflags"))
        for(auto &i : j["admflags"])
            admflags.set(i.get<int>());

    if(j.contains("hometown")) hometown = j["hometown"];

    if(j.contains("time")) {
        time.deserialize(j["time"]);
    }

    if(j.contains("health")) health = j["health"];
    if(j.contains("energy")) energy = j["energy"];
    if(j.contains("stamina")) stamina = j["stamina"];
    if(j.contains("life")) life = j["life"];

    if(j.contains("limb_condition")) {
        for(auto &i : j["limb_condition"]) {
            limb_condition[i[0].get<int>()] = i[1];
        }
    }

    if(j.contains("exp")) exp = j["exp"];

    if(j.contains("was_in_room")) was_in_room = j["was_in_room"];

    if(j.contains("skill")) {
        for(auto &i : j["skill"]) {
            auto id = i[0].get<uint16_t>();
            skill.emplace(id, i[1]);
        }
    }

    if(j.contains("affected")) {
        auto ja = j["affected"];
        // reverse iterate using .rbegin() and .rend() while filling out
        // the linked list.
        for(auto it = ja.rbegin(); it != ja.rend(); ++it) {
            auto a = new affected_type(*it);
            a->next = affected;
            affected = a;
        }
    }

    if(j.contains("affectedv")) {
        auto ja = j["affectedv"];
        // reverse iterate using .rbegin() and .rend() while filling out
        // the linked list.
        for(auto it = ja.rbegin(); it != ja.rend(); ++it) {
            auto a = new affected_type(*it);
            a->next = affectedv;
            affectedv = a;
        }
    }

    if(j.contains("absorbs")) absorbs = j["absorbs"];
    if(j.contains("blesslvl")) blesslvl = j["blesslvl"];
    if(j.contains("lboard")) {
        for(auto &i : j["lboard"]) {
            lboard[i[0].get<int>()] = i[1];
        }
    }

    if(j.contains("bonuses")) {
        for(auto &i : j["bonuses"]) {
            bonuses[i.get<int>()] = true;
        }
    }

    if(j.contains("boosts")) boosts = j["boosts"];

    if(j.contains("clan")) clan = strdup(j["clan"].get<std::string>().c_str());
    if(j.contains("crank")) crank = j["crank"];
    if(j.contains("con_cooldown")) con_cooldown = j["con_cooldown"];
    if(j.contains("deathtime")) deathtime = j["deathtime"];
    if(j.contains("dcount")) dcount = j["dcount"];
    if(j.contains("death_type")) death_type = j["death_type"];

    if(j.contains("conditions")) {
        for(auto &i : j["conditions"]) {
            conditions[i[0].get<int>()] = i[1];
        }
    }

    if(j.contains("damage_mod")) damage_mod = j["damage_mod"];
    if(j.contains("droom")) droom = j["droom"];
    if(j.contains("accuracy_mod")) accuracy_mod = j["accuracy_mod"];
    if(j.contains("genome")) {
        for(auto &i : j["genome"]) {
            genome[i[0].get<int>()] = i[1];
        }
    }
    if(j.contains("gauntlet")) gauntlet = j["gauntlet"];
    if(j.contains("ingestLearned")) ingestLearned = j["ingestLearned"];
    if(j.contains("kaioken")) kaioken = j["kaioken"];
    if(j.contains("lifeperc")) lifeperc = j["lifeperc"];
    if(j.contains("lastint")) lastint = j["lastint"];
    if(j.contains("lastpl")) lastpl = j["lastpl"];
    if(j.contains("moltexp")) moltexp = j["moltexp"];
    if(j.contains("moltlevel")) moltlevel = j["moltlevel"];
    if(j.contains("majinize")) majinize = j["majinize"];
    if(j.contains("majinizer")) majinizer = j["majinizer"];
    if(j.contains("mimic")) mimic = j["mimic"].get<RaceID>();
    if(j.contains("olc_zone")) olc_zone = j["olc_zone"];
    if(j.contains("starphase")) starphase = j["starphase"];
    if(j.contains("accuracy")) accuracy = j["accuracy"];
    if(j.contains("position")) position = j["position"];

    if(j.contains("rdisplay")) rdisplay = strdup(j["rdisplay"].get<std::string>().c_str());
    if(j.contains("relax_count")) relax_count = j["relax_count"];
    if(j.contains("radar1")) radar1 = j["radar1"];
    if(j.contains("radar2")) radar2 = j["radar2"];
    if(j.contains("radar3")) radar3 = j["radar3"];
    if(j.contains("feature")) feature = strdup(j["feature"].get<std::string>().c_str());
    if(j.contains("ship")) ship = j["ship"];
    if(j.contains("con_sdcooldown")) con_sdcooldown = j["con_sdcooldown"];
    if(j.contains("shipr")) shipr = j["shipr"];
    if(j.contains("skill_slots")) skill_slots = j["skill_slots"];
    if(j.contains("stupidkiss")) stupidkiss = j["stupidkiss"];
    if(j.contains("suppression")) suppression = j["suppression"];
    if(j.contains("tail_growth")) tail_growth = j["tail_growth"];

    if(j.contains("saving_throw")) {
        for(auto t : j["saving_throw"]) {
            saving_throw[t[0].get<int>()] = t[1];
        }
    }
    if(j.contains("apply_saving_throw")) {
        for(auto t : j["apply_saving_throw"]) {
            apply_saving_throw[t[0].get<int>()] = t[1];
        }
    }
    if(j.contains("upgrade")) upgrade = j["upgrade"];
    if(j.contains("voice")) voice = strdup(j["voice"].get<std::string>().c_str());
    if(j.contains("wimp_level")) wimp_level = j["wimp_level"];

    if(!proto_script.empty()) {
        assign_triggers(this, OBJ_TRIGGER);
    }

    if(j.contains("dgvariables")) {
        if(!script) script = new script_data(this);
        deserializeVars(&script->global_vars, j["dgvariables"]);
    }

    auto proto = mob_proto.find(vn);
    if(proto != mob_proto.end()) {
        proto_script = proto->second.proto_script;
    }

    if(j.contains("load_room")) load_room = j["load_room"];

    if(j.contains("transBonus")) transBonus = j["transBonus"];
    if(j.contains("form")) form = j["form"].get<FormID>();
    if(j.contains("transforms")) {
        // it is a list of pairs that fills up the transforms map.
        for(const auto &j2 : j["transforms"]) {
            transforms.emplace(j2[0].get<FormID>(), j2[1]);
        }
    }

    if(j.contains("preference")) preference = j["preference"];
    if(j.contains("freeze_level")) freeze_level = j["freeze_level"];
    if(j.contains("practice_points")) practice_points = j["practice_points"];
    if(j.contains("speaking")) speaking = j["speaking"];

}

void char_data::deserializeProto(const nlohmann::json &j) {
    deserializeBase(j);

    if(j.contains("proto_script")) {
        for(const auto& p : j["proto_script"]) proto_script.emplace_back(p.get<trig_vnum>());
    }

}

void char_data::deserializePlayer(const nlohmann::json &j, bool isActive) {
    deserializeInstance(j, isActive);


}

void char_data::deserializeMobile(const nlohmann::json &j) {
    deserializeInstance(j, true);


}

char_data::char_data(const nlohmann::json &j) : char_data() {
    deserializeProto(j);

    if (!IS_HUMAN(this))
        affected_by.set(AFF_INFRAVISION);

    SPEAKING(this) = SKILL_LANG_COMMON;
    set_height_and_weight_by_race(this);

    mobFlags.set(MOB_ISNPC);
    mobFlags.reset(MOB_NOTDEADYET);

    playerFlags.reset(PLR_NOTDEADYET);

}

nlohmann::json skill_data::serialize() {
    nlohmann::json j = nlohmann::json::object();

    if(level) j["level"] = level;
    if(perfs) j["perfs"] = perfs;

    return j;
}

void skill_data::deserialize(const nlohmann::json &j) {
    if(j.contains("level")) level = j["level"];
    if(j.contains("perfs")) perfs = j["perfs"];
}

skill_data::skill_data(const nlohmann::json &j) : skill_data() {
    deserialize(j);
}

void char_data::deserializeContents(const nlohmann::json &j, bool isActive) {
    for(const auto& jo : j) {
        auto obj = new obj_data();
        obj->deserializeInstance(jo, isActive);
        obj_to_char(obj, this);
    }
}

void char_data::deserializeEquipment(const nlohmann::json &j, bool isActive) {
    for(const auto& jo : j) {
        // each jo is a two-element array. the first element is the id number of the equip
        // location, and the second is the item dump data.
        int id = jo[0];
        auto data = jo[1];
        auto obj = new obj_data();
        obj->deserializeInstance(data, isActive);
        // autoequip has a decrementer for some reason, so we'll increment.
        auto_equip(this, obj, id+1);
    }
}

nlohmann::json char_data::serializeEquipment() {
    auto j = nlohmann::json::array();
    for(auto i = 1; i < NUM_WEARS; i++) {
        if(auto obj = GET_EQ(this, i); obj) {
            j.push_back(std::make_pair(i, obj->serializeInstance()));
        }
    }

    return j;
}

nlohmann::json alias_data::serialize() {
    auto j = nlohmann::json::object();

    if(!name.empty()) j["name"] = name;
    if(!replacement.empty()) j["replacement"] = replacement;
    if(type) j["type"] = type;

    return j;
}

alias_data::alias_data(const nlohmann::json &j) : alias_data() {
    if(j.contains("name")) name = j["name"].get<std::string>();
    if(j.contains("replacement")) replacement = j["replacement"].get<std::string>();
    if(j.contains("type")) type = j["type"];
}

nlohmann::json player_data::serialize() {
    auto j = nlohmann::json::object();
    j["id"] = id;
    j["name"] = name;
    if(account) j["account"] = account->vn;

    for(auto &a : aliases) {
        j["aliases"].push_back(a.serialize());
    }

    for(auto &i : sensePlayer) {
        j["sensePlayer"].push_back(i);
    }

    for(auto &i : senseMemory) {
        j["senseMemory"].push_back(i);
    }

    for(auto &i : dubNames) {
        j["dubNames"].push_back(i);
    }

    for(auto i = 0; i < NUM_COLOR; i++) {
        if(color_choices[i] && strlen(color_choices[i])) j["color_choices"].push_back(std::make_pair(i, color_choices[i]));
    }

    return j;
}

player_data::player_data(const nlohmann::json &j) {
    id = j["id"];
    name = j["name"].get<std::string>();
    if(j.contains("account")) {
        auto accID = j["account"].get<vnum>();
        auto accFind = accounts.find(accID);
        if(accFind != accounts.end()) account = &accFind->second;
    }

    if(j.contains("aliases")) {
        for(auto ja : j["aliases"]) {
            aliases.emplace_back(ja);
        }
    }

    if(j.contains("sensePlayer")) {
        for(auto &i : j["sensePlayer"]) {
            sensePlayer.insert(i.get<int64_t>());
        }
    }

    if(j.contains("senseMemory")) {
        for(auto &i : j["senseMemory"]) {
            senseMemory.insert(i.get<vnum>());
        }
    }

    if(j.contains("dubNames")) {
        for(auto &i : j["dubNames"]) {
            dubNames.emplace(i[0].get<int64_t>(), i[1].get<std::string>());
        }
    }

    if(j.contains("color_choices")) {
        for(auto &i : j["color_choices"]) {
            color_choices[i[0].get<int>()] = strdup(i[1].get<std::string>().c_str());
        }
    }

}

void char_data::activate() {
    if(active) {
        logger->warn("Attempted to activate an already active character.");
        return;
    }
    active = true;
    next = character_list;
    character_list = this;

    if(script) script->activate();

    if(mob_proto.contains(vn)) {
        insert_vnum(characterVnumIndex, this);
    }

    if(contents) activateContents();
    for(auto i = 0; i < NUM_WEARS; i++) {
        if(GET_EQ(this, i)) {
            auto obj = GET_EQ(this, i);
            obj->activate();
        }
    }
    if(affected) {
        next_affect = affect_list;
        affect_list = this;
    }
    if(affectedv) {
        next_affectv = affectv_list;
        affectv_list = this;
    }
}


void char_data::deactivate() {
    if(!active) return;
    active = false;
    struct char_data *temp;
    REMOVE_FROM_LIST(this, character_list, next, temp);

    if(vn != NOTHING) {
        erase_vnum(characterVnumIndex, this);
    }

    if(affected) {
        REMOVE_FROM_LIST(this, affect_list, next_affect, temp);
    }
    if(affectedv) {
        REMOVE_FROM_LIST(this, affectv_list, next_affectv, temp);
    }
    if(contents) deactivateContents();
    for(auto i = 0; i < NUM_WEARS; i++) {
        if(GET_EQ(this, i)) {
            auto obj = GET_EQ(this, i);
            obj->deactivate();
        }
    }
}

nlohmann::json affected_type::serialize() {
    auto j = nlohmann::json::object();

    if(type) j["type"] = type;
    if(duration) j["duration"] = duration;
    if(modifier) j["modifier"] = modifier;
    if(location) j["location"] = location;
    if(specific) j["specific"] = specific;
    if(bitvector) j["bitvector"] = bitvector;

    return j;
}

affected_type::affected_type(const nlohmann::json &j) {
    if(j.contains("type")) type = j["type"];
    if(j.contains("duration")) duration = j["duration"];
    if(j.contains("modifier")) modifier = j["modifier"];
    if(j.contains("location")) location = j["location"];
    if(j.contains("specific")) specific = j["specific"];
    if(j.contains("bitvector")) bitvector = j["bitvector"];
}

weight_t char_data::getWeight(bool base) {
    auto total = weight;

    if(!base) {
        total += getAffectModifier(APPLY_CHAR_WEIGHT);
        total *= (1.0 + getAffectModifier(APPLY_WEIGHT_MULT));
    }

    return total;
}

int char_data::getHeight(bool base) {
    int total = get(CharNum::Height);

    if(!base) {
        total += getAffectModifier(APPLY_CHAR_HEIGHT);
        total *= (1.0 + getAffectModifier(APPLY_HEIGHT_MULT));
    }

    return total;
}

int char_data::setHeight(int val) {
    return set(CharNum::Height, std::max(0, val));
}

int char_data::modHeight(int val) {
    return setHeight(getHeight(true) + val);
}



double char_data::getTotalWeight() {
    return getWeight() + getCarriedWeight();
}

std::string char_data::getUID(bool active) {
    return fmt::format("#C{}:{}{}", id, generation, active ? "" : "!");
}

bool char_data::isActive() {
    return active;
}

nlohmann::json char_data::serializeLocation() {
    auto j = nlohmann::json::object();

    if(IS_NPC(this)) {
        j["in_room"] = in_room;
    } else {
        auto room = in_room != NOWHERE ? in_room : was_in_room;
        if(!desc) {
            room = load_room;
        }
        j["load_room"] = normalizeLoadRoom(room);
    }

    return j;
}

nlohmann::json char_data::serializeRelations() {
    auto j = nlohmann::json::object();

    return j;
}

void char_data::deserializeLocation(const nlohmann::json &j) {
    if(j.contains("in_room")) {
        auto vn = j["in_room"].get<room_vnum>();
        char_to_room(this, vn);
    } else if(j.contains("load_room")) {
        load_room = j["load_room"].get<room_vnum>();
    }
}

void char_data::deserializeRelations(const nlohmann::json &j) {

}

void char_data::save() {

}

bool char_data::isProvidingLight() {
    if(!IS_NPC(this) && PLR_FLAGGED(this, PLR_AURALIGHT)) return true;
    for(auto i = 0; i < NUM_WEARS; i++) if(auto e = GET_EQ(this, i); e) if(e->isProvidingLight()) return true;
    return false;
}

struct room_data* char_data::getRoom() {
    auto roomFound = world.find(in_room);
    if(roomFound != world.end()) return &roomFound->second;
    return nullptr;
}

double char_data::currentGravity() {
    if(auto room = getRoom(); room) {
        return room->getGravity();
    }
    return 1.0;
}

struct obj_data* char_data::findObject(const std::function<bool(struct obj_data*)> &func, bool working) {
    auto o = unit_data::findObject(func, working);
    if(o) return o;

    for(auto i = 0; i < NUM_WEARS; i++) {
        auto obj = equipment[i];
        if(!obj) continue;
        if(working && !obj->isWorking()) continue;
        if(func(obj)) return obj;
        auto p = obj->findObject(func, working);
        if(p) return p;
    }

    return nullptr;
}

std::set<struct obj_data*> char_data::gatherObjects(const std::function<bool(struct obj_data*)> &func, bool working) {
    auto out = unit_data::gatherObjects(func, working);

    for(auto i = 0; i < NUM_WEARS; i++) {
        auto obj = equipment[i];
        if(!obj) continue;
        if(working && !obj->isWorking()) continue;
        if(func(obj)) out.insert(obj);
        auto contents = obj->gatherObjects(func, working);
        out.insert(contents.begin(), contents.end());
    }
    return out;
}

void char_data::ageBy(double addedTime) {
    this->time.secondsAged += addedTime;
}

void char_data::setAge(double newAge) {
    this->time.secondsAged = newAge * SECS_PER_GAME_YEAR;
}
