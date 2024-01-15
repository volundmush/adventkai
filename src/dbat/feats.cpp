/*****************************************************************************
** FEATS.C                                                                  **
** Source code for the Gates of Krynn Feats System.                         **
** Initial code by Paladine (Stephen Squires)                               **
** Created Thursday, September 5, 2002                                      **
**                                                                          **
*****************************************************************************/


#include "dbat/feats.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/handler.h"
#include "dbat/constants.h"
#include "dbat/interpreter.h"

/* Local Functions */

void feato(int featnum, char *name, int in_game, int can_learn, int can_stack);

void list_feats_known(struct char_data *ch);

void list_feats_available(struct char_data *ch);

void list_feats_complete(struct char_data *ch);

int compare_feats(const void *x, const void *y);


/* Global Variables and Structures */
struct feat_info feat_list[NUM_FEATS_DEFINED + 1];
int feat_sort_info[MAX_FEATS + 1];
char buf3[MAX_STRING_LENGTH];
char buf4[MAX_STRING_LENGTH];

/* External functions*/
int count_metamagic_feats(struct char_data *ch);

void feato(int featnum, char *name, int in_game, int can_learn, int can_stack) {
    feat_list[featnum].name = name;
    feat_list[featnum].in_game = in_game;
    feat_list[featnum].can_learn = can_learn;
    feat_list[featnum].can_stack = can_stack;
}

void free_feats() {
    /* Nothing to do right now */
}

void assign_feats() {

    int i;

    // Initialize the list of feats.

    for (i = 0; i <= NUM_FEATS_DEFINED; i++) {
        feat_list[i].name = "Unused Feat";
        feat_list[i].in_game = false;
        feat_list[i].can_learn = false;
        feat_list[i].can_stack = false;
    }

// Below are the various feat initializations.
// First parameter is the feat number, defined in feats.h
// Second parameter is the displayed name of the feat and argument used to train it
// Third parameter defines whether or not the feat is in the game or not, and thus can be learned and displayed
// Fourth parameter defines whether or not the feat can be learned through a trainer or whether it is
// a feat given automatically to certain classes or races.
// Fifth parameter defines whether or not the feat can be learned multiple times.

    feato(FEAT_ALERTNESS, "alertness", true, false, false);
    feato(FEAT_ARMOR_PROFICIENCY_HEAVY, "heavy armor proficiency", false, true, false);
    feato(FEAT_ARMOR_PROFICIENCY_LIGHT, "light armor proficiency", false, true, false);
    feato(FEAT_ARMOR_PROFICIENCY_MEDIUM, "medium armor proficiency", false, true, false);
    feato(FEAT_BLIND_FIGHT, "blind fighting", true, true, false);
    feato(FEAT_BREW_POTION, "brew potion", false, true, false);
    feato(FEAT_CLEAVE, "cleave", false, true, false);
    feato(FEAT_COMBAT_CASTING, "combat casting", false, true, false);
    feato(FEAT_COMBAT_REFLEXES, "combat reflexes", false, true, false);
    feato(FEAT_CRAFT_MAGICAL_ARMS_AND_ARMOR, "craft magical arms and armor", false, true, false);
    feato(FEAT_CRAFT_ROD, "craft rod", false, true, false);
    feato(FEAT_CRAFT_STAFF, "craft staff", false, true, false);
    feato(FEAT_CRAFT_WAND, "craft wand", false, true, false);
    feato(FEAT_CRAFT_WONDEROUS_ITEM, "craft wonderous item", false, true, false);
    feato(FEAT_DEFLECT_ARROWS, "deflect arrows", false, false, false);
    feato(FEAT_DODGE, "dodge", true, true, false);
    feato(FEAT_EMPOWER_SPELL, "empower spell", false, true, false);
    feato(FEAT_ENDURANCE, "endurance", false, true, false);
    feato(FEAT_ENLARGE_SPELL, "enlarge spell", false, false, false);
    feato(FEAT_WEAPON_PROFICIENCY_BASTARD_SWORD, "weapon proficiency - bastard sword", false, true, false);
    feato(FEAT_EXTEND_SPELL, "extend spell", false, true, false);
    feato(FEAT_EXTRA_TURNING, "extra turning", false, true, false);
    feato(FEAT_FAR_SHOT, "far shot", false, false, false);
    feato(FEAT_FORGE_RING, "forge ring", false, true, false);
    feato(FEAT_GREAT_CLEAVE, "great cleave", false, false, false);
    feato(FEAT_GREAT_FORTITUDE, "great fortitude", true, true, false);
    feato(FEAT_HEIGHTEN_SPELL, "heighten spell", false, true, false);
    feato(FEAT_IMPROVED_BULL_RUSH, "improved bull rush", false, false, false);
    feato(FEAT_IMPROVED_CRITICAL, "improved critical", true, true, true);
    feato(FEAT_IMPROVED_DISARM, "improved disarm", false, true, false);
    feato(FEAT_IMPROVED_INITIATIVE, "improved initiative", true, true, false);
    feato(FEAT_IMPROVED_TRIP, "improved trip", true, true, false);
    feato(FEAT_IMPROVED_TWO_WEAPON_FIGHTING, "improved two weapon fighting", true, true, false);
    feato(FEAT_IMPROVED_UNARMED_STRIKE, "improved unarmed strike", false, false, false);
    feato(FEAT_IRON_WILL, "iron will", true, true, false);
    feato(FEAT_LEADERSHIP, "leadership", false, false, false);
    feato(FEAT_LIGHTNING_REFLEXES, "lightning reflexes", true, true, false);
    feato(FEAT_MARTIAL_WEAPON_PROFICIENCY, "martial weapon proficiency", false, true, false);
    feato(FEAT_MAXIMIZE_SPELL, "maximize spell", false, true, false);
    feato(FEAT_MOBILITY, "mobility", true, true, false);
    feato(FEAT_MOUNTED_ARCHERY, "mounted archery", false, false, false);
    feato(FEAT_MOUNTED_COMBAT, "mounted combat", false, false, false);
    feato(FEAT_POINT_BLANK_SHOT, "point blank shot", false, false, false);
    feato(FEAT_POWER_ATTACK, "power attack", true, true, false);
    feato(FEAT_PRECISE_SHOT, "precise shot", false, false, false);
    feato(FEAT_QUICK_DRAW, "quick draw", false, false, false);
    feato(FEAT_QUICKEN_SPELL, "quicken spell", false, true, false);
    feato(FEAT_RAPID_SHOT, "rapid shot", false, false, false);
    feato(FEAT_RIDE_BY_ATTACK, "ride by attack", false, false, false);
    feato(FEAT_RUN, "run", false, false, false);
    feato(FEAT_SCRIBE_SCROLL, "scribe scroll", false, true, false);
    feato(FEAT_SHOT_ON_THE_RUN, "shot on the run", false, false, false);
    feato(FEAT_SILENT_SPELL, "silent spell", false, true, false);
    feato(FEAT_SIMPLE_WEAPON_PROFICIENCY, "simple weapon proficiency", true, true, false);
    feato(FEAT_SKILL_FOCUS, "skill focus", true, true, true);
    feato(FEAT_SPELL_FOCUS, "spell focus", false, true, true);
    feato(FEAT_SPELL_MASTERY, "spell mastery", false, true, true);
    feato(FEAT_SPELL_PENETRATION, "spell penetration", false, true, false);
    feato(FEAT_SPIRITED_CHARGE, "spirited charge", false, false, false);
    feato(FEAT_SPRING_ATTACK, "spring attack", true, false, false);
    feato(FEAT_STILL_SPELL, "still spell", false, true, false);
    feato(FEAT_STUNNING_FIST, "stunning fist", false, true, false);
    feato(FEAT_SUNDER, "sunder", false, true, false);
    feato(FEAT_TOUGHNESS, "toughness", true, true, true);
    feato(FEAT_TRACK, "track", false, true, false);
    feato(FEAT_TRAMPLE, "trample", false, false, false);
    feato(FEAT_TWO_WEAPON_FIGHTING, "two weapon fighting", true, true, false);
    feato(FEAT_WEAPON_FINESSE, "weapon finesse", true, true, true);
    feato(FEAT_WEAPON_FOCUS, "weapon focus", false, true, true);
    feato(FEAT_WEAPON_SPECIALIZATION, "weapon specialization", false, true, true);
    feato(FEAT_WHIRLWIND_ATTACK, "whirlwind attack", false, true, false);
    feato(FEAT_WEAPON_PROFICIENCY_DRUID, "weapon proficiency - druids", false, false, false);
    feato(FEAT_WEAPON_PROFICIENCY_ROGUE, "weapon proficiency - rogues", false, false, false);
    feato(FEAT_WEAPON_PROFICIENCY_MONK, "weapon proficiency - monks", false, false, false);
    feato(FEAT_WEAPON_PROFICIENCY_WIZARD, "weapon proficiency - wizards", false, false, false);
    feato(FEAT_WEAPON_PROFICIENCY_ELF, "weapon proficiency - elves", false, false, false);
    feato(FEAT_ARMOR_PROFICIENCY_SHIELD, "shield armor proficiency", false, false, false);
    feato(FEAT_SNEAK_ATTACK, "sneak attack", true, false, true);
    feato(FEAT_EVASION, "evasion", true, false, false);
    feato(FEAT_IMPROVED_EVASION, "improved evasion", true, false, false);
    feato(FEAT_ACROBATIC, "acrobatic", true, true, false);
    feato(FEAT_AGILE, "agile", true, true, false);
    feato(FEAT_ALERTNESS, "alertness", true, false, false);
    feato(FEAT_ANIMAL_AFFINITY, "animal affinity", false, true, false);
    feato(FEAT_ATHLETIC, "athletic", true, true, false);
    feato(FEAT_AUGMENT_SUMMONING, "augment summoning", false, false, false);
    feato(FEAT_COMBAT_EXPERTISE, "combat expertise", false, false, false);
    feato(FEAT_DECEITFUL, "deceitful", true, true, false);
    feato(FEAT_DEFT_HANDS, "deft hands", false, true, false);
    feato(FEAT_DIEHARD, "diehard", true, false, false);
    feato(FEAT_DILIGENT, "diligent", true, true, false);
    feato(FEAT_ESCHEW_MATERIALS, "eschew materials", false, false, false);
    feato(FEAT_EXOTIC_WEAPON_PROFICIENCY, "exotic weapon proficiency", false, false, false);
    feato(FEAT_GREATER_SPELL_FOCUS, "greater spell focus", false, false, true);
    feato(FEAT_GREATER_SPELL_PENETRATION, "greater spell penetration", false, false, false);
    feato(FEAT_GREATER_TWO_WEAPON_FIGHTING, "greater two weapon fighting", true, false, false);
    feato(FEAT_GREATER_WEAPON_FOCUS, "greater weapon focus", false, true, true);
    feato(FEAT_GREATER_WEAPON_SPECIALIZATION, "greater weapon specialization", false, true, true);
    feato(FEAT_IMPROVED_COUNTERSPELL, "improved counterspell", false, false, false);
    feato(FEAT_IMPROVED_FAMILIAR, "improved familiar", false, false, false);
    feato(FEAT_IMPROVED_FEINT, "improved feint", false, false, false);
    feato(FEAT_IMPROVED_GRAPPLE, "improved grapple", false, false, false);
    feato(FEAT_IMPROVED_OVERRUN, "improved overrun", false, false, false);
    feato(FEAT_IMPROVED_PRECISE_SHOT, "improved precise shot", false, false, false);
    feato(FEAT_IMPROVED_SHIELD_BASH, "improved shield bash", false, false, false);
    feato(FEAT_IMPROVED_SUNDER, "improved sunder", false, false, false);
    feato(FEAT_IMPROVED_TURNING, "improved turning", false, false, false);
    feato(FEAT_INVESTIGATOR, "investigator", false, true, false);
    feato(FEAT_MAGICAL_APTITUDE, "magical aptitude", false, true, false);
    feato(FEAT_MANYSHOT, "manyshot", false, false, false);
    feato(FEAT_NATURAL_SPELL, "natural spell", false, false, false);
    feato(FEAT_NEGOTIATOR, "negotiator", false, true, false);
    feato(FEAT_NIMBLE_FINGERS, "nimble fingers", false, true, false);
    feato(FEAT_PERSUASIVE, "persuasive", false, true, false);
    feato(FEAT_RAPID_RELOAD, "rapid reload", false, false, false);
    feato(FEAT_SELF_SUFFICIENT, "self sufficient", false, true, false);
    feato(FEAT_STEALTHY, "stealthy", true, true, false);
    feato(FEAT_ARMOR_PROFICIENCY_TOWER_SHIELD, "tower shield armor proficiency", false, false, false);
    feato(FEAT_TWO_WEAPON_DEFENSE, "two weapon defense", false, false, false);
    feato(FEAT_WIDEN_SPELL, "widen spell", false, false, false);
}

// The follwing function is used to check if the character satisfies the various prerequisite(s) (if any)
// of a feat in order to learn it.

int feat_is_available(struct char_data *ch, int featnum, int iarg, char *sarg) {
    return false;
}

int is_proficient_with_armor(const struct char_data *ch, int cmarmor_type) {
    return false;
}

int is_proficient_with_weapon(const struct char_data *ch, int cmweapon_type) {
    switch (cmweapon_type) {
        case WEAPON_TYPE_UNARMED:
            return 1;
        case WEAPON_TYPE_DAGGER:
        case WEAPON_TYPE_MACE:
        case WEAPON_TYPE_SICKLE:
        case WEAPON_TYPE_SPEAR:
        case WEAPON_TYPE_STAFF:
        case WEAPON_TYPE_CROSSBOW:
        case WEAPON_TYPE_SLING:
        case WEAPON_TYPE_THROWN:
        case WEAPON_TYPE_CLUB:
            if (HAS_FEAT(ch, FEAT_SIMPLE_WEAPON_PROFICIENCY))
                return true;
            break;
        case WEAPON_TYPE_SHORTBOW:
        case WEAPON_TYPE_LONGBOW:
        case WEAPON_TYPE_HAMMER:
        case WEAPON_TYPE_LANCE:
        case WEAPON_TYPE_FLAIL:
        case WEAPON_TYPE_LONGSWORD:
        case WEAPON_TYPE_SHORTSWORD:
        case WEAPON_TYPE_GREATSWORD:
        case WEAPON_TYPE_RAPIER:
        case WEAPON_TYPE_SCIMITAR:
        case WEAPON_TYPE_POLEARM:
        case WEAPON_TYPE_BASTARD_SWORD:
        case WEAPON_TYPE_AXE:
            if (HAS_FEAT(ch, FEAT_MARTIAL_WEAPON_PROFICIENCY))
                return true;
            break;
        default:
            return false;
            break;
    }
    return false;
}

int compare_feats(const void *x, const void *y) {
    int a = *(const int *) x,
            b = *(const int *) y;

    return strcmp(feat_list[a].name, feat_list[b].name);
}

void sort_feats() {
    int a;

    /* initialize array, avoiding reserved. */
    for (a = 1; a <= NUM_FEATS_DEFINED; a++)
        feat_sort_info[a] = a;

    qsort(&feat_sort_info[1], NUM_FEATS_DEFINED, sizeof(int), compare_feats);
}

void list_feats_known(struct char_data *ch) {
    int i, j, sortpos;
    int none_shown = true;
    int temp_value;
    int added_hp = 0;
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    // Display Headings
    sprintf(buf + strlen(buf), "\r\n");
    sprintf(buf + strlen(buf), "@WFeats Known@n\r\n");
    sprintf(buf + strlen(buf), "@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@n\r\n");
    sprintf(buf + strlen(buf), "\r\n");

    strcpy(buf2, buf);

    for (sortpos = 1; sortpos <= NUM_FEATS_DEFINED; sortpos++) {

        if (strlen(buf2) > MAX_STRING_LENGTH - 32)
            break;

        i = feat_sort_info[sortpos];
        if (HAS_FEAT(ch, i) && feat_list[i].in_game) {
            switch (i) {
                case FEAT_SKILL_FOCUS:
                    sprintf(buf, "%-20s (+%d points overall)\r\n", feat_list[i].name, HAS_FEAT(ch, i) * 2);
                    strcat(buf2, buf);
                    none_shown = false;
                    break;
                case FEAT_TOUGHNESS:
                    temp_value = HAS_FEAT(ch, FEAT_TOUGHNESS);
                    added_hp = temp_value * 3;
                    sprintf(buf, "%-20s (+%d hp)\r\n", feat_list[i].name, added_hp);
                    strcat(buf2, buf);
                    none_shown = false;
                    break;
                case FEAT_IMPROVED_CRITICAL:
                case FEAT_WEAPON_FINESSE:
                case FEAT_WEAPON_FOCUS:
                case FEAT_WEAPON_SPECIALIZATION:
                case FEAT_GREATER_WEAPON_FOCUS:
                case FEAT_GREATER_WEAPON_SPECIALIZATION:
                    for (j = 0; j <= MAX_WEAPON_TYPES; j++) {
                        if (HAS_COMBAT_FEAT(ch, feat_to_subfeat(i), j)) {
                            sprintf(buf, "%-20s (%s)\r\n", feat_list[i].name, weapon_type[j]);
                            strcat(buf2, buf);
                            none_shown = false;
                        }
                    }
                    break;
                default:
                    sprintf(buf, "%-20s\r\n", feat_list[i].name);
                    strcat(buf2, buf);        /* The above, @ should always be safe to do. */
                    none_shown = false;
                    break;
            }
        }
    }

    if (none_shown) {
        sprintf(buf, "You do not know any feats at this time.\r\n");
        strcat(buf2, buf);
    }

    write_to_output(ch->desc, buf2);
}

void list_feats_available(struct char_data *ch) {
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    int i, sortpos;
    int none_shown = true;

    // Display Headings
    sprintf(buf + strlen(buf), "\r\n");
    sprintf(buf + strlen(buf), "@WFeats Available to Learn@n\r\n");
    sprintf(buf + strlen(buf), "@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@n\r\n");
    sprintf(buf + strlen(buf), "\r\n");

    strcpy(buf2, buf);

    for (sortpos = 1; sortpos <= NUM_FEATS_DEFINED; sortpos++) {
        i = feat_sort_info[sortpos];
        if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
            strcat(buf2, "**OVERFLOW**\r\n");
            break;
        }
        if (feat_is_available(ch, i, 0, nullptr) && feat_list[i].in_game && feat_list[i].can_learn) {
            sprintf(buf, "%-20s\r\n", feat_list[i].name);
            strcat(buf2, buf);        /* The above, @ should always be safe to do. */
            none_shown = false;
        }
    }

    if (none_shown) {
        sprintf(buf, "There are no feats available for you to learn at this point.\r\n");
        strcat(buf2, buf);
    }

    write_to_output(ch->desc, buf2);
}

void list_feats_complete(struct char_data *ch) {

    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    int i, sortpos;
    int none_shown = true;

    // Display Headings
    sprintf(buf + strlen(buf), "\r\n");
    sprintf(buf + strlen(buf), "@WComplete Feat List@n\r\n");
    sprintf(buf + strlen(buf), "@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@B~@R~@n\r\n");
    sprintf(buf + strlen(buf), "\r\n");

    strcpy(buf2, buf);

    for (sortpos = 1; sortpos <= NUM_FEATS_DEFINED; sortpos++) {
        i = feat_sort_info[sortpos];
        if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
            strcat(buf2, "**OVERFLOW**\r\n");
            break;
        }
//	sprintf(buf, "%s : %s\r\n", feat_list[i].name, feat_list[i].in_game ? "In Game" : "Not In Game");
//	strcat(buf2, buf);
        if (feat_list[i].in_game) {
            sprintf(buf, "%-20s\r\n", feat_list[i].name);
            strcat(buf2, buf);        /* The above, @ should always be safe to do. */
            none_shown = false;
        }
    }

    if (none_shown) {
        sprintf(buf, "There are currently no feats in the game.\r\n");
        strcat(buf2, buf);
    }

    write_to_output(ch->desc, buf2);
}

int find_feat_num(char *name) {
    int ftindex, ok;
    char *temp, *temp2;
    char first[256], first2[256];

    for (ftindex = 1; ftindex <= NUM_FEATS_DEFINED; ftindex++) {
        if (is_abbrev(name, feat_list[ftindex].name))
            return (ftindex);

        ok = true;
        /* It won't be changed, but other uses of this function elsewhere may. */
        temp = any_one_arg((char *) feat_list[ftindex].name, first);
        temp2 = any_one_arg(name, first2);
        while (*first && *first2 && ok) {
            if (!is_abbrev(first2, first))
                ok = false;
            temp = any_one_arg(temp, first);
            temp2 = any_one_arg(temp2, first2);
        }

        if (ok && !*first2)
            return (ftindex);
    }

    return (-1);
}

ACMD(do_feats) {
    char arg[80];

    one_argument(argument, arg);

    if (is_abbrev(arg, "known") || !*arg) {
        send_to_char(ch, "Syntax is \"feats <available | complete | known>\".\r\n");
        list_feats_known(ch);
    } else if (is_abbrev(arg, "available")) {
        list_feats_available(ch);
    } else if (is_abbrev(arg, "complete")) {
        list_feats_complete(ch);
    }
}

int feat_to_subfeat(int feat) {
    switch (feat) {
        case FEAT_IMPROVED_CRITICAL:
            return CFEAT_IMPROVED_CRITICAL;
        case FEAT_WEAPON_FINESSE:
            return CFEAT_WEAPON_FINESSE;
        case FEAT_WEAPON_FOCUS:
            return CFEAT_WEAPON_FOCUS;
        case FEAT_WEAPON_SPECIALIZATION:
            return CFEAT_WEAPON_SPECIALIZATION;
        case FEAT_GREATER_WEAPON_FOCUS:
            return CFEAT_GREATER_WEAPON_FOCUS;
        case FEAT_GREATER_WEAPON_SPECIALIZATION:
            return CFEAT_GREATER_WEAPON_SPECIALIZATION;
        case FEAT_SPELL_FOCUS:
            return CFEAT_SPELL_FOCUS;
        case FEAT_GREATER_SPELL_FOCUS:
            return CFEAT_GREATER_SPELL_FOCUS;
        default:
            return -1;
    }
}
