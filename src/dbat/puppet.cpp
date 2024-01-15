#include "dbat/puppet.h"
#include "dbat/structs.h"
#include "dbat/utils.h"
#include "dbat/dg_scripts.h"
#include "dbat/class.h"

namespace net {
    PuppetParser::PuppetParser(std::shared_ptr<Connection>& co, char_data *c) : ConnectionParser(co) {
        ch = c;
    }

    void PuppetParser::start() {
        ch->playerFlags.reset(PLR_NOTDEADYET);
        if(ch->desc) {
            if(STATE(ch->desc) == CON_COPYOVER) return;

            // The character already has a descriptor! Let's just join up with it.
            if(ch->desc->conns.empty()) {
                // We are reviving a session that was in timeout.
                ch->desc->timeoutCounter = 0.0;
            }
            conn->account->descriptors.insert(ch->desc);
            conn->desc = ch->desc;
            conn->desc->conns[conn->connId] = conn;
            conn->sendText("Joining existing session...\r\n");
            return;
        }

        auto desc = new descriptor_data();
        STATE(desc) = CON_LOGIN;
        desc->raw_input_queue = std::make_unique<Channel<std::string>>(*io, 200);
        desc->character = ch;
        desc->id = ch->id;
        ch->desc = desc;
        conn->desc = desc;
        desc->account = conn->account;
        conn->account->descriptors.insert(desc);
        desc->conns[conn->connId] = conn;
        sessions[ch->id] = desc;

        desc->next = descriptor_list;
        descriptor_list = desc;
    }

    void PuppetParser::parse(const std::string &txt) {
        conn->desc->raw_input_queue->try_send(boost::system::error_code{}, txt);
    }
}