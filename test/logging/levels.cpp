/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"
#include "mwr.h"

TEST(levels, operators) {
    std::stringstream ss("error warning info debug");
    mwr::log_level lvl;

    ss >> lvl;
    EXPECT_EQ(lvl, mwr::LOG_ERROR);
    ss >> lvl;
    EXPECT_EQ(lvl, mwr::LOG_WARN);
    ss >> lvl;
    EXPECT_EQ(lvl, mwr::LOG_INFO);
    ss >> lvl;
    EXPECT_EQ(lvl, mwr::LOG_DEBUG);
}
