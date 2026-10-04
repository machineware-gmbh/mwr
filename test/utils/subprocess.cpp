/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"
#include "mwr.h"

#ifdef MWR_WINDOWS
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#endif

// a process that keeps running until it is terminated
static void run_forever(mwr::subprocess& proc) {
#ifdef MWR_WINDOWS
    ASSERT_TRUE(proc.run("cmd.exe", { "/K" }));
#else
    ASSERT_TRUE(proc.run("/bin/cat"));
#endif
}

static constexpr int WAIT_TIMEOUT_MS = 10000;

static bool wait_for_output(mwr::subprocess& proc, const std::string& expected,
                            std::string* output = nullptr) {
    std::string buf;
    bool found = false;
    for (int i = 0; i < WAIT_TIMEOUT_MS / 10 && !found; i++) {
        buf += proc.peek();
        found = buf.find(expected) != std::string::npos;
        if (!found)
            mwr::usleep(10000);
    }

    if (output)
        *output = buf;
    return found;
}

static bool wait_for_exit(mwr::subprocess& proc) {
    for (int i = 0; i < WAIT_TIMEOUT_MS / 10 && proc.is_running(); i++)
        mwr::usleep(10000);
    return !proc.is_running();
}

static size_t open_handles() {
#ifdef MWR_WINDOWS
    DWORD count = 0;
    GetProcessHandleCount(GetCurrentProcess(), &count);
    return count;
#else
    size_t count = 0;
    for (int fd = 0; fd < 4096; fd++) {
        if (fcntl(fd, F_GETFD) != -1)
            count++;
    }
    return count;
#endif
}

TEST(process, environment) {
#ifdef MWR_WINDOWS
    // On Windows, use cmd.exe /C set to list environment variables.
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/C", "set" };
#else
    // On POSIX, use /usr/bin/env to list environment variables.
    const std::string exec = "/usr/bin/env";
    const std::vector<std::string> args = {};
#endif

    mwr::subprocess proc;
    proc.env["MYVAR"] = "HelloWorld";
    ASSERT_TRUE(proc.run(exec, args));

    std::string output;
    EXPECT_TRUE(wait_for_output(proc, "MYVAR=HelloWorld", &output)) << output;
}

TEST(process, read_stdout) {
#ifdef MWR_WINDOWS
    // On Windows, use cmd.exe /C to run a one-shot echo command.
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/C", "echo Hello World" };
#else
    // On POSIX, use /bin/echo for a one-shot echo command.
    const std::string exec = "/bin/echo";
    const std::vector<std::string> args = { "Hello", "World" };
#endif

    mwr::subprocess proc;
    ASSERT_TRUE(proc.run(exec, args));

    char buf[13] = { 0 };
    ASSERT_TRUE(proc.read(buf, 12));
    EXPECT_STREQ(buf, "Hello World\n");
}

TEST(process, peek_stdout) {
#ifdef MWR_WINDOWS
    // On Windows, use cmd.exe /C to run a one-shot echo command.
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/C", "echo Hello World" };
#else
    // On POSIX, use /bin/echo for a one-shot echo command.
    const std::string exec = "/bin/echo";
    const std::vector<std::string> args = { "Hello", "World" };
#endif

    mwr::subprocess proc;
    ASSERT_TRUE(proc.run(exec, args));

    // test if the output was printed
    std::string output;
    EXPECT_TRUE(wait_for_output(proc, "Hello World\n", &output));
    EXPECT_EQ(output, "Hello World\n");
}

TEST(process, write_stdin) {
#ifdef MWR_WINDOWS
    // On Windows, launch an interactive cmd shell.
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/K" };
#else
    // On POSIX, use "cat" which echoes input back.
    const std::string exec = "/bin/cat";
    const std::vector<std::string> args = {};
#endif

    mwr::subprocess proc;
    ASSERT_TRUE(proc.run(exec, args));
    std::string input = "Hello World\n";
    EXPECT_TRUE(proc.write(input));
    std::string output;
    EXPECT_TRUE(wait_for_output(proc, input, &output)) << output;
}

TEST(process, is_running) {
#ifdef MWR_WINDOWS
    // On Windows, launch an interactive cmd shell that stays alive.
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/K" };
#else
    // On POSIX, use "cat" which blocks waiting for input.
    const std::string exec = "/bin/cat";
    const std::vector<std::string> args = {};
#endif

    mwr::subprocess proc;
    EXPECT_FALSE(proc.is_running());

    ASSERT_TRUE(proc.run(exec, args));
    EXPECT_TRUE(proc.is_running());

    proc.terminate();
    EXPECT_FALSE(proc.is_running());
}

TEST(process, cwd_pwd) {
#ifdef MWR_WINDOWS
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/C", "cd" };
    const std::string dir = "C:\\Windows";
    const std::string expected = "C:\\Windows";
#else
    const std::string exec = "/bin/pwd";
    const std::vector<std::string> args = {};
    const std::string dir = "/tmp";
    const std::string expected = "/tmp";
#endif

    mwr::subprocess proc;
    proc.cwd = dir;
    ASSERT_TRUE(proc.run(exec, args));

    std::string output;
    EXPECT_TRUE(wait_for_output(proc, expected, &output))
        << "output: " << output;
}

TEST(process, cwd_invalid) {
#ifdef MWR_WINDOWS
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/C", "echo hi" };
#else
    const std::string exec = "/bin/echo";
    const std::vector<std::string> args = { "hi" };
#endif

    mwr::subprocess proc;
    proc.cwd = "/this/path/does/not/exist_xyz_42";

    proc.run(exec, args);
    EXPECT_TRUE(wait_for_exit(proc));
}

TEST(process, terminate_waits) {
    mwr::subprocess proc;
    run_forever(proc);
    ASSERT_TRUE(proc.is_running());

    // the process must be gone when terminate returns, so that it no longer
    // holds any resources, e.g. listening sockets
#ifdef MWR_WINDOWS
    HANDLE handle = NULL;
    ASSERT_TRUE(
        DuplicateHandle(GetCurrentProcess(), (HANDLE)proc.native_handle(),
                        GetCurrentProcess(), &handle, SYNCHRONIZE, FALSE, 0));
    EXPECT_TRUE(proc.terminate());
    EXPECT_EQ(WaitForSingleObject(handle, 0), WAIT_OBJECT_0);
    CloseHandle(handle);
#else
    int pid = proc.pid();
    EXPECT_TRUE(proc.terminate());
    errno = 0;
    EXPECT_EQ(kill(pid, 0), -1);
    EXPECT_EQ(errno, ESRCH);
#endif

    EXPECT_FALSE(proc.is_running());
    EXPECT_FALSE(proc.terminate()); // nothing left to terminate
}

TEST(process, terminate_exited) {
#ifdef MWR_WINDOWS
    const std::string exec = "cmd.exe";
    const std::vector<std::string> args = { "/C", "exit" };
#else
    const std::string exec = "/bin/true";
    const std::vector<std::string> args = {};
#endif

    mwr::subprocess proc;
    ASSERT_TRUE(proc.run(exec, args));
    ASSERT_TRUE(wait_for_exit(proc));

    // a process that has exited on its own is still cleaned up
    EXPECT_TRUE(proc.terminate());
    EXPECT_FALSE(proc.terminate());
}

TEST(process, terminate_releases_handles) {
    size_t before = open_handles();

    for (int i = 0; i < 3; i++) {
        mwr::subprocess proc;
        run_forever(proc);
        EXPECT_TRUE(proc.terminate());
    }

    EXPECT_EQ(open_handles(), before);
}
