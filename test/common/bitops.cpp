/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 *                                                                            *
 ******************************************************************************/

#include <gtest/gtest.h>
#include "mwr.h"
using namespace mwr;

TEST(bitops, clz32) {
    EXPECT_EQ(clz(~0u), 0);
    EXPECT_EQ(clz(0u), 32);
    EXPECT_EQ(clz(0x8u), 28);
}

TEST(bitops, clz64) {
    EXPECT_EQ(clz(~0ul), 0);
    EXPECT_EQ(clz(0ul), 64);
    EXPECT_EQ(clz(0x8ul), 60);
}

TEST(bitops, ctz32) {
    EXPECT_EQ(ctz(~0u), 0);
    EXPECT_EQ(ctz(0u), 32);
    EXPECT_EQ(ctz(0x8u), 3);
    EXPECT_EQ(ctz(0x80000000u), 31);
}

TEST(bitops, ctz64) {
    EXPECT_EQ(ctz(~0ul), 0);
    EXPECT_EQ(ctz(0ul), 64);
    EXPECT_EQ(ctz(0x8ul), 3);
    EXPECT_EQ(ctz(0x8000000000000000ul), 63);
}

TEST(bitops, ffs32) {
    EXPECT_EQ(ffs(0u), -1);
    EXPECT_EQ(ffs(1u << 31 | 1u << 0), 0);
    EXPECT_EQ(ffs(1u << 31 | 1u << 7), 7);
    EXPECT_EQ(ffs(1u << 31 | 1u << 15), 15);
    EXPECT_EQ(ffs(1u << 31), 31);
}

TEST(bitops, ffs64) {
    EXPECT_EQ(ffs(0ul), -1);
    EXPECT_EQ(ffs(1ul << 63 | 1ul << 0), 0);
    EXPECT_EQ(ffs(1ul << 63 | 1ul << 7), 7);
    EXPECT_EQ(ffs(1ul << 63 | 1ul << 15), 15);
    EXPECT_EQ(ffs(1ul << 63 | 1ul << 31), 31);
    EXPECT_EQ(ffs(1ul << 63), 63);
}

TEST(bitops, fls32) {
    EXPECT_EQ(fls(0u), -1);
    EXPECT_EQ(fls(1u | 1u << 0), 0);
    EXPECT_EQ(fls(1u | 1u << 7), 7);
    EXPECT_EQ(fls(1u | 1u << 15), 15);
    EXPECT_EQ(fls(1u | 1u << 31), 31);
}

TEST(bitops, fls64) {
    EXPECT_EQ(fls(0ul), -1);
    EXPECT_EQ(fls(1ul | 1ul << 0), 0);
    EXPECT_EQ(fls(1ul | 1ul << 7), 7);
    EXPECT_EQ(fls(1ul | 1ul << 15), 15);
    EXPECT_EQ(fls(1ul | 1ul << 31), 31);
    EXPECT_EQ(fls(1ul | 1ul << 63), 63);
}

TEST(bitops, popcnt32) {
    EXPECT_EQ(popcnt(0u), 0);
    EXPECT_EQ(popcnt(~0u), 32);
    EXPECT_EQ(popcnt(1u), 1);
    EXPECT_EQ(popcnt(0xf0f0f0f0u), 16);
}

TEST(bitops, popcnt64) {
    EXPECT_EQ(popcnt(0ul), 0);
    EXPECT_EQ(popcnt(~0ul), 64);
    EXPECT_EQ(popcnt(1ul), 1);
    EXPECT_EQ(popcnt(0b10110111101000011u), 10);
    EXPECT_EQ(popcnt(0xf0f0f0f0f0f0f0f0ul), 32);
}

TEST(bitops, parity32) {
    EXPECT_EQ(parity(1u), 1u);
    EXPECT_EQ(parity(3u), 0u);
    EXPECT_EQ(parity(7u), 1u);
}

TEST(bitops, parity64) {
    EXPECT_EQ(parity(0x1f00000000000000ul), 1u);
    EXPECT_EQ(parity(0x0f00000000000000ul), 0u);
}

TEST(bitops, parity8) {
    EXPECT_EQ(parity<u8>(0x00), 0u);
    EXPECT_EQ(parity<u8>(0x11), 0u);
    EXPECT_EQ(parity<u8>(0x01), 1u);
}

TEST(bitops, is_pow2) {
    EXPECT_FALSE(is_pow2(0));
    EXPECT_FALSE(is_pow2(3));
    EXPECT_FALSE(is_pow2(15));
    EXPECT_FALSE(is_pow2(238));

    EXPECT_TRUE(is_pow2(2));
    EXPECT_TRUE(is_pow2(4));
    EXPECT_TRUE(is_pow2(64));
    EXPECT_TRUE(is_pow2(256));
}

TEST(bitops, fourcc) {
    EXPECT_EQ(fourcc("vcml"), 0x6c6d6376);
    EXPECT_EQ(fourcc("virt"), 0x74726976);
    EXPECT_EQ(fourcc("\0\0\0\0"), 0);
}

TEST(bitops, bitrev) {
    EXPECT_EQ(bitrev((u32)0x01020304), 0x20c04080);
    EXPECT_EQ(bitrev((u32)0x77e51f08), 0x10f8a7ee);
    EXPECT_EQ(bitrev((u32)0xcb3e0121), 0x84807cd3);
    EXPECT_EQ(bitrev((u32)0xf490ee20), 0x0477092f);
}

TEST(bitops, bswap) {
    u8 val1 = 0x11;
    u16 val2 = 0x1122;
    u32 val3 = 0x11223344;
    u64 val4 = 0x1122334455667788ull;

    EXPECT_EQ(bswap(val1), 0x11);
    EXPECT_EQ(bswap(val2), 0x2211);
    EXPECT_EQ(bswap(val3), 0x44332211);
    EXPECT_EQ(bswap(val4), 0x8877665544332211ull);
}

TEST(bitops, memswap) {
    u8 x8 = 0x11;
    memswap(&x8, sizeof(x8));
    EXPECT_EQ(x8, 0x11);

    u16 x16 = 0x1122;
    memswap(&x16, sizeof(x16));
    EXPECT_EQ(x16, 0x2211);

    u32 x32 = 0x11223344;
    memswap(&x32, sizeof(x32));
    EXPECT_EQ(x32, 0x44332211);

    u64 x64 = 0x1122334455667788ull;
    memswap(&x64, sizeof(x64));
    EXPECT_EQ(x64, 0x8877665544332211ull);
}

TEST(bitops, crc7) {
    u8 b0[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    u8 b1[] = { 0xff, 0xff, 0xff, 0xff, 0xff };
    u8 b2[] = { 0x11, 0x22, 0x33, 0x44, 0x55 };

    EXPECT_EQ(crc7(b0, sizeof(b0)), 0x00 << 1);
    EXPECT_EQ(crc7(b1, sizeof(b1)), 0x11 << 1);
    EXPECT_EQ(crc7(b2, sizeof(b2)), 0x08 << 1);

    u8 cmd0[] = { 0x40, 0x00, 0x00, 0x00, 0x00 };
    u8 cmd17[] = { 0x51, 0x00, 0x00, 0x00, 0x00 };
    u8 resp[] = { 0x11, 0x00, 0x00, 0x09, 0x00 };

    EXPECT_EQ(crc7(cmd0, sizeof(cmd0)), 0x4a << 1);
    EXPECT_EQ(crc7(cmd17, sizeof(cmd17)), 0x2a << 1);
    EXPECT_EQ(crc7(resp, sizeof(resp)), 0x33 << 1);
}

TEST(bitops, crc16) {
    u8 b0[512] = { 0xFF };
    memset(b0, 0xff, 512);
    EXPECT_EQ(crc16(b0, sizeof(b0)), 0x7fa1);
}

TEST(bitops, crc32) {
    EXPECT_EQ(crc32((const u8*)"123456789", 9), 0x9b63d02c);
}
