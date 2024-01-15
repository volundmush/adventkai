#pragma once
#include "net.h"

namespace net {
    enum class AccountMenuState : uint8_t {
        MainMenu = 0,
    };

    class AccountMenu : public ConnectionParser {
    public:
        using ConnectionParser::ConnectionParser;
        void parse(const std::string &txt) override;
        void start() override;
    protected:
        AccountMenuState state{AccountMenuState::MainMenu};
        void displayMenu();
    };
}