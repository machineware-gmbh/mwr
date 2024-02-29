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

#include "mwr/core/bitops.h"
#include "mwr/utils/memory.h"

using namespace mwr;

TEST(memory, allocation) {
    size_t pgsz = memory::page_size();
    ASSERT_GT(pgsz, 0);

    memory mem(pgsz + 1);
    EXPECT_EQ(mem.size(), pgsz + 1);
    EXPECT_EQ(mem.total_size(), 2 * pgsz);
    EXPECT_TRUE(is_aligned(mem.raw(), pgsz));

    mem[10] = 0xab;
    EXPECT_EQ(mem.raw()[10], 0xab);
    EXPECT_EQ(mem[10], 0xab);
    EXPECT_EQ(&mem[10], mem + 10);
}

TEST(memory, free) {
    size_t pgsz = memory::page_size();
    memory mem(pgsz);
    EXPECT_TRUE(mem.raw() != nullptr);
    mem.free();
    EXPECT_TRUE(mem.raw() == nullptr);
    EXPECT_EQ(mem.total_size(), 0);
    EXPECT_EQ(mem.size(), 0);
}

TEST(memory, alloc) {
    memory mem;
    EXPECT_TRUE(mem.raw() == nullptr);
    EXPECT_EQ(mem.total_size(), 0);
    EXPECT_EQ(mem.size(), 0);

    mem.alloc(1);
    EXPECT_TRUE(mem.raw() != nullptr);
    EXPECT_EQ(mem.size(), 1);
    EXPECT_GE(mem.total_size(), mem.size());
}
