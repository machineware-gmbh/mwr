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

TEST(atomic, add) {
    i32 val32 = 7;
    EXPECT_EQ(atomic_add(&val32, 19), 7);
    EXPECT_EQ(val32, 26);

    i64 val64 = 12;
    EXPECT_EQ(atomic_add(&val64, 5), 12);
    EXPECT_EQ(val64, 17);
}

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

TEST(atomic, cas8) {
    u8 data = 0x11;
    u8 cmpv = 0x11;
    u8 wval = 0x22;

    ASSERT_TRUE(atomic_cas8(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas8(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas16) {
    u16 data = 0x1122;
    u16 cmpv = 0x1122;
    u16 wval = 0x2233;

    ASSERT_TRUE(atomic_cas16(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas16(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas32) {
    u32 data = 0x11223344;
    u32 cmpv = 0x11223344;
    u32 wval = 0x2233aabb;

    ASSERT_TRUE(atomic_cas32(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas32(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas64) {
    u64 data = 0x1122334455667788;
    u64 cmpv = 0x1122334455667788;
    u64 wval = 0x2233aabbccddeeff;

    ASSERT_TRUE(atomic_cas64(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas64(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas128) {
    u64 data[2] = { 0x1111222233334444, 0x5555666677778888 };
    u64 cmpv[2] = { 0x1111222233334444, 0x5555666677778888 };
    u64 wval[2] = { 0xaaaabbbbccccdddd, 0xeeeeffff00001111 };

    ASSERT_TRUE(atomic_cas128(data, cmpv, wval));
    ASSERT_EQ(data[0], wval[0]);
    ASSERT_EQ(data[1], wval[1]);
    ASSERT_FALSE(atomic_cas128(data, cmpv, wval));
    ASSERT_EQ(data[0], wval[0]);
    ASSERT_EQ(data[1], wval[1]);
}
