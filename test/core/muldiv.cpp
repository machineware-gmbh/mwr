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
#include "mwr/core/muldiv.h"

using namespace mwr;

TEST(muldiv, divup) {
    EXPECT_EQ(udivup(10, 5), 2);
    EXPECT_EQ(udivup(10, 3), 4);
    EXPECT_EQ(udivup(10, 1), 10);
    EXPECT_EQ(udivup(10, 9), 2);
    EXPECT_EQ(udivup(10, 10), 1);
    EXPECT_EQ(udivup(0, 10), 0);
}

TEST(muldiv, umul64) {
    std::mt19937 rng;
    std::uniform_int_distribution<u64> dist;

    for (size_t i = 0; i < 1000; i++) {
        u64 a = dist(rng);
        u64 b = dist(rng);

        u64 h0, l0;
        umul64(h0, l0, a, b);

        u64 h1, l1;
        umul64_slow(h1, l1, a, b);

        ASSERT_EQ(l0, a * b);
        ASSERT_EQ(l1, a * b);
        ASSERT_EQ(h0, h1);
    }
}

TEST(muldiv, imul64) {
    std::mt19937 rng;
    std::uniform_int_distribution<i64> dist;

    for (size_t i = 0; i < 1000; i++) {
        i64 a = dist(rng);
        i64 b = dist(rng);

        i64 h0, l0;
        imul64(h0, l0, a, b);

        i64 h1, l1;
        imul64_slow(h1, l1, a, b);

        ASSERT_EQ(l0, a * b);
        ASSERT_EQ(l1, a * b);
        ASSERT_EQ(h0, h1);
    }
}
