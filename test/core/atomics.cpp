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

TEST(atomic, and8) {
    u8 a = 0x50;
    EXPECT_EQ(atomic_and8(&a, 0xe0), 0x50);
    EXPECT_EQ(a, 0x50 & 0xe0);

    u8 c = 0x30;
    u8 d = 0xf0;
    EXPECT_EQ(atomic_and(&c, d), 0x30);
    EXPECT_EQ(c, 0x30 & 0xf0);
}

TEST(atomic, and16) {
    u16 a = 0x5000;
    EXPECT_EQ(atomic_and16(&a, 0xe000), 0x5000);
    EXPECT_EQ(a, 0x5000 & 0xe000);

    u16 c = 0x3000;
    u16 d = 0xf000;
    EXPECT_EQ(atomic_and(&c, d), 0x3000);
    EXPECT_EQ(c, 0x3000 & 0xf000);
}

TEST(atomic, and32) {
    u32 a = 0x50000000;
    EXPECT_EQ(atomic_and32(&a, 0xe0000000), 0x50000000);
    EXPECT_EQ(a, 0x50000000 & 0xe0000000);

    u32 c = 0x30000000;
    u32 d = 0xf0000000;
    EXPECT_EQ(atomic_and(&c, d), 0x30000000);
    EXPECT_EQ(c, 0x30000000 & 0xf0000000);
}

TEST(atomic, and64) {
    u64 a = 0x5000000000000000;
    EXPECT_EQ(atomic_and64(&a, 0xe000000000000000), 0x5000000000000000);
    EXPECT_EQ(a, 0x5000000000000000 & 0xe000000000000000);

    u64 c = 0x3000000000000000;
    u64 d = 0xf000000000000000;
    EXPECT_EQ(atomic_and(&c, d), 0x3000000000000000);
    EXPECT_EQ(c, 0x3000000000000000 & 0xf000000000000000);
}

TEST(atomic, or8) {
    u8 a = 0x50;
    EXPECT_EQ(atomic_or8(&a, 0xa0), 0x50);
    EXPECT_EQ(a, 0x50 | 0xa0);

    u8 c = 0x0f;
    u8 d = 0xf0;
    EXPECT_EQ(atomic_or(&c, d), 0x0f);
    EXPECT_EQ(c, 0x0f | 0xf0);
}

TEST(atomic, or16) {
    u16 a = 0x5000;
    EXPECT_EQ(atomic_or16(&a, 0xa000), 0x5000);
    EXPECT_EQ(a, 0x5000 | 0xa000);

    u16 c = 0x0f00;
    u16 d = 0xf000;
    EXPECT_EQ(atomic_or(&c, d), 0x0f00);
    EXPECT_EQ(c, 0x0f00 | 0xf000);
}

TEST(atomic, or32) {
    u32 a = 0x50000000;
    EXPECT_EQ(atomic_or32(&a, 0xa0000000), 0x50000000);
    EXPECT_EQ(a, 0x50000000 | 0xa0000000);

    u32 c = 0x0f000000;
    u32 d = 0xf0000000;
    EXPECT_EQ(atomic_or(&c, d), 0x0f000000);
    EXPECT_EQ(c, 0x0f000000 | 0xf0000000);
}

TEST(atomic, or64) {
    u64 a = 0x5000000000000000;
    EXPECT_EQ(atomic_or64(&a, 0xa000000000000000), 0x5000000000000000);
    EXPECT_EQ(a, 0x5000000000000000 | 0xa000000000000000);

    u64 c = 0x0f00000000000000;
    u64 d = 0xf000000000000000;
    EXPECT_EQ(atomic_or(&c, d), 0x0f00000000000000);
    EXPECT_EQ(c, 0x0f00000000000000 | 0xf000000000000000);
}

TEST(atomic, xor8) {
    u8 a = 0x5a;
    EXPECT_EQ(atomic_xor8(&a, 0xa5), 0x5a);
    EXPECT_EQ(a, 0x5a ^ 0xa5);

    u8 c = 0x1f;
    u8 d = 0xf1;
    EXPECT_EQ(atomic_xor(&c, d), 0x1f);
    EXPECT_EQ(c, 0x1f ^ 0xf1);
}

TEST(atomic, xor16) {
    u16 a = 0x5a00;
    EXPECT_EQ(atomic_xor16(&a, 0xa500), 0x5a00);
    EXPECT_EQ(a, 0x5a00 ^ 0xa500);

    u16 c = 0x1f00;
    u16 d = 0xf100;
    EXPECT_EQ(atomic_xor(&c, d), 0x1f00);
    EXPECT_EQ(c, 0x1f00 ^ 0xf100);
}

TEST(atomic, xor32) {
    u32 a = 0x5a000000;
    EXPECT_EQ(atomic_xor32(&a, 0xa5000000), 0x5a000000);
    EXPECT_EQ(a, 0x5a000000 ^ 0xa5000000);

    u32 c = 0x1f000000;
    u32 d = 0xf1000000;
    EXPECT_EQ(atomic_xor(&c, d), 0x1f000000);
    EXPECT_EQ(c, 0x1f000000 ^ 0xf1000000);
}

TEST(atomic, xor64) {
    u64 a = 0x5a00000000000000;
    EXPECT_EQ(atomic_xor64(&a, 0xa500000000000000), 0x5a00000000000000);
    EXPECT_EQ(a, 0x5a00000000000000 ^ 0xa500000000000000);

    u64 c = 0x1f00000000000000;
    u64 d = 0xf100000000000000;
    EXPECT_EQ(atomic_xor(&c, d), 0x1f00000000000000);
    EXPECT_EQ(c, 0x1f00000000000000 ^ 0xf100000000000000);
}

TEST(atomic, add8) {
    i8 a = -7;
    EXPECT_EQ(atomic_add8(&a, 19), -7);
    EXPECT_EQ(a, -7 + 19);

    u8 b = 12;
    EXPECT_EQ(atomic_add8(&b, -5), 12);
    EXPECT_EQ(b, 12 - 5);

    i8 c = -1;
    u8 d = 14;
    EXPECT_EQ(atomic_add(&c, d), 0xff);
    EXPECT_EQ(c, -1 + 14);
}

TEST(atomic, add16) {
    i16 a = -700;
    EXPECT_EQ(atomic_add16(&a, 1900), -700);
    EXPECT_EQ(a, -700 + 1900);

    u16 b = 1200;
    EXPECT_EQ(atomic_add16(&b, -500), 1200);
    EXPECT_EQ(b, 1200 - 500);

    i16 c = -1;
    u16 d = 1400;
    EXPECT_EQ(atomic_add(&c, d), 0xffff);
    EXPECT_EQ(c, -1 + 1400);
}

TEST(atomic, add32) {
    i32 a = -70000;
    EXPECT_EQ(atomic_add32(&a, 190000), -70000);
    EXPECT_EQ(a, -70000 + 190000);

    u32 b = 120000;
    EXPECT_EQ(atomic_add32(&b, -50000), 120000);
    EXPECT_EQ(b, 120000 - 50000);

    i32 c = -1;
    u32 d = 140000;
    EXPECT_EQ(atomic_add(&c, d), 0xffffffff);
    EXPECT_EQ(c, -1 + 140000);
}

TEST(atomic, add64) {
    i64 a = -70000000000;
    EXPECT_EQ(atomic_add64(&a, 190000000000), -70000000000);
    EXPECT_EQ(a, -70000000000 + 190000000000);

    u64 b = 120000000000;
    EXPECT_EQ(atomic_add64(&b, -50000000000), 120000000000);
    EXPECT_EQ(b, 120000000000 - 50000000000);

    i64 c = -1;
    u64 d = 140000000000;
    EXPECT_EQ(atomic_add(&c, d), 0xffffffffffffffff);
    EXPECT_EQ(c, -1 + 140000000000);
}

TEST(atomic, swap8) {
    u8 a = 0xab;
    EXPECT_EQ(atomic_swap8(&a, 0xbc), 0xab);
    EXPECT_EQ(a, 0xbc);

    u8 b = 0xcd;
    u8 c = 0xef;
    EXPECT_EQ(atomic_swap(&b, c), 0xcd);
    EXPECT_EQ(b, 0xef);
    EXPECT_EQ(c, 0xef);
}

TEST(atomic, swap16) {
    u16 a = 0xab00;
    EXPECT_EQ(atomic_swap16(&a, 0xbc00), 0xab00);
    EXPECT_EQ(a, 0xbc00);

    u16 b = 0xcd00;
    u16 c = 0xef00;
    EXPECT_EQ(atomic_swap(&b, c), 0xcd00);
    EXPECT_EQ(b, 0xef00);
    EXPECT_EQ(c, 0xef00);
}

TEST(atomic, swap32) {
    u32 a = 0xab000000;
    EXPECT_EQ(atomic_swap32(&a, 0xbc000000), 0xab000000);
    EXPECT_EQ(a, 0xbc000000);

    u32 b = 0xcd000000;
    u32 c = 0xef000000;
    EXPECT_EQ(atomic_swap(&b, c), 0xcd000000);
    EXPECT_EQ(b, 0xef000000);
    EXPECT_EQ(c, 0xef000000);
}

TEST(atomic, swap64) {
    u64 a = 0xab00000000000000;
    EXPECT_EQ(atomic_swap64(&a, 0xbc00000000000000), 0xab00000000000000);
    EXPECT_EQ(a, 0xbc00000000000000);

    u64 b = 0xcd00000000000000;
    u64 c = 0xef00000000000000;
    EXPECT_EQ(atomic_swap(&b, c), 0xcd00000000000000);
    EXPECT_EQ(b, 0xef00000000000000);
    EXPECT_EQ(c, 0xef00000000000000);
}

TEST(atomic, cas8) {
    u8 data = 0x11;
    u8 cmpv = 0x11;
    u8 wval = 0x22;

    ASSERT_TRUE(atomic_cas8(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas(&data, &cmpv, &wval, 1));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas16) {
    u16 data = 0x1122;
    u16 cmpv = 0x1122;
    u16 wval = 0x2233;

    ASSERT_TRUE(atomic_cas16(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas(&data, &cmpv, &wval, 2));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas32) {
    u32 data = 0x11223344;
    u32 cmpv = 0x11223344;
    u32 wval = 0x2233aabb;

    ASSERT_TRUE(atomic_cas32(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas(&data, &cmpv, &wval, 4));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas64) {
    u64 data = 0x1122334455667788;
    u64 cmpv = 0x1122334455667788;
    u64 wval = 0x2233aabbccddeeff;

    ASSERT_TRUE(atomic_cas64(&data, &cmpv, &wval));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas(&data, &cmpv, &wval, 8));
    ASSERT_EQ(data, wval);
    ASSERT_FALSE(atomic_cas(&data, cmpv, wval));
    ASSERT_EQ(data, wval);
}

TEST(atomic, cas128) {
    MWR_DECL_ALIGN(16) u64 data[] = { 0x1111222233334444, 0x5555666677778888 };
    MWR_DECL_ALIGN(16) u64 cmpv[] = { 0x1111222233334444, 0x5555666677778888 };
    MWR_DECL_ALIGN(16) u64 wval[] = { 0xaaaabbbbccccdddd, 0xeeeeffff00001111 };

    ASSERT_TRUE(atomic_cas128(data, cmpv, wval));
    ASSERT_EQ(data[0], wval[0]);
    ASSERT_EQ(data[1], wval[1]);
    ASSERT_FALSE(atomic_cas(data, cmpv, wval, 16));
    ASSERT_EQ(data[0], wval[0]);
    ASSERT_EQ(data[1], wval[1]);
}

TEST(atomic, min) {
    u32 a = 123;
    EXPECT_EQ(atomic_min(&a, 32), 123);
    EXPECT_EQ(a, 32);
    EXPECT_EQ(atomic_min(&a, 0xfffffffe), 32);
    EXPECT_EQ(a, 32);
    EXPECT_EQ(atomic_min(&a, -2), 32); // uses signed version
    EXPECT_EQ(a, 0xfffffffe);
}

TEST(atomic, max) {
    u32 a = 123;
    EXPECT_EQ(atomic_max(&a, 132), 123);
    EXPECT_EQ(a, 132);
    EXPECT_EQ(atomic_max(&a, -2), 132); // uses signed version
    EXPECT_EQ(a, 132);
    EXPECT_EQ(atomic_max(&a, 0xfffffffe), 132);
    EXPECT_EQ(a, 0xfffffffe);
}
