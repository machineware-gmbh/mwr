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

#include <fcntl.h>
#include <unistd.h>

TEST(utils, tty) {
    int fd = mwr::new_tty();
    ASSERT_TRUE(mwr::is_tty(fd));
    mwr::tty_push(fd, false);

    EXPECT_TRUE(mwr::tty_is_echo(fd));
    EXPECT_TRUE(mwr::tty_is_isig(fd));

    mwr::tty_set(fd, false, false);

    EXPECT_FALSE(mwr::tty_is_echo(fd));
    EXPECT_FALSE(mwr::tty_is_isig(fd));

    mwr::tty_pop(fd);

    EXPECT_TRUE(mwr::tty_is_echo(fd));
    EXPECT_TRUE(mwr::tty_is_isig(fd));

    {
        mwr::tty_guard guard(fd, false, false);
        EXPECT_FALSE(mwr::tty_is_echo(fd));
        EXPECT_FALSE(mwr::tty_is_isig(fd));
    }

    EXPECT_TRUE(mwr::tty_is_echo(fd));
    EXPECT_TRUE(mwr::tty_is_isig(fd));

    close(fd);
}

TEST(utils, tty_restore) {
    int fd = mwr::new_tty();
    ASSERT_TRUE(mwr::is_tty(fd));
    mwr::tty_push(fd, true);
    mwr::tty_set(fd, false, false);
    EXPECT_FALSE(mwr::tty_is_echo(fd));
    EXPECT_FALSE(mwr::tty_is_isig(fd));
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
