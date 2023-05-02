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
#include "mwr/core/types.h"
#include <type_traits>

using namespace mwr;

TEST(types, widen) {
    EXPECT_TRUE((std::is_same<widen<u8>::type, u16>::value));
    EXPECT_TRUE((std::is_same<widen<i8>::type, i16>::value));
    EXPECT_TRUE((std::is_same<widen<u16>::type, u32>::value));
    EXPECT_TRUE((std::is_same<widen<i16>::type, i32>::value));
    EXPECT_TRUE((std::is_same<widen<u32>::type, u64>::value));
    EXPECT_TRUE((std::is_same<widen<i32>::type, i64>::value));
    EXPECT_TRUE((std::is_same<widen<u64>::type, u64>::value));
    EXPECT_TRUE((std::is_same<widen<i64>::type, i64>::value));
    EXPECT_TRUE((std::is_same<widen<f32>::type, f64>::value));
    EXPECT_TRUE((std::is_same<widen<f64>::type, f64>::value));
}

TEST(types, narrow) {
    EXPECT_TRUE((std::is_same<narrow<u8>::type, u8>::value));
    EXPECT_TRUE((std::is_same<narrow<i8>::type, i8>::value));
    EXPECT_TRUE((std::is_same<narrow<u16>::type, u8>::value));
    EXPECT_TRUE((std::is_same<narrow<i16>::type, i8>::value));
    EXPECT_TRUE((std::is_same<narrow<u32>::type, u16>::value));
    EXPECT_TRUE((std::is_same<narrow<i32>::type, i16>::value));
    EXPECT_TRUE((std::is_same<narrow<u64>::type, u32>::value));
    EXPECT_TRUE((std::is_same<narrow<i64>::type, i32>::value));
    EXPECT_TRUE((std::is_same<narrow<f32>::type, f32>::value));
    EXPECT_TRUE((std::is_same<narrow<f64>::type, f32>::value));
}
