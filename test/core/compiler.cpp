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
#include "mwr/core/compiler.h"

int global = 0;

MWR_DECL_CONSTRUCTOR static void do_init() {
    global = 1;
}

TEST(compiler, constructor) {
    EXPECT_EQ(global, 1);
}

#define MACRO macro
#define ABCD  "abcd"

TEST(compiler, str) {
    EXPECT_EQ(MWR_STR(hello), "hello");
    EXPECT_EQ(MWR_STR(MACRO), "macro");
    EXPECT_EQ(MWR_CAT(AB, CD), "abcd");
}
