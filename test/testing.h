/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_TESTING_H
#define MWR_TESTING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>
#include <random>

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
