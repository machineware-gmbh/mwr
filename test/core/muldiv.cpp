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

        ASSERT_EQ(l0, l1);
        ASSERT_EQ(h0, h1);
    }
}

static void add128(u64& hi, u64& lo, u64 s) {
    u64 temp = lo;
    lo += s;
    if (lo < temp)
        hi++;
}

TEST(muldiv, udivmod128) {
    std::mt19937 rng;
    std::uniform_int_distribution<u64> dist;

    for (size_t i = 0; i < 1000; i++) {
        u64 lo = dist(rng);
        u64 hi = dist(rng);
        u64 d = dist(rng);

        // we only return the bottom 64bits of the division result, so in
        // order for our test verifcation to work, we clamp hi to d, so that
        // the result definitly fits into 64bits without truncation.
        if (d)
            hi %= d;

        u64 div = udiv128lo(hi, lo, d);
        u64 mod = umod128(hi, lo, d);

        if (d == 0) {
            ASSERT_EQ(div, 0u);
            ASSERT_EQ(mod, 0u);
        } else {
            u64 reslo, reshi;
            umul64(reshi, reslo, div, d);
            add128(reshi, reslo, mod);
            ASSERT_EQ(hi, reshi);
            ASSERT_EQ(lo, reslo);
        }
    }
}

TEST(muldiv, idivmod128) {
    std::mt19937 rng;
    std::uniform_int_distribution<i64> dist;

    for (size_t i = 0; i < 1000; i++) {
        i64 lo = dist(rng);
        i64 hi = dist(rng);
        i64 d = dist(rng);

        // we only return the bottom 64bits of the division result, so in
        // order for our test verifcation to work, we clamp hi so that
        // |hi| < |d|, ensuring the result fits in 64bits without truncation.
        if (d > 0)
            hi %= d;
        if (d < 0)
            hi %= -d;

        i64 div = idiv128lo(hi, lo, d);
        i64 mod = imod128(hi, lo, d);

        if (d == 0) {
            ASSERT_EQ(div, 0);
            ASSERT_EQ(mod, 0);
        } else {
            // Use unsigned for the reconstruction to avoid signed overflow
            // issues The bit patterns are identical in two's complement
            u64 reslo, reshi;
            umul64(reshi, reslo, (u64)div, (u64)d);
            add128(reshi, reslo, mod);
            ASSERT_EQ((u64)hi, reshi);
            ASSERT_EQ((u64)lo, reslo);
        }
    }
}
