/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
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
