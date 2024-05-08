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
#include "mwr/utils/license.h"

#if __GNUC__ >= 14
#pragma GCC diagnostic ignored "-Wdangling-reference"
#endif

TEST(license, apache) {
    auto& license = mwr::license::find("Apache-2.0");
    EXPECT_EQ(license.name, "Apache License 2.0");
}

TEST(license, custom) {
    mwr::license custom("custom", "test license", true);
    auto& license = mwr::license::find("custom");
    EXPECT_EQ(license.name, custom.name);
}

TEST(license, copyright) {
    auto& license = mwr::license::find("Copyright 2022 Company");
    EXPECT_EQ(license, mwr::UNLICENSED);
    EXPECT_FALSE(license.libre);
}
