#pragma once
#include "sysdep.h"
#include "nlohmann/json.hpp"
#include "defs.h"
#include "shared/net.h"

namespace net {

    enum class DisconnectReason {
        // In these first two examples, the connection is dead on the portal and we have been informed of such.
        ConnectionLost = 0,
        ConnectionClosed = 1,
        // In the remaining enums, the connection is still alive on the portal, but we are disconnecting for some reason.
        // We must inform the portal.
        GameLogoff = 2,
    };

    extern std::mutex connectionsMutex, pendingConnectionsMutex;
    extern std::map<int64_t, std::shared_ptr<Connection>> connections;
    extern std::set<int64_t> pendingConnections;

    extern std::unordered_map<int64_t, DisconnectReason> deadConnections;

    class Connection;

    class ConnectionParser {
    public:
        explicit ConnectionParser(const std::shared_ptr<Connection>& conn) : conn(conn) {}
        virtual ~ConnectionParser() = default;
        virtual void parse(const std::string &txt) = 0;
        virtual void handleGMCP(const std::string &txt, const nlohmann::json &j);
        virtual void start();
        virtual void close();
    protected:
        void sendText(const std::string &txt);
        std::shared_ptr<Connection> conn;
    };

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        Connection();
        void sendGMCP(const std::string &cmd, const nlohmann::json &j);
        void sendText(const std::string &messg);
        void onHeartbeat(double deltaTime);
        void onNetworkDisconnected();
        void onWelcome();
        void close();

        void cleanup(DisconnectReason reason);

        void setParser(ConnectionParser *p);

        boost::asio::awaitable<void> run();

        int64_t connId{};
        account_data *account{};
        int64_t adminLevel{0};
        struct descriptor_data *desc{};

        // Some time structs to handle when we received connections.
        // These probably need some updating on this and Thermite side...
        std::chrono::system_clock::time_point connected{};
        std::chrono::steady_clock::time_point connectedSteady{}, lastActivity{}, lastMsg{};

        // This is embedded for ease of segmentation but this struct isn't
        // actually used anywhere else.
        ProtocolCapabilities capabilities{};

        std::unique_ptr<ConnectionParser> parser;
    protected:
        void handleMessage(const GameMessage &msg);
    };


}