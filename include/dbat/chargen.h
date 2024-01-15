#pragma once
#include "net.h"
#include "structs.h"
#include "races.h"
#include "class.h"

namespace net {

    class ChargenParser : public ConnectionParser {
    public:
        explicit ChargenParser(std::shared_ptr<Connection>& co);
        ~ChargenParser() override;
        void parse(const std::string &txt) override;
        void start() override;

    protected:
        char_data *ch{};
        int state{CON_GET_NAME};
        int total{};
        int ccpoints{};
        int negcount{};
        std::string maybeName;
        int roll_stats(int type, int bonus);
        void display_bonus_menu(int type);
        std::vector<RaceID> valid_races();
        void display_races();
        void display_races_sub();
        std::vector<SenseiID> valid_classes();
        void display_classes_sub();
        void display_classes();
        void display_races_help();
        void display_classes_help();
        void exchange_ccpoints(int value);
        int opp_bonus(int value, int type);
        bool bonus_exclusive(int type, int value, int exc);
        void finish();
    };

}