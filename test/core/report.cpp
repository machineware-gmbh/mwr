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

namespace N {

template <typename T>
struct struct_a {
    struct struct_b {
        void func() {
            auto bt = mwr::backtrace(1, 1);
            EXPECT_EQ(bt.size(), 1);
        }

        void func(T t) {
            auto bt = mwr::backtrace(1, 1);
            EXPECT_EQ(bt.size(), 1);
        }

        void func2() {
            auto bt = mwr::backtrace(1, 1);
            EXPECT_EQ(bt.size(), 1);
        }
    };
};

struct struct_u {
    template <int N>
    void unroll(double d) {
        unroll<N - 1>(d);
    }
};

template <>
void struct_u::unroll<0>(double d) {
    auto bt = mwr::backtrace(5, 1);
    EXPECT_EQ(bt.size(), 5);
    for (const auto& func : bt)
        std::cout << func << std::endl;
}
} // namespace N

TEST(utils, backtrace) {
    N::struct_a<int>::struct_b().func();
    N::struct_a<const char*>::struct_b().func("42");
    N::struct_a<N::struct_a<std::map<int, double> > >::struct_b().func2();
    N::struct_u().unroll<5>(42.0);
}
