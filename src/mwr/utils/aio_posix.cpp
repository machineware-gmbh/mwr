/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/utils.h"
#include "mwr/stl/threads.h"
#include "mwr/stl/containers.h"
#include "mwr/utils/aio.h"

#include <poll.h>

namespace mwr {

class aio
{
private:
    mutable mutex m_mtx;
    unordered_map<int, aio_handler> m_handlers;
    atomic<u64> m_gen;

    atomic<bool> m_running;
    thread m_thread;

    static const int TIMEOUT_MS = 10;

    void aio_thread() {
        set_thread_name("aio_thread");
        vector<struct pollfd> polls;
        u64 curgen = 0;
        int ret = 0;

        while (m_running) {
            if (curgen != m_gen) {
                polls.clear();

                lock_guard<mutex> guard(m_mtx);
                for (const auto& it : m_handlers)
                    polls.push_back({ it.first, POLLIN | POLLPRI, 0 });

                curgen = m_gen;
            }

            if (polls.empty()) {
                auto timeout = std::chrono::milliseconds(TIMEOUT_MS);
                std::this_thread::sleep_for(timeout);
                continue;
            }

            do {
                ret = poll(polls.data(), polls.size(), TIMEOUT_MS);
            } while (ret < 0 && errno == EINTR);
            MWR_ERROR_ON(ret < 0, "aio error: %s", strerror(errno));
            vector<pair<int, aio_handler>> scheduled;

            if (ret > 0 && m_running) {
                lock_guard<mutex> guard(m_mtx);
                for (const auto& pfd : polls) {
                    if (m_handlers.count(pfd.fd) == 0)
                        continue; // fd has been removed

                    if (pfd.revents & POLLNVAL)
                        MWR_ERROR("invalid file descriptor: %d", pfd.fd);

                    if (pfd.revents & (POLLIN | POLLPRI))
                        scheduled.emplace_back(pfd.fd, m_handlers[pfd.fd]);
                }
            }

            for (const auto& handler : scheduled)
                handler.second(handler.first);
        }
    }

public:
    aio(): m_mtx(), m_handlers(), m_gen(), m_running(true), m_thread() {
        m_thread = thread(&aio::aio_thread, this);
    }

    virtual ~aio() {
        m_running = false;
        if (m_thread.joinable())
            m_thread.join();
    }

    void notify(int fd, aio_handler handler) {
        lock_guard<mutex> guard(m_mtx);
        m_handlers[fd] = std::move(handler);
        m_gen++;
    }

    void cancel(int fd) {
        lock_guard<mutex> guard(m_mtx);
        m_handlers.erase(fd);
        m_gen++;
    }

    static aio& instance() {
        static aio singleton;
        return singleton;
    }
};

void aio_notify(int fd, aio_handler handler) {
    aio::instance().notify(fd, std::move(handler));
}

void aio_cancel(int fd) {
    aio::instance().cancel(fd);
}

} // namespace mwr
