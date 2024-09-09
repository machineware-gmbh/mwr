/******************************************************************************
 *                                                                            *
 * Copyright (C) 2024 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"

#include "mwr.h"

using namespace mwr;

const vector<u8> V1 = { 0x21, 0x46, 0x01, 0x36, 0x01, 0x21, 0x47, 0x01,
                        0x36, 0x00, 0x7E, 0xFE, 0x09, 0xD2, 0x19, 0x01 };

const vector<u8> V2 = { 0x21, 0x46, 0x01, 0x7E, 0x17, 0xC2, 0x00, 0x01,
                        0xFF, 0x5F, 0x16, 0x00, 0x21, 0x48, 0x01, 0x19 };

const vector<u8> V3 = { 0x19, 0x4E, 0x79, 0x23, 0x46, 0x23, 0x96, 0x57,
                        0x78, 0x23, 0x9E, 0xDA, 0x3F, 0x01, 0xB2, 0xCA };

const vector<u8> V4 = { 0x3F, 0x01, 0x56, 0x70, 0x2B, 0x5E, 0x71, 0x2B,
                        0x72, 0x2B, 0x73, 0x21, 0x46, 0x01, 0x34, 0x21 };

TEST(ihex, load) {
    ihex reader(get_resource_path("sample.ihex"));

    EXPECT_EQ(reader.start_addr(), 0xcd);
    ASSERT_EQ(reader.records().size(), 4);

    EXPECT_EQ(reader.records()[0].addr, 0x0100);
    EXPECT_EQ(reader.records()[1].addr, 0x0110);
    EXPECT_EQ(reader.records()[2].addr, 0x0120 | 0x800 << 16);
    EXPECT_EQ(reader.records()[3].addr, 0x0130 | 0x800 << 16 | 0x1 << 4);

    EXPECT_EQ(reader.records()[0].data, V1);
    EXPECT_EQ(reader.records()[1].data, V2);
    EXPECT_EQ(reader.records()[2].data, V3);
    EXPECT_EQ(reader.records()[3].data, V4);
}
