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
