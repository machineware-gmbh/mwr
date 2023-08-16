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

#include <Windows.h>
#include <io.h>

namespace mwr {

class aio
{
private:
    struct aio_info {
        int fd;
        aio_handler handler;
    };

    mutable mutex m_mtx;
    unordered_map<HANDLE, aio_info> m_handlers;
    atomic<u64> m_gen;

    atomic<bool> m_running;
    thread m_thread;

    static const int TIMEOUT_MS = 10;

    void aio_thread() {
        vector<HANDLE> polls;
        u64 curgen = 0;

        while (m_running) {
            if (curgen != m_gen) {
                polls.clear();

                lock_guard<mutex> guard(m_mtx);
                for (const auto& it : m_handlers)
                    polls.push_back(it.first);

                curgen = m_gen;
            }

            if (polls.empty()) {
                auto timeout = std::chrono::milliseconds(TIMEOUT_MS);
                std::this_thread::sleep_for(timeout);
                continue;
            }

            for (size_t i = 0; i < polls.size(); i += MAXIMUM_WAIT_OBJECTS) {
                DWORD n = (DWORD)min(polls.size() - i, MAXIMUM_WAIT_OBJECTS);
                int ret = WaitForMultipleObjects(n, polls.data() + i, FALSE,
                                                 TIMEOUT_MS);
                if (!m_running)
                    return;

                if (ret >= WAIT_OBJECT_0 && ret < polls.size()) {
                    lock_guard<mutex> guard(m_mtx);
                    auto handle = polls[ret];
                    if (m_handlers.count(handle) == 0)
                        continue; // fd has been removed

                    auto& triggered = m_handlers[handle];
                    if (fd_peek(triggered.fd))
                        triggered.handler(triggered.fd);
                }
            }
        }
    }

public:
    aio(): m_mtx(), m_handlers(), m_gen(), m_running(true), m_thread() {
        m_thread = thread(&aio::aio_thread, this);
        mwr::set_thread_name(m_thread, "aio_thread");
    }

    virtual ~aio() {
        m_running = false;
        if (m_thread.joinable())
            m_thread.join();
    }

    void notify(int fd, aio_handler handler) {
        lock_guard<mutex> guard(m_mtx);
        HANDLE handle = (HANDLE)_get_osfhandle(fd);
        if (handle == INVALID_HANDLE_VALUE)
            MWR_ERROR("invalid file descriptor: %d", fd);
        m_handlers[handle].fd = fd;
        m_handlers[handle].handler = std::move(handler);
        m_gen++;
    }

    void cancel(int fd) {
        lock_guard<mutex> guard(m_mtx);
        HANDLE handle = (HANDLE)_get_osfhandle(fd);
        if (handle == INVALID_HANDLE_VALUE)
            MWR_ERROR("invalid file descriptor: %d", fd);
        m_handlers.erase(handle);
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
