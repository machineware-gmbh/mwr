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
#include "mwr/core/compiler.h"

int global = 0;

MWR_CONSTRUCTOR(do_init) {
    global = 1;
}

TEST(compiler, constructor) {
    EXPECT_EQ(global, 1);
}

#define MACRO macro
#define ABCD  "abcd"

TEST(compiler, str) {
    EXPECT_STREQ(MWR_STR(hello), "hello");
    EXPECT_STREQ(MWR_STR(MACRO), "macro");
    EXPECT_STREQ(MWR_CAT(AB, CD), "abcd");
}
