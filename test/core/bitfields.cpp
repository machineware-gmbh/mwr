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
#include "mwr/core/bitfields.h"

using namespace mwr;

TEST(bitops, extract32) {
    EXPECT_EQ(extract(0b01011001u, 3, 2), 0b11u);
    EXPECT_EQ(extract(0b11000110u, 2, 3), 0b001u);
    EXPECT_EQ(extract(0b10100000u, 5, 3), 0b101u);
    EXPECT_EQ(extract(0b11111111u, 0, 0), 0);
    EXPECT_EQ(extract(0b11111111u, 31, 0), 0);
}

TEST(bitops, extract64) {
    EXPECT_EQ(extract(0b01011001ull, 3, 2), 0b11ul);
    EXPECT_EQ(extract(0b11000110ull, 2, 3), 0b001ul);
    EXPECT_EQ(extract(0b10100000ull, 5, 3), 0b101ul);
    EXPECT_EQ(extract(0b11111111ull, 0, 0), 0);
    EXPECT_EQ(extract(0b11111111ull, 63, 0), 0);
}

TEST(bitops, deposit32) {
    EXPECT_EQ(deposit(0u, 3, 2, 0b101), 0b1000u);
    EXPECT_EQ(deposit(0u, 2, 3, 0b101), 0b10100u);
    EXPECT_EQ(deposit(0u, 5, 3, 0b101), 0b10100000u);
}

TEST(bitops, deposit64) {
    EXPECT_EQ(deposit(0ull, 3, 2, 0b101), 0b1000ul);
    EXPECT_EQ(deposit(0ull, 2, 3, 0b101), 0b10100ul);
    EXPECT_EQ(deposit(0ull, 5, 3, 0b101), 0b10100000ul);
}

typedef field<0, 3, u32> TEST_FIELD1;
typedef field<4, 1, u32> TEST_FIELD2;

TEST(bitops, field) {
    EXPECT_EQ(TEST_FIELD1::MASK, 0b111);
    EXPECT_EQ(TEST_FIELD2::MASK, 0b10000);

    EXPECT_EQ(0xffff & TEST_FIELD1(), 0b111);
    EXPECT_EQ(0xffff & TEST_FIELD2(), 0b10000);

    u32 val = 0;

    set_field<TEST_FIELD1>(val, 0b101);
    EXPECT_EQ(val, 0b101);
    EXPECT_EQ(get_field<TEST_FIELD1>(val), 0b101);

    set_field<TEST_FIELD2>(val, true);
    EXPECT_EQ(get_field<TEST_FIELD2>(val), 1);
    EXPECT_EQ(val, 0b10101);

    set_field<TEST_FIELD1>(val, 0);
    EXPECT_EQ(get_field<TEST_FIELD1>(val), 0);
    EXPECT_EQ(val, 0b10000);

    set_field<TEST_FIELD2>(val, 0);
    EXPECT_EQ(get_field<TEST_FIELD2>(val), 0);
    EXPECT_EQ(val, 0b00000);

    set_field<TEST_FIELD2>(val, 0xf0);
    EXPECT_EQ(val, 0b00000);
}
