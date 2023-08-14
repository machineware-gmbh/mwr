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

using namespace mwr;

int global = 0;

MWR_CONSTRUCTOR(init_global) {
    global = 10;
}

TEST(core, init) {
    ASSERT_EQ(global, 10);
}

