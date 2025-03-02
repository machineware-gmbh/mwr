/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"
#include "mwr.h"

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

    // give the process 100ms to start
    mwr::usleep(100000);

    // test if our environment variable was printed
    std::string output = proc.peek();
    EXPECT_NE(output.find("MYVAR=HelloWorld"), std::string::npos);
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

    // give the process 100ms to start
    mwr::usleep(100000);

    // test if the output was printed
    std::string output = proc.peek();
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
    mwr::usleep(100000); // 100 ms
    std::string output = proc.peek();
    EXPECT_NE(output.find(input), std::string::npos);
}
