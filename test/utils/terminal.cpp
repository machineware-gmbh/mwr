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

#include "mwr.h"

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

    mwr::tty_push(fd, false);

    EXPECT_FALSE(mwr::tty_is_vt100(fd));

    mwr::tty_setup_vt100(fd);

    EXPECT_TRUE(mwr::tty_is_vt100(fd));

    mwr::tty_pop(fd);

    EXPECT_FALSE(mwr::tty_is_vt100(fd));

    {
        mwr::tty_guard guard(fd);
        mwr::tty_setup_vt100(fd);
        EXPECT_TRUE(mwr::tty_is_vt100(fd));
    }

    EXPECT_FALSE(mwr::tty_is_vt100(fd));

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
