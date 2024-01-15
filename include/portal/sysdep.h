#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <coroutine>
#include <optional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/detached.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include "shared/net.h"

namespace portal {
    using namespace boost::asio;
    using tcp = ip::tcp;       // from <boost/asio/ip/tcp.hpp>
    using namespace boost::asio::experimental::awaitable_operators;

    template<typename T>
    using Channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;

    extern boost::asio::io_context ioc;


}