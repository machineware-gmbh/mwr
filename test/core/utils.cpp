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
#include "mwr/core/utils.h"

#include <fstream>
#include <filesystem>

using namespace mwr;

TEST(utils, paths) {
    std::filesystem::create_directory("test");
    std::ofstream("test/file");
    std::filesystem::create_symlink("test", "test/test.link");
    std::filesystem::create_symlink("test/file", "test/file.link");
    std::filesystem::create_symlink("test/file.link", "test/file.link.link");

    EXPECT_TRUE(is_file("test/file"));
    EXPECT_TRUE(is_file("test/file.link"));
    EXPECT_TRUE(is_file("test/file.link.link"));

    EXPECT_TRUE(is_directory("test"));
    EXPECT_TRUE(is_directory("test/test.link"));

    EXPECT_FALSE(is_directory("test/file"));
    EXPECT_FALSE(is_directory("test/file.link"));
    EXPECT_FALSE(is_directory("test/file.link.link"));

    EXPECT_FALSE(is_file("test"));
    EXPECT_FALSE(is_file("test/test.link"));

    EXPECT_FALSE(is_directory("nothing"));
    EXPECT_FALSE(is_file("nothing"));

    EXPECT_TRUE(file_exists("test"));
    EXPECT_TRUE(file_exists("test/file"));
    EXPECT_FALSE(file_exists("nothing"));

    std::filesystem::remove_all("test");
}

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
