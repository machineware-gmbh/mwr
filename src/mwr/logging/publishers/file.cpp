/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/logging/publishers/file.h"

namespace mwr {
namespace publishers {

file::file(const string& filename):
    publisher(LOG_ERROR, LOG_DEBUG),
    m_file(filename.c_str(), std::ios_base::out) {
    // nothing to do
}

file::~file() {
    // nothing to do
}

void file::publish(const logmsg& msg) {
    m_file << msg << std::endl;
}

} // namespace publishers
} // namespace mwr
