/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/logging/publishers/stream.h"

namespace mwr {
namespace publishers {

void stream::publish(const logmsg& msg) {
    os << msg << std::endl;
}

stream::stream(ostream& o): publisher(LOG_ERROR, LOG_DEBUG), os(o) {
    // nothing to do
}

stream::~stream() {
    // nothing to do
}

} // namespace publishers
} // namespace mwr
