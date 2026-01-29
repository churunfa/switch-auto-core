//
// Created by churunfa on 2026/1/29.
//

#ifndef SWITCH_AUTO_CORE_COROUTINEUTILS_H
#define SWITCH_AUTO_CORE_COROUTINEUTILS_H

#include <boost/asio.hpp>
#include <chrono>

namespace coroutine {
    inline asio::awaitable<void> async_sleep_task(const int exec_sleep_time) {
        const auto executor = co_await asio::this_coro::executor;

        asio::steady_timer timer(executor);
        timer.expires_after(std::chrono::milliseconds(exec_sleep_time));
        co_await timer.async_wait(asio::use_awaitable);
    }

}

#endif //SWITCH_AUTO_CORE_COROUTINEUTILS_H