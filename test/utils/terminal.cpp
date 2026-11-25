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

#include "mwr.h"

#if defined(MWR_LINUX) || defined(MWR_MACOS)
#include <sys/ioctl.h>
#endif

TEST(utils, tty) {
    // Try to get a new pseudo terminal, if the operating system supports
    // that. Otherwise try STDIN (0), that should usually be a terminal as
    // well, unless it has been redirected to a file. If we can not get a
    // file descriptor for a terminal, just skip this test quietly.
    int fd = mwr::new_tty();
    if (fd < 0)
        fd = 0;
    if (!mwr::is_tty(fd))
        return;

    bool is_vt100 = mwr::tty_is_vt100(fd);

    mwr::tty_push(fd, false);

    EXPECT_EQ(mwr::tty_is_vt100(fd), is_vt100);

    mwr::tty_setup_vt100(fd);

    EXPECT_TRUE(mwr::tty_is_vt100(fd));

    mwr::tty_pop(fd);

    EXPECT_EQ(mwr::tty_is_vt100(fd), is_vt100);

    {
        mwr::tty_guard guard(fd);
        mwr::tty_setup_vt100(fd);
        EXPECT_TRUE(mwr::tty_is_vt100(fd));
    }

    EXPECT_EQ(mwr::tty_is_vt100(fd), is_vt100);

    if (fd != 0)
        mwr::fd_close(fd);
}

TEST(utils, tty_restore) {
    // Try to get a new pseudo terminal, if the operating system supports
    // that. If we can not get one, just skip this test quietly.
    int fd = mwr::new_tty();
    if (!mwr::is_tty(fd))
        return;

    mwr::tty_push(fd, true);
    mwr::tty_setup_vt100(fd);
    EXPECT_TRUE(mwr::tty_is_vt100(fd));
    // fd will be closed on exit
}

TEST(utils, tty_null_device) {
    // The null device is what a process without a console gets as stdin. On
    // Windows it is a character device like the console, but it is no tty.
#ifdef MWR_WINDOWS
    int fd = mwr::fd_open("NUL", "r");
#else
    int fd = mwr::fd_open("/dev/null", "r");
#endif
    ASSERT_GE(fd, 0);
    EXPECT_FALSE(mwr::is_tty(fd));
    mwr::fd_close(fd);
}

TEST(utils, tty_size) {
    size_t cols = 123;
    size_t rows = 456;

#ifdef MWR_WINDOWS
    int null = mwr::fd_open("NUL", "r");
#else
    int null = mwr::fd_open("/dev/null", "r");
#endif
    ASSERT_GE(null, 0);
    EXPECT_FALSE(mwr::tty_size(null, cols, rows));
    EXPECT_FALSE(mwr::tty_size(-1, cols, rows));
    EXPECT_EQ(cols, 123);
    EXPECT_EQ(rows, 456);
    mwr::fd_close(null);

#if defined(MWR_LINUX) || defined(MWR_MACOS)
    int fd = mwr::new_tty();
    if (!mwr::is_tty(fd))
        return;

    winsize size{};
    ASSERT_EQ(ioctl(fd, TIOCSWINSZ, &size), 0);
    EXPECT_FALSE(mwr::tty_size(fd, cols, rows));
    EXPECT_EQ(cols, 123);
    EXPECT_EQ(rows, 456);

    size.ws_col = 132;
    size.ws_row = 43;
    ASSERT_EQ(ioctl(fd, TIOCSWINSZ, &size), 0);
    EXPECT_TRUE(mwr::tty_size(fd, cols, rows));
    EXPECT_EQ(cols, 132);
    EXPECT_EQ(rows, 43);

    mwr::fd_close(fd);
#else
    int out = mwr::STDOUT_FDNO;
    if (mwr::is_tty(out)) {
        EXPECT_TRUE(mwr::tty_size(out, cols, rows));
        EXPECT_GT(cols, 0);
        EXPECT_GT(rows, 0);
    }
#endif
}

TEST(utils, termcolors) {
    EXPECT_GT(strlen(mwr::termcolors::CLEAR), 1);
    EXPECT_GT(strlen(mwr::termcolors::BLACK), 1);
    EXPECT_GT(strlen(mwr::termcolors::RED), 1);
    EXPECT_GT(strlen(mwr::termcolors::GREEN), 1);
    EXPECT_GT(strlen(mwr::termcolors::YELLOW), 1);
    EXPECT_GT(strlen(mwr::termcolors::BLUE), 1);
    EXPECT_GT(strlen(mwr::termcolors::MAGENTA), 1);
    EXPECT_GT(strlen(mwr::termcolors::CYAN), 1);
    EXPECT_GT(strlen(mwr::termcolors::WHITE), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_BLACK), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_RED), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_GREEN), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_YELLOW), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_BLUE), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_MAGENTA), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_CYAN), 1);
    EXPECT_GT(strlen(mwr::termcolors::BRIGHT_WHITE), 1);
}
