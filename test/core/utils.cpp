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
    ASSERT_FALSE(std::filesystem::is_directory("temp"));

    std::filesystem::create_directory("temp");
    std::filesystem::create_directory("temp/dir");
    std::ofstream("temp/file");
    std::filesystem::create_symlink("../temp", "temp/temp.link");
    std::filesystem::create_symlink("file", "temp/file.link");
    std::filesystem::create_symlink("file.link", "temp/file.link.link");
    std::filesystem::create_symlink("../file", "temp/dir/file.link");
    std::filesystem::create_symlink("../file.link", "temp/dir/file.link.link");
    std::filesystem::create_symlink("temp/file", "temp/dir/wrong.link");

    EXPECT_TRUE(file_exists("temp/file"));
    EXPECT_TRUE(file_exists("temp/file.link"));
    EXPECT_TRUE(file_exists("temp/file.link.link"));
    EXPECT_TRUE(file_exists("temp/dir/file.link"));
    EXPECT_TRUE(file_exists("temp/dir/file.link.link"));
    EXPECT_FALSE(file_exists("temp/dir/wrong.link"));

    EXPECT_TRUE(directory_exists("temp"));
    EXPECT_TRUE(directory_exists("temp/temp.link"));

    EXPECT_FALSE(directory_exists("temp/file"));
    EXPECT_FALSE(directory_exists("temp/file.link"));
    EXPECT_FALSE(directory_exists("temp/file.link.link"));

    EXPECT_FALSE(file_exists("temp"));
    EXPECT_FALSE(file_exists("temp/temp.link"));

    EXPECT_FALSE(directory_exists("nothing"));
    EXPECT_FALSE(file_exists("nothing"));

    std::filesystem::remove_all("temp");
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

TEST(utils, timestamp) {
    double ts = timestamp();
    u64 ts_us = timestamp_us();
    u64 ts_ns = timestamp_ns();

    EXPECT_GT(ts, 0.0);
    EXPECT_GT(ts_us, 0u);
    EXPECT_GT(ts_ns, 0u);
    EXPECT_GT(ts_ns, ts_us);
}

TEST(utils, fd_write) {
    const char* str = "hello world!\n";
    size_t len = strlen(str);
    EXPECT_EQ(fd_write(STDOUT_FILENO, str, len), len);
    EXPECT_EQ(fd_write(-STDOUT_FILENO, str, len), 0);
    EXPECT_EQ(fd_write(STDOUT_FILENO, nullptr, len), 0);
    EXPECT_EQ(fd_write(STDOUT_FILENO, str, 0), 0);
}
