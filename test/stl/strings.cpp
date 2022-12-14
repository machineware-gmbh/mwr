/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
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
    string s = "abc def\nghi\tjkl :.; ";
    vector<string> v = split(s, [](unsigned char c) { return isspace(c); });

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.at(0), "abc");
    EXPECT_EQ(v.at(1), "def");
    EXPECT_EQ(v.at(2), "ghi");
    EXPECT_EQ(v.at(3), "jkl");
    EXPECT_EQ(v.at(4), ":.;");
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
