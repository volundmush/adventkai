#include "dbat/net.h"
#include "dbat/utils.h"
#include <regex>
#include "dbat/screen.h"
#include "dbat/players.h"
#include "dbat/login.h"
#include "dbat/config.h"


#define COLOR_ON(ch) (COLOR_LEV(ch) > 0)

namespace net {
    std::mutex connectionsMutex, pendingConnectionsMutex;

    std::map<int64_t, std::shared_ptr<Connection>> connections;
    std::set<int64_t> pendingConnections;

    std::unordered_map<int64_t, DisconnectReason> deadConnections;

    void Connection::sendGMCP(const std::string &cmd, const nlohmann::json &j) {
        GameMessage msg;
        msg.type = GameMessageType::GMCP;
        msg.data.push_back(cmd);
        msg.data.push_back(j);


    }

    void Connection::sendText(const std::string &text) {
        if(text.empty()) return;
        GameMessage msg;
        msg.type = GameMessageType::Command;

        if(desc && desc->character) {
            auto &p = players[desc->character->id];
            msg.data = processColors(text, COLOR_ON(desc->character), p.color_choices);
        } else {
            msg.data = processColors(text, true, nullptr);
        }

    }


    void Connection::onWelcome() {
        account = nullptr;
        desc = nullptr;
        setParser(new LoginParser(shared_from_this()));
    }

    void Connection::close() {
        deadConnections[this->connId] = DisconnectReason::GameLogoff;
    }

    void Connection::onNetworkDisconnected() {
        deadConnections[this->connId] = DisconnectReason::ConnectionLost;
    }


    void Connection::cleanup(DisconnectReason reason) {
        if (account) {
            // Remove ourselves from the account's connections list.
            account->connections.erase(this);
            account = nullptr;
        }
        switch (reason) {
            case DisconnectReason::ConnectionLost:
                // The connection to remote host was lost for unknown reasons.
                // The portal is aware of this and already cleaned them up.
                if (desc) {
                    // If we have a desc then we inform it of the unexpected disconnect.
                    // It can handle this as it sees fit.
                    // onConnectionLost must remove the connection from the descriptor.
                    desc->onConnectionLost(connId);
                    desc = nullptr;
                }
                break;
            case DisconnectReason::ConnectionClosed:
                // The remote socket gracefully/voluntarily closed by the remote host.
                // The portal is aware of this and already cleaned them up.
                if (desc) {
                    // If we have a desc then we can attempt a 'quit' on the
                    // connected character. OR however else it wants to handle it.
                    // onConnectionClosed must remove the connection from the descriptor.
                    desc->onConnectionClosed(connId);
                    desc = nullptr;
                }
                break;
            case DisconnectReason::GameLogoff: {
                // The game server is initiating disconnection for some reason or another.
                // The portal must be informed so it can boot the remote host.
                break;
            }
                // We do not remove ourselves from net::connections; that's done by the main loop.
        }
    }

    void Connection::onHeartbeat(double deltaTime) {

    }


    void ConnectionParser::close() {

    }

	void ConnectionParser::sendText(const std::string &txt) {
        conn->sendText(txt);
    }

    void ConnectionParser::start() {

    }

    void ConnectionParser::handleGMCP(const std::string &txt, const nlohmann::json &j) {

    }

    void Connection::setParser(ConnectionParser *p) {
        if(parser) parser->close();
        parser.reset(p);
        p->start();
    }

    void Connection::handleMessage(const GameMessage &msg) {
        switch(msg.type) {
            case GameMessageType::Update:
                capabilities.deserialize(msg.data);
                break;
            case GameMessageType::Command: {
                auto cmd = msg.data.get<std::string>();
                if(parser) parser->parse(cmd);
            }
                break;
            case GameMessageType::GMCP:
                if(msg.data.is_array() && msg.data.size() == 2) {
                    auto cmd = msg.data[0].get<std::string>();
                    auto data = msg.data[1];
                    if(parser) parser->handleGMCP(cmd, data);
                }
                break;
            case GameMessageType::Disconnect:
                break;
            case GameMessageType::Timeout:
                break;
        }

    }


}