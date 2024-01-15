//********************************************************************************
//
// This is the API for clans on Aeonian Dreams.
//
// Clans can be created in order for players to create their own rules to
// follow within some scope (i.e. the clan). For instance, there could be a
// PKiller's clan that allows all members to kill each other, or a Roleplayer's
// clan where all members agree to roleplay whenever they are in the presence
// of another member.
//
// Clans were implemented as a curiousity to see how players handle a world
// where they create and govern their own rules. Call me a sadist, but I'm
// interested in what will happen...
//
// If this isn't your cup of tea, Clans can easily act as a fairly decent
// method for implementing clans. And, unlike most clan code out there, any given
// player can belong to any number of clans. If you decide to use our clan
// code, please make note of it in your helpfile(s) relating to clans. Simply
// noting that this was orionally written by Alister of Aeonian Dreams would work
// splendid.
//
//  - Alister
//
//  Aeonian Dreams:
//    telnet://dreams.game-host.org:4000
//
//********************************************************************************
#pragma once

#include "structs.h"        // for char_data

#define LVL_CLAN_MOD            32
#define DEFAULT_OPEN_JOIN       false
#define DEFAULT_OPEN_LEAVE      false
#define DEFAULT_CLAN_INFO       "little is known about this clan, currently."
#define LIB_CLAN                LIB_ETC"clan/"
#define CLAN_LIST               LIB_CLAN"clans.cla"


//
// Boot up all the clans we have
//
extern void clanBoot();


//
// Reload a clan from disk
// return true if soccessful. False otherwise
//
extern bool clanReload(const char *name);


//
// create a new clan with the given name, and default info
// return false if a clan already has that name
// returns true if the clan is created
//
extern bool clanCreate(const char *name);


//
// Write a clan's info.
//
extern void clanINFOW(char *name, struct char_data *ch);

extern void clanSAFE(char *name);

extern void clan_update(uint64_t heartPulse, double deltaTime);

//
// Remove the clan with the given name from the list
// of all clans
//
extern void clanDestroy(const char *name);


//
// Returns true if a clan with the given name has formed
// returns false otherwise
//
extern bool isClan(const char *name);


//
// Sets a character as applying to become a member of the
// given clan.
//
// returns false if the character does not exist or is an NPC
// returns true if the member has already applied
// or if the application is successful
//
extern bool clanApply(const char *name, struct char_data *ch);


//
// add a person to a clan.
// returns false if the person is an NPC or does not exist
// returns true if the person was added or is already a member
//
extern bool clanInduct(const char *name, struct char_data *ch);


//
// These handle clan ranks
//

extern bool clanHIGHRANK(const char *name, const struct char_data *ch, const char *rank);

extern bool clanMIDRANK(const char *name, const struct char_data *ch, const char *rank);

extern bool clanRANK(const char *name, const struct char_data *ch, struct char_data *vict, int num);

extern bool clanRANKD(const char *name, struct char_data *ch, struct char_data *vict);

//
// This handles deposit, withdraw, and checking the clan bank
//
extern bool clanBANKADD(const char *name, const struct char_data *ch, long amt);

extern bool clanBANKSUB(const char *name, const struct char_data *ch, long amt);

extern long clanBANK(const char *name, const struct char_data *ch);

extern bool clanBANY(const char *name, const struct char_data *ch);

extern bool clanBSET(const char *name, struct char_data *ch);

//
// make a person a moderator of the clan.
//
extern bool clanMakeModerator(const char *name, struct char_data *ch);


//
// Expels a character from the clan.
//
extern void clanExpel(const char *name, struct char_data *ch);


//
// Decline a character's application to the clan.
//
extern void clanDecline(const char *name, const struct char_data *ch);


//
// returns true if the character is a member of the clan
// return false otherwise
//
extern bool clanIsMember(const char *name, const struct char_data *ch);

//
// returns clan mod, member, and applicant lists
//
extern void handle_clan_member_list(struct char_data *ch);

//
// returns true if the character is a moderator for the clan
// (i.e. can induct and expell other members)
//
extern bool clanIsModerator(const char *name, const struct char_data *ch);


//
// returns true if the character is applying for the given
// clan. returns false otherwise, or if the char is an NPC
//
extern bool clanIsApplicant(const char *name, const struct char_data *ch);


//
// returns true if anyone can join the clan
// returns false if new members need to be inducted by a moderator
//
extern bool clanOpenJoin(const char *name);


//
// return strue if anyone can feely leave the clan
// return false if people need to be expelled by a moderator
//
extern bool clanOpenLeave(const char *name);


//
// Sets open_join to true or false
// returns true if successful and false otherwise
//
extern bool clanSetOpenJoin(const char *name, int val);


//
// Sets open_leave to true or false
// returns true if successful and false otherwise
//
extern bool clanSetOpenLeave(const char *name, int val);


//
// list all info for the given clan to a character
//
extern void listClanInfo(const char *name, struct char_data *ch);


//
// send a list of all clans to the character
//
extern void listClans(struct char_data *ch);

extern int checkCLAN(struct char_data *ch);

extern void checkAPP(struct char_data *ch);


//
// send a list of the ranks of a clan NOT FINISHED
//
//void listClanRanks(struct char_data *ch);

//
// number of clans
//
extern int num_clans;


//
// List all of the clans vict belongs to, to char
//
extern void listClansOfVictToChar(const struct char_data *vict, struct char_data *ch);
