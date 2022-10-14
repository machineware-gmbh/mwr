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

#include <gtest/gtest.h>

#include "mwr/core/utils.h"

using namespace mwr;

TEST(utils, dirname) {
    EXPECT_EQ(dirname("/a/b/c.txt"), "/a/b");
    EXPECT_EQ(dirname("a/b/c.txt"), "a/b");
    EXPECT_EQ(dirname("/a/b/c/"), "/a/b/c");
    EXPECT_EQ(dirname("nothing"), ".");
}

TEST(utils, filename) {
    EXPECT_EQ(filename("/a/b/c.txt"), "c.txt");
    EXPECT_EQ(filename("a/b/c.txt"), "c.txt");
    EXPECT_EQ(filename("/a/b/c/"), "");
    EXPECT_EQ(filename("nothing"), "nothing");
}

TEST(utils, filename_noext) {
    EXPECT_EQ(filename_noext("/a/b/c.txt"), "c");
    EXPECT_EQ(filename_noext("a/b/c.c.txt"), "c.c");
    EXPECT_EQ(filename_noext("/a/b/c/"), "");
    EXPECT_EQ(filename_noext("nothing"), "nothing");
}

TEST(utils, curr_dir) {
    EXPECT_TRUE(curr_dir() != "");
}

namespace N {

template <typename T>
struct struct_a {
    struct struct_b {
        void func() {
            vector<string> bt = backtrace(1, 1);
            EXPECT_EQ(bt.size(), 1);
            // EXPECT_TRUE(str_begins_with(bt[0], "N::A<int>::B::func()"));
        }

        void func(T t) {
            vector<string> bt = backtrace(1, 1);
            EXPECT_EQ(bt.size(), 1);
            // EXPECT_TRUE(str_begins_with(bt[0], "N::A<char
            // const*>::B::func(char const*)"));
        }

        void func2() {
            vector<string> bt = backtrace(1, 1);
            EXPECT_EQ(bt.size(), 1);
            // EXPECT_TRUE(str_begins_with(bt[0], "N::A<N::A<std::map<int,
            // double, std::less<int>, std::allocator<std::pair<int const,
            // double> > > > >::B::func2()"));
        }
    };
};

struct struct_u {
    template <int N>
    void unroll(double d) {
        unroll<N - 1>(d);
    }
};

template <>
void struct_u::unroll<0>(double d) {
    vector<string> bt = backtrace(5, 1);
    EXPECT_EQ(bt.size(), 5);
    for (const std::string& func : bt)
        std::cout << func << std::endl;
}
} // namespace N

TEST(utils, backtrace) {
    N::struct_a<int>::struct_b().func();
    N::struct_a<const char*>::struct_b().func("42");
    N::struct_a<N::struct_a<std::map<int, double> > >::struct_b().func2();
    N::struct_u().unroll<5>(42.0);
}
