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

#include <gtest/gtest.h>

#include "mwr/stl/containers.h"

using namespace mwr;

TEST(container, contains) {
    vector<int> v1{ 1, 2, 3, 4 };
    EXPECT_TRUE(stl_contains(v1, 3));
    EXPECT_TRUE(stl_contains(v1, 4));
    EXPECT_FALSE(stl_contains(v1, 5));

    set<int> s1{ 1, 2, 3, 4 };
    EXPECT_TRUE(stl_contains(s1, 3));
    EXPECT_TRUE(stl_contains(s1, 4));
    EXPECT_FALSE(stl_contains(s1, 5));

    unordered_set<int> s2{ 1, 2, 3, 4 };
    EXPECT_TRUE(stl_contains(s2, 3));
    EXPECT_TRUE(stl_contains(s2, 4));
    EXPECT_FALSE(stl_contains(s2, 5));

    map<int, const char*> m1{ { 1, "one" }, { 2, "two" } };
    EXPECT_TRUE(stl_contains(m1, 1));
    EXPECT_TRUE(stl_contains(m1, 2));
    EXPECT_FALSE(stl_contains(m1, 3));

    unordered_map<int, const char*> m2{ { 1, "one" }, { 2, "two" } };
    EXPECT_TRUE(stl_contains(m2, 1));
    EXPECT_TRUE(stl_contains(m2, 2));
    EXPECT_FALSE(stl_contains(m2, 3));

    int x = 5;
    int* px = &x;
    const int* cpx = &x;
    vector<int*> v2{ px };
    EXPECT_TRUE(stl_contains(v2, cpx));
}

TEST(container, remove) {
    vector<int> v1{ 1, 2, 3, 4 };
    stl_remove(v1, 4);
    EXPECT_TRUE(stl_contains(v1, 1));
    EXPECT_TRUE(stl_contains(v1, 2));
    EXPECT_TRUE(stl_contains(v1, 3));
    EXPECT_FALSE(stl_contains(v1, 4));
}

TEST(container, remove_if) {
    vector<int> v1{ 1, 2, 3, 4 };
    stl_remove_if(v1, [](int val) { return val > 2; });
    EXPECT_TRUE(stl_contains(v1, 1));
    EXPECT_TRUE(stl_contains(v1, 2));
    EXPECT_FALSE(stl_contains(v1, 3));
    EXPECT_FALSE(stl_contains(v1, 4));

    map<int, const char*> m1{ { 1, "one" }, { 2, "two" } };
    stl_remove_if(m1, [](const auto& val) -> bool { return val->first > 1; });
    EXPECT_TRUE(stl_contains(m1, 1));
    EXPECT_FALSE(stl_contains(m1, 2));

    unordered_map<int, const char*> m2{ { 1, "one" }, { 2, "two" } };
    stl_remove_if(m2, [](const auto& val) -> bool { return val->first > 1; });
    EXPECT_TRUE(stl_contains(m2, 1));
    EXPECT_FALSE(stl_contains(m2, 2));
}

TEST(container, insert_sorted) {
    vector<int> v1;
    stl_insert_sorted(v1, 4);
    stl_insert_sorted(v1, 3);
    stl_insert_sorted(v1, 2);
    stl_insert_sorted(v1, 1);
    EXPECT_EQ(v1[0], 1);
    EXPECT_EQ(v1[1], 2);
    EXPECT_EQ(v1[2], 3);
    EXPECT_EQ(v1[3], 4);

    vector<int> v2;
    stl_insert_sorted(v2, 2, [](int a, int b) -> bool { return a > b; });
    stl_insert_sorted(v2, 4, [](int a, int b) -> bool { return a > b; });
    stl_insert_sorted(v2, 1, [](int a, int b) -> bool { return a > b; });
    stl_insert_sorted(v2, 3, [](int a, int b) -> bool { return a > b; });
    EXPECT_EQ(v2[0], 4);
    EXPECT_EQ(v2[1], 3);
    EXPECT_EQ(v2[2], 2);
    EXPECT_EQ(v2[3], 1);
}
