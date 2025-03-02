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
    std::string output = proc.read();
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

    // give the process 100ms to start
    mwr::usleep(100000);

    // test if the output was printed
    std::string output = proc.read();
    EXPECT_EQ(output, "Hello World\n");
}

TEST(process, write_stdin) {
#ifdef MWR_WINDOWS
    // TODO
#else
    // On POSIX, use "cat" which echoes input back.
    mwr::subprocess proc;
    ASSERT_TRUE(proc.run("/bin/cat"));
#endif
    std::string input = "Hello World\n";
    EXPECT_TRUE(proc.write(input));
    mwr::usleep(100000); // 100 ms
    std::string output = proc.read();
    EXPECT_EQ(output, input);
}

TEST(process, interrupt) {
#ifdef MWR_WINDOWS
    std::string exec = mwr::dirname(args[0]) + "/sigint_helper.exe";
#else
    std::string exec = mwr::dirname(args[0]) + "/sigint_helper";
#endif

    mwr::subprocess proc;
    proc.run(exec);
    mwr::usleep(100000);

    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(proc.interrupt());
        mwr::usleep(100000);
        std::string output = proc.read();
        EXPECT_EQ(output, "SIGINT received");
    }
}
