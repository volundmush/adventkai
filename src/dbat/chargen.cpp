#include "dbat/chargen.h"
#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/class.h"
#include "dbat/races.h"
#include "dbat/spells.h"
#include "dbat/comm.h"
#include "dbat/constants.h"
#include "dbat/charmenu.h"
#include "dbat/players.h"

static const std::map<RaceID, std::map<CharAttribute, std::pair<int, int>>> startAttrRanges = {
        {
            RaceID::Saiyan, {
                {CharAttribute::Strength, {12, 18}},
                {CharAttribute::Constitution, {12, 18}},
                {CharAttribute::Wisdom, {8, 16}},
                {CharAttribute::Intelligence, {8, 14}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 16}},
            }
        },
        {
            RaceID::Halfbreed, {
                {CharAttribute::Strength, {10, 18}},
                {CharAttribute::Constitution, {10, 18}},
                {CharAttribute::Wisdom, {8, 18}},
                {CharAttribute::Intelligence, {8, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Human, {
                {CharAttribute::Strength, {8, 18}},
                {CharAttribute::Constitution, {8, 18}},
                {CharAttribute::Wisdom, {10, 18}},
                {CharAttribute::Intelligence, {12, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Hoshijin, {
                {CharAttribute::Strength, {10, 18}},
                {CharAttribute::Constitution, {9, 18}},
                {CharAttribute::Wisdom, {9, 18}},
                {CharAttribute::Intelligence, {9, 18}},
                {CharAttribute::Speed, {10, 18}},
                {CharAttribute::Agility, {9, 18}},
            }
        },
        {
            RaceID::Namekian, {
                {CharAttribute::Strength, {9, 18}},
                {CharAttribute::Constitution, {9, 18}},
                {CharAttribute::Wisdom, {12, 18}},
                {CharAttribute::Intelligence, {8, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Arlian, {
                {CharAttribute::Strength, {15, 20}},
                {CharAttribute::Constitution, {15, 20}},
                {CharAttribute::Wisdom, {8, 16}},
                {CharAttribute::Intelligence, {8, 16}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Android, {
                {CharAttribute::Strength, {12, 18}},
                {CharAttribute::Constitution, {8, 18}},
                {CharAttribute::Wisdom, {8, 16}},
                {CharAttribute::Intelligence, {8, 16}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::BioAndroid, {
                {CharAttribute::Strength, {14, 18}},
                {CharAttribute::Constitution, {8, 18}},
                {CharAttribute::Wisdom, {8, 18}},
                {CharAttribute::Intelligence, {8, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 14}},
            }
        },
        {
            RaceID::Majin, {
                {CharAttribute::Strength, {11, 18}},
                {CharAttribute::Constitution, {14, 18}},
                {CharAttribute::Wisdom, {8, 14}},
                {CharAttribute::Intelligence, {8, 14}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 17}},
            }
        },
        {
            RaceID::Tuffle, {
                {CharAttribute::Strength, {8, 14}},
                {CharAttribute::Constitution, {8, 14}},
                {CharAttribute::Wisdom, {8, 18}},
                {CharAttribute::Intelligence, {14, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Kai, {
                {CharAttribute::Strength, {9, 18}},
                {CharAttribute::Constitution, {8, 18}},
                {CharAttribute::Wisdom, {14, 18}},
                {CharAttribute::Intelligence, {10, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Icer, {
                {CharAttribute::Strength, {10, 18}},
                {CharAttribute::Constitution, {12, 18}},
                {CharAttribute::Wisdom, {8, 18}},
                {CharAttribute::Intelligence, {8, 18}},
                {CharAttribute::Speed, {8, 15}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Mutant, {
                {CharAttribute::Strength, {9, 18}},
                {CharAttribute::Constitution, {9, 18}},
                {CharAttribute::Wisdom, {9, 18}},
                {CharAttribute::Intelligence, {9, 18}},
                {CharAttribute::Speed, {9, 18}},
                {CharAttribute::Agility, {9, 18}},
            }
        },
        {
            RaceID::Kanassan, {
                {CharAttribute::Strength, {8, 16}},
                {CharAttribute::Constitution, {8, 16}},
                {CharAttribute::Wisdom, {12, 18}},
                {CharAttribute::Intelligence, {12, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Demon, {
                {CharAttribute::Strength, {11, 18}},
                {CharAttribute::Constitution, {8, 18}},
                {CharAttribute::Wisdom, {10, 18}},
                {CharAttribute::Intelligence, {10, 18}},
                {CharAttribute::Speed, {8, 18}},
                {CharAttribute::Agility, {8, 18}},
            }
        },
        {
            RaceID::Konatsu, {
                {CharAttribute::Strength, {10, 14}},
                {CharAttribute::Constitution, {10, 14}},
                {CharAttribute::Wisdom, {10, 16}},
                {CharAttribute::Intelligence, {10, 14}},
                {CharAttribute::Speed, {12, 18}},
                {CharAttribute::Agility, {14, 18}},
            }
        }
    };

namespace net {

    int ChargenParser::roll_stats(int type, int bonus) {

        int pool = 0, base_num = bonus, max_num = bonus;
        int powerlevel = 0, ki = 1, stamina = 2;

        if (type == powerlevel) {
            auto base = ch->get(CharAttribute::Strength, true);
            base_num = base * 3;
            max_num = base * 5;
        } else if (type == ki) {
            auto base = ch->get(CharAttribute::Intelligence, true);
            base_num = base * 3;
            max_num = base * 5;
        } else if (type == stamina) {
            auto base = ch->get(CharAttribute::Constitution, true);
            base_num = base * 3;
            max_num = base * 5;
        }

        pool = rand_number(base_num, max_num) + bonus;

        return (pool);
    }

    bool ChargenParser::bonus_exclusive(int type, int value, int exc) {
        if(GET_BONUS(ch, exc) > 0) {
            display_bonus_menu(type);
            sendText(fmt::format("@R{} and {} are mutually exclusive.\n\n", list_bonus[exc], list_bonus[value]));
            return false;
        }
        return true;
    }

    static std::map<int, int> exclusive = {
            {0, 1}, {2, 40}, {6, 39}, {8, 50}, {9, 43}, {10, 44}, {11, 45}, {12, 46}, {13, 47}, {14, 48},
            {15, 33}, {17, 34}, {18, 26}, {19, 29}, {20, 27}, {21, 30}, {22, 31}, {23, 32}, {24, 35}, {25, 51},
            {26, 18}, {27, 20}, {29, 19}, {30, 21}, {31, 22}, {32, 23}, {33, 15}, {34, 17}, {35, 24}, {39, 6},
            {40, 2}, {43, 9}, {44, 10}, {45, 11}, {46, 12}, {47, 13}, {48, 14}, {50, 8}, {51, 25}
    };

    int ChargenParser::opp_bonus(int value, int type) {

        switch (value) {
            case 3:
            case 16:
            case 20:
            case 27:
                if (IS_ANDROID(ch)) {
                    display_bonus_menu(type);
                    sendText(fmt::format("You can not take {} as an android!@n\r\n", list_bonus[value]));
                    return false;
                }
                break;
            case 17:
                if (IS_DEMON(ch)) {
                    display_bonus_menu(type);
                    sendText("As a demon you are already fireproof.\r\n");
                    return false;
                }
                break;
        }

        auto find = exclusive.find(value);
        if(find == exclusive.end()) return true;
        return bonus_exclusive(type, value, find->second);
    }

/* Handle CC point exchange for Bonus/negative */
    void ChargenParser::exchange_ccpoints(int value) {
        int type = 0;

        if (state == CON_BONUS) {
            type = 0;
        } else {
            type = 1;
        }


        if (GET_BONUS(ch, value) > 0 && ccpoints - list_bonus_cost[value] < 0) {
            display_bonus_menu(type);
            sendText("@RYou must unselect some bonus traits first.\r\n");
            return;
        } else if (GET_BONUS(ch, value) > 0 && ccpoints - list_bonus_cost[value] >= 0) {
            ccpoints -= list_bonus_cost[value];
            if (list_bonus_cost[value] > 0) {
                negcount -= list_bonus_cost[value];
            }
            GET_BONUS(ch, value) = 0;
            display_bonus_menu(type);
            sendText(fmt::format("@GYou cancel your selection of {}.\r\n", list_bonus[value]));
            return;
        }
        if (type == 0) {
            if (value > 25) {
                display_bonus_menu(type);
                sendText("@RYou are not in the negatives menu, enter B to switch.\r\n");
                return;
            } else if (ccpoints + list_bonus_cost[value] < 0) {
                display_bonus_menu(type);
                sendText(fmt::format("@RYou do not have enough points for {}.\r\n", list_bonus[value]));
                return;
            } else if (!opp_bonus(value, type)) {
                return;
            } else if (list_bonus_cost[value] < 0) {
                ccpoints += list_bonus_cost[value];
                GET_BONUS(ch, value) = 1;
                display_bonus_menu(type);
                sendText(fmt::format("@GYou select the bonus {}\r\n", list_bonus[value]));
                return;
            }
        } else {

            if (value < 26) {
                display_bonus_menu(type);
                sendText("@RYou are not in the bonuses menu, enter B to switch.\r\n");
                return;
            }

            int x, count = 0;

            for (x = 14; x < 52; x++) {
                if (GET_BONUS(ch, x) > 1) {
                    count += list_bonus_cost[x];
                }
            }

            if (list_bonus_cost[value] + count > 10) {
                display_bonus_menu(type);
                sendText("@RYou can not have more than +10 points from negatives.\r\n");
                return;
            } else if (!opp_bonus(value, type)) {
                return;
            } else {
                ccpoints += list_bonus_cost[value];
                negcount += list_bonus_cost[value];
                GET_BONUS(ch, value) = 2;
                display_bonus_menu(type);
                sendText(fmt::format("@GYou select the negative {}\r\n", list_bonus[value]));
                return;
            }
        }
    }

    static const char *bonus[] = {
            "Thrifty     - -10% Shop Buy Cost and +10% Shop Sell Cost             @D[@G-2pts @D]", /* Bonus 0 */
            "Prodigy     - +25% Experience Gained Until Level 80                  @D[@G-5pts @D]", /* Bonus 1 */
            "Quick Study - Character auto-trains skills faster                    @D[@G-3pts @D]", /* Bonus 2 */
            "Die Hard    - Life Force's PL regen doubled, but cost is the same    @D[@G-6pts @D]", /* Bonus 3 */
            "Brawler     - Physical attacks do 20% more damage                    @D[@G-4pts @D]", /* Bonus 4 */
            "Destroyer   - Damaged Rooms act as regen rooms for you               @D[@G-3pts @D]", /* Bonus 5 */
            "Hard Worker - Physical rewards better + activity drains less stamina @D[@G-3pts @D]", /* Bonus 6 */
            "Healer      - Heal/First-aid/Vigor/Repair restore +10%               @D[@G-3pts @D]", /* Bonus 7 */
            "Loyal       - +20% Experience When Grouped As Follower               @D[@G-2pts @D]", /* Bonus 8 */
            "Brawny      - Strength gains +2 every 10 levels, Train STR + 75%     @D[@G-5pts @D]", /* Bonus 9 */
            "Scholarly   - Intelligence gains +2 every 10 levels, Train INT + 75% @D[@G-5pts @D]", /* Bonus 10 */
            "Sage        - Wisdom gains +2 every 10 levels, Train WIS + 75%       @D[@G-5pts @D]", /* Bonus 11 */
            "Agile       - Agility gains +2 every 10 levels, Train AGL + 75%      @D[@G-4pts @D]", /* Bonus 12 */
            "Quick       - Speed gains +2 every 10 levels, Train SPD + 75%        @D[@G-6pts @D]", /* Bonus 13 */
            "Sturdy      - Constitution +2 every 10 levels, Train CON + 75%       @D[@G-5pts @D]", /* Bonus 14 */
            "Thick Skin  - -20% Physical and -10% ki dmg received                 @D[@G-5pts @D]", /* Bonus 15 */
            "Recipe Int. - Food cooked by you lasts longer/heals better           @D[@G-2pts @D]", /* Bonus 16 */
            "Fireproof   - -50% Fire Dmg taken, -10% ki, immunity to burn         @D[@G-4pts @D]", /* Bonus 17 */
            "Powerhitter - 15% critical hits will be x4 instead of x2             @D[@G-4pts @D]", /* Bonus 18 */
            "Healthy     - 40% chance to recover from ill effects when sleeping   @D[@G-3pts @D]", /* Bonus  19 */
            "Insomniac   - Can't Sleep. Immune to yoikominminken and paralysis    @D[@G-2pts @D]", /* Bonus  20 */
            "Evasive     - +15% to dodge rolls                                    @D[@G-3pts @D]", /* Bonus  21 */
            "The Wall    - +20% chance to block                                   @D[@G-3pts @D]", /* Bonus  22 */
            "Accurate    - +20% chance to hit physical, +10% to hit with ki       @D[@G-4pts @D]", /* Bonus  23 */
            "Energy Leech- -2% ki damage received for every 5 character levels,   @D[@G-5pts @D]\n                  @cas long as you can take that ki to your charge pool.@D        ", /* Bonus  24*/
            "Good Memory - +2 Skill Slots initially, +1 every 20 levels after     @D[@G-6pts @D]", /* Bonus 25 */
            "Soft Touch  - Half damage for all hit locations                      @D[@G+5pts @D]", /* Neg 26 */
            "Late Sleeper- Can only wake automatically. 33% every hour if maxed   @D[@G+5pts @D]", /* Neg 27 */
            "Impulse Shop- +25% shop costs                                        @D[@G+3pts @D]", /* Neg 28 */
            "Sickly      - Suffer from harmful effects longer                     @D[@G+5pts @D]", /* Neg 29 */
            "Punching Bag- -15% to dodge rolls                                    @D[@G+3pts @D]", /* Neg 30 */
            "Pushover    - -20% block chance                                      @D[@G+3pts @D]", /* Neg 31 */
            "Poor D. Perc- -20% chance to hit with physical, -10% with ki         @D[@G+4pts @D]", /* Neg 32 */
            "Thin Skin   - +20% physical and +10% ki damage received              @D[@G+4pts @D]", /* Neg 33 */
            "Fireprone   - +50% Fire Dmg taken, +10% ki, always burned            @D[@G+5pts @D]", /* Neg 34 */
            "Energy Int. - +2% ki damage received for every 5 character levels,   @D[@G+6pts @D]\n                  @rif you have ki charged you have 10% chance to lose   \n                  it and to take 1/4th damage equal to it.@D                    ", /* Neg 35 */
            "Coward      - Can't Attack Enemy With 150% Your Powerlevel           @D[@G+6pts @D]", /* Neg 36 */
            "Arrogant    - Cannot Suppress                                        @D[@G+1pt  @D]", /* Neg 37 */
            "Unfocused   - Charge concentration randomly breaks                   @D[@G+3pts @D]", /* Neg 38 */
            "Slacker     - Physical activity drains more stamina                  @D[@G+3pts @D]", /* Neg 39 */
            "Slow Learner- Character auto-trains skills slower                    @D[@G+3pts @D]", /* Neg 40 */
            "Masochistic - Defense Skills Cap At 75                               @D[@G+5pts @D]", /* Neg 41 */
            "Mute        - Can't use IC speech related commands                   @D[@G+4pts @D]", /* Neg 42 */
            "Wimp        - Strength is capped at 45                               @D[@G+6pts @D]", /* Neg 43 */
            "Dull        - Intelligence is capped at 45                           @D[@G+6pts @D]", /* Neg 44 */
            "Foolish     - Wisdom is capped at 45                                 @D[@G+6pts @D]", /* Neg 45 */
            "Clumsy      - Agility is capped at 45                                @D[@G+3pts @D]", /* Neg 46 */
            "Slow        - Speed is capped at 45                                  @D[@G+6pts @D]", /* Neg 47 */
            "Frail       - Constitution capped at 45                              @D[@G+4pts @D]", /* Neg 48 */
            "Sadistic    - Half Experience Gained For Quick Kills                 @D[@G+3pts @D]", /* Neg 49 */
            "Loner       - Can't Group, +5% Train gains, +10% to physical gains   @D[@G+2pts @D]", /* Neg 50 */
            "Bad Memory  - -5 Skill Slots                                         @D[@G+6pts @D]"  /* Neg 51 */
    };

/* This is the bonus/negatives menu for Character Creation */
    void ChargenParser::display_bonus_menu(int type) {

        int BonusCount = 26, NegCount = 52, x = 0, y = 0;

        if (type == 0) {
            sendText("\r\n@YBonus Trait SELECTION menu:\r\n@D---------------------------------------\r\n@n");
            for (x = 0; x < BonusCount; x++) {
                sendText(fmt::format("@C{:-2}@D)@c {} <@g{}@D>\n", x + 1, bonus[x], GET_BONUS(ch, x) > 0 ? "X" : " "));
            }
            sendText("\n");
        }

        if (type == 1) {
            y = BonusCount;
            sendText("@YNegative Trait SELECTION menu:\r\n@D---------------------------------------\r\n@n");
            while (y < NegCount) {
                sendText(fmt::format("@R{:-2}@D)@r {} <@g{}@D>\n", y - 14, bonus[y], GET_BONUS(ch, y) > 0 ? "X" : " "));
                y += 1;
            }
        }

        if (type == 0) {
            sendText("\n@CN@D)@c Show Negatives@n\n");
        } else {
            sendText("\n@CB@D)@c Show Bonuses@n\n");
        }
        sendText("@CX@D)@c Exit Traits Section and complete your character@n\n");
        sendText(fmt::format("@D---------------------------------------\n[@WCurrent Points Pool@W: @y{}@D] [@WPTS From Neg@W: @y{}@D]@w\n",
                  ccpoints, negcount));
    }

/* Return -1 if not an acceptable menu option *
 * Return 31 if selection is X                 *
 * Return other value if Bonus/Negative        */

    int parse_bonuses(const std::string &arg) {
        int value = -1, ident = -1;

        if(arg.empty()) return value;

        switch (toupper(arg[0])) {
            case 'B':
                value = 53;
                break;
            case 'N':
                value = 54;
                break;
            case 'X':
                value = 55;
                break;
        }

        if (value < 52) {
            ident = atoi(arg.c_str());
        }

        switch (ident) {
            /* Valid Selections */
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
                value = ident - 1;
                break;

                /* Invalid Selections */
            default:
                value = -1;
                break;
        }

        return (value);
    }


    ChargenParser::ChargenParser(std::shared_ptr<Connection>& co) : ConnectionParser(co) {
        ch = new char_data();
    }

    void ChargenParser::start() {
        parse("");
    }

    ChargenParser::~ChargenParser() {
        // Completing chargen should set our state to -1.
        // That will prevent the character from being freed.
        // Because in all other cases, we don't want this char_data to be laying around if
        // chargen is canceled somehow.
        if(state != -1 && ch) {
            free_char(ch);
        }
    }


    std::vector<RaceID> ChargenParser::valid_races() {
        auto check = [&](RaceID id) {return race::getValidSexes(id).contains(GET_SEX(ch)) && race::isPlayable(id);};
        return race::filterRaces(check);
    }

    void ChargenParser::display_races_sub() {
        int i = 0;
        for (const auto &r: valid_races())
            sendText(fmt::format("@C{:^15}@n{}", race::getName(r), !(++i % 2) ? "\r\n" : "   "));
    }

    void ChargenParser::display_races() {
        sendText("\r\n@YRace SELECTION menu:\r\n@D---------------------------------------\r\n@n");
        display_races_sub();

        sendText("\n @BR@W) @CRandom Race Selection!\r\n@n");
        sendText("\n @BT@W) @CToggle between SELECTION/HELP Menu\r\n@n");
        sendText("\n@WRace: @n");
    }

    std::vector<SenseiID> ChargenParser::valid_classes() {
        auto check = [&](SenseiID id) {return sensei::isPlayable(id) && sensei::isValidSenseiForRace(id, ch->race);};
        return sensei::filterSenseis(check);
    }

    void ChargenParser::display_classes_sub() {
        int i = 0;
        for (const auto &s: valid_classes())
            sendText(fmt::format("@C{}@n{}", sensei::getName(s).c_str(), !(++i % 2) ? "\r\n" : "	"));
    }

    void ChargenParser::display_classes() {
        sendText("\r\n@YSensei SELECTION menu:\r\n@D--------------------------------------\r\n@n");
        display_classes_sub();

        sendText("\n @BR@W) @CRandom Sensei Selection!\r\n@n");
        sendText("\n @BT@W) @CToggle between SELECTION/HELP Menu\r\n@n");
        sendText("\n@WSensei: @n");
    }

    void ChargenParser::display_races_help() {
        sendText("\r\n@YRace HELP menu:\r\n@G--------------------------------------------\r\n@n");
        display_races_sub();

        sendText("\n @BT@W) @CToggle between SELECTION/HELP Menu\r\n@n");
        sendText("\n@WHelp on Race #: @n");
    }

    void ChargenParser::display_classes_help() {
        sendText("\r\n@YClass HELP menu:\r\n@G-------------------------------------------\r\n@n");
        display_classes_sub();

        sendText("\n @BT@W) @CToggle between SELECTION/HELP Menu\r\n@n");
        sendText("\n@WHelp on Class #: @n");
    }

    void ChargenParser::parse(const std::string &arg) {
        std::optional<RaceID> chosen_race;
        std::optional<SenseiID> chosen_sensei;
        bool penalty = false, moveon = false;
        int roll = rand_number(1, 6);
        int value;

        struct char_data *found;

        switch(state) {
            case CON_GET_NAME:
                if(arg.empty()) {
                    sendText("\r\nPick a good name for this character.\r\nName: ");
                    return;
                } else {
                    maybeName = boost::trim_copy(arg);
                    if(findPlayer(maybeName)) {
                        sendText("\r\nUnfortunately that name's already taken.");
                        sendText("\r\nPick a good name for this character.\r\nName: ");
                        return;
                    }
                    auto result = validate_pc_name(maybeName);
                    if(!result.first) {
                        sendText("\r\n" + result.second.value() + "\r\n");
                        maybeName.clear();
                        sendText("\r\nPick a good name for this character.\r\nName: ");
                        return;
                    }
                    maybeName = result.second.value();
                    sendText(fmt::format("\r\n{}, huh? Are you sure?\r\n(Y/N): ", maybeName));
                    state = CON_NAME_CNFRM;
                    break;
                }
            case CON_NAME_CNFRM:
                if(boost::iequals(arg, "Y")) {
                    ch->name = strdup(maybeName.c_str());
                    state = CON_QRACE;
                    display_races();
                    break;
                } else if(boost::iequals(arg, "N")) {
                    maybeName.clear();
                    state = CON_GET_NAME;
                    sendText("\r\nOkay, let's try that again then.");
                    sendText("\r\nPick a good name for this character.\r\nName: ");
                    return;
                } else {
                    sendText(fmt::format("\r\n{}, huh? Are you sure?\r\n(Y/N): ", maybeName));
                    return;
                }
                break;
            case CON_QRACE:
            case CON_RACE_HELP:
                switch (arg.size()) {
                    case 0:
                        sendText("\r\nThat's not a race.\r\nRace: ");
                        return;
                    case 1:
                        switch(arg[0]) {
                            case 't':
                            case 'T':
                                switch (state) {
                                    case CON_QRACE:
                                        display_races_help();
                                        state = CON_RACE_HELP;
                                        break;
                                    case CON_RACE_HELP:
                                        display_races();
                                        state = CON_QRACE;
                                        break;
                                }
                                return;
                            default:
                                sendText("\r\nThat's not a race.\r\nRace: ");
                                return;
                        }
                        break;
                    default: {
                        auto check = [&](RaceID id) {return race::getValidSexes(id).contains(GET_SEX(ch)) && race::isPlayable(id);};
                        chosen_race = race::findRace(arg, check);
                        if (!chosen_race) {
                            sendText("\r\nThat's not a race.\r\nRace: ");
                            return;
                        }

                        auto race = chosen_race.value();

                        switch (state) {
                            case CON_QRACE:
                                ch->race = race;
                            break;
                            case CON_RACE_HELP:
                                show_help(conn, race::getName(race).c_str());
                            chosen_race.reset();
                            return;
                        }
                    }
                }
                if (IS_HALFBREED(ch)) {
                    sendText("@YWhat race do you prefer to by identified with?\r\n");
                    sendText("@cThis controls how others first view you and whether you start with\na tail and how fast it regrows when missing.\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Human\n@B2@W)@C Saiyan\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_RACIAL;
                } else if (IS_ANDROID(ch)) {
                    sendText("@YWhat do you want to be identified as at first glance?\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Android\n@B2@W)@C Human\n@B3@W)@C Robotic Humanoid\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_RACIAL;
                } else if (IS_NAMEK(ch)) {
                    ch->set(CharAppearance::Sex, SEX_NEUTRAL);
                    state = CON_QSEX;
                } else {
                    sendText("\r\n@wWhat is your sex @W(@BM@W/@MF@W/@GN@W)@w?@n");
                    state = CON_QSEX;
                }
                break;

            case CON_RACIAL:
                switch (arg[0]) {
                    case '1':
                        ch->set(CharNum::RacialPref, 1);
                        break;
                    case '2':
                        ch->set(CharNum::RacialPref, 2);
                        break;
                    case '3':
                        if (IS_HALFBREED(ch)) {
                            sendText("That is not an acceptable option.\r\n");
                            return;
                        } else {
                            ch->set(CharNum::RacialPref, 3);
                        }
                        break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                        return;
                }
                sendText("\r\n@wWhat is your sex @W(@BM@W/@MF@W/@GN@W)@w?@n");
                state = CON_QSEX;
                break;

            case CON_HAIRL:                /* query hair length */
                if (IS_HUMAN(ch) || IS_SAIYAN(ch) || IS_KONATSU(ch) ||
                    IS_MUTANT(ch) || IS_ANDROID(ch) || IS_KAI(ch) ||
                    IS_HALFBREED(ch) || IS_TRUFFLE(ch) ||
                    (IS_HOSHIJIN(ch) && IS_FEMALE(ch))) {
                    switch(arg[0]) {
                        case '1':
                            ch->set(CharAppearance::HairLength, HAIRL_BALD);
                            ch->set(CharAppearance::HairColor, HAIRC_NONE);
                            ch->set(CharAppearance::HairStyle, HAIRS_NONE);
                            break;
                        case '2':
                            ch->set(CharAppearance::HairLength, HAIRL_SHORT);
                            break;
                        case '3':
                            ch->set(CharAppearance::HairLength, HAIRL_MEDIUM);
                            break;
                        case '4':
                            ch->set(CharAppearance::HairLength, HAIRL_LONG);
                            break;
                        case '5':
                            ch->set(CharAppearance::HairLength, HAIRL_RLONG);
                            break;
                        default:
                            sendText("That is not an acceptable option.\r\n");
                            return;
                    }
                    if(IS_SAIYAN(ch)) {
                        ch->set(CharAppearance::HairColor, HAIRC_BLACK);
                        sendText("Saiyans always have black hair, if not bald.\r\n");
                    }
                    if (ch->get(CharAppearance::HairLength) == HAIRL_BALD || IS_SAIYAN(ch)) {
                        sendText("@YSkin color SELECTION menu:\r\n");
                        sendText("@D---------------------------------------@n\r\n");
                        sendText("@B1@W)@C White  @B2@W)@C Black  @B3@W)@C Green\r\n");
                        sendText("@B4@W)@C Orange @B5@W)@C Yellow @B6@W)@C Red@n\r\n");
                        sendText("@B7@W)@C Grey   @B8@W)@C Blue   @B9@W)@C Aqua\r\n");
                        sendText("@BA@W)@C Pink   @BB@W)@C Purple @BC@W)@C Tan@n\r\n");
                        sendText("@w\r\nMake a selection:@n\r\n");
                        state = CON_SKIN;
                    } else {
                        sendText("@YHair color SELECTION menu:\r\n");
                        sendText("@D---------------------------------------@n\r\n");
                        sendText("@B1@W)@C Black  @B2@W)@C Brown  @B3@W)@C Blonde\r\n");
                        sendText("@B4@W)@C Grey   @B5@W)@C Red    @B6@W)@C Orange@n\r\n");
                        sendText("@B7@W)@C Green  @B8@W)@C Blue   @B9@W)@C Pink\r\n");
                        sendText("@BA@W)@C Purple @BB@W)@C Silver @BC@W)@C Crimson@n\r\n");
                        sendText("@BD@W)@C White@n\r\n");
                        sendText("@w\r\nMake a selection:@n\r\n");
                        state = CON_HAIRC;
                    }
                } else {
                    if (IS_DEMON(ch) || IS_ICER(ch)) {
                        int hairTo;
                        switch(arg[0]) {
                            case '1':
                                hairTo = HAIRL_BALD;
                                break;
                            case '2':
                                hairTo = HAIRL_SHORT;
                                break;
                            case '3':
                                hairTo = HAIRL_MEDIUM;
                                break;
                            case '4':
                                hairTo = HAIRL_LONG;
                                break;
                            case '5':
                                hairTo = HAIRL_RLONG;
                                break;
                            default:
                                sendText("That is not an acceptable option.\r\n");
                                return;
                        }
                        ch->set(CharAppearance::HairLength, hairTo);
                        sendText("@YSkin color SELECTION menu:\r\n");
                        sendText("@D---------------------------------------@n\r\n");
                        sendText("@B1@W)@C White  @B2@W)@C Black  @B3@W)@C Green\r\n");
                        sendText("@B4@W)@C Orange @B5@W)@C Yellow @B6@W)@C Red@n\r\n");
                        sendText("@B7@W)@C Grey   @B8@W)@C Blue   @B9@W)@C Aqua\r\n");
                        sendText("@BA@W)@C Pink   @BB@W)@C Purple @BC@W)@C Tan@n\r\n");
                        sendText("@w\r\nMake a selection:@n\r\n");
                        state = CON_SKIN;
                    }
                    if (IS_MAJIN(ch) || IS_NAMEK(ch) || IS_ARLIAN(ch)) {
                        int hairTo;
                        switch(arg[0]) {
                            case '1':
                                hairTo = HAIRL_BALD;
                                break;
                            case '2':
                                hairTo = HAIRL_SHORT;
                                break;
                            case '3':
                                hairTo = HAIRL_MEDIUM;
                                break;
                            case '4':
                                hairTo = HAIRL_LONG;
                                break;
                            case '5':
                                hairTo = HAIRL_RLONG;
                                break;
                            default:
                                sendText("That is not an acceptable option.\r\n");
                                return;
                        }
                        ch->set(CharAppearance::HairLength, hairTo);
                        if (IS_ARLIAN(ch) && IS_FEMALE(ch)) {
                            sendText("@YWing color SELECTION menu:\r\n");
                            sendText("@D---------------------------------------@n\r\n");
                            sendText("@B1@W)@C Black  @B2@W)@C Brown  @B3@W)@C Blonde\r\n");
                            sendText("@B4@W)@C Grey   @B5@W)@C Red    @B6@W)@C Orange@n\r\n");
                            sendText("@B7@W)@C Green  @B8@W)@C Blue   @B9@W)@C Pink\r\n");
                            sendText("@BA@W)@C Purple @BB@W)@C Silver @BC@W)@C Crimson@n\r\n");
                            sendText("@BD@W)@C White@n\r\n");
                            sendText("@w\r\nMake a selection:@n\r\n");
                            state = CON_HAIRC;
                        } else {
                            sendText("@YSkin color SELECTION menu:\r\n");
                            sendText("@D---------------------------------------@n\r\n");
                            sendText("@B1@W)@C White  @B2@W)@C Black  @B3@W)@C Green\r\n");
                            sendText("@B4@W)@C Orange @B5@W)@C Yellow @B6@W)@C Red@n\r\n");
                            sendText("@B7@W)@C Grey   @B8@W)@C Blue   @B9@W)@C Aqua\r\n");
                            sendText("@BA@W)@C Pink   @BB@W)@C Purple @BC@W)@C Tan@n\r\n");
                            sendText("@w\r\nMake a selection:@n\r\n");
                            state = CON_SKIN;
                        }
                    } else {
                        state = CON_SKIN;
                    }
                }
                break;

            case CON_HAIRC:     /* query hair color */ {
                int hairTo;
                 switch(std::toupper(arg[0])) {
                    case '1':
                        hairTo = HAIRC_BLACK;
                        break;
                    case '2':
                        hairTo = HAIRC_BROWN;
                        break;
                    case '3':
                        hairTo = HAIRC_BLONDE;
                        break;
                    case '4':
                        hairTo = HAIRC_GREY;
                        break;
                    case '5':
                        hairTo = HAIRC_RED;
                        break;
                    case '6':
                        hairTo = HAIRC_ORANGE;
                        break;
                    case '7':
                        hairTo = HAIRC_GREEN;
                        break;
                    case '8':
                        hairTo = HAIRC_BLUE;
                        break;
                    case '9':
                        hairTo = HAIRC_PINK;
                        break;
                    case 'A':
                        hairTo = HAIRC_PURPLE;
                        break;
                    case 'B':
                        hairTo = HAIRC_SILVER;
                        break;
                    case 'C':
                        hairTo = HAIRC_CRIMSON;
                        break;
                    case 'D':
                        hairTo = HAIRC_WHITE;
                        break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                        return;
                }
                ch->set(CharAppearance::HairColor, hairTo);
            }
                if (IS_ARLIAN(ch)) {
                    sendText("@YSkin color SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C White  @B2@W)@C Black  @B3@W)@C Green\r\n");
                    sendText("@B4@W)@C Orange @B5@W)@C Yellow @B6@W)@C Red@n\r\n");
                    sendText("@B7@W)@C Grey   @B8@W)@C Blue   @B9@W)@C Aqua\r\n");
                    sendText("@BA@W)@C Pink   @BB@W)@C Purple @BC@W)@C Tan@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_SKIN;
                } else {
                    sendText("@YHair style SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Plain     @B2@W)@C Mohawk    @B3@W)@C Spiky\r\n");
                    sendText("@B4@W)@C Curly     @B5@W)@C Uneven    @B6@W)@C Ponytail@n\r\n");
                    sendText("@B7@W)@C Afro      @B8@W)@C Fade      @B9@W)@C Crew Cut\r\n");
                    sendText("@BA@W)@C Feathered @BB@W)@C Dread Locks@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_HAIRS;
                }
                break;

            case CON_HAIRS: {
                int hairTo;
                switch(std::toupper(arg[0])) {
                    case '1':
                        hairTo = HAIRS_PLAIN;
                    break;
                    case '2':
                        hairTo = HAIRS_MOHAWK;
                    break;
                    case '3':
                        hairTo = HAIRS_SPIKY;
                    break;
                    case '4':
                        hairTo = HAIRS_CURLY;
                    break;
                    case '5':
                        hairTo = HAIRS_UNEVEN;
                    break;
                    case '6':
                        hairTo = HAIRS_PONYTAIL;
                    break;
                    case '7':
                        hairTo = HAIRS_AFRO;
                    break;
                    case '8':
                        hairTo = HAIRS_FADE;
                    break;
                    case '9':
                        hairTo = HAIRS_CREW;
                    break;
                    case 'A':
                        hairTo = HAIRS_FEATHERED;
                    break;
                    case 'B':
                        hairTo = HAIRS_DRED;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->set(CharAppearance::HairStyle, hairTo);
            }

                sendText("@YSkin color SELECTION menu:\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C White  @B2@W)@C Black  @B3@W)@C Green\r\n");
                sendText("@B4@W)@C Orange @B5@W)@C Yellow @B6@W)@C Red@n\r\n");
                sendText("@B7@W)@C Grey   @B8@W)@C Blue   @B9@W)@C Aqua\r\n");
                sendText("@BA@W)@C Pink   @BB@W)@C Purple @BC@W)@C Tan@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_SKIN;
                break;

            case CON_SKIN: {
                int skinTo;
                switch(std::toupper(arg[0])) {
                    case '1':
                        skinTo = SKIN_WHITE;
                    break;
                    case '2':
                        skinTo = SKIN_BLACK;
                    break;
                    case '3':
                        skinTo = SKIN_GREEN;
                    break;
                    case '4':
                        skinTo = SKIN_ORANGE;
                    break;
                    case '5':
                        skinTo = SKIN_YELLOW;
                    break;
                    case '6':
                        skinTo = SKIN_RED;
                    break;
                    case '7':
                        skinTo = SKIN_GREY;
                    break;
                    case '8':
                        skinTo = SKIN_BLUE;
                    break;
                    case '9':
                        skinTo = SKIN_AQUA;
                    break;
                    case 'A':
                        skinTo = SKIN_PINK;
                    break;
                    case 'B':
                        skinTo = SKIN_PURPLE;
                    break;
                    case 'C':
                        skinTo = SKIN_TAN;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->set(CharAppearance::SkinColor, skinTo);
            }

                sendText("@YEye color SELECTION menu:\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C Blue  @B2@W)@C Black  @B3@W)@C Green\r\n");
                sendText("@B4@W)@C Brown @B5@W)@C Red    @B6@W)@C Aqua@n\r\n");
                sendText("@B7@W)@C Pink  @B8@W)@C Purple @B9@W)@C Crimson\r\n");
                sendText("@BA@W)@C Gold  @BB@W)@C Amber  @BC@W)@C Emerald@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_EYE;
                break;

            case CON_EYE: {
                int eyeTo;
                switch(std::toupper(arg[0])) {
                    case '1':
                        eyeTo = EYE_BLUE;
                    break;
                    case '2':
                        eyeTo = EYE_BLACK;
                    break;
                    case '3':
                        eyeTo = EYE_GREEN;
                    break;
                    case '4':
                        eyeTo = EYE_BROWN;
                    break;
                    case '5':
                        eyeTo = EYE_RED;
                    break;
                    case '6':
                        eyeTo = EYE_AQUA;
                    break;
                    case '7':
                        eyeTo = EYE_PINK;
                    break;
                    case '8':
                        eyeTo = EYE_PURPLE;
                    break;
                    case '9':
                        eyeTo = EYE_CRIMSON;
                    break;
                    case 'A':
                        eyeTo = EYE_GOLD;
                    break;
                    case 'B':
                        eyeTo = EYE_AMBER;
                    break;
                    case 'C':
                        eyeTo = EYE_EMERALD;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                    break;
                }
                ch->set(CharAppearance::EyeColor, eyeTo);
            }
                sendText("@YWhat do you want to be your most distinguishing feature:\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C My Eyes@n\r\n");
                if (IS_MAJIN(ch)) {
                    sendText("@B2@W)@C My Forelock@n\r\n");
                } else if (IS_NAMEK(ch) || IS_ARLIAN(ch)) {
                    sendText("@B2@W)@C My Antennae@n\r\n");
                } else if (IS_ICER(ch) || IS_DEMON(ch)) {
                    sendText("@B2@W)@C My Horns@n\r\n");
                } else if (GET_HAIRL(ch) == HAIRL_BALD) {
                    sendText("@B2@W)@C My Baldness@n\r\n");
                } else {
                    sendText("@B2@W)@C My Hair@n\r\n");
                }
                sendText("@B3@W)@C My Skin@n\r\n");
                sendText("@B4@W)@C My Height@n\r\n");
                sendText("@B5@W)@C My Weight@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");

                state = CON_DISTFEA;
                break;

            case CON_DISTFEA: {
                int dist;
                switch(arg[0]) {
                    case '1':
                        dist = 0;
                    break;
                    case '2':
                        dist = 1;
                    break;
                    case '3':
                        dist = 2;
                    break;
                    case '4':
                        dist = 3;
                    break;
                    case '5':
                        dist = 4;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                    break;
                }
                ch->set(CharAppearance::DistinguishingFeature, dist);
            }

                sendText("@YWhat Height should your character be?:\r\n");
                sendText("@D---------------------------------------@n\r\n");
                if (!IS_TRUFFLE(ch)) {
                    sendText("@C Please enter a number between 80 and 300, height is in cm.\r\n");
                } else {
                    sendText("@C Please enter a number between 20 and 150, height is in cm.\r\n");
                }
                state = CON_HEIGHT;
                break;

            case CON_HEIGHT: {
                int height = atoi(arg.c_str());

                if (IS_TRUFFLE(ch) && (height <= 20 || height >= 150)) {
                    sendText("That is not an option. For Tuffles, please keep height above 20cm, and below 150cm.\r\n");
                    return;
                }
                else if (height <= 80 || height >= 300) {
                    sendText("That is not an option. Please keep height above 80cm, and below 300cm.\r\n");
                    return;
                }
                else {
                    ch->setHeight(height);
                }

            }

                sendText("@YWhat Weight should your character be?:\r\n");
                sendText("@D---------------------------------------@n\r\n");
                if (!IS_TRUFFLE(ch)) {
                     sendText("@C Please enter a number between 25 and 150, weight is in kg.\r\n");
                } else {
                     sendText("@C Please enter a number between 3 and 40, height is in kg.\r\n");
                }
                state = CON_WEIGHT;
                break;

            case CON_WEIGHT: {
                int weight = atoi(arg.c_str());

                if (IS_TRUFFLE(ch) && (weight <= 3 || weight >= 40)) {
                    sendText("That is not an option. For Tuffles, please keep height above 3kg, and below 40kg.\r\n");
                    return;
                }
                else if (weight <= 25 || weight >= 150) {
                    sendText("That is not an option. Please keep height above 25kg, and below 150kg.\r\n");
                    return;
                }
                else {
                    ch->weight = weight;
                }
                
            }


                sendText("@YAura color SELECTION menu:\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C White  @B2@W)@C Blue@n\r\n");
                sendText("@B3@W)@C Red    @B4@W)@C Green@n\r\n");
                sendText("@B5@W)@C Pink   @B6@W)@C Purple@n\r\n");
                sendText("@B7@W)@C Yellow @B8@W)@C Black@n\r\n");
                sendText("@B9@W)@C Orange@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_AURA;
                break;

            case CON_AURA: {
                appearance_t aura = 0;
                switch(arg[0]) {
                    case '1':
                        aura = 0;
                    break;
                    case '2':
                        aura = 1;
                    break;
                    case '3':
                        aura = 2;
                    break;
                    case '4':
                        aura = 3;
                    break;
                    case '5':
                        aura = 4;
                    break;
                    case '6':
                        aura = 5;
                    break;
                    case '7':
                        aura = 6;
                    break;
                    case '8':
                        aura = 7;
                    break;
                    case '9':
                        aura = 8;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                    break;
                }
                ch->set(CharAppearance::Aura, aura);
            }

                display_classes();
                state = CON_QCLASS;
                break;

            case CON_Q1: {
                stat_t basepl = rand_number(30, 50);
                stat_t basest = rand_number(30, 50);
                stat_t baseki = rand_number(30, 50);
                {
                    auto defStats = startAttrRanges.at(ch->race);
                    for(auto &[attr, range] : defStats) {
                        ch->set(attr, rand_number(range.first, range.second));
                    }
                }

                switch(arg[0]) {
                    case '1':
                        basepl += roll_stats(5, 25);
                    basest += roll_stats(8, 50);
                    baseki += roll_stats(6, 50);
                    break;
                    case '2':
                        basepl += roll_stats(5, 55);
                    basest += roll_stats(8, 40);
                    baseki += roll_stats(6, 40);
                    break;
                    case '3':
                        basepl += roll_stats(5, 125);
                    basest += roll_stats(8, 50);
                    baseki += roll_stats(6, 40);
                    break;
                    case '4':
                        basepl += roll_stats(5, 65);
                    basest += roll_stats(8, 65);
                    baseki += roll_stats(6, 65);
                    ch->playerFlags.set(PLR_SKILLP);
                    break;
                    case '5':
                        basepl += roll_stats(5, 75);
                    basest += roll_stats(8, 100);
                    baseki += roll_stats(6, 75);
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->set(CharStat::PowerLevel, basepl);
                ch->set(CharStat::Stamina, basest);
                ch->set(CharStat::Ki, baseki);
            }
                sendText("\r\n@WQuestion (@G2@W out of @g10@W)\r\n");
                sendText("@YAnswer the following question:\r\n");
                sendText("@wYou are faced with the strongest opponent you have ever\r\nfaced in your life. You both have beat each other to the\r\nlimits of both your strengths. A situation has presented \r\nan opportunity to win the fight, what do you do?\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C Kill my opponent in a very brutal fashion!@n\r\n");
                sendText("@B2@W)@C Disable my opponent, but spare their life.@n\r\n");
                sendText("@B3@W)@C Kill my opponent, I have no other choice.@n\r\n");
                sendText("@B4@W)@C Try to evade my opponent till I can get away.@n\r\n");
                sendText("@B5@W)@C Take their head clean off and bathe in their blood!@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q2;
                break;

            case CON_Q2: {
                align_t align = 0;
                switch(arg[0]) {
                    case '1':
                        align += -200;
                    break;
                    case '2':
                        align += 100;
                    break;
                    case '3':
                        align += 10;
                    break;
                    case '4':
                        align += 0;
                    break;
                    case '5':
                        align += -400;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharAlign::GoodEvil, align);
            }

                sendText("\r\n@WQuestion (@G3@W out of @g10@W)\r\n");
                sendText("@YAnswer the following question:\r\n");
                sendText("@wYou are one day offered a means to gain incredible strength\r\nby some extraordinary means. The only problem is it requires\r\nthe lives of innocents to obtain. What do you do?\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C Hell yeah I take the opportunity to get stronger!@n\r\n");
                sendText("@B2@W)@C I refuse to gain unnatural strength.@n\r\n");
                sendText("@B3@W)@C I refuse to gain strength at the cost of the innocent.@n\r\n");
                sendText("@B4@W)@C I kill that many innocents before breakfast anyway...@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q3;
                break;
            case CON_Q3: {
                align_t align = 0;
                stat_t basepl = 0, basest = 0, baseki = 0;
                switch(arg[0]) {
                    case '1':
                        align += -100;
                    basepl += 100;
                    basest += 80;
                    baseki += 10;
                    break;
                    case '2':
                        align += 10;
                    basepl += 25;
                    basest += 25;
                    baseki += 25;
                    break;
                    case '3':
                        align += 50;
                    basepl += 20;
                    basest += 20;
                    baseki += 20;
                    break;
                    case '4':
                        align += -200;
                    basepl += 100;
                    basest += 100;
                    baseki += 100;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharAlign::GoodEvil, align);
                ch->mod(CharStat::PowerLevel, basepl);
                ch->mod(CharStat::Stamina, basest);
                ch->mod(CharStat::Ki, baseki);
            }

                sendText("\r\n@WQuestion (@G4@W out of @g10@W)\r\n");
                sendText("@YAnswer the following question:\r\n");
                sendText("@wOne day you are offered a way to make a lot of money, but in order\r\nto do so you will need to stop training for a whole month to\r\nhandle business. What do you do?\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C I take the opportunity, with more money I can train better later.@n\r\n");
                sendText("@B2@W)@C I refuse to waste my time. What I need is some nice hard training.@n\r\n");
                sendText("@B3@W)@C Hmm. With more money I can live better, certainly that is worth the time.@n\r\n");
                sendText("@B4@W)@C I choose to earn a little money while still training instead.@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q4;
                break;

            case CON_Q4: {
                money_t gold = 0;
                stat_t basepl = 0, basest = 0, baseki = 0;
                switch(arg[0]) {
                    case '1':
                        gold += 1000;
                    basepl -= rand_number(10, 30);
                    basest -= rand_number(10, 30);
                    baseki -= rand_number(10, 30);
                    break;
                    case '2':
                        gold = 0;
                    basepl += rand_number(50, 165);
                    basest += rand_number(50, 165);
                    baseki += rand_number(50, 165);
                    break;
                    case '3':
                        gold = 2500;
                    basepl -= rand_number(15, 25);
                    basest -= rand_number(15, 25);
                    baseki -= rand_number(15, 25);
                    break;
                    case '4':
                        gold = 150;
                    basepl += rand_number(25, 80);
                    basest += rand_number(25, 80);
                    baseki += rand_number(25, 80);
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                if(gold) ch->mod(CharMoney::Carried, gold);
                ch->mod(CharStat::PowerLevel, basepl);
                ch->mod(CharStat::Stamina, basest);
                ch->mod(CharStat::Ki, baseki);
            }
                sendText("\r\n@WQuestion (@G5@W out of @g10@W)\r\n");
                sendText("@YAnswer the following question:\r\n");
                sendText("@wYou are introduced to a new way of training one day, what do you do?\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C I prefer my way, it has worked so far.@n\r\n");
                sendText("@B2@W)@C I am open to new possibilites, sure.@n\r\n");
                sendText("@B3@W)@C I will at least try it, for a little while.@n\r\n");
                sendText("@B4@W)@C No way is superior to eating spinach everyday...@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q5;
                break;

            case CON_Q5: {
                stat_t pl = 0;
                switch(arg[0]) {
                    case '1':
                        pl += rand_number(0, 40);
                    break;
                    case '2':
                        pl += rand_number(-30, 80);
                    break;
                    case '3':
                        pl += rand_number(-25, 60);
                    break;
                    case '4':
                        pl += rand_number(0, 50);
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharStat::PowerLevel, pl);
            }
                sendText("\r\n@WQuestion (@G6@W out of @g10@W)\r\n");
                sendText("\r\n@YAnswer the following question:\r\n");
                sendText("@wYou have an enemy before you, what is your prefered method to attack him?@n\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C I prefer to defend rather than attack.@n\r\n");
                sendText("@B2@W)@C I prefer to throw a strong punch at the enemy's throat.@n\r\n");
                sendText("@B3@W)@C I prefer to send a devestating kick at the enemy's neck.@n\r\n");
                sendText("@B4@W)@C I prefer to smash them with a two-handed slam!@n\r\n");
                sendText("@B5@W)@C I prefer to throw one of my many energy attacks!@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q6;
                break;


            case CON_Q6: {
                stat_t basepl = 0, basest = 0, baseki = 0;
                switch(arg[0]) {
                    case '1':
                        basepl += rand_number(0, 15);
                    basest += rand_number(0, 15);
                    ch->choice = 1;
                    break;
                    case '2':
                        basepl += rand_number(0, 30);
                    ch->choice = 2;
                    break;
                    case '3':
                        basepl += rand_number(0, 30);
                    ch->choice = 3;
                    break;
                    case '4':
                        basepl += rand_number(0, 30);
                    ch->choice = 4;
                    break;
                    case '5':
                        baseki += rand_number(0, 50);
                    ch->choice = 5;
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharStat::PowerLevel, basepl);
                ch->mod(CharStat::Stamina, basest);
                ch->mod(CharStat::Ki, baseki);
            }
                sendText("\r\n@WQuestion (@G7@W out of @g10@W)\r\n");
                sendText("\r\n@YAnswer the following question:\r\n");
                sendText("@wYou are camped out one night in a field, the sky is clear and the\r\nstars visible. Looking at them, what thought crosses your mind?@n\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C One day, I am going to conquer every one of those.@n\r\n");
                sendText("@B2@W)@C I really wish I had brought that special someone along as this is a great night for romance@n\r\n");
                sendText("@B3@W)@C Those stars hold greater meaning to you than just planets, they dictate\r\nyour life or guide you and those arround you in one form or another.@n\r\n");
                sendText("@B4@W)@C You'd very much like to travel into space to get to one of these stars to study all that it holds.@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q7;
                break;

            case CON_Q7: {
                align_t align = 0;
                switch(arg[0]) {
                    case '1':
                        align += -10;
                    ch->mod(CharAttribute::Strength, 1);
                    break;
                    case '2':
                        align += +10;
                    ch->mod(CharAttribute::Speed, 1);
                    break;
                    case '3':
                        ch->mod(CharAttribute::Wisdom, 1);
                    break;
                    case '4':
                        ch->mod(CharAttribute::Intelligence, 1);
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharAlign::GoodEvil, align);
            }
                sendText("\r\n@WQuestion (@G8@W out of @g10@W)\r\n");
                sendText("\r\n@YAnswer the following question:\r\n");
                sendText("@wOne day, you are on the way home. You happen to walk past two\r\nof your best friends about to lock horns. Do you@n\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C You try to talk them out of it. With a silver tongue, you can stop the fight before it has began.@n\r\n");
                sendText("@B2@W)@C It's clear that they are quite chuffed about something, you could stop the fight now but they may just try again later. It's best to investigate and find out the source of the problem.@n\r\n");
                sendText("@B3@W)@C Pick the one you like the most and help them to win.@n\r\n");
                sendText("@B4@W)@C You try to sneak past them, reckoning no matter what you do there its a no win situation, so long as they don't see you.@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q8;
                break;

            case CON_Q8: {
                align_t align = 0;
                switch(arg[0]) {
                    case '1':
                        align += +10;
                    ch->mod(CharAttribute::Speed, 1);
                    break;
                    case '2':
                        align += +20;
                    ch->mod(CharAttribute::Wisdom, 1);
                    break;
                    case '3':
                        align += -10;
                    ch->mod(CharAttribute::Strength, 1);
                    break;
                    case '4':
                        align += -20;
                    ch->mod(CharAttribute::Agility, 1);
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharAlign::GoodEvil, align);
            }

                sendText("\r\n@WQuestion (@G9@W out of @g10@W)\r\n");
                sendText("\r\n@YAnswer the following question:\r\n");
                sendText("@wAs a kid you were confronted with this. On the way from the bakery,\r\na group of kids corner you in an alley and the leader demands that\r\nyou surrender your jam donut to him. Do you...@n\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@B1@W)@C Throw the donut up in the air, hoping that the leader will pay enough attention to it so that you can get at least one good shot in.@n\r\n");
                sendText("@B2@W)@C Surrender the donut for now but come back later with your friends and gain your revenge.@n\r\n");
                sendText("@B3@W)@C Give him the donut, then return to the baker. You tell him a sob story and convince him to give you a replacement donut.@n\r\n");
                sendText("@B4@W)@C It's better just to do as he says and leave it to that. After all its only a donut.@n\r\n");
                sendText("@w\r\nMake a selection:@n\r\n");
                state = CON_Q9;
                break;

            case CON_Q9: {
                align_t align = 0;
                switch(arg[0]) {
                    case '1':
                        ch->mod(CharAttribute::Strength, 1);
                    break;
                    case '2':
                        align += -30;
                    ch->mod(CharAttribute::Wisdom, 1);
                    break;
                    case '3':
                        align += -10;
                    ch->mod(CharAttribute::Speed, 1);
                    break;
                    case '4':
                        align += -5;
                    ch->mod(CharAttribute::Intelligence, 1);
                    break;
                    default:
                        sendText("That is not an acceptable option.\r\n");
                    return;
                }
                ch->mod(CharAlign::GoodEvil, align);
            }
                sendText("\r\n@WQuestion (@G10@W out of @g10@W)\r\n");
                sendText("\r\n@YAnswer the following question:\r\n");
                sendText("@wWhat do you wish your starting age to be?@n\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@wPlease enter something reasonable for your\r\n");
                sendText("@wrace and backstory. Don't fret too much, as\r\n");
                sendText("@wadmin can alter it later.\r\n");
                sendText("@w\r\nEnter Age in Years (decimals are supported):@n\r\n");
                state = CON_QX;
                break;

            case CON_QX: {
                try {
                    auto years = std::stod(arg);
                    if(years <= 0.0) {
                        sendText("Yeah... time doesn't work like that, sorry.\r\n");
                        return;
                    }
                    ch->setAge(years);
                }
                catch(const std::invalid_argument& ia) {
                    sendText("That is not an acceptable option.\r\n");
                    return;
                }

                if (!IS_HOSHIJIN(ch)) {
                    ccpoints = 5;
                } else if (IS_BIO(ch)) {
                    ccpoints = 3;
                } else {
                    ccpoints = 10;
                }
                sendText("@C             Alignment Menu@n\r\n");
                sendText("@D---------------------------------------@n\r\n");
                sendText(fmt::format("@cCurrent Alignment@D: {}{}@n\r\n",
                                     GET_ALIGNMENT(ch) < -50 ? "@R" : (GET_ALIGNMENT(ch) > 50 ? "@C" : "@G"),
                                     disp_align(ch)));
                sendText("@YThis is the alignment your character has based on your choices.\r\n");
                sendText("Choose to keep this alignment with no penalty, or choose new\r\n");
                sendText("alignment and suffer a -5%s PL and -1 stat (random) penalty.\r\n@n");
                sendText("@D---------------------------------------@n\r\n");
                sendText("@BK@W) @wKeep This Alignment@n\r\n");
                sendText("@B1@W) @wSaintly@n\r\n");
                sendText("@B2@W) @wValiant@n\r\n");
                sendText("@B3@W) @wHero@n\r\n");
                sendText("@B4@W) @wDo-gooder@n\r\n");
                sendText("@B5@W) @wNeutral\r\n");
                sendText("@B6@W) @wCrook@n\r\n");
                sendText("@B7@W) @wVillain@n\r\n");
                sendText("@B8@W) @wTerrible@n\r\n");
                sendText("@B9@W) @wHorribly Evil@n\r\n");
                sendText("Choose: \r\n");

                state = CON_ALIGN;
                break;
            }

            case CON_ALIGN:
                sendText("Choose: \r\n");
            {
                align_t align = -1;
                switch(toupper(arg[0])) {
                    case 'K':
                        moveon = true;
                    break;
                    case '1':
                        align = 1000;
                    penalty = true;
                    break;
                    case '2':
                        align = 799;
                    penalty = true;
                    break;
                    case '3':
                        align = 599;
                    penalty = true;
                    break;
                    case '4':
                        align = 299;
                    penalty = true;
                    break;
                    case '5':
                        align = 0;
                    penalty = true;
                    break;
                    case '6':
                        align = -299;
                    penalty = true;
                    break;
                    case '7':
                        align = -599;
                    penalty = true;
                    break;
                    case '8':
                        align = -799;
                    penalty = true;
                    break;
                    case '9':
                        align = -1000;
                    penalty = true;
                    break;
                    default:
                        sendText("That is not an acceptable option! Choose again...\r\n");
                    return;
                }
                if(align != -1) ch->set(CharAlign::GoodEvil, align);
            }

                if (moveon == true) {
                    sendText("@CWould you like to keep skills gained from your sensei/race combo (skills, not abilities)\r\nor would you prefer to keep those skill slots empty? If you choose\r\nto forget then you will receive 200 PS in exchange.@n\r\n");
                    sendText("keep or forget: \r\n");
                    state = CON_SKILLS;
                } else if (penalty == true) {
                    ch->loseBasePLPercent(.2);

                    switch (roll) {
                        case 1:
                            ch->mod(CharAttribute::Strength, -1);
                        case 2:
                            ch->mod(CharAttribute::Constitution, -1);
                        case 3:
                            ch->mod(CharAttribute::Wisdom, -1);
                        case 4:
                            ch->mod(CharAttribute::Intelligence, -1);
                        case 5:
                            ch->mod(CharAttribute::Speed, -1);
                        case 6:
                            ch->mod(CharAttribute::Agility, -1);
                            break;
                    }
                    sendText("@CWould you like to keep skills gained from your sensei/race combo (skills, not abilities)\r\nor would you prefer to keep those skill slots empty? If you choose\r\nto forget then you get 200 PS in exchange.@n\r\n");
                    sendText("keep or forget: \r\n");
                    state = CON_SKILLS;
                } else {
                    return;
                }
                break;

            case CON_QSEX:        /* query sex of new user         */
                if (!IS_NAMEK(ch)) {
                    int setSex;
                    switch(std::tolower(arg[0])) {
                        case 'm':
                            setSex = SEX_MALE;
                            break;
                        case 'f':
                            setSex = SEX_FEMALE;
                            break;
                        case 'n':
                            setSex = SEX_NEUTRAL;
                            break;
                        default:
                            sendText("That is not a sex..\r\n"
                                      "What IS your sex? ");
                            return;
                    }
                    ch->set(CharAppearance::Sex, setSex);
                }
                if (IS_HUMAN(ch) || IS_SAIYAN(ch) || IS_KONATSU(ch) ||
                    IS_MUTANT(ch) || IS_ANDROID(ch) || IS_KAI(ch) ||
                    IS_HALFBREED(ch) || IS_TRUFFLE(ch) ||
                    (IS_HOSHIJIN(ch) && IS_FEMALE(ch))) {
                    sendText("@YHair Length SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Bald  @B2@W)@C Short  @B3@W)@C Medium\r\n");
                    sendText("@B4@W)@C Long  @B5@W)@C Really Long@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_HAIRL;
                } else if (IS_DEMON(ch) || IS_ICER(ch)) {
                    sendText("@YHorn Length SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C None  @B2@W)@C Short  @B3@W)@C Medium\r\n");
                    sendText("@B4@W)@C Long  @B5@W)@C Really Long@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_HAIRL;
                } else if (IS_MAJIN(ch)) {
                    sendText("@YForelock Length SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Tiny  @B2@W)@C Short  @B3@W)@C Medium\r\n");
                    sendText("@B4@W)@C Long  @B5@W)@C Really Long@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_HAIRL;
                } else if (IS_NAMEK(ch) || IS_ARLIAN(ch)) {
                    sendText("@YAntenae Length SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Tiny  @B2@W)@C Short  @B3@W)@C Medium\r\n");
                    sendText("@B4@W)@C Long  @B5@W)@C Really Long@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_HAIRL;
                } else {
                    sendText("@YSkin color SELECTION menu:\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C White  @B2@W)@C Black  @B3@W)@C Green\r\n");
                    sendText("@B4@W)@C Orange @B5@W)@C Yellow @B6@W)@C Red@n\r\n");
                    sendText("@B7@W)@C Grey   @B8@W)@C Blue   @B9@W)@C Aqua\r\n");
                    sendText("@BA@W)@C Pink   @BB@W)@C Purple @BC@W)@C Tan@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_SKIN;
                }
                break;

            case CON_SKILLS:
                if(arg.empty()) {
                    sendText("keep or forget: \r\n");
                    return;
                } else if (!strcasecmp(arg.c_str(), "keep")) {
                    if (!IS_BIO(ch) && !IS_MUTANT(ch)) {
                        display_bonus_menu(0);
                        sendText("@CThis menu (and the Negatives menu) are for selecting various traits about your character.\n");
                        sendText("@wChoose: ");
                        state = CON_BONUS;
                    } else if (IS_MUTANT(ch)) {
                        sendText("\n@RSelect a mutation. A second will be chosen automatically..\n");
                        sendText("@D--------------------------------------------------------@n\n");
                        sendText("@B 1@W) @CExtreme Speed       @c-+30%s to Speed Index @C@n\n");
                        sendText("@B 2@W) @CInc. Cell Regen     @c-LF regen refills 12%s instead of 5%s@C@n\n");
                        sendText("@B 3@W) @CExtreme Reflexes    @c-+10 to parry, block, and dodge. +10 agility at creation.@C@n\n");
                        sendText("@B 4@W) @CInfravision         @c-+5 to spot hiding, can see in dark @C@n\n");
                        sendText("@B 5@W) @CNatural Camo        @c-+10 to hide/sneak rolls@C@n\n");
                        sendText("@B 6@W) @CLimb Regen          @c-Limbs regen almost instantly.@C@n\n");
                        sendText("@B 7@W) @CPoisonous           @c-Immune to poison, poison bite attack.@C@n\n");
                        sendText("@B 8@W) @CRubbery Body        @c-10%s of physical dmg to you is reduced and attacker takes that much loss in stamina.@C@n\n");
                        sendText("@B 9@w) @CInnate Telepathy    @c-Start with telepathy at SLVL 50@n\n");
                        sendText("@B10@w) @CNatural Energy      @c-Get 5%s of your ki damage refunded back into your current ki total.@n\n");
                        sendText("@wChoose: ");
                        ch->genome[0] = 0;
                        ch->genome[1] = 0;
                        state = CON_GENOME;
                    } else {
                        sendText("\n@RSelect two genomes to be your primary DNA strains.\n");
                        sendText("@D--------------------------------------------------------@n\n");
                        sendText("@B1@W) @CHuman   @c- @CHigher PS gains from fighting@n\n");
                        sendText("@B2@W) @CSaiyan  @c- @CSaiyan fight gains (halved)@n\n");
                        sendText("@B3@W) @CNamek   @c- @CStretchy arms that allow greater reach@n\n");
                        sendText("@B4@W) @CIcer    @c- @C+20%s damage for Tier 4 attacks@n\n");
                        sendText("@B5@W) @CTruffle @c- @CGrant Truffle Auto-train bonus@n\n");
                        sendText("@B6@W) @CArlian  @c- @CGrants Arlian Adrenaline ability@n\n\n");
                        sendText("@B7@W) @CKai     @c- @CStart with SLVL 30 Telepathy and SLVL 30 Focus.\r\n");
                        sendText("@B8@w) @CKonatsu @c- @C40%s higher chance to multihit on physical attacks.\r\n");
                        sendText("@wChoose: ");
                        ch->genome[0] = 0;
                        ch->genome[1] = 0;
                        state = CON_GENOME;
                    }
                } else if (!strcasecmp(arg.c_str(), "forget")) {
                    if (!IS_BIO(ch) && !IS_MUTANT(ch)) {
                        ch->modPractices(200);
                        ch->playerFlags.set(PLR_FORGET);
                        display_bonus_menu(0);
                        sendText("@CThis menu (and the Negatives menu) are for selecting various traits about your character.\n");
                        sendText("@wChoose: ");
                        state = CON_BONUS;
                    } else if (IS_MUTANT(ch)) {
                        ch->modPractices(200);
                        ch->playerFlags.set(PLR_FORGET);
                        sendText("\n@RSelect a mutation. A second will be chosen automatically..\n");
                        sendText("@D--------------------------------------------------------@n\n");
                        sendText("@B 1@W) @CExtreme Speed       @c-+30%s to Speed Index @C@n\n");
                        sendText("@B 2@W) @CInc. Cell Regen     @c-LF regen refills 12%s instead of 5%s@C@n\n");
                        sendText("@B 3@W) @CExtreme Reflexes    @c-+10 to parry, block, and dodge. +10 agility at creation.@C@n\n");
                        sendText("@B 4@W) @CInfravision         @c-+5 to spot hiding, can see in dark @C@n\n");
                        sendText("@B 5@W) @CNatural Camo        @c-+10 to hide/sneak rolls@C@n\n");
                        sendText("@B 6@W) @CLimb Regen          @c-Limbs regen almost instantly.@C@n\n");
                        sendText("@B 7@W) @CPoisonous           @c-Immune to poison, poison bite attack.@C@n\n");
                        sendText("@B 8@W) @CRubbery Body        @c-10%s less physical dmg to you is reduced and attacker takes that much loss in stamina.@C@n\n");
                        sendText("@B 9@w) @CInnate Telepathy    @c-Start with telepathy at SLVL 50@n\n");
                        sendText("@B10@w) @CNatural Energy      @c-Get 5%s of your ki damage refunded back into your current ki total.@n\n");
                        sendText("@wChoose: ");
                        ch->genome[0] = 0;
                        ch->genome[1] = 0;
                        state = CON_GENOME;
                    } else {

                        ch->modPractices(200);
                        ch->playerFlags.set(PLR_FORGET);
                        sendText("\n@RSelect two genomes to be your primary DNA strains.\n");
                        sendText("@D--------------------------------------------------------@n\n");
                        sendText("@B1@W) @CHuman   @c- @CHigher PS gains from fighting@n\n");
                        sendText("@B2@W) @CSaiyan  @c- @CSaiyan fight gains (halved)@n\n");
                        sendText("@B3@W) @CNamek   @c- @CNo food needed@n\n");
                        sendText("@B4@W) @CIcer    @c- @C+20%s damage for Tier 4 attacks@n\n");
                        sendText("@B5@W) @CTruffle @c- @CGrant Truffle Auto-train bonus@n\n");
                        sendText("@B6@W) @CArlian  @c- @CGrants Arlian Adrenaline ability@n\n\n");
                        sendText("@B7@W) @CKai     @c- @CStart with SLVL 30 Telepathy and SLVL 30 Focus.\r\n");
                        sendText("@B8@w) @CKonatsu @c- @C40%s higher chance to multihit on physical attacks.\r\n");
                        sendText("@wChoose: ");
                        ch->genome[0] = 0;
                        ch->genome[1] = 0;
                        state = CON_GENOME;
                    }
                } else {
                    sendText("keep or forget: \r\n");
                    return;
                }
                break;

            case CON_GENOME:
                if (IS_MUTANT(ch)) {
                    const char *display_genome[11] = {"Unselected", /* 0 */
                                                      "Extreme Speed", /* 1 */
                                                      "Increased Cell Regen", /* 2 */
                                                      "Extreme Reflexes", /* 3 */
                                                      "Infravision", /* 4 */
                                                      "Natural Camo", /* 5 */
                                                      "Limb Regen", /* 6 */
                                                      "Poisonous", /* 7 */
                                                      "Rubbery Body", /* 8 */
                                                      "Innate Telepathy", /* 9 */
                                                      "Natural Energy" /* 10 */

                    };

                    sendText("\n@RSelect a mutation. A second will be chosen automatically..\n");
                    sendText("@D--------------------------------------------------------@n\n");
                    sendText("@B 1@W) @CExtreme Speed       @c-+30%s to Speed Index @C@n\n");
                    sendText("@B 2@W) @CInc. Cell Regen     @c-LF regen refills 12%s instead of 5%s@C@n\n");
                    sendText("@B 3@W) @CExtreme Reflexes    @c-+10 to parry, block, and dodge. +10 agility at creation.@C@n\n");
                    sendText("@B 4@W) @CInfravision         @c-+5 to spot hiding, can see in dark @C@n\n");
                    sendText("@B 5@W) @CNatural Camo        @c-+10 to hide/sneak rolls@C@n\n");
                    sendText("@B 6@W) @CLimb Regen          @c-Limbs regen almost instantly.@C@n\n");
                    sendText("@B 7@W) @CPoisonous           @c-Immune to poison, poison bite attack.@C@n\n");
                    sendText("@B 8@W) @CRubbery Body        @c-10%s less physical dmg to you is reduced and attacker takes that much loss in stamina.@C@n\n");
                    sendText("@B 9@w) @CInnate Telepathy    @c-Start with telepathy at SLVL 50@n\n");
                    sendText("@B10@w) @CNatural Energy      @c-Get 5%s of your ki damage refunded back into your current ki total.@n\n");
                    sendText("\n@wChoose: ");

                    if (ch->genome[0] > 0 && ch->genome[1] <= 0) {
                        int num = rand_number(1, 8);
                        if (num == ch->genome[0]) {
                            while (num == ch->genome[0]) {
                                num = rand_number(1, 8);
                            }
                        }
                        if (num == 3) {
                            ch->mod(CharAttribute::Agility, 10);
                        }
                        sendText(fmt::format("@CRolling second mutation... Your second mutation is @D[@Y{}@D]@n\r\n",
                                  display_genome[num]));
                        ch->genome[1] = num;
                        return;
                    } else if (ch->genome[1] > 0) {
                        display_bonus_menu(0);
                        sendText("@CThis menu (and the Negatives menu) are for selecting various traits about your character.\n");
                        sendText("\n@wChoose: ");
                        state = CON_BONUS;
                    } else if(arg.empty()) {
                        sendText("That is not an acceptable choice!\r\n");
                        sendText("\n@wChoose: ");
                        return;
                    } else {
                        int choice = atoi(arg.c_str());
                        if (choice < 1 || choice > 10) {
                            sendText("That is not an acceptable choice!\r\n");
                            return;
                        } else {
                            sendText(fmt::format("@CYou have chosen the mutation @D[@Y{}@D]@n\r\n", display_genome[choice]));
                            ch->genome[0] = choice;
                            if (choice == 3) {
                                ch->mod(CharAttribute::Agility, 10);
                            } else if (choice == 9) {
                                SET_SKILL(ch, SKILL_TELEPATHY, 50);
                            }
                            return;
                        }
                    }
                } else if (ch->genome[1] > 0) {
                    display_bonus_menu(0);
                    sendText("@CThis menu (and the Negatives menu) are for selecting various traits about your character.\n");
                    sendText("@wChoose: ");
                    state = CON_BONUS;
                } else if(arg.empty()) {
                    const char *display_genome[9] = {"Unselected", /* 0 */
                                                     "Human", /* 1 */
                                                     "Saiyan", /* 2 */
                                                     "Namek", /* 3 */
                                                     "Icer", /* 4 */
                                                     "Truffle", /* 5 */
                                                     "Arlian", /* 6 */
                                                     "Kai", /* 7 */
                                                     "Konatsu" /* 8 */

                    };
                    sendText("@RSelect two genomes to be your primary DNA strains.\n");
                    sendText("@D--------------------------------------------------------@n\n");
                    sendText("@B1@W) @CHuman   @c- @CHigher PS gains from fighting@n\n");
                    sendText("@B2@W) @CSaiyan  @c- @CSaiyan fight gains (halved)@n\n");
                    sendText("@B3@W) @CNamek   @c- @CNo food needed@n\n");
                    sendText("@B4@W) @CIcer    @c- @C+20%s damage for Tier 4 attacks@n\n");
                    sendText("@B5@W) @CTruffle @c- @CGrant Truffle Auto-train bonus@n\n");
                    sendText("@B6@W) @CArlian  @c- @CGrants Arlian Adrenaline ability@n\n\n");
                    sendText("@B7@W) @CKai     @c- @CStart with SLVL 30 Telepathy and SLVL 30 Focus.\r\n");
                    sendText("@B8@w) @CKonatsu @c- @C40%s higher chance to multihit on physical attacks.\r\n");
                    sendText(fmt::format("@D----[@gGenome 1@W: @G{}@D]----[@gGenome 2@W: @G{}@D]----",
                              display_genome[ch->genome[0]], display_genome[ch->genome[1]]));

                    sendText("\n@wChoose: ");
                    return;

                } else {
                    int select = atoi(arg.c_str());
                    if (select > 8 || select < 1) {
                        sendText("@RSelect two genomes to be your primary DNA strains.\n");
                        sendText("@D--------------------------------------------------------@n\n");
                        sendText("@B1@W) @CHuman   @c- @CHigher PS gains from fighting@n\n");
                        sendText("@B2@W) @CSaiyan  @c- @CSaiyan fight gains (halved)@n\n");
                        sendText("@B3@W) @CNamek   @c- @CNo food needed@n\n");
                        sendText("@B4@W) @CIcer    @c- @C+20%s damage for Tier 4 attacks@n\n");
                        sendText("@B5@W) @CTruffle @c- @CGrant Truffle Auto-train bonus@n\n");
                        sendText("@B6@W) @CArlian  @c- @CGrants Arlian Adrenaline ability@n\n\n");
                        sendText("@B7@W) @CKai     @c- @CStart with SLVL 30 Telepathy and SLVL 30 Focus.\r\n");
                        sendText("@B8@w) @CKonatsu @c- @C40%s higher chance to multihit on physical attacks.\r\n");
                        sendText("@RThat is not an acceptable selection. @WTry again:@n\n");
                        return;
                    } else {
                        if (ch->genome[0] == 0) {
                            ch->genome[0] = select;
                            if (select == 7) {
                                SET_SKILL(ch, SKILL_TELEPATHY, 30);
                                SET_SKILL(ch, SKILL_FOCUS, 30);
                            }
                        } else if (ch->genome[0] > 0 && ch->genome[0] == select) {
                            sendText("You can't choose the same thing for both genomes!\r\n");
                            sendText("\n@wChoose: ");
                            return;
                        } else if (ch->genome[1] == 0) {
                            ch->genome[1] = select;
                            if (select == 7) {
                                SET_SKILL(ch, SKILL_TELEPATHY, 30);
                                SET_SKILL(ch, SKILL_FOCUS, 30);
                            }
                        }
                        const char *display_genome[9] = {"Unselected", /* 0 */
                                                         "Human", /* 1 */
                                                         "Saiyan", /* 2 */
                                                         "Namek", /* 3 */
                                                         "Icer", /* 4 */
                                                         "Truffle", /* 5 */
                                                         "Arlian", /* 6 */
                                                         "Kai", /* 7 */
                                                         "Konatsu" /* 8 */

                        };
                        sendText("@RSelect two genomes to be your primary DNA strains.\n");
                        sendText("@D--------------------------------------------------------@n\n");
                        sendText("@B1@W) @CHuman   @c- @CHigher PS gains from fighting@n\n");
                        sendText("@B2@W) @CSaiyan  @c- @CSaiyan fight gains (halved)@n\n");
                        sendText("@B3@W) @CNamek   @c- @CNo food needed@n\n");
                        sendText("@B4@W) @CIcer    @c- @C+20%s damage for Tier 4 attacks@n\n");
                        sendText("@B5@W) @CTruffle @c- @CGrant Truffle Auto-train bonus@n\n");
                        sendText("@B6@W) @CArlian  @c- @CGrants Arlian Adrenaline ability@n\n\n");
                        sendText("@B7@W) @CKai     @c- @CStart with SLVL 30 Telepathy and SLVL 30 Focus.\r\n");
                        sendText("@B8@w) @CKonatsu @c- @C40%s higher chance to multihit on physical attacks.\r\n");
                        sendText(fmt::format("@D----[@gGenome 1@W: @G%s@D]----[@gGenome 2@W: @G%s@D]----",
                                  display_genome[ch->genome[0]], display_genome[ch->genome[1]]));
                        return;
                    }
                }
                break;

            case CON_BONUS:
                if(arg.empty()) {
                    display_bonus_menu(0);
                    sendText("@wChoose: ");
                    return;
                } else if (!strcasecmp(arg.c_str(), "b") || !strcasecmp(arg.c_str(), "B")) {
                    display_bonus_menu(0);
                    sendText("@RYou are already in that menu.\r\n");
                    sendText("@wChoose: ");
                    return;
                } else if (!strcasecmp(arg.c_str(), "N") || !strcasecmp(arg.c_str(), "n")) {
                    display_bonus_menu(1);
                    sendText("@wChoose: ");
                    state = CON_NEGATIVE;
                } else if (!strcasecmp(arg.c_str(), "x") || !strcasecmp(arg.c_str(), "X")) {
                    negcount = 0;
                    sendText("\r\n@wTo check the bonuses/negatives you have in game use the status command");
                    if (ccpoints > 0) {
                        sendText("\r\n@GYour left over points were spent on Practice Sessions@w");
                        ch->modPractices(100 * ccpoints);
                    }
                    finish();
                    return;
                } else if ((value = parse_bonuses(arg)) != 1337) {
                    if (value == -1) {
                        display_bonus_menu(0);
                        sendText("@RThat is not an option.\r\n");
                        sendText("@wChoose: ");
                        return;
                    } else {
                        exchange_ccpoints(value);
                        sendText("@wChoose: ");
                        return;
                    }
                } else {
                    display_bonus_menu(0);
                    sendText("@wChoose: ");
                    return;
                }
                break;

            case CON_NEGATIVE:
                if(arg.empty()) {
                    display_bonus_menu(1);
                    sendText("@wChoose: ");
                    return;
                } else if (!strcasecmp(arg.c_str(), "n") || !strcasecmp(arg.c_str(), "N")) {
                    display_bonus_menu(1);
                    sendText("@RYou are already in that menu.\r\n");
                    sendText("@wChoose: ");
                    return;
                } else if (!strcasecmp(arg.c_str(), "b") || !strcasecmp(arg.c_str(), "B")) {
                    display_bonus_menu(0);
                    sendText("@wChoose: ");
                    state = CON_BONUS;
                } else if (!strcasecmp(arg.c_str(), "x") || !strcasecmp(arg.c_str(), "X")) {
                    negcount = 0;
                    for(auto c : {CharStat::PowerLevel, CharStat::Ki, CharStat::Stamina}) {
                        if(ch->get(c) < 90) ch->set(c, 90);
                    }
                    sendText("\r\n@wTo check the bonuses/negatives you have in game use the status command");
                    if (ccpoints > 0) {
                        sendText("\r\n@GYour left over points were spent on Practice Sessions@w");
                        ch->modPractices(100 * ccpoints);
                    }
                    finish();
                } else if ((value = parse_bonuses(arg)) != 1337) {
                    if (value == -1) {
                        display_bonus_menu(1);
                        sendText("@RThat is not an option.\r\n");
                        sendText("@wChoose: ");
                        return;
                    } else {
                        value += 15;
                        exchange_ccpoints(value);
                        sendText("@wChoose: ");
                        return;
                    }
                } else {
                    display_bonus_menu(1);
                    sendText("@wChoose: ");
                    return;
                }
                break;

            case CON_QCLASS:
            case CON_CLASS_HELP:
                switch (arg.size()) {
                    case 1:
                        switch(arg[0]) {
                            case 't':
                            case 'T':
                                switch (state) {
                                    case CON_CLASS_HELP:
                                        state = CON_QCLASS;
                                        display_classes();
                                        return;
                                    case CON_QCLASS:
                                        display_classes_help();
                                        state = CON_CLASS_HELP;
                                        return;
                                }
                                break;
                            default:
                                sendText("\r\nThat's not a sensei.\r\nSensei: ");
                                return;
                        }
                        break;
                    default: {
                        auto check = [&](SenseiID id) {return sensei::isPlayable(id) && sensei::isValidSenseiForRace(id, ch->race);};
                        chosen_sensei = sensei::findSensei(arg, check);
                        if (!chosen_sensei) {
                            sendText("\r\nThat's not a sensei.\r\nSensei: ");
                            return;
                        }

                        auto sensei = chosen_sensei.value();

                        switch (state) {
                            case CON_CLASS_HELP:
                                show_help(conn, sensei::getName(sensei).c_str());
                                chosen_sensei.reset();
                                return;
                            case CON_QCLASS:
                                ch->chclass = sensei;
                                break;
                        }
                    }
                }

                if (IS_ANDROID(ch)) {
                    sendText("\r\n@YChoose your model type.\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Absorbtion Model@n\r\n");
                    sendText("@B2@W)@C Repair Model@n\r\n");
                    sendText("@B3@W)@C Sense, Powersense Model@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_ANDROID;
                } else {
                    sendText("\r\n@RAnswer The following questions carefully, they construct your alignment\r\nand affect your stats.\r\n\r\n");
                    sendText("@WQuestion (@G1@W out of @g10@W)\r\n");
                    sendText("@YAnswer the following question:\r\n");
                    sendText("@wYou go to train one day, but do not know the best\r\nway to approach it, What do you do?\r\n");
                    sendText("@D---------------------------------------@n\r\n");
                    sendText("@B1@W)@C Ask someone with more experience what to do.@n\r\n");
                    sendText("@B2@W)@C Jump in with some nice classic pushups!@n\r\n");
                    sendText("@B3@W)@C Search for something magical to increase my strength.@n\r\n");
                    sendText("@B4@W)@C Practice my favorite skills instead of working just on my body.@n\r\n");
                    sendText("@B5@W)@C Spar with a friend so we can both improve our abilities.@n\r\n");
                    sendText("@w\r\nMake a selection:@n\r\n");
                    state = CON_Q1;
                }
                break;

            case CON_ANDROID:
                switch(arg[0]) {
                    case '1':
                        ch->playerFlags.set(PLR_ABSORB);
                        sendText("\r\n@RAnswer The following questions carefully, they may construct your alignment in conflict with your trainer, or your stats contrary to your liking.\r\n\r\n");
                        sendText("\r\n@WQuestion (@G1@W out of @g10@W)");
                        sendText("@YAnswer the following question:\r\n");
                        sendText("@wYou go to train one day, but do not know the best\r\nway to approach it, What do you do?\r\n");
                        sendText("@D---------------------------------------@n\r\n");
                        sendText("@B1@W)@C Ask someone with more experience what to do.@n\r\n");
                        sendText("@B2@W)@C Jump in with some nice classic pushups!@n\r\n");
                        sendText("@B3@W)@C Search for something magical to increase my strength.@n\r\n");
                        sendText("@B4@W)@C Practice my favorite skills instead of working just on my body.@n\r\n");
                        sendText("@B5@W)@C Spar with a friend so we can both improve our abilities.@n\r\n");
                        sendText("@w\r\nMake a selection:@n\r\n");
                        state = CON_Q1;
                        break;
                    case '2':
                        ch->playerFlags.set(PLR_REPAIR);
                        sendText("\r\n@RAnswer The following questions carefully, they may construct your alignment in conflict with your trainer, or your stats contrary to your linking.\r\n\r\n");
                        sendText("@YAnswer the following question:\r\n");
                        sendText("@wYou go to train one day, but do not know the best\r\nway to approach it, What do you do?\r\n");
                        sendText("@D---------------------------------------@n\r\n");
                        sendText("@B1@W)@C Ask someone with more experience what to do.@n\r\n");
                        sendText("@B2@W)@C Jump in with some nice classic pushups!@n\r\n");
                        sendText("@B3@W)@C Search for something magical to increase my strength.@n\r\n");
                        sendText("@B4@W)@C Practice my favorite skills instead of working just on my body.@n\r\n");
                        sendText("@B5@W)@C Spar with a friend so we can both improve our abilities.@n\r\n");
                        sendText("@w\r\nMake a selection:@n\r\n");
                        state = CON_Q1;
                        break;
                    case '3':
                        ch->playerFlags.set(PLR_SENSEM);
                        sendText("\r\n@RAnswer The following questions carefully, they may construct your alignment in conflict with your trainer or your stats contrary to your liking.\r\n\r\n");
                        sendText("@YAnswer the following question:\r\n");
                        sendText("@wYou go to train one day, but do not know the best\r\nway to approach it, What do you do?\r\n");
                        sendText("@D---------------------------------------@n\r\n");
                        sendText("@B1@W)@C Ask someone with more experience what to do.@n\r\n");
                        sendText("@B2@W)@C Jump in with some nice classic pushups!@n\r\n");
                        sendText("@B3@W)@C Search for something magical to increase my strength.@n\r\n");
                        sendText("@B4@W)@C Practice my favorite skills instead of working just on my body.@n\r\n");
                        sendText("@B5@W)@C Spar with a friend so we can both improve our abilities.@n\r\n");
                        sendText("@w\r\nMake a selection:@n\r\n");
                        state = CON_Q1;
                        break;
                    default:
                        sendText("@wThat is not a correct selection, try again.@n\r\n");
                        return;
                }
                break;
        }
    }

    void ChargenParser::finish() {
        // CREATE PLAYER ENTRY
        ch->id = nextCharID();
        ch->pref.set(PRF_COLOR);
        ch->generation = time(nullptr);
        check_unique_id(ch);
        add_unique_id(ch);
        auto &p = players[ch->id];
        p.name = ch->name;
        p.id = ch->id;
        p.account = conn->account;
        conn->account->characters.push_back(p.id);
        dirty_accounts.insert(conn->account->vn);
        p.character = ch;
        init_char(ch);
        ch->save();
        // set state to -1 to prevent accidental freeing of ch...
        state = -1;
        send_to_imm("New Character '%s' created by Account: %s", ch->name, p.account->name.c_str());
        conn->setParser(new CharacterMenu(conn, ch));
    }
}