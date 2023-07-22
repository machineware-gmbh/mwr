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

#include "mwr.h"

TEST(elf32, init) {
    mwr::elf reader(get_resource_path("elf32.elf"));

    EXPECT_TRUE(reader.is_little_endian());
    EXPECT_TRUE(reader.is_32bit());
    EXPECT_EQ(reader.entry(), 0x8049b60);
    EXPECT_EQ(reader.machine(), mwr::elf::X86);
    EXPECT_EQ(reader.segments().size(), 4);
}

TEST(elf64, init) {
    mwr::elf reader(get_resource_path("elf64.elf"));

    EXPECT_TRUE(reader.is_little_endian());
    EXPECT_TRUE(reader.is_64bit());
    EXPECT_EQ(reader.entry(), 0x401020);
    EXPECT_EQ(reader.machine(), mwr::elf::X86_64);
    EXPECT_EQ(reader.segments().size(), 4);
}

TEST(elf32, segments) {
    mwr::elf reader(get_resource_path("elf32.elf"));

    EXPECT_TRUE(reader.is_little_endian());

    auto segments = reader.segments();
    ASSERT_EQ(segments.size(), 4);

    EXPECT_EQ(segments[0].size, 0x204);
    EXPECT_TRUE(segments[0].r);
    EXPECT_FALSE(segments[0].w);
    EXPECT_FALSE(segments[0].x);

    EXPECT_EQ(segments[1].size, 0x67a45);
    EXPECT_TRUE(segments[1].r);
    EXPECT_FALSE(segments[1].w);
    EXPECT_TRUE(segments[1].x);

    EXPECT_EQ(segments[2].size, 0x2d2bd);
    EXPECT_TRUE(segments[2].r);
    EXPECT_FALSE(segments[2].w);
    EXPECT_FALSE(segments[2].x);

    EXPECT_EQ(segments[3].size, 0x36e8);
    EXPECT_TRUE(segments[3].r);
    EXPECT_TRUE(segments[3].w);
    EXPECT_FALSE(segments[3].x);
}

TEST(elf64, segments) {
    mwr::elf reader(get_resource_path("elf64.elf"));

    EXPECT_TRUE(reader.is_little_endian());

    auto segments = reader.segments();
    ASSERT_EQ(segments.size(), 4);

    EXPECT_EQ(segments[0].size, 0x27c);
    EXPECT_TRUE(segments[0].r);
    EXPECT_FALSE(segments[0].w);
    EXPECT_FALSE(segments[0].x);

    EXPECT_EQ(segments[1].size, 0x20d);
    EXPECT_TRUE(segments[1].r);
    EXPECT_FALSE(segments[1].w);
    EXPECT_TRUE(segments[1].x);

    EXPECT_EQ(segments[2].size, 0xb8);
    EXPECT_TRUE(segments[2].r);
    EXPECT_FALSE(segments[2].w);
    EXPECT_FALSE(segments[2].x);

    EXPECT_EQ(segments[3].size, 0xc);
    EXPECT_TRUE(segments[3].r);
    EXPECT_TRUE(segments[3].w);
    EXPECT_FALSE(segments[3].x);

    std::vector<mwr::u8> seg0(segments[0].size);
    std::vector<mwr::u8> seg3(segments[3].size);

    EXPECT_EQ(reader.read_segment(segments[0], seg0.data()), seg0.size());
    EXPECT_EQ(reader.read_segment(segments[3], seg3.data()), seg3.size());

    mwr::u8 code[4] = {
        0x7f, 0x45, 0x4c, 0x46, // ELF header at start of code
    };

    mwr::u8 data[12] = {
        0x42, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // long global_b = 0x42
        0x04, 0x0, 0x0, 0x0,                     // int global_a = 4;
    };

    EXPECT_EQ(memcmp(seg0.data(), code, 4), 0);
    EXPECT_EQ(memcmp(seg3.data(), data, 12), 0);
}

TEST(elf64, symbols) {
    mwr::elf reader(get_resource_path("elf64.elf"));

    EXPECT_TRUE(reader.is_little_endian());

    const mwr::elf::symbol* global_a = reader.find_symbol("global_a");
    ASSERT_TRUE(global_a);
    EXPECT_EQ(global_a->name, "global_a");
    EXPECT_EQ(global_a->kind, mwr::elf::KIND_OBJECT);
    EXPECT_EQ(global_a->bind, mwr::elf::BIND_GLOBAL);
    EXPECT_EQ(global_a->virt, 0x404008);
    EXPECT_EQ(global_a->size, 4);

    const mwr::elf::symbol* global_b = reader.find_symbol("global_b");
    ASSERT_TRUE(global_b);
    EXPECT_EQ(global_b->name, "global_b");
    EXPECT_EQ(global_b->kind, mwr::elf::KIND_OBJECT);
    EXPECT_EQ(global_b->bind, mwr::elf::BIND_GLOBAL);
    EXPECT_EQ(global_b->virt, 0x404000);
    EXPECT_EQ(global_b->size, 8);

    const mwr::elf::symbol* global_tls = reader.find_symbol("global_tls");
    ASSERT_TRUE(global_tls);
    EXPECT_EQ(global_tls->name, "global_tls");
    EXPECT_EQ(global_tls->kind, mwr::elf::KIND_TLS);
    EXPECT_EQ(global_tls->bind, mwr::elf::BIND_GLOBAL);
    EXPECT_EQ(global_tls->size, 4);

    const mwr::elf::symbol* func_c = reader.find_symbol("func_c");
    ASSERT_TRUE(func_c);
    EXPECT_EQ(func_c->name, "func_c");
    EXPECT_EQ(func_c->kind, mwr::elf::KIND_FUNC);
    EXPECT_EQ(func_c->bind, mwr::elf::BIND_GLOBAL);
    EXPECT_EQ(func_c->virt, 0x401000);
    EXPECT_EQ(func_c->size, 22);
}
