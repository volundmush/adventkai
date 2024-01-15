#pragma once
#include "portal/sysdep.h"

namespace portal::config {
    // the IP address of the thermite server used as the networking front-end.
    extern std::string listenAddress;
    // the port of the thermite server used as the networking front-end.
    extern uint16_t listenPort;

    extern std::string serverAddress;
    extern std::string serverPort;
    extern std::string serverPath;
    extern bool serverSecure;
}
