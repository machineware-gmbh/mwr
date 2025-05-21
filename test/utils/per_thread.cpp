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

TEST(per_thread, has_value) {
    mwr::per_thread<int> data;
    EXPECT_FALSE(data.has_value());
    EXPECT_FALSE(data);

    data = 42;
    EXPECT_TRUE(data.has_value());
    EXPECT_TRUE(data);

    data.clear();
    EXPECT_FALSE(data.has_value());
    EXPECT_FALSE(data);
}

TEST(per_thread, this_thread) {
    mwr::per_thread<int> data;
    data = 42;
    EXPECT_EQ(data, data[std::this_thread::get_id()]);
}

TEST(per_thread, structs) {
    struct test_struct {
        std::string s;
        double f;
    };

    mwr::per_thread<test_struct> data;

    data = test_struct{ "hello", 6.7 };
    EXPECT_EQ(data->s, "hello");
    EXPECT_EQ(data->f, 6.7);

    data->s = "world";
    EXPECT_EQ(data->s, "world");
}

TEST(per_thread, threads) {
    mwr::per_thread<int> data;
    auto fn = [&]() {
        data = 0;
        for (int i = 1; i < 100000; i++) {
            EXPECT_EQ(data, i - 1);
            data = i;
        }
    };

    std::thread t1(fn);
    std::thread t2(fn);

    t1.join();
    t2.join();
}
