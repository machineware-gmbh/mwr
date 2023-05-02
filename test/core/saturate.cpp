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
#include "mwr/core/saturate.h"

using namespace mwr;

TEST(saturate, unsigned_add8) {
    bool sat;
    u8 a, b, c;

    a = 250;
    b = 5;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 255);
    EXPECT_FALSE(sat);

    a = 5;
    b = 251;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 255);
    EXPECT_TRUE(sat);
}

TEST(saturate, unsigned_add16) {
    bool sat;
    u16 a, b, c;

    a = 0xfff0;
    b = 15;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 0xffff);
    EXPECT_FALSE(sat);

    a = 15;
    b = 0xfff1;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 0xffff);
    EXPECT_TRUE(sat);
}

TEST(saturate, signed_add8) {
    bool sat;
    i8 a, b, c;

    a = 120;
    b = 7;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 127);
    EXPECT_FALSE(sat);

    a = 7;
    b = 121;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 127);
    EXPECT_TRUE(sat);
}

TEST(saturate, signed_add16) {
    bool sat;
    i16 a, b, c;

    a = -32760;
    b = -8;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, -32768);
    EXPECT_FALSE(sat);

    a = 32760;
    b = 8;
    c = sat_add(a, b, sat);

    EXPECT_EQ(c, 32767);
    EXPECT_TRUE(sat);
}

TEST(saturate, unsigned_sub8) {
    bool sat;
    u8 a, b, c;

    a = 255;
    b = 255;
    c = sat_sub(a, b, sat);

    EXPECT_EQ(c, 0);
    EXPECT_FALSE(sat);

    a = 128;
    b = 255;
    c = sat_sub(a, b, sat);

    EXPECT_EQ(c, 0);
    EXPECT_TRUE(sat);
}

TEST(saturate, signed_sub8) {
    bool sat;
    i8 a, b, c;

    a = -1;
    b = 127;
    c = sat_sub(a, b, sat);

    EXPECT_EQ(c, -128);
    EXPECT_FALSE(sat);

    a = -120;
    b = 10;
    c = sat_sub(a, b, sat);

    EXPECT_EQ(c, -128);
    EXPECT_TRUE(sat);

    a = 126;
    b = 127;
    c = sat_sub(a, b, sat);

    EXPECT_EQ(c, -1);
    EXPECT_FALSE(sat);

    a = 10;
    b = -120;
    c = sat_sub(a, b, sat);

    EXPECT_EQ(c, 127);
    EXPECT_TRUE(sat);
}
