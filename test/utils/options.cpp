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
#include "mwr/utils/options.h"
#include "mwr/utils/modules.h"

using namespace mwr;

TEST(options, opt_bool) {
    const char* argv[] = { "-a", "-b" };

    option<bool> a("-a", "option a");
    option<bool> b("-b", "option b");
    option<bool> c("-c", "option c");

    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(c.has_value());

    EXPECT_TRUE(options::parse(2, argv));

    EXPECT_TRUE(a);
    EXPECT_TRUE(b);
    EXPECT_FALSE(c);
}

TEST(options, opt_int) {
    const char* argv[] = { "-a", "1", "-b", "2" };

    option<int> a("-a", "option a");
    option<int> b("-b", "option b");
    option<int> c("-c", "option c");

    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(c.has_value());

    EXPECT_TRUE(options::parse(4, argv));

    EXPECT_GT(a, 0);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 2);
    EXPECT_LT(b, 3);
    EXPECT_FALSE(c.has_value());
}

TEST(options, opt_string) {
    const char* argv[] = { "-a", "one", "-b", "two" };

    option<string> a("-a", "option a");
    option<string> b("-b", "option b");
    option<string> c("-c", "option c");

    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(c.has_value());

    EXPECT_TRUE(options::parse(4, argv));

    EXPECT_EQ(a, "one");
    EXPECT_EQ(b, "two");
    EXPECT_FALSE(c.has_value());
}

TEST(options, multistring) {
    const char* argv[] = { "-s", "one", "-s", "two" };

    option<string> s("-s", "option s");

    EXPECT_FALSE(s.has_value());

    EXPECT_TRUE(options::parse(4, argv));

    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(s.values(0), "one");
    EXPECT_EQ(s.values(1), "two");
}

TEST(options, callbacks) {
    const char* argv[] = { "-f", "-f", "-f", "-f" };

    size_t count = 0;
    option<bool> f("-f", "option f", [&]() -> bool {
        count++;
        return true;
    });

    ASSERT_TRUE(options::parse(4, argv));
    EXPECT_EQ(count, 4);
    EXPECT_FALSE(f.has_value());
}

TEST(options, callbacks_once) {
    const char* argv[] = { "-s", "a", "-s", "b" };

    size_t count = 0;
    option<string> s("-s", "option s", [&](const string& s) -> bool {
        EXPECT_EQ(s, "a");
        count++;
        return false;
    });

    ASSERT_TRUE(options::parse(4, argv));
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(s.has_value());
}

TEST(options, fail) {
    const char* argv[] = { "-x", "-a", "one", "-b", "2", "-c" };

    option<string> a("-a", "option a");
    option<int> b("-b", "option b");
    option<bool> c("-c", "option c");
    option<string> x("-x", "option x");

    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(x.has_value());

    EXPECT_FALSE(options::parse(6, argv));

    EXPECT_EQ(a, "one");
    EXPECT_EQ(b, 2);
    EXPECT_TRUE(c);
    EXPECT_FALSE(x.has_value());
}

TEST(options, extra) {
    const char* argv[] = { "name", "-a", "string", "extra1", "-b", "extra2" };

    option<string> x("-a", "option a");
    option<bool> b("-b", "option b");

    vector<string> extra;
    ASSERT_TRUE(options::parse(6, argv, extra));
    ASSERT_EQ(extra.size(), 2);
    EXPECT_EQ(extra[0], "extra1");
    EXPECT_EQ(extra[1], "extra2");
}

TEST(options, help) {
    option<string> a("-a", "--long-a", "This is the description for option a");
    option<int> b("-b", "Option b description text");
    option<bool> c("-c", "--long-c", "This is how to use option c");
    option<string> x("-x", "--long-x", "Option x is also available");

    options::print_help(std::cout);
}
