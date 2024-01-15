#pragma once
#include "net.h"
#include "structs.h"

namespace net {
    enum class LoginState : uint8_t {
        GetName = 0,
        GetEmail = 1,
        GetPassword = 2,
        ConfName = 3,
        ConfPassword = 4
    };

    class LoginParser : public ConnectionParser {
    public:
        using ConnectionParser::ConnectionParser;
        void parse(const std::string &txt) override;
        void start() override;
    protected:
        std::string name, email, password;
        account_data *account;
        LoginState state{LoginState::GetName};

        void getName(const std::string &txt);
        void confName(const std::string &txt);
        void getPassword(const std::string &txt);
        void confPassword(const std::string &txt);
    };
}