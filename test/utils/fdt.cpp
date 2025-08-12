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

#ifdef MWR_HOST_BIG_ENDIAN
#define BE(val) (val)
#else
#define BE(val)                                               \
    (((val & 0x000000ff) << 24) | ((val & 0x0000ff00) << 8) | \
     ((val & 0x00ff0000) >> 8) | ((val & 0xff000000) >> 24))
#endif

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

TEST(fdt, compile_memory) {
    auto root = mwr::fdtnode("/");
    auto& test = root.add_child("test");
    test.add_property("prop_a", { 0x123 });
    test.add_property("prop_b", { 0x456, 0x678 });
    test.add_property("compatible", { "foo", "bar" });
    test.add_property("parent", { test.handle() });

    mwr::u32 memory[54] = {};
    mwr::fdtcompile(root, memory, sizeof(memory));

    const mwr::u32 expected[54] = {
        BE(0xd00dfeed), BE(0x000000d8), BE(0x00000038), BE(0x000000b0),
        BE(0x00000028), BE(0x00000011), BE(0x00000010), BE(0x00000000),
        BE(0x00000028), BE(0x00000078), BE(0x00000000), BE(0x00000000),
        BE(0x00000000), BE(0x00000000), BE(0x00000001), BE(0x00000000),
        BE(0x00000001), BE(0x74657374), BE(0x00000000), BE(0x00000003),
        BE(0x00000004), BE(0x00000000), BE(0x00000123), BE(0x00000003),
        BE(0x00000008), BE(0x00000007), BE(0x00000456), BE(0x00000678),
        BE(0x00000003), BE(0x00000008), BE(0x0000000e), BE(0x666f6f00),
        BE(0x62617200), BE(0x00000003), BE(0x00000004), BE(0x00000019),
        BE(0x00000001), BE(0x00000003), BE(0x00000004), BE(0x00000021),
        BE(0x00000001), BE(0x00000002), BE(0x00000002), BE(0x00000009),
        BE(0x70726f70), BE(0x5f610070), BE(0x726f705f), BE(0x6200636f),
        BE(0x6d706174), BE(0x69626c65), BE(0x00706861), BE(0x6e646c65),
        BE(0x00706172), BE(0x656e7400),
    };

    for (int i = 0; i < 54; i++)
        EXPECT_EQ(memory[i], expected[i]) << "word " << i;
}

TEST(fdt, compile_file) {
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
        BE(0xd00dfeed), BE(0x000000d8), BE(0x00000038), BE(0x000000b0),
        BE(0x00000028), BE(0x00000011), BE(0x00000010), BE(0x00000000),
        BE(0x00000028), BE(0x00000078), BE(0x00000000), BE(0x00000000),
        BE(0x00000000), BE(0x00000000), BE(0x00000001), BE(0x00000000),
        BE(0x00000001), BE(0x74657374), BE(0x00000000), BE(0x00000003),
        BE(0x00000004), BE(0x00000000), BE(0x00000123), BE(0x00000003),
        BE(0x00000008), BE(0x00000007), BE(0x00000456), BE(0x00000678),
        BE(0x00000003), BE(0x00000008), BE(0x0000000e), BE(0x666f6f00),
        BE(0x62617200), BE(0x00000003), BE(0x00000004), BE(0x00000019),
        BE(0x00000002), BE(0x00000003), BE(0x00000004), BE(0x00000021),
        BE(0x00000002), BE(0x00000002), BE(0x00000002), BE(0x00000009),
        BE(0x70726f70), BE(0x5f610070), BE(0x726f705f), BE(0x6200636f),
        BE(0x6d706174), BE(0x69626c65), BE(0x00706861), BE(0x6e646c65),
        BE(0x00706172), BE(0x656e7400),
    };

    for (int i = 0; i < 54; i++)
        EXPECT_EQ(memory[i], expected[i]) << "word " << i;

    std::filesystem::remove("test.dtb");
}

TEST(fdt, decompile_memory) {
    const mwr::u32 dtb[54] = {
        BE(0xd00dfeed), BE(0x000000d8), BE(0x00000038), BE(0x000000b0),
        BE(0x00000028), BE(0x00000011), BE(0x00000010), BE(0x00000000),
        BE(0x00000028), BE(0x00000078), BE(0x00000000), BE(0x00000000),
        BE(0x00000000), BE(0x00000000), BE(0x00000001), BE(0x00000000),
        BE(0x00000001), BE(0x74657374), BE(0x00000000), BE(0x00000003),
        BE(0x00000004), BE(0x00000000), BE(0x00000123), BE(0x00000003),
        BE(0x00000008), BE(0x00000007), BE(0x00000456), BE(0x00000678),
        BE(0x00000003), BE(0x00000008), BE(0x0000000e), BE(0x666f6f00),
        BE(0x62617200), BE(0x00000003), BE(0x00000004), BE(0x00000019),
        BE(0x00000002), BE(0x00000003), BE(0x00000004), BE(0x00000021),
        BE(0x00000002), BE(0x00000002), BE(0x00000002), BE(0x00000009),
        BE(0x70726f70), BE(0x5f610070), BE(0x726f705f), BE(0x6200636f),
        BE(0x6d706174), BE(0x69626c65), BE(0x00706861), BE(0x6e646c65),
        BE(0x00706172), BE(0x656e7400),
    };

    auto root = mwr::fdtdecompile(dtb, sizeof(dtb));
    EXPECT_EQ(root.name(), "/");
    auto* test = root.find_child("test");
    ASSERT_NE(test, nullptr);
    EXPECT_EQ(test->name(), "test");
    auto prop_a = test->find_property("prop_a");
    ASSERT_NE(prop_a, nullptr);
    EXPECT_EQ(prop_a->name(), "prop_a");
    EXPECT_EQ(prop_a->numbers()[0], 0x123);
    auto prop_b = test->find_property("prop_b");
    ASSERT_NE(prop_b, nullptr);
    EXPECT_EQ(prop_b->name(), "prop_b");
    EXPECT_EQ(prop_b->numbers()[0], 0x456);
    EXPECT_EQ(prop_b->numbers()[1], 0x678);
    auto compatible = test->find_property("compatible");
    ASSERT_NE(compatible, nullptr);
    EXPECT_EQ(compatible->name(), "compatible");
    EXPECT_EQ(compatible->strings()[0], "foo");
    EXPECT_EQ(compatible->strings()[1], "bar");
    auto phandle = test->find_property("phandle");
    ASSERT_NE(phandle, nullptr);
    EXPECT_EQ(phandle->name(), "phandle");
    EXPECT_EQ(test->handle(), phandle->numbers()[0]);
}

TEST(fdt, decompile_file) {
    auto root = mwr::fdtdecompile(get_resource_path("test.dtb"));
    EXPECT_EQ(root.name(), "/");
    auto* node_a = root.find_child("node_a");
    ASSERT_NE(node_a, nullptr);
    auto* prop_a = node_a->find_property("prop_a");
    ASSERT_NE(prop_a, nullptr);
    EXPECT_EQ(prop_a->numbers()[0], 123);
    auto* prop_b = node_a->find_property("prop_b");
    ASSERT_NE(prop_b, nullptr);
    EXPECT_EQ(prop_b->numbers()[0], 0);
    EXPECT_EQ(prop_b->numbers()[1], 1);
    EXPECT_EQ(prop_b->numbers()[2], 2);
    EXPECT_EQ(prop_b->numbers()[3], 3);
    auto* phandle = node_a->find_property("phandle");
    ASSERT_NE(phandle, nullptr);
    EXPECT_EQ(node_a->handle(), phandle->numbers()[0]);
    auto* node_b = node_a->find_child("node_b");
    ASSERT_NE(node_b, nullptr);
    auto* parent = node_b->find_property("parent");
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->numbers()[0], node_a->handle());
    auto* prob_c = node_b->find_property("prob_c");
    ASSERT_NE(prob_c, nullptr);
    EXPECT_EQ(prob_c->strings()[0], "hello");
    EXPECT_EQ(prob_c->strings()[1], "world");
}
