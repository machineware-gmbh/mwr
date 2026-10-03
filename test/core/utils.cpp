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

#ifndef MWR_MINGW
    // mingw does not support create_symlink
    std::filesystem::create_symlink("../temp", "temp/temp.link");
    std::filesystem::create_symlink("file", "temp/file.link");
    std::filesystem::create_symlink("file.link", "temp/file.link.link");
    std::filesystem::create_symlink("../file", "temp/dir/file.link");
    std::filesystem::create_symlink("../file.link", "temp/dir/file.link.link");
    std::filesystem::create_symlink("temp/file", "temp/dir/wrong.link");
#endif

    EXPECT_TRUE(directory_exists("temp"));
    EXPECT_TRUE(file_exists("temp/file"));

#ifndef MWR_MINGW
    EXPECT_TRUE(file_exists("temp/file.link"));
    EXPECT_TRUE(file_exists("temp/file.link.link"));
    EXPECT_TRUE(file_exists("temp/dir/file.link"));
    EXPECT_TRUE(file_exists("temp/dir/file.link.link"));
    EXPECT_FALSE(file_exists("temp/dir/wrong.link"));
    EXPECT_TRUE(directory_exists("temp/temp.link"));
#endif

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

    mwr::setenv("TESTVAR2", "123");
    int x = mwr::getenv_or_default("TESTVAR2", 456);
    EXPECT_EQ(x, 123);
    int y = mwr::getenv_or_default("TESTVAR3", 456);
    EXPECT_EQ(y, 456);
}

TEST(utils, clrenv) {
    mwr::setenv("CLRENV", "abc");
    EXPECT_EQ(mwr::getenv("CLRENV"), "abc");
    mwr::clrenv("CLRENV");
    EXPECT_EQ(mwr::getenv("CLRENV"), std::nullopt);
}

TEST(utils, environment) {
    mwr::setenv("FINDME", "findme");

    bool found = false;
    for (auto& [name, val] : get_environment()) {
        if (name == "FINDME" && val == "findme") {
            found = true;
            break;
        }
    }

    EXPECT_TRUE(found);
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

#ifndef MWR_WINDOWS
#include <unistd.h>
#include <sys/resource.h>

TEST(utils, fd_peek) {
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);

    EXPECT_EQ(fd_peek(-1), 0);
    EXPECT_EQ(fd_peek(fds[0]), 0);

    auto t0 = std::chrono::steady_clock::now();
    EXPECT_EQ(fd_peek(fds[0], 50), 0);
    EXPECT_GE(std::chrono::steady_clock::now() - t0,
              std::chrono::milliseconds(40));

    // reports the number of available bytes
    ASSERT_EQ(write(fds[1], "hello", 5), 5);
    EXPECT_EQ(fd_peek(fds[0]), 5);
    char buf[8];
    ASSERT_EQ(read(fds[0], buf, 2), 2);
    EXPECT_EQ(fd_peek(fds[0]), 3);

    // works with fds beyond FD_SETSIZE, where select would fail
    struct rlimit lim;
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &lim), 0);
    int high = FD_SETSIZE + 100;
    if (lim.rlim_cur <= (rlim_t)high && lim.rlim_max > (rlim_t)high) {
        lim.rlim_cur = high + 1;
        setrlimit(RLIMIT_NOFILE, &lim);
    }

    if (dup2(fds[0], high) == high) {
        EXPECT_EQ(fd_peek(high), 3);
        close(high);
    }

    // end of file still reports one byte, so that read can return it
    close(fds[1]);
    ASSERT_EQ(read(fds[0], buf, 3), 3);
    EXPECT_EQ(fd_peek(fds[0]), 1);
    EXPECT_EQ(read(fds[0], buf, 1), 0);
    close(fds[0]);
}
#endif
