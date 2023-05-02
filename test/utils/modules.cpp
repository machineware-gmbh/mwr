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
#include "mwr/utils/modules.h"
#include "mwr/core/version.h"

#define TEST_VERSION        1234
#define TEST_VERSION_STRING "1.2.34"
#define TEST_VERSION_MAJOR  1
#define TEST_VERSION_MINOR  2
#define TEST_VERSION_PATCH  34
#define TEST_GIT_REV        "abcdef0123456"
#define TEST_GIT_REV_SHORT  "abcdef"

MWR_DECLARE_MODULE(TEST, "test", "Apache-2.0")

TEST(modules, declare) {
    const mwr::module* test = mwr::modules::find("test");
    ASSERT_NE(test, nullptr);
    EXPECT_EQ(test->name, "test");
    EXPECT_EQ(test->version, TEST_VERSION);
    EXPECT_EQ(test->version_major, TEST_VERSION_MAJOR);
    EXPECT_EQ(test->version_minor, TEST_VERSION_MINOR);
    EXPECT_EQ(test->version_patch, TEST_VERSION_PATCH);
    EXPECT_EQ(test->version_string, TEST_VERSION_STRING);
    EXPECT_EQ(test->git_rev, TEST_GIT_REV);
    EXPECT_EQ(test->git_rev_short, TEST_GIT_REV_SHORT);
    EXPECT_EQ(test->license, "Apache-2.0");
}

TEST(modules, list) {
    for (const auto* module : mwr::modules::all())
        std::cout << *module << std::endl;
    EXPECT_EQ(mwr::modules::count(), 2);
}

TEST(modules, version) {
    mwr::modules::print_versions(std::cout);
}

TEST(modules, license) {
    mwr::modules::print_licenses(std::cout);
}
