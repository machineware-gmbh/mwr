/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/logging/publishers/terminal.h"
#include "mwr/utils/terminal.h"

#include <unistd.h>

namespace mwr {
namespace publishers {

terminal::terminal(bool use_cerr):
    terminal(use_cerr, isatty(use_cerr ? STDERR_FILENO : STDIN_FILENO)) {
    // nothing to do
}

terminal::terminal(bool use_cerr, bool use_colors):
    publisher(), m_colors(use_colors), m_os(use_cerr ? std::cerr : std::cout) {
    // nothing to do
}

terminal::~terminal() {
    // nothing to do
}

const char* terminal::colors[NUM_LOG_LEVELS] = {
    /* [LOG_ERROR] = */ mwr::termcolors::RED,
    /* [LOG_WARN]  = */ mwr::termcolors::YELLOW,
    /* [LOG_INFO]  = */ mwr::termcolors::GREEN,
    /* [LOG_DEBUG] = */ mwr::termcolors::BLUE,
};

void terminal::publish(const logmsg& msg) {
    MWR_ERROR_ON(!m_os.good(), "log stream broken");

    stringstream ss;
    if (m_colors)
        ss << colors[msg.level];
    ss << msg;
    if (m_colors)
        ss << mwr::termcolors::CLEAR;
    ss << std::endl;

    m_os << ss.rdbuf() << std::flush;
}

} // namespace publishers
} // namespace mwr
