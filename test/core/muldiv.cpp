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
#include "mwr/core/muldiv.h"

using namespace mwr;

TEST(muldiv, umul64) {
    std::mt19937 rng;
    std::uniform_int_distribution<u64> dist;

    for (size_t i = 0; i < 1000; i++) {
        u64 a = dist(rng);
        u64 b = dist(rng);

        u64 h0, l0;
        umul64(h0, l0, a, b);

        u64 h1, l1;
        umul64_slow(h1, l1, a, b);

        ASSERT_EQ(l0, a * b);
        ASSERT_EQ(l1, a * b);
        ASSERT_EQ(h0, h1);
    }
}

TEST(muldiv, imul64) {
    std::mt19937 rng;
    std::uniform_int_distribution<i64> dist;

    for (size_t i = 0; i < 1000; i++) {
        i64 a = dist(rng);
        i64 b = dist(rng);

        i64 h0, l0;
        imul64(h0, l0, a, b);

        i64 h1, l1;
        imul64_slow(h1, l1, a, b);

        ASSERT_EQ(l0, a * b);
        ASSERT_EQ(l1, a * b);
        ASSERT_EQ(h0, h1);
    }
}
