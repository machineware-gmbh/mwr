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

#include <stdlib.h>
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
    int fd = STDOUT_FDNO;
    EXPECT_EQ(fd_write(fd, str, len), len);
    EXPECT_EQ(fd_write(-fd, str, len), 0);
    EXPECT_EQ(fd_write(fd, nullptr, len), 0);
    EXPECT_EQ(fd_write(fd, str, 0), 0);
}

TEST(utils, fd_io) {
    int fd = fd_open("testfile", "w+");
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

TEST(utils, getenv) {
    ASSERT_FALSE(mwr::getenv("NONEXISTENT"));

    mwr::setenv("TESTVAR", "somevalue");
    auto var = mwr::getenv("TESTVAR");
    ASSERT_TRUE(var);
    EXPECT_EQ(var, "somevalue");
}

TEST(utils, getpid) {
    EXPECT_GT(mwr::getpid(), 0);
}

TEST(utils, page_size) {
    EXPECT_GE(mwr::get_page_size(), 4 * mwr::KiB);
}

TEST(utils, fill_random) {
    vector<char> buf1(100);
    vector<char> buf2(100);

    ASSERT_TRUE(mwr::fill_random(buf1.data(), buf1.size()));
    ASSERT_TRUE(mwr::fill_random(buf2.data(), buf2.size()));

    EXPECT_NE(buf1, buf2);
}
