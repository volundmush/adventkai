#include "dbat/transformation.h"

#include "dbat/comm.h"
#include "dbat/utils.h"
#include "dbat/weather.h"

namespace trans {
    static std::string getCustomName(struct char_data* ch, FormID form) {
        return "Unknown";
    }

    std::string getName(struct char_data* ch, FormID form) {
        switch (form) {
            case FormID::Base:
                return "Base";
            case FormID::Custom1:
            case FormID::Custom2:
            case FormID::Custom3:
            case FormID::Custom4:
            case FormID::Custom5:
            case FormID::Custom6:
            case FormID::Custom7:
            case FormID::Custom8:
            case FormID::Custom9:
                return getCustomName(ch, form);

            // Saiyan'y forms.
            case FormID::Oozaru:
                return "Oozaru";
            case FormID::GoldenOozaru:
                return "@YGolden Oozaru@n";
            case FormID::SuperSaiyan:
                return "@YSuper @CSaiyan @WFirst@n";
            case FormID::SuperSaiyan2:
                return "@YSuper @CSaiyan @WSecond@n";
            case FormID::SuperSaiyan3:
                return "@YSuper @CSaiyan @WThird@n";
            case FormID::SuperSaiyan4:
                return "@YSuper @CSaiyan @RFourth@n";
            case FormID::SuperSaiyanGod:
                return "@YSuper @CSaiyan @RGod@n";
            case FormID::SuperSaiyanBlue:
                return "@YSuper @CSaiyan @BBlue@n";

            // Human'y forms.
            case FormID::SuperHuman:
                return "@YSuper @CHuman @WFirst@n";
            case FormID::SuperHuman2:
                return "@YSuper @CHuman @WSecond@n";
            case FormID::SuperHuman3:
                return "@YSuper @CHuman @WThird@n";
            case FormID::SuperHuman4:
                return "@YSuper @CHuman @WFourth@n";

            // Icer'y forms.
            case FormID::IcerFirst:
                return "@YTransform @WFirst@n";
            case FormID::IcerSecond:
                return "@YTransform @WSecond@n";
            case FormID::IcerThird:
                return "@YTransform @WThird@n";
            case FormID::IcerFourth:
                return "@YTransform @WFourth@n";
            case FormID::IcerMetal:
                return "@YTransform @WMetal@n";
            case FormID::IcerGolden:
                return "@YTransform @YGolden@n";
            case FormID::IcerBlack:
                return "@YTransform @XBlack@n";

            // Konatsu'y forms.
            case FormID::ShadowFirst:
                return "@YShadow @WFirst@n";
            case FormID::ShadowSecond:
                return "@YShadow @WSecond@n";
            case FormID::ShadowThird:
                return "@YShadow @WThird@n";

            // Namekian'y forms.
            case FormID::SuperNamekian:
                return "@YSuper @CNamekian @WFirst@n";
            case FormID::SuperNamekian2:
                return "@YSuper @CNamekian @WSecond@n";
            case FormID::SuperNamekian3:
                return "@YSuper @CNamekian @WThird@n";
            case FormID::SuperNamekian4:
                return "@YSuper @CNamekian @WFourth@n";

            // Mutant Forms.
            case FormID::MutateFirst:
                return "@YMutate @WFirst@n";
            case FormID::MutateSecond:
                return "@YMutate @WSecond@n";
            case FormID::MutateThird:
                return "@YMutate @WThird@n";

            // BioAndroid
            case FormID::BioMature:
                return "@YMature@n";
            case FormID::BioSemiPerfect:
                return "@YSemi-Perfect@n";
            case FormID::BioPerfect:
                return "@YPerfect@n";
            case FormID::BioSuperPerfect:
                return "@YSuper Perfect@n";

            // Android
            case FormID::Android10:
                return "@YVersion 1.0@n";
            case FormID::Android20:
                return "@YVersion 2.0@n";
            case FormID::Android30:
                return "@YVersion 3.0@n";
            case FormID::Android40:
                return "@YVersion 4.0@n";
            case FormID::Android50:
                return "@YVersion 5.0@n";
            case FormID::Android60:
                return "@YVersion 6.0@n";

            // Majin
            case FormID::MajAffinity:
                return "@YAffinity@n";
            case FormID::MajSuper:
                return "@YSuper@n";
            case FormID::MajTrue:
                return "@YTrue@n";

            // Kai
            case FormID::MysticFirst:
                return "@YMystic @WFirst@n";
            case FormID::MysticSecond:
                return "@YMystic @WSecond@n";
            case FormID::MysticThird:
                return "@YMystic @WThird@n";

            // Tuffle
            case FormID::AscendFirst:
                return "@YAscend @WFirst@n";
            case FormID::AscendSecond:
                return "@YAscend @WSecond@n";
            case FormID::AscendThird:
                return "@YAscend @WThird@n";

            // Whoops?
            default:
                return "Unknown";
        }
    }

    static std::string getCustomAbbr(struct char_data* ch, FormID form) {
        return "N/A";
    }


    std::string getAbbr(struct char_data* ch, FormID form) {
        switch (form) {
            case FormID::Base:
                return "Base";
            case FormID::Custom1:
            case FormID::Custom2:
            case FormID::Custom3:
            case FormID::Custom4:
            case FormID::Custom5:
            case FormID::Custom6:
            case FormID::Custom7:
            case FormID::Custom8:
            case FormID::Custom9:
                return getCustomAbbr(ch, form);

            // Saiyan'y forms.
            case FormID::Oozaru:
                return "ooz";
            case FormID::GoldenOozaru:
                return "gooz";
            case FormID::SuperSaiyan:
                return "ssj";
            case FormID::SuperSaiyan2:
                return "ssj2";
            case FormID::SuperSaiyan3:
                return "ssj3";
            case FormID::SuperSaiyan4:
                return "ssj4";
            case FormID::SuperSaiyanGod:
                return "god";
            case FormID::SuperSaiyanBlue:
                return "blue";

            // Human'y forms.
            case FormID::SuperHuman:
                return "first";
            case FormID::SuperHuman2:
                return "second";
            case FormID::SuperHuman3:
                return "third";
            case FormID::SuperHuman4:
                return "fourth";

            // Icer'y forms.
            case FormID::IcerFirst:
                return "first";
            case FormID::IcerSecond:
                return "second";
            case FormID::IcerThird:
                return "third";
            case FormID::IcerFourth:
                return "fourth";
            case FormID::IcerMetal:
                return "metal";
            case FormID::IcerGolden:
                return "golden";
            case FormID::IcerBlack:
                return "black";

            // Konatsu'y forms.
            case FormID::ShadowFirst:
                return "first";
            case FormID::ShadowSecond:
                return "second";
            case FormID::ShadowThird:
                return "third";

            // Namekian'y forms.
            case FormID::SuperNamekian:
                return "first";
            case FormID::SuperNamekian2:
                return "second";
            case FormID::SuperNamekian3:
                return "third";
            case FormID::SuperNamekian4:
                return "fourth";

            // Mutant Forms.
            case FormID::MutateFirst:
                return "first";
            case FormID::MutateSecond:
                return "second";
            case FormID::MutateThird:
                return "third";

            // BioAndroid
            case FormID::BioMature:
                return "mature";
            case FormID::BioSemiPerfect:
                return "semi-perfect";
            case FormID::BioPerfect:
                return "perfect";
            case FormID::BioSuperPerfect:
                return "super-perfect";

            // Android
            case FormID::Android10:
                return "1.0";
            case FormID::Android20:
                return "2.0";
            case FormID::Android30:
                return "3.0";
            case FormID::Android40:
                return "4.0";
            case FormID::Android50:
                return "5.0";
            case FormID::Android60:
                return "6.0";

            // Majin
            case FormID::MajAffinity:
                return "affinity";
            case FormID::MajSuper:
                return "super";
            case FormID::MajTrue:
                return "true";

            // Kai
            case FormID::MysticFirst:
                return "first";
            case FormID::MysticSecond:
                return "second";
            case FormID::MysticThird:
                return "third";

            // Tuffle
            case FormID::AscendFirst:
                return "first";
            case FormID::AscendSecond:
                return "second";
            case FormID::AscendThird:
                return "third";

            // Whoops?
            default:
                return "Unknown";
        }
    }

    struct trans_affect_type {
        int location{};
        double modifier{0.0};
        int specific{-1};
        std::function<double(struct char_data *ch)> func{};
    };

    static double getModifierHelper(char_data* ch, FormID form, int location, int specific);

    static std::unordered_map<FormID, std::vector<trans_affect_type>> trans_affects = {

        // Saiyan'y forms...
        {
            FormID::Oozaru, {
                {APPLY_HEIGHT_MULT, 9.0},
                {APPLY_WEIGHT_MULT, 49.0},
                {APPLY_ALL_VITALS, 10000},
                {APPLY_DEFENSE_PERC, -0.3},
                {APPLY_DAMAGE_PERC, 0.3},
                {APPLY_VITALS_MULT, 1.0}
            }
        },
        {
            FormID::GoldenOozaru, {
                {APPLY_HEIGHT_MULT, 9.0},
                {APPLY_WEIGHT_MULT, 49.0},
                {APPLY_ALL_VITALS, 0.0, -1, [](struct char_data *ch) {
                    if(ch->transforms.contains(FormID::SuperSaiyan4)) return getModifierHelper(ch, FormID::SuperSaiyan4, APPLY_ALL_VITALS, -1);
                    if(ch->transforms.contains(FormID::SuperSaiyan3)) return getModifierHelper(ch, FormID::SuperSaiyan3, APPLY_ALL_VITALS, -1);
                    if(ch->transforms.contains(FormID::SuperSaiyan2)) return getModifierHelper(ch, FormID::SuperSaiyan2, APPLY_ALL_VITALS, -1);
                    return getModifierHelper(ch, FormID::SuperSaiyan, APPLY_ALL_VITALS, -1);
                }},
                {APPLY_DEFENSE_PERC, -0.3},
                {APPLY_DAMAGE_PERC, 0.3},
                {APPLY_VITALS_MULT, 0.0, -1, [](struct char_data *ch) {
                    if(ch->transforms.contains(FormID::SuperSaiyan4)) return getModifierHelper(ch, FormID::SuperSaiyan4, APPLY_VITALS_MULT, -1);
                    if(ch->transforms.contains(FormID::SuperSaiyan3)) return getModifierHelper(ch, FormID::SuperSaiyan3, APPLY_VITALS_MULT, -1);
                    if(ch->transforms.contains(FormID::SuperSaiyan2)) return getModifierHelper(ch, FormID::SuperSaiyan2, APPLY_VITALS_MULT, -1);
                    return getModifierHelper(ch, FormID::SuperSaiyan, APPLY_VITALS_MULT, -1);
                }},
            }
        },

        {
            FormID::SuperSaiyan, {
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 800000},
            }
        },

        {
            FormID::SuperSaiyan2, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 20000000},
            }
        },

        {
            FormID::SuperSaiyan3, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 80000000},
            }
        },

        {
            FormID::SuperSaiyan4, {
                {APPLY_VITALS_MULT, 4.5},
                {APPLY_ALL_VITALS, 182000000},
            }
        },

        // Human'y forms.
        {
            FormID::SuperHuman, {
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 1000000},
            }
        },

        {
            FormID::SuperHuman2, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 12000000},
            }
        },

        {
            FormID::SuperHuman3, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 50000000},
            }
        },

        {
            FormID::SuperHuman4, {
                {APPLY_VITALS_MULT, 3.5},
                {APPLY_ALL_VITALS, 270000000},
            }
        },

        // Icer'y Forms.
        {
            FormID::IcerFirst, {
                {APPLY_HEIGHT_MULT, 2.0},
                {APPLY_WEIGHT_MULT, 3.0},
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 400000},


            }
        },
        {
            FormID::IcerSecond, {
                {APPLY_HEIGHT_MULT, 2.0},
                {APPLY_WEIGHT_MULT, 3.0},
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 7000000},
            }
        },

        {
            FormID::IcerThird, {
                {APPLY_HEIGHT_MULT, .5},
                {APPLY_WEIGHT_MULT, 1.0},
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 45000000},
            }
        },

        {
            FormID::IcerFourth, {
                {APPLY_HEIGHT_MULT, 1.0},
                {APPLY_WEIGHT_MULT, 2.0},
                {APPLY_VITALS_MULT, 4.0},
                {APPLY_ALL_VITALS, 200000000},
            }
        },

        // Namekian Forms.
        {
            FormID::SuperNamekian, {
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 200000},
            }
        },
        {
            FormID::SuperNamekian2, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 4000000},
            }
        },
        {
            FormID::SuperNamekian3, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 65000000},
            }
        },
        {
            FormID::SuperNamekian4, {
                {APPLY_VITALS_MULT, 3.5},
                {APPLY_ALL_VITALS, 230000000},
            }
        },
        // Mutant Forms.
        {
            FormID::MutateFirst, {
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 100000},
            }
        },
        {
            FormID::MutateSecond, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 8500000},
            }
        },
        {
            FormID::MutateThird, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 80000000},
            }
        },
        // BioAndroid forms.
        {
            FormID::BioMature, {
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 1000000},
            }
        },
        {
            FormID::BioSemiPerfect, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 8000000},
            }
        },
        {
            FormID::BioPerfect, {
                {APPLY_VITALS_MULT, 2.5},
                {APPLY_ALL_VITALS, 70000000},
            }
        },
        {
            FormID::BioSuperPerfect, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 400000000},
            }
        },

        // Android Forms
        {
            FormID::Android10, {
                {APPLY_ALL_VITALS, 12500000},
            }
        },
        {
            FormID::Android20, {
                {APPLY_ALL_VITALS, 50000000},
            }
        },
        {
            FormID::Android30, {
                {APPLY_ALL_VITALS, 312000000},
            }
        },
        {
            FormID::Android40, {
                {APPLY_ALL_VITALS, 2500000000},
            }
        },
        {
            FormID::Android50, {
                {APPLY_ALL_VITALS, 5000000000},
            }
        },
        {
            FormID::Android60, {
                {APPLY_ALL_VITALS, 10000000000},
            }
        },

        // Majin Forms
        {
            FormID::MajAffinity, {
                {APPLY_VITALS_MULT, 1.0},
                {APPLY_ALL_VITALS, 1250000},
            }
        },
        {
            FormID::MajAffinity, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 15000000},
            }
        },
        {
            FormID::MajAffinity, {
                {APPLY_VITALS_MULT, 3.5},
                {APPLY_ALL_VITALS, 340000000},
            }
        },

        // Kai Forms
        {
            FormID::MysticFirst, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 1100000},
            }
        },
        {
            FormID::MysticSecond, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 115000000},
            }
        },
        {
            FormID::MysticThird, {
                {APPLY_VITALS_MULT, 4.0},
                {APPLY_ALL_VITALS, 270000000},
            }
        },
        // Tuffle Forms
        {
            FormID::AscendFirst, {
                {APPLY_VITALS_MULT, 2.0},
                {APPLY_ALL_VITALS, 1300000},
            }
        },
        {
            FormID::AscendSecond, {
                {APPLY_VITALS_MULT, 3.0},
                {APPLY_ALL_VITALS, 80000000},
            }
        },
        {
            FormID::AscendSecond, {
                {APPLY_VITALS_MULT, 4.0},
                {APPLY_ALL_VITALS, 300000000},
            }
        },

    };

    static double getModifierHelper(char_data* ch, FormID form, int location, int specific) {
        if (form == FormID::Base) return 0.0;
        double out = 0.0;
        if (auto found = trans_affects.find(form); found != trans_affects.end()) {
            for (auto& affect: found->second) {
                if (affect.location == location) {if(specific != -1 && specific != affect.specific) continue;
                    out += affect.modifier;
                    if(affect.func) {
                        out += affect.func(ch);
                    }
                }
            }

        }

        return out;
    }

    double getModifier(char_data* ch, int location, int specific) {
        return getModifierHelper(ch, ch->form, location, specific);
    }

    static std::unordered_map<FormID, double> trans_drain = {
        // Saiyan forms.
        {FormID::SuperSaiyan, .06},
        {FormID::SuperSaiyan2, .08},
        {FormID::SuperSaiyan3, .12},
        {FormID::SuperSaiyan4, .14},

        // Human Forms
        {FormID::SuperHuman, .06},
        {FormID::SuperHuman2, .08},
        {FormID::SuperHuman3, .12},
        {FormID::SuperHuman4, .14},

        // icer Forms
        {FormID::IcerFirst, .06},
        {FormID::IcerSecond, .08},
        {FormID::IcerThird, .12},
        {FormID::IcerFourth, .14},

        // Konatsu Forms
        {FormID::ShadowFirst, .06},
        {FormID::ShadowSecond, .08},
        {FormID::ShadowThird, .1},

        // Namekian Forms
        {FormID::SuperNamekian, .06},
        {FormID::SuperNamekian2, .07},
        {FormID::SuperNamekian3, .08},
        {FormID::SuperNamekian4, .1},

        // Mutant Forms
        {FormID::MutateFirst, .05},
        {FormID::MutateSecond, .07},
        {FormID::MutateThird, .1},

        // Bio, Android, Majin, have no drain.

        // Kai
        {FormID::MysticFirst, .05},
        {FormID::MysticSecond, .07},
        {FormID::MysticThird, .1}

    };

    double getStaminaDrain(char_data* ch, FormID form, bool upkeep) {
        if (ch->form == FormID::Base) return 0.0;

        double drain = 0.0;

        if (auto found = trans_drain.find(form); found != trans_drain.end()) {
            drain = found->second;
        }

        if(form == FormID::SuperSaiyan && PLR_FLAGGED(ch, PLR_FPSSJ)) drain *= 0.5;

        if(upkeep) {
            drain *= 0.01;
            if(ch->race == RaceID::Icer) drain = 0.0;
        }
        return drain;
    }

    void gamesys_transform(uint64_t heartPulse, double deltaTime) {
        for(auto ch = character_list; ch; ch = ch->next) {
            auto form = ch->form;
            auto &data = ch->transforms[form];
            data.timeSpentInForm += deltaTime;

            if(ch->form == FormID::Oozaru || form == FormID::GoldenOozaru) {
                if(auto room = ch->getRoom(); room) {
                    // Top off the blutz.
                    if(room->checkMoon() == MoonCheck::Full) data.blutz = 60.0 * 30;
                }
                data.blutz -= deltaTime;
                if(data.blutz <= 0 || !ch->playerFlags.test(PLR_TAIL)) {
                    data.blutz = 0.0;
                    oozaru_revert(ch);
                }

            }

            // Check stamina drain.
            if (auto drain = getStaminaDrain(ch, ch->form, true) * deltaTime; drain > 0) {
                if(ch->decCurSTPercent(drain) == 0) {
                    if(!blockRevertDisallowed(ch, FormID::Base)) {
                        act("@mExhausted of stamina, your body forcibly reverts from its form.@n", true, ch, nullptr,
                            nullptr, TO_CHAR);
                        act("@C$n @wbreathing heavily, reverts from $s form, returning to normal.@n", true, ch, nullptr,
                            nullptr, TO_ROOM);
                        ch->form = FormID::Base;
                        ch->remove_kaioken(true);
                    }
                }
            }
        }
    }

    struct trans_echo {
        std::string self;
        std::string room;
    };

    static std::unordered_map<FormID, trans_echo> trans_echoes = {
            {
                    FormID::SuperSaiyan,     {
                                                     "@WSomething inside your mind snaps as your rage spills over! Lightning begins to strike the ground all around you as you feel torrents of power rushing through every fiber of your being. Your hair suddenly turns golden as your eyes change to the color of emeralds. In a final rush of power a golden aura rushes up around your body! You have become a @CSuper @YSaiyan@W!@n",
                                                     "@C$n@W screams in rage as lightning begins to crash all around! $s hair turns golden and $s eyes change to an emerald color as a bright golden aura bursts up around $s body! As $s energy stabilizes $e wears a fierce look upon $s face, having transformed into a @CSuper @YSaiyan@W!@n"
                                             }
            },

            // Old LSSJ: "@WYou roar and then stand at your full height. You flex every muscle in your body as you feel your strength grow! Your eyes begin to glow @wwhite@W with energy, your hair turns @Ygold@W, and at the same time a @wbright @Yg@yo@Yl@yd@Ye@yn@W aura flashes up around your body! You release your @YL@ye@Dg@We@wn@Yd@ya@Dr@Yy@W power upon the universe!@n"
            // OLd LSSJ: "@C$n @Wroars and then stands at $s full height. Then $s muscles start to buldge and grow as $e flexes them! Suddenly $s eyes begin to glow @wwhite@W with energy, $s hair turns @Ygold@W, and at the same time a @wbright @Yg@yo@Yl@yd@Ye@yn@W aura flashes up around $s body! @C$n@W releases $s @YL@ye@Dg@We@wn@Yd@ya@Dr@Yy@W power upon the universe!@n"

            {
                    FormID::SuperSaiyan2,    {
                                                     "@WBlinding rage burns through your mind as a sudden eruption of energy surges forth! A golden aura bursts up around your body, glowing as bright as the sun. Rushing winds rocket out from your body in every direction as bolts of electricity begin to crackle in your aura. As your aura dims you are left standing confidently, having achieved @CSuper @YSaiyan @GSecond@W!@n",
                                                     "@C$n@W stands up straight with $s head back as $e releases an ear piercing scream! A blindingly bright golden aura bursts up around $s body, glowing as bright as the sun. As rushing winds begin to rocket out from $m in every direction, bolts of electricity flash and crackle in $s aura. As $s aura begins to dim $e is left standing confidently, having achieved @CSuper @YSaiyan @GSecond@W!@n"
                                             }
            },

            {
                    FormID::SuperSaiyan3,    {
                                                     "@WElectricity begins to crackle around your body as your aura grows explosively! You yell as your powerlevel begins to skyrocket while your hair grows to multiple times the length it was previously. Your muscles become incredibly dense instead of growing in size, preserving your speed. Finally your irises appear just as your transformation becomes complete, having achieved @CSuper @YSaiyan @GThird@W!@n",
                                                     "@WElectricity begins to crackle around @C$n@W, as $s aura grows explosively! $e yells as the energy around $m skyrockets and $s hair grows to multiple times its previous length. $e smiles as $s irises appear and $s muscles tighten up. $s transformation complete, $e now stands confidently, having achieved @CSuper @YSaiyan @GThird@W!@n"
                                             }
            },

            {
                    FormID::SuperSaiyan4,    {
                                                     "@WHaving absorbed enough blutz waves, your body begins to transform! Red fur grows over certain parts of your skin as your hair grows longer and unkempt. A red outline forms around your eyes while the irises of those very same eyes change to an amber color. Energy crackles about your body violently as you achieve the peak of saiyan perfection, @CSuper @YSaiyan @GFourth@W!@n",
                                                     "@WHaving absorbed enough blutz waves, @C$n@W's body begins to transform! Red fur grows over certain parts of $s skin as $s hair grows longer and unkempt. A red outline forms around $s eyes while the irises of those very same eyes change to an amber color. Energy crackles about $s body violently as $e achieves the peak of saiyan perfection, @CSuper @YSaiyan @GFourth@W!@n"
                                             }
            },


            // Human Forms
            {
                    FormID::SuperHuman,      {
                                                     "@WYou spread your feet out and crouch slightly as a bright white aura bursts around your body. Torrents of white and blue energy burn upwards around your body while your muscles grow and become more defined at the same time. In a sudden rush of power you achieve @CSuper @cHuman @GFirst@W sending surrounding debris high into the sky!",
                                                     "@C$n@W crouches slightly while spreading $s feet as a bright white aura bursts up around $s body. Torrents of white and blue energy burn upwards around $s body while $s muscles grow and become more defined at the same time. In a sudden rush of power debris is sent flying high into the air with $m achieving @CSuper @cHuman @GFirst@W!"
                                             }
            },

            {
                    FormID::SuperHuman2,     {
                                                     "@WSuddenly a bright white aura bursts into existance around your body, you feel the intensity of your hidden potential boil until it can't be contained any longer! Waves of ki shoot out from your aura streaking outwards in many directions. A roar that shakes everything in the surrounding area sounds right as your energy reaches its potential and you achieve @CSuper @cHuman @GSecond@W!",
                                                     "@C$n@W is suddenly covered with a bright white aura as $e grits $s teeth, apparently struggling with the power boiling to the surface! Waves of ki shoot out from $s aura, streaking in several directions as a mighty roar shakes everything in the surrounding area. As $s aura calms $e smiles, having achieved @CSuper @cHuman @GSecond@W!"
                                             }
            },

            {
                    FormID::SuperHuman3,     {
                                                     "@WYou clench both of your fists as the bright white aura around your body is absorbed back into your flesh. As it is absorbed, your muscles triple in size and electricity crackles across your flesh. You grin as you feel the power of @CSuper @cHuman @GThird@W!",
                                                     "@C$n@W clenches both of $s fists as the bright white aura around $s body is absorbed back into $s flesh. As it is absorbed, $s muscles triple in size and bright electricity crackles across $s flesh. $e smiles as $e achieves the power of @CSuper @cHuman @GThird@W!"
                                             }
            },

            {
                    FormID::SuperHuman4,     {
                                                     "@WYou grit your teeth and clench your fists as a sudden surge of power begins to tear through your body! Your muscles lose volume and gain mass, condensing into sleek hyper efficiency as a spectacular shimmering white aura flows over you, flashes of multicolored light flaring up in rising stars around your new form, a corona of glory! You feel your ultimate potential realized as you ascend to @CSuper @cHuman @GFourth@W!@n",
                                                     "@C$n@W grits $s teeth and clenches $s fists as a sudden surge of power begins to tear through $s body! $n@W's muscles lose volume and gain mass, condensing into sleek hyper efficiency as a spectacular shimmering white aura flows over $m, flashes of multicolored light flare up in rising stars around $s new form, a corona of glory! $n@W smiles as his ultimate potential is realized as $e ascends to @CSuper @cHuman @GFourth@W!@n"
                                             }
            },

            // Icer Forms
            {
                    FormID::IcerFirst,       {
                                                     "@WYou yell with pain as your body begins to grow and power surges within! Your legs expand outward to triple their previous length. Soon after your arms, chest, and head follow. Your horns grow longer and curve upwards while lastly your tail expands. You are left confidently standing, having completed your @GFirst @cTransformation@W.@n",
                                                     "@C$n@W yells with pain as $s body begins to grow and power surges outward! $s legs expand outward to triple their previous length. Soon after $s arms, chest, and head follow. $s horns grow longer and curve upwards while lastly $s tail expands. $e is left confidently standing, having completed $s @GFirst @cTransformation@W.@n"
                                             }
            },

            {
                    FormID::IcerSecond,      {
                                                     "@WSpikes grow out from your elbows as your power begins to climb to new heights. The muscles along your forearms grow to double their former size as the spikes growing from your elbows flatten and sharpen into blades. You have achieved your @GSecond @mMutation@W!@n",
                                                     "@WSpikes grow out from @C$n@W's elbows as $s power begins to climb to new heights. The muscles along $s forearms grow to double their former size as the spikes growing from $s elbows flatten and sharpen into blades. $e has achieved your @GSecond @mMutation@W!@n"
                                             }
            },

            {
                    FormID::IcerThird,       {
                                                     "@WA blinding light surrounds your body while your rising power begins to rip up the ground beneath you! Your skin and torso shell begin to crack as your new body struggles to free its self. Huge chunks of debris lift free of the ground as your power begins to rise to unbelievable heights. Suddenly your old skin and torso shell burst off from your body, leaving a sleek form glowing where they had been. Everything comes crashing down as your power evens out, leaving you with your @GThird @cTransformation @Wcompleted!@n",
                                                     "@WA blinding light surrounds @C$n@W's body while $s rising power begins to rip up the ground beneath $m! $s skin and torso shell begin to crack as $s new body struggles to free its self. Huge chunks of debris lift free of the ground as $s power begins to rise to unbelievable heights. Suddenly $s old skin and torso shell burst off from $s body, leaving a sleek form glowing where they had been. Everything comes crashing down as @C$n@W's power evens out, leaving $m with $s @GThird @cTransformation @Wcompleted!@n"
                                             }
            },

            {
                    FormID::IcerFourth,      {
                                                     "@WA feeling of complete power courses through your viens as your body begins to change radically! You triple in height while a hard shell forms over your entire torso. Hard bones grow out from your head forming four ridges that jut outward. A hard covering grows up over your mouth and nose completing the transformation! A dark crimson aura flames around your body as you realize your @GFourth @cTransformation@W!@n",
                                                     "@C$n@W's body begins to change radically! $e triples in height while a hard shell forms over $s entire torso. Hard bones grow out from $s head forming four ridges that jut outward. A hard covering grows up over $s mouth and nose completing the transformation! A dark crimson aura flames around @C$n@W's body as $e realizes $s @GFourth @cTransformation@W!@n"
                                             }
            },


            // Konatsu Forms
            {
                    FormID::ShadowFirst,     {
                                                     "@WA dark shadowy aura with flecks of white energy begins to burn around your body! Strength and agility can be felt rising up within as your form becomes blurred and ethereal looking. You smile as you realize your @GFirst @DShadow @BForm@W!@n",
                                                     "@WA dark shadowy aura with flecks of white energy begins to burn around @C$n@W's body! $s form becomes blurred and ethereal-looking as $s muscles become strong and lithe. $e smiles as $e achieves $s @GFirst @DShadow @BForm@W!@n"
                                             }
            },

            {
                    FormID::ShadowSecond,    {
                                                     "@WThe shadowy aura surrounding your body burns larger than ever as dark bolts of purple electricity crackles across your skin. Your eyes begin to glow white as shockwaves of power explode outward! All the shadows in the immediate area are absorbed into your aura in an instant as you achieve your @GSecond @DShadow @BForm@W!@n",
                                                     "@WThe shadowy aura surrounding @C$n@W's body burns larger than ever as dark bolts of purple electricity crackles across $s skin. $s eyes begin to glow white as shockwaves of power explode outward! All the shadows in the immediate area are absorbed into $s aura in an instant as $e achieves $s @GSecond @DShadow @BForm@W!@n"
                                             }
            },

            {
                    FormID::ShadowThird,     {
                                                     "@WThe shadowy aura around you explodes outward as your power begins to rise!  You're overcome with a sudden realization, that the shadows are an extension of yourself, that light isn't needed for your shadows to bloom.  With this newfound wisdom comes ability and power!  The color in your aura drains as the shadows slide inward and cling to your body like a second, solid black skin!  Shockwaves roll off of you in quick succession, pelting the surrounding area harshly!  Accompanying the waves, a pool of darkness blossoms underneath you, slowly spreading the shadows to the whole area, projecting onto any surface nearby!  Purple and black electricity crackle in your solid white aura, and you grin as you realize your @GThird @DShadow @BForm@W!@n",
                                                     "@WThe shadowy aura around $n explodes outward as $s power begins to rise!  Realization dawns on $s face, followed shortly by confidence! The color in $s aura drains as the shadows slide inward to cling to $s body like a second, solid black skin! Shockwaves roll off of $n in quick succession, pelting the surrounding area harshly!  Accompanying the waves, a pool of darkness blossoms underneath them, slowly spreading the shadows to the whole area, projecting onto any surface nearby! Purple and black electricity crackle in $s solid white aura, and he grins as $e realizes $s @GThird @DShadow @BForm@W!@n"
                                             }
            },

            // Namekian Forms
            {
                    FormID::SuperNamekian,   {
                                                     "@WYou crouch down and clench your fists as your muscles begin to bulge! Sweat pours down your body as the ground beneath your feet cracks and warps under the pressure of your rising ki! With a sudden burst that sends debris flying you realize a new plateau in your power, having achieved @CSuper @gNamek @GFirst@W!@n",
                                                     "@C$n @Wcrouches down and clenches $s fists as $s muscles begin to bulge! Sweat pours down $s body as the ground beneath $s feet cracks and warps under the pressure of  $s rising ki! With a sudden burst that sends debris flying $e seems to realize a new plateau in $s power, having achieved @CSuper @gNamek @GFirst@W!@n"
                                             }
            },
            {
                    FormID::SuperNamekian2,  {
                                                     "@WYou gasp in shock as a power within your body that you had not been aware of begins to surge to the surface! Your muscles grow larger as energy crackles between your antennae intensely! A shockwave of energy explodes outward as you achieve a new plateau in power, @CSuper @gNamek @GSecond@W!@n",
                                                     "@C$n @Wgasps in shock as a power within $s body begins to surge out! $s muscles grow larger as energy crackles between $s antennae intensely! A shockwave of energy explodes outward as $e achieves a new plateau in power, @CSuper @gNamek @GSecond@W!@n"
                                             }
            },
            {
                    FormID::SuperNamekian3,  {
                                                     "@WA fierce clear aura bursts up around your body as you struggle to control a growing power within! Energy leaks off of your aura at an astounding rate filling the air around you with small orbs of ki. As your power begins to level off the ambient ki hovering around you is absorbed inward in a sudden shock that leaves your skin glowing! You have achieved a rare power, @CSuper @gNamek @GThird@W!@n",
                                                     "@WA fierce clear aura bursts up around @C$n@W's body as $e struggles to control $s own power! Energy leaks off of $s aura at an astounding rate filling the air around $m with small orbs of ki. As $s power begins to level off the ambient ki hovering around $m is absorbed inward in a sudden shock that leaves $s skin glowing! $e has achieved a rare power, @CSuper @gNamek @GThird@W!@n"
                                             }
            },
            {
                    FormID::SuperNamekian4,  {
                                                     "@WAn inner calm fills your mind as your power surges higher than ever before. Complete clarity puts everything once questioned into perspective. While this inner calm is filling your mind, an outer storm of energy erupts around your body! The storm of energy boils and crackles while growing larger. You have achieved @CSuper @gNamek @GFourth@W, a mystery of the ages.@n",
                                                     "@C$n@W smiles calmly as a look of complete understand fills $s eyes. While $e remains perfectly calm and detached a massivly powerful storm of energy erupts from his body. This storm of energy shimmers with the colors of the rainbow and boils and crackles with awesome power! $s smile disappears as he realizes a mysterious power of the ages, @CSuper @gNamek @GFourth@W!@n"
                                             }
            },

            {
                    FormID::MutateFirst,     {
                                                     "@WYour flesh grows tougher as power surges up from within. Your fingernails grow longer, sharper, and more claw-like. Lastly your muscles double in size as you achieve your @GFirst @mMutation@W!@n",
                                                     "@C$n@W flesh grows tougher as power surges up around $m. $s fingernails grow longer, sharper, and more claw-like. Lastly $s muscles double in size as $e achieves $s @GFirst @mMutation@W!@n"
                                             }
            },

            {
                    FormID::MutateSecond,    {
                                                     "@WSpikes grow out from your elbows as your power begins to climb to new heights. The muscles along your forearms grow to double their former size as the spikes growing from your elbows flatten and sharpen into blades. You have achieved your @GSecond @mMutation@W!@n",
                                                     "@WSpikes grow out from @C$n@W's elbows as $s power begins to climb to new heights. The muscles along $s forearms grow to double their former size as the spikes growing from $s elbows flatten and sharpen into blades. $e has achieved your @GSecond @mMutation@W!@n"
                                             }
            },

            {
                    FormID::MutateThird,     {
                                                     "@WA dark cyan aura bursts up around your body as the ground begins to crack beneath you! You scream out in pain as your power begins to explode! Two large spikes grow out from your shoulder blades as you reach your @GThird @mMutation!@n",
                                                     "@WA dark cyan aura bursts up around @C$n@W's body as the ground begins to crack beneath $m and $e screams out in pain as $s power begins to explode! Two large spikes grow out from $s shoulder blades as $e reaches $s @GThird @mMutation!@n"
                                             }
            },

            {
                    FormID::BioMature,       {
                                                     "@gYou bend over as @rpain@g wracks your body! Your limbs begin to grow out, becoming more defined and muscular. As your limbs finish growing outward you feel a painful sensation coming from your back as a long tail with a spike grows out of your back! As the pain subsides you stand up straight and a current of power shatters part of the ground beneath you. You have @rmatured@g beyond your @Gl@ga@Dr@gv@Ga@ge stage!@n",
                                                     "@W$n @gbends over as a @rpainful@g look covers $s face! $s limbs begin to grow out, becoming more defined and muscular. As $s limbs finish growing outward $e screams as a long tail with a spike grows rips out of $s back! As $e calms $e stands up straight and a current of power shatters part of the ground beneath $m. $e has @rmatured@g beyond $s @Gl@ga@Dr@gv@Ga@ge stage!@n"
                                             }
            },

            {
                    FormID::BioSemiPerfect,  {
                                                     "@WYour exoskeleton begins to glow spectacularly while the shape of your body begins to change. Your tail shrinks slightly. Your hands, feet, and facial features become more refined. While your body colors change slightly. The crests on your head change, standing up straighter on either side of your head as well. As you finish transforming a wave of power floods your being. You have achieved your @gSemi@D-@GPerfect @BForm@W!@n",
                                                     "@C$n@W's exoskeleton begins to glow spectacularly while the shape of $s body begins to change. $s tail shrinks slightly. $s hands, feet, and facial features become more refined. While $s body colors change slightly. The crests on $s head change, standing up straighter on either side of $s head as well. As $e finishes transforming a wave of power rushes out from $m. $e has achieved $s @gSemi@D-@GPerfect @BForm@W!@n"
                                             }
            },

            {
                    FormID::BioPerfect,      {
                                                     "@WYour whole body is engulfed in blinding light as your exoskeleton begins to change shape! Your hands, feet, and facial features become more refined and humanoid. While your colors change, becoming more subdued and neutral. A bright golden aura bursts up around your body as you achieve your @GPerfect @BForm@W!@n",
                                                     "@C$n@W whole body is engulfed in blinding light as $s exoskeleton begins to change shape! $s hands, feet, and facial features become more refined and humanoid. While $s colors change, becoming more subdued and neutral. A bright golden aura bursts up around $s body as $e achieves $s @GPerfect @BForm@W!@n"
                                             }
            },

            {
                    FormID::BioSuperPerfect, {
                                                     "@WA rush of power explodes from your perfect body, crushing nearby debris and sending dust billowing in all directions. Electricity crackles throughout your aura intensely while your muscles grow slightly larger but incredibly dense. You smile as you realize that you have taken your perfect form beyond imagination. You are now @CSuper @GPerfect@W!@n",
                                                     "@WA rush of power explodes from @C$n@W's perfect body, crushing nearby debris and sending dust billowing in all directions. Electricity crackles throughout $s aura intensely while $s muscles grow slightly larger but incredibly dense. $e smiles as $e has taken $s perfect form beyond imagination. $e is now @CSuper @GPerfect@W!@n"
                                             }
            },

            {
                    FormID::Android10,       {
                                                     "@WYou stop for a moment as the nano-machines within your body reprogram and restructure you. You are now more powerful and efficient!@n",
                                                     "@C$n @Wstops for a moment as the nano-machines within $s body reprogram and restructure $m. $e is now more powerful and efficient!@n"
                                             }
            },

            {
                    FormID::Android20,       {
                                                     "@WYou stop for a moment as the nano-machines within your body reprogram and restructure you. You are now more powerful and efficient!@n",
                                                     "@C$n @Wstops for a moment as the nano-machines within $s body reprogram and restructure $m. $e is now more powerful and efficient!@n"
                                             }
            },

            {
                    FormID::Android30,       {
                                                     "@WYou stop for a moment as the nano-machines within your body reprogram and restructure you. You are now more powerful and efficient!@n",
                                                     "@C$n @Wstops for a moment as the nano-machines within $s body reprogram and restructure $m. $e is now more powerful and efficient!@n"
                                             }
            },

            {
                    FormID::Android40,       {
                                                     "@WYou stop for a moment as the nano-machines within your body reprogram and restructure you. You are now more powerful and efficient!@n",
                                                     "@C$n @Wstops for a moment as the nano-machines within $s body reprogram and restructure $m. $e is now more powerful and efficient!@n"
                                             }
            },

            {
                    FormID::Android50,       {
                                                     "@WYou stop for a moment as the nano-machines within your body reprogram and restructure you. You are now more powerful and efficient!@n",
                                                     "@C$n @Wstops for a moment as the nano-machines within $s body reprogram and restructure $m. $e is now more powerful and efficient!@n"
                                             }
            },

            {
                    FormID::Android60,       {
                                                     "@WYou stop for a moment as the nano-machines within your body reprogram and restructure you. You are now more powerful and efficient!@n",
                                                     "@C$n @Wstops for a moment as the nano-machines within $s body reprogram and restructure $m. $e is now more powerful and efficient!@n"
                                             }
            },

            {
                    FormID::MajAffinity,     {
                                                     "@WA dark pink aura bursts up around your body as images of good and evil fill your mind! You feel the power within your body growing intensely, reflecting your personal alignment as your body changes!@n",
                                                     "@WA dark pink aura bursts up around @C$n@W's body as images of good and evil fill $s mind! $e feels the power within $s body growing intensely, reflecting $s personal alignment as $s body changes!@n"
                                             }
            },

            {
                    FormID::MajSuper,        {
                                                     "@WAn intense pink aura surrounds your body as it begins to change, taking on the characteristics of those you have ingested! Explosions of pink energy burst into existence all around you as your power soars to sights unseen!@n",
                                                     "@WAn intense pink aura surrounds @C$n@W's body as it begins to change, taking on the characteristics of those $e has ingested! Explosions of pink energy burst into existence all around $m as $s power soars to sights unseen!@n"
                                             }
            },

            {
                    FormID::MajTrue,         {
                                                     "@WRipples of intense pink energy rush upwards around your body as it begins to morph into its truest form! The ground beneath your feet forms into a crater from the very pressure of your rising ki! Earthquakes shudder throughout the area as your finish morphing!@n",
                                                     "@WRipples of intense pink energy rush upwards around @C$n@W's body as it begins to morph into its truest form! The ground beneath $s feet forms into a crater from the very pressure of $s rising ki! Earthquakes shudder throughout the area as $e finishes morphing!@n"
                                             }
            },

            {
                    FormID::MysticFirst,     {
                                                     "@WThoughts begin to flow through your mind of events throughout your life. The progression leads up to more recent events and finally to this very moment. All of it's significance overwhelms you momentarily and your motivation and drive increase. As your attention is drawn back to your surroundings, you feel as though your thinking, senses, and reflexes have sharpened dramatically.  At the core of your being, a greater depth of power can be felt.@n",
                                                     "@W$n@W's face tenses, it becoming clear momentarily that they are deep in thought. After a brief lapse in focus, their attention seems to return to their surroundings. Though it's not apparent why they were so distracted, something definitely seems different about $m.@n"
                                             }
            },

            {
                    FormID::MysticSecond,    {
                                                     "@WYou feel a sudden rush of emotion, escalating almost to a loss of control as your thoughts race. Your heart begins to beat fast as memories mix with the raw emotion. A faint blue glow begins to surround you. As your emotions level off, you feel a deeper understanding of the universe as you know it. You visibly calm back down to an almost steely eyed resolve as you assess your surroundings. The blue aura wicks around you for a few moments and then dissipates. Thought it's full impact is not yet clear to you, you are left feeling as though both your power and inner strength have turned into nearly bottomless wells.@n",
                                                     "@W$n@W's appears to be hit by some sudden pangs of agony, their face contorted in pain.  After a moment a faint blue aura appears around them, glowing brighter as time passes. You can feel something in the pit of your stomach, letting you know that something very significant is changing around you. Before long $n@W's aura fades, leaving a very determined looking person in your presence.@n"
                                             }
            },

            {
                    FormID::MysticThird,     {
                                                     "@WYour minds' eye becomes overwhelmed by secrets unimaginable. The threads of the very universe become visible in your heightened state of awareness. Reaching out, a single thread vibrates, producing a @Rred @Wcolor -- yours. Your fingertips brush against it and your senses become clouded by a vast expanse of white color and noise. As your vision and hearing return, you understand the threads tying every living being together. Your awareness has expanded beyond comprehension!@n",
                                                     "@C$n@W's eyes grow wide, mouth agape. $s body begins to shiver uncontrollably! $s arms reaches out cautiously before falling back down to $s side. $s face relaxes visibly, features returning to a normal state. $s irises remain larger than before, a slight smile softening $s gaze.@n"
                                             }
            },

            {
                    FormID::AscendFirst,     {
                                                     "@WYour mind accelerates working through the mysteries of the universe while at the same time your body begins to change! Innate nano-technology within your body begins to activate, forming flexible metal plating across parts of your skin!@n",
                                                     "@C$n@W begins to write complicated calculations in the air as though $e were possessed while at the same time $s body begins to change! Innate nano-technology within $s body begins to activate, forming flexible metal plating across parts of $s skin!@n"
                                             }
            },

            {
                    FormID::AscendSecond,    {
                                                     "@WComplete understanding of every physical thing floods your mind as the nano-technology within you continues to change your body! Your eyes change; becoming glassy, hard, and glowing. Your muscles merge with a nano-fiber strengthening them at the molecular level! Finally your very bones become plated in nano-metals that have yet to be invented naturally!@n",
                                                     "@C$n@.s nano-technology continues to change $s body! $s eyes change; becoming glassy, hard, and glowing. $s muscles merge with a nano-fiber strengthening them at the molecular level! Finally $s very bones become plated in nano-metals that have yet to be invented naturally!@n"
                                             }
            },

            {
                    FormID::AscendThird,     {
                                                     "@WYou have reached the final stage of enlightenment and the nano-technology thriving inside you begin to initiate the changes! Your neural pathways become refined, your reflexes honed, your auditory and ocular senses sharpening far beyond normal levels! Your gravitational awareness improves, increasing sensitivity and accuracy in your equilibrum!@n",
                                                     "@C$n begins to mumble quietly, slowly at first and gradually picking up speed. A glint is seen from $s eyes and $s arms reach outwards briefly as $e appears to catch his balance. $s arms drop back to $s sides as balance is regained, a vicious smile on $s face.@n"
                                             }
            },

            {       FormID::Oozaru,          {
                                                     "@rLooking up at the moon your heart begins to beat loudly. Sudden rage begins to fill your mind while your body begins to grow. Hair sprouts  all over your body and your teeth become sharp as your body takes on the Oozaru form!@n",
                                                     "@R$n@r looks up at the moon as $s eyes turn red and $s heart starts to beat loudly. Hair starts to grow all over $s body as $e starts screaming. The scream turns into a roar as $s body begins to grow into a giant ape!@n"
                                             }},
            {       FormID::GoldenOozaru,          {
                                                     "@rLooking up at the moon your heart begins to beat loudly. Sudden rage begins to fill your mind while your body begins to grow. Golden hair sprouts all over your body, your teeth sharpen, as your body takes on the Golden Oozaru form!@n",
                                                     "@R$n@r looks up at the moon as $s eyes turn red and $s heart starts to beat loudly. Golden hair starts to grow all over $s body as $e starts screaming. The scream turns into a roar as $s body begins to grow into a giant golden ape!@n"
                                             }},


    };

    void handleEchoTransform(struct char_data* ch, FormID form) {
        if (auto found = trans_echoes.find(form); found != trans_echoes.end()) {
            auto& echo = found->second;
            act(echo.self.c_str(), true, ch, nullptr, nullptr, TO_CHAR);
            act(echo.room.c_str(), true, ch, nullptr, nullptr, TO_ROOM);
        }
    }

    void handleEchoRevert(struct char_data* ch, FormID form) {
        if (form == FormID::Base) return;

        auto name = getName(ch, form);
        auto self = fmt::format("@wYou revert from {}.@n", name);
        auto room = fmt::format("@w$n@w reverts from {}.@n", name);

        act(self.c_str(), true, ch, nullptr, nullptr, TO_CHAR);
        act(room.c_str(), true, ch, nullptr, nullptr, TO_ROOM);
    }

    static const std::vector<FormID> forms = {
        // Universal
        FormID::Base,
        FormID::Custom1,
        FormID::Custom2,
        FormID::Custom3,
        FormID::Custom4,
        FormID::Custom5,
        FormID::Custom6,
        FormID::Custom7,
        FormID::Custom8,
        FormID::Custom9,

        // Saiyan'y forms.
        FormID::Oozaru,
        FormID::GoldenOozaru,
        FormID::SuperSaiyan,
        FormID::SuperSaiyan2,
        FormID::SuperSaiyan3,
        FormID::SuperSaiyan4,
        FormID::SuperSaiyanGod,
        FormID::SuperSaiyanBlue,

        // Human'y Forms
        FormID::SuperHuman,
        FormID::SuperHuman2,
        FormID::SuperHuman3,
        FormID::SuperHuman4,

        // Icer'y Forms
        FormID::IcerFirst,
        FormID::IcerSecond,
        FormID::IcerThird,
        FormID::IcerFourth,
        FormID::IcerMetal,
        FormID::IcerGolden,
        FormID::IcerBlack,

        // Konatsu
        FormID::ShadowFirst,
        FormID::ShadowSecond,
        FormID::ShadowThird,

        // Namekian
        FormID::SuperNamekian,
        FormID::SuperNamekian2,
        FormID::SuperNamekian3,
        FormID::SuperNamekian4,

        // Mutant
        FormID::MutateFirst,
        FormID::MutateSecond,
        FormID::MutateThird,

        // BioAndroid
        FormID::BioMature,
        FormID::BioSemiPerfect,
        FormID::BioPerfect,
        FormID::BioSuperPerfect,

        // Android
        FormID::Android10,
        FormID::Android20,
        FormID::Android30,
        FormID::Android40,
        FormID::Android50,
        FormID::Android60,

        // Majin
        FormID::MajAffinity,
        FormID::MajSuper,
        FormID::MajTrue,

        // Kai
        FormID::MysticFirst,
        FormID::MysticSecond,
        FormID::MysticThird,

        // Tuffle
        FormID::AscendFirst,
        FormID::AscendSecond,
        FormID::AscendThird
    };


    static const std::unordered_map<RaceID, std::vector<FormID>> race_forms = {
        {RaceID::Human, {FormID::SuperHuman, FormID::SuperHuman2, FormID::SuperHuman3, FormID::SuperHuman4}},
        {RaceID::Saiyan, {FormID::SuperSaiyan, FormID::SuperSaiyan2, FormID::SuperSaiyan3, FormID::SuperSaiyan4}},
        {RaceID::Halfbreed, {FormID::SuperSaiyan, FormID::SuperSaiyan2, FormID::SuperSaiyan3, FormID::SuperSaiyan4}},
        {RaceID::Icer, {FormID::IcerFirst, FormID::IcerSecond, FormID::IcerThird, FormID::IcerFourth}},
        {RaceID::Konatsu, {FormID::ShadowFirst, FormID::ShadowSecond, FormID::ShadowThird}},
        {
            RaceID::Namekian,
            {FormID::SuperNamekian, FormID::SuperNamekian2, FormID::SuperNamekian3, FormID::SuperNamekian4}
        },
        {RaceID::Mutant, {FormID::MutateFirst, FormID::MutateSecond, FormID::MutateThird}},
        {RaceID::BioAndroid, {FormID::BioMature, FormID::BioSemiPerfect, FormID::BioPerfect, FormID::BioSuperPerfect}},
        {
            RaceID::Android,
            {
                FormID::Android10, FormID::Android20, FormID::Android30, FormID::Android40, FormID::Android50,
                FormID::Android60
            }
        },
        {RaceID::Majin, {FormID::MajAffinity, FormID::MajSuper, FormID::MajTrue}},
        {RaceID::Kai, {FormID::MysticFirst, FormID::MysticSecond, FormID::MysticThird}},
        {RaceID::Tuffle, {FormID::AscendFirst, FormID::AscendSecond, FormID::AscendThird}}
    };

    void displayForms(char_data* ch) {
        auto forms = race_forms.find(ch->race);
        if (forms == race_forms.end()) {
            send_to_char(ch, "You have no forms. Bummer.\r\n");
            return;
        }

        send_to_char(ch, "              @YForms@n\r\n");
        send_to_char(ch, "@b------------------------------------------------@n\r\n");
        auto pl = ch->getBasePL();
        std::vector<std::string> form_names;
        for (auto form: forms->second) {
            auto name = getName(ch, form);
            auto req = getRequiredPL(ch, form);
            send_to_char(ch, "@W%s@n @R-@G %s BPL Req\r\n", name,
                         (pl >= (req * 0.75)) ? add_commas(req) : "??????????");
            if (pl >= req) form_names.push_back(getAbbr(ch, form));
        }

        send_to_char(ch, "@b------------------------------------------------@n\r\n");
        if (!form_names.empty()) {
            auto names = boost::join(form_names, ", ");
            send_to_char(ch, "Available Forms: @W%s@n\r\n", names.c_str());
        }

        const double epsilon = 0.05;  // A small tolerance value for floating point comparison

        if (ch->transBonus <= -0.3 + epsilon) {
            send_to_char(ch, "\r\n@WYou have @wGREAT@W transformation BPL Requirements.@n\r\n");
        } else if (ch->transBonus <= -0.2 + epsilon) {
            send_to_char(ch, "\r\n@MYou have @mabove average@M transformation BPL Requirements.@n\r\n");
        } else if (ch->transBonus <= -0.1 + epsilon) {
            send_to_char(ch, "\r\n@BYou have @bslightly above average@B transformation BPL Requirements.@n\r\n");
        } else if (ch->transBonus < 0.1 - epsilon) {
            send_to_char(ch, "\r\n@GYou have @gaverage@G transformation BPL Requirements.@n\r\n");
        } else if (ch->transBonus < 0.2 - epsilon) {
            send_to_char(ch, "\r\n@YYou have @yslightly below average@Y transformation BPL Requirements.@n\r\n");
        } else if (ch->transBonus < 0.3 - epsilon) {
            send_to_char(ch, "\r\n@CYou have @cbelow average@C transformation BPL Requirements.@n\r\n");
        } else {
            send_to_char(ch, "\r\n@RYou have @rterrible@R transformation BPL Requirements.@n\r\n");
        }
    }

    bool blockRevertDisallowed(struct char_data* ch, FormID form) {
        auto curForm = ch->form;
        if (curForm == FormID::Base) return false;

        switch (ch->race) {
            case RaceID::Majin:
            case RaceID::Android:
            case RaceID::BioAndroid:
            case RaceID::Tuffle: {
                if (form == FormID::Base) {
                    return true;
                }
                if (auto forms = race_forms.find(ch->race); forms != race_forms.end()) {
                    auto& f = forms->second;
                    // We need to find the index number of the current form and the proposed form...
                    // If the proposed form is lower than the current form, we need to block it.
                    auto cur = std::find(f.begin(), f.end(), curForm);
                    auto proposed = std::find(f.begin(), f.end(), form);
                    if (cur != f.end() && proposed != f.end()) {
                        if (std::distance(f.begin(), proposed) < std::distance(f.begin(), cur)) {
                            return true;
                        }
                    }
                }
            }
            default:
                break;
        }
        return false;
    }


    static const std::unordered_map<FormID, int64_t> trans_pl = {
        // Human
        {FormID::SuperHuman, 1800000},
        {FormID::SuperHuman2, 35000000},
        {FormID::SuperHuman3, 190000000},
        {FormID::SuperHuman4, 1200000000},

        // Saiyan/Halfbreed.
        {FormID::SuperSaiyan, 1200000},
        {FormID::SuperSaiyan2, 55000000},
        {FormID::SuperSaiyan3, 150000000},
        {FormID::SuperSaiyan4, 1625000000},

        // Namek Forms
        {FormID::SuperNamekian, 360000},
        {FormID::SuperNamekian2, 9500000},
        {FormID::SuperNamekian3, 220000000},
        {FormID::SuperNamekian4, 900000000},

        // Icer Forms
        {FormID::IcerFirst, 500000},
        {FormID::IcerSecond, 17500000},
        {FormID::IcerThird, 150000000},
        {FormID::IcerFourth, 850000000},

        // Majin Forms
        {FormID::MajAffinity, 2200000},
        {FormID::MajSuper, 45000000},
        {FormID::MajTrue, 1550000000},

        // Tuffle Forms
        {FormID::AscendFirst, 3600000},
        {FormID::AscendSecond, 300000000},
        {FormID::AscendThird, 1450000000},

        // Mutant Forms
        {FormID::MutateFirst, 180000},
        {FormID::MutateSecond, 27500000},
        {FormID::MutateThird, 700000000},


        // kai Forms
        {FormID::MysticFirst, 3000000},
        {FormID::MysticSecond, 650000000},
        {FormID::MysticThird, 1300000000},

        // Konatsu Forms

        {FormID::ShadowFirst, 1800000},
        {FormID::ShadowSecond, 225000000},
        {FormID::ShadowThird, 1400000000},

        // Android Forms
        {FormID::Android10, 1000000},
        {FormID::Android20, 8000000},
        {FormID::Android30, 50000000},
        {FormID::Android40, 300000000},
        {FormID::Android50, 800000000},
        {FormID::Android60, 1200000000},

        // Bio Forms
        {FormID::BioMature, 1800000},
        {FormID::BioSemiPerfect, 25000000},
        {FormID::BioPerfect, 220000000},
        {FormID::BioSuperPerfect, 1300000000}

    };

    int64_t getRequiredPL(struct char_data* ch, FormID trans) {
        if (auto req = trans_pl.find(trans); req != trans_pl.end()) {
            return req->second * (1.0 + ch->transBonus);
        }
        return 0;
    }

    std::optional<FormID> findForm(struct char_data* ch, const std::string& arg) {
        auto forms = race_forms.find(ch->race);
        if (forms == race_forms.end()) {
            return {};
        }

        for (auto form: forms->second) {
            if (boost::iequals(arg, getAbbr(ch, form))) return form;
        }

        return {};
    }

    bool unlock(struct char_data *ch, FormID form) {
        return true;
    }

    void gamesys_oozaru(uint64_t heartPulse, double deltaTime) {

    };

}

trans_data::trans_data(const nlohmann::json &j) : trans_data() {
    deserialize(j);
}

void trans_data::deserialize(const nlohmann::json &j) {
    if(j.contains("timeSpentInForm")) timeSpentInForm = j["timeSpentInForm"];
    if(j.contains("blutz")) blutz = j["blutz"];
}

nlohmann::json trans_data::serialize() {
    nlohmann::json j;
    if(timeSpentInForm != 0.0) j["timeSpentInForm"] = timeSpentInForm;
    if(blutz != 0.0) j["blutz"] = blutz;
    return j;
}