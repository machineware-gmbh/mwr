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

#ifndef MWR_TESTING_H
#define MWR_TESTING_H

#include <gtest/gtest.h>

#include <string>
#include <vector>

std::vector<std::string> args;

std::string get_resource_path(const std::string& name) {
    if (args.size() < 2) {
        ADD_FAILURE() << "test resource path information not provided";
        std::abort();
    }

    return args[1] + "/" + name;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);
    return RUN_ALL_TESTS();
}

#endif
