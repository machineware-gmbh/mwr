/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"

#include "mwr/core/compiler.h"
#include "mwr/stl/strings.h"
#include "mwr/utils/locale.h"

using namespace mwr;

TEST(locale, guard) {
    EXPECT_STREQ(std::setlocale(LC_ALL, NULL), "C");
    {
        locale_guard guard(LC_ALL, "en_US.UTF-8");
        EXPECT_EQ(guard.saved, "C");

#ifndef MWR_WINDOWS
        // en_US.UTF-8 might not be installed, just expect when it is available
        if (strcmp(std::setlocale(LC_ALL, NULL), "en_US.UTF-8") == 0) {
            EXPECT_EQ(mkstr("%'d", 1000), "1,000");
        }
#endif
    }

    EXPECT_STREQ(std::setlocale(LC_ALL, NULL), "C");
}
