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

namespace mwr {
namespace publishers {

static bool supports_colors(bool use_cerr) {
    int fd = use_cerr ? STDERR_FDNO : STDOUT_FDNO;
    if (!is_tty(fd))
        return false;

    auto env = getenv("TERM");
    if (!env)
        return false;

    string term = env.value();

    static const string colors[]{
        "color", "linux", "xterm", "screen", "ansi",
    };

    for (const string& console : colors)
        if (term.find(console) != string::npos)
            return true;

    return false;
}

terminal::terminal(bool use_cerr):
    terminal(use_cerr, supports_colors(use_cerr)) {
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
