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

TEST(utils, info) {
    EXPECT_NE(curr_dir(), "");
    EXPECT_NE(temp_dir(), "");
    EXPECT_NE(progname(), "");
    EXPECT_NE(username(), "");
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
    int fd = 1; // STDOUT_FILENO
    EXPECT_EQ(fd_write(fd, str, len), len);
    EXPECT_EQ(fd_write(-fd, str, len), 0);
    EXPECT_EQ(fd_write(fd, nullptr, len), 0);
    EXPECT_EQ(fd_write(fd, str, 0), 0);
}

TEST(utils, fd_io) {
    int fd = fd_open("testfile", FD_CREATE | FD_TRUNCATE | FD_RDWR);
    ASSERT_GE(fd, 0);

    const char* text = "hello world";
    size_t n = strlen(text);
    ASSERT_EQ(fd_write(fd, text, n), n);

    ASSERT_EQ(mwr::fd_seek(fd, 0), 0);

    char buffer[20] = {};
    ASSERT_EQ(fd_read(fd, buffer, n), n);
    EXPECT_STREQ(text, buffer);

    fd_close(fd);
    std::filesystem::remove_all("testfile");
}
