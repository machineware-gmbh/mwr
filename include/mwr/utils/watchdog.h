/******************************************************************************
 *                                                                            *
 * Copyright 2025 MachineWare GmbH                                            *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_WATCHDOG_H
#define MWR_UTILS_WATCHDOG_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "mwr/stl/threads.h"

namespace mwr {

class watchdog
{
public:
    using clock_t = std::chrono::high_resolution_clock;

    void schedule(typename clock_t::duration delta,
                  std::function<void()> task) {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto timeout = clock_t::now() + delta;
        m_tasks.push({ timeout, std::move(task) });
        m_cv.notify_one();
    }

    void cancel_all() {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_tasks = std::priority_queue<task, std::vector<task>,
                                      std::greater<>>();
        m_cv.notify_one();
    }

    void schedule(size_t timeout_ns, std::function<void()> task) {
        schedule(std::chrono::nanoseconds(timeout_ns), std::move(task));
    }

    watchdog(watchdog const&) = delete;
    watchdog(const string& name):
        m_tasks(), m_worker(), m_mtx(), m_cv(), m_terminate(false) {
        m_worker = std::thread(&watchdog::work, this, name);
    }

    ~watchdog() {
        m_mtx.lock();
        m_terminate = true;
        m_cv.notify_one();
        m_mtx.unlock();
        m_worker.join();
    }

    static watchdog& instance() {
        static watchdog singleton("watchdog");
        return singleton;
    }

private:
    struct task {
        typename clock_t::time_point timeout;
        std::function<void()> func;
        bool operator>(const task& o) const { return timeout > o.timeout; }
    };

    std::priority_queue<task, std::vector<task>, std::greater<>> m_tasks;
    std::thread m_worker;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::atomic<bool> m_terminate;

    void work(const string& name) {
        mwr::set_thread_name(name);
        std::unique_lock<std::mutex> lock(m_mtx);
        while (true) {
            if (m_terminate)
                break;
            if (m_tasks.empty()) {
                m_cv.wait(lock);
            } else {
                auto next_timeout = m_tasks.top().timeout;
                auto delta = next_timeout - clock_t::now();
                const std::chrono::milliseconds threshold(10);

                // the condition variable takes quite some time to wakeup on
                // timeout (measured between 1 and 2ms), so we just spin for
                // shorter deltas
                if (delta <= threshold) {
                    lock.unlock();
                    while (clock_t::now() < next_timeout)
                        ; // spin
                    lock.lock();
                } else {
                    m_cv.wait_until(lock, next_timeout, [this] {
                        return m_terminate ||
                               (!m_tasks.empty() &&
                                (m_tasks.top().timeout < clock_t::now()));
                    });
                }

                while (!m_tasks.empty() &&
                       (m_tasks.top().timeout <= clock_t::now())) {
                    auto task = m_tasks.top().func;
                    m_tasks.pop();
                    lock.unlock();
                    task();
                    lock.lock();
                }
            }
        }

        while (!m_tasks.empty() && (m_tasks.top().timeout <= clock_t::now())) {
            auto task = m_tasks.top().func;
            m_tasks.pop();
            lock.unlock();
            task();
            lock.lock();
        }
    }
};

} // namespace mwr

#endif
