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
#include "mwr/core/sfinae.h"

using namespace mwr;

template <typename T>
constexpr enable_if_signed_t<T> test_is_signed(T x) {
    return -x;
}

template <typename T>
constexpr enable_if_unsigned_t<T> test_is_signed(T x) {
    return x + 1;
}

TEST(sfinae, is_signed) {
    EXPECT_EQ(test_is_signed(14), -14);
    EXPECT_EQ(test_is_signed(100u), 101u);
}

struct my_struct {
    int a;
    std::string s;
};

using my_struct_has_a = MWR_DECL_MEMBER_CHECKER(my_struct, a);
using my_struct_has_b = MWR_DECL_MEMBER_CHECKER(my_struct, b);
using my_struct_has_s = MWR_DECL_MEMBER_CHECKER(my_struct, s);

TEST(sfinae, has_member) {
    if constexpr (!my_struct_has_a())
        GTEST_FAIL();
    if constexpr (my_struct_has_b())
        GTEST_FAIL();
    if constexpr (!my_struct_has_s())
        GTEST_FAIL();

    bool a = my_struct_has_a();
    bool b = my_struct_has_b();
    bool s = my_struct_has_s();

    EXPECT_TRUE(a);
    EXPECT_FALSE(b);
    EXPECT_TRUE(s);
}
