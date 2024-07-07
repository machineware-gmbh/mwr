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
    EXPECT_DEATH({ MWR_ERROR("this is an error"); }, "this is an error");
}

TEST(report, error_on) {
    MWR_ERROR_ON(false, "should not happen");
}
