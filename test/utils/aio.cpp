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

TEST(aio, callback) {
    const char msg = 'X';

    int fds[2];
    EXPECT_EQ(pipe(fds), 0);

    std::mutex mtx;
    mtx.lock();
    std::condition_variable_any cv;
    std::atomic<int> count(0);

    mwr::aio_notify(fds[0], [&](int fd) -> void {
        char buf;
        EXPECT_EQ(fd, fds[0]) << "wrong file descriptor passed to handler";
        EXPECT_EQ(read(fd, &buf, 1), 1) << "cannot read file descriptor";
        EXPECT_EQ(buf, msg) << "read incorrect data";

        count++;
        cv.notify_all();
    });

    EXPECT_EQ(write(fds[1], &msg, 1), 1);

    cv.wait(mtx);
    ASSERT_EQ(count, 1) << "handler called multiple times, should be once";

    mwr::aio_cancel(fds[0]);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_EQ(count, 1) << "handler after being cancelled";

    close(fds[0]);
    close(fds[1]);
}
