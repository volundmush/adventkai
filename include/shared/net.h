#pragma once
#include "nlohmann/json.hpp"
#include <spdlog/spdlog.h>
#include <memory>

extern std::shared_ptr<spdlog::logger> logger;

extern std::shared_ptr<spdlog::logger> setup_logging(const std::string &name, const std::string& path);

namespace net {

    enum class GameMessageType : uint8_t {
        Connect = 0,
        Update = 1,
        Command = 2,
        GMCP = 3,
        MSSP = 4,
        Disconnect = 5,
        Timeout = 6
    };

    struct GameMessage {
        GameMessage() = default;
        explicit GameMessage(const nlohmann::json& j);
        GameMessageType type{GameMessageType::Connect};
        nlohmann::json data;
        [[nodiscard]] nlohmann::json serialize() const;
    };

    enum class Protocol : uint8_t {
        Telnet = 0,
        WebSocket = 1
    };

    enum class ColorType : uint8_t {
        NoColor = 0,
        Standard = 1,
        Xterm256 = 2,
        TrueColor = 3
    };

    struct ProtocolCapabilities {
        Protocol protocol{Protocol::Telnet};

        std::string clientName = "UNKNOWN", clientVersion = "UNKNOWN";
        std::string hostAddress = "UNKNOWN";
        std::string encoding;
        std::vector<std::string> hostNames{};
        ColorType colorType = ColorType::NoColor;
        int16_t hostPort{0};

        bool encryption = false;
        bool utf8 = false;
        int width = 80, height = 52;
        bool gmcp = false, msdp = false, mssp = false, mxp = false;
        bool mccp2 = false, mccp2_active = false, mccp3 = false, mccp3_active = false;
        bool ttype = false, naws = false, sga = false, linemode = false;
        bool force_endline = false, oob = false, tls = false;
        bool screen_reader = false, mouse_tracking = false, vt100 = false;
        bool osc_color_palette = false, proxy = false, mnes = false;

        void deserialize(const nlohmann::json& j);
        nlohmann::json serialize();
        std::string protocolName();
    };

}