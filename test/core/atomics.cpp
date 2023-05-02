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
#include "mwr/core/atomics.h"

using namespace mwr;

TEST(atomic, max) {
    i64 cmp, data = 5;
    EXPECT_EQ(atomic_max(&data, 6), 5);
    EXPECT_EQ(data, 6);

    EXPECT_EQ(atomic_max(&data, -1), 6);
    EXPECT_EQ(data, 6);

    cmp = -2;
    EXPECT_EQ(atomic_max_ptr(&data, &cmp, sizeof(data)), 6);
    EXPECT_EQ(data, 6);

    cmp = 9;
    EXPECT_EQ(atomic_max_ptr(&data, &cmp, sizeof(data)), 6);
    EXPECT_EQ(data, 9);
}

TEST(atomic, umax) {
    u64 cmp, data = 5;
    EXPECT_EQ(atomic_max(&data, 6), 5);
    EXPECT_EQ(data, 6);

    EXPECT_EQ(atomic_max(&data, -2), 6);
    EXPECT_EQ(data, -2);

    cmp = -1;
    EXPECT_EQ(atomic_umax_ptr(&data, &cmp, sizeof(data)), -2);
    EXPECT_EQ(data, -1);

    cmp = 9;
    EXPECT_EQ(atomic_umax_ptr(&data, &cmp, sizeof(data)), -1);
    EXPECT_EQ(data, -1);
}

TEST(atomic, bitwise) {
    u32 a = 1, b = 2, c = 4;

    ASSERT_EQ(atomic_or(&a, 2), 1);
    EXPECT_EQ(a, 3);

    ASSERT_EQ(atomic_and(&b, 2), 2);
    EXPECT_EQ(b, 2);

    ASSERT_EQ(atomic_xor(&c, 2), 4);
    EXPECT_EQ(c, 6);
}

TEST(atomic, ptrops) {
    const u64 data = -1;
    u64 dest = 0;

    EXPECT_EQ(atomic_add_ptr(&dest, &data, sizeof(u8)), 0);
    EXPECT_EQ(dest, 0xff);

    dest = 0;
    EXPECT_EQ(atomic_add_ptr(&dest, &data, sizeof(u16)), 0);
    EXPECT_EQ(dest, 0xffff);

    dest = 0;
    EXPECT_EQ(atomic_add_ptr(&dest, &data, sizeof(u32)), 0);
    EXPECT_EQ(dest, 0xffffffff);

    dest = 0;
    EXPECT_EQ(atomic_add_ptr(&dest, &data, sizeof(u64)), 0);
    EXPECT_EQ(dest, 0xffffffffffffffff);
}
