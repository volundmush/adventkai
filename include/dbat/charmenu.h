#pragma once
#include "net.h"
#include "structs.h"

namespace net {
    class CharacterMenu : public ConnectionParser {
    public:
        CharacterMenu(std::shared_ptr<Connection>& co, char_data *c);
        void parse(const std::string &txt) override;
        void start() override;
    protected:
        struct char_data *ch;
        int state;
    };
}