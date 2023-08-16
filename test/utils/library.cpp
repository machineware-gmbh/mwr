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

#include "mwr/utils/library.h"

using namespace mwr;

static string get_test_library() {
#if defined(_MSC_VER)
    return get_resource_path("shared-x86.dll");
#elif defined(__x86_64__)
    return get_resource_path("shared-x86.so");
#elif defined(__aarch64__)
    return get_resource_path("shared-arm64.so");
#else
#error "no test library available for your host architecture"
#endif
}

TEST(library, basic) {
    string path = get_test_library();
    library lib;

    ASSERT_NO_THROW(lib.open(path));
    ASSERT_TRUE(lib.is_open());
    EXPECT_EQ(lib.path(), path);
    EXPECT_TRUE(lib.has("global"));
    EXPECT_TRUE(lib.has("function"));
    EXPECT_FALSE(lib.has("notfound"));

    int* global = nullptr;
    EXPECT_NO_THROW(lib.get(global, "global"));
    ASSERT_TRUE(global);
    if (global) { // check to silence clang-tidy
        EXPECT_EQ(*global, 42);
    }

    int (*function)(int) = nullptr;
    EXPECT_NO_THROW(lib.get(function, "function"));
    ASSERT_TRUE(function);
    if (function && global) { // check to silence clang-tidy
        EXPECT_EQ(function(1), *global + 1);
    }

    EXPECT_THROW(lib.get(global, "notfound"), mwr::report);
    EXPECT_THROW(library lib2("notfound.so"), mwr::report);
}

TEST(libary, reopen) {
    library a, b;
    string path = get_test_library();

    ASSERT_NO_THROW(a.open(path));
    ASSERT_NO_THROW(b.open(path));
    EXPECT_STREQ(a.path(), b.path());

    ASSERT_TRUE(a.has("global"));
    ASSERT_TRUE(b.has("global"));

    // test both libraries reference the same global
    int* a_global = nullptr;
    int* b_global = nullptr;
    EXPECT_NO_THROW(a.get(a_global, "global"));
    EXPECT_NO_THROW(b.get(b_global, "global"));
    ASSERT_TRUE(a_global);
    ASSERT_TRUE(b_global);
    EXPECT_EQ(a_global, b_global);
}

TEST(libary, mopen) {
    library a, b;
    string path = get_test_library();

    ASSERT_NO_THROW(a.mopen(path));
    ASSERT_NO_THROW(b.mopen(path));
    EXPECT_STREQ(a.path(), b.path());

    ASSERT_TRUE(a.has("global"));
    ASSERT_TRUE(b.has("global"));

    // test both globals got their separate location in memory
    int* a_global = nullptr;
    int* b_global = nullptr;
    EXPECT_NO_THROW(a.get(a_global, "global"));
    EXPECT_NO_THROW(b.get(b_global, "global"));
    ASSERT_TRUE(a_global);
    ASSERT_TRUE(b_global);
    EXPECT_NE(a_global, b_global);
}
