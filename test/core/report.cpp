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
#include "mwr/core/report.h"

TEST(report, what) {
    const char* what = "test report";
    const char* file = __FILE__;
    const size_t line = __LINE__;

    mwr::report rep(what, file, line);
    EXPECT_STREQ(rep.what(), what);
    EXPECT_STREQ(rep.file(), file);
    EXPECT_EQ(rep.line(), line);
}

TEST(report, report) {
    EXPECT_THROW(MWR_REPORT("test %d", 123), mwr::report);
    EXPECT_THROW(MWR_REPORT_ON(true, "test %d", 456), mwr::report);
    EXPECT_NO_THROW(MWR_REPORT_ON(false, "test"));
}

TEST(report, error) {
    MWR_ERROR_ON(false, "should not happen");
}
