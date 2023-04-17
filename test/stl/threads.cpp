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
#include "mwr/stl/threads.h"

using namespace mwr;

TEST(threads, name) {
    atomic<bool> exitreq = false;
    thread t([&]() -> void {
        while (!exitreq)
            usleep(1);
    });

    ASSERT_TRUE(set_thread_name(t, "test_thread"));
    EXPECT_EQ(get_thread_name(t), "test_thread");

    exitreq = true;
    t.join();
}
