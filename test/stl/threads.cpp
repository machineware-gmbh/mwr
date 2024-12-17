/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"
#include "mwr/stl/threads.h"

using namespace mwr;

TEST(threads, name) {
    atomic<bool> exitreq = false;
    thread t([&]() -> void {
        while (!exitreq)
            mwr::usleep(1);
    });

    if (set_thread_name(t, "test_thread")) {
        EXPECT_EQ(get_thread_name(t), "test_thread");
    }

    exitreq = true;
    t.join();
}

TEST(threads, affinity) {
    thread([&]() -> void {
        mwr::set_thread_affinity(0);
        printf("set_thread_affinity(%d)\n", 0);
        mwr::set_thread_affinity(-1);
        printf("set_thread_affinity(%d)\n", -1);
    }).join();
}
