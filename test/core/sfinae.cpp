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
