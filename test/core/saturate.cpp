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
