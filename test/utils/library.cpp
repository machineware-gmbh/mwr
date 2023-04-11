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

#include "mwr/utils/library.h"

using namespace mwr;

static string get_test_library() {
#if defined(__x86_64__)
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
    if (global) // check to silence clang-tidy
        EXPECT_EQ(*global, 42);

    int (*function)(int) = nullptr;
    EXPECT_NO_THROW(lib.get(function, "function"));
    ASSERT_TRUE(function);
    if (function && global) // check to silence clang-tidy
        EXPECT_EQ(function(1), *global + 1);

    EXPECT_THROW(lib.get(global, "notfound"), mwr::report);
    EXPECT_THROW(library lib2("notfound.so"), mwr::report);
}
