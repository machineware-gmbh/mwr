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
#include "mwr/core/atomics.h"

using namespace mwr;

TEST(atomic, max) {
    i64 cmp, data = 5;
    EXPECT_EQ(atomic_max(&data, 6), 5);
    EXPECT_EQ(data, 6);

    EXPECT_EQ(atomic_max(&data, -1), 6);
    EXPECT_EQ(data, 6);

    cmp = -2;
    EXPECT_EQ(atomic_max_ptr(&data, &cmp, sizeof(data)), 6);
    EXPECT_EQ(data, 6);

    cmp = 9;
    EXPECT_EQ(atomic_max_ptr(&data, &cmp, sizeof(data)), 6);
    EXPECT_EQ(data, 9);
}

TEST(atomic, umax) {
    u64 cmp, data = 5;
    EXPECT_EQ(atomic_max(&data, 6), 5);
    EXPECT_EQ(data, 6);

    EXPECT_EQ(atomic_max(&data, -2), 6);
    EXPECT_EQ(data, -2);

    cmp = -1;
    EXPECT_EQ(atomic_umax_ptr(&data, &cmp, sizeof(data)), -2);
    EXPECT_EQ(data, -1);

    cmp = 9;
    EXPECT_EQ(atomic_umax_ptr(&data, &cmp, sizeof(data)), -1);
    EXPECT_EQ(data, -1);
}
