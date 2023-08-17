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
#include "mwr/core/bitops.h"

using namespace mwr;

TEST(bitops, width_of) {
    u32 x = 0;
    EXPECT_EQ(width_of(x), 32);
    EXPECT_EQ(width_of<i16>(), 16);
}

TEST(bitops, clz32) {
    EXPECT_EQ(clz(~0u), 0);
    EXPECT_EQ(clz(0u), 32);
    EXPECT_EQ(clz(0x8u), 28);
}

TEST(bitops, clz64) {
    EXPECT_EQ(clz(~0ull), 0);
    EXPECT_EQ(clz(0ull), 64);
    EXPECT_EQ(clz(0x8ull), 60);
}

TEST(bitops, ctz32) {
    EXPECT_EQ(ctz(~0u), 0);
    EXPECT_EQ(ctz(0u), 32);
    EXPECT_EQ(ctz(0x8u), 3);
    EXPECT_EQ(ctz(0x80000000u), 31);
}

TEST(bitops, ctz64) {
    EXPECT_EQ(ctz(~0ull), 0);
    EXPECT_EQ(ctz(0ull), 64);
    EXPECT_EQ(ctz(0x8ull), 3);
    EXPECT_EQ(ctz(0x8000000000000000ull), 63);
}

TEST(bitops, ffs32) {
    EXPECT_EQ(ffs(0u), -1);
    EXPECT_EQ(ffs(1u << 31 | 1u << 0), 0);
    EXPECT_EQ(ffs(1u << 31 | 1u << 7), 7);
    EXPECT_EQ(ffs(1u << 31 | 1u << 15), 15);
    EXPECT_EQ(ffs(1u << 31), 31);
}

TEST(bitops, ffs64) {
    EXPECT_EQ(ffs(0ull), -1);
    EXPECT_EQ(ffs(1ull << 63 | 1ull << 0), 0);
    EXPECT_EQ(ffs(1ull << 63 | 1ull << 7), 7);
    EXPECT_EQ(ffs(1ull << 63 | 1ull << 15), 15);
    EXPECT_EQ(ffs(1ull << 63 | 1ull << 31), 31);
    EXPECT_EQ(ffs(1ull << 63), 63);
}

TEST(bitops, fls32) {
    EXPECT_EQ(fls(0u), -1);
    EXPECT_EQ(fls(1u | 1u << 0), 0);
    EXPECT_EQ(fls(1u | 1u << 7), 7);
    EXPECT_EQ(fls(1u | 1u << 15), 15);
    EXPECT_EQ(fls(1u | 1u << 31), 31);
}

TEST(bitops, fls64) {
    EXPECT_EQ(fls(0ull), -1);
    EXPECT_EQ(fls(1ull | 1ull << 0), 0);
    EXPECT_EQ(fls(1ull | 1ull << 7), 7);
    EXPECT_EQ(fls(1ull | 1ull << 15), 15);
    EXPECT_EQ(fls(1ull | 1ull << 31), 31);
    EXPECT_EQ(fls(1ull | 1ull << 63), 63);
}

TEST(bitops, popcnt32) {
    EXPECT_EQ(popcnt(0u), 0);
    EXPECT_EQ(popcnt(~0u), 32);
    EXPECT_EQ(popcnt(1u), 1);
    EXPECT_EQ(popcnt(0xf0f0f0f0u), 16);
}

TEST(bitops, popcnt64) {
    EXPECT_EQ(popcnt(0ull), 0);
    EXPECT_EQ(popcnt(~0ull), 64);
    EXPECT_EQ(popcnt(1ull), 1);
    EXPECT_EQ(popcnt(0b10110111101000011ull), 10);
    EXPECT_EQ(popcnt(0xf0f0f0f0f0f0f0f0ull), 32);
}

TEST(bitops, parity32) {
    EXPECT_EQ(parity(1u), 1u);
    EXPECT_EQ(parity(3u), 0u);
    EXPECT_EQ(parity(7u), 1u);
}

TEST(bitops, parity64) {
    EXPECT_EQ(parity(0x1f00000000000000ull), 1u);
    EXPECT_EQ(parity(0x0f00000000000000ull), 0u);
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

TEST(bitops, bitmask) {
    EXPECT_EQ(bitmask(8, 12), 0x000ff000);
    EXPECT_EQ(bitmask(1, 64), 0x0);
    EXPECT_EQ(bitmask(64, 0), ~0ull);
}

TEST(bitops, fits) {
    size_t val0 = 0;
    EXPECT_TRUE(fits_i8(val0));

    size_t val1 = 127;
    EXPECT_TRUE(fits_i8(val1));

    size_t val2 = 128;
    EXPECT_FALSE(fits_i8(val2));

    size_t val3 = -128;
    EXPECT_TRUE(fits_i8(val3));

    size_t val4 = -128;
    EXPECT_TRUE(fits_i8(val4));
}

TEST(bitops, encode_size) {
    EXPECT_EQ(encode_size(127), 8);
    EXPECT_EQ(encode_size(127u), 8);
    EXPECT_EQ(encode_size(128), 16);
    EXPECT_EQ(encode_size(128u), 8);

    EXPECT_EQ(encode_size<i32>(127), 8);
    EXPECT_EQ(encode_size<i64>(127), 8);
    EXPECT_EQ(encode_size<u32>(127), 8);
    EXPECT_EQ(encode_size<u64>(127), 8);

    EXPECT_EQ(encode_size<i32>(127u), 8);
    EXPECT_EQ(encode_size<i64>(127u), 8);
    EXPECT_EQ(encode_size<u32>(127u), 8);
    EXPECT_EQ(encode_size<u64>(127u), 8);

    EXPECT_EQ(encode_size<i32>(128), 16);
    EXPECT_EQ(encode_size<u32>(128), 8);
    EXPECT_EQ(encode_size<i64>(128), 16);
    EXPECT_EQ(encode_size<u64>(128), 8);

    EXPECT_EQ(encode_size<i32>(128u), 16);
    EXPECT_EQ(encode_size<u32>(128u), 8);
    EXPECT_EQ(encode_size<i64>(128u), 16);
    EXPECT_EQ(encode_size<u64>(128u), 8);

    EXPECT_EQ(encode_size<i64>(0x80000000), 64);
    EXPECT_EQ(encode_size<u64>(0x80000000), 32);
}

TEST(bitops, pow2) {
    EXPECT_FALSE(is_pow2(0));
    EXPECT_FALSE(is_pow2(255));
    EXPECT_FALSE(is_pow2(1020));
    EXPECT_FALSE(is_pow2(779632));

    EXPECT_TRUE(is_pow2(1));
    EXPECT_TRUE(is_pow2(2));
    EXPECT_TRUE(is_pow2(32));
    EXPECT_TRUE(is_pow2(4096));
    EXPECT_TRUE(is_pow2(65536));
    EXPECT_TRUE(is_pow2(2097152));
    EXPECT_TRUE(is_pow2(274877906944));
}

TEST(bitops, log2i) {
    EXPECT_EQ(log2i(2), 1);
    EXPECT_EQ(log2i(4), 2);
    EXPECT_EQ(log2i(8), 3);
    EXPECT_EQ(log2i(16), 4);
    EXPECT_EQ(log2i(32), 5);
    EXPECT_EQ(log2i(64), 6);
    EXPECT_EQ(log2i(128), 7);
    EXPECT_EQ(log2i(256), 8);
    EXPECT_EQ(log2i(512), 9);
    EXPECT_EQ(log2i(1024), 10);
    EXPECT_EQ(log2i(2048), 11);
    EXPECT_EQ(log2i(4096), 12);
    EXPECT_EQ(log2i(8192), 13);
    EXPECT_EQ(log2i(16384), 14);
    EXPECT_EQ(log2i(32768), 15);
}

TEST(bitops, fourcc) {
    EXPECT_EQ(fourcc("vcml"), 0x6c6d6376);
    EXPECT_EQ(fourcc("virt"), 0x74726976);
    EXPECT_EQ(fourcc("\0\0\0\0"), 0);
}

TEST(bitops, bitrev) {
    // 32bit
    EXPECT_EQ(bitrev((u32)0x01020304), 0x20c04080);
    EXPECT_EQ(bitrev((u32)0x77e51f08), 0x10f8a7ee);
    EXPECT_EQ(bitrev((u32)0xcb3e0121), 0x84807cd3);
    EXPECT_EQ(bitrev((u32)0xf490ee20), 0x0477092f);

    // 64bit
    EXPECT_EQ(bitrev((u64)0x48ec4111ea60e4a5), 0xa527065788823712);
    EXPECT_EQ(bitrev((u64)0x1e657cbd6d9a8532), 0x4ca159b6bd3ea678);
    EXPECT_EQ(bitrev((u64)0x8f5afea5ee02d82d), 0xb41b4077a57f5af1);
    EXPECT_EQ(bitrev((u64)0x97553f96847066c6), 0x63660e2169fcaae9);
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
