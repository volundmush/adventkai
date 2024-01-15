#include "portal/config.h"

namespace portal::config {
    std::string listenAddress{"0.0.0.0"};
    uint16_t listenPort{1280};

    std::string serverAddress{"127.0.0.1"};
    std::string serverPort("8000");
    std::string serverPath{"/ws"};
    bool serverSecure{false};
}
boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv13_client);