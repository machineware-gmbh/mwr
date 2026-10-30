/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
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

TEST(compiler, version) {
#if defined(MWR_GCC)
    EXPECT_GT(MWR_GCC_VERSION, 0);
    EXPECT_EQ(MWR_CLANG_VERSION, 0);
    EXPECT_EQ(MWR_MSVC_VERSION, 0);
#elif defined(MWR_CLANG)
    EXPECT_EQ(MWR_GCC_VERSION, 0);
    EXPECT_GT(MWR_CLANG_VERSION, 0);
    EXPECT_EQ(MWR_MSVC_VERSION, 0);
#elif defined(MWR_MSVC)
    EXPECT_EQ(MWR_GCC_VERSION, 0);
    EXPECT_EQ(MWR_CLANG_VERSION, 0);
    EXPECT_GT(MWR_MSVC_VERSION, 0);
#else
    GTEST_FAIL() << "compiler undefined";
#endif
}

MWR_DECL_INLINE int add_inline(int a, int b) {
    return a + b;
}

TEST(compiler, decl_inline) {
    EXPECT_EQ(add_inline(2, 3), 5);
}

MWR_DECL_MULTIVERSION("default", "avx2")
static int dot(const int* a, const int* b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i] * b[i];
    return sum;
}

TEST(compiler, multiversion) {
    int a[64], b[64], expect = 0;
    for (int i = 0; i < 64; i++) {
        a[i] = i - 32;
        b[i] = 3 * i + 1;
        expect += a[i] * b[i];
    }

    EXPECT_EQ(dot(a, b, 64), expect);
}

MWR_DECL_VECTORIZE
static int dot_vectorized(const int* a, const int* b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i] * b[i];
    return sum;
}

TEST(compiler, vectorize) {
    int a[64], b[64], expect = 0;
    for (int i = 0; i < 64; i++) {
        a[i] = i - 32;
        b[i] = 3 * i + 1;
        expect += a[i] * b[i];
    }

    EXPECT_EQ(dot_vectorized(a, b, 64), expect);
}

TEST(utils, gcov_enabled) {
#ifdef MWR_COVERAGE
    EXPECT_TRUE(mwr::gcov_enabled());
#else
    EXPECT_FALSE(mwr::gcov_enabled());
#endif
}

TEST(utils, host_endian) {
#if !defined(MWR_HOST_LITTLE_ENDIAN) && !defined(MWR_HOST_BIG_ENDIAN)
    GTEST_FAIL() << "host endian undefined";
#endif
}
