#pragma once
#include "dbat/accmenu.h"
#include "dbat/charmenu.h"
#include "dbat/db.h"
#include "dbat/players.h"
#include "fmt/format.h"
#include "dbat/chargen.h"
#include <boost/algorithm/string.hpp>

namespace net {

    void AccountMenu::displayMenu() {
        auto a = conn->account;
        sendText("                 @RUser Menu@n\n");
        sendText("@D=============================================@n\r\n");
        sendText(fmt::format("@D|@gUser Account  @D: @w{:>27}@D|@n\n", a->name));
        sendText(fmt::format("@D|@gEmail Address @D: @w{:>27}@D|@n\n", a->email));
        sendText(fmt::format("@D|@gMax Characters@D: @w{:>27}@D|@n\n", a->slots));
        sendText(fmt::format("@D|@gRP Points     @D: @w{:>27}@D|@n\n", a->rpp));

        sendText("@D=============================================@n\r\n\r\n");
        sendText("      @D[@y------@YActive Connections@y------@D]@n\n");
        for(const auto &c : a->connections) {
            auto &cap = c->capabilities;
            std::string line = fmt::format("[{}] {} over ({}) {} version {}", c->connId, cap.hostAddress, cap.protocolName(), cap.clientName, cap.clientVersion);
            if(c->desc) {
                line += fmt::format(" as {}@n\r\n", c->desc->character->name);
            } else {
                line += "@n\r\n";
            }
            sendText(line);
        }

        sendText("@D=============================================@n\r\n\r\n");
        sendText("      @D[@y------@YAvailable Characters@y------@D]@n\n");
        int counter = 0;
        for(auto cid : a->characters) {
            auto p = players.find(cid);
            if(p == players.end()) continue;
            auto c = p->second.character;
            std::string line = fmt::format("                @B(@W{}@B) @C{}@n", ++counter, c->name);
            if(c->desc) {
                line += fmt::format(" @D[@y{} Connections@D]@n\r\n", c->desc->conns.size());
            } else {
                line += "\r\n";
            }
            sendText(line);

        }
        while(counter < conn->account->slots) {
            sendText(fmt::format("                @B(@W{}@B) @C{}@n\r\n", ++counter, "<empty>"));
        }

        sendText("      @D[@y---- @YSelect Another Option @y----@D]@n\r\n");
        sendText("                @B(@WA@B) @CCreate New Character@n\r\n");
        sendText("                @B(@WB@B) @CBuy New C. Slot @D(@R15 RPP@D)@n\r\n");
        sendText("                @B(@WC@B) @CUser's Customs\r\n");
        sendText("                @B(@WD@B) @RDelete User@n\r\n");
        sendText("                @B(@WE@B) @CEmail@n\r\n");
        sendText("                @B(@WP@B) @CNew Password@n\r\n");
        sendText("                @B(@WQ@B) @CQuit@n\r\n");
        sendText("\r\nMake your choice: \r\n");
    }

    void AccountMenu::start() {
        state = AccountMenuState::MainMenu;
        displayMenu();
    }

    void AccountMenu::parse(const std::string &txt) {
        if(boost::iequals(txt, "return") || boost::istarts_with(txt, "return ")) {
            start();
            return;
        }

        if(!txt.empty() && boost::all(txt, boost::is_digit())) {
            auto num = std::stoi(txt);
            auto slot = num - 1;

            if(num < 0 || num > conn->account->characters.size()) {
                sendText("No such slot.\r\n");
                return;
            }

            auto id = conn->account->characters[slot];
            auto p = players.find(id);
            if(p == players.end()) {
                sendText(fmt::format("ERROR: Player ID {} not found. Please alert staff.\r\n", id));
                return;
            }

            conn->setParser(new CharacterMenu(conn, p->second.character));
            return;
        }

        if(boost::iequals(txt, "A") || boost::istarts_with(txt, "A ")) {
            // Start chargen...
            if(conn->account->slots <= conn->account->characters.size()) {
                sendText("Sorry, your slots are all full.\r\n");
                return;
            }
            conn->setParser(new ChargenParser(conn));
            return;
        }

        if (boost::iequals(txt, "B") || boost::istarts_with(txt, "B ")) {
            // Buying a slot.
            if(conn->account->rpp < 15) {
                sendText("You need at least 15 RPP to purchase a new character slot.\r\n");
                return;
            }
            if(conn->account->slots > 4) {
                sendText("You can't buy any more character slots.\r\n");
                return;
            }
            conn->account->modRPP(-15);
            conn->account->slots++;
            // no need to set account dirty since modRPP does that.
            sendText("New slot purchased. [-15 RPP].\r\n");
            return;
        }

        if(boost::iequals(txt, "D") || boost::istarts_with(txt, "D ")) {
            // todo: implement...
        }

        if(boost::iequals(txt, "Q") || boost::istarts_with(txt, "Q ")) {
            sendText("Goodbye!\r\n");
            conn->close();
            return;
        }

        sendText("Invalid option.\r\n");
        start();
    }
}