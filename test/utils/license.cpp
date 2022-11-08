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
#include "mwr/utils/license.h"

TEST(license, apache) {
    auto& license = mwr::license::find("Apache-2.0");
    EXPECT_EQ(license.name, "Apache License 2.0");
}

TEST(license, custom) {
    mwr::license custom("custom", "test license", true);
    auto& license = mwr::license::find("custom");
    EXPECT_EQ(license.name, custom.name);
}

TEST(license, copyright) {
    auto& license = mwr::license::find("Copyright 2022 Company");
    EXPECT_EQ(license, mwr::UNLICENSED);
    EXPECT_FALSE(license.libre);
}
