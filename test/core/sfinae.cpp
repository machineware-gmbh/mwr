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
