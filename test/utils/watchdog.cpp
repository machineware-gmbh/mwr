/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"

#include "mwr.h"

using namespace std::literals;

TEST(watchdog, execution_order) {
    auto& wd = mwr::watchdog::instance();

    std::atomic<int> counter = 0;

    wd.schedule(100ms, [&counter] {
        ASSERT_EQ(counter.load(), 1);
        counter++;
    });

    wd.schedule(50ms, [&counter] {
        ASSERT_EQ(counter.load(), 0);
        counter++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    ASSERT_EQ(counter.load(), 2);
}

TEST(watchdog, execution) {
    auto& wd = mwr::watchdog::instance();
    std::atomic<bool> executed = false;
    wd.schedule(10ms, [&executed] { executed = true; });
    std::this_thread::sleep_for(100ms);
    ASSERT_TRUE(executed);
}

static void test_rearm(std::atomic<int>& counter, mwr::watchdog& wd) {
    counter++;
    if (counter < 10)
        wd.schedule(10ms, [&]() { test_rearm(counter, wd); });
}

TEST(watchdog, rearm) {
    auto& wd = mwr::watchdog::instance();
    std::atomic<int> counter = 0;
    test_rearm(counter, wd);
    std::this_thread::sleep_for(100ms);
    ASSERT_EQ(counter, 10);
}

TEST(watchdog, cancel) {
    auto& wd = mwr::watchdog::instance();
    std::atomic<int> counter = 0;
    wd.schedule(100ms, [&counter] { counter++; });
    std::this_thread::sleep_for(10ms);
    wd.cancel_all();
    std::this_thread::sleep_for(100ms);
    ASSERT_EQ(counter, 0);
}

TEST(watchdog, extra) {
    std::atomic<int> counter = 0;

    {
        mwr::watchdog extra("extra");
        extra.schedule(0ms, [&counter] { counter++; });
        extra.schedule(10ms, [&counter] { counter++; });
        extra.schedule(100ms, [&counter] { counter++; });
        std::this_thread::sleep_for(10ms);
    }

    EXPECT_EQ(counter, 2);
}
