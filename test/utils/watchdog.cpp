/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/
#include "testing.h"

#include "mwr.h"

using namespace std::literals;

using clock_type = std::chrono::steady_clock;

// Polls until the condition holds. The timeout is only there to keep a broken
// watchdog from hanging the test, so it is deliberately generous: tests must
// not fail just because the host is slow or busy.
template <typename CONDITION>
static bool wait_for(CONDITION&& condition) {
    auto timeout = clock_type::now() + 10s;
    while (!condition()) {
        if (clock_type::now() > timeout)
            return false;
        std::this_thread::sleep_for(1ms);
    }

    return true;
}

// State shared between a test and its tasks. Tasks hold on to it by value, so
// that a task running late can never touch the stack of a finished test.
struct test_state {
    std::atomic<int> counter{ 0 };
    std::atomic<int> first{ 0 };
    std::atomic<bool> early{ false };
    std::atomic<bool> late{ false };
    std::atomic<clock_type::rep> early_ms{ -1 };
};

class watchdog : public testing::Test
{
protected:
    std::shared_ptr<test_state> state = std::make_shared<test_state>();

    // do not leak pending tasks into the next test
    void TearDown() override { mwr::watchdog::instance().cancel_all(); }
};

TEST_F(watchdog, execution_order) {
    auto& wd = mwr::watchdog::instance();

    // scheduled first, but due last
    wd.schedule(300ms, [s = state] {
        if (s->counter++ == 0)
            s->first = 2;
    });

    wd.schedule(50ms, [s = state] {
        if (s->counter++ == 0)
            s->first = 1;
    });

    ASSERT_TRUE(wait_for([&] { return state->counter == 2; }));
    EXPECT_EQ(state->first, 1);
}

TEST_F(watchdog, earlier_task) {
    auto& wd = mwr::watchdog::instance();
    auto start = clock_type::now();

    // a task must not be delayed by one that is already pending, even if
    // that other one is due later
    wd.schedule(600ms, [s = state] { s->late = true; });
    std::this_thread::sleep_for(20ms);
    wd.schedule(50ms, [s = state, start] {
        auto elapsed = clock_type::now() - start;
        s->early_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          elapsed)
                          .count();
        s->early = true;
    });

    ASSERT_TRUE(wait_for([&] { return state->early.load(); }));
    EXPECT_LT(state->early_ms, 500) << "task had to wait for a later one";

    ASSERT_TRUE(wait_for([&] { return state->late.load(); }));
}

TEST_F(watchdog, execution) {
    auto& wd = mwr::watchdog::instance();
    wd.schedule(10ms, [s = state] { s->counter++; });
    ASSERT_TRUE(wait_for([&] { return state->counter == 1; }));
}

static void test_rearm(const std::shared_ptr<test_state>& s,
                       mwr::watchdog& wd) {
    s->counter++;
    if (s->counter < 10)
        wd.schedule(10ms, [s, &wd]() { test_rearm(s, wd); });
}

TEST_F(watchdog, rearm) {
    auto& wd = mwr::watchdog::instance();
    test_rearm(state, wd);
    ASSERT_TRUE(wait_for([&] { return state->counter == 10; }));

    // the chain must have ended
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(state->counter, 10);
}

TEST_F(watchdog, cancel) {
    auto& wd = mwr::watchdog::instance();
    wd.schedule(300ms, [s = state] { s->counter++; });
    std::this_thread::sleep_for(10ms);
    wd.cancel_all();

    // a task scheduled after the cancel still runs, the cancelled one that
    // would have been due before it does not
    wd.schedule(350ms, [s = state] { s->late = true; });
    ASSERT_TRUE(wait_for([&] { return state->late.load(); }));
    EXPECT_EQ(state->counter, 0);
}

TEST_F(watchdog, extra) {
    {
        mwr::watchdog extra("extra");
        extra.schedule(0ms, [s = state] { s->counter++; });
        extra.schedule(10ms, [s = state] { s->counter++; });
        extra.schedule(2000ms, [s = state] { s->counter++; });
        std::this_thread::sleep_for(50ms);
    }

    // tasks that are due get executed on destruction, others are dropped
    EXPECT_EQ(state->counter, 2);
}
