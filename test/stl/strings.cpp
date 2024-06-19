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
#include "mwr/stl/strings.h"

using namespace mwr;

TEST(strings, mkstr) {
    EXPECT_EQ(mkstr("%d %s", 42, "fortytwo"), "42 fortytwo");
    EXPECT_EQ(mkstr("%.9f", 1.987654321), "1.987654321");
}

TEST(strings, split) {
    string s = "abc def\nghi\tjkl :\\ ; ";
    vector<string> v = split(s, [](unsigned char c) { return isspace(c); });

    ASSERT_EQ(v.size(), 5);
    EXPECT_EQ(v.at(0), "abc");
    EXPECT_EQ(v.at(1), "def");
    EXPECT_EQ(v.at(2), "ghi");
    EXPECT_EQ(v.at(3), "jkl");
    EXPECT_EQ(v.at(4), ": ;");

    vector<string> w = split("hello!world!", '!');
    ASSERT_EQ(w.size(), 2);
    EXPECT_EQ(w.at(0), "hello");
    EXPECT_EQ(w.at(1), "world");

    vector<string> u = split("\"a b\" 'c  d' \"e\\\"f", ' ');
    ASSERT_EQ(u.size(), 3);
    EXPECT_EQ(u.at(0), "a b");
    EXPECT_EQ(u.at(1), "c  d");
    EXPECT_EQ(u.at(2), "e\"f");

    vector<string> t = split("\"\\\"unterminated\\\"", ' ');
    ASSERT_EQ(t.size(), 1);
    EXPECT_EQ(t.at(0), "\"unterminated\"");
}

TEST(strings, join) {
    vector<string> v0 = {};
    vector<string> v1 = { "a" };
    vector<string> v3 = { "a", "b", "c" };

    EXPECT_EQ(join(v0, ", "), "");
    EXPECT_EQ(join(v1, ", "), "a");
    EXPECT_EQ(join(v3, ", "), "a, b, c");
}

TEST(strings, upper_lower) {
    EXPECT_EQ(to_upper("true"), "TRUE");
    EXPECT_EQ(to_upper("TRUE"), "TRUE");
    EXPECT_EQ(to_lower("true"), "true");
    EXPECT_EQ(to_lower("TRUE"), "true");
}

TEST(strings, trim) {
    EXPECT_EQ(ltrim("\ntest0? \t"), "test0? \t");
    EXPECT_EQ(rtrim("\ntest0? \t"), "\ntest0?");
    EXPECT_EQ(trim("\ntest0? \t"), "test0?");
}

TEST(strings, from_string) {
    EXPECT_EQ(from_string<u64>("0xF"), 0xf);
    EXPECT_EQ(from_string<u64>("0x0000000b"), 0xb);
    EXPECT_EQ(from_string<i32>("10"), 10);
    EXPECT_EQ(from_string<i32>("-10"), -10);
    EXPECT_EQ(from_string<u64>("010"), 8);

    EXPECT_TRUE(from_string<bool>("true"));
    EXPECT_TRUE(from_string<bool>("True"));
    EXPECT_TRUE(from_string<bool>("1"));
    EXPECT_TRUE(from_string<bool>("0x1234"));
    EXPECT_FALSE(from_string<bool>("false"));
    EXPECT_FALSE(from_string<bool>("False"));
    EXPECT_FALSE(from_string<bool>("0"));
    EXPECT_FALSE(from_string<bool>("0x0"));
}

TEST(strings, replace) {
    string s = "replace this";
    EXPECT_EQ(replace(s, "this", "done"), 1);
    EXPECT_EQ(s, "replace done");

    string s2 = "$dir/file.txt";
    EXPECT_EQ(replace(s2, "$dir", "/home/user"), 1);
    EXPECT_EQ(s2, "/home/user/file.txt");
}

TEST(strings, contains) {
    string s = "hello world";

    EXPECT_TRUE(contains(s, "hello"));
    EXPECT_TRUE(contains(s, "o wor"));
    EXPECT_FALSE(contains(s, "wrold"));

    EXPECT_TRUE(starts_with(s, "hell"));
    EXPECT_FALSE(starts_with(s, "world"));

    EXPECT_TRUE(ends_with(s, "world"));
    EXPECT_FALSE(ends_with(s, "hello"));
}

TEST(strings, strcat) {
    EXPECT_EQ(strcat("abc", "def"), "abcdef");
    EXPECT_EQ(strcat(123, "-", 456), "123-456");
    EXPECT_EQ(strcat(true, false, true, 85), "truefalsetrue85");
}

TEST(strings, escape) {
    string s = escape("hello world!", " !");
    ASSERT_EQ(s, "hello\\ world\\!");
    EXPECT_EQ(unescape(s, " !"), "hello world!");

    string s2 = escape("C:\\a\\b\\c");
    ASSERT_EQ(s2, "C:\\\\a\\\\b\\\\c");
    EXPECT_EQ(unescape(s2), "C:\\a\\b\\c");
}
