/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include <filesystem>

#include "testing.h"

#include "mwr.h"

TEST(fdt, to_string) {
    auto root = mwr::fdtnode("/");
    auto& a = root.add_child("node_a");
    auto& b = root.add_child("node_b");
    a.add_property("integer", 0x123);
    a.add_property("array", { 0x1, 0x2, 0x3 });
    b.add_property("string", "hello world");
    b.add_property("list", { "hello", "world" });

    EXPECT_EQ(to_string(root),
              "/ {\n"
              "    node_a {\n"
              "        integer = <0x123>;\n"
              "        array = <0x1 0x2 0x3>;\n"
              "    };\n"
              "    node_b {\n"
              "        string = \"hello world\";\n"
              "        list = \"hello\", \"world\";\n"
              "    };\n"
              "};");
}

TEST(fdt, memory) {
    auto root = mwr::fdtnode("/");
    auto& test = root.add_child("test");
    test.add_property("prop_a", { 0x123 });
    test.add_property("prop_b", { 0x456, 0x678 });
    test.add_property("compatible", { "foo", "bar" });
    test.add_property("parent", { test.handle() });

    mwr::u32 memory[54] = {};
    mwr::fdtcompile(root, memory, sizeof(memory));

    const mwr::u32 expected[54] = {
        0xd00dfeed, 0x000000d8, 0x00000038, 0x000000b0, 0x00000028, 0x00000011,
        0x00000010, 0x00000000, 0x00000028, 0x00000078, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x74657374,
        0x00000000, 0x00000003, 0x00000004, 0x00000000, 0x00000123, 0x00000003,
        0x00000008, 0x00000007, 0x00000456, 0x00000678, 0x00000003, 0x00000008,
        0x0000000e, 0x666f6f00, 0x62617200, 0x00000003, 0x00000004, 0x00000019,
        0x00000001, 0x00000003, 0x00000004, 0x00000021, 0x00000001, 0x00000002,
        0x00000002, 0x00000009, 0x70726f70, 0x5f610070, 0x726f705f, 0x6200636f,
        0x6d706174, 0x69626c65, 0x00706861, 0x6e646c65, 0x00706172, 0x656e7400,
    };

    for (int i = 0; i < 54; i++)
        EXPECT_EQ(mwr::bswap(memory[i]), expected[i]) << "word " << i;
}

TEST(fdt, file) {
    auto root = mwr::fdtnode("/");
    auto& test = root.add_child("test");
    test.add_property("prop_a", { 0x123 });
    test.add_property("prop_b", { 0x456, 0x678 });
    test.add_property("compatible", { "foo", "bar" });
    test.add_property("parent", { test.handle() });

    mwr::fdtcompile(root, "test.dtb");

    std::fstream file("test.dtb", std::ios::in | std::ios::binary);
    ASSERT_TRUE(file);

    mwr::u32 memory[54] = {};
    file.read((char*)memory, sizeof(memory));
    ASSERT_TRUE(file);
    file.close();

    const mwr::u32 expected[54] = {
        0xd00dfeed, 0x000000d8, 0x00000038, 0x000000b0, 0x00000028, 0x00000011,
        0x00000010, 0x00000000, 0x00000028, 0x00000078, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x74657374,
        0x00000000, 0x00000003, 0x00000004, 0x00000000, 0x00000123, 0x00000003,
        0x00000008, 0x00000007, 0x00000456, 0x00000678, 0x00000003, 0x00000008,
        0x0000000e, 0x666f6f00, 0x62617200, 0x00000003, 0x00000004, 0x00000019,
        0x00000002, 0x00000003, 0x00000004, 0x00000021, 0x00000002, 0x00000002,
        0x00000002, 0x00000009, 0x70726f70, 0x5f610070, 0x726f705f, 0x6200636f,
        0x6d706174, 0x69626c65, 0x00706861, 0x6e646c65, 0x00706172, 0x656e7400,
    };

    for (int i = 0; i < 54; i++)
        EXPECT_EQ(mwr::bswap(memory[i]), expected[i]) << "word " << i;

    std::filesystem::remove("test.dtb");
}
