/******************************************************************************
 *                                                                            *
 * Copyright (C) 2026 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"

#include "mwr/utils/interval.h"
#include <type_traits>

using namespace mwr;

TEST(interval, traits) {
    EXPECT_FALSE(std::is_copy_constructible<interval_tree<int>>::value);
    EXPECT_FALSE(std::is_copy_assignable<interval_tree<int>>::value);
    EXPECT_TRUE(std::is_move_constructible<interval_tree<int>>::value);
    EXPECT_TRUE(std::is_move_assignable<interval_tree<int>>::value);
}

TEST(interval, insert_remove) {
    interval_tree<int> tree;
    auto it1 = tree.insert(5, 6, 1);
    auto it2 = tree.insert(1, 2, 2);
    auto it3 = tree.insert(6, 9, 3);
    auto it4 = tree.insert(6, 7, 4);
    EXPECT_TRUE(tree.remove(it1));
    EXPECT_TRUE(tree.remove(it3));
    EXPECT_TRUE(tree.remove(it2));
    EXPECT_TRUE(tree.remove(it4));
}

TEST(interval, iterators) {
    interval_tree<int> tree;
    tree.insert(3, 6, 4);
    tree.insert(2, 3, 2);
    tree.insert(1, 2, 1);
    tree.insert(3, 4, 3);
    tree.insert(6, 7, 6);
    tree.insert(5, 6, 5);
    tree.insert(7, 8, 7);

    int i = 0;
    for (auto it = tree.begin(); it != tree.end(); it++) {
        EXPECT_EQ(*it, ++i);
    }

    i = 0;
    for (const auto& val : tree) {
        EXPECT_EQ(val, ++i);
    }

    EXPECT_TRUE(tree.remove(tree.begin()));
    EXPECT_EQ(tree.size(), 6);
}

TEST(interval, const_iterators) {
    interval_tree<int> tree;
    tree.insert(3, 6, 4);
    tree.insert(2, 3, 2);
    tree.insert(1, 2, 1);
    tree.insert(3, 4, 3);
    tree.insert(6, 7, 6);
    tree.insert(5, 6, 5);
    tree.insert(7, 8, 7);

    int i = 0;
    const auto& ctree = tree;
    for (auto it = ctree.begin(); it != ctree.end(); it++) {
        EXPECT_EQ(*it, ++i);
    }

    i = 0;
    for (const auto& val : ctree) {
        EXPECT_EQ(val, ++i);
    }

    EXPECT_TRUE(tree.remove(ctree.begin()));
    EXPECT_EQ(tree.size(), 6);
}

TEST(interval, lookup) {
    interval_tree<int> tree;
    tree.insert(10, 20, 1);
    tree.insert(30, 40, 2);
    tree.insert(15, 35, 3);

    auto res1 = tree.find_overlaps(5, 15);
    EXPECT_EQ(res1.size(), 2);

    auto res2 = tree.find_overlaps(21, 29);
    EXPECT_EQ(res2.size(), 1);
    EXPECT_EQ(*res2[0], 3);

    auto res3 = tree.find_overlaps(50, 60);
    EXPECT_TRUE(res3.empty());

    const auto& ctree = tree;
    auto res4 = ctree.find_overlaps(5, 15);
    EXPECT_EQ(res4.size(), 2);

    auto res5 = ctree.find_overlaps(21, 29);
    EXPECT_EQ(res5.size(), 1);
    EXPECT_EQ(*res5[0], 3);

    auto res6 = ctree.find_overlaps(50, 60);
    EXPECT_TRUE(res6.empty());
}

TEST(interval, overlaps) {
    interval_tree<int> tree;
    tree.insert(10, 20, 1);
    tree.insert(30, 40, 2);

    EXPECT_TRUE(tree.overlaps(5, 15));
    EXPECT_TRUE(tree.overlaps(15, 25));
    EXPECT_TRUE(tree.overlaps(35, 45));
    EXPECT_TRUE(tree.overlaps(10, 40));

    EXPECT_FALSE(tree.overlaps(0, 9));
    EXPECT_FALSE(tree.overlaps(21, 29));
    EXPECT_FALSE(tree.overlaps(41, 50));
}

TEST(interval, clear) {
    interval_tree<int> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);

    tree.insert(10, 20, 1);
    tree.insert(30, 40, 2);

    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 2);

    tree.clear();

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

TEST(interval, move) {
    interval_tree<int> tree;
    tree.insert(10, 20, 1);
    tree.insert(30, 40, 2);

    interval_tree<int> move(std::move(tree));
    EXPECT_EQ(move.size(), 2);
    EXPECT_TRUE(move.overlaps(15, 16));
    EXPECT_TRUE(move.overlaps(35, 36));

    interval_tree<int> assign;
    assign.insert(50, 60, 3);
    assign = std::move(move);
    EXPECT_EQ(assign.size(), 2);
    EXPECT_TRUE(assign.overlaps(15, 16));
    EXPECT_TRUE(assign.overlaps(35, 36));
    EXPECT_FALSE(assign.overlaps(50, 60));
}

TEST(interval, duplicates) {
    interval_tree<int> tree;
    tree.insert(10, 20, 1);
    tree.insert(10, 20, 2);
    EXPECT_EQ(tree.size(), 2);

    auto overlaps = tree.find_overlaps(10, 20);
    EXPECT_EQ(overlaps.size(), 2);
}

TEST(interval, for_each) {
    interval_tree<int> tree;
    tree.insert(10, 20, 1);
    tree.insert(15, 25, 2);
    tree.insert(30, 40, 3);
    tree.insert(5, 12, 4);

    std::vector<int> visited;
    tree.for_each(
        12, 18,
        [&](u64 start, u64 end, const interval_tree<int>::iterator& it) {
            visited.push_back(*it);
            if (*it == 2) {
                *it = 200;
            }
        });

    std::sort(visited.begin(), visited.end());
    std::vector<int> expected = { 1, 2, 4 };
    EXPECT_EQ(visited, expected);

    std::vector<int> modified;
    tree.for_each(15, 25,
                  [&](u64, u64, const interval_tree<int>::iterator& it) {
                      modified.push_back(*it);
                  });

    std::sort(modified.begin(), modified.end());
    std::vector<int> expected_modified = { 1, 200 };
    EXPECT_EQ(modified, expected_modified);
}

TEST(interval, for_each_const) {
    interval_tree<int> tree;

    tree.insert(10, 20, 1);
    tree.insert(15, 25, 2);
    tree.insert(30, 40, 3);
    tree.insert(5, 12, 4);

    const interval_tree<int>& ctree = tree;
    std::vector<int> visited;

    ctree.for_each(
        12, 18,
        [&](u64 start, u64 end, const interval_tree<int>::const_iterator& it) {
            visited.push_back(*it);
        });

    std::sort(visited.begin(), visited.end());
    std::vector<int> expected = { 1, 2, 4 };
    EXPECT_EQ(visited, expected);

    std::vector<int> empty;
    ctree.for_each(
        50, 60, [&](u64, u64, const interval_tree<int>::const_iterator& it) {
            empty.push_back(*it);
        });
    EXPECT_TRUE(empty.empty());
}

TEST(interval, fuzzer) {
    interval_tree<int> tree;
    vector<interval_tree<int>::iterator> elements;

    for (size_t round = 0; round < 10000; round++) {
        size_t fuzz = (size_t)rand() % 100;
        if (fuzz < elements.size()) {
            auto it = elements.begin() + fuzz;
            EXPECT_TRUE(tree.remove(*it));
            elements.erase(it);
        } else {
            u64 start = (u64)rand() % 10000;
            u64 length = (u64)rand() % 10000;
            auto it = tree.insert(start, start + length, fuzz);
            elements.push_back(it);
        }

        tree.validate();
    }
}
