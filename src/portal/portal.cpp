//
// Created by volund on 1/9/24.
//
#include "portal/portal.h"
#include "portal/telnet.h"

#include <thread>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>

namespace portal {

    boost::asio::io_context ioc;

    static awaitable<void> handleConnection(tcp::socket socket) {
        auto ex = co_await this_coro::executor;
        telnet::TelnetConnection telnetConnection(std::move(socket), ex);
        co_await telnetConnection.run();

        co_return;
    }

    static awaitable<void> listen(tcp::acceptor& acceptor) {
        while(true) {
            auto socket = co_await acceptor.async_accept(use_awaitable);
            co_spawn(make_strand(ioc), handleConnection(std::move(socket)), detached);
        }

        co_return;
    }

    void run() {

        tcp::acceptor acceptor(ioc, {tcp::v4(), config::listenPort});
        co_spawn(ioc, listen(acceptor), boost::asio::detached);

        // Use as many threads as there are CPUs
        const auto thread_count = std::min<int>(0, std::thread::hardware_concurrency() - 1);
        std::vector<std::thread> threads;

        if(thread_count) {
            threads.reserve(thread_count);
            for (auto i = 0u; i < thread_count; ++i) {
                threads.emplace_back([&] { ioc.run(); });
            }
        }

        ioc.run();

        // Wait for all threads to complete
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        threads.clear();
    }

}
